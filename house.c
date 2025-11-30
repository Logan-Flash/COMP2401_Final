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
}
void house_init_hunters(House* house){
	if (house == NULL){
		return;
	}
    
    char input_buffer[MAX_HUNTER_NAME];
    int count = 0;

    printf("Enter hunters one at a time. Type 'done' as the name to finish.\n");

    for (int i = 0; i < MAX_HUNTERS; i++) {
        // MATCHING SCREENSHOT: Name Prompt
        printf("Enter hunter name (max 63 characters) or 'done' to finish: ");
        
        if (fgets(input_buffer, MAX_HUNTER_NAME, stdin) == NULL) {
            break; 
        }
        
        // Remove newline
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        // Check for to see if user is done
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
        
        Hunter* temp = realloc(house->hunters,sizeof(Hunter) * (count+1));
        house->hunters = temp;
        hunter_init(&house->hunters[count], hunter_id, hunter_name, house);
        
        if (house->starting_room != NULL) { // Puts the hunter in the starting room
            room_add_hunter(house->starting_room, &house->hunters[i]);
        }
        count++;
    }
    // Update capacity to the actual number of hunters created
    house->hunter_capacity = count;
}

void house_cleanup(House* house) {
    if (house == NULL) return;
    if (house->hunters != NULL) {
        for (int i = 0; i < house->hunter_capacity; i++) {
            hunter_cleanup(&house->hunters[i]);
        }
        free(house->hunters);
    }
    sem_destroy(&house->casefile.mutex);
    for (int i = 0; i < house->room_count; i++) {
        room_cleanup(&house->rooms[i]);
    }
}