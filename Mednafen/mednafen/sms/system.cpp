/*
    Copyright (C) 1998-2004  Charles MacDonald

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
#include "../mempatcher.h"

namespace MDFN_IEN_SMS
{

bitmap_t bitmap;
input_t input;
static int32 SoftResetCount;

/* Run the virtual console emulation for one frame */
void system_frame(int skip_render)
{
    /* Debounce pause key */
    if(input.system & INPUT_PAUSE)
    {
        if(!sms.paused)
        {
            sms.paused = 1;

	    z80_nmi();
        }
    }
    else
    {
         sms.paused = 0;
    }

    if(SoftResetCount)
    {
     SoftResetCount--;
     if(!SoftResetCount)
      input.system &= ~INPUT_RESET;     
    }
    SMS_VDPRunFrame(skip_render);
}


void system_reset(void)
{
    SoftResetCount = 0;

    SMS_CartReset();

    sms_reset();
    pio_reset();
    vdp_reset();
    render_reset();
    SMS_SoundReset();
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(SoftResetCount),
  SFVAR(sms.cycle_counter),
  SFARRAYN(sms.wram, 0x2000, "RAM"),

  SFVAR(sms.paused),

  SFVAR(input.pad[0]),
  SFVAR(input.pad[1]),

  SFVAR(input.analog[0]),
  SFVAR(input.analog[1]),

  SFVAR(input.system),

  SFVAR(sms.fm_detect),
  SFVAR(sms.memctrl),

  //SFVAR(z80_runtime),
  //SFARRAY(CPUExRAM, 16384),
  //SFVAR(FlashStatusEnable),
  SFEND
 };
 int ret = 1;

 ret &= MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN");
 ret &= z80_state_action(sm, load, data_only, "Z80");
 ret &= SMS_CartStateAction(sm, load, data_only);
 ret &= SMS_PIOStateAction(sm, load, data_only);
 ret &= SMS_SoundStateAction(sm, load, data_only);
 ret &= SMS_VDPStateAction(sm, load, data_only);

 if(load)
 {
  if(sms.cycle_counter > 1000)
  {
   sms.cycle_counter = 1000;
   puts("sms.cycle_counter sanity failed");
  }
 } 

 return(ret);
}

static uint8 *InputPtrs[2];

static void SetInput(int port, const char *type, void *ptr)
{
 InputPtrs[port] = (uint8 *)ptr;
}

static void Emulate(EmulateSpecStruct *espec)
{
 if(espec->VideoFormatChanged)
  SMS_VDPSetPixelFormat(espec->surface->format);

 if(espec->SoundFormatChanged)
  SMS_SetSoundRate(espec->SoundRate);


 sms.timestamp = 0;

 input.pad[0] = *InputPtrs[0] & 0x3F;

 if(IS_SMS)
 {
  input.pad[1] = *InputPtrs[1] & 0x3F;

  if((*InputPtrs[0] | *InputPtrs[1]) & 0x40)
   input.system |= INPUT_PAUSE;
  else
   input.system &= ~INPUT_PAUSE;
 }
 else // GG:
 {
  if(*InputPtrs[0] & 0x40)
  {
   input.system |= INPUT_START;
  }
  else
   input.system &= ~INPUT_START;
 }

 //NGPJoyLatch = *chee;

 MDFNMP_ApplyPeriodicCheats();

 if(sms.console == CONSOLE_GG)
 {
  espec->DisplayRect.x = 48;
  espec->DisplayRect.y = 48;
  espec->DisplayRect.w = 160;
  espec->DisplayRect.h = 144;
 }
 else
 {
  espec->DisplayRect.x = 0;
  espec->DisplayRect.y = 0;
  espec->DisplayRect.w = 256;
  espec->DisplayRect.h = 240;
 }

 bitmap.data = (uint8*)espec->surface->pixels;
 bitmap.width = 256;
 bitmap.height = 240;
 bitmap.pitch = 256 * sizeof(uint32);

 system_frame(espec->skip);

 espec->MasterCycles = sms.timestamp;
 espec->SoundBufSize = SMS_SoundFlush(espec->SoundBuf, espec->SoundBufMaxSize);
}

static void CloseGame(void)
{
 SMS_CartClose();

 sms_shutdown();
 pio_shutdown();
 vdp_shutdown();
 render_shutdown();
 SMS_SoundClose();
}

static int LoadCommon(const char *name, MDFNFILE *fp)
{
 int32 size = fp->size;
 const uint8 *data_ptr = fp->data;

 if(size & 512)
 {
  size -= 512;
  data_ptr += 512;
 }

 /* Assign default settings (US NTSC machine) */
 sms.display     = DISPLAY_NTSC;

 sms.territory   = MDFN_GetSettingI("sms.territory");
 sms.use_fm      = FALSE;


 if(!SMS_CartInit(data_ptr, size))
  return(0);

 if(IS_SMS && sms.territory == TERRITORY_DOMESTIC)
  sms.use_fm = MDFN_GetSettingB("sms.fm");

 MDFNMP_Init(1024, 65536 / 1024);

 system_assign_device(PORT_A, DEVICE_PAD2B);
 system_assign_device(PORT_B, DEVICE_PAD2B);

 MDFNMP_AddRAM(8192, 0xC000, sms.wram);

 sms_init();
 pio_init();
 vdp_init();
 render_init();

 MDFNGameInfo->GameSetMD5Valid = FALSE;

 uint32 sndclk;

 if(sms.display == DISPLAY_PAL)
 {
  sndclk = 3546893;
  MDFNGameInfo->fps = (uint32)((uint64)65536 * 256 * sndclk / 313 / 228); //6144000 * 65536 * 256 / 515 / 198); // 3072000 * 2 * 10000 / 515 / 198
 }
 else
 {
  sndclk = 3579545;
  MDFNGameInfo->fps = (uint32)((uint64)65536 * 256 * sndclk / 262 / 228); //6144000 * 65536 * 256 / 515 / 198); // 3072000 * 2 * 10000 / 515 / 198
 }

 MDFNGameInfo->MasterClock = MDFN_MASTERCLOCK_FIXED(sndclk);

 SMS_SoundInit(sndclk, sms.use_fm);

 sms.save = 0;


 system_reset();

 return(1);
}

static bool TestMagicSMS(const char *name, MDFNFILE *fp)
{
 if(strcasecmp(fp->ext, "sms") && strcasecmp(fp->ext, "sg") && strcasecmp(fp->ext, "sc"))
  return(FALSE);

 return(TRUE);
}

static bool TestMagicGG(const char *name, MDFNFILE *fp)
{
 if(strcasecmp(fp->ext, "gg"))
  return(FALSE);

 return(TRUE);
}


static int LoadSMS(const char *name, MDFNFILE *fp)
{
 sms.console = CONSOLE_SMS;

 return(LoadCommon(name, fp));
}

static int LoadGG(const char *name, MDFNFILE *fp)
{
 sms.console = CONSOLE_GG;

 return(LoadCommon(name, fp));
}

}

static const InputDeviceInputInfoStruct GGGamepadIDII[] =
{
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "button1", "Button 1", 4, IDIT_BUTTON_CAN_RAPID, NULL },
 { "button2", "Button 2", 5, IDIT_BUTTON_CAN_RAPID,  NULL },
 { "Start", "Start", 6, IDIT_BUTTON, NULL },
};

static const InputDeviceInputInfoStruct SMSGamepadIDII[] =
{
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "fire1", "Fire 1/Start", 4, IDIT_BUTTON_CAN_RAPID, NULL },
 { "fire2", "Fire 2", 5, IDIT_BUTTON_CAN_RAPID,  NULL },
 { "pause", "Pause", 6, IDIT_BUTTON, NULL },
};

static InputDeviceInfoStruct GGInputDeviceInfo[] =
{
 {
  "gamepad",
  "Gamepad",
  NULL,
  sizeof(GGGamepadIDII) / sizeof(InputDeviceInputInfoStruct),
  GGGamepadIDII,
 }
};

static InputDeviceInfoStruct SMSInputDeviceInfo[] =
{
 {
  "gamepad",
  "Gamepad",
  NULL,
  sizeof(SMSGamepadIDII) / sizeof(InputDeviceInputInfoStruct),
  SMSGamepadIDII,
 }
};

static const InputPortInfoStruct GGPortInfo[] =
{
 { 0, "builtin", "Built-In", sizeof(GGInputDeviceInfo) / sizeof(InputDeviceInfoStruct), GGInputDeviceInfo, "gamepad" },
};

static const InputPortInfoStruct SMSPortInfo[] =
{
 { 0, "port1", "Port 1", sizeof(SMSInputDeviceInfo) / sizeof(InputDeviceInfoStruct), SMSInputDeviceInfo, "gamepad" },
 { 0, "port2", "Port 2", sizeof(SMSInputDeviceInfo) / sizeof(InputDeviceInfoStruct), SMSInputDeviceInfo, "gamepad" }
};

static InputInfoStruct GGInputInfo =
{
 sizeof(GGPortInfo) / sizeof(InputPortInfoStruct),
 GGPortInfo
};

static InputInfoStruct SMSInputInfo =
{
 sizeof(SMSPortInfo) / sizeof(InputPortInfoStruct),
 SMSPortInfo
};

static void DoSimpleCommand(int cmd)
{
 switch(cmd)
 {
  case MDFN_MSC_POWER: system_reset(); break;
  case MDFN_MSC_RESET: if(IS_SMS)
			{
			 input.system |= INPUT_RESET;
			 SoftResetCount = 20;
			}
			else
			 system_reset();
			break;
 }
}

static bool ToggleLayer(int which)
{
 return(TRUE);
}

static MDFNSetting_EnumList Territory_List[] =
{
 { "domestic", TERRITORY_DOMESTIC, gettext_noop("Domestic(Japanese)") },
 { "export", TERRITORY_EXPORT, gettext_noop("Export(World)") },
 { NULL, 0 },
};

static MDFNSetting SMSSettings[] =
{
 { "sms.territory", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("System territory/region."), NULL, MDFNST_ENUM, "export", NULL, NULL, NULL, NULL, Territory_List },
 { "sms.fm", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, gettext_noop("Enable FM sound emulation when playing domestic/Japan-region games."), NULL, MDFNST_BOOL, "1" },
 { NULL }
};

static MDFNSetting GGSettings[] =
{
 { NULL }
};


static const FileExtensionSpecStruct SMSKnownExtensions[] =
{
 { ".sms", gettext_noop("Sega Master System ROM Image") },
 { NULL, NULL }
};

static const FileExtensionSpecStruct GGKnownExtensions[] =
{
 { ".gg", gettext_noop("Game Gear ROM Image") },
 { NULL, NULL }
};

MDFNGI EmulatedSMS =
{
 "sms",
 "Sega Master System",
 SMSKnownExtensions,
 MODPRIO_INTERNAL_HIGH,
 NULL,
 &SMSInputInfo,
 LoadSMS,
 TestMagicSMS,
 NULL,
 NULL,
 CloseGame,
 ToggleLayer,
 "Hi\0Ho\0Mo\0", //"Background Scroll\0Foreground Scroll\0Sprites\0",
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 SetInput,
 DoSimpleCommand,
 SMSSettings,
 0,
 0,
 FALSE, // Multires possible?

 256,   // lcm_width
 240,   // lcm_height
 NULL,  // Dummy

 256,	// Nominal width
 240,	// Nominal height

 256,	// Framebuffer width
 256,	// Framebuffer height

 2,     // Number of output sound channels
};

MDFNGI EmulatedGG =
{
 "gg",
 "Game Gear",
 GGKnownExtensions,
 MODPRIO_INTERNAL_HIGH,
 NULL,
 &GGInputInfo,
 LoadGG,
 TestMagicGG,
 NULL,
 NULL,
 CloseGame,
 ToggleLayer,
 "Hi\0Ho\0Mo\0", //"Background Scroll\0Foreground Scroll\0Sprites\0",
 NULL,
 NULL,
 NULL,
 StateAction,
 Emulate,
 SetInput,
 DoSimpleCommand,
 GGSettings,
 0,
 0,
 FALSE, // Multires possible?

 160,   // lcm_width
 144,   // lcm_height           
 NULL,  // Dummy


 160,	// nominal width
 144,	// nominal height

 256,	// Framebuffer width
 256,	// Framebuffer height 

 2,     // Number of output sound channels
};

