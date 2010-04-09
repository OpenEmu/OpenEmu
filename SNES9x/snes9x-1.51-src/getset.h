/**********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2007  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),
                             zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com)
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti


  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley,
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001-2006    byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight,

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound DSP emulator code is derived from SNEeSe and OpenSPC:
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2007  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
**********************************************************************************/




#ifndef _GETSET_H_
#define _GETSET_H_

#include "ppu.h"
#include "dsp1.h"
#include "cpuexec.h"
#include "sa1.h"
#include "spc7110.h"
#include "obc1.h"
#include "seta.h"
#include "bsx.h"

extern "C"
{
	extern uint8 OpenBus;
}

INLINE uint8 S9xGetByte (uint32 Address)
{
    int block;
    uint8 *GetAddress = Memory.Map [block = ((Address&0xffffff) >> MEMMAP_SHIFT)];

    if(!CPU.InDMAorHDMA)
        CPU.Cycles += Memory.MemorySpeed [block];

    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
#ifdef CPU_SHUTDOWN
        if (Memory.BlockIsRAM [block])
            CPU.WaitAddress = CPU.PBPCAtOpcodeStart;
#endif
        return (*(GetAddress + (Address & 0xffff)));
    }

    switch ((pint) GetAddress)
    {
      case CMemory::MAP_PPU:
        if(CPU.InDMAorHDMA && (Address&0xff00)==0x2100) return OpenBus;
        return (S9xGetPPU (Address & 0xffff));
      case CMemory::MAP_CPU:
        return (S9xGetCPU (Address & 0xffff));
      case CMemory::MAP_DSP:
        return (S9xGetDSP (Address & 0xffff));
      case CMemory::MAP_SA1RAM:
      case CMemory::MAP_LOROM_SRAM:
        //Address &0x7FFF -offset into bank
        //Address&0xFF0000 -bank
        //bank>>1 | offset = s-ram address, unbound
        //unbound & SRAMMask = Sram offset
        return (*(Memory.SRAM + ((((Address&0xFF0000)>>1) |(Address&0x7FFF)) &Memory.SRAMMask)));

      case CMemory::MAP_LOROM_SRAM_B:
        return (*(Multi.sramB + ((((Address&0xFF0000)>>1) |(Address&0x7FFF)) &Multi.sramMaskB)));

      case CMemory::MAP_RONLY_SRAM:
      case CMemory::MAP_HIROM_SRAM:
        return (*(Memory.SRAM + (((Address & 0x7fff) - 0x6000 +
                                  ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)));

      case CMemory::MAP_BWRAM:
        return (*(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)));

      case CMemory::MAP_C4:
        return (S9xGetC4 (Address & 0xffff));

      case CMemory::MAP_SPC7110_ROM:
#ifdef SPC7110_DEBUG
        printf("reading spc7110 ROM (byte) at %06X\n", Address);
#endif
        return S9xGetSPC7110Byte(Address);

      case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
        printf("reading Bank 50 (byte)\n");
#endif
        return S9xGetSPC7110(0x4800);

      case CMemory::MAP_OBC_RAM:
        return GetOBC1(Address & 0xffff);

      case CMemory::MAP_SETA_DSP:
        return S9xGetSetaDSP(Address);

      case CMemory::MAP_SETA_RISC:
        return S9xGetST018(Address);

      case CMemory::MAP_BSX:
        return S9xGetBSX(Address);

      case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
        printf ("DEBUG R(B) %06x\n", Address);
#endif
      default:
      case CMemory::MAP_NONE:
        return OpenBus;
    }
}

#ifdef NO_INLINE_SET_GET
INLINE uint16 S9xGetWord (uint32 Address, enum s9xwrap_t w)
#else
INLINE uint16 S9xGetWord (uint32 Address, enum s9xwrap_t w=WRAP_NONE)
#endif
{
    uint32 mask=MEMMAP_MASK&(w==WRAP_PAGE?0xff:(w==WRAP_BANK?0xffff:0xffffff));
    if ((Address & mask) == mask)
    {
        OpenBus=S9xGetByte(Address);
        switch(w){
          case WRAP_PAGE:
            {
                PC_t a;
                a.xPBPC = Address;
                a.B.xPCl++;
                return (OpenBus | (S9xGetByte (a.xPBPC) << 8));
            }
          case WRAP_BANK:
            {
                PC_t a;
                a.xPBPC = Address;
                a.W.xPC++;
                return (OpenBus | (S9xGetByte (a.xPBPC) << 8));
            }
          case WRAP_NONE:
          default:
            return (OpenBus | (S9xGetByte (Address + 1) << 8));
        }
    }
    int block;
    uint8 *GetAddress = Memory.Map [block = ((Address&0xffffff) >> MEMMAP_SHIFT)];

    if(!CPU.InDMAorHDMA)
        CPU.Cycles += (Memory.MemorySpeed [block]<<1);


    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
#ifdef CPU_SHUTDOWN
        if (Memory.BlockIsRAM [block])
            CPU.WaitAddress = CPU.PBPCAtOpcodeStart;
#endif
        return READ_WORD(GetAddress + (Address & 0xffff));
    }

    switch ((pint) GetAddress)
    {
      case CMemory::MAP_PPU:
        if(CPU.InDMAorHDMA){
            OpenBus=S9xGetByte (Address);
            return (OpenBus | (S9xGetByte (Address + 1) << 8));
        }
        return (S9xGetPPU (Address & 0xffff) |
                (S9xGetPPU ((Address + 1) & 0xffff) << 8));
      case CMemory::MAP_CPU:
        return (S9xGetCPU (Address & 0xffff) |
                (S9xGetCPU ((Address + 1) & 0xffff) << 8));
      case CMemory::MAP_DSP:
#ifdef DSP_DUMMY_LOOPS
        printf("Get DSP Word @ %06X\n", Address);
#endif
        return (S9xGetDSP (Address & 0xffff) |
                (S9xGetDSP ((Address + 1) & 0xffff) << 8));
      case CMemory::MAP_SA1RAM:
      case CMemory::MAP_LOROM_SRAM:
        //Address &0x7FFF -offset into bank
        //Address&0xFF0000 -bank
        //bank>>1 | offset = s-ram address, unbound
        //unbound & SRAMMask = Sram offset
        if(Memory.SRAMMask>=MEMMAP_MASK){
            return READ_WORD(Memory.SRAM + ((((Address&0xFF0000)>>1) |(Address&0x7FFF)) &Memory.SRAMMask));
        } else {
            /* no READ_WORD here, since if Memory.SRAMMask=0x7ff
             * then the high byte doesn't follow the low byte. */
            return
                (*(Memory.SRAM + ((((Address&0xFF0000)>>1) |(Address&0x7FFF)) &Memory.SRAMMask)))|
                ((*(Memory.SRAM + (((((Address+1)&0xFF0000)>>1) |((Address+1)&0x7FFF)) &Memory.SRAMMask)))<<8);
        }

      case CMemory::MAP_LOROM_SRAM_B:
        if(Multi.sramMaskB>=MEMMAP_MASK){
            return READ_WORD(Multi.sramB + ((((Address&0xFF0000)>>1) |(Address&0x7FFF)) &Multi.sramMaskB));
        } else {
            return
                (*(Multi.sramB + ((((Address&0xFF0000)>>1) |(Address&0x7FFF)) &Multi.sramMaskB)))|
                ((*(Multi.sramB + (((((Address+1)&0xFF0000)>>1) |((Address+1)&0x7FFF)) &Multi.sramMaskB)))<<8);
        }

      case CMemory::MAP_RONLY_SRAM:
      case CMemory::MAP_HIROM_SRAM:
        if(Memory.SRAMMask>=MEMMAP_MASK){
            return READ_WORD(Memory.SRAM +
                             (((Address & 0x7fff) - 0x6000 +
                               ((Address & 0xf0000) >> 3)) & Memory.SRAMMask));
        } else {
            /* no READ_WORD here, since if Memory.SRAMMask=0x7ff
             * then the high byte doesn't follow the low byte. */
            return (*(Memory.SRAM +
                      (((Address & 0x7fff) - 0x6000 +
                        ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)) |
                    (*(Memory.SRAM +
                       ((((Address + 1) & 0x7fff) - 0x6000 +
                         (((Address + 1) & 0xf0000) >> 3)) & Memory.SRAMMask)) << 8));
        }

      case CMemory::MAP_BWRAM:
        return READ_WORD(Memory.BWRAM + ((Address & 0x7fff) - 0x6000));

      case CMemory::MAP_C4:
        return (S9xGetC4 (Address & 0xffff) |
                (S9xGetC4 ((Address + 1) & 0xffff) << 8));

      case CMemory::MAP_SPC7110_ROM:
#ifdef SPC7110_DEBUG
        printf("reading spc7110 ROM (word) at %06X\n", Address);
#endif
        return (S9xGetSPC7110Byte(Address)|
                (S9xGetSPC7110Byte (Address+1))<<8);
      case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
        printf("reading Bank 50 (word)\n");
#endif
        return (S9xGetSPC7110(0x4800)|
                (S9xGetSPC7110 (0x4800) << 8));
      case CMemory::MAP_OBC_RAM:
        return GetOBC1(Address&0xFFFF)| (GetOBC1((Address+1)&0xFFFF)<<8);

      case CMemory::MAP_SETA_DSP:
        return S9xGetSetaDSP(Address)| (S9xGetSetaDSP((Address+1))<<8);

      case CMemory::MAP_SETA_RISC:
        return S9xGetST018(Address)| (S9xGetST018((Address+1))<<8);

      case CMemory::MAP_BSX:
        return S9xGetBSX(Address)| (S9xGetBSX((Address+1))<<8);

      case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
        printf ("DEBUG R(W) %06x\n", Address);
#endif
      default:
      case CMemory::MAP_NONE:
        return (OpenBus | (OpenBus<<8));
    }
}

INLINE void S9xSetByte (uint8 Byte, uint32 Address)
{
#if defined(CPU_SHUTDOWN)
    CPU.WaitAddress = 0xffffffff;
#endif
    int block;
    uint8 *SetAddress = Memory.WriteMap [block = ((Address&0xffffff) >> MEMMAP_SHIFT)];

    if (!CPU.InDMAorHDMA)
        CPU.Cycles += Memory.MemorySpeed [block];


    if (SetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
#ifdef CPU_SHUTDOWN
        SetAddress += Address & 0xffff;
        if (SetAddress == SA1.WaitByteAddress1 ||
            SetAddress == SA1.WaitByteAddress2)
        {
            SA1.Executing = SA1.S9xOpcodes != NULL;
            SA1.WaitCounter = 0;
        }
        *SetAddress = Byte;
#else
        *(SetAddress + (Address & 0xffff)) = Byte;
#endif
        return;
    }

    switch ((pint) SetAddress)
    {
      case CMemory::MAP_PPU:
        if(CPU.InDMAorHDMA && (Address&0xff00)==0x2100) return;
        S9xSetPPU (Byte, Address & 0xffff);
        return;

      case CMemory::MAP_CPU:
        S9xSetCPU (Byte, Address & 0xffff);
        return;

      case CMemory::MAP_DSP:
#ifdef DSP_DUMMY_LOOPS
        printf("DSP Byte: %02X to %06X\n", Byte, Address);
#endif
        S9xSetDSP (Byte, Address & 0xffff);
        return;

      case CMemory::MAP_LOROM_SRAM:
        if (Memory.SRAMMask)
        {
            *(Memory.SRAM + ((((Address&0xFF0000)>>1)|(Address&0x7FFF))& Memory.SRAMMask))=Byte;
            CPU.SRAMModified = TRUE;
        }
        return;

      case CMemory::MAP_LOROM_SRAM_B:
        if (Multi.sramMaskB)
        {
            *(Multi.sramB + ((((Address&0xFF0000)>>1)|(Address&0x7FFF))& Multi.sramMaskB))=Byte;
            CPU.SRAMModified = TRUE;
        }
        return;

     case CMemory::MAP_HIROM_SRAM:
        if (Memory.SRAMMask)
        {
            *(Memory.SRAM + (((Address & 0x7fff) - 0x6000 +
                              ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)) = Byte;
            CPU.SRAMModified = TRUE;
        }
        return;

      case CMemory::MAP_BWRAM:
        *(Memory.BWRAM + ((Address & 0x7fff) - 0x6000)) = Byte;
        CPU.SRAMModified = TRUE;
        return;

      case CMemory::MAP_SA1RAM:
        *(Memory.SRAM + (Address & 0xffff)) = Byte;
        SA1.Executing = !SA1.Waiting;
        break;

      case CMemory::MAP_C4:
        S9xSetC4 (Byte, Address & 0xffff);
        return;

      case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
        printf("Writing Byte at %06X\n", Address);
#endif
        s7r.bank50[(Address & 0xffff)]= (uint8) Byte;
        break;

      case CMemory::MAP_OBC_RAM:
        SetOBC1(Byte, Address &0xFFFF);
        return;

      case CMemory::MAP_SETA_DSP:
        S9xSetSetaDSP(Byte,Address);
        return;

      case CMemory::MAP_SETA_RISC:
        S9xSetST018(Byte,Address);
        return;

      case CMemory::MAP_BSX:
        S9xSetBSX(Byte,Address);
        return;

      case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
        printf ("W(B) %06x\n", Address);
#endif
      default:
      case CMemory::MAP_NONE:
        return;
    }
}

#ifdef NO_INLINE_SET_GET
INLINE void S9xSetWord (uint16 Word, uint32 Address, enum s9xwrap_t w, enum s9xwriteorder_t o)
#else
INLINE void S9xSetWord (uint16 Word, uint32 Address, enum s9xwrap_t w=WRAP_NONE, enum s9xwriteorder_t o=WRITE_01)
#endif
{
    uint32 mask=MEMMAP_MASK&(w==WRAP_PAGE?0xff:(w==WRAP_BANK?0xffff:0xffffff));
    if ((Address & mask) == mask)
    {
        if(!o) S9xSetByte(Word&0x00FF, Address);
        switch(w){
          case WRAP_PAGE:
            {
                PC_t a;
                a.xPBPC = Address;
                a.B.xPCl++;
                S9xSetByte(Word>>8, a.xPBPC);
            }
          case WRAP_BANK:
            {
                PC_t a;
                a.xPBPC = Address;
                a.W.xPC++;
                S9xSetByte(Word>>8, a.xPBPC);
            }
          case WRAP_NONE:
          default:
            S9xSetByte(Word>>8, Address+1);
        }
        if(o) S9xSetByte(Word&0x00FF, Address);
        return;
    }

#if defined(CPU_SHUTDOWN)
    CPU.WaitAddress = 0xffffffff;
#endif
    int block;
    uint8 *SetAddress = Memory.WriteMap [block = ((Address&0xffffff) >> MEMMAP_SHIFT)];

    if (!CPU.InDMAorHDMA)
        CPU.Cycles += Memory.MemorySpeed [block] << 1;


    if (SetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
#ifdef CPU_SHUTDOWN
        SetAddress += Address & 0xffff;
        if (SetAddress == SA1.WaitByteAddress1 ||
            SetAddress == SA1.WaitByteAddress2)
        {
            SA1.Executing = SA1.S9xOpcodes != NULL;
            SA1.WaitCounter = 0;
        }
        WRITE_WORD(SetAddress, Word);
#else
        WRITE_WORD(SetAddress + (Address & 0xffff), Word);
#endif
        return;
    }

    switch ((pint) SetAddress)
    {
      case CMemory::MAP_PPU:
        if(CPU.InDMAorHDMA){
            if((Address&0xff00)!=0x2100) S9xSetPPU((uint8)Word, Address&0xffff);
            if(((Address+1)&0xff00)!=0x2100) S9xSetPPU(Word>>8, (Address+1)&0xffff);
            return;
        }
        if(o){
            S9xSetPPU (Word >> 8, (Address & 0xffff) + 1);
            S9xSetPPU ((uint8) Word, Address & 0xffff);
        } else {
            S9xSetPPU ((uint8) Word, Address & 0xffff);
            S9xSetPPU (Word >> 8, (Address & 0xffff) + 1);
        }
        return;

      case CMemory::MAP_CPU:
        if(o){
            S9xSetCPU (Word >> 8, (Address & 0xffff) + 1);
            S9xSetCPU ((uint8) Word, (Address & 0xffff));
        } else {
            S9xSetCPU ((uint8) Word, (Address & 0xffff));
            S9xSetCPU (Word >> 8, (Address & 0xffff) + 1);
        }
        return;

      case CMemory::MAP_DSP:
#ifdef DSP_DUMMY_LOOPS
        printf("DSP Word: %04X to %06X\n", Word, Address);
#endif
        if(o){
            S9xSetDSP (Word >> 8, (Address & 0xffff) + 1);
            S9xSetDSP ((uint8) Word, (Address & 0xffff));
        } else {
            S9xSetDSP ((uint8) Word, (Address & 0xffff));
            S9xSetDSP (Word >> 8, (Address & 0xffff) + 1);
        }
        return;

      case CMemory::MAP_LOROM_SRAM:
        if (Memory.SRAMMask) {
            if(Memory.SRAMMask>=MEMMAP_MASK){
                WRITE_WORD(Memory.SRAM + ((((Address&0xFF0000)>>1)|(Address&0x7FFF))&Memory.SRAMMask), Word);
            } else {
                /* no WRITE_WORD here, since if Memory.SRAMMask=0x7ff
                 * then the high byte doesn't follow the low byte. */
                *(Memory.SRAM + ((((Address&0xFF0000)>>1)|(Address&0x7FFF))& Memory.SRAMMask)) = (uint8) Word;
                *(Memory.SRAM + (((((Address+1)&0xFF0000)>>1)|((Address+1)&0x7FFF))& Memory.SRAMMask)) = Word >> 8;
            }

            CPU.SRAMModified = TRUE;
        }
        return;

      case CMemory::MAP_LOROM_SRAM_B:
        if (Multi.sramMaskB) {
            if(Multi.sramMaskB>=MEMMAP_MASK){
                WRITE_WORD(Multi.sramB + ((((Address&0xFF0000)>>1)|(Address&0x7FFF))&Multi.sramMaskB), Word);
            } else {
                *(Multi.sramB + ((((Address&0xFF0000)>>1)|(Address&0x7FFF))& Multi.sramMaskB)) = (uint8) Word;
                *(Multi.sramB + (((((Address+1)&0xFF0000)>>1)|((Address+1)&0x7FFF))& Multi.sramMaskB)) = Word >> 8;
            }

            CPU.SRAMModified = TRUE;
        }
        return;

      case CMemory::MAP_HIROM_SRAM:
        if (Memory.SRAMMask) {
            if(Memory.SRAMMask>=MEMMAP_MASK){
                WRITE_WORD(Memory.SRAM +
                           (((Address & 0x7fff) - 0x6000 +
                             ((Address & 0xf0000) >> 3) & Memory.SRAMMask)), Word);
            } else {
                /* no WRITE_WORD here, since if Memory.SRAMMask=0x7ff
                 * then the high byte doesn't follow the low byte. */
                *(Memory.SRAM +
                  (((Address & 0x7fff) - 0x6000 +
                    ((Address & 0xf0000) >> 3) & Memory.SRAMMask))) = (uint8) Word;
                *(Memory.SRAM +
                  ((((Address + 1) & 0x7fff) - 0x6000 +
                    (((Address + 1) & 0xf0000) >> 3) & Memory.SRAMMask))) = (uint8) (Word >> 8);
            }
            CPU.SRAMModified = TRUE;
        }
        return;

      case CMemory::MAP_BWRAM:
        WRITE_WORD(Memory.BWRAM + ((Address & 0x7fff) - 0x6000), Word);
        CPU.SRAMModified = TRUE;
        return;

      case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
        printf("Writing Word at %06X\n", Address);
#endif
        WRITE_WORD(s7r.bank50+(Address & 0xffff), Word);
        break;

      case CMemory::MAP_SA1RAM:
        WRITE_WORD(Memory.SRAM + (Address & 0xffff), Word);
        SA1.Executing = !SA1.Waiting;
        break;

      case CMemory::MAP_C4:
        if(o){
            S9xSetC4 ((uint8) (Word >> 8), (Address + 1) & 0xffff);
            S9xSetC4 (Word & 0xff, Address & 0xffff);
        } else {
            S9xSetC4 (Word & 0xff, Address & 0xffff);
            S9xSetC4 ((uint8) (Word >> 8), (Address + 1) & 0xffff);
        }
        return;

      case CMemory::MAP_OBC_RAM:
        if(o){
            SetOBC1((uint8) (Word >> 8), (Address + 1) & 0xffff);
            SetOBC1(Word & 0xff, Address &0xFFFF);
        } else {
            SetOBC1(Word & 0xff, Address &0xFFFF);
            SetOBC1 ((uint8) (Word >> 8), (Address + 1) & 0xffff);
        }
        return;

      case CMemory::MAP_SETA_DSP:
        if(o){
            S9xSetSetaDSP ((uint8) (Word >> 8),(Address + 1));
            S9xSetSetaDSP (Word & 0xff, Address);
        } else {
            S9xSetSetaDSP (Word & 0xff, Address);
            S9xSetSetaDSP ((uint8) (Word >> 8),(Address + 1));
        }
        return;

      case CMemory::MAP_SETA_RISC:
        if(o){
            S9xSetST018 ((uint8) (Word >> 8),(Address + 1));
            S9xSetST018 (Word & 0xff, Address);
        } else {
            S9xSetST018 (Word & 0xff, Address);
            S9xSetST018 ((uint8) (Word >> 8),(Address + 1));
        }
        return;

      case CMemory::MAP_BSX:
        if(o){
            S9xSetBSX ((uint8) (Word >> 8),(Address + 1));
            S9xSetBSX (Word & 0xff, Address);
        } else {
            S9xSetBSX (Word & 0xff, Address);
            S9xSetBSX ((uint8) (Word >> 8),(Address + 1));
        }
        return;

      case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
        printf ("W(W) %06x\n", Address);
#endif
      default:
      case CMemory::MAP_NONE:
        return;
    }
}

INLINE uint8 *GetBasePointer (uint32 Address)
{
    uint8 *GetAddress = Memory.Map [((Address&0xffffff) >> MEMMAP_SHIFT)];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
        return (GetAddress);

    switch ((pint) GetAddress)
    {
//      case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
//        printf("Getting Base pointer to DRAM\n");
#endif
//        {
//            return s7r.bank50;
//        }

      case CMemory::MAP_SPC7110_ROM:
#ifdef SPC7110_DEBUG
        printf("Getting Base pointer to SPC7110ROM\n");
#endif
        return Get7110BasePtr(Address);

      case CMemory::MAP_SA1RAM:
        return (Memory.SRAM);

      case CMemory::MAP_LOROM_SRAM:
        if((Memory.SRAMMask&MEMMAP_MASK)!=MEMMAP_MASK) return NULL;
        return (Memory.SRAM + ((((Address&0xFF0000)>>1)|(Address&0x7FFF)) & Memory.SRAMMask) - (Address&0xffff));

      case CMemory::MAP_LOROM_SRAM_B:
        if((Multi.sramMaskB&MEMMAP_MASK)!=MEMMAP_MASK) return NULL;
        return (Multi.sramB + ((((Address&0xFF0000)>>1)|(Address&0x7FFF)) & Multi.sramMaskB) - (Address&0xffff));

      case CMemory::MAP_BWRAM:
        return (Memory.BWRAM - 0x6000 - (Address&0x8000));

      case CMemory::MAP_HIROM_SRAM:
        if((Memory.SRAMMask&MEMMAP_MASK)!=MEMMAP_MASK) return NULL;
        return (Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask) - (Address&0xffff));

      case CMemory::MAP_C4:
        return S9xGetBasePointerC4(Address);

      case CMemory::MAP_OBC_RAM:
        return GetBasePointerOBC1(Address);

      case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
        printf ("GBP %06x\n", Address);
#endif
      default:
      case CMemory::MAP_NONE:
        return (0);
    }
}

INLINE uint8 *S9xGetMemPointer (uint32 Address)
{
    uint8 *GetAddress = Memory.Map [((Address&0xffffff) >> MEMMAP_SHIFT)];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
        return (GetAddress + (Address&0xffff));

    switch ((pint) GetAddress)
    {
//      case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
//        printf("Getting Mem pointer to DRAM\n");
#endif
//        {
//            return s7r.bank50 + (Address&0xffff);
//        }

      case CMemory::MAP_SPC7110_ROM:
#ifdef SPC7110_DEBUG
        printf("Getting Mem pointer to SPC7110ROM\n");
#endif
        return Get7110BasePtr(Address) + (Address&0xffff);

      case CMemory::MAP_SA1RAM:
        return (Memory.SRAM + (Address&0xffff));

      case CMemory::MAP_LOROM_SRAM:
        if((Memory.SRAMMask&MEMMAP_MASK)!=MEMMAP_MASK) return NULL;
        return (Memory.SRAM + ((((Address&0xFF0000)>>1)|(Address&0x7FFF)) & Memory.SRAMMask));

      case CMemory::MAP_LOROM_SRAM_B:
        if((Multi.sramMaskB&MEMMAP_MASK)!=MEMMAP_MASK) return NULL;
        return (Multi.sramB + ((((Address&0xFF0000)>>1)|(Address&0x7FFF)) & Multi.sramMaskB));

      case CMemory::MAP_BWRAM:
        return (Memory.BWRAM - 0x6000 + (Address&0x7fff));

      case CMemory::MAP_HIROM_SRAM:
        if((Memory.SRAMMask&MEMMAP_MASK)!=MEMMAP_MASK) return NULL;
        return (Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask));

      case CMemory::MAP_C4:
        return S9xGetBasePointerC4(Address) + (Address&0xffff);

      case CMemory::MAP_OBC_RAM:
        return GetMemPointerOBC1(Address);

      case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
        printf ("GMP %06x\n", Address);
#endif
      default:
      case CMemory::MAP_NONE:
        return (0);
    }
}

INLINE void S9xSetPCBase (uint32 Address)
{
    Registers.PBPC = Address & 0xffffff;
    ICPU.ShiftedPB = Address & 0xff0000;

    int block;
    uint8 *GetAddress = Memory.Map [block = ((Address&0xffffff) >> MEMMAP_SHIFT)];

    CPU.MemSpeed = Memory.MemorySpeed [block];
    CPU.MemSpeedx2 = CPU.MemSpeed << 1;

    if (GetAddress >= (uint8 *) CMemory::MAP_LAST){
        CPU.PCBase = GetAddress;
        return;
    }

    switch ((pint) GetAddress)
    {
//      case CMemory::MAP_SPC7110_DRAM:
#ifdef SPC7110_DEBUG
//        printf("Getting Base pointer to DRAM\n");
#endif
//        {
//            CPU.PCBase = s7r.bank50;
//            return;
//        }

      case CMemory::MAP_SPC7110_ROM:
#ifdef SPC7110_DEBUG
        printf("Getting Base pointer to SPC7110ROM\n");
#endif
        CPU.PCBase = Get7110BasePtr(Address);
        return;

      case CMemory::MAP_SA1RAM:
        CPU.PCBase = Memory.SRAM;
        return;

      case CMemory::MAP_LOROM_SRAM:
        if((Memory.SRAMMask&MEMMAP_MASK)!=MEMMAP_MASK){
            CPU.PCBase = NULL;
        } else {
            CPU.PCBase = (Memory.SRAM + ((((Address&0xFF0000)>>1)|(Address&0x7FFF)) & Memory.SRAMMask)) - (Address&0xffff);
        }
        return;

      case CMemory::MAP_LOROM_SRAM_B:
        if((Multi.sramMaskB&MEMMAP_MASK)!=MEMMAP_MASK){
            CPU.PCBase = NULL;
        } else {
            CPU.PCBase = (Multi.sramB + ((((Address&0xFF0000)>>1)|(Address&0x7FFF)) & Multi.sramMaskB)) - (Address&0xffff);
        }
        return;

      case CMemory::MAP_BWRAM:
        CPU.PCBase = (Memory.BWRAM - 0x6000 - (Address&0x8000));
        return;

      case CMemory::MAP_HIROM_SRAM:
        if((Memory.SRAMMask&MEMMAP_MASK)!=MEMMAP_MASK){
            CPU.PCBase = NULL;
        } else {
            CPU.PCBase = (Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask)) - (Address&0xffff);
        }
        return;

      case CMemory::MAP_C4:
        CPU.PCBase = S9xGetBasePointerC4(Address);
        return;

      case CMemory::MAP_OBC_RAM:
        CPU.PCBase = GetBasePointerOBC1(Address);
        return;

      case CMemory::MAP_BSX:
        CPU.PCBase = S9xGetBasePointerBSX(Address);
        return;

      case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
        printf ("SBP %06x\n", Address);
#endif
      default:
      case CMemory::MAP_NONE:
        CPU.PCBase = NULL;
        return;
    }
}
#endif

