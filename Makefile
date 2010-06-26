CFLAGS = -Wall -Wextra 

all: client server

client: client.o common.o

server: server.o common.o

server.o: server.c common.h

client.o: client.c common.h

common.o: common.c common.h

.PHONY: clean
clean:
	rm -f client server *.o
