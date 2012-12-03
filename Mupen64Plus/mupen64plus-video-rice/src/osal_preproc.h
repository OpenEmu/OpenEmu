/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - osal_preproc.h                                          *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* this header file is for system-dependent #defines, #includes, and typedefs */

#if !defined(OSAL_PREPROC_H)
#define OSAL_PREPROC_H

#if defined(WIN32)
  #include <windows.h>
  #define strncasecmp _strnicmp
  #define strcasecmp _stricmp
  #define ALIGN(BYTES,DATA) __declspec(align(BYTES)) DATA;
#else

  #define ALIGN(BYTES,DATA) DATA __attribute__((aligned(BYTES)));

typedef unsigned int BOOL;
typedef void* HBITMAP;

typedef struct
{
   int top;
   int bottom;
   int right;
   int left;
} RECT;

#define __cdecl

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef struct tagBITMAPINFOHEADER
{
   unsigned int biSize;
   int biWidth;
   int biHeight;
   unsigned short biPlanes;
   unsigned short biBitCount;
   unsigned int biCompression;
   unsigned int biSizeImage;
   int biXPelsPerMeter;
   int biYPelsPerMeter;
   unsigned int biClrUsed;
   unsigned int biClrImportant;
}  __attribute__ ((packed)) BITMAPINFOHEADER;

typedef struct tagBITMAPINFO
{
   BITMAPINFOHEADER bmiHeader;
   unsigned int unused;
} BITMAPINFO;

typedef struct tagBITMAPFILEHEADER
{
   unsigned short    bfType; 
   unsigned int   bfSize; 
   unsigned short    bfReserved1; 
   unsigned short    bfReserved2; 
   unsigned int   bfOffBits; 
} __attribute__ ((packed)) BITMAPFILEHEADER;

#define BI_RGB 0

#endif // WIN32

#endif // OSAL_PREPROC_H
