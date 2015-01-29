#include <fcntl.h>
#include <unistd.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif 

#include "catena-helpers.h"

const uint8_t ZERO8[H_LEN/sizeof(uint8_t)] = {0}; //H_LEN 0s


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
  uint16_t i;
  for(i = 0; i < H_LEN; i++){
    output[i] = input1[i] ^ input2[i];
  }
#endif
}


uint64_t jwndw(uint64_t* S, uint64_t j, uint8_t g){
	uint8_t w = (j*g)/64; //index of (first) word
	uint8_t start = j*g % 64; //as bit index
	uint8_t end = ((j+1) * g - 1) % 64;
	uint64_t result;
	/* garlic can't exceed 63 so start > end means we crossed a word boundary
	 * start = end can occur when garlic = 1
	 */
	if(start <= end){	//window is part of only one word
		result = S[w] << (63 - end);
		result >>= (63 - (g-1));
	}
	else{ //window crosses word boundaries into next word
		result = S[w+1] << (63-end);
		result >>= (63 - (g-1));
		result ^= S[w] >> start; //add remaining bits from lower word
	}
	return result;
}