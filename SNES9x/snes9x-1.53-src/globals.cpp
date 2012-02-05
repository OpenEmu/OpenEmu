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
#include "dma.h"
#include "apu/apu.h"
#include "fxinst.h"
#include "fxemu.h"
#include "srtc.h"
#include "cheats.h"
#ifdef NETPLAY_SUPPORT
#include "netplay.h"
#endif
#ifdef DEBUGGER
#include "debug.h"
#include "missing.h"
#endif

struct SCPUState		CPU;
struct SICPU			ICPU;
struct SRegisters		Registers;
struct SPPU				PPU;
struct InternalPPU		IPPU;
struct SDMA				DMA[8];
struct STimings			Timings;
struct SGFX				GFX;
struct SBG				BG;
struct SLineData		LineData[240];
struct SLineMatrixData	LineMatrixData[240];
struct SDSP0			DSP0;
struct SDSP1			DSP1;
struct SDSP2			DSP2;
struct SDSP3			DSP3;
struct SDSP4			DSP4;
struct SSA1				SA1;
struct SSA1Registers	SA1Registers;
struct FxRegs_s			GSU;
struct FxInfo_s			SuperFX;
struct SST010			ST010;
struct SST011			ST011;
struct SST018			ST018;
struct SOBC1			OBC1;
struct SSPC7110Snapshot	s7snap;
struct SSRTCSnapshot	srtcsnap;
struct SRTCData			RTCData;
struct SBSX				BSX;
struct SMulti			Multi;
struct SSettings		Settings;
struct SSNESGameFixes	SNESGameFixes;
#ifdef NETPLAY_SUPPORT
struct SNetPlay			NetPlay;
#endif
#ifdef DEBUGGER
struct Missing			missing;
#endif
struct SCheatData		Cheat;
struct Watch			watches[16];
CMemory					Memory;

char	String[513];
uint8	OpenBus = 0;
uint8	*HDMAMemPointers[8];
uint16	BlackColourMap[256];
uint16	DirectColourMaps[8][256];

SnesModel	M1SNES = { 1, 3, 2 };
SnesModel	M2SNES = { 2, 4, 3 };
SnesModel	*Model = &M1SNES;

#ifdef GFX_MULTI_FORMAT
uint32	RED_LOW_BIT_MASK           = RED_LOW_BIT_MASK_RGB565;
uint32	GREEN_LOW_BIT_MASK         = GREEN_LOW_BIT_MASK_RGB565;
uint32	BLUE_LOW_BIT_MASK          = BLUE_LOW_BIT_MASK_RGB565;
uint32	RED_HI_BIT_MASK            = RED_HI_BIT_MASK_RGB565;
uint32	GREEN_HI_BIT_MASK          = GREEN_HI_BIT_MASK_RGB565;
uint32	BLUE_HI_BIT_MASK           = BLUE_HI_BIT_MASK_RGB565;
uint32	MAX_RED                    = MAX_RED_RGB565;
uint32	MAX_GREEN                  = MAX_GREEN_RGB565;
uint32	MAX_BLUE                   = MAX_BLUE_RGB565;
uint32	SPARE_RGB_BIT_MASK         = SPARE_RGB_BIT_MASK_RGB565;
uint32	GREEN_HI_BIT               = (MAX_GREEN_RGB565 + 1) >> 1;
uint32	RGB_LOW_BITS_MASK          = (RED_LOW_BIT_MASK_RGB565 | GREEN_LOW_BIT_MASK_RGB565 | BLUE_LOW_BIT_MASK_RGB565);
uint32	RGB_HI_BITS_MASK           = (RED_HI_BIT_MASK_RGB565  | GREEN_HI_BIT_MASK_RGB565  | BLUE_HI_BIT_MASK_RGB565);
uint32	RGB_HI_BITS_MASKx2         = (RED_HI_BIT_MASK_RGB565  | GREEN_HI_BIT_MASK_RGB565  | BLUE_HI_BIT_MASK_RGB565) << 1;
uint32	RGB_REMOVE_LOW_BITS_MASK   = ~RGB_LOW_BITS_MASK;
uint32	FIRST_COLOR_MASK           = FIRST_COLOR_MASK_RGB565;
uint32	SECOND_COLOR_MASK          = SECOND_COLOR_MASK_RGB565;
uint32	THIRD_COLOR_MASK           = THIRD_COLOR_MASK_RGB565;
uint32	ALPHA_BITS_MASK            = ALPHA_BITS_MASK_RGB565;
uint32	FIRST_THIRD_COLOR_MASK     = 0;
uint32	TWO_LOW_BITS_MASK          = 0;
uint32	HIGH_BITS_SHIFTED_TWO_MASK = 0;
#endif

uint16 SignExtend[2] =
{
	0x0000,
	0xff00
};

int HDMA_ModeByteCounts[8] =
{
	1, 2, 2, 4, 4, 4, 2, 4
};

uint8 mul_brightness[16][32] =
{
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02 },
	{ 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,
	  0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04 },
	{ 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03,
	  0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06 },
	{ 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x04,
	  0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08 },
	{ 0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05,
	  0x05, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0a },
	{ 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06,
	  0x06, 0x07, 0x07, 0x08, 0x08, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c },
	{ 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
	  0x07, 0x08, 0x08, 0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0e },
	{ 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07, 0x08,
	  0x09, 0x09, 0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x10, 0x11 },
	{ 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x08, 0x08, 0x09,
	  0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f, 0x10, 0x10, 0x11, 0x11, 0x12, 0x13 },
	{ 0x00, 0x01, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x09, 0x0a,
	  0x0b, 0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x11, 0x12, 0x13, 0x13, 0x14, 0x15 },
	{ 0x00, 0x01, 0x01, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b,
	  0x0c, 0x0c, 0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17 },
	{ 0x00, 0x01, 0x02, 0x02, 0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c,
	  0x0d, 0x0e, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x16, 0x16, 0x17, 0x18, 0x19 },
	{ 0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0d,
	  0x0e, 0x0f, 0x10, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x17, 0x18, 0x19, 0x1a, 0x1b },
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	  0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d },
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f }
};

uint8 S9xOpLengthsM0X0[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 0
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 1
	3, 2, 4, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 2
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 3
	1, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 4
	2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, // 5
	1, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 6
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 7
	2, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 8
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 9
	3, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // A
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // B
	3, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // C
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // D
	3, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // E
	2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4  // F
};

uint8 S9xOpLengthsM0X1[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 0
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 1
	3, 2, 4, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 2
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 3
	1, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 4
	2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, // 5
	1, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 6
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 7
	2, 2, 3, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 8
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 9
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // A
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // B
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // C
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // D
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // E
	2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4  // F
};

uint8 S9xOpLengthsM1X0[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 0
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 1
	3, 2, 4, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 2
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 3
	1, 2, 2, 2, 3, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 4
	2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, // 5
	1, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 6
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 7
	2, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 8
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 9
	3, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // A
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // B
	3, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // C
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // D
	3, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // E
	2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4  // F
};

uint8 S9xOpLengthsM1X1[256] =
{
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 0
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 1
	3, 2, 4, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 2
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 3
	1, 2, 2, 2, 3, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 4
	2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 4, 3, 3, 4, // 5
	1, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 6
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 7
	2, 2, 3, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // 8
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // 9
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // A
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // B
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // C
	2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4, // D
	2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 3, 3, 3, 4, // E
	2, 2, 2, 2, 3, 2, 2, 2, 1, 3, 1, 1, 3, 3, 3, 4  // F
};
