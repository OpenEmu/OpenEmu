/*
 * Texture Filtering
 * Version:  1.0
 *
 * Copyright (C) 2007  Hiroshi Morii   All Rights Reserved.
 * Email koolsmoky(at)users.sourceforge.net
 * Web   http://www.3dfxzone.it/koolsmoky
 *
 * this is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * this is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Make; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Based on Derek Liauw Kie Fa and Rice1964 Super2xSaI code */

#include "TextureFilters.h"

#define GET_RESULT(A, B, C, D) ((A != C || A != D) - (B != C || B != D))

void Super2xSaI_8888(uint32 *srcPtr, uint32 *destPtr, uint32 width, uint32 height, uint32 pitch)
{
#define SAI_INTERPOLATE_8888(A, B) ((A & 0xFEFEFEFE) >> 1) + ((B & 0xFEFEFEFE) >> 1) + (A & B & 0x01010101)
#define SAI_Q_INTERPOLATE_8888(A, B, C, D) ((A & 0xFCFCFCFC) >> 2) + ((B & 0xFCFCFCFC) >> 2) + ((C & 0xFCFCFCFC) >> 2) + ((D & 0xFCFCFCFC) >> 2) \
  + ((((A & 0x03030303) + (B & 0x03030303) + (C & 0x03030303) + (D & 0x03030303)) >> 2) & 0x03030303)

#define SAI_INTERPOLATE SAI_INTERPOLATE_8888
#define SAI_Q_INTERPOLATE SAI_Q_INTERPOLATE_8888

  uint32 destWidth = width << 1;
  uint32 destHeight = height << 1;
  
  uint32 color4, color5, color6;
  uint32 color1, color2, color3;
  uint32 colorA0, colorA1, colorA2, colorA3;
  uint32 colorB0, colorB1, colorB2, colorB3;
  uint32 colorS1, colorS2;
  uint32 product1a, product1b, product2a, product2b;

#include "TextureFilters_2xsai.h"

#undef SAI_INTERPOLATE
#undef SAI_Q_INTERPOLATE
}

#if !_16BPP_HACK
void Super2xSaI_4444(uint16 *srcPtr, uint16 *destPtr, uint32 width, uint32 height, uint32 pitch)
{
#define SAI_INTERPOLATE_4444(A, B) ((A & 0xEEEE) >> 1) + ((B & 0xEEEE) >> 1) + (A & B & 0x1111)
#define SAI_Q_INTERPOLATE_4444(A, B, C, D) ((A & 0xCCCC) >> 2) + ((B & 0xCCCC) >> 2) + ((C & 0xCCCC) >> 2) + ((D & 0xCCCC) >> 2) \
  + ((((A & 0x3333) + (B & 0x3333) + (C & 0x3333) + (D & 0x3333)) >> 2) & 0x3333)

#define SAI_INTERPOLATE SAI_INTERPOLATE_4444
#define SAI_Q_INTERPOLATE SAI_Q_INTERPOLATE_4444

  uint32 destWidth = width << 1;
  uint32 destHeight = height << 1;

  uint16 color4, color5, color6;
  uint16 color1, color2, color3;
  uint16 colorA0, colorA1, colorA2, colorA3;
  uint16 colorB0, colorB1, colorB2, colorB3;
  uint16 colorS1, colorS2;
  uint16 product1a, product1b, product2a, product2b;

#include "TextureFilters_2xsai.h"

#undef SAI_INTERPOLATE
#undef SAI_Q_INTERPOLATE
}

void Super2xSaI_1555(uint16 *srcPtr, uint16 *destPtr, uint32 width, uint32 height, uint32 pitch)
{
#define SAI_INTERPOLATE_1555(A, B) ((A & 0x7BDE) >> 1) + ((B & 0x7BDE) >> 1) + (A & B & 0x8421)
#define SAI_Q_INTERPOLATE_1555(A, B, C, D) ((A & 0x739C) >> 2) + ((B & 0x739C) >> 2) + ((C & 0x739C) >> 2) + ((D & 0x739C) >> 2) \
  + ((((A & 0x8C63) + (B & 0x8C63) + (C & 0x8C63) + (D & 0x8C63)) >> 2) & 0x8C63)

#define SAI_INTERPOLATE SAI_INTERPOLATE_1555
#define SAI_Q_INTERPOLATE SAI_Q_INTERPOLATE_1555

  uint32 destWidth = width << 1;
  uint32 destHeight = height << 1;

  uint16 color4, color5, color6;
  uint16 color1, color2, color3;
  uint16 colorA0, colorA1, colorA2, colorA3;
  uint16 colorB0, colorB1, colorB2, colorB3;
  uint16 colorS1, colorS2;
  uint16 product1a, product1b, product2a, product2b;

#include "TextureFilters_2xsai.h"

#undef SAI_INTERPOLATE
#undef SAI_Q_INTERPOLATE
}

void Super2xSaI_565(uint16 *srcPtr, uint16 *destPtr, uint32 width, uint32 height, uint32 pitch)
{
#define SAI_INTERPOLATE_565(A, B) ((A & 0xF7DE) >> 1) + ((B & 0xF7DE) >> 1) + (A & B & 0x0821)
#define SAI_Q_INTERPOLATE_565(A, B, C, D) ((A & 0xE79C) >> 2) + ((B & 0xE79C) >> 2) + ((C & 0xE79C) >> 2) + ((D & 0xE79C) >> 2) \
  + ((((A & 0x1863) + (B & 0x1863) + (C & 0x1863) + (D & 0x1863)) >> 2) & 0x1863)

#define SAI_INTERPOLATE SAI_INTERPOLATE_565
#define SAI_Q_INTERPOLATE SAI_Q_INTERPOLATE_565

  uint32 destWidth = width << 1;
  uint32 destHeight = height << 1;

  uint16 color4, color5, color6;
  uint16 color1, color2, color3;
  uint16 colorA0, colorA1, colorA2, colorA3;
  uint16 colorB0, colorB1, colorB2, colorB3;
  uint16 colorS1, colorS2;
  uint16 product1a, product1b, product2a, product2b;

#include "TextureFilters_2xsai.h"

#undef SAI_INTERPOLATE
#undef SAI_Q_INTERPOLATE
}

void Super2xSaI_8(uint8 *srcPtr, uint8 *destPtr, uint32 width, uint32 height, uint32 pitch)
{
#define SAI_INTERPOLATE_8(A, B) ((A & 0xFE) >> 1) + ((B & 0xFE) >> 1) + (A & B & 0x01)
#define SAI_Q_INTERPOLATE_8(A, B, C, D) ((A & 0xFC) >> 2) + ((B & 0xFC) >> 2) + ((C & 0xFC) >> 2) + ((D & 0xFC) >> 2) \
  + ((((A & 0x03) + (B & 0x03) + (C & 0x03) + (D & 0x03)) >> 2) & 0x03)

#define SAI_INTERPOLATE SAI_INTERPOLATE_8
#define SAI_Q_INTERPOLATE SAI_Q_INTERPOLATE_8

  uint32 destWidth = width << 1;
  uint32 destHeight = height << 1;

  uint8 color4, color5, color6;
  uint8 color1, color2, color3;
  uint8 colorA0, colorA1, colorA2, colorA3;
  uint8 colorB0, colorB1, colorB2, colorB3;
  uint8 colorS1, colorS2;
  uint8 product1a, product1b, product2a, product2b;

#include "TextureFilters_2xsai.h"

#undef SAI_INTERPOLATE
#undef SAI_Q_INTERPOLATE
}
#endif /* !_16BPP_HACK */
