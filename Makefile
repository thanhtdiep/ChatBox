CC = gcc 
CFLAGS = -Wall # Show all reasonable warnings
LDFLAGS = -lpthread

all: Server Client

server: Server
		-o Server Server.c

client: Client
		-o Client Client.c

clean:
	rm -f Server Client
 
.PHONY: clean
