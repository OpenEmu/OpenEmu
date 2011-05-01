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

#include "GBA.h"
#include "Globals.h"
#include "Sound.h"

#include <memory.h>
#include <math.h>

#include <blip/Blip_Buffer.h>
#include "gb_apu/Gb_Apu.h"

#include <blip/Stereo_Buffer.h>

typedef Blip_Synth<blip_good_quality, 0xFF * 2> Synth;

static Synth synth;

static Gb_Apu gba_apu;
static Stereo_Buffer gba_buf;

extern uint32 soundTS;

static int lleft = 0, lright = 0;

int soundControl = 0;

typedef struct
{
 int32 FifoIndex;
 int32 FifoCount;
 int32 FifoWriteIndex;
 bool Enabled;
 int32 Timer;
 uint8 Fifo[32];
 uint8 Value;
} GBADigiSound;

GBADigiSound DSChans[2]; // Digital sound channels, not Nintendo DS :b

#define soundDSFifoAIndex DSChans[0].FifoIndex
#define soundDSFifoACount DSChans[0].FifoCount
#define soundDSFifoAWriteIndex DSChans[0].FifoWriteIndex
#define soundDSAEnabled  DSChans[0].Enabled
#define soundDSATimer DSChans[0].Timer
#define soundDSFifoA DSChans[0].Fifo
#define soundDSAValue DSChans[0].Value

#define soundDSFifoBIndex DSChans[1].FifoIndex
#define soundDSFifoBCount DSChans[1].FifoCount
#define soundDSFifoBWriteIndex DSChans[1].FifoWriteIndex
#define soundDSBEnabled  DSChans[1].Enabled
#define soundDSBTimer DSChans[1].Timer
#define soundDSFifoB DSChans[1].Fifo
#define soundDSBValue DSChans[1].Value

#include "../state.h"

int MDFNGBASOUND_StateAction(StateMem *sm, int load, int data_only)
{
 gb_apu_state_t apu_state;
 
 //if(!load) // always save state, in case there is none to load
  gba_apu.save_state( &apu_state );
 
 SFORMAT StateRegs[] = 
 {
  SFVAR(soundControl),
  SFVAR(soundDSFifoAIndex),
  SFVAR(soundDSFifoACount),
  SFVAR(soundDSFifoAWriteIndex),
  SFVAR(soundDSAEnabled),
  SFVAR(soundDSATimer),
  SFARRAYN(soundDSFifoA, sizeof(soundDSFifoA), "soundDSFifoA"),
  SFVAR(soundDSAValue),
  SFVAR(soundDSFifoBIndex),
  SFVAR(soundDSFifoBCount),
  SFVAR(soundDSFifoBWriteIndex),
  SFVAR(soundDSBEnabled),
  SFVAR(soundDSBTimer),
  SFARRAYN(soundDSFifoB, sizeof(soundDSFifoB), "soundDSFifoB"),
  SFVAR(soundDSBValue),

  SFVARN(apu_state, "apu_state"),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "SND");
 if(ret && load)
 {
  gba_apu.reset( gba_apu.mode_agb, true );
  (void) gba_apu.load_state( apu_state ); // TODO: warn if this returns error
 }
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

#ifndef max
#define max(a,b) (a)<(b)?(b):(a)
#endif

static inline void soundMix(int &left, int &right)
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

int meow = 0;

static inline void soundLick(void)
{
 int left, right;

 left = right = 0;

 soundMix(left, right);

 if(left != lleft)
  synth.offset_inline(soundTS, left - lleft, gba_buf.left());

 if(right != lright)
  synth.offset_inline(soundTS, right - lright, gba_buf.right());

 lleft = left;
 lright = right;
}

static void DSTimer(int which, int dmamask)
{
 if(DSChans[which].Enabled)
 {
  if(DSChans[which].FifoCount <= 16)
  {
   CPUCheckDMA(3, dmamask);
  }

  if(DSChans[which].FifoCount > 16)
  {
   DSChans[which].Value = (DSChans[which].Fifo[DSChans[which].FifoIndex]);
   DSChans[which].FifoIndex = (DSChans[which].FifoIndex + 1) & 0x1F;
   DSChans[which].FifoCount--;
  }
 }
 else
 {
  DSChans[which].Value = 0;
 }
}

void soundTimerOverflow(int timer)
{
 bool NeedLick = FALSE;

 if(soundDSAEnabled && (soundDSATimer == timer))
 {
  DSTimer(0, 2);
  NeedLick = TRUE;
 }

 if(soundDSBEnabled && (soundDSBTimer == timer))
 {
  DSTimer(1, 4);
  NeedLick = TRUE;
 }

 if(NeedLick)
  soundLick();
}

int32 MDFNGBASOUND_Flush(int16 *SoundBuf, const int32 MaxSoundFrames)
{
 const int ratio = ioMem[0x82] & 3;
 static const int rat_table[4] = { 2, 1, 0, 2 };
 int32 FrameCount = 0;
 
 gba_apu.volume(0.333 * (double)(4 >> rat_table[ratio]) / 4);

 gba_apu.end_frame(soundTS);

 gba_buf.end_frame(soundTS);

 if(SoundBuf)
  FrameCount = gba_buf.read_samples(SoundBuf, MaxSoundFrames * 2) / 2;
 else
  gba_buf.clear();

 soundTS = 0;


#if 0
 if(SoundBuf)
 {
  unsigned long long crf = (unsigned long long)gba_buf.left()->clock_rate_factor(gba_buf.left()->clock_rate());
  double real_rate = (double)crf * gba_buf.left()->clock_rate() / (1ULL << BLIP_BUFFER_ACCURACY);

  printf("%f\n", real_rate);
 }
#endif

 return(FrameCount);
}

void MDFNGBASOUND_Init(void)
{
 MDFNGBA_SetSoundRate(0);	

 gba_buf.clock_rate((long)(4194304 * 4));

 gba_apu.set_output(gba_buf.center(), gba_buf.left(), gba_buf.right());

 gba_apu.reset( gba_apu.mode_agb, true );

 gba_apu.volume(0.333);
 synth.volume(0.666);

 gba_buf.bass_freq(20);
}

void soundReset()
{
  for(int ch = 0; ch < 2; ch++)
  {
   DSChans[ch].FifoIndex = 0;
   DSChans[ch].FifoCount = 0;
   DSChans[ch].FifoWriteIndex = 0;
   DSChans[ch].Enabled = false;
   DSChans[ch].Timer = 0;
   memset(DSChans[ch].Fifo, 0, 32);
   DSChans[ch].Value = 0;
  }

  for(int addr = 0x90; addr < 0xA0;)
  {
   ioMem[addr++] = 0x00;
   ioMem[addr++] = 0xff;
  }

  gba_apu.reset( gba_apu.mode_agb, true );
}

bool MDFNGBA_SetSoundRate(uint32 rate)
{
 gba_buf.set_sample_rate(rate?rate:44100, 60);
 return(TRUE);
}
