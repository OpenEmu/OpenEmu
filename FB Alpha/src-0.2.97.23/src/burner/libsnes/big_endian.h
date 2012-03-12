#ifndef _FBA_ENDIAN_H
#define _FBA_ENDIAN_H

typedef union {
  struct { UINT8 h3,h2,h,l; } b;
  struct { UINT16 h,l; } w;
  UINT32 d;
} PAIR;

#if defined(_XBOX)
#define BURN_ENDIAN_SWAP_INT8(x)				(x^1)
#define BURN_ENDIAN_SWAP_INT16(x)				((((x) & 0xff) <<  8) | (((x) & 0xff00) >> 8))
#define BURN_ENDIAN_SWAP_INT32(x)				(_byteswap_ulong(x))
#define BURN_ENDIAN_SWAP_INT64(x)				(_byteswap_uint64(x))
#endif

#endif
