#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

/*
    You are free to rename all of the types and functions defined here.

    The ghost ID must remain the same for the validator to work correctly.
*/

#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_HUNTERS 10
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057

typedef unsigned char EvidenceByte; // Just giving a helpful name to unsigned char for evidence bitmasks
typedef struct Ghost Ghost;
typedef struct GhostNode GhostNode;
typedef struct GhostList GhostList;
typedef struct Room Room;
typedef struct RoomArray RoomArray;
typedef struct House House;
typedef struct Hunter Hunter;
typedef struct RoomNode RoomNode;
typedef struct RoomStack RoomStack;

enum LogReason {
    LR_EVIDENCE = 0,
    LR_BORED = 1,
    LR_AFRAID = 2
};

enum EvidenceType {
    EV_EMF          = 1 << 0,
    EV_ORBS         = 1 << 1,
    EV_RADIO        = 1 << 2,
    EV_TEMPERATURE  = 1 << 3,
    EV_FINGERPRINTS = 1 << 4,
    EV_WRITING      = 1 << 5,
    EV_INFRARED     = 1 << 6,
};

enum GhostType {
    GH_POLTERGEIST  = EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING,
    GH_THE_MIMIC    = EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO,
    GH_HANTU        = EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS,
    GH_JINN         = EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF,
    GH_PHANTOM      = EV_FINGERPRINTS | EV_INFRARED    | EV_RADIO,
    GH_BANSHEE      = EV_FINGERPRINTS | EV_INFRARED    | EV_ORBS,
    GH_GORYO        = EV_FINGERPRINTS | EV_INFRARED    | EV_EMF,
    GH_BULLIES      = EV_FINGERPRINTS | EV_WRITING     | EV_RADIO,
    GH_MYLING       = EV_FINGERPRINTS | EV_WRITING     | EV_EMF,
    GH_OBAKE        = EV_FINGERPRINTS | EV_ORBS        | EV_EMF,
    GH_YUREI        = EV_TEMPERATURE  | EV_INFRARED    | EV_ORBS,
    GH_ONI          = EV_TEMPERATURE  | EV_INFRARED    | EV_EMF,
    GH_MOROI        = EV_TEMPERATURE  | EV_WRITING     | EV_RADIO,
    GH_REVENANT     = EV_TEMPERATURE  | EV_WRITING     | EV_ORBS,
    GH_SHADE        = EV_TEMPERATURE  | EV_WRITING     | EV_EMF,
    GH_ONRYO        = EV_TEMPERATURE  | EV_RADIO       | EV_ORBS,
    GH_THE_TWINS    = EV_TEMPERATURE  | EV_RADIO       | EV_EMF,
    GH_DEOGEN       = EV_INFRARED     | EV_WRITING     | EV_RADIO,
    GH_THAYE        = EV_INFRARED     | EV_WRITING     | EV_ORBS,
    GH_YOKAI        = EV_INFRARED     | EV_RADIO       | EV_ORBS,
    GH_WRAITH       = EV_INFRARED     | EV_RADIO       | EV_EMF,
    GH_RAIJU        = EV_INFRARED     | EV_ORBS        | EV_EMF,
    GH_MARE         = EV_WRITING      | EV_RADIO       | EV_ORBS,
    GH_SPIRIT       = EV_WRITING      | EV_RADIO       | EV_EMF,
};

struct CaseFile {
    EvidenceByte collected; // Union of all of the evidence bits collected between all hunters
    bool         solved;    // True when >=3 unique bits set
    sem_t        mutex;     // Used for synchronizing both fields when multithreading
};
struct RoomArray {
	Room* data [MAX_ROOMS];
	int count;
};
// Implement here based on the requirements, should all be allocated to the House structure
struct Room {
	char name [MAX_ROOM_NAME];
	struct RoomArray connections;
	Ghost *ghost;
	Hunter* hunters[MAX_HUNTERS];
	int num_of_hunters;
	bool exit; 
	EvidenceByte evidence;
};
struct RoomNode {
	Room* room;
	struct Room* next_room;
};

struct RoomStack {
	RoomNode* front;
	int size;
	int top;
};

// Implement here based on the requirements, should be allocated to the House structure
struct Ghost {
	int id;
	enum GhostType type;
	struct Room *room;
	int boredom;
	bool exited;
};
struct GhostNode {
	Ghost* ghost;
	GhostNode* nextGhost;
	GhostNode* previousGhost;
};

struct GhostList {
	GhostNode* headGhost;
	GhostNode* tailGhost;
};

struct Hunter {
	char name [MAX_HUNTER_NAME];
	Room* curr_room;
	struct CaseFile* casefile;
	enum EvidenceType curr_device;
	struct RoomStack path; 
	int fear;
	int boredom;
	enum LogReason reason;
	bool exited;
	
};


// Can be either stack or heap allocated
struct House {
    struct Room* starting_room; // Needed by house_populate_rooms, but can be adjusted to suit your needs.
	struct RoomArray rooms [MAX_ROOMS];
	struct Hunter *hunters;
	int hunter_capacity;
	struct CaseFile casefile;
	struct GhostList ghosts;
};

/* The provided `house_populate_rooms()` function requires the following functions.
   You are free to rename them and change their parameters and modify house_populate_rooms()
   as needed as long as the house has the correct rooms and connections after calling it.
*/

void room_init(struct Room* room, const char* name, bool is_exit);
void rooms_connect(struct Room* a, struct Room* b); // Bidirectional connection
void room_create(Room** room, int id, const char* name);
void room_add_hunter(Room* room, Hunter* h);
void room_add_ghost(Room* room, Ghost* ghost);
void room_print(Room* room);
void room_cleanup(Room* room); 
void room_remove_hunter(Room* room, Hunter* h);
bool room_has_ghost(Room* room);

void roomarray_init(RoomArray* arr);
void roomarray_add(RoomArray* arr, Room* room);
void roomarray_print(RoomArray* arr);
void roomarray_cleanup(RoomArray* arr);

void roomstack_init(RoomStack* stack);
Room* roomstack_push(RoomStack* stack, Room* room);
Room* roomstack_peek(RoomStack* stack);
void roomstack_cleanup(RoomStack* stack);

void ghost_create(Ghost* ghost);
void ghost_print(Ghost* ghost);
void ghost_cleanup(Ghost* ghost);

void ghostlist_init(GhostList* list);
void ghostlist_push(GhostList* list, GhostNode node);
void ghostlist_print(GhostList* list);
void ghostlist_cleanup(GhostList* list);

void ghostnode_cleanup(GhostNode* node);

void hunter_create(Hunter* hunter);
void hunter_print(Hunter* hunter);
void hunter_cleanup(Hunter* hunter);



#endif // DEFS_H
