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


#ifndef _65C816_H_
#define _65C816_H_

#define Carry		1
#define Zero		2
#define IRQ			4
#define Decimal		8
#define IndexFlag	16
#define MemoryFlag	32
#define Overflow	64
#define Negative	128
#define Emulation	256

#define SetCarry()			(ICPU._Carry = 1)
#define ClearCarry()		(ICPU._Carry = 0)
#define SetZero()			(ICPU._Zero = 0)
#define ClearZero()			(ICPU._Zero = 1)
#define SetIRQ()			(Registers.PL |= IRQ)
#define ClearIRQ()			(Registers.PL &= ~IRQ)
#define SetDecimal()		(Registers.PL |= Decimal)
#define ClearDecimal()		(Registers.PL &= ~Decimal)
#define SetIndex()			(Registers.PL |= IndexFlag)
#define ClearIndex()		(Registers.PL &= ~IndexFlag)
#define SetMemory()			(Registers.PL |= MemoryFlag)
#define ClearMemory()		(Registers.PL &= ~MemoryFlag)
#define SetOverflow()		(ICPU._Overflow = 1)
#define ClearOverflow()		(ICPU._Overflow = 0)
#define SetNegative()		(ICPU._Negative = 0x80)
#define ClearNegative()		(ICPU._Negative = 0)

#define CheckCarry()		(ICPU._Carry)
#define CheckZero()			(ICPU._Zero == 0)
#define CheckIRQ()			(Registers.PL & IRQ)
#define CheckDecimal()		(Registers.PL & Decimal)
#define CheckIndex()		(Registers.PL & IndexFlag)
#define CheckMemory()		(Registers.PL & MemoryFlag)
#define CheckOverflow()		(ICPU._Overflow)
#define CheckNegative()		(ICPU._Negative & 0x80)
#define CheckEmulation()	(Registers.P.W & Emulation)

#define SetFlags(f)			(Registers.P.W |= (f))
#define ClearFlags(f)		(Registers.P.W &= ~(f))
#define CheckFlag(f)		(Registers.PL & (f))

typedef union
{
#ifdef LSB_FIRST
	struct { uint8	l, h; } B;
#else
	struct { uint8	h, l; } B;
#endif
	uint16	W;
}	pair;

typedef union
{
#ifdef LSB_FIRST
	struct { uint8	xPCl, xPCh, xPB, z; } B;
	struct { uint16	xPC, d; } W;
#else
	struct { uint8	z, xPB, xPCh, xPCl; } B;
	struct { uint16	d, xPC; } W;
#endif
    uint32	xPBPC;
}	PC_t;

struct SRegisters
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

#define AL		A.B.l
#define AH		A.B.h
#define XL		X.B.l
#define XH		X.B.h
#define YL		Y.B.l
#define YH		Y.B.h
#define SL		S.B.l
#define SH		S.B.h
#define DL		D.B.l
#define DH		D.B.h
#define PL		P.B.l
#define PH		P.B.h
#define PBPC	PC.xPBPC
#define PCw		PC.W.xPC
#define PCh		PC.B.xPCh
#define PCl		PC.B.xPCl
#define PB		PC.B.xPB

extern struct SRegisters	Registers;

#endif
