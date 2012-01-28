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

#define CPU								SA1
#define ICPU							SA1
#define Registers						SA1Registers
#define OpenBus							SA1OpenBus
#define S9xGetByte						S9xSA1GetByte
#define S9xGetWord						S9xSA1GetWord
#define S9xSetByte						S9xSA1SetByte
#define S9xSetWord						S9xSA1SetWord
#define S9xSetPCBase					S9xSA1SetPCBase
#define S9xOpcodesM1X1					S9xSA1OpcodesM1X1
#define S9xOpcodesM1X0					S9xSA1OpcodesM1X0
#define S9xOpcodesM0X1					S9xSA1OpcodesM0X1
#define S9xOpcodesM0X0					S9xSA1OpcodesM0X0
#define S9xOpcodesE1					S9xSA1OpcodesE1
#define S9xOpcodesSlow					S9xSA1OpcodesSlow
#define S9xOpcode_IRQ					S9xSA1Opcode_IRQ
#define S9xOpcode_NMI					S9xSA1Opcode_NMI
#define S9xUnpackStatus					S9xSA1UnpackStatus
#define S9xPackStatus					S9xSA1PackStatus
#define S9xFixCycles					S9xSA1FixCycles
#define Immediate8						SA1Immediate8
#define Immediate16						SA1Immediate16
#define Relative						SA1Relative
#define RelativeLong					SA1RelativeLong
#define Absolute						SA1Absolute
#define AbsoluteLong					SA1AbsoluteLong
#define AbsoluteIndirect				SA1AbsoluteIndirect
#define AbsoluteIndirectLong			SA1AbsoluteIndirectLong
#define AbsoluteIndexedIndirect			SA1AbsoluteIndexedIndirect
#define Direct							SA1Direct
#define DirectIndirectIndexed			SA1DirectIndirectIndexed
#define DirectIndirectIndexedLong		SA1DirectIndirectIndexedLong
#define DirectIndexedIndirect			SA1DirectIndexedIndirect
#define DirectIndexedX					SA1DirectIndexedX
#define DirectIndexedY					SA1DirectIndexedY
#define AbsoluteIndexedX				SA1AbsoluteIndexedX
#define AbsoluteIndexedY				SA1AbsoluteIndexedY
#define AbsoluteLongIndexedX			SA1AbsoluteLongIndexedX
#define DirectIndirect					SA1DirectIndirect
#define DirectIndirectLong				SA1DirectIndirectLong
#define StackRelative					SA1StackRelative
#define StackRelativeIndirectIndexed	SA1StackRelativeIndirectIndexed

#define SA1_OPCODES

#include "cpuops.cpp"

static void S9xSA1UpdateTimer (void);


void S9xSA1MainLoop (void)
{
	if (Memory.FillRAM[0x2200] & 0x60)
	{
		SA1.Cycles += 6; // FIXME
		S9xSA1UpdateTimer();
		return;
	}

	// SA-1 NMI
	if ((Memory.FillRAM[0x2200] & 0x10) && !(Memory.FillRAM[0x220b] & 0x10))
	{
		Memory.FillRAM[0x2301] |= 0x10;
		Memory.FillRAM[0x220b] |= 0x10;

		if (SA1.WaitingForInterrupt)
		{
			SA1.WaitingForInterrupt = FALSE;
			SA1Registers.PCw++;
		}

		S9xSA1Opcode_NMI();
	}
	else
	if (!SA1CheckFlag(IRQ))
	{
		// SA-1 Timer IRQ
		if ((Memory.FillRAM[0x220a] & 0x40) && !(Memory.FillRAM[0x220b] & 0x40))
		{
			Memory.FillRAM[0x2301] |= 0x40;

			if (SA1.WaitingForInterrupt)
			{
				SA1.WaitingForInterrupt = FALSE;
				SA1Registers.PCw++;
			}

			S9xSA1Opcode_IRQ();
		}
		else
		// SA-1 DMA IRQ
		if ((Memory.FillRAM[0x220a] & 0x20) && !(Memory.FillRAM[0x220b] & 0x20))
		{
			Memory.FillRAM[0x2301] |= 0x20;

			if (SA1.WaitingForInterrupt)
			{
				SA1.WaitingForInterrupt = FALSE;
				SA1Registers.PCw++;
			}

			S9xSA1Opcode_IRQ();
		}
		else
		// SA-1 IRQ
		if ((Memory.FillRAM[0x2200] & 0x80) && !(Memory.FillRAM[0x220b] & 0x80))
		{
			Memory.FillRAM[0x2301] |= 0x80;

			if (SA1.WaitingForInterrupt)
			{
				SA1.WaitingForInterrupt = FALSE;
				SA1Registers.PCw++;
			}

			S9xSA1Opcode_IRQ();
		}
	}

	for (int i = 0; i < 3 && !(Memory.FillRAM[0x2200] & 0x60); i++)
	{
	#ifdef DEBUGGER
		if (SA1.Flags & TRACE_FLAG)
			S9xSA1Trace();
	#endif

		register uint8				Op;
		register struct SOpcodes	*Opcodes;

		if (SA1.PCBase)
		{
			SA1OpenBus = Op = SA1.PCBase[Registers.PCw];
			Opcodes = SA1.S9xOpcodes;
		}
		else
		{
			Op = S9xSA1GetByte(Registers.PBPC);
			Opcodes = S9xOpcodesSlow;
		}

		if ((SA1Registers.PCw & MEMMAP_MASK) + SA1.S9xOpLengths[Op] >= MEMMAP_BLOCK_SIZE)
		{
			uint32	oldPC = SA1Registers.PBPC;
			S9xSA1SetPCBase(SA1Registers.PBPC);
			SA1Registers.PBPC = oldPC;
			Opcodes = S9xSA1OpcodesSlow;
		}

		Registers.PCw++;
		(*Opcodes[Op].S9xOpcode)();
	}

	S9xSA1UpdateTimer();
}

static void S9xSA1UpdateTimer (void) // FIXME
{
	SA1.PrevHCounter = SA1.HCounter;

	if (Memory.FillRAM[0x2210] & 0x80)
	{
		SA1.HCounter += (SA1.Cycles - SA1.PrevCycles);
		if (SA1.HCounter >= 0x800)
		{
			SA1.HCounter -= 0x800;
			SA1.PrevHCounter -= 0x800;
			if (++SA1.VCounter >= 0x200)
				SA1.VCounter = 0;
		}
	}
	else
	{
		SA1.HCounter += (SA1.Cycles - SA1.PrevCycles);
		if (SA1.HCounter >= Timings.H_Max_Master)
		{
			SA1.HCounter -= Timings.H_Max_Master;
			SA1.PrevHCounter -= Timings.H_Max_Master;
			if (++SA1.VCounter >= Timings.V_Max_Master)
				SA1.VCounter = 0;
		}
	}

	if (SA1.Cycles >= Timings.H_Max_Master)
		SA1.Cycles -= Timings.H_Max_Master;

	SA1.PrevCycles = SA1.Cycles;

	bool8	thisIRQ = Memory.FillRAM[0x2210] & 0x03;

	if (Memory.FillRAM[0x2210] & 0x01)
	{
		if (SA1.PrevHCounter >= SA1.HTimerIRQPos * ONE_DOT_CYCLE || SA1.HCounter < SA1.HTimerIRQPos * ONE_DOT_CYCLE)
			thisIRQ = FALSE;
	}

	if (Memory.FillRAM[0x2210] & 0x02)
	{
		if (SA1.VCounter != SA1.VTimerIRQPos * ONE_DOT_CYCLE)
			thisIRQ = FALSE;
	}

	// SA-1 Timer IRQ control
	if (!SA1.TimerIRQLastState && thisIRQ)
	{
		Memory.FillRAM[0x2301] |= 0x40;
		if (Memory.FillRAM[0x220a] & 0x40)
		{
			Memory.FillRAM[0x220b] &= ~0x40;
		#ifdef DEBUGGER
			S9xTraceFormattedMessage("--- SA-1 Timer IRQ triggered  prev HC:%04d  curr HC:%04d  HTimer:%d Pos:%04d  VTimer:%d Pos:%03d",
				SA1.PrevHCounter, SA1.HCounter,
				(Memory.FillRAM[0x2210] & 0x01) ? 1 : 0, SA1.HTimerIRQPos * ONE_DOT_CYCLE,
				(Memory.FillRAM[0x2210] & 0x02) ? 1 : 0, SA1.VTimerIRQPos);
		#endif
		}
	}

	SA1.TimerIRQLastState = thisIRQ;
}
