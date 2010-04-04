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

	interrupt.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

23 JUL 2002 - neopop_uk
=======================================
- Simplified the timing of Horizontal and Vertical interrupts, and of the
	retreival of delayed settings.

25 JUL 2002 - neopop_uk
=======================================
- Added DMA update calls for timers 0,1,2,3
- Added timer 0-1 and 2-3 chain modes.
- Fixed H/V timing for some games.
- Added interrupt disabling for H/V ints. ("Neo Turf Masters")

26 JUL 2002 - neopop_uk
=======================================
- The h_int timer is now moved back by the cycle period, rather than
	being reset. This is to give it fractional accuracy.
- Simplified the order of things (perhaps).
- Boiled the H/V problem down so that it hangs on one variable: gfx_hack.
	This isn't great, but it'll have to do for now...
 
01 AUG 2002 - neopop_uk
=======================================
- Added support for frameskipping.

03 AUG 2002 - neopop_uk
=======================================
- Added hack for Timer 2 in T1 mode to run at double time - fixes DAC.

15 AUG 2002 - neopop_uk
=======================================
- Split the timer code into two, the first part does timer 0,1 and H-INT
	the other part does Timers 2,3

22 AUG 2002 - neopop_uk
=======================================
- Added correct DMA chaining behaviour.

30 AUG 2002 - neopop_uk
=======================================
- Removed frameskipping.
- Added comms. read interrupt checking every scanline.

06 SEP 2002 - neopop_uk
=======================================
- Added better emulation of comms system memory at ram[0x50].

09 SEP 2002 - neopop_uk
=======================================
- Unified the timers again.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "TLCS900h_registers.h"
#include "mem.h"
#include "gfx.h"
#include "interrupt.h"
#include "TLCS900h_interpret.h"
#include "Z80_interface.h"
#include "dma.h"

//=============================================================================

_u32 timer_hint;
_u32 timer_clock0, timer_clock1, timer_clock2, timer_clock3;
_u8 timer[4];	//Up-counters

BOOL gfx_hack = FALSE;

//=============================================================================

static BOOL h_int = FALSE, timer0, timer2;

//=============================================================================

void interrupt(_u8 index)
{
	push32(pc);
	push16(sr);

	//Up the IFF
	if (((sr & 0x7000) >> 12) < 7)
		setStatusIFF(((sr & 0x7000) >> 12) + 1);

	//Access the interrupt vector table to find the jump destination
	pc = le32toh(*(_u32*)(ram + 0x6FB8 + (index * 4)));

#ifdef NEOPOP_DEBUG
//	if (index != 5 && index != 7 && index != 10)
//	system_debug_message("interrupt %d: pc -> %06X", index, pc);
#endif
}

//=============================================================================

static __inline void gfx_hint(void)
{
	//H_Int / Delayed settings
	if ((ram[0x8009] < SCREEN_HEIGHT-1 || ram[0x8009] == 198))
	{
		gfx_delayed_settings();	//Get delayed settings

		//Allowed?
		if (ram[0x8000] & 0x40)
			h_int = 1;
	}
}

static __inline void gfx_draw(void)
{
	if (frameskip_count == 0)
	{
		//Draw the scanline
		if (ram[0x8009] < SCREEN_HEIGHT)
		{
			if (ram[0x6F95] == 0x10)	gfx_draw_scanline_colour();
			else						gfx_draw_scanline_mono();
		}
	}
}

void updateTimers(_u8 cputicks)
{
	//increment H-INT timer
	timer_hint += cputicks;

	//=======================

	//End of scanline / Start of Next one
	if (timer_hint >= TIMER_HINT_RATE)
	{
		_u8 data;

		// ============= END OF CURRENT SCANLINE =============

		if (gfx_hack)	{	gfx_draw(); gfx_hint(); }
		else			{	gfx_hint();	gfx_draw(); }

		// ============= START OF NEXT SCANLINE =============

		ram[0x8009]++;	//Next scanline
		timer_hint = 0;	//Start of next scanline

		//Comms. Read interrupt
		if ((ram[0xB2] & 1) == 0 && system_comms_poll(&data) && 
			(statusIFF() <= (ram[0x77] & 7)))
		{
			ram[0x50] = data;
			interrupt(12); 
			if (ram[0x007C] == 0x19)		DMA_update(0);
			else { if (ram[0x007D] == 0x19)	DMA_update(1);
			else { if (ram[0x007E] == 0x19)	DMA_update(2);
			else { if (ram[0x007F] == 0x19)	DMA_update(3);	}}}
		}

		//V_Int?
		if (ram[0x8009] == SCREEN_HEIGHT)
		{
			//Frameskip
			frameskip_count = (frameskip_count + 1) % system_frameskip_key;

			ram[0x8010] = 0x40;	//Character Over / Vblank Status
			system_VBL();	//Update the screen

			//Vertical Interrupt? (Confirmed IRQ Level)
			if (statusIFF() <= 4 && (ram[0x8000] & 0x80))
			{
				interrupt(5); // VBL

				if (ram[0x007C] == 0x0B)		DMA_update(0);
				else { if (ram[0x007D] == 0x0B)	DMA_update(1);
				else { if (ram[0x007E] == 0x0B)	DMA_update(2);
				else { if (ram[0x007F] == 0x0B)	DMA_update(3);	}}}
			}
		}

		//End of V_Int
		if (ram[0x8009] == 198 + 1)	//Last scanline + 1
		{
			ram[0x8009] = 0;
			ram[0x8010] = 0;	//Character Over / Vblank Status
		}
	}

	//=======================

	//Tick the Clock Generator
	timer_clock0 += cputicks;
	timer_clock1 += cputicks;
	
	timer0 = FALSE;	//Clear the timer0 tick, for timer1 chain mode.

	//=======================

	//Run Timer 0 (TRUN)?
	if ((ram[0x20] & 0x01))
	{
		//T01MOD
		switch(ram[0x24] & 0x03)
		{
		case 0:	if (h_int)	//Horizontal interrupt trigger
				{
					timer[0]++;
					timer_clock0 = 0;
					h_int = FALSE;	// Stop h_int remaining active
				}
				break;

		case 1:	if (timer_clock0 >= TIMER_T1_RATE)
				{
					timer[0]++;
					timer_clock0 = 0;
				}
				break;

		case 2:	if (timer_clock0 >= TIMER_T4_RATE)
				{
					timer[0]++;
					timer_clock0 = 0;
				}
				break;

		case 3:	if (timer_clock0 >= TIMER_T16_RATE)
				{
					timer[0]++;
					timer_clock0 = 0;
				}
				break;
		}


		//Threshold check
		if (ram[0x22] && timer[0] >= ram[0x22])
		{
			timer[0] = 0;
			timer0 = TRUE;

			if (statusIFF() <= (ram[0x73] & 0x7))
				interrupt(7); // Timer 0 Int.

			if (ram[0x007C] == 0x10)		DMA_update(0);
			else { if (ram[0x007D] == 0x10)	DMA_update(1);
			else { if (ram[0x007E] == 0x10)	DMA_update(2);
			else { if (ram[0x007F] == 0x10)	DMA_update(3);	}}}
		}
	}

	//=======================

	//Run Timer 1 (TRUN)?
	if ((ram[0x20] & 0x02))
	{
		//T23MOD
		switch((ram[0x24] & 0x0C) >> 2)
		{
		case 0:	if (timer0)	//Timer 0 chain mode.
				{
					timer[1]++;
					timer_clock1 = 0;
				}
				break;

		case 1:	if (timer_clock1 >= TIMER_T1_RATE)
				{
					timer[1]++;
					timer_clock1 = 0;
				}
				break;

		case 2:	if (timer_clock1 >= TIMER_T16_RATE)
				{
					timer[1]++;
					timer_clock1 = 0;
				}
				break;

		case 3:	if (timer_clock1 >= TIMER_T256_RATE)
				{
					timer[1]++;
					timer_clock1 = 0;
				}
				break;
		}

		//Threshold check
		if (ram[0x23] && timer[1] >= ram[0x23])
		{
			timer[1] = 0;

			if (statusIFF() <= ((ram[0x73] & 0x70) >> 4))
				interrupt(8); // Timer 1 Int.

			if (ram[0x007C] == 0x11)		DMA_update(0);
			else { if (ram[0x007D] == 0x11)	DMA_update(1);
			else { if (ram[0x007E] == 0x11)	DMA_update(2);
			else { if (ram[0x007F] == 0x11)	DMA_update(3);	}}}
		}
	}

	//=======================

	//Tick the Clock Generator
	timer_clock2 += cputicks;
	timer_clock3 += cputicks;

	timer2 = FALSE;	//Clear the timer2 tick, for timer3 chain mode.

	//=======================
	
	//Run Timer 2 (TRUN)?
	if ((ram[0x20] & 0x04))
	{
		//T23MOD
		switch(ram[0x28] & 0x03)
		{
		case 0:	// -
				break;

		case 1:	if (timer_clock2 >= 56/*TIMER_T1_RATE*/)	//HACK - Fixes DAC
				{
					timer[2]++;
					timer_clock2 = 0;
				}
				break;

		case 2:	if (timer_clock2 >= TIMER_T4_RATE)
				{
					timer[2]++;
					timer_clock2 = 0;
				}
				break;

		case 3:	if (timer_clock2 >= TIMER_T16_RATE)
				{
					timer[2]++;
					timer_clock2 = 0;
				}
				break;
		}

		//Threshold check
		if (ram[0x26] && timer[2] >= ram[0x26])
		{
			timer[2] = 0;
			timer2 = TRUE;

			if (statusIFF() <= ((ram[0x74] & 0x07)))
				interrupt(9);	// Timer 2 Int.

			if (ram[0x007C] == 0x12)		DMA_update(0);
			else { if (ram[0x007D] == 0x12)	DMA_update(1);
			else { if (ram[0x007E] == 0x12)	DMA_update(2);
			else { if (ram[0x007F] == 0x12)	DMA_update(3);	}}}
		}
	}

	//=======================

	//Run Timer 3 (TRUN)?
	if ((ram[0x20] & 0x08))
	{
		//T23MOD
		switch((ram[0x28] & 0x0C) >> 2)
		{
		case 0:	if (timer2)	//Timer 2 chain mode.
				{
					timer[3]++;
					timer_clock3 = 0;
				}
				break;

		case 1:	if (timer_clock3 >= TIMER_T1_RATE)
				{
					timer[3]++;
					timer_clock3 = 0;
				}
				break;

		case 2:	if (timer_clock3 >= TIMER_T16_RATE)
				{
					timer[3]++;
					timer_clock3 = 0;
				}
				break;

		case 3:	if (timer_clock3 >= TIMER_T256_RATE)
				{
					timer[3]++;
					timer_clock3 = 0;
				}
				break;
		}

		//Threshold check
		if (ram[0x27] && timer[3] >= ram[0x27])
		{
			timer[3] = 0;

			Z80_irq();

			if (statusIFF() <= ((ram[0x74] & 0x70) >> 4))
				interrupt(10); // Timer 3 Int.

			if (ram[0x007C] == 0x13)		DMA_update(0);
			else { if (ram[0x007D] == 0x13)	DMA_update(1);
			else { if (ram[0x007E] == 0x13)	DMA_update(2);
			else { if (ram[0x007F] == 0x13)	DMA_update(3);	}}}
		}
	}

	//=======================
}

//=============================================================================

void reset_timers(void)
{
	timer_hint = 0;

	timer[0] = 0;
	timer[1] = 0;
	timer[2] = 0;
	timer[3] = 0;
	
	timer_clock0 = 0;
	timer_clock1 = 0;
	timer_clock2 = 0;
	timer_clock3 = 0;
}

//=============================================================================

