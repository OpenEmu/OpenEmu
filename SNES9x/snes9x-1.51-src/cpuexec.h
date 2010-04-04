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




#ifndef _CPUEXEC_H_
#define _CPUEXEC_H_

#include "snes9x.h"

struct SOpcodes {
#ifdef __WIN32__
	void (__cdecl *S9xOpcode)( void);
#else
	void (*S9xOpcode)( void);
#endif
};

struct SICPU
{
    uint8  *Speed; // unused
    struct SOpcodes *S9xOpcodes;
    uint8  *S9xOpLengths;
    uint8  _Carry;
    uint8  _Zero;
    uint8  _Negative;
    uint8  _Overflow;
    bool8  CPUExecuting;
    uint32 ShiftedPB;
    uint32 ShiftedDB;
    uint32 Frame;
    uint32 Scanline;
    uint32 FrameAdvanceCount;
	bool8 SavedAtOp;
};

START_EXTERN_C
extern struct SICPU ICPU;
END_EXTERN_C

#include "ppu.h"
#include "memmap.h"
#include "65c816.h"

START_EXTERN_C
void S9xMainLoop (void);
void S9xReset (void);
void S9xSoftReset (void);
void S9xDoHEventProcessing ();
void S9xClearIRQ (uint32);
void S9xSetIRQ (uint32);

extern struct SOpcodes S9xOpcodesE1 [256];
extern struct SOpcodes S9xOpcodesM1X1 [256];
extern struct SOpcodes S9xOpcodesM1X0 [256];
extern struct SOpcodes S9xOpcodesM0X1 [256];
extern struct SOpcodes S9xOpcodesM0X0 [256];
extern struct SOpcodes S9xOpcodesSlow [256];
extern uint8 S9xOpLengthsM1X1 [256];
extern uint8 S9xOpLengthsM1X0 [256];
extern uint8 S9xOpLengthsM0X1 [256];
extern uint8 S9xOpLengthsM0X0 [256];
END_EXTERN_C

STATIC inline void S9xUnpackStatus()
{
    ICPU._Zero = (Registers.PL & Zero) == 0;
    ICPU._Negative = (Registers.PL & Negative);
    ICPU._Carry = (Registers.PL & Carry);
    ICPU._Overflow = (Registers.PL & Overflow) >> 6;
}

STATIC inline void S9xPackStatus()
{
    Registers.PL &= ~(Zero | Negative | Carry | Overflow);
    Registers.PL |= ICPU._Carry | ((ICPU._Zero == 0) << 1) |
		    (ICPU._Negative & 0x80) | (ICPU._Overflow << 6);
}

STATIC inline void CLEAR_IRQ_SOURCE (uint32 M)
{
    CPU.IRQActive &= ~M;
    if (!CPU.IRQActive)
	CPU.Flags &= ~IRQ_FLAG;
}

STATIC inline void S9xFixCycles ()
{
    if (CheckEmulation ())
    {
	ICPU.S9xOpcodes = S9xOpcodesE1;
        ICPU.S9xOpLengths = S9xOpLengthsM1X1;
    }
    else
    if (CheckMemory ())
    {
	if (CheckIndex ())
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
	if (CheckIndex ())
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

STATIC inline void S9xReschedule (void)
{
	uint8	next = 0;
	int32	hpos = 0;

	switch (CPU.WhichEvent)
	{
		case HC_HBLANK_START_EVENT:
		case HC_IRQ_1_3_EVENT:
			next = HC_HDMA_START_EVENT;
			hpos = Timings.HDMAStart;
			break;

		case HC_HDMA_START_EVENT:
		case HC_IRQ_3_5_EVENT:
			next = HC_HCOUNTER_MAX_EVENT;
			hpos = Timings.H_Max;
			break;

		case HC_HCOUNTER_MAX_EVENT:
		case HC_IRQ_5_7_EVENT:
			next = HC_HDMA_INIT_EVENT;
			hpos = Timings.HDMAInit;
			break;

		case HC_HDMA_INIT_EVENT:
		case HC_IRQ_7_9_EVENT:
			next = HC_RENDER_EVENT;
			hpos = Timings.RenderPos;
			break;

		case HC_RENDER_EVENT:
		case HC_IRQ_9_A_EVENT:
			next = HC_WRAM_REFRESH_EVENT;
			hpos = Timings.WRAMRefreshPos;
			break;

		case HC_WRAM_REFRESH_EVENT:
		case HC_IRQ_A_1_EVENT:
			next = HC_HBLANK_START_EVENT;
			hpos = Timings.HBlankStart;
			break;
	}

	if (((int32) PPU.HTimerPosition > CPU.NextEvent) && ((int32) PPU.HTimerPosition < hpos))
	{
		hpos = (int32) PPU.HTimerPosition;

		switch (next)
		{
			case HC_HDMA_START_EVENT:
				next = HC_IRQ_1_3_EVENT;
				break;

			case HC_HCOUNTER_MAX_EVENT:
				next = HC_IRQ_3_5_EVENT;
				break;

			case HC_HDMA_INIT_EVENT:
				next = HC_IRQ_5_7_EVENT;
				break;

			case HC_RENDER_EVENT:
				next = HC_IRQ_7_9_EVENT;
				break;

			case HC_WRAM_REFRESH_EVENT:
				next = HC_IRQ_9_A_EVENT;
				break;

			case HC_HBLANK_START_EVENT:
				next = HC_IRQ_A_1_EVENT;
				break;
		}
	}

	CPU.NextEvent  = hpos;
	CPU.WhichEvent = next;
}

#endif
