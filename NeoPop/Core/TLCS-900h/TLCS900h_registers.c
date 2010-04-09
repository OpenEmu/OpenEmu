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

	TLCS900h_registers.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

10 AUG 2002 - neopop_uk
=======================================
- Moved default PC setup to the 'reset_registers' function.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "TLCS900h_registers.h"

#ifdef MSB_FIRST
#define BYTE0	3
#define BYTE1	2
#define BYTE2	1
#define BYTE3	0
#define WORD0	2
#define WORD1	0
#else
#define BYTE0	0
#define BYTE1	1
#define BYTE2	2
#define BYTE3	3
#define WORD0	0
#define WORD1	2
#endif

//=============================================================================

_u32 pc, gprBank[4][4], gpr[4];
_u16 sr;
_u8 f_dash;

//=============================================================================

//Bank Data
_u8* gprMapB[4][8] =
{
	#include "TLCS900h_registers_mapB.h"
};

_u16* gprMapW[4][8] =
{
	#include "TLCS900h_registers_mapW.h"
};

_u32* gprMapL[4][8] =
{
	#include "TLCS900h_registers_mapL.h"
};

//=============================================================================

_u32 rErr;

_u8* regCodeMapB[4][256] =
{
	{
		#include "TLCS900h_registers_mapCodeB0.h"
	},

	{
		#include "TLCS900h_registers_mapCodeB1.h"
	},

	{
		#include "TLCS900h_registers_mapCodeB2.h"
	},

	{
		#include "TLCS900h_registers_mapCodeB3.h"
	}
};

_u16* regCodeMapW[4][128] =
{
	{
		#include "TLCS900h_registers_mapCodeW0.h"
	},

	{
		#include "TLCS900h_registers_mapCodeW1.h"
	},

	{
		#include "TLCS900h_registers_mapCodeW2.h"
	},

	{
		#include "TLCS900h_registers_mapCodeW3.h"
	}
};

_u32* regCodeMapL[4][64] =
{
	{
		#include "TLCS900h_registers_mapCodeL0.h"
	},

	{
		#include "TLCS900h_registers_mapCodeL1.h"
	},

	{
		#include "TLCS900h_registers_mapCodeL2.h"
	},

	{
		#include "TLCS900h_registers_mapCodeL3.h"
	}
};

//=============================================================================

_u8 statusIFF(void)	
{
	_u8 iff = (sr & 0x7000) >> 12;

	if (iff == 1)
		return 0;
	else
		return iff;
}

void setStatusIFF(_u8 iff)
{
	sr = (sr & 0x8FFF) | ((iff & 0x7) << 12);
}

//=============================================================================

_u8 statusRFP;

void setStatusRFP(_u8 rfp)
{
	sr = (sr & 0xF8FF) | ((rfp & 0x3) << 8);
	changedSP();
}

void changedSP(void)
{
	//Store global RFP for optimisation. 
	statusRFP = ((sr & 0x300) >> 8);
}

//=============================================================================

void reset_registers(void)
{
	memset(gprBank, 0, sizeof(gprBank));
	memset(gpr, 0, sizeof(gpr));

	if (rom.data)
		pc = le32toh(rom_header->startPC) & 0xFFFFFF;
	else
		pc = 0xFFFFFE;

	sr = 0xF800;		// = %11111000???????? (?) are undefined in the manual)
	changedSP();
	
	f_dash = 00;

	rErr = RERR_VALUE;

	REGXSP = 0x00006C00; //Confirmed from BIOS, 
						//immediately changes value from default of 0x100
}

//=============================================================================
