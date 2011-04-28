/* 2xSaI
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
/* http://lists.fedoraproject.org/pipermail/legal/2009-October/000928.html */

//#include "System.h"
//#include "main.h"
//#include "../gba/Port.h"

//#if defined(__GNUC__) && defined(__i386__)
//  #define 2XSAI_MMX 1
//#else
//  #undef 2XSAI_MMX
//#endif

#ifndef __2XSAI_H
#define __2XSAI_H

typedef unsigned char sai_uint8;
typedef unsigned short sai_uint16;
typedef unsigned sai_uint32;

//#define BLUE_MASK565 0x001F001F
//#define RED_MASK565 0xF800F800
//#define GREEN_MASK565 0x07E007E0

//#define BLUE_MASK555 0x001F001F
//#define RED_MASK555 0x7C007C00
//#define GREEN_MASK555 0x03E003E0

int Init_2xSaI(sai_uint32 systemColorDepth, sai_uint32 BitFormat);
void Super2xSaI   (sai_uint8 *srcPtr, sai_uint32 srcPitch, sai_uint8 *deltaPtr,        sai_uint8 *dstPtr, sai_uint32 dstPitch, int width, int height);
void Super2xSaI32 (sai_uint8 *srcPtr, sai_uint32 srcPitch, sai_uint8 *dstPtr, sai_uint32 dstPitch, int width, int height);
void SuperEagle   (sai_uint8 *srcPtr, sai_uint32 srcPitch, sai_uint8 *deltaPtr,        sai_uint8 *dstPtr, sai_uint32 dstPitch, int width, int height);
void SuperEagle32 (sai_uint8 *srcPtr, sai_uint32 srcPitch, sai_uint8 *dstPtr, sai_uint32 dstPitch, int width, int height);
void _2xSaI       (sai_uint8 *srcPtr, sai_uint32 srcPitch, sai_uint8 *deltaPtr,        sai_uint8 *dstPtr, sai_uint32 dstPitch, int width, int height);
void _2xSaI32     (sai_uint8 *srcPtr, sai_uint32 srcPitch, sai_uint8 *dstPtr, sai_uint32 dstPitch, int width, int height);
void Scale_2xSaI  (sai_uint8 *srcPtr, sai_uint32 srcPitch, sai_uint8 * /* deltaPtr */, sai_uint8 *dstPtr, sai_uint32 dstPitch, sai_uint32 dstWidth, sai_uint32 dstHeight, int width, int height);

#endif
