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

#include "../pcfx.h"
#include "../input.h"
#include "mouse.h"

class PCFX_Input_Mouse : public PCFX_Input_Device
{
 public:
 PCFX_Input_Mouse(int which)
 {
  x = 0;
  y = 0;
  button = 0;
 }

 virtual ~PCFX_Input_Mouse()
 {

 }

 virtual uint32 ReadTransferTime(void)
 {
  return(1536);
 }

 virtual uint32 WriteTransferTime(void)
 {
  return(1536);
 }

 virtual uint32 Read(void)
 {
  uint32 moo = FX_SIG_MOUSE << 28;
  int32 rel_x = (int32)(x);
  int32 rel_y = (int32)(y);

  if(rel_x < -127) rel_x = -127;
  if(rel_x > 127) rel_x = 127;
  if(rel_y < -127) rel_y = -127;
  if(rel_y > 127) rel_y = 127;

  moo |= ((rel_x & 0xFF) << 8) | ((rel_y & 0xFF) << 0);

  x += (int32)(0 - rel_x);
  y += (int32)(0 - rel_y);

  moo |= button << 16;

  return(moo);
 }

 virtual void Write(uint32 data)
 {

 }


 virtual void Power(void)
 {
  button = 0;
  x = 0;
  y = 0;
 }

 virtual void Frame(const void *data)
 {
  x += (int32)MDFN_de32lsb((uint8 *)data + 0);
  y += (int32)MDFN_de32lsb((uint8 *)data + 4);
  button = *(uint8 *)((uint8 *)data + 8);
 }

 virtual int StateAction(StateMem *sm, int load, int data_only, const char *section_name)
 {
  SFORMAT StateRegs[] =
  {
   SFVAR(x),
   SFVAR(y),
   SFVAR(button),
   SFEND
  };
  int ret =  MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name);

  return(ret);
 }

 private:


 int32 x, y;
 uint8 button;
};

PCFX_Input_Device *PCFXINPUT_MakeMouse(int which)
{
 return(new PCFX_Input_Mouse(which));
}

const InputDeviceInputInfoStruct PCFX_MouseIDII[4] =
{
 { "x_axis", "X Axis", -1, IDIT_X_AXIS_REL },
 { "y_axis", "Y Axis", -1, IDIT_Y_AXIS_REL },
 { "left", "Left Button", 0, IDIT_BUTTON, NULL },
 { "right", "Right Button", 1, IDIT_BUTTON, NULL },
};

