#include "neopop.h"
#include "sound.h"

#include <blip/Blip_Buffer.h>
#include "T6W28_Apu.h"
#include <blip/Stereo_Buffer.h>

static T6W28_Apu apu;

static Stereo_Buffer buf;

static bool forcemono;

static uint8 LastDACLeft = 0, LastDACRight = 0;
static uint8 CurrentDACLeft = 0, CurrentDACRight = 0;

typedef Blip_Synth<blip_good_quality, 0xFF> Synth;
static Synth synth;
extern int32 ngpc_soundTS;
static bool8 schipenable = 0;

void MDFNNGPCSOUND_SetEnable(bool set)
{
 schipenable = set;
 if(!set)
  apu.reset();
}

void Write_SoundChipLeft(uint8 data)
{
 if(schipenable)
  apu.write_data_left(ngpc_soundTS >> 1, data);
}

void Write_SoundChipRight(uint8 data)
{
 if(schipenable)
  apu.write_data_right(ngpc_soundTS >> 1, data);
}


void dac_write_left(uint8 data)
{
 CurrentDACLeft = data;

 synth.offset_inline(ngpc_soundTS >> 1, CurrentDACLeft - LastDACLeft, buf.left());

 LastDACLeft = data;
}

void dac_write_right(uint8 data)
{
 CurrentDACRight = data;

 synth.offset_inline(ngpc_soundTS >> 1, CurrentDACRight - LastDACRight, buf.right());

 LastDACRight = data;
}


int16 *MDFNNGPCSOUND_Flush(int32 *length)
{
        static int16 buffer[8000];

        apu.end_frame(ngpc_soundTS >> 1);

        if(forcemono)
        {
         buf.left()->end_frame(ngpc_soundTS >> 1);
         *length = buf.left()->read_samples(buffer, 8000);
        }
        else
        {
         buf.end_frame(ngpc_soundTS >> 1);
         *length = buf.read_samples(buffer, 8000);
         *length /= 2;
        }

	if(!FSettings.SndRate)
	{
	 *length = 0;
	 return(NULL);
	}


        return(buffer);
}

static void RedoVolume(void)
{
 if(forcemono)
 {
  apu.output(buf.center(), buf.left(), buf.left());
  apu.volume((double)FSettings.SoundVolume * 0.30 / 2 / 100);
  synth.volume((double)FSettings.SoundVolume * 0.40 / 2 / 100);
 }
 else
 {
  apu.output(buf.center(), buf.left(), buf.right());
  apu.volume((double)FSettings.SoundVolume * 0.30 / 100);
  synth.volume((double)FSettings.SoundVolume * 0.40 / 100);
 }
}

void MDFNNGPCSOUND_Init(bool WantMono)
{
 buf.set_sample_rate(FSettings.SndRate?FSettings.SndRate:44100, 60);
 buf.clock_rate((long)(3072000 * FSettings.soundmultiplier));

 forcemono = WantMono;

 RedoVolume();
 buf.bass_freq(20);
}

void MDFNNGPC_SetSoundMultiplier(double multiplier)
{
        buf.clock_rate((long)(3072000 * multiplier));
}

void MDFNNGPC_SetSoundVolume(uint32 volume)
{
 RedoVolume();
}

void MDFNNGPC_Sound(int rate)
{
        buf.set_sample_rate(rate?rate:44100, 60);
}

int MDFNNGPCSOUND_StateAction(StateMem *sm, int load, int data_only)
{
 T6W28_ApuState *sn_state;

 if(!load)
 {
  sn_state = apu.save_state();
 }
 else
 {
  sn_state = (T6W28_ApuState *)malloc(sizeof(T6W28_ApuState));
 }

 SFORMAT StateRegs[] =
 {
  SFVAR(CurrentDACLeft),
  SFVAR(CurrentDACRight),

  SFVAR(schipenable),

  SFARRAY32N(sn_state->volume_left, 4, "VolumeLeft"),
  SFARRAY32N(sn_state->volume_right, 4, "VolumeRight"),
  SFARRAY32N(sn_state->sq_period, 3, "SQPeriod"),
  SFARRAY32N(sn_state->sq_phase, 3, "SQPhase"),
  SFVARN(sn_state->noise_period, "NPeriod"),
  SFVARN(sn_state->noise_shifter, "NShifter"),
  SFVARN(sn_state->noise_tap, "NTap"),
  SFVARN(sn_state->latch_left, "LatchLeft"),
  SFVARN(sn_state->latch_right, "LatchRight"),
  SFEND
 };

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "SND"))
 {
  free(sn_state);
  return(0);
 }

 if(load)
 {
  apu.load_state(sn_state);
  synth.offset(ngpc_soundTS >> 1, CurrentDACLeft - LastDACLeft, buf.left());
  synth.offset(ngpc_soundTS >> 1, CurrentDACRight - LastDACRight, buf.right());
  LastDACLeft = CurrentDACLeft;
  LastDACRight = CurrentDACRight;
 }

 free(sn_state);
 return(1);
}
