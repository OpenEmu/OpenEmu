#include "../psx.h"
#include "../frontio.h"
#include "multitap.h"

/*
 Notes from tests on real thing(not necessarily emulated the same way here):

	Manual port selection read mode:
		Write 0x01-0x04 instead of 0x01 as first byte, selects port(1=A,2=B,3=C,4=D) to access.

		Ports that don't exist(0x00, 0x05-0xFF) or don't have a device plugged in will not respond(no DSR pulse).

	Full read mode:
		Bit0 of third byte(from-zero-index=0x02) should be set to 1 to enter full read mode, on subsequent reads.

		Appears to require a controller to be plugged into the port specified by the first byte as per manual port selection read mode,
		to write the byte necessary to enter full-read mode; but once the third byte with the bit set has been written, no controller in
		that port is required for doing full reads(and the manual port selection is ignored when doing a full read).

		However, if there are no controllers plugged in, or the first byte written in a full-mode communication has one or more bits in the upper
		nybble set to 1, the returned data will be short:
			% 0: 0xff
			% 1: 0x80
			% 2: 0x5a

		Example full-read bytestream(with controllers plugged into port A, port B, and port C, with port D empty):
			% 0: 0xff
			% 1: 0x80
			% 2: 0x5a

			% 3: 0x73	(Port A controller data start)
			% 4: 0x5a
			% 5: 0xff
			% 6: 0xff
			% 7: 0x80
			% 8: 0x8c
			% 9: 0x79
			% 10: 0x8f

			% 11: 0x53	(Port B controller data start)
			% 12: 0x5a
			% 13: 0xff
			% 14: 0xff
			% 15: 0x80
			% 16: 0x80
			% 17: 0x75
			% 18: 0x8e

			% 19: 0x41	(Port C controller data start)
			% 20: 0x5a
			% 21: 0xff
			% 22: 0xff
			% 23: 0xff
			% 24: 0xff
			% 25: 0xff
			% 26: 0xff

			% 27: 0xff	(Port D controller data start)
			% 28: 0xff
			% 29: 0xff
			% 30: 0xff
			% 31: 0xff
			% 32: 0xff
			% 33: 0xff
			% 34: 0xff

*/

namespace MDFN_IEN_PSX
{

InputDevice_Multitap::InputDevice_Multitap()
{
 for(int i = 0; i < 4; i++)
 {
  pad_devices[i] = NULL;
  mc_devices[i] = NULL;
 }
 Power();
}

InputDevice_Multitap::~InputDevice_Multitap()
{
}

void InputDevice_Multitap::SetSubDevice(unsigned int sub_index, InputDevice *device, InputDevice *mc_device)
{
 assert(sub_index < 4);

 //printf("%d\n", sub_index);

 pad_devices[sub_index] = device;
 mc_devices[sub_index] = mc_device;
}


void InputDevice_Multitap::Power(void)
{
 selected_device = -1;
 bit_counter = 0;
 receive_buffer = 0;
 byte_counter = 0;

 mc_mode = false;
 full_mode = false;
 full_mode_setting = false;

 for(int i = 0; i < 4; i++)
 {
  if(pad_devices[i])
   pad_devices[i]->Power();

  if(mc_devices[i])
   mc_devices[i]->Power();
 } 
}

void InputDevice_Multitap::SetDTR(bool new_dtr)
{
 bool old_dtr = dtr;
 dtr = new_dtr;

 if(!dtr)
 {
  bit_counter = 0;
  byte_counter = 0;
  receive_buffer = 0;
  selected_device = -1;
  mc_mode = false;
  full_mode = false;
 }

 if(!old_dtr && dtr)
 {
  full_mode = full_mode_setting;
  //if(full_mode) {
  // printf("Full mode start\n"); }
 }

 for(int i = 0; i < 4; i++)
 {
  pad_devices[i]->SetDTR(dtr);
  mc_devices[i]->SetDTR(dtr);
 }
}

bool InputDevice_Multitap::GetDSR(void)
{
 return(0);
}

bool InputDevice_Multitap::Clock(bool TxD, int32 &dsr_pulse_delay)
{
 if(!dtr)
  return(1);

 bool ret = 1;
 bool mangled_txd = TxD;
 int32 tmp_pulse_delay[2][4] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };

 //printf("Receive bit: %d\n", TxD);
 //printf("TxD %d\n", TxD);

 receive_buffer &= ~ (1 << bit_counter);
 receive_buffer |= TxD << bit_counter;

 if(1)
 {
  uint8 sub_clock = 0;
  uint8 sub_rxd_ignore = 0;

  if(full_mode)
  {
   if(byte_counter == 0)
   {
    sub_clock = 0;
    sub_rxd_ignore = 0xF;
   }
   else if(byte_counter == 1)
   {
    ret = (0x80 >> bit_counter) & 1;
    sub_clock = false;
    sub_rxd_ignore = 0xF;
   }
   else if(byte_counter == 2)
   {
    ret = (0x5A >> bit_counter) & 1;
    sub_rxd_ignore = 0xF;

    if(!mc_mode)
    {
     sub_clock = 0xF;
     mangled_txd = (0x01 >> bit_counter) & 1;
    }
   }
   else if(byte_counter == 0x03 || byte_counter == (0x03 + 0x08 * 1) || byte_counter == (0x03 + 0x08 * 2) || byte_counter == (0x03 + 0x08 * 3))
   {
    sub_clock = 1 << selected_device;
    sub_rxd_ignore = 0;
    mangled_txd = (command >> bit_counter) & 1;
   }
   else
   {
    sub_clock = 1 << selected_device;
    sub_rxd_ignore = 0;
    // Not sure about this, would need to test with rumble-capable device on real thing?
    //mangled_txd = (0x00 >> bit_counter) & 1;
   }
  }
  else
  {
   if(byte_counter == 0)
   {
    if(bit_counter < 4)
     mangled_txd = (0x01 >> bit_counter) & 1;

    sub_clock = 0xF;
    sub_rxd_ignore = 0xF;
   }
   else if((unsigned)selected_device < 4)
   {
    sub_clock = 1 << selected_device;
    sub_rxd_ignore = 0;
   }
  }

  for(int i = 0; i < 4; i++)
  {
   if(sub_clock & (1 << i))
   {
    ret &= pad_devices[i]->Clock(mangled_txd, tmp_pulse_delay[0][i]) | ((sub_rxd_ignore >> i) & 1);
    ret &= mc_devices[i]->Clock(mangled_txd, tmp_pulse_delay[1][i]) | ((sub_rxd_ignore >> i) & 1);
   }
  }
 }

  
#if 0
 {
  static uint8 sendy = 0;

  sendy &= ~(1 << bit_counter);
  sendy |= ret << bit_counter;

  if(bit_counter == 7)
   printf("Multitap to PSX: 0x%02x\n", sendy);
 }
#endif


 bit_counter = (bit_counter + 1) & 0x7;
 if(bit_counter == 0)
 {
  if(byte_counter == 0)
  {
   mc_mode = (bool)(receive_buffer & 0xF0);

   //printf("Full mode: %d %d %d\n", full_mode, bit_counter, byte_counter);

   if(full_mode)
    selected_device = 0;
   else
   {
    //printf("Device select: %02x\n", receive_buffer);
    selected_device = ((receive_buffer & 0xF) - 1) & 0xFF;
   }
  }

  if(byte_counter == 1)
  {
   command = receive_buffer;
   //printf("Multitap sub-command: %02x\n", command);
  }

  if((!mc_mode || full_mode) && byte_counter == 2 && command == 0x42)
  {
   //printf("Full mode setting: %02x\n", receive_buffer);
   full_mode_setting = receive_buffer & 0x01;
  }

  // Handle DSR stuff
  if(full_mode)
  {
   if(byte_counter == 0 || byte_counter == 1)
    dsr_pulse_delay = 0x40;
   else if(byte_counter == 2 || byte_counter == 3)
   {
    //int32 td = 0;
    //for(int i = 0; i < 4; i++)
    //{
    // td = std::max<int32>(td, tmp_pulse_delay[0][i]);
    // td = std::max<int32>(td, tmp_pulse_delay[1][i]);
    //}
    //dsr_pulse_delay = td;
    //printf("%d %d\n", byte_counter, dsr_pulse_delay);

    // Just route the first port's DSR through here; at least one game relies on this(Formula One 2000), or else it freezes.  Well, even when it doesn't
    // freeze, the game crashes(as of Jan 20, 2012), but that's not the multitap emulation's fault. :b
    dsr_pulse_delay = std::max<int32>(tmp_pulse_delay[0][0], tmp_pulse_delay[1][0]);
   }
   else if(byte_counter > 3 && byte_counter < 34)
   {
    dsr_pulse_delay = 0x80;
   }
  }
  else
  {
   if((unsigned)selected_device < 4)
   {
    dsr_pulse_delay = std::max<int32>(tmp_pulse_delay[0][selected_device], tmp_pulse_delay[1][selected_device]);
   }
  }


  //
  //
  //

  //printf("Byte Counter Increment\n");
  if(byte_counter < 255)
   byte_counter++;

  if(full_mode && (byte_counter == (0x03 + 0x08 * 1) || byte_counter == (0x03 + 0x08 * 2) || byte_counter == (0x03 + 0x08 * 3)))
  {
   //printf("Device Select Increment\n");
   selected_device++;
  }
 }



 return(ret);
}

}
