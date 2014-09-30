#include <stdlib.h>

#include "catena.h"
#include "hash.h"

/*  Sigma function that defines the diagonal connections of a DBG
*	diagonal front: flip the (g-i)th bit (Inverse Buttferly Graph)
*	diagonal back: flip the i-(g-1)th bit (Regular Butterfly Graph)
*/
uint64_t sigma(const uint8_t g, const uint64_t i, const uint64_t j)
{
  if(i < g){
    return (j ^ (1 << (g-1-i))); //diagonal front
  }
  else{
    return (j ^ (1 << (i-(g-1)))); //diagonal back
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
uint64_t idx(uint16_t i, uint64_t j, uint8_t co, uint64_t c, uint32_t m){
  i += co;
  if(i % 3 == 0){
    return j;
  }
  else if(i % 3 == 1){
    if(j < m){ //still fits in the array
      return j + c;
    }
    else{ //start overwriting elements at the beginning
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
  uint8_t co = 0; //carry over from last iteration


  //top row
  __Hash1(x, H_LEN, r);
  for(j = 1; j < c; j++){
    __Hash1(r + (j-1)*H_LEN, H_LEN, r + j*H_LEN);
  }

  //iterations
  for (k = 0; k < lambda; k++) {
    //rows
    for(i=1; i < l; i++){
      //tmp:= v2^g-1 XOR v0
      XOR(r + idx(i-1,0,co,c,m) * H_LEN, r + idx(i-1,c-1,co,c,m)*H_LEN, tmp);
      //r0 := H(tmp || vsigma(g,i-1,0) )
      __Hash2(tmp, H_LEN, r + idx(i-1,sigma(garlic,i-1,0),co,c,m) * H_LEN,
         H_LEN, r + idx(i,0,co,c,m) *H_LEN);
      //vertices
      for(j = 1; j < c; j++){
        //tmp:= ri-1 XOR vi
        XOR(r + idx(i-1,j,co,c,m) * H_LEN, r + idx(i,j-1,co,c,m)*H_LEN, tmp);
        //ri := H(tmp || vsigma(g,i-1,j))
        __Hash2(tmp, H_LEN, r + idx(i-1,sigma(garlic,i-1,j),co,c,m) * H_LEN, 
          H_LEN, r + idx(i,j,co,c,m) * H_LEN);
      }
    }
    co = (co + (i-1)) % 3;
  }
  memcpy(h, r + idx(0,c-1,co,c,m) * H_LEN, H_LEN);
  free(r);
}