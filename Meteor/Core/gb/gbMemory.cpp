#include "../System.h"
#include "../common/Port.h"
#include "gbGlobals.h"
#include "gbMemory.h"
#include "gb.h"
u8 gbDaysinMonth [12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const u8 gbDisabledRam [8] = {0x80, 0xff, 0xf0, 0x00, 0x30, 0xbf, 0xbf, 0xbf};
extern int gbGBCColorType;
extern gbRegister PC;

mapperMBC1 gbDataMBC1 = {
  0, // RAM enable
  1, // ROM bank
  0, // RAM bank
  0, // memory model
  0, // ROM high address
  0, // RAM address
  0  // Rom Bank 0 remapping
};

// MBC1 ROM write registers
void mapperMBC1ROM(u16 address, u8 value)
{
  int tmpAddress = 0;

  switch(address & 0x6000) {
  case 0x0000: // RAM enable register
    gbDataMBC1.mapperRAMEnable = ( ( value & 0x0a) == 0x0a ? 1 : 0);
    break;
  case 0x2000: // ROM bank select
    //    value = value & 0x1f;
    if ((value == 1) && (address == 0x2100))
      gbDataMBC1.mapperRomBank0Remapping = 1;

    if((value & 0x1f) == 0)
      value += 1;
    if(value == gbDataMBC1.mapperROMBank)
      break;

    tmpAddress = value << 14;

    // check current model
    if (gbDataMBC1.mapperRomBank0Remapping == 3) {
      tmpAddress = (value & 0xf) << 14;
      tmpAddress |= (gbDataMBC1.mapperROMHighAddress & 3) << 18;
    }
    else
    if(gbDataMBC1.mapperMemoryModel == 0) {
      // model is 16/8, so we have a high address in use
      tmpAddress |= (gbDataMBC1.mapperROMHighAddress & 3) << 19;
    }

    tmpAddress &= gbRomSizeMask;
    gbDataMBC1.mapperROMBank = value;
    gbMemoryMap[0x04] = &gbRom[tmpAddress];
    gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
    gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
    gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
    break;
  case 0x4000: // RAM bank select
    if(gbDataMBC1.mapperMemoryModel == 1) {
      if (!gbRamSize)
      {
        if (gbDataMBC1.mapperRomBank0Remapping == 3)
        {
          gbDataMBC1.mapperROMHighAddress = value & 0x03;
          tmpAddress = (gbDataMBC1.mapperROMHighAddress) << 18;
          tmpAddress &= gbRomSizeMask;
          gbMemoryMap[0x00] = &gbRom[tmpAddress];
          gbMemoryMap[0x01] = &gbRom[tmpAddress + 0x1000];
          gbMemoryMap[0x02] = &gbRom[tmpAddress + 0x2000];
          gbMemoryMap[0x03] = &gbRom[tmpAddress + 0x3000];
          gbMemoryMap[0x04] = &gbRom[tmpAddress + 0x4000];
          gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x5000];
          gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x6000];
          gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x7000];
        }
        else gbDataMBC1.mapperRomBank0Remapping = 0;
      }
      // 4/32 model, RAM bank switching provided
      value = value & 0x03;
      if(value == gbDataMBC1.mapperRAMBank)
        break;
      tmpAddress = value << 13;
      tmpAddress &= gbRamSizeMask;
      if(gbRamSize) {
        gbMemoryMap[0x0a] = &gbRam[tmpAddress];
        gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
      }
      gbDataMBC1.mapperRAMBank = value;
      gbDataMBC1.mapperRAMAddress = tmpAddress;

      if (gbDataMBC1.mapperRomBank0Remapping != 3)
        gbDataMBC1.mapperROMHighAddress = 0;
    } else {
      // 16/8, set the high address
      gbDataMBC1.mapperROMHighAddress = value & 0x03;
      tmpAddress = gbDataMBC1.mapperROMBank << 14;
      tmpAddress |= (gbDataMBC1.mapperROMHighAddress) << 19;
      tmpAddress &= gbRomSizeMask;
      gbMemoryMap[0x04] = &gbRom[tmpAddress];
      gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
      gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
      gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
      if(gbRamSize) {
        gbMemoryMap[0x0a] = &gbRam[0];
        gbMemoryMap[0x0b] = &gbRam[0x1000];
      }

      gbDataMBC1.mapperRAMBank = 0;
    }
    break;
  case 0x6000: // memory model select
    gbDataMBC1.mapperMemoryModel = value & 1;

    if(gbDataMBC1.mapperMemoryModel == 1) {
      // 4/32 model, RAM bank switching provided

      value = gbDataMBC1.mapperRAMBank & 0x03;
      tmpAddress = value << 13;
      tmpAddress &= gbRamSizeMask;
      if(gbRamSize) {
        gbMemoryMap[0x0a] = &gbRam[gbDataMBC1.mapperRAMAddress];
        gbMemoryMap[0x0b] = &gbRam[gbDataMBC1.mapperRAMAddress + 0x1000];
        gbDataMBC1.mapperRomBank0Remapping = 0;
      }
      else gbDataMBC1.mapperRomBank0Remapping |=2;

      gbDataMBC1.mapperRAMBank = value;
      gbDataMBC1.mapperRAMAddress = tmpAddress;

      tmpAddress = gbDataMBC1.mapperROMBank << 14;


      tmpAddress &= gbRomSizeMask;
      gbMemoryMap[0x04] = &gbRom[tmpAddress];
      gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
      gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
      gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

    } else {
      // 16/8, set the high address

      tmpAddress = gbDataMBC1.mapperROMBank << 14;
      tmpAddress |= (gbDataMBC1.mapperROMHighAddress) << 19;
      tmpAddress &= gbRomSizeMask;
      gbMemoryMap[0x04] = &gbRom[tmpAddress];
      gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
      gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
      gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
      if(gbRamSize) {
        gbMemoryMap[0x0a] = &gbRam[0];
        gbMemoryMap[0x0b] = &gbRam[0x1000];
      }
    }
    break;
  }
}

// MBC1 RAM write
void mapperMBC1RAM(u16 address, u8 value)
{
  if(gbDataMBC1.mapperRAMEnable) {
    if(gbRamSize) {
      gbMemoryMap[address >> 12][address & 0x0fff] = value;
      systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
    }
  }
}

// MBC1 read RAM
u8 mapperMBC1ReadRAM(u16 address)
{

  if(gbDataMBC1.mapperRAMEnable)
    return gbMemoryMap[address>>12][address & 0x0fff];

  if (!genericflashcardEnable)
    return 0xff;
  else
  if ((address & 0x1000) >= 0x1000)
  {
  // The value returned when reading RAM while it's disabled
  // is constant, exept for the GBASP hardware.
  // (actually, is the address that read is out of the ROM, the returned value if 0xff...)
    if (PC.W>=0xff80)
      return 0xff;
    else
    if ((gbHardware & 0x08) && (gbGBCColorType == 2))
    {
      if (address & 1)
        return 0xfb;
      else
        return 0x7a;
    }
    else
      return 0x0a;
  }
  else
    return gbDisabledRam[address & 7];
}

void memoryUpdateMapMBC1()
{
  int tmpAddress = gbDataMBC1.mapperROMBank << 14;

  // check current model
  if (gbDataMBC1.mapperRomBank0Remapping == 3) {
    tmpAddress = (gbDataMBC1.mapperROMHighAddress & 3) << 18;
    tmpAddress &= gbRomSizeMask;
    gbMemoryMap[0x00] = &gbRom[tmpAddress];
    gbMemoryMap[0x01] = &gbRom[tmpAddress + 0x1000];
    gbMemoryMap[0x02] = &gbRom[tmpAddress + 0x2000];
    gbMemoryMap[0x03] = &gbRom[tmpAddress + 0x3000];

    tmpAddress |= (gbDataMBC1.mapperROMBank & 0xf) << 14;
    gbMemoryMap[0x04] = &gbRom[tmpAddress];
    gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
    gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
    gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
  }
  else
  {
    if(gbDataMBC1.mapperMemoryModel == 0) {
      // model is 16/8, so we have a high address in use
      tmpAddress |= (gbDataMBC1.mapperROMHighAddress & 3) << 19;
    }

    tmpAddress &= gbRomSizeMask;
    gbMemoryMap[0x04] = &gbRom[tmpAddress];
    gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
    gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
    gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
  }

  if(gbRamSize) {
    if(gbDataMBC1.mapperMemoryModel == 1) {
      gbMemoryMap[0x0a] = &gbRam[gbDataMBC1.mapperRAMAddress];
      gbMemoryMap[0x0b] = &gbRam[gbDataMBC1.mapperRAMAddress + 0x1000];
    } else {
      gbMemoryMap[0x0a] = &gbRam[0];
      gbMemoryMap[0x0b] = &gbRam[0x1000];
    }
  }
}

mapperMBC2 gbDataMBC2 = {
  0, // RAM enable
  1  // ROM bank
};

// MBC2 ROM write registers
void mapperMBC2ROM(u16 address, u8 value)
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
void mapperMBC2RAM(u16 address, u8 value)
{
  if(gbDataMBC2.mapperRAMEnable) {
    if(gbRamSize && address < 0xa200) {
      gbMemoryMap[address >> 12][address & 0x0fff] = value;
      systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
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
    gbDataMBC3.mapperSeconds += (int)(diff % 60);
    if(gbDataMBC3.mapperSeconds > 59) {
      gbDataMBC3.mapperSeconds -= 60;
      gbDataMBC3.mapperMinutes++;
    }

    diff /= 60;

    gbDataMBC3.mapperMinutes += (int)(diff % 60);
    if(gbDataMBC3.mapperMinutes > 59) {
      gbDataMBC3.mapperMinutes -= 60;
      gbDataMBC3.mapperHours++;
    }

    diff /= 60;

    gbDataMBC3.mapperHours += (int)(diff % 24);
    if(gbDataMBC3.mapperHours > 23) {
      gbDataMBC3.mapperHours -= 24;
      gbDataMBC3.mapperDays++;
    }
    diff /= 24;

    gbDataMBC3.mapperDays += (int)(diff & 0xffffffff);
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
void mapperMBC3ROM(u16 address, u8 value)
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
void mapperMBC3RAM(u16 address, u8 value)
{
  if(gbDataMBC3.mapperRAMEnable) {
    if(gbDataMBC3.mapperRAMBank != -1) {
      if(gbRamSize) {
        gbMemoryMap[address>>12][address & 0x0fff] = value;
        systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
      }
    } else {
      time(&gbDataMBC3.mapperLastTime);
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
u8 mapperMBC3ReadRAM(u16 address)
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

  if (!genericflashcardEnable)
    return 0xff;
  else
  if ((address & 0x1000) >= 0x1000)
  {
  // The value returned when reading RAM while it's disabled
  // is constant, exept for the GBASP hardware.
  // (actually, is the address that read is out of the ROM, the returned value if 0xff...)
    if (PC.W>=0xff80)
      return 0xff;
    else
    if ((gbHardware & 0x08) && (gbGBCColorType == 2))
    {
      if (address & 1)
        return 0xfb;
      else
        return 0x7a;
    }
    else
      return 0x0a;
  }
  else
    return gbDisabledRam[address & 7];
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
void mapperMBC5ROM(u16 address, u8 value)
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
      gbMemoryMap[0x0a] = &gbRam[tmpAddress];
      gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];

      gbDataMBC5.mapperRAMBank = value;
      gbDataMBC5.mapperRAMAddress = tmpAddress;
    }
    break;
  }
}

// MBC5 RAM write
void mapperMBC5RAM(u16 address, u8 value)
{
  if(gbDataMBC5.mapperRAMEnable) {
    if(gbRamSize) {
      gbMemoryMap[address >> 12][address & 0x0fff] = value;
      systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
    }
  }
}

// MBC5 read RAM
u8 mapperMBC5ReadRAM(u16 address)
{

  if(gbDataMBC5.mapperRAMEnable)
    return gbMemoryMap[address>>12][address & 0x0fff];

  if (!genericflashcardEnable)
    return 0xff;
  else
  if ((address & 0x1000) >= 0x1000)
  {
  // The value returned when reading RAM while it's disabled
  // is constant, exept for the GBASP hardware.
  // (actually, is the address that read is out of the ROM, the returned value if 0xff...)
    if (PC.W>=0xff80)
      return 0xff;
    else
    if ((gbHardware & 0x08) && (gbGBCColorType == 2))
    {
      if (address & 1)
        return 0xfb;
      else
        return 0x7a;
    }
    else
      return 0x0a;
  }
  else
    return gbDisabledRam[address & 7];
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
void mapperMBC7ROM(u16 address, u8 value)
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
      gbMemoryMap[0x0a] = &gbMemory[0xa000];
      gbMemoryMap[0x0b] = &gbMemory[0xb000];

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
u8 mapperMBC7ReadRAM(u16 address)
{
  switch(address & 0xa0f0) {
  case 0xa000:
  case 0xa010:
  case 0xa060:
  case 0xa070:
    return 0;
  case 0xa020:
    // sensor X low byte
    return systemGetSensorX() & 255;
  case 0xa030:
    // sensor X high byte
    return systemGetSensorX() >> 8;
  case 0xa040:
    // sensor Y low byte
    return systemGetSensorY() & 255;
  case 0xa050:
    // sensor Y high byte
    return systemGetSensorY() >> 8;
  case 0xa080:
    return gbDataMBC7.value;
  }

  if (!genericflashcardEnable)
    return 0xff;
  else
  if ((address & 0x1000) >= 0x1000)
  {
  // The value returned when reading RAM while it's disabled
  // is constant, exept for the GBASP hardware.
  // (actually, is the address that read is out of the ROM, the returned value if 0xff...)
    if (PC.W>=0xff80)
      return 0xff;
    else
    if ((gbHardware & 0x08) && (gbGBCColorType == 2))
    {
      if (address & 1)
        return 0xfb;
      else
        return 0x7a;
    }
    else
      return 0x0a;
  }
  else
    return gbDisabledRam[address & 7];
}

// MBC7 RAM write
void mapperMBC7RAM(u16 address, u8 value)
{
  if(address == 0xa080) {
    // special processing needed
    int oldCs = gbDataMBC7.cs,oldSk=gbDataMBC7.sk;

    gbDataMBC7.cs=value>>7;
    gbDataMBC7.sk=(value>>6)&1;

    if(!oldCs && gbDataMBC7.cs) {
      if(gbDataMBC7.state==5) {
        if(gbDataMBC7.writeEnable) {
          gbMemory[0xa000+gbDataMBC7.address*2]=gbDataMBC7.buffer>>8;
          gbMemory[0xa000+gbDataMBC7.address*2+1]=gbDataMBC7.buffer&0xff;
          systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
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
                    gbMemory[0xa000+i*2] = gbDataMBC7.buffer >> 8;
                    gbMemory[0xa000+i*2+1] = gbDataMBC7.buffer & 0xff;
                    systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
                  }
                }
                gbDataMBC7.state=5;
              } else if((gbDataMBC7.address>>6) == 2) {
                if (gbDataMBC7.writeEnable) {
                  for(int i=0;i<256;i++)
                    WRITE16LE((u16 *)&gbMemory[0xa000+i*2], 0xffff);
                  systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
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
              gbDataMBC7.buffer = (gbMemory[0xa000+gbDataMBC7.address*2]<<8)|
                (gbMemory[0xa000+gbDataMBC7.address*2+1]);
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
  int tmpAddress = (gbDataMBC7.mapperROMBank << 14);

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
void mapperHuC1ROM(u16 address, u8 value)
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
void mapperHuC1RAM(u16 address, u8 value)
{
  if(gbDataHuC1.mapperRAMEnable) {
    if(gbRamSize) {
      gbMemoryMap[address >> 12][address & 0x0fff] = value;
      systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
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
void mapperHuC3ROM(u16 address, u8 value)
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
u8 mapperHuC3ReadRAM(u16 address)
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
void mapperHuC3RAM(u16 address, u8 value)
{
  int *p;

  if(gbDataHuC3.mapperRAMFlag < 0x0b ||
     gbDataHuC3.mapperRAMFlag > 0x0e) {
    if(gbDataHuC3.mapperRAMEnable) {
      if(gbRamSize) {
        gbMemoryMap[address >> 12][address & 0x0fff] = value;
        systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
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
    gbMemoryMap[0x0a] = &gbRam[tmpAddress];
    gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
  }
}

// TAMA5 (for Tamagotchi 3 (gb)).
// Very basic (and ugly :p) support, only rom bank switching is actually working...
mapperTAMA5 gbDataTAMA5 = {
  1, // RAM enable
  1, // ROM bank
  0, // RAM bank
  0, // RAM address
  0, // RAM Byte select
  0, // mapper command number
  0, // mapper last command;
  {
    0, // commands 0x0
    0, // commands 0x1
    0, // commands 0x2
    0, // commands 0x3
    0, // commands 0x4
    0, // commands 0x5
    0, // commands 0x6
    0, // commands 0x7
    0, // commands 0x8
    0, // commands 0x9
    0, // commands 0xa
    0, // commands 0xb
    0, // commands 0xc
    0, // commands 0xd
    0, // commands 0xe
    0  // commands 0xf
  },
  0, // register
  0, // timer clock latch
  0, // timer clock register
  0, // timer seconds
  0, // timer minutes
  0, // timer hours
  0, // timer days
  0, // timer months
  0, // timer years
  0, // timer control
  0, // timer latched seconds
  0, // timer latched minutes
  0, // timer latched hours
  0, // timer latched days
  0, // timer latched months
  0, // timer latched years
  0, // timer latched control
  (time_t)-1 // last time
};


void memoryUpdateTAMA5Clock()
{
  if ((gbDataTAMA5.mapperYears & 3) == 0)
      gbDaysinMonth[1] = 29;
  else
      gbDaysinMonth[1] = 28;

  time_t now = time(NULL);
  time_t diff = now - gbDataTAMA5.mapperLastTime;
  if(diff > 0) {
    // update the clock according to the last update time
    gbDataTAMA5.mapperSeconds += (int)(diff % 60);
    if(gbDataTAMA5.mapperSeconds > 59) {
      gbDataTAMA5.mapperSeconds -= 60;
      gbDataTAMA5.mapperMinutes++;
    }

    diff /= 60;

    gbDataTAMA5.mapperMinutes += (int)(diff % 60);
    if(gbDataTAMA5.mapperMinutes > 59) {
      gbDataTAMA5.mapperMinutes -= 60;
      gbDataTAMA5.mapperHours++;
    }

    diff /= 60;

    gbDataTAMA5.mapperHours += (int)(diff % 24);
    diff /= 24;
    if(gbDataTAMA5.mapperHours > 23) {
      gbDataTAMA5.mapperHours -= 24;
      diff++;

    }

    time_t days = diff;
    while (days)
    {
      gbDataTAMA5.mapperDays++;
      days--;
      if (gbDataTAMA5.mapperDays>gbDaysinMonth[gbDataTAMA5.mapperMonths-1])
      {
        gbDataTAMA5.mapperDays = 1;
        gbDataTAMA5.mapperMonths++;
        if (gbDataTAMA5.mapperMonths>12)
        {
           gbDataTAMA5.mapperMonths = 1;
           gbDataTAMA5.mapperYears++;
           if ((gbDataTAMA5.mapperYears & 3) == 0)
             gbDaysinMonth[1] = 29;
           else
             gbDaysinMonth[1] = 28;
        }
      }
    }
  }
  gbDataTAMA5.mapperLastTime = now;

}



// TAMA5 RAM write
void mapperTAMA5RAM(u16 address, u8 value)
{
  if ((address & 0xffff) <= 0xa001)
  {
    switch (address & 1)
    {
      case 0: // 'Values' Register
      {
        value &= 0xf;
        gbDataTAMA5.mapperCommands[gbDataTAMA5.mapperCommandNumber] = value;
        gbMemoryMap[0xa][0] = value;

/*        int test = gbDataTAMA5.mapperCommands[gbDataTAMA5.mapperCommandNumber & 0x0e] |
                                    (gbDataTAMA5.mapperCommands[(gbDataTAMA5.mapperCommandNumber & 0x0e) +1]<<4);*/

        if ((gbDataTAMA5.mapperCommandNumber & 0xe) == 0) // Read Command !!!
        {
          gbDataTAMA5.mapperROMBank = gbDataTAMA5.mapperCommands[0] |
                                     (gbDataTAMA5.mapperCommands[1]<<4);

          int  tmpAddress = (gbDataTAMA5.mapperROMBank << 14);

          tmpAddress &= gbRomSizeMask;
          gbMemoryMap[0x04] = &gbRom[tmpAddress];
          gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
          gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
          gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

          gbDataTAMA5.mapperCommands[0x0f] = 0;
        }
        else if ((gbDataTAMA5.mapperCommandNumber & 0xe) == 4)
        {
          gbDataTAMA5.mapperCommands[0x0f] = 1;
          if (gbDataTAMA5.mapperCommandNumber == 4)
            gbDataTAMA5.mapperCommands[5] =0; // correct ?
        }
        else if ((gbDataTAMA5.mapperCommandNumber & 0xe) == 6)
        {
          gbDataTAMA5.mapperRamByteSelect = (gbDataTAMA5.mapperCommands[7]<<4) |
                                            (gbDataTAMA5.mapperCommands[6]&0x0f);

          // Write Commands !!!
          if (gbDataTAMA5.mapperCommands[0x0f] && (gbDataTAMA5.mapperCommandNumber == 7))
          {
            int data = (gbDataTAMA5.mapperCommands[0x04] & 0x0f) |
                       (gbDataTAMA5.mapperCommands[0x05] <<4);

            // Not sure when the write command should reset...
            // but it doesn't seem to matter.
            // gbDataTAMA5.mapperCommands[0x0f] = 0;

            if (gbDataTAMA5.mapperRamByteSelect == 0x8) // Timer stuff
            {
              switch (data & 0xf)
              {
                case 0x7:
                  gbDataTAMA5.mapperDays = ((gbDataTAMA5.mapperDays)/10)*10 + (data >> 4);
                  break;
                case 0x8:
                  gbDataTAMA5.mapperDays = (gbDataTAMA5.mapperDays%10) + (data >>4)*10;
                  break;
                case 0x9:
                  gbDataTAMA5.mapperMonths = ((gbDataTAMA5.mapperMonths)/10)*10 + (data >> 4);
                  break;
                case 0xa:
                  gbDataTAMA5.mapperMonths = (gbDataTAMA5.mapperMonths%10) + (data >>4)*10;
                  break;
                case 0xb:
                  gbDataTAMA5.mapperYears = ((gbDataTAMA5.mapperYears)%1000) + (data >> 4)*1000;
                  break;
                case 0xc:
                  gbDataTAMA5.mapperYears = (gbDataTAMA5.mapperYears%100) + (gbDataTAMA5.mapperYears/1000)*1000 +
                                            (data >>4)*100;
                  break;
                default :
                  break;
              }
            }
            else if (gbDataTAMA5.mapperRamByteSelect == 0x18) // Timer stuff again
            {
              memoryUpdateTAMA5Clock();
              gbDataTAMA5.mapperLSeconds = gbDataTAMA5.mapperSeconds;
              gbDataTAMA5.mapperLMinutes = gbDataTAMA5.mapperMinutes;
              gbDataTAMA5.mapperLHours   = gbDataTAMA5.mapperHours;
              gbDataTAMA5.mapperLDays    = gbDataTAMA5.mapperDays;
              gbDataTAMA5.mapperLMonths   = gbDataTAMA5.mapperMonths;
              gbDataTAMA5.mapperLYears    = gbDataTAMA5.mapperYears;
              gbDataTAMA5.mapperLControl = gbDataTAMA5.mapperControl;

              int seconds = (gbDataTAMA5.mapperLSeconds / 10)*16 + gbDataTAMA5.mapperLSeconds %10;
              int secondsL = (gbDataTAMA5.mapperLSeconds % 10);
              int secondsH = (gbDataTAMA5.mapperLSeconds / 10);
              int minutes = (gbDataTAMA5.mapperLMinutes / 10)*16 + gbDataTAMA5.mapperLMinutes %10;
              int hours = (gbDataTAMA5.mapperLHours / 10)*16 + gbDataTAMA5.mapperLHours %10;
              int DaysL = gbDataTAMA5.mapperLDays % 10;
              int DaysH = gbDataTAMA5.mapperLDays /10;
              int MonthsL = gbDataTAMA5.mapperLMonths % 10;
              int MonthsH = gbDataTAMA5.mapperLMonths / 10;
              int Years3 = (gbDataTAMA5.mapperLYears / 100) % 10;
              int Years4 = (gbDataTAMA5.mapperLYears / 1000);

              switch (data & 0x0f)
              {
                // I guess cases 0 and 1 are used for secondsL and secondsH
                // so the game would update the timer values on screen when
                // the seconds reset to 0... ?
                case 0x0:
                  gbTAMA5ram[gbDataTAMA5.mapperRamByteSelect] = secondsL;
                  break;
                case 0x1:
                  gbTAMA5ram[gbDataTAMA5.mapperRamByteSelect] = secondsH;
                  break;
                case 0x7:
                  gbTAMA5ram[gbDataTAMA5.mapperRamByteSelect] = DaysL; // days low
                  break;
                case 0x8:
                  gbTAMA5ram[gbDataTAMA5.mapperRamByteSelect] = DaysH; // days high
                  break;
                case 0x9:
                  gbTAMA5ram[gbDataTAMA5.mapperRamByteSelect] = MonthsL; // month low
                  break;
                case 0xa:
                  gbTAMA5ram[gbDataTAMA5.mapperRamByteSelect] = MonthsH; // month high
                  break;
                case 0xb:
                  gbTAMA5ram[gbDataTAMA5.mapperRamByteSelect] = Years4; // years 4th digit
                  break;
                case 0xc:
                  gbTAMA5ram[gbDataTAMA5.mapperRamByteSelect] = Years3; // years 3rd digit
                  break;
                default :
                  break;
              }

              gbTAMA5ram[0x54] = seconds; // incorrect ? (not used by the game) ?
              gbTAMA5ram[0x64] = minutes;
              gbTAMA5ram[0x74] = hours;
              gbTAMA5ram[0x84] = DaysH*16+DaysL; // incorrect ? (not used by the game) ?
              gbTAMA5ram[0x94] = MonthsH*16+MonthsL; // incorrect ? (not used by the game) ?

              time(&gbDataTAMA5.mapperLastTime);

              gbMemoryMap[0xa][0] = 1;
            }
            else if (gbDataTAMA5.mapperRamByteSelect == 0x28) // Timer stuff again
            {
              if ((data & 0xf) == 0xb)
                 gbDataTAMA5.mapperYears = ((gbDataTAMA5.mapperYears>>2)<<2) + (data & 3);
            }
            else if (gbDataTAMA5.mapperRamByteSelect == 0x44)
            {
              gbDataTAMA5.mapperMinutes = (data/16)*10 + data%16;
            }
            else if (gbDataTAMA5.mapperRamByteSelect == 0x54)
            {
              gbDataTAMA5.mapperHours = (data/16)*10 + data%16;
            }
            else
            {
              gbTAMA5ram[gbDataTAMA5.mapperRamByteSelect] = data;
            }
          }
        }
      }
      break;
      case 1: // 'Commands' Register
      {
        gbMemoryMap[0xa][1] = gbDataTAMA5.mapperCommandNumber = value;

        // This should be only a 'is the flashrom ready ?' command.
        // However as I couldn't find any 'copy' command
        // (that seems to be needed for the saving system to work)
        // I put it there...
        if (value == 0x0a)
        {
          for (int i = 0; i<0x10; i++)
            for (int j = 0; j<0x10; j++)
              if (!(j&2))
                gbTAMA5ram[((i*0x10)+j) | 2] = gbTAMA5ram[(i*0x10)+j];
          // Enable this to see the content of the flashrom in 0xe000
          /*for (int k = 0; k<0x100; k++)
            gbMemoryMap[0xe][k] = gbTAMA5ram[k];*/

          gbMemoryMap[0xa][0] = gbDataTAMA5.mapperRAMEnable = 1;
        }
        else
        {
          if ((value & 0x0e) == 0x0c)
          {
            gbDataTAMA5.mapperRamByteSelect = gbDataTAMA5.mapperCommands[6] |
                                             (gbDataTAMA5.mapperCommands[7]<<4);

            u8 byte = gbTAMA5ram[gbDataTAMA5.mapperRamByteSelect];

            gbMemoryMap[0xa][0] = (value & 1) ? byte >> 4 : byte & 0x0f;

            gbDataTAMA5.mapperCommands[0x0f] = 0;
          }
        }
        break;
      }
    }
  }
  else
  {
    if(gbDataTAMA5.mapperRAMEnable) {
      if(gbDataTAMA5.mapperRAMBank != -1) {
        if(gbRamSize) {
          gbMemoryMap[address>>12][address & 0x0fff] = value;
          systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
        }
      }
    }
  }
}


// TAMA5 read RAM
u8 mapperTAMA5ReadRAM(u16 address)
{
  return gbMemoryMap[address>>12][address & 0xfff];
}


void memoryUpdateMapTAMA5()
{
  int tmpAddress = (gbDataTAMA5.mapperROMBank << 14);

  tmpAddress &= gbRomSizeMask;
  gbMemoryMap[0x04] = &gbRom[tmpAddress];
  gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
  gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
  gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

  if(gbRamSize) {
    tmpAddress = 0 << 13;
    tmpAddress &= gbRamSizeMask;
    gbMemoryMap[0x0a] = &gbRam[tmpAddress];
    gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
  }
}

// MMM01 Used in Momotarou collection (however the rom is corrupted)
mapperMMM01 gbDataMMM01 ={
  0, // RAM enable
  1, // ROM bank
  0, // RAM bank
  0, // memory model
  0, // ROM high address
  0, // RAM address
  0  // Rom Bank 0 remapping
};

// MMM01 ROM write registers
void mapperMMM01ROM(u16 address, u8 value)
{
  int tmpAddress = 0;

  switch(address & 0x6000) {
  case 0x0000: // RAM enable register
    gbDataMMM01.mapperRAMEnable = ( ( value & 0x0a) == 0x0a ? 1 : 0);
    break;
  case 0x2000: // ROM bank select
    //    value = value & 0x1f;
    if(value == 0)
      value = 1;
    if(value == gbDataMMM01.mapperROMBank)
      break;

    tmpAddress = value << 14;

    // check current model
    if(gbDataMMM01.mapperMemoryModel == 0) {
      // model is 16/8, so we have a high address in use
      tmpAddress |= (gbDataMMM01.mapperROMHighAddress) << 19;
    }
    else
      tmpAddress |= gbDataMMM01.mapperRomBank0Remapping << 18;

    tmpAddress &= gbRomSizeMask;
    gbDataMMM01.mapperROMBank = value;
    gbMemoryMap[0x04] = &gbRom[tmpAddress];
    gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
    gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
    gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];
    break;
  case 0x4000: // RAM bank select
    if(gbDataMMM01.mapperMemoryModel == 1) {
      // 4/32 model, RAM bank switching provided
      value = value & 0x03;
      if(value == gbDataMBC1.mapperRAMBank)
        break;
      tmpAddress = value << 13;
      tmpAddress &= gbRamSizeMask;
      gbMemoryMap[0x0a] = &gbRam[tmpAddress];
      gbMemoryMap[0x0b] = &gbRam[tmpAddress + 0x1000];
      gbDataMMM01.mapperRAMBank = value;
      gbDataMMM01.mapperRAMAddress = tmpAddress;
    } else {
      // 16/8, set the high address
      gbDataMMM01.mapperROMHighAddress = value & 0x03;
      tmpAddress = gbDataMMM01.mapperROMBank << 14;
      tmpAddress |= (gbDataMMM01.mapperROMHighAddress) << 19;
      tmpAddress &= gbRomSizeMask;
      gbMemoryMap[0x04] = &gbRom[tmpAddress];
      gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
      gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
      gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

      gbDataMMM01.mapperRomBank0Remapping = ((value<<1) | (value & 0x40 ? 1 : 0)) & 0xff;
      tmpAddress = gbDataMMM01.mapperRomBank0Remapping << 18;
      tmpAddress &= gbRomSizeMask;
      gbMemoryMap[0x00] = &gbRom[tmpAddress];
      gbMemoryMap[0x01] = &gbRom[tmpAddress + 0x1000];
      gbMemoryMap[0x02] = &gbRom[tmpAddress + 0x2000];
      gbMemoryMap[0x03] = &gbRom[tmpAddress + 0x3000];
    }
    break;
  case 0x6000: // memory model select
    gbDataMMM01.mapperMemoryModel = value & 1;
    break;
  }
}

// MMM01 RAM write
void mapperMMM01RAM(u16 address, u8 value)
{
  if(gbDataMMM01.mapperRAMEnable) {
    if(gbRamSize) {
      gbMemoryMap[address >> 12][address & 0x0fff] = value;
      systemSaveUpdateCounter = SYSTEM_SAVE_UPDATED;
    }
  }
}

void memoryUpdateMapMMM01()
{
  int tmpAddress = gbDataMMM01.mapperROMBank << 14;

  // check current model
  if(gbDataMMM01.mapperMemoryModel == 1) {
    // model is 16/8, so we have a high address in use
    tmpAddress |= (gbDataMMM01.mapperROMHighAddress) << 19;
  }

  tmpAddress &= gbRomSizeMask;
  gbMemoryMap[0x04] = &gbRom[tmpAddress];
  gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
  gbMemoryMap[0x06] = &gbRom[tmpAddress + 0x2000];
  gbMemoryMap[0x07] = &gbRom[tmpAddress + 0x3000];

  tmpAddress = gbDataMMM01.mapperRomBank0Remapping << 18;
  tmpAddress &= gbRomSizeMask;
  gbMemoryMap[0x00] = &gbRom[tmpAddress];
  gbMemoryMap[0x01] = &gbRom[tmpAddress + 0x1000];
  gbMemoryMap[0x02] = &gbRom[tmpAddress + 0x2000];
  gbMemoryMap[0x03] = &gbRom[tmpAddress + 0x3000];

  if(gbRamSize) {
    gbMemoryMap[0x0a] = &gbRam[gbDataMMM01.mapperRAMAddress];
    gbMemoryMap[0x0b] = &gbRam[gbDataMMM01.mapperRAMAddress + 0x1000];
  }
}

// GameGenie ROM write registers
void mapperGGROM(u16 address, u8 value)
{
  switch(address & 0x6000) {
  case 0x0000: // RAM enable register
    break;
  case 0x2000: // GameGenie has only a half bank
    break;
  case 0x4000: // GameGenie has no RAM
      if ((address >=0x4001) && (address <= 0x4020)) // GG Hardware Registers
        gbMemoryMap[address >> 12][address & 0x0fff] = value;
    break;
  case 0x6000: // GameGenie has only a half bank
    break;
  }
}


// GS3 Used to emulate the GS V3.0 rom bank switching
mapperGS3 gbDataGS3 = { 1 }; // ROM bank

void mapperGS3ROM(u16 address, u8 value)
{
  int tmpAddress = 0;

  switch(address & 0x6000) {
  case 0x0000: // GS has no ram
    break;
  case 0x2000: // GS has no 'classic' ROM bank select
    break;
  case 0x4000: // GS has no ram
    break;
  case 0x6000: // 0x6000 area is RW, and used for GS hardware registers

    if (address == 0x7FE1) // This is the (half) ROM bank select register
    {
      if(value == gbDataGS3.mapperROMBank)
        break;
      tmpAddress = value << 13;

      tmpAddress &= gbRomSizeMask;
      gbDataGS3.mapperROMBank = value;
      gbMemoryMap[0x04] = &gbRom[tmpAddress];
      gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
    }
    else
      gbMemoryMap[address>>12][address & 0x0fff] = value;
    break;
  }
}

void memoryUpdateMapGS3()
{
  int tmpAddress = gbDataGS3.mapperROMBank << 13;

  tmpAddress &= gbRomSizeMask;
  // GS can only change a half ROM bank
  gbMemoryMap[0x04] = &gbRom[tmpAddress];
  gbMemoryMap[0x05] = &gbRom[tmpAddress + 0x1000];
}
