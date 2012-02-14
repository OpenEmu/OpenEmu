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

#ifndef WII_HANDY_DB
#define WII_HANDY_DB

#include <gctypes.h>

#include "wii_util.h"

// Max frames default
#define MAX_FRAMES_DEFAULT      0

/*
 * Handy database entry 
 */
typedef struct HandyDBEntry
{
  char name[255];       // The name of the game
  u8 orient;            // The orientation
  s32 maxFrames;        // Maximum frames
  u8 loaded;            // Whether the settings were loaded 
} HandyDBEntry;

/*
 * Returns the database entry for the game with the specified hash
 *
 * hash	  The hash of the game
 * entry  The entry to populate for the specified game
 */
extern void wii_handy_db_get_entry( char* hash, HandyDBEntry* entry );

/*
 * Writes the specified entry to the database for the game with the specified
 * hash.
 *
 * hash		The hash of the game
 * entry	The entry to write to the database
 * return	Whether the write was successful
 */
extern int wii_handy_db_write_entry( char* hash, HandyDBEntry *entry );

/*
 * Deletes the entry from the database with the specified hash
 *
 * hash		The hash of the game
 * return	Whether the delete was successful
 */
extern int wii_handy_db_delete_entry( char* hash );

/*
 * Populates the specified entry with default values.
 *
 * entry      The entry to populate with default values
 * fullClear  Whether to fully clear the entry
 */
extern void wii_handy_db_get_defaults( HandyDBEntry* entry );

#endif
