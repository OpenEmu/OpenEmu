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
static int32 z80_runtime;

static void Emulate(EmulateSpecStruct *espec)
{
	bool MeowMeow = 0;

	espec->DisplayRect.x = 0;
	espec->DisplayRect.y = 0;
	espec->DisplayRect.w = 160;
	espec->DisplayRect.h = 152;

	if(espec->VideoFormatChanged)
	 NGPGfx->set_pixel_format(espec->surface->format);
		//espec->surface->format.Rshift, espec->surface->format.Gshift,
		//espec->surface->format.Bshift);

	if(espec->SoundFormatChanged)
	 MDFNNGPC_SetSoundRate(espec->SoundRate);


	NGPJoyLatch = *chee;
	storeB(0x6F82, *chee);

	MDFNMP_ApplyPeriodicCheats();

	ngpc_soundTS = 0;
	NGPFrameSkip = espec->skip;
	do
	{
	 int timetime = TLCS900h_interpret();

	 MeowMeow |= updateTimers(espec->surface, timetime);

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


	espec->MasterCycles = ngpc_soundTS;
	espec->SoundBufSize = MDFNNGPCSOUND_Flush(espec->SoundBuf, espec->SoundBufMaxSize);
}

static bool TestMagic(const char *name, MDFNFILE *fp)
{
 if(strcasecmp(fp->ext, "ngp") && strcasecmp(fp->ext, "ngpc") && strcasecmp(fp->ext, "ngc") && strcasecmp(fp->ext, "npc"))
  return(FALSE);

 return(TRUE);
}

static int Load(const char *name, MDFNFILE *fp)
{
 if(!(ngpc_rom.data = (uint8 *)MDFN_malloc(fp->size, _("Cart ROM"))))
  return(0);

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

 MDFNGameInfo->fps = (uint32)((uint64)6144000 * 65536 * 256 / 515 / 198); // 3072000 * 2 * 10000 / 515 / 198
 MDFNGameInfo->GameSetMD5Valid = FALSE;

 MDFNNGPCSOUND_Init();

 MDFNMP_AddRAM(16384, 0x4000, CPUExRAM);

 SetFRM(); // Set up fast read memory mapping

 bios_install();

 z80_runtime = 0;

 reset();

 return(1);
}

static void CloseGame(void)
{
 rom_unload();
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
  case MDFN_MSC_POWER:
  case MDFN_MSC_RESET: reset();
			break;
 }
}

static const MDFNSetting_EnumList LanguageList[] =
{
 { "english", 0, gettext_noop("English") },
 { "0", 0 },

 { "japanese", 1, gettext_noop("Japanese") },
 { "1", 1 },

 { NULL, 0 },
};

static MDFNSetting NGPSettings[] =
{
 { "ngp.language", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Language games should display text in."), NULL, MDFNST_ENUM, "english", NULL, NULL, NULL, NULL, LanguageList },
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
 { 0, "builtin", "Built-In", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" }
};

static InputInfoStruct InputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};

static const FileExtensionSpecStruct KnownExtensions[] =
{
 { ".ngp", gettext_noop("Neo Geo Pocket ROM Image") },
 { ".ngc", gettext_noop("Neo Geo Pocket Color ROM Image") },
 { NULL, NULL }
};

MDFNGI EmulatedNGP =
{
 "ngp",
 "Neo Geo Pocket (Color)",
 KnownExtensions,
 MODPRIO_INTERNAL_HIGH,
 NULL,
 &InputInfo,
 Load,
 TestMagic,
 NULL,
 NULL,
 CloseGame,
 ToggleLayer,
 "Background Scroll\0Foreground Scroll\0Sprites\0",
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 SetInput,
 DoSimpleCommand,
 NGPSettings,
 MDFN_MASTERCLOCK_FIXED(6144000),
 0,

 false, // Multires possible?

 160,   // lcm_width
 152,   // lcm_height
 NULL,  // Dummy

 160,	// Nominal width
 152,	// Nominal height

 160,	// Framebuffer width
 152,	// Framebuffer height

 2,     // Number of output sound channels
};

