/*****************************************************************************/
/*                                                                           */
/* Module:  POKEY Chip Simulator Includes, V2.3                              */
/* Purpose: To emulate the sound generation hardware of the Atari POKEY chip. */
/* Author:  Ron Fries                                                        */
/*                                                                           */
/* Revision History:                                                         */
/*                                                                           */
/* 09/22/96 - Ron Fries - Initial Release                                    */
/* 04/06/97 - Brad Oliver - Some cross-platform modifications. Added         */
/*                          big/little endian #defines, removed <dos.h>,     */
/*                          conditional defines for TRUE/FALSE               */
/* 01/19/98 - Ron Fries - Changed signed/unsigned sample support to a        */
/*                        compile-time option.  Defaults to unsigned -       */
/*                        define SIGNED_SAMPLES to create signed.            */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*                 License Information and Copyright Notice                  */
/*                 ========================================                  */
/*                                                                           */
/* PokeySound is Copyright(c) 1996-1998 by Ron Fries                         */
/*                                                                           */
/* This library is free software; you can redistribute it and/or modify it   */
/* under the terms of version 2 of the GNU Library General Public License    */
/* as published by the Free Software Foundation.                             */
/*                                                                           */
/* This library is distributed in the hope that it will be useful, but       */
/* WITHOUT ANY WARRANTY; without even the implied warranty of                */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library */
/* General Public License for more details.                                  */
/* To obtain a copy of the GNU Library General Public License, write to the  */
/* Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.   */
/*                                                                           */
/* Any permitted reproduction of these routines, in whole or in part, must   */
/* bear this legend.                                                         */
/*                                                                           */
/*****************************************************************************/

#ifndef POKEYSND_H_
#define POKEYSND_H_

#include "config.h"
#include "pokey.h"

/* CONSTANT DEFINITIONS */

/* As an alternative to using the exact frequencies, selecting a playback
   frequency that is an exact division of the main clock provides a higher
   quality output due to less aliasing.  For best results, a value of
   1787520 MHz is used for the main clock.  With this value, both the
   64 kHz and 15 kHz clocks are evenly divisible.  Selecting a playback
   frequency that is also a division of the clock provides the best
   results.  The best options are FREQ_64 divided by either 2, 3, or 4.
   The best selection is based on a trade off between performance and
   sound quality.

   Of course, using a main clock frequency that is not exact will affect
   the pitch of the output.  With these numbers, the pitch will be low
   by 0.127%.  (More than likely, an actual unit will vary by this much!) */

#define POKEYSND_FREQ_17_EXACT     1789790	/* exact 1.79 MHz clock freq */
#define POKEYSND_FREQ_17_APPROX    1787520	/* approximate 1.79 MHz clock freq */

#ifdef __cplusplus
extern "C" {
#endif

	/* #define SIGNED_SAMPLES */ /* define for signed output */

#ifdef  POKEYSND_SIGNED_SAMPLES			/* if signed output selected */
#define POKEYSND_SAMP_MAX 127			/* then set signed 8-bit clipping ranges */
#define POKEYSND_SAMP_MIN -128
#define POKEYSND_SAMP_MID 0
#else
#define POKEYSND_SAMP_MAX 255			/* else set unsigned 8-bit clip ranges */
#define POKEYSND_SAMP_MIN 0
#define POKEYSND_SAMP_MID 128
#endif

/* init flags */
#define POKEYSND_BIT16	1

extern SLONG POKEYSND_playback_freq;
extern UBYTE POKEYSND_num_pokeys;
extern int POKEYSND_snd_flags;

extern int POKEYSND_enable_new_pokey;
extern int POKEYSND_stereo_enabled;
extern int POKEYSND_serio_sound_enabled;
extern int POKEYSND_console_sound_enabled;
extern int POKEYSND_bienias_fix;

extern void (*POKEYSND_Process_ptr)(void *sndbuffer, int sndn);
extern void (*POKEYSND_Update)(UWORD addr, UBYTE val, UBYTE /*chip*/, UBYTE gain);
extern void (*POKEYSND_UpdateSerio)(int out, UBYTE data);
extern void (*POKEYSND_UpdateConsol)(int set);
extern void (*POKEYSND_UpdateVolOnly)(void);

int POKEYSND_Init(ULONG freq17, int playback_freq, UBYTE num_pokeys,
                     int flags
#ifdef __PLUS
                     , int clear_regs
#endif
                     );
void POKEYSND_Process(void *sndbuffer, int sndn);
int POKEYSND_DoInit(void);
void POKEYSND_SetMzQuality(int quality);

/* Volume only emulations declarations */
#ifdef VOL_ONLY_SOUND

#define	POKEYSND_SAMPBUF_MAX	2000
extern int	POKEYSND_sampbuf_val[POKEYSND_SAMPBUF_MAX];	/* volume values */
extern int	POKEYSND_sampbuf_cnt[POKEYSND_SAMPBUF_MAX];	/* relative start time */
extern int	POKEYSND_sampbuf_ptr;                    /* pointer to sampbuf */
extern int	POKEYSND_sampbuf_rptr;                   /* pointer to read from sampbuf */
extern int	POKEYSND_sampbuf_last;                   /* last absolute time */
extern int	POKEYSND_sampbuf_AUDV[4 * POKEY_MAXPOKEYS];	/* prev. channel volume */
extern int	POKEYSND_sampbuf_lastval;		/* last volume */
extern int	POKEYSND_sampout;			/* last out volume */
extern int	POKEYSND_samp_freq;
extern int	POKEYSND_samp_consol_val;		/* actual value of console sound */
#endif  /* VOL_ONLY_SOUND */

#ifdef __cplusplus
}

#endif
#endif /* POKEYSND_H_ */
