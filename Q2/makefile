CC = gcc
CFlags =-Wall 

.PHONY: all clean

all: mync

mync: mync.c
	$(CC) $(CFLAGS) -o mync mync.c

clean:
	rm -f *.o mync
