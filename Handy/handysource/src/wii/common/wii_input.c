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

#include <math.h>

#include "wii_input.h"

#define PI 3.14159265f

/*
 * Returns the analog value for the joystick (-128.0 to 128.0)
 * 
 * exp      The expansion (Nunchuk or Classic)
 * isX      Whether to get the X or Y axis value 
 * isRjs    Whether we are reading for the right joystick
 * return   The analog value for the joystick (-128.0 to 128.0)
 */
float wii_exp_analog_val( const expansion_t* exp, BOOL isX, BOOL isRjs )
{
  return wii_exp_analog_val_range( exp, isX, isRjs, 128.0f );
}

/*
 * Returns the analog value for the joystick
 * 
 * exp      The expansion (Nunchuk or Classic)
 * isX      Whether to get the X or Y axis value 
 * isRjs    Whether we are reading for the right joystick
 * range    The analog range (+/-)
 * return   The analog value for the joystick
 */
float wii_exp_analog_val_range( 
  const expansion_t* exp, BOOL isX, BOOL isRjs, float range )
{
  float mag = 0.0;
  float ang = 0.0;

  if( exp->type == WPAD_EXP_CLASSIC )
  {
    if( isRjs )
    {
      mag = exp->classic.rjs.mag;
      ang = exp->classic.rjs.ang;
    }
    else
    {
      mag = exp->classic.ljs.mag;
      ang = exp->classic.ljs.ang;
    }
  }
  else if ( exp->type == WPAD_EXP_NUNCHUK )
  {
    mag = exp->nunchuk.js.mag;
    ang = exp->nunchuk.js.ang;
  }
  else
  {
    return 0.0;
  }

  if( mag > 1.0 ) mag = 1.0;
  else if( mag < -1.0 ) mag = -1.0;
  double val = 
    ( isX ? 
      mag * sin( PI * ang / 180.0f ) :
      mag * cos( PI * ang / 180.0f ) );

  if( isnan( val ) )
  {
    return 0.0f;
  }

  return val * range;
}

/*
 * Whether the right digital pad is pressed
 *
 * wmHorizontal   Whether the Wiimote is horizontal or vertical
 * classic        Whether to check the classic controller
 * held           The current held state
 * return         Whether the right digital pad is pressed
 */
BOOL wii_digital_right( BOOL wmHorizontal, BOOL classic, u32 held )
{
  return 
    ( held &
      ( ( wmHorizontal ? WPAD_BUTTON_DOWN : WPAD_BUTTON_RIGHT ) | 
        ( classic ? WPAD_CLASSIC_BUTTON_RIGHT : 0 ) ) );
}

/*
 * Whether the left digital pad is pressed
 *
 * wmHorizontal   Whether the Wiimote is horizontal or vertical
 * classic        Whether to check the classic controller
 * held           The current held state
 * return         Whether the left digital pad is pressed
 */
BOOL wii_digital_left( BOOL wmHorizontal, BOOL classic, u32 held )
{
  return 
    ( held &
      ( ( wmHorizontal ? WPAD_BUTTON_UP : WPAD_BUTTON_LEFT ) | 
        ( classic ? WPAD_CLASSIC_BUTTON_LEFT : 0 ) ) );
}

/*
 * Whether the up digital pad is pressed
 *
 * wmHorizontal   Whether the Wiimote is horizontal or vertical
 * classic        Whether to check the classic controller
 * held           The current held state
 * return         Whether the up digital pad is pressed
 */
BOOL wii_digital_up( BOOL wmHorizontal, BOOL classic, u32 held )
{
  return 
    ( held &
      ( ( wmHorizontal ? WPAD_BUTTON_RIGHT : WPAD_BUTTON_UP ) | 
        ( classic ? WPAD_CLASSIC_BUTTON_UP : 0 ) ) );
}

/*
 * Whether the down digital pad is pressed
 *
 * wmHorizontal   Whether the Wiimote is horizontal or vertical
 * classic        Whether to check the classic controller
 * held           The current held state
 * return         Whether the down digital pad is pressed
 */
BOOL wii_digital_down( BOOL wmHorizontal, BOOL classic, u32 held )
{
  return 
    ( held &
      ( ( wmHorizontal ? WPAD_BUTTON_LEFT : WPAD_BUTTON_DOWN ) | 
        ( classic ? WPAD_CLASSIC_BUTTON_DOWN : 0 ) ) );
}

/*
 * Waits until no buttons are pressed
 *
 * joys   The number of joysticks
 */
void wii_wait_until_no_buttons( int joys )
{
  while( 1 )
  {  
    WPAD_ScanPads(); PAD_ScanPads();     
    if( !wii_is_any_button_held( joys ) )
    {
      break;
    }
    VIDEO_WaitVSync();
  }
}

/*
 * Checks to see if any buttons are held
 *
 * joys   The number of joysticks to check
 * return Whether any of the buttons are held
 */
BOOL wii_is_any_button_held( int joys )
{
  int i;
  for( i = 0; i < joys; i++ )
  {
    if( WPAD_ButtonsHeld( i ) || PAD_ButtonsHeld( i ) )
    {
      return TRUE;
    }
  }

  return FALSE;
}

