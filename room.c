#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"


void roomarray_add(RoomArray* arr, Room* room){
	if (arr == NULL || room == NULL){
		return;
	}
	if (arr->count < MAX_ROOMS){
		arr->data[arr->count] = room;
		arr->count++;
	}
}
void roomarray_init(RoomArray* arr) {
    if (arr == NULL) return;
    arr->count = 0;
    
    for (int i = 0; i < MAX_ROOMS; i++) {
        arr->data[i] = NULL;
    }
}

void room_init(Room* room, const char* name, bool is_exit){
	
	strncpy(room->name, name, MAX_HUNTER_NAME - 1); 
	room->name[MAX_ROOM_NAME - 1] = '\0'; 
	
	roomarray_init(&room->connections);
	
	room->ghost = NULL;
	
	for (int i = 0; i < MAX_HUNTERS; i++){
		room->hunters[i] = NULL;
	}
	room->num_of_hunters = 0;
	room->exit = is_exit;
	room->evidence = 0;
	sem_init(&room->mutex, 0, 1);
	
}

void room_connect(Room* a, Room* b){
	roomarray_add(&a->connections, b);
	roomarray_add(&b->connections, a);
}

void room_add_hunter(Room* room, Hunter* h){
	if (room == NULL || h == NULL || room->num_of_hunters >= MAX_HUNTERS){
		return;
	}
	 for (int i = 0; i < MAX_HUNTERS; i++) {
        if (room->hunters[i] == NULL) {
            room->hunters[i] = h;
            room->num_of_hunters++;
            break;
        }
    }
}
void room_remove_hunter(Room* room, Hunter* h){
	if (room == NULL || h == NULL){ 
		return;
	}
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (room->hunters[i] == h) {
            room->hunters[i] = NULL;
            room->num_of_hunters--;
            break;
        }
    }
}
void room_add_ghost(Room* room, Ghost* ghost){
	if (room == NULL || ghost == NULL){
		return;
	}
	room->ghost = ghost;
	ghost->room = room;
}
void room_remove_ghost(Room* room, Ghost* ghost){
	if (room == NULL || ghost == NULL){
		return;
	}
	if (room->ghost == ghost){
		room->ghost = NULL;
	}
	if (ghost->room == room){
		ghost->room = NULL;	
	}
}
void room_print(Room* room){

}
void room_cleanup(Room* room){

}
void room_remove_hunter(Room* room, Hunter* h);
bool room_has_ghost(Room* room);