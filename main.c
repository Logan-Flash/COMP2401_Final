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
    printf("--- Initializing Ghost ---\n");
    ghost_init(&house.ghost, &house);
    printf("Ghost Type: %s", ghost_to_string(house.ghost.type));
    
    printf("\n--- Starting Simulation ---\n");
    
    bool ghost_running = true;
    bool hunters_remaining = true;

    while (ghost_running && hunters_remaining) {
        ghost_running = ghost_take_turn(&house.ghost);

        int active = 0;
        for (int i = 0; i < house.hunter_capacity; i++) {
            if (hunter_take_turn(&house.hunters[i])) {
                active++;
            }
        }
        if (active == 0) hunters_remaining = false;
    }

    printf("\n--- FINAL RESULTS ---\n");
    printf("Ghost: %s\n", ghost_to_string(house.ghost.type));
    
    for (int i = 0; i < house.hunter_capacity; i++) {
        Hunter* h = &house.hunters[i];
        printf("Hunter %s: %s\n", h->name, exit_reason_to_string(h->reason));
    }

    // Evidence
    EvidenceByte collected = house.casefile.collected;
    
    // --- DEBUG PRINT: Show exactly what evidence was found ---
    printf("\nCollected Evidence Byte: %02X (Hex)\n", collected); // Print raw hex
    printf("Evidence Items: ");
    
    // Get all possible evidence types from helper
    const enum EvidenceType* ev_list;
    int ev_count = get_all_evidence_types(&ev_list);
    int found_count = 0;

    for (int i = 0; i < ev_count; i++) {
        // Check if the specific bit is set
        if (collected & ev_list[i]) {
            printf("[%s] ", evidence_to_string(ev_list[i]));
            found_count++;
        }
    }
    if (found_count == 0) {
        printf("None");
    }
    printf("\n");
    // ---------------------------------------------------------

    const enum GhostType* all_ghosts;
    int num_ghosts = get_all_ghost_types(&all_ghosts);
    bool hunters_won = false;

    for (int i = 0; i < num_ghosts; i++) {
        if (collected == all_ghosts[i]) {
            printf("Evidence matches: %s\n", ghost_to_string(all_ghosts[i]));
            if (all_ghosts[i] == house.ghost.type){
            	hunters_won = true;
            }
            break;
        }
    }

    // 3. Final Conclusion
    if (hunters_won) {
        printf("RESULT: Hunters Won! Correctly identified the ghost.\n");
    } else {
        printf("RESULT: Ghost Won! Hunters failed to find the correct 3 pieces of evidence.\n");
    }
 	house_cleanup(&house);
    return 0;
}
