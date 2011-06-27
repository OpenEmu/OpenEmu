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

#include "../pce.h"
#include "../input.h"
#include "gamepad.h"

namespace MDFN_IEN_PCE
{

class PCE_Input_Gamepad : public PCE_Input_Device
{
 public:
 PCE_Input_Gamepad(int which);
 virtual void Power(int32 timestamp);
 virtual void Write(int32 timestamp, bool old_SEL, bool new_SEL, bool old_CLR, bool new_CLR);
 virtual uint8 Read(int32 timestamp);
 virtual void Update(const void *data);
 virtual int StateAction(StateMem *sm, int load, int data_only, const char *section_name);


 private:
 bool SEL, CLR;
 uint16 buttons;
 bool AVPad6Which;
 bool AVPad6Enabled;
 int which_this;
};

PCE_Input_Gamepad::PCE_Input_Gamepad(int which)
{
 which_this = which;
 Power(0); // FIXME?
}

void PCE_Input_Gamepad::Power(int32 timestamp)
{
 SEL = 0;
 CLR = 0;
 buttons = 0;
 AVPad6Which = 0;
 AVPad6Enabled = 0;
}

void PCE_Input_Gamepad::Update(const void *data)
{
 uint16 new_data = MDFN_de16lsb((uint8 *)data);

 if((new_data & 0x1000) && !(buttons & 0x1000))
 {
  AVPad6Enabled = !AVPad6Enabled;
  MDFN_DispMessage("%d-button mode selected for pad %d", AVPad6Enabled ? 6 : 2, which_this + 1);
 }

 buttons = new_data;
}

uint8 PCE_Input_Gamepad::Read(int32 timestamp)
{
 uint8 ret = 0xF;

 if(AVPad6Which && AVPad6Enabled)
 {
  if(SEL)
   ret ^= 0xF;
  else
   ret ^= (buttons >> 8) & 0x0F;
 }
 else
 {
  if(SEL)
   ret ^= (buttons >> 4) & 0x0F;
  else
   ret ^= buttons & 0x0F;
 }

 //if(CLR)
 // ret = 0;

 return(ret);
}

void PCE_Input_Gamepad::Write(int32 timestamp, bool old_SEL, bool new_SEL, bool old_CLR, bool new_CLR)
{
 SEL = new_SEL;
 CLR = new_CLR;

 //if(old_SEL && new_SEL && old_CLR && !new_CLR)
 if(!old_SEL && new_SEL)
  AVPad6Which = !AVPad6Which;
}

int PCE_Input_Gamepad::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(SEL),
  SFVAR(CLR),
  SFVAR(buttons),
  SFVAR(AVPad6Which),
  SFVAR(AVPad6Enabled),
  SFEND
 };
 int ret =  MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name);

 return(ret);
}

// GamepadIDII and GamepadIDII_DSR must be EXACTLY the same except for the RUN+SELECT exclusion in the latter.
const InputDeviceInputInfoStruct PCE_GamepadIDII[0xD] =
{
 { "i", "I", 12, IDIT_BUTTON_CAN_RAPID, NULL },
 { "ii", "II", 11, IDIT_BUTTON_CAN_RAPID, NULL },
 { "select", "SELECT", 4, IDIT_BUTTON, NULL },
 { "run", "RUN", 5, IDIT_BUTTON, NULL },
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "iii", "III", 10, IDIT_BUTTON, NULL },
 { "iv", "IV", 7, IDIT_BUTTON, NULL },
 { "v", "V", 8, IDIT_BUTTON, NULL },
 { "vi", "VI", 9, IDIT_BUTTON, NULL },
 { "mode_select", "2/6 Mode Select", 6, IDIT_BUTTON, NULL },
};
const InputDeviceInputInfoStruct PCE_GamepadIDII_DSR[0xD] =
{
 { "i", "I", 12, IDIT_BUTTON_CAN_RAPID, NULL },
 { "ii", "II", 11, IDIT_BUTTON_CAN_RAPID, NULL },
 { "select", "SELECT", 4, IDIT_BUTTON, "run" },
 { "run", "RUN", 5, IDIT_BUTTON, "select" },
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "iii", "III", 10, IDIT_BUTTON, NULL },
 { "iv", "IV", 7, IDIT_BUTTON, NULL },
 { "v", "V", 8, IDIT_BUTTON, NULL },
 { "vi", "VI", 9, IDIT_BUTTON, NULL },
 { "mode_select", "2/6 Mode Select", 6, IDIT_BUTTON, NULL },
};

PCE_Input_Device *PCEINPUT_MakeGamepad(int which)
{
 return(new PCE_Input_Gamepad(which));
}

};
