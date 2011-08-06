// -*- C++ -*-
// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2005 Forgotten and the VBA development team

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
#include "Port.h"
#include "RTC.h"
#include "GBAinline.h"

uint32 CPUReadMemory(uint32 address)
{  
  uint32 value;

  switch(address >> 24) 
  {
   case 0:
    if(reg[15].I >> 24) 
    {
      if(address < 0x4000) 
      {
        value = READ32LE(((uint32 *)&biosProtected));
      }
      else goto unreadable;
    } 
    else
      value = READ32LE(((uint32 *)&bios[address & 0x3FFC]));
    break;
  case 2:
    value = READ32LE(((uint32 *)&workRAM[address & 0x3FFFC]));
    break;
  case 3:
    value = READ32LE(((uint32 *)&internalRAM[address & 0x7ffC]));
    break;
  case 4:
    if((address < 0x4000400) && ioReadable[address & 0x3fc]) {
      if(ioReadable[(address & 0x3fc) + 2])
        value = READ32LE(((uint32 *)&ioMem[address & 0x3fC]));
      else
        value = READ16LE(((uint16 *)&ioMem[address & 0x3fc]));
    } else goto unreadable;
    break;
  case 5:
    value = READ32LE(((uint32 *)&paletteRAM[address & 0x3fC]));
    break;
  case 6:
    address = (address & 0x1fffc);
    if (((DISPCNT & 7) >2) && ((address & 0x1C000) == 0x18000))
    {
     value = 0;
     break;
    }
    if ((address & 0x18000) == 0x18000)
     address &= 0x17fff;
    value = READ32LE(((uint32 *)&vram[address]));
    break;
  case 7:
    value = READ32LE(((uint32 *)&oam[address & 0x3FC]));
    break;
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
    value = READ32LE(((uint32 *)&rom[address&0x1FFFFFC]));
    break;    
  case 13:
    if(cpuEEPROMEnabled)
      // no need to swap this
      return eepromRead(address);
    goto unreadable;
  case 14:
    if(cpuFlashEnabled | cpuSramEnabled)
      // no need to swap this
      return flashRead(address);
    // default

  default:
  unreadable:
    if(cpuDmaHack) {
      value = cpuDmaLast;
    } else {
      if(armState) {
        value = CPUReadMemoryQuick(reg[15].I);
      } else {
        value = CPUReadHalfWordQuick(reg[15].I) |
          CPUReadHalfWordQuick(reg[15].I) << 16;
      }
    }
  }

  if(address & 3) {
    int shift = (address & 3) << 3;
    value = (value >> shift) | (value << (32 - shift));
  }
  return value;
}

uint32 CPUReadHalfWord(uint32 address)
{
  uint32 value;
  
  switch(address >> 24) {
  case 0:
    if (reg[15].I >> 24) {
      if(address < 0x4000) {
        value = READ16LE(((uint16 *)&biosProtected[address&2]));
      } else goto unreadable;
    } else
      value = READ16LE(((uint16 *)&bios[address & 0x3FFE]));
    break;
  case 2:
    value = READ16LE(((uint16 *)&workRAM[address & 0x3FFFE]));
    break;
  case 3:
    value = READ16LE(((uint16 *)&internalRAM[address & 0x7ffe]));
    break;
  case 4:
    if((address < 0x4000400) && ioReadable[address & 0x3fe])
    {
      value =  READ16LE(((uint16 *)&ioMem[address & 0x3fe]));
      if (((address & 0x3fe)>0xFF) && ((address & 0x3fe)<0x10E))
      {
        if (((address & 0x3fe) == 0x100) && timers[0].On)
          value = 0xFFFF - ((timers[0].Ticks-cpuTotalTicks) >> timers[0].ClockReload);
        else
        if (((address & 0x3fe) == 0x104) && timers[1].On && !(timers[1].CNT & 4))
          value = 0xFFFF - ((timers[1].Ticks-cpuTotalTicks) >> timers[1].ClockReload);
        else
        if (((address & 0x3fe) == 0x108) && timers[2].On && !(timers[2].CNT & 4))
          value = 0xFFFF - ((timers[2].Ticks-cpuTotalTicks) >> timers[2].ClockReload);
        else
        if (((address & 0x3fe) == 0x10C) && timers[3].On && !(timers[3].CNT & 4))
          value = 0xFFFF - ((timers[3].Ticks-cpuTotalTicks) >> timers[3].ClockReload);
      }
    }
    else goto unreadable;
    break;
  case 5:
    value = READ16LE(((uint16 *)&paletteRAM[address & 0x3fe]));
    break;
  case 6:
	     address = (address & 0x1fffe);
  	     if (((DISPCNT & 7) >2) && ((address & 0x1C000) == 0x18000))
  	     {
  	         value = 0;
  	         break;
  	     }
  	     if ((address & 0x18000) == 0x18000)
  	       address &= 0x17fff;
  	     value = READ16LE(((uint16 *)&vram[address]));
    break;
  case 7:
    value = READ16LE(((uint16 *)&oam[address & 0x3fe]));
    break;
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
    if(GBA_RTC && (address == 0x80000c4 || address == 0x80000c6 || address == 0x80000c8))
     value = GBA_RTC->Read(address);
    else
      value = READ16LE(((uint16 *)&rom[address & 0x1FFFFFE]));
    break;    
  case 13:
    if(cpuEEPROMEnabled)
      // no need to swap this
      return  eepromRead(address);
    goto unreadable;
  case 14:
    if(cpuFlashEnabled | cpuSramEnabled)
      // no need to swap this
      return flashRead(address);
    // default
  default:
  unreadable:
    if(cpuDmaHack) {
      value = cpuDmaLast & 0xFFFF;
    } else {
      if(armState) {
        value = CPUReadHalfWordQuick(reg[15].I + (address & 2));
      } else {
        value = CPUReadHalfWordQuick(reg[15].I);
      }
    }
    break;
  }

  if(address & 1) {
    value = (value >> 8) | (value << 24);
  }
  
  return value;
}

uint8 CPUReadByte(uint32 address)
{
  switch(address >> 24) {
  case 0:
    if (reg[15].I >> 24) {
      if(address < 0x4000) {
        return biosProtected[address & 3];
      } else goto unreadable;
    }
    return bios[address & 0x3FFF];
  case 2:
    return workRAM[address & 0x3FFFF];
  case 3:
    return internalRAM[address & 0x7fff];
  case 4:
    if((address < 0x4000400) && ioReadable[address & 0x3ff])
      return ioMem[address & 0x3ff];
    else goto unreadable;
  case 5:
    return paletteRAM[address & 0x3ff];
  case 6:
	     address = (address & 0x1ffff);
  	     if (((DISPCNT & 7) >2) && ((address & 0x1C000) == 0x18000))
  	         return 0;
  	     if ((address & 0x18000) == 0x18000)
  	       address &= 0x17fff;
  	     return vram[address];
  case 7:
    return oam[address & 0x3ff];
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
    return rom[address & 0x1FFFFFF];        
  case 13:
    if(cpuEEPROMEnabled)
      return eepromRead(address);
    goto unreadable;
  case 14:
    if(cpuSramEnabled | cpuFlashEnabled)
      return flashRead(address);
    if(cpuEEPROMSensorEnabled) {
      switch(address & 0x00008f00) {
      case 0x8200:
        //return systemGetSensorX() & 255;
      case 0x8300:
        //return (systemGetSensorX() >> 8)|0x80;
      case 0x8400:
        //return systemGetSensorY() & 255;
      case 0x8500:
        //return systemGetSensorY() >> 8;
	return(0);
      }
    }
    // default
  default:
  unreadable:
    if(cpuDmaHack) {
      return cpuDmaLast & 0xFF;
    } else {
      if(armState) {
        return CPUReadByteQuick(reg[15].I+(address & 3));
      } else {
        return CPUReadByteQuick(reg[15].I+(address & 1));
      }
    }
    break;
  }
}
