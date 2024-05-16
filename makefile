CC = gcc
CFlags =-Wall 

.PHONY: all clean

all: ttt mync

mync: mync.c
	$(CC) $(CFLAGS) -o mync mync.c

ttt: ttt.o
	$(CC) $(CFlags) ttt.o -o ttt -lm

ttt.o:
	$(CC) $(CFlags) -c ttt.c -o ttt.o

clean:
	rm -f *.o ttt mync