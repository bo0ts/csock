CFLAGS = -Wall -Wextra 

all: client

client: client.o common.o

client.o: client.c common.h

common.o: common.c common.h

.PHONY: clean
clean:
	rm -f client *.o
