#ifndef _CATENA_HASH_H__
#define _CATENA_HASH_H__

#include "catena.h"

inline void __Hash1(const uint8_t *input, const uint32_t inputlen,
		    uint8_t hash[H_LEN]);


inline void __Hash2(const uint8_t *i1, const uint32_t i1len,
		    const uint8_t *i2, const uint32_t i2len,
		    uint8_t hash[H_LEN]);


inline void __Hash3(const uint8_t *i1, const uint32_t i1len,
		    const uint8_t *i2, const uint32_t i2len,
		    const uint8_t *i3, const uint32_t i3len,
		    uint8_t hash[H_LEN]);


inline void __Hash4(const uint8_t *i1, const uint32_t i1len,
		    const uint8_t *i2, const uint32_t i2len,
		    const uint8_t *i3, const uint32_t i3len,
		    const uint8_t *i4, const uint32_t i4len,
		    uint8_t hash[H_LEN]);


inline void __Hash5(const uint8_t *i1, const uint32_t i1len,
		    const uint8_t *i2, const uint32_t i2len,
		    const uint8_t *i3, const uint32_t i3len,
		    const uint8_t *i4, const uint32_t i4len,
		    const uint8_t *i5, const uint32_t i5len,
		    uint8_t hash[H_LEN]);

#ifdef FAST_HASH
inline void __FastHash1(const uint8_t *input, const uint32_t inputlen,
                    uint8_t hash[H_LEN]);
inline void __FastHash2(const uint8_t *i1, const uint32_t i1len,
		    const uint8_t *i2, const uint32_t i2len,
		    uint8_t hash[H_LEN]);
#else
#define __FastHash1 __Hash1
#define __FastHash2 __Hash2
#endif

#endif
