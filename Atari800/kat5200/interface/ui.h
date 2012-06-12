/******************************************************************************
*
* FILENAME: ui.h
*
* DESCRIPTION:  This contains function declarations for user interface
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   12/19/07  bberlin      Creation
******************************************************************************/
#ifndef ui_h
#define ui_h

#include "input.h"

typedef enum {
	UI_KEY_EXIT,
	UI_KEY_BACK,
	UI_KEY_FULLSCREEN,
	UI_KEY_SCREENSHOT,
	UI_KEY_LOAD_STATE_1,
	UI_KEY_LOAD_STATE_2,
	UI_KEY_LOAD_STATE_3,
	UI_KEY_LOAD_STATE_4,
	UI_KEY_LOAD_STATE_5,
	UI_KEY_LOAD_STATE_6,
	UI_KEY_LOAD_STATE_7,
	UI_KEY_LOAD_STATE_8,
	UI_KEY_LOAD_STATE_9,
	UI_KEY_SAVE_STATE_1,
	UI_KEY_SAVE_STATE_2,
	UI_KEY_SAVE_STATE_3,
	UI_KEY_SAVE_STATE_4,
	UI_KEY_SAVE_STATE_5,
	UI_KEY_SAVE_STATE_6,
	UI_KEY_SAVE_STATE_7,
	UI_KEY_SAVE_STATE_8,
	UI_KEY_SAVE_STATE_9,
	UI_KEY_MAX
} e_ui_key;

typedef struct {
	char name[257];
	char description[257];
	t_atari_common keys[UI_KEY_MAX];
} t_ui;

/*
 * Function Declarations
 */
int ui_set_defaults ( void );
int ui_set_profile ( char *name );
int ui_load_profile ( const char *name, t_ui *p_ui );
int ui_save_profile ( char *name, t_ui *p_ui );
int ui_setup_keys ( void );
int ui_screenshot (int key, int temp, int temp1);

#endif
