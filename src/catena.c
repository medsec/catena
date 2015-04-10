#include <string.h>
#include <stdio.h>
#include <byteswap.h>
#include <stdlib.h>
#include <sys/param.h>
#define __STDC_CONSTANT_MACROS
#include <stdint.h>

#include "catena.h"
#include "catena-helpers.h"
#include "hash.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN
  #define TO_LITTLE_ENDIAN_64(n) (n)
  #define TO_LITTLE_ENDIAN_32(n) (n)
#elif __BYTE_ORDER == __BIG_ENDIAN
  #define TO_LITTLE_ENDIAN_64(n) bswap_64(n)
  #define TO_LITTLE_ENDIAN_32(n) bswap_32(n)
#else
  #warning "byte order couldn't be detected. This affects key generation and keyed hashing"
  #define TO_LITTLE_ENDIAN_64(n) (n)
  #define TO_LITTLE_ENDIAN_32(n) (n)
#endif

/* Ensure that a pointer passed to the PHS interface stays const
 */
#ifdef OVERWRITE
  #define MAYBECONST 
#else
  #define MAYBECONST const
#endif

/***************************************************/

int __Catena(MAYBECONST uint8_t *pwd,   const uint32_t pwdlen,
	     const uint8_t *salt,  const uint8_t  saltlen,
	     const uint8_t *data,  const uint32_t datalen,
	     const uint8_t lambda, const uint8_t  min_garlic,
	     const uint8_t garlic, const uint8_t  hashlen,
	     const uint8_t client, const uint8_t  tweak_id, uint8_t *hash)
{
  uint8_t x[H_LEN];
  uint8_t hv[H_LEN];
  uint8_t t[4];
  uint8_t c;

  if((hashlen > H_LEN) || (garlic > 63) || (min_garlic > garlic) || 
    (lambda == 0) || (garlic == 0)){
     return -1;
  }

  /*Compute H(V)*/
  __Hash1(VERSION_ID, strlen((char*)VERSION_ID), hv);

  /* Compute Tweak */
  t[0] = tweak_id;
  t[1] = lambda;
  t[2] = hashlen;
  t[3] = saltlen;

  /* Compute H(AD) */
  __Hash1((uint8_t *) data, datalen,x);

  /* Compute the initial value to hash  */
  __Hash5(hv, H_LEN, t, 4, x, H_LEN, pwd,  pwdlen, salt, saltlen, x);

  /*Overwrite Password if enabled*/
#ifdef OVERWRITE
  erasepwd(pwd,pwdlen);
#endif

  Flap(x, lambda, (min_garlic+1)/2, salt, saltlen, x);

  for(c=min_garlic; c <= garlic; c++)
  {
      Flap(x, lambda, c, salt, saltlen, x);
      if( (c==garlic) && (client == CLIENT))
      {
        memcpy(hash, x, H_LEN);
        return 0;
      }
      __Hash2(&c,1, x,H_LEN, x);
      memset(x+hashlen, 0, H_LEN-hashlen);
  }
  memcpy(hash, x, hashlen);

  return 0;
}


/***************************************************/

int Catena(uint8_t *pwd,   const uint32_t pwdlen,
	   const uint8_t *salt,  const uint8_t  saltlen,
	   const uint8_t *data,  const uint32_t datalen,
	   const uint8_t lambda, const uint8_t  min_garlic,
	   const uint8_t garlic, const uint8_t  hashlen,  uint8_t *hash)
{
  return __Catena(pwd, pwdlen, salt, saltlen, data, datalen,
		  lambda, min_garlic, garlic,
		  hashlen,  REGULAR, PASSWORD_HASHING_MODE, hash);

}


/***************************************************/


int Naive_Catena(char *pwd,  const char *salt, const char *data,
		  uint8_t hash[H_LEN])
{
  return __Catena( (uint8_t  *) pwd, strlen(pwd),
		   (uint8_t  *) salt, strlen(salt),
		   (uint8_t  *) data, strlen(data),
		   LAMBDA, MIN_GARLIC, GARLIC,
		   H_LEN, REGULAR, PASSWORD_HASHING_MODE, hash);
}

/***************************************************/


int Simple_Catena(uint8_t *pwd,   const uint32_t pwdlen,
		  const uint8_t *salt,  const uint8_t  saltlen,
		  const uint8_t *data,  const uint32_t datalen,
		  uint8_t hash[H_LEN])
{
  return __Catena(pwd, pwdlen, salt, saltlen, data, datalen,
		  LAMBDA, MIN_GARLIC, GARLIC, H_LEN,
		  REGULAR, PASSWORD_HASHING_MODE, hash);
}


/***************************************************/

int Catena_Client(uint8_t  *pwd,   const uint32_t pwdlen,
		  const uint8_t  *salt,  const uint8_t  saltlen,
		  const uint8_t  *data,  const uint32_t datalen,
		  const uint8_t lambda, const uint8_t  min_garlic,
		  const uint8_t  garlic, const uint8_t  hashlen,
		  uint8_t x[H_LEN])
{
  return __Catena(pwd, pwdlen, (uint8_t *) salt, saltlen, data, datalen,
		  lambda, min_garlic, garlic, hashlen,
		  CLIENT, PASSWORD_HASHING_MODE, x);
}

/***************************************************/

int Catena_Server(const uint8_t garlic,  const uint8_t x[H_LEN],
		  const uint8_t hashlen, uint8_t *hash)
{
  uint8_t z[H_LEN];

  if (hashlen > H_LEN) return -1;
  __Hash2(&garlic,1,x, H_LEN, z);
    memcpy(hash, z, hashlen);

  return 0;
}

/***************************************************/

void CI_Update(const uint8_t *old_hash,  const uint8_t lambda,
         const uint8_t *salt,  const uint8_t saltlen,
	       const uint8_t old_garlic, const uint8_t new_garlic,
	       const uint8_t hashlen, uint8_t *new_hash)
{
  uint8_t c;
  uint8_t x[H_LEN];

  memcpy(x, old_hash, hashlen);
  memset(x+hashlen, 0, H_LEN-hashlen);

  for(c=old_garlic+1; c <= new_garlic; c++)
    {
      Flap(x, lambda, c, salt, saltlen, x);
      __Hash2(&c,1,x, H_LEN, x);
      memset(x+hashlen, 0, H_LEN-hashlen);
    }
  memcpy(new_hash,x,hashlen);
}

/***************************************************/

void CI_Keyed_Update(const uint8_t *old_hash,  const uint8_t lambda,
            const uint8_t *salt,      const uint8_t saltlen,
            const uint8_t old_garlic, const uint8_t new_garlic,
            const uint8_t hashlen,    const uint8_t *key,
            const uint64_t uuid,      uint8_t *new_hash)
{
  uint8_t keystream[H_LEN];
  uint64_t tmp = TO_LITTLE_ENDIAN_64(uuid);
  uint8_t c;
  uint8_t x[H_LEN];

  memcpy(x, old_hash, hashlen);
  memset(x+hashlen, 0, H_LEN-hashlen);

  __Hash4(key, KEY_LEN,  (uint8_t*) &tmp, 8, &old_garlic, sizeof(uint8_t), key, 
      KEY_LEN, keystream);

  //decrypt the old hash
  for(int i=0; i<hashlen; i++) x[i] ^= keystream[i];

  for(c=old_garlic+1; c <= new_garlic; c++)
  {
      Flap(x, lambda, c, salt, saltlen, x);
      __Hash2(&c,1,x, H_LEN, x);
      memset(x+hashlen, 0, H_LEN-hashlen);
  }

  __Hash4(key, KEY_LEN,  (uint8_t*) &tmp, 8, &new_garlic, sizeof(uint8_t), key, 
      KEY_LEN, keystream);

  //encrypt the new hash
  for(int i=0; i<hashlen; i++) x[i] ^= keystream[i];

  memcpy(new_hash,x,hashlen);
}


/***************************************************/

void Catena_KG(uint8_t *pwd,   const uint32_t pwdlen,
	       const uint8_t *salt,  const uint8_t saltlen,
	       const uint8_t *data,  const uint32_t datalen,
	       const uint8_t lambda, const uint8_t  min_garlic,
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
	   lambda, min_garlic, garlic, H_LEN, REGULAR, KEY_DERIVATION_MODE,
	   hash);

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

void Catena_Keyed_Hashing(uint8_t *pwd,   const uint32_t pwdlen,
			  const uint8_t *salt,  const uint8_t saltlen,
			  const uint8_t *data,  const uint32_t datalen,
			  const uint8_t lambda, const uint8_t  min_garlic,
			  const uint8_t garlic, const uint8_t  hashlen,
			  const uint8_t *key,   const uint64_t uuid,
			  uint8_t *chash)
{
  uint8_t keystream[H_LEN];
  uint64_t tmp = TO_LITTLE_ENDIAN_64(uuid);
  int i;

   __Catena(pwd, pwdlen, salt, saltlen, data, datalen,
	    lambda, min_garlic, garlic, hashlen,
	    REGULAR, PASSWORD_HASHING_MODE, chash);

   __Hash4(key, KEY_LEN,  (uint8_t*) &tmp, 8, &garlic, sizeof(uint8_t), key, 
      KEY_LEN, keystream);

   for(i=0; i<hashlen; i++) chash[i] ^= keystream[i];
}


/***************************************************/
#pragma GCC diagnostic ignored "-Wunused-parameter"
int PHS(void *out, size_t outlen,  const void *in, size_t inlen,
	   const void *salt, size_t saltlen, unsigned int t_cost,
	   unsigned int m_cost) {
  //check range of parameters against the parameter types of __Catena
  if((outlen > UINT8_MAX) || (inlen > UINT32_MAX) || (saltlen > UINT32_MAX) || 
      (t_cost > UINT8_MAX) || (m_cost > UINT8_MAX)){
    return 1;
  }
  return __Catena((const uint8_t * )in, inlen, salt, saltlen, (const uint8_t *)
		  "", 0, t_cost, m_cost, m_cost, outlen, REGULAR,
		  PASSWORD_HASHING_MODE, out);
}

#pragma GCC diagnostic warning "-Wunused-parameter"