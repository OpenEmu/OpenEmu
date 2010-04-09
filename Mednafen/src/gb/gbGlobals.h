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

extern int gbRomSizeMask;
extern int gbRomSize;
extern int gbRamSize;
extern int gbRamSizeMask;

extern uint8 *gbRom;
extern uint8 *gbRam;
extern uint8 *gbVram;
extern uint8 *gbWram;
extern uint16 *gbLineBuffer;

extern uint8 *gbMemoryMap[16];

extern int gbFrameSkip;
extern int gbPaletteOption;
extern int gbEmulatorType;
extern int gbCgbMode;
extern int gbSgbMode;
extern int gbWindowLine;
extern int gbSpeed;
extern uint8 gbBgp[4];
extern uint8 gbObp0[4];
extern uint8 gbObp1[4];
extern uint16 gbPalette[128];

extern uint8 register_LCDC;
extern uint8 register_LY;
extern uint8 register_SCY;
extern uint8 register_SCX;
extern uint8 register_WY;
extern uint8 register_WX;
extern uint8 register_VBK;

extern int emulating;

extern int gbDmaTicks;

extern void gbRenderLine();
extern void gbDrawSprites();

extern uint32 gblayerSettings;
extern uint8 (*gbSerialFunction)(uint8);
