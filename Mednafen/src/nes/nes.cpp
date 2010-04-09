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

#include        <string.h>
#include	<stdarg.h>

#include	"nes.h"
#include	"../netplay.h"

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
static readfunc *AReadG;
static writefunc *BWriteG;
static int RWWrap=0;

bool NESIsVSUni;


static DECLFW(BNull)
{
// printf("Null Write: %04x %02x\n", A, V);
}

static DECLFR(ANull)
{
 //printf("Null Read: %04x\n", A);
 return(X.DB);
}

int AllocGenieRW(void)
{
 if(!(AReadG=(readfunc *)MDFN_malloc(0x8000*sizeof(readfunc), _("Game Genie Read Handler Save"))))
  return 0;
 if(!(BWriteG=(writefunc *)MDFN_malloc(0x8000*sizeof(writefunc), _("Game Genie Write Handler Save"))))
  return 0;
 RWWrap=1;
 return 1;
}

void FlushGenieRW(void)
{
 int32 x;

 if(RWWrap)
 {
  for(x=0;x<0x8000;x++)
  {
   ARead[x+0x8000]=AReadG[x];
   BWrite[x+0x8000]=BWriteG[x];
  }
  free(AReadG);
  free(BWriteG);
  AReadG=0;
  BWriteG=0;
  RWWrap=0;
 }
}

readfunc GetReadHandler(int32 a)
{
  if(a>=0x8000 && RWWrap)
   return AReadG[a-0x8000];
  else
   return ARead[a];
}

void SetReadHandler(int32 start, int32 end, readfunc func, bool snc)
{
  int32 x;

  //printf("%08x %08x %lld %d\n", start, end, func, snc);

  if(!func)
   func=ANull;

  if(RWWrap)
  {
   for(x=end;x>=start;x--)
   {
    if(x>=0x8000)
     AReadG[x-0x8000]=func;
    else
    {
     ARead[x]=func;
    }
    if(snc)
     NonCheatARead[x] = func;
   }
  }
  else
   for(x=end;x>=start;x--)
   {
    ARead[x]=func;
    if(snc)
     NonCheatARead[x] = func;
   }
}

writefunc GetWriteHandler(int32 a)
{
  if(RWWrap && a>=0x8000)
   return BWriteG[a-0x8000];
  else
   return BWrite[a];
}

void SetWriteHandler(int32 start, int32 end, writefunc func)
{
  int32 x;

  if(!func)
   func=BNull;

  if(RWWrap)
   for(x=end;x>=start;x--)
   {
    if(x>=0x8000)
     BWriteG[x-0x8000]=func;
    else
     BWrite[x]=func;
   }
  else
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

#ifdef WANT_DEBUGGER
static RegType NESCPURegs[] =
{
        { "PC", "Program Counter", 2 },
        { "A", "Accumulator", 1 },
        { "X", "X Index", 1 },
        { "Y", "Y Index", 1 },
        { "SP", "Stack Pointer", 1 },
        { "P", "Status", 1 },
        { "", "", 0 },
};

static RegGroupType NESCPURegsGroup =
{
        NESCPURegs,
        NESDBG_GetRegister,
        NESDBG_SetRegister
};

static RegType NESPPURegs[] =
{
        { "PPU0", "PPU0", 1 },
        { "PPU1", "PPU1", 1 },
        { "PPU2", "PPU2", 1 },
        { "PPU3", "PPU3", 1 },
        { "XOffset", "Tile X Offset", 1},
        { "RAddr", "Refresh Address", 2},
        { "TAddr", "Temp Address", 2},
        { "VRAM Buf", "VRAM Buffer", 1},
        { "V-Toggle", "High/low Toggle", 1},
	{ "Scanline", "Current Scanline(0 = first visible, 0xF0 = in vblank)", 1 },
        { "", "", 0 },
};

static RegGroupType NESPPURegsGroup =
{
 NESPPURegs,
 NESDBG_GetRegister,
 NESDBG_SetRegister
};
#endif

static void CloseGame(void)
{
 if(MDFNGameInfo->GameType != GMT_PLAYER)
  MDFN_FlushGameCheats(0);

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
 CloseGenie();
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
        MDFN_SetPPUPalette(0);

        MDFNGameInfo->GameType = GMT_CART;
        MDFNGameInfo->VideoSystem = VIDSYS_NONE;

        MDFNGameInfo->cspecial = NULL;
        MDFNGameInfo->soundchan = 1;
        MDFNGameInfo->GameSetMD5Valid = FALSE;


        if(MDFN_GetSettingB("nes.fnscan"))
        {
         if(strstr(name, "(U)"))
          MDFNGameInfo->VideoSystem = VIDSYS_NTSC;
         else if(strstr(name, "(J)"))
          MDFNGameInfo->VideoSystem = VIDSYS_NTSC;
         else if(strstr(name, "(E)") || strstr(name, "(G)"))
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
        MDFNDBG_AddRegGroup(&NESCPURegsGroup);
        MDFNDBG_AddRegGroup(&NESPPURegsGroup);

        MDFNDBG_AddASpace(NESDBG_GetAddressSpaceBytes, NESDBG_PutAddressSpaceBytes, "cpu", "CPU", 16);
        MDFNDBG_AddASpace(NESPPU_GetAddressSpaceBytes, NESPPU_PutAddressSpaceBytes, "ppu", "PPU", 14);
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

static int Load(const char *name, MDFNFILE *fp)
{
	bool (*MagicFunctions[4])(const char *name, MDFNFILE *fp) = { iNES_TestMagic, UNIF_TestMagic, NSF_TestMagic, FDS_TestMagic };
	bool (*LoadFunctions[4])(const char *name, MDFNFILE *fp, NESGameType *) = { iNESLoad, UNIFLoad, NSFLoad, FDSLoad };
	bool (*LoadFunction)(const char *name, MDFNFILE *fp, NESGameType *) = NULL;
	int x;

	for(x = 0; x < 4; x++)
	 if(MagicFunctions[x](name, fp))
	 {
	  LoadFunction = LoadFunctions[x];
	  break;
	 }

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

	if(MDFNGameInfo->GameType != GMT_PLAYER)
         MDFN_LoadGameCheats(0);

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
	}

	X6502_Init();
	MDFNPPU_Init();
        MDFNSND_Init(PAL);
	NESINPUT_Init();

	if(MDFNGameInfo->GameType != GMT_PLAYER)
         if(MDFN_GetSettingB("nes.gg"))
	  OpenGenie();

        PowerNES();

	if(MDFNGameInfo->GameType != GMT_PLAYER)
         MDFN_LoadGamePalette();

        MDFN_ResetPalette();
        return(1);
}

void MDFNNES_Kill(void)
{
 MDFN_KillGenie();
}

static void Emulate(EmulateSpecStruct *espec)
{
 int ssize;

 MDFNGameInfo->fb = espec->pixels;

 MDFN_UpdateInput();

 if(geniestage!=1) MDFNMP_ApplyPeriodicCheats();
 MDFNPPU_Loop(espec->skip);

 ssize = FlushEmulateSound(espec->NeedSoundReverse);
 espec->NeedSoundReverse = 0;

 timestampbase += timestamp;

 timestamp = 0;

 if(MDFNGameInfo->GameType == GMT_PLAYER)
  MDFNNES_DrawNSF(espec->pixels, WaveFinal, ssize);

 *(espec->SoundBuf) = WaveFinal;
 *(espec->SoundBufSize) = ssize;


 if(MDFNGameInfo->GameType != GMT_PLAYER)
 {
  if(NESIsVSUni)
   MDFN_VSUniDraw(espec->pixels);
 }
 MDFN_DrawInput(espec->pixels);
}

void ResetNES(void)
{
        MDFNMOV_AddCommand(MDFNNPCMD_RESET);
	if(GameInterface->Reset)
         GameInterface->Reset();
        MDFNSND_Reset();
        MDFNPPU_Reset();
        X6502_Reset();
}

void MDFN_MemoryRand(uint8 *ptr, uint32 size)
{
 int x=0;
 while(size)
 {
  *ptr=(x&4)?0xFF:0x00;
  x++;
  size--;
  ptr++;
 }
}

void PowerNES(void) 
{
        MDFNMOV_AddCommand(MDFNNPCMD_POWER);
        if(!MDFNGameInfo) return;

	MDFNMP_RemoveReadPatches();

	MDFNMP_AddRAM(0x0800, 0x0000, RAM);

        GeniePower();

	MDFN_MemoryRand(RAM,0x800);
	//memset(RAM,0xFF,0x800);

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


	timestampbase=0;
	X6502_Power();
	MDFNMP_InstallReadPatches();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 #if 0
 uint8 new_geniestage = geniestage;

 SFORMAT StateRegs[] =
 {
  SFVAR(new_geniestage),
  SFEND
 };

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN"))
  return(0);

 if(geniestage)
 {
  if(geniestage == 1 && new_geniestage == 2)
  {

  }
 }
 else if(new_geniestage == 1) // If Game Genie emulation is disabled, and we're trying to load a save-state where we're
			      // in the Game Genie screen, error out!
 {
  return(0);
 }
 #endif

 if(geniestage == 1)
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

static void SettingChanged(const char *name)
{

 puts(name);
}

static MDFNSetting NESSettings[] =
{
  { "nes.input.port1", gettext_noop("Select input device for input port 1."), MDFNST_STRING, "gamepad", NULL, NULL },
  { "nes.input.port2", gettext_noop("Select input device for input port 2."), MDFNST_STRING, "gamepad", NULL, NULL },
  { "nes.input.port3", gettext_noop("Select input device for input port 3."), MDFNST_STRING, "gamepad", NULL, NULL },
  { "nes.input.port4", gettext_noop("Select input device for input port 4."), MDFNST_STRING, "gamepad", NULL, NULL },
  { "nes.input.fcexp", gettext_noop("Select Famicom expansion-port device."), MDFNST_STRING, "none", NULL, NULL },

  { "nes.nofs", gettext_noop("Disabled four-score emulation."), MDFNST_BOOL, "0" },
  { "nes.no8lim", gettext_noop("No 8-sprites-per-scanline limit option."), MDFNST_BOOL, "0", NULL, NULL, NULL, NESPPU_SettingChanged },
  { "nes.n106bs", gettext_noop("Enable less-accurate, but better sounding, Namco 106(mapper 19) sound emulation."), MDFNST_BOOL, "0" },
  { "nes.fnscan", gettext_noop("Scan filename for (U),(J),(E),etc. strings to en/dis-able PAL emulation."), MDFNST_BOOL, "1" },
  { "nes.pal", gettext_noop("Enable PAL(50Hz) NES emulation."), MDFNST_BOOL, "0" },
  { "nes.gg", gettext_noop("Enable Game Genie emulation."), MDFNST_BOOL, "0" },
  { "nes.ggrom", gettext_noop("Path to Game Genie ROM image."), MDFNST_STRING, "" },
  { "nes.clipsides", gettext_noop("Clip left+right 8 pixel columns."), MDFNST_BOOL, "0" },
  { "nes.slstart", gettext_noop("First rendered scanline in NTSC mode."), MDFNST_UINT, "8" },
  { "nes.slend", gettext_noop("Last rendered scanlines in NTSC mode."), MDFNST_UINT, "231" },
  { "nes.slstartp", gettext_noop("First rendered scanline in PAL mode."), MDFNST_UINT, "0" },
  { "nes.slendp", gettext_noop("Last rendered scanlines in PAL mode."), MDFNST_UINT, "239" },
  { "nes.cpalette", gettext_noop("Filename of custom NES palette."), MDFNST_STRING, "" },
  { "nes.ntscblitter", gettext_noop("Enable NTSC color generation and blitter."), MDFNST_UINT, "0" },
  { "nes.ntsc.preset", gettext_noop("Select video quality/type preset."), MDFNST_STRING, "none" },
  { "nes.ntsc.mergefields", gettext_noop("Merge fields to partially work around !=60.1Hz refresh rates."), MDFNST_BOOL, "0" },
  { "nes.ntsc.saturation", gettext_noop("NTSC composite blitter saturation."), MDFNST_FLOAT, "0", "-1", "1" },
  { "nes.ntsc.hue", gettext_noop("NTSC composite blitter hue."), MDFNST_FLOAT, "0", "-1", "1" },
  { "nes.ntsc.sharpness", gettext_noop("NTSC composite blitter sharpness."), MDFNST_FLOAT, "0", "-1", "1" },
  { "nes.ntsc.brightness", gettext_noop("NTSC composite blitter brightness."), MDFNST_FLOAT, "0", "-1", "1" },
  { "nes.ntsc.contrast", gettext_noop("NTSC composite blitter contrast."), MDFNST_FLOAT, "0", "-1", "1" },

  { "nes.ntsc.matrix", gettext_noop("Enable NTSC custom decoder matrix."), MDFNST_BOOL, "0" },

  /* Default custom decoder matrix(not plain default matrix) is from Sony */
  { "nes.ntsc.matrix.0", gettext_noop("NTSC custom decoder matrix element 0(red, value * V)."), MDFNST_FLOAT, "1.539", "-2", "2", NULL, NESPPU_SettingChanged },
  { "nes.ntsc.matrix.1", gettext_noop("NTSC custom decoder matrix element 1(red, value * U)."), MDFNST_FLOAT, "-0.622", "-2", "2", NULL, NESPPU_SettingChanged },
  { "nes.ntsc.matrix.2", gettext_noop("NTSC custom decoder matrix element 2(green, value * V)."), MDFNST_FLOAT, "-0.571", "-2", "2", NULL, NESPPU_SettingChanged },
  { "nes.ntsc.matrix.3", gettext_noop("NTSC custom decoder matrix element 3(green, value * U)."), MDFNST_FLOAT, "-0.185", "-2", "2", NULL, NESPPU_SettingChanged },
  { "nes.ntsc.matrix.4", gettext_noop("NTSC custom decoder matrix element 4(blue, value * V)."), MDFNST_FLOAT, "0.000", "-2", "2", NULL, NESPPU_SettingChanged },
  { "nes.ntsc.matrix.5", gettext_noop("NTSC custom decoder matrix element 5(blue, value * U."), MDFNST_FLOAT, "2.000", "-2", "2", NULL, NESPPU_SettingChanged },
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
 address &= 0xFFFF;

 SetReadHandler(address, address, CheatReadFunc, 0);
}

static void RemoveReadPatches(void)
{
 for(uint32 A = 0; A <= 0xFFFF; A++)
 {
  SetReadHandler(A, A, NonCheatARead[A], 0);
 }
}

#ifdef WANT_DEBUGGER
static DebuggerInfoStruct DBGInfo =
{
 3,
 16,
 16,
 0x0000, // Default watch addr
 0x0000, // ZP
 NESDBG_MemPeek,
 NESDBG_Disassemble,
 NULL,
 NESDBG_IRQ,
 NESDBG_GetVector,
 NESDBG_FlushBreakPoints,
 NESDBG_AddBreakPoint,
 NESDBG_SetCPUCallback,
 NESDBG_SetBPCallback,
 NESDBG_GetBranchTrace,
 NESPPU_SetGraphicsDecode,
 NESPPU_GetGraphicsDecodeBuffer,
};
#endif


MDFNGI EmulatedNES =
{
 "nes",
 #ifdef WANT_DEBUGGER
 &DBGInfo,
 #endif
 &NESInputInfo,
 Load,
 NULL,
 CloseGame,
 MDFNNES_ToggleLayer,
 "Background\0Sprites\0",
 InstallReadPatch,
 RemoveReadPatches,
 MemRead,
 StateAction,
 Emulate,
 MDFNNES_SetPixelFormat,
 MDFNNES_SetInput,
 FDS_DiskInsert,
 FDS_DiskEject,
 FDS_DiskSelect,
 MDFNNES_SetSoundMultiplier,
 MDFNNES_SetSoundVolume,
 MDFNNES_Sound,
 MDFNNES_DoSimpleCommand,
 NESSettings,
 0,
 NULL,
 256,
 240,
 256, // Save state preview width
 256 * sizeof(uint32),
 { 0, 0, 256, 240 },
};
