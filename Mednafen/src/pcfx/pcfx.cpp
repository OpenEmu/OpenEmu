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

#include "pcfx.h"
#include "vdc.h"
#include "soundbox.h"
#include "input.h"
#include "king.h"
#include "timer.h"
#include "interrupt.h"
#include "debug.h"
#include "rainbow.h"
#include "huc6273.h"
#include "../cdrom/scsicd.h"
#include "../mempatcher.h"
#include "../netplay.h"
#include "../endian.h"
#include "../cdrom/cdromif.h"
#include "../md5.h"

#include <errno.h>
#include <string.h>
#include <math.h>

#ifdef WANT_PCFX_MMAP
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static bool UsingMMAP;
static uint8 *pcfx_mmap_base = NULL;
#endif

//#define FXDBG(format, ...) printf("%lld - FX: " format, (long long)pcfx_timestamp_base + v810_timestamp, ## __VA_ARGS__)
#define FXDBG(format, ...)

int64 pcfx_timestamp_base;

static uint8 *BIOSROM = NULL; // 1MB
static const uint8 *BIOSROMAdjusted = NULL; // BIOSROM - 0xFFF00000
static uint8 *RAM = NULL; // 2MB

static uint16 lastchoo[2];

static bool WantHuC6273 = TRUE;
bool PCFXGA = 0;
static uint32 PCFXGACodeSegmentAddress, PCFXGADataSegmentAddress, PCFXGAGlobalDataSegmentAddress;

//static 
fx_vdc_t *fx_vdc_chips[2];

static uint16 BackupControl;
static uint8 BackupRAM[0x8000], ExBackupRAM[0x8000];

static uint8 ExBusReset; // I/O Register at 0x0700

uint8 mem_rbyte(const uint32 A)
{
 if(A <= 0x001FFFFF)
 {
  return(RAM[A]);
 }
 else if(A >= 0xFFF00000)
 {
  return(BIOSROMAdjusted[A]);
 }
 else if(A >= 0xE0000000 && A <= 0xE7FFFFFF)
 {
  return(BackupRAM[(A & 0xFFFF) >> 1]);
 }
 else if(A >= 0xE8000000 && A <= 0xE9FFFFFF)
 {
  return(ExBackupRAM[(A & 0xFFFF) >> 1]);
 }
 //printf("Unknown byte read: %08x\n", A);
 return(0xFF);
}

uint16 mem_rhword(const uint32 A)
{
 if(A <= 0x001FFFFF)
 {
  return(le16toh(*(uint16*)&RAM[A]));
 }
 else if(A >= 0xFFF00000)
 {
  return(le16toh(*(uint16 *)&BIOSROMAdjusted[A]));
 }
 else if(A >= 0xA0000000 && A <= 0xA3FFFFFF)
 {
  return(port_rhword(0x304));
 }
 else if(A >= 0xA4000000 && A <= 0xA7FFFFFF)
 {
  return(FXVDC_Read16(fx_vdc_chips[0], 0x2));
 }
 else if(A >= 0xA8000000 && A <= 0xABFFFFFF)
 {
  return(FXVDC_Read16(fx_vdc_chips[1], 0x2));
 }
 else if(A >= 0xAC000000 && A <= 0xAFFFFFFF)
 {
  return(KING_Read16(0x604));
 }
 else if(A >= 0xB0000000 && A <= 0xBFFFFFFF) // Write only?
 {
  return(0);
 }
 else if(A >= 0xE0000000 && A <= 0xE7FFFFFF)
 {
  return(BackupRAM[(A & 0xFFFF) >> 1]);
 }
 else if(A >= 0xE8000000 && A <= 0xE9FFFFFF)
 {
  return(ExBackupRAM[(A & 0xFFFF) >> 1]);
 }
 else if(A >= 0xF8000000 && A <= 0xFFEFFFFF) // PIO
 {
  //puts("Err");
  return(0x00);
 }

 //printf("Unknown hword read: %08x\n", A);

 return(0xFFFF);
}

uint32 mem_rword(const uint32 A)
{
 if(A <= 0x001FFFFF)
 {
  return(le32toh(*(uint32*)&RAM[A]));
 }
 else if(A >= 0xFFF00000)
 {
  return(le32toh(*(uint32 *)&BIOSROMAdjusted[A]));
 }
 else
 {
  return(mem_rhword(A) | (mem_rhword(A | 2) << 16));
 }

 //printf("Unknown word read: %08x\n", A);
 return(0xFFFFFFFF);
}

void mem_wbyte(const uint32 A, const uint8 V)
{
 if(A <= 0x001FFFFF)
 {
  RAM[A] = V;
 }
 else if(A >= 0xE0000000 && A <= 0xE7FFFFFF)
 {
  if(BackupControl & 0x1)
  {
   BackupRAM[(A & 0xFFFF) >> 1] = V;
  }
 }
 else if(A >= 0xE8000000 && A <= 0xE9FFFFFF)
 {
  if(BackupControl & 0x2)
  {
   ExBackupRAM[(A & 0xFFFF) >> 1] = V;
  }
 }
 else if(A >= 0xF8000000 && A <= 0xFFEFFFFF)
 {
  // PIO?
 }
 else if(A >= 0x80000000 && A <= 0x80000FFF)
  port_wbyte(A & 0xFFF, V);
}

void mem_whword(const uint32 A, const uint16 V)
{
 if(A <= 0x001FFFFF)
 {
  *(uint16*)&RAM[A] = htole16(V);
 }
 else if(A >= 0xE0000000 && A <= 0xE7FFFFFF)
 {
  if(BackupControl & 0x1)
  {
   BackupRAM[(A & 0xFFFF) >> 1] = (uint8)V;
  }
 }
 else if(A >= 0xE8000000 && A <= 0xE9FFFFFF)
 {
  if(BackupControl & 0x2)
  {
   ExBackupRAM[(A & 0xFFFF) >> 1] = (uint8)V;
  }
 }
 else if(A >= 0xF8000000 && A <= 0xFFEFFFFF)
 {
  // PIO?
 }
 else if(A >= 0xB0000000 && A <= 0xB3FFFFFF)
 {
  port_whword(0x304, V);
 }
 else if(A >= 0xB4000000 && A <= 0xB7FFFFFF)
 {
  FXVDC_Write16(fx_vdc_chips[0], 0x2, V);
 }
 else if(A >= 0xB8000000 && A <= 0xBBFFFFFF)
 {
  FXVDC_Write16(fx_vdc_chips[1], 0x2, V);
 }
 else if(A >= 0xBC000000 && A <= 0xBFFFFFFF)
 {
  KING_Write16(0x604, V);
 }
 else if(A >= 0x80000000 && A <= 0x80FFFFFF)
  port_whword(A & 0xFFFFFF, V);
 else
 {
  //printf("Unknown hword write: %08x %04x\n", A, V);
 }
}

void mem_wword(const uint32 A, const uint32 V)
{
 if(A <= 0x001FFFFF)
 {
  *(uint32*)&RAM[A] = htole32(V);
 }
 else
 {
  mem_whword(A, V);
  mem_whword(A | 2, V >> 16);
 }
}

uint8 port_rbyte(uint32 A)
{
 if(A >= 0x000 && A <= 0x0FF)
  return(FXINPUT_Read8(A));
 else if(A >= 0x300 && A <= 0x3FF) // FXVCE
 {
  return(FXVCE_Read16(A));
 }
 else if(A >= 0x400 && A <= 0x4FF) // VDC-A
 {
  return(FXVDC_Read16(fx_vdc_chips[0], (A & 4) >> 1));
 }
 else if(A >= 0x500 && A <= 0x5FF) // VDC-B
 {
  return(FXVDC_Read16(fx_vdc_chips[1], (A & 4) >> 1));
 }
 else if(A >= 0x600 && A <= 0x6FF)
 {
  return(KING_Read8(A));
 }
 else if(A >= 0x700 && A <= 0x7FF)
 {
  if(!(A & 1))
   return(ExBusReset);

  return(0);
 }
 else if(A >= 0xc00 && A <= 0xCFF) // Backup memory control
 {
  switch(A & 0xC0)
  {
   case 0x80: return(BackupControl);
   case 0x00: return(lastchoo[0]);
   case 0x40: return(lastchoo[1]);
  }
 }
 else if(A >= 0xe00 && A <= 0xeff)
 {
  return(PCFXIRQ_Read8(A));
 }
 else if(A >= 0xf00 && A <= 0xfff)
 {
  return(FXTIMER_Read8(A));
 }
 else if((A & 0x7FFFFFFF) >= 0x500000 && (A & 0x7FFFFFFF) <= 0x52ffff)
 {
  if(WantHuC6273)
   return(HuC6273_Read8(A));
 }

 FXDBG("Unknown 8-bit port read: %08x\n", A);

 return(0x00);
}

uint16 port_rhword(uint32 A)
{
 if(A >= 0x000 && A <= 0x0FF)
  return(FXINPUT_Read16(A));
 else if(A >= 0x300 && A <= 0x3FF)
 {
  return(FXVCE_Read16(A));
 }
 else if(A >= 0x400 && A <= 0x4FF) // VDC-A
 {
  return(FXVDC_Read16(fx_vdc_chips[0], (A & 4) >> 1));
 }
 else if(A >= 0x500 && A <= 0x5FF) // VDC-B
 {
  return(FXVDC_Read16(fx_vdc_chips[1], (A & 4) >> 1));
 }
 else if(A >= 0x600 && A <= 0x6FF)
 {
  return(KING_Read16(A));
 }
 else if(A >= 0x700 && A <= 0x7FF)
 {
  return(ExBusReset);
 }
 else if(A >= 0xc00 && A <= 0xCFF) // Backup memory control
 {
  switch(A & 0xC0)
  {
   case 0x80: return(BackupControl);
   case 0x00: return(lastchoo[0]);
   case 0x40: return(lastchoo[1]);
  }
 }
 else if(A >= 0xe00 && A <= 0xeff)
 {
  return(PCFXIRQ_Read16(A));
 }
 else if(A >= 0xf00 && A <= 0xfff)
 {
  return(FXTIMER_Read16(A));
 }
 else if((A & 0x7FFFFFFF) >= 0x500000 && (A & 0x7FFFFFFF) <= 0x52ffff)
 {
  if(WantHuC6273)
   return(HuC6273_Read16(A));
 }

 FXDBG("Unknown 16-bit port read: %08x\n", A);

 return(0x00);
}

uint32 port_rword(uint32 A)
{
 uint32 ret = port_rhword(A);
 ret |= port_rhword(A | 2) << 16;

 return(ret);
}

void port_wbyte(uint32 A, uint8 V)
{
  if(A >= 0x000 && A <= 0x0FF)
   FXINPUT_Write8(A, V);
  else if(A >= 0x100 && A <= 0x1FF)
  {
   SoundBox_Write(A, V);
  }
  else if(A >= 0x200 && A <= 0x2FF)
  {
   RAINBOW_Write8(A, V);
  }
  else if(A >= 0x300 && A <= 0x3FF) // FXVCE
  {
   FXVCE_Write16(A, V);
  }
  else if(A >= 0x400 && A <= 0x4FF) // VDC-A
  {
   if(!(A & 3)) lastchoo[0] = V;
   FXVDC_Write16(fx_vdc_chips[0], (A & 4) >> 1, V);
  }
  else if(A >= 0x500 && A <= 0x5FF) // VDC-B
  {
   if(!(A & 3)) lastchoo[1] = V;
   FXVDC_Write16(fx_vdc_chips[1], (A & 4) >> 1, V);
  }
  else if(A >= 0x600 && A <= 0x6FF)
  {
   KING_Write8(A, V);
  }
  else if(A >= 0x700 && A <= 0x7FF)
  {
   if(!(A & 1))
    ExBusReset = V & 1;
  }
  else if(A >= 0xc00 && A <= 0xCFF)
  {
   switch(A & 0xC1)
   {
    case 0x80: BackupControl = V & 0x3;
               break;

    default:   FXDBG("Port 8-bit write: %08x %02x\n", A, V);
               break;
   }
  }
  else if((A & 0x7FFFFFFF) >= 0x500000 && (A & 0x7FFFFFFF) <= 0x52ffff)
  {
   if(WantHuC6273)
    HuC6273_Write16(A, V);
  }
  else
   FXDBG("Port 8-bit write: %08x %02x\n", A, V);
}

void port_whword(uint32 A, uint16 V)
{
  if(A >= 0x000 && A <= 0x0FF)
   FXINPUT_Write16(A, V);
  else if(A >= 0x100 && A <= 0x1FF)
  {
   SoundBox_Write(A, V & 0xFF);
   SoundBox_Write(A | 1, V >> 8);
  }
  else if(A >= 0x200 && A <= 0x2FF)
  {
   RAINBOW_Write16(A, V);
  }
  else if(A >= 0x300 && A <= 0x3FF)
  {
   FXVCE_Write16(A, V);
  }
  else if(A >= 0x400 && A <= 0x4FF) // VDC-A
  {
   if(!(A & 4)) lastchoo[0] = V;
   FXVDC_Write16(fx_vdc_chips[0], (A & 4) >> 1, V);
  }
  else if(A >= 0x500 && A <= 0x5FF) // VDC-B
  {
   if(!(A & 4)) lastchoo[1] = V;
   FXVDC_Write16(fx_vdc_chips[1], (A & 4) >> 1, V);
  }
  else if(A >= 0x600 && A <= 0x6FF)
  {
   KING_Write16(A, V);
  }
  else if(A >= 0x700 && A <= 0x7FF)
  {
   ExBusReset = V & 1;
   FXDBG("ExBusReset Write: %04x\n", V);
  }
  else if(A >= 0x800 && A <= 0x8FF) // ?? LIP writes here
  {

  }
  else if(A >= 0xc00 && A <= 0xCFF)
  {
   switch(A & 0xC0)
   {
    case 0x80: BackupControl = V & 0x3; 
	       break;

    default:   FXDBG("Port 16-bit write: %08x %04x\n", A, V);
	       break;
   }
  }
  else if(A >= 0xe00 && A <= 0xeff)
  { 
   PCFXIRQ_Write16(A, V);
  }
  else if(A >= 0xF00 && A <= 0xFFF)
  {
   FXTIMER_Write16(A, V);
  }
  else if((A & 0x7FFFFFFF) >= 0x500000 && (A & 0x7FFFFFFF) <= 0x52ffff)
  {
   if(WantHuC6273)
    HuC6273_Write16(A, V);
  }
  else
  {
   FXDBG("Port 16-bit write: %08x %04x\n", A, V);
  }
}

void port_wword(uint32 A, uint32 V)
{
 port_whword(A, V);
 port_whword(A | 2, V >> 16);
}

static void Emulate(EmulateSpecStruct *espec)
{
 MDFNGameInfo->fb = espec->pixels;
 FXINPUT_Frame();

 MDFNMP_ApplyPeriodicCheats();

 KING_RunFrame(fx_vdc_chips, espec->pixels, espec->LineWidths, espec->skip);
 *(espec->SoundBuf) = SoundBox_Flush(espec->SoundBufSize);
}

static void PCFX_Reset(void)
{
 if(!PCFXGA)
  memset(RAM, 0x00, 2048 * 1024);

 FXVDC_Reset(fx_vdc_chips[0]);
 FXVDC_Reset(fx_vdc_chips[1]);
 KING_Reset();	// SCSICD_Power() is called from KING_Reset()
 SoundBox_Reset();
 RAINBOW_Reset();

 if(WantHuC6273)
  HuC6273_Reset();

 v810_reset();
 pcfx_timestamp_base = 0;

 if(PCFXGA)
 {
  v810_setPC(PCFXGACodeSegmentAddress);

  P_REG[0x3] = 0x1fffd4;		 // Stack pointer
  P_REG[0x4] = PCFXGAGlobalDataSegmentAddress; // Global pointer
  P_REG[0x5] = PCFXGACodeSegmentAddress; // Text pointer
  P_REG[31] = 0xFFFFE134;
  S_REG[PSW] =  0x00000000;
 }
}

static void PCFX_Power(void)
{
 PCFX_Reset();
}

#ifdef WANT_DEBUGGER

static uint8 GAS_SectorCache[2048];
static int32 GAS_SectorCacheWhich = -1;

static void PCFXDBG_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFFFFFF;
   *Buffer = mem_rbyte(Address);
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "ram"))
 {
  while(Length--)
  {
   Address &= 2048 * 1024 - 1;
   *Buffer = RAM[Address];
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "backup"))
 {
  while(Length--)
  {
   Address &= 0x7FFF;
   *Buffer = BackupRAM[Address];
   Address++;
   Buffer++;
  }  
 }
 else if(!strcmp(name, "exbackup"))
 {
  while(Length--)
  {
   Address &= 0x7FFF;
   *Buffer = ExBackupRAM[Address];
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "bios"))
 {
  while(Length--)
  {
   Address &= 1024 * 1024 - 1;
   *Buffer = BIOSROM[Address];
   Address++;
   Buffer++;
  }
 }
 else if(!strncmp(name, "track", strlen("track")))
 {
  int track = atoi(name + strlen("track"));
  int32 sector_base = CDIF_GetTrackStartPositionLBA(track);

  while(Length--)
  {
   int32 sector = (Address / 2048) + sector_base;
   int32 sector_offset = Address % 2048;

   if(sector != GAS_SectorCacheWhich)
   {
    CDIF_ReadSector(GAS_SectorCache, NULL, sector, 1);
    GAS_SectorCacheWhich = sector;
   }

   *Buffer = GAS_SectorCache[sector_offset];

   Address++;
   Buffer++;
  }
 }
}

static void PCFXDBG_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFFFFFF;
   if(Address >= 0xFFF00000 && Address <= 0xFFFFFFFF)
   {
    BIOSROM[Address & 0xFFFFF] = *Buffer;
   }
   else if(Address <= 0x1FFFFF)
   {
    RAM[Address & 0x1FFFFF] = *Buffer;
   }
   else if(Address >= 0xE0000000 && Address <= 0xE7FFFFFF)
   {
    BackupRAM[(Address & 0xFFFF) >> 1] = *Buffer;
   }
   else if(Address >= 0xE8000000 && Address <= 0xE9FFFFFF)
   {
    ExBackupRAM[(Address & 0xFFFF) >> 1] = *Buffer;
   }
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "ram"))
 {
  while(Length--)
  {
   Address &= 2048 * 1024 - 1;
   RAM[Address] = *Buffer;
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "backup"))
 {
  while(Length--)
  {
   Address &= 0x7FFF;
   BackupRAM[Address] = *Buffer;
   Address++;
   Buffer++;
  }  
 }
 else if(!strcmp(name, "exbackup"))
 {
  while(Length--)
  {
   Address &= 0x7FFF;
   ExBackupRAM[Address] = *Buffer;
   Address++;
   Buffer++;
  }
 }

 else if(!strcmp(name, "bios"))
 {
  while(Length--)
  {
   Address &= 1024 * 1024 - 1;
   BIOSROM[Address] = *Buffer;
   Address++;
   Buffer++;
  }
 }

}
#endif

static void SetRegGroups(void);

static bool LoadCommon(void)
{
 FILE *fp;
 std::string biospath = "/pcfx.rom";

 #ifdef WANT_PCFX_MMAP
 UsingMMAP = MDFN_GetSettingB("pcfx.use_mmap");
 #endif

 #ifdef WANT_DEBUGGER
 SetRegGroups();
 #endif

 if(!(fp = fopen(biospath.c_str(), "rb")))
 {
  MDFN_PrintError(_("Could not open BIOS file \"%s\": %s\n"), biospath.c_str(), strerror(errno));
  return(0);
 }

 #ifdef WANT_PCFX_MMAP
 if(UsingMMAP)
 {
  int zero_fd = -1;

  MDFN_printf(_("Using mmap() address space emulation...\n"));

  #ifdef MAP_ANONYMOUS
   #define MDFN_MAP_ANONYMOUS MAP_ANONYMOUS
  #elif defined(MAP_ANON)
   #define MDFN_MAP_ANONYMOUS MAP_ANON
  #else
  #define MDFN_MAP_ANONYMOUS 0
  MDFN_printf(_("Anonymous mmap() not supported?  Then /dev/zero it is!\n"));
  zero_fd = open("/dev/zero", O_RDWR);
  if(zero_fd == -1)
  {
   MDFN_PrintError(_("Error opening \"%s\": %m"), "/dev/zero", errno);
   return(0);
  }
  #endif

  if((void *)-1 == (pcfx_mmap_base = (uint8_t *)mmap(NULL, (size_t)1 << 32, PROT_READ, MAP_PRIVATE | MDFN_MAP_ANONYMOUS, zero_fd, 0)))
  {
   MDFN_PrintError(_("Base mmap failed: %m"), errno);
   pcfx_mmap_base = NULL;
   return(0);
  }

  RAM = (uint8_t *)mmap(pcfx_mmap_base + 0x00000000, 2048 * 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MDFN_MAP_ANONYMOUS | MAP_FIXED, zero_fd, 0);
  if(RAM == (void *)-1)
  {
   MDFN_PrintError(_("RAM mmap failed: %m"), errno);
   RAM = NULL;
   return(0);
  }

  BIOSROM = (uint8_t *)mmap(pcfx_mmap_base + 0xFFF00000, 1024 * 1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MDFN_MAP_ANONYMOUS | MAP_FIXED, zero_fd, 0);
  if(BIOSROM == (void *)-1)
  {
   MDFN_PrintError(("BIOS ROM mmap failed: %m"), errno);
   BIOSROM = NULL;
   return(0);
  }

  if(zero_fd != -1)
   close(zero_fd);

  v810_set_mmap_base(pcfx_mmap_base);
 }
 else
 #endif
 {
  if(!(BIOSROM = (uint8 *)MDFN_malloc(1024 * 1024, _("BIOS ROM"))))  // 1MB
  {
   return(0);
  }
  if(!(RAM = (uint8 *)MDFN_calloc(1, 2048 * 1024, _("RAM")))) // 2MB
  {
   return(0);
  }
 }

 BIOSROMAdjusted = BIOSROM - 0xFFF00000;

 if(fread(BIOSROM, 1024 * 1024, 1, fp) < 1)
 {
  MDFN_PrintError(_("Error reading BIOS file \"%s\": %s\n"), biospath.c_str(), strerror(errno));
  if(BIOSROM)
  {
   MDFN_free(BIOSROM);
   BIOSROM = NULL;
   BIOSROMAdjusted = NULL;
  }
  if(RAM)
  {
   MDFN_free(RAM);
   RAM = NULL;
  }
  return(0);
 }
 fclose(fp);

 #ifdef WANT_DEBUGGER
 MDFNDBG_AddASpace(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, "cpu", "CPU Physical", 32);
 MDFNDBG_AddASpace(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, "ram", "RAM", 21);
 MDFNDBG_AddASpace(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, "backup", "Internal Backup Memory", 15);
 MDFNDBG_AddASpace(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, "exbackup", "External Backup Memory", 15);
 MDFNDBG_AddASpace(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, "bios", "BIOS ROM", 20);
 #endif

 fx_vdc_chips[0] = FXVDC_Init(12, MDFN_GetSettingB("pcfx.nospritelimit"));
 fx_vdc_chips[1] = FXVDC_Init(14, MDFN_GetSettingB("pcfx.nospritelimit"));

 SoundBox_Init();
 RAINBOW_Init();
 FXINPUT_Init();

 if(!KING_Init())
 {
  MDFN_free(BIOSROM);
  MDFN_free(RAM);
  BIOSROM = NULL;
  BIOSROMAdjusted = NULL;
  RAM = NULL;
  return(0);
 }

 #ifdef WANT_DEBUGGER
 for(int32 track = CDIF_GetFirstTrack(); track <= CDIF_GetLastTrack(); track++)
 {
  CDIF_Track_Format format;

  if(CDIF_GetTrackFormat(track, format) && format == CDIF_FORMAT_MODE1)
  {
   char tmpn[256], tmpln[256];
   uint32 sectors;

   snprintf(tmpn, 256, "track%d", track);
   snprintf(tmpln, 256, "CD Track %d", track);

   sectors = CDIF_GetTrackSectorCount(track);
   MDFNDBG_AddASpace(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, tmpn, tmpln, 0, sectors * 2048);
  }
 }
 #endif


 MDFNGameInfo->fps = (uint32)((double)7159090.90909090 / 455 / 263 * 65536 * 256);
 MDFNGameInfo->soundchan = 2;


 MDFNMP_Init(1024 * 1024, ((uint64)1 << 32) / (1024 * 1024));
 MDFNMP_AddRAM(2048 * 1024, 0x00000000, RAM);

 // Initialize backup RAM
 memset(BackupRAM, 0, sizeof(BackupRAM));
 memset(ExBackupRAM, 0, sizeof(ExBackupRAM));

 static const uint8 BRInit00[] = { 0x24, 0x8A, 0xDF, 0x50, 0x43, 0x46, 0x58, 0x53, 0x72, 0x61, 0x6D, 0x80,
                                   0x00, 0x01, 0x01, 0x00, 0x01, 0x40, 0x00, 0x00, 0x01, 0xF9, 0x03, 0x00,
                                   0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
                                  };
 static const uint8 BRInit80[] = { 0xF9, 0xFF, 0xFF };

 memcpy(BackupRAM + 0x00, BRInit00, sizeof(BRInit00));
 memcpy(BackupRAM + 0x80, BRInit80, sizeof(BRInit80));


 static const uint8 ExBRInit00[] = { 0x24, 0x8A, 0xDF, 0x50, 0x43, 0x46, 0x58, 0x43, 0x61, 0x72, 0x64, 0x80,
                                     0x00, 0x01, 0x01, 0x00, 0x01, 0x40, 0x00, 0x00, 0x01, 0xF9, 0x03, 0x00,
                                     0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00
                                  };
 static const uint8 ExBRInit80[] = { 0xF9, 0xFF, 0xFF };

 memcpy(ExBackupRAM + 0x00, ExBRInit00, sizeof(ExBRInit00));
 memcpy(ExBackupRAM + 0x80, ExBRInit80, sizeof(ExBRInit80));

 gzFile savefp;
 if((savefp = gzopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), "rb")))
 {
  gzread(savefp, BackupRAM, 0x8000);
  gzread(savefp, ExBackupRAM, 0x8000);
  gzclose(savefp);
 }

 return(1);
}

typedef struct
{
 const char *name;   // The game name.  DUUUHHHHHHAAAAAAAHAHAHAHA
 const char *MD5Sexy[2]; // MD5 hash string of first two sectors of the first data track
} CDGameEntry;

// Don't change the case of the MD5 strings in this list, as they are directly MD5'd together
// to create the special game set md5 string
static CDGameEntry GameList[] = 
{
 { "Aa! Megami Sama", { "a93ae00cc1c5084ea39c101622d2a06f", "a93ae00cc1c5084ea39c101622d2a06f" } },
 { "All Japan Female Pro Wrestle - Queen of Queens", { "40c3804754a188ae98a34d7a6d6601dc", "c10ad3734b01f3d2c5f0057aae755b22" } },
 { "Anime Freak FX Vol 5", { "048c0781e7aac561edff13b3f4906852", "cf40cc69e529f8b7adf98fb497aea3e8" } },
 { "Anime Freak FX Vol 6", { "9e15355ad93018f2107a9786887681f8", "277faf579939a40787853775a3818193" } },
 { "Last Imperial Prince", { "ac6748228447390c8b250ca843645d9e", "f6b6c9a54ed9aaab387c2c6e5592b240" } },
 { "Voice Paradise", { "fe12dcb4085da97dc87e143b182bc629", "fe12dcb4085da97dc87e143b182bc629" } },
 { "Zoku Hatukoi Monogatari", { "279dab0f1c23f8171ebec938e406b6f6", "279dab0f1c23f8171ebec938e406b6f6" } },
};

static void DoMD5CDVoodoo(void)
{
 bool DTFound = FALSE;
 uint32 track;
 uint8 CDIDMD5[16];

 // Find the first data track
 for(track = CDIF_GetFirstTrack(); track <= CDIF_GetLastTrack(); track++)
 {
  CDIF_Track_Format format;

  if(CDIF_GetTrackFormat(track, format) && format == CDIF_FORMAT_MODE1)
  {
   DTFound = 1;
   break;
  }
 }

 md5_context md5, md5_gameset;

 md5.starts();
 md5_gameset.starts();

 if(DTFound)
 {
  uint32 start_sector = CDIF_GetTrackStartPositionLBA(track);
  uint8 sector_buffer[2048];

  for(int x = 0; x < 128; x++)
  {
   memset(sector_buffer, 0, 2048);
   CDIF_ReadSector(sector_buffer, NULL, start_sector + x, 1);
   md5.update(sector_buffer, 2048);
   if(x < 2)
    md5_gameset.update(sector_buffer, 2048);
  }
 }

 md5.finish(MDFNGameInfo->MD5);
 md5_gameset.finish(CDIDMD5);

 std::string CDIDMD5_String = md5_context::asciistr(CDIDMD5, 0);
 bool GameFound = FALSE;
 
 for(int x = 0; x < sizeof(GameList) / sizeof(CDGameEntry); x++)
 {
  if(!strcasecmp(GameList[x].MD5Sexy[0], CDIDMD5_String.c_str()) || (GameList[x].MD5Sexy[1] && !strcasecmp(GameList[x].MD5Sexy[1], CDIDMD5_String.c_str())   ) )
  {
   MDFNGameInfo->name = (UTF8*)strdup(GameList[x].name);
   if(GameList[x].MD5Sexy[1]) // Only do the GameSet kludge if it's a multi-disc game
   {
    md5_context gid;

    gid.starts();
    gid.update((uint8*)GameList[x].MD5Sexy[0], strlen(GameList[x].MD5Sexy[0]));
    gid.update((uint8*)GameList[x].MD5Sexy[1], strlen(GameList[x].MD5Sexy[1]));

    gid.finish(MDFNGameInfo->GameSetMD5);
    MDFNGameInfo->GameSetMD5Valid = TRUE;
   }
   else
    MDFNGameInfo->GameSetMD5Valid = FALSE;

   GameFound = TRUE;
   break;
  }
 }


 MDFN_printf(_("CD ID MD5(first 4KiB):  0x%s\n"), md5_context::asciistr(CDIDMD5, 0).c_str());

 if(MDFNGameInfo->GameSetMD5Valid)
  MDFN_printf(_("GameSet MD5:	          0x%s\n"), md5_context::asciistr(MDFNGameInfo->GameSetMD5, 0).c_str());

 MDFN_printf(_("CD MD5(first 256KiB):   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());
}


static int LoadCD(void)
{
 DoMD5CDVoodoo();

 if(!LoadCommon())
  return(0);

 MDFN_printf(_("Emulated CD-ROM drive speed: %ux\n"), (unsigned int)MDFN_GetSettingUI("pcfx.cdspeed"));

 MDFNGameInfo->GameType = GMT_CDROM;
 MDFN_LoadGameCheats(0);

 PCFXGA = FALSE;
 PCFX_Power();

 return(1);
}

// HuEXE file reading code based on the information in "PC-FX(GA) loader ver0.8", http://hp.vector.co.jp/authors/VA007898/pcfxga/
static int Load(const char *name, MDFNFILE *fp)
{
 uint8 header[0x40];

 MDFN_fseek(fp, 0, SEEK_SET);

 if(!strcmp(fp->ext, "fxbin"))
 {
  MDFNGameInfo->GameSetMD5Valid = FALSE;
  if(!LoadCommon())
   return(0);

  PCFXGACodeSegmentAddress = 0x8000;
  PCFXGAGlobalDataSegmentAddress = 0x1F8000;
  MDFN_fread(RAM + 0x8000, 1, 2048 * 1024 - 0x8000, fp);
 }
 else
 {
  MDFN_fread(header, 1, 0x40, fp);
  if(memcmp(header, "HuEXE", 5))
  {
   return(-1);
  }

  MDFNGameInfo->GameSetMD5Valid = FALSE;
  if(!LoadCommon())
   return(0);

  uint32 NumSegments;
  uint32 AddressTableOffset;
  uint32 AddressTableSize;
  uint32 SymbolTableSize;
  uint32 SymbolTableOffset;

  NumSegments = MDFN_de32msb(header + 0xC);
  AddressTableOffset = MDFN_de32msb(header + 0x14);
  AddressTableSize = MDFN_de32msb(header + 0x18);
  SymbolTableSize = MDFN_de32msb(header + 0x1C);
  SymbolTableOffset = AddressTableOffset + AddressTableSize;

  printf("%d\n", NumSegments);
  for(int n = 0; n < NumSegments; n++)
  {
   uint8 subheader[0x30];
   uint32 SegmentOffset;
   uint32 SegmentSize;
   uint32 SegmentAddr;
   uint32 SegmentFlag;

   MDFN_fseek(fp, 0x40 + n * 0x30, SEEK_SET);
   MDFN_fread(subheader, 1, 0x30, fp);

   SegmentOffset = MDFN_de32msb(subheader + 0x10);
   SegmentSize = MDFN_de32msb(subheader + 0x14);
   SegmentFlag = MDFN_de32msb(subheader + 0x20);
   SegmentAddr = MDFN_de32msb(subheader + 0x24);

   if(!(SegmentFlag & 0x00000004))
   {
    memcpy(RAM + SegmentAddr, fp->data + SegmentOffset, SegmentSize);
    printf("%.16s %08x %08x %08x %02x %02x\n", subheader, SegmentAddr, SegmentSize, SegmentOffset, RAM[0x8000], *(uint8 *)(fp->data + SegmentOffset));
    if(!strncmp((char*)subheader, "cseg", 16))
     PCFXGACodeSegmentAddress = SegmentAddr;
    if(!strncmp((char *)subheader, "dseg", 16))
     PCFXGADataSegmentAddress = SegmentAddr;
    if(!strncmp((char *)subheader, "gdseg", 16))
     PCFXGAGlobalDataSegmentAddress = SegmentAddr;
   }
   else // MRAM fill?
   {
    printf("Fill: %08x %08x\n", SegmentAddr, SegmentSize);
 //   memset(RAM + SegmentAddr, 0, SegmentSize);
   }
  }
 }

 PCFXGA = TRUE;
 PCFX_Power();
 return(1);
}

static int PCFX_CDInsert(int oride)
{
 if(1)
 {
  if(SCSICD_IsInserted())
  {
   SCSICD_EjectVirtual();
   MDFN_DispMessage(_("Virtual CD Ejected"));
  }
  else
  {
   SCSICD_InsertVirtual();
   MDFN_DispMessage(_("Virtual CD Inserted"));
  }
  return(1);
 }
 else
 {
  return(1);
 }
 return(0);
}

static int PCFX_CDEject(void)
{
 if(SCSICD_IsInserted())
 {
  SCSICD_EjectVirtual();
  MDFN_DispMessage(_("Virtual CD Ejected"));
  return(1);
 }
 return(0);
}

static int PCFX_CDSelect(void)
{
 return(0);
}

static void CloseGame(void)
{
 std::vector<PtrLengthPair> EvilRams;

 EvilRams.push_back(PtrLengthPair(BackupRAM, 0x8000));
 EvilRams.push_back(PtrLengthPair(ExBackupRAM, 0x8000));

 MDFN_DumpToFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), 6, EvilRams);

 for(int i = 0; i < 2; i++)
  if(fx_vdc_chips[i])
  {
   FXVDC_Close(fx_vdc_chips[i]);
   fx_vdc_chips[i] = NULL;
  }

 RAINBOW_Close();
 KING_Close();
 v810_kill();

 MDFN_FlushGameCheats(0);

 #ifdef WANT_PCFX_MMAP
 if(UsingMMAP)
 {
  if(pcfx_mmap_base)
  {
   munmap(pcfx_mmap_base, (size_t)1 << 32);
   pcfx_mmap_base = NULL;
  }
 }
 else
 #endif
 {
  if(BIOSROM)
  {
   MDFN_free(BIOSROM);
   BIOSROM = NULL;
  }

  if(RAM)
  {
   MDFN_free(RAM);
   RAM = NULL;
  }
 }
}

static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
  case MDFNNPCMD_RESET: PCFX_Reset(); break;
  case MDFNNPCMD_POWER: PCFX_Power(); break;
 }
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(RAM, 0x200000),
  SFARRAY16(lastchoo, 2),
  SFVAR(BackupControl),
  SFARRAY(BackupRAM, 0x8000),
  SFARRAY(ExBackupRAM, 0x8000),

  SFVAR(pcfx_timestamp_base),

  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN");

 ret &= FXVDC_StateAction(sm, load, data_only, fx_vdc_chips[0], "VDC0");
 ret &= FXVDC_StateAction(sm, load, data_only, fx_vdc_chips[1], "VDC1");
 ret &= FXINPUT_StateAction(sm, load, data_only);
 ret &= PCFXIRQ_StateAction(sm, load, data_only);
 ret &= KING_StateAction(sm, load, data_only);
 ret &= V810_StateAction(sm, load, data_only);
 ret &= FXTIMER_StateAction(sm, load, data_only);
 ret &= SoundBox_StateAction(sm, load, data_only);
 ret &= SCSICD_StateAction(sm, load, data_only, "CDRM");
 ret &= RAINBOW_StateAction(sm, load, data_only);

 if(load)
 {
  v810_setevent(V810_EVENT_SCSI, SCSICD_Run(v810_timestamp)); // hmm, fixme?
 }

 return(ret);
}

static bool SValidateFunc(const char *name, const char *value)
{
 if(!strcasecmp(name, "pcfx.high_dotclock_width"))
 {
  if(!strcmp(value, "256"))
   return(1);
  if(!strcmp(value, "341"))
   return(1);
  if(!strcmp(value, "1024"))
   return(1);
 }
 return(0);
}

static MDFNSetting PCFXSettings[] =
{
  { "pcfx.input.port1", gettext_noop("Select input device for input port 1."), MDFNST_STRING, "gamepad", NULL, NULL },
  { "pcfx.input.port2", gettext_noop("Select input device for input port 2."), MDFNST_STRING, "gamepad", NULL, NULL },
  { "pcfx.mouse_sensitivity", gettext_noop("Set mouse sensitivity."), MDFNST_FLOAT, "1.25", NULL, NULL },
  { "pcfx.disable_softreset", gettext_noop("If set, when RUN+SEL are pressed simultaneously, disable both buttons temporarily."), MDFNST_BOOL, "0", NULL, NULL, NULL, FXINPUT_SettingChanged },
  { "pcfx.bios", gettext_noop("Path to the ROM BIOS"), MDFNST_STRING, "pcfx.bios PATH NOT SET" },
  { "pcfx.nospritelimit", gettext_noop("No 16-sprites-per-scanline limit option."), MDFNST_BOOL, "0" },
  { "pcfx.high_dotclock_width", gettext_noop("Emulated width for 7.16MHz dot-clock mode: 256, 341, or 1024."), MDFNST_UINT, "1024", NULL, NULL, SValidateFunc },
  { "pcfx.cdspeed", gettext_noop("Emulated CD-ROM speed."), MDFNST_UINT, "2", "1", "10" },
  #ifdef WANT_PCFX_MMAP
  { "pcfx.use_mmap", gettext_noop("Use mmap() to create a virtual 32-bit address space to speed up V810 emulation."), MDFNST_BOOL, "0" },
  #endif
  { NULL }
};

#ifdef WANT_DEBUGGER
static RegType PCFXRegs0[] =
{
        { "PC", "Program Counter", 4 },
	{ "PR1", "Program Register 1", 4 },
	{ "HSP", "Program Register 2(Handler Stack Pointer)", 4 },
	{ "SP", "Program Register 3(Stack Pointer)", 4 },
	{ "GP", "Program Register 4(Global Pointer)", 4 },
	{ "TP", "Program Register 5(Text Pointer)", 4 },
	{ "PR6", "Program Register 6", 4 },
	{ "PR7", "Program Register 7", 4 },
	{ "PR8", "Program Register 8", 4 },
	{ "PR9", "Program Register 9", 4 },
	{ "PR10", "Program Register 10", 4 },
	{ "PR11", "Program Register 11", 4 },
	{ "PR12", "Program Register 12", 4 },
	{ "PR13", "Program Register 13", 4 },
	{ "PR14", "Program Register 14", 4 },
	{ "PR15", "Program Register 15", 4 },
        { "PR16", "Program Register 16", 4 },
        { "PR17", "Program Register 17", 4 },
        { "PR18", "Program Register 18", 4 },
        { "PR19", "Program Register 19", 4 },
        { "PR20", "Program Register 20", 4 },
        { "PR21", "Program Register 21", 4 },
        { "PR22", "Program Register 22", 4 },
        { "PR23", "Program Register 23", 4 },
        { "PR24", "Program Register 24", 4 },
        { "PR25", "Program Register 25", 4 },
        { "PR26", "Program Register 26(String Dest Bit Offset)", 4 },
        { "PR27", "Program Register 27(String Source Bit Offset)", 4 },
        { "PR28", "Program Register 28(String Length)", 4 },
        { "PR29", "Program Register 29(String Dest)", 4 },
        { "PR30", "Program Register 30(String Source)", 4 },
        { "LP", "Program Register 31(Link Pointer)", 4 },
        { "SR0", "Exception/Interrupt PC", 4 },
        { "SR1", "Exception/Interrupt PSW", 4 },
        { "SR2", "Fatal Error PC", 4 },
        { "SR3", "Fatal Error PSW", 4 },
        { "SR4", "Exception Cause Register", 4 },
        { "SR5", "Program Status Word", 4 },
        { "SR6", "Processor ID Register", 4 },
        { "SR7", "Task Control Word", 4 },
        { "SR24", "Cache Control Word", 4 },
        { "SR25", "Address Trap Register", 4 },
	{ "IPEND", "Interrupts Pending", 2 },
        { "IMASK", "Interrupt Mask", 2 },
        { "IPRIO0", "Interrupt Priority Register 0", 2 },
        { "IPRIO1", "Interrupt Priority Register 1", 2 },
        { "TCTRL", "Timer Control", 2 },
        { "TPRD", "Timer Period", 2 },
        { "TCNTR", "Timer Counter", 3 },

	{ "KPCTRL0", "Keyport 0 Control", 1 },
	{ "KPCTRL1", "Keyport 1 Control", 1 },
        { "", "", 0 },
};

static RegType PCFXRegs1[] =
{
	{ "AR", "Active Register", 1 },
	{ "MPROGADDR", "Micro-program Address", 2 },
        { "MPROGCTRL", "Micro-program Control", 2 },

	{ "PAGESET", "KRAM Page Settings", 2 },
        { "RTCTRL", "Rainbow Transfer Control", 2 },
        { "RKRAMA", "Rainbow Transfer K-RAM Address", 3},
        { "RSTART", "Rainbow Transfer Start Position", 2},
        { "RCOUNT", "Rainbow Transfer Block Count", 2 },
        { "RIRQLINE", "Raster IRQ Line", 2 },
	{ "KRAMWA", "K-RAM Write Address", 4 },
	{ "KRAMRA", "K-RAM Read Address", 4 },
        { "DMATA", "DMA Transfer Address", 3 },
        { "DMATS", "DMA Transfer Size", 4 },
        { "DMASTT", "DMA Status", 2 },
        { "ADPCMCTRL", "ADPCM Control", 2 },
        { "ADPCMBM0", "ADPCM Buffer Mode Ch0", 2 },
        { "ADPCMBM1", "ADPCM Buffer Mode Ch1", 2 },
        { "ADPCMPA0", "ADPCM PlayAddress Ch0", 3 },
        { "ADPCMPA1", "ADPCM PlayAddress Ch1", 3 },
        { "ADPCMSA0", "ADPCM Start Address Ch0", 2 },
        { "ADPCMSA1", "ADPCM Start Address Ch1", 2 },
        { "ADPCMIA0", "ADPCM Intermediate Address Ch0", 2 },
        { "ADPCMIA1", "ADPCM Intermediate Address Ch1", 2 },
        { "ADPCMEA0", "ADPCM End Address Ch0", 3 },
        { "ADPCMEA1", "ADPCM End Address Ch1", 3 },
	{ "Reg01", "KING Register 0x01", 1 },
	{ "Reg02", "KING Register 0x02", 1 },
	{ "Reg03", "KING Register 0x03", 1 },
	{ "SUBCC", "Sub-channel Control", 1 },
	{ "------", "", 0xFFFF },
	{ "BSY", "SCSI BSY", 1 },
	{ "REQ", "SCSI REQ", 1 },
	{ "ACK", "SCSI ACK", 1 },
	{ "MSG", "SCSI MSG", 1 },
	{ "IO", "SCSI IO", 1 },
	{ "CD", "SCSI CD", 1 },
	{ "SEL", "SCSI SEL", 1 },

        { "", "", 0 },
};

static RegType PCFXRegs4[] =
{
        { "VCEPRIO0", "VCE Priority 0", 2 },
        { "VCEPRIO1", "VCE Priority 1", 2 },
        { "VCEPICMODE", "VCE Picture Mode", 2},
        { "Frame Cntr", "VCE Frame Counter", 2},
        { "VCEPALRWOF", "VCE Palette R/W Offset", 2 },
        { "VCEPALRWLA", "VCE Palette R/W Latch", 2 },
        { "VCEPALOFS0", "VCE Palette Offset 0", 2 } ,
        { "VCEPALOFS1", "VCE Palette Offset 1", 2 },
        { "VCEPALOFS2", "VCE Palette Offset 2", 2 },
        { "VCEPALOFS3", "VCE Palette Offset 3", 2 },
        { "VCECCR", "VCE Fixed Color Register", 2 },
        { "VCEBLE", "VCE Cellophane Setting Register", 2 },
        { "VCESPBL", "VCE Sprite Cellophane Setting Register", 2 },
        { "VCECOEFF0", "VCE Cellophane Coefficient 0(1A)", 2 },
        { "VCECOEFF1", "VCE Cellophane Coefficient 1(1B)", 2 },
        { "VCECOEFF2", "VCE Cellophane Coefficient 2(2A)", 2 },
        { "VCECOEFF3", "VCE Cellophane Coefficient 3(2B)", 2 },
        { "VCECOEFF4", "VCE Cellophane Coefficient 4(3A)", 2 },
        { "VCECOEFF5", "VCE Cellophane Coefficient 5(3B)", 2 },
        { "ChromaKeyY", "VCE Chroma Key Y", 2 },
        { "ChromaKeyU", "VCE Chroma Key U", 2 },
        { "ChromaKeyV", "VCE Chroma Key V", 2 },

        { "MAWR-A", "Memory Write Address, VDC-A", 2 },
        { "MARR-A", "Memory Read Address, VDC-A", 2 },
	{ "Status-A", "Status, VDC-A", 1 },
        { "CR-A", "Control, VDC-A", 2 },
        { "RCR-A", "Raster Counter, VDC-A", 2 },
        { "BXR-A", "X Scroll, VDC-A", 2 },
        { "BYR-A", "Y Scroll, VDC-A", 2 },
        { "MWR-A", "Memory Width, VDC-A", 2 },

        { "HSR-A", "HSR, VDC-A", 2 },
        { "HDR-A", "HDR, VDC-A", 2 },
        { "VSR-A", "VSR, VDC-A", 2 },
        { "VDR-A", "VDR, VDC-A", 2 },

        { "VCR-A", "VCR, VDC-A", 2 },
        { "DCR-A", "DMA Control, VDC-A", 2 },
        { "SOUR-A", "VRAM DMA Source Address, VDC-A", 2 },
        { "DESR-A", "VRAM DMA Dest Address, VDC-A", 2 },
        { "LENR-A", "VRAM DMA Length, VDC-A", 2 },
        { "SATB-A", "SATB Update Address, VDC-A", 2 },

        { "MAWR-B", "Memory Write Address, VDC-B", 2 },
        { "MARR-B", "Memory Read Address, VDC-B", 2 },
        { "Status-B", "Status, VDC-B", 1 },
        { "CR-B", "Control, VDC-B", 2 },
        { "RCR-B", "Raster Counter, VDC-B", 2 },
        { "BXR-B", "X Scroll, VDC-B", 2 },
        { "BYR-B", "Y Scroll, VDC-B", 2 },
        { "MWR-B", "Memory Width, VDC-B", 2 },

        { "HSR-B", "HSR, VDC-B", 2 },
        { "HDR-B", "HDR, VDC-B", 2 },
        { "VSR-B", "VSR, VDC-B", 2 },
        { "VDR-B", "VDR, VDC-B", 2 },

        { "VCR-B", "VCR, VDC-B", 2 },
        { "DCR-B", "DMA Control, VDC-B", 2 },
        { "SOUR-B", "VRAM DMA Source Address, VDC-B", 2 },
        { "DESR-B", "VRAM DMA Dest Address, VDC-B", 2 },
        { "LENR-B", "VRAM DMA Length, VDC-B", 2 },
        { "SATB-B", "SATB Update Address, VDC-B", 2 },
        { "", "", 0 },
};

static RegGroupType PCFXRegs0Group =
{
 PCFXRegs0,
 PCFXDBG_GetRegister,
 PCFXDBG_SetRegister,
};

static RegGroupType PCFXRegs1Group =
{
 PCFXRegs1,
 KING_GetRegister,
 KING_SetRegister
};


static RegGroupType PCFXRegs4Group =
{
 PCFXRegs4,
 FXVDCVCE_GetRegister,
 FXVDCVCE_SetRegister
};

static void SetRegGroups(void)
{
 MDFNDBG_AddRegGroup(&PCFXRegs0Group);
 MDFNDBG_AddRegGroup(&PCFXRegs1Group);
 MDFNDBG_AddRegGroup(&PCFXRegs4Group);
}

static void ForceIRQ(int level)
{
 v810_int(level);
}

static DebuggerInfoStruct DBGInfo =
{
 4,
 32,
 32,
 0x00000000,
 ~0,

 PCFXDBG_MemPeek,
 PCFXDBG_Disassemble,
 NULL,
 ForceIRQ,
 NULL, //NESDBG_GetVector,
 PCFXDBG_FlushBreakPoints,
 PCFXDBG_AddBreakPoint,
 PCFXDBG_SetCPUCallback,
 PCFXDBG_SetBPCallback,
 PCFXDBG_GetBranchTrace,
 KING_SetGraphicsDecode,
 KING_GetGraphicsDecodeBuffer,
 PCFXDBG_SetLogFunc,
};
#endif

MDFNGI EmulatedPCFX =
{
 "pcfx",
 #ifdef WANT_DEBUGGER
 &DBGInfo,
 #endif
 &PCFXInputInfo,
 Load,
 LoadCD,
 CloseGame,
 KING_ToggleLayer,
 "BG0\0BG1\0BG2\0BG3\0VDC-A BG\0VDC-A SPR\0VDC-B BG\0VDC-B SPR\0RAINBOW\0",
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 KING_SetPixelFormat,
 FXINPUT_SetInput,
 PCFX_CDInsert,
 PCFX_CDEject,
 PCFX_CDSelect,
 SoundBox_SetSoundMultiplier,
 SoundBox_SetSoundVolume,
 SoundBox_Sound,
 DoSimpleCommand,
 PCFXSettings,
 0,
 NULL,
 288,
 240,
 256, // Save state preview width
 1024 * sizeof(uint32),
 { 0, 4, 320, 232 },
};

