#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>

#include "catena.h"
#include "hash.h"

/***************************************************/

/* This only works for hash = input + H_LEN, or hash = input + 2*H_LEN. */
inline void __FastHash1(const uint8_t *input, const uint32_t inputlen,
		      uint8_t hash[H_LEN])
{
  if(hash != input + H_LEN && hash != input + 2*H_LEN) {
      fprintf(stderr, "Expected hash to follow input\n");
      exit(1);
  }
  __FastHash2(input, inputlen, input + (inputlen >> 1), inputlen, hash);
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
  (void)i1len; /* Ignore i1len and i2len */
  (void)i2len;
  uint32_t value = prevBlock[H_LEN-1];
  uint32_t i;
  for(i = 0; i < H_LEN/sizeof(uint32_t); i++) {
    value = value*(*prevBlock++ | 3) + *fromBlock++;
    *toBlock++ = value;
  }
}



/***************************************************/
