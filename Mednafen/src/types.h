#ifndef __MDFN_TYPES
#define __MDFN_TYPES

// Yes, yes, I know:  There's a better place for including config.h than here, but I'm tired, and this should work fine. :b
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <inttypes.h>

#if HAVE_MKDIR
 #if MKDIR_TAKES_ONE_ARG
  #define MDFN_mkdir(a, b) mkdir(a)
 #else
  #define MDFN_mkdir(a, b) mkdir(a, b)
 #endif
#else
 #if HAVE__MKDIR
  /* Plain Win32 */
  #define MDFN_mkdir(a, b) _mkdir(a)
 #else
  #error "Don't know how to create a directory on this system."
 #endif
#endif

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32; 

typedef uint8_t uint8;  
typedef uint16_t uint16;
typedef uint32_t uint32;

typedef uint8 bool8;

#ifdef __GNUC__
 typedef unsigned long long uint64;
 typedef long long int64;
 #define INLINE inline
 #define GINLINE inline

 #define ALWAYS_INLINE inline __attribute__((always_inline))
#elif MSVC
 typedef __int64 int64;
 typedef unsigned __int64 uint64;

 #define ALWAYS_INLINE __inline
 #define INLINE __inline
 #define GINLINE			/* Can't declare a function INLINE
					   and global in MSVC.  Bummer.
					*/
 #define PSS_STYLE 2			/* Does MSVC compile for anything
					   other than Windows/DOS targets?
					*/
#endif


typedef struct
{
 union
 {
  struct
  {
   #ifdef MSB_FIRST
   uint8   High;
   uint8   Low;
   #else
   uint8   Low;
   uint8   High;
   #endif
  } Union8;
  uint16 Val16;
 };
} Uuint16;

typedef struct
{
 union
 {
  struct
  {
   #ifdef MSB_FIRST
   Uuint16   High;
   Uuint16   Low;
   #else
   Uuint16   Low;
   Uuint16   High;
   #endif
  } Union16;
  uint32  Val32;
 };
} Uuint32;


#if PSS_STYLE==2

#define PSS "\\"
#define MDFN_PS '\\'

#elif PSS_STYLE==1

#define PSS "/"
#define MDFN_PS '/'

#elif PSS_STYLE==3

#define PSS "\\"
#define MDFN_PS '\\'

#elif PSS_STYLE==4

#define PSS ":" 
#define MDFN_PS ':'

#endif

typedef void (*writefunc)(uint32 A, uint8 V);
typedef uint8 (*readfunc)(uint32 A);

typedef uint32   UTF32;  /* at least 32 bits */
typedef uint16  UTF16;  /* at least 16 bits */
typedef uint8   UTF8;   /* typically 8 bits */
typedef unsigned char   Boolean; /* 0 or 1 */

typedef struct
{
 int x, y, w, h;
} MDFN_Rect;

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#undef require
#define require( expr ) assert( expr )

/*! Macros for big- and little-endian support (defaults to little-endian). */

#if !defined(MSB_FIRST) && !defined(LSB_FIRST)
 #error "Define MSB_FIRST or LSB_FIRST!"
#endif

#ifdef MSB_FIRST

 #ifdef LSB_FIRST
  #error Only define one of LSB_FIRST and MSB_FIRST
 #endif

 #ifndef le32toh
  #define le32toh(l)      ((((l)>>24) & 0xff) | (((l)>>8) & 0xff00) \
                         | (((l)<<8) & 0xff0000) | (((l)<<24) & 0xff000000))
 #endif
 #ifndef le16toh
  #define le16toh(l)      ((((l)>>8) & 0xff) | (((l)<<8) & 0xff00))
 #endif
#else
 #ifndef le32toh
  #define le32toh(l)      (l)
 #endif
 #ifndef le16toh
  #define le16toh(l)      (l)
 #endif
#endif

#ifndef htole32
#define htole32 le32toh
#endif

#ifndef htole16
#define htole16 le16toh
#endif


#define INT_TO_BCD(A)  (((A) / 10) * 16 + ((A) % 10))              // convert INT --> BCD
#define BCD_TO_INT(B)  (((B) / 16) * 10 + ((B) % 16))              // convert BCD --> INT

#define INT16_TO_BCD(A)  ((((((A) % 100) / 10) * 16 + ((A) % 10))) | (((((((A) / 100) % 100) / 10) * 16 + (((A) / 100) % 10))) << 8))   // convert INT16 --> BCD

// Some compilers' optimizers and some platforms might fubar the generated code from these macros, so some tests are run in...tests.cpp
#define sign_9_to_s16(_value)  (((int16)((unsigned int)(_value) << 7)) >> 7)
#define sign_10_to_s16(_value)  (((int16)((uint32)(_value) << 6)) >> 6)
#define sign_11_to_s16(_value)  (((int16)((uint32)(_value) << 5)) >> 5)
#define sign_12_to_s16(_value)  (((int16)((uint32)(_value) << 4)) >> 4)
#define sign_13_to_s16(_value)  (((int16)((uint32)(_value) << 3)) >> 3)
#define sign_14_to_s16(_value)  (((int16)((uint32)(_value) << 2)) >> 2)
#define sign_15_to_s16(_value)  (((int16)((uint32)(_value) << 1)) >> 1)

// This obviously won't convert higher-than-32 bit numbers to signed 32-bit ;)
// Also, this shouldn't be used for 8-bit and 16-bit signed numbers, since you can
// convert those faster with typecasts...
#define sign_x_to_s32(_bits, _value) (((int32)((uint32)(_value) << (32 - _bits))) >> (32 - _bits))

#include "error.h"

#endif
