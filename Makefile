all:	final
		
final: main.c room.c house.c ghost.c helpers.c hunter.c
			gcc -Wall -g -pthread -o final main.c room.c house.c helpers.c hunter.c ghost.c
clean:
			rm -f final *.csv