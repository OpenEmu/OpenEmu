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


// This file includes itself multiple times.
// The other option would be to have 4 files, where A includes B, and B includes C 3 times, and C includes D 5 times.
// Look for the following marker to find where the divisions are.

// Top-level compilation.

#ifndef _NEWTILE_CPP
#define _NEWTILE_CPP

#include "snes9x.h"
#include "ppu.h"
#include "tile.h"

static uint32	pixbit[8][16];
static uint8	hrbit_odd[256];
static uint8	hrbit_even[256];


void S9xInitTileRenderer (void)
{
	register int	i;

	for (i = 0; i < 16; i++)
	{
		register uint32	b = 0;

	#ifdef LSB_FIRST
		if (i & 8)
			b |= 1;
		if (i & 4)
			b |= 1 << 8;
		if (i & 2)
			b |= 1 << 16;
		if (i & 1)
			b |= 1 << 24;
	#else
		if (i & 8)
			b |= 1 << 24;
		if (i & 4)
			b |= 1 << 16;
		if (i & 2)
			b |= 1 << 8;
		if (i & 1)
			b |= 1;
	#endif

		for (uint8 bitshift = 0; bitshift < 8; bitshift++)
			pixbit[bitshift][i] = b << bitshift;
	}

	for (i = 0; i < 256; i++)
	{
		register uint8	m = 0;
		register uint8	s = 0;

		if (i & 0x80)
			s |= 8;
		if (i & 0x40)
			m |= 8;
		if (i & 0x20)
			s |= 4;
		if (i & 0x10)
			m |= 4;
		if (i & 0x08)
			s |= 2;
		if (i & 0x04)
			m |= 2;
		if (i & 0x02)
			s |= 1;
		if (i & 0x01)
			m |= 1;

		hrbit_odd[i]  = m;
		hrbit_even[i] = s;
	}
}

// Here are the tile converters, selected by S9xSelectTileConverter().
// Really, except for the definition of DOBIT and the number of times it is called, they're all the same.

#define DOBIT(n, i) \
	if ((pix = *(tp + (n)))) \
	{ \
		p1 |= pixbit[(i)][pix >> 4]; \
		p2 |= pixbit[(i)][pix & 0xf]; \
	}

static uint8 ConvertTile2 (uint8 *pCache, uint32 TileAddr, uint32)
{
	register uint8	*tp      = &Memory.VRAM[TileAddr];
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;
	uint8			line;

	for (line = 8; line != 0; line--, tp += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		register uint8	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile4 (uint8 *pCache, uint32 TileAddr, uint32)
{
	register uint8	*tp      = &Memory.VRAM[TileAddr];
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;
	uint8			line;

	for (line = 8; line != 0; line--, tp += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		register uint8	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		DOBIT(16, 2);
		DOBIT(17, 3);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile8 (uint8 *pCache, uint32 TileAddr, uint32)
{
	register uint8	*tp      = &Memory.VRAM[TileAddr];
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;
	uint8			line;

	for (line = 8; line != 0; line--, tp += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		register uint8	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		DOBIT(16, 2);
		DOBIT(17, 3);
		DOBIT(32, 4);
		DOBIT(33, 5);
		DOBIT(48, 6);
		DOBIT(49, 7);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

#undef DOBIT

#define DOBIT(n, i) \
	if ((pix = hrbit_odd[*(tp1 + (n))])) \
		p1 |= pixbit[(i)][pix]; \
	if ((pix = hrbit_odd[*(tp2 + (n))])) \
		p2 |= pixbit[(i)][pix];

static uint8 ConvertTile2h_odd (uint8 *pCache, uint32 TileAddr, uint32 Tile)
{
	register uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;
	uint8			line;

	if (Tile == 0x3ff)
		tp2 = tp1 - (0x3ff << 4);
	else
		tp2 = tp1 + (1 << 4);

	for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		register uint8	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile4h_odd (uint8 *pCache, uint32 TileAddr, uint32 Tile)
{
	register uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;
	uint8			line;

	if (Tile == 0x3ff)
		tp2 = tp1 - (0x3ff << 5);
	else
		tp2 = tp1 + (1 << 5);

	for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		register uint8	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		DOBIT(16, 2);
		DOBIT(17, 3);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

#undef DOBIT

#define DOBIT(n, i) \
	if ((pix = hrbit_even[*(tp1 + (n))])) \
		p1 |= pixbit[(i)][pix]; \
	if ((pix = hrbit_even[*(tp2 + (n))])) \
		p2 |= pixbit[(i)][pix];

static uint8 ConvertTile2h_even (uint8 *pCache, uint32 TileAddr, uint32 Tile)
{
	register uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;
	uint8			line;

	if (Tile == 0x3ff)
		tp2 = tp1 - (0x3ff << 4);
	else
		tp2 = tp1 + (1 << 4);

	for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		register uint8	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

static uint8 ConvertTile4h_even (uint8 *pCache, uint32 TileAddr, uint32 Tile)
{
	register uint8	*tp1     = &Memory.VRAM[TileAddr], *tp2;
	uint32			*p       = (uint32 *) pCache;
	uint32			non_zero = 0;
	uint8			line;

	if (Tile == 0x3ff)
		tp2 = tp1 - (0x3ff << 5);
	else
		tp2 = tp1 + (1 << 5);

	for (line = 8; line != 0; line--, tp1 += 2, tp2 += 2)
	{
		uint32			p1 = 0;
		uint32			p2 = 0;
		register uint8	pix;

		DOBIT( 0, 0);
		DOBIT( 1, 1);
		DOBIT(16, 2);
		DOBIT(17, 3);
		*p++ = p1;
		*p++ = p2;
		non_zero |= p1 | p2;
	}

	return (non_zero ? TRUE : BLANK_TILE);
}

#undef DOBIT

// First-level include: Get all the renderers.

#include "tile.cpp"

// Functions to select which converter and renderer to use.

void S9xSelectTileRenderers (int BGMode, bool8 sub, bool8 obj)
{
	void	(**DT)		(uint32, uint32, uint32, uint32);
	void	(**DCT)		(uint32, uint32, uint32, uint32, uint32, uint32);
	void	(**DMP)		(uint32, uint32, uint32, uint32, uint32, uint32);
	void	(**DB)		(uint32, uint32, uint32);
	void	(**DM7BG1)	(uint32, uint32, int);
	void	(**DM7BG2)	(uint32, uint32, int);
	bool8	M7M1, M7M2;

	M7M1 = PPU.BGMosaic[0] && PPU.Mosaic > 1;
	M7M2 = PPU.BGMosaic[1] && PPU.Mosaic > 1;

	bool8 interlace = obj ? FALSE : IPPU.Interlace;
	bool8 hires = !sub && (BGMode == 5 || BGMode == 6 || IPPU.PseudoHires);

	if (!IPPU.DoubleWidthPixels)	// normal width
	{
		DT     = Renderers_DrawTile16Normal1x1;
		DCT    = Renderers_DrawClippedTile16Normal1x1;
		DMP    = Renderers_DrawMosaicPixel16Normal1x1;
		DB     = Renderers_DrawBackdrop16Normal1x1;
		DM7BG1 = M7M1 ? Renderers_DrawMode7MosaicBG1Normal1x1 : Renderers_DrawMode7BG1Normal1x1;
		DM7BG2 = M7M2 ? Renderers_DrawMode7MosaicBG2Normal1x1 : Renderers_DrawMode7BG2Normal1x1;
		GFX.LinesPerTile = 8;
	}
	else if(hires)					// hires double width
	{
		if (interlace)
		{
			DT     = Renderers_DrawTile16HiresInterlace;
			DCT    = Renderers_DrawClippedTile16HiresInterlace;
			DMP    = Renderers_DrawMosaicPixel16HiresInterlace;
			DB     = Renderers_DrawBackdrop16Hires;
			DM7BG1 = M7M1 ? Renderers_DrawMode7MosaicBG1Hires : Renderers_DrawMode7BG1Hires;
			DM7BG2 = M7M2 ? Renderers_DrawMode7MosaicBG2Hires : Renderers_DrawMode7BG2Hires;
			GFX.LinesPerTile = 4;
		}
		else
		{
			DT     = Renderers_DrawTile16Hires;
			DCT    = Renderers_DrawClippedTile16Hires;
			DMP    = Renderers_DrawMosaicPixel16Hires;
			DB     = Renderers_DrawBackdrop16Hires;
			DM7BG1 = M7M1 ? Renderers_DrawMode7MosaicBG1Hires : Renderers_DrawMode7BG1Hires;
			DM7BG2 = M7M2 ? Renderers_DrawMode7MosaicBG2Hires : Renderers_DrawMode7BG2Hires;
			GFX.LinesPerTile = 8;
		}
	}
	else							// normal double width
	{
		if (interlace)
		{
			DT     = Renderers_DrawTile16Interlace;
			DCT    = Renderers_DrawClippedTile16Interlace;
			DMP    = Renderers_DrawMosaicPixel16Interlace;
			DB     = Renderers_DrawBackdrop16Normal2x1;
			DM7BG1 = M7M1 ? Renderers_DrawMode7MosaicBG1Normal2x1 : Renderers_DrawMode7BG1Normal2x1;
			DM7BG2 = M7M2 ? Renderers_DrawMode7MosaicBG2Normal2x1 : Renderers_DrawMode7BG2Normal2x1;
			GFX.LinesPerTile = 4;
		}
		else
		{
			DT     = Renderers_DrawTile16Normal2x1;
			DCT    = Renderers_DrawClippedTile16Normal2x1;
			DMP    = Renderers_DrawMosaicPixel16Normal2x1;
			DB     = Renderers_DrawBackdrop16Normal2x1;
			DM7BG1 = M7M1 ? Renderers_DrawMode7MosaicBG1Normal2x1 : Renderers_DrawMode7BG1Normal2x1;
			DM7BG2 = M7M2 ? Renderers_DrawMode7MosaicBG2Normal2x1 : Renderers_DrawMode7BG2Normal2x1;
			GFX.LinesPerTile = 8;
		}
	}

	GFX.DrawTileNomath        = DT[0];
	GFX.DrawClippedTileNomath = DCT[0];
	GFX.DrawMosaicPixelNomath = DMP[0];
	GFX.DrawBackdropNomath    = DB[0];
	GFX.DrawMode7BG1Nomath    = DM7BG1[0];
	GFX.DrawMode7BG2Nomath    = DM7BG2[0];

	int	i;

	if (!Settings.Transparency)
		i = 0;
	else
	{
		i = (Memory.FillRAM[0x2131] & 0x80) ? 4 : 1;
		if (Memory.FillRAM[0x2131] & 0x40)
		{
			i++;
			if (Memory.FillRAM[0x2130] & 2)
				i++;
		}
	}

	GFX.DrawTileMath        = DT[i];
	GFX.DrawClippedTileMath = DCT[i];
	GFX.DrawMosaicPixelMath = DMP[i];
	GFX.DrawBackdropMath    = DB[i];
	GFX.DrawMode7BG1Math    = DM7BG1[i];
	GFX.DrawMode7BG2Math    = DM7BG2[i];
}

void S9xSelectTileConverter (int depth, bool8 hires, bool8 sub, bool8 mosaic)
{
	switch (depth)
	{
		case 8:
			BG.ConvertTile      = BG.ConvertTileFlip = ConvertTile8;
			BG.Buffer           = BG.BufferFlip      = IPPU.TileCache[TILE_8BIT];
			BG.Buffered         = BG.BufferedFlip    = IPPU.TileCached[TILE_8BIT];
			BG.TileShift        = 6;
			BG.PaletteShift     = 0;
			BG.PaletteMask      = 0;
			BG.DirectColourMode = Memory.FillRAM[0x2130] & 1;

			break;

		case 4:
			if (hires)
			{
				if (sub || mosaic)
				{
					BG.ConvertTile     = ConvertTile4h_even;
					BG.Buffer          = IPPU.TileCache[TILE_4BIT_EVEN];
					BG.Buffered        = IPPU.TileCached[TILE_4BIT_EVEN];
					BG.ConvertTileFlip = ConvertTile4h_odd;
					BG.BufferFlip      = IPPU.TileCache[TILE_4BIT_ODD];
					BG.BufferedFlip    = IPPU.TileCached[TILE_4BIT_ODD];
				}
				else
				{
					BG.ConvertTile     = ConvertTile4h_odd;
					BG.Buffer          = IPPU.TileCache[TILE_4BIT_ODD];
					BG.Buffered        = IPPU.TileCached[TILE_4BIT_ODD];
					BG.ConvertTileFlip = ConvertTile4h_even;
					BG.BufferFlip      = IPPU.TileCache[TILE_4BIT_EVEN];
					BG.BufferedFlip    = IPPU.TileCached[TILE_4BIT_EVEN];
				}
			}
			else
			{
				BG.ConvertTile = BG.ConvertTileFlip = ConvertTile4;
				BG.Buffer      = BG.BufferFlip      = IPPU.TileCache[TILE_4BIT];
				BG.Buffered    = BG.BufferedFlip    = IPPU.TileCached[TILE_4BIT];
			}

			BG.TileShift        = 5;
			BG.PaletteShift     = 10 - 4;
			BG.PaletteMask      = 7 << 4;
			BG.DirectColourMode = FALSE;

			break;

		case 2:
			if (hires)
			{
				if (sub || mosaic)
				{
					BG.ConvertTile     = ConvertTile2h_even;
					BG.Buffer          = IPPU.TileCache[TILE_2BIT_EVEN];
					BG.Buffered        = IPPU.TileCached[TILE_2BIT_EVEN];
					BG.ConvertTileFlip = ConvertTile2h_odd;
					BG.BufferFlip      = IPPU.TileCache[TILE_2BIT_ODD];
					BG.BufferedFlip    = IPPU.TileCached[TILE_2BIT_ODD];
				}
				else
				{
					BG.ConvertTile     = ConvertTile2h_odd;
					BG.Buffer          = IPPU.TileCache[TILE_2BIT_ODD];
					BG.Buffered        = IPPU.TileCached[TILE_2BIT_ODD];
					BG.ConvertTileFlip = ConvertTile2h_even;
					BG.BufferFlip      = IPPU.TileCache[TILE_2BIT_EVEN];
					BG.BufferedFlip    = IPPU.TileCached[TILE_2BIT_EVEN];
				}
			}
			else
			{
				BG.ConvertTile = BG.ConvertTileFlip = ConvertTile2;
				BG.Buffer      = BG.BufferFlip      = IPPU.TileCache[TILE_2BIT];
				BG.Buffered    = BG.BufferedFlip    = IPPU.TileCached[TILE_2BIT];
			}

			BG.TileShift        = 4;
			BG.PaletteShift     = 10 - 2;
			BG.PaletteMask      = 7 << 2;
			BG.DirectColourMode = FALSE;

			break;
	}
}

/*****************************************************************************/
#else
#ifndef NAME1 // First-level: Get all the renderers.
/*****************************************************************************/

#define GET_CACHED_TILE() \
	uint32	TileNumber; \
	uint32	TileAddr = BG.TileAddress + ((Tile & 0x3ff) << BG.TileShift); \
	if (Tile & 0x100) \
		TileAddr += BG.NameSelect; \
	TileAddr &= 0xffff; \
	TileNumber = TileAddr >> BG.TileShift; \
	if (Tile & H_FLIP) \
	{ \
		pCache = &BG.BufferFlip[TileNumber << 6]; \
		if (!BG.BufferedFlip[TileNumber]) \
			BG.BufferedFlip[TileNumber] = BG.ConvertTileFlip(pCache, TileAddr, Tile & 0x3ff); \
	} \
	else \
	{ \
		pCache = &BG.Buffer[TileNumber << 6]; \
		if (!BG.Buffered[TileNumber]) \
			BG.Buffered[TileNumber] = BG.ConvertTile(pCache, TileAddr, Tile & 0x3ff); \
	}

#define IS_BLANK_TILE() \
	(BG.Buffered[TileNumber] == BLANK_TILE)

#define SELECT_PALETTE() \
	if (BG.DirectColourMode) \
	{ \
		if (IPPU.DirectColourMapsNeedRebuild) \
			S9xBuildDirectColourMaps(); \
		GFX.RealScreenColors = DirectColourMaps[(Tile >> 10) & 7]; \
	} \
	else \
		GFX.RealScreenColors = &IPPU.ScreenColors[((Tile >> BG.PaletteShift) & BG.PaletteMask) + BG.StartPalette]; \
	GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors

#define NOMATH(Op, Main, Sub, SD) \
	(Main)

#define REGMATH(Op, Main, Sub, SD) \
	(COLOR_##Op((Main), ((SD) & 0x20) ? (Sub) : GFX.FixedColour))

#define MATHF1_2(Op, Main, Sub, SD) \
	(GFX.ClipColors ? (COLOR_##Op((Main), GFX.FixedColour)) : (COLOR_##Op##1_2((Main), GFX.FixedColour)))

#define MATHS1_2(Op, Main, Sub, SD) \
	(GFX.ClipColors ? REGMATH(Op, Main, Sub, SD) : (((SD) & 0x20) ? COLOR_##Op##1_2((Main), (Sub)) : COLOR_##Op((Main), GFX.FixedColour)))

// Basic routine to render an unclipped tile.
// Input parameters:
//     BPSTART = either StartLine or (StartLine * 2 + BG.InterlaceLine),
//     so interlace modes can render every other line from the tile.
//     PITCH = 1 or 2, again so interlace can count lines properly.
//     DRAW_PIXEL(N, M) is a routine to actually draw the pixel. N is the pixel in the row to draw,
//     and M is a test which if false means the pixel should be skipped.
//     Z1 is the "draw if Z1 > cur_depth".
//     Z2 is the "cur_depth = new_depth". OBJ need the two separate.
//     Pix is the pixel to draw.

#define Z1	GFX.Z1
#define Z2	GFX.Z2

#define DRAW_TILE() \
	uint8			*pCache; \
	register int32	l; \
	register uint8	*bp, Pix; \
	\
	GET_CACHED_TILE(); \
	if (IS_BLANK_TILE()) \
		return; \
	SELECT_PALETTE(); \
	\
	if (!(Tile & (V_FLIP | H_FLIP))) \
	{ \
		bp = pCache + BPSTART; \
		for (l = LineCount; l > 0; l--, bp += 8 * PITCH, Offset += GFX.PPL) \
		{ \
			DRAW_PIXEL(0, Pix = bp[0]); \
			DRAW_PIXEL(1, Pix = bp[1]); \
			DRAW_PIXEL(2, Pix = bp[2]); \
			DRAW_PIXEL(3, Pix = bp[3]); \
			DRAW_PIXEL(4, Pix = bp[4]); \
			DRAW_PIXEL(5, Pix = bp[5]); \
			DRAW_PIXEL(6, Pix = bp[6]); \
			DRAW_PIXEL(7, Pix = bp[7]); \
		} \
	} \
	else \
	if (!(Tile & V_FLIP)) \
	{ \
		bp = pCache + BPSTART; \
		for (l = LineCount; l > 0; l--, bp += 8 * PITCH, Offset += GFX.PPL) \
		{ \
			DRAW_PIXEL(0, Pix = bp[7]); \
			DRAW_PIXEL(1, Pix = bp[6]); \
			DRAW_PIXEL(2, Pix = bp[5]); \
			DRAW_PIXEL(3, Pix = bp[4]); \
			DRAW_PIXEL(4, Pix = bp[3]); \
			DRAW_PIXEL(5, Pix = bp[2]); \
			DRAW_PIXEL(6, Pix = bp[1]); \
			DRAW_PIXEL(7, Pix = bp[0]); \
		} \
	} \
	else \
	if (!(Tile & H_FLIP)) \
	{ \
		bp = pCache + 56 - BPSTART; \
		for (l = LineCount; l > 0; l--, bp -= 8 * PITCH, Offset += GFX.PPL) \
		{ \
			DRAW_PIXEL(0, Pix = bp[0]); \
			DRAW_PIXEL(1, Pix = bp[1]); \
			DRAW_PIXEL(2, Pix = bp[2]); \
			DRAW_PIXEL(3, Pix = bp[3]); \
			DRAW_PIXEL(4, Pix = bp[4]); \
			DRAW_PIXEL(5, Pix = bp[5]); \
			DRAW_PIXEL(6, Pix = bp[6]); \
			DRAW_PIXEL(7, Pix = bp[7]); \
		} \
	} \
	else \
	{ \
		bp = pCache + 56 - BPSTART; \
		for (l = LineCount; l > 0; l--, bp -= 8 * PITCH, Offset += GFX.PPL) \
		{ \
			DRAW_PIXEL(0, Pix = bp[7]); \
			DRAW_PIXEL(1, Pix = bp[6]); \
			DRAW_PIXEL(2, Pix = bp[5]); \
			DRAW_PIXEL(3, Pix = bp[4]); \
			DRAW_PIXEL(4, Pix = bp[3]); \
			DRAW_PIXEL(5, Pix = bp[2]); \
			DRAW_PIXEL(6, Pix = bp[1]); \
			DRAW_PIXEL(7, Pix = bp[0]); \
		} \
	}

#define NAME1	DrawTile16
#define ARGS	uint32 Tile, uint32 Offset, uint32 StartLine, uint32 LineCount

// Second-level include: Get the DrawTile16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Z1
#undef Z2

// Basic routine to render a clipped tile. Inputs same as above.

#define Z1	GFX.Z1
#define Z2	GFX.Z2

#define DRAW_TILE() \
	uint8			*pCache; \
	register int32	l; \
	register uint8	*bp, Pix, w; \
	\
	GET_CACHED_TILE(); \
	if (IS_BLANK_TILE()) \
		return; \
	SELECT_PALETTE(); \
	\
	if (!(Tile & (V_FLIP | H_FLIP))) \
	{ \
		bp = pCache + BPSTART; \
		for (l = LineCount; l > 0; l--, bp += 8 * PITCH, Offset += GFX.PPL) \
		{ \
			w = Width; \
			switch (StartPixel) \
			{ \
				case 0: DRAW_PIXEL(0, Pix = bp[0]); if (!--w) break; \
				case 1: DRAW_PIXEL(1, Pix = bp[1]); if (!--w) break; \
				case 2: DRAW_PIXEL(2, Pix = bp[2]); if (!--w) break; \
				case 3: DRAW_PIXEL(3, Pix = bp[3]); if (!--w) break; \
				case 4: DRAW_PIXEL(4, Pix = bp[4]); if (!--w) break; \
				case 5: DRAW_PIXEL(5, Pix = bp[5]); if (!--w) break; \
				case 6: DRAW_PIXEL(6, Pix = bp[6]); if (!--w) break; \
				case 7: DRAW_PIXEL(7, Pix = bp[7]); break; \
			} \
		} \
	} \
	else \
	if (!(Tile & V_FLIP)) \
	{ \
		bp = pCache + BPSTART; \
		for (l = LineCount; l > 0; l--, bp += 8 * PITCH, Offset += GFX.PPL) \
		{ \
			w = Width; \
			switch (StartPixel) \
			{ \
				case 0: DRAW_PIXEL(0, Pix = bp[7]); if (!--w) break; \
				case 1: DRAW_PIXEL(1, Pix = bp[6]); if (!--w) break; \
				case 2: DRAW_PIXEL(2, Pix = bp[5]); if (!--w) break; \
				case 3: DRAW_PIXEL(3, Pix = bp[4]); if (!--w) break; \
				case 4: DRAW_PIXEL(4, Pix = bp[3]); if (!--w) break; \
				case 5: DRAW_PIXEL(5, Pix = bp[2]); if (!--w) break; \
				case 6: DRAW_PIXEL(6, Pix = bp[1]); if (!--w) break; \
				case 7: DRAW_PIXEL(7, Pix = bp[0]); break; \
			} \
		} \
	} \
	else \
	if (!(Tile & H_FLIP)) \
	{ \
		bp = pCache + 56 - BPSTART; \
		for (l = LineCount; l > 0; l--, bp -= 8 * PITCH, Offset += GFX.PPL) \
		{ \
			w = Width; \
			switch (StartPixel) \
			{ \
				case 0: DRAW_PIXEL(0, Pix = bp[0]); if (!--w) break; \
				case 1: DRAW_PIXEL(1, Pix = bp[1]); if (!--w) break; \
				case 2: DRAW_PIXEL(2, Pix = bp[2]); if (!--w) break; \
				case 3: DRAW_PIXEL(3, Pix = bp[3]); if (!--w) break; \
				case 4: DRAW_PIXEL(4, Pix = bp[4]); if (!--w) break; \
				case 5: DRAW_PIXEL(5, Pix = bp[5]); if (!--w) break; \
				case 6: DRAW_PIXEL(6, Pix = bp[6]); if (!--w) break; \
				case 7: DRAW_PIXEL(7, Pix = bp[7]); break; \
			} \
		} \
	} \
	else \
	{ \
		bp = pCache + 56 - BPSTART; \
		for (l = LineCount; l > 0; l--, bp -= 8 * PITCH, Offset += GFX.PPL) \
		{ \
			w = Width; \
			switch (StartPixel) \
			{ \
				case 0: DRAW_PIXEL(0, Pix = bp[7]); if (!--w) break; \
				case 1: DRAW_PIXEL(1, Pix = bp[6]); if (!--w) break; \
				case 2: DRAW_PIXEL(2, Pix = bp[5]); if (!--w) break; \
				case 3: DRAW_PIXEL(3, Pix = bp[4]); if (!--w) break; \
				case 4: DRAW_PIXEL(4, Pix = bp[3]); if (!--w) break; \
				case 5: DRAW_PIXEL(5, Pix = bp[2]); if (!--w) break; \
				case 6: DRAW_PIXEL(6, Pix = bp[1]); if (!--w) break; \
				case 7: DRAW_PIXEL(7, Pix = bp[0]); break; \
			} \
		} \
	}

#define NAME1	DrawClippedTile16
#define ARGS	uint32 Tile, uint32 Offset, uint32 StartPixel, uint32 Width, uint32 StartLine, uint32 LineCount

// Second-level include: Get the DrawClippedTile16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Z1
#undef Z2

// Basic routine to render a single mosaic pixel.
// DRAW_PIXEL, BPSTART, Z1, Z2 and Pix are the same as above, but PITCH is not used.

#define Z1	GFX.Z1
#define Z2	GFX.Z2

#define DRAW_TILE() \
	uint8			*pCache; \
	register int32	l, w; \
	register uint8	Pix; \
	\
	GET_CACHED_TILE(); \
	if (IS_BLANK_TILE()) \
		return; \
	SELECT_PALETTE(); \
	\
	if (Tile & H_FLIP) \
		StartPixel = 7 - StartPixel; \
	\
	if (Tile & V_FLIP) \
		Pix = pCache[56 - BPSTART + StartPixel]; \
	else \
		Pix = pCache[BPSTART + StartPixel]; \
	\
	if (Pix) \
	{ \
		for (l = LineCount; l > 0; l--, Offset += GFX.PPL) \
		{ \
			for (w = Width - 1; w >= 0; w--) \
				DRAW_PIXEL(w, 1); \
		} \
	}

#define NAME1	DrawMosaicPixel16
#define ARGS	uint32 Tile, uint32 Offset, uint32 StartLine, uint32 StartPixel, uint32 Width, uint32 LineCount

// Second-level include: Get the DrawMosaicPixel16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Z1
#undef Z2

// Basic routine to render the backdrop.
// DRAW_PIXEL is the same as above, but since we're just replicating a single pixel there's no need for PITCH or BPSTART
// (or interlace at all, really).
// The backdrop is always depth = 1, so Z1 = Z2 = 1. And backdrop is always color 0.

#define NO_INTERLACE	1
#define Z1				1
#define Z2				1
#define Pix				0

#define DRAW_TILE() \
	register uint32	l, x; \
	\
	GFX.RealScreenColors = IPPU.ScreenColors; \
	GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors; \
	\
	for (l = GFX.StartY; l <= GFX.EndY; l++, Offset += GFX.PPL) \
	{ \
		for (x = Left; x < Right; x++) \
			DRAW_PIXEL(x, 1); \
	}

#define NAME1	DrawBackdrop16
#define ARGS	uint32 Offset, uint32 Left, uint32 Right

// Second-level include: Get the DrawBackdrop16 renderers.

#include "tile.cpp"

#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef Pix
#undef Z1
#undef Z2
#undef NO_INTERLACE

// Basic routine to render a chunk of a Mode 7 BG.
// Mode 7 has no interlace, so BPSTART and PITCH are unused.
// We get some new parameters, so we can use the same DRAW_TILE to do BG1 or BG2:
//     DCMODE tests if Direct Color should apply.
//     BG is the BG, so we use the right clip window.
//     MASK is 0xff or 0x7f, the 'color' portion of the pixel.
// We define Z1/Z2 to either be constant 5 or to vary depending on the 'priority' portion of the pixel.

#define CLIP_10_BIT_SIGNED(a)	(((a) & 0x2000) ? ((a) | ~0x3ff) : ((a) & 0x3ff))

extern struct SLineMatrixData	LineMatrixData[240];

#define NO_INTERLACE	1
#define Z1				(D + 7)
#define Z2				(D + 7)
#define MASK			0xff
#define DCMODE			(Memory.FillRAM[0x2130] & 1)
#define BG				0

#define DRAW_TILE_NORMAL() \
	uint8	*VRAM1 = Memory.VRAM + 1; \
	\
	if (DCMODE) \
	{ \
		if (IPPU.DirectColourMapsNeedRebuild) \
			S9xBuildDirectColourMaps(); \
		GFX.RealScreenColors = DirectColourMaps[0]; \
	} \
	else \
		GFX.RealScreenColors = IPPU.ScreenColors; \
	\
	GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors; \
	\
	int	aa, cc; \
	int	startx; \
	\
	uint32	Offset = GFX.StartY * GFX.PPL; \
	struct SLineMatrixData	*l = &LineMatrixData[GFX.StartY]; \
	\
	for (uint32 Line = GFX.StartY; Line <= GFX.EndY; Line++, Offset += GFX.PPL, l++) \
	{ \
		int	yy, starty; \
		\
		int32	HOffset = ((int32) l->M7HOFS  << 19) >> 19; \
		int32	VOffset = ((int32) l->M7VOFS  << 19) >> 19; \
		\
		int32	CentreX = ((int32) l->CentreX << 19) >> 19; \
		int32	CentreY = ((int32) l->CentreY << 19) >> 19; \
		\
		if (PPU.Mode7VFlip) \
			starty = 255 - (int) (Line + 1); \
		else \
			starty = Line + 1; \
		\
		yy = CLIP_10_BIT_SIGNED(VOffset - CentreY); \
		\
		int	BB = ((l->MatrixB * starty) & ~63) + ((l->MatrixB * yy) & ~63) + (CentreX << 8); \
		int	DD = ((l->MatrixD * starty) & ~63) + ((l->MatrixD * yy) & ~63) + (CentreY << 8); \
		\
		if (PPU.Mode7HFlip) \
		{ \
			startx = Right - 1; \
			aa = -l->MatrixA; \
			cc = -l->MatrixC; \
		} \
		else \
		{ \
			startx = Left; \
			aa = l->MatrixA; \
			cc = l->MatrixC; \
		} \
		\
		int	xx = CLIP_10_BIT_SIGNED(HOffset - CentreX); \
		int	AA = l->MatrixA * startx + ((l->MatrixA * xx) & ~63); \
		int	CC = l->MatrixC * startx + ((l->MatrixC * xx) & ~63); \
		\
		uint8	Pix; \
		\
		if (!PPU.Mode7Repeat) \
		{ \
			for (uint32 x = Left; x < Right; x++, AA += aa, CC += cc) \
			{ \
				int	X = ((AA + BB) >> 8) & 0x3ff; \
				int	Y = ((CC + DD) >> 8) & 0x3ff; \
				\
				uint8	*TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
				uint8	b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				\
				DRAW_PIXEL(x, Pix = (b & MASK)); \
			} \
		} \
		else \
		{ \
			for (uint32 x = Left; x < Right; x++, AA += aa, CC += cc) \
			{ \
				int	X = ((AA + BB) >> 8); \
				int	Y = ((CC + DD) >> 8); \
				\
				uint8	b; \
				\
				if (((X | Y) & ~0x3ff) == 0) \
				{ \
					uint8	*TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
					b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				} \
				else \
				if (PPU.Mode7Repeat == 3) \
					b = *(VRAM1    + ((Y & 7) << 4) + ((X & 7) << 1)); \
				else \
					continue; \
				\
				DRAW_PIXEL(x, Pix = (b & MASK)); \
			} \
		} \
	}

#define DRAW_TILE_MOSAIC() \
	uint8	*VRAM1 = Memory.VRAM + 1; \
	\
	if (DCMODE) \
	{ \
		if (IPPU.DirectColourMapsNeedRebuild) \
			S9xBuildDirectColourMaps(); \
		GFX.RealScreenColors = DirectColourMaps[0]; \
	} \
	else \
		GFX.RealScreenColors = IPPU.ScreenColors; \
	\
	GFX.ScreenColors = GFX.ClipColors ? BlackColourMap : GFX.RealScreenColors; \
	\
	int	aa, cc; \
	int	startx, StartY = GFX.StartY; \
	\
	int		HMosaic = 1, VMosaic = 1, MosaicStart = 0; \
	int32	MLeft = Left, MRight = Right; \
	\
	if (PPU.BGMosaic[0]) \
	{ \
		VMosaic = PPU.Mosaic; \
		MosaicStart = ((uint32) GFX.StartY - PPU.MosaicStart) % VMosaic; \
		StartY -= MosaicStart; \
	} \
	\
	if (PPU.BGMosaic[BG]) \
	{ \
		HMosaic = PPU.Mosaic; \
		MLeft  -= MLeft  % HMosaic; \
		MRight += HMosaic - 1; \
		MRight -= MRight % HMosaic; \
	} \
	\
	uint32	Offset = StartY * GFX.PPL; \
	struct SLineMatrixData	*l = &LineMatrixData[StartY]; \
	\
	for (uint32 Line = StartY; Line <= GFX.EndY; Line += VMosaic, Offset += VMosaic * GFX.PPL, l += VMosaic) \
	{ \
		if (Line + VMosaic > GFX.EndY) \
			VMosaic = GFX.EndY - Line + 1; \
		\
		int	yy, starty; \
		\
		int32	HOffset = ((int32) l->M7HOFS  << 19) >> 19; \
		int32	VOffset = ((int32) l->M7VOFS  << 19) >> 19; \
		\
		int32	CentreX = ((int32) l->CentreX << 19) >> 19; \
		int32	CentreY = ((int32) l->CentreY << 19) >> 19; \
		\
		if (PPU.Mode7VFlip) \
			starty = 255 - (int) (Line + 1); \
		else \
			starty = Line + 1; \
		\
		yy = CLIP_10_BIT_SIGNED(VOffset - CentreY); \
		\
		int	BB = ((l->MatrixB * starty) & ~63) + ((l->MatrixB * yy) & ~63) + (CentreX << 8); \
		int	DD = ((l->MatrixD * starty) & ~63) + ((l->MatrixD * yy) & ~63) + (CentreY << 8); \
		\
		if (PPU.Mode7HFlip) \
		{ \
			startx = MRight - 1; \
			aa = -l->MatrixA; \
			cc = -l->MatrixC; \
		} \
		else \
		{ \
			startx = MLeft; \
			aa = l->MatrixA; \
			cc = l->MatrixC; \
		} \
		\
		int	xx = CLIP_10_BIT_SIGNED(HOffset - CentreX); \
		int	AA = l->MatrixA * startx + ((l->MatrixA * xx) & ~63); \
		int	CC = l->MatrixC * startx + ((l->MatrixC * xx) & ~63); \
		\
		uint8	Pix; \
		uint8	ctr = 1; \
		\
		if (!PPU.Mode7Repeat) \
		{ \
			for (int32 x = MLeft; x < MRight; x++, AA += aa, CC += cc) \
			{ \
				if (--ctr) \
					continue; \
				ctr = HMosaic; \
				\
				int	X = ((AA + BB) >> 8) & 0x3ff; \
				int	Y = ((CC + DD) >> 8) & 0x3ff; \
				\
				uint8	*TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
				uint8	b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				\
				if ((Pix = (b & MASK))) \
				{ \
					for (int32 h = MosaicStart; h < VMosaic; h++) \
					{ \
						for (int32 w = x + HMosaic - 1; w >= x; w--) \
							DRAW_PIXEL(w + h * GFX.PPL, (w >= (int32) Left && w < (int32) Right)); \
					} \
				} \
			} \
		} \
		else \
		{ \
			for (int32 x = MLeft; x < MRight; x++, AA += aa, CC += cc) \
			{ \
				if (--ctr) \
					continue; \
				ctr = HMosaic; \
				\
				int	X = ((AA + BB) >> 8); \
				int	Y = ((CC + DD) >> 8); \
				\
				uint8	b; \
				\
				if (((X | Y) & ~0x3ff) == 0) \
				{ \
					uint8	*TileData = VRAM1 + (Memory.VRAM[((Y & ~7) << 5) + ((X >> 2) & ~1)] << 7); \
					b = *(TileData + ((Y & 7) << 4) + ((X & 7) << 1)); \
				} \
				else \
				if (PPU.Mode7Repeat == 3) \
					b = *(VRAM1    + ((Y & 7) << 4) + ((X & 7) << 1)); \
				else \
					continue; \
				\
				if ((Pix = (b & MASK))) \
				{ \
					for (int32 h = MosaicStart; h < VMosaic; h++) \
					{ \
						for (int32 w = x + HMosaic - 1; w >= x; w--) \
							DRAW_PIXEL(w + h * GFX.PPL, (w >= (int32) Left && w < (int32) Right)); \
					} \
				} \
			} \
		} \
		\
		MosaicStart = 0; \
	}

#define DRAW_TILE()	DRAW_TILE_NORMAL()
#define NAME1		DrawMode7BG1
#define ARGS		uint32 Left, uint32 Right, int D

// Second-level include: Get the DrawMode7BG1 renderers.

#include "tile.cpp"

#undef NAME1
#undef DRAW_TILE

#define DRAW_TILE()	DRAW_TILE_MOSAIC()
#define NAME1		DrawMode7MosaicBG1

// Second-level include: Get the DrawMode7MosaicBG1 renderers.

#include "tile.cpp"

#undef DRAW_TILE
#undef NAME1
#undef Z1
#undef Z2
#undef MASK
#undef DCMODE
#undef BG

#define NAME1		DrawMode7BG2
#define DRAW_TILE()	DRAW_TILE_NORMAL()
#define Z1			(D + ((b & 0x80) ? 11 : 3))
#define Z2			(D + ((b & 0x80) ? 11 : 3))
#define MASK		0x7f
#define DCMODE		0
#define BG			1

// Second-level include: Get the DrawMode7BG2 renderers.

#include "tile.cpp"

#undef NAME1
#undef DRAW_TILE

#define DRAW_TILE()	DRAW_TILE_MOSAIC()
#define NAME1		DrawMode7MosaicBG2

// Second-level include: Get the DrawMode7MosaicBG2 renderers.

#include "tile.cpp"

#undef MASK
#undef DCMODE
#undef BG
#undef NAME1
#undef ARGS
#undef DRAW_TILE
#undef DRAW_TILE_NORMAL
#undef DRAW_TILE_MOSAIC
#undef Z1
#undef Z2
#undef NO_INTERLACE

/*****************************************************************************/
#else
#ifndef NAME2 // Second-level: Get all the NAME1 renderers.
/*****************************************************************************/

#define BPSTART	StartLine
#define PITCH	1

// The 1x1 pixel plotter, for speedhacking modes.

#define DRAW_PIXEL(N, M) \
	if (Z1 > GFX.DB[Offset + N] && (M)) \
	{ \
		GFX.S[Offset + N] = MATH(GFX.ScreenColors[Pix], GFX.SubScreen[Offset + N], GFX.SubZBuffer[Offset + N]); \
		GFX.DB[Offset + N] = Z2; \
	}

#define NAME2	Normal1x1

// Third-level include: Get the Normal1x1 renderers.

#include "tile.cpp"

#undef NAME2
#undef DRAW_PIXEL

// The 2x1 pixel plotter, for normal rendering when we've used hires/interlace already this frame.

#define DRAW_PIXEL_N2x1(N, M) \
	if (Z1 > GFX.DB[Offset + 2 * N] && (M)) \
	{ \
		GFX.S[Offset + 2 * N] = GFX.S[Offset + 2 * N + 1] = MATH(GFX.ScreenColors[Pix], GFX.SubScreen[Offset + 2 * N], GFX.SubZBuffer[Offset + 2 * N]); \
		GFX.DB[Offset + 2 * N] = GFX.DB[Offset + 2 * N + 1] = Z2; \
	}

#define DRAW_PIXEL(N, M)	DRAW_PIXEL_N2x1(N, M)
#define NAME2				Normal2x1

// Third-level include: Get the Normal2x1 renderers.

#include "tile.cpp"

#undef NAME2
#undef DRAW_PIXEL

// Hires pixel plotter, this combines the main and subscreen pixels as appropriate to render hires or pseudo-hires images.
// Use it only on the main screen, subscreen should use Normal2x1 instead.
// Hires math:
//     Main pixel is mathed as normal: Main(x, y) * Sub(x, y).
//     Sub pixel is mathed somewhat weird: Basically, for Sub(x + 1, y) we apply the same operation we applied to Main(x, y)
//     (e.g. no math, add fixed, add1/2 subscreen) using Main(x, y) as the "corresponding subscreen pixel".
//     Also, color window clipping clips Sub(x + 1, y) if Main(x, y) is clipped, not Main(x + 1, y).
//     We don't know how Sub(0, y) is handled.

#define DRAW_PIXEL_H2x1(N, M) \
	if (Z1 > GFX.DB[Offset + 2 * N] && (M)) \
	{ \
		GFX.S[Offset + 2 * N] = MATH((GFX.ClipColors ? 0 : GFX.SubScreen[Offset + 2 * N]), GFX.RealScreenColors[Pix], GFX.SubZBuffer[Offset + 2 * N]); \
		GFX.S[Offset + 2 * N + 1] = MATH(GFX.ScreenColors[Pix], GFX.SubScreen[Offset + 2 * N], GFX.SubZBuffer[Offset + 2 * N]); \
		GFX.DB[Offset + 2 * N] = GFX.DB[Offset + 2 * N + 1] = Z2; \
	}

#define DRAW_PIXEL(N, M)	DRAW_PIXEL_H2x1(N, M)
#define NAME2				Hires

// Third-level include: Get the Hires renderers.

#include "tile.cpp"

#undef NAME2
#undef DRAW_PIXEL

// Interlace: Only draw every other line, so we'll redefine BPSTART and PITCH to do so.
// Otherwise, it's the same as Normal2x1/Hires2x1.

#undef BPSTART
#undef PITCH

#define BPSTART	(StartLine * 2 + BG.InterlaceLine)
#define PITCH	2

#ifndef NO_INTERLACE

#define DRAW_PIXEL(N, M)	DRAW_PIXEL_N2x1(N, M)
#define NAME2				Interlace

// Third-level include: Get the double width Interlace renderers.

#include "tile.cpp"

#undef NAME2
#undef DRAW_PIXEL

#define DRAW_PIXEL(N, M)	DRAW_PIXEL_H2x1(N, M)
#define NAME2				HiresInterlace

// Third-level include: Get the HiresInterlace renderers.

#include "tile.cpp"

#undef NAME2
#undef DRAW_PIXEL

#endif

#undef BPSTART
#undef PITCH

/*****************************************************************************/
#else // Third-level: Renderers for each math mode for NAME1 + NAME2.
/*****************************************************************************/

#define CONCAT3(A, B, C)	A##B##C
#define MAKENAME(A, B, C)	CONCAT3(A, B, C)

static void MAKENAME(NAME1, _, NAME2) (ARGS)
{
#define MATH(A, B, C)	NOMATH(x, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, Add_, NAME2) (ARGS)
{
#define MATH(A, B, C)	REGMATH(ADD, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, AddF1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C)	MATHF1_2(ADD, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, AddS1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C)	MATHS1_2(ADD, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, Sub_, NAME2) (ARGS)
{
#define MATH(A, B, C)	REGMATH(SUB, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, SubF1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C)	MATHF1_2(SUB, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void MAKENAME(NAME1, SubS1_2_, NAME2) (ARGS)
{
#define MATH(A, B, C)	MATHS1_2(SUB, A, B, C)
	DRAW_TILE();
#undef MATH
}

static void (*MAKENAME(Renderers_, NAME1, NAME2)[7]) (ARGS) =
{
	MAKENAME(NAME1, _, NAME2),
	MAKENAME(NAME1, Add_, NAME2),
	MAKENAME(NAME1, AddF1_2_, NAME2),
	MAKENAME(NAME1, AddS1_2_, NAME2),
	MAKENAME(NAME1, Sub_, NAME2),
	MAKENAME(NAME1, SubF1_2_, NAME2),
	MAKENAME(NAME1, SubS1_2_, NAME2)
};

#undef MAKENAME
#undef CONCAT3

#endif
#endif
#endif
