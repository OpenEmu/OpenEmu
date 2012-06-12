/******************************************************************************
*
* FILENAME: sound.c
*
* DESCRIPTION:  This contains sound handling
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   01/31/07  bberlin      Creation, break out from config
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sound.h"
#include "kconfig.h"
#include "db_if.h"
#include "util.h"
#include "../kat5200.h"

t_atari_sound g_sound;

/******************************************************************************
**  Function   :  sound_set_defaults                                            
**                                                                    
**  Objective  :  This function sets up defaults for sound settings
**                                                   
**  Parameters :  NONE
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int sound_set_defaults ( void ) {

	strcpy ( g_sound.name, "Internal Default" );
	strcpy ( g_sound.description, "Internal Default" );
	g_sound.on = 1;
	g_sound.filter_on = 0;
	g_sound.freq = 44100;
	g_sound.samples = 1024;
	g_sound.treble = -8;
	g_sound.bass = 16;

	return 0;

} /* end sound_set_defaults */

/************************************************************************
 *
 *  Profile functions
 *
 ************************************************************************/

/******************************************************************************
**  Function   :  sound_set_profile                                            
**                                                                    
**  Objective  :  This function loads up an sound profile from db and puts it
**                in our global sound struct
**                                                   
**  Parameters :  name - name of sound profile to load
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int sound_set_profile ( char *name ) {

	if ( *name == '\0' )
		sound_load_profile ( config_get_ptr()->default_sound_profile, &g_sound );
	else
		sound_load_profile ( name, &g_sound );

	return 0;

} /* end sound_set_profile */

/******************************************************************************
**  Function   :  sound_load_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  sound     - pointer to sound struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                -1 if failure
******************************************************************************/ 
int sound_load_callback ( void *sound, int argc, char **argv, char **azColName ) {

	int i;
	t_atari_sound *p_sound = sound;

	if ( argc < 7 )
		return -1;

	/*
	 * THIS DEPENDS ON MATCHING THE DATABASE!
	 */
	i = -1;
	if ( argv[++i] ) strcpy ( p_sound->name, argv[i] );
	if ( argv[++i] ) strcpy ( p_sound->description, argv[i] );
	if ( argv[++i] ) p_sound->on = atoi ( argv[i] );
	if ( argv[++i] ) p_sound->filter_on = atoi ( argv[i] );
	if ( argv[++i] ) p_sound->freq = atoi ( argv[i] );
	if ( argv[++i] ) p_sound->samples = atoi ( argv[i] );
	if ( argv[++i] ) p_sound->treble = atoi ( argv[i] );
	if ( argv[++i] ) p_sound->bass = atoi ( argv[i] );

	return 0;

} /* end sound_load_callback */

/******************************************************************************
**  Function   :  sound_load_profile                                            
**                                                                    
**  Objective  :  This function loads up a sound profile from db
**                                                   
**  Parameters :  name     - name of sound profile
**                p_sound  - sound structure to load with info
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int sound_load_profile ( const char *name, t_atari_sound *p_sound ) {
	
	char statement[257];
	int status;
	char *p_tmp;

	/*
	 * Create the SQL query statement
	 */
	sprintf ( statement, "SELECT * FROM Sound WHERE Name='%s'", 
	            p_tmp=util_replace_and_copy(name, "'", "''" ) );

	free ( p_tmp );

	status = db_if_exec_sql ( statement, sound_load_callback, p_sound );

	return status;

} /* end sound_load_profile */

/******************************************************************************
**  Function   :  sound_save_profile                                            
**                                                                    
**  Objective  :  This function saves the sound configuration to db
**                                                   
**  Parameters :  name     - name of sound profile to save
**                p_sound  - sound structure containing info to save
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int sound_save_profile ( char *name, t_atari_sound *p_sound ) {

	char statement[257];
	char *p_tmp[2];
	int status = 0;
	int i;

	/*
	 * Create the INSERT statement and execute
	 */
	sprintf ( statement, "INSERT OR REPLACE INTO Sound VALUES ( '%s', '%s', %d, %d, %d, %d, %d, %d )",
	            p_tmp[0] = util_replace_and_copy(name, "'", "''" ),
	            p_tmp[1] = util_replace_and_copy(p_sound->description, "'", "''" ),
	            p_sound->on, p_sound->filter_on, p_sound->freq, p_sound->samples, 
	            p_sound->treble, p_sound->bass );
	status = db_if_exec_sql ( statement, 0, 0 );

	for ( i = 0; i < 2; ++i )
		free ( p_tmp[i] );

	return status;

} /* end sound_save_profile */

