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




#ifndef _GFX_H_
#define _GFX_H_

#include "port.h"
#include "snes9x.h"

struct SGFX{
    // Initialize these variables
    uint16 *Screen;
    uint16 *SubScreen;
    uint8  *ZBuffer;
    uint8  *SubZBuffer;
    uint32 Pitch;
    uint32 ScreenSize;

    uint16 *S;
    uint8  *DB;

    // Setup in call to S9xGraphicsInit()
    uint16 *X2;
    uint16 *ZERO_OR_X2;
    uint16 *ZERO;

    uint32 RealPPL;      // True PPL of Screen buffer.
    uint32 PPL;	         // Number of pixels on each of Screen buffer
    uint32 LinesPerTile; // number of lines in 1 tile (4 or 8 due to interlace)
    uint16 *ScreenColors;     // Screen colors for rendering main
    uint16 *RealScreenColors; // Screen colors, ignoring color window clipping
    uint8  Z1;          // Depth for comparison
    uint8  Z2;          // Depth to save
    uint32 FixedColour;
    const char *InfoString;
    uint32 InfoStringTimeout;
	char   FrameDisplayString[256];
	bool8  FrameDisplay;
	bool8  Repainting; // True if the frame is being re-drawn
    uint8  DoInterlace;
    uint8  InterlaceFrame;
    uint32 StartY;
    uint32 EndY;
    struct ClipData *Clip;
    bool8  ClipColors;
    uint8  OBJWidths[128];
    uint8  OBJVisibleTiles[128];
    struct {
        uint8 RTOFlags;
        int16 Tiles;
        struct {
            int8 Sprite;
            uint8 Line;
        } OBJ[32];
    } OBJLines [SNES_HEIGHT_EXTENDED];

#ifdef GFX_MULTI_FORMAT
    uint32 PixelFormat;
    uint32 (*BuildPixel) (uint32 R, uint32 G, uint32 B);
    uint32 (*BuildPixel2) (uint32 R, uint32 G, uint32 B);
    void   (*DecomposePixel) (uint32 Pixel, uint32 &R, uint32 &G, uint32 &B);
#endif

    void (*DrawBackdropMath)(uint32,uint32,uint32);
    void (*DrawBackdropNomath)(uint32,uint32,uint32);
    void (*DrawTileMath)(uint32,uint32,uint32,uint32);
    void (*DrawTileNomath)(uint32,uint32,uint32,uint32);
    void (*DrawClippedTileMath)(uint32,uint32,uint32,uint32,uint32,uint32);
    void (*DrawClippedTileNomath)(uint32,uint32,uint32,uint32,uint32,uint32);
    void (*DrawMosaicPixelMath)(uint32,uint32,uint32,uint32,uint32,uint32);
    void (*DrawMosaicPixelNomath)(uint32,uint32,uint32,uint32,uint32,uint32);
    void (*DrawMode7BG1Math)(uint32,uint32,int);
    void (*DrawMode7BG1Nomath)(uint32,uint32,int);
    void (*DrawMode7BG2Math)(uint32,uint32,int);
    void (*DrawMode7BG2Nomath)(uint32,uint32,int);
};

struct SLineData {
    struct {
        uint16 VOffset;
        uint16 HOffset;
    } BG [4];
};

#define H_FLIP 0x4000
#define V_FLIP 0x8000
#define BLANK_TILE 2

struct SBG {
    uint8 (*ConvertTile)(uint8 *,uint32,uint32);
    uint8 (*ConvertTileFlip)(uint8 *,uint32,uint32);

    uint32 TileSizeH;
    uint32 TileSizeV;
    uint32 OffsetSizeH;
    uint32 OffsetSizeV;
    uint32 TileShift;
    uint32 TileAddress;
    uint32 NameSelect;
    uint32 SCBase;

    uint32 StartPalette;
    uint32 PaletteShift;
    uint32 PaletteMask;
    uint8  EnableMath;
    uint8  InterlaceLine;

    uint8 *Buffer, *BufferFlip;
    uint8 *Buffered, *BufferedFlip;
    bool8  DirectColourMode;
};

struct SLineMatrixData
{
    short MatrixA;
    short MatrixB;
    short MatrixC;
    short MatrixD;
    short CentreX;
    short CentreY;
    short M7HOFS;
    short M7VOFS;
};

extern SBG BG;
extern uint16 BlackColourMap [256];
extern uint16 DirectColourMaps [8][256];

extern uint8 add32_32 [32][32];
extern uint8 add32_32_half [32][32];
extern uint8 sub32_32 [32][32];
extern uint8 sub32_32_half [32][32];
extern uint8 mul_brightness [16][32];

// Could use BSWAP instruction on Intel port...
#define SWAP_DWORD(dw) dw = ((dw & 0xff) << 24) | ((dw & 0xff00) << 8) | \
		            ((dw & 0xff0000) >> 8) | ((dw & 0xff000000) >> 24)

#define SUB_SCREEN_DEPTH 0
#define MAIN_SCREEN_DEPTH 32

#if defined(OLD_COLOUR_BLENDING)
#define COLOR_ADD(C1, C2) \
GFX.X2 [((((C1) & RGB_REMOVE_LOW_BITS_MASK) + \
	  ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
	((C1) & (C2) & RGB_LOW_BITS_MASK)]
#else
#define COLOR_ADD(C1, C2) \
(GFX.X2 [((((C1) & RGB_REMOVE_LOW_BITS_MASK) + \
	  ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
	 ((C1) & (C2) & RGB_LOW_BITS_MASK)] | \
 (((C1) ^ (C2)) & RGB_LOW_BITS_MASK))
#endif

#define COLOR_ADD1_2(C1, C2) \
(((((C1) & RGB_REMOVE_LOW_BITS_MASK) + \
          ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1) + \
         ((C1) & (C2) & RGB_LOW_BITS_MASK) | ALPHA_BITS_MASK)

#if defined(OLD_COLOUR_BLENDING)
#define COLOR_SUB(C1, C2) \
GFX.ZERO_OR_X2 [(((C1) | RGB_HI_BITS_MASKx2) - \
		 ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1]
#elif !defined(NEW_COLOUR_BLENDING)
#define COLOR_SUB(C1, C2) \
(GFX.ZERO_OR_X2 [(((C1) | RGB_HI_BITS_MASKx2) - \
                  ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1] + \
((C1) & RGB_LOW_BITS_MASK) - ((C2) & RGB_LOW_BITS_MASK))
#else
inline uint16 COLOR_SUB(uint16, uint16);

inline uint16 COLOR_SUB(uint16 C1, uint16 C2)
{
	uint16	mC1, mC2, v = 0;

	mC1 = C1 & FIRST_COLOR_MASK;
	mC2 = C2 & FIRST_COLOR_MASK;
	if (mC1 > mC2) v += (mC1 - mC2);

	mC1 = C1 & SECOND_COLOR_MASK;
	mC2 = C2 & SECOND_COLOR_MASK;
	if (mC1 > mC2) v += (mC1 - mC2);

	mC1 = C1 & THIRD_COLOR_MASK;
	mC2 = C2 & THIRD_COLOR_MASK;
	if (mC1 > mC2) v += (mC1 - mC2);

	return v;
}
#endif

#define COLOR_SUB1_2(C1, C2) \
GFX.ZERO [(((C1) | RGB_HI_BITS_MASKx2) - \
	   ((C2) & RGB_REMOVE_LOW_BITS_MASK)) >> 1]

START_EXTERN_C
void S9xStartScreenRefresh ();
void S9xDrawScanLine (uint8 Line);
void S9xEndScreenRefresh ();
void S9xSetupOBJ ();
void S9xUpdateScreen ();
void RenderLine (uint8 line);
void S9xBuildDirectColourMaps ();

void S9xDisplayMessages (uint16 *screen, int ppl, int width, int height, int scale); // called automatically unless Settings.AutoDisplayMessages is false

extern struct SGFX GFX;

// External port interface which must be implemented or initialised for each
// port.
bool8 S9xGraphicsInit ();
void S9xGraphicsDeinit();
bool8 S9xInitUpdate (void);
bool8 S9xDeinitUpdate (int Width, int Height);
bool8 S9xContinueUpdate (int Width, int Height);
void S9xSetPalette ();
void S9xSyncSpeed ();

extern void (*S9xCustomDisplayString) (const char *string, int linesFromBottom, int pixelsFromLeft, bool allowWrap); // called instead of S9xDisplayString if set to non-NULL

#ifdef GFX_MULTI_FORMAT
bool8 S9xSetRenderPixelFormat (int format);
#endif

END_EXTERN_C

#endif

