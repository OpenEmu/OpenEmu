/* 2xSaI
 * Copyright (c) Derek Liauw Kie Fa, 1999-2002
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

#include "2xSaI.h"

//#include "System.h"
#include "main.h"
//#include "../gba/Port.h"

#if defined(__GNUC__) && defined(__i386__)
  #define MMX 1
#else
  #undef MMX
#endif

#undef MMX	// Remove when we have a system for compiling 32-bit x86 standalone assembly files.

extern "C"
{
#ifdef MMX
  void _2xSaILine (sai_uint8 *srcPtr, sai_uint8 *deltaPtr, sai_uint32 srcPitch,
                   sai_uint32 width, sai_uint8 *dstPtr, sai_uint32 dstPitch);
  void _2xSaISuperEagleLine (sai_uint8 *srcPtr, sai_uint8 *deltaPtr,
                             sai_uint32 srcPitch, sai_uint32 width,
                             sai_uint8 *dstPtr, sai_uint32 dstPitch);
  void _2xSaISuper2xSaILine (sai_uint8 *srcPtr, sai_uint8 *deltaPtr,
                             sai_uint32 srcPitch, sai_uint32 width,
                             sai_uint8 *dstPtr, sai_uint32 dstPitch);
  void Init_2xSaIMMX (sai_uint32 BitFormat);

  bool cpu_mmx = 1;
#endif
}
static sai_uint32 colorMask = 0xF7DEF7DE;
static sai_uint32 lowPixelMask = 0x08210821;
static sai_uint32 qcolorMask = 0xE79CE79C;
static sai_uint32 qlowpixelMask = 0x18631863;
static sai_uint32 redblueMask = 0xF81F;
static sai_uint32 greenMask = 0x7E0;

sai_uint32 qRGB_COLOR_MASK[2] = { 0xF7DEF7DE, 0xF7DEF7DE };

//extern void hq2x_init(unsigned);

int Init_2xSaI(sai_uint32 systemColorDepth, sai_uint32 BitFormat)
{
  if(systemColorDepth == 16) {
    if (BitFormat == 565) {
      colorMask = 0xF7DEF7DE;
      lowPixelMask = 0x08210821;
      qcolorMask = 0xE79CE79C;
      qlowpixelMask = 0x18631863;
      redblueMask = 0xF81F;
      greenMask = 0x7E0;
      qRGB_COLOR_MASK[0] = qRGB_COLOR_MASK[1] = 0xF7DEF7DE;
      //hq2x_init(16);
    } else if (BitFormat == 555) {
      colorMask = 0x7BDE7BDE;
      lowPixelMask = 0x04210421;
      qcolorMask = 0x739C739C;
      qlowpixelMask = 0x0C630C63;
      redblueMask = 0x7C1F;
      greenMask = 0x3E0;
      qRGB_COLOR_MASK[0] = qRGB_COLOR_MASK[1] = 0x7BDE7BDE;
      //hq2x_init(15);
    } else {
      return 0;
    }
  } else if(systemColorDepth == 32) {
    colorMask = 0xfefefefe;
    lowPixelMask = 0x01010101;
    qcolorMask = 0xfcfcfcfc;
    qlowpixelMask = 0x03030303;
    qRGB_COLOR_MASK[0] = qRGB_COLOR_MASK[1] = 0xfefefefe;
    //hq2x_init(32);
  } else
    return 0;

#ifdef MMX
    Init_2xSaIMMX (BitFormat);
#endif

  return 1;
}

static inline int GetResult1 (sai_uint32 A, sai_uint32 B, sai_uint32 C, sai_uint32 D,
                              sai_uint32 /* E */)
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C)
      x += 1;
    else if (B == C)
      y += 1;
    if (A == D)
      x += 1;
    else if (B == D)
      y += 1;
    if (x <= 1)
      r += 1;
    if (y <= 1)
      r -= 1;
    return r;
}

static inline int GetResult2 (sai_uint32 A, sai_uint32 B, sai_uint32 C, sai_uint32 D,
                              sai_uint32 /* E */)
{
  int x = 0;
  int y = 0;
  int r = 0;

  if (A == C)
    x += 1;
  else if (B == C)
    y += 1;
  if (A == D)
    x += 1;
  else if (B == D)
    y += 1;
  if (x <= 1)
    r -= 1;
  if (y <= 1)
    r += 1;
  return r;
}

static inline int GetResult (sai_uint32 A, sai_uint32 B, sai_uint32 C, sai_uint32 D)
{
  int x = 0;
  int y = 0;
  int r = 0;

  if (A == C)
    x += 1;
  else if (B == C)
    y += 1;
  if (A == D)
    x += 1;
  else if (B == D)
    y += 1;
  if (x <= 1)
    r += 1;
  if (y <= 1)
    r -= 1;
  return r;
}

static inline sai_uint32 INTERPOLATE (sai_uint32 A, sai_uint32 B)
{
  if (A != B) {
    return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) +
            (A & B & lowPixelMask));
  } else
    return A;
}

static inline sai_uint32 Q_INTERPOLATE (sai_uint32 A, sai_uint32 B, sai_uint32 C, sai_uint32 D)
{
  register sai_uint32 x = ((A & qcolorMask) >> 2) +
    ((B & qcolorMask) >> 2) +
    ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
  register sai_uint32 y = (A & qlowpixelMask) +
    (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);

  y = (y >> 2) & qlowpixelMask;
  return x + y;
}

static inline int GetResult1_32 (sai_uint32 A, sai_uint32 B, sai_uint32 C, sai_uint32 D,
                                 sai_uint32 /* E */)
{
    int x = 0;
    int y = 0;
    int r = 0;

    if (A == C)
      x += 1;
    else if (B == C)
      y += 1;
    if (A == D)
      x += 1;
    else if (B == D)
      y += 1;
    if (x <= 1)
      r += 1;
    if (y <= 1)
      r -= 1;
    return r;
}

static inline int GetResult2_32 (sai_uint32 A, sai_uint32 B, sai_uint32 C, sai_uint32 D,
                                 sai_uint32 /* E */)
{
  int x = 0;
  int y = 0;
  int r = 0;

  if (A == C)
    x += 1;
  else if (B == C)
    y += 1;
  if (A == D)
    x += 1;
  else if (B == D)
    y += 1;
  if (x <= 1)
    r -= 1;
  if (y <= 1)
    r += 1;
  return r;
}

void Super2xSaI (sai_uint8 *srcPtr, sai_uint32 srcPitch,
                 sai_uint8 *deltaPtr, sai_uint8 *dstPtr, sai_uint32 dstPitch,
                 int width, int height)
{
  sai_uint16 *bP;
  sai_uint8  *dP;
  sai_uint32 inc_bP;
  sai_uint32 Nextline = srcPitch >> 1;
#ifdef MMX
  if (cpu_mmx) {
    for (; height; height--) {
      _2xSaISuper2xSaILine (srcPtr, deltaPtr, srcPitch, width,
                            dstPtr, dstPitch);
      srcPtr += srcPitch;
      dstPtr += dstPitch * 2;
      deltaPtr += srcPitch;
    }
  } else
#endif
    {
      inc_bP = 1;

      for (; height; height--) {
        bP = (sai_uint16 *) srcPtr;
        dP = (sai_uint8 *) dstPtr;

        for (sai_uint32 finish = width; finish; finish -= inc_bP) {
          sai_uint32 color4, color5, color6;
          sai_uint32 color1, color2, color3;
          sai_uint32 colorA0, colorA1, colorA2, colorA3,
            colorB0, colorB1, colorB2, colorB3, colorS1, colorS2;
          sai_uint32 product1a, product1b, product2a, product2b;

          //---------------------------------------    B1 B2
          //                                         4  5  6 S2
          //                                         1  2  3 S1
          //                                           A1 A2

          colorB0 = *(bP - Nextline - 1);
          colorB1 = *(bP - Nextline);
          colorB2 = *(bP - Nextline + 1);
          colorB3 = *(bP - Nextline + 2);

          color4 = *(bP - 1);
          color5 = *(bP);
          color6 = *(bP + 1);
          colorS2 = *(bP + 2);

          color1 = *(bP + Nextline - 1);
          color2 = *(bP + Nextline);
          color3 = *(bP + Nextline + 1);
          colorS1 = *(bP + Nextline + 2);

          colorA0 = *(bP + Nextline + Nextline - 1);
          colorA1 = *(bP + Nextline + Nextline);
          colorA2 = *(bP + Nextline + Nextline + 1);
          colorA3 = *(bP + Nextline + Nextline + 2);

          //--------------------------------------
          if (color2 == color6 && color5 != color3) {
            product2b = product1b = color2;
          } else if (color5 == color3 && color2 != color6) {
            product2b = product1b = color5;
          } else if (color5 == color3 && color2 == color6) {
            register int r = 0;

            r += GetResult (color6, color5, color1, colorA1);
            r += GetResult (color6, color5, color4, colorB1);
            r += GetResult (color6, color5, colorA2, colorS1);
            r += GetResult (color6, color5, colorB2, colorS2);

            if (r > 0)
              product2b = product1b = color6;
            else if (r < 0)
              product2b = product1b = color5;
            else {
              product2b = product1b = INTERPOLATE (color5, color6);
            }
          } else {
            if (color6 == color3 && color3 == colorA1
                && color2 != colorA2 && color3 != colorA0)
              product2b =
                Q_INTERPOLATE (color3, color3, color3, color2);
            else if (color5 == color2 && color2 == colorA2
                     && colorA1 != color3 && color2 != colorA3)
              product2b =
                Q_INTERPOLATE (color2, color2, color2, color3);
            else
              product2b = INTERPOLATE (color2, color3);

            if (color6 == color3 && color6 == colorB1
                && color5 != colorB2 && color6 != colorB0)
              product1b =
                Q_INTERPOLATE (color6, color6, color6, color5);
            else if (color5 == color2 && color5 == colorB2
                     && colorB1 != color6 && color5 != colorB3)
              product1b =
                Q_INTERPOLATE (color6, color5, color5, color5);
            else
              product1b = INTERPOLATE (color5, color6);
          }

          if (color5 == color3 && color2 != color6 && color4 == color5
              && color5 != colorA2)
            product2a = INTERPOLATE (color2, color5);
          else
            if (color5 == color1 && color6 == color5
                && color4 != color2 && color5 != colorA0)
              product2a = INTERPOLATE (color2, color5);
            else
              product2a = color2;

          if (color2 == color6 && color5 != color3 && color1 == color2
              && color2 != colorB2)
            product1a = INTERPOLATE (color2, color5);
          else
            if (color4 == color2 && color3 == color2
                && color1 != color5 && color2 != colorB0)
              product1a = INTERPOLATE (color2, color5);
            else
              product1a = color5;

#ifdef WORDS_BIGENDIAN
          product1a = (product1a << 16) | product1b;
          product2a = (product2a << 16) | product2b;
#else
          product1a = product1a | (product1b << 16);
          product2a = product2a | (product2b << 16);
#endif

          *((sai_uint32 *) dP) = product1a;
          *((sai_uint32 *) (dP + dstPitch)) = product2a;

          bP += inc_bP;
          dP += sizeof (sai_uint32);
        }                       // end of for ( finish= width etc..)

        srcPtr   += srcPitch;
        dstPtr   += dstPitch << 1;
        deltaPtr += srcPitch;
      }                 // endof: for (; height; height--)
    }
}

void Super2xSaI32 (sai_uint8 *srcPtr, sai_uint32 srcPitch,
                   sai_uint8 *dstPtr, sai_uint32 dstPitch,
                   int width, int height)
{
  sai_uint32 *bP;
  sai_uint32 *dP;
  sai_uint32 inc_bP;
  sai_uint32 Nextline = srcPitch >> 2;
  inc_bP = 1;

  for (; height; height--) {
    bP = (sai_uint32 *) srcPtr;
    dP = (sai_uint32 *) dstPtr;

    for (sai_uint32 finish = width; finish; finish -= inc_bP) {
      sai_uint32 color4, color5, color6;
      sai_uint32 color1, color2, color3;
      sai_uint32 colorA0, colorA1, colorA2, colorA3,
        colorB0, colorB1, colorB2, colorB3, colorS1, colorS2;
      sai_uint32 product1a, product1b, product2a, product2b;

      //---------------------------------------    B1 B2
      //                                         4  5  6 S2
      //                                         1  2  3 S1
      //                                           A1 A2

      colorB0 = *(bP - Nextline - 1);
      colorB1 = *(bP - Nextline);
      colorB2 = *(bP - Nextline + 1);
      colorB3 = *(bP - Nextline + 2);

      color4 = *(bP - 1);
      color5 = *(bP);
      color6 = *(bP + 1);
      colorS2 = *(bP + 2);

      color1 = *(bP + Nextline - 1);
      color2 = *(bP + Nextline);
      color3 = *(bP + Nextline + 1);
      colorS1 = *(bP + Nextline + 2);

      colorA0 = *(bP + Nextline + Nextline - 1);
      colorA1 = *(bP + Nextline + Nextline);
      colorA2 = *(bP + Nextline + Nextline + 1);
      colorA3 = *(bP + Nextline + Nextline + 2);

      //--------------------------------------
      if (color2 == color6 && color5 != color3) {
        product2b = product1b = color2;
      } else if (color5 == color3 && color2 != color6) {
        product2b = product1b = color5;
      } else if (color5 == color3 && color2 == color6) {
        register int r = 0;

        r += GetResult (color6, color5, color1, colorA1);
        r += GetResult (color6, color5, color4, colorB1);
        r += GetResult (color6, color5, colorA2, colorS1);
        r += GetResult (color6, color5, colorB2, colorS2);

        if (r > 0)
          product2b = product1b = color6;
        else if (r < 0)
          product2b = product1b = color5;
        else {
          product2b = product1b = INTERPOLATE (color5, color6);
        }
      } else {
        if (color6 == color3 && color3 == colorA1
            && color2 != colorA2 && color3 != colorA0)
          product2b =
            Q_INTERPOLATE (color3, color3, color3, color2);
        else if (color5 == color2 && color2 == colorA2
                 && colorA1 != color3 && color2 != colorA3)
          product2b =
            Q_INTERPOLATE (color2, color2, color2, color3);
        else
          product2b = INTERPOLATE (color2, color3);

        if (color6 == color3 && color6 == colorB1
            && color5 != colorB2 && color6 != colorB0)
          product1b =
            Q_INTERPOLATE (color6, color6, color6, color5);
        else if (color5 == color2 && color5 == colorB2
                 && colorB1 != color6 && color5 != colorB3)
          product1b =
            Q_INTERPOLATE (color6, color5, color5, color5);
        else
          product1b = INTERPOLATE (color5, color6);
      }

      if (color5 == color3 && color2 != color6 && color4 == color5
          && color5 != colorA2)
        product2a = INTERPOLATE (color2, color5);
      else
        if (color5 == color1 && color6 == color5
            && color4 != color2 && color5 != colorA0)
          product2a = INTERPOLATE (color2, color5);
        else
          product2a = color2;

      if (color2 == color6 && color5 != color3 && color1 == color2
          && color2 != colorB2)
        product1a = INTERPOLATE (color2, color5);
      else
        if (color4 == color2 && color3 == color2
            && color1 != color5 && color2 != colorB0)
          product1a = INTERPOLATE (color2, color5);
        else
          product1a = color5;
      *(dP) = product1a;
      *(dP+1) = product1b;
      *(dP + (dstPitch >> 2)) = product2a;
      *(dP + (dstPitch >> 2) + 1) = product2b;

      bP += inc_bP;
      dP += 2;
    }                       // end of for ( finish= width etc..)

    srcPtr   += srcPitch;
    dstPtr   += dstPitch << 1;
    //        deltaPtr += srcPitch;
  }                 // endof: for (; height; height--)
}

void SuperEagle (sai_uint8 *srcPtr, sai_uint32 srcPitch, sai_uint8 *deltaPtr,
                 sai_uint8 *dstPtr, sai_uint32 dstPitch, int width, int height)
{
  sai_uint8  *dP;
  sai_uint16 *bP;
  sai_uint16 *xP;
  sai_uint32 inc_bP;

#ifdef MMX
  if (cpu_mmx) {
    for (; height; height--) {
      _2xSaISuperEagleLine (srcPtr, deltaPtr, srcPitch, width,
                            dstPtr, dstPitch);
      srcPtr += srcPitch;
      dstPtr += dstPitch * 2;
      deltaPtr += srcPitch;
    }
  } else
#endif
  {
    inc_bP = 1;

    sai_uint32 Nextline = srcPitch >> 1;

    for (; height; height--) {
      bP = (sai_uint16 *) srcPtr;
      xP = (sai_uint16 *) deltaPtr;
      dP = dstPtr;
      for (sai_uint32 finish = width; finish; finish -= inc_bP) {
        sai_uint32 color4, color5, color6;
        sai_uint32 color1, color2, color3;
        sai_uint32 colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
        sai_uint32 product1a, product1b, product2a, product2b;

        colorB1 = *(bP - Nextline);
        colorB2 = *(bP - Nextline + 1);

        color4 = *(bP - 1);
        color5 = *(bP);
        color6 = *(bP + 1);
        colorS2 = *(bP + 2);

        color1 = *(bP + Nextline - 1);
        color2 = *(bP + Nextline);
        color3 = *(bP + Nextline + 1);
        colorS1 = *(bP + Nextline + 2);

        colorA1 = *(bP + Nextline + Nextline);
        colorA2 = *(bP + Nextline + Nextline + 1);

        // --------------------------------------
        if (color2 == color6 && color5 != color3) {
          product1b = product2a = color2;
          if ((color1 == color2) || (color6 == colorB2)) {
            product1a = INTERPOLATE (color2, color5);
            product1a = INTERPOLATE (color2, product1a);
            //                       product1a = color2;
          } else {
            product1a = INTERPOLATE (color5, color6);
          }

          if ((color6 == colorS2) || (color2 == colorA1)) {
            product2b = INTERPOLATE (color2, color3);
            product2b = INTERPOLATE (color2, product2b);
            //                       product2b = color2;
          } else {
            product2b = INTERPOLATE (color2, color3);
          }
        } else if (color5 == color3 && color2 != color6) {
          product2b = product1a = color5;

          if ((colorB1 == color5) || (color3 == colorS1)) {
            product1b = INTERPOLATE (color5, color6);
            product1b = INTERPOLATE (color5, product1b);
            //                       product1b = color5;
          } else {
            product1b = INTERPOLATE (color5, color6);
          }

          if ((color3 == colorA2) || (color4 == color5)) {
            product2a = INTERPOLATE (color5, color2);
            product2a = INTERPOLATE (color5, product2a);
            //                       product2a = color5;
          } else {
            product2a = INTERPOLATE (color2, color3);
          }

        } else if (color5 == color3 && color2 == color6) {
          register int r = 0;

          r += GetResult (color6, color5, color1, colorA1);
          r += GetResult (color6, color5, color4, colorB1);
          r += GetResult (color6, color5, colorA2, colorS1);
          r += GetResult (color6, color5, colorB2, colorS2);

          if (r > 0) {
            product1b = product2a = color2;
            product1a = product2b = INTERPOLATE (color5, color6);
          } else if (r < 0) {
            product2b = product1a = color5;
            product1b = product2a = INTERPOLATE (color5, color6);
          } else {
            product2b = product1a = color5;
            product1b = product2a = color2;
          }
        } else {
          product2b = product1a = INTERPOLATE (color2, color6);
          product2b =
            Q_INTERPOLATE (color3, color3, color3, product2b);
          product1a =
            Q_INTERPOLATE (color5, color5, color5, product1a);

          product2a = product1b = INTERPOLATE (color5, color3);
          product2a =
            Q_INTERPOLATE (color2, color2, color2, product2a);
          product1b =
            Q_INTERPOLATE (color6, color6, color6, product1b);

          //                    product1a = color5;
          //                    product1b = color6;
          //                    product2a = color2;
          //                    product2b = color3;
        }
#ifdef WORDS_BIGENDIAN
        product1a = (product1a << 16) | product1b;
        product2a = (product2a << 16) | product2b;
#else
        product1a = product1a | (product1b << 16);
        product2a = product2a | (product2b << 16);
#endif

        *((sai_uint32 *) dP) = product1a;
        *((sai_uint32 *) (dP + dstPitch)) = product2a;
        *xP = color5;

        bP += inc_bP;
        xP += inc_bP;
        dP += sizeof (sai_uint32);
      }                 // end of for ( finish= width etc..)

      srcPtr += srcPitch;
      dstPtr += dstPitch << 1;
      deltaPtr += srcPitch;
    }                   // endof: for (height; height; height--)
  }
}

void SuperEagle32 (sai_uint8 *srcPtr, sai_uint32 srcPitch,
                   sai_uint8 *dstPtr, sai_uint32 dstPitch, int width, int height)
{
  sai_uint32  *dP;
  sai_uint32 *bP;
  sai_uint32 inc_bP;

  inc_bP = 1;

  sai_uint32 Nextline = srcPitch >> 2;

  for (; height; height--) {
    bP = (sai_uint32 *) srcPtr;
    dP = (sai_uint32 *)dstPtr;
    for (sai_uint32 finish = width; finish; finish -= inc_bP) {
      sai_uint32 color4, color5, color6;
      sai_uint32 color1, color2, color3;
      sai_uint32 colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
      sai_uint32 product1a, product1b, product2a, product2b;

      colorB1 = *(bP - Nextline);
      colorB2 = *(bP - Nextline + 1);

      color4 = *(bP - 1);
      color5 = *(bP);
      color6 = *(bP + 1);
      colorS2 = *(bP + 2);

      color1 = *(bP + Nextline - 1);
      color2 = *(bP + Nextline);
      color3 = *(bP + Nextline + 1);
      colorS1 = *(bP + Nextline + 2);

      colorA1 = *(bP + Nextline + Nextline);
      colorA2 = *(bP + Nextline + Nextline + 1);

      // --------------------------------------
      if (color2 == color6 && color5 != color3) {
        product1b = product2a = color2;
        if ((color1 == color2) || (color6 == colorB2)) {
          product1a = INTERPOLATE (color2, color5);
          product1a = INTERPOLATE (color2, product1a);
          //                       product1a = color2;
        } else {
          product1a = INTERPOLATE (color5, color6);
        }

        if ((color6 == colorS2) || (color2 == colorA1)) {
          product2b = INTERPOLATE (color2, color3);
          product2b = INTERPOLATE (color2, product2b);
          //                       product2b = color2;
        } else {
          product2b = INTERPOLATE (color2, color3);
        }
      } else if (color5 == color3 && color2 != color6) {
        product2b = product1a = color5;

        if ((colorB1 == color5) || (color3 == colorS1)) {
          product1b = INTERPOLATE (color5, color6);
          product1b = INTERPOLATE (color5, product1b);
          //                       product1b = color5;
        } else {
          product1b = INTERPOLATE (color5, color6);
        }

        if ((color3 == colorA2) || (color4 == color5)) {
          product2a = INTERPOLATE (color5, color2);
          product2a = INTERPOLATE (color5, product2a);
          //                       product2a = color5;
        } else {
          product2a = INTERPOLATE (color2, color3);
        }

      } else if (color5 == color3 && color2 == color6) {
        register int r = 0;

        r += GetResult (color6, color5, color1, colorA1);
        r += GetResult (color6, color5, color4, colorB1);
        r += GetResult (color6, color5, colorA2, colorS1);
        r += GetResult (color6, color5, colorB2, colorS2);

        if (r > 0) {
          product1b = product2a = color2;
          product1a = product2b = INTERPOLATE (color5, color6);
        } else if (r < 0) {
          product2b = product1a = color5;
          product1b = product2a = INTERPOLATE (color5, color6);
        } else {
          product2b = product1a = color5;
          product1b = product2a = color2;
        }
      } else {
        product2b = product1a = INTERPOLATE (color2, color6);
        product2b =
          Q_INTERPOLATE (color3, color3, color3, product2b);
        product1a =
          Q_INTERPOLATE (color5, color5, color5, product1a);

        product2a = product1b = INTERPOLATE (color5, color3);
        product2a =
          Q_INTERPOLATE (color2, color2, color2, product2a);
        product1b =
          Q_INTERPOLATE (color6, color6, color6, product1b);

        //                    product1a = color5;
        //                    product1b = color6;
        //                    product2a = color2;
        //                    product2b = color3;
      }
      *(dP) = product1a;
      *(dP+1) = product1b;
      *(dP + (dstPitch >> 2)) = product2a;
      *(dP + (dstPitch >> 2) +1) = product2b;

      bP += inc_bP;
      dP += 2;
    }                 // end of for ( finish= width etc..)

    srcPtr += srcPitch;
    dstPtr += dstPitch << 1;
  }                   // endof: for (height; height; height--)
}

void _2xSaI (sai_uint8 *srcPtr, sai_uint32 srcPitch, sai_uint8 *deltaPtr,
             sai_uint8 *dstPtr, sai_uint32 dstPitch, int width, int height)
{
  sai_uint8  *dP;
  sai_uint16 *bP;
  sai_uint32 inc_bP;

#ifdef MMX
  if (cpu_mmx) {
    for (; height; height -= 1) {
      _2xSaILine (srcPtr, deltaPtr, srcPitch, width, dstPtr, dstPitch);
      srcPtr += srcPitch;
      dstPtr += dstPitch * 2;
      deltaPtr += srcPitch;
    }
  } else
#endif
  {
    inc_bP = 1;

    sai_uint32 Nextline = srcPitch >> 1;

    for (; height; height--) {
      bP = (sai_uint16 *) srcPtr;
      dP = dstPtr;

      for (sai_uint32 finish = width; finish; finish -= inc_bP) {

        register sai_uint32 colorA, colorB;
        sai_uint32 colorC, colorD,
          colorE, colorF, colorG, colorH,
          colorI, colorJ, colorK, colorL,

          colorM, colorN, colorO, colorP;
        sai_uint32 product, product1, product2;

        //---------------------------------------
        // Map of the pixels:                    I|E F|J
        //                                       G|A B|K
        //                                       H|C D|L
        //                                       M|N O|P
        colorI = *(bP - Nextline - 1);
        colorE = *(bP - Nextline);
        colorF = *(bP - Nextline + 1);
        colorJ = *(bP - Nextline + 2);

        colorG = *(bP - 1);
        colorA = *(bP);
        colorB = *(bP + 1);
        colorK = *(bP + 2);

        colorH = *(bP + Nextline - 1);
        colorC = *(bP + Nextline);
        colorD = *(bP + Nextline + 1);
        colorL = *(bP + Nextline + 2);

        colorM = *(bP + Nextline + Nextline - 1);
        colorN = *(bP + Nextline + Nextline);
        colorO = *(bP + Nextline + Nextline + 1);
        colorP = *(bP + Nextline + Nextline + 2);

        if ((colorA == colorD) && (colorB != colorC)) {
          if (((colorA == colorE) && (colorB == colorL)) ||
              ((colorA == colorC) && (colorA == colorF)
               && (colorB != colorE) && (colorB == colorJ))) {
            product = colorA;
          } else {
            product = INTERPOLATE (colorA, colorB);
          }

          if (((colorA == colorG) && (colorC == colorO)) ||
              ((colorA == colorB) && (colorA == colorH)
               && (colorG != colorC) && (colorC == colorM))) {
            product1 = colorA;
          } else {
            product1 = INTERPOLATE (colorA, colorC);
          }
          product2 = colorA;
        } else if ((colorB == colorC) && (colorA != colorD)) {
          if (((colorB == colorF) && (colorA == colorH)) ||
              ((colorB == colorE) && (colorB == colorD)
               && (colorA != colorF) && (colorA == colorI))) {
            product = colorB;
          } else {
            product = INTERPOLATE (colorA, colorB);
          }

          if (((colorC == colorH) && (colorA == colorF)) ||
              ((colorC == colorG) && (colorC == colorD)
               && (colorA != colorH) && (colorA == colorI))) {
            product1 = colorC;
          } else {
            product1 = INTERPOLATE (colorA, colorC);
          }
          product2 = colorB;
        } else if ((colorA == colorD) && (colorB == colorC)) {
          if (colorA == colorB) {
            product = colorA;
            product1 = colorA;
            product2 = colorA;
          } else {
            register int r = 0;

            product1 = INTERPOLATE (colorA, colorC);
            product = INTERPOLATE (colorA, colorB);

            r +=
              GetResult1 (colorA, colorB, colorG, colorE,
                          colorI);
            r +=
              GetResult2 (colorB, colorA, colorK, colorF,
                          colorJ);
            r +=
              GetResult2 (colorB, colorA, colorH, colorN,
                          colorM);
            r +=
              GetResult1 (colorA, colorB, colorL, colorO,
                          colorP);

            if (r > 0)
              product2 = colorA;
            else if (r < 0)
              product2 = colorB;
            else {
              product2 =
                Q_INTERPOLATE (colorA, colorB, colorC,
                               colorD);
            }
          }
        } else {
          product2 = Q_INTERPOLATE (colorA, colorB, colorC, colorD);

          if ((colorA == colorC) && (colorA == colorF)
              && (colorB != colorE) && (colorB == colorJ)) {
            product = colorA;
          } else if ((colorB == colorE) && (colorB == colorD)
                     && (colorA != colorF) && (colorA == colorI)) {
            product = colorB;
          } else {
            product = INTERPOLATE (colorA, colorB);
          }

          if ((colorA == colorB) && (colorA == colorH)
              && (colorG != colorC) && (colorC == colorM)) {
            product1 = colorA;
          } else if ((colorC == colorG) && (colorC == colorD)
                     && (colorA != colorH) && (colorA == colorI)) {
            product1 = colorC;
          } else {
            product1 = INTERPOLATE (colorA, colorC);
          }
        }

#ifdef WORDS_BIGENDIAN
        product = (colorA << 16) | product ;
        product1 = (product1 << 16) | product2 ;
#else
        product = colorA | (product << 16);
        product1 = product1 | (product2 << 16);
#endif
        *((int *) dP) = product;
        *((sai_uint32 *) (dP + dstPitch)) = product1;

        bP += inc_bP;
        dP += sizeof (sai_uint32);
      }                 // end of for ( finish= width etc..)

      srcPtr += srcPitch;
      dstPtr += dstPitch << 1;
      deltaPtr += srcPitch;
    }                   // endof: for (height; height; height--)
  }
}

void _2xSaI32 (sai_uint8 *srcPtr, sai_uint32 srcPitch,
               sai_uint8 *dstPtr, sai_uint32 dstPitch, int width, int height)
{
  sai_uint32  *dP;
  sai_uint32 *bP;
  sai_uint32 inc_bP = 1;

  sai_uint32 Nextline = srcPitch >> 2;

  for (; height; height--) {
    bP = (sai_uint32 *) srcPtr;
    dP = (sai_uint32 *) dstPtr;

    for (sai_uint32 finish = width; finish; finish -= inc_bP) {
      register sai_uint32 colorA, colorB;
      sai_uint32 colorC, colorD,
        colorE, colorF, colorG, colorH,
        colorI, colorJ, colorK, colorL,

        colorM, colorN, colorO, colorP;
      sai_uint32 product, product1, product2;

      //---------------------------------------
      // Map of the pixels:                    I|E F|J
      //                                       G|A B|K
      //                                       H|C D|L
      //                                       M|N O|P
      colorI = *(bP - Nextline - 1);
      colorE = *(bP - Nextline);
      colorF = *(bP - Nextline + 1);
      colorJ = *(bP - Nextline + 2);

      colorG = *(bP - 1);
      colorA = *(bP);
      colorB = *(bP + 1);
      colorK = *(bP + 2);

      colorH = *(bP + Nextline - 1);
      colorC = *(bP + Nextline);
      colorD = *(bP + Nextline + 1);
      colorL = *(bP + Nextline + 2);

      colorM = *(bP + Nextline + Nextline - 1);
      colorN = *(bP + Nextline + Nextline);
      colorO = *(bP + Nextline + Nextline + 1);
      colorP = *(bP + Nextline + Nextline + 2);

      if ((colorA == colorD) && (colorB != colorC)) {
        if (((colorA == colorE) && (colorB == colorL)) ||
            ((colorA == colorC) && (colorA == colorF)
             && (colorB != colorE) && (colorB == colorJ))) {
          product = colorA;
        } else {
          product = INTERPOLATE (colorA, colorB);
        }

        if (((colorA == colorG) && (colorC == colorO)) ||
            ((colorA == colorB) && (colorA == colorH)
             && (colorG != colorC) && (colorC == colorM))) {
          product1 = colorA;
        } else {
          product1 = INTERPOLATE (colorA, colorC);
        }
        product2 = colorA;
      } else if ((colorB == colorC) && (colorA != colorD)) {
        if (((colorB == colorF) && (colorA == colorH)) ||
            ((colorB == colorE) && (colorB == colorD)
             && (colorA != colorF) && (colorA == colorI))) {
          product = colorB;
        } else {
          product = INTERPOLATE (colorA, colorB);
        }

        if (((colorC == colorH) && (colorA == colorF)) ||
            ((colorC == colorG) && (colorC == colorD)
             && (colorA != colorH) && (colorA == colorI))) {
          product1 = colorC;
        } else {
          product1 = INTERPOLATE (colorA, colorC);
        }
        product2 = colorB;
      } else if ((colorA == colorD) && (colorB == colorC)) {
        if (colorA == colorB) {
          product = colorA;
          product1 = colorA;
          product2 = colorA;
        } else {
          register int r = 0;

          product1 = INTERPOLATE (colorA, colorC);
          product = INTERPOLATE (colorA, colorB);

          r +=
            GetResult1 (colorA, colorB, colorG, colorE,
                        colorI);
          r +=
            GetResult2 (colorB, colorA, colorK, colorF,
                        colorJ);
          r +=
            GetResult2 (colorB, colorA, colorH, colorN,
                           colorM);
          r +=
            GetResult1 (colorA, colorB, colorL, colorO,
                           colorP);

          if (r > 0)
            product2 = colorA;
          else if (r < 0)
            product2 = colorB;
          else {
            product2 =
              Q_INTERPOLATE (colorA, colorB, colorC,
                               colorD);
            }
        }
      } else {
        product2 = Q_INTERPOLATE (colorA, colorB, colorC, colorD);

        if ((colorA == colorC) && (colorA == colorF)
            && (colorB != colorE) && (colorB == colorJ)) {
          product = colorA;
        } else if ((colorB == colorE) && (colorB == colorD)
                   && (colorA != colorF) && (colorA == colorI)) {
          product = colorB;
        } else {
          product = INTERPOLATE (colorA, colorB);
        }

        if ((colorA == colorB) && (colorA == colorH)
            && (colorG != colorC) && (colorC == colorM)) {
          product1 = colorA;
        } else if ((colorC == colorG) && (colorC == colorD)
                   && (colorA != colorH) && (colorA == colorI)) {
          product1 = colorC;
        } else {
          product1 = INTERPOLATE (colorA, colorC);
        }
      }
      *(dP) = colorA;
      *(dP + 1) = product;
      *(dP + (dstPitch >> 2)) = product1;
      *(dP + (dstPitch >> 2) + 1) = product2;

      bP += inc_bP;
      dP += 2;
    }                 // end of for ( finish= width etc..)

    srcPtr += srcPitch;
    dstPtr += dstPitch << 1;
    //    deltaPtr += srcPitch;
  }                   // endof: for (height; height; height--)
}

static sai_uint32 Bilinear (sai_uint32 A, sai_uint32 B, sai_uint32 x)
{
  unsigned long areaA, areaB;
  unsigned long result;

  if (A == B)
    return A;

  areaB = (x >> 11) & 0x1f;     // reduce 16 bit fraction to 5 bits
  areaA = 0x20 - areaB;

  A = (A & redblueMask) | ((A & greenMask) << 16);
  B = (B & redblueMask) | ((B & greenMask) << 16);

  result = ((areaA * A) + (areaB * B)) >> 5;

  return (result & redblueMask) | ((result >> 16) & greenMask);
}

static sai_uint32 Bilinear4 (sai_uint32 A, sai_uint32 B, sai_uint32 C, sai_uint32 D, sai_uint32 x,
                         sai_uint32 y)
{
  unsigned long areaA, areaB, areaC, areaD;
  unsigned long result, xy;

  x = (x >> 11) & 0x1f;
  y = (y >> 11) & 0x1f;
  xy = (x * y) >> 5;

  A = (A & redblueMask) | ((A & greenMask) << 16);
  B = (B & redblueMask) | ((B & greenMask) << 16);
  C = (C & redblueMask) | ((C & greenMask) << 16);
  D = (D & redblueMask) | ((D & greenMask) << 16);

  areaA = 0x20 + xy - x - y;
  areaB = x - xy;
  areaC = y - xy;
  areaD = xy;

  result = ((areaA * A) + (areaB * B) + (areaC * C) + (areaD * D)) >> 5;

  return (result & redblueMask) | ((result >> 16) & greenMask);
}

void Scale_2xSaI (sai_uint8 *srcPtr, sai_uint32 srcPitch, sai_uint8 * /* deltaPtr */,
                  sai_uint8 *dstPtr, sai_uint32 dstPitch,
                  sai_uint32 dstWidth, sai_uint32 dstHeight, int width, int height)
{
  sai_uint8  *dP;
  sai_uint16 *bP;

  sai_uint32 w;
  sai_uint32 h;
  sai_uint32 dw;
  sai_uint32 dh;
  sai_uint32 hfinish;
  sai_uint32 wfinish;

  sai_uint32 Nextline = srcPitch >> 1;

  wfinish = (width - 1) << 16;  // convert to fixed point
  dw = wfinish / (dstWidth - 1);
  hfinish = (height - 1) << 16; // convert to fixed point
  dh = hfinish / (dstHeight - 1);

  for (h = 0; h < hfinish; h += dh) {
    sai_uint32 y1, y2;

    y1 = h & 0xffff;    // fraction part of fixed point
    bP = (sai_uint16 *) (srcPtr + ((h >> 16) * srcPitch));
    dP = dstPtr;
    y2 = 0x10000 - y1;

    w = 0;

    for (; w < wfinish;) {
      sai_uint32 A, B, C, D;
      sai_uint32 E, F, G, H;
      sai_uint32 I, J, K, L;
      sai_uint32 x1, x2, a1, f1, f2;
      sai_uint32 position, product1;

      position = w >> 16;
      A = bP[position]; // current pixel
      B = bP[position + 1];     // next pixel
      C = bP[position + Nextline];
      D = bP[position + Nextline + 1];
      E = bP[position - Nextline];
      F = bP[position - Nextline + 1];
      G = bP[position - 1];
      H = bP[position + Nextline - 1];
      I = bP[position + 2];
      J = bP[position + Nextline + 2];
      K = bP[position + Nextline + Nextline];
      L = bP[position + Nextline + Nextline + 1];

      x1 = w & 0xffff;  // fraction part of fixed point
      x2 = 0x10000 - x1;

      /*0*/
      if (A == B && C == D && A == C)
        product1 = A;
      else /*1*/ if (A == D && B != C) {
        f1 = (x1 >> 1) + (0x10000 >> 2);
        f2 = (y1 >> 1) + (0x10000 >> 2);
        if (y1 <= f1 && A == J && A != E)       // close to B
          {
            a1 = f1 - y1;
            product1 = Bilinear (A, B, a1);
          } else if (y1 >= f1 && A == G && A != L)      // close to C
            {
              a1 = y1 - f1;
              product1 = Bilinear (A, C, a1);
            }
        else if (x1 >= f2 && A == E && A != J)  // close to B
          {
            a1 = x1 - f2;
            product1 = Bilinear (A, B, a1);
          }
        else if (x1 <= f2 && A == L && A != G)  // close to C
          {
            a1 = f2 - x1;
            product1 = Bilinear (A, C, a1);
          }
        else if (y1 >= x1)      // close to C
          {
            a1 = y1 - x1;
            product1 = Bilinear (A, C, a1);
          }
        else if (y1 <= x1)      // close to B
          {
            a1 = x1 - y1;
            product1 = Bilinear (A, B, a1);
          }
      }
      else
        /*2*/
        if (B == C && A != D)
          {
            f1 = (x1 >> 1) + (0x10000 >> 2);
            f2 = (y1 >> 1) + (0x10000 >> 2);
            if (y2 >= f1 && B == H && B != F)   // close to A
              {
                a1 = y2 - f1;
                product1 = Bilinear (B, A, a1);
              }
            else if (y2 <= f1 && B == I && B != K)      // close to D
              {
                a1 = f1 - y2;
                product1 = Bilinear (B, D, a1);
              }
            else if (x2 >= f2 && B == F && B != H)      // close to A
              {
                a1 = x2 - f2;
                product1 = Bilinear (B, A, a1);
              }
            else if (x2 <= f2 && B == K && B != I)      // close to D
              {
                a1 = f2 - x2;
                product1 = Bilinear (B, D, a1);
              }
            else if (y2 >= x1)  // close to A
              {
                a1 = y2 - x1;
                product1 = Bilinear (B, A, a1);
              }
            else if (y2 <= x1)  // close to D
              {
                a1 = x1 - y2;
                product1 = Bilinear (B, D, a1);
              }
          }
      /*3*/
        else
          {
            product1 = Bilinear4 (A, B, C, D, x1, y1);
          }

      //end First Pixel
      *(sai_uint32 *) dP = product1;
      dP += 2;
      w += dw;
    }
    dstPtr += dstPitch;
  }
}
/*
int Init_2xSaI(sai_uint32 BitFormat) {
	// Store some stuff
//	push ebp
//	mov ebp, esp
//	push edx

	__asm__ __volatile__(
		 //"movq 0(%1), %%mm0\n"
         "mov eax, (%1)\n"         //PixelFormat
         "cmp eax, 555\n"
         "jz Bits555\n"
         "cmp eax, 565\n"
         "jz Bits565\n"
"end2:\n"
         "mov eax, 1\n"
         "jmp end3\n"
"Bits555:\n"
         "mov edx, 0x7BDE7BDE\n"
         "mov eax, colorMask\n"
         "mov [eax], edx\n"
         "mov [eax+4], edx\n"
         "mov edx, 0x04210421\n"
         "mov eax, lowPixelMask\n"
         "mov [eax], edx\n"
         "mov [eax+4], edx\n"
         "mov edx, 0x739C739C\n"
         "mov eax, qcolorMask\n"
         "mov [eax], edx\n"
         "mov [eax+4], edx\n"
         "mov edx, 0x0C630C63\n"
         "mov eax, qlowpixelMask\n"
         "mov [eax], edx\n"
         "mov [eax+4], edx\n"
         "mov eax, 0\n"
         "jmp end3\n"
"Bits565:\n"
         "mov edx, 0xF7DEF7DE\n"
         "mov eax, colorMask\n"
         "mov [eax], edx\n"
         "mov [eax+4], edx\n"
         "mov edx, 0x08210821\n"
         "mov eax, lowPixelMask\n"
         "mov [eax], edx\n"
         "mov [eax+4], edx\n"
         "mov edx, 0xE79CE79C\n"
         "mov eax, qcolorMask\n"
         "mov [eax], edx\n"
         "mov [eax+4], edx\n"
         "mov edx, 0x18631863\n"
         "mov eax, qlowpixelMask\n"
         "mov [eax], edx\n"
         "mov [eax+4], edx\n"
         "mov eax, 0\n"
         "jmp end3\n"
"end3:\n"
//         "pop edx\n"
//         "mov esp, ebp\n"
//         "pop ebp\n"
//         "ret\n"
		: "+r" (BitFormat)
		:
		: "cc"
	);
}*/
