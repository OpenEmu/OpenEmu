/*****************************************************************************/
/*                                                                           */
/* Module:  POKEY Chip Simulator Includes, V2.3                              */
/* Purpose: To emulate the sound generation hardware of the Atari POKEY chip.*/
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
/* Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.   */
/*                                                                           */
/* Any permitted reproduction of these routines, in whole or in part, must   */
/* bear this legend.                                                         */
/*                                                                           */
/*****************************************************************************/

#ifndef _POKEYSOUND_H
#define _POKEYSOUND_H

#ifndef _TYPEDEF_H
#define _TYPEDEF_H

/* define some data types to keep it platform independent */
#ifdef COMP16                 /* if 16-bit compiler defined */
#define int8  char
#define int16 int
#define int32 long
#else                         /* else default to 32-bit compiler */
#define int8  char
#define int16 short
#define int32 int
#endif

#define uint8  unsigned int8
#define uint16 unsigned int16
#define uint32 unsigned int32

#ifdef __MWERKS__
#define BIG_ENDIAN
#endif

#endif

/* CONSTANT DEFINITIONS */

/* POKEY WRITE LOGICALS */
/* Note: only 0x00 - 0x09 are emulated by POKEYSND */
#define AUDF1_C     0x00
#define AUDC1_C     0x01
#define AUDF2_C     0x02
#define AUDC2_C     0x03
#define AUDF3_C     0x04
#define AUDC3_C     0x05
#define AUDF4_C     0x06
#define AUDC4_C     0x07
#define AUDCTL_C    0x08
#define STIMER_C    0x09
#define SKREST_C    0x0A
#define POTGO_C     0x0B
#define SEROUT_C    0x0D
#define IRQEN_C     0x0E
#define SKCTL_C     0x0F

/* POKEY READ LOGICALS */
#define POT0_C      0x00
#define POT1_C      0x01
#define POT2_C      0x02
#define POT3_C      0x03
#define POT4_C      0x04
#define POT5_C      0x05
#define POT6_C      0x06
#define POT7_C      0x07
#define ALLPOT_C    0x08
#define KBCODE_C    0x09
#define RANDOM_C    0x0A
#define SERIN_C     0x0D
#define IRQST_C     0x0E
#define SKSTAT_C    0x0F


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

#define FREQ_17_EXACT     1789790  /* exact 1.79 MHz clock freq */
#define FREQ_17_APPROX    1787520  /* approximate 1.79 MHz clock freq */

#define MAXPOKEYS         4        /* max number of emulated chips */

#ifdef __cplusplus
extern "C" {
#endif

/* #define SIGNED_SAMPLES */            /* define for signed output */
/* #define CLIP           */            /* required to force clipping */

#ifdef  SIGNED_SAMPLES             /* if signed output selected */
#define SAMP_MAX 127               /* then set signed 8-bit clipping ranges */
#define SAMP_MIN -128
#define SAMP_MID 0
#else
#define SAMP_MAX 255               /* else set unsigned 8-bit clip ranges */
#define SAMP_MIN 0
#define SAMP_MID 128
#endif

void Pokey_sound_init (uint32 freq17, uint16 playback_freq,
                       uint8 num_pokeys);
void Pokey_sound_update_freq (uint32 freq17, uint16 playback_freq);
void Update_pokey_sound (uint16 addr, uint8 val, uint8 chip, uint8 gain);
void Pokey_process (register uint8 *buffer, register uint16 n);

#ifdef __cplusplus
}
#endif

#endif
