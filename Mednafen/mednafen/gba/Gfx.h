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

#ifndef VBA_GFX_H
#define VBA_GFX_H

#include "GBA.h"
#include "Gfx.h"
#include "Globals.h"

#include "Port.h"

//#define SPRITE_DEBUG

void mode0RenderLine();
void mode0RenderLineNoWindow();
void mode0RenderLineAll();

void mode1RenderLine();
void mode1RenderLineNoWindow();
void mode1RenderLineAll();

void mode2RenderLine();
void mode2RenderLineNoWindow();
void mode2RenderLineAll();

void mode3RenderLine();
void mode3RenderLineNoWindow();
void mode3RenderLineAll();

void mode4RenderLine();
void mode4RenderLineNoWindow();
void mode4RenderLineAll();

void mode5RenderLine();
void mode5RenderLineNoWindow();
void mode5RenderLineAll();

extern int all_coeff[32];
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

#endif // VBA_GFX_H
