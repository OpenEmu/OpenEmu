#include "../psx.h"
#include "../frontio.h"
#include "dualanalog.h"

namespace MDFN_IEN_PSX
{

class InputDevice_DualAnalog : public InputDevice
{
 public:

 InputDevice_DualAnalog(bool joystick_mode_);
 virtual ~InputDevice_DualAnalog();

 virtual void Power(void);
 virtual void UpdateInput(const void *data);

 //
 //
 //
 virtual void SetDTR(bool new_dtr);
 virtual bool GetDSR(void);
 virtual bool Clock(bool TxD, int32 &dsr_pulse_delay);

 private:

 bool joystick_mode;
 bool dtr;

 uint8 buttons[2];
 uint8 axes[2][2];

 int32 command_phase;
 uint32 bitpos;
 uint8 receive_buffer;

 uint8 command;

 uint8 transmit_buffer[8];
 uint32 transmit_pos;
 uint32 transmit_count;
};

InputDevice_DualAnalog::InputDevice_DualAnalog(bool joystick_mode_) : joystick_mode(joystick_mode_)
{
 Power();
}

InputDevice_DualAnalog::~InputDevice_DualAnalog()
{

}

void InputDevice_DualAnalog::Power(void)
{
 dtr = 0;

 buttons[0] = buttons[1] = 0;

 command_phase = 0;

 bitpos = 0;

 receive_buffer = 0;

 command = 0;

 memset(transmit_buffer, 0, sizeof(transmit_buffer));

 transmit_pos = 0;
 transmit_count = 0;
}

void InputDevice_DualAnalog::UpdateInput(const void *data)
{
 uint8 *d8 = (uint8 *)data;

 buttons[0] = d8[0];
 buttons[1] = d8[1];

 for(int stick = 0; stick < 2; stick++)
 {
  for(int axis = 0; axis < 2; axis++)
  {
   int32 tmp;

   tmp = 32768 + MDFN_de32lsb((const uint8 *)data + stick * 16 + axis * 8 + 4) - ((int32)MDFN_de32lsb((const uint8 *)data + stick * 16 + axis * 8 + 8) * 32768 / 32767);
   tmp >>= 8;

   axes[stick][axis] = tmp;
  }
 }

 //printf("%d %d %d %d\n", axes[0][0], axes[0][1], axes[1][0], axes[1][1]);

}


void InputDevice_DualAnalog::SetDTR(bool new_dtr)
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

bool InputDevice_DualAnalog::GetDSR(void)
{
 if(!dtr)
  return(0);

 if(!bitpos && transmit_count)
  return(1);

 return(0);
}

bool InputDevice_DualAnalog::Clock(bool TxD, int32 &dsr_pulse_delay)
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
	   transmit_buffer[0] = joystick_mode ? 0x53 : 0x73;
	   transmit_pos = 0;
	   transmit_count = 1;
	   command_phase++;
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
	 transmit_buffer[3] = axes[0][0];
	 transmit_buffer[4] = axes[0][1];
	 transmit_buffer[5] = axes[1][0];
	 transmit_buffer[6] = axes[1][1];
         transmit_pos = 0;
         transmit_count = 7;
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
	//if(receive_buffer)
	// printf("%d: %02x\n", 7 - transmit_count, receive_buffer);
	break;
  }
 }

 if(!bitpos && transmit_count)
  dsr_pulse_delay = 0x40; //0x100;

 return(ret);
}

InputDevice *Device_DualAnalog_Create(bool joystick_mode)
{
 return new InputDevice_DualAnalog(joystick_mode);
}


InputDeviceInputInfoStruct Device_DualAnalog_IDII[24] =
{
 { "select", "SELECT", 4, IDIT_BUTTON, NULL },
 { "l3", "Left Stick, Button(L3)", 18, IDIT_BUTTON, NULL },
 { "r3", "Right stick, Button(R3)", 23, IDIT_BUTTON, NULL },
 { "start", "START", 5, IDIT_BUTTON, NULL },
 { "up", "D-Pad UP ↑", 0, IDIT_BUTTON, "down" },
 { "right", "D-Pad RIGHT →", 3, IDIT_BUTTON, "left" },
 { "down", "D-Pad DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "left", "D-Pad LEFT ←", 2, IDIT_BUTTON, "right" },

 { "l2", "L2 (rear left shoulder)", 11, IDIT_BUTTON, NULL },
 { "r2", "R2 (rear right shoulder)", 13, IDIT_BUTTON, NULL },
 { "l1", "L1 (front left shoulder)", 10, IDIT_BUTTON, NULL },
 { "r1", "R1 (front right shoulder)", 12, IDIT_BUTTON, NULL },

 { "triangle", "△ (upper)", 6, IDIT_BUTTON_CAN_RAPID, NULL },
 { "circle", "○ (right)", 9, IDIT_BUTTON_CAN_RAPID, NULL },
 { "cross", "x (lower)", 7, IDIT_BUTTON_CAN_RAPID, NULL },
 { "square", "□ (left)", 8, IDIT_BUTTON_CAN_RAPID, NULL },

 { "rstick_right", "Right Stick RIGHT →", 22, IDIT_BUTTON_ANALOG },
 { "rstick_left", "Right Stick LEFT ←", 21, IDIT_BUTTON_ANALOG },
 { "rstick_down", "Right Stick DOWN ↓", 20, IDIT_BUTTON_ANALOG },
 { "rstick_up", "Right Stick UP ↑", 19, IDIT_BUTTON_ANALOG },

 { "lstick_right", "Left Stick RIGHT →", 17, IDIT_BUTTON_ANALOG },
 { "lstick_left", "Left Stick LEFT ←", 16, IDIT_BUTTON_ANALOG },
 { "lstick_down", "Left Stick DOWN ↓", 15, IDIT_BUTTON_ANALOG },
 { "lstick_up", "Left Stick UP ↑", 14, IDIT_BUTTON_ANALOG },

};

// Not sure if all these buttons are named correctly!
InputDeviceInputInfoStruct Device_AnalogJoy_IDII[24] =
{
 { "select", "SELECT", 8, IDIT_BUTTON, NULL },
 { NULL, "empty", 0, IDIT_BUTTON },
 { NULL, "empty", 0, IDIT_BUTTON },
 { "start", "START", 9, IDIT_BUTTON, NULL },
 { "up", "Thumbstick UP ↑", 14, IDIT_BUTTON, "down" },
 { "right", "Thumbstick RIGHT →", 17, IDIT_BUTTON, "left" },
 { "down", "Thumbstick DOWN ↓", 15, IDIT_BUTTON, "up" },
 { "left", "Thumbstick LEFT ←", 16, IDIT_BUTTON, "right" },

 { "l2", "Left stick, Trigger", 2, IDIT_BUTTON, NULL },
 { "r2", "Left stick, Pinky", 3, IDIT_BUTTON, NULL },
 { "l1", "Left stick, L-thumb", 0, IDIT_BUTTON, NULL },
 { "r1", "Left stick, R-thumb", 1, IDIT_BUTTON, NULL },

 { "triangle", "Right stick, Pinky", 13, IDIT_BUTTON, NULL },
 { "circle", "Right stick, R-thumb", 11, IDIT_BUTTON, NULL },
 { "cross",  "Right stick, L-thumb", 10, IDIT_BUTTON, NULL },
 { "square", "Right stick, Trigger", 12, IDIT_BUTTON, NULL },

 { "rstick_right", "Right Stick, RIGHT →", 21, IDIT_BUTTON_ANALOG },
 { "rstick_left", "Right Stick, LEFT ←", 20, IDIT_BUTTON_ANALOG },
 { "rstick_down", "Right Stick, BACK ↓", 19, IDIT_BUTTON_ANALOG },
 { "rstick_up", "Right Stick, FORE ↑", 18, IDIT_BUTTON_ANALOG },

 { "lstick_right", "Left Stick, RIGHT →", 7, IDIT_BUTTON_ANALOG },
 { "lstick_left", "Left Stick, LEFT ←", 6, IDIT_BUTTON_ANALOG },
 { "lstick_down", "Left Stick, BACK ↓", 5, IDIT_BUTTON_ANALOG },
 { "lstick_up", "Left Stick, FORE ↑", 4, IDIT_BUTTON_ANALOG },

};


}
