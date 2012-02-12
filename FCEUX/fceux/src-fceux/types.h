/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2001 Aaron Oneal
 *  Copyright (C) 2002 Xodnizel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */  
#ifdef __cplusplus

#ifndef __FCEU_TYPES
#define __FCEU_TYPES

///causes the code fragment argument to be compiled in if the build includes debugging
#ifdef FCEUDEF_DEBUGGER
#define DEBUG(X) X;
#else
#define DEBUG(X)
#endif

#ifdef MSVC
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
#define dup _dup
#define stat _stat
#define fstat _fstat
#define mkdir _mkdir
#define alloca _alloca
#define snprintf _snprintf
#if _MSC_VER < 1500
#define vsnprintf _vsnprintf
#endif
#define W_OK 2
#define R_OK 2
#define X_OK 1
#define F_OK 0
#define PATH_MAX 260
#else

//mingw32 doesnt prototype this for some reason
#if defined(__MINGW32__) || defined(_WIN64)
#undef alloca
#define alloca __builtin_alloca
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <inttypes.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
#endif

#ifdef __GNUC__
 typedef unsigned long long uint64;
 typedef uint64 u64;
 typedef long long int64;
 #define INLINE inline
 #define GINLINE inline
#elif MSVC
 typedef __int64 int64;
 typedef unsigned __int64 uint64;
 #define __restrict__ 
 #define INLINE __inline
 #define GINLINE			/* Can't declare a function INLINE
					   and global in MSVC.  Bummer.
					*/
 #define PSS_STYLE 2			/* Does MSVC compile for anything
					   other than Windows/DOS targets?
					*/

 #if _MSC_VER >= 1300 
  #pragma warning(disable:4244) //warning C4244: '=' : conversion from 'uint32' to 'uint8', possible loss of data
  #pragma warning(disable:4996) //'strdup' was declared deprecated
#endif

 #if _MSC_VER < 1400
  #define vsnprintf _vsnprintf
 #endif
#endif

#if PSS_STYLE==2

#define PSS "\\"
#define PS '\\'

#elif PSS_STYLE==1

#define PSS "/"
#define PS '/'

#elif PSS_STYLE==3

#define PSS "\\"
#define PS '\\'

#elif PSS_STYLE==4

#define PSS ":"
#define PS ':'

#else

#error PSS_STYLE undefined or invalid; see "types.h" for possible values, and add as compile-time option.  

#endif


typedef void (*writefunc)(uint32 A, uint8 V);
typedef uint8 (*readfunc)(uint32 A);

#ifndef CTASSERT
#define CTASSERT(x)  typedef char __assert ## y[(x) ? 1 : -1];
#endif

#include "utils/endian.h"

#endif
#endif
