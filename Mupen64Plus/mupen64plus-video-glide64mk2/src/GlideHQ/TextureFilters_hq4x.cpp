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

/*  Based on Maxim Stepin and Rice1964 hq4x code */

#include <math.h>
#include <stdlib.h>
#include "TextureFilters.h"

#if !_16BPP_HACK
static uint32 RGB444toYUV[4096];
#define RGB444toYUV(val) RGB444toYUV[val & 0x0FFF]   /* val = ARGB4444 */

/*inline static uint32 RGB444toYUV(uint32 val)
{
  uint32 r, g, b, Y, u, v;

  r = (val & 0x0F00) >> 4;
  g = (val & 0x00F0);
  b = val & 0x000F;
  r |= r >> 4;
  g |= g >> 4;
  b |= b << 4;

  Y = (r + g + b) >> 2;
  u = 128 + ((r - b) >> 2);
  v = 128 + ((2*g - r - b)>>3);

  return ((Y << 16) | (u << 8) | v);
}*/

static uint32 RGB555toYUV(uint32 val)
{
  uint32 r, g, b, Y, u, v;

  r = (val & 0x7C00) >> 7;
  g = (val & 0x03E0) >> 2;
  b = (val & 0x001F) << 3;
  r |= r >> 5;
  g |= g >> 5;
  b |= b >> 5;

  Y = (r + g + b) >> 2;
  u = 128 + ((r - b) >> 2);
  v = 128 + ((2*g - r - b)>>3);

  return ((Y << 16) | (u << 8) | v);
}

static uint32 RGB565toYUV(uint32 val)
{
  uint32 r, g, b, Y, u, v;

  r = (val & 0xF800) >> 8;
  g = (val & 0x07E0) >> 3;
  b = (val & 0x001F) << 3;
  r |= r >> 5;
  g |= g >> 6;
  b |= b >> 5;

  Y = (r + g + b) >> 2;
  u = 128 + ((r - b) >> 2);
  v = 128 + ((2*g - r - b)>>3);

  return ((Y << 16) | (u << 8) | v);
}
#endif /* !_16BPP_HACK */

static uint32 RGB888toYUV(uint32 val)
{
#if 0
  uint32 Yuv;

  __asm {
    mov eax, dword ptr [val];
    mov ebx, eax;
    mov ecx, eax;
    and ebx, 0x000000ff; // b
    and eax, 0x00ff0000; // r
    and ecx, 0x0000ff00; // g
    shl ebx, 14;
    shr eax, 2;
    shl ecx, 6;
    mov edx, ebx;
    add edx, eax;
    add edx, ecx;
    and edx, 0xffff0000;

    sub eax, ebx;
    add eax, 0x00800000;
    shr eax, 8;
    or  edx, eax;
    sub eax, 0x00800000;
    and edx, 0xffffff00;

    add ecx, 0x00800000;
    shr ecx, 5;
    shr ebx, 7;
    add eax, ebx;
    sub ecx, eax;
    shr ecx, 11;
    or  edx, ecx;

    mov dword ptr [Yuv], edx;
  }

  return Yuv;
#else
  uint32 r, g, b, Y, u, v;

  r = (val & 0x00ff0000) >> 16;
  g = (val & 0x0000ff00) >> 8;
  b = val & 0x000000ff;

  Y = (r + g + b) >> 2;
  u = (0x00000200 + r - b) >> 2;
  v = (0x00000400 + (g << 1) - r - b) >> 3;

  return ((Y << 16) | (u << 8) | v);
#endif
}

#define Ymask 0x00FF0000
#define Umask 0x0000FF00
#define Vmask 0x000000FF
#define trY 0x00300000 // ?
#define trU 0x00000700 // ??
#define trV 0x00000006 // ???

#define HQ4X_INTERP1(n, b) \
static void hq4x_Interp1_##n (uint8 * pc, uint##b p1, uint##b p2) \
{ \
  /* *((uint##b*)pc) = (p1*3+p2) >> 2; */ \
  *((uint##b*)pc) = INTERP_##n##_MASK_1_3((INTERP_##n##_MASK_1_3(p1)*3 + INTERP_##n##_MASK_1_3(p2)) / 4) \
    | INTERP_##n##_MASK_SHIFTBACK_2_4((INTERP_##n##_MASK_SHIFT_2_4(p1)*3 + INTERP_##n##_MASK_SHIFT_2_4(p2)) / 4 ); \
}

#define HQ4X_INTERP2(n, b) \
static void hq4x_Interp2_##n (uint8 * pc, uint##b p1, uint##b p2, uint##b p3) \
{ \
  /**((uint##b*)pc) = (p1*2+p2+p3) >> 2;*/ \
  *((uint##b*)pc) =  INTERP_##n##_MASK_1_3((INTERP_##n##_MASK_1_3(p1)*2 + INTERP_##n##_MASK_1_3(p2) + INTERP_##n##_MASK_1_3(p3)) / 4) \
    | INTERP_##n##_MASK_SHIFTBACK_2_4((INTERP_##n##_MASK_SHIFT_2_4(p1)*2 + INTERP_##n##_MASK_SHIFT_2_4(p2) + INTERP_##n##_MASK_SHIFT_2_4(p3)) / 4); \
}

#define HQ4X_INTERP3(n, b) \
static void hq4x_Interp3_##n (uint8 * pc, uint##b p1, uint##b p2) \
{ \
  /**((uint##b*)pc) = (p1*7+p2)/8;*/ \
  *((uint##b*)pc) =  INTERP_##n##_MASK_1_3((INTERP_##n##_MASK_1_3(p1)*7 + INTERP_##n##_MASK_1_3(p2)) / 8) \
    | INTERP_##n##_MASK_SHIFTBACK_2_4((INTERP_##n##_MASK_SHIFT_2_4(p1)*7 + INTERP_##n##_MASK_SHIFT_2_4(p2)) / 8); \
}

#define HQ4X_INTERP5(n, b) \
static void hq4x_Interp5_##n (uint8 * pc, uint##b p1, uint##b p2) \
{ \
  /**((uint##b*)pc) = (p1+p2) >> 1;*/ \
  *((uint##b*)pc) =  INTERP_##n##_MASK_1_3((INTERP_##n##_MASK_1_3(p1) + INTERP_##n##_MASK_1_3(p2)) / 2) \
    | INTERP_##n##_MASK_SHIFTBACK_2_4((INTERP_##n##_MASK_SHIFT_2_4(p1) + INTERP_##n##_MASK_SHIFT_2_4(p2)) / 2); \
}

#define HQ4X_INTERP6(n, b) \
static void hq4x_Interp6_##n (uint8 * pc, uint##b p1, uint##b p2, uint##b p3) \
{ \
  /**((uint##b*)pc) = (p1*5+p2*2+p3)/8;*/ \
  *((uint##b*)pc) =  INTERP_##n##_MASK_1_3((INTERP_##n##_MASK_1_3(p1)*5 + INTERP_##n##_MASK_1_3(p2)*2 + INTERP_##n##_MASK_1_3(p3)) / 8) \
    | INTERP_##n##_MASK_SHIFTBACK_2_4((INTERP_##n##_MASK_SHIFT_2_4(p1)*5 + INTERP_##n##_MASK_SHIFT_2_4(p2)*2 + INTERP_##n##_MASK_SHIFT_2_4(p3)) / 8); \
}

#define HQ4X_INTERP7(n, b) \
static void hq4x_Interp7_##n (uint8 * pc, uint##b p1, uint##b p2, uint##b p3) \
{ \
  /**((uint##b*)pc) = (p1*6+p2+p3)/8;*/ \
  *((uint##b*)pc) =   INTERP_##n##_MASK_1_3((INTERP_##n##_MASK_1_3(p1)*6 + INTERP_##n##_MASK_1_3(p2) + INTERP_##n##_MASK_1_3(p3)) / 8) \
    | INTERP_##n##_MASK_SHIFTBACK_2_4((INTERP_##n##_MASK_SHIFT_2_4(p1)*6 + INTERP_##n##_MASK_SHIFT_2_4(p2) + INTERP_##n##_MASK_SHIFT_2_4(p3)) / 8); \
}

#define HQ4X_INTERP8(n, b) \
static void hq4x_Interp8_##n (uint8 * pc, uint##b p1, uint##b p2) \
{ \
  /**((uint##b*)pc) = (p1*5+p2*3)/8;*/ \
  *((uint##b*)pc) =   INTERP_##n##_MASK_1_3((INTERP_##n##_MASK_1_3(p1)*5 + INTERP_##n##_MASK_1_3(p2)*3) / 8) \
    | INTERP_##n##_MASK_SHIFTBACK_2_4((INTERP_##n##_MASK_SHIFT_2_4(p1)*5 + INTERP_##n##_MASK_SHIFT_2_4(p2)*3) / 8); \
}

#if !_16BPP_HACK
#define INTERP_4444_MASK_1_3(v)           (v & 0x0F0F)
#define INTERP_4444_MASK_SHIFT_2_4(v)     ((v & 0xF0F0) >> 4)
#define INTERP_4444_MASK_SHIFTBACK_2_4(v) (INTERP_4444_MASK_1_3(v) << 4)
HQ4X_INTERP1(4444, 16)
HQ4X_INTERP2(4444, 16)
HQ4X_INTERP3(4444, 16)
HQ4X_INTERP5(4444, 16)
HQ4X_INTERP6(4444, 16)
HQ4X_INTERP7(4444, 16)
HQ4X_INTERP8(4444, 16)

#define INTERP_1555_MASK_1_3(v)           (v & 0x7C1F)
#define INTERP_1555_MASK_SHIFT_2_4(v)     ((v & 0x83E0) >> 5)
#define INTERP_1555_MASK_SHIFTBACK_2_4(v) (INTERP_1555_MASK_1_3(v) << 5)
HQ4X_INTERP1(1555, 16)
HQ4X_INTERP2(1555, 16)
HQ4X_INTERP3(1555, 16)
HQ4X_INTERP5(1555, 16)
HQ4X_INTERP6(1555, 16)
HQ4X_INTERP7(1555, 16)
HQ4X_INTERP8(1555, 16)

#define INTERP_565_MASK_1_3(v)           (v & 0xF81F)
#define INTERP_565_MASK_SHIFT_2_4(v)     ((v & 0x7E0) >> 5)
#define INTERP_565_MASK_SHIFTBACK_2_4(v) (INTERP_565_MASK_1_3(v) << 5)
HQ4X_INTERP1(565, 16)
HQ4X_INTERP2(565, 16)
HQ4X_INTERP3(565, 16)
HQ4X_INTERP5(565, 16)
HQ4X_INTERP6(565, 16)
HQ4X_INTERP7(565, 16)
HQ4X_INTERP8(565, 16)
#endif /* !_16BPP_HACK */

#define INTERP_8888_MASK_1_3(v)           (v & 0x00FF00FF)
#define INTERP_8888_MASK_SHIFT_2_4(v)     ((v & 0xFF00FF00) >> 8)
#define INTERP_8888_MASK_SHIFTBACK_2_4(v) (INTERP_8888_MASK_1_3(v) << 8)
HQ4X_INTERP1(8888, 32)
HQ4X_INTERP2(8888, 32)
HQ4X_INTERP3(8888, 32)
HQ4X_INTERP5(8888, 32)
HQ4X_INTERP6(8888, 32)
HQ4X_INTERP7(8888, 32)
HQ4X_INTERP8(8888, 32)

#define PIXEL00_0     *((int*)(pOut)) = c[5];
#define PIXEL00_11    hq4x_Interp1(pOut, c[5], c[4]);
#define PIXEL00_12    hq4x_Interp1(pOut, c[5], c[2]);
#define PIXEL00_20    hq4x_Interp2(pOut, c[5], c[2], c[4]);
#define PIXEL00_50    hq4x_Interp5(pOut, c[2], c[4]);
#define PIXEL00_80    hq4x_Interp8(pOut, c[5], c[1]);
#define PIXEL00_81    hq4x_Interp8(pOut, c[5], c[4]);
#define PIXEL00_82    hq4x_Interp8(pOut, c[5], c[2]);
#define PIXEL01_0     *((int*)(pOut+BPP)) = c[5];
#define PIXEL01_10    hq4x_Interp1(pOut+BPP, c[5], c[1]);
#define PIXEL01_12    hq4x_Interp1(pOut+BPP, c[5], c[2]);
#define PIXEL01_14    hq4x_Interp1(pOut+BPP, c[2], c[5]);
#define PIXEL01_21    hq4x_Interp2(pOut+BPP, c[2], c[5], c[4]);
#define PIXEL01_31    hq4x_Interp3(pOut+BPP, c[5], c[4]);
#define PIXEL01_50    hq4x_Interp5(pOut+BPP, c[2], c[5]);
#define PIXEL01_60    hq4x_Interp6(pOut+BPP, c[5], c[2], c[4]);
#define PIXEL01_61    hq4x_Interp6(pOut+BPP, c[5], c[2], c[1]);
#define PIXEL01_82    hq4x_Interp8(pOut+BPP, c[5], c[2]);
#define PIXEL01_83    hq4x_Interp8(pOut+BPP, c[2], c[4]);
#define PIXEL02_0     *((int*)(pOut+BPP2)) = c[5];
#define PIXEL02_10    hq4x_Interp1(pOut+BPP2, c[5], c[3]);
#define PIXEL02_11    hq4x_Interp1(pOut+BPP2, c[5], c[2]);
#define PIXEL02_13    hq4x_Interp1(pOut+BPP2, c[2], c[5]);
#define PIXEL02_21    hq4x_Interp2(pOut+BPP2, c[2], c[5], c[6]);
#define PIXEL02_32    hq4x_Interp3(pOut+BPP2, c[5], c[6]);
#define PIXEL02_50    hq4x_Interp5(pOut+BPP2, c[2], c[5]);
#define PIXEL02_60    hq4x_Interp6(pOut+BPP2, c[5], c[2], c[6]);
#define PIXEL02_61    hq4x_Interp6(pOut+BPP2, c[5], c[2], c[3]);
#define PIXEL02_81    hq4x_Interp8(pOut+BPP2, c[5], c[2]);
#define PIXEL02_83    hq4x_Interp8(pOut+BPP2, c[2], c[6]);
#define PIXEL03_0     *((int*)(pOut+BPP3)) = c[5];
#define PIXEL03_11    hq4x_Interp1(pOut+BPP3, c[5], c[2]);
#define PIXEL03_12    hq4x_Interp1(pOut+BPP3, c[5], c[6]);
#define PIXEL03_20    hq4x_Interp2(pOut+BPP3, c[5], c[2], c[6]);
#define PIXEL03_50    hq4x_Interp5(pOut+BPP3, c[2], c[6]);
#define PIXEL03_80    hq4x_Interp8(pOut+BPP3, c[5], c[3]);
#define PIXEL03_81    hq4x_Interp8(pOut+BPP3, c[5], c[2]);
#define PIXEL03_82    hq4x_Interp8(pOut+BPP3, c[5], c[6]);
#define PIXEL10_0     *((int*)(pOut+BpL)) = c[5];
#define PIXEL10_10    hq4x_Interp1(pOut+BpL, c[5], c[1]);
#define PIXEL10_11    hq4x_Interp1(pOut+BpL, c[5], c[4]);
#define PIXEL10_13    hq4x_Interp1(pOut+BpL, c[4], c[5]);
#define PIXEL10_21    hq4x_Interp2(pOut+BpL, c[4], c[5], c[2]);
#define PIXEL10_32    hq4x_Interp3(pOut+BpL, c[5], c[2]);
#define PIXEL10_50    hq4x_Interp5(pOut+BpL, c[4], c[5]);
#define PIXEL10_60    hq4x_Interp6(pOut+BpL, c[5], c[4], c[2]);
#define PIXEL10_61    hq4x_Interp6(pOut+BpL, c[5], c[4], c[1]);
#define PIXEL10_81    hq4x_Interp8(pOut+BpL, c[5], c[4]);
#define PIXEL10_83    hq4x_Interp8(pOut+BpL, c[4], c[2]);
#define PIXEL11_0     *((int*)(pOut+BpL+BPP)) = c[5];
#define PIXEL11_30    hq4x_Interp3(pOut+BpL+BPP, c[5], c[1]);
#define PIXEL11_31    hq4x_Interp3(pOut+BpL+BPP, c[5], c[4]);
#define PIXEL11_32    hq4x_Interp3(pOut+BpL+BPP, c[5], c[2]);
#define PIXEL11_70    hq4x_Interp7(pOut+BpL+BPP, c[5], c[4], c[2]);
#define PIXEL12_0     *((int*)(pOut+BpL+BPP2)) = c[5];
#define PIXEL12_30    hq4x_Interp3(pOut+BpL+BPP2, c[5], c[3]);
#define PIXEL12_31    hq4x_Interp3(pOut+BpL+BPP2, c[5], c[2]);
#define PIXEL12_32    hq4x_Interp3(pOut+BpL+BPP2, c[5], c[6]);
#define PIXEL12_70    hq4x_Interp7(pOut+BpL+BPP2, c[5], c[6], c[2]);
#define PIXEL13_0     *((int*)(pOut+BpL+BPP3)) = c[5];
#define PIXEL13_10    hq4x_Interp1(pOut+BpL+BPP3, c[5], c[3]);
#define PIXEL13_12    hq4x_Interp1(pOut+BpL+BPP3, c[5], c[6]);
#define PIXEL13_14    hq4x_Interp1(pOut+BpL+BPP3, c[6], c[5]);
#define PIXEL13_21    hq4x_Interp2(pOut+BpL+BPP3, c[6], c[5], c[2]);
#define PIXEL13_31    hq4x_Interp3(pOut+BpL+BPP3, c[5], c[2]);
#define PIXEL13_50    hq4x_Interp5(pOut+BpL+BPP3, c[6], c[5]);
#define PIXEL13_60    hq4x_Interp6(pOut+BpL+BPP3, c[5], c[6], c[2]);
#define PIXEL13_61    hq4x_Interp6(pOut+BpL+BPP3, c[5], c[6], c[3]);
#define PIXEL13_82    hq4x_Interp8(pOut+BpL+BPP3, c[5], c[6]);
#define PIXEL13_83    hq4x_Interp8(pOut+BpL+BPP3, c[6], c[2]);
#define PIXEL20_0     *((int*)(pOut+BpL+BpL)) = c[5];
#define PIXEL20_10    hq4x_Interp1(pOut+BpL+BpL, c[5], c[7]);
#define PIXEL20_12    hq4x_Interp1(pOut+BpL+BpL, c[5], c[4]);
#define PIXEL20_14    hq4x_Interp1(pOut+BpL+BpL, c[4], c[5]);
#define PIXEL20_21    hq4x_Interp2(pOut+BpL+BpL, c[4], c[5], c[8]);
#define PIXEL20_31    hq4x_Interp3(pOut+BpL+BpL, c[5], c[8]);
#define PIXEL20_50    hq4x_Interp5(pOut+BpL+BpL, c[4], c[5]);
#define PIXEL20_60    hq4x_Interp6(pOut+BpL+BpL, c[5], c[4], c[8]);
#define PIXEL20_61    hq4x_Interp6(pOut+BpL+BpL, c[5], c[4], c[7]);
#define PIXEL20_82    hq4x_Interp8(pOut+BpL+BpL, c[5], c[4]);
#define PIXEL20_83    hq4x_Interp8(pOut+BpL+BpL, c[4], c[8]);
#define PIXEL21_0     *((int*)(pOut+BpL+BpL+BPP)) = c[5];
#define PIXEL21_30    hq4x_Interp3(pOut+BpL+BpL+BPP, c[5], c[7]);
#define PIXEL21_31    hq4x_Interp3(pOut+BpL+BpL+BPP, c[5], c[8]);
#define PIXEL21_32    hq4x_Interp3(pOut+BpL+BpL+BPP, c[5], c[4]);
#define PIXEL21_70    hq4x_Interp7(pOut+BpL+BpL+BPP, c[5], c[4], c[8]);
#define PIXEL22_0     *((int*)(pOut+BpL+BpL+BPP2)) = c[5];
#define PIXEL22_30    hq4x_Interp3(pOut+BpL+BpL+BPP2, c[5], c[9]);
#define PIXEL22_31    hq4x_Interp3(pOut+BpL+BpL+BPP2, c[5], c[6]);
#define PIXEL22_32    hq4x_Interp3(pOut+BpL+BpL+BPP2, c[5], c[8]);
#define PIXEL22_70    hq4x_Interp7(pOut+BpL+BpL+BPP2, c[5], c[6], c[8]);
#define PIXEL23_0     *((int*)(pOut+BpL+BpL+BPP3)) = c[5];
#define PIXEL23_10    hq4x_Interp1(pOut+BpL+BpL+BPP3, c[5], c[9]);
#define PIXEL23_11    hq4x_Interp1(pOut+BpL+BpL+BPP3, c[5], c[6]);
#define PIXEL23_13    hq4x_Interp1(pOut+BpL+BpL+BPP3, c[6], c[5]);
#define PIXEL23_21    hq4x_Interp2(pOut+BpL+BpL+BPP3, c[6], c[5], c[8]);
#define PIXEL23_32    hq4x_Interp3(pOut+BpL+BpL+BPP3, c[5], c[8]);
#define PIXEL23_50    hq4x_Interp5(pOut+BpL+BpL+BPP3, c[6], c[5]);
#define PIXEL23_60    hq4x_Interp6(pOut+BpL+BpL+BPP3, c[5], c[6], c[8]);
#define PIXEL23_61    hq4x_Interp6(pOut+BpL+BpL+BPP3, c[5], c[6], c[9]);
#define PIXEL23_81    hq4x_Interp8(pOut+BpL+BpL+BPP3, c[5], c[6]);
#define PIXEL23_83    hq4x_Interp8(pOut+BpL+BpL+BPP3, c[6], c[8]);
#define PIXEL30_0     *((int*)(pOut+BpL+BpL+BpL)) = c[5];
#define PIXEL30_11    hq4x_Interp1(pOut+BpL+BpL+BpL, c[5], c[8]);
#define PIXEL30_12    hq4x_Interp1(pOut+BpL+BpL+BpL, c[5], c[4]);
#define PIXEL30_20    hq4x_Interp2(pOut+BpL+BpL+BpL, c[5], c[8], c[4]);
#define PIXEL30_50    hq4x_Interp5(pOut+BpL+BpL+BpL, c[8], c[4]);
#define PIXEL30_80    hq4x_Interp8(pOut+BpL+BpL+BpL, c[5], c[7]);
#define PIXEL30_81    hq4x_Interp8(pOut+BpL+BpL+BpL, c[5], c[8]);
#define PIXEL30_82    hq4x_Interp8(pOut+BpL+BpL+BpL, c[5], c[4]);
#define PIXEL31_0     *((int*)(pOut+BpL+BpL+BpL+BPP)) = c[5];
#define PIXEL31_10    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP, c[5], c[7]);
#define PIXEL31_11    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP, c[5], c[8]);
#define PIXEL31_13    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP, c[8], c[5]);
#define PIXEL31_21    hq4x_Interp2(pOut+BpL+BpL+BpL+BPP, c[8], c[5], c[4]);
#define PIXEL31_32    hq4x_Interp3(pOut+BpL+BpL+BpL+BPP, c[5], c[4]);
#define PIXEL31_50    hq4x_Interp5(pOut+BpL+BpL+BpL+BPP, c[8], c[5]);
#define PIXEL31_60    hq4x_Interp6(pOut+BpL+BpL+BpL+BPP, c[5], c[8], c[4]);
#define PIXEL31_61    hq4x_Interp6(pOut+BpL+BpL+BpL+BPP, c[5], c[8], c[7]);
#define PIXEL31_81    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP, c[5], c[8]);
#define PIXEL31_83    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP, c[8], c[4]);
#define PIXEL32_0     *((int*)(pOut+BpL+BpL+BpL+BPP2)) = c[5];
#define PIXEL32_10    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP2, c[5], c[9]);
#define PIXEL32_12    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP2, c[5], c[8]);
#define PIXEL32_14    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP2, c[8], c[5]);
#define PIXEL32_21    hq4x_Interp2(pOut+BpL+BpL+BpL+BPP2, c[8], c[5], c[6]);
#define PIXEL32_31    hq4x_Interp3(pOut+BpL+BpL+BpL+BPP2, c[5], c[6]);
#define PIXEL32_50    hq4x_Interp5(pOut+BpL+BpL+BpL+BPP2, c[8], c[5]);
#define PIXEL32_60    hq4x_Interp6(pOut+BpL+BpL+BpL+BPP2, c[5], c[8], c[6]);
#define PIXEL32_61    hq4x_Interp6(pOut+BpL+BpL+BpL+BPP2, c[5], c[8], c[9]);
#define PIXEL32_82    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP2, c[5], c[8]);
#define PIXEL32_83    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP2, c[8], c[6]);
#define PIXEL33_0     *((int*)(pOut+BpL+BpL+BpL+BPP3)) = c[5];
#define PIXEL33_11    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP3, c[5], c[6]);
#define PIXEL33_12    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP3, c[5], c[8]);
#define PIXEL33_20    hq4x_Interp2(pOut+BpL+BpL+BpL+BPP3, c[5], c[8], c[6]);
#define PIXEL33_50    hq4x_Interp5(pOut+BpL+BpL+BpL+BPP3, c[8], c[6]);
#define PIXEL33_80    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP3, c[5], c[9]);
#define PIXEL33_81    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP3, c[5], c[6]);
#define PIXEL33_82    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP3, c[5], c[8]);

#define HQ4X_DIFF(n, b) \
static int Diff_##n (uint##b w1, uint##b w2) \
{ \
  int YUV1, YUV2; \
  YUV1 = RGB##n##toYUV(w1); \
  YUV2 = RGB##n##toYUV(w2); \
  return ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) || \
           ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) || \
           ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) ); \
}

HQ4X_DIFF(888, 32)

#if !_16BPP_HACK
HQ4X_DIFF(444, 16)
HQ4X_DIFF(555, 16)
HQ4X_DIFF(565, 16)

void hq4x_4444(unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL)
{
#define hq4x_Interp1 hq4x_Interp1_4444
#define hq4x_Interp2 hq4x_Interp2_4444
#define hq4x_Interp3 hq4x_Interp3_4444
#define hq4x_Interp4 hq4x_Interp4_4444
#define hq4x_Interp5 hq4x_Interp5_4444
#define hq4x_Interp6 hq4x_Interp6_4444
#define hq4x_Interp7 hq4x_Interp7_4444
#define hq4x_Interp8 hq4x_Interp8_4444
#define Diff Diff_444
#define BPP   2
#define BPP2  4
#define BPP3  6

  int  i, j, k;
  int  prevline, nextline;
  uint16  w[10];
  uint16  c[10];

  int pattern;
  int flag;

  int YUV1, YUV2;

  //   +----+----+----+
  //   |    |    |    |
  //   | w1 | w2 | w3 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w4 | w5 | w6 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w7 | w8 | w9 |
  //   +----+----+----+

  for (j = 0; j < Yres; j++) {
    if (j>0)      prevline = -SrcPPL*2; else prevline = 0;
    if (j<Yres-1) nextline =  SrcPPL*2; else nextline = 0;

    for (i=0; i<Xres; i++) {
      w[2] = *((uint16*)(pIn + prevline));
      w[5] = *((uint16*)pIn);
      w[8] = *((uint16*)(pIn + nextline));

      if (i>0) {
        w[1] = *((uint16*)(pIn + prevline - 2));
        w[4] = *((uint16*)(pIn - 2));
        w[7] = *((uint16*)(pIn + nextline - 2));
      } else {
        w[1] = w[2];
        w[4] = w[5];
        w[7] = w[8];
      }

      if (i<Xres-1) {
        w[3] = *((uint16*)(pIn + prevline + 2));
        w[6] = *((uint16*)(pIn + 2));
        w[9] = *((uint16*)(pIn + nextline + 2));
      }   else {
        w[3] = w[2];
        w[6] = w[5];
        w[9] = w[8];
      }

      pattern = 0;
      flag = 1;

      YUV1 = RGB444toYUV(w[5]);

      for (k=1; k<=9; k++) {
        if (k==5) continue;

        if ( w[k] != w[5] ) {
          YUV2 = RGB444toYUV(w[k]);
          if ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
               ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
               ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
            pattern |= flag;
        }
        flag <<= 1;
      }

      for (k=1; k<=9; k++)
        c[k] = w[k];

#include "TextureFilters_hq4x.h"

      pIn+=2;
      pOut+=8;
    }
    pIn += 2*(SrcPPL-Xres);
    pOut+= 8*(SrcPPL-Xres);
    pOut+=BpL;
    pOut+=BpL;
    pOut+=BpL;
  }

#undef BPP
#undef BPP2
#undef BPP3
#undef Diff
#undef hq4x_Interp1
#undef hq4x_Interp2
#undef hq4x_Interp3
#undef hq4x_Interp4
#undef hq4x_Interp5
#undef hq4x_Interp6
#undef hq4x_Interp7
#undef hq4x_Interp8
}

void hq4x_1555(unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL)
{
#define hq4x_Interp1 hq4x_Interp1_1555
#define hq4x_Interp2 hq4x_Interp2_1555
#define hq4x_Interp3 hq4x_Interp3_1555
#define hq4x_Interp4 hq4x_Interp4_1555
#define hq4x_Interp5 hq4x_Interp5_1555
#define hq4x_Interp6 hq4x_Interp6_1555
#define hq4x_Interp7 hq4x_Interp7_1555
#define hq4x_Interp8 hq4x_Interp8_1555
#define Diff Diff_555
#define BPP   2
#define BPP2  4
#define BPP3  6

  int  i, j, k;
  int  prevline, nextline;
  uint16  w[10];
  uint16  c[10];

  int pattern;
  int flag;

  int YUV1, YUV2;

  //   +----+----+----+
  //   |    |    |    |
  //   | w1 | w2 | w3 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w4 | w5 | w6 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w7 | w8 | w9 |
  //   +----+----+----+

  for (j = 0; j < Yres; j++) {
    if (j>0)      prevline = -SrcPPL*2; else prevline = 0;
    if (j<Yres-1) nextline =  SrcPPL*2; else nextline = 0;

    for (i=0; i<Xres; i++) {
      w[2] = *((uint16*)(pIn + prevline));
      w[5] = *((uint16*)pIn);
      w[8] = *((uint16*)(pIn + nextline));

      if (i>0) {
        w[1] = *((uint16*)(pIn + prevline - 2));
        w[4] = *((uint16*)(pIn - 2));
        w[7] = *((uint16*)(pIn + nextline - 2));
      } else {
        w[1] = w[2];
        w[4] = w[5];
        w[7] = w[8];
      }

      if (i<Xres-1) {
        w[3] = *((uint16*)(pIn + prevline + 2));
        w[6] = *((uint16*)(pIn + 2));
        w[9] = *((uint16*)(pIn + nextline + 2));
      }   else {
        w[3] = w[2];
        w[6] = w[5];
        w[9] = w[8];
      }

      pattern = 0;
      flag = 1;

      YUV1 = RGB555toYUV(w[5]);

      for (k=1; k<=9; k++) {
        if (k==5) continue;

        if ( w[k] != w[5] ) {
          YUV2 = RGB555toYUV(w[k]);
          if ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
               ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
               ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
            pattern |= flag;
        }
        flag <<= 1;
      }

      for (k=1; k<=9; k++)
        c[k] = w[k];

#include "TextureFilters_hq4x.h"

      pIn+=2;
      pOut+=8;
    }
    pIn += 2*(SrcPPL-Xres);
    pOut+= 8*(SrcPPL-Xres);
    pOut+=BpL;
    pOut+=BpL;
    pOut+=BpL;
  }

#undef BPP
#undef BPP2
#undef BPP3
#undef Diff
#undef hq4x_Interp1
#undef hq4x_Interp2
#undef hq4x_Interp3
#undef hq4x_Interp4
#undef hq4x_Interp5
#undef hq4x_Interp6
#undef hq4x_Interp7
#undef hq4x_Interp8
}

void hq4x_565(unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL)
{
#define hq4x_Interp1 hq4x_Interp1_565
#define hq4x_Interp2 hq4x_Interp2_565
#define hq4x_Interp3 hq4x_Interp3_565
#define hq4x_Interp4 hq4x_Interp4_565
#define hq4x_Interp5 hq4x_Interp5_565
#define hq4x_Interp6 hq4x_Interp6_565
#define hq4x_Interp7 hq4x_Interp7_565
#define hq4x_Interp8 hq4x_Interp8_565
#define Diff Diff_565
#define BPP   2
#define BPP2  4
#define BPP3  6

  int  i, j, k;
  int  prevline, nextline;
  uint16  w[10];
  uint16  c[10];

  int pattern;
  int flag;

  int YUV1, YUV2;

  //   +----+----+----+
  //   |    |    |    |
  //   | w1 | w2 | w3 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w4 | w5 | w6 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w7 | w8 | w9 |
  //   +----+----+----+

  for (j = 0; j < Yres; j++) {
    if (j>0)      prevline = -SrcPPL*2; else prevline = 0;
    if (j<Yres-1) nextline =  SrcPPL*2; else nextline = 0;

    for (i=0; i<Xres; i++) {
      w[2] = *((uint16*)(pIn + prevline));
      w[5] = *((uint16*)pIn);
      w[8] = *((uint16*)(pIn + nextline));

      if (i>0) {
        w[1] = *((uint16*)(pIn + prevline - 2));
        w[4] = *((uint16*)(pIn - 2));
        w[7] = *((uint16*)(pIn + nextline - 2));
      } else {
        w[1] = w[2];
        w[4] = w[5];
        w[7] = w[8];
      }

      if (i<Xres-1) {
        w[3] = *((uint16*)(pIn + prevline + 2));
        w[6] = *((uint16*)(pIn + 2));
        w[9] = *((uint16*)(pIn + nextline + 2));
      } else {
        w[3] = w[2];
        w[6] = w[5];
        w[9] = w[8];
      }

      pattern = 0;
      flag = 1;

      YUV1 = RGB565toYUV(w[5]);

      for (k=1; k<=9; k++) {
        if (k==5) continue;

        if ( w[k] != w[5] ) {
          YUV2 = RGB565toYUV(w[k]);
          if ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
               ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
               ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
            pattern |= flag;
        }
        flag <<= 1;
      }

      for (k=1; k<=9; k++)
        c[k] = w[k];

#include "TextureFilters_hq4x.h"

      pIn+=2;
      pOut+=8;
    }
    pIn += 2*(SrcPPL-Xres);
    pOut+= 8*(SrcPPL-Xres);
    pOut+=BpL;
    pOut+=BpL;
    pOut+=BpL;
  }

#undef BPP
#undef BPP2
#undef BPP3
#undef Diff
#undef hq4x_Interp1
#undef hq4x_Interp2
#undef hq4x_Interp3
#undef hq4x_Interp4
#undef hq4x_Interp5
#undef hq4x_Interp6
#undef hq4x_Interp7
#undef hq4x_Interp8
}
#endif /* !_16BPP_HACK */

void hq4x_8888(unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL)
{
#define hq4x_Interp1 hq4x_Interp1_8888
#define hq4x_Interp2 hq4x_Interp2_8888
#define hq4x_Interp3 hq4x_Interp3_8888
#define hq4x_Interp4 hq4x_Interp4_8888
#define hq4x_Interp5 hq4x_Interp5_8888
#define hq4x_Interp6 hq4x_Interp6_8888
#define hq4x_Interp7 hq4x_Interp7_8888
#define hq4x_Interp8 hq4x_Interp8_8888
#define Diff Diff_888
#define BPP  4
#define BPP2 8
#define BPP3 12

  int  i, j, k;
  int  prevline, nextline;
  uint32  w[10];
  uint32  c[10];

  int pattern;
  int flag;

  int YUV1, YUV2;

  //   +----+----+----+
  //   |    |    |    |
  //   | w1 | w2 | w3 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w4 | w5 | w6 |
  //   +----+----+----+
  //   |    |    |    |
  //   | w7 | w8 | w9 |
  //   +----+----+----+

  for (j = 0; j < Yres; j++) {
    if (j>0)      prevline = -SrcPPL*4; else prevline = 0;
    if (j<Yres-1) nextline =  SrcPPL*4; else nextline = 0;

    for (i=0; i<Xres; i++) {
      w[2] = *((uint32*)(pIn + prevline));
      w[5] = *((uint32*)pIn);
      w[8] = *((uint32*)(pIn + nextline));

      if (i>0) {
        w[1] = *((uint32*)(pIn + prevline - 4));
        w[4] = *((uint32*)(pIn - 4));
        w[7] = *((uint32*)(pIn + nextline - 4));
      } else {
        w[1] = w[2];
        w[4] = w[5];
        w[7] = w[8];
      }

      if (i<Xres-1) {
        w[3] = *((uint32*)(pIn + prevline + 4));
        w[6] = *((uint32*)(pIn + 4));
        w[9] = *((uint32*)(pIn + nextline + 4));
      } else {
        w[3] = w[2];
        w[6] = w[5];
        w[9] = w[8];
      }

      pattern = 0;
      flag = 1;

      YUV1 = RGB888toYUV(w[5]);

      for (k=1; k<=9; k++) {
        if (k==5) continue;

        if ( w[k] != w[5] ) {
          YUV2 = RGB888toYUV(w[k]);
          if ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
               ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
               ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
            pattern |= flag;
        }
        flag <<= 1;
      }

      for (k=1; k<=9; k++)
        c[k] = w[k];

#include "TextureFilters_hq4x.h"

      pIn+=4;
      pOut+=16;
    }

    pIn += 4*(SrcPPL-Xres);
    pOut+= 16*(SrcPPL-Xres);
    pOut+=BpL;
    pOut+=BpL;
    pOut+=BpL;
  }

#undef BPP
#undef BPP2
#undef BPP3
#undef Diff
#undef hq4x_Interp1
#undef hq4x_Interp2
#undef hq4x_Interp3
#undef hq4x_Interp4
#undef hq4x_Interp5
#undef hq4x_Interp6
#undef hq4x_Interp7
#undef hq4x_Interp8
}

#if !_16BPP_HACK
void hq4x_init(void)
{
  static int done = 0;
  int r, g, b, Y, u, v, i, j, k;

  if (done ) return;

  for (i = 0; i < 16; i++) {
    for (j = 0; j < 16; j++) {
      for (k = 0; k < 16; k++) {
        r = (i << 4) | i;
        g = (j << 4) | j;
        b = (k << 4) | k;

        /* Microsoft's RGB888->YUV conversion */
        /*Y = (((  66 * r + 129 * g +  25 * b + 128) >> 8) + 16) & 0xFF;
        u = ((( -38 * r -  74 * g + 112 * b + 128) >> 8) + 128) & 0xFF;
        v = ((( 112 * r -  94 * g -  18 * b + 128) >> 8) + 128) & 0xFF;*/

        Y = (r + g + b) >> 2;
        u = 128 + ((r - b) >> 2);
        v = 128 + ((-r + 2*g -b)>>3);

        RGB444toYUV[(i << 8) | (j << 4) | k] = (Y << 16) | (u << 8) | v;
      }
    }
  }

  done = 1;
}
#endif /* !_16BPP_HACK */
