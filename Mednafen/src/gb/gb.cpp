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

#include "../mednafen.h"
#include "../file.h"
#include "../general.h"
#include "../state.h"
#include "../movie.h"
#include "../netplay.h"
#include "../memory.h"
#include "../mempatcher.h"
#include "../md5.h"

#include <string.h>
#include <memory.h>
#include <zlib.h>

#include "gb.h"
#include "gbGlobals.h"
#include "memory.h"
#include "sound.h"
#include "z80.h"

static uint32 *gbColorFilter = NULL; //[32768];
static uint32 gbMonoColorMap[8 + 1];	// Mono color map(+1 = LCD off color)!

static bool gbUpdateSizes();
static int32 SoundTS = 0;
extern uint16 gbLineMix[160];

// mappers
void (*mapper)(uint16,uint8) = NULL;
void (*mapperRAM)(uint16,uint8) = NULL;
uint8 (*mapperReadRAM)(uint16) = NULL;

static uint8 HRAM[0x80];
uint8 gbOAM[0xA0];

// 0xff00
uint8 register_P1    = 0;

// 0xff01
uint8 register_SB    = 0;
// 0xff02
uint8 register_SC    = 0;
// 0xff04
uint8 register_DIV   = 0;
// 0xff05
uint8 register_TIMA  = 0;
// 0xff06
uint8 register_TMA   = 0;
// 0xff07
uint8 register_TAC   = 0;
// 0xff0f
uint8 register_IF    = 0;
// 0xff40
uint8 register_LCDC  = 0;
// 0xff41
uint8 register_STAT  = 0;
// 0xff42
uint8 register_SCY   = 0;
// 0xff43
uint8 register_SCX   = 0;
// 0xff44
uint8 register_LY    = 0;
// 0xff45
uint8 register_LYC   = 0;
// 0xff46
uint8 register_DMA   = 0;
// 0xff4a
uint8 register_WY    = 0;
// 0xff4b
uint8 register_WX    = 0;
// 0xff4d
uint8 register_KEY1  = 0;
// 0xff4f
uint8 register_VBK   = 0;
// 0xff51
uint8 register_HDMA1 = 0;
// 0xff52
uint8 register_HDMA2 = 0;
// 0xff53
uint8 register_HDMA3 = 0;
// 0xff54
uint8 register_HDMA4 = 0;
// 0xff55
uint8 register_HDMA5 = 0;

// 0xff56
uint8 register_RP = 0;

// 0xff68
uint8 register_BCPS = 0;
// 0xff69
uint8 register_BCPD = 0;
// 0xff6a
uint8 register_OCPS = 0;
// 0xff6b
uint8 register_OCPD = 0;

// 0xff6c
uint8 register_FF6C = 0;
// 0xff72
uint8 register_FF72 = 0;
// 0xff73
uint8 register_FF73 = 0;
// 0xff74
uint8 register_FF74 = 0;
// 0xff75
uint8 register_FF75 = 0;

// 0xff70
uint8 register_SVBK  = 0;
// 0xffff
uint8 register_IE    = 0;

// ticks definition
int GBDIV_CLOCK_TICKS          = 64;
int GBLCD_MODE_0_CLOCK_TICKS   = 51;
int GBLCD_MODE_1_CLOCK_TICKS   = 1140;
int GBLCD_MODE_2_CLOCK_TICKS   = 20;
int GBLCD_MODE_3_CLOCK_TICKS   = 43;
int GBLY_INCREMENT_CLOCK_TICKS = 114;
int GBTIMER_MODE_0_CLOCK_TICKS = 256;
int GBTIMER_MODE_1_CLOCK_TICKS = 4;
int GBTIMER_MODE_2_CLOCK_TICKS = 16;
int GBTIMER_MODE_3_CLOCK_TICKS = 64;
int GBSERIAL_CLOCK_TICKS       = 128;
int GBSYNCHRONIZE_CLOCK_TICKS  = 52920;

// state variables

// serial
int gbSerialOn = 0;
int gbSerialTicks = 0;
int gbSerialBits = 0;
// timer
int gbTimerOn = 0;
int gbTimerTicks = GBTIMER_MODE_0_CLOCK_TICKS;
int gbTimerClockTicks = GBTIMER_MODE_0_CLOCK_TICKS;
int gbTimerMode = 0;


enum
{
 GBLCDM_HBLANK = 0,
 GBLCDM_VBLANK = 1,
 GBLCDM_OAM = 2,
 GBLCDM_OAM_VRAM = 3,
};

// lcd
int gbLcdMode = GBLCDM_OAM;
int gbLcdTicks = GBLCD_MODE_2_CLOCK_TICKS;
int gbLcdLYIncrementTicks = 0;
// div
int gbDivTicks = GBDIV_CLOCK_TICKS;
// cgb
int gbVramBank = 0;
int gbWramBank = 1;
int gbHdmaSource = 0x0000;
int gbHdmaDestination = 0x8000;
int gbHdmaBytes = 0x0000;
int gbHdmaOn = 0;
int gbSpeed = 0;

// timing
int gbSynchronizeTicks = GBSYNCHRONIZE_CLOCK_TICKS;

// emulator features
int gbBattery = 0;

static uint8 gbJoymask;

static const int gbRomSizes[] = { 0x00008000, // 32K
                     0x00010000, // 64K
                     0x00020000, // 128K
                     0x00040000, // 256K
                     0x00080000, // 512K
                     0x00100000, // 1024K
                     0x00200000, // 2048K
                     0x00400000, // 4096K
                     0x00800000  // 8192K
};

static const int gbRomSizesMasks[] = { 0x00007fff,
                          0x0000ffff,
                          0x0001ffff,
                          0x0003ffff,
                          0x0007ffff,
                          0x000fffff,
                          0x001fffff,
                          0x003fffff,
                          0x007fffff
};

static const int gbRamSizes[6] = { 0x00000000, // 0K
                      0x00000800, // 2K
                      0x00002000, // 8K
                      0x00008000, // 32K
                      0x00020000, // 128K
                      0x00010000  // 64K
};

static const int gbRamSizesMasks[6] = { 0x00000000,
                           0x000007ff,
                           0x00001fff,
                           0x00007fff,
                           0x0001ffff,
                           0x0000ffff
};

static void gbGenFilter(int rs, int gs, int bs)
{
 for(int r = 0; r < 32; r++)
  for(int g = 0; g < 32; g++)
   for(int b = 0; b < 32; b++)
   {
    int nr = r * 226 + g * 29 + b * 0;
    int ng = r * 29 + g * 197 + b * 29;
    int nb = r * 30 + g * 73 + b * 152;

    nr /= 31;
    ng /= 31;
    nb /= 31;

    gbColorFilter[(b << 10) | (g << 5) | r] = (nr << rs) | (ng << gs) | (nb << bs);
   }

 for(int i = 0; i < 4; i++)
 {
  int r,g,b;

  r = (3 - (i & 3)) * 48 + 32;
  g = (3 - (i & 3)) * 48 + 32;
  b = (3 - (i & 3)) * 48 + 32;

  gbMonoColorMap[i] = gbMonoColorMap[i + 4] = (r << rs) | (g << gs) | (b << bs);
 }

 gbMonoColorMap[8] = gbMonoColorMap[0];
}

void gbCopyMemory(uint16 d, uint16 s, int count)
{
  while(count) 
  {
    gbWriteMemory(d, gbReadMemory(s));
    s++;
    d++;
    count--;
  }
}

void gbDoHdma()
{
  gbCopyMemory(gbHdmaDestination,
               gbHdmaSource,
               0x10);
  
  gbHdmaDestination += 0x10;
  gbHdmaSource += 0x10;
  
  register_HDMA2 += 0x10;
  if(register_HDMA2 == 0x00)
    register_HDMA1++;
  
  register_HDMA4 += 0x10;
  if(register_HDMA4 == 0x00)
    register_HDMA3++;
  
  if(gbHdmaDestination == 0x96b0)
    gbHdmaBytes = gbHdmaBytes;
  gbHdmaBytes -= 0x10;
  register_HDMA5--;
  if(register_HDMA5 == 0xff)
    gbHdmaOn = 0;
}

// fix for Harley and Lego Racers
void gbCompareLYToLYC()
{
 if(register_LY == register_LYC) 
 {
  // mark that we have a match
  register_STAT |= 4;
    
  // check if we need an interrupt
  if(register_STAT & 0x40)
   register_IF |= 2;
 }
 else // no match
  register_STAT &= 0xfb;
}

static void ClockTIMA(void)
{
 register_TIMA++;

 if(register_TIMA == 0) 
 {
  // timer overflow!

  // reload timer modulo
  register_TIMA = register_TMA;

  // flag interrupt
  register_IF |= 4;
 }
}

void gbWriteMemory(uint16 address, uint8 value)
{
  if(address < 0x8000) {
    if(mapper)
      (*mapper)(address, value);
    return;
  }
   
  if(address < 0xa000) {
    gbMemoryMap[address>>12][address&0x0fff] = value;
    return;
  }
  
  if(address < 0xc000) 
  {
    if(mapper)
      (*mapperRAM)(address, value);
    return;
  }
  
  if(address < 0xfe00) {
     unsigned int page = (address >> 12);
     if(page >= 0xE) page -= 2;
    gbMemoryMap[page][address & 0x0fff] = value;
    return;
  }
  
  if(address < 0xff00) {
    if(address < 0xFEA0)
     gbOAM[address & 0xFF] = value;
    return;
  }
  //printf("Write: %04x %02x, %d\n", address, value, register_LY);
  switch(address & 0x00ff) 
  {
   case 0x00:
    register_P1 = ((register_P1 & 0xcf) | (value & 0x30));
    return;

  case 0x01: {
    register_SB = value;
    return;
  }
    
  // serial control
  case 0x02: {
    gbSerialOn = (value & 0x80);
    register_SC = value;    
    if(gbSerialOn) {
      gbSerialTicks = GBSERIAL_CLOCK_TICKS;
#ifdef LINK_EMULATION
      if(linkConnected) {
        if(value & 1) {
          linkSendByte(0x100|register_SB);
          Sleep(5);
        }
      }
#endif
    }

    gbSerialBits = 0;
    return;
  }
  
  // DIV register resets on any write
  case 0x04: {
    register_DIV = 0;
    gbDivTicks = GBDIV_CLOCK_TICKS;
    // Another weird timer 'bug' :
    // Writing to DIV register resets the internal timer,
    // and can also increase TIMA/trigger an interrupt
    // in some cases...
    //if (gbTimerOn && !(gbInternalTimer & (gbTimerClockTicks>>1)))
    //{
    // ClockTIMA();
    //}
    return;
  }
  case 0x05:
    register_TIMA = value;
    return;

  case 0x06:
    register_TMA = value;
    return;
    
    // TIMER control
  case 0x07: {
    register_TAC = value;
    gbTimerOn = (value & 4);
    gbTimerMode = value & 3;
    //    register_TIMA = register_TMA;
    switch(gbTimerMode) {
    case 0:
      gbTimerClockTicks = gbTimerTicks = GBTIMER_MODE_0_CLOCK_TICKS;
      break;
    case 1:
      gbTimerClockTicks = gbTimerTicks = GBTIMER_MODE_1_CLOCK_TICKS;
      break;
    case 2:
      gbTimerClockTicks = gbTimerTicks = GBTIMER_MODE_2_CLOCK_TICKS;
      break;
    case 3:
      gbTimerClockTicks = gbTimerTicks = GBTIMER_MODE_3_CLOCK_TICKS;
      break;
    }
    return;
  }

  case 0x0f: 
  {
    register_IF = value;
    return;
  }
  
  case 0x10 ... 0x3f:
  {
    MDFNGBSOUND_Write(SoundTS, address, value);
    return;
  }
  case 0x40: {
    int lcdChange = (register_LCDC & 0x80) ^ (value & 0x80);

    if(lcdChange) {
      if(value & 0x80) {
        gbLcdTicks = GBLCD_MODE_1_CLOCK_TICKS;
        gbLcdMode = GBLCDM_HBLANK;
        register_STAT &= 0xfc;
        register_LY = 0x00;
      } else {
        gbLcdTicks = 0;
        gbLcdMode = GBLCDM_HBLANK;
        register_STAT &= 0xfc;
        register_LY = 0x00;
      }
      //      compareLYToLYC();
    }
    // don't draw the window if it was not enabled and not being drawn before
    if(!(register_LCDC & 0x20) && (value & 0x20) && gbWindowLine == -1 &&
       register_LY > register_WY)
      gbWindowLine = 144;

    register_LCDC = value;

    return;
  }

  // STAT
  case 0x41: {
    //register_STAT = (register_STAT & 0x87) |
    //      (value & 0x7c);
    register_STAT = (value & 0xf8) | (register_STAT & 0x07); // fix ?
    // GB bug from Devrs FAQ
    if(!gbCgbMode && (register_LCDC & 0x80) && gbLcdMode < 2)
      register_IF |= 2;
    return;
  }

  // SCY
  case 0x42: {
    register_SCY = value;
    return;
  }
  
  // SCX
  case 0x43: {
    register_SCX = value;
    return;
  }
  
  // LY
  case 0x44: {
    // read only
    return;
  }

  // LYC
  case 0x45: {
    register_LYC = value;
    if((register_LCDC & 0x80)) {
      gbCompareLYToLYC();
    }
    return;
  }
  
  // DMA!
  case 0x46: {
    int source = value * 0x0100;

    gbCopyMemory(0xfe00,
                 source,
                 0xa0);
    register_DMA = value;
    return;
  }
  
  // BGP
  case 0x47: {
    gbBgp[0] = value & 0x03;
    gbBgp[1] = (value & 0x0c)>>2;
    gbBgp[2] = (value & 0x30)>>4;
    gbBgp[3] = (value & 0xc0)>>6;
    return;
  }
  
  // OBP0
  case 0x48: {
    gbObp0[0] = value & 0x03;
    gbObp0[1] = (value & 0x0c)>>2;
    gbObp0[2] = (value & 0x30)>>4;
    gbObp0[3] = (value & 0xc0)>>6;
    return;
  }

  // OBP1
  case 0x49: {
    gbObp1[0] = value & 0x03;
    gbObp1[1] = (value & 0x0c)>>2;
    gbObp1[2] = (value & 0x30)>>4;
    gbObp1[3] = (value & 0xc0)>>6;
    return;
  }

  case 0x4a:
    register_WY = value;
    return;
    
  case 0x4b:
    register_WX = value;
    return;
  
    // KEY1
  case 0x4d: {
    if(gbCgbMode) {
      register_KEY1 = (register_KEY1 & 0x80) | (value & 1);
      return;
    }
  }
  break;
    
  // VBK
  case 0x4f: {
    if(gbCgbMode) {
      value = value & 1;
      if(value == gbVramBank)
        return;
      
      int vramAddress = value * 0x2000;
      gbMemoryMap[0x08] = &gbVram[vramAddress];
      gbMemoryMap[0x09] = &gbVram[vramAddress + 0x1000];
      
      gbVramBank = value;
      register_VBK = value;
    }
    return;
  }
  break;

  // HDMA1
  case 0x51: {
    if(gbCgbMode) {
      if(value > 0x7f && value < 0xa0)
        value = 0;
      
      gbHdmaSource = (value << 8) | (register_HDMA2 & 0xf0);
      
      register_HDMA1 = value;
      return;
    }
  }
  break;
  
  // HDMA2
  case 0x52: {
    if(gbCgbMode) {
      value = value & 0xf0;
      
      gbHdmaSource = (register_HDMA1 << 8) | (value);
      
      register_HDMA2 = value;
      return;
    }
  }
  break;

  // HDMA3
  case 0x53: {
    if(gbCgbMode) {
      value = value & 0x1f;
      gbHdmaDestination = (value << 8) | (register_HDMA4 & 0xf0);
      gbHdmaDestination += 0x8000;
      register_HDMA3 = value;
      return;
    }
  }
  break;
  
  // HDMA4
  case 0x54: {
    if(gbCgbMode) {
      value = value & 0xf0;
      gbHdmaDestination = ((register_HDMA3 & 0x1f) << 8) | value;
      gbHdmaDestination += 0x8000;
      register_HDMA4 = value;
      return;
    }
  }
  break;
  
  // HDMA5
  case 0x55: {
    if(gbCgbMode) {
      gbHdmaBytes = 16 + (value & 0x7f) * 16;
      if(gbHdmaOn) {
        if(value & 0x80) {
          register_HDMA5 = (value & 0x7f);
        } else {
          register_HDMA5 = 0xff;
          gbHdmaOn = 0;
        }
      } else {
        if(value & 0x80) {
          gbHdmaOn = 1;
          register_HDMA5 = value & 0x7f;
          if(gbLcdMode == GBLCDM_HBLANK)
            gbDoHdma();
        } else {
          // we need to take the time it takes to complete the transfer into
          // account... according to GB DEV FAQs, the setup time is the same
          // for single and double speed, but the actual transfer takes the
          // same time
          //if(gbSpeed)
          //  gbDmaTicks = 231 + 16 * (value & 0x7f);
          //else
          //  gbDmaTicks = 231 + 8 * (value & 0x7f);
	  gbDmaTicks = 231;
          gbCopyMemory(gbHdmaDestination,
                       gbHdmaSource,
                       gbHdmaBytes);
          gbHdmaDestination += gbHdmaBytes;
          gbHdmaSource += gbHdmaBytes;
          
          register_HDMA3 = ((gbHdmaDestination - 0x8000) >> 8) & 0x1f;
          register_HDMA4 = gbHdmaDestination & 0xf0;
          register_HDMA1 = (gbHdmaSource >> 8) & 0xff;
          register_HDMA2 = gbHdmaSource & 0xf0;
        }
      }
      return;
    }
  }
  break;
  
  
  case 0x56:
   	register_RP = value & 0xC1;
	break;

  // BCPS
  case 0x68: {
     if(gbCgbMode) {
      int paletteIndex = (value & 0x3f) >> 1;
      int paletteHiLo   = (value & 0x01);
      
      register_BCPS = value;
      register_BCPD = (paletteHiLo ?
                        (gbPalette[paletteIndex] >> 8) :
                        (gbPalette[paletteIndex] & 0x00ff));
      return;
    }
  }
  break;
  
  // BCPD
  case 0x69: {
    if(gbCgbMode) {
      int v = register_BCPS;
      int paletteIndex = (v & 0x3f) >> 1;
      int paletteHiLo  = (v & 0x01);
      register_BCPD = value;
      gbPalette[paletteIndex] = (paletteHiLo ?
                                 ((value << 8) | (gbPalette[paletteIndex] & 0xff)) :
                                 ((gbPalette[paletteIndex] & 0xff00) | (value))) & 0x7fff;
                                        
      if(register_BCPS & 0x80) {
        int index = ((register_BCPS & 0x3f) + 1) & 0x3f;
        
        register_BCPS = (register_BCPS & 0x80) | index;
        
        register_BCPD = (index & 1 ?
                          (gbPalette[index>>1] >> 8) :
                          (gbPalette[index>>1] & 0x00ff));
        
      }
      return;
    }
  }
  break;
  
  // OCPS 
  case 0x6a: {
    if(gbCgbMode) {
      int paletteIndex = (value & 0x3f) >> 1;
      int paletteHiLo   = (value & 0x01);
      
      paletteIndex += 32;
      
      register_OCPS = value;
      register_OCPD = (paletteHiLo ?
                        (gbPalette[paletteIndex] >> 8) :
                        (gbPalette[paletteIndex] & 0x00ff));
      return;
    }
  }
  break;
  
  // OCPD
  case 0x6b: {
    if(gbCgbMode) {
      int v = register_OCPS;
      int paletteIndex = (v & 0x3f) >> 1;
      int paletteHiLo  = (v & 0x01);
      
      paletteIndex += 32;
      
      register_OCPD = value;
      gbPalette[paletteIndex] = (paletteHiLo ?
                                 ((value << 8) | (gbPalette[paletteIndex] & 0xff)) :
                                 ((gbPalette[paletteIndex] & 0xff00) | (value))) & 0x7fff;
      if(register_OCPS & 0x80) {
        int index = ((register_OCPS & 0x3f) + 1) & 0x3f;
        
        register_OCPS = (register_OCPS & 0x80) | index;
        
        register_OCPD = (index & 1 ?
                          (gbPalette[(index>>1) + 32] >> 8) :
                          (gbPalette[(index>>1) + 32] & 0x00ff));
        
      }      
      return;
    }
  }
  break;

  case 0x6c:
   register_FF6C = value & 1;
   break;
  
  // SVBK
  case 0x70: {
    if(gbCgbMode) {
      value = value & 7;

      int bank = value;
      if(value == 0)
        bank = 1;

      if(bank == gbWramBank)
        return;
      
      int wramAddress = bank * 0x1000;
      gbMemoryMap[0x0d] = &gbWram[wramAddress];

      MDFNMP_AddRAM(0x1000, 0xD000, &gbWram[wramAddress]);

      gbWramBank = bank;
      register_SVBK = value;
      return;
    }
  }
  break;

  case 0x80 ... 0xFE: 
	  HRAM[address & 0x7F] = value;
	  return;

  case 0xff: 
    register_IE = value;
    return;
  }
  
  //printf("%04x\n", address);
  //gbMemory[address - 0x8000] = value;
}

uint8 gbReadMemory(uint16 address)
{
  uint8 retval = 0xFF;

  if(address < 0xa000)
    retval = gbMemoryMap[address>>12][address&0x0fff];
  else if(address < 0xc000) 
  {
    if(mapperReadRAM)
      retval = mapperReadRAM(address);
    else if(mapper && gbMemoryMap[address >> 12])
      retval = gbMemoryMap[address >> 12][address & 0x0fff & gbRamSizeMask];
  }
  else if(address < 0xfe00)
  {
   unsigned int page = (address >> 12);
   if(page >= 0xE) page -= 2;
   retval = gbMemoryMap[page][address & 0x0fff];
  }
  else if(address < 0xfea0)
   retval = gbOAM[address & 0xFF];
  else if(address >= 0xff00) 
  {
    switch(address & 0x00ff) 
    {
     case 0x00:
      {
        int b = register_P1;

        if((b & 0x30) == 0x20) {
          b &= 0xf0;

	  b |= ((gbJoymask >> 4) & 0xF) ^ 0xF;

          register_P1 = b;
        } else if((b & 0x30) == 0x10) {
          b &= 0xf0;

          b |= ((gbJoymask >> 0) & 0xF) ^ 0xF;
          
          register_P1 = b;
        } else {
            register_P1 = 0xff;
        }
      }
      retval = register_P1;
      break;
    case 0x01:
      retval = register_SB;
      break;
    case 0x02:
      retval = register_SC;
      break;
    case 0x04:
      retval = register_DIV;
      break;
    case 0x05:
	retval = register_TIMA;
	break;
    case 0x06:
	retval = register_TMA;
	break;
    case 0x07:
	retval = 0xf8 | register_TAC;
	break;
    case 0x0f:
	retval = 0xe0 | register_IF;
	break;
    case 0x10 ... 0x3f:
	retval = MDFNGBSOUND_Read(SoundTS, address);
	break;
    case 0x40:
	retval = register_LCDC;
	break;
    case 0x41:
	retval = 0x80 | register_STAT;
	break;
    case 0x42:
	retval = register_SCY;
	break;
    case 0x43:
	retval = register_SCX;
	break;
    case 0x44:
	retval = register_LY;
	break;
    case 0x45:
	retval = register_LYC;
	break;
    case 0x46:
	retval = register_DMA;
	break;
    case 0x47:
	retval = gbBgp[0] | (gbBgp[1] << 2) | (gbBgp[2] << 4) | (gbBgp[3] << 6);
	break;
    case 0x48:
	retval = gbObp0[0] | (gbObp0[1] << 2) | (gbObp0[2] << 4) | (gbObp0[3] << 6);
	break;
    case 0x49:
	retval = gbObp1[0] | (gbObp1[1] << 2) | (gbObp1[2] << 4) | (gbObp1[3] << 6);
	break;
    case 0x4a:
	retval = register_WY;
	break;
    case 0x4b:
	retval = register_WX;
	break;
    case 0x4d:
	retval = register_KEY1;
	break;
    case 0x4f:
	retval = 0xfe | register_VBK;
	break;
    case 0x51:
	retval = register_HDMA1;
	break;
    case 0x52:
	retval = register_HDMA2;
	break;
    case 0x53:
	retval = register_HDMA3;
	break;
    case 0x54:
	retval = register_HDMA4;
	break;
    case 0x55:
	retval = register_HDMA5;
	break;
    case 0x56:
	retval = register_RP;
	break;
    case 0x68:
	retval = register_BCPS;
	break;
    case 0x69:
	retval = register_BCPD;
	break;
    case 0x6a:
	retval = register_OCPS;
	break;
    case 0x6b:
	retval = register_OCPD;
	break;
    case 0x6c:
	retval = (register_FF6C & 1) | 0xFE;
	break;
    case 0x70:
	retval = (0xf8 | register_SVBK);
	break;
    case 0x72:
	retval = gbCgbMode ? register_FF72 : 0xFF;
	break;
    case 0x73:
	retval = gbCgbMode ? register_FF73 : 0xFF;
	break;
    case 0x74:
	retval = gbCgbMode ? register_FF74 : 0xFF;
	break;
    case 0x75:
	retval = gbCgbMode ? ((register_FF75 &~0x8F) | 0x8F) : 0xFF;
	break;
    case 0x76:
	retval = gbCgbMode ? 0x00 : 0xFF;
	break;
    case 0x77:
	retval = gbCgbMode ? 0x00 : 0xFF;
	break;
    case 0x80 ... 0xFE: 
	retval = HRAM[address & 0x7F];
	break;
    case 0xff:
	retval = register_IE;
	break;
    }
  }

 if(SubCheatsOn)
 {
  std::vector<SUBCHEAT>::iterator chit;

  for(chit = SubCheats[address & 0x7].begin(); chit != SubCheats[address & 0x7].end(); chit++)
  {
   if(address == chit->addr)
   {
    //printf("%02x %02x %02x\n", retval, chit->value, chit->compare);
    if(chit->compare == -1 || chit->compare == retval)
    {
     retval = chit->value;
    }
   }
  }
 }
  
 return(retval);
 //printf("Unknown read: %04x\n", address);
 //return(0xFF);
 //return gbMemoryMap[address>>12][address & 0x0fff];
}

void gbSpeedSwitch()
{
  if(gbSpeed == 0) {
    gbSpeed = 1;
    GBLCD_MODE_0_CLOCK_TICKS = 51 * 2; //127; //51 * 2;
    GBLCD_MODE_1_CLOCK_TICKS = 1140 * 2;
    GBLCD_MODE_2_CLOCK_TICKS = 20 * 2; //52; //20 * 2;
    GBLCD_MODE_3_CLOCK_TICKS = 43 * 2; //99; //43 * 2;
    GBDIV_CLOCK_TICKS = 64 * 2;
    GBLY_INCREMENT_CLOCK_TICKS = 114 * 2;
    GBTIMER_MODE_0_CLOCK_TICKS = 256; //256*2;
    GBTIMER_MODE_1_CLOCK_TICKS = 4; //4*2;
    GBTIMER_MODE_2_CLOCK_TICKS = 16; //16*2;
    GBTIMER_MODE_3_CLOCK_TICKS = 64; //64*2;
    GBSERIAL_CLOCK_TICKS = 128 * 2;
    gbDivTicks *= 2;
    gbLcdTicks *= 2;
    gbLcdLYIncrementTicks *= 2;
    //    timerTicks *= 2;
    //    timerClockTicks *= 2;
    gbSerialTicks *= 2;
    //    synchronizeTicks *= 2;
    //    SYNCHRONIZE_CLOCK_TICKS *= 2;
  } else {
    gbSpeed = 0;
    GBLCD_MODE_0_CLOCK_TICKS = 51;
    GBLCD_MODE_1_CLOCK_TICKS = 1140;
    GBLCD_MODE_2_CLOCK_TICKS = 20;
    GBLCD_MODE_3_CLOCK_TICKS = 43;
    GBDIV_CLOCK_TICKS = 64;
    GBLY_INCREMENT_CLOCK_TICKS = 114;
    GBTIMER_MODE_0_CLOCK_TICKS = 256;
    GBTIMER_MODE_1_CLOCK_TICKS = 4;
    GBTIMER_MODE_2_CLOCK_TICKS = 16;
    GBTIMER_MODE_3_CLOCK_TICKS = 64;
    GBSERIAL_CLOCK_TICKS = 128;
    gbDivTicks /= 2;
    gbLcdTicks /= 2;
    gbLcdLYIncrementTicks /= 2;
    //    timerTicks /= 2;
    //    timerClockTicks /= 2;
    gbSerialTicks /= 2;
    //    synchronizeTicks /= 2;
    //    SYNCHRONIZE_CLOCK_TICKS /= 2;    
  }
}

void gbReset()
{
 GBZ80_Reset();

  register_DIV = 0;
  register_TIMA = 0;
  register_TMA = 0;
  register_TAC = 0;
  register_IF = 1;
  register_LCDC = 0x91;
  register_STAT = 0;
  register_SCY = 0;
  register_SCX = 0;  
  register_LY = 0;  
  register_LYC = 0;
  register_DMA = 0;
  register_WY = 0;
  register_WX = 0;
  register_VBK = 0;
  register_HDMA1 = 0;
  register_HDMA2 = 0;
  register_HDMA3 = 0;
  register_HDMA4 = 0;
  register_HDMA5 = 0;
  register_SVBK = 0;
  register_IE = 0;  

  if(gbCgbMode) 
  {
    register_HDMA5 = 0xff;
    register_BCPS = 0xc0;
    register_OCPS = 0xc0;    
  } 
  else 
  {
    for(int i = 0; i < 8; i++)
    {
     int fun = 3 - (i & 3);
     fun *= 6;
     fun += 4;
     gbPalette[i] = fun | (fun << 5) | (fun << 10);
    }
  }

  if(gbSpeed) {
    gbSpeedSwitch();
    register_KEY1 = 0;
  }
  
  gbDivTicks = GBDIV_CLOCK_TICKS;
  gbLcdMode = GBLCDM_OAM;
  gbLcdTicks = GBLCD_MODE_2_CLOCK_TICKS;
  gbLcdLYIncrementTicks = 0;
  gbTimerTicks = 0;
  gbTimerClockTicks = 0;
  gbSerialTicks = 0;
  gbSerialBits = 0;
  gbSerialOn = 0;
  gbWindowLine = -1;
  gbTimerOn = 0;
  gbTimerMode = 0;
  //  gbSynchronizeTicks = GBSYNCHRONIZE_CLOCK_TICKS;
  gbSpeed = 0;
  gbJoymask = 0;
  
  if(gbCgbMode) {
    gbSpeed = 0;
    gbHdmaOn = 0;
    gbHdmaSource = 0x0000;
    gbHdmaDestination = 0x8000;
    gbVramBank = 0;
    gbWramBank = 1;
    register_LY = 0x90;
    gbLcdMode = GBLCDM_VBLANK;
    for(int i = 0; i < 64; i++)
      gbPalette[i] = 0x7fff;
  }

  for(int i =0; i < 4; i++)
    gbBgp[i] = gbObp0[i] = gbObp1[i] = i;

  memset(&gbDataMBC1,0, sizeof(gbDataMBC1));
  gbDataMBC1.mapperROMBank = 1;

  gbDataMBC2.mapperRAMEnable = 0;
  gbDataMBC2.mapperROMBank = 1;

  memset(&gbDataMBC3,0, 6 * sizeof(int));
  gbDataMBC3.mapperROMBank = 1;

  memset(&gbDataMBC5, 0, sizeof(gbDataMBC5));
  gbDataMBC5.mapperROMBank = 1;

  memset(&gbDataHuC1, 0, sizeof(gbDataHuC1));
  gbDataHuC1.mapperROMBank = 1;

  memset(&gbDataHuC3, 0, sizeof(gbDataHuC3));
  gbDataHuC3.mapperROMBank = 1;

  gbMemoryMap[0x00] = &gbRom[0x0000];
  gbMemoryMap[0x01] = &gbRom[0x1000];
  gbMemoryMap[0x02] = &gbRom[0x2000];
  gbMemoryMap[0x03] = &gbRom[0x3000];
  gbMemoryMap[0x04] = &gbRom[0x4000];
  gbMemoryMap[0x05] = &gbRom[0x5000];
  gbMemoryMap[0x06] = &gbRom[0x6000];
  gbMemoryMap[0x07] = &gbRom[0x7000];
  gbMemoryMap[0x08] = &gbVram[0x0000];
  gbMemoryMap[0x09] = &gbVram[0x1000];
  gbMemoryMap[0x0a] = NULL;
  gbMemoryMap[0x0b] = NULL;
  gbMemoryMap[0x0c] = &gbWram[0x0000];
  gbMemoryMap[0x0d] = &gbWram[0x1000];
  gbMemoryMap[0x0e] = NULL; 
  gbMemoryMap[0x0f] = NULL; 

 if(gbRam) 
 {
  gbMemoryMap[0x0a] = &gbRam[0x0000];

  if(gbRamSize > 0x1000)
   gbMemoryMap[0x0b] = &gbRam[0x1000];
  else
   gbMemoryMap[0x0b] = gbMemoryMap[0x0a];
 }

 MDFNGBSOUND_Reset();

 // BIOS simulate
 MDFNGBSOUND_Write(0, 0xFF26, 0x80);
 MDFNGBSOUND_Write(0, 0xFF11, 0x80);
 MDFNGBSOUND_Write(0, 0xFF12, 0xF3);
 MDFNGBSOUND_Write(0, 0xFF25, 0xF3);
 MDFNGBSOUND_Write(0, 0xFF24, 0x77);
}

static void gbPower(void)
{
  if(gbCgbMode)
  {
   memset(gbWram,0,0x8000);
   memset(gbVram, 0, 0x4000);
  }
  else
  {
   memset(gbWram, 0x00, 0x2000);
   memset(gbVram, 0x00, 0x2000);
  }
  memset(gbOAM, 0x00, 0xA0);
  memset(HRAM, 0x00, 0x80);

  if(gbRam && !gbBattery)
   memset(gbRam, 0xFF, gbRamSize);

  gbReset();
}

void gbWriteSaveMBC1(const char * name)
{
  gzFile file = gzopen(name, "wb");

  if(file == NULL) {
    return;
  }
  
  gzwrite(file, gbRam, gbRamSize);
  
  gzclose(file);
}

void gbWriteSaveMBC2(const char * name)
{
  gzFile file = gzopen(name, "wb");

  if(file == NULL) {
    return;
  }
  gzwrite(file, gbRam, 256 * 2);

  gzclose(file);
}

void gbWriteSaveMBC3(const char * name, bool extendedSave)
{
  gzFile file = gzopen(name, "wb");

  if(file == NULL) {
    return;
  }
  
  gzwrite(file, gbRam, gbRamSize);

  if(extendedSave)
  {
    gzwrite(file, &gbDataMBC3.mapperSeconds,           
           10*sizeof(int) + sizeof(time_t));
  }  
  gzclose(file);
}

void gbWriteSaveMBC5(const char * name)
{
  gzFile file = gzopen(name,"wb");

  if(file == NULL) {
    return;
  }
  
  gzwrite(file, gbRam, gbRamSize);

  gzclose(file);
}

void gbWriteSaveMBC7(const char * name)
{
  gzFile file = gzopen(name, "wb");

  if(file == NULL) {
    return;
  }
  gzwrite(file, gbRam, 256);
  gzclose(file);
}

bool gbReadSaveMBC1(const char * name)
{
  gzFile file = gzopen(name, "rb");

  if(file == NULL) {
    return false;
  }
  
  int read = gzread(file, gbRam, gbRamSize);
  
  if(read != gbRamSize) {
    gzclose(file);
    return false;
  }
  
  gzclose(file);
  return true;
}

bool gbReadSaveMBC2(const char * name)
{
  gzFile file = gzopen(name, "rb");

  if(file == NULL) {
    return false;
  }

  int read = gzread(file, gbRam, 256 * 2);
  if(read != 256 * 2) {
    gzclose(file);
    return false;
  }
  gzclose(file);
  return true;
}

bool gbReadSaveMBC3(const char * name)
{
  gzFile file = gzopen(name, "rb");

  if(file == NULL) {
    return false;
  }

  int read = gzread(file, gbRam, gbRamSize);

  bool res = true;
  
  if(read != gbRamSize) {
  } else {
    read = gzread(file,
                  &gbDataMBC3.mapperSeconds,
                  sizeof(int)*10 + sizeof(time_t));

    if(read != (sizeof(int)*10 + sizeof(time_t)) && read != 0) 
    {
      res = false;
    }
  }
  
  gzclose(file);
  return res;
}

bool gbReadSaveMBC5(const char * name)
{
  gzFile file = gzopen(name, "rb");

  if(file == NULL) {
    return false;
  }

  int read = gzread(file, gbRam, gbRamSize);
  
  if(read != gbRamSize) {
    gzclose(file);
    return false;
  }
  
  gzclose(file);
  return true;
}

bool gbReadSaveMBC7(const char * name)
{
  gzFile file = gzopen(name, "rb");

  if(file == NULL) {
    return false;
  }

  int read = gzread(file, gbRam, 256);
  
  if(read != 256) {
    gzclose(file);
    return false;
  }
  
  gzclose(file);
  return true;
}

bool gbWriteBatteryFile(const char *file, bool extendedSave)
{
  if(gbBattery) {
    int type = gbRom[0x147];

    switch(type) {
    case 0x03:
      gbWriteSaveMBC1(file);
      break;
    case 0x06:
      gbWriteSaveMBC2(file);
      break;
    case 0x0f:
    case 0x10:
    case 0x13:
      gbWriteSaveMBC3(file, extendedSave);
      break;
    case 0x1b:
    case 0x1e:
      gbWriteSaveMBC5(file);
      break;
    case 0x22:
      gbWriteSaveMBC7(file);
      break;
    case 0xff:
      gbWriteSaveMBC1(file);
      break;
    }
  }
  return true;
}

bool gbWriteBatteryFile(const char *file)
{
  gbWriteBatteryFile(file, true);
  return true;
}

bool gbReadBatteryFile(const char *file)
{
  bool res = false;
  if(gbBattery) {
    int type = gbRom[0x147];
    
    switch(type) {
    case 0x03:
      res = gbReadSaveMBC1(file);
      break;
    case 0x06:
      res = gbReadSaveMBC2(file);
      break;
    case 0x0f:
    case 0x10:
    case 0x13:
      if(!gbReadSaveMBC3(file)) 
	{
	time_t tmp;

        time(&tmp);
        gbDataMBC3.mapperLastTime = tmp;
        struct tm *lt;
        lt = localtime(&tmp);
        gbDataMBC3.mapperSeconds = lt->tm_sec;
        gbDataMBC3.mapperMinutes = lt->tm_min;
        gbDataMBC3.mapperHours = lt->tm_hour;
        gbDataMBC3.mapperDays = lt->tm_yday & 255;
        gbDataMBC3.mapperControl = (gbDataMBC3.mapperControl & 0xfe) |
          (lt->tm_yday > 255 ? 1: 0);
        res = false;
        break;
      }
      res = true;
      break;
    case 0x1b:
    case 0x1e:
      res = gbReadSaveMBC5(file);
      break;
    case 0x22:
      res = gbReadSaveMBC7(file);
    case 0xff:
      res = gbReadSaveMBC1(file);
      break;
    }
  }
  return res;
}

bool gbReadGSASnapshot(const char *fileName)
{
  FILE *file = fopen(fileName, "rb");
    
  if(!file) {
    return false;
  }
  
  //  long size = ftell(file);
  fseek(file, 0x4, SEEK_SET);
  char buffer[16];
  char buffer2[16];
  fread(buffer, 1, 15, file);
  buffer[15] = 0;
  memcpy(buffer2, &gbRom[0x134], 15);
  buffer2[15] = 0;
  if(memcmp(buffer, buffer2, 15)) {
    fclose(file);
    return false;
  }
  fseek(file, 0x13, SEEK_SET);
  int read = 0;
  int toRead = 0;
  switch(gbRom[0x147]) {
  case 0x03:
  case 0x0f:
  case 0x10:
  case 0x13:
  case 0x1b:
  case 0x1e:
  case 0xff:
    read = fread(gbRam, 1, gbRamSize, file);
    toRead = gbRamSize;
    break;
  case 0x06:
  case 0x22:
    read = fread(gbRam,1,256,file);
    toRead = 256;
    break;
  default:
    fclose(file);
    return false;
  }    
  fclose(file);

  gbReset();
  return true;  
}

static SFORMAT Joy_StateRegs[] =
{
 SFVAR(gbJoymask),
 SFEND
};

static SFORMAT MBC1_StateRegs[] =
{
 SFVARN(gbDataMBC1.mapperRAMEnable, "RAME"),
 SFVARN(gbDataMBC1.mapperROMBank, "ROMB"),
 SFVARN(gbDataMBC1.mapperRAMBank,"RAMB"),
 SFVARN(gbDataMBC1.mapperMemoryModel, "MEMM"),
 SFEND
};

static SFORMAT MBC2_StateRegs[] =
{
 SFVARN(gbDataMBC2.mapperRAMEnable, "RAME"),
 SFVARN(gbDataMBC2.mapperROMBank, "ROMB"),
 SFEND
};

static SFORMAT MBC3_StateRegs[] =
{
 SFVARN(gbDataMBC3.mapperRAMEnable, "RAME"),
 SFVARN(gbDataMBC3.mapperROMBank, "ROMB"),
 SFVARN(gbDataMBC3.mapperRAMBank, "RAMB"),
 SFVARN(gbDataMBC3.mapperRAMAddress, "RAMA"),
 SFVARN(gbDataMBC3.mapperClockLatch, "CLKL"),
 SFVARN(gbDataMBC3.mapperClockRegister, "CLKR"),
 SFVARN(gbDataMBC3.mapperSeconds, "SEC"),
 SFVARN(gbDataMBC3.mapperHours, "HOUR"),
 SFVARN(gbDataMBC3.mapperDays, "DAY"),
 SFVARN(gbDataMBC3.mapperControl, "CTRL"),

 SFVARN(gbDataMBC3.mapperLSeconds, "LSEC"),
 SFVARN(gbDataMBC3.mapperLHours, "LHUR"),
 SFVARN(gbDataMBC3.mapperLDays, "LDAY"),
 SFVARN(gbDataMBC3.mapperLControl, "LCTR"),
 SFVARN(gbDataMBC3.mapperLastTime, "LTIM"),
 SFEND
};

static SFORMAT MBC5_StateRegs[] =
{
 SFVAR(gbDataMBC5.mapperRAMEnable),
 SFVAR(gbDataMBC5.mapperROMBank),
 SFVAR(gbDataMBC5.mapperRAMBank),
 SFVAR(gbDataMBC5.mapperROMHighAddress),
 SFVAR(gbDataMBC5.mapperRAMAddress),
 SFVAR(gbDataMBC5.isRumbleCartridge),
 SFEND
};

static SFORMAT MBC7_StateRegs[] =
{
 { &gbDataMBC7.mapperRAMEnable, sizeof(int) | MDFNSTATE_RLSB, "RAME" },
 { &gbDataMBC7.mapperROMBank, sizeof(int) | MDFNSTATE_RLSB, "ROMB" },
 { &gbDataMBC7.mapperRAMBank, sizeof(int) | MDFNSTATE_RLSB, "RAMB" },
 { &gbDataMBC7.mapperRAMAddress, sizeof(int) | MDFNSTATE_RLSB, "RAMA" },
 { &gbDataMBC7.cs, sizeof(int) | MDFNSTATE_RLSB, "CS\0" },
 { &gbDataMBC7.sk, sizeof(int) | MDFNSTATE_RLSB, "SK\0" },
 { &gbDataMBC7.state, sizeof(int) | MDFNSTATE_RLSB, "STTE" },
 { &gbDataMBC7.buffer, sizeof(int) | MDFNSTATE_RLSB, "BUF" },
 { &gbDataMBC7.idle, sizeof(int) | MDFNSTATE_RLSB, "IDLE" },
 { &gbDataMBC7.count, sizeof(int) | MDFNSTATE_RLSB, "CONT" },
 { &gbDataMBC7.code, sizeof(int) | MDFNSTATE_RLSB, "CODE" },
 { &gbDataMBC7.address, sizeof(int) | MDFNSTATE_RLSB, "ADDR" },
 { &gbDataMBC7.writeEnable, sizeof(int) | MDFNSTATE_RLSB, "WRE" },
 { &gbDataMBC7.value, sizeof(int) | MDFNSTATE_RLSB, "VALU" },
 SFEND
};

static SFORMAT HuC1_StateRegs[] =
{
 SFVARN(gbDataHuC1.mapperRAMEnable, "RAME"),
 SFVARN(gbDataHuC1.mapperROMBank, "ROMB"),
 SFVARN(gbDataHuC1.mapperRAMBank, "RAMB"),
 SFVARN(gbDataHuC1.mapperMemoryModel, "MEMM"),
 SFVARN(gbDataHuC1.mapperROMHighAddress, "ROMH"),
 SFVARN(gbDataHuC1.mapperRAMAddress, "RAMA"),
 SFEND
};

static SFORMAT HuC3_StateRegs[] =
{
 SFVARN(gbDataHuC3.mapperRAMEnable, "RAME"),
 SFVARN(gbDataHuC3.mapperROMBank, "ROMB"),
 SFVARN(gbDataHuC3.mapperRAMBank, "RAMB"),
 SFVARN(gbDataHuC3.mapperRAMAddress, "RAMA"),
 SFVARN(gbDataHuC3.mapperAddress, "ADDR"),
 SFVARN(gbDataHuC3.mapperRAMFlag, "RAMF"),
 SFVARN(gbDataHuC3.mapperRAMValue, "RAMV"),
 SFVARN(gbDataHuC3.mapperRegister1, "REG1"),
 SFVARN(gbDataHuC3.mapperRegister2, "REG2"),
 SFVARN(gbDataHuC3.mapperRegister3, "REG3"),
 SFVARN(gbDataHuC3.mapperRegister4, "REG4"),
 SFVARN(gbDataHuC3.mapperRegister5, "REG5"),
 SFVARN(gbDataHuC3.mapperRegister6, "REG6"),
 SFVARN(gbDataHuC3.mapperRegister7, "REG7"),
 SFVARN(gbDataHuC3.mapperRegister8, "REG8"),

 SFEND
};


static SFORMAT gbSaveGameStruct[] = 
{
  SFVAR(GBLCD_MODE_0_CLOCK_TICKS),
  SFVAR(GBLCD_MODE_1_CLOCK_TICKS),
  SFVAR(GBLCD_MODE_2_CLOCK_TICKS),
  SFVAR(GBLCD_MODE_3_CLOCK_TICKS),
  SFVAR(GBDIV_CLOCK_TICKS),
  SFVAR(GBLY_INCREMENT_CLOCK_TICKS),
  SFVAR(GBTIMER_MODE_0_CLOCK_TICKS),
  SFVAR(GBTIMER_MODE_1_CLOCK_TICKS),
  SFVAR(GBTIMER_MODE_2_CLOCK_TICKS),
  SFVAR(GBTIMER_MODE_3_CLOCK_TICKS),
  SFVAR(GBSERIAL_CLOCK_TICKS),
  SFVAR(GBSYNCHRONIZE_CLOCK_TICKS),
  SFVAR(gbDivTicks),
  SFVAR(gbLcdMode),
  SFVAR(gbLcdTicks),
  SFVAR(gbLcdLYIncrementTicks),
  SFVAR(gbTimerTicks),
  SFVAR(gbTimerClockTicks),
  SFVAR(gbSerialTicks),
  SFVAR(gbSerialBits),
  SFVAR(gbSynchronizeTicks),
  SFVAR(gbTimerOn),
  SFVAR(gbTimerMode),
  SFVAR(gbSerialOn),
  SFVAR(gbWindowLine),
  //SFVAR(gbCgbMode),
  SFVAR(gbVramBank),
  SFVAR(gbWramBank),
  SFVAR(gbHdmaSource),
  SFVAR(gbHdmaDestination),
  SFVAR(gbHdmaBytes),
  SFVAR(gbHdmaOn),
  SFVAR(gbSpeed),
  SFVAR(gbDmaTicks),
  SFVAR(register_P1),
  SFVAR(register_SB),
  SFVAR(register_SC),
  SFVAR(register_DIV),
  SFVAR(register_TIMA),
  SFVAR(register_TMA),
  SFVAR(register_TAC),
  SFVAR(register_IF),
  SFVAR(register_LCDC),
  SFVAR(register_STAT),
  SFVAR(register_SCY),
  SFVAR(register_SCX),
  SFVAR(register_LY),
  SFVAR(register_LYC),
  SFVAR(register_DMA),
  SFVAR(register_WY),
  SFVAR(register_WX),
  SFVAR(register_VBK),
  SFVAR(register_HDMA1),
  SFVAR(register_HDMA2),
  SFVAR(register_HDMA3),
  SFVAR(register_HDMA4),
  SFVAR(register_HDMA5),
  SFVAR(register_RP),
  SFVAR(register_FF6C),
  SFVAR(register_SVBK),
  SFVAR(register_FF72),
  SFVAR(register_FF73),
  SFVAR(register_FF74),
  SFVAR(register_FF75),
  SFVAR(register_IE),
  SFARRAYN(gbBgp, 4, "BGP"),
  SFARRAYN(gbObp0, 4, "OBP0"),
  SFARRAYN(gbObp1, 4, "OBP1"),
  SFEND
};

static void CloseGame(void)
{
 MDFN_FlushGameCheats(0);
 gbWriteBatteryFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str());

 if(gbRam != NULL) 
 {
  free(gbRam);
  gbRam = NULL;
 }

 if(gbRom != NULL) 
 {
  free(gbRom);
  gbRom = NULL;
 }

 if(gbLineBuffer != NULL) 
 {
  free(gbLineBuffer);
  gbLineBuffer = NULL;
 }

 if(gbVram != NULL) 
 {
  free(gbVram);
  gbVram = NULL;
 }

 if(gbWram != NULL) 
 {
  free(gbWram);
  gbWram = NULL;
 }

 if(gbColorFilter)
 {
  free(gbColorFilter);
  gbColorFilter = NULL;
 }

}

static void StateRest(int version)
{
  gbMemoryMap[0x00] = &gbRom[0x0000];
  gbMemoryMap[0x01] = &gbRom[0x1000];
  gbMemoryMap[0x02] = &gbRom[0x2000];
  gbMemoryMap[0x03] = &gbRom[0x3000];
  gbMemoryMap[0x04] = &gbRom[0x4000];
  gbMemoryMap[0x05] = &gbRom[0x5000];
  gbMemoryMap[0x06] = &gbRom[0x6000];
  gbMemoryMap[0x07] = &gbRom[0x7000];
  gbMemoryMap[0x08] = &gbVram[0x0000];
  gbMemoryMap[0x09] = &gbVram[0x1000];
  gbMemoryMap[0x0a] = NULL;
  gbMemoryMap[0x0b] = NULL;
  gbMemoryMap[0x0c] = &gbWram[0x0000];
  gbMemoryMap[0x0d] = &gbWram[0x1000];
  gbMemoryMap[0x0e] = NULL;
  gbMemoryMap[0x0f] = NULL;

  if(gbRam)
  {
   gbMemoryMap[0x0a] = &gbRam[0x0000];
   gbMemoryMap[0x0b] = &gbRam[0x1000];
  }

  int type = gbRom[0x147];

  switch(type) {
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
    // MBC 1
    memoryUpdateMapMBC1();
    break;
  case 0x05:
  case 0x06:
    // MBC2
    memoryUpdateMapMBC2();
    break;
  case 0x0f:
  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
    // MBC 3
    memoryUpdateMapMBC3();
    break;
  case 0x19:
  case 0x1a:
  case 0x1b:
    // MBC5
    memoryUpdateMapMBC5();
    break;
  case 0x1c:
  case 0x1d:
  case 0x1e:
    // MBC 5 Rumble
    memoryUpdateMapMBC5();
    break;
  case 0x22:
    // MBC 7
    memoryUpdateMapMBC7();
    break;
  case 0xfe:
    // HuC3
    memoryUpdateMapHuC3();
    break;
  case 0xff:
    // HuC1
    memoryUpdateMapHuC1();
    break;
  }

  if(gbCgbMode)
  {
    int value = register_SVBK;
    if(value == 0)
      value = 1;

    gbMemoryMap[0x08] = &gbVram[register_VBK * 0x2000];
    gbMemoryMap[0x09] = &gbVram[register_VBK * 0x2000 + 0x1000];
    gbMemoryMap[0x0d] = &gbWram[value * 0x1000];

    gbVramBank = register_VBK;
    gbWramBank = value;
  }

}

uint32 gblayerSettings;

static bool TestMagic(const char *name, MDFNFILE *fp)
{
 static const uint8 GBMagic[8] = { 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B };

 if(fp->size < 0x10C || memcmp(fp->data + 0x104, GBMagic, 8))
  return(FALSE);

 return(TRUE);
}

static int Load(const char *name, MDFNFILE *fp)
{
 if(!TestMagic(name, fp))
  return(-1);

 if(!(gbColorFilter = (uint32 *)MDFN_malloc(32768 * sizeof(uint32), _("GB Color Map"))))
 {
  return(0);
 }

 gbRom = (uint8 *)malloc(fp->size);
 memcpy(gbRom, fp->data, fp->size);
 gbRomSize = fp->size;

 md5_context md5;
 md5.starts();
 md5.update(gbRom, gbRomSize);
 md5.finish(MDFNGameInfo->MD5);

 MDFNGameInfo->GameSetMD5Valid = FALSE;

 MDFN_printf(_("ROM:       %dKiB\n"), (gbRomSize + 1023) / 1024);
 MDFN_printf(_("ROM CRC32: 0x%08x\n"), (unsigned int)crc32(0, gbRom, gbRomSize));
 MDFN_printf(_("ROM MD5:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());
 MDFNMP_Init(128, (65536 + 32768) / 128); // + 32768 for GBC WRAM for supported GameShark cheats with RAM page numbers

 MDFN_LoadGameCheats(NULL);

 if(MDFN_GetSettingB("gb.forcemono"))
 {
  MDFNGameInfo->soundchan = 1;
  MDFNGBSOUND_Init(1);
 }
 else
 {
  MDFNGameInfo->soundchan = 2;
  MDFNGBSOUND_Init(0);
 }

 if(!gbUpdateSizes())
 {
  return(0);
 }

 gbReadBatteryFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str());
 gblayerSettings = 0xFF;
 return(1);
}

static bool gbUpdateSizes()
{
  if(gbRom[0x148] > 8) 
  {
    MDFN_PrintError(_("Invalid ROM size"));
    return false;
  }

  //MDFN_printf("ROM Size: %d\n", gbRomSizes[gbRom[0x148]]);

  if(gbRomSize < gbRomSizes[gbRom[0x148]]) {
    gbRom = (uint8 *)realloc(gbRom, gbRomSizes[gbRom[0x148]]);
  }
  gbRomSize = gbRomSizes[gbRom[0x148]];
  gbRomSizeMask = gbRomSizesMasks[gbRom[0x148]];

  if(gbRom[0x149] > 5) 
  {
    MDFN_PrintError(_("Invalid RAM size"));
    return false;
  }

  gbRamSize = gbRamSizes[gbRom[0x149]];
  gbRamSizeMask = gbRamSizesMasks[gbRom[0x149]];

  //MDFN_printf(_("External RAM: %dKiB\n"), gbRamSize / 1024);


  int type = gbRom[0x147];

  mapperReadRAM = NULL;
  
  switch(type) {
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
    // MBC 1
    mapper = mapperMBC1ROM;
    mapperRAM = mapperMBC1RAM;
    break;
  case 0x05:
  case 0x06:
    // MBC2
    mapper = mapperMBC2ROM;
    mapperRAM = mapperMBC2RAM;
    gbRamSize = 0x200;
    gbRamSizeMask = 0x1ff;
    break;
  case 0x0f:
  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
    // MBC 3
    mapper = mapperMBC3ROM;
    mapperRAM = mapperMBC3RAM;
    mapperReadRAM = mapperMBC3ReadRAM;
    break;
  case 0x19:
  case 0x1a:
  case 0x1b:
    // MBC5
    mapper = mapperMBC5ROM;
    mapperRAM = mapperMBC5RAM;
    break;
  case 0x1c:
  case 0x1d:
  case 0x1e:
    // MBC 5 Rumble
    mapper = mapperMBC5ROM;
    mapperRAM = mapperMBC5RAM;
    break;
  case 0x22:
    // MBC 7
    mapper = mapperMBC7ROM;
    mapperRAM = mapperMBC7RAM;
    mapperReadRAM = mapperMBC7ReadRAM;
    break;
  case 0xfe:
    // HuC3
    mapper = mapperHuC3ROM;
    mapperRAM = mapperHuC3RAM;
    mapperReadRAM = mapperHuC3ReadRAM;
    break;
  case 0xff:
    // HuC1
    mapper = mapperHuC1ROM;
    mapperRAM = mapperHuC1RAM;
    break;
  default:
    return false;
  }

  switch(type) {
  case 0x03:
  case 0x06:
  case 0x0f:
  case 0x10:
  case 0x13:
  case 0x1b:
  case 0x1d:
  case 0x1e:
  case 0x22:
  case 0xff:
    gbBattery = 1;
    break;
  }

  if(gbRamSize) {
    gbRam = (uint8 *)malloc(gbRamSize);
    memset(gbRam, 0xFF, gbRamSize);
  }

  if(gbRom[0x143] & 0x80) 
  {
    if(gbEmulatorType == 0 ||
       gbEmulatorType == 1 ||
       gbEmulatorType == 4 ||
       gbEmulatorType == 5) {
      gbCgbMode = 1;
      memset(gbPalette,0, 2*128);
    } else {
      gbCgbMode = 0;
    }
  } else
    gbCgbMode = 0;

  if(gbCgbMode)
  {
   gbWram = (uint8 *)malloc(0x8000);
   memset(gbWram,0,0x8000);
   MDFNMP_AddRAM(0x8000, 0x10000, gbWram);

   gbVram = (uint8 *)malloc(0x4000);
   memset(gbVram, 0, 0x4000);
  }
  else
  {
   gbWram = (uint8 *)malloc(0x2000);
   memset(gbWram,0,0x2000);
   gbVram = (uint8 *)malloc(0x2000);
   memset(gbVram, 0, 0x2000);
  }

  MDFNMP_AddRAM(0x80, 0xFF80, HRAM);
  MDFNMP_AddRAM(0x2000, 0xC000, gbWram);

  if(gbRam)
   MDFNMP_AddRAM(gbRamSize > 8192 ? 8192 : gbRamSize, 0xA000, gbRam);

  gbLineBuffer = (uint16 *)malloc(160 * sizeof(uint16));

  switch(type) {
  case 0x1c:
  case 0x1d:
  case 0x1e:
    gbDataMBC5.isRumbleCartridge = 1;
  }

  gbPower();

  return true;
}

static void SetPixelFormat(int rshift, int gshift, int bshift)
{
 gbGenFilter(rshift, gshift, bshift);
}

static uint8 *paddie;

static void MDFNGB_SetInput(int port, const char *type, void *ptr)
{
 paddie = (uint8*)ptr;
}

static int32 snooze = 0;
static int32 PadInterruptDelay = 0;

static void Emulate(EmulateSpecStruct *espec)
{
 bool linedrawn[144];

 memset(linedrawn, 0, sizeof(linedrawn));

 //if(gbRom[0x147] == 0x22)
 //{
 //  systemUpdateMotionSensor();
 //}

 if(*paddie != gbJoymask)
 {
  PadInterruptDelay = 20;
 }

 MDFNMP_ApplyPeriodicCheats();
 int clockTicks = 0;
 int doret = 0;
  
 MDFNGameInfo->fb = espec->pixels;

 while(!doret && SoundTS < 72000)
 {
  if(gbDmaTicks)
  {
   clockTicks = 4;
   gbDmaTicks -= 4;
   if(gbDmaTicks < 0) { clockTicks += gbDmaTicks; gbDmaTicks = 0; }
  }
  else
   clockTicks = GBZ80_RunOp();

  SoundTS += clockTicks << (2 - gbSpeed);
    
  gbDivTicks -= clockTicks;
  while(gbDivTicks <= 0) 
  {
    register_DIV++;
    gbDivTicks += GBDIV_CLOCK_TICKS;
  }

  if(PadInterruptDelay > 0)
  {
   PadInterruptDelay -= clockTicks;
   if(PadInterruptDelay <= 0)
   {
    gbJoymask = *paddie;
    register_IF |= 0x10;
   }
  }

  if(snooze > 0)
  {
   snooze -= clockTicks;
   if(snooze <= 0)
   {
              register_IF |= 1; // V-Blank interrupt
              if(register_STAT & 0x10)
                register_IF |= 2;
   }
  }


  if(register_LCDC & 0x80) 
  {
    // LCD stuff
    gbLcdTicks -= clockTicks;
    if(gbLcdMode == GBLCDM_VBLANK) 
    {
      // during V-BLANK,we need to increment LY at the same rate!
      gbLcdLYIncrementTicks -= clockTicks;
      while(gbLcdLYIncrementTicks <= 0) 
      {
       gbLcdLYIncrementTicks += GBLY_INCREMENT_CLOCK_TICKS;

       if(register_LY < 153) 
       {
        register_LY++;
        gbCompareLYToLYC();
            
        if(register_LY >= 153)
         gbLcdLYIncrementTicks = 6;
       } 
       else 
       {
        register_LY = 0x00;
        // reset the window line
        gbWindowLine = -1;
        gbLcdLYIncrementTicks = GBLY_INCREMENT_CLOCK_TICKS * 2;
        gbCompareLYToLYC();
       }
      }
     }

      // our counter is off, see what we need to do
      while(gbLcdTicks <= 0) 
      {
       switch(gbLcdMode) 
       {
        case GBLCDM_HBLANK:
          // H-Blank
          register_LY++;
          gbCompareLYToLYC();
          
          // check if we reached the V-Blank period       
          if(register_LY == 144) 
	  {
	    doret = 1;
            // Yes, V-Blank
            // set the LY increment counter
            gbLcdLYIncrementTicks = gbLcdTicks + GBLY_INCREMENT_CLOCK_TICKS;
            gbLcdTicks += GBLCD_MODE_1_CLOCK_TICKS;
            gbLcdMode = 1;

            if(register_LCDC & 0x80) 
	    {
	     snooze = 6;
             //register_IF |= 1; // V-Blank interrupt
             //if(register_STAT & 0x10)
             //  register_IF |= 2;
            }
           } else {
            // go the the OAM being accessed mode
            gbLcdTicks += GBLCD_MODE_2_CLOCK_TICKS;
            gbLcdMode = 2;

            // only one LCD interrupt per line. may need to generalize...
            if(!(register_STAT & 0x40) ||
               (register_LY != register_LYC)) {
              if((register_STAT & 0x28) == 0x20)
                register_IF |= 2;
            }
          }
          break;

        case GBLCDM_VBLANK:
          // V-Blank
          // next mode is OAM being accessed mode
          gbLcdTicks += GBLCD_MODE_2_CLOCK_TICKS;
          gbLcdMode = GBLCDM_OAM;
          if(!(register_STAT & 0x40) ||
             (register_LY != register_LYC)) {
            if((register_STAT & 0x28) == 0x20)
              register_IF |= 2;
          }
          break;

        case GBLCDM_OAM:
          // OAM being accessed mode
          
          // next mode is OAM and VRAM in use
          gbLcdTicks += GBLCD_MODE_3_CLOCK_TICKS;
          gbLcdMode = GBLCDM_OAM_VRAM;
          break;

        case GBLCDM_OAM_VRAM:
          // OAM and VRAM in use
          // next mode is H-Blank
          if(register_LY < 144) 
	  {
                uint32 *dest = (uint32 *)MDFNGameInfo->fb + register_LY * (MDFNGameInfo->pitch >> 2);

		linedrawn[register_LY] = 1;
                gbRenderLine();
                gbDrawSprites();
             
		if(gbCgbMode)
		{
                 for(int x = 0; x < 160;) 
		 {
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];

                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];

                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                      *dest++ = gbColorFilter[gbLineMix[x++]];
                 }              
		}
		else // to if(gbCgbMode)
		{
                 for(int x = 0; x < 160; x++)
	          dest[x] = gbMonoColorMap[gbLineMix[x]];
		}
          }
          gbLcdMode = GBLCDM_HBLANK;
          // only one LCD interrupt per line. may need to generalize...
          if(!(register_STAT & 0x40) || (register_LY != register_LYC)) 
	  {
            if(register_STAT & 0x08)
              register_IF |= 2;
          }
          if(gbHdmaOn) 
	  {
            gbDoHdma();
          }
	  else
	  {
	   gbLcdTicks += GBLCD_MODE_0_CLOCK_TICKS;
	  }
          break;
        }
        // mark the correct lcd mode on STAT register
        register_STAT = (register_STAT & 0xfc) | gbLcdMode;
      }
    }

    // serial emulation
    if(gbSerialOn) {
#ifdef LINK_EMULATION
      if(linkConnected) {
        gbSerialTicks -= clockTicks;

        while(gbSerialTicks <= 0) {
          // increment number of shifted bits
          gbSerialBits++;
          linkProc();
          if(gbSerialOn && (register_SC & 1)) {
            if(gbSerialBits == 8) {
              gbSerialBits = 0;
	      register_SB = 0xff;
              register_SC &= 0x7f;
              gbSerialOn = 0;
              register_IF |= 8;
              gbSerialTicks = 0;
            }
          }
          gbSerialTicks += GBSERIAL_CLOCK_TICKS;
        }
      } else {
#endif
        if(register_SC & 1) {
          gbSerialTicks -= clockTicks;
          
          // overflow
          while(gbSerialTicks <= 0) {
            // shift serial byte to right and put a 1 bit in its place
            //      register_SB = 0x80 | (register_SB>>1);
            // increment number of shifted bits
            gbSerialBits++;
            if(gbSerialBits == 8) {
              // end of transmission
              if(gbSerialFunction) // external device
                register_SB = gbSerialFunction(register_SB);
              else
                register_SB = 0xff;
              gbSerialTicks = 0;
              register_SC &= 0x7f;
              gbSerialOn = 0;
              register_IF |= 8;
              gbSerialBits  = 0;
            } else
              gbSerialTicks += GBSERIAL_CLOCK_TICKS;
          }
        }
#ifdef LINK_EMULATION
      }
#endif
    }

    // timer emulation
    if(gbTimerOn) {
      gbTimerTicks -= clockTicks;
      while(gbTimerTicks <= 0) {
	ClockTIMA();
        gbTimerTicks += gbTimerClockTicks;
      }
    }
 }

 //printf("%d %d\n", register_LY, SoundTS);
 for(int y = 0; y < 144; y++)
 {
  if(!linedrawn[y])
  {
   uint32 * dest = (uint32 *)MDFNGameInfo->fb + y * (MDFNGameInfo->pitch >> 2);
   uint32 fill_color = gbCgbMode ? gbColorFilter[gbPalette[0]] : gbMonoColorMap[8];

   MDFN_FastU32MemsetM8(dest, fill_color, 160);
  }
 }
 *(espec->SoundBuf) = MDFNGBSOUND_Flush(SoundTS, espec->SoundBufSize);
 SoundTS = 0;
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 // Error out if we try to load old save states that are definitely not compatible.
 if(load && load < 0x802)
 {
  return(0);
 }

 SFORMAT RAMDesc[] =
 {
  SFARRAYN(gbOAM, 0xA0, "OAM"),
  SFARRAYN(HRAM, 0x80, "HRAM"),
  SFARRAYN(gbRam, gbRamSize, "RAM"),
  SFARRAYN(gbVram, gbCgbMode ? 0x4000 : 0x2000, "VRAM"),
  SFARRAYN(gbWram, gbCgbMode ? 0x8000 : 0x2000, "WRAM"),
  SFARRAY16(gbPalette, (gbCgbMode ? 128 : 0)),
  { NULL, 0, NULL }
 };

 std::vector <SSDescriptor> love;

 love.push_back(SSDescriptor(gbSaveGameStruct, "MAIN"));
 love.push_back(SSDescriptor(Joy_StateRegs, "JOY"));
 love.push_back(SSDescriptor(MBC1_StateRegs, "MBC1"));
 love.push_back(SSDescriptor(MBC2_StateRegs, "MBC2"));
 love.push_back(SSDescriptor(MBC3_StateRegs, "MBC3"));
 love.push_back(SSDescriptor(MBC5_StateRegs, "MBC5"));
 love.push_back(SSDescriptor(MBC7_StateRegs, "MBC7"));
 love.push_back(SSDescriptor(HuC1_StateRegs, "HuC1"));
 love.push_back(SSDescriptor(HuC3_StateRegs, "HuC3"));
 love.push_back(SSDescriptor(RAMDesc, "RAM"));

 int ret = MDFNSS_StateAction(sm, load, data_only, love);

 ret &= GBZ80_StateAction(sm, load, data_only);

 if(load)
  StateRest(load);

 if(!MDFNGBSOUND_StateAction(sm, load, data_only))
  return(0);

 return(ret);
}

static bool ToggleLayer(int which) 
{
 gblayerSettings ^= 1 << which;
 return((gblayerSettings >> which) & 1);
}

static void DoSimpleCommand(int cmd)
{
 if(cmd == MDFNNPCMD_POWER || cmd == MDFNNPCMD_RESET)
 {
  gbPower();
 }
}

static MDFNSetting GBSettings[] =
{
 { "gb.forcemono", gettext_noop("Force monophonic sound output."), MDFNST_BOOL, "0" },
 { NULL }
};

static const InputDeviceInputInfoStruct IDII[] =
{
 { "a", "A", 7, IDIT_BUTTON_CAN_RAPID, NULL },
 { "b", "B", 6, IDIT_BUTTON_CAN_RAPID, NULL },
 { "select", "SELECT", 4, IDIT_BUTTON, NULL },
 { "start", "START", 5, IDIT_BUTTON, NULL },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
};

static InputDeviceInfoStruct InputDeviceInfo[] =
{
 {
  "gamepad",
  "Gamepad",
  NULL,
  sizeof(IDII) / sizeof(InputDeviceInputInfoStruct),
  IDII,
 }
};

static const InputPortInfoStruct PortInfo[] =
{
 { 0, "builtin", "Built-In", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo }
};

static InputInfoStruct InputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};


MDFNGI EmulatedGB =
{
 "gb",
 #ifdef WANT_DEBUGGER
 NULL,
 #endif
 &InputInfo,
 Load,
 NULL,
 CloseGame,
 ToggleLayer,
 "Background\0Sprites\0Window\0",
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 SetPixelFormat,
 MDFNGB_SetInput,
 NULL,
 NULL,
 NULL,
 MDFNGB_SetSoundMultiplier,
 MDFNGB_SetSoundVolume,
 MDFNGB_Sound,
 DoSimpleCommand,
 GBSettings,
 (uint32)((double)4194304 / 70224 * 65536 * 256),
 NULL,
 160,
 144,
 160, // Save state preview width
 256 * sizeof(uint32),
 {0, 0, 160, 144},
};
