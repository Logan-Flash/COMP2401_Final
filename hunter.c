#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "defs.h"
#include "helpers.h"

/* Stack Functions */
void roomstack_init(RoomStack* stack) {
    if (stack == NULL) return;
    stack->front = NULL;
    stack->size = 0;
}

void roomstack_push(RoomStack* stack, Room* room) {
    if (stack == NULL || room == NULL) return;
    RoomNode* new_node = malloc(sizeof(RoomNode));
    if (new_node == NULL) return;
    new_node->room = room;
    new_node->next = stack->front;
    stack->front = new_node;
    stack->size++;
}

Room* roomstack_pop(RoomStack* stack) {
    if (stack == NULL || stack->front == NULL) return NULL;
    RoomNode* temp = stack->front;
    Room* room = temp->room;
    stack->front = temp->next;
    free(temp);
    stack->size--;
    return room;
}

void roomstack_cleanup(RoomStack* stack) {
    if (stack == NULL) return;
    while (stack->front != NULL) {
        roomstack_pop(stack);
    }
}

int count_evidence(EvidenceByte collected) {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if ((collected >> i) & 1) count++;
    }
    return count;
}

enum EvidenceType random_device(){
    enum EvidenceType devices[] = {
        EV_EMF, EV_ORBS, EV_RADIO, EV_TEMPERATURE, EV_FINGERPRINTS, EV_WRITING, EV_INFRARED 
    };
    int num = sizeof(devices)/sizeof(devices[0]);
    return devices[rand() % num];
}

void hunter_init(Hunter* hunter, int id, char* name, House* house){
    if (hunter == NULL) return;
    hunter->id = id;
    strncpy(hunter->name, name, MAX_HUNTER_NAME - 1);
    hunter->name[MAX_HUNTER_NAME - 1] = '\0';
    
    hunter->curr_room = house->starting_room;
    hunter->casefile = &house->casefile;
    hunter->curr_device = random_device();
    
    // Init stack 
    roomstack_init(&hunter->path);
    
    hunter->fear = 0;
    hunter->boredom = 0;
    hunter->exited = false;
    hunter->returning = false;
    
    if (hunter->curr_room) {
        log_hunter_init(id, hunter->curr_room->name, hunter->name, hunter->curr_device);
    }
}

void hunter_collect(Hunter* hunter) {
    if (hunter == NULL || hunter->curr_room == NULL){
        return;
    }
    Room* r = hunter->curr_room;
    
    sem_wait(&r->mutex);
    bool has_evidence = (r->evidence & hunter->curr_device);
    
    if (has_evidence) {
        // Remove evidence while still holding the lock
        r->evidence &= ~hunter->curr_device;
        sem_post(&r->mutex);
        
        log_evidence(hunter->id, hunter->boredom, hunter->fear, r->name, hunter->curr_device);
        
        // Lock shared case file
        sem_wait(&hunter->casefile->mutex);
        hunter->casefile->collected |= hunter->curr_device;
        bool hunters_won = count_evidence(hunter->casefile->collected) >= 3; // Checks if the hunters have won (found 3 evidence)
        sem_post(&hunter->casefile->mutex);
        
        hunter->boredom = 0;
        
        if (hunters_won) { 
             log_exit(hunter->id, hunter->boredom, hunter->fear, r->name, hunter->curr_device, LR_EVIDENCE);
             hunter->reason = LR_EVIDENCE;
             hunter->exited = true;
             
             sem_wait(&r->mutex);
             room_remove_hunter(r, hunter);
             sem_post(&r->mutex);
             return;
        }
        
        if (strcmp(r->name, STARTING_ROOM_NAME) != 0) {
            hunter->returning = true;
            log_return_to_van(hunter->id, hunter->boredom, hunter->fear, r->name, hunter->curr_device, true);
        }
    } else {
        sem_post(&r->mutex);
        
        // Random return to van chance
        if (rand() % 100 < 5) {
             if (strcmp(r->name, STARTING_ROOM_NAME) != 0 && !hunter->returning) {
                hunter->returning = true;
                log_return_to_van(hunter->id, hunter->boredom, hunter->fear, r->name, hunter->curr_device, true);
            }
        }
    }
}
void lock_two(Room* a, Room* b) { // Unlocks both rooms (used to prevent deadlocks)
    Room* first = a;
    Room* second = b;

    if (first > second) {
        Room* tmp = first;
        first = second;
        second = tmp;
    }

    sem_wait(&first->mutex);
    sem_wait(&second->mutex);
}

void unlock_two(Room* a, Room* b) { // Unlocks both rooms (used to prevent deadlocks)
    Room* first = a;
    Room* second = b;

    if (first > second) {
        Room* tmp = first;
        first = second;
        second = tmp;
    }

    sem_post(&second->mutex);
    sem_post(&first->mutex);
}
void move_randomly(Hunter* hunter) {
    Room* curr = hunter->curr_room;
    
    // Lock to check connections
    sem_wait(&curr->mutex);
    int count = curr->connections.count;
    sem_post(&curr->mutex);

    if (count > 0) {
        int r = rand()%count;
        
        // Temporarily access data without lock
        Room* next = curr->connections.data[r];
        if (next == NULL) return;

        lock_two(curr, next);

        // Check capacity of target
        if (next->num_of_hunters < MAX_ROOM_OCCUPANCY) { 
            roomstack_push(&hunter->path, curr);
            room_remove_hunter(curr, hunter);
            room_add_hunter(next, hunter);
            hunter->curr_room = next;
            log_move(hunter->id, hunter->boredom, hunter->fear, curr->name, next->name, hunter->curr_device);
        }
        
        unlock_two(curr, next);
    }
}

void move_return(Hunter* hunter) { // Moves the hunter to the van (by iterating through a stack, one step at a time)
    if (hunter->path.front == NULL){ 
    	return; 
	}
    Room* curr = hunter->curr_room;
    Room* prev = roomstack_pop(&hunter->path);

    if (prev != NULL) {
        lock_two(curr, prev);

        if (prev->num_of_hunters < MAX_ROOM_OCCUPANCY) {
            room_remove_hunter(curr, hunter);
            room_add_hunter(prev, hunter);
            hunter->curr_room = prev;
            log_move(hunter->id, hunter->boredom, hunter->fear, curr->name, prev->name, hunter->curr_device);
        } else {
            roomstack_push(&hunter->path, prev);
        }

        unlock_two(curr, prev);
    }
}

void hunter_move(Hunter* hunter) { // Wrapper for different move functions
    if (hunter == NULL || hunter->curr_room == NULL) return;

    if (hunter->returning) {
        move_return(hunter);
    } else {
        move_randomly(hunter);
    }
}

bool hunter_take_turn(Hunter* hunter) {
    if (hunter->exited || hunter == NULL){
    	return false;
    }
    sem_wait(&hunter->curr_room->mutex); // Locks room
    bool ghost_in_room = hunter->curr_room->ghost != NULL;
    sem_post(&hunter->curr_room->mutex); // Unlocks room
    
    if (ghost_in_room){ // Checks if the ghost is in the room
    	hunter->fear++;
    	hunter->boredom = 0;
	} else {
		hunter->boredom++;
	}
	hunter_collect(hunter);
    if (hunter->fear >= HUNTER_FEAR_MAX) { // If hunter gets too scared (leaves)
        log_exit(hunter->id, hunter->boredom, hunter->fear, hunter->curr_room->name, hunter->curr_device, LR_AFRAID);
        hunter->reason = LR_AFRAID;
        hunter->exited = true;
        
        sem_wait(&hunter->curr_room->mutex);
        room_remove_hunter(hunter->curr_room, hunter);
        sem_post(&hunter->curr_room->mutex);
        return false;
    }
    if (hunter->boredom >= ENTITY_BOREDOM_MAX) { // If hunter gets too bored and leaves
        log_exit(hunter->id, hunter->boredom, hunter->fear, hunter->curr_room->name, hunter->curr_device, LR_BORED);
        hunter->reason = LR_BORED;
        hunter->exited = true;
        
        sem_wait(&hunter->curr_room->mutex);
        room_remove_hunter(hunter->curr_room, hunter);
        sem_post(&hunter->curr_room->mutex);
        
        return false;
    }

    if (strcmp(hunter->curr_room->name, STARTING_ROOM_NAME) == 0 && hunter->returning) {
        log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->curr_room->name, hunter->curr_device, false);
        hunter->returning = false;
        roomstack_cleanup(&hunter->path);
        
        // Lock the casefile
        sem_wait(&hunter->casefile->mutex);
        bool hunters_won = count_evidence(hunter->casefile->collected) >= 3;
        sem_post(&hunter->casefile->mutex); // Unlock the casefile
        if (hunters_won) { // If found all evidence
             log_exit(hunter->id, hunter->boredom, hunter->fear, hunter->curr_room->name, hunter->curr_device, LR_EVIDENCE);
             hunter->reason = LR_EVIDENCE;
             hunter->exited = true;
             
             sem_wait(&hunter->curr_room->mutex);
             room_remove_hunter(hunter->curr_room, hunter);
             sem_post(&hunter->curr_room->mutex);
             return false;
        }
        // Swap devices
        enum EvidenceType new_d = random_device();
        while(new_d == hunter->curr_device) new_d = random_device();
        log_swap(hunter->id, hunter->boredom, hunter->fear, hunter->curr_device, new_d);
        hunter->curr_device = new_d;
        hunter->boredom = 0;
    }
    hunter_move(hunter);
    return true;
}
void* hunter_thread(void* arg) {
    Hunter* hunter = (Hunter*)arg;
    while(hunter_take_turn(hunter)) {
        usleep(10000); 
    }
    return NULL;
}

void hunter_print(Hunter* h) { // Prints hunter by name
	if (h == NULL){
		printf("Hunter doesn't exist");
	}
    printf("Hunter %s\n", h->name);
}

void hunter_cleanup(Hunter* h) { // Frees the stack
    roomstack_cleanup(&h->path);
}