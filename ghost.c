#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"


enum GhostType random_ghost_type(){
	enum GhostType types[] = {
        GH_POLTERGEIST, GH_THE_MIMIC, GH_HANTU, GH_JINN, 
        GH_PHANTOM, GH_BANSHEE, GH_GORYO, GH_BULLIES, 
        GH_MYLING, GH_OBAKE, GH_YUREI, GH_ONI, 
        GH_MOROI, GH_REVENANT, GH_SHADE, GH_ONRYO, 
        GH_THE_TWINS, GH_DEOGEN, GH_THAYE, GH_YOKAI, 
        GH_WRAITH, GH_RAIJU, GH_MARE, GH_SPIRIT
    };
    int num_of_types = sizeof(types)/sizeof(types[0]);
    int rand_type = rand()%num_of_types;
    printf("%d",rand_type);
    return types[rand_type];
} 
struct Room* random_room(House* house){
	if (house->room_count == 0){
		return NULL;	
	}
	int rand_room = rand()%house->room_count;
	return &house->rooms[rand_room];
}
void ghost_init(Ghost* ghost, House* house){
    if (ghost == NULL || house == NULL){
        return;
    }
    ghost->id = DEFAULT_GHOST_ID;
    ghost->type = random_ghost_type();
    
    // Get a random room that's not the Van
    do {
        ghost->room = random_room(house);
    } while (ghost->room != NULL && strcmp(ghost->room->name, STARTING_ROOM_NAME) == 0);
    
    if (ghost->room != NULL) {
        room_add_ghost(ghost->room, ghost);
        log_ghost_init(ghost->id, ghost->room->name, ghost->type);
    }
    ghost->boredom = 0;
    ghost->exited = false;
}

void ghost_move(Ghost* ghost) {
    if (ghost->room == NULL) return;
    
    Room* curr = ghost->room;
    
    // Don't lock yet - just read connection count
    if (curr->connections.count == 0) return;
    
    // Pick a random room
    int r = rand() % curr->connections.count;
    Room* next = curr->connections.data[r];
    
    if (next == NULL) return;
    if (strcmp(next->name, STARTING_ROOM_NAME) == 0) return;
    
    // lock both rooms in consistent order
    lock_two(curr, next);
    
    // Check conditions AFTER acquiring both locks
    if (curr->num_of_hunters > 0) {
        unlock_two(curr, next);
        return;
    }
    
    if (next->num_of_hunters > 0) {  // Can't move to room with hunters
        unlock_two(curr, next);
        return;
    }
    
    // Perform the move
    room_remove_ghost(curr, ghost);
    room_add_ghost(next, ghost);
    ghost->room = next;
    
    unlock_two(curr, next);
    
    log_ghost_move(ghost->id, ghost->boredom, curr->name, next->name);
}
// Leaves evidence
void ghost_haunt(Ghost* ghost) {
    if (ghost->room == NULL) return;
    
    EvidenceByte all_evidence = (EvidenceByte)ghost->type; 
    
    
    // Extract valid evidence bits into an array
    EvidenceByte valid_bits[3]; // Max 3 bits per ghost
    int count = 0;
    
    // Iterate through all 7 evidence types to find which ones this ghost has
    for (int i = 0; i < 7; i++) {
        EvidenceByte check = (1 << i);
        if (all_evidence & check) {
            valid_bits[count] = check;
            count++;
        }
    }
    
    // Pick one random piece from the valid options
    if (count > 0) {
        int r = rand() % count;
        EvidenceByte chosen_evidence = valid_bits[r];
        
        // Drop it (Add to room)
        sem_wait(&ghost->room->mutex);
        ghost->room->evidence |= chosen_evidence;
        sem_post(&ghost->room->mutex);
        log_ghost_evidence(ghost->id, ghost->boredom, ghost->room->name, chosen_evidence);
    }
}

bool ghost_take_turn(Ghost* ghost) {
    if (ghost == NULL || ghost->exited) return false;

    if (ghost->room == NULL) {
        ghost->boredom++;
        if (ghost->boredom >= ENTITY_BOREDOM_MAX) {
            ghost->exited = true;
            log_ghost_exit(ghost->id, ghost->boredom, "Unknown");
            return false;
        }
        return true;
    }

    // Check boredom BEFORE action
    if (ghost->boredom >= ENTITY_BOREDOM_MAX) {
        Room* r = ghost->room;
        if (r != NULL) {
            sem_wait(&r->mutex);
            room_remove_ghost(r, ghost);
            sem_post(&r->mutex);
        }
        ghost->exited = true;
        const char* room_name = "Unknown";
        if (r != NULL) {
            room_name = r->name;
        }
        log_ghost_exit(ghost->id, ghost->boredom, room_name);
        return false;
    }

    // Take random action
    int action = rand() % 3; // 0 = Idle, 1 = Haunt, 2 = Move

    if (action == 1) {
        ghost_haunt(ghost);
    }
    else if (action == 2) {
        ghost_move(ghost);
    }
    else {
        if (ghost->room != NULL) {
            sem_wait(&ghost->room->mutex);
            log_ghost_idle(ghost->id, ghost->boredom, ghost->room->name);
            sem_post(&ghost->room->mutex);
        }
    }

    // Check if hunters are present and update boredom accordingly
    if (ghost->room != NULL) {
        sem_wait(&ghost->room->mutex);
        bool hunters_present = ghost->room->num_of_hunters > 0;
        sem_post(&ghost->room->mutex);

        if (hunters_present) {
            ghost->boredom = 0;  // Reset if hunters present
        } else {
            ghost->boredom++;     // Increment if alone
        }
    }

    return true;
}
void* ghost_thread(void* arg) {
    Ghost* ghost = (Ghost*)arg;
    while(ghost_take_turn(ghost)) {
        usleep(10000); 
    }
    return NULL;
}
