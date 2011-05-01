/* Mednafen - Multi-system Emulator
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
#include "huc.h"
#include "../cdrom/pcecd.h"
#include "arcade_card/arcade_card.h"
#include "../md5.h"
#include "../file.h"
#include "../cdrom/cdromif.h"
#include "../mempatcher.h"

#include <errno.h>
#include <string.h>

namespace MDFN_IEN_PCE
{

static const char BRAM_Init_String[8] = { 'H', 'U', 'B', 'M', 0x00, 0x88, 0x10, 0x80 }; //"HUBM\x00\x88\x10\x80";
static bool BRAM_Disabled;       // Cached at game load, don't remove this caching behavior or save game loss may result(if we ever get a GUI).

ArcadeCard *arcade_card = NULL;


static uint8 *HuCROM = NULL;
static uint8 *ROMMap[0x100] = { NULL };

static bool IsPopulous;
bool IsTsushin;
bool PCE_IsCD;

static uint8 *TsushinRAM = NULL; // 0x8000
static uint8 *PopRAM = NULL; // 0x8000
static uint8 SaveRAM[2048];
static uint8 *CDRAM = NULL; //262144;

static uint8 *SysCardRAM = NULL;

static void Cleanup(void)
{
 if(HuCROM)
 {
  MDFN_free(HuCROM);
  HuCROM = NULL;
 }

 if(PopRAM)
 {
  MDFN_free(PopRAM);
  PopRAM = NULL;
 }

 if(TsushinRAM)
 {
  MDFN_free(TsushinRAM);
  TsushinRAM = NULL;
 }

 if(CDRAM)
 {
  MDFN_free(CDRAM);
  CDRAM = NULL;
 }

 if(SysCardRAM)
 {
  MDFN_free(SysCardRAM);
  SysCardRAM = NULL;
 }

 if(arcade_card)
 {
  delete arcade_card;
  arcade_card = NULL;
 }
}


static DECLFR(AC_PhysRead)
{
 return(arcade_card->PhysRead(A, PCE_InDebug));
}

static DECLFW(AC_PhysWrite)
{
 return(arcade_card->PhysWrite(A, V));
}

static DECLFW(SysCardRAMWrite)
{
 SysCardRAM[A - 0x68 * 8192] = V;
}

static DECLFR(SysCardRAMRead)
{
 return(SysCardRAM[A - 0x68 * 8192]);
}

static DECLFW(CDRAMWrite)
{
 CDRAM[A - 0x80 * 8192] = V;
}

static DECLFR(CDRAMRead)
{
 return(CDRAM[A - 0x80 * 8192]);
}

static DECLFR(SaveRAMRead)
{
 if(BRAM_Disabled)
  return(0xFF);

 if((!PCE_IsCD || PCECD_IsBRAMEnabled()) && (A & 8191) < 2048)
  return(SaveRAM[A & 2047]);
 else
  return(0xFF);
}

static DECLFW(SaveRAMWrite)
{
 if(BRAM_Disabled)
  return;

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

static uint8 HuCSF2Latch;
static uint8 HuCSF2BankMask;

static DECLFR(HuCSF2ReadLow)
{
 return(HuCROM[A]);
}

static DECLFR(HuCSF2Read)
{
 return(HuCROM[(A & 0x7FFFF) + 0x80000 + (HuCSF2Latch & HuCSF2BankMask) * 0x80000 ]);
}

static DECLFW(HuCSF2Write)
{
 if((A & 0x1FF0) == 0x1FF0)
  HuCSF2Latch = A & 0xF;
}

int HuCLoad(const uint8 *data, uint32 len, uint32 crc32, bool DisableBRAM, SysCardType syscard)
{
 const uint32 sf2_threshold = 2048 * 1024;
 uint32 m_len = (len + 8191) &~ 8191;
 bool sf2_mapper = FALSE;
 bool UseBRAM = FALSE;

 if(!syscard && m_len >= sf2_threshold)
 {
  sf2_mapper = TRUE;

  // Only used the "extended" SF2 mapper if it's considerably larger than the normal SF2 mapper size.
  if(m_len < (512 * 1024 * 6))
   m_len = 512 * 1024 * 5;
  else
   m_len = round_up_pow2(m_len - 512 * 1024) + 512 * 1024;

  if(m_len > 8912896)
  {
   MDFN_PrintError(_("ROM image is too large!"));
   m_len = 8912896;
  }

  HuCSF2BankMask = ((m_len - 512 * 1024) / (512 * 1024)) - 1;

  //printf("%d %d, %02x\n", len, m_len, HuCSF2BankMask);
 }

 IsPopulous = 0;
 PCE_IsCD = 0;

 if(syscard == SYSCARD_NONE)
 {
  md5_context md5;
  md5.starts();
  md5.update(data, len);
  md5.finish(MDFNGameInfo->MD5);

  MDFN_printf(_("ROM:       %dKiB\n"), (len + 1023) / 1024);
  MDFN_printf(_("ROM CRC32: 0x%08x\n"), crc32);
  MDFN_printf(_("ROM MD5:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());
 }

 if(!(HuCROM = (uint8 *)MDFN_malloc(m_len, _("HuCard ROM"))))
 {
  return(0);
 }

 memset(HuCROM, 0xFF, m_len);
 memcpy(HuCROM, data, (m_len < len) ? m_len : len);

 if(m_len == 0x60000)
 {
  for(int x = 0; x < 128; x++)
  {
   ROMMap[x] = &HuCROM[(x & 0x1F) * 8192] - x * 8192;

   HuCPU->SetFastRead(x, ROMMap[x] + x * 8192);
   HuCPU->SetReadHandler(x, HuCRead);
  }

  for(int x = 64; x < 128; x++)
  {
   ROMMap[x] = &HuCROM[((x & 0xF) + 32) * 8192] - x * 8192;

   HuCPU->SetFastRead(x, ROMMap[x] + x * 8192);
   HuCPU->SetReadHandler(x, HuCRead);
  }
 }
 else if(m_len == 0x80000)
 {
  for(int x = 0; x < 64; x++)
  {
   ROMMap[x] = &HuCROM[(x & 0x3F) * 8192] - x * 8192;

   HuCPU->SetFastRead(x, ROMMap[x] + x * 8192);
   HuCPU->SetReadHandler(x, HuCRead);
  }
  for(int x = 64; x < 128; x++)
  {
   ROMMap[x] = &HuCROM[((x & 0x1F) + 32) * 8192] - x * 8192;

   HuCPU->SetFastRead(x, ROMMap[x] + x * 8192);
   HuCPU->SetReadHandler(x, HuCRead);
  }
 }
 else
 {
  for(int x = 0; x < 128; x++)
  {
   uint8 bank = x % (m_len / 8192);
   
   ROMMap[x] = &HuCROM[bank * 8192] - x * 8192;

   HuCPU->SetFastRead(x, ROMMap[x] + x * 8192);
   HuCPU->SetReadHandler(x, HuCRead);
  }
 }

 if(syscard)
 {
  if(syscard == SYSCARD_3 || syscard == SYSCARD_ARCADE)
  {
   if(!(SysCardRAM = (uint8 *)MDFN_calloc(1, 24 * 8192, _("System Card RAM"))))
   {
    Cleanup();
    return(0);
   }

   for(int x = 0x68; x < 0x80; x++)
   {
    ROMMap[x] = &SysCardRAM[(x - 0x68) * 8192] - x * 8192;
    HuCPU->SetFastRead(x, ROMMap[x] + x * 8192);

    HuCPU->SetReadHandler(x, SysCardRAMRead);
    HuCPU->SetWriteHandler(x, SysCardRAMWrite);
   } 
   MDFNMP_AddRAM(24 * 8192, 0x68 * 8192, SysCardRAM); 
  }

  if(syscard == SYSCARD_ARCADE)
  {
   try
   {
    arcade_card = new ArcadeCard();
   }
   catch(std::exception &e)
   {
    MDFN_PrintError(_("Error creating %s object: %s"), "ArcadeCard", e.what());
    Cleanup();
    return(0);
   }

   for(int x = 0x40; x < 0x44; x++)
   {
    ROMMap[x] = NULL;
    HuCPU->SetFastRead(x, NULL);

    HuCPU->SetReadHandler(x, AC_PhysRead);
    HuCPU->SetWriteHandler(x, AC_PhysWrite);
   }
  }

  if(!(CDRAM = (uint8 *)MDFN_calloc(1, 8 * 8192, _("CD RAM"))))
  {
   Cleanup();
   return(0);
  }

  for(int x = 0x80; x < 0x88; x++)
  {
   ROMMap[x] = &CDRAM[(x - 0x80) * 8192] - x * 8192;
   HuCPU->SetFastRead(x, ROMMap[x] + x * 8192);

   HuCPU->SetReadHandler(x, CDRAMRead);
   HuCPU->SetWriteHandler(x, CDRAMWrite);
  }
  MDFNMP_AddRAM(8 * 8192, 0x80 * 8192, CDRAM);

  UseBRAM = TRUE;
 }
 else
 {
  if(!memcmp(HuCROM + 0x1F26, "POPULOUS", strlen("POPULOUS")))
  {
   gzFile fp;
  
   if(!(PopRAM = (uint8 *)MDFN_malloc(32768, _("Populous RAM"))))
   {
    Cleanup();
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
    ROMMap[x] = &PopRAM[(x & 3) * 8192] - x * 8192;
    HuCPU->SetFastRead(x, ROMMap[x] + x * 8192);

    HuCPU->SetReadHandler(x, HuCRead);
    HuCPU->SetWriteHandler(x, HuCRAMWrite);
   }
   MDFNMP_AddRAM(32768, 0x40 * 8192, PopRAM);
  }
  else if(crc32 == 0x34dc65c4) // Tsushin Booster
  {
   gzFile fp;

   if(!(TsushinRAM = (uint8*)MDFN_malloc(0x8000, _("Tsushin Booster RAM"))))
   {
    Cleanup();
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
    ROMMap[x] = &TsushinRAM[(x & 3) * 8192] - x * 8192;
    HuCPU->SetFastRead(x, ROMMap[x] + x * 8192);

    HuCPU->SetReadHandler(x, HuCRead);
    HuCPU->SetWriteHandler(x, HuCRAMWrite);
   }
   MDFNMP_AddRAM(32768, 0x88 * 8192, TsushinRAM);
  }
  else
   UseBRAM = TRUE;

  // 0x1A558
  if(sf2_mapper)
  {
   for(int x = 0x20; x < 0x40; x++)
    HuCPU->SetReadHandler(x, HuCSF2ReadLow);
   for(int x = 0x40; x < 0x80; x++)
   {
    HuCPU->SetFastRead(x, NULL);		// Make sure our reads go through our read function, and not a table lookup
    HuCPU->SetReadHandler(x, HuCSF2Read);
   }
   HuCPU->SetWriteHandler(0, HuCSF2Write);

   MDFN_printf("Street Fighter 2 Mapper\n");
   HuCSF2Latch = 0;
  }
 }	// end else to if(syscard)

 BRAM_Disabled = DisableBRAM;
 if(BRAM_Disabled)
  UseBRAM = false;

 if(UseBRAM)
 {
  gzFile fp;

  memset(SaveRAM, 0x00, 2048);
  memcpy(SaveRAM, BRAM_Init_String, 8);                	// So users don't have to manually intialize the file cabinet
                                                	// in the CD BIOS screen.
  if((fp = gzopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), "rb")))
  {
   gzread(fp, SaveRAM, 2048);
   gzclose(fp);
  }

  HuCPU->SetWriteHandler(0xF7, SaveRAMWrite);
  HuCPU->SetReadHandler(0xF7, SaveRAMRead);
  MDFNMP_AddRAM(2048, 0xF7 * 8192, SaveRAM);
 }

 return(1);
}
 
bool IsBRAMUsed(void)
{
 if(BRAM_Disabled)
  return(false);

 if(memcmp(SaveRAM, BRAM_Init_String, 8)) // HUBM string is modified/missing
  return(1);

 for(int x = 8; x < 2048; x++)
  if(SaveRAM[x]) return(1);

 return(0);
}

int HuC_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] = 
 {
  SFARRAY(PopRAM, IsPopulous ? 32768 : 0),
  SFARRAY(TsushinRAM, IsTsushin ? 32768 : 0),
  SFARRAY(SaveRAM, (IsPopulous || IsTsushin || BRAM_Disabled) ? 0 : 2048),
  SFARRAY(CDRAM, CDRAM ? (8192 * 8) : 0),
  SFARRAY(SysCardRAM, SysCardRAM ? (8192 * 24) : 0),
  SFVAR(HuCSF2Latch),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "HuC");

 if(load)
  HuCSF2Latch &= 0xF;

 if(PCE_IsCD)
 {
  if(arcade_card)
   ret &= arcade_card->StateAction(sm, load, data_only);

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
 else if(!BRAM_Disabled && IsBRAMUsed())
 {
  MDFN_DumpToFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), 6, SaveRAM, 2048);
 }

 Cleanup();
}

void HuC_Power(void)
{
 if(CDRAM) 
  memset(CDRAM, 0x00, 8 * 8192);

 if(SysCardRAM)
  memset(SysCardRAM, 0x00, 24 * 8192);

 if(arcade_card)
  arcade_card->Power();

 HuCSF2Latch = 0;
}



};
