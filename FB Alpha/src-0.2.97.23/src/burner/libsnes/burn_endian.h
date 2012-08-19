#ifndef _FBA_ENDIAN_H
#define _FBA_ENDIAN_H

#ifndef _XBOX
#define NO_64BIT_BYTESWAP
#endif

typedef union {
  struct { UINT8 h3,h2,h,l; } b;
  struct { UINT16 h,l; } w;
  UINT32 d;
} PAIR;

/*   Only Xbox 360 so far seems to have byteswap 64-bit intrinsic */
#ifdef NO_64BIT_BYTESWAP
typedef union {
   UINT64 ll;
   struct { UINT32 l, h; } l;
} BYTESWAP_INT64;
#endif

/* Libogc doesn't have intrinsics or ASM macros defined for this */
#if defined(HW_RVL) || defined(GEKKO)
#define __sthbrx(base,index,value)      \
        __asm__ volatile ("sthbrx       %0,%1,%2" : : "r"(value), "b%"(index), "r"(base) : "memory")

#define __stwbrx(base,index,value)      \
        __asm__ volatile ("stwbrx       %0,%1,%2" : : "r"(value), "b%"(index), "r"(base) : "memory")
#endif

/* Xbox 360 */
#if defined(_XBOX)
#define BURN_ENDIAN_SWAP_INT8(x)				(x^1)
#define BURN_ENDIAN_SWAP_INT16(x)				(_byteswap_ushort(x))
#define BURN_ENDIAN_SWAP_INT32(x)				(_byteswap_ulong(x))
#define BURN_ENDIAN_SWAP_INT64(x)				(_byteswap_uint64(x))
/* PlayStation3 */
#elif defined(__CELLOS_LV2__)
#include <ppu_intrinsics.h>
#define BURN_ENDIAN_SWAP_INT8(x)				(x^1)
#define BURN_ENDIAN_SWAP_INT16(x)				({uint16_t t; __sthbrx(&t, x); t;})
#define BURN_ENDIAN_SWAP_INT32(x)				({uint32_t t; __stwbrx(&t, x); t;})
/* Wii */
#elif defined(HW_RVL)
#define BURN_ENDIAN_SWAP_INT8(x)				(x^1)
#define BURN_ENDIAN_SWAP_INT16(x)				({uint16_t t; __sthbrx(&t, 0, x); t;})
#define BURN_ENDIAN_SWAP_INT32(x)				({uint32_t t; __stwbrx(&t, 0, x); t;})
#endif

#ifdef NO_64BIT_BYTESWAP
static inline UINT64 BURN_ENDIAN_SWAP_INT64(UINT64 x)
{
	BYTESWAP_INT64 r = {0};
	r.l.l = BURN_ENDIAN_SWAP_INT32(x);
	r.l.h = BURN_ENDIAN_SWAP_INT32(x >> 32);
	return r.ll;
}
#endif

#endif
