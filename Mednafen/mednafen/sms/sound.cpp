/*
    Copyright (C) 1998-2004  Charles MacDonald

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "shared.h"
#include <blip/Blip_Buffer.h>
#include <blip/Stereo_Buffer.h>

#include "sms_apu/Sms_Apu.h"
#include "ym2413/emu2413.h"

namespace MDFN_IEN_SMS
{

static Sms_Apu apu;
static Stereo_Buffer zebuf;
typedef Blip_Synth<blip_good_quality, 8192> FMSynth_t;
static FMSynth_t fmsynth;
static EMU2413 *FMThing = NULL;
static uint32 SoundClock;

/*--------------------------------------------------------------------------*/
/* Sound chip access handlers                                               */
/*--------------------------------------------------------------------------*/

void psg_stereo_w(int data)
{
 apu.write_ggstereo(sms.timestamp, data);
}

void psg_write(int data)
{
 //printf("Write: %d, %02x\n", sms.timestamp, data);
 apu.write_data(sms.timestamp, data);
}

/*--------------------------------------------------------------------------*/
/* Mark III FM Unit / Master System (J) built-in FM handlers                */
/*--------------------------------------------------------------------------*/

int fmunit_detect_r(void)
{
	//printf("Detect_r: %02x\n", sms.fm_detect);
    return sms.fm_detect;
}

void fmunit_detect_w(int data)
{
	//printf("Detect_w: %02x\n", data);
    sms.fm_detect = data;
}

static int16 fm_last_value;
static int32 fm_last_timestamp;
static int32 fm_div;

static void UpdateFM(void)
{
 int32 cycles = sms.timestamp - fm_last_timestamp;
 
 fm_div -= cycles;
 while(fm_div <= 0)
 {
  int32 new_value;

  new_value = EMU2413_calc(FMThing) >> 1;

  if(new_value > 32767) 
   new_value = 32767;

  if(new_value < -32768)
   new_value = -32768;

  fmsynth.offset(sms.timestamp + fm_div, new_value - fm_last_value, zebuf.left());
  fmsynth.offset(sms.timestamp + fm_div, new_value - fm_last_value, zebuf.right());
  fm_last_value = new_value;
  fm_div += 72;
 }

 fm_last_timestamp = sms.timestamp;
}

void fmunit_write(int offset, int data)
{
 //printf("FM Write: %d %d\n", offset, data);
 if(FMThing)
 {
  UpdateFM();
  EMU2413_writeIO(FMThing, offset, data);
 }
}

void SMS_SoundReset(void)
{
 apu.reset();
 if(FMThing)
  EMU2413_reset(FMThing);
}


int32 SMS_SoundFlush(int16 *SoundBuf, int32 MaxSoundFrames)
{
	int32 FrameCount = 0;

	if(FMThing)
 	 UpdateFM();

        apu.end_frame(sms.timestamp);

        zebuf.end_frame(sms.timestamp);

        if(SoundBuf)
         FrameCount = zebuf.read_samples(SoundBuf, MaxSoundFrames * 2) / 2;
	else
	 zebuf.clear();

	fm_last_timestamp = 0;

        return(FrameCount);
}

static void RedoVolume(void)
{
 apu.output(zebuf.center(), zebuf.left(), zebuf.right());
 apu.volume(0.50);
 fmsynth.volume(0.20);
}

void SMS_SoundInit(uint32 clock, bool WantFM)
{
 SoundClock = clock;

 SMS_SetSoundRate(0);
 zebuf.clock_rate((long)(SoundClock));

 RedoVolume();
 zebuf.bass_freq(20);

 if(WantFM)
  FMThing = EMU2413_new(SoundClock);
}

void SMS_SoundClose(void)
{
 if(FMThing)
 {
  EMU2413_delete(FMThing);
  FMThing = NULL;
 } 

}

bool SMS_SetSoundRate(uint32 rate)
{
 zebuf.set_sample_rate(rate ? rate : 44100, 60);

 return(TRUE);
}


int SMS_SoundStateAction(StateMem *sm, int load, int data_only)
{
 Sms_ApuState sn_state;
 int ret = 1;

 memset(&sn_state, 0, sizeof(Sms_ApuState));

 if(!load)
 {
  apu.save_state(&sn_state);
 }

 SFORMAT StateRegs[] =
 {
  SFARRAY32N(sn_state.volume, 4, "Volume"),
  SFARRAY32N(sn_state.sq_period, 3, "SQPeriod"),
  SFARRAY32N(sn_state.sq_phase, 3, "SQPhase"),
  SFVARN(sn_state.noise_period, "NPeriod"),
  SFVARN(sn_state.noise_shifter, "NShifter"),
  SFVARN(sn_state.noise_feedback, "NFeedback"),
  SFVARN(sn_state.latch, "Latch"),
  SFVARN(sn_state.ggstereo, "GGStereo"),
  SFEND
 };

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "PSG"))
  ret = 0;
 else if(load)
 {
  apu.load_state(&sn_state);
 }
 return(ret);
}

}
