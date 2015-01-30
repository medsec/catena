#ifndef _CATENA_HELPERS_H_
#define _CATENA_HELPERS_H_

#include "catena.h" 

extern const uint8_t ZERO8[H_LEN/sizeof(uint8_t)]; //H_LEN 0s

union v8_v64
{
	uint8_t v8[H_LEN];
	uint64_t v64[H_LEN/8];
};

/* Writes the XOR of the H_LEN long sequences input1 and input2  to output
* Uses Intrinsic to speed up computation when SSE2 is avaiable
*/
void XOR(const uint8_t *input1, const uint8_t *input2, uint8_t *output);

/* Init the state for Xorshift with two 512bit values
 */
void initXSState(const uint8_t* a, const uint8_t* b);

/*  Xorshift RNG with a 1024bit state that must be seeded first.
 */
uint64_t xorshift1024star();

#endif