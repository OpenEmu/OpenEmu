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


#ifndef _GFX_H_
#define _GFX_H_

struct SGFX
{
	uint16	*Screen;
	uint16	*SubScreen;
	uint8	*ZBuffer;
	uint8	*SubZBuffer;
	uint32	Pitch;
	uint32	ScreenSize;
	uint16	*S;
	uint8	*DB;
	uint16	*X2;
	uint16	*ZERO;
	uint32	RealPPL;			// true PPL of Screen buffer
	uint32	PPL;				// number of pixels on each of Screen buffer
	uint32	LinesPerTile;		// number of lines in 1 tile (4 or 8 due to interlace)
	uint16	*ScreenColors;		// screen colors for rendering main
	uint16	*RealScreenColors;	// screen colors, ignoring color window clipping
	uint8	Z1;					// depth for comparison
	uint8	Z2;					// depth to save
	uint32	FixedColour;
	uint8	DoInterlace;
	uint8	InterlaceFrame;
	uint32	StartY;
	uint32	EndY;
	bool8	ClipColors;
	uint8	OBJWidths[128];
	uint8	OBJVisibleTiles[128];

	struct ClipData	*Clip;

	struct
	{
		uint8	RTOFlags;
		int16	Tiles;

		struct
		{
			int8	Sprite;
			uint8	Line;
		}	OBJ[32];
	}	OBJLines[SNES_HEIGHT_EXTENDED];

#ifdef GFX_MULTI_FORMAT
	uint32	PixelFormat;
	uint32	(*BuildPixel) (uint32, uint32, uint32);
	uint32	(*BuildPixel2) (uint32, uint32, uint32);
	void	(*DecomposePixel) (uint32, uint32 &, uint32 &, uint32 &);
#endif

	void	(*DrawBackdropMath) (uint32, uint32, uint32);
	void	(*DrawBackdropNomath) (uint32, uint32, uint32);
	void	(*DrawTileMath) (uint32, uint32, uint32, uint32);
	void	(*DrawTileNomath) (uint32, uint32, uint32, uint32);
	void	(*DrawClippedTileMath) (uint32, uint32, uint32, uint32, uint32, uint32);
	void	(*DrawClippedTileNomath) (uint32, uint32, uint32, uint32, uint32, uint32);
	void	(*DrawMosaicPixelMath) (uint32, uint32, uint32, uint32, uint32, uint32);
	void	(*DrawMosaicPixelNomath) (uint32, uint32, uint32, uint32, uint32, uint32);
	void	(*DrawMode7BG1Math) (uint32, uint32, int);
	void	(*DrawMode7BG1Nomath) (uint32, uint32, int);
	void	(*DrawMode7BG2Math) (uint32, uint32, int);
	void	(*DrawMode7BG2Nomath) (uint32, uint32, int);

	const char	*InfoString;
	uint32	InfoStringTimeout;
	char	FrameDisplayString[256];
};

struct SBG
{
	uint8	(*ConvertTile) (uint8 *, uint32, uint32);
	uint8	(*ConvertTileFlip) (uint8 *, uint32, uint32);

	uint32	TileSizeH;
	uint32	TileSizeV;
	uint32	OffsetSizeH;
	uint32	OffsetSizeV;
	uint32	TileShift;
	uint32	TileAddress;
	uint32	NameSelect;
	uint32	SCBase;

	uint32	StartPalette;
	uint32	PaletteShift;
	uint32	PaletteMask;
	uint8	EnableMath;
	uint8	InterlaceLine;

	uint8	*Buffer;
	uint8	*BufferFlip;
	uint8	*Buffered;
	uint8	*BufferedFlip;
	bool8	DirectColourMode;
};

struct SLineData
{
	struct
	{
		uint16	VOffset;
		uint16	HOffset;
	}	BG[4];
};

struct SLineMatrixData
{
	short	MatrixA;
	short	MatrixB;
	short	MatrixC;
	short	MatrixD;
	short	CentreX;
	short	CentreY;
	short	M7HOFS;
	short	M7VOFS;
};

extern uint16		BlackColourMap[256];
extern uint16		DirectColourMaps[8][256];
extern uint8		mul_brightness[16][32];
extern struct SBG	BG;
extern struct SGFX	GFX;

#define H_FLIP		0x4000
#define V_FLIP		0x8000
#define BLANK_TILE	2

#define COLOR_ADD1_2(C1, C2) \
	((((((C1) & RGB_REMOVE_LOW_BITS_MASK) + \
	((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
	((C1) & (C2) & RGB_LOW_BITS_MASK)) | ALPHA_BITS_MASK)

#define COLOR_ADD(C1, C2) \
	(GFX.X2[((((C1) & RGB_REMOVE_LOW_BITS_MASK) + \
	((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
	((C1) & (C2) & RGB_LOW_BITS_MASK)] | \
	(((C1) ^ (C2)) & RGB_LOW_BITS_MASK))

#define COLOR_SUB1_2(C1, C2) \
	GFX.ZERO[(((C1) | RGB_HI_BITS_MASKx2) - \
	((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1]

inline uint16 COLOR_SUB (uint16 C1, uint16 C2)
{
	uint16	mC1, mC2, v = ALPHA_BITS_MASK;

	mC1 = C1 & FIRST_COLOR_MASK;
	mC2 = C2 & FIRST_COLOR_MASK;
	if (mC1 > mC2) v += (mC1 - mC2);

	mC1 = C1 & SECOND_COLOR_MASK;
	mC2 = C2 & SECOND_COLOR_MASK;
	if (mC1 > mC2) v += (mC1 - mC2);

	mC1 = C1 & THIRD_COLOR_MASK;
	mC2 = C2 & THIRD_COLOR_MASK;
	if (mC1 > mC2) v += (mC1 - mC2);

	return (v);
}

void S9xStartScreenRefresh (void);
void S9xEndScreenRefresh (void);
void S9xUpdateScreen (void);
void S9xBuildDirectColourMaps (void);
void RenderLine (uint8);
void S9xComputeClipWindows (void);
void S9xDisplayChar (uint16 *, uint8);
// called automatically unless Settings.AutoDisplayMessages is false
void S9xDisplayMessages (uint16 *, int, int, int, int);
#ifdef GFX_MULTI_FORMAT
bool8 S9xSetRenderPixelFormat (int);
#endif

// external port interface which must be implemented or initialised for each port
bool8 S9xGraphicsInit (void);
void S9xGraphicsDeinit (void);
bool8 S9xInitUpdate (void);
bool8 S9xDeinitUpdate (int, int);
bool8 S9xContinueUpdate (int, int);
void S9xReRefresh (void);
void S9xSetPalette (void);
void S9xSyncSpeed (void);

// called instead of S9xDisplayString if set to non-NULL
extern void (*S9xCustomDisplayString) (const char *, int, int, bool);

#endif
