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

#ifndef VBA_GBA_H
#define VBA_GBA_H

#include "../mednafen.h"
#include <zlib.h>

typedef struct {
  uint8 *address;
  uint32 mask;
} memoryMap;

typedef union {
  struct {
#ifdef MSB_FIRST
    uint8 B3;
    uint8 B2;
    uint8 B1;
    uint8 B0;
#else
    uint8 B0;
    uint8 B1;
    uint8 B2;
    uint8 B3;
#endif
  } B;
  struct {
#ifdef MSB_FIRST
    uint16 W1;
    uint16 W0;
#else
    uint16 W0;
    uint16 W1;
#endif
  } W;
#ifdef MSB_FIRST
  volatile uint32 I;
#else
	uint32 I;
#endif
} reg_pair;

#ifndef NO_GBA_MAP
extern memoryMap map[256];
#endif

extern bool busPrefetch;
extern bool busPrefetchEnable;
extern uint32 busPrefetchCount;
extern uint32 cpuPrefetch[2];


extern uint8 memoryWait[16];
extern uint8 memoryWait32[16];
extern uint8 memoryWaitSeq[16];
extern uint8 memoryWaitSeq32[16];

extern reg_pair reg[45];
extern uint8 biosProtected[4];

extern uint32 N_FLAG;
extern bool Z_FLAG;
extern bool C_FLAG;
extern bool V_FLAG;
extern bool armIrqEnable;
extern bool armState;
extern int armMode;
extern void (*cpuSaveGameFunc)(uint32,uint8);

extern bool CPUWriteBatteryFile(const char *);
extern bool CPUReadBatteryFile(const char *);

extern void CPUCleanUp();
extern void CPUUpdateRender();
extern void doMirroring(bool);
extern void CPUUpdateRegister(uint32, uint16);
extern void applyTimer ();

void CPUWriteMemory(uint32 address, uint32 value);
void CPUWriteHalfWord(uint32, uint16);
void CPUWriteByte(uint32, uint8);

extern void CPULoop(int);
extern void CPUCheckDMA(int,int);

extern void CPUSwitchMode(int mode, bool saveState, bool breakLoop);
extern void CPUSwitchMode(int mode, bool saveState);
extern void CPUUndefinedException();
extern void CPUSoftwareInterrupt();
extern void CPUSoftwareInterrupt(int comment);
extern void CPUCompareVCOUNT();
extern void CPUUpdateCPSR();
extern void CPUUpdateFlags(bool breakLoop);
extern void CPUUpdateFlags();


extern uint8 cpuBitsSet[256];
extern uint8 cpuLowestBitSet[256];

extern struct EmulatedSystem GBASystem;

int32 MDFNGBA_GetTimerPeriod(int which);


#define R13_IRQ  18
#define R14_IRQ  19
#define SPSR_IRQ 20
#define R13_USR  26
#define R14_USR  27
#define R13_SVC  28
#define R14_SVC  29
#define SPSR_SVC 30
#define R13_ABT  31
#define R14_ABT  32
#define SPSR_ABT 33
#define R13_UND  34
#define R14_UND  35
#define SPSR_UND 36
#define R8_FIQ   37
#define R9_FIQ   38
#define R10_FIQ  39
#define R11_FIQ  40
#define R12_FIQ  41
#define R13_FIQ  42
#define R14_FIQ  43
#define SPSR_FIQ 44

#include "Globals.h"
#include "eeprom.h"
#include "flash.h"
#include "RTC.h"

extern RTC *GBA_RTC;


#endif //VBA_GBA_H
