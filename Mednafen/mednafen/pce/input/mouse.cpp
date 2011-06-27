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
#include "mouse.h"

namespace MDFN_IEN_PCE
{

class PCE_Input_Mouse : public PCE_Input_Device
{
 public:
 PCE_Input_Mouse();
 virtual void Power(int32 timestamp);

 virtual void AdjustTS(int32 delta);
 virtual void Write(int32 timestamp, bool old_SEL, bool new_SEL, bool old_CLR, bool new_CLR);
 virtual uint8 Read(int32 timestamp);
 virtual void Update(const void *data);
 virtual int StateAction(StateMem *sm, int load, int data_only, const char *section_name);

 private:
 bool SEL, CLR;
 int64 mouse_last_meow;

 int32 mouse_x, mouse_y;
 uint8 pce_mouse_button;
 uint8 mouse_index;


 uint16 mouse_shifter;
};

void PCE_Input_Mouse::Power(int32 timestamp)
{
 SEL = CLR = 0;
 mouse_last_meow = 0;
 mouse_x = mouse_y = 0;
 pce_mouse_button = 0;
 mouse_index = 0;
 mouse_shifter = 0;
}

PCE_Input_Mouse::PCE_Input_Mouse()
{
 Power(0);
}

void PCE_Input_Mouse::Update(const void *data)
{
 //puts("Frame");
 uint8 *data_ptr = (uint8 *)data;

 mouse_x += (int32)MDFN_de32lsb(data_ptr + 0);
 mouse_y += (int32)MDFN_de32lsb(data_ptr + 4);
 pce_mouse_button = *(uint8 *)(data_ptr + 8);
}

void PCE_Input_Mouse::AdjustTS(int32 delta)
{
 //printf("Adjust: %d\n", delta);
 mouse_last_meow += delta;
}

void PCE_Input_Mouse::Write(int32 timestamp, bool old_SEL, bool new_SEL, bool old_CLR, bool new_CLR)
{
 //printf("Write: %d old_SEL=%d, new_SEL=%d, old_CLR=%d, new_CLR=%d\n", timestamp, old_SEL, new_SEL, old_CLR, new_CLR);
 if(!old_CLR && new_CLR)
 {
  //printf("%lld\n", (int64)timestamp - mouse_last_meow);
  if(((int64)timestamp - mouse_last_meow) > 10000 * 3)
  {
   mouse_last_meow = timestamp;

   int32 rel_x = (int32)((0-mouse_x));
   int32 rel_y = (int32)((0-mouse_y));

   if(rel_x < -127) rel_x = -127;
   if(rel_x > 127) rel_x = 127;
   if(rel_y < -127) rel_y = -127;
   if(rel_y > 127) rel_y = 127;

   mouse_shifter = ((rel_x & 0xF0) >> 4) | ((rel_x & 0x0F) << 4);
   mouse_shifter |= (((rel_y & 0xF0) >> 4) | ((rel_y & 0x0F) << 4)) << 8;

   mouse_x += (int32)(rel_x);
   mouse_y += (int32)(rel_y);

   //printf("Latch: %d %d, %04x\n", rel_x, rel_y, mouse_shifter);
  }
  else
  {
   //puts("Shift");
   mouse_shifter >>= 4;
  }
 }

 SEL = new_SEL;
 CLR = new_CLR;
}

uint8 PCE_Input_Mouse::Read(int32 timestamp)
{
 uint8 ret = 0xF;

 //printf("Read: %d\n", timestamp);

 if(SEL)
 {
  ret = (mouse_shifter & 0xF);
  //printf("Read: %02x\n", ret);
 }
 else
 {
  ret ^= pce_mouse_button & 0xF;
 }

 return(ret);
}

int PCE_Input_Mouse::StateAction(StateMem *sm, int load, int data_only, const char *section_name)
{
 SFORMAT StateRegs[] =
 {
  SFVAR(SEL),
  SFVAR(CLR),
  SFVAR(mouse_last_meow),
  SFVAR(mouse_x),
  SFVAR(mouse_y),
  SFVAR(pce_mouse_button),
  SFVAR(mouse_index),
  SFVAR(mouse_shifter),

  SFEND
 };
 int ret =  MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name);
 
 return(ret);
}

const InputDeviceInputInfoStruct PCE_MouseIDII[0x6] =
{
 { "x_axis", "X Axis", -1, IDIT_X_AXIS_REL },
 { "y_axis", "Y Axis", -1, IDIT_Y_AXIS_REL },
 { "right", "Right Button", 3, IDIT_BUTTON, NULL },
 { "left", "Left Button", 2, IDIT_BUTTON, NULL },
 { "select", "SELECT", 0, IDIT_BUTTON, NULL },
 { "run", "RUN", 1, IDIT_BUTTON, NULL },
};

PCE_Input_Device *PCEINPUT_MakeMouse(void)
{
 return(new PCE_Input_Mouse());
}

};
