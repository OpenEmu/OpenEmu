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




#include "snes9x.h"
#include "ppu.h"
#include "cpuexec.h"

#include "sa1.h"

uint8 SA1OpenBus;

static void S9xSA1CharConv2 ();
static void S9xSA1DMA ();
static void S9xSA1ReadVariableLengthData (bool8 inc, bool8 no_shift);

void S9xSA1Init ()
{
    SA1.NMIActive = FALSE;
    SA1.IRQActive = FALSE;
    SA1.WaitingForInterrupt = FALSE;
    SA1.Waiting = FALSE;
    SA1.Flags = 0;
    SA1.Executing = FALSE;
    memset (&Memory.FillRAM [0x2200], 0, 0x200);
    Memory.FillRAM [0x2200] = 0x20;
    Memory.FillRAM [0x2220] = 0x00;
    Memory.FillRAM [0x2221] = 0x01;
    Memory.FillRAM [0x2222] = 0x02;
    Memory.FillRAM [0x2223] = 0x03;
    Memory.FillRAM [0x2228] = 0xff;
    SA1.op1 = 0;
    SA1.op2 = 0;
    SA1.arithmetic_op = 0;
    SA1.sum = 0;
    SA1.overflow = FALSE;
	SA1.S9xOpcodes=NULL;
}

void S9xSA1Reset ()
{
    SA1Registers.PBPC = 0;
    SA1Registers.PB = 0;
    SA1Registers.PCw = Memory.FillRAM [0x2203] |
		      (Memory.FillRAM [0x2204] << 8);
    SA1Registers.D.W = 0;
    SA1Registers.DB = 0;
    SA1Registers.SH = 1;
    SA1Registers.SL = 0xFF;
    SA1Registers.XH = 0;
    SA1Registers.YH = 0;
    SA1Registers.P.W = 0;

    SA1.ShiftedPB = 0;
    SA1.ShiftedDB = 0;
    SA1SetFlags (MemoryFlag | IndexFlag | IRQ | Emulation);
    SA1ClearFlags (Decimal);

    SA1.WaitingForInterrupt = FALSE;
    SA1.PCBase = NULL;
    S9xSA1SetPCBase (SA1Registers.PBPC);
    SA1.S9xOpcodes = S9xSA1OpcodesM1X1;
    SA1.S9xOpLengths = S9xOpLengthsM1X1;

    S9xSA1UnpackStatus();
    S9xSA1FixCycles ();
    SA1.Executing = TRUE;
    SA1.BWRAM = Memory.SRAM;
    Memory.FillRAM [0x2225] = 0;
}

void S9xSA1SetBWRAMMemMap (uint8 val)
{
    int c;

    if (val & 0x80)
    {
	for (c = 0; c < 0x400; c += 16)
	{
	    SA1.Map [c + 6] = SA1.Map [c + 0x806] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
	    SA1.Map [c + 7] = SA1.Map [c + 0x807] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
	    SA1.WriteMap [c + 6] = SA1.WriteMap [c + 0x806] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
	    SA1.WriteMap [c + 7] = SA1.WriteMap [c + 0x807] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
	}
	SA1.BWRAM = Memory.SRAM + (val & 0x7f) * 0x2000 / 4;
    }
    else
    {
	for (c = 0; c < 0x400; c += 16)
	{
	    SA1.Map [c + 6] = SA1.Map [c + 0x806] = (uint8 *) CMemory::MAP_BWRAM;
	    SA1.Map [c + 7] = SA1.Map [c + 0x807] = (uint8 *) CMemory::MAP_BWRAM;
	    SA1.WriteMap [c + 6] = SA1.WriteMap [c + 0x806] = (uint8 *) CMemory::MAP_BWRAM;
	    SA1.WriteMap [c + 7] = SA1.WriteMap [c + 0x807] = (uint8 *) CMemory::MAP_BWRAM;
	}
	SA1.BWRAM = Memory.SRAM + (val & 7) * 0x2000;
    }
}

void S9xFixSA1AfterSnapshotLoad ()
{
    SA1.ShiftedPB = (uint32) SA1Registers.PB << 16;
    SA1.ShiftedDB = (uint32) SA1Registers.DB << 16;

    S9xSA1SetPCBase (SA1Registers.PBPC);
    S9xSA1UnpackStatus ();
    S9xSA1FixCycles ();
    SA1.VirtualBitmapFormat = (Memory.FillRAM [0x223f] & 0x80) ? 2 : 4;
    Memory.BWRAM = Memory.SRAM + (Memory.FillRAM [0x2224] & 7) * 0x2000;
    S9xSA1SetBWRAMMemMap (Memory.FillRAM [0x2225]);

    SA1.Waiting = (Memory.FillRAM [0x2200] & 0x60) != 0;
    SA1.Executing = !SA1.Waiting;
}

uint8 S9xSA1GetByte (uint32 address)
{
    uint8 *GetAddress = SA1.Map [(address&0xffffff) >> MEMMAP_SHIFT];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
	return (*(GetAddress + (address & 0xffff)));

    switch ((pint) GetAddress)
    {
    case CMemory::MAP_PPU:
	return (S9xGetSA1 (address & 0xffff));
    case CMemory::MAP_LOROM_SRAM:
    case CMemory::MAP_SA1RAM:
	return (*(Memory.SRAM + (address & 0xffff)));
    case CMemory::MAP_BWRAM:
	return (*(SA1.BWRAM + ((address & 0x7fff) - 0x6000)));
    case CMemory::MAP_BWRAM_BITMAP:
	address -= 0x600000;
	if (SA1.VirtualBitmapFormat == 2)
	    return ((Memory.SRAM [(address >> 2) & 0xffff] >> ((address & 3) << 1)) & 3);
	else
	    return ((Memory.SRAM [(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);
    case CMemory::MAP_BWRAM_BITMAP2:
	address = (address & 0xffff) - 0x6000;
	if (SA1.VirtualBitmapFormat == 2)
	    return ((SA1.BWRAM [(address >> 2) & 0xffff] >> ((address & 3) << 1)) & 3);
	else
	    return ((SA1.BWRAM [(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);

    case CMemory::MAP_DEBUG:
    default:
#ifdef DEBUGGER
//	printf ("R(B) %06x\n", address);
#endif
        return SA1OpenBus;
    }
}

uint16 S9xSA1GetWord (uint32 address, s9xwrap_t w)
{
    SA1OpenBus = S9xSA1GetByte (address);
    switch(w){
      case WRAP_PAGE:
        {
            PC_t a;
            a.xPBPC = address;
            a.B.xPCl++;
            return (SA1OpenBus | (S9xSA1GetByte (a.xPBPC) << 8));
        }
      case WRAP_BANK:
        {
            PC_t a;
            a.xPBPC = address;
            a.W.xPC++;
            return (SA1OpenBus | (S9xSA1GetByte (a.xPBPC) << 8));
        }
      case WRAP_NONE:
      default:
        return (SA1OpenBus | (S9xSA1GetByte (address + 1) << 8));
    }
}

void S9xSA1SetByte (uint8 byte, uint32 address)
{
    uint8 *Setaddress = SA1.WriteMap [(address&0xffffff) >> MEMMAP_SHIFT];

    if (Setaddress >= (uint8 *) CMemory::MAP_LAST)
    {
	*(Setaddress + (address & 0xffff)) = byte;
	return;
    }

    switch ((pint) Setaddress)
    {
    case CMemory::MAP_PPU:
	S9xSetSA1 (byte, address & 0xffff);
	return;
    case CMemory::MAP_SA1RAM:
    case CMemory::MAP_LOROM_SRAM:
	*(Memory.SRAM + (address & 0xffff)) = byte;
	return;
    case CMemory::MAP_BWRAM:
	*(SA1.BWRAM + ((address & 0x7fff) - 0x6000)) = byte;
	return;
    case CMemory::MAP_BWRAM_BITMAP:
	address -= 0x600000;
	if (SA1.VirtualBitmapFormat == 2)
	{
	    uint8 *ptr = &Memory.SRAM [(address >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address & 3) << 1));
	    *ptr |= (byte & 3) << ((address & 3) << 1);
	}
	else
	{
	    uint8 *ptr = &Memory.SRAM [(address >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address & 1) << 2));
	    *ptr |= (byte & 15) << ((address & 1) << 2);
	}
	break;
    case CMemory::MAP_BWRAM_BITMAP2:
	address = (address & 0xffff) - 0x6000;
	if (SA1.VirtualBitmapFormat == 2)
	{
	    uint8 *ptr = &SA1.BWRAM [(address >> 2) & 0xffff];
	    *ptr &= ~(3 << ((address & 3) << 1));
	    *ptr |= (byte & 3) << ((address & 3) << 1);
	}
	else
	{
	    uint8 *ptr = &SA1.BWRAM [(address >> 1) & 0xffff];
	    *ptr &= ~(15 << ((address & 1) << 2));
	    *ptr |= (byte & 15) << ((address & 1) << 2);
	}
    default:
	return;
    }
}

void S9xSA1SetWord (uint16 Word, uint32 address, enum s9xwrap_t w, enum s9xwriteorder_t o)
{
    if(!o) S9xSA1SetByte ((uint8) Word, address);
    switch(w){
      case WRAP_PAGE:
        {
            PC_t a;
            a.xPBPC = address;
            a.B.xPCl++;
            S9xSA1SetByte (Word>>8, a.xPBPC);
        }
      case WRAP_BANK:
        {
            PC_t a;
            a.xPBPC = address;
            a.W.xPC++;
            S9xSA1SetByte (Word>>8, a.xPBPC);
        }
      case WRAP_NONE:
      default:
        S9xSA1SetByte (Word>>8, address+1);
    }
    if(o) S9xSA1SetByte ((uint8) Word, address);
}

void S9xSA1SetPCBase (uint32 address)
{
    SA1Registers.PBPC = address&0xffffff;
    SA1.ShiftedPB = address&0xff0000;

    uint8 *GetAddress = SA1.Map [(address&0xffffff) >> MEMMAP_SHIFT];
    if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
    {
	SA1.PCBase = GetAddress;
	return;
    }

    switch ((pint) GetAddress)
    {
    case CMemory::MAP_SA1RAM:
	SA1.PCBase = Memory.SRAM;
	return;

    case CMemory::MAP_LOROM_SRAM:
        if((Memory.SRAMMask&MEMMAP_MASK)!=MEMMAP_MASK){
            SA1.PCBase = NULL;
        } else {
            SA1.PCBase = (Memory.SRAM + ((((address&0xFF0000)>>1)|(address&0x7FFF)) & Memory.SRAMMask)) - (address&0xffff);
        }
        return;

    case CMemory::MAP_BWRAM:
	SA1.PCBase = SA1.BWRAM - 0x6000 - (address&0x8000);
	return;

    case CMemory::MAP_HIROM_SRAM:
        if((Memory.SRAMMask&MEMMAP_MASK)!=MEMMAP_MASK){
            SA1.PCBase = NULL;
        } else {
            SA1.PCBase = (Memory.SRAM + (((address&0x7fff) - 0x6000 + ((address & 0xf0000) >> 3)) & Memory.SRAMMask)) - (address&0xffff);
        }
	return;

    case CMemory::MAP_DEBUG:
#ifdef DEBUGGER
	printf ("SBP %06x\n", address);
#endif

    default:
    case CMemory::MAP_NONE:
	SA1.PCBase = NULL;
	return;
    }
}

void S9xSA1ExecuteDuringSleep ()
{
#if 0
    if (SA1.Executing)
    {
	while (CPU.Cycles < CPU.NextEvent)
	{
	    S9xSA1MainLoop ();
	    CPU.Cycles += TWO_CYCLES * 2;
	}
    }
#endif
}

void S9xSetSA1MemMap (uint32 which1, uint8 map)
{
    int c;
    int start = which1 * 0x100 + 0xc00;
    int start2 = which1 * 0x200;

    if (which1 >= 2)
	start2 += 0x400;

    for (c = 0; c < 0x100; c += 16)
    {
	uint8 *block = &Memory.ROM [(map & 7) * 0x100000 + (c << 12)];
	int i;

	for (i = c; i < c + 16; i++)
	    Memory.Map [start + i] = SA1.Map [start + i] = block;
    }

    for (c = 0; c < 0x200; c += 16)
    {
	uint8 *block = &Memory.ROM [(map & 7) * 0x100000 + (c << 11) - 0x8000];
	int i;

	for (i = c + 8; i < c + 16; i++)
	    Memory.Map [start2 + i] = SA1.Map [start2 + i] = block;
    }
}

uint8 S9xGetSA1 (uint32 address)
{
//	printf ("R: %04x\n", address);
    switch (address)
    {
    case 0x2300:
	return ((uint8) ((Memory.FillRAM [0x2209] & 0x5f) |
		 (CPU.IRQActive & (SA1_IRQ_SOURCE | SA1_DMA_IRQ_SOURCE))));
    case 0x2301:
	return ((Memory.FillRAM [0x2200] & 0xf) |
		(Memory.FillRAM [0x2301] & 0xf0));
    case 0x2306:
	return ((uint8)  SA1.sum);
    case 0x2307:
	return ((uint8) (SA1.sum >>  8));
    case 0x2308:
	return ((uint8) (SA1.sum >> 16));
    case 0x2309:
	return ((uint8) (SA1.sum >> 24));
    case 0x230a:
	return ((uint8) (SA1.sum >> 32));
    case 0x230c:
	return (Memory.FillRAM [0x230c]);
    case 0x230d:
    {
	uint8 byte = Memory.FillRAM [0x230d];

	if (Memory.FillRAM [0x2258] & 0x80)
	{
	    S9xSA1ReadVariableLengthData (TRUE, FALSE);
	}
	return (byte);
    }
    default:
	printf ("R: %04x\n", address);
	break;
    }
    return (Memory.FillRAM [address]);
}

void S9xSetSA1 (uint8 byte, uint32 address)
{
//printf ("W: %02x -> %04x\n", byte, address);
    switch (address)
    {
    case 0x2200:
	SA1.Waiting = (byte & 0x60) != 0;
//	SA1.Executing = !SA1.Waiting && SA1.S9xOpcodes;

	if (!(byte & 0x20) && (Memory.FillRAM [0x2200] & 0x20))
	{
	    S9xSA1Reset ();
	}
	if (byte & 0x80)
	{
	    Memory.FillRAM [0x2301] |= 0x80;
	    if (Memory.FillRAM [0x220a] & 0x80)
	    {
		SA1.Flags |= IRQ_FLAG;
		SA1.IRQActive |= SNES_IRQ_SOURCE;
		SA1.Executing = !SA1.Waiting && SA1.S9xOpcodes;
	    }
	}
	if (byte & 0x10)
	{
	    Memory.FillRAM [0x2301] |= 0x10;
#ifdef DEBUGGER
		printf ("###SA1 NMI\n");
#endif
	    if (Memory.FillRAM [0x220a] & 0x10)
	    {
	    }
	}
	break;

    case 0x2201:
	if (((byte ^ Memory.FillRAM [0x2201]) & 0x80) &&
	    (Memory.FillRAM [0x2300] & byte & 0x80))
	{
	    S9xSetIRQ (SA1_IRQ_SOURCE);
	}
	if (((byte ^ Memory.FillRAM [0x2201]) & 0x20) &&
	    (Memory.FillRAM [0x2300] & byte & 0x20))
	{
	    S9xSetIRQ (SA1_DMA_IRQ_SOURCE);
	}
	break;
    case 0x2202:
	if (byte & 0x80)
	{
	    Memory.FillRAM [0x2300] &= ~0x80;
	    S9xClearIRQ (SA1_IRQ_SOURCE);
	}
	if (byte & 0x20)
	{
	    Memory.FillRAM [0x2300] &= ~0x20;
	    S9xClearIRQ (SA1_DMA_IRQ_SOURCE);
	}
	break;
    case 0x2203:
//	printf ("SA1 reset vector: %04x\n", byte | (Memory.FillRAM [0x2204] << 8));
	break;
    case 0x2204:
//	printf ("SA1 reset vector: %04x\n", (byte << 8) | Memory.FillRAM [0x2203]);
	break;

    case 0x2205:
//	printf ("SA1 NMI vector: %04x\n", byte | (Memory.FillRAM [0x2206] << 8));
	break;
    case 0x2206:
//	printf ("SA1 NMI vector: %04x\n", (byte << 8) | Memory.FillRAM [0x2205]);
	break;

    case 0x2207:
//	printf ("SA1 IRQ vector: %04x\n", byte | (Memory.FillRAM [0x2208] << 8));
	break;
    case 0x2208:
//	printf ("SA1 IRQ vector: %04x\n", (byte << 8) | Memory.FillRAM [0x2207]);
	break;

    case 0x2209:
	Memory.FillRAM [0x2209] = byte;
	if (byte & 0x80)
	    Memory.FillRAM [0x2300] |= 0x80;

	if (byte & Memory.FillRAM [0x2201] & 0x80)
	{
	    S9xSetIRQ (SA1_IRQ_SOURCE);
	}
	break;
    case 0x220a:
	if (((byte ^ Memory.FillRAM [0x220a]) & 0x80) &&
	    (Memory.FillRAM [0x2301] & byte & 0x80))
	{
	    SA1.Flags |= IRQ_FLAG;
	    SA1.IRQActive |= SNES_IRQ_SOURCE;
//	    SA1.Executing = !SA1.Waiting;
	}
	if (((byte ^ Memory.FillRAM [0x220a]) & 0x40) &&
	    (Memory.FillRAM [0x2301] & byte & 0x40))
	{
	    SA1.Flags |= IRQ_FLAG;
	    SA1.IRQActive |= TIMER_IRQ_SOURCE;
//	    SA1.Executing = !SA1.Waiting;
	}
	if (((byte ^ Memory.FillRAM [0x220a]) & 0x20) &&
	    (Memory.FillRAM [0x2301] & byte & 0x20))
	{
	    SA1.Flags |= IRQ_FLAG;
	    SA1.IRQActive |= DMA_IRQ_SOURCE;
//	    SA1.Executing = !SA1.Waiting;
	}
	if (((byte ^ Memory.FillRAM [0x220a]) & 0x10) &&
	    (Memory.FillRAM [0x2301] & byte & 0x10))
	{
#ifdef DEBUGGER
	    printf ("###SA1 NMI\n");
#endif
	}
	break;
    case 0x220b:
	if (byte & 0x80)
	{
	    SA1.IRQActive &= ~SNES_IRQ_SOURCE;
	    Memory.FillRAM [0x2301] &= ~0x80;
	}
	if (byte & 0x40)
	{
	    SA1.IRQActive &= ~TIMER_IRQ_SOURCE;
	    Memory.FillRAM [0x2301] &= ~0x40;
	}
	if (byte & 0x20)
	{
	    SA1.IRQActive &= ~DMA_IRQ_SOURCE;
	    Memory.FillRAM [0x2301] &= ~0x20;
	}
	if (byte & 0x10)
	{
	    // Clear NMI
	    Memory.FillRAM [0x2301] &= ~0x10;
	}
	if (!SA1.IRQActive)
	    SA1.Flags &= ~IRQ_FLAG;
	break;
    case 0x220c:
//	printf ("SNES NMI vector: %04x\n", byte | (Memory.FillRAM [0x220d] << 8));
	break;
    case 0x220d:
//	printf ("SNES NMI vector: %04x\n", (byte << 8) | Memory.FillRAM [0x220c]);
	break;

    case 0x220e:
//	printf ("SNES IRQ vector: %04x\n", byte | (Memory.FillRAM [0x220f] << 8));
	break;
    case 0x220f:
//	printf ("SNES IRQ vector: %04x\n", (byte << 8) | Memory.FillRAM [0x220e]);
	break;

    case 0x2210:
#if 0
	printf ("Timer %s\n", (byte & 0x80) ? "linear" : "HV");
	printf ("Timer H-IRQ %s\n", (byte & 1) ? "enabled" : "disabled");
	printf ("Timer V-IRQ %s\n", (byte & 2) ? "enabled" : "disabled");
#endif
	break;
    case 0x2211:
//	printf ("Timer reset\n");
	break;
    case 0x2212:
//	printf ("H-Timer %04x\n", byte | (Memory.FillRAM [0x2213] << 8));
	break;
    case 0x2213:
//	printf ("H-Timer %04x\n", (byte << 8) | Memory.FillRAM [0x2212]);
	break;
    case 0x2214:
//	printf ("V-Timer %04x\n", byte | (Memory.FillRAM [0x2215] << 8));
	break;
    case 0x2215:
//	printf ("V-Timer %04x\n", (byte << 8) | Memory.FillRAM [0x2214]);
	break;
    case 0x2220:
    case 0x2221:
    case 0x2222:
    case 0x2223:
	S9xSetSA1MemMap (address - 0x2220, byte);
//	printf ("MMC: %02x\n", byte);
	break;
    case 0x2224:
//	printf ("BWRAM image SNES %02x -> 0x6000\n", byte);
	Memory.BWRAM = Memory.SRAM + (byte & 7) * 0x2000;
	break;
    case 0x2225:
//	printf ("BWRAM image SA1 %02x -> 0x6000 (%02x)\n", byte, Memory.FillRAM [address]);
	if (byte != Memory.FillRAM [address])
	    S9xSA1SetBWRAMMemMap (byte);
	break;
    case 0x2226:
//	printf ("BW-RAM SNES write %s\n", (byte & 0x80) ? "enabled" : "disabled");
	break;
    case 0x2227:
//	printf ("BW-RAM SA1 write %s\n", (byte & 0x80) ? "enabled" : "disabled");
	break;

    case 0x2228:
//	printf ("BW-RAM write protect area %02x\n", byte);
	break;
    case 0x2229:
//	printf ("I-RAM SNES write protect area %02x\n", byte);
	break;
    case 0x222a:
//	printf ("I-RAM SA1 write protect area %02x\n", byte);
	break;
    case 0x2230:
#if 0
	printf ("SA1 DMA %s\n", (byte & 0x80) ? "enabled" : "disabled");
	printf ("DMA priority %s\n", (byte & 0x40) ? "DMA" : "SA1");
	printf ("DMA %s\n", (byte & 0x20) ? "char conv" : "normal");
	printf ("DMA type %s\n", (byte & 0x10) ? "BW-RAM -> I-RAM" : "SA1 -> I-RAM");
	printf ("DMA distination %s\n", (byte & 4) ? "BW-RAM" : "I-RAM");
	printf ("DMA source %s\n", DMAsource [byte & 3]);
#endif
	break;
    case 0x2231:
	if (byte & 0x80)
	    SA1.in_char_dma = FALSE;
#if 0
	printf ("CHDEND %s\n", (byte & 0x80) ? "complete" : "incomplete");
	printf ("DMA colour mode %d\n", byte & 3);
	printf ("virtual VRAM width %d\n", (byte >> 2) & 7);
#endif
	break;
    case 0x2232:
    case 0x2233:
    case 0x2234:
	Memory.FillRAM [address] = byte;
#if 0
	printf ("DMA source start %06x\n",
		Memory.FillRAM [0x2232] | (Memory.FillRAM [0x2233] << 8) |
		(Memory.FillRAM [0x2234] << 16));
#endif
	break;
    case 0x2235:
	Memory.FillRAM [address] = byte;
	break;
    case 0x2236:
	Memory.FillRAM [address] = byte;
	if ((Memory.FillRAM [0x2230] & 0xa4) == 0x80)
	{
	    // Normal DMA to I-RAM
	    S9xSA1DMA ();
	}
	else
	if ((Memory.FillRAM [0x2230] & 0xb0) == 0xb0)
	{
	    Memory.FillRAM [0x2300] |= 0x20;
	    if (Memory.FillRAM [0x2201] & 0x20)
		S9xSetIRQ (SA1_DMA_IRQ_SOURCE);
	    SA1.in_char_dma = TRUE;
	}
	break;
    case 0x2237:
	Memory.FillRAM [address] = byte;
	if ((Memory.FillRAM [0x2230] & 0xa4) == 0x84)
	{
	    // Normal DMA to BW-RAM
	    S9xSA1DMA ();
	}
#if 0
	printf ("DMA dest address %06x\n",
		Memory.FillRAM [0x2235] | (Memory.FillRAM [0x2236] << 8) |
		(Memory.FillRAM [0x2237] << 16));
#endif
	break;
    case 0x2238:
    case 0x2239:
	Memory.FillRAM [address] = byte;
#if 0
	printf ("DMA length %04x\n",
		Memory.FillRAM [0x2238] | (Memory.FillRAM [0x2239] << 8));
#endif
	break;
    case 0x223f:
	SA1.VirtualBitmapFormat = (byte & 0x80) ? 2 : 4;
	//printf ("virtual VRAM depth %d\n", (byte & 0x80) ? 2 : 4);
	break;

    case 0x2240:    case 0x2241:    case 0x2242:    case 0x2243:
    case 0x2244:    case 0x2245:    case 0x2246:    case 0x2247:
    case 0x2248:    case 0x2249:    case 0x224a:    case 0x224b:
    case 0x224c:    case 0x224d:    case 0x224e:
#if 0
	if (!(SA1.Flags & TRACE_FLAG))
	{
	    TraceSA1 ();
	    Trace ();
	}
#endif
	Memory.FillRAM [address] = byte;
	break;

    case 0x224f:
	Memory.FillRAM [address] = byte;
	if ((Memory.FillRAM [0x2230] & 0xb0) == 0xa0)
	{
	    // Char conversion 2 DMA enabled
	    memmove (&Memory.ROM [CMemory::MAX_ROM_SIZE - 0x10000] + SA1.in_char_dma * 16,
		     &Memory.FillRAM [0x2240], 16);
	    SA1.in_char_dma = (SA1.in_char_dma + 1) & 7;
	    if ((SA1.in_char_dma & 3) == 0)
	    {
		S9xSA1CharConv2 ();
	    }
	}
	break;
    case 0x2250:
	if (byte & 2)
	    SA1.sum = 0;
	SA1.arithmetic_op = byte & 3;
	break;

    case 0x2251:
	SA1.op1 = (SA1.op1 & 0xff00) | byte;
	break;
    case 0x2252:
	SA1.op1 = (SA1.op1 & 0xff) | (byte << 8);
	break;
    case 0x2253:
	SA1.op2 = (SA1.op2 & 0xff00) | byte;
	break;
    case 0x2254:
	SA1.op2 = (SA1.op2 & 0xff) | (byte << 8);
	switch (SA1.arithmetic_op)
	{
        case 0:	// multiply
	    SA1.sum = SA1.op1 * SA1.op2;
	    break;
	case 1: // divide
	    if (SA1.op2 == 0)
		SA1.sum = SA1.op1 << 16;
	    else
	    {
		SA1.sum = (SA1.op1 / (int) ((uint16) SA1.op2)) |
			  ((SA1.op1 % (int) ((uint16) SA1.op2)) << 16);
	    }
	    break;
	case 2:
	default: // cumulative sum
	    SA1.sum += SA1.op1 * SA1.op2;
	    if (SA1.sum & ((int64) 0xffffff << 32))
		SA1.overflow = TRUE;
	    break;
	}
	break;
    case 0x2258:    // Variable bit-field length/auto inc/start.
	Memory.FillRAM [0x2258] = byte;
	S9xSA1ReadVariableLengthData (TRUE, FALSE);
	return;
    case 0x2259:
    case 0x225a:
    case 0x225b:    // Variable bit-field start address
	Memory.FillRAM [address] = byte;
	// XXX: ???
	SA1.variable_bit_pos = 0;
	S9xSA1ReadVariableLengthData (FALSE, TRUE);
	return;
    default:
//	printf ("W: %02x->%04x\n", byte, address);
	break;
    }
    if (address >= 0x2200 && address <= 0x22ff)
	Memory.FillRAM [address] = byte;
}

static void S9xSA1CharConv2 ()
{
    uint32 dest = Memory.FillRAM [0x2235] | (Memory.FillRAM [0x2236] << 8);
    uint32 offset = (SA1.in_char_dma & 7) ? 0 : 1;
    int depth = (Memory.FillRAM [0x2231] & 3) == 0 ? 8 :
		(Memory.FillRAM [0x2231] & 3) == 1 ? 4 : 2;
    int bytes_per_char = 8 * depth;
    uint8 *p = &Memory.FillRAM [0x3000] + dest + offset * bytes_per_char;
    uint8 *q = &Memory.ROM [CMemory::MAX_ROM_SIZE - 0x10000] + offset * 64;

    switch (depth)
    {
    case 2:
	break;
    case 4:
	break;
    case 8:
	for (int l = 0; l < 8; l++, q += 8)
	{
	    for (int b = 0; b < 8; b++)
	    {
		uint8 r = *(q + b);
		*(p +  0) = (*(p +  0) << 1) | ((r >> 0) & 1);
		*(p +  1) = (*(p +  1) << 1) | ((r >> 1) & 1);
		*(p + 16) = (*(p + 16) << 1) | ((r >> 2) & 1);
		*(p + 17) = (*(p + 17) << 1) | ((r >> 3) & 1);
		*(p + 32) = (*(p + 32) << 1) | ((r >> 4) & 1);
		*(p + 33) = (*(p + 33) << 1) | ((r >> 5) & 1);
		*(p + 48) = (*(p + 48) << 1) | ((r >> 6) & 1);
		*(p + 49) = (*(p + 49) << 1) | ((r >> 7) & 1);
	    }
	    p += 2;
	}
	break;
    }
}

static void S9xSA1DMA ()
{
    uint32 src =  Memory.FillRAM [0x2232] |
	         (Memory.FillRAM [0x2233] << 8) |
		 (Memory.FillRAM [0x2234] << 16);
    uint32 dst =  Memory.FillRAM [0x2235] |
	         (Memory.FillRAM [0x2236] << 8) |
		 (Memory.FillRAM [0x2237] << 16);
    uint32 len =  Memory.FillRAM [0x2238] |
		 (Memory.FillRAM [0x2239] << 8);

    uint8 *s;
    uint8 *d;

    switch (Memory.FillRAM [0x2230] & 3)
    {
    case 0: // ROM
	s = SA1.Map [((src&0xffffff) >> MEMMAP_SHIFT)];
	if (s >= (uint8 *) CMemory::MAP_LAST)
	    s += (src & 0xffff);
	else
	    s = Memory.ROM + (src & 0xffff);
	break;
    case 1: // BW-RAM
	src &= Memory.SRAMMask;
	len &= Memory.SRAMMask;
	s = Memory.SRAM + src;
	break;
    default:
    case 2:
	src &= 0x3ff;
	len &= 0x3ff;
	s = &Memory.FillRAM [0x3000] + src;
	break;
    }

    if (Memory.FillRAM [0x2230] & 4)
    {
	dst &= Memory.SRAMMask;
	len &= Memory.SRAMMask;
	d = Memory.SRAM + dst;
    }
    else
    {
	dst &= 0x3ff;
	len &= 0x3ff;
	d = &Memory.FillRAM [0x3000] + dst;
    }
    memmove (d, s, len);
    Memory.FillRAM [0x2301] |= 0x20;

    if (Memory.FillRAM [0x220a] & 0x20)
    {
	SA1.Flags |= IRQ_FLAG;
	SA1.IRQActive |= DMA_IRQ_SOURCE;
//	SA1.Executing = !SA1.Waiting;
    }
}

void S9xSA1ReadVariableLengthData (bool8 inc, bool8 no_shift)
{
    uint32 addr =  Memory.FillRAM [0x2259] |
		  (Memory.FillRAM [0x225a] << 8) |
		  (Memory.FillRAM [0x225b] << 16);
    uint8 shift = Memory.FillRAM [0x2258] & 15;

    if (no_shift)
	shift = 0;
    else
    if (shift == 0)
	shift = 16;

    uint8 s = shift + SA1.variable_bit_pos;

    if (s >= 16)
    {
	addr += (s >> 4) << 1;
	s &= 15;
    }
    uint32 data = S9xSA1GetWord (addr) |
		  (S9xSA1GetWord (addr + 2) << 16);

    data >>= s;
    Memory.FillRAM [0x230c] = (uint8) data;
    Memory.FillRAM [0x230d] = (uint8) (data >> 8);
    if (inc)
    {
	SA1.variable_bit_pos = (SA1.variable_bit_pos + shift) & 15;
	Memory.FillRAM [0x2259] = (uint8) addr;
	Memory.FillRAM [0x225a] = (uint8) (addr >> 8);
	Memory.FillRAM [0x225b] = (uint8) (addr >> 16);
    }
}

