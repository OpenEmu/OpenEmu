/*
Permission is granted to anyone to use this software for any purpose,
including commercial applications. To alter this software and redistribute it freely,
if the origin of this software is not misrepresented.
*/

/* This software has been heavily modified for VRC7.  To get a stock YM2413 emulator, download
   MSXplug.
*/

/***********************************************************************************

  emu2413.c -- YM2413 emulator written by Mitsutaka Okazaki 2001

  2001 01-08 : Version 0.10 -- 1st version.
  2001 01-15 : Version 0.20 -- semi-public version.
  2001 01-16 : Version 0.30 -- 1st public version.
  2001 01-17 : Version 0.31 -- Fixed bassdrum problem.
             : Version 0.32 -- LPF implemented.
  2001 01-18 : Version 0.33 -- Fixed the drum problem, refine the mix-down method.
                            -- Fixed the LFO bug.
  2001 01-24 : Version 0.35 -- Fixed the drum problem, 
                               support undocumented EG behavior.
  2001 02-02 : Version 0.38 -- Improved the performance.
                               Fixed the hi-hat and cymbal model.
                               Fixed the default percussive datas.
                               Noise reduction.
                               Fixed the feedback problem.
  2001 03-03 : Version 0.39 -- Fixed some drum bugs.
                               Improved the performance.
  2001 03-04 : Version 0.40 -- Improved the feedback.
                               Change the default table size.
                               Clock and Rate can be changed during play.
  2001 06-24 : Version 0.50 -- Improved the hi-hat and the cymbal tone.
                               Added VRC7 patch (OPLL_reset_patch is changed).
                               Fixed OPLL_reset() bug.
                               Added OPLL_setMask, OPLL_getMask and OPLL_toggleMask.
                               Added OPLL_writeIO.
  2001 09-28 : Version 0.51 -- Removed the noise table.
  2002 01-28 : Version 0.52 -- Added Stereo mode.
  2002 02-07 : Version 0.53 -- Fixed some drum bugs.
  2002 02-20 : Version 0.54 -- Added the best quality mode.
  2002 03-02 : Version 0.55 -- Removed OPLL_init & OPLL_close.
  2002 05-30 : Version 0.60 -- Fixed HH&CYM generator and all voice datas.

  2004 01-24 : Modified by xodnizel to remove code not needed for the VRC7, among other things.

  References: 
    fmopl.c        -- 1999,2000 written by Tatsuyuki Satoh (MAME development).
    fmopl.c(fixed) -- (C) 2002 Jarek Burczynski.
    s_opl.c        -- 2001 written by Mamiya (NEZplug development).
    fmgen.cpp      -- 1999,2000 written by cisc.
    fmpac.ill      -- 2000 created by NARUTO.
    MSX-Datapack
    YMU757 data sheet
    YM2143 data sheet

**************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "emu2413.h"

static const unsigned char default_inst[15][8] = {
 #include "vrc7tone.h"
};

#define EG2DB(d) ((d)*(e_int32)(EG_STEP/DB_STEP))
#define TL2EG(d) ((d)*(e_int32)(TL_STEP/EG_STEP))
#define SL2EG(d) ((d)*(e_int32)(SL_STEP/EG_STEP))

#define DB_POS(x) (e_uint32)((x)/DB_STEP)
#define DB_NEG(x) (e_uint32)(DB_MUTE+DB_MUTE+(x)/DB_STEP)

/* Bits for liner value */
#define DB2LIN_AMP_BITS 11
#define SLOT_AMP_BITS (DB2LIN_AMP_BITS)

/* Bits for envelope phase incremental counter */
#define EG_DP_BITS 22
#define EG_DP_WIDTH (1<<EG_DP_BITS)

/* PM table is calcurated by PM_AMP * pow(2,PM_DEPTH*sin(x)/1200) */
#define PM_AMP_BITS 8
#define PM_AMP (1<<PM_AMP_BITS)

/* PM speed(Hz) and depth(cent) */
#define PM_SPEED 6.4
#define PM_DEPTH 13.75

/* AM speed(Hz) and depth(dB) */
#define AM_SPEED 3.7
//#define AM_DEPTH 4.8
#define AM_DEPTH 2.4

/* Cut the lower b bit(s) off. */
#define HIGHBITS(c,b) ((c)>>(b))

/* Leave the lower b bit(s). */
#define LOWBITS(c,b) ((c)&((1<<(b))-1))

/* Expand x which is s bits to d bits. */
#define EXPAND_BITS(x,s,d) ((x)<<((d)-(s)))

/* Expand x which is s bits to d bits and fill expanded bits '1' */
#define EXPAND_BITS_X(x,s,d) (((x)<<((d)-(s)))|((1<<((d)-(s)))-1))

#define MOD(o,x) (&(o)->slot[(x)<<1])
#define CAR(o,x) (&(o)->slot[((x)<<1)|1])

#define BIT(s,b) (((s)>>(b))&1)

/* Definition of envelope mode */
enum
{ SETTLE, ATTACK, DECAY, SUSHOLD, SUSTINE, RELEASE, FINISH };

/***************************************************
 
                  Create tables
 
****************************************************/
INLINE static e_int32 Min (e_int32 i, e_int32 j)
{
  if (i < j)
    return i;
  else
    return j;
}

/* Table for AR to LogCurve. */
static void makeAdjustTable (OPLL * opll)
{
  e_int32 i;

  opll->AR_ADJUST_TABLE[0] = (1 << EG_BITS);
  for (i = 1; i < 128; i++)
    opll->AR_ADJUST_TABLE[i] = (e_uint16) ((double) (1 << EG_BITS) - 1 - (1 << EG_BITS) * log (i) / log (128));
}


/* Table for dB(0 -- (1<<DB_BITS)-1) to Liner(0 -- DB2LIN_AMP_WIDTH) */
static void makeDB2LinTable (OPLL * opll)
{
  e_int32 i;

  for (i = 0; i < DB_MUTE + DB_MUTE; i++)
  {
    opll->DB2LIN_TABLE[i] = (e_int16) ((double) ((1 << DB2LIN_AMP_BITS) - 1) * pow (10, -(double) i * DB_STEP / 20));
    if (i >= DB_MUTE) opll->DB2LIN_TABLE[i] = 0;
    //printf("%d\n",DB2LIN_TABLE[i]);
    opll->DB2LIN_TABLE[i + DB_MUTE + DB_MUTE] = (e_int16) (-opll->DB2LIN_TABLE[i]);
  }
}

/* Liner(+0.0 - +1.0) to dB((1<<DB_BITS) - 1 -- 0) */
static e_int32 lin2db (double d)
{
  if (d == 0)
    return (DB_MUTE - 1);
  else
    return Min (-(e_int32) (20.0 * log10 (d) / DB_STEP), DB_MUTE-1);  /* 0 -- 127 */
}


/* Sin Table */
static void makeSinTable (OPLL * opll)
{
  e_int32 i;

  for (i = 0; i < PG_WIDTH / 4; i++)
  {
    opll->fullsintable[i] = (e_uint32) lin2db (sin (2.0 * PI * i / PG_WIDTH) );
  }

  for (i = 0; i < PG_WIDTH / 4; i++)
  {
    opll->fullsintable[PG_WIDTH / 2 - 1 - i] = opll->fullsintable[i];
  }

  for (i = 0; i < PG_WIDTH / 2; i++)
  {
    opll->fullsintable[PG_WIDTH / 2 + i] = (e_uint32) (DB_MUTE + DB_MUTE + opll->fullsintable[i]);
  }

  for (i = 0; i < PG_WIDTH / 2; i++)
    opll->halfsintable[i] = opll->fullsintable[i];
  for (i = PG_WIDTH / 2; i < PG_WIDTH; i++)
    opll->halfsintable[i] = opll->fullsintable[0];
}

/* Table for Pitch Modulator */
static void makePmTable (OPLL * opll)
{
  e_int32 i;

  for (i = 0; i < PM_PG_WIDTH; i++)
    opll->pmtable[i] = (e_int32) ((double) PM_AMP * pow (2, (double) PM_DEPTH * sin (2.0 * PI * i / PM_PG_WIDTH) / 1200));
}

/* Table for Amp Modulator */
static void makeAmTable (OPLL * opll)
{
  e_int32 i;

  for (i = 0; i < AM_PG_WIDTH; i++)
    opll->amtable[i] = (e_int32) ((double) AM_DEPTH / 2 / DB_STEP * (1.0 + sin (2.0 * PI * i / PM_PG_WIDTH)));
}

/* Phase increment counter table */
static void makeDphaseTable (OPLL * opll)
{
  e_uint32 fnum, block, ML;
  e_uint32 mltable[16] =
    { 1, 1 * 2, 2 * 2, 3 * 2, 4 * 2, 5 * 2, 6 * 2, 7 * 2, 8 * 2, 9 * 2, 10 * 2, 10 * 2, 12 * 2, 12 * 2, 15 * 2, 15 * 2 };

  for (fnum = 0; fnum < 512; fnum++)
    for (block = 0; block < 8; block++)
      for (ML = 0; ML < 16; ML++)
        opll->dphaseTable[fnum][block][ML] = (((fnum * mltable[ML]) << block) >> (20 - DP_BITS));
}

static void makeTllTable (OPLL *opll)
{
#define dB2(x) ((x)*2)

  static const double kltable[16] = {
    dB2 (0.000), dB2 (9.000), dB2 (12.000), dB2 (13.875), dB2 (15.000), dB2 (16.125), dB2 (16.875), dB2 (17.625),
    dB2 (18.000), dB2 (18.750), dB2 (19.125), dB2 (19.500), dB2 (19.875), dB2 (20.250), dB2 (20.625), dB2 (21.000)
  };

  e_int32 tmp;
  e_int32 fnum, block, TL, KL;

  for (fnum = 0; fnum < 16; fnum++)
    for (block = 0; block < 8; block++)
      for (TL = 0; TL < 64; TL++)
        for (KL = 0; KL < 4; KL++)
        {
          if (KL == 0)
          {
            opll->tllTable[fnum][block][TL][KL] = TL2EG (TL);
          }
          else
          {
            tmp = (e_int32) (kltable[fnum] - dB2 (3.000) * (7 - block));
            if (tmp <= 0)
              opll->tllTable[fnum][block][TL][KL] = TL2EG (TL);
            else
              opll->tllTable[fnum][block][TL][KL] = (e_uint32) ((tmp >> (3 - KL)) / EG_STEP) + TL2EG (TL);
          }
        }
}

#ifdef USE_SPEC_ENV_SPEED
static const double attacktime[16][4] = {
  {0, 0, 0, 0},
  {1730.15, 1400.60, 1153.43, 988.66},
  {865.08, 700.30, 576.72, 494.33},
  {432.54, 350.15, 288.36, 247.16},
  {216.27, 175.07, 144.18, 123.58},
  {108.13, 87.54, 72.09, 61.79},
  {54.07, 43.77, 36.04, 30.90},
  {27.03, 21.88, 18.02, 15.45},
  {13.52, 10.94, 9.01, 7.72},
  {6.76, 5.47, 4.51, 3.86},
  {3.38, 2.74, 2.25, 1.93},
  {1.69, 1.37, 1.13, 0.97},
  {0.84, 0.70, 0.60, 0.54},
  {0.50, 0.42, 0.34, 0.30},
  {0.28, 0.22, 0.18, 0.14},
  {0.00, 0.00, 0.00, 0.00}
};

static const double decaytime[16][4] = {
  {0, 0, 0, 0},
  {20926.60, 16807.20, 14006.00, 12028.60},
  {10463.30, 8403.58, 7002.98, 6014.32},
  {5231.64, 4201.79, 3501.49, 3007.16},
  {2615.82, 2100.89, 1750.75, 1503.58},
  {1307.91, 1050.45, 875.37, 751.79},
  {653.95, 525.22, 437.69, 375.90},
  {326.98, 262.61, 218.84, 187.95},
  {163.49, 131.31, 109.42, 93.97},
  {81.74, 65.65, 54.71, 46.99},
  {40.87, 32.83, 27.36, 23.49},
  {20.44, 16.41, 13.68, 11.75},
  {10.22, 8.21, 6.84, 5.87},
  {5.11, 4.10, 3.42, 2.94},
  {2.55, 2.05, 1.71, 1.47},
  {1.27, 1.27, 1.27, 1.27}
};
#endif

/* Rate Table for Attack */
static void makeDphaseARTable(OPLL * opll)
{
  e_int32 AR, Rks, RM, RL;
#ifdef USE_SPEC_ENV_SPEED
  e_uint32 attacktable[16][4];

  for (RM = 0; RM < 16; RM++)
    for (RL = 0; RL < 4; RL++)
    {
      if (RM == 0)
        attacktable[RM][RL] = 0;
      else if (RM == 15)
        attacktable[RM][RL] = EG_DP_WIDTH;
      else
        attacktable[RM][RL] = (e_uint32) ((double) (1 << EG_DP_BITS) / (attacktime[RM][RL] * 3579545 / 72000));

    }
#endif

  for (AR = 0; AR < 16; AR++)
    for (Rks = 0; Rks < 16; Rks++)
    {
      RM = AR + (Rks >> 2);
      RL = Rks & 3;
      if (RM > 15)
        RM = 15;
      switch (AR)
      {
      case 0:
        opll->dphaseARTable[AR][Rks] = 0;
        break;
      case 15:
        opll->dphaseARTable[AR][Rks] = 0;/*EG_DP_WIDTH;*/ 
        break;
      default:
#ifdef USE_SPEC_ENV_SPEED
        opll->dphaseARTable[AR][Rks] = (attacktable[RM][RL]);
#else
        opll->dphaseARTable[AR][Rks] = ((3 * (RL + 4) << (RM + 1)));
#endif
        break;
      }
    }
}

/* Rate Table for Decay and Release */
static void makeDphaseDRTable (OPLL * opll)
{
  e_int32 DR, Rks, RM, RL;

#ifdef USE_SPEC_ENV_SPEED
  e_uint32 decaytable[16][4];

  for (RM = 0; RM < 16; RM++)
    for (RL = 0; RL < 4; RL++)
      if (RM == 0)
        decaytable[RM][RL] = 0;
      else
        decaytable[RM][RL] = (e_uint32) ((double) (1 << EG_DP_BITS) / (decaytime[RM][RL] * 3579545 / 72000));
#endif

  for (DR = 0; DR < 16; DR++)
    for (Rks = 0; Rks < 16; Rks++)
    {
      RM = DR + (Rks >> 2);
      RL = Rks & 3;
      if (RM > 15)
        RM = 15;
      switch (DR)
      {
      case 0:
        opll->dphaseDRTable[DR][Rks] = 0;
        break;
      default:
#ifdef USE_SPEC_ENV_SPEED
        opll->dphaseDRTable[DR][Rks] = (decaytable[RM][RL]);
#else
        opll->dphaseDRTable[DR][Rks] = ((RL + 4) << (RM - 1));
#endif
        break;
      }
    }
}

static void makeRksTable (OPLL *opll)
{

  e_int32 fnum8, block, KR;

  for (fnum8 = 0; fnum8 < 2; fnum8++)
    for (block = 0; block < 8; block++)
      for (KR = 0; KR < 2; KR++)
      {
        if (KR != 0)
          opll->rksTable[fnum8][block][KR] = (block << 1) + fnum8;
        else
          opll->rksTable[fnum8][block][KR] = block >> 1;
      }
}

/************************************************************

                      Calc Parameters

************************************************************/

INLINE static e_uint32 calc_eg_dphase (OPLL *opll, OPLL_SLOT * slot)
{

  switch (slot->eg_mode)
  {
  case ATTACK:
    return opll->dphaseARTable[slot->patch.AR][slot->rks];

  case DECAY:
    return opll->dphaseDRTable[slot->patch.DR][slot->rks];

  case SUSHOLD:
    return 0;

  case SUSTINE:
    return opll->dphaseDRTable[slot->patch.RR][slot->rks];

  case RELEASE:
    if (slot->sustine)
      return opll->dphaseDRTable[5][slot->rks];
    else if (slot->patch.EG)
      return opll->dphaseDRTable[slot->patch.RR][slot->rks];
    else
      return opll->dphaseDRTable[7][slot->rks];

  case FINISH:
    return 0;

  default:
    return 0;
  }
}

/*************************************************************

                    OPLL internal interfaces

*************************************************************/

#define UPDATE_PG(S)  (S)->dphase = opll->dphaseTable[(S)->fnum][(S)->block][(S)->patch.ML]
#define UPDATE_TLL(S)\
(((S)->type==0)?\
((S)->tll = opll->tllTable[((S)->fnum)>>5][(S)->block][(S)->patch.TL][(S)->patch.KL]):\
((S)->tll = opll->tllTable[((S)->fnum)>>5][(S)->block][(S)->volume][(S)->patch.KL]))
#define UPDATE_RKS(S) (S)->rks = opll->rksTable[((S)->fnum)>>8][(S)->block][(S)->patch.KR]
#define UPDATE_WF(S)  (S)->sintbl = opll->waveform[(S)->patch.WF]
#define UPDATE_EG(S)  (S)->eg_dphase = calc_eg_dphase(opll,S)
#define UPDATE_ALL(S)\
  UPDATE_PG(S);\
  UPDATE_TLL(S);\
  UPDATE_RKS(S);\
  UPDATE_WF(S); \
  UPDATE_EG(S)                  /* EG should be updated last. */


/* Slot key on  */
INLINE static void slotOn (OPLL_SLOT * slot)
{
  slot->eg_mode = ATTACK;
  slot->eg_phase = 0;
  slot->phase = 0;
}

/* Slot key on without reseting the phase */
INLINE static void slotOn2 (OPLL_SLOT * slot)
{
  slot->eg_mode = ATTACK;
  slot->eg_phase = 0;
}

/* Slot key off */
INLINE static void slotOff (OPLL *opll, OPLL_SLOT * slot)
{
  if (slot->eg_mode == ATTACK)
    slot->eg_phase = EXPAND_BITS (opll->AR_ADJUST_TABLE[HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS)], EG_BITS, EG_DP_BITS);
  slot->eg_mode = RELEASE;
}

/* Channel key on */
INLINE static void keyOn (OPLL * opll, e_int32 i)
{
  if (!opll->slot_on_flag[i * 2])
    slotOn (MOD(opll,i));
  if (!opll->slot_on_flag[i * 2 + 1])
    slotOn (CAR(opll,i));
  opll->key_status[i] = 1;
}

/* Channel key off */
INLINE static void keyOff (OPLL * opll, e_int32 i)
{
  if (opll->slot_on_flag[i * 2 + 1])
    slotOff (opll, CAR(opll,i));
  opll->key_status[i] = 0;
}

/* Set sustine parameter */
INLINE static void setSustine (OPLL * opll, e_int32 c, e_int32 sustine)
{
  CAR(opll,c)->sustine = sustine;
  if (MOD(opll,c)->type)
    MOD(opll,c)->sustine = sustine;
}

/* Volume : 6bit ( Volume register << 2 ) */
INLINE static void setVolume (OPLL * opll, e_int32 c, e_int32 volume)
{
  CAR(opll,c)->volume = volume;
}

INLINE static void setSlotVolume (OPLL_SLOT * slot, e_int32 volume)
{
  slot->volume = volume;
}

/* Set F-Number ( fnum : 9bit ) */
INLINE static void setFnumber (OPLL * opll, e_int32 c, e_int32 fnum)
{
  CAR(opll,c)->fnum = fnum;
  MOD(opll,c)->fnum = fnum;
}

/* Set Block data (block : 3bit ) */
INLINE static void setBlock (OPLL * opll, e_int32 c, e_int32 block)
{
  CAR(opll,c)->block = block;
  MOD(opll,c)->block = block;
}

INLINE static void update_key_status (OPLL * opll)
{
  int ch;

  for (ch = 0; ch < 6; ch++)
    opll->slot_on_flag[ch * 2] = opll->slot_on_flag[ch * 2 + 1] = (opll->HiFreq[ch]) & 0x10;
}

/***********************************************************

                      Initializing

***********************************************************/

static void OPLL_SLOT_reset (OPLL *opll, OPLL_SLOT * slot, int type)
{
  slot->type = type;
  slot->sintbl = opll->waveform[0];
  slot->phase = 0;
  slot->dphase = 0;
  slot->output[0] = 0;
  slot->output[1] = 0;
  slot->feedback = 0;
  slot->eg_mode = SETTLE;
  slot->eg_phase = EG_DP_WIDTH;
  slot->eg_dphase = 0;
  slot->rks = 0;
  slot->tll = 0;
  slot->sustine = 0;
  slot->fnum = 0;
  slot->block = 0;
  slot->volume = 0;
  slot->pgout = 0;
  slot->egout = 0;
}

static void internal_refresh (OPLL *opll)
{
  makeDphaseTable (opll);
  makeDphaseARTable (opll);
  makeDphaseDRTable (opll);
  opll->pm_dphase = (e_uint32)  (PM_SPEED * PM_DP_WIDTH / (opll->clk / 72));
  opll->am_dphase = (e_uint32)  (AM_SPEED * AM_DP_WIDTH / (opll->clk / 72));
}

static void maketables (OPLL *opll, e_uint32 c)
{
    opll->clk = c;
    makePmTable (opll);
    makeAmTable (opll);
    makeDB2LinTable (opll);
    makeAdjustTable (opll);
    makeTllTable (opll);
    makeRksTable (opll);
    makeSinTable (opll);
    //makeDefaultPatch ();
  internal_refresh (opll);
}

OPLL *OPLL_new (e_uint32 clk)
{
  OPLL *opll;

  opll = (OPLL *) calloc (sizeof (OPLL), 1);
  if (opll == NULL)
    return NULL;
  maketables(opll,clk);

  opll->waveform[0]=opll->fullsintable;
  opll->waveform[1]=opll->halfsintable;

  opll->mask = 0;

  OPLL_reset (opll);

  return opll;
}


void OPLL_delete (OPLL * opll)
{
  free (opll);
}

/* Reset whole of OPLL except patch datas. */
void OPLL_reset (OPLL * opll)
{
  e_int32 i;

  if (!opll)
    return;

  opll->adr = 0;
  opll->out = 0;

  opll->pm_phase = 0;
  opll->am_phase = 0;

  opll->mask = 0;

  for (i = 0; i < 12; i++)
    OPLL_SLOT_reset(opll, &opll->slot[i], i%2);

  for (i = 0; i < 6; i++)
  {
    opll->key_status[i] = 0;
    //setPatch (opll, i, 0);
  }

  for (i = 0; i < 0x40; i++)
    OPLL_writeReg (opll, i, 0);
}

/* Force Refresh (When external program changes some parameters). */
void OPLL_forceRefresh (OPLL * opll)
{
  e_int32 i;

  if (opll == NULL)
    return;

  for (i = 0; i < 12; i++)
  {
    UPDATE_PG (&opll->slot[i]);
    UPDATE_RKS (&opll->slot[i]);
    UPDATE_TLL (&opll->slot[i]);
    UPDATE_WF (&opll->slot[i]);
    UPDATE_EG (&opll->slot[i]);
  }
}

void OPLL_set_rate (OPLL * opll, e_uint32 r)
{
  internal_refresh (opll);
}

/*********************************************************

                 Generate wave data

*********************************************************/
/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 2PI). */
#if ( SLOT_AMP_BITS - PG_BITS ) > 0
#define wave2_2pi(e)  ( (e) >> ( SLOT_AMP_BITS - PG_BITS ))
#else
#define wave2_2pi(e)  ( (e) << ( PG_BITS - SLOT_AMP_BITS ))
#endif

/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 4PI). */
#if ( SLOT_AMP_BITS - PG_BITS - 1 ) == 0
#define wave2_4pi(e)  (e)
#elif ( SLOT_AMP_BITS - PG_BITS - 1 ) > 0
#define wave2_4pi(e)  ( (e) >> ( SLOT_AMP_BITS - PG_BITS - 1 ))
#else
#define wave2_4pi(e)  ( (e) << ( 1 + PG_BITS - SLOT_AMP_BITS ))
#endif

/* Convert Amp(0 to EG_HEIGHT) to Phase(0 to 8PI). */
#if ( SLOT_AMP_BITS - PG_BITS - 2 ) == 0
#define wave2_8pi(e)  (e)
#elif ( SLOT_AMP_BITS - PG_BITS - 2 ) > 0
#define wave2_8pi(e)  ( (e) >> ( SLOT_AMP_BITS - PG_BITS - 2 ))
#else
#define wave2_8pi(e)  ( (e) << ( 2 + PG_BITS - SLOT_AMP_BITS ))
#endif



/* Update AM, PM unit */
static void update_ampm (OPLL * opll)
{
  opll->pm_phase = (opll->pm_phase + opll->pm_dphase) & (PM_DP_WIDTH - 1);
  opll->am_phase = (opll->am_phase + opll->am_dphase) & (AM_DP_WIDTH - 1);
  opll->lfo_am = opll->amtable[HIGHBITS (opll->am_phase, AM_DP_BITS - AM_PG_BITS)];
  opll->lfo_pm = opll->pmtable[HIGHBITS (opll->pm_phase, PM_DP_BITS - PM_PG_BITS)];
}

/* PG */
INLINE static void
calc_phase (OPLL_SLOT * slot, e_int32 lfo)
{
  if (slot->patch.PM)
    slot->phase += (slot->dphase * lfo) >> PM_AMP_BITS;
  else
    slot->phase += slot->dphase;

  slot->phase &= (DP_WIDTH - 1);

  slot->pgout = HIGHBITS (slot->phase, DP_BASE_BITS);
}

/* EG */
static void calc_envelope(OPLL *opll, OPLL_SLOT * slot, e_int32 lfo)
{
#define S2E(x) (SL2EG((e_int32)(x/SL_STEP))<<(EG_DP_BITS-EG_BITS))

  static const e_uint32 SL[16] = {
    S2E (0.0), S2E (3.0), S2E (6.0), S2E (9.0), S2E (12.0), S2E (15.0), S2E (18.0), S2E (21.0),
    S2E (24.0), S2E (27.0), S2E (30.0), S2E (33.0), S2E (36.0), S2E (39.0), S2E (42.0), S2E (48.0)
  };

  e_uint32 egout;

  switch (slot->eg_mode)
  {

  case ATTACK:
    egout = opll->AR_ADJUST_TABLE[HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS)];
    slot->eg_phase += slot->eg_dphase;
    if((EG_DP_WIDTH & slot->eg_phase)||(slot->patch.AR==15))
    {
      egout = 0;
      slot->eg_phase = 0;
      slot->eg_mode = DECAY;
      UPDATE_EG (slot);
    }
    break;

  case DECAY:
    egout = HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS);
    slot->eg_phase += slot->eg_dphase;
    if (slot->eg_phase >= SL[slot->patch.SL])
    {
      if (slot->patch.EG)
      {
        slot->eg_phase = SL[slot->patch.SL];
        slot->eg_mode = SUSHOLD;
        UPDATE_EG (slot);
      }
      else
      {
        slot->eg_phase = SL[slot->patch.SL];
        slot->eg_mode = SUSTINE;
        UPDATE_EG (slot);
      }
    }
    break;

  case SUSHOLD:
    egout = HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS);
    if (slot->patch.EG == 0)
    {
      slot->eg_mode = SUSTINE;
      UPDATE_EG (slot);
    }
    break;

  case SUSTINE:
  case RELEASE:
    egout = HIGHBITS (slot->eg_phase, EG_DP_BITS - EG_BITS);
    slot->eg_phase += slot->eg_dphase;
    if (egout >= (1 << EG_BITS))
    {
      slot->eg_mode = FINISH;
      egout = (1 << EG_BITS) - 1;
    }
    break;

  case FINISH:
    egout = (1 << EG_BITS) - 1;
    break;

  default:
    egout = (1 << EG_BITS) - 1;
    break;
  }

  if (slot->patch.AM)
    egout = EG2DB (egout + slot->tll) + lfo;
  else
    egout = EG2DB (egout + slot->tll);

  if (egout >= DB_MUTE)
    egout = DB_MUTE - 1;
  
  slot->egout = egout;
}

/* CARRIOR */
INLINE static e_int32 calc_slot_car(OPLL *opll, OPLL_SLOT * slot, e_int32 fm)
{
  slot->output[1] = slot->output[0];

  if (slot->egout >= (DB_MUTE - 1))
  {
    slot->output[0] = 0;
  }
  else
  {
    slot->output[0] = opll->DB2LIN_TABLE[slot->sintbl[(slot->pgout+wave2_8pi(fm))&(PG_WIDTH-1)] + slot->egout];
  }

  return (slot->output[1] + slot->output[0]) >> 1;
}

/* MODULATOR */
INLINE static e_int32 calc_slot_mod(OPLL *opll, OPLL_SLOT * slot)
{
  e_int32 fm;

  slot->output[1] = slot->output[0];

  if (slot->egout >= (DB_MUTE - 1))
  {
    slot->output[0] = 0;
  }
  else if (slot->patch.FB != 0)
  {
    fm = wave2_4pi (slot->feedback) >> (7 - slot->patch.FB);
    slot->output[0] = opll->DB2LIN_TABLE[slot->sintbl[(slot->pgout + fm)&(PG_WIDTH-1)] + slot->egout];
  }
  else
  {
    slot->output[0] = opll->DB2LIN_TABLE[slot->sintbl[slot->pgout] + slot->egout];
  }

  slot->feedback = (slot->output[1] + slot->output[0]) >> 1;

  return slot->feedback;

}

static INLINE e_int16 calc (OPLL * opll)
{
  e_int32 inst = 0, out = 0;
  e_int32 i;

  update_ampm (opll);

  for (i = 0; i < 12; i++)
  {
    calc_phase(&opll->slot[i],opll->lfo_pm);
    calc_envelope(opll, &opll->slot[i],opll->lfo_am);
  }

  for (i = 0; i < 6; i++)
    if (!(opll->mask & OPLL_MASK_CH (i)) && (CAR(opll,i)->eg_mode != FINISH))
      inst += calc_slot_car (opll, CAR(opll,i), calc_slot_mod(opll,MOD(opll,i)));

  out = inst;
  return (e_int16) out;
}

e_int16 OPLL_calc (OPLL * opll)
{
  return calc (opll);
}

e_uint32
OPLL_setMask (OPLL * opll, e_uint32 mask)
{
  e_uint32 ret;

  if (opll)
  {
    ret = opll->mask;
    opll->mask = mask;
    return ret;
  }
  else
    return 0;
}

e_uint32
OPLL_toggleMask (OPLL * opll, e_uint32 mask)
{
  e_uint32 ret;

  if (opll)
  {
    ret = opll->mask;
    opll->mask ^= mask;
    return ret;
  }
  else
    return 0;
}

/****************************************************

                       I/O Ctrl

*****************************************************/

static void setInstrument(OPLL * opll, e_uint i, e_uint inst)
{
 const e_uint8 *src;
 OPLL_PATCH *modp, *carp;

 opll->patch_number[i]=inst;

 if(inst)
  src=default_inst[inst-1];
 else
  src=opll->CustInst;

 modp=&MOD(opll,i)->patch;
 carp=&CAR(opll,i)->patch;

 modp->AM=(src[0]>>7)&1;
 modp->PM=(src[0]>>6)&1;
 modp->EG=(src[0]>>5)&1;
 modp->KR=(src[0]>>4)&1;
 modp->ML=(src[0]&0xF);

 carp->AM=(src[1]>>7)&1;
 carp->PM=(src[1]>>6)&1;
 carp->EG=(src[1]>>5)&1;
 carp->KR=(src[1]>>4)&1;
 carp->ML=(src[1]&0xF);  

 modp->KL=(src[2]>>6)&3;
 modp->TL=(src[2]&0x3F);

 carp->KL = (src[3] >> 6) & 3; 
 carp->WF = (src[3] >> 4) & 1;

 modp->WF = (src[3] >> 3) & 1;

 modp->FB = (src[3]) & 7;

 modp->AR = (src[4]>>4)&0xF;
 modp->DR = (src[4]&0xF);

 carp->AR = (src[5]>>4)&0xF;
 carp->DR = (src[5]&0xF);

 modp->SL = (src[6]>>4)&0xF;
 modp->RR = (src[6]&0xF);
 
 carp->SL = (src[7]>>4)&0xF;
 carp->RR = (src[7]&0xF);
}


void OPLL_writeReg (OPLL * opll, e_uint32 reg, e_uint32 data)
{

  e_int32 i, v, ch;

  data = data & 0xff;
  reg = reg & 0x3f;

  switch (reg)
  {
   case 0x00:
    opll->CustInst[0]=data;
    for (i = 0; i < 6; i++)
    {
      if (opll->patch_number[i] == 0)
      {
	setInstrument(opll, i, 0);
        UPDATE_PG (MOD(opll,i));
        UPDATE_RKS (MOD(opll,i));
        UPDATE_EG (MOD(opll,i));
      }
    }
    break;

  case 0x01:
    opll->CustInst[1]=data;
    for (i = 0; i < 6; i++)
    {
      if (opll->patch_number[i] == 0)
      {
	setInstrument(opll, i, 0);
        UPDATE_PG (CAR(opll,i));
        UPDATE_RKS (CAR(opll,i));
        UPDATE_EG (CAR(opll,i));
      }
    }
    break;

  case 0x02:
    opll->CustInst[2]=data;
    for (i = 0; i < 6; i++)
    {
      if (opll->patch_number[i] == 0)
      {
	setInstrument(opll, i, 0);
        UPDATE_TLL(MOD(opll,i));
      }
    }
    break;

  case 0x03:
    opll->CustInst[3]=data;
    for (i = 0; i < 6; i++)
    {
      if (opll->patch_number[i] == 0)
      {
	setInstrument(opll, i, 0);
        UPDATE_WF(MOD(opll,i));
        UPDATE_WF(CAR(opll,i));
      }
    }
    break;

  case 0x04:
    opll->CustInst[4]=data;
    for (i = 0; i < 6; i++)
    {
      if (opll->patch_number[i] == 0)
      {
	setInstrument(opll, i, 0);
        UPDATE_EG (MOD(opll,i));
      }
    }
    break;

  case 0x05:
    opll->CustInst[5]=data;
    for (i = 0; i < 6; i++)
    {
      if (opll->patch_number[i] == 0)
      {
	setInstrument(opll, i, 0);
        UPDATE_EG(CAR(opll,i));
      }
    }
    break;

  case 0x06:
    opll->CustInst[6]=data;
    for (i = 0; i < 6; i++)
    {
      if (opll->patch_number[i] == 0)
      {
	setInstrument(opll, i, 0);
        UPDATE_EG (MOD(opll,i));
      }
    }
    break;

  case 0x07:
    opll->CustInst[7]=data;
    for (i = 0; i < 6; i++)
    {
      if (opll->patch_number[i] == 0)
      {
	setInstrument(opll, i, 0);
         UPDATE_EG (CAR(opll,i));
      }
    }
    break;

  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
  case 0x14:
  case 0x15:
    ch = reg - 0x10;
    opll->LowFreq[ch]=data;
    setFnumber (opll, ch, data + ((opll->HiFreq[ch] & 1) << 8));
    UPDATE_ALL (MOD(opll,ch));
    UPDATE_ALL (CAR(opll,ch));
    break;

  case 0x20:
  case 0x21:
  case 0x22:
  case 0x23:
  case 0x24:
  case 0x25:
    ch = reg - 0x20;
    opll->HiFreq[ch]=data;

    setFnumber (opll, ch, ((data & 1) << 8) + opll->LowFreq[ch]);
    setBlock (opll, ch, (data >> 1) & 7);
    setSustine (opll, ch, (data >> 5) & 1);
    if (data & 0x10)
      keyOn (opll, ch);
    else
      keyOff (opll, ch);
    UPDATE_ALL (MOD(opll,ch));
    UPDATE_ALL (CAR(opll,ch));
    update_key_status (opll);
    break;

  case 0x30:
  case 0x31:
  case 0x32:
  case 0x33:
  case 0x34:
  case 0x35:
    opll->InstVol[reg-0x30]=data;
    i = (data >> 4) & 15;
    v = data & 15;
    setInstrument(opll, reg-0x30, i);
    setVolume (opll, reg - 0x30, v << 2);
    UPDATE_ALL (MOD(opll,reg - 0x30));
    UPDATE_ALL (CAR(opll,reg - 0x30));
    break;

  default:
    break;

  }
}

void OPLL_writeIO (OPLL * opll, e_uint32 adr, e_uint32 val)
{
  if (adr & 1)
    OPLL_writeReg (opll, opll->adr, val);
  else
    opll->adr = val;
}
