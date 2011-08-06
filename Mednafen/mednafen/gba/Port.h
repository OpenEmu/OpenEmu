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

#ifndef VBA_PORT_H
#define VBA_PORT_H

// swaps a 16-bit value
static inline uint16 swap16(uint16 v)
{
  return (v<<8)|(v>>8);
}

// swaps a 32-bit value
static inline uint32 swap32(uint32 v)
{
  return (v<<24)|((v<<8)&0xff0000)|((v>>8)&0xff00)|(v>>24);
}

#ifndef LSB_FIRST
#if defined(__GNUC__) && defined(__ppc__)

#define READ16LE(base) \
  ({ unsigned short lhbrxResult;       \
     __asm__ ("lhbrx %0, 0, %1" : "=r" (lhbrxResult) : "r" (base) : "memory"); \
      lhbrxResult; })

#define READ32LE(base) \
  ({ unsigned long lwbrxResult; \
     __asm__ ("lwbrx %0, 0, %1" : "=r" (lwbrxResult) : "r" (base) : "memory"); \
      lwbrxResult; })

#define WRITE16LE(base, value) \
  __asm__ ("sthbrx %0, 0, %1" : : "r" (value), "r" (base) : "memory")
  
#define WRITE32LE(base, value) \
  __asm__ ("stwbrx %0, 0, %1" : : "r" (value), "r" (base) : "memory")
  
#else
#define READ16LE(x) \
  swap16(*((uint16 *)(x)))
#define READ32LE(x) \
  swap32(*((uint32 *)(x)))
#define WRITE16LE(x,v) \
  *((uint16 *)x) = swap16((v))
#define WRITE32LE(x,v) \
  *((uint32 *)x) = swap32((v))
#endif
#else
#define READ16LE(x) \
  *((uint16 *)x)
#define READ32LE(x) \
  *((uint32 *)x)
#define WRITE16LE(x,v) \
  *((uint16 *)x) = (v)
#define WRITE32LE(x,v) \
  *((uint32 *)x) = (v)
#endif

#endif
