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



#include <numeric>

#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <ctype.h>
#include <assert.h>

#ifdef __linux
#include <unistd.h>
#endif

#ifdef __W32_HEAP
#include <malloc.h>
#endif

#include "memmap.h"
#include "display.h"
#include "cpuexec.h"
#include "ppu.h"
#include "apu.h"
#include "dsp1.h"
#include "sa1.h"
#include "sdd1.h"
#include "spc7110.h"
#include "seta.h"
#include "srtc.h"
#include "bsx.h"
#include "reader.h"
#include "cheats.h"
#include "controls.h"
#include "movie.h"

#ifndef ZSNES_FX
#include "fxemu.h"
#endif

#ifdef UNZIP_SUPPORT
#include "unzip/unzip.h"
#endif

#ifdef JMA_SUPPORT
#include "jma/s9x-jma.h"
#endif

#ifdef __WIN32__
#ifndef _XBOX
#include "win32/wsnes9x.h" // FIXME: shouldn't be necessary
#endif
#endif

#ifdef __WIN32__
#define snprintf _snprintf // needs ANSI compliant name
#endif

#ifndef SET_UI_COLOR
#define SET_UI_COLOR(r,g,b) ;
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef ZSNES_FX
extern struct FxInit_s SuperFX;
#else
EXTERN_C uint8 *SFXPlotTable;
#endif

static bool8	stopMovie = TRUE;
static char		LastRomFilename[_MAX_PATH + 1] = "";

static const uint32	crc32Table[256] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static void S9xDeinterleaveType1 (int, uint8 *);
static void S9xDeinterleaveType2 (int, uint8 *);
static void S9xDeinterleaveGD24 (int, uint8 *);
static bool8 allASCII (uint8 *, int);
static bool8 is_SufamiTurbo_BIOS (uint8 *, uint32);
static bool8 is_SufamiTurbo_Cart (uint8 *, uint32);
static bool8 is_SameGame_BIOS (uint8 *, uint32);
static bool8 is_SameGame_Add_On (uint8 *, uint32);
static uint32 caCRC32 (uint8 *, uint32, uint32 crc32 = 0xffffffff);
static long ReadInt (Reader *, unsigned);
static bool8 ReadIPSPatch (Reader *, long, int32 &);
static int unzFindExtension (unzFile &, const char *, bool restart = TRUE, bool print = TRUE);

// deinterleave

static void S9xDeinterleaveType1 (int size, uint8 *base)
{
	if (Settings.DisplayColor == 0xffff)
	{
		Settings.DisplayColor = BUILD_PIXEL(0, 31, 0);
		SET_UI_COLOR(0, 255, 0);
	}

	uint8	blocks[256];
	int		nblocks = size >> 16;

	for (int i = 0; i < nblocks; i++)
	{
		blocks[i * 2] = i + nblocks;
		blocks[i * 2 + 1] = i;
	}

	uint8	*tmp = (uint8 *) malloc(0x8000);
	if (tmp)
	{
		for (int i = 0; i < nblocks * 2; i++)
		{
			for (int j = i; j < nblocks * 2; j++)
			{
				if (blocks[j] == i)
				{
					memmove(tmp, &base[blocks[j] * 0x8000], 0x8000);
					memmove(&base[blocks[j] * 0x8000], &base[blocks[i] * 0x8000], 0x8000);
					memmove(&base[blocks[i] * 0x8000], tmp, 0x8000);
					uint8	b = blocks[j];
					blocks[j] = blocks[i];
					blocks[i] = b;
					break;
				}
			}
		}

		free(tmp);
	}
}

static void S9xDeinterleaveType2 (int size, uint8 *base)
{
	// for odd Super FX images
	if (Settings.DisplayColor == 0xffff || Settings.DisplayColor == BUILD_PIXEL(0, 31, 0))
	{
		Settings.DisplayColor = BUILD_PIXEL(31, 14, 6);
		SET_UI_COLOR(255, 119, 25);
	}

	uint8	blocks[256];
	int		nblocks = size >> 16;
	int		step = 64;

	while (nblocks <= step)
		step >>= 1;
	nblocks = step;

	for (int i = 0; i < nblocks * 2; i++)
		blocks[i] = (i & ~0xf) | ((i & 3) << 2) | ((i & 12) >> 2);

	uint8	*tmp = (uint8 *) malloc(0x10000);
	if (tmp)
	{
		for (int i = 0; i < nblocks * 2; i++)
		{
			for (int j = i; j < nblocks * 2; j++)
			{
				if (blocks[j] == i)
				{
					memmove(tmp, &base[blocks[j] * 0x10000], 0x10000);
					memmove(&base[blocks[j] * 0x10000], &base[blocks[i] * 0x10000], 0x10000);
					memmove(&base[blocks[i] * 0x10000], tmp, 0x10000);
					uint8	b = blocks[j];
					blocks[j] = blocks[i];
					blocks[i] = b;
					break;
				}
			}
		}

		free(tmp);
	}
}

static void S9xDeinterleaveGD24 (int size, uint8 *base)
{
	// for 24Mb images dumped with Game Doctor
	if (size != 0x300000)
		return;

	if (Settings.DisplayColor == 0xffff)
	{
		Settings.DisplayColor = BUILD_PIXEL(0, 31, 31);
		SET_UI_COLOR(0, 255, 255);
	}

	uint8	*tmp = (uint8 *) malloc(0x80000);
	if (tmp)
	{
		memmove(tmp, &base[0x180000], 0x80000);
		memmove(&base[0x180000], &base[0x200000], 0x80000);
		memmove(&base[0x200000], &base[0x280000], 0x80000);
		memmove(&base[0x280000], tmp, 0x80000);

		free(tmp);

		S9xDeinterleaveType1(size, base);
	}
}

// allocation and deallocation

bool8 CMemory::Init (void)
{
    RAM	 = (uint8 *) malloc(0x20000);
    SRAM = (uint8 *) malloc(0x20000);
    VRAM = (uint8 *) malloc(0x10000);
    ROM  = (uint8 *) malloc(MAX_ROM_SIZE + 0x200 + 0x8000);

	IPPU.TileCache[TILE_2BIT]       = (uint8 *) malloc(MAX_2BIT_TILES * 64);
	IPPU.TileCache[TILE_4BIT]       = (uint8 *) malloc(MAX_4BIT_TILES * 64);
	IPPU.TileCache[TILE_8BIT]       = (uint8 *) malloc(MAX_8BIT_TILES * 64);
	IPPU.TileCache[TILE_2BIT_EVEN]  = (uint8 *) malloc(MAX_2BIT_TILES * 64);
	IPPU.TileCache[TILE_2BIT_ODD]   = (uint8 *) malloc(MAX_2BIT_TILES * 64);
	IPPU.TileCache[TILE_4BIT_EVEN]  = (uint8 *) malloc(MAX_4BIT_TILES * 64);
	IPPU.TileCache[TILE_4BIT_ODD]   = (uint8 *) malloc(MAX_4BIT_TILES * 64);

	IPPU.TileCached[TILE_2BIT]      = (uint8 *) malloc(MAX_2BIT_TILES);
	IPPU.TileCached[TILE_4BIT]      = (uint8 *) malloc(MAX_4BIT_TILES);
	IPPU.TileCached[TILE_8BIT]      = (uint8 *) malloc(MAX_8BIT_TILES);
	IPPU.TileCached[TILE_2BIT_EVEN] = (uint8 *) malloc(MAX_2BIT_TILES);
	IPPU.TileCached[TILE_2BIT_ODD]  = (uint8 *) malloc(MAX_2BIT_TILES);
	IPPU.TileCached[TILE_4BIT_EVEN] = (uint8 *) malloc(MAX_4BIT_TILES);
	IPPU.TileCached[TILE_4BIT_ODD]  = (uint8 *) malloc(MAX_4BIT_TILES);

	if (!RAM || !SRAM || !VRAM || !ROM ||
		!IPPU.TileCache[TILE_2BIT]       ||
		!IPPU.TileCache[TILE_4BIT]       ||
		!IPPU.TileCache[TILE_8BIT]       ||
		!IPPU.TileCache[TILE_2BIT_EVEN]  ||
		!IPPU.TileCache[TILE_2BIT_ODD]   ||
		!IPPU.TileCache[TILE_4BIT_EVEN]  ||
		!IPPU.TileCache[TILE_4BIT_ODD]   ||
		!IPPU.TileCached[TILE_2BIT]      ||
		!IPPU.TileCached[TILE_4BIT]      ||
		!IPPU.TileCached[TILE_8BIT]      ||
		!IPPU.TileCached[TILE_2BIT_EVEN] ||
		!IPPU.TileCached[TILE_2BIT_ODD]  ||
		!IPPU.TileCached[TILE_4BIT_EVEN] ||
		!IPPU.TileCached[TILE_4BIT_ODD])
    {
		Deinit();
		return (FALSE);
    }

	ZeroMemory(RAM,  0x20000);
	ZeroMemory(SRAM, 0x20000);
	ZeroMemory(VRAM, 0x10000);
	ZeroMemory(ROM,  MAX_ROM_SIZE + 0x200 + 0x8000);

	ZeroMemory(IPPU.TileCache[TILE_2BIT],       MAX_2BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_4BIT],       MAX_4BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_8BIT],       MAX_8BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_2BIT_EVEN],  MAX_2BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_2BIT_ODD],   MAX_2BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_4BIT_EVEN],  MAX_4BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_4BIT_ODD],   MAX_4BIT_TILES * 64);

	ZeroMemory(IPPU.TileCached[TILE_2BIT],      MAX_2BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_4BIT],      MAX_4BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_8BIT],      MAX_8BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_2BIT_EVEN], MAX_2BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_2BIT_ODD],  MAX_2BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_4BIT_EVEN], MAX_4BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_4BIT_ODD],  MAX_4BIT_TILES);

	// FillRAM uses first 32K of ROM image area, otherwise space just
	// wasted. Might be read by the SuperFX code.

	FillRAM = ROM;

	// Add 0x8000 to ROM image pointer to stop SuperFX code accessing
	// unallocated memory (can cause crash on some ports).

	ROM += 0x8000;

	C4RAM   = ROM + 0x400000 + 8192 * 8; // C4
	BIOSROM = ROM + 0x300000; // BS
	BSRAM   = ROM + 0x400000; // BS

#if defined(ZSNES_FX) || defined(ZSNES_C4)
    ::ROM    = ROM;
    ::SRAM   = SRAM;
    ::RegRAM = FillRAM;
#endif

#ifdef ZSNES_FX
	SFXPlotTable = ROM + 0x400000;
#else
	SuperFX.pvRegisters = FillRAM + 0x3000;
	SuperFX.nRamBanks   = 2; // Most only use 1.  1=64KB=512Mb, 2=128KB=1024Mb
	SuperFX.pvRam       = SRAM;
	SuperFX.nRomBanks   = (2 * 1024 * 1024) / (32 * 1024);
	SuperFX.pvRom       = (uint8 *) ROM;
#endif

	SDD1Data  = NULL;
	SDD1Index = NULL;

	PostRomInitFunc = NULL;

	return (TRUE);
}

void CMemory::Deinit (void)
{
	// XXX: Please remove port specific codes
#ifdef __W32_HEAP
	if (_HEAPOK! = _heapchk())
		MessageBox(GUI.hWnd, "CMemory::Deinit", "Heap Corrupt", MB_OK);
#endif

	if (RAM)
	{
		free(RAM);
		RAM = NULL;
	}

	if (SRAM)
	{
		free(SRAM);
		SRAM = NULL;
	}

	if (VRAM)
	{
		free(VRAM);
		VRAM = NULL;
	}

	if (ROM)
	{
		ROM -= 0x8000;
		free(ROM);
		ROM = NULL;
	}

	for (int t = 0; t < 7; t++)
	{
		if (IPPU.TileCache[t])
		{
			free(IPPU.TileCache[t]);
			IPPU.TileCache[t] = NULL;
		}

		if (IPPU.TileCached[t])
		{
			free(IPPU.TileCached[t]);
			IPPU.TileCached[t] = NULL;
		}
	}

	FreeSDD1Data();

	Safe(NULL);
	SafeANK(NULL);
}

void CMemory::FreeSDD1Data (void)
{
	if (SDD1Index)
	{
		free(SDD1Index);
		SDD1Index = NULL;
	}

	if (SDD1Data)
	{
		free(SDD1Data);
		SDD1Data = NULL;
	}
}

// file management and ROM detection

static bool8 allASCII (uint8 *b, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (b[i] < 32 || b[i] > 126)
			return (FALSE);
	}

	return (TRUE);
}

static bool8 is_SufamiTurbo_BIOS (uint8 *data, uint32 size)
{
	if (size == 0x40000 &&
		strncmp((char *) data, "BANDAI SFC-ADX", 14) == 0 && strncmp((char * ) (data + 0x10), "SFC-ADX BACKUP", 14) == 0)
		return (TRUE);
	else
		return (FALSE);
}

static bool8 is_SufamiTurbo_Cart (uint8 *data, uint32 size)
{
	if (size >= 0x80000 && size <= 0x100000 &&
		strncmp((char *) data, "BANDAI SFC-ADX", 14) == 0 && strncmp((char * ) (data + 0x10), "SFC-ADX BACKUP", 14) != 0)
		return (TRUE);
	else
		return (FALSE);
}

static bool8 is_SameGame_BIOS (uint8 *data, uint32 size)
{
	if (size == 0x100000 && strncmp((char *) (data + 0xffc0), "Same Game Tsume Game", 20) == 0)
		return (TRUE);
	else
		return (FALSE);
}

static bool8 is_SameGame_Add_On (uint8 *data, uint32 size)
{
	if (size == 0x80000)
		return (TRUE);
	else
		return (FALSE);
}

int CMemory::ScoreHiROM (bool8 skip_header, int32 romoff)
{
	uint8	*buf = ROM + 0xff00 + romoff + (skip_header ? 0x200 : 0);
	int		score = 0;

	if (buf[0xd5] & 0x1)
		score += 2;

	// Mode23 is SA-1
	if (buf[0xd5] == 0x23)
		score -= 2;

	if (buf[0xd4] == 0x20)
		score += 2;

	if ((buf[0xdc] + (buf[0xdd] << 8)) + (buf[0xde] + (buf[0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if (0 != (buf[0xde] + (buf[0xdf] << 8)))
			score++;
	}

	if (buf[0xda] == 0x33)
		score += 2;

	if ((buf[0xd5] & 0xf) < 4)
		score += 2;

	if (!(buf[0xfd] & 0x80))
		score -= 6;

	if ((buf[0xfc] + (buf[0xfd] << 8)) > 0xffb0)
		score -= 2; // reduced after looking at a scan by Cowering

	if (CalculatedSize > 1024 * 1024 * 3)
		score += 4;

	if ((1 << (buf[0xd7] - 7)) > 48)
		score -= 1;

	if (!allASCII(&buf[0xb0], 6))
		score -= 1;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

int CMemory::ScoreLoROM (bool8 skip_header, int32 romoff)
{
	uint8	*buf = ROM + 0x7f00 + romoff + (skip_header ? 0x200 : 0);
	int		score = 0;

	if (!(buf[0xd5] & 0x1))
		score += 3;

	// Mode23 is SA-1
	if (buf[0xd5] == 0x23)
		score += 2;

	if ((buf[0xdc] + (buf[0xdd] << 8)) + (buf[0xde] + (buf[0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if (0 != (buf[0xde] + (buf[0xdf] << 8)))
			score++;
	}

	if (buf[0xda] == 0x33)
		score += 2;

	if ((buf[0xd5] & 0xf) < 4)
		score += 2;

	if (!(buf[0xfd] & 0x80))
		score -= 6;

	if ((buf[0xfc] + (buf[0xfd] << 8)) > 0xffb0)
		score -= 2; // reduced per Cowering suggestion

	if (CalculatedSize <= 1024 * 1024 * 16)
		score += 2;

	if ((1 << (buf[0xd7] - 7)) > 48)
		score -= 1;

	if (!allASCII(&buf[0xb0], 6))
		score -= 1;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

uint32 CMemory::HeaderRemove (uint32 size, int32 &headerCount, uint8 *buf)
{
	uint32	calc_size = (size / 0x2000) * 0x2000;

	if ((size - calc_size == 512 && !Settings.ForceNoHeader) || Settings.ForceHeader)
	{
		uint8	*NSRTHead = buf + 0x1D0; // NSRT Header Location

		// detect NSRT header
		if (!strncmp("NSRT", (char *) &NSRTHead[24], 4))
		{
			if (NSRTHead[28] == 22)
			{
				if (((std::accumulate(NSRTHead, NSRTHead + sizeof(NSRTHeader), 0) & 0xFF) == NSRTHead[30]) &&
					(NSRTHead[30] + NSRTHead[31] == 255) && ((NSRTHead[0] & 0x0F) <= 13) &&
					(((NSRTHead[0] & 0xF0) >> 4) <= 3) && ((NSRTHead[0] & 0xF0) >> 4))
					memcpy(NSRTHeader, NSRTHead, sizeof(NSRTHeader));
			}
		}

		memmove(buf, buf + 512, calc_size);
		headerCount++;
		size -= 512;
	}

	return (size);
}

uint32 CMemory::FileLoader (uint8 *buffer, const char *filename, int32 maxsize)
{
	// <- ROM size without header
	// ** Memory.HeaderCount
	// ** Memory.ROMFilename

	int32	totalSize = 0;
    char	fname[_MAX_PATH + 1];
    char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], name[_MAX_FNAME + 1], exts[_MAX_EXT + 1];
	char	*ext;

#if defined(__WIN32__) || defined(__MACOSX__)
	ext = &exts[1];
#else
	ext = &exts[0];
#endif

	memset(NSRTHeader, 0, sizeof(NSRTHeader));
	HeaderCount = 0;

	_splitpath(filename, drive, dir, name, exts);
	_makepath(fname, drive, dir, name, exts);

	int	nFormat = FILE_DEFAULT;
	if (strcasecmp(ext, "zip") == 0)
		nFormat = FILE_ZIP;
	else
	if (strcasecmp(ext, "jma") == 0)
		nFormat = FILE_JMA;

	switch (nFormat)
	{
		case FILE_ZIP:
		{
#ifdef UNZIP_SUPPORT
			if (!LoadZip(fname, &totalSize, &HeaderCount, buffer))
			{
			 	S9xMessage(S9X_ERROR, S9X_ROM_INFO, "Invalid Zip archive.");
				return (0);
			}

			strcpy(ROMFilename, fname);
#else
			S9xMessage(S9X_ERROR, S9X_ROM_INFO, "This binary was not created with Zip support.");
			return (0);
#endif
			break;
		}

		case FILE_JMA:
		{
#ifdef JMA_SUPPORT
			size_t	size = load_jma_file(fname, buffer);
			if (!size)
			{
			 	S9xMessage(S9X_ERROR, S9X_ROM_INFO, "Invalid JMA archive.");
				return (0);
			}

			totalSize = HeaderRemove(size, HeaderCount, buffer);

			strcpy(ROMFilename, fname);
#else
			S9xMessage(S9X_ERROR, S9X_ROM_INFO, "This binary was not created with JMA support.");
			return (0);
#endif
			break;
		}

		case FILE_DEFAULT:
		default:
		{
			STREAM	fp = OPEN_STREAM(fname, "rb");
			if (!fp)
				return (0);

			strcpy(ROMFilename, fname);

			int 	len  = 0;
			uint32	size = 0;
			bool8	more = FALSE;
			uint8	*ptr = buffer;

			do
			{
				size = READ_STREAM(ptr, maxsize + 0x200 - (ptr - buffer), fp);
				CLOSE_STREAM(fp);

				size = HeaderRemove(size, HeaderCount, ptr);
				totalSize += size;
				ptr += size;

				// check for multi file roms
				if (ptr - buffer < maxsize + 0x200 &&
					(isdigit(ext[0]) && ext[1] == 0 && ext[0] < '9'))
				{
					more = TRUE;
					ext[0]++;
					_makepath(fname, drive, dir, name, exts);
				}
				else
				if (ptr - buffer < maxsize + 0x200 &&
					(((len = strlen(name)) == 7 || len == 8) &&
					strncasecmp(name, "sf", 2) == 0 &&
					isdigit(name[2]) && isdigit(name[3]) && isdigit(name[4]) && isdigit(name[5]) &&
					isalpha(name[len - 1])))
				{
					more = TRUE;
					name[len - 1]++;
					_makepath(fname, drive, dir, name, exts);
				}
				else
					more = FALSE;

			}	while (more && (fp = OPEN_STREAM(fname, "rb")) != NULL);

			break;
		}
	}

    if (HeaderCount == 0)
		S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "No ROM file header found.");
    else
    if (HeaderCount == 1)
		S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "Found ROM file header (and ignored it).");
	else
		S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "Found multiple ROM file headers (and ignored them).");

	return ((uint32) totalSize);
}

bool8 CMemory::LoadROM (const char *filename)
{
	int	retry_count = 0;

	if (!filename || !*filename)
		return (FALSE);

	ZeroMemory(ROM, MAX_ROM_SIZE);
	ZeroMemory(&Multi, sizeof(Multi));
 
again:
	Settings.DisplayColor = 0xffff;
	SET_UI_COLOR(255, 255, 255);

	CalculatedSize = 0;
	ExtendedFormat = NOPE;

	int32 totalFileSize;

	totalFileSize = FileLoader(ROM, filename, MAX_ROM_SIZE);
	if (!totalFileSize)
		return (FALSE);

	if (!Settings.NoPatch)
		CheckForIPSPatch(filename, HeaderCount != 0, totalFileSize);

	int	hi_score, lo_score;

	hi_score = ScoreHiROM(FALSE);
	lo_score = ScoreLoROM(FALSE);

	if (HeaderCount == 0 && !Settings.ForceNoHeader &&
		((hi_score >  lo_score && ScoreHiROM(TRUE) > hi_score) ||
		 (hi_score <= lo_score && ScoreLoROM(TRUE) > lo_score)))
	{
		memmove(ROM, ROM + 512, totalFileSize - 512);
		totalFileSize -= 512;
		S9xMessage(S9X_INFO, S9X_HEADER_WARNING, "Try 'force no-header' option if the game doesn't work");
		// modifying ROM, so we need to rescore
		hi_score = ScoreHiROM(FALSE);
		lo_score = ScoreLoROM(FALSE);
	}

	CalculatedSize = (totalFileSize / 0x2000) * 0x2000;

	if (CalculatedSize > 0x400000 &&
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x4332 && // exclude S-DD1
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x4532 &&
		(ROM[0xffd5] + (ROM[0xffd6] << 8)) != 0xF93a && // exclude SPC7110
		(ROM[0xffd5] + (ROM[0xffd6] << 8)) != 0xF53a)
		ExtendedFormat = YEAH;

	// if both vectors are invalid, it's type 1 LoROM
	if (ExtendedFormat == NOPE &&
		((ROM[0x7ffc] + (ROM[0x7ffd] << 8)) < 0x8000) &&
		((ROM[0xfffc] + (ROM[0xfffd] << 8)) < 0x8000))
	{
		if (Settings.DisplayColor == 0xffff)
		{
			Settings.DisplayColor = BUILD_PIXEL(0, 31, 0);
			SET_UI_COLOR(0, 255, 0);
		}

		if (!Settings.ForceInterleaved)
			S9xDeinterleaveType1(totalFileSize, ROM);
	}

	// CalculatedSize is now set, so rescore
	hi_score = ScoreHiROM(FALSE);
	lo_score = ScoreLoROM(FALSE);

	uint8	*RomHeader = ROM;

	if (ExtendedFormat != NOPE)
	{
		int	swappedhirom, swappedlorom;

		swappedhirom = ScoreHiROM(FALSE, 0x400000);
		swappedlorom = ScoreLoROM(FALSE, 0x400000);

		// set swapped here
		if (max(swappedlorom, swappedhirom) >= max(lo_score, hi_score))
		{
			ExtendedFormat = BIGFIRST;
			hi_score = swappedhirom;
			lo_score = swappedlorom;
			RomHeader += 0x400000;
		}
		else
			ExtendedFormat = SMALLFIRST;
	}

	bool8	interleaved, tales = FALSE;

    interleaved = Settings.ForceInterleaved || Settings.ForceInterleaved2;

	if (Settings.ForceLoROM || (!Settings.ForceHiROM && lo_score >= hi_score))
	{
		LoROM = TRUE;
		HiROM = FALSE;

		// ignore map type byte if not 0x2x or 0x3x
		if ((RomHeader[0x7fd5] & 0xf0) == 0x20 || (RomHeader[0x7fd5] & 0xf0) == 0x30)
		{
			switch (RomHeader[0x7fd5] & 0xf)
			{
				case 1:
					interleaved = TRUE;
					break;

				case 5:
					interleaved = TRUE;
					tales = TRUE;
					break;
			}
		}
	}
	else
	{
		LoROM = FALSE;
		HiROM = TRUE;

		if ((RomHeader[0xffd5] & 0xf0) == 0x20 || (RomHeader[0xffd5] & 0xf0) == 0x30)
		{
			switch (RomHeader[0xffd5] & 0xf)
			{
				case 0:
				case 3:
					interleaved = TRUE;
					break;
			}
		}
	}

	// this two games fail to be detected
	if (!Settings.ForceHiROM && !Settings.ForceLoROM)
	{
		if (strncmp((char *) &ROM[0x7fc0], "YUYU NO QUIZ DE GO!GO!", 22) == 0 ||
		   (strncmp((char *) &ROM[0xffc0], "BATMAN--REVENGE JOKER",  21) == 0))
		{
			LoROM = TRUE;
			HiROM = FALSE;
			interleaved = FALSE;
			tales = FALSE;
		}
	}

	if (!Settings.ForceNotInterleaved && interleaved)
	{
		S9xMessage(S9X_INFO, S9X_ROM_INTERLEAVED_INFO, "ROM image is in interleaved format - converting...");

		if (tales)
		{
			if (ExtendedFormat == BIGFIRST)
			{
				S9xDeinterleaveType1(0x400000, ROM);
				S9xDeinterleaveType1(CalculatedSize - 0x400000, ROM + 0x400000);
			}
			else
			{
				S9xDeinterleaveType1(CalculatedSize - 0x400000, ROM);
				S9xDeinterleaveType1(0x400000, ROM + CalculatedSize - 0x400000);
			}

			LoROM = FALSE;
			HiROM = TRUE;
		}
		else
		if (Settings.ForceInterleaveGD24 && CalculatedSize == 0x300000)
		{
			bool8	t = LoROM;
			LoROM = HiROM;
			HiROM = t;
			S9xDeinterleaveGD24(CalculatedSize, ROM);
		}
		else
		if (Settings.ForceInterleaved2)
			S9xDeinterleaveType2(CalculatedSize, ROM);
		else
		{
			bool8	t = LoROM;
			LoROM = HiROM;
			HiROM = t;
			S9xDeinterleaveType1(CalculatedSize, ROM);
		}

		hi_score = ScoreHiROM(FALSE);
		lo_score = ScoreLoROM(FALSE);

		if ((HiROM && (lo_score >= hi_score || hi_score < 0)) ||
			(LoROM && (hi_score >  lo_score || lo_score < 0)))
		{
			if (retry_count == 0)
			{
				S9xMessage(S9X_INFO, S9X_ROM_CONFUSING_FORMAT_INFO, "ROM lied about its type! Trying again.");
				Settings.ForceNotInterleaved = TRUE;
				Settings.ForceInterleaved = FALSE;
				retry_count++;
				goto again;
			}
		}
    }

	if (ExtendedFormat == SMALLFIRST)
		tales = TRUE;

	if (tales)
	{
		uint8	*tmp = (uint8 *) malloc(CalculatedSize - 0x400000);
		if (tmp)
		{
			S9xMessage(S9X_INFO, S9X_ROM_INTERLEAVED_INFO, "Fixing swapped ExHiROM...");
			memmove(tmp, ROM, CalculatedSize - 0x400000);
			memmove(ROM, ROM + CalculatedSize - 0x400000, 0x400000);
			memmove(ROM + 0x400000, tmp, CalculatedSize - 0x400000);
			free(tmp);
		}
	}

	if (strncmp(LastRomFilename, filename, _MAX_PATH))
	{
		strncpy(LastRomFilename, filename, _MAX_PATH);
		LastRomFilename[_MAX_PATH] = 0;
	}

	FreeSDD1Data();
	if (CleanUp7110)
		(*CleanUp7110)();

	ZeroMemory(&SNESGameFixes, sizeof(SNESGameFixes));
	SNESGameFixes.SRAMInitialValue = 0x60;

	S9xLoadCheatFile(S9xGetFilename(".cht", PATCH_DIR));

	InitROM();

	S9xInitCheatData();
	S9xApplyCheats();

	S9xReset();

    return (TRUE);
}

bool8 CMemory::LoadMultiCart (const char *cartA, const char *cartB)
{
	bool8	r = TRUE;

	ZeroMemory(ROM, MAX_ROM_SIZE);
	ZeroMemory(&Multi, sizeof(Multi));

	Settings.DisplayColor = 0xffff;
	SET_UI_COLOR(255, 255, 255);

	CalculatedSize = 0;
	ExtendedFormat = NOPE;

	if (cartA && cartA[0])
		Multi.cartSizeA = FileLoader(ROM, cartA, MAX_ROM_SIZE);

	if (Multi.cartSizeA == 0)
	{
		if (cartB && cartB[0])
			Multi.cartSizeB = FileLoader(ROM, cartB, MAX_ROM_SIZE);
	}

	if (Multi.cartSizeA)
	{
		if (is_SufamiTurbo_Cart(ROM, Multi.cartSizeA))
			Multi.cartType = 4;
		else
		if (is_SameGame_BIOS(ROM, Multi.cartSizeA))
			Multi.cartType = 3;
	}
	else
	if (Multi.cartSizeB)
	{
		if (is_SufamiTurbo_Cart(ROM, Multi.cartSizeB))
			Multi.cartType = 4;
	}
	else
		Multi.cartType = 4; // assuming BIOS only

	switch (Multi.cartType)
	{
		case 4:
			r = LoadSufamiTurbo(cartA, cartB);
			break;

		case 3:
			r = LoadSameGame(cartA, cartB);
			break;

		default:
			r = FALSE;
	}

	if (!r)
	{
		ZeroMemory(&Multi, sizeof(Multi));
		return (FALSE);
	}

	FreeSDD1Data();
	if (CleanUp7110)
		(*CleanUp7110)();

	ZeroMemory(&SNESGameFixes, sizeof(SNESGameFixes));
	SNESGameFixes.SRAMInitialValue = 0x60;

	S9xLoadCheatFile(S9xGetFilename(".cht", PATCH_DIR));

	InitROM();

	S9xInitCheatData();
	S9xApplyCheats();

	S9xReset();

	return (TRUE);
}

bool8 CMemory::LoadSufamiTurbo (const char *cartA, const char *cartB)
{
	Multi.cartOffsetA = 0x100000;
	Multi.cartOffsetB = 0x200000;
	Multi.sramA = SRAM;
	Multi.sramB = SRAM + 0x10000;

	if (Multi.cartSizeA)
	{
		Multi.sramSizeA = 4; // ROM[0x37]?
		Multi.sramMaskA = Multi.sramSizeA ? ((1 << (Multi.sramSizeA + 3)) * 128 - 1) : 0;

		if (!Settings.NoPatch)
			CheckForIPSPatch(cartA, HeaderCount != 0, Multi.cartSizeA);

		strcpy(Multi.fileNameA, cartA);
		memcpy(ROM + Multi.cartOffsetA, ROM, Multi.cartSizeA);
	}

	if (Multi.cartSizeA && !Multi.cartSizeB)
	{
		if (cartB && cartB[0])
			Multi.cartSizeB = FileLoader(ROM, cartB, MAX_ROM_SIZE);

		if (Multi.cartSizeB)
		{
			if (!is_SufamiTurbo_Cart(ROM, Multi.cartSizeB))
				Multi.cartSizeB = 0;
		}
	}

	if (Multi.cartSizeB)
	{
		Multi.sramSizeB = 4; // ROM[0x37]?
		Multi.sramMaskB = Multi.sramSizeB ? ((1 << (Multi.sramSizeB + 3)) * 128 - 1) : 0;

		if (!Settings.NoPatch)
			CheckForIPSPatch(cartB, HeaderCount != 0, Multi.cartSizeB);

		strcpy(Multi.fileNameB, cartB);
		memcpy(ROM + Multi.cartOffsetB, ROM, Multi.cartSizeB);
	}

	FILE	*fp;
	size_t	size;
	char	path[_MAX_PATH + 1];

	strcpy(path, S9xGetDirectory(BIOS_DIR));
	strcat(path, SLASH_STR);
	strcat(path, "STBIOS.bin");

	fp = fopen(path, "rb");
	if (fp)
	{
		size = fread((void *) ROM, 1, 0x40000, fp);
		fclose(fp);
		if (!is_SufamiTurbo_BIOS(ROM, size))
			return (FALSE);
	}
	else
		return (FALSE);

	if (Multi.cartSizeA)
		strcpy(ROMFilename, Multi.fileNameA);
	else
	if (Multi.cartSizeB)
		strcpy(ROMFilename, Multi.fileNameB);
	else
		strcpy(ROMFilename, path);

	LoROM = TRUE;
	HiROM = FALSE;
	CalculatedSize = 0x40000;

	return (TRUE);
}

bool8 CMemory::LoadSameGame (const char *cartA, const char *cartB)
{
	Multi.cartOffsetA = 0;
	Multi.cartOffsetB = 0x200000;
	Multi.sramA = SRAM;
	Multi.sramB = NULL;

	Multi.sramSizeA = ROM[0xffd8];
	Multi.sramMaskA = Multi.sramSizeA ? ((1 << (Multi.sramSizeA + 3)) * 128 - 1) : 0;
	Multi.sramSizeB = 0;
	Multi.sramMaskB = 0;

	if (!Settings.NoPatch)
		CheckForIPSPatch(cartA, HeaderCount != 0, Multi.cartSizeA);

	strcpy(Multi.fileNameA, cartA);

	if (cartB && cartB[0])
		Multi.cartSizeB = FileLoader(ROM + Multi.cartOffsetB, cartB, MAX_ROM_SIZE - Multi.cartOffsetB);

	if (Multi.cartSizeB)
	{
		if (!is_SameGame_Add_On(ROM + Multi.cartOffsetB, Multi.cartSizeB))
			Multi.cartSizeB = 0;
		else
			strcpy(Multi.fileNameB, cartB);
	}

	strcpy(ROMFilename, Multi.fileNameA);

	LoROM = FALSE;
	HiROM = TRUE;
	CalculatedSize = Multi.cartSizeA;

	return (TRUE);
}

bool8 CMemory::LoadLastROM (void)
{
	if (Multi.cartType)
		return (FALSE);

	bool8	r;

	stopMovie = FALSE;
	r = LoadROM(LastRomFilename);
	stopMovie = TRUE;

	return (r);
}

void CMemory::ClearSRAM (bool8 onlyNonSavedSRAM)
{
	if(onlyNonSavedSRAM)
		if (!(Settings.SuperFX && ROMType < 0x15) && !(Settings.SA1 && ROMType == 0x34)) // can have SRAM
			return;

	memset(SRAM, SNESGameFixes.SRAMInitialValue, 0x20000);
}

bool8 CMemory::LoadSRAM (const char *filename)
{
	FILE	*file;
	int		size, len;
	char	sramName[_MAX_PATH + 1];

	strcpy(sramName, filename);

	ClearSRAM();

	if (Multi.cartType && Multi.sramSizeB)
	{
		char	temp[_MAX_PATH + 1];

		strcpy(temp, ROMFilename);
		strcpy(ROMFilename, Multi.fileNameB);

		size = (1 << (Multi.sramSizeB + 3)) * 128;

		file = fopen(S9xGetFilename(".srm", SRAM_DIR), "rb");
		if (file)
		{
			len = fread((char *) Multi.sramB, 1, 0x10000, file);
			fclose(file);
			if (len - size == 512)
				memmove(Multi.sramB, Multi.sramB + 512, size);
		}

		strcpy(ROMFilename, temp);
	}

	size = SRAMSize ? (1 << (SRAMSize + 3)) * 128 : 0;
	if (size > 0x20000)
		size = 0x20000;

	if (size)
	{
		file = fopen(sramName, "rb");
		if (file)
		{
			len = fread((char *) SRAM, 1, 0x20000, file);
			fclose(file);
			if (len - size == 512)
				memmove(SRAM, SRAM + 512, size);

			if (len == size + SRTC_SRAM_PAD)
			{
				S9xSRTCPostLoadState();
				S9xResetSRTC();
				rtc.index = -1;
				rtc.mode = MODE_READ;
			}
			else
				S9xHardResetSRTC();

			if (Settings.SPC7110RTC)
				S9xLoadSPC7110RTC(&rtc_f9);

			return (TRUE);
		}
		else
		if (Settings.BS && !Settings.BSXItself)
		{
			// The BS game's SRAM was not found
			// Try to read BS-X.srm instead
			char	path[_MAX_PATH + 1];

			strcpy(path, S9xGetDirectory(SRAM_DIR));
			strcat(path, SLASH_STR);
			strcat(path, "BS-X.srm");

			file = fopen(path, "rb");
			if (file)
			{
				len = fread((char *) SRAM, 1, 0x20000, file);
				fclose(file);
				if (len - size == 512)
					memmove(SRAM, SRAM + 512, size);

				S9xMessage(S9X_INFO, S9X_ROM_INFO, "The SRAM file wasn't found: BS-X.srm was read instead.");
				S9xHardResetSRTC();
				return (TRUE);
			}
			else
			{
				S9xMessage(S9X_INFO, S9X_ROM_INFO, "The SRAM file wasn't found, BS-X.srm wasn't found either.");
				S9xHardResetSRTC();
				return (FALSE);
			}
		}

		S9xHardResetSRTC();
		return (FALSE);
	}

	if (Settings.SDD1)
		S9xSDD1LoadLoggedData();

	return (TRUE);
}

bool8 CMemory::SaveSRAM (const char *filename)
{
	if (Settings.SuperFX && ROMType < 0x15) // doesn't have SRAM
		return (TRUE);

	if (Settings.SA1 && ROMType == 0x34)    // doesn't have SRAM
		return (TRUE);

	FILE	*file;
	int		size;
	char	sramName[_MAX_PATH + 1];

	strcpy(sramName, filename);

	if (Multi.cartType && Multi.sramSizeB)
	{
		char	name[_MAX_PATH + 1], temp[_MAX_PATH + 1];

		strcpy(temp, ROMFilename);
		strcpy(ROMFilename, Multi.fileNameB);
		strcpy(name, S9xGetFilename(".srm", SRAM_DIR));

		size = (1 << (Multi.sramSizeB + 3)) * 128;

		file = fopen(name, "wb");
		if (file)
		{
			fwrite((char *) Multi.sramB, size, 1, file);
			fclose(file);
#ifdef __linux
			chown(name, getuid(), getgid());
#endif
		}

		strcpy(ROMFilename, temp);
    }

    size = SRAMSize ? (1 << (SRAMSize + 3)) * 128 : 0;

	if (Settings.SRTC)
	{
		size += SRTC_SRAM_PAD;
		S9xSRTCPreSaveState();
	}

	if (Settings.SDD1)
		S9xSDD1SaveLoggedData();

	if (size > 0x20000)
		size = 0x20000;

	if (size)
	{
		file = fopen(sramName, "wb");
		if (file)
		{
			fwrite((char *) SRAM, size, 1, file);
			fclose(file);
#ifdef __linux
			chown(sramName, getuid(), getgid());
#endif
			if (Settings.SPC7110RTC)
				S9xSaveSPC7110RTC(&rtc_f9);

			return (TRUE);
		}
	}

	return (FALSE);
}

// initialization

static uint32 caCRC32 (uint8 *array, uint32 size, uint32 crc32)
{
	for (uint32 i = 0; i < size; i++)
		crc32 = ((crc32 >> 8) & 0x00FFFFFF) ^ crc32Table[(crc32 ^ array[i]) & 0xFF];

	return (~crc32);
}

char * CMemory::Safe (const char *s)
{
	static char	*safe = NULL;
	static int	safe_len = 0;

	if (s == NULL)
	{
		if (safe)
		{
			free(safe);
			safe = NULL;
		}

		return (NULL);
	}

	int	len = strlen(s);
	if (!safe || len + 1 > safe_len)
	{
		if (safe)
			free(safe);

		safe_len = len + 1;
		safe = (char *) malloc(safe_len);
	}

	for (int i = 0; i < len; i++)
	{
		if (s[i] >= 32 && s[i] < 127)
			safe[i] = s[i];
		else
			safe[i] = '_';
	}

	safe[len] = 0;

	return (safe);
}

char * CMemory::SafeANK (const char *s)
{
	static char	*safe = NULL;
	static int	safe_len = 0;

	if (s == NULL)
	{
		if (safe)
		{
			free(safe);
			safe = NULL;
		}

		return (NULL);
	}

	int	len = strlen(s);
	if (!safe || len + 1 > safe_len)
	{
		if (safe)
			free(safe);

		safe_len = len + 1;
		safe = (char *) malloc(safe_len);
	}

	for (int i = 0; i < len; i++)
	{
		if (s[i] >= 32 && s[i] < 127) // ASCII
			safe [i] = s[i];
		else
		if (ROMRegion == 0 && ((uint8) s[i] >= 0xa0 && (uint8) s[i] < 0xe0)) // JIS X 201 - Katakana
			safe [i] = s[i];
		else
			safe [i] = '_';
	}

	safe [len] = 0;

	return (safe);
}

void CMemory::ParseSNESHeader (uint8 *RomHeader)
{
	bool8	bs = Settings.BS & !Settings.BSXItself;

	strncpy(ROMName, (char *) &RomHeader[0x10], ROM_NAME_LEN - 1);
	if (bs)
		memset(ROMName + 16, 0x20, ROM_NAME_LEN - 17);

	if (bs)
	{
		if (!(((RomHeader[0x29] & 0x20) && CalculatedSize <  0x100000) ||
			 (!(RomHeader[0x29] & 0x20) && CalculatedSize == 0x100000)))
			printf("BS: Size mismatch\n");

		// FIXME
		int	p = 0;
		while ((1 << p) < (int) CalculatedSize)
			p++;
		ROMSize = p - 10;
	}
	else
		ROMSize = RomHeader[0x27];

	SRAMSize  = bs ? 5 /* BS-X */    : RomHeader[0x28];
	ROMSpeed  = bs ? RomHeader[0x28] : RomHeader[0x25];
	ROMType   = bs ? 0xE5 /* BS-X */ : RomHeader[0x26];
	ROMRegion = bs ? 0               : RomHeader[0x29];

	ROMChecksum           = RomHeader[0x2E] + (RomHeader[0x2F] << 8);
	ROMComplementChecksum = RomHeader[0x2C] + (RomHeader[0x2D] << 8);

	memmove(ROMId, &RomHeader[0x02], 4);

	if (RomHeader[0x2A] == 0x33)
		memmove(CompanyId, &RomHeader[0x00], 2);
	else
		sprintf(CompanyId, "%02X", RomHeader[0x2A]);
}

void CMemory::InitROM (void)
{
	Settings.DSP1Master = FALSE;
	Settings.SuperFX = FALSE;
	Settings.SA1 = FALSE;
	Settings.C4 = FALSE;
	Settings.SDD1 = FALSE;
	Settings.SRTC = FALSE;
	Settings.SPC7110 = FALSE;
	Settings.SPC7110RTC = FALSE;
	Settings.BS = FALSE;
	Settings.OBC1 = FALSE;
	Settings.SETA = FALSE;
#ifndef ZSNES_FX
	SuperFX.nRomBanks = CalculatedSize >> 15;
#endif
	s7r.DataRomSize = 0;

	//// Parse ROM header and read ROM informatoin

	memset(ROMId, 0, 5);
	memset(CompanyId, 0, 3);

	uint8	*RomHeader = ROM + 0x7FB0;
	if (ExtendedFormat == BIGFIRST)
		RomHeader += 0x400000;
	if (HiROM)
		RomHeader += 0x8000;

	S9xInitBSX(); // Set BS header before parsing

	ParseSNESHeader(RomHeader);

	//// Detect and initialize chips
	//// detection codes are compatible with NSRT

	// DSP1/2/3/4
	Settings.DSP1Master = Settings.ForceDSP1;
	DSP1.version = 0xff;

	if (ROMType == 0x03)
	{
		if (ROMSpeed == 0x30)
			DSP1.version = 3; // DSP4
		else
			DSP1.version = 0; // DSP1
	}
	else
	if (ROMType == 0x05)
	{
		if (ROMSpeed == 0x20)
			DSP1.version = 1; // DSP2
		else
		if (ROMSpeed == 0x30 && RomHeader[0x2a] == 0xb2)
			DSP1.version = 2; // DSP3
		else
			DSP1.version = 0; // DSP1
	}

	if (DSP1.version != 0xff)
		Settings.DSP1Master = !Settings.ForceNoDSP1;

	switch (DSP1.version)
	{
		case 0:	// DSP1
			if (HiROM)
			{
				DSP1.boundary = 0x7000;
				DSP1.maptype = M_DSP1_HIROM;
			}
			else
			if (CalculatedSize > 0x100000)
			{
				DSP1.boundary = 0x4000;
				DSP1.maptype = M_DSP1_LOROM_L;
			}
			else
			{
				DSP1.boundary = 0xc000;
				DSP1.maptype = M_DSP1_LOROM_S;
			}

			SetDSP = &DSP1SetByte;
			GetDSP = &DSP1GetByte;
			break;

		case 1: // DSP2
			DSP1.boundary = 0x10000;
			DSP1.maptype = M_DSP2_LOROM;
			SetDSP = &DSP2SetByte;
			GetDSP = &DSP2GetByte;
			break;

		case 2: // DSP3
			DSP1.boundary = 0xc000;
			DSP1.maptype = M_DSP3_LOROM;
			SetDSP = &DSP3SetByte;
			GetDSP = &DSP3GetByte;
			DSP3_Reset();
			break;

		case 3: // DSP4
			DSP1.boundary = 0xc000;
			DSP1.maptype = M_DSP4_LOROM;
			SetDSP = &DSP4SetByte;
			GetDSP = &DSP4GetByte;
			break;

		default:
			SetDSP = NULL;
			GetDSP = NULL;
			break;
	}

	Settings.SA1     = Settings.ForceSA1;
	Settings.SuperFX = Settings.ForceSuperFX;
	Settings.SDD1    = Settings.ForceSDD1;
	Settings.C4      = Settings.ForceC4;

	uint32	identifier = ((ROMType & 0xff) << 8) + (ROMSpeed & 0xff);

	switch (identifier)
	{
	    // SRTC
		case 0x5535:
			Settings.SRTC = TRUE;
			break;

		// SPC7110
		case 0xF93A:
			Settings.SPC7110RTC = TRUE;
		case 0xF53A:
			Settings.SPC7110 = TRUE;
			S9xSpc7110Init();
			break;

		// OBC1
		case 0x2530:
			Settings.OBC1 = TRUE;
			break;

		// SA1
		case 0x3423:
		case 0x3523:
			Settings.SA1 = !Settings.ForceNoSA1;
			break;

		// SuperFX
		case 0x1320:
		case 0x1420:
		case 0x1520:
		case 0x1A20:
			Settings.SuperFX = !Settings.ForceNoSuperFX;
			if (Settings.SuperFX)
			{
				if (ROM[0x7FDA] == 0x33)
					SRAMSize = ROM[0x7FBD];
				else
					SRAMSize = 5;
			}

			break;

		// SDD1
		case 0x4332:
		case 0x4532:
			Settings.SDD1 = !Settings.ForceNoSDD1;
			if (Settings.SDD1)
				S9xLoadSDD1Data();
			break;

		// ST018
		case 0xF530:
			Settings.SETA = ST_018;
			SetSETA = NULL;
			GetSETA = NULL;
			SRAMSize = 2;
			SNESGameFixes.SRAMInitialValue = 0x00;
			break;

		// ST010/011
		case 0xF630:
			if (ROM[0x7FD7] == 0x09)
			{
				Settings.SETA = ST_011;
				SetSETA = &S9xSetST011;
				GetSETA = &S9xGetST011;
			}
			else
			{
				Settings.SETA = ST_010;
				SetSETA = &S9xSetST010;
				GetSETA = &S9xGetST010;
			}

			SRAMSize = 2;
			SNESGameFixes.SRAMInitialValue = 0x00;
			break;

		// C4
		case 0xF320:
			Settings.C4 = !Settings.ForceNoC4;
			break;
	}

	//// Map memory and calculate checksum

	Map_Initialize();
	CalculatedChecksum = 0;

	if (HiROM)
    {
		if (Settings.BS)
			/* Do nothing */;
		else
		if (Settings.SPC7110)
			Map_SPC7110HiROMMap();
		else
		if (ExtendedFormat != NOPE)
			Map_ExtendedHiROMMap();
		else
		if (Multi.cartType == 3)
			Map_SameGameHiROMMap();
		else
			Map_HiROMMap();
    }
    else
    {
		if (Settings.BS)
			/* Do nothing */;
		else
		if (Settings.SETA && Settings.SETA != ST_018)
			Map_SetaDSPLoROMMap();
		else
		if (Settings.SuperFX)
			Map_SuperFXLoROMMap();
		else
		if (Settings.SA1)
			Map_SA1LoROMMap();
		else
		if (Settings.SDD1)
			Map_SDD1LoROMMap();
		else
		if (ExtendedFormat != NOPE)
			Map_JumboLoROMMap();
		else
		if (strncmp(ROMName, "WANDERERS FROM YS", 17) == 0)
			Map_NoMAD1LoROMMap();
		else
		if (strncmp(ROMName, "SOUND NOVEL-TCOOL", 17) == 0 ||
			strncmp(ROMName, "DERBY STALLION 96", 17) == 0)
			Map_ROM24MBSLoROMMap();
		else
		if (strncmp(ROMName, "THOROUGHBRED BREEDER3", 21) == 0 ||
			strncmp(ROMName, "RPG-TCOOL 2", 11) == 0)
			Map_SRAM512KLoROMMap();
		else
		if (strncmp(ROMName, "ADD-ON BASE CASSETE", 19) == 0)
		{
			if (Multi.cartType == 4)
			{
				SRAMSize = Multi.sramSizeA;
				Map_SufamiTurboLoROMMap();
			}
			else
			{
				SRAMSize = 5;
				Map_SufamiTurboPseudoLoROMMap();
			}
		}
		else
			Map_LoROMMap();
    }

	Checksum_Calculate();

	bool8 isChecksumOK = (ROMChecksum + ROMComplementChecksum == 0xffff) &
						 (ROMChecksum == CalculatedChecksum);

	//// Build more ROM information

	// CRC32
	if (!Settings.BS || Settings.BSXItself) // Not BS Dump
		ROMCRC32 = caCRC32(ROM, CalculatedSize);
	else // Convert to correct format before scan
	{
		int offset = HiROM ? 0xffc0 : 0x7fc0;
		// Backup
		uint8 BSMagic0 = ROM[offset + 22],
			  BSMagic1 = ROM[offset + 23];
		// uCONSRT standard
		ROM[offset + 22] = 0x42;
		ROM[offset + 23] = 0x00;
		// Calc
		ROMCRC32 = caCRC32(ROM, CalculatedSize);
		// Convert back
		ROM[offset + 22] = BSMagic0;
		ROM[offset + 23] = BSMagic1;
	}

	// NTSC/PAL
	if (Settings.ForceNTSC)
		Settings.PAL = FALSE;
	else
	if (Settings.ForcePAL)
		Settings.PAL = TRUE;
	else
	if (!Settings.BS && (ROMRegion >= 2) && (ROMRegion <= 12))
		Settings.PAL = TRUE;
	else
		Settings.PAL = FALSE;

	if (Settings.PAL)
	{
		Settings.FrameTime = Settings.FrameTimePAL;
		ROMFramesPerSecond = 50;
	}
	else
	{
		Settings.FrameTime = Settings.FrameTimeNTSC;
		ROMFramesPerSecond = 60;
	}

	// truncate cart name
	ROMName[ROM_NAME_LEN - 1] = 0;
	if (strlen(ROMName))
	{
		char *p = ROMName + strlen(ROMName);
		if (p > ROMName + 21 && ROMName[20] == ' ')
			p = ROMName + 21;
		while (p > ROMName && *(p - 1) == ' ')
			p--;
		*p = 0;
	}

	// SRAM size
	SRAMMask = SRAMSize ? ((1 << (SRAMSize + 3)) * 128) - 1 : 0;

	// checksum
	if (!isChecksumOK || ((uint32) CalculatedSize > (uint32) (((1 << (ROMSize - 7)) * 128) * 1024)))
	{
		if (Settings.DisplayColor == 0xffff || Settings.DisplayColor != BUILD_PIXEL(31, 0, 0))
		{
			Settings.DisplayColor = BUILD_PIXEL(31, 31, 0);
			SET_UI_COLOR(255, 255, 0);
		}
	}

	if (Multi.cartType == 4)
	{
		Settings.DisplayColor = BUILD_PIXEL(0, 16, 31);
		SET_UI_COLOR(0, 128, 255);
	}

	//// Initialize emulation

	Timings.H_Max_Master = SNES_CYCLES_PER_SCANLINE;
	Timings.H_Max        = Timings.H_Max_Master;
	Timings.HBlankStart  = SNES_HBLANK_START_HC;
	Timings.HBlankEnd    = SNES_HBLANK_END_HC;
	Timings.HDMAInit     = SNES_HDMA_INIT_HC;
	Timings.HDMAStart    = SNES_HDMA_START_HC;
	Timings.RenderPos    = SNES_RENDER_START_HC;
	Timings.V_Max_Master = Settings.PAL ? SNES_MAX_PAL_VCOUNTER : SNES_MAX_NTSC_VCOUNTER;
	Timings.V_Max        = Timings.V_Max_Master;
	/* From byuu: The total delay time for both the initial (H)DMA sync (to the DMA clock),
	   and the end (H)DMA sync (back to the last CPU cycle's mcycle rate (6, 8, or 12)) always takes between 12-24 mcycles.
	   Possible delays: { 12, 14, 16, 18, 20, 22, 24 }
	   XXX: Snes9x can't emulate this timing :( so let's use the average value... */
	Timings.DMACPUSync   = 18;

	IAPU.OneCycle = SNES_APU_ONE_CYCLE_SCALED;

	CPU.FastROMSpeed = 0;
	ResetSpeedMap();

	IPPU.TotalEmulatedFrames = 0;

	Settings.Shutdown = Settings.ShutdownMaster;

	//// Hack games

	ApplyROMFixes();

	//// Show ROM information
	char displayName[ROM_NAME_LEN];

	strcpy(RawROMName, ROMName);
	sprintf(displayName, "%s", SafeANK(ROMName));
	sprintf(ROMName, "%s", Safe(ROMName));
	sprintf(ROMId, "%s", Safe(ROMId));
	sprintf(CompanyId, "%s", Safe(CompanyId));

	sprintf(String, "\"%s\" [%s] %s, %s, Type: %s, Mode: %s, TV: %s, S-RAM: %s, ROMId: %s Company: %2.2s CRC32: %08X",
		displayName, isChecksumOK ? "checksum ok" : ((Multi.cartType == 4) ? "no checksum" : "bad checksum"),
		MapType(), Size(), KartContents(), MapMode(), TVStandard(), StaticRAMSize(), ROMId, CompanyId, ROMCRC32);
	S9xMessage(S9X_INFO, S9X_ROM_INFO, String);

	// XXX: Please remove port specific codes
#ifdef __WIN32__
#ifndef _XBOX
	EnableMenuItem(GUI.hMenu, IDM_ROM_INFO, MF_ENABLED);
#endif
#ifdef RTC_DEBUGGER
	if (Settings.SPC7110RTC)
		EnableMenuItem(GUI.hMenu, IDM_7110_RTC, MF_ENABLED);
	else
		EnableMenuItem(GUI.hMenu, IDM_7110_RTC, MF_GRAYED);
#endif
#endif

	Settings.ForceHiROM = Settings.ForceLoROM = FALSE;
	Settings.ForceHeader = Settings.ForceNoHeader = FALSE;
	Settings.ForceInterleaved = Settings.ForceNotInterleaved = Settings.ForceInterleaved2 = FALSE;

	if (stopMovie)
		S9xMovieStop(TRUE);

	if (PostRomInitFunc)
		PostRomInitFunc();

    S9xVerifyControllers();
}

void CMemory::FixROMSpeed (void)
{
	if (CPU.FastROMSpeed == 0)
		CPU.FastROMSpeed = SLOW_ONE_CYCLE;

	// [80-bf]:[8000-ffff], [c0-ff]:[0000-ffff]
	for (int c = 0x800; c < 0x1000; c++)
	{
		if (c & 0x8 || c & 0x400)
			MemorySpeed[c] = (uint8) CPU.FastROMSpeed;
	}
}

void CMemory::ResetSpeedMap (void)
{
	memset(MemorySpeed, SLOW_ONE_CYCLE, 0x1000);

	// Fast  - [00-3f|80-bf]:[2000-3fff|4200-5fff]
	// XSlow - [00-3f|80-bf]:[4000-41ff] see also S9xGet/SetCPU()
	for (int i = 0; i < 0x400; i += 0x10)
	{
		MemorySpeed[i + 2] = MemorySpeed[0x800 + i + 2] = ONE_CYCLE;
		MemorySpeed[i + 3] = MemorySpeed[0x800 + i + 3] = ONE_CYCLE;
		MemorySpeed[i + 4] = MemorySpeed[0x800 + i + 4] = ONE_CYCLE;
		MemorySpeed[i + 5] = MemorySpeed[0x800 + i + 5] = ONE_CYCLE;
	}

	FixROMSpeed();
}

// memory map

uint32 CMemory::map_mirror (uint32 size, uint32 pos)
{
	// from bsnes
	if (size == 0)
		return (0);
	if (pos < size)
		return (pos);

	uint32	mask = 1 << 31;
	while (!(pos & mask))
		mask >>= 1;

	if (size <= (pos & mask))
		return (map_mirror(size, pos - mask));
	else
		return (mask + map_mirror(size - mask, pos - mask));
}

void CMemory::map_lorom (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = (c & 0x7f) * 0x8000;
			Map[p] = ROM + map_mirror(size, addr) - (i & 0x8000);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_hirom (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = c << 16;
			Map[p] = ROM + map_mirror(size, addr);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_lorom_offset (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size, uint32 offset)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = ((c - bank_s) & 0x7f) * 0x8000;
			Map[p] = ROM + offset + map_mirror(size, addr) - (i & 0x8000);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_hirom_offset (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size, uint32 offset)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = (c - bank_s) << 16;
			Map[p] = ROM + offset + map_mirror(size, addr);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_space (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint8 *data)
{
	uint32	c, i, p;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			Map[p] = data;
			BlockIsROM[p] = FALSE;
			BlockIsRAM[p] = TRUE;
		}
	}
}

void CMemory::map_index (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, int index, int type)
{
	uint32	c, i, p;
	bool8	isROM, isRAM;

	isROM = ((type == MAP_TYPE_I_O) || (type == MAP_TYPE_RAM)) ? FALSE : TRUE;
	isRAM = ((type == MAP_TYPE_I_O) || (type == MAP_TYPE_ROM)) ? FALSE : TRUE;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			Map[p] = (uint8 *) index;
			BlockIsROM[p] = isROM;
			BlockIsRAM[p] = isRAM;
		}
	}
}

void CMemory::map_System (void)
{
	// will be overwritten
	map_space(0x00, 0x3f, 0x0000, 0x1fff, RAM);
	map_index(0x00, 0x3f, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	map_index(0x00, 0x3f, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
	map_space(0x80, 0xbf, 0x0000, 0x1fff, RAM);
	map_index(0x80, 0xbf, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
}

void CMemory::map_WRAM (void)
{
	// will overwrite others
	map_space(0x7e, 0x7e, 0x0000, 0xffff, RAM);
	map_space(0x7f, 0x7f, 0x0000, 0xffff, RAM + 0x10000);
}

void CMemory::map_LoROMSRAM (void)
{
	map_index(0x70, 0x7f, 0x0000, 0x7fff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	map_index(0xf0, 0xff, 0x0000, 0x7fff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
}

void CMemory::map_HiROMSRAM (void)
{
	map_index(0x20, 0x3f, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
	map_index(0xa0, 0xbf, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
}

void CMemory::map_DSP (void)
{
	switch (DSP1.maptype)
	{
		case M_DSP1_LOROM_S:
			map_index(0x20, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP1_LOROM_L:
			map_index(0x60, 0x6f, 0x0000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xe0, 0xef, 0x0000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP1_HIROM:
			map_index(0x00, 0x1f, 0x6000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0x80, 0x9f, 0x6000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP2_LOROM:
			map_index(0x20, 0x3f, 0x6000, 0x6fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0x20, 0x3f, 0x8000, 0xbfff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x6000, 0x6fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x8000, 0xbfff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP3_LOROM:
			map_index(0x20, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP4_LOROM:
			map_index(0x30, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xb0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			break;
	}
}

void CMemory::map_C4 (void)
{
	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_C4, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x6000, 0x7fff, MAP_C4, MAP_TYPE_I_O);
}

void CMemory::map_OBC1 (void)
{
	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_OBC_RAM, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x6000, 0x7fff, MAP_OBC_RAM, MAP_TYPE_I_O);
}

void CMemory::map_SetaRISC (void)
{
	map_index(0x00, 0x3f, 0x3000, 0x3fff, MAP_SETA_RISC, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x3000, 0x3fff, MAP_SETA_RISC, MAP_TYPE_I_O);
}

void CMemory::map_SetaDSP (void)
{
	// where does the SETA chip access, anyway?
	// please confirm this?
	map_index(0x68, 0x6f, 0x0000, 0x7fff, MAP_SETA_DSP, MAP_TYPE_RAM);
	// and this!
	map_index(0x60, 0x67, 0x0000, 0x3fff, MAP_SETA_DSP, MAP_TYPE_I_O);

	// ST-0010:
	// map_index(0x68, 0x6f, 0x0000, 0x0fff, MAP_SETA_DSP, ?);
}

void CMemory::map_WriteProtectROM (void)
{
	memmove((void *) WriteMap, (void *) Map, sizeof(Map));

	for (int c = 0; c < 0x1000; c++)
	{
		if (BlockIsROM[c])
			WriteMap[c] = (uint8 *) MAP_NONE;
	}
}

void CMemory::Map_Initialize (void)
{
	for (int c = 0; c < 0x1000; c++)
	{
		Map[c]      = (uint8 *) MAP_NONE;
		WriteMap[c] = (uint8 *) MAP_NONE;
		BlockIsROM[c] = FALSE;
		BlockIsRAM[c] = FALSE;
	}
}

void CMemory::Map_LoROMMap (void)
{
	printf("Map_LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	if (Settings.DSP1Master)
		map_DSP();
	else
	if (Settings.C4)
		map_C4();
	else
	if (Settings.OBC1)
		map_OBC1();
	else
	if (Settings.SETA == ST_018)
		map_SetaRISC();

    map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_NoMAD1LoROMMap (void)
{
	printf("Map_NoMAD1LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	map_index(0x70, 0x7f, 0x0000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	map_index(0xf0, 0xff, 0x0000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_JumboLoROMMap (void)
{
	// XXX: Which game uses this?
	printf("Map_JumboLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize - 0x400000, 0x400000);
	map_lorom_offset(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize - 0x400000, 0x400000);
	map_lorom_offset(0x80, 0xbf, 0x8000, 0xffff, 0x400000, 0);
	map_lorom_offset(0xc0, 0xff, 0x0000, 0xffff, 0x400000, 0x200000);

	map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_ROM24MBSLoROMMap (void)
{
	// PCB: BSC-1A5M-01, BSC-1A7M-10
	printf("Map_ROM24MBSLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x1f, 0x8000, 0xffff, 0x100000, 0);
	map_lorom_offset(0x20, 0x3f, 0x8000, 0xffff, 0x100000, 0x100000);
	map_lorom_offset(0x80, 0x9f, 0x8000, 0xffff, 0x100000, 0x200000);
	map_lorom_offset(0xa0, 0xbf, 0x8000, 0xffff, 0x100000, 0x100000);

	map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SRAM512KLoROMMap (void)
{
	printf("Map_SRAM512KLoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	map_space(0x70, 0x70, 0x0000, 0xffff, SRAM);
	map_space(0x71, 0x71, 0x0000, 0xffff, SRAM + 0x8000);
	map_space(0x72, 0x72, 0x0000, 0xffff, SRAM + 0x10000);
	map_space(0x73, 0x73, 0x0000, 0xffff, SRAM + 0x18000);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SufamiTurboLoROMMap (void)
{
	printf("Map_SufamiTurboLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x1f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0x20, 0x3f, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_lorom_offset(0x40, 0x5f, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_lorom_offset(0x80, 0x9f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0xa0, 0xbf, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_lorom_offset(0xc0, 0xdf, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);

	if (Multi.sramSizeA)
	{
		map_index(0x60, 0x63, 0x8000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
		map_index(0xe0, 0xe3, 0x8000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	}

	if (Multi.sramSizeB)
	{
		map_index(0x70, 0x73, 0x8000, 0xffff, MAP_LOROM_SRAM_B, MAP_TYPE_RAM);
		map_index(0xf0, 0xf3, 0x8000, 0xffff, MAP_LOROM_SRAM_B, MAP_TYPE_RAM);
	}

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SufamiTurboPseudoLoROMMap (void)
{
	// for combined images
	printf("Map_SufamiTurboPseudoLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x1f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0x20, 0x3f, 0x8000, 0xffff, 0x100000, 0x100000);
	map_lorom_offset(0x40, 0x5f, 0x8000, 0xffff, 0x100000, 0x200000);
	map_lorom_offset(0x80, 0x9f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0xa0, 0xbf, 0x8000, 0xffff, 0x100000, 0x100000);
	map_lorom_offset(0xc0, 0xdf, 0x8000, 0xffff, 0x100000, 0x200000);

	// I don't care :P
	map_space(0x60, 0x63, 0x8000, 0xffff, SRAM - 0x8000);
	map_space(0xe0, 0xe3, 0x8000, 0xffff, SRAM - 0x8000);
	map_space(0x70, 0x73, 0x8000, 0xffff, SRAM + 0x4000 - 0x8000);
	map_space(0xf0, 0xf3, 0x8000, 0xffff, SRAM + 0x4000 - 0x8000);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SuperFXLoROMMap (void)
{
	printf("Map_SuperFXLoROMMap\n");
	map_System();

	// Replicate the first 2Mb of the ROM at ROM + 2MB such that each 32K
	// block is repeated twice in each 64K block.
	for (int c = 0; c < 64; c++)
	{
		memmove(&ROM[0x200000 + c * 0x10000], &ROM[c * 0x8000], 0x8000);
		memmove(&ROM[0x208000 + c * 0x10000], &ROM[c * 0x8000], 0x8000);
	}

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);

	map_hirom_offset(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize, 0);
	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0);

	map_space(0x00, 0x3f, 0x6000, 0x7fff, SRAM - 0x6000);
	map_space(0x80, 0xbf, 0x6000, 0x7fff, SRAM - 0x6000);
	map_space(0x70, 0x70, 0x0000, 0xffff, SRAM);
	map_space(0x71, 0x71, 0x0000, 0xffff, SRAM + 0x10000);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SetaDSPLoROMMap (void)
{
	printf("Map_SetaDSPLoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x8000, 0xffff, CalculatedSize);

	map_SetaDSP();

    map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SDD1LoROMMap (void)
{
	printf("Map_SDD1LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);

	map_hirom_offset(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize, 0);
	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0); // will be overwritten dynamically

	map_index(0x70, 0x7f, 0x0000, 0x7fff, MAP_LOROM_SRAM, MAP_TYPE_RAM);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SA1LoROMMap (void)
{
	printf("Map_SA1LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);

	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0);

	map_space(0x00, 0x3f, 0x3000, 0x3fff, FillRAM);
	map_space(0x80, 0xbf, 0x3000, 0x3fff, FillRAM);
	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_BWRAM, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x6000, 0x7fff, MAP_BWRAM, MAP_TYPE_I_O);

	for (int c = 0x40; c < 0x80; c++)
		map_space(c, c, 0x0000, 0xffff, SRAM + (c & 1) * 0x10000);

	map_WRAM();

	map_WriteProtectROM();

	// Now copy the map and correct it for the SA1 CPU.
	memmove((void *) SA1.Map, (void *) Map, sizeof(Map));
	memmove((void *) SA1.WriteMap, (void *) WriteMap, sizeof(WriteMap));

	// SA-1 Banks 00->3f and 80->bf
	for (int c = 0x000; c < 0x400; c += 0x10)
	{
		SA1.Map[c + 0] = SA1.Map[c + 0x800] = FillRAM + 0x3000;
		SA1.Map[c + 1] = SA1.Map[c + 0x801] = (uint8 *) MAP_NONE;
		SA1.WriteMap[c + 0] = SA1.WriteMap[c + 0x800] = FillRAM + 0x3000;
		SA1.WriteMap[c + 1] = SA1.WriteMap[c + 0x801] = (uint8 *) MAP_NONE;
	}

	// SA-1 Banks 60->6f
	for (int c = 0x600; c < 0x700; c++)
		SA1.Map[c] = SA1.WriteMap[c] = (uint8 *) MAP_BWRAM_BITMAP;

	BWRAM = SRAM;
}

void CMemory::Map_HiROMMap (void)
{
	printf("Map_HiROMMap\n");
	map_System();

	map_hirom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_hirom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_hirom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_hirom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	if (Settings.DSP1Master)
		map_DSP();

	map_HiROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_ExtendedHiROMMap (void)
{
	printf("Map_ExtendedHiROMMap\n");
	map_System();

	map_hirom_offset(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize - 0x400000, 0x400000);
	map_hirom_offset(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize - 0x400000, 0x400000);
	map_hirom_offset(0x80, 0xbf, 0x8000, 0xffff, 0x400000, 0);
	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, 0x400000, 0);

	map_HiROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SameGameHiROMMap (void)
{
	printf("Map_SameGameHiROMMap\n");
	map_System();

	map_hirom_offset(0x00, 0x1f, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_hirom_offset(0x20, 0x3f, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_hirom_offset(0x40, 0x5f, 0x0000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_hirom_offset(0x60, 0x7f, 0x0000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_hirom_offset(0x80, 0x9f, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_hirom_offset(0xa0, 0xbf, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_hirom_offset(0xc0, 0xdf, 0x0000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_hirom_offset(0xe0, 0xff, 0x0000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);

	map_HiROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SPC7110HiROMMap (void)
{
	printf("Map_SPC7110HiROMMap\n");
	map_System();

	map_hirom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_hirom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_hirom_offset(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize, 0);
	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0);

	map_index(0x50, 0x50, 0x0000, 0xffff, MAP_SPC7110_DRAM, MAP_TYPE_ROM);
	map_index(0xd0, 0xff, 0x0000, 0xffff, MAP_SPC7110_ROM,  MAP_TYPE_ROM);

	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);

    map_WRAM();

	map_WriteProtectROM();
}

// checksum

uint16 CMemory::checksum_calc_sum (uint8 *data, uint32 length)
{
	uint16	sum = 0;

	for (uint32 i = 0; i < length; i++)
		sum += data[i];

	return (sum);
}

uint16 CMemory::checksum_mirror_sum (uint8 *start, uint32 &length, uint32 mask)
{
	// from NSRT
	while (!(length & mask))
		mask >>= 1;

	uint16	part1 = checksum_calc_sum(start, mask);
	uint16	part2 = 0;

	uint32	next_length = length - mask;
	if (next_length)
	{
		part2 = checksum_mirror_sum(start + mask, next_length, mask >> 1);

		while (next_length < mask)
		{
			next_length += next_length;
			part2 += part2;
		}

		length = mask + mask;
	}

	return (part1 + part2);
}

void CMemory::Checksum_Calculate (void)
{
	// from NSRT
	uint16	sum = 0;

	if (Settings.BS && !Settings.BSXItself)
		sum = checksum_calc_sum(ROM, CalculatedSize) - checksum_calc_sum(ROM + (HiROM ? 0xffb0 : 0x7fb0), 48);
	else
	if (Settings.SPC7110)
	{
		sum = checksum_calc_sum(ROM, CalculatedSize);
		if (CalculatedSize == 0x300000)
			sum += sum;
	}
	else
	{
		if (CalculatedSize & 0x7fff)
			sum = checksum_calc_sum(ROM, CalculatedSize);
		else
		{
			uint32	length = CalculatedSize;
			sum = checksum_mirror_sum(ROM, length);
		}
	}

	CalculatedChecksum = sum;
}

// information

const char * CMemory::TVStandard (void)
{
	return (Settings.PAL ? "PAL" : "NTSC");
}

const char * CMemory::MapType (void)
{
	return (HiROM ? ((ExtendedFormat != NOPE) ? "ExHiROM": "HiROM") : "LoROM");
}

const char * CMemory::MapMode (void)
{
	static char	str[4];

	sprintf(str, "%02x", ROMSpeed & ~0x10);

	return (str);
}

const char * CMemory::StaticRAMSize (void)
{
	static char	str[20];

	if (SRAMSize > 16)
		strcpy(str, "Corrupt");
	else
		sprintf(str, "%dKB", (SRAMMask + 1) / 1024);

	return (str);
}

const char * CMemory::Size (void)
{
	static char	str[20];

	if (Multi.cartType == 4)
		strcpy(str, "N/A");
	else
	if (ROMSize < 7 || ROMSize - 7 > 23)
		strcpy(str, "Corrupt");
	else
		sprintf(str, "%dMbits", 1 << (ROMSize - 7));

	return (str);
}

const char * CMemory::Revision (void)
{
	static char	str[20];

	sprintf(str, "1.%d", HiROM ? ((ExtendedFormat != NOPE) ? ROM[0x40ffdb] : ROM[0xffdb]) : ROM[0x7fdb]);

	return (str);
}

const char * CMemory::KartContents (void)
{
	static char	str[30];
	static char	*contents[3] = { "ROM", "ROM+RAM", "ROM+RAM+BAT" };

	char	chip[16];

	if (ROMType == 0 && !Settings.BS)
		return ("ROM");

	if (Settings.BS)
		strcpy(chip, "+BSX");
	else
	if (Settings.SuperFX)
		strcpy(chip, "+SuperFX");
	else
	if (Settings.SDD1)
		strcpy(chip, "+S-DD1");
	else
	if (Settings.OBC1)
		strcpy(chip, "+OBC1");
	else
	if (Settings.SA1)
		strcpy(chip, "+SA-1");
	else
	if (Settings.SPC7110RTC)
		strcpy(chip, "+SPC7110+RTC");
	else
	if (Settings.SPC7110)
		strcpy(chip, "+SPC7110");
	else
	if (Settings.SRTC)
		strcpy(chip, "+S-RTC");
	else
	if (Settings.C4)
		strcpy(chip, "+C4");
	else
	if (Settings.SETA == ST_010)
		strcpy(chip, "+ST-010");
	else
	if (Settings.SETA == ST_011)
		strcpy(chip, "+ST-011");
	else
	if (Settings.SETA == ST_018)
		strcpy(chip, "+ST-018");
	else
	if (Settings.DSP1Master)
		sprintf(chip, "+DSP%d", DSP1.version + 1);
	else
		strcpy(chip, "");

	sprintf(str, "%s%s", contents[(ROMType & 0xf) % 3], chip);

	return (str);
}

// hack

bool8 CMemory::match_na (const char *str)
{
	return (strcmp(ROMName, str) == 0);
}

bool8 CMemory::match_nn (const char *str)
{
	return (strncmp(ROMName, str, strlen(str)) == 0);
}

bool8 CMemory::match_nc (const char *str)
{
	return (strncasecmp(ROMName, str, strlen(str)) == 0);
}

bool8 CMemory::match_id (const char *str)
{
	return (strncmp(ROMId, str, strlen(str)) == 0);
}

void CMemory::ApplyROMFixes (void)
{
#ifdef __W32_HEAP
	if (_HEAPOK != _heapchk())
		MessageBox(GUI.hWnd, "CMemory::ApplyROMFixes", "Heap Corrupt", MB_OK);
#endif

	//// Warnings

	// Don't steal my work! -MK
	if ((ROMCRC32 == 0x1B4A5616) && match_nn("RUDORA NO HIHOU"))
	{
		strncpy(ROMName, "THIS SCRIPT WAS STOLEN", 22);
		Settings.DisplayColor = BUILD_PIXEL(31, 0, 0);
		SET_UI_COLOR(255, 0, 0);
	}

	// Reject strange hacked games
	if ((ROMCRC32 == 0x6810aa95) ||
		(ROMCRC32 == 0x340f23e5) ||
		(ROMCRC32 == 0x77fd806a) ||
		(match_nn("HIGHWAY BATTLE 2")) ||
		(match_na("FX SKIING NINTENDO 96") && (ROM[0x7fda] == 0)) ||
		(match_nn("HONKAKUHA IGO GOSEI")   && (ROM[0xffd5] != 0x31)))
	{
		Settings.DisplayColor = BUILD_PIXEL(31, 0, 0);
		SET_UI_COLOR(255, 0, 0);
	}

	//// APU timing hacks :(

	// This game cannot work well anyway
	if (match_id("AVCJ"))                                      // Rendering Ranger R2
	{
		IAPU.OneCycle = (int32) (15.7 * (1 << SNES_APU_ACCURACY));
		printf("APU OneCycle hack: %d\n", IAPU.OneCycle);
	}

	// XXX: All Quintet games?
	if (match_na("GAIA GENSOUKI 1 JPN")                     || // Gaia Gensouki
		match_id("JG  ")                                    || // Illusion of Gaia
		match_id("CQ  "))                                      // Stunt Race FX
	{
		IAPU.OneCycle = (int32) (13.0 * (1 << SNES_APU_ACCURACY));
		printf("APU OneCycle hack: %d\n", IAPU.OneCycle);
	}

	if (match_na("SOULBLADER - 1")                          || // Soul Blader
		match_na("SOULBLAZER - 1 USA")                      || // Soul Blazer
		match_na("SLAP STICK 1 JPN")                        || // Slap Stick
		match_id("E9 ")                                     || // Robotrek
		match_nn("ACTRAISER")                               || // Actraiser
		match_nn("ActRaiser-2")                             || // Actraiser 2
		match_id("AQT")                                     || // Tenchi Souzou, Terranigma
		match_id("ATV")                                     || // Tales of Phantasia
		match_id("ARF")                                     || // Star Ocean
		match_id("APR")                                     || // Zen-Nippon Pro Wrestling 2 - 3-4 Budoukan
		match_id("A4B")                                     || // Super Bomberman 4
		match_id("Y7 ")                                     || // U.F.O. Kamen Yakisoban - Present Ban
		match_id("Y9 ")                                     || // U.F.O. Kamen Yakisoban - Shihan Ban
		match_id("APB")                                     || // Super Bomberman - Panic Bomber W
		match_na("DARK KINGDOM")                            || // Dark Kingdom
		match_na("ZAN3 SFC")                                || // Zan III Spirits
		match_na("HIOUDEN")                                 || // Hiouden - Mamono-tachi Tono Chikai
		match_na("\xC3\xDD\xBC\xC9\xB3\xC0")                || // Tenshi no Uta
		match_na("FORTUNE QUEST")                           || // Fortune Quest - Dice wo Korogase
		match_na("FISHING TO BASSING")                      || // Shimono Masaki no Fishing To Bassing
		match_na("OHMONO BLACKBASS")                        || // Oomono Black Bass Fishing - Jinzouko Hen
		match_na("MASTERS")                                 || // Harukanaru Augusta 2 - Masters
		match_na("SFC \xB6\xD2\xDD\xD7\xB2\xC0\xDE\xB0")    || // Kamen Rider
		match_na("ZENKI TENCHIMEIDOU")					    || // Kishin Douji Zenki - Tenchi Meidou
		match_nn("TokyoDome '95Battle 7")                   || // Shin Nippon Pro Wrestling Kounin '95 - Tokyo Dome Battle 7
		match_nn("SWORD WORLD SFC")                         || // Sword World SFC/2
		match_nn("LETs PACHINKO(")                          || // BS Lets Pachinko Nante Gindama 1/2/3/4
		match_nn("THE FISHING MASTER")                      || // Mark Davis The Fishing Master
		match_nn("Parlor")                                  || // Parlor mini/2/3/4/5/6/7, Parlor Parlor!/2/3/4/5
		match_na("HEIWA Parlor!Mini8")                      || // Parlor mini 8
		match_nn("SANKYO Fever! \xCC\xA8\xB0\xCA\xDE\xB0!"))   // SANKYO Fever! Fever!
	{
		IAPU.OneCycle = (int32) (15.0 * (1 << SNES_APU_ACCURACY));
		printf("APU OneCycle hack: %d\n", IAPU.OneCycle);
	}

	//// DMA/HDMA timing hacks :(

	Timings.HDMAStart   = SNES_HDMA_START_HC + Settings.HDMATimingHack - 100;
	Timings.HBlankStart = SNES_HBLANK_START_HC + Timings.HDMAStart - SNES_HDMA_START_HC;

	// The HC counter (CPU.Cycles for snes9x) passes over the WRAM refresh point (HC~536)
	// while preparing to jump to the IRQ vector address.
	// That is to say, the WRAM refresh point is passed over in S9xOpcode_IRQ().
	// Then, HDMA starts just after $210e is half updated, and it causes the flicker of the ground.
	// IRQ timing is bad? HDMA timing is bad? else?
	if (match_na("GUNDAMW ENDLESSDUEL")) // Shin Kidou Senki Gundam W - Endless Duel
	{
		Timings.HDMAStart   -= 10;
		Timings.HBlankStart -= 10;
		printf("HDMA timing hack: %d\n", Timings.HDMAStart);
	}

	// The delay to sync CPU and DMA which Snes9x cannot emulate.
	// Some games need really severe delay timing...
	if (match_na("BATTLE GRANDPRIX")) // Battle Grandprix
	{
		Timings.DMACPUSync = 20;
		printf("DMA sync: %d\n", Timings.DMACPUSync);
	}

	//// CPU speed-ups (CPU_Shutdown())

	// Force disabling a speed-up hack
    // Games which spool sound samples between the SNES and sound CPU using
    // H-DMA as the sample is playing.
	if (match_na("EARTHWORM JIM 2") || // Earth Worm Jim 2
		match_na("PRIMAL RAGE")     || // Primal Rage
		match_na("CLAY FIGHTER")    || // Clay Fighter
		match_na("ClayFighter 2")   || // Clay Fighter 2
		match_na("WeaponLord")      || // Weapon Lord
		match_nn("WAR 2410")        || // War 2410
		match_id("ARF")             || // Star Ocean
		match_id("A4WJ")            || // Mini Yonku Shining Scorpion - Let's & Go!!
		match_nn("NHL")             ||
		match_nc("MADDEN"))
	{
		if (Settings.Shutdown)
			printf("Disabled CPU shutdown hack.\n");
		Settings.Shutdown = FALSE;
	}

	// SA-1
	SA1.WaitAddress = 0xffffffff;
	SA1.WaitByteAddress1 = NULL;
	SA1.WaitByteAddress2 = NULL;

	if (Settings.SA1)
	{
		// Itoi Shigesato no Bass Tsuri No.1 (J)
		if (match_id("ZBPJ"))
		{
			SA1.WaitAddress = 0x0093f1;
			SA1.WaitByteAddress1 = FillRAM + 0x304a;
		}

		// Daisenryaku Expert WWII (J)
		if (match_id("AEVJ"))
		{
			SA1.WaitAddress = 0x0ed18d;
			SA1.WaitByteAddress1 = FillRAM + 0x3000;
		}

		// Derby Jockey 2 (J)
		if (match_id("A2DJ"))
		{
			SA1.WaitAddress = 0x008b62;
		}

		// Dragon Ball Z - Hyper Dimension (J)
		if (match_id("AZIJ"))
		{
			SA1.WaitAddress = 0x008083;
			SA1.WaitByteAddress1 = FillRAM + 0x3020;
		}

		// SD Gundam G NEXT (J)
		if (match_id("ZX3J"))
		{
			SA1.WaitAddress = 0x0087f2;
			SA1.WaitByteAddress1 = FillRAM + 0x30c4;
		}

		// Shougi no Hanamichi (J)
		if (match_id("AARJ"))
		{
			SA1.WaitAddress = 0xc1f85a;
			SA1.WaitByteAddress1 = SRAM + 0x0c64;
			SA1.WaitByteAddress2 = SRAM + 0x0c66;
		}

		// Asahi Shinbun Rensai Katou Hifumi Kudan Shougi Shingiryu (J)
		if (match_id("A23J"))
		{
			SA1.WaitAddress = 0xc25037;
			SA1.WaitByteAddress1 = SRAM + 0x0c06;
			SA1.WaitByteAddress2 = SRAM + 0x0c08;
		}

		// Taikyoku Igo - Idaten (J)
		if (match_id("AIIJ"))
		{
			SA1.WaitAddress = 0xc100be;
			SA1.WaitByteAddress1 = SRAM + 0x1002;
			SA1.WaitByteAddress2 = SRAM + 0x1004;
		}

		// Takemiya Masaki Kudan no Igo Taishou (J)
		if (match_id("AITJ"))
		{
			SA1.WaitAddress = 0x0080b7;
		}

		// J. League '96 Dream Stadium (J)
		if (match_id("AJ6J"))
		{
			SA1.WaitAddress = 0xc0f74a;
		}

		// Jumpin' Derby (J)
		if (match_id("AJUJ"))
		{
			SA1.WaitAddress = 0x00d926;
		}

		// Kakinoki Shougi (J)
		if (match_id("AKAJ"))
		{
			SA1.WaitAddress = 0x00f070;
		}

		// Hoshi no Kirby 3 (J), Kirby's Dream Land 3 (U)
		if (match_id("AFJJ") || match_id("AFJE"))
		{
			SA1.WaitAddress = 0x0082d4;
			SA1.WaitByteAddress1 = SRAM + 0x72a4;
		}

		// Hoshi no Kirby - Super Deluxe (J)
		if (match_id("AKFJ"))
		{
			SA1.WaitAddress = 0x008c93;
			SA1.WaitByteAddress1 = FillRAM + 0x300a;
			SA1.WaitByteAddress2 = FillRAM + 0x300e;
		}

		// Kirby Super Star (U)
		if (match_id("AKFE"))
		{
			SA1.WaitAddress = 0x008cb8;
			SA1.WaitByteAddress1 = FillRAM + 0x300a;
			SA1.WaitByteAddress2 = FillRAM + 0x300e;
		}

		// Super Mario RPG (J), (U)
		if (match_id("ARWJ") || match_id("ARWE"))
		{
			SA1.WaitAddress = 0xc0816f;
			SA1.WaitByteAddress1 = FillRAM + 0x3000;
		}

		// Marvelous (J)
		if (match_id("AVRJ"))
		{
			SA1.WaitAddress = 0x0085f2;
			SA1.WaitByteAddress1 = FillRAM + 0x3024;
		}

		// Harukanaru Augusta 3 - Masters New (J)
		if (match_id("AO3J"))
		{
			SA1.WaitAddress = 0x00dddb;
			SA1.WaitByteAddress1 = FillRAM + 0x37b4;
		}

		// Jikkyou Oshaberi Parodius (J)
		if (match_id("AJOJ"))
		{
			SA1.WaitAddress = 0x8084e5;
		}

		// Super Bomberman - Panic Bomber W (J)
		if (match_id("APBJ"))
		{
			SA1.WaitAddress = 0x00857a;
		}

		// Pebble Beach no Hatou New - Tournament Edition (J)
		if (match_id("AONJ"))
		{
			SA1.WaitAddress = 0x00df33;
			SA1.WaitByteAddress1 = FillRAM + 0x37b4;
		}

		// PGA European Tour (U)
		if (match_id("AEPE"))
		{
			SA1.WaitAddress = 0x003700;
			SA1.WaitByteAddress1 = FillRAM + 0x3102;
		}

		// PGA Tour 96 (U)
		if (match_id("A3GE"))
		{
			SA1.WaitAddress = 0x003700;
			SA1.WaitByteAddress1 = FillRAM + 0x3102;
		}

		// Power Rangers Zeo - Battle Racers (U)
		if (match_id("A4RE"))
		{
			SA1.WaitAddress = 0x009899;
			SA1.WaitByteAddress1 = FillRAM + 0x3000;
		}

		// SD F-1 Grand Prix (J)
		if (match_id("AGFJ"))
		{
			SA1.WaitAddress = 0x0181bc;
		}

		// Saikousoku Shikou Shougi Mahjong (J)
		if (match_id("ASYJ"))
		{
			SA1.WaitAddress = 0x00f2cc;
			SA1.WaitByteAddress1 = SRAM + 0x7ffe;
			SA1.WaitByteAddress2 = SRAM + 0x7ffc;
		}

		// Shougi Saikyou II (J)
		if (match_id("AX2J"))
		{
			SA1.WaitAddress = 0x00d675;
		}

		// Mini Yonku Shining Scorpion - Let's & Go!! (J)
		if (match_id("A4WJ"))
		{
			SA1.WaitAddress = 0xc048be;
		}

		// Shin Shougi Club (J)
		if (match_id("AHJJ"))
		{
			SA1.WaitAddress = 0xc1002a;
			SA1.WaitByteAddress1 = SRAM + 0x0806;
			SA1.WaitByteAddress2 = SRAM + 0x0808;
		}

		// rest games:
		// Habu Meijin no Omoshiro Shougi (J)
		// Hayashi Kaihou Kudan no Igo Taidou (J)
		// Shougi Saikyou (J)
		// Super Robot Wars Gaiden (J)
		// Super Shougi 3 - Kitaihei (J)
	}

	//// SRAM fixes

	if (match_na("HITOMI3"))
	{
		SRAMSize = 1;
		SRAMMask = ((1 << (SRAMSize + 3)) * 128) - 1;
	}

	// SRAM value fixes
	if (match_na("SUPER DRIFT OUT")      || // Super Drift Out
		match_na("SATAN IS OUR FATHER!") ||
		match_na("goemon 4"))               // Ganbare Goemon Kirakira Douchuu
		SNESGameFixes.SRAMInitialValue = 0x00;

    // Additional game fixes by sanmaiwashi ...
	// XXX: unnecessary?
    if (match_na("SFX \xC5\xB2\xC4\xB6\xDE\xDD\xC0\xDE\xD1\xD3\xC9\xB6\xDE\xC0\xD8 1")) // SD Gundam Gaiden - Knight Gundam Monogatari
    	SNESGameFixes.SRAMInitialValue = 0x6b;

	// others: BS and ST-01x games are 0x00.

	//// Specific game fixes

	// for ZSNES SuperFX: is it still necessary?
	Settings.WinterGold = match_na("FX SKIING NINTENDO 96") || match_na("DIRT RACER");

	// OAM hacks because we don't fully understand the behavior of the SNES.
	// Totally wacky display in 2P mode...
	// seems to need a disproven behavior, so we're definitely overlooking some other bug?
	if (match_nn("UNIRACERS")) // Uniracers
	{
		SNESGameFixes.Uniracers = TRUE;
		printf("Applied Uniracers hack.\n");
	}

	/*
	// XXX: What's this?
	if (match_na("\xBD\xB0\xCA\xDF\xB0\xCC\xA7\xD0\xBD\xC0")   || // Super Famista
		match_na("\xBD\xB0\xCA\xDF\xB0\xCC\xA7\xD0\xBD\xC0 2") || // Super Famista 2
		match_na("GANBA LEAGUE"))                                 // Hakunetsu Pro Yakyuu - Ganba League
		SNESGameFixes.APU_OutPorts_ReturnValueFix = TRUE;
	*/
}

// IPS

// Read variable size MSB int from a file
static long ReadInt (Reader *r, unsigned nbytes)
{
    long v = 0;
    while (nbytes--)
    {
		int c = r->get_char();
		if (c == EOF)
			return -1;
		v = (v << 8) | (c & 0xFF);
    }
    return (v);
}

#define IPS_EOF 0x00454F46l

static bool8 ReadIPSPatch (Reader *r, long offset, int32 &rom_size)
{
    char fname[6];
    for(int i=0; i<5; i++){
        int c=r->get_char();
        if(c==EOF) return 0;
        fname[i]=(char) c;
    }
    fname[5]=0;
    if (strncmp (fname, "PATCH", 5) != 0)
    {
        return 0;
    }

    int32 ofs;

    for (;;)
    {
        long len;
        long rlen;
        int  rchar;

        ofs = ReadInt (r, 3);
        if (ofs == -1)
            goto err_eof;

        if (ofs == IPS_EOF)
            break;

        ofs -= offset;

        len = ReadInt (r, 2);
        if (len == -1)
            goto err_eof;

        /* Apply patch block */
        if (len)
        {
            if (ofs + len > CMemory::MAX_ROM_SIZE)
                goto err_eof;

            while (len--)
            {
                rchar = r->get_char();
                if (rchar == EOF)
                    goto err_eof;
                Memory.ROM [ofs++] = (uint8) rchar;
            }
            if (ofs > rom_size)
                rom_size = ofs;
        }
        else
        {
            rlen = ReadInt (r, 2);
            if (rlen == -1)
                goto err_eof;

            rchar = r->get_char();
            if (rchar == EOF)
                goto err_eof;

            if (ofs + rlen > CMemory::MAX_ROM_SIZE)
                goto err_eof;

            while (rlen--)
                Memory.ROM [ofs++] = (uint8) rchar;

            if (ofs > rom_size)
                rom_size = ofs;
        }
    }

    // Check if ROM image needs to be truncated
    ofs = ReadInt (r, 3);
    if (ofs != -1 && ofs - offset < rom_size)
    {
        // Need to truncate ROM image
        rom_size = ofs - offset;
    }
    return 1;

err_eof:
    return 0;
}

static int unzFindExtension (unzFile &file, const char *ext, bool restart, bool print)
{
    int port;
    int l=strlen(ext);

    if(restart){
        port=unzGoToFirstFile(file);
    } else {
        port=unzGoToNextFile(file);
    }
    unz_file_info info;
    while(port==UNZ_OK){
        char name[132];
        unzGetCurrentFileInfo(file, &info, name,128, NULL,0, NULL,0);
        int len=strlen(name);
        if(len>=l+1 && name[len-l-1]=='.' && strcasecmp(name+len-l, ext)==0 &&
           unzOpenCurrentFile(file)==UNZ_OK){
            if(print) printf("Using IPS patch %s", name);
            return port;
        }
        port = unzGoToNextFile(file);
    }
    return port;
}

void CMemory::CheckForIPSPatch (const char *rom_filename, bool8 header, int32 &rom_size)
{
    if(Settings.NoPatch) return;

    char   dir [_MAX_DIR + 1];
    char   drive [_MAX_DRIVE + 1];
    char   name [_MAX_FNAME + 1];
    char   ext [_MAX_EXT + 1];
    char   ips [_MAX_EXT + 3];
    char   fname [_MAX_PATH + 1];
    STREAM patch_file  = NULL;
    long   offset = header ? 512 : 0;
    int    ret;
    bool8  flag;
    uint32 i;

    _splitpath (rom_filename, drive, dir, name, ext);
    _makepath (fname, drive, dir, name, "ips");
    if ((patch_file = OPEN_STREAM (fname, "rb")))
    {
        printf ("Using IPS patch %s", fname);
        ret=ReadIPSPatch(new fReader(patch_file), offset, rom_size);
        CLOSE_STREAM(patch_file);
        if(ret){
            printf("!\n");
            return;
        } else printf(" failed!\n");
    }
    if(_MAX_EXT>6){
        i=0;
        flag=0;
        do {
            snprintf(ips, 8, "%03d.ips", i);
            _makepath(fname, drive, dir, name, ips);
            if(!(patch_file = OPEN_STREAM (fname, "rb"))) break;
            printf ("Using IPS patch %s", fname);
            ret=ReadIPSPatch(new fReader(patch_file), offset, rom_size);
            CLOSE_STREAM(patch_file);
            if(ret){
                flag=1;
                printf("!\n");
            } else {
                printf(" failed!\n");
                break;
            }
        } while(++i<1000);
        if(flag) return;
    }
    if(_MAX_EXT>3){
        i=0;
        flag=0;
        do {
            snprintf(ips, _MAX_EXT+2, "ips%d", i);
            if(strlen(ips)>_MAX_EXT) break;
            _makepath(fname, drive, dir, name, ips);
            if(!(patch_file = OPEN_STREAM (fname, "rb"))) break;
            printf ("Using IPS patch %s", fname);
            ret=ReadIPSPatch(new fReader(patch_file), offset, rom_size);
            CLOSE_STREAM(patch_file);
            if(ret){
                flag=1;
                printf("!\n");
            } else {
                printf(" failed!\n");
                break;
            }
        } while(++i!=0);
        if(flag) return;
    }
    if(_MAX_EXT>2){
        i=0;
        flag=0;
        do {
            snprintf(ips, 4, "ip%d", i);
            _makepath(fname, drive, dir, name, ips);
            if(!(patch_file = OPEN_STREAM (fname, "rb"))) break;
            printf ("Using IPS patch %s", fname);
            ret=ReadIPSPatch(new fReader(patch_file), offset, rom_size);
            CLOSE_STREAM(patch_file);
            if(ret){
                flag=1;
                printf("!\n");
            } else {
                printf(" failed!\n");
                break;
            }
        } while(++i<10);
        if(flag) return;
    }

#ifdef UNZIP_SUPPORT
    if (strcasecmp(ext, "zip")==0) {
        unzFile file=unzOpen(rom_filename);
        if(file!=NULL){
            int port=unzFindExtension(file, "ips");
            while(port==UNZ_OK){
                printf(" in %s", rom_filename);
                ret=ReadIPSPatch(new unzReader(file), offset, rom_size);
                unzCloseCurrentFile(file);
                if(ret){
                    printf("!\n");
                    flag=1;
                } else printf (" failed!\n");
                port = unzFindExtension(file, "ips", false);
            }
            if(!flag){
                i=0;
                do {
                    snprintf(ips, _MAX_EXT+2, "ips%d", i);
                    if(strlen(ips)>_MAX_EXT) break;
                    if(unzFindExtension(file, ips)!=UNZ_OK) break;
                    printf(" in %s", rom_filename);
                    ret=ReadIPSPatch(new unzReader(file), offset, rom_size);
                    unzCloseCurrentFile(file);
                    if(ret){
                        flag=1;
                        printf("!\n");
                    } else {
                        printf(" failed!\n");
                        break;
                    }
                    if(unzFindExtension(file, ips, false, false)==UNZ_OK) printf("WARNING: Ignoring extra .%s files!\n", ips);
                } while(++i!=0);
            }
            if(!flag){
                i=0;
                do {
                    snprintf(ips, 4, "ip%d", i);
                    if(unzFindExtension(file, ips)!=UNZ_OK) break;
                    printf(" in %s", rom_filename);
                    ret=ReadIPSPatch(new unzReader(file), offset, rom_size);
                    unzCloseCurrentFile(file);
                    if(ret){
                        flag=1;
                        printf("!\n");
                    } else {
                        printf(" failed!\n");
                        break;
                    }
                    if(unzFindExtension(file, ips, false, false)==UNZ_OK) printf("WARNING: Ignoring extra .%s files!\n", ips);
                } while(++i<10);
            }
            assert(unzClose(file)==UNZ_OK);
            if(flag) return;
        }
    }
#endif

    const char *n;
    n=S9xGetFilename(".ips", PATCH_DIR);
    if((patch_file=OPEN_STREAM(n, "rb"))) {
        printf("Using IPS patch %s", n);
        ret=ReadIPSPatch(new fReader(patch_file), offset, rom_size);
        CLOSE_STREAM(patch_file);
        if(ret){
            printf("!\n");
            return;
        } else printf(" failed!\n");
    }
    if(_MAX_EXT>6){
        i=0;
        flag=0;
        do {
            snprintf(ips, 9, ".%03d.ips", i);
            n=S9xGetFilename(ips, PATCH_DIR);
            if(!(patch_file = OPEN_STREAM(n, "rb"))) break;
            printf ("Using IPS patch %s", n);
            ret=ReadIPSPatch(new fReader(patch_file), offset, rom_size);
            CLOSE_STREAM(patch_file);
            if(ret){
                flag=1;
                printf("!\n");
            } else {
                printf(" failed!\n");
                break;
            }
        } while(++i<1000);
        if(flag) return;
    }
    if(_MAX_EXT>3){
        i=0;
        flag=0;
        do {
            snprintf(ips, _MAX_EXT+3, ".ips%d", i);
            if(strlen(ips)>_MAX_EXT+1) break;
            n=S9xGetFilename(ips, PATCH_DIR);
            if(!(patch_file = OPEN_STREAM(n, "rb"))) break;
            printf ("Using IPS patch %s", n);
            ret=ReadIPSPatch(new fReader(patch_file), offset, rom_size);
            CLOSE_STREAM(patch_file);
            if(ret){
                flag=1;
                printf("!\n");
            } else {
                printf(" failed!\n");
                break;
            }
        } while(++i!=0);
        if(flag) return;
    }
    if(_MAX_EXT>2){
        i=0;
        flag=0;
        do {
            snprintf(ips, 5, ".ip%d", i);
            n=S9xGetFilename(ips, PATCH_DIR);
            if(!(patch_file = OPEN_STREAM(n, "rb"))) break;
            printf ("Using IPS patch %s", n);
            ret=ReadIPSPatch(new fReader(patch_file), offset, rom_size);
            CLOSE_STREAM(patch_file);
            if(ret){
                flag=1;
                printf("!\n");
            } else {
                printf(" failed!\n");
                break;
            }
        } while(++i<10);
        if(flag) return;
    }
}

#undef INLINE
#define INLINE
#include "getset.h"
