CC=g++
CFLAGS=-std=c++17 -Wall -Wextra -Werror -pedantic -g
CLINKS=-pthread -lrt

PROGRAM_NAME = ipk-simpleftp-server
PROGRAM = ipk-simpleftp-server
ARGS =

REMOVE = rm -f
OBJ = ipk-simpleftp-server.o ipk-simpleftp-client.o 
BIN = ipk-simpleftp-server ipk-simpleftp-client 

all: ipk-simpleftp-server ipk-simpleftp-client

ipk-simpleftp-server: ipk-simpleftp-server.o
	$(CC) $(CFLAGS) ipk-simpleftp-server.o -o ipk-simpleftp-server $(CLINKS)
ipk-simpleftp-server.o: ipk-simpleftp-server.cpp

ipk-simpleftp-client: ipk-simpleftp-client.o
	$(CC) $(CFLAGS) ipk-simpleftp-client.o -o ipk-simpleftp-client $(CLINKS)
ipk-simpleftp-client.o: ipk-simpleftp-client.cpp

.PHONY: runserver 
runserver: ${PROGRAM_NAME}
	./${PROGRAM} ${ARGS}

.PHONY: runclient 
runclient: ${PROGRAM_NAME}
	./${PROGRAM} ${ARGS}

clean: clear
clear:
	$(REMOVE) $(OBJ) $(BIN)


