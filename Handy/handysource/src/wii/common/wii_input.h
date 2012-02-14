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

#ifndef WII_INPUT_H
#define WII_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <wiiuse/wpad.h>

// UI Navigation, Standard Buttons
#define WII_BUTTON_LEFT ( WPAD_BUTTON_LEFT )
#define WII_CLASSIC_BUTTON_LEFT ( WPAD_CLASSIC_BUTTON_LEFT )
#define GC_BUTTON_LEFT ( PAD_BUTTON_LEFT )
#define WII_BUTTON_RIGHT ( WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT )
#define GC_BUTTON_RIGHT ( PAD_BUTTON_RIGHT )
#define WII_BUTTON_UP ( WPAD_BUTTON_UP  )
#define WII_CLASSIC_BUTTON_UP ( WPAD_CLASSIC_BUTTON_UP  )
#define GC_BUTTON_UP ( PAD_BUTTON_UP )
#define WII_BUTTON_DOWN ( WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN  )
#define GC_BUTTON_DOWN ( PAD_BUTTON_DOWN )
#define WII_BUTTON_ENTER ( WPAD_BUTTON_A | WPAD_BUTTON_2 )
#define WII_CLASSIC_BUTTON_ENTER ( WPAD_CLASSIC_BUTTON_A )
#define WII_NUNCHUK_BUTTON_ENTER ( WPAD_NUNCHUK_BUTTON_C )
#define GC_BUTTON_ENTER ( PAD_BUTTON_A )
#define WII_BUTTON_ESC ( WPAD_BUTTON_B | WPAD_BUTTON_1 )
#define WII_CLASSIC_BUTTON_ESC ( WPAD_CLASSIC_BUTTON_B )
#define WII_NUNCHUK_BUTTON_ESC ( WPAD_NUNCHUK_BUTTON_Z )
#define GC_BUTTON_ESC ( PAD_BUTTON_B )
#define WII_BUTTON_HOME ( WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME )
#define GC_BUTTON_HOME ( PAD_TRIGGER_Z )

// General
#define WII_BUTTON_A WII_BUTTON_ENTER
#define WII_CLASSIC_BUTTON_A WII_CLASSIC_BUTTON_ENTER
#define WII_NUNCHUK_BUTTON_A WII_NUNCHUK_BUTTON_ENTER
#define GC_BUTTON_A GC_BUTTON_ENTER
#define WII_BUTTON_B WII_BUTTON_ESC
#define WII_CLASSIC_BUTTON_B WII_CLASSIC_BUTTON_ESC
#define WII_NUNCHUK_BUTTON_B WII_NUNCHUK_BUTTON_ESC
#define GC_BUTTON_B GC_BUTTON_ESC

/*
 * Returns the analog value for the joystick (-128.0 to 128.0)
 * 
 * exp      The expansion (Nunchuk or Classic)
 * isX      Whether to get the X or Y axis value 
 * isRjs    Whether we are reading for the right joystick
 * return   The analog value for the joystick (-128.0 to 128.0)
 */
extern float wii_exp_analog_val( const expansion_t* exp, BOOL isX, BOOL isRjs );

/*
 * Returns the analog value for the joystick
 * 
 * exp      The expansion (Nunchuk or Classic)
 * isX      Whether to get the X or Y axis value 
 * isRjs    Whether we are reading for the right joystick
 * range    The analog range (+/-)
 * return   The analog value for the joystick
 */
extern float wii_exp_analog_val_range( 
  const expansion_t* exp, BOOL isX, BOOL isRjs, float range );

extern inline BOOL wii_analog_right( float expX, s8 gcX ) { return expX > 60 || gcX > 46; }
extern inline BOOL wii_analog_left( float expX, s8 gcX ) { return expX < -60 || gcX < -46; }
extern inline BOOL wii_analog_up( float expY, s8 gcY ) { return expY > 70 || gcY > 54; }
extern inline BOOL wii_analog_down( float expY, s8 gcY ) { return expY < -70 || gcY < -54; }

/*
 * Whether the right digital pad is pressed
 *
 * wmHorizontal   Whether the Wiimote is horizontal or vertical
 * classic        Whether to check the classic controller
 * held           The current held state
 * return         Whether the right digital pad is pressed
 */
extern BOOL wii_digital_right( BOOL wmHorizontal, BOOL classic, u32 held );

/*
 * Whether the left digital pad is pressed
 *
 * wmHorizontal   Whether the Wiimote is horizontal or vertical
 * classic        Whether to check the classic controller
 * held           The current held state
 * return         Whether the left digital pad is pressed
 */
extern BOOL wii_digital_left( BOOL wmHorizontal, BOOL classic, u32 held );

/*
 * Whether the up digital pad is pressed
 *
 * wmHorizontal   Whether the Wiimote is horizontal or vertical
 * classic        Whether to check the classic controller
 * held           The current held state
 * return         Whether the up digital pad is pressed
 */
extern BOOL wii_digital_up( BOOL wmHorizontal, BOOL classic, u32 held );

/*
 * Whether the down digital pad is pressed
 *
 * wmHorizontal   Whether the Wiimote is horizontal or vertical
 * classic        Whether to check the classic controller
 * held           The current held state
 * return         Whether the down digital pad is pressed
 */
extern BOOL wii_digital_down( BOOL wmHorizontal, BOOL classic, u32 held );

/*
 * Waits until no buttons are pressed
 *
 * joys   The number of joysticks
 */
extern void wii_wait_until_no_buttons( int joys );

/*
 * Checks to see if any buttons are held
 *
 * joys   The number of joysticks to check
 * return Whether any of the buttons are held
 */
extern BOOL wii_is_any_button_held( int joys );

#ifdef __cplusplus
}
#endif

#endif
