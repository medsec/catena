#include <stdlib.h>
#include <byteswap.h>

#include "catena.h"
#include "hash.h" 

/* Return the reverse bit order of x where x is interpreted as n-bit value */
uint64_t reverse(uint64_t x, const uint8_t n)
{
  x = bswap_64(x);
  x = ((x & UINT64_C(0x0f0f0f0f0f0f0f0f)) << 4) |
    ((x & UINT64_C(0xf0f0f0f0f0f0f0f0)) >> 4);
  x = ((x & UINT64_C(0x3333333333333333)) << 2) |
    ((x & UINT64_C(0xcccccccccccccccc)) >> 2);
  x = ((x & UINT64_C(0x5555555555555555)) << 1) |
    ((x & UINT64_C(0xaaaaaaaaaaaaaaaa)) >> 1);
  return x >> (64 - n);
}

/* This function computes the  \lambda-BRH of x. */
void F(const uint8_t x[H_LEN], const uint8_t lambda,
	  const uint8_t garlic,   uint8_t h[H_LEN])
{
  const uint64_t c = UINT64_C(1) << garlic;
  uint8_t *r = malloc(c*H_LEN);
  uint64_t i = 0;
  uint32_t k;

  __Hash1(x, H_LEN, r);

  /* Top row */
  for (i = 1; i < c; i++) {
    __Hash1(r + (i-1)*H_LEN, H_LEN, r + i*H_LEN);
  }

  /* Mid rows */
  for (k = 0; k < lambda; k++) {
    __Hash2(r + (c-1)*H_LEN, H_LEN, r, H_LEN, r);

    /* Replace r[reverse(i, garlic)] with new value */
    uint8_t *previousR = r, *p;
    for (i = 1; i < c; i++) {
      p = r + reverse(i, garlic) * H_LEN;
      __Hash2(previousR, H_LEN, p, H_LEN, p);
      previousR = p;
    }
    k++;
    if (k >= lambda) {
      break;
    }
    /* This is now sequential because (reverse(reverse(i, garlic), garlic) == i) */
    __Hash2(r + (c-1)*H_LEN, H_LEN, r, H_LEN, r);
    p = r + H_LEN;
    for (i = 1; i < c; i++, p += H_LEN) {
      __Hash1(p - H_LEN, 2 * H_LEN, p);
    }
  }

  /* reverse(c - 1, garlic) == c - 1 */
  memcpy(h, r + (c - 1) * H_LEN, H_LEN);
  free(r);
}