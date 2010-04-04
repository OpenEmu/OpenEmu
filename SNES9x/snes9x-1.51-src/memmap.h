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



#ifndef _memmap_h_
#define _memmap_h_

#include "snes9x.h"

#define MEMMAP_BLOCK_SIZE				(0x1000)
#define MEMMAP_NUM_BLOCKS				(0x1000000 / MEMMAP_BLOCK_SIZE)
#define MEMMAP_SHIFT					(12)
#define MEMMAP_MASK						(MEMMAP_BLOCK_SIZE - 1)
#define MEMMAP_MAX_SDD1_LOGGED_ENTRIES	(0x10000 / 8)

struct CMemory
{
	enum
	{ MAX_ROM_SIZE = 0x800000 };

	enum file_formats
	{ FILE_ZIP, FILE_JMA, FILE_DEFAULT };

	enum
	{ NOPE, YEAH, BIGFIRST, SMALLFIRST };

	enum
	{ MAP_TYPE_I_O, MAP_TYPE_ROM, MAP_TYPE_RAM };

	enum
	{
		MAP_PPU,
		MAP_CPU,
		MAP_LOROM_SRAM,
		MAP_LOROM_SRAM_B,
		MAP_HIROM_SRAM,
		MAP_DSP,
		MAP_C4,
		MAP_BWRAM,
		MAP_BWRAM_BITMAP,
		MAP_BWRAM_BITMAP2,
		MAP_SA1RAM,
		MAP_SPC7110_ROM,
		MAP_SPC7110_DRAM,
		MAP_RONLY_SRAM,
		MAP_OBC_RAM,
		MAP_SETA_DSP,
		MAP_SETA_RISC,
		MAP_BSX,
		MAP_NONE,
		MAP_DEBUG,
		MAP_LAST
	};

	uint8	NSRTHeader[32];
	int32	HeaderCount;

	uint8	*RAM;
	uint8	*ROM;
	uint8	*SRAM;
	uint8	*VRAM;
	uint8	*FillRAM;
	uint8	*BWRAM;
	uint8	*C4RAM;
	uint8	*BSRAM;
	uint8	*BIOSROM;

	uint8	*Map[MEMMAP_NUM_BLOCKS];
	uint8	*WriteMap[MEMMAP_NUM_BLOCKS];
	uint8	BlockIsRAM[MEMMAP_NUM_BLOCKS];
	uint8	BlockIsROM[MEMMAP_NUM_BLOCKS];
	uint8	MemorySpeed[MEMMAP_NUM_BLOCKS];
	uint8	ExtendedFormat;

	char	ROMFilename[_MAX_PATH + 1];
	char	ROMName[ROM_NAME_LEN];
	char	RawROMName[ROM_NAME_LEN];
	char	ROMId[5];
	char	CompanyId[3];
	uint8	ROMRegion;
	uint8	ROMSpeed;
	uint8	ROMType;
	uint8	ROMSize;
	uint32	ROMChecksum;
	uint32	ROMComplementChecksum;
	uint32	ROMCRC32;
	int32	ROMFramesPerSecond;

	bool8	HiROM;
	bool8	LoROM;
	uint8	SRAMSize;
	uint32	SRAMMask;
	uint32	CalculatedSize;
	uint32	CalculatedChecksum;

	uint8	*SDD1Index;
	uint8	*SDD1Data;
	uint32	SDD1Entries;
	uint32	SDD1LoggedDataCountPrev;
	uint32	SDD1LoggedDataCount;
	uint8	SDD1LoggedData[MEMMAP_MAX_SDD1_LOGGED_ENTRIES];

	// ports can assign this to perform some custom action upon loading a ROM (such as adjusting controls)
	void	(*PostRomInitFunc) ();

	bool8	Init (void);
	void	Deinit (void);
	void	FreeSDD1Data (void);

	int		ScoreHiROM (bool8, int32 romoff = 0);
	int		ScoreLoROM (bool8, int32 romoff = 0);
	uint32	HeaderRemove (uint32, int32 &, uint8 *);
	uint32	FileLoader (uint8 *, const char *, int32);
	bool8	LoadROM (const char *);
	bool8	LoadMultiCart (const char *, const char *);
	bool8	LoadSufamiTurbo (const char *, const char *);
	bool8	LoadSameGame (const char *, const char *);
	bool8	LoadLastROM (void);
	bool8	LoadSRAM (const char *);
	bool8	SaveSRAM (const char *);
	void	ClearSRAM (bool8 onlyNonSavedSRAM = 0);

	char *	Safe (const char *);
	char *	SafeANK (const char *);
	void	ParseSNESHeader (uint8 *);
	void	InitROM (void);
	void	FixROMSpeed (void);
	void	ResetSpeedMap (void);

	uint32	map_mirror (uint32, uint32);
	void	map_lorom (uint32, uint32, uint32, uint32, uint32);
	void	map_hirom (uint32, uint32, uint32, uint32, uint32);
	void	map_lorom_offset (uint32, uint32, uint32, uint32, uint32, uint32);
	void	map_hirom_offset (uint32, uint32, uint32, uint32, uint32, uint32);
	void	map_space (uint32, uint32, uint32, uint32, uint8 *);
	void	map_index (uint32, uint32, uint32, uint32, int, int);
	void	map_System (void);
	void	map_WRAM (void);
	void	map_LoROMSRAM (void);
	void	map_HiROMSRAM (void);
	void	map_DSP (void);
	void	map_C4 (void);
	void	map_OBC1 (void);
	void	map_SetaRISC (void);
	void	map_SetaDSP (void);
	void	map_WriteProtectROM (void);
	void	Map_Initialize (void);
	void	Map_LoROMMap (void);
	void	Map_NoMAD1LoROMMap (void);
	void	Map_JumboLoROMMap (void);
	void	Map_ROM24MBSLoROMMap (void);
	void	Map_SRAM512KLoROMMap (void);
	void	Map_SufamiTurboLoROMMap (void);
	void	Map_SufamiTurboPseudoLoROMMap (void);
	void	Map_SuperFXLoROMMap (void);
	void	Map_SetaDSPLoROMMap (void);
	void	Map_SDD1LoROMMap (void);
	void	Map_SA1LoROMMap (void);
	void	Map_HiROMMap (void);
	void	Map_ExtendedHiROMMap (void);
	void	Map_SameGameHiROMMap (void);
	void	Map_SPC7110HiROMMap (void);

	uint16	checksum_calc_sum (uint8 *, uint32);
	uint16	checksum_mirror_sum (uint8 *, uint32 &, uint32 mask = 0x800000);
	void	Checksum_Calculate (void);

	bool8	match_na (const char *);
	bool8	match_nn (const char *);
	bool8	match_nc (const char *);
	bool8	match_id (const char *);
	void	ApplyROMFixes (void);
	void	CheckForIPSPatch (const char *, bool8, int32 &);

	const char *	TVStandard (void);
	const char *	MapType (void);
	const char *	MapMode (void);
	const char *	StaticRAMSize (void);
	const char *	Size (void);
	const char *	Revision (void);
	const char *	KartContents (void);
};

struct SMulti
{
	int		cartType;
	int32	cartSizeA, cartSizeB;
	int32	sramSizeA, sramSizeB;
	uint32	sramMaskA, sramMaskB;
	uint32	cartOffsetA, cartOffsetB;
	uint8	*sramA, *sramB;
	char	fileNameA[_MAX_PATH + 1], fileNameB[_MAX_PATH + 1];
};

START_EXTERN_C
extern CMemory	Memory;
extern SMulti	Multi;
#if defined(ZSNES_FX) || defined(ZSNES_C4)
extern uint8	*ROM;
extern uint8	*SRAM;
extern uint8	*RegRAM;
#endif
bool8 LoadZip(const char *, int32 *, int32 *, uint8 *);
END_EXTERN_C

void S9xAutoSaveSRAM (void);

enum s9xwrap_t
{
	WRAP_NONE,
	WRAP_BANK,
	WRAP_PAGE
};

enum s9xwriteorder_t
{
	WRITE_01,
	WRITE_10
};

#ifdef NO_INLINE_SET_GET

uint8	S9xGetByte (uint32);
uint16	S9xGetWord (uint32, enum s9xwrap_t w = WRAP_NONE);
void	S9xSetByte (uint8, uint32);
void	S9xSetWord (uint16, uint32, enum s9xwrap_t w = WRAP_NONE, enum s9xwriteorder_t o = WRITE_01);
void	S9xSetPCBase (uint32);
uint8 *	S9xGetMemPointer (uint32);
uint8 *	GetBasePointer (uint32);

START_EXTERN_C
extern uint8	OpenBus;
END_EXTERN_C

#else

#define INLINE inline
#include "getset.h"

#endif // NO_INLINE_SET_GET

#endif // _memmap_h_
