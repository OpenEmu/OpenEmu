/*
 * pokey.c - POKEY sound chip emulation
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
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include "atari.h"
#include "cpu.h"
#include "esc.h"
#include "pia.h"
#include "pokey.h"
#include "gtia.h"
#include "sio.h"
#ifndef BASIC
#include "input.h"
#include "statesav.h"
#endif
#ifdef SOUND
#include "pokeysnd.h"
#endif
#include "antic.h"
#include "cassette.h"
#include "log.h"
#include "input.h"
#include "pbi.h"

#ifdef VOICEBOX
#include "voicebox.h"
#include "votraxsnd.h"
#endif

#ifdef POKEY_UPDATE
void pokey_update(void);
#endif

UBYTE POKEY_KBCODE;
UBYTE POKEY_SERIN;
UBYTE POKEY_IRQST;
UBYTE POKEY_IRQEN;
UBYTE POKEY_SKSTAT;
UBYTE POKEY_SKCTL;
int POKEY_DELAYED_SERIN_IRQ;
int POKEY_DELAYED_SEROUT_IRQ;
int POKEY_DELAYED_XMTDONE_IRQ;

/* structures to hold the 9 pokey control bytes */
UBYTE POKEY_AUDF[4 * POKEY_MAXPOKEYS];	/* AUDFx (D200, D202, D204, D206) */
UBYTE POKEY_AUDC[4 * POKEY_MAXPOKEYS];	/* AUDCx (D201, D203, D205, D207) */
UBYTE POKEY_AUDCTL[POKEY_MAXPOKEYS];	/* AUDCTL (D208) */
int POKEY_DivNIRQ[4], POKEY_DivNMax[4];
int POKEY_Base_mult[POKEY_MAXPOKEYS];		/* selects either 64Khz or 15Khz clock mult */

UBYTE POKEY_POT_input[8] = {228, 228, 228, 228, 228, 228, 228, 228};
static int pot_scanline;

UBYTE POKEY_poly9_lookup[511];
UBYTE POKEY_poly17_lookup[16385];
static ULONG random_scanline_counter;

ULONG POKEY_GetRandomCounter(void)
{
	return random_scanline_counter;
}

void POKEY_SetRandomCounter(ULONG value)
{
	random_scanline_counter = value;
}

UBYTE POKEY_GetByte(UWORD addr)
{
	UBYTE byte = 0xff;

#ifdef STEREO_SOUND
	if (addr & 0x0010 && POKEYSND_stereo_enabled)
		return 0;
#endif
	addr &= 0x0f;
	if (addr < 8) {
		byte = POKEY_POT_input[addr];
		if (byte <= pot_scanline)
			return byte;
		return pot_scanline;
	}
	switch (addr) {
	case POKEY_OFFSET_ALLPOT:
		{
			int i;
			for (i = 0; i < 8; i++)
				if (POKEY_POT_input[i] <= pot_scanline)
					byte &= ~(1 << i);		/* reset bit if pot value known */
		}
		break;
	case POKEY_OFFSET_KBCODE:
		byte = POKEY_KBCODE;
		break;
	case POKEY_OFFSET_RANDOM:
		if ((POKEY_SKCTL & 0x03) != 0) {
			int i = random_scanline_counter + ANTIC_XPOS;
			if (POKEY_AUDCTL[0] & POKEY_POLY9)
				byte = POKEY_poly9_lookup[i % POKEY_POLY9_SIZE];
			else {
				const UBYTE *ptr;
				i %= POKEY_POLY17_SIZE;
				ptr = POKEY_poly17_lookup + (i >> 3);
				i &= 7;
				byte = (UBYTE) ((ptr[0] >> i) + (ptr[1] << (8 - i)));
			}
		}
		break;
	case POKEY_OFFSET_SERIN:
		byte = POKEY_SERIN;
#ifdef DEBUG3
		printf("SERIO: SERIN read, bytevalue %02x\n", POKEY_SERIN);
#endif
#ifdef SERIO_SOUND
		POKEYSND_UpdateSerio(0,byte);
#endif
		break;
	case POKEY_OFFSET_IRQST:
		byte = POKEY_IRQST;
		break;
	case POKEY_OFFSET_SKSTAT:
		byte = POKEY_SKSTAT + (CASSETTE_IOLineStatus() << 4);
#ifdef VOICEBOX
		if (VOICEBOX_enabled) {
			byte = POKEY_SKSTAT + (VOTRAXSND_busy  << 4);
		}
#endif
		break;
	}

	return byte;
}

static void Update_Counter(int chan_mask);

static int POKEY_siocheck(void)
{
	return (((POKEY_AUDF[POKEY_CHAN3] == 0x28 || POKEY_AUDF[POKEY_CHAN3] == 0x10
	        || POKEY_AUDF[POKEY_CHAN3] == 0x08 || POKEY_AUDF[POKEY_CHAN3] == 0x0a)
		&& POKEY_AUDF[POKEY_CHAN4] == 0x00) /* intelligent peripherals speeds */
		|| (POKEY_SKCTL & 0x78) == 0x28) /* cassette save mode */
		&& (POKEY_AUDCTL[0] & 0x28) == 0x28;
}

#ifndef SOUND_GAIN /* sound gain can be pre-defined in the configure/Makefile */
#define SOUND_GAIN 4
#endif

#ifndef SOUND
#define POKEYSND_Update(addr, val, chip, gain)
#endif

void POKEY_PutByte(UWORD addr, UBYTE byte)
{
#ifdef STEREO_SOUND
	addr &= POKEYSND_stereo_enabled ? 0x1f : 0x0f;
#else
	addr &= 0x0f;
#endif
	switch (addr) {
	case POKEY_OFFSET_AUDC1:
		POKEY_AUDC[POKEY_CHAN1] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDC1, byte, 0, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDC2:
		POKEY_AUDC[POKEY_CHAN2] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDC2, byte, 0, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDC3:
		POKEY_AUDC[POKEY_CHAN3] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDC3, byte, 0, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDC4:
		POKEY_AUDC[POKEY_CHAN4] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDC4, byte, 0, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDCTL:
		POKEY_AUDCTL[0] = byte;

		/* determine the base multiplier for the 'div by n' calculations */
		if (byte & POKEY_CLOCK_15)
			POKEY_Base_mult[0] = POKEY_DIV_15;
		else
			POKEY_Base_mult[0] = POKEY_DIV_64;

		Update_Counter((1 << POKEY_CHAN1) | (1 << POKEY_CHAN2) | (1 << POKEY_CHAN3) | (1 << POKEY_CHAN4));
		POKEYSND_Update(POKEY_OFFSET_AUDCTL, byte, 0, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDF1:
		POKEY_AUDF[POKEY_CHAN1] = byte;
		Update_Counter((POKEY_AUDCTL[0] & POKEY_CH1_CH2) ? ((1 << POKEY_CHAN2) | (1 << POKEY_CHAN1)) : (1 << POKEY_CHAN1));
		POKEYSND_Update(POKEY_OFFSET_AUDF1, byte, 0, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDF2:
		POKEY_AUDF[POKEY_CHAN2] = byte;
		Update_Counter(1 << POKEY_CHAN2);
		POKEYSND_Update(POKEY_OFFSET_AUDF2, byte, 0, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDF3:
		POKEY_AUDF[POKEY_CHAN3] = byte;
		Update_Counter((POKEY_AUDCTL[0] & POKEY_CH3_CH4) ? ((1 << POKEY_CHAN4) | (1 << POKEY_CHAN3)) : (1 << POKEY_CHAN3));
		POKEYSND_Update(POKEY_OFFSET_AUDF3, byte, 0, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDF4:
		POKEY_AUDF[POKEY_CHAN4] = byte;
		Update_Counter(1 << POKEY_CHAN4);
		POKEYSND_Update(POKEY_OFFSET_AUDF4, byte, 0, SOUND_GAIN);
		break;
	case POKEY_OFFSET_IRQEN:
		POKEY_IRQEN = byte;
#ifdef DEBUG1
		printf("WR: IRQEN = %x, PC = %x\n", POKEY_IRQEN, PC);
#endif
		POKEY_IRQST |= ~byte & 0xf7;	/* Reset disabled IRQs except XMTDONE */
		if (POKEY_IRQEN & 0x20) {
			SLONG delay;
			delay = CASSETTE_GetInputIRQDelay();
			if (delay > 0)
				POKEY_DELAYED_SERIN_IRQ = delay;
		}
		if ((~POKEY_IRQST & POKEY_IRQEN) == 0 && PBI_IRQ == 0)
			CPU_IRQ = 0;
		else
			CPU_GenerateIRQ();
		break;
	case POKEY_OFFSET_SKRES:
		POKEY_SKSTAT |= 0xe0;
		break;
	case POKEY_OFFSET_POTGO:
		if (!(POKEY_SKCTL & 4))
			pot_scanline = 0;	/* slow pot mode */
		break;
	case POKEY_OFFSET_SEROUT:
#ifdef VOICEBOX
		VOICEBOX_SEROUTPutByte(byte);
#endif
		if ((POKEY_SKCTL & 0x70) == 0x20 && POKEY_siocheck())
			SIO_PutByte(byte);
		/* check if cassette 2-tone mode has been enabled */
		if ((POKEY_SKCTL & 0x08) == 0x00) {
			/* intelligent device */
			POKEY_DELAYED_SEROUT_IRQ = SIO_SEROUT_INTERVAL;
			POKEY_IRQST |= 0x08;
			POKEY_DELAYED_XMTDONE_IRQ = SIO_XMTDONE_INTERVAL;
		}
		else {
			/* cassette */
			/* some savers patch the cassette baud rate, so we evaluate it here */
			/* scanlines per second*10 bit*audiofrequency/(1.79 MHz/2) */
                        POKEY_DELAYED_SEROUT_IRQ = 312*50*10*(POKEY_AUDF[POKEY_CHAN3] + POKEY_AUDF[POKEY_CHAN4]*0x100)/895000;
			/* safety check */
			if (POKEY_DELAYED_SEROUT_IRQ >= 3) {
                        	POKEY_IRQST |= 0x08;
                        	POKEY_DELAYED_XMTDONE_IRQ = 2*POKEY_DELAYED_SEROUT_IRQ - 2;
			}
			else {
				POKEY_DELAYED_SEROUT_IRQ = 0;
				POKEY_DELAYED_XMTDONE_IRQ = 0;
			}
		};
#ifdef SERIO_SOUND
		POKEYSND_UpdateSerio(1, byte);
#endif
		break;
	case POKEY_OFFSET_STIMER:
		POKEY_DivNIRQ[POKEY_CHAN1] = POKEY_DivNMax[POKEY_CHAN1];
		POKEY_DivNIRQ[POKEY_CHAN2] = POKEY_DivNMax[POKEY_CHAN2];
		POKEY_DivNIRQ[POKEY_CHAN4] = POKEY_DivNMax[POKEY_CHAN4];
		POKEYSND_Update(POKEY_OFFSET_STIMER, byte, 0, SOUND_GAIN);
#ifdef DEBUG1
		printf("WR: STIMER = %x\n", byte);
#endif
		break;
	case POKEY_OFFSET_SKCTL:
#ifdef VOICEBOX
		VOICEBOX_SKCTLPutByte(byte);
#endif
		POKEY_SKCTL = byte;
		POKEYSND_Update(POKEY_OFFSET_SKCTL, byte, 0, SOUND_GAIN);
		if (byte & 4)
			pot_scanline = 228;	/* fast pot mode - return results immediately */
		break;
#ifdef STEREO_SOUND
	case POKEY_OFFSET_AUDC1 + POKEY_OFFSET_POKEY2:
		POKEY_AUDC[POKEY_CHAN1 + POKEY_CHIP2] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDC1, byte, 1, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDC2 + POKEY_OFFSET_POKEY2:
		POKEY_AUDC[POKEY_CHAN2 + POKEY_CHIP2] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDC2, byte, 1, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDC3 + POKEY_OFFSET_POKEY2:
		POKEY_AUDC[POKEY_CHAN3 + POKEY_CHIP2] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDC3, byte, 1, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDC4 + POKEY_OFFSET_POKEY2:
		POKEY_AUDC[POKEY_CHAN4 + POKEY_CHIP2] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDC4, byte, 1, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDCTL + POKEY_OFFSET_POKEY2:
		POKEY_AUDCTL[1] = byte;
		/* determine the base multiplier for the 'div by n' calculations */
		if (byte & POKEY_CLOCK_15)
			POKEY_Base_mult[1] = POKEY_DIV_15;
		else
			POKEY_Base_mult[1] = POKEY_DIV_64;

		POKEYSND_Update(POKEY_OFFSET_AUDCTL, byte, 1, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDF1 + POKEY_OFFSET_POKEY2:
		POKEY_AUDF[POKEY_CHAN1 + POKEY_CHIP2] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDF1, byte, 1, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDF2 + POKEY_OFFSET_POKEY2:
		POKEY_AUDF[POKEY_CHAN2 + POKEY_CHIP2] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDF2, byte, 1, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDF3 + POKEY_OFFSET_POKEY2:
		POKEY_AUDF[POKEY_CHAN3 + POKEY_CHIP2] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDF3, byte, 1, SOUND_GAIN);
		break;
	case POKEY_OFFSET_AUDF4 + POKEY_OFFSET_POKEY2:
		POKEY_AUDF[POKEY_CHAN4 + POKEY_CHIP2] = byte;
		POKEYSND_Update(POKEY_OFFSET_AUDF4, byte, 1, SOUND_GAIN);
		break;
	case POKEY_OFFSET_STIMER + POKEY_OFFSET_POKEY2:
		POKEYSND_Update(POKEY_OFFSET_STIMER, byte, 1, SOUND_GAIN);
		break;
	case POKEY_OFFSET_SKCTL + POKEY_OFFSET_POKEY2:
		POKEYSND_Update(POKEY_OFFSET_SKCTL, byte, 1, SOUND_GAIN);
		break;
#endif
	}
}

int POKEY_Initialise(int *argc, char *argv[])
{
	int i;
	ULONG reg;

	/* Initialise Serial Port Interrupts */
	POKEY_DELAYED_SERIN_IRQ = 0;
	POKEY_DELAYED_SEROUT_IRQ = 0;
	POKEY_DELAYED_XMTDONE_IRQ = 0;

	POKEY_KBCODE = 0xff;
	POKEY_SERIN = 0x00;	/* or 0xff ? */
	POKEY_IRQST = 0xff;
	POKEY_IRQEN = 0x00;
	POKEY_SKSTAT = 0xef;
	POKEY_SKCTL = 0x00;

	for (i = 0; i < (POKEY_MAXPOKEYS * 4); i++) {
		POKEY_AUDC[i] = 0;
		POKEY_AUDF[i] = 0;
	}

	for (i = 0; i < POKEY_MAXPOKEYS; i++) {
		POKEY_AUDCTL[i] = 0;
		POKEY_Base_mult[i] = POKEY_DIV_64;
	}

	for (i = 0; i < 4; i++)
		POKEY_DivNIRQ[i] = POKEY_DivNMax[i] = 0;

	pot_scanline = 0;

	/* initialise poly9_lookup */
	reg = 0x1ff;
	for (i = 0; i < 511; i++) {
		reg = ((((reg >> 5) ^ reg) & 1) << 8) + (reg >> 1);
		POKEY_poly9_lookup[i] = (UBYTE) reg;
	}
	/* initialise poly17_lookup */
	reg = 0x1ffff;
	for (i = 0; i < 16385; i++) {
		reg = ((((reg >> 5) ^ reg) & 0xff) << 9) + (reg >> 8);
		POKEY_poly17_lookup[i] = (UBYTE) (reg >> 1);
	}

#ifndef BASIC
	if (INPUT_Playingback()) {
		random_scanline_counter = INPUT_PlaybackInt();
	}
	else
#endif
	{
		random_scanline_counter =
#ifdef HAVE_WINDOWS_H
		GetTickCount() % POKEY_POLY17_SIZE;
#elif defined(HAVE_TIME)
		time(NULL) % POKEY_POLY17_SIZE;
#else
		0;
#endif
	}
#ifndef BASIC
	if (INPUT_Recording()) {
		INPUT_RecordInt(random_scanline_counter);
	}
#endif

	return TRUE;
}

void POKEY_Frame(void)
{
	random_scanline_counter %= (POKEY_AUDCTL[0] & POKEY_POLY9) ? POKEY_POLY9_SIZE : POKEY_POLY17_SIZE;
}

/***************************************************************************
 ** Generate POKEY Timer IRQs if required                                 **
 ** called on a per-scanline basis, not very precise, but good enough     **
 ** for most applications                                                 **
 ***************************************************************************/

void POKEY_Scanline(void)
{
#ifdef POKEY_UPDATE
	pokey_update();
#endif

#ifdef VOL_ONLY_SOUND
	POKEYSND_UpdateVolOnly();
#endif

#ifndef BASIC
	INPUT_Scanline();	/* Handle Amiga and ST mice. */
						/* It's not a part of POKEY emulation, */
						/* but it looks to be the best place to put it. */
#endif

	if (pot_scanline < 228)
		pot_scanline++;

	random_scanline_counter += ANTIC_LINE_C;

	/* on nonpatched i/o-operation, enable the cassette timing */
	if (!ESC_enable_sio_patch)
		CASSETTE_AddScanLine();

	if (POKEY_DELAYED_SERIN_IRQ > 0) {
		if (--POKEY_DELAYED_SERIN_IRQ == 0) {
			if (POKEY_IRQEN & 0x20) {
				if (POKEY_IRQST & 0x20) {
					POKEY_IRQST &= 0xdf;
					POKEY_SERIN = SIO_GetByte();
#ifdef DEBUG2
					printf("SERIO: SERIN Interrupt triggered, bytevalue %02x\n", POKEY_SERIN);
#endif
				}
				else {
					POKEY_SKSTAT &= 0xdf;
#ifdef DEBUG2
					printf("SERIO: SERIN Interrupt triggered\n");
#endif
				}
				CPU_GenerateIRQ();
			}
#ifdef DEBUG2
			else {
				printf("SERIO: SERIN Interrupt missed\n");
			}
#endif
		}
	}

	if (POKEY_DELAYED_SEROUT_IRQ > 0) {
		if (--POKEY_DELAYED_SEROUT_IRQ == 0) {
			if (POKEY_IRQEN & 0x10) {
#ifdef DEBUG2
				printf("SERIO: SEROUT Interrupt triggered\n");
#endif
				POKEY_IRQST &= 0xef;
				CPU_GenerateIRQ();
			}
#ifdef DEBUG2
			else {
				printf("SERIO: SEROUT Interrupt missed\n");
			}
#endif
		}
	}

	if (POKEY_DELAYED_XMTDONE_IRQ > 0)
		if (--POKEY_DELAYED_XMTDONE_IRQ == 0) {
			POKEY_IRQST &= 0xf7;
			if (POKEY_IRQEN & 0x08) {
#ifdef DEBUG2
				printf("SERIO: XMTDONE Interrupt triggered\n");
#endif
				CPU_GenerateIRQ();
			}
#ifdef DEBUG2
			else
				printf("SERIO: XMTDONE Interrupt missed\n");
#endif
		}

	if ((POKEY_DivNIRQ[POKEY_CHAN1] -= ANTIC_LINE_C) < 0 ) {
		POKEY_DivNIRQ[POKEY_CHAN1] += POKEY_DivNMax[POKEY_CHAN1];
		if (POKEY_IRQEN & 0x01) {
			POKEY_IRQST &= 0xfe;
			CPU_GenerateIRQ();
		}
	}

	if ((POKEY_DivNIRQ[POKEY_CHAN2] -= ANTIC_LINE_C) < 0 ) {
		POKEY_DivNIRQ[POKEY_CHAN2] += POKEY_DivNMax[POKEY_CHAN2];
		if (POKEY_IRQEN & 0x02) {
			POKEY_IRQST &= 0xfd;
			CPU_GenerateIRQ();
		}
	}

	if ((POKEY_DivNIRQ[POKEY_CHAN4] -= ANTIC_LINE_C) < 0 ) {
		POKEY_DivNIRQ[POKEY_CHAN4] += POKEY_DivNMax[POKEY_CHAN4];
		if (POKEY_IRQEN & 0x04) {
			POKEY_IRQST &= 0xfb;
			CPU_GenerateIRQ();
		}
	}
}

/*****************************************************************************/
/* Module:  Update_Counter()                                                 */
/* Purpose: To process the latest control values stored in the AUDF, AUDC,   */
/*          and AUDCTL registers.  It pre-calculates as much information as  */
/*          possible for better performance.  This routine has been added    */
/*          here again as I need the precise frequency for the pokey timers  */
/*          again. The pokey emulation is therefore somewhat sub-optimal     */
/*          since the actual pokey emulation should grab the frequency values */
/*          directly from here instead of calculating them again.            */
/*                                                                           */
/* Author:  Ron Fries,Thomas Richter                                         */
/* Date:    March 27, 1998                                                   */
/*                                                                           */
/* Inputs:  chan_mask: Channel mask, one bit per channel.                    */
/*          The channels that need to be updated                             */
/*                                                                           */
/* Outputs: Adjusts local globals - no return value                          */
/*                                                                           */
/*****************************************************************************/

static void Update_Counter(int chan_mask)
{

/************************************************************/
/* As defined in the manual, the exact Div_n_cnt values are */
/* different depending on the frequency and resolution:     */
/*    64 kHz or 15 kHz - AUDF + 1                           */
/*    1 MHz, 8-bit -     AUDF + 4                           */
/*    1 MHz, 16-bit -    AUDF[CHAN1]+256*AUDF[CHAN2] + 7    */
/************************************************************/

	/* only reset the channels that have changed */

	if (chan_mask & (1 << POKEY_CHAN1)) {
		/* process channel 1 frequency */
		if (POKEY_AUDCTL[0] & POKEY_CH1_179)
			POKEY_DivNMax[POKEY_CHAN1] = POKEY_AUDF[POKEY_CHAN1] + 4;
		else
			POKEY_DivNMax[POKEY_CHAN1] = (POKEY_AUDF[POKEY_CHAN1] + 1) * POKEY_Base_mult[0];
		if (POKEY_DivNMax[POKEY_CHAN1] < ANTIC_LINE_C)
			POKEY_DivNMax[POKEY_CHAN1] = ANTIC_LINE_C;
	}

	if (chan_mask & (1 << POKEY_CHAN2)) {
		/* process channel 2 frequency */
		if (POKEY_AUDCTL[0] & POKEY_CH1_CH2) {
			if (POKEY_AUDCTL[0] & POKEY_CH1_179)
				POKEY_DivNMax[POKEY_CHAN2] = POKEY_AUDF[POKEY_CHAN2] * 256 + POKEY_AUDF[POKEY_CHAN1] + 7;
			else
				POKEY_DivNMax[POKEY_CHAN2] = (POKEY_AUDF[POKEY_CHAN2] * 256 + POKEY_AUDF[POKEY_CHAN1] + 1) * POKEY_Base_mult[0];
		}
		else
			POKEY_DivNMax[POKEY_CHAN2] = (POKEY_AUDF[POKEY_CHAN2] + 1) * POKEY_Base_mult[0];
		if (POKEY_DivNMax[POKEY_CHAN2] < ANTIC_LINE_C)
			POKEY_DivNMax[POKEY_CHAN2] = ANTIC_LINE_C;
	}

	if (chan_mask & (1 << POKEY_CHAN4)) {
		/* process channel 4 frequency */
		if (POKEY_AUDCTL[0] & POKEY_CH3_CH4) {
			if (POKEY_AUDCTL[0] & POKEY_CH3_179)
				POKEY_DivNMax[POKEY_CHAN4] = POKEY_AUDF[POKEY_CHAN4] * 256 + POKEY_AUDF[POKEY_CHAN3] + 7;
			else
				POKEY_DivNMax[POKEY_CHAN4] = (POKEY_AUDF[POKEY_CHAN4] * 256 + POKEY_AUDF[POKEY_CHAN3] + 1) * POKEY_Base_mult[0];
		}
		else
			POKEY_DivNMax[POKEY_CHAN4] = (POKEY_AUDF[POKEY_CHAN4] + 1) * POKEY_Base_mult[0];
		if (POKEY_DivNMax[POKEY_CHAN4] < ANTIC_LINE_C)
			POKEY_DivNMax[POKEY_CHAN4] = ANTIC_LINE_C;
	}
}

#ifndef BASIC

void POKEY_StateSave(void)
{
	int shift_key = 0;
	int keypressed = 0;

	StateSav_SaveUBYTE(&POKEY_KBCODE, 1);
	StateSav_SaveUBYTE(&POKEY_IRQST, 1);
	StateSav_SaveUBYTE(&POKEY_IRQEN, 1);
	StateSav_SaveUBYTE(&POKEY_SKCTL, 1);

	StateSav_SaveINT(&shift_key, 1);
	StateSav_SaveINT(&keypressed, 1);
	StateSav_SaveINT(&POKEY_DELAYED_SERIN_IRQ, 1);
	StateSav_SaveINT(&POKEY_DELAYED_SEROUT_IRQ, 1);
	StateSav_SaveINT(&POKEY_DELAYED_XMTDONE_IRQ, 1);

	StateSav_SaveUBYTE(&POKEY_AUDF[0], 4);
	StateSav_SaveUBYTE(&POKEY_AUDC[0], 4);
	StateSav_SaveUBYTE(&POKEY_AUDCTL[0], 1);

	StateSav_SaveINT(&POKEY_DivNIRQ[0], 4);
	StateSav_SaveINT(&POKEY_DivNMax[0], 4);
	StateSav_SaveINT(&POKEY_Base_mult[0], 1);
}

void POKEY_StateRead(void)
{
	int i;
	int shift_key;
	int keypressed;

	StateSav_ReadUBYTE(&POKEY_KBCODE, 1);
	StateSav_ReadUBYTE(&POKEY_IRQST, 1);
	StateSav_ReadUBYTE(&POKEY_IRQEN, 1);
	StateSav_ReadUBYTE(&POKEY_SKCTL, 1);

	StateSav_ReadINT(&shift_key, 1);
	StateSav_ReadINT(&keypressed, 1);
	StateSav_ReadINT(&POKEY_DELAYED_SERIN_IRQ, 1);
	StateSav_ReadINT(&POKEY_DELAYED_SEROUT_IRQ, 1);
	StateSav_ReadINT(&POKEY_DELAYED_XMTDONE_IRQ, 1);

	StateSav_ReadUBYTE(&POKEY_AUDF[0], 4);
	StateSav_ReadUBYTE(&POKEY_AUDC[0], 4);
	StateSav_ReadUBYTE(&POKEY_AUDCTL[0], 1);
	for (i = 0; i < 4; i++) {
		POKEY_PutByte((UWORD) (POKEY_OFFSET_AUDF1 + i * 2), POKEY_AUDF[i]);
		POKEY_PutByte((UWORD) (POKEY_OFFSET_AUDC1 + i * 2), POKEY_AUDC[i]);
	}
	POKEY_PutByte(POKEY_OFFSET_AUDCTL, POKEY_AUDCTL[0]);

	StateSav_ReadINT(&POKEY_DivNIRQ[0], 4);
	StateSav_ReadINT(&POKEY_DivNMax[0], 4);
	StateSav_ReadINT(&POKEY_Base_mult[0], 1);
}

#endif
