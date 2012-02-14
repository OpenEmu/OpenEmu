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

#ifndef WII_HANDY_H
#define WII_HANDY_H

#include <wiiuse/wpad.h>
#include "wii_main.h"

#include "wii_handy_db.h"

// FPS
#define HANDY_FPS 60

// Handy size
#define HANDY_WIDTH 160
#define HANDY_HEIGHT 102

// Wii width and height
#define WII_WIDTH 640
#define WII_HEIGHT 480

// Default screen size
#define DEFAULT_SCREEN_X ((int)(WII_WIDTH*1.9))
#define DEFAULT_SCREEN_Y ((int)(WII_HEIGHT*1.9))

#define DEFAULT_SCREEN_X_VERT ((int)(WII_WIDTH*1.34)) 
#define DEFAULT_SCREEN_Y_VERT ((int)(WII_HEIGHT*1.34)) 

#define WII_BUTTON_LYNX_OPT1 ( WPAD_BUTTON_PLUS | WPAD_CLASSIC_BUTTON_PLUS )
#define GC_BUTTON_LYNX_OPT1 ( PAD_TRIGGER_R )
#define WII_BUTTON_LYNX_OPT2 ( WPAD_BUTTON_MINUS | WPAD_CLASSIC_BUTTON_MINUS )
#define GC_BUTTON_LYNX_OPT2 ( PAD_TRIGGER_L )
#define WII_BUTTON_LYNX_PAUSE ( WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_FULL_L | WPAD_CLASSIC_BUTTON_FULL_R )
#define GC_BUTTON_LYNX_PAUSE ( PAD_BUTTON_START )
#define WII_BUTTON_LYNX_A ( WPAD_BUTTON_2 )
#define WII_CLASSIC_LYNX_A ( WPAD_CLASSIC_BUTTON_A )
#define WII_NUNCHUK_LYNX_A ( WPAD_NUNCHUK_BUTTON_C )
#define GC_BUTTON_LYNX_A ( PAD_BUTTON_A )
#define WII_BUTTON_LYNX_B ( WPAD_BUTTON_1 ) 
#define WII_CLASSIC_LYNX_B ( WPAD_CLASSIC_BUTTON_B )
#define WII_NUNCHUK_LYNX_B ( WPAD_NUNCHUK_BUTTON_Z )
#define GC_BUTTON_LYNX_B ( PAD_BUTTON_B )

// The last cartridge hash
extern char wii_cartridge_hash[33];
// The cartridge hash with header (may be the same)
extern char wii_cartridge_hash_with_header[33];
// The database entry for current game
extern HandyDBEntry wii_handy_db_entry;
// Whether to display debug info (FPS, etc.)
extern BOOL wii_debug;
// Hardware buttons (reset, power, etc.)
extern u8 wii_hw_button;
// Auto load state?
extern BOOL wii_auto_load_state;
// Auto save state?
extern BOOL wii_auto_save_state;
// The screen X size
extern int wii_screen_x;
// The screen Y size
extern int wii_screen_y;
// The screen X size (vert)
extern int wii_screen_x_vert;
// The screen Y size (vert)
extern int wii_screen_y_vert;
// Maximum frame rate
extern u8 wii_max_frames;


/*
 * Returns the roms directory
 *
 * return   The roms directory
 */
extern char* wii_get_roms_dir();

/*
 * Returns the saves directory
 *
 * return   The saves directory
 */
extern char* wii_get_saves_dir();

/*
 * Returns the current orientation
 *
 * return The current orientation
 */
extern int wii_handy_orientation();

#endif
