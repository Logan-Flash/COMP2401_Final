#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"
void roomstack_init(RoomStack* stack){
	stack->front = NULL;
	stack->size = 0;
}
void roomstack_cleanup(RoomStack* stack){
	(void)stack;
}
enum EvidenceType random_device(){
	enum EvidenceType devices[] = {
		EV_EMF, EV_ORBS, EV_RADIO, EV_TEMPERATURE, EV_FINGERPRINTS, EV_WRITING, EV_INFRARED 
	};
	int num_of_devices = sizeof(devices)/sizeof(devices[0]);
	return devices[rand()%num_of_devices];
}
void hunter_init(Hunter* hunter, int id, char* name, House* house){
	strncpy(hunter->name, name, MAX_HUNTER_NAME - 1);
	hunter->name[MAX_HUNTER_NAME - 1] = '\0';
	
	hunter->curr_room = house->starting_room;
	hunter->curr_device = random_device();
	hunter->casefile = &house->casefile;
	roomstack_init(&hunter->path);
	hunter->fear = 0;
	hunter->boredom = 0;
	hunter->exited = false;
	hunter->id = id;
	log_hunter_init(hunter->id, hunter->curr_room->name, hunter->name, hunter->curr_device); 
}
void hunter_print(Hunter* hunter){
    if (hunter == NULL) return;
    const char* device_name = evidence_to_string(hunter->curr_device);

    printf("Hunter: %-15s | Room: %-15s | Device: %s\n", 
           hunter->name, hunter->curr_room->name, device_name);
}
int count_evidence(EvidenceByte collected) {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if ((collected >> i) & 1) {
            count++;
        }
    }
    return count;
}
void hunter_collect(Hunter* hunter) {
	if (hunter->curr_room == NULL) {
        printf("[ERROR] Hunter %s is in a NULL room!\n", hunter->name);
        return;
    }
    if (hunter->casefile == NULL) {
        printf("[ERROR] Hunter %s has no CaseFile!\n", hunter->name);
        return;
    }
    Room* r = hunter->curr_room;
    
    // Check if the room has the evidence detectable by this hunter's device
    if (r->evidence & hunter->curr_device) {
        // Evidence found!
        log_evidence(0, hunter->boredom, hunter->fear, r->name, hunter->curr_device);
        
        // Add to shared casefile
        hunter->casefile->collected |= hunter->curr_device;
        
        // Remove from room (so others don't find the same bit indefinitely)
        r->evidence &= ~hunter->curr_device;
        
        // Reset boredom on success
        hunter->boredom = 0;
        if (count_evidence(hunter->casefile->collected) >= 3) { // Hunter leaves if collected enough evidence
            log_exit(0, hunter->boredom, hunter->fear, r->name, hunter->curr_device, LR_EVIDENCE);
            hunter->reason = LR_EVIDENCE;
            hunter->exited = true;
            room_remove_hunter(r, hunter);
        }
    }
    
}


void hunter_move(Hunter* hunter) {
	if (hunter == NULL || hunter->curr_room == NULL) return;
    Room* curr = hunter->curr_room;
    
    if (curr->connections.count > 0) {
        int r = rand_int_threadsafe(0, curr->connections.count);
        Room* next = curr->connections.data[r];
        
        // Move Logic
        room_remove_hunter(curr, hunter);
        room_add_hunter(next, hunter);
        hunter->curr_room = next;
        
        log_move(0, hunter->boredom, hunter->fear, curr->name, next->name, hunter->curr_device);
    }
}

/* New Function: hunter_take_turn 
   Performs one "tick" of hunter logic.
*/
// Add 'bool hunter_take_turn(Hunter* h);' to defs.h
bool hunter_take_turn(Hunter* hunter) {
    if (hunter->exited) return false;

    // 1. Update Stats (R-17)
    if (hunter->curr_room->ghost != NULL) {
        hunter->fear++;
        hunter->boredom = 0;
    } else {
        hunter->boredom++;
    }

    // 2. Check Exits (R-19)
    if (hunter->fear >= HUNTER_FEAR_MAX) {
        log_exit(0, hunter->boredom, hunter->fear, hunter->curr_room->name, hunter->curr_device, LR_AFRAID);
        hunter->reason = LR_AFRAID;
        hunter->exited = true;
        room_remove_hunter(hunter->curr_room, hunter);
        return false;
    }
    if (hunter->boredom >= ENTITY_BOREDOM_MAX) {
        log_exit(0, hunter->boredom, hunter->fear, hunter->curr_room->name, hunter->curr_device, LR_BORED);
        hunter->reason = LR_BORED;
        hunter->exited = true;
        room_remove_hunter(hunter->curr_room, hunter);
        return false;
    }

    // 3. Collect Evidence
    hunter_collect(hunter);
	if (hunter->exited){
		hunter->reason = LR_EVIDENCE;
		return false;
	}
    // 4. Move
    hunter_move(hunter);

    return true;
}
