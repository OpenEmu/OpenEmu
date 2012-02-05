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


#ifndef _SA1_H_
#define _SA1_H_

struct SSA1Registers
{
	uint8	DB;
	pair	P;
	pair	A;
	pair	D;
	pair	S;
	pair	X;
	pair	Y;
	PC_t	PC;
};

struct SSA1
{
	struct SOpcodes	*S9xOpcodes;
	uint8	*S9xOpLengths;
	uint8	_Carry;
	uint8	_Zero;
	uint8	_Negative;
	uint8	_Overflow;
	uint32	ShiftedPB;
	uint32	ShiftedDB;

	uint32	Flags;
	int32	Cycles;
	int32	PrevCycles;
	uint8	*PCBase;
	bool8	WaitingForInterrupt;

	uint8	*Map[MEMMAP_NUM_BLOCKS];
	uint8	*WriteMap[MEMMAP_NUM_BLOCKS];
	uint8	*BWRAM;

	bool8	in_char_dma;
	bool8	TimerIRQLastState;
	uint16	HTimerIRQPos;
	uint16	VTimerIRQPos;
	int16	HCounter;
	int16	VCounter;
	int16	PrevHCounter;
	int32	MemSpeed;
	int32	MemSpeedx2;
	int32	arithmetic_op;
	uint16	op1;
	uint16	op2;
	uint64	sum;
	bool8	overflow;
	uint8	VirtualBitmapFormat;
	uint8	variable_bit_pos;
};

#define SA1CheckCarry()		(SA1._Carry)
#define SA1CheckZero()		(SA1._Zero == 0)
#define SA1CheckIRQ()		(SA1Registers.PL & IRQ)
#define SA1CheckDecimal()	(SA1Registers.PL & Decimal)
#define SA1CheckIndex()		(SA1Registers.PL & IndexFlag)
#define SA1CheckMemory()	(SA1Registers.PL & MemoryFlag)
#define SA1CheckOverflow()	(SA1._Overflow)
#define SA1CheckNegative()	(SA1._Negative & 0x80)
#define SA1CheckEmulation()	(SA1Registers.P.W & Emulation)

#define SA1SetFlags(f)		(SA1Registers.P.W |= (f))
#define SA1ClearFlags(f)	(SA1Registers.P.W &= ~(f))
#define SA1CheckFlag(f)		(SA1Registers.PL & (f))

extern struct SSA1Registers	SA1Registers;
extern struct SSA1			SA1;
extern uint8				SA1OpenBus;
extern struct SOpcodes		S9xSA1OpcodesM1X1[256];
extern struct SOpcodes		S9xSA1OpcodesM1X0[256];
extern struct SOpcodes		S9xSA1OpcodesM0X1[256];
extern struct SOpcodes		S9xSA1OpcodesM0X0[256];
extern uint8				S9xOpLengthsM1X1[256];
extern uint8				S9xOpLengthsM1X0[256];
extern uint8				S9xOpLengthsM0X1[256];
extern uint8				S9xOpLengthsM0X0[256];

uint8 S9xSA1GetByte (uint32);
void S9xSA1SetByte (uint8, uint32);
uint16 S9xSA1GetWord (uint32, enum s9xwrap_t w = WRAP_NONE);
void S9xSA1SetWord (uint16, uint32, enum s9xwrap_t w = WRAP_NONE, enum s9xwriteorder_t o = WRITE_01);
void S9xSA1SetPCBase (uint32);
uint8 S9xGetSA1 (uint32);
void S9xSetSA1 (uint8, uint32);
void S9xSA1Init (void);
void S9xSA1MainLoop (void);
void S9xSA1PostLoadState (void);

static inline void S9xSA1UnpackStatus (void)
{
	SA1._Zero = (SA1Registers.PL & Zero) == 0;
	SA1._Negative = (SA1Registers.PL & Negative);
	SA1._Carry = (SA1Registers.PL & Carry);
	SA1._Overflow = (SA1Registers.PL & Overflow) >> 6;
}

static inline void S9xSA1PackStatus (void)
{
	SA1Registers.PL &= ~(Zero | Negative | Carry | Overflow);
	SA1Registers.PL |= SA1._Carry | ((SA1._Zero == 0) << 1) | (SA1._Negative & 0x80) | (SA1._Overflow << 6);
}

static inline void S9xSA1FixCycles (void)
{
	if (SA1CheckEmulation())
	{
		SA1.S9xOpcodes = S9xSA1OpcodesM1X1;
		SA1.S9xOpLengths = S9xOpLengthsM1X1;
	}
	else
	if (SA1CheckMemory())
	{
		if (SA1CheckIndex())
		{
			SA1.S9xOpcodes = S9xSA1OpcodesM1X1;
			SA1.S9xOpLengths = S9xOpLengthsM1X1;
		}
		else
		{
			SA1.S9xOpcodes = S9xSA1OpcodesM1X0;
			SA1.S9xOpLengths = S9xOpLengthsM1X0;
		}
	}
	else
	{
		if (SA1CheckIndex())
		{
			SA1.S9xOpcodes = S9xSA1OpcodesM0X1;
			SA1.S9xOpLengths = S9xOpLengthsM0X1;
		}
		else
		{
			SA1.S9xOpcodes = S9xSA1OpcodesM0X0;
			SA1.S9xOpLengths = S9xOpLengthsM0X0;
		}
	}
}

#endif
