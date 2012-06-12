/*****************************************************************************/
/*                                                                           */
/* Module:  POKEY Chip Emulator, V2.3                                        */
/* Purpose: To emulate the sound generation hardware of the Atari POKEY chip.*/
/* Author:  Ron Fries                                                        */
/*                                                                           */
/* Revision History:                                                         */
/*                                                                           */
/* 09/22/96 - Ron Fries - Initial Release                                    */
/* 01/14/97 - Ron Fries - Corrected a minor problem to improve sound quality */
/*                        Also changed names from POKEY11.x to POKEY.x       */
/* 01/17/97 - Ron Fries - Added support for multiple POKEY chips.            */
/* 03/31/97 - Ron Fries - Made some minor mods for MAME (changed to signed   */
/*                        8-bit sample, increased gain range, removed        */
/*                        _disable() and _enable().)                         */
/* 04/06/97 - Brad Oliver - Some cross-platform modifications. Added         */
/*                          big/little endian #defines, removed <dos.h>,     */
/*                          conditional defines for TRUE/FALSE               */
/* 01/19/98 - Ron Fries - Changed signed/unsigned sample support to a        */
/*                        compile-time option.  Defaults to unsigned -       */
/*                        define SIGNED_SAMPLES to create signed.            */
/*                                                                           */
/* V2.0 Detailed Changes                                                     */
/* ---------------------                                                     */
/*                                                                           */
/* Now maintains both a POLY9 and POLY17 counter.  Though this slows the     */
/* emulator in general, it was required to support mutiple POKEYs since      */
/* each chip can individually select POLY9 or POLY17 operation.  Also,       */
/* eliminated the Poly17_size variable.                                      */
/*                                                                           */
/* Changed address of POKEY chip.  In the original, the chip was fixed at    */
/* location D200 for compatibility with the Atari 800 line of 8-bit          */
/* computers. The update function now only examines the lower four bits, so  */
/* the location for all emulated chips is effectively xxx0 - xxx8.           */
/*                                                                           */
/* The Update_pokey_sound function has two additional parameters which       */
/* selects the desired chip and selects the desired gain.                    */
/*                                                                           */
/* Added clipping to reduce distortion, configurable at compile-time.        */
/*                                                                           */
/* The Pokey_sound_init function has an additional parameter which selects   */
/* the number of pokey chips to emulate.                                     */
/*                                                                           */
/* The output will be amplified by gain/16.  If the output exceeds the       */
/* maximum value after the gain, it will be limited to reduce distortion.    */
/* The best value for the gain depends on the number of POKEYs emulated      */
/* and the maximum volume used.  The maximum possible output for each        */
/* channel is 15, making the maximum possible output for a single chip to    */
/* be 60.  Assuming all four channels on the chip are used at full volume,   */
/* a gain of 64 can be used without distortion.  If 4 POKEY chips are        */
/* emulated and all 16 channels are used at full volume, the gain must be    */
/* no more than 16 to prevent distortion.  Of course, if only a few of the   */
/* 16 channels are used or not all channels are used at full volume, a       */
/* larger gain can be used.                                                  */
/*                                                                           */
/* The Pokey_process routine automatically processes and mixes all selected  */
/* chips/channels.  No additional calls or functions are required.           */
/*                                                                           */
/* The unoptimized Pokey_process2() function has been removed.               */
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pokeysound.h"

extern unsigned int pokey_cpu_clock;

/* CONSTANT DEFINITIONS */

/* definitions for AUDCx (D201, D203, D205, D207) */
#define NOTPOLY5    0x80     /* selects POLY5 or direct CLOCK */
#define POLY4       0x40     /* selects POLY4 or POLY17 */
#define PURE        0x20     /* selects POLY4/17 or PURE tone */
#define VOL_ONLY    0x10     /* selects VOLUME OUTPUT ONLY */
#define VOLUME_MASK 0x0f     /* volume mask */

/* definitions for AUDCTL (D208) */
#define POLY9       0x80     /* selects POLY9 or POLY17 */
#define CH1_179     0x40     /* selects 1.78979 MHz for Ch 1 */
#define CH3_179     0x20     /* selects 1.78979 MHz for Ch 3 */
#define CH1_CH2     0x10     /* clocks channel 1 w/channel 2 */
#define CH3_CH4     0x08     /* clocks channel 3 w/channel 4 */
#define CH1_FILTER  0x04     /* selects channel 1 high pass filter */
#define CH2_FILTER  0x02     /* selects channel 2 high pass filter */
#define CLOCK_15    0x01     /* selects 15.6999kHz or 63.9210kHz */

/* for accuracy, the 64kHz and 15kHz clocks are exact divisions of
   the 1.79MHz clock */
#define DIV_64      28       /* divisor for 1.79MHz clock to 64 kHz */
#define DIV_15      114      /* divisor for 1.79MHz clock to 15 kHz */

/* the size (in entries) of the 4 polynomial tables */
#define POLY4_SIZE  0x000f
#define POLY5_SIZE  0x001f
#define POLY9_SIZE  0x01ff

#ifdef COMP16                  /* if 16-bit compiler */
#define POLY17_SIZE 0x00007fffL    /* reduce to 15 bits for simplicity */
#else
#define POLY17_SIZE 0x0001ffffL    /* else use the full 17 bits */
#endif

/* channel/chip definitions */
#define CHAN1       0
#define CHAN2       1
#define CHAN3       2
#define CHAN4       3
#define CHIP1       0
#define CHIP2       4
#define CHIP3       8
#define CHIP4      12
#define SAMPLE    127

// LBO - changed for cross-platform support
#ifndef FALSE
#define FALSE       0
#endif
#ifndef TRUE
#define TRUE        1
#endif

/* GLOBAL VARIABLE DEFINITIONS */

/* number of pokey chips currently emulated */
static uint8 Num_pokeys;

/* structures to hold the 9 pokey control bytes */
static uint8 AUDF[4 * MAXPOKEYS];   /* AUDFx (D200, D202, D204, D206) */
static uint8 AUDC[4 * MAXPOKEYS];   /* AUDCx (D201, D203, D205, D207) */
static uint8 AUDCTL[MAXPOKEYS];     /* AUDCTL (D208) */

static uint8 AUDV[4 * MAXPOKEYS];   /* Channel volume - derived */

static uint8 Outbit[4 * MAXPOKEYS]; /* current state of the output (high or low) */

static uint8 Outvol[4 * MAXPOKEYS]; /* last output volume for each channel */


/* Initialze the bit patterns for the polynomials. */

/* The 4bit and 5bit patterns are the identical ones used in the pokey chip. */
/* Though the patterns could be packed with 8 bits per byte, using only a */
/* single bit per byte keeps the math simple, which is important for */
/* efficient processing. */

static uint8 bit4[POLY4_SIZE] =
      { 1,1,0,1,1,1,0,0,0,0,1,0,1,0,0 };

static uint8 bit5[POLY5_SIZE] =
      { 0,0,1,1,0,0,0,1,1,1,1,0,0,1,0,1,0,1,1,0,1,1,1,0,1,0,0,0,0,0,1 };

static uint8 bit17[POLY17_SIZE];  /* Rather than have a table with 131071 */
                            /* entries, I use a random number generator. */
                            /* It shouldn't make much difference since */
                            /* the pattern rarely repeats anyway. */

static uint32 Poly_adjust; /* the amount that the polynomial will need */
                           /* to be adjusted to process the next bit */

static uint32 P4=0,   /* Global position pointer for the 4-bit  POLY array */
              P5=0,   /* Global position pointer for the 5-bit  POLY array */
              P9=0,   /* Global position pointer for the 9-bit  POLY array */
              P17=0;  /* Global position pointer for the 17-bit POLY array */

static uint32 Div_n_cnt[4 * MAXPOKEYS],   /* Divide by n counter. one for each channel */
              Div_n_max[4 * MAXPOKEYS];   /* Divide by n maximum, one for each channel */

static uint32 Samp_n_max,     /* Sample max.  For accuracy, it is *256 */
              Samp_n_cnt[2];  /* Sample cnt. */

static uint32 Base_mult[MAXPOKEYS]; /* selects either 64Khz or 15Khz clock mult */

#define	SAMPBUF_MAX	6000
int	sampbuf_val[SAMPBUF_MAX];	/* volume values */
int	sampbuf_cnt[SAMPBUF_MAX];	/* relative start time */
int	sampbuf_ptr = 0;		/* pointer to sampbuf */
int	sampbuf_rptr = 0;		/* pointer to read from sampbuf */
int	sampbuf_last = 0;		/* last absolute time */
int	sampbuf_AUDV[4 * MAXPOKEYS];	/* prev. channel volume */
int	sampbuf_lastval = 0;		/* last volume */
int	sampout;			/* last out volume */
uint16	samp_freq;
int	samp_consol_val=0;		/* actual value of console sound */

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
/* while (uint32 *)((uint8 *)(&Samp_n_cnt[0])+1) gives me the 32-bit whole   */
/* number only.                                                              */
/*                                                                           */
/* Representation on big-endian machines:                                    */
/* xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx | xxxxxxxx xxxxxxxx xxxxxxxx.xxxxxxxx */
/*  unused   unused   unused    whole      whole    whole    whole  fraction */
/*                                                                           */
/* Samp_n_cnt[1] gives me a 32-bit int 24 whole bits with 8 fractional bits, */
/* while (uint32 *)((uint8 *)(&Samp_n_cnt[0])+3) gives me the 32-bit whole   */
/* number only.                                                              */
/*****************************************************************************/


/*****************************************************************************/
/* Module:  Pokey_sound_init()                                               */
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

void Pokey_sound_init (uint32 freq17, uint16 playback_freq, uint8 num_pokeys)
{
   uint8 chan,chip;
   int32 n;

	samp_freq=playback_freq;

   /* fill the 17bit polynomial with random bits */
   for (n=0; n<POLY17_SIZE; n++)
   {
      bit17[n] = rand() & 0x01;       /* fill poly 17 with random bits */
   }

   /* disable interrupts to handle critical sections */
/*    _disable();*/     /* RSF - removed for portability 31-MAR-97 */

   /* start all of the polynomial counters at zero */
   Poly_adjust = 0;
   P4 = 0;
   P5 = 0;
   P9 = 0;
   P17 = 0;

   /* calculate the sample 'divide by N' value based on the playback freq. */
   Samp_n_max = ((uint32)freq17 << 8) / playback_freq;

   Samp_n_cnt[0] = 0;  /* initialize all bits of the sample */
   Samp_n_cnt[1] = 0;  /* 'divide by N' counter */

   for (chan = 0; chan < (MAXPOKEYS * 4); chan++)
   {
      Outvol[chan] = 0;
      Outbit[chan] = 0;
      Div_n_cnt[chan] = 0;
      Div_n_max[chan] = 0x7fffffffL;
      AUDC[chan] = 0;
      AUDF[chan] = 0;
      AUDV[chan] = 0;
	  sampbuf_AUDV[chan] = 0;
   }

   for (chip = 0; chip < MAXPOKEYS; chip++)
   {
      AUDCTL[chip] = 0;
      Base_mult[chip] = DIV_64;
   }

   /* set the number of pokey chips currently emulated */
   Num_pokeys = num_pokeys;

/*    _enable();*/      /* RSF - removed for portability 31-MAR-97 */
}

void Pokey_sound_update_freq (uint32 freq17, uint16 playback_freq) {
	samp_freq=playback_freq;
	Samp_n_max = ((uint32)freq17 << 8) / playback_freq;
}

/*****************************************************************************/
/* Module:  Update_pokey_sound()                                             */
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

void Update_pokey_sound (uint16 addr, uint8 val, uint8 chip, uint8 gain)
{
    uint32 new_val = 0;
    uint8 chan;
    uint8 chan_mask;
    uint8 chip_offs;

    /* disable interrupts to handle critical sections */
/*    _disable();*/     /* RSF - removed for portability 31-MAR-97 */

    /* calculate the chip_offs for the channel arrays */
    chip_offs = chip << 2;

    /* determine which address was changed */
    switch (addr & 0x0f)
    {
       case AUDF1_C:
          AUDF[CHAN1 + chip_offs] = val;
          chan_mask = 1 << CHAN1;

          if (AUDCTL[chip] & CH1_CH2)    /* if ch 1&2 tied together */
            chan_mask |= 1 << CHAN2;   /* then also change on ch2 */
          break;

       case AUDC1_C:
          AUDC[CHAN1 + chip_offs] = val;
               /* RSF - changed gain (removed >> 4) 31-MAR-97 */
          AUDV[CHAN1 + chip_offs] = (val & VOLUME_MASK) * gain;
          chan_mask = 1 << CHAN1;
          break;

       case AUDF2_C:
          AUDF[CHAN2 + chip_offs] = val;
          chan_mask = 1 << CHAN2;
          break;

       case AUDC2_C:
          AUDC[CHAN2 + chip_offs] = val;
               /* RSF - changed gain (removed >> 4) 31-MAR-97 */
          AUDV[CHAN2 + chip_offs] = (val & VOLUME_MASK) * gain;
          chan_mask = 1 << CHAN2;
          break;

       case AUDF3_C:
          AUDF[CHAN3 + chip_offs] = val;
          chan_mask = 1 << CHAN3;

          if (AUDCTL[chip] & CH3_CH4)   /* if ch 3&4 tied together */
             chan_mask |= 1 << CHAN4;  /* then also change on ch4 */
          break;

       case AUDC3_C:
          AUDC[CHAN3 + chip_offs] = val;
               /* RSF - changed gain (removed >> 4) 31-MAR-97 */
          AUDV[CHAN3 + chip_offs] = (val & VOLUME_MASK) * gain;
          chan_mask = 1 << CHAN3;
          break;

       case AUDF4_C:
          AUDF[CHAN4 + chip_offs] = val;
          chan_mask = 1 << CHAN4;
          break;

       case AUDC4_C:
          AUDC[CHAN4 + chip_offs] = val;
               /* RSF - changed gain (removed >> 4) 31-MAR-97 */
          AUDV[CHAN4 + chip_offs] = (val & VOLUME_MASK) * gain;
          chan_mask = 1 << CHAN4;
          break;

       case AUDCTL_C:
          AUDCTL[chip] = val;
          chan_mask = 15;       /* all channels */

          /* determine the base multiplier for the 'div by n' calculations */
          if (AUDCTL[chip] & CLOCK_15)
             Base_mult[chip] = DIV_15;
          else
             Base_mult[chip] = DIV_64;

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
    /*    1 MHz, 16-bit -    AUDF[CHAN1]+256*AUDF[CHAN2] + 7    */
    /************************************************************/

    /* only reset the channels that have changed */

    if (chan_mask & (1 << CHAN1))
    {
       /* process channel 1 frequency */
       if (AUDCTL[chip] & CH1_179)
          new_val = AUDF[CHAN1 + chip_offs] + 4;
       else
          new_val = (AUDF[CHAN1 + chip_offs] + 1) * Base_mult[chip];

       if (new_val != Div_n_max[CHAN1 + chip_offs])
       {
          Div_n_max[CHAN1 + chip_offs] = new_val;

          if (Div_n_cnt[CHAN1 + chip_offs] > new_val)
          {
             Div_n_cnt[CHAN1 + chip_offs] = new_val;
          }
       }
    }

    if (chan_mask & (1 << CHAN2))
    {
       /* process channel 2 frequency */
       if (AUDCTL[chip] & CH1_CH2)
          if (AUDCTL[chip] & CH1_179)
             new_val = AUDF[CHAN2 + chip_offs] * 256 +
                       AUDF[CHAN1 + chip_offs] + 7;
          else
             new_val = (AUDF[CHAN2 + chip_offs] * 256 +
                        AUDF[CHAN1 + chip_offs] + 1) * Base_mult[chip];
       else
          new_val = (AUDF[CHAN2 + chip_offs] + 1) * Base_mult[chip];

       if (new_val != Div_n_max[CHAN2 + chip_offs])
       {
          Div_n_max[CHAN2 + chip_offs] = new_val;

          if (Div_n_cnt[CHAN2 + chip_offs] > new_val)
          {
             Div_n_cnt[CHAN2 + chip_offs] = new_val;
          }
       }
    }

    if (chan_mask & (1 << CHAN3))
    {
       /* process channel 3 frequency */
       if (AUDCTL[chip] & CH3_179)
          new_val = AUDF[CHAN3 + chip_offs] + 4;
       else
          new_val= (AUDF[CHAN3 + chip_offs] + 1) * Base_mult[chip];

       if (new_val!= Div_n_max[CHAN3 + chip_offs])
       {
          Div_n_max[CHAN3 + chip_offs] = new_val;

          if (Div_n_cnt[CHAN3 + chip_offs] > new_val)
          {
             Div_n_cnt[CHAN3 + chip_offs] = new_val;
          }
       }
    }

    if (chan_mask & (1 << CHAN4))
    {
       /* process channel 4 frequency */
       if (AUDCTL[chip] & CH3_CH4)
          if (AUDCTL[chip] & CH3_179)
             new_val = AUDF[CHAN4 + chip_offs] * 256 +
                       AUDF[CHAN3 + chip_offs] + 7;
          else
             new_val = (AUDF[CHAN4 + chip_offs] * 256 +
                        AUDF[CHAN3 + chip_offs] + 1) * Base_mult[chip];
       else
          new_val = (AUDF[CHAN4 + chip_offs] + 1) * Base_mult[chip];

       if (new_val != Div_n_max[CHAN4 + chip_offs])
       {
          Div_n_max[CHAN4 + chip_offs] = new_val;

          if (Div_n_cnt[CHAN4 + chip_offs] > new_val)
          {
             Div_n_cnt[CHAN4 + chip_offs] = new_val;
          }
       }
    }

    /* if channel is volume only, set current output */
    for (chan = CHAN1; chan <= CHAN4; chan++)
    {
       if (chan_mask & (1 << chan))
       {
			if( (AUDC[chan + chip_offs] & VOL_ONLY) )
			{
				sampbuf_lastval+=AUDV[chan + chip_offs]
					-sampbuf_AUDV[chan + chip_offs];

				sampbuf_val[sampbuf_ptr]=sampbuf_lastval;
				sampbuf_AUDV[chan + chip_offs]=AUDV[chan + chip_offs];
				sampbuf_cnt[sampbuf_ptr]=
					(pokey_cpu_clock-sampbuf_last)*128*samp_freq/178979;
				sampbuf_last=pokey_cpu_clock;
				sampbuf_ptr++;
				if( sampbuf_ptr>=SAMPBUF_MAX )
					sampbuf_ptr=0;
				if( sampbuf_ptr==sampbuf_rptr )
				{	sampbuf_rptr++;
					if( sampbuf_rptr>=SAMPBUF_MAX )
						sampbuf_rptr=0;
				}
			}
          /* I've disabled any frequencies that exceed the sampling
             frequency.  There isn't much point in processing frequencies
             that the hardware can't reproduce.  I've also disabled
             processing if the volume is zero. */

          /* if the channel is volume only */
          /* or the channel is off (volume == 0) */
          /* or the channel freq is greater than the playback freq */
	      if ((AUDC[chan + chip_offs] & VOL_ONLY) ||
             ((AUDC[chan + chip_offs] & VOLUME_MASK) == 0) ||
              (Div_n_max[chan + chip_offs] < (Samp_n_max >> 8)))
	      //if ((AUDC[chan + chip_offs] & VOL_ONLY) ||
          //   ((AUDC[chan + chip_offs] & VOLUME_MASK) == 0))
          {
             /* indicate the channel is 'on' */
             Outvol[chan + chip_offs] = 1;

			if ((chan == CHAN3 && !(AUDCTL[chip] & CH1_FILTER)) ||
				(chan == CHAN4 && !(AUDCTL[chip] & CH2_FILTER)) ||
				(chan == CHAN1) ||
				(chan == CHAN2) ) {

                /* and set channel freq to max to reduce processing */
                Div_n_max[chan + chip_offs] = 0x7fffffffL;
                Div_n_cnt[chan + chip_offs] = 0x7fffffffL;
			 }
          }
       }
    }

/*    _enable();*/      /* RSF - removed for portability 31-MAR-97 */
}


/*****************************************************************************/
/* Module:  Pokey_process()                                                  */
/* Purpose: To fill the output buffer with the sound output based on the     */
/*          pokey chip parameters.                                           */
/*                                                                           */
/* Author:  Ron Fries                                                        */
/* Date:    January 1, 1997                                                  */
/*                                                                           */
/* Inputs:  *buffer - pointer to the buffer where the audio output will      */
/*                    be placed                                              */
/*          n - size of the playback buffer                                  */
/*          num_pokeys - number of currently active pokeys to process        */
/*                                                                           */
/* Outputs: the buffer will be filled with n bytes of audio - no return val  */
/*                                                                           */
/*****************************************************************************/

void Pokey_process (register uint8 *buffer, register uint16 n)
{
    register uint32 *div_n_ptr;
    register uint32 *samp_cnt_w_ptr;
    register uint32 event_min;
    register uint8 next_event;
#ifdef CLIP                    /* if clipping is selected */
    register int16 cur_val;    /* then we have to count as 16-bit signed */
#else                       
    register uint8 cur_val;    /* otherwise we'll simplify as 8-bit unsigned */
#endif
    register uint8 *out_ptr;
    register uint8 audc;
    register uint8 toggle;
    register uint8 count;
    register uint8 *vol_ptr;

    /* set a pointer to the whole portion of the samp_n_cnt */
#ifdef BIG_ENDIAN
    samp_cnt_w_ptr = (uint32 *)((uint8 *)(&Samp_n_cnt[0])+3);
#else
    samp_cnt_w_ptr = (uint32 *)((uint8 *)(&Samp_n_cnt[0])+1);
#endif

    /* set a pointer for optimization */
    out_ptr = Outvol;
    vol_ptr = AUDV;

    /* The current output is pre-determined and then adjusted based on each */
    /* output change for increased performance (less over-all math). */
    /* add the output values of all 4 channels */
    cur_val = SAMP_MID;

    count = Num_pokeys;
    do
    {
       cur_val -= *vol_ptr/2;
       if (*out_ptr++)
         cur_val += *vol_ptr;
       vol_ptr++;

       cur_val -= *vol_ptr/2;
       if (*out_ptr++)
         cur_val += *vol_ptr;
       vol_ptr++;

       cur_val -= *vol_ptr/2;
       if (*out_ptr++)
         cur_val += *vol_ptr;
       vol_ptr++;

       cur_val -= *vol_ptr/2;
       if (*out_ptr++)
         cur_val += *vol_ptr;
       vol_ptr++;

       count--;
    } while (count);

    /* loop until the buffer is filled */
    while (n)
    {
       /* Normally the routine would simply decrement the 'div by N' */
       /* counters and react when they reach zero.  Since we normally */
       /* won't be processing except once every 80 or so counts, */
       /* I've optimized by finding the smallest count and then */
       /* 'accelerated' time by adjusting all pointers by that amount. */

       /* find next smallest event (either sample or chan 1-4) */
       next_event = SAMPLE;
       event_min = *samp_cnt_w_ptr;

       div_n_ptr = Div_n_cnt;

       count = 0;
       do
       {
          /* Though I could have used a loop here, this is faster */
          if (*div_n_ptr <= event_min)
          {
             event_min = *div_n_ptr;
             next_event = CHAN1 + (count<<2);
          }
          div_n_ptr++;
          if (*div_n_ptr <= event_min)
          {
             event_min = *div_n_ptr;
             next_event = CHAN2 + (count<<2);
          }
          div_n_ptr++;
          if (*div_n_ptr <= event_min)
          {
             event_min = *div_n_ptr;
             next_event = CHAN3 + (count<<2);
          }
          div_n_ptr++;
          if (*div_n_ptr <= event_min)
          {
             event_min = *div_n_ptr;
             next_event = CHAN4 + (count<<2);
          }
          div_n_ptr++;

          count++;
       } while (count < Num_pokeys);

       /* if the next event is a channel change */
       if (next_event != SAMPLE)
       {

          count = Num_pokeys;
          do
          {
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

          *samp_cnt_w_ptr -= event_min;

          /* since the polynomials require a mod (%) function which is
             division, I don't adjust the polynomials on the SAMPLE events,
             only the CHAN events.  I have to keep track of the change,
             though. */
          Poly_adjust += event_min;

          /* shift the polynomial counters */
          P4  = (P4  + Poly_adjust) % POLY4_SIZE;
          P5  = (P5  + Poly_adjust) % POLY5_SIZE;
          P9  = (P9  + Poly_adjust) % POLY9_SIZE;
          P17 = (P17 + Poly_adjust) % POLY17_SIZE;

          /* reset the polynomial adjust counter to zero */
          Poly_adjust = 0;

          /* adjust channel counter */
          Div_n_cnt[next_event] += Div_n_max[next_event];

          /* get the current AUDC into a register (for optimization) */
          audc = AUDC[next_event];

          /* set a pointer to the current output (for opt...) */
          out_ptr = &Outvol[next_event];

          /* assume no changes to the output */
          toggle = FALSE;

          /* From here, a good understanding of the hardware is required */
          /* to understand what is happening.  I won't be able to provide */
          /* much description to explain it here. */

          /* if VOLUME only then nothing to process */
          if (!(audc & VOL_ONLY)) {

          /* if the output is pure or the output is poly5 and the poly5 bit */
          /* is set */
          if ((audc & NOTPOLY5) || bit5[P5])
          {
             /* if the PURE bit is set */
             if (audc & PURE)
             {
                /* then simply toggle the output */
                toggle = TRUE;
             }
             /* otherwise if POLY4 is selected */
             else if (audc & POLY4)
             {
                /* then compare to the poly4 bit */
                toggle = (bit4[P4] == !(*out_ptr));
             }
             else
             {
                /* if 9-bit poly is selected on this chip */
                if (AUDCTL[next_event >> 2] & POLY9)
                {
                   /* compare to the poly9 bit */
                   toggle = (bit17[P9] == !(*out_ptr));
                }
                else
                {
                   /* otherwise compare to the poly17 bit */
                   toggle = (bit17[P17] == !(*out_ptr));
                }
             }
          }
		  }

			/* check channel 1 filter (clocked by channel 3) */
			if ( AUDCTL[next_event >> 2] & CH1_FILTER) {
				/* if we're processing channel 3 */
				if ((next_event & 0x03) == CHAN3) {
					/* check output of channel 1 on same chip */
					if (Outvol[next_event & 0xfd]) {
						/* if on, turn it off */
						Outvol[next_event & 0xfd] = 0;
						cur_val -= AUDV[next_event & 0xfd];
					}
				}
			}

			/* check channel 2 filter (clocked by channel 4) */
			if ( AUDCTL[next_event >> 2] & CH2_FILTER) {
				/* if we're processing channel 4 */
				if ((next_event & 0x03) == CHAN4) {
					/* check output of channel 2 on same chip */
					if (Outvol[next_event & 0xfd]) {
						/* if on, turn it off */
						Outvol[next_event & 0xfd] = 0;
						cur_val -= AUDV[next_event & 0xfd];
					}
				}
			}

          /* if the current output bit has changed */
          if (toggle)
          {
             if (*out_ptr)
             {
                /* remove this channel from the signal */
                cur_val -= AUDV[next_event];

                /* and turn the output off */
                *out_ptr = 0;
             }
             else
             {
                /* turn the output on */
                *out_ptr = 1;

                /* and add it to the output signal */
                cur_val += AUDV[next_event];
             }
          }
       }
       else /* otherwise we're processing a sample */
       {
          /* adjust the sample counter - note we're using the 24.8 integer
             which includes an 8 bit fraction for accuracy */
		  int iout;
		  iout = cur_val;

			if( sampbuf_rptr!=sampbuf_ptr )
			{ int l;
				if( sampbuf_cnt[sampbuf_rptr]>0 )
					sampbuf_cnt[sampbuf_rptr]-=1280;
				while(  (l=sampbuf_cnt[sampbuf_rptr])<=0 )
				{	sampout=sampbuf_val[sampbuf_rptr];
					sampbuf_rptr++;
					if( sampbuf_rptr>=SAMPBUF_MAX )
						sampbuf_rptr=0;
					if( sampbuf_rptr!=sampbuf_ptr )
					{
					    sampbuf_cnt[sampbuf_rptr]+=l;
					}
					else	break;
				}
			}
			iout+=sampout;

#ifdef BIG_ENDIAN
          *(Samp_n_cnt + 1) += Samp_n_max;
#else
          *Samp_n_cnt += Samp_n_max;
#endif

#ifdef CLIP                               /* if clipping is selected */
          if (iout > SAMP_MAX)         /* then check high limit */
          {
             *buffer++ = (uint8)SAMP_MAX; /* and limit if greater */
          }
          else if (iout < SAMP_MIN)    /* else check low limit */
          {
             *buffer++ = (uint8)SAMP_MIN; /* and limit if less */
          }
          else                            /* otherwise use raw value */
          {
             *buffer++ = (uint8)iout;
          }
#else
          *buffer++ = (uint8)iout;     /* clipping not selected, use value */
#endif

          /* and indicate one less byte in the buffer */
          n--;
       }
    }

	if( sampbuf_rptr==sampbuf_ptr )
		sampbuf_last=pokey_cpu_clock;
}
