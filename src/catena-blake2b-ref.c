#include "catena.h"
//Loading in blake2b is faster on little endian
#if __BYTE_ORDER == __LITTLE_ENDIAN
#ifndef NATIVE_LITTLE_ENDIAN
  #define NATIVE_LITTLE_ENDIAN
#endif
#endif
#include "blake2-ref/blake2.h"
#include "hash.h"

#include "blake2-ref/blake2-impl.h"

#ifdef FAST
static blake2b_state _state;
#endif


inline void __Hash1(const uint8_t *input, const uint32_t inputlen,
		      uint8_t hash[H_LEN])
{
  blake2b_state ctx;
  blake2b_init(&ctx,H_LEN);
  blake2b_update(&ctx, input, inputlen);
  blake2b_final(&ctx, hash, H_LEN);
}


/***************************************************/

inline void __Hash2(const uint8_t *i1, const uint8_t i1len,
		    const uint8_t *i2, const uint8_t i2len,
		    uint8_t hash[H_LEN])
{
  blake2b_state ctx;
  blake2b_init(&ctx,H_LEN);
  blake2b_update(&ctx, i1, i1len);
  blake2b_update(&ctx, i2, i2len);
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

#ifdef FAST
/* Copies of necessary parts of blake2b-sse, that aren't directly accessible
 */

static const uint64_t blake2b_IV[8] =
{
  0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL,
  0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
  0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
  0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};

static const uint8_t blake2b_sigma[12][16] =
{
  {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 } ,
  { 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 } ,
  { 11,  8, 12,  0,  5,  2, 15, 13, 10, 14,  3,  6,  7,  1,  9,  4 } ,
  {  7,  9,  3,  1, 13, 12, 11, 14,  2,  6,  5, 10,  4,  0, 15,  8 } ,
  {  9,  0,  5,  7,  2,  4, 10, 15, 14,  1, 11, 12,  6,  8,  3, 13 } ,
  {  2, 12,  6, 10,  0, 11,  8,  3,  4, 13,  7,  5, 15, 14,  1,  9 } ,
  { 12,  5,  1, 15, 14, 13,  4, 10,  0,  7,  6,  3,  9,  2,  8, 11 } ,
  { 13, 11,  7, 14, 12,  1,  3,  9,  5,  0, 15,  4,  8,  6,  2, 10 } ,
  {  6, 15, 14,  9, 11,  3,  0,  8, 12,  2, 13,  7,  1,  4, 10,  5 } ,
  { 10,  2,  8,  4,  7,  6,  1,  5, 15, 11,  9, 14,  3, 12, 13 , 0 } ,
  {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 } ,
  { 14, 10,  4,  8,  9, 15, 13,  6,  1, 12,  0,  2, 11,  7,  5,  3 }
};

static inline int blake2b_increment_counter( blake2b_state *S, const uint64_t inc )
{
  S->t[0] += inc;
  S->t[1] += ( S->t[0] < inc );
  return 0;
}


static inline int blake2b_set_lastblock( blake2b_state *S )
{
  // if( S->last_node ) blake2b_set_lastnode( S );

  S->f[0] = ~0ULL;
  return 0;
}

/* Blake2b compression function modified to do only one single round
 */
static inline void blake2round(blake2b_state* S, 
    const uint8_t block[BLAKE2B_BLOCKBYTES], unsigned ridx)
{
  uint64_t m[16];
  uint64_t v[16];
  int i;

  for( i = 0; i < 16; ++i )
    m[i] = load64( block + i * sizeof( m[i] ) );

  for( i = 0; i < 8; ++i )
    v[i] = S->h[i];

  v[ 8] = blake2b_IV[0];
  v[ 9] = blake2b_IV[1];
  v[10] = blake2b_IV[2];
  v[11] = blake2b_IV[3];
  v[12] = S->t[0] ^ blake2b_IV[4];
  v[13] = S->t[1] ^ blake2b_IV[5];
  v[14] = S->f[0] ^ blake2b_IV[6];
  v[15] = S->f[1] ^ blake2b_IV[7];
#define G(r,i,a,b,c,d) \
  do { \
    a = a + b + m[blake2b_sigma[r][2*i+0]]; \
    d = rotr64(d ^ a, 32); \
    c = c + d; \
    b = rotr64(b ^ c, 24); \
    a = a + b + m[blake2b_sigma[r][2*i+1]]; \
    d = rotr64(d ^ a, 16); \
    c = c + d; \
    b = rotr64(b ^ c, 63); \
  } while(0)
#define ROUND(r)  \
  do { \
    G(r,0,v[ 0],v[ 4],v[ 8],v[12]); \
    G(r,1,v[ 1],v[ 5],v[ 9],v[13]); \
    G(r,2,v[ 2],v[ 6],v[10],v[14]); \
    G(r,3,v[ 3],v[ 7],v[11],v[15]); \
    G(r,4,v[ 0],v[ 5],v[10],v[15]); \
    G(r,5,v[ 1],v[ 6],v[11],v[12]); \
    G(r,6,v[ 2],v[ 7],v[ 8],v[13]); \
    G(r,7,v[ 3],v[ 4],v[ 9],v[14]); \
  } while(0)

  switch(ridx){
    case 0:ROUND( 0 );break;
    case 1:ROUND( 1 );break;
    case 2:ROUND( 2 );break;
    case 3:ROUND( 3 );break;
    case 4:ROUND( 4 );break;
    case 5:ROUND( 5 );break;
    case 6:ROUND( 6 );break;
    case 7:ROUND( 7 );break;
    case 8:ROUND( 8 );break;
    case 9:ROUND( 9 );break;
    case 10:ROUND( 10 );break;
    case 11:ROUND( 11 );break;
  }
  
 for( i = 0; i < 8; ++i )
    S->h[i] = S->h[i] ^ v[i] ^ v[i + 8];

#undef G
#undef ROUND
}

/* Single round of Blake2b that hashes two 512bit inputs to one 512bit hash
*  The round that is used is determined by the current vertex index(vindex).
*  A single state is used for every consecutive call.
*/
void __HashFast(int vindex, const uint8_t* i1, 
       const uint8_t* i2, uint8_t hash[H_LEN]){
  uint8_t buffer[BLAKE2B_OUTBYTES];

  memcpy(_state.buf, i1, H_LEN);
  memcpy(_state.buf + H_LEN, i2, H_LEN);
  _state.buflen = 128;
  blake2b_increment_counter(&_state, _state.buflen);
  blake2b_set_lastblock(&_state);
  //No Padding necessary because the last 1024bits of _state.buf are 0 anyways
  const int rindex = vindex % 12;
  blake2round(&_state, _state.buf, rindex);
  
  for( int i = 0; i < 8; ++i ) /* Output full hash to temp buffer */
    store64( buffer + sizeof(_state.h[i] ) * i, _state.h[i] );

  memcpy(hash, buffer, H_LEN );
}

void __ResetState(void){
  blake2b_init(&_state,H_LEN);
}

#else

#pragma GCC diagnostic ignored "-Wunused-parameter"
void __HashFast(int vindex, const uint8_t* i1, 
       const uint8_t* i2, uint8_t hash[H_LEN]){
  __Hash2(i1, H_LEN, i2, H_LEN, hash);
}
#pragma GCC diagnostic warning "-Wunused-parameter"

void __ResetState(void){}
#endif