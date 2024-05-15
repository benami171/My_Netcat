CC = gcc
CFlags = -Wall -g

.PHONY: all clean

all: ttt

ttt: ttt.o
	$(CC) $(CFlags) ttt.o -o ttt

ttt.o:
	$(CC) $(CFlags) -c ttt.c -o ttt.o

clean:
	rm -f *.o ttt