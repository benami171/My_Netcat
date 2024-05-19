CC = gcc
CFLAGS = -Wall -g

.PHONY: all clean

all: mync ttt ttt-client

mync: mync.o
	$(CC) $(CFLAGS) -o $@ $^

ttt: ttt.o
	$(CC) $(CFLAGS) -o $@ $^

ttt-client: ttt-client.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o mync ttt ttt-client