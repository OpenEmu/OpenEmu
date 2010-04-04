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
#include "../memory.h"
#include "../netplay.h"
#include "../mempatcher.h"

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

static int SMS_Z80StateAction(StateMem *sm, int load, int data_only)
{
 uint8 r_register;

 SFORMAT StateRegs[] =
 {
  SFVARN(z80.af.w, "AF"),
  SFVARN(z80.bc, "BC"),
  SFVARN(z80.de, "DE"),
  SFVARN(z80.hl, "HL"),
  SFVARN(z80.af_, "AF_"),
  SFVARN(z80.bc_, "BC_"),
  SFVARN(z80.de_, "DE_"),
  SFVARN(z80.hl_, "HL_"),
  SFVARN(z80.ix, "IX"),
  SFVARN(z80.iy, "IY"),
  SFVARN(z80.sp, "SP"),
  SFVARN(z80.pc, "PC"),
  SFVARN(z80.iff1, "IFF1"),
  SFVARN(z80.iff2, "IFF2"),
  SFVARN(z80.im, "IM"),
  SFVARN(r_register, "R"),
  SFEND
 };

 if(!load)
  r_register = (z80.r7 & 0x80) | (z80.r & 0x7f);

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "Z80"))
 {
  return(0);
 }

 if(load)
 {
  z80.r7 = r_register & 0x80;
  z80.r = r_register & 0x7F;
 }

 return(1);
}

static int StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(SoftResetCount),
  SFARRAYN(sms.wram, 0x2000, "RAM"),
  //SFVAR(z80_runtime),
  //SFARRAY(CPUExRAM, 16384),
  //SFVAR(FlashStatusEnable),
  SFEND
 };
 int ret = 1;

 ret &= MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN");
 ret &= SMS_Z80StateAction(sm, load, data_only);
 ret &= SMS_CartStateAction(sm, load, data_only);
 ret &= SMS_SoundStateAction(sm, load, data_only);
 ret &= SMS_VDPStateAction(sm, load, data_only);

 if(load)
 {


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
 MDFNGameInfo->fb = espec->pixels;
 sms.timestamp = 0;

	input.pad[0] = 0;//*InputPtrs[0] & 0x3F;

// if(IS_SMS)
// {
  //input.pad[1] = 0//*InputPtrs[1] & 0x3F;

/*  if((*InputPtrs[0] | *InputPtrs[1]) & 0x40)
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
*/
 //NGPJoyLatch = *chee;

 MDFNMP_ApplyPeriodicCheats();

 bitmap.data = (uint8*)espec->pixels;
 bitmap.width = 256;
 bitmap.height = 240;
 bitmap.pitch = 256 * sizeof(uint32);

 system_frame(espec->skip);

 *(espec->SoundBuf) = SMS_SoundFlush(espec->SoundBufSize);
}

static void CloseGame(void)
{
 MDFN_FlushGameCheats(0);
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
 {
  std::string tmp_string;

  sms.display     = DISPLAY_NTSC;

  tmp_string = MDFN_GetSettingS("sms.territory");
  if(!strcasecmp(tmp_string.c_str(), "domestic"))
   sms.territory = TERRITORY_DOMESTIC;
  else
   sms.territory = TERRITORY_EXPORT;
  sms.use_fm      = FALSE;
 }

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

 MDFN_LoadGameCheats(NULL);
 MDFNGameInfo->GameSetMD5Valid = FALSE;

 //if(sms.console == CONSOLE_SMS || MDFN_GetSettingB("gg.forcemono"))
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

 MDFNGameInfo->soundchan = 2;
 SMS_SoundInit(0, sndclk, sms.use_fm);

 sms.save = 0;


 system_reset();

 return(1);
}

static int LoadSMS(const char *name, MDFNFILE *fp)
{
 if(strcasecmp(fp->ext, "sms") && strcasecmp(fp->ext, "sg") && strcasecmp(fp->ext, "sc"))
  return(-1);
 
 sms.console = CONSOLE_SMS;

 return(LoadCommon(name, fp));
}

static int LoadGG(const char *name, MDFNFILE *fp)
{
 if(strcasecmp(fp->ext, "gg"))
  return(-1);

 sms.console = CONSOLE_GG;

 return(LoadCommon(name, fp));
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
 { 0, "builtin", "Built-In", sizeof(GGInputDeviceInfo) / sizeof(InputDeviceInfoStruct), GGInputDeviceInfo },
};

static const InputPortInfoStruct SMSPortInfo[] =
{
 { 0, "port1", "Port 1", sizeof(SMSInputDeviceInfo) / sizeof(InputDeviceInfoStruct), SMSInputDeviceInfo },
 { 0, "port2", "Port 2", sizeof(SMSInputDeviceInfo) / sizeof(InputDeviceInfoStruct), SMSInputDeviceInfo }
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
  case MDFNNPCMD_POWER: system_reset(); break;
  case MDFNNPCMD_RESET: if(IS_SMS)
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

static void SetPixelFormat(int rs, int gs, int bs)
{
 SMS_VDPSetPixelFormat(rs, gs, bs);
}

static bool ValidateSetting(const char *name, const char *value)
{
 if(!strcasecmp(name, "sms.territory"))
 {
  if(strcasecmp(value, "domestic") && strcasecmp(value, "export"))
   return(FALSE);
 }
 return(TRUE);
}

static MDFNSetting SMSSettings[] =
{
 //{ "ngp.language", gettext_noop("If =1, tell games to display in English, if =0, in Japanese."), MDFNST_UINT, "1", "0", "1" },
 //{ "ngp.forcemono", gettext_noop("Force monophonic sound output."), MDFNST_BOOL, "0" },
 { "sms.territory", gettext_noop("Territory, \"domestic\"(Japan) or \"export\"."), MDFNST_STRING, "export", NULL, NULL, ValidateSetting },
 { "sms.fm", gettext_noop("Enable FM sound emulation when playing domestic/Japan-region games."), MDFNST_BOOL, "1" },
 { NULL }
};

static MDFNSetting GGSettings[] =
{
 { "gg.forcemono", gettext_noop("Force monophonic sound output."), MDFNST_BOOL, "0" },
 { NULL }
};


MDFNGI EmulatedSMS =
{
 "sms",
 #ifdef WANT_DEBUGGER
 NULL,
 #endif
 &SMSInputInfo,
 LoadSMS,
 NULL,
 CloseGame,
 ToggleLayer,
 "Hi\0Ho\0Mo\0", //"Background Scroll\0Foreground Scroll\0Sprites\0",
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
 SMS_SetSoundMultiplier,
 SMS_SetSoundVolume,
 SMS_Sound,
 DoSimpleCommand,
 SMSSettings,
 0,
 NULL,
 256,
 240,
 256, // Save state preview width
 256 * sizeof(uint32),
 {0, 0, 256, 240},
};

MDFNGI EmulatedGG =
{
 "gg",
 #ifdef WANT_DEBUGGER
 NULL,
 #endif
 &GGInputInfo,
 LoadGG,
 NULL,
 CloseGame,
 ToggleLayer,
 "Hi\0Ho\0Mo\0", //"Background Scroll\0Foreground Scroll\0Sprites\0",
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
 SMS_SetSoundMultiplier,
 SMS_SetSoundVolume,
 SMS_Sound,
 DoSimpleCommand,
 GGSettings,
 0,
 NULL,
 160,
 144 * 2,
 160, // Save state preview width
 256 * sizeof(uint32),
 {48, 48, 160, 144},
};

