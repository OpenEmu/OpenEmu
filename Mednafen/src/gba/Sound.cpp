// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <memory.h>

#include "GBA.h"
#include "Globals.h"
#include "Sound.h"

#include <blip/Blip_Buffer.h>
#include "Gba_Apu.h"
#include <blip/Stereo_Buffer.h>

typedef Blip_Synth<blip_good_quality, 0xFF * 2> Synth;

static Synth synth;
static Gba_Apu gba_apu;
static Stereo_Buffer gba_buf;

extern uint32 soundTS;

int soundControl = 0;

int soundDSFifoAIndex = 0;
int soundDSFifoACount = 0;
int soundDSFifoAWriteIndex = 0;
bool8 soundDSAEnabled = false;
int soundDSATimer = 0;
uint8  soundDSFifoA[32];
uint8 soundDSAValue = 0;

int soundDSFifoBIndex = 0;
int soundDSFifoBCount = 0;
int soundDSFifoBWriteIndex = 0;
bool8 soundDSBEnabled = false;
int soundDSBTimer = 0;
uint8  soundDSFifoB[32];
uint8 soundDSBValue = 0;

#include "../state.h"


int MDFNGBASOUND_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] = 
 {
  SFVAR(soundControl),
  SFVAR(soundDSFifoAIndex),
  SFVAR(soundDSFifoACount),
  SFVAR(soundDSFifoAWriteIndex),
  SFVAR(soundDSAEnabled),
  SFVAR(soundDSATimer),
  {soundDSFifoA, sizeof(soundDSFifoA), "soundDSFifoA" },
  SFVAR(soundDSAValue),
  SFVAR(soundDSFifoBIndex),
  SFVAR(soundDSFifoBCount),
  SFVAR(soundDSFifoBWriteIndex),
  SFVAR(soundDSBEnabled),
  SFVAR(soundDSBTimer),
  {soundDSFifoB, sizeof(soundDSFifoB), "soundDSFifoB" },
  SFVAR(soundDSBValue),

 {&gba_apu.regs, sizeof(gba_apu.regs), "REGS"},

 {&gba_apu.square1.phase, sizeof(int) | MDFNSTATE_RLSB, "s1PH" },
 {&gba_apu.square1.duty, sizeof(int) | MDFNSTATE_RLSB, "s1DU" },

 {&gba_apu.square1.sweep_period, sizeof(int) | MDFNSTATE_RLSB, "s1SP" },
 {&gba_apu.square1.sweep_delay, sizeof(int) | MDFNSTATE_RLSB, "s1SweepDelay" },
 {&gba_apu.square1.sweep_shift, sizeof(int) | MDFNSTATE_RLSB, "s1SS" },
 {&gba_apu.square1.sweep_dir, sizeof(int) | MDFNSTATE_RLSB, "s1SweepDir" },
 {&gba_apu.square1.sweep_freq, sizeof(int) | MDFNSTATE_RLSB, "s1SF" },

 {&gba_apu.square1.env_period, sizeof(int) | MDFNSTATE_RLSB, "s1EP" },
 {&gba_apu.square1.env_dir, sizeof(int) | MDFNSTATE_RLSB, "s1ED" },
 {&gba_apu.square1.env_delay, sizeof(int) | MDFNSTATE_RLSB, "s1Ed" },

 {&gba_apu.square1.new_env_period, sizeof(int) | MDFNSTATE_RLSB, "s1NP" },
 {&gba_apu.square1.new_env_dir, sizeof(int) | MDFNSTATE_RLSB, "s1ND" },
 {&gba_apu.square1.new_volume, sizeof(int) | MDFNSTATE_RLSB, "s1NV" },

 {&gba_apu.square1.period, sizeof(int) | MDFNSTATE_RLSB, "s1OP" },
 {&gba_apu.square1.volume, sizeof(int) | MDFNSTATE_RLSB, "s1OV" },
 {&gba_apu.square1.frequency, sizeof(int) | MDFNSTATE_RLSB, "s1OF" },
 {&gba_apu.square1.length, sizeof(int) | MDFNSTATE_RLSB, "s1OL" },
 {&gba_apu.square1.new_length, sizeof(int) | MDFNSTATE_RLSB, "s1Ol" },
 {&gba_apu.square1.ME, sizeof(int) | MDFNSTATE_RLSB, "s1OM" },
 {&gba_apu.square1.enabled, sizeof(bool8) | MDFNSTATE_RLSB, "s1OE" },
 {&gba_apu.square1.length_enabled, sizeof(bool8) | MDFNSTATE_RLSB, "s1Oe" },
 {&gba_apu.square1.output_select, sizeof(int) | MDFNSTATE_RLSB, "s1OS" },


 {&gba_apu.square2.phase, sizeof(int) | MDFNSTATE_RLSB, "s2PH" },
 {&gba_apu.square2.duty, sizeof(int) | MDFNSTATE_RLSB, "s2DU" },

 {&gba_apu.square2.sweep_period, sizeof(int) | MDFNSTATE_RLSB, "s2SP" },
 {&gba_apu.square2.sweep_delay, sizeof(int) | MDFNSTATE_RLSB, "s2SweepDelay" },
 {&gba_apu.square2.sweep_shift, sizeof(int) | MDFNSTATE_RLSB, "s2SS" },
 {&gba_apu.square2.sweep_dir, sizeof(int) | MDFNSTATE_RLSB, "s2SweepDir" },
 {&gba_apu.square2.sweep_freq, sizeof(int) | MDFNSTATE_RLSB, "s2SF" },
 {&gba_apu.square2.env_period, sizeof(int) | MDFNSTATE_RLSB, "s2EP" },
 {&gba_apu.square2.env_dir, sizeof(int) | MDFNSTATE_RLSB, "s2ED" },
 {&gba_apu.square2.env_delay, sizeof(int) | MDFNSTATE_RLSB, "s2Ed" },

 {&gba_apu.square2.new_env_period, sizeof(int) | MDFNSTATE_RLSB, "s2NP" },
 {&gba_apu.square2.new_env_dir, sizeof(int) | MDFNSTATE_RLSB, "s2ND" },
 {&gba_apu.square2.new_volume, sizeof(int) | MDFNSTATE_RLSB, "s2NV" },

 {&gba_apu.square2.period, sizeof(int) | MDFNSTATE_RLSB, "s2OP" },
 {&gba_apu.square2.volume, sizeof(int) | MDFNSTATE_RLSB, "s2OV" },
 {&gba_apu.square2.frequency, sizeof(int) | MDFNSTATE_RLSB, "s2OF" },
 {&gba_apu.square2.length, sizeof(int) | MDFNSTATE_RLSB, "s2OL" },
 {&gba_apu.square2.new_length, sizeof(int) | MDFNSTATE_RLSB, "s2Ol" },
 {&gba_apu.square2.ME, sizeof(int) | MDFNSTATE_RLSB, "s2OM" },
 {&gba_apu.square2.enabled, sizeof(bool8) | MDFNSTATE_RLSB, "s2OE" },
 {&gba_apu.square2.length_enabled, sizeof(bool8) | MDFNSTATE_RLSB, "s2Oe" },
 {&gba_apu.square2.output_select, sizeof(int) | MDFNSTATE_RLSB, "s2OS" },


 {&gba_apu.noise.bits, sizeof(int) | MDFNSTATE_RLSB, "sNNB" },
 {&gba_apu.noise.tap, sizeof(int) | MDFNSTATE_RLSB, "sNNT" },

 {&gba_apu.noise.env_period, sizeof(int) | MDFNSTATE_RLSB, "sNEP" },
 {&gba_apu.noise.env_dir, sizeof(int) | MDFNSTATE_RLSB, "sNED" },
 {&gba_apu.noise.env_delay, sizeof(int) | MDFNSTATE_RLSB, "sNEd" },

 {&gba_apu.noise.new_env_period, sizeof(int) | MDFNSTATE_RLSB, "sNNP" },
 {&gba_apu.noise.new_env_dir, sizeof(int) | MDFNSTATE_RLSB, "sNND" },
 {&gba_apu.noise.new_volume, sizeof(int) | MDFNSTATE_RLSB, "sNNV" },

 {&gba_apu.noise.period, sizeof(int) | MDFNSTATE_RLSB, "sNOP" },
 {&gba_apu.noise.volume, sizeof(int) | MDFNSTATE_RLSB, "sNOV" },
 {&gba_apu.noise.frequency, sizeof(int) | MDFNSTATE_RLSB, "sNOF" },
 {&gba_apu.noise.length, sizeof(int) | MDFNSTATE_RLSB, "sNOL" },
 {&gba_apu.noise.new_length, sizeof(int) | MDFNSTATE_RLSB, "sNOl" },
 {&gba_apu.noise.ME, sizeof(int) | MDFNSTATE_RLSB, "sNOM" },
 {&gba_apu.noise.enabled, sizeof(bool8) | MDFNSTATE_RLSB, "sNOE" },
 {&gba_apu.noise.length_enabled, sizeof(bool8) | MDFNSTATE_RLSB, "sNOe" },


 {&gba_apu.wave.volume_shift, sizeof(int) | MDFNSTATE_RLSB, "sWWS" },
 {&gba_apu.wave.wave_pos, sizeof(int) | MDFNSTATE_RLSB, "sWWP" },
 {gba_apu.wave.wave, 32 * 2, "WaveData" },

 {&gba_apu.wave.period, sizeof(int) | MDFNSTATE_RLSB, "sWOP" },
 {&gba_apu.wave.volume, sizeof(int) | MDFNSTATE_RLSB, "sWOV" },
 {&gba_apu.wave.frequency, sizeof(int) | MDFNSTATE_RLSB, "sWOF" },
 {&gba_apu.wave.length, sizeof(int) | MDFNSTATE_RLSB, "sWOL" },
 {&gba_apu.wave.new_length, sizeof(int) | MDFNSTATE_RLSB, "sWOl" },
 {&gba_apu.wave.ME, sizeof(int) | MDFNSTATE_RLSB, "sWOM" },
 {&gba_apu.wave.enabled, sizeof(bool8) | MDFNSTATE_RLSB, "sWOE" },
 {&gba_apu.wave.length_enabled, sizeof(bool8) | MDFNSTATE_RLSB, "sWOe" },
 SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "SND");
 if(load)
  gba_apu.dirty();
 return(ret);
}

uint8 soundRead(uint32 address)
{
 if(address == 0x80)
  return(gba_apu.read_register(soundTS, 0xff24));
 else if(address == 0x81)
  return(gba_apu.read_register(soundTS, 0xff25));
 else if(address == 0x84)
  return(gba_apu.read_register(soundTS, 0xff26));
 else if(address >= 0x90 && address <= 0x9f)
 {
  return(gba_apu.read_register(soundTS, address - 0x90 + 0xff30));
 }
 return(ioMem[address]);
}

void soundEvent(uint32 address, uint8 data)
{
 uint32 origa = address;
 address &= 0xFF;

 if(address == 0x60)
  address = 0xff10;
 else if(address == 0x62)
  address = 0xff11;
 else if(address == 0x63)
  address = 0xff12;
 else if(address == 0x64)
  address = 0xff13;
 else if(address == 0x65)
  address = 0xff14;

 else if(address == 0x68)
  address = 0xff16;
 else if(address == 0x69)
  address = 0xff17;
 else if(address == 0x6c)
  address = 0xff18;
 else if(address == 0x6d)
  address = 0xff19;

 else if(address == 0x70)
  address = 0xff1a;
 else if(address == 0x72)
  address = 0xff1b;
 else if(address == 0x73)
  address = 0xff1c;
 else if(address == 0x74)
  address = 0xff1d;
 else if(address == 0x75)
  address = 0xff1e;

 else if(address == 0x78)
  address = 0xff20;
 else if(address == 0x79)
  address = 0xff21;
 else if(address == 0x7c)
  address = 0xff22;
 else if(address == 0x7d)
  address = 0xff23;

 else if(address == 0x80)
  address = 0xff24;
 else if(address == 0x81)
  address = 0xff25;
 else if(address == 0x84)
  address = 0xff26;
 else if(address >= 0x90 && address <= 0x9f)
 {
  address += 0xff30 - 0x90;
 }
 else 
 {
  ioMem[origa] = data;
  return;
 }
 ioMem[origa] = data;

 gba_apu.write_register(soundTS, address, data);
}

void soundEvent(uint32 address, uint16 data)
{
  switch(address) {
  case SGCNT0_H:
    data &= 0xFF0F;
    soundControl = data & 0x770F;;
    if(data & 0x0800) {
      soundDSFifoAWriteIndex = 0;
      soundDSFifoAIndex = 0;
      soundDSFifoACount = 0;
      soundDSAValue = 0;
      memset(soundDSFifoA, 0, 32);
    }
    soundDSAEnabled = (data & 0x0300) ? true : false;
    soundDSATimer = (data & 0x0400) ? 1 : 0;    
    if(data & 0x8000) {
      soundDSFifoBWriteIndex = 0;
      soundDSFifoBIndex = 0;
      soundDSFifoBCount = 0;
      soundDSBValue = 0;
      memset(soundDSFifoB, 0, 32);
    }
    soundDSBEnabled = (data & 0x3000) ? true : false;
    soundDSBTimer = (data & 0x4000) ? 1 : 0;
    *((uint16 *)&ioMem[address]) = data;    
    break;
  case FIFOA_L:
  case FIFOA_H:
    soundDSFifoA[soundDSFifoAWriteIndex++] = data & 0xFF;
    soundDSFifoA[soundDSFifoAWriteIndex++] = data >> 8;
    soundDSFifoACount += 2;
    soundDSFifoAWriteIndex &= 31;
    *((uint16 *)&ioMem[address]) = data;    
    break;
  case FIFOB_L:
  case FIFOB_H:
    soundDSFifoB[soundDSFifoBWriteIndex++] = data & 0xFF;
    soundDSFifoB[soundDSFifoBWriteIndex++] = data >> 8;
    soundDSFifoBCount += 2;
    soundDSFifoBWriteIndex &= 31;
    *((uint16 *)&ioMem[address]) = data;    
    break;
  case 0x88:
    data &= 0xC3FF;
    *((uint16 *)&ioMem[address]) = data;
    break;
  case 0x90:
  case 0x92:
  case 0x94:
  case 0x96:
  case 0x98:
  case 0x9a:
  case 0x9c:
  case 0x9e:
    //printf("Yay: %04x: %04x\n", 0xFF30 + (address & 0xF), data);
    gba_apu.write_register(soundTS, 0xFF30 + (address & 0xF), data & 0xFF);
    gba_apu.write_register(soundTS, 0xFF30 + (address & 0xF) + 1, data >> 8);
    //*((uint16 *)&sound3WaveRam[(sound3Bank*0x10)^0x10+(address&14)]) = data;
    //*((uint16 *)&ioMem[address]) = data;    
    break;    
  }
}
static void soundLick(void);

void soundDirectSoundATimer()
{
  if(soundDSAEnabled) {
    if(soundDSFifoACount <= 16) {
      CPUCheckDMA(3, 2);
    }
    
    if(soundDSFifoACount > 16)
    {
     soundDSAValue = (soundDSFifoA[soundDSFifoAIndex]);
     soundDSFifoAIndex = (soundDSFifoAIndex + 1) & 31;
     soundDSFifoACount--;
    } 
  } 
  else
  {
   soundDSAValue = 0;
  }
 soundLick();
}

void soundDirectSoundBTimer()
{
  if(soundDSBEnabled) {
    if(soundDSFifoBCount <= 16) {
      CPUCheckDMA(3, 4);
    }
    
    if(soundDSFifoBCount > 16)
    {
     soundDSBValue = (soundDSFifoB[soundDSFifoBIndex]);
     soundDSFifoBIndex = (soundDSFifoBIndex + 1) & 31;
     soundDSFifoBCount--;
    }
  } else {
    soundDSBValue = 0;
  }
  soundLick();
}

void soundTimerOverflow(int timer)
{
  if(soundDSAEnabled && (soundDSATimer == timer)) {
    soundDirectSoundATimer();
  }
  if(soundDSBEnabled && (soundDSBTimer == timer)) {
    soundDirectSoundBTimer();
  }
}

#ifndef max
#define max(a,b) (a)<(b)?(b):(a)
#endif

static ALWAYS_INLINE void soundMix(int &left, int &right)
{
  int res = 0;
  int dsaRatio = ioMem[0x82] & 4;
  int dsbRatio = ioMem[0x82] & 8;
  
  if(soundControl & 0x0200)
  {
    if(!dsaRatio)
    {
      res =  (int8)soundDSAValue >> 1;
    }
    else
      res = (int8)soundDSAValue;
  }
  
  if(soundControl & 0x2000)
  {
    if(!dsbRatio)
    {
      res += (int8)soundDSBValue >> 1;
    }
    else
      res += (int8)soundDSBValue;
  }
  
  left = res;
  
  res = 0;
  
  if(soundControl & 0x0100)
  {
    if(!dsaRatio)
      res = (int8)soundDSAValue >> 1;
    else
      res = (int8)soundDSAValue;
  }

  if(soundControl & 0x1000)
  {
    if(!dsbRatio)
      res += (int8)soundDSAValue >> 1;
    else
      res += (int8)soundDSBValue;
  }
 right = res;
}

static int lleft = 0, lright = 0;
static bool forcemono;

void soundLick()
{
 int left, right;

 left=right=0;

 soundMix(left, right);

 if(forcemono)
 {
  int tmp = (left + right) / 2;
  left = right = tmp;

  if(left != lleft)
   synth.offset_inline(soundTS, left - lleft, gba_buf.left());
 }
 else
 {
  if(left != lleft)
   synth.offset_inline(soundTS, left - lleft, gba_buf.left());

  if(right != lright)
   synth.offset_inline(soundTS, right - lright, gba_buf.right());
 }

 lleft = left;
 lright = right;
}

int16 *MDFNGBASOUND_Flush(int32 *len)
{
 static int16 yaybuf[8000];
 int love2;

 int ratio = ioMem[0x82] & 3;
 int rat_table[4] = { 2, 1, 0, 2 };
 
 gba_apu.volume(0.30 * (double)(4 >> rat_table[ratio]) / 4);

 gba_apu.end_frame(soundTS);

 if(forcemono)
 {
  gba_buf.left()->end_frame(soundTS);
  love2 = gba_buf.left()->read_samples(yaybuf, 8000);
 }
 else
 {
  gba_buf.end_frame(soundTS);
  love2 = gba_buf.read_samples(yaybuf, 8000);
 }

 soundTS = 0;

 if(!FSettings.SndRate)
 {
  *len = 0;
  return(NULL);
 }

 if(!forcemono)
  love2 /= 2;

 *len = love2;
 return(yaybuf);
}

void MDFNGBASOUND_Init(bool WantMono)
{
 gba_buf.set_sample_rate(FSettings.SndRate?FSettings.SndRate:44100, 60);
 gba_buf.clock_rate((long)(4194304 * 4 * FSettings.soundmultiplier));

 forcemono = WantMono;

 if(forcemono)
  gba_apu.output(gba_buf.left(), NULL, NULL);
 else
  gba_apu.output(gba_buf.center(), gba_buf.left(), gba_buf.right());

 gba_apu.setgba(1);
 gba_apu.volume(0.30);

 synth.volume(0.60); //0.25);
 gba_buf.bass_freq(20);
}

void soundShutdown()
{

}

void soundReset()
{
  int addr = 0x90;

  while(addr < 0xA0) {
    ioMem[addr++] = 0x00;
    ioMem[addr++] = 0xff;
  }

  addr = 0;
}

bool8 soundInit()
{
    return true;
}  

void MDFNGBA_SetSoundMultiplier(double multiplier)
{
        gba_buf.clock_rate((long)(4194304 * 4 * multiplier));
}

void MDFNGBA_SetSoundVolume(uint32 volume)
{

}
void MDFNGBA_Sound(int rate)
{
        gba_buf.set_sample_rate(rate?rate:44100, 60);
}
