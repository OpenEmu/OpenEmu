// -*- C++ -*-
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

#ifndef VBA_GLOBALS_H
#define VBA_GLOBALS_H

#define VERBOSE_SWI                  1
#define VERBOSE_UNALIGNED_MEMORY     2
#define VERBOSE_ILLEGAL_WRITE        4
#define VERBOSE_ILLEGAL_READ         8
#define VERBOSE_DMA0                16
#define VERBOSE_DMA1                32
#define VERBOSE_DMA2                64
#define VERBOSE_DMA3               128
#define VERBOSE_UNDEFINED          256
#define VERBOSE_AGBPRINT           512

extern reg_pair reg[45];
extern bool ioReadable[0x400];

extern uint32 N_FLAG;
extern bool C_FLAG;
extern bool Z_FLAG;
extern bool V_FLAG;

extern bool armState;
extern bool armIrqEnable;
extern uint32 armNextPC;
extern int armMode;
extern uint32 stop;
extern int saveType;
extern bool useBios;
extern bool skipBios;
extern int frameSkip;
extern bool speedup;
extern bool synchronize;
extern bool cpuDisableSfx;
extern bool cpuIsMultiBoot;
extern bool parseDebug;
extern int layerSettings;
extern int layerEnable;

extern uint8 *bios;
extern uint8 *rom;
extern uint8 *internalRAM;
extern uint8 *workRAM;
extern uint8 *paletteRAM;
extern uint8 *vram;
extern uint8 *pix;
extern uint8 *oam;
extern uint8 *ioMem;

extern uint16 DISPCNT;
extern uint16 DISPSTAT;
extern uint16 VCOUNT;
extern uint16 BG0CNT;
extern uint16 BG1CNT;
extern uint16 BG2CNT;
extern uint16 BG3CNT;

extern uint16 BGHOFS[4];
extern uint16 BGVOFS[4];

extern uint16 BG2PA;
extern uint16 BG2PB;
extern uint16 BG2PC;
extern uint16 BG2PD;
extern uint16 BG2X_L;
extern uint16 BG2X_H;
extern uint16 BG2Y_L;
extern uint16 BG2Y_H;
extern uint16 BG3PA;
extern uint16 BG3PB;
extern uint16 BG3PC;
extern uint16 BG3PD;
extern uint16 BG3X_L;
extern uint16 BG3X_H;
extern uint16 BG3Y_L;
extern uint16 BG3Y_H;
extern uint16 WIN0H;
extern uint16 WIN1H;
extern uint16 WIN0V;
extern uint16 WIN1V;
extern uint16 WININ;
extern uint16 WINOUT;
extern uint16 MOSAIC;
extern uint16 BLDMOD;
extern uint16 COLEV;
extern uint16 COLY;

extern uint16 DMSAD_L[4];
extern uint16 DMSAD_H[4];
extern uint16 DMDAD_L[4];
extern uint16 DMDAD_H[4];
extern uint16 DMCNT_L[4];
extern uint16 DMCNT_H[4];

extern uint16 TM0D;
extern uint16 TM0CNT;
extern uint16 TM1D;
extern uint16 TM1CNT;
extern uint16 TM2D;
extern uint16 TM2CNT;
extern uint16 TM3D;
extern uint16 TM3CNT;
extern uint16 P1;
extern uint16 IE;
extern uint16 IF;
extern uint16 IME;

#endif // VBA_GLOBALS_H
