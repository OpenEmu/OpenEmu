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
#include <memory.h>
#include "eeprom.h"

extern int cpuDmaCount;

int eepromMode = EEPROM_IDLE;
int eepromByte = 0;
int eepromBits = 0;
int eepromAddress = 0;
uint8 eepromData[0x2000];
static uint8 eepromBuffer[16];
bool8 eepromInUse = false;
int eepromSize = 512;

#include "../state.h"

SFORMAT eepromSaveData[] = 
{
  SFVAR(eepromMode),
  SFVAR(eepromByte),
  SFVAR(eepromBits),
  SFVAR(eepromAddress),
  SFVAR(eepromInUse),
  SFVAR(eepromSize),
  { eepromData, 0x2000, "eepromData" },
  { eepromBuffer, 16, "eepromBuffer" },
  SFEND
};

bool GBA_EEPROM_SaveFile(const char *filename)
{
 if(eepromInUse)
 {
  if(!MDFN_DumpToFile(filename, 0, eepromData, eepromSize))
   return(0);
 }

 return(1);
}

bool GBA_EEPROM_LoadFile(const char *filename)
{
 FILE *fp = fopen(filename, "rb");

 if(fp)
 {
  long size;

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if(size == 512 || size == 0x2000)
  {
   if(fread(eepromData, 1, size, fp) == size)
   {
    eepromInUse = TRUE;
    eepromSize = size;
    fclose(fp);
    return(1);
   }
  }
  fclose(fp);
 }

 return(0);
}


void eepromInit()
{
  memset(eepromData, 0xFF, sizeof(eepromData));
}

void eepromReset()
{
  eepromMode = EEPROM_IDLE;
  eepromByte = 0;
  eepromBits = 0;
  eepromAddress = 0;
  eepromInUse = false;
  eepromSize = 512;
}

int eepromRead(uint32 /* address */)
{
  switch(eepromMode) {
  case EEPROM_IDLE:
  case EEPROM_READADDRESS:
  case EEPROM_WRITEDATA:
    return 1;
  case EEPROM_READDATA:
    {
      eepromBits++;
      if(eepromBits == 4) {
        eepromMode = EEPROM_READDATA2;
        eepromBits = 0;
        eepromByte = 0;
      }
      return 0;
    }
  case EEPROM_READDATA2:
    {
      int data = 0;
      int address = eepromAddress << 3;
      int mask = 1 << (7 - (eepromBits & 7));
      data = (eepromData[address+eepromByte] & mask) ? 1 : 0;
      eepromBits++;
      if((eepromBits & 7) == 0)
        eepromByte++;
      if(eepromBits == 0x40)
        eepromMode = EEPROM_IDLE;
      return data;
    }
  default:
      return 0;
  }
  return 1;
}

void eepromWrite(uint32 /* address */, uint8 value)
{
  if(cpuDmaCount == 0)
    return;
  int bit = value & 1;
  switch(eepromMode) {
  case EEPROM_IDLE:
    eepromByte = 0;
    eepromBits = 1;
    eepromBuffer[eepromByte] = bit;
    eepromMode = EEPROM_READADDRESS;
    break;
  case EEPROM_READADDRESS:
    eepromBuffer[eepromByte] <<= 1;
    eepromBuffer[eepromByte] |= bit;
    eepromBits++;
    if((eepromBits & 7) == 0) {
      eepromByte++;
    }
    if(cpuDmaCount == 0x11 || cpuDmaCount == 0x51) {
      if(eepromBits == 0x11) {
        eepromInUse = true;
        eepromSize = 0x2000;
        eepromAddress = ((eepromBuffer[0] & 0x3F) << 8) |
          ((eepromBuffer[1] & 0xFF));
        if(!(eepromBuffer[0] & 0x40)) {
          eepromBuffer[0] = bit;          
          eepromBits = 1;
          eepromByte = 0;
          eepromMode = EEPROM_WRITEDATA;
        } else {
          eepromMode = EEPROM_READDATA;
          eepromByte = 0;
          eepromBits = 0;
        }
      }
    } else {
      if(eepromBits == 9) {
        eepromInUse = true;
        eepromAddress = (eepromBuffer[0] & 0x3F);
        if(!(eepromBuffer[0] & 0x40)) {
          eepromBuffer[0] = bit;
          eepromBits = 1;
          eepromByte = 0;         
          eepromMode = EEPROM_WRITEDATA;
        } else {
          eepromMode = EEPROM_READDATA;
          eepromByte = 0;
          eepromBits = 0;
        }
      }
    }
    break;
  case EEPROM_READDATA:
  case EEPROM_READDATA2:
    // should we reset here?
    eepromMode = EEPROM_IDLE;
    break;
  case EEPROM_WRITEDATA:
    eepromBuffer[eepromByte] <<= 1;
    eepromBuffer[eepromByte] |= bit;
    eepromBits++;
    if((eepromBits & 7) == 0) {
      eepromByte++;
    }
    if(eepromBits == 0x40) {
      eepromInUse = true;
      // write data;
      for(int i = 0; i < 8; i++) {
        eepromData[(eepromAddress << 3) + i] = eepromBuffer[i];
      }
    } else if(eepromBits == 0x41) {
      eepromMode = EEPROM_IDLE;
      eepromByte = 0;
      eepromBits = 0;
    }
    break;
  }
}
  
