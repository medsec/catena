
all: blake2b

blake2b:
	cd src; make $@; cd 

clean:
	cd src;	make clean; cd ..
	rm -f *~ catena-Dragonfly-blake2b-test
	rm -f catena-Butterfly-blake2b-test
	rm -f catena-Dragonfly-blake2b-test_vectors
	rm -f catena-Butterfly-blake2b-test_vectors
