#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"

int main() {

    /*
    1. Initialize a House structure.
    2. Populate the House with rooms using the provided helper function.
    3. Initialize all of the ghost data and hunters.
    4. Create threads for the ghost and each hunter.
    5. Wait for all threads to complete.
    6. Print final results to the console:
         - Type of ghost encountered.
         - The reason that each hunter exited
         - The evidence collected by each hunter and which ghost is represented by that evidence.
    7. Clean up all dynamically allocated resources and call sem_destroy() on all semaphores.
    */
    printf("--- Initializing House ---\n");
	struct House house;
	house_init(&house);
	printf("--- Populating Rooms ---\n");
	house_populate_rooms(&house);
	printf("--- Initializing Hunters ---\n");
    house_init_hunters(&house);
    printf("--- Initializing Ghosts ---\n");
    ghost_init(&house.ghost, &house);
    printf("\n--- Starting Simulation ---\n");
    
    bool ghost_running = true;
    bool hunters_remaining = true;

    while (ghost_running && hunters_remaining) {
        // A. Ghost Turn
        ghost_running = ghost_take_turn(&house.ghost);

        // B. Hunter Turns
        int active_hunters = 0;
        for (int i = 0; i < house.hunter_capacity; i++) {
            if (hunter_take_turn(&house.hunters[i])) {
                active_hunters++;
            }
        }

        // C. Check Termination
        if (active_hunters == 0) {
            hunters_remaining = false;
        }

    }

    /* 6. Print Final Results */
    printf("\n=========================================\n");
    printf("       FINAL RESULTS       \n");
    printf("=========================================\n");

    // Ghost Result
    printf("Ghost: %s\n", ghost_to_string(house.ghost.type));
    
    // Check room using standard if/else logic instead of ternary operator
    char* ghost_room_name;
    if (house.ghost.room != NULL) {
        ghost_room_name = house.ghost.room->name;
    } else {
        ghost_room_name = "Exited";
    }
    printf("       Ended in room: %s\n", ghost_room_name);

    // Hunter Results
    printf("\n--- Hunters ---\n");
    for (int i = 0; i < house.hunter_capacity; i++) {
        Hunter* h = &house.hunters[i];
        printf("Hunter %s [%s]: ", h->name, exit_reason_to_string(h->reason));
        
        if (h->reason == LR_EVIDENCE) {
            printf("SUCCESS! Found enough evidence.\n");
        } else if (h->reason == LR_AFRAID) {
            printf("RAN AWAY in fear!\n");
        } else {
            printf("Left due to boredom.\n");
        }
    }

    // Evidence Results
    printf("\n--- Evidence Collected ---\n");
    EvidenceByte collected = house.casefile.collected;
    printf("Evidence Found: ");
    
    const enum EvidenceType* ev_list;
    int ev_count = get_all_evidence_types(&ev_list);
    int found_count = 0;
    
    for(int i=0; i<ev_count; i++) {
        if (collected & ev_list[i]) {
            printf("[%s] ", evidence_to_string(ev_list[i]));
            found_count++;
        }
    }
    if (found_count == 0) printf("None");
    printf("\n");

    // Determine Winner
    // (You can implement the complex ghost matching logic here later)
    if (found_count >= 3) {
        printf("RESULT: Hunters Won! (3 evidence found)\n");
    } else {
        printf("RESULT: Ghost Won! (Not enough evidence)\n");
    }
 
    return 0;
}
