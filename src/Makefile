CC=gcc
HDIR=./blake2
TARGETDIR=../

CFLAGS=-fomit-frame-pointer -O3 -std=c99 -fgnu89-inline -s -W -Wall -L$(HDIR) -I$(HDIR)

HASH=$(HDIR)/blake2b.c

#enable 64bit if avaiable
ARCH := $(shell getconf LONG_BIT)

ifeq ($(ARCH), 64)
	CFLAGS += -m64
endif

# Enable all possible SSE levels
SSE_TAGS = $(shell /bin/grep -m 1 flags /proc/cpuinfo | /bin/grep -o \
	'sse2\|sse3\|ssse3\|sse4a\|sse4.1\|sse4.2' | sed  's/\_/./g')

ifneq (${SSE_TAGS},) 
    CFLAGS += -mfpmath=sse
    CFLAGS += $(foreach tag,$(SSE_TAGS),-m$(tag))
endif

BASEFILES = catena.c catena-helpers.c

#if requested use the full hash function for H' instead of single rounds
ifndef FULLHASH
	CFLAGS += -DFAST
endif

.PHONY: all blake2b Butterfly Dragonfly catena

all: blake2b catena

blake2b: catena-Dragonfly-blake2b-test catena-Dragonfly-blake2b-test_vectors catena-Butterfly-blake2b-test catena-Butterfly-blake2b-test_vectors 
Butterfly: catena-Butterfly-blake2b-test catena-Butterfly-blake2b-test_vectors
Dragonfly: catena-Dragonfly-blake2b-test catena-Dragonfly-blake2b-test_vectors
catena: catena-BRG catena-DBG

catena-Butterfly-blake2b-test:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ test-catena.c $(BASEFILES) catena-blake2b.c catena-DBG.c $(HASH)


catena-Butterfly-blake2b-test_vectors:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ catena_test_vectors.c $(BASEFILES) catena-blake2b.c catena-DBG.c $(HASH)

catena-DBG:
	$(CC) $(CFLAGS) -c $(BASEFILES) catena-DBG.c $(HASH)

catena-Dragonfly-blake2b-test:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ test-catena.c $(BASEFILES) catena-blake2b.c catena-BRG.c $(HASH)


catena-Dragonfly-blake2b-test_vectors:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ catena_test_vectors.c $(BASEFILES) catena-blake2b.c catena-BRG.c $(HASH)

catena-BRG:
	$(CC) $(CFLAGS) -c $(BASEFILES) catena-BRG.c $(HASH)

clean:
	rm -f  *~ *.o
