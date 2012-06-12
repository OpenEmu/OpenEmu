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

// Game_Music_Emu 0.5.2. http://www.slack.net/~ant/

// Based on Gens 2.10 ym2612.c

/* Some emulation improvements(ostensibly!) done for Mednafen:
	Handle detune underflow correctly(hopefully. :b)

	Don't update active fnum/block(octave) on $A4-$A6, $AC-$AE writes; only on $A0-$A2, $A8-$AA writes.

	LFO FM applied to 11-bit fnum

	Rewritten EG code.

	Misc other stuff.
*/


#include "Ym2612_Emu.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>

const int output_bits = 14;

#include "ym2612_opeg.inc"

struct slot_t
{
	const int *DT;  // parametre detune
	int MUL;    // parametre "multiple de frequence"
	int KSR_S;  // Key Scale Rate Shift = facteur de prise en compte du KSL dans la variations de l'enveloppe

	int Fcnt;   // Frequency Count = compteur-frequence pour determiner l'amplitude actuelle (SIN[Finc >> 16])
	int INd;    // input data of the slot = donnees en entree du slot
	int AMS;    // AMS depth level of this SLOT = degre de modulation de l'amplitude par le LFO
	int AMSon;  // AMS enable flag = drapeau d'activation de l'AMS

	OpEG eg;
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

	uint32 EGCycleCounter;
	uint32 EGDivCounter;

	int LFOcnt;         // LFO counter = compteur-frequence pour le LFO
	int LFOinc;         // LFO step counter = pas d'incrementation du compteur-frequence du LFO
						// plus le pas est grand, plus la frequence est grande

	channel_t CHANNEL[Ym2612_Emu::channel_count];   // Les 6 voies du YM2612
	int REG[2][0x100];  // Sauvegardes des valeurs de tout les registres, c'est facultatif
						// cela nous rend le debuggage plus facile
};

#ifndef PI
#define PI 3.14159265358979323846
#endif

// SIN_LBITS <= 16
// LFO_HBITS <= 16
// (SIN_LBITS + SIN_HBITS) <= 26
// (LFO_LBITS + LFO_HBITS) <= 28

// DON'T CHANGE THESE TWO DEFINES.
#define SIN_HBITS      10                               // Sinus phase counter int part
#define SIN_LBITS      10				// Sinus phase counter float part (best setting)



#define ENV_HBITS      12                               // Env phase counter int part

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

#define MAX_OUT_BITS   (SIN_HBITS + SIN_LBITS + 2)      // Modulation = -4 <--> +4
#define MAX_OUT        ((1 << MAX_OUT_BITS) - 1)

#define PG_CUT_OFF     ((int) (78.0 / ENV_STEP))

#define S0             0    // Stupid typo of the YM2612
#define S1             2
#define S2             1
#define S3             3

struct tables_t
{
	short SIN_TAB [SIN_LENGHT];                 // SINUS TABLE (offset into TL TABLE)
	int DT_TAB [8] [32];               // Detune table
	int LFO_INC_TAB [8];                        // LFO step table
	
	short LFO_ENV_TAB [LFO_LENGHT];             // LFO AMS TABLE (adjusted for 11.8 dB)
	int TL_TAB [TL_LENGHT * 2];                 // TOTAL LEVEL TABLE (positif and minus)
};

static const unsigned char DT_DEF_TAB [4][32] =
{
// FD = 0
 {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 },

// FD = 1
 {
  0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
  2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 8, 8, 8, 8,
 },

// FD = 2
 {
  1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5,
  5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 16, 16, 16, 16,
 },

// FD = 3
 {
  2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7,
  8 , 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 20, 22, 22, 22, 22
 }
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
	31, 3, 1, 0
};

static const unsigned int LFO_PhaseMod_Table[8][8] = 
{
   { 0, 0, 0, 0, 0, 0, 0, 0 },	// 0
   { 0, 0, 0, 0, 1, 1, 1, 1 },  // 1
   { 0, 0, 0, 1, 1, 1, 2, 2 },  // 2
   { 0, 0, 1, 1, 2, 2, 3, 3 },  // 3
   { 0, 0, 1, 2, 2, 2, 3, 4 },  // 4
   { 0, 0, 2, 3, 4, 4, 5, 6 },  // 5
   { 0, 0, 4, 6, 8, 8,10,12 },  // 6
   { 0, 0, 8,12,16,16,20,24 },  // 7
};

struct Ym2612_Impl
{
	enum { channel_count = Ym2612_Emu::channel_count };
	
	state_t YM2612;
	int mute_mask;
	tables_t g;
	
	int SLOT_SET( int, int );
	int CHANNEL_SET( int, int );
	int YM_SET( int, int );
	
	void set_rate(void);
	void reset();
	void write0( int addr, int data );
	void write1( int addr, int data );
	int read(void);
	void run_timer( void );
	void run( Ym2612_Emu::sample_t* );
};

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

			sl.DT = &g.DT_TAB [(data >> 4) & 7][0];

			break;

		case 0x40:
			sl.eg.SetTL(data & 0x7F);
			break;

		case 0x50:
			sl.eg.SetAR(data & 0x1F);

			sl.KSR_S = 3 - (data >> 6);

			break;

		case 0x60:
			sl.eg.SetDR(data & 0x1F);

			if ( (sl.AMSon = (data & 0x80)) != 0 ) sl.AMS = ch.AMS;
			else sl.AMS = 31;
			break;

		case 0x70:
			sl.eg.SetSR(data & 0x1F);
			break;

		case 0x80:
			sl.eg.SetSL((data >> 4) & 0x0F);
			sl.eg.SetRR(data & 0x0F);
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
			//printf("SSG: %02x, %02x\n", Adr, data);
			sl.eg.SetSSG_EG(data & 0x0F);
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
			{
			 int od = YM2612.REG[(bool)(Adr & 0x100)][(Adr & 0xFF) | 0x04];

                         ch.FNUM [0] = (ch.FNUM [0] & 0x0FF) + ((od & 0x07) << 8);
                         ch.FOCT [0] = (od & 0x38) >> 3;
			}

                        ch.FNUM [0] = (ch.FNUM [0] & 0x700) + data;
			ch.KC [0] = (ch.FOCT [0] << 2) | FKEY_TAB [ch.FNUM [0] >> 7];
			break;

		case 0xA4:
			break;

		case 0xA8:
			if ( Adr < 0x100 )
			{
				num++;

				{
	                         int od = YM2612.REG[(bool)(Adr & 0x100)][(Adr & 0xFF) | 0x04];

                                 YM2612.CHANNEL [2].FNUM [num] = (YM2612.CHANNEL [2].FNUM [num] & 0x0FF) + ((od & 0x07) << 8);
                                 YM2612.CHANNEL [2].FOCT [num] = (od & 0x38) >> 3;
				}

				YM2612.CHANNEL [2].FNUM [num] = (YM2612.CHANNEL [2].FNUM [num] & 0x700) + data;
				YM2612.CHANNEL [2].KC [num] = (YM2612.CHANNEL [2].FOCT [num] << 2) |
						FKEY_TAB [YM2612.CHANNEL [2].FNUM [num] >> 7];
			}
			break;

		case 0xAC:
			break;

		case 0xB0:
			if ( ch.ALGO != (data & 7) )
			{
				ch.ALGO = data & 7;
				ch.SLOT[S0].eg.InstrParamChanged();
				ch.SLOT[S1].eg.InstrParamChanged();
				ch.SLOT[S2].eg.InstrParamChanged();
				ch.SLOT[S3].eg.InstrParamChanged();
			}

			ch.FB = 9 - ((data >> 3) & 7);                              // Real thing ?

//          if (ch.FB = ((data >> 3) & 7)) ch.FB = 9 - ch.FB;       // Thunder force 4 (music stage 8), Gynoug, Aladdin bug sound...
//          else ch.FB = 31;
			break;

		case 0xB4: {
			ch.LEFT = 0 - ((data >> 7) & 1);
			ch.RIGHT = 0 - ((data >> 6) & 1);
			
			ch.AMS = LFO_AMS_TAB [(data >> 4) & 3];
			ch.FMS = data & 7;
			
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
			//printf("%02x\n", data);
			if (data & 8) // LFO enable
			{
				// Cool Spot music 1, LFO modified severals time which
				// distord the sound, have to check that on a real genesis...

				if(YM2612.LFOinc == 0)
				 YM2612.LFOcnt = 0;

				YM2612.LFOinc = g.LFO_INC_TAB [data & 7];
			}
			else
			{
				YM2612.LFOinc = 0;				
			}
			//printf("%d\n", g.LFOcnt);
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

			ch.SLOT[S0].eg.SetKONOFF((bool)(data & 0x10));
			ch.SLOT[S1].eg.SetKONOFF((bool)(data & 0x20));
			ch.SLOT[S2].eg.SetKONOFF((bool)(data & 0x40));
			ch.SLOT[S3].eg.SetKONOFF((bool)(data & 0x80));
			break;
		}
		case 0x2A:
			YM2612.DACdata = ((int)data - 0x80) << 5;
			break;

		case 0x2B:
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
	double sample_rate = 53267;

	// Tableau TL :
	// [0     -  4095] = +output  [4095  - ...] = +output overflow (fill with 0)
	// [12288 - 16383] = -output  [16384 - ...] = -output overflow (fill with 0)

	for(i = 0; i < TL_LENGHT; i++)
	{
		double x = MAX_OUT;                         // Max output

		x /= pow(2.0, 16.0 * i / ENV_LENGHT);

		if(i >= 4096)
		 x = 0;

		//printf("%d: %f, %f\n", i, x, 20.0 * log10(x / MAX_OUT));
		//printf("%d\n", MAX_OUT);

		g.TL_TAB [i] = (int) x;
		g.TL_TAB [TL_LENGHT + i] = -g.TL_TAB [i];
	}

	// Tableau SIN :
	// g.SIN_TAB [x] [y] = sin(x) * y; 
	// x = phase and y = volume

	assert(SIN_LENGHT == 1024);
	for(int x = 0; x < 256; x++)
	{
	 double y = round(-log(sin((x+0.5)*M_PI/256/2))/log(2)*256);

 	 g.SIN_TAB [x] = g.SIN_TAB [(SIN_LENGHT / 2) - x - 1] = y;
 	 g.SIN_TAB [(SIN_LENGHT / 2) + x] = g.SIN_TAB [SIN_LENGHT - x - 1] = TL_LENGHT + y;
	}

	// Tableau LFO (LFO wav) :
	assert(LFO_LENGHT == 1024);
	for(i = 0; i < LFO_LENGHT; i++)
	{
		double x = sin(PI + (PI / 2) + 2.0 * PI * (double) (i) / (double) (LFO_LENGHT));    // Sinus
		x += 1.0;
		x /= 2.0;                   // positive only
		x *= 11.8 / ENV_STEP;       // ajusted to MAX enveloppe modulation

		g.LFO_ENV_TAB [i] = (int) x;
		//printf("%d %d\n", i, (int)x);
	}

	// Tableau Detune

	for(i = 0; i < 4; i++)
	{
		for (unsigned int kc = 0; kc < 32; kc++)
		{
			unsigned int y = DT_DEF_TAB[i][kc];

			g.DT_TAB [i + 0][kc] = (int)  y;
			g.DT_TAB [i + 4][kc] = (int) -y;
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


Ym2612_Emu::Ym2612_Emu()
{
 impl = new Ym2612_Impl();
 impl->mute_mask = 0;

 memset( &impl->YM2612, 0, sizeof impl->YM2612 );	// fixme, don't memset() a non-POD type(OpEG)
 impl->set_rate();
}

Ym2612_Emu::~Ym2612_Emu()
{
 delete impl;
}

inline void Ym2612_Impl::write0( int opn_addr, int data )
{
	assert( (unsigned) data <= 0xFF );
	
	if ( opn_addr < 0x30 )
	{
		YM2612.REG [0] [opn_addr] = data;
		YM_SET( opn_addr, data );
	}
	else
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
	
	if ( opn_addr >= 0x30 )
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
	YM2612.LFOcnt = 0;
	YM2612.TimerA = 0;
	YM2612.TimerAL = 0;
	YM2612.TimerAcnt = 0;
	YM2612.TimerB = 0;
	YM2612.TimerBL = 0;
	YM2612.TimerBcnt = 0;
	YM2612.DAC = 0;
	YM2612.DACdata = 0;
	YM2612.Status = 0;

	YM2612.EGDivCounter = 0;
	YM2612.EGCycleCounter = 0;

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

			ch.SLOT [j].eg.Reset();
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

template<int algo>
struct ym2612_update_chan {
	static void func( state_t& st, tables_t&, channel_t&, Ym2612_Emu::sample_t*);
};

typedef void (*ym2612_update_chan_t)( state_t& st, tables_t&, channel_t&, Ym2612_Emu::sample_t*);

//void MDFN_DispMessage(const char *format, ...) throw();

template<int algo>
void ym2612_update_chan<algo>::func( state_t& st, tables_t& g, channel_t& ch,
		Ym2612_Emu::sample_t* buf )
{
	// algo is a compile-time constant, so all conditions based on it are resolved
	// during compilation
	int CH_S0_OUT_1 = ch.S0_OUT [1];

	int in0 = ch.SLOT [S0].Fcnt;
	int in1 = ch.SLOT [S1].Fcnt;
	int in2 = ch.SLOT [S2].Fcnt;
	int in3 = ch.SLOT [S3].Fcnt;

//	MDFN_DispMessage("%08x", in2);
	{
		// envelope
		int const env_LFO = g.LFO_ENV_TAB [(st.LFOcnt >> LFO_LBITS) & LFO_MASK];

	#define CALC_EN( x ) \
		int temp##x = (ch.SLOT [S##x].eg.GetOutAttenuation() << 2) + (env_LFO >> ch.SLOT [S##x].AMS);  \
		int en##x = (temp##x < 0x1000) ? temp##x : 0xFFF;	\
		
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
				
		int t0 = buf [0] + (CH_OUTd & ch.LEFT);
		int t1 = buf [1] + (CH_OUTd & ch.RIGHT);
		
		ch.S0_OUT [0] = CH_S0_OUT_0;
		buf [0] = t0;
		buf [1] = t1;
		buf += 2;
	}
	
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

void Ym2612_Impl::run_timer(void)
{
		YM2612.CHANNEL[2].SLOT[0].eg.SetCSMKONOFF(false);
		YM2612.CHANNEL[2].SLOT[1].eg.SetCSMKONOFF(false);
		YM2612.CHANNEL[2].SLOT[2].eg.SetCSMKONOFF(false);
		YM2612.CHANNEL[2].SLOT[3].eg.SetCSMKONOFF(false);

		if (YM2612.Mode & 1)                            // Timer A ON ?
		{
		 YM2612.TimerAcnt--;

		 if(YM2612.TimerAcnt <= 0)
		 {
		  // timer a overflow		
		  YM2612.Status |= (YM2612.Mode & 0x04) >> 2;
		  YM2612.TimerAcnt += YM2612.TimerAL;

		  if((YM2612.Mode & 0xC0) == 0x80)
		  {
			YM2612.CHANNEL[2].SLOT[0].eg.SetCSMKONOFF(true);
			YM2612.CHANNEL[2].SLOT[1].eg.SetCSMKONOFF(true);
			YM2612.CHANNEL[2].SLOT[2].eg.SetCSMKONOFF(true);
			YM2612.CHANNEL[2].SLOT[3].eg.SetCSMKONOFF(true);
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
}
#include "../../math_ops.h"
void Ym2612_Impl::run( Ym2612_Emu::sample_t* out )
{
	if ( YM2612.Mode & 3 )
		run_timer();

	for(int i = 0; i < channel_count; i++)
	{
	 Ym2612_Emu::sample_t tmp_out[2] = { 0, 0 };

	 UPDATE_CHAN[YM2612.CHANNEL [i].ALGO](YM2612, g, YM2612.CHANNEL [i], tmp_out);

	 if(i == 5 && YM2612.DAC)
	 {
	  tmp_out[0] = YM2612.DACdata & YM2612.CHANNEL[5].LEFT;
	  tmp_out[1] = YM2612.DACdata & YM2612.CHANNEL[5].RIGHT;
	 }

	 if(mute_mask & (1 << i))
	 {
	  tmp_out[0] = tmp_out[1] = 0;
	 }

	 out[0] += tmp_out[0];
	 out[1] += tmp_out[1];
	}

	unsigned lfo_pm_position = ((YM2612.LFOcnt >> LFO_LBITS) & LFO_MASK) >> (LFO_HBITS - 5);
	unsigned lfo_pm_tabposition = (lfo_pm_position & 0x8) ? (0x7 - (lfo_pm_position & 0x7)) : (lfo_pm_position & 0x7);

//	printf("%d %d\n", lfo_pm_position, lfo_pm_tabposition);

	for ( int chi = 0; chi < channel_count; chi++ )
	{
		channel_t& ch = YM2612.CHANNEL [chi];

		int i2 = 0;
		if ( chi == 2 && (YM2612.Mode & 0x40) )
			i2 = 2;

		for ( int i = 0; i < 4; i++ )
		{
			// static int seq [4] = { 2, 1, 3, 0 };
			// if ( i2 ) i2 = seq [i];
			slot_t& sl = ch.SLOT [i];
			int lfo_fnm_delta = 0;

			if(ch.FNUM[i2])
			{
			 int amp = LFO_PhaseMod_Table[ch.FMS][lfo_pm_tabposition];

			 lfo_fnm_delta = (amp << uilog2(ch.FNUM[i2])) >> 9;

			 if(lfo_pm_position & 0x10)
			  lfo_fnm_delta = -lfo_fnm_delta;

			 //if(lfo_fnm_delta)
			 // printf("%d %d: %d\n", chi, i, lfo_fnm_delta);
			}

			int fnm = (ch.FNUM[i2] + lfo_fnm_delta) & 0x7FF;
			int finc_tmp = (fnm << ch.FOCT [i2]) >> 1;
			int finc = ((((finc_tmp + sl.DT[ch.KC[i2]]) & 0x1FFFF) * sl.MUL) >> 1) & 0xFFFFF;
			int ksr = ch.KC [i2] >> sl.KSR_S;   // keycode attenuation

			if(sl.eg.Run(YM2612.EGDivCounter == 0, YM2612.EGCycleCounter, ksr))
			 sl.Fcnt += finc;
			else
			 sl.Fcnt = 0;
			
			if ( i2 )
				i2 = (i2 ^ 2) ^ (i2 >> 1);
		}
	}

	//printf("%d %d\n", YM2612.EGDivCounter, YM2612.EGCycleCounter);
	YM2612.EGDivCounter++;
	if(YM2612.EGDivCounter == 3)
	{
	 YM2612.EGDivCounter = 0;
	 YM2612.EGCycleCounter++;
	}

	YM2612.LFOcnt += YM2612.LFOinc;
}

void Ym2612_Emu::run( sample_t* out ) { impl->run( out ); }


void Ym2612_Emu::serialize(MDFN::LEPacker &slizer, bool load)
{
 state_t *s = &impl->YM2612;
 tables_t *g = &impl->g;
 const int cur_version = 0;
 int version_saved = cur_version;

 slizer.set_read_mode(load);

 slizer ^ version_saved;

 if(version_saved != cur_version)
 {
  // ERROR  
 }

 for(int r0 = 0; r0 < 2; r0++)
 {
  for(int r1 = 0; r1 < 0x100; r1++)
  {
   slizer ^ s->REG[r0][r1];
  }
 }

 slizer ^ s->Status;
 slizer ^ s->TimerA;
 slizer ^ s->TimerAL;
 slizer ^ s->TimerAcnt;
 slizer ^ s->TimerB;
 slizer ^ s->TimerBL;
 slizer ^ s->TimerBcnt;
 slizer ^ s->Mode;
 slizer ^ s->DAC;
 slizer ^ s->DACdata;
 slizer ^ s->EGCycleCounter;
 slizer ^ s->EGDivCounter;

 slizer ^ s->LFOinc;
 slizer ^ s->LFOcnt;


 for(int ch = 0; ch < channel_count; ch++)
 {
  channel_t *c = &s->CHANNEL[ch];

  for(int i = 0; i < 4; i++)
  {
   slizer ^ c->S0_OUT[i];
  }

  slizer ^ c->LEFT;
  slizer ^ c->RIGHT;
  slizer ^ c->ALGO;
  slizer ^ c->FB;
  slizer ^ c->FMS;
  slizer ^ c->AMS;

  if(load)
  {
   c->ALGO &= 0x07;
   c->FMS &= 0x07;
  }

  for(int i = 0; i < 4; i++)
  {
   slizer ^ c->FNUM[i];
  }

  for(int i = 0; i < 4; i++)
  {
   slizer ^ c->FOCT[i];
  }
  for(int i = 0; i < 4; i++)
  {
   slizer ^ c->KC[i];

   if(load)
   {
    c->KC[i] &= 0x1F;
   }
  }

  for(int slot = 0; slot < 4; slot++)
  {
   slot_t *sl = &c->SLOT[slot];
   unsigned int tun = (unsigned int)(sl->DT - &g->DT_TAB[0][0]);

   slizer ^ tun;

   if(load)
   {
    tun &= 0x07;
    sl->DT = &g->DT_TAB[tun][0];
   }

   slizer ^ sl->MUL;
   slizer ^ sl->KSR_S;

   slizer ^ sl->Fcnt;

   slizer ^ sl->INd;
   slizer ^ sl->AMS;
   slizer ^ sl->AMSon;

   sl->eg.serialize(slizer, load);
  }
 }
}



#if 0

#define SLOT_STATEREG(ch,sln)							\
			SFVARN(s->CHANNEL[ch].SLOT[sln].DT, "DT"),		\
			SFVARN(s->CHANNEL[ch].SLOT[sln].MUL, "MUL"),		\
			SFVARN(s->CHANNEL[ch].SLOT[sln].KSR_S, "KSR_S"),	\
			SFVARN(s->CHANNEL[ch].SLOT[sln].Fcnt, "Fcnt"),		\
			SFVARN(s->CHANNEL[ch].SLOT[sln].INd, "INd"),		\
			SFVARN(s->CHANNEL[ch].SLOT[sln].AMS, "AMS"),		\
			SFVARN(s->CHANNEL[ch].SLOT[sln].AMSon, "AMSon"),

#define CH_STATEREG(ch)	SFARRAY32N(s->CHANNEL[ch].S0_OUT, 4, "S0_OUT"),		\
			SFVARN(s->CHANNEL[ch].LEFT, "LEFT"),			\
			SFVARN(s->CHANNEL[ch].RIGHT, "RIGHT"),			\
			SFVARN(s->CHANNEL[ch].ALGO, "ALGO"),			\
			SFVARN(s->CHANNEL[ch].FB, "FB"),			\
			SFVARN(s->CHANNEL[ch].FMS, "FMS"),			\
			SFVARN(s->CHANNEL[ch].AMS, "AMS"),			\
			SFARRAY32N(s->CHANNEL[ch].FNUM, 4, "FNUM"),		\
			SFARRAY32N(s->CHANNEL[ch].FOCT, 4, "FOCT"),		\
			SFARRAY32N(s->CHANNEL[ch].KC, 4, "KC"),			\
			SLOT_STATEREG(ch,0)					\
			SLOT_STATEREG(ch,1)					\
			SLOT_STATEREG(ch,2)					\
			SLOT_STATEREG(ch,3)

// EG here

#define STATEREG(n)	SFVARN(s->n, #n),

int Ym2612_Emu::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 state_t *s = &impl->YM2612;
 tables_t *g = &impl->g;

 SFORMAT StateRegs[] = 
 {
  STATEREG(Status)
  STATEREG(TimerA)
  STATEREG(TimerAL)
  STATEREG(TimerAcnt)
  STATEREG(TimerB)
  STATEREG(TimerBL)
  STATEREG(TimerBcnt)
  STATEREG(Mode)
  STATEREG(DAC)
  STATEREG(DACdata)
  STATEREG(EGCycleCounter)
  STATEREG(EGDivCounter)

  CH_STATEREG(0)
  CH_STATEREG(1)
  CH_STATEREG(2)
  CH_STATEREG(3)
  CH_STATEREG(4)
  CH_STATEREG(5)

  SFARRAY32N(&s->REG[0][0], sizeof(s->REG) / sizeof(s->REG[0][0]), "REGS"),
  SFEND
 };

}

#undef SLOT_STATEREG
#undef CH_STATEREG
#undef STATEREG

#endif
