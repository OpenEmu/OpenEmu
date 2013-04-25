/*
 * sound_falcon.c - high-level sound routines for the Atari Falcon port
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2005 Atari800 development team (see DOC/CREDITS)
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

#ifdef SOUND
#include <stdlib.h>
#include <stdio.h>
#include <mint/osbind.h>
#include "atari.h"
#include "log.h"
#include "pokeysnd.h"
#include "util.h"

extern int get_cookie(long cookie, long *value);

static char *dsp_buffer1, *dsp_endbuf1;
static char *dsp_buffer2, *dsp_endbuf2;

static int sound_enabled = TRUE;

#define RATE12KHZ		12517
#define RATE25KHZ		25033
#define RATE50KHZ		50066

#define SNDBUFSIZE		600

static int dsprate = RATE12KHZ;
static int sndbufsize = SNDBUFSIZE;

/* Atari DMA sound hardware */
extern void timer_A(void);

#define	TIMERA		*(long *)0x134

#define	TACTRL		*(UBYTE *)0xFFFA19
#define	TADATA		*(UBYTE *)0xFFFA1F

#define	IEA			*(UBYTE *)0xFFFA07
#define	ISRA		*(UBYTE *)0xFFFA0F
#define	IMA			*(UBYTE *)0xFFFA13
#define IVECTOR		*(UBYTE *)0xFFFA17

long old_timer_A;
UBYTE old_tactrl, old_tadata, old_ivector, old_iea, old_isra, old_ima;
short *DMActrlptr = (short *) 0xff8900;

void Setbuffer(long bufbeg, long bufsize)
{
	long bufend = bufbeg + bufsize;
	DMActrlptr[1] = (bufbeg >> 16) & 0xff;
	DMActrlptr[2] = (bufbeg >> 8) & 0xff;
	DMActrlptr[3] = bufbeg & 0xff;
	DMActrlptr[7] = (bufend >> 16) & 0xff;
	DMActrlptr[8] = (bufend >> 8) & 0xff;
	DMActrlptr[9] = bufend & 0xff;
}

void Sound_Update(void)
{
	/* dunno what to do here as the sound precomputing is interrupt driven */
}

void timer_A_v_C(void)
{
	static int first = FALSE;	/* start computing second buffer */

	if (first) {
		Setbuffer((long)dsp_buffer1, sndbufsize);		/* set next DMA buffer */
		POKEYSND_Process(dsp_buffer1, sndbufsize);		/* quickly compute it */
		first = FALSE;
	}
	else {
		Setbuffer((long)dsp_buffer2, sndbufsize);
		POKEYSND_Process(dsp_buffer2, sndbufsize);
		first = TRUE;
	}
}

void MFP_IRQ_on(void)
{
	Setbuffer((long)dsp_buffer1, sndbufsize);		/* start playing first buffer */
	if (dsprate == RATE25KHZ)
		DMActrlptr[0x10] = 0x80 | 2;	/* mono 25 kHz */
	else if (dsprate == RATE50KHZ)
		DMActrlptr[0x10] = 0x80 | 3;	/* mono 50 kHz */
	else
		DMActrlptr[0x10] = 0x80 | 1;	/* mono 12 kHz */

	DMActrlptr[0] = 0x400 | 3;	/* play until stopped, interrupt at end of frame */

	Mfpint(13, timer_A);
	Xbtimer(0, 8, 1, timer_A);	/* event count mode, interrupt after 1st frame */
	IVECTOR &= ~(1 << 3);		/* turn on AEO */
	Jenabint(13);
}

void MFP_IRQ_off(void)
{
	Jdisint(13);
	DMActrlptr[0] = 0;			/* stop playing */
}

int Sound_Initialise(int *argc, char *argv[])
{
	int i, j;
	int help_only = FALSE;

	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */

		if (strcmp(argv[i], "-sound") == 0)
			sound_enabled = TRUE;
		else if (strcmp(argv[i], "-nosound") == 0)
			sound_enabled = FALSE;
		else if (strcmp(argv[i], "-dsprate") == 0) {
			if (i_a) {
				dsprate = Util_sscandec(argv[++i]);
				if (dsprate == RATE50KHZ)
					sndbufsize = 4*SNDBUFSIZE;
				else if (dsprate == RATE25KHZ)
					sndbufsize = 2*SNDBUFSIZE;
				else {
					dsprate = RATE12KHZ;
					sndbufsize = SNDBUFSIZE;
				}
			}
			else a_m = TRUE;
		}
		else {
			if (strcmp(argv[i], "-help") == 0) {
				help_only = TRUE;
				Log_print("\t-sound           Enable sound\n"
				       "\t-nosound         Disable sound\n"
				       "\t-dsprate <rate>  Set sample rate in Hz"
				      );
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			sound_enabled = FALSE;
			return FALSE;
		}
	}

	*argc = j;

	if (help_only) {
		sound_enabled = FALSE;
		return TRUE;
	}

	/* test of Sound hardware availability */
	if (sound_enabled) {
		long val;

		if (get_cookie('_SND', &val)) {
			if (!(val & 2))
				sound_enabled = FALSE;	/* Sound DMA hardware is missing */
		}
		else
			sound_enabled = FALSE;	/* CookieJar is missing */
	}

	if (sound_enabled) {
		dsp_buffer1 = (char *) Mxalloc(2 * sndbufsize, 0);
		if (!dsp_buffer1) {
			printf("can't allocate sound buffer\n");
			exit(1);
		}
		dsp_buffer2 = dsp_endbuf1 = dsp_buffer1 + sndbufsize;
		dsp_endbuf2 = dsp_buffer2 + sndbufsize;
		memset(dsp_buffer1, 0, sndbufsize);
		memset(dsp_buffer2, 127, sndbufsize);

#ifdef STEREO_SOUND
#  error "Unsupported Stereo Sound"
#else
		POKEYSND_Init(POKEYSND_FREQ_17_EXACT, dsprate, 1, 0);
#endif
		Supexec(MFP_IRQ_on);
	}
	return TRUE;
}

void Sound_Exit(void)
{
	if (sound_enabled) {
		Supexec(MFP_IRQ_off);
		Mfree(dsp_buffer1);
	}
}

#endif
void Sound_Pause(void)
{
#ifdef SOUND
	if (sound_enabled)
		Supexec(MFP_IRQ_off);
#endif
}

void Sound_Continue(void)
{
#ifdef SOUND
	if (sound_enabled)
		Supexec(MFP_IRQ_on);
#endif
}

