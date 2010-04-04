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

#include <string.h>
#include <ctype.h>
#include <trio/trio.h>
#include <map>

#include "main.h"
#include "input.h"
#include "input-config.h"
#include "sound.h"
#include "video.h"
#include "joystick.h"
#include "netplay.h"
#include "cheat.h"
#include "fps.h"
#include "debugger.h"
#include "help.h"

#include <math.h>

static int RewindState = 0;
static uint32 MouseData[3];
static double MouseDataRel[2];

static unsigned int autofirefreq;
static unsigned int ckdelay;
static bool fftoggle;

int ConfigDevice(int arg);
static void ConfigDeviceBegin(void);

static void subcon_begin(std::vector<ButtConfig> &bc);
static int subcon(const char *text, std::vector<ButtConfig> &bc, int commandkey);

static char keys[MKK_COUNT];

bool DNeedRewind = FALSE;

static std::string BCToString(const ButtConfig &bc)
{
 std::string string = "";
 char tmp[256];

 if(bc.ButtType == BUTTC_KEYBOARD)
 {
  trio_snprintf(tmp, 256, "keyboard %d", bc.ButtonNum & 0xFFFF);

  string = string + std::string(tmp);

  if(bc.ButtonNum & (4 << 24))
   string = string + "+ctrl";
  if(bc.ButtonNum & (1 << 24))
   string = string + "+alt";
  if(bc.ButtonNum & (2 << 24))
   string = string + "+shift";
 }
 else if(bc.ButtType == BUTTC_JOYSTICK)
 {
  trio_snprintf(tmp, 256, "joystick %16llx %08x", bc.DeviceID, bc.ButtonNum);

  string = string + std::string(tmp);
 }
 else if(bc.ButtType == BUTTC_MOUSE)
 {
  trio_snprintf(tmp, 256, "mouse %d", bc.ButtonNum);

  string = string + std::string(tmp);
 }
 return(string);
}

static std::string BCsToString(const ButtConfig *bc, unsigned int n)
{
 std::string ret = "";

 for(unsigned int x = 0; x < n; x++)
 {
  if(bc[x].ButtType)
  {
   if(x) ret += "~";
   ret += BCToString(bc[x]);
  }
 }

 return(ret);
}

static std::string BCsToString(const std::vector<ButtConfig> &bc)
{
 std::string ret = "";

 for(unsigned int x = 0; x < bc.size(); x++)
 {
  if(x) ret += "~";
  ret += BCToString(bc[x]);
 }

 return(ret);
}


static void StringToBC(const char *string, std::vector<ButtConfig> &bc)
{
 char device_name[64];
 char extra[256];
 ButtConfig tmp_bc;

 do
 {
  if(trio_sscanf(string, "%.63s %.255[^~]", device_name, extra) == 2)
  {
   if(!strcasecmp(device_name, "keyboard"))
   {
    uint32 bnum = atoi(extra);
  
    if(strstr(extra, "+shift"))
     bnum |= 2 << 24;
    if(strstr(extra, "+alt"))
     bnum |= 1 << 24;
    if(strstr(extra, "+ctrl"))
     bnum |= 4 << 24;

    tmp_bc.ButtType = BUTTC_KEYBOARD;
    tmp_bc.DeviceNum = 0;
    tmp_bc.ButtonNum = bnum;
    tmp_bc.DeviceID = 0;

    bc.push_back(tmp_bc);
   }
   else if(!strcasecmp(device_name, "joystick"))
   {
    tmp_bc.ButtType = BUTTC_JOYSTICK;
    tmp_bc.DeviceNum = 0;
    tmp_bc.ButtonNum = 0;
    tmp_bc.DeviceID = 0;

    trio_sscanf(extra, "%16llx %08x", &tmp_bc.DeviceID, &tmp_bc.ButtonNum);

    JoyClearBC(tmp_bc);
    bc.push_back(tmp_bc);
   }
   else if(!strcasecmp(device_name, "mouse"))
   {
    tmp_bc.ButtType = BUTTC_MOUSE;
    tmp_bc.DeviceNum = 0;
    tmp_bc.ButtonNum = 0;
    tmp_bc.DeviceID = 0;
  
    trio_sscanf(extra, "%d", &tmp_bc.ButtonNum);
    bc.push_back(tmp_bc);
   }
  }
  string = strchr(string, '~');
  if(string) string++;
 } while(string);
}

static void CleanSettingName(char *string)
{
 size_t len = strlen(string);

 for(size_t x = 0; x < len; x++)
 {
  if(string[x] >= 0)
  {
   if(string[x] == ' ') string[x] = '_';
   else string[x] = tolower(string[x]);
  }
 }

}


// We have a hardcoded limit of 16 ports here for simplicity.
static unsigned int NumPorts;

//static std::vector<const InputDeviceInfoStruct*> PortPossibleDevices[MDFN_EMULATED_SYSTEM_COUNT][16];
//typedef std::vector<const InputDeviceInfoStruct*>[16]
// An unpacked list of possible devices for each port.

typedef std::vector<const InputDeviceInfoStruct*> SnugglyWuggly;
static std::map<const char *, SnugglyWuggly *> PortPossibleDevices;


static unsigned int PortCurrentDeviceIndex[16]; // index into PortPossibleDevices
static void *PortData[16];
static uint32 PortDataSize[16]; // In bytes, not bits!

static const InputDeviceInfoStruct *PortDevice[16];
static std::vector<char *>PortButtons[16];

static std::vector<std::vector<ButtConfig> > PortButtConfig[16];
static std::vector<int> PortButtConfigOffsets[16];
static std::vector<int> PortButtConfigPrettyPrio[16];


static std::vector<uint32> PortBitOffsets[16]; // within PortData, mask with 0x80000000 for rapid buttons
static std::vector<uint32> PortButtBitOffsets[16];

static std::vector<uint32> PortButtExclusionBitOffsets[16]; // 0xFFFFFFFF represents none
static std::vector<char *> PortButtSettingNames[16];

static void KillPortInfo(void) // Murder it!!
{
 for(unsigned int port = 0; port < NumPorts; port++)
 {
  if(PortData[port])
   free(PortData[port]);
  PortData[port] = NULL;
  PortDataSize[port] = 0;
  PortDevice[port] = NULL;

  for(unsigned int x = 0; x < PortButtons[port].size(); x++)
   free(PortButtons[port][x]);

  PortButtons[port].clear();
  PortButtConfig[port].clear();
  PortButtConfigOffsets[port].clear();
  PortButtConfigPrettyPrio[port].clear();


  PortBitOffsets[port].clear();
  PortButtBitOffsets[port].clear();
  PortButtExclusionBitOffsets[port].clear();

  for(unsigned int x = 0; x < PortButtSettingNames[port].size(); x++)
   free(PortButtSettingNames[port][x]);

  PortButtSettingNames[port].clear();
 }


 NumPorts = 0;
}

static void BuildPortInfo(MDFNGI *gi)
{
 NumPorts = 0;

 while(PortPossibleDevices[gi->shortname][NumPorts].size())
 {
  const InputDeviceInfoStruct *zedevice = NULL;
  char *port_device_name;
  unsigned int device;
  char tmp_setting_name[256];

  if(PortPossibleDevices[gi->shortname][NumPorts].size() > 1)
  {  
   if(CurGame->DesiredInput.size() > NumPorts && CurGame->DesiredInput[NumPorts])
   {
    port_device_name = strdup(CurGame->DesiredInput[NumPorts]);
   }
   else
   {
    trio_snprintf(tmp_setting_name, 256, "%s.input.%s", gi->shortname, gi->InputInfo->Types[NumPorts].ShortName);
    port_device_name = strdup(MDFN_GetSettingS(tmp_setting_name).c_str());
   }
  }
  else
  {
   port_device_name = strdup(PortPossibleDevices[gi->shortname][NumPorts][0]->ShortName);
  }

  for(device = 0; device < PortPossibleDevices[gi->shortname][NumPorts].size(); device++)
  {
   //printf("Port: %d, Dev: %d, Meow: %s\n", NumPorts, device, PortPossibleDevices[gi->shortname][NumPorts][device]->ShortName);
   if(!strcasecmp(PortPossibleDevices[gi->shortname][NumPorts][device]->ShortName, port_device_name))
   {
    zedevice = PortPossibleDevices[gi->shortname][NumPorts][device];
    break;
   }
  }
  free(port_device_name); port_device_name = NULL;

  PortCurrentDeviceIndex[NumPorts] = device;

  if(!zedevice)
  {
   puts("Dackadaf");
   exit(1);
  }

  PortDevice[NumPorts] = zedevice;

  // Figure out how much data should be allocated for each port
  int bit_offset = 0;
  int buttconfig_offset = 0;

  if(!zedevice->PortExpanderDeviceInfo)
  for(int x = 0; x < zedevice->NumInputs; x++)
  {
   if(zedevice->IDII[x].Type == IDIT_BUTTON || zedevice->IDII[x].Type == IDIT_BUTTON_CAN_RAPID || zedevice->IDII[x].Type == IDIT_BUTTON_BYTE)
   {
    if(zedevice->IDII[x].Type == IDIT_BUTTON_BYTE)
     bit_offset = (bit_offset + 7) &~ 7; // Align to an 8-bit boundary

    std::vector<ButtConfig> buttc;
    char buttsn[256];

    trio_snprintf(buttsn, 256, "%s.input.%s.%s.%s", gi->shortname, gi->InputInfo->Types[NumPorts].ShortName, zedevice->ShortName, zedevice->IDII[x].SettingName);
    CleanSettingName(buttsn);

    //printf("Buttsn: %s, %s\n", buttsn, MDFN_GetSettingS(buttsn).c_str());
    PortButtSettingNames[NumPorts].push_back(strdup(buttsn));
    StringToBC(MDFN_GetSettingS(buttsn).c_str(), buttc);

    PortButtConfig[NumPorts].push_back(buttc);
    PortBitOffsets[NumPorts].push_back(bit_offset);
    PortButtBitOffsets[NumPorts].push_back(bit_offset);

    PortButtons[NumPorts].push_back(strdup(zedevice->IDII[x].Name));
    PortButtConfigOffsets[NumPorts].push_back(buttconfig_offset);

    PortButtConfigPrettyPrio[NumPorts].push_back(zedevice->IDII[x].ConfigOrder);
    buttconfig_offset++;

    if(zedevice->IDII[x].Type == IDIT_BUTTON_CAN_RAPID)
    {
     buttc.clear();

     trio_snprintf(buttsn, 256, "%s.input.%s.%s.rapid_%s", gi->shortname, gi->InputInfo->Types[NumPorts].ShortName, zedevice->ShortName, zedevice->IDII[x].SettingName);
     CleanSettingName(buttsn);
     PortButtSettingNames[NumPorts].push_back(strdup(buttsn));
     StringToBC(MDFN_GetSettingS(buttsn).c_str(), buttc);

     PortButtConfig[NumPorts].push_back(buttc);
     PortBitOffsets[NumPorts].push_back(bit_offset | 0x80000000);
     PortButtBitOffsets[NumPorts].push_back(bit_offset | 0x80000000);

     PortButtons[NumPorts].push_back(trio_aprintf("Rapid %s", zedevice->IDII[x].Name));
     PortButtConfigOffsets[NumPorts].push_back(buttconfig_offset);

     PortButtConfigPrettyPrio[NumPorts].push_back(zedevice->IDII[x].ConfigOrder);
     buttconfig_offset++;
    }

    if(zedevice->IDII[x].Type == IDIT_BUTTON_BYTE)
    {
     bit_offset += 8;
    }
    else
    {
     bit_offset += 1;
    }

   }
   else if(zedevice->IDII[x].Type == IDIT_BYTE_SPECIAL)
   {
    PortBitOffsets[NumPorts].push_back(bit_offset);
    bit_offset += 8;
   }
   else // axis, uint32!
   {
    bit_offset = (bit_offset + 31) &~ 31; // Align it to a 32-bit boundary

    PortBitOffsets[NumPorts].push_back(bit_offset);
    bit_offset += 32;
   }
  }
  //printf("Love: %d %d %d %s\n", NumPorts, bit_offset, zedevice->NumInputs, zedevice->ShortName);

  for(unsigned int x = 0; x < PortButtConfigPrettyPrio[NumPorts].size(); x++)
  {
   int this_prio = PortButtConfigPrettyPrio[NumPorts][x];

   if(this_prio >= 0)
   {
    bool FooFound = FALSE;

    // First, see if any duplicate priorities come after this one! or something
    for(unsigned int i = x + 1; i < PortButtConfigPrettyPrio[NumPorts].size(); i++)
     if(PortButtConfigPrettyPrio[NumPorts][i] == this_prio)
     {
      FooFound = TRUE;
      break;
     }

    // Now adjust all priorities >= this_prio except for 'x' by +1
    if(FooFound)
    {
     for(unsigned int i = 0; i < PortButtConfigPrettyPrio[NumPorts].size(); i++)
      if(i != x && PortButtConfigPrettyPrio[NumPorts][i] >= this_prio)
       PortButtConfigPrettyPrio[NumPorts][i]++;
    }
    
   } 
  }

  PortDataSize[NumPorts] = (bit_offset + 7) / 8;

  if(!PortDataSize[NumPorts])
   PortData[NumPorts] = NULL;
  else
  {
   PortData[NumPorts] = malloc(PortDataSize[NumPorts]);
   memset(PortData[NumPorts], 0, PortDataSize[NumPorts]);
  }

  // Now, search for exclusion buttons
  if(!zedevice->PortExpanderDeviceInfo)
  for(int x = 0; x < zedevice->NumInputs; x++)
  {
   if(zedevice->IDII[x].Type == IDIT_BUTTON || zedevice->IDII[x].Type == IDIT_BUTTON_CAN_RAPID)
   {
    if(zedevice->IDII[x].ExcludeName)
    {
     int bo = 0xFFFFFFFF;

     for(unsigned int sub_x = 0; sub_x < PortButtSettingNames[NumPorts].size(); sub_x++)
     {
      if(!strcasecmp(zedevice->IDII[x].ExcludeName, strrchr(PortButtSettingNames[NumPorts][sub_x], '.') + 1 ))
       bo = PortButtBitOffsets[NumPorts][sub_x];
     }
     //printf("%s %s %d\n", zedevice->IDII[x].SettingName, zedevice->IDII[x].ExcludeName, bo);
     PortButtExclusionBitOffsets[NumPorts].push_back(bo);
    }
    else
     PortButtExclusionBitOffsets[NumPorts].push_back(0xFFFFFFFF);

    if(zedevice->IDII[x].Type == IDIT_BUTTON_CAN_RAPID) // FIXME in the future, but I doubt we'll ever have rapid-fire directional buttons!
     PortButtExclusionBitOffsets[NumPorts].push_back(0xFFFFFFFF);

   }
   else if(zedevice->IDII[x].Type == IDIT_BUTTON_BYTE)
    PortButtExclusionBitOffsets[NumPorts].push_back(0xFFFFFFFF);

  }

  MDFNI_SetInput(NumPorts, zedevice->ShortName, PortData[NumPorts], PortDataSize[NumPorts]);
  NumPorts++;
 }
}

static void IncSelectedDevice(unsigned int port)
{
 if(MDFNDnetplay)
 {
  MDFN_DispMessage(_("Cannot change input device during netplay."));
 }
 else if(RewindState)
 {
  MDFN_DispMessage(_("Cannot change input device while state rewinding is active."));
 }
 else if(PortPossibleDevices[CurGame->shortname][port].size() > 1)
 {
  char tmp_setting_name[256];

  if(CurGame->DesiredInput.size() > port)
   CurGame->DesiredInput[port] = NULL;

  trio_snprintf(tmp_setting_name, 256, "%s.input.%s", CurGame->shortname, CurGame->InputInfo->Types[port].ShortName);

  PortCurrentDeviceIndex[port] = (PortCurrentDeviceIndex[port] + 1) % PortPossibleDevices[CurGame->shortname][port].size();

  const char *devname = PortPossibleDevices[CurGame->shortname][port][PortCurrentDeviceIndex[port]]->ShortName;

  KillPortInfo();
  MDFNI_SetSetting(tmp_setting_name, devname);
  BuildPortInfo(CurGame);

  MDFN_DispMessage(_("%s selected on port %d"), PortPossibleDevices[CurGame->shortname][port][PortCurrentDeviceIndex[port]]->FullName, port + 1);
 }
}

#define MK(x)		{ BUTTC_KEYBOARD, 0, MKK(x) }

#define MK_CK(x)        { { BUTTC_KEYBOARD, 0, MKK(x) }, { 0, 0, 0 } }
#define MK_CK2(x,y)        { { BUTTC_KEYBOARD, 0, MKK(x) }, { BUTTC_KEYBOARD, 0, MKK(y) } }

#define MK_CK_SHIFT(x)	{ { BUTTC_KEYBOARD, 0, MKK(x) | (2<<24) }, { 0, 0, 0 } }
#define MK_CK_ALT(x)	{ { BUTTC_KEYBOARD, 0, MKK(x) | (1<<24) }, { 0, 0, 0 } }
#define MK_CK_ALT_SHIFT(x)    { { BUTTC_KEYBOARD, 0, MKK(x) | (3<<24) }, { 0, 0, 0 } }
#define MK_CK_CTRL(x)	{ { BUTTC_KEYBOARD, 0, MKK(x) | (4 << 24) },  { 0, 0, 0 } }
#define MK_CK_CTRL_SHIFT(x) { { BUTTC_KEYBOARD, 0, MKK(x) | (6 << 24) },  { 0, 0, 0 } }

#define MKZ()   {0, 0, 0}

typedef enum {
	_CK_FIRST = 0,
        CK_SAVE_STATE = 0,
	CK_LOAD_STATE,
	CK_SAVE_MOVIE,
	CK_LOAD_MOVIE,
	CK_STATE_REWIND_TOGGLE,
	CK_0,CK_1,CK_2,CK_3,CK_4,CK_5,CK_6,CK_7,CK_8,CK_9,
	CK_M0,CK_M1,CK_M2,CK_M3,CK_M4,CK_M5,CK_M6,CK_M7,CK_M8,CK_M9,
	CK_TL1, CK_TL2, CK_TL3, CK_TL4, CK_TL5, CK_TL6, CK_TL7, CK_TL8, CK_TL9,
	CK_TAKE_SNAPSHOT,
	CK_TOGGLE_FS,
	CK_FAST_FORWARD,

	CK_INSERT_COIN,
	CK_TOGGLE_DIPVIEW,
	CK_SELECT_DISK,
	CK_INSERTEJECT_DISK,
	CK_ACTIVATE_BARCODE,

        CK_TOGGLE_GRAB_INPUT,
	CK_TOGGLE_CDISABLE,
	CK_INPUT_CONFIG1,
	CK_INPUT_CONFIG2,
	CK_INPUT_CONFIGC,
        CK_INPUT_CONFIG3,
        CK_INPUT_CONFIG4,
	CK_INPUT_CONFIG5,
	CK_RESET,
	CK_POWER,
	CK_EXIT,
	CK_STATE_REWIND,
	CK_ROTATESCREEN,
	CK_TOGGLENETVIEW,
	CK_ADVANCE_FRAME,
	CK_RUN_NORMAL,
	CK_TOGGLECHEATVIEW,
	CK_TOGGLE_CHEAT_ACTIVE,
	CK_TOGGLE_FPS_VIEW,
	CK_TOGGLE_DEBUGGER,
	CK_STATE_SLOT_DEC,
        CK_STATE_SLOT_INC,
	CK_TOGGLE_HELP,
	CK_DEVICE_SELECT1,
        CK_DEVICE_SELECT2,
        CK_DEVICE_SELECT3,
        CK_DEVICE_SELECT4,
        CK_DEVICE_SELECT5,
	_CK_COUNT
} CommandKey;

typedef struct __COKE
{
 ButtConfig bc[2];
 const char *text;
 unsigned int system;
 bool SkipCKDelay;
 const char *description;
} COKE;

static COKE CKeys[_CK_COUNT]	=
{
	{ MK_CK(F5), "save_state", ~0, 1, gettext_noop("Save state") },
	{ MK_CK(F7), "load_state", ~0, 0, gettext_noop("Load state") },
	{ MK_CK_SHIFT(F5), "save_movie", ~0, 1, gettext_noop("Save movie") },
	{ MK_CK_SHIFT(F7), "load_movie", ~0, 0, gettext_noop("Load movie") },
	{ MK_CK_ALT(s), "toggle_state_rewind", ~0, 1, gettext_noop("Toggle state rewind functionality") },

	{ MK_CK(0), "0", ~0, 1, gettext_noop("Save state 0 select")},
        { MK_CK(1), "1", ~0, 1, gettext_noop("Save state 1 select")},
        { MK_CK(2), "2", ~0, 1, gettext_noop("Save state 2 select")},
        { MK_CK(3), "3", ~0, 1, gettext_noop("Save state 3 select")},
        { MK_CK(4), "4", ~0, 1, gettext_noop("Save state 4 select")},
        { MK_CK(5), "5", ~0, 1, gettext_noop("Save state 5 select")},
        { MK_CK(6), "6", ~0, 1, gettext_noop("Save state 6 select")},
        { MK_CK(7), "7", ~0, 1, gettext_noop("Save state 7 select")},
        { MK_CK(8), "8", ~0, 1, gettext_noop("Save state 8 select")},
        { MK_CK(9), "9", ~0, 1, gettext_noop("Save state 9 select")},

	{ MK_CK_SHIFT(0), "m0", ~0, 1, gettext_noop("Movie 0 select") },
        { MK_CK_SHIFT(1), "m1", ~0, 1, gettext_noop("Movie 1 select")  },
        { MK_CK_SHIFT(2), "m2", ~0, 1, gettext_noop("Movie 2 select")  },
        { MK_CK_SHIFT(3), "m3", ~0, 1, gettext_noop("Movie 3 select")  },
        { MK_CK_SHIFT(4), "m4", ~0, 1, gettext_noop("Movie 4 select")  },
        { MK_CK_SHIFT(5), "m5", ~0, 1, gettext_noop("Movie 5 select")  },
        { MK_CK_SHIFT(6), "m6", ~0, 1, gettext_noop("Movie 6 select")  },
        { MK_CK_SHIFT(7), "m7", ~0, 1, gettext_noop("Movie 7 select")  },
        { MK_CK_SHIFT(8), "m8", ~0, 1, gettext_noop("Movie 8 select")  },
        { MK_CK_SHIFT(9), "m9", ~0, 1, gettext_noop("Movie 9 select")  },

        { MK_CK_CTRL(1), "tl1", ~0, 1, gettext_noop("Toggle graphics layer 1")  },
        { MK_CK_CTRL(2), "tl2", ~0, 1, gettext_noop("Toggle graphics layer 2") },
        { MK_CK_CTRL(3), "tl3", ~0, 1, gettext_noop("Toggle graphics layer 3") },
        { MK_CK_CTRL(4), "tl4", ~0, 1, gettext_noop("Toggle graphics layer 4") },
        { MK_CK_CTRL(5), "tl5", ~0, 1, gettext_noop("Toggle graphics layer 5") },
        { MK_CK_CTRL(6), "tl6", ~0, 1, gettext_noop("Toggle graphics layer 6") },
        { MK_CK_CTRL(7), "tl7", ~0, 1, gettext_noop("Toggle graphics layer 7") },
        { MK_CK_CTRL(8), "tl8", ~0, 1, gettext_noop("Toggle graphics layer 8") },
        { MK_CK_CTRL(9), "tl9", ~0, 1, gettext_noop("Toggle graphics layer 9") },

	{ MK_CK(F9), "take_snapshot", ~0, 1, gettext_noop("Take screen snapshot") },
	{ MK_CK_ALT(RETURN), "toggle_fs", ~0, 1, gettext_noop("Toggle fullscreen mode") },
	{ MK_CK(BACKQUOTE), "fast_forward", ~0, 1, gettext_noop("Fast-forward") },
	{ MK_CK(F8), "insert_coin", ~0, 1, gettext_noop("Insert coin") },
	{ MK_CK(F6), "toggle_dipview", ~0, 1, gettext_noop("Toggle DIP switch view") },
	{ MK_CK(F6), "select_disk", ~0, 1, gettext_noop("Select disk/disc") },
	{ MK_CK(F8), "insert_eject_disk", ~0, 1, gettext_noop("Insert/Eject disk/disc") },
	{ MK_CK(F8), "activate_barcode", ~0, 1, gettext_noop("Activate barcode(for Famicom)") },
	{ MK_CK(SCROLLOCK), "toggle_grab_input", ~0, 1, gettext_noop("Grab input") },
	{ MK_CK_SHIFT(SCROLLOCK), "toggle_cidisable", ~0, 1, gettext_noop("Grab input and disable commands") },
	{ MK_CK_ALT_SHIFT(1), "input_config1", ~0, 0, gettext_noop("Configure buttons on virtual port 1") },
	{ MK_CK_ALT_SHIFT(2), "input_config2", ~0, 0, gettext_noop("Configure buttons on virtual port 2")  },
        { MK_CK(F2), "input_configc", ~0, 0, gettext_noop("Configure command key") },
        { MK_CK_ALT_SHIFT(3), "input_config3", ~0, 0, gettext_noop("Configure buttons on virtual port 3")  },
        { MK_CK_ALT_SHIFT(4), "input_config4", ~0, 0, gettext_noop("Configure buttons on virtual port 4")  },
	{ MK_CK_ALT_SHIFT(5), "input_config5", ~0, 0, gettext_noop("Configure buttons on virtual port 5")  },
	{ MK_CK(F10), "reset", ~0, 0, gettext_noop("Reset") },
	{ MK_CK(F11), "power", ~0, 0, gettext_noop("Power toggle") },
	{ MK_CK2(F12, ESCAPE), "exit", ~0, 0, gettext_noop("Exit") },
	{ MK_CK(BACKSPACE), "state_rewind", ~0, 1, gettext_noop("Rewind") },
	{ MK_CK(F8), "rotatescreen", ~0, 1, gettext_noop("Rotate screen") },

	{ MK_CK(t), "togglenetview", ~0, 1, gettext_noop("Toggle netplay console")},
	{ MK_CK_ALT(a), "advance_frame", ~0, 1, gettext_noop("Advance frame") },
	{ MK_CK_ALT(r), "run_normal", ~0, 1, gettext_noop("Return to normal mode after advancing frames") },
	{ MK_CK_ALT(c), "togglecheatview", ~0, 1, gettext_noop("Toggle cheat console") },
	{ MK_CK_ALT(t), "togglecheatactive", ~0, 1, gettext_noop("Enable/Disable cheats") },
        { MK_CK_SHIFT(F1), "toggle_fps_view", ~0, 1, gettext_noop("Toggle frames-per-second display") },
	{ MK_CK_ALT(d), "toggle_debugger", ~0, 1, gettext_noop("Toggle debugger") },
	{ MK_CK(MINUS), "state_slot_dec", ~0, 1, gettext_noop("Decrease selected save state slot by 1") },
	{ MK_CK(EQUALS), "state_slot_inc", ~0, 1, gettext_noop("Increase selected save state slot by 1") },
	{ MK_CK(F1), "toggle_help", ~0, 1, gettext_noop("Toggle help screen") },
	{ MK_CK_CTRL_SHIFT(1), "device_select1", ~0, 1, gettext_noop("Select virtual device on virtual input port 1") },
        { MK_CK_CTRL_SHIFT(2), "device_select2", ~0, 1, gettext_noop("Select virtual device on virtual input port 2") },
        { MK_CK_CTRL_SHIFT(3), "device_select3", ~0, 1, gettext_noop("Select virtual device on virtual input port 3") },
        { MK_CK_CTRL_SHIFT(4), "device_select4", ~0, 1, gettext_noop("Select virtual device on virtual input port 4") },
        { MK_CK_CTRL_SHIFT(5), "device_select5", ~0, 1, gettext_noop("Select virtual device on virtual input port 5") },
};

static std::vector<ButtConfig> CKeysBC[_CK_COUNT];

static int CKeysLastState[_CK_COUNT];
static uint32 CKeysPressTime[_CK_COUNT];
static uint32 CurTicks = 0;	// Optimization, SDL_GetTicks() might be slow on some platforms?

static int CK_Check(CommandKey which)
{
 int last = CKeysLastState[which];
 int tmp_ckdelay = ckdelay;

 if(CKeys[which].SkipCKDelay)
  tmp_ckdelay = 0;

 if((CKeysLastState[which] = DTestButtonCombo(CKeysBC[which], keys)))
 {
  if(!last)
   CKeysPressTime[which] = CurTicks;
 }
 else
  CKeysPressTime[which] = 0xFFFFFFFF;

 if(CurTicks >= ((int64)CKeysPressTime[which] + tmp_ckdelay))
 {
  CKeysPressTime[which] = 0xFFFFFFFF;
  return(1);
 }
 return(0);
}

static int CK_CheckActive(CommandKey which)
{
 return(DTestButtonCombo(CKeysBC[which], keys));
}

int NoWaiting = 0;

static int DIPS=0;

#define KEY(__a) keys[MKK(__a)]

static int cidisabled=0;
static int inff = 0;

typedef enum
{
	none,
	Port1,
	Port2,
	Port3,
	Port4,
	Port5,
	Command
} ICType;

static ICType IConfig = none;
static int ICLatch;

static bool NeedBLExitNow = 0;
bool MDFND_ExitBlockingLoop(void)
{
 SDL_Delay(1);
 memcpy(keys, SDL_GetKeyState(0), MKK_COUNT);

 CurTicks = SDL_GetTicks();

 if(CK_Check(CK_EXIT))
 {
       SDL_Event evt;
       NeedBLExitNow = 1;
       evt.quit.type = SDL_QUIT;
       SDL_PushEvent(&evt);
 }
 return(NeedBLExitNow);
}

static void KeyboardCommands(void)
{
  memcpy(keys, SDL_GetKeyState(0), MKK_COUNT);

  CurTicks = SDL_GetTicks();

  if(IConfig == none && !(cidisabled & 0x1))
  {
   if(CK_Check(CK_TOGGLE_HELP))
    Help_Toggle();

   if(!IsConsoleCheatConfigActive() && !MDFNDnetplay)
   {
    if(CK_Check(CK_TOGGLE_DEBUGGER))
    {
     Debugger_Toggle();
    }
   }
   if(!Debugger_IsActive() && !MDFNDnetplay)
    if(CK_Check(CK_TOGGLECHEATVIEW))
    {
     ShowConsoleCheatConfig(!IsConsoleCheatConfigActive());
    }

   if(!(cidisabled & 1))
   {
    if(CK_Check(CK_EXIT))
    {
     SendCEvent(CEVT_WANT_EXIT, NULL, NULL);
    }
   }

   if(Netplay_IsTextInput() || IsConsoleCheatConfigActive())
   {
    memset(keys, 0, sizeof(keys)); // This effectively disables keyboard input, but still
				   // allows physical joystick input when in the chat mode.
    //return;
   }

   if(!IsConsoleCheatConfigActive() && !Debugger_IsActive())
    if(CK_Check(CK_TOGGLENETVIEW))
    {
     Netplay_ToggleTextView();
    }


   if(Debugger_IsActive())
   {
    static char keys_backup[MKK_COUNT];
    memcpy(keys_backup, keys, MKK_COUNT);
    memset(keys, 0, sizeof(keys));

    keys[SDLK_F1] = keys_backup[SDLK_F1];
    keys[SDLK_F2] = keys_backup[SDLK_F2];
    keys[SDLK_F3] = keys_backup[SDLK_F3];
    keys[SDLK_F4] = keys_backup[SDLK_F4];
    keys[SDLK_F5] = keys_backup[SDLK_F5];
    keys[SDLK_F6] = keys_backup[SDLK_F6];
    keys[SDLK_F7] = keys_backup[SDLK_F7];
    keys[SDLK_F8] = keys_backup[SDLK_F8];
    keys[SDLK_F9] = keys_backup[SDLK_F9];
    keys[SDLK_F10] = keys_backup[SDLK_F10];
    keys[SDLK_F11] = keys_backup[SDLK_F11];
    keys[SDLK_F12] = keys_backup[SDLK_F12];
    keys[SDLK_F13] = keys_backup[SDLK_F13];
    keys[SDLK_F14] = keys_backup[SDLK_F14];
    keys[SDLK_F15] = keys_backup[SDLK_F15];
   }
  }

   if(IConfig == Port1)
   {
    if(CK_Check(CK_INPUT_CONFIG1))
    {
     IConfig = none;
     SetJoyReadMode(1);
     MDFNI_DispMessage(_("Configuration interrupted."));
    }
    else if(ConfigDevice(0))
    {
     IConfig = none;
     SetJoyReadMode(1);
    }
   }
   else if(IConfig == Port2)
   {
    if(CK_Check(CK_INPUT_CONFIG2))
    {
     IConfig = none;
     SetJoyReadMode(1);
     MDFNI_DispMessage(_("Configuration interrupted."));
    }
    else if(ConfigDevice(1))
    {
     IConfig = none;
     SetJoyReadMode(1);
    }
   }
   if(IConfig == Port3)
   {
    if(CK_Check(CK_INPUT_CONFIG3))
    {
     IConfig = none;
     SetJoyReadMode(1);
     MDFNI_DispMessage(_("Configuration interrupted."));
    }
    else if(ConfigDevice(2))
    {
     IConfig = none;
     SetJoyReadMode(1);
    }
   }
   else if(IConfig == Port4)
   {
    if(CK_Check(CK_INPUT_CONFIG4))
    {
     IConfig = none;
     SetJoyReadMode(1);
     MDFNI_DispMessage(_("Configuration interrupted."));
    }
    else if(ConfigDevice(3))
    {
     IConfig = none;
     SetJoyReadMode(1);
    }
   }
   else if(IConfig == Port5)
   {
    if(CK_Check(CK_INPUT_CONFIG5))
    {
     IConfig = none;
     SetJoyReadMode(1);
     MDFNI_DispMessage(_("Configuration interrupted."));
    }
    else if(ConfigDevice(4))
    {
     IConfig = none;
     SetJoyReadMode(1);
    }
   }
   else if(IConfig == Command)
   {
    if(ICLatch != -1)
    {
     if(subcon(CKeys[ICLatch].text, CKeysBC[ICLatch], 1))
     {
      MDFNI_DispMessage(_("Configuration finished."));
      IConfig = none;
      SetJoyReadMode(1);
      CKeysLastState[ICLatch] = 1;	// We don't want to accidentally
      return;				// trigger the command. :b
     }
    }
    else
    {
     int x;
     MDFNI_DispMessage(_("Press a command key now."));
     for(x = (int)_CK_FIRST; x < (int)_CK_COUNT; x++)
      if(CK_Check((CommandKey)x))
      {      
       ICLatch = x;
       subcon_begin(CKeysBC[ICLatch]);
       break;
      }
    }
   }

  if(CK_Check(CK_TOGGLE_GRAB_INPUT))
  {
   cidisabled ^= 2;

   if(cidisabled & 0x2)
    MDFNI_DispMessage(_("Input grabbing enabled."));
   else
    MDFNI_DispMessage(_("Input grabbing disabled."));

   SDL_Event evt;
   evt.user.type = SDL_USEREVENT;
   evt.user.code = CEVT_SET_GRAB_INPUT;
   evt.user.data1 = malloc(1);

   *(int *)evt.user.data1 = cidisabled & 0x2;
   SDL_PushEvent(&evt);
  }

  if(CK_Check(CK_TOGGLE_CDISABLE))
  {
   cidisabled = cidisabled ? 0 : 0x3;

   if(cidisabled)
    MDFNI_DispMessage(_("Command processing disabled."));
   else
    MDFNI_DispMessage(_("Command processing enabled."));

   SDL_Event evt;
   evt.user.type = SDL_USEREVENT;
   evt.user.code = CEVT_SET_GRAB_INPUT;
   evt.user.data1 = malloc(1);

   *(int *)evt.user.data1 = cidisabled;
   SDL_PushEvent(&evt);
  }

  if(cidisabled & 0x1) return;

  if(IConfig != none)
   return;

  if(CK_Check(CK_TOGGLE_CHEAT_ACTIVE))
  {
   bool isactive = MDFN_GetSettingB("cheats");
   
   isactive = !isactive;
   
   MDFNI_SetSettingB("cheats", isactive);

   if(isactive)
    MDFNI_DispMessage(_("Application of cheats enabled."));
   else
    MDFNI_DispMessage(_("Application of cheats disabled."));
  }

  if(CK_Check(CK_TOGGLE_FPS_VIEW))
   FPS_ToggleView();

  if(CK_Check(CK_TOGGLE_FS)) 
  {
   GT_ToggleFS();
   //SDL_Event evt;
   //evt.user.type = SDL_USEREVENT;
   //evt.user.code = CEVT_TOGGLEFS;
   //SDL_PushEvent(&evt);
  }

  if(!CurGame)
	return;
  
  if(!MDFNDnetplay)
  {
   if(CK_Check(CK_ADVANCE_FRAME))
    DoFrameAdvance();

   if(CK_Check(CK_RUN_NORMAL))
    DoRunNormal();
  }

  if(!Debugger_IsActive()) // We don't want to start button configuration when the debugger is active!
  {
   if(CK_Check(CK_INPUT_CONFIG1))
   {
    if(!PortButtConfig[0].size())
    {
     MDFN_DispMessage(_("No buttons to configure for input port %d!"), 1);
    }
    else
    {
     SetJoyReadMode(0);
     ConfigDeviceBegin();
     IConfig = Port1;
    }
   }
   else if(CK_Check(CK_INPUT_CONFIG2))
   {
    if(!PortButtConfig[1].size())
    {
     MDFN_DispMessage(_("No buttons to configure for input port %d!"), 2);
    }
    else
    {
     SetJoyReadMode(0);
     ConfigDeviceBegin();
     IConfig = Port2;
    }
   }
   else if(CK_Check(CK_INPUT_CONFIG3))
   {
    if(!PortButtConfig[2].size())
    {
     MDFN_DispMessage(_("No buttons to configure for input port %d!"), 3);
    }
    else
    {
     SetJoyReadMode(0);
     ConfigDeviceBegin();
     IConfig = Port3;
    }
   }
   else if(CK_Check(CK_INPUT_CONFIG4))
   {
    if(!PortButtConfig[3].size())
    {
     MDFN_DispMessage(_("No buttons to configure for input port %d!"), 4);
    }
    else
    {
     SetJoyReadMode(0);
     ConfigDeviceBegin();
     IConfig = Port4;
    }
   }
   else if(CK_Check(CK_INPUT_CONFIG5))
   {
    if(!PortButtConfig[4].size())
    {
     MDFN_DispMessage(_("No buttons to configure for input port %d!"), 5);
    }
    else
    {
     SetJoyReadMode(0);
     ConfigDeviceBegin();
     IConfig = Port5;
    }
   }
   else if(CK_Check(CK_INPUT_CONFIGC))
   {
    SetJoyReadMode(0);
    ConfigDeviceBegin();
    ICLatch = -1;
    IConfig = Command;
   }
  }

 
 if(!strcmp(CurGame->shortname, "lynx") || !strcmp(CurGame->shortname, "wswan"))
 {
  if(CK_Check(CK_ROTATESCREEN))
  {
   //SDL_Event evt;

   if(CurGame->rotated == MDFN_ROTATE0)
    CurGame->rotated = MDFN_ROTATE90;
   else if(CurGame->rotated == MDFN_ROTATE90)
    CurGame->rotated = MDFN_ROTATE270;
   else if(CurGame->rotated == MDFN_ROTATE270)
    CurGame->rotated = MDFN_ROTATE0;

   //evt.user.type = SDL_USEREVENT;
   //evt.user.code = CEVT_VIDEOSYNC;
   //SDL_PushEvent(&evt);
   GT_ReinitVideo();
  }
 }
  if(CK_CheckActive(CK_STATE_REWIND))
	DNeedRewind = TRUE;
  else
	DNeedRewind = FALSE;

  if(CK_Check(CK_STATE_REWIND_TOGGLE))
  {
   RewindState ^= 1;
   MDFNI_EnableStateRewind(RewindState);

   MDFNI_DispMessage(RewindState?_("State rewinding functionality enabled.") : _("State rewinding functionality disabled."));
  }

  if(fftoggle)
  {
   if(CK_Check(CK_FAST_FORWARD))
   {
    inff = !inff;
    RefreshThrottleFPS(inff ? MDFN_GetSettingUI("ffspeed") : 1);
   }
  }
  else
  {
   if(CK_CheckActive(CK_FAST_FORWARD))
   {
    if(!inff) 
    {
     RefreshThrottleFPS(MDFN_GetSettingUI("ffspeed"));
     inff = 1;
    }
   }
   else
   {
    if(inff) 
    {
     RefreshThrottleFPS(1);
    }
    inff = 0;
   }
  }

  if(CurGame->GameType == GMT_DISK || CurGame->GameType == GMT_CDROM)
  {
   if(CK_Check(CK_SELECT_DISK)) 
   {
    LockGameMutex(1);
    MDFNI_DiskSelect();
    LockGameMutex(0);
   }
   if(CK_Check(CK_INSERTEJECT_DISK)) 
   {
    LockGameMutex(1);
    MDFNI_DiskInsert(0);
    LockGameMutex(0);
   }
  }

  if(CurGame->GameType != GMT_PLAYER)
  {
   if(CK_Check(CK_DEVICE_SELECT1))
    IncSelectedDevice(0);
   if(CK_Check(CK_DEVICE_SELECT2))
    IncSelectedDevice(1);
   if(CK_Check(CK_DEVICE_SELECT3))
    IncSelectedDevice(2);
   if(CK_Check(CK_DEVICE_SELECT4))
    IncSelectedDevice(3);
   if(CK_Check(CK_DEVICE_SELECT5))
    IncSelectedDevice(4);
  }

  if(CK_Check(CK_TAKE_SNAPSHOT)) 
	pending_snapshot = 1;

  if(CurGame->GameType != GMT_PLAYER)
  {
   if(CK_Check(CK_SAVE_STATE))
	pending_save_state = 1;

   if(CK_Check(CK_SAVE_MOVIE))
	pending_save_movie = 1;

   if(CK_Check(CK_LOAD_STATE))
   {
	LockGameMutex(1);
	MDFNI_LoadState(NULL, NULL);
	Debugger_ForceStepIfStepping();
	LockGameMutex(0);
   }

   if(CK_Check(CK_LOAD_MOVIE))
   {
	LockGameMutex(1);
	MDFNI_LoadMovie(NULL);
	Debugger_ForceStepIfStepping();
	LockGameMutex(0);
   }

   if(CK_Check(CK_TL1))
    MDFNI_ToggleLayer(0);
   if(CK_Check(CK_TL2))
    MDFNI_ToggleLayer(1);
   if(CK_Check(CK_TL3))
    MDFNI_ToggleLayer(2);
   if(CK_Check(CK_TL4))
    MDFNI_ToggleLayer(3);
   if(CK_Check(CK_TL5))
    MDFNI_ToggleLayer(4);
   if(CK_Check(CK_TL6))
    MDFNI_ToggleLayer(5);
   if(CK_Check(CK_TL7))
    MDFNI_ToggleLayer(6);
   if(CK_Check(CK_TL8))
    MDFNI_ToggleLayer(7);
   if(CK_Check(CK_TL9))
    MDFNI_ToggleLayer(8);

   if(CK_Check(CK_STATE_SLOT_INC))
   {
    MDFNI_SelectState(666 + 1);
   }

   if(CK_Check(CK_STATE_SLOT_DEC))
   {
    MDFNI_SelectState(666 - 1);
   }
  }

  if(CK_Check(CK_RESET))
  {
	LockGameMutex(1);
	MDFNI_Reset();
	LockGameMutex(0);
	Debugger_ForceStepIfStepping();
  }

  if(CK_Check(CK_POWER))
  {
	LockGameMutex(1);
	MDFNI_Power();
	LockGameMutex(0);
	Debugger_ForceStepIfStepping();
  }

  if(CurGame->GameType == GMT_ARCADE)
  {
	#ifdef WANT_NES_EMU
	if(CK_Check(CK_INSERT_COIN))
		MDFNI_VSUniCoin();
	if(CK_Check(CK_TOGGLE_DIPVIEW))
        {
	 DIPS^=1;
	 MDFNI_VSUniToggleDIPView();
	}
	if(!(DIPS&1)) goto DIPSless;
	if(CK_Check(CK_1)) MDFNI_VSUniToggleDIP(0);
	if(CK_Check(CK_2)) MDFNI_VSUniToggleDIP(1);
	if(CK_Check(CK_3)) MDFNI_VSUniToggleDIP(2);
	if(CK_Check(CK_4)) MDFNI_VSUniToggleDIP(3);
	if(CK_Check(CK_5)) MDFNI_VSUniToggleDIP(4);
	if(CK_Check(CK_6)) MDFNI_VSUniToggleDIP(5);
	if(CK_Check(CK_7)) MDFNI_VSUniToggleDIP(6);
	if(CK_Check(CK_8)) MDFNI_VSUniToggleDIP(7);
	#endif
  }
  else
  {
   #ifdef WANT_NES_EMU
   static uint8 bbuf[32];
   static int bbuft;
   static int barcoder = 0;

   if(!strcmp(CurGame->shortname, "nes") && (!strcmp(PortDevice[4]->ShortName, "bworld") || (CurGame->cspecial && !strcasecmp(CurGame->cspecial, "datach"))))
   {
    if(CK_Check(CK_ACTIVATE_BARCODE))
    {
     barcoder ^= 1;
     if(!barcoder)
     {
      if(!strcmp(PortDevice[4]->ShortName, "bworld"))
      {
       uint8 *meowptr = (uint8 *)PortData[4];

       memset(meowptr + 1, 0, 13);
       meowptr[0] = 1;

       strncpy((char*)meowptr + 1, (char *)bbuf, 13);
      }
      else
       MDFNI_DatachSet(bbuf);
      MDFNI_DispMessage(_("Barcode Entered"));
     } 
     else { bbuft = 0; MDFNI_DispMessage(_("Enter Barcode"));}
    }
   } 
   else 
    barcoder = 0;

   #define SSM(x) { if(bbuft < 13) {bbuf[bbuft++] = '0' + x; bbuf[bbuft] = 0;} MDFNI_DispMessage(_("Barcode: %s"),bbuf); }

   DIPSless:

   if(barcoder)
   {
    if(CK_Check(CK_0)) SSM(0);
    if(CK_Check(CK_1)) SSM(1);
    if(CK_Check(CK_2)) SSM(2);
    if(CK_Check(CK_3)) SSM(3);
    if(CK_Check(CK_4)) SSM(4);
    if(CK_Check(CK_5)) SSM(5);
    if(CK_Check(CK_6)) SSM(6);
    if(CK_Check(CK_7)) SSM(7);
    if(CK_Check(CK_8)) SSM(8);
    if(CK_Check(CK_9)) SSM(9);
   }
   else
   #endif
   {
    if(CurGame->GameType != GMT_PLAYER)
    {
     if(CK_Check(CK_0)) MDFNI_SelectState(0);
     if(CK_Check(CK_1)) MDFNI_SelectState(1);
     if(CK_Check(CK_2)) MDFNI_SelectState(2);
     if(CK_Check(CK_3)) MDFNI_SelectState(3);
     if(CK_Check(CK_4)) MDFNI_SelectState(4);
     if(CK_Check(CK_5)) MDFNI_SelectState(5);
     if(CK_Check(CK_6)) MDFNI_SelectState(6);
     if(CK_Check(CK_7)) MDFNI_SelectState(7);
     if(CK_Check(CK_8)) MDFNI_SelectState(8);
     if(CK_Check(CK_9)) MDFNI_SelectState(9);

     if(CK_Check(CK_M0)) MDFNI_SelectMovie(0);
     if(CK_Check(CK_M1)) MDFNI_SelectMovie(1);
     if(CK_Check(CK_M2)) MDFNI_SelectMovie(2);
     if(CK_Check(CK_M3)) MDFNI_SelectMovie(3);
     if(CK_Check(CK_M4)) MDFNI_SelectMovie(4);
     if(CK_Check(CK_M5)) MDFNI_SelectMovie(5);
     if(CK_Check(CK_M6)) MDFNI_SelectMovie(6);
     if(CK_Check(CK_M7)) MDFNI_SelectMovie(7);
     if(CK_Check(CK_M8)) MDFNI_SelectMovie(8);
     if(CK_Check(CK_M9)) MDFNI_SelectMovie(9);
    }
   }
   #undef SSM
 }
}

void MDFND_UpdateInput(void)
{
 static unsigned int rapid=0;
 int mouse_x, mouse_y;

 MouseData[2] = SDL_GetMouseState(&mouse_x,&mouse_y);
 PtoV(&mouse_x, &mouse_y);
 MouseData[0] = mouse_x&0xFFFF;
 MouseData[1] = mouse_y&0xFFFF;

 SDL_GetRelativeMouseState(&mouse_x, &mouse_y);

 MouseDataRel[0] -= (int32)MouseDataRel[0]; //floor(MouseDataRel[0]);
 MouseDataRel[1] -= (int32)MouseDataRel[1]; //floor(MouseDataRel[1]);

 MouseDataRel[0] += CurGame->mouse_sensitivity * mouse_x;
 MouseDataRel[1] += CurGame->mouse_sensitivity * mouse_y;

 SDL_JoystickUpdate();

 rapid = (rapid + 1) % (autofirefreq + 1);

 // Do stuff here
 for(unsigned int x = 0; x < NumPorts; x++)
 {
  if(!PortData[x]) continue;

  if(IConfig != none && (Port1 + x) == IConfig) continue;	// Don't update state if we're configuring for this input port.

  memset(PortData[x], 0, PortDataSize[x]);
  
  // First, handle buttons
  for(unsigned int butt = 0; butt < PortButtConfig[x].size(); butt++)
  {
   if(DTestButton(PortButtConfig[x][butt], keys, MouseData))
   {
    uint8 *tptr = (uint8 *)PortData[x];
    uint32 bo = PortButtBitOffsets[x][butt];

    if(!(bo & 0x80000000) || rapid >= (autofirefreq + 1) / 2)
     tptr[(bo & 0x7FFFFFFF) / 8] |= 1 << (bo & 7);
   }
  }

  // Handle button exclusion!
  for(unsigned int butt = 0; butt < PortButtConfig[x].size(); butt++)
  {
   uint32 bo[2];
   uint8 *tptr = (uint8 *)PortData[x];

   bo[0] = PortButtBitOffsets[x][butt];
   bo[1] = PortButtExclusionBitOffsets[x][butt];

   if(bo[1] != 0xFFFFFFFF)
   {
    //printf("%08x %08x\n", bo[0], bo[1]);
    if( (tptr[(bo[0] & 0x7FFFFFFF) / 8] & (1 << (bo[0] & 7))) && (tptr[(bo[1] & 0x7FFFFFFF) / 8] & (1 << (bo[1] & 7))) )
    {
     tptr[(bo[0] & 0x7FFFFFFF) / 8] &= ~(1 << (bo[0] & 7));
     tptr[(bo[1] & 0x7FFFFFFF) / 8] &= ~(1 << (bo[1] & 7));
    }
   }
  }

  // Now, axis data...
  for(int tmi = 0; tmi < PortDevice[x]->NumInputs; tmi++)
  {
   switch(PortDevice[x]->IDII[tmi].Type)
   {
    default: break;
    case IDIT_X_AXIS_REL:
    case IDIT_Y_AXIS_REL:
                      MDFN_en32lsb(((uint8 *)PortData[x] + PortBitOffsets[x][tmi] / 8), (uint32)((PortDevice[x]->IDII[tmi].Type == IDIT_Y_AXIS_REL) ? MouseDataRel[1] : MouseDataRel[0]));
		      break;
    case IDIT_X_AXIS:
    case IDIT_Y_AXIS:
		     {
		      MDFN_en32lsb(((uint8 *)PortData[x] + PortBitOffsets[x][tmi] / 8), (uint32)((PortDevice[x]->IDII[tmi].Type == IDIT_Y_AXIS) ? MouseData[1] : MouseData[0]));
		     }
		     break;
   }
  }
 }

 // Handle Lynx and WonderSwan screen rotation...this code is ugly and wrong, FIXME
 if(CurGame->rotated != MDFN_ROTATE0)
 {
  bool WantRotate = 0;
  int numsets = 1;
  char tmp_setting_name[64];

  snprintf(tmp_setting_name, 64, "%s.rotateinput", CurGame->shortname);
  WantRotate = MDFN_GetSettingB(tmp_setting_name);

  if(!strcasecmp(CurGame->shortname, "wswan"))
   numsets = 2;

  if(WantRotate)
  for(int i = 0; i < numsets; i++)
   for(unsigned int x = 0; x < NumPorts; x++)
   {
    if(!PortData[x]) continue;
    uint32 oldbo[4] = {0, 0, 0, 0}; // right, up, left, down.
    bool meowb[4];

    uint8 *tptr = (uint8 *)PortData[x];

    // FIXME:  Factor this code out so it doesn't run every frame.
    for(unsigned int butt = 0; butt < PortButtConfig[x].size(); butt++)
    {
     uint32 bo = PortButtBitOffsets[x][butt];
     const char *sname =  PortButtSettingNames[x][butt];

     if(numsets == 2)
     {
      if(i == 0)
      {
       if(strstr(sname, ".up-x"))
        oldbo[1] = bo;
       else if(strstr(sname, ".down-x"))
        oldbo[3] = bo;
       else if(strstr(sname, ".left-x"))
        oldbo[2] = bo;
       else if(strstr(sname, ".right-x"))
        oldbo[0] = bo;
      }
      else
      {
       if(strstr(sname, ".up-y"))
        oldbo[1] = bo;
       else if(strstr(sname, ".down-y"))
        oldbo[3] = bo;
       else if(strstr(sname, ".left-y"))
        oldbo[2] = bo;
       else if(strstr(sname, ".right-y"))
        oldbo[0] = bo;
      }
     }
     else
     {
      if(strstr(sname, ".up"))
       oldbo[1] = bo;
      else if(strstr(sname, ".down"))
       oldbo[3] = bo;
      else if(strstr(sname, ".left"))
       oldbo[2] = bo;
      else if(strstr(sname, ".right"))
       oldbo[0] = bo;
     }
    }

    for(int caca = 0; caca < 4; caca++)
    {
     uint32 bo = oldbo[caca];
     meowb[caca] = (tptr[bo / 8] >> (bo & 0x7)) & 0x1;
     tptr[bo / 8] &= ~(1 << (bo & 0x7));
    }
    int maha = 0;

    if(CurGame->rotated == MDFN_ROTATE90)
     maha = 3;
    else if(CurGame->rotated == MDFN_ROTATE270)
     maha = 1;

    for(int caca = 0; caca < 4; caca++)
    {
     uint32 bo = oldbo[maha];

     tptr[bo / 8] |= meowb[caca] << (bo & 0x7);
     maha = (maha + 1) & 0x3;
    }
   }
 }


 // KLUDGE, call command handling after general port handling(with the memset() 0 fun) so that
 // the Barcode World stuff will work.
 KeyboardCommands();
}

void InitGameInput(MDFNGI *gi)
{
 autofirefreq = MDFN_GetSettingUI("autofirefreq");
 fftoggle = MDFN_GetSettingB("fftoggle");
 ckdelay = MDFN_GetSettingUI("ckdelay");

 memset(CKeysPressTime, 0xff, sizeof(CKeysPressTime));
 memset(CKeysLastState, 0, sizeof(CKeysLastState));

 SetJoyReadMode(1); // Disable joystick event handling, and allow manual state updates.

 BuildPortInfo(gi);
}


// Resync butt configs to setting strings
void ResyncGameInputSettings(void)
{
 for(unsigned int port = 0; port < NumPorts; port++)
 {
  for(unsigned int x = 0; x < PortButtSettingNames[port].size(); x++)
   MDFNI_SetSetting(PortButtSettingNames[port][x], BCsToString( PortButtConfig[port][x] ).c_str());

 }
}


static ButtConfig subcon_bc;
static int subcon_tb;
static int subcon_wc;
static int jitter_correct;

static void subcon_begin(std::vector<ButtConfig> &bc)
{
 bc.clear();

 memset(&subcon_bc, 0, sizeof(subcon_bc));
 subcon_tb = -1;
 subcon_wc = 0;
 jitter_correct = 0;
}

/* Configures an individual virtual button. */
static int subcon(const char *text, std::vector<ButtConfig> &bc, int commandkey)
{
 char buf[256];

 while(1)
 {
  sprintf(buf,"%s (%d)",text,subcon_wc+1);
  MDFNI_DispMessage(buf);

  if(subcon_tb != subcon_wc)
  {
   DTryButtonBegin(&subcon_bc, commandkey);
   subcon_tb = subcon_wc;
   jitter_correct = 0;
  }

  if(!jitter_correct)
   if(!DTryButton())
    return(0);

  DTryButtonEnd(&subcon_bc);

  if(subcon_bc.ButtType != BUTTC_JOYSTICK)	// Only do jitter correction on joysticks.
   jitter_correct = 10;

  if(jitter_correct < 10)
  {
   ButtConfig tmpbc;

   tmpbc = subcon_bc;

   if((!commandkey && DTestButton(tmpbc, keys, MouseData)) || (commandkey && DTestButtonCombo(tmpbc, keys)))
   {
    jitter_correct++;
   }
   else
   {
    //puts("Jitter failure!");
    subcon_tb = subcon_wc - 1;	// Redo this physical button.
   }
   return(0);
  }

  if(subcon_wc && !memcmp(&subcon_bc, &bc[subcon_wc - 1], sizeof(ButtConfig)))
   break;

  bc.push_back(subcon_bc);
  subcon_wc++;
 }

 //puts("DONE");
 return(1);
}

static int cd_x;
static int cd_lx = -1;
static void ConfigDeviceBegin(void)
{
 cd_x = 0;
 cd_lx = -1;
}

int ConfigDevice(int arg)
{
 char buf[256];

 //for(int i = 0; i < PortButtons[arg].size(); i++)
 // printf("%d\n", PortButtConfigPrettyPrio[arg][i]);
 //exit(1);

 for(;cd_x < (int)PortButtons[arg].size(); cd_x++)
 {
  int snooty = 0;

  for(unsigned int i = 0; i < PortButtons[arg].size(); i++)
   if(PortButtConfigPrettyPrio[arg][i] == cd_x)
    snooty = i;

  // For Lynx, GB, GBA, NGP, WonderSwan(especially wonderswan!)
  //if(!strcasecmp(PortDevice[arg]->ShortName, "builtin")) // && !arg)
  if(NumPorts == 1 && PortPossibleDevices[CurGame->shortname][0].size() == 1)
   sprintf(buf, "%s", PortButtons[arg][snooty]);
  else
   sprintf(buf, "%s %d: %s", PortDevice[arg]->FullName, arg + 1, PortButtons[arg][snooty]);

  if(cd_x != cd_lx)
  {
   cd_lx = cd_x;
   subcon_begin(PortButtConfig[arg][snooty]);
  }
  if(!subcon(buf, PortButtConfig[arg][snooty], 0))
   return(0);
 }

 MDFNI_DispMessage(_("Configuration finished."));

 return(1);
}

#include "input-default-buttons.h"

static std::vector<void *> PendingGarbage;

static void MakeSettingsForDevice(std::vector <MDFNSetting> &settings, const MDFNGI *system, const int w, const InputDeviceInfoStruct *info)
{
 const ButtConfig *def_bc = NULL;
 char setting_def_search[256];

 PortPossibleDevices[system->shortname][w].push_back(info);

 trio_snprintf(setting_def_search, 256, "%s.input.%s.%s", system->shortname, system->InputInfo->Types[w].ShortName, info->ShortName);
 CleanSettingName(setting_def_search);

 for(unsigned int d = 0; d < sizeof(defset) / sizeof(DefaultSettingsMeow); d++)
 {
  if(!strcasecmp(setting_def_search, defset[d].base_name))
  {
   def_bc = defset[d].bc;
   break;
  }
 }

 int butti = 0;

 for(int x = 0; x < info->NumInputs; x++)
 {
  if(info->IDII[x].Type != IDIT_BUTTON && info->IDII[x].Type != IDIT_BUTTON_CAN_RAPID && info->IDII[x].Type != IDIT_BUTTON_BYTE)
   continue;

  MDFNSetting tmp_setting;

  const char *default_value = "";

  char setting_name[256];

  trio_snprintf(setting_name, 256, "%s.input.%s.%s.%s", system->shortname, system->InputInfo->Types[w].ShortName, info->ShortName, info->IDII[x].SettingName);
  CleanSettingName(setting_name);

  // Don't duplicate settings due to certain port expander configurations on emulated systems,
  // such as the Famicom 4-player adapter.
  bool AlreadySetting = 0;
  for(unsigned int s = 0; s < settings.size(); s++)
  {
   if(!strcasecmp(settings[s].name, setting_name))
   {
    //printf("Oops: %s\n", setting_name);
    AlreadySetting = TRUE;
    break;
   }
  }

  if(AlreadySetting)
   continue;

  if(def_bc)
   PendingGarbage.push_back((void *)(default_value = strdup(BCToString(def_bc[butti]).c_str()) ));

  //printf("Maketset: %s %s\n", setting_name, default_value);

  memset(&tmp_setting, 0, sizeof(tmp_setting));

  PendingGarbage.push_back((void *)(tmp_setting.name = strdup(setting_name) ));
  PendingGarbage.push_back((void *)(tmp_setting.description = trio_aprintf("%s, %s, %s: %s", system->shortname, system->InputInfo->Types[w].FullName, info->FullName, info->IDII[x].Name) ));
  tmp_setting.type = MDFNST_STRING;
  tmp_setting.default_value = default_value;
  
  settings.push_back(tmp_setting);

  // Now make a rapid butt-on-stick-on-watermelon
  if(info->IDII[x].Type == IDIT_BUTTON_CAN_RAPID)
  {
   memset(&tmp_setting, 0, sizeof(tmp_setting));
   trio_snprintf(setting_name, 256, "%s.input.%s.%s.rapid_%s", system->shortname, system->InputInfo->Types[w].ShortName, info->ShortName, info->IDII[x].SettingName);
   CleanSettingName(setting_name);

   PendingGarbage.push_back((void *)( tmp_setting.name = strdup(setting_name) ));
   PendingGarbage.push_back((void *)( tmp_setting.description = trio_aprintf("%s, %s, %s: Rapid %s", system->shortname, system->InputInfo->Types[w].FullName, info->FullName, info->IDII[x].Name) ));
   tmp_setting.type = MDFNST_STRING;
   tmp_setting.default_value = "";
   settings.push_back(tmp_setting);
  }
  butti++;
 }
}


static void MakeSettingsForPort(std::vector <MDFNSetting> &settings, const MDFNGI *system, const int w, const InputPortInfoStruct *info)
{
 for(int device = 0; device < info->NumTypes; device++)
 {
  const InputDeviceInfoStruct *dinfo = &info->DeviceInfo[device];

  if(dinfo->PortExpanderDeviceInfo)
  {
   const InputPortInfoStruct *sub_ports = (const InputPortInfoStruct *)dinfo->PortExpanderDeviceInfo;
   const int sub_port_count = dinfo->NumInputs;

   PortPossibleDevices[system->shortname][w].push_back(&info->DeviceInfo[device]);

   for(int sub_port = 0; sub_port < sub_port_count; sub_port++)
   {
    MakeSettingsForPort(settings, system, w + sub_port, &sub_ports[sub_port]);
   }
   //for(int sub_device = 0; sub_device < dinfo->PortExp
  }
  else
  {
   //printf("%d %d\n", w, info->pid_offset);
   MakeSettingsForDevice(settings, system, w + info->pid_offset, &info->DeviceInfo[device]);
  }
 }
}

// Called on emulator startup
void MakeInputSettings(std::vector <MDFNSetting> &settings)
{
 // First, build system settings
 for(unsigned int x = 0; x < MDFNSystemCount; x++)
 {
  if(MDFNSystems[x]->InputInfo)
  {
   PortPossibleDevices[MDFNSystems[x]->shortname] = new SnugglyWuggly[16];

   for(int port = 0; port < MDFNSystems[x]->InputInfo->InputPorts; port++)
    MakeSettingsForPort(settings, MDFNSystems[x], port, &MDFNSystems[x]->InputInfo->Types[port]);
  }
 }

 // Now build command key settings
 for(int x = 0; x < _CK_COUNT; x++)
 {
  char setting_name[256];

  MDFNSetting tmp_setting;

  trio_snprintf(setting_name, 256, "command.%s", CKeys[x].text);

  memset(&tmp_setting, 0, sizeof(tmp_setting));

  PendingGarbage.push_back((void *)( tmp_setting.name = strdup(setting_name) ));
  tmp_setting.description = CKeys[x].description;
  tmp_setting.type = MDFNST_STRING;

  PendingGarbage.push_back((void *)( tmp_setting.default_value = strdup(BCsToString(CKeys[x].bc, 2).c_str()) ));
  settings.push_back(tmp_setting);

  //printf("%s %s\n", setting_name, default_value);
 }
}

void KillGameInput(void)
{
 ResyncGameInputSettings();

 KillPortInfo();
}

void KillInputSettings(void)
{
 for(unsigned int x = 0; x < PendingGarbage.size(); x++)
  free(PendingGarbage[x]);

 PendingGarbage.clear();
}

bool InitCommandInput(void)
{
 // Load the command key mappings from settings
 for(int x = 0; x < _CK_COUNT; x++)
 {
  char setting_name[256];

  trio_snprintf(setting_name, 256, "command.%s", CKeys[x].text);

  StringToBC(MDFN_GetSettingS(setting_name).c_str(), CKeysBC[x]);
 }
 return(1);
}

void KillCommandInput(void)
{
 // Save the command key mappings to settings
 for(int x = 0; x < _CK_COUNT; x++)
 {
  char setting_name[256];

  trio_snprintf(setting_name, 256, "command.%s", CKeys[x].text);

  MDFNI_SetSetting(setting_name, BCsToString(CKeysBC[x]).c_str());
 }
}
