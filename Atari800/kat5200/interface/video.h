/******************************************************************************
*
* FILENAME: video.h
*
* DESCRIPTION:  This contains function declarations for video handling
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   01/31/06  bberlin      Creation, break out from config
******************************************************************************/
#ifndef video_h
#define video_h

#include "atari_ntsc.h"

typedef struct {
	char name[257];
	char description[257];
	char ntsc_palette[257];
	char pal_palette[257];
	int fullscreen;
	int height;
	int width;
	int pixel_depth;
	int zoom;
	int widescreen;
	int hires_artifacts;
	int ntsc_filter_on;
	atari_ntsc_setup_t ntsc_setup;
} t_atari_video;

/*
 * Function Declarations
 */
int video_set_defaults ( void );
int video_set_profile ( char *filename );
int video_load_profile ( const char *filename, t_atari_video *p_video );
int video_save_profile ( char *filename, t_atari_video *p_video );

#endif
