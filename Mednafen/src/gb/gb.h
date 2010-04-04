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

#ifndef VBA_GB_GB_H
#define VBA_GB_GB_H

#define C_FLAG 0x10
#define H_FLAG 0x20
#define N_FLAG 0x40
#define Z_FLAG 0x80

typedef union {
  struct {
#ifndef LSB_FIRST
    uint8 B1, B0;
#else
    uint8 B0,B1;
#endif
  } B;
  uint16 W;
} gbRegister;

extern uint8 gbOAM[0xA0];
extern uint8 register_IF, register_IE, register_KEY1;

extern void gbEmulate(int);
extern void gbSoundReset();
extern void gbReset();
extern void gbCleanUp();
extern bool gbWriteBatteryFile(const char *);
extern bool gbWriteBatteryFile(const char *, bool);
extern bool gbReadBatteryFile(const char *);
extern bool gbWriteSaveState(const char *);
extern bool gbWriteMemSaveState(char *, int);
extern bool gbReadSaveState(const char *);
extern bool gbReadMemSaveState(char *, int);
extern void gbSgbRenderBorder();
extern bool gbWritePNGFile(const char *);
extern bool gbWriteBMPFile(const char *);
extern bool gbReadGSASnapshot(const char *);

extern struct EmulatedSystem GBSystem;


void gbWriteMemory(uint16 address, uint8 value);
uint8 gbReadMemory(uint16 address);
void gbSpeedSwitch();

#endif
