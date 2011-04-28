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

void dump_memory(uint32 start, uint32 length);

extern bool debug_abort_memory;
extern bool debug_mask_memory_error_messages;

extern bool memory_unlock_flash_write;
extern bool memory_flash_error;
extern bool memory_flash_command;

extern bool FlashStatusEnable;
extern uint8 COMMStatus;

//=============================================================================

uint8  loadB(uint32 address);
uint16 loadW(uint32 address);
uint32 loadL(uint32 address);

void storeB(uint32 address, uint8 data);
void storeW(uint32 address, uint16 data);
void storeL(uint32 address, uint32 data);

void SetFRM(void);
void RecacheFRM(void);

//=============================================================================
#endif
