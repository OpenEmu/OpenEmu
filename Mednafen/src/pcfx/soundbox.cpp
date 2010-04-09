/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <math.h>
#include <string.h>
#include "pcfx.h"
#include "soundbox.h"
#include "v810_cpu.h"
#include "vdc.h"
#include "king.h"
#include "../cdrom/scsicd.h"
#include "timer.h"
#include "input.h"

typedef Blip_Synth<blip_good_quality, 65536> ADSynth;
static ADSynth ADPCMSynth[2][2]; // [Ch0, Ch1][Left, right]
static Blip_Synth<blip_good_quality, 4096> WaveSynth;
static Blip_Synth<blip_med_quality, 4096> NoiseSynth;

Blip_Buffer FXsbuf[2];		// Used in the CDROM code

static t_soundbox psg;
static double ADPCMVolTable[0x40];

#define CHPDMOO(n)	\
 { "CH"#n"Freq", "PSG Ch"#n" Frequency(Period)", 2 },	\
 { "CH"#n"Ctrl", "PSG Ch"#n" Control", 1 },	\
 { "CH"#n"Balance", "PSG Ch"#n" Balance", 1 },	\
 { "CH"#n"WIndex", "PSG Ch"#n" Waveform Index", 1},	\
 { "CH"#n"SCache", "PSG Ch"#n" Sample Cache", 1 }


#ifdef WANT_DEBUGGER
static RegType SBoxRegs[] =
{
 { "Select", "PSG Channel Select", 1 },
 { "GBalance", "PSG Global Balance", 1 },
 { "LFOFreq", "PSG LFO Freq", 1 },
 { "LFOCtrl", "PSG LFO Control", 1 },
 
 CHPDMOO(0),
 CHPDMOO(1),
 CHPDMOO(2),
 CHPDMOO(3),
 CHPDMOO(4),
 { "CH4NCtrl", "PSG Ch4 Noise Control", 1 },
 { "CH4LFSR", "PSG Ch4 Noise LFSR", 2 },
 CHPDMOO(5),
 { "CH5NCtrl", "PSG Ch5 Noise Control", 1 },
 { "CH5LFSR", "PSG Ch5 Noise LFSR", 2 },

 { "ADPCMCtrl", "ADPCM Control", 2 },
 { "ADPCM0LVol", "ADPCM Ch0 Left Volume", 1 },
 { "ADPCM0RVol", "ADPCM Ch0 Right Volume", 1 },
 { "ADPCM1LVol", "ADPCM Ch1 Left Volume", 1 },
 { "ADPCM1RVol", "ADPCM Ch1 Right Volume", 1 },

 { "CDDALVol", "CD-DA Left Volume", 1 },
 { "CDDARVol", "CD-DA Right Volume", 1 },
 { "", "", 0 },
};

static uint32 SBoxDBG_GetRegister(const std::string &name, std::string *special)
{
 uint32 value = 0xDEADBEEF;

 if(name == "Select")
  value = psg.select;
 else if(name == "GBalance")
  value = psg.globalbalance;
 else if(name == "LFOFreq")
  value = psg.lfofreq;
 else if(name == "LFOCtrl")
  value = psg.lfoctrl;
 else if(name == "ADPCMCtrl")
 {
  value = psg.ADPCMControl;
  if(special)
  {
   char buf[256];
   int tmp_freq = 32 / (1 << (value & 0x3));
   snprintf(buf, 256, "Frequency: ~%dKHz, Ch0 Linear: %s, Ch1 Linear: %s, Ch0 Reset: %d, Ch1 Reset: %d", tmp_freq, (value & 0x4) ? "On" : "Off", (value & 0x8) ? "On":"Off",
	(int)(bool)(value & 0x10), (int)(bool)(value & 0x20));
   *special = std::string(buf);
  }
 }
 else if(name == "ADPCM0LVol")
  value = psg.ADPCMVolume[0][0];
 else if(name == "ADPCM0RVol")
  value = psg.ADPCMVolume[0][1];
 else if(name == "ADPCM1LVol")
  value = psg.ADPCMVolume[1][0];
 else if(name == "ADPCM1RVol")
  value = psg.ADPCMVolume[1][1];
 else if(name == "CDDALVol")
  value = psg.CDDAVolume[0];
 else if(name == "CDDARVol")
  value = psg.CDDAVolume[1];
 else if(!strncmp(name.c_str(), "CH", 2))
 {
  int ch = name[2] - '0';
  char moomoo[64];
  strncpy(moomoo, name.c_str() + 3, 63);
  if(!strcmp(moomoo, "Freq"))
   value = psg.channel[ch].frequency;
  else if(!strcmp(moomoo, "Ctrl"))
   value = psg.channel[ch].control;
  else if(!strcmp(moomoo, "Balance"))
   value = psg.channel[ch].balance;
  else if(!strcmp(moomoo, "WIndex"))
   value = psg.channel[ch].waveform_index;
  else if(!strcmp(moomoo, "SCache"))
   value = psg.channel[ch].dda;
  else if(!strcmp(moomoo, "NCtrl"))
   value = psg.channel[ch].noisectrl;
  else if(!strcmp(moomoo, "LFSR"))
   value = psg.channel[ch].lfsr & 0x7FFF;
 }
 return(value);
}


static RegGroupType SBoxRegsGroup =
{
 SBoxRegs,
 SBoxDBG_GetRegister,
 NULL
};


#endif


void SoundBox_SetSoundMultiplier(double multiplier)
{
 for(int y = 0; y < 2; y++)
 {
  FXsbuf[y].clock_rate((long)(1789772.727272 * 4 * multiplier));
 }
}

static INLINE void RedoADPCMVolume(uint8 ch, uint8 lr) // bitmasks
{
 if((ch & 1) && (lr & 1))
  ADPCMSynth[0][0].volume((double)FSettings.SoundVolume / 100);
 if((ch & 2) && (lr & 1))
  ADPCMSynth[1][0].volume((double)FSettings.SoundVolume / 100);

 if((ch & 1) && (lr & 2))
  ADPCMSynth[0][1].volume((double)FSettings.SoundVolume / 100);
 if((ch & 2) && (lr & 2))
  ADPCMSynth[1][1].volume((double)FSettings.SoundVolume / 100);
}

static void RedoVolume(void)
{
 WaveSynth.volume((double)FSettings.SoundVolume / 100 * 0.227 * 0.50);
 NoiseSynth.volume((double)FSettings.SoundVolume / 100 * 0.227 * 0.50);
 RedoADPCMVolume(0x03, 0x03);
}

void SoundBox_SetSoundVolume(uint32 volume)
{
 RedoVolume();
}

void SoundBox_Sound(int rate)
{
 for(int y = 0; y < 2; y++)
 {
  FXsbuf[y].set_sample_rate(rate ? rate : 44100, 50);
  FXsbuf[y].clock_rate((long)(1789772.727272 * 4));
  FXsbuf[y].bass_freq(20);
 }

 if(psg.WaveIL)
  free(psg.WaveIL);

 psg.WaveFinalLen = rate / 60 * 2; // *2 for extra room
 psg.WaveIL = (int16 *)malloc(sizeof(int16) * psg.WaveFinalLen * 2); // *2 for stereo

 RedoVolume();
}

static int32 dbtable[32][32 * 3];
 static const int scale_tab[] = {
        0x00, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F,
        0x10, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F
        };

#define redo_ddacache(ch)	\
{	\
 int vll,vlr;	\
 vll = (ch)->lal + (ch)->al + psg.lmal;	\
 vlr = (ch)->ral + (ch)->al + psg.rmal;	\
 (ch)->dda_cache[0] = dbtable[(ch)->dda][vll];	\
 (ch)->dda_cache[1] = dbtable[(ch)->dda][vlr];	\
}

int SoundBox_Init(void)
{
    int x;

    #ifdef WANT_DEBUGGER
    MDFNDBG_AddRegGroup(&SBoxRegsGroup);
    #endif

    memset(&psg, 0, sizeof(psg));
    if(FSettings.SndRate)
     SoundBox_Sound(FSettings.SndRate);

    for(x=0;x<32 * 3;x++)
    {
     int y;
     double flub;

     flub = 1;

     if(x)
      flub /= pow(2, (double)1/4*x); //4*x);		// ~1.5dB reduction per increment of x
     for(y=0;y<32;y++)
      dbtable[y][x] = (int32)(flub * (y - 0x10) * 128);
     //printf("%f\n",flub);
    }

    // Build ADPCM volume table, 1.5dB per step, ADPCM volume settings of 0x0 through 0x1B result in silence.
    for(x = 0; x < 0x40; x++)
    {
     double flub = 1;
     int vti = 0x3F - x;

     if(x) 
      flub /= pow(2, (double)1 / 4 * x);

     if(vti <= 0x1B)
      ADPCMVolTable[vti] = 0;
     else
      ADPCMVolTable[vti] =  256 * flub * 0.25;
    }
    return (1);
}

void SoundBox_Kill(void)
{
	if(psg.WaveIL)
	 free(psg.WaveIL);
	psg.WaveIL = 0;
}

/*--------------------------------------------------------------------------*/
/* PSG emulation                                                            */
/*--------------------------------------------------------------------------*/

/* Macro to access currently selected PSG channel */
#define PSGCH   psg.channel[psg.select]
void SoundBox_Update(void);
void snortus(void);
void SoundBox_Write(uint32 A, uint16 V)
{
    int x;

    SoundBox_Update();

    switch(A & 0x3F)
    {
	//default: printf("HARUM: %04x %04x\n", A, V); break;
	case 0x20: DoADPCMUpdate();
		   for(int ch = 0; ch < 2; ch++)
		    if(!(psg.ADPCMControl & (0x10 << ch)) && (V & (0x10 << ch)))
		    {
		     //printf("Reset: %d\n", ch);
		     //psg.ADPCMCurrent[ch][0] = psg.ADPCMCurrent[ch][1] = 0;
		     psg.ADPCMEstim[ch] = 0;
		    }
		   psg.ADPCMControl = V; 
		   //printf("ADPCM Control: %04x\n", V);
		   break;
	case 0x22: DoADPCMUpdate();
		   psg.ADPCMVolume[0][0] = V & 0x3F; 
		   RedoADPCMVolume(0x01, 0x01);
		   break;
	case 0x24: DoADPCMUpdate();
		   psg.ADPCMVolume[0][1] = V & 0x3F;
		   RedoADPCMVolume(0x01, 0x02);
		   break;
	case 0x26: DoADPCMUpdate();
		   psg.ADPCMVolume[1][0] = V & 0x3F;
		   RedoADPCMVolume(0x02, 0x01);
		   break;
	case 0x28: DoADPCMUpdate();
		   psg.ADPCMVolume[1][1] = V & 0x3F; 
		   RedoADPCMVolume(0x02, 0x02);
		   break;
	case 0x2A: psg.CDDAVolume[0] = V & 0x3F; SCSICD_SetCDDAVolume(0.50f * psg.CDDAVolume[0] / 63, 0.50f * psg.CDDAVolume[1] / 63); break;
	case 0x2C: psg.CDDAVolume[1] = V & 0x3F; SCSICD_SetCDDAVolume(0.50f * psg.CDDAVolume[0] / 63, 0.50f * psg.CDDAVolume[1] / 63); break;

	case 0x00: psg.select = (V & 0x07); break;
        case 0x02: /* Global sound balance */
            psg.globalbalance = V;
	    psg.lmal = 0x1F - scale_tab[(psg.globalbalance >> 4) & 0xF];
	    psg.rmal = 0x1F - scale_tab[(psg.globalbalance >> 0) & 0xF];
	    for(x=0;x<6;x++)
	     redo_ddacache(&psg.channel[x]);
            break;

        case 0x04: /* Channel frequency (LSB) */
	    if(psg.select > 5) return; // no more than 6 channels, silly game.
            PSGCH.frequency = (PSGCH.frequency & 0x0F00) | V;
	    PSGCH.base_frequency = PSGCH.frequency;
            break;

        case 0x06: /* Channel frequency (MSB) */
	    if(psg.select > 5) return; // no more than 6 channels, silly game.
            PSGCH.frequency = (PSGCH.frequency & 0x00FF) | ((V & 0x0F) << 8);
            PSGCH.base_frequency = PSGCH.frequency;
            break;

        case 0x08: /* Channel enable, DDA, volume */
	    if(psg.select > 5) return; // no more than 6 channels, silly game.
            PSGCH.control = V;
	    PSGCH.al = 0x1F - (PSGCH.control & 0x1F);
            if((V & 0xC0) == 0x40) PSGCH.waveform_index = 0;
            redo_ddacache(&PSGCH);
            break;

        case 0x0A: /* Channel balance */
	    if(psg.select > 5) return; // no more than 6 channels, silly game.
            PSGCH.balance = V;
	    PSGCH.lal = 0x1F - scale_tab[(V >> 4) & 0xF];
	    PSGCH.ral = 0x1F - scale_tab[(V >> 0) & 0xF];
            redo_ddacache(&PSGCH);
            break;

        case 0x0C: /* Channel waveform data */
	    if(psg.select > 5) return; // no more than 6 channels, silly game.
            V &= 0x1F;

            PSGCH.waveform[PSGCH.waveform_index] = V;

	    if((PSGCH.control & 0xC0) == 0x00)
             PSGCH.waveform_index = ((PSGCH.waveform_index + 1) & 0x1F);

	    if(PSGCH.control & 0x80)
	     PSGCH.dda = V;

	    redo_ddacache(&PSGCH);
            break;

        case 0x0E: /* Noise enable and frequency */
	    if(psg.select > 5) return; // no more than 6 channels, silly game.
            if(psg.select >= 4) PSGCH.noisectrl = V;
            break;

        case 0x10: /* LFO frequency */
            psg.lfofreq = V;
            break;

        case 0x12: /* LFO trigger and control */
	    if((V & 0x80) && !(psg.lfoctrl & 0x80))
	     psg.channel[1].waveform_index = 0;
            psg.lfoctrl = V;
            break;
    }
 SoundBox_Update();
}

/* MSM 6258 code transcribed/implemented/whatever from msm6258.c from NetBSD,
   Copyright (c) 2001 Tetsuya Isaki. All rights reserved.
*/

static const int EstimIndexTable[16] = 
{
          2,  6,  10,  14,  18,  22,  26,  30,
         -2, -6, -10, -14, -18, -22, -26, -30
};

static const int EstimTable[49] = 
{
         16,  17,  19,  21,  23,  25,  28,  31,  34,  37,
         41,  45,  50,  55,  60,  66,  73,  80,  88,  97,
        107, 118, 130, 143, 157, 173, 190, 209, 230, 253,
        279, 307, 337, 371, 408, 449, 494, 544, 598, 658,
        724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552
};

static const int EstimStepTable[16] = 
{
        -1, -1, -1, -1, 2, 4, 6, 8,
        -1, -1, -1, -1, 2, 4, 6, 8
};

static uint32 KINGADPCMControl;
static uint32 LSampleFreq = 0xFFFF;

void SoundBox_SetKINGADPCMControl(uint32 value)
{
 KINGADPCMControl = value;

 uint32 SampleFreq = (KINGADPCMControl >> 2) & 0x3;
 if(LSampleFreq != SampleFreq)
 {
  int rolloff = (int)((double)0.90 * 21477272 * 2 / 1365 / (1 << SampleFreq) ) / 2;
  for(int ch = 0; ch < 2; ch++)
   for(int lr = 0; lr < 2; lr++)
    ADPCMSynth[ch][lr].treble_eq(blip_eq_t::blip_eq_t(-1000, rolloff, FSettings.SndRate));
  LSampleFreq = SampleFreq;
 }
}

static int32 adpcm_lastts;

void DoADPCMUpdate(void)
{
 int32 run_time = v810_timestamp - adpcm_lastts;

 adpcm_lastts = v810_timestamp;

 psg.bigdiv -= run_time * 2;

 while(psg.bigdiv <= 0)
 {
  //printf("%d\n", v810_timestamp);
  psg.smalldiv--;
  while(psg.smalldiv <= 0)
  {
   psg.smalldiv += 1 << ((KINGADPCMControl >> 2) & 0x3);
   for(int ch = 0; ch < 2; ch++)
   {
    uint32 synthtime = ((v810_timestamp + (psg.bigdiv >> 19))) / 3;

    if(psg.ADPCMHaveHalfWord[ch] || KINGADPCMControl & (1 << ch)) // Keep playing our last halfword fetched even if KING ADPCM is disabled
    {
     uint8 zenibble;

     if(!psg.ADPCMWhichNibble[ch])
     {
      psg.ADPCMHalfWord[ch] = KING_GetADPCMHalfWord(ch);
      psg.ADPCMHaveHalfWord[ch] = TRUE;
     }

     zenibble = (psg.ADPCMHalfWord[ch] >> psg.ADPCMWhichNibble[ch]) & 0xF;

     psg.ADPCMWhichNibble[ch] = (psg.ADPCMWhichNibble[ch] + 4) & 0xF;

     if(!psg.ADPCMWhichNibble[ch])
      psg.ADPCMHaveHalfWord[ch] = FALSE;

     // FIXME DC-bias removal kludge
     psg.ADPCMCurrent[ch][0] -= psg.ADPCMCurrent[ch][0] >> 16;
     psg.ADPCMCurrent[ch][1] -= psg.ADPCMCurrent[ch][1] >> 16;

     psg.ADPCMCurrent[ch][0] += EstimTable[psg.ADPCMEstim[ch]] * EstimIndexTable[zenibble] * ADPCMVolTable[psg.ADPCMVolume[ch][0]];
     psg.ADPCMCurrent[ch][1] += EstimTable[psg.ADPCMEstim[ch]] * EstimIndexTable[zenibble] * ADPCMVolTable[psg.ADPCMVolume[ch][1]];

     
     // this isn't right, but very little about this ADPCM decoding is right, and it might make the ADPCM sound sound a bit better. :b
     if(psg.ADPCMCurrent[ch][0] >  0x0FFFFFF) { psg.ADPCMCurrent[ch][0] = 0xFFFFFF; }
     if(psg.ADPCMCurrent[ch][1] >  0x0FFFFFF) { psg.ADPCMCurrent[ch][1] = 0xFFFFFF; }
     if(psg.ADPCMCurrent[ch][0] < -0x1000000) { psg.ADPCMCurrent[ch][0] = -0x1000000; }
     if(psg.ADPCMCurrent[ch][1] < -0x1000000) { psg.ADPCMCurrent[ch][1] = -0x1000000; }

     //if(psg.ADPCMCurrent[ch][0] >  0x7FFFFF) { psg.ADPCMCurrent[ch][0] = 0x7FFFFF; }
     //if(psg.ADPCMCurrent[ch][1] >  0x7FFFFF) { psg.ADPCMCurrent[ch][1] = 0x7FFFFF; }
     //if(psg.ADPCMCurrent[ch][0] < -0x800000) { psg.ADPCMCurrent[ch][0] = -0x800000; }
     //if(psg.ADPCMCurrent[ch][1] < -0x800000) { psg.ADPCMCurrent[ch][1] = -0x800000; }

     psg.ADPCMEstim[ch] += EstimStepTable[zenibble];

     if(psg.ADPCMEstim[ch] < 0)
     {
        //printf("Estim underflow: %d\n", psg.ADPCMEstim[ch]);
	psg.ADPCMEstim[ch] = 0;
     }
     if(psg.ADPCMEstim[ch] > 48)
     {
        //printf("Estim overflow: %d\n", psg.ADPCMEstim[ch]);
	psg.ADPCMEstim[ch] = 48;
     }
    }
    else
    {
     // FIXME DC-bias removal kludge
     psg.ADPCMCurrent[ch][0] -= psg.ADPCMCurrent[ch][0] >> 12;
     psg.ADPCMCurrent[ch][1] -= psg.ADPCMCurrent[ch][1] >> 12;
    }
    if(FSettings.SndRate)
    {
     ADPCMSynth[ch][0].offset(synthtime, (psg.ADPCMCurrent[ch][0] >> 8) - psg.ADPCM_last[ch][0], &FXsbuf[0]);
     ADPCMSynth[ch][1].offset(synthtime, (psg.ADPCMCurrent[ch][1] >> 8) - psg.ADPCM_last[ch][1], &FXsbuf[1]);

     psg.ADPCM_last[ch][0] = psg.ADPCMCurrent[ch][0] >> 8;
     psg.ADPCM_last[ch][1] = psg.ADPCMCurrent[ch][1] >> 8;
    }
   }
  }
  psg.bigdiv += 1365 * 2 / 2;
 }

 v810_setevent(V810_EVENT_ADPCM, (psg.bigdiv + 1) / 2);
}

static void DoLFOUpdate(int32 run_time, int32 timestamp)
{
  int chc;
  for(chc = 2; chc < 6; chc++)
  {
   psg_channel *ch = &psg.channel[chc];
   int disabled = ((ch->control & 0x80)^0x80) >> 7;
   //puts("ROO");
   if(disabled) 
    continue; 
   #include "psg-loop.h"
  }

  #define LFO_ON
  chc = 0;
  psg_channel *ch = &psg.channel[0];
  #include "psg-loop.h"
  #undef LFO_ON
}

static void DoNormalUpdate(int32 run_time, int32 timestamp)
{
 int chc;

 for(chc = 0; chc < 6; chc++)
 {
  psg_channel *ch = &psg.channel[chc];
  int disabled = ((ch->control & 0x80)^0x80) >> 7;
  if(disabled) 
   continue; 
  #include "psg-loop.h"
 }
}

void SoundBox_Update(void)
{
 int32 timestamp = v810_timestamp / 3;
 int32 run_time = timestamp - psg.lastts;

 if(!FSettings.SndRate) return;

 if(timestamp == psg.lastts) return;

 int lfo_on = psg.lfoctrl & 0x03;
 if(!(psg.channel[1].control & 0x80))
  lfo_on = 0;

 if(!(psg.channel[0].control & 0x80))
  lfo_on = 0;

 if(psg.lfoctrl & 0x80)
  lfo_on = 0;

 if(lfo_on)
  DoLFOUpdate(run_time, timestamp);
 else
  DoNormalUpdate(run_time, timestamp);
 
 psg.lastts = timestamp;
}

int16 *SoundBox_Flush(int32 *len)
{
 int32 timestamp;

 if(FSettings.SndRate);
  SoundBox_Update();

 
 DoADPCMUpdate();
 SCSICD_Run(v810_timestamp);
 FXTIMER_Update();
 FXINPUT_Update();

 timestamp = v810_timestamp / 3;
 if(FSettings.SndRate)
 {
  int love;
  for(int y = 0; y < 2; y++)
  {
   FXsbuf[y].end_frame(timestamp);
   love = FXsbuf[y].read_samples(&psg.WaveIL[y], psg.WaveFinalLen, 1);
  }
  *len = love;
 }

 KING_ResetTS(); // Call it before setting v810_timestamp to 0

 pcfx_timestamp_base += v810_timestamp;
 v810_timestamp = 0;

 psg.lastts = 0;
 adpcm_lastts = 0;

 SCSICD_ResetTS();
 FXTIMER_ResetTS();
 FXINPUT_ResetTS();
 
 if(!FSettings.SndRate)
 {
  *len = 0;
  return(NULL);
 }
 return(psg.WaveIL);
}

void SoundBox_Reset(void)
{
 memset(psg.channel, 0, sizeof(psg.channel));
 psg.select = 0;
 psg.globalbalance = 0;
 psg.lfofreq = 0;
 psg.lfoctrl = 0;
 psg.lfo_counter = 0;

 psg.ADPCMControl = 0;
 memset(psg.ADPCMVolume, 0, sizeof(psg.ADPCMVolume));
 memset(psg.CDDAVolume, 0, sizeof(psg.CDDAVolume));
 memset(psg.ADPCMCurrent, 0, sizeof(psg.ADPCMCurrent));
 memset(psg.ADPCMEstim, 0, sizeof(psg.ADPCMEstim));
 memset(psg.ADPCMWhichNibble, 0, sizeof(psg.ADPCMWhichNibble));
 memset(psg.ADPCMHalfWord, 0, sizeof(psg.ADPCMHalfWord));
 memset(psg.ADPCMHaveHalfWord, 0, sizeof(psg.ADPCMHaveHalfWord));

 RedoADPCMVolume(0x03, 0x03);
 SCSICD_SetCDDAVolume(0.50f * psg.CDDAVolume[0] / 63, 0.50f * psg.CDDAVolume[1] / 63);

 psg.bigdiv = (1365 - 85 * 4) * 2; //1365 * 2 / 2;
 psg.smalldiv = 0;
 v810_setevent(V810_EVENT_ADPCM, (psg.bigdiv + 1) / 2);
}

int SoundBox_StateAction(StateMem *sm, int load, int data_only)
{
 int ret = 1;

 for(int ch = 0; ch < 6; ch++)
 {
  char tmpstr[5] = "SCHx";
  psg_channel *pt = &psg.channel[ch];

  SFORMAT CH_StateRegs[] = 
  {
   SFVARN(pt->counter, "counter"),
   SFVARN(pt->frequency, "frequency"),
   SFVARN(pt->control, "control"),
   SFVARN(pt->balance, "balance"),
   SFARRAYN(pt->waveform, 32, "waveform"),
   SFVARN(pt->waveform_index, "waveform_index"),
   SFVARN(pt->dda, "dda"),
   SFVARN(pt->noisectrl, "noisectrl"),
   SFVARN(pt->noisecount, "noisecount"),
   SFVARN(pt->lfsr, "lfsr"),
   SFEND
  };
  tmpstr[3] = '0' + ch;
  ret &= MDFNSS_StateAction(sm, load, data_only, CH_StateRegs, tmpstr);
 }

 SFORMAT SoundBox_StateRegs[] =
 {
  SFVARN(psg.select, "select"),
  SFVARN(psg.globalbalance, "globalbalance"),
  SFVARN(psg.lfofreq, "lfofreq"),
  SFVARN(psg.lfoctrl, "lfoctrl"),

  SFVARN(psg.ADPCMControl, "ADPCMControl"),
  SFARRAY32N(psg.ADPCMCurrent, 2 * 2, "ADPCMCurrent"),
  SFARRAY32N(psg.ADPCMEstim, 2, "ADPCMEstim"),
  SFARRAY32N(psg.ADPCMWhichNibble, 2, "ADPCMWNibble"),
  SFARRAY16N(psg.ADPCMHalfWord, 2, "ADPCMHalfWord"),
  SFARRAYN(psg.ADPCMHaveHalfWord, 2, "ADPCMHHW"),

  SFARRAYN(psg.ADPCMVolume, 2 * 2, "ADPCMVolume"),
  SFVARN(psg.bigdiv, "bigdiv"),
  SFVARN(psg.smalldiv, "smalldiv"),

  SFARRAYN(psg.CDDAVolume, 2, "CDDAVolume"),
  SFEND
 };
 
 ret &= MDFNSS_StateAction(sm, load, data_only, SoundBox_StateRegs, "SBOX");

 if(load)
 {
  psg.lmal = 0x1F - scale_tab[(psg.globalbalance >> 4) & 0xF];
  psg.rmal = 0x1F - scale_tab[(psg.globalbalance >> 0) & 0xF];

  for(int ch = 0; ch < 6; ch++)
  {
   psg.channel[ch].lal = 0x1F - scale_tab[(psg.channel[ch].balance >> 4) & 0xF];
   psg.channel[ch].ral = 0x1F - scale_tab[(psg.channel[ch].balance >> 0) & 0xF];
   psg.channel[ch].al = 0x1F - (psg.channel[ch].control & 0x1F);
   redo_ddacache(&psg.channel[ch]);
  }
  RedoADPCMVolume(0x03, 0x03);
  SCSICD_SetCDDAVolume(0.50f * psg.CDDAVolume[0] / 63, 0.50f * psg.CDDAVolume[1] / 63);

  v810_setevent(V810_EVENT_ADPCM, (psg.bigdiv + 1) / 2);
 }
 return(ret); 
}
