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

// I could find no other commands than 'R', 'W', and 'S' (not sure what 'S' is for, however)

#include "../psx.h"
#include "../frontio.h"
#include "memcard.h"

namespace MDFN_IEN_PSX
{

class InputDevice_Memcard : public InputDevice
{
 public:

 InputDevice_Memcard();
 virtual ~InputDevice_Memcard();

 virtual void Power(void);

 //
 //
 //
 virtual void SetDTR(bool new_dtr);
 virtual bool GetDSR(void);
 virtual bool Clock(bool TxD, int32 &dsr_pulse_delay);

 //
 //
 virtual uint32 GetNVSize(void);
 virtual void ReadNV(uint8 *buffer, uint32 offset, uint32 size);
 virtual void WriteNV(const uint8 *buffer, uint32 offset, uint32 size);

 virtual uint64 GetNVDirtyCount(void);
 virtual void ResetNVDirtyCount(void);

 private:

 bool presence_new;

 uint8 card_data[1 << 17];
 uint8 rw_buffer[128];
 uint8 write_xor;

 uint64 dirty_count;

 bool dtr;
 int32 command_phase;
 uint32 bitpos;
 uint8 receive_buffer;

 uint8 command;
 uint16 addr;
 uint8 calced_xor;

 uint8 transmit_buffer;
 uint32 transmit_count;
};

InputDevice_Memcard::InputDevice_Memcard()
{
 Power();

 dirty_count = 0;

 // Init memcard as formatted.
 assert(sizeof(card_data) == (1 << 17));
 memset(card_data, 0x00, sizeof(card_data));

 card_data[0x00] = 0x4D;
 card_data[0x01] = 0x43;
 card_data[0x7F] = 0x0E;

 for(unsigned int A = 0x80; A < 0x800; A += 0x80)
 {
  card_data[A + 0x00] = 0xA0;
  card_data[A + 0x08] = 0xFF;
  card_data[A + 0x09] = 0xFF;
  card_data[A + 0x7F] = 0xA0;
 }

 for(unsigned int A = 0x0800; A < 0x1200; A += 0x80)
 {
  card_data[A + 0x00] = 0xFF;
  card_data[A + 0x01] = 0xFF;
  card_data[A + 0x02] = 0xFF;
  card_data[A + 0x03] = 0xFF;
  card_data[A + 0x08] = 0xFF;
  card_data[A + 0x09] = 0xFF;
 }

}

InputDevice_Memcard::~InputDevice_Memcard()
{

}

void InputDevice_Memcard::Power(void)
{
 dtr = 0;

 //buttons[0] = buttons[1] = 0;

 command_phase = 0;

 bitpos = 0;

 receive_buffer = 0;

 command = 0;

 transmit_buffer = 0;

 transmit_count = 0;

 addr = 0;

 presence_new = true;
}

void InputDevice_Memcard::SetDTR(bool new_dtr)
{
 if(!dtr && new_dtr)
 {
  command_phase = 0;
  bitpos = 0;
  transmit_count = 0;
 }
 else if(dtr && !new_dtr)
 {
  if(command_phase > 0)
   PSX_WARNING("[MCR] Communication aborted???");
 }
 dtr = new_dtr;
}

bool InputDevice_Memcard::GetDSR(void)
{
 if(!dtr)
  return(0);

 if(!bitpos && transmit_count)
  return(1);

 return(0);
}

bool InputDevice_Memcard::Clock(bool TxD, int32 &dsr_pulse_delay)
{
 bool ret = 1;

 dsr_pulse_delay = 0;

 if(!dtr)
  return(1);

 if(transmit_count)
  ret = (transmit_buffer >> bitpos) & 1;

 receive_buffer &= ~(1 << bitpos);
 receive_buffer |= TxD << bitpos;
 bitpos = (bitpos + 1) & 0x7;

 if(!bitpos)
 {
  //if(command_phase > 0 || transmit_count)
  // printf("[MCRDATA] Received_data=0x%02x, Sent_data=0x%02x\n", receive_buffer, transmit_buffer);

  if(transmit_count)
  {
   transmit_count--;
  }


  switch(command_phase)
  {
   case 0:
          if(receive_buffer != 0x81)
            command_phase = -1;
          else
          {
	   //printf("[MCR] Device selected\n");
           transmit_buffer = presence_new ? 0x08 : 0x00;
           transmit_count = 1;
           command_phase++;
          }
          break;

   case 1:
        command = receive_buffer;
	//printf("[MCR] Command received: %c\n", command);
	if(command == 'R' || command == 'W')
	{
	 command_phase++;
         transmit_buffer = 0x5A;
         transmit_count = 1;
	}
	else
	{
	 if(command == 'S')
	 {
	  PSX_WARNING("[MCR] Memcard S command unsupported.");
	 }

	 command_phase = -1;
	 transmit_buffer = 0;
	 transmit_count = 0;
	}
        break;

   case 2:
	transmit_buffer = 0x5D;
	transmit_count = 1;
	command_phase++;
	break;

   case 3:
	transmit_buffer = 0x00;
	transmit_count = 1;
	if(command == 'R')
	 command_phase = 1000;
	else if(command == 'W')
	 command_phase = 2000;
	break;

  //
  // Read
  //
  case 1000:
	addr = receive_buffer << 8;
	transmit_buffer = receive_buffer;
	transmit_count = 1;
	command_phase++;
	break;

  case 1001:
	addr |= receive_buffer & 0xFF;
	transmit_buffer = '\\';
	transmit_count = 1;
	command_phase++;
	break;

  case 1002:
	//printf("[MCR]   READ ADDR=0x%04x\n", addr);
	if(addr >= (sizeof(card_data) >> 7))
	 addr = 0xFFFF;

	calced_xor = 0;
	transmit_buffer = ']';
	transmit_count = 1;
	command_phase++;

	// TODO: enable this code(or something like it) when CPU instruction timing is a bit better.
	//
	//dsr_pulse_delay = 32000;
	//goto SkipDPD;
	//

	break;

  case 1003:
	transmit_buffer = addr >> 8;
	calced_xor ^= transmit_buffer;
	transmit_count = 1;
	command_phase++;
	break;

  case 1004:
	transmit_buffer = addr & 0xFF;
	calced_xor ^= transmit_buffer;

	if(addr == 0xFFFF)
	{
	 transmit_count = 1;
	 command_phase = -1;
	}
	else
	{
	 transmit_count = 1;
	 command_phase = 1024;
	}
	break;

  // Transmit actual 128 bytes data
  case (1024 + 0) ... (1024 + 128 - 1):
	transmit_buffer = card_data[(addr << 7) + (command_phase - 1024)];
	calced_xor ^= transmit_buffer;
	transmit_count = 1;
	command_phase++;
	break;

  // XOR
  case (1024 + 128):
	transmit_buffer = calced_xor;
	transmit_count = 1;
	command_phase++;
	break;

  // End flag
  case (1024 + 129):
	transmit_buffer = 'G';
	transmit_count = 1;
	command_phase = -1;
	break;

  //
  // Write
  //
  case 2000:
	calced_xor = receive_buffer;
        addr = receive_buffer << 8;
        transmit_buffer = receive_buffer;
        transmit_count = 1;
        command_phase++;
	break;

  case 2001:
	calced_xor ^= receive_buffer;
        addr |= receive_buffer & 0xFF;
	//printf("[MCR]   WRITE ADDR=0x%04x\n", addr);
        transmit_buffer = receive_buffer;
        transmit_count = 1;
        command_phase = 2048;
        break;

  case (2048 + 0) ... (2048 + 128 - 1):
	calced_xor ^= receive_buffer;
	rw_buffer[command_phase - 2048] = receive_buffer;

        transmit_buffer = receive_buffer;
        transmit_count = 1;
        command_phase++;
        break;

  case (2048 + 128):	// XOR
	write_xor = receive_buffer;
	transmit_buffer = '\\';
	transmit_count = 1;
	command_phase++;
	break;

  case (2048 + 129):
	transmit_buffer = ']';
	transmit_count = 1;
	command_phase++;
	break;

  case (2048 + 130):	// End flag
	//MDFN_DispMessage("%02x %02x", calced_xor, write_xor);
	//printf("[MCR] Write End.  Actual_XOR=0x%02x, CW_XOR=0x%02x\n", calced_xor, write_xor);

	if(calced_xor != write_xor)
 	 transmit_buffer = 'N';
	else if(addr >= (sizeof(card_data) >> 7))
	 transmit_buffer = 0xFF;
	else
	{
	 transmit_buffer = 'G';
	 presence_new = false;

	 // If the current data is different from the data to be written, increment the dirty count.
	 // memcpy()'ing over to card_data is also conditionalized here for a slight optimization.
         if(memcmp(&card_data[addr << 7], rw_buffer, 128))
	 {
	  memcpy(&card_data[addr << 7], rw_buffer, 128);
	  dirty_count++;
	 }
	}

	transmit_count = 1;
	command_phase = -1;
	break;

  }

  //if(command_phase != -1 || transmit_count)
  // printf("[MCR] Receive: 0x%02x, Send: 0x%02x -- %d\n", receive_buffer, transmit_buffer, command_phase);
 }

 if(!bitpos && transmit_count)
  dsr_pulse_delay = 0x100;

 //SkipDPD: ;

 return(ret);
}

uint32 InputDevice_Memcard::GetNVSize(void)
{
 return(sizeof(card_data));
}

void InputDevice_Memcard::ReadNV(uint8 *buffer, uint32 offset, uint32 size)
{
 while(size--)
 {
  *buffer = card_data[offset & (sizeof(card_data) - 1)];
  buffer++;
  offset++;
 }
}

void InputDevice_Memcard::WriteNV(const uint8 *buffer, uint32 offset, uint32 size)
{
 if(size)
  dirty_count++;

 while(size--)
 {
  card_data[offset & (sizeof(card_data) - 1)] = *buffer;
  buffer++;
  offset++;
 }
}

uint64 InputDevice_Memcard::GetNVDirtyCount(void)
{
 return(dirty_count);
}

void InputDevice_Memcard::ResetNVDirtyCount(void)
{
 dirty_count = 0;
}


InputDevice *Device_Memcard_Create(void)
{
 return new InputDevice_Memcard();
}

}
