CC=gcc
HDIR=./blake2
CFLAGS=-fomit-frame-pointer -O3 -std=c99  -s -m64 -W -Wall  -L$(HDIR) -I$(HDIR)

HASH=$(HDIR)/blake2b.c

all: test-catena catena_test_vectors


test-catena:
	$(CC) $(CFLAGS) -o $@ $@.c catena.c $(HASH)

catena_test_vectors:
	$(CC) $(CFLAGS) -o $@ $@.c catena.c $(HASH)

catena:
	$(CC) $(CFLAGS) -c $@.c $(HASH)


clean:
	rm -f test-catena catena_test_vectors *~ *.o
