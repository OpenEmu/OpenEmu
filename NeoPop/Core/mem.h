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

	mem.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

15 AUG 2002 - neopop_uk
=======================================
- Removed the legacy 'eeprom' variables.

18 AUG 2002 - neopop_uk
=======================================
- Moved RAM_START/RAM_END definition and ram[] declaration to NeoPop.h

//---------------------------------------------------------------------------
*/

#ifndef __MEM__
#define __MEM__
//=============================================================================

#define ROM_START	0x200000
#define ROM_END		0x3FFFFF

#define HIROM_START	0x800000
#define HIROM_END	0x9FFFFF

#define BIOS_START	0xFF0000
#define BIOS_END	0xFFFFFF

void reset_memory(void);

void* translate_address_read(_u32 address);
void* translate_address_write(_u32 address);

void dump_memory(_u32 start, _u32 length);

extern BOOL debug_abort_memory;
extern BOOL debug_mask_memory_error_messages;

extern BOOL memory_unlock_flash_write;
extern BOOL memory_flash_error;
extern BOOL memory_flash_command;

extern BOOL eepromStatusEnable;

//=============================================================================

_u8  loadB(_u32 address);
_u16 loadW(_u32 address);
_u32 loadL(_u32 address);

void storeB(_u32 address, _u8 data);
void storeW(_u32 address, _u16 data);
void storeL(_u32 address, _u32 data);

//=============================================================================
#endif
