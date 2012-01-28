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


#ifdef DEBUGGER

#include <stdarg.h>
#include "snes9x.h"
#include "memmap.h"
#include "cpuops.h"
#include "dma.h"
#include "apu/apu.h"
#include "display.h"
#include "debug.h"
#include "missing.h"

extern SDMA	DMA[8];
extern FILE	*apu_trace;
FILE		*trace = NULL, *trace2 = NULL;

struct SBreakPoint	S9xBreakpoint[6];

struct SDebug
{
	struct
	{
		uint8	Bank;
		uint16	Address;
	}	Dump;

	struct
	{
		uint8	Bank;
		uint16	Address;
	}	Unassemble;
};

static struct SDebug	Debug = { { 0, 0 }, { 0, 0 } };

static const char	*HelpMessage[] =
{
	"Command Help:",
	"?, help                - Shows this command help",
	"r                      - Shows the registers",
	"i                      - Shows the interrupt vectors",
	"t                      - Trace current instruction   [step-into]",
	"p                      - Proceed to next instruction [step-over]",
	"s                      - Skip to next instruction    [skip]",
	"T                      - Toggle CPU instruction tracing to trace.log",
	"TS                     - Toggle SA-1 instruction tracing to trace_sa1.log",
	"E                      - Toggle HC-based event tracing to trace.log",
	"V                      - Toggle non-DMA V-RAM read/write tracing to stdout",
	"D                      - Toggle on-screen DMA tracing",
	"H                      - Toggle on-screen HDMA tracing",
	"U                      - Toggle on-screen unknown register read/write tracing",
	"P                      - Toggle on-screen DSP tracing",
	"S                      - Dump sprite (OBJ) status",
	"g [Address]            - Go or go to [Address]",
	"u [Address]            - Disassemble from PC or [Address]",
	"d [Address]            - Dump from PC or [Address]",
	"bv [Number]            - View breakpoints or view breakpoint [Number]",
	"bs [Number] [Address]  - Enable/disable breakpoint",
	"                         [enable example: bs #2 $02:8002]",
	"                         [disable example: bs #2]",
	"c                      - Dump SNES colour palette",
	"W                      - Show what SNES hardware features the ROM is using",
	"                         which might not be implemented yet",
	"w                      - Show some SNES hardware features used so far in this frame",
	"R                      - Reset SNES",
	"q                      - Quit emulation",
//	"ai                     - Shou APU vectors",
//	"a                      - Show APU status",
//	"x                      - Show Sound DSP status",
//	"A                      - Toggle APU instruction tracing to aputrace.log",
//	"B                      - Toggle sound DSP register tracing to aputrace.log",
//	"C                      - Dump sound sample addresses",
//	"ad [Address]           - Dump APU RAM from PC or [Address]",
	"",
	"[Address]              - $Bank:Address or $Address",
	"                         [for example: $01:8123]",
	"[Number]               - #Number",
	"                         [for example: #1]",
	"z                      - ",
	"f                      - ",
	"dump                   - ",
	"",
	NULL
};

static const char	*S9xMnemonics[256] =
{
	"BRK", "ORA", "COP", "ORA", "TSB", "ORA", "ASL", "ORA",
	"PHP", "ORA", "ASL", "PHD", "TSB", "ORA", "ASL", "ORA",
	"BPL", "ORA", "ORA", "ORA", "TRB", "ORA", "ASL", "ORA",
	"CLC", "ORA", "INC", "TCS", "TRB", "ORA", "ASL", "ORA",
	"JSR", "AND", "JSL", "AND", "BIT", "AND", "ROL", "AND",
	"PLP", "AND", "ROL", "PLD", "BIT", "AND", "ROL", "AND",
	"BMI", "AND", "AND", "AND", "BIT", "AND", "ROL", "AND",
	"SEC", "AND", "DEC", "TSC", "BIT", "AND", "ROL", "AND",
	"RTI", "EOR", "WDM", "EOR", "MVP", "EOR", "LSR", "EOR",
	"PHA", "EOR", "LSR", "PHK", "JMP", "EOR", "LSR", "EOR",
	"BVC", "EOR", "EOR", "EOR", "MVN", "EOR", "LSR", "EOR",
	"CLI", "EOR", "PHY", "TCD", "JMP", "EOR", "LSR", "EOR",
	"RTS", "ADC", "PER", "ADC", "STZ", "ADC", "ROR", "ADC",
	"PLA", "ADC", "ROR", "RTL", "JMP", "ADC", "ROR", "ADC",
	"BVS", "ADC", "ADC", "ADC", "STZ", "ADC", "ROR", "ADC",
	"SEI", "ADC", "PLY", "TDC", "JMP", "ADC", "ROR", "ADC",
	"BRA", "STA", "BRL", "STA", "STY", "STA", "STX", "STA",
	"DEY", "BIT", "TXA", "PHB", "STY", "STA", "STX", "STA",
	"BCC", "STA", "STA", "STA", "STY", "STA", "STX", "STA",
	"TYA", "STA", "TXS", "TXY", "STZ", "STA", "STZ", "STA",
	"LDY", "LDA", "LDX", "LDA", "LDY", "LDA", "LDX", "LDA",
	"TAY", "LDA", "TAX", "PLB", "LDY", "LDA", "LDX", "LDA",
	"BCS", "LDA", "LDA", "LDA", "LDY", "LDA", "LDX", "LDA",
	"CLV", "LDA", "TSX", "TYX", "LDY", "LDA", "LDX", "LDA",
	"CPY", "CMP", "REP", "CMP", "CPY", "CMP", "DEC", "CMP",
	"INY", "CMP", "DEX", "WAI", "CPY", "CMP", "DEC", "CMP",
	"BNE", "CMP", "CMP", "CMP", "PEI", "CMP", "DEC", "CMP",
	"CLD", "CMP", "PHX", "STP", "JML", "CMP", "DEC", "CMP",
	"CPX", "SBC", "SEP", "SBC", "CPX", "SBC", "INC", "SBC",
	"INX", "SBC", "NOP", "XBA", "CPX", "SBC", "INC", "SBC",
	"BEQ", "SBC", "SBC", "SBC", "PEA", "SBC", "INC", "SBC",
	"SED", "SBC", "PLX", "XCE", "JSR", "SBC", "INC", "SBC"
};

static int	AddrModes[256] =
{
  // 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
	 3, 10,  3, 19,  6,  6,  6, 12,  0,  1, 24,  0, 14, 14, 14, 17, // 0
	 4, 11,  9, 20,  6,  7,  7, 13,  0, 16, 24,  0, 14, 15, 15, 18, // 1
	14, 10, 17, 19,  6,  6,  6, 12,  0,  1, 24,  0, 14, 14, 14, 17, // 2
	 4, 11,  9, 20,  7,  7,  7, 13,  0, 16, 24,  0, 15, 15, 15, 18, // 3
	 0, 10,  3, 19, 25,  6,  6, 12,  0,  1, 24,  0, 14, 14, 14, 17, // 4
	 4, 11,  9, 20, 25,  7,  7, 13,  0, 16,  0,  0, 17, 15, 15, 18, // 5
	 0, 10,  5, 19,  6,  6,  6, 12,  0,  1, 24,  0, 21, 14, 14, 17, // 6
	 4, 11,  9, 20,  7,  7,  7, 13,  0, 16,  0,  0, 23, 15, 15, 18, // 7
	 4, 10,  5, 19,  6,  6,  6, 12,  0,  1,  0,  0, 14, 14, 14, 17, // 8
	 4, 11,  9, 20,  7,  7,  8, 13,  0, 16,  0,  0, 14, 15, 15, 18, // 9
	 2, 10,  2, 19,  6,  6,  6, 12,  0,  1,  0,  0, 14, 14, 14, 17, // A
	 4, 11,  9, 20,  7,  7,  8, 13,  0, 16,  0,  0, 15, 15, 16, 18, // B
	 2, 10,  3, 19,  6,  6,  6, 12,  0,  1,  0,  0, 14, 14, 14, 17, // C
	 4, 11,  9,  9, 27,  7,  7, 13,  0, 16,  0,  0, 22, 15, 15, 18, // D
	 2, 10,  3, 19,  6,  6,  6, 12,  0,  1,  0,  0, 14, 14, 14, 17, // E
	 4, 11,  9, 20, 26,  7,  7, 13,  0, 16,  0,  0, 23, 15, 15, 18  // F
};

static uint8 S9xDebugGetByte (uint32);
static uint16 S9xDebugGetWord (uint32);
static uint8 S9xDebugSA1GetByte (uint32);
static uint16 S9xDebugSA1GetWord (uint32);
static uint8 debug_cpu_op_print (char *, uint8, uint16);
static uint8 debug_sa1_op_print (char *, uint8, uint16);
static void debug_line_print (const char *);
static int debug_get_number (char *, uint16 *);
static short debug_get_start_address (char *, uint8 *, uint32 *);
static void debug_process_command (char *);
static void debug_print_window (uint8 *);
static const char * debug_clip_fn (int);
static void debug_whats_used (void);
static void debug_whats_missing (void);


static uint8 S9xDebugGetByte (uint32 Address)
{
	int		block = (Address & 0xffffff) >> MEMMAP_SHIFT;
	uint8	*GetAddress = Memory.Map[block];
	uint8	byte = 0;

	if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
	{
		byte = *(GetAddress + (Address & 0xffff));
		return (byte);
	}

	switch ((pint) GetAddress)
	{
		case CMemory::MAP_LOROM_SRAM:
		case CMemory::MAP_SA1RAM:
			byte = *(Memory.SRAM + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Memory.SRAMMask));
			return (byte);

		case CMemory::MAP_LOROM_SRAM_B:
			byte = *(Multi.sramB + ((((Address & 0xff0000) >> 1) | (Address & 0x7fff)) & Multi.sramMaskB));
			return (byte);

		case CMemory::MAP_HIROM_SRAM:
		case CMemory::MAP_RONLY_SRAM:
			byte = *(Memory.SRAM + (((Address & 0x7fff) - 0x6000 + ((Address & 0xf0000) >> 3)) & Memory.SRAMMask));
			return (byte);

		case CMemory::MAP_BWRAM:
			byte = *(Memory.BWRAM + ((Address & 0x7fff) - 0x6000));
			return (byte);

		default:
			return (byte);
	}
}

static uint16 S9xDebugGetWord (uint32 Address)
{
	uint16	word;

	word  = S9xDebugGetByte(Address);
	word |= S9xDebugGetByte(Address + 1) << 8;

	return (word);
}

static uint8 S9xDebugSA1GetByte (uint32 Address)
{
	int		block = (Address & 0xffffff) >> MEMMAP_SHIFT;
	uint8	*GetAddress = SA1.Map[block];
	uint8	byte = 0;

	if (GetAddress >= (uint8 *) CMemory::MAP_LAST)
	{
		byte = *(GetAddress + (Address & 0xffff));
		return (byte);
	}

	switch ((pint) GetAddress)
	{
		case CMemory::MAP_LOROM_SRAM:
		case CMemory::MAP_SA1RAM:
			byte = *(Memory.SRAM + (Address & 0xffff));
			return (byte);

		case CMemory::MAP_BWRAM:
			byte = *(SA1.BWRAM + ((Address & 0x7fff) - 0x6000));
			return (byte);

		case CMemory::MAP_BWRAM_BITMAP:
			Address -= 0x600000;
			if (SA1.VirtualBitmapFormat == 2)
				byte = (Memory.SRAM[(Address >> 2) & 0xffff] >> ((Address & 3) << 1)) &  3;
			else
				byte = (Memory.SRAM[(Address >> 1) & 0xffff] >> ((Address & 1) << 2)) & 15;
			return (byte);

		case CMemory::MAP_BWRAM_BITMAP2:
			Address = (Address & 0xffff) - 0x6000;
			if (SA1.VirtualBitmapFormat == 2)
				byte = (SA1.BWRAM[(Address >> 2) & 0xffff] >> ((Address & 3) << 1)) &  3;
			else
				byte = (SA1.BWRAM[(Address >> 1) & 0xffff] >> ((Address & 1) << 2)) & 15;
			return (byte);

		default:
			return (byte);
	}
}

static uint16 S9xDebugSA1GetWord (uint32 Address)
{
	uint16	word;

	word  = S9xDebugSA1GetByte(Address);
	word |= S9xDebugSA1GetByte(Address + 1) << 8;

	return (word);
}

static uint8 debug_cpu_op_print (char *Line, uint8 Bank, uint16 Address)
{
	uint8	S9xOpcode;
	uint8	Operant[3];
	uint16	Word;
	uint8	Byte;
	int16	SWord;
	int8	SByte;
	uint8	Size = 0;

	S9xOpcode = S9xDebugGetByte((Bank << 16) + Address);
	sprintf(Line, "$%02X:%04X %02X ", Bank, Address, S9xOpcode);

	Operant[0] = S9xDebugGetByte((Bank << 16) + Address + 1);
	Operant[1] = S9xDebugGetByte((Bank << 16) + Address + 2);
	Operant[2] = S9xDebugGetByte((Bank << 16) + Address + 3);

	switch (AddrModes[S9xOpcode])
	{
		case 0:
			// Implied
			sprintf(Line, "%s         %s",
					Line,
					S9xMnemonics[S9xOpcode]);
			Size = 1;
			break;

		case 1:
			// Immediate[MemoryFlag]
			if (!CheckFlag(MemoryFlag))
			{
				// Accumulator 16 - Bit
				sprintf(Line, "%s%02X %02X    %s #$%02X%02X",
						Line,
						Operant[0],
						Operant[1],
						S9xMnemonics[S9xOpcode],
						Operant[1],
						Operant[0]);
				Size = 3;
			}
			else
			{
				// Accumulator 8 - Bit
				sprintf(Line, "%s%02X       %s #$%02X",
						Line,
						Operant[0],
						S9xMnemonics[S9xOpcode],
						Operant[0]);
				Size = 2;
			}

			break;

		case 2:
			// Immediate[IndexFlag]
			if (!CheckFlag(IndexFlag))
			{
				// X / Y 16 - Bit
				sprintf(Line, "%s%02X %02X    %s #$%02X%02X",
				        Line,
				        Operant[0],
				        Operant[1],
				        S9xMnemonics[S9xOpcode],
				        Operant[1],
				        Operant[0]);
				Size = 3;
			}
			else
			{
				// X / Y 8 - Bit
				sprintf(Line, "%s%02X       %s #$%02X",
				        Line,
				        Operant[0],
				        S9xMnemonics[S9xOpcode],
				        Operant[0]);
				Size = 2;
			}

			break;

		case 3:
			// Immediate[Always 8 - Bit]
			sprintf(Line, "%s%02X       %s #$%02X",
					Line,
					Operant[0],
					S9xMnemonics[S9xOpcode],
					Operant[0]);
			Size = 2;
			break;

		case 4:
			// Relative
			sprintf(Line, "%s%02X       %s $%02X",
					Line,
					Operant[0],
					S9xMnemonics[S9xOpcode],
					Operant[0]);
			SByte = Operant[0];
			Word = Address;
			Word += SByte;
			Word += 2;
			sprintf(Line, "%-32s[$%04X]", Line, Word);
			Size = 2;
			break;

		case 5:
			// Relative Long
			sprintf(Line, "%s%02X %02X    %s $%02X%02X",
					Line,
					Operant[0],
					Operant[1],
					S9xMnemonics[S9xOpcode],
					Operant[1],
					Operant[0]);
			SWord = (Operant[1] << 8) | Operant[0];
			Word = Address;
			Word += SWord;
			Word += 3;
			sprintf(Line, "%-32s[$%04X]", Line, Word);
			Size = 3;
			break;

		case 6:
			// Direct
			sprintf(Line, "%s%02X       %s $%02X",
					Line,
					Operant[0],
					S9xMnemonics[S9xOpcode],
					Operant[0]);
			Word = Operant[0];
			Word += Registers.D.W;
			sprintf(Line, "%-32s[$00:%04X]", Line, Word);
			Size = 2;
			break;

		case 7:
			// Direct Indexed (with X)
			sprintf(Line, "%s%02X       %s $%02X,x",
					Line,
					Operant[0],
					S9xMnemonics[S9xOpcode],
					Operant[0]);
			Word = Operant[0];
			Word += Registers.D.W;
			Word += Registers.X.W;
			sprintf(Line, "%-32s[$00:%04X]", Line, Word);
			Size = 2;
			break;

		case 8:
			// Direct Indexed (with Y)
			sprintf(Line, "%s%02X       %s $%02X,y",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += Registers.D.W;
			Word += Registers.Y.W;
			sprintf(Line, "%-32s[$00:%04X]", Line, Word);
			Size = 2;
			break;

		case 9:
			// Direct Indirect
			sprintf(Line, "%s%02X       %s ($%02X)",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += Registers.D.W;
			Word = S9xDebugGetWord(Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Registers.DB, Word);
			Size = 2;
			break;

		case 10:
			// Direct Indexed Indirect
			sprintf(Line, "%s%02X       %s ($%02X,x)",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += Registers.D.W;
			Word += Registers.X.W;
			Word = S9xDebugGetWord(Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Registers.DB, Word);
			Size = 2;
			break;

		case 11:
			// Direct Indirect Indexed
			sprintf(Line, "%s%02X       %s ($%02X),y",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += Registers.D.W;
			Word = S9xDebugGetWord(Word);
			Word += Registers.Y.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Registers.DB, Word);
			Size = 2;
			break;

		case 12:
			// Direct Indirect Long
			sprintf(Line, "%s%02X       %s [$%02X]",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += Registers.D.W;
			Byte = S9xDebugGetByte(Word + 2);
			Word = S9xDebugGetWord(Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Byte, Word);
			Size = 2;
			break;

		case 13:
			// Direct Indirect Indexed Long
			sprintf(Line, "%s%02X       %s [$%02X],y",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += Registers.D.W;
			Byte = S9xDebugGetByte(Word + 2);
			Word = S9xDebugGetWord(Word);
			Word += Registers.Y.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Byte, Word);
			Size = 2;
			break;

		case 14:
			// Absolute
			sprintf(Line, "%s%02X %02X    %s $%02X%02X",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Registers.DB, Word);
			Size = 3;
			break;

		case 15:
			// Absolute Indexed (with X)
			sprintf(Line, "%s%02X %02X    %s $%02X%02X,x",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Word += Registers.X.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Registers.DB, Word);
			Size = 3;
			break;

		case 16:
			// Absolute Indexed (with Y)
			sprintf(Line, "%s%02X %02X    %s $%02X%02X,y",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Word += Registers.Y.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Registers.DB, Word);
			Size = 3;
			break;

		case 17:
			// Absolute Long
			sprintf(Line, "%s%02X %02X %02X %s $%02X%02X%02X",
			        Line,
			        Operant[0],
			        Operant[1],
			        Operant[2],
			        S9xMnemonics[S9xOpcode],
			        Operant[2],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Operant[2], Word);
			Size = 4;
			break;

		case 18:
			// Absolute Indexed Long
			sprintf(Line, "%s%02X %02X %02X %s $%02X%02X%02X,x",
			        Line,
			        Operant[0],
			        Operant[1],
			        Operant[2],
			        S9xMnemonics[S9xOpcode],
			        Operant[2],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Word += Registers.X.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Operant[2], Word);
			Size = 4;
			break;

		case 19:
			// Stack Relative
			sprintf(Line, "%s%02X       %s $%02X,s",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Registers.S.W;
			Word += Operant[0];
			sprintf(Line, "%-32s[$00:%04X]", Line, Word);
			Size = 2;
			break;

		case 20:
			// Stack Relative Indirect Indexed
			sprintf(Line, "%s%02X       %s ($%02X,s),y",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Registers.S.W;
			Word += Operant[0];
			Word = S9xDebugGetWord(Word);
			Word += Registers.Y.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Registers.DB, Word);
			Size = 2;
			break;

		case 21:
			// Absolute Indirect
			sprintf(Line, "%s%02X %02X    %s ($%02X%02X)",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Word = S9xDebugGetWord(Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Registers.PB, Word);
			Size = 3;
			break;

		case 22:
			// Absolute Indirect Long
			sprintf(Line, "%s%02X %02X    %s [$%02X%02X]",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Byte = S9xDebugGetByte(Word + 2);
			Word = S9xDebugGetWord(Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Byte, Word);
			Size = 3;
			break;

		case 23:
			// Absolute Indexed Indirect
			sprintf(Line, "%s%02X %02X    %s ($%02X%02X,x)",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Word += Registers.X.W;
			Word = S9xDebugGetWord(ICPU.ShiftedPB + Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Registers.PB, Word);
			Size = 3;
			break;

		case 24:
			// Implied Accumulator
			sprintf(Line, "%s         %s A",
			        Line,
			        S9xMnemonics[S9xOpcode]);
			Size = 1;
			break;

		case 25:
			// MVN/MVP SRC DST
			sprintf(Line, "%s%02X %02X    %s %02X %02X",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Size = 3;
			break;

		case 26:
			// PEA
			sprintf(Line, "%s%02X %02X    %s $%02X%02X",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Size = 3;
			break;

		case 27:
			// PEI Direct Indirect
			sprintf(Line, "%s%02X       %s ($%02X)",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += Registers.D.W;
			Word = S9xDebugGetWord(Word);
			sprintf(Line, "%-32s[$%04X]", Line, Word);
			Size = 2;
			break;
	}

	sprintf(Line, "%-44s A:%04X X:%04X Y:%04X D:%04X DB:%02X S:%04X P:%c%c%c%c%c%c%c%c%c HC:%04ld VC:%03ld FC:%02d %03x",
	        Line, Registers.A.W, Registers.X.W, Registers.Y.W,
	        Registers.D.W, Registers.DB, Registers.S.W,
	        CheckEmulation() ? 'E' : 'e',
	        CheckNegative() ? 'N' : 'n',
	        CheckOverflow() ? 'V' : 'v',
	        CheckMemory() ? 'M' : 'm',
	        CheckIndex() ? 'X' : 'x',
	        CheckDecimal() ? 'D' : 'd',
	        CheckIRQ() ? 'I' : 'i',
	        CheckZero() ? 'Z' : 'z',
	        CheckCarry() ? 'C' : 'c',
	        (long) CPU.Cycles,
	        (long) CPU.V_Counter,
	        IPPU.FrameCount,
	        (CPU.IRQExternal ? 0x100 : 0) | (PPU.HTimerEnabled ? 0x10 : 0) | (PPU.VTimerEnabled ? 0x01 : 0));

	return (Size);
}

static uint8 debug_sa1_op_print (char *Line, uint8 Bank, uint16 Address)
{
	uint8	S9xOpcode;
	uint8	Operant[3];
	uint16	Word;
	uint8	Byte;
	int16	SWord;
	int8	SByte;
	uint8	Size = 0;

	S9xOpcode = S9xDebugSA1GetByte((Bank << 16) + Address);
	sprintf(Line, "$%02X:%04X %02X ", Bank, Address, S9xOpcode);

	Operant[0] = S9xDebugSA1GetByte((Bank << 16) + Address + 1);
	Operant[1] = S9xDebugSA1GetByte((Bank << 16) + Address + 2);
	Operant[2] = S9xDebugSA1GetByte((Bank << 16) + Address + 3);

	switch (AddrModes[S9xOpcode])
	{
		case 0:
			// Implied
			sprintf(Line, "%s         %s",
			        Line,
			        S9xMnemonics[S9xOpcode]);
			Size = 1;
			break;

		case 1:
			// Immediate[MemoryFlag]
			if (!SA1CheckFlag(MemoryFlag))
			{
				// Accumulator 16 - Bit
				sprintf(Line, "%s%02X %02X    %s #$%02X%02X",
				        Line,
				        Operant[0],
				        Operant[1],
				        S9xMnemonics[S9xOpcode],
				        Operant[1],
				        Operant[0]);
				Size = 3;
			}
			else
			{
				// Accumulator 8 - Bit
				sprintf(Line, "%s%02X       %s #$%02X",
				        Line,
				        Operant[0],
				        S9xMnemonics[S9xOpcode],
				        Operant[0]);
				Size = 2;
			}

			break;

		case 2:
			// Immediate[IndexFlag]
			if (!SA1CheckFlag(IndexFlag))
			{
				// X / Y 16 - Bit
				sprintf(Line, "%s%02X %02X    %s #$%02X%02X",
				        Line,
				        Operant[0],
				        Operant[1],
				        S9xMnemonics[S9xOpcode],
				        Operant[1],
				        Operant[0]);
				Size = 3;
			}
			else
			{
				// X / Y 8 - Bit
				sprintf(Line, "%s%02X       %s #$%02X",
				        Line,
				        Operant[0],
				        S9xMnemonics[S9xOpcode],
				        Operant[0]);
				Size = 2;
			}

			break;

		case 3:
			// Immediate[Always 8 - Bit]
			sprintf(Line, "%s%02X       %s #$%02X",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Size = 2;
			break;

		case 4:
			// Relative
			sprintf(Line, "%s%02X       %s $%02X",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			SByte = Operant[0];
			Word = Address;
			Word += SByte;
			Word += 2;
			sprintf(Line, "%-32s[$%04X]", Line, Word);
			Size = 2;
			break;

		case 5:
			// Relative Long
			sprintf(Line, "%s%02X %02X    %s $%02X%02X",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			SWord = (Operant[1] << 8) | Operant[0];
			Word = Address;
			Word += SWord;
			Word += 3;
			sprintf(Line, "%-32s[$%04X]", Line, Word);
			Size = 3;
			break;

		case 6:
			// Direct
			sprintf(Line, "%s%02X       %s $%02X",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += SA1Registers.D.W;
			sprintf(Line, "%-32s[$00:%04X]", Line, Word);
			Size = 2;
			break;

		case 7:
			// Direct Indexed (with X)
			sprintf(Line, "%s%02X       %s $%02X,x",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += SA1Registers.D.W;
			Word += SA1Registers.X.W;
			sprintf(Line, "%-32s[$00:%04X]", Line, Word);
			Size = 2;
			break;

		case 8:
			// Direct Indexed (with Y)
			sprintf(Line, "%s%02X       %s $%02X,y",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += SA1Registers.D.W;
			Word += SA1Registers.Y.W;
			sprintf(Line, "%-32s[$00:%04X]", Line, Word);
			Size = 2;
			break;

		case 9:
			// Direct Indirect
			sprintf(Line, "%s%02X       %s ($%02X)",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += SA1Registers.D.W;
			Word = S9xDebugSA1GetWord(Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, SA1Registers.DB, Word);
			Size = 2;
			break;

		case 10:
			// Direct Indexed Indirect
			sprintf(Line, "%s%02X       %s ($%02X,x)",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += SA1Registers.D.W;
			Word += SA1Registers.X.W;
			Word = S9xDebugSA1GetWord(Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, SA1Registers.DB, Word);
			Size = 2;
			break;

		case 11:
			// Direct Indirect Indexed
			sprintf(Line, "%s%02X       %s ($%02X),y",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += SA1Registers.D.W;
			Word = S9xDebugSA1GetWord(Word);
			Word += SA1Registers.Y.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, SA1Registers.DB, Word);
			Size = 2;
			break;

		case 12:
			// Direct Indirect Long
			sprintf(Line, "%s%02X       %s [$%02X]",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += SA1Registers.D.W;
			Byte = S9xDebugSA1GetByte(Word + 2);
			Word = S9xDebugSA1GetWord(Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Byte, Word);
			Size = 2;
			break;

		case 13:
			// Direct Indirect Indexed Long
			sprintf(Line, "%s%02X       %s [$%02X],y",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = Operant[0];
			Word += SA1Registers.D.W;
			Byte = S9xDebugSA1GetByte(Word + 2);
			Word = S9xDebugSA1GetWord(Word);
			Word += SA1Registers.Y.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Byte, Word);
			Size = 2;
			break;

		case 14:
			// Absolute
			sprintf(Line, "%s%02X %02X    %s $%02X%02X",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			sprintf(Line, "%-32s[$%02X:%04X]", Line, SA1Registers.DB, Word);
			Size = 3;
			break;

		case 15:
			// Absolute Indexed (with X)
			sprintf(Line, "%s%02X %02X    %s $%02X%02X,x",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Word += SA1Registers.X.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, SA1Registers.DB, Word);
			Size = 3;
			break;

		case 16:
			// Absolute Indexed (with Y)
			sprintf(Line, "%s%02X %02X    %s $%02X%02X,y",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Word += SA1Registers.Y.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, SA1Registers.DB, Word);
			Size = 3;
			break;

		case 17:
			// Absolute Long
			sprintf(Line, "%s%02X %02X %02X %s $%02X%02X%02X",
			        Line,
			        Operant[0],
			        Operant[1],
			        Operant[2],
			        S9xMnemonics[S9xOpcode],
			        Operant[2],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Operant[2], Word);
			Size = 4;
			break;

		case 18:
			// Absolute Indexed Long
			sprintf(Line, "%s%02X %02X %02X %s $%02X%02X%02X,x",
			        Line,
			        Operant[0],
			        Operant[1],
			        Operant[2],
			        S9xMnemonics[S9xOpcode],
			        Operant[2],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Word += SA1Registers.X.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Operant[2], Word);
			Size = 4;
			break;

		case 19:
			// Stack Relative
			sprintf(Line, "%s%02X       %s $%02X,s",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = SA1Registers.S.W;
			Word += Operant[0];
			sprintf(Line, "%-32s[$00:%04X]", Line, Word);
			Size = 2;
			break;

		case 20:
			// Stack Relative Indirect Indexed
			sprintf(Line, "%s%02X       %s ($%02X,s),y",
			        Line,
			        Operant[0],
			        S9xMnemonics[S9xOpcode],
			        Operant[0]);
			Word = SA1Registers.S.W;
			Word += Operant[0];
			Word = S9xDebugSA1GetWord(Word);
			Word += SA1Registers.Y.W;
			sprintf(Line, "%-32s[$%02X:%04X]", Line, SA1Registers.DB, Word);
			Size = 2;
			break;

		case 21:
			// Absolute Indirect
			sprintf(Line, "%s%02X %02X    %s ($%02X%02X)",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Word = S9xDebugSA1GetWord(Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, SA1Registers.PB, Word);
			Size = 3;
			break;

		case 22:
			// Absolute Indirect Long
			sprintf(Line, "%s%02X %02X    %s [$%02X%02X]",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Byte = S9xDebugSA1GetByte(Word + 2);
			Word = S9xDebugSA1GetWord(Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, Byte, Word);
			Size = 3;
			break;

		case 23:
			// Absolute Indexed Indirect
			sprintf(Line, "%s%02X %02X    %s ($%02X%02X,x)",
			        Line,
			        Operant[0],
			        Operant[1],
			        S9xMnemonics[S9xOpcode],
			        Operant[1],
			        Operant[0]);
			Word = (Operant[1] << 8) | Operant[0];
			Word += SA1Registers.X.W;
			Word = S9xDebugSA1GetWord(SA1.ShiftedPB + Word);
			sprintf(Line, "%-32s[$%02X:%04X]", Line, SA1Registers.PB, Word);
			Size = 3;
			break;

		case 24:
			// Implied Accumulator
			sprintf(Line, "%s         %s A",
			        Line,
			        S9xMnemonics[S9xOpcode]);
			Size = 1;
			break;

		case 25:
			// MVN/MVP SRC DST
			sprintf(Line, "%s         %s %02X %02X",
			        Line,
			        S9xMnemonics[S9xOpcode],
			        Operant[0],
			        Operant[1]);
			Size = 3;
			break;
	}

	sprintf(Line, "%-44s A:%04X X:%04X Y:%04X D:%04X DB:%02X S:%04X P:%c%c%c%c%c%c%c%c%c HC:%04ld VC:%03ld FC:%02d",
	        Line, SA1Registers.A.W, SA1Registers.X.W, SA1Registers.Y.W,
	        SA1Registers.D.W, SA1Registers.DB, SA1Registers.S.W,
	        SA1CheckEmulation() ? 'E' : 'e',
	        SA1CheckNegative() ? 'N' : 'n',
	        SA1CheckOverflow() ? 'V' : 'v',
	        SA1CheckMemory() ? 'M' : 'm',
	        SA1CheckIndex() ? 'X' : 'x',
	        SA1CheckDecimal() ? 'D' : 'd',
	        SA1CheckIRQ() ? 'I' : 'i',
	        SA1CheckZero() ? 'Z' : 'z',
	        SA1CheckCarry() ? 'C' : 'c',
	        (long) CPU.Cycles,
	        (long) CPU.V_Counter,
	        IPPU.FrameCount);

	return (Size);
}

static void debug_line_print (const char *Line)
{
	printf("%s\n", Line);
}

static int debug_get_number (char *Line, uint16 *Number)
{
	int	i;

	if (sscanf(Line, " #%d", &i) == 1)
	{
		*Number = i;
		return (1);
	}

	return (-1);
}

static short debug_get_start_address (char *Line, uint8 *Bank, uint32 *Address)
{
	uint32	a, b;

	if (sscanf(Line + 1, " $%x:%x", &b, &a) != 2)
		return (-1);

	*Bank = b;
	*Address = a;

	return (1);
}

static void debug_process_command (char *Line)
{
	uint8	Bank = Registers.PB;
	uint32	Address = Registers.PCw;
	uint16	Hold = 0;
	uint16	Number;
	short	ErrorCode;
	char	string[512];

	if (strncasecmp(Line, "dump", 4) == 0)
	{
		int	Count;

		if (sscanf(&Line[4], "%x %d", &Address, &Count) == 2)
		{
			FILE	*fs;

			sprintf(string, "%06x%05d.sd2", Address, Count);
			fs = fopen(string, "wb");
			if (fs)
			{
				for (int i = 0; i < Count; i++)
					putc(S9xDebugGetByte(Address + i), fs);
				fclose(fs);
			}
			else
				printf("Can't open %s for writing\n", string);
		}
		else
			printf("Usage: dump start_address_in_hex count_in_decimal\n");

		return;
	}

	if (*Line == 'i')
	{
		printf("Vectors:\n");
		sprintf(string, "      8 Bit   16 Bit ");
		debug_line_print(string);
		sprintf(string, "ABT $00:%04X|$00:%04X", S9xDebugGetWord(0xFFF8), S9xDebugGetWord(0xFFE8));
		debug_line_print(string);
		sprintf(string, "BRK $00:%04X|$00:%04X", S9xDebugGetWord(0xFFFE), S9xDebugGetWord(0xFFE6));
		debug_line_print(string);
		sprintf(string, "COP $00:%04X|$00:%04X", S9xDebugGetWord(0xFFF4), S9xDebugGetWord(0xFFE4));
		debug_line_print(string);
		sprintf(string, "IRQ $00:%04X|$00:%04X", S9xDebugGetWord(0xFFFE), S9xDebugGetWord(0xFFEE));
		debug_line_print(string);
		sprintf(string, "NMI $00:%04X|$00:%04X", S9xDebugGetWord(0xFFFA), S9xDebugGetWord(0xFFEA));
		debug_line_print(string);
		sprintf(string, "RES     $00:%04X", S9xDebugGetWord(0xFFFC));
		debug_line_print(string);
	}

/*
	if (strncmp(Line, "ai", 2) == 0)
	{
		printf("APU vectors:");

		for (int i = 0; i < 0x40; i += 2)
		{
			if (i % 16 == 0)
				printf("\n%04x ", 0xffc0 + i);

			printf("%04x ", APU.ExtraRAM[i]);
		}

		printf("\n");
	}
*/

	if (*Line == 's')
	{
		Registers.PCw += debug_cpu_op_print(string, Bank, Address);
		Bank = Registers.PB;
		Address = Registers.PCw;
		*Line = 'r';
	}

	if (*Line == 'z')
	{
		uint16	*p = (uint16 *) &Memory.VRAM[PPU.BG[2].SCBase << 1];

		for (int l = 0; l < 32; l++)
		{
			for (int c = 0; c < 32; c++, p++)
				printf("%04x,", *p++);

			printf("\n");
		}
	}

	if (*Line == 'c')
	{
		printf("Colours:\n");

		for (int i = 0; i < 256; i++)
			printf("%02x%02x%02x  ", PPU.CGDATA[i] & 0x1f, (PPU.CGDATA[i] >> 5) & 0x1f, (PPU.CGDATA[i] >> 10) & 0x1f);

		printf("\n");
	}

	if (*Line == 'S')
	{
		int	SmallWidth, LargeWidth, SmallHeight, LargeHeight;

		switch ((Memory.FillRAM[0x2101] >> 5) & 7)
		{

			case 0:
				SmallWidth = SmallHeight = 8;
				LargeWidth = LargeHeight = 16;
				break;

			case 1:
				SmallWidth = SmallHeight = 8;
				LargeWidth = LargeHeight = 32;
				break;

			case 2:
				SmallWidth = SmallHeight = 8;
				LargeWidth = LargeHeight = 64;
				break;

			case 3:
				SmallWidth = SmallHeight = 16;
				LargeWidth = LargeHeight = 32;
				break;

			case 4:
				SmallWidth = SmallHeight = 16;
				LargeWidth = LargeHeight = 64;
				break;

			default:
			case 5:
				SmallWidth = SmallHeight = 32;
				LargeWidth = LargeHeight = 64;
				break;

			case 6:
				SmallWidth = 16;
				SmallHeight = 32;
				LargeWidth = 32;
				LargeHeight = 64;
				break;

			case 7:
				SmallWidth = 16;
				SmallHeight = 32;
				LargeWidth = LargeHeight = 32;
				break;
		}

		printf("Sprites: Small: %dx%d, Large: %dx%d, OAMAddr: 0x%04x, OBJNameBase: 0x%04x, OBJNameSelect: 0x%04x, First: %d\n",
		       SmallWidth, SmallHeight, LargeWidth, LargeHeight, PPU.OAMAddr, PPU.OBJNameBase, PPU.OBJNameSelect, PPU.FirstSprite);

		for (int i = 0; i < 128; i++)
		{
			printf("X:%3d Y:%3d %c%c%d%c ",
			       PPU.OBJ[i].HPos,
			       PPU.OBJ[i].VPos,
			       PPU.OBJ[i].VFlip ? 'V' : 'v',
			       PPU.OBJ[i].HFlip ? 'H' : 'h',
			       PPU.OBJ[i].Priority,
			       PPU.OBJ[i].Size ? 'S' : 's');

			if (i % 4 == 3)
				printf("\n");
		}
	}

	if (*Line == 'T')
	{
		if (Line[1] == 'S')
		{
			SA1.Flags ^= TRACE_FLAG;

			if (SA1.Flags & TRACE_FLAG)
			{
				printf("SA1 CPU instruction tracing enabled.\n");
				ENSURE_TRACE_OPEN(trace2, "trace_sa1.log", "wb")
			}
			else
			{
				printf("SA1 CPU instruction tracing disabled.\n");
				fclose(trace2);
				trace2 = NULL;
			}
		}
		else
		{
			CPU.Flags ^= TRACE_FLAG;

			if (CPU.Flags & TRACE_FLAG)
			{
				printf("CPU instruction tracing enabled.\n");
				ENSURE_TRACE_OPEN(trace, "trace.log", "wb")
			}
			else
			{
				printf("CPU instruction tracing disabled.\n");
				fclose(trace);
				trace = NULL;
			}
		}
	}

	if (*Line == 'E')
	{
		Settings.TraceHCEvent = !Settings.TraceHCEvent;
		printf("HC event tracing %s.\n", Settings.TraceHCEvent ? "enabled" : "disabled");
	}

	if (*Line == 'A')
		spc_core->debug_toggle_trace();

/*
	if (*Line == 'B')
	{
		Settings.TraceSoundDSP = !Settings.TraceSoundDSP;
		printf("Sound DSP register tracing %s.\n", Settings.TraceSoundDSP ? "enabled" : "disabled");
	}

	if (*Line == 'x')
		S9xPrintSoundDSPState();

	if (*Line == 'C')
	{
		printf("SPC700 sample addresses at 0x%04x:\n", APU.DSP[APU_DIR] << 8);

		for (int i = 0; i < 256; i++)
		{
			uint8	*dir = IAPU.RAM + (((APU.DSP[APU_DIR] << 8) + i * 4) & 0xffff);
			int		addr = *dir + (*(dir + 1) << 8);
			int		addr2 = *(dir + 2) + (*(dir + 3) << 8);
			printf("%04X %04X;", addr, addr2);

			if (i % 8 == 7)
				printf("\n");
		}
	}
*/

	if (*Line == 'R')
	{
		S9xReset();
		printf("SNES reset.\n");
		CPU.Flags |= DEBUG_MODE_FLAG;
	}

/*
	if (strncmp(Line, "ad", 2) == 0)
	{
		uint32	Count = 16;
		Address = 0;

		if (sscanf(Line + 2, "%x,%x", &Address, &Count) != 2)
		{
			if (sscanf(Line + 2, "%x", &Address) == 1)
				Count = 16;
		}

		printf("APU RAM dump:\n");

		for (uint32 l = 0; l < Count; l += 16)
		{
			printf("%04X ", Address);

			for (int i = 0; i < 16; i++)
				printf("%02X ", IAPU.RAM[Address++]);

			printf("\n");
		}

		*Line = 0;
	}

	if (*Line == 'a')
	{
		printf("APU in-ports : %02X %02X %02X %02X\n", IAPU.RAM[0xF4], IAPU.RAM[0xF5], IAPU.RAM[0xF6], IAPU.RAM[0xF7]);
		printf("APU out-ports: %02X %02X %02X %02X\n", APU.OutPorts[0], APU.OutPorts[1], APU.OutPorts[2], APU.OutPorts[3]);
		printf("ROM/RAM switch: %s\n", (IAPU.RAM[0xf1] & 0x80) ? "ROM" : "RAM");

		for (int i = 0; i < 3; i++)
			if (APU.TimerEnabled[i])
				printf("Timer%d enabled, Value: 0x%03X, 4-bit: 0x%02X, Target: 0x%03X\n",
				       i, APU.Timer[i], IAPU.RAM[0xfd + i], APU.TimerTarget[i]);
	}

	if (*Line == 'P')
	{
		Settings.TraceDSP = !Settings.TraceDSP;
		printf("DSP tracing %s.\n", Settings.TraceDSP ? "enabled" : "disabled");
	}
*/

	if (*Line == 'p')
	{
		S9xBreakpoint[5].Enabled = FALSE;
		Address += debug_cpu_op_print(string, Bank, Address);

		if (strncmp(&string[18], "JMP", 3) != 0 &&
		    strncmp(&string[18], "JML", 3) != 0 &&
		    strncmp(&string[18], "RT" , 2) != 0 &&
		    strncmp(&string[18], "BRA", 3))
		{
			S9xBreakpoint[5].Enabled = TRUE;
			S9xBreakpoint[5].Bank = Bank;
			S9xBreakpoint[5].Address = Address;
		}
		else
		{
			CPU.Flags |= SINGLE_STEP_FLAG;
			CPU.Flags &= ~DEBUG_MODE_FLAG;
		}
	}

	if (*Line == 'b')
	{
		if (Line[1] == 's')
		{
			debug_get_number(Line + 2, &Hold);

			if (Hold > 4)
				Hold = 0;

			if (Hold < 5)
			{
				if (debug_get_start_address(Line + 5, &Bank, &Address) == -1)
					S9xBreakpoint[Hold].Enabled = FALSE;
				else
				{
					S9xBreakpoint[Hold].Enabled = TRUE;
					S9xBreakpoint[Hold].Bank = Bank;
					S9xBreakpoint[Hold].Address = Address;
					CPU.Flags |= BREAK_FLAG;
				}
			}

			Line[1] = 'v';
		}

		if (Line[1] == 'v')
		{
			Number = 0;

			if (debug_get_number(Line + 2, &Number) == -1 && Number < 5)
			{
				debug_line_print("Breakpoints:");

				for (Number = 0; Number != 5; Number++)
				{
					if (S9xBreakpoint[Number].Enabled)
						sprintf(string, "%i @ $%02X:%04X", Number, S9xBreakpoint[Number].Bank, S9xBreakpoint[Number].Address);
					else
						sprintf(string, "%i @ Disabled", Number);

					debug_line_print(string);
				}
			}
			else
			{
				debug_line_print("Breakpoint:");

				if (S9xBreakpoint[Number].Enabled)
					sprintf(string, "%i @ $%02X:%04X", Number, S9xBreakpoint[Number].Bank, S9xBreakpoint[Number].Address);
				else
					sprintf(string, "%i @ Disabled", Number);

				debug_line_print(string);
			}
		}
	}

	if (*Line == '?' || strcasecmp(Line, "help") == 0)
	{
		for (int i = 0; HelpMessage[i] != NULL; i++)
			debug_line_print(HelpMessage[i]);
	}

	if (*Line == 't')
	{
		CPU.Flags |= SINGLE_STEP_FLAG;
		CPU.Flags &= ~DEBUG_MODE_FLAG;
	}

	if (*Line == 'f')
	{
		CPU.Flags |= FRAME_ADVANCE_FLAG;
		CPU.Flags &= ~DEBUG_MODE_FLAG;

		IPPU.RenderThisFrame = TRUE;
		IPPU.FrameSkip = 0;

		if (sscanf(&Line[1], "%u", &ICPU.FrameAdvanceCount) != 1)
			ICPU.Frame = 0;
	}

	if (*Line == 'g')
	{
		S9xBreakpoint[5].Enabled = FALSE;

		bool8	found = FALSE;

		for (int i = 0; i < 5; i++)
		{
			if (S9xBreakpoint[i].Enabled)
			{
				found = TRUE;

				if (S9xBreakpoint[i].Bank == Registers.PB && S9xBreakpoint[i].Address == Registers.PCw)
				{
					S9xBreakpoint[i].Enabled = 2;
					break;
				}
			}
		}

		if (!found)
			CPU.Flags &= ~BREAK_FLAG;

		ErrorCode = debug_get_start_address(Line, &Bank, &Address);

		if (ErrorCode == 1)
		{
			S9xBreakpoint[5].Enabled = TRUE;
			S9xBreakpoint[5].Bank = Bank;
			S9xBreakpoint[5].Address = Address;
			CPU.Flags |= BREAK_FLAG;
		}

		CPU.Flags &= ~DEBUG_MODE_FLAG;
	}

	if (*Line == 'D')
	{
		Settings.TraceDMA = !Settings.TraceDMA;
		printf("DMA tracing %s.\n", Settings.TraceDMA ? "enabled" : "disabled");
	}

	if (*Line == 'V')
	{
		Settings.TraceVRAM = !Settings.TraceVRAM;
		printf("Non-DMA VRAM write tracing %s.\n", Settings.TraceVRAM ? "enabled" : "disabled");
	}

	if (*Line == 'H')
	{
		Settings.TraceHDMA = !Settings.TraceHDMA;
		printf("HDMA tracing %s.\n", Settings.TraceHDMA ? "enabled" : "disabled");
	}

	if (*Line == 'U')
	{
		Settings.TraceUnknownRegisters = !Settings.TraceUnknownRegisters;
		printf("Unknown registers read/write tracing %s.\n", Settings.TraceUnknownRegisters ? "enabled" : "disabled");
	}

	if (*Line == 'd')
	{
		int		CLine;
		int		CByte;
		uint8	MemoryByte;

		if (Debug.Dump.Bank != 0 || Debug.Dump.Address != 0)
		{
			Bank = Debug.Dump.Bank;
			Address = Debug.Dump.Address;
		}

		ErrorCode = debug_get_start_address(Line, &Bank, &Address);

		for (CLine = 0; CLine != 10; CLine++)
		{
			sprintf(string, "$%02X:%04X", Bank, Address);

			for (CByte = 0; CByte != 16; CByte++)
			{
				if (Address + CByte == 0x2140 ||
				    Address + CByte == 0x2141 ||
				    Address + CByte == 0x2142 ||
				    Address + CByte == 0x2143 ||
				    Address + CByte == 0x4210)
					MemoryByte = 0;
				else
					MemoryByte = S9xDebugGetByte((Bank << 16) + Address + CByte);

				sprintf(string, "%s %02X", string, MemoryByte);
			}

			sprintf(string, "%s-", string);

			for (CByte = 0; CByte != 16; CByte++)
			{
				if (Address + CByte == 0x2140 ||
				    Address + CByte == 0x2141 ||
				    Address + CByte == 0x2142 ||
				    Address + CByte == 0x2143 ||
				    Address + CByte == 0x4210)
					MemoryByte = 0;
				else
					MemoryByte = S9xDebugGetByte((Bank << 16) + Address + CByte);

				if (MemoryByte < 32 || MemoryByte >= 127)
					MemoryByte = '?';

				sprintf(string, "%s%c", string, MemoryByte);
			}

			Address += 16;

			debug_line_print(string);
		}

		Debug.Dump.Bank = Bank;
		Debug.Dump.Address = Address;
	}

	if (*Line == 'q')
		S9xExit();

	if (*Line == 'W')
		debug_whats_missing();

	if (*Line == 'w')
		debug_whats_used();

	if (*Line == 'r')
	{
		debug_cpu_op_print(string, Bank, Address);
		debug_line_print(string);
	}

	if (*Line == 'u')
	{
		if (Debug.Unassemble.Bank != 0 || Debug.Unassemble.Address != 0)
		{
			Bank = Debug.Unassemble.Bank;
			Address = Debug.Unassemble.Address;
		}

		ErrorCode = debug_get_start_address(Line, &Bank, &Address);

		for (int i = 0; i != 10; i++)
		{
			Address += debug_cpu_op_print(string, Bank, Address);
			debug_line_print(string);
		}

		Debug.Unassemble.Bank = Bank;
		Debug.Unassemble.Address = Address;
	}

	debug_line_print("");

	return;
}

static void debug_print_window (uint8 *window)
{
	for (int i = 0; i < 6; i++)
	{
		if (window[i])
		{
			switch (i)
			{
				case 0:
					printf("Background 0, ");
					break;

				case 1:
					printf("Background 1, ");
					break;

				case 2:
					printf("Background 2, ");
					break;

				case 3:
					printf("Background 3, ");
					break;

				case 4:
					printf("Objects, ");
					break;

				case 5:
					printf("Color window, ");
					break;
			}
		}
	}
}

static const char * debug_clip_fn (int logic)
{
	switch (logic)
	{
		case CLIP_OR:
			return ("OR");

		case CLIP_AND:
			return ("AND");

		case CLIP_XOR:
			return ("XOR");

		case CLIP_XNOR:
			return ("XNOR");

		default:
			return ("???");
	}
}

static void debug_whats_used (void)
{
	printf("V-line: %ld, H-Pos: %ld, \n", (long) CPU.V_Counter, (long) CPU.Cycles);

	printf("Screen mode: %d, ", PPU.BGMode);

	if (PPU.BGMode <= 1 && (Memory.FillRAM[0x2105] & 8))
		printf("(BG#2 Priority), ");

	printf("Brightness: %d, ", PPU.Brightness);

	if (Memory.FillRAM[0x2100] & 0x80)
		printf("(screen blanked), ");

	printf("\n");

	if (Memory.FillRAM[0x2133] & 1)
		printf("Interlace, ");

	if (Memory.FillRAM[0x2133] & 4)
		printf("240 line visible, ");

	if (Memory.FillRAM[0x2133] & 8)
		printf("Pseudo 512 pixels horizontal resolution, ");

	if (Memory.FillRAM[0x2133] & 0x40)
		printf("Mode 7 priority per pixel, ");

	printf("\n");

	if (PPU.BGMode == 7 && (Memory.FillRAM[0x211a] & 3))
		printf("Mode 7 flipping, ");

	if (PPU.BGMode == 7)
		printf("Mode 7 screen repeat: %d, ", (Memory.FillRAM[0x211a] & 0xc0) >> 6);

	if (Memory.FillRAM[0x2130] & 1)
		printf("32K colour mode, ");

	printf("\n");

	if (PPU.BGMode == 7)
	{
		// Sign extend 13 bit values to 16 bit values...
		if (PPU.CentreX & (1 << 12))
			PPU.CentreX |= 0xe000;

		if (PPU.CentreY & (1 << 12))
			PPU.CentreY |= 0xe000;

		printf("Matrix A: %.3f, B: %.3f, C: %.3f, D: %.3f, Centre X: %d Y:%d, \n",
		       (double) PPU.MatrixA / 256, (double) PPU.MatrixB / 256,
		       (double) PPU.MatrixC / 256, (double) PPU.MatrixD / 256,
		       PPU.CentreX, PPU.CentreY);
	}

	if ((Memory.FillRAM[0x2106] & 0xf0) && (Memory.FillRAM[0x2106] & 0x0f))
	{
		printf("Mosaic effect(%d) on, ", PPU.Mosaic);

		for (int i = 0; i < 4; i++)
			if (Memory.FillRAM[0x2106] & (1 << i))
				printf("BG%d, ", i);
	}

	printf("\n");

	if (PPU.HVBeamCounterLatched)
		printf("V and H beam pos latched, \n");

	if (Memory.FillRAM[0x4200] & 0x20)
		printf("V-IRQ enabled at %d, \n", PPU.IRQVBeamPos);

	if (Memory.FillRAM[0x4200] & 0x10)
		printf("H-IRQ enabled at %d, \n", PPU.IRQHBeamPos);

	if (Memory.FillRAM[0x4200] & 0x80)
		printf("V-blank NMI enabled, \n");

	for (int i = 0; i < 8; i++)
	{
		if (missing.hdma_this_frame & (1 << i))
		{
			printf("H-DMA %d [%d] 0x%02X%04X->0x21%02X %s %s 0x%02X%04X %s addressing, \n",
			       i, DMA[i].TransferMode, DMA[i].ABank, DMA[i].AAddress, DMA[i].BAddress,
			       DMA[i].AAddressDecrement ? "dec" : "inc",
			       DMA[i].Repeat ? "repeat" : "continue",
			       DMA[i].IndirectBank, DMA[i].IndirectAddress,
			       DMA[i].HDMAIndirectAddressing ? "indirect" : "absolute");
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (missing.dma_this_frame & (1 << i))
		{
			printf("DMA %d [%d] 0x%02X%04X->0x21%02X Num: %d %s, \n",
			       i, DMA[i].TransferMode, DMA[i].ABank, DMA[i].AAddress, DMA[i].BAddress, DMA[i].TransferBytes,
			       DMA[i].AAddressFixed ? "fixed" : (DMA[i].AAddressDecrement ? "dec" : "inc"));
		}
	}

	printf("VRAM write address: 0x%04x(%s), Full Graphic: %d, Address inc: %d, \n",
	       PPU.VMA.Address,
	       PPU.VMA.High  ? "Byte" : "Word",
	       PPU.VMA.FullGraphicCount, PPU.VMA.Increment);

	for (int i = 0; i < 4; i++)
	{
		printf("BG%d: VOffset:%d, HOffset:%d, W:%d, H:%d, TS:%d, BA:0x%04x, TA:0x%04X, \n",
		       i, PPU.BG[i].VOffset, PPU.BG[i].HOffset,
		       (PPU.BG[i].SCSize & 1) * 32 + 32,
		       (PPU.BG[i].SCSize & 2) * 16 + 32,
		       PPU.BG[i].BGSize * 8 + 8,
		       PPU.BG[i].SCBase,
		       PPU.BG[i].NameBase);
	}

	const char	*s = "";

	switch ((Memory.FillRAM[0x2130] & 0xc0) >> 6)
	{
		case 0:
			s = "always on";
			break;

		case 1:
			s = "inside";
			break;

		case 2:
			s = "outside";
			break;

		case 3:
			s = "always off";
			break;
	}

	printf("Main screen (%s): ", s);

	for (int i = 0; i < 5; i++)
	{
		if (Memory.FillRAM[0x212c] & (1 << i))
		{
			switch (i)
			{
				case 0:
					printf("BG0, ");
					break;

				case 1:
					printf("BG1, ");
					break;

				case 2:
					printf("BG2, ");
					break;

				case 3:
					printf("BG3, ");
					break;

				case 4:
					printf("OBJ, ");
					break;
			}
		}
	}

	printf("\n");

	switch ((Memory.FillRAM[0x2130] & 0x30) >> 4)
	{
		case 0:
			s = "always on";
			break;

		case 1:
			s = "inside";
			break;

		case 2:
			s = "outside";
			break;

		case 3:
			s = "always off";
			break;
	}

	printf("Subscreen (%s): ", s);

	for (int i = 0; i < 5; i++)
	{
		if (Memory.FillRAM[0x212d] & (1 << i))
		{
			switch (i)
			{
				case 0:
					printf("BG0, ");
					break;

				case 1:
					printf("BG1, ");
					break;

				case 2:
					printf("BG2, ");
					break;

				case 3:
					printf("BG3, ");
					break;

				case 4:
					printf("OBJ, ");
					break;
			}
		}
	}

	printf("\n");

	if ((Memory.FillRAM[0x2131] & 0x3f))
	{
		if (Memory.FillRAM[0x2131] & 0x80)
		{
			if (Memory.FillRAM[0x2130] & 0x02)
				printf("Subscreen subtract");
			else
				printf("Fixed colour subtract");
		}
		else
		{
			if (Memory.FillRAM[0x2130] & 0x02)
				printf("Subscreen addition");
			else
				printf("Fixed colour addition");
		}

		if (Memory.FillRAM[0x2131] & 0x40)
			printf("(half): ");
		else
			printf(": ");

		for (int i = 0; i < 6; i++)
		{
			if (Memory.FillRAM[0x2131] & (1 << i))
			{
				switch (i)
				{
					case 0:
						printf("BG0, ");
						break;

					case 1:
						printf("BG1, ");
						break;

					case 2:
						printf("BG2, ");
						break;

					case 3:
						printf("BG3, ");
						break;

					case 4:
						printf("OBJ, ");
						break;

					case 5:
						printf("BACK, ");
						break;
				}
			}
		}

		printf("\n");
	}

	printf("Window 1 (%d, %d, %02x, %02x): ", PPU.Window1Left, PPU.Window1Right, Memory.FillRAM[0x212e], Memory.FillRAM[0x212f]);

	for (int i = 0; i < 6; i++)
	{
		if (PPU.ClipWindow1Enable[i])
		{
			switch (i)
			{
				case 0:
					printf("BG0(%s-%s), ", PPU.ClipWindow1Inside[0] ? "I" : "O", debug_clip_fn(PPU.ClipWindowOverlapLogic[0]));
					break;

				case 1:
					printf("BG1(%s-%s), ", PPU.ClipWindow1Inside[1] ? "I" : "O", debug_clip_fn(PPU.ClipWindowOverlapLogic[1]));
					break;

				case 2:
					printf("BG2(%s-%s), ", PPU.ClipWindow1Inside[2] ? "I" : "O", debug_clip_fn(PPU.ClipWindowOverlapLogic[2]));
					break;

				case 3:
					printf("BG3(%s-%s), ", PPU.ClipWindow1Inside[3] ? "I" : "O", debug_clip_fn(PPU.ClipWindowOverlapLogic[3]));
					break;

				case 4:
					printf("OBJ(%s-%s), ", PPU.ClipWindow1Inside[4] ? "I" : "O", debug_clip_fn(PPU.ClipWindowOverlapLogic[4]));
					break;

				case 5:
					printf("COL(%s-%s), ", PPU.ClipWindow1Inside[5] ? "I" : "O", debug_clip_fn(PPU.ClipWindowOverlapLogic[5]));
					break;
			}
		}
	}

	printf("\n");

	printf("Window 2 (%d, %d): ", PPU.Window2Left, PPU.Window2Right);

	for (int i = 0; i < 6; i++)
	{
		if (PPU.ClipWindow2Enable[i])
		{
			switch (i)
			{
				case 0:
					printf("BG0(%s), ", PPU.ClipWindow2Inside[0] ? "I" : "O");
					break;

				case 1:
					printf("BG1(%s), ", PPU.ClipWindow2Inside[1] ? "I" : "O");
					break;

				case 2:
					printf("BG2(%s), ", PPU.ClipWindow2Inside[2] ? "I" : "O");
					break;

				case 3:
					printf("BG3(%s), ", PPU.ClipWindow2Inside[3] ? "I" : "O");
					break;

				case 4:
					printf("OBJ(%s), ", PPU.ClipWindow2Inside[4] ? "I" : "O");
					break;

				case 5:
					printf("COL(%s), " , PPU.ClipWindow2Inside[5] ? "I" : "O");
					break;
			}
		}
	}

	printf("\n");

	printf("Fixed colour: %02x%02x%02x, \n", PPU.FixedColourRed, PPU.FixedColourGreen, PPU.FixedColourBlue);
}

static void debug_whats_missing (void)
{
	printf("Processor: ");

	if (missing.emulate6502)
		printf("emulation mode, ");

	if (missing.decimal_mode)
		printf("decimal mode, ");

	if (missing.mv_8bit_index)
		printf("MVP/MVN with 8bit index registers and XH or YH > 0, ");

	if (missing.mv_8bit_acc)
		printf("MVP/MVN with 8bit accumulator > 255, ");

	printf("\n");

	printf("Screen modes used: ");

	for (int i = 0; i < 8; i++)
		if (missing.modes[i])
			printf("%d, ", i);

	printf("\n");

	if (missing.interlace)
		printf("Interlace, ");

	if (missing.pseudo_512)
		printf("Pseudo 512 pixels horizontal resolution, ");

	if (missing.lines_239)
		printf("240 lines visible, ");

	if (missing.sprite_double_height)
		printf("double-hight sprites, ");

	printf("\n");

	if (missing.mode7_fx)
		printf("Mode 7 rotation/scaling, ");

	if (missing.matrix_read)
		printf("Mode 7 read matrix registers, ");

	if (missing.mode7_flip)
		printf("Mode 7 flipping, ");

	if (missing.mode7_bgmode)
		printf("Mode 7 priority per pixel, ");

	if (missing.direct)
		printf("Direct 32000 colour mode, ");

	printf("\n");

	if (missing.mosaic)
		printf("Mosaic effect, ");

	if (missing.subscreen)
		printf("Subscreen enabled, ");

	if (missing.subscreen_add)
		printf("Subscreen colour add, ");

	if (missing.subscreen_sub)
		printf("Subscreen colour subtract, ");

	if (missing.fixed_colour_add)
		printf("Fixed colour add, ");

	if (missing.fixed_colour_sub)
		printf("Fixed colour subtract, ");

	printf("\n");

	printf("Window 1 enabled on: ");
	debug_print_window(missing.window1);

	printf("\n");
	
	printf("Window 2 enabled on: ");
	debug_print_window(missing.window2);

	printf("\n");

	if (missing.bg_offset_read)
		printf("BG offset read, ");

	if (missing.oam_address_read)
		printf("OAM address read, ");

	if (missing.sprite_priority_rotation)
		printf("Sprite priority rotation, ");

	if (missing.fast_rom)
		printf("Fast 3.58MHz ROM access enabled, ");

	if (missing.matrix_multiply)
		printf("Matrix multiply 16bit by 8bit used, ");

	printf("\n");

	if (missing.virq)
		printf("V-IRQ used at line %d, ", missing.virq_pos);

	if (missing.hirq)
		printf("H-IRQ used at position %d, ", missing.hirq_pos);

	printf("\n");

	if (missing.h_v_latch)
		printf("H and V-Pos latched, ");

	if (missing.h_counter_read)
		printf("H-Pos read, ");

	if (missing.v_counter_read)
		printf("V-Pos read, ");

	printf("\n");

	if (missing.oam_read)
		printf("OAM read, ");

	if (missing.vram_read)
		printf("VRAM read, ");

	if (missing.cgram_read)
		printf("CG-RAM read, ");

	if (missing.wram_read)
		printf("WRAM read, ");

	if (missing.dma_read)
		printf("DMA read, ");

	if (missing.vram_inc)
		printf("VRAM inc: %d, ", missing.vram_inc);

	if (missing.vram_full_graphic_inc)
		printf("VRAM full graphic inc: %d, ", missing.vram_full_graphic_inc);

	printf("\n");

	for (int i = 0; i < 8; i++)
	{
		if (missing.hdma[i].used)
		{
			printf("HDMA %d 0x%02X%04X->0x21%02X %s, ",
			       i, missing.hdma[i].abus_bank, missing.hdma[i].abus_address, missing.hdma[i].bbus_address,
			       missing.hdma[i].indirect_address ? "indirect" : "absolute");

			if (missing.hdma[i].force_table_address_write)
				printf("Forced address write, ");

			if (missing.hdma[i].force_table_address_read)
				printf("Current address read, ");

			if (missing.hdma[i].line_count_write)
				printf("Line count write, ");

			if (missing.hdma[i].line_count_read)
				printf("Line count read, ");

			printf("\n");
		}
	}

	for (int i = 0; i < 8; i++)
	{
		if (missing.dma_channels & (1 << i))
		{
			printf("DMA %d [%d] 0x%02X%04X->0x21%02X Num: %d %s, \n",
			       i, DMA[i].TransferMode, DMA[i].ABank, DMA[i].AAddress, DMA[i].BAddress, DMA[i].TransferBytes,
			       DMA[i].AAddressFixed ? "fixed" : (DMA[i].AAddressDecrement ? "dec" : "inc"));
		}
	}

	if (missing.unknownppu_read)
		printf("Read from unknown PPU register: $%04X, \n", missing.unknownppu_read);

	if (missing.unknownppu_write)
		printf("Write to unknown PPU register: $%04X, \n", missing.unknownppu_write);

	if (missing.unknowncpu_read)
		printf("Read from unknown CPU register: $%04X, \n", missing.unknowncpu_read);

	if (missing.unknowncpu_write)
		printf("Write to unknown CPU register: $%04X, \n", missing.unknowncpu_write);

	if (missing.unknowndsp_read)
		printf("Read from unknown DSP register: $%04X, \n", missing.unknowndsp_read);

	if (missing.unknowndsp_write)
		printf("Write to unknown DSP register: $%04X, \n", missing.unknowndsp_write);
}

void S9xDoDebug (void)
{
	char	Line[513];

	Debug.Dump.Bank = 0;
	Debug.Dump.Address = 0;
	Debug.Unassemble.Bank = 0;
	Debug.Unassemble.Address = 0;

	S9xTextMode();

	strcpy(Line, "r");
	debug_process_command(Line);

	while (CPU.Flags & DEBUG_MODE_FLAG)
	{
		int32	Cycles;
		char	*p;

		printf("> ");
		fflush(stdout);

		p = fgets(Line, sizeof(Line) - 1, stdin);
		Line[strlen(Line) - 1] = 0;

		Cycles = CPU.Cycles;
		debug_process_command(Line);
		CPU.Cycles = Cycles;
	}

	if (!(CPU.Flags & SINGLE_STEP_FLAG))
		S9xGraphicsMode();
}

void S9xTrace (void)
{
	char	msg[512];

	ENSURE_TRACE_OPEN(trace, "trace.log", "a")

	debug_cpu_op_print(msg, Registers.PB, Registers.PCw);
	fprintf(trace, "%s\n", msg);
}

void S9xSA1Trace (void)
{
	char	msg[512];

	ENSURE_TRACE_OPEN(trace2, "trace_sa1.log", "a")

	debug_sa1_op_print(msg, SA1Registers.PB, SA1Registers.PCw);
	fprintf(trace2, "%s\n", msg);
}

void S9xTraceMessage (const char *s)
{
	if (s)
	{
		if (trace)
			fprintf(trace, "%s\n", s);
		else
		if (trace2)
			fprintf(trace2, "%s\n", s);
	}
}

void S9xTraceFormattedMessage (const char *s, ...)
{
	char	msg[512];

	if (s)
	{
		va_list	argptr;

		va_start(argptr, s);
		vsprintf(msg, s, argptr);
		va_end(argptr);

		S9xTraceMessage(msg);
	}
}

void S9xPrintHVPosition (char *s)
{
	sprintf(s, "HC:%04ld VC:%03ld FC:%02d", (long) CPU.Cycles, (long) CPU.V_Counter, IPPU.FrameCount);
}

#endif
