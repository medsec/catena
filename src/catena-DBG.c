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

/*calculate actual index from level and element index*/
uint64_t ind(uint16_t i, uint64_t j, uint64_t c, uint32_t m){
  if(i % 3 == 0){
    return j;
  }
  else if(i % 3 == 1){
    if(j < m){
      return j + c;
    }
    else{
      return j - m;
    }
  }
  else{ //i % 3 == 2
    return j + m;
  }

}

/* Computes the hash of x using a Double Butterfly Graph,
* that forms as (2^g,\lamba)-Superconcentrator
*/
void F(const uint8_t x[H_LEN], const uint8_t lambda,
    const uint8_t garlic,   uint8_t h[H_LEN])
{
  const uint64_t c = UINT64_C(1) << garlic;
  const uint32_t m = UINT32_C(1) << (garlic-1);
  const uint16_t l = 2 * garlic;

  uint8_t *r = malloc((c+m)*H_LEN);
  uint8_t *tmp = malloc(H_LEN);

  uint16_t i;
  uint64_t j;
  uint8_t k;

  //top row
  __Hash1(x, H_LEN, r);
  for(j = 1; j < c; j++){
    __Hash1(r + (j-1)*H_LEN, H_LEN, r + j*H_LEN);
  }

  //iterations
  for (k = 0; k < lambda; k++) {
    //levels
    for(i=1; i < l; i++){
      XOR(r + ind(i-1, 0, c,m) * H_LEN, 
        r + ind(i-1,c-1,c,m)*H_LEN, tmp);
      __Hash2(r + ind(i-1,sigma(garlic,i-1,0),c,m) * H_LEN, H_LEN, tmp,
         H_LEN, r + ind(i,0,c,m) *H_LEN);
      //elements
      for(j = 1; j < c; j++){
        XOR(r + ind(i-1,j,c,m) * H_LEN, r + ind(i,j-1,c,m)*H_LEN, tmp);
        __Hash2(r + ind(i-1,sigma(garlic,i-1,j),c,m) * H_LEN, H_LEN, tmp, 
          H_LEN, r + ind(i,j,c,m) * H_LEN);
      }
    }
    memcpy(r,r +ind(i-1,0,c,m) * H_LEN, c * H_LEN);
  }

  memcpy(h, r + ind(l-1,c-1,c,m) * H_LEN, H_LEN);
  free(r);
}
