#include <stdlib.h>
#include <stdio.h>

#include "catena.h"
#include "hash.h"
#include "catena-helpers.h"

// Default values for Catena-Butterfly
const uint8_t VERSION_ID[] = "Butterfly";
const uint8_t LAMBDA = 4;
const uint8_t GARLIC = 16;
const uint8_t MIN_GARLIC = 16;

/*  Sigma function that defines the diagonal connections of a DBG
*	diagonal front: flip the (g-i)th bit (Inverse Buttferly Graph)
*	diagonal back: flip the i-(g-1)th bit (Regular Butterfly Graph)
*/
uint64_t sigma(const uint8_t g, const uint64_t i, const uint64_t j)
{
  if(i < g){
    return (j ^ (UINT64_C(1) << (g-1-i))); //diagonal front
  }
  else{
    return (j ^ (UINT64_C(1) << (i-(g-1)))); //diagonal back
  }
}


/*calculate actual index from level and element index*/
uint64_t idx(uint64_t i, uint64_t j, uint8_t co, uint64_t c, uint64_t m){
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
void Flap(const uint8_t x[H_LEN], const uint8_t lambda, const uint8_t garlic,
  const uint8_t *salt, const uint8_t saltlen, uint8_t h[H_LEN])
{
  const uint64_t c = UINT64_C(1) << garlic;
  const uint64_t m = UINT64_C(1) << (garlic-1); //0.5 * 2^g
  const uint32_t l = 2 * garlic;

  uint8_t *r   = malloc((c+m)*H_LEN);
  uint8_t *tmp = malloc(H_LEN);
  uint64_t i,j;
  uint8_t k;
  uint8_t co = 0; //carry over from last iteration


  /* Top row */
  initmem(x, c, r);

  /*Gamma Function*/
  gamma(garlic, salt, saltlen, r);

  /* DBH */
  for (k = 0; k < lambda; k++) {
    //rows
    for(i=1; i < l; i++){
      //tmp:= v2^g-1 XOR v0
      XOR(r + idx(i-1,c-1,co,c,m)*H_LEN, r + idx(i-1,0,co,c,m)*H_LEN, tmp);

      //r0 := H(tmp || vsigma(g,i-1,0) )
      __Hash2(tmp, H_LEN, r+idx(i-1,sigma(garlic,i-1,0),co,c,m) * H_LEN, H_LEN,
	       r+idx(i,0,co,c,m) *H_LEN);
      __ResetState();

      //vertices
      for(j = 1; j < c; j++){
        //tmp:= rj-1 XOR vj
        XOR(r + idx(i,j-1,co,c,m)*H_LEN, r + idx(i-1,j,co,c,m) * H_LEN, tmp);
        //rj := H(tmp || vsigma(g,i-1,j))
        __HashFast(j, tmp, r + idx(i-1,sigma(garlic,i-1,j),co,c,m) * H_LEN,
		        r + idx(i,j,co,c,m) * H_LEN);
      }
    }
    co = (co + (i-1)) % 3;
  }
  memcpy(h, r + idx(0,c-1,co,c,m) * H_LEN, H_LEN);
  free(r);
  free(tmp);
}
