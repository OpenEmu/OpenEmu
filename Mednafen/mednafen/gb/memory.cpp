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
#include "../mempatcher.h"
#include "gbGlobals.h"
#include "memory.h"

mapperMBC1 gbDataMBC1 = {
  0, // RAM enable
  1, // ROM bank
  0, // RAM bank or high address
  0, // memory model
};

void memoryUpdateMapMBC1()
{
  int tmpAddress = gbDataMBC1.mapperROMBank << 14;

  // check current model
  if(gbDataMBC1.mapperMemoryModel == 0) {
    // model is 16/8, so we have a high address in use
    tmpAddress |= gbDataMBC1.mapperRAMBank << 19;
  }

  gbMemoryMap[0x0a] = &gbRam[0];
  gbMemoryMap[0x0b] = &gbRam[0x1000];

  tmpAddress &= gbRomSizeMask;
  gbMemoryMap[0x04] = &gbRom[tmpAddress];
  gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
  gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
  gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

  if((gbRamSize) && (gbDataMBC1.mapperMemoryModel == 1))
  {
   tmpAddress = gbDataMBC1.mapperRAMBank << 13;
   tmpAddress &= gbRamSizeMask;

   MDFNMP_AddRAM(8192, 0xA000, &gbRam[tmpAddress]);
   gbMemoryMap[0x0a] = &gbRam[tmpAddress];
   gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
  }
  else
   MDFNMP_AddRAM(8192, 0xA000, gbRam);

}

// MBC1 ROM write registers
void mapperMBC1ROM(uint16 address, uint8 value)
{
  switch(address & 0x6000) 
  {
   case 0x0000: // RAM enable register
    gbDataMBC1.mapperRAMEnable = ( ( value & 0x0a) == 0x0a ? 1 : 0);
    break;

   case 0x2000: // ROM bank select
    //    value = value & 0x1f;
    if((value & 0x1f) == 0)
      value++;

    gbDataMBC1.mapperROMBank = value;
    memoryUpdateMapMBC1();
    break;

  case 0x4000: // RAM bank select
    gbDataMBC1.mapperRAMBank = value & 0x03;
    memoryUpdateMapMBC1();
    break;

  case 0x6000: // memory model select
    gbDataMBC1.mapperMemoryModel = value & 1;
    memoryUpdateMapMBC1();
    break;
  }
}

// MBC1 RAM write
void mapperMBC1RAM(uint16 address, uint8 value)
{
  if(gbDataMBC1.mapperRAMEnable) {
    if(gbRamSize) {
      gbMemoryMap[address >> 12][address & 0x0fff & gbRamSizeMask] = value;
    }
  }
}

mapperMBC2 gbDataMBC2 = {
  0, // RAM enable
  1  // ROM bank
};

// MBC2 ROM write registers
void mapperMBC2ROM(uint16 address, uint8 value)
{
  switch(address & 0x6000) {
  case 0x0000: // RAM enable
    if(!(address & 0x0100)) {
      gbDataMBC2.mapperRAMEnable = (value & 0x0f) == 0x0a;
    }
    break;
  case 0x2000: // ROM bank select
    if(address & 0x0100) {
      value &= 0x0f;

      if(value == 0)
        value = 1;
      if(gbDataMBC2.mapperROMBank != value) {
        gbDataMBC2.mapperROMBank = value;

        int tmpAddress = value << 14;

        tmpAddress &= gbRomSizeMask;

        gbMemoryMap[0x04] = &gbRom[tmpAddress];
        gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
        gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
        gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
      }
    }
    break;
  }
}

// MBC2 RAM write
void mapperMBC2RAM(uint16 address, uint8 value)
{
  if(gbDataMBC2.mapperRAMEnable) {
    if(gbRamSize && address < 0xa200) {
      gbMemoryMap[address >> 12][address & 0x0fff & gbRamSizeMask] = value;
    }
  }
}

void memoryUpdateMapMBC2()
{
  int tmpAddress = gbDataMBC2.mapperROMBank << 14;

  tmpAddress &= gbRomSizeMask;

  gbMemoryMap[0x04] = &gbRom[tmpAddress];
  gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
  gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
  gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
}

mapperMBC3 gbDataMBC3 = {
  0, // RAM enable
  1, // ROM bank
  0, // RAM bank
  0, // RAM address
  0, // timer clock latch
  0, // timer clock register
  0, // timer seconds
  0, // timer minutes
  0, // timer hours
  0, // timer days
  0, // timer control
  0, // timer latched seconds
  0, // timer latched minutes
  0, // timer latched hours
  0, // timer latched days
  0, // timer latched control
  (time_t)-1 // last time
};

void memoryUpdateMBC3Clock()
{
  time_t now = time(NULL);
  time_t diff = now - gbDataMBC3.mapperLastTime;
  if(diff > 0) {
    // update the clock according to the last update time
    gbDataMBC3.mapperSeconds += diff % 60;
    if(gbDataMBC3.mapperSeconds > 59) {
      gbDataMBC3.mapperSeconds -= 60;
      gbDataMBC3.mapperMinutes++;
    }

    diff /= 60;

    gbDataMBC3.mapperMinutes += diff % 60;
    if(gbDataMBC3.mapperMinutes > 60) {
      gbDataMBC3.mapperMinutes -= 60;
      gbDataMBC3.mapperHours++;
    }

    diff /= 60;

    gbDataMBC3.mapperHours += diff % 24;
    if(gbDataMBC3.mapperHours > 24) {
      gbDataMBC3.mapperHours -= 24;
      gbDataMBC3.mapperDays++;
    }
    diff /= 24;

    gbDataMBC3.mapperDays += diff;
    if(gbDataMBC3.mapperDays > 255) {
      if(gbDataMBC3.mapperDays > 511) {
        gbDataMBC3.mapperDays %= 512;
        gbDataMBC3.mapperControl |= 0x80;
      }
      gbDataMBC3.mapperControl = (gbDataMBC3.mapperControl & 0xfe) |
        (gbDataMBC3.mapperDays>255 ? 1 : 0);
    }
  }
  gbDataMBC3.mapperLastTime = now;
}

// MBC3 ROM write registers
void mapperMBC3ROM(uint16 address, uint8 value)
{
  int tmpAddress = 0;

  switch(address & 0x6000) {
  case 0x0000: // RAM enable register
    gbDataMBC3.mapperRAMEnable = ( ( value & 0x0a) == 0x0a ? 1 : 0);
    break;
  case 0x2000: // ROM bank select
    value = value & 0x7f;
    if(value == 0)
      value = 1;
    if(value == gbDataMBC3.mapperROMBank)
      break;

    tmpAddress = value << 14;

    tmpAddress &= gbRomSizeMask;
    gbDataMBC3.mapperROMBank = value;
    gbMemoryMap[0x04] = &gbRom[tmpAddress];
    gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
    gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
    gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

    break;
  case 0x4000: // RAM bank select
    if(value < 8) {
      if(value == gbDataMBC3.mapperRAMBank)
        break;
      tmpAddress = value << 13;
      tmpAddress &= gbRamSizeMask;
      MDFNMP_AddRAM(8192, 0xA000, &gbRam[tmpAddress]);
      gbMemoryMap[0x0a] = &gbRam[tmpAddress];
      gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
      gbDataMBC3.mapperRAMBank = value;
      gbDataMBC3.mapperRAMAddress = tmpAddress;
    } else {
      if(gbDataMBC3.mapperRAMEnable) {
        gbDataMBC3.mapperRAMBank = -1;

        gbDataMBC3.mapperClockRegister = value;
      }
    }
    break;
  case 0x6000: // clock latch
    if(gbDataMBC3.mapperClockLatch == 0 && value == 1) {
      memoryUpdateMBC3Clock();
      gbDataMBC3.mapperLSeconds = gbDataMBC3.mapperSeconds;
      gbDataMBC3.mapperLMinutes = gbDataMBC3.mapperMinutes;
      gbDataMBC3.mapperLHours   = gbDataMBC3.mapperHours;
      gbDataMBC3.mapperLDays    = gbDataMBC3.mapperDays;
      gbDataMBC3.mapperLControl = gbDataMBC3.mapperControl;
    }
    if(value == 0x00 || value == 0x01)
      gbDataMBC3.mapperClockLatch = value;
    break;
  }
}

// MBC3 RAM write
void mapperMBC3RAM(uint16 address, uint8 value)
{
  if(gbDataMBC3.mapperRAMEnable) {
    if(gbDataMBC3.mapperRAMBank != -1) {
      if(gbRamSize) {
        gbMemoryMap[address>>12][address & 0x0fff & gbRamSizeMask] = value;
      }
    } else {
      time_t tmp;
      time(&tmp);
  
      gbDataMBC3.mapperLastTime = tmp;
      switch(gbDataMBC3.mapperClockRegister) {
      case 0x08:
        gbDataMBC3.mapperSeconds = value;
        break;
      case 0x09:
        gbDataMBC3.mapperMinutes = value;
        break;
      case 0x0a:
        gbDataMBC3.mapperHours = value;
        break;
      case 0x0b:
        gbDataMBC3.mapperDays = value;
        break;
      case 0x0c:
        if(gbDataMBC3.mapperControl & 0x80)
          gbDataMBC3.mapperControl = 0x80 | value;
        else
          gbDataMBC3.mapperControl = value;
        break;
      }
    }
  }
}

// MBC3 read RAM
uint8 mapperMBC3ReadRAM(uint16 address)
{
  if(gbDataMBC3.mapperRAMEnable) {
    if(gbDataMBC3.mapperRAMBank != -1) {
      return gbMemoryMap[address>>12][address & 0x0fff];
    }

    switch(gbDataMBC3.mapperClockRegister) {
      case 0x08:
        return gbDataMBC3.mapperLSeconds;
        break;
      case 0x09:
        return gbDataMBC3.mapperLMinutes;
        break;
      case 0x0a:
        return gbDataMBC3.mapperLHours;
        break;
      case 0x0b:
        return gbDataMBC3.mapperLDays;
        break;
      case 0x0c:
        return gbDataMBC3.mapperLControl;
    }
  }
  return 0;
}

void memoryUpdateMapMBC3()
{
  int tmpAddress = gbDataMBC3.mapperROMBank << 14;

  tmpAddress &= gbRomSizeMask;

  gbMemoryMap[0x04] = &gbRom[tmpAddress];
  gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
  gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
  gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

  if(gbDataMBC3.mapperRAMBank >= 0 && gbRamSize) {
    tmpAddress = gbDataMBC3.mapperRAMBank << 13;
    tmpAddress &= gbRamSizeMask;
    MDFNMP_AddRAM(8192, 0xA000, &gbRam[tmpAddress]);
    gbMemoryMap[0x0a] = &gbRam[tmpAddress];
    gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
  }
}

mapperMBC5 gbDataMBC5 = {
  0, // RAM enable
  1, // ROM bank
  0, // RAM bank
  0, // ROM high address
  0, // RAM address
  0  // is rumble cartridge?
};

// MBC5 ROM write registers
void mapperMBC5ROM(uint16 address, uint8 value)
{
  int tmpAddress = 0;

  switch(address & 0x6000) {
  case 0x0000: // RAM enable register
    gbDataMBC5.mapperRAMEnable = ( ( value & 0x0a) == 0x0a ? 1 : 0);
    break;
  case 0x2000: // ROM bank select
    if(address < 0x3000) {
      value = value & 0xff;
      if(value == gbDataMBC5.mapperROMBank)
        break;

      tmpAddress = (value << 14) | (gbDataMBC5.mapperROMHighAddress << 22) ;

      tmpAddress &= gbRomSizeMask;
      gbDataMBC5.mapperROMBank = value;
      gbMemoryMap[0x04] = &gbRom[tmpAddress];
      gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
      gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
      gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

    } else {
      value = value & 1;
      if(value == gbDataMBC5.mapperROMHighAddress)
        break;

      tmpAddress = (gbDataMBC5.mapperROMBank << 14) | (value << 22);

      tmpAddress &= gbRomSizeMask;
      gbDataMBC5.mapperROMHighAddress = value;
      gbMemoryMap[0x04] = &gbRom[tmpAddress];
      gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
      gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
      gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
    }
    break;
  case 0x4000: // RAM bank select
    if(gbDataMBC5.isRumbleCartridge)
      value &= 0x07;
    else
      value &= 0x0f;
    if(value == gbDataMBC5.mapperRAMBank)
      break;
    tmpAddress = value << 13;
    tmpAddress &= gbRamSizeMask;
    if(gbRamSize) {
      MDFNMP_AddRAM(8192, 0xA000, &gbRam[tmpAddress]);
      gbMemoryMap[0x0a] = &gbRam[tmpAddress];
      gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];

      gbDataMBC5.mapperRAMBank = value;
      gbDataMBC5.mapperRAMAddress = tmpAddress;
    }
    break;
  }
}

// MBC5 RAM write
void mapperMBC5RAM(uint16 address, uint8 value)
{
  if(gbDataMBC5.mapperRAMEnable) {
    if(gbRamSize) {
      gbMemoryMap[address >> 12][address & 0x0fff & gbRamSizeMask] = value;
    }
  }
}

void memoryUpdateMapMBC5()
{
  int tmpAddress = (gbDataMBC5.mapperROMBank << 14) |
    (gbDataMBC5.mapperROMHighAddress << 22) ;

  tmpAddress &= gbRomSizeMask;
  gbMemoryMap[0x04] = &gbRom[tmpAddress];
  gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
  gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
  gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

  if(gbRamSize) {
    tmpAddress = gbDataMBC5.mapperRAMBank << 13;
    tmpAddress &= gbRamSizeMask;
    MDFNMP_AddRAM(8192, 0xA000, &gbRam[tmpAddress]);
    gbMemoryMap[0x0a] = &gbRam[tmpAddress];
    gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
  }
}

mapperMBC7 gbDataMBC7 = {
  0, // RAM enable
  1, // ROM bank
  0, // RAM bank
  0, // RAM address
  0, // chip select
  0, // ??
  0, // mapper state
  0, // buffer for receiving serial data
  0, // idle state
  0, // count of bits received
  0, // command received
  0, // address received
  0, // write enable
  0, // value to return on ram
};

// MBC7 ROM write registers
void mapperMBC7ROM(uint16 address, uint8 value)
{
  int tmpAddress = 0;

  switch(address & 0x6000) {
  case 0x0000:
    break;
  case 0x2000: // ROM bank select
    value = value & 0x7f;
    if(value == 0)
      value = 1;

    if(value == gbDataMBC7.mapperROMBank)
      break;

    tmpAddress = (value << 14);

    tmpAddress &= gbRomSizeMask;
    gbDataMBC7.mapperROMBank = value;
    gbMemoryMap[0x04] = &gbRom[tmpAddress];
    gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
    gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
    gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
    break;
  case 0x4000: // RAM bank select/enable
    if(value < 8) {
      tmpAddress = (value&3) << 13;
      tmpAddress &= gbRamSizeMask;
      gbMemoryMap[0x0a] = &gbRam[0x0000];
      gbMemoryMap[0x0b] = &gbRam[0x1000];

      gbDataMBC7.mapperRAMBank = value;
      gbDataMBC7.mapperRAMAddress = tmpAddress;
      gbDataMBC7.mapperRAMEnable = 0;
    } else {
      gbDataMBC7.mapperRAMEnable = 0;
    }
    break;
  }
}

// MBC7 read RAM
uint8 mapperMBC7ReadRAM(uint16 address)
{
  switch(address & 0xa0f0) {
  case 0xa000:
  case 0xa010:
  case 0xa060:
  case 0xa070:
    return 0;
  case 0xa020:
    // sensor X low byte
    //return systemGetSensorX() & 255;
  case 0xa030:
    // sensor X high byte
    //return systemGetSensorX() >> 8;
  case 0xa040:
    // sensor Y low byte
    //return systemGetSensorY() & 255;
  case 0xa050:
    // sensor Y high byte
    //return systemGetSensorY() >> 8;
  case 0xa080:
    return gbDataMBC7.value;
  }
  return 0xff;
}

// MBC7 RAM write
void mapperMBC7RAM(uint16 address, uint8 value)
{
  if(address == 0xa080) {
    // special processing needed
    int oldCs = gbDataMBC7.cs,oldSk=gbDataMBC7.sk;
    
    gbDataMBC7.cs=value>>7;
    gbDataMBC7.sk=(value>>6)&1;
    
    if(!oldCs && gbDataMBC7.cs) {
      if(gbDataMBC7.state==5) {
        if(gbDataMBC7.writeEnable) {
          gbRam[gbDataMBC7.address*2]=gbDataMBC7.buffer>>8;
          gbRam[gbDataMBC7.address*2+1]=gbDataMBC7.buffer&0xff;
        }
        gbDataMBC7.state=0;
        gbDataMBC7.value=1;
      } else {
        gbDataMBC7.idle=true;
        gbDataMBC7.state=0;
      }
    }
    
    if(!oldSk && gbDataMBC7.sk) {
      if(gbDataMBC7.idle) {
        if(value & 0x02) {
          gbDataMBC7.idle=false;
          gbDataMBC7.count=0;
          gbDataMBC7.state=1;
        }
      } else {
        switch(gbDataMBC7.state) {
        case 1:
          // receiving command
          gbDataMBC7.buffer <<= 1;
          gbDataMBC7.buffer |= (value & 0x02)?1:0;
          gbDataMBC7.count++;
          if(gbDataMBC7.count==2) {
            // finished receiving command
            gbDataMBC7.state=2;
            gbDataMBC7.count=0;
            gbDataMBC7.code=gbDataMBC7.buffer & 3;
          }
          break;
        case 2:
          // receive address
          gbDataMBC7.buffer <<= 1;
          gbDataMBC7.buffer |= (value&0x02)?1:0;
          gbDataMBC7.count++;
          if(gbDataMBC7.count==8) {
          // finish receiving
            gbDataMBC7.state=3;
            gbDataMBC7.count=0;
            gbDataMBC7.address=gbDataMBC7.buffer&0xff;
            if(gbDataMBC7.code==0) {
              if((gbDataMBC7.address>>6)==0) {
                gbDataMBC7.writeEnable=0;
                gbDataMBC7.state=0;
              } else if((gbDataMBC7.address>>6) == 3) {
                gbDataMBC7.writeEnable=1;
                gbDataMBC7.state=0;
              }
            }
          }
          break;
        case 3:
          gbDataMBC7.buffer <<= 1;
          gbDataMBC7.buffer |= (value&0x02)?1:0;
          gbDataMBC7.count++;
          
          switch(gbDataMBC7.code) {
          case 0:
            if(gbDataMBC7.count==16) {
              if((gbDataMBC7.address>>6)==0) {
                gbDataMBC7.writeEnable = 0;
                gbDataMBC7.state=0;
              } else if((gbDataMBC7.address>>6)==1) {
                if (gbDataMBC7.writeEnable) {
                  for(int i=0;i<256;i++) {
                    gbRam[i*2] = gbDataMBC7.buffer >> 8;
                    gbRam[i*2+1] = gbDataMBC7.buffer & 0xff;
                  }
                }
                gbDataMBC7.state=5;
              } else if((gbDataMBC7.address>>6) == 2) {
                if (gbDataMBC7.writeEnable) {
                  for(int i=0;i<256;i++)
                    *((uint16 *)&gbRam[i*2]) = 0xffff;
                }
                gbDataMBC7.state=5;
              } else if((gbDataMBC7.address>>6)==3) {
                gbDataMBC7.writeEnable = 1;
                gbDataMBC7.state=0;
              }
              gbDataMBC7.count=0;
            }
            break;
          case 1:
            if(gbDataMBC7.count==16) {
              gbDataMBC7.count=0;
              gbDataMBC7.state=5;
              gbDataMBC7.value=0;
            }
            break;
          case 2:
            if(gbDataMBC7.count==1) {
              gbDataMBC7.state=4;
              gbDataMBC7.count=0;
              gbDataMBC7.buffer = (gbRam[gbDataMBC7.address*2]<<8)|
                (gbRam[gbDataMBC7.address*2+1]);
            }
            break;
          case 3:
            if(gbDataMBC7.count==16) {
              gbDataMBC7.count=0;
              gbDataMBC7.state=5;
              gbDataMBC7.value=0;
              gbDataMBC7.buffer=0xffff;
            }
            break;
          }
          break;
        }
      }
    }
    
    if (oldSk && !gbDataMBC7.sk) {
      if (gbDataMBC7.state==4) { 
        gbDataMBC7.value = (gbDataMBC7.buffer & 0x8000)?1:0;
        gbDataMBC7.buffer <<= 1;
        gbDataMBC7.count++;
        if (gbDataMBC7.count==16) {
          gbDataMBC7.count=0;
          gbDataMBC7.state=0;
        }
      }
    }
  }
}

void memoryUpdateMapMBC7()
{
  int tmpAddress = (gbDataMBC5.mapperROMBank << 14);

  tmpAddress &= gbRomSizeMask;
  gbMemoryMap[0x04] = &gbRom[tmpAddress];
  gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
  gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
  gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
}

mapperHuC1 gbDataHuC1 = {
  0, // RAM enable
  1, // ROM bank
  0, // RAM bank
  0, // memory model
  0, // ROM high address
  0  // RAM address
};

// HuC1 ROM write registers
void mapperHuC1ROM(uint16 address, uint8 value)
{
  int tmpAddress = 0;

  switch(address & 0x6000) {
  case 0x0000: // RAM enable register
    gbDataHuC1.mapperRAMEnable = ( ( value & 0x0a) == 0x0a ? 1 : 0);
    break;
  case 0x2000: // ROM bank select
    value = value & 0x3f;
    if(value == 0)
      value = 1;
    if(value == gbDataHuC1.mapperROMBank)
      break;

    tmpAddress = value << 14;

    tmpAddress &= gbRomSizeMask;
    gbDataHuC1.mapperROMBank = value;
    gbMemoryMap[0x04] = &gbRom[tmpAddress];
    gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
    gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
    gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
    break;
  case 0x4000: // RAM bank select
    if(gbDataHuC1.mapperMemoryModel == 1) {
      // 4/32 model, RAM bank switching provided
      value = value & 0x03;
      if(value == gbDataHuC1.mapperRAMBank)
        break;
      tmpAddress = value << 13;
      tmpAddress &= gbRamSizeMask;
      MDFNMP_AddRAM(8192, 0xA000, &gbRam[tmpAddress]);
      gbMemoryMap[0x0a] = &gbRam[tmpAddress];
      gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
      gbDataHuC1.mapperRAMBank = value;
      gbDataHuC1.mapperRAMAddress = tmpAddress;
    } else {
      // 16/8, set the high address
      gbDataHuC1.mapperROMHighAddress = value & 0x03;
      tmpAddress = gbDataHuC1.mapperROMBank << 14;
      tmpAddress |= (gbDataHuC1.mapperROMHighAddress) << 19;
      tmpAddress &= gbRomSizeMask;
      gbMemoryMap[0x04] = &gbRom[tmpAddress];
      gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
      gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
      gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
    }
    break;
  case 0x6000: // memory model select
    gbDataHuC1.mapperMemoryModel = value & 1;
    break;
  }
}

// HuC1 RAM write
void mapperHuC1RAM(uint16 address, uint8 value)
{
  if(gbDataHuC1.mapperRAMEnable) {
    if(gbRamSize) {
      gbMemoryMap[address >> 12][address & 0x0fff] = value;
    }
  }
}

void memoryUpdateMapHuC1()
{
  int tmpAddress = gbDataHuC1.mapperROMBank << 14;

  tmpAddress &= gbRomSizeMask;

  gbMemoryMap[0x04] = &gbRom[tmpAddress];
  gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
  gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
  gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

  if(gbRamSize) {
    tmpAddress = gbDataHuC1.mapperRAMBank << 13;
    tmpAddress &= gbRamSizeMask;
    MDFNMP_AddRAM(8192, 0xA000, &gbRam[tmpAddress]);
    gbMemoryMap[0x0a] = &gbRam[tmpAddress];
    gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
  }
}

mapperHuC3 gbDataHuC3 = {
  0, // RAM enable
  1, // ROM bank
  0, // RAM bank
  0, // RAM address
  0, // RAM flag
  0  // RAM read value
};


// HuC3 ROM write registers
void mapperHuC3ROM(uint16 address, uint8 value)
{
  int tmpAddress = 0;

  switch(address & 0x6000) {
  case 0x0000: // RAM enable register
    gbDataHuC3.mapperRAMEnable = ( value == 0x0a ? 1 : 0);
    gbDataHuC3.mapperRAMFlag = value;
    if(gbDataHuC3.mapperRAMFlag != 0x0a)
      gbDataHuC3.mapperRAMBank = -1;
    break;
  case 0x2000: // ROM bank select
    value = value & 0x7f;
    if(value == 0)
      value = 1;
    if(value == gbDataHuC3.mapperROMBank)
      break;

    tmpAddress = value << 14;

    tmpAddress &= gbRomSizeMask;
    gbDataHuC3.mapperROMBank = value;
    gbMemoryMap[0x04] = &gbRom[tmpAddress];
    gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
    gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
    gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
    break;
  case 0x4000: // RAM bank select
    value = value & 0x03;
    if(value == gbDataHuC3.mapperRAMBank)
      break;
    tmpAddress = value << 13;
    tmpAddress &= gbRamSizeMask;
    MDFNMP_AddRAM(8192, 0xA000, &gbRam[tmpAddress]);
    gbMemoryMap[0x0a] = &gbRam[tmpAddress];
    gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
    gbDataHuC3.mapperRAMBank = value;
    gbDataHuC3.mapperRAMAddress = tmpAddress;
    break;
  case 0x6000: // nothing to do!
    break;
  }
}

// HuC3 read RAM
uint8 mapperHuC3ReadRAM(uint16 address)
{
  if(gbDataHuC3.mapperRAMFlag > 0x0b &&
     gbDataHuC3.mapperRAMFlag < 0x0e) {
    if(gbDataHuC3.mapperRAMFlag != 0x0c)
      return 1;
    return gbDataHuC3.mapperRAMValue;
  } else
    return gbMemoryMap[address >> 12][address & 0x0fff];
}

// HuC3 RAM write
void mapperHuC3RAM(uint16 address, uint8 value)
{
  int *p;

  if(gbDataHuC3.mapperRAMFlag < 0x0b ||
     gbDataHuC3.mapperRAMFlag > 0x0e) {
    if(gbDataHuC3.mapperRAMEnable) {
      if(gbRamSize) {
        gbMemoryMap[address >> 12][address & 0x0fff] = value;
      }
    }
  } else {
    if(gbDataHuC3.mapperRAMFlag == 0x0b) {
      if(value == 0x62) {
        gbDataHuC3.mapperRAMValue = 1;
      } else {
        switch(value & 0xf0) {
        case 0x10:
          p = &gbDataHuC3.mapperRegister2;
          gbDataHuC3.mapperRAMValue = *(p+gbDataHuC3.mapperRegister1++);
          if(gbDataHuC3.mapperRegister1 > 6)
            gbDataHuC3.mapperRegister1 = 0;
          break;
        case 0x30:
          p = &gbDataHuC3.mapperRegister2;
          *(p+gbDataHuC3.mapperRegister1++) = value & 0x0f;
          if(gbDataHuC3.mapperRegister1 > 6)
            gbDataHuC3.mapperRegister1 = 0;
          gbDataHuC3.mapperAddress =
            (gbDataHuC3.mapperRegister6 << 24) |
            (gbDataHuC3.mapperRegister5 << 16) |
            (gbDataHuC3.mapperRegister4 <<  8) |
            (gbDataHuC3.mapperRegister3 <<  4) |
            (gbDataHuC3.mapperRegister2);
          break;
        case 0x40:
          gbDataHuC3.mapperRegister1 = (gbDataHuC3.mapperRegister1 & 0xf0) |
            (value & 0x0f);
          gbDataHuC3.mapperRegister2 = (gbDataHuC3.mapperAddress & 0x0f);
          gbDataHuC3.mapperRegister3 = ((gbDataHuC3.mapperAddress>>4)&0x0f);
          gbDataHuC3.mapperRegister4 = ((gbDataHuC3.mapperAddress>>8)&0x0f);
          gbDataHuC3.mapperRegister5 = ((gbDataHuC3.mapperAddress>>16)&0x0f);
          gbDataHuC3.mapperRegister6 = ((gbDataHuC3.mapperAddress>>24)&0x0f);
          gbDataHuC3.mapperRegister7 = 0;
          gbDataHuC3.mapperRegister8 = 0;
          gbDataHuC3.mapperRAMValue = 0;
          break;
        case 0x50:
          gbDataHuC3.mapperRegister1 = (gbDataHuC3.mapperRegister1 & 0x0f) |
            ((value << 4)&0x0f);
          break;
        default:
          gbDataHuC3.mapperRAMValue = 1;
          break;
        }
      }
    }
  }
}

void memoryUpdateMapHuC3()
{
  int tmpAddress = gbDataHuC3.mapperROMBank << 14;

  tmpAddress &= gbRomSizeMask;
  gbMemoryMap[0x04] = &gbRom[tmpAddress];
  gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
  gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
  gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

  if(gbRamSize) {
    tmpAddress = gbDataHuC3.mapperRAMBank << 13;
    tmpAddress &= gbRamSizeMask;
    MDFNMP_AddRAM(8192, 0xA000, &gbRam[tmpAddress]);
    gbMemoryMap[0x0a] = &gbRam[tmpAddress];
    gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
  }
}
