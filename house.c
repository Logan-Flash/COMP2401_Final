#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

void house_init(struct House* house){
	roomarray_init(&house->rooms);
	
	house->starting_room = NULL;
	
	house->hunter_capacity = 0;
	hosue->hunters = NULL;
	
	sem_init(&house->casefile.mutex, 0, 1);
	house->casefile.collected = 0;
	house->casefile.solved = false;
}