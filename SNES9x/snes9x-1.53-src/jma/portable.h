/*
Copyright (C) 2004-2006 NSRT Team ( http://nsrt.edgeemu.com )
Copyright (C) 2002 Andrea Mazzoleni ( http://advancemame.sf.net )

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __PORTABLE_H
#define __PORTABLE_H

#include <string.h>
#ifdef __GNUC__
#include <stdint.h>

typedef int8_t    INT8;
typedef uint8_t   UINT8;
typedef int16_t   INT16;
typedef uint16_t  UINT16;
typedef int32_t   INT32;
typedef uint32_t  UINT32;
typedef int64_t   INT64;
typedef uint64_t  UINT64;
typedef uintptr_t UINT_PTR;

#else

typedef signed char        INT8;
typedef unsigned char      UINT8;
typedef short              INT16;
typedef unsigned short     UINT16;
typedef int                INT32;
typedef unsigned int       UINT32;
#ifdef _MSC_VER
typedef __int64            INT64;
typedef unsigned __int64   UINT64;
#else
typedef long long          INT64;
typedef unsigned long long UINT64;
#endif
typedef unsigned           UINT_PTR;

#endif

typedef UINT8  BYTE;
typedef UINT16 WORD;
typedef UINT32 DWORD;

typedef int BOOL;
#define FALSE 0
#define TRUE  1

#define HRESULT           int
#define S_OK              0
#define E_INVALIDARG     -1
#define E_OUTOFMEMORY    -2
#define E_FAIL           -3
#define E_INTERNAL_ERROR -4
#define E_INVALIDDATA    -5

template <class T> inline T MyMin(T a, T b) {
	return a < b ? a : b;
}

template <class T> inline T MyMax(T a, T b) {
	return a > b ? a : b;
}

#define RETURN_IF_NOT_S_OK(x) { HRESULT __aResult_ = (x); if(__aResult_ != S_OK) return __aResult_; }


#define UINT_SIZE   ((int)sizeof(unsigned int))
#define USHORT_SIZE ((int)sizeof(unsigned short))

//Convert an array of 4 bytes back into an integer
inline UINT32 charp_to_uint(const UINT8 buffer[UINT_SIZE])
{
  UINT32 num = (UINT32)buffer[3];
  num |= ((UINT32)buffer[2]) << 8;
  num |= ((UINT32)buffer[1]) << 16;
  num |= ((UINT32)buffer[0]) << 24;
  return(num);
}

//Convert an array of 2 bytes back into a short integer
inline UINT16 charp_to_ushort(const UINT8 buffer[USHORT_SIZE])
{
  UINT16 num = (UINT16)buffer[1];
  num |= ((UINT16)buffer[0]) << 8;
  return(num);
}

#endif
