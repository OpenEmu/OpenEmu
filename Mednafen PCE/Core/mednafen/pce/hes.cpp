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
#include "../cdrom/pcecd.h"
#include "../player.h"

namespace MDFN_IEN_PCE
{

static uint8 mpr_start[8];
static uint8 IBP[0x100];
static uint8 *rom = NULL, *rom_backup = NULL;

static uint8 CurrentSong;
static bool bootstrap;
static bool ROMWriteWarningGiven;

uint8 ReadIBP(unsigned int A)
{
 if(!(A & 0x100))
  return(IBP[A & 0xFF]);

 if(bootstrap && !PCE_InDebug)
 {
  memcpy(rom + 0x1FF0, rom_backup + 0x1FF0, 16);
  bootstrap = false;
  return(CurrentSong);
 }

 return(0xFF);
}

static DECLFW(HESROMWrite)
{
 if(bootstrap)
 {
  puts("Write during bootstrap?");
  return;
 }

 rom[A] = V;
 //printf("%08x: %02x\n", A, V);
 if(!ROMWriteWarningGiven)
 {
  MDFN_printf(_("Warning:  HES is writing to physical address %08x.  Future warnings of this nature are temporarily disabled for this HES file.\n"), A);
  ROMWriteWarningGiven = TRUE;
 }
}

static const uint8 BootROM[16] = { 0xA9, 0xFF, 0x53, 0x01, 0xEA, 0xEA, 0xEA, 0xEA, 
				   0xEA, 0xEA, 0xEA, 0x4C, 0x00, 0x1C, 0xF0, 0xFF };

static DECLFR(HESROMRead)
{
 return(rom[A]);
}

int PCE_HESLoad(const uint8 *buf, uint32 size)
{
 uint32 LoadAddr, LoadSize;
 uint32 CurPos;
 uint16 InitAddr;
 uint8 StartingSong;
 int TotalSongs;

 if(size < 0x10)
 {
  MDFN_PrintError(_("HES header is too small."));
  return(0);
 }

 if(memcmp(buf, "HESM", 4))
 {
  MDFN_PrintError(_("HES header magic is invalid."));
  return(0);
 }

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

 MDFN_printf(_("HES Information:\n"));
 MDFN_indent(1);

 StartingSong = buf[5];

 MDFN_printf(_("Init address: 0x%04x\n"), InitAddr);
 MDFN_printf(_("Starting song: %d\n"), StartingSong + 1);

 for(int x = 0; x < 8; x++)
 {
  mpr_start[x] = buf[0x8 + x];
  MDFN_printf("MPR%d: %02x\n", x, mpr_start[x]);
 }

 memset(rom, 0, 0x88 * 8192);
 memset(rom_backup, 0, 0x88 * 8192);

 while(CurPos < (size - 0x10))
 {
  LoadSize = MDFN_de32lsb(&buf[CurPos + 0x4]);
  LoadAddr = MDFN_de32lsb(&buf[CurPos + 0x8]);

  //printf("Size: %08x(%d), Addr: %08x, La: %02x\n", LoadSize, LoadSize, LoadAddr, LoadAddr / 8192);
  MDFN_printf(_("Chunk load:\n"));
  MDFN_indent(1);
  MDFN_printf(_("File offset:  0x%08x\n"), CurPos);
  MDFN_printf(_("Load size:  0x%08x\n"), LoadSize);
  MDFN_printf(_("Load target address:  0x%08x\n"), LoadAddr);

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
  MDFN_indent(-1);
  memcpy(rom + LoadAddr, &buf[CurPos], LoadSize);
  CurPos += LoadSize;
 }

 memcpy(rom_backup, rom, 0x88 * 8192);

 CurrentSong = StartingSong;
 TotalSongs = 256;
 uint8 *IBP_WR = IBP;

 for(int i = 0; i < 8; i++)
 {
  *IBP_WR++ = 0xA9;		// LDA (immediate)
  *IBP_WR++ = mpr_start[i];
  *IBP_WR++ = 0x53;		// TAM
  *IBP_WR++ = 1 << i;
 }

 // Initialize VDC registers.
 {
  //  uint8 vdc_init_rom[] = { 0xA2, 0x0A, 0xA9, 0xFF, 0x8E, 0x00, 0x00, 0x8D, 0x02, 0x00, 0x8D, 0x03, 0x00, 0xE8, 0xE0, 0x0F, 0xD0, 0xF2 };
  //  uint8 vdc_init_rom[] = { 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEA, 0xEa, 0xEA };

  static const uint8 vdc_init_rom[] = {
	0xA2, 0x1F, 0x8E, 0x00,
	0x00, 0x9C, 0x02, 0x00,
	0x9C, 0x03, 0x00, 0xCA,
	0x10, 0xF4, 0x03, 0x0A,
	0x13, 0x02, 0x23, 0x02,
	0x03, 0x0B, 0x13, 0x1F,
	0x23, 0x04, 0x03, 0x0C,
	0x13, 0x02, 0x23, 0x0D,
	0x03, 0x0D, 0x13, 0xEF,
	0x23, 0x00, 0x03, 0x0E,
	0x13, 0x04, 0xA9, 0x04,
	0x8D, 0x00, 0x04
       };

  memcpy(IBP_WR, vdc_init_rom, sizeof(vdc_init_rom));
  IBP_WR += sizeof(vdc_init_rom);
 }


 *IBP_WR++ = 0xAD;		// LDA(absolute)
 *IBP_WR++ = 0x00;		//
 *IBP_WR++ = 0x1D;		//
 *IBP_WR++ = 0x20;               // JSR
 *IBP_WR++ = InitAddr;           //  JSR target LSB
 *IBP_WR++ = InitAddr >> 8;      //  JSR target MSB
 *IBP_WR++ = 0x58;               // CLI
 *IBP_WR++ = 0xCB;               // (Mednafen Special)
 *IBP_WR++ = 0x80;               // BRA
 *IBP_WR++ = 0xFD;               //  -3

 assert((unsigned int)(IBP_WR - IBP) <= sizeof(IBP));

 Player_Init(TotalSongs, "", "", ""); //NULL, NULL, NULL, NULL); //UTF8 **snames);

 for(int x = 0; x < 0x88; x++)
 {
  if(x)
   HuCPU->SetFastRead(x, rom + x * 8192);
  HuCPU->SetReadHandler(x, HESROMRead);
  HuCPU->SetWriteHandler(x, HESROMWrite);
 }

 ROMWriteWarningGiven = FALSE;

 MDFN_indent(-1);

 return(1);
}

void HES_Reset(void)
{
 memcpy(rom, rom_backup, 0x88 * 8192);
 memcpy(rom + 0x1FF0, BootROM, 16);
 bootstrap = true;
}

void HES_Update(EmulateSpecStruct *espec, uint16 jp_data)
{
 static uint8 last = 0;
 bool needreload = 0;

 if((jp_data & 0x20) && !(last & 0x20))
 {
  CurrentSong++;
  needreload = 1;
 }

 if((jp_data & 0x80) && !(last & 0x80))
 {
  CurrentSong--;
  needreload = 1;
 }

 if((jp_data & 0x8) && !(last & 0x8))
  needreload = 1;

 if((jp_data & 0x10) && !(last & 0x10))
 {
  CurrentSong += 10;
  needreload = 1;
 }

 if((jp_data & 0x40) && !(last & 0x40))
 {
  CurrentSong -= 10;
  needreload = 1;
 }

 last = jp_data;

 if(needreload)
  PCE_Power();

 if(!espec->skip)
  Player_Draw(espec->surface, &espec->DisplayRect, CurrentSong, espec->SoundBuf, espec->SoundBufSize);
}

void HES_Close(void)
{
 if(rom)
 {
  MDFN_free(rom);
  rom = NULL;
 }

 if(rom_backup)
 {
  MDFN_free(rom_backup);
  rom_backup = NULL;
 }
}


};
