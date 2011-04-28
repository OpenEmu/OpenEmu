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

#include "input/gamepad.h"
#include "input/mouse.h"

#define PCFX_PORTS	2
#define TOTAL_PORTS	8

#define TAP_PORTS	4

static const int TapMap[2][TAP_PORTS] =
{
 { 0, 2, 3, 4 },
 { 1, 5, 6, 7 },
};


static void RemakeDevices(int which = -1);
static uint8 MultiTapEnabled;

// Mednafen-specific input type numerics
enum
{
 FXIT_NONE = 0,
 FXIT_GAMEPAD = 1,
 FXIT_MOUSE = 2,
};

PCFX_Input_Device::~PCFX_Input_Device()
{

}

uint32 PCFX_Input_Device::ReadTransferTime(void)
{
 return(1536);
}

uint32 PCFX_Input_Device::WriteTransferTime(void)
{
 return(1536);
}

uint32 PCFX_Input_Device::Read(void)
{
 return(0);
}

void PCFX_Input_Device::Write(uint32 data)
{


}

void PCFX_Input_Device::Power(void)
{

}

void PCFX_Input_Device::Frame(const void *data)
{

}

int PCFX_Input_Device::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 return(1);
}

static PCFX_Input_Device *devices[TOTAL_PORTS] = { NULL };


// D0 = TRG, trigger bit
// D1 = MOD, multi-tap clear mode?
// D2 = IOS, data direction.  0 = output, 1 = input

static uint8 TapCounter[PCFX_PORTS];
static uint8 control[PCFX_PORTS];
static bool latched[PCFX_PORTS];
static int32 LatchPending[PCFX_PORTS];

static int InputTypes[TOTAL_PORTS];
static void *data_ptr[TOTAL_PORTS];
static uint32 data_latch[TOTAL_PORTS];

static void SyncSettings(void);

void FXINPUT_Init(void)
{
 SyncSettings();
 RemakeDevices();
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

static INLINE int32 min(int32 a, int32 b, int32 c)
{
 int32 ret = a;

 if(b < ret)
  ret = b;
 if(c < ret)
  ret = c;

 return(ret);
}

static INLINE int32 CalcNextEventTS(const v810_timestamp_t timestamp)
{
 return(min(LatchPending[0] > 0 ? (timestamp + LatchPending[0]) : PCFX_EVENT_NONONO, LatchPending[1] > 0 ? (timestamp + LatchPending[1]) : PCFX_EVENT_NONONO, PCFX_EVENT_NONONO));
}

static void RemakeDevices(int which)
{
 int s = 0;
 int e = TOTAL_PORTS;

 if(which != -1)
 {
  s = which;
  e = which + 1;
 }

 for(int i = s; i < e; i++)
 {
  if(devices[i])
   delete devices[i];
  devices[i] = NULL;

  switch(InputTypes[i])
  {
   default:
   case FXIT_NONE: devices[i] = new PCFX_Input_Device(); break;
   case FXIT_GAMEPAD: devices[i] = PCFXINPUT_MakeGamepad(i); break;
   case FXIT_MOUSE: devices[i] = PCFXINPUT_MakeMouse(i); break;
  }
 }
}

void FXINPUT_SetInput(int port, const char *type, void *ptr)
{
 data_ptr[port] = ptr;

 if(!strcasecmp(type, "mouse"))
 {
  InputTypes[port] = FXIT_MOUSE;
 }
 else if(!strcasecmp(type, "gamepad"))
  InputTypes[port] = FXIT_GAMEPAD;
 else
  InputTypes[port] = FXIT_NONE;
 RemakeDevices(port);
}

uint8 FXINPUT_Read8(uint32 A, const v810_timestamp_t timestamp)
{
 //printf("Read8: %04x\n", A);

 return(FXINPUT_Read16(A &~1, timestamp) >> ((A & 1) * 8));
}

uint16 FXINPUT_Read16(uint32 A, const v810_timestamp_t timestamp)
{
 FXINPUT_Update(timestamp);

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
   PCFXIRQ_Assert(PCFXIRQ_SOURCE_INPUT, FALSE);

 return(ret);
}

void FXINPUT_Write16(uint32 A, uint16 V, const v810_timestamp_t timestamp)
{
 FXINPUT_Update(timestamp);

 //printf("Write16: %04x:%02x, %d\n", A, V, timestamp / 1365);

 //PCFXIRQ_Assert(PCFXIRQ_SOURCE_INPUT, FALSE);
 //if(V != 7 && V != 5)
 //printf("PAD Write16: %04x %04x %d\n", A, V, timestamp);

 switch(A & 0xC0)
 {
  case 0x80:
  case 0x00:
	    {
	     int w = (A & 0x80) >> 7;

	     if((V & 0x1) && !(control[w] & 0x1))
	     {
	      //printf("Start: %d\n", w);
	      if(MultiTapEnabled & (1 << w))
	      {
	       if(V & 0x2)
	        TapCounter[w] = 0;
	      }
	      LatchPending[w] = 1536;
	      PCFX_SetEvent(PCFX_EVENT_PAD, CalcNextEventTS(timestamp));
	     }
	     control[w] = V & 0x7;
	    }
	    break;
 }
}

void FXINPUT_Write8(uint32 A, uint8 V, const v810_timestamp_t timestamp)
{
 FXINPUT_Write16(A, V, timestamp); 
}

void FXINPUT_Frame(void)
{
 for(int i = 0; i < TOTAL_PORTS; i++)
 {
  devices[i]->Frame(data_ptr[i]);
 }
}

static v810_timestamp_t lastts;

v810_timestamp_t FXINPUT_Update(const v810_timestamp_t timestamp)
{
 int32 run_time = timestamp - lastts;

 for(int i = 0; i < 2; i++)
 {
  if(LatchPending[i] > 0)
  {
   LatchPending[i] -= run_time;
   if(LatchPending[i] <= 0)
   {
    //printf("Update: %d, %d\n", i, timestamp / 1365);

    if(MultiTapEnabled & (1 << i))
    {
     if(TapCounter[i] >= TAP_PORTS)
      data_latch[i] = FX_SIG_TAP << 28;
     else
     {
      data_latch[i] = devices[TapMap[i][TapCounter[i]]]->Read();
     }
    }
    else
    {
     data_latch[i] = devices[i]->Read();
    }
    // printf("Moo: %d, %d, %08x\n", i, TapCounter[i], data_latch[i]);
    latched[i] = TRUE;
    control[i] &= ~1;
    PCFXIRQ_Assert(PCFXIRQ_SOURCE_INPUT, TRUE);

    if(MultiTapEnabled & (1 << i))
    {
     if(TapCounter[i] < TAP_PORTS)
     {
      TapCounter[i]++;
     }
    }

   }
  }
 }

 lastts = timestamp;

 return(CalcNextEventTS(timestamp));
}

void FXINPUT_ResetTS(void)
{
 lastts = 0;
}


int FXINPUT_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(TapCounter, 2),
  SFARRAY32(LatchPending, 2),
  SFARRAY(control, 2),
  SFARRAYB(latched, 2),
  SFARRAY32(data_latch, 2),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "INPUT");

 for(int i = 0; i < TOTAL_PORTS; i++)
 {
  char sname[256];
  snprintf(sname, 256, "INPUT%d:%d", i, InputTypes[i]);
  ret &= devices[i]->StateAction(sm, load, data_only, sname);
 }

 if(load)
 {

 }
  
 return(ret);
}

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
  sizeof(PCFX_GamepadIDII) / sizeof(InputDeviceInputInfoStruct),
  PCFX_GamepadIDII,
 },

 // Mouse
 {
  "mouse",
  "Mouse",
  NULL,
  sizeof(PCFX_MouseIDII) / sizeof(InputDeviceInputInfoStruct),
  PCFX_MouseIDII
 }
};

static const InputPortInfoStruct PortInfo[] =
{
 { 0, "port1", "Port 1", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" },
 { 0, "port2", "Port 2", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" },
 { 0, "port3", "Port 3", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" },
 { 0, "port4", "Port 4", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" },
 { 0, "port5", "Port 5", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" },
 { 0, "port6", "Port 6", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" },
 { 0, "port7", "Port 7", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" },
 { 0, "port8", "Port 8", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" },
};

InputInfoStruct PCFXInputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};


static void SyncSettings(void)
{
 MDFNGameInfo->mouse_sensitivity = MDFN_GetSettingF("pcfx.mouse_sensitivity");
 InputDeviceInfo[1].IDII = MDFN_GetSettingB("pcfx.disable_softreset") ? PCFX_GamepadIDII_DSR : PCFX_GamepadIDII;

 MultiTapEnabled = MDFN_GetSettingB("pcfx.input.port1.multitap");
 MultiTapEnabled |= MDFN_GetSettingB("pcfx.input.port2.multitap") << 1;
}

