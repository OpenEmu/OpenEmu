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

#include "wii_app.h"
#include "wii_util.h"

#include "wii_handy.h"
#include "wii_handy_emulation.h"
#include "wii_handy_main.h"

/*
 * Determines the save name for the specified rom file
 *
 * romfile  The name of the rom file
 * buffer   The buffer to write the save name to
 */
extern "C" void wii_snapshot_handle_get_name( 
  const char *romfile, char *buffer )
{
  char filename[WII_MAX_PATH];            
  Util_splitpath( romfile, NULL, filename );
  snprintf( buffer, WII_MAX_PATH, "%s%s.%s",  
    wii_get_saves_dir(), filename, WII_SAVE_GAME_EXT );
}

/*
 * Saves with the specified save name
 *
 * filename   The name of the save file
 * return     Whether the save was successful
 */
extern "C" BOOL wii_snapshot_handle_save( char* filename )
{
  return mpLynx->ContextSave( filename ) ? TRUE : FALSE;  
}

/*
 * Starts the emulator for the specified snapshot file.
 *
 * savefile The name of the save file to load. 
 */
BOOL wii_start_snapshot( char *savefile )
{
  BOOL succeeded = FALSE;
  BOOL seterror = FALSE;

  // Determine the extension
  char ext[WII_MAX_PATH];
  Util_getextension( savefile, ext );

  if( !strcmp( ext, WII_SAVE_GAME_EXT ) )
  {
    char savename[WII_MAX_PATH];

    // Get the save name (without extension)
    Util_splitpath( savefile, NULL, savename );

    int namelen = strlen( savename );
    int extlen = strlen( WII_SAVE_GAME_EXT );

    if( namelen > extlen )
    {
      // build the associated rom name
      savename[namelen - extlen - 1] = '\0';

      char romfile[WII_MAX_PATH];
      snprintf( romfile, WII_MAX_PATH, "%s%s", wii_get_roms_dir(), savename );

      int exists = Util_fileexists( romfile );

      // Ensure the rom exists
      if( !exists )            
      {
        wii_set_status_message(
          "Unable to find associated ROM file." );                
        seterror = TRUE;
      }
      else
      {
        // launch the emulator for the save
        wii_start_emulation( romfile, savefile );
        succeeded = TRUE;
      }
    }
  }

  if( !succeeded && !seterror )
  {
    wii_set_status_message( 
      "The file selected is not a valid saved state file." );    
  }

  return succeeded;
}