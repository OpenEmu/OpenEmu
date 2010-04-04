/* Mednafen - Multi-system Emulator
 *
 *  Portions of this file Copyright (C) 2004 Ki
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

#include <errno.h>
#include <string.h>

#include "pce.h"
#include "cdrom.h"
#include "adpcm.h"
#include "../md5.h"
#include "../file.h"
#include "../cdrom/cdromif.h"
#include "../mempatcher.h"

static const char BRAM_Init_String[8] = { 'H', 'U', 'B', 'M', 0x00, 0x88, 0x10, 0x80 }; //"HUBM\x00\x88\x10\x80";

static uint8 *HuCROM;
static uint8 HuCROMMask;
static uint8 *ROMMap[0x100];

static bool IsPopulous;
bool IsTsushin;
bool PCE_IsCD;

static uint8 *TsushinRAM = NULL; // 0x8000
static uint8 *PopRAM = NULL; // 0x8000
static uint8 SaveRAM[2048];
static uint8 *CDRAM = NULL; //262144;
static uint8 *ACRAM = NULL; //0x200000 Yes, *2* MEGABYTES.  wow.
static uint8 ACRAMUsed;	      // Something to speed up save states.
static uint32 ACShift, ACShiftBits;
static uint8 AC1ae5;

typedef struct
{
	uint8 control;
	uint32 base;
	uint16 offset;
	uint16 increment;
} ACIO;

static ACIO AC[4];

static INLINE void increment_acaddr(ACIO* port)
{
        if (port->control & 1)          // CONFIRMED:  D0 enables base / offset increment
        {
                if (port->control & 0x10)       // CONFIRMED: D4 selects base / offset to be incremented
                {
                        port->base += port->increment;
                        port->base &= 0xffffff;
                }
                else
                {
                        port->offset += port->increment;
                }
        }
}

DECLFR(PCE_ACRead)
{
	ACIO *port = &AC[(A >> 4) & 0x3];
	uint8 ret;

	if((A & 0x1ae0) == 0x1ae0)
	{
	 switch(A & 0x1aef)
	 {
	  case 0x1ae0: return (uint8)ACShift;
	  case 0x1ae1: return (uint8)(ACShift >> 8);
	  case 0x1ae2: return (uint8)(ACShift >> 16);
	  case 0x1ae3: return (uint8)(ACShift >> 24);
	  case 0x1ae4: return (uint8)ACShiftBits;
	  case 0x1ae5: return(AC1ae5);
	  case 0x1aee: return(0x10);
	  case 0x1aef: return(0x51);
	 }
	 return(0xFF);
	}

	switch(A & 0xF)
	{
	 case 0x0:
	 case 0x1:
		if(ACRAMUsed)
		{
                 uint32 aci;

                 aci = port->base;
                 if(port->control & 0x2)
                 {
                  aci += port->offset;
                  if(port->control & 0x8)
                   aci += 0xFF0000;
                 }
                 aci &= 0x1FFFFF;
		 ret = ACRAM[aci];
		}
		else
		 ret = 0;

		if(!PCE_InDebug)
		 increment_acaddr(port);

		return(ret);
	 case 0x2: return (uint8)port->base;
	 case 0x3: return (uint8)(port->base >> 8);
	 case 0x4: return (uint8)(port->base >> 16);
	 case 0x5: return (uint8)(port->offset);
	 case 0x6: return (uint8)(port->offset >> 8);
	 case 0x7: return (uint8)(port->increment);
	 case 0x8: return (uint8)(port->increment >> 8);
	 case 0x9: return(port->control);
	 case 0xa: return(0x00);
	}
	return(0xFF);
}

DECLFW(PCE_ACWrite)
{
        if ((A & 0x1ae0) == 0x1ae0)
        {
                switch (A & 0xf)
                {
                        case 0:
                                ACShift = (ACShift & ~0xff) | V;
                                return;
                        case 1:
                                ACShift = (ACShift & ~0xff00) | (V << 8);
                                return;
                        case 2:
                                ACShift = (ACShift & ~0xff0000) | (V << 16);
                                return;
                        case 3:
                                ACShift = (ACShift & ~0xff000000) | (V << 24);
                                return;
                        case 4:
                                if ((ACShiftBits = V & 0xf) != 0)
                                {
                                        if (ACShiftBits < 8)
                                                ACShift <<= ACShiftBits;
                                        else
                                                ACShift >>= 16 - ACShiftBits;
                                }
                                return;
                        case 5:
                                AC1ae5 = V;
                                return;
                }
        }
        else
        {
                ACIO            *port = &AC[(A >> 4) & 3];

                switch (A & 0xf)
                {
                        case 0x0:
                        case 0x1:
				if(!ACRAMUsed)
				{
				 ACRAMUsed = 1;
				 if(!(ACRAM = (uint8*)MDFN_calloc(0x200000, 1, _("Arcade Card RAM"))))
				 {
				  exit(1); // FIXME!  
				 }
				}

				{
                 		 uint32 aci;

                 		 aci = port->base;
		                 if(port->control & 0x2)
		                 {
		                  aci += port->offset;
		                  if(port->control & 0x8)
		                   aci += 0xFF0000;
		                 }
		                 aci &= 0x1FFFFF;
		                 ACRAM[aci] = V;
				}
                                increment_acaddr(port);
                                return;

                        case 0x2:
                                port->base = (port->base & ~0xff) | V;
                                return;
                        case 0x3:
                                port->base = (port->base & ~0xff00) | (V << 8);
                                return;
                        case 0x4:
                                port->base = (port->base & ~0xff0000) | (V << 16);
                                return;
                        case 0x5:
                                port->offset = (port->offset & ~0xff) | V;
                                return;
                        case 0x6:
                                port->offset = (port->offset & ~0xff00) | (V << 8);

                                if ((port->control & 0x60) == 0x40)
                                {
                                 port->base += port->offset;

			         if(port->control & 0x08)
			          port->base += 0xFF0000;

                                 port->base &= 0xffffff;
                                }
                                return;
                        case 0x7:
                                port->increment = (port->increment & ~0xff) | V;
                                return;
                        case 0x8:
                                port->increment = (port->increment & ~0xff00) | (V << 8);
                                return;
                        case 0x9:
                                port->control = V & 0x7f;            // D7 is not used
				//if((V & 0x0C)) printf("%02x\n", V);
                                return;
                        case 0xa:
                                // value written is not used
                                if ((port->control & 0x60) == 0x60)
                                {
                                 port->base += port->offset;

			         if(port->control & 0x08)
			          port->base += 0xFF0000;

                                 port->base &= 0xffffff;
                                }
                                return;
                }
        }
}

static DECLFW(ACPhysWrite)
{
 PCE_ACWrite(0x1a00 | ((A >> 9) & 0x30), V);
}

static DECLFR(ACPhysRead)
{
 return(PCE_ACRead(0x1a00 | ((A >> 9) & 0x30)));
}

#ifdef WANT_DEBUGGER
static void AC_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 while(Length--)
 {
  Address &= (1 << 21) - 1;

  if(ACRAMUsed)
   *Buffer = ACRAM[Address];
  else
   *Buffer = 0x00;

  Address++;
  Buffer++;
 }
}

void AC_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 while(Length--)
 {
  Address &= (1 << 21) - 1;

  if(ACRAMUsed)
   ACRAM[Address] = *Buffer;
  else if(*Buffer)
  {
   ACRAMUsed = 1;
   if(!(ACRAM = (uint8*)MDFN_calloc(0x200000, 1, _("Arcade Card RAM"))))
   {
    exit(1); // FIXME!
   }
   ACRAM[Address] = *Buffer;
  }
  Address++;
  Buffer++;
 }
}

#endif


static DECLFW(CDRAMWrite)
{
 CDRAM[A - 0x68 * 8192] = V;
}

static DECLFR(CDRAMRead)
{
 return(CDRAM[A - 0x68 * 8192]);
}

static DECLFR(SaveRAMRead)
{
 if((!PCE_IsCD || PCECD_IsBRAMEnabled()) && (A & 8191) < 2048)
  return(SaveRAM[A & 2047]);
 else
  return(0xFF);
}

static DECLFW(SaveRAMWrite)
{
 if((!PCE_IsCD || PCECD_IsBRAMEnabled()) && (A & 8191) < 2048)
  SaveRAM[A & 2047] = V;
}

static DECLFR(HuCRead)
{
 return(ROMMap[A >> 13][A]);
}

static DECLFW(HuCRAMWrite)
{
 ROMMap[A >> 13][A] = V;
}

static uint8 HuCSF2Latch = 0;

static DECLFR(HuCSF2ReadLow)
{
 return(HuCROM[A]);
}

static DECLFR(HuCSF2Read)
{
 return(HuCROM[(A & 0x7FFFF) + 0x80000 + HuCSF2Latch * 0x80000 ]); // | (HuCSF2Latch << 19) ]);
}

static DECLFW(HuCSF2Write)
{
 if((A & 0x1FFC) == 0x1FF0)
 {
  HuCSF2Latch = (A & 0x3);
 }
}

int HuCLoad(uint8 *data, uint32 len, uint32 crc32)
{
 uint32 sf2_threshold = 2048 * 1024;
 uint32 sf2_required_size = 2048 * 1024 + 512 * 1024;
 uint32 m_len = (len + 8191)&~8191;
 bool sf2_mapper = FALSE;

 if(m_len >= sf2_threshold)
 {
  sf2_mapper = TRUE;

  if(m_len != sf2_required_size)
   m_len = sf2_required_size;
 }

 IsPopulous = 0;
 PCE_IsCD = 0;

 md5_context md5;
 md5.starts();
 md5.update(data, len);
 md5.finish(MDFNGameInfo->MD5);

 MDFN_printf(_("ROM:       %dKiB\n"), (len + 1023) / 1024);
 MDFN_printf(_("ROM CRC32: 0x%08x\n"), crc32);
 MDFN_printf(_("ROM MD5:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());

 if(!(HuCROM = (uint8 *)MDFN_malloc(m_len, _("HuCard ROM"))))
 {
  return(0);
 }

 memset(HuCROM, 0xFF, m_len);
 memcpy(HuCROM, data, len);

 if(m_len == 0x60000)
 {
  for(int x = 0; x < 128; x++)
  {
   ROMMap[x] = HuCPUFastMap[x] = &HuCROM[(x & 0x1F) * 8192] - x * 8192;
   PCERead[x] = HuCRead;
  }

  for(int x = 64; x < 128; x++)
  {
   ROMMap[x] = HuCPUFastMap[x] = &HuCROM[((x & 0xF) + 32) * 8192] - x * 8192;
   PCERead[x] = HuCRead;
  }
 }
 else if(m_len == 0x80000)
 {
  for(int x = 0; x < 64; x++)
  {
   ROMMap[x] = HuCPUFastMap[x] = &HuCROM[(x & 0x3F) * 8192] - x * 8192;
   PCERead[x] = HuCRead;
  }
  for(int x = 64; x < 128; x++)
  {
   ROMMap[x] = HuCPUFastMap[x] = &HuCROM[((x & 0x1F) + 32) * 8192] - x * 8192;
   PCERead[x] = HuCRead;
  }
 }
 else
 {
  for(int x = 0; x < 128; x++)
  {
   uint8 bank = x % (m_len / 8192);
   ROMMap[x] = HuCPUFastMap[x] = &HuCROM[bank * 8192] - x * 8192;
   PCERead[x] = HuCRead;
  }
 }

 HuCROMMask = (m_len / 8192) - 1;

 if(!memcmp(HuCROM + 0x1F26, "POPULOUS", strlen("POPULOUS")))
 {
  gzFile fp;
  
  if(!(PopRAM = (uint8 *)MDFN_malloc(32768, _("Populous RAM"))))
  {
   MDFN_free(HuCROM);
   return(0);
  }
  memset(PopRAM, 0xFF, 32768);
  if((fp = gzopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), "rb")))
  {
   gzread(fp, PopRAM, 32768);
   gzclose(fp);
  }
  IsPopulous = 1;
  MDFN_printf("Populous\n");
  for(int x = 0x40; x < 0x44; x++)
  {
   ROMMap[x] = HuCPUFastMap[x] = &PopRAM[(x & 3) * 8192] - x * 8192;
   PCERead[x] = HuCRead;
   PCEWrite[x] = HuCRAMWrite;
  }
  MDFNMP_AddRAM(32768, 0x40 * 8192, PopRAM);
 }
 else if(crc32 == 0x34dc65c4) // Tsushin Booster
 {
  gzFile fp;

  if(!(TsushinRAM = (uint8*)MDFN_malloc(0x8000, _("Tsushin Booster RAM"))))
  {
   MDFN_free(HuCROM);
   return(0);
  }
  memset(TsushinRAM, 0xFF, 0x8000);

  if((fp = gzopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), "rb")))
  {
   gzread(fp, TsushinRAM, 32768);
   gzclose(fp);
  }
  IsTsushin = 1;
  MDFN_printf("Tsushin Booster\n");
  for(int x = 0x88; x < 0x8C; x++)
  {
   ROMMap[x] = HuCPUFastMap[x] = &TsushinRAM[(x & 3) * 8192] - x * 8192;
   PCERead[x] = HuCRead;
   PCEWrite[x] = HuCRAMWrite;
  }
  MDFNMP_AddRAM(32768, 0x88 * 8192, TsushinRAM);
 }
 else
 {
  gzFile fp;

  memset(SaveRAM, 0x00, 2048);
  memcpy(SaveRAM, BRAM_Init_String, 8);    // So users don't have to manually intialize the file cabinet
                                                // in the CD BIOS screen.
  if((fp = gzopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), "rb")))
  {
   gzread(fp, SaveRAM, 2048);
   gzclose(fp);
  }
  PCEWrite[0xF7] = SaveRAMWrite;
  PCERead[0xF7] = SaveRAMRead;
  MDFNMP_AddRAM(2048, 0xF7 * 8192, SaveRAM);
 }

 // 0x1A558
 //if(len >= 0x20000 && !memcmp(HuCROM + 0x1A558, "STREET FIGHTER#", strlen("STREET FIGHTER#")))
 if(sf2_mapper)
 {
  for(int x = 0x20; x < 0x40; x++)
   PCERead[x] = HuCSF2ReadLow;
  for(int x = 0x40; x < 0x80; x++)
  {
   HuCPUFastMap[x] = NULL; // Make sure our reads go through our read function, and not a table lookup
   PCERead[x] = HuCSF2Read;
  }
  PCEWrite[0] = HuCSF2Write;
  MDFN_printf("Street Fighter 2 Mapper");
  HuCSF2Latch = 0;
 }

 return(1);
}

bool IsBRAMUsed(void)
{
 if(memcmp(SaveRAM, BRAM_Init_String, 8)) // HUBM string is modified/missing
  return(1);

 for(int x = 8; x < 2048; x++)
  if(SaveRAM[x]) return(1);

 return(0);
}

int HuCLoadCD(const char *bios_path)
{
 MDFNFILE *fp = MDFN_fopen(bios_path, NULL, "rb", ".pce\0.bin\0");
 if(!fp) 
 {
  MDFN_PrintError(_("Could not open CD BIOS file \"%s\": %s\n"), bios_path, strerror(errno));
  return(0);
 }
 if(!(HuCROM = (uint8 *)MDFN_malloc(262144, _("BIOS ROM"))))
 {
  return(0);
 }
 memset(HuCROM, 0xFF, 262144);
 if(fp->size & 512)
  MDFN_fseek(fp, 512, SEEK_SET);
 MDFN_fread(HuCROM, 1, 262144, fp);
 MDFN_fclose(fp);

 ACRAMUsed = 0;
 PCE_IsCD = 1;
 PCECD_Init();
 ADPCM_Init();

 md5_context md5;
 md5.starts();
// md5_update(&md5, HuCROM, 262144);


 int32 track = CDIF_GetFirstTrack();
 int32 last_track = CDIF_GetLastTrack();
 bool DTFound = 0;
 for(; track <= last_track; track++)
 {
  CDIF_Track_Format format;

  if(CDIF_GetTrackFormat(track, format) && format == CDIF_FORMAT_MODE1)
  {
   DTFound = 1;
   break;
  }
 }
 
 if(DTFound) // Only add the MD5 hash if we were able to find a data track.
 {
  uint32 start_sector = CDIF_GetTrackStartPositionLBA(track);
  uint8 sector_buffer[2048];

  for(int x = 0; x < 128; x++)
  {
   memset(sector_buffer, 0, 2048);
   CDIF_ReadSector(sector_buffer, NULL, start_sector + x, 1);
   md5.update(sector_buffer, 2048);
  }
 }
 md5.finish(MDFNGameInfo->MD5);
 MDFN_printf(_("CD MD5(first 256KiB):   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());
 MDFN_printf(_("Arcade Card Emulation:  %s\n"), PCE_ACEnabled ? _("Enabled") : _("Disabled")); 
 for(int x = 0; x < 0x40; x++)
 {
  ROMMap[x] = HuCPUFastMap[x] = &HuCROM[(x & 0x1F) * 8192] - x * 8192;
  PCERead[x] = HuCRead;
 }

 if(!(CDRAM = (uint8 *)MDFN_malloc(262144, _("CD RAM"))))
 {
  MDFN_free(HuCROM);
  return(0);
 }

 for(int x = 0x68; x < 0x88; x++)
 {
  ROMMap[x] = HuCPUFastMap[x] = &CDRAM[(x - 0x68) * 8192] - x * 8192;
  PCERead[x] = CDRAMRead;
  PCEWrite[x] = CDRAMWrite;
 }
 MDFNMP_AddRAM(262144, 0x68 * 8192, CDRAM);

 if(PCE_ACEnabled)
 {
  for(int x = 0x40; x < 0x48; x++)
  {
   ROMMap[x] = HuCPUFastMap[x] = NULL;
   PCERead[x] = ACPhysRead;
   PCEWrite[x] = ACPhysWrite;
  }
 }

 gzFile srp;

 memset(SaveRAM, 0x00, 2048);
 memcpy(SaveRAM, BRAM_Init_String, 8);	// So users don't have to manually intialize the file cabinet
						// in the CD BIOS screen.

 if((srp = gzopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), "rb")))
 {
  gzread(srp, SaveRAM, 2048);
  gzclose(srp);
 }
 PCEWrite[0xF7] = SaveRAMWrite;
 PCERead[0xF7] = SaveRAMRead;
 MDFNMP_AddRAM(2048, 0xF7 * 8192, SaveRAM);


 #ifdef WANT_DEBUGGER
 MDFNDBG_AddASpace(AC_GetAddressSpaceBytes, AC_PutAddressSpaceBytes, "acram", "Arcade Card RAM", 21);
 #endif


 return(1);
}

int HuC_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] = 
 {
  SFARRAY(PopRAM, IsPopulous ? 32768 : 0),
  SFARRAY(TsushinRAM, IsTsushin ? 32768 : 0),
  SFARRAY(SaveRAM, (IsPopulous || IsTsushin) ? 0 : 2048),
  SFARRAY(CDRAM, PCE_IsCD ? 262144 : 0),
  SFVAR(HuCSF2Latch),
  SFVAR(ACRAMUsed),	// This needs to be saved/loaded before the ACStateRegs[] struct is saved/loaded.
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "HuC");
 if(load)
  HuCSF2Latch &= 0x3;

 if(PCE_ACEnabled)
 {
  if(ACRAM && !ACRAMUsed)
  {
   MDFN_free(ACRAM);
   ACRAM = NULL;
  }
  else if(!ACRAM && ACRAMUsed)
  {
   if(!(ACRAM = (uint8*)MDFN_calloc(0x200000, 1, _("Arcade Card RAM"))))
   {
    exit(1); // FIXME!
   }
  }

  SFORMAT ACStateRegs[] =
  {
   SFVAR(AC[0].control), SFVAR(AC[0].base), SFVAR(AC[0].offset), SFVAR(AC[0].increment),
   SFVAR(AC[1].control), SFVAR(AC[1].base), SFVAR(AC[1].offset), SFVAR(AC[1].increment),
   SFVAR(AC[2].control), SFVAR(AC[2].base), SFVAR(AC[2].offset), SFVAR(AC[2].increment),
   SFVAR(AC[3].control), SFVAR(AC[3].base), SFVAR(AC[3].offset), SFVAR(AC[3].increment),
   SFVAR(ACShiftBits),
   SFVAR(ACShift),
   SFVAR(AC1ae5),
   SFARRAY(ACRAM, ACRAMUsed ? 0x200000 : 0x0),
   SFEND
  };
  ret &= MDFNSS_StateAction(sm, load, data_only, ACStateRegs, "ACRD");
 } // end if(PCE_ACEnabled)

 if(PCE_IsCD)
 {
  ret &= ADPCM_StateAction(sm, load, data_only);
  ret &= PCECD_StateAction(sm, load, data_only);
 }
 return(ret);
}

void HuCClose(void)
{
 if(IsPopulous)
 {
  if(PopRAM)
  {
   MDFN_DumpToFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), 6, PopRAM, 32768);
   MDFN_free(PopRAM);
   PopRAM = NULL;
  }
 }
 else if(IsTsushin)
 {
  if(TsushinRAM)
  {
   MDFN_DumpToFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), 6, TsushinRAM, 32768);
   MDFN_free(TsushinRAM);
   TsushinRAM = NULL;
  }
 }
 else if(IsBRAMUsed())
 {
  MDFN_DumpToFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), 6, SaveRAM, 2048);
 }

 if(CDRAM)
 {
  MDFN_free(CDRAM);
  CDRAM = NULL;
 }

 if(ACRAM)
 {
  MDFN_free(ACRAM);
  ACRAM = NULL;
 }

 if(PCE_IsCD)
 {
  PCECD_Close();
 }
}

void HuC_Power(void)
{
 if(CDRAM) memset(CDRAM, 0, 262144);
 if(ACRAM) memset(ACRAM, 0, 0x200000);
 ACRAMUsed = 0;
 ACShift = 0;
 ACShiftBits = 0;
 AC1ae5 = 0;
 memset(AC, 0, sizeof(AC));
}
