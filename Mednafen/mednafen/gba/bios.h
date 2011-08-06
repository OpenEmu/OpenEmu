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

#ifndef VBA_BIOS_H
#define VBA_BIOS_H

extern void BIOS_ArcTan();
extern void BIOS_ArcTan2();
extern void BIOS_BitUnPack();
extern void BIOS_BgAffineSet();
extern void BIOS_CpuSet();
extern void BIOS_CpuFastSet();
extern void BIOS_Diff8bitUnFilterWram();
extern void BIOS_Diff8bitUnFilterVram();
extern void BIOS_Diff16bitUnFilter();
extern void BIOS_Div();
extern void BIOS_DivARM();
extern void BIOS_HuffUnComp();
extern void BIOS_LZ77UnCompVram();
extern void BIOS_LZ77UnCompWram();
extern void BIOS_ObjAffineSet();
extern void BIOS_RegisterRamReset();
extern void BIOS_RegisterRamReset(uint32);
extern void BIOS_RLUnCompVram();
extern void BIOS_RLUnCompWram();
extern void BIOS_SoftReset();
extern void BIOS_Sqrt();
extern void BIOS_MidiKey2Freq();
extern void BIOS_SndDriverJmpTableCopy();
#endif // VBA_BIOS_H
