all: client

client: client.c

.PHONY: clean
clean:
	rm -f client
