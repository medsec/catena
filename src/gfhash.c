#include "gfhash.h"

#ifdef __PCLMUL__
/* Faster version if the pclmulqdq instruction is avaiable
*/

/* Bitwise shift for __m128i types
*/
inline __m128i sll128(__m128i a, uint8_t count){
	__m128i result,tmp;
	result = _mm_slli_epi64(a, count);
	//repair by recovering the missing bits in the first 64bit word of result
    tmp = _mm_slli_si128(a, 8);
    tmp = _mm_srli_epi64(tmp, 64 - (count));
    result = _mm_or_si128(result, tmp);
    return result;
}

/* Galois Field multiplication in GF2^128 with modulus x^128+x^7+x^2+x+1
* see "Intel® Carry-Less Multiplication Instruction and its Usage for Computing
* the GCM Mode" by Shay Gueron and Michael E. Kounavis
*/
__m128i gfmul128(__m128i A, __m128i B)
{
    //A: [A1,A0] B:[B1,B0]
    //Karatsuba multiplication
    const __m128i c =_mm_clmulepi64_si128(A, B, 0x11); //A1 * B1
    const uint64_t c0 = _mm_extract_epi64(c,0);
    const __m128i d =_mm_clmulepi64_si128(A, B, 0x00); //A0 * B0
    const uint64_t d1 = _mm_extract_epi64(d,1);
    //shift A by 64 bit: A1 in the position of A0
    const __m128i a1 = _mm_srli_si128(A, 8); 
    const __m128i a0Xa1 = _mm_xor_si128(A,a1);
    const __m128i b1 = _mm_srli_si128(B, 8); //same for b
    const __m128i b0Xb1 = _mm_xor_si128(B,b1);
    //lower halfs of a0Xa1 and b0Xb1 now contain A1^A0 and B1^B0
    const __m128i e =_mm_clmulepi64_si128(a0Xa1, b0Xb1, 0x00); //A0^A1 * B0^B1

    const __m128i cXd = _mm_xor_si128(c,d);
    const __m128i cXdXe = _mm_xor_si128(cXd,e);
    const uint64_t c1Xd1Xe1 = _mm_extract_epi64(cXdXe,1);
    const uint64_t c0Xd0Xe0 = _mm_extract_epi64(cXdXe,0);
    //A * B = [X3;X2;X1;X0]
    const uint64_t x3 = _mm_extract_epi64(c,1); //c1
    const uint64_t x2 = c0 ^ c1Xd1Xe1;
    const uint64_t x1 = d1 ^ c0Xd0Xe0;
    const uint64_t x0 = _mm_extract_epi64(d,0);

    //reduction of x with polynomial x^128+x^7+x^2+x+1
    const uint64_t ra = x3 >> (64-1);
    const uint64_t rb = x3 >> (64-2);
    const uint64_t rc = x3 >> (64-7);

    const uint64_t rd = x2 ^ ra ^ rb ^ rc;

    const __m128i rx3d = _mm_set_epi64((__m64)x3,(__m64)rd);

    const __m128i re = sll128(rx3d,1);
    const __m128i rf = sll128(rx3d,2);
    const __m128i rg = sll128(rx3d,7);
    const __m128i reXrf = _mm_xor_si128(re,rf);
    const __m128i reXrfXrg = _mm_xor_si128(reXrf,rg);

    const __m128i rh = _mm_xor_si128(rx3d, reXrfXrg);
    const __m128i x0x1 = _mm_set_epi64((__m64)x1,(__m64)x0);
    const __m128i result = _mm_xor_si128(x0x1, rh);

    return result;
}

void gfhash(uint8_t * source1, uint8_t * source2, uint8_t* target){
  int blocks = H_LEN/sizeof(__m128i);
  __m128i left, right, result;
  for(int i = 0; i < blocks; i++){
    left = _mm_load_si128((__m128i*) (source1 + (i*sizeof(__m128i))));
    right = _mm_load_si128((__m128i*) (source2 + (i*sizeof(__m128i))));
    result = gfmul128(left, right);
    _mm_store_si128((__m128i*) (target + (i*sizeof(__m128i))), result);
  }
}

#else //__PCLMUL__
/* Version without any intrinsics
*/

/* Carry less multiplication of two 64bit words
* see Faster Multiplication in GF(2)[x] by Richard P. Brent et al.
*/
void clmul64(uint64_t a, uint64_t b, uint64_t* r){
    uint8_t s = 4,i; //window size
    uint64_t two_s = 1 << s; //2^s
    uint64_t smask = two_s-1; //s 1 bits
    uint64_t u[two_s];
    uint64_t tmp;
    uint64_t ifmask;
    //Precomputation
    u[0] = 0;
    u[1] = b;
    for(i = 2 ; i < two_s; i += 2){
        u[i] = u[i >> 1] << 1; //even indices: left shift
        u[i + 1] = u[i] ^ b; //odd indices: xor b
    }
    //Multiply
    r[0] = u[a & smask]; //first window only affects lower word
    r[1] = 0;
    for(i = s ; i < 64 ; i += s){
        tmp = u[a >> i & smask];     
        r[0] ^= tmp << i;
        r[1] ^= tmp >> (64 - i);
    }
    //Repair
    uint64_t m = 0xEEEEEEEEEEEEEEEE; //s=4 => 16 times 1110
    for(i = 1 ; i < s ; i++){
        //a = (a << 1) & m;
        tmp = ((a & m) >> i);
        m &= m << 1; //shift mask to exclude all bit j': j' mod s = i
        ifmask = -((b >> (64-i)) & 1); //if the (64-i)th bit of b is 1
        r[1] ^= (tmp & ifmask);
    }
}

/* Galois Field multiplication in GF2^128 with modulus x^128+x^7+x^2+x+1
 */
void gfmul128(const uint64_t* a, const uint64_t* b, uint64_t* r){
    uint64_t e[2];
    uint64_t x[4];
    //Karatsuba
    clmul64(a[0], b[0], x); //lower = d
    clmul64(a[1], b[1], x+2); //higher = c
    clmul64(a[0]^a[1], b[0]^b[1], e); // mid = e

    const uint64_t d1Xc0 = x[1]^x[2]; //d1 ^ c0
    x[1] = x[0]^e[0]^d1Xc0; //d0 ^ e0 ^ d1 ^ c0
    x[2] = x[3]^e[1]^d1Xc0; //d1 ^ e1 ^ d1 ^ c0

     //reduction of x with polynomial x^128+x^7+x^2+x+1
    const uint64_t ra = x[3] >> (64-1);
    const uint64_t rb = x[3] >> (64-2);
    const uint64_t rc = x[3] >> (64-7);

    const uint64_t rd = x[2] ^ ra ^ rb ^ rc;
    //shift 128 bit word [x3;d] to the left by 1,2,7
    const uint64_t re1 = (x[3] << 1) ^ (rd >> (64-1));
    const uint64_t re0 = rd << 1;
    const uint64_t f1 = (x[3] << 2) ^ (rd >> (64-2));
    const uint64_t f0 = rd << 2;
    const uint64_t g1 = (x[3] << 7) ^ (rd >> (64-7));
    const uint64_t g0 = rd << 7;

    const uint64_t e1Xf1Xg1 = re1 ^ f1 ^ g1;
    const uint64_t e0Xf0Xg0 = re0 ^ f0 ^ g0;

    const uint64_t h1 = x[3] ^ e1Xf1Xg1;
    const uint64_t h0 = rd ^ e0Xf0Xg0;
    r[1] = h1 ^ x[1];
    r[0] = h0 ^ x[0];
}

void gfhash(uint8_t * source1, uint8_t * source2, uint8_t* target){
  int blocks = H_LEN/16;
  for(int i = 0; i < blocks; i++){
    gfmul128((uint64_t*)(source1 + (i*16)), (uint64_t*)(source2 + (i*16)),
    	(uint64_t*)(target + (i*16)));
  }
}

#endif //__PCLMUL__