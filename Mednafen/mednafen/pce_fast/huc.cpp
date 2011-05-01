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

#include "pce.h"
#include <errno.h>
#include "../cdrom/pcecd.h"
#include "arcade_card/arcade_card.h"
#include "../md5.h"
#include "../file.h"
#include "../cdrom/cdromif.h"
#include "../mempatcher.h"

namespace PCE_Fast
{

static const char BRAM_Init_String[8] = { 'H', 'U', 'B', 'M', 0x00, 0x88, 0x10, 0x80 }; //"HUBM\x00\x88\x10\x80";

ArcadeCard *arcade_card = NULL;

static uint8 *HuCROM = NULL;

static bool IsPopulous;
bool IsTsushin;
bool PCE_IsCD;

static uint8 *TsushinRAM = NULL; // 0x8000
static uint8 SaveRAM[2048];

static DECLFW(ACPhysWrite)
{
 arcade_card->PhysWrite(A, V);
}

static DECLFR(ACPhysRead)
{
 return(arcade_card->PhysRead(A));
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
 return ROMSpace[A];
}

static DECLFW(HuCRAMWrite)
{
 ROMSpace[A] = V;
}

static DECLFW(HuCRAMWriteCDSpecial) // Hyper Dyne Special hack
{
 BaseRAM[0x2000 | (A & 0x1FFF)] = V;
 ROMSpace[A] = V;
}

static uint8 HuCSF2Latch = 0;

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

int HuCLoad(const uint8 *data, uint32 len, uint32 crc32)
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
 MDFN_printf(_("ROM CRC32: 0x%04x\n"), crc32);
 MDFN_printf(_("ROM MD5:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());

 if(!(HuCROM = (uint8 *)MDFN_malloc(m_len, _("HuCard ROM"))))
 {
  return(0);
 }

 memset(HuCROM, 0xFF, m_len);
 memcpy(HuCROM, data, len);

 memset(ROMSpace, 0xFF, 0x88 * 8192 + 8192);

 if(m_len == 0x60000)
 {
  memcpy(ROMSpace + 0x00 * 8192, HuCROM, 0x20 * 8192);
  memcpy(ROMSpace + 0x20 * 8192, HuCROM, 0x20 * 8192);
  memcpy(ROMSpace + 0x40 * 8192, HuCROM + 0x20 * 8192, 0x10 * 8192);
  memcpy(ROMSpace + 0x50 * 8192, HuCROM + 0x20 * 8192, 0x10 * 8192);
  memcpy(ROMSpace + 0x60 * 8192, HuCROM + 0x20 * 8192, 0x10 * 8192);
  memcpy(ROMSpace + 0x70 * 8192, HuCROM + 0x20 * 8192, 0x10 * 8192);
 }
 else if(m_len == 0x80000)
 {
  memcpy(ROMSpace + 0x00 * 8192, HuCROM, 0x40 * 8192);
  memcpy(ROMSpace + 0x40 * 8192, HuCROM + 0x20 * 8192, 0x20 * 8192);
  memcpy(ROMSpace + 0x60 * 8192, HuCROM + 0x20 * 8192, 0x20 * 8192);
 }
 else
 {
  memcpy(ROMSpace + 0x00 * 8192, HuCROM, (m_len < 1024 * 1024) ? m_len : 1024 * 1024);
 }

 for(int x = 0x00; x < 0x80; x++)
 {
  HuCPUFastMap[x] = ROMSpace;
  PCERead[x] = HuCRead;
 }

 if(!memcmp(HuCROM + 0x1F26, "POPULOUS", strlen("POPULOUS")))
 {
  uint8 *PopRAM = ROMSpace + 0x40 * 8192;
  gzFile fp;
  
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
   HuCPUFastMap[x] = &PopRAM[(x & 3) * 8192] - x * 8192;
   PCERead[x] = HuCRead;
   PCEWrite[x] = HuCRAMWrite;
  }
  MDFNMP_AddRAM(32768, 0x40 * 8192, PopRAM);
 }
 else if(crc32 == 0x34dc65c4) // Tsushin Booster
 {
  gzFile fp;

  if(!(TsushinRAM = (uint8*)MDFN_calloc(1, 0x8000 + 8192, _("Tsushin Booster RAM"))))	// + 8192 for PC-as-ptr safety padding
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
   HuCPUFastMap[x] = &TsushinRAM[(x & 3) * 8192] - x * 8192;
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
  for(int x = 0x40; x < 0x80; x++)
  {
   // FIXME: PCE_FAST
   HuCPUFastMap[x] = NULL; // Make sure our reads go through our read function, and not a table lookup
   PCERead[x] = HuCSF2Read;
  }
  PCEWrite[0] = HuCSF2Write;
  MDFN_printf("Street Fighter 2 Mapper\n");
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
 static const FileExtensionSpecStruct KnownBIOSExtensions[] =
 {
  { ".pce", gettext_noop("PC Engine ROM Image") },
  { ".bin", gettext_noop("PC Engine ROM Image") },
  { ".bios", gettext_noop("BIOS Image") },
  { NULL, NULL }
 };

 MDFNFILE fp;

 if(!fp.Open(bios_path, KnownBIOSExtensions, _("CD BIOS")))
 {
  return(0);
 }

 memset(ROMSpace, 0xFF, 262144);

 memcpy(ROMSpace, fp.Data() + (fp.Size() & 512), ((fp.Size() & ~512) > 262144) ? 262144 : (fp.Size() &~ 512) );

 fp.Close();

 PCE_IsCD = 1;
 PCE_InitCD();

 md5_context md5;
 md5.starts();
// md5_update(&md5, HuCROM, 262144);

#if 0
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
 #endif

 MDFN_printf(_("Arcade Card Emulation:  %s\n"), PCE_ACEnabled ? _("Enabled") : _("Disabled")); 
 for(int x = 0; x < 0x40; x++)
 {
  HuCPUFastMap[x] = ROMSpace;
  PCERead[x] = HuCRead;
 }

 for(int x = 0x68; x < 0x88; x++)
 {
  HuCPUFastMap[x] = ROMSpace;
  PCERead[x] = HuCRead;
  PCEWrite[x] = HuCRAMWrite;
 }
 PCEWrite[0x80] = HuCRAMWriteCDSpecial; 	// Hyper Dyne Special hack
 MDFNMP_AddRAM(262144, 0x68 * 8192, ROMSpace + 0x68 * 8192);

 if(PCE_ACEnabled)
 {
   try
   {
    arcade_card = new ArcadeCard();
   }
   catch(std::exception &e)
   {
    MDFN_PrintError(_("Error creating %s object: %s"), "ArcadeCard", e.what());
    //Cleanup();	// TODO
    return(0);
   }

  for(int x = 0x40; x < 0x44; x++)
  {
   HuCPUFastMap[x] = NULL;
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
 return(1);
}

int HuC_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] = 
 {
  SFARRAY(ROMSpace + 0x40 * 8192, IsPopulous ? 32768 : 0),
  SFARRAY(TsushinRAM, IsTsushin ? 32768 : 0),
  SFARRAY(SaveRAM, (IsPopulous || IsTsushin) ? 0 : 2048),
  SFARRAY(ROMSpace + 0x68 * 8192, PCE_IsCD ? 262144 : 0),
  SFVAR(HuCSF2Latch),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "HuC");

 if(load)
  HuCSF2Latch &= 0x3;

 if(PCE_IsCD)
 {
  ret &= PCECD_StateAction(sm, load, data_only);

  if(arcade_card)
   ret &= arcade_card->StateAction(sm, load, data_only);
 }
 return(ret);
}

void HuCClose(void)
{
 if(IsPopulous)
 {
  MDFN_DumpToFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), 6, ROMSpace + 0x40 * 8192, 32768);
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

 if(arcade_card)
 {
  delete arcade_card;
  arcade_card = NULL;
 }

 if(PCE_IsCD)
 {
  PCECD_Close();
 }

 if(HuCROM)
 {
  MDFN_free(HuCROM);
  HuCROM = NULL;
 }
}

void HuC_Power(void)
{
 if(PCE_IsCD)
  memset(ROMSpace + 0x68 * 8192, 0x00, 262144);

 if(arcade_card)
  arcade_card->Power();
}

};
