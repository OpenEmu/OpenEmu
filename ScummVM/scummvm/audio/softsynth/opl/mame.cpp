/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * LGPL licensed version of MAMEs fmopl (V0.37a modified) by
 * Tatsuyuki Satoh. Included from LGPL'ed AdPlug.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "mame.h"

#include "common/textconsole.h"
#include "common/util.h"

#if defined(_WIN32_WCE) || defined(__SYMBIAN32__) || defined(__GP32__) || defined(GP2X) || defined(__MAEMO__) || defined(__DS__) || defined(__MINT__) || defined(__N64__)
#include "common/config-manager.h"
#endif

#if defined(__DS__)
#include "dsmain.h"
#endif

namespace OPL {
namespace MAME {

OPL::~OPL() {
	MAME::OPLDestroy(_opl);
	_opl = 0;
}

bool OPL::init(int rate) {
	if (_opl)
		MAME::OPLDestroy(_opl);

	_opl = MAME::makeAdLibOPL(rate);
	return (_opl != 0);
}

void OPL::reset() {
	MAME::OPLResetChip(_opl);
}

void OPL::write(int a, int v) {
	MAME::OPLWrite(_opl, a, v);
}

byte OPL::read(int a) {
	return MAME::OPLRead(_opl, a);
}

void OPL::writeReg(int r, int v) {
	MAME::OPLWriteReg(_opl, r, v);
}

void OPL::readBuffer(int16 *buffer, int length) {
	MAME::YM3812UpdateOne(_opl, buffer, length);
}

/* -------------------- preliminary define section --------------------- */
/* attack/decay rate time rate */
#define OPL_ARRATE     141280  /* RATE 4 =  2826.24ms @ 3.6MHz */
#define OPL_DRRATE    1956000  /* RATE 4 = 39280.64ms @ 3.6MHz */

#define FREQ_BITS 24			/* frequency turn          */

/* counter bits = 20 , octerve 7 */
#define FREQ_RATE   (1<<(FREQ_BITS-20))
#define TL_BITS    (FREQ_BITS+2)

/* final output shift , limit minimum and maximum */
#define OPL_OUTSB   (TL_BITS+3-16)		/* OPL output final shift 16bit */
#define OPL_MAXOUT (0x7fff<<OPL_OUTSB)
#define OPL_MINOUT (-0x8000<<OPL_OUTSB)

/* -------------------- quality selection --------------------- */

/* sinwave entries */
/* used static memory = SIN_ENT * 4 (byte) */
#ifdef __DS__
#define SIN_ENT_SHIFT 8
#else
#define SIN_ENT_SHIFT 11
#endif
#define SIN_ENT (1<<SIN_ENT_SHIFT)

/* output level entries (envelope,sinwave) */
/* envelope counter lower bits */
int ENV_BITS;
/* envelope output entries */
int EG_ENT;

/* used dynamic memory = EG_ENT*4*4(byte)or EG_ENT*6*4(byte) */
/* used static  memory = EG_ENT*4 (byte)                     */
int EG_OFF;								 /* OFF */
int EG_DED;
int EG_DST;								 /* DECAY START */
int EG_AED;
#define EG_AST   0                       /* ATTACK START */

#define EG_STEP (96.0/EG_ENT) /* OPL is 0.1875 dB step  */

/* LFO table entries */
#define VIB_ENT 512
#define VIB_SHIFT (32-9)
#define AMS_ENT 512
#define AMS_SHIFT (32-9)

#define VIB_RATE_SHIFT 8
#define VIB_RATE (1<<VIB_RATE_SHIFT)

/* -------------------- local defines , macros --------------------- */

/* register number to channel number , slot offset */
#define SLOT1 0
#define SLOT2 1

/* envelope phase */
#define ENV_MOD_RR  0x00
#define ENV_MOD_DR  0x01
#define ENV_MOD_AR  0x02

/* -------------------- tables --------------------- */
static const int slot_array[32] = {
	 0, 2, 4, 1, 3, 5,-1,-1,
	 6, 8,10, 7, 9,11,-1,-1,
	12,14,16,13,15,17,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1
};

static uint KSL_TABLE[8 * 16];

static const double KSL_TABLE_SEED[8 * 16] = {
	/* OCT 0 */
	0.000, 0.000, 0.000, 0.000,
	0.000, 0.000, 0.000, 0.000,
	0.000, 0.000, 0.000, 0.000,
	0.000, 0.000, 0.000, 0.000,
	/* OCT 1 */
	0.000, 0.000, 0.000, 0.000,
	0.000, 0.000, 0.000, 0.000,
	0.000, 0.750, 1.125, 1.500,
	1.875, 2.250, 2.625, 3.000,
	/* OCT 2 */
	0.000, 0.000, 0.000, 0.000,
	0.000, 1.125, 1.875, 2.625,
	3.000, 3.750, 4.125, 4.500,
	4.875, 5.250, 5.625, 6.000,
	/* OCT 3 */
	0.000, 0.000, 0.000, 1.875,
	3.000, 4.125, 4.875, 5.625,
	6.000, 6.750, 7.125, 7.500,
	7.875, 8.250, 8.625, 9.000,
	/* OCT 4 */
	0.000, 0.000, 3.000, 4.875,
	6.000, 7.125, 7.875, 8.625,
	9.000, 9.750, 10.125, 10.500,
	10.875, 11.250, 11.625, 12.000,
	/* OCT 5 */
	0.000, 3.000, 6.000, 7.875,
	9.000, 10.125, 10.875, 11.625,
	12.000, 12.750, 13.125, 13.500,
	13.875, 14.250, 14.625, 15.000,
	/* OCT 6 */
	0.000, 6.000, 9.000, 10.875,
	12.000, 13.125, 13.875, 14.625,
	15.000, 15.750, 16.125, 16.500,
	16.875, 17.250, 17.625, 18.000,
	/* OCT 7 */
	0.000, 9.000, 12.000, 13.875,
	15.000, 16.125, 16.875, 17.625,
	18.000, 18.750, 19.125, 19.500,
	19.875, 20.250, 20.625, 21.000
};

/* sustain level table (3db per step) */
/* 0 - 15: 0, 3, 6, 9,12,15,18,21,24,27,30,33,36,39,42,93 (dB)*/

static int SL_TABLE[16];

static const uint SL_TABLE_SEED[16] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 31
};

#define TL_MAX (EG_ENT * 2) /* limit(tl + ksr + envelope) + sinwave */
/* TotalLevel : 48 24 12  6  3 1.5 0.75 (dB) */
/* TL_TABLE[ 0      to TL_MAX          ] : plus  section */
/* TL_TABLE[ TL_MAX to TL_MAX+TL_MAX-1 ] : minus section */
static int *TL_TABLE;

/* pointers to TL_TABLE with sinwave output offset */
static int **SIN_TABLE;

/* LFO table */
static int *AMS_TABLE;
static int *VIB_TABLE;

/* envelope output curve table */
/* attack + decay + OFF */
//static int ENV_CURVE[2*EG_ENT+1];
//static int ENV_CURVE[2 * 4096 + 1];   // to keep it static ...
static int *ENV_CURVE;


/* multiple table */
#define ML(a) (int)(a * 2)
static const uint MUL_TABLE[16]= {
/* 1/2, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 */
	ML(0.50), ML(1.00), ML(2.00),  ML(3.00), ML(4.00), ML(5.00), ML(6.00), ML(7.00),
	ML(8.00), ML(9.00), ML(10.00), ML(10.00),ML(12.00),ML(12.00),ML(15.00),ML(15.00)
};
#undef ML

/* dummy attack / decay rate ( when rate == 0 ) */
static int RATE_0[16]=
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/* -------------------- static state --------------------- */

/* lock level of common table */
static int num_lock = 0;

/* work table */
static void *cur_chip = NULL;	/* current chip point */
/* currenct chip state */
/* static OPLSAMPLE  *bufL,*bufR; */
static OPL_CH *S_CH;
static OPL_CH *E_CH;
OPL_SLOT *SLOT7_1, *SLOT7_2, *SLOT8_1, *SLOT8_2;

static int outd[1];
static int ams;
static int vib;
int *ams_table;
int *vib_table;
static int amsIncr;
static int vibIncr;
static int feedback2;		/* connect for SLOT 2 */

/* --------------------- rebuild tables ------------------- */

#define SC_KSL(mydb) ((uint) (mydb / (EG_STEP / 2)))
#define SC_SL(db) (int)(db * ((3 / EG_STEP) * (1 << ENV_BITS))) + EG_DST

void OPLBuildTables(int ENV_BITS_PARAM, int EG_ENT_PARAM) {
	int i;

	ENV_BITS = ENV_BITS_PARAM;
	EG_ENT = EG_ENT_PARAM;
	EG_OFF = ((2 * EG_ENT)<<ENV_BITS);  /* OFF          */
	EG_DED = EG_OFF;
	EG_DST = (EG_ENT << ENV_BITS);     /* DECAY  START */
	EG_AED = EG_DST;
	//EG_STEP = (96.0/EG_ENT);

	for (i = 0; i < ARRAYSIZE(KSL_TABLE_SEED); i++)
		KSL_TABLE[i] = SC_KSL(KSL_TABLE_SEED[i]);

	for (i = 0; i < ARRAYSIZE(SL_TABLE_SEED); i++)
		SL_TABLE[i] = SC_SL(SL_TABLE_SEED[i]);
}

#undef SC_KSL
#undef SC_SL

/* --------------------- subroutines  --------------------- */

/* status set and IRQ handling */
inline void OPL_STATUS_SET(FM_OPL *OPL, int flag) {
	/* set status flag */
	OPL->status |= flag;
	if (!(OPL->status & 0x80)) {
		if (OPL->status & OPL->statusmask) {	/* IRQ on */
			OPL->status |= 0x80;
			/* callback user interrupt handler (IRQ is OFF to ON) */
			if (OPL->IRQHandler)
				(OPL->IRQHandler)(OPL->IRQParam,1);
		}
	}
}

/* status reset and IRQ handling */
inline void OPL_STATUS_RESET(FM_OPL *OPL, int flag) {
	/* reset status flag */
	OPL->status &= ~flag;
	if ((OPL->status & 0x80)) {
		if (!(OPL->status & OPL->statusmask)) {
			OPL->status &= 0x7f;
			/* callback user interrupt handler (IRQ is ON to OFF) */
			if (OPL->IRQHandler) (OPL->IRQHandler)(OPL->IRQParam,0);
		}
	}
}

/* IRQ mask set */
inline void OPL_STATUSMASK_SET(FM_OPL *OPL, int flag) {
	OPL->statusmask = flag;
	/* IRQ handling check */
	OPL_STATUS_SET(OPL,0);
	OPL_STATUS_RESET(OPL,0);
}

/* ----- key on  ----- */
inline void OPL_KEYON(OPL_SLOT *SLOT) {
	/* sin wave restart */
	SLOT->Cnt = 0;
	/* set attack */
	SLOT->evm = ENV_MOD_AR;
	SLOT->evs = SLOT->evsa;
	SLOT->evc = EG_AST;
	SLOT->eve = EG_AED;
}

/* ----- key off ----- */
inline void OPL_KEYOFF(OPL_SLOT *SLOT) {
	if (SLOT->evm > ENV_MOD_RR) {
		/* set envelope counter from envleope output */

		// WORKAROUND: The Kyra engine does something very strange when
		// starting a new song. For each channel:
		//
		// * The release rate is set to "fastest".
		// * Any note is keyed off.
		// * A very low-frequency note is keyed on.
		//
		// Usually, what happens next is that the real notes is keyed
		// on immediately, in which case there's no problem.
		//
		// However, if the note is again keyed off (because the channel
		// begins on a rest rather than a note), the envelope counter
		// was moved from the very lowest point on the attack curve to
		// the very highest point on the release curve.
		//
		// Again, this might not be a problem, if the release rate is
		// still set to "fastest". But in many cases, it had already
		// been increased. And, possibly because of inaccuracies in the
		// envelope generator, that would cause the note to "fade out"
		// for quite a long time.
		//
		// What we really need is a way to find the correct starting
		// point for the envelope counter, and that may be what the
		// commented-out line below is meant to do. For now, simply
		// handle the pathological case.

		if (SLOT->evm == ENV_MOD_AR && SLOT->evc == EG_AST)
			SLOT->evc = EG_DED;
		else if (!(SLOT->evc & EG_DST))
			//SLOT->evc = (ENV_CURVE[SLOT->evc>>ENV_BITS]<<ENV_BITS) + EG_DST;
			SLOT->evc = EG_DST;
		SLOT->eve = EG_DED;
		SLOT->evs = SLOT->evsr;
		SLOT->evm = ENV_MOD_RR;
	}
}

/* ---------- calcrate Envelope Generator & Phase Generator ---------- */

/* return : envelope output */
inline uint OPL_CALC_SLOT(OPL_SLOT *SLOT) {
	/* calcrate envelope generator */
	if ((SLOT->evc += SLOT->evs) >= SLOT->eve) {
		switch (SLOT->evm) {
		case ENV_MOD_AR: /* ATTACK -> DECAY1 */
			/* next DR */
			SLOT->evm = ENV_MOD_DR;
			SLOT->evc = EG_DST;
			SLOT->eve = SLOT->SL;
			SLOT->evs = SLOT->evsd;
			break;
		case ENV_MOD_DR: /* DECAY -> SL or RR */
			SLOT->evc = SLOT->SL;
			SLOT->eve = EG_DED;
			if (SLOT->eg_typ) {
				SLOT->evs = 0;
			} else {
				SLOT->evm = ENV_MOD_RR;
				SLOT->evs = SLOT->evsr;
			}
			break;
		case ENV_MOD_RR: /* RR -> OFF */
			SLOT->evc = EG_OFF;
			SLOT->eve = EG_OFF + 1;
			SLOT->evs = 0;
			break;
		}
	}
	/* calcrate envelope */
	return SLOT->TLL + ENV_CURVE[SLOT->evc>>ENV_BITS] + (SLOT->ams ? ams : 0);
}

/* set algorythm connection */
static void set_algorythm(OPL_CH *CH) {
	int *carrier = &outd[0];
	CH->connect1 = CH->CON ? carrier : &feedback2;
	CH->connect2 = carrier;
}

/* ---------- frequency counter for operater update ---------- */
inline void CALC_FCSLOT(OPL_CH *CH, OPL_SLOT *SLOT) {
	int ksr;

	/* frequency step counter */
	SLOT->Incr = CH->fc * SLOT->mul;
	ksr = CH->kcode >> SLOT->KSR;

	if (SLOT->ksr != ksr) {
		SLOT->ksr = ksr;
		/* attack , decay rate recalcration */
		SLOT->evsa = SLOT->AR[ksr];
		SLOT->evsd = SLOT->DR[ksr];
		SLOT->evsr = SLOT->RR[ksr];
	}
	SLOT->TLL = SLOT->TL + (CH->ksl_base>>SLOT->ksl);
}

/* set multi,am,vib,EG-TYP,KSR,mul */
inline void set_mul(FM_OPL *OPL, int slot, int v) {
	OPL_CH   *CH   = &OPL->P_CH[slot>>1];
	OPL_SLOT *SLOT = &CH->SLOT[slot & 1];

	SLOT->mul    = MUL_TABLE[v & 0x0f];
	SLOT->KSR    = (v & 0x10) ? 0 : 2;
	SLOT->eg_typ = (v & 0x20) >> 5;
	SLOT->vib    = (v & 0x40);
	SLOT->ams    = (v & 0x80);
	CALC_FCSLOT(CH, SLOT);
}

/* set ksl & tl */
inline void set_ksl_tl(FM_OPL *OPL, int slot, int v) {
	OPL_CH   *CH   = &OPL->P_CH[slot>>1];
	OPL_SLOT *SLOT = &CH->SLOT[slot & 1];
	int ksl = v >> 6; /* 0 / 1.5 / 3 / 6 db/OCT */

	SLOT->ksl = ksl ? 3-ksl : 31;
	SLOT->TL  = (int)((v & 0x3f) * (0.75 / EG_STEP)); /* 0.75db step */

	if (!(OPL->mode & 0x80)) {	/* not CSM latch total level */
		SLOT->TLL = SLOT->TL + (CH->ksl_base >> SLOT->ksl);
	}
}

/* set attack rate & decay rate  */
inline void set_ar_dr(FM_OPL *OPL, int slot, int v) {
	OPL_CH   *CH   = &OPL->P_CH[slot>>1];
	OPL_SLOT *SLOT = &CH->SLOT[slot & 1];
	int ar = v >> 4;
	int dr = v & 0x0f;

	SLOT->AR = ar ? &OPL->AR_TABLE[ar << 2] : RATE_0;
	SLOT->evsa = SLOT->AR[SLOT->ksr];
	if (SLOT->evm == ENV_MOD_AR)
		SLOT->evs = SLOT->evsa;

	SLOT->DR = dr ? &OPL->DR_TABLE[dr<<2] : RATE_0;
	SLOT->evsd = SLOT->DR[SLOT->ksr];
	if (SLOT->evm == ENV_MOD_DR)
		SLOT->evs = SLOT->evsd;
}

/* set sustain level & release rate */
inline void set_sl_rr(FM_OPL *OPL, int slot, int v) {
	OPL_CH   *CH   = &OPL->P_CH[slot>>1];
	OPL_SLOT *SLOT = &CH->SLOT[slot & 1];
	int sl = v >> 4;
	int rr = v & 0x0f;

	SLOT->SL = SL_TABLE[sl];
	if (SLOT->evm == ENV_MOD_DR)
		SLOT->eve = SLOT->SL;
	SLOT->RR = &OPL->DR_TABLE[rr<<2];
	SLOT->evsr = SLOT->RR[SLOT->ksr];
	if (SLOT->evm == ENV_MOD_RR)
		SLOT->evs = SLOT->evsr;
}

/* operator output calcrator */

#define OP_OUT(slot,env,con)   slot->wavetable[((slot->Cnt + con)>>(24-SIN_ENT_SHIFT)) & (SIN_ENT-1)][env]
/* ---------- calcrate one of channel ---------- */
inline void OPL_CALC_CH(OPL_CH *CH) {
	uint env_out;
	OPL_SLOT *SLOT;

	feedback2 = 0;
	/* SLOT 1 */
	SLOT = &CH->SLOT[SLOT1];
	env_out=OPL_CALC_SLOT(SLOT);
	if (env_out < (uint)(EG_ENT - 1)) {
		/* PG */
		if (SLOT->vib)
			SLOT->Cnt += (SLOT->Incr * vib) >> VIB_RATE_SHIFT;
		else
			SLOT->Cnt += SLOT->Incr;
		/* connection */
		if (CH->FB) {
			int feedback1 = (CH->op1_out[0] + CH->op1_out[1]) >> CH->FB;
			CH->op1_out[1] = CH->op1_out[0];
			*CH->connect1 += CH->op1_out[0] = OP_OUT(SLOT, env_out, feedback1);
		} else {
			*CH->connect1 += OP_OUT(SLOT, env_out, 0);
		}
	} else {
		CH->op1_out[1] = CH->op1_out[0];
		CH->op1_out[0] = 0;
	}
	/* SLOT 2 */
	SLOT = &CH->SLOT[SLOT2];
	env_out=OPL_CALC_SLOT(SLOT);
	if (env_out < (uint)(EG_ENT - 1)) {
		/* PG */
		if (SLOT->vib)
			SLOT->Cnt += (SLOT->Incr * vib) >> VIB_RATE_SHIFT;
		else
			SLOT->Cnt += SLOT->Incr;
		/* connection */
		outd[0] += OP_OUT(SLOT, env_out, feedback2);
	}
}

/* ---------- calcrate rythm block ---------- */
#define WHITE_NOISE_db 6.0
inline void OPL_CALC_RH(FM_OPL *OPL, OPL_CH *CH) {
	uint env_tam, env_sd, env_top, env_hh;
	// This code used to do int(OPL->rnd.getRandomBit() * (WHITE_NOISE_db / EG_STEP)),
	// but EG_STEP = 96.0/EG_ENT, and WHITE_NOISE_db=6.0. So, that's equivalent to
	// int(OPL->rnd.getRandomBit() * EG_ENT/16). We know that EG_ENT is 4096, or 1024,
	// or 128, so we can safely avoid any FP ops.
	int whitenoise = OPL->rnd->getRandomBit() * (EG_ENT>>4);

	int tone8;

	OPL_SLOT *SLOT;
	int env_out;

	/* BD : same as FM serial mode and output level is large */
	feedback2 = 0;
	/* SLOT 1 */
	SLOT = &CH[6].SLOT[SLOT1];
	env_out = OPL_CALC_SLOT(SLOT);
	if (env_out < EG_ENT-1) {
		/* PG */
		if (SLOT->vib)
			SLOT->Cnt += (SLOT->Incr * vib) >> VIB_RATE_SHIFT;
		else
			SLOT->Cnt += SLOT->Incr;
		/* connection */
		if (CH[6].FB) {
			int feedback1 = (CH[6].op1_out[0] + CH[6].op1_out[1]) >> CH[6].FB;
			CH[6].op1_out[1] = CH[6].op1_out[0];
			feedback2 = CH[6].op1_out[0] = OP_OUT(SLOT, env_out, feedback1);
		}
		else {
			feedback2 = OP_OUT(SLOT, env_out, 0);
		}
	} else {
		feedback2 = 0;
		CH[6].op1_out[1] = CH[6].op1_out[0];
		CH[6].op1_out[0] = 0;
	}
	/* SLOT 2 */
	SLOT = &CH[6].SLOT[SLOT2];
	env_out = OPL_CALC_SLOT(SLOT);
	if (env_out < EG_ENT-1) {
		/* PG */
		if (SLOT->vib)
			SLOT->Cnt += (SLOT->Incr * vib) >> VIB_RATE_SHIFT;
		else
			SLOT->Cnt += SLOT->Incr;
		/* connection */
		outd[0] += OP_OUT(SLOT, env_out, feedback2) * 2;
	}

	// SD  (17) = mul14[fnum7] + white noise
	// TAM (15) = mul15[fnum8]
	// TOP (18) = fnum6(mul18[fnum8]+whitenoise)
	// HH  (14) = fnum7(mul18[fnum8]+whitenoise) + white noise
	env_sd = OPL_CALC_SLOT(SLOT7_2) + whitenoise;
	env_tam =OPL_CALC_SLOT(SLOT8_1);
	env_top = OPL_CALC_SLOT(SLOT8_2);
	env_hh = OPL_CALC_SLOT(SLOT7_1) + whitenoise;

	/* PG */
	if (SLOT7_1->vib)
		SLOT7_1->Cnt += (SLOT7_1->Incr * vib) >> (VIB_RATE_SHIFT-1);
	else
		SLOT7_1->Cnt += 2 * SLOT7_1->Incr;
	if (SLOT7_2->vib)
		SLOT7_2->Cnt += (CH[7].fc * vib) >> (VIB_RATE_SHIFT-3);
	else
		SLOT7_2->Cnt += (CH[7].fc * 8);
	if (SLOT8_1->vib)
		SLOT8_1->Cnt += (SLOT8_1->Incr * vib) >> VIB_RATE_SHIFT;
	else
		SLOT8_1->Cnt += SLOT8_1->Incr;
	if (SLOT8_2->vib)
		SLOT8_2->Cnt += ((CH[8].fc * 3) * vib) >> (VIB_RATE_SHIFT-4);
	else
		SLOT8_2->Cnt += (CH[8].fc * 48);

	tone8 = OP_OUT(SLOT8_2,whitenoise,0 );

	/* SD */
	if (env_sd < (uint)(EG_ENT - 1))
		outd[0] += OP_OUT(SLOT7_1, env_sd, 0) * 8;
	/* TAM */
	if (env_tam < (uint)(EG_ENT - 1))
		outd[0] += OP_OUT(SLOT8_1, env_tam, 0) * 2;
	/* TOP-CY */
	if (env_top < (uint)(EG_ENT - 1))
		outd[0] += OP_OUT(SLOT7_2, env_top, tone8) * 2;
	/* HH */
	if (env_hh  < (uint)(EG_ENT-1))
		outd[0] += OP_OUT(SLOT7_2, env_hh, tone8) * 2;
}

/* ----------- initialize time tabls ----------- */
static void init_timetables(FM_OPL *OPL, int ARRATE, int DRRATE) {
	int i;
	double rate;

	/* make attack rate & decay rate tables */
	for (i = 0; i < 4; i++)
		OPL->AR_TABLE[i] = OPL->DR_TABLE[i] = 0;
	for (i = 4; i <= 60; i++) {
		rate = OPL->freqbase;						/* frequency rate */
		if (i < 60)
			rate *= 1.0 + (i & 3) * 0.25;		/* b0-1 : x1 , x1.25 , x1.5 , x1.75 */
		rate *= 1 << ((i >> 2) - 1);						/* b2-5 : shift bit */
		rate *= (double)(EG_ENT << ENV_BITS);
		OPL->AR_TABLE[i] = (int)(rate / ARRATE);
		OPL->DR_TABLE[i] = (int)(rate / DRRATE);
	}
	for (i = 60; i < 76; i++) {
		OPL->AR_TABLE[i] = EG_AED-1;
		OPL->DR_TABLE[i] = OPL->DR_TABLE[60];
	}
}

/* ---------- generic table initialize ---------- */
static int OPLOpenTable(void) {
	int s,t;
	double rate;
	int i,j;
	double pom;

#ifdef __DS__
	DS::fastRamReset();

	TL_TABLE = (int *) DS::fastRamAlloc(TL_MAX * 2 * sizeof(int *));
	SIN_TABLE = (int **) DS::fastRamAlloc(SIN_ENT * 4 * sizeof(int *));
#else

	/* allocate dynamic tables */
	if ((TL_TABLE = (int *)malloc(TL_MAX * 2 * sizeof(int))) == NULL)
		return 0;

	if ((SIN_TABLE = (int **)malloc(SIN_ENT * 4 * sizeof(int *))) == NULL) {
		free(TL_TABLE);
		return 0;
	}
#endif

	if ((AMS_TABLE = (int *)malloc(AMS_ENT * 2 * sizeof(int))) == NULL) {
		free(TL_TABLE);
		free(SIN_TABLE);
		return 0;
	}

	if ((VIB_TABLE = (int *)malloc(VIB_ENT * 2 * sizeof(int))) == NULL) {
		free(TL_TABLE);
		free(SIN_TABLE);
		free(AMS_TABLE);
		return 0;
	}
	/* make total level table */
	for (t = 0; t < EG_ENT - 1; t++) {
		rate = ((1 << TL_BITS) - 1) / pow(10.0, EG_STEP * t / 20);	/* dB -> voltage */
		TL_TABLE[         t] =  (int)rate;
		TL_TABLE[TL_MAX + t] = -TL_TABLE[t];
	}
	/* fill volume off area */
	for (t = EG_ENT - 1; t < TL_MAX; t++) {
		TL_TABLE[t] = TL_TABLE[TL_MAX + t] = 0;
	}

	/* make sinwave table (total level offet) */
	/* degree 0 = degree 180                   = off */
	SIN_TABLE[0] = SIN_TABLE[SIN_ENT /2 ] = &TL_TABLE[EG_ENT - 1];
	for (s = 1;s <= SIN_ENT / 4; s++) {
		pom = sin(2 * M_PI * s / SIN_ENT); /* sin     */
		pom = 20 * log10(1 / pom);	   /* decibel */
		j = int(pom / EG_STEP);         /* TL_TABLE steps */

		/* degree 0   -  90    , degree 180 -  90 : plus section */
		SIN_TABLE[          s] = SIN_TABLE[SIN_ENT / 2 - s] = &TL_TABLE[j];
		/* degree 180 - 270    , degree 360 - 270 : minus section */
		SIN_TABLE[SIN_ENT / 2 + s] = SIN_TABLE[SIN_ENT - s] = &TL_TABLE[TL_MAX + j];
	}
	for (s = 0;s < SIN_ENT; s++) {
		SIN_TABLE[SIN_ENT * 1 + s] = s < (SIN_ENT / 2) ? SIN_TABLE[s] : &TL_TABLE[EG_ENT];
		SIN_TABLE[SIN_ENT * 2 + s] = SIN_TABLE[s % (SIN_ENT / 2)];
		SIN_TABLE[SIN_ENT * 3 + s] = (s / (SIN_ENT / 4)) & 1 ? &TL_TABLE[EG_ENT] : SIN_TABLE[SIN_ENT * 2 + s];
	}


	ENV_CURVE = (int *)malloc(sizeof(int) * (2*EG_ENT+1));
	if (!ENV_CURVE)
		error("[OPLOpenTable] Cannot allocate memory");

	/* envelope counter -> envelope output table */
	for (i=0; i < EG_ENT; i++) {
		/* ATTACK curve */
		pom = pow(((double)(EG_ENT - 1 - i) / EG_ENT), 8) * EG_ENT;
		/* if (pom >= EG_ENT) pom = EG_ENT-1; */
		ENV_CURVE[i] = (int)pom;
		/* DECAY ,RELEASE curve */
		ENV_CURVE[(EG_DST >> ENV_BITS) + i]= i;
	}
	/* off */
	ENV_CURVE[EG_OFF >> ENV_BITS]= EG_ENT - 1;
	/* make LFO ams table */
	for (i=0; i < AMS_ENT; i++) {
		pom = (1.0 + sin(2 * M_PI * i / AMS_ENT)) / 2; /* sin */
		AMS_TABLE[i]         = (int)((1.0 / EG_STEP) * pom); /* 1dB   */
		AMS_TABLE[AMS_ENT + i] = (int)((4.8 / EG_STEP) * pom); /* 4.8dB */
	}
	/* make LFO vibrate table */
	for (i=0; i < VIB_ENT; i++) {
		/* 100cent = 1seminote = 6% ?? */
		pom = (double)VIB_RATE * 0.06 * sin(2 * M_PI * i / VIB_ENT); /* +-100sect step */
		VIB_TABLE[i]         = (int)(VIB_RATE + (pom * 0.07)); /* +- 7cent */
		VIB_TABLE[VIB_ENT + i] = (int)(VIB_RATE + (pom * 0.14)); /* +-14cent */
	}
	return 1;
}

static void OPLCloseTable(void) {
#ifndef __DS__
	free(TL_TABLE);
	free(SIN_TABLE);
#endif
	free(AMS_TABLE);
	free(VIB_TABLE);
	free(ENV_CURVE);
}

/* CSM Key Controll */
inline void CSMKeyControll(OPL_CH *CH) {
	OPL_SLOT *slot1 = &CH->SLOT[SLOT1];
	OPL_SLOT *slot2 = &CH->SLOT[SLOT2];
	/* all key off */
	OPL_KEYOFF(slot1);
	OPL_KEYOFF(slot2);
	/* total level latch */
	slot1->TLL = slot1->TL + (CH->ksl_base>>slot1->ksl);
	slot1->TLL = slot1->TL + (CH->ksl_base>>slot1->ksl);
	/* key on */
	CH->op1_out[0] = CH->op1_out[1] = 0;
	OPL_KEYON(slot1);
	OPL_KEYON(slot2);
}

/* ---------- opl initialize ---------- */
static void OPL_initalize(FM_OPL *OPL) {
	int fn;

	/* frequency base */
	OPL->freqbase = (OPL->rate) ? ((double)OPL->clock / OPL->rate) / 72 : 0;
	/* Timer base time */
	OPL->TimerBase = 1.0/((double)OPL->clock / 72.0 );
	/* make time tables */
	init_timetables(OPL, OPL_ARRATE, OPL_DRRATE);
	/* make fnumber -> increment counter table */
	for (fn=0; fn < 1024; fn++) {
		OPL->FN_TABLE[fn] = (uint)(OPL->freqbase * fn * FREQ_RATE * (1<<7) / 2);
	}
	/* LFO freq.table */
	OPL->amsIncr = (int)(OPL->rate ? (double)AMS_ENT * (1 << AMS_SHIFT) / OPL->rate * 3.7 * ((double)OPL->clock/3600000) : 0);
	OPL->vibIncr = (int)(OPL->rate ? (double)VIB_ENT * (1 << VIB_SHIFT) / OPL->rate * 6.4 * ((double)OPL->clock/3600000) : 0);
}

/* ---------- write a OPL registers ---------- */
void OPLWriteReg(FM_OPL *OPL, int r, int v) {
	OPL_CH *CH;
	int slot;
	uint block_fnum;

	switch (r & 0xe0) {
	case 0x00: /* 00-1f:controll */
		switch (r & 0x1f) {
		case 0x01:
			/* wave selector enable */
			if (OPL->type&OPL_TYPE_WAVESEL) {
				OPL->wavesel = v & 0x20;
				if (!OPL->wavesel) {
					/* preset compatible mode */
					int c;
					for (c = 0; c < OPL->max_ch; c++) {
						OPL->P_CH[c].SLOT[SLOT1].wavetable = &SIN_TABLE[0];
						OPL->P_CH[c].SLOT[SLOT2].wavetable = &SIN_TABLE[0];
					}
				}
			}
			return;
		case 0x02:	/* Timer 1 */
			OPL->T[0] = (256-v) * 4;
			break;
		case 0x03:	/* Timer 2 */
			OPL->T[1] = (256-v) * 16;
			return;
		case 0x04:	/* IRQ clear / mask and Timer enable */
			if (v & 0x80) {	/* IRQ flag clear */
				OPL_STATUS_RESET(OPL, 0x7f);
			} else {	/* set IRQ mask ,timer enable*/
				uint8 st1 = v & 1;
				uint8 st2 = (v >> 1) & 1;
				/* IRQRST,T1MSK,t2MSK,EOSMSK,BRMSK,x,ST2,ST1 */
				OPL_STATUS_RESET(OPL, v & 0x78);
				OPL_STATUSMASK_SET(OPL,((~v) & 0x78) | 0x01);
				/* timer 2 */
				if (OPL->st[1] != st2) {
					double interval = st2 ? (double)OPL->T[1] * OPL->TimerBase : 0.0;
					OPL->st[1] = st2;
					if (OPL->TimerHandler) (OPL->TimerHandler)(OPL->TimerParam + 1, interval);
				}
				/* timer 1 */
				if (OPL->st[0] != st1) {
					double interval = st1 ? (double)OPL->T[0] * OPL->TimerBase : 0.0;
					OPL->st[0] = st1;
					if (OPL->TimerHandler) (OPL->TimerHandler)(OPL->TimerParam + 0, interval);
				}
			}
			return;
		}
		break;
	case 0x20:	/* am,vib,ksr,eg type,mul */
		slot = slot_array[r&0x1f];
		if (slot == -1)
			return;
		set_mul(OPL,slot,v);
		return;
	case 0x40:
		slot = slot_array[r&0x1f];
		if (slot == -1)
			return;
		set_ksl_tl(OPL,slot,v);
		return;
	case 0x60:
		slot = slot_array[r&0x1f];
		if (slot == -1)
			return;
		set_ar_dr(OPL,slot,v);
		return;
	case 0x80:
		slot = slot_array[r&0x1f];
		if (slot == -1)
			return;
		set_sl_rr(OPL,slot,v);
		return;
	case 0xa0:
		switch (r) {
		case 0xbd:
			/* amsep,vibdep,r,bd,sd,tom,tc,hh */
			{
			uint8 rkey = OPL->rythm ^ v;
			OPL->ams_table = &AMS_TABLE[v & 0x80 ? AMS_ENT : 0];
			OPL->vib_table = &VIB_TABLE[v & 0x40 ? VIB_ENT : 0];
			OPL->rythm  = v & 0x3f;
			if (OPL->rythm & 0x20) {
				/* BD key on/off */
				if (rkey & 0x10) {
					if (v & 0x10) {
						OPL->P_CH[6].op1_out[0] = OPL->P_CH[6].op1_out[1] = 0;
						OPL_KEYON(&OPL->P_CH[6].SLOT[SLOT1]);
						OPL_KEYON(&OPL->P_CH[6].SLOT[SLOT2]);
					} else {
						OPL_KEYOFF(&OPL->P_CH[6].SLOT[SLOT1]);
						OPL_KEYOFF(&OPL->P_CH[6].SLOT[SLOT2]);
					}
				}
				/* SD key on/off */
				if (rkey & 0x08) {
					if (v & 0x08)
						OPL_KEYON(&OPL->P_CH[7].SLOT[SLOT2]);
					else
						OPL_KEYOFF(&OPL->P_CH[7].SLOT[SLOT2]);
				}/* TAM key on/off */
				if (rkey & 0x04) {
					if (v & 0x04)
						OPL_KEYON(&OPL->P_CH[8].SLOT[SLOT1]);
					else
						OPL_KEYOFF(&OPL->P_CH[8].SLOT[SLOT1]);
				}
				/* TOP-CY key on/off */
				if (rkey & 0x02) {
					if (v & 0x02)
						OPL_KEYON(&OPL->P_CH[8].SLOT[SLOT2]);
					else
						OPL_KEYOFF(&OPL->P_CH[8].SLOT[SLOT2]);
				}
				/* HH key on/off */
				if (rkey & 0x01) {
					if (v & 0x01)
						OPL_KEYON(&OPL->P_CH[7].SLOT[SLOT1]);
					else
						OPL_KEYOFF(&OPL->P_CH[7].SLOT[SLOT1]);
				}
			}
			}
			return;

		default:
			break;
		}
		/* keyon,block,fnum */
		if ((r & 0x0f) > 8)
			return;
		CH = &OPL->P_CH[r & 0x0f];
		if (!(r&0x10)) {	/* a0-a8 */
			block_fnum  = (CH->block_fnum & 0x1f00) | v;
		} else {	/* b0-b8 */
			int keyon = (v >> 5) & 1;
			block_fnum = ((v & 0x1f) << 8) | (CH->block_fnum & 0xff);
			if (CH->keyon != keyon) {
				if ((CH->keyon=keyon)) {
					CH->op1_out[0] = CH->op1_out[1] = 0;
					OPL_KEYON(&CH->SLOT[SLOT1]);
					OPL_KEYON(&CH->SLOT[SLOT2]);
				} else {
					OPL_KEYOFF(&CH->SLOT[SLOT1]);
					OPL_KEYOFF(&CH->SLOT[SLOT2]);
				}
			}
		}
		/* update */
		if (CH->block_fnum != block_fnum) {
			int blockRv = 7 - (block_fnum >> 10);
			int fnum = block_fnum & 0x3ff;
			CH->block_fnum = block_fnum;
			CH->ksl_base = KSL_TABLE[block_fnum >> 6];
			CH->fc = OPL->FN_TABLE[fnum] >> blockRv;
			CH->kcode = CH->block_fnum >> 9;
			if ((OPL->mode & 0x40) && CH->block_fnum & 0x100)
				CH->kcode |=1;
			CALC_FCSLOT(CH,&CH->SLOT[SLOT1]);
			CALC_FCSLOT(CH,&CH->SLOT[SLOT2]);
		}
		return;
	case 0xc0:
		/* FB,C */
		if ((r & 0x0f) > 8)
			return;
		CH = &OPL->P_CH[r&0x0f];
		{
			int feedback = (v >> 1) & 7;
			CH->FB = feedback ? (8 + 1) - feedback : 0;
			CH->CON = v & 1;
			set_algorythm(CH);
		}
		return;
	case 0xe0: /* wave type */
		slot = slot_array[r & 0x1f];
		if (slot == -1)
			return;
		CH = &OPL->P_CH[slot>>1];
		if (OPL->wavesel) {
			CH->SLOT[slot&1].wavetable = &SIN_TABLE[(v & 0x03) * SIN_ENT];
		}
		return;
	}
}

/* lock/unlock for common table */
static int OPL_LockTable(void) {
	num_lock++;
	if (num_lock>1)
		return 0;
	/* first time */
	cur_chip = NULL;
	/* allocate total level table (128kb space) */
	if (!OPLOpenTable()) {
		num_lock--;
		return -1;
	}
	return 0;
}

static void OPL_UnLockTable(void) {
	if (num_lock)
		num_lock--;
	if (num_lock)
		return;
	/* last time */
	cur_chip = NULL;
	OPLCloseTable();
}

/*******************************************************************************/
/*		YM3812 local section                                                   */
/*******************************************************************************/

/* ---------- update one of chip ----------- */
void YM3812UpdateOne(FM_OPL *OPL, int16 *buffer, int length) {
	int i;
	int data;
	int16 *buf = buffer;
	uint amsCnt = OPL->amsCnt;
	uint vibCnt = OPL->vibCnt;
	uint8 rythm = OPL->rythm & 0x20;
	OPL_CH *CH, *R_CH;


	if ((void *)OPL != cur_chip) {
		cur_chip = (void *)OPL;
		/* channel pointers */
		S_CH = OPL->P_CH;
		E_CH = &S_CH[9];
		/* rythm slot */
		SLOT7_1 = &S_CH[7].SLOT[SLOT1];
		SLOT7_2 = &S_CH[7].SLOT[SLOT2];
		SLOT8_1 = &S_CH[8].SLOT[SLOT1];
		SLOT8_2 = &S_CH[8].SLOT[SLOT2];
		/* LFO state */
		amsIncr = OPL->amsIncr;
		vibIncr = OPL->vibIncr;
		ams_table = OPL->ams_table;
		vib_table = OPL->vib_table;
	}
	R_CH = rythm ? &S_CH[6] : E_CH;
	for (i = 0; i < length; i++) {
		/*            channel A         channel B         channel C      */
		/* LFO */
		ams = ams_table[(amsCnt += amsIncr) >> AMS_SHIFT];
		vib = vib_table[(vibCnt += vibIncr) >> VIB_SHIFT];
		outd[0] = 0;
		/* FM part */
		for (CH = S_CH; CH < R_CH; CH++)
			OPL_CALC_CH(CH);
		/* Rythn part */
		if (rythm)
			OPL_CALC_RH(OPL, S_CH);
		/* limit check */
		data = CLIP(outd[0], OPL_MINOUT, OPL_MAXOUT);
		/* store to sound buffer */
		buf[i] = data >> OPL_OUTSB;
	}

	OPL->amsCnt = amsCnt;
	OPL->vibCnt = vibCnt;
}

/* ---------- reset a chip ---------- */
void OPLResetChip(FM_OPL *OPL) {
	int c,s;
	int i;

	/* reset chip */
	OPL->mode = 0;	/* normal mode */
	OPL_STATUS_RESET(OPL, 0x7f);
	/* reset with register write */
	OPLWriteReg(OPL, 0x01,0); /* wabesel disable */
	OPLWriteReg(OPL, 0x02,0); /* Timer1 */
	OPLWriteReg(OPL, 0x03,0); /* Timer2 */
	OPLWriteReg(OPL, 0x04,0); /* IRQ mask clear */
	for (i = 0xff; i >= 0x20; i--)
		OPLWriteReg(OPL,i,0);
	/* reset OPerator parameter */
	for (c = 0; c < OPL->max_ch; c++) {
		OPL_CH *CH = &OPL->P_CH[c];
		/* OPL->P_CH[c].PAN = OPN_CENTER; */
		for (s = 0; s < 2; s++) {
			/* wave table */
			CH->SLOT[s].wavetable = &SIN_TABLE[0];
			/* CH->SLOT[s].evm = ENV_MOD_RR; */
			CH->SLOT[s].evc = EG_OFF;
			CH->SLOT[s].eve = EG_OFF + 1;
			CH->SLOT[s].evs = 0;
		}
	}
}

/* ----------  Create a virtual YM3812 ----------       */
/* 'rate'  is sampling rate and 'bufsiz' is the size of the  */
FM_OPL *OPLCreate(int type, int clock, int rate) {
	char *ptr;
	FM_OPL *OPL;
	int state_size;
	int max_ch = 9; /* normaly 9 channels */

	if (OPL_LockTable() == -1)
		return NULL;
	/* allocate OPL state space */
	state_size  = sizeof(FM_OPL);
	state_size += sizeof(OPL_CH) * max_ch;

	/* allocate memory block */
	ptr = (char *)calloc(state_size, 1);
	if (ptr == NULL)
		return NULL;

	/* clear */
	memset(ptr, 0, state_size);
	OPL       = (FM_OPL *)ptr; ptr += sizeof(FM_OPL);
	OPL->P_CH = (OPL_CH *)ptr; ptr += sizeof(OPL_CH) * max_ch;

	/* set channel state pointer */
	OPL->type  = type;
	OPL->clock = clock;
	OPL->rate  = rate;
	OPL->max_ch = max_ch;

	// Init the random source. Note: We use a fixed name for it here.
	// So if multiple FM_OPL objects exist in parallel, then their
	// random sources will have an equal name. At least in the
	// current EventRecorder implementation, this causes no problems;
	// but this is probably not guaranteed.
	// Alas, it does not seem worthwhile to bother much with this
	// at the time, so I am leaving it as it is.
	OPL->rnd = new Common::RandomSource("mame");

	/* init grobal tables */
	OPL_initalize(OPL);

	/* reset chip */
	OPLResetChip(OPL);
	return OPL;
}

/* ----------  Destroy one of virtual YM3812 ----------       */
void OPLDestroy(FM_OPL *OPL) {
	OPL_UnLockTable();
	delete OPL->rnd;
	free(OPL);
}

/* ----------  Option handlers ----------       */
void OPLSetTimerHandler(FM_OPL *OPL, OPL_TIMERHANDLER TimerHandler,int channelOffset) {
	OPL->TimerHandler   = TimerHandler;
	OPL->TimerParam = channelOffset;
}

void OPLSetIRQHandler(FM_OPL *OPL, OPL_IRQHANDLER IRQHandler, int param) {
	OPL->IRQHandler     = IRQHandler;
	OPL->IRQParam = param;
}

void OPLSetUpdateHandler(FM_OPL *OPL, OPL_UPDATEHANDLER UpdateHandler,int param) {
	OPL->UpdateHandler = UpdateHandler;
	OPL->UpdateParam = param;
}

/* ---------- YM3812 I/O interface ---------- */
int OPLWrite(FM_OPL *OPL,int a,int v) {
	if (!(a & 1)) {	/* address port */
		OPL->address = v & 0xff;
	} else {	/* data port */
		if (OPL->UpdateHandler)
			OPL->UpdateHandler(OPL->UpdateParam,0);
		OPLWriteReg(OPL, OPL->address,v);
	}
	return OPL->status >> 7;
}

unsigned char OPLRead(FM_OPL *OPL,int a) {
	if (!(a & 1)) {	/* status port */
		return OPL->status & (OPL->statusmask | 0x80);
	}
	/* data port */
	switch (OPL->address) {
	case 0x05: /* KeyBoard IN */
		warning("OPL:read unmapped KEYBOARD port");
		return 0;
	case 0x19: /* I/O DATA    */
		warning("OPL:read unmapped I/O port");
		return 0;
	case 0x1a: /* PCM-DATA    */
		return 0;
	default:
		break;
	}
	return 0;
}

int OPLTimerOver(FM_OPL *OPL, int c) {
	if (c) {	/* Timer B */
		OPL_STATUS_SET(OPL, 0x20);
	} else {	/* Timer A */
		OPL_STATUS_SET(OPL, 0x40);
		/* CSM mode key,TL controll */
		if (OPL->mode & 0x80) {	/* CSM mode total level latch and auto key on */
			int ch;
			if (OPL->UpdateHandler)
				OPL->UpdateHandler(OPL->UpdateParam,0);
			for (ch = 0; ch < 9; ch++)
				CSMKeyControll(&OPL->P_CH[ch]);
		}
	}
	/* reload timer */
	if (OPL->TimerHandler)
		(OPL->TimerHandler)(OPL->TimerParam + c, (double)OPL->T[c] * OPL->TimerBase);
	return OPL->status >> 7;
}

FM_OPL *makeAdLibOPL(int rate) {
	// We need to emulate one YM3812 chip
	int env_bits = FMOPL_ENV_BITS_HQ;
	int eg_ent = FMOPL_EG_ENT_HQ;
#if defined(_WIN32_WCE) || defined(__SYMBIAN32__) || defined(__GP32__) || defined(GP2X) || defined(__MAEMO__) || defined(__DS__) || defined(__MINT__) || defined(__N64__)
	if (ConfMan.hasKey("FM_high_quality") && ConfMan.getBool("FM_high_quality")) {
		env_bits = FMOPL_ENV_BITS_HQ;
		eg_ent = FMOPL_EG_ENT_HQ;
	} else if (ConfMan.hasKey("FM_medium_quality") && ConfMan.getBool("FM_medium_quality")) {
		env_bits = FMOPL_ENV_BITS_MQ;
		eg_ent = FMOPL_EG_ENT_MQ;
	} else {
		env_bits = FMOPL_ENV_BITS_LQ;
		eg_ent = FMOPL_EG_ENT_LQ;
	}
#endif

	OPLBuildTables(env_bits, eg_ent);
	return OPLCreate(OPL_TYPE_YM3812, 3579545, rate);
}

} // End of namespace MAME
} // End of namespace OPL
