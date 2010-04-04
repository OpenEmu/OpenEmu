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

#ifndef __FLASH__
#define __FLASH__
//=============================================================================

void flash_read(void);

//Marks flash blocks for saving.
void flash_write(uint32 start_address, uint16 length);

//Stores the flash data
void flash_commit(void);


int FLASH_StateAction(StateMem *sm, int load, int data_only);

//=============================================================================
#endif
