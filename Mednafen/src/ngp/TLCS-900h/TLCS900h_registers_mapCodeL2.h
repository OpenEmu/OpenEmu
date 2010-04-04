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

	TLCS900h_registers_mapCodeL2.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

	//BANK 0
	&(gprBank[0][0]), &(gprBank[0][1]),	&(gprBank[0][2]), &(gprBank[0][3]),

	//BANK 1
	&(gprBank[1][0]), &(gprBank[1][1]),	&(gprBank[1][2]), &(gprBank[1][3]),
	
	//BANK 2
	&(gprBank[2][0]), &(gprBank[2][1]),	&(gprBank[2][2]), &(gprBank[2][3]),

	//BANK 3
	&(gprBank[3][0]), &(gprBank[3][1]),	&(gprBank[3][2]), &(gprBank[3][3]),

	&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,
	&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,
	&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,
	&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,&rErr,
	&rErr,&rErr,&rErr,&rErr,

	//Previous Bank
	&(gprBank[1][0]), &(gprBank[1][1]),	&(gprBank[1][2]), &(gprBank[1][3]),

	//Current Bank
	&(gprBank[2][0]), &(gprBank[2][1]), &(gprBank[2][2]), &(gprBank[2][3]),

	&(gpr[0]), &(gpr[1]), &(gpr[2]), &(gpr[3])

//=============================================================================
