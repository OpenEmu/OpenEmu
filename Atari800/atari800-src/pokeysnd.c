/*
 * pokeysnd.c - POKEY sound chip emulation, v2.4
 *
 * Copyright (C) 1996-1998 Ron Fries
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

#ifdef ASAP /* external project, see http://asap.sf.net */
#include "asap_internal.h"
#else
#include "atari.h"
#ifndef __PLUS
#include "sndsave.h"
#else
#include "sound_win.h"
#endif
#endif
#include "mzpokeysnd.h"
#include "pokeysnd.h"
#if defined(PBI_XLD) || defined (VOICEBOX)
#include "votraxsnd.h"
#endif
#include "antic.h"
#include "gtia.h"

#ifdef WORDS_UNALIGNED_OK
#  define READ_U32(x)     (*(ULONG *) (x))
#  define WRITE_U32(x, d) (*(ULONG *) (x) = (d))
#else
#  ifdef WORDS_BIGENDIAN
#    define READ_U32(x) (((*(unsigned char *)(x)) << 24) | ((*((unsigned char *)(x) + 1)) << 16) | \
                        ((*((unsigned char *)(x) + 2)) << 8) | ((*((unsigned char *)(x) + 3))))
#    define WRITE_U32(x, d) \
  { \
  ULONG i = d; \
  (*(unsigned char *) (x)) = (((i) >> 24) & 255); \
  (*((unsigned char *) (x) + 1)) = (((i) >> 16) & 255); \
  (*((unsigned char *) (x) + 2)) = (((i) >> 8) & 255); \
  (*((unsigned char *) (x) + 3)) = ((i) & 255); \
  }
#  else
#    define READ_U32(x) ((*(unsigned char *) (x)) | ((*((unsigned char *) (x) + 1)) << 8) | \
                        ((*((unsigned char *) (x) + 2)) << 16) | ((*((unsigned char *) (x) + 3)) << 24))
#    define WRITE_U32(x, d) \
  { \
  ULONG i = d; \
  (*(unsigned char *)(x)) = ((i) & 255); \
  (*((unsigned char *)(x) + 1)) = (((i) >> 8) & 255); \
  (*((unsigned char *)(x) + 2)) = (((i) >> 16) & 255); \
  (*((unsigned char *)(x) + 3)) = (((i) >> 24) & 255); \
  }
#  endif
#endif

/* GLOBAL VARIABLE DEFINITIONS */

/* number of pokey chips currently emulated */
static UBYTE Num_pokeys;

static UBYTE pokeysnd_AUDV[4 * POKEY_MAXPOKEYS];	/* Channel volume - derived */

static UBYTE Outbit[4 * POKEY_MAXPOKEYS];		/* current state of the output (high or low) */

static UBYTE Outvol[4 * POKEY_MAXPOKEYS];		/* last output volume for each channel */

/* Initialize the bit patterns for the polynomials. */

/* The 4bit and 5bit patterns are the identical ones used in the pokey chip. */
/* Though the patterns could be packed with 8 bits per byte, using only a */
/* single bit per byte keeps the math simple, which is important for */
/* efficient processing. */

static UBYTE bit4[POKEY_POLY4_SIZE] =
#ifndef POKEY23_POLY
{1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0};	/* new table invented by Perry */
#else
{1, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0};	/* original POKEY 2.3 table */
#endif

static UBYTE bit5[POKEY_POLY5_SIZE] =
#ifndef POKEY23_POLY
{1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0};
#else
{0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1};
#endif

static ULONG P4 = 0,			/* Global position pointer for the 4-bit  POLY array */
 P5 = 0,						/* Global position pointer for the 5-bit  POLY array */
 P9 = 0,						/* Global position pointer for the 9-bit  POLY array */
 P17 = 0;						/* Global position pointer for the 17-bit POLY array */

static ULONG Div_n_cnt[4 * POKEY_MAXPOKEYS],		/* Divide by n counter. one for each channel */
 Div_n_max[4 * POKEY_MAXPOKEYS];		/* Divide by n maximum, one for each channel */

static ULONG Samp_n_max,		/* Sample max.  For accuracy, it is *256 */
 Samp_n_cnt[2];					/* Sample cnt. */

#ifdef INTERPOLATE_SOUND
static UWORD last_val = 0;		/* last output value */
#ifdef STEREO_SOUND
static UWORD last_val2 = 0;	/* last output value */
#endif
#endif

/* Volume only emulations declarations */
#ifdef VOL_ONLY_SOUND

int	POKEYSND_sampbuf_val[POKEYSND_SAMPBUF_MAX];	/* volume values */
int	POKEYSND_sampbuf_cnt[POKEYSND_SAMPBUF_MAX];	/* relative start time */
int	POKEYSND_sampbuf_ptr = 0;		/* pointer to sampbuf */
int	POKEYSND_sampbuf_rptr = 0;		/* pointer to read from sampbuf */
int	POKEYSND_sampbuf_last = 0;		/* last absolute time */
int	POKEYSND_sampbuf_AUDV[4 * POKEY_MAXPOKEYS];	/* prev. channel volume */
int	POKEYSND_sampbuf_lastval = 0;		/* last volume */
int	POKEYSND_sampout;			/* last out volume */
int	POKEYSND_samp_freq;
int	POKEYSND_samp_consol_val = 0;		/* actual value of console sound */
#ifdef STEREO_SOUND
static int	sampbuf_val2[POKEYSND_SAMPBUF_MAX];	/* volume values */
static int	sampbuf_cnt2[POKEYSND_SAMPBUF_MAX];	/* relative start time */
static int	sampbuf_ptr2 = 0;		/* pointer to sampbuf */
static int	sampbuf_rptr2 = 0;		/* pointer to read from sampbuf */
static int	sampbuf_last2 = 0;		/* last absolute time */
static int	sampbuf_lastval2 = 0;		/* last volume */
static int	sampout2;			/* last out volume */
#endif
#endif  /* VOL_ONLY_SOUND */

static ULONG snd_freq17 = POKEYSND_FREQ_17_EXACT;
int POKEYSND_playback_freq = 44100;
UBYTE POKEYSND_num_pokeys = 1;
int POKEYSND_snd_flags = 0;
static int mz_quality = 0;		/* default quality for mzpokeysnd */
#ifdef __PLUS
int mz_clear_regs = 0;
#endif

int POKEYSND_enable_new_pokey = TRUE;
int POKEYSND_bienias_fix = TRUE;  /* when TRUE, high frequencies get emulated: better sound but slower */
#if defined(__PLUS) && !defined(_WX_)
#define BIENIAS_FIX (g_Sound.nBieniasFix)
#else
#define BIENIAS_FIX POKEYSND_bienias_fix
#endif
#ifndef ASAP
int POKEYSND_stereo_enabled = FALSE;
#endif

/* multiple sound engine interface */
static void pokeysnd_process_8(void *sndbuffer, int sndn);
static void pokeysnd_process_16(void *sndbuffer, int sndn);
static void null_pokey_process(void *sndbuffer, int sndn) {}
void (*POKEYSND_Process_ptr)(void *sndbuffer, int sndn) = null_pokey_process;

static void Update_pokey_sound_rf(UWORD, UBYTE, UBYTE, UBYTE);
static void null_pokey_sound(UWORD addr, UBYTE val, UBYTE chip, UBYTE gain) {}
void (*POKEYSND_Update) (UWORD addr, UBYTE val, UBYTE chip, UBYTE gain)
  = null_pokey_sound;

#ifdef SERIO_SOUND
static void Update_serio_sound_rf(int out, UBYTE data);
static void null_serio_sound(int out, UBYTE data) {}
void (*POKEYSND_UpdateSerio)(int out, UBYTE data) = null_serio_sound;
int POKEYSND_serio_sound_enabled = 1;
#endif

#ifdef CONSOLE_SOUND
static void Update_consol_sound_rf(int set);
static void null_consol_sound(int set) {}
void (*POKEYSND_UpdateConsol)(int set) = null_consol_sound;
int POKEYSND_console_sound_enabled = 1;
#endif

#ifdef VOL_ONLY_SOUND
static void Update_vol_only_sound_rf(void);
static void null_vol_only_sound(void) {}
void (*POKEYSND_UpdateVolOnly)(void) = null_vol_only_sound;
#endif

/*****************************************************************************/
/* In my routines, I treat the sample output as another divide by N counter  */
/* For better accuracy, the Samp_n_cnt has a fixed binary decimal point      */
/* which has 8 binary digits to the right of the decimal point.  I use a two */
/* byte array to give me a minimum of 40 bits, and then use pointer math to  */
/* reference either the 24.8 whole/fraction combination or the 32-bit whole  */
/* only number.  This is mainly used to keep the math simple for             */
/* optimization. See below:                                                  */
/*                                                                           */
/* Representation on little-endian machines:                                 */
/* xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx | xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx */
/* fraction   whole    whole    whole      whole   unused   unused   unused  */
/*                                                                           */
/* Samp_n_cnt[0] gives me a 32-bit int 24 whole bits with 8 fractional bits, */
/* while (ULONG *)((UBYTE *)(&Samp_n_cnt[0])+1) gives me the 32-bit whole   */
/* number only.                                                              */
/*                                                                           */
/* Representation on big-endian machines:                                    */
/* xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx | xxxxxxxx xxxxxxxx xxxxxxxx.xxxxxxxx */
/*  unused   unused   unused    whole      whole    whole    whole  fraction */
/*                                                                           */
/* Samp_n_cnt[1] gives me a 32-bit int 24 whole bits with 8 fractional bits, */
/* while (ULONG *)((UBYTE *)(&Samp_n_cnt[0])+3) gives me the 32-bit whole   */
/* number only.                                                              */
/*****************************************************************************/


/*****************************************************************************/
/* Module:  pokeysnd_init_rf()                                              */
/* Purpose: to handle the power-up initialization functions                  */
/*          these functions should only be executed on a cold-restart        */
/*                                                                           */
/* Author:  Ron Fries                                                        */
/* Date:    January 1, 1997                                                  */
/*                                                                           */
/* Inputs:  freq17 - the value for the '1.79MHz' Pokey audio clock           */
/*          playback_freq - the playback frequency in samples per second     */
/*          num_pokeys - specifies the number of pokey chips to be emulated  */
/*                                                                           */
/* Outputs: Adjusts local globals - no return value                          */
/*                                                                           */
/*****************************************************************************/

static int pokeysnd_init_rf(ULONG freq17, int playback_freq,
           UBYTE num_pokeys, int flags);

int POKEYSND_DoInit(void)
{
	SndSave_CloseSoundFile();
	if (POKEYSND_enable_new_pokey)
		return MZPOKEYSND_Init(snd_freq17, POKEYSND_playback_freq,
				POKEYSND_num_pokeys, POKEYSND_snd_flags, mz_quality
#ifdef __PLUS
				, mz_clear_regs
#endif
		);
	else
		return pokeysnd_init_rf(snd_freq17, POKEYSND_playback_freq,
				POKEYSND_num_pokeys, POKEYSND_snd_flags);
}

int POKEYSND_Init(ULONG freq17, int playback_freq, UBYTE num_pokeys,
                     int flags
#ifdef __PLUS
                     , int clear_regs
#endif
)
{
	snd_freq17 = freq17;
	POKEYSND_playback_freq = playback_freq;
	POKEYSND_num_pokeys = num_pokeys;
	POKEYSND_snd_flags = flags;
#ifdef __PLUS
	mz_clear_regs = clear_regs;
#endif

#if defined(PBI_XLD) || defined (VOICEBOX)
	VOTRAXSND_Init(playback_freq, num_pokeys, (flags & POKEYSND_BIT16));
#endif
	return POKEYSND_DoInit();
}

void POKEYSND_SetMzQuality(int quality)	/* specially for win32, perhaps not needed? */
{
	mz_quality = quality;
}

void POKEYSND_Process(void *sndbuffer, int sndn)
{
	POKEYSND_Process_ptr(sndbuffer, sndn);
#if defined(PBI_XLD) || defined (VOICEBOX)
	VOTRAXSND_Process(sndbuffer,sndn);
#endif
#if !defined(__PLUS) && !defined(ASAP)
	SndSave_WriteToSoundFile((const unsigned char *)sndbuffer, sndn);
#endif
}

static int pokeysnd_init_rf(ULONG freq17, int playback_freq,
           UBYTE num_pokeys, int flags)
{
	UBYTE chan;

	POKEYSND_Update = Update_pokey_sound_rf;
#ifdef SERIO_SOUND
	POKEYSND_UpdateSerio = Update_serio_sound_rf;
#endif
#ifdef CONSOLE_SOUND
	POKEYSND_UpdateConsol = Update_consol_sound_rf;
#endif
#ifdef VOL_ONLY_SOUND
	POKEYSND_UpdateVolOnly = Update_vol_only_sound_rf;
#endif

	POKEYSND_Process_ptr = (flags & POKEYSND_BIT16) ? pokeysnd_process_16 : pokeysnd_process_8;

#ifdef VOL_ONLY_SOUND
	POKEYSND_samp_freq = playback_freq;
#endif

	/* start all of the polynomial counters at zero */
	P4 = 0;
	P5 = 0;
	P9 = 0;
	P17 = 0;

	/* calculate the sample 'divide by N' value based on the playback freq. */
	Samp_n_max = ((ULONG) freq17 << 8) / playback_freq;

	Samp_n_cnt[0] = 0;			/* initialize all bits of the sample */
	Samp_n_cnt[1] = 0;			/* 'divide by N' counter */

	for (chan = 0; chan < (POKEY_MAXPOKEYS * 4); chan++) {
		Outvol[chan] = 0;
		Outbit[chan] = 0;
		Div_n_cnt[chan] = 0;
		Div_n_max[chan] = 0x7fffffffL;
		pokeysnd_AUDV[chan] = 0;
#ifdef VOL_ONLY_SOUND
		POKEYSND_sampbuf_AUDV[chan] = 0;
#endif
	}

	/* set the number of pokey chips currently emulated */
	Num_pokeys = num_pokeys;

	return 0; /* OK */
}


/*****************************************************************************/
/* Module:  Update_pokey_sound_rf()                                          */
/* Purpose: To process the latest control values stored in the AUDF, AUDC,   */
/*          and AUDCTL registers.  It pre-calculates as much information as  */
/*          possible for better performance.  This routine has not been      */
/*          optimized.                                                       */
/*                                                                           */
/* Author:  Ron Fries                                                        */
/* Date:    January 1, 1997                                                  */
/*                                                                           */
/* Inputs:  addr - the address of the parameter to be changed                */
/*          val - the new value to be placed in the specified address        */
/*          gain - specified as an 8-bit fixed point number - use 1 for no   */
/*                 amplification (output is multiplied by gain)              */
/*                                                                           */
/* Outputs: Adjusts local globals - no return value                          */
/*                                                                           */
/*****************************************************************************/

static void Update_pokey_sound_rf(UWORD addr, UBYTE val, UBYTE chip,
				  UBYTE gain)
{
	ULONG new_val = 0;
	UBYTE chan;
	UBYTE chan_mask;
	UBYTE chip_offs;

	/* calculate the chip_offs for the channel arrays */
	chip_offs = chip << 2;

	/* determine which address was changed */
	switch (addr & 0x0f) {
	case POKEY_OFFSET_AUDF1:
		/* POKEY_AUDF[POKEY_CHAN1 + chip_offs] = val; */
		chan_mask = 1 << POKEY_CHAN1;
		if (POKEY_AUDCTL[chip] & POKEY_CH1_CH2)		/* if ch 1&2 tied together */
			chan_mask |= 1 << POKEY_CHAN2;	/* then also change on ch2 */
		break;
	case POKEY_OFFSET_AUDC1:
		/* POKEY_AUDC[POKEY_CHAN1 + chip_offs] = val; */
		pokeysnd_AUDV[POKEY_CHAN1 + chip_offs] = (val & POKEY_VOLUME_MASK) * gain;
		chan_mask = 1 << POKEY_CHAN1;
		break;
	case POKEY_OFFSET_AUDF2:
		/* POKEY_AUDF[POKEY_CHAN2 + chip_offs] = val; */
		chan_mask = 1 << POKEY_CHAN2;
		break;
	case POKEY_OFFSET_AUDC2:
		/* POKEY_AUDC[POKEY_CHAN2 + chip_offs] = val; */
		pokeysnd_AUDV[POKEY_CHAN2 + chip_offs] = (val & POKEY_VOLUME_MASK) * gain;
		chan_mask = 1 << POKEY_CHAN2;
		break;
	case POKEY_OFFSET_AUDF3:
		/* POKEY_AUDF[POKEY_CHAN3 + chip_offs] = val; */
		chan_mask = 1 << POKEY_CHAN3;
		if (POKEY_AUDCTL[chip] & POKEY_CH3_CH4)		/* if ch 3&4 tied together */
			chan_mask |= 1 << POKEY_CHAN4;	/* then also change on ch4 */
		break;
	case POKEY_OFFSET_AUDC3:
		/* POKEY_AUDC[POKEY_CHAN3 + chip_offs] = val; */
		pokeysnd_AUDV[POKEY_CHAN3 + chip_offs] = (val & POKEY_VOLUME_MASK) * gain;
		chan_mask = 1 << POKEY_CHAN3;
		break;
	case POKEY_OFFSET_AUDF4:
		/* POKEY_AUDF[POKEY_CHAN4 + chip_offs] = val; */
		chan_mask = 1 << POKEY_CHAN4;
		break;
	case POKEY_OFFSET_AUDC4:
		/* POKEY_AUDC[POKEY_CHAN4 + chip_offs] = val; */
		pokeysnd_AUDV[POKEY_CHAN4 + chip_offs] = (val & POKEY_VOLUME_MASK) * gain;
		chan_mask = 1 << POKEY_CHAN4;
		break;
	case POKEY_OFFSET_AUDCTL:
		/* POKEY_AUDCTL[chip] = val; */
		chan_mask = 15;			/* all channels */
		break;
	default:
		chan_mask = 0;
		break;
	}

	/************************************************************/
	/* As defined in the manual, the exact Div_n_cnt values are */
	/* different depending on the frequency and resolution:     */
	/*    64 kHz or 15 kHz - AUDF + 1                           */
	/*    1 MHz, 8-bit -     AUDF + 4                           */
	/*    1 MHz, 16-bit -    POKEY_AUDF[POKEY_CHAN1]+256*POKEY_AUDF[POKEY_CHAN2] + 7    */
	/************************************************************/

	/* only reset the channels that have changed */

	if (chan_mask & (1 << POKEY_CHAN1)) {
		/* process channel 1 frequency */
		if (POKEY_AUDCTL[chip] & POKEY_CH1_179)
			new_val = POKEY_AUDF[POKEY_CHAN1 + chip_offs] + 4;
		else
			new_val = (POKEY_AUDF[POKEY_CHAN1 + chip_offs] + 1) * POKEY_Base_mult[chip];

		if (new_val != Div_n_max[POKEY_CHAN1 + chip_offs]) {
			Div_n_max[POKEY_CHAN1 + chip_offs] = new_val;

			if (Div_n_cnt[POKEY_CHAN1 + chip_offs] > new_val) {
				Div_n_cnt[POKEY_CHAN1 + chip_offs] = new_val;
			}
		}
	}

	if (chan_mask & (1 << POKEY_CHAN2)) {
		/* process channel 2 frequency */
		if (POKEY_AUDCTL[chip] & POKEY_CH1_CH2) {
			if (POKEY_AUDCTL[chip] & POKEY_CH1_179)
				new_val = POKEY_AUDF[POKEY_CHAN2 + chip_offs] * 256 +
					POKEY_AUDF[POKEY_CHAN1 + chip_offs] + 7;
			else
				new_val = (POKEY_AUDF[POKEY_CHAN2 + chip_offs] * 256 +
						   POKEY_AUDF[POKEY_CHAN1 + chip_offs] + 1) * POKEY_Base_mult[chip];
		}
		else
			new_val = (POKEY_AUDF[POKEY_CHAN2 + chip_offs] + 1) * POKEY_Base_mult[chip];

		if (new_val != Div_n_max[POKEY_CHAN2 + chip_offs]) {
			Div_n_max[POKEY_CHAN2 + chip_offs] = new_val;

			if (Div_n_cnt[POKEY_CHAN2 + chip_offs] > new_val) {
				Div_n_cnt[POKEY_CHAN2 + chip_offs] = new_val;
			}
		}
	}

	if (chan_mask & (1 << POKEY_CHAN3)) {
		/* process channel 3 frequency */
		if (POKEY_AUDCTL[chip] & POKEY_CH3_179)
			new_val = POKEY_AUDF[POKEY_CHAN3 + chip_offs] + 4;
		else
			new_val = (POKEY_AUDF[POKEY_CHAN3 + chip_offs] + 1) * POKEY_Base_mult[chip];

		if (new_val != Div_n_max[POKEY_CHAN3 + chip_offs]) {
			Div_n_max[POKEY_CHAN3 + chip_offs] = new_val;

			if (Div_n_cnt[POKEY_CHAN3 + chip_offs] > new_val) {
				Div_n_cnt[POKEY_CHAN3 + chip_offs] = new_val;
			}
		}
	}

	if (chan_mask & (1 << POKEY_CHAN4)) {
		/* process channel 4 frequency */
		if (POKEY_AUDCTL[chip] & POKEY_CH3_CH4) {
			if (POKEY_AUDCTL[chip] & POKEY_CH3_179)
				new_val = POKEY_AUDF[POKEY_CHAN4 + chip_offs] * 256 +
					POKEY_AUDF[POKEY_CHAN3 + chip_offs] + 7;
			else
				new_val = (POKEY_AUDF[POKEY_CHAN4 + chip_offs] * 256 +
						   POKEY_AUDF[POKEY_CHAN3 + chip_offs] + 1) * POKEY_Base_mult[chip];
		}
		else
			new_val = (POKEY_AUDF[POKEY_CHAN4 + chip_offs] + 1) * POKEY_Base_mult[chip];

		if (new_val != Div_n_max[POKEY_CHAN4 + chip_offs]) {
			Div_n_max[POKEY_CHAN4 + chip_offs] = new_val;

			if (Div_n_cnt[POKEY_CHAN4 + chip_offs] > new_val) {
				Div_n_cnt[POKEY_CHAN4 + chip_offs] = new_val;
			}
		}
	}

	/* if channel is volume only, set current output */
	for (chan = POKEY_CHAN1; chan <= POKEY_CHAN4; chan++) {
		if (chan_mask & (1 << chan)) {

#ifdef VOL_ONLY_SOUND

#ifdef __PLUS
			if (g_Sound.nDigitized)
#endif
			if ((POKEY_AUDC[chan + chip_offs] & POKEY_VOL_ONLY)) {

#ifdef STEREO_SOUND

#ifdef __PLUS
				if (POKEYSND_stereo_enabled && chip & 0x01)
#else
				if (chip & 0x01)
#endif
				{
					sampbuf_lastval2 += pokeysnd_AUDV[chan + chip_offs]
						- POKEYSND_sampbuf_AUDV[chan + chip_offs];

					sampbuf_val2[sampbuf_ptr2] = sampbuf_lastval2;
					POKEYSND_sampbuf_AUDV[chan + chip_offs] = pokeysnd_AUDV[chan + chip_offs];
					sampbuf_cnt2[sampbuf_ptr2] =
						(ANTIC_CPU_CLOCK - sampbuf_last2) * 128 * POKEYSND_samp_freq / 178979;
					sampbuf_last2 = ANTIC_CPU_CLOCK;
					sampbuf_ptr2++;
					if (sampbuf_ptr2 >= POKEYSND_SAMPBUF_MAX)
						sampbuf_ptr2 = 0;
					if (sampbuf_ptr2 == sampbuf_rptr2) {
						sampbuf_rptr2++;
						if (sampbuf_rptr2 >= POKEYSND_SAMPBUF_MAX)
							sampbuf_rptr2 = 0;
					}
				}
				else
#endif /* STEREO_SOUND */
				{
					POKEYSND_sampbuf_lastval += pokeysnd_AUDV[chan + chip_offs]
						-POKEYSND_sampbuf_AUDV[chan + chip_offs];

					POKEYSND_sampbuf_val[POKEYSND_sampbuf_ptr] = POKEYSND_sampbuf_lastval;
					POKEYSND_sampbuf_AUDV[chan + chip_offs] = pokeysnd_AUDV[chan + chip_offs];
					POKEYSND_sampbuf_cnt[POKEYSND_sampbuf_ptr] =
						(ANTIC_CPU_CLOCK - POKEYSND_sampbuf_last) * 128 * POKEYSND_samp_freq / 178979;
					POKEYSND_sampbuf_last = ANTIC_CPU_CLOCK;
					POKEYSND_sampbuf_ptr++;
					if (POKEYSND_sampbuf_ptr >= POKEYSND_SAMPBUF_MAX)
						POKEYSND_sampbuf_ptr = 0;
					if (POKEYSND_sampbuf_ptr == POKEYSND_sampbuf_rptr) {
						POKEYSND_sampbuf_rptr++;
						if (POKEYSND_sampbuf_rptr >= POKEYSND_SAMPBUF_MAX)
							POKEYSND_sampbuf_rptr = 0;
					}
				}
			}

#endif /* VOL_ONLY_SOUND */

			/* I've disabled any frequencies that exceed the sampling
			   frequency.  There isn't much point in processing frequencies
			   that the hardware can't reproduce.  I've also disabled
			   processing if the volume is zero. */

			/* if the channel is volume only */
			/* or the channel is off (volume == 0) */
			/* or the channel freq is greater than the playback freq */
			if ( (POKEY_AUDC[chan + chip_offs] & POKEY_VOL_ONLY) ||
				((POKEY_AUDC[chan + chip_offs] & POKEY_VOLUME_MASK) == 0)
				|| (!BIENIAS_FIX && (Div_n_max[chan + chip_offs] < (Samp_n_max >> 8)))
				) {
				/* indicate the channel is 'on' */
				Outvol[chan + chip_offs] = 1;

				/* can only ignore channel if filtering off */
				if ((chan == POKEY_CHAN3 && !(POKEY_AUDCTL[chip] & POKEY_CH1_FILTER)) ||
					(chan == POKEY_CHAN4 && !(POKEY_AUDCTL[chip] & POKEY_CH2_FILTER)) ||
					(chan == POKEY_CHAN1) ||
					(chan == POKEY_CHAN2)
					|| (!BIENIAS_FIX && (Div_n_max[chan + chip_offs] < (Samp_n_max >> 8)))
				) {
					/* and set channel freq to max to reduce processing */
					Div_n_max[chan + chip_offs] = 0x7fffffffL;
					Div_n_cnt[chan + chip_offs] = 0x7fffffffL;
				}
			}
		}
	}

	/*    _enable(); */ /* RSF - removed for portability 31-MAR-97 */
}


/*****************************************************************************/
/* Module:  pokeysnd_process()                                                  */
/* Purpose: To fill the output buffer with the sound output based on the     */
/*          pokey chip parameters.                                           */
/*                                                                           */
/* Author:  Ron Fries                                                        */
/* Date:    January 1, 1997                                                  */
/*                                                                           */
/* Inputs:  *buffer - pointer to the buffer where the audio output will      */
/*                    be placed                                              */
/*          sndn - for mono, size of the playback buffer in samples          */
/*                 for stereo, size of the playback buffer in left samples   */
/*                    plus right samples.                                    */
/*          num_pokeys - number of currently active pokeys to process        */
/*                                                                           */
/* Outputs: the buffer will be filled with n bytes of audio - no return val  */
/*          Also the buffer will be written to disk if Sound recording is ON */
/*                                                                           */
/*****************************************************************************/

static void pokeysnd_process_8(void *sndbuffer, int sndn)
{
	register UBYTE *buffer = (UBYTE *) sndbuffer;
	register int n = sndn;

	register ULONG *div_n_ptr;
	register UBYTE *samp_cnt_w_ptr;
	register ULONG event_min;
	register UBYTE next_event;
#ifdef CLIP_SOUND
	register SWORD cur_val;		/* then we have to count as 16-bit signed */
#ifdef STEREO_SOUND
	register SWORD cur_val2;
#endif
#else /* CLIP_SOUND */
	register UBYTE cur_val;		/* otherwise we'll simplify as 8-bit unsigned */
#ifdef STEREO_SOUND
	register UBYTE cur_val2;
#endif
#endif /* CLIP_SOUND */
	register UBYTE *out_ptr;
	register UBYTE audc;
	register UBYTE toggle;
	register UBYTE count;
	register UBYTE *vol_ptr;

	/* set a pointer to the whole portion of the samp_n_cnt */
#ifdef WORDS_BIGENDIAN
	samp_cnt_w_ptr = ((UBYTE *) (&Samp_n_cnt[0]) + 3);
#else
	samp_cnt_w_ptr = ((UBYTE *) (&Samp_n_cnt[0]) + 1);
#endif

	/* set a pointer for optimization */
	out_ptr = Outvol;
	vol_ptr = pokeysnd_AUDV;

	/* The current output is pre-determined and then adjusted based on each */
	/* output change for increased performance (less over-all math). */
	/* add the output values of all 4 channels */
	cur_val = POKEYSND_SAMP_MIN;
#ifdef STEREO_SOUND
#ifdef __PLUS
	if (POKEYSND_stereo_enabled)
#endif
	cur_val2 = POKEYSND_SAMP_MIN;
#endif /* STEREO_SOUND */

	count = Num_pokeys;
	do {
		if (*out_ptr++)
			cur_val += *vol_ptr;
		vol_ptr++;

		if (*out_ptr++)
			cur_val += *vol_ptr;
		vol_ptr++;

		if (*out_ptr++)
			cur_val += *vol_ptr;
		vol_ptr++;

		if (*out_ptr++)
			cur_val += *vol_ptr;
		vol_ptr++;
#ifdef STEREO_SOUND
#ifdef __PLUS
		if (POKEYSND_stereo_enabled)
#endif
		{
			count--;
			if (count) {
				if (*out_ptr++)
					cur_val2 += *vol_ptr;
				vol_ptr++;

				if (*out_ptr++)
					cur_val2 += *vol_ptr;
				vol_ptr++;

				if (*out_ptr++)
					cur_val2 += *vol_ptr;
				vol_ptr++;

				if (*out_ptr++)
					cur_val2 += *vol_ptr;
				vol_ptr++;
			}
			else
				break;
		}
#endif /* STEREO_SOUND */
		count--;
	} while (count);
/*
#if defined (USE_DOSSOUND)
	cur_val += 32 * GTIA_speaker;
#endif
*/

	/* loop until the buffer is filled */
	while (n) {
		/* Normally the routine would simply decrement the 'div by N' */
		/* counters and react when they reach zero.  Since we normally */
		/* won't be processing except once every 80 or so counts, */
		/* I've optimized by finding the smallest count and then */
		/* 'accelerated' time by adjusting all pointers by that amount. */

		/* find next smallest event (either sample or chan 1-4) */
		next_event = POKEY_SAMPLE;
		event_min = READ_U32(samp_cnt_w_ptr);

		div_n_ptr = Div_n_cnt;

		count = 0;
		do {
			/* Though I could have used a loop here, this is faster */
			if (*div_n_ptr <= event_min) {
				event_min = *div_n_ptr;
				next_event = POKEY_CHAN1 + (count << 2);
			}
			div_n_ptr++;
			if (*div_n_ptr <= event_min) {
				event_min = *div_n_ptr;
				next_event = POKEY_CHAN2 + (count << 2);
			}
			div_n_ptr++;
			if (*div_n_ptr <= event_min) {
				event_min = *div_n_ptr;
				next_event = POKEY_CHAN3 + (count << 2);
			}
			div_n_ptr++;
			if (*div_n_ptr <= event_min) {
				event_min = *div_n_ptr;
				next_event = POKEY_CHAN4 + (count << 2);
			}
			div_n_ptr++;

			count++;
		} while (count < Num_pokeys);

		/* if the next event is a channel change */
		if (next_event != POKEY_SAMPLE) {
			/* shift the polynomial counters */

			count = Num_pokeys;
			do {
				/* decrement all counters by the smallest count found */
				/* again, no loop for efficiency */
				div_n_ptr--;
				*div_n_ptr -= event_min;
				div_n_ptr--;
				*div_n_ptr -= event_min;
				div_n_ptr--;
				*div_n_ptr -= event_min;
				div_n_ptr--;
				*div_n_ptr -= event_min;

				count--;
			} while (count);


			WRITE_U32(samp_cnt_w_ptr, READ_U32(samp_cnt_w_ptr) - event_min);

			/* since the polynomials require a mod (%) function which is
			   division, I don't adjust the polynomials on the SAMPLE events,
			   only the CHAN events.  I have to keep track of the change,
			   though. */

			P4 = (P4 + event_min) % POKEY_POLY4_SIZE;
			P5 = (P5 + event_min) % POKEY_POLY5_SIZE;
			P9 = (P9 + event_min) % POKEY_POLY9_SIZE;
			P17 = (P17 + event_min) % POKEY_POLY17_SIZE;

			/* adjust channel counter */
			Div_n_cnt[next_event] += Div_n_max[next_event];

			/* get the current AUDC into a register (for optimization) */
			audc = POKEY_AUDC[next_event];

			/* set a pointer to the current output (for opt...) */
			out_ptr = &Outvol[next_event];

			/* assume no changes to the output */
			toggle = FALSE;

			/* From here, a good understanding of the hardware is required */
			/* to understand what is happening.  I won't be able to provide */
			/* much description to explain it here. */

			/* if VOLUME only then nothing to process */
			if (!(audc & POKEY_VOL_ONLY)) {
				/* if the output is pure or the output is poly5 and the poly5 bit */
				/* is set */
				if ((audc & POKEY_NOTPOLY5) || bit5[P5]) {
					/* if the PURETONE bit is set */
					if (audc & POKEY_PURETONE) {
						/* then simply toggle the output */
						toggle = TRUE;
					}
					/* otherwise if POLY4 is selected */
					else if (audc & POKEY_POLY4) {
						/* then compare to the poly4 bit */
						toggle = (bit4[P4] == !(*out_ptr));
					}
					else {
						/* if 9-bit poly is selected on this chip */
						if (POKEY_AUDCTL[next_event >> 2] & POKEY_POLY9) {
							/* compare to the poly9 bit */
							toggle = ((POKEY_poly9_lookup[P9] & 1) == !(*out_ptr));
						}
						else {
							/* otherwise compare to the poly17 bit */
							toggle = (((POKEY_poly17_lookup[P17 >> 3] >> (P17 & 7)) & 1) == !(*out_ptr));
						}
					}
				}
			}

			/* check channel 1 filter (clocked by channel 3) */
			if ( POKEY_AUDCTL[next_event >> 2] & POKEY_CH1_FILTER) {
				/* if we're processing channel 3 */
				if ((next_event & 0x03) == POKEY_CHAN3) {
					/* check output of channel 1 on same chip */
					if (Outvol[next_event & 0xfd]) {
						/* if on, turn it off */
						Outvol[next_event & 0xfd] = 0;
#ifdef STEREO_SOUND
#ifdef __PLUS
						if (POKEYSND_stereo_enabled && (next_event & 0x04))
#else
						if ((next_event & 0x04))
#endif
							cur_val2 -= pokeysnd_AUDV[next_event & 0xfd];
						else
#endif /* STEREO_SOUND */
							cur_val -= pokeysnd_AUDV[next_event & 0xfd];
					}
				}
			}

			/* check channel 2 filter (clocked by channel 4) */
			if ( POKEY_AUDCTL[next_event >> 2] & POKEY_CH2_FILTER) {
				/* if we're processing channel 4 */
				if ((next_event & 0x03) == POKEY_CHAN4) {
					/* check output of channel 2 on same chip */
					if (Outvol[next_event & 0xfd]) {
						/* if on, turn it off */
						Outvol[next_event & 0xfd] = 0;
#ifdef STEREO_SOUND
#ifdef __PLUS
						if (POKEYSND_stereo_enabled && (next_event & 0x04))
#else
						if ((next_event & 0x04))
#endif
							cur_val2 -= pokeysnd_AUDV[next_event & 0xfd];
						else
#endif /* STEREO_SOUND */
							cur_val -= pokeysnd_AUDV[next_event & 0xfd];
					}
				}
			}

			/* if the current output bit has changed */
			if (toggle) {
				if (*out_ptr) {
					/* remove this channel from the signal */
#ifdef STEREO_SOUND
#ifdef __PLUS
					if (POKEYSND_stereo_enabled && (next_event & 0x04))
#else
					if ((next_event & 0x04))
#endif
						cur_val2 -= pokeysnd_AUDV[next_event];
					else
#endif /* STEREO_SOUND */
						cur_val -= pokeysnd_AUDV[next_event];

					/* and turn the output off */
					*out_ptr = 0;
				}
				else {
					/* turn the output on */
					*out_ptr = 1;

					/* and add it to the output signal */
#ifdef STEREO_SOUND
#ifdef __PLUS
					if (POKEYSND_stereo_enabled && (next_event & 0x04))
#else
					if ((next_event & 0x04))
#endif
						cur_val2 += pokeysnd_AUDV[next_event];
					else
#endif /* STEREO_SOUND */
						cur_val += pokeysnd_AUDV[next_event];
				}
			}
		}
		else {					/* otherwise we're processing a sample */
			/* adjust the sample counter - note we're using the 24.8 integer
			   which includes an 8 bit fraction for accuracy */

			int iout;
#ifdef STEREO_SOUND
			int iout2;
#endif
#ifdef INTERPOLATE_SOUND
			if (cur_val != last_val) {
				if (*Samp_n_cnt < Samp_n_max) {		/* need interpolation */
					iout = (cur_val * (*Samp_n_cnt) +
							last_val * (Samp_n_max - *Samp_n_cnt))
						/ Samp_n_max;
				}
				else
					iout = cur_val;
				last_val = cur_val;
			}
			else
				iout = cur_val;
#ifdef STEREO_SOUND
#ifdef __PLUS
		if (POKEYSND_stereo_enabled)
#endif
			if (cur_val2 != last_val2) {
				if (*Samp_n_cnt < Samp_n_max) {		/* need interpolation */
					iout2 = (cur_val2 * (*Samp_n_cnt) +
							last_val2 * (Samp_n_max - *Samp_n_cnt))
						/ Samp_n_max;
				}
				else
					iout2 = cur_val2;
				last_val2 = cur_val2;
			}
			else
				iout2 = cur_val2;
#endif  /* STEREO_SOUND */
#else   /* INTERPOLATE_SOUND */
			iout = cur_val;
#ifdef STEREO_SOUND
#ifdef __PLUS
		if (POKEYSND_stereo_enabled)
#endif
			iout2 = cur_val2;
#endif  /* STEREO_SOUND */
#endif  /* INTERPOLATE_SOUND */

#ifdef VOL_ONLY_SOUND
#ifdef __PLUS
			if (g_Sound.nDigitized)
#endif
			{
				if (POKEYSND_sampbuf_rptr != POKEYSND_sampbuf_ptr) {
					int l;
					if (POKEYSND_sampbuf_cnt[POKEYSND_sampbuf_rptr] > 0)
						POKEYSND_sampbuf_cnt[POKEYSND_sampbuf_rptr] -= 1280;
					while ((l = POKEYSND_sampbuf_cnt[POKEYSND_sampbuf_rptr]) <= 0) {
						POKEYSND_sampout = POKEYSND_sampbuf_val[POKEYSND_sampbuf_rptr];
						POKEYSND_sampbuf_rptr++;
						if (POKEYSND_sampbuf_rptr >= POKEYSND_SAMPBUF_MAX)
							POKEYSND_sampbuf_rptr = 0;
						if (POKEYSND_sampbuf_rptr != POKEYSND_sampbuf_ptr)
							POKEYSND_sampbuf_cnt[POKEYSND_sampbuf_rptr] += l;
						else
							break;
					}
				}
				iout += POKEYSND_sampout;
#ifdef STEREO_SOUND
#ifdef __PLUS
				if (POKEYSND_stereo_enabled)
#endif
				{
					if (sampbuf_rptr2 != sampbuf_ptr2) {
						int l;
						if (sampbuf_cnt2[sampbuf_rptr2] > 0)
							sampbuf_cnt2[sampbuf_rptr2] -= 1280;
						while ((l = sampbuf_cnt2[sampbuf_rptr2]) <= 0) {
							sampout2 = sampbuf_val2[sampbuf_rptr2];
							sampbuf_rptr2++;
							if (sampbuf_rptr2 >= POKEYSND_SAMPBUF_MAX)
								sampbuf_rptr2 = 0;
							if (sampbuf_rptr2 != sampbuf_ptr2)
								sampbuf_cnt2[sampbuf_rptr2] += l;
							else
								break;
						}
					}
					iout2 += sampout2;
				}
#endif  /* STEREO_SOUND */
			}
#endif  /* VOL_ONLY_SOUND */

#ifdef CLIP_SOUND
			if (iout > POKEYSND_SAMP_MAX) {	/* then check high limit */
				*buffer++ = (UBYTE) POKEYSND_SAMP_MAX;	/* and limit if greater */
			}
			else if (iout < POKEYSND_SAMP_MIN) {		/* else check low limit */
				*buffer++ = (UBYTE) POKEYSND_SAMP_MIN;	/* and limit if less */
			}
			else {				/* otherwise use raw value */
				*buffer++ = (UBYTE) iout;
			}
#ifdef STEREO_SOUND
#ifdef __PLUS
			if (POKEYSND_stereo_enabled) {
				if (iout2 > POKEYSND_SAMP_MAX)
					*buffer++ = (UBYTE) POKEYSND_SAMP_MAX;
				else if (iout2 < POKEYSND_SAMP_MIN)
					*buffer++ = (UBYTE) POKEYSND_SAMP_MIN;
				else
					*buffer++ = (UBYTE) iout2;
			}
#else /* __PLUS */
			if (Num_pokeys > 1) {
				if ((POKEYSND_stereo_enabled ? iout2 : iout) > POKEYSND_SAMP_MAX) {	/* then check high limit */
					*buffer++ = (UBYTE) POKEYSND_SAMP_MAX;	/* and limit if greater */
				}
				else if ((POKEYSND_stereo_enabled ? iout2 : iout) < POKEYSND_SAMP_MIN) {		/* else check low limit */
					*buffer++ = (UBYTE) POKEYSND_SAMP_MIN;	/* and limit if less */
				}
				else {				/* otherwise use raw value */
					*buffer++ = (UBYTE) (POKEYSND_stereo_enabled ? iout2 : iout);
				}
			}
#endif /* __PLUS */
#endif /* STEREO_SOUND */
#else /* CLIP_SOUND */
			*buffer++ = (UBYTE) iout;	/* clipping not selected, use value */
#ifdef STEREO_SOUND
			if (Num_pokeys > 1)
#ifdef ASAP
				*buffer++ = (UBYTE) iout2;
#else
				*buffer++ = (UBYTE) (POKEYSND_stereo_enabled ? iout2 : iout);
#endif
#endif /* STEREO_SOUND */
#endif /* CLIP_SOUND */

#ifdef WORDS_BIGENDIAN
			*(Samp_n_cnt + 1) += Samp_n_max;
#else
			*Samp_n_cnt += Samp_n_max;
#endif
			/* and indicate one less byte in the buffer */
			n--;
#ifdef STEREO_SOUND
#ifdef __PLUS
			if (POKEYSND_stereo_enabled)
#endif
			if (Num_pokeys > 1)
				n--;
#endif
		}
	}
#ifdef VOL_ONLY_SOUND
#ifdef __PLUS
	if (g_Sound.nDigitized)
#endif
	{
		if (POKEYSND_sampbuf_rptr == POKEYSND_sampbuf_ptr)
			POKEYSND_sampbuf_last = ANTIC_CPU_CLOCK;
#ifdef STEREO_SOUND
#ifdef __PLUS
	if (POKEYSND_stereo_enabled)
#endif
		if (sampbuf_rptr2 == sampbuf_ptr2)
			sampbuf_last2 = ANTIC_CPU_CLOCK;
#endif /* STEREO_SOUND */
	}
#endif  /* VOL_ONLY_SOUND */
}

#ifdef SERIO_SOUND
static void Update_serio_sound_rf(int out, UBYTE data)
{
#ifdef VOL_ONLY_SOUND
#ifdef __PLUS
	if (g_Sound.nDigitized) {
#endif
	int bits, pv, future;
	if (!POKEYSND_serio_sound_enabled) return;

	pv = 0;
	future = 0;
	bits = (data << 1) | 0x200;
	while (bits)
	{
		POKEYSND_sampbuf_lastval -= pv;
		pv = (bits & 0x01) * pokeysnd_AUDV[3];	/* FIXME!!! - set volume from AUDV */
		POKEYSND_sampbuf_lastval += pv;

	POKEYSND_sampbuf_val[POKEYSND_sampbuf_ptr] = POKEYSND_sampbuf_lastval;
	POKEYSND_sampbuf_cnt[POKEYSND_sampbuf_ptr] =
		(ANTIC_CPU_CLOCK + future-POKEYSND_sampbuf_last) * 128 * POKEYSND_samp_freq / 178979;
	POKEYSND_sampbuf_last = ANTIC_CPU_CLOCK + future;
	POKEYSND_sampbuf_ptr++;
	if (POKEYSND_sampbuf_ptr >= POKEYSND_SAMPBUF_MAX )
		POKEYSND_sampbuf_ptr = 0;
	if (POKEYSND_sampbuf_ptr == POKEYSND_sampbuf_rptr ) {
		POKEYSND_sampbuf_rptr++;
		if (POKEYSND_sampbuf_rptr >= POKEYSND_SAMPBUF_MAX)
			POKEYSND_sampbuf_rptr = 0;
	}
		/* 1789790/19200 = 93 */
		future += 93;	/* ~ 19200 bit/s - FIXME!!! set speed form AUDF [2] ??? */
		bits >>= 1;
	}
	POKEYSND_sampbuf_lastval -= pv;
#ifdef __PLUS
	}
#endif
#endif  /* VOL_ONLY_SOUND */
}
#endif /* SERIO_SOUND */

static void pokeysnd_process_16(void *sndbuffer, int sndn)
{
	UWORD *buffer = (UWORD *) sndbuffer;
	int i;

	pokeysnd_process_8(buffer, sndn);

	for (i = sndn - 1; i >= 0; i--) {
		int smp = ((int) (((UBYTE *) buffer)[i]) - 0x80) * 0x100;

		if (smp > 32767)
			smp = 32767;
		else if (smp < -32768)
			smp = -32768;

		buffer[i] = smp;
	}
}

#ifdef CONSOLE_SOUND
static void Update_consol_sound_rf(int set)
{
#ifdef VOL_ONLY_SOUND
	static int prev_atari_speaker = 0;
	static unsigned int prev_cpu_clock = 0;
	int d;
#ifdef __PLUS
	if (!g_Sound.nDigitized)
		return;
#endif
	if (!POKEYSND_console_sound_enabled)
		return;

	if (!set && POKEYSND_samp_consol_val == 0)
		return;
	POKEYSND_sampbuf_lastval -= POKEYSND_samp_consol_val;
	if (prev_atari_speaker != GTIA_speaker) {
		POKEYSND_samp_consol_val = GTIA_speaker * 8 * 4;	/* gain */
		prev_cpu_clock = ANTIC_CPU_CLOCK;
	}
	else if (!set) {
		d = ANTIC_CPU_CLOCK - prev_cpu_clock;
		if (d < 114) {
			POKEYSND_sampbuf_lastval += POKEYSND_samp_consol_val;
			return;
		}
		while (d >= 114 /* CPUL */) {
			POKEYSND_samp_consol_val = POKEYSND_samp_consol_val * 99 / 100;
			d -= 114;
		}
		prev_cpu_clock = ANTIC_CPU_CLOCK - d;
	}
	POKEYSND_sampbuf_lastval += POKEYSND_samp_consol_val;
	prev_atari_speaker = GTIA_speaker;

	POKEYSND_sampbuf_val[POKEYSND_sampbuf_ptr] = POKEYSND_sampbuf_lastval;
	POKEYSND_sampbuf_cnt[POKEYSND_sampbuf_ptr] =
		(ANTIC_CPU_CLOCK - POKEYSND_sampbuf_last) * 128 * POKEYSND_samp_freq / 178979;
	POKEYSND_sampbuf_last = ANTIC_CPU_CLOCK;
	POKEYSND_sampbuf_ptr++;
	if (POKEYSND_sampbuf_ptr >= POKEYSND_SAMPBUF_MAX)
		POKEYSND_sampbuf_ptr = 0;
	if (POKEYSND_sampbuf_ptr == POKEYSND_sampbuf_rptr) {
		POKEYSND_sampbuf_rptr++;
		if (POKEYSND_sampbuf_rptr >= POKEYSND_SAMPBUF_MAX)
			POKEYSND_sampbuf_rptr = 0;
	}
#endif  /* VOL_ONLY_SOUND */
}
#endif /* CONSOLE_SOUND */

#ifdef VOL_ONLY_SOUND
static void Update_vol_only_sound_rf(void)
{
#ifdef CONSOLE_SOUND
	POKEYSND_UpdateConsol(0);	/* mmm */
#endif /* CONSOLE_SOUND */
}
#endif  /* VOL_ONLY_SOUND */
