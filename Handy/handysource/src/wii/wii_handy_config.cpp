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

#include <stdio.h>

#include "wii_util.h"

#include "wii_handy.h"

/*
 * Handles reading a particular configuration value
 *
 * name   The name of the config value
 * value  The config value
 */
extern "C" void wii_config_handle_read_value( char *name, char* value )
{
  if( strcmp( name, "DEBUG" ) == 0 )
  {
    wii_debug = Util_sscandec( value );				
  }
  else if( strcmp( name, "TOP_MENU_EXIT" ) == 0 )
  {
    wii_top_menu_exit = Util_sscandec( value );				
  }
  else if( strcmp( name, "AUTO_LOAD_STATE" ) == 0 )
  {
    wii_auto_load_state = Util_sscandec( value );				
  }
  else if( strcmp( name, "AUTO_SAVE_STATE" ) == 0 )
  {
    wii_auto_save_state = Util_sscandec( value );				
  }
  else if( strcmp( name, "VSYNC" ) == 0 )
  {
    wii_vsync = Util_sscandec( value );
  }
  else if( strcmp( name, "MAX_FRAMES" ) == 0 )
  {
    wii_max_frames = Util_sscandec( value );
  }
  else if( strcmp( name, "SEL_OFFSET" ) == 0 )
  {
    wii_menu_sel_offset = Util_sscandec( value );
  }
  else if( strcmp( name, "SEL_COLOR" ) == 0 )
  {
    Util_hextorgba( value, &wii_menu_sel_color );
  }
  else if( strcmp( name, "MOTE_MENU_VERTICAL" ) == 0 )
  {
    wii_mote_menu_vertical = Util_sscandec( value );				
  }
  else if( strcmp( name, "SCREEN_X" ) == 0 )
  {
    wii_screen_x = Util_sscandec( value );
  }
  else if( strcmp( name, "SCREEN_Y" ) == 0 )
  {
    wii_screen_y = Util_sscandec( value );
  }
  else if( strcmp( name, "SCREEN_X_VERT" ) == 0 )
  {
    wii_screen_x_vert = Util_sscandec( value );
  }
  else if( strcmp( name, "SCREEN_Y_VERT" ) == 0 )
  {
    wii_screen_y_vert = Util_sscandec( value );
  }
}

/*
 * Handles the writing of the configuration file
 *
 * fp   The file pointer
 */
extern "C" void wii_config_handle_write_config( FILE *fp )
{
  fprintf( fp, "DEBUG=%d\n", wii_debug );
  fprintf( fp, "TOP_MENU_EXIT=%d\n", wii_top_menu_exit );
  fprintf( fp, "AUTO_LOAD_STATE=%d\n", wii_auto_load_state );
  fprintf( fp, "AUTO_SAVE_STATE=%d\n", wii_auto_save_state );
  fprintf( fp, "VSYNC=%d\n", wii_vsync );
  fprintf( fp, "MAX_FRAMES=%d\n", wii_max_frames );  
  fprintf( fp, "SEL_OFFSET=%d\n", wii_menu_sel_offset );  
  fprintf( fp, "MOTE_MENU_VERTICAL=%d\n", wii_mote_menu_vertical );  
  fprintf( fp, "SCREEN_X=%d\n", wii_screen_x );
  fprintf( fp, "SCREEN_Y=%d\n", wii_screen_y );
  fprintf( fp, "SCREEN_X_VERT=%d\n", wii_screen_x_vert );
  fprintf( fp, "SCREEN_Y_VERT=%d\n", wii_screen_y_vert );
}
