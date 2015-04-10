Catena
======
Catena is a memory-consuming password scrambler that excellently
thwarts massively parallel attacks on cheap memory-constrained
hardware, such as recent graphical processing units (GPUs).
Furthermore, Catena provides resistance against cache-timing attacks, since
its memory-access pattern is password-independent.

Academic paper:
<a href="http://www.uni-weimar.de/fileadmin/user/fak/medien/professuren/Mediensicherheit/Research/Publications/catena-v3.1.pdf">catena-v3.1.pdf</a>

Content
-------
* C reference implementation of Catena-blake2b

Instances
-------
* Catena-Dragonfly (formerly known as Catena-BRG)
* Catena-Butterfly (formerly known as Catena-DBG)

Options
-------
* When using Catena in a productive environment we recommend to activate 
  password overwriting. The password will then be erased from memory as soon as 
  possible and the corresponding memory will be deallocated. This requires the 
  password to lie in a writable part of the heap. Since this conflicts with the 
  official PHS interface, it is not available when this option is enabled. To 
  activate password overwriting append `SAFE=1` to the invocation of make:

        make all SAFE=1

* A tweak that replaces the reduced hash function H' with the full hash function,
  can be enabled by appending `FULLHASH=1` to the invocation of make:

        make all FULLHASH=1

Compatibility
-------------
Both instances of Catena should work fine with clang as well as gcc. In our 
tests clang turned out to be faster in every scenario. The biggest speedup, 
roughly 10%, occurred when using Catena-Butterfly with the default values.
We therefore choose clang as the default compiler for this implementation. 
Please let us know if you encounter any compiler-related bugs.
Endianess detection relies on `endian.h`. If you are working on a big-endian 
system without this header, key-generation and keyed-hashing won't work as 
expected. You can fix this by editing the endianess detection at the top of 
`catena.c`


Dependencies
------------
* clang   (http://clang.llvm.org/)
* make    (http://www.gnu.org/software/make/)