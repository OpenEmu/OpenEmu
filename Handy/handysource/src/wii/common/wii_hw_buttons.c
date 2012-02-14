/*
Copyright (C) 2010
raz0red (www.twitchasylum.com)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.
*/

#include <wiiuse/wpad.h>

// Hardware buttons (reset, power, etc.)
u8 wii_hw_button = 0;

/*
 * Callback for the reset button on the Wii.
 */
void wii_reset_pressed()
{
  wii_hw_button = SYS_RETURNTOMENU;
}

/*
 * Callback for the power button on the Wii.
 */
void wii_power_pressed()
{
  wii_hw_button = SYS_POWEROFF_STANDBY;
}

/*
 * Callback for the power button on the Wiimote.
 *
 * chan The Wiimote that pressed the button
 */
void wii_mote_power_pressed(s32 chan)
{
  wii_hw_button = SYS_POWEROFF_STANDBY;
} 

/*
 * Registers the hardware button callbacks 
 */
void wii_register_hw_buttons()
{
  SYS_SetResetCallback( wii_reset_pressed );
  SYS_SetPowerCallback( wii_power_pressed );
  WPAD_SetPowerButtonCallback( wii_mote_power_pressed );
}
