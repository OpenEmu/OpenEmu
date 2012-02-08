#ifndef PORT_H
#define PORT_H

#include "Types.h"

/* if a >= 0 return x else y*/
#define isel(a, x, y) ((x & (~(a >> 31))) + (y & (a >> 31)))

#ifdef _MSC_VER
#include <stdlib.h>
#define strcasecmp _stricmp
#endif

#ifndef LSB_FIRST
#if defined(__SNC__)
#include <ppu_intrinsics.h>
#define READ16LE( base )        (__builtin_lhbrx(base, 0))
#define READ32LE( base )        (__builtin_lwbrx(base, 0))
#define WRITE16LE( base, value )    (__builtin_sthbrx(value, base, 0))
#define WRITE32LE( base, value )    (__builtin_stwbrx(value, base, 0))
#elif defined(__GNUC__) && defined(__ppc__)
#define READ16LE( base )        ({unsigned ppc_lhbrx_; asm( "lhbrx %0,0,%1" : "=r" (ppc_lhbrx_) : "r" (base), "0" (ppc_lhbrx_) ); ppc_lhbrx_;})
#define READ32LE( base )        ({unsigned ppc_lwbrx_; asm( "lwbrx %0,0,%1" : "=r" (ppc_lwbrx_) : "r" (base), "0" (ppc_lwbrx_) ); ppc_lwbrx_;})
#define WRITE16LE( base, value )    ({asm( "sthbrx %0,0,%1" : : "r" (value), "r" (base) );})
#define WRITE32LE( base, value)    ({asm( "stwbrx %0,0,%1" : : "r" (value), "r" (base) );})
#elif _XBOX
#define READ16LE( base)	_byteswap_ushort(*((u16 *)(base)))
#define READ32LE( base) _byteswap_ulong(*((u32 *)(base)))
#define WRITE16LE( base, value) *((u16 *)base) = _byteswap_ushort((value))
#define WRITE32LE( base, value) *((u32 *)base) = _byteswap_ulong((value))
#else
#define READ16LE(x) (*((u16 *)(x))<<8)|(*((u16 *)(x))>>8);
#define READ32LE(x) (*((u32 *)(x))<<24)|((*((u32 *)(x))<<8)&0xff0000)|((((*((u32 *)(x))x>>8)&0xff00)|(*((u32 *)(x))>>24);
#define WRITE16LE(x,v) *((u16 *)x) = (*((u16 *)(v))<<8)|(*((u16 *)(v))>>8);
#define WRITE32LE(x,v) *((u32 *)x) = (v<<24)|((v<<8)&0xff0000)|((v>>8)&0xff00)|(v>>24);
#endif
#else
#define READ16LE(x) *((u16 *)x)
#define READ32LE(x) *((u32 *)x)
#define WRITE16LE(x,v) *((u16 *)x) = (v)
#define WRITE32LE(x,v) *((u32 *)x) = (v)
#endif

#endif
