#ifndef _CATENA_HASH_H__
#define _CATENA_HASH_H__

#include "catena.h"

inline void __Hash1(const uint8_t *input, const uint32_t inputlen,
		    uint8_t hash[H_LEN]);


inline void __Hash2(const uint8_t *i1, const uint8_t i1len,
		    const uint8_t *i2, const uint8_t i2len,
		    uint8_t hash[H_LEN]);



inline void __Hash3(const uint8_t *i1, const uint8_t i1len,
		    const uint8_t *i2, const uint8_t i2len,
		    const uint8_t *i3, const uint8_t i3len,
		    uint8_t hash[H_LEN]);


inline void __Hash4(const uint8_t *i1, const uint8_t i1len,
		    const uint8_t *i2, const uint8_t i2len,
		    const uint8_t *i3, const uint8_t i3len,
		    const uint8_t *i4, const uint8_t i4len,
		    uint8_t hash[H_LEN]);


inline void __Hash5(const uint8_t *i1, const uint8_t i1len,
		    const uint8_t *i2, const uint8_t i2len,
		    const uint8_t *i3, const uint8_t i3len,
		    const uint8_t *i4, const uint8_t i4len,
		    const uint8_t *i5, const uint8_t i5len,
		    uint8_t hash[H_LEN]);

/* If FAST is defined then this is a call to a reduced version of the 
*  underlying hash function. Else this will be a call to __Hash2.
*  The 2 Inputs are expected to be of size H_LEN
*/
void __HashFast(int vindex, const uint8_t* i1, 
    		const uint8_t* i2, uint8_t hash[H_LEN]);

/* Resets the State used for __HashFast if necessary 
*/
void __ResetState();

#endif
