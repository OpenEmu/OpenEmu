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

#include "neopop.h"
#include "flash.h"
#include "interrupt.h"
#include "TLCS900h_disassemble.h"

//=============================================================================

RomInfo ngpc_rom;
RomHeader* rom_header;

#define MATCH_CATALOG(c, s)	(rom_header->catalog == htole16(c) \
				 && rom_header->subCatalog == (s))

//=============================================================================

static void rom_hack(void)
{
	//=============================
	// SPECIFIC ROM HACKS !
	//=============================

	//"Neo-Neo! V1.0 (PD)"
	if (MATCH_CATALOG(0, 16))
	{
		ngpc_rom.data[0x23] = 0x10;	// Fix rom header

		MDFN_printf("HACK: \"Neo-Neo! V1.0 (PD)\"\n");
	}

	//"Cool Cool Jam SAMPLE (U)"
	if (MATCH_CATALOG(4660, 161))
	{
		ngpc_rom.data[0x23] = 0x10;	// Fix rom header

		MDFN_printf("HACK: \"Cool Cool Jam SAMPLE (U)\"\n");
	}

	//"Dokodemo Mahjong (J)"
	if (MATCH_CATALOG(51, 33))
	{
		ngpc_rom.data[0x23] = 0x00;	// Fix rom header

		MDFN_printf("HACK: \"Dokodemo Mahjong (J)\"\n");
	}

	//"Puyo Pop (V05) (JUE)"
	if (MATCH_CATALOG(65, 5))
	{
		int i;
		for (i = 0x8F0; i < 0x8FC; i++)
			ngpc_rom.data[i] = 0;

		MDFN_printf("HACK: \"Puyo Pop (V05) (JUE)\"\n");
	}

	//"Puyo Pop (V06) (JUE)"
	if (MATCH_CATALOG(65, 6))
	{
		int i;
		for (i = 0x8F0; i < 0x8FC; i++)
			ngpc_rom.data[i] = 0;
		//extern uint32 pc;
		//pc = 0x200000 + 0x100;
		//for(int x = 0; x < 65536; x++)
		//puts(TLCS900h_disassemble());
		MDFN_printf("HACK: \"Puyo Pop (V06) (JUE)\"\n");
	}

	//"Metal Slug - 2nd Mission (JUE) [!]"
	//"Metal Slug - 2nd Mission (JUE) [h1]"
	if (MATCH_CATALOG(97, 4))
	{
		//Enable dev-kit code path, because otherwise it doesn't
		//allow jumping or firing (for some reason!)
		
		ngpc_rom.data[0x1f] = 0xFF;

		//Enables in-game voices ("Pineapple", etc.)
		//that were aren't supposed to be available in Dev-kit mode.
		ngpc_rom.data[0x8DDF8] = 0xF0;	//28DDF7: "RET NZ" -> "RET F"

		MDFN_printf("HACK: \"Metal Slug - 2nd Mission (JUE)\"\n");
	}
}

//=============================================================================

static void rom_display_header(void)
{
	MDFN_printf(_("Name:    %s\n"), ngpc_rom.name);
	MDFN_printf(_("System:  "));

	if(rom_header->mode & 0x10)
	 MDFN_printf(_("Color"));
	else
	 MDFN_printf(_("Greyscale"));

	MDFN_printf("\n");

        MDFN_printf(_("Catalog:  %d (sub %d)\n"),
                             le16toh(rom_header->catalog),
                             rom_header->subCatalog);

        //Starting PC
        MDFN_printf(_("Starting PC:  0x%06X\n"), le32toh(rom_header->startPC) & 0xFFFFFF);
}

//=============================================================================

//-----------------------------------------------------------------------------
// rom_loaded()
//-----------------------------------------------------------------------------
void rom_loaded(void)
{
	int i;

	ngpc_rom.orig_data = (uint8 *)malloc(ngpc_rom.length);
	memcpy(ngpc_rom.orig_data, ngpc_rom.data, ngpc_rom.length);

	//Extract the header
	rom_header = (RomHeader*)(ngpc_rom.data);

	//Rom Name
	for(i = 0; i < 12; i++)
	{
		if (rom_header->name[i] >= 32 && rom_header->name[i] < 128)
			ngpc_rom.name[i] = rom_header->name[i];
		else
			ngpc_rom.name[i] = ' ';
	}
	ngpc_rom.name[i] = 0;

	rom_hack();	//Apply a hack if required!

	rom_display_header();

	flash_read();
}

//-----------------------------------------------------------------------------
// rom_unload()
//-----------------------------------------------------------------------------
void rom_unload(void)
{
 if (ngpc_rom.data)
 {
	int i;

	flash_commit();

	free(ngpc_rom.data);
	ngpc_rom.data = NULL;
	ngpc_rom.length = 0;
	rom_header = 0;

	for (i = 0; i < 16; i++)
	 ngpc_rom.name[i] = 0;
 }		

 if(ngpc_rom.orig_data)
 {
  free(ngpc_rom.orig_data);
  ngpc_rom.orig_data = NULL;
 }
}

//=============================================================================
