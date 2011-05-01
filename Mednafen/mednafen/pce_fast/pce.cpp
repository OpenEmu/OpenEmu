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
#include <zlib.h>
#include "vdc.h"
#include "pce_psg/pce_psg.h"
#include "input.h"
#include "huc.h"
#include "../cdrom/pcecd.h"
#include "hes.h"
#include "tsushin.h"
#include "arcade_card/arcade_card.h"
#include "../mempatcher.h"
#include "../cdrom/cdromif.h"

namespace PCE_Fast
{

static PCE_PSG *psg = NULL;
extern ArcadeCard *arcade_card; // Bah, lousy globals.

static Blip_Buffer sbuf[2];

bool PCE_ACEnabled;

static bool IsSGX;
static bool IsHES;
int pce_overclocked;

// Statically allocated for speed...or something.
uint8 ROMSpace[0x88 * 8192 + 8192];	// + 8192 for PC-as-pointer safety padding

uint8 BaseRAM[32768 + 8192]; // 8KB for PCE, 32KB for Super Grafx // + 8192 for PC-as-pointer safety padding

uint8 PCEIODataBuffer;
readfunc PCERead[0x100];
writefunc PCEWrite[0x100];

static DECLFR(PCEBusRead)
{
 //printf("BUS Read: %02x %04x\n", A >> 13, A);
 return(0xFF);
}

static DECLFW(PCENullWrite)
{
 //printf("Null Write: %02x, %08x %02x\n", A >> 13, A, V);
}

static DECLFR(BaseRAMReadSGX)
{
 return((BaseRAM - (0xF8 * 8192))[A]);
}

static DECLFW(BaseRAMWriteSGX)
{
 (BaseRAM - (0xF8 * 8192))[A] = V;
}

static DECLFR(BaseRAMRead)
{
 return((BaseRAM - (0xF8 * 8192))[A]);
}

static DECLFR(BaseRAMRead_Mirrored)
{
 return(BaseRAM[A & 0x1FFF]);
}

static DECLFW(BaseRAMWrite)
{
 (BaseRAM - (0xF8 * 8192))[A] = V;
}

static DECLFW(BaseRAMWrite_Mirrored)
{
 BaseRAM[A & 0x1FFF] = V;
}

static DECLFR(IORead)
{
 #define IOREAD_SGX 0
 #include "ioread.inc"
 #undef IOREAD_SGX
}

static DECLFR(IOReadSGX)
{
 #define IOREAD_SGX 1
 #include "ioread.inc"
 #undef IOREAD_SGX
}

static DECLFW(IOWrite)
{
 A &= 0x1FFF;
  
 switch(A & 0x1c00)
 {
  case 0x0000: HuC6280_StealCycle();
	       VDC_Write(A, V);
	       break;
  case 0x0400: HuC6280_StealCycle();
	       VCE_Write(A, V);
	       break;

  case 0x0800: PCEIODataBuffer = V;
	       psg->Write(HuCPU.timestamp / pce_overclocked, A, V);
	       break;

  case 0x0c00: PCEIODataBuffer = V;
	       HuC6280_TimerWrite(A, V);
	       break;

  case 0x1000: PCEIODataBuffer = V; INPUT_Write(A, V); break;
  case 0x1400: PCEIODataBuffer = V; HuC6280_IRQStatusWrite(A, V); break;
  case 0x1800: if(IsTsushin)
                PCE_TsushinWrite(A, V);

	       if(!PCE_IsCD)
		break;

	       if((A & 0x1E00) == 0x1A00)
	       {
		if(arcade_card)
		 arcade_card->Write(A & 0x1FFF, V);
	       }
	       else
	        PCECD_Write(HuCPU.timestamp * 3, A, V); 
	       break;
  //case 0x1C00: break; // Expansion
  //default: printf("Eep: %04x\n", A); break;
 }

}

static void PCECDIRQCB(bool asserted)
{
 if(asserted)
  HuC6280_IRQBegin(MDFN_IQIRQ2);
 else
  HuC6280_IRQEnd(MDFN_IQIRQ2);
}

bool PCE_InitCD(void)
{
 PCECD_Settings cd_settings;
 memset(&cd_settings, 0, sizeof(PCECD_Settings));

 cd_settings.CDDA_Volume = (double)MDFN_GetSettingUI("pce_fast.cddavolume") / 100;
 cd_settings.CD_Speed = MDFN_GetSettingUI("pce_fast.cdspeed");

 cd_settings.ADPCM_Volume = (double)MDFN_GetSettingUI("pce_fast.adpcmvolume") / 100;
 cd_settings.ADPCM_LPF = MDFN_GetSettingB("pce_fast.adpcmlp");

 if(cd_settings.CDDA_Volume != 1.0)
  MDFN_printf(_("CD-DA Volume: %d%%\n"), (int)(100 * cd_settings.CDDA_Volume));

 if(cd_settings.ADPCM_Volume != 1.0)
  MDFN_printf(_("ADPCM Volume: %d%%\n"), (int)(100 * cd_settings.ADPCM_Volume));

 return(PCECD_Init(&cd_settings, PCECDIRQCB, PCE_MASTER_CLOCK, pce_overclocked, &sbuf[0], &sbuf[1]));
}


static int LoadCommon(void);
static void LoadCommonPre(void);

static bool TestMagic(const char *name, MDFNFILE *fp)
{
 if(memcmp(fp->data, "HESM", 4) && strcasecmp(fp->ext, "pce") && strcasecmp(fp->ext, "sgx"))
  return(FALSE);

 return(TRUE);
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

 for(int x = 0; x < 0x100; x++)
 {
  PCERead[x] = PCEBusRead;
  PCEWrite[x] = PCENullWrite;
 }

 uint32 crc = crc32(0, fp->data + headerlen, fp->size - headerlen);


 if(IsHES)
 {
  if(!PCE_HESLoad(fp->data, fp->size))
   return(0);
 }
 else
  HuCLoad(fp->data + headerlen, fp->size - headerlen, crc);

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
 pce_overclocked = MDFN_GetSettingUI("pce_fast.ocmultiplier");
 PCE_ACEnabled = MDFN_GetSettingB("pce_fast.arcadecard");

 if(pce_overclocked > 1)
  MDFN_printf(_("CPU overclock: %dx\n"), pce_overclocked);

 if(MDFN_GetSettingUI("pce_fast.cdspeed") > 1)
  MDFN_printf(_("CD-ROM speed:  %ux\n"), (unsigned int)MDFN_GetSettingUI("pce_fast.cdspeed"));

 memset(HuCPUFastMap, 0, sizeof(HuCPUFastMap));
 for(int x = 0; x < 0x100; x++)
 {
  PCERead[x] = PCEBusRead;
  PCEWrite[x] = PCENullWrite;
 }

 MDFNMP_Init(1024, (1 << 21) / 1024);
}

static int LoadCommon(void)
{ 
 IsSGX |= MDFN_GetSettingB("pce_fast.forcesgx") ? 1 : 0;

 if(IsHES)
  IsSGX = 1;
 // Don't modify IsSGX past this point.
 
 VDC_Init(IsSGX);

 if(IsSGX)
 {
  MDFN_printf("SuperGrafx Emulation Enabled.\n");
  PCERead[0xF8] = PCERead[0xF9] = PCERead[0xFA] = PCERead[0xFB] = BaseRAMReadSGX;
  PCEWrite[0xF8] = PCEWrite[0xF9] = PCEWrite[0xFA] = PCEWrite[0xFB] = BaseRAMWriteSGX;

  for(int x = 0xf8; x < 0xfb; x++)
   HuCPUFastMap[x] = BaseRAM - 0xf8 * 8192;

  PCERead[0xFF] = IOReadSGX;
 }
 else
 {
  PCERead[0xF8] = BaseRAMRead;
  PCERead[0xF9] = PCERead[0xFA] = PCERead[0xFB] = BaseRAMRead_Mirrored;

  PCEWrite[0xF8] = BaseRAMWrite;
  PCEWrite[0xF9] = PCEWrite[0xFA] = PCEWrite[0xFB] = BaseRAMWrite_Mirrored;

  for(int x = 0xf8; x < 0xfb; x++)
   HuCPUFastMap[x] = BaseRAM - x * 8192;

  PCERead[0xFF] = IORead;
 }

 MDFNMP_AddRAM(IsSGX ? 32768 : 8192, 0xf8 * 8192, BaseRAM);

 PCEWrite[0xFF] = IOWrite;

 HuC6280_Init();

 psg = new PCE_PSG(&sbuf[0], &sbuf[1], PCE_PSG::REVISION_ENHANCED);	//HUC6280A);

 psg->SetVolume(1.0);

 if(PCE_IsCD)
 {
  unsigned int cdpsgvolume = MDFN_GetSettingUI("pce_fast.cdpsgvolume");

  if(cdpsgvolume != 100)
  {
   MDFN_printf(_("CD PSG Volume: %d%%\n"), cdpsgvolume);
  }

  psg->SetVolume(0.678 * cdpsgvolume / 100);

 }

 PCEINPUT_Init();

 PCE_Power();

 MDFNGameInfo->LayerNames = IsSGX ? "BG0\0SPR0\0BG1\0SPR1\0" : "Background\0Sprites\0";
 MDFNGameInfo->fps = (uint32)((double)7159090.90909090 / 455 / 263 * 65536 * 256);

 // Clean this up:
 if(!MDFN_GetSettingB("pce_fast.correct_aspect"))
  MDFNGameInfo->fb_width = 682;

 if(!IsHES)
 {
  MDFNGameInfo->nominal_width = MDFN_GetSettingB("pce_fast.correct_aspect") ? 320 : 341;
  MDFNGameInfo->nominal_height = MDFN_GetSettingUI("pce_fast.slend") - MDFN_GetSettingUI("pce_fast.slstart") + 1;

  MDFNGameInfo->lcm_width = MDFN_GetSettingB("pce_fast.correct_aspect") ? 1024 : 341;
  MDFNGameInfo->lcm_height = MDFNGameInfo->nominal_height;
 }

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
 std::string bios_path = MDFN_MakeFName(MDFNMKF_FIRMWARE, 0, MDFN_GetSettingS("pce_fast.cdbios").c_str() );

 IsHES = 0;
 IsSGX = 0;

 LoadCommonPre();

 if(!HuCLoadCD(bios_path.c_str()))
  return(0);

 return(LoadCommon());
}


static void CloseGame(void)
{
 if(IsHES)
  HES_Close();
 else
 {
  HuCClose();
 }
 VDC_Close();
 if(psg)
 {
  delete psg;
  psg = NULL;
 }
}

static void Emulate(EmulateSpecStruct *espec)
{
 INPUT_Frame();

 MDFNMP_ApplyPeriodicCheats();

 if(espec->VideoFormatChanged)
  VDC_SetPixelFormat(espec->surface->format); //.Rshift, espec->surface->format.Gshift, espec->surface->format.Bshift);

 if(espec->SoundFormatChanged)
 {
  for(int y = 0; y < 2; y++)
  {
   sbuf[y].set_sample_rate(espec->SoundRate ? espec->SoundRate : 44100, 50);
   sbuf[y].clock_rate((long)(PCE_MASTER_CLOCK / 3));
   sbuf[y].bass_freq(20);
  }
 }
 VDC_RunFrame(espec->surface, &espec->DisplayRect, espec->LineWidths, IsHES ? 1 : espec->skip);


 if(PCE_IsCD)
  PCECD_Run(HuCPU.timestamp * 3);

 psg->EndFrame(HuCPU.timestamp / pce_overclocked);

 if(espec->SoundBuf)
 {
  for(int y = 0; y < 2; y++)
  {
   sbuf[y].end_frame(HuCPU.timestamp / pce_overclocked);
   espec->SoundBufSize = sbuf[y].read_samples(espec->SoundBuf + y, espec->SoundBufMaxSize, 1);
  }
 }

 espec->MasterCycles = HuCPU.timestamp * 3;

 INPUT_FixTS();

 HuC6280_ResetTS();

 if(PCE_IsCD)
  PCECD_ResetTS();

 if(IsHES && !espec->skip)
  HES_Draw(espec->surface, &espec->DisplayRect, espec->SoundBuf, espec->SoundBufSize);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(BaseRAM, IsSGX? 32768 : 8192),
  SFVAR(PCEIODataBuffer),
  SFEND
 };

 //for(int i = 8192; i < 32768; i++)
 // if(BaseRAM[i] != 0xFF)
 //  printf("%d %02x\n", i, BaseRAM[i]);

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN");

 ret &= HuC6280_StateAction(sm, load, data_only);
 ret &= VDC_StateAction(sm, load, data_only);
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

 if(!IsSGX)
  for(int i = 8192; i < 32768; i++)
   BaseRAM[i] = 0xFF;

 PCEIODataBuffer = 0xFF;

 if(IsHES)
  HES_Reset();

 HuC6280_Power();
 VDC_Power();
 psg->Power(HuCPU.timestamp / pce_overclocked);
 HuC_Power();

 if(PCE_IsCD)
  PCECD_Power(HuCPU.timestamp * 3);
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
/*
  { "pce_fast.input.port1", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 1."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
  { "pce_fast.input.port2", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 2."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
  { "pce_fast.input.port3", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 3."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
  { "pce_fast.input.port4", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 4."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
  { "pce_fast.input.port5", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Input device for input port 5."), NULL, MDFNST_STRING, "gamepad", NULL, NULL },
*/

  { "pce_fast.correct_aspect", MDFNSF_CAT_VIDEO, gettext_noop("Correct the aspect ratio."), NULL, MDFNST_BOOL, "1" },
  { "pce_fast.slstart", MDFNSF_NOFLAGS, gettext_noop("First rendered scanline."), NULL, MDFNST_UINT, "4", "0", "239" },
  { "pce_fast.slend", MDFNSF_NOFLAGS, gettext_noop("Last rendered scanline."), NULL, MDFNST_UINT, "235", "0", "239" },
  { "pce_fast.mouse_sensitivity", MDFNSF_NOFLAGS, gettext_noop("Mouse sensitivity."), NULL, MDFNST_FLOAT, "0.50", NULL, NULL, NULL, PCEINPUT_SettingChanged },
  { "pce_fast.disable_softreset", MDFNSF_NOFLAGS, gettext_noop("If set, when RUN+SEL are pressed simultaneously, disable both buttons temporarily."), NULL, MDFNST_BOOL, "0", NULL, NULL, NULL, PCEINPUT_SettingChanged },
  { "pce_fast.forcesgx", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Force SuperGrafx emulation."), NULL, MDFNST_BOOL, "0" },
  { "pce_fast.arcadecard", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable Arcade Card emulation."), NULL, MDFNST_BOOL, "1" },
  { "pce_fast.ocmultiplier", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("CPU overclock multiplier."), NULL, MDFNST_UINT, "1", "1", "100"},
  { "pce_fast.cdspeed", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("CD-ROM data transfer speed multiplier."), NULL, MDFNST_UINT, "1", "1", "100" },
  { "pce_fast.nospritelimit", MDFNSF_NOFLAGS, gettext_noop("Remove 16-sprites-per-scanline hardware limit."), NULL, MDFNST_BOOL, "0" },

  { "pce_fast.cdbios", MDFNSF_EMU_STATE, gettext_noop("Path to the CD BIOS"), NULL, MDFNST_STRING, "syscard3.pce" },
  { "pce_fast.adpcmlp", MDFNSF_NOFLAGS, gettext_noop("Enable lowpass filter dependent on playback-frequency."), NULL, MDFNST_BOOL, "0" },
  { "pce_fast.cdpsgvolume", MDFNSF_NOFLAGS, gettext_noop("PSG volume when playing a CD game."), NULL, MDFNST_UINT, "100", "0", "200" },
  { "pce_fast.cddavolume", MDFNSF_NOFLAGS, gettext_noop("CD-DA volume."), NULL, MDFNST_UINT, "100", "0", "200" },
  { "pce_fast.adpcmvolume", MDFNSF_NOFLAGS, gettext_noop("ADPCM volume."), NULL, MDFNST_UINT, "100", "0", "200" },
  { NULL }
};

static uint8 MemRead(uint32 addr)
{
 return(PCERead[(addr / 8192) & 0xFF](addr));
}

static const FileExtensionSpecStruct KnownExtensions[] =
{
 { ".pce", gettext_noop("PC Engine ROM Image") },
 { ".hes", gettext_noop("PC Engine Music Rip") },
 { ".sgx", gettext_noop("SuperGrafx ROM Image") },
 { NULL, NULL }
};

};

MDFNGI EmulatedPCE_Fast =
{
 "pce_fast",
 "PC Engine (CD)/TurboGrafx 16 (CD)/SuperGrafx",
 KnownExtensions,
 MODPRIO_INTERNAL_LOW,
 NULL,
 &PCEInputInfo,
 Load,
 TestMagic,
 LoadCD,
 TestMagicCD,
 CloseGame,
 VDC_ToggleLayer,
 NULL,
 NULL,
 NULL,
 MemRead,
 StateAction,
 Emulate,
 PCEINPUT_SetInput,
 DoSimpleCommand,
 PCESettings,
 MDFN_MASTERCLOCK_FIXED(PCE_MASTER_CLOCK),
 0,

 true,  // Multires possible?

 0,   // lcm_width
 0,   // lcm_height           
 NULL,  // Dummy

 320,   // Nominal width
 232,   // Nominal height

 512,	// Framebuffer width
 242,	// Framebuffer height

 2,     // Number of output sound channels
};

