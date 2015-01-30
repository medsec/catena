#include <fcntl.h>
#include <unistd.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif 

#include "catena-helpers.h"

const uint8_t ZERO8[H_LEN] = {0}; //H_LEN 0s


void XOR(const uint8_t *input1, const uint8_t *input2, uint8_t *output)
{
#ifdef __SSE2__
  int blocks = H_LEN/sizeof(__m128i);
  for(int i = 0; i < blocks; i++){
    __m128i left = _mm_load_si128((__m128i*) (input1 + (i*sizeof(__m128i))));
    __m128i right = _mm_load_si128((__m128i*) (input2 + (i*sizeof(__m128i))));
    __m128i result = _mm_xor_si128 (left,right);
    _mm_store_si128((__m128i*) (output + (i*sizeof(__m128i))), result);
  }
#else
  uint32_t i;
  for(i = 0; i < H_LEN; i++){
    output[i] = input1[i] ^ input2[i];
  }
#endif
}


//see: http://en.wikipedia.org/wiki/Xorshift#Variations
static int p;
static uint64_t s[16];

void initXSState(const uint8_t* a, const uint8_t* b){
	p = 0;
	memcpy(s, a, H_LEN);
	memcpy(&s[8], b, H_LEN);
}

uint64_t xorshift1024star() {
	uint64_t s0 = s[ p ];
	uint64_t s1 = s[ p = (p+1) & 15 ];
	s1 ^= s1 << 31; // a
	s1 ^= s1 >> 11; // b
	s0 ^= s0 >> 30; // c
	return ( s[p] = s0 ^ s1 ) * UINT64_C(1181783497276652981);
}