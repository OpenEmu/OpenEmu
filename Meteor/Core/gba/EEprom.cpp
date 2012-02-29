#include <memory.h>
#include "GBA.h"
#include "EEprom.h"
#include "../Util.h"

extern int cpuDmaCount;

int eepromMode = EEPROM_IDLE;
int eepromByte = 0;
int eepromBits = 0;
int eepromAddress = 0;
u8 eepromData[0x2000];
u8 eepromBuffer[16];
bool eepromInUse = false;
int eepromSize = 512;

variable_desc eepromSaveData[] = {
  { &eepromMode, sizeof(int) },
  { &eepromByte, sizeof(int) },
  { &eepromBits , sizeof(int) },
  { &eepromAddress , sizeof(int) },
  { &eepromInUse, sizeof(bool) },
  { &eepromData[0], 512 },
  { &eepromBuffer[0], 16 },
  { NULL, 0 }
};

void eepromInit()
{
  memset(eepromData, 255, sizeof(eepromData));
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

void eepromSaveGame(gzFile gzFile)
{
  utilWriteData(gzFile, eepromSaveData);
  utilWriteInt(gzFile, eepromSize);
  utilGzWrite(gzFile, eepromData, 0x2000);
}

void eepromReadGame(gzFile gzFile, int version)
{
  utilReadData(gzFile, eepromSaveData);
  if(version >= SAVE_GAME_VERSION_3) {
    eepromSize = utilReadInt(gzFile);
    utilGzRead(gzFile, eepromData, 0x2000);
  } else {
    // prior to 0.7.1, only 4K EEPROM was supported
    eepromSize = 512;
  }
}

void eepromReadGameSkip(gzFile gzFile, int version)
{
  // skip the eeprom data in a save game
  utilReadDataSkip(gzFile, eepromSaveData);
  if(version >= SAVE_GAME_VERSION_3) {
    utilGzSeek(gzFile, sizeof(int), SEEK_CUR);
    utilGzSeek(gzFile, 0x2000, SEEK_CUR);
  }
}

int eepromRead(u32 /* address */)
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

void eepromWrite(u32 /* address */, u8 value)
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
      systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
    } else if(eepromBits == 0x41) {
      eepromMode = EEPROM_IDLE;
      eepromByte = 0;
      eepromBits = 0;
    }
    break;
  }
}
