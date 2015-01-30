#include <stdlib.h>
#include <byteswap.h>

#include "catena.h"
#include "hash.h" 
#include "catena-helpers.h"

// Default values for Catena-Dragonfly
const uint8_t VERSION_ID[] = "Dragonfly";
const uint8_t LAMBDA = 3;
const uint8_t GARLIC = 14;
const uint8_t MIN_GARLIC = 14;

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
void F(const uint8_t x[H_LEN], const uint8_t lambda, const uint8_t garlic,
  const uint8_t *salt, const uint8_t saltlen, uint8_t h[H_LEN])
{
  const uint64_t c = UINT64_C(1) << garlic;
  uint8_t *r = malloc(c*H_LEN);
  uint8_t *tmp = malloc(H_LEN);
  union v8_v64 s;
  uint64_t i = 0, j = 0;
  uint8_t k;

  __Hash1(x, H_LEN, r);

  /* Top row */
  __Hash2(x, H_LEN, ZERO8, H_LEN, r); //v_0 <- H(x||0)
  __ResetState();
  __HashFast(1, r, x, r+H_LEN); //v_1 <- H'(v_0||x)
  for(i = 2; i < c; i++){
    __HashFast(i, r + (i-1)*H_LEN, r + (i-2)*H_LEN, r + i*H_LEN);
  }

  /*Gamma Function*/
  __Hash1(salt, saltlen, s.v8);
  XOR(r + (c-1)*H_LEN, r, tmp); //tmp = v_(2^g-1) XOR v_0
  //v_0 = H(tmp||v_(S[0]))
  __Hash2(tmp, H_LEN, r + jwndw(s.v64,0,garlic) * H_LEN, H_LEN, r);
  __ResetState();
  for(i = 1; i < c; i++){
    j = i % ((H_LEN*8)/garlic);
    if(j == 0){
      __Hash1(s.v8, H_LEN, s.v8);
    }
    XOR(r + (i-1)*H_LEN, r + i*H_LEN, tmp); //tmp = v_(i-1) XOR v_i
    __HashFast(i, tmp, r + jwndw(s.v64,j,garlic) * H_LEN, r); //v_i= H'(tmp||v_(S[j]))
  }

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
