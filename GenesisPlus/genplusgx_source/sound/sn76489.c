/* 
    SN76489 emulation
    by Maxim in 2001 and 2002
    converted from my original Delphi implementation

    I'm a C newbie so I'm sure there are loads of stupid things
    in here which I'll come back to some day and redo

    Includes:
    - Super-high quality tone channel "oversampling" by calculating fractional positions on transitions
    - Noise output pattern reverse engineered from actual SMS output
    - Volume levels taken from actual SMS output

    07/08/04  Charles MacDonald
    Modified for use with SMS Plus:
    - Added support for multiple PSG chips.
    - Added reset/config/update routines.
    - Added context management routines.
    - Removed SN76489_GetValues().
    - Removed some unused variables.

   25/04/07 Eke-Eke (Genesis Plus GX)
    - Removed stereo GG support (unused)
    - Made SN76489_Update outputs 16bits mono samples
    - Replaced volume table with VGM plugin's one

   05/01/09 Eke-Eke (Genesis Plus GX)
    - Modified Cut-Off frequency (according to Steve Snake: http://www.smspower.org/forums/viewtopic.php?t=1746)

   24/08/10 Eke-Eke (Genesis Plus GX)
    - Removed multichip support (unused)
    - Removed alternate volume table, panning & mute support (unused)
    - Removed configurable Feedback and Shift Register Width (always use Sega ones)
    - Added linear resampling using Blip Buffer (based on Blargg's implementation: http://www.smspower.org/forums/viewtopic.php?t=11376)

   01/09/12 Eke-Eke (Genesis Plus GX)
    - Added generic Blip-Buffer support internally, using common Master Clock as timebase
    - Re-added stereo GG support
    - Re-added configurable Feedback and Shift Register Width
    - Rewrote core with various optimizations
*/

#include "shared.h"

#define PSG_MCYCLES_RATIO (16 * 15)

/* Initial state of shift register */
#define NoiseInitialState 0x8000

/* Value below which PSG does not output  */
/*#define PSG_CUTOFF 0x6*/
#define PSG_CUTOFF 0x1

/* original Texas Instruments TMS SN76489AN (rev. A) used in SG-1000, SC-3000H & SF-7000 computers */
#define FB_DISCRETE 0x0006
#define SRW_DISCRETE  15

/* SN76489AN clone integrated in Sega's VDP chips (315-5124, 315-5246, 315-5313, Game Gear) */
#define FB_SEGAVDP 0x0009
#define SRW_SEGAVDP 16

typedef struct
{
  /* Configuration */
  int PreAmp[4][2];       /* stereo channels pre-amplification ratio (%) */
  int NoiseFeedback;
  int SRWidth;

  /* PSG registers: */
  int Registers[8];       /* Tone, vol x4 */
  int LatchedRegister;
  int NoiseShiftRegister;
  int NoiseFreq;          /* Noise channel signal generator frequency */

  /* Output calculation variables */
  int ToneFreqVals[4];    /* Frequency register values (counters) */
  int ToneFreqPos[4];     /* Frequency channel flip-flops */
  int Channel[4][2];      /* current amplitude of each (stereo) channel */
  int ChanOut[4][2];      /* current output value of each (stereo) channel */

  /* Internal M-clock counter */
  unsigned long clocks;

} SN76489_Context;

static const uint16 PSGVolumeValues[16] =
{
  /* These values are taken from a real SMS2's output */
  /*{892,892,892,760,623,497,404,323,257,198,159,123,96,75,60,0}, */
  /* I can't remember why 892... :P some scaling I did at some point */
  /* these values are true volumes for 2dB drops at each step (multiply previous by 10^-0.1) */
  1516,1205,957,760,603,479,381,303,240,191,152,120,96,76,60,0
};

static SN76489_Context SN76489;

static blip_t* blip[2];

void SN76489_Init(blip_t* left, blip_t* right, int type)
{
  int i;
  
  blip[0] = left;
  blip[1] = right;

  for (i=0; i<4; i++)
  {
    SN76489.PreAmp[i][0] = 100;
    SN76489.PreAmp[i][1] = 100;
  }

  if (type == SN_DISCRETE)
  {
    SN76489.NoiseFeedback = FB_DISCRETE;
    SN76489.SRWidth = SRW_DISCRETE;
  }
  else
  {
    SN76489.NoiseFeedback = FB_SEGAVDP;
    SN76489.SRWidth = SRW_SEGAVDP;
  }
}

void SN76489_Reset()
{
  int i;

  for(i = 0; i <= 3; i++)
  {
    /* Initialise PSG state */
    SN76489.Registers[2*i] = 1; /* tone freq=1 */
    SN76489.Registers[2*i+1] = 0xf; /* vol=off */

    /* Set counters to 0 */
    SN76489.ToneFreqVals[i] = 0;

    /* Set flip-flops to 1 */
    SN76489.ToneFreqPos[i] = 1;

    /* Clear stereo channels amplitude */
    SN76489.Channel[i][0] = 0;
    SN76489.Channel[i][1] = 0;

   /* Clear stereo channel outputs in delta buffer */
    SN76489.ChanOut[i][0] = 0;
    SN76489.ChanOut[i][1] = 0;
  }

  /* Initialise latched register index */
  SN76489.LatchedRegister = 0;

  /* Initialise noise generator */
  SN76489.NoiseShiftRegister=NoiseInitialState;
  SN76489.NoiseFreq = 0x10;

  /* Reset internal M-cycle counter */
  SN76489.clocks = 0;
}

void *SN76489_GetContextPtr(void)
{
  return (uint8 *)&SN76489;
}

int SN76489_GetContextSize(void)
{
  return sizeof(SN76489_Context);
}

/* Updates tone amplitude in delta buffer. Call whenever amplitude might have changed. */
INLINE void UpdateToneAmplitude(int i, int time)
{
  int delta;

  /* left output */
  delta = (SN76489.Channel[i][0] * SN76489.ToneFreqPos[i]) - SN76489.ChanOut[i][0];
  if (delta != 0)
  {
    SN76489.ChanOut[i][0] += delta;
    blip_add_delta_fast(blip[0], time, delta);
  }

  /* right output */
  delta = (SN76489.Channel[i][1] * SN76489.ToneFreqPos[i]) - SN76489.ChanOut[i][1];
  if (delta != 0)
  {
    SN76489.ChanOut[i][1] += delta;
    blip_add_delta_fast(blip[1], time, delta);
  }
}

/* Updates noise amplitude in delta buffer. Call whenever amplitude might have changed. */
INLINE void UpdateNoiseAmplitude(int time)
{
  int delta;

  /* left output */
  delta = (SN76489.Channel[3][0] * ( SN76489.NoiseShiftRegister & 0x1 )) - SN76489.ChanOut[3][0];
  if (delta != 0)
  {
    SN76489.ChanOut[3][0] += delta;
    blip_add_delta_fast(blip[0], time, delta);
  }

  /* right output */
  delta = (SN76489.Channel[3][1] * ( SN76489.NoiseShiftRegister & 0x1 )) - SN76489.ChanOut[3][1];
  if (delta != 0)
  {
    SN76489.ChanOut[3][1] += delta;
    blip_add_delta_fast(blip[1], time, delta);
  }
}

/* Runs tone channel for clock_length clocks */
static void RunTone(int i, int clocks)
{
  int time;

  /* Update in case a register changed etc. */
  UpdateToneAmplitude(i, SN76489.clocks);

  /* Time of next transition */
  time = SN76489.ToneFreqVals[i];

  /* Process any transitions that occur within clocks we're running */
  while (time < clocks)
  {
    if (SN76489.Registers[i*2]>PSG_CUTOFF) {
      /* Flip the flip-flop */
      SN76489.ToneFreqPos[i] = -SN76489.ToneFreqPos[i];
    } else {
      /* stuck value */
      SN76489.ToneFreqPos[i] = 1;
    }
    UpdateToneAmplitude(i, time);

    /* Advance to time of next transition */
    time += SN76489.Registers[i*2] * PSG_MCYCLES_RATIO;
  }
  
  /* Update channel tone counter */
  SN76489.ToneFreqVals[i] = time;
}

/* Runs noise channel for clock_length clocks */
static void RunNoise(int clocks)
{
  int time;

  /* Noise channel: match to tone2 if in slave mode */
  int NoiseFreq = SN76489.NoiseFreq;
  if (NoiseFreq == 0x80)
  {
    NoiseFreq = SN76489.Registers[2*2];
    SN76489.ToneFreqVals[3] = SN76489.ToneFreqVals[2];
  }

  /* Update in case a register changed etc. */
  UpdateNoiseAmplitude(SN76489.clocks);

  /* Time of next transition */
  time = SN76489.ToneFreqVals[3];

  /* Process any transitions that occur within clocks we're running */
  while (time < clocks)
  {
    /* Flip the flip-flop */
    SN76489.ToneFreqPos[3] = -SN76489.ToneFreqPos[3];
    if (SN76489.ToneFreqPos[3] == 1)
    {
      /* On the positive edge of the square wave (only once per cycle) */
      int Feedback = SN76489.NoiseShiftRegister;
      if ( SN76489.Registers[6] & 0x4 )
      {
        /* White noise */
        /* Calculate parity of fed-back bits for feedback */
        /* Do some optimised calculations for common (known) feedback values */
        /* If two bits fed back, I can do Feedback=(nsr & fb) && (nsr & fb ^ fb) */
        /* since that's (one or more bits set) && (not all bits set) */
        Feedback = ((Feedback & SN76489.NoiseFeedback) && ((Feedback & SN76489.NoiseFeedback) ^ SN76489.NoiseFeedback));
      }
      else    /* Periodic noise */
        Feedback = Feedback & 1;

      SN76489.NoiseShiftRegister = (SN76489.NoiseShiftRegister >> 1) | (Feedback << (SN76489.SRWidth - 1));
      UpdateNoiseAmplitude(time);
    }

    /* Advance to time of next transition */
    time += NoiseFreq * PSG_MCYCLES_RATIO;
  }

  /* Update channel tone counter */
  SN76489.ToneFreqVals[3] = time;
}

static void SN76489_RunUntil(unsigned int clocks)
{
  int i;

  /* Run noise first, since it might use current value of third tone frequency counter */
  RunNoise(clocks);

  /* Run tone channels */
  for (i=0; i<3; ++i)
  {
    RunTone(i, clocks);
  }
}

void SN76489_Config(unsigned int clocks, int preAmp, int boostNoise, int stereo)
{
  int i;

  /* cycle-accurate Game Gear stereo */
  if (clocks > SN76489.clocks)
  {
    /* Run chip until current timestamp */
    SN76489_RunUntil(clocks);

    /* Update internal M-cycle counter */
    SN76489.clocks += ((clocks - SN76489.clocks + PSG_MCYCLES_RATIO - 1) / PSG_MCYCLES_RATIO) * PSG_MCYCLES_RATIO;
  }

  for (i=0; i<4; i++)
  {
    /* stereo channel pre-amplification */
    SN76489.PreAmp[i][0] = preAmp * ((stereo >> (i + 4)) & 1);
    SN76489.PreAmp[i][1] = preAmp * ((stereo >> (i + 0)) & 1);

    /* noise channel boost */
    if (i == 3)
    {
      SN76489.PreAmp[3][0] = SN76489.PreAmp[3][0] << boostNoise;
      SN76489.PreAmp[3][1] = SN76489.PreAmp[3][1] << boostNoise;
    }

    /* update stereo channel amplitude */
    SN76489.Channel[i][0]= (PSGVolumeValues[SN76489.Registers[i*2 + 1]] * SN76489.PreAmp[i][0]) / 100;
    SN76489.Channel[i][1]= (PSGVolumeValues[SN76489.Registers[i*2 + 1]] * SN76489.PreAmp[i][1]) / 100;
  }
}

void SN76489_Update(unsigned int clocks)
{
  int i;

  if (clocks > SN76489.clocks)
  {
    /* Run chip until current timestamp */
    SN76489_RunUntil(clocks);

    /* Update internal M-cycle counter */
    SN76489.clocks += ((clocks - SN76489.clocks + PSG_MCYCLES_RATIO - 1) / PSG_MCYCLES_RATIO) * PSG_MCYCLES_RATIO;
  }

  /* Adjust internal M-cycle counter for next frame */
  SN76489.clocks -= clocks;

	/* Adjust channel time counters for new frame */
	for (i=0; i<4; ++i)
	{
		SN76489.ToneFreqVals[i] -= clocks;
	}
}

void SN76489_Write(unsigned int clocks, unsigned int data)
{
  unsigned int index;

  if (clocks > SN76489.clocks)
  {
    /* run chip until current timestamp */
    SN76489_RunUntil(clocks);

    /* update internal M-cycle counter */
    SN76489.clocks += ((clocks - SN76489.clocks + PSG_MCYCLES_RATIO - 1) / PSG_MCYCLES_RATIO) * PSG_MCYCLES_RATIO;
  }

  if (data & 0x80)
  {
    /* latch byte  %1 cc t dddd */
    SN76489.LatchedRegister = index = (data >> 4) & 0x07;
  }
  else
  {
    /* restore latched register index */
    index = SN76489.LatchedRegister;
  }

  switch (index)
  {
    case 0:
    case 2:
    case 4: /* Tone Channels frequency */
    {
      if (data & 0x80)
      {
        /* Data byte  %1 cc t dddd */
        SN76489.Registers[index] = (SN76489.Registers[index] & 0x3f0) | (data & 0xf);
      }
      else
      {
        /* Data byte  %0 - dddddd */
        SN76489.Registers[index] = (SN76489.Registers[index] & 0x00f) | ((data & 0x3f) << 4);
      }

      /* zero frequency behaves the same as a value of 1 */
      if (SN76489.Registers[index] == 0)
      {
        SN76489.Registers[index] = 1;
      }
      break;
    }

    case 1:
    case 3:
    case 5: /* Tone Channels attenuation */
    {
      data &= 0x0f;
      SN76489.Registers[index] = data;
      data = PSGVolumeValues[data];
      index >>= 1;
      SN76489.Channel[index][0] = (data * SN76489.PreAmp[index][0]) / 100;
      SN76489.Channel[index][1] = (data * SN76489.PreAmp[index][1]) / 100;
      break;
    }

    case 6: /* Noise control */
    {
      SN76489.Registers[6] = data & 0x0f;

      /* reset shift register */
      SN76489.NoiseShiftRegister = NoiseInitialState;

      /* set noise signal generator frequency */
      SN76489.NoiseFreq = 0x10 << (data&0x3);
      break;
    }

    case 7: /* Noise attenuation */
    {
      data &= 0x0f;
      SN76489.Registers[7] = data;
      data = PSGVolumeValues[data];
      SN76489.Channel[3][0] = (data * SN76489.PreAmp[3][0]) / 100;
      SN76489.Channel[3][1] = (data * SN76489.PreAmp[3][1]) / 100;
      break;
    }
  }
}
