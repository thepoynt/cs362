CC = g++
CFLAGS=-g 

all: server client

server:	main.cpp
	$(CC) $(CFLAGS) -o server main.cpp

client: client.cpp
	$(CC) $(CFLAGS) -o client client.cpp

clean:
	rm -r *.dSYM server client
