#include "GBA.h"
#include "Globals.h"
#include "Flash.h"
#include "Sram.h"

u8 sramRead(u32 address)
{
  return flashSaveMemory[address & 0xFFFF];
}
void sramDelayedWrite(u32 address, u8 byte)
{
  saveType = 1;
  cpuSaveGameFunc = sramWrite;
  sramWrite(address, byte);
}

void sramWrite(u32 address, u8 byte)
{
  flashSaveMemory[address & 0xFFFF] = byte;
  systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
}
