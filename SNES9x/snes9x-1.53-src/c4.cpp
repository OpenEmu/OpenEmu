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


#include <math.h>
#include "snes9x.h"
#include "memmap.h"

#define	C4_PI	3.14159265

int16	C4WFXVal;
int16	C4WFYVal;
int16	C4WFZVal;
int16	C4WFX2Val;
int16	C4WFY2Val;
int16	C4WFDist;
int16	C4WFScale;
int16	C41FXVal;
int16	C41FYVal;
int16	C41FAngleRes;
int16	C41FDist;
int16	C41FDistVal;

static double	tanval;
static double	c4x, c4y, c4z;
static double	c4x2, c4y2, c4z2;


void C4TransfWireFrame (void)
{
	c4x = (double) C4WFXVal;
	c4y = (double) C4WFYVal;
	c4z = (double) C4WFZVal - 0x95;

	// Rotate X
	tanval = -(double) C4WFX2Val * C4_PI * 2 / 128;
	c4y2 = c4y  *  cos(tanval) - c4z  * sin(tanval);
	c4z2 = c4y  *  sin(tanval) + c4z  * cos(tanval);

	// Rotate Y
	tanval = -(double) C4WFY2Val * C4_PI * 2 / 128;
	c4x2 = c4x  *  cos(tanval) + c4z2 * sin(tanval);
	c4z  = c4x  * -sin(tanval) + c4z2 * cos(tanval);

	// Rotate Z
	tanval = -(double) C4WFDist  * C4_PI * 2 / 128;
	c4x  = c4x2 *  cos(tanval) - c4y2 * sin(tanval);
	c4y  = c4x2 *  sin(tanval) + c4y2 * cos(tanval);

	// Scale
	C4WFXVal = (int16) (c4x * (double) C4WFScale / (0x90 * (c4z + 0x95)) * 0x95);
	C4WFYVal = (int16) (c4y * (double) C4WFScale / (0x90 * (c4z + 0x95)) * 0x95);
}

void C4TransfWireFrame2 (void)
{
	c4x = (double) C4WFXVal;
	c4y = (double) C4WFYVal;
	c4z = (double) C4WFZVal;

	// Rotate X
	tanval = -(double) C4WFX2Val * C4_PI * 2 / 128;
	c4y2 = c4y  *  cos(tanval) - c4z  * sin(tanval);
	c4z2 = c4y  *  sin(tanval) + c4z  * cos(tanval);

	// Rotate Y
	tanval = -(double) C4WFY2Val * C4_PI * 2 / 128;
	c4x2 = c4x  *  cos(tanval) + c4z2 * sin(tanval);
	c4z  = c4x  * -sin(tanval) + c4z2 * cos(tanval);

	// Rotate Z
	tanval = -(double) C4WFDist  * C4_PI * 2 / 128;
	c4x  = c4x2 *  cos(tanval) - c4y2 * sin(tanval);
	c4y  = c4x2 *  sin(tanval) + c4y2 * cos(tanval);

	// Scale
	C4WFXVal = (int16) (c4x * (double) C4WFScale / 0x100);
	C4WFYVal = (int16) (c4y * (double) C4WFScale / 0x100);
}

void C4CalcWireFrame (void)
{
	C4WFXVal = C4WFX2Val - C4WFXVal;
	C4WFYVal = C4WFY2Val - C4WFYVal;

	if (abs(C4WFXVal) > abs(C4WFYVal))
	{
		C4WFDist = abs(C4WFXVal) + 1;
		C4WFYVal = (int16) (256 * (double) C4WFYVal / abs(C4WFXVal));
		if (C4WFXVal < 0)
			C4WFXVal = -256;
		else
			C4WFXVal =  256;
	}
	else
	{
		if (C4WFYVal != 0)
		{
			C4WFDist = abs(C4WFYVal) + 1;
			C4WFXVal = (int16) (256 * (double) C4WFXVal / abs(C4WFYVal));
			if (C4WFYVal < 0)
				C4WFYVal = -256;
			else
				C4WFYVal =  256;
		}
		else
			C4WFDist = 0;
	}
}

void C4Op1F (void)
{
	if (C41FXVal == 0)
	{
		if (C41FYVal > 0)
			C41FAngleRes = 0x80;
		else
			C41FAngleRes = 0x180;
	}
	else
	{
		tanval = (double) C41FYVal / C41FXVal;
		C41FAngleRes = (int16) (atan(tanval) / (C4_PI * 2) * 512);
		C41FAngleRes = C41FAngleRes;
		if (C41FXVal< 0)
			C41FAngleRes += 0x100;
		C41FAngleRes &= 0x1FF;
	}
}

void C4Op15 (void)
{
	tanval = sqrt((double) C41FYVal * C41FYVal + (double) C41FXVal * C41FXVal);
	C41FDist = (int16) tanval;
}

void C4Op0D (void)
{
	tanval = sqrt((double) C41FYVal * C41FYVal + (double) C41FXVal * C41FXVal);
	tanval = C41FDistVal / tanval;
	C41FYVal = (int16) (C41FYVal * tanval * 0.99);
	C41FXVal = (int16) (C41FXVal * tanval * 0.98);
}

uint8 * S9xGetBasePointerC4 (uint16 Address)
{
	if (Address >= 0x7f40 && Address <= 0x7f5e)
		return (NULL);
	return (Memory.C4RAM - 0x6000);
}

uint8 * S9xGetMemPointerC4 (uint16 Address)
{
	if (Address >= 0x7f40 && Address <= 0x7f5e)
		return (NULL);
	return (Memory.C4RAM - 0x6000 + (Address & 0xffff));
}
