#include "../psx.h"
#include "../frontio.h"
#include "mouse.h"

namespace MDFN_IEN_PSX
{

class InputDevice_Mouse : public InputDevice
{
 public:

 InputDevice_Mouse();
 virtual ~InputDevice_Mouse();

 virtual void Power(void);
 virtual void UpdateInput(const void *data);

 virtual void Update(const pscpu_timestamp_t timestamp);
 virtual void ResetTS(void);

 //
 //
 //
 virtual void SetDTR(bool new_dtr);
 virtual bool Clock(bool TxD, int32 &dsr_pulse_delay);

 private:

 int32 lastts;
 int32 clear_timeout;

 bool dtr;

 uint8 button;
 uint8 button_post_mask;
 int32 accum_xdelta;
 int32 accum_ydelta;

 int32 command_phase;
 uint32 bitpos;
 uint8 receive_buffer;

 uint8 command;

 uint8 transmit_buffer[5];
 uint32 transmit_pos;
 uint32 transmit_count;
};

InputDevice_Mouse::InputDevice_Mouse()
{
 Power();
}

InputDevice_Mouse::~InputDevice_Mouse()
{
 
}

void InputDevice_Mouse::Update(const pscpu_timestamp_t timestamp)
{
 int32 cycles = timestamp - lastts;

 clear_timeout += cycles;
 if(clear_timeout >= (33868800 / 4))
 {
  //puts("Mouse timeout\n");
  clear_timeout = 0;
  accum_xdelta = 0;
  accum_ydelta = 0;
  button &= button_post_mask;
 }

 lastts = timestamp;
}

void InputDevice_Mouse::ResetTS(void)
{
 lastts = 0;
}

void InputDevice_Mouse::Power(void)
{
 lastts = 0;
 clear_timeout = 0;

 dtr = 0;

 button = 0;
 button_post_mask = 0;
 accum_xdelta = 0;
 accum_ydelta = 0;

 command_phase = 0;

 bitpos = 0;

 receive_buffer = 0;

 command = 0;

 memset(transmit_buffer, 0, sizeof(transmit_buffer));

 transmit_pos = 0;
 transmit_count = 0;
}

void InputDevice_Mouse::UpdateInput(const void *data)
{
 accum_xdelta += (int32)MDFN_de32lsb((uint8*)data + 0);
 accum_ydelta += (int32)MDFN_de32lsb((uint8*)data + 4);

 if(accum_xdelta > 30 * 127) accum_xdelta = 30 * 127;
 if(accum_xdelta < 30 * -128) accum_xdelta = 30 * -128;

 if(accum_ydelta > 30 * 127) accum_ydelta = 30 * 127;
 if(accum_ydelta < 30 * -128) accum_ydelta = 30 * -128;

 button |= *((uint8 *)data + 8);
 button_post_mask = *((uint8 *)data + 8);

 //if(button)
 // MDFN_DispMessage("Button\n");
 //printf("%d %d\n", accum_xdelta, accum_ydelta);
}


void InputDevice_Mouse::SetDTR(bool new_dtr)
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

bool InputDevice_Mouse::Clock(bool TxD, int32 &dsr_pulse_delay)
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
	   transmit_buffer[0] = 0x12;
	   transmit_pos = 0;
	   transmit_count = 1;
	   command_phase++;
	  }
	  break;

   case 1:
	command = receive_buffer;
	command_phase++;

	transmit_buffer[0] = 0x5A;

	if(command == 0x42)
	{
	 int32 xdelta = accum_xdelta;
	 int32 ydelta = accum_ydelta;

	 if(xdelta < -128) xdelta = -128;
	 if(xdelta > 127) xdelta = 127;

	 if(ydelta < -128) ydelta = -128;
	 if(ydelta > 127) ydelta = 127;

	 transmit_buffer[1] = 0xFF;
	 transmit_buffer[2] = 0xFC ^ (button << 2);
	 transmit_buffer[3] = xdelta;
         transmit_buffer[4] = ydelta;

	 accum_xdelta -= xdelta;
	 accum_ydelta -= ydelta;

	 button &= button_post_mask;

         transmit_pos = 0;
         transmit_count = 5;

	 clear_timeout = 0;
	}
	else
	{
	 command_phase = -1;
         transmit_pos = 0;
         transmit_count = 0;
	}
	break;

  }
 }

 if(!bitpos && transmit_count)
  dsr_pulse_delay = 0x40; //0x100;

 return(ret);
}

InputDevice *Device_Mouse_Create(void)
{
 return new InputDevice_Mouse();
}


InputDeviceInputInfoStruct Device_Mouse_IDII[4] =
{
 { "x_axis", "X Axis", -1, IDIT_X_AXIS_REL },
 { "y_axis", "Y Axis", -1, IDIT_Y_AXIS_REL },
 { "right", "Right Button", 1, IDIT_BUTTON, NULL },
 { "left", "Left Button", 0, IDIT_BUTTON, NULL },
};



}
