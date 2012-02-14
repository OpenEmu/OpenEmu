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

#include <stdio.h>
#include <string.h>

#include <wiiuse/wpad.h>

#include "wii_input.h"
#include "wii_main.h"

#include "wii_app_common.h"

#ifdef WII_NETTRACE
#include <network.h>
#include "net_print.h"  
#endif

// The base application path
static char app_path[WII_MAX_PATH] = "";

// The most recent status message
char wii_status_message[WII_MENU_BUFF_SIZE] = "";

// The status message display count down
u8 wii_status_message_count = 0;

// Whether we are installed on a USB drive
BOOL wii_is_usb = FALSE;

/*
 * Updates the specified result string with the location of the file relative
 * to the appliation root directory.
 * 
 * file     The file name
 * result   The buffer to store the result in
 */
void wii_get_app_relative( const char *file, char *result )
{
  snprintf( result, WII_MAX_PATH, "%s%s", app_path, file );
}

/*
 * Returns the base application path
 *
 * return   The base application path
 */
char* wii_get_app_path()
{
  return app_path;
}

/*
 * Determines and stores the base application path
 *
 * argc     The count of main arguments
 * argv     The array of argument values
 */
void wii_set_app_path( int argc, char *argv[] )
{    
  snprintf( app_path, WII_MAX_PATH, "%s", WII_BASE_APP_DIR );

  if( ( argc > 0 ) && 
      ( argv[0] != NULL ) && 
      ( strchr( argv[0], ':' ) != NULL ) ) // To support wiiload
  {
#ifdef WII_NETTRACE
    net_print_string(__FILE__,__LINE__, "Found drive prefix\n" );  
#endif

    char temp_path[WII_MAX_PATH];
    snprintf( temp_path, WII_MAX_PATH, "%s", argv[0] );

    char *loc;

    // Remove the file name
    loc = strrchr( temp_path, '/' );
    if (loc != NULL)
    {
      *loc = '\0'; 
    }

    // Convert the prefix to lowercase (consistent with our mapped drive)
    for( loc = temp_path; *loc && *loc != ':'; ++loc )
    {
      *loc = tolower( *loc );
    }

    snprintf( app_path, WII_MAX_PATH, "%s/", temp_path );
  }
  else
  {
#ifdef WII_NETTRACE
    net_print_string(__FILE__,__LINE__, "No drive prefix." );  
#endif
  }

  if( strncmpi( "usb", app_path, 3 ) == 0 )
  {
    wii_is_usb = TRUE;
  }
}

/*
 * Returns the file system prefix
 *
 * return   The file system prefix
 */
char* wii_get_fs_prefix()
{
  return ( wii_is_usb ? "usb:" : "sd:" );
}

/*
 * Pause and wait for input. Usually used when debugging.
 */
void wii_pause()
{
  int done = 0;
  while( !done )
  {		
    WPAD_ScanPads();
    PAD_ScanPads();

    expansion_t exp;
    WPAD_Expansion( 0, &exp );        
    bool isClassic = ( exp.type == WPAD_EXP_CLASSIC );

    u32 down = WPAD_ButtonsDown(0);
    u32 gcDown = PAD_ButtonsDown(0);

    if( ( down & ( WII_BUTTON_A | 
          ( isClassic ? 
            WII_CLASSIC_BUTTON_A : WII_NUNCHUK_BUTTON_A ) ) ) || 
        ( gcDown & GC_BUTTON_A ) )
    {	
      done = 1;
    }
  }
}

/*
 * Writes the specified string to the VT display
 */
void wii_write_vt( char *buffer )
{
  printf( buffer );
}

/*
 * Initializes the console output to the specified frame buffer. This 
 * allows us to swap between console output buffers.
 *
 * fb       The frame buffer to associated with the console
 */
void wii_console_init( void *fb )
{
  console_init(
    fb, 20, 20,
    vmode->fbWidth,
    vmode->xfbHeight,
    vmode->fbWidth * VI_DISPLAY_PIX_SZ );
}

/*
 * Stores a status message for display in the footer of the menu
 */ 
void wii_set_status_message( const char *message )
{
  wii_status_message_count = 3;
  snprintf( wii_status_message, sizeof(wii_status_message), "%s", message );
}
