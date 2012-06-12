/******************************************************************************
*
* FILENAME: ui.c
*
* DESCRIPTION:  This contains user interface key handling
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.6.0   12/19/07  bberlin      Creation
******************************************************************************/
#include <sys/stat.h>
#include "db_if.h"
#include "ui.h"
#include "util.h"
#include "states.h"
#include "media.h"
#include "video.h"
#include "sound.h"
#include "../core/debug.h"
#include "../gui/gui.h"

t_ui g_ui;
extern t_pc_input g_pc_input;

int ui_debug (int key, int temp, int temp1);
int ui_exit (int key, int temp, int temp1);
int ui_back (int key, int temp, int temp1);
int ui_fullscreen (int key, int temp, int temp1);
int ui_load_state (int player, int temp, int temp1);
int ui_save_state (int player, int temp, int temp1);

/******************************************************************************
**  Function   :  ui_set_defaults                                            
**                                                                    
**  Objective  :  This function sets up defaults for sound settings
**                                                   
**  Parameters :  NONE
**                 
**  return     :  0
** 
******************************************************************************/ 
int ui_set_defaults ( void ) {

	strcpy ( g_ui.name, "Internal Default" );
	strcpy ( g_ui.description, "Internal Default" );
	pc_set_ui_defaults ();

	return 0;

} /* end ui_set_defaults */

/************************************************************************
 *
 *  Profile functions
 *
 ************************************************************************/

/******************************************************************************
**  Function   :  ui_set_profile                                            
**                                                                    
**  Objective  :  This function loads up a ui profile from db and puts it
**                in our global ui struct
**                                                   
**  Parameters :  name - name of profile to load
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int ui_set_profile ( char *name ) {
		
	ui_load_profile ( name, &g_ui );

	return 0;

} /* end ui_set_profile */

/******************************************************************************
**  Function   :  ui_load_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  ui        - pointer to ui struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int ui_load_callback ( void *ui, int argc, char **argv, char **azColName ) {

	int i,j;
	t_ui *p_ui = ui;

	if ( argc < 7 )
		return -1;

	/*
	 * THIS DEPENDS ON MATCHING THE DATABASE!
	 */
	i = -1;
	if ( argv[++i] ) strcpy ( p_ui->name, argv[i] );
	if ( argv[++i] ) strcpy ( p_ui->description, argv[i] );
	for ( j = 0; j < UI_KEY_MAX; ++j )
		if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_ui->keys[j] );

	return 0;

} /* end ui_load_callback */

/******************************************************************************
**  Function   :  ui_load_profile                                            
**                                                                    
**  Objective  :  This function loads up a ui profile from db
**                                                   
**  Parameters :  name     - name of ui profile
**                p_ui     - ui structure to load with info
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int ui_load_profile ( const char *name, t_ui *p_ui ) {
	
	char statement[257];
	int status;

	/*
	 * Create the SQL query statement
	 */
	sprintf ( statement, "SELECT * FROM UIKeys WHERE Name='%s'", name );
	status = db_if_exec_sql ( statement, ui_load_callback, p_ui );

	return status;

} /* end ui_load_profile */

/******************************************************************************
**  Function   :  ui_save_profile                                            
**                                                                    
**  Objective  :  This function saves the ui configuration to db
**                                                   
**  Parameters :  name - name of ui profile to save
**                p_ui - ui structure containing info to save
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int ui_save_profile ( char *name, t_ui *p_ui ) {

	char statement[257];
	char tstatement[257];
	char column[50];
	int status = 0;
	int i, id=0;

	db_if_exec_sql ( "BEGIN", 0, 0 );

	/*
	 * Create the INSERT statement and execute
	 */
	sprintf ( statement, "INSERT OR REPLACE INTO UIKeys VALUES ( '%s', '%s'",
	            name, p_ui->description );

	for ( i = 0; i < UI_KEY_MAX; ++i ) {
		switch ( i ) {
			case 0: strcpy ( column, "Exit" ); break;
			case 1: strcpy ( column, "Back" ); break;
			case 2: strcpy ( column, "Fullscreen" ); break;
			case 3: strcpy ( column, "Screenshot" ); break;
			case 4: strcpy ( column, "LoadState1" ); break;
			case 5: strcpy ( column, "LoadState2" ); break;
			case 6: strcpy ( column, "LoadState3" ); break;
			case 7: strcpy ( column, "LoadState4" ); break;
			case 8: strcpy ( column, "LoadState5" ); break;
			case 9: strcpy ( column, "LoadState6" ); break;
			case 10: strcpy ( column, "LoadState7" ); break;
			case 11: strcpy ( column, "LoadState8" ); break;
			case 12: strcpy ( column, "LoadState9" ); break;
			case 13: strcpy ( column, "SaveState1" ); break;
			case 14: strcpy ( column, "SaveState2" ); break;
			case 15: strcpy ( column, "SaveState3" ); break;
			case 16: strcpy ( column, "SaveState4" ); break;
			case 17: strcpy ( column, "SaveState5" ); break;
			case 18: strcpy ( column, "SaveState6" ); break;
			case 19: strcpy ( column, "SaveState7" ); break;
			case 20: strcpy ( column, "SaveState8" ); break;
			case 21: strcpy ( column, "SaveState9" ); break;
		}
		sprintf ( tstatement, "SELECT %s FROM UIKeys WHERE Name='%s'", column, name );
		input_write_common ( tstatement, &p_ui->keys[i], &id );
		sprintf ( statement, "%s, %d", statement, id );
	}

	strcat ( statement, " )" );

	status = db_if_exec_sql ( statement, 0, 0 );

	db_if_exec_sql ( "END", 0, 0 );

	return status;

} /* end ui_save_profile */

/************************************************************************
 *
 *  Assign PC inputs to Atari Inputs and setup input functions
 *
 ************************************************************************/
/******************************************************************************
**  Function   :  ui_setup_keys
**                            
**  Objective  :  This function sets up the user interface keys and long as no
**                modifier keys are involved.
**
**  Parameters :  NONE
**                                                
**  return     :  0
**      
******************************************************************************/
int ui_setup_keys ( void ) {
	
	int i;
	int tmp_part_num, tmp_device_num, tmp_player;
	int ( *tmp_function ) ( int,int,int );

//	g_pc_input.keyboard[SDLK_F1].key_down_function = ui_debug;

	for ( i = 0; i < UI_KEY_MAX; ++i ) {

		tmp_part_num = g_ui.keys[i].part_num;
		tmp_device_num = g_ui.keys[i].device_num;
		tmp_player = 0;

		switch ( i ) {
			case UI_KEY_EXIT: tmp_function = ui_exit; break;
			case UI_KEY_BACK: tmp_function = ui_back; break;
			case UI_KEY_FULLSCREEN: tmp_function = ui_fullscreen; break;
			case UI_KEY_SCREENSHOT: tmp_function = ui_screenshot; break;
			case UI_KEY_LOAD_STATE_1:
			case UI_KEY_LOAD_STATE_2:
			case UI_KEY_LOAD_STATE_3:
			case UI_KEY_LOAD_STATE_4:
			case UI_KEY_LOAD_STATE_5:
			case UI_KEY_LOAD_STATE_6:
			case UI_KEY_LOAD_STATE_7:
			case UI_KEY_LOAD_STATE_8:
			case UI_KEY_LOAD_STATE_9:
				tmp_function = ui_load_state;
				tmp_player = i - UI_KEY_LOAD_STATE_1 + 1;
				break;
			case UI_KEY_SAVE_STATE_1:
			case UI_KEY_SAVE_STATE_2:
			case UI_KEY_SAVE_STATE_3:
			case UI_KEY_SAVE_STATE_4:
			case UI_KEY_SAVE_STATE_5:
			case UI_KEY_SAVE_STATE_6:
			case UI_KEY_SAVE_STATE_7:
			case UI_KEY_SAVE_STATE_8:
			case UI_KEY_SAVE_STATE_9:
				tmp_function = ui_save_state;
				tmp_player = i - UI_KEY_SAVE_STATE_1 + 1;
				break;
			default: tmp_function = ui_fullscreen; break;
		}

		if ( g_ui.keys[i].device == DEV_KEYBOARD ) {
			if ( !g_ui.keys[i].device_num ) {
				g_pc_input.keyboard[tmp_part_num].key_down_function = tmp_function;
				g_pc_input.keyboard[tmp_part_num].player = tmp_player;
			}
			else {
				g_pc_input.ui_keyboard[i].key_down_function = tmp_function;
				g_pc_input.ui_keyboard[i].modifier = tmp_device_num;
				g_pc_input.ui_keyboard[i].key = tmp_part_num;
				g_pc_input.ui_keyboard[i].player = tmp_player;
			}
		}

		if ( g_ui.keys[i].device == DEV_JOYSTICK ) {
			g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = tmp_function;
			g_pc_input.button[tmp_device_num][tmp_part_num].player = tmp_player;
		}

		if ( g_ui.keys[i].device == DEV_MOUSE ) {
			g_pc_input.mouse_button[tmp_part_num].button_down_function = tmp_function;
			g_pc_input.mouse_button[tmp_part_num].player = tmp_player;
		}

	} /* end for UI keys */

	return 0;

} /* ui_setup_keys */

/******************************************************************************
**  Function   :  ui_functions
**                            
**  Objective  :  This function handles ui functions occuring on key down
**                events.
**
**  Parameters :  key   - which key has been pressed
**                temp  - do nothing variable
**                temp1 - do nothing variable
**                                                
**  return     :  NONE
**      
******************************************************************************/
int ui_debug (int key, int temp, int temp1) {

	debug_get_input ();

	return 1;

} /* end ui_debug */

int ui_exit (int key, int temp, int temp1) {

	return -1;

} /* end ui_exit */

int ui_back (int key, int temp, int temp1) {

	int i;

	i = gui_show_main (0,0);
	if ( i != -1 )
		pc_game_init ();
	if ( !i ) 
		debug_get_input ();
	return i;

} /* end ui_back */

int ui_fullscreen (int key, int temp, int temp1) {

	pc_toggle_fullscreen();

	return 0;

} /* end ui_fullscreen */

int ui_screenshot (int key, int temp, int temp1) {

	char file[1100];
	int x = 0;
	struct stat file_status;

	sprintf ( file, "%lX.bmp", media_get_ptr()->crc );
	util_set_file_to_program_dir ( file );

	while ( !stat ( file, &file_status ) ) {

		sprintf ( file, "%lX_%02d.bmp", media_get_ptr()->crc, ++x );
		util_set_file_to_program_dir ( file );
	}

	pc_save_screenshot ( file );

	return 0;

} /* end ui_screenshot */

int ui_load_state (int player, int temp, int temp1) {

	recall_quick_state ( player, console_get_ptr() );

	input_set_profile ( media_get_ptr()->input_profile );
	video_set_profile ( media_get_ptr()->video_profile );
	sound_set_profile ( media_get_ptr()->sound_profile );

	mem_assign_ops ( console_get_ptr()->cpu );

	return 1;

} /* end ui_load_state */

int ui_save_state (int player, int temp, int temp1) {

	save_quick_state ( player, console_get_ptr() );

	return 0;

} /* end ui_save_state */

