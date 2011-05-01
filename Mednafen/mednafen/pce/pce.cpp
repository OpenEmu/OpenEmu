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
#include "vce.h"
#include "pce_psg/pce_psg.h"
#include "input.h"
#include "huc.h"
#include "../cdrom/pcecd.h"
#include "hes.h"
#include "debug.h"
#include "tsushin.h"
#include "arcade_card/arcade_card.h"
#include "../mempatcher.h"
#include "../cdrom/cdromif.h"
#include "../md5.h"

#include <zlib.h>
#include <errno.h>

#define PCE_DEBUG(x, ...) { /* printf(x, ## __VA_ARGS__); */ }

namespace MDFN_IEN_PCE
{

static const MDFNSetting_EnumList PSGRevisionList[] =
{
 { "huc6280", PCE_PSG::REVISION_HUC6280, "HuC6280", gettext_noop("HuC6280 as found in the original PC Engine.") },
 { "huc6280a", PCE_PSG::REVISION_HUC6280A, "HuC6280A", gettext_noop("HuC6280A as found in the SuperGrafx and CoreGrafx I.  Provides proper channel amplitude centering, but may cause clicking in a few games designed with the original HuC6280's sound characteristics in mind.") },
 { "enhanced", PCE_PSG::REVISION_ENHANCED, "Enhanced", gettext_noop("Emulator-only pseudo-revision, designed to reduce clicking and audio artifacts in games designed for either real revision.") },
 { "match", PCE_PSG::_REVISION_COUNT, gettext_noop("Match emulation mode."), gettext_noop("Selects \"huc6280\" for non-SuperGrafx mode, and \"huc6280a\" for SuperGrafx(full) mode.") },
 { NULL, 0 },
};

HuC6280 *HuCPU;

VCE *vce = NULL;

static PCE_PSG *psg = NULL;

extern ArcadeCard *arcade_card;	// Bah, lousy globals.

static Blip_Buffer sbuf[2];
static bool SetSoundRate(double rate);


bool PCE_ACEnabled;
uint32 PCE_InDebug = 0;
uint64 PCE_TimestampBase;	// Only used with the debugger for the time being.

extern MDFNGI EmulatedPCE;
static bool IsSGX;
static bool IsHES;

// Accessed in debug.cpp
static uint8 BaseRAM[32768]; // 8KB for PCE, 32KB for Super Grafx

HuC6280::readfunc NonCheatPCERead[0x100];

static DECLFR(PCEBusRead)
{
 if(!PCE_InDebug)
 {
  PCE_DEBUG("Unmapped Read: %02x %04x\n", A >> 13, A);
 }
 return(0xFF);
}

static DECLFW(PCENullWrite)
{
 if(!PCE_InDebug)
 {
  PCE_DEBUG("Unmapped Write: %02x, %08x %02x\n", A >> 13, A, V);
 }
}

static DECLFR(BaseRAMReadSGX)
{
 return(BaseRAM[A & 0x7FFF]);
}

static DECLFW(BaseRAMWriteSGX)
{
 BaseRAM[A & 0x7FFF] = V;
}

static DECLFR(BaseRAMRead)
{
 return(BaseRAM[A & 0x1FFF]);
}

static DECLFW(BaseRAMWrite)
{
 BaseRAM[A & 0x1FFF] = V;
}

static DECLFR(IORead)
{
 A &= 0x1FFF;

 switch(A & 0x1c00)
 {
  case 0x0000: if(!PCE_InDebug)
		HuCPU->StealCycle(); 
	       return(vce->ReadVDC(A));

  case 0x0400: if(!PCE_InDebug)
		HuCPU->StealCycle(); 
	       return(vce->Read(A));

  case 0x0800: if(HuCPU->InBlockMove())
		return(0);
	       return(HuCPU->GetIODataBuffer());

  case 0x0c00: if(HuCPU->InBlockMove())
                return(0);
	       {
		uint8 ret = HuCPU->TimerRead(A, PCE_InDebug);
                if(!PCE_InDebug)
		 HuCPU->SetIODataBuffer(ret);
                return(ret);
               }

  case 0x1000: if(HuCPU->InBlockMove())
                return(0);
	       {
	        uint8 ret = INPUT_Read(HuCPU->Timestamp(), A);
                if(!PCE_InDebug)
		 HuCPU->SetIODataBuffer(ret);
                return(ret);
               }

  case 0x1400: if(HuCPU->InBlockMove())
                return(0);
	       {
	        uint8 ret = HuCPU->IRQStatusRead(A, PCE_InDebug);
		if(!PCE_InDebug)
		 HuCPU->SetIODataBuffer(ret);
	        return(ret);
	       }

  case 0x1800: if(IsTsushin)
		return(PCE_TsushinRead(A));

	       if(!PCE_IsCD)
		break;
	       if((A & 0x1E00) == 0x1A00)
	       {
		if(arcade_card)
		 return(arcade_card->Read(A, PCE_InDebug));
		else
		 return(0);
	       }
	       else
	       {
		int32 next_cd_event;
		uint8 ret;

		ret = PCECD_Read(HuCPU->Timestamp(), A, next_cd_event, PCE_InDebug);

		vce->SetCDEvent(next_cd_event);

		return(ret);
	       }

  case 0x1C00: if(IsHES)
		return(ReadIBP(A)); 
	       break; // Expansion
 }

 if(!PCE_InDebug)
 {
  PCE_DEBUG("I/O Unmapped Read: %04x\n", A);
 }

 return(0xFF);
}

static DECLFW(IOWrite)
{
 switch(A & 0x1c00)
 {
  case 0x0000: HuCPU->StealCycle();
	       vce->WriteVDC(A & 0x80001FFF, V);
	       break;

  case 0x0400: HuCPU->StealCycle(); 
	       vce->Write(A & 0x1FFF, V);
	       break;

  case 0x0800: HuCPU->SetIODataBuffer(V); 
	       psg->Write(HuCPU->Timestamp() / 3, A & 0x1FFF, V);
	       break;

  case 0x0c00: HuCPU->SetIODataBuffer(V);
	       HuCPU->TimerWrite(A & 0x1FFF, V);
	       break;

  case 0x1000: HuCPU->SetIODataBuffer(V);
	       INPUT_Write(HuCPU->Timestamp(), A & 0x1FFF, V);
	       break;

  case 0x1400: HuCPU->SetIODataBuffer(V);
	       HuCPU->IRQStatusWrite(A & 0x1FFF, V);
	       break;

  case 0x1800: if(IsTsushin)
                PCE_TsushinWrite(A & 0x1FFF, V);

	       if(!PCE_IsCD)
	       {
		if(!PCE_InDebug)
		{
		 PCE_DEBUG("I/O Unmapped Write: %04x %02x\n", A, V);
		}
		break;
	       }

	       if((A & 0x1E00) == 0x1A00)
	       {
		if(arcade_card)
		 arcade_card->Write(A& 0x1FFF, V);
	       }
	       else
	       {
	        int32 next_cd_event = PCECD_Write(HuCPU->Timestamp(), A & 0x1FFF, V);

		vce->SetCDEvent(next_cd_event);
	       }
		
	       break;

  case 0x1C00:  if(!PCE_InDebug)
		{
		 PCE_DEBUG("I/O Unmapped Write: %04x %02x\n", A, V);
		}
		break;
 }
}

static void PCECDIRQCB(bool asserted)
{
 if(asserted)
  HuCPU->IRQBegin(HuC6280::IQIRQ2);
 else
  HuCPU->IRQEnd(HuC6280::IQIRQ2);
}

static bool LoadCustomPalette(const char *path)
{
 uint8 CustomColorMap[1024 * 3];

 MDFN_printf(_("Loading custom palette from \"%s\"...\n"),  path);
 MDFN_indent(1);

 gzFile gp = gzopen(path, "rb");
 if(!gp)
 {
  ErrnoHolder ene(errno);

  MDFN_printf(_("Error opening file: %s\n"), ene.StrError());        // FIXME, zlib and errno...
  MDFN_indent(-1);
  return(FALSE);
 }
 else
 {
  long length_read;

  length_read = gzread(gp, CustomColorMap, 1024 * 3);

  if(length_read == 512 * 3)
   MDFN_printf("Palette only has 512 entries.  Calculating the strip-colorburst entries.\n");

  if(length_read != 1024 * 3 && length_read != 512 * 3)
  {
   MDFN_printf(_("Error reading file\n"));
   MDFN_indent(-1);
   return(FALSE);
  }

  vce->SetCustomColorMap(CustomColorMap, length_read / 3);
 }
 MDFN_indent(-1);

 return(TRUE);
}


static int LoadCommon(void);
static void LoadCommonPre(void);

static bool TestMagic(const char *name, MDFNFILE *fp)
{
 if(memcmp(fp->data, "HESM", 4) && strcasecmp(fp->ext, "pce") && strcasecmp(fp->ext, "sgx"))
  return(FALSE);

 return(TRUE);
}

static void SetCDSettings(bool silent_status = false)
{
 PCECD_Settings cd_settings;
 memset(&cd_settings, 0, sizeof(PCECD_Settings));

 cd_settings.CDDA_Volume = (double)MDFN_GetSettingUI("pce.cddavolume") / 100;
 cd_settings.CD_Speed = 1;

 cd_settings.ADPCM_Volume = (double)MDFN_GetSettingUI("pce.adpcmvolume") / 100;
 cd_settings.ADPCM_LPF = MDFN_GetSettingB("pce.adpcmlp");

 PCECD_SetSettings(&cd_settings);

 if(!silent_status)
 {
  if(cd_settings.CDDA_Volume != 1.0)
   MDFN_printf(_("CD-DA Volume: %d%%\n"), (int)(100 * cd_settings.CDDA_Volume));

  if(cd_settings.ADPCM_Volume != 1.0)
   MDFN_printf(_("ADPCM Volume: %d%%\n"), (int)(100 * cd_settings.ADPCM_Volume));
 }


 unsigned int cdpsgvolume = MDFN_GetSettingUI("pce.cdpsgvolume");

 if(cdpsgvolume != 100)
 {
  MDFN_printf(_("CD PSG Volume: %d%%\n"), cdpsgvolume);
 }

 psg->SetVolume(0.678 * cdpsgvolume / 100);
}

static void CDSettingChanged(const char *name)
{
 SetCDSettings(true);
}

static int Load(const char *name, MDFNFILE *fp)
{
 uint32 headerlen = 0;
 uint32 r_size;

 IsHES = 0;
 IsSGX = 0;

 if(!memcmp(fp->data, "HESM", 4))
  IsHES = 1;

 LoadCommonPre();

 if(!IsHES)
 {
  if(fp->size & 0x200) // 512 byte header!
   headerlen = 512;
 }

 r_size = fp->size - headerlen;
 if(r_size > 4096 * 1024) r_size = 4096 * 1024;

 uint32 crc = crc32(0, fp->data + headerlen, fp->size - headerlen);


 if(IsHES)
 {
  if(!PCE_HESLoad(fp->data, fp->size))
   return(0);

  // FIXME:  If a HES rip tries to execute a SCSI command, the CD emulation code will probably crash.  Obviously, a HES rip shouldn't do this,
  // but Mednafen shouldn't crash either. ;)
  PCE_IsCD = 1;
  PCECD_Init(NULL, PCECDIRQCB, PCE_MASTER_CLOCK, 1, &sbuf[0], &sbuf[1]);
 }
 else
 {
  HuCLoad(fp->data + headerlen, fp->size - headerlen, crc, MDFN_GetSettingB("pce.disable_bram_hucard"));
  #if 0	// For testing
  PCE_IsCD = 1;
  PCECD_Init(NULL, PCECDIRQCB, PCE_MASTER_CLOCK, 1, &sbuf[0], &sbuf[1]);
  #endif
 }
 if(!strcasecmp(fp->ext, "sgx"))
  IsSGX = TRUE;

 if(fp->size >= 8192 && !memcmp(fp->data + headerlen, "DARIUS Version 1.11b", strlen("DARIUS VERSION 1.11b")))
 {
  MDFN_printf("SuperGfx:  Darius Plus\n");
  IsSGX = 1;
 }

 if(crc == 0x4c2126b0)
 {
  MDFN_printf("SuperGfx:  Aldynes\n");
  IsSGX = 1;
 }

 if(crc == 0x8c4588e2)
 {
  MDFN_printf("SuperGfx:  1941 - Counter Attack\n");
  IsSGX = 1;
 }
 if(crc == 0x1f041166)
 {
  MDFN_printf("SuperGfx:  Madouou Granzort\n");
  IsSGX = 1;
 }
 if(crc == 0xb486a8ed)
 {
  MDFN_printf("SuperGfx:  Daimakaimura\n");
  IsSGX = 1;
 }
 if(crc == 0x3b13af61)
 {
  MDFN_printf("SuperGfx:  Battle Ace\n");
  IsSGX = 1;
 }

 return(LoadCommon());
}

static void LoadCommonPre(void)
{
 // FIXME:  Make these globals less global!
 PCE_ACEnabled = MDFN_GetSettingB("pce.arcadecard");

 HuCPU = new HuC6280(IsHES);

 for(int x = 0; x < 0x100; x++)
 {
  HuCPU->SetFastRead(x, NULL);
  HuCPU->SetReadHandler(x, PCEBusRead);
  HuCPU->SetWriteHandler(x, PCENullWrite);
 }

 MDFNMP_Init(1024, (1 << 21) / 1024);
}

static int LoadCommon(void)
{ 
 IsSGX |= MDFN_GetSettingB("pce.forcesgx") ? 1 : 0;

 if(IsHES)
  IsSGX = 1;
 // Don't modify IsSGX past this point.
 
 vce = new VCE(IsSGX, MDFN_GetSettingB("pce.nospritelimit"));

 if(IsSGX)
  MDFN_printf("SuperGrafx Emulation Enabled.\n");

 for(int i = 0xF8; i < 0xFC; i++)
 {
  HuCPU->SetReadHandler(i, IsSGX ? BaseRAMReadSGX : BaseRAMRead);
  HuCPU->SetWriteHandler(i, IsSGX ? BaseRAMWriteSGX : BaseRAMWrite);

  if(IsSGX)
   HuCPU->SetFastRead(i, BaseRAM + (i & 0x3) * 8192);
  else
   HuCPU->SetFastRead(i, BaseRAM);
 }

 LoadCustomPalette(MDFN_MakeFName(MDFNMKF_PALETTE, 0, NULL).c_str());

 MDFNMP_AddRAM(IsSGX ? 32768 : 8192, 0xf8 * 8192, BaseRAM);

 HuCPU->SetReadHandler(0xFF, IORead);
 HuCPU->SetWriteHandler(0xFF, IOWrite);

 {
  int psgrevision = MDFN_GetSettingI("pce.psgrevision");

  if(psgrevision == PCE_PSG::_REVISION_COUNT)
  {
   psgrevision = IsSGX ? PCE_PSG::REVISION_HUC6280A : PCE_PSG::REVISION_HUC6280;
  }

  for(const MDFNSetting_EnumList *el = PSGRevisionList; el->string; el++)
  {
   if(el->number == psgrevision)
   {
    MDFN_printf(_("PSG Revision: %s\n"), el->description);
    break;
   }
  }
  psg = new PCE_PSG(&sbuf[0], &sbuf[1], psgrevision);
 }

 psg->SetVolume(1.0);

 if(PCE_IsCD)
  SetCDSettings();

 SetSoundRate(0);

 PCEINPUT_Init();

 PCE_Power();

 MDFNGameInfo->LayerNames = IsSGX ? "BG0\0SPR0\0BG1\0SPR1\0" : "Background\0Sprites\0";
 MDFNGameInfo->fps = (uint32)((double)7159090.90909090 / 455 / 263 * 65536 * 256);


 for(unsigned int i = 0; i < 0x100; i++)
  NonCheatPCERead[i] = HuCPU->GetReadHandler(i);

 if(!IsHES)
 {
  MDFNGameInfo->nominal_height = MDFN_GetSettingUI("pce.slend") - MDFN_GetSettingUI("pce.slstart") + 1;
  MDFNGameInfo->nominal_width = MDFN_GetSettingB("pce.h_overscan") ? 320 : 288;

  MDFNGameInfo->lcm_width = MDFN_GetSettingB("pce.h_overscan") ? 1120 : 1024;
  MDFNGameInfo->lcm_height = MDFNGameInfo->nominal_height;
 }

 vce->SetShowHorizOS(MDFN_GetSettingB("pce.h_overscan")); 

#ifdef WANT_DEBUGGER
 PCEDBG_Init(IsSGX, psg);
#endif


 return(1);
}

static bool TestMagicCD(void)
{
 static const uint8 magic_test[0x20] = { 0x82, 0xB1, 0x82, 0xCC, 0x83, 0x76, 0x83, 0x8D, 0x83, 0x4F, 0x83, 0x89, 0x83, 0x80, 0x82, 0xCC,  
				   	 0x92, 0x98, 0x8D, 0xEC, 0x8C, 0xA0, 0x82, 0xCD, 0x8A, 0x94, 0x8E, 0xAE, 0x89, 0xEF, 0x8E, 0xD0
				       };
 uint8 sector_buffer[2048];
 CD_TOC toc;
 bool ret = FALSE;

 memset(sector_buffer, 0, sizeof(sector_buffer));

 CDIF_ReadTOC(&toc);

 for(int32 track = toc.first_track; track <= toc.last_track; track++)
 {
  if(toc.tracks[track].control & 0x4)
  {
   CDIF_ReadSector(sector_buffer, toc.tracks[track].lba, 1);

   if(!memcmp((char*)sector_buffer, (char *)magic_test, 0x20))
    ret = TRUE;

   // PCE CD BIOS apparently only looks at the first data track.
   break;
  }
 }

 // If it's a PC-FX CD(Battle Heat), return false.
 // This is very kludgy.
 for(int32 track = toc.first_track; track <= toc.last_track; track++)
 {
  if(toc.tracks[track].control & 0x4)
  {
   CDIF_ReadSector(sector_buffer, toc.tracks[track].lba, 1);
   if(!strncmp("PC-FX:Hu_CD-ROM", (char*)sector_buffer, strlen("PC-FX:Hu_CD-ROM")))
   {
    return(false);
   }
  }
 }


 // Now, test for the Games Express CD games.  The GE BIOS seems to always look at sector 0x10, but only if the first track is a
 // data track.
 if(toc.first_track == 1 && (toc.tracks[1].control & 0x4))
 {
  if(CDIF_ReadSector(sector_buffer, 0x10, 1))
  {
   if(!memcmp((char *)sector_buffer + 0x8, "HACKER CD ROM SYSTEM", 0x14))
   {
    ret = TRUE;
   }
  }
 }

 return(ret);
}

static int LoadCD(void)
{
 static const FileExtensionSpecStruct KnownBIOSExtensions[] =
 {
  { ".pce", gettext_noop("PC Engine ROM Image") },
  { ".bin", gettext_noop("PC Engine ROM Image") },
  { ".bios", gettext_noop("BIOS Image") },
  { NULL, NULL }
 };
 md5_context md5;
 uint32 headerlen = 0;

 MDFNFILE fp;

 IsHES = 0;
 IsSGX = 0;

 LoadCommonPre();

 std::string bios_path = MDFN_MakeFName(MDFNMKF_FIRMWARE, 0, MDFN_GetSettingS("pce.cdbios").c_str() );

 if(!fp.Open(bios_path, KnownBIOSExtensions, _("CD BIOS")))
 {
  return(0);
 }

 if(fp.Size() & 0x200)
  headerlen = 512;


 //md5.starts();
 //md5.update(fp.Data(), fp.Size());
 //md5.update(MDFNGameInfo->MD5, 16);
 //md5.finish(MDFNGameInfo->MD5);

 bool disable_bram_cd = MDFN_GetSettingB("pce.disable_bram_cd");

 if(disable_bram_cd)
  MDFN_printf(_("Warning: BRAM is disabled per pcfx.disable_bram_cd setting.  This is simulating a malfunction.\n"));

 if(!HuCLoad(fp.Data() + headerlen, fp.Size() - headerlen, 0, disable_bram_cd, PCE_ACEnabled ? SYSCARD_ARCADE : SYSCARD_3))
 {
  return(0);
 }

 PCE_IsCD = 1;

 if(!PCECD_Init(NULL, PCECDIRQCB, PCE_MASTER_CLOCK, 1, &sbuf[0], &sbuf[1]))
 {
  HuCClose();
  return(0);
 }

 MDFN_printf(_("CD Layout:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());
 MDFN_printf(_("Arcade Card Emulation:  %s\n"), PCE_ACEnabled ? _("Enabled") : _("Disabled"));

 return(LoadCommon());
}


static void CloseGame(void)
{
 if(PCE_IsCD)
 {
  PCECD_Close();
 }

 if(IsHES)
  HES_Close();
 else
 {
  HuCClose();
 }

 if(vce)
 {
  delete vce;
  vce = NULL;
 }

 if(psg)
 {
  delete psg;
  psg = NULL;
 }

 if(HuCPU)
 {
  delete HuCPU;
  HuCPU = NULL;
 }
}

static EmulateSpecStruct *es;
static void Emulate(EmulateSpecStruct *espec)
{
 es = espec;

 espec->MasterCycles = 0;
 espec->SoundBufSize = 0;

 MDFNMP_ApplyPeriodicCheats();

 if(espec->VideoFormatChanged)
  vce->SetPixelFormat(espec->surface->format);

 if(espec->SoundFormatChanged)
  SetSoundRate(espec->SoundRate);

 vce->StartFrame(espec->surface, &espec->DisplayRect, espec->LineWidths, IsHES ? 1 : espec->skip);

 // Begin loop here:
 //for(int i = 0; i < 2; i++)
 bool rp_rv;
 do
 {
  INPUT_Frame();

  //vce->RunFrame(espec->surface, &espec->DisplayRect, espec->LineWidths, IsHES ? 1 : espec->skip);
  rp_rv = vce->RunPartial();

  INPUT_FixTS(HuCPU->Timestamp());

  psg->EndFrame(HuCPU->Timestamp() / 3);

  //assert(!(HuCPU->Timestamp() % 3));

  if(espec->SoundBuf)
  {
   int32 new_sc;
   for(int y = 0; y < 2; y++)
   {
    sbuf[y].end_frame(HuCPU->Timestamp() / 3);

    new_sc = sbuf[y].read_samples(espec->SoundBuf + espec->SoundBufSize * 2 + y, espec->SoundBufMaxSize - espec->SoundBufSize, 1);
   }
   espec->SoundBufSize += new_sc;
  }

  PCE_TimestampBase += HuCPU->Timestamp();
  espec->MasterCycles += HuCPU->Timestamp();

  HuCPU->SyncAndResetTimestamp();

  if(PCE_IsCD)
   PCECD_ResetTS();

  if(!rp_rv)
  {
   MDFN_MidSync(espec);
  }
 } while(!rp_rv);

 // End loop here.
 //printf("%d\n", vce->GetScanlineNo());

 if(IsHES)
  HES_Update(espec, INPUT_HESHack());	//Draw(espec->skip ? NULL : espec->surface, espec->skip ? NULL : &espec->DisplayRect, espec->SoundBuf, espec->SoundBufSize, INPUT_HESHack());

#if 0
 if(espec->SoundRate)
 {
  unsigned long long crf = (unsigned long long)sbuf[0].clock_rate_factor(sbuf[0].clock_rate());
  double real_rate = (double)crf * sbuf[0].clock_rate() / (1ULL << BLIP_BUFFER_ACCURACY);

  printf("%f\n", real_rate);
 }
#endif
}

void PCE_MidSync(void)
{
 INPUT_Frame();

 es->MasterCycles = HuCPU->Timestamp();

 //MDFN_MidSync(es);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(BaseRAM, IsSGX? 32768 : 8192),
  SFVAR(PCE_TimestampBase),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN");

 ret &= HuCPU->StateAction(sm, load, data_only);
 ret &= vce->StateAction(sm, load, data_only);
 ret &= psg->StateAction(sm, load, data_only);
 ret &= INPUT_StateAction(sm, load, data_only);
 ret &= HuC_StateAction(sm, load, data_only);

 if(load)
 {

 }

 return(ret);
}

void PCE_Power(void)
{
 memset(BaseRAM, 0x00, sizeof(BaseRAM));

 HuCPU->Power();
 PCE_TimestampBase = 0;
 const int32 timestamp = HuCPU->Timestamp();

 vce->Reset(timestamp);
 psg->Power(timestamp);

 if(IsHES)
  HES_Reset();
 else
  HuC_Power();

 PCEINPUT_Power(timestamp);

 if(PCE_IsCD)
 {
  vce->SetCDEvent(PCECD_Power(timestamp));
 }
}

static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
  case MDFN_MSC_RESET: PCE_Power(); break;
  case MDFN_MSC_POWER: PCE_Power(); break;
 }
}

static MDFNSetting PCESettings[] = 
{
  { "pce.input.multitap", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable multitap(TurboTap) emulation."), NULL, MDFNST_BOOL, "1" },

  { "pce.slstart", MDFNSF_NOFLAGS, gettext_noop("First rendered scanline."), NULL, MDFNST_UINT, "4", "0", "239" },
  { "pce.slend", MDFNSF_NOFLAGS, gettext_noop("Last rendered scanline."), NULL, MDFNST_UINT, "235", "0", "239" },

  { "pce.h_overscan", MDFNSF_NOFLAGS, gettext_noop("Show horizontal overscan area."), NULL, MDFNST_BOOL, "0" },

  { "pce.mouse_sensitivity", MDFNSF_NOFLAGS, gettext_noop("Emulated mouse sensitivity."), NULL, MDFNST_FLOAT, "0.50", NULL, NULL, NULL, PCEINPUT_SettingChanged },
  { "pce.disable_softreset", MDFNSF_NOFLAGS, gettext_noop("If set, when RUN+SEL are pressed simultaneously, disable both buttons temporarily."), NULL, MDFNST_BOOL, "0", NULL, NULL, NULL, PCEINPUT_SettingChanged },

  { "pce.disable_bram_cd", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Disable BRAM(saved game memory) for CD games."), gettext_noop("It is intended for viewing CD games' error screens that may be different from simple BRAM full and uninitialized BRAM error screens, though it can cause the game to crash outright."), MDFNST_BOOL, "0" },
  { "pce.disable_bram_hucard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Disable BRAM(saved game memory) for HuCard games."), gettext_noop("It is intended for changing the behavior(passwords vs save games) of some HuCard games."), MDFNST_BOOL, "0" },

  { "pce.forcesgx", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Force SuperGrafx emulation."), 
		gettext_noop("Enabling this option is not necessary to run unrecognized PCE ROM images in SuperGrafx mode, and enabling it is discouraged; ROM images with a file extension of \".sgx\" will automatically enable SuperGrafx emulation."), MDFNST_BOOL, "0" },

  { "pce.arcadecard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable Arcade Card emulation."), 
	gettext_noop("Leaving this option enabled is recommended, unless you want to see special warning screens on ACD games, or you prefer the non-enhanced modes of ACD-enhanced SCD games.  Additionally, you may want to disable it you you wish to use state rewinding with a SCD ACD-enhanced game on a slow CPU, as the extra 2MiB of RAM the Arcade Card offers is difficult to compress in real-time."), MDFNST_BOOL, "1" },

  { "pce.nospritelimit", MDFNSF_NOFLAGS, gettext_noop("Remove 16-sprites-per-scanline hardware limit."), 
					 gettext_noop("WARNING: Enabling this option may cause undesirable graphics glitching on some games(such as \"Bloody Wolf\")."), MDFNST_BOOL, "0" },

  { "pce.cdbios", MDFNSF_EMU_STATE, gettext_noop("Path to the CD BIOS"), NULL, MDFNST_STRING, "syscard3.pce" },

  { "pce.adpcmlp", MDFNSF_NOFLAGS, gettext_noop("Enable lowpass filter with rolloff dependent on playback-frequency."), 
	gettext_noop("This makes ADPCM voices sound less \"harsh\", however, the downside is that it will cause many ADPCM sound effects to sound a bit muffled."), MDFNST_BOOL, "0", NULL, NULL, NULL, CDSettingChanged },

  { "pce.psgrevision", MDFNSF_NOFLAGS, gettext_noop("Select PSG revision."), gettext_noop("WARNING: HES playback will always use the \"huc6280a\" revision if this setting is set to \"match\", since HES playback is always done with SuperGrafx emulation enabled."), MDFNST_ENUM, "huc6280a", NULL, NULL, NULL, NULL, PSGRevisionList  },

  { "pce.cdpsgvolume", MDFNSF_NOFLAGS, gettext_noop("PSG volume when playing a CD game."), NULL, MDFNST_UINT, "100", "0", "200", NULL, CDSettingChanged },
  { "pce.cddavolume", MDFNSF_NOFLAGS, gettext_noop("CD-DA volume."), NULL, MDFNST_UINT, "100", "0", "200", NULL, CDSettingChanged },
  { "pce.adpcmvolume", MDFNSF_NOFLAGS, gettext_noop("ADPCM volume."), NULL, MDFNST_UINT, "100", "0", "200", NULL, CDSettingChanged },

  { "pce.vramsize", MDFNSF_NOFLAGS, gettext_noop("Size of emulated VRAM per VDC in 16-bit words.  DO NOT CHANGE THIS UNLESS YOU KNOW WTF YOU ARE DOING."), NULL, MDFNST_UINT, "32768", "32768", "65536" },
  { NULL }
};

static DECLFR(CheatReadFunc)
{
  std::vector<SUBCHEAT>::iterator chit;
  uint8 retval = NonCheatPCERead[(A / 8192) & 0xFF](A);

  for(chit = SubCheats[A & 0x7].begin(); chit != SubCheats[A & 0x7].end(); chit++)
  {
   if(A == chit->addr)
   {
    if(chit->compare == -1 || chit->compare == retval)
    {
     retval = chit->value;
     break;
    }
   }
  }
 return(retval);
}

static uint8 MemRead(uint32 addr)
{
 return(NonCheatPCERead[(addr / 8192) & 0xFF](addr));
}

static void InstallReadPatch(uint32 address)
{
 HuCPU->SetFastRead(address >> 13, NULL);
 HuCPU->SetReadHandler(address >> 13, CheatReadFunc);
}

static void RemoveReadPatches(void)
{
 for(int x = 0; x < 0x100; x++)
  HuCPU->SetReadHandler(x, NonCheatPCERead[x]);
}

static bool ToggleLayer(int which)
{
 return(vce->ToggleLayer(which));
}

static const FileExtensionSpecStruct KnownExtensions[] =
{
 { ".pce", gettext_noop("PC Engine ROM Image") },
 { ".hes", gettext_noop("PC Engine Music Rip") },
 { ".sgx", gettext_noop("SuperGrafx ROM Image") },
 { NULL, NULL }
};

static bool SetSoundRate(double rate)
{
 for(int y = 0; y < 2; y++)
 {
  sbuf[y].set_sample_rate(rate ? rate : 44100, 50);
  sbuf[y].clock_rate((long)(PCE_MASTER_CLOCK / 3));
  sbuf[y].bass_freq(20);
 }

 return(TRUE);
}

};

using namespace MDFN_IEN_PCE;

MDFNGI EmulatedPCE =
{
 "pce",
 "PC Engine (CD)/TurboGrafx 16 (CD)/SuperGrafx",
 KnownExtensions,
 MODPRIO_INTERNAL_HIGH,
 #ifdef WANT_DEBUGGER
 &PCEDBGInfo,
 #else
 NULL,
 #endif
 &PCEInputInfo,
 Load,
 TestMagic,
 LoadCD,
 TestMagicCD,
 CloseGame,
 ToggleLayer,
 NULL,
 InstallReadPatch,
 RemoveReadPatches,
 MemRead,
 StateAction,
 Emulate,
 PCEINPUT_SetInput,
 DoSimpleCommand,
 PCESettings,
 MDFN_MASTERCLOCK_FIXED(PCE_MASTER_CLOCK),
 0,
 TRUE,  // Multires possible?

 0,   // lcm_width
 0,   // lcm_height
 NULL,  // Dummy

 320,	// Nominal width
 232,	// Nominal height
 1365,	// Framebuffer width
 270,	// Framebuffer height(TODO: decrease to 264(263 + spillover line))

 2,     // Number of output sound channels
};

