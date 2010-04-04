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
#include "Sms_Apu.h"
#include <blip/Stereo_Buffer.h>
#include "emu2413.h"

static Sms_Apu apu;
static Stereo_Buffer zebuf;
typedef Blip_Synth<blip_good_quality, 8192> FMSynth_t;
static FMSynth_t fmsynth;
static bool forcemono;
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
    return sms.fm_detect;
}

void fmunit_detect_w(int data)
{
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
  int16 new_value;

  new_value = EMU2413_calc(FMThing);
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
  EMU2413_writeIO(FMThing, offset, data);
 }
}

void SMS_SoundReset(void)
{
 apu.reset();
 if(FMThing)
  EMU2413_reset(FMThing);
}


int16 *SMS_SoundFlush(int32 *length)
{
        static int16 buffer[8000];

	if(FMThing)
 	 UpdateFM();

        apu.end_frame(sms.timestamp);

        if(forcemono)
        {
         zebuf.left()->end_frame(sms.timestamp);
         *length = zebuf.left()->read_samples(buffer, 8000);
        }
        else
        {
         zebuf.end_frame(sms.timestamp);
         *length = zebuf.read_samples(buffer, 8000);
         *length /= 2;
        }

        if(!FSettings.SndRate)
        {
         *length = 0;
         return(NULL);
        }

	fm_last_timestamp = 0;

        return(buffer);
}

static void RedoVolume(void)
{
 if(forcemono)
 {
  apu.output(zebuf.center(), zebuf.left(), zebuf.left());
  apu.volume((double)FSettings.SoundVolume * 0.50 / 2 / 100);
  fmsynth.volume((double)FSettings.SoundVolume * 0.10 / 2 / 100);
 }
 else
 {
  apu.output(zebuf.center(), zebuf.left(), zebuf.right());
  apu.volume((double)FSettings.SoundVolume * 0.50 / 100);
  fmsynth.volume((double)FSettings.SoundVolume * 0.10 / 100);
 }
}

void SMS_SoundInit(bool WantMono, uint32 clock, bool WantFM)
{
 SoundClock = clock;

 zebuf.set_sample_rate(FSettings.SndRate?FSettings.SndRate:44100, 60);
 zebuf.clock_rate((long)(SoundClock * FSettings.soundmultiplier));

 forcemono = WantMono;

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

void SMS_SetSoundMultiplier(double multiplier)
{
 zebuf.clock_rate((long)(SoundClock * multiplier));
}

void SMS_SetSoundVolume(uint32 volume)
{
 RedoVolume();
}

void SMS_Sound(int rate)
{
 zebuf.set_sample_rate(rate?rate:44100, 60);
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
