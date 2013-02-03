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

/* 2007 Mudlord - Added hq2xS lq2xS filters */

#include "TextureFilters.h"

/************************************************************************/
/* hq2x filters                                                         */
/************************************************************************/

/***************************************************************************/
/* Basic types */

/***************************************************************************/
/* interpolation */

//static unsigned interp_bits_per_pixel;

#if !_16BPP_HACK
#define INTERP_16_MASK_1_3(v) ((v)&0x0F0F)
#define INTERP_16_MASK_SHIFT_2_4(v) (((v)&0xF0F0)>>4)
#define INTERP_16_MASK_SHIFTBACK_2_4(v) ((INTERP_16_MASK_1_3(v))<<4)

static uint16 hq2x_interp_16_521(uint16 p1, uint16 p2, uint16 p3)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*5 + INTERP_16_MASK_1_3(p2)*2 + INTERP_16_MASK_1_3(p3)*1) / 8)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*5 + INTERP_16_MASK_SHIFT_2_4(p2)*2 + INTERP_16_MASK_SHIFT_2_4(p3)*1) / 8);
}

static uint16 hq2x_interp_16_332(uint16 p1, uint16 p2, uint16 p3)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*3 + INTERP_16_MASK_1_3(p2)*3 + INTERP_16_MASK_1_3(p3)*2) / 8)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*3 + INTERP_16_MASK_SHIFT_2_4(p2)*3 + INTERP_16_MASK_SHIFT_2_4(p3)*2) / 8);
}

static uint16 hq2x_interp_16_611(uint16 p1, uint16 p2, uint16 p3)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*6 + INTERP_16_MASK_1_3(p2) + INTERP_16_MASK_1_3(p3)) / 8)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*6 + INTERP_16_MASK_SHIFT_2_4(p2) + INTERP_16_MASK_SHIFT_2_4(p3)) / 8);
}

static uint16 hq2x_interp_16_71(uint16 p1, uint16 p2)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*7 + INTERP_16_MASK_1_3(p2)) / 8)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*7 + INTERP_16_MASK_SHIFT_2_4(p2)) / 8);
}

static uint16 hq2x_interp_16_211(uint16 p1, uint16 p2, uint16 p3)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*2 + INTERP_16_MASK_1_3(p2) + INTERP_16_MASK_1_3(p3)) / 4)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*2 + INTERP_16_MASK_SHIFT_2_4(p2) + INTERP_16_MASK_SHIFT_2_4(p3)) / 4);
}

static uint16 hq2x_interp_16_772(uint16 p1, uint16 p2, uint16 p3)
{
  return INTERP_16_MASK_1_3(((INTERP_16_MASK_1_3(p1) + INTERP_16_MASK_1_3(p2))*7 + INTERP_16_MASK_1_3(p3)*2) / 16)
    | INTERP_16_MASK_SHIFTBACK_2_4(((INTERP_16_MASK_SHIFT_2_4(p1) + INTERP_16_MASK_SHIFT_2_4(p2))*7 + INTERP_16_MASK_SHIFT_2_4(p3)*2) / 16);
}

static uint16 hq2x_interp_16_11(uint16 p1, uint16 p2)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1) + INTERP_16_MASK_1_3(p2)) / 2)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1) + INTERP_16_MASK_SHIFT_2_4(p2)) / 2);
}

static uint16 hq2x_interp_16_31(uint16 p1, uint16 p2)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*3 + INTERP_16_MASK_1_3(p2)) / 4)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*3 + INTERP_16_MASK_SHIFT_2_4(p2)) / 4);
}

static uint16 hq2x_interp_16_1411(uint16 p1, uint16 p2, uint16 p3)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*14 + INTERP_16_MASK_1_3(p2) + INTERP_16_MASK_1_3(p3)) / 16)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*14 + INTERP_16_MASK_SHIFT_2_4(p2) + INTERP_16_MASK_SHIFT_2_4(p3)) / 16);
}

static uint16 hq2x_interp_16_431(uint16 p1, uint16 p2, uint16 p3)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*4 + INTERP_16_MASK_1_3(p2)*3 + INTERP_16_MASK_1_3(p3)) / 8)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*4 + INTERP_16_MASK_SHIFT_2_4(p2)*3 + INTERP_16_MASK_SHIFT_2_4(p3)) / 8);
}

static uint16 hq2x_interp_16_53(uint16 p1, uint16 p2)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*5 + INTERP_16_MASK_1_3(p2)*3) / 8)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*5 + INTERP_16_MASK_SHIFT_2_4(p2)*3) / 8);
}

static uint16 hq2x_interp_16_151(uint16 p1, uint16 p2)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*15 + INTERP_16_MASK_1_3(p2)) / 16)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*15 + INTERP_16_MASK_SHIFT_2_4(p2)) / 16);
}

static uint16 hq2x_interp_16_97(uint16 p1, uint16 p2)
{
  return INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*9 + INTERP_16_MASK_1_3(p2)*7) / 16)
    | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*9 + INTERP_16_MASK_SHIFT_2_4(p2)*7) / 16);
}
#endif /* !_16BPP_HACK */

#define INTERP_32_MASK_1_3(v) ((v)&0x00FF00FF)
#define INTERP_32_MASK_SHIFT_2_4(v) (((v)&0xFF00FF00)>>8)
#define INTERP_32_MASK_SHIFTBACK_2_4(v) (((INTERP_32_MASK_1_3(v))<<8))

static uint32 hq2x_interp_32_521(uint32 p1, uint32 p2, uint32 p3)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*5 + INTERP_32_MASK_1_3(p2)*2 + INTERP_32_MASK_1_3(p3)*1) / 8)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*5 + INTERP_32_MASK_SHIFT_2_4(p2)*2 + INTERP_32_MASK_SHIFT_2_4(p3)*1) / 8);
}

static uint32 hq2x_interp_32_332(uint32 p1, uint32 p2, uint32 p3)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*3 + INTERP_32_MASK_1_3(p2)*3 + INTERP_32_MASK_1_3(p3)*2) / 8)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*3 + INTERP_32_MASK_SHIFT_2_4(p2)*3 + INTERP_32_MASK_SHIFT_2_4(p3)*2) / 8);
}

static uint32 hq2x_interp_32_211(uint32 p1, uint32 p2, uint32 p3)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*2 + INTERP_32_MASK_1_3(p2) + INTERP_32_MASK_1_3(p3)) / 4)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*2 + INTERP_32_MASK_SHIFT_2_4(p2) + INTERP_32_MASK_SHIFT_2_4(p3)) / 4);
}

static uint32 hq2x_interp_32_611(uint32 p1, uint32 p2, uint32 p3)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*6 + INTERP_32_MASK_1_3(p2) + INTERP_32_MASK_1_3(p3)) / 8)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*6 + INTERP_32_MASK_SHIFT_2_4(p2) + INTERP_32_MASK_SHIFT_2_4(p3)) / 8);
}

static uint32 hq2x_interp_32_71(uint32 p1, uint32 p2)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*7 + INTERP_32_MASK_1_3(p2)) / 8)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*7 + INTERP_32_MASK_SHIFT_2_4(p2)) / 8);
}

static uint32 hq2x_interp_32_772(uint32 p1, uint32 p2, uint32 p3)
{
  return INTERP_32_MASK_1_3(((INTERP_32_MASK_1_3(p1) + INTERP_32_MASK_1_3(p2))*7 + INTERP_32_MASK_1_3(p3)*2) / 16)
    | INTERP_32_MASK_SHIFTBACK_2_4(((INTERP_32_MASK_SHIFT_2_4(p1) + INTERP_32_MASK_SHIFT_2_4(p2))*7 + INTERP_32_MASK_SHIFT_2_4(p3)*2) / 16);
}

static uint32 hq2x_interp_32_11(uint32 p1, uint32 p2)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1) + INTERP_32_MASK_1_3(p2)) / 2)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1) + INTERP_32_MASK_SHIFT_2_4(p2)) / 2);
}

static uint32 hq2x_interp_32_31(uint32 p1, uint32 p2)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*3 + INTERP_32_MASK_1_3(p2)) / 4)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*3 + INTERP_32_MASK_SHIFT_2_4(p2)) / 4);
}

static uint32 hq2x_interp_32_1411(uint32 p1, uint32 p2, uint32 p3)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*14 + INTERP_32_MASK_1_3(p2) + INTERP_32_MASK_1_3(p3)) / 16)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*14 + INTERP_32_MASK_SHIFT_2_4(p2) + INTERP_32_MASK_SHIFT_2_4(p3)) / 16);
}

static uint32 hq2x_interp_32_431(uint32 p1, uint32 p2, uint32 p3)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*4 + INTERP_32_MASK_1_3(p2)*3 + INTERP_32_MASK_1_3(p3)) / 8)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*4 + INTERP_32_MASK_SHIFT_2_4(p2)*3 + INTERP_32_MASK_SHIFT_2_4(p3)) / 8);
}

static uint32 hq2x_interp_32_53(uint32 p1, uint32 p2)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*5 + INTERP_32_MASK_1_3(p2)*3) / 8)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*5 + INTERP_32_MASK_SHIFT_2_4(p2)*3) / 8);
}

static uint32 hq2x_interp_32_151(uint32 p1, uint32 p2)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*15 + INTERP_32_MASK_1_3(p2)) / 16)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*15 + INTERP_32_MASK_SHIFT_2_4(p2)) / 16);
}

static uint32 hq2x_interp_32_97(uint32 p1, uint32 p2)
{
  return INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*9 + INTERP_32_MASK_1_3(p2)*7) / 16)
    | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*9 + INTERP_32_MASK_SHIFT_2_4(p2)*7) / 16);
}

/***************************************************************************/
/* diff */

#define INTERP_Y_LIMIT (0x30*4)
#define INTERP_U_LIMIT (0x07*4)
#define INTERP_V_LIMIT (0x06*8)

#if !_16BPP_HACK
static int hq2x_interp_16_diff(uint16 p1, uint16 p2)
{
  int r, g, b;
  int y, u, v;

  if (p1 == p2)
    return 0;

  b = (int)((p1 & 0x000F) - (p2 & 0x000F));
  g = (int)((p1 & 0x00F0) - (p2 & 0x00F0)) >> 4;
  r = (int)((p1 & 0x0F00) - (p2 & 0x0F00)) >> 8;

  y = r + g + b;
  u = r - b;
  v = -r + 2*g - b;

  if (y < -INTERP_Y_LIMIT || y > INTERP_Y_LIMIT)
    return 1;

  if (u < -INTERP_U_LIMIT || u > INTERP_U_LIMIT)
    return 1;

  if (v < -INTERP_V_LIMIT || v > INTERP_V_LIMIT)
    return 1;

  return 0;
}
#endif /* !_16BPP_HACK */

static int hq2x_interp_32_diff(uint32 p1, uint32 p2)
{
  int r, g, b;
  int y, u, v;

  if ((p1 & 0xF8F8F8) == (p2 & 0xF8F8F8))
    return 0;

  b = (int)((p1 & 0xFF) - (p2 & 0xFF));
  g = (int)((p1 & 0xFF00) - (p2 & 0xFF00)) >> 8;
  r = (int)((p1 & 0xFF0000) - (p2 & 0xFF0000)) >> 16;

  y = r + g + b;
  u = r - b;
  v = -r + 2*g - b;

  if (y < -INTERP_Y_LIMIT || y > INTERP_Y_LIMIT)
    return 1;

  if (u < -INTERP_U_LIMIT || u > INTERP_U_LIMIT)
    return 1;

  if (v < -INTERP_V_LIMIT || v > INTERP_V_LIMIT)
    return 1;

  return 0;
}

/*static void interp_set(unsigned bits_per_pixel)
{
   interp_bits_per_pixel = bits_per_pixel;
}*/

#if !_16BPP_HACK
static void hq2x_16_def(uint16* dst0, uint16* dst1, const uint16* src0, const uint16* src1, const uint16* src2, unsigned count)
{
  unsigned i;

  for(i=0;i<count;++i) {
    unsigned char mask;

    uint16 c[9];

    c[1] = src0[0];
    c[4] = src1[0];
    c[7] = src2[0];

    if (i>0) {
      c[0] = src0[-1];
      c[3] = src1[-1];
      c[6] = src2[-1];
    } else {
      c[0] = c[1];
      c[3] = c[4];
      c[6] = c[7];
    }

    if (i<count-1) {
      c[2] = src0[1];
      c[5] = src1[1];
      c[8] = src2[1];
    } else {
      c[2] = c[1];
      c[5] = c[4];
      c[8] = c[7];
    }

    mask = 0;

    if (hq2x_interp_16_diff(c[0], c[4]))
      mask |= 1 << 0;
    if (hq2x_interp_16_diff(c[1], c[4]))
      mask |= 1 << 1;
    if (hq2x_interp_16_diff(c[2], c[4]))
      mask |= 1 << 2;
    if (hq2x_interp_16_diff(c[3], c[4]))
      mask |= 1 << 3;
    if (hq2x_interp_16_diff(c[5], c[4]))
      mask |= 1 << 4;
    if (hq2x_interp_16_diff(c[6], c[4]))
      mask |= 1 << 5;
    if (hq2x_interp_16_diff(c[7], c[4]))
      mask |= 1 << 6;
    if (hq2x_interp_16_diff(c[8], c[4]))
      mask |= 1 << 7;

#define P0 dst0[0]
#define P1 dst0[1]
#define P2 dst1[0]
#define P3 dst1[1]
#define HQ2X_MUR hq2x_interp_16_diff(c[1], c[5])
#define HQ2X_MDR hq2x_interp_16_diff(c[5], c[7])
#define HQ2X_MDL hq2x_interp_16_diff(c[7], c[3])
#define HQ2X_MUL hq2x_interp_16_diff(c[3], c[1])
#define IC(p0) c[p0]
#define I11(p0,p1) hq2x_interp_16_11(c[p0], c[p1])
#define I211(p0,p1,p2) hq2x_interp_16_211(c[p0], c[p1], c[p2])
#define I31(p0,p1) hq2x_interp_16_31(c[p0], c[p1])
#define I332(p0,p1,p2) hq2x_interp_16_332(c[p0], c[p1], c[p2])
#define I431(p0,p1,p2) hq2x_interp_16_431(c[p0], c[p1], c[p2])
#define I521(p0,p1,p2) hq2x_interp_16_521(c[p0], c[p1], c[p2])
#define I53(p0,p1) hq2x_interp_16_53(c[p0], c[p1])
#define I611(p0,p1,p2) hq2x_interp_16_611(c[p0], c[p1], c[p2])
#define I71(p0,p1) hq2x_interp_16_71(c[p0], c[p1])
#define I772(p0,p1,p2) hq2x_interp_16_772(c[p0], c[p1], c[p2])
#define I97(p0,p1) hq2x_interp_16_97(c[p0], c[p1])
#define I1411(p0,p1,p2) hq2x_interp_16_1411(c[p0], c[p1], c[p2])
#define I151(p0,p1) hq2x_interp_16_151(c[p0], c[p1])

    switch (mask) {
#include "TextureFilters_hq2x.h"
    }

#undef P0
#undef P1
#undef P2
#undef P3
#undef HQ2X_MUR
#undef HQ2X_MDR
#undef HQ2X_MDL
#undef HQ2X_MUL
#undef IC
#undef I11
#undef I211
#undef I31
#undef I332
#undef I431
#undef I521
#undef I53
#undef I611
#undef I71
#undef I772
#undef I97
#undef I1411
#undef I151

    src0 += 1;
    src1 += 1;
    src2 += 1;
    dst0 += 2;
    dst1 += 2;
  }
}

static void hq2xS_16_def(uint16* dst0, uint16* dst1, const uint16* src0, const uint16* src1, const uint16* src2, unsigned count)
{
  unsigned i;

  for(i=0;i<count;++i) {
    unsigned char mask;

    uint16 c[9];

    c[1] = src0[0];
    c[4] = src1[0];
    c[7] = src2[0];

    if (i>0) {
      c[0] = src0[-1];
      c[3] = src1[-1];
      c[6] = src2[-1];
    } else {
      c[0] = c[1];
      c[3] = c[4];
      c[6] = c[7];
    }

    if (i<count-1) {
      c[2] = src0[1];
      c[5] = src1[1];
      c[8] = src2[1];
    } else {
      c[2] = c[1];
      c[5] = c[4];
      c[8] = c[7];
    }

    mask = 0;

    // hq2xS dynamic edge detection:
    // simply comparing the center color against its surroundings will give bad results in many cases,
    // so, instead, compare the center color relative to the max difference in brightness of this 3x3 block
    int brightArray[9];
    int maxBright = 0, minBright = 999999;
    for(int j = 0 ; j < 9 ; j++) {
      int r,g,b;
      if (interp_bits_per_pixel == 16) {
        b = (int)((c[j] & 0x1F)) << 3;
        g = (int)((c[j] & 0x7E0)) >> 3;
        r = (int)((c[j] & 0xF800)) >> 8;
      } else {
        b = (int)((c[j] & 0x1F)) << 3;
        g = (int)((c[j] & 0x3E0)) >> 2;
        r = (int)((c[j] & 0x7C00)) >> 7;
      }
      const int bright = r+r+r + g+g+g + b+b;
      if(bright > maxBright) maxBright = bright;
      if(bright < minBright) minBright = bright;

      brightArray[j] = bright;
    }
    int diffBright = ((maxBright - minBright) * 7) >> 4;
    if(diffBright > 7) {
#define ABS(x) ((x) < 0 ? -(x) : (x))

      const int centerBright = brightArray[4];
      if(ABS(brightArray[0] - centerBright) > diffBright)
        mask |= 1 << 0;
      if(ABS(brightArray[1] - centerBright) > diffBright)
        mask |= 1 << 1;
      if(ABS(brightArray[2] - centerBright) > diffBright)
        mask |= 1 << 2;
      if(ABS(brightArray[3] - centerBright) > diffBright)
        mask |= 1 << 3;
      if(ABS(brightArray[5] - centerBright) > diffBright)
        mask |= 1 << 4;
      if(ABS(brightArray[6] - centerBright) > diffBright)
        mask |= 1 << 5;
      if(ABS(brightArray[7] - centerBright) > diffBright)
        mask |= 1 << 6;
      if(ABS(brightArray[8] - centerBright) > diffBright)
        mask |= 1 << 7;
    }

#define P0 dst0[0]
#define P1 dst0[1]
#define P2 dst1[0]
#define P3 dst1[1]
#define HQ2X_MUR false
#define HQ2X_MDR false
#define HQ2X_MDL false
#define HQ2X_MUL false
#define IC(p0) c[p0]
#define I11(p0,p1) hq2x_interp_16_11(c[p0], c[p1])
#define I211(p0,p1,p2) hq2x_interp_16_211(c[p0], c[p1], c[p2])
#define I31(p0,p1) hq2x_interp_16_31(c[p0], c[p1])
#define I332(p0,p1,p2) hq2x_interp_16_332(c[p0], c[p1], c[p2])
#define I431(p0,p1,p2) hq2x_interp_16_431(c[p0], c[p1], c[p2])
#define I521(p0,p1,p2) hq2x_interp_16_521(c[p0], c[p1], c[p2])
#define I53(p0,p1) hq2x_interp_16_53(c[p0], c[p1])
#define I611(p0,p1,p2) hq2x_interp_16_611(c[p0], c[p1], c[p2])
#define I71(p0,p1) hq2x_interp_16_71(c[p0], c[p1])
#define I772(p0,p1,p2) hq2x_interp_16_772(c[p0], c[p1], c[p2])
#define I97(p0,p1) hq2x_interp_16_97(c[p0], c[p1])
#define I1411(p0,p1,p2) hq2x_interp_16_1411(c[p0], c[p1], c[p2])
#define I151(p0,p1) hq2x_interp_16_151(c[p0], c[p1])

    switch (mask) {
#include "TextureFilters_hq2x.h"
    }

#undef P0
#undef P1
#undef P2
#undef P3
#undef HQ2X_MUR
#undef HQ2X_MDR
#undef HQ2X_MDL
#undef HQ2X_MUL
#undef IC
#undef I11
#undef I211
#undef I31
#undef I332
#undef I431
#undef I521
#undef I53
#undef I611
#undef I71
#undef I772
#undef I97
#undef I1411
#undef I151

    src0 += 1;
    src1 += 1;
    src2 += 1;
    dst0 += 2;
    dst1 += 2;
  }
}
#endif /* !_16BPP_HACK */

static void hq2x_32_def(uint32* dst0, uint32* dst1, const uint32* src0, const uint32* src1, const uint32* src2, unsigned count)
{
  unsigned i;

  for(i=0;i<count;++i) {
    unsigned char mask;

    uint32 c[9];

    c[1] = src0[0];
    c[4] = src1[0];
    c[7] = src2[0];

    if (i>0) {
      c[0] = src0[-1];
      c[3] = src1[-1];
      c[6] = src2[-1];
    } else {
      c[0] = src0[0];
      c[3] = src1[0];
      c[6] = src2[0];
    }

    if (i<count-1) {
      c[2] = src0[1];
      c[5] = src1[1];
      c[8] = src2[1];
    } else {
      c[2] = src0[0];
      c[5] = src1[0];
      c[8] = src2[0];
    }

    mask = 0;

    if (hq2x_interp_32_diff(c[0], c[4]))
      mask |= 1 << 0;
    if (hq2x_interp_32_diff(c[1], c[4]))
      mask |= 1 << 1;
    if (hq2x_interp_32_diff(c[2], c[4]))
      mask |= 1 << 2;
    if (hq2x_interp_32_diff(c[3], c[4]))
      mask |= 1 << 3;
    if (hq2x_interp_32_diff(c[5], c[4]))
      mask |= 1 << 4;
    if (hq2x_interp_32_diff(c[6], c[4]))
      mask |= 1 << 5;
    if (hq2x_interp_32_diff(c[7], c[4]))
      mask |= 1 << 6;
    if (hq2x_interp_32_diff(c[8], c[4]))
      mask |= 1 << 7;

#define P0 dst0[0]
#define P1 dst0[1]
#define P2 dst1[0]
#define P3 dst1[1]
#define HQ2X_MUR hq2x_interp_32_diff(c[1], c[5])
#define HQ2X_MDR hq2x_interp_32_diff(c[5], c[7])
#define HQ2X_MDL hq2x_interp_32_diff(c[7], c[3])
#define HQ2X_MUL hq2x_interp_32_diff(c[3], c[1])
#define IC(p0) c[p0]
#define I11(p0,p1) hq2x_interp_32_11(c[p0], c[p1])
#define I211(p0,p1,p2) hq2x_interp_32_211(c[p0], c[p1], c[p2])
#define I31(p0,p1) hq2x_interp_32_31(c[p0], c[p1])
#define I332(p0,p1,p2) hq2x_interp_32_332(c[p0], c[p1], c[p2])
#define I431(p0,p1,p2) hq2x_interp_32_431(c[p0], c[p1], c[p2])
#define I521(p0,p1,p2) hq2x_interp_32_521(c[p0], c[p1], c[p2])
#define I53(p0,p1) hq2x_interp_32_53(c[p0], c[p1])
#define I611(p0,p1,p2) hq2x_interp_32_611(c[p0], c[p1], c[p2])
#define I71(p0,p1) hq2x_interp_32_71(c[p0], c[p1])
#define I772(p0,p1,p2) hq2x_interp_32_772(c[p0], c[p1], c[p2])
#define I97(p0,p1) hq2x_interp_32_97(c[p0], c[p1])
#define I1411(p0,p1,p2) hq2x_interp_32_1411(c[p0], c[p1], c[p2])
#define I151(p0,p1) hq2x_interp_32_151(c[p0], c[p1])

    switch (mask) {
#include "TextureFilters_hq2x.h"
    }

#undef P0
#undef P1
#undef P2
#undef P3
#undef HQ2X_MUR
#undef HQ2X_MDR
#undef HQ2X_MDL
#undef HQ2X_MUL
#undef IC
#undef I11
#undef I211
#undef I31
#undef I332
#undef I431
#undef I521
#undef I53
#undef I611
#undef I71
#undef I772
#undef I97
#undef I1411
#undef I151

    src0 += 1;
    src1 += 1;
    src2 += 1;
    dst0 += 2;
    dst1 += 2;
  }
}

static void hq2xS_32_def(uint32* dst0, uint32* dst1, const uint32* src0, const uint32* src1, const uint32* src2, unsigned count)
{
  unsigned i;

  for(i=0;i<count;++i) {
    unsigned char mask;

    uint32 c[9];

    c[1] = src0[0];
    c[4] = src1[0];
    c[7] = src2[0];

    if (i>0) {
      c[0] = src0[-1];
      c[3] = src1[-1];
      c[6] = src2[-1];
    } else {
      c[0] = src0[0];
      c[3] = src1[0];
      c[6] = src2[0];
    }

    if (i<count-1) {
      c[2] = src0[1];
      c[5] = src1[1];
      c[8] = src2[1];
    } else {
      c[2] = src0[0];
      c[5] = src1[0];
      c[8] = src2[0];
    }
    
    mask = 0;
    // hq2xS dynamic edge detection:
    // simply comparing the center color against its surroundings will give bad results in many cases,
    // so, instead, compare the center color relative to the max difference in brightness of this 3x3 block
    int brightArray[9];
    int maxBright = 0, minBright = 999999;
    for(int j = 0 ; j < 9 ; j++) {
      const int b = (int)((c[j] & 0xF8));
      const int g = (int)((c[j] & 0xF800)) >> 8;
      const int r = (int)((c[j] & 0xF80000)) >> 16;
      const int bright = r+r+r + g+g+g + b+b;
      if(bright > maxBright) maxBright = bright;
      if(bright < minBright) minBright = bright;

      brightArray[j] = bright;
    }
    int diffBright = ((maxBright - minBright) * 7) >> 4;
    if(diffBright > 7) {
#define ABS(x) ((x) < 0 ? -(x) : (x))

      const int centerBright = brightArray[4];
      if(ABS(brightArray[0] - centerBright) > diffBright)
        mask |= 1 << 0;
      if(ABS(brightArray[1] - centerBright) > diffBright)
        mask |= 1 << 1;
      if(ABS(brightArray[2] - centerBright) > diffBright)
        mask |= 1 << 2;
      if(ABS(brightArray[3] - centerBright) > diffBright)
        mask |= 1 << 3;
      if(ABS(brightArray[5] - centerBright) > diffBright)
        mask |= 1 << 4;
      if(ABS(brightArray[6] - centerBright) > diffBright)
        mask |= 1 << 5;
      if(ABS(brightArray[7] - centerBright) > diffBright)
        mask |= 1 << 6;
      if(ABS(brightArray[8] - centerBright) > diffBright)
        mask |= 1 << 7;
    }
#define P0 dst0[0]
#define P1 dst0[1]
#define P2 dst1[0]
#define P3 dst1[1]
#define HQ2X_MUR false
#define HQ2X_MDR false
#define HQ2X_MDL false
#define HQ2X_MUL false
#define IC(p0) c[p0]
#define I11(p0,p1) hq2x_interp_32_11(c[p0], c[p1])
#define I211(p0,p1,p2) hq2x_interp_32_211(c[p0], c[p1], c[p2])
#define I31(p0,p1) hq2x_interp_32_31(c[p0], c[p1])
#define I332(p0,p1,p2) hq2x_interp_32_332(c[p0], c[p1], c[p2])
#define I431(p0,p1,p2) hq2x_interp_32_431(c[p0], c[p1], c[p2])
#define I521(p0,p1,p2) hq2x_interp_32_521(c[p0], c[p1], c[p2])
#define I53(p0,p1) hq2x_interp_32_53(c[p0], c[p1])
#define I611(p0,p1,p2) hq2x_interp_32_611(c[p0], c[p1], c[p2])
#define I71(p0,p1) hq2x_interp_32_71(c[p0], c[p1])
#define I772(p0,p1,p2) hq2x_interp_32_772(c[p0], c[p1], c[p2])
#define I97(p0,p1) hq2x_interp_32_97(c[p0], c[p1])
#define I1411(p0,p1,p2) hq2x_interp_32_1411(c[p0], c[p1], c[p2])
#define I151(p0,p1) hq2x_interp_32_151(c[p0], c[p1])

    switch (mask) {
#include "TextureFilters_hq2x.h"
    }

#undef P0
#undef P1
#undef P2
#undef P3
#undef HQ2X_MUR
#undef HQ2X_MDR
#undef HQ2X_MDL
#undef HQ2X_MUL
#undef IC
#undef I11
#undef I211
#undef I31
#undef I332
#undef I431
#undef I521
#undef I53
#undef I611
#undef I71
#undef I772
#undef I97
#undef I1411
#undef I151

    src0 += 1;
    src1 += 1;
    src2 += 1;
    dst0 += 2;
    dst1 += 2;
  }
}

/***************************************************************************/
/* LQ2x C implementation */

/*
* This effect is derived from the hq2x effect made by Maxim Stepin
*/

#if !_16BPP_HACK
static void lq2x_16_def(uint16* dst0, uint16* dst1, const uint16* src0, const uint16* src1, const uint16* src2, unsigned count)
{
  unsigned i;

  for(i=0;i<count;++i) {
    unsigned char mask;

    uint16 c[9];

    c[1] = src0[0];
    c[4] = src1[0];
    c[7] = src2[0];

    if (i>0) {
      c[0] = src0[-1];
      c[3] = src1[-1];
      c[6] = src2[-1];
    } else {
      c[0] = c[1];
      c[3] = c[4];
      c[6] = c[7];
    }

    if (i<count-1) {
      c[2] = src0[1];
      c[5] = src1[1];
      c[8] = src2[1];
    } else {
      c[2] = c[1];
      c[5] = c[4];
      c[8] = c[7];
    }

    mask = 0;

    if (c[0] != c[4])
      mask |= 1 << 0;
    if (c[1] != c[4])
      mask |= 1 << 1;
    if (c[2] != c[4])
      mask |= 1 << 2;
    if (c[3] != c[4])
      mask |= 1 << 3;
    if (c[5] != c[4])
      mask |= 1 << 4;
    if (c[6] != c[4])
      mask |= 1 << 5;
    if (c[7] != c[4])
      mask |= 1 << 6;
    if (c[8] != c[4])
      mask |= 1 << 7;

#define P0 dst0[0]
#define P1 dst0[1]
#define P2 dst1[0]
#define P3 dst1[1]
#define HQ2X_MUR (c[1] != c[5])
#define HQ2X_MDR (c[5] != c[7])
#define HQ2X_MDL (c[7] != c[3])
#define HQ2X_MUL (c[3] != c[1])
#define IC(p0) c[p0]
#define I11(p0,p1) hq2x_interp_16_11(c[p0], c[p1])
#define I211(p0,p1,p2) hq2x_interp_16_211(c[p0], c[p1], c[p2])
#define I31(p0,p1) hq2x_interp_16_31(c[p0], c[p1])
#define I332(p0,p1,p2) hq2x_interp_16_332(c[p0], c[p1], c[p2])
#define I431(p0,p1,p2) hq2x_interp_16_431(c[p0], c[p1], c[p2])
#define I521(p0,p1,p2) hq2x_interp_16_521(c[p0], c[p1], c[p2])
#define I53(p0,p1) hq2x_interp_16_53(c[p0], c[p1])
#define I611(p0,p1,p2) hq2x_interp_16_611(c[p0], c[p1], c[p2])
#define I71(p0,p1) hq2x_interp_16_71(c[p0], c[p1])
#define I772(p0,p1,p2) hq2x_interp_16_772(c[p0], c[p1], c[p2])
#define I97(p0,p1) hq2x_interp_16_97(c[p0], c[p1])
#define I1411(p0,p1,p2) hq2x_interp_16_1411(c[p0], c[p1], c[p2])
#define I151(p0,p1) hq2x_interp_16_151(c[p0], c[p1])

    switch (mask) {
#include "TextureFilters_lq2x.h"
    }

#undef P0
#undef P1
#undef P2
#undef P3
#undef HQ2X_MUR
#undef HQ2X_MDR
#undef HQ2X_MDL
#undef HQ2X_MUL
#undef IC
#undef I11
#undef I211
#undef I31
#undef I332
#undef I431
#undef I521
#undef I53
#undef I611
#undef I71
#undef I772
#undef I97
#undef I1411
#undef I151

    src0 += 1;
    src1 += 1;
    src2 += 1;
    dst0 += 2;
    dst1 += 2;
  }
}

static void lq2xS_16_def(uint16* dst0, uint16* dst1, const uint16* src0, const uint16* src1, const uint16* src2, unsigned count)
{
  unsigned i;

  for(i=0;i<count;++i) {
    unsigned char mask;

    uint16 c[9];

    c[1] = src0[0];
    c[4] = src1[0];
    c[7] = src2[0];

    if (i>0) {
      c[0] = src0[-1];
      c[3] = src1[-1];
      c[6] = src2[-1];
    } else {
      c[0] = c[1];
      c[3] = c[4];
      c[6] = c[7];
    }

    if (i<count-1) {
      c[2] = src0[1];
      c[5] = src1[1];
      c[8] = src2[1];
    } else {
      c[2] = c[1];
      c[5] = c[4];
      c[8] = c[7];
    }

    // hq2xS dynamic edge detection:
    // simply comparing the center color against its surroundings will give bad results in many cases,
    // so, instead, compare the center color relative to the max difference in brightness of this 3x3 block
    int brightArray[9];
    int maxBright = 0, minBright = 999999;
    for(int j = 0 ; j < 9 ; j++) {
      const int b = (int)((c[j] & 0xF8));
      const int g = (int)((c[j] & 0xF800)) >> 8;
      const int r = (int)((c[j] & 0xF80000)) >> 16;
      const int bright = r+r+r + g+g+g + b+b;
      if(bright > maxBright) maxBright = bright;
      if(bright < minBright) minBright = bright;

      brightArray[j] = bright;
    }
    int diffBright = ((maxBright - minBright) * 7) >> 4;
    if(diffBright > 7) {
#define ABS(x) ((x) < 0 ? -(x) : (x))

      const int centerBright = brightArray[4];
      if(ABS(brightArray[0] - centerBright) > diffBright)
        mask |= 1 << 0;
      if(ABS(brightArray[1] - centerBright) > diffBright)
        mask |= 1 << 1;
      if(ABS(brightArray[2] - centerBright) > diffBright)
        mask |= 1 << 2;
      if(ABS(brightArray[3] - centerBright) > diffBright)
        mask |= 1 << 3;
      if(ABS(brightArray[5] - centerBright) > diffBright)
        mask |= 1 << 4;
      if(ABS(brightArray[6] - centerBright) > diffBright)
        mask |= 1 << 5;
      if(ABS(brightArray[7] - centerBright) > diffBright)
        mask |= 1 << 6;
      if(ABS(brightArray[8] - centerBright) > diffBright)
        mask |= 1 << 7;
    }

#define P0 dst0[0]
#define P1 dst0[1]
#define P2 dst1[0]
#define P3 dst1[1]
#define HQ2X_MUR false
#define HQ2X_MDR false
#define HQ2X_MDL false
#define HQ2X_MUL false
#define IC(p0) c[p0]
#define I11(p0,p1) hq2x_interp_16_11(c[p0], c[p1])
#define I211(p0,p1,p2) hq2x_interp_16_211(c[p0], c[p1], c[p2])
#define I31(p0,p1) hq2x_interp_16_31(c[p0], c[p1])
#define I332(p0,p1,p2) hq2x_interp_16_332(c[p0], c[p1], c[p2])
#define I431(p0,p1,p2) hq2x_interp_16_431(c[p0], c[p1], c[p2])
#define I521(p0,p1,p2) hq2x_interp_16_521(c[p0], c[p1], c[p2])
#define I53(p0,p1) hq2x_interp_16_53(c[p0], c[p1])
#define I611(p0,p1,p2) hq2x_interp_16_611(c[p0], c[p1], c[p2])
#define I71(p0,p1) hq2x_interp_16_71(c[p0], c[p1])
#define I772(p0,p1,p2) hq2x_interp_16_772(c[p0], c[p1], c[p2])
#define I97(p0,p1) hq2x_interp_16_97(c[p0], c[p1])
#define I1411(p0,p1,p2) hq2x_interp_16_1411(c[p0], c[p1], c[p2])
#define I151(p0,p1) hq2x_interp_16_151(c[p0], c[p1])

    switch (mask) {
#include "TextureFilters_lq2x.h"
    }

#undef P0
#undef P1
#undef P2
#undef P3
#undef HQ2X_MUR
#undef HQ2X_MDR
#undef HQ2X_MDL
#undef HQ2X_MUL
#undef IC
#undef I11
#undef I211
#undef I31
#undef I332
#undef I431
#undef I521
#undef I53
#undef I611
#undef I71
#undef I772
#undef I97
#undef I1411
#undef I151

    src0 += 1;
    src1 += 1;
    src2 += 1;
    dst0 += 2;
    dst1 += 2;
  }
}
#endif /* !_16BPP_HACK */

static void lq2x_32_def(uint32* dst0, uint32* dst1, const uint32* src0, const uint32* src1, const uint32* src2, unsigned count)
{
  unsigned i;

  for(i=0;i<count;++i) {
    unsigned char mask;

    uint32 c[9];

    c[1] = src0[0];
    c[4] = src1[0];
    c[7] = src2[0];

    if (i>0) {
      c[0] = src0[-1];
      c[3] = src1[-1];
      c[6] = src2[-1];
    } else {
      c[0] = c[1];
      c[3] = c[4];
      c[6] = c[7];
    }

    if (i<count-1) {
      c[2] = src0[1];
      c[5] = src1[1];
      c[8] = src2[1];
    } else {
      c[2] = c[1];
      c[5] = c[4];
      c[8] = c[7];
    }

    mask = 0;

    if (c[0] != c[4])
      mask |= 1 << 0;
    if (c[1] != c[4])
      mask |= 1 << 1;
    if (c[2] != c[4])
      mask |= 1 << 2;
    if (c[3] != c[4])
      mask |= 1 << 3;
    if (c[5] != c[4])
      mask |= 1 << 4;
    if (c[6] != c[4])
      mask |= 1 << 5;
    if (c[7] != c[4])
      mask |= 1 << 6;
    if (c[8] != c[4])
      mask |= 1 << 7;

#define P0 dst0[0]
#define P1 dst0[1]
#define P2 dst1[0]
#define P3 dst1[1]
#define HQ2X_MUR (c[1] != c[5])
#define HQ2X_MDR (c[5] != c[7])
#define HQ2X_MDL (c[7] != c[3])
#define HQ2X_MUL (c[3] != c[1])
#define IC(p0) c[p0]
#define I11(p0,p1) hq2x_interp_32_11(c[p0], c[p1])
#define I211(p0,p1,p2) hq2x_interp_32_211(c[p0], c[p1], c[p2])
#define I31(p0,p1) hq2x_interp_32_31(c[p0], c[p1])
#define I332(p0,p1,p2) hq2x_interp_32_332(c[p0], c[p1], c[p2])
#define I431(p0,p1,p2) hq2x_interp_32_431(c[p0], c[p1], c[p2])
#define I521(p0,p1,p2) hq2x_interp_32_521(c[p0], c[p1], c[p2])
#define I53(p0,p1) hq2x_interp_32_53(c[p0], c[p1])
#define I611(p0,p1,p2) hq2x_interp_32_611(c[p0], c[p1], c[p2])
#define I71(p0,p1) hq2x_interp_32_71(c[p0], c[p1])
#define I772(p0,p1,p2) hq2x_interp_32_772(c[p0], c[p1], c[p2])
#define I97(p0,p1) hq2x_interp_32_97(c[p0], c[p1])
#define I1411(p0,p1,p2) hq2x_interp_32_1411(c[p0], c[p1], c[p2])
#define I151(p0,p1) hq2x_interp_32_151(c[p0], c[p1])

    switch (mask) {
#include "TextureFilters_lq2x.h"
    }

#undef P0
#undef P1
#undef P2
#undef P3
#undef HQ2X_MUR
#undef HQ2X_MDR
#undef HQ2X_MDL
#undef HQ2X_MUL
#undef IC
#undef I11
#undef I211
#undef I31
#undef I332
#undef I431
#undef I521
#undef I53
#undef I611
#undef I71
#undef I772
#undef I97
#undef I1411
#undef I151

    src0 += 1;
    src1 += 1;
    src2 += 1;
    dst0 += 2;
    dst1 += 2;
  }
}

static void lq2xS_32_def(uint32* dst0, uint32* dst1, const uint32* src0, const uint32* src1, const uint32* src2, unsigned count)
{
  unsigned i;

  for(i=0;i<count;++i) {
    unsigned char mask;

    uint32 c[9];

    c[1] = src0[0];
    c[4] = src1[0];
    c[7] = src2[0];

    if (i>0) {
      c[0] = src0[-1];
      c[3] = src1[-1];
      c[6] = src2[-1];
    } else {
      c[0] = c[1];
      c[3] = c[4];
      c[6] = c[7];
    }

    if (i<count-1) {
      c[2] = src0[1];
      c[5] = src1[1];
      c[8] = src2[1];
    } else {
      c[2] = c[1];
      c[5] = c[4];
      c[8] = c[7];
    }

    // hq2xS dynamic edge detection:
    // simply comparing the center color against its surroundings will give bad results in many cases,
    // so, instead, compare the center color relative to the max difference in brightness of this 3x3 block
    int brightArray[9];
    int maxBright = 0, minBright = 999999;
    for(int j = 0 ; j < 9 ; j++) {
      const int b = (int)((c[j] & 0xF8));
      const int g = (int)((c[j] & 0xF800)) >> 8;
      const int r = (int)((c[j] & 0xF80000)) >> 16;
      const int bright = r+r+r + g+g+g + b+b;
      if(bright > maxBright) maxBright = bright;
      if(bright < minBright) minBright = bright;

      brightArray[j] = bright;
    }
    int diffBright = ((maxBright - minBright) * 7) >> 4;
    if(diffBright > 7) {
#define ABS(x) ((x) < 0 ? -(x) : (x))

      const int centerBright = brightArray[4];
      if(ABS(brightArray[0] - centerBright) > diffBright)
        mask |= 1 << 0;
      if(ABS(brightArray[1] - centerBright) > diffBright)
        mask |= 1 << 1;
      if(ABS(brightArray[2] - centerBright) > diffBright)
        mask |= 1 << 2;
      if(ABS(brightArray[3] - centerBright) > diffBright)
        mask |= 1 << 3;
      if(ABS(brightArray[5] - centerBright) > diffBright)
        mask |= 1 << 4;
      if(ABS(brightArray[6] - centerBright) > diffBright)
        mask |= 1 << 5;
      if(ABS(brightArray[7] - centerBright) > diffBright)
        mask |= 1 << 6;
      if(ABS(brightArray[8] - centerBright) > diffBright)
        mask |= 1 << 7;
    }

#define P0 dst0[0]
#define P1 dst0[1]
#define P2 dst1[0]
#define P3 dst1[1]
#define HQ2X_MUR false
#define HQ2X_MDR false
#define HQ2X_MDL false
#define HQ2X_MUL false
#define IC(p0) c[p0]
#define I11(p0,p1) hq2x_interp_32_11(c[p0], c[p1])
#define I211(p0,p1,p2) hq2x_interp_32_211(c[p0], c[p1], c[p2])
#define I31(p0,p1) hq2x_interp_32_31(c[p0], c[p1])
#define I332(p0,p1,p2) hq2x_interp_32_332(c[p0], c[p1], c[p2])
#define I431(p0,p1,p2) hq2x_interp_32_431(c[p0], c[p1], c[p2])
#define I521(p0,p1,p2) hq2x_interp_32_521(c[p0], c[p1], c[p2])
#define I53(p0,p1) hq2x_interp_32_53(c[p0], c[p1])
#define I611(p0,p1,p2) hq2x_interp_32_611(c[p0], c[p1], c[p2])
#define I71(p0,p1) hq2x_interp_32_71(c[p0], c[p1])
#define I772(p0,p1,p2) hq2x_interp_32_772(c[p0], c[p1], c[p2])
#define I97(p0,p1) hq2x_interp_32_97(c[p0], c[p1])
#define I1411(p0,p1,p2) hq2x_interp_32_1411(c[p0], c[p1], c[p2])
#define I151(p0,p1) hq2x_interp_32_151(c[p0], c[p1])

    switch (mask) {
#include "TextureFilters_lq2x.h"
    }

#undef P0
#undef P1
#undef P2
#undef P3
#undef HQ2X_MUR
#undef HQ2X_MDR
#undef HQ2X_MDL
#undef HQ2X_MUL
#undef IC
#undef I11
#undef I211
#undef I31
#undef I332
#undef I431
#undef I521
#undef I53
#undef I611
#undef I71
#undef I772
#undef I97
#undef I1411
#undef I151

    src0 += 1;
    src1 += 1;
    src2 += 1;
    dst0 += 2;
    dst1 += 2;
  }
}

#if !_16BPP_HACK
void hq2x_16(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
  uint16 *dst0 = (uint16 *)dstPtr;
  uint16 *dst1 = dst0 + (dstPitch >> 1);

  uint16 *src0 = (uint16 *)srcPtr;
  uint16 *src1 = src0 + (srcPitch >> 1);
  uint16 *src2 = src1 + (srcPitch >> 1);

  int count;

  hq2x_16_def(dst0, dst1, src0, src0, src1, width);
  if( height == 1 ) return;

  count = height;

  count -= 2;
  while(count>0) {
    dst0 += dstPitch;
    dst1 += dstPitch;
    hq2x_16_def(dst0, dst1, src0, src1, src2, width);
    src0 = src1;
    src1 = src2;
    src2 += srcPitch >> 1;
    --count;
  }
  dst0 += dstPitch;
  dst1 += dstPitch;
  hq2x_16_def(dst0, dst1, src0, src1, src1, width);
}


void hq2xS_16(u8 *srcPtr, u32 srcPitch, u8 * /* deltaPtr */,
              u8 *dstPtr, u32 dstPitch, int width, int height)
{
  u16 *dst0 = (u16 *)dstPtr;
  u16 *dst1 = dst0 + (dstPitch >> 1);
  
  u16 *src0 = (u16 *)srcPtr;
  u16 *src1 = src0 + (srcPitch >> 1);
  u16 *src2 = src1 + (srcPitch >> 1);
  
  hq2xS_16_def(dst0, dst1, src0, src0, src1, width);
  
  int count = height;
  
  count -= 2;
  while(count) {
    dst0 += dstPitch;
    dst1 += dstPitch;
    hq2xS_16_def(dst0, dst1, src0, src1, src2, width);
    src0 = src1;
    src1 = src2;
    src2 += srcPitch >> 1;
    --count;
  }
  dst0 += dstPitch;
  dst1 += dstPitch;
  hq2xS_16_def(dst0, dst1, src0, src1, src1, width);
}
#endif /* !_16BPP_HACK */

void hq2x_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
  uint32 *dst0 = (uint32 *)dstPtr;
  uint32 *dst1 = dst0 + (dstPitch >> 2);

  uint32 *src0 = (uint32 *)srcPtr;
  uint32 *src1 = src0 + (srcPitch >> 2);
  uint32 *src2 = src1 + (srcPitch >> 2);

  int count;

  hq2x_32_def(dst0, dst1, src0, src0, src1, width);
  if( height == 1 ) return;

  count = height;

  count -= 2;
  while(count>0) {
    dst0 += dstPitch >> 1;
    dst1 += dstPitch >> 1;
    hq2x_32_def(dst0, dst1, src0, src1, src2, width);
    src0 = src1;
    src1 = src2;
    src2 += srcPitch >> 2;
    --count;
  }
  dst0 += dstPitch >> 1;
  dst1 += dstPitch >> 1;
  hq2x_32_def(dst0, dst1, src0, src1, src1, width);
}

void hq2xS_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
  uint32 *dst0 = (uint32 *)dstPtr;
  uint32 *dst1 = dst0 + (dstPitch >> 2);

  uint32 *src0 = (uint32 *)srcPtr;
  uint32 *src1 = src0 + (srcPitch >> 2);
  uint32 *src2 = src1 + (srcPitch >> 2);
  hq2xS_32_def(dst0, dst1, src0, src0, src1, width);
  
  int count = height;
  
  count -= 2;
  while(count) {
    dst0 += dstPitch >> 1;
    dst1 += dstPitch >> 1;
    hq2xS_32_def(dst0, dst1, src0, src1, src2, width);
    src0 = src1;
    src1 = src2;
    src2 += srcPitch >> 2;
    --count;
  }
  dst0 += dstPitch >> 1;
  dst1 += dstPitch >> 1;
  hq2xS_32_def(dst0, dst1, src0, src1, src1, width);
}

#if !_16BPP_HACK
void lq2x_16(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
  uint16 *dst0 = (uint16 *)dstPtr;
  uint16 *dst1 = dst0 + (dstPitch >> 1);

  uint16 *src0 = (uint16 *)srcPtr;
  uint16 *src1 = src0 + (srcPitch >> 1);
  uint16 *src2 = src1 + (srcPitch >> 1);

  int count;

  lq2x_16_def(dst0, dst1, src0, src0, src1, width);
  if( height == 1 ) return;

  count = height;

  count -= 2;
  while(count>0) {
    dst0 += dstPitch;
    dst1 += dstPitch;
    hq2x_16_def(dst0, dst1, src0, src1, src2, width);
    src0 = src1;
    src1 = src2;
    src2 += srcPitch >> 1;
    --count;
  }
  dst0 += dstPitch;
  dst1 += dstPitch;
  lq2x_16_def(dst0, dst1, src0, src1, src1, width);
}

void lq2xS_16(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
  uint16 *dst0 = (uint16 *)dstPtr;
  uint16 *dst1 = dst0 + (dstPitch >> 1);

  uint16 *src0 = (uint16 *)srcPtr;
  uint16 *src1 = src0 + (srcPitch >> 1);
  uint16 *src2 = src1 + (srcPitch >> 1);

  int count;

  lq2xS_16_def(dst0, dst1, src0, src0, src1, width);
  if( height == 1 ) return;

  count = height;

  count -= 2;
  while(count>0) {
    dst0 += dstPitch;
    dst1 += dstPitch;
    hq2x_16_def(dst0, dst1, src0, src1, src2, width);
    src0 = src1;
    src1 = src2;
    src2 += srcPitch >> 1;
    --count;
  }
  dst0 += dstPitch;
  dst1 += dstPitch;
  lq2xS_16_def(dst0, dst1, src0, src1, src1, width);
}
#endif /* !_16BPP_HACK */

void lq2x_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
  uint32 *dst0 = (uint32 *)dstPtr;
  uint32 *dst1 = dst0 + (dstPitch >> 2);

  uint32 *src0 = (uint32 *)srcPtr;
  uint32 *src1 = src0 + (srcPitch >> 2);
  uint32 *src2 = src1 + (srcPitch >> 2);

  int count;

  lq2x_32_def(dst0, dst1, src0, src0, src1, width);
  if( height == 1 ) return;

  count = height;

  count -= 2;
  while(count>0) {
    dst0 += dstPitch >> 1;
    dst1 += dstPitch >> 1;
    hq2x_32_def(dst0, dst1, src0, src1, src2, width);
    src0 = src1;
    src1 = src2;
    src2 += srcPitch >> 2;
    --count;
  }
  dst0 += dstPitch >> 1;
  dst1 += dstPitch >> 1;
  lq2x_32_def(dst0, dst1, src0, src1, src1, width);
}

void lq2xS_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height)
{
  uint32 *dst0 = (uint32 *)dstPtr;
  uint32 *dst1 = dst0 + (dstPitch >> 2);

  uint32 *src0 = (uint32 *)srcPtr;
  uint32 *src1 = src0 + (srcPitch >> 2);
  uint32 *src2 = src1 + (srcPitch >> 2);

  int count;

  lq2xS_32_def(dst0, dst1, src0, src0, src1, width);
  if( height == 1 ) return;

  count = height;

  count -= 2;
  while(count>0) {
    dst0 += dstPitch >> 1;
    dst1 += dstPitch >> 1;
    hq2x_32_def(dst0, dst1, src0, src1, src2, width);
    src0 = src1;
    src1 = src2;
    src2 += srcPitch >> 2;
    --count;
  }
  dst0 += dstPitch >> 1;
  dst1 += dstPitch >> 1;
  lq2xS_32_def(dst0, dst1, src0, src1, src1, width);
}

/************************************************************************/
/* hq3x filters                                                         */
/************************************************************************/

/************************************************************************/
/* scale2x filters                                                      */
/************************************************************************/

/************************************************************************/
/* scale3x filters                                                      */
/************************************************************************/

