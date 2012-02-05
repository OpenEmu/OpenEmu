/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


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
                             Andreas Naive (andreasnaive@gmail.com),
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
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

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
 ***********************************************************************************/


#include "snes9x.h"
#include "memmap.h"

uint8	SA1OpenBus;

static void S9xSA1SetBWRAMMemMap (uint8);
static void S9xSetSA1MemMap (uint32, uint8);
static void S9xSA1CharConv2 (void);
static void S9xSA1DMA (void);
static void S9xSA1ReadVariableLengthData (bool8, bool8);


void S9xSA1Init (void)
{
	SA1.Cycles = 0;
	SA1.PrevCycles = 0;
	SA1.Flags = 0;
	SA1.WaitingForInterrupt = FALSE;

	memset(&Memory.FillRAM[0x2200], 0, 0x200);
	Memory.FillRAM[0x2200] = 0x20;
	Memory.FillRAM[0x2220] = 0x00;
	Memory.FillRAM[0x2221] = 0x01;
	Memory.FillRAM[0x2222] = 0x02;
	Memory.FillRAM[0x2223] = 0x03;
	Memory.FillRAM[0x2228] = 0x0f;

	SA1.in_char_dma = FALSE;
	SA1.TimerIRQLastState = FALSE;
	SA1.HTimerIRQPos = 0;
	SA1.VTimerIRQPos = 0;
	SA1.HCounter = 0;
	SA1.VCounter = 0;
	SA1.PrevHCounter = 0;
	SA1.arithmetic_op = 0;
	SA1.op1 = 0;
	SA1.op2 = 0;
	SA1.sum = 0;
	SA1.overflow = FALSE;
	SA1.VirtualBitmapFormat = 0;
	SA1.variable_bit_pos = 0;

	SA1Registers.PBPC = 0;
	SA1Registers.PB = 0;
	SA1Registers.PCw = 0;
	SA1Registers.D.W = 0;
	SA1Registers.DB = 0;
	SA1Registers.SH = 1;
	SA1Registers.SL = 0xFF;
	SA1Registers.XH = 0;
	SA1Registers.YH = 0;
	SA1Registers.P.W = 0;

	SA1.ShiftedPB = 0;
	SA1.ShiftedDB = 0;
	SA1SetFlags(MemoryFlag | IndexFlag | IRQ | Emulation);
	SA1ClearFlags(Decimal);

	SA1.MemSpeed = SLOW_ONE_CYCLE;
	SA1.MemSpeedx2 = SLOW_ONE_CYCLE * 2;

	SA1.S9xOpcodes = S9xSA1OpcodesM1X1;
	SA1.S9xOpLengths = S9xOpLengthsM1X1;

	S9xSA1SetPCBase(SA1Registers.PBPC);

	S9xSA1UnpackStatus();
	S9xSA1FixCycles();

	SA1.BWRAM = Memory.SRAM;

	CPU.IRQExternal = FALSE;
}

static void S9xSA1SetBWRAMMemMap (uint8 val)
{
	if (val & 0x80)
	{
		for (int c = 0; c < 0x400; c += 16)
		{
			SA1.Map[c + 6] = SA1.Map[c + 0x806] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
			SA1.Map[c + 7] = SA1.Map[c + 0x807] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
			SA1.WriteMap[c + 6] = SA1.WriteMap[c + 0x806] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
			SA1.WriteMap[c + 7] = SA1.WriteMap[c + 0x807] = (uint8 *) CMemory::MAP_BWRAM_BITMAP2;
		}

		SA1.BWRAM = Memory.SRAM + (val & 0x7f) * 0x2000 / 4;
	}
	else
	{
		for (int c = 0; c < 0x400; c += 16)
		{
			SA1.Map[c + 6] = SA1.Map[c + 0x806] = (uint8 *) CMemory::MAP_BWRAM;
			SA1.Map[c + 7] = SA1.Map[c + 0x807] = (uint8 *) CMemory::MAP_BWRAM;
			SA1.WriteMap[c + 6] = SA1.WriteMap[c + 0x806] = (uint8 *) CMemory::MAP_BWRAM;
			SA1.WriteMap[c + 7] = SA1.WriteMap[c + 0x807] = (uint8 *) CMemory::MAP_BWRAM;
		}

		SA1.BWRAM = Memory.SRAM + (val & 7) * 0x2000;
	}
}

void S9xSA1PostLoadState (void)
{
	SA1.ShiftedPB = (uint32) SA1Registers.PB << 16;
	SA1.ShiftedDB = (uint32) SA1Registers.DB << 16;

	S9xSA1SetPCBase(SA1Registers.PBPC);
	S9xSA1UnpackStatus();
	S9xSA1FixCycles();
	SA1.VirtualBitmapFormat = (Memory.FillRAM[0x223f] & 0x80) ? 2 : 4;
	Memory.BWRAM = Memory.SRAM + (Memory.FillRAM[0x2224] & 7) * 0x2000;
	S9xSA1SetBWRAMMemMap(Memory.FillRAM[0x2225]);
}

static void S9xSetSA1MemMap (uint32 which1, uint8 map)
{
	int	start  = which1 * 0x100 + 0xc00;
	int	start2 = which1 * 0x200;

	if (which1 >= 2)
		start2 += 0x400;

	for (int c = 0; c < 0x100; c += 16)
	{
		uint8	*block = &Memory.ROM[(map & 7) * 0x100000 + (c << 12)];
		for (int i = c; i < c + 16; i++)
			Memory.Map[start  + i] = SA1.Map[start  + i] = block;
	}

	for (int c = 0; c < 0x200; c += 16)
	{
		uint8	*block = &Memory.ROM[(map & 7) * 0x100000 + (c << 11) - 0x8000];
		for (int i = c + 8; i < c + 16; i++)
			Memory.Map[start2 + i] = SA1.Map[start2 + i] = block;
	}
}

uint8 S9xGetSA1 (uint32 address)
{
	switch (address)
	{
		case 0x2300: // S-CPU flag
			return ((Memory.FillRAM[0x2209] & 0x5f) | (Memory.FillRAM[0x2300] & 0xa0));

		case 0x2301: // SA-1 flag
			return ((Memory.FillRAM[0x2200] & 0x0f) | (Memory.FillRAM[0x2301] & 0xf0));

		case 0x2302: // H counter (L)
			SA1.HTimerIRQPos = SA1.HCounter / ONE_DOT_CYCLE;
			SA1.VTimerIRQPos = SA1.VCounter;
			return ((uint8)  SA1.HTimerIRQPos);

		case 0x2303: // H counter (H)
			return ((uint8) (SA1.HTimerIRQPos >> 8));

		case 0x2304: // V counter (L)
			return ((uint8)  SA1.VTimerIRQPos);

		case 0x2305: // V counter (H)
			return ((uint8) (SA1.VTimerIRQPos >> 8));

		case 0x2306: // arithmetic result (LLL)
			return ((uint8)  SA1.sum);

		case 0x2307: // arithmetic result (LLH)
			return ((uint8) (SA1.sum >>  8));

		case 0x2308: // arithmetic result (LHL)
			return ((uint8) (SA1.sum >> 16));

		case 0x2309: // arithmetic result (LLH)
			return ((uint8) (SA1.sum >> 24));

		case 0x230a: // arithmetic result (HLL)
			return ((uint8) (SA1.sum >> 32));

		case 0x230b: // arithmetic overflow
			return (SA1.overflow ? 0x80 : 0);

		case 0x230c: // variable-length data read port (L)
			return (Memory.FillRAM[0x230c]);

		case 0x230d: // variable-length data read port (H)
		{
			uint8	byte = Memory.FillRAM[0x230d];

			if (Memory.FillRAM[0x2258] & 0x80)
				S9xSA1ReadVariableLengthData(TRUE, FALSE);

			return (byte);
		}

		case 0x230e: // version code register
			return (0x01);

		default:
			break;
	}

	return (Memory.FillRAM[address]);
}

void S9xSetSA1 (uint8 byte, uint32 address)
{
	switch (address)
	{
		case 0x2200: // SA-1 control
		#ifdef DEBUGGER
			if (byte & 0x60)
				printf("SA-1 sleep\n");
		#endif

			// SA-1 reset
			if (!(byte & 0x80) && (Memory.FillRAM[0x2200] & 0x20))
			{
			#ifdef DEBUGGER
				printf("SA-1 reset\n");
			#endif
				SA1Registers.PBPC = 0;
				SA1Registers.PB = 0;
				SA1Registers.PCw = Memory.FillRAM[0x2203] | (Memory.FillRAM[0x2204] << 8);
				S9xSA1SetPCBase(SA1Registers.PBPC);
			}

			// SA-1 IRQ control
			if (byte & 0x80)
			{
				Memory.FillRAM[0x2301] |= 0x80;
				if (Memory.FillRAM[0x220a] & 0x80)
					Memory.FillRAM[0x220b] &= ~0x80;
			}

			// SA-1 NMI control
			if (byte & 0x10)
			{
				Memory.FillRAM[0x2301] |= 0x10;
				if (Memory.FillRAM[0x220a] & 0x10)
					Memory.FillRAM[0x220b] &= ~0x10;
			}

			break;

		case 0x2201: // S-CPU interrupt enable
			// S-CPU IRQ enable
			if (((byte ^ Memory.FillRAM[0x2201]) & 0x80) && (Memory.FillRAM[0x2300] & byte & 0x80))
			{
				Memory.FillRAM[0x2202] &= ~0x80;
				CPU.IRQExternal = TRUE;
			}

			// S-CPU CHDMA IRQ enable
			if (((byte ^ Memory.FillRAM[0x2201]) & 0x20) && (Memory.FillRAM[0x2300] & byte & 0x20))
			{
				Memory.FillRAM[0x2202] &= ~0x20;
				CPU.IRQExternal = TRUE;
			}

			break;

		case 0x2202: // S-CPU interrupt clear
			// S-CPU IRQ clear
			if (byte & 0x80)
				Memory.FillRAM[0x2300] &= ~0x80;

			// S-CPU CHDMA IRQ clear
			if (byte & 0x20)
				Memory.FillRAM[0x2300] &= ~0x20;

			if (!(Memory.FillRAM[0x2300] & 0xa0))
				CPU.IRQExternal = FALSE;

			break;

		case 0x2203: // SA-1 reset vector (L)
		case 0x2204: // SA-1 reset vector (H)
		case 0x2205: // SA-1 NMI vector (L)
		case 0x2206: // SA-1 NMI vector (H)
		case 0x2207: // SA-1 IRQ vector (L)
		case 0x2208: // SA-1 IRQ vector (H)
			break;

		case 0x2209: // S-CPU control
			// 0x40: S-CPU IRQ overwrite
			// 0x20: S-CPU NMI overwrite

			// S-CPU IRQ control
			if (byte & 0x80)
			{
				Memory.FillRAM[0x2300] |= 0x80;
				if (Memory.FillRAM[0x2201] & 0x80)
				{
					Memory.FillRAM[0x2202] &= ~0x80;
					CPU.IRQExternal = TRUE;
				}
			}

			break;

		case 0x220a: // SA-1 interrupt enable
			// SA-1 IRQ enable
			if (((byte ^ Memory.FillRAM[0x220a]) & 0x80) && (Memory.FillRAM[0x2301] & byte & 0x80))
				Memory.FillRAM[0x220b] &= ~0x80;

			// SA-1 timer IRQ enable
			if (((byte ^ Memory.FillRAM[0x220a]) & 0x40) && (Memory.FillRAM[0x2301] & byte & 0x40))
				Memory.FillRAM[0x220b] &= ~0x40;

			// SA-1 DMA IRQ enable
			if (((byte ^ Memory.FillRAM[0x220a]) & 0x20) && (Memory.FillRAM[0x2301] & byte & 0x20))
				Memory.FillRAM[0x220b] &= ~0x20;

			// SA-1 NMI enable
			if (((byte ^ Memory.FillRAM[0x220a]) & 0x10) && (Memory.FillRAM[0x2301] & byte & 0x10))
				Memory.FillRAM[0x220b] &= ~0x10;

			break;

		case 0x220b: // SA-1 interrupt clear
			// SA-1 IRQ clear
			if (byte & 0x80)
				Memory.FillRAM[0x2301] &= ~0x80;

			// SA-1 timer IRQ clear
			if (byte & 0x40)
				Memory.FillRAM[0x2301] &= ~0x40;

			// SA-1 DMA IRQ clear
			if (byte & 0x20)
				Memory.FillRAM[0x2301] &= ~0x20;

			// SA-1 NMI clear
			if (byte & 0x10)
				Memory.FillRAM[0x2301] &= ~0x10;

			break;

		case 0x220c: // S-CPU NMI vector (L)
		case 0x220d: // S-CPU NMI vector (H)
		case 0x220e: // S-CPU IRQ vector (L)
		case 0x220f: // S-CPU IRQ vector (H)
			break;

		case 0x2210: // SA-1 timer control
			// 0x80: mode (linear / HV)
			// 0x02: V timer enable
			// 0x01: H timer enable
		#ifdef DEBUGGER
			printf("SA-1 timer control write:%02x\n", byte);
		#endif
			break;

		case 0x2211: // SA-1 timer reset
			SA1.HCounter = 0;
			SA1.VCounter = 0;
			break;

		case 0x2212: // SA-1 H-timer (L)
			SA1.HTimerIRQPos = byte | (Memory.FillRAM[0x2213] << 8);
			break;

		case 0x2213: // SA-1 H-timer (H)
			SA1.HTimerIRQPos = (byte << 8) | Memory.FillRAM[0x2212];
			break;

		case 0x2214: // SA-1 V-timer (L)
			SA1.VTimerIRQPos = byte | (Memory.FillRAM[0x2215] << 8);
			break;

		case 0x2215: // SA-1 V-timer (H)
			SA1.VTimerIRQPos = (byte << 8) | Memory.FillRAM[0x2214];
			break;

		case 0x2220: // MMC bank C
		case 0x2221: // MMC bank D
		case 0x2222: // MMC bank E
		case 0x2223: // MMC bank F
			S9xSetSA1MemMap(address - 0x2220, byte);
			break;

		case 0x2224: // S-CPU BW-RAM mapping
			Memory.BWRAM = Memory.SRAM + (byte & 7) * 0x2000;
			break;

		case 0x2225: // SA-1 BW-RAM mapping
			if (byte != Memory.FillRAM[0x2225])
				S9xSA1SetBWRAMMemMap(byte);

			break;

		case 0x2226: // S-CPU BW-RAM write enable
		case 0x2227: // SA-1 BW-RAM write enable
		case 0x2228: // BW-RAM write-protected area
		case 0x2229: // S-CPU I-RAM write protection
		case 0x222a: // SA-1 I-RAM write protection
			break;

		case 0x2230: // DMA control
			// 0x80: enable
			// 0x40: priority (DMA / SA-1)
			// 0x20: character conversion / normal
			// 0x10: BW-RAM -> I-RAM / SA-1 -> I-RAM
			// 0x04: destinatin (BW-RAM / I-RAM)
			// 0x03: source
			break;

		case 0x2231: // character conversion DMA parameters
			// 0x80: CHDEND (complete / incomplete)
			// 0x03: color mode
			// (byte >> 2) & 7: virtual VRAM width
			if (byte & 0x80)
				SA1.in_char_dma = FALSE;

			break;

		case 0x2232: // DMA source start address (LL)
		case 0x2233: // DMA source start address (LH)
		case 0x2234: // DMA source start address (HL)
			break;

		case 0x2235: // DMA destination start address (LL)
			break;

		case 0x2236: // DMA destination start address (LH)
			Memory.FillRAM[0x2236] = byte;

			if ((Memory.FillRAM[0x2230] & 0xa4) == 0x80) // Normal DMA to I-RAM
				S9xSA1DMA();
			else
			if ((Memory.FillRAM[0x2230] & 0xb0) == 0xb0) // CC1
			{
				SA1.in_char_dma = TRUE;

				Memory.FillRAM[0x2300] |= 0x20;
				if (Memory.FillRAM[0x2201] & 0x20)
				{
					Memory.FillRAM[0x2202] &= ~0x20;
					CPU.IRQExternal = TRUE;
				}
			}

			break;

		case 0x2237: // DMA destination start address (HL)
			Memory.FillRAM[0x2237] = byte;

			if ((Memory.FillRAM[0x2230] & 0xa4) == 0x84) // Normal DMA to BW-RAM
				S9xSA1DMA();

			break;

		case 0x2238: // DMA terminal counter (L)
		case 0x2239: // DMA terminal counter (H)
			break;

		case 0x223f: // BW-RAM bitmap format
			SA1.VirtualBitmapFormat = (byte & 0x80) ? 2 : 4;
			break;

		case 0x2240: // bitmap register 0
		case 0x2241: // bitmap register 1
		case 0x2242: // bitmap register 2
		case 0x2243: // bitmap register 3
		case 0x2244: // bitmap register 4
		case 0x2245: // bitmap register 5
		case 0x2246: // bitmap register 6
		case 0x2247: // bitmap register 7
		case 0x2248: // bitmap register 8
		case 0x2249: // bitmap register 9
		case 0x224a: // bitmap register A
		case 0x224b: // bitmap register B
		case 0x224c: // bitmap register C
		case 0x224d: // bitmap register D
		case 0x224e: // bitmap register E
			break;

		case 0x224f: // bitmap register F
			Memory.FillRAM[0x224f] = byte;

			if ((Memory.FillRAM[0x2230] & 0xb0) == 0xa0) // CC2
			{
				memmove(&Memory.ROM[CMemory::MAX_ROM_SIZE - 0x10000] + SA1.in_char_dma * 16, &Memory.FillRAM[0x2240], 16);
				SA1.in_char_dma = (SA1.in_char_dma + 1) & 7;
				if ((SA1.in_char_dma & 3) == 0)
					S9xSA1CharConv2();
			}

			break;

		case 0x2250: // arithmetic control
			if (byte & 2)
				SA1.sum = 0;
			SA1.arithmetic_op = byte & 3;
			break;

		case 0x2251: // multiplicand / dividend (L)
			SA1.op1 = (SA1.op1 & 0xff00) |  byte;
			break;

		case 0x2252: // multiplicand / dividend (H)
			SA1.op1 = (SA1.op1 & 0x00ff) | (byte << 8);
			break;

		case 0x2253: // multiplier / divisor (L)
			SA1.op2 = (SA1.op2 & 0xff00) |  byte;
			break;

		case 0x2254: // multiplier / divisor (H)
			SA1.op2 = (SA1.op2 & 0x00ff) | (byte << 8);

			switch (SA1.arithmetic_op)
			{
				case 0:	// signed multiplication
					SA1.sum = (int16) SA1.op1 * (int16) SA1.op2;
					SA1.op2 = 0;
					break;

				case 1: // unsigned division
					if (SA1.op2 == 0)
						SA1.sum = 0;
					else
					{
						int16	quotient  = (int16) SA1.op1 / (uint16) SA1.op2;
						uint16	remainder = (int16) SA1.op1 % (uint16) SA1.op2;
						SA1.sum = (remainder << 16) | quotient;
					}

					SA1.op1 = 0;
					SA1.op2 = 0;
					break;

				case 2: // cumulative sum
				default:
					SA1.sum += (int16) SA1.op1 * (int16) SA1.op2;
					SA1.overflow = (SA1.sum >= (1ULL << 40));
					SA1.sum &= (1ULL << 40) - 1;
					SA1.op2 = 0;
					break;
			}

			break;

		case 0x2258: // variable bit-field length / auto inc / start
			Memory.FillRAM[0x2258] = byte;
			S9xSA1ReadVariableLengthData(TRUE, FALSE);
			return;

		case 0x2259: // variable bit-field start address (LL)
		case 0x225a: // variable bit-field start address (LH)
		case 0x225b: // variable bit-field start address (HL)
			Memory.FillRAM[address] = byte;
			// XXX: ???
			SA1.variable_bit_pos = 0;
			S9xSA1ReadVariableLengthData(FALSE, TRUE);
			return;

		default:
			break;
	}

	if (address >= 0x2200 && address <= 0x22ff)
		Memory.FillRAM[address] = byte;
}

static void S9xSA1CharConv2 (void)
{
	uint32	dest           = Memory.FillRAM[0x2235] | (Memory.FillRAM[0x2236] << 8);
	uint32	offset         = (SA1.in_char_dma & 7) ? 0 : 1;
	int		depth          = (Memory.FillRAM[0x2231] & 3) == 0 ? 8 : (Memory.FillRAM[0x2231] & 3) == 1 ? 4 : 2;
	int		bytes_per_char = 8 * depth;
	uint8	*p             = &Memory.FillRAM[0x3000] + (dest & 0x7ff) + offset * bytes_per_char;
	uint8	*q             = &Memory.ROM[CMemory::MAX_ROM_SIZE - 0x10000] + offset * 64;

	switch (depth)
	{
		case 2:
			for (int l = 0; l < 8; l++, q += 8)
			{
				for (int b = 0; b < 8; b++)
				{
					uint8	r = *(q + b);
					*(p +  0) = (*(p +  0) << 1) | ((r >> 0) & 1);
					*(p +  1) = (*(p +  1) << 1) | ((r >> 1) & 1);
				}

				p += 2;
			}

			break;

		case 4:
			for (int l = 0; l < 8; l++, q += 8)
			{
				for (int b = 0; b < 8; b++)
				{
					uint8	r = *(q + b);
					*(p +  0) = (*(p +  0) << 1) | ((r >> 0) & 1);
					*(p +  1) = (*(p +  1) << 1) | ((r >> 1) & 1);
					*(p + 16) = (*(p + 16) << 1) | ((r >> 2) & 1);
					*(p + 17) = (*(p + 17) << 1) | ((r >> 3) & 1);
				}

				p += 2;
			}

			break;

		case 8:
			for (int l = 0; l < 8; l++, q += 8)
			{
				for (int b = 0; b < 8; b++)
				{
					uint8	r = *(q + b);
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

static void S9xSA1DMA (void)
{
	uint32	src = Memory.FillRAM[0x2232] | (Memory.FillRAM[0x2233] << 8) | (Memory.FillRAM[0x2234] << 16);
	uint32	dst = Memory.FillRAM[0x2235] | (Memory.FillRAM[0x2236] << 8) | (Memory.FillRAM[0x2237] << 16);
	uint32	len = Memory.FillRAM[0x2238] | (Memory.FillRAM[0x2239] << 8);
	uint8	*s, *d;

	switch (Memory.FillRAM[0x2230] & 3)
	{
		case 0: // ROM
			s = SA1.Map[((src & 0xffffff) >> MEMMAP_SHIFT)];
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
			s = &Memory.FillRAM[0x3000] + src;
			break;
	}

	if (Memory.FillRAM[0x2230] & 4)
	{
		dst &= Memory.SRAMMask;
		len &= Memory.SRAMMask;
		d = Memory.SRAM + dst;
	}
	else
	{
		dst &= 0x3ff;
		len &= 0x3ff;
		d = &Memory.FillRAM[0x3000] + dst;
	}

	memmove(d, s, len);

	// SA-1 DMA IRQ control
	Memory.FillRAM[0x2301] |= 0x20;
	if (Memory.FillRAM[0x220a] & 0x20)
		Memory.FillRAM[0x220b] &= ~0x20;
}

static void S9xSA1ReadVariableLengthData (bool8 inc, bool8 no_shift)
{
	uint32	addr  = Memory.FillRAM[0x2259] | (Memory.FillRAM[0x225a] << 8) | (Memory.FillRAM[0x225b] << 16);
	uint8	shift = Memory.FillRAM[0x2258] & 15;

	if (no_shift)
		shift = 0;
	else
	if (shift == 0)
		shift = 16;

	uint8	s = shift + SA1.variable_bit_pos;

	if (s >= 16)
	{
		addr += (s >> 4) << 1;
		s &= 15;
	}

	uint32	data = S9xSA1GetWord(addr) | (S9xSA1GetWord(addr + 2) << 16);

	data >>= s;
	Memory.FillRAM[0x230c] = (uint8) data;
	Memory.FillRAM[0x230d] = (uint8) (data >> 8);

	if (inc)
	{
		SA1.variable_bit_pos = (SA1.variable_bit_pos + shift) & 15;
		Memory.FillRAM[0x2259] = (uint8) addr;
		Memory.FillRAM[0x225a] = (uint8) (addr >> 8);
		Memory.FillRAM[0x225b] = (uint8) (addr >> 16);
	}
}

uint8 S9xSA1GetByte (uint32 address)
{
	uint8	*GetAddress = SA1.Map[(address & 0xffffff) >> MEMMAP_SHIFT];

	if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
		return (*(GetAddress + (address & 0xffff)));

	switch ((pint) GetAddress)
	{
		case CMemory::MAP_PPU:
			return (S9xGetSA1(address & 0xffff));

		case CMemory::MAP_LOROM_SRAM:
		case CMemory::MAP_SA1RAM:
			return (*(Memory.SRAM + (address & 0xffff)));

		case CMemory::MAP_BWRAM:
			return (*(SA1.BWRAM + ((address & 0x7fff) - 0x6000)));

		case CMemory::MAP_BWRAM_BITMAP:
			address -= 0x600000;
			if (SA1.VirtualBitmapFormat == 2)
				return ((Memory.SRAM[(address >> 2) & 0xffff] >> ((address & 3) << 1)) &  3);
			else
				return ((Memory.SRAM[(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);

		case CMemory::MAP_BWRAM_BITMAP2:
			address = (address & 0xffff) - 0x6000;
			if (SA1.VirtualBitmapFormat == 2)
				return ((SA1.BWRAM[(address >> 2) & 0xffff] >> ((address & 3) << 1)) &  3);
			else
				return ((SA1.BWRAM[(address >> 1) & 0xffff] >> ((address & 1) << 2)) & 15);

		default:
			return (SA1OpenBus);
	}
}

uint16 S9xSA1GetWord (uint32 address, s9xwrap_t w)
{
	PC_t	a;

	SA1OpenBus = S9xSA1GetByte(address);

	switch (w)
	{
		case WRAP_PAGE:
			a.xPBPC = address;
			a.B.xPCl++;
			return (SA1OpenBus | (S9xSA1GetByte(a.xPBPC) << 8));

		case WRAP_BANK:
			a.xPBPC = address;
			a.W.xPC++;
			return (SA1OpenBus | (S9xSA1GetByte(a.xPBPC) << 8));

		case WRAP_NONE:
		default:
			return (SA1OpenBus | (S9xSA1GetByte(address + 1) << 8));
	}
}

void S9xSA1SetByte (uint8 byte, uint32 address)
{
	uint8	*SetAddress = SA1.WriteMap[(address & 0xffffff) >> MEMMAP_SHIFT];

	if (SetAddress >= (uint8 *) CMemory::MAP_LAST)
	{
		*(SetAddress + (address & 0xffff)) = byte;
		return;
	}

	switch ((pint) SetAddress)
	{
		case CMemory::MAP_PPU:
			S9xSetSA1(byte, address & 0xffff);
			return;

		case CMemory::MAP_LOROM_SRAM:
		case CMemory::MAP_SA1RAM:
			*(Memory.SRAM + (address & 0xffff)) = byte;
			return;

		case CMemory::MAP_BWRAM:
			*(SA1.BWRAM + ((address & 0x7fff) - 0x6000)) = byte;
			return;

		case CMemory::MAP_BWRAM_BITMAP:
			address -= 0x600000;
			if (SA1.VirtualBitmapFormat == 2)
			{
				uint8	*ptr = &Memory.SRAM[(address >> 2) & 0xffff];
				*ptr &= ~(3  << ((address & 3) << 1));
				*ptr |= (byte &  3) << ((address & 3) << 1);
			}
			else
			{
				uint8	*ptr = &Memory.SRAM[(address >> 1) & 0xffff];
				*ptr &= ~(15 << ((address & 1) << 2));
				*ptr |= (byte & 15) << ((address & 1) << 2);
			}

			return;

		case CMemory::MAP_BWRAM_BITMAP2:
			address = (address & 0xffff) - 0x6000;
			if (SA1.VirtualBitmapFormat == 2)
			{
				uint8	*ptr = &SA1.BWRAM[(address >> 2) & 0xffff];
				*ptr &= ~(3  << ((address & 3) << 1));
				*ptr |= (byte &  3) << ((address & 3) << 1);
			}
			else
			{
				uint8	*ptr = &SA1.BWRAM[(address >> 1) & 0xffff];
				*ptr &= ~(15 << ((address & 1) << 2));
				*ptr |= (byte & 15) << ((address & 1) << 2);
			}

			return;

		default:
			return;
	}
}

void S9xSA1SetWord (uint16 Word, uint32 address, enum s9xwrap_t w, enum s9xwriteorder_t o)
{
	PC_t	a;

	if (!o)
		S9xSA1SetByte((uint8) Word, address);

	switch (w)
	{
		case WRAP_PAGE:
			a.xPBPC = address;
			a.B.xPCl++;
			S9xSA1SetByte(Word >> 8, a.xPBPC);
			break;

		case WRAP_BANK:
			a.xPBPC = address;
			a.W.xPC++;
			S9xSA1SetByte(Word >> 8, a.xPBPC);
			break;

		case WRAP_NONE:
		default:
			S9xSA1SetByte(Word >> 8, address + 1);
			break;
	}

	if (o)
		S9xSA1SetByte((uint8) Word, address);
}

void S9xSA1SetPCBase (uint32 address)
{
	SA1Registers.PBPC = address & 0xffffff;
	SA1.ShiftedPB = address & 0xff0000;

	// FIXME
	SA1.MemSpeed = memory_speed(address);
	SA1.MemSpeedx2 = SA1.MemSpeed << 1;

	uint8	*GetAddress = SA1.Map[(address & 0xffffff) >> MEMMAP_SHIFT];

	if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
	{
		SA1.PCBase = GetAddress;
		return;
	}

	switch ((pint) GetAddress)
	{
		case CMemory::MAP_LOROM_SRAM:
			if ((Memory.SRAMMask & MEMMAP_MASK) != MEMMAP_MASK)
				SA1.PCBase = NULL;
			else
				SA1.PCBase = (Memory.SRAM + ((((address & 0xff0000) >> 1) | (address & 0x7fff)) & Memory.SRAMMask)) - (address & 0xffff);
			return;

		case CMemory::MAP_HIROM_SRAM:
			if ((Memory.SRAMMask & MEMMAP_MASK) != MEMMAP_MASK)
				SA1.PCBase = NULL;
			else
				SA1.PCBase = (Memory.SRAM + (((address & 0x7fff) - 0x6000 + ((address & 0xf0000) >> 3)) & Memory.SRAMMask)) - (address & 0xffff);
			return;

		case CMemory::MAP_BWRAM:
			SA1.PCBase = SA1.BWRAM - 0x6000 - (address & 0x8000);
			return;

		case CMemory::MAP_SA1RAM:
			SA1.PCBase = Memory.SRAM;
			return;

		default:
			SA1.PCBase = NULL;
			return;
	}
}
