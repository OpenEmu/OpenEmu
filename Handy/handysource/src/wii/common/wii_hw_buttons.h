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

#ifndef WII_HW_BUTTONS_H
#define WII_HW_BUTTONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gctypes.h>

// Hardware buttons (reset, power, etc.)
extern u8 wii_hw_button;

/*
 * Callback for the reset button on the Wii.
 */
extern void wii_reset_pressed();
 
/*
 * Callback for the power button on the Wii.
 */
extern void wii_power_pressed();
 
/*
 * Callback for the power button on the Wiimote.
 *
 * chan The Wiimote that pressed the button
 */
extern void wii_mote_power_pressed(s32 chan);

/*
 * Registers the hardware button callbacks 
 */
extern void wii_register_hw_buttons();

#ifdef __cplusplus
}
#endif

#endif
