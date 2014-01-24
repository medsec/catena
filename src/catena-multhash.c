# include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>

#include "catena.h"
#include "hash.h"

/***************************************************/

// Simple rand() function.
// from: http://www.codeproject.com/Articles/25172/Simple-Random-Number-Generation
static inline uint32_t randNum(uint32_t *z, uint32_t *w) {
    *z = 36969 * (*z & 65535) + (*z >> 16);
    *w = 18000 * (*w & 65535) + (*w >> 16);
    return (*z << 16) + *w;
}

/* This only works for hash = input + H_LEN, or hash = input + 2*H_LEN. */
inline void __FastHash1(const uint8_t *input, const uint32_t inputlen,
		      uint8_t hash[H_LEN])
{
  if(hash != input + H_LEN) {
    fprintf(stderr, "Expected hash to follow input\n");
    exit(1);
  }
  if(inputlen == 2*H_LEN) {
    // Must be even rows, not the first
    __FastHash2(input, H_LEN, input + H_LEN, H_LEN, hash);
  } else {
    // Must be the first row
    if(inputlen != H_LEN) {
      fprintf(stderr, "Expected H_LEN input length\n");
      exit(1);
    }
    const uint32_t *prevVal = (const uint32_t *)(const void *)input;
    const uint32_t *startVal = prevVal;
    const uint32_t *fromVal;
    uint32_t *toVal = (uint32_t *)(void *)hash;
    static uint32_t prevValue = 0;
    uint32_t value = prevVal[H_LEN/sizeof(uint32_t)-1];
    if(prevValue != 0 && value != prevValue) {
      printf("Bad previous value\n");
      exit(1);
    }
    uint32_t z = 1, w = 1;
    uint32_t i;
    for(i = 0; i < (H_LEN/sizeof(uint32_t))/8; i++) {
      fromVal = startVal + (randNum(&z, &w) & ((H_LEN/sizeof(uint32_t))-1));
      uint32_t j;
      for(j = 0; j < 8; j++) {
          value = value*(*prevVal++ | 3) + *fromVal++;
          *toVal++ = value;
      }
    }
  }
}

/***************************************************/

/* Note that endian-ness is ignored here.  It needs to be dealt with in catena.c.
   Also note that i1len and i2len are ignored, and must be H_LEN. */
inline void __FastHash2(const uint8_t *i1, const uint32_t i1len,
		    const uint8_t *i2, const uint32_t i2len,
		    uint8_t hash[H_LEN])
{
  /* Cast to uint32 */
  const uint32_t *prevBlock = (const uint32_t *)(const void *)i1;
  const uint32_t *fromBlock = (const uint32_t *)(const void *)i2;
  uint32_t *toBlock = (uint32_t *)(void *)hash;
  if(i1len != i2len) {
    printf("Expected ilen1 == ilen2\n");
    exit(1);
  }
  static uint32_t prevValue = 0;
  uint32_t value = prevBlock[i1len/sizeof(uint32_t)-1];
  if(prevValue != 0 && value != prevValue) {
    printf("Bad previous value\n");
    exit(1);
  }
  uint32_t i;
  for(i = 0; i < i1len/sizeof(uint32_t); i++) {
    value = value*(*prevBlock++ | 3) + *fromBlock++;
    *toBlock++ = value;
  }
  prevValue = value;
}



/***************************************************/
