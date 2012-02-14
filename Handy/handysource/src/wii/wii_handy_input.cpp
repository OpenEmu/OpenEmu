/*
WiiHandy : Port of the Handy Emulator for the Wii

Copyright (C) 2011
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

#include "system.h"

#include "wii_input.h"

#include "wii_handy.h"
#include "wii_handy_main.h"

/*
 * Updates the controls
 */
void wii_handy_update_controls()
{
  // Initialise Handy button events
  int OldKeyMask, KeyMask = mpLynx->GetButtonData();
  OldKeyMask = KeyMask;

  // Getting events for keyboard and/or joypad handling
  WPAD_ScanPads();
  PAD_ScanPads();

  // Check the state of the controllers
  u32 pressed = WPAD_ButtonsDown( 0 );
  u32 held = WPAD_ButtonsHeld( 0 );
  u32 gcPressed = PAD_ButtonsDown( 0 );
  u32 gcHeld = PAD_ButtonsHeld( 0 );

  // Analog controls
  expansion_t exp;
  WPAD_Expansion( 0, &exp );        
  float expX = wii_exp_analog_val( &exp, TRUE, FALSE );
  float expY = wii_exp_analog_val( &exp, FALSE, FALSE );
  s8 gcX = PAD_StickX( 0 );
  s8 gcY = PAD_StickY( 0 );

  // Classic or Nunchuck?
  BOOL isClassic = ( exp.type == WPAD_EXP_CLASSIC );

  if( ( pressed & WII_BUTTON_HOME ) ||
    ( gcPressed & GC_BUTTON_HOME ) ||
    wii_hw_button )
  {
    emulationRunning = 0;
  }

  if( ( held & WII_BUTTON_LYNX_PAUSE ) ||
    ( gcHeld & GC_BUTTON_LYNX_PAUSE ) )
    KeyMask|=BUTTON_PAUSE;
  else
    KeyMask&=~BUTTON_PAUSE;

  if( held & ( WII_BUTTON_LYNX_A | 
      ( isClassic ? WII_CLASSIC_LYNX_A : WII_NUNCHUK_LYNX_A ) ) || 
      gcHeld & GC_BUTTON_LYNX_A )
    KeyMask|=BUTTON_A;
  else
    KeyMask&=~BUTTON_A;

  if( held & ( WII_BUTTON_LYNX_B | 
      ( isClassic ? WII_CLASSIC_LYNX_B : WII_NUNCHUK_LYNX_B ) ) || 
      gcHeld & GC_BUTTON_LYNX_B )
    KeyMask|=BUTTON_B;
  else
    KeyMask&=~BUTTON_B;

  if( ( held & WII_BUTTON_LYNX_OPT1 ) ||
    ( gcHeld & GC_BUTTON_LYNX_OPT1 ) )
    KeyMask|=BUTTON_OPT1;
  else
    KeyMask&=~BUTTON_OPT1;

  if( ( held & WII_BUTTON_LYNX_OPT2 ) ||
    ( gcHeld & GC_BUTTON_LYNX_OPT2 ) )
    KeyMask|=BUTTON_OPT2;
  else
    KeyMask&=~BUTTON_OPT2;

  int orient = wii_handy_orientation();

  int left, right, up, down;
  switch( orient )
  {
    case MIKIE_NO_ROTATE:
      up = BUTTON_UP;
      left = BUTTON_LEFT;
      down = BUTTON_DOWN;
      right = BUTTON_RIGHT;
      break;
    case MIKIE_ROTATE_L:
      up = BUTTON_LEFT;
      left = BUTTON_DOWN;
      down = BUTTON_RIGHT;
      right = BUTTON_UP;
      break;
    case MIKIE_ROTATE_R:
      up = BUTTON_RIGHT;
      left = BUTTON_UP;
      down = BUTTON_LEFT;
      right = BUTTON_DOWN;
  }

  if( wii_digital_right( TRUE, isClassic, held ) ||
      ( gcHeld & GC_BUTTON_RIGHT ) ||
      wii_analog_right( expX, gcX ) )
    KeyMask|=right;
  else
    KeyMask&=~right;

  if( wii_digital_left( TRUE, isClassic, held ) || 
      ( gcHeld & GC_BUTTON_LEFT ) ||                       
      wii_analog_left( expX, gcX ) )
    KeyMask|=left;
  else
    KeyMask&=~left;

  if( wii_digital_up( TRUE, isClassic, held ) || 
      ( gcHeld & GC_BUTTON_UP ) ||
    wii_analog_up( expY, gcY ) )
    KeyMask|=up;
  else
    KeyMask&=~up;

  if( wii_digital_down( TRUE, isClassic, held ) ||
      ( gcHeld & GC_BUTTON_DOWN ) ||
      wii_analog_down( expY, gcY ) )
    KeyMask|=down;
  else
    KeyMask&=~down;

  if (OldKeyMask != KeyMask)
    mpLynx->SetButtonData(KeyMask);
}
