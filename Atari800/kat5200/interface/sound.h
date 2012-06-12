/******************************************************************************
*
* FILENAME: sound.h
*
* DESCRIPTION:  This contains function declarations for sound handling
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   01/31/07  bberlin      Creation, break out from config
******************************************************************************/
#ifndef sound_h
#define sound_h

typedef struct {
	char name[257];
	char description[257];
	int on;
	int filter_on;
	int freq;
	int samples;
	int treble;
	int bass;
} t_atari_sound;

/*
 * Function Declarations
 */
int sound_set_defaults ( void );
int sound_set_profile ( char *name );
int sound_load_profile ( const char *name, t_atari_sound *p_sound );
int sound_save_profile ( char *name, t_atari_sound *p_sound );

#endif
