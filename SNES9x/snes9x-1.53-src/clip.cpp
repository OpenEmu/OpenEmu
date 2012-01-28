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

static uint8	region_map[6][6] =
{
	{ 0, 0x01, 0x03, 0x07, 0x0f, 0x1f },
	{ 0,    0, 0x02, 0x06, 0x0e, 0x1e },
	{ 0,    0,    0, 0x04, 0x0c, 0x1c },
	{ 0,    0,    0,    0, 0x08, 0x18 },
	{ 0,    0,    0,    0,    0, 0x10 }
};

static inline uint8 CalcWindowMask (int, uint8, uint8);
static inline void StoreWindowRegions (uint8, struct ClipData *, int, int16 *, uint8 *, bool8, bool8 s = FALSE);


static inline uint8 CalcWindowMask (int i, uint8 W1, uint8 W2)
{
	if (!PPU.ClipWindow1Enable[i])
	{
		if (!PPU.ClipWindow2Enable[i])
			return (0);
		else
		{
			if (!PPU.ClipWindow2Inside[i])
				return (~W2);
			return (W2);
		}
	}
	else
	{
		if (!PPU.ClipWindow2Enable[i])
		{
			if (!PPU.ClipWindow1Inside[i])
				return (~W1);
			return (W1);
		}
		else
		{
			if (!PPU.ClipWindow1Inside[i])
				W1 = ~W1;
			if (!PPU.ClipWindow2Inside[i])
				W2 = ~W2;

			switch (PPU.ClipWindowOverlapLogic[i])
			{
				case 0: // OR
					return (W1 | W2);

				case 1: // AND
					return (W1 & W2);

				case 2: // XOR
					return (W1 ^ W2);

				case 3: // XNOR
					return (~(W1 ^ W2));
			}
		}
	}

	// Never get here
	return (0);
}

static inline void StoreWindowRegions (uint8 Mask, struct ClipData *Clip, int n_regions, int16 *windows, uint8 *drawing_modes, bool8 sub, bool8 StoreMode0)
{
	int	ct = 0;

	for (int j = 0; j < n_regions; j++)
	{
		int	DrawMode = drawing_modes[j];
		if (sub)
			DrawMode |= 1;
		if (Mask & (1 << j))
			DrawMode = 0;

		if (!StoreMode0 && !DrawMode)
			continue;

		if (ct > 0 && Clip->Right[ct - 1] == windows[j] && Clip->DrawMode[ct - 1] == DrawMode)
			Clip->Right[ct - 1] = windows[j + 1]; // This region borders with and has the same drawing mode as the previous region: merge them.
		else
		{
			// Add a new region to the BG
			Clip->Left[ct]     = windows[j];
			Clip->Right[ct]    = windows[j + 1];
			Clip->DrawMode[ct] = DrawMode;
			ct++;
		}
	}

	Clip->Count = ct;
}

void S9xComputeClipWindows (void)
{
	int16	windows[6] = { 0, 256, 256, 256, 256, 256 };
	uint8	drawing_modes[5] = { 0, 0, 0, 0, 0 };
	int		n_regions = 1;
	int		i, j;

	// Calculate window regions. We have at most 5 regions, because we have 6 control points
	// (screen edges, window 1 left & right, and window 2 left & right).

	if (PPU.Window1Left <= PPU.Window1Right)
	{
		if (PPU.Window1Left > 0)
		{
			windows[2] = 256;
			windows[1] = PPU.Window1Left;
			n_regions = 2;
		}

		if (PPU.Window1Right < 255)
		{
			windows[n_regions + 1] = 256;
			windows[n_regions] = PPU.Window1Right + 1;
			n_regions++;
		}
	}

	if (PPU.Window2Left <= PPU.Window2Right)
	{
		for (i = 0; i <= n_regions; i++)
		{
			if (PPU.Window2Left == windows[i])
				break;

			if (PPU.Window2Left <  windows[i])
			{
				for (j = n_regions; j >= i; j--)
					windows[j + 1] = windows[j];

				windows[i] = PPU.Window2Left;
				n_regions++;
				break;
			}
		}

		for (; i <= n_regions; i++)
		{
			if (PPU.Window2Right + 1 == windows[i])
				break;

			if (PPU.Window2Right + 1 <  windows[i])
			{
				for (j = n_regions; j >= i; j--)
					windows[j + 1] = windows[j];

				windows[i] = PPU.Window2Right + 1;
				n_regions++;
				break;
			}
		}
	}

	// Get a bitmap of which regions correspond to each window.

	uint8	W1, W2;

	if (PPU.Window1Left <= PPU.Window1Right)
	{
		for (i = 0; windows[i] != PPU.Window1Left; i++) ;
		for (j = i; windows[j] != PPU.Window1Right + 1; j++) ;
		W1 = region_map[i][j];
	}
	else
		W1 = 0;

	if (PPU.Window2Left <= PPU.Window2Right)
	{
		for (i = 0; windows[i] != PPU.Window2Left; i++) ;
		for (j = i; windows[j] != PPU.Window2Right + 1; j++) ;
		W2 = region_map[i][j];
	}
	else
		W2 = 0;

	// Color Window affects the drawing mode for each region.
	// Modes are: 3=Draw as normal, 2=clip color (math only), 1=no math (draw only), 0=nothing.

	uint8	CW_color = 0, CW_math = 0;
	uint8	CW = CalcWindowMask(5, W1, W2);

	switch (Memory.FillRAM[0x2130] & 0xc0)
	{
		case 0x00:	CW_color = 0;		break;
		case 0x40:	CW_color = ~CW;		break;
		case 0x80:	CW_color = CW;		break;
		case 0xc0:	CW_color = 0xff;	break;
	}

	switch (Memory.FillRAM[0x2130] & 0x30)
	{
		case 0x00:	CW_math  = 0;		break;
		case 0x10:	CW_math  = ~CW;		break;
		case 0x20:	CW_math  = CW;		break;
		case 0x30:	CW_math  = 0xff;	break;
	}

	for (i = 0; i < n_regions; i++)
	{
		if (!(CW_color & (1 << i)))
			drawing_modes[i] |= 1;
		if (!(CW_math  & (1 << i)))
			drawing_modes[i] |= 2;
	}

	// Store backdrop clip window (draw everywhere color window allows)

	StoreWindowRegions(0, &IPPU.Clip[0][5], n_regions, windows, drawing_modes, FALSE, TRUE);
	StoreWindowRegions(0, &IPPU.Clip[1][5], n_regions, windows, drawing_modes, TRUE,  TRUE);

	// Store per-BG and OBJ clip windows

	for (j = 0; j < 5; j++)
	{
		uint8	W = Settings.DisableGraphicWindows ? 0 : CalcWindowMask(j, W1, W2);
		for (int sub = 0; sub < 2; sub++)
		{
			if (Memory.FillRAM[sub + 0x212e] & (1 << j))
				StoreWindowRegions(W, &IPPU.Clip[sub][j], n_regions, windows, drawing_modes, sub);
			else
				StoreWindowRegions(0, &IPPU.Clip[sub][j], n_regions, windows, drawing_modes, sub);
		}
	}
}
