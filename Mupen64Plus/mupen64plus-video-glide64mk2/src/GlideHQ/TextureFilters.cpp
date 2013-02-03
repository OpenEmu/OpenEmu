/*
Copyright (C) 2003 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

/* Copyright (C) 2007 Hiroshi Morii <koolsmoky(at)users.sourceforge.net>
 * Modified for the Texture Filtering library
 */

#include <string.h>
#include "TextureFilters.h"

/************************************************************************/
/* 2X filters                                                           */
/************************************************************************/

#define DWORD_MAKE(r, g, b, a)   ((uint32) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
#define WORD_MAKE(r, g, b, a)   ((uint16) (((a) << 12) | ((r) << 8) | ((g) << 4) | (b)))

// Basic 2x R8G8B8A8 filter with interpolation

void Texture2x_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
  uint32 *pDst1, *pDst2;
  uint32 *pSrc, *pSrc2;
  uint32 nWidth = width;
  uint32 nHeight = height;

  uint32 b1;
  uint32 g1;
  uint32 r1;
  uint32 a1;
  uint32 b2;
  uint32 g2;
  uint32 r2;
  uint32 a2;
  uint32 b3;
  uint32 g3;
  uint32 r3;
  uint32 a3;
  uint32 b4;
  uint32 g4;
  uint32 r4;
  uint32 a4;

  uint32 xSrc;
  uint32 ySrc;

  for (ySrc = 0; ySrc < nHeight; ySrc++)
  {
    pSrc = (uint32*)(((uint8*)srcPtr)+ySrc*srcPitch);
    pSrc2 = (uint32*)(((uint8*)srcPtr)+(ySrc+1)*srcPitch);
    pDst1 = (uint32*)(((uint8*)dstPtr)+(ySrc*2)*dstPitch);
    pDst2 = (uint32*)(((uint8*)dstPtr)+(ySrc*2+1)*dstPitch);

    for (xSrc = 0; xSrc < nWidth; xSrc++)
    {
      b1 = (pSrc[xSrc]>>0)&0xFF;
      g1 = (pSrc[xSrc]>>8)&0xFF;
      r1 = (pSrc[xSrc]>>16)&0xFF;
      a1 = (pSrc[xSrc]>>24)&0xFF;

      if( xSrc<nWidth-1 )
      {
        b2 = (pSrc[xSrc+1]>>0)&0xFF;
        g2 = (pSrc[xSrc+1]>>8)&0xFF;
        r2 = (pSrc[xSrc+1]>>16)&0xFF;
        a2 = (pSrc[xSrc+1]>>24)&0xFF;
      }

      if( ySrc<nHeight-1 )
      {
        b3 = (pSrc2[xSrc]>>0)&0xFF;
        g3 = (pSrc2[xSrc]>>8)&0xFF;
        r3 = (pSrc2[xSrc]>>16)&0xFF;
        a3 = (pSrc2[xSrc]>>24)&0xFF;
        if( xSrc<nWidth-1 )
        {
          b4 = (pSrc2[xSrc+1]>>0)&0xFF;
          g4 = (pSrc2[xSrc+1]>>8)&0xFF;
          r4 = (pSrc2[xSrc+1]>>16)&0xFF;
          a4 = (pSrc2[xSrc+1]>>24)&0xFF;
        }
      }


      // Pixel 1
      pDst1[xSrc*2] = pSrc[xSrc];

      // Pixel 2
      if( xSrc<nWidth-1 )
      {
        pDst1[xSrc*2+1] = DWORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
      }
      else
        pDst1[xSrc*2+1] = pSrc[xSrc];


      // Pixel 3
      if( ySrc<nHeight-1 )
      {
        pDst2[xSrc*2] = DWORD_MAKE((r1+r3)/2, (g1+g3)/2, (b1+b3)/2, (a1+a3)/2);
      }
      else
        pDst2[xSrc*2] = pSrc[xSrc];

      // Pixel 4
      if( xSrc<nWidth-1 )
      {
        if( ySrc<nHeight-1 )
        {
          pDst2[xSrc*2+1] = DWORD_MAKE((r1+r2+r3+r4)/4, (g1+g2+g3+g4)/4, (b1+b2+b3+b4)/4, (a1+a2+a3+a4)/4);
        }
        else
        {
          pDst2[xSrc*2+1] = DWORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
        }
      }
      else
      {
        if( ySrc<nHeight-1 )
        {
          pDst2[xSrc*2+1] = DWORD_MAKE((r1+r3)/2, (g1+g3)/2, (b1+b3)/2, (a1+a3)/2);
        }
        else
          pDst2[xSrc*2+1] = pSrc[xSrc];
      }
    }
  }
}

#if !_16BPP_HACK
// Basic 2x R4G4B4A4 filter with interpolation
void Texture2x_16(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
  uint16 *pDst1, *pDst2;
  uint16 *pSrc, *pSrc2;
  uint32 nWidth = width;
  uint32 nHeight = height;

  uint16 b1;
  uint16 g1;
  uint16 r1;
  uint16 a1;
  uint16 b2;
  uint16 g2;
  uint16 r2;
  uint16 a2;
  uint16 b3;
  uint16 g3;
  uint16 r3;
  uint16 a3;
  uint16 b4;
  uint16 g4;
  uint16 r4;
  uint16 a4;

  uint16 xSrc;
  uint16 ySrc;

  for (ySrc = 0; ySrc < nHeight; ySrc++)
  {
    pSrc = (uint16*)(((uint8*)srcPtr)+ySrc*srcPitch);
    pSrc2 = (uint16*)(((uint8*)srcPtr)+(ySrc+1)*srcPitch);
    pDst1 = (uint16*)(((uint8*)dstPtr)+(ySrc*2)*dstPitch);
    pDst2 = (uint16*)(((uint8*)dstPtr)+(ySrc*2+1)*dstPitch);

    for (xSrc = 0; xSrc < nWidth; xSrc++)
    {
      b1 = (pSrc[xSrc]>> 0)&0xF;
      g1 = (pSrc[xSrc]>> 4)&0xF;
      r1 = (pSrc[xSrc]>> 8)&0xF;
      a1 = (pSrc[xSrc]>>12)&0xF;

      if( xSrc<nWidth-1 )
      {
        b2 = (pSrc[xSrc+1]>> 0)&0xF;
        g2 = (pSrc[xSrc+1]>> 4)&0xF;
        r2 = (pSrc[xSrc+1]>> 8)&0xF;
        a2 = (pSrc[xSrc+1]>>12)&0xF;
      }

      if( ySrc<nHeight-1 )
      {
        b3 = (pSrc2[xSrc]>> 0)&0xF;
        g3 = (pSrc2[xSrc]>> 4)&0xF;
        r3 = (pSrc2[xSrc]>> 8)&0xF;
        a3 = (pSrc2[xSrc]>>12)&0xF;
        if( xSrc<nWidth-1 )
        {
          b4 = (pSrc2[xSrc+1]>> 0)&0xF;
          g4 = (pSrc2[xSrc+1]>> 4)&0xF;
          r4 = (pSrc2[xSrc+1]>> 8)&0xF;
          a4 = (pSrc2[xSrc+1]>>12)&0xF;
        }
      }

      // Pixel 1
      pDst1[xSrc*2] = pSrc[xSrc];

      // Pixel 2
      if( xSrc<nWidth-1 )
      {
        pDst1[xSrc*2+1] = WORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
      }
      else
        pDst1[xSrc*2+1] = pSrc[xSrc];


      // Pixel 3
      if( ySrc<nHeight-1 )
      {
        pDst2[xSrc*2] = WORD_MAKE((r1+r3)/2, (g1+g3)/2, (b1+b3)/2, (a1+a3)/2);
      }
      else
        pDst2[xSrc*2] = pSrc[xSrc];

      // Pixel 4
      if( xSrc<nWidth-1 )
      {
        if( ySrc<nHeight-1 )
        {
          pDst2[xSrc*2+1] = WORD_MAKE((r1+r2+r3+r4)/4, (g1+g2+g3+g4)/4, (b1+b2+b3+b4)/4, (a1+a2+a3+a4)/4);
        }
        else
        {
          pDst2[xSrc*2+1] = WORD_MAKE((r1+r2)/2, (g1+g2)/2, (b1+b2)/2, (a1+a2)/2);
        }
      }
      else
      {
        if( ySrc<nHeight-1 )
        {
          pDst2[xSrc*2+1] = WORD_MAKE((r1+r3)/2, (g1+g3)/2, (b1+b3)/2, (a1+a3)/2);
        }
        else
          pDst2[xSrc*2+1] = pSrc[xSrc];
      }
    }
  }
}
#endif /* !_16BPP_HACK */

/*
 * Sharp filters
 * Hiroshi Morii <koolsmoky@users.sourceforge.net>
 */
void SharpFilter_8888(uint32 *src, uint32 srcwidth, uint32 srcheight, uint32 *dest, uint32 filter)
{
  // NOTE: for now we get away with copying the boundaries
  //       filter the boundaries if we face problems

  uint32 mul1, mul2, mul3, shift4;

  uint32 x,y,z;
  uint32 *_src1, *_src2, *_src3, *_dest;
  uint32 val[4];
  uint32 t1,t2,t3,t4,t5,t6,t7,t8,t9;

  switch( filter )
  {
  case SHARP_FILTER_2:
    mul1=1;
    mul2=8;
    mul3=12;
    shift4=2;
    break;
  case SHARP_FILTER_1:
  default:
    mul1=1;
    mul2=8;
    mul3=16;
    shift4=3;
    break;
  }

  // setup rows
  _src1 = src;
  _src2 = _src1 + srcwidth;
  _src3 = _src2 + srcwidth;
  _dest = dest;

  // copy the first row
  memcpy(_dest, _src1, (srcwidth << 2));
  _dest += srcwidth;
  // filter 2nd row to 1 row before the last
  for (y = 1; y < srcheight-1; y++) {
    // copy the first pixel
    _dest[0] = *_src2;
    // filter 2nd pixel to 1 pixel before last
    for (x = 1; x < srcwidth-1; x++) {
      for (z=0; z<4; z++) {
        t1 = *((uint8*)(_src1+x-1)+z);
        t2 = *((uint8*)(_src1+x  )+z);
        t3 = *((uint8*)(_src1+x+1)+z);
        t4 = *((uint8*)(_src2+x-1)+z);
        t5 = *((uint8*)(_src2+x  )+z);
        t6 = *((uint8*)(_src2+x+1)+z);
        t7 = *((uint8*)(_src3+x-1)+z);
        t8 = *((uint8*)(_src3+x  )+z);
        t9 = *((uint8*)(_src3+x+1)+z);
        
        if( (t5*mul2) > (t1+t3+t7+t9+t2+t4+t6+t8)*mul1 ) {
          val[z]= ((t5*mul3) - (t1+t3+t7+t9+t2+t4+t6+t8)*mul1)>>shift4;
          if (val[z] > 0xFF) val[z] = 0xFF;
        } else {
          val[z] = t5;
        }
      }
      _dest[x] = val[0]|(val[1]<<8)|(val[2]<<16)|(val[3]<<24);
    }
    // copy the ending pixel
    _dest[srcwidth-1] = *(_src3 - 1);
    // next row
    _src1 += srcwidth;
    _src2 += srcwidth;
    _src3 += srcwidth;
    _dest += srcwidth;
  }
  // copy the last row
  memcpy(_dest, _src2, (srcwidth << 2));
}

#if !_16BPP_HACK
void SharpFilter_4444(uint16 *src, uint32 srcwidth, uint32 srcheight, uint16 *dest, uint32 filter)
{
  // NOTE: for now we get away with copying the boundaries
  //       filter the boundaries if we face problems

  uint16 mul1, mul2, mul3, shift4;

  uint32 x,y,z;
  uint16 *_src1, *_src2, *_src3, *_dest;
  uint16 val[4];
  uint16 t1,t2,t3,t4,t5,t6,t7,t8,t9;

  switch( filter ) {
  case SHARP_FILTER_2:
    mul1=1;
    mul2=8;
    mul3=12;
    shift4=2;
    break;
  case SHARP_FILTER_1:
  default:
    mul1=1;
    mul2=8;
    mul3=16;
    shift4=3;
    break;
  }

  // setup rows
  _src1 = src;
  _src2 = _src1 + srcwidth;
  _src3 = _src2 + srcwidth;
  _dest = dest;

  // copy the first row
  memcpy(_dest, _src1, (srcwidth << 1));
  _dest += srcwidth;
  // filter 2nd row to 1 row before the last
  for( y = 1; y < srcheight - 1; y++) {
    // copy the first pixel
    _dest[0] = *_src2;
    // filter 2nd pixel to 1 pixel before last
    for( x = 1; x < srcwidth - 1; x++) {
      for( z = 0; z < 4; z++ ) {
        /* Hiroshi Morii <koolsmoky@users.sourceforge.net>
         * Read the entire 16bit pixel and then extract the A,R,G,B components.
         */
        uint32 shift = z << 2;
        t1 = ((*((uint16*)(_src1+x-1))) >> shift) & 0xF;
        t2 = ((*((uint16*)(_src1+x  ))) >> shift) & 0xF;
        t3 = ((*((uint16*)(_src1+x+1))) >> shift) & 0xF;
        t4 = ((*((uint16*)(_src2+x-1))) >> shift) & 0xF;
        t5 = ((*((uint16*)(_src2+x  ))) >> shift) & 0xF;
        t6 = ((*((uint16*)(_src2+x+1))) >> shift) & 0xF;
        t7 = ((*((uint16*)(_src3+x-1))) >> shift) & 0xF;
        t8 = ((*((uint16*)(_src3+x  ))) >> shift) & 0xF;
        t9 = ((*((uint16*)(_src3+x+1))) >> shift) & 0xF;
        
        if( (t5*mul2) > (t1+t3+t7+t9+t2+t4+t6+t8)*mul1 ) {
          val[z] = ((t5*mul3) - (t1+t3+t7+t9+t2+t4+t6+t8)*mul1)>>shift4;
          if (val[z] > 0xF) val[z] = 0xF;
        } else {
          val[z] = t5;
        }
      }
      _dest[x] = val[0]|(val[1]<<4)|(val[2]<<8)|(val[3]<<12);
    }
    // copy the ending pixel
    _dest[srcwidth-1] = *(_src3 - 1);
    // next row
    _src1 += srcwidth;
    _src2 += srcwidth;
    _src3 += srcwidth;
    _dest += srcwidth;
  }
  // copy the last row
  memcpy(_dest, _src2, (srcwidth << 1));
}
#endif /* !_16BPP_HACK */

/*
 * Smooth filters
 * Hiroshi Morii <koolsmoky@users.sourceforge.net>
 */
void SmoothFilter_8888(uint32 *src, uint32 srcwidth, uint32 srcheight, uint32 *dest, uint32 filter)
{
  // NOTE: for now we get away with copying the boundaries
  //       filter the boundaries if we face problems

  uint32 mul1, mul2, mul3, shift4;

  uint32 x,y,z;
  uint32 *_src1, *_src2, *_src3, *_dest;
  uint32 val[4];
  uint32 t1,t2,t3,t4,t5,t6,t7,t8,t9;

  switch( filter ) {
  case SMOOTH_FILTER_4:
    mul1=1;
    mul2=2;
    mul3=4;
    shift4=4;
    break;
  case SMOOTH_FILTER_3:
    mul1=1;
    mul2=1;
    mul3=8;
    shift4=4;
    break;
  case SMOOTH_FILTER_2:
    mul1=1;
    mul2=1;
    mul3=2;
    shift4=2;
    break;
  case SMOOTH_FILTER_1:
  default:
    mul1=1;
    mul2=1;
    mul3=6;
    shift4=3;
    break;
  }

  switch (filter) {
  case SMOOTH_FILTER_3:
  case SMOOTH_FILTER_4:
    // setup rows
    _src1 = src;
    _src2 = _src1 + srcwidth;
    _src3 = _src2 + srcwidth;
    _dest = dest;
    // copy the first row
    memcpy(_dest, _src1, (srcwidth << 2));
    _dest += srcwidth;
    // filter 2nd row to 1 row before the last
    for (y = 1; y < srcheight - 1; y++){
      // copy the first pixel
      _dest[0] = _src2[0];
      // filter 2nd pixel to 1 pixel before last
      for (x = 1; x < srcwidth - 1; x++) {
        for (z = 0; z < 4; z++ ) {
          t1 = *((uint8*)(_src1+x-1)+z);
          t2 = *((uint8*)(_src1+x  )+z);
          t3 = *((uint8*)(_src1+x+1)+z);
          t4 = *((uint8*)(_src2+x-1)+z);
          t5 = *((uint8*)(_src2+x  )+z);
          t6 = *((uint8*)(_src2+x+1)+z);
          t7 = *((uint8*)(_src3+x-1)+z);
          t8 = *((uint8*)(_src3+x  )+z);
          t9 = *((uint8*)(_src3+x+1)+z);
          /* the component value must not overflow 0xFF */
          val[z] = ((t1+t3+t7+t9)*mul1+((t2+t4+t6+t8)*mul2)+(t5*mul3))>>shift4;
          if (val[z] > 0xFF) val[z] = 0xFF;
        }
        _dest[x] = val[0]|(val[1]<<8)|(val[2]<<16)|(val[3]<<24);
      }
      // copy the ending pixel
      _dest[srcwidth-1] = *(_src3 - 1);
      // next row
      _src1 += srcwidth;
      _src2 += srcwidth;
      _src3 += srcwidth;
      _dest += srcwidth;
    }
    // copy the last row
    memcpy(_dest, _src2, (srcwidth << 2));
    break;
  case SMOOTH_FILTER_1:
  case SMOOTH_FILTER_2:
  default:
    // setup rows
    _src1 = src;
    _src2 = _src1 + srcwidth;
    _src3 = _src2 + srcwidth;
    _dest = dest;
    // copy the first row
    memcpy(_dest, _src1, (srcwidth << 2));
    _dest += srcwidth;
    // filter 2nd row to 1 row before the last
    for (y = 1; y < srcheight - 1; y++) {
      // filter 1st pixel to the last
      if (y & 1) {
        for( x = 0; x < srcwidth; x++) {
          for( z = 0; z < 4; z++ ) {
            t2 = *((uint8*)(_src1+x  )+z);
            t5 = *((uint8*)(_src2+x  )+z);
            t8 = *((uint8*)(_src3+x  )+z);
            /* the component value must not overflow 0xFF */
            val[z] = ((t2+t8)*mul2+(t5*mul3))>>shift4;
            if (val[z] > 0xFF) val[z] = 0xFF;
          }
          _dest[x] = val[0]|(val[1]<<8)|(val[2]<<16)|(val[3]<<24);
        }
      } else {
         memcpy(_dest, _src2, (srcwidth << 2));
      }
      // next row
      _src1 += srcwidth;
      _src2 += srcwidth;
      _src3 += srcwidth;
      _dest += srcwidth;
    }
    // copy the last row
    memcpy(_dest, _src2, (srcwidth << 2));
    break;
  }
}

#if !_16BPP_HACK
void SmoothFilter_4444(uint16 *src, uint32 srcwidth, uint32 srcheight, uint16 *dest, uint32 filter)
{
  // NOTE: for now we get away with copying the boundaries
  //       filter the boundaries if we face problems

  uint16 mul1, mul2, mul3, shift4;

  uint32 x,y,z;
  uint16 *_src1, *_src2, *_src3, *_dest;
  uint16 val[4];
  uint16 t1,t2,t3,t4,t5,t6,t7,t8,t9;

  switch( filter ) {
  case SMOOTH_FILTER_4:
    mul1=1;
    mul2=2;
    mul3=4;
    shift4=4;
    break;
  case SMOOTH_FILTER_3:
    mul1=1;
    mul2=1;
    mul3=8;
    shift4=4;
    break;
  case SMOOTH_FILTER_2:
    mul1=1;
    mul2=1;
    mul3=2;
    shift4=2;
    break;
  case SMOOTH_FILTER_1:
  default:
    mul1=1;
    mul2=1;
    mul3=6;
    shift4=3;
    break;
  }

  switch (filter) {
  case SMOOTH_FILTER_3:
  case SMOOTH_FILTER_4:
    // setup rows
    _src1 = src;
    _src2 = _src1 + srcwidth;
    _src3 = _src2 + srcwidth;
    _dest = dest;
    // copy the first row
    memcpy(_dest, _src1, (srcwidth << 1));
    _dest += srcwidth;
    // filter 2nd row to 1 row before the last
    for (y = 1; y < srcheight - 1; y++) {
      // copy the first pixel
      _dest[0] = *_src2;
      // filter 2nd pixel to 1 pixel before last
      for (x = 1; x < srcwidth - 1; x++) {
        for (z = 0; z < 4; z++ ) {
          /* Read the entire 16bit pixel and then extract the A,R,G,B components. */
          uint32 shift = z << 2;
          t1 = ((*(uint16*)(_src1+x-1)) >> shift) & 0xF;
          t2 = ((*(uint16*)(_src1+x  )) >> shift) & 0xF;
          t3 = ((*(uint16*)(_src1+x+1)) >> shift) & 0xF;
          t4 = ((*(uint16*)(_src2+x-1)) >> shift) & 0xF;
          t5 = ((*(uint16*)(_src2+x  )) >> shift) & 0xF;
          t6 = ((*(uint16*)(_src2+x+1)) >> shift) & 0xF;
          t7 = ((*(uint16*)(_src3+x-1)) >> shift) & 0xF;
          t8 = ((*(uint16*)(_src3+x  )) >> shift) & 0xF;
          t9 = ((*(uint16*)(_src3+x+1)) >> shift) & 0xF;
          /* the component value must not overflow 0xF */
          val[z] = ((t1+t3+t7+t9)*mul1+((t2+t4+t6+t8)*mul2)+(t5*mul3))>>shift4;
          if (val[z] > 0xF) val[z] = 0xF;
        }
        _dest[x] = val[0]|(val[1]<<4)|(val[2]<<8)|(val[3]<<12);
      }
      // copy the ending pixel
      _dest[srcwidth-1] = *(_src3 - 1);
      // next row
      _src1 += srcwidth;
      _src2 += srcwidth;
      _src3 += srcwidth;
      _dest += srcwidth;
    }
    // copy the last row
    memcpy(_dest, _src2, (srcwidth << 1));
    break;
  case SMOOTH_FILTER_1:
  case SMOOTH_FILTER_2:
  default:
    // setup rows
    _src1 = src;
    _src2 = _src1 + srcwidth;
    _src3 = _src2 + srcwidth;
    _dest = dest;
    // copy the first row
    memcpy(_dest, _src1, (srcwidth << 1));
    _dest += srcwidth;
    // filter 2nd row to 1 row before the last
    for( y = 1; y < srcheight - 1; y++) {
      if (y & 1) {
        for( x = 0; x < srcwidth; x++) {
          for( z = 0; z < 4; z++ ) {
            /* Read the entire 16bit pixel and then extract the A,R,G,B components. */
            uint32 shift = z << 2;
            t2 = ((*(uint16*)(_src1+x)) >> shift) & 0xF;
            t5 = ((*(uint16*)(_src2+x)) >> shift) & 0xF;
            t8 = ((*(uint16*)(_src3+x)) >> shift) & 0xF;
            /* the component value must not overflow 0xF */
            val[z] = ((t2+t8)*mul2+(t5*mul3))>>shift4;
            if (val[z] > 0xF) val[z] = 0xF;
          }
          _dest[x] = val[0]|(val[1]<<4)|(val[2]<<8)|(val[3]<<12);
        }
      } else {
         memcpy(_dest, _src2, (srcwidth << 1));
      }
      // next row
      _src1 += srcwidth;
      _src2 += srcwidth;
      _src3 += srcwidth;
      _dest += srcwidth;
    }
    // copy the last row
    memcpy(_dest, _src2, (srcwidth << 1));
    break;
  }
}
#endif /* !_16BPP_HACK */

void filter_8888(uint32 *src, uint32 srcwidth, uint32 srcheight, uint32 *dest, uint32 filter) {
  switch (filter & ENHANCEMENT_MASK) {
  case HQ4X_ENHANCEMENT:
    hq4x_8888((uint8*)src, (uint8*)dest, srcwidth, srcheight, srcwidth, (srcwidth << 4));
    return;
  case HQ2X_ENHANCEMENT:
    hq2x_32((uint8*)src, (srcwidth << 2), (uint8*)dest, (srcwidth << 3), srcwidth, srcheight);
    return;
  case HQ2XS_ENHANCEMENT:
    hq2xS_32((uint8*)src, (srcwidth << 2), (uint8*)dest, (srcwidth << 3), srcwidth, srcheight);
    return;
  case LQ2X_ENHANCEMENT:
    lq2x_32((uint8*)src, (srcwidth << 2), (uint8*)dest, (srcwidth << 3), srcwidth, srcheight);
    return;
  case LQ2XS_ENHANCEMENT:
    lq2xS_32((uint8*)src, (srcwidth << 2), (uint8*)dest, (srcwidth << 3), srcwidth, srcheight);
    return;
  case X2SAI_ENHANCEMENT:
    Super2xSaI_8888((uint32*)src, (uint32*)dest, srcwidth, srcheight, srcwidth);
    return;
  case X2_ENHANCEMENT:
    Texture2x_32((uint8*)src, (srcwidth << 2), (uint8*)dest, (srcwidth << 3), srcwidth, srcheight);
    return;
  }

  switch (filter & (SMOOTH_FILTER_MASK|SHARP_FILTER_MASK)) {
  case SMOOTH_FILTER_1:
  case SMOOTH_FILTER_2:
  case SMOOTH_FILTER_3:
  case SMOOTH_FILTER_4:
    SmoothFilter_8888((uint32*)src, srcwidth, srcheight, (uint32*)dest, (filter & SMOOTH_FILTER_MASK));
    return;
  case SHARP_FILTER_1:
  case SHARP_FILTER_2:
    SharpFilter_8888((uint32*)src, srcwidth, srcheight, (uint32*)dest, (filter & SHARP_FILTER_MASK));
    return;
  }
}
