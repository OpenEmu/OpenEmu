/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2002  Dave2001
* Copyright (c) 2003-2009  Sergey 'Gonetz' Lipski
* Copyright (c) 2012-2013 balrog, wahrhaft
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
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


#ifndef WINLNXDEFS_H
#define WINLNXDEFS_H

#define wxPtrToUInt (uintptr_t)
#define TRUE 1
#define FALSE 0

#define _T(x) x

#include <stdint.h>

typedef int BOOL;
typedef uint32_t wxUint32;
typedef uint16_t wxUint16;
typedef uint8_t wxUint8;
typedef uint8_t BYTE;
typedef long long LONGLONG;


typedef int32_t wxInt32;
typedef int16_t wxInt16;
typedef int8_t wxInt8;

typedef uint64_t wxUint64;
typedef int64_t wxInt64;

typedef unsigned char wxChar;
typedef uintptr_t wxUIntPtr;

#ifndef WIN32

typedef union _LARGE_INTEGER
{
   struct
     {
    uint32_t LowPart;
    uint32_t HighPart;
     } s;
   struct
     {
    uint32_t LowPart;
    uint32_t HighPart;
     } u;
   long long QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

#define WINAPI

#endif

#endif
