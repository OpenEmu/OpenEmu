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

#include "wii_app.h"
#include "wii_config.h"
#include "wii_util.h"

// The config file
static char config_file[WII_MAX_PATH] = "";

/*
 * Returns the path to the config file
 *
 * return   The path to the config file
 */
static char* get_config_file_path()
{
  if( config_file[0] == '\0' )
  {
    snprintf( config_file, WII_MAX_PATH, "%s%s", wii_get_fs_prefix(), WII_CONFIG_FILE );
  }

  return config_file;
}

/*
 * Read the configuration file
 *
 * return   Whether we read the configuration file successfully
 */
BOOL wii_read_config()
{
  char buff[512];

  FILE *fp;
  fp = fopen( get_config_file_path(), "r" );
  if (fp == NULL) 
  {	
    return FALSE;		
  }

  while( fgets( buff, sizeof( buff ), fp ) ) 
  {
    char *ptr;
    Util_chomp( buff );
    ptr = strchr( buff, '=' );
    if( ptr != NULL ) 
    {
      *ptr++ = '\0';
      Util_trim( buff );
      Util_trim( ptr );

      // Read the value
      wii_config_handle_read_value( buff, ptr );
    }
  }

  fclose(fp);

  return TRUE;
}

/*
 * Write the configuration file
 *
 * return   Whether we wrote the configuration file successfully
 */
BOOL wii_write_config()
{
  FILE *fp;
  fp = fopen( get_config_file_path(), "w" );
  if( fp == NULL ) 
  {
    return FALSE;
  }

  // Write the configuration file
  wii_config_handle_write_config( fp );

  fclose(fp);

  return TRUE;
}