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
#include "flash.h"
#include "sram.h"
#include "../memory.h"

#include <memory.h>

#define FLASH_READ_ARRAY         0
#define FLASH_CMD_1              1
#define FLASH_CMD_2              2
#define FLASH_AUTOSELECT         3
#define FLASH_CMD_3              4
#define FLASH_CMD_4              5
#define FLASH_CMD_5              6
#define FLASH_ERASE_COMPLETE     7
#define FLASH_PROGRAM            8
#define FLASH_SETBANK            9

uint8 *flashSaveMemory = NULL;
int flashState = FLASH_READ_ARRAY;
int flashReadState = FLASH_READ_ARRAY;
int flashSize = 0x10000;
static int flashDeviceID = 0x1b;
static int flashManufacturerID = 0x32;
static int flashBank = 0;

int GBA_Flash_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT flashSaveData[] =
 {
   SFVAR(flashState),
   SFVAR(flashReadState),
   SFVAR(flashSize),
   SFVAR(flashBank),
   { flashSaveMemory, 0x20000, "flashSaveMemory" },
   SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, flashSaveData, "FLSH");

 return(ret);
};

bool GBA_Flash_Init(void)
{
 if(!(flashSaveMemory = (uint8 *)MDFN_malloc(0x20000, _("flash memory"))))
  return(0);

 memset(flashSaveMemory, 0x00, 0x20000);
 return(1);
}

void GBA_Flash_Kill(void)
{
 if(flashSaveMemory)
 {
  free(flashSaveMemory);
  flashSaveMemory = NULL;
 }
}

void GBA_Flash_Reset(void)
{
  flashState = FLASH_READ_ARRAY;
  flashReadState = FLASH_READ_ARRAY;
  flashBank = 0;
}

void flashSetSize(int size)
{
  //printf("Setting flash size to %d\n", size);
  flashSize = size;
  if(size == 0x10000) {
    flashDeviceID = 0x1b;
    flashManufacturerID = 0x32;
  } else {
    flashDeviceID = 0x13; //0x09;
    flashManufacturerID = 0x62; //0xc2;
  }
}

uint8 flashRead(uint32 address)
{
  //  log("Reading %08x from %08x\n", address, reg[15].I);
  //  log("Current read state is %d\n", flashReadState);
  address &= 0xFFFF;

  //printf("Read: %04x\n", address);
  switch(flashReadState) {
  case FLASH_READ_ARRAY:
    return flashSaveMemory[(flashBank << 16) + address];
  case FLASH_AUTOSELECT:
    switch(address & 0xFF) {
    case 0:
      // manufacturer ID
      return flashManufacturerID;
    case 1:
      // device ID
      return flashDeviceID;
    }
    break;
  case FLASH_ERASE_COMPLETE:
    flashState = FLASH_READ_ARRAY;
    flashReadState = FLASH_READ_ARRAY;
    return 0xFF;
  };
  return 0;
}

void flashWrite(uint32 address, uint8 byte)
{
  
  //printf("Writing %02x at %08x\n", byte, address);
  //  log("Current state is %d\n", flashState);
  address &= 0xFFFF;
  switch(flashState) {
  case FLASH_READ_ARRAY:
    if(address == 0x5555 && byte == 0xAA)
      flashState = FLASH_CMD_1;
    break;
  case FLASH_CMD_1:
    if(address == 0x2AAA && byte == 0x55)
      flashState = FLASH_CMD_2;
    else
      flashState = FLASH_READ_ARRAY;
    break;
  case FLASH_CMD_2:
    if(address == 0x5555) {
      if(byte == 0x90) {
        flashState = FLASH_AUTOSELECT;
        flashReadState = FLASH_AUTOSELECT;
      } else if(byte == 0x80) {
        flashState = FLASH_CMD_3;
      } else if(byte == 0xF0) {
        flashState = FLASH_READ_ARRAY;
        flashReadState = FLASH_READ_ARRAY;
      } else if(byte == 0xA0) {
        flashState = FLASH_PROGRAM;
      } else if(byte == 0xB0 && flashSize == 0x20000) {
        flashState = FLASH_SETBANK;
      } else {
        flashState = FLASH_READ_ARRAY;
        flashReadState = FLASH_READ_ARRAY;
      }
    } else {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    }
    break;
  case FLASH_CMD_3:
    if(address == 0x5555 && byte == 0xAA) {
      flashState = FLASH_CMD_4;
    } else {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    }
    break;
  case FLASH_CMD_4:
    if(address == 0x2AAA && byte == 0x55) {
      flashState = FLASH_CMD_5;
    } else {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    }
    break;
  case FLASH_CMD_5:
    if(byte == 0x30) {
      // SECTOR ERASE
      memset(&flashSaveMemory[(flashBank << 16) + (address & 0xF000)],
             0x00,
             0x1000);
      flashReadState = FLASH_ERASE_COMPLETE;
    } else if(byte == 0x10) {
      // CHIP ERASE
      memset(flashSaveMemory, 0, flashSize);
      flashReadState = FLASH_ERASE_COMPLETE;
    } else {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    }
    break;
  case FLASH_AUTOSELECT:
    if(byte == 0xF0) {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    } else if(address == 0x5555 && byte == 0xAA)
      flashState = FLASH_CMD_1;
    else {
      flashState = FLASH_READ_ARRAY;
      flashReadState = FLASH_READ_ARRAY;
    }
    break;
  case FLASH_PROGRAM:
    flashSaveMemory[(flashBank<<16)+address] = byte;
    flashState = FLASH_READ_ARRAY;
    flashReadState = FLASH_READ_ARRAY;
    break;
  case FLASH_SETBANK:
    if(address == 0) {
      flashBank = (byte & 1);
    }
    flashState = FLASH_READ_ARRAY;
    flashReadState = FLASH_READ_ARRAY;
    break;
  }
}
