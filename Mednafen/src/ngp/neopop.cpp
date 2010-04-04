//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

#include "neopop.h"
#include "../netplay.h"
#include "../general.h"
#include "../md5.h"

#include "TLCS900h_interpret.h"
#include "TLCS900h_registers.h"
#include "Z80_interface.h"
#include "interrupt.h"
#include "mem.h"
#include "gfx.h"
#include "sound.h"
#include "dma.h"
#include "bios.h"
#include "flash.h"

extern uint8 CPUExRAM[16384];

NGPGFX_CLASS *NGPGfx;

COLOURMODE system_colour = COLOURMODE_AUTO;

uint8 NGPJoyLatch;

bool system_comms_read(uint8* buffer)
{
 return(0);
}

bool system_comms_poll(uint8* buffer)
{
 return(0);
}

void system_comms_write(uint8 data)
{
 return;
}

void  instruction_error(char* vaMessage,...)
{
	char message[1000];
	va_list vl;

	va_start(vl, vaMessage);
	vsprintf(message, vaMessage, vl);
	va_end(vl);

	MDFN_printf("[PC %06X] %s\n", pc, message);
}

static uint8 *chee;

bool NGPFrameSkip;
int32 ngpc_soundTS = 0;
static int32 z80_runtime = 0;

static void Emulate(EmulateSpecStruct *espec)
{
	bool MeowMeow = 0;

	MDFNGameInfo->fb = espec->pixels;

//	NGPJoyLatch = *chee;
//	storeB(0x6F82, *chee);

	MDFNMP_ApplyPeriodicCheats();

	ngpc_soundTS = 0;
	NGPFrameSkip = espec->skip;
	do
	{
	 int timetime = TLCS900h_interpret();

	 MeowMeow |= updateTimers(timetime);

	 z80_runtime += timetime;

         while(z80_runtime > 0)
	 {
	  int z80rantime = Z80_RunOP();

	  if(z80rantime < 0) // Z80 inactive, so take up all run time!
	  {
	   z80_runtime = 0;
	   break;
	  }

	  z80_runtime -= z80rantime << 1;

	 }
	} while(!MeowMeow);

	*(espec->SoundBuf) = MDFNNGPCSOUND_Flush(espec->SoundBufSize);
}

static bool TestMagic(const char *name, MDFNFILE *fp)
{
 if(strcasecmp(fp->ext, "ngp") && strcasecmp(fp->ext, "ngpc") && strcasecmp(fp->ext, "ngc") && strcasecmp(fp->ext, "npc"))
  return(FALSE);

 return(TRUE);
}

static int Load(const char *name, MDFNFILE *fp)
{
 if(strcasecmp(fp->ext, "ngp") && strcasecmp(fp->ext, "ngpc") && strcasecmp(fp->ext, "ngc") && strcasecmp(fp->ext, "npc"))
  return(-1);

 ngpc_rom.data = (uint8 *)malloc(fp->size);
 ngpc_rom.length = fp->size;
 memcpy(ngpc_rom.data, fp->data, fp->size);

 md5_context md5;
 md5.starts();
 md5.update(ngpc_rom.data, ngpc_rom.length);
 md5.finish(MDFNGameInfo->MD5);

 rom_loaded();
 MDFN_printf(_("ROM:       %dKiB\n"), (ngpc_rom.length + 1023) / 1024);
 MDFN_printf(_("ROM MD5:   0x%s\n"), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str());

 MDFNMP_Init(1024, 1024 * 1024 * 16 / 1024);

 NGPGfx = new NGPGFX_CLASS();

 NGPGfx->set_pixel_format(FSettings.rshift, FSettings.gshift, FSettings.bshift);


 MDFN_LoadGameCheats(NULL);
 MDFNGameInfo->fps = (uint32)((uint64)6144000 * 65536 * 256 / 515 / 198); // 3072000 * 2 * 10000 / 515 / 198
 MDFNGameInfo->GameSetMD5Valid = FALSE;

 if(MDFN_GetSettingB("ngp.forcemono"))
 {
  MDFNGameInfo->soundchan = 1;
  MDFNNGPCSOUND_Init(1);
 }
 else
 {
  MDFNGameInfo->soundchan = 2;
  MDFNNGPCSOUND_Init(0);
 }
 MDFNMP_AddRAM(16384, 0x4000, CPUExRAM);

 SetFRM(); // Set up fast read memory mapping

 bios_install();

 reset();

 return(1);
}

static void CloseGame(void)
{
 MDFN_FlushGameCheats(0);
 rom_unload();
}

static void SetPixelFormat(int rs, int gs, int bs)
{
 NGPGfx->set_pixel_format(rs, gs, bs);
}

static void SetInput(int port, const char *type, void *ptr)
{
 if(!port) chee = (uint8 *)ptr;
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(z80_runtime),
  SFARRAY(CPUExRAM, 16384),
  SFVAR(FlashStatusEnable),
  SFEND
 };

 SFORMAT TLCS_StateRegs[] =
 {
  SFVARN(pc, "PC"),
  SFVARN(sr, "SR"),
  SFVARN(f_dash, "F_DASH"),
  SFARRAY32N(gpr, 4, "GPR"),
  SFARRAY32N(gprBank[0], 4, "GPRB0"),
  SFARRAY32N(gprBank[1], 4, "GPRB1"),
  SFARRAY32N(gprBank[2], 4, "GPRB2"),
  SFARRAY32N(gprBank[3], 4, "GPRB3"),
  SFEND
 };

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN"))
  return(0);

 if(!MDFNSS_StateAction(sm, load, data_only, TLCS_StateRegs, "TLCS"))
  return(0);

 if(!MDFNNGPCDMA_StateAction(sm, load, data_only))
  return(0);

 if(!MDFNNGPCSOUND_StateAction(sm, load, data_only))
  return(0);

 if(!NGPGfx->StateAction(sm, load, data_only))
  return(0);

 if(!MDFNNGPCZ80_StateAction(sm, load, data_only))
  return(0);

 if(!int_timer_StateAction(sm, load, data_only))
  return(0);

 if(!BIOSHLE_StateAction(sm, load, data_only))
  return(0);

 if(!FLASH_StateAction(sm, load, data_only))
  return(0);

 if(load)
 {
  RecacheFRM();
  changedSP();
 }
 return(1);
}

static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
  case MDFNNPCMD_POWER:
  case MDFNNPCMD_RESET: reset();
			break;
 }
}

static MDFNSetting NGPSettings[] =
{
 { "ngp.language", gettext_noop("If =1, tell games to display in English, if =0, in Japanese."), MDFNST_UINT, "1", "0", "1" },
 { "ngp.forcemono", gettext_noop("Force monophonic sound output."), MDFNST_BOOL, "0" },
 { NULL }
};


bool system_io_flash_read(uint8* buffer, uint32 bufferLength)
{
 FILE *fp = fopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "flash").c_str(), "rb");
 if(!fp) return(0);

 if(!fread(buffer, 1, bufferLength, fp))
 {
  fclose(fp);
  return(0);
 }

 fclose(fp);

 return(1);
}

bool system_io_flash_write(uint8* buffer, uint32 bufferLength)
{
 FILE *fp = fopen(MDFN_MakeFName(MDFNMKF_SAV, 0, "flash").c_str(), "wb");
 if(!fp) return(0);

 if(!fwrite(buffer, 1, bufferLength, fp))
 {
  fclose(fp);
  return(0);
 }

 fclose(fp);

 return(1);
}

static bool ToggleLayer(int which)
{
 return(NGPGfx->ToggleLayer(which));
}

static const InputDeviceInputInfoStruct IDII[] =
{
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "a", "A", 5, IDIT_BUTTON_CAN_RAPID,  NULL },
 { "b", "B", 6, IDIT_BUTTON_CAN_RAPID, NULL },
 { "option", "OPTION", 4, IDIT_BUTTON, NULL },
};
static InputDeviceInfoStruct InputDeviceInfo[] =
{
 {
  "gamepad",
  "Gamepad",
  NULL,
  sizeof(IDII) / sizeof(InputDeviceInputInfoStruct),
  IDII,
 }
};

static const InputPortInfoStruct PortInfo[] =
{
 { 0, "builtin", "Built-In", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo }
};

static InputInfoStruct InputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};

MDFNGI EmulatedNGP =
{
 "ngp",
 #ifdef WANT_DEBUGGER
 NULL,
 #endif
 &InputInfo,
 Load,
 NULL,
 CloseGame,
 ToggleLayer,
 "Background Scroll\0Foreground Scroll\0Sprites\0",
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 SetPixelFormat,
 SetInput,
 NULL,
 NULL,
 NULL,
 MDFNNGPC_SetSoundMultiplier,
 MDFNNGPC_SetSoundVolume,
 MDFNNGPC_Sound,
 DoSimpleCommand,
 NGPSettings,
 0,
 NULL,
 160,
 152,
 160, // Save state preview width
 256 * sizeof(uint32),
 {0, 0, 160, 152},
};

