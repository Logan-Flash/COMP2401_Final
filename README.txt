COMP2401 Final Project

What the program does:

Simulates a Ghost Hunt: Creates a multi-threaded simulation of hunters and a ghost moving through a house (Willow Street House from Phasmaphobia).

Manages House and Rooms: Dynamically allocates memory for the House, Room array, and Hunter collection.

Tracks Hunters:

	Hunters move autonomously between connected rooms using a stack to trace their way back to the van if they get scared, bored, or find evidence of the ghost.

	They collect evidence (EMF, Temperature, Fingerprints, etc.) using bitwise operations.

	They monitor their Fear and Boredom levels, exiting the simulation if levels get too high.

Tracks the Ghost:

	The ghost wanders randomly (or haunts) based on its boredom level.

	It leaves specific evidence types in rooms for hunters to find.

Prevents Deadlocks: Uses a specific semaphore locking order (by memory address) to ensure thread safety when entities move between rooms.

Logs Activity: Automatically logs movement, evidence collection, and exits using the provided helper functions.

Cleans Up Memory: Manages all dynamically allocated memory (linked lists, node structures, entity structs) to ensure no memory leaks on exit.

List of Files and Purpose

	main.c: The entry point. Initializes the House/Rooms, starts all threads, and prints final results.

	defs.h: Stores global constants (MAX_FEAR, etc.) and enum definitions.

	house.c: Manages the main House structure and the dynamic array of Hunters.

	room.c: Defines Room logic, connections, evidence storage, and the RoomStack for hunter backtracking.

	hunter.c: Contains the Hunter thread loop, movement logic, and evidence collection.

	ghost.c: Contains the Ghost thread loop, haunting logic, and boredom updates.

	helpers.c/h: Provided logging functions .

	Makefile: Handles compilation.

To run the program execute the following commands in the terminal:
make
./final
The first command compiles the c files into one executable file final using the Makefile.
The second command runs the simulation.

(Optional) To clean up files:
make clean
This removes all object files and the executable along with log files.

Sources and Collaboration
	
Resources Used:

	Course Notes: Chapter 5 ConccurrentComputing for threading, Chapter 4 BuildsAndMakeFiles for my Makefile,
	 and Chapter 2  DataRepresentation for bit operations when working with EvidenceByte



Collaboration:
	Discussed memory allocation and how the overall logic works with Kishaan Gidda
	Discussed room entity limits and MAX_OCCUPANCY with Michael Holmes

Assumptions

	Deadlock Prevention: 
		To prevent deadlocks when moving between rooms, 
		I implemented a helper function that accepts two room pointers. It compares their addresses (if (first > second) swap)
		and ensures sem_wait is always called on the lower memory address first.

	Hunter Movement: 
		If a hunter attempts to move to a room that is full, 
		the move fails and they wait for the next turn (they do not retry immediately).

	Ghost Start: 
		The ghost begins in a random room chosen uniformly from the room list.

	Randomness: The random number generator is seeded exactly once in main.c.


Credits

Logan McFarling 101341717