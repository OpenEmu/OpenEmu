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
*/

#include "shared.h"
#include "blip.h"

/* Initial state of shift register */
#define NoiseInitialState 0x8000

/* Value below which PSG does not output  */
/*#define PSG_CUTOFF 0x6*/
#define PSG_CUTOFF 0x1

/* SN76489 clone in Sega's VDP chips (315-5124, 315-5246, 315-5313, Game Gear) */
#define FB_SEGAVDP  0x0009
#define SRW_SEGAVDP 16

typedef struct
{
  /* Configuration */
  int BoostNoise;         /* double noise volume when non-zero */

  /* PSG registers: */
  int Registers[8];       /* Tone, vol x4 */
  int LatchedRegister;
  int NoiseShiftRegister;
  int NoiseFreq;          /* Noise channel signal generator frequency */

  /* Output calculation variables */
  int ToneFreqVals[4];    /* Frequency register values (counters) */
  int ToneFreqPos[4];     /* Frequency channel flip-flops */
  int Channels[4];        /* Value of each channel, before stereo is applied */

  /* Blip-Buffer variables */
  int chan_amp[4];        /* current channel amplitudes in delta buffers */
} SN76489_Context;

static const uint16 PSGVolumeValues[16] =
{
  /* These values are taken from a real SMS2's output */
  /*{892,892,892,760,623,497,404,323,257,198,159,123,96,75,60,0}, */
  /* I can't remember why 892... :P some scaling I did at some point */
  /* these values are true volumes for 2dB drops at each step (multiply previous by 10^-0.1) */
  1516,1205,957,760,603,479,381,303,240,191,152,120,96,76,60,0
};

static struct blip_buffer_t* blip;  /* delta resampler */

static SN76489_Context SN76489;

void SN76489_Init(double PSGClockValue, int SamplingRate)
{
  SN76489_Shutdown();
  
  /* SamplingRate*16 instead of PSGClockValue/16 since division would lose some
      precision. blip_alloc doesn't care about the absolute sampling rate, just the
      ratio to clock rate. */
  blip = blip_alloc(PSGClockValue, SamplingRate * 16.0, SamplingRate / 4);
}

void SN76489_Shutdown(void)
{
  if (blip) blip_free(blip);
  blip = NULL;
}

void SN76489_Reset()
{
  int i;

  for(i = 0; i <= 3; i++)
  {
    /* Initialise PSG state */
    SN76489.Registers[2*i] = 1;         /* tone freq=1 */
    SN76489.Registers[2*i+1] = 0xf;     /* vol=off */

   /* Set counters to 0 */
    SN76489.ToneFreqVals[i] = 0;

   /* Set flip-flops to 1 */
    SN76489.ToneFreqPos[i] = 1;

   /* Clear channels output */
    SN76489.Channels[i] = 0;

   /* Clear current amplitudes in delta buffer */
    SN76489.chan_amp[i] = 0;
  }

  SN76489.LatchedRegister=0;

  /* Initialise noise generator */
  SN76489.NoiseShiftRegister=NoiseInitialState;
  SN76489.NoiseFreq = 0x10;
  SN76489.BoostNoise = config.psgBoostNoise;

  /* Clear Blip delta buffer */
  if (blip) blip_clear(blip);
}

void SN76489_BoostNoise(int boost)
{
  SN76489.BoostNoise = boost;
  SN76489.Channels[3]= PSGVolumeValues[SN76489.Registers[7]] << boost;
}

void SN76489_SetContext(uint8 *data)
{
  memcpy(&SN76489, data, sizeof(SN76489_Context));
}

void SN76489_GetContext(uint8 *data)
{
  memcpy(data, &SN76489, sizeof(SN76489_Context));
}

uint8 *SN76489_GetContextPtr(void)
{
  return (uint8 *)&SN76489;
}

int SN76489_GetContextSize(void)
{
  return sizeof(SN76489_Context);
}

void SN76489_Write(int data)
{
  if (data & 0x80)
  {
    /* Latch byte  %1 cc t dddd */
    SN76489.LatchedRegister = (data >> 4) & 0x07;
  }

  switch (SN76489.LatchedRegister)
  {
    case 0:
    case 2:
    case 4: /* Tone channels */
      if (data & 0x80)
      {
        /* Data byte  %1 cc t dddd */
        SN76489.Registers[SN76489.LatchedRegister] = (SN76489.Registers[SN76489.LatchedRegister] & 0x3f0) | (data & 0xf);
      }
      else
      {
        /* Data byte  %0 - dddddd */
        SN76489.Registers[SN76489.LatchedRegister] = (SN76489.Registers[SN76489.LatchedRegister] & 0x00f) | ((data & 0x3f) << 4);
      }
      /* Zero frequency changed to 1 to avoid div/0 */
      if (SN76489.Registers[SN76489.LatchedRegister] == 0) SN76489.Registers[SN76489.LatchedRegister] = 1;  
      break;

    case 1:
    case 3:
    case 5: /* Channel attenuation */
      SN76489.Registers[SN76489.LatchedRegister] = data & 0x0f;
      SN76489.Channels[SN76489.LatchedRegister>>1] = PSGVolumeValues[data&0x0f];
      break;

    case 6: /* Noise control */
      SN76489.Registers[6] = data & 0x0f;
      SN76489.NoiseShiftRegister = NoiseInitialState;  /* reset shift register */
      SN76489.NoiseFreq = 0x10 << (data&0x3); /* set noise signal generator frequency */
      break;

    case 7: /* Noise attenuation */
      SN76489.Registers[7] = data & 0x0f;
      SN76489.Channels[3] = PSGVolumeValues[data&0x0f] << SN76489.BoostNoise;
      break;
  }
}

/* Updates tone amplitude in delta buffer. Call whenever amplitude might have changed. */
static void UpdateToneAmplitude(int i, int time)
{
  int delta = (SN76489.Channels[i] * SN76489.ToneFreqPos[i]) - SN76489.chan_amp[i];
  if (delta != 0)
  {
    SN76489.chan_amp[i] += delta;
    blip_add(blip, time, delta);
  }
}

/* Updates noise amplitude in delta buffer. Call whenever amplitude might have changed. */
static void UpdateNoiseAmplitude(int time)
{
  int delta = (SN76489.Channels[3] * ( SN76489.NoiseShiftRegister & 0x1 )) - SN76489.chan_amp[3];
  if (delta != 0)
  {
    SN76489.chan_amp[3] += delta;
    blip_add(blip, time, delta);
  }
}

/* Runs tone channel for clock_length clocks */
static void RunTone(int i, int clock_length)
{
  int time;

  /* Update in case a register changed etc. */
  UpdateToneAmplitude(i, 0);

  /* Time of next transition */
  time = SN76489.ToneFreqVals[i];

  /* Process any transitions that occur within clocks we're running */
  while (time < clock_length)
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
    time += SN76489.Registers[i*2];
  }
  
  /* Calculate new value for register, now that next transition is past number of clocks we're running */
  SN76489.ToneFreqVals[i] = time - clock_length;
}

/* Runs noise channel for clock_length clocks */
static void RunNoise(int clock_length)
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
  UpdateNoiseAmplitude(0);

  /* Time of next transition */
  time = SN76489.ToneFreqVals[3];

  /* Process any transitions that occur within clocks we're running */
  while ( time < clock_length )
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
        Feedback = ((Feedback & FB_SEGAVDP) && ((Feedback & FB_SEGAVDP) ^ FB_SEGAVDP));
      }
      else    /* Periodic noise */
        Feedback = Feedback & 1;

      SN76489.NoiseShiftRegister = (SN76489.NoiseShiftRegister >> 1) | (Feedback << (SRW_SEGAVDP - 1));
      UpdateNoiseAmplitude(time);
    }

    /* Advance to time of next transition */
    time += NoiseFreq;
  }

  /* Calculate new value for register, now that next transition is past number of clocks we're running */
  SN76489.ToneFreqVals[3] = time - clock_length;
}

int SN76489_Update(INT16 *buffer, int clock_length)
{
  int i;

  /* Run noise first, since it might use current value of third tone frequency counter */
  RunNoise(clock_length);

  /* Run tone channels */
  for( i = 0; i <= 2; ++i )
    RunTone(i, clock_length);

  /* Read samples into output buffer */
  blip_end_frame(blip, clock_length);
  return blip_read_samples(blip, buffer, 0);
}

int SN76489_Clocks(int length)
{
  return blip_clocks_needed(blip, length);
}