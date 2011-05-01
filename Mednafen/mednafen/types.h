#ifndef __MDFN_TYPES
#define __MDFN_TYPES

// Make sure this file is included BEFORE a few common standard C header files(stdio.h, errno.h, math.h, AND OTHERS, but this is not an exhaustive check, nor
// should it be), so that any defines in config.h that change header file behavior will work properly.
#if defined(EOF) || defined(EACCES) || defined(F_LOCK) || defined(NULL) || defined(O_APPEND) || defined(M_LOG2E)
 #error "Wrong include order for types.h"
#endif

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
typedef int64_t int64;

typedef uint8_t uint8;  
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


#if !defined(HAVE_NATIVE64BIT) && SIZEOF_VOID_P >= 8
#define HAVE_NATIVE64BIT 1
#endif

#if 0
// Multi-type union array!
template<size_t array_byte_size>
struct mtuarray
{
 union
 {
  uint64 u64[array_byte_size / sizeof(uint64)];
  int64 s64[array_byte_size / sizeof(uint64)];

  uint8 u8[array_byte_size];
  int8 s8[array_byte_size];

  uint16 u16[array_byte_size / sizeof(uint16)];
  int16 s16[array_byte_size / sizeof(int16)];

  uint32 u32[array_byte_size / sizeof(uint32)];
  int32 s32[array_byte_size / sizeof(int32)];
 };
};
#endif

#ifdef __GNUC__

  #define INLINE inline __attribute__((always_inline))
  #define NO_INLINE __attribute__((noinline))

  #if defined(__386__) || defined(__i386__) || defined(__i386) || defined(_M_IX86) || defined(_M_I386)
    #define MDFN_FASTCALL __attribute__((fastcall))
  #else
    #define MDFN_FASTCALL
  #endif

  #define MDFN_ALIGN(n)	__attribute__ ((aligned (n)))
  #define MDFN_FORMATSTR(a,b,c) __attribute__ ((format (a, b, c)));
  #define MDFN_WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))

#elif defined(_MSC_VER)

  #warning "Compiling with MSVC, untested"
  #define INLINE inline __forceinline
  #define NO_INLINE __declspec(noinline)

  #define MDFN_FASTCALL __fastcall

  #define MDFN_ALIGN(n) __declspec(align(n))

  #define MDFN_FORMATSTR(a,b,c)

  #define MDFN_WARN_UNUSED_RESULT

#else
  #error "Not compiling with GCC nor MSVC"
  #define INLINE inline
  #define NO_INLINE

  #define MDFN_FASTCALL

  #define MDFN_ALIGN(n)	// hence the #error.

  #define MDFN_FORMATSTR(a,b,c)

  #define MDFN_WARN_UNUSED_RESULT

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

typedef uint32   UTF32;  /* at least 32 bits */
typedef uint16  UTF16;  /* at least 16 bits */
typedef uint8   UTF8;   /* typically 8 bits */
typedef unsigned char   Boolean; /* 0 or 1 */

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#undef require
#define require( expr ) assert( expr )

#if !defined(MSB_FIRST) && !defined(LSB_FIRST)
 #error "Define MSB_FIRST or LSB_FIRST!"
#endif

#include "error.h"

#endif
