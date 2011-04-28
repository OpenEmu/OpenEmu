/*
    Copyright (C) 1999, 2000, 2001, 2002, 2003  Charles MacDonald

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "shared.h"
#include "cart/cart.h"
#include "cd/cd.h"
#include "../md5.h"
#include "../general.h"
#include "../mempatcher.h"

namespace MDFN_IEN_MD
{

enum
{
 REGION_SAME = 1,
 REGION_GAME,
 REGION_OVERSEAS_NTSC,
 REGION_OVERSEAS_PAL,
 REGION_DOMESTIC_NTSC,
 REGION_DOMESTIC_PAL
};


int MD_HackyHackyMode = 0;
bool MD_IsCD;
static int32 z80_cycle_counter;
int32 md_timestamp;
static bool suspend68k = FALSE;

static bool run_cpu;

bool MD_OverClock = 0;

void MD_Suspend68K(bool state)
{
 suspend68k = state;
}

bool MD_Is68KSuspended(void)
{
 return(suspend68k);
}

static void system_init(bool overseas, bool PAL, bool overseas_reported, bool PAL_reported)
{
    gen_running = 1;

    z80_init();
    z80_readbyte = MD_Z80_ReadByte;
    z80_writebyte = MD_Z80_WriteByte;
    z80_readport = MD_Z80_ReadPort;
    z80_writeport = MD_Z80_WritePort;

    gen_init();
    MDIO_Init(overseas, PAL, overseas_reported, PAL_reported);
    MainVDP.SetSettings(PAL, PAL_reported, MDFN_GetSettingB("md.correct_aspect"));

#ifdef WANT_DEBUGGER
    MDDBG_Init();
#endif

}

static void system_reset(void)
{
 z80_cycle_counter = 0;

 if(MD_IsCD)
  MDCD_Reset();
 else
  MDCart_Reset();

 gen_reset();
 MainVDP.Reset();
 MDSound_Power();

 if(MD_IsCD)
  MDCD_Reset(); 
}

static void system_shutdown(void)
{
    gen_shutdown();
}

void MD_ExitCPULoop(void)
{
 run_cpu = FALSE;
}

static int system_frame(int do_skip)
{
 run_cpu = TRUE;

 while(run_cpu > 0)
 {
  int32 temp_master;
  int32 max_md_timestamp;

  if(suspend68k)
  {
   temp_master = 8 * 7;
  }
  else
  {
   #ifdef WANT_DEBUGGER
   if(MD_DebugMode)
    MDDBG_CPUHook();
   #endif
   temp_master = C68k_Exec(&Main68K) * 7;
  }
  max_md_timestamp = md_timestamp + temp_master;

  if(zreset == 1 && zbusreq == 0)
  {
   z80_cycle_counter += temp_master;

   while(z80_cycle_counter > 0)
   {
    int32 z80_temp = z80_do_opcode() * 15;

    z80_cycle_counter -= z80_temp;
    md_timestamp += z80_temp;

    if(md_timestamp > max_md_timestamp)
    {
     //printf("Meow: %d\n", md_timestamp - max_md_timestamp);
     md_timestamp = max_md_timestamp;
    }
    MainVDP.Run();
   }
  }
  md_timestamp = max_md_timestamp;
  MainVDP.Run();

  if(MD_IsCD)
   MDCD_Run(temp_master);
 }
 return gen_running;
}

static void Emulate(EmulateSpecStruct *espec)
{
 MDFNMP_ApplyPeriodicCheats();

 MDINPUT_Frame();

 if(espec->VideoFormatChanged)
  MainVDP.SetPixelFormat(espec->surface->format); //.Rshift, espec->surface->format.Gshift, espec->surface->format.Bshift);

 if(espec->SoundFormatChanged)
  MDSound_SetSoundRate(espec->SoundRate);

 MainVDP.SetSurface(espec->surface, &espec->DisplayRect);

 system_frame(0);

 espec->MasterCycles = md_timestamp;

 espec->SoundBufSize = MDSound_Flush(espec->SoundBuf, espec->SoundBufMaxSize);

 md_timestamp = 0;
 MainVDP.ResetTS();

 //MainVDP.SetSurface(NULL);
}

static void CloseGame(void)
{
 MDCart_Close();
}

static bool decode_region_setting(const int setting, bool &overseas, bool &pal)
{
 switch(setting)
 {
  default: assert(0);
	   return(false);

  case REGION_OVERSEAS_NTSC:
	overseas = TRUE;
	pal = FALSE;
	return(TRUE);

  case REGION_OVERSEAS_PAL:
	overseas = TRUE;
	pal = TRUE;
	return(TRUE);

  case REGION_DOMESTIC_NTSC:
	overseas = FALSE;
	pal = FALSE;
	return(TRUE);

  case REGION_DOMESTIC_PAL:
	overseas = FALSE;
	pal = TRUE;
	return(TRUE);
 }
}

static int LoadCommonPost(const md_game_info &ginfo);
static int LoadCommonPost(const md_game_info &ginfo)
{
 MDFN_printf(_("ROM:       %dKiB\n"), (ginfo.rom_size + 1023) / 1024);
 MDFN_printf(_("ROM MD5:   0x%s\n"), md5_context::asciistr(ginfo.md5, 0).c_str());
 MDFN_printf(_("Header MD5: 0x%s\n"), md5_context::asciistr(ginfo.info_header_md5, 0).c_str());
 MDFN_printf(_("Product Code: %s\n"), ginfo.product_code);
 MDFN_printf(_("Domestic name: %s\n"), ginfo.domestic_name); // TODO: Character set conversion(shift_jis -> utf-8)
 MDFN_printf(_("Overseas name: %s\n"), ginfo.overseas_name);
 MDFN_printf(_("Copyright: %s\n"), ginfo.copyright);
 if(ginfo.checksum == ginfo.checksum_real)
  MDFN_printf(_("Checksum:  0x%04x\n"), ginfo.checksum);
 else
  MDFN_printf(_("Checksum:  0x%04x\n Warning: calculated checksum(0x%04x) does not match\n"), ginfo.checksum, ginfo.checksum_real);

 MDFN_printf(_("Supported I/O devices:\n"));
 MDFN_indent(1);
 for(unsigned int iot = 0; iot < sizeof(IO_types) / sizeof(IO_type_t); iot++)
 {
  if(ginfo.io_support & (1 << IO_types[iot].id))
   MDFN_printf(_("%s\n"), _(IO_types[iot].name));
 }
 MDFN_indent(-1);

 MDFNMP_Init(8192, (1 << 24) / 8192);
 MDFNMP_AddRAM(65536, 0x7 << 21, work_ram);

 MDFNGameInfo->GameSetMD5Valid = FALSE;

 MDSound_Init();

 MDFN_printf(_("Supported regions:\n"));
 MDFN_indent(1);
 if(ginfo.region_support & REGIONMASK_JAPAN_NTSC)
  MDFN_printf(_("Japan/Domestic NTSC\n"));
 if(ginfo.region_support & REGIONMASK_JAPAN_PAL)
  MDFN_printf(_("Japan/Domestic PAL\n"));
 if(ginfo.region_support & REGIONMASK_OVERSEAS_NTSC)
  MDFN_printf(_("Overseas NTSC\n"));
 if(ginfo.region_support & REGIONMASK_OVERSEAS_PAL)
  MDFN_printf(_("Overseas PAL\n"));
 MDFN_indent(-1);

 {
  const int region_setting = MDFN_GetSettingI("md.region");
  const int reported_region_setting = MDFN_GetSettingI("md.reported_region");

  // Default, in case the game doesn't support any regions!
  bool game_overseas = TRUE;
  bool game_pal = FALSE;
  bool overseas;
  bool pal;
  bool overseas_reported;
  bool pal_reported;

  // Preference order, TODO:  Make it configurable
  if(ginfo.region_support & REGIONMASK_OVERSEAS_NTSC)
  {
   game_overseas = TRUE;
   game_pal = FALSE;
  }
  else if(ginfo.region_support & REGIONMASK_OVERSEAS_PAL)
  {
   game_overseas = TRUE;
   game_pal = TRUE;
  }
  else if(ginfo.region_support & REGIONMASK_JAPAN_NTSC)
  {
   game_overseas = FALSE;
   game_pal = FALSE;
  }
  else if(ginfo.region_support & REGIONMASK_JAPAN_PAL) // WTF?
  {
   game_overseas = FALSE;
   game_pal = TRUE;
  }
 
  if(region_setting == REGION_GAME)
  {
   overseas = game_overseas;
   pal = game_pal;
  }
  else
  {
   decode_region_setting(region_setting, overseas, pal);
  }

  if(reported_region_setting == REGION_GAME)
  {
   overseas_reported = game_overseas;
   pal_reported = game_pal;
  }
  else if(reported_region_setting == REGION_SAME)
  {
   overseas_reported = overseas;
   pal_reported = pal;   
  }
  else
  {
   decode_region_setting(reported_region_setting, overseas_reported, pal_reported);
  }

  MDFN_printf("\n");
  MDFN_printf(_("Active Region: %s %s\n"), overseas ? _("Overseas") : _("Domestic"), pal ? _("PAL") : _("NTSC"));
  MDFN_printf(_("Active Region Reported: %s %s\n"), overseas_reported ? _("Overseas") : _("Domestic"), pal_reported ? _("PAL") : _("NTSC"));

  system_init(overseas, pal, overseas_reported, pal_reported);

  if(pal)
   MDFNGameInfo->nominal_height = 240;
  else
   MDFNGameInfo->nominal_height = 224;

  MDFNGameInfo->MasterClock = MDFN_MASTERCLOCK_FIXED(pal ? CLOCK_PAL : CLOCK_NTSC);

  if(pal)
   MDFNGameInfo->fps = (int64)CLOCK_PAL * 65536 * 256 / (313 * 3420);
  else
   MDFNGameInfo->fps = (int64)CLOCK_NTSC * 65536 * 256 / (262 * 3420);
 }

 if(MDFN_GetSettingB("md.correct_aspect"))
 {
  MDFNGameInfo->nominal_width = 288;
  MDFNGameInfo->lcm_width = 1280;
 }
 else
 {
  MDFNGameInfo->nominal_width = 320;
  MDFNGameInfo->lcm_width = 320;
 }

 MDFNGameInfo->lcm_height = MDFNGameInfo->nominal_height * 2;

 MDFNGameInfo->LayerNames = "BG0\0BG1\0OBJ\0";

 system_reset();

 return(1);
}

static int Load(const char *name, MDFNFILE *fp)
{
 md_game_info ginfo;
 int ret;

 memset(&ginfo, 0, sizeof(md_game_info));
 ret = MDCart_Load(&ginfo, name, fp);
 if(ret <= 0)
  return(ret);

 memcpy(MDFNGameInfo->MD5, ginfo.md5, 16);

 MD_IsCD = FALSE;

 MD_ExtRead8 = MDCart_Read8;
 MD_ExtRead16 = MDCart_Read16;
 MD_ExtWrite8 = MDCart_Write8;
 MD_ExtWrite16 = MDCart_Write16;

 MDCart_LoadNV();

 if(!LoadCommonPost(ginfo))
  return(0);

 return(1);
}

static int LoadCD(void)
{
 md_game_info ginfo;

 memset(&ginfo, 0, sizeof(md_game_info));

 MD_IsCD = TRUE;

 if(!MDCD_Load(&ginfo))
 {
  puts("BOOM");
  return(FALSE);
 }

 memcpy(MDFNGameInfo->MD5, ginfo.md5, 16);

 if(!LoadCommonPost(ginfo))
  return(0);

 return(TRUE);
}

static bool TestMagicCD(void)
{
 return(MDCD_TestMagic());
}

static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
  case MDFN_MSC_POWER:
  case MDFN_MSC_RESET: system_reset(); break;
 }
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 int ret = 1;
 unsigned int c68k_state_len = C68k_Get_State_Max_Len();
 uint8 c68k_state[c68k_state_len];

 C68k_Save_State(&Main68K, c68k_state);

 SFORMAT StateRegs[] =
 {
  SFARRAY(work_ram, 65536),
  SFARRAY(zram, 8192),
  SFVAR(zbusreq),
  SFVAR(zreset),
  SFVAR(zbusack),
  SFVAR(zirq),
  SFVAR(zbank),

  SFVAR(md_timestamp),
  SFVAR(suspend68k),
  SFVAR(z80_cycle_counter),

  SFARRAY(c68k_state, c68k_state_len),
  SFEND
 };


 ret &= MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN");
 ret &= z80_state_action(sm, load, data_only, "Z80");
 ret &= MDINPUT_StateAction(sm, load, data_only);
 ret &= MainVDP.StateAction(sm, load, data_only);
 ret &= MDSound_StateAction(sm, load, data_only);
 ret &= MDCart_StateAction(sm, load, data_only);

 if(load)
 {
  C68k_Load_State(&Main68K, c68k_state);
  z80_set_interrupt(zirq);
 }

 return(ret);
}

static const MDFNSetting_EnumList RegionList[] =
{
 { "game", REGION_GAME, gettext_noop("Match game's header."), gettext_noop("Emulate the region that the game indicates it expects to run in via data in the header(or in an internal database for a few games that may have bad header data).") },

 { "overseas_ntsc", REGION_OVERSEAS_NTSC, gettext_noop("Overseas(non-Japan), NTSC"), gettext_noop("Region used in North America.") },
 { "overseas_pal", REGION_OVERSEAS_PAL, gettext_noop("Overseas(non-Japan), PAL"), gettext_noop("Region used in Europe.") },

 { "domestic_ntsc", REGION_DOMESTIC_NTSC, gettext_noop("Domestic(Japan), NTSC"), gettext_noop("Region used in Japan.") },
 { "domestic_pal", REGION_DOMESTIC_PAL, gettext_noop("Domestic(Japan), PAL"), gettext_noop("Probably an invalid region, but available for testing purposes anyway.") },

 { NULL, 0 }
};

static const MDFNSetting_EnumList ReportedRegionList[] =
{
 { "same", REGION_SAME, gettext_noop("Match the region emulated.") },

 { "game", REGION_GAME, gettext_noop("Match game's header."), gettext_noop("This option, in conjunction with the \"md.region\" setting, can be used to run all games at NTSC speeds, or all games at PAL speeds.")  },

 { "overseas_ntsc", REGION_OVERSEAS_NTSC, gettext_noop("Overseas(non-Japan), NTSC"), gettext_noop("Region used in North America.") },
 { "overseas_pal", REGION_OVERSEAS_PAL, gettext_noop("Overseas(non-Japan), PAL"), gettext_noop("Region used in Europe.") },

 { "domestic_ntsc", REGION_DOMESTIC_NTSC, gettext_noop("Domestic(Japan), NTSC"), gettext_noop("Region used in Japan.") },
 { "domestic_pal", REGION_DOMESTIC_PAL, gettext_noop("Domestic(Japan), PAL"), gettext_noop("Probably an invalid region, but available for testing purposes anyway.") },

 { NULL, 0 },
};

static MDFNSetting MDSettings[] =
{
 { "md.region", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Emulate the specified region's Genesis/MegaDrive"), NULL, MDFNST_ENUM, "game", NULL, NULL, NULL, NULL, RegionList },
 { "md.reported_region", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Region reported to the game."), NULL, MDFNST_ENUM, "same", NULL, NULL, NULL, NULL, ReportedRegionList },

 { "md.cdbios", MDFNSF_EMU_STATE, gettext_noop("Path to the CD BIOS"), gettext_noop("SegaCD/MegaCD emulation is currently nonfunctional."), MDFNST_STRING, "us_scd1_9210.bin" },

 { "md.correct_aspect", MDFNSF_CAT_VIDEO, gettext_noop("Correct the aspect ratio."), NULL, MDFNST_BOOL, "1" },

 // off, teamplay, 4way, auto
 // { "md.input.4player", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE | MDFNSF_CAT_INPUT, gettext_noop("Which 4-player adapter to use."), NULL, MDFNST_STRING, "auto", NULL, NULL },

 { NULL }
};

static const FileExtensionSpecStruct KnownExtensions[] =
{
 { ".bin", gettext_noop("Super Magic Drive binary ROM Image") },
 { ".smd", gettext_noop("Super Magic Drive interleaved format ROM Image") },
 { ".md", gettext_noop("Multi Game Doctor format ROM Image") },
 { NULL, NULL }
};

bool ToggleLayer(int which)
{
 return(MainVDP.ToggleLayer(which));
}

}


MDFNGI EmulatedMD =
{
 "md",
 "Sega Genesis/MegaDrive",
 KnownExtensions,
 MODPRIO_INTERNAL_HIGH,
 #ifdef WANT_DEBUGGER
 &DBGInfo,
 #else
 NULL,
 #endif
 &MDInputInfo,
 Load,
 MDCart_TestMagic,
 LoadCD,
 TestMagicCD,
 CloseGame,
 ToggleLayer,
 NULL,
 NULL, //InstallReadPatch,
 NULL, //RemoveReadPatches,
 NULL, //MemRead,
 StateAction,
 Emulate,
 MDINPUT_SetInput,
 DoSimpleCommand,
 MDSettings,
 0,	// MasterClock(set in game loading code)
 0,
 TRUE, // Multires possible?

 0,   // lcm_width		// Calculated in game load
 0,   // lcm_height         	// Calculated in game load
 NULL,  // Dummy


 // We want maximum values for nominal width and height here so the automatic fullscreen setting generation code will have
 // selected a setting suitable if aspect ratio correction is turned off.
 320,   // Nominal width(adjusted in game loading code, with aspect ratio correction enabled, it's 288, otherwise 320)
 240,   // Nominal height(adjusted in game loading code to 224 for NTSC, and 240 for PAL)
 1024,	// Framebuffer width
 512,	// Framebuffer height

 2,     // Number of output sound channels
};

