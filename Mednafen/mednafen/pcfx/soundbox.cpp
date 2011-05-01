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

#include "pcfx.h"
#include "soundbox.h"
#include "king.h"
#include "../cdrom/scsicd.h"
#include "pce_psg/pce_psg.h"
#include "../clamp.h"

#include <math.h>
#include <string.h>

static const int StepSizes[49] =
{
 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50,
 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157,
 173, 190,  209, 230, 253, 279, 307, 337, 371, 408, 449,
 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552
};

static const int StepIndexDeltas[16] =
{
 -1, -1, -1, -1, 2, 4, 6, 8,
 -1, -1, -1, -1, 2, 4, 6, 8
};


typedef Blip_Synth<blip_good_quality, 4096> ADSynth;
static ADSynth ADPCMSynth;

Blip_Buffer FXsbuf[2];		// Used in the CDROM code

static PCE_PSG *pce_psg = NULL;


static bool SoundEnabled;
static uint32 adpcm_lastts;

static t_soundbox psg;
static double ADPCMVolTable[0x40];

static bool EmulateBuggyCodec;		// If true, emulate the buggy codec/algorithm used by an official PC-FX ADPCM encoder, rather than how the
					// hardware actually works.
static bool ResetAntiClickEnabled;	// = true;


#ifdef WANT_DEBUGGER


enum
{
 GSREG_ADPCM_CTRL = _PSG_GSREG_COUNT,
 GSREG_ADPCM0_LVOL,
 GSREG_ADPCM0_RVOL,

 GSREG_ADPCM1_LVOL,
 GSREG_ADPCM1_RVOL,

 GSREG_ADPCM0_CUR,
 GSREG_ADPCM1_CUR,

 GSREG_CDDA_LVOL,
 GSREG_CDDA_RVOL
};

#define CHPDMOO(n)      \
 { 0, "--CH"#n"--:", "", 0xFFFF },	\
 { PSG_GSREG_CH0_FREQ | (n << 8), "Freq", "PSG Ch"#n" Frequency(Period)", 2 },   \
 { PSG_GSREG_CH0_CTRL | (n << 8), "Ctrl", "PSG Ch"#n" Control", 1 },     \
 { PSG_GSREG_CH0_BALANCE | (n << 8), "Balance", "PSG Ch"#n" Balance", 1 },  \
 { PSG_GSREG_CH0_WINDEX | (n << 8), "WIndex", "PSG Ch"#n" Waveform Index", 1},     \
 { PSG_GSREG_CH0_SCACHE | (n << 8), "SCache", "PSG Ch"#n" Sample Cache", 1 }

static RegType SBoxRegs[] =
{
 { PSG_GSREG_SELECT, "Select", "PSG Channel Select", 1 },
 { PSG_GSREG_GBALANCE, "GBal", "PSG Global Balance", 1 },
 { PSG_GSREG_LFOFREQ, "LFOFreq", "PSG LFO Freq", 1 },
 { PSG_GSREG_LFOCTRL, "LFOCtrl", "PSG LFO Control", 1 },

 CHPDMOO(0),
 CHPDMOO(1),
 CHPDMOO(2),
 CHPDMOO(3),
 CHPDMOO(4),
 { PSG_GSREG_CH4_NCTRL, "NCtrl", "PSG Ch4 Noise Control", 1 },
 { PSG_GSREG_CH4_LFSR, "LFSR", "PSG Ch4 Noise LFSR", 2 },
 CHPDMOO(5),
 { PSG_GSREG_CH5_NCTRL, "NCtrl", "PSG Ch5 Noise Control", 1 },
 { PSG_GSREG_CH5_LFSR, "LFSR", "PSG Ch5 Noise LFSR", 2 },

 { 0, "--ADPCM:--", "", 0xFFFF },

 { GSREG_ADPCM_CTRL, "Ctrl", "ADPCM Control", 2 },
 { GSREG_ADPCM0_LVOL, "CH0LVol", "ADPCM Ch0 Left Volume", 1 },
 { GSREG_ADPCM0_RVOL, "CH0RVol", "ADPCM Ch0 Right Volume", 1 },
 { GSREG_ADPCM1_LVOL, "CH1LVol", "ADPCM Ch1 Left Volume", 1 },
 { GSREG_ADPCM1_RVOL, "CH1RVol", "ADPCM Ch1 Right Volume", 1 },

 { GSREG_ADPCM0_CUR, "CH0Prc", "ADPCM Ch0 Predictor Value", 2 },
 { GSREG_ADPCM1_CUR, "CH1Prc", "ADPCM Ch1 Predictor Value", 2 },

 { 0, "--CD-DA:--", "", 0xFFFF },
 { GSREG_CDDA_LVOL, "CDLVol", "CD-DA Left Volume", 1 },
 { GSREG_CDDA_RVOL, "CDRVol", "CD-DA Right Volume", 1 },
 { 0, "", "", 0 },
};

static uint32 SBoxDBG_GetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 uint32 value = 0xDEADBEEF;

 switch(id)
 {
  case GSREG_ADPCM_CTRL:
	value = psg.ADPCMControl;
  	if(special)
	{
	 int tmp_freq = 32 / (1 << (value & 0x3));
	 snprintf(special, special_len, "Frequency: ~%dKHz, Ch0 Interpolation: %s, Ch1 Interpolation: %s, Ch0 Reset: %d, Ch1 Reset: %d", tmp_freq, (value & 0x4) ? "On" : "Off", (value & 0x8) ? "On":"Off",
		(int)(bool)(value & 0x10), (int)(bool)(value & 0x20));
	}
	break;

  case GSREG_ADPCM0_LVOL:
  	value = psg.ADPCMVolume[0][0];
	break;

  case GSREG_ADPCM0_RVOL:
        value = psg.ADPCMVolume[0][1];
        break;

  case GSREG_ADPCM1_LVOL:
        value = psg.ADPCMVolume[1][0];
        break;

  case GSREG_ADPCM1_RVOL:
        value = psg.ADPCMVolume[1][1];
        break;

  case GSREG_CDDA_LVOL:
        value = psg.CDDAVolume[0];
        break;

  case GSREG_CDDA_RVOL:
        value = psg.CDDAVolume[1];
        break;

  case GSREG_ADPCM0_CUR:
	value = psg.ADPCMPredictor[0] + 0x4000;
	break;

  case GSREG_ADPCM1_CUR:
	value = psg.ADPCMPredictor[1] + 0x4000;
	break;

  default:
	value = pce_psg->GetRegister(id, special, special_len);
	break;
 }
 return(value);
}


static RegGroupType SBoxRegsGroup =
{
 "SndBox",
 SBoxRegs,
 SBoxDBG_GetRegister,
 NULL
};


#endif

static void RedoVolume(void)
{
 pce_psg->SetVolume(0.681);	//0.227 * 0.50);
 ADPCMSynth.volume(0.50);
}

bool SoundBox_SetSoundRate(uint32 rate)
{
 SoundEnabled = (bool)rate;

 for(int y = 0; y < 2; y++)
 {
  FXsbuf[y].set_sample_rate(rate ? rate : 44100, 50);
  FXsbuf[y].clock_rate((long)(1789772.727272 * 4));
  FXsbuf[y].bass_freq(20);
 }

 RedoVolume();

 return(TRUE);
}

int SoundBox_Init(bool arg_EmulateBuggyCodec, bool arg_ResetAntiClickEnabled)
{
    int x;

    SoundEnabled = false;

    EmulateBuggyCodec = arg_EmulateBuggyCodec;
    ResetAntiClickEnabled = arg_ResetAntiClickEnabled;


    pce_psg = new PCE_PSG(&FXsbuf[0], &FXsbuf[1], PCE_PSG::REVISION_HUC6280A);

    #ifdef WANT_DEBUGGER
    MDFNDBG_AddRegGroup(&SBoxRegsGroup);
    #endif

    memset(&psg, 0, sizeof(psg));

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
      ADPCMVolTable[vti] = flub / 8;
    }

    return (1);
}

void SoundBox_Kill(void)
{
 if(pce_psg)
 {
  delete pce_psg;
  pce_psg = NULL;
 }
}

/* Macro to access currently selected PSG channel */
void SoundBox_Write(uint32 A, uint16 V, const v810_timestamp_t timestamp)
{
    A &= 0x3F;

    if(A < 0x20)
    {
     pce_psg->Write(timestamp / 3, A >> 1, V);
    }
    else
    {
     //printf("%04x %04x %d\n", A, V, timestamp);
     switch(A & 0x3F)
     {
	//default: printf("HARUM: %04x %04x\n", A, V); break;
	case 0x20: SoundBox_ADPCMUpdate(timestamp);
		   for(int ch = 0; ch < 2; ch++)
		   {
		    if(!(psg.ADPCMControl & (0x10 << ch)) && (V & (0x10 << ch)))
		    {
		     //printf("Reset: %d\n", ch);

		     if(ResetAntiClickEnabled)
		     {
		      psg.ResetAntiClick[ch] += (int64)psg.ADPCMPredictor[ch] << 32;
		      if(psg.ResetAntiClick[ch] > ((int64)0x3FFF << 32))
		       psg.ResetAntiClick[ch] = (int64)0x3FFF << 32;
		      if(psg.ResetAntiClick[ch] < ((int64)-0x4000 << 32))
		       psg.ResetAntiClick[ch] = (int64)-0x4000 << 32;
		     }

		     psg.ADPCMPredictor[ch] = 0;
		     psg.StepSizeIndex[ch] = 0;
		    }
		   }
		   psg.ADPCMControl = V; 
		   break;

	case 0x22: SoundBox_ADPCMUpdate(timestamp);
		   psg.ADPCMVolume[0][0] = V & 0x3F; 
		   break;

	case 0x24: SoundBox_ADPCMUpdate(timestamp);
		   psg.ADPCMVolume[0][1] = V & 0x3F;
		   break;

	case 0x26: SoundBox_ADPCMUpdate(timestamp);
		   psg.ADPCMVolume[1][0] = V & 0x3F;
		   break;

	case 0x28: SoundBox_ADPCMUpdate(timestamp);
		   psg.ADPCMVolume[1][1] = V & 0x3F; 
		   break;

	case 0x2A: psg.CDDAVolume[0] = V & 0x3F;
		   SCSICD_SetCDDAVolume(0.50f * psg.CDDAVolume[0] / 63, 0.50f * psg.CDDAVolume[1] / 63);
		   break;

	case 0x2C: psg.CDDAVolume[1] = V & 0x3F;
		   SCSICD_SetCDDAVolume(0.50f * psg.CDDAVolume[0] / 63, 0.50f * psg.CDDAVolume[1] / 63);
		   break;
    }
   }
}

static uint32 KINGADPCMControl;

void SoundBox_SetKINGADPCMControl(uint32 value)
{
 KINGADPCMControl = value;
}

/* Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
   Command line: /www/usr/fisher/helpers/mkfilter -Bu -Lp -o 1 -a 1.5888889125e-04 0.0000000000e+00 -l */
static void DoVolumeFilter(int ch, int lr)
{
 psg.vf_xv[ch][lr][0] = psg.vf_xv[ch][lr][1]; 
 psg.vf_xv[ch][lr][1] = (double)ADPCMVolTable[psg.ADPCMVolume[ch][lr]] / 2.004348738e+03;

 psg.vf_yv[ch][lr][0] = psg.vf_yv[ch][lr][1]; 
 psg.vf_yv[ch][lr][1] = (psg.vf_xv[ch][lr][0] + psg.vf_xv[ch][lr][1]) + (  0.9990021696 * psg.vf_yv[ch][lr][0]);
 psg.VolumeFiltered[ch][lr] = psg.vf_yv[ch][lr][1];
}

v810_timestamp_t SoundBox_ADPCMUpdate(const v810_timestamp_t timestamp)
{
 int32 run_time = timestamp - adpcm_lastts;

 adpcm_lastts = timestamp;

 psg.bigdiv -= run_time * 2;

 while(psg.bigdiv <= 0)
 {
  psg.smalldiv--;
  while(psg.smalldiv <= 0)
  {
   psg.smalldiv += 1 << ((KINGADPCMControl >> 2) & 0x3);
   for(int ch = 0; ch < 2; ch++)
   {
    // Keep playing our last halfword fetched even if KING ADPCM is disabled
    if(psg.ADPCMHaveHalfWord[ch] || KINGADPCMControl & (1 << ch)) 
    {
     if(!psg.ADPCMWhichNibble[ch])
     {
      psg.ADPCMHalfWord[ch] = KING_GetADPCMHalfWord(ch);
      psg.ADPCMHaveHalfWord[ch] = TRUE;
     }

     // If the channel's reset bit is set, don't update its ADPCM state.
     if(psg.ADPCMControl & (0x10 << ch))
     {
      psg.ADPCMDelta[ch] = 0;
     }
     else
     {
      uint8 nibble = (psg.ADPCMHalfWord[ch] >> (psg.ADPCMWhichNibble[ch])) & 0xF;
      int32 BaseStepSize = StepSizes[psg.StepSizeIndex[ch]];

      //if(!ch)
      //printf("Nibble: %02x\n", nibble);

      if(EmulateBuggyCodec)
      {
       if(BaseStepSize == 1552)
        BaseStepSize = 1522;

       psg.ADPCMDelta[ch] = BaseStepSize * ((nibble & 0x7) + 1) * 2;
      }
      else
       psg.ADPCMDelta[ch] = BaseStepSize * ((nibble & 0x7) + 1);

      // Linear interpolation turned on?
      if(psg.ADPCMControl & (0x4 << ch))
       psg.ADPCMDelta[ch] >>= (KINGADPCMControl >> 2) & 0x3;

      if(nibble & 0x8)
       psg.ADPCMDelta[ch] = -psg.ADPCMDelta[ch];

      psg.StepSizeIndex[ch] += StepIndexDeltas[nibble];

      if(psg.StepSizeIndex[ch] < 0)
       psg.StepSizeIndex[ch] = 0;

      if(psg.StepSizeIndex[ch] > 48)
       psg.StepSizeIndex[ch] = 48;
     }
     psg.ADPCMHaveDelta[ch] = 1;

     // Linear interpolation turned on?
     if(psg.ADPCMControl & (0x4 << ch))
      psg.ADPCMHaveDelta[ch] = 1 << ((KINGADPCMControl >> 2) & 0x3);

     psg.ADPCMWhichNibble[ch] = (psg.ADPCMWhichNibble[ch] + 4) & 0xF;

     if(!psg.ADPCMWhichNibble[ch])
      psg.ADPCMHaveHalfWord[ch] = FALSE;
    }
   } // for(int ch...)
  } // while(psg.smalldiv <= 0)

  for(int ch = 0; ch < 2; ch++)
  {
   //static int32 last_synthtime = 0;
   //uint32 synthtime = (timestamp + psg.bigdiv / 2) / 3;
   // This is as correct as the divide(I think), but may be faster(or maybe not?)
   uint32 synthtime = (timestamp - ((0 - psg.bigdiv) >> 1)) / 3;

   //if(!ch)
   //{
   // printf("%d\n", synthtime - last_synthtime);
   // last_synthtime = synthtime;
   //}

   if(psg.ADPCMHaveDelta[ch]) 
   {
    psg.ADPCMPredictor[ch] += psg.ADPCMDelta[ch];

    psg.ADPCMHaveDelta[ch]--;

    if(psg.ADPCMPredictor[ch] > 0x3FFF) { psg.ADPCMPredictor[ch] = 0x3FFF; /*printf("Overflow: %d\n", ch);*/ }
    if(psg.ADPCMPredictor[ch] < -0x4000) { psg.ADPCMPredictor[ch] = -0x4000; /*printf("Underflow: %d\n", ch);*/ }
   }
   else
   {

   }

   if(SoundEnabled)
   {
    int32 samp[2];

    if(EmulateBuggyCodec)
    {
     samp[0] = (int32)(((psg.ADPCMPredictor[ch] >> 1) + (psg.ResetAntiClick[ch] >> 33)) * psg.VolumeFiltered[ch][0]);
     samp[1] = (int32)(((psg.ADPCMPredictor[ch] >> 1) + (psg.ResetAntiClick[ch] >> 33)) * psg.VolumeFiltered[ch][1]);
    }
    else
    {
     samp[0] = (int32)((psg.ADPCMPredictor[ch] + (psg.ResetAntiClick[ch] >> 32)) * psg.VolumeFiltered[ch][0]);
     samp[1] = (int32)((psg.ADPCMPredictor[ch] + (psg.ResetAntiClick[ch] >> 32)) * psg.VolumeFiltered[ch][1]);
    }
    ADPCMSynth.offset(synthtime, samp[0] - psg.ADPCM_last[ch][0], &FXsbuf[0]);
    ADPCMSynth.offset(synthtime, samp[1] - psg.ADPCM_last[ch][1], &FXsbuf[1]);

    psg.ADPCM_last[ch][0] = samp[0];
    psg.ADPCM_last[ch][1] = samp[1];
   }
  }

  for(int ch = 0; ch < 2; ch++)
  {
   psg.ResetAntiClick[ch] -= psg.ResetAntiClick[ch] >> 8;
   //if(ch)
   // MDFN_DispMessage("%d", (int)(psg.ResetAntiClick[ch] >> 32));
  }

  for(int ch = 0; ch < 2; ch++)
   for(int lr = 0; lr < 2; lr++)
   {
    DoVolumeFilter(ch, lr);
   }
  psg.bigdiv += 1365 * 2 / 2;
 }


 assert((psg.bigdiv + 1) / 2);
 return(timestamp + (psg.bigdiv + 1) / 2);
}

int32 SoundBox_Flush(const uint32 v810_timestamp, int16 *SoundBuf, const int32 MaxSoundFrames)
{
 int32 timestamp;
 int32 FrameCount = 0;

 timestamp = v810_timestamp / 3;

 pce_psg->EndFrame(timestamp);

 if(SoundEnabled)
 {
  for(int y = 0; y < 2; y++)
  {
   FXsbuf[y].end_frame(timestamp);
   FrameCount = FXsbuf[y].read_samples(SoundBuf + y, MaxSoundFrames, 1);
  }
 }

 return(FrameCount);
}

void SoundBox_ResetTS(void)
{
 //assert(adpcm_lastts == PCFX_V810.v810_timestamp);
 adpcm_lastts = 0;
}

void SoundBox_Reset(void)
{
 pce_psg->Power(0);	// FIXME

 psg.ADPCMControl = 0;

 memset(&psg.vf_xv, 0, sizeof(psg.vf_xv));
 memset(&psg.vf_yv, 0, sizeof(psg.vf_yv));

 for(int lr = 0; lr < 2; lr++)
 {
  for(int ch = 0; ch < 2; ch++)
  {
   psg.ADPCMVolume[ch][lr] = 0;
   psg.VolumeFiltered[ch][lr] = 0;
  }

  psg.CDDAVolume[lr] = 0;
 }

 for(int ch = 0; ch < 2; ch++)
 {
  psg.ADPCMPredictor[ch] = 0;
  psg.StepSizeIndex[ch] = 0;
 }

 memset(psg.ADPCMWhichNibble, 0, sizeof(psg.ADPCMWhichNibble));
 memset(psg.ADPCMHalfWord, 0, sizeof(psg.ADPCMHalfWord));
 memset(psg.ADPCMHaveHalfWord, 0, sizeof(psg.ADPCMHaveHalfWord));

 SCSICD_SetCDDAVolume(0.50f * psg.CDDAVolume[0] / 63, 0.50f * psg.CDDAVolume[1] / 63);

 psg.bigdiv = 2;	// TODO: KING->SBOX ADPCM Synch //(1365 - 85 * 4) * 2; //1365 * 2 / 2;
 psg.smalldiv = 0;
 adpcm_lastts = 0;
}

int SoundBox_StateAction(StateMem *sm, int load, int data_only)
{
 int ret = 1;

 ret &= pce_psg->StateAction(sm, load, data_only);

 SFORMAT SoundBox_StateRegs[] =
 {
  SFVARN(psg.ADPCMControl, "ADPCMControl"),
  SFARRAYN(&psg.ADPCMVolume[0][0], 2 * 2, "ADPCMVolume"),
  SFARRAYN(psg.CDDAVolume, 2, "CDDAVolume"),
  SFVARN(psg.bigdiv, "bigdiv"),
  SFVARN(psg.smalldiv, "smalldiv"),

  SFARRAY64N(&psg.ResetAntiClick[0], 2, "ResetAntiClick"),
  SFARRAYDN(&psg.VolumeFiltered[0][0], 2 * 2, "VolumeFiltered"),
  SFARRAYDN(&psg.vf_xv[0][0][0], 2 * 2 * (1 + 1), "vf_xv"),
  SFARRAYDN(&psg.vf_yv[0][0][0], 2 * 2 * (1 + 1), "vf_yv"),

  SFARRAY32N(psg.ADPCMDelta, 2, "ADPCMDelta"),
  SFARRAY32N(psg.ADPCMHaveDelta, 2, "ADPCMHaveDelta"),

  SFARRAY32N(&psg.ADPCMPredictor[0], 2, "ADPCMPredictor"),
  SFARRAY32N(&psg.StepSizeIndex[0], 2, "ADPCMStepSizeIndex"),

  SFARRAY32N(psg.ADPCMWhichNibble, 2, "ADPCMWNibble"),
  SFARRAY16N(psg.ADPCMHalfWord, 2, "ADPCMHalfWord"),
  SFARRAYBN(psg.ADPCMHaveHalfWord, 2, "ADPCMHHW"),

  SFEND
 };
 
 ret &= MDFNSS_StateAction(sm, load, data_only, SoundBox_StateRegs, "SBOX");

 if(load)
 {
  for(int ch = 0; ch < 2; ch++)
  {
   clamp(&psg.ADPCMPredictor[ch], -0x4000, 0x3FFF);
   clamp(&psg.ResetAntiClick[ch], (int64)-0x4000 << 32, (int64)0x3FFF << 32);

   if(!ResetAntiClickEnabled)
    psg.ResetAntiClick[ch] = 0;

   clamp(&psg.StepSizeIndex[ch], 0, 48);

   clamp(&psg.bigdiv, 1, 1365);
   clamp(&psg.smalldiv, 1, 8);

   for(int lr = 0; lr < 2; lr++)
   {

   }
  }
  SCSICD_SetCDDAVolume(0.50f * psg.CDDAVolume[0] / 63, 0.50f * psg.CDDAVolume[1] / 63);
 }
 return(ret); 
}
