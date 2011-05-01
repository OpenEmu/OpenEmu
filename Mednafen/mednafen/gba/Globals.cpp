// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

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

#include "GBA.h"

reg_pair reg[45];
memoryMap map[256];
bool ioReadable[0x400];

uint32 N_FLAG = 0;
bool C_FLAG = 0;
bool Z_FLAG = 0;
bool V_FLAG = 0;

bool armState = true;
bool armIrqEnable = true;
uint32 armNextPC = 0x00000000;
int armMode = 0x1f;
uint32 stop = 0x08000568;
int saveType = 0;
bool useBios = false;
bool skipBios = false;
int frameSkip = 1;
bool speedup = false;
bool synchronize = true;
bool cpuDisableSfx = false;
bool cpuIsMultiBoot = false;
bool parseDebug = true;
int layerSettings = 0xff00;
int layerEnable = 0xff00;

uint8 *bios = NULL;
uint8 *rom = NULL;
uint8 *internalRAM = NULL;
uint8 *workRAM = NULL;
uint8 *paletteRAM = NULL;
uint8 *vram = NULL;
uint8 *pix = NULL;
uint8 *oam = NULL;
uint8 *ioMem = NULL;

uint16 DISPCNT  = 0x0080;
uint16 DISPSTAT = 0x0000;
uint16 VCOUNT   = 0x0000;
uint16 BG0CNT   = 0x0000;
uint16 BG1CNT   = 0x0000;
uint16 BG2CNT   = 0x0000;
uint16 BG3CNT   = 0x0000;

uint16 BGHOFS[4];
uint16 BGVOFS[4];

uint16 BG2PA    = 0x0100;
uint16 BG2PB    = 0x0000;
uint16 BG2PC    = 0x0000;
uint16 BG2PD    = 0x0100;
uint16 BG2X_L   = 0x0000;
uint16 BG2X_H   = 0x0000;
uint16 BG2Y_L   = 0x0000;
uint16 BG2Y_H   = 0x0000;
uint16 BG3PA    = 0x0100;
uint16 BG3PB    = 0x0000;
uint16 BG3PC    = 0x0000;
uint16 BG3PD    = 0x0100;
uint16 BG3X_L   = 0x0000;
uint16 BG3X_H   = 0x0000;
uint16 BG3Y_L   = 0x0000;
uint16 BG3Y_H   = 0x0000;
uint16 WIN0H    = 0x0000;
uint16 WIN1H    = 0x0000;
uint16 WIN0V    = 0x0000;
uint16 WIN1V    = 0x0000;
uint16 WININ    = 0x0000;
uint16 WINOUT   = 0x0000;
uint16 MOSAIC   = 0x0000;
uint16 BLDMOD   = 0x0000;
uint16 COLEV    = 0x0000;
uint16 COLY     = 0x0000;

uint16 DMSAD_L[4] = {0};
uint16 DMSAD_H[4] = {0};
uint16 DMDAD_L[4] = {0};
uint16 DMDAD_H[4] = {0};
uint16 DMCNT_L[4] = {0};
uint16 DMCNT_H[4] = {0};

uint16 P1       = 0xFFFF;
uint16 IE       = 0x0000;
uint16 IF       = 0x0000;
uint16 IME      = 0x0000;
