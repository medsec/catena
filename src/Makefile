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
	'sse\|sse2\|sse3\|ssse3\|sse4a\|sse4.1\|sse4.2\|sse5' | sed  's/\_/./g')

ifneq (${SSE_TAGS},) 
    CFLAGS += -mfpmath=sse
    CFLAGS += $(foreach tag,$(SSE_TAGS),-m$(tag))
endif

BASEFILES = catena.c
#if specified use the gfmul tweak
ifdef GFMUL
	CFLAGS += -DGFMUL
	BASEFILES += gfhash.c
	#test for pclmulqdq
	PCMUL_TAG = $(shell /bin/grep -m 1 flags /proc/cpuinfo | /bin/grep -o \
		pclmulqdq)
	ifneq (${PCMUL_TAG},)
		CFLAGS += -mpclmul
	endif
endif


all: blake2b sha512

blake2b: catena-BRG-blake2b-test catena-BRG-blake2b-test_vectors catena-DBG-blake2b-test catena-DBG-blake2b-test_vectors 
sha512: catena-BRG-sha512-test catena-BRG-sha512-test_vectors catena-DBG-sha512-test catena-DBG-sha512-test_vectors
DBG: catena-DBG-blake2b-test catena-DBG-blake2b-test_vectors catena-DBG-sha512-test catena-DBG-sha512-test_vectors
BRG: catena-BRG-blake2b-test catena-BRG-blake2b-test_vectors catena-BRG-sha512-test catena-BRG-sha512-test_vectors
catena: catena-DBG catena-BRG

catena-DBG-blake2b-test:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ test-catena.c $(BASEFILES) catena-blake2b.c catena-DBG.c $(HASH)


catena-DBG-blake2b-test_vectors:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ catena_test_vectors.c $(BASEFILES) catena-blake2b.c catena-DBG.c $(HASH)


catena-DBG-sha512-test:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ test-catena.c $(BASEFILES) catena-sha512.c catena-DBG.c -lssl -lcrypto


catena-DBG-sha512-test_vectors:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ catena_test_vectors.c $(BASEFILES) catena-sha512.c catena-DBG.c -lssl -lcrypto

catena-DBG:
	$(CC) $(CFLAGS) -c $@.c $(HASH)



catena-BRG-blake2b-test:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ test-catena.c $(BASEFILES) catena-blake2b.c catena-BRG.c $(HASH)


catena-BRG-blake2b-test_vectors:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ catena_test_vectors.c $(BASEFILES) catena-blake2b.c catena-BRG.c $(HASH)


catena-BRG-sha512-test:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ test-catena.c $(BASEFILES) catena-sha512.c catena-BRG.c -lssl -lcrypto


catena-BRG-sha512-test_vectors:
	$(CC) $(CFLAGS) -o $(TARGETDIR)/$@ catena_test_vectors.c $(BASEFILES) catena-sha512.c catena-BRG.c -lssl -lcrypto

catena-BRG:
	$(CC) $(CFLAGS) -c $@.c $(HASH)


clean:
	rm -f  *~ *.o
