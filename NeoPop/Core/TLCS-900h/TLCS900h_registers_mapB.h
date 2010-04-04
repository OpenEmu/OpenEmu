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

	TLCS900h_registers_mapB.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

	//Bank 0
	{
		(_u8*)&(gprBank[0][0]) + BYTE1,
		(_u8*)&(gprBank[0][0]) + BYTE0,
		(_u8*)&(gprBank[0][1]) + BYTE1,
		(_u8*)&(gprBank[0][1]) + BYTE0,
		(_u8*)&(gprBank[0][2]) + BYTE1,
		(_u8*)&(gprBank[0][2]) + BYTE0,
		(_u8*)&(gprBank[0][3]) + BYTE1,
		(_u8*)&(gprBank[0][3]) + BYTE0,
	},

	//Bank 1
	{
		(_u8*)&(gprBank[1][0]) + BYTE1,
		(_u8*)&(gprBank[1][0]) + BYTE0,
		(_u8*)&(gprBank[1][1]) + BYTE1,
		(_u8*)&(gprBank[1][1]) + BYTE0,
		(_u8*)&(gprBank[1][2]) + BYTE1,
		(_u8*)&(gprBank[1][2]) + BYTE0,
		(_u8*)&(gprBank[1][3]) + BYTE1,
		(_u8*)&(gprBank[1][3]) + BYTE0,
	},
	
	//Bank 2
	{
		(_u8*)&(gprBank[2][0]) + BYTE1,
		(_u8*)&(gprBank[2][0]) + BYTE0,
		(_u8*)&(gprBank[2][1]) + BYTE1,
		(_u8*)&(gprBank[2][1]) + BYTE0,
		(_u8*)&(gprBank[2][2]) + BYTE1,
		(_u8*)&(gprBank[2][2]) + BYTE0,
		(_u8*)&(gprBank[2][3]) + BYTE1,
		(_u8*)&(gprBank[2][3]) + BYTE0,
	},

	//Bank 3
	{
		(_u8*)&(gprBank[3][0]) + BYTE1,
		(_u8*)&(gprBank[3][0]) + BYTE0,
		(_u8*)&(gprBank[3][1]) + BYTE1,
		(_u8*)&(gprBank[3][1]) + BYTE0,
		(_u8*)&(gprBank[3][2]) + BYTE1,
		(_u8*)&(gprBank[3][2]) + BYTE0,
		(_u8*)&(gprBank[3][3]) + BYTE1,
		(_u8*)&(gprBank[3][3]) + BYTE0,
	}

//=============================================================================

