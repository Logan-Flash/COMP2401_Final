#include <stdio.h>
#include <stdlib.h>
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
	ghost->room = random_room(house);
	if (ghost->room != NULL) {
        // Tell the room the ghost is here
        room_add_ghost(ghost->room, ghost);
    
        log_ghost_init(ghost->id, ghost->room->name, ghost->type); // Logs the ghost being initiated
    }
	ghost->boredom = 0;
	ghost->exited = false;
}

void ghost_move(Ghost* ghost) {
    if (ghost->room == NULL) return;
    
    Room* curr = ghost->room;

    if (curr->num_of_hunters > 0) { // Cannot move if hunters are in the room 
        return; 
    }

    if (curr->connections.count > 0) {
        int r = rand() % curr->connections.count;
        Room* next = curr->connections.data[r];

        // Perform Move
        room_remove_ghost(curr, ghost);
        room_add_ghost(next, ghost);
        ghost->room = next;
        
        log_ghost_move(ghost->id, ghost->boredom, curr->name, next->name);
    }
}

// Leaves evidence
void ghost_haunt(Ghost* ghost) {
    if (ghost->room == NULL) return;
    
    EvidenceByte all_evidence = (EvidenceByte)ghost->type; 
    
    
    // 2. Extract valid evidence bits into an array
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
    
    // 3. Pick one random piece from the valid options
    if (count > 0) {
        int r = rand() % count;
        EvidenceByte chosen_evidence = valid_bits[r];
        
        // 4. Drop it (Add to room)
        ghost->room->evidence |= chosen_evidence;
        
        log_ghost_evidence(ghost->id, ghost->boredom, ghost->room->name, chosen_evidence);
    }
}

/* New Function: ghost_take_turn
   Performs one "tick" of the ghost logic.
   Returns true if the ghost is still active, false if it exited.
*/
// You should add 'bool ghost_take_turn(Ghost* ghost);' to defs.h
bool ghost_take_turn(Ghost* ghost) {
    if (ghost->exited) return false;

    if (ghost->room->num_of_hunters > 0) {
        ghost->boredom = 0;
    } else {
        ghost->boredom++;
    }

    if (ghost->boredom >= ENTITY_BOREDOM_MAX) {
        ghost->exited = true;
        log_ghost_exit(ghost->id, ghost->boredom, ghost->room->name);
        room_remove_ghost(ghost->room, ghost);
        return false;
    }

    int action = rand() % 3; // 0 = Idle, 1 = Haunt, 2 = Move
    
    if (action == 0) {
        log_ghost_idle(ghost->id, ghost->boredom, ghost->room->name);
    } else if (action == 1) {
        ghost_haunt(ghost);
    } else {
        ghost_move(ghost);
    }

    return true;
}
