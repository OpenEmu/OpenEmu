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

#include "wii_app.h"
#include "wii_config.h"
#include "wii_input.h"
#include "wii_sdl.h"

#include "wii_handy.h"
#include "wii_handy_main.h"
#include "wii_handy_menu.h"

#include "FreeTypeGX.h"

#include "font_ttf.h"

extern "C" {
  void WII_VideoStop();
}

// The last cartridge hash
char wii_cartridge_hash[33];
// The cartridge hash with header (may be the same)
char wii_cartridge_hash_with_header[33];
// The database entry for current game
HandyDBEntry wii_handy_db_entry;
// Whether to display debug info (FPS, etc.)
BOOL wii_debug = FALSE;
// Auto save state?
BOOL wii_auto_save_state = FALSE;
// Auto load state?
BOOL wii_auto_load_state = TRUE;
// The screen X size
int wii_screen_x = DEFAULT_SCREEN_X;
// The screen Y size
int wii_screen_y = DEFAULT_SCREEN_Y;
// The screen X size (vert)
int wii_screen_x_vert = DEFAULT_SCREEN_X_VERT;
// The screen Y size (vert)
int wii_screen_y_vert = DEFAULT_SCREEN_Y_VERT;
// Maximum frame rate
u8 wii_max_frames = HANDY_FPS;

/*
 * Initializes the application
 */
void wii_handle_init()
{  
  // Read the config values
  wii_read_config();

  // Startup the SDL
  if( !wii_sdl_init() ) 
  {
    fprintf( stderr, "FAILED : Unable to init SDL: %s\n", SDL_GetError() );
    exit( EXIT_FAILURE );
  }

  // FreeTypeGX
  InitFreeType( (uint8_t*)font_ttf, (FT_Long)font_ttf_size  );

  // Initializes the menu
  wii_handy_menu_init();

  // Initializes handy
  wii_handy_init();
}

/*
 * Frees resources prior to the application exiting
 */
void wii_handle_free_resources()
{
  // We be done, write the config settings, free resources and exit
  wii_write_config();

  wii_sdl_free_resources();
  SDL_Quit();
}

/*
 * Runs the application (main loop)
 */
void wii_handle_run()
{
  WII_VideoStop();

  // Show the menu
  wii_menu_show();
}

// The roms dir
static char roms_dir[WII_MAX_PATH] = "";

/*
 * Returns the roms directory
 *
 * return   The roms directory
 */
char* wii_get_roms_dir()
{
  if( roms_dir[0] == '\0' )
  {
    snprintf( 
      roms_dir, WII_MAX_PATH, "%s%s", wii_get_fs_prefix(), WII_ROMS_DIR );
  }

  return roms_dir;
}

// The saves dir
static char saves_dir[WII_MAX_PATH] = "";

/*
 * Returns the saves directory
 *
 * return   The saves directory
 */
char* wii_get_saves_dir()
{
  if( saves_dir[0] == '\0' )
  {
    snprintf( 
      saves_dir, WII_MAX_PATH, "%s%s", wii_get_fs_prefix(), WII_SAVES_DIR );
  }

  return saves_dir;
}

/*
 * Returns the current orientation
 *
 * return The current orientation
 */
extern int wii_handy_orientation()
{
  return 
    ( wii_handy_db_entry.orient == MIKIE_BAD_MODE ?  
        ( mpLynx->CartGetRotate() + 1 ) : wii_handy_db_entry.orient );
}

/*
 * Returns the (current) max frame rate
 *
 * return The (current) max frame rate
 */
extern u8 wii_get_max_frames()
{
  return 
    ( wii_handy_db_entry.maxFrames == MAX_FRAMES_DEFAULT ?
        wii_max_frames : wii_handy_db_entry.maxFrames );
}