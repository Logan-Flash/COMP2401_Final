#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"

int main() {
	srand(time(NULL));
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
    
   	pthread_t ghost_tid;
   	pthread_t* hunter_tids = malloc(sizeof(pthread_t) * house.hunter_capacity);
    
    // 1. Create Ghost Thread
    pthread_create(&ghost_tid, NULL, ghost_thread, &house.ghost);

    // 2. Create Hunter Threads
    for (int i = 0; i < house.hunter_capacity; i++) {
        pthread_create(&hunter_tids[i], NULL, hunter_thread, &house.hunters[i]);
    }

    // 3. Wait for Ghost
    pthread_join(ghost_tid, NULL);
    
    // 4. Wait for Hunters
    for (int i = 0; i < house.hunter_capacity; i++) {
        pthread_join(hunter_tids[i], NULL);
    }

    // Free the thread ID array
    free(hunter_tids);
    // Evidence
    EvidenceByte collected = house.casefile.collected;
    
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
    
	printf("Actual Ghost Type: %s\n", ghost_to_string(house.ghost.type));
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
