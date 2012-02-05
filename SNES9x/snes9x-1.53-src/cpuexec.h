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


#ifndef _CPUEXEC_H_
#define _CPUEXEC_H_

#include "ppu.h"
#ifdef DEBUGGER
#include "debug.h"
#endif

struct SOpcodes
{
	void (*S9xOpcode) (void);
};

struct SICPU
{
	struct SOpcodes	*S9xOpcodes;
	uint8	*S9xOpLengths;
	uint8	_Carry;
	uint8	_Zero;
	uint8	_Negative;
	uint8	_Overflow;
	uint32	ShiftedPB;
	uint32	ShiftedDB;
	uint32	Frame;
	uint32	FrameAdvanceCount;
};

extern struct SICPU		ICPU;

extern struct SOpcodes	S9xOpcodesE1[256];
extern struct SOpcodes	S9xOpcodesM1X1[256];
extern struct SOpcodes	S9xOpcodesM1X0[256];
extern struct SOpcodes	S9xOpcodesM0X1[256];
extern struct SOpcodes	S9xOpcodesM0X0[256];
extern struct SOpcodes	S9xOpcodesSlow[256];
extern uint8			S9xOpLengthsM1X1[256];
extern uint8			S9xOpLengthsM1X0[256];
extern uint8			S9xOpLengthsM0X1[256];
extern uint8			S9xOpLengthsM0X0[256];

void S9xMainLoop (void);
void S9xReset (void);
void S9xSoftReset (void);
void S9xDoHEventProcessing (void);

static inline void S9xUnpackStatus (void)
{
	ICPU._Zero = (Registers.PL & Zero) == 0;
	ICPU._Negative = (Registers.PL & Negative);
	ICPU._Carry = (Registers.PL & Carry);
	ICPU._Overflow = (Registers.PL & Overflow) >> 6;
}

static inline void S9xPackStatus (void)
{
	Registers.PL &= ~(Zero | Negative | Carry | Overflow);
	Registers.PL |= ICPU._Carry | ((ICPU._Zero == 0) << 1) | (ICPU._Negative & 0x80) | (ICPU._Overflow << 6);
}

static inline void S9xFixCycles (void)
{
	if (CheckEmulation())
	{
		ICPU.S9xOpcodes = S9xOpcodesE1;
		ICPU.S9xOpLengths = S9xOpLengthsM1X1;
	}
	else
	if (CheckMemory())
	{
		if (CheckIndex())
		{
			ICPU.S9xOpcodes = S9xOpcodesM1X1;
			ICPU.S9xOpLengths = S9xOpLengthsM1X1;
		}
		else
		{
			ICPU.S9xOpcodes = S9xOpcodesM1X0;
			ICPU.S9xOpLengths = S9xOpLengthsM1X0;
		}
	}
	else
	{
		if (CheckIndex())
		{
			ICPU.S9xOpcodes = S9xOpcodesM0X1;
			ICPU.S9xOpLengths = S9xOpLengthsM0X1;
		}
		else
		{
			ICPU.S9xOpcodes = S9xOpcodesM0X0;
			ICPU.S9xOpLengths = S9xOpLengthsM0X0;
		}
	}
}

static inline void S9xCheckInterrupts (void)
{
	bool8	thisIRQ = PPU.HTimerEnabled || PPU.VTimerEnabled;

	if (CPU.IRQLine && thisIRQ)
		CPU.IRQTransition = TRUE;

	if (PPU.HTimerEnabled)
	{
		int32	htimepos = PPU.HTimerPosition;
		if (CPU.Cycles >= Timings.H_Max)
			htimepos += Timings.H_Max;

		if (CPU.PrevCycles >= htimepos || CPU.Cycles < htimepos)
			thisIRQ = FALSE;
	}

	if (PPU.VTimerEnabled)
	{
		int32	vcounter = CPU.V_Counter;
		if (CPU.Cycles >= Timings.H_Max)
			vcounter++;

		if (vcounter != PPU.VTimerPosition)
			thisIRQ = FALSE;
	}

	if (!CPU.IRQLastState && thisIRQ)
	{
#ifdef DEBUGGER
		S9xTraceFormattedMessage("--- /IRQ High->Low  prev HC:%04d  curr HC:%04d  HTimer:%d Pos:%04d  VTimer:%d Pos:%03d",
			CPU.PrevCycles, CPU.Cycles, PPU.HTimerEnabled, PPU.HTimerPosition, PPU.VTimerEnabled, PPU.VTimerPosition);
#endif
		CPU.IRQLine = TRUE;
	}

	CPU.IRQLastState = thisIRQ;
}

#endif
