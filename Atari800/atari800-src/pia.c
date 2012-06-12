/*
 * pia.c - PIA chip emulation
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2008 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "config.h"

#include "atari.h"
#include "cpu.h"
#include "memory.h"
#include "pia.h"
#include "sio.h"
#ifdef XEP80_EMULATION
#include "xep80.h"
#endif
#ifndef BASIC
#include "input.h"
#include "statesav.h"
#endif

UBYTE PIA_PACTL;
UBYTE PIA_PBCTL;
UBYTE PIA_PORTA;
UBYTE PIA_PORTB;
UBYTE PIA_PORT_input[2];

UBYTE PIA_PORTA_mask;
UBYTE PIA_PORTB_mask;

int PIA_Initialise(int *argc, char *argv[])
{
	PIA_PACTL = 0x3f;
	PIA_PBCTL = 0x3f;
	PIA_PORTA = 0xff;
	PIA_PORTB = 0xff;
	PIA_PORTA_mask = 0xff;
	PIA_PORTB_mask = 0xff;
	PIA_PORT_input[0] = 0xff;
	PIA_PORT_input[1] = 0xff;

	return TRUE;
}

void PIA_Reset(void)
{
	PIA_PORTA = 0xff;
	if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
		MEMORY_HandlePORTB(0xff, (UBYTE) (PIA_PORTB | PIA_PORTB_mask));
	}
	PIA_PORTB = 0xff;
}

UBYTE PIA_GetByte(UWORD addr)
{
	switch (addr & 0x03) {
	case PIA_OFFSET_PACTL:
		return PIA_PACTL & 0x3f;
	case PIA_OFFSET_PBCTL:
		return PIA_PBCTL & 0x3f;
	case PIA_OFFSET_PORTA:
		if ((PIA_PACTL & 0x04) == 0) {
			/* direction register */
			return ~PIA_PORTA_mask;
		}
		else {
			/* port state */
#ifdef XEP80_EMULATION
			if (XEP80_enabled) {
				return(XEP80_GetBit() & PIA_PORT_input[0] & (PIA_PORTA | PIA_PORTA_mask));
			}
#endif /* XEP80_EMULATION */
			return PIA_PORT_input[0] & (PIA_PORTA | PIA_PORTA_mask);
		}
	case PIA_OFFSET_PORTB:
		if ((PIA_PBCTL & 0x04) == 0) {
			/* direction register */
			return ~PIA_PORTB_mask;
		}
		else {
			/* port state */
			if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
				return PIA_PORTB | PIA_PORTB_mask;
			}
			else {
				return PIA_PORT_input[1] & (PIA_PORTB | PIA_PORTB_mask);
			}
		}
	}
	/* for stupid compilers */
	return 0xff;
}

void PIA_PutByte(UWORD addr, UBYTE byte)
{
	switch (addr & 0x03) {
	case PIA_OFFSET_PACTL:
                /* This code is part of the cassette emulation */
		/* The motor status has changed */
		SIO_TapeMotor(byte & 0x08 ? 0 : 1);
	
		PIA_PACTL = byte;
		break;
	case PIA_OFFSET_PBCTL:
		/* This code is part of the serial I/O emulation */
		if ((PIA_PBCTL ^ byte) & 0x08) {
			/* The command line status has changed */
			SIO_SwitchCommandFrame(byte & 0x08 ? 0 : 1);
		}
		PIA_PBCTL = byte;
		break;
	case PIA_OFFSET_PORTA:
		if ((PIA_PACTL & 0x04) == 0) {
			/* set direction register */
 			PIA_PORTA_mask = ~byte;
		}
		else {
			/* set output register */
#ifdef XEP80_EMULATION
			if (XEP80_enabled && (~PIA_PORTA_mask & 0x11)) {
				XEP80_PutBit(byte);
			}
#endif /* XEP80_EMULATION */
			PIA_PORTA = byte;		/* change from thor */
		}
#ifndef BASIC
		INPUT_SelectMultiJoy((PIA_PORTA | PIA_PORTA_mask) >> 4);
#endif
		break;
	case PIA_OFFSET_PORTB:
		if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
			if ((PIA_PBCTL & 0x04) == 0) {
				/* direction register */
				MEMORY_HandlePORTB((UBYTE) (PIA_PORTB | ~byte), (UBYTE) (PIA_PORTB | PIA_PORTB_mask));
				PIA_PORTB_mask = ~byte;
			}
			else {
				/* output register */
				MEMORY_HandlePORTB((UBYTE) (byte | PIA_PORTB_mask), (UBYTE) (PIA_PORTB | PIA_PORTB_mask));
				PIA_PORTB = byte;
			}
		}
		else {
			if ((PIA_PBCTL & 0x04) == 0) {
				/* direction register */
				PIA_PORTB_mask = ~byte;
			}
			else {
				/* output register */
				PIA_PORTB = byte;
			}
		}
		break;
	}
}

#ifndef BASIC

void PIA_StateSave(void)
{
	int Ram256 = 0;
	if (MEMORY_ram_size == MEMORY_RAM_320_RAMBO)
		Ram256 = 1;
	else if (MEMORY_ram_size == MEMORY_RAM_320_COMPY_SHOP)
		Ram256 = 2;

	StateSav_SaveUBYTE( &PIA_PACTL, 1 );
	StateSav_SaveUBYTE( &PIA_PBCTL, 1 );
	StateSav_SaveUBYTE( &PIA_PORTA, 1 );
	StateSav_SaveUBYTE( &PIA_PORTB, 1 );

	StateSav_SaveINT( &MEMORY_xe_bank, 1 );
	StateSav_SaveINT( &MEMORY_selftest_enabled, 1 );
	StateSav_SaveINT( &Ram256, 1 );

	StateSav_SaveINT( &MEMORY_cartA0BF_enabled, 1 );

	StateSav_SaveUBYTE( &PIA_PORTA_mask, 1 );
	StateSav_SaveUBYTE( &PIA_PORTB_mask, 1 );
}

void PIA_StateRead(void)
{
	int Ram256 = 0;

	StateSav_ReadUBYTE( &PIA_PACTL, 1 );
	StateSav_ReadUBYTE( &PIA_PBCTL, 1 );
	StateSav_ReadUBYTE( &PIA_PORTA, 1 );
	StateSav_ReadUBYTE( &PIA_PORTB, 1 );

	StateSav_ReadINT( &MEMORY_xe_bank, 1 );
	StateSav_ReadINT( &MEMORY_selftest_enabled, 1 );
	StateSav_ReadINT( &Ram256, 1 );

	if (Ram256 == 1 && Atari800_machine_type == Atari800_MACHINE_XLXE && MEMORY_ram_size == MEMORY_RAM_320_COMPY_SHOP)
		MEMORY_ram_size = MEMORY_RAM_320_RAMBO;

	StateSav_ReadINT( &MEMORY_cartA0BF_enabled, 1 );

	StateSav_ReadUBYTE( &PIA_PORTA_mask, 1 );
	StateSav_ReadUBYTE( &PIA_PORTB_mask, 1 );
}

#endif /* BASIC */
