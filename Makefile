CFLAGS=-c -g -Wall
CC=gcc

all: client host

host: host.o readthread.o
	$(CC) -o host host.o readthread.o -lpthread

client: client.o readthread.o
	$(CC) -o client client.o readthread.o -lpthread

clean:
	rm -f *.o client host logs/*

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

