/*
   Timer and PCM YMF278B.  The FM will be shared with the ymf262, eventually.

   This chip roughly splits the difference between the Sega 315-5560 MultiPCM
   (Multi32, Model 1/2) and YMF 292-F SCSP (later Model 2, STV, Saturn, Model 3).

   By R. Belmont and O. Galibert.

   Copyright (c) 2002-2003 R. Belmont and O. Galibert.

   This software is dual-licensed: it may be used in MAME and properly licensed
   MAME derivatives under the terms of the MAME license.  For use outside of
   MAME and properly licensed derivatives, it is available under the
   terms of the GNU Lesser General Public License (LGPL), version 2.1.
   You may read the LGPL at http://www.gnu.org/licenses/lgpl.html

   Changelog:
   Sep. 8, 2002 - fixed ymf278b_compute_rate when OCT is negative (RB)
   Dec. 11, 2002 - added ability to set non-standard clock rates (RB)
                   fixed envelope target for release (fixes missing
		   instruments in hotdebut).
                   Thanks to Team Japump! for MP3s from a real PCB.
		   fixed crash if MAME is run with no sound.
   June 4, 2003 -  Changed to dual-license with LGPL for use in OpenMSX.
                   OpenMSX contributed a bugfix where looped samples were
 		    not being addressed properly, causing pitch fluctuation.

   ---

   Jan     2005 - All changes needed for FB Alpha are marked as !!! FBA
				  and contained within #if blocks

*/

#include <math.h>
#include "driver.h"

#ifdef FBA															/* !!! FBA */
#include "ymf278b.h"
typedef struct { int sample_rate; } MACHINE;
MACHINE machine;
MACHINE* Machine = &machine;
#else																/* !!! FBA */
#include "cpuintrf.h"
#include "ymf278b.h"
#endif																/* !!! FBA */

#undef VERBOSE

typedef struct
{
	INT16 wave;		/* wavetable number */
	INT16 FN;		/* f-number */
	INT8 OCT;		/* octave */
	INT8 PRVB;		/* pseudo-reverb */
	INT8 LD;		/* level direct */
	INT8 TL;		/* total level */
	INT8 pan;		/* panpot */
	INT8 lfo;		/* LFO */
	INT8 vib;		/* vibrato */
	INT8 AM;		/* AM level */

	INT8 AR;
	INT8 D1R;
	INT8 DL;
	INT8 D2R;
	INT8 RC;   		/* rate correction */
	INT8 RR;

	UINT32 step;	/* fixed-point frequency step */
	UINT32 stepptr;	/* fixed-point pointer into the sample */

	INT8 active;		/* slot keyed on */
	INT8 bits;		/* width of the samples */
	UINT32 startaddr;
	UINT32 loopaddr;
	UINT32 endaddr;

	int env_step;
	UINT32 env_vol;
	UINT32 env_vol_step;
	UINT32 env_vol_lim;
} YMF278BSlot;

typedef struct
{
	YMF278BSlot slots[24];
	INT8 lsitest0;
	INT8 lsitest1;
	INT8 wavetblhdr;
	INT8 memmode;
	INT32 memadr;

	INT32 fm_l, fm_r;
	INT32 pcm_l, pcm_r;

	UINT8 timer_a_count, timer_b_count, enable, current_irq;
#ifndef FBA															/* !!! FBA */
	void *timer_a, *timer_b;
#endif																/* !!! FBA */
	int irq_line;

	UINT8 port_A, port_B, port_C;
	void (*irq_callback)(int, int);
#ifdef FBA															/* !!! FBA */
	void (*timer_callback)(int, int, double);
#endif																/* !!! FBA */

	const UINT8 *rom;
	float clock_ratio;
} YMF278BChip;

static YMF278BChip YMF278B[MAX_YMF278B];
static INT32 volume[256*4];				// precalculated attenuation values with some marging for enveloppe and pan levels
static int pan_left[16], pan_right[16];	// pan volume offsets
static INT32 mix_level[8];

static int ymf278b_compute_rate(YMF278BSlot *slot, int val)
{
	int res, oct;

	if(val == 0)
		return 0;
	if(val == 15)
		return 63;
	if(slot->RC != 15)
	{
		oct = slot->OCT;
		if (oct & 8) oct |= -8;

		res = (oct+slot->RC)*2 + (slot->FN & 0x200 ? 1 : 0) + val*4;
	}
	else
		res = val * 4;
	if(res < 0)
		res = 0;
	else if(res > 63)
		res = 63;
	return res;
}

static UINT32 ymf278_compute_decay_rate(int num)
{
	int samples;

	if (num <= 3)
		samples = 0;
	else if (num >= 60)
		samples = 15 << 4;
	else
	{
		samples = (15 << (21 - num / 4)) / (4 + num % 4);
		if (num % 4 && num / 4 <= 11)
			samples += 2;
		else if (num == 51)
			samples += 2;
	}

	return ((UINT64)samples * Machine->sample_rate) / 44100;
}

static void ymf278b_envelope_next(YMF278BSlot *slot, float clock_ratio)
{
	if(slot->env_step == 0)
	{
		// Attack
		slot->env_vol = (256U << 23) - 1;
		slot->env_vol_lim = 256U<<23;
#ifdef VERBOSE
		logerror("YMF278B: Skipping attack (rate = %d)\n", slot->AR);
#endif
		slot->env_step++;
	}
	if(slot->env_step == 1)
	{
		// Decay 1
		slot->env_vol = 0;
		slot->env_step++;
		if(slot->DL)
		{
			int rate = ymf278b_compute_rate(slot, slot->D1R);
#ifdef VERBOSE
			logerror("YMF278B: Decay step 1, dl=%d, val = %d rate = %d, delay = %g\n", slot->DL, slot->D1R, rate, ymf278_compute_decay_rate(rate)*1000.0*clock_ratio/Machine->sample_rate);
#endif
			if(rate<4)
				slot->env_vol_step = 0;
			else
				slot->env_vol_step = ((slot->DL*8)<<23) / (ymf278_compute_decay_rate(rate) * clock_ratio);
			slot->env_vol_lim = (slot->DL*8)<<23;
			return;
		}
	}
	if(slot->env_step == 2)
	{
		// Decay 2
		int rate = ymf278b_compute_rate(slot, slot->D2R);
#ifdef VERBOSE
		logerror("YMF278B: Decay step 2, val = %d, rate = %d, delay = %g, current vol = %d\n", slot->D2R, rate, ymf278_compute_decay_rate(rate)*1000.0*clock_ratio/Machine->sample_rate, slot->env_vol >> 23);
#endif
		if(rate<4)
			slot->env_vol_step = 0;
		else
			slot->env_vol_step = ((256U-slot->DL*8)<<23) / (ymf278_compute_decay_rate(rate) * clock_ratio);
		slot->env_vol_lim = 256U<<23;
		slot->env_step++;
		return;
	}
	if(slot->env_step == 3)
	{
		// Decay 2 reached -96dB
#ifdef VERBOSE
		logerror("YMF278B: Voice cleared because of decay 2\n");
#endif
		slot->env_vol = 256U<<23;
		slot->env_vol_step = 0;
		slot->env_vol_lim = 0;
		slot->active = 0;
		return;
	}
	if(slot->env_step == 4)
	{
		// Release
		int rate = ymf278b_compute_rate(slot, slot->RR);
#ifdef VERBOSE
		logerror("YMF278B: Release, val = %d, rate = %d, delay = %g\n", slot->RR, rate, ymf278_compute_decay_rate(rate)*1000.0*clock_ratio/Machine->sample_rate);
#endif
		if(rate<4)
			slot->env_vol_step = 0;
		else
			slot->env_vol_step = ((256U<<23)-slot->env_vol) / (ymf278_compute_decay_rate(rate) * clock_ratio);
		slot->env_vol_lim = 256U<<23;
		slot->env_step++;
		return;
	}
	if(slot->env_step == 5)
	{
		// Release reached -96dB
#ifdef VERBOSE
		logerror("YMF278B: Release ends\n");
#endif
		slot->env_vol = 256U<<23;
		slot->env_vol_step = 0;
		slot->env_vol_lim = 0;
		slot->active = 0;
		return;
	}
}

void ymf278b_pcm_update(int num, INT16 **outputs, int length)
{
	int i, j;
	YMF278BSlot *slot = NULL;
	INT16 sample = 0;
	const UINT8 *rombase;
	INT32 mix[44100*2];
	INT32 *mixp;
	INT32 vl, vr;

	memset(mix, 0, sizeof(mix[0])*length*2);

	rombase = YMF278B[num].rom;

	for (i = 0; i < 24; i++)
	{
		slot = &YMF278B[num].slots[i];

		if (slot->active)
		{
			mixp = mix;

			for (j = 0; j < length; j++)
			{
				switch (slot->bits)
				{
					case 8: 	// 8 bit
						sample = rombase[slot->startaddr + (slot->stepptr>>16)]<<8;
						break;

					case 12:  	// 12 bit
						if (slot->stepptr & 1)
							sample = rombase[slot->startaddr + (slot->stepptr>>17)*3 + 2]<<8 | ((rombase[slot->startaddr + (slot->stepptr>>17)*3 + 1] << 4) & 0xf0);
						else
							sample = rombase[slot->startaddr + (slot->stepptr>>17)*3]<<8 | (rombase[slot->startaddr + (slot->stepptr>>17)*3 + 1] & 0xf0);
						break;

					case 16:  	// 16 bit
						sample = rombase[slot->startaddr + ((slot->stepptr>>16)*2)]<<8;
						sample |= rombase[slot->startaddr + ((slot->stepptr>>16)*2) + 1];
						break;
				}

				*mixp++ += (sample * volume[slot->TL+pan_left [slot->pan]+(slot->env_vol>>23)])>>17;
				*mixp++ += (sample * volume[slot->TL+pan_right[slot->pan]+(slot->env_vol>>23)])>>17;

				// update frequency
				slot->stepptr += slot->step;
				if(slot->stepptr >= slot->endaddr)
				{
					slot->stepptr = slot->stepptr - slot->endaddr + slot->loopaddr;
					// If the step is bigger than the loop, finish the sample forcibly
					if(slot->stepptr >= slot->endaddr)
					{
						slot->env_vol = 256U<<23;
						slot->env_vol_step = 0;
						slot->env_vol_lim = 0;
						slot->active = 0;
						slot->stepptr = 0;
						slot->step = 0;
					}
				}

				// update envelope
				slot->env_vol += slot->env_vol_step;
				if(((INT32)(slot->env_vol - slot->env_vol_lim)) >= 0)
			 		ymf278b_envelope_next(slot, YMF278B[num].clock_ratio);
			}
		}
	}

	mixp = mix;
	vl = mix_level[YMF278B[num].pcm_l];
	vr = mix_level[YMF278B[num].pcm_r];
	for (i = 0; i < length; i++)
	{
		outputs[0][i] = (*mixp++ * vl) >> 16;
		outputs[1][i] = (*mixp++ * vr) >> 16;
	}
}

static void ymf278b_irq_check(int num)
{
	YMF278BChip *chip = &YMF278B[num];
	int prev_line = chip->irq_line;
	chip->irq_line = chip->current_irq ? ASSERT_LINE : CLEAR_LINE;
	if(chip->irq_line != prev_line && chip->irq_callback)
		chip->irq_callback(num, chip->irq_line);
}

#ifdef FBA															/* !!! FBA */
int ymf278b_timer_over(int num, int timer)
{
	YMF278BChip *chip = &YMF278B[num];
	if(!(chip->enable & (0x20 << timer)))
	{
		chip->current_irq |= (0x20 << timer);
		ymf278b_irq_check(num);
	}

	return 0;
}
#else																/* !!! FBA */
static void ymf278b_timer_a_tick(int num)
{
	YMF278BChip *chip = &YMF278B[num];
	if(!(chip->enable & 0x40))
	{
		chip->current_irq |= 0x40;
		ymf278b_irq_check(num);
	}
}

static void ymf278b_timer_b_tick(int num)
{
	YMF278BChip *chip = &YMF278B[num];
	if(!(chip->enable & 0x20))
	{
		chip->current_irq |= 0x20;
		ymf278b_irq_check(num);
	}
}
#endif																/* !!! FBA */

static void ymf278b_timer_a_reset(int num)
{
	YMF278BChip *chip = &YMF278B[num];
	if(chip->enable & 1)
	{
		double period = (256-chip->timer_a_count) * 80.8 * chip->clock_ratio;
		chip->timer_callback(num, 0, period);
	}
	else
		chip->timer_callback(num, 0, 0);
}

static void ymf278b_timer_b_reset(int num)
{
	YMF278BChip *chip = &YMF278B[num];
	if(chip->enable & 2)
	{
		double period = (256-chip->timer_b_count) * 323.1 * chip->clock_ratio;
		chip->timer_callback(num, 1, period);
	}
	else
		chip->timer_callback(num, 1, 0);
}

static void ymf278b_A_w(int num, UINT8 reg, UINT8 data)
{
	YMF278BChip *chip = &YMF278B[num];

	if (!Machine->sample_rate) return;

	switch(reg)
	{
		case 0x02:
			chip->timer_a_count = data;
			ymf278b_timer_a_reset(num);
			break;
		case 0x03:
			chip->timer_b_count = data;
			ymf278b_timer_b_reset(num);
			break;
		case 0x04:
			if(data & 0x80)
				chip->current_irq = 0;
			else
			{
				UINT8 old_enable = chip->enable;
				chip->enable = data;
				chip->current_irq &= ~data;
				if((old_enable ^ data) & 1)
					ymf278b_timer_a_reset(num);
				if((old_enable ^ data) & 2)
					ymf278b_timer_b_reset(num);
			}
			ymf278b_irq_check(num);
			break;
		default:
			logerror("YMF278B:  Port A write %02x, %02x\n", reg, data);
	}
}

static void ymf278b_B_w(int num, UINT8 reg, UINT8 data)
{
	if (!Machine->sample_rate) return;

	logerror("YMF278B:  Port B write %02x, %02x\n", reg, data);
}

static void ymf278b_C_w(int num, UINT8 reg, UINT8 data)
{
	YMF278BChip *chip = &YMF278B[num];

	if (!Machine->sample_rate) return;

	// Handle slot registers specifically
	if (reg >= 0x08 && reg <= 0xf7)
	{
		YMF278BSlot *slot = NULL;
		int snum;
		snum = (reg-8) % 24;
		slot = &YMF278B[num].slots[snum];
		switch((reg-8) / 24)
		{
			case 0:
			{
				const UINT8 *p;

				slot->wave &= 0x100;
				slot->wave |= data;

				if(slot->wave < 384 || !chip->wavetblhdr)
					p = chip->rom + (slot->wave * 12);
				else
					p = chip->rom + chip->wavetblhdr*0x80000 + ((slot->wave - 384) * 12);

				switch (p[0]&0xc0)
				{
					case 0:
						slot->bits = 8;
						break;
					case 0x40:
						slot->bits = 12;
						break;
					case 0x80:
						slot->bits = 16;
						break;
				}

				slot->lfo = (p[7] >> 2) & 7;
				slot->vib = p[7] & 7;
				slot->AR = p[8] >> 4;
				slot->D1R = p[8] & 0xf;
				slot->DL = p[9] >> 4;
				slot->D2R = p[9] & 0xf;
				slot->RC = p[10] >> 4;
				slot->RR = p[10] & 0xf;
				slot->AM = p[11] & 7;

				slot->startaddr = (p[2] | (p[1]<<8) | ((p[0]&0x3f)<<16));
				slot->loopaddr = (p[4]<<16) | (p[3]<<24);
				slot->endaddr = (p[6]<<16) | (p[5]<<24);
				slot->endaddr -= 0x00010000U;
				slot->endaddr ^= 0xffff0000U;
				break;
			}
			case 1:
				slot->wave &= 0xff;
				slot->wave |= ((data&0x1)<<8);
				slot->FN &= 0x380;
				slot->FN |= (data>>1);
				break;
			case 2:
				slot->FN &= 0x07f;
				slot->FN |= ((data&0x07)<<7);
				slot->PRVB = ((data&0x4)>>3);
				slot->OCT = ((data&0xf0)>>4);
				break;
			case 3:
				slot->TL = (data>>1);
				slot->LD = data&0x1;
				break;
			case 4:
				slot->pan = data&0xf;
				if (data & 0x80)
				{
					unsigned int step;
					int oct;

					slot->active = 1;

					oct = slot->OCT;
					if(oct & 8)
						oct |= -8;

					slot->env_step = 0;
					slot->env_vol = 256U<<23;
					slot->env_vol_step = 0;
					slot->env_vol_lim = 256U<<23;
					slot->stepptr = 0;
					slot->step = 0;

					step = (slot->FN | 1024) << (oct + 7);
					slot->step = (UINT32) ((((INT64)step)*(44100/4)) / Machine->sample_rate * chip->clock_ratio);

					ymf278b_envelope_next(slot, chip->clock_ratio);

#ifdef VERBOSE
					logerror("YMF278B: slot %2d wave %3d lfo=%d vib=%d ar=%d d1r=%d dl=%d d2r=%d rc=%d rr=%d am=%d\n", snum, slot->wave,
							 slot->lfo, slot->vib, slot->AR, slot->D1R, slot->DL, slot->D2R, slot->RC, slot->RR, slot->AM);
					logerror("                  b=%d, start=%x, loop=%x, end=%x, oct=%d, fn=%d, step=%x\n", slot->bits, slot->startaddr, slot->loopaddr>>16, slot->endaddr>>16, oct, slot->FN, slot->step);
#endif
				}
				else
				{
#ifdef VERBOSE
					logerror("YMF278B: slot %2d off\n", snum);
#endif
					if(slot->active)
					{
						slot->env_step = 4;
						ymf278b_envelope_next(slot, chip->clock_ratio);
					}
				}
				break;
			case 5:
				slot->vib = data&0x7;
				slot->lfo = (data>>3)&0x7;
		       	break;
			case 6:
				slot->AR = data>>4;
				slot->D1R = data&0xf;
				break;
			case 7:
				slot->DL = data>>4;
				slot->D2R = data&0xf;
				break;
			case 8:
				slot->RC = data>>4;
				slot->RR = data&0xf;
				break;
			case 9:
				slot->AM = data & 0x7;
				break;
		}
	}
	else
	{
		// All non-slot registers
		switch (reg)
		{
			case 0x00:    	// TEST
			case 0x01:
				break;

			case 0x02:
				chip->wavetblhdr = (data>>2)&0x7;
				chip->memmode = data&1;
				break;

			case 0x03:
				chip->memadr &= 0xffff;
				chip->memadr |= (data<<16);
				break;

			case 0x04:
				chip->memadr &= 0xff00ff;
				chip->memadr |= (data<<8);
				break;

			case 0x05:
				chip->memadr &= 0xffff00;
				chip->memadr |= data;
				break;

			case 0x06:  // memory data (ignored, we don't support RAM)
			case 0x07:	// unused
				break;

			case 0xf8:
				chip->fm_l = data & 0x7;
				chip->fm_r = (data>>3)&0x7;
				break;

			case 0xf9:
				chip->pcm_l = data & 0x7;
				chip->pcm_r = (data>>3)&0x7;
				break;
		}
	}
}

static UINT8 ymf278b_status_port_r(int num)
{
	return YMF278B[num].current_irq | (YMF278B[num].irq_line == ASSERT_LINE ? 0x80 : 0x00);
}

// Not implemented yet
static UINT8 ymf278b_data_port_r(int num)
{
	return 0;
}

static void ymf278b_control_port_A_w(int num, UINT8 data)
{
	YMF278B[num].port_A = data;
}

static void ymf278b_data_port_A_w(int num, UINT8 data)
{
	ymf278b_A_w(num, YMF278B[num].port_A, data);
}

static void ymf278b_control_port_B_w(int num, UINT8 data)
{
	YMF278B[num].port_B = data;
}

static void ymf278b_data_port_B_w(int num, UINT8 data)
{
	ymf278b_B_w(num, YMF278B[num].port_B, data);
}

static void ymf278b_control_port_C_w(int num, UINT8 data)
{
	YMF278B[num].port_C = data;
}

static void ymf278b_data_port_C_w(int num, UINT8 data)
{
	ymf278b_C_w(num, YMF278B[num].port_C, data);
}

#ifdef FBA															/* !!! FBA */
int ymf278b_start(INT8 num, UINT8 *rom, void (*irq_cb)(int, int), void (*timer_cb)(int, int, double), int clock, int rate)
{
	int i;

	memset(&YMF278B[num], 0, sizeof(YMF278BChip));
	YMF278B[num].rom = rom;
	YMF278B[num].irq_callback = irq_cb;
	YMF278B[num].timer_callback = timer_cb;
	YMF278B[num].irq_line = CLEAR_LINE;
	YMF278B[num].clock_ratio = (float)clock / (float)YMF278B_STD_CLOCK;

	Machine->sample_rate = rate;

	// Volume table, 1 = -0.375dB, 8 = -3dB, 256 = -96dB
	for(i = 0; i < 256; i++)
		volume[i] = 65536*pow(2.0, (-0.375/6)*i);
	for(i = 256; i < 256*4; i++)
		volume[i] = 0;

	// Pan values, units are -3dB, i.e. 8.
	for(i = 0; i < 16; i++)
	{
		pan_left[i] = i < 7 ? i*8 : i < 9 ? 256 : 0;
		pan_right[i] = i < 8 ? 0 : i < 10 ? 256 : (16-i)*8;
	}

	// Mixing levels, units are -3dB, and add some marging to avoid clipping
	for(i=0; i<7; i++)
		mix_level[i] = volume[8*i+8];
	mix_level[7] = 0;

	return 0;
}
#else																/* !!! FBA */
static void ymf278b_init(INT8 num, UINT8 *rom, void (*cb)(int), int clock)
{
	memset(&YMF278B[num], 0, sizeof(YMF278BChip));
	YMF278B[num].rom = rom;
	YMF278B[num].irq_callback = cb;
	YMF278B[num].timer_a = timer_alloc(ymf278b_timer_a_tick);
	YMF278B[num].timer_b = timer_alloc(ymf278b_timer_b_tick);
	YMF278B[num].irq_line = CLEAR_LINE;
	YMF278B[num].clock_ratio = (float)clock / (float)YMF278B_STD_CLOCK;
}

int YMF278B_sh_start( const struct MachineSound *msound )
{
	char buf[2][40];
	const char *name[2];
	int  vol[2];
	struct YMF278B_interface *intf;
	int i;

	intf = msound->sound_interface;

	for(i=0; i<intf->num; i++)
	{
		sprintf(buf[0], "YMF278B %d L", i);
		sprintf(buf[1], "YMF278B %d R", i);
		name[0] = buf[0];
		name[1] = buf[1];
		vol[0]=intf->mixing_level[i] >> 16;
		vol[1]=intf->mixing_level[i] & 0xffff;
		ymf278b_init(i, memory_region(intf->region[0]), intf->irq_callback[i], intf->clock[i]);
		stream_init_multi(2, name, vol, Machine->sample_rate, i, ymf278b_pcm_update);
	}

	// Volume table, 1 = -0.375dB, 8 = -3dB, 256 = -96dB
	for(i = 0; i < 256; i++)
		volume[i] = 65536*pow(2.0, (-0.375/6)*i);
	for(i = 256; i < 256*4; i++)
		volume[i] = 0;

	// Pan values, units are -3dB, i.e. 8.
	for(i = 0; i < 16; i++)
	{
		pan_left[i] = i < 7 ? i*8 : i < 9 ? 256 : 0;
		pan_right[i] = i < 8 ? 0 : i < 10 ? 256 : (16-i)*8;
	}

	// Mixing levels, units are -3dB, and add some marging to avoid clipping
	for(i=0; i<7; i++)
		mix_level[i] = volume[8*i+8];
	mix_level[7] = 0;

	return 0;
}
#endif																/* !!! FBA */

void YMF278B_sh_stop( void )
{
}

READ8_HANDLER( YMF278B_status_port_0_r )
{
	return ymf278b_status_port_r(0);
}

READ8_HANDLER( YMF278B_data_port_0_r )
{
	return ymf278b_data_port_r(0);
}

WRITE8_HANDLER( YMF278B_control_port_0_A_w )
{
	ymf278b_control_port_A_w(0, data);
}

WRITE8_HANDLER( YMF278B_data_port_0_A_w )
{
	ymf278b_data_port_A_w(0, data);
}

WRITE8_HANDLER( YMF278B_control_port_0_B_w )
{
	ymf278b_control_port_B_w(0, data);
}

WRITE8_HANDLER( YMF278B_data_port_0_B_w )
{
	ymf278b_data_port_B_w(0, data);
}

WRITE8_HANDLER( YMF278B_control_port_0_C_w )
{
	ymf278b_control_port_C_w(0, data);
}

WRITE8_HANDLER( YMF278B_data_port_0_C_w )
{
	ymf278b_data_port_C_w(0, data);
}

#ifndef FBA															/* !!! FBA */
READ8_HANDLER( YMF278B_status_port_1_r )
{
	return ymf278b_status_port_r(1);
}

READ8_HANDLER( YMF278B_data_port_1_r )
{
	return ymf278b_data_port_r(1);
}

WRITE8_HANDLER( YMF278B_control_port_1_A_w )
{
	ymf278b_control_port_A_w(1, data);
}

WRITE8_HANDLER( YMF278B_data_port_1_A_w )
{
	ymf278b_data_port_A_w(1, data);
}

WRITE8_HANDLER( YMF278B_control_port_1_B_w )
{
	ymf278b_control_port_B_w(1, data);
}

WRITE8_HANDLER( YMF278B_data_port_1_B_w )
{
	ymf278b_data_port_B_w(1, data);
}

WRITE8_HANDLER( YMF278B_control_port_1_C_w )
{
	ymf278b_control_port_C_w(1, data);
}

WRITE8_HANDLER( YMF278B_data_port_1_C_w )
{
	ymf278b_data_port_C_w(1, data);
}
#endif																/* !!! FBA */
