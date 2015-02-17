#include <stdlib.h>
#include <byteswap.h>

#include "catena.h"
#include "hash.h" 
#include "catena-helpers.h"

// Default values for Catena-Dragonfly
const uint8_t VERSION_ID[] = "Dragonfly";
const uint8_t LAMBDA = 2;
const uint8_t GARLIC = 21;
const uint8_t MIN_GARLIC = 21;

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
void Flap(const uint8_t x[H_LEN], const uint8_t lambda, const uint8_t garlic,
  const uint8_t *salt, const uint8_t saltlen, uint8_t h[H_LEN])
{
  const uint64_t c = UINT64_C(1) << garlic;
  uint8_t *r = malloc(c*H_LEN);
  uint64_t i;
  uint8_t k;

  /* Top row */
  initmem(x, c, r);

  /*Gamma Function*/
  gamma(garlic, salt, saltlen, r);

  /* BRH */
  for (k = 0; k < lambda; k++) {
    __Hash2(r + (c-1)*H_LEN, H_LEN, r, H_LEN, r);
    __ResetState();

    /* Replace r[reverse(i, garlic)] with new value */
    uint8_t *previousR = r, *p;
    for (i = 1; i < c; i++) {
      p = r + reverse(i, garlic) * H_LEN;
      __HashFast(i, previousR, p, p);
      previousR = p;
    }
    k++;
    if (k >= lambda) {
      break;
    }
    /* This is now sequential because (reverse(reverse(i, garlic), garlic) == i) */
    __Hash2(r + (c-1)*H_LEN, H_LEN, r, H_LEN, r);
    __ResetState();
    p = r + H_LEN;
    for (i = 1; i < c; i++, p += H_LEN) {
      __HashFast(i, p - H_LEN, p, p);
    }
  }

  /* reverse(c - 1, garlic) == c - 1 */
  memcpy(h, r + (c - 1) * H_LEN, H_LEN);
  free(r);
}
