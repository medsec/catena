#include <string.h>
#include <stdio.h>
#include <byteswap.h>
#include <stdlib.h>
#include <sys/param.h>
#define __STDC_CONSTANT_MACROS
#include <stdint.h>

#include "catena.h"

#ifdef ARC_BIG_ENDIAN
  #define TO_LITTLE_ENDIAN_64(n) bswap_64(n)
  #define TO_LITTLE_ENDIAN_32(n) bswap_32(n)
#else
  #define TO_LITTLE_ENDIAN_64(n) (n)
  #define TO_LITTLE_ENDIAN_32(n) (n)
#endif

uint64_t reverse(uint64_t x, const uint8_t n)
{
  unsigned int i;
  uint64_t r = 0;
  for(i=0;i<n;i++)
    {
      r<<=1;
      r|=x&1;
      x>>=1;
    }
  return r;
}

/***************************************************/

inline void __Hash1(const uint8_t *input, const uint8_t inputlen,
		      uint8_t hash[H_LEN])
{
  blake2b_state ctx;
  blake2b_init(&ctx,H_LEN);
  blake2b_update(&ctx, input, inputlen);
  blake2b_final(&ctx, hash, H_LEN);
}

/***************************************************/

inline void __Hash3(const uint8_t *i1, const uint8_t i1len,
		    const uint8_t *i2, const uint8_t i2len,
		    const uint8_t *i3, const uint8_t i3len,
		    uint8_t hash[H_LEN])
{
  blake2b_state ctx;
  blake2b_init(&ctx,H_LEN);
  blake2b_update(&ctx, i1, i1len);
  blake2b_update(&ctx, i2, i2len);
  blake2b_update(&ctx, i3, i3len);
  blake2b_final(&ctx, hash, H_LEN);
}
/***************************************************/

inline void __Hash4(const uint8_t *i1, const uint8_t i1len,
		    const uint8_t *i2, const uint8_t i2len,
		    const uint8_t *i3, const uint8_t i3len,
		     const uint8_t *i4, const uint8_t i4len,
		    uint8_t hash[H_LEN])
{
  blake2b_state ctx;
  blake2b_init(&ctx,H_LEN);
  blake2b_update(&ctx, i1, i1len);
  blake2b_update(&ctx, i2, i2len);
  blake2b_update(&ctx, i3, i3len);
  blake2b_update(&ctx, i4, i4len);
  blake2b_final(&ctx, hash, H_LEN);
}


/***************************************************/

inline void __Hash5(const uint8_t *i1, const uint8_t i1len,
		    const uint8_t *i2, const uint8_t i2len,
		    const uint8_t *i3, const uint8_t i3len,
		    const uint8_t *i4, const uint8_t i4len,
		    const uint8_t *i5, const uint8_t i5len,
		    uint8_t hash[H_LEN])
{
  blake2b_state ctx;
  blake2b_init(&ctx,H_LEN);
  blake2b_update(&ctx, i1, i1len);
  blake2b_update(&ctx, i2, i2len);
  blake2b_update(&ctx, i3, i3len);
  blake2b_update(&ctx, i4, i4len);
  blake2b_update(&ctx, i5, i5len);
  blake2b_final(&ctx, hash, H_LEN);
}

/***************************************************/


void LBRH(const uint8_t x[H_LEN], const uint8_t garlic, uint8_t h[H_LEN])
{
  const uint64_t c = UINT64_C(1) << garlic;
  uint8_t *v = malloc(c*H_LEN);
  uint8_t *r = malloc(c*H_LEN);
  uint64_t i=0;
  uint32_t k;

  __Hash3(&garlic,1, (uint8_t *) &i, 8, x, H_LEN, v);

  /* Top row */
  for(i=1 ; i<c ; i++) {
    uint64_t tmp = TO_LITTLE_ENDIAN_64(i);
    __Hash3(&garlic,1, (uint8_t *) &tmp, 8, &v[(i-1)*H_LEN],H_LEN,&v[i*H_LEN]);
  }

  /* Mid rows */
  for(k=0; k< LAMBDA; k++) {
    uint64_t tmp = TO_LITTLE_ENDIAN_64(c);
    __Hash4(&garlic,1, (uint8_t *) &tmp, 8, v, H_LEN, &v[(c-1)*H_LEN], H_LEN, r);
    for(i=1 ; i<c ; i++) {
      tmp = TO_LITTLE_ENDIAN_64(i);
      __Hash4(&garlic,1, (uint8_t *) &tmp, 8, &r[(i-1)*H_LEN], H_LEN,
	      &v[reverse(i,garlic)*H_LEN] , H_LEN, &r[i*H_LEN]);
    }
    memcpy(v,r,c*H_LEN);
  }
  memcpy(h, r + (c - 1) * H_LEN, H_LEN);
  free(v);
  free(r);
}



/***************************************************/


int __Catena(const uint8_t *pwd,   const uint32_t pwdlen,
	     const uint8_t *salt,  const uint8_t saltlen,
	     const uint8_t *data,  const uint32_t datalen,
	     const uint8_t garlic, const uint8_t hashlen,
	     const uint8_t client, const uint8_t tweak_id, uint8_t *hash)
{
 uint8_t x[H_LEN];
 uint8_t t[5];
 uint64_t invokation_counter;
 uint8_t c;

  if ((hashlen > H_LEN) || (garlic > 63)) return -1;

  /* Compute Tweak */
  t[0] = 0xFF;
  t[1] = tweak_id;
  t[2] = LAMBDA;
  t[3] = hashlen;
  t[4] = saltlen;

  /* Compute H(AD) */
  __Hash1((uint8_t *) data, datalen,x);

  /* Compute the initial value to hash  */
  __Hash4(t,5, x, H_LEN, (uint8_t *) pwd,  pwdlen, salt, saltlen, x);

  memset(x+hashlen, 0, H_LEN-hashlen);

  for(c=1;c <= garlic; c++)
    {
      LBRH(x, c, x);
      if( (c==garlic) && (client == CLIENT))
	{
	  memcpy(hash, x, H_LEN);
	  return 0;
	}

      invokation_counter = TO_LITTLE_ENDIAN_64(((uint64_t) LAMBDA+1) << c);
      __Hash3( (uint8_t *) &c,1, (uint8_t *) &invokation_counter,8, x,H_LEN, x);
      memset(x+hashlen, 0, H_LEN-hashlen);
    }
  memcpy(hash, x, hashlen);

  return 0;
}


/***************************************************/

int Catena(const uint8_t  *pwd,   const uint32_t pwdlen,
	   const uint8_t *salt,  const uint8_t saltlen,
	   const uint8_t *data,  const uint32_t datalen,
	   const uint8_t garlic, const uint8_t hashlen, uint8_t *hash)
{
  return __Catena(pwd, pwdlen, salt, saltlen, data, datalen, garlic, hashlen,
		  REGULAR, PASSWORD_HASHING_MODE, hash);
}


/***************************************************/


int Simple_Catena(const char *pwd,  const char *salt, const char *data,
		  const uint8_t garlic, uint8_t hash[H_LEN]){

  return __Catena( (uint8_t  *) pwd, strlen(pwd),
		   (uint8_t  *) salt, strlen(salt),
		   (uint8_t  *) data, strlen(data),
		   garlic, H_LEN, REGULAR, PASSWORD_HASHING_MODE, hash);
}

/***************************************************/

int Catena_Client(const uint8_t  *pwd,   const uint32_t pwdlen,
		  const uint8_t  *salt,  const uint8_t  saltlen,
		  const uint8_t  *data,  const uint32_t datalen,
		  const uint8_t  garlic, const uint8_t  hashlen,
		  uint8_t x[H_LEN])
{
  return __Catena(pwd, pwdlen, (uint8_t *) salt, saltlen, data, datalen,
		  garlic, hashlen, CLIENT, PASSWORD_HASHING_MODE, x);
}

/***************************************************/

int Catena_Server(const uint8_t garlic, const uint8_t x[H_LEN],
		  const uint8_t hashlen, uint8_t *hash)
{
  const uint64_t invokation_counter = TO_LITTLE_ENDIAN_64(((uint64_t) LAMBDA+1) << garlic);
  uint8_t z[H_LEN];

  if (hashlen > H_LEN) return -1;

  __Hash3((uint8_t *) &garlic,1,(uint8_t *) &invokation_counter, 8, x, H_LEN, z);
  memcpy(hash, z, hashlen);

  return 0;
}

/***************************************************/

void CI_Update(const uint8_t *old_hash, const uint8_t old_garlic,
	       const uint8_t new_garlic, const uint8_t hashlen,
	       uint8_t *new_hash)
{
  uint8_t c;
  uint8_t x[H_LEN];
  uint64_t invokation_counter;

  memcpy(x, old_hash, hashlen);
  memset(x+hashlen, 0, H_LEN-hashlen);

  for(c=old_garlic+1; c <= new_garlic; c++)
    {
      LBRH(x, c, x);
      invokation_counter = TO_LITTLE_ENDIAN_64(((uint64_t) LAMBDA+1) << c);
      __Hash3(&c, 1, (uint8_t *) &invokation_counter,8,  x, H_LEN, x);
      memset(x+hashlen, 0, H_LEN-hashlen);
    }
  memcpy(new_hash,x,hashlen);
}


/***************************************************/

void Catena_KG(const uint8_t *pwd,   const uint32_t pwdlen,
	       const uint8_t *salt,  const uint8_t saltlen,
	       const uint8_t *data,  const uint32_t datalen,
	       const uint8_t garlic, uint32_t keylen,
	       const uint8_t key_id, uint8_t *key)
{
  uint8_t hash[H_LEN];
  const uint8_t zero = 0;
  const uint32_t len = keylen/H_LEN;
  const uint32_t rest = keylen%H_LEN;
  uint64_t i;
  keylen = TO_LITTLE_ENDIAN_32(keylen);

  __Catena(pwd, pwdlen, salt, saltlen, data, datalen,
	   garlic, H_LEN, REGULAR, KEY_DERIVATION_MODE, hash);

  for(i=0; i < len; i++) {
    uint64_t tmp = TO_LITTLE_ENDIAN_64(i);
    __Hash5(&zero, 1, (uint8_t *) &tmp, 8, &key_id, 1,(uint8_t *) &keylen,4,
	      hash, H_LEN, &key[i*H_LEN]);
  }

  if(rest)
    {
      uint64_t tmp = TO_LITTLE_ENDIAN_64(i);
      __Hash5(&zero, 1, (uint8_t *) &tmp, 8, &key_id, 1,(uint8_t *) &keylen,4,
		hash, H_LEN, hash);
      memcpy(&key[len*H_LEN], hash,rest);
    }
}


/***************************************************/
#pragma GCC diagnostic ignored "-Wunused-parameter"
int PHS(void *out, size_t outlen,  const void *in, size_t inlen,
	const void *salt, size_t saltlen, unsigned int t_cost,
	unsigned int m_cost) {

  return __Catena((const uint8_t *) in, inlen, salt, saltlen, (const uint8_t *)
		  "", 0, m_cost, outlen, REGULAR, PASSWORD_HASHING_MODE, out);
}

#pragma GCC diagnostic warning "-Wunused-parameter"
