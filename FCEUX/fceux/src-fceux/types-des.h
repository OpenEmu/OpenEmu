/*  Copyright (C) 2005 Guillaume Duhamel
	Copyright (C) 2008-2009 DeSmuME team

    This file is part of DeSmuME

    DeSmuME is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    DeSmuME is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DeSmuME; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef TYPES_HPP
#define TYPES_HPP

//analyze microsoft compilers
#ifdef _MSC_VER
	#ifdef _XBOX
		//#define _XBOX //already defined
	#else
		#define _WINDOWS
		#ifdef _M_X64
			//#define _WIN64 //already defined in x64 compiler
		#else
			//#define _WIN32 //already defined
		#endif
	#endif
#endif

//todo - everyone will want to support this eventually, i suppose
#ifdef _WINDOWS
#include "config.h"
#endif

//xbox needs to include this to resemble windows
#ifdef _XBOX
	#include <xtl.h>
	#include <io.h>
#endif

#ifdef DEVELOPER
#define IF_DEVELOPER(X) X
#else
#define IF_DEVELOPER(X)
#endif

#ifdef _WINDOWS
	//#define HAVE_WX //not useful yet....
	#define HAVE_LIBAGG
	#define ENABLE_SSE
	#define ENABLE_SSE2
	#ifdef DEVELOPER
		#define HAVE_LUA
	#endif
#endif

#ifdef __GNUC__
#ifdef __SSE__
#define ENABLE_SSE
#endif
#ifdef __SSE2__
#define ENABLE_SSE2
#endif
#endif

#ifdef NOSSE
#undef ENABLE_SSE
#endif

#ifdef NOSSE2
#undef ENABLE_SSE2
#endif

#ifdef _MSC_VER 
#define strcasecmp(x,y) _stricmp(x,y)
#define snprintf _snprintf
#else
#define WINAPI
#endif

#ifdef __GNUC__
#include <limits.h>
#ifndef PATH_MAX
#define MAX_PATH 1024
#else
#define MAX_PATH PATH_MAX
#endif
#endif


#ifdef _XBOX
#define MAX_PATH 1024
#define PATH_MAX 1024
#endif


#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define ALIGN(X) __declspec(align(X))
#elif __GNUC__
#define ALIGN(X) __attribute__ ((aligned (X)))
#else
#define ALIGN(X)
#endif

#define CACHE_ALIGN ALIGN(32)

//use this for example when you want a byte value to be better-aligned
#define FAST_ALIGN ALIGN(4)

#ifndef FASTCALL
#ifdef __MINGW32__
#define FASTCALL __attribute__((fastcall))
#elif defined (__i386__) && !defined(__clang__)
#define FASTCALL __attribute__((regparm(3)))
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define FASTCALL
#else
#define FASTCALL
#endif
#endif

#ifdef _MSC_VER
#define _CDECL_ __cdecl
#else
#define _CDECL_
#endif

#ifndef INLINE
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define INLINE _inline
#else
#define INLINE inline
#endif
#endif

#ifndef FORCEINLINE
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define FORCEINLINE __forceinline
#define MSC_FORCEINLINE __forceinline
#else
#define FORCEINLINE inline __attribute__((always_inline)) 
#define MSC_FORCEINLINE
#endif
#endif

#if defined(__LP64__)
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;
#else
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
typedef unsigned __int64 u64;
#else
typedef unsigned long long u64;
#endif

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
typedef __int64 s64;
#else
typedef signed long long s64;
#endif
#endif

typedef u8  uint8;
typedef u16 uint16;

#ifndef OBJ_C
typedef u32 uint32;
#else
#define uint32 u32 //uint32 is defined in Leopard somewhere, avoid conflicts
#endif

/*---------- GPU3D fixed-points types -----------*/

typedef s32 f32;
#define inttof32(n)          ((n) << 12)
#define f32toint(n)          ((n) >> 12)
#define floattof32(n)        ((int32)((n) * (1 << 12)))
#define f32tofloat(n)        (((float)(n)) / (float)(1<<12))

typedef s16 t16;
#define f32tot16(n)          ((t16)(n >> 8))
#define inttot16(n)          ((n) << 4)
#define t16toint(n)          ((n) >> 4)
#define floattot16(n)        ((t16)((n) * (1 << 4)))
#define t16ofloat(n)         (((float)(n)) / (float)(1<<4))

typedef s16 v16;
#define inttov16(n)          ((n) << 12)
#define f32tov16(n)          (n)
#define floattov16(n)        ((v16)((n) * (1 << 12)))
#define v16toint(n)          ((n) >> 12)
#define v16tofloat(n)        (((float)(n)) / (float)(1<<12))

typedef s16 v10;
#define inttov10(n)          ((n) << 9)
#define f32tov10(n)          ((v10)(n >> 3))
#define v10toint(n)          ((n) >> 9)
#define floattov10(n)        ((v10)((n) * (1 << 9)))
#define v10tofloat(n)        (((float)(n)) / (float)(1<<9))

/*----------------------*/

#ifndef OBJ_C
typedef int BOOL;
#else
//apple also defines BOOL
typedef int desmume_BOOL;
#define BOOL desmume_BOOL
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef __BIG_ENDIAN__
#ifndef WORDS_BIGENDIAN
#define WORDS_BIGENDIAN
#endif
#endif

#ifdef WORDS_BIGENDIAN
# define LOCAL_BE
#else
# define LOCAL_LE
#endif

/* little endian (ds' endianess) to local endianess convert macros */
#ifdef LOCAL_BE	/* local arch is big endian */
# define LE_TO_LOCAL_16(x) ((((x)&0xff)<<8)|(((x)>>8)&0xff))
# define LE_TO_LOCAL_32(x) ((((x)&0xff)<<24)|(((x)&0xff00)<<8)|(((x)>>8)&0xff00)|(((x)>>24)&0xff))
# define LE_TO_LOCAL_64(x) ((((x)&0xff)<<56)|(((x)&0xff00)<<40)|(((x)&0xff0000)<<24)|(((x)&0xff000000)<<8)|(((x)>>8)&0xff000000)|(((x)>>24)&0xff00)|(((x)>>40)&0xff00)|(((x)>>56)&0xff))
# define LOCAL_TO_LE_16(x) ((((x)&0xff)<<8)|(((x)>>8)&0xff))
# define LOCAL_TO_LE_32(x) ((((x)&0xff)<<24)|(((x)&0xff00)<<8)|(((x)>>8)&0xff00)|(((x)>>24)&0xff))
# define LOCAL_TO_LE_64(x) ((((x)&0xff)<<56)|(((x)&0xff00)<<40)|(((x)&0xff0000)<<24)|(((x)&0xff000000)<<8)|(((x)>>8)&0xff000000)|(((x)>>24)&0xff00)|(((x)>>40)&0xff00)|(((x)>>56)&0xff))
#else		/* local arch is little endian */
# define LE_TO_LOCAL_16(x) (x)
# define LE_TO_LOCAL_32(x) (x)
# define LE_TO_LOCAL_64(x) (x)
# define LOCAL_TO_LE_16(x) (x)
# define LOCAL_TO_LE_32(x) (x)
# define LOCAL_TO_LE_64(x) (x)
#endif

// kilobytes and megabytes macro
#define MB(x) ((x)*1024*1024)
#define KB(x) ((x)*1024)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define CPU_STR(c) ((c==ARM9)?"ARM9":"ARM7")
typedef enum
{
	ARM9 = 0,
	ARM7 = 1
} cpu_id_t;

///endian-flips count bytes.  count should be even and nonzero.
inline void FlipByteOrder(u8 *src, u32 count)
{
	u8 *start=src;
	u8 *end=src+count-1;

	if((count&1) || !count)        return;         /* This shouldn't happen. */

	while(count--)
	{
		u8 tmp;

		tmp=*end;
		*end=*start;
		*start=tmp;
		end--;
		start++;
	}
}



inline u64 double_to_u64(double d) {
	union {
		u64 a;
		double b;
	} fuxor;
	fuxor.b = d;
	return fuxor.a;
}

inline double u64_to_double(u64 u) {
	union {
		u64 a;
		double b;
	} fuxor;
	fuxor.a = u;
	return fuxor.b;
}

inline u32 float_to_u32(float f) {
	union {
		u32 a;
		float b;
	} fuxor;
	fuxor.b = f;
	return fuxor.a;
}

inline float u32_to_float(u32 u) {
	union {
		u32 a;
		float b;
	} fuxor;
	fuxor.a = u;
	return fuxor.b;
}


///stores a 32bit value into the provided byte array in guaranteed little endian form
inline void en32lsb(u8 *buf, u32 morp)
{ 
	buf[0]=(u8)(morp);
	buf[1]=(u8)(morp>>8);
	buf[2]=(u8)(morp>>16);
	buf[3]=(u8)(morp>>24);
} 

inline void en16lsb(u8* buf, u16 morp)
{
	buf[0]=(u8)morp;
	buf[1]=(u8)(morp>>8);
}

///unpacks a 64bit little endian value from the provided byte array into host byte order
inline u64 de64lsb(u8 *morp)
{
	return morp[0]|(morp[1]<<8)|(morp[2]<<16)|(morp[3]<<24)|((u64)morp[4]<<32)|((u64)morp[5]<<40)|((u64)morp[6]<<48)|((u64)morp[7]<<56);
}

///unpacks a 32bit little endian value from the provided byte array into host byte order
inline u32 de32lsb(u8 *morp)
{
	return morp[0]|(morp[1]<<8)|(morp[2]<<16)|(morp[3]<<24);
}

///unpacks a 16bit little endian value from the provided byte array into host byte order
inline u16 de16lsb(u8 *morp)
{
	return morp[0]|(morp[1]<<8);
}

#ifndef ARRAY_SIZE
//taken from winnt.h
extern "C++" // templates cannot be declared to have 'C' linkage
template <typename T, size_t N>
char (*BLAHBLAHBLAH( UNALIGNED T (&)[N] ))[N];

#define ARRAY_SIZE(A) (sizeof(*BLAHBLAHBLAH(A)))
#endif


//fairly standard for loop macros
#define MACRODO1(TRICK,TODO) { const int X = TRICK; TODO; }
#define MACRODO2(X,TODO)   { MACRODO1((X),TODO)   MACRODO1(((X)+1),TODO) }
#define MACRODO4(X,TODO)   { MACRODO2((X),TODO)   MACRODO2(((X)+2),TODO) }
#define MACRODO8(X,TODO)   { MACRODO4((X),TODO)   MACRODO4(((X)+4),TODO) }
#define MACRODO16(X,TODO)  { MACRODO8((X),TODO)   MACRODO8(((X)+8),TODO) }
#define MACRODO32(X,TODO)  { MACRODO16((X),TODO)  MACRODO16(((X)+16),TODO) }
#define MACRODO64(X,TODO)  { MACRODO32((X),TODO)  MACRODO32(((X)+32),TODO) }
#define MACRODO128(X,TODO) { MACRODO64((X),TODO)  MACRODO64(((X)+64),TODO) }
#define MACRODO256(X,TODO) { MACRODO128((X),TODO) MACRODO128(((X)+128),TODO) }

//this one lets you loop any number of times (as long as N<256)
#define MACRODO_N(N,TODO) {\
	if((N)&0x100) MACRODO256(0,TODO); \
	if((N)&0x080) MACRODO128((N)&(0x100),TODO); \
	if((N)&0x040) MACRODO64((N)&(0x100|0x080),TODO); \
	if((N)&0x020) MACRODO32((N)&(0x100|0x080|0x040),TODO); \
	if((N)&0x010) MACRODO16((N)&(0x100|0x080|0x040|0x020),TODO); \
	if((N)&0x008) MACRODO8((N)&(0x100|0x080|0x040|0x020|0x010),TODO); \
	if((N)&0x004) MACRODO4((N)&(0x100|0x080|0x040|0x020|0x010|0x008),TODO); \
	if((N)&0x002) MACRODO2((N)&(0x100|0x080|0x040|0x020|0x010|0x008|0x004),TODO); \
	if((N)&0x001) MACRODO1((N)&(0x100|0x080|0x040|0x020|0x010|0x008|0x004|0x002),TODO); \
}

//---------------------------
//Binary constant generator macro By Tom Torfs - donated to the public domain

//turn a numeric literal into a hex constant
//(avoids problems with leading zeroes)
//8-bit constants max value 0x11111111, always fits in unsigned long
#define HEX__(n) 0x##n##LU

//8-bit conversion function 
#define B8__(x) ((x&0x0000000FLU)?1:0) \
+((x&0x000000F0LU)?2:0) \
+((x&0x00000F00LU)?4:0) \
+((x&0x0000F000LU)?8:0) \
+((x&0x000F0000LU)?16:0) \
+((x&0x00F00000LU)?32:0) \
+((x&0x0F000000LU)?64:0) \
+((x&0xF0000000LU)?128:0)

//for upto 8-bit binary constants
#define B8(d) ((unsigned char)B8__(HEX__(d)))

// for upto 16-bit binary constants, MSB first
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<<8) \
+ B8(dlsb))

// for upto 32-bit binary constants, MSB first */
#define B32(dmsb,db2,db3,dlsb) (((unsigned long)B8(dmsb)<<24) \
+ ((unsigned long)B8(db2)<<16) \
+ ((unsigned long)B8(db3)<<8) \
+ B8(dlsb))

//Sample usage:
//B8(01010101) = 85
//B16(10101010,01010101) = 43605
//B32(10000000,11111111,10101010,01010101) = 2164238933
//---------------------------

#ifndef CTASSERT
#define	CTASSERT(x)		typedef char __assert ## y[(x) ? 1 : -1]
#endif

static const char hexValid[23] = {"0123456789ABCDEFabcdef"};


template<typename T> inline void reconstruct(T* t) { 
	t->~T();
	new(t) T();
}


#endif
