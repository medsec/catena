CC=gcc
HDIR=./blake2
TARGETDIR=../

ARCH := $(shell getconf LONG_BIT)

CFLAGS_32=-fomit-frame-pointer -O3 -std=c99 -fgnu89-inline -s -msse4.2 -W -Wall  -L$(HDIR) -I$(HDIR)
CFLAGS_64=-fomit-frame-pointer -O3 -std=c99 -fgnu89-inline -s -m64 -msse4.2 -W -Wall  -L$(HDIR) -I$(HDIR)

HASH=$(HDIR)/blake2b.c

all: blake2b sha512

blake2b: catena-BRG-blake2b-test catena-BRG-blake2b-test_vectors catena-DBG-blake2b-test catena-DBG-blake2b-test_vectors 
sha512: catena-BRG-sha512-test catena-BRG-sha512-test_vectors catena-DBG-sha512-test catena-DBG-sha512-test_vectors
DBG: catena-DBG-blake2b-test catena-DBG-blake2b-test_vectors catena-DBG-sha512-test catena-DBG-sha512-test_vectors
BRG: catena-BRG-blake2b-test catena-BRG-blake2b-test_vectors catena-BRG-sha512-test catena-BRG-sha512-test_vectors
catena: catena-DBG catena-BRG

catena-DBG-blake2b-test:
	$(CC) $(CFLAGS_$(ARCH)) -o $(TARGETDIR)/$@ test-catena.c catena.c catena-blake2b.c catena-DBG.c $(HASH)


catena-DBG-blake2b-test_vectors:
	$(CC) $(CFLAGS_$(ARCH)) -o $(TARGETDIR)/$@ catena_test_vectors.c catena.c catena-blake2b.c catena-DBG.c $(HASH)


catena-DBG-sha512-test:
	$(CC) $(CFLAGS_$(ARCH)) -o $(TARGETDIR)/$@ test-catena.c catena.c catena-sha512.c catena-DBG.c -lssl -lcrypto


catena-DBG-sha512-test_vectors:
	$(CC) $(CFLAGS_$(ARCH)) -o $(TARGETDIR)/$@ catena_test_vectors.c catena.c catena-sha512.c catena-DBG.c -lssl -lcrypto

catena-DBG:
	$(CC) $(CFLAGS_$(ARCH)) -c $@.c catena-DBG.c $(HASH)



catena-BRG-blake2b-test:
	$(CC) $(CFLAGS_$(ARCH)) -o $(TARGETDIR)/$@ test-catena.c catena.c catena-blake2b.c catena-BRG.c $(HASH)


catena-BRG-blake2b-test_vectors:
	$(CC) $(CFLAGS_$(ARCH)) -o $(TARGETDIR)/$@ catena_test_vectors.c catena.c catena-blake2b.c catena-BRG.c $(HASH)


catena-BRG-sha512-test:
	$(CC) $(CFLAGS_$(ARCH)) -o $(TARGETDIR)/$@ test-catena.c catena.c catena-sha512.c catena-BRG.c -lssl -lcrypto


catena-BRG-sha512-test_vectors:
	$(CC) $(CFLAGS_$(ARCH)) -o $(TARGETDIR)/$@ catena_test_vectors.c catena.c catena-sha512.c catena-BRG.c -lssl -lcrypto

catena-BRG:
	$(CC) $(CFLAGS_$(ARCH)) -c $@.c catena-BRG.c $(HASH)


clean:
	rm -f  *~ *.o
	rm -rf $(TARGETDIR)
