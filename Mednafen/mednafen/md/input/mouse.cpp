/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003  Charles MacDonald
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

#include "../shared.h"
#include "mouse.h"

enum
{
 MASK_TH = 0x40,
 MASK_TR = 0x20,
 MASK_TL = 0x10,
 MASK_DATA = 0x0F
};

class MegaMouse : public MD_Input_Device
{
        public:
	MegaMouse();
	virtual ~MegaMouse();
        virtual void Write(uint8 data);
        virtual uint8 Read();
        virtual void Update(const void *data);

	private:
	int32 mouse_x;
	int32 mouse_y;
	uint8 buttons;

	uint8 data_buffer[0xA];
	uint8 counter;
	bool last_th;
	bool last_tr;

	bool tl_ret;
	bool happy_mode;
	int32 busy_meow;
};

const InputDeviceInputInfoStruct MegaMouseIDII[6] =
{
 { "x_axis", "X Axis", -1, IDIT_X_AXIS_REL },
 { "y_axis", "Y Axis", -1, IDIT_Y_AXIS_REL },
 { "left", "Left Button", 0, IDIT_BUTTON, NULL },
 { "right", "Right Button", 1, IDIT_BUTTON, NULL },
 { "middle", "Middle Button", 2, IDIT_BUTTON, NULL },
 { "start", "Start Button", 3, IDIT_BUTTON, NULL },
};

MegaMouse::MegaMouse()
{
        mouse_x = 0;
        mouse_y = 0;
        buttons = 0;

        counter = 0;
        last_th = 0;
        last_tr = 0;

        tl_ret = 0;
        happy_mode = 0;
        busy_meow = 0;
}

MegaMouse::~MegaMouse()
{

}

void MegaMouse::Write(uint8 data)
{
 //printf("Write: %02x\n", data);
 #if 0
 if((data & 0x60) == 0x60 && counter == 9)
 {
  happy_mode = FALSE;
  tl_ret = FALSE;
  last_th = 0;
  last_tr = 0;
  return;
 }
 #endif

 if(((data & MASK_TH) && !(last_th)) || (data & 0x60) == 0x60)
 {
  if((data & 0x60) == 0x60)
  {
   happy_mode = TRUE;
   tl_ret = TRUE;
  }
  int32 rel_x = mouse_x;
  int32 rel_y = mouse_y;
  bool x_neg = 0;
  bool y_neg = 0;

  if(rel_x < -255)
   rel_x = -255;

  if(rel_x > 255)
   rel_x = 255;

  if(rel_y < -255)
   rel_y = -255;

  if(rel_y > 255)
   rel_y = 255;

  mouse_x -= rel_x;
  mouse_y -= rel_y;

  rel_y = -rel_y;

  if(rel_x < 0)
  {
   x_neg = TRUE;
  }

  if(rel_y < 0)
  {
   y_neg = TRUE;
  }

  //printf("%02x, %d %d\n", buttons, rel_x, rel_y);
  counter = 0;
  data_buffer[0] = 0x0;
  data_buffer[1] = 0xB;
  data_buffer[2] = 0xF;
  data_buffer[3] = 0xF;
  data_buffer[4] = (x_neg ? 0x1 : 0x0) | (y_neg ? 0x2 : 0x0); // Axis sign and overflow
  data_buffer[5] = buttons; // Button state
  data_buffer[6] = (rel_x >> 4) & 0xF; // X axis MSN
  data_buffer[7] = (rel_x >> 0) & 0xF; // X axis LSN
  data_buffer[8] = (rel_y >> 4) & 0xF; // Y axis MSN
  data_buffer[9] = (rel_y >> 0) & 0xF; // Y axis LSN
 }
 else if(!(data & MASK_TH) && last_th)
 {
  counter++;
  if(counter > 9)
   counter = 9;
 }

 // Mouse vs Mega
 if(!(data & MASK_TH))
 {
  if((data & MASK_TR) && !last_tr)
  {
   tl_ret = TRUE;
   counter++;

   if(counter > 9)
    counter = 9;
   if(counter == 9)
    busy_meow = 10;
  }
  else if(!(data & MASK_TR) && (last_tr))
  {
   tl_ret = FALSE;
   counter++;
   if(counter > 9)
    counter = 9;
  }
 }

 last_th = data & MASK_TH;
 last_tr = data & MASK_TR;
}

uint8 MegaMouse::Read(void)
{
 uint8 ret = 0;

 ret |= data_buffer[counter];

 if(tl_ret)
  ret |= MASK_TL;

 if(busy_meow > 0)
 {
  busy_meow--;
  if(!busy_meow)
   tl_ret = 0;
 }
 //printf("Read: %02x, %d\n", ret, counter);
 return(ret);
}

void MegaMouse::Update(const void *data)
{
 mouse_x += (int32)MDFN_de32lsb((uint8 *)data + 0);
 mouse_y += (int32)MDFN_de32lsb((uint8 *)data + 4);
 buttons = ((uint8 *)data)[8];
 MDFNGameInfo->mouse_sensitivity = 1.0; //MDFN_GetSettingF("pcfx.mouse_sensitivity");
}

MD_Input_Device *MDInput_MakeMegaMouse(void)
{
 MD_Input_Device *ret = new MegaMouse();


 return(ret);
}
