CC=gcc
HDIR=./blake2
CFLAGS=-fomit-frame-pointer -O3 -std=c99 -fgnu89-inline -s -m64 -msse4.2 -W -Wall  -L$(HDIR) -I$(HDIR)
#CFLAGS=-g -std=c99 -fgnu89-inline -m64 -W -Wall  -L$(HDIR) -I$(HDIR)

HASH=$(HDIR)/blake2b.c

all: blake2b sha512 multhash

blake2b: catena-blake2b-test catena-blake2b-test_vectors 
sha512: catena-sha512-test catena-sha512-test_vectors
multhash: catena-multhash-test catena-multhash-test_vectors

catena-blake2b-test:test-catena.c catena.c catena-blake2b.c catena.h hash.h
	$(CC) $(CFLAGS) -o $@ test-catena.c catena.c catena-blake2b.c $(HASH)


catena-blake2b-test_vectors:catena_test_vectors.c catena.c catena-blake2b.c catena.h hash.h
	$(CC) $(CFLAGS) -o $@ catena_test_vectors.c catena.c catena-blake2b.c $(HASH)


catena-sha512-test: test-catena.c catena.c catena-sha512.c catena.h hash.h
	$(CC) $(CFLAGS) -o $@ test-catena.c catena.c catena-sha512.c -lssl -lcrypto


catena-sha512-test_vectors: catena_test_vectors.c catena.c catena-sha512.c catena.h hash.h
	$(CC) $(CFLAGS) -o $@ catena_test_vectors.c catena.c catena-sha512.c -lssl -lcrypto

catena:
	$(CC) $(CFLAGS) -c $@.c $(HASH)

catena-multhash-test: test-catena.c catena.c catena-sha512.c catena-multhash.c catena.h hash.h
	$(CC) $(CFLAGS) -DFAST_HASH -o $@ test-catena.c catena.c catena-sha512.c catena-multhash.c -lssl -lcrypto


catena-multhash-test_vectors: catena_test_vectors.c catena.c catena-sha512.c catena.h hash.h
	$(CC) $(CFLAGS) -DFAST_HASH -o $@ catena_test_vectors.c catena.c catena-sha512.c catena-multhash.c -lssl -lcrypto

clean:
	rm -f  *~ *.o catena-blake2b-test catena-blake2b-test_vectors catena-sha512-test catena-sha512-test_vectors catena-blake2b-opt-test catena-multhash-test catena-multhash-test_vectors
