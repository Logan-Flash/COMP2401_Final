#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>


#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057
#define STARTING_ROOM_NAME "Van"

// --- Typedefs ---
typedef unsigned char EvidenceByte;
// Typedefs for structs
typedef struct Ghost Ghost;
typedef struct Room Room;
typedef struct RoomArray RoomArray;
typedef struct House House;
typedef struct Hunter Hunter;
typedef struct RoomNode RoomNode;
typedef struct RoomStack RoomStack;
typedef struct CaseFile CaseFile;

// --- Enums ---
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

// --- Structures ---
struct CaseFile {
    EvidenceByte collected;
    bool solved;
    sem_t mutex;
};

struct RoomArray {
    Room* data[MAX_CONNECTIONS];
    int count;
};

struct Ghost {
    int id;
    enum GhostType type;
    struct Room* room;
    int boredom;
    bool exited;
};

struct Room {
    char name[MAX_ROOM_NAME];
    struct RoomArray connections;
    Ghost* ghost;
    Hunter* hunters[MAX_ROOM_OCCUPANCY];
    int num_of_hunters;
    bool exit; 
    EvidenceByte evidence;
    sem_t mutex;
};


struct RoomNode {
    Room* room;
    struct RoomNode* next;
};

struct RoomStack {
    RoomNode* front;
    int size;
};

struct Hunter {
    int id;
    char name[MAX_HUNTER_NAME];
    Room* curr_room;
    struct CaseFile* casefile;
    enum EvidenceType curr_device;
    struct RoomStack path; // NOT a pointer
    int fear;
    int boredom;
    enum LogReason reason;
    bool exited;
    bool returning;
};

struct House {
    struct Room* starting_room;
    struct Room rooms[MAX_ROOMS];
    int room_count;
    struct Hunter* hunters;
    int hunter_capacity;
    struct CaseFile casefile;
    struct Ghost ghost; 
};

// --- Prototypes ---

// House
void house_init(House* house);
void house_init_hunters(House* house);
void house_populate_rooms(House* house);
void house_cleanup(House* house);

// Room
void room_init(Room* room, const char* name, bool is_exit);
void room_connect(Room* a, Room* b);
void room_add_hunter(Room* room, Hunter* h);
void room_remove_hunter(Room* room, Hunter* h);
void room_add_ghost(Room* room, Ghost* ghost);
void room_remove_ghost(Room* room, Ghost* ghost);
void room_cleanup(Room* room);
void roomarray_add(RoomArray* arr, Room* room);

// Room Stack 
void roomstack_init(RoomStack* stack);
void roomstack_push(RoomStack* stack, Room* room);
Room* roomstack_pop(RoomStack* stack);
void roomstack_cleanup(RoomStack* stack);

// Ghost
void ghost_init(Ghost* ghost, House* house);
bool ghost_take_turn(Ghost* ghost);
void* ghost_thread(void* arg);

// Hunter
void hunter_init(Hunter* hunter, int id, char* name, House* house);
bool hunter_take_turn(Hunter* hunter);
void hunter_cleanup(Hunter* hunter);
void* hunter_thread(void* arg);
void hunter_print(Hunter* hunter);

// Threading
void lock_two(Room* a, Room* b);
void unlock_two(Room* a, Room* b);

#endif