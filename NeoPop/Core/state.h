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

	state.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- Added all of the necessary #includes to this file.

25 JUL 2002 - neopop_uk
=======================================
- Added comments to say that new state fields should be added to
	the end of the structure.

28 JUL 2002 - neopop_uk
=======================================
- Renamed the state struct to NEOPOPSTATE0050 so that multiple versions
can exist - should a new one be required.
- Removed 'state_valid_id' as it's not used any more.
 
15 AUG 2002 - neopop_uk
=======================================
- Changed the saved 'halt' bool into 'int reserved1'.
- Changed 'int reserved1' into 'BOOL eepromStatusEnable'
 
18 AUG 2002 - neopop_uk
=======================================
- Moved state_store/state_restore prototypes to NeoPop.h

//---------------------------------------------------------------------------
*/

#ifndef __STATE__
#define __STATE__
//=============================================================================

#include "Z80_interface.h"			//For Z80_regs
#include "sound.h"					//For SoundChip

//-----------------------------------------------------------------------------
// State Definitions:
//-----------------------------------------------------------------------------

typedef struct
{
	//Save State Id
	_u16 valid_state_id;		// = 0x0050

	//Memory
	_u8 ram[0xC000];	

	//TLCS-900h Registers
	_u32 pc;
	_u16 sr;
	_u8 f_dash;
	_u32 gprBank[4][4], gpr[4];
	
	BOOL eepromStatusEnable;

	//Z80 Registers
	Z80 Z80_regs;

	//Timers
	_u32 timer_hint;
	_u8 timer[4];	//Up-counters
	_u32 timer_clock0, timer_clock1, timer_clock2, timer_clock3;

	//Sound Chips
	SoundChip toneChip;
	SoundChip noiseChip;

	//DMA
	_u32 dmaS[4], dmaD[4];
	_u16 dmaC[4];
	_u8 dmaM[4];

	//Rom Description
	RomHeader header;
}
NEOPOPSTATE0050;

//=============================================================================
#endif

