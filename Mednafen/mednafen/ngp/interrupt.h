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

#ifndef __INTERRUPT__
#define __INTERRUPT__
//=============================================================================

void interrupt(uint8 index);

#define TIMER_HINT_RATE		515		//CPU Ticks between horizontal interrupts

#define TIMER_BASE_RATE		32 //1		//ticks

#define TIMER_T1_RATE		(8 * TIMER_BASE_RATE)
#define TIMER_T4_RATE		(32 * TIMER_BASE_RATE)
#define TIMER_T16_RATE		(128 * TIMER_BASE_RATE)
#define TIMER_T256_RATE		(2048 * TIMER_BASE_RATE)

void reset_timers(void);
void reset_int(void);

//Call this after each instruction
bool updateTimers(MDFN_Surface *surface, uint8 cputicks);

//H-INT Timer
extern uint32 timer_hint;

void timer_write8(uint32 address, uint8 data);
uint8 timer_read8(uint32 address);


// Set this value to fix problems with glitching extra lines.
extern bool gfx_hack;


void int_write8(uint32 address, uint8 data);
uint8 int_read8(uint32 address);
void int_check_pending(void);
void TestIntHDMA(int bios_num, int vec_num);

int int_timer_StateAction(StateMem *sm, int load, int data_only);

//=============================================================================
#endif




