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
#include "justifier.h"

namespace MDFN_IEN_PSX
{

class InputDevice_Justifier : public InputDevice
{
 public:

 InputDevice_Justifier(void);
 virtual ~InputDevice_Justifier();

 virtual void Power(void);
 virtual void UpdateInput(const void *data);
 virtual void SetCrosshairsColor(uint32 color);
 virtual bool RequireNoFrameskip(void);
 virtual pscpu_timestamp_t GPULineHook(const pscpu_timestamp_t timestamp, bool vsync, uint32 *pixels, const MDFN_PixelFormat* const format, const unsigned width, const unsigned pix_clock_offset, const unsigned pix_clock);

 //
 //
 //
 virtual void SetDTR(bool new_dtr);
 virtual bool GetDSR(void);
 virtual bool Clock(bool TxD, int32 &dsr_pulse_delay);

 private:

 bool dtr;

 uint8 buttons;
 bool trigger_eff;
 bool trigger_noclear;

 bool need_hit_detect;

 int32 nom_x, nom_y;
 int32 os_shot_counter;
 bool prev_oss;

 int32 command_phase;
 uint32 bitpos;
 uint8 receive_buffer;

 uint8 command;

 uint8 transmit_buffer[16];
 uint32 transmit_pos;
 uint32 transmit_count;

 //
 // Video timing stuff
 bool prev_vsync;
 int line_counter;

 //
 unsigned chair_r, chair_g, chair_b;
 bool draw_chair;
};

InputDevice_Justifier::InputDevice_Justifier(void) : chair_r(0), chair_g(0), chair_b(0), draw_chair(false)
{
 Power();
}

InputDevice_Justifier::~InputDevice_Justifier()
{

}

void InputDevice_Justifier::SetCrosshairsColor(uint32 color)
{
 chair_r = (color >> 16) & 0xFF;
 chair_g = (color >>  8) & 0xFF;
 chair_b = (color >>  0) & 0xFF;

 draw_chair = (color != (1 << 24));
}

void InputDevice_Justifier::Power(void)
{
 dtr = 0;

 buttons = 0;
 trigger_eff = 0;
 trigger_noclear = 0;

 need_hit_detect = false;

 nom_x = 0;
 nom_y = 0;

 os_shot_counter = 0;
 prev_oss = 0;

 command_phase = 0;

 bitpos = 0;

 receive_buffer = 0;

 command = 0;

 memset(transmit_buffer, 0, sizeof(transmit_buffer));

 transmit_pos = 0;
 transmit_count = 0;

 prev_vsync = 0;
 line_counter = 0;
}

void InputDevice_Justifier::UpdateInput(const void *data)
{
 uint8 *d8 = (uint8 *)data;

 nom_x = MDFN_de32lsb(&d8[0]);
 nom_y = MDFN_de32lsb(&d8[4]);

 trigger_noclear = (bool)(d8[8] & 0x1);
 trigger_eff |= trigger_noclear;

 buttons = (d8[8] >> 1) & 0x3;

 if(os_shot_counter > 0)	// FIXME if UpdateInput() is ever called more than once per video frame(at ~50 or ~60Hz).
  os_shot_counter--;

 if((d8[8] & 0x8) && !prev_oss && os_shot_counter == 0)
  os_shot_counter = 10;
 prev_oss = d8[8] & 0x8;
}

bool InputDevice_Justifier::RequireNoFrameskip(void)
{
 return(true);
}

pscpu_timestamp_t InputDevice_Justifier::GPULineHook(const pscpu_timestamp_t timestamp, bool vsync, uint32 *pixels, const MDFN_PixelFormat* const format, const unsigned width,
				     const unsigned pix_clock_offset, const unsigned pix_clock)
{
 pscpu_timestamp_t ret = PSX_EVENT_MAXTS;

 if(vsync && !prev_vsync)
  line_counter = 0;

 if(pixels && pix_clock)
 {
  const int avs = 16; // Not 16 for PAL, fixme.
  int32 gx;
  int32 gy;
  int32 gxa;

  gx = ((int64)nom_x * width / MDFNGameInfo->nominal_width + 0x8000) >> 16;
  gy = (nom_y + 0x8000) >> 16;
  gxa = gx; // - (pix_clock / 400000);
  //if(gxa < 0 && gx >= 0)
  // gxa = 0;

  if(!os_shot_counter && need_hit_detect && gxa >= 0 && gxa < (int)width && line_counter >= (avs + gy - 1) && line_counter <= (avs + gy + 1))
  {
   int r, g, b, a;

   format->DecodeColor(pixels[gxa], r, g, b, a);

   if((r + g + b) >= 0x40)	// Wrong, but not COMPLETELY ABSOLUTELY wrong, at least. ;)
   {
    ret = timestamp + (int64)(gxa + pix_clock_offset) * (44100 * 768) / pix_clock - 177;
   }
  }

  if(draw_chair)
  {
   if(line_counter == (avs + gy))
   {
    const int ic = pix_clock / 762925;

    for(int32 x = std::max<int32>(0, gx - ic); x < std::min<int32>(width, gx + ic); x++)
    {
     int r, g, b, a;
     int nr, ng, nb;

     format->DecodeColor(pixels[x], r, g, b, a);

     nr = (r + chair_r * 3) >> 2;
     ng = (g + chair_g * 3) >> 2;
     nb = (b + chair_b * 3) >> 2;

     if(abs((r * 76 + g * 150 + b * 29) - (nr * 76 + ng * 150 + nb * 29)) < 16384)
     { 
      nr >>= 1;
      ng >>= 1;
      nb >>= 1;
     }

     pixels[x] = format->MakeColor(nr, ng, nb, a);
    }
   }
   else if(line_counter >= (avs + gy - 8) && line_counter <= (avs + gy + 8))
   {
    int r, g, b, a;
    int nr, ng, nb;

    format->DecodeColor(pixels[gx], r, g, b, a);

    nr = (r + chair_r * 3) >> 2;
    ng = (g + chair_g * 3) >> 2;
    nb = (b + chair_b * 3) >> 2;

    if(abs((r * 76 + g * 150 + b * 29) - (nr * 76 + ng * 150 + nb * 29)) < 16384)
    { 
     nr >>= 1;
     ng >>= 1;
     nb >>= 1;
    }

    pixels[gx] = format->MakeColor(nr, ng, nb, a);
   }
  }
 }

 line_counter++;

 return(ret);
}

void InputDevice_Justifier::SetDTR(bool new_dtr)
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

bool InputDevice_Justifier::GetDSR(void)
{
 if(!dtr)
  return(0);

 if(!bitpos && transmit_count)
  return(1);

 return(0);
}

bool InputDevice_Justifier::Clock(bool TxD, int32 &dsr_pulse_delay)
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
	   transmit_buffer[0] = 0x31;
	   transmit_pos = 0;
	   transmit_count = 1;
	   command_phase++;
	  }
	  break;

   case 2:
	//if(receive_buffer)
	// printf("%02x\n", receive_buffer);
	command_phase++;
	break;

   case 3:
	need_hit_detect = receive_buffer & 0x10;	// TODO, see if it's (val&0x10) == 0x10, or some other mask value.
	command_phase++;
	break;

   case 1:
	command = receive_buffer;
	command_phase++;

	transmit_buffer[0] = 0x5A;

	//if(command != 0x42)
	// fprintf(stderr, "Justifier unhandled command: 0x%02x\n", command);
	//assert(command == 0x42);
	if(command == 0x42)
	{
	 transmit_buffer[1] = 0xFF ^ ((buttons & 2) << 2);
	 transmit_buffer[2] = 0xFF ^ (trigger_eff << 7) ^ ((buttons & 1) << 6);

	 if(os_shot_counter > 0)
	 {
	  transmit_buffer[2] |= (1 << 7);
	  if(os_shot_counter == 6 || os_shot_counter == 5)
	  {
	   transmit_buffer[2] &= ~(1 << 7);
	  }
	 }

         transmit_pos = 0;
         transmit_count = 3;

	 trigger_eff = trigger_noclear;
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

  }
 }

 if(!bitpos && transmit_count)
  dsr_pulse_delay = 200;

 return(ret);
}

InputDevice *Device_Justifier_Create(void)
{
 return new InputDevice_Justifier();
}


InputDeviceInputInfoStruct Device_Justifier_IDII[6] =
{
 { "x_axis", "X Axis", -1, IDIT_X_AXIS },
 { "y_axis", "Y Axis", -1, IDIT_Y_AXIS },

 { "trigger", "Trigger", 0, IDIT_BUTTON, NULL  },

 { "o",	"O",		 1, IDIT_BUTTON,	NULL },
 { "start", "Start",	 2, IDIT_BUTTON,	NULL },

 { "offscreen_shot", "Offscreen Shot(Simulated)", 3, IDIT_BUTTON, NULL },
};



}
