#include <stdlib.h>

#include "catena.h"
#include "hash.h"

/*  Sigma function that defines the diagonal connections of a DBG
*	diagonal front: flip the (g-i)th bit. Inverse Buttferly Graph
*	diagonal back: flip the i-(g-1)th bit. Regular Butterfly Graph
*/
uint64_t sigma(const uint8_t g, const uint64_t i, const uint64_t j)
{
  if(i < g){
    return (j ^ (1 << (g-1-i)));
  }
  else{
    return (j ^ (1 << (i-(g-1)))); 
  }
}

/* Writes the XOR of the H_LEN long sequences input1 and input2  to output*/
void XOR(const uint8_t *input1, const uint8_t *input2, uint8_t *output)
{
  uint16_t i;
  for(i = 0; i < H_LEN; i++){
    output[i] = input1[i] ^ input2[i];
  }
}

/* Computes the hash of x using a Double Butterfly Graph,
* that forms as (2^g,\lamba)-Superconcentrator
*/
void F(const uint8_t x[H_LEN], const uint8_t lambda,
    const uint8_t garlic,   uint8_t h[H_LEN])
{
  const uint64_t c = UINT64_C(1) << garlic;
  const uint64_t l = 2 * garlic;

  uint8_t *v = malloc(c*H_LEN);
  uint8_t *r = malloc(c*H_LEN);

  uint64_t i, j;
  uint32_t k;

  //top row: v is the input level of the first iteration
  __Hash1(x, H_LEN, v);
  for(j = 1; j < c; j++){
    __Hash1(v + (j-1)*H_LEN, H_LEN, v + j*H_LEN);
  }

  //iterations
  for (k = 0; k < lambda; k++) {
    //levels
    for(i=1; i < l; i++){
      XOR(v, v + sigma(garlic,i-1,0) * H_LEN, r);
      __Hash2(v + (c-1)*H_LEN, H_LEN, r, H_LEN, r);

      for(j = 1; j < c; j++){
        XOR(v + j * H_LEN, v + sigma(garlic,i-1,j) * H_LEN, r + j * H_LEN);
        __Hash2(r + (j-1)*H_LEN, H_LEN, r + j * H_LEN, H_LEN, r + j * H_LEN);
      }

      memcpy(v, r, c * H_LEN); //v <-r
    }
  }

  memcpy(h, v + (c - 1) * H_LEN, H_LEN);
  free(r);
  free(v);
}
