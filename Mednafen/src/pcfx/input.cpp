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
#include "interrupt.h"
#include "input.h"
#include "v810_cpu.h"
#include "../endian.h"

enum
{
 FX_SIG_MOUSE = 0xD,
 FX_SIG_TAP = 0xE,
 FX_SIG_PAD = 0xF
};

// Mednafen-specific input type numerics
enum
{
 FXIT_NONE = 0,
 FXIT_PAD = 1,
 FXIT_MOUSE = 2,
};

// D0 = TRG, trigger bit
// D1 = MOD, multi-tap clear mode?
// D2 = IOS, data direction.  0 = output, 1 = input

static uint8 control[2];
static uint8 latched[2];

static int InputTypes[2];
static void *data_ptr[2];
static uint16 data_save[2];
static uint32 data_latch[2];

static int32 LatchPending[2];

static int32 mouse_x[2], mouse_y[2];
static uint8 mouse_button[2];

static void SyncSettings(void);

void FXINPUT_Init(void)
{
 SyncSettings();
}

void FXINPUT_SettingChanged(const char *name)
{
 SyncSettings();
}


#ifdef WANT_DEBUGGER
bool FXINPUT_GetRegister(const std::string &name, uint32 &value, std::string *special)
{
 if(name == "KPCTRL0" || name == "KPCTRL1")
 {
  value = control[name[6] - '0'];
  if(special)
  {
   char buf[256];

   snprintf(buf, 256, "Trigger: %d, MOD: %d, IOS: %s", value & 0x1, value & 0x2, (value & 0x4) ? "Input" : "Output");

   *special = std::string(buf);
  }
  return(TRUE);
 }
 return(FALSE);
}
#endif

static ALWAYS_INLINE int32 min(int32 a, int32 b, int32 c)
{
 int32 ret = a;

 if(b < ret)
  ret = b;
 if(c < ret)
  ret = c;

 return(ret);
}

static ALWAYS_INLINE void SetEvent(void)
{
 v810_setevent(V810_EVENT_PAD, min(LatchPending[0] > 0 ? LatchPending[0] : V810_EVENT_NONONO, LatchPending[1] > 0 ? LatchPending[1] : V810_EVENT_NONONO, V810_EVENT_NONONO));
}

void FXINPUT_SetInput(int port, const char *type, void *ptr)
{
 data_ptr[port] = ptr;

 if(!strcasecmp(type, "mouse"))
 {
  InputTypes[port] = 2;
 }
 else if(!strcasecmp(type, "gamepad"))
  InputTypes[port] = 1;
 else
  InputTypes[port] = 0;
}

uint8 FXINPUT_Read8(uint32 A)
{
 //printf("Read8: %04x\n", A);

 return(FXINPUT_Read16(A &~1) >> ((A & 1) * 8));
}

uint16 FXINPUT_Read16(uint32 A)
{
 FXINPUT_Update();

 uint16 ret = 0;
 
 A &= 0xC2;

 //printf("Read: %04x\n", A);

 if(A == 0x00 || A == 0x80)
 {
  int w = (A & 0x80) >> 7;

  if(latched[w])
   ret = 0x8;
  else
   ret = 0x0;
 }
 else 
 {
  int which = (A >> 7) & 1;

  ret = data_latch[which] >> ((A & 2) ? 16 : 0);

  // Which way is correct?  Clear on low reads, or both?  Official docs only say low...
  if(!(A & 0x2))
   latched[which] = FALSE;
 }

 if(!latched[0] && !latched[1])
   PCFXIRQ_Assert(11, FALSE);

 return(ret);
}

void FXINPUT_Write16(uint32 A, uint16 V)
{
 FXINPUT_Update();

 //printf("Write: %04x:%02x\n", A, V);

 //PCFXIRQ_Assert(11, FALSE);
 //if(V != 7 && V != 5)
 //printf("PAD Write16: %04x %04x %d\n", A, V, v810_timestamp);

 switch(A & 0xC0)
 {
  case 0x80:
  case 0x00:
	    {
	     int w = (A & 0x80) >> 7;

	     if((V & 0x1) && !(control[w] & 0x1))
	     {
	      LatchPending[w] = 1536;
	      //if(InputTypes[w] == FXIT_PAD)
 	      // LatchPending[w] = 100; //2560; //100;
	      //else if(InputTypes[w] == FXIT_MOUSE)
	      // LatchPending[w] = 2560;
	      //else
	      // LatchPending[w] = 100;
	      SetEvent();
	     }
	     control[w] = V & 0x7;
	    }
	    break;
 }
}

void FXINPUT_Write8(uint32 A, uint8 V)
{
 FXINPUT_Write16(A, V); 
}

void FXINPUT_Frame(void)
{
 for(int i = 0; i < 2; i++)
 {
  if(InputTypes[i] == FXIT_PAD)
  {
   data_save[i] = MDFN_de16lsb((uint8 *)data_ptr[i]);
  }
  else if(InputTypes[i] == FXIT_MOUSE)
  {
   mouse_x[i] += (int32)MDFN_de32lsb((uint8 *)data_ptr[i] + 0);
   mouse_y[i] += (int32)MDFN_de32lsb((uint8 *)data_ptr[i] + 4);
   mouse_button[i] = *(uint8 *)((uint8 *)data_ptr[i] + 8);
  }
 }
}

static uint32 lastts;

void FXINPUT_Update(void)
{
 int32 run_time = v810_timestamp - lastts;

 for(int i = 0; i < 2; i++)
 {
  if(LatchPending[i] > 0)
  {
   LatchPending[i] -= run_time;
   if(LatchPending[i] <= 0)
   {
    if(InputTypes[i] == FXIT_MOUSE)
    {
     uint32 moo = FX_SIG_MOUSE << 28;
     int32 rel_x = (int32)(mouse_x[i]);
     int32 rel_y = (int32)(mouse_y[i]);

     if(rel_x < -127) rel_x = -127;
     if(rel_x > 127) rel_x = 127;
     if(rel_y < -127) rel_y = -127;
     if(rel_y > 127) rel_y = 127;

     moo |= ((rel_x & 0xFF) << 8) | ((rel_y & 0xFF) << 0);

     mouse_x[i] += (int32)(0 - rel_x);
     mouse_y[i] += (int32)(0 - rel_y);

     //printf("%d %d\n", rel_x, rel_y);

     moo |= mouse_button[i] << 16;
     data_latch[i] = moo;
    }
    else if(InputTypes[i] == FXIT_PAD)
     data_latch[i] = data_save[i] | (FX_SIG_PAD << 28);
    else
     data_latch[i] = 0;

    latched[i] = TRUE;
    control[i] &= ~1;
    PCFXIRQ_Assert(11, TRUE);
   }
  }
 }

 lastts = v810_timestamp;

 SetEvent();
}

void FXINPUT_ResetTS(void)
{
 lastts = 0;
}


int FXINPUT_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY32(LatchPending, 2),
  SFARRAY(control, 2),
  SFARRAY(latched, 2),
  SFARRAY32(data_latch, 2),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "PAD");

 if(load)
  SetEvent();

 return(ret);
}

// GamepadIDII and GamepadIDII_DSR must be EXACTLY the same except for the RUN+SELECT exclusion in the latter.
static const InputDeviceInputInfoStruct GamepadIDII[] =
{
 { "i", "I", 11, IDIT_BUTTON, NULL },
 { "ii", "II", 10, IDIT_BUTTON, NULL },
 { "iii", "III", 9, IDIT_BUTTON, NULL },
 { "iv", "IV", 6, IDIT_BUTTON, NULL },
 { "v", "V", 7, IDIT_BUTTON, NULL },
 { "vi", "VI", 8, IDIT_BUTTON, NULL },
 { "select", "SELECT", 4, IDIT_BUTTON, NULL },
 { "run", "RUN", 5, IDIT_BUTTON, NULL },
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
};
static const InputDeviceInputInfoStruct GamepadIDII_DSR[] =
{
 { "i", "I", 11, IDIT_BUTTON, NULL },
 { "ii", "II", 10, IDIT_BUTTON, NULL },
 { "iii", "III", 9, IDIT_BUTTON, NULL },
 { "iv", "IV", 6, IDIT_BUTTON, NULL },
 { "v", "V", 7, IDIT_BUTTON, NULL },
 { "vi", "VI", 8, IDIT_BUTTON, NULL },
 { "select", "SELECT", 4, IDIT_BUTTON, "run" },
 { "run", "RUN", 5, IDIT_BUTTON, "select" },
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
};


static const InputDeviceInputInfoStruct MouseIDII[] =
{
 { "x_axis", "X Axis", -1, IDIT_X_AXIS_REL },
 { "y_axis", "Y Axis", -1, IDIT_Y_AXIS_REL },
 { "left", "Left Button", 0, IDIT_BUTTON, NULL },
 { "right", "Right Button", 1, IDIT_BUTTON, NULL },
};

// If we add more devices to this array, REMEMBER TO UPDATE the hackish array indexing in the SyncSettings() function
// below.
static InputDeviceInfoStruct InputDeviceInfo[] =
{
 // None
 {
  "none",
  "none",
  NULL,
  0,
  NULL,
 },

 // Gamepad
 {
  "gamepad",
  "Gamepad",
  NULL,
  sizeof(GamepadIDII) / sizeof(InputDeviceInputInfoStruct),
  GamepadIDII,
 },

 // Mouse
 {
  "mouse",
  "Mouse",
  NULL,
  sizeof(MouseIDII) / sizeof(InputDeviceInputInfoStruct),
  MouseIDII
 }
};

static const InputPortInfoStruct PortInfo[] =
{
 { 0, "port1", "Port 1", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo},
 { 0, "port2", "Port 2", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo },
};

InputInfoStruct PCFXInputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};


static void SyncSettings(void)
{
 MDFNGameInfo->mouse_sensitivity = MDFN_GetSettingF("pcfx.mouse_sensitivity");
 InputDeviceInfo[1].IDII = MDFN_GetSettingB("pcfx.disable_softreset") ? GamepadIDII_DSR : GamepadIDII;
}

