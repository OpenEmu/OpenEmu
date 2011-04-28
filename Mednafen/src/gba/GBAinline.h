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

#ifndef VBA_GBAinline_H
#define VBA_GBAinline_H

#include "Port.h"
#include "RTC.h"
#include "sram.h"

#ifdef INCED_FROM_ARM_CPP
#define EXCLUDE_ARM_FROM_INLINE
#else
#define EXCLUDE_ARM_FROM_INLINE INLINE
#endif

extern bool cpuSramEnabled;
extern bool cpuFlashEnabled;
extern bool cpuEEPROMEnabled;
extern bool cpuEEPROMSensorEnabled;
extern bool cpuDmaHack;
extern uint32 cpuDmaLast;

typedef struct
{
        uint16 Value;
        bool On;
        int32 Ticks;
        int32 Reload;
        int32 ClockReload;
	uint16 D;
	uint16 CNT;
} GBATimer;

extern GBATimer timers[4];

extern int cpuTotalTicks;

#define ARM_PREFETCH \
  {\
    cpuPrefetch[0] = CPUReadMemoryQuick(armNextPC);\
    cpuPrefetch[1] = CPUReadMemoryQuick(armNextPC+4);\
  }

#define THUMB_PREFETCH \
  {\
    cpuPrefetch[0] = CPUReadHalfWordQuick(armNextPC);\
    cpuPrefetch[1] = CPUReadHalfWordQuick(armNextPC+2);\
  }

#define ARM_PREFETCH_NEXT \
  cpuPrefetch[1] = CPUReadMemoryQuick(armNextPC+4);

#define THUMB_PREFETCH_NEXT\
  cpuPrefetch[1] = CPUReadHalfWordQuick(armNextPC+2);

#define CPUReadByteQuick(addr) \
  map[(addr)>>24].address[(addr) & map[(addr)>>24].mask]

#define CPUReadHalfWordQuick(addr) \
  READ16LE(((uint16*)&map[(addr)>>24].address[(addr) & map[(addr)>>24].mask]))

#define CPUReadMemoryQuick(addr) \
  READ32LE(((uint32*)&map[(addr)>>24].address[(addr) & map[(addr)>>24].mask]))

uint32 CPUReadMemory(uint32 address);

uint32 CPUReadHalfWord(uint32 address);

static INLINE uint16 CPUReadHalfWordSigned(uint32 address)
{
  uint16 value = CPUReadHalfWord(address);
  if((address & 1))
    value = (int8)value;
  return value;
}

uint8 CPUReadByte(uint32 address);

// Waitstates when accessing data
static EXCLUDE_ARM_FROM_INLINE int dataTicksAccesint16(uint32 address) // DATA 8/16bits NON SEQ
{
  int addr = (address>>24)&15;
  int value =  memoryWait[addr];

  if (addr>=0x08)
  {
    busPrefetchCount=0;
    busPrefetch=false;
  }
  else if (busPrefetch)
  {
    int waitState = value;
    if (waitState>0)
      waitState--;
    waitState++;
    busPrefetchCount = (busPrefetchCount<<waitState) | (0xFF>>(8-waitState));
  }

  return value;
}

static EXCLUDE_ARM_FROM_INLINE int dataTicksAccesint32(uint32 address) // DATA 32bits NON SEQ
{
  int addr = (address>>24)&15;
  int value = memoryWait32[addr];

  if (addr>=0x08)
  {
    busPrefetchCount=0;
    busPrefetch=false;
  }
  else if (busPrefetch)
  {
    int waitState = value;
    if (waitState>0)
      waitState--;
    waitState++;
    busPrefetchCount = (busPrefetchCount<<waitState) | (0xFF>>(8-waitState));
  }

  return value;
}

static EXCLUDE_ARM_FROM_INLINE int dataTicksAccessSeq16(uint32 address)// DATA 8/16bits SEQ
{
  int addr = (address>>24)&15;
  int value = memoryWaitSeq[addr];

  if (addr>=0x08)
  {
    busPrefetchCount=0;
    busPrefetch=false;
  }
  else if (busPrefetch)
  {
    int waitState = value;
    if (waitState>0)
      waitState--;
    waitState++;
    busPrefetchCount = (busPrefetchCount<<waitState) | (0xFF>>(8-waitState));
  }

  return value;
}

static EXCLUDE_ARM_FROM_INLINE int dataTicksAccessSeq32(uint32 address)// DATA 32bits SEQ
{
  int addr = (address>>24)&15;
  int value =  memoryWaitSeq32[addr];

  if (addr>=0x08)
  {
    busPrefetchCount=0;
    busPrefetch=false;
  }
  else if (busPrefetch)
  {
    int waitState = value;
    if (waitState>0)
      waitState--;
    waitState++;
    busPrefetchCount = (busPrefetchCount<<waitState) | (0xFF>>(8-waitState));
  }

  return value;
}

// Waitstates when executing opcode
static EXCLUDE_ARM_FROM_INLINE int codeTicksAccesint16(uint32 address) // THUMB NON SEQ
{
  int addr = (address>>24)&15;

  if ((addr>=0x08) && (addr<=0x0D))
  {
    if ((busPrefetchCount&0x3) == 3)
    {
      busPrefetchCount=((busPrefetchCount&0xFF)>>1) | (busPrefetchCount&0xFFFFFF00);
      return 0;
    }
    else
    if (busPrefetchCount&0x1)
    {
      busPrefetchCount=((busPrefetchCount&0xFF)>>1) | (busPrefetchCount&0xFFFFFF00);
      return memoryWaitSeq[addr]-1;
    }
    else
    {
      busPrefetchCount=0;
      return memoryWait[addr];
    }
  }
  else
  {
    busPrefetchCount = 0;
    return memoryWait[addr];
  }
}

static EXCLUDE_ARM_FROM_INLINE int codeTicksAccesint32(uint32 address) // ARM NON SEQ
{
  int addr = (address>>24)&15;

  if ((addr>=0x08) && (addr<=0x0D))
  {
    if (busPrefetchCount&0x1)
    {
      busPrefetchCount=((busPrefetchCount&0xFF)>>1) | (busPrefetchCount&0xFFFFFF00);
      if (busPrefetchCount&0x1)
      {
        busPrefetchCount=((busPrefetchCount&0xFF)>>1) | (busPrefetchCount&0xFFFFFF00);
        return 0;
      }
      else
      {
        busPrefetchCount = 0;
        return memoryWaitSeq[addr];
      }

    }
    else
    {
        busPrefetchCount = 0;
      return memoryWait32[addr];
    }
  }
  else
  {
    busPrefetchCount = 0;
    return memoryWait32[addr];
  }
}

static EXCLUDE_ARM_FROM_INLINE int codeTicksAccessSeq16(uint32 address) // THUMB SEQ
{
  int addr = (address>>24)&15;

  if ((addr>=0x08) && (addr<=0x0D))
  {
    if (busPrefetchCount&0x1)
    {
      busPrefetchCount=((busPrefetchCount&0xFF)>>1) | (busPrefetchCount&0xFFFFFF00);
      return 0;
    }
    else
    if (busPrefetchCount>0xFF)
    {
      busPrefetchCount=0;
      return memoryWait[addr];
    }
    else
      return memoryWaitSeq[addr];
  }
  else
  {
    busPrefetchCount = 0;
    return memoryWaitSeq[addr];
  }
}

static EXCLUDE_ARM_FROM_INLINE int codeTicksAccessSeq32(uint32 address) // ARM SEQ
{
  int addr = (address>>24)&15;

  if ((addr>=0x08) && (addr<=0x0D))
  {
    if (busPrefetchCount&0x1)
    {
      busPrefetchCount=((busPrefetchCount&0xFF)>>1) | (busPrefetchCount&0xFFFFFF00);
      if (busPrefetchCount&0x1)
      {
        busPrefetchCount=((busPrefetchCount&0xFF)>>1) | (busPrefetchCount&0xFFFFFF00);
        return 0;
      }
      else
        return memoryWaitSeq[addr];

    }
    else
    if (busPrefetchCount>0xFF)
    {
      busPrefetchCount=0;
      return memoryWait32[addr];
    }
    else
      return memoryWaitSeq32[addr];
  }
  else
  {
    return memoryWaitSeq32[addr];
  }
}

#endif //VBA_GBAinline_H
