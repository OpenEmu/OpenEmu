/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2003 Xodnizel
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

#include	"nes.h"
#include        <stdarg.h>

#include	"x6502.h"
#include	"ppu/ppu.h"
#include	"ppu/palette.h"
#include	"sound.h"

#include	"cart.h"
#include	"nsf.h"
#include	"fds.h"
#include	"ines.h"
#include	"unif.h"
#include        "../mempatcher.h"
#include	"input.h"
#include	"vsuni.h"
#include	"debug.h"


extern MDFNGI EmulatedNES;

uint64 timestampbase;

// Accessed in debug.cpp
NESGameType *GameInterface = NULL;

static readfunc NonCheatARead[0x10000 + 0x100];
readfunc ARead[0x10000 + 0x100];
writefunc BWrite[0x10000 + 0x100];

bool NESIsVSUni;

//static
DECLFW(BNull)
{
 //printf("Null Write: %04x %02x\n", A, V);
}

//static 
DECLFR(ANull)
{
 //printf("Null Read: %04x\n", A);
 return(X.DB);
}

readfunc GetReadHandler(int32 a)
{
	return ARead[a];
}

void SetReadHandler(int32 start, int32 end, readfunc func, bool snc)
{
  int32 x;

  //printf("%08x %08x %lld %d\n", start, end, func, snc);

  if(!func)
   func=ANull;

  for(x=end;x>=start;x--)
  {
   ARead[x]=func;
   if(snc)
    NonCheatARead[x] = func;
  }
}

writefunc GetWriteHandler(int32 a)
{
	return BWrite[a];
}

void SetWriteHandler(int32 start, int32 end, writefunc func)
{
  int32 x;

  if(!func)
   func=BNull;

  for(x=end;x>=start;x--)
   BWrite[x]=func;
}

uint8 RAM[0x800];
uint8 PAL=0;

static DECLFW(BRAML)
{  
        RAM[A]=V;
}

static DECLFW(BRAMH)
{
        RAM[A&0x7FF]=V;
}

static DECLFR(ARAML)
{
        return RAM[A];
}

static DECLFR(ARAMH)
{
        return RAM[A&0x7FF];
}

// We need to look up the correct function in the ARead[] and BWrite[] tables
// for these overflow functions, because the RAM access handlers might be hooked
// for cheats or other things.

static DECLFR(AOverflow)
{
	A &= 0xFFFF;
	X.PC &= 0xFFFF;
	return(ARead[A](A));
}

static DECLFW(BOverflow)
{
	A &= 0xFFFF;
	return(BWrite[A](A, V));
}

static void CloseGame(void)
{
 for(std::vector<EXPSOUND>::iterator ep = GameExpSound.begin(); ep != GameExpSound.end(); ep++)
  if(ep->Kill)
   ep->Kill();
 GameExpSound.clear();

 if(GameInterface)
 {
  if(GameInterface->Close)
   GameInterface->Close();
  free(GameInterface);
  GameInterface = NULL;
 }

 Genie_Kill();
 MDFNSND_Close();
}

static void InitCommon(const char *name)
{
        NESIsVSUni = FALSE;
        PPU_hook = 0;
        GameHBIRQHook = 0;

        MapIRQHook = 0;
        MMC5Hack = 0;
        PAL &= 1;

        MDFNGameInfo->GameType = GMT_CART;
        MDFNGameInfo->VideoSystem = VIDSYS_NONE;

        MDFNGameInfo->cspecial = NULL;
        MDFNGameInfo->GameSetMD5Valid = FALSE;


        if(MDFN_GetSettingB("nes.fnscan"))
        {
	 const char *fn = GetFNComponent(name);	// TODO: This seems awfully spaghetti-like/inefficient.  Fixme?

         if(strstr(fn, "(U)") || strstr(fn, "(USA)"))
          MDFNGameInfo->VideoSystem = VIDSYS_NTSC;
         else if(strstr(fn, "(J)") || strstr(fn, "(Japan)"))
          MDFNGameInfo->VideoSystem = VIDSYS_NTSC;
         else if(strstr(fn, "(E)") || strstr(fn, "(G)") || strstr(fn, "(Europe)") || strstr(fn, "(Germany)") )
          MDFNGameInfo->VideoSystem = VIDSYS_PAL;
        }

        GameInterface = (NESGameType *)calloc(1, sizeof(NESGameType));

        SetReadHandler(0x0000, 0xFFFF, ANull);
        SetWriteHandler(0x0000, 0xFFFF, BNull);

        SetReadHandler(0x10000,0x10000 + 0xFF, AOverflow);
        SetWriteHandler(0x10000,0x10000 + 0xFF, BOverflow);

        SetReadHandler(0,0x7FF,ARAML);
        SetWriteHandler(0,0x7FF,BRAML);

        SetReadHandler(0x800,0x1FFF,ARAMH);  /* Part of a little */
        SetWriteHandler(0x800,0x1FFF,BRAMH); /* hack for a small speed boost. */

        MDFNMP_Init(1024, 65536 / 1024);


	#ifdef WANT_DEBUGGER
	NESDBG_Init();

        ASpace_Add(NESDBG_GetAddressSpaceBytes, NESDBG_PutAddressSpaceBytes, "cpu", "CPU", 16);
        ASpace_Add(NESPPU_GetAddressSpaceBytes, NESPPU_PutAddressSpaceBytes, "ppu", "PPU", 14);
        #endif
}

bool UNIFLoad(const char *name, MDFNFILE *fp, NESGameType *);
bool iNESLoad(const char *name, MDFNFILE *fp, NESGameType *);
bool FDSLoad(const char *name, MDFNFILE *fp, NESGameType *);
bool NSFLoad(const char *name, MDFNFILE *fp, NESGameType *);

bool iNES_TestMagic(const char *name, MDFNFILE *fp);
bool UNIF_TestMagic(const char *name, MDFNFILE *fp);
bool FDS_TestMagic(const char *name, MDFNFILE *fp);
bool NSF_TestMagic(const char *name, MDFNFILE *fp);

typedef bool (*LoadFunction_t)(const char *name, MDFNFILE *fp, NESGameType *);

static LoadFunction_t GetLoadFunctionByMagic(const char *name, MDFNFILE *fp)
{
 bool (*MagicFunctions[4])(const char *name, MDFNFILE *fp) = { iNES_TestMagic, UNIF_TestMagic, NSF_TestMagic, FDS_TestMagic };
 LoadFunction_t LoadFunctions[4] = { iNESLoad, UNIFLoad, NSFLoad, FDSLoad };
 LoadFunction_t ret = NULL;

 for(int x = 0; x < 4; x++)
 {
  if(MagicFunctions[x](name, fp))
  {
   ret = LoadFunctions[x];
   break;
  }
 }
 return(ret);
}

static bool TestMagic(const char *name, MDFNFILE *fp)
{
 return(GetLoadFunctionByMagic(name, fp) != NULL);
}

static int Load(const char *name, MDFNFILE *fp)
{
	LoadFunction_t LoadFunction = NULL;

	LoadFunction = GetLoadFunctionByMagic(name, fp);

	// If the file type isn't recognized, return -1!
	if(!LoadFunction)
	 return(-1);

	InitCommon(name);

	if(!LoadFunction(name, fp, GameInterface))
	{
	 free(GameInterface);
	 GameInterface = NULL;
	 return(0);
	}

	{
	 int w;

	 if(MDFNGameInfo->VideoSystem == VIDSYS_NTSC)
	  w = 0;
	 else if(MDFNGameInfo->VideoSystem == VIDSYS_PAL)
	  w = 1;
	 else
	 {
	  w = MDFN_GetSettingB("nes.pal");
	  MDFNGameInfo->VideoSystem = w ? VIDSYS_PAL : VIDSYS_NTSC;
	 }
	 PAL=w?1:0;
	 MDFNGameInfo->fps = PAL? 838977920 : 1008307711;
	 MDFNGameInfo->MasterClock = MDFN_MASTERCLOCK_FIXED(PAL ? PAL_CPU : NTSC_CPU);
	}

	X6502_Init();
	MDFNPPU_Init();
        MDFNSND_Init(PAL);
	NESINPUT_Init();

	if(NESIsVSUni)
	 MDFN_VSUniInstallRWHooks();


	if(MDFNGameInfo->GameType != GMT_PLAYER)
         if(MDFN_GetSettingB("nes.gg"))
	  Genie_Init();

        PowerNES();

        MDFN_InitPalette(NESIsVSUni ? MDFN_VSUniGetPaletteNum() : 0);

        return(1);
}

static void Emulate(EmulateSpecStruct *espec)
{
 int ssize;

 if(espec->VideoFormatChanged)
  MDFNNES_SetPixelFormat(espec->surface->format); //.Rshift, espec->surface->format.Gshift, espec->surface->format.Bshift);

 if(espec->SoundFormatChanged)
  MDFNNES_SetSoundRate(espec->SoundRate);

 NESPPU_GetDisplayRect(&espec->DisplayRect);

 MDFN_UpdateInput();

 if(!Genie_BIOSInstalled())
  MDFNMP_ApplyPeriodicCheats();

 MDFNPPU_Loop(espec->surface, espec->skip);

 ssize = FlushEmulateSound(espec->NeedSoundReverse, espec->SoundBuf, espec->SoundBufMaxSize);
 espec->NeedSoundReverse = 0;

 timestampbase += timestamp;
 espec->MasterCycles = timestamp;

 timestamp = 0;

 if(MDFNGameInfo->GameType == GMT_PLAYER)
  MDFNNES_DrawNSF(espec->surface, &espec->DisplayRect, espec->SoundBuf, ssize);

 espec->SoundBufSize = ssize;


 if(MDFNGameInfo->GameType != GMT_PLAYER)
 {
  if(NESIsVSUni)
   MDFN_VSUniDraw(espec->surface);
 }
 MDFN_DrawInput(espec->surface);
}

void ResetNES(void)
{
        MDFNMOV_AddCommand(MDFN_MSC_RESET);
	if(GameInterface->Reset)
         GameInterface->Reset();
        MDFNSND_Reset();
        MDFNPPU_Reset();
        X6502_Reset();
}

void PowerNES(void) 
{
        MDFNMOV_AddCommand(MDFN_MSC_POWER);

        if(!MDFNGameInfo)
	 return;

	if(!Genie_BIOSInstalled())
 	 MDFNMP_RemoveReadPatches();

	MDFNMP_AddRAM(0x0800, 0x0000, RAM);

	// Genie_Power() will remove any cheat read patches, and then install the BIOS(and its read hooks)
        Genie_Power();

	// http://wiki.nesdev.com/w/index.php/CPU_power_up_state
	memset(RAM, 0xFF, 0x800);
	RAM[0x008] = 0xF7;
	RAM[0x009] = 0xEF;
	RAM[0x00A] = 0xDF;
	RAM[0x00F] = 0xBF;

        NESINPUT_Power();
        MDFNSND_Power();
        MDFNPPU_Power();

	/* Have the external game hardware "powered" after the internal NES stuff.  
	   Needed for the NSF code and VS System code.
	*/
	if(GameInterface->Power)
	 GameInterface->Power();

	if(NESIsVSUni)
         MDFN_VSUniPower();

	timestampbase = 0;
	X6502_Power();

        if(!Genie_BIOSInstalled())
         MDFNMP_InstallReadPatches();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 if(Genie_BIOSInstalled())
 {
  if(!data_only)
   MDFN_DispMessage(_("Cannot use states in GG Screen."));
  return(0);
 }

 if(!X6502_StateAction(sm, load, data_only))
  return(0);

 if(!MDFNPPU_StateAction(sm, load, data_only))
  return(0);

 if(!MDFNSND_StateAction(sm, load, data_only))
  return(0);

 if(!load || load >= 0x0500)
 {
  if(!NESINPUT_StateAction(sm, load, data_only))
   return(0);
 }

 if(GameInterface->StateAction)
 {
  if(!GameInterface->StateAction(sm, load, data_only))
   return(0);
 }

 return(1);
}

// TODO: Actual enum vals
static const MDFNSetting_EnumList NTSCPresetList[] =
{
 { "disabled",	-1, gettext_noop("Disabled") },
 { "none",	-1 }, // Old setting value

 { "composite", -1, gettext_noop("Composite Video") },
 { "svideo", 	-1, gettext_noop("S-Video") },
 { "rgb", 	-1, gettext_noop("RGB") },
 { "monochrome", -1, gettext_noop("Monochrome") },

 { NULL, 0 },
};

static MDFNSetting NESSettings[] =
{
  { "nes.nofs", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Disable four-score emulation."), NULL, MDFNST_BOOL, "0" },

  { "nes.no8lim", MDFNSF_NOFLAGS, gettext_noop("Remove 8-sprites-per-scanline hardware limit."), 
	gettext_noop("WARNING: Enabling this option will cause graphical glitches in some games, including \"Solstice\"."), MDFNST_BOOL, "0", NULL, NULL, NULL, NESPPU_SettingChanged },

  { "nes.soundq", MDFNSF_NOFLAGS, gettext_noop("Sound quality."), NULL, MDFNST_INT, "0", "-2", "3" },
  { "nes.sound_rate_error", MDFNSF_NOFLAGS, gettext_noop("Output rate tolerance."), NULL, MDFNST_FLOAT, "0.00004", "0.0000001", "0.01" },
  { "nes.n106bs", MDFNSF_NOFLAGS, gettext_noop("Enable less-accurate, but better sounding, Namco 106(mapper 19) sound emulation."), NULL, MDFNST_BOOL, "0" },
  { "nes.fnscan", MDFNSF_EMU_STATE, gettext_noop("Scan filename for (U),(J),(E),etc. strings to en/dis-able PAL emulation."), 
	gettext_noop("Warning: This option may break NES network play when enabled IF the players are using ROM images with different filenames."), MDFNST_BOOL, "1" },

  { "nes.pal", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable PAL(50Hz) NES emulation."), NULL, MDFNST_BOOL, "0" },
  { "nes.gg", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable Game Genie emulation."), NULL, MDFNST_BOOL, "0" },
  { "nes.ggrom", MDFNSF_EMU_STATE, gettext_noop("Path to Game Genie ROM image."), NULL, MDFNST_STRING, "gg.rom" },
  { "nes.clipsides", MDFNSF_NOFLAGS, gettext_noop("Clip left+right 8 pixel columns."), NULL, MDFNST_BOOL, "0" },
  { "nes.slstart", MDFNSF_NOFLAGS, gettext_noop("First displayed scanline in NTSC mode."), NULL, MDFNST_UINT, "8", "0", "239" },
  { "nes.slend", MDFNSF_NOFLAGS, gettext_noop("Last displayed scanlines in NTSC mode."), NULL, MDFNST_UINT, "231", "0", "239" },
  { "nes.slstartp", MDFNSF_NOFLAGS, gettext_noop("First displayed scanline in PAL mode."), NULL, MDFNST_UINT, "0", "0", "239" },
  { "nes.slendp", MDFNSF_NOFLAGS, gettext_noop("Last displayedscanlines in PAL mode."), NULL, MDFNST_UINT, "239", "0", "239" },
  { "nes.ntscblitter", MDFNSF_NOFLAGS, gettext_noop("Enable NTSC color generation and blitter."), 
	gettext_noop("NOTE: If your refresh rate isn't very close to 60.1Hz(+-0.1), you will need to set the nes.ntsc.mergefields setting to \"1\" to avoid excessive flickering."), MDFNST_BOOL, "0" },


  { "nes.ntsc.preset", MDFNSF_NOFLAGS, gettext_noop("Video quality/type preset."), NULL, MDFNST_ENUM, "none", NULL, NULL, NULL, NULL, NTSCPresetList },
  { "nes.ntsc.mergefields", MDFNSF_NOFLAGS, gettext_noop("Merge fields to partially work around !=60.1Hz refresh rates."), NULL, MDFNST_BOOL, "0" },
  { "nes.ntsc.saturation", MDFNSF_NOFLAGS, gettext_noop("NTSC composite blitter saturation."), NULL, MDFNST_FLOAT, "0", "-1", "1" },
  { "nes.ntsc.hue", MDFNSF_NOFLAGS, gettext_noop("NTSC composite blitter hue."), NULL, MDFNST_FLOAT, "0", "-1", "1" },
  { "nes.ntsc.sharpness", MDFNSF_NOFLAGS, gettext_noop("NTSC composite blitter sharpness."), NULL, MDFNST_FLOAT, "0", "-1", "1" },
  { "nes.ntsc.brightness", MDFNSF_NOFLAGS, gettext_noop("NTSC composite blitter brightness."), NULL, MDFNST_FLOAT, "0", "-1", "1" },
  { "nes.ntsc.contrast", MDFNSF_NOFLAGS, gettext_noop("NTSC composite blitter contrast."), NULL, MDFNST_FLOAT, "0", "-1", "1" },

  { "nes.ntsc.matrix", MDFNSF_NOFLAGS, gettext_noop("Enable NTSC custom decoder matrix."), NULL, MDFNST_BOOL, "0" },

  /* Default custom decoder matrix(not plain default matrix) is from Sony */
  { "nes.ntsc.matrix.0", MDFNSF_NOFLAGS, gettext_noop("NTSC custom decoder matrix element 0(red, value * V)."), NULL, MDFNST_FLOAT, "1.539", "-2", "2", NULL, NESPPU_SettingChanged },
  { "nes.ntsc.matrix.1", MDFNSF_NOFLAGS, gettext_noop("NTSC custom decoder matrix element 1(red, value * U)."), NULL, MDFNST_FLOAT, "-0.622", "-2", "2", NULL, NESPPU_SettingChanged },
  { "nes.ntsc.matrix.2", MDFNSF_NOFLAGS, gettext_noop("NTSC custom decoder matrix element 2(green, value * V)."), NULL, MDFNST_FLOAT, "-0.571", "-2", "2", NULL, NESPPU_SettingChanged },
  { "nes.ntsc.matrix.3", MDFNSF_NOFLAGS, gettext_noop("NTSC custom decoder matrix element 3(green, value * U)."), NULL, MDFNST_FLOAT, "-0.185", "-2", "2", NULL, NESPPU_SettingChanged },
  { "nes.ntsc.matrix.4", MDFNSF_NOFLAGS, gettext_noop("NTSC custom decoder matrix element 4(blue, value * V)."), NULL, MDFNST_FLOAT, "0.000", "-2", "2", NULL, NESPPU_SettingChanged },
  { "nes.ntsc.matrix.5", MDFNSF_NOFLAGS, gettext_noop("NTSC custom decoder matrix element 5(blue, value * U."), NULL, MDFNST_FLOAT, "2.000", "-2", "2", NULL, NESPPU_SettingChanged },
  { NULL }
};

static uint8 MemRead(uint32 addr)
{
 addr &= 0xFFFF;

 return(NonCheatARead[addr](addr));
}

static DECLFR(CheatReadFunc)
{
  std::vector<SUBCHEAT>::iterator chit;
  //printf("%08x, %d\n", A, NonCheatARead[A]);
  uint8 retval = NonCheatARead[A](A);

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

static void InstallReadPatch(uint32 address)
{
 if(Genie_BIOSInstalled())
  return;

 address &= 0xFFFF;

 SetReadHandler(address, address, CheatReadFunc, 0);
}

static void RemoveReadPatches(void)
{
 if(Genie_BIOSInstalled())
  return;

 for(uint32 A = 0; A <= 0xFFFF; A++)
 {
  SetReadHandler(A, A, NonCheatARead[A], 0);
 }
}

static const FileExtensionSpecStruct KnownExtensions[] =
{
 { ".nes", "iNES Format ROM Image" },
 { ".nez", "iNES Format ROM Image" }, // Odd naming variant
 { ".fds", "Famicom Disk System Disk Image" },
 { ".nsf", "Nintendo Sound Format" },
 { ".nsfe", "Extended Nintendo Sound Format" },
 { ".unf", "UNIF Format ROM Image" }, // Sexy 8.3 variant
 { ".unif", "UNIF Format ROM Image" },
 { NULL, NULL }
};

MDFNGI EmulatedNES =
{
 "nes",
 "Nintendo Entertainment System/Famicom",
 KnownExtensions,
 MODPRIO_INTERNAL_HIGH,
 #ifdef WANT_DEBUGGER
 &NESDBGInfo,
 #else
 NULL,
 #endif
 &NESInputInfo,
 Load,
 TestMagic,
 NULL,
 NULL,
 CloseGame,
 MDFNNES_ToggleLayer,
 "Background\0Sprites\0",
 InstallReadPatch,
 RemoveReadPatches,
 MemRead,
 StateAction,
 Emulate,
 MDFNNES_SetInput,
 MDFNNES_DoSimpleCommand,
 NESSettings,
 0,
 0,
 FALSE,	// Multires

 0,   // lcm_width		(replaced in game load)
 0,   // lcm_height           	(replaced in game load)
 NULL,  // Dummy

 256,	// Nominal width
 240,	// Nominal height
 256,	// Framebuffer width(altered if NTSC blitter is enabled)
 256,	// Framebuffer height

 1,     // Number of output sound channels
};
