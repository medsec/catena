
all: blake2b sha512

blake2b:
	cd src; make $@; cd 
	cp bin/catena-BRG-$@-test .
	cp bin/catena-BRG-$@-test_vectors .
	cp bin/catena-DBG-$@-test .
	cp bin/catena-DBG-$@-test_vectors .


sha512:
	cd src; make $@; cd ..
	cp bin/catena-BRG-$@-test .
	cp bin/catena-BRG-$@-test_vectors .
	cp bin/catena-DBG-$@-test .
	cp bin/catena-DBG-$@-test_vectors .


clean:
	cd src;	make clean; cd ..
	rm -f *~ catena-BRG-blake2b-test catena-BRG-sha512-test 
	rm -f catena-DBG-blake2b-test catena-DBG-sha512-test 
	rm -f catena-BRG-blake2b-test_vectors catena-BRG-sha512-test_vectors 
	rm -f catena-DBG-blake2b-test_vectors catena-DBG-sha512-test_vectors 