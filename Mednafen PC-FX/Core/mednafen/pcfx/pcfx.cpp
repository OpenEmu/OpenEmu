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
#include "soundbox.h"
#include "input.h"
#include "king.h"
#include "timer.h"
#include "interrupt.h"
#include "debug.h"
#include "rainbow.h"
#include "huc6273.h"
#include "fxscsi.h"
#include "../cdrom/scsicd.h"
#include "../mempatcher.h"
#include "../cdrom/cdromif.h"
#include "../md5.h"
#include "../clamp.h"

#include <trio/trio.h>
#include <errno.h>
#include <string.h>
#include <math.h>

/* FIXME:  soundbox, vce, vdc, rainbow, and king store wait states should be 4, not 2, but V810 has write buffers which can mask wait state penalties.
  This is a hack to somewhat address the issue, but to really fix it, we need to handle write buffer emulation in the V810 emulation core itself.
*/

static std::vector<CDIF*> *cdifs = NULL;
static bool CD_TrayOpen;
static int CD_SelectedDisc;	// -1 for no disc

V810 PCFX_V810;

int64 pcfx_timestamp_base;

static uint8 *BIOSROM = NULL; 	// 1MB
static uint8 *RAM = NULL; 	// 2MB
static uint8 *FXSCSIROM = NULL;	// 512KiB

static uint32 RAM_LPA;		// Last page access

static const int RAM_PageSize = 2048;
static const int RAM_PageNOTMask = ~(RAM_PageSize - 1);

static uint16 Last_VDC_AR[2];

static bool WantHuC6273 = FALSE;

//static 
VDC *fx_vdc_chips[2];

static uint16 BackupControl;
static uint8 BackupRAM[0x8000], ExBackupRAM[0x8000];
static uint8 ExBusReset; // I/O Register at 0x0700

static bool BRAMDisabled;	// Cached at game load, don't remove this caching behavior or save game loss may result(if we ever get a GUI).

// Checks to see if this main-RAM-area access
// is in the same DRAM page as the last access.
#define RAMLPCHECK	\
{					\
  if((A & RAM_PageNOTMask) != RAM_LPA)	\
  {					\
   timestamp += 3;			\
   RAM_LPA = A & RAM_PageNOTMask;	\
  }					\
}

static v810_timestamp_t next_pad_ts, next_timer_ts, next_adpcm_ts, next_king_ts;

void PCFX_FixNonEvents(void)
{
 if(next_pad_ts & 0x40000000)
  next_pad_ts = PCFX_EVENT_NONONO;

 if(next_timer_ts & 0x40000000)
  next_timer_ts = PCFX_EVENT_NONONO;

 if(next_adpcm_ts & 0x40000000)
  next_adpcm_ts = PCFX_EVENT_NONONO;

 if(next_king_ts & 0x40000000)
  next_king_ts = PCFX_EVENT_NONONO;
}

void PCFX_Event_Reset(void)
{
 next_pad_ts = PCFX_EVENT_NONONO;
 next_timer_ts = PCFX_EVENT_NONONO;
 next_adpcm_ts = PCFX_EVENT_NONONO;
 next_king_ts = PCFX_EVENT_NONONO;
}

static INLINE uint32 CalcNextTS(void)
{
 v810_timestamp_t next_timestamp = next_king_ts;

 if(next_timestamp > next_pad_ts)
  next_timestamp  = next_pad_ts;

 if(next_timestamp > next_timer_ts)
  next_timestamp = next_timer_ts;

 if(next_timestamp > next_adpcm_ts)
  next_timestamp = next_adpcm_ts;

 return(next_timestamp);
}

static void RebaseTS(const v810_timestamp_t timestamp)
{
 assert(next_pad_ts > timestamp);
 assert(next_timer_ts > timestamp);
 assert(next_adpcm_ts > timestamp);
 assert(next_king_ts > timestamp);

 next_pad_ts -= timestamp;
 next_timer_ts -= timestamp;
 next_adpcm_ts -= timestamp;
 next_king_ts -= timestamp;
}


void PCFX_SetEvent(const int type, const v810_timestamp_t next_timestamp)
{
 //assert(next_timestamp > PCFX_V810.v810_timestamp);

 if(type == PCFX_EVENT_PAD)
  next_pad_ts = next_timestamp;
 else if(type == PCFX_EVENT_TIMER)
  next_timer_ts = next_timestamp;
 else if(type == PCFX_EVENT_ADPCM)
  next_adpcm_ts = next_timestamp;
 else if(type == PCFX_EVENT_KING)
  next_king_ts = next_timestamp;

 if(next_timestamp < PCFX_V810.GetEventNT())
  PCFX_V810.SetEventNT(next_timestamp);
}

int32 MDFN_FASTCALL pcfx_event_handler(const v810_timestamp_t timestamp)
{
     if(timestamp >= next_king_ts)
      next_king_ts = KING_Update(timestamp);

     if(timestamp >= next_pad_ts)
      next_pad_ts = FXINPUT_Update(timestamp);

     if(timestamp >= next_timer_ts)
      next_timer_ts = FXTIMER_Update(timestamp);

     if(timestamp >= next_adpcm_ts)
      next_adpcm_ts = SoundBox_ADPCMUpdate(timestamp);

#if 1
     assert(next_king_ts > timestamp);
     assert(next_pad_ts > timestamp);
     assert(next_timer_ts > timestamp);
     assert(next_adpcm_ts > timestamp);
#endif
     return(CalcNextTS());
}

static void ForceEventUpdates(const uint32 timestamp)
{
 next_king_ts = KING_Update(timestamp);
 next_pad_ts = FXINPUT_Update(timestamp);
 next_timer_ts = FXTIMER_Update(timestamp);
 next_adpcm_ts = SoundBox_ADPCMUpdate(timestamp);

 //printf("Meow: %d\n", CalcNextTS());
 PCFX_V810.SetEventNT(CalcNextTS());
}

#include "io-handler.inc"
#include "mem-handler.inc"

typedef struct
{
 int8 tracknum;
 int8 format;
 uint32 lba;
} CDGameEntryTrack;

typedef struct
{
 const char *name;
 const char *name_original;     // Original non-Romanized text.
 const uint32 flags;            // Emulation flags.
 const unsigned int discs;      // Number of discs for this game.
 CDGameEntryTrack tracks[2][100]; // 99 tracks and 1 leadout track
} CDGameEntry;

#define CDGE_FORMAT_AUDIO		0
#define CDGE_FORMAT_DATA		1

#define CDGE_FLAG_ACCURATE_V810         0x01
#define CDGE_FLAG_FXGA			0x02

static uint32 EmuFlags;

static CDGameEntry GameList[] =
{
 #include "gamedb.inc"
};


static void Emulate(EmulateSpecStruct *espec)
{
 v810_timestamp_t v810_timestamp;

 FXINPUT_Frame();

 MDFNMP_ApplyPeriodicCheats();

 if(espec->VideoFormatChanged)
  KING_SetPixelFormat(espec->surface->format); //.Rshift, espec->surface->format.Gshift, espec->surface->format.Bshift);

 if(espec->SoundFormatChanged)
  SoundBox_SetSoundRate(espec->SoundRate);


 KING_StartFrame(fx_vdc_chips, espec);	//espec->surface, &espec->DisplayRect, espec->LineWidths, espec->skip);

 v810_timestamp = PCFX_V810.Run(pcfx_event_handler);


 PCFX_FixNonEvents();

 // Call before resetting v810_timestamp to 0
 ForceEventUpdates(v810_timestamp);

 espec->SoundBufSize = SoundBox_Flush(v810_timestamp, espec->SoundBuf, espec->SoundBufMaxSize);

 KING_EndFrame(v810_timestamp);
 FXTIMER_ResetTS();
 FXINPUT_ResetTS();
 SoundBox_ResetTS();

 // Call this AFTER all the EndFrame/Flush/ResetTS stuff
 RebaseTS(v810_timestamp);

 espec->MasterCycles = v810_timestamp;

 pcfx_timestamp_base += v810_timestamp;

 PCFX_V810.ResetTS();
}

static void PCFX_Reset(void)
{
 PCFX_Event_Reset();

 RAM_LPA = 0;

 ExBusReset = 0;
 BackupControl = 0;

 Last_VDC_AR[0] = 0;
 Last_VDC_AR[1] = 0;

 memset(RAM, 0x00, 2048 * 1024);

 for(int i = 0; i < 2; i++)
 {
  int32 dummy_ne;

  dummy_ne = fx_vdc_chips[i]->Reset();
 }

 KING_Reset();	// SCSICD_Power() is called from KING_Reset()
 SoundBox_Reset();
 RAINBOW_Reset();

 if(WantHuC6273)
  HuC6273_Reset();

 PCFXIRQ_Reset();
 FXTIMER_Reset();
 PCFX_V810.Reset();
 pcfx_timestamp_base = 0;

 ForceEventUpdates(0);
}

static void PCFX_Power(void)
{
 PCFX_Reset();
}

#ifdef WANT_DEBUGGER

static uint8 GAS_SectorCache[2048];
static int32 GAS_SectorCacheWhich = -1;	// disc num is |'d in after << 24

static void PCFXDBG_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 int32 ws = 0;

 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFFFFFF;
   *Buffer = mem_rbyte(ws, Address);
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
  int disc = 0, track = 0, sector_base = 0;
  
  trio_sscanf(name, "track%d-%d-%d", &disc, &track, &sector_base);

  while(Length--)
  {
   int32 sector = (Address / 2048) + sector_base;
   int32 sector_offset = Address % 2048;

   if((sector | (disc << 24)) != GAS_SectorCacheWhich)
   {
    if(!(*cdifs)[disc]->ReadSector(GAS_SectorCache, sector, 1))
     memset(GAS_SectorCache, 0, 2048);

    GAS_SectorCacheWhich = sector | (disc << 24);
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
    if(!(Address & 1))
     BackupRAM[(Address & 0xFFFF) >> 1] = *Buffer;
   }
   else if(Address >= 0xE8000000 && Address <= 0xE9FFFFFF)
   {
    if(!(Address & 1))
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

static void VDCA_IRQHook(bool asserted)
{
 PCFXIRQ_Assert(PCFXIRQ_SOURCE_VDCA, asserted);
}

static void VDCB_IRQHook(bool asserted)
{
 PCFXIRQ_Assert(PCFXIRQ_SOURCE_VDCB, asserted);
}

static void SetRegGroups(void);

static bool LoadCommon(std::vector<CDIF *> *CDInterfaces)
{
 std::string biospath = MDFN_MakeFName(MDFNMKF_FIRMWARE, 0, MDFN_GetSettingS("pcfx.bios").c_str());
 std::string fxscsi_path = MDFN_GetSettingS("pcfx.fxscsi");	// For developers only, so don't make it convenient.
 MDFNFILE BIOSFile;
 V810_Emu_Mode cpu_mode;

 #ifdef WANT_DEBUGGER
 SetRegGroups();
 #endif

 if(!BIOSFile.Open(biospath, NULL, "BIOS"))
  return(0);

 cpu_mode = (V810_Emu_Mode)MDFN_GetSettingI("pcfx.cpu_emulation");
 if(cpu_mode == _V810_EMU_MODE_COUNT)
 {
  cpu_mode = (EmuFlags & CDGE_FLAG_ACCURATE_V810) ? V810_EMU_MODE_ACCURATE : V810_EMU_MODE_FAST;
 }

 if(EmuFlags & CDGE_FLAG_FXGA)
 {
  //WantHuC6273 = TRUE;
 }

 MDFN_printf(_("V810 Emulation Mode: %s\n"), (cpu_mode == V810_EMU_MODE_ACCURATE) ? _("Accurate") : _("Fast"));
 PCFX_V810.Init(cpu_mode, false);

 uint32 RAM_Map_Addresses[1] = { 0x00000000 };
 uint32 BIOSROM_Map_Addresses[1] = { 0xFFF00000 };

 // todo: cleanup on error
 if(!(RAM = PCFX_V810.SetFastMap(RAM_Map_Addresses, 0x00200000, 1, _("RAM"))))
 {
  return(0);
 }

 if(!(BIOSROM = PCFX_V810.SetFastMap(BIOSROM_Map_Addresses, 0x00100000, 1, _("BIOS ROM"))))
 {
  return(0);
 }

 if(BIOSFile.Size() != 1024 * 1024)
 {
  MDFN_PrintError(_("BIOS ROM file is incorrect size.\n"));
  return(0);
 }

 memcpy(BIOSROM, BIOSFile.Data(), 1024 * 1024);

 BIOSFile.Close();

 if(fxscsi_path != "0" && fxscsi_path != "" && fxscsi_path != "none")
 {
  MDFNFILE FXSCSIFile;

  if(!FXSCSIFile.Open(fxscsi_path, NULL, "FX-SCSI ROM"))
   return(0);

  if(FXSCSIFile.Size() != 1024 * 512)
  {
   MDFN_PrintError(_("BIOS ROM file is incorrect size.\n"));
   return(0);
  }

  uint32 FXSCSI_Map_Addresses[1] = { 0x80780000 };

  if(!(FXSCSIROM = PCFX_V810.SetFastMap(FXSCSI_Map_Addresses, 0x0080000, 1, _("FX-SCSI ROM"))))
  {
   return(0);
  }

  memcpy(FXSCSIROM, FXSCSIFile.Data(), 1024 * 512);

  FXSCSIFile.Close();
 }

 #ifdef WANT_DEBUGGER
 ASpace_Add(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, "cpu", "CPU Physical", 32);
 ASpace_Add(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, "ram", "RAM", 21);
 ASpace_Add(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, "backup", "Internal Backup Memory", 15);
 ASpace_Add(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, "exbackup", "External Backup Memory", 15);
 ASpace_Add(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, "bios", "BIOS ROM", 20);
 #endif

 for(int i = 0; i < 2; i++)
 {
  fx_vdc_chips[i] = new VDC(MDFN_GetSettingB("pcfx.nospritelimit"), 65536);
  fx_vdc_chips[i]->SetWSHook(NULL);
  fx_vdc_chips[i]->SetIRQHook(i ? VDCB_IRQHook : VDCA_IRQHook);

  //fx_vdc_chips[0] = FXVDC_Init(PCFXIRQ_SOURCE_VDCA, MDFN_GetSettingB("pcfx.nospritelimit"));
  //fx_vdc_chips[1] = FXVDC_Init(PCFXIRQ_SOURCE_VDCB, MDFN_GetSettingB("pcfx.nospritelimit"));
 }

 SoundBox_Init(MDFN_GetSettingB("pcfx.adpcm.emulate_buggy_codec"), MDFN_GetSettingB("pcfx.adpcm.suppress_channel_reset_clicks"));
 RAINBOW_Init(MDFN_GetSettingB("pcfx.rainbow.chromaip"));
 FXINPUT_Init();

 if(WantHuC6273)
  HuC6273_Init();

 if(!KING_Init())
 {
  MDFN_free(BIOSROM);
  MDFN_free(RAM);
  BIOSROM = NULL;
  RAM = NULL;
  return(0);
 }

 CD_TrayOpen = false;
 CD_SelectedDisc = 0;

 SCSICD_SetDisc(true, NULL, true);
 SCSICD_SetDisc(false, (*CDInterfaces)[0], true);


 #ifdef WANT_DEBUGGER
 for(unsigned disc = 0; disc < CDInterfaces->size(); disc++)
 {
  CDUtility::TOC toc;

  (*CDInterfaces)[disc]->ReadTOC(&toc);

  for(int32 track = toc.first_track; track <= toc.last_track; track++)
  {
   if(toc.tracks[track].control & 0x4)
   {
    char tmpn[256], tmpln[256];
    uint32 sectors;

    trio_snprintf(tmpn, 256, "track%d-%d-%d", disc, track, toc.tracks[track].lba);
    trio_snprintf(tmpln, 256, "CD - Disc %d/%d - Track %d/%d", disc + 1, CDInterfaces->size(), track, toc.last_track - toc.first_track + 1);

    sectors = toc.tracks[track + 1].lba - toc.tracks[track].lba;
    ASpace_Add(PCFXDBG_GetAddressSpaceBytes, PCFXDBG_PutAddressSpaceBytes, tmpn, tmpln, 0, sectors * 2048);
   }
  }
 }
 #endif


 MDFNGameInfo->fps = (uint32)((double)7159090.90909090 / 455 / 263 * 65536 * 256);

 MDFNGameInfo->nominal_height = MDFN_GetSettingUI("pcfx.slend") - MDFN_GetSettingUI("pcfx.slstart") + 1;

 // Emulation raw framebuffer image should always be of 256 width when the pcfx.high_dotclock_width setting is set to "256",
 // but it could be either 256 or 341 when the setting is set to "341", so stay with 1024 in that case so we won't have
 // a messed up aspect ratio in our recorded QuickTime movies.
 MDFNGameInfo->lcm_width = (MDFN_GetSettingUI("pcfx.high_dotclock_width") == 256) ? 256 : 1024;
 MDFNGameInfo->lcm_height = MDFNGameInfo->nominal_height;

 MDFNMP_Init(1024 * 1024, ((uint64)1 << 32) / (1024 * 1024));
 MDFNMP_AddRAM(2048 * 1024, 0x00000000, RAM);


 BRAMDisabled = MDFN_GetSettingB("pcfx.disable_bram");

 if(BRAMDisabled)
  MDFN_printf(_("Warning: BRAM is disabled per pcfx.disable_bram setting.  This is simulating a malfunction.\n"));

 if(!BRAMDisabled)
 {
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
 }

 // Default to 16-bit bus.
 for(int i = 0; i < 256; i++)
 {
  PCFX_V810.SetMemReadBus32(i, FALSE);
  PCFX_V810.SetMemWriteBus32(i, FALSE);
 }

 // 16MiB RAM area.
 PCFX_V810.SetMemReadBus32(0, TRUE);
 PCFX_V810.SetMemWriteBus32(0, TRUE);

 // Bitstring read range
 for(int i = 0xA0; i <= 0xAF; i++)
 {
  PCFX_V810.SetMemReadBus32(i, FALSE);       // Reads to the read range are 16-bit, and
  PCFX_V810.SetMemWriteBus32(i, TRUE);       // writes are 32-bit.
 }

 // Bitstring write range
 for(int i = 0xB0; i <= 0xBF; i++)
 {
  PCFX_V810.SetMemReadBus32(i, TRUE);	// Reads to the write range are 32-bit,
  PCFX_V810.SetMemWriteBus32(i, FALSE);	// but writes are 16-bit!
 }

 // BIOS area
 for(int i = 0xF0; i <= 0xFF; i++)
 {
  PCFX_V810.SetMemReadBus32(i, FALSE);
  PCFX_V810.SetMemWriteBus32(i, FALSE);
 }

 PCFX_V810.SetMemReadHandlers(mem_rbyte, mem_rhword, mem_rword);
 PCFX_V810.SetMemWriteHandlers(mem_wbyte, mem_whword, mem_wword);

 PCFX_V810.SetIOReadHandlers(port_rbyte, port_rhword, NULL);
 PCFX_V810.SetIOWriteHandlers(port_wbyte, port_whword, NULL);



 return(1);
}

static void DoMD5CDVoodoo(std::vector<CDIF *> *CDInterfaces)
{
 const CDGameEntry *found_entry = NULL;
 CDUtility::TOC toc;

 #if 0
 puts("{");
 puts(" ,");
 puts(" ,");
 puts(" 0,");
 puts(" 1,");
 puts(" {");
 puts("  {");

 for(int i = CDIF_GetFirstTrack(); i <= CDIF_GetLastTrack(); i++)
 {
  CDIF_Track_Format tf;

  CDIF_GetTrackFormat(i, tf);
  
  printf("   { %d, %s, %d },\n", i, (tf == CDIF_FORMAT_AUDIO) ? "CDIF_FORMAT_AUDIO" : "CDIF_FORMAT_MODE1", CDIF_GetTrackStartPositionLBA(i));
 }
 printf("   { -1, (CDIF_Track_Format)-1, %d },\n", CDIF_GetSectorCountLBA());
 puts("  }");
 puts(" }");
 puts("},");
 //exit(1);
 #endif

 for(unsigned if_disc = 0; if_disc < CDInterfaces->size(); if_disc++)
 {
  (*CDInterfaces)[if_disc]->ReadTOC(&toc);

  if(toc.first_track == 1)
  {
   for(unsigned int g = 0; g < sizeof(GameList) / sizeof(CDGameEntry); g++)
   {
    const CDGameEntry *entry = &GameList[g];

    assert(entry->discs == 1 || entry->discs == 2);

    for(unsigned int disc = 0; disc < entry->discs; disc++)
    {
     const CDGameEntryTrack *et = entry->tracks[disc];
     bool GameFound = TRUE;

     while(et->tracknum != -1 && GameFound)
     {
      assert(et->tracknum > 0 && et->tracknum < 100);

      if(toc.tracks[et->tracknum].lba != et->lba)
       GameFound = FALSE;

      if( ((et->format == CDGE_FORMAT_DATA) ? 0x4 : 0x0) != (toc.tracks[et->tracknum].control & 0x4))
       GameFound = FALSE;

      et++;
     }

     if(et->tracknum == -1)
     {
      if((et - 1)->tracknum != toc.last_track)
       GameFound = FALSE;
 
      if(et->lba != toc.tracks[100].lba)
       GameFound = FALSE;
     }

     if(GameFound)
     {
      found_entry = entry;
      goto FoundIt;
     }
    } // End disc count loop
   }
  }

  FoundIt: ;

  if(found_entry)
  {
   EmuFlags = found_entry->flags;

   if(found_entry->discs > 1)
   {
    const char *hash_prefix = "Mednafen PC-FX Multi-Game Set";
    md5_context md5_gameset;

    md5_gameset.starts();

    md5_gameset.update_string(hash_prefix);

    for(unsigned int disc = 0; disc < found_entry->discs; disc++)
    {
     const CDGameEntryTrack *et = found_entry->tracks[disc];

     while(et->tracknum)
     {
      md5_gameset.update_u32_as_lsb(et->tracknum);
      md5_gameset.update_u32_as_lsb((uint32)et->format);
      md5_gameset.update_u32_as_lsb(et->lba);

      if(et->tracknum == -1)
       break;
      et++;
     }
    }
    md5_gameset.finish(MDFNGameInfo->GameSetMD5);
    MDFNGameInfo->GameSetMD5Valid = TRUE;
   }
   //printf("%s\n", found_entry->name);
   MDFNGameInfo->name = (UTF8*)strdup(found_entry->name);
   break;
  }
 } // end: for(unsigned if_disc = 0; if_disc < CDInterfaces->size(); if_disc++)

 MDFN_printf(_("CD Layout MD5:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());

 if(MDFNGameInfo->GameSetMD5Valid)
  MDFN_printf(_("GameSet MD5:     0x%s\n"), md5_context::asciistr(MDFNGameInfo->GameSetMD5, 0).c_str());
}

// PC-FX BIOS will look at all data tracks(not just the first one), in contrast to the PCE CD BIOS, which only looks
// at the first data track.
static bool TestMagicCD(std::vector<CDIF *> *CDInterfaces)
{
 CDIF *cdiface = (*CDInterfaces)[0];
 CDUtility::TOC toc;
 uint8 sector_buffer[2048];

 memset(sector_buffer, 0, sizeof(sector_buffer));

 cdiface->ReadTOC(&toc);

 for(int32 track = toc.first_track; track <= toc.last_track; track++)
 {
  if(toc.tracks[track].control & 0x4)
  {
   cdiface->ReadSector(sector_buffer, toc.tracks[track].lba, 1);
   if(!strncmp("PC-FX:Hu_CD-ROM", (char*)sector_buffer, strlen("PC-FX:Hu_CD-ROM")))
   {
    return(TRUE);
   }

   if(!strncmp((char *)sector_buffer + 64, "PPPPHHHHOOOOTTTTOOOO____CCCCDDDD", 32))
    return(true);
  }
 }
 return(FALSE);
}

static int LoadCD(std::vector<CDIF *> *CDInterfaces)
{
 EmuFlags = 0;

 cdifs = CDInterfaces;

 DoMD5CDVoodoo(CDInterfaces);

 if(!LoadCommon(CDInterfaces))
  return(0);

 MDFN_printf(_("Emulated CD-ROM drive speed: %ux\n"), (unsigned int)MDFN_GetSettingUI("pcfx.cdspeed"));

 MDFNGameInfo->GameType = GMT_CDROM;

 PCFX_Power();

 return(1);
}

static void PCFX_CDInsertEject(void)
{
 CD_TrayOpen = !CD_TrayOpen;

 for(unsigned disc = 0; disc < cdifs->size(); disc++)
 {
  if(!(*cdifs)[disc]->Eject(CD_TrayOpen))
  {
   MDFN_DispMessage(_("Eject error."));
   CD_TrayOpen = !CD_TrayOpen;
  }
 }

 if(CD_TrayOpen)
  MDFN_DispMessage(_("Virtual CD Drive Tray Open"));
 else
  MDFN_DispMessage(_("Virtual CD Drive Tray Closed"));

 SCSICD_SetDisc(CD_TrayOpen, (CD_SelectedDisc >= 0 && !CD_TrayOpen) ? (*cdifs)[CD_SelectedDisc] : NULL);
}

static void PCFX_CDEject(void)
{
 if(!CD_TrayOpen)
  PCFX_CDInsertEject();
}

static void PCFX_CDSelect(void)
{
 if(cdifs && CD_TrayOpen)
 {
  CD_SelectedDisc = (CD_SelectedDisc + 1) % (cdifs->size() + 1);

  if((unsigned)CD_SelectedDisc == cdifs->size())
   CD_SelectedDisc = -1;

  if(CD_SelectedDisc == -1)
   MDFN_DispMessage(_("Disc absence selected."));
  else
   MDFN_DispMessage(_("Disc %d of %d selected."), CD_SelectedDisc + 1, (int)cdifs->size());
 }
}

static void CloseGame(void)
{
 if(!BRAMDisabled)
 {
  std::vector<PtrLengthPair> EvilRams;
 
  EvilRams.push_back(PtrLengthPair(BackupRAM, 0x8000));
  EvilRams.push_back(PtrLengthPair(ExBackupRAM, 0x8000));

  MDFN_DumpToFile(MDFN_MakeFName(MDFNMKF_SAV, 0, "sav").c_str(), 6, EvilRams);
 }

 for(int i = 0; i < 2; i++)
  if(fx_vdc_chips[i])
  {
   delete fx_vdc_chips[i];
   fx_vdc_chips[i] = NULL;
  }

 RAINBOW_Close();
 KING_Close();
 PCFX_V810.Kill();

 // The allocated memory RAM and BIOSROM is free'd in V810_Kill()
 RAM = NULL;
 BIOSROM = NULL;
}

static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
   case MDFN_MSC_INSERT_DISK:
		PCFX_CDInsertEject();
                break;

   case MDFN_MSC_SELECT_DISK:
		PCFX_CDSelect();
                break;

   case MDFN_MSC_EJECT_DISK:
		PCFX_CDEject();
                break;

  case MDFN_MSC_RESET: PCFX_Reset(); break;
  case MDFN_MSC_POWER: PCFX_Power(); break;
 }
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(RAM, 0x200000),
  SFARRAY16(Last_VDC_AR, 2),
  SFVAR(BackupControl),
  SFVAR(ExBusReset),
  SFARRAY(BackupRAM, BRAMDisabled ? 0 : 0x8000),
  SFARRAY(ExBackupRAM, BRAMDisabled ? 0 : 0x8000),

  SFVAR(pcfx_timestamp_base),

  SFVAR(next_pad_ts),
  SFVAR(next_timer_ts),
  SFVAR(next_adpcm_ts),
  SFVAR(next_king_ts),

  SFVAR(CD_TrayOpen),
  SFVAR(CD_SelectedDisc),

  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN");

 for(int i = 0; i < 2; i++)
  ret &= fx_vdc_chips[i]->StateAction(sm, load, data_only, i ? "VDC1" : "VDC0");

 ret &= FXINPUT_StateAction(sm, load, data_only);
 ret &= PCFXIRQ_StateAction(sm, load, data_only);
 ret &= KING_StateAction(sm, load, data_only);
 ret &= PCFX_V810.StateAction(sm, load, data_only);
 ret &= FXTIMER_StateAction(sm, load, data_only);
 ret &= SoundBox_StateAction(sm, load, data_only);
 ret &= SCSICD_StateAction(sm, load, data_only, "CDRM");
 ret &= RAINBOW_StateAction(sm, load, data_only);

 if(load)
 {
  //clamp(&PCFX_V810.v810_timestamp, 0, 30 * 1000 * 1000);
  //PCFX_SetEvent(PCFX_EVENT_SCSI, SCSICD_Run(v810_timestamp)); // hmm, fixme?

  if(cdifs)
  {
   // Sanity check.
   if(CD_SelectedDisc >= (int)cdifs->size())
    CD_SelectedDisc = (int)cdifs->size() - 1;

   SCSICD_SetDisc(CD_TrayOpen, (CD_SelectedDisc >= 0 && !CD_TrayOpen) ? (*cdifs)[CD_SelectedDisc] : NULL, true);
  }
 }

 return(ret);
}

static const MDFNSetting_EnumList V810Mode_List[] =
{
 { "fast", (int)V810_EMU_MODE_FAST, gettext_noop("Fast Mode"), gettext_noop("Fast mode trades timing accuracy, cache emulation, and executing from hardware registers and RAM not intended for code use for performance.")},
 { "accurate", (int)V810_EMU_MODE_ACCURATE, gettext_noop("Accurate Mode"), gettext_noop("Increased timing accuracy, though not perfect, along with cache emulation, at the cost of decreased performance.  Additionally, even the pipeline isn't correctly and fully emulated in this mode.") },
 { "auto", (int)_V810_EMU_MODE_COUNT, gettext_noop("Auto Mode"), gettext_noop("Selects \"fast\" or \"accurate\" automatically based on an internal database.  If the CD image is not recognized, defaults to \"fast\".") },
 { NULL, 0 },
};


static const MDFNSetting_EnumList HDCWidthList[] =
{
 { "256", 256,	"256 pixels", gettext_noop("This value will cause heavy pixel distortion.") },
 { "341", 341,	"341 pixels", gettext_noop("This value will cause moderate pixel distortion.") },
 { "1024", 1024, "1024 pixels", gettext_noop("This value will cause no pixel distortion as long as interpolation is enabled on the video output device and the resolution is sufficiently high, but it will use a lot of CPU time.") },
 { NULL, 0 },
};

static MDFNSetting PCFXSettings[] =
{
/*
  { "pcfx.input.port1", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 1."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
  { "pcfx.input.port2", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 2."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
  { "pcfx.input.port3", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 3."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
  { "pcfx.input.port4", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 4."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
  { "pcfx.input.port5", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 5."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
  { "pcfx.input.port6", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 6."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
  { "pcfx.input.port7", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 7."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
  { "pcfx.input.port8", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 8."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
*/

  { "pcfx.input.port1.multitap", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable multitap on PC-FX port 1."), gettext_noop("EXPERIMENTAL emulation of the unreleased multitap.  Enables ports 3 4 5."), MDFNST_BOOL, "0", NULL, NULL },
  { "pcfx.input.port2.multitap", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable multitap on PC-FX port 2."), gettext_noop("EXPERIMENTAL emulation of the unreleased multitap.  Enables ports 6 7 8."), MDFNST_BOOL, "0", NULL, NULL },


  { "pcfx.mouse_sensitivity", MDFNSF_NOFLAGS, gettext_noop("Mouse sensitivity."), NULL, MDFNST_FLOAT, "1.25", NULL, NULL },
  { "pcfx.disable_softreset", MDFNSF_NOFLAGS, gettext_noop("When RUN+SEL are pressed simultaneously, disable both buttons temporarily."), NULL, MDFNST_BOOL, "0", NULL, NULL, NULL, FXINPUT_SettingChanged },

  { "pcfx.cpu_emulation", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("CPU emulation mode."), NULL, MDFNST_ENUM, "auto", NULL, NULL, NULL, NULL, V810Mode_List },
  { "pcfx.bios", MDFNSF_EMU_STATE, gettext_noop("Path to the ROM BIOS"), NULL, MDFNST_STRING, "pcfx.rom" },
  { "pcfx.fxscsi", MDFNSF_EMU_STATE, gettext_noop("Path to the FX-SCSI ROM"), gettext_noop("Intended for developers only."), MDFNST_STRING, "0" },
  { "pcfx.disable_bram", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Disable internal and external BRAM."), gettext_noop("It is intended for viewing games' error screens that may be different from simple BRAM full and uninitialized BRAM error screens, though it can cause the game to crash outright."), MDFNST_BOOL, "0" },
  { "pcfx.cdspeed", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulated CD-ROM speed."), gettext_noop("Setting the value higher than 2, the default, will decrease loading times in most games by some degree."), MDFNST_UINT, "2", "2", "10" },

  { "pcfx.nospritelimit", MDFNSF_NOFLAGS, gettext_noop("Remove 16-sprites-per-scanline hardware limit."), NULL, MDFNST_BOOL, "0" },
  { "pcfx.high_dotclock_width", MDFNSF_NOFLAGS, gettext_noop("Emulated width for 7.16MHz dot-clock mode."), gettext_noop("Lower values are faster, but will cause some degree of pixel distortion."), MDFNST_ENUM, "1024", NULL, NULL, NULL, NULL, HDCWidthList },

  { "pcfx.slstart", MDFNSF_NOFLAGS, gettext_noop("First rendered scanline."), NULL, MDFNST_UINT, "4", "0", "239" },
  { "pcfx.slend", MDFNSF_NOFLAGS, gettext_noop("Last rendered scanline."), NULL, MDFNST_UINT, "235", "0", "239" },

  { "pcfx.rainbow.chromaip", MDFNSF_NOFLAGS, gettext_noop("Enable bilinear interpolation on the chroma channel of RAINBOW YUV output."), NULL, MDFNST_BOOL, "1" },

  { "pcfx.adpcm.suppress_channel_reset_clicks", MDFNSF_NOFLAGS, gettext_noop("Hack to suppress clicks caused by forced channel resets."), NULL, MDFNST_BOOL, "1" },

  // Hack that emulates the codec a buggy ADPCM encoder used for some games' ADPCM.  Not enabled by default because it makes some games(with 
  //correctly-encoded  ADPCM?) sound worse
  { "pcfx.adpcm.emulate_buggy_codec", MDFNSF_NOFLAGS, gettext_noop("Hack that emulates the codec a buggy ADPCM encoder used for some games' ADPCM."), NULL, MDFNST_BOOL, "0" },

  { NULL }
};

#ifdef WANT_DEBUGGER
static RegType PCFXRegs0[] =
{
        { 0, "PC", "Program Counter", 4 },
	{ 0, "PR1", "Program Register 1", 4 },
	{ 0, "HSP", "Program Register 2(Handler Stack Pointer)", 4 },
	{ 0, "SP", "Program Register 3(Stack Pointer)", 4 },
	{ 0, "GP", "Program Register 4(Global Pointer)", 4 },
	{ 0, "TP", "Program Register 5(Text Pointer)", 4 },
	{ 0, "PR6", "Program Register 6", 4 },
	{ 0, "PR7", "Program Register 7", 4 },
	{ 0, "PR8", "Program Register 8", 4 },
	{ 0, "PR9", "Program Register 9", 4 },
	{ 0, "PR10", "Program Register 10", 4 },
	{ 0, "PR11", "Program Register 11", 4 },
	{ 0, "PR12", "Program Register 12", 4 },
	{ 0, "PR13", "Program Register 13", 4 },
	{ 0, "PR14", "Program Register 14", 4 },
	{ 0, "PR15", "Program Register 15", 4 },
        { 0, "PR16", "Program Register 16", 4 },
        { 0, "PR17", "Program Register 17", 4 },
        { 0, "PR18", "Program Register 18", 4 },
        { 0, "PR19", "Program Register 19", 4 },
        { 0, "PR20", "Program Register 20", 4 },
        { 0, "PR21", "Program Register 21", 4 },
        { 0, "PR22", "Program Register 22", 4 },
        { 0, "PR23", "Program Register 23", 4 },
        { 0, "PR24", "Program Register 24", 4 },
        { 0, "PR25", "Program Register 25", 4 },
        { 0, "PR26", "Program Register 26(String Dest Bit Offset)", 4 },
        { 0, "PR27", "Program Register 27(String Source Bit Offset)", 4 },
        { 0, "PR28", "Program Register 28(String Length)", 4 },
        { 0, "PR29", "Program Register 29(String Dest)", 4 },
        { 0, "PR30", "Program Register 30(String Source)", 4 },
        { 0, "LP", "Program Register 31(Link Pointer)", 4 },
        { 0, "SR0", "Exception/Interrupt PC", 4 },
        { 0, "SR1", "Exception/Interrupt PSW", 4 },
        { 0, "SR2", "Fatal Error PC", 4 },
        { 0, "SR3", "Fatal Error PSW", 4 },
        { 0, "SR4", "Exception Cause Register", 4 },
        { 0, "SR5", "Program Status Word", 4 },
        { 0, "SR6", "Processor ID Register", 4 },
        { 0, "SR7", "Task Control Word", 4 },
        { 0, "SR24", "Cache Control Word", 4 },
        { 0, "SR25", "Address Trap Register", 4 },
	{ 0, "IPEND", "Interrupts Pending", 2 },
        { 0, "IMASK", "Interrupt Mask", 2 },
        { 0, "IPRIO0", "Interrupt Priority Register 0", 2 },
        { 0, "IPRIO1", "Interrupt Priority Register 1", 2 },
        { 0, "TCTRL", "Timer Control", 2 },
        { 0, "TPRD", "Timer Period", 2 },
        { 0, "TCNTR", "Timer Counter", 3 },

	{ 0, "KPCTRL0", "Keyport 0 Control", 1 },
	{ 0, "KPCTRL1", "Keyport 1 Control", 1 },

	{ 0, "TStamp", "Timestamp", 3 },
        { 0, "", "", 0 },
};

static RegType PCFXRegs1[] =
{
	{ 0, "AR", "Active Register", 1 },
	{ 0, "MPROGADDR", "Micro-program Address", 2 },
        { 0, "MPROGCTRL", "Micro-program Control", 2 },

	{ 0, "PAGESET", "KRAM Page Settings", 2 },
        { 0, "RTCTRL", "Rainbow Transfer Control", 2 },
        { 0, "RKRAMA", "Rainbow Transfer K-RAM Address", 3},
        { 0, "RSTART", "Rainbow Transfer Start Position", 2},
        { 0, "RCOUNT", "Rainbow Transfer Block Count", 2 },
        { 0, "RIRQLINE", "Raster IRQ Line", 2 },
	{ 0, "KRAMWA", "K-RAM Write Address", 4 },
	{ 0, "KRAMRA", "K-RAM Read Address", 4 },
        { 0, "DMATA", "DMA Transfer Address", 3 },
        { 0, "DMATS", "DMA Transfer Size", 4 },
        { 0, "DMASTT", "DMA Status", 2 },
        { 0, "ADPCMCTRL", "ADPCM Control", 2 },
        { 0, "ADPCMBM0", "ADPCM Buffer Mode Ch0", 2 },
        { 0, "ADPCMBM1", "ADPCM Buffer Mode Ch1", 2 },
        { 0, "ADPCMPA0", "ADPCM PlayAddress Ch0", 0x100 | 18 },
        { 0, "ADPCMPA1", "ADPCM PlayAddress Ch1", 0x100 | 18 },
        { 0, "ADPCMSA0", "ADPCM Start Address Ch0", 2 },
        { 0, "ADPCMSA1", "ADPCM Start Address Ch1", 2 },
        { 0, "ADPCMIA0", "ADPCM Intermediate Address Ch0", 2 },
        { 0, "ADPCMIA1", "ADPCM Intermediate Address Ch1", 2 },
        { 0, "ADPCMEA0", "ADPCM End Address Ch0", 0x100 | 18 },
        { 0, "ADPCMEA1", "ADPCM End Address Ch1", 0x100 | 18 },
	{ 0, "ADPCMStat", "ADPCM Status Register", 1 },
	{ 0, "Reg01", "KING Register 0x01", 1 },
	{ 0, "Reg02", "KING Register 0x02", 1 },
	{ 0, "Reg03", "KING Register 0x03", 1 },
	{ 0, "SUBCC", "Sub-channel Control", 1 },
	{ 0, "------", "", 0xFFFF },
	{ 0, "DB", "SCSI Data Bus", 0x100 | 8 },
	{ 0, "BSY", "SCSI BSY", 0x100 | 1 },
	{ 0, "REQ", "SCSI REQ", 0x100 | 1 },
	{ 0, "ACK", "SCSI ACK", 0x100 | 1 },
	{ 0, "MSG", "SCSI MSG", 0x100 | 1 },
	{ 0, "IO", "SCSI IO", 0x100 | 1 },
	{ 0, "CD", "SCSI CD", 0x100 | 1 },
	{ 0, "SEL", "SCSI SEL", 0x100 | 1 },

        { 0, "", "", 0 },
};

static RegType PCFXRegs4[] =
{
	{ 0, "--VCE--", "", 0xFFFF },
        { 0, "Line", "VCE Frame Counter", 0x100 | 9 },
        { 0, "PRIO0", "VCE Priority 0", 0x100 | 12 },
        { 0, "PRIO1", "VCE Priority 1", 2 },
        { 0, "PICMODE", "VCE Picture Mode", 2},
        { 0, "PALRWOF", "VCE Palette R/W Offset", 2 },
        { 0, "PALRWLA", "VCE Palette R/W Latch", 2 },
        { 0, "PALOFS0", "VCE Palette Offset 0", 2 } ,
        { 0, "PALOFS1", "VCE Palette Offset 1", 2 },
        { 0, "PALOFS2", "VCE Palette Offset 2", 2 },
        { 0, "PALOFS3", "VCE Palette Offset 3", 1 },
        { 0, "CCR", "VCE Fixed Color Register", 2 },
        { 0, "BLE", "VCE Cellophane Setting Register", 2 },
        { 0, "SPBL", "VCE Sprite Cellophane Setting Register", 2 },
        { 0, "COEFF0", "VCE Cellophane Coefficient 0(1A)", 0x100 | 12 },
        { 0, "COEFF1", "VCE Cellophane Coefficient 1(1B)", 0x100 | 12 },
        { 0, "COEFF2", "VCE Cellophane Coefficient 2(2A)", 0x100 | 12 },
        { 0, "COEFF3", "VCE Cellophane Coefficient 3(2B)", 0x100 | 12 },
        { 0, "COEFF4", "VCE Cellophane Coefficient 4(3A)", 0x100 | 12 },
        { 0, "COEFF5", "VCE Cellophane Coefficient 5(3B)", 0x100 | 12 },
        { 0, "CKeyY", "VCE Chroma Key Y", 2 },
        { 0, "CKeyU", "VCE Chroma Key U", 2 },
        { 0, "CKeyV", "VCE Chroma Key V", 2 },
        { 0, "", "", 0 },
};

static uint32 GetRegister_VDC(const unsigned int id, char *special, const uint32 special_len)
{
 return(fx_vdc_chips[(id >> 15) & 1]->GetRegister(id &~0x8000, special, special_len));
}

static void SetRegister_VDC(const unsigned int id, uint32 value)
{
 fx_vdc_chips[(id >> 15) & 1]->SetRegister(id &~0x8000, value);
} 


static RegType VDCRegs[] =
{
        { 0, "--VDC-A--", "", 0xFFFF },
	
        { VDC::GSREG_SELECT, "Select", "Register Select, VDC-A", 1 },
        { VDC::GSREG_STATUS, "Status", "Status, VDC-A", 1 },

        { VDC::GSREG_MAWR, "MAWR", "Memory Write Address, VDC-A", 2 },
        { VDC::GSREG_MARR, "MARR", "Memory Read Address, VDC-A", 2 },
        { VDC::GSREG_CR, "CR", "Control, VDC-A", 2 },
        { VDC::GSREG_RCR, "RCR", "Raster Counter, VDC-A", 2 },
        { VDC::GSREG_BXR, "BXR", "X Scroll, VDC-A", 2 },
        { VDC::GSREG_BYR, "BYR", "Y Scroll, VDC-A", 2 },
        { VDC::GSREG_MWR, "MWR", "Memory Width, VDC-A", 2 },

        { VDC::GSREG_HSR, "HSR", "HSR, VDC-A", 2 },
        { VDC::GSREG_HDR, "HDR", "HDR, VDC-A", 2 },
        { VDC::GSREG_VSR, "VSR", "VSR, VDC-A", 2 },
        { VDC::GSREG_VDR, "VDR", "VDR, VDC-A", 2 },

        { VDC::GSREG_VCR, "VCR", "VCR, VDC-A", 2 },
        { VDC::GSREG_DCR, "DCR", "DMA Control, VDC-A", 2 },
        { VDC::GSREG_SOUR, "SOUR", "VRAM DMA Source Address, VDC-A", 2 },
        { VDC::GSREG_DESR, "DESR", "VRAM DMA Dest Address, VDC-A", 2 },
        { VDC::GSREG_LENR, "LENR", "VRAM DMA Length, VDC-A", 2 },
        { VDC::GSREG_DVSSR, "DVSSR", "DVSSR Update Address, VDC-A", 2 },
        { 0, "------", "", 0xFFFF },

        { 0, "--VDC-B--", "", 0xFFFF },

        { 0x8000 | VDC::GSREG_SELECT, "Select", "Register Select, VDC-B", 1 },
        { 0x8000 | VDC::GSREG_STATUS, "Status", "Status, VDC-B", 1 },

        { 0x8000 | VDC::GSREG_MAWR, "MAWR", "Memory Write Address, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_MARR, "MARR", "Memory Read Address, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_CR, "CR", "Control, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_RCR, "RCR", "Raster Counter, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_BXR, "BXR", "X Scroll, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_BYR, "BYR", "Y Scroll, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_MWR, "MWR", "Memory Width, VDC-B", 2 },

        { 0x8000 | VDC::GSREG_HSR, "HSR", "HSR, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_HDR, "HDR", "HDR, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_VSR, "VSR", "VSR, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_VDR, "VDR", "VDR, VDC-B", 2 },

        { 0x8000 | VDC::GSREG_VCR, "VCR", "VCR, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_DCR, "DCR", "DMA Control, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_SOUR, "SOUR", "VRAM DMA Source Address, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_DESR, "DESR", "VRAM DMA Dest Address, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_LENR, "LENR", "VRAM DMA Length, VDC-B", 2 },
        { 0x8000 | VDC::GSREG_DVSSR, "DVSSR", "DVSSR Update Address, VDC-B", 2 },

        { 0, "", "", 0 },
};

static RegGroupType PCFXRegs0Group =
{
 NULL,
 PCFXRegs0,
 NULL,
 NULL,
 PCFXDBG_GetRegister,
 PCFXDBG_SetRegister,
};

static RegGroupType PCFXRegs1Group =
{
 NULL,
 PCFXRegs1,
 NULL,
 NULL,
 KING_GetRegister,
 KING_SetRegister
};


static RegGroupType PCFXRegs4Group =
{
 NULL,
 PCFXRegs4,
 NULL,
 NULL,
 FXVDCVCE_GetRegister,
 FXVDCVCE_SetRegister
};

static RegGroupType VDCRegsGroup =
{
 NULL,
 VDCRegs,
 GetRegister_VDC,
 SetRegister_VDC
};


static void SetRegGroups(void)
{
 MDFNDBG_AddRegGroup(&PCFXRegs0Group);
 MDFNDBG_AddRegGroup(&PCFXRegs4Group);
 MDFNDBG_AddRegGroup(&VDCRegsGroup);
 MDFNDBG_AddRegGroup(&PCFXRegs1Group);
}

static void ForceIRQ(int level)
{
 //v810_int(level);
}

static DebuggerInfoStruct DBGInfo =
{
 "shift_jis",
 4,
 2,             // Instruction alignment(bytes)
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
 PCFXDBG_SetLogFunc,
};
#endif

static const FileExtensionSpecStruct KnownExtensions[] =
{
 //{ ".ex", gettext_noop("PC-FX HuEXE") },
 { NULL, NULL }
};

MDFNGI EmulatedPCFX =
{
 "pcfx",
 "PC-FX",
 KnownExtensions,
 MODPRIO_INTERNAL_HIGH,
 #ifdef WANT_DEBUGGER
 &DBGInfo,
 #else
 NULL,
 #endif
 &PCFXInputInfo,
 NULL,
 NULL,
 LoadCD,
 TestMagicCD,
 CloseGame,
 KING_SetLayerEnableMask,
 "BG0\0BG1\0BG2\0BG3\0VDC-A BG\0VDC-A SPR\0VDC-B BG\0VDC-B SPR\0RAINBOW\0",
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 false,
 StateAction,
 Emulate,
 FXINPUT_SetInput,
 DoSimpleCommand,
 PCFXSettings,
 MDFN_MASTERCLOCK_FIXED(PCFX_MASTER_CLOCK),
 0,
 TRUE,  // Multires possible?

 0,   // lcm_width
 0,   // lcm_height
 NULL,  // Dummy

 288,	// Nominal width
 240,	// Nominal height

 1024,	// Framebuffer width
 512,	// Framebuffer height

 2,     // Number of output sound channels
};

