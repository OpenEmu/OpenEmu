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

#include "wii_app.h"
#include "wii_snapshot.h"
#include "wii_util.h"

/*
 * Deletes the snapshot for the current rom
 */
void wii_delete_snapshot()
{
  if( wii_last_rom != NULL )
  {
    char filename[WII_MAX_PATH];     
    wii_snapshot_handle_get_name( wii_last_rom, filename );       

    int status = remove( filename );

    if( !status )
    {
      wii_menu_reset_indexes();
      wii_menu_move( wii_menu_stack[wii_menu_stack_head], 1 );
    }

    wii_set_status_message( 
      !status ?
        "Successly deleted saved state." :
        "An error occurred attempting to delete saved state."  
    );
  }
}

/*
 * Saves the current games state to the specified save file
 *
 * savefile The name of the save file to write state to. If this value is NULL,
 *          the default save name for the last rom is used.
 */
void wii_save_snapshot( const char *savefile, BOOL status_update )
{    
  bool succeeded = false;

  char filename[WII_MAX_PATH];
  filename[0] = '\0';

  if( savefile != NULL )
  {
    Util_strlcpy( filename, savefile, WII_MAX_PATH );
  }
  else if( wii_last_rom != NULL )
  {                        
    wii_snapshot_handle_get_name( wii_last_rom, filename );
  }    

  if( strlen( filename ) != 0 )
  {
    // For some reason sometimes the save file seems to use a corrupted name
    // even though the string we are passing it looks correct. This may be
    // a libogc issue.... So, we retry until it works (or fail after 5 times)
    int i;
    for( i = 0; i < 5; i++ )
    {
      remove( filename );
      succeeded = wii_snapshot_handle_save( filename );   

      if( wii_check_snapshot( filename ) == 0 )
      {
        break;
      }
      else
      {
        succeeded = FALSE;
      }
    }
  }

  if( status_update )
  {
    wii_set_status_message( 
      succeeded ?
        "Successly saved state." :
        "An error occurred attempting to save state." 
    );
  }
}

/*
 * Determines whether the specified snapshot is valid
 *
 * return   0  = valid
 *          -1 = does not exist
 *          -2 = not a valid save (wrong size)
 */
int wii_check_snapshot( const char *savefile )
{
  struct stat statbuf;
  int result = stat( savefile, &statbuf );
  return result < 0 ? -1 : 0;
}
