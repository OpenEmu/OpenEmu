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
#include "dualshock.h"

/*
   TODO:
	If we ever call Update() more than once per video frame(IE 50/60Hz), we'll need to add debounce logic to the analog mode button evaluation code.
*/

/* Notes:

     Both DA and DS style rumblings work in both analog and digital modes.

     Regarding getting Dual Shock style rumble working, Sony is evil and/or mean.  The owl tells me to burn Sony with boiling oil.

     To enable Dual Shock-style rumble, the game has to at least enter MAD MUNCHKINS MODE with command 0x43, and send the appropriate data(not the actual rumble type data per-se)
     with command 0x4D.

     DualAnalog-style rumble support seems to be borked until power loss if MAD MUNCHKINS MODE is even entered once...investigate further.

     Command 0x44 in MAD MUNCHKINS MODE can turn on/off analog mode(and the light with it).

     Command 0x42 in MAD MUNCHKINS MODE will return the analog mode style gamepad data, even when analog mode is off.  In combination with command 0x44, this could hypothetically
     be used for using the light in the gamepad as some kind of game mechanic).

     Dual Analog-style rumble notes(some of which may apply to DS too):
	Rumble appears to stop if you hold DTR active(TODO: for how long? instant?). (TODO: investigate if it's still stopped even if a memory card device number is sent.  It may be, since rumble may
							      cause excessive current draw in combination with memory card access)

	Rumble will work even if you interrupt the communication process after you've sent the rumble data(via command 0x42).
		Though if you interrupt it when you've only sent partial rumble data, dragons will eat you and I don't know(seems to have timing-dependent or random effects or something;
	        based on VERY ROUGH testing).
*/

namespace MDFN_IEN_PSX
{

class InputDevice_DualShock : public InputDevice
{
 public:

 InputDevice_DualShock(const std::string &arg_name);
 virtual ~InputDevice_DualShock();

 virtual void Power(void);
 virtual void Update(const pscpu_timestamp_t timestamp);
 virtual void ResetTS(void);
 virtual void UpdateInput(const void *data);

 virtual void SetAMCT(bool enabled);
 //
 //
 //
 virtual void SetDTR(bool new_dtr);
 virtual bool GetDSR(void);
 virtual bool Clock(bool TxD, int32 &dsr_pulse_delay);

 private:

 void CheckManualAnaModeChange(void);

 //
 //
 bool cur_ana_button_state;
 bool prev_ana_button_state;
 int64 combo_anatoggle_counter;
 //

 bool da_rumble_compat;

 bool analog_mode;
 bool analog_mode_locked;

 bool mad_munchkins;
 uint8 rumble_magic[6];

 uint8 rumble_param[2];

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

 //
 //
 //
 bool am_prev_info;
 bool aml_prev_info;
 std::string gp_name;
 pscpu_timestamp_t lastts;

 //
 //
 bool amct_enabled;
};

InputDevice_DualShock::InputDevice_DualShock(const std::string &name)
{
 gp_name = name;
 Power();
 am_prev_info = analog_mode;
 aml_prev_info = analog_mode_locked;
 amct_enabled = false;
}

InputDevice_DualShock::~InputDevice_DualShock()
{

}

void InputDevice_DualShock::Update(const pscpu_timestamp_t timestamp)
{
 lastts = timestamp;
}

void InputDevice_DualShock::ResetTS(void)
{
 //printf("%lld\n", combo_anatoggle_counter);
 if(combo_anatoggle_counter >= 0)
  combo_anatoggle_counter += lastts;
 lastts = 0;
}

void InputDevice_DualShock::SetAMCT(bool enabled)
{
 amct_enabled = enabled;
}

//
// This simulates the behavior of the actual DualShock(analog toggle button evaluation is suspended while DTR is active).
// Call in Update(), and whenever dtr goes inactive in the port access code.
void InputDevice_DualShock::CheckManualAnaModeChange(void)
{
 if(!dtr)
 {
  bool need_mode_toggle = false;

  if(amct_enabled)
  {
   if(buttons[0] == 0x09 && buttons[1] == 0x0f)
   {
    if(combo_anatoggle_counter == -1)
     combo_anatoggle_counter = 0;
    else if(combo_anatoggle_counter >= (44100 * 768))
    {
     need_mode_toggle = true;
     combo_anatoggle_counter = -2;
    }
   }
   else
    combo_anatoggle_counter = -1;
  }  
  else
  {
   combo_anatoggle_counter = -1;
   if(cur_ana_button_state && (cur_ana_button_state != prev_ana_button_state))
   {
    need_mode_toggle = true;
   }
  }

  if(need_mode_toggle)
  {
   if(analog_mode_locked)
   {
    MDFN_DispMessage(_("%s: Analog mode is locked %s."), gp_name.c_str(), analog_mode ? _("on") : _("off"));
   }
   else
    analog_mode = !analog_mode;
  }

  prev_ana_button_state = cur_ana_button_state; 	// Don't move this outside of the if(!dtr) block!
 }
}

void InputDevice_DualShock::Power(void)
{
 combo_anatoggle_counter = -2;
 lastts = 0;
 //
 //

 dtr = 0;

 buttons[0] = buttons[1] = 0;

 command_phase = 0;

 bitpos = 0;

 receive_buffer = 0;

 command = 0;

 memset(transmit_buffer, 0, sizeof(transmit_buffer));

 transmit_pos = 0;
 transmit_count = 0;

 analog_mode = false;
 analog_mode_locked = false;

 mad_munchkins = false;
 memset(rumble_magic, 0, sizeof(rumble_magic));
 memset(rumble_param, 0, sizeof(rumble_param));

 da_rumble_compat = true;

 prev_ana_button_state = false;
}

void InputDevice_DualShock::UpdateInput(const void *data)
{
 uint8 *d8 = (uint8 *)data;

 buttons[0] = d8[0];
 buttons[1] = d8[1];
 cur_ana_button_state = d8[2] & 0x01;

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

 if(da_rumble_compat == false)
 {
  uint8 sneaky_weaky = 0;

  if(rumble_param[0] == 0x01)
   sneaky_weaky = 0xFF;

  MDFN_en32lsb(&d8[4 + 32 + 0], (sneaky_weaky << 0) | (rumble_param[1] << 8));
 }
 else
 {
  uint8 sneaky_weaky = 0;

  if(((rumble_param[0] & 0xC0) == 0x40) && ((rumble_param[1] & 0x01) == 0x01))
   sneaky_weaky = 0xFF;

  MDFN_en32lsb(&d8[4 + 32 + 0], sneaky_weaky << 0);
 }

 //printf("%d %d %d %d\n", axes[0][0], axes[0][1], axes[1][0], axes[1][1]);

 //
 //
 //
 CheckManualAnaModeChange();

 if(am_prev_info != analog_mode || aml_prev_info != analog_mode_locked)
 {
  MDFN_DispMessage(_("%s: Analog mode is %s(%s)."), gp_name.c_str(), analog_mode ? _("on") : _("off"), analog_mode_locked ? _("locked") : _("unlocked"));
 }
 am_prev_info = analog_mode;
 aml_prev_info = analog_mode_locked;
}


void InputDevice_DualShock::SetDTR(bool new_dtr)
{
 const bool old_dtr = dtr;
 dtr = new_dtr;	// Set it to new state before we call CheckManualAnaModeChange().

 if(!old_dtr && dtr)
 {
  command_phase = 0;
  bitpos = 0;
  transmit_pos = 0;
  transmit_count = 0;
 }
 else if(old_dtr && !dtr)
 {
  CheckManualAnaModeChange();
  //if(bitpos || transmit_count)
  // printf("[PAD] Abort communication!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
 }
}

bool InputDevice_DualShock::GetDSR(void)
{
 if(!dtr)
  return(0);

 if(!bitpos && transmit_count)
  return(1);

 return(0);
}

bool InputDevice_DualShock::Clock(bool TxD, int32 &dsr_pulse_delay)
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
  //if(command == 0x44)
  //if(mad_munchkins || command == 0x43)
  // fprintf(stderr, "[PAD] Receive: %02x -- command=%02x, command_phase=%d, transmit_pos=%d\n", receive_buffer, command, command_phase, transmit_pos);

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
	   if(mad_munchkins)
	   {
	    transmit_buffer[0] = 0xF3;
	    transmit_pos = 0;
	    transmit_count = 1;
	    command_phase = 101;
	   }
	   else
	   {
	    transmit_buffer[0] = analog_mode ? 0x73 : 0x41;
	    transmit_pos = 0;
	    transmit_count = 1;
	    command_phase++;
	   }
	  }
	  break;

   case 1:
	command = receive_buffer;
	command_phase++;

	transmit_buffer[0] = 0x5A;

	//fprintf(stderr, "Gamepad command: 0x%02x\n", command);
	//if(command != 0x42 && command != 0x43)
	// fprintf(stderr, "Gamepad unhandled command: 0x%02x\n", command);

	if(command == 0x42)
	{
	 transmit_buffer[0] = 0x5A;
	 transmit_pos = 0;
	 transmit_count = 1;
	 command_phase = (command << 8) | 0x00;
	}
	else if(command == 0x43)
	{
	 transmit_pos = 0;
	 if(analog_mode)
	 {
	  transmit_buffer[1] = 0xFF ^ buttons[0];
	  transmit_buffer[2] = 0xFF ^ buttons[1];
	  transmit_buffer[3] = axes[0][0];
	  transmit_buffer[4] = axes[0][1];
	  transmit_buffer[5] = axes[1][0];
	  transmit_buffer[6] = axes[1][1];
          transmit_count = 7;
	 }
	 else
	 {
	  transmit_buffer[1] = 0xFF ^ buttons[0];
 	  transmit_buffer[2] = 0xFF ^ buttons[1];
 	  transmit_count = 3;
	 }
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
	{
	 if(command == 0x43 && transmit_pos == 2 && (receive_buffer == 0x01))
	 {
	  //fprintf(stderr, "Mad Munchkins mode entered!\n");
	  mad_munchkins = true;

	  if(da_rumble_compat)
	  {
	   rumble_param[0] = 0;
	   rumble_param[1] = 0;
	   da_rumble_compat = false;
	  }
	  command_phase = -1;
	 }
	}
	break;

   case 101:
	command = receive_buffer;

	//fprintf(stderr, "Mad Munchkins DualShock command: 0x%02x\n", command);

	if(command >= 0x40 && command <= 0x4F)
	{
	 transmit_buffer[0] = 0x5A;
	 transmit_pos = 0;
	 transmit_count = 1;
	 command_phase = (command << 8) | 0x00;
	}
	else
	{
	 transmit_count = 0;
	 command_phase = -1;
	}
	break;

  /************************/
  /* MMMode 1, Command 0x40 */
  /************************/
  case 0x4000:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4001:
	transmit_buffer[0] = 0x00;
	transmit_buffer[1] = 0x00;
	transmit_buffer[2] = 0x00;
	transmit_buffer[3] = 0x00;
	transmit_buffer[4] = 0x00;
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;


  /************************/
  /* MMMode 1, Command 0x41 */
  /************************/
  case 0x4100:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4101:
	transmit_buffer[0] = 0x00;
	transmit_buffer[1] = 0x00;
	transmit_buffer[2] = 0x00;
	transmit_buffer[3] = 0x00;
	transmit_buffer[4] = 0x00;
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;

  /**************************/
  /* MMMode 0&1, Command 0x42 */
  /**************************/
  case 0x4200:
	transmit_pos = 0;
	if(analog_mode || mad_munchkins)
	{
	 transmit_buffer[0] = 0xFF ^ buttons[0];
	 transmit_buffer[1] = 0xFF ^ buttons[1];
	 transmit_buffer[2] = axes[0][0];
	 transmit_buffer[3] = axes[0][1];
	 transmit_buffer[4] = axes[1][0];
	 transmit_buffer[5] = axes[1][1];
 	 transmit_count = 6;
	}
	else
	{
	 transmit_buffer[0] = 0xFF ^ buttons[0];
	 transmit_buffer[1] = 0xFF ^ buttons[1];
	 transmit_count = 2;
	}
	command_phase++;
	break;
 
  case 0x4201:			// Weak(in DS mode)
	if(da_rumble_compat || (rumble_magic[0] == 0x00 && rumble_magic[2] != 0x00 && rumble_magic[3] != 0x00 && rumble_magic[4] != 0x00 && rumble_magic[5] != 0x00))
	 rumble_param[0] = receive_buffer;
	command_phase++;
	break;

  case 0x4202:			// Strong(in DS mode)
	if(da_rumble_compat || rumble_magic[1] == 0x01)
	 rumble_param[1] = receive_buffer;
	command_phase++;	// Nowhere here we come!
	break;

  /************************/
  /* MMMode 1, Command 0x43 */
  /************************/
  case 0x4300:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4301:
	if(receive_buffer == 0x00)
	{
	 //fprintf(stderr, "Mad Munchkins mode left!\n");
	 mad_munchkins = false;
	}
	transmit_buffer[0] = 0x00;
	transmit_buffer[1] = 0x00;
	transmit_buffer[2] = 0x00;
	transmit_buffer[3] = 0x00;
	transmit_buffer[4] = 0x00;
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;

  /************************/
  /* MMMode 1, Command 0x44 */
  /************************/
  case 0x4400:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4401:
	transmit_buffer[0] = 0x00;
	transmit_buffer[1] = 0x00;
	transmit_buffer[2] = 0x00;
	transmit_buffer[3] = 0x00;
	transmit_buffer[4] = 0x00;
	transmit_pos = 0;
	transmit_count = 5;
	command_phase++;

	// Ignores locking state.
	switch(receive_buffer)
	{
	 case 0x00:
		analog_mode = false;
		//fprintf(stderr, "Analog mode disabled\n");
		break;

	 case 0x01:
		analog_mode = true;
		//fprintf(stderr, "Analog mode enabled\n");
		break;
	}
	break;

  case 0x4402:
	switch(receive_buffer)
	{
	 case 0x02:
		analog_mode_locked = false;
		break;

	 case 0x03:
		analog_mode_locked = true;
		break;
	}
	command_phase = -1;
	break;

  /************************/
  /* MMMode 1, Command 0x45 */
  /************************/
  case 0x4500:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0x01; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4501:
	transmit_buffer[0] = 0x02;
	transmit_buffer[1] = analog_mode ? 0x01 : 0x00;
	transmit_buffer[2] = 0x02;
	transmit_buffer[3] = 0x01;
	transmit_buffer[4] = 0x00;
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;


  /************************/
  /* MMMode 1, Command 0x46 */
  /************************/
  case 0x4600:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4601:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0x00;
	 transmit_buffer[1] = 0x01;
	 transmit_buffer[2] = 0x02;
	 transmit_buffer[3] = 0x00;
	 transmit_buffer[4] = 0x0A;
	}
	else if(receive_buffer == 0x01)
	{
	 transmit_buffer[0] = 0x00;
	 transmit_buffer[1] = 0x01;
	 transmit_buffer[2] = 0x01;
	 transmit_buffer[3] = 0x01;
	 transmit_buffer[4] = 0x14;
	}
	else
	{
	 transmit_buffer[0] = 0x00;
	 transmit_buffer[1] = 0x00;
	 transmit_buffer[2] = 0x00;
	 transmit_buffer[3] = 0x00;
	 transmit_buffer[4] = 0x00;
	}
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;

  /************************/
  /* MMMode 1, Command 0x47 */
  /************************/
  case 0x4700:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4701:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0x00;
	 transmit_buffer[1] = 0x02;
	 transmit_buffer[2] = 0x00;
	 transmit_buffer[3] = 0x01;
	 transmit_buffer[4] = 0x00;
	}
	else
	{
	 transmit_buffer[0] = 0x00;
	 transmit_buffer[1] = 0x00;
	 transmit_buffer[2] = 0x00;
	 transmit_buffer[3] = 0x00;
	 transmit_buffer[4] = 0x00;
	}
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;

  /************************/
  /* MMMode 1, Command 0x48 */
  /************************/
  case 0x4800:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4801:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0x00;
	 transmit_buffer[1] = 0x00;
	 transmit_buffer[2] = 0x00;
	 transmit_buffer[3] = 0x01;
	 transmit_buffer[4] = rumble_param[0];
	}
	else if(receive_buffer == 0x01)
	{
	 transmit_buffer[0] = 0x00;
	 transmit_buffer[1] = 0x00;
	 transmit_buffer[2] = 0x00;
	 transmit_buffer[3] = 0x01;
	 transmit_buffer[4] = rumble_param[1];
	}
	else
	{
	 transmit_buffer[0] = 0x00;
	 transmit_buffer[1] = 0x00;
	 transmit_buffer[2] = 0x00;
	 transmit_buffer[3] = 0x00;
	 transmit_buffer[4] = 0x00;
	}
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;

  /************************/
  /* MMMode 1, Command 0x49 */
  /************************/
  case 0x4900:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4901:
	transmit_buffer[0] = 0x00;
	transmit_buffer[1] = 0x00;
	transmit_buffer[2] = 0x00;
	transmit_buffer[3] = 0x00;
	transmit_buffer[4] = 0x00;
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;

  /************************/
  /* MMMode 1, Command 0x4A */
  /************************/
  case 0x4A00:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4A01:
	transmit_buffer[0] = 0x00;
	transmit_buffer[1] = 0x00;
	transmit_buffer[2] = 0x00;
	transmit_buffer[3] = 0x00;
	transmit_buffer[4] = 0x00;
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;

  /************************/
  /* MMMode 1, Command 0x4B */
  /************************/
  case 0x4B00:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4B01:
	transmit_buffer[0] = 0x00;
	transmit_buffer[1] = 0x00;
	transmit_buffer[2] = 0x00;
	transmit_buffer[3] = 0x00;
	transmit_buffer[4] = 0x00;
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;

  /************************/
  /* MMMode 1, Command 0x4C */
  /************************/
  case 0x4C00:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4C01:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0x00;
	 transmit_buffer[1] = 0x00;
	 transmit_buffer[2] = 0x04;
	 transmit_buffer[3] = 0x00;
	 transmit_buffer[4] = 0x00;
	}
	else if(receive_buffer == 0x01)
	{
	 transmit_buffer[0] = 0x00;
	 transmit_buffer[1] = 0x00;
	 transmit_buffer[2] = 0x07;
	 transmit_buffer[3] = 0x00;
	 transmit_buffer[4] = 0x00;
	}
	else
	{
	 transmit_buffer[0] = 0x00;
	 transmit_buffer[1] = 0x00;
	 transmit_buffer[2] = 0x00;
	 transmit_buffer[3] = 0x00;
	 transmit_buffer[4] = 0x00;
	}

	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;

  /************************/
  /* MMMode 1, Command 0x4D */
  /************************/
  case 0x4D00:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = rumble_magic[0]; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4D01:
  case 0x4D02:
  case 0x4D03:
  case 0x4D04:
  case 0x4D05:
  case 0x4D06:
	{
	 unsigned index = command_phase - 0x4D01;

	 if(index < 5)
	 {
 	  transmit_buffer[0] = rumble_magic[1 + index];
	  transmit_pos = 0;
	  transmit_count = 1;
	  command_phase++;
	 }
	 else
	  command_phase = -1;

 	 rumble_magic[index] = receive_buffer;	 
	}
	break;

  /************************/
  /* MMMode 1, Command 0x4E */
  /************************/
  case 0x4E00:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4E01:
	transmit_buffer[0] = 0x00;
	transmit_buffer[1] = 0x00;
	transmit_buffer[2] = 0x00;
	transmit_buffer[3] = 0x00;
	transmit_buffer[4] = 0x00;
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;


  /************************/
  /* MMMode 1, Command 0x4F */
  /************************/
  case 0x4F00:
	if(receive_buffer == 0x00)
	{
	 transmit_buffer[0] = 0; /**/ transmit_pos = 0; transmit_count = 1; /**/
	 command_phase++;
	}
	else
	 command_phase = -1;
	break;

  case 0x4F01:
	transmit_buffer[0] = 0x00;
	transmit_buffer[1] = 0x00;
	transmit_buffer[2] = 0x00;
	transmit_buffer[3] = 0x00;
	transmit_buffer[4] = 0x00;
	transmit_pos = 0;
	transmit_count = 5;
	command_phase = -1;
	break;
  }
 }

 if(!bitpos && transmit_count)
  dsr_pulse_delay = 0x40; //0x100;

 return(ret);
}

InputDevice *Device_DualShock_Create(const std::string &name)
{
 return new InputDevice_DualShock(name);
}


InputDeviceInputInfoStruct Device_DualShock_IDII[26] =
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

 { "analog", "Analog(mode toggle)", 24, IDIT_BUTTON, NULL },

 { "rstick_right", "Right Stick RIGHT →", 22, IDIT_BUTTON_ANALOG },
 { "rstick_left", "Right Stick LEFT ←", 21, IDIT_BUTTON_ANALOG },
 { "rstick_down", "Right Stick DOWN ↓", 20, IDIT_BUTTON_ANALOG },
 { "rstick_up", "Right Stick UP ↑", 19, IDIT_BUTTON_ANALOG },

 { "lstick_right", "Left Stick RIGHT →", 17, IDIT_BUTTON_ANALOG },
 { "lstick_left", "Left Stick LEFT ←", 16, IDIT_BUTTON_ANALOG },
 { "lstick_down", "Left Stick DOWN ↓", 15, IDIT_BUTTON_ANALOG },
 { "lstick_up", "Left Stick UP ↑", 14, IDIT_BUTTON_ANALOG },

 { "rumble", "RUMBLE MONSTER RUMBA", 100, IDIT_RUMBLE },
};

}
