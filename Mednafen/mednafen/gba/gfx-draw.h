// -*- C++ -*-
// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2005 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef VBA_GFX_DRAW_H
#define VBA_GFX_DRAW_H

#include "Gfx.h"

//#define SPRITE_DEBUG

void gfxDrawTextScreen(uint16, uint16, uint16, uint32 *);
void gfxDrawRotScreen(uint16,
                      uint16, uint16,
                      uint16, uint16,
                      uint16, uint16,
                      uint16, uint16,
                      int&, int&,
                      int,
                      uint32*);
void gfxDrawRotScreen16Bit(uint16,
                           uint16, uint16,
                           uint16, uint16,
                           uint16, uint16,
                           uint16, uint16,
                           int&, int&,
                           int,
                           uint32*);
void gfxDrawRotScreen256(uint16,
                         uint16, uint16,
                         uint16, uint16,
                         uint16, uint16,
                         uint16, uint16,
                         int&, int&,
                         int,
                         uint32*);
void gfxDrawRotScreen16Bit160(uint16,
                              uint16, uint16,
                              uint16, uint16,
                              uint16, uint16,
                              uint16, uint16,
                              int&, int&,
                              int,
                              uint32*);
void gfxDrawSprites(void);

void gfxDrawOBJWin(void);



//void gfxIncreaseBrightness(uint32 *line, int coeff);
//void gfxDecreaseBrightness(uint32 *line, int coeff);
//void gfxAlphaBlend(uint32 *ta, uint32 *tb, int ca, int cb);

extern uint32 AlphaClampLUT[64];
extern MDFN_ALIGN(16) uint32 line0[512];
extern MDFN_ALIGN(16) uint32 line1[512];
extern MDFN_ALIGN(16) uint32 line2[512];
extern MDFN_ALIGN(16) uint32 line3[512];
extern MDFN_ALIGN(16) uint32 lineOBJ[512];
extern MDFN_ALIGN(16) uint32 lineOBJWin[512];
extern MDFN_ALIGN(16) uint32 lineMix[512];
extern bool gfxInWin0[512];
extern bool gfxInWin1[512];

extern int gfxBG2Changed;
extern int gfxBG3Changed;

extern int gfxBG2X;
extern int gfxBG2Y;
extern int gfxBG2LastX;
extern int gfxBG2LastY;
extern int gfxBG3X;
extern int gfxBG3Y;
extern int gfxBG3LastX;
extern int gfxBG3LastY;
extern int gfxLastVCOUNT;

static INLINE void gfxClearArray(uint32 *array)
{
 MDFN_FastU32MemsetM8(array, 0x80000000, 240);
 // for(int i = 0; i < 240; i++) {
 //   *array++ = 0x80000000;
 // }
}

// Max coefficient is 16, so...
static INLINE uint32 gfxIncreaseBrightness(uint32 color, int coeff)
{
  int r = (color & 0x1F);
  int g = ((color >> 5) & 0x1F);
  int b = ((color >> 10) & 0x1F);
  
  r = r + (((31 - r) * coeff) >> 4);
  g = g + (((31 - g) * coeff) >> 4);
  b = b + (((31 - b) * coeff) >> 4);
  //if(r > 31)
  //  r = 31;
  //if(g > 31)
  //  g = 31;
  //if(b > 31)
  //  b = 31;
  color = (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
  return color;
}

static INLINE void gfxIncreaseBrightness(uint32 *line, int coeff)
{
  for(int x = 0; x < 240; x++) {
    uint32 color = *line;
    int r = (color & 0x1F);
    int g = ((color >> 5) & 0x1F);
    int b = ((color >> 10) & 0x1F);
    
    r = r + (((31 - r) * coeff) >> 4);
    g = g + (((31 - g) * coeff) >> 4);
    b = b + (((31 - b) * coeff) >> 4);
    //if(r > 31)
    //  r = 31;
    //if(g > 31)
    //  g = 31;
    //if(b > 31)
    //  b = 31;
    *line++ = (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
  }
}

static INLINE uint32 gfxDecreaseBrightness(uint32 color, int coeff)
{
  int r = (color & 0x1F);
  int g = ((color >> 5) & 0x1F);
  int b = ((color >> 10) & 0x1F);
  
  r = r - ((r * coeff) >> 4);
  g = g - ((g * coeff) >> 4);
  b = b - ((b * coeff) >> 4);
  //if(r < 0)
  //  r = 0;
  //if(g < 0)
  //  g = 0;
  //if(b < 0)
  //  b = 0;
  color = (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
  
  return color;
}

static INLINE void gfxDecreaseBrightness(uint32 *line, int coeff)
{
  for(int x = 0; x < 240; x++) {
    uint32 color = *line;
    int r = (color & 0x1F);
    int g = ((color >> 5) & 0x1F);
    int b = ((color >> 10) & 0x1F);
    
    r = r - ((r * coeff) >> 4);
    g = g - ((g * coeff) >> 4);
    b = b - ((b * coeff) >> 4);
    //if(r < 0)
    //  r = 0;
    //if(g < 0)
    //  g = 0;
    //if(b < 0)
    //  b = 0;
    *line++ = (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
  }
}

static INLINE uint32 gfxAlphaBlend(uint32 color, uint32 color2, int ca, int cb)
{
  if(color < 0x80000000) {
    int r = (color & 0x1F);
    int g = ((color >> 5) & 0x1F);
    int b = ((color >> 10) & 0x1F);
    int r0 = (color2 & 0x1F);
    int g0 = ((color2 >> 5) & 0x1F);
    int b0 = ((color2 >> 10) & 0x1F);
    
    r = AlphaClampLUT[((r * ca) >> 4) + ((r0 * cb) >> 4)];
    g = AlphaClampLUT[((g * ca) >> 4) + ((g0 * cb) >> 4)];
    b = AlphaClampLUT[((b * ca) >> 4) + ((b0 * cb) >> 4)];
    
    return (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
  }
  return color;
}

static INLINE void gfxAlphaBlend(uint32 *ta, uint32 *tb, int ca, int cb)
{
  for(int x = 0; x < 240; x++) {
    uint32 color = *ta;
    if(color < 0x80000000) {
      int r = (color & 0x1F);
      int g = ((color >> 5) & 0x1F);
      int b = ((color >> 10) & 0x1F);
      uint32 color2 = (*tb++);
      int r0 = (color2 & 0x1F);
      int g0 = ((color2 >> 5) & 0x1F);
      int b0 = ((color2 >> 10) & 0x1F);
      
      r = AlphaClampLUT[((r * ca) >> 4) + ((r0 * cb) >> 4)];
      g = AlphaClampLUT[((g * ca) >> 4) + ((g0 * cb) >> 4)];
      b = AlphaClampLUT[((b * ca) >> 4) + ((b0 * cb) >> 4)];
      
      *ta++ = (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
    } else {
      ta++;
      tb++;
    }
  }
}

#endif // VBA_GFX_DRAW_H
