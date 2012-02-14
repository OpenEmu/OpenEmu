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

#ifndef WII_CONFIG_H
#define WII_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <gctypes.h>

/*
 * Read the configuration file
 *
 * return   Whether we read the configuration file successfully
 */
extern BOOL wii_read_config();

/*
 * Write the configuration file
 *
 * return   Whether we wrote the configuration file successfully
 */
extern BOOL wii_write_config();

//
// Methods to be implemented by application
//

/*
 * Handles reading a particular configuration value
 *
 * name   The name of the config value
 * value  The config value
 */
extern void wii_config_handle_read_value( char* name, char* value );

/*
 * Handles the writing of the configuration file
 *
 * fp   The file pointer
 */
extern void wii_config_handle_write_config( FILE *fp );

#ifdef __cplusplus
}
#endif

#endif  
