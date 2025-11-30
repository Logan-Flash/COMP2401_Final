#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
void house_init(House* house){
	house->starting_room = NULL;
	house->room_count = 0;
	house->hunter_capacity = 0;
	house->hunters = NULL;
	
	sem_init(&house->casefile.mutex, 0, 1);
	house->casefile.collected = 0;
	house->casefile.solved = false;
	ghost_init(&house->ghost, house);
}
void house_init_hunters(House* house){
	if (house == NULL){
		return;
	}
	// Allocate space for the maximum possible hunters
    house->hunters = malloc(sizeof(Hunter) * MAX_HUNTERS);
    
    char input_buffer[MAX_HUNTER_NAME];
    int count = 0;

    // MATCHING SCREENSHOT: Header message
    printf("Enter hunters one at a time. Type 'done' as the name to finish.\n");

    for (int i = 0; i < MAX_HUNTERS; i++) {
        // MATCHING SCREENSHOT: Name Prompt
        printf("Enter hunter name (max 63 characters) or 'done' to finish: ");
        
        if (fgets(input_buffer, MAX_HUNTER_NAME, stdin) == NULL) {
            break; 
        }
        
        // Remove newline
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        // Check for 'done'
        if (strcmp(input_buffer, "done") == 0) {
            break;
        }

        // Save the name
        char hunter_name[MAX_HUNTER_NAME];
        strncpy(hunter_name, input_buffer, MAX_HUNTER_NAME);

        printf("Enter hunter ID (integer): ");
        int hunter_id = 0;
        
        scanf("%d", &hunter_id);
        
        // Clears anything after the id is entered
        while (getchar() != '\n');
        
        hunter_init(&house->hunters[i], hunter_id, hunter_name, house);
        
        if (house->starting_room != NULL) { // Puts the hunter in the starting room
            room_add_hunter(house->starting_room, &house->hunters[i]);
        }
        count++;
    }
    // Update capacity to the actual number of hunters created
    house->hunter_capacity = count;
}