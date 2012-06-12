/******************************************************************************
*
* FILENAME: video.c
*
* DESCRIPTION:  This contains video handling
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   01/31/06  bberlin      Creation, break out from config
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "video.h"
#include "db_if.h"
#include "util.h"
#include "kconfig.h"
#include "../kat5200.h"

t_atari_video g_video;
static float g_decoder_matrix[6];

/******************************************************************************
**  Function   :  video_set_defaults                                            
**                                                                    
**  Objective  :  This function sets up defaults for video settings
**                                                   
**  Parameters :  NONE
**                 
**  return     :  0 
**
******************************************************************************/ 
int video_set_defaults ( void ) {

	strcpy ( g_video.name, "Internal Defaults" );
	strcpy ( g_video.description, "Internal Defaults" );
	strcpy ( g_video.ntsc_palette, "" );
	strcpy ( g_video.pal_palette, "" );
	g_video.fullscreen = 0;
	g_video.height = 240;
	g_video.width = 320;
	g_video.hires_artifacts = 0;
	g_video.pixel_depth = 8;
	g_video.zoom = 0;
	g_video.widescreen = 0;
	g_video.ntsc_filter_on = 0;

	g_video.ntsc_setup = atari_ntsc_rf;

	return 0;

} /* end video_set_defaults */

/************************************************************************
 *
 *  Profile functions
 *
 ************************************************************************/

/******************************************************************************
**  Function   :  video_set_profile                                            
**                                                                    
**  Objective  :  This function loads up an video profile from file and puts it
**                in our global video struct
**                                                   
**  Parameters :  name - name of video profile to load
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int video_set_profile ( char *name ) {
		
	if ( *name == '\0' )
		video_load_profile ( config_get_ptr()->default_video_profile, &g_video );
	else
		video_load_profile ( name, &g_video );

	return 0;

} /* end video_set_profile */

/******************************************************************************
**  Function   :  video_filter_load_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  ntsc_fil  - pointer to the ntsc filter struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success
**                -1 if failure
******************************************************************************/ 
int video_filter_load_callback ( void *ntsc_filter, int argc, char **argv, char **azColName ) {

	atari_ntsc_setup_t *p_ntsc_filter = ntsc_filter;
	int temp=0;
	int i;

	if ( argc < 20 )
		return -1;

	/*
	 * THIS DEPENDS ON MATCHING THE ORDER IN THE DATABASE!
	 */
	i = 0;
	if ( argv[++i] ) p_ntsc_filter->hue = strtod ( argv[i], 0 );
	if ( argv[++i] ) p_ntsc_filter->saturation = strtod ( argv[i], 0 );
	if ( argv[++i] ) p_ntsc_filter->contrast = strtod ( argv[i], 0 );
	if ( argv[++i] ) p_ntsc_filter->brightness = strtod ( argv[i], 0 );
	if ( argv[++i] ) p_ntsc_filter->sharpness = strtod ( argv[i], 0 );
	if ( argv[++i] ) p_ntsc_filter->gamma = strtod ( argv[i], 0 );
	if ( argv[++i] ) p_ntsc_filter->resolution = strtod ( argv[i], 0 );
	if ( argv[++i] ) p_ntsc_filter->burst_phase = strtod ( argv[i], 0 );
	if ( argv[++i] ) p_ntsc_filter->fringing = strtod ( argv[i], 0 );
	if ( argv[++i] ) p_ntsc_filter->bleed = strtod ( argv[i], 0 );
	if ( argv[++i] ) p_ntsc_filter->hue_warping = strtod ( argv[i], 0 );
	if ( argv[++i] ) p_ntsc_filter->merge_fields = atoi ( argv[i] );
	if ( argv[++i] ) temp = atoi ( argv[i] );
	if ( temp ) {
		for ( i = 0; i < 6; ++i ) {
			if ( argv[i] ) g_decoder_matrix[i] = (float)strtod ( argv[i], 0 );
		}
		p_ntsc_filter->decoder_matrix = g_decoder_matrix;
	}
	else {
		p_ntsc_filter->decoder_matrix = 0;
	}

	return 0;

} /* end video_filter_load_callback */

/******************************************************************************
**  Function   :  video_load_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  video     - pointer to video struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**
**  return     :  0 if success
**                -1 if failure
******************************************************************************/ 
int video_load_callback ( void *video, int argc, char **argv, char **azColName ) {

	char statement[257];
	t_atari_video *p_video = video;
	int ntsc_filter_num=0;
	int i, status;

	if ( argc < 13 )
		return -1;

	/*
	 * THIS DEPENDS ON MATCHING THE ORDER IN THE DATABASE!
	 */
	i = -1;
	if ( argv[++i] ) strcpy ( p_video->name, argv[i] );
	if ( argv[++i] ) strcpy ( p_video->description, argv[i] );
	if ( argv[++i] ) strcpy ( p_video->ntsc_palette, argv[i] );
	if ( argv[++i] ) strcpy ( p_video->pal_palette, argv[i] );
	if ( argv[++i] ) p_video->fullscreen = atoi ( argv[i] );
	if ( argv[++i] ) p_video->width = atoi ( argv[i] );
	if ( argv[++i] ) p_video->height = atoi ( argv[i] );
	if ( argv[++i] ) p_video->pixel_depth = atoi ( argv[i] );
	if ( argv[++i] ) p_video->zoom = atoi ( argv[i] );
	if ( argv[++i] ) p_video->widescreen = atoi ( argv[i] );
	if ( argv[++i] ) p_video->hires_artifacts = atoi ( argv[i] );
	if ( argv[++i] ) p_video->ntsc_filter_on = atoi ( argv[i] );
	if ( argv[++i] ) ntsc_filter_num = atoi ( argv[i] );

	sprintf ( statement, "SELECT * FROM NTSCFilter WHERE NTSCID='%d'", ntsc_filter_num );
	status = db_if_exec_sql ( statement, video_filter_load_callback, &p_video->ntsc_setup );

	return 0;

} /* end video_load_callback */

/******************************************************************************
**  Function   :  video_load_profile                                            
**                                                                    
**  Objective  :  This function loads up a video profile from file
**                                                   
**  Parameters :  name     - ame containing video to load
**                p_video  - video structure to load with info
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int video_load_profile ( const char *name, t_atari_video *p_video ) {
	
	char statement[257];
	int status;
	char *p_tmp;

	/*
	 * Create the SQL query statement
	 */
	sprintf ( statement, "SELECT * FROM Video WHERE Name='%s'", 
	           p_tmp=util_replace_and_copy(name, "'", "''" ) );

	free ( p_tmp );

	status = db_if_exec_sql ( statement, video_load_callback, p_video );

	return status;

} /* end video_load_profile */

/******************************************************************************
**  Function   :  video_save_profile                                            
**                                                                    
**  Objective  :  This function saves the video configuration to the database
**                                                   
**  Parameters :  name    - name of profile to save
**                p_video - video structure containing info to save
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int video_save_profile ( char *name, t_atari_video *p_video ) {

	int status = 0;
	int filter_exists = 0;
	int dm_enabled = 0;
	char statement[512];
	char values[512];
	char *p_tmp[4];
	int i;
	atari_ntsc_setup_t *p_ntsc_filter = &p_video->ntsc_setup;

	/*
	 * First find out if the NTSC Filter exists
	 */
	sprintf ( statement, "SELECT NTSCFilterID FROM Video WHERE Name='%s'", name );

	dm_enabled = p_ntsc_filter->decoder_matrix ? 1: 0;
	sprintf ( values, "%0.1lf, %0.1lf, %0.1lf, %0.1lf, %0.1lf, %0.1lf, %0.1lf, \
	                   %0.1lf, %0.1lf, %0.1lf, %0.1lf, %d, %d, %0.1f, %0.1f, \
					   %0.1f, %0.1f, %0.1f, %0.1f",
	            p_ntsc_filter->hue, p_ntsc_filter->saturation,
	            p_ntsc_filter->brightness, p_ntsc_filter->contrast, 
	            p_ntsc_filter->sharpness, p_ntsc_filter->gamma,
	            p_ntsc_filter->resolution, p_ntsc_filter->burst_phase,
	            p_ntsc_filter->fringing, p_ntsc_filter->bleed,
	            p_ntsc_filter->hue_warping, p_ntsc_filter->merge_fields,
				dm_enabled, g_decoder_matrix[0], g_decoder_matrix[1],
				g_decoder_matrix[2], g_decoder_matrix[3],
				g_decoder_matrix[4], g_decoder_matrix[5] );
	status = db_if_insert_or_update ( statement, values, "NTSCFilter", &filter_exists );

	sprintf ( statement, "INSERT OR REPLACE INTO Video VALUES ( '%s', '%s', '%s', '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d )",
	            p_tmp[0]=util_replace_and_copy(name, "'", "''" ),
	            p_tmp[1]=util_replace_and_copy(p_video->description, "'", "''" ),
	            p_tmp[2]=util_replace_and_copy(p_video->ntsc_palette, "'", "''" ),
	            p_tmp[3]=util_replace_and_copy(p_video->pal_palette, "'", "''" ),
	            p_video->fullscreen, p_video->width, p_video->height, 
	            p_video->pixel_depth, p_video->zoom, p_video->widescreen, 
				p_video->hires_artifacts, p_video->ntsc_filter_on, 
	            filter_exists );

	for ( i = 0; i < 4; ++i )
		free ( p_tmp[i] );

	status = db_if_exec_sql ( statement, 0, 0 );

	return status;

} /* end video_save_profile */

