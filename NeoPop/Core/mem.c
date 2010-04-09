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

	mem.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

22 JUL 2002 - neopop_uk
=======================================
- Added default state of interrupt control register, both enabled.
- Added auto detection of rom's colour mode.

25 JUL 2002 - neopop_uk
=======================================
- Added default hardware window values.

27 JUL 2002 - neopop_uk
=======================================
- Bios calls are only decoded in non-hle mode now. The HLE code provides
a more efficient way of decoding calls. 

01 AUG 2002 - neopop_uk
=======================================
- Added a default background colour. Improves "Mezase! Kanji Ou"

01 AUG 2002 - neopop_uk
=======================================
- Added default settings for memory error and mask memory error.
  
10 AUG 2002 - neopop_uk
=======================================
- Moved all of the bios memory setup from bios.c
- Added more detailed setup of memory, to fool clever games.
- Added raster horizontal position emulation, fixes "NeoGeo Cup '98 Plus Color"

15 AUG 2002 - neopop_uk
=======================================
- Made EEPROM status more secure if something goes wrong.

16 AUG 2002 - neopop_uk
=======================================
- Ignores direct EEPROM access to a rom that's too short.

05 SEP 2002 - neopop_uk
=======================================
- In debug mode, NULL memory read/writes only create an error if the
	error message mask is disabled.

07 SEP 2002 - neopop_uk
=======================================
- Made the direct EEPROM access more secure by requiring an EEPROM
command to be issued before every write. This isn't perfect, but it
should stop "Gals Fighters" from writing all over itself.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "TLCS900h_registers.h"
#include "Z80_interface.h"
#include "bios.h"
#include "gfx.h"
#include "mem.h"
#include "interrupt.h"
#include "sound.h"
#include "flash.h"

//=============================================================================

//Hack way of returning good EEPROM status.
BOOL eepromStatusEnable = FALSE;
static _u32 eepromStatus;	

_u8 ram[1 + RAM_END - RAM_START];

BOOL debug_abort_memory = FALSE;
BOOL debug_mask_memory_error_messages = FALSE;

BOOL memory_unlock_flash_write = FALSE;
BOOL memory_flash_error = FALSE;
BOOL memory_flash_command = FALSE;

//=============================================================================

#ifdef NEOPOP_DEBUG
static void memory_error(_u32 address, BOOL read)
{
	debug_abort_memory = TRUE;

	if (filter_mem)
	{
		if (debug_mask_memory_error_messages)
			return;

		if (read)
			system_debug_message("Memory Exception: Read from %06X", address);
		else
			system_debug_message("Memory Exception: Write to %06X", address);
	}
}
#endif

//=============================================================================

void* translate_address_read(_u32 address)
{
	address &= 0xFFFFFF;

#ifdef NEOPOP_DEBUG

	if (address == 0 && debug_mask_memory_error_messages == FALSE)
	{ memory_error(address, TRUE); return NULL; }

#endif

	// ===================================

	//RAS.H read (Simulated horizontal raster position)
	if (address == 0x8008)
		ram[0x8008] = (_u8)((abs(TIMER_HINT_RATE - (int)timer_hint)) >> 2);

	if (address <= RAM_END)
		return ram + address;

	// ===================================

	//Get EEPROM status?
	if (eepromStatusEnable)
	{
		eepromStatusEnable = FALSE;
		if (address == 0x220000 || address == 0x230000)
		{
			eepromStatus = 0xFFFFFFFF;
			return &eepromStatus;
		}
	}

	//ROM (LOW)
	if (rom.data && address >= ROM_START && address <= ROM_END)
	{
		if (address <= ROM_START + rom.length)
			return rom.data + (address - ROM_START);
		else
			return NULL;
	}

	//ROM (HIGH)
	if (rom.data && address >= HIROM_START && address <= HIROM_END)
	{
		if (address <= HIROM_START + (rom.length - 0x200000))
			return rom.data + 0x200000 + (address - HIROM_START);
		else
			return NULL;
	}

	// ===================================

	//BIOS Access?
	if ((address & 0xFF0000) == 0xFF0000)
		return bios + (address & 0xFFFF); // BIOS ROM

	// ===================================

	//Signal a flash memory error
	if (memory_unlock_flash_write)
		memory_flash_error = TRUE;

#ifdef NEOPOP_DEBUG
	memory_error(address, TRUE);
#endif
	return NULL;
}

//=============================================================================

void* translate_address_write(_u32 address)
{	
	address &= 0xFFFFFF;

#ifdef NEOPOP_DEBUG

	if (address == 0 && debug_mask_memory_error_messages == FALSE)
	{ memory_error(address, FALSE); return NULL; }

#endif

	// ===================================


	if (address <= RAM_END)
		return ram + address;

	// ===================================

	if (memory_unlock_flash_write)
	{
		//ROM (LOW)
		if (rom.data && address >= ROM_START && address <= ROM_END)
		{
			if (address <= ROM_START + rom.length)
				return rom.data + (address - ROM_START);
			else
				return NULL;
		}

		//ROM (HIGH)
		if (rom.data && address >= HIROM_START && address <= HIROM_END)
		{
			if (address <= HIROM_START + (rom.length - 0x200000))
				return rom.data + 0x200000 + (address - HIROM_START);
			else
				return NULL;
		}

		//Signal a flash memory error
		memory_flash_error = TRUE;
	}
	else
	{
		//ROM (LOW)
		if (rom.data && address >= ROM_START && address <= ROM_END)
		{
			//Ignore EEPROM commands
			if (address == 0x202AAA || address == 0x205555)
			{
	//			system_debug_message("%06X: Enable EEPROM command from %06X", pc, address);
				memory_flash_command = TRUE;
				return NULL;
			}

			//Set EEPROM status reading?
			if (address == 0x220000 || address == 0x230000)
			{
	//			system_debug_message("%06X: EEPROM status read from %06X", pc, address);
				eepromStatusEnable = TRUE;
				return NULL;
			}

			if (memory_flash_command)
			{
				//Write the 256byte block around the flash data
				flash_write(address & 0xFFFF00, 256);
				
				//Need to issue a new command before writing will work again.
				memory_flash_command = FALSE;
		
	//			system_debug_message("%06X: Direct EEPROM write to %06X", pc, address & 0xFFFF00);
	//			system_debug_stop();

				//Write to the rom itself.
				if (address <= ROM_START + rom.length)
					return rom.data + (address - ROM_START);
			}
		}
	}

	// ===================================

#ifdef NEOPOP_DEBUG
	memory_error(address, FALSE);
#endif
	return NULL;
}

//=============================================================================

void post_write(_u32 address)
{
	address &= 0xFFFFFF;

	//Direct Access to Sound Chips
	if ((*(_u16*)(ram + 0xb8)) == htole16(0xAA55))
	{
		if (address == 0xA1)	Write_SoundChipTone(ram[0xA1]);
		if (address == 0xA0)	Write_SoundChipNoise(ram[0xA0]);
	}

	//DAC Write
	if (address == 0xA2)	dac_write();

	//Clear counters?
	if (address == 0x20)
	{
		_u8 TRUN = ram[0x20];

		if ((TRUN & 0x01) == 0)		timer[0] = 0;
		if ((TRUN & 0x02) == 0)		timer[1] = 0;
		if ((TRUN & 0x04) == 0)		timer[2] = 0;
		if ((TRUN & 0x08) == 0)		timer[3] = 0;
	}

	//z80 - NMI
	if (address == 0xBA)
		Z80_nmi();
}

//=============================================================================

_u8 loadB(_u32 address)
{
	_u8* ptr = translate_address_read(address);
	if (ptr == NULL)
		return 0;
	else
		return *ptr;
}

_u16 loadW(_u32 address)
{
	_u16* ptr = translate_address_read(address);
	if (ptr == NULL)
		return 0;
	else
		return le16toh(*ptr);
}

_u32 loadL(_u32 address)
{
	_u32* ptr = translate_address_read(address);
	if (ptr == NULL)
		return 0;
	else
		return le32toh(*ptr);
}

//=============================================================================

void storeB(_u32 address, _u8 data)
{
	_u8* ptr = translate_address_write(address);

	//Write
	if (ptr)
	{
		*ptr = data;
		post_write(address);
	}
}

void storeW(_u32 address, _u16 data)
{
	_u16* ptr = translate_address_write(address);

	//Write
	if (ptr)
	{
		*ptr = htole16(data);
		post_write(address);
	}
}

void storeL(_u32 address, _u32 data)
{
	_u32* ptr = translate_address_write(address);

	//Write
	if (ptr)
	{
		*ptr = htole32(data);
		post_write(address);
	}
}

//=============================================================================

static _u8 systemMemory[] = 
{
	// 0x00												// 0x08
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x08, 0xFF, 0xFF,
	// 0x10												// 0x18
	0x34, 0x3C, 0xFF, 0xFF, 0xFF, 0x3F, 0x00, 0x00,		0x3F, 0xFF, 0x2D, 0x01, 0xFF, 0xFF, 0x03, 0xB2,
	// 0x20												// 0x28
	0x80, 0x00, 0x01, 0x90, 0x03, 0xB0, 0x90, 0x62,		0x05, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x4C, 0x4C,
	// 0x30												// 0x38
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x30, 0x00, 0x00, 0x00, 0x20, 0xFF, 0x80, 0x7F,
	// 0x40												// 0x48
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0x50												// 0x58
	0x00, 0x20, 0x69, 0x15, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	// 0x60												// 0x68
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x17, 0x17, 0x03, 0x03, 0x02, 0x00, 0x00, 0x4E,
	// 0x70												// 0x78
	0x02, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0x80												// 0x88
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0x90												// 0x98
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xA0												// 0xA8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xB0												// 0xB8
	0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,		0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xC0												// 0xC8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xD0												// 0xD8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xE0												// 0xE8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xF0												// 0xF8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//=============================================================================

void reset_memory(void)
{
	int i;

	eepromStatusEnable = FALSE;
	memory_flash_command = FALSE;

	memset(ram, 0, sizeof(ram));	//Clear ram

//=============================================================================
//000000 -> 000100	CPU Internal RAM (Timers/DMA/Z80)
//=====================================================

	for (i = 0; i < sizeof(systemMemory); i++)
		ram[i] = systemMemory[i];

//=============================================================================
//006C00 -> 006FFF	BIOS Workspace
//==================================

	if (rom.data)
	{
		*(_u32*)(ram + 0x6C00) = rom_header->startPC;		//Start
		*(_u16*)(ram + 0x6E82) = 
			*(_u16*)(ram + 0x6C04) = rom_header->catalog;	//Catalog
		*( _u8*)(ram + 0x6E84) = 
			*( _u8*)(ram + 0x6C06) = rom_header->subCatalog;	//Sub-Cat
		memcpy(ram + 0x6C08, rom.data + 0x24, 12);			//name

		*(_u8*)(ram + 0x6C58) = 0x01;			//LO-EEPROM present

		//32MBit cart?
		if (rom.length > 0x200000)
			*(_u8*)(ram + 0x6C59) = 0x01;			//HI-EEPROM present
		else
			*(_u8*)(ram + 0x6C59) = 0x00;			//HI-EEPROM not present

		ram[0x6C55] = 1;	//Commercial game
	}
	else
	{
		*(_u32*)(ram + 0x6C00) = htole32(0x00FF970A);	//Start
		*(_u16*)(ram + 0x6C04) = htole16(0xFFFF);	//Catalog
		*( _u8*)(ram + 0x6C06) = 0x00;			//Sub-Cat
		sprintf(ram + 0x6C08, "NEOGEOPocket");	//bios rom 'Name'

		*(_u8*)(ram + 0x6C58) = 0x00;			//LO-EEPROM not present
		*(_u8*)(ram + 0x6C59) = 0x00;			//HI-EEPROM not present

		ram[0x6C55] = 0;	//Bios menu
	}
	

	ram[0x6F80] = 0xFF;	//Lots of battery power!
	ram[0x6F81] = 0x03;

	ram[0x6F84] = 0x40;	// "Power On" startup
	ram[0x6F85] = 0x00;	// No shutdown request
	ram[0x6F86] = 0x00;	// No user answer (?)

	//Language: 0 = Japanese, 1 = English
	ram[0x6F87] = (_u8)language_english;	

	//Color Mode Selection: 0x00 = B&W, 0x10 = Colour
	if (system_colour == COLOURMODE_GREYSCALE)
		ram[0x6F91] = ram[0x6F95] = 0x00; //Force Greyscale
	if (system_colour == COLOURMODE_COLOUR)	
		ram[0x6F91] = ram[0x6F95] = 0x10; //Force Colour
	if (system_colour == COLOURMODE_AUTO) 
	{
		if (rom.data) 
			ram[0x6F91] = ram[0x6F95] = rom_header->mode;	//Auto-detect
		else 
			ram[0x6F91] = ram[0x6F95] = 0x10; // Default = Colour
	}

	//Interrupt table
	for (i = 0; i < 0x12; i++)
		*(_u32*)(ram + 0x6FB8 + (i * 4)) = htole32(0x00FF23DF);


//=============================================================================
//008000 -> 00BFFF	Video RAM
//=============================

	ram[0x8000] = 0xC0;	// Both interrupts allowed

	//Hardware window
	ram[0x8002] = 0x00;
	ram[0x8003] = 0x00;
	ram[0x8004] = 0xFF;
	ram[0x8005] = 0xFF;

	ram[0x8006] = 0xc6;	// Frame Rate Register

	ram[0x8012] = 0x00;	// NEG / OOWC setting.

	ram[0x8118] = 0x80;	// BGC on!

	ram[0x83E0] = 0xFF;	// Default background colour
	ram[0x83E1] = 0x0F;

	ram[0x83F0] = 0xFF;	// Default window colour
	ram[0x83F1] = 0x0F;

	ram[0x8400] = 0xFF;	// LED on
	ram[0x8402] = 0x80;	// Flash cycle = 1.3s
}

//=============================================================================


