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

#ifndef WII_SNAPSHOT_H
#define WII_SNAPSHOT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gctypes.h>

/*
 * Deletes the snapshot for the current rom
 */
extern void wii_delete_snapshot();

/*
 * Saves the current games state to the specified save file
 *
 * savefile The name of the save file to write state to. If this value is NULL,
 *          the default save name for the last rom is used. ( NULL, TRUE )
 */
extern void wii_save_snapshot( const char *savefile, BOOL status_update );

/*
 * Determines whether the specified snapshot is valid
 *
 * return   0  = valid
 *          -1 = does not exist
 *          -2 = not a valid save (wrong size)
 */
extern int wii_check_snapshot( const char *savefile );

//
// Methods to be implemented by application
//

/*
 * Determines the save name for the specified rom file
 *
 * romfile  The name of the rom file
 * buffer   The buffer to write the save name to
 */
extern void wii_snapshot_handle_get_name( const char *romfile, char *buffer );

/*
 * Saves with the specified save name
 *
 * filename   The name of the save file
 * return     Whether the save was successful
 */
extern BOOL wii_snapshot_handle_save( char* filename );

#ifdef __cplusplus
}
#endif

#endif
