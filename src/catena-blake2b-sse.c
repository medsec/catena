#include "catena.h"
//Loading in blake2b is faster on little endian
#if __BYTE_ORDER == __LITTLE_ENDIAN
#ifndef NATIVE_LITTLE_ENDIAN
  #define NATIVE_LITTLE_ENDIAN
#endif
#endif
#include "blake2-sse/blake2.h"
#include "hash.h"

#include "blake2-sse/blake2-config.h"
#include "blake2-sse/blake2b-round.h"
#include "blake2-sse/blake2-impl.h"

#include <emmintrin.h>
#if defined(HAVE_SSSE3)
#include <tmmintrin.h>
#endif
#if defined(HAVE_SSE41)
#include <smmintrin.h>
#endif
#if defined(HAVE_AVX)
#include <immintrin.h>
#endif
#if defined(HAVE_XOP)
#include <x86intrin.h>
#endif

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

ALIGN( 64 ) static const uint64_t blake2b_IV[8] =
{
  0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL,
  0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
  0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
  0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};


static inline int blake2b_increment_counter( blake2b_state *S, const uint64_t inc )
{
#if __x86_64__
  // ADD/ADC chain
  __uint128_t t = ( ( __uint128_t )S->t[1] << 64 ) | S->t[0];
  t += inc;
  S->t[0] = ( uint64_t )( t >>  0 );
  S->t[1] = ( uint64_t )( t >> 64 );
#else
  S->t[0] += inc;
  S->t[1] += ( S->t[0] < inc );
#endif
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
  const uint8_t block[BLAKE2B_BLOCKBYTES], unsigned ridx){
    __m128i row1l, row1h;
  __m128i row2l, row2h;
  __m128i row3l, row3h;
  __m128i row4l, row4h;
  __m128i b0, b1;
  __m128i t0, t1;
#if defined(HAVE_SSSE3) && !defined(HAVE_XOP)
  const __m128i r16 = _mm_setr_epi8( 2, 3, 4, 5, 6, 7, 0, 1, 10, 11, 12, 13, 14, 15, 8, 9 );
  const __m128i r24 = _mm_setr_epi8( 3, 4, 5, 6, 7, 0, 1, 2, 11, 12, 13, 14, 15, 8, 9, 10 );
#endif
#if defined(HAVE_SSE41)
  const __m128i m0 = LOADU( block + 00 );
  const __m128i m1 = LOADU( block + 16 );
  const __m128i m2 = LOADU( block + 32 );
  const __m128i m3 = LOADU( block + 48 );
  const __m128i m4 = LOADU( block + 64 );
  const __m128i m5 = LOADU( block + 80 );
  const __m128i m6 = LOADU( block + 96 );
  const __m128i m7 = LOADU( block + 112 );
#else
  const uint64_t  m0 = ( ( uint64_t * )block )[ 0];
  const uint64_t  m1 = ( ( uint64_t * )block )[ 1];
  const uint64_t  m2 = ( ( uint64_t * )block )[ 2];
  const uint64_t  m3 = ( ( uint64_t * )block )[ 3];
  const uint64_t  m4 = ( ( uint64_t * )block )[ 4];
  const uint64_t  m5 = ( ( uint64_t * )block )[ 5];
  const uint64_t  m6 = ( ( uint64_t * )block )[ 6];
  const uint64_t  m7 = ( ( uint64_t * )block )[ 7];
  const uint64_t  m8 = ( ( uint64_t * )block )[ 8];
  const uint64_t  m9 = ( ( uint64_t * )block )[ 9];
  const uint64_t m10 = ( ( uint64_t * )block )[10];
  const uint64_t m11 = ( ( uint64_t * )block )[11];
  const uint64_t m12 = ( ( uint64_t * )block )[12];
  const uint64_t m13 = ( ( uint64_t * )block )[13];
  const uint64_t m14 = ( ( uint64_t * )block )[14];
  const uint64_t m15 = ( ( uint64_t * )block )[15];
#endif
  row1l = LOAD( &S->h[0] );
  row1h = LOAD( &S->h[2] );
  row2l = LOAD( &S->h[4] );
  row2h = LOAD( &S->h[6] );
  row3l = LOAD( &blake2b_IV[0] );
  row3h = LOAD( &blake2b_IV[2] );
  row4l = _mm_xor_si128( LOAD( &blake2b_IV[4] ), LOAD( &S->t[0] ) );
  row4h = _mm_xor_si128( LOAD( &blake2b_IV[6] ), LOAD( &S->f[0] ) );

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
  
  row1l = _mm_xor_si128( row3l, row1l );
  row1h = _mm_xor_si128( row3h, row1h );
  STORE( &S->h[0], _mm_xor_si128( LOAD( &S->h[0] ), row1l ) );
  STORE( &S->h[2], _mm_xor_si128( LOAD( &S->h[2] ), row1h ) );
  row2l = _mm_xor_si128( row4l, row2l );
  row2h = _mm_xor_si128( row4h, row2h );
  STORE( &S->h[4], _mm_xor_si128( LOAD( &S->h[4] ), row2l ) );
  STORE( &S->h[6], _mm_xor_si128( LOAD( &S->h[6] ), row2h ) );
}

/* Single round of Blake2b that hashes two 512bit inputs to one 512bit hash
*  The round that is used is determined by the current vertex index(vindex).
*  A single state is used for every consecutive call.
*/
void __HashFast(int vindex, const uint8_t* i1, 
       const uint8_t* i2, uint8_t hash[H_LEN]){
  memcpy(_state.buf, i1, H_LEN);
  memcpy(_state.buf + H_LEN, i2, H_LEN);
  _state.buflen = 128;
  blake2b_increment_counter(&_state, _state.buflen);
  blake2b_set_lastblock(&_state);
  //No Padding necessary because the last 1024bits of _state.buf are 0 anyways
  const int rindex = vindex % 12;
  blake2round(&_state, _state.buf, rindex);
  
  memcpy(hash, &_state.h[0], H_LEN);
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