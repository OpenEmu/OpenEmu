/* Cygne
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Dox dox@space.pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "wswan.h"
#include "gfx.h"
#include "memory.h"
#include "sound.h"
#include "eeprom.h"
#include "rtc.h"
#include "v30mz.h"
#include "../mempatcher.h"
#include <time.h>
#include <math.h>
#include <trio/trio.h>

namespace MDFN_IEN_WSWAN
{


static bool SkipSL; // Skip save and load

uint32 wsRAMSize;
uint8 wsRAM[65536];
static uint8 *wsSRAM = NULL;

uint8 *wsCartROM;
static uint32 sram_size;
uint32 eeprom_size;

static uint8 ButtonWhich, ButtonReadLatch;

static uint32 DMASource, DMADest;
static uint16 DMALength;
static uint8 DMAControl;

static uint32 SoundDMASource;
static uint16 SoundDMALength;
static uint8 SoundDMAControl;

static uint8 BankSelector[4];

static uint8 CommControl, CommData;

static bool language;

extern uint16 WSButtonStatus;

void WSwan_writemem20(uint32 A, uint8 V)
{
 uint32 offset, bank;

 offset = A & 0xffff;
 bank = (A>>16) & 0xF;

 if(!bank) /*RAM*/
 {
  WSwan_SoundCheckRAMWrite(offset);
  wsRAM[offset] = V;

  WSWan_TCacheInvalidByAddr(offset);

  if(offset>=0xfe00) /*WSC palettes*/
   WSwan_GfxWSCPaletteRAMWrite(offset, V);
 }
 else if(bank == 1) /* SRAM */
 {	 
  if(sram_size)
  {
   wsSRAM[(offset | (BankSelector[1] << 16)) & (sram_size - 1)] = V;
  }
 }
}



uint8 WSwan_readmem20(uint32 A)
{
 uint32	offset, bank;

 offset = A & 0xFFFF;
 bank = (A >> 16) & 0xF;
  
 switch(bank)
 {
	case 0:  return wsRAM[offset];
	case 1:  if(sram_size)
		 {
		  return wsSRAM[(offset | (BankSelector[1] << 16)) & (sram_size - 1)];
		 }
		 else
		  return(0);

	case 2:
	case 3:  return wsCartROM[offset+((BankSelector[bank]&((rom_size>>16)-1))<<16)];

	default: 
		{
		 uint8 bank_num = ((BankSelector[0] & 0xF) << 4) | (bank & 0xf);
		 bank_num &= (rom_size >> 16) - 1;
		 return(wsCartROM[(bank_num << 16) | offset]);
		}
 }
}

static void ws_CheckDMA(void)
{
 if(DMAControl & 0x80)
 {
  while(DMALength)
  {
   WSwan_writemem20(DMADest, WSwan_readmem20(DMASource));

   DMASource++; // = ((DMASource + 1) & 0xFFFF) | (DMASource & 0xFF0000);
   //if(!(DMASource & 0xFFFF)) puts("Warning: DMA source bank crossed.");
   DMADest = ((DMADest + 1) & 0xFFFF) | (DMADest & 0xFF0000);
   DMALength--;
  }
 }
 DMAControl &= ~0x80;
}

void WSwan_CheckSoundDMA(void)
{
 if(SoundDMAControl & 0x80)
 {
  if(SoundDMALength)
  {
   uint8 zebyte = WSwan_readmem20(SoundDMASource);

   if(SoundDMAControl & 0x08)
    zebyte ^= 0x80;

   if(SoundDMAControl & 0x10)
    WSwan_SoundWrite(0x95, zebyte); // Pick a port, any port?!
   else
    WSwan_SoundWrite(0x89, zebyte);

   SoundDMASource++; // = ((SoundDMASource + 1) & 0xFFFF) | (SoundDMASource & 0xFF0000);
   //if(!(SoundDMASource & 0xFFFF)) puts("Warning:  Sound DMA source bank crossed.");
   SoundDMALength--;
  }
  if(!SoundDMALength)
   SoundDMAControl &= ~0x80;
 }
}

uint8 WSwan_readport(uint32 number)
{
  number &= 0xFF;

  if(number >= 0x80 && number <= 0x9F)
   return(WSwan_SoundRead(number));
  else if(number <= 0x3F || (number >= 0xA0 && number <= 0xAF) || (number == 0x60))
   return(WSwan_GfxRead(number));
  else if((number >= 0xBA && number <= 0xBE) || (number >= 0xC4 && number <= 0xC8))
   return(WSwan_EEPROMRead(number));
  else if(number >= 0xCA && number <= 0xCB)
   return(WSwan_RTCRead(number));
  else switch(number)
  {
   //default: printf("Read: %04x\n", number); break;
   case 0x40: return(DMASource >> 0);
   case 0x41: return(DMASource >> 8);
   case 0x42: return(DMASource >> 16);

   case 0x43: return(DMADest >> 16);
   case 0x44: return(DMADest >> 0);
   case 0x45: return(DMADest >> 8);

   case 0x46: return(DMALength >> 0);
   case 0x47: return(DMALength >> 8);

   case 0x48: return(DMAControl);

   case 0xB0:
   case 0xB2:
   case 0xB6: return(WSwan_InterruptRead(number));

   case 0xC0: return(BankSelector[0] | 0x20);
   case 0xC1: return(BankSelector[1]);
   case 0xC2: return(BankSelector[2]);
   case 0xC3: return(BankSelector[3]);

   case 0x4a: return(SoundDMASource >> 0);
   case 0x4b: return(SoundDMASource >> 8);
   case 0x4c: return(SoundDMASource >> 16);
   case 0x4e: return(SoundDMALength >> 0);
   case 0x4f: return(SoundDMALength >> 8);
   case 0x52: return(SoundDMAControl);

   case 0xB1: return(CommData);

   case 0xb3: 
	     {
	      uint8 ret = CommControl & 0xf0;

	      if(CommControl & 0x80)
	       ret |= 0x4; // Send complete

	      return(ret);
	     }
   case 0xb5: 
	     {
	      uint8 ret = (ButtonWhich << 4) | ButtonReadLatch;
  	      return(ret);
	     }
 }

 if(number >= 0xC8)
  return(0xD0 | language);

 return(0);
}

void WSwan_writeport(uint32 IOPort, uint8 V)
{
 IOPort &= 0xFF;

	if(IOPort >= 0x80 && IOPort <= 0x9F)
	{
	 WSwan_SoundWrite(IOPort, V);
	}
	else if((IOPort >= 0x00 && IOPort <= 0x3F) || (IOPort >= 0xA0 && IOPort <= 0xAF) || (IOPort == 0x60))
	{
	 WSwan_GfxWrite(IOPort, V);
	}
	else if((IOPort >= 0xBA && IOPort <= 0xBE) || (IOPort >= 0xC4 && IOPort <= 0xC8))
	 WSwan_EEPROMWrite(IOPort, V);
	else if(IOPort >= 0xCA && IOPort <= 0xCB)
	 WSwan_RTCWrite(IOPort, V);
	else switch(IOPort)
	{
		//default: printf("%04x %02x\n", IOPort, V); break;

		case 0x40: DMASource &= 0xFFFF00; DMASource |= (V << 0); break;
		case 0x41: DMASource &= 0xFF00FF; DMASource |= (V << 8); break;
		case 0x42: DMASource &= 0x00FFFF; DMASource |= ((V & 0x0F) << 16); break;

		case 0x43: DMADest &= 0x00FFFF; DMADest |= ((V & 0x0F) << 16); break;
		case 0x44: DMADest &= 0xFFFF00; DMADest |= (V << 0); break;
		case 0x45: DMADest &= 0xFF00FF; DMADest |= (V << 8); break;

		case 0x46: DMALength &= 0xFF00; DMALength |= (V << 0); break;
		case 0x47: DMALength &= 0x00FF; DMALength |= (V << 8); break;

		case 0x48: DMAControl = V;
			   //if(V&0x80) 
			   // printf("DMA%02x: %08x %08x %08x\n", V, DMASource, DMADest, DMALength); 
			   ws_CheckDMA(); 
			   break;

                case 0x4a: SoundDMASource &= 0xFFFF00; SoundDMASource |= (V << 0); break;
                case 0x4b: SoundDMASource &= 0xFF00FF; SoundDMASource |= (V << 8); break;
                case 0x4c: SoundDMASource &= 0x00FFFF; SoundDMASource |= (V << 16); break;
		//case 0x4d: break; // Unused?
                case 0x4e: SoundDMALength &= 0xFF00; SoundDMALength |= (V << 0); break;
                case 0x4f: SoundDMALength &= 0x00FF; SoundDMALength |= (V << 8); break;
		//case 0x50: break; // Unused?
		//case 0x51: break; // Unused?
		case 0x52: SoundDMAControl = V; 
			   //if(V & 0x80) printf("Sound DMA: %02x, %08x %08x\n", V, SoundDMASource, SoundDMALength);
			   break;

	case 0xB0:
	case 0xB2:
	case 0xB6: WSwan_InterruptWrite(IOPort, V); break;

	case 0xB1: CommData = V; break;
	case 0xB3: CommControl = V & 0xF0; break;

	case 0xb5: ButtonWhich = V >> 4;
		   ButtonReadLatch = 0;

                   if(ButtonWhich & 0x4) /*buttons*/
		    ButtonReadLatch |= ((WSButtonStatus >> 8) << 1) & 0xF;

		   if(ButtonWhich & 0x2) /* H/X cursors */
		    ButtonReadLatch |= WSButtonStatus & 0xF;

		   if(ButtonWhich & 0x1) /* V/Y cursors */
		    ButtonReadLatch |= (WSButtonStatus >> 4) & 0xF;
                   break;

   case 0xC0: BankSelector[0] = V & 0xF; break;
   case 0xC1: BankSelector[1] = V; break;
   case 0xC2: BankSelector[2] = V; break;
   case 0xC3: BankSelector[3] = V; break;
 }
}

#ifdef WANT_DEBUGGER
static void GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 if(!strcmp(name, "ram"))
 {
  while(Length--)
  {
   Address &= wsRAMSize - 1;
   *Buffer = wsRAM[Address];
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "physical"))
 {
  while(Length--)
  {
   Address &= 0xFFFFF;
   *Buffer = WSwan_readmem20(Address);
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "cs") || !strcmp(name, "ds") || !strcmp(name, "ss") || !strcmp(name, "es"))
 {
  uint32 segment;
  uint32 phys_address;

  if(!strcmp(name, "cs"))
   segment = v30mz_get_reg(NEC_PS);
  else if(!strcmp(name, "ss"))
   segment = v30mz_get_reg(NEC_SS);
  else if(!strcmp(name, "ds"))
   segment = v30mz_get_reg(NEC_DS0);
  else if(!strcmp(name, "es"))
   segment = v30mz_get_reg(NEC_DS1);

  phys_address = (Address + (segment << 4)) & 0xFFFFF;

  GetAddressSpaceBytes("physical", phys_address, Length, Buffer);
 }
}

static void PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 if(!strcmp(name, "ram"))
 {
  while(Length--)
  {
   Address &= wsRAMSize - 1;
   wsRAM[Address] = *Buffer;
   WSWan_TCacheInvalidByAddr(Address);
   if(Address >= 0xfe00)
    WSwan_GfxWSCPaletteRAMWrite(Address, *Buffer);

   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "physical"))
 {
  while(Length--)
  {
   uint32 offset, bank;

   Address &= 0xFFFFF;

   offset = Address & 0xFFFF;
   bank = (Address >> 16) & 0xF;
  
   switch(bank)
   {
    case 0:  wsRAM[offset & (wsRAMSize - 1)] = *Buffer;
	     WSWan_TCacheInvalidByAddr(offset & (wsRAMSize - 1));
	     if(Address >= 0xfe00)
	      WSwan_GfxWSCPaletteRAMWrite(offset & (wsRAMSize - 1), *Buffer);
	     break;
    case 1:  if(sram_size)
	       wsSRAM[(offset | (BankSelector[1] << 16)) & (sram_size - 1)] = *Buffer;
	     break;
    case 2:
    case 3:  wsCartROM[offset+((BankSelector[bank]&((rom_size>>16)-1))<<16)] = *Buffer;
	     break;

    default:
            {
             uint8 bank_num = ((BankSelector[0] & 0xF) << 4) | (bank & 0xf);
             bank_num &= (rom_size >> 16) - 1;
             wsCartROM[(bank_num << 16) | offset] = *Buffer;
            }
	    break;
   }

   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "cs") || !strcmp(name, "ds") || !strcmp(name, "ss") || !strcmp(name, "es"))
 {
  uint32 segment;
  uint32 phys_address;

  if(!strcmp(name, "cs"))
   segment = v30mz_get_reg(NEC_PS);
  else if(!strcmp(name, "ss"))
   segment = v30mz_get_reg(NEC_SS);
  else if(!strcmp(name, "ds"))
   segment = v30mz_get_reg(NEC_DS0);
  else if(!strcmp(name, "es"))
   segment = v30mz_get_reg(NEC_DS1);

  phys_address = (Address + (segment << 4)) & 0xFFFFF;

  PutAddressSpaceBytes("physical", phys_address, Length, Granularity, hl, Buffer);
 }
}

uint32 WSwan_MemoryGetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 uint32 ret = 0;

 switch(id)
 {
  case MEMORY_GSREG_ROMBBSLCT:
        ret = BankSelector[0];

	if(special)
	{
         trio_snprintf(special, special_len, "((0x%02x * 0x100000) %% 0x%08x) + 20 bit address = 0x%08x + 20 bit address", BankSelector[0], rom_size, (BankSelector[0] * 0x100000) & (rom_size - 1));
	}
        break;

  case MEMORY_GSREG_BNK1SLCT:
        ret = BankSelector[1];
        break;

  case MEMORY_GSREG_BNK2SLCT:
        ret = BankSelector[2];
        break;

  case MEMORY_GSREG_BNK3SLCT:
        ret = BankSelector[3];
        break;
 }

 return(ret);
}

void WSwan_MemorySetRegister(const unsigned int id, uint32 value)
{
 switch(id)
 {
  case MEMORY_GSREG_ROMBBSLCT:
	BankSelector[0] = value;
	break;

  case MEMORY_GSREG_BNK1SLCT:
	BankSelector[1] = value;
	break;

  case MEMORY_GSREG_BNK2SLCT:
	BankSelector[2] = value;
	break;

  case MEMORY_GSREG_BNK3SLCT:
	BankSelector[3] = value;
	break;
 }
}

#endif

void WSwan_MemoryKill(void)
{
 if((sram_size || eeprom_size) && !SkipSL)
 {
  std::vector<PtrLengthPair> EvilRams;

  if(eeprom_size)
   EvilRams.push_back(PtrLengthPair(wsEEPROM, eeprom_size));

  if(sram_size)
   EvilRams.push_back(PtrLengthPair(wsSRAM, sram_size));

  MDFN_DumpToFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), 6, EvilRams);
 }

 if(wsSRAM)
 {
  free(wsSRAM);
  wsSRAM = NULL;
 }
}

void WSwan_MemoryInit(bool lang, bool IsWSC, uint32 ssize, bool SkipSaveLoad)
{
 const uint16 byear = MDFN_GetSettingUI("wswan.byear");
 const uint8 bmonth = MDFN_GetSettingUI("wswan.bmonth");
 const uint8 bday = MDFN_GetSettingUI("wswan.bday");
 const uint8 sex = MDFN_GetSettingI("wswan.sex");
 const uint8 blood = MDFN_GetSettingI("wswan.blood");

 language = lang;
 SkipSL = SkipSaveLoad;

 wsRAMSize = 65536;
 sram_size = ssize;

 #ifdef WANT_DEBUGGER
 {
  ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "physical", "CPU Physical", 20);
  ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "ram", "RAM", (int)(log(wsRAMSize) / log(2)));

  ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "cs", "Code Segment", 16);
  ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "ss", "Stack Segment", 16);
  ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "ds", "Data Segment", 16);
  ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "es", "Extra Segment", 16);
 }
 #endif

 // WSwan_EEPROMInit() will also clear wsEEPROM
 WSwan_EEPROMInit(MDFN_GetSettingS("wswan.name").c_str(), byear, bmonth, bday, sex, blood);

 if(sram_size)
 {
  wsSRAM = (uint8*)malloc(sram_size);
  memset(wsSRAM, 0, sram_size);
 }

 if((sram_size || eeprom_size) && !SkipSL)
 {
  gzFile savegame_fp;

  savegame_fp = gzopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), "rb");
  if(savegame_fp)
  {
   if(eeprom_size)
    gzread(savegame_fp, wsEEPROM, eeprom_size);
   if(sram_size)
    gzread(savegame_fp, wsSRAM, sram_size);
   gzclose(savegame_fp);
  }
 }

 MDFNMP_AddRAM(wsRAMSize, 0x00000, wsRAM);

 if(sram_size)
  MDFNMP_AddRAM(sram_size, 0x10000, wsSRAM);
}

void WSwan_MemoryReset(void)
{
 memset(wsRAM, 0, 65536);

 wsRAM[0x75AC] = 0x41;
 wsRAM[0x75AD] = 0x5F;
 wsRAM[0x75AE] = 0x43;
 wsRAM[0x75AF] = 0x31;
 wsRAM[0x75B0] = 0x6E;
 wsRAM[0x75B1] = 0x5F;
 wsRAM[0x75B2] = 0x63;
 wsRAM[0x75B3] = 0x31;

 memset(BankSelector, 0, sizeof(BankSelector));
 ButtonWhich = 0;
 ButtonReadLatch = 0;
 DMASource = 0;
 DMADest = 0;
 DMALength = 0;
 DMAControl = 0;

 SoundDMASource = 0;
 SoundDMALength = 0;
 SoundDMAControl = 0;

 CommControl = 0;
 CommData = 0;
}

int WSwan_MemoryStateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAYN(wsRAM, 65536, "RAM"),
  SFARRAYN(sram_size ? wsSRAM : NULL, sram_size, "SRAM"),
  SFVAR(ButtonWhich),
  SFVAR(ButtonReadLatch),
  SFVAR(WSButtonStatus),
  SFVAR(DMASource),
  SFVAR(DMADest),
  SFVAR(DMALength),
  SFVAR(DMAControl),

  SFVAR(SoundDMASource),
  SFVAR(SoundDMALength),
  SFVAR(SoundDMAControl),

  SFVAR(CommControl),
  SFVAR(CommData),

  SFARRAY(BankSelector, 4),

  SFEND
 };

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "MEMR"))
  return(0);

 if(load)
 {
  for(uint32 A = 0xfe00; A <= 0xFFFF; A++)
  {
   WSwan_GfxWSCPaletteRAMWrite(A, wsRAM[A]);
  }
 }
 return(1);
}

}
