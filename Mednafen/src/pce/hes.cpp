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
#include "hes.h"
#include "huc.h"
#include "cdrom.h"
#include "adpcm.h"
#include "../player.h"
#include "../endian.h"

static uint8 mpr_start[8];
static uint8 IBP[0x10];
static uint8 *rom = NULL, *rom_backup = NULL;

static uint8 CurrentSong;
static bool8 bootstrap;
static bool ROMWriteWarningGiven;

#if 0
static void AllocateBanks(uint32 start, uint32 count)
{
 uint32 start_bank = start / 8192;
 uint32 limit_bank = (start + count + 8191) / 8192;

 if(limit_bank > 0x100) 
 {
  puts("Hrm");
  limit_bank = 0x100;
 }

 for(uint32 b = start_bank; b < limit_bank; b++)
 {
  if(!rom[b])
   rom[b] = MDFN_malloc(8192, _("HES ROM"));
  if(!rom_backup[b])
   rom_backup[b] = MDFN_malloc(8192, _("HES ROM"));
 }
}
#endif

uint8 ReadIBP(unsigned int A)
{
 // Lovely EVIL speed hack.
 if(!PCE_InDebug)
  if((A&0xF) == 0x6)
  {
   int cycrun = HuCPU.count - HuCPU.tcount;
   if(cycrun > 0 && HuCPU.timer_div > 0)
   {
    if(HuCPU.timer_div < cycrun)
     cycrun = HuCPU.timer_div;
    HuC6280_StealCycles(cycrun);
   }
  }

 return(IBP[A & 0xF]);
}

static DECLFW(HESROMWrite)
{
 rom[A] = V;
 //printf("%08x: %02x\n", A, V);
 if(!ROMWriteWarningGiven)
 {
  MDFN_printf(_("Warning:  HES is writing to physical address %08x.  Future warnings of this nature are temporarily disabled for this HES file.\n"), A);
  ROMWriteWarningGiven = TRUE;
 }
}

static DECLFR(HESROMRead)
{
 if(bootstrap)
 {
  if(A == 0x1FFE)
   return(0x00);
  else if(A == 0x1FFF)
  {
   if(!PCE_InDebug)
   {
    PCECD_Power();

    memcpy(rom, rom_backup, 0x88 * 8192);

    HuCPU.A = CurrentSong;
    HuCPU.PC = 0x1C00;

    HuCPUFastMap[0] = rom;

    for(int x=0;x<8;x++)
    {
     HuCPU.MPR[x] = mpr_start[x];
    }
    HuCPU.MPR[8] = HuCPU.MPR[0];
   
    HuC6280_FlushMPRCache();

    bootstrap = 0;
   }
   return(0x1C);
  }
 }
 return(rom[A]);
}

int PCE_HESLoad(uint8 *buf, uint32 size)
{
 uint32 LoadAddr, LoadSize;
 uint32 CurPos;
 uint16 InitAddr;
 uint8 StartingSong;
 int TotalSongs;

 InitAddr = MDFN_de16lsb(&buf[0x6]);

 CurPos = 0x10;
 
 if(!(rom = (uint8 *)MDFN_malloc(0x88 * 8192, _("HES ROM"))))
 {
  return(0);
 }

 if(!(rom_backup = (uint8 *)MDFN_malloc(0x88 * 8192, _("HES ROM"))))
 {
  return(0);
 }

 memset(rom, 0, 0x88 * 8192);
 memset(rom_backup, 0, 0x88 * 8192);

 while(CurPos < (size - 0x10))
 {
  LoadSize = MDFN_de32lsb(&buf[CurPos + 0x4]);
  LoadAddr = MDFN_de32lsb(&buf[CurPos + 0x8]);

  //printf("Size: %08x(%d), Addr: %08x, La: %02x\n", LoadSize, LoadSize, LoadAddr, LoadAddr / 8192);

  CurPos += 0x10;

  if(((uint64)LoadSize + CurPos) > size)
  {
   uint32 NewLoadSize = size - CurPos;

   MDFN_printf(_("Warning:  HES is trying to load more data than is present in the file(%u attempted, %u left)!\n"), LoadSize, NewLoadSize);

   LoadSize = NewLoadSize;
  }

  // 0x88 * 8192 = 0x110000
  if(((uint64)LoadAddr + LoadSize) > 0x110000)
  {
   MDFN_printf(_("Warning:  HES is trying to load data past boundary.\n"));

   if(LoadAddr >= 0x110000)
    break;

   LoadSize = 0x110000 - LoadAddr;
  }

  memcpy(rom + LoadAddr, &buf[CurPos], LoadSize);
  CurPos += LoadSize;
 }

 memcpy(rom_backup, rom, 0x88 * 8192);

 //system_init(hes);
 //system_reset(hes);

 CurrentSong = StartingSong = buf[5];
 TotalSongs = 256;

 IBP[0x00] = 0x20;
 IBP[0x01] = InitAddr;
 IBP[0x02] = InitAddr >> 8;
 IBP[0x03] = 0x58;
 IBP[0x04] = 0x4C;
 IBP[0x05] = 0x04;
 IBP[0x06] = 0x1C;

 Player_Init(TotalSongs, NULL, NULL, NULL, NULL); //UTF8 **snames);

 for(int x = 0; x < 8; x++)
  mpr_start[x] = buf[0x8 + x];

 for(int x = 0; x < 0x80; x++)
 {
  HuCPUFastMap[x] = rom;
  PCERead[x] = HESROMRead;
  PCEWrite[x] = HESROMWrite;
 }

 // FIXME:  If a HES rip tries to execute a SCSI command, the CD emulation code will probably crash.  Obviously, a HES rip shouldn't do this,
 // but Mednafen shouldn't crash either. ;)
 PCE_IsCD = 1;
 PCECD_Init();
 ADPCM_Init();

 ROMWriteWarningGiven = FALSE;

 return(1);
}

void HES_Reset(void)
{
 HuCPUFastMap[0] = NULL;
 HuC6280_FlushMPRCache();
 bootstrap = 1;
}


void HES_Draw(uint32 *pXBuf, int16 *SoundBuf, int32 SoundBufSize)
{
 extern uint16 pce_jp_data[5];
 static uint8 last = 0;
 bool needreload = 0;

 if((pce_jp_data[0] & 0x20) && !(last & 0x20))
 {
  CurrentSong++;
  needreload = 1;
 }

 if((pce_jp_data[0] & 0x80) && !(last & 0x80))
 {
  CurrentSong--;
  needreload = 1;
 }

 if((pce_jp_data[0] & 0x8) && !(last & 0x8))
  needreload = 1;

 if((pce_jp_data[0] & 0x10) && !(last & 0x10))
 {
  CurrentSong += 10;
  needreload = 1;
 }

 if((pce_jp_data[0] & 0x40) && !(last & 0x40))
 {
  CurrentSong -= 10;
  needreload = 1;
 }

 last = pce_jp_data[0];

 if(needreload)
  PCE_Power();

 Player_Draw(pXBuf, CurrentSong, SoundBuf, SoundBufSize);
}

void HES_Close(void)
{
 if(rom)
 {
  MDFN_free(rom);
  rom = NULL;
 }
}
