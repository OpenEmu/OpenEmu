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

	interrupt.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

26 JUL 2002 - neopop_uk
=======================================
- Boiled the H/V problem down so that it hangs on one variable: gfx_hack.
	This isn't great, but it'll have to do for now...
 
01 AUG 2002 - neopop_uk
=======================================
- Added support for frameskipping.

16 AUG 2002 - neopop_uk
=======================================
- Added prototypes for split interrupt functions.

30 AUG 2002 - neopop_uk
=======================================
- Removed frameskipping.

09 SEP 2002 - neopop_uk
=======================================
- Unified timer update calls.
- Corrected timer speeds.

//---------------------------------------------------------------------------
*/

#ifndef __INTERRUPT__
#define __INTERRUPT__
//=============================================================================

void interrupt(_u8 index);

#define TIMER_HINT_RATE		515		//CPU Ticks between horizontal interrupts

#define TIMER_BASE_RATE		240		//ticks

#define TIMER_T1_RATE		(1 * TIMER_BASE_RATE)
#define TIMER_T4_RATE		(4 * TIMER_BASE_RATE)
#define TIMER_T16_RATE		(16 * TIMER_BASE_RATE)
#define TIMER_T256_RATE		(256 * TIMER_BASE_RATE)

void reset_timers(void);

//Call this after each instruction
void updateTimers(_u8 cputicks);

//H-INT Timer
extern _u32 timer_hint;
extern _u8 timer[4];	//Up-counters
extern _u32 timer_clock0, timer_clock1, timer_clock2, timer_clock3;

// Set this value to fix problems with glitching extra lines.
extern BOOL gfx_hack;

//=============================================================================
#endif




