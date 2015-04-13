#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef __SSE2__
#include <emmintrin.h>
#endif 

#include "hash.h"
#include "catena-helpers.h"

const uint8_t ZERO8[H_LEN] = {0}; //H_LEN 0s


inline void initmem(const uint8_t x[H_LEN], const uint64_t c, uint8_t *r)
{
  uint8_t *tmp = malloc(H_LEN);

  memcpy(tmp, x, H_LEN);
  tmp[H_LEN-1] ^= 1;
  __Hash2(x, H_LEN, tmp, H_LEN, r); //v_0 <- H(x||xXOR1)
  __ResetState();

  __HashFast(1, r, x, r+H_LEN); //v_1 <- H'(v_0||x)
  
  for(uint64_t i = 2; i < c; i++){
    __HashFast(i, r + (i-1)*H_LEN, r + (i-2)*H_LEN, r + i*H_LEN);
  }

  free(tmp);
}


inline void gamma(const uint8_t garlic, const uint8_t *salt, 
                  const uint8_t saltlen, uint8_t *r)
{
  const uint64_t q = UINT64_C(1) << ((3*garlic+3)/4);

  uint64_t i, j, j2;
  uint8_t *tmp = malloc(H_LEN);
  uint8_t *tmp2 = malloc(H_LEN);

  __Hash1(salt, saltlen, tmp);  //tmp <- H(S)
  __Hash1(tmp, H_LEN, tmp2);    //tmp2 <- H(H(S))
  initXSState(tmp, tmp2);
  
  __ResetState();
  for(i = 0; i < q; i++){
    j = xorshift1024star() >> (64 - garlic);
    j2 = xorshift1024star() >> (64 - garlic);
    //v_j1= H'(v_j1||v_j2)
    __HashFast(i, r + j * H_LEN, r + j2 * H_LEN, r + j * H_LEN); 
  }

  free(tmp);
  free(tmp2);
}


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
  //endianess independet equivalent to
  // memcpy(s, a, H_LEN);
  // memcpy(&s[8], b, H_LEN);
  //on little endian
  for(int i = 0; i < 8; i++){
    s[i] = UINT64_C(0);
    s[i+8] = UINT64_C(0);

    for(int j = 0; j < 8; j++){
      s[i] |= ((uint64_t)a[i*8+j]) << j*8;
      s[i+8] |= ((uint64_t)b[i*8+j]) << j*8;
    }
  }
}

uint64_t xorshift1024star() {
	uint64_t s0 = s[ p ];
	uint64_t s1 = s[ p = (p+1) & 15 ];
	s1 ^= s1 << 31; // a
	s1 ^= s1 >> 11; // b
	s0 ^= s0 >> 30; // c
	return ( s[p] = s0 ^ s1 ) * UINT64_C(1181783497276652981);
}


/* Our tests showed the memset to be always executed independent of 
* optimization. To ensure it stays that way, enable the no optimization flags.
*/
void 
#if defined(__clang__)
#if __has_attribute(optnone)
 __attribute__((optnone))
#endif //end has attribute

#elif defined(__GNUC__)

#define GCC_VERSION (__GNUC__ * 10000 \
                    + __GNUC_MINOR__ * 100 \
                    + __GNUC_PATCHLEVEL__)
//only supported by GCC >= 4.4
#if GCC_VERSION >= 40400
  __attribute__((optimize("O0")))
#endif //end GCC >= 4.4

#endif //end compiler_id

erasepwd(uint8_t* pwd, const uint32_t pwdlen)
{
  memset((char *)pwd, 0, pwdlen);
  free(pwd);
}