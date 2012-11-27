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


#ifndef AUDIO_SOFTSYNTH_OPL_MAME_H
#define AUDIO_SOFTSYNTH_OPL_MAME_H

#include "common/scummsys.h"
#include "common/random.h"

#include "audio/fmopl.h"

namespace OPL {
namespace MAME {

enum {
	FMOPL_ENV_BITS_HQ = 16,
	FMOPL_ENV_BITS_MQ = 8,
	FMOPL_ENV_BITS_LQ = 8,
	FMOPL_EG_ENT_HQ = 4096,
	FMOPL_EG_ENT_MQ = 1024,
	FMOPL_EG_ENT_LQ = 128
};


typedef void (*OPL_TIMERHANDLER)(int channel,double interval_Sec);
typedef void (*OPL_IRQHANDLER)(int param,int irq);
typedef void (*OPL_UPDATEHANDLER)(int param,int min_interval_us);

#define OPL_TYPE_WAVESEL   0x01  /* waveform select    */

/* Saving is necessary for member of the 'R' mark for suspend/resume */
/* ---------- OPL one of slot  ---------- */
typedef struct fm_opl_slot {
	int TL;		/* total level     :TL << 8				*/
	int TLL;	/* adjusted now TL						*/
	uint8 KSR;	/* key scale rate  :(shift down bit)	*/
	int *AR;	/* attack rate     :&AR_TABLE[AR<<2]	*/
	int *DR;	/* decay rate      :&DR_TABLE[DR<<2]	*/
	int SL;		/* sustain level   :SL_TABLE[SL]		*/
	int *RR;	/* release rate    :&DR_TABLE[RR<<2]	*/
	uint8 ksl;	/* keyscale level  :(shift down bits)	*/
	uint8 ksr;	/* key scale rate  :kcode>>KSR			*/
	uint mul;	/* multiple        :ML_TABLE[ML]		*/
	uint Cnt;	/* frequency count						*/
	uint Incr;	/* frequency step						*/

	/* envelope generator state */
	uint8 eg_typ;/* envelope type flag					*/
	uint8 evm;	/* envelope phase						*/
	int evc;	/* envelope counter						*/
	int eve;	/* envelope counter end point			*/
	int evs;	/* envelope counter step				*/
	int evsa;	/* envelope step for AR :AR[ksr]		*/
	int evsd;	/* envelope step for DR :DR[ksr]		*/
	int evsr;	/* envelope step for RR :RR[ksr]		*/

	/* LFO */
	uint8 ams;		/* ams flag                            */
	uint8 vib;		/* vibrate flag                        */
	/* wave selector */
	int **wavetable;
} OPL_SLOT;

/* ---------- OPL one of channel  ---------- */
typedef struct fm_opl_channel {
	OPL_SLOT SLOT[2];
	uint8 CON;			/* connection type					*/
	uint8 FB;			/* feed back       :(shift down bit)*/
	int *connect1;		/* slot1 output pointer				*/
	int *connect2;		/* slot2 output pointer				*/
	int op1_out[2];		/* slot1 output for selfeedback		*/

	/* phase generator state */
	uint block_fnum;	/* block+fnum						*/
	uint8 kcode;		/* key code        : KeyScaleCode	*/
	uint fc;			/* Freq. Increment base				*/
	uint ksl_base;		/* KeyScaleLevel Base step			*/
	uint8 keyon;		/* key on/off flag					*/
} OPL_CH;

/* OPL state */
typedef struct fm_opl_f {
	uint8 type;			/* chip type                         */
	int clock;			/* master clock  (Hz)                */
	int rate;			/* sampling rate (Hz)                */
	double freqbase;	/* frequency base                    */
	double TimerBase;	/* Timer base time (==sampling time) */
	uint8 address;		/* address register                  */
	uint8 status;		/* status flag                       */
	uint8 statusmask;	/* status mask                       */
	uint mode;			/* Reg.08 : CSM , notesel,etc.       */

	/* Timer */
	int T[2];			/* timer counter                     */
	uint8 st[2];		/* timer enable                      */

	/* FM channel slots */
	OPL_CH *P_CH;		/* pointer of CH                     */
	int	max_ch;			/* maximum channel                   */

	/* Rythm sention */
	uint8 rythm;		/* Rythm mode , key flag */

	/* time tables */
	int AR_TABLE[76];	/* atttack rate tables				*/
	int DR_TABLE[76];	/* decay rate tables				*/
	uint FN_TABLE[1024];/* fnumber -> increment counter		*/

	/* LFO */
	int *ams_table;
	int *vib_table;
	int amsCnt;
	int amsIncr;
	int vibCnt;
	int vibIncr;

	/* wave selector enable flag */
	uint8 wavesel;

	/* external event callback handler */
	OPL_TIMERHANDLER  TimerHandler;		/* TIMER handler   */
	int TimerParam;						/* TIMER parameter */
	OPL_IRQHANDLER    IRQHandler;		/* IRQ handler    */
	int IRQParam;						/* IRQ parameter  */
	OPL_UPDATEHANDLER UpdateHandler;	/* stream update handler   */
	int UpdateParam;					/* stream update parameter */

	Common::RandomSource *rnd;
} FM_OPL;

/* ---------- Generic interface section ---------- */
#define OPL_TYPE_YM3526 (0)
#define OPL_TYPE_YM3812 (OPL_TYPE_WAVESEL)

void OPLBuildTables(int ENV_BITS_PARAM, int EG_ENT_PARAM);

FM_OPL *OPLCreate(int type, int clock, int rate);
void OPLDestroy(FM_OPL *OPL);
void OPLSetTimerHandler(FM_OPL *OPL, OPL_TIMERHANDLER TimerHandler, int channelOffset);
void OPLSetIRQHandler(FM_OPL *OPL, OPL_IRQHANDLER IRQHandler, int param);
void OPLSetUpdateHandler(FM_OPL *OPL, OPL_UPDATEHANDLER UpdateHandler, int param);

void OPLResetChip(FM_OPL *OPL);
int OPLWrite(FM_OPL *OPL, int a, int v);
unsigned char OPLRead(FM_OPL *OPL, int a);
int OPLTimerOver(FM_OPL *OPL, int c);
void OPLWriteReg(FM_OPL *OPL, int r, int v);
void YM3812UpdateOne(FM_OPL *OPL, int16 *buffer, int length);

// Factory method
FM_OPL *makeAdLibOPL(int rate);

// OPL API implementation
class OPL : public ::OPL::OPL {
private:
	FM_OPL *_opl;
public:
	OPL() : _opl(0) {}
	~OPL();

	bool init(int rate);
	void reset();

	void write(int a, int v);
	byte read(int a);

	void writeReg(int r, int v);

	void readBuffer(int16 *buffer, int length);
	bool isStereo() const { return false; }
};

} // End of namespace MAME
} // End of namespace OPL

#endif
