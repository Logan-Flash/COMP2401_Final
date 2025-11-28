#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

void roomarray_init(RoomArray* arr) {
    if (arr == NULL) return;
    arr->count = 0;
    // Optional: Zero out the pointers for debugging safety
    for (int i = 0; i < MAX_ROOMS; i++) {
        arr->data[i] = NULL;
    }
}
roomarray_add(struct RoomArray a, struct Room room){
	if (a == NULL || room == NULL){
		return;
	}
	if (a->count < MAX_ROOMS){
		a->data[count] = room;
		a->count++;
	}
}


void room_init(struct Room* room, const char* name, bool is_exit){
	
	strncpy(room->name, name, MAX_STR - 1); 
	room->name[MAX_ROOM_NAME - 1] = '\0'; 
	
	roomarray_init(&room->connections);
	
	room->ghost = NULL;
	
	for (int i = 0; i < MAX_HUNTERS; i++){
		room->hunters[i] = NULL:
	}
	room->num_of_hunters = 0;
	room->exit = is_exit;
	room->evidence = 0;
	sem_init(&room->mutex, 0, 1);
	
}

void rooms_connect(struct Room* a, struct Room* b){
	roomarray_add(a->connections, b);
	roomarray_add(b->connections, a);
}
void room_create(Room** room, int id, const char* name){

}
void room_add_hunter(Room* room, Hunter* h){

}
void room_add_ghost(Room* room, Ghost* ghost){

}
void room_print(Room* room){

}
void room_cleanup(Room* room){

}
void room_remove_hunter(Room* room, Hunter* h);
bool room_has_ghost(Room* room);