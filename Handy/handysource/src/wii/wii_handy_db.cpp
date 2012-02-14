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

#include "system.h"

#include "wii_app.h"
#include "wii_util.h"

#include "wii_handy.h"
#include "wii_handy_db.h"

#ifdef WII_NETTRACE
#include <network.h>
#include "net_print.h"  
#endif

#define DB_FILE_PATH WII_FILES_DIR "wiihandy.db"
#define DB_TMP_FILE_PATH WII_FILES_DIR "wiihandy.db.tmp"
#define DB_OLD_FILE_PATH WII_FILES_DIR "wiihandy.db.old"

// The database file
static char db_file[WII_MAX_PATH] = "";
// The database temp file
static char db_tmp_file[WII_MAX_PATH] = "";
// The database old file
static char db_old_file[WII_MAX_PATH] = "";

/*
 * Returns the path to the database file
 *
 * return   The path to the database file
 */
static char* get_db_path()
{
  if( db_file[0] == '\0' )
  {
    snprintf( 
      db_file, WII_MAX_PATH, "%s%s", wii_get_fs_prefix(), 
      DB_FILE_PATH );
  }

  return db_file;
}

/*
 * Returns the path to the database temporary file
 *
 * return   The path to the database temporary file
 */
static char* get_db_tmp_path()
{
  if( db_tmp_file[0] == '\0' )
  {
    snprintf( 
      db_tmp_file, WII_MAX_PATH, "%s%s", wii_get_fs_prefix(), 
      DB_TMP_FILE_PATH );
  }

  return db_tmp_file;
}

/*
 * Returns the path to the database old file
 *
 * return   The path to the database old file
 */
static char* get_db_old_path()
{
  if( db_old_file[0] == '\0' )
  {
    snprintf( 
      db_old_file, WII_MAX_PATH, "%s%s", wii_get_fs_prefix(), 
      DB_OLD_FILE_PATH );
  }

  return db_old_file;
}

/*
 * Populates the specified entry with default values.
 *
 * entry      The entry to populate with default values
 * fullClear  Whether to fully clear the entry
 */
void wii_handy_db_get_defaults( HandyDBEntry* entry )
{
  entry->maxFrames = 0;
  entry->orient = MIKIE_BAD_MODE;
}

/*
 * Attempts to locate a hash in the specified source string. If it
 * is found, it is copied into dest.
 *
 * source   The source string
 * dest     The destination string
 * return   non-zero if the hash was found and copied
 */
static int get_hash( char* source, char* dest )
{
  int db_hash_len = 0;  // The length of the hash
  char *end_idx;		    // End index of the hash  
  char *start_idx;      // Start index of the hash

  start_idx = source;
  if( *start_idx == '[' )
  {
    ++start_idx;
    end_idx = strrchr( start_idx, ']' );
    if( end_idx != 0 )
    {				  			
      db_hash_len = end_idx - start_idx;  
      strncpy( dest, start_idx, db_hash_len );		  
      dest[db_hash_len] = '\0';
      return 1;
    }    
  }

  return 0;
} 

/*
 * Writes the database entry to the specified file
 *
 * file	  The file to write the entry to
 * hash	  The hash for the entry
 * entry  The entry
 */
static void write_entry( FILE* file, char* hash, HandyDBEntry *entry )
{
  int i;

  if( !entry ) return;

  char hex[64] = "";

  fprintf( file, "[%s]\n", hash );
  fprintf( file, "name=%s\n", entry->name );
  fprintf( file, "maxFrames=%d\n", entry->maxFrames );
  fprintf( file, "orient=%d\n", entry->orient );
}

/*
 * Returns the database entry for the game with the specified hash
 *
 * hash	  The hash of the game
 * entry  The entry to populate for the specified game
 */
void wii_handy_db_get_entry( char* hash, HandyDBEntry* entry )
{
  char buff[255];     // The buffer to use when reading the file    
  FILE* db_file;      // The database file
  char db_hash[255];  // A hash found in the file we are reading from
  int read_mode = 0;  // The current read mode 
  char* ptr;          // Pointer into the current entry value

  // Populate the entry with the defaults
  memset( entry, 0x0, sizeof( HandyDBEntry ) );
  wii_handy_db_get_defaults( entry );  

  db_file = fopen( get_db_path(), "r" );

#ifdef WII_NETTRACE
#if 0
  char val[256];
  sprintf( val, "fopen %s=%p\n", get_db_path(), db_file );
  net_print_string(__FILE__,__LINE__, val );
#endif
#endif

  if( db_file != 0 )
  {	
    while( fgets( buff, sizeof(buff), db_file ) != 0 )
    {                
      if( read_mode == 2 )
      {
        // We moved past the current record, exit.
        break;
      }
      
      if( read_mode == 1 )
      {
        // Read from the matching database entry        
        ptr = strchr( buff, '=' );
        if( ptr )
        {
          *ptr++ = '\0';
          Util_trim( buff );
          Util_trim( ptr );
          
          if( !strcmp( buff, "name" ) )
          {
            Util_strlcpy( entry->name, ptr, sizeof(entry->name) );          
          }
          else if( !strcmp( buff, "maxFrames" ) )
          {
            entry->maxFrames = Util_sscandec( ptr );
          }          
          else if( !strcmp( buff, "orient" ) )
          {
            entry->orient = Util_sscandec( ptr );
          }
        }                
      }
    
      // Search for the hash
      if( get_hash( buff, db_hash ) && read_mode < 2 )
      {        
        if( read_mode || !strcmp( hash, db_hash ) )
        {
          entry->loaded = 1;
          read_mode++;        
        }                
      }
    }

    fclose( db_file );
  }
}

/*
 * Deletes the entry from the database with the specified hash
 *
 * hash		The hash of the game
 * return	Whether the delete was successful
 */
int wii_handy_db_delete_entry( char* hash )
{
  return wii_handy_db_write_entry( hash, 0 );
}

/*
 * Writes the specified entry to the database for the game with the specified
 * hash.
 *
 * hash		The hash of the game
 * entry	The entry to write to the database (null to delete the entry)
 * return	Whether the write was successful
 */
int wii_handy_db_write_entry( char* hash, HandyDBEntry *entry )
{  
  char buff[255];		    // The buffer to use when reading the file  
  char db_hash[255];	  // A hash found in the file we are reading from
  int copy_mode = 0;	  // The current copy mode 
  FILE* tmp_file = 0;	  // The temp file
  FILE* old_file = 0;	  // The old file

  // The database file
  FILE* db_file = fopen( get_db_path(), "r" );

  // A database file doesn't exist, create a new one
  if( !db_file )
  {
    db_file = fopen( get_db_path(), "w" );
    if( !db_file )
    {
      // Unable to create DB file
      return 0;
    }

    // Write the entry
    write_entry( db_file, hash, entry );

    fclose( db_file );
  }  
  else
  {
    //
    // A database exists, search for the appropriate hash while copying
    // its current contents to a temp file
    //

    // Open up the temp file
    tmp_file = fopen( get_db_tmp_path(), "w" );
    if( !tmp_file )
    {
      fclose( db_file );

      // Unable to create temp file
      return 0;
    }

    //
    // Loop and copy	
    //

    while( fgets( buff, sizeof(buff), db_file ) != 0 )
    {	  	            
      // Check if we found a hash
      if( copy_mode < 2 && get_hash( buff, db_hash ) )
      {
        if( copy_mode )
        {
          copy_mode++;
        }
        else if( !strcmp( hash, db_hash ) )
        {		  
          // We have matching hashes, write out the new entry
          write_entry( tmp_file, hash, entry );
          copy_mode++;
        }
      }

      if( copy_mode != 1 )
      {	
        fprintf( tmp_file, "%s", buff );
      }
    }

    if( !copy_mode )
    {
      // We didn't find the hash in the database, add it
      write_entry( tmp_file, hash, entry );	
    }

    fclose( db_file );
    fclose( tmp_file );  

    //
    // Make sure the temporary file exists
    // We do this due to the instability of the Wii SD card
    //
    tmp_file = fopen( get_db_tmp_path(), "r" );
    if( !tmp_file )
    {      
      // Unable to find temp file
      return 0;
    }
    fclose( tmp_file );

    // Delete old file (if it exists)
    if( ( old_file = fopen( get_db_old_path(), "r" ) ) != 0 )
    {
      fclose( old_file );
      if( remove( get_db_old_path() ) != 0 )
      {
        return 0;
      }
    }

    // Rename database file to old file
    if( rename( get_db_path(), get_db_old_path() ) != 0 )
    {
      return 0;
    }

    // Rename temp file to database file	
    if( rename( get_db_tmp_path(), get_db_path() ) != 0 )
    {
      return 0;
    }
  }

  return 1;
}
