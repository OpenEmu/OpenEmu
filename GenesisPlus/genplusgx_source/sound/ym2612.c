/*
**
** software implementation of Yamaha FM sound generator (YM2612/YM3438)
**
** Original code (MAME fm.c)
**
** Copyright (C) 2001, 2002, 2003 Jarek Burczynski (bujar at mame dot net)
** Copyright (C) 1998 Tatsuyuki Satoh , MultiArcadeMachineEmulator development
**
** Version 1.4 (final beta) 
**
** Additional code & fixes by Eke-Eke for Genesis Plus GX
**
** Huge thanks to Nemesis, most of those fixes came from his tests on Sega Genesis hardware
** More informations at http://gendev.spritesmind.net/forum/viewtopic.php?t=386
**
**  TODO:
**  - better documentation
**  - BUSY flag emulation
*/

/*
**  CHANGELOG:
**
** 01-09-2012 Eke-Eke (Genesis Plus GX):
**  - removed input clock / output samplerate frequency ratio, chip now always run at (original) internal sample frequency
**  - removed now uneeded extra bits of precision
**
** 2006~2012  Eke-Eke (Genesis Plus GX):
**  - removed unused multichip support
**  - added YM2612 Context external access functions
**  - fixed LFO implementation:
**      .added support for CH3 special mode: fixes various sound effects (birds in Warlock, bug sound in Aladdin...)
**      .inverted LFO AM waveform: fixes Spider-Man & Venom : Separation Anxiety (intro), California Games (surfing event)
**      .improved LFO timing accuracy: now updated AFTER sample output, like EG/PG updates, and without any precision loss anymore.
**  - improved internal timers emulation
**  - adjusted lowest EG rates increment values
**  - fixed Attack Rate not being updated in some specific cases (Batman & Robin intro)
**  - fixed EG behavior when Attack Rate is maximal
**  - fixed EG behavior when SL=0 (Mega Turrican tracks 03,09...) or/and Key ON occurs at minimal attenuation 
**  - implemented EG output immediate changes on register writes
**  - fixed YM2612 initial values (after the reset): fixes missing intro in B.O.B
**  - implemented Detune overflow (Ariel, Comix Zone, Shaq Fu, Spiderman & many other games using GEMS sound engine)
**  - implemented accurate CSM mode emulation
**  - implemented accurate SSG-EG emulation (Asterix, Beavis&Butthead, Bubba'n Stix & many other games)
**  - implemented accurate address/data ports behavior
**  - added preliminar support for DAC precision
**
** 03-08-2003 Jarek Burczynski:
**  - fixed YM2608 initial values (after the reset)
**  - fixed flag and irqmask handling (YM2608)
**  - fixed BUFRDY flag handling (YM2608)
**
** 14-06-2003 Jarek Burczynski:
**  - implemented all of the YM2608 status register flags
**  - implemented support for external memory read/write via YM2608
**  - implemented support for deltat memory limit register in YM2608 emulation
**
** 22-05-2003 Jarek Burczynski:
**  - fixed LFO PM calculations (copy&paste bugfix)
**
** 08-05-2003 Jarek Burczynski:
**  - fixed SSG support
**
** 22-04-2003 Jarek Burczynski:
**  - implemented 100% correct LFO generator (verified on real YM2610 and YM2608)
**
** 15-04-2003 Jarek Burczynski:
**  - added support for YM2608's register 0x110 - status mask
**
** 01-12-2002 Jarek Burczynski:
**  - fixed register addressing in YM2608, YM2610, YM2610B chips. (verified on real YM2608)
**    The addressing patch used for early Neo-Geo games can be removed now.
**
** 26-11-2002 Jarek Burczynski, Nicola Salmoria:
**  - recreated YM2608 ADPCM ROM using data from real YM2608's output which leads to:
**  - added emulation of YM2608 drums.
**  - output of YM2608 is two times lower now - same as YM2610 (verified on real YM2608)
**
** 16-08-2002 Jarek Burczynski:
**  - binary exact Envelope Generator (verified on real YM2203);
**    identical to YM2151
**  - corrected 'off by one' error in feedback calculations (when feedback is off)
**  - corrected connection (algorithm) calculation (verified on real YM2203 and YM2610)
**
** 18-12-2001 Jarek Burczynski:
**  - added SSG-EG support (verified on real YM2203)
**
** 12-08-2001 Jarek Burczynski:
**  - corrected sin_tab and tl_tab data (verified on real chip)
**  - corrected feedback calculations (verified on real chip)
**  - corrected phase generator calculations (verified on real chip)
**  - corrected envelope generator calculations (verified on real chip)
**  - corrected FM volume level (YM2610 and YM2610B).
**  - changed YMxxxUpdateOne() functions (YM2203, YM2608, YM2610, YM2610B, YM2612) :
**    this was needed to calculate YM2610 FM channels output correctly.
**    (Each FM channel is calculated as in other chips, but the output of the channel
**    gets shifted right by one *before* sending to accumulator. That was impossible to do
**    with previous implementation).
**
** 23-07-2001 Jarek Burczynski, Nicola Salmoria:
**  - corrected YM2610 ADPCM type A algorithm and tables (verified on real chip)
**
** 11-06-2001 Jarek Burczynski:
**  - corrected end of sample bug in ADPCMA_calc_cha().
**    Real YM2610 checks for equality between current and end addresses (only 20 LSB bits).
**
** 08-12-98 hiro-shi:
** rename ADPCMA -> ADPCMB, ADPCMB -> ADPCMA
** move ROM limit check.(CALC_CH? -> 2610Write1/2)
** test program (ADPCMB_TEST)
** move ADPCM A/B end check.
** ADPCMB repeat flag(no check)
** change ADPCM volume rate (8->16) (32->48).
**
** 09-12-98 hiro-shi:
** change ADPCM volume. (8->16, 48->64)
** replace ym2610 ch0/3 (YM-2610B)
** change ADPCM_SHIFT (10->8) missing bank change 0x4000-0xffff.
** add ADPCM_SHIFT_MASK
** change ADPCMA_DECODE_MIN/MAX.
*/

/************************************************************************/
/*    comment of hiro-shi(Hiromitsu Shioya)                             */
/*    YM2610(B) = OPN-B                                                 */
/*    YM2610  : PSG:3ch FM:4ch ADPCM(18.5KHz):6ch DeltaT ADPCM:1ch      */
/*    YM2610B : PSG:3ch FM:6ch ADPCM(18.5KHz):6ch DeltaT ADPCM:1ch      */
/************************************************************************/

#include "shared.h"

/* envelope generator */
#define ENV_BITS    10
#define ENV_LEN      (1<<ENV_BITS)
#define ENV_STEP    (128.0/ENV_LEN)

#define MAX_ATT_INDEX  (ENV_LEN-1) /* 1023 */
#define MIN_ATT_INDEX  (0)      /* 0 */

#define EG_ATT      4
#define EG_DEC      3
#define EG_SUS      2
#define EG_REL      1
#define EG_OFF      0

/* phase generator (detune mask) */
#define DT_BITS     17
#define DT_LEN      (1 << DT_BITS)
#define DT_MASK     (DT_LEN - 1)

/* operator unit */
#define SIN_BITS    10
#define SIN_LEN      (1<<SIN_BITS)
#define SIN_MASK    (SIN_LEN-1)

#define TL_RES_LEN    (256) /* 8 bits addressing (real chip) */

#define TL_BITS    14 /* channel output */

/*  TL_TAB_LEN is calculated as:
*   13 - sinus amplitude bits     (Y axis)
*   2  - sinus sign bit           (Y axis)
*   TL_RES_LEN - sinus resolution (X axis)
*/
#define TL_TAB_LEN (13*2*TL_RES_LEN)
static signed int tl_tab[TL_TAB_LEN];

#define ENV_QUIET    (TL_TAB_LEN>>3)

/* sin waveform table in 'decibel' scale */
static unsigned int sin_tab[SIN_LEN];

/* sustain level table (3dB per step) */
/* bit0, bit1, bit2, bit3, bit4, bit5, bit6 */
/* 1,    2,    4,    8,    16,   32,   64   (value)*/
/* 0.75, 1.5,  3,    6,    12,   24,   48   (dB)*/

/* 0 - 15: 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,93 (dB)*/
/* attenuation value (10 bits) = (SL << 2) << 3 */
#define SC(db) (UINT32) ( db * (4.0/ENV_STEP) )
static const UINT32 sl_table[16]={
 SC( 0),SC( 1),SC( 2),SC(3 ),SC(4 ),SC(5 ),SC(6 ),SC( 7),
 SC( 8),SC( 9),SC(10),SC(11),SC(12),SC(13),SC(14),SC(31)
};
#undef SC


#define RATE_STEPS (8)
static const UINT8 eg_inc[19*RATE_STEPS]={

/*cycle:0 1  2 3  4 5  6 7*/

/* 0 */ 0,1, 0,1, 0,1, 0,1, /* rates 00..11 0 (increment by 0 or 1) */
/* 1 */ 0,1, 0,1, 1,1, 0,1, /* rates 00..11 1 */
/* 2 */ 0,1, 1,1, 0,1, 1,1, /* rates 00..11 2 */
/* 3 */ 0,1, 1,1, 1,1, 1,1, /* rates 00..11 3 */

/* 4 */ 1,1, 1,1, 1,1, 1,1, /* rate 12 0 (increment by 1) */
/* 5 */ 1,1, 1,2, 1,1, 1,2, /* rate 12 1 */
/* 6 */ 1,2, 1,2, 1,2, 1,2, /* rate 12 2 */
/* 7 */ 1,2, 2,2, 1,2, 2,2, /* rate 12 3 */

/* 8 */ 2,2, 2,2, 2,2, 2,2, /* rate 13 0 (increment by 2) */
/* 9 */ 2,2, 2,4, 2,2, 2,4, /* rate 13 1 */
/*10 */ 2,4, 2,4, 2,4, 2,4, /* rate 13 2 */
/*11 */ 2,4, 4,4, 2,4, 4,4, /* rate 13 3 */

/*12 */ 4,4, 4,4, 4,4, 4,4, /* rate 14 0 (increment by 4) */
/*13 */ 4,4, 4,8, 4,4, 4,8, /* rate 14 1 */
/*14 */ 4,8, 4,8, 4,8, 4,8, /* rate 14 2 */
/*15 */ 4,8, 8,8, 4,8, 8,8, /* rate 14 3 */

/*16 */ 8,8, 8,8, 8,8, 8,8, /* rates 15 0, 15 1, 15 2, 15 3 (increment by 8) */
/*17 */ 16,16,16,16,16,16,16,16, /* rates 15 2, 15 3 for attack */
/*18 */ 0,0, 0,0, 0,0, 0,0, /* infinity rates for attack and decay(s) */
};


#define O(a) (a*RATE_STEPS)

/*note that there is no O(17) in this table - it's directly in the code */
static const UINT8 eg_rate_select[32+64+32]={  /* Envelope Generator rates (32 + 64 rates + 32 RKS) */
/* 32 infinite time rates (same as Rate 0) */
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),
O(18),O(18),O(18),O(18),O(18),O(18),O(18),O(18),

/* rates 00-11 */
/*
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
*/
O(18),O(18),O( 0),O( 0),
O( 0),O( 0),O( 2),O( 2),   /* Nemesis's tests */

O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),
O( 0),O( 1),O( 2),O( 3),

/* rate 12 */
O( 4),O( 5),O( 6),O( 7),

/* rate 13 */
O( 8),O( 9),O(10),O(11),

/* rate 14 */
O(12),O(13),O(14),O(15),

/* rate 15 */
O(16),O(16),O(16),O(16),

/* 32 dummy rates (same as 15 3) */
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16),
O(16),O(16),O(16),O(16),O(16),O(16),O(16),O(16)

};
#undef O

/*rate  0,    1,    2,   3,   4,   5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15*/
/*shift 11,   10,   9,   8,   7,   6,  5,  4,  3,  2, 1,  0,  0,  0,  0,  0 */
/*mask  2047, 1023, 511, 255, 127, 63, 31, 15, 7,  3, 1,  0,  0,  0,  0,  0 */

#define O(a) (a*1)
static const UINT8 eg_rate_shift[32+64+32]={  /* Envelope Generator counter shifts (32 + 64 rates + 32 RKS) */
/* 32 infinite time rates */
/* O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0),
O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0),
O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0),
O(0),O(0),O(0),O(0),O(0),O(0),O(0),O(0), */

/* fixed (should be the same as rate 0, even if it makes no difference since increment value is 0 for these rates) */
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),
O(11),O(11),O(11),O(11),O(11),O(11),O(11),O(11),

/* rates 00-11 */
O(11),O(11),O(11),O(11),
O(10),O(10),O(10),O(10),
O( 9),O( 9),O( 9),O( 9),
O( 8),O( 8),O( 8),O( 8),
O( 7),O( 7),O( 7),O( 7),
O( 6),O( 6),O( 6),O( 6),
O( 5),O( 5),O( 5),O( 5),
O( 4),O( 4),O( 4),O( 4),
O( 3),O( 3),O( 3),O( 3),
O( 2),O( 2),O( 2),O( 2),
O( 1),O( 1),O( 1),O( 1),
O( 0),O( 0),O( 0),O( 0),

/* rate 12 */
O( 0),O( 0),O( 0),O( 0),

/* rate 13 */
O( 0),O( 0),O( 0),O( 0),

/* rate 14 */
O( 0),O( 0),O( 0),O( 0),

/* rate 15 */
O( 0),O( 0),O( 0),O( 0),

/* 32 dummy rates (same as 15 3) */
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),
O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0),O( 0)

};
#undef O

static const UINT8 dt_tab[4 * 32]={
/* this is YM2151 and YM2612 phase increment data (in 10.10 fixed point format)*/
/* FD=0 */
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* FD=1 */
  0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
  2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 8, 8, 8,
/* FD=2 */
  1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5,
  5, 6, 6, 7, 8, 8, 9,10,11,12,13,14,16,16,16,16,
/* FD=3 */
  2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7,
  8 , 8, 9,10,11,12,13,14,16,17,19,20,22,22,22,22
};


/* OPN key frequency number -> key code follow table */
/* fnum higher 4bit -> keycode lower 2bit */
static const UINT8 opn_fktable[16] = {0,0,0,0,0,0,0,1,2,3,3,3,3,3,3,3};


/* 8 LFO speed parameters */
/* each value represents number of samples that one LFO level will last for */
static const UINT32 lfo_samples_per_step[8] = {108, 77, 71, 67, 62, 44, 8, 5};


/*There are 4 different LFO AM depths available, they are:
  0 dB, 1.4 dB, 5.9 dB, 11.8 dB
  Here is how it is generated (in EG steps):

  11.8 dB = 0, 2, 4, 6, 8, 10,12,14,16...126,126,124,122,120,118,....4,2,0
   5.9 dB = 0, 1, 2, 3, 4, 5, 6, 7, 8....63, 63, 62, 61, 60, 59,.....2,1,0
   1.4 dB = 0, 0, 0, 0, 1, 1, 1, 1, 2,...15, 15, 15, 15, 14, 14,.....0,0,0

  (1.4 dB is loosing precision as you can see)

  It's implemented as generator from 0..126 with step 2 then a shift
  right N times, where N is:
    8 for 0 dB
    3 for 1.4 dB
    1 for 5.9 dB
    0 for 11.8 dB
*/
static const UINT8 lfo_ams_depth_shift[4] = {8, 3, 1, 0};



/*There are 8 different LFO PM depths available, they are:
  0, 3.4, 6.7, 10, 14, 20, 40, 80 (cents)

  Modulation level at each depth depends on F-NUMBER bits: 4,5,6,7,8,9,10
  (bits 8,9,10 = FNUM MSB from OCT/FNUM register)

  Here we store only first quarter (positive one) of full waveform.
  Full table (lfo_pm_table) containing all 128 waveforms is build
  at run (init) time.

  One value in table below represents 4 (four) basic LFO steps
  (1 PM step = 4 AM steps).

  For example:
   at LFO SPEED=0 (which is 108 samples per basic LFO step)
   one value from "lfo_pm_output" table lasts for 432 consecutive
   samples (4*108=432) and one full LFO waveform cycle lasts for 13824
   samples (32*432=13824; 32 because we store only a quarter of whole
            waveform in the table below)
*/
static const UINT8 lfo_pm_output[7*8][8]={
/* 7 bits meaningful (of F-NUMBER), 8 LFO output levels per one depth (out of 32), 8 LFO depths */
/* FNUM BIT 4: 000 0001xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 5 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 6 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 7 */ {0,   0,   0,   0,   1,   1,   1,   1},

/* FNUM BIT 5: 000 0010xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 5 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 6 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 7 */ {0,   0,   1,   1,   2,   2,   2,   3},

/* FNUM BIT 6: 000 0100xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 3 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 4 */ {0,   0,   0,   0,   0,   0,   0,   1},
/* DEPTH 5 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 6 */ {0,   0,   1,   1,   2,   2,   2,   3},
/* DEPTH 7 */ {0,   0,   2,   3,   4,   4,   5,   6},

/* FNUM BIT 7: 000 1000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 2 */ {0,   0,   0,   0,   0,   0,   1,   1},
/* DEPTH 3 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 4 */ {0,   0,   0,   1,   1,   1,   1,   2},
/* DEPTH 5 */ {0,   0,   1,   1,   2,   2,   2,   3},
/* DEPTH 6 */ {0,   0,   2,   3,   4,   4,   5,   6},
/* DEPTH 7 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},

/* FNUM BIT 8: 001 0000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   1,   1,   1,   1},
/* DEPTH 2 */ {0,   0,   0,   1,   1,   1,   2,   2},
/* DEPTH 3 */ {0,   0,   1,   1,   2,   2,   3,   3},
/* DEPTH 4 */ {0,   0,   1,   2,   2,   2,   3,   4},
/* DEPTH 5 */ {0,   0,   2,   3,   4,   4,   5,   6},
/* DEPTH 6 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},
/* DEPTH 7 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},

/* FNUM BIT 9: 010 0000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   2,   2,   2,   2},
/* DEPTH 2 */ {0,   0,   0,   2,   2,   2,   4,   4},
/* DEPTH 3 */ {0,   0,   2,   2,   4,   4,   6,   6},
/* DEPTH 4 */ {0,   0,   2,   4,   4,   4,   6,   8},
/* DEPTH 5 */ {0,   0,   4,   6,   8,   8, 0xa, 0xc},
/* DEPTH 6 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},
/* DEPTH 7 */ {0,   0,0x10,0x18,0x20,0x20,0x28,0x30},

/* FNUM BIT10: 100 0000xxxx */
/* DEPTH 0 */ {0,   0,   0,   0,   0,   0,   0,   0},
/* DEPTH 1 */ {0,   0,   0,   0,   4,   4,   4,   4},
/* DEPTH 2 */ {0,   0,   0,   4,   4,   4,   8,   8},
/* DEPTH 3 */ {0,   0,   4,   4,   8,   8, 0xc, 0xc},
/* DEPTH 4 */ {0,   0,   4,   8,   8,   8, 0xc,0x10},
/* DEPTH 5 */ {0,   0,   8, 0xc,0x10,0x10,0x14,0x18},
/* DEPTH 6 */ {0,   0,0x10,0x18,0x20,0x20,0x28,0x30},
/* DEPTH 7 */ {0,   0,0x20,0x30,0x40,0x40,0x50,0x60},

};

/* all 128 LFO PM waveforms */
static INT32 lfo_pm_table[128*8*32]; /* 128 combinations of 7 bits meaningful (of F-NUMBER), 8 LFO depths, 32 LFO output levels per one depth */

/* register number to channel number , slot offset */
#define OPN_CHAN(N) (N&3)
#define OPN_SLOT(N) ((N>>2)&3)

/* slot number */
#define SLOT1 0
#define SLOT2 2
#define SLOT3 1
#define SLOT4 3

/* struct describing a single operator (SLOT) */
typedef struct
{
  INT32   *DT;        /* detune          :dt_tab[DT]      */
  UINT8   KSR;        /* key scale rate  :3-KSR           */
  UINT32  ar;         /* attack rate                      */
  UINT32  d1r;        /* decay rate                       */
  UINT32  d2r;        /* sustain rate                     */
  UINT32  rr;         /* release rate                     */
  UINT8   ksr;        /* key scale rate  :kcode>>(3-KSR)  */
  UINT32  mul;        /* multiple        :ML_TABLE[ML]    */

  /* Phase Generator */
  UINT32  phase;      /* phase counter */
  INT32   Incr;       /* phase step */

  /* Envelope Generator */
  UINT8   state;      /* phase type */
  UINT32  tl;         /* total level: TL << 3 */
  INT32   volume;     /* envelope counter */
  UINT32  sl;         /* sustain level:sl_table[SL] */
  UINT32  vol_out;    /* current output from EG circuit (without AM from LFO) */

  UINT8  eg_sh_ar;    /*  (attack state)  */
  UINT8  eg_sel_ar;   /*  (attack state)  */
  UINT8  eg_sh_d1r;   /*  (decay state)   */
  UINT8  eg_sel_d1r;  /*  (decay state)   */
  UINT8  eg_sh_d2r;   /*  (sustain state) */
  UINT8  eg_sel_d2r;  /*  (sustain state) */
  UINT8  eg_sh_rr;    /*  (release state) */
  UINT8  eg_sel_rr;   /*  (release state) */

  UINT8  ssg;         /* SSG-EG waveform  */
  UINT8  ssgn;        /* SSG-EG negated output  */

  UINT8  key;         /* 0=last key was KEY OFF, 1=KEY ON */

  /* LFO */
  UINT32  AMmask;     /* AM enable flag */

} FM_SLOT;

typedef struct
{
  FM_SLOT  SLOT[4];     /* four SLOTs (operators) */

  UINT8   ALGO;         /* algorithm */
  UINT8   FB;           /* feedback shift */
  INT32   op1_out[2];   /* op1 output for feedback */

  INT32   *connect1;    /* SLOT1 output pointer */
  INT32   *connect3;    /* SLOT3 output pointer */
  INT32   *connect2;    /* SLOT2 output pointer */
  INT32   *connect4;    /* SLOT4 output pointer */

  INT32   *mem_connect; /* where to put the delayed sample (MEM) */
  INT32   mem_value;    /* delayed sample (MEM) value */

  INT32   pms;          /* channel PMS */
  UINT8   ams;          /* channel AMS */

  UINT32  fc;           /* fnum,blk */
  UINT8   kcode;        /* key code */
  UINT32  block_fnum;   /* blk/fnum value (for LFO PM calculations) */
} FM_CH;


typedef struct
{
  UINT16  address;        /* address register     */
  UINT8   status;         /* status flag          */
  UINT32  mode;           /* mode  CSM / 3SLOT    */
  UINT8   fn_h;           /* freq latch           */
  INT32   TA;             /* timer a value        */
  INT32   TAL;            /* timer a base         */
  INT32   TAC;            /* timer a counter      */
  INT32   TB;             /* timer b value        */
  INT32   TBL;            /* timer b base         */
  INT32   TBC;            /* timer b counter      */
  INT32   dt_tab[8][32];  /* DeTune table         */

} FM_ST;


/***********************************************************/
/* OPN unit                                                */
/***********************************************************/

/* OPN 3slot struct */
typedef struct
{
  UINT32  fc[3];          /* fnum3,blk3: calculated */
  UINT8   fn_h;           /* freq3 latch */
  UINT8   kcode[3];       /* key code */
  UINT32  block_fnum[3];  /* current fnum value for this slot (can be different betweeen slots of one channel in 3slot mode) */
  UINT8   key_csm;        /* CSM mode Key-ON flag */

} FM_3SLOT;

/* OPN/A/B common state */
typedef struct
{
  FM_ST  ST;                  /* general state */
  FM_3SLOT SL3;               /* 3 slot mode state */
  unsigned int pan[6*2];      /* fm channels output masks (0xffffffff = enable) */

  /* EG */
  UINT32  eg_cnt;             /* global envelope generator counter */
  UINT32  eg_timer;           /* global envelope generator counter works at frequency = chipclock/144/3 */

  /* LFO */
  UINT8   lfo_cnt;            /* current LFO phase (out of 128) */
  UINT32  lfo_timer;          /* current LFO phase runs at LFO frequency */
  UINT32  lfo_timer_overflow; /* LFO timer overflows every N samples (depends on LFO frequency) */
  UINT32  LFO_AM;             /* current LFO AM step */
  UINT32  LFO_PM;             /* current LFO PM step */

} FM_OPN;

/***********************************************************/
/* YM2612 chip                                                */
/***********************************************************/
typedef struct
{
  FM_CH   CH[6];  /* channel state */
  UINT8   dacen;  /* DAC mode  */
  INT32   dacout; /* DAC output */
  FM_OPN  OPN;    /* OPN state */

} YM2612;

/* emulated chip */
static YM2612 ym2612;

/* current chip state */
static INT32  m2,c1,c2;   /* Phase Modulation input for operators 2,3,4 */
static INT32  mem;        /* one sample delay memory */
static INT32  out_fm[8];  /* outputs of working channels */

INLINE void FM_KEYON(FM_CH *CH , int s )
{
  FM_SLOT *SLOT = &CH->SLOT[s];

  if (!SLOT->key && !ym2612.OPN.SL3.key_csm)
  {
    /* restart Phase Generator */
    SLOT->phase = 0;

    /* reset SSG-EG inversion flag */
    SLOT->ssgn = 0;

    if ((SLOT->ar + SLOT->ksr) < 94 /*32+62*/)
    {
      SLOT->state = (SLOT->volume <= MIN_ATT_INDEX) ? ((SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC) : EG_ATT;
    }
    else
    {
      /* force attenuation level to 0 */
      SLOT->volume = MIN_ATT_INDEX;

      /* directly switch to Decay (or Sustain) */
      SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC;
    }

    /* recalculate EG output */
    if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)))
      SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
    else
      SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
  }

  SLOT->key = 1;
}

INLINE void FM_KEYOFF(FM_CH *CH , int s )
{
  FM_SLOT *SLOT = &CH->SLOT[s];

  if (SLOT->key && !ym2612.OPN.SL3.key_csm)
  {
    if (SLOT->state>EG_REL)
    {
      SLOT->state = EG_REL; /* phase -> Release */

      /* SSG-EG specific update */
      if (SLOT->ssg&0x08)
      {
        /* convert EG attenuation level */
        if (SLOT->ssgn ^ (SLOT->ssg&0x04))
          SLOT->volume = (0x200 - SLOT->volume);

        /* force EG attenuation level */
        if (SLOT->volume >= 0x200)
        {
          SLOT->volume = MAX_ATT_INDEX;
          SLOT->state  = EG_OFF;
        }

        /* recalculate EG output */
        SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
      }
    }
  }

  SLOT->key = 0;
}

INLINE void FM_KEYON_CSM(FM_CH *CH , int s )
{
  FM_SLOT *SLOT = &CH->SLOT[s];

  if (!SLOT->key && !ym2612.OPN.SL3.key_csm)
  {
    /* restart Phase Generator */
    SLOT->phase = 0;

    /* reset SSG-EG inversion flag */
    SLOT->ssgn = 0;

    if ((SLOT->ar + SLOT->ksr) < 94 /*32+62*/)
    {
      SLOT->state = (SLOT->volume <= MIN_ATT_INDEX) ? ((SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC) : EG_ATT;
    }
    else
    {
      /* force attenuation level to 0 */
      SLOT->volume = MIN_ATT_INDEX;

      /* directly switch to Decay (or Sustain) */
      SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC;
    }

    /* recalculate EG output */
    if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)))
      SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
    else
      SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
  }
}

INLINE void FM_KEYOFF_CSM(FM_CH *CH , int s )
{
  FM_SLOT *SLOT = &CH->SLOT[s];
  if (!SLOT->key)
  {
    if (SLOT->state>EG_REL)
    {
      SLOT->state = EG_REL; /* phase -> Release */

      /* SSG-EG specific update */
      if (SLOT->ssg&0x08)
      {
        /* convert EG attenuation level */
        if (SLOT->ssgn ^ (SLOT->ssg&0x04))
          SLOT->volume = (0x200 - SLOT->volume);

        /* force EG attenuation level */
        if (SLOT->volume >= 0x200)
        {
          SLOT->volume = MAX_ATT_INDEX;
          SLOT->state  = EG_OFF;
        }

        /* recalculate EG output */
        SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
      }
    }
  }
}

/* CSM Key Controll */
INLINE void CSMKeyControll(FM_CH *CH)
{
  /* all key ON (verified by Nemesis on real hardware) */
  FM_KEYON_CSM(CH,SLOT1);
  FM_KEYON_CSM(CH,SLOT2);
  FM_KEYON_CSM(CH,SLOT3);
  FM_KEYON_CSM(CH,SLOT4);
  ym2612.OPN.SL3.key_csm = 1;
}

INLINE void INTERNAL_TIMER_A()
{
  if (ym2612.OPN.ST.mode & 0x01)
  {
    ym2612.OPN.ST.TAC--;
    if (ym2612.OPN.ST.TAC <= 0)
    {
      /* set status (if enabled) */
      if (ym2612.OPN.ST.mode & 0x04)
        ym2612.OPN.ST.status |= 0x01;

      /* reload the counter */
      ym2612.OPN.ST.TAC = ym2612.OPN.ST.TAL;

      /* CSM mode auto key on */
      if ((ym2612.OPN.ST.mode & 0xC0) == 0x80)
        CSMKeyControll(&ym2612.CH[2]);
    }
  }
}

INLINE void INTERNAL_TIMER_B(int step)
{
  if (ym2612.OPN.ST.mode & 0x02)
  {
    ym2612.OPN.ST.TBC-=step;
    if (ym2612.OPN.ST.TBC <= 0)
    {
      /* set status (if enabled) */
      if (ym2612.OPN.ST.mode & 0x08)
        ym2612.OPN.ST.status |= 0x02;

      /* reload the counter */
      if (ym2612.OPN.ST.TBL)
        ym2612.OPN.ST.TBC += ym2612.OPN.ST.TBL;
      else
        ym2612.OPN.ST.TBC = ym2612.OPN.ST.TBL;
    }
  }
}

/* OPN Mode Register Write */
INLINE void set_timers(int v )
{
  /* b7 = CSM MODE */
  /* b6 = 3 slot mode */
  /* b5 = reset b */
  /* b4 = reset a */
  /* b3 = timer enable b */
  /* b2 = timer enable a */
  /* b1 = load b */
  /* b0 = load a */

  if ((ym2612.OPN.ST.mode ^ v) & 0xC0)
  {
    /* phase increment need to be recalculated */
    ym2612.CH[2].SLOT[SLOT1].Incr=-1;

    /* CSM mode disabled and CSM key ON active*/
    if (((v & 0xC0) != 0x80) && ym2612.OPN.SL3.key_csm)
    {
      /* CSM Mode Key OFF (verified by Nemesis on real hardware) */
      FM_KEYOFF_CSM(&ym2612.CH[2],SLOT1);
      FM_KEYOFF_CSM(&ym2612.CH[2],SLOT2);
      FM_KEYOFF_CSM(&ym2612.CH[2],SLOT3);
      FM_KEYOFF_CSM(&ym2612.CH[2],SLOT4);
      ym2612.OPN.SL3.key_csm = 0;
    }
  }

  /* reload Timers */
  if ((v&1) && !(ym2612.OPN.ST.mode&1))
    ym2612.OPN.ST.TAC = ym2612.OPN.ST.TAL;
  if ((v&2) && !(ym2612.OPN.ST.mode&2))
    ym2612.OPN.ST.TBC = ym2612.OPN.ST.TBL;
  
  /* reset Timers flags */
  ym2612.OPN.ST.status &= (~v >> 4); 

  ym2612.OPN.ST.mode = v;
}

/* set algorithm connection */
INLINE void setup_connection( FM_CH *CH, int ch )
{
  INT32 *carrier = &out_fm[ch];

  INT32 **om1 = &CH->connect1;
  INT32 **om2 = &CH->connect3;
  INT32 **oc1 = &CH->connect2;

  INT32 **memc = &CH->mem_connect;

  switch( CH->ALGO ){
    case 0:
      /* M1---C1---MEM---M2---C2---OUT */
      *om1 = &c1;
      *oc1 = &mem;
      *om2 = &c2;
      *memc= &m2;
      break;
    case 1:
      /* M1------+-MEM---M2---C2---OUT */
      /*      C1-+                     */
      *om1 = &mem;
      *oc1 = &mem;
      *om2 = &c2;
      *memc= &m2;
      break;
    case 2:
      /* M1-----------------+-C2---OUT */
      /*      C1---MEM---M2-+          */
      *om1 = &c2;
      *oc1 = &mem;
      *om2 = &c2;
      *memc= &m2;
      break;
    case 3:
      /* M1---C1---MEM------+-C2---OUT */
      /*                 M2-+          */
      *om1 = &c1;
      *oc1 = &mem;
      *om2 = &c2;
      *memc= &c2;
      break;
    case 4:
      /* M1---C1-+-OUT */
      /* M2---C2-+     */
      /* MEM: not used */
      *om1 = &c1;
      *oc1 = carrier;
      *om2 = &c2;
      *memc= &mem;  /* store it anywhere where it will not be used */
      break;
    case 5:
      /*    +----C1----+     */
      /* M1-+-MEM---M2-+-OUT */
      /*    +----C2----+     */
      *om1 = 0;  /* special mark */
      *oc1 = carrier;
      *om2 = carrier;
      *memc= &m2;
      break;
    case 6:
      /* M1---C1-+     */
      /*      M2-+-OUT */
      /*      C2-+     */
      /* MEM: not used */
      *om1 = &c1;
      *oc1 = carrier;
      *om2 = carrier;
      *memc= &mem;  /* store it anywhere where it will not be used */
      break;
    case 7:
      /* M1-+     */
      /* C1-+-OUT */
      /* M2-+     */
      /* C2-+     */
      /* MEM: not used*/
      *om1 = carrier;
      *oc1 = carrier;
      *om2 = carrier;
      *memc= &mem;  /* store it anywhere where it will not be used */
      break;
  }

  CH->connect4 = carrier;
}

/* set detune & multiple */
INLINE void set_det_mul(FM_CH *CH,FM_SLOT *SLOT,int v)
{
  SLOT->mul = (v&0x0f)? (v&0x0f)*2 : 1;
  SLOT->DT  = ym2612.OPN.ST.dt_tab[(v>>4)&7];
  CH->SLOT[SLOT1].Incr=-1;
}

/* set total level */
INLINE void set_tl(FM_SLOT *SLOT , int v)
{
  SLOT->tl = (v&0x7f)<<(ENV_BITS-7); /* 7bit TL */

  /* recalculate EG output */
  if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)) && (SLOT->state > EG_REL))
    SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
  else
    SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
}

/* set attack rate & key scale  */
INLINE void set_ar_ksr(FM_CH *CH,FM_SLOT *SLOT,int v)
{
  UINT8 old_KSR = SLOT->KSR;

  SLOT->ar = (v&0x1f) ? 32 + ((v&0x1f)<<1) : 0;

  SLOT->KSR = 3-(v>>6);
  if (SLOT->KSR != old_KSR)
  {
    CH->SLOT[SLOT1].Incr=-1;
  }

  /* Even if it seems unnecessary to do it here, it could happen that KSR and KC  */
  /* are modified but the resulted SLOT->ksr value (kc >> SLOT->KSR) remains unchanged. */
  /* In such case, Attack Rate would not be recalculated by "refresh_fc_eg_slot". */
  /* This actually fixes the intro of "The Adventures of Batman & Robin" (Eke-Eke)         */
  if ((SLOT->ar + SLOT->ksr) < (32+62))
  {
    SLOT->eg_sh_ar  = eg_rate_shift [SLOT->ar  + SLOT->ksr ];
    SLOT->eg_sel_ar = eg_rate_select[SLOT->ar  + SLOT->ksr ];
  }
  else
  {
    /* verified by Nemesis on real hardware (Attack phase is blocked) */
    SLOT->eg_sh_ar  = 0;
    SLOT->eg_sel_ar = 18*RATE_STEPS;
  }
 }

/* set decay rate */
INLINE void set_dr(FM_SLOT *SLOT,int v)
{
  SLOT->d1r = (v&0x1f) ? 32 + ((v&0x1f)<<1) : 0;

  SLOT->eg_sh_d1r = eg_rate_shift [SLOT->d1r + SLOT->ksr];
  SLOT->eg_sel_d1r= eg_rate_select[SLOT->d1r + SLOT->ksr];

}

/* set sustain rate */
INLINE void set_sr(FM_SLOT *SLOT,int v)
{
  SLOT->d2r = (v&0x1f) ? 32 + ((v&0x1f)<<1) : 0;

  SLOT->eg_sh_d2r = eg_rate_shift [SLOT->d2r + SLOT->ksr];
  SLOT->eg_sel_d2r= eg_rate_select[SLOT->d2r + SLOT->ksr];
}

/* set release rate */
INLINE void set_sl_rr(FM_SLOT *SLOT,int v)
{
  SLOT->sl = sl_table[ v>>4 ];
  
  /* check EG state changes */
  if ((SLOT->state == EG_DEC) && (SLOT->volume >= (INT32)(SLOT->sl)))
    SLOT->state = EG_SUS;

  SLOT->rr  = 34 + ((v&0x0f)<<2);

  SLOT->eg_sh_rr  = eg_rate_shift [SLOT->rr  + SLOT->ksr];
  SLOT->eg_sel_rr = eg_rate_select[SLOT->rr  + SLOT->ksr];
}

/* advance LFO to next sample */
INLINE void advance_lfo()
{
  if (ym2612.OPN.lfo_timer_overflow)   /* LFO enabled ? */
  {
    /* increment LFO timer (every samples) */
    ym2612.OPN.lfo_timer ++;

    /* when LFO is enabled, one level will last for 108, 77, 71, 67, 62, 44, 8 or 5 samples */
    if (ym2612.OPN.lfo_timer >= ym2612.OPN.lfo_timer_overflow)
    {
      ym2612.OPN.lfo_timer = 0;

      /* There are 128 LFO steps */
      ym2612.OPN.lfo_cnt = ( ym2612.OPN.lfo_cnt + 1 ) & 127;

      /* triangle (inverted) */
      /* AM: from 126 to 0 step -2, 0 to 126 step +2 */
      if (ym2612.OPN.lfo_cnt<64)
        ym2612.OPN.LFO_AM = (ym2612.OPN.lfo_cnt ^ 63) << 1;
      else
        ym2612.OPN.LFO_AM = (ym2612.OPN.lfo_cnt & 63) << 1;

      /* PM works with 4 times slower clock */
      ym2612.OPN.LFO_PM = ym2612.OPN.lfo_cnt >> 2;
    }
  }
}


INLINE void advance_eg_channels(FM_CH *CH, unsigned int eg_cnt)
{
  unsigned int i = 6; /* six channels */
  unsigned int j;
  FM_SLOT *SLOT;

  do
  {
    SLOT = &CH->SLOT[SLOT1];
    j = 4; /* four operators per channel */
    do
    {
      switch(SLOT->state)
      {
        case EG_ATT:    /* attack phase */
        {
          if (!(eg_cnt & ((1<<SLOT->eg_sh_ar)-1)))
          {
            /* update attenuation level */
            SLOT->volume += (~SLOT->volume * (eg_inc[SLOT->eg_sel_ar + ((eg_cnt>>SLOT->eg_sh_ar)&7)]))>>4;

            /* check phase transition*/
            if (SLOT->volume <= MIN_ATT_INDEX)
            {
              SLOT->volume = MIN_ATT_INDEX;
              SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC; /* special case where SL=0 */
            }

            /* recalculate EG output */
            if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)))  /* SSG-EG Output Inversion */
              SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
            else
              SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
          }
          break;
        }

        case EG_DEC:  /* decay phase */
        {
          if (!(eg_cnt & ((1<<SLOT->eg_sh_d1r)-1)))
          {
            /* SSG EG type */
            if (SLOT->ssg&0x08)
            {
              /* update attenuation level */
              if (SLOT->volume < 0x200)
              {
                SLOT->volume += 4 * eg_inc[SLOT->eg_sel_d1r + ((eg_cnt>>SLOT->eg_sh_d1r)&7)];

                /* recalculate EG output */
                if (SLOT->ssgn ^ (SLOT->ssg&0x04))   /* SSG-EG Output Inversion */
                  SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
                else
                  SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
              }
            }
            else
            {
              /* update attenuation level */
              SLOT->volume += eg_inc[SLOT->eg_sel_d1r + ((eg_cnt>>SLOT->eg_sh_d1r)&7)];

              /* recalculate EG output */
              SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
            }

            /* check phase transition*/
            if (SLOT->volume >= (INT32)(SLOT->sl))
              SLOT->state = EG_SUS;
          }
          break;
        }

        case EG_SUS:  /* sustain phase */
        {
          if (!(eg_cnt & ((1<<SLOT->eg_sh_d2r)-1)))
          {
            /* SSG EG type */
            if (SLOT->ssg&0x08)
            {
              /* update attenuation level */
              if (SLOT->volume < 0x200)
              {
                SLOT->volume += 4 * eg_inc[SLOT->eg_sel_d2r + ((eg_cnt>>SLOT->eg_sh_d2r)&7)];

                /* recalculate EG output */
                if (SLOT->ssgn ^ (SLOT->ssg&0x04))   /* SSG-EG Output Inversion */
                  SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
                else
                  SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
              }
            }
            else
            {
              /* update attenuation level */
              SLOT->volume += eg_inc[SLOT->eg_sel_d2r + ((eg_cnt>>SLOT->eg_sh_d2r)&7)];

              /* check phase transition*/
              if ( SLOT->volume >= MAX_ATT_INDEX )
                SLOT->volume = MAX_ATT_INDEX;
                /* do not change SLOT->state (verified on real chip) */

              /* recalculate EG output */
              SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
            }
          }
          break;
        }

        case EG_REL:  /* release phase */
        {
         if (!(eg_cnt & ((1<<SLOT->eg_sh_rr)-1)))
          {
             /* SSG EG type */
            if (SLOT->ssg&0x08)
            {
              /* update attenuation level */
              if (SLOT->volume < 0x200)
                SLOT->volume += 4 * eg_inc[SLOT->eg_sel_rr + ((eg_cnt>>SLOT->eg_sh_rr)&7)];

              /* check phase transition */
              if (SLOT->volume >= 0x200)
              {
                SLOT->volume = MAX_ATT_INDEX;
                SLOT->state = EG_OFF;
              }
            }
            else
            {
              /* update attenuation level */
              SLOT->volume += eg_inc[SLOT->eg_sel_rr + ((eg_cnt>>SLOT->eg_sh_rr)&7)];

              /* check phase transition*/
              if (SLOT->volume >= MAX_ATT_INDEX)
              {
                SLOT->volume = MAX_ATT_INDEX;
                SLOT->state = EG_OFF;
              }
            }

            /* recalculate EG output */
            SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;

          }
          break;
        }
      }

      /* next slot */
      SLOT++;
    } while (--j);

    /* next channel */
    CH++;
  } while (--i);
}

/* SSG-EG update process */
/* The behavior is based upon Nemesis tests on real hardware */
/* This is actually executed before each samples */
INLINE void update_ssg_eg_channels(FM_CH *CH)
{
  unsigned int i = 6; /* six channels */
  unsigned int j;
  FM_SLOT *SLOT;

  do
  {
    j = 4; /* four operators per channel */
    SLOT = &CH->SLOT[SLOT1];

    do
    {
      /* detect SSG-EG transition */
      /* this is not required during release phase as the attenuation has been forced to MAX and output invert flag is not used */
      /* if an Attack Phase is programmed, inversion can occur on each sample */
      if ((SLOT->ssg & 0x08) && (SLOT->volume >= 0x200) && (SLOT->state > EG_REL))
      {
        if (SLOT->ssg & 0x01)  /* bit 0 = hold SSG-EG */
        {
          /* set inversion flag */
          if (SLOT->ssg & 0x02)
            SLOT->ssgn = 4;

          /* force attenuation level during decay phases */
          if ((SLOT->state != EG_ATT) && !(SLOT->ssgn ^ (SLOT->ssg & 0x04)))
            SLOT->volume  = MAX_ATT_INDEX;
        }
        else  /* loop SSG-EG */
        {
          /* toggle output inversion flag or reset Phase Generator */
          if (SLOT->ssg & 0x02)
            SLOT->ssgn ^= 4;
          else
            SLOT->phase = 0;

          /* same as Key ON */
          if (SLOT->state != EG_ATT)
          {
            if ((SLOT->ar + SLOT->ksr) < 94 /*32+62*/)
            {
              SLOT->state = (SLOT->volume <= MIN_ATT_INDEX) ? ((SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC) : EG_ATT;
            }
            else
            {
              /* Attack Rate is maximal: directly switch to Decay or Substain */
              SLOT->volume = MIN_ATT_INDEX;
              SLOT->state = (SLOT->sl == MIN_ATT_INDEX) ? EG_SUS : EG_DEC;
            }
          }
        }

        /* recalculate EG output */
        if (SLOT->ssgn ^ (SLOT->ssg&0x04))
          SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
        else
          SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
      }

      /* next slot */
      SLOT++;
    } while (--j);

    /* next channel */
    CH++;
  } while (--i);
}

INLINE void update_phase_lfo_slot(FM_SLOT *SLOT, INT32 pms, UINT32 block_fnum)
{
  INT32 lfo_fn_table_index_offset = lfo_pm_table[(((block_fnum & 0x7f0) >> 4) << 8) + pms + ym2612.OPN.LFO_PM];
  
  if (lfo_fn_table_index_offset)  /* LFO phase modulation active */
  {
    UINT8 blk;
    unsigned int kc, fc;

    /* there are 2048 FNUMs that can be generated using FNUM/BLK registers
          but LFO works with one more bit of a precision so we really need 4096 elements */
    block_fnum = block_fnum*2 + lfo_fn_table_index_offset;
    blk = (block_fnum&0x7000) >> 12;
    block_fnum = block_fnum & 0xfff;

    /* keyscale code */
    kc = (blk<<2) | opn_fktable[block_fnum >> 8];

    /* (frequency) phase increment counter */
    fc = (((block_fnum << 5) >> (7 - blk)) + SLOT->DT[kc]) & DT_MASK;

    /* update phase */
    SLOT->phase += (fc * SLOT->mul) >> 1;
  }
  else  /* LFO phase modulation  = zero */
  {
    SLOT->phase += SLOT->Incr;
  }
}

INLINE void update_phase_lfo_channel(FM_CH *CH)
{
  UINT32 block_fnum = CH->block_fnum;
  
  INT32 lfo_fn_table_index_offset = lfo_pm_table[(((block_fnum & 0x7f0) >> 4) << 8) + CH->pms + ym2612.OPN.LFO_PM];

  if (lfo_fn_table_index_offset)  /* LFO phase modulation active */
  {
    UINT8 blk;
    unsigned int kc, fc, finc;
   
    /* there are 2048 FNUMs that can be generated using FNUM/BLK registers
          but LFO works with one more bit of a precision so we really need 4096 elements */
    block_fnum = block_fnum*2 + lfo_fn_table_index_offset;
    blk = (block_fnum&0x7000) >> 12;
    block_fnum = block_fnum & 0xfff;
    
    /* keyscale code */
    kc = (blk<<2) | opn_fktable[block_fnum >> 8];

    /* (frequency) phase increment counter */
    fc = (block_fnum << 5) >> (7 - blk);

    /* apply DETUNE & MUL operator specific values */
    finc = (fc + CH->SLOT[SLOT1].DT[kc]) & DT_MASK;
    CH->SLOT[SLOT1].phase += (finc*CH->SLOT[SLOT1].mul) >> 1;

    finc = (fc + CH->SLOT[SLOT2].DT[kc]) & DT_MASK;
    CH->SLOT[SLOT2].phase += (finc*CH->SLOT[SLOT2].mul) >> 1;

    finc = (fc + CH->SLOT[SLOT3].DT[kc]) & DT_MASK;
    CH->SLOT[SLOT3].phase += (finc*CH->SLOT[SLOT3].mul) >> 1;

    finc = (fc + CH->SLOT[SLOT4].DT[kc]) & DT_MASK;
    CH->SLOT[SLOT4].phase += (finc*CH->SLOT[SLOT4].mul) >> 1;
  }
  else  /* LFO phase modulation  = zero */
  {
    CH->SLOT[SLOT1].phase += CH->SLOT[SLOT1].Incr;
    CH->SLOT[SLOT2].phase += CH->SLOT[SLOT2].Incr;
    CH->SLOT[SLOT3].phase += CH->SLOT[SLOT3].Incr;
    CH->SLOT[SLOT4].phase += CH->SLOT[SLOT4].Incr;
  }
}

/* update phase increment and envelope generator */
INLINE void refresh_fc_eg_slot(FM_SLOT *SLOT , unsigned int fc , unsigned int kc )
{
  /* add detune value */
  fc += SLOT->DT[kc];

  /* (frequency) phase overflow (credits to Nemesis) */
  fc &= DT_MASK;

  /* (frequency) phase increment counter */
  SLOT->Incr = (fc * SLOT->mul) >> 1;

  /* ksr */
  kc = kc >> SLOT->KSR;

  if( SLOT->ksr != kc )
  {
    SLOT->ksr = kc;

    /* recalculate envelope generator rates */
    if ((SLOT->ar + kc) < (32+62))
    {
      SLOT->eg_sh_ar  = eg_rate_shift [SLOT->ar  + kc ];
      SLOT->eg_sel_ar = eg_rate_select[SLOT->ar  + kc ];
    }
    else
    {
      /* verified by Nemesis on real hardware (Attack phase is blocked) */
      SLOT->eg_sh_ar  = 0;
      SLOT->eg_sel_ar = 18*RATE_STEPS;
    }

    SLOT->eg_sh_d1r = eg_rate_shift [SLOT->d1r + kc];
    SLOT->eg_sel_d1r= eg_rate_select[SLOT->d1r + kc];

    SLOT->eg_sh_d2r = eg_rate_shift [SLOT->d2r + kc];
    SLOT->eg_sel_d2r= eg_rate_select[SLOT->d2r + kc];

    SLOT->eg_sh_rr  = eg_rate_shift [SLOT->rr  + kc];
    SLOT->eg_sel_rr = eg_rate_select[SLOT->rr  + kc];
  }
}

/* update phase increment counters */
INLINE void refresh_fc_eg_chan(FM_CH *CH )
{
  if( CH->SLOT[SLOT1].Incr==-1)
  {
    int fc = CH->fc;
    int kc = CH->kcode;
    refresh_fc_eg_slot(&CH->SLOT[SLOT1] , fc , kc );
    refresh_fc_eg_slot(&CH->SLOT[SLOT2] , fc , kc );
    refresh_fc_eg_slot(&CH->SLOT[SLOT3] , fc , kc );
    refresh_fc_eg_slot(&CH->SLOT[SLOT4] , fc , kc );
  }
}

#define volume_calc(OP) ((OP)->vol_out + (AM & (OP)->AMmask))

INLINE signed int op_calc(UINT32 phase, unsigned int env, unsigned int pm)
{
  UINT32 p = (env<<3) + sin_tab[ ( (phase >> SIN_BITS) + (pm >> 1) ) & SIN_MASK ];

  if (p >= TL_TAB_LEN)
    return 0;
  return tl_tab[p];
}

INLINE signed int op_calc1(UINT32 phase, unsigned int env, unsigned int pm)
{
  UINT32 p = (env<<3) + sin_tab[ ( (phase + pm ) >> SIN_BITS ) & SIN_MASK ];

  if (p >= TL_TAB_LEN)
    return 0;
  return tl_tab[p];
}

INLINE void chan_calc(FM_CH *CH, int num)
{
  do
  {
    UINT32 AM = ym2612.OPN.LFO_AM >> CH->ams;
    unsigned int eg_out = volume_calc(&CH->SLOT[SLOT1]);

    m2 = c1 = c2 = mem = 0;

    *CH->mem_connect = CH->mem_value;  /* restore delayed sample (MEM) value to m2 or c2 */
    {
      INT32 out = CH->op1_out[0] + CH->op1_out[1];
      CH->op1_out[0] = CH->op1_out[1];

      if( !CH->connect1 ){
        /* algorithm 5  */
        mem = c1 = c2 = CH->op1_out[0];
      }else{
        /* other algorithms */
        *CH->connect1 += CH->op1_out[0];
      }

      CH->op1_out[1] = 0;
      if( eg_out < ENV_QUIET )  /* SLOT 1 */
      {
        if (!CH->FB)
          out=0;

        CH->op1_out[1] = op_calc1(CH->SLOT[SLOT1].phase, eg_out, (out<<CH->FB) );
      }
    }

    eg_out = volume_calc(&CH->SLOT[SLOT3]);
    if( eg_out < ENV_QUIET )    /* SLOT 3 */
      *CH->connect3 += op_calc(CH->SLOT[SLOT3].phase, eg_out, m2);

    eg_out = volume_calc(&CH->SLOT[SLOT2]);
    if( eg_out < ENV_QUIET )    /* SLOT 2 */
    *CH->connect2 += op_calc(CH->SLOT[SLOT2].phase, eg_out, c1);

    eg_out = volume_calc(&CH->SLOT[SLOT4]);
    if( eg_out < ENV_QUIET )    /* SLOT 4 */
      *CH->connect4 += op_calc(CH->SLOT[SLOT4].phase, eg_out, c2);


    /* store current MEM */
    CH->mem_value = mem;

    /* update phase counters AFTER output calculations */
    if(CH->pms)
    {
      /* add support for 3 slot mode */
      if ((ym2612.OPN.ST.mode & 0xC0) && (CH == &ym2612.CH[2]))
      {
        update_phase_lfo_slot(&CH->SLOT[SLOT1], CH->pms, ym2612.OPN.SL3.block_fnum[1]);
        update_phase_lfo_slot(&CH->SLOT[SLOT2], CH->pms, ym2612.OPN.SL3.block_fnum[2]);
        update_phase_lfo_slot(&CH->SLOT[SLOT3], CH->pms, ym2612.OPN.SL3.block_fnum[0]);
        update_phase_lfo_slot(&CH->SLOT[SLOT4], CH->pms, CH->block_fnum);
      }
      else
      {
        update_phase_lfo_channel(CH);
      }
    }
    else  /* no LFO phase modulation */
    {
      CH->SLOT[SLOT1].phase += CH->SLOT[SLOT1].Incr;
      CH->SLOT[SLOT2].phase += CH->SLOT[SLOT2].Incr;
      CH->SLOT[SLOT3].phase += CH->SLOT[SLOT3].Incr;
      CH->SLOT[SLOT4].phase += CH->SLOT[SLOT4].Incr;
    }

    /* next channel */
    CH++;
  } while (--num);
}

/* write a OPN mode register 0x20-0x2f */
INLINE void OPNWriteMode(int r, int v)
{
  UINT8 c;
  FM_CH *CH;

  switch(r){
    case 0x21:  /* Test */
      break;

    case 0x22:  /* LFO FREQ (YM2608/YM2610/YM2610B/ym2612) */
      if (v&8) /* LFO enabled ? */
      {
        ym2612.OPN.lfo_timer_overflow = lfo_samples_per_step[v&7];
      }
      else
      {
        /* hold LFO waveform in reset state */
        ym2612.OPN.lfo_timer_overflow = 0;
        ym2612.OPN.lfo_timer = 0;
        ym2612.OPN.lfo_cnt = 0;
        ym2612.OPN.LFO_PM = 0;
        ym2612.OPN.LFO_AM = 126;
      }
      break;
    case 0x24:  /* timer A High 8*/
      ym2612.OPN.ST.TA = (ym2612.OPN.ST.TA & 0x03)|(((int)v)<<2);
      ym2612.OPN.ST.TAL = 1024 - ym2612.OPN.ST.TA;
      break;
    case 0x25:  /* timer A Low 2*/
      ym2612.OPN.ST.TA = (ym2612.OPN.ST.TA & 0x3fc)|(v&3);
      ym2612.OPN.ST.TAL = 1024 - ym2612.OPN.ST.TA;
      break;
    case 0x26:  /* timer B */
      ym2612.OPN.ST.TB = v;
      ym2612.OPN.ST.TBL = (256 - v) << 4;
      break;
    case 0x27:  /* mode, timer control */
      set_timers(v);
      break;
    case 0x28:  /* key on / off */
      c = v & 0x03;
      if( c == 3 ) break;
      if (v&0x04) c+=3; /* CH 4-6 */
      CH = &ym2612.CH[c];

      if (v&0x10) FM_KEYON(CH,SLOT1); else FM_KEYOFF(CH,SLOT1);
      if (v&0x20) FM_KEYON(CH,SLOT2); else FM_KEYOFF(CH,SLOT2);
      if (v&0x40) FM_KEYON(CH,SLOT3); else FM_KEYOFF(CH,SLOT3);
      if (v&0x80) FM_KEYON(CH,SLOT4); else FM_KEYOFF(CH,SLOT4);
      break;
  }
}

/* write a OPN register (0x30-0xff) */
INLINE void OPNWriteReg(int r, int v)
{
  FM_CH *CH;
  FM_SLOT *SLOT;

  UINT8 c = OPN_CHAN(r);

  if (c == 3) return; /* 0xX3,0xX7,0xXB,0xXF */

  if (r >= 0x100) c+=3;

  CH = &ym2612.CH[c];

  SLOT = &(CH->SLOT[OPN_SLOT(r)]);

  switch( r & 0xf0 ) {
    case 0x30:  /* DET , MUL */
      set_det_mul(CH,SLOT,v);
      break;

    case 0x40:  /* TL */
      set_tl(SLOT,v);
      break;

    case 0x50:  /* KS, AR */
      set_ar_ksr(CH,SLOT,v);
      break;

    case 0x60:  /* bit7 = AM ENABLE, DR */
      set_dr(SLOT,v);
      SLOT->AMmask = (v&0x80) ? ~0 : 0;
      break;

    case 0x70:  /*     SR */
      set_sr(SLOT,v);
      break;

    case 0x80:  /* SL, RR */
      set_sl_rr(SLOT,v);
      break;

    case 0x90:  /* SSG-EG */
      SLOT->ssg  = v&0x0f;

      /* recalculate EG output */
      if (SLOT->state > EG_REL)
      {
        if ((SLOT->ssg&0x08) && (SLOT->ssgn ^ (SLOT->ssg&0x04)))
          SLOT->vol_out = ((UINT32)(0x200 - SLOT->volume) & MAX_ATT_INDEX) + SLOT->tl;
        else
          SLOT->vol_out = (UINT32)SLOT->volume + SLOT->tl;
      }

      /* SSG-EG envelope shapes :

      E AtAlH
      1 0 0 0  \\\\

      1 0 0 1  \___

      1 0 1 0  \/\/
                ___
      1 0 1 1  \

      1 1 0 0  ////
                ___
      1 1 0 1  /

      1 1 1 0  /\/\

      1 1 1 1  /___


      E = SSG-EG enable


      The shapes are generated using Attack, Decay and Sustain phases.

      Each single character in the diagrams above represents this whole
      sequence:

      - when KEY-ON = 1, normal Attack phase is generated (*without* any
        difference when compared to normal mode),

      - later, when envelope level reaches minimum level (max volume),
        the EG switches to Decay phase (which works with bigger steps
        when compared to normal mode - see below),

      - later when envelope level passes the SL level,
        the EG swithes to Sustain phase (which works with bigger steps
        when compared to normal mode - see below),

      - finally when envelope level reaches maximum level (min volume),
        the EG switches to Attack phase again (depends on actual waveform).

      Important is that when switch to Attack phase occurs, the phase counter
      of that operator will be zeroed-out (as in normal KEY-ON) but not always.
      (I havent found the rule for that - perhaps only when the output level is low)

      The difference (when compared to normal Envelope Generator mode) is
      that the resolution in Decay and Sustain phases is 4 times lower;
      this results in only 256 steps instead of normal 1024.
      In other words:
      when SSG-EG is disabled, the step inside of the EG is one,
      when SSG-EG is enabled, the step is four (in Decay and Sustain phases).

      Times between the level changes are the same in both modes.


      Important:
      Decay 1 Level (so called SL) is compared to actual SSG-EG output, so
      it is the same in both SSG and no-SSG modes, with this exception:

      when the SSG-EG is enabled and is generating raising levels
      (when the EG output is inverted) the SL will be found at wrong level !!!
      For example, when SL=02:
        0 -6 = -6dB in non-inverted EG output
        96-6 = -90dB in inverted EG output
      Which means that EG compares its level to SL as usual, and that the
      output is simply inverted afterall.


      The Yamaha's manuals say that AR should be set to 0x1f (max speed).
      That is not necessary, but then EG will be generating Attack phase.

      */


      break;

    case 0xa0:
      switch( OPN_SLOT(r) ){
        case 0:    /* 0xa0-0xa2 : FNUM1 */
        {
          UINT32 fn = (((UINT32)((ym2612.OPN.ST.fn_h)&7))<<8) + v;
          UINT8 blk = ym2612.OPN.ST.fn_h>>3;
          /* keyscale code */
          CH->kcode = (blk<<2) | opn_fktable[fn >> 7];
          /* phase increment counter */
          CH->fc = (fn << 6) >> (7 - blk);

          /* store fnum in clear form for LFO PM calculations */
          CH->block_fnum = (blk<<11) | fn;

          CH->SLOT[SLOT1].Incr=-1;
          break;
        }
        case 1:    /* 0xa4-0xa6 : FNUM2,BLK */
          ym2612.OPN.ST.fn_h = v&0x3f;
          break;
        case 2:    /* 0xa8-0xaa : 3CH FNUM1 */
          if(r < 0x100)
          {
            UINT32 fn = (((UINT32)(ym2612.OPN.SL3.fn_h&7))<<8) + v;
            UINT8 blk = ym2612.OPN.SL3.fn_h>>3;
            /* keyscale code */
            ym2612.OPN.SL3.kcode[c]= (blk<<2) | opn_fktable[fn >> 7];
            /* phase increment counter */
            ym2612.OPN.SL3.fc[c] = (fn << 6) >> (7 - blk);
            ym2612.OPN.SL3.block_fnum[c] = (blk<<11) | fn;
            ym2612.CH[2].SLOT[SLOT1].Incr=-1;
          }
          break;            
        case 3:    /* 0xac-0xae : 3CH FNUM2,BLK */
          if(r < 0x100)
            ym2612.OPN.SL3.fn_h = v&0x3f;
          break;
      }
      break;

    case 0xb0:
      switch( OPN_SLOT(r) ){
        case 0:    /* 0xb0-0xb2 : FB,ALGO */
        {
          CH->ALGO = v&7;
          CH->FB   = (v>>3)&7;
          setup_connection( CH, c );
          break;        
        }
        case 1:    /* 0xb4-0xb6 : L , R , AMS , PMS (ym2612/YM2610B/YM2610/YM2608) */
          /* b0-2 PMS */
          CH->pms = (v & 7) * 32; /* CH->pms = PM depth * 32 (index in lfo_pm_table) */

          /* b4-5 AMS */
          CH->ams = lfo_ams_depth_shift[(v>>4) & 0x03];

          /* PAN :  b7 = L, b6 = R */
          ym2612.OPN.pan[ c*2   ] = (v & 0x80) ? ~0 : 0;
          ym2612.OPN.pan[ c*2+1 ] = (v & 0x40) ? ~0 : 0;
          break;
      }
      break;
  }
}

static void reset_channels(FM_CH *CH , int num )
{
  int c,s;

  for( c = 0 ; c < num ; c++ )
  {
    CH[c].mem_value   = 0;
    CH[c].op1_out[0]  = 0;
    CH[c].op1_out[1]  = 0;
    for(s = 0 ; s < 4 ; s++ )
    {
      CH[c].SLOT[s].Incr    = -1;
      CH[c].SLOT[s].key     = 0;
      CH[c].SLOT[s].phase   = 0;
      CH[c].SLOT[s].ssgn    = 0;
      CH[c].SLOT[s].state   = EG_OFF;
      CH[c].SLOT[s].volume  = MAX_ATT_INDEX;
      CH[c].SLOT[s].vol_out = MAX_ATT_INDEX;
    }
  }
}

/* initialize generic tables */
static void init_tables(unsigned char dac_bits)
{
  signed int d,i,x;
  signed int n;
  double o,m;
  
  /* DAC precision */
  unsigned int mask = ~((1 << (14 - dac_bits)) - 1);

  /* build Linear Power Table */
  for (x=0; x<TL_RES_LEN; x++)
  {
    m = (1<<16) / pow(2,(x+1) * (ENV_STEP/4.0) / 8.0);
    m = floor(m);

    /* we never reach (1<<16) here due to the (x+1) */
    /* result fits within 16 bits at maximum */

    n = (int)m; /* 16 bits here */
    n >>= 4;    /* 12 bits here */
    if (n&1)    /* round to nearest */
      n = (n>>1)+1;
    else
      n = n>>1;
                /* 11 bits here (rounded) */
    n <<= 2;    /* 13 bits here (as in real chip) */

    /* 14 bits (with sign bit) */
    tl_tab[ x*2 + 0 ] = n & mask;
    tl_tab[ x*2 + 1 ] = -tl_tab[ x*2 + 0 ] & mask;

    /* one entry in the 'Power' table use the following format, xxxxxyyyyyyyys with:            */
    /*        s = sign bit                                                                      */
    /* yyyyyyyy = 8-bits decimal part (0-TL_RES_LEN)                                            */
    /* xxxxx    = 5-bits integer 'shift' value (0-31) but, since Power table output is 13 bits, */
    /*            any value above 13 (included) would be discarded.                             */
    for (i=1; i<13; i++)
    {
      tl_tab[ x*2+0 + i*2*TL_RES_LEN ] =  (tl_tab[ x*2+0 ]>>i) & mask;
      tl_tab[ x*2+1 + i*2*TL_RES_LEN ] = -tl_tab[ x*2+0 + i*2*TL_RES_LEN ] & mask;
    }
  }

  /* build Logarithmic Sinus table */
  for (i=0; i<SIN_LEN; i++)
  {
    /* non-standard sinus */
    m = sin( ((i*2)+1) * M_PI / SIN_LEN ); /* checked against the real chip */
    /* we never reach zero here due to ((i*2)+1) */

    if (m>0.0)
      o = 8*log(1.0/m)/log(2);  /* convert to 'decibels' */
    else
      o = 8*log(-1.0/m)/log(2);  /* convert to 'decibels' */

    o = o / (ENV_STEP/4);

    n = (int)(2.0*o);
    if (n&1)            /* round to nearest */
      n = (n>>1)+1;
    else
      n = n>>1;

    /* 13-bits (8.5) value is formatted for above 'Power' table */
    sin_tab[ i ] = n*2 + (m>=0.0? 0: 1 );
  }

  /* build LFO PM modulation table */
  for(i = 0; i < 8; i++) /* 8 PM depths */
  {
    UINT8 fnum;
    for (fnum=0; fnum<128; fnum++) /* 7 bits meaningful of F-NUMBER */
    {
      UINT8 value;
      UINT8 step;
      UINT32 offset_depth = i;
      UINT32 offset_fnum_bit;
      UINT32 bit_tmp;

      for (step=0; step<8; step++) 
      {
        value = 0;
        for (bit_tmp=0; bit_tmp<7; bit_tmp++) /* 7 bits */
        {
          if (fnum & (1<<bit_tmp)) /* only if bit "bit_tmp" is set */
          {
            offset_fnum_bit = bit_tmp * 8;
            value += lfo_pm_output[offset_fnum_bit + offset_depth][step];
          }
        }
        /* 32 steps for LFO PM (sinus) */
        lfo_pm_table[(fnum*32*8) + (i*32) + step   + 0] = value;
        lfo_pm_table[(fnum*32*8) + (i*32) +(step^7)+ 8] = value;
        lfo_pm_table[(fnum*32*8) + (i*32) + step   +16] = -value;
        lfo_pm_table[(fnum*32*8) + (i*32) +(step^7)+24] = -value;
      }
    }
  }

  /* build DETUNE table */
  for (d = 0;d <= 3;d++)
  {
    for (i = 0;i <= 31;i++)
    {
      ym2612.OPN.ST.dt_tab[d][i]   = (INT32) dt_tab[d*32 + i];
      ym2612.OPN.ST.dt_tab[d+4][i] = -ym2612.OPN.ST.dt_tab[d][i];
    }
  }

}



/* initialize ym2612 emulator */
void YM2612Init(void)
{
  memset(&ym2612,0,sizeof(YM2612));
  init_tables(TL_BITS);
}

/* reset OPN registers */
void YM2612ResetChip(void)
{
  int i;

  ym2612.OPN.eg_timer     = 0;
  ym2612.OPN.eg_cnt       = 0;

  ym2612.OPN.lfo_timer_overflow = 0;
  ym2612.OPN.lfo_timer          = 0;
  ym2612.OPN.lfo_cnt            = 0;
  ym2612.OPN.LFO_AM             = 126;
  ym2612.OPN.LFO_PM             = 0;

  ym2612.OPN.ST.TAC       = 0;
  ym2612.OPN.ST.TBC       = 0;

  ym2612.OPN.SL3.key_csm  = 0;

  ym2612.dacen            = 0;
  ym2612.dacout           = 0;
 
  set_timers(0x30);
  ym2612.OPN.ST.TB = 0;
  ym2612.OPN.ST.TBL = 256 << 4;
  ym2612.OPN.ST.TA = 0;
  ym2612.OPN.ST.TAL = 1024;

  reset_channels(&ym2612.CH[0] , 6 );

  for(i = 0xb6 ; i >= 0xb4 ; i-- )
  {
    OPNWriteReg(i      ,0xc0);
    OPNWriteReg(i|0x100,0xc0);
  }
  for(i = 0xb2 ; i >= 0x30 ; i-- )
  {
    OPNWriteReg(i      ,0);
    OPNWriteReg(i|0x100,0);
  }
}

/* ym2612 write */
/* n = number  */
/* a = address */
/* v = value   */
void YM2612Write(unsigned int a, unsigned int v)
{
  v &= 0xff;  /* adjust to 8 bit bus */

  switch( a )
  {
    case 0:  /* address port 0 */
      ym2612.OPN.ST.address = v;
      break;

    case 2:  /* address port 1 */
      ym2612.OPN.ST.address = v | 0x100;
      break;

    default:  /* data port */
    {
      int addr = ym2612.OPN.ST.address; /* verified by Nemesis on real YM2612 */
      switch( addr & 0x1f0 )
      {
        case 0x20:  /* 0x20-0x2f Mode */
          switch( addr )
          {
            case 0x2a:  /* DAC data (ym2612) */
              ym2612.dacout = ((int)v - 0x80) << 6; /* level unknown (5 is too low, 8 is too loud) */
              break;
            case 0x2b:  /* DAC Sel  (ym2612) */
              /* b7 = dac enable */
              ym2612.dacen = v & 0x80;
              break;
            default:  /* OPN section */
              /* write register */
              OPNWriteMode(addr,v);
          }
          break;
        default:  /* 0x30-0xff OPN section */
          /* write register */
          OPNWriteReg(addr,v);
      }
      break;
    }
  }
}

unsigned int YM2612Read(void)
{
  return ym2612.OPN.ST.status & 0xff;
}

/* Generate samples for ym2612 */
void YM2612Update(int *buffer, int length)
{
  int i;
  int lt,rt;

  /* refresh PG increments and EG rates if required */
  refresh_fc_eg_chan(&ym2612.CH[0]);
  refresh_fc_eg_chan(&ym2612.CH[1]);

  if (!(ym2612.OPN.ST.mode & 0xC0))
  {
    refresh_fc_eg_chan(&ym2612.CH[2]);
  }
  else
  {  
    /* 3SLOT MODE (operator order is 0,1,3,2) */
    if(ym2612.CH[2].SLOT[SLOT1].Incr==-1)
    {
      refresh_fc_eg_slot(&ym2612.CH[2].SLOT[SLOT1] , ym2612.OPN.SL3.fc[1] , ym2612.OPN.SL3.kcode[1] );
      refresh_fc_eg_slot(&ym2612.CH[2].SLOT[SLOT2] , ym2612.OPN.SL3.fc[2] , ym2612.OPN.SL3.kcode[2] );
      refresh_fc_eg_slot(&ym2612.CH[2].SLOT[SLOT3] , ym2612.OPN.SL3.fc[0] , ym2612.OPN.SL3.kcode[0] );
      refresh_fc_eg_slot(&ym2612.CH[2].SLOT[SLOT4] , ym2612.CH[2].fc , ym2612.CH[2].kcode );
    }
  }

  refresh_fc_eg_chan(&ym2612.CH[3]);
  refresh_fc_eg_chan(&ym2612.CH[4]);
  refresh_fc_eg_chan(&ym2612.CH[5]);

  /* buffering */
  for(i=0; i < length ; i++)
  {
    /* clear outputs */
    out_fm[0] = 0;
    out_fm[1] = 0;
    out_fm[2] = 0;
    out_fm[3] = 0;
    out_fm[4] = 0;
    out_fm[5] = 0;

    /* update SSG-EG output */
    update_ssg_eg_channels(&ym2612.CH[0]);

    /* calculate FM */
    if (!ym2612.dacen)
    {
      chan_calc(&ym2612.CH[0],6);
    }
    else
    {
      /* DAC Mode */
      out_fm[5] = ym2612.dacout;
      chan_calc(&ym2612.CH[0],5);
    }

    /* advance LFO */
    advance_lfo();

    /* advance envelope generator */
    ym2612.OPN.eg_timer ++;

    /* EG is updated every 3 samples */
    if (ym2612.OPN.eg_timer >= 3)
    {
      ym2612.OPN.eg_timer = 0;
      ym2612.OPN.eg_cnt++;
      advance_eg_channels(&ym2612.CH[0], ym2612.OPN.eg_cnt);
    }

    /* 14-bit DAC inputs (range is -8192;+8192) */
    if (out_fm[0] > 8192) out_fm[0] = 8192;
    else if (out_fm[0] < -8192) out_fm[0] = -8192;
    if (out_fm[1] > 8192) out_fm[1] = 8192;
    else if (out_fm[1] < -8192) out_fm[1] = -8192;
    if (out_fm[2] > 8192) out_fm[2] = 8192;
    else if (out_fm[2] < -8192) out_fm[2] = -8192;
    if (out_fm[3] > 8192) out_fm[3] = 8192;
    else if (out_fm[3] < -8192) out_fm[3] = -8192;
    if (out_fm[4] > 8192) out_fm[4] = 8192;
    else if (out_fm[4] < -8192) out_fm[4] = -8192;
    if (out_fm[5] > 8192) out_fm[5] = 8192;
    else if (out_fm[5] < -8192) out_fm[5] = -8192;

    /* 6-channels stereo mixing  */
    lt  = ((out_fm[0]) & ym2612.OPN.pan[0]);
    rt  = ((out_fm[0]) & ym2612.OPN.pan[1]);
    lt += ((out_fm[1]) & ym2612.OPN.pan[2]);
    rt += ((out_fm[1]) & ym2612.OPN.pan[3]);
    lt += ((out_fm[2]) & ym2612.OPN.pan[4]);
    rt += ((out_fm[2]) & ym2612.OPN.pan[5]);
    lt += ((out_fm[3]) & ym2612.OPN.pan[6]);
    rt += ((out_fm[3]) & ym2612.OPN.pan[7]);
    lt += ((out_fm[4]) & ym2612.OPN.pan[8]);
    rt += ((out_fm[4]) & ym2612.OPN.pan[9]);
    lt += ((out_fm[5]) & ym2612.OPN.pan[10]);
    rt += ((out_fm[5]) & ym2612.OPN.pan[11]);

    /* buffering */
    *buffer++ = lt;
    *buffer++ = rt;

    /* CSM mode: if CSM Key ON has occured, CSM Key OFF need to be sent       */
    /* only if Timer A does not overflow again (i.e CSM Key ON not set again) */
    ym2612.OPN.SL3.key_csm <<= 1;

    /* timer A control */
    INTERNAL_TIMER_A();

    /* CSM Mode Key ON still disabled */
    if (ym2612.OPN.SL3.key_csm & 2)
    {
      /* CSM Mode Key OFF (verified by Nemesis on real hardware) */
      FM_KEYOFF_CSM(&ym2612.CH[2],SLOT1);
      FM_KEYOFF_CSM(&ym2612.CH[2],SLOT2);
      FM_KEYOFF_CSM(&ym2612.CH[2],SLOT3);
      FM_KEYOFF_CSM(&ym2612.CH[2],SLOT4);
      ym2612.OPN.SL3.key_csm = 0;
    }
  }

  /* timer B control */
  INTERNAL_TIMER_B(length);
}

void YM2612Config(unsigned char dac_bits)
{
  /* reinitialize TL table */
  init_tables(dac_bits);
}

int YM2612LoadContext(unsigned char *state)
{
  int c,s;
  uint8 index;
  int bufferptr = 0;

  /* restore YM2612 context */
  load_param(&ym2612, sizeof(ym2612));

  /* restore DT table address pointer for each channel slots */
  for (c=0; c<6; c++)
  {
    for (s=0; s<4; s++)
    {
      load_param(&index,sizeof(index));
      bufferptr += sizeof(index);
      ym2612.CH[c].SLOT[s].DT = ym2612.OPN.ST.dt_tab[index&7];
    }
  }

  /* restore outputs connections */
  setup_connection(&ym2612.CH[0],0);
  setup_connection(&ym2612.CH[1],1);
  setup_connection(&ym2612.CH[2],2);
  setup_connection(&ym2612.CH[3],3);
  setup_connection(&ym2612.CH[4],4);
  setup_connection(&ym2612.CH[5],5);

  return bufferptr;
}

int YM2612SaveContext(unsigned char *state)
{
  int c,s;
  uint8 index;
  int bufferptr = 0;

  /* save YM2612 context */
  save_param(&ym2612, sizeof(ym2612));

  /* save DT table index for each channel slots */
  for (c=0; c<6; c++)
  {
    for (s=0; s<4; s++)
    {
      index = (ym2612.CH[c].SLOT[s].DT - ym2612.OPN.ST.dt_tab[0]) >> 5;
      save_param(&index,sizeof(index));
      bufferptr += sizeof(index);
    }
  }

  return bufferptr;
}
