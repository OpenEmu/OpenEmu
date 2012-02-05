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
#include "fxinst.h"
#include "fxemu.h"

// Set this define if you wish the plot instruction to check for y-pos limits (I don't think it's nessecary)
#define CHECK_LIMITS


/*
 Codes used:
   rn   = a GSU register (r0 - r15)
   #n   = 4 bit immediate value
   #pp  = 8 bit immediate value
   (yy) = 8 bit word address (0x0000 - 0x01fe)
   #xx  = 16 bit immediate value
   (xx) = 16 bit address (0x0000 - 0xffff)
*/

// 00 - stop - stop GSU execution (and maybe generate an IRQ)
static void fx_stop (void)
{
	CF(G);
	GSU.vCounter = 0;
	GSU.vInstCount = GSU.vCounter;

	// Check if we need to generate an IRQ
	if (!(GSU.pvRegisters[GSU_CFGR] & 0x80))
		SF(IRQ);

	GSU.vPlotOptionReg = 0;
	GSU.vPipe = 1;
	CLRFLAGS;
	R15++;
}

// 01 - nop - no operation
static void fx_nop (void)
{
	CLRFLAGS;
	R15++;
}

// 02 - cache - reintialize GSU cache
static void fx_cache (void)
{
	uint32	c = R15 & 0xfff0;

	if (GSU.vCacheBaseReg != c || !GSU.bCacheActive)
	{
		fx_flushCache();
		GSU.vCacheBaseReg = c;
		GSU.bCacheActive = TRUE;

	#if 0
		if (c < (0x10000 - 512))
		{
			const uint8	*t = &ROM(c);
			memcpy(GSU.pvCache, t, 512);
		}
		else
		{
			const uint8	*t1, t2;
			uint32	i = 0x10000 - c;
			t1 = &ROM(c);
			t2 = &ROM(0);
			memcpy(GSU.pvCache, t1, i);
			memcpy(&GSU.pvCache[i], t2, 512 - i);
		}
	#endif
	}

	CLRFLAGS;
	R15++;
}

// 03 - lsr - logic shift right
static void fx_lsr (void)
{
	uint32	v;
	GSU.vCarry = SREG & 1;
	v = USEX16(SREG) >> 1;
	R15++;
	DREG = v;
	GSU.vSign = v;
	GSU.vZero = v;
	TESTR14;
	CLRFLAGS;
}

// 04 - rol - rotate left
static void fx_rol (void)
{
	uint32	v = USEX16((SREG << 1) + GSU.vCarry);
	GSU.vCarry = (SREG >> 15) & 1;
	R15++;
	DREG = v;
	GSU.vSign = v;
	GSU.vZero = v;
	TESTR14;
	CLRFLAGS;
}

// 05 - bra - branch always
static void fx_bra (void)
{
	uint8	v = PIPE;
	R15++;
	FETCHPIPE;
	R15 += SEX8(v);
}

// Branch on condition
#define	BRA_COND(cond) \
	uint8 v = PIPE; \
	R15++; \
	FETCHPIPE; \
	if (cond) \
		R15 += SEX8(v); \
	else \
		R15++

#define TEST_S	(GSU.vSign & 0x8000)
#define TEST_Z	(USEX16(GSU.vZero) == 0)
#define TEST_OV	(GSU.vOverflow >= 0x8000 || GSU.vOverflow < -0x8000)
#define TEST_CY	(GSU.vCarry & 1)

// 06 - blt - branch on less than
static void fx_blt (void)
{
	BRA_COND((TEST_S != 0) != (TEST_OV != 0));
}

// 07 - bge - branch on greater or equals
static void fx_bge (void)
{
	BRA_COND((TEST_S != 0) == (TEST_OV != 0));
}

// 08 - bne - branch on not equal
static void fx_bne (void)
{
	BRA_COND(!TEST_Z);
}

// 09 - beq - branch on equal
static void fx_beq (void)
{
	BRA_COND(TEST_Z);
}

// 0a - bpl - branch on plus
static void fx_bpl (void)
{
	BRA_COND(!TEST_S);
}

// 0b - bmi - branch on minus
static void fx_bmi (void)
{
	BRA_COND(TEST_S);
}

// 0c - bcc - branch on carry clear
static void fx_bcc (void)
{
	BRA_COND(!TEST_CY);
}

// 0d - bcs - branch on carry set
static void fx_bcs (void)
{
	BRA_COND(TEST_CY);
}

// 0e - bvc - branch on overflow clear
static void fx_bvc (void)
{
	BRA_COND(!TEST_OV);
}

// 0f - bvs - branch on overflow set
static void fx_bvs (void)
{
	BRA_COND(TEST_OV);
}

// 10-1f - to rn - set register n as destination register
// 10-1f (B) - move rn - move one register to another (if B flag is set)
#define FX_TO(reg) \
	if (TF(B)) \
	{ \
		GSU.avReg[(reg)] = SREG; \
		CLRFLAGS; \
	} \
	else \
		GSU.pvDreg = &GSU.avReg[reg]; \
	R15++

#define FX_TO_R14(reg) \
	if (TF(B)) \
	{ \
		GSU.avReg[(reg)] = SREG; \
		CLRFLAGS; \
		READR14; \
	} \
	else \
		GSU.pvDreg = &GSU.avReg[reg]; \
	R15++

#define FX_TO_R15(reg) \
	if (TF(B)) \
	{ \
		GSU.avReg[(reg)] = SREG; \
		CLRFLAGS; \
	} \
	else \
	{ \
		GSU.pvDreg = &GSU.avReg[reg]; \
		R15++; \
	}

static void fx_to_r0 (void)
{
	FX_TO(0);
}

static void fx_to_r1 (void)
{
	FX_TO(1);
}

static void fx_to_r2 (void)
{
	FX_TO(2);
}

static void fx_to_r3 (void)
{
	FX_TO(3);
}

static void fx_to_r4 (void)
{
	FX_TO(4);
}

static void fx_to_r5 (void)
{
	FX_TO(5);
}

static void fx_to_r6 (void)
{
	FX_TO(6);
}

static void fx_to_r7 (void)
{
	FX_TO(7);
}

static void fx_to_r8 (void)
{
	FX_TO(8);
}

static void fx_to_r9 (void)
{
	FX_TO(9);
}

static void fx_to_r10 (void)
{
	FX_TO(10);
}

static void fx_to_r11 (void)
{
	FX_TO(11);
}

static void fx_to_r12 (void)
{
	FX_TO(12);
}

static void fx_to_r13 (void)
{
	FX_TO(13);
}

static void fx_to_r14 (void)
{
	FX_TO_R14(14);
}

static void fx_to_r15 (void)
{
	FX_TO_R15(15);
}

// 20-2f - to rn - set register n as source and destination register
#define FX_WITH(reg) \
	SF(B); \
	GSU.pvSreg = GSU.pvDreg = &GSU.avReg[reg]; \
	R15++

static void fx_with_r0 (void)
{
	FX_WITH(0);
}

static void fx_with_r1 (void)
{
	FX_WITH(1);
}

static void fx_with_r2 (void)
{
	FX_WITH(2);
}

static void fx_with_r3 (void)
{
	FX_WITH(3);
}

static void fx_with_r4 (void)
{
	FX_WITH(4);
}

static void fx_with_r5 (void)
{
	FX_WITH(5);
}

static void fx_with_r6 (void)
{
	FX_WITH(6);
}

static void fx_with_r7 (void)
{
	FX_WITH(7);
}

static void fx_with_r8 (void)
{
	FX_WITH(8);
}

static void fx_with_r9 (void)
{
	FX_WITH(9);
}

static void fx_with_r10 (void)
{
	FX_WITH(10);
}

static void fx_with_r11 (void)
{
	FX_WITH(11);
}

static void fx_with_r12 (void)
{
	FX_WITH(12);
}

static void fx_with_r13 (void)
{
	FX_WITH(13);
}

static void fx_with_r14 (void)
{
	FX_WITH(14);
}

static void fx_with_r15 (void)
{
	FX_WITH(15);
}

// 30-3b - stw (rn) - store word
#define FX_STW(reg) \
	GSU.vLastRamAdr = GSU.avReg[reg]; \
	RAM(GSU.avReg[reg]) = (uint8) SREG; \
	RAM(GSU.avReg[reg] ^ 1) = (uint8) (SREG >> 8); \
	CLRFLAGS; \
	R15++

static void fx_stw_r0 (void)
{
	FX_STW(0);
}

static void fx_stw_r1 (void)
{
	FX_STW(1);
}

static void fx_stw_r2 (void)
{
	FX_STW(2);
}

static void fx_stw_r3 (void)
{
	FX_STW(3);
}

static void fx_stw_r4 (void)
{
	FX_STW(4);
}

static void fx_stw_r5 (void)
{
	FX_STW(5);
}

static void fx_stw_r6 (void)
{
	FX_STW(6);
}

static void fx_stw_r7 (void)
{
	FX_STW(7);
}

static void fx_stw_r8 (void)
{
	FX_STW(8);
}

static void fx_stw_r9 (void)
{
	FX_STW(9);
}

static void fx_stw_r10 (void)
{
	FX_STW(10);
}

static void fx_stw_r11 (void)
{
	FX_STW(11);
}

// 30-3b (ALT1) - stb (rn) - store byte
#define FX_STB(reg) \
	GSU.vLastRamAdr = GSU.avReg[reg]; \
	RAM(GSU.avReg[reg]) = (uint8) SREG; \
	CLRFLAGS; \
	R15++

static void fx_stb_r0 (void)
{
	FX_STB(0);
}

static void fx_stb_r1 (void)
{
	FX_STB(1);
}

static void fx_stb_r2 (void)
{
	FX_STB(2);
}

static void fx_stb_r3 (void)
{
	FX_STB(3);
}

static void fx_stb_r4 (void)
{
	FX_STB(4);
}

static void fx_stb_r5 (void)
{
	FX_STB(5);
}

static void fx_stb_r6 (void)
{
	FX_STB(6);
}

static void fx_stb_r7 (void)
{
	FX_STB(7);
}

static void fx_stb_r8 (void)
{
	FX_STB(8);
}

static void fx_stb_r9 (void)
{
	FX_STB(9);
}

static void fx_stb_r10 (void)
{
	FX_STB(10);
}

static void fx_stb_r11 (void)
{
	FX_STB(11);
}

// 3c - loop - decrement loop counter, and branch on not zero
static void fx_loop (void)
{
	GSU.vSign = GSU.vZero = --R12;
	if ((uint16) R12 != 0)
		R15 = R13;
	else
		R15++;
	CLRFLAGS;
}

// 3d - alt1 - set alt1 mode
static void fx_alt1 (void)
{
	SF(ALT1);
	CF(B);
	R15++;
}

// 3e - alt2 - set alt2 mode
static void fx_alt2 (void)
{
	SF(ALT2);
	CF(B);
	R15++;
}

// 3f - alt3 - set alt3 mode
static void fx_alt3 (void)
{
	SF(ALT1);
	SF(ALT2);
	CF(B);
	R15++;
}

// 40-4b - ldw (rn) - load word from RAM
#define FX_LDW(reg) \
	uint32	v; \
	GSU.vLastRamAdr = GSU.avReg[reg]; \
	v = (uint32) RAM(GSU.avReg[reg]); \
	v |= ((uint32) RAM(GSU.avReg[reg] ^ 1)) << 8; \
	R15++; \
	DREG = v; \
	TESTR14; \
	CLRFLAGS

static void fx_ldw_r0 (void)
{
	FX_LDW(0);
}

static void fx_ldw_r1 (void)
{
	FX_LDW(1);
}

static void fx_ldw_r2 (void)
{
	FX_LDW(2);
}

static void fx_ldw_r3 (void)
{
	FX_LDW(3);
}

static void fx_ldw_r4 (void)
{
	FX_LDW(4);
}

static void fx_ldw_r5 (void)
{
	FX_LDW(5);
}

static void fx_ldw_r6 (void)
{
	FX_LDW(6);
}

static void fx_ldw_r7 (void)
{
	FX_LDW(7);
}

static void fx_ldw_r8 (void)
{
	FX_LDW(8);
}

static void fx_ldw_r9 (void)
{
	FX_LDW(9);
}

static void fx_ldw_r10 (void)
{
	FX_LDW(10);
}

static void fx_ldw_r11 (void)
{
	FX_LDW(11);
}

// 40-4b (ALT1) - ldb (rn) - load byte
#define FX_LDB(reg) \
	uint32	v; \
	GSU.vLastRamAdr = GSU.avReg[reg]; \
	v = (uint32) RAM(GSU.avReg[reg]); \
	R15++; \
	DREG = v; \
	TESTR14; \
	CLRFLAGS

static void fx_ldb_r0 (void)
{
	FX_LDB(0);
}

static void fx_ldb_r1 (void)
{
	FX_LDB(1);
}

static void fx_ldb_r2 (void)
{
	FX_LDB(2);
}

static void fx_ldb_r3 (void)
{
	FX_LDB(3);
}

static void fx_ldb_r4 (void)
{
	FX_LDB(4);
}

static void fx_ldb_r5 (void)
{
	FX_LDB(5);
}

static void fx_ldb_r6 (void)
{
	FX_LDB(6);
}

static void fx_ldb_r7 (void)
{
	FX_LDB(7);
}

static void fx_ldb_r8 (void)
{
	FX_LDB(8);
}

static void fx_ldb_r9 (void)
{
	FX_LDB(9);
}

static void fx_ldb_r10 (void)
{
	FX_LDB(10);
}

static void fx_ldb_r11 (void)
{
	FX_LDB(11);
}

// 4c - plot - plot pixel with R1, R2 as x, y and the color register as the color
static void fx_plot_2bit (void)
{
	uint32	x = USEX8(R1);
	uint32	y = USEX8(R2);
	uint8	*a;
	uint8	v, c;

	R15++;
	CLRFLAGS;
	R1++;

#ifdef CHECK_LIMITS
	if (y >= GSU.vScreenHeight)
		return;
#endif

	if (GSU.vPlotOptionReg & 0x02)
		c = (x ^ y) & 1 ? (uint8) (GSU.vColorReg >> 4) : (uint8) GSU.vColorReg;
	else
		c = (uint8) GSU.vColorReg;

	if (!(GSU.vPlotOptionReg & 0x01) && !(c & 0xf))
		return;

	a = GSU.apvScreen[y >> 3] + GSU.x[x >> 3] + ((y & 7) << 1);
	v = 128 >> (x & 7);

	if (c & 0x01)
		a[0] |=  v;
	else
		a[0] &= ~v;

	if (c & 0x02)
		a[1] |=  v;
	else
		a[1] &= ~v;
}

// 4c (ALT1) - rpix - read color of the pixel with R1, R2 as x, y
static void fx_rpix_2bit (void)
{
	uint32	x = USEX8(R1);
	uint32	y = USEX8(R2);
	uint8	*a;
	uint8	v;

	R15++;
	CLRFLAGS;

#ifdef CHECK_LIMITS
	if (y >= GSU.vScreenHeight)
		return;
#endif

	a = GSU.apvScreen[y >> 3] + GSU.x[x >> 3] + ((y & 7) << 1);
	v = 128 >> (x & 7);

	DREG = 0;
	DREG |= ((uint32) ((a[0] & v) != 0)) << 0;
	DREG |= ((uint32) ((a[1] & v) != 0)) << 1;
	TESTR14;
}

// 4c - plot - plot pixel with R1, R2 as x, y and the color register as the color
static void fx_plot_4bit (void)
{
	uint32	x = USEX8(R1);
	uint32	y = USEX8(R2);
	uint8	*a;
	uint8	v, c;

	R15++;
	CLRFLAGS;
	R1++;

#ifdef CHECK_LIMITS
	if (y >= GSU.vScreenHeight)
		return;
#endif

	if (GSU.vPlotOptionReg & 0x02)
		c = (x ^ y) & 1 ? (uint8) (GSU.vColorReg >> 4) : (uint8) GSU.vColorReg;
	else
		c = (uint8) GSU.vColorReg;

	if (!(GSU.vPlotOptionReg & 0x01) && !(c & 0xf))
		return;

	a = GSU.apvScreen[y >> 3] + GSU.x[x >> 3] + ((y & 7) << 1);
	v = 128 >> (x & 7);

	if (c & 0x01)
		a[0x00] |=  v;
	else
		a[0x00] &= ~v;

	if (c & 0x02)
		a[0x01] |=  v;
	else
		a[0x01] &= ~v;

	if (c & 0x04)
		a[0x10] |=  v;
	else
		a[0x10] &= ~v;

	if (c & 0x08)
		a[0x11] |=  v;
	else
		a[0x11] &= ~v;
}

// 4c (ALT1) - rpix - read color of the pixel with R1, R2 as x, y
static void fx_rpix_4bit (void)
{
	uint32	x = USEX8(R1);
	uint32	y = USEX8(R2);
	uint8	*a;
	uint8	v;

	R15++;
	CLRFLAGS;

#ifdef CHECK_LIMITS
	if (y >= GSU.vScreenHeight)
		return;
#endif

	a = GSU.apvScreen[y >> 3] + GSU.x[x >> 3] + ((y & 7) << 1);
	v = 128 >> (x & 7);

	DREG = 0;
	DREG |= ((uint32) ((a[0x00] & v) != 0)) << 0;
	DREG |= ((uint32) ((a[0x01] & v) != 0)) << 1;
	DREG |= ((uint32) ((a[0x10] & v) != 0)) << 2;
	DREG |= ((uint32) ((a[0x11] & v) != 0)) << 3;
	TESTR14;
}

// 4c - plot - plot pixel with R1, R2 as x, y and the color register as the color
static void fx_plot_8bit (void)
{
	uint32	x = USEX8(R1);
	uint32	y = USEX8(R2);
	uint8	*a;
	uint8	v, c;

	R15++;
	CLRFLAGS;
	R1++;

#ifdef CHECK_LIMITS
	if (y >= GSU.vScreenHeight)
		return;
#endif

	c = (uint8) GSU.vColorReg;
	if (!(GSU.vPlotOptionReg & 0x10))
	{
		if (!(GSU.vPlotOptionReg & 0x01) && !(c & 0xf))
			return;
	}
	else
	if (!(GSU.vPlotOptionReg & 0x01) && !c)
		return;

	a = GSU.apvScreen[y >> 3] + GSU.x[x >> 3] + ((y & 7) << 1);
	v = 128 >> (x & 7);

	if (c & 0x01)
		a[0x00] |=  v;
	else
		a[0x00] &= ~v;

	if (c & 0x02)
		a[0x01] |=  v;
	else
		a[0x01] &= ~v;

	if (c & 0x04)
		a[0x10] |=  v;
	else
		a[0x10] &= ~v;

	if (c & 0x08)
		a[0x11] |=  v;
	else
		a[0x11] &= ~v;

	if (c & 0x10)
		a[0x20] |=  v;
	else
		a[0x20] &= ~v;

	if (c & 0x20)
		a[0x21] |=  v;
	else
		a[0x21] &= ~v;

	if (c & 0x40)
		a[0x30] |=  v;
	else
		a[0x30] &= ~v;

	if (c & 0x80)
		a[0x31] |=  v;
	else
		a[0x31] &= ~v;
}

// 4c (ALT1) - rpix - read color of the pixel with R1, R2 as x, y
static void fx_rpix_8bit (void)
{
	uint32	x = USEX8(R1);
	uint32	y = USEX8(R2);
	uint8	*a;
	uint8	v;

	R15++;
	CLRFLAGS;

#ifdef CHECK_LIMITS
	if (y >= GSU.vScreenHeight)
		return;
#endif

	a = GSU.apvScreen[y >> 3] + GSU.x[x >> 3] + ((y & 7) << 1);
	v = 128 >> (x & 7);

	DREG = 0;
	DREG |= ((uint32) ((a[0x00] & v) != 0)) << 0;
	DREG |= ((uint32) ((a[0x01] & v) != 0)) << 1;
	DREG |= ((uint32) ((a[0x10] & v) != 0)) << 2;
	DREG |= ((uint32) ((a[0x11] & v) != 0)) << 3;
	DREG |= ((uint32) ((a[0x20] & v) != 0)) << 4;
	DREG |= ((uint32) ((a[0x21] & v) != 0)) << 5;
	DREG |= ((uint32) ((a[0x30] & v) != 0)) << 6;
	DREG |= ((uint32) ((a[0x31] & v) != 0)) << 7;
	GSU.vZero = DREG;
	TESTR14;
}

// 4c - plot - plot pixel with R1, R2 as x, y and the color register as the color
static void fx_plot_obj (void)
{
#ifdef DEBUGGER
	fprintf(stderr, "ERROR fx_plot_obj called\n");
#endif
}

// 4c (ALT1) - rpix - read color of the pixel with R1, R2 as x, y
static void fx_rpix_obj (void)
{
#ifdef DEBUGGER
	fprintf(stderr, "ERROR fx_rpix_obj called\n");
#endif
}

// 4d - swap - swap upper and lower byte of a register
static void fx_swap (void)
{
	uint8	c = (uint8) SREG;
	uint8	d = (uint8) (SREG >> 8);
	uint32	v = (((uint32) c) << 8) | ((uint32) d);
	R15++;
	DREG = v;
	GSU.vSign = v;
	GSU.vZero = v;
	TESTR14;
	CLRFLAGS;
}

// 4e - color - copy source register to color register
static void fx_color (void)
{
	uint8	c = (uint8) SREG;

	if (GSU.vPlotOptionReg & 0x04)
		c = (c & 0xf0) | (c >> 4);
	if (GSU.vPlotOptionReg & 0x08)
	{
		GSU.vColorReg &= 0xf0;
		GSU.vColorReg |= c & 0x0f;
	}
	else
		GSU.vColorReg = USEX8(c);

	CLRFLAGS;
	R15++;
}

// 4e (ALT1) - cmode - set plot option register
static void fx_cmode (void)
{
	GSU.vPlotOptionReg = SREG;

	if (GSU.vPlotOptionReg & 0x10)		
		GSU.vScreenHeight = 256; // OBJ Mode (for drawing into sprites)
	else
		GSU.vScreenHeight = GSU.vScreenRealHeight;

	fx_computeScreenPointers();
	CLRFLAGS;
	R15++;
}

// 4f - not - perform exclusive exor with 1 on all bits
static void fx_not (void)
{
	uint32	v = ~SREG;
	R15++;
	DREG = v;
	GSU.vSign = v;
	GSU.vZero = v;
	TESTR14;
	CLRFLAGS;
}

// 50-5f - add rn - add, register + register
#define FX_ADD(reg) \
	int32	s = SUSEX16(SREG) + SUSEX16(GSU.avReg[reg]); \
	GSU.vCarry = s >= 0x10000; \
	GSU.vOverflow = ~(SREG ^ GSU.avReg[reg]) & (GSU.avReg[reg] ^ s) & 0x8000; \
	GSU.vSign = s; \
	GSU.vZero = s; \
	R15++; \
	DREG = s; \
	TESTR14; \
	CLRFLAGS

static void fx_add_r0 (void)
{
	FX_ADD(0);
}

static void fx_add_r1 (void)
{
	FX_ADD(1);
}

static void fx_add_r2 (void)
{
	FX_ADD(2);
}

static void fx_add_r3 (void)
{
	FX_ADD(3);
}

static void fx_add_r4 (void)
{
	FX_ADD(4);
}

static void fx_add_r5 (void)
{
	FX_ADD(5);
}

static void fx_add_r6 (void)
{
	FX_ADD(6);
}

static void fx_add_r7 (void)
{
	FX_ADD(7);
}

static void fx_add_r8 (void)
{
	FX_ADD(8);
}

static void fx_add_r9 (void)
{
	FX_ADD(9);
}

static void fx_add_r10 (void)
{
	FX_ADD(10);
}

static void fx_add_r11 (void)
{
	FX_ADD(11);
}

static void fx_add_r12 (void)
{
	FX_ADD(12);
}

static void fx_add_r13 (void)
{
	FX_ADD(13);
}

static void fx_add_r14 (void)
{
	FX_ADD(14);
}

static void fx_add_r15 (void)
{
	FX_ADD(15);
}

// 50-5f (ALT1) - adc rn - add with carry, register + register
#define FX_ADC(reg) \
	int32	s = SUSEX16(SREG) + SUSEX16(GSU.avReg[reg]) + SEX16(GSU.vCarry); \
	GSU.vCarry = s >= 0x10000; \
	GSU.vOverflow = ~(SREG ^ GSU.avReg[reg]) & (GSU.avReg[reg] ^ s) & 0x8000; \
	GSU.vSign = s; \
	GSU.vZero = s; \
	R15++; \
	DREG = s; \
	TESTR14; \
	CLRFLAGS

static void fx_adc_r0 (void)
{
	FX_ADC(0);
}

static void fx_adc_r1 (void)
{
	FX_ADC(1);
}

static void fx_adc_r2 (void)
{
	FX_ADC(2);
}

static void fx_adc_r3 (void)
{
	FX_ADC(3);
}

static void fx_adc_r4 (void)
{
	FX_ADC(4);
}

static void fx_adc_r5 (void)
{
	FX_ADC(5);
}

static void fx_adc_r6 (void)
{
	FX_ADC(6);
}

static void fx_adc_r7 (void)
{
	FX_ADC(7);
}

static void fx_adc_r8 (void)
{
	FX_ADC(8);
}

static void fx_adc_r9 (void)
{
	FX_ADC(9);
}

static void fx_adc_r10 (void)
{
	FX_ADC(10);
}

static void fx_adc_r11 (void)
{
	FX_ADC(11);
}

static void fx_adc_r12 (void)
{
	FX_ADC(12);
}

static void fx_adc_r13 (void)
{
	FX_ADC(13);
}

static void fx_adc_r14 (void)
{
	FX_ADC(14);
}

static void fx_adc_r15 (void)
{
	FX_ADC(15);
}

// 50-5f (ALT2) - add #n - add, register + immediate
#define FX_ADD_I(imm) \
	int32	s = SUSEX16(SREG) + imm; \
	GSU.vCarry = s >= 0x10000; \
	GSU.vOverflow = ~(SREG ^ imm) & (imm ^ s) & 0x8000; \
	GSU.vSign = s; \
	GSU.vZero = s; \
	R15++; \
	DREG = s; \
	TESTR14; \
	CLRFLAGS

static void fx_add_i0 (void)
{
	FX_ADD_I(0);
}

static void fx_add_i1 (void)
{
	FX_ADD_I(1);
}

static void fx_add_i2 (void)
{
	FX_ADD_I(2);
}

static void fx_add_i3 (void)
{
	FX_ADD_I(3);
}

static void fx_add_i4 (void)
{
	FX_ADD_I(4);
}

static void fx_add_i5 (void)
{
	FX_ADD_I(5);
}

static void fx_add_i6 (void)
{
	FX_ADD_I(6);
}

static void fx_add_i7 (void)
{
	FX_ADD_I(7);
}

static void fx_add_i8 (void)
{
	FX_ADD_I(8);
}

static void fx_add_i9 (void)
{
	FX_ADD_I(9);
}

static void fx_add_i10 (void)
{
	FX_ADD_I(10);
}

static void fx_add_i11 (void)
{
	FX_ADD_I(11);
}

static void fx_add_i12 (void)
{
	FX_ADD_I(12);
}

static void fx_add_i13 (void)
{
	FX_ADD_I(13);
}

static void fx_add_i14 (void)
{
	FX_ADD_I(14);
}

static void fx_add_i15 (void)
{
	FX_ADD_I(15);
}

// 50-5f (ALT3) - adc #n - add with carry, register + immediate
#define FX_ADC_I(imm) \
	int32	s = SUSEX16(SREG) + imm + SUSEX16(GSU.vCarry); \
	GSU.vCarry = s >= 0x10000; \
	GSU.vOverflow = ~(SREG ^ imm) & (imm ^ s) & 0x8000; \
	GSU.vSign = s; \
	GSU.vZero = s; \
	R15++; \
	DREG = s; \
	TESTR14; \
	CLRFLAGS

static void fx_adc_i0 (void)
{
	FX_ADC_I(0);
}

static void fx_adc_i1 (void)
{
	FX_ADC_I(1);
}

static void fx_adc_i2 (void)
{
	FX_ADC_I(2);
}

static void fx_adc_i3 (void)
{
	FX_ADC_I(3);
}

static void fx_adc_i4 (void)
{
	FX_ADC_I(4);
}

static void fx_adc_i5 (void)
{
	FX_ADC_I(5);
}

static void fx_adc_i6 (void)
{
	FX_ADC_I(6);
}

static void fx_adc_i7 (void)
{
	FX_ADC_I(7);
}

static void fx_adc_i8 (void)
{
	FX_ADC_I(8);
}

static void fx_adc_i9 (void)
{
	FX_ADC_I(9);
}

static void fx_adc_i10 (void)
{
	FX_ADC_I(10);
}

static void fx_adc_i11 (void)
{
	FX_ADC_I(11);
}

static void fx_adc_i12 (void)
{
	FX_ADC_I(12);
}

static void fx_adc_i13 (void)
{
	FX_ADC_I(13);
}

static void fx_adc_i14 (void)
{
	FX_ADC_I(14);
}

static void fx_adc_i15 (void)
{
	FX_ADC_I(15);
}

// 60-6f - sub rn - subtract, register - register
#define FX_SUB(reg) \
	int32	s = SUSEX16(SREG) - SUSEX16(GSU.avReg[reg]); \
	GSU.vCarry = s >= 0; \
	GSU.vOverflow = (SREG ^ GSU.avReg[reg]) & (SREG ^ s) & 0x8000; \
	GSU.vSign = s; \
	GSU.vZero = s; \
	R15++; \
	DREG = s; \
	TESTR14; \
	CLRFLAGS

static void fx_sub_r0 (void)
{
	FX_SUB(0);
}

static void fx_sub_r1 (void)
{
	FX_SUB(1);
}

static void fx_sub_r2 (void)
{
	FX_SUB(2);
}

static void fx_sub_r3 (void)
{
	FX_SUB(3);
}

static void fx_sub_r4 (void)
{
	FX_SUB(4);
}

static void fx_sub_r5 (void)
{
	FX_SUB(5);
}

static void fx_sub_r6 (void)
{
	FX_SUB(6);
}

static void fx_sub_r7 (void)
{
	FX_SUB(7);
}

static void fx_sub_r8 (void)
{
	FX_SUB(8);
}

static void fx_sub_r9 (void)
{
	FX_SUB(9);
}

static void fx_sub_r10 (void)
{
	FX_SUB(10);
}

static void fx_sub_r11 (void)
{
	FX_SUB(11);
}

static void fx_sub_r12 (void)
{
	FX_SUB(12);
}

static void fx_sub_r13 (void)
{
	FX_SUB(13);
}

static void fx_sub_r14 (void)
{
	FX_SUB(14);
}

static void fx_sub_r15 (void)
{
	FX_SUB(15);
}

// 60-6f (ALT1) - sbc rn - subtract with carry, register - register
#define FX_SBC(reg) \
	int32	s = SUSEX16(SREG) - SUSEX16(GSU.avReg[reg]) - (SUSEX16(GSU.vCarry ^ 1)); \
	GSU.vCarry = s >= 0; \
	GSU.vOverflow = (SREG ^ GSU.avReg[reg]) & (SREG ^ s) & 0x8000; \
	GSU.vSign = s; \
	GSU.vZero = s; \
	R15++; \
	DREG = s; \
	TESTR14; \
	CLRFLAGS

static void fx_sbc_r0 (void)
{
	FX_SBC(0);
}

static void fx_sbc_r1 (void)
{
	FX_SBC(1);
}

static void fx_sbc_r2 (void)
{
	FX_SBC(2);
}

static void fx_sbc_r3 (void)
{
	FX_SBC(3);
}

static void fx_sbc_r4 (void)
{
	FX_SBC(4);
}

static void fx_sbc_r5 (void)
{
	FX_SBC(5);
}

static void fx_sbc_r6 (void)
{
	FX_SBC(6);
}

static void fx_sbc_r7 (void)
{
	FX_SBC(7);
}

static void fx_sbc_r8 (void)
{
	FX_SBC(8);
}

static void fx_sbc_r9 (void)
{
	FX_SBC(9);
}

static void fx_sbc_r10 (void)
{
	FX_SBC(10);
}

static void fx_sbc_r11 (void)
{
	FX_SBC(11);
}

static void fx_sbc_r12 (void)
{
	FX_SBC(12);
}

static void fx_sbc_r13 (void)
{
	FX_SBC(13);
}

static void fx_sbc_r14 (void)
{
	FX_SBC(14);
}

static void fx_sbc_r15 (void)
{
	FX_SBC(15);
}

// 60-6f (ALT2) - sub #n - subtract, register - immediate
#define FX_SUB_I(imm) \
	int32	s = SUSEX16(SREG) - imm; \
	GSU.vCarry = s >= 0; \
	GSU.vOverflow = (SREG ^ imm) & (SREG ^ s) & 0x8000; \
	GSU.vSign = s; \
	GSU.vZero = s; \
	R15++; \
	DREG = s; \
	TESTR14; \
	CLRFLAGS

static void fx_sub_i0 (void)
{
	FX_SUB_I(0);
}

static void fx_sub_i1 (void)
{
	FX_SUB_I(1);
}

static void fx_sub_i2 (void)
{
	FX_SUB_I(2);
}

static void fx_sub_i3 (void)
{
	FX_SUB_I(3);
}

static void fx_sub_i4 (void)
{
	FX_SUB_I(4);
}

static void fx_sub_i5 (void)
{
	FX_SUB_I(5);
}

static void fx_sub_i6 (void)
{
	FX_SUB_I(6);
}

static void fx_sub_i7 (void)
{
	FX_SUB_I(7);
}

static void fx_sub_i8 (void)
{
	FX_SUB_I(8);
}

static void fx_sub_i9 (void)
{
	FX_SUB_I(9);
}

static void fx_sub_i10 (void)
{
	FX_SUB_I(10);
}

static void fx_sub_i11 (void)
{
	FX_SUB_I(11);
}

static void fx_sub_i12 (void)
{
	FX_SUB_I(12);
}

static void fx_sub_i13 (void)
{
	FX_SUB_I(13);
}

static void fx_sub_i14 (void)
{
	FX_SUB_I(14);
}

static void fx_sub_i15 (void)
{
	FX_SUB_I(15);
}

// 60-6f (ALT3) - cmp rn - compare, register, register
#define FX_CMP(reg) \
	int32	s = SUSEX16(SREG) - SUSEX16(GSU.avReg[reg]); \
	GSU.vCarry = s >= 0; \
	GSU.vOverflow = (SREG ^ GSU.avReg[reg]) & (SREG ^ s) & 0x8000; \
	GSU.vSign = s; \
	GSU.vZero = s; \
	R15++; \
	CLRFLAGS

static void fx_cmp_r0 (void)
{
	FX_CMP(0);
}

static void fx_cmp_r1 (void)
{
	FX_CMP(1);
}

static void fx_cmp_r2 (void)
{
	FX_CMP(2);
}

static void fx_cmp_r3 (void)
{
	FX_CMP(3);
}

static void fx_cmp_r4 (void)
{
	FX_CMP(4);
}

static void fx_cmp_r5 (void)
{
	FX_CMP(5);
}

static void fx_cmp_r6 (void)
{
	FX_CMP(6);
}

static void fx_cmp_r7 (void)
{
	FX_CMP(7);
}

static void fx_cmp_r8 (void)
{
	FX_CMP(8);
}

static void fx_cmp_r9 (void)
{
	FX_CMP(9);
}

static void fx_cmp_r10 (void)
{
	FX_CMP(10);
}

static void fx_cmp_r11 (void)
{
	FX_CMP(11);
}

static void fx_cmp_r12 (void)
{
	FX_CMP(12);
}

static void fx_cmp_r13 (void)
{
	FX_CMP(13);
}

static void fx_cmp_r14 (void)
{
	FX_CMP(14);
}

static void fx_cmp_r15 (void)
{
	FX_CMP(15);
}

// 70 - merge - R7 as upper byte, R8 as lower byte (used for texture-mapping)
static void fx_merge (void)
{
	uint32	v = (R7 & 0xff00) | ((R8 & 0xff00) >> 8);
	R15++;
	DREG = v;
	GSU.vOverflow = (v & 0xc0c0) << 16;
	GSU.vZero = !(v & 0xf0f0);
	GSU.vSign = ((v | (v << 8)) & 0x8000);
	GSU.vCarry = (v & 0xe0e0) != 0;
	TESTR14;
	CLRFLAGS;
}

// 71-7f - and rn - reister & register
#define FX_AND(reg) \
	uint32	v = SREG & GSU.avReg[reg]; \
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_and_r1 (void)
{
	FX_AND(1);
}

static void fx_and_r2 (void)
{
	FX_AND(2);
}

static void fx_and_r3 (void)
{
	FX_AND(3);
}

static void fx_and_r4 (void)
{
	FX_AND(4);
}

static void fx_and_r5 (void)
{
	FX_AND(5);
}

static void fx_and_r6 (void)
{
	FX_AND(6);
}

static void fx_and_r7 (void)
{
	FX_AND(7);
}

static void fx_and_r8 (void)
{
	FX_AND(8);
}

static void fx_and_r9 (void)
{
	FX_AND(9);
}

static void fx_and_r10 (void)
{
	FX_AND(10);
}

static void fx_and_r11 (void)
{
	FX_AND(11);
}

static void fx_and_r12 (void)
{
	FX_AND(12);
}

static void fx_and_r13 (void)
{
	FX_AND(13);
}

static void fx_and_r14 (void)
{
	FX_AND(14);
}

static void fx_and_r15 (void)
{
	FX_AND(15);
}

// 71-7f (ALT1) - bic rn - reister & ~register
#define FX_BIC(reg) \
	uint32	v = SREG & ~GSU.avReg[reg];	\
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_bic_r1 (void)
{
	FX_BIC(1);
}

static void fx_bic_r2 (void)
{
	FX_BIC(2);
}

static void fx_bic_r3 (void)
{
	FX_BIC(3);
}

static void fx_bic_r4 (void)
{
	FX_BIC(4);
}

static void fx_bic_r5 (void)
{
	FX_BIC(5);
}

static void fx_bic_r6 (void)
{
	FX_BIC(6);
}

static void fx_bic_r7 (void)
{
	FX_BIC(7);
}

static void fx_bic_r8 (void)
{
	FX_BIC(8);
}

static void fx_bic_r9 (void)
{
	FX_BIC(9);
}

static void fx_bic_r10 (void)
{
	FX_BIC(10);
}

static void fx_bic_r11 (void)
{
	FX_BIC(11);
}

static void fx_bic_r12 (void)
{
	FX_BIC(12);
}

static void fx_bic_r13 (void)
{
	FX_BIC(13);
}

static void fx_bic_r14 (void)
{
	FX_BIC(14);
}

static void fx_bic_r15 (void)
{
	FX_BIC(15);
}

// 71-7f (ALT2) - and #n - reister & immediate
#define FX_AND_I(imm) \
	uint32	v = SREG & imm; \
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_and_i1 (void)
{
	FX_AND_I(1);
}

static void fx_and_i2 (void)
{
	FX_AND_I(2);
}

static void fx_and_i3 (void)
{
	FX_AND_I(3);
}

static void fx_and_i4 (void)
{
	FX_AND_I(4);
}

static void fx_and_i5 (void)
{
	FX_AND_I(5);
}

static void fx_and_i6 (void)
{
	FX_AND_I(6);
}

static void fx_and_i7 (void)
{
	FX_AND_I(7);
}

static void fx_and_i8 (void)
{
	FX_AND_I(8);
}

static void fx_and_i9 (void)
{
	FX_AND_I(9);
}

static void fx_and_i10 (void)
{
	FX_AND_I(10);
}

static void fx_and_i11 (void)
{
	FX_AND_I(11);
}

static void fx_and_i12 (void)
{
	FX_AND_I(12);
}

static void fx_and_i13 (void)
{
	FX_AND_I(13);
}

static void fx_and_i14 (void)
{
	FX_AND_I(14);
}

static void fx_and_i15 (void)
{
	FX_AND_I(15);
}

// 71-7f (ALT3) - bic #n - reister & ~immediate
#define FX_BIC_I(imm) \
	uint32	v = SREG & ~imm; \
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_bic_i1 (void)
{
	FX_BIC_I(1);
}

static void fx_bic_i2 (void)
{
	FX_BIC_I(2);
}

static void fx_bic_i3 (void)
{
	FX_BIC_I(3);
}

static void fx_bic_i4 (void)
{
	FX_BIC_I(4);
}

static void fx_bic_i5 (void)
{
	FX_BIC_I(5);
}

static void fx_bic_i6 (void)
{
	FX_BIC_I(6);
}

static void fx_bic_i7 (void)
{
	FX_BIC_I(7);
}

static void fx_bic_i8 (void)
{
	FX_BIC_I(8);
}

static void fx_bic_i9 (void)
{
	FX_BIC_I(9);
}

static void fx_bic_i10 (void)
{
	FX_BIC_I(10);
}

static void fx_bic_i11 (void)
{
	FX_BIC_I(11);
}

static void fx_bic_i12 (void)
{
	FX_BIC_I(12);
}

static void fx_bic_i13 (void)
{
	FX_BIC_I(13);
}

static void fx_bic_i14 (void)
{
	FX_BIC_I(14);
}

static void fx_bic_i15 (void)
{
	FX_BIC_I(15);
}

// 80-8f - mult rn - 8 bit to 16 bit signed multiply, register * register
#define FX_MULT(reg) \
	uint32	v = (uint32) (SEX8(SREG) * SEX8(GSU.avReg[reg])); \
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_mult_r0 (void)
{
	FX_MULT(0);
}

static void fx_mult_r1 (void)
{
	FX_MULT(1);
}

static void fx_mult_r2 (void)
{
	FX_MULT(2);
}

static void fx_mult_r3 (void)
{
	FX_MULT(3);
}

static void fx_mult_r4 (void)
{
	FX_MULT(4);
}

static void fx_mult_r5 (void)
{
	FX_MULT(5);
}

static void fx_mult_r6 (void)
{
	FX_MULT(6);
}

static void fx_mult_r7 (void)
{
	FX_MULT(7);
}

static void fx_mult_r8 (void)
{
	FX_MULT(8);
}

static void fx_mult_r9 (void)
{
	FX_MULT(9);
}

static void fx_mult_r10 (void)
{
	FX_MULT(10);
}

static void fx_mult_r11 (void)
{
	FX_MULT(11);
}

static void fx_mult_r12 (void)
{
	FX_MULT(12);
}

static void fx_mult_r13 (void)
{
	FX_MULT(13);
}

static void fx_mult_r14 (void)
{
	FX_MULT(14);
}

static void fx_mult_r15 (void)
{
	FX_MULT(15);
}

// 80-8f (ALT1) - umult rn - 8 bit to 16 bit unsigned multiply, register * register
#define FX_UMULT(reg) \
	uint32	v = USEX8(SREG) * USEX8(GSU.avReg[reg]); \
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_umult_r0 (void)
{
	FX_UMULT(0);
}

static void fx_umult_r1 (void)
{
	FX_UMULT(1);
}

static void fx_umult_r2 (void)
{
	FX_UMULT(2);
}

static void fx_umult_r3 (void)
{
	FX_UMULT(3);
}

static void fx_umult_r4 (void)
{
	FX_UMULT(4);
}

static void fx_umult_r5 (void)
{
	FX_UMULT(5);
}

static void fx_umult_r6 (void)
{
	FX_UMULT(6);
}

static void fx_umult_r7 (void)
{
	FX_UMULT(7);
}

static void fx_umult_r8 (void)
{
	FX_UMULT(8);
}

static void fx_umult_r9 (void)
{
	FX_UMULT(9);
}

static void fx_umult_r10 (void)
{
	FX_UMULT(10);
}

static void fx_umult_r11 (void)
{
	FX_UMULT(11);
}

static void fx_umult_r12 (void)
{
	FX_UMULT(12);
}

static void fx_umult_r13 (void)
{
	FX_UMULT(13);
}

static void fx_umult_r14 (void)
{
	FX_UMULT(14);
}

static void fx_umult_r15 (void)
{
	FX_UMULT(15);
}

// 80-8f (ALT2) - mult #n - 8 bit to 16 bit signed multiply, register * immediate
#define FX_MULT_I(imm) \
	uint32	v = (uint32) (SEX8(SREG) * ((int32) imm)); \
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_mult_i0 (void)
{
	FX_MULT_I(0);
}

static void fx_mult_i1 (void)
{
	FX_MULT_I(1);
}

static void fx_mult_i2 (void)
{
	FX_MULT_I(2);
}

static void fx_mult_i3 (void)
{
	FX_MULT_I(3);
}

static void fx_mult_i4 (void)
{
	FX_MULT_I(4);
}

static void fx_mult_i5 (void)
{
	FX_MULT_I(5);
}

static void fx_mult_i6 (void)
{
	FX_MULT_I(6);
}

static void fx_mult_i7 (void)
{
	FX_MULT_I(7);
}

static void fx_mult_i8 (void)
{
	FX_MULT_I(8);
}

static void fx_mult_i9 (void)
{
	FX_MULT_I(9);
}

static void fx_mult_i10 (void)
{
	FX_MULT_I(10);
}

static void fx_mult_i11 (void)
{
	FX_MULT_I(11);
}

static void fx_mult_i12 (void)
{
	FX_MULT_I(12);
}

static void fx_mult_i13 (void)
{
	FX_MULT_I(13);
}

static void fx_mult_i14 (void)
{
	FX_MULT_I(14);
}

static void fx_mult_i15 (void)
{
	FX_MULT_I(15);
}

// 80-8f (ALT3) - umult #n - 8 bit to 16 bit unsigned multiply, register * immediate
#define FX_UMULT_I(imm) \
	uint32	v = USEX8(SREG) * ((uint32) imm); \
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_umult_i0 (void)
{
	FX_UMULT_I(0);
}

static void fx_umult_i1 (void)
{
	FX_UMULT_I(1);
}

static void fx_umult_i2 (void)
{
	FX_UMULT_I(2);
}

static void fx_umult_i3 (void)
{
	FX_UMULT_I(3);
}

static void fx_umult_i4 (void)
{
	FX_UMULT_I(4);
}

static void fx_umult_i5 (void)
{
	FX_UMULT_I(5);
}

static void fx_umult_i6 (void)
{
	FX_UMULT_I(6);
}

static void fx_umult_i7 (void)
{
	FX_UMULT_I(7);
}

static void fx_umult_i8 (void)
{
	FX_UMULT_I(8);
}

static void fx_umult_i9 (void)
{
	FX_UMULT_I(9);
}

static void fx_umult_i10 (void)
{
	FX_UMULT_I(10);
}

static void fx_umult_i11 (void)
{
	FX_UMULT_I(11);
}

static void fx_umult_i12 (void)
{
	FX_UMULT_I(12);
}

static void fx_umult_i13 (void)
{
	FX_UMULT_I(13);
}

static void fx_umult_i14 (void)
{
	FX_UMULT_I(14);
}

static void fx_umult_i15 (void)
{
	FX_UMULT_I(15);
}

// 90 - sbk - store word to last accessed RAM address
static void fx_sbk (void)
{
	RAM(GSU.vLastRamAdr) = (uint8) SREG;
	RAM(GSU.vLastRamAdr ^ 1) = (uint8) (SREG >> 8);
	CLRFLAGS;
	R15++;
}

// 91-94 - link #n - R11 = R15 + immediate
#define FX_LINK_I(lkn) \
	R11 = R15 + lkn; \
	CLRFLAGS; \
	R15++

static void fx_link_i1 (void)
{
	FX_LINK_I(1);
}

static void fx_link_i2 (void)
{
	FX_LINK_I(2);
}

static void fx_link_i3 (void)
{
	FX_LINK_I(3);
}

static void fx_link_i4 (void)
{
	FX_LINK_I(4);
}

// 95 - sex - sign extend 8 bit to 16 bit
static void fx_sex (void)
{
	uint32	v = (uint32) SEX8(SREG);
	R15++;
	DREG = v;
	GSU.vSign = v;
	GSU.vZero = v;
	TESTR14;
	CLRFLAGS;
}

// 96 - asr - aritmetric shift right by one
static void fx_asr (void)
{
	uint32	v;
	GSU.vCarry = SREG & 1;
	v = (uint32) (SEX16(SREG) >> 1);
	R15++;
	DREG = v;
	GSU.vSign = v;
	GSU.vZero = v;
	TESTR14;
	CLRFLAGS;
}

// 96 (ALT1) - div2 - aritmetric shift right by one
static void fx_div2 (void)
{
	uint32	v;
	int32	s = SEX16(SREG);
	GSU.vCarry = s & 1;
	if (s == -1)
		v = 0;
	else
		v = (uint32) (s >> 1);
	R15++;
	DREG = v;
	GSU.vSign = v;
	GSU.vZero = v;
	TESTR14;
	CLRFLAGS;
}

// 97 - ror - rotate right by one
static void fx_ror (void)
{
	uint32	v = (USEX16(SREG) >> 1) | (GSU.vCarry << 15);
	GSU.vCarry = SREG & 1;
	R15++;
	DREG = v;
	GSU.vSign = v;
	GSU.vZero = v;
	TESTR14;
	CLRFLAGS;
}

// 98-9d - jmp rn - jump to address of register
#define FX_JMP(reg) \
	R15 = GSU.avReg[reg]; \
	CLRFLAGS

static void fx_jmp_r8 (void)
{
	FX_JMP(8);
}

static void fx_jmp_r9 (void)
{
	FX_JMP(9);
}

static void fx_jmp_r10 (void)
{
	FX_JMP(10);
}

static void fx_jmp_r11 (void)
{
	FX_JMP(11);
}

static void fx_jmp_r12 (void)
{
	FX_JMP(12);
}

static void fx_jmp_r13 (void)
{
	FX_JMP(13);
}

// 98-9d (ALT1) - ljmp rn - set program bank to source register and jump to address of register
#define FX_LJMP(reg) \
	GSU.vPrgBankReg = GSU.avReg[reg] & 0x7f; \
	GSU.pvPrgBank = GSU.apvRomBank[GSU.vPrgBankReg]; \
	R15 = SREG; \
	GSU.bCacheActive = FALSE; \
	fx_cache(); \
	R15--

static void fx_ljmp_r8 (void)
{
	FX_LJMP(8);
}

static void fx_ljmp_r9 (void)
{
	FX_LJMP(9);
}

static void fx_ljmp_r10 (void)
{
	FX_LJMP(10);
}

static void fx_ljmp_r11 (void)
{
	FX_LJMP(11);
}

static void fx_ljmp_r12 (void)
{
	FX_LJMP(12);
}

static void fx_ljmp_r13 (void)
{
	FX_LJMP(13);
}

// 9e - lob - set upper byte to zero (keep low byte)
static void fx_lob (void)
{
	uint32	v = USEX8(SREG);
	R15++;
	DREG = v;
	GSU.vSign = v << 8;
	GSU.vZero = v << 8;
	TESTR14;
	CLRFLAGS;
}

// 9f - fmult - 16 bit to 32 bit signed multiplication, upper 16 bits only
static void fx_fmult (void)
{
	uint32	v;
	uint32	c = (uint32) (SEX16(SREG) * SEX16(R6));
	v = c >> 16;
	R15++;
	DREG = v;
	GSU.vSign = v;
	GSU.vZero = v;
	GSU.vCarry = (c >> 15) & 1;
	TESTR14;
	CLRFLAGS;
}

// 9f (ALT1) - lmult - 16 bit to 32 bit signed multiplication
static void fx_lmult (void)
{
	uint32	v;
	uint32	c = (uint32) (SEX16(SREG) * SEX16(R6));
	R4 = c;
	v = c >> 16;
	R15++;
	DREG = v;
	GSU.vSign = v;
	GSU.vZero = v;
	// XXX: R6 or R4?
	GSU.vCarry = (R4 >> 15) & 1; // should it be bit 15 of R4 instead ?
	TESTR14;
	CLRFLAGS;
}

// a0-af - ibt rn, #pp - immediate byte transfer
#define FX_IBT(reg) \
	uint8	v = PIPE; \
	R15++; \
	FETCHPIPE; \
	R15++; \
	GSU.avReg[reg] = SEX8(v); \
	CLRFLAGS

static void fx_ibt_r0 (void)
{
	FX_IBT(0);
}

static void fx_ibt_r1 (void)
{
	FX_IBT(1);
}

static void fx_ibt_r2 (void)
{
	FX_IBT(2);
}

static void fx_ibt_r3 (void)
{
	FX_IBT(3);
}

static void fx_ibt_r4 (void)
{
	FX_IBT(4);
}

static void fx_ibt_r5 (void)
{
	FX_IBT(5);
}

static void fx_ibt_r6 (void)
{
	FX_IBT(6);
}

static void fx_ibt_r7 (void)
{
	FX_IBT(7);
}

static void fx_ibt_r8 (void)
{
	FX_IBT(8);
}

static void fx_ibt_r9 (void)
{
	FX_IBT(9);
}

static void fx_ibt_r10 (void)
{
	FX_IBT(10);
}

static void fx_ibt_r11 (void)
{
	FX_IBT(11);
}

static void fx_ibt_r12 (void)
{
	FX_IBT(12);
}

static void fx_ibt_r13 (void)
{
	FX_IBT(13);
}

static void fx_ibt_r14 (void)
{
	FX_IBT(14);
	READR14;
}

static void fx_ibt_r15 (void)
{
	FX_IBT(15);
}

// a0-af (ALT1) - lms rn, (yy) - load word from RAM (short address)
#define FX_LMS(reg) \
	GSU.vLastRamAdr = ((uint32) PIPE) << 1; \
	R15++; \
	FETCHPIPE; \
	R15++; \
	GSU.avReg[reg] = (uint32) RAM(GSU.vLastRamAdr); \
	GSU.avReg[reg] |= ((uint32) RAM(GSU.vLastRamAdr + 1)) << 8; \
	CLRFLAGS

static void fx_lms_r0 (void)
{
	FX_LMS(0);
}

static void fx_lms_r1 (void)
{
	FX_LMS(1);
}

static void fx_lms_r2 (void)
{
	FX_LMS(2);
}

static void fx_lms_r3 (void)
{
	FX_LMS(3);
}

static void fx_lms_r4 (void)
{
	FX_LMS(4);
}

static void fx_lms_r5 (void)
{
	FX_LMS(5);
}

static void fx_lms_r6 (void)
{
	FX_LMS(6);
}

static void fx_lms_r7 (void)
{
	FX_LMS(7);
}

static void fx_lms_r8 (void)
{
	FX_LMS(8);
}

static void fx_lms_r9 (void)
{
	FX_LMS(9);
}

static void fx_lms_r10 (void)
{
	FX_LMS(10);
}

static void fx_lms_r11 (void)
{
	FX_LMS(11);
}

static void fx_lms_r12 (void)
{
	FX_LMS(12);
}

static void fx_lms_r13 (void)
{
	FX_LMS(13);
}

static void fx_lms_r14 (void)
{
	FX_LMS(14);
	READR14;
}

static void fx_lms_r15 (void)
{
	FX_LMS(15);
}

// a0-af (ALT2) - sms (yy), rn - store word in RAM (short address)
// XXX: If rn == r15, is the value of r15 before or after the extra byte is read ?
#define FX_SMS(reg) \
	uint32	v = GSU.avReg[reg]; \
	GSU.vLastRamAdr = ((uint32) PIPE) << 1; \
	R15++; \
	FETCHPIPE; \
	RAM(GSU.vLastRamAdr) = (uint8) v; \
	RAM(GSU.vLastRamAdr + 1) = (uint8) (v >> 8); \
	CLRFLAGS; \
	R15++

static void fx_sms_r0 (void)
{
	FX_SMS(0);
}

static void fx_sms_r1 (void)
{
	FX_SMS(1);
}

static void fx_sms_r2 (void)
{
	FX_SMS(2);
}

static void fx_sms_r3 (void)
{
	FX_SMS(3);
}

static void fx_sms_r4 (void)
{
	FX_SMS(4);
}

static void fx_sms_r5 (void)
{
	FX_SMS(5);
}

static void fx_sms_r6 (void)
{
	FX_SMS(6);
}

static void fx_sms_r7 (void)
{
	FX_SMS(7);
}

static void fx_sms_r8 (void)
{
	FX_SMS(8);
}

static void fx_sms_r9 (void)
{
	FX_SMS(9);
}

static void fx_sms_r10 (void)
{
	FX_SMS(10);
}

static void fx_sms_r11 (void)
{
	FX_SMS(11);
}

static void fx_sms_r12 (void)
{
	FX_SMS(12);
}

static void fx_sms_r13 (void)
{
	FX_SMS(13);
}

static void fx_sms_r14 (void)
{
	FX_SMS(14);
}

static void fx_sms_r15 (void)
{
	FX_SMS(15);
}

// b0-bf - from rn - set source register
// b0-bf (B) - moves rn - move register to register, and set flags, (if B flag is set)
#define FX_FROM(reg) \
	if (TF(B)) \
	{ \
		uint32	v = GSU.avReg[reg]; \
		R15++; \
		DREG = v; \
		GSU.vOverflow = (v & 0x80) << 16; \
		GSU.vSign = v; \
		GSU.vZero = v; \
		TESTR14; \
		CLRFLAGS; \
	} \
	else \
	{ \
		GSU.pvSreg = &GSU.avReg[reg]; \
		R15++; \
	}

static void fx_from_r0 (void)
{
	FX_FROM(0);
}

static void fx_from_r1 (void)
{
	FX_FROM(1);
}

static void fx_from_r2 (void)
{
	FX_FROM(2);
}

static void fx_from_r3 (void)
{
	FX_FROM(3);
}

static void fx_from_r4 (void)
{
	FX_FROM(4);
}

static void fx_from_r5 (void)
{
	FX_FROM(5);
}

static void fx_from_r6 (void)
{
	FX_FROM(6);
}

static void fx_from_r7 (void)
{
	FX_FROM(7);
}

static void fx_from_r8 (void)
{
	FX_FROM(8);
}

static void fx_from_r9 (void)
{
	FX_FROM(9);
}

static void fx_from_r10 (void)
{
	FX_FROM(10);
}

static void fx_from_r11 (void)
{
	FX_FROM(11);
}

static void fx_from_r12 (void)
{
	FX_FROM(12);
}

static void fx_from_r13 (void)
{
	FX_FROM(13);
}

static void fx_from_r14 (void)
{
	FX_FROM(14);
}

static void fx_from_r15 (void)
{
	FX_FROM(15);
}

// c0 - hib - move high-byte to low-byte
static void fx_hib (void)
{
	uint32	v = USEX8(SREG >> 8);
	R15++;
	DREG = v;
	GSU.vSign = v << 8;
	GSU.vZero = v << 8;
	TESTR14;
	CLRFLAGS;
}

// c1-cf - or rn
#define FX_OR(reg) \
	uint32	v = SREG | GSU.avReg[reg]; \
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_or_r1 (void)
{
	FX_OR(1);
}

static void fx_or_r2 (void)
{
	FX_OR(2);
}

static void fx_or_r3 (void)
{
	FX_OR(3);
}

static void fx_or_r4 (void)
{
	FX_OR(4);
}

static void fx_or_r5 (void)
{
	FX_OR(5);
}

static void fx_or_r6 (void)
{
	FX_OR(6);
}

static void fx_or_r7 (void)
{
	FX_OR(7);
}

static void fx_or_r8 (void)
{
	FX_OR(8);
}

static void fx_or_r9 (void)
{
	FX_OR(9);
}

static void fx_or_r10 (void)
{
	FX_OR(10);
}

static void fx_or_r11 (void)
{
	FX_OR(11);
}

static void fx_or_r12 (void)
{
	FX_OR(12);
}

static void fx_or_r13 (void)
{
	FX_OR(13);
}

static void fx_or_r14 (void)
{
	FX_OR(14);
}

static void fx_or_r15 (void)
{
	FX_OR(15);
}

// c1-cf (ALT1) - xor rn
#define FX_XOR(reg) \
	uint32	v = SREG ^ GSU.avReg[reg]; \
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_xor_r1 (void)
{
	FX_XOR(1);
}

static void fx_xor_r2 (void)
{
	FX_XOR(2);
}

static void fx_xor_r3 (void)
{
	FX_XOR(3);
}

static void fx_xor_r4 (void)
{
	FX_XOR(4);
}

static void fx_xor_r5 (void)
{
	FX_XOR(5);
}

static void fx_xor_r6 (void)
{
	FX_XOR(6);
}

static void fx_xor_r7 (void)
{
	FX_XOR(7);
}

static void fx_xor_r8 (void)
{
	FX_XOR(8);
}

static void fx_xor_r9 (void)
{
	FX_XOR(9);
}

static void fx_xor_r10 (void)
{
	FX_XOR(10);
}

static void fx_xor_r11 (void)
{
	FX_XOR(11);
}

static void fx_xor_r12 (void)
{
	FX_XOR(12);
}

static void fx_xor_r13 (void)
{
	FX_XOR(13);
}

static void fx_xor_r14 (void)
{
	FX_XOR(14);
}

static void fx_xor_r15 (void)
{
	FX_XOR(15);
}

// c1-cf (ALT2) - or #n
#define FX_OR_I(imm) \
	uint32	v = SREG | imm; \
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_or_i1 (void)
{
	FX_OR_I(1);
}

static void fx_or_i2 (void)
{
	FX_OR_I(2);
}

static void fx_or_i3 (void)
{
	FX_OR_I(3);
}

static void fx_or_i4 (void)
{
	FX_OR_I(4);
}

static void fx_or_i5 (void)
{
	FX_OR_I(5);
}

static void fx_or_i6 (void)
{
	FX_OR_I(6);
}

static void fx_or_i7 (void)
{
	FX_OR_I(7);
}

static void fx_or_i8 (void)
{
	FX_OR_I(8);
}

static void fx_or_i9 (void)
{
	FX_OR_I(9);
}

static void fx_or_i10 (void)
{
	FX_OR_I(10);
}

static void fx_or_i11 (void)
{
	FX_OR_I(11);
}

static void fx_or_i12 (void)
{
	FX_OR_I(12);
}

static void fx_or_i13 (void)
{
	FX_OR_I(13);
}

static void fx_or_i14 (void)
{
	FX_OR_I(14);
}

static void fx_or_i15 (void)
{
	FX_OR_I(15);
}

// c1-cf (ALT3) - xor #n
#define FX_XOR_I(imm) \
	uint32	v = SREG ^ imm; \
	R15++; \
	DREG = v; \
	GSU.vSign = v; \
	GSU.vZero = v; \
	TESTR14; \
	CLRFLAGS

static void fx_xor_i1 (void)
{
	FX_XOR_I(1);
}

static void fx_xor_i2 (void)
{
	FX_XOR_I(2);
}

static void fx_xor_i3 (void)
{
	FX_XOR_I(3);
}

static void fx_xor_i4 (void)
{
	FX_XOR_I(4);
}

static void fx_xor_i5 (void)
{
	FX_XOR_I(5);
}

static void fx_xor_i6 (void)
{
	FX_XOR_I(6);
}

static void fx_xor_i7 (void)
{
	FX_XOR_I(7);
}

static void fx_xor_i8 (void)
{
	FX_XOR_I(8);
}

static void fx_xor_i9 (void)
{
	FX_XOR_I(9);
}

static void fx_xor_i10 (void)
{
	FX_XOR_I(10);
}

static void fx_xor_i11 (void)
{
	FX_XOR_I(11);
}

static void fx_xor_i12 (void)
{
	FX_XOR_I(12);
}

static void fx_xor_i13 (void)
{
	FX_XOR_I(13);
}

static void fx_xor_i14 (void)
{
	FX_XOR_I(14);
}

static void fx_xor_i15 (void)
{
	FX_XOR_I(15);
}

// d0-de - inc rn - increase by one
#define FX_INC(reg) \
	GSU.avReg[reg] += 1; \
	GSU.vSign = GSU.avReg[reg]; \
	GSU.vZero = GSU.avReg[reg]; \
	CLRFLAGS; \
	R15++

static void fx_inc_r0 (void)
{
	FX_INC(0);
}

static void fx_inc_r1 (void)
{
	FX_INC(1);
}

static void fx_inc_r2 (void)
{
	FX_INC(2);
}

static void fx_inc_r3 (void)
{
	FX_INC(3);
}

static void fx_inc_r4 (void)
{
	FX_INC(4);
}

static void fx_inc_r5 (void)
{
	FX_INC(5);
}

static void fx_inc_r6 (void)
{
	FX_INC(6);
}

static void fx_inc_r7 (void)
{
	FX_INC(7);
}

static void fx_inc_r8 (void)
{
	FX_INC(8);
}

static void fx_inc_r9 (void)
{
	FX_INC(9);
}

static void fx_inc_r10 (void)
{
	FX_INC(10);
}

static void fx_inc_r11 (void)
{
	FX_INC(11);
}

static void fx_inc_r12 (void)
{
	FX_INC(12);
}

static void fx_inc_r13 (void)
{
	FX_INC(13);
}

static void fx_inc_r14 (void)
{
	FX_INC(14);
	READR14;
}

// df - getc - transfer ROM buffer to color register
static void fx_getc (void)
{
#ifndef FX_DO_ROMBUFFER
	uint8	c = ROM(R14);
#else
	uint8	c = GSU.vRomBuffer;
#endif

	if (GSU.vPlotOptionReg & 0x04)
		c = (c & 0xf0) | (c >> 4);

	if (GSU.vPlotOptionReg & 0x08)
	{
		GSU.vColorReg &= 0xf0;
		GSU.vColorReg |= c & 0x0f;
	}
	else
		GSU.vColorReg = USEX8(c);

	CLRFLAGS;
	R15++;
}

// df (ALT2) - ramb - set current RAM bank
static void fx_ramb (void)
{
	GSU.vRamBankReg = SREG & (FX_RAM_BANKS - 1);
	GSU.pvRamBank = GSU.apvRamBank[GSU.vRamBankReg & 0x3];
	CLRFLAGS;
	R15++;
}

// df (ALT3) - romb - set current ROM bank
static void fx_romb (void)
{
	GSU.vRomBankReg = USEX8(SREG) & 0x7f;
	GSU.pvRomBank = GSU.apvRomBank[GSU.vRomBankReg];
	CLRFLAGS;
	R15++;
}

// e0-ee - dec rn - decrement by one
#define FX_DEC(reg) \
	GSU.avReg[reg] -= 1; \
	GSU.vSign = GSU.avReg[reg]; \
	GSU.vZero = GSU.avReg[reg]; \
	CLRFLAGS; \
	R15++

static void fx_dec_r0 (void)
{
	FX_DEC(0);
}

static void fx_dec_r1 (void)
{
	FX_DEC(1);
}

static void fx_dec_r2 (void)
{
	FX_DEC(2);
}

static void fx_dec_r3 (void)
{
	FX_DEC(3);
}

static void fx_dec_r4 (void)
{
	FX_DEC(4);
}

static void fx_dec_r5 (void)
{
	FX_DEC(5);
}

static void fx_dec_r6 (void)
{
	FX_DEC(6);
}

static void fx_dec_r7 (void)
{
	FX_DEC(7);
}

static void fx_dec_r8 (void)
{
	FX_DEC(8);
}

static void fx_dec_r9 (void)
{
	FX_DEC(9);
}

static void fx_dec_r10 (void)
{
	FX_DEC(10);
}

static void fx_dec_r11 (void)
{
	FX_DEC(11);
}

static void fx_dec_r12 (void)
{
	FX_DEC(12);
}

static void fx_dec_r13 (void)
{
	FX_DEC(13);
}

static void fx_dec_r14 (void)
{
	FX_DEC(14);
	READR14;
}

// ef - getb - get byte from ROM at address R14
static void fx_getb (void)
{
	uint32	v;
#ifndef FX_DO_ROMBUFFER
	v = (uint32) ROM(R14);
#else
	v = (uint32) GSU.vRomBuffer;
#endif
	R15++;
	DREG = v;
	TESTR14;
	CLRFLAGS;
}

// ef (ALT1) - getbh - get high-byte from ROM at address R14
static void fx_getbh (void)
{
	uint32	v;
#ifndef FX_DO_ROMBUFFER
	uint32	c = (uint32) ROM(R14);
#else
	uint32	c = USEX8(GSU.vRomBuffer);
#endif
	v = USEX8(SREG) | (c << 8);
	R15++;
	DREG = v;
	TESTR14;
	CLRFLAGS;
}

// ef (ALT2) - getbl - get low-byte from ROM at address R14
static void fx_getbl (void)
{
	uint32	v;
#ifndef FX_DO_ROMBUFFER
	uint32	c = (uint32) ROM(R14);
#else
	uint32	c = USEX8(GSU.vRomBuffer);
#endif
	v = (SREG & 0xff00) | c;
	R15++;
	DREG = v;
	TESTR14;
	CLRFLAGS;
}

// ef (ALT3) - getbs - get sign extended byte from ROM at address R14
static void fx_getbs (void)
{
	uint32	v;
#ifndef FX_DO_ROMBUFFER
	int8	c;
	c = ROM(R14);
	v = SEX8(c);
#else
	v = SEX8(GSU.vRomBuffer);
#endif
	R15++;
	DREG = v;
	TESTR14;
	CLRFLAGS;
}

// f0-ff - iwt rn, #xx - immediate word transfer to register
#define FX_IWT(reg) \
	uint32	v = PIPE; \
	R15++; \
	FETCHPIPE; \
	R15++; \
	v |= USEX8(PIPE) << 8; \
	FETCHPIPE; \
	R15++; \
	GSU.avReg[reg] = v; \
	CLRFLAGS

static void fx_iwt_r0 (void)
{
	FX_IWT(0);
}

static void fx_iwt_r1 (void)
{
	FX_IWT(1);
}

static void fx_iwt_r2 (void)
{
	FX_IWT(2);
}

static void fx_iwt_r3 (void)
{
	FX_IWT(3);
}

static void fx_iwt_r4 (void)
{
	FX_IWT(4);
}

static void fx_iwt_r5 (void)
{
	FX_IWT(5);
}

static void fx_iwt_r6 (void)
{
	FX_IWT(6);
}

static void fx_iwt_r7 (void)
{
	FX_IWT(7);
}

static void fx_iwt_r8 (void)
{
	FX_IWT(8);
}

static void fx_iwt_r9 (void)
{
	FX_IWT(9);
}

static void fx_iwt_r10 (void)
{
	FX_IWT(10);
}

static void fx_iwt_r11 (void)
{
	FX_IWT(11);
}

static void fx_iwt_r12 (void)
{
	FX_IWT(12);
}

static void fx_iwt_r13 (void)
{
	FX_IWT(13);
}

static void fx_iwt_r14 (void)
{
	FX_IWT(14);
	READR14;
}

static void fx_iwt_r15 (void)
{
	FX_IWT(15);
}

// f0-ff (ALT1) - lm rn, (xx) - load word from RAM
#define FX_LM(reg) \
	GSU.vLastRamAdr = PIPE; \
	R15++; \
	FETCHPIPE; \
	R15++; \
	GSU.vLastRamAdr |= USEX8(PIPE) << 8; \
	FETCHPIPE; \
	R15++; \
	GSU.avReg[reg] = RAM(GSU.vLastRamAdr); \
	GSU.avReg[reg] |= USEX8(RAM(GSU.vLastRamAdr ^ 1)) << 8; \
	CLRFLAGS

static void fx_lm_r0 (void)
{
	FX_LM(0);
}

static void fx_lm_r1 (void)
{
	FX_LM(1);
}

static void fx_lm_r2 (void)
{
	FX_LM(2);
}

static void fx_lm_r3 (void)
{
	FX_LM(3);
}

static void fx_lm_r4 (void)
{
	FX_LM(4);
}

static void fx_lm_r5 (void)
{
	FX_LM(5);
}

static void fx_lm_r6 (void)
{
	FX_LM(6);
}

static void fx_lm_r7 (void)
{
	FX_LM(7);
}

static void fx_lm_r8 (void)
{
	FX_LM(8);
}

static void fx_lm_r9 (void)
{
	FX_LM(9);
}

static void fx_lm_r10 (void)
{
	FX_LM(10);
}

static void fx_lm_r11 (void)
{
	FX_LM(11);
}

static void fx_lm_r12 (void)
{
	FX_LM(12);
}

static void fx_lm_r13 (void)
{
	FX_LM(13);
}

static void fx_lm_r14 (void)
{
	FX_LM(14);
	READR14;
}

static void fx_lm_r15 (void)
{
	FX_LM(15);
}

// f0-ff (ALT2) - sm (xx), rn - store word in RAM
// XXX: If rn == r15, is the value of r15 before or after the extra bytes are read ?
#define FX_SM(reg) \
	uint32	v = GSU.avReg[reg]; \
	GSU.vLastRamAdr = PIPE; \
	R15++; \
	FETCHPIPE; \
	R15++; \
	GSU.vLastRamAdr |= USEX8(PIPE) << 8; \
	FETCHPIPE; \
	RAM(GSU.vLastRamAdr) = (uint8) v; \
	RAM(GSU.vLastRamAdr ^ 1) = (uint8) (v >> 8); \
	CLRFLAGS; \
	R15++

static void fx_sm_r0 (void)
{
	FX_SM(0);
}

static void fx_sm_r1 (void)
{
	FX_SM(1);
}

static void fx_sm_r2 (void)
{
	FX_SM(2);
}

static void fx_sm_r3 (void)
{
	FX_SM(3);
}

static void fx_sm_r4 (void)
{
	FX_SM(4);
}

static void fx_sm_r5 (void)
{
	FX_SM(5);
}

static void fx_sm_r6 (void)
{
	FX_SM(6);
}

static void fx_sm_r7 (void)
{
	FX_SM(7);
}

static void fx_sm_r8 (void)
{
	FX_SM(8);
}

static void fx_sm_r9 (void)
{
	FX_SM(9);
}

static void fx_sm_r10 (void)
{
	FX_SM(10);
}

static void fx_sm_r11 (void)
{
	FX_SM(11);
}

static void fx_sm_r12 (void)
{
	FX_SM(12);
}

static void fx_sm_r13 (void)
{
	FX_SM(13);
}

static void fx_sm_r14 (void)
{
	FX_SM(14);
}

static void fx_sm_r15 (void)
{
	FX_SM(15);
}

// GSU executions functions

uint32 fx_run (uint32 nInstructions)
{
	GSU.vCounter = nInstructions;
	READR14;
	while (TF(G) && (GSU.vCounter-- > 0))
		FX_STEP;
#if 0
#ifndef FX_ADDRESS_CHECK
	GSU.vPipeAdr = USEX16(R15 - 1) | (USEX8(GSU.vPrgBankReg) << 16);
#endif
#endif

	return (nInstructions - GSU.vInstCount);
}

/*
uint32 fx_run_to_breakpoint (uint32 nInstructions)
{
	uint32	vCounter = 0;

	while (TF(G) && vCounter < nInstructions)
	{
		vCounter++;
		FX_STEP;

		if (USEX16(R15) == GSU.vBreakPoint)
		{
			GSU.vErrorCode = FX_BREAKPOINT;
			break;
		}
	}

#if 0
#ifndef FX_ADDRESS_CHECK
	GSU.vPipeAdr = USEX16(R15 - 1) | (USEX8(GSU.vPrgBankReg) << 16);
#endif
#endif

	return (vCounter);
}
*/

/*
uint32 fx_step_over (uint32 nInstructions)
{
	uint32	vCounter = 0;

	while (TF(G) && vCounter < nInstructions)
	{
		vCounter++;
		FX_STEP;

		if (USEX16(R15) == GSU.vBreakPoint)
		{
			GSU.vErrorCode = FX_BREAKPOINT;
			break;
		}

		if (USEX16(R15) == GSU.vStepPoint)
			break;
	}

#if 0
#ifndef FX_ADDRESS_CHECK
	GSU.vPipeAdr = USEX16(R15 - 1) | (USEX8(GSU.vPrgBankReg) << 16);
#endif
#endif

	return (vCounter);
}
*/

// Special table for the different plot configurations

void (*fx_PlotTable[]) (void) =
{
	&fx_plot_2bit, &fx_plot_4bit, &fx_plot_4bit, &fx_plot_8bit, &fx_plot_obj,
	&fx_rpix_2bit, &fx_rpix_4bit, &fx_rpix_4bit, &fx_rpix_8bit, &fx_rpix_obj
};

// Opcode table

void (*fx_OpcodeTable[]) (void) =
{
	// ALT0 Table

	// 00 - 0f
	&fx_stop,      &fx_nop,       &fx_cache,     &fx_lsr,       &fx_rol,       &fx_bra,       &fx_bge,       &fx_blt,
	&fx_bne,       &fx_beq,       &fx_bpl,       &fx_bmi,       &fx_bcc,       &fx_bcs,       &fx_bvc,       &fx_bvs,
	// 10 - 1f
	&fx_to_r0,     &fx_to_r1,     &fx_to_r2,     &fx_to_r3,     &fx_to_r4,     &fx_to_r5,     &fx_to_r6,     &fx_to_r7,
	&fx_to_r8,     &fx_to_r9,     &fx_to_r10,    &fx_to_r11,    &fx_to_r12,    &fx_to_r13,    &fx_to_r14,    &fx_to_r15,
	// 20 - 2f
	&fx_with_r0,   &fx_with_r1,   &fx_with_r2,   &fx_with_r3,   &fx_with_r4,   &fx_with_r5,   &fx_with_r6,   &fx_with_r7,
	&fx_with_r8,   &fx_with_r9,   &fx_with_r10,  &fx_with_r11,  &fx_with_r12,  &fx_with_r13,  &fx_with_r14,  &fx_with_r15,
	// 30 - 3f
	&fx_stw_r0,    &fx_stw_r1,    &fx_stw_r2,    &fx_stw_r3,    &fx_stw_r4,    &fx_stw_r5,    &fx_stw_r6,    &fx_stw_r7,
	&fx_stw_r8,    &fx_stw_r9,    &fx_stw_r10,   &fx_stw_r11,   &fx_loop,      &fx_alt1,      &fx_alt2,      &fx_alt3,
	// 40 - 4f
	&fx_ldw_r0,    &fx_ldw_r1,    &fx_ldw_r2,    &fx_ldw_r3,    &fx_ldw_r4,    &fx_ldw_r5,    &fx_ldw_r6,    &fx_ldw_r7,
	&fx_ldw_r8,    &fx_ldw_r9,    &fx_ldw_r10,   &fx_ldw_r11,   &fx_plot_2bit, &fx_swap,      &fx_color,     &fx_not,
	// 50 - 5f
	&fx_add_r0,    &fx_add_r1,    &fx_add_r2,    &fx_add_r3,    &fx_add_r4,    &fx_add_r5,    &fx_add_r6,    &fx_add_r7,
	&fx_add_r8,    &fx_add_r9,    &fx_add_r10,   &fx_add_r11,   &fx_add_r12,   &fx_add_r13,   &fx_add_r14,   &fx_add_r15,
	// 60 - 6f
	&fx_sub_r0,    &fx_sub_r1,    &fx_sub_r2,    &fx_sub_r3,    &fx_sub_r4,    &fx_sub_r5,    &fx_sub_r6,    &fx_sub_r7,
	&fx_sub_r8,    &fx_sub_r9,    &fx_sub_r10,   &fx_sub_r11,   &fx_sub_r12,   &fx_sub_r13,   &fx_sub_r14,   &fx_sub_r15,
	// 70 - 7f
	&fx_merge,     &fx_and_r1,    &fx_and_r2,    &fx_and_r3,    &fx_and_r4,    &fx_and_r5,    &fx_and_r6,    &fx_and_r7,
	&fx_and_r8,    &fx_and_r9,    &fx_and_r10,   &fx_and_r11,   &fx_and_r12,   &fx_and_r13,   &fx_and_r14,   &fx_and_r15,
	// 80 - 8f
	&fx_mult_r0,   &fx_mult_r1,   &fx_mult_r2,   &fx_mult_r3,   &fx_mult_r4,   &fx_mult_r5,   &fx_mult_r6,   &fx_mult_r7,
	&fx_mult_r8,   &fx_mult_r9,   &fx_mult_r10,  &fx_mult_r11,  &fx_mult_r12,  &fx_mult_r13,  &fx_mult_r14,  &fx_mult_r15,
	// 90 - 9f
	&fx_sbk,       &fx_link_i1,   &fx_link_i2,   &fx_link_i3,   &fx_link_i4,   &fx_sex,       &fx_asr,       &fx_ror,
	&fx_jmp_r8,    &fx_jmp_r9,    &fx_jmp_r10,   &fx_jmp_r11,   &fx_jmp_r12,   &fx_jmp_r13,   &fx_lob,       &fx_fmult,
	// a0 - af
	&fx_ibt_r0,    &fx_ibt_r1,    &fx_ibt_r2,    &fx_ibt_r3,    &fx_ibt_r4,    &fx_ibt_r5,    &fx_ibt_r6,    &fx_ibt_r7,
	&fx_ibt_r8,    &fx_ibt_r9,    &fx_ibt_r10,   &fx_ibt_r11,   &fx_ibt_r12,   &fx_ibt_r13,   &fx_ibt_r14,   &fx_ibt_r15,
	// b0 - bf
	&fx_from_r0,   &fx_from_r1,   &fx_from_r2,   &fx_from_r3,   &fx_from_r4,   &fx_from_r5,   &fx_from_r6,   &fx_from_r7,
	&fx_from_r8,   &fx_from_r9,   &fx_from_r10,  &fx_from_r11,  &fx_from_r12,  &fx_from_r13,  &fx_from_r14,  &fx_from_r15,
	// c0 - cf
	&fx_hib,       &fx_or_r1,     &fx_or_r2,     &fx_or_r3,     &fx_or_r4,     &fx_or_r5,     &fx_or_r6,     &fx_or_r7,
	&fx_or_r8,     &fx_or_r9,     &fx_or_r10,    &fx_or_r11,    &fx_or_r12,    &fx_or_r13,    &fx_or_r14,    &fx_or_r15,
	// d0 - df
	&fx_inc_r0,    &fx_inc_r1,    &fx_inc_r2,    &fx_inc_r3,    &fx_inc_r4,    &fx_inc_r5,    &fx_inc_r6,    &fx_inc_r7,
	&fx_inc_r8,    &fx_inc_r9,    &fx_inc_r10,   &fx_inc_r11,   &fx_inc_r12,   &fx_inc_r13,   &fx_inc_r14,   &fx_getc,
	// e0 - ef
	&fx_dec_r0,    &fx_dec_r1,    &fx_dec_r2,    &fx_dec_r3,    &fx_dec_r4,    &fx_dec_r5,    &fx_dec_r6,    &fx_dec_r7,
	&fx_dec_r8,    &fx_dec_r9,    &fx_dec_r10,   &fx_dec_r11,   &fx_dec_r12,   &fx_dec_r13,   &fx_dec_r14,   &fx_getb,
	// f0 - ff
	&fx_iwt_r0,    &fx_iwt_r1,    &fx_iwt_r2,    &fx_iwt_r3,    &fx_iwt_r4,    &fx_iwt_r5,    &fx_iwt_r6,    &fx_iwt_r7,
	&fx_iwt_r8,    &fx_iwt_r9,    &fx_iwt_r10,   &fx_iwt_r11,   &fx_iwt_r12,   &fx_iwt_r13,   &fx_iwt_r14,   &fx_iwt_r15,

	// ALT1 Table

	// 00 - 0f
	&fx_stop,      &fx_nop,       &fx_cache,     &fx_lsr,       &fx_rol,       &fx_bra,       &fx_bge,       &fx_blt,
	&fx_bne,       &fx_beq,       &fx_bpl,       &fx_bmi,       &fx_bcc,       &fx_bcs,       &fx_bvc,       &fx_bvs,
	// 10 - 1f
	&fx_to_r0,     &fx_to_r1,     &fx_to_r2,     &fx_to_r3,     &fx_to_r4,     &fx_to_r5,     &fx_to_r6,     &fx_to_r7,
	&fx_to_r8,     &fx_to_r9,     &fx_to_r10,    &fx_to_r11,    &fx_to_r12,    &fx_to_r13,    &fx_to_r14,    &fx_to_r15,
	// 20 - 2f
	&fx_with_r0,   &fx_with_r1,   &fx_with_r2,   &fx_with_r3,   &fx_with_r4,   &fx_with_r5,   &fx_with_r6,   &fx_with_r7,
	&fx_with_r8,   &fx_with_r9,   &fx_with_r10,  &fx_with_r11,  &fx_with_r12,  &fx_with_r13,  &fx_with_r14,  &fx_with_r15,
	// 30 - 3f
	&fx_stb_r0,    &fx_stb_r1,    &fx_stb_r2,    &fx_stb_r3,    &fx_stb_r4,    &fx_stb_r5,    &fx_stb_r6,    &fx_stb_r7,
	&fx_stb_r8,    &fx_stb_r9,    &fx_stb_r10,   &fx_stb_r11,   &fx_loop,      &fx_alt1,      &fx_alt2,      &fx_alt3,
	// 40 - 4f
	&fx_ldb_r0,    &fx_ldb_r1,    &fx_ldb_r2,    &fx_ldb_r3,    &fx_ldb_r4,    &fx_ldb_r5,    &fx_ldb_r6,    &fx_ldb_r7,
	&fx_ldb_r8,    &fx_ldb_r9,    &fx_ldb_r10,   &fx_ldb_r11,   &fx_rpix_2bit, &fx_swap,      &fx_cmode,     &fx_not,
	// 50 - 5f
	&fx_adc_r0,    &fx_adc_r1,    &fx_adc_r2,    &fx_adc_r3,    &fx_adc_r4,    &fx_adc_r5,    &fx_adc_r6,    &fx_adc_r7,
	&fx_adc_r8,    &fx_adc_r9,    &fx_adc_r10,   &fx_adc_r11,   &fx_adc_r12,   &fx_adc_r13,   &fx_adc_r14,   &fx_adc_r15,
	// 60 - 6f
	&fx_sbc_r0,    &fx_sbc_r1,    &fx_sbc_r2,    &fx_sbc_r3,    &fx_sbc_r4,    &fx_sbc_r5,    &fx_sbc_r6,    &fx_sbc_r7,
	&fx_sbc_r8,    &fx_sbc_r9,    &fx_sbc_r10,   &fx_sbc_r11,   &fx_sbc_r12,   &fx_sbc_r13,   &fx_sbc_r14,   &fx_sbc_r15,
	// 70 - 7f
	&fx_merge,     &fx_bic_r1,    &fx_bic_r2,    &fx_bic_r3,    &fx_bic_r4,    &fx_bic_r5,    &fx_bic_r6,    &fx_bic_r7,
	&fx_bic_r8,    &fx_bic_r9,    &fx_bic_r10,   &fx_bic_r11,   &fx_bic_r12,   &fx_bic_r13,   &fx_bic_r14,   &fx_bic_r15,
	// 80 - 8f
	&fx_umult_r0,  &fx_umult_r1,  &fx_umult_r2,  &fx_umult_r3,  &fx_umult_r4,  &fx_umult_r5,  &fx_umult_r6,  &fx_umult_r7,
	&fx_umult_r8,  &fx_umult_r9,  &fx_umult_r10, &fx_umult_r11, &fx_umult_r12, &fx_umult_r13, &fx_umult_r14, &fx_umult_r15,
	// 90 - 9f
	&fx_sbk,       &fx_link_i1,   &fx_link_i2,   &fx_link_i3,   &fx_link_i4,   &fx_sex,       &fx_div2,      &fx_ror,
	&fx_ljmp_r8,   &fx_ljmp_r9,   &fx_ljmp_r10,  &fx_ljmp_r11,  &fx_ljmp_r12,  &fx_ljmp_r13,  &fx_lob,       &fx_lmult,
	// a0 - af
	&fx_lms_r0,    &fx_lms_r1,    &fx_lms_r2,    &fx_lms_r3,    &fx_lms_r4,    &fx_lms_r5,    &fx_lms_r6,    &fx_lms_r7,
	&fx_lms_r8,    &fx_lms_r9,    &fx_lms_r10,   &fx_lms_r11,   &fx_lms_r12,   &fx_lms_r13,   &fx_lms_r14,   &fx_lms_r15,
	// b0 - bf
	&fx_from_r0,   &fx_from_r1,   &fx_from_r2,   &fx_from_r3,   &fx_from_r4,   &fx_from_r5,   &fx_from_r6,   &fx_from_r7,
	&fx_from_r8,   &fx_from_r9,   &fx_from_r10,  &fx_from_r11,  &fx_from_r12,  &fx_from_r13,  &fx_from_r14,  &fx_from_r15,
	// c0 - cf
	&fx_hib,       &fx_xor_r1,    &fx_xor_r2,    &fx_xor_r3,    &fx_xor_r4,    &fx_xor_r5,    &fx_xor_r6,    &fx_xor_r7,
	&fx_xor_r8,    &fx_xor_r9,    &fx_xor_r10,   &fx_xor_r11,   &fx_xor_r12,   &fx_xor_r13,   &fx_xor_r14,   &fx_xor_r15,
	// d0 - df
	&fx_inc_r0,    &fx_inc_r1,    &fx_inc_r2,    &fx_inc_r3,    &fx_inc_r4,    &fx_inc_r5,    &fx_inc_r6,    &fx_inc_r7,
	&fx_inc_r8,    &fx_inc_r9,    &fx_inc_r10,   &fx_inc_r11,   &fx_inc_r12,   &fx_inc_r13,   &fx_inc_r14,   &fx_getc,
	// e0 - ef
	&fx_dec_r0,    &fx_dec_r1,    &fx_dec_r2,    &fx_dec_r3,    &fx_dec_r4,    &fx_dec_r5,    &fx_dec_r6,    &fx_dec_r7,
	&fx_dec_r8,    &fx_dec_r9,    &fx_dec_r10,   &fx_dec_r11,   &fx_dec_r12,   &fx_dec_r13,   &fx_dec_r14,   &fx_getbh,
	// f0 - ff
	&fx_lm_r0,     &fx_lm_r1,     &fx_lm_r2,     &fx_lm_r3,     &fx_lm_r4,     &fx_lm_r5,     &fx_lm_r6,     &fx_lm_r7,
	&fx_lm_r8,     &fx_lm_r9,     &fx_lm_r10,    &fx_lm_r11,    &fx_lm_r12,    &fx_lm_r13,    &fx_lm_r14,    &fx_lm_r15,

	// ALT2 Table

	// 00 - 0f
	&fx_stop,      &fx_nop,       &fx_cache,     &fx_lsr,       &fx_rol,       &fx_bra,       &fx_bge,       &fx_blt,
	&fx_bne,       &fx_beq,       &fx_bpl,       &fx_bmi,       &fx_bcc,       &fx_bcs,       &fx_bvc,       &fx_bvs,
	// 10 - 1f
	&fx_to_r0,     &fx_to_r1,     &fx_to_r2,     &fx_to_r3,     &fx_to_r4,     &fx_to_r5,     &fx_to_r6,     &fx_to_r7,
	&fx_to_r8,     &fx_to_r9,     &fx_to_r10,    &fx_to_r11,    &fx_to_r12,    &fx_to_r13,    &fx_to_r14,    &fx_to_r15,
	// 20 - 2f
	&fx_with_r0,   &fx_with_r1,   &fx_with_r2,   &fx_with_r3,   &fx_with_r4,   &fx_with_r5,   &fx_with_r6,   &fx_with_r7,
	&fx_with_r8,   &fx_with_r9,   &fx_with_r10,  &fx_with_r11,  &fx_with_r12,  &fx_with_r13,  &fx_with_r14,  &fx_with_r15,
	// 30 - 3f
	&fx_stw_r0,    &fx_stw_r1,    &fx_stw_r2,    &fx_stw_r3,    &fx_stw_r4,    &fx_stw_r5,    &fx_stw_r6,    &fx_stw_r7,
	&fx_stw_r8,    &fx_stw_r9,    &fx_stw_r10,   &fx_stw_r11,   &fx_loop,      &fx_alt1,      &fx_alt2,      &fx_alt3,
	// 40 - 4f
	&fx_ldw_r0,    &fx_ldw_r1,    &fx_ldw_r2,    &fx_ldw_r3,    &fx_ldw_r4,    &fx_ldw_r5,    &fx_ldw_r6,    &fx_ldw_r7,
	&fx_ldw_r8,    &fx_ldw_r9,    &fx_ldw_r10,   &fx_ldw_r11,   &fx_plot_2bit, &fx_swap,      &fx_color,     &fx_not,
	// 50 - 5f
	&fx_add_i0,    &fx_add_i1,    &fx_add_i2,    &fx_add_i3,    &fx_add_i4,    &fx_add_i5,    &fx_add_i6,    &fx_add_i7,
	&fx_add_i8,    &fx_add_i9,    &fx_add_i10,   &fx_add_i11,   &fx_add_i12,   &fx_add_i13,   &fx_add_i14,   &fx_add_i15,
	// 60 - 6f
	&fx_sub_i0,    &fx_sub_i1,    &fx_sub_i2,    &fx_sub_i3,    &fx_sub_i4,    &fx_sub_i5,    &fx_sub_i6,    &fx_sub_i7,
	&fx_sub_i8,    &fx_sub_i9,    &fx_sub_i10,   &fx_sub_i11,   &fx_sub_i12,   &fx_sub_i13,   &fx_sub_i14,   &fx_sub_i15,
	// 70 - 7f
	&fx_merge,     &fx_and_i1,    &fx_and_i2,    &fx_and_i3,    &fx_and_i4,    &fx_and_i5,    &fx_and_i6,    &fx_and_i7,
	&fx_and_i8,    &fx_and_i9,    &fx_and_i10,   &fx_and_i11,   &fx_and_i12,   &fx_and_i13,   &fx_and_i14,   &fx_and_i15,
	// 80 - 8f
	&fx_mult_i0,   &fx_mult_i1,   &fx_mult_i2,   &fx_mult_i3,   &fx_mult_i4,   &fx_mult_i5,   &fx_mult_i6,   &fx_mult_i7,
	&fx_mult_i8,   &fx_mult_i9,   &fx_mult_i10,  &fx_mult_i11,  &fx_mult_i12,  &fx_mult_i13,  &fx_mult_i14,  &fx_mult_i15,
	// 90 - 9f
	&fx_sbk,       &fx_link_i1,   &fx_link_i2,   &fx_link_i3,   &fx_link_i4,   &fx_sex,       &fx_asr,       &fx_ror,
	&fx_jmp_r8,    &fx_jmp_r9,    &fx_jmp_r10,   &fx_jmp_r11,   &fx_jmp_r12,   &fx_jmp_r13,   &fx_lob,       &fx_fmult,
	// a0 - af
	&fx_sms_r0,    &fx_sms_r1,    &fx_sms_r2,    &fx_sms_r3,    &fx_sms_r4,    &fx_sms_r5,    &fx_sms_r6,    &fx_sms_r7,
	&fx_sms_r8,    &fx_sms_r9,    &fx_sms_r10,   &fx_sms_r11,   &fx_sms_r12,   &fx_sms_r13,   &fx_sms_r14,   &fx_sms_r15,
	// b0 - bf
	&fx_from_r0,   &fx_from_r1,   &fx_from_r2,   &fx_from_r3,   &fx_from_r4,   &fx_from_r5,   &fx_from_r6,   &fx_from_r7,
	&fx_from_r8,   &fx_from_r9,   &fx_from_r10,  &fx_from_r11,  &fx_from_r12,  &fx_from_r13,  &fx_from_r14,  &fx_from_r15,
	// c0 - cf
	&fx_hib,       &fx_or_i1,     &fx_or_i2,     &fx_or_i3,     &fx_or_i4,     &fx_or_i5,     &fx_or_i6,     &fx_or_i7,
	&fx_or_i8,     &fx_or_i9,     &fx_or_i10,    &fx_or_i11,    &fx_or_i12,    &fx_or_i13,    &fx_or_i14,    &fx_or_i15,
	// d0 - df
	&fx_inc_r0,    &fx_inc_r1,    &fx_inc_r2,    &fx_inc_r3,    &fx_inc_r4,    &fx_inc_r5,    &fx_inc_r6,    &fx_inc_r7,
	&fx_inc_r8,    &fx_inc_r9,    &fx_inc_r10,   &fx_inc_r11,   &fx_inc_r12,   &fx_inc_r13,   &fx_inc_r14,   &fx_ramb,
	// e0 - ef
	&fx_dec_r0,    &fx_dec_r1,    &fx_dec_r2,    &fx_dec_r3,    &fx_dec_r4,    &fx_dec_r5,    &fx_dec_r6,    &fx_dec_r7,
	&fx_dec_r8,    &fx_dec_r9,    &fx_dec_r10,   &fx_dec_r11,   &fx_dec_r12,   &fx_dec_r13,   &fx_dec_r14,   &fx_getbl,
	// f0 - ff
	&fx_sm_r0,     &fx_sm_r1,     &fx_sm_r2,     &fx_sm_r3,     &fx_sm_r4,     &fx_sm_r5,     &fx_sm_r6,     &fx_sm_r7,
	&fx_sm_r8,     &fx_sm_r9,     &fx_sm_r10,    &fx_sm_r11,    &fx_sm_r12,    &fx_sm_r13,    &fx_sm_r14,    &fx_sm_r15,

	// ALT3 Table

	// 00 - 0f
	&fx_stop,      &fx_nop,       &fx_cache,     &fx_lsr,       &fx_rol,       &fx_bra,       &fx_bge,       &fx_blt,
	&fx_bne,       &fx_beq,       &fx_bpl,       &fx_bmi,       &fx_bcc,       &fx_bcs,       &fx_bvc,       &fx_bvs,
	// 10 - 1f
	&fx_to_r0,     &fx_to_r1,     &fx_to_r2,     &fx_to_r3,     &fx_to_r4,     &fx_to_r5,     &fx_to_r6,     &fx_to_r7,
	&fx_to_r8,     &fx_to_r9,     &fx_to_r10,    &fx_to_r11,    &fx_to_r12,    &fx_to_r13,    &fx_to_r14,    &fx_to_r15,
	// 20 - 2f
	&fx_with_r0,   &fx_with_r1,   &fx_with_r2,   &fx_with_r3,   &fx_with_r4,   &fx_with_r5,   &fx_with_r6,   &fx_with_r7,
	&fx_with_r8,   &fx_with_r9,   &fx_with_r10,  &fx_with_r11,  &fx_with_r12,  &fx_with_r13,  &fx_with_r14,  &fx_with_r15,
	// 30 - 3f
	&fx_stb_r0,    &fx_stb_r1,    &fx_stb_r2,    &fx_stb_r3,    &fx_stb_r4,    &fx_stb_r5,    &fx_stb_r6,    &fx_stb_r7,
	&fx_stb_r8,    &fx_stb_r9,    &fx_stb_r10,   &fx_stb_r11,   &fx_loop,      &fx_alt1,      &fx_alt2,      &fx_alt3,
	// 40 - 4f
	&fx_ldb_r0,    &fx_ldb_r1,    &fx_ldb_r2,    &fx_ldb_r3,    &fx_ldb_r4,    &fx_ldb_r5,    &fx_ldb_r6,    &fx_ldb_r7,
	&fx_ldb_r8,    &fx_ldb_r9,    &fx_ldb_r10,   &fx_ldb_r11,   &fx_rpix_2bit, &fx_swap,      &fx_cmode,     &fx_not,
	// 50 - 5f
	&fx_adc_i0,    &fx_adc_i1,    &fx_adc_i2,    &fx_adc_i3,    &fx_adc_i4,    &fx_adc_i5,    &fx_adc_i6,    &fx_adc_i7,
	&fx_adc_i8,    &fx_adc_i9,    &fx_adc_i10,   &fx_adc_i11,   &fx_adc_i12,   &fx_adc_i13,   &fx_adc_i14,   &fx_adc_i15,
	// 60 - 6f
	&fx_cmp_r0,    &fx_cmp_r1,    &fx_cmp_r2,    &fx_cmp_r3,    &fx_cmp_r4,    &fx_cmp_r5,    &fx_cmp_r6,    &fx_cmp_r7,
	&fx_cmp_r8,    &fx_cmp_r9,    &fx_cmp_r10,   &fx_cmp_r11,   &fx_cmp_r12,   &fx_cmp_r13,   &fx_cmp_r14,   &fx_cmp_r15,
	// 70 - 7f
	&fx_merge,     &fx_bic_i1,    &fx_bic_i2,    &fx_bic_i3,    &fx_bic_i4,    &fx_bic_i5,    &fx_bic_i6,    &fx_bic_i7,
	&fx_bic_i8,    &fx_bic_i9,    &fx_bic_i10,   &fx_bic_i11,   &fx_bic_i12,   &fx_bic_i13,   &fx_bic_i14,   &fx_bic_i15,
	// 80 - 8f
	&fx_umult_i0,  &fx_umult_i1,  &fx_umult_i2,  &fx_umult_i3,  &fx_umult_i4,  &fx_umult_i5,  &fx_umult_i6,  &fx_umult_i7,
	&fx_umult_i8,  &fx_umult_i9,  &fx_umult_i10, &fx_umult_i11, &fx_umult_i12, &fx_umult_i13, &fx_umult_i14, &fx_umult_i15,
	// 90 - 9f
	&fx_sbk,       &fx_link_i1,   &fx_link_i2,   &fx_link_i3,   &fx_link_i4,   &fx_sex,       &fx_div2,      &fx_ror,
	&fx_ljmp_r8,   &fx_ljmp_r9,   &fx_ljmp_r10,  &fx_ljmp_r11,  &fx_ljmp_r12,  &fx_ljmp_r13,  &fx_lob,       &fx_lmult,
	// a0 - af
	&fx_lms_r0,    &fx_lms_r1,    &fx_lms_r2,    &fx_lms_r3,    &fx_lms_r4,    &fx_lms_r5,    &fx_lms_r6,    &fx_lms_r7,
	&fx_lms_r8,    &fx_lms_r9,    &fx_lms_r10,   &fx_lms_r11,   &fx_lms_r12,   &fx_lms_r13,   &fx_lms_r14,   &fx_lms_r15,
	// b0 - bf
	&fx_from_r0,   &fx_from_r1,   &fx_from_r2,   &fx_from_r3,   &fx_from_r4,   &fx_from_r5,   &fx_from_r6,   &fx_from_r7,
	&fx_from_r8,   &fx_from_r9,   &fx_from_r10,  &fx_from_r11,  &fx_from_r12,  &fx_from_r13,  &fx_from_r14,  &fx_from_r15,
	// c0 - cf
	&fx_hib,       &fx_xor_i1,    &fx_xor_i2,    &fx_xor_i3,    &fx_xor_i4,    &fx_xor_i5,    &fx_xor_i6,    &fx_xor_i7,
	&fx_xor_i8,    &fx_xor_i9,    &fx_xor_i10,   &fx_xor_i11,   &fx_xor_i12,   &fx_xor_i13,   &fx_xor_i14,   &fx_xor_i15,
	// d0 - df
	&fx_inc_r0,    &fx_inc_r1,    &fx_inc_r2,    &fx_inc_r3,    &fx_inc_r4,    &fx_inc_r5,    &fx_inc_r6,    &fx_inc_r7,
	&fx_inc_r8,    &fx_inc_r9,    &fx_inc_r10,   &fx_inc_r11,   &fx_inc_r12,   &fx_inc_r13,   &fx_inc_r14,   &fx_romb,
	// e0 - ef
	&fx_dec_r0,    &fx_dec_r1,    &fx_dec_r2,    &fx_dec_r3,    &fx_dec_r4,    &fx_dec_r5,    &fx_dec_r6,    &fx_dec_r7,
	&fx_dec_r8,    &fx_dec_r9,    &fx_dec_r10,   &fx_dec_r11,   &fx_dec_r12,   &fx_dec_r13,   &fx_dec_r14,   &fx_getbs,
	// f0 - ff
	&fx_lm_r0,     &fx_lm_r1,     &fx_lm_r2,     &fx_lm_r3,     &fx_lm_r4,     &fx_lm_r5,     &fx_lm_r6,     &fx_lm_r7,
	&fx_lm_r8,     &fx_lm_r9,     &fx_lm_r10,    &fx_lm_r11,    &fx_lm_r12,    &fx_lm_r13,    &fx_lm_r14,    &fx_lm_r15
};
