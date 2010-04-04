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

	state.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

28 JUL 2002 - neopop_uk
=======================================
- Converted the load and save functions to use the new state name.
- Move the version "0050" state loader into it's own function, this makes
	it much easier to add new loaders in the future.

15 AUG 2002 - neopop_uk
=======================================
- Removed storing the 'halt' state as the variable no longer exists.
- Changed 'int reserved1' into 'BOOL eepromStatusEnable'

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "state.h"
#include "TLCS900h_registers.h"
#include "interrupt.h"
#include "dma.h"
#include "mem.h"

//=============================================================================

static void read_state_0050(char* filename);

//-----------------------------------------------------------------------------
// state_restore()
//-----------------------------------------------------------------------------
void state_restore(char* filename)
{
	_u16 version;

	if (system_io_state_read(filename, (_u8*)&version, sizeof(_u16)))
	{
		switch(version)
		{
		case 0x0050:	read_state_0050(filename);	break;

		default:
			system_message(system_get_string(IDS_BADSTATE));
			return;
		}

#ifdef NEOPOP_DEBUG
		system_debug_message("Restoring State ...");
		system_debug_refresh();
#endif
	}
}

//=============================================================================

//-----------------------------------------------------------------------------
// state_store()
//-----------------------------------------------------------------------------
void state_store(char* filename)
{
	NEOPOPSTATE0050	state;
	int i,j;

	//Build a state description
	state.valid_state_id = 0x0050;
	memcpy(&state.header, rom_header, sizeof(RomHeader));

	state.eepromStatusEnable = eepromStatusEnable;

	//TLCS-900h Registers
	state.pc = pc;
	state.sr = sr;
	state.f_dash = f_dash;

	for (i = 0; i < 4; i++)
	{
		state.gpr[i] = gpr[i];
		for (j = 0; j < 4; j++)
			state.gprBank[i][j] = gprBank[i][j];
	}

	//Z80 Registers
	memcpy(&state.Z80_regs, &Z80_regs, sizeof(Z80));

	//Sound Chips
	memcpy(&state.toneChip, &toneChip, sizeof(SoundChip));
	memcpy(&state.noiseChip, &noiseChip, sizeof(SoundChip));

	//Memory
	memcpy(&state.ram, ram, 0xC000);

	//Timers
	state.timer_hint = timer_hint;

	for (i = 0; i < 4; i++)	//Up-counters
		state.timer[i] = timer[i];

	state.timer_clock0 = timer_clock0;
	state.timer_clock1 = timer_clock1;
	state.timer_clock2 = timer_clock2;
	state.timer_clock3 = timer_clock3;

	//DMA
	for (i = 0; i < 4; i++)
	{
		state.dmaS[i] = dmaS[i];
		state.dmaD[i] = dmaD[i];
		state.dmaC[i] = dmaC[i];
		state.dmaM[i] = dmaM[i];
	}

#ifdef NEOPOP_DEBUG
	system_debug_message("Saving State ...");
#endif

	system_io_state_write(filename, (_u8*)&state, sizeof(NEOPOPSTATE0050));
}

//=============================================================================

static void read_state_0050(char* filename)
{
	NEOPOPSTATE0050	state;
	int i,j;

	if (system_io_state_read(filename, (_u8*)&state, sizeof(NEOPOPSTATE0050)))
	{
		//Verify correct rom...
		if (memcmp(rom_header, &state.header, sizeof(RomHeader)) != 0)
		{
			system_message(system_get_string(IDS_WRONGROM));
			return;
		}

		//Apply state description
		reset();

		eepromStatusEnable = state.eepromStatusEnable;

		//TLCS-900h Registers
		pc = state.pc;
		sr = state.sr;				changedSP();
		f_dash = state.f_dash;

		eepromStatusEnable = state.eepromStatusEnable;

		for (i = 0; i < 4; i++)
		{
			gpr[i] = state.gpr[i];
			for (j = 0; j < 4; j++)
				gprBank[i][j] = state.gprBank[i][j];
		}

		//Timers
		timer_hint = state.timer_hint;

		for (i = 0; i < 4; i++)	//Up-counters
			timer[i] = state.timer[i];

		timer_clock0 = state.timer_clock0;
		timer_clock1 = state.timer_clock1;
		timer_clock2 = state.timer_clock2;
		timer_clock3 = state.timer_clock3;

		//Z80 Registers
		memcpy(&Z80_regs, &state.Z80_regs, sizeof(Z80));

		//Sound Chips
		memcpy(&toneChip, &state.toneChip, sizeof(SoundChip));
		memcpy(&noiseChip, &state.noiseChip, sizeof(SoundChip));

		//DMA
		for (i = 0; i < 4; i++)
		{
			dmaS[i] = state.dmaS[i];
			dmaD[i] = state.dmaD[i];
			dmaC[i] = state.dmaC[i];
			dmaM[i] = state.dmaM[i];
		}

		//Memory
		memcpy(ram, &state.ram, 0xC000);
	}
}

//=============================================================================
