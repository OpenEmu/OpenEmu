//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------
//=========================================================================

	rom.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- Added new function 'rom_loaded' which unifies rom_hack, rom_display_header,
	and also builds RomInfo::name.

21 JUL 2002 - neopop_uk
=======================================
- Added a hack for "Evolution - Eternal Dungeons" to allow you to play in
	one of the dungeon stages. Not 100% sure this is a good hack...

25 JUL 2002 - neopop_uk
=======================================
- Removed hack for Evolution - it was bad.

26 JUL 2002 - neopop_uk
=======================================
- Added graphics hacks for all of the affected games.

27 JUL 2002 - neopop_uk
=======================================
- Added hack for "Ogre Battle" to fix the background colour - seems to
	look correct now, but the code I change was quite explicit in it's
	task of disabling the colour.

01 AUG 2002 - neopop_uk
=======================================
- Removed the stupid hack for "Metal Slug 2" that was making it run
	too fast, and not solving the problem it was meant too.

01 AUG 2002 - neopop_uk
=======================================
- Removed the "Densetsu no Ogre Battle" hack. It's not required any more.

04 AUG 2002 - neopop_uk
=======================================
- Moved common rom management tasks back into this code.

10 AUG 2002 - neopop_uk
=======================================
- No need for "Sonic Pocket Adventure" hack any more.

16 AUG 2002 - neopop_uk
=======================================
- Added hacks for "Cool Cool Jam SAMPLE" and "Dokodemo Mahjong"
	Their rom headers gave incorrect colour mode information.

23 AUG 2002 - neopop_uk
=======================================
- Added hack for "Neo-Neo! 1.0", it's rom header also had incorrect
	colour mode information.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "flash.h"
#include "interrupt.h"

//=============================================================================

RomInfo rom;
RomHeader* rom_header;

#define MATCH_CATALOG(c, s)	(rom_header->catalog == htole16(c) \
				 && rom_header->subCatalog == (s))

//=============================================================================

static void rom_hack(void)
{
	gfx_hack = FALSE;

	//=============================
	// GRAPHICS HACKS
	//=============================
	if (MATCH_CATALOG(89, 5) ||		//Sonic
		MATCH_CATALOG(149, 36) ||	//LastBlade(US)
		MATCH_CATALOG(100, 31) ||	//LastBlade(J)
		MATCH_CATALOG(2, 8) ||		//NG.Cup
		MATCH_CATALOG(57, 14) ||	//NG.Cup+Colour
		MATCH_CATALOG(133, 8) ||	//Ogre
		MATCH_CATALOG(148, 4) ||	//Rockman
		MATCH_CATALOG(105, 21) ||	//SNKvsCAPCOM
		MATCH_CATALOG(48, 26) ||	//Sam.Shodown2
		MATCH_CATALOG(102, 241) ||	//Wrestle-Beta
		MATCH_CATALOG(102, 2) ||	//Pro-Wrestle
		MATCH_CATALOG(1, 10) ||		//KOF-R1
		MATCH_CATALOG(35, 21))		//KOF-R2
	{
		gfx_hack = TRUE;
#ifdef NEOPOP_DEBUG
		system_debug_message("HACK: Graphics Timing");
#endif
	}

	//### Quick way of displaying the rom information
//	system_message("%d %d", le16toh(rom_header->catalog),
//		       rom_header->subCatalog); 
//	gfx_hack = TRUE;

	//=============================
	// SPECIFIC ROM HACKS !
	//=============================

	//"Neo-Neo! V1.0 (PD)"
	if (MATCH_CATALOG(0, 16))
	{
		rom.data[0x23] = 0x10;	// Fix rom header

#ifdef NEOPOP_DEBUG
		system_debug_message("HACK: \"Neo-Neo! V1.0 (PD)\"");
#endif
	}

	//"Cool Cool Jam SAMPLE (U)"
	if (MATCH_CATALOG(4660, 161))
	{
		rom.data[0x23] = 0x10;	// Fix rom header

#ifdef NEOPOP_DEBUG
		system_debug_message("HACK: \"Cool Cool Jam SAMPLE (U)\"");
#endif
	}

	//"Dokodemo Mahjong (J)"
	if (MATCH_CATALOG(51, 33))
	{
		rom.data[0x23] = 0x00;	// Fix rom header

#ifdef NEOPOP_DEBUG
		system_debug_message("HACK: \"Dokodemo Mahjong (J)\"");
#endif
	}

	//"Puyo Pop (V05) (JUE)"
	if (MATCH_CATALOG(65, 5))
	{
		int i;
		for (i = 0x8F0; i < 0x8FC; i++)
			rom.data[i] = 0;

#ifdef NEOPOP_DEBUG
		system_debug_message("HACK: \"Puyo Pop (V05) (JUE)\"");
#endif
	}

	//"Puyo Pop (V06) (JUE)"
	if (MATCH_CATALOG(65, 6))
	{
		int i;
		for (i = 0x8F0; i < 0x8FC; i++)
			rom.data[i] = 0;

#ifdef NEOPOP_DEBUG
		system_debug_message("HACK: \"Puyo Pop (V06) (JUE)\"");
#endif
	}

	//"Metal Slug - 2nd Mission (JUE) [!]"
	//"Metal Slug - 2nd Mission (JUE) [h1]"
	if (MATCH_CATALOG(97, 4))
	{
		//Enable dev-kit code path, because otherwise it doesn't
		//allow jumping or firing (for some reason!)
		
		rom.data[0x1f] = 0xFF;

		//Enables in-game voices ("Pineapple", etc.)
		//that were aren't supposed to be available in Dev-kit mode.
		rom.data[0x8DDF8] = 0xF0;	//28DDF7: "RET NZ" -> "RET F"

#ifdef NEOPOP_DEBUG
		system_debug_message("HACK: \"Metal Slug - 2nd Mission (JUE)\"");
#endif
	}
}

//=============================================================================

static void rom_display_header(void)
{
#ifdef NEOPOP_DEBUG
	system_debug_message("\nRom Header =========================");
	system_debug_message("\"%s\"", rom.name);

	//Colour / BW mode?
	switch(rom_header->mode)
	{
	case 0x00: system_debug_message("Presented in Black & White"); break;
	case 0x10: system_debug_message("Presented in Colour"); break;
	default: system_debug_message("Presented in Unknown mode %x", rom_header->mode); break;
	}

	//Catalog Numbers
	system_debug_message("Catalogue %d (sub %d)",
			     le16toh(rom_header->catalog),
			     rom_header->subCatalog);

	//Starting PC
	system_debug_message("Starting PC = %06X",
			     le32toh(rom_header->startPC) & 0xFFFFFF);

	system_debug_message("====================================\n");
#endif
}

//=============================================================================

//-----------------------------------------------------------------------------
// rom_loaded()
//-----------------------------------------------------------------------------
void rom_loaded(void)
{
	int i;

	//Extract the header
	rom_header = (RomHeader*)(rom.data);

	//Rom Name
	for(i = 0; i < 12; i++)
	{
		if (rom_header->name[i] >= 32 && rom_header->name[i] < 128)
			rom.name[i] = rom_header->name[i];
		else
			rom.name[i] = ' ';
	}
	rom.name[i] = 0;

	rom_hack();	//Apply a hack if reuqired!

	rom_display_header();	//Show the header (debugger only)

	flash_read();
}

//-----------------------------------------------------------------------------
// rom_unload()
//-----------------------------------------------------------------------------
void rom_unload(void)
{
	if (rom.data)
	{
		int i;

		flash_commit();

		free(rom.data);
		rom.data = NULL;
		rom.length = 0;
		rom_header = 0;

		for (i = 0; i < 16; i++)
			rom.name[i] = 0;

		for (i = 0; i < 256; i++)
			rom.filename[i] = 0;

		reset();

#ifdef NEOPOP_DEBUG
		system_debug_clear();
		system_debug_message("Game removed. Returning to bios...");
#endif
	}			
}

//=============================================================================
