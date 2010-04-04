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
#include "pce.h"
#include "psg.h"
#include "huc.h"
#include "cdrom.h"
#include "input.h"
#include <trio/trio.h>

static Blip_Synth<blip_good_quality, 4096> WaveSynth;
static Blip_Synth<blip_med_quality, 4096> NoiseSynth;

Blip_Buffer sbuf[2];		// Used in the CDROM ADPCM code
t_psg psg;
void PSG_Update(void);

void PSG_SetSoundMultiplier(double multiplier)
{
 for(int y = 0; y < 2; y++)
 {
  sbuf[y].clock_rate((long)(1789772.727272 * 4 * multiplier));
 }
}

static void RedoVolume(void)
{
 if(PCE_IsCD)
 {
  WaveSynth.volume((double)FSettings.SoundVolume / 100 * 0.227 * 0.50 * psg.cdpsgvolume / 100);
  NoiseSynth.volume((double)FSettings.SoundVolume / 100 * 0.227 * 0.50 * psg.cdpsgvolume / 100);
 }
 else
 {
  WaveSynth.volume((double)FSettings.SoundVolume / 100 / 6);
  NoiseSynth.volume((double)FSettings.SoundVolume / 100 / 6);
 }
}

void PSG_SetSoundVolume(uint32 volume)
{
 RedoVolume();
}

void PSG_Sound(int rate)
{
 for(int y = 0; y < 2; y++)
 {
  sbuf[y].set_sample_rate(rate ? rate : 44100, 50);
  sbuf[y].clock_rate((long)(1789772.727272 * 4));
  sbuf[y].bass_freq(20);
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

#define CLOCK_LFSR(lfsr) { unsigned int newbit = ((lfsr >> 0) ^ (lfsr >> 1) ^ (lfsr >> 11) ^ (lfsr >> 12) ^ (lfsr >> 17)) & 1; lfsr = (lfsr >> 1) | (newbit << 17); }


#define redo_ddacache(ch)	\
{	\
 int vll,vlr;	\
 vll = (ch)->lal + (ch)->al + psg.lmal;	\
 vlr = (ch)->ral + (ch)->al + psg.rmal;	\
 (ch)->dda_cache[0] = dbtable[(ch)->dda][vll];	\
 (ch)->dda_cache[1] = dbtable[(ch)->dda][vlr];	\
}

#ifdef WANT_DEBUGGER
void PSG_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 int which_chan;

 if(trio_sscanf(name, "psgram%d", &which_chan) == 1)
 {
  while(Length--)
  {
   Address &= 0x1F;
   *Buffer = psg.channel[which_chan].waveform[Address];
   Address++;
   Buffer++;
  }
 }
}

void PSG_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 int which_chan;

 if(trio_sscanf(name, "psgram%d", &which_chan) == 1)
 {
  while(Length--)
  {
   Address &= 0x1F;
   psg.channel[which_chan].waveform[Address] = *Buffer & 0x1F;
   Address++;
   Buffer++;
  }
 }
}

#define CHPDMOO(n)      \
 { "CH"#n"Freq", "PSG Ch"#n" Frequency(Period)", 2 },   \
 { "CH"#n"Ctrl", "PSG Ch"#n" Control", 1 },     \
 { "CH"#n"Balance", "PSG Ch"#n" Balance", 1 },  \
 { "CH"#n"WIndex", "PSG Ch"#n" Waveform Index", 1},     \
 { "CH"#n"SCache", "PSG Ch"#n" Sample Cache", 1 }

static RegType PSGRegs[] =
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

 { "", "", 0 },
};

static uint32 PSGDBG_GetRegister(const std::string &name, std::string *special)
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

static void PSGDBG_SetRegister(const std::string &name, uint32 V)
{
 PSG_Update();

 if(name == "Select")
  PSG_Write(0x00, V);
 else if(name == "GBalance")
  PSG_Write(0x01, V);
 else if(name == "LFOFreq")
 {
  PSG_Write(0x08, V);
 }
 else if(name == "LFOCtrl")
  PSG_Write(0x09, V);
 else if(!strncmp(name.c_str(), "CH", 2))
 {
  unsigned int psg_sel_save = psg.select;
  int ch = name[2] - '0';
  char moomoo[64];
  strncpy(moomoo, name.c_str() + 3, 63);

  PSG_Write(0x00, ch);

  if(!strcmp(moomoo, "Freq"))
  {
   PSG_Write(0x02, V);
   PSG_Write(0x03, V >> 8);
  }
  else if(!strcmp(moomoo, "Ctrl"))
   PSG_Write(0x04, V);
  else if(!strcmp(moomoo, "Balance"))
   PSG_Write(0x05, V);
  else if(!strcmp(moomoo, "WIndex"))
   psg.channel[ch].waveform_index = V & 0x1F;
  else if(!strcmp(moomoo, "SCache"))
  {
   psg.channel[ch].dda = V & 0x1F;
   redo_ddacache(&psg.channel[ch]);
  }
  else if(!strcmp(moomoo, "NCtrl") && ch < 4)
   psg.channel[ch].noisectrl = V;
  else if(!strcmp(moomoo, "LFSR") && ch < 4)
   psg.channel[ch].lfsr = V & 0x3FFFF;

  PSG_Write(0x00, psg_sel_save);
 }
}

static RegGroupType PSGRegsGroup =
{
 PSGRegs,
 PSGDBG_GetRegister,
 PSGDBG_SetRegister
};


#endif

int PSG_Init(bool WantMono)
{
    memset(&psg, 0, sizeof(psg));
    if(FSettings.SndRate)
     PSG_Sound(FSettings.SndRate);

    for(int x=0; x < 32 * 3; x++)
    {
     int y;
     double flub;

     flub = 1;

     if(x)
      flub /= pow(2, (double)1/4*x); //4*x);		// ~1.5dB reduction per increment of x
     for(y=0;y<32;y++)
      dbtable[y][x] = (int32)(flub * (y - 0x10) * 128);
     //printf("%02x: %f\n", 0x1F - x, flub);
    }
    
    psg.forcemono = WantMono;
    psg.cdpsgvolume = MDFN_GetSettingUI("pce.cdpsgvolume");

    if(psg.cdpsgvolume != 100)
    {
     MDFN_printf(_("CD PSG Volume: %d%%\n"), psg.cdpsgvolume);
    }

    #ifdef WANT_DEBUGGER
    for(int x = 0; x < 6; x++)
    {
     char tmpname[128], tmpinfo[128];

     trio_snprintf(tmpname, 128, "psgram%d", x);
     trio_snprintf(tmpinfo, 128, "PSG Ch %d RAM", x + 1);

     MDFNDBG_AddASpace(PSG_GetAddressSpaceBytes, PSG_PutAddressSpaceBytes, tmpname, tmpinfo, 5);
    }
    MDFNDBG_AddRegGroup(&PSGRegsGroup);
    #endif

    return (1);
}

void PSG_Kill(void)
{
	if(psg.WaveIL)
	 free(psg.WaveIL);
	psg.WaveIL = NULL;
}

/*--------------------------------------------------------------------------*/
/* PSG emulation                                                            */
/*--------------------------------------------------------------------------*/

/* Macro to access currently selected PSG channel */
#define PSGCH   psg.channel[psg.select]

DECLFW(PSG_Write)
{
    int x;

    A &= 0x0F;

    if(A == 0x00)
     psg.select = (V & 0x07);
    else
     PSG_Update();

    switch(A)
    {
        case 0x01: /* Global sound balance */
            psg.globalbalance = V;
	    psg.lmal = 0x1F - scale_tab[(psg.globalbalance >> 4) & 0xF];
	    psg.rmal = 0x1F - scale_tab[(psg.globalbalance >> 0) & 0xF];
	    for(x=0;x<6;x++)
	     redo_ddacache(&psg.channel[x]);
            break;

        case 0x02: /* Channel frequency (LSB) */
	    if(psg.select > 5) return; // no more than 6 channels, silly game.
            PSGCH.frequency = (PSGCH.frequency & 0x0F00) | V;
	    PSGCH.base_frequency = PSGCH.frequency;
            break;

        case 0x03: /* Channel frequency (MSB) */
	    if(psg.select > 5) return; // no more than 6 channels, silly game.
            PSGCH.frequency = (PSGCH.frequency & 0x00FF) | ((V & 0x0F) << 8);
            PSGCH.base_frequency = PSGCH.frequency;
            break;

        case 0x04: /* Channel enable, DDA, volume */
	    if(psg.select > 5) return; // no more than 6 channels, silly game.

            if((PSGCH.control & 0x40) && !(V & 0x40)) PSGCH.waveform_index = 0;

            PSGCH.control = V;
	    PSGCH.al = 0x1F - (PSGCH.control & 0x1F);
            redo_ddacache(&PSGCH);
            break;

        case 0x05: /* Channel balance */
	    if(psg.select > 5) return; // no more than 6 channels, silly game.
            PSGCH.balance = V;
	    PSGCH.lal = 0x1F - scale_tab[(V >> 4) & 0xF];
	    PSGCH.ral = 0x1F - scale_tab[(V >> 0) & 0xF];
            redo_ddacache(&PSGCH);
            break;

        case 0x06: /* Channel waveform data */
            if(psg.select > 5) return; // no more than 6 channels, silly game.
            V &= 0x1F;

            if(!(PSGCH.control & 0x40))
             PSGCH.waveform[PSGCH.waveform_index] = V;

            if((PSGCH.control & 0xC0) == 0x00)
             PSGCH.waveform_index = ((PSGCH.waveform_index + 1) & 0x1F);

	    if(PSGCH.control & 0x80)
             PSGCH.dda = V;

            redo_ddacache(&PSGCH);
            break;

        case 0x07: /* Noise enable and frequency */
	    if(psg.select > 5) return; // no more than 6 channels, silly game.
            if(psg.select >= 4) PSGCH.noisectrl = V;
            break;

        case 0x08: /* LFO frequency */
            psg.lfofreq = V;
            break;

        case 0x09: /* LFO trigger and control */
	    if((V & 0x80) && !(psg.lfoctrl & 0x80))
	     psg.channel[1].waveform_index = 0;
            psg.lfoctrl = V;
            break;
    }

 if(A)
  PSG_Update();
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
 {
  #define LFO_ON
  chc = 0;
  psg_channel *ch = &psg.channel[0];
  #include "psg-loop.h"
  #undef LFO_ON
 }
 // Update the noise channel LFSRs if they weren't updated before
 for(chc = 4; chc < 6; chc++)
 {
  psg_channel *ch = &psg.channel[chc];

  // If channel is disabled, or noise is not selected.
  if((ch->control & 0xC0) != 0x80 || !(ch->noisectrl & 0x80))
  {
   int32 freq = 0x1F - (ch->noisectrl & 0x1F);
   if(!freq)
    freq = 0x20;
   else
    freq <<= 6;
   freq <<= 1;

   ch->noisecount -= run_time;
   while(ch->noisecount <= 0)
   {
    CLOCK_LFSR(ch->lfsr);
    ch->noisecount += freq;
   }
  }
 }
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

 // Update the noise channel LFSRs if they weren't updated before
 for(chc = 4; chc < 6; chc++)
 {
  psg_channel *ch = &psg.channel[chc];

  // If channel is disabled, or noise is not selected.
  if((ch->control & 0xC0) != 0x80 || !(ch->noisectrl & 0x80))
  {
   int32 freq = 0x1F - (ch->noisectrl & 0x1F);
   if(!freq)
    freq = 0x20;
   else
    freq <<= 6;
   freq <<= 1;

   ch->noisecount -= run_time;
   while(ch->noisecount <= 0)
   {
    CLOCK_LFSR(ch->lfsr);
    ch->noisecount += freq;
   }
  }
 }
}

void PSG_Update(void)
{
 int32 timestamp = HuCPU.timestamp / pce_overclocked;
 int32 run_time = timestamp - psg.lastts;

 if(!FSettings.SndRate) return;

 //if(timestamp == psg.lastts) return;

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

int16 *PSG_Flush(int32 *len)
{
 int32 timestamp;

 if(FSettings.SndRate);
 {
  PSG_Update();
  if(PCE_IsCD)
   PCECD_Run();
 }
 timestamp = HuCPU.timestamp / pce_overclocked;
 if(FSettings.SndRate)
 {
  int love;
  for(int y = 0; y < 2; y++)
  {
   sbuf[y].end_frame(timestamp);
   love = sbuf[y].read_samples(&psg.WaveIL[y], psg.WaveFinalLen, 1);
  }
  if(psg.forcemono)
  {
   for(int x = 0; x < love; x++)
    psg.WaveIL[x] = ((int32)psg.WaveIL[x * 2] + psg.WaveIL[x * 2 + 1]) >> 1;
  }
  *len = love;
 }
 if(PCE_IsCD)
  RedoVolume();

 INPUT_FixTS();

 HuCPU.timestamp = 0;
 psg.lastts = 0;

 if(PCE_IsCD)
 {
  PCECD_ResetTS();
 }

 if(!FSettings.SndRate)
 {
  *len = 0;
  return(NULL);
 }

 return(psg.WaveIL);
}

void PSG_Power(void)
{
 memset(psg.channel, 0, sizeof(psg.channel));
 psg.select = 0;
 psg.globalbalance = 0;
 psg.lfofreq = 0;
 psg.lfoctrl = 0;
 psg.lfo_counter = 0;

 psg.channel[4].lfsr = 1;
 psg.channel[5].lfsr = 1;
}

int PSG_StateAction(StateMem *sm, int load, int data_only)
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

 SFORMAT PSG_StateRegs[] =
 {
  SFVARN(psg.select, "select"),
  SFVARN(psg.globalbalance, "globalbalance"),
  SFVARN(psg.lfofreq, "lfofreq"),
  SFVARN(psg.lfoctrl, "lfoctrl"),
  SFEND
 };
 
 ret &= MDFNSS_StateAction(sm, load, data_only, PSG_StateRegs, "PSG");

 if(load)
 {
  psg.lmal = 0x1F - scale_tab[(psg.globalbalance >> 4) & 0xF];
  psg.rmal = 0x1F - scale_tab[(psg.globalbalance >> 0) & 0xF];

  if(!psg.channel[4].lfsr)
   psg.channel[4].lfsr = 1;

  if(!psg.channel[5].lfsr)
   psg.channel[5].lfsr = 1;

  for(int ch = 0; ch < 6; ch++)
  {
   psg.channel[ch].lal = 0x1F - scale_tab[(psg.channel[ch].balance >> 4) & 0xF];
   psg.channel[ch].ral = 0x1F - scale_tab[(psg.channel[ch].balance >> 0) & 0xF];
   psg.channel[ch].al = 0x1F - (psg.channel[ch].control & 0x1F);
   redo_ddacache(&psg.channel[ch]);
  }
 }
 return(ret); 
}
