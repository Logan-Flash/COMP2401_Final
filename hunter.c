#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/* Hunter Functions */
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
    
    // Init stack (Pass address)
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
    if (hunter == NULL || hunter->curr_room == NULL) return;
    Room* r = hunter->curr_room;
    
    if (r->evidence & hunter->curr_device) {
        log_evidence(hunter->id, hunter->boredom, hunter->fear, r->name, hunter->curr_device);
        hunter->casefile->collected |= hunter->curr_device;
        r->evidence &= ~hunter->curr_device;
        hunter->boredom = 0;
        
        if (strcmp(r->name, STARTING_ROOM_NAME) != 0) {
            hunter->returning = true;
            log_return_to_van(hunter->id, hunter->boredom, hunter->fear, r->name, hunter->curr_device, true);
        }
    } else {
        // Random return chance
        if (rand() % 100 < 10) {
             if (strcmp(r->name, STARTING_ROOM_NAME) != 0 && !hunter->returning) {
                hunter->returning = true;
                log_return_to_van(hunter->id, hunter->boredom, hunter->fear, r->name, hunter->curr_device, true);
            }
        }
    }
}

void hunter_move(Hunter* hunter) {
    if (hunter == NULL || hunter->curr_room == NULL) return;
    
    Room* curr = hunter->curr_room;
    Room* next = NULL;

    if (hunter->returning) {
        // Return logic
        next = roomstack_pop(&hunter->path);
        if (next == NULL) return; // Should allow 'next' to be NULL if at start? 
    } else {
        // Explore logic
        if (curr->connections.count > 0) {
            int r = rand() % curr->connections.count;
            next = curr->connections.data[r];
            roomstack_push(&hunter->path, curr);
        }
    }

    if (next != NULL) {
        room_remove_hunter(curr, hunter);
        room_add_hunter(next, hunter);
        hunter->curr_room = next;
        log_move(hunter->id, hunter->boredom, hunter->fear, curr->name, next->name, hunter->curr_device);
    }
}

bool hunter_take_turn(Hunter* hunter) {
    if (hunter->exited){
    	return false;
    }
    
    if (hunter->curr_room->ghost != NULL) {
        hunter->fear++;
        hunter->boredom = 0;
    } else {
        hunter->boredom++;
    }
	hunter_collect(hunter);
    if (hunter->fear >= HUNTER_FEAR_MAX) {
        log_exit(hunter->id, hunter->boredom, hunter->fear, hunter->curr_room->name, hunter->curr_device, LR_AFRAID);
        hunter->reason = LR_AFRAID;
        hunter->exited = true;
        room_remove_hunter(hunter->curr_room, hunter);
        return false;
    }
    if (hunter->boredom >= ENTITY_BOREDOM_MAX) {
        log_exit(hunter->id, hunter->boredom, hunter->fear, hunter->curr_room->name, hunter->curr_device, LR_BORED);
        hunter->reason = LR_BORED;
        hunter->exited = true;
        room_remove_hunter(hunter->curr_room, hunter);
        return false;
    }

    if (strcmp(hunter->curr_room->name, STARTING_ROOM_NAME) == 0 && hunter->returning) {
        log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->curr_room->name, hunter->curr_device, false);
        hunter->returning = false;
        roomstack_cleanup(&hunter->path);
        
        if (count_evidence(hunter->casefile->collected) >= 3) {
             log_exit(hunter->id, hunter->boredom, hunter->fear, hunter->curr_room->name, hunter->curr_device, LR_EVIDENCE);
             hunter->reason = LR_EVIDENCE;
             hunter->exited = true;
             room_remove_hunter(hunter->curr_room, hunter);
             return false;
        }
        
        enum EvidenceType new_d = random_device();
        while(new_d == hunter->curr_device) new_d = random_device();
        log_swap(hunter->id, hunter->boredom, hunter->fear, hunter->curr_device, new_d);
        hunter->curr_device = new_d;
        hunter->boredom = 0;
    }
    hunter_move(hunter);
    return true;
}

void hunter_print(Hunter* h) {
	if (h == NULL){
		printf("Hunter doesn't exist");
	}
    printf("Hunter %s\n", h->name);
}

void hunter_cleanup(Hunter* h) {
    roomstack_cleanup(&h->path);
}