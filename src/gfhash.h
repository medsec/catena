#ifndef _GFHASH_H_
#define _GFHASH_H_

#include <stdint.h>
#include "hash.h"

#ifdef __PCLMUL__
#include <emmintrin.h>
#include <wmmintrin.h>
#include <smmintrin.h>
#endif //__PCLMUL__

/* Takes two H_LEN Inputs and multiplies them in 128 bit blocks in a GF2^128
 */
void gfhash(uint8_t * source1, uint8_t * source2, uint8_t* target);

#endif //_GFHASH_H_