// Game_Music_Emu 0.5.2. http://www.slack.net/~ant/

// Based on Gens 2.10 ym2612.c

#include "Ym2612_Emu.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>

/* Copyright (C) 2002 Stéphane Dallongeville (gens AT consolemul.com) */
/* Copyright (C) 2004-2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

// This is mostly the original source in its C style and all.
//
// Somewhat optimized and simplified. Uses a template to generate the many
// variants of Update_Chan. Rewrote header file. In need of full rewrite by
// someone more familiar with FM sound and the YM2612. Has some inaccuracies
// compared to the Sega Genesis sound, particularly being mixed at such a
// high sample accuracy (the Genesis sounds like it has only 8 bit samples).
// - Shay

#ifdef BLARGG_ENABLE_OPTIMIZER
	#include BLARGG_ENABLE_OPTIMIZER
#endif

const int output_bits = 14;

struct slot_t
{
	const int *DT;  // parametre detune
	int MUL;    // parametre "multiple de frequence"
	int TL;     // Total Level = volume lorsque l'enveloppe est au plus haut
	int TLL;    // Total Level ajusted
	int SLL;    // Sustin Level (ajusted) = volume où l'enveloppe termine sa premiere phase de regression
	int KSR_S;  // Key Scale Rate Shift = facteur de prise en compte du KSL dans la variations de l'enveloppe
	int KSR;    // Key Scale Rate = cette valeur est calculee par rapport à la frequence actuelle, elle va influer
				// sur les differents parametres de l'enveloppe comme l'attaque, le decay ...  comme dans la realite !
	int SEG;    // Type enveloppe SSG
	int env_xor;
	int env_max;

	const int *AR;  // Attack Rate (table pointeur) = Taux d'attaque (AR[KSR])
	const int *DR;  // Decay Rate (table pointeur) = Taux pour la regression (DR[KSR])
	const int *SR;  // Sustin Rate (table pointeur) = Taux pour le maintien (SR[KSR])
	const int *RR;  // Release Rate (table pointeur) = Taux pour le rel'chement (RR[KSR])
	int Fcnt;   // Frequency Count = compteur-frequence pour determiner l'amplitude actuelle (SIN[Finc >> 16])
	int Finc;   // frequency step = pas d'incrementation du compteur-frequence
				// plus le pas est grand, plus la frequence est aïgu (ou haute)
	int Ecurp;  // Envelope current phase = cette variable permet de savoir dans quelle phase
				// de l'enveloppe on se trouve, par exemple phase d'attaque ou phase de maintenue ...
				// en fonction de la valeur de cette variable, on va appeler une fonction permettant
				// de mettre à jour l'enveloppe courante.
	int Ecnt;   // Envelope counter = le compteur-enveloppe permet de savoir où l'on se trouve dans l'enveloppe
	int Einc;   // Envelope step courant
	int Ecmp;   // Envelope counter limite pour la prochaine phase
	int EincA;  // Envelope step for Attack = pas d'incrementation du compteur durant la phase d'attaque
				// cette valeur est egal à AR[KSR]
	int EincD;  // Envelope step for Decay = pas d'incrementation du compteur durant la phase de regression
				// cette valeur est egal à DR[KSR]
	int EincS;  // Envelope step for Sustain = pas d'incrementation du compteur durant la phase de maintenue
				// cette valeur est egal à SR[KSR]
	int EincR;  // Envelope step for Release = pas d'incrementation du compteur durant la phase de rel'chement
				// cette valeur est egal à RR[KSR]
	int INd;    // input data of the slot = donnees en entree du slot
	int ChgEnM; // Change envelop mask.
	int AMS;    // AMS depth level of this SLOT = degre de modulation de l'amplitude par le LFO
	int AMSon;  // AMS enable flag = drapeau d'activation de l'AMS
};

struct channel_t
{
	int S0_OUT[4];          // anciennes sorties slot 0 (pour le feed back)
	int LEFT;               // LEFT enable flag
	int RIGHT;              // RIGHT enable flag
	int ALGO;               // Algorythm = determine les connections entre les operateurs
	int FB;                 // shift count of self feed back = degre de "Feed-Back" du SLOT 1 (il est son unique entree)
	int FMS;                // Frequency Modulation Sensitivity of channel = degre de modulation de la frequence sur la voie par le LFO
	int AMS;                // Amplitude Modulation Sensitivity of channel = degre de modulation de l'amplitude sur la voie par le LFO
	int FNUM[4];            // hauteur frequence de la voie (+ 3 pour le mode special)
	int FOCT[4];            // octave de la voie (+ 3 pour le mode special)
	int KC[4];              // Key Code = valeur fonction de la frequence (voir KSR pour les slots, KSR = KC >> KSR_S)
	slot_t SLOT[4]; // four slot.operators = les 4 slots de la voie
	int FFlag;              // Frequency step recalculation flag
};

struct state_t
{
	int Status;         // YM2612 Status (timer overflow)
	int TimerA;         // timerA limit = valeur jusqu'à laquelle le timer A doit compter
	int TimerAL;
	int TimerAcnt;      // timerA counter = valeur courante du Timer A
	int TimerB;         // timerB limit = valeur jusqu'à laquelle le timer B doit compter
	int TimerBL;
	int TimerBcnt;      // timerB counter = valeur courante du Timer B
	int Mode;           // Mode actuel des voie 3 et 6 (normal / special)
	int DAC;            // DAC enabled flag
	int DACdata;
	channel_t CHANNEL[Ym2612_Emu::channel_count];   // Les 6 voies du YM2612
	int REG[2][0x100];  // Sauvegardes des valeurs de tout les registres, c'est facultatif
						// cela nous rend le debuggage plus facile
};

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define ATTACK    0
#define DECAY     1
#define SUBSTAIN  2
#define RELEASE   3

// SIN_LBITS <= 16
// LFO_HBITS <= 16
// (SIN_LBITS + SIN_HBITS) <= 26
// (ENV_LBITS + ENV_HBITS) <= 28
// (LFO_LBITS + LFO_HBITS) <= 28

#define SIN_HBITS      12                               // Sinus phase counter int part
#define SIN_LBITS      (26 - SIN_HBITS)                 // Sinus phase counter float part (best setting)

#if (SIN_LBITS > 16)
#define SIN_LBITS      16                               // Can't be greater than 16 bits
#endif

#define ENV_HBITS      12                               // Env phase counter int part
#define ENV_LBITS      (28 - ENV_HBITS)                 // Env phase counter float part (best setting)

#define LFO_HBITS      10                               // LFO phase counter int part
#define LFO_LBITS      (28 - LFO_HBITS)                 // LFO phase counter float part (best setting)

#define SIN_LENGHT     (1 << SIN_HBITS)
#define ENV_LENGHT     (1 << ENV_HBITS)
#define LFO_LENGHT     (1 << LFO_HBITS)

#define TL_LENGHT      (ENV_LENGHT * 3)                 // Env + TL scaling + LFO

#define SIN_MASK       (SIN_LENGHT - 1)
#define ENV_MASK       (ENV_LENGHT - 1)
#define LFO_MASK       (LFO_LENGHT - 1)

#define ENV_STEP       (96.0 / ENV_LENGHT)              // ENV_MAX = 96 dB

#define ENV_ATTACK     ((ENV_LENGHT * 0) << ENV_LBITS)
#define ENV_DECAY      ((ENV_LENGHT * 1) << ENV_LBITS)
#define ENV_END        ((ENV_LENGHT * 2) << ENV_LBITS)

#define MAX_OUT_BITS   (SIN_HBITS + SIN_LBITS + 2)      // Modulation = -4 <--> +4
#define MAX_OUT        ((1 << MAX_OUT_BITS) - 1)

#define PG_CUT_OFF     ((int) (78.0 / ENV_STEP))
#define ENV_CUT_OFF    ((int) (68.0 / ENV_STEP))

#define AR_RATE        399128
#define DR_RATE        5514396

//#define AR_RATE        426136
//#define DR_RATE        (AR_RATE * 12)

#define LFO_FMS_LBITS  9    // FIXED (LFO_FMS_BASE gives somethink as 1)
#define LFO_FMS_BASE   ((int) (0.05946309436 * 0.0338 * (double) (1 << LFO_FMS_LBITS)))

#define S0             0    // Stupid typo of the YM2612
#define S1             2
#define S2             1
#define S3             3

inline void set_seg( slot_t& s, int seg )
{
	s.env_xor = 0;
	s.env_max = INT_MAX;
	s.SEG = seg;
	if ( seg & 4 )
	{
		s.env_xor = ENV_MASK;
		s.env_max = ENV_MASK;
	}
}

struct tables_t
{
	short SIN_TAB [SIN_LENGHT];                 // SINUS TABLE (offset into TL TABLE)
	int LFOcnt;         // LFO counter = compteur-frequence pour le LFO
	int LFOinc;         // LFO step counter = pas d'incrementation du compteur-frequence du LFO
						// plus le pas est grand, plus la frequence est grande
	unsigned int AR_TAB [128];                  // Attack rate table
	unsigned int DR_TAB [96];                   // Decay rate table
	unsigned int DT_TAB [8] [32];               // Detune table
	unsigned int SL_TAB [16];                   // Substain level table
	unsigned int NULL_RATE [32];                // Table for NULL rate
	int LFO_INC_TAB [8];                        // LFO step table
	
	short ENV_TAB [2 * ENV_LENGHT + 8];         // ENV CURVE TABLE (attack & decay)
	
	short LFO_ENV_TAB [LFO_LENGHT];             // LFO AMS TABLE (adjusted for 11.8 dB)
	short LFO_FREQ_TAB [LFO_LENGHT];            // LFO FMS TABLE
	int TL_TAB [TL_LENGHT * 2];                 // TOTAL LEVEL TABLE (positif and minus)
	unsigned int DECAY_TO_ATTACK [ENV_LENGHT];  // Conversion from decay to attack phase
	unsigned int FINC_TAB [2048];               // Frequency step table
};

static const unsigned char DT_DEF_TAB [4 * 32] =
{
// FD = 0
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

// FD = 1
  0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
  2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 8, 8, 8,

// FD = 2
  1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5,
  5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 16, 16, 16, 16,

// FD = 3
  2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7,
  8 , 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 20, 22, 22, 22, 22
};

static const unsigned char FKEY_TAB [16] =
{ 
	0, 0, 0, 0,
	0, 0, 0, 1,
	2, 3, 3, 3,
	3, 3, 3, 3
};

static const unsigned char LFO_AMS_TAB [4] =
{
	31, 4, 1, 0
};

static const unsigned char LFO_FMS_TAB [8] =
{
	LFO_FMS_BASE * 0, LFO_FMS_BASE * 1,
	LFO_FMS_BASE * 2, LFO_FMS_BASE * 3,
	LFO_FMS_BASE * 4, LFO_FMS_BASE * 6,
	LFO_FMS_BASE * 12, LFO_FMS_BASE * 24
};

inline void YM2612_Special_Update() { }

struct Ym2612_Impl
{
	enum { channel_count = Ym2612_Emu::channel_count };
	
	state_t YM2612;
	int mute_mask;
	tables_t g;
	
	void KEY_ON( channel_t&, int );
	void KEY_OFF( channel_t&, int );
	int SLOT_SET( int, int );
	int CHANNEL_SET( int, int );
	int YM_SET( int, int );
	
	void set_rate(void);
	void reset();
	void write0( int addr, int data );
	void write1( int addr, int data );
	int read(void);
	void run_timer( int );
	void run( int pair_count, Ym2612_Emu::sample_t* );
};

void Ym2612_Impl::KEY_ON( channel_t& ch, int nsl)
{
	slot_t *SL = &(ch.SLOT [nsl]);  // on recupere le bon pointeur de slot
	
	if (SL->Ecurp == RELEASE)       // la touche est-elle rel'chee ?
	{
		SL->Fcnt = 0;

		// Fix Ecco 2 splash sound
		
		SL->Ecnt = (g.DECAY_TO_ATTACK [g.ENV_TAB [SL->Ecnt >> ENV_LBITS]] + ENV_ATTACK) & SL->ChgEnM;
		SL->ChgEnM = ~0;

//      SL->Ecnt = g.DECAY_TO_ATTACK [g.ENV_TAB [SL->Ecnt >> ENV_LBITS]] + ENV_ATTACK;
//      SL->Ecnt = 0;

		SL->Einc = SL->EincA;
		SL->Ecmp = ENV_DECAY;
		SL->Ecurp = ATTACK;
	}
}


void Ym2612_Impl::KEY_OFF(channel_t& ch, int nsl)
{
	slot_t *SL = &(ch.SLOT [nsl]);  // on recupere le bon pointeur de slot
	
	if (SL->Ecurp != RELEASE)       // la touche est-elle appuyee ?
	{
		if (SL->Ecnt < ENV_DECAY)   // attack phase ?
		{
			SL->Ecnt = (g.ENV_TAB [SL->Ecnt >> ENV_LBITS] << ENV_LBITS) + ENV_DECAY;
		}

		SL->Einc = SL->EincR;
		SL->Ecmp = ENV_END;
		SL->Ecurp = RELEASE;
	}
}


int Ym2612_Impl::SLOT_SET( int Adr, int data )
{
	int nch = Adr & 3;
	if ( nch == 3 )
		return 1;
	
	channel_t& ch = YM2612.CHANNEL [nch + (Adr & 0x100 ? 3 : 0)];
	slot_t& sl = ch.SLOT [(Adr >> 2) & 3];

	switch ( Adr & 0xF0 )
	{
		case 0x30:
			if ( (sl.MUL = (data & 0x0F)) != 0 ) sl.MUL <<= 1;
			else sl.MUL = 1;

			sl.DT = (int*) g.DT_TAB [(data >> 4) & 7];

			ch.SLOT [0].Finc = -1;

			break;

		case 0x40:
			sl.TL = data & 0x7F;

			// SOR2 do a lot of TL adjustement and this fix R.Shinobi jump sound...
			YM2612_Special_Update();

#if ((ENV_HBITS - 7) < 0)
			sl.TLL = sl.TL >> (7 - ENV_HBITS);
#else
			sl.TLL = sl.TL << (ENV_HBITS - 7);
#endif

			break;

		case 0x50:
			sl.KSR_S = 3 - (data >> 6);

			ch.SLOT [0].Finc = -1;

			if (data &= 0x1F) sl.AR = (int*) &g.AR_TAB [data << 1];
			else sl.AR = (int*) &g.NULL_RATE [0];

			sl.EincA = sl.AR [sl.KSR];
			if (sl.Ecurp == ATTACK) sl.Einc = sl.EincA;
			break;

		case 0x60:
			if ( (sl.AMSon = (data & 0x80)) != 0 ) sl.AMS = ch.AMS;
			else sl.AMS = 31;

			if (data &= 0x1F) sl.DR = (int*) &g.DR_TAB [data << 1];
			else sl.DR = (int*) &g.NULL_RATE [0];

			sl.EincD = sl.DR [sl.KSR];
			if (sl.Ecurp == DECAY) sl.Einc = sl.EincD;
			break;

		case 0x70:
			if (data &= 0x1F) sl.SR = (int*) &g.DR_TAB [data << 1];
			else sl.SR = (int*) &g.NULL_RATE [0];

			sl.EincS = sl.SR [sl.KSR];
			if ((sl.Ecurp == SUBSTAIN) && (sl.Ecnt < ENV_END)) sl.Einc = sl.EincS;
			break;

		case 0x80:
			sl.SLL = g.SL_TAB [data >> 4];

			sl.RR = (int*) &g.DR_TAB [((data & 0xF) << 2) + 2];

			sl.EincR = sl.RR [sl.KSR];
			if ((sl.Ecurp == RELEASE) && (sl.Ecnt < ENV_END)) sl.Einc = sl.EincR;
			break;

		case 0x90:
			// SSG-EG envelope shapes :
			/*
			   E  At Al H
			  
			   1  0  0  0  \\\\
			   1  0  0  1  \___
			   1  0  1  0  \/\/
			   1  0  1  1  \
			   1  1  0  0  ////
			   1  1  0  1  /
			   1  1  1  0  /\/\
			   1  1  1  1  /___
			  
			   E  = SSG-EG enable
			   At = Start negate
			   Al = Altern
			   H  = Hold */
			//data = 0;
			//if(data & 8)
			// printf("SSG: %02x, %02x\n", Adr, data);
			set_seg( sl, (data & 8) ? (data & 0x0F) : 0 );
			break;
	}

	return 0;
}


int Ym2612_Impl::CHANNEL_SET( int Adr, int data )
{
	int num = Adr & 3;
	if ( num == 3 )
		return 1;
	
	channel_t& ch = YM2612.CHANNEL [num + (Adr & 0x100 ? 3 : 0)];
	
	switch ( Adr & 0xFC )
	{
		case 0xA0:
			YM2612_Special_Update();

                        ch.FNUM [0] = (ch.FNUM [0] & 0x700) + data;
			ch.KC [0] = (ch.FOCT [0] << 2) | FKEY_TAB [ch.FNUM [0] >> 7];
			ch.SLOT [0].Finc = -1;
			break;

		case 0xA4:
			YM2612_Special_Update();
			ch.FNUM [0] = (ch.FNUM [0] & 0x0FF) + ((data & 0x07) << 8);
			ch.FOCT [0] = (data & 0x38) >> 3;
			ch.KC [0] = (ch.FOCT [0] << 2) | FKEY_TAB [ch.FNUM [0] >> 7];
			ch.SLOT [0].Finc = -1;
			break;

		case 0xA8:
			if ( Adr < 0x100 )
			{
				num++;

				YM2612_Special_Update();

				YM2612.CHANNEL [2].FNUM [num] = (YM2612.CHANNEL [2].FNUM [num] & 0x700) + data;
				YM2612.CHANNEL [2].KC [num] = (YM2612.CHANNEL [2].FOCT [num] << 2) |
						FKEY_TAB [YM2612.CHANNEL [2].FNUM [num] >> 7];

				YM2612.CHANNEL [2].SLOT [0].Finc = -1;
			}
			break;

		case 0xAC:
			if ( Adr < 0x100 )
			{
				num++;

				YM2612_Special_Update();

				YM2612.CHANNEL [2].FNUM [num] = (YM2612.CHANNEL [2].FNUM [num] & 0x0FF) + ((data & 0x07) << 8);
				YM2612.CHANNEL [2].FOCT [num] = (data & 0x38) >> 3;
				YM2612.CHANNEL [2].KC [num] = (YM2612.CHANNEL [2].FOCT [num] << 2) |
						FKEY_TAB [YM2612.CHANNEL [2].FNUM [num] >> 7];

				YM2612.CHANNEL [2].SLOT [0].Finc = -1;
			}
			break;

		case 0xB0:
			if ( ch.ALGO != (data & 7) )
			{
				// Fix VectorMan 2 heli sound (level 1)
				YM2612_Special_Update();

				ch.ALGO = data & 7;
				
				ch.SLOT [0].ChgEnM = 0;
				ch.SLOT [1].ChgEnM = 0;
				ch.SLOT [2].ChgEnM = 0;
				ch.SLOT [3].ChgEnM = 0;
			}

			ch.FB = 9 - ((data >> 3) & 7);                              // Real thing ?

//          if (ch.FB = ((data >> 3) & 7)) ch.FB = 9 - ch.FB;       // Thunder force 4 (music stage 8), Gynoug, Aladdin bug sound...
//          else ch.FB = 31;
			break;

		case 0xB4: {
			YM2612_Special_Update();
			
			ch.LEFT = 0 - ((data >> 7) & 1);
			ch.RIGHT = 0 - ((data >> 6) & 1);
			
			ch.AMS = LFO_AMS_TAB [(data >> 4) & 3];
			ch.FMS = LFO_FMS_TAB [data & 7];
			
			for ( int i = 0; i < 4; i++ )
			{
				slot_t& sl = ch.SLOT [i];
				sl.AMS = (sl.AMSon ? ch.AMS : 31);
			}
			break;
		}
	}
	
	return 0;
}


int Ym2612_Impl::YM_SET(int Adr, int data)
{
	switch ( Adr )
	{
		case 0x22:
			if (data & 8) // LFO enable
			{
				// Cool Spot music 1, LFO modified severals time which
				// distord the sound, have to check that on a real genesis...

				g.LFOinc = g.LFO_INC_TAB [data & 7];
			}
			else
			{
				g.LFOinc = g.LFOcnt = 0;
			}
			break;

		case 0x24:
			YM2612.TimerA = (YM2612.TimerA & 0x003) | (((int) data) << 2);

			if (YM2612.TimerAL != (1024 - YM2612.TimerA))
			{
				YM2612.TimerAcnt = YM2612.TimerAL = (1024 - YM2612.TimerA);
			}
			break;

		case 0x25:
			YM2612.TimerA = (YM2612.TimerA & 0x3FC) | (data & 3);

			if (YM2612.TimerAL != (1024 - YM2612.TimerA))
			{
				YM2612.TimerAcnt = YM2612.TimerAL = (1024 - YM2612.TimerA);
			}
			break;

		case 0x26:
			YM2612.TimerB = data;

			if (YM2612.TimerBL != (256 - YM2612.TimerB) << 4)
			{
				YM2612.TimerBcnt = YM2612.TimerBL = (256 - YM2612.TimerB) << 4;
			}
			break;

		case 0x27:
			// Parametre divers
			// b7 = CSM MODE
			// b6 = 3 slot mode
			// b5 = reset b
			// b4 = reset a
			// b3 = timer enable b
			// b2 = timer enable a
			// b1 = load b
			// b0 = load a

			if ((data ^ YM2612.Mode) & 0x40)
			{
				// We changed the channel 2 mode, so recalculate phase step
				// This fix the punch sound in Street of Rage 2

				YM2612_Special_Update();

				YM2612.CHANNEL [2].SLOT [0].Finc = -1;      // recalculate phase step
			}

//          if ((data & 2) && (YM2612.Status & 2)) YM2612.TimerBcnt = YM2612.TimerBL;
//          if ((data & 1) && (YM2612.Status & 1)) YM2612.TimerAcnt = YM2612.TimerAL;

//          YM2612.Status &= (~data >> 4);                  // Reset du Status au cas ou c'est demande
			YM2612.Status &= (~data >> 4) & (data >> 2);    // Reset Status

			YM2612.Mode = data;
			break;

		case 0x28: {
			int nch = data & 3;
			if ( nch == 3 )
				return 1;
			if ( data & 4 )
				nch += 3;
			channel_t& ch = YM2612.CHANNEL [nch];

			YM2612_Special_Update();

			if (data & 0x10) KEY_ON(ch, S0);    // On appuie sur la touche pour le slot 1
			else KEY_OFF(ch, S0);               // On rel'che la touche pour le slot 1
			if (data & 0x20) KEY_ON(ch, S1);    // On appuie sur la touche pour le slot 3
			else KEY_OFF(ch, S1);               // On rel'che la touche pour le slot 3
			if (data & 0x40) KEY_ON(ch, S2);    // On appuie sur la touche pour le slot 2
			else KEY_OFF(ch, S2);               // On rel'che la touche pour le slot 2
			if (data & 0x80) KEY_ON(ch, S3);    // On appuie sur la touche pour le slot 4
			else KEY_OFF(ch, S3);               // On rel'che la touche pour le slot 4
			break;
		}
		case 0x2A:
			// FIXME:  Possibly a hack.
			if(data)
			 YM2612.DACdata = ((int)data - 0x80) << 5;
			break;

		case 0x2B:
			if (YM2612.DAC ^ (data & 0x80)) YM2612_Special_Update();
			YM2612.DAC = data & 0x80;   // activation/desactivation du DAC
			break;
	}
	
	return 0;
}

void Ym2612_Impl::set_rate(void)
{
	int i;

	// 144 = 12 * (prescale * 2) = 12 * 6 * 2
	// prescale set to 6 by default
	
	double Frequence = 1.0;
	double sample_rate = 53267;

	// Tableau TL :
	// [0     -  4095] = +output  [4095  - ...] = +output overflow (fill with 0)
	// [12288 - 16383] = -output  [16384 - ...] = -output overflow (fill with 0)

	for(i = 0; i < TL_LENGHT; i++)
	{
		if (i >= PG_CUT_OFF)    // YM2612 cut off sound after 78 dB (14 bits output ?)
		{
			g.TL_TAB [TL_LENGHT + i] = g.TL_TAB [i] = 0;
		}
		else
		{
			double x = MAX_OUT;                         // Max output
			x /= pow( 10.0, (ENV_STEP * i) / 20.0 );    // Decibel -> Voltage

			g.TL_TAB [i] = (int) x;
			g.TL_TAB [TL_LENGHT + i] = -g.TL_TAB [i];
		}
	}
	
	// Tableau SIN :
	// g.SIN_TAB [x] [y] = sin(x) * y; 
	// x = phase and y = volume

	g.SIN_TAB [0] = g.SIN_TAB [SIN_LENGHT / 2] = PG_CUT_OFF;

	for(i = 1; i <= SIN_LENGHT / 4; i++)
	{
		double x = sin(2.0 * PI * (double) (i) / (double) (SIN_LENGHT));    // Sinus
		x = 20 * log10(1 / x);                                      // convert to dB

		int j = (int) (x / ENV_STEP);                       // Get TL range

		if (j > PG_CUT_OFF) j = (int) PG_CUT_OFF;

		g.SIN_TAB [i] = g.SIN_TAB [(SIN_LENGHT / 2) - i] = j;
		g.SIN_TAB [(SIN_LENGHT / 2) + i] = g.SIN_TAB [SIN_LENGHT - i] = TL_LENGHT + j;
	}

	// Tableau LFO (LFO wav) :

	for(i = 0; i < LFO_LENGHT; i++)
	{
		double x = sin(2.0 * PI * (double) (i) / (double) (LFO_LENGHT));    // Sinus
		x += 1.0;
		x /= 2.0;                   // positive only
		x *= 11.8 / ENV_STEP;       // ajusted to MAX enveloppe modulation

		g.LFO_ENV_TAB [i] = (int) x;

		x = sin(2.0 * PI * (double) (i) / (double) (LFO_LENGHT));   // Sinus
		x *= (double) ((1 << (LFO_HBITS - 1)) - 1);

		g.LFO_FREQ_TAB [i] = (int) x;

	}

	// Tableau Enveloppe :
	// g.ENV_TAB [0] -> g.ENV_TAB [ENV_LENGHT - 1]              = attack curve
	// g.ENV_TAB [ENV_LENGHT] -> g.ENV_TAB [2 * ENV_LENGHT - 1] = decay curve

	for(i = 0; i < ENV_LENGHT; i++)
	{
		// Attack curve (x^8 - music level 2 Vectorman 2)
		double x = pow(((double) ((ENV_LENGHT - 1) - i) / (double) (ENV_LENGHT)), 8);
		x *= ENV_LENGHT;

		g.ENV_TAB [i] = (int) x;

		// Decay curve (just linear)
		x = pow(((double) (i) / (double) (ENV_LENGHT)), 1);
		x *= ENV_LENGHT;

		g.ENV_TAB [ENV_LENGHT + i] = (int) x;
	}
	for ( i = 0; i < 8; i++ )
		g.ENV_TAB [i + ENV_LENGHT * 2] = 0;
	
	g.ENV_TAB [ENV_END >> ENV_LBITS] = ENV_LENGHT - 1;      // for the stopped state
	
	// Tableau pour la conversion Attack -> Decay and Decay -> Attack
	
	int j = ENV_LENGHT - 1;
	for ( i = 0; i < ENV_LENGHT; i++ )
	{
		while ( j && g.ENV_TAB [j] < i )
			j--;

		g.DECAY_TO_ATTACK [i] = j << ENV_LBITS;
	}

	// Tableau pour le Substain Level
	
	for(i = 0; i < 15; i++)
	{
		double x = i * 3;           // 3 and not 6 (Mickey Mania first music for test)
		x /= ENV_STEP;

		g.SL_TAB [i] = ((int) x << ENV_LBITS) + ENV_DECAY;
	}

	g.SL_TAB [15] = ((ENV_LENGHT - 1) << ENV_LBITS) + ENV_DECAY; // special case : volume off

	// Tableau Frequency Step

	for(i = 0; i < 2048; i++)
	{
		double x = (double) (i) * Frequence;

#if ((SIN_LBITS + SIN_HBITS - (21 - 7)) < 0)
		x /= (double) (1 << ((21 - 7) - SIN_LBITS - SIN_HBITS));
#else
		x *= (double) (1 << (SIN_LBITS + SIN_HBITS - (21 - 7)));
#endif

		x /= 2.0;   // because MUL = value * 2

		g.FINC_TAB [i] = (unsigned int) x;
	}

	// Tableaux Attack & Decay Rate

	for(i = 0; i < 4; i++)
	{
		g.AR_TAB [i] = 0;
		g.DR_TAB [i] = 0;
	}
	
	for(i = 0; i < 60; i++)
	{
		double x = Frequence;

		x *= 1.0 + ((i & 3) * 0.25);                    // bits 0-1 : x1.00, x1.25, x1.50, x1.75
		x *= (double) (1 << ((i >> 2)));                // bits 2-5 : shift bits (x2^0 - x2^15)
		x *= (double) (ENV_LENGHT << ENV_LBITS);        // on ajuste pour le tableau g.ENV_TAB

		g.AR_TAB [i + 4] = (unsigned int) (x / AR_RATE);
		g.DR_TAB [i + 4] = (unsigned int) (x / DR_RATE);
	}

	for(i = 64; i < 96; i++)
	{
		g.AR_TAB [i] = g.AR_TAB [63];
		g.DR_TAB [i] = g.DR_TAB [63];

		g.NULL_RATE [i - 64] = 0;
	}
	
	for ( i = 96; i < 128; i++ )
		g.AR_TAB [i] = 0;
	
	// Tableau Detune

	for(i = 0; i < 4; i++)
	{
		for (int j = 0; j < 32; j++)
		{
#if ((SIN_LBITS + SIN_HBITS - 21) < 0)
			double y = (double) DT_DEF_TAB [(i << 5) + j] * Frequence / (double) (1 << (21 - SIN_LBITS - SIN_HBITS));
#else
			double y = (double) DT_DEF_TAB [(i << 5) + j] * Frequence * (double) (1 << (SIN_LBITS + SIN_HBITS - 21));
#endif

			g.DT_TAB [i + 0] [j] = (int)  y;
			g.DT_TAB [i + 4] [j] = (int) -y;
		}
	}
	
	// Tableau LFO
	g.LFO_INC_TAB [0] = (unsigned int) (3.98 * (double) (1 << (LFO_HBITS + LFO_LBITS)) / sample_rate);
	g.LFO_INC_TAB [1] = (unsigned int) (5.56 * (double) (1 << (LFO_HBITS + LFO_LBITS)) / sample_rate);
	g.LFO_INC_TAB [2] = (unsigned int) (6.02 * (double) (1 << (LFO_HBITS + LFO_LBITS)) / sample_rate);
	g.LFO_INC_TAB [3] = (unsigned int) (6.37 * (double) (1 << (LFO_HBITS + LFO_LBITS)) / sample_rate);
	g.LFO_INC_TAB [4] = (unsigned int) (6.88 * (double) (1 << (LFO_HBITS + LFO_LBITS)) / sample_rate);
	g.LFO_INC_TAB [5] = (unsigned int) (9.63 * (double) (1 << (LFO_HBITS + LFO_LBITS)) / sample_rate);
	g.LFO_INC_TAB [6] = (unsigned int) (48.1 * (double) (1 << (LFO_HBITS + LFO_LBITS)) / sample_rate);
	g.LFO_INC_TAB [7] = (unsigned int) (72.2 * (double) (1 << (LFO_HBITS + LFO_LBITS)) / sample_rate);
	
	reset();
}

const char* Ym2612_Emu::set_rate(void)
{
	if ( !impl )
	{
		impl = (Ym2612_Impl*) malloc( sizeof *impl );
		if ( !impl )
			return "Out of memory";
		impl->mute_mask = 0;
	}
	memset( &impl->YM2612, 0, sizeof impl->YM2612 );
	
	impl->set_rate();
	
	return 0;
}

Ym2612_Emu::~Ym2612_Emu()
{
	free( impl );
}

inline void Ym2612_Impl::write0( int opn_addr, int data )
{
	assert( (unsigned) data <= 0xFF );
	
	if ( opn_addr < 0x30 )
	{
		YM2612.REG [0] [opn_addr] = data;
		YM_SET( opn_addr, data );
	}
	else if ( YM2612.REG [0] [opn_addr] != data )
	{
		YM2612.REG [0] [opn_addr] = data;
		
		if ( opn_addr < 0xA0 )
			SLOT_SET( opn_addr, data );
		else
			CHANNEL_SET( opn_addr, data );
	}
}

inline void Ym2612_Impl::write1( int opn_addr, int data )
{
	assert( (unsigned) data <= 0xFF );
	
	if ( opn_addr >= 0x30 && YM2612.REG [1] [opn_addr] != data )
	{
		YM2612.REG [1] [opn_addr] = data;

		if ( opn_addr < 0xA0 )
			SLOT_SET( opn_addr + 0x100, data );
		else
			CHANNEL_SET( opn_addr + 0x100, data );
	}
}

inline int Ym2612_Impl::read( void )
{
	return(YM2612.Status);
}

void Ym2612_Emu::reset()
{
	impl->reset();
}

void Ym2612_Impl::reset()
{
	g.LFOcnt = 0;
	YM2612.TimerA = 0;
	YM2612.TimerAL = 0;
	YM2612.TimerAcnt = 0;
	YM2612.TimerB = 0;
	YM2612.TimerBL = 0;
	YM2612.TimerBcnt = 0;
	YM2612.DAC = 0;
	YM2612.DACdata = 0;
	YM2612.Status = 0;

	int i;
	for ( i = 0; i < channel_count; i++ )
	{
		channel_t& ch = YM2612.CHANNEL [i];
		
		ch.LEFT = ~0;
		ch.RIGHT = ~0;
		ch.ALGO = 0;
		ch.FB = 31;
		ch.FMS = 0;
		ch.AMS = 0;

		for ( int j = 0 ;j < 4 ; j++ )
		{
			ch.S0_OUT [j] = 0;
			ch.FNUM [j] = 0;
			ch.FOCT [j] = 0;
			ch.KC [j] = 0;

			ch.SLOT [j].Fcnt = 0;
			ch.SLOT [j].Finc = 0;
			ch.SLOT [j].Ecnt = ENV_END;     // Put it at the end of Decay phase...
			ch.SLOT [j].Einc = 0;
			ch.SLOT [j].Ecmp = 0;
			ch.SLOT [j].Ecurp = RELEASE;

			ch.SLOT [j].ChgEnM = 0;
		}
	}

	for ( i = 0; i < 0x100; i++ )
	{
		YM2612.REG [0] [i] = -1;
		YM2612.REG [1] [i] = -1;
	}

	for ( i = 0xB6; i >= 0xB4; i-- )
	{
		write0( i, 0xC0 );
		write1( i, 0xC0 );
	}

	for ( i = 0xB2; i >= 0x22; i-- )
	{
		write0( i, 0 );
		write1( i, 0 );
	}
	
	write0( 0x2A, 0x80 );
}

void Ym2612_Emu::write0( int addr, int data )
{
	impl->write0( addr, data );
}

void Ym2612_Emu::write1( int addr, int data )
{
	impl->write1( addr, data );
}

int Ym2612_Emu::read(void)
{
	return(impl->read());
}

void Ym2612_Emu::mute_voices( int mask ) { impl->mute_mask = mask; }

unsigned int Ym2612_Emu::get_state_max_len(void)
{
 // An approximation
 //printf("YM: %d\n", sizeof(state_t));
 //return(sizeof(state_t) * 2);
 return(8192);
}

#define PACK_INT(val)	{ int temp = (val); buffer[index++] = temp; buffer[index++] = temp >> 8; buffer[index++] = temp >> 16; buffer[index++] = temp >> 24; }

#define UNPACK_INT(target) { if(index >= length) goto BadBad; target = buffer[index] | (buffer[index + 1] << 8) | (buffer[index + 2] << 16) | (buffer[index + 3] << 24); index += 4; }

#define UNPACK_INT_P(target, table)	\
{			\
 int tt;		\
 UNPACK_INT(tt);	\
 if(tt >= ((int)sizeof(table) - (int)sizeof(int) + 1))\
 {					\
  puts("Bogus!");			\
  tt = 0;				\
 }					\
 if(tt < 0)				\
  target = (int *)&g->NULL_RATE[0];	\
 else					\
  target = (int *)((char *)table + tt);	\
}

void Ym2612_Emu::save_state(unsigned char *buffer)
{
 unsigned int index = 0;
 state_t *s = &impl->YM2612;
 tables_t *g = &impl->g;

 PACK_INT(0);
 PACK_INT(s->Status);
 PACK_INT(s->TimerA);
 PACK_INT(s->TimerAL);
 PACK_INT(s->TimerAcnt);
 PACK_INT(s->TimerB);
 PACK_INT(s->TimerBL);
 PACK_INT(s->TimerBcnt);
 PACK_INT(s->Mode);
 PACK_INT(s->DAC);
 PACK_INT(s->DACdata);

 for(int ch = 0; ch < channel_count; ch++)
 {
  channel_t *c = &s->CHANNEL[ch];

  for(int i = 0; i < 4; i++)
  {
   PACK_INT(c->S0_OUT[i]);
  }

  PACK_INT(c->LEFT);
  PACK_INT(c->RIGHT);
  PACK_INT(c->ALGO);
  PACK_INT(c->FB);
  PACK_INT(c->FMS);
  PACK_INT(c->AMS);

  for(int i = 0; i < 4; i++)
  {
   PACK_INT(c->FNUM[i]);
  }

  for(int i = 0; i < 4; i++)
  {
   PACK_INT(c->FOCT[i]);
  }
 
  for(int i = 0; i < 4; i++)
  {
   PACK_INT(c->KC[i]);
  }

  for(int slot = 0; slot < 4; slot++)
  {
   slot_t *sl = &c->SLOT[slot];
   
   PACK_INT((char *)sl->DT - (char *)g->DT_TAB);
   PACK_INT(sl->MUL);
   PACK_INT(sl->TL);
   PACK_INT(sl->TLL);
   PACK_INT(sl->SLL);
   PACK_INT(sl->KSR_S);
   PACK_INT(sl->KSR);
   PACK_INT(sl->SEG);
   PACK_INT(sl->env_xor);
   PACK_INT(sl->env_max);

   if(sl->AR == (int *)&g->NULL_RATE[0])
   {
    PACK_INT(-1);
   }
   else
   {
    PACK_INT((char *)sl->AR - (char *)g->AR_TAB);
   }

   if(sl->DR == (int *)&g->NULL_RATE[0])
   {
    PACK_INT(-1);
   }
   else
   {
    PACK_INT((char *)sl->DR - (char *)g->DR_TAB);
   }

   if(sl->SR == (int *)&g->NULL_RATE[0])
   {
    PACK_INT(-1);
   }
   else
   {
    PACK_INT((char *)sl->SR - (char *)g->DR_TAB);
   }

   if(sl->RR == (int *)&g->NULL_RATE[0])
   {
    PACK_INT(-1);
   }
   else
   {
    PACK_INT((char *)sl->RR - (char *)g->DR_TAB);
   }


   PACK_INT(sl->Fcnt);
   PACK_INT(sl->Finc);
   PACK_INT(sl->Ecurp);
   PACK_INT(sl->Ecnt);
   PACK_INT(sl->Einc);
   PACK_INT(sl->Ecmp);
   PACK_INT(sl->EincA);
   PACK_INT(sl->EincD);
   PACK_INT(sl->EincS);
   PACK_INT(sl->EincR);

   PACK_INT(sl->INd);
   PACK_INT(sl->ChgEnM);
   PACK_INT(sl->AMS);
   PACK_INT(sl->AMSon);
  }

 
  PACK_INT(c->FFlag);
 }

 for(int r0 = 0; r0 < 2; r0++)
  for(int r1 = 0; r1 < 0x100; r1++)
  {
   PACK_INT(s->REG[r0][r1]);
  }

// printf("Save: %d\n", index);
// impl->YM2612;
}

void Ym2612_Emu::load_state(const unsigned char *buffer)
{
 unsigned int length = get_state_max_len();

 state_t *s = &impl->YM2612;
 tables_t *g = &impl->g;
 unsigned int index = 0;
 int version;
 int tun;

 UNPACK_INT(version);

 UNPACK_INT(s->Status);
 UNPACK_INT(s->TimerA);
 UNPACK_INT(s->TimerAL);
 UNPACK_INT(s->TimerAcnt);
 UNPACK_INT(s->TimerB);
 UNPACK_INT(s->TimerBL);
 UNPACK_INT(s->TimerBcnt);
 UNPACK_INT(s->Mode);
 UNPACK_INT(s->DAC);
 UNPACK_INT(s->DACdata);

 for(int ch = 0; ch < channel_count; ch++)
 {
  channel_t *c = &s->CHANNEL[ch];

  for(int i = 0; i < 4; i++)
  {
   UNPACK_INT(c->S0_OUT[i]);
  }

  UNPACK_INT(c->LEFT);
  UNPACK_INT(c->RIGHT);
  UNPACK_INT(c->ALGO);
  UNPACK_INT(c->FB);
  UNPACK_INT(c->FMS);
  UNPACK_INT(c->AMS);

  for(int i = 0; i < 4; i++)
  {
   UNPACK_INT(c->FNUM[i]);
  }

  for(int i = 0; i < 4; i++)
  {
   UNPACK_INT(c->FOCT[i]);
  }
  for(int i = 0; i < 4; i++)
  {
   UNPACK_INT(c->KC[i]);
  }

  for(int slot = 0; slot < 4; slot++)
  {
   slot_t *sl = &c->SLOT[slot];

   UNPACK_INT(tun);
   sl->DT = (int *)((char *)g->DT_TAB + tun);

   UNPACK_INT(sl->MUL);
   UNPACK_INT(sl->TL);
   UNPACK_INT(sl->TLL);
   UNPACK_INT(sl->SLL);
   UNPACK_INT(sl->KSR_S);
   UNPACK_INT(sl->KSR);
   UNPACK_INT(sl->SEG);
   UNPACK_INT(sl->env_xor);
   UNPACK_INT(sl->env_max);

   UNPACK_INT_P(sl->AR, g->AR_TAB);
   UNPACK_INT_P(sl->DR, g->DR_TAB);
   UNPACK_INT_P(sl->SR, g->DR_TAB);
   UNPACK_INT_P(sl->RR, g->DR_TAB);

   UNPACK_INT(sl->Fcnt);
   UNPACK_INT(sl->Finc);
   UNPACK_INT(sl->Ecurp);
   UNPACK_INT(sl->Ecnt);
   UNPACK_INT(sl->Einc);
   UNPACK_INT(sl->Ecmp);
   UNPACK_INT(sl->EincA);
   UNPACK_INT(sl->EincD);
   UNPACK_INT(sl->EincS);
   UNPACK_INT(sl->EincR);

   UNPACK_INT(sl->INd);
   UNPACK_INT(sl->ChgEnM);
   UNPACK_INT(sl->AMS);
   UNPACK_INT(sl->AMSon);
  }


  UNPACK_INT(c->FFlag);
 }

 for(int r0 = 0; r0 < 2; r0++)
  for(int r1 = 0; r1 < 0x100; r1++)
  {
   UNPACK_INT(s->REG[r0][r1]);
  }

 //printf("Load: %d\n", index);
 return;

 BadBad:

 printf("Bad: %d %d\n", index, length);
 return;
}

static void update_envelope_( slot_t* sl )
{
	switch ( sl->Ecurp )
	{
	case 0:
		// Env_Attack_Next
		
		// Verified with Gynoug even in HQ (explode SFX)
		sl->Ecnt = ENV_DECAY;

		sl->Einc = sl->EincD;
		sl->Ecmp = sl->SLL;
		sl->Ecurp = DECAY;
		break;
	
	case 1:
		// Env_Decay_Next
		
		// Verified with Gynoug even in HQ (explode SFX)
		sl->Ecnt = sl->SLL;

		sl->Einc = sl->EincS;
		sl->Ecmp = ENV_END;
		sl->Ecurp = SUBSTAIN;
		break;
	
	case 2:
		// Env_Substain_Next(slot_t *SL)
		if (sl->SEG & 8)    // SSG envelope type
		{
			int release = sl->SEG & 1;
			
			if ( !release )
			{
				// re KEY ON

				// sl->Fcnt = 0;
				// sl->ChgEnM = ~0;

				sl->Ecnt = 0;
				sl->Einc = sl->EincA;
				sl->Ecmp = ENV_DECAY;
				sl->Ecurp = ATTACK;
			}

			set_seg( *sl, (sl->SEG << 1) & 4 );
			
			if ( !release )
				break;
		}
		// fall through
	
	case 3:
		// Env_Release_Next
		sl->Ecnt = ENV_END;
		sl->Einc = 0;
		sl->Ecmp = ENV_END + 1;
		break;
	
	// default: no op
	}
}

inline void update_envelope( slot_t& sl )
{
	int ecmp = sl.Ecmp;
	if ( (sl.Ecnt += sl.Einc) >= ecmp )
		update_envelope_( &sl );
}

template<int algo>
struct ym2612_update_chan {
	static void func( tables_t&, channel_t&, Ym2612_Emu::sample_t*, int );
};

typedef void (*ym2612_update_chan_t)( tables_t&, channel_t&, Ym2612_Emu::sample_t*, int );

template<int algo>
void ym2612_update_chan<algo>::func( tables_t& g, channel_t& ch,
		Ym2612_Emu::sample_t* buf, int length )
{
	int not_end = ch.SLOT [S3].Ecnt - ENV_END;
	
	// algo is a compile-time constant, so all conditions based on it are resolved
	// during compilation
	
	// special cases
	if ( algo == 7 )
		not_end |= ch.SLOT [S0].Ecnt - ENV_END;
	
	if ( algo >= 5 )
		not_end |= ch.SLOT [S2].Ecnt - ENV_END;
	
	if ( algo >= 4 )
		not_end |= ch.SLOT [S1].Ecnt - ENV_END;
	
	int CH_S0_OUT_1 = ch.S0_OUT [1];
	
	int in0 = ch.SLOT [S0].Fcnt;
	int in1 = ch.SLOT [S1].Fcnt;
	int in2 = ch.SLOT [S2].Fcnt;
	int in3 = ch.SLOT [S3].Fcnt;
	
	int YM2612_LFOinc = g.LFOinc;
	int YM2612_LFOcnt = g.LFOcnt + YM2612_LFOinc;
	
	if ( !not_end )
		return;
	
	do
	{
		// envelope
		int const env_LFO = g.LFO_ENV_TAB [YM2612_LFOcnt >> LFO_LBITS & LFO_MASK];
		
		short const* const ENV_TAB = g.ENV_TAB;
		
	#define CALC_EN( x ) \
		int temp##x = ENV_TAB [ch.SLOT [S##x].Ecnt >> ENV_LBITS] + ch.SLOT [S##x].TLL;  \
		int en##x = ((temp##x ^ ch.SLOT [S##x].env_xor) + (env_LFO >> ch.SLOT [S##x].AMS)) &    \
				((temp##x - ch.SLOT [S##x].env_max) >> 31);
		
		CALC_EN( 0 )
		CALC_EN( 1 )
		CALC_EN( 2 )
		CALC_EN( 3 )
		
		int const* const TL_TAB = g.TL_TAB;
		
	#define SINT( i, o ) (TL_TAB [g.SIN_TAB [(i)] + (o)])
		
		// feedback
		int CH_S0_OUT_0 = ch.S0_OUT [0];
		{
			int temp = in0 + ((CH_S0_OUT_0 + CH_S0_OUT_1) >> ch.FB);
			CH_S0_OUT_1 = CH_S0_OUT_0;
			CH_S0_OUT_0 = SINT( (temp >> SIN_LBITS) & SIN_MASK, en0 );
		}
		
		int CH_OUTd;
		if ( algo == 0 )
		{
			int temp = in1 + CH_S0_OUT_1;
			temp = in2 + SINT( (temp >> SIN_LBITS) & SIN_MASK, en1 );
			temp = in3 + SINT( (temp >> SIN_LBITS) & SIN_MASK, en2 );
			CH_OUTd = SINT( (temp >> SIN_LBITS) & SIN_MASK, en3 );
		}
		else if ( algo == 1 )
		{
			int temp = in2 + CH_S0_OUT_1 + SINT( (in1 >> SIN_LBITS) & SIN_MASK, en1 );
			temp = in3 + SINT( (temp >> SIN_LBITS) & SIN_MASK, en2 );
			CH_OUTd = SINT( (temp >> SIN_LBITS) & SIN_MASK, en3 );
		}
		else if ( algo == 2 )
		{
			int temp = in2 + SINT( (in1 >> SIN_LBITS) & SIN_MASK, en1 );
			temp = in3 + CH_S0_OUT_1 + SINT( (temp >> SIN_LBITS) & SIN_MASK, en2 );
			CH_OUTd = SINT( (temp >> SIN_LBITS) & SIN_MASK, en3 );
		}
		else if ( algo == 3 )
		{
			int temp = in1 + CH_S0_OUT_1;
			temp = in3 + SINT( (temp >> SIN_LBITS) & SIN_MASK, en1 ) +
					SINT( (in2 >> SIN_LBITS) & SIN_MASK, en2 );
			CH_OUTd = SINT( (temp >> SIN_LBITS) & SIN_MASK, en3 );
		}
		else if ( algo == 4 )
		{
			int temp = in3 + SINT( (in2 >> SIN_LBITS) & SIN_MASK, en2 );
			CH_OUTd = SINT( (temp >> SIN_LBITS) & SIN_MASK, en3 ) +
					SINT( ((in1 + CH_S0_OUT_1) >> SIN_LBITS) & SIN_MASK, en1 );
			//DO_LIMIT
		}
		else if ( algo == 5 )
		{
			int temp = CH_S0_OUT_1;
			CH_OUTd = SINT( ((in3 + temp) >> SIN_LBITS) & SIN_MASK, en3 ) +
					SINT( ((in1 + temp) >> SIN_LBITS) & SIN_MASK, en1 ) +
					SINT( ((in2 + temp) >> SIN_LBITS) & SIN_MASK, en2 );
			//DO_LIMIT
		}
		else if ( algo == 6 )
		{
			CH_OUTd = SINT( (in3 >> SIN_LBITS) & SIN_MASK, en3 ) +
					SINT( ((in1 + CH_S0_OUT_1) >> SIN_LBITS) & SIN_MASK, en1 ) +
					SINT( (in2 >> SIN_LBITS) & SIN_MASK, en2 );
			//DO_LIMIT
		}
		else if ( algo == 7 )
		{
			CH_OUTd = SINT( (in3 >> SIN_LBITS) & SIN_MASK, en3 ) +
					SINT( (in1 >> SIN_LBITS) & SIN_MASK, en1 ) +
					SINT( (in2 >> SIN_LBITS) & SIN_MASK, en2 ) + CH_S0_OUT_1;
			//DO_LIMIT
		}
		
		CH_OUTd >>= MAX_OUT_BITS - output_bits + 2;
		
		// update phase
		unsigned freq_LFO = ((g.LFO_FREQ_TAB [YM2612_LFOcnt >> LFO_LBITS & LFO_MASK] *
				ch.FMS) >> (LFO_HBITS - 1 + 1)) + (1L << (LFO_FMS_LBITS - 1));
		YM2612_LFOcnt += YM2612_LFOinc;
		in0 += (ch.SLOT [S0].Finc * freq_LFO) >> (LFO_FMS_LBITS - 1);
		in1 += (ch.SLOT [S1].Finc * freq_LFO) >> (LFO_FMS_LBITS - 1);
		in2 += (ch.SLOT [S2].Finc * freq_LFO) >> (LFO_FMS_LBITS - 1);
		in3 += (ch.SLOT [S3].Finc * freq_LFO) >> (LFO_FMS_LBITS - 1);
		
		int t0 = buf [0] + (CH_OUTd & ch.LEFT);
		int t1 = buf [1] + (CH_OUTd & ch.RIGHT);
		
		update_envelope( ch.SLOT [0] );
		update_envelope( ch.SLOT [1] );
		update_envelope( ch.SLOT [2] );
		update_envelope( ch.SLOT [3] );
		
		ch.S0_OUT [0] = CH_S0_OUT_0;
		buf [0] = t0;
		buf [1] = t1;
		buf += 2;
	}
	while ( --length );
	
	ch.S0_OUT [1] = CH_S0_OUT_1;
	
	ch.SLOT [S0].Fcnt = in0;
	ch.SLOT [S1].Fcnt = in1;
	ch.SLOT [S2].Fcnt = in2;
	ch.SLOT [S3].Fcnt = in3;
}

static const ym2612_update_chan_t UPDATE_CHAN [8] = {
	&ym2612_update_chan<0>::func,
	&ym2612_update_chan<1>::func,
	&ym2612_update_chan<2>::func,
	&ym2612_update_chan<3>::func,
	&ym2612_update_chan<4>::func,
	&ym2612_update_chan<5>::func,
	&ym2612_update_chan<6>::func,
	&ym2612_update_chan<7>::func
};

void Ym2612_Impl::run_timer( int length )
{
	int remain = length;
	do
	{
		if (YM2612.Mode & 1)                            // Timer A ON ?
		{
		 YM2612.TimerAcnt--;

		 if(YM2612.TimerAcnt <= 0)
		 {
		  // timer a overflow		
		  YM2612.Status |= (YM2612.Mode & 0x04) >> 2;
		  YM2612.TimerAcnt += YM2612.TimerAL;

		  if (YM2612.Mode & 0x80)
		  {
			KEY_ON( YM2612.CHANNEL [2], 0 );
			KEY_ON( YM2612.CHANNEL [2], 1 );
			KEY_ON( YM2612.CHANNEL [2], 2 );
			KEY_ON( YM2612.CHANNEL [2], 3 );
		  }
		 }
		}


		if (YM2612.Mode & 2)                            // Timer B ON ?
		{
		 YM2612.TimerBcnt--;

		 if (YM2612.TimerBcnt <= 0)
		 {
		  // timer b overflow
		  YM2612.Status |= (YM2612.Mode & 0x08) >> 2;
		  YM2612.TimerBcnt += YM2612.TimerBL;
		 }
		}

		remain--;
	}
	while ( remain > 0 );
}

void Ym2612_Impl::run( int pair_count, Ym2612_Emu::sample_t* out )
{
	if ( pair_count <= 0 )
		return;
	
	if ( YM2612.Mode & 3 )
		run_timer( pair_count );
	
	// Mise à jour des pas des compteurs-frequences s'ils ont ete modifies
	
	for ( int chi = 0; chi < channel_count; chi++ )
	{
		channel_t& ch = YM2612.CHANNEL [chi];
		if ( ch.SLOT [0].Finc != -1 )
			continue;
		
		int i2 = 0;
		if ( chi == 2 && (YM2612.Mode & 0x40) )
			i2 = 2;
		
		for ( int i = 0; i < 4; i++ )
		{
			// static int seq [4] = { 2, 1, 3, 0 };
			// if ( i2 ) i2 = seq [i];
			
			slot_t& sl = ch.SLOT [i];
			int finc = g.FINC_TAB [ch.FNUM [i2]] >> (7 - ch.FOCT [i2]);
			int ksr = ch.KC [i2] >> sl.KSR_S;   // keycode attenuation
			sl.Finc = (finc + sl.DT [ch.KC [i2]]) * sl.MUL;
			if (sl.KSR != ksr)          // si le KSR a change alors
			{                       // les differents taux pour l'enveloppe sont mis à jour
				sl.KSR = ksr;

				sl.EincA = sl.AR [ksr];
				sl.EincD = sl.DR [ksr];
				sl.EincS = sl.SR [ksr];
				sl.EincR = sl.RR [ksr];

				if (sl.Ecurp == ATTACK)
				{
					sl.Einc = sl.EincA;
				}
				else if (sl.Ecurp == DECAY)
				{
					sl.Einc = sl.EincD;
				}
				else if (sl.Ecnt < ENV_END)
				{
					if (sl.Ecurp == SUBSTAIN)
						sl.Einc = sl.EincS;
					else if (sl.Ecurp == RELEASE)
						sl.Einc = sl.EincR;
				}
			}
			
			if ( i2 )
				i2 = (i2 ^ 2) ^ (i2 >> 1);
		}
	}
	
	for ( int i = 0; i < channel_count; i++ )
	{
		if ( !(mute_mask & (1 << i)))
		{
		  if(i == 5 && YM2612.DAC)
		  {
			 out[0] += YM2612.DACdata & YM2612.CHANNEL[5].LEFT;
			 out[1] += YM2612.DACdata & YM2612.CHANNEL[5].RIGHT;
		  }
		  else
		  {
			UPDATE_CHAN [YM2612.CHANNEL [i].ALGO]( g, YM2612.CHANNEL [i], out, pair_count );
		  }
		}
	}	
	g.LFOcnt += g.LFOinc * pair_count;
}

void Ym2612_Emu::run( int pair_count, sample_t* out ) { impl->run( pair_count, out ); }
