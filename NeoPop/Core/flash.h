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

	flash.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

26 JUL 2002 - neopop_uk
=======================================
- Removed the eeprom emulation functions and data.
- Renamed this file "flash.h"

10 AUG 2002 - neopop_uk
=======================================
- Added flash_commit function to do the writing.

//---------------------------------------------------------------------------
*/

#ifndef __FLASH__
#define __FLASH__
//=============================================================================

void flash_read(void);

//Marks flash blocks for saving.
void flash_write(_u32 start_address, _u16 length);

//Stores the flash data
void flash_commit(void);

//=============================================================================
#endif
