Catena
======
Catena is a memory-consuming password scrambler that excellently
thwarts massively parallel attacks on cheap memory-constrained
hardware, such as recent graphical processing units (GPUs).
Furthermore, Catena is resistance against cache-timing attacks, since
its memory-access pattern is password-independent.

Academic paper:
http://eprint.iacr.org/2013/525

Content
-------
* C reference implementation of Catena-blake2b

Instances
-------
* Catena-Dragonfly (formerly known as Catena-BRG)
* Catena-Butterfly (formerly known as Catena-DBG)

Notes
-------
This implementation tries to purge the supplied password from memory as soon
as possible to prevent any leakage. This requires the memory address of
the password to be writable. While the implementation tries to determine if the
dress is writable before overwriting, it may fail to do so.
We advise all users of this implementation to store the password in writable 
memory!
Please note that the official PHS API requires the password to be qualified as
const.

Tweaks
-------
A tweak that replaces the reduced hash function H' with the full hash function, 
can be enabled by appending FULLHASH=1 to the make invocation. Like this:

    make all FULLHASH=1

Dependencies
------------
* gcc     (http://gcc.gnu.org/)
* openssl (http://www.openssl.org/)
* make    (http://www.gnu.org/software/make/)