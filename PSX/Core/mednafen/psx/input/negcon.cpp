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

#include "../psx.h"
#include "../frontio.h"
#include "negcon.h"

namespace MDFN_IEN_PSX
{

class InputDevice_neGcon : public InputDevice
{
 public:

 InputDevice_neGcon(void);
 virtual ~InputDevice_neGcon();

 virtual void Power(void);
 virtual void UpdateInput(const void *data);

 //
 //
 //
 virtual void SetDTR(bool new_dtr);
 virtual bool GetDSR(void);
 virtual bool Clock(bool TxD, int32 &dsr_pulse_delay);

 private:

 bool dtr;

 uint8 buttons[2];
 uint8 twist;
 uint8 anabuttons[3];

 int32 command_phase;
 uint32 bitpos;
 uint8 receive_buffer;

 uint8 command;

 uint8 transmit_buffer[8];
 uint32 transmit_pos;
 uint32 transmit_count;
};

InputDevice_neGcon::InputDevice_neGcon(void)
{
 Power();
}

InputDevice_neGcon::~InputDevice_neGcon()
{

}

void InputDevice_neGcon::Power(void)
{
 dtr = 0;

 buttons[0] = buttons[1] = 0;
 twist = 0;
 anabuttons[0] = 0;
 anabuttons[1] = 0;
 anabuttons[2] = 0;

 command_phase = 0;

 bitpos = 0;

 receive_buffer = 0;

 command = 0;

 memset(transmit_buffer, 0, sizeof(transmit_buffer));

 transmit_pos = 0;
 transmit_count = 0;
}

void InputDevice_neGcon::UpdateInput(const void *data)
{
 uint8 *d8 = (uint8 *)data;

 buttons[0] = d8[0];
 buttons[1] = d8[1];

 twist = ((32768 + MDFN_de32lsb((const uint8 *)data + 4) - (((int32)MDFN_de32lsb((const uint8 *)data + 8) * 32768 + 16383) / 32767)) * 255 + 32767) / 65535;

 anabuttons[0] = (MDFN_de32lsb((const uint8 *)data + 12) * 255 + 16383) / 32767; 
 anabuttons[1] = (MDFN_de32lsb((const uint8 *)data + 16) * 255 + 16383) / 32767;
 anabuttons[2] = (MDFN_de32lsb((const uint8 *)data + 20) * 255 + 16383) / 32767;

 //printf("%02x %02x %02x %02x\n", twist, anabuttons[0], anabuttons[1], anabuttons[2]);
}


void InputDevice_neGcon::SetDTR(bool new_dtr)
{
 if(!dtr && new_dtr)
 {
  command_phase = 0;
  bitpos = 0;
  transmit_pos = 0;
  transmit_count = 0;
 }
 else if(dtr && !new_dtr)
 {
  //if(bitpos || transmit_count)
  // printf("[PAD] Abort communication!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
 }

 dtr = new_dtr;
}

bool InputDevice_neGcon::GetDSR(void)
{
 if(!dtr)
  return(0);

 if(!bitpos && transmit_count)
  return(1);

 return(0);
}

bool InputDevice_neGcon::Clock(bool TxD, int32 &dsr_pulse_delay)
{
 bool ret = 1;

 dsr_pulse_delay = 0;

 if(!dtr)
  return(1);

 if(transmit_count)
  ret = (transmit_buffer[transmit_pos] >> bitpos) & 1;

 receive_buffer &= ~(1 << bitpos);
 receive_buffer |= TxD << bitpos;
 bitpos = (bitpos + 1) & 0x7;

 if(!bitpos)
 {
  //printf("[PAD] Receive: %02x -- command_phase=%d\n", receive_buffer, command_phase);

  if(transmit_count)
  {
   transmit_pos++;
   transmit_count--;
  }


  switch(command_phase)
  {
   case 0:
 	  if(receive_buffer != 0x01)
	    command_phase = -1;
	  else
	  {
	   transmit_buffer[0] = 0x23;
	   transmit_pos = 0;
	   transmit_count = 1;
	   command_phase++;
	   dsr_pulse_delay = 256;
	  }
	  break;

   case 1:
	command = receive_buffer;
	command_phase++;

	transmit_buffer[0] = 0x5A;

	//if(command != 0x42)
	// fprintf(stderr, "Gamepad unhandled command: 0x%02x\n", command);

	if(command == 0x42)
	{
	 transmit_buffer[1] = 0xFF ^ buttons[0];
	 transmit_buffer[2] = 0xFF ^ buttons[1];
	 transmit_buffer[3] = twist;			// Twist, 0x00 through 0xFF, 0x80 center.
	 transmit_buffer[4] = anabuttons[0];		// Analog button I, 0x00 through 0xFF, 0x00 = no pressing, 0xFF = max.
	 transmit_buffer[5] = anabuttons[1];		// Analog button II, ""
	 transmit_buffer[6] = anabuttons[2];		// Left shoulder analog button, ""
         transmit_pos = 0;
         transmit_count = 7;
	 dsr_pulse_delay = 256;
	}
	else
	{
	 command_phase = -1;
	 transmit_buffer[1] = 0;
	 transmit_buffer[2] = 0;
         transmit_pos = 0;
         transmit_count = 0;
	}
	break;

   case 2:
	if(transmit_count > 0)
	 dsr_pulse_delay = 128;
	break;
  }
 }

 return(ret);
}

InputDevice *Device_neGcon_Create(void)
{
 return new InputDevice_neGcon();
}


InputDeviceInputInfoStruct Device_neGcon_IDII[21] =
{
 { NULL, "empty", -1, IDIT_BUTTON, NULL },
 { NULL, "empty", -1, IDIT_BUTTON, NULL },
 { NULL, "empty", -1, IDIT_BUTTON, NULL },
 { "start", "START", 4, IDIT_BUTTON, NULL },
 { "up", "D-Pad UP ↑", 0, IDIT_BUTTON, "down" },
 { "right", "D-Pad RIGHT →", 3, IDIT_BUTTON, "left" },
 { "down", "D-Pad DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "left", "D-Pad LEFT ←", 2, IDIT_BUTTON, "right" },

 { NULL, "empty", -1, IDIT_BUTTON, NULL },
 { NULL, "empty", -1, IDIT_BUTTON, NULL },
 { NULL, "empty", -1, IDIT_BUTTON, NULL },
 { "r", "Right Shoulder", 12, IDIT_BUTTON },

 { "b", "B", 9, IDIT_BUTTON, NULL },
 { "a", "A", 10, IDIT_BUTTON, NULL },
 { NULL, "empty", -1, IDIT_BUTTON, NULL },
 { NULL, "empty", -1, IDIT_BUTTON, NULL },

 { "twist_cwise",  "Twist ↓|↑ (Analog, Turn Right)", 6, IDIT_BUTTON_ANALOG },
 { "twist_ccwise", "Twist ↑|↓ (Analog, Turn Left)", 5, IDIT_BUTTON_ANALOG },
 { "i", "I (Analog)", 8, IDIT_BUTTON_ANALOG },
 { "ii", "II (Analog)", 7, IDIT_BUTTON_ANALOG },

 { "l", "Left Shoulder (Analog)", 11, IDIT_BUTTON_ANALOG },
};

}
