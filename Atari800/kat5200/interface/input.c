/******************************************************************************
*
* FILENAME: input.c
*
* DESCRIPTION:  This contains input handling
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   01/10/06  bberlin      Creation, break out from config
* 0.6.2   06/24/09  bberlin      input_setup_keypad change stop to 16, fixes
*                                keypad #1 not working at all
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "input.h"
#include "kconfig.h"
#include "states.h"
#include "media.h"
#include "video.h"
#include "sound.h"
#include "db_if.h"
#include "ui.h"
#include "util.h"
#include "../kat5200.h"
#include "../core/pokey.h"
#include "../core/pia.h"
#include "../core/debug.h"
#include "../core/memory.h"
#include "../gui/gui.h"

typedef struct {
	int axis_idle[2];
	int axis_value[2];
	int pot_low;
	int pot_high;
	int sa_high_threshold;
	int sa_low_threshold;
	int pressed[MAX_DIR];
	int digital_pressed[MAX_DIR];
	unsigned char pot_values[0x10000];
} t_pc_stick_interface;

static t_pc_stick_interface g_stick_interface[MAX_CONTROLLER];

t_atari_input g_input;
t_pc_input g_pc_input;

/*
 * Profile functions
 */
char * input_get_key_string ( int key, char *name );

/*
 * Setup functions
 */
void input_set_controller_defaults ( void );
int input_set_controls ( void );
int input_preload_controls ( void );
int input_setup_frame_functions ( int player );
int input_setup_stick_values ( int player );
int input_setup_stick ( int player, t_atari_stick *stick );
int input_setup_trackball ( int player, t_atari_stick *stick );
int input_setup_keypad ( int player, t_atari_common *ctrl );
int input_setup_digital_stick ( int player, t_atari_stick *stick );
int input_setup_800keypad ( int player );
int input_setup_buttons ( int player );
int input_setup_paddle_button ( int player );
int input_setup_switch ( void );

/*
 * Frame functions
 *   Called by our PC event loop every frame
 */
int input_center_x_axis_frame (int player, int dir);
int input_center_y_axis_frame (int player, int dir);
int input_simulate_analog_x_axis_frame (int player, int tmp);
int input_simulate_analog_y_axis_frame (int player, int tmp);

/*
 * Device Interface functions
 *   Called by our PC event loop to set Atari Registers
 */
int input_do_nothing (int key, int dir, int temp);
int input_frame_do_nothing (int key, int dir);
int input_key_down_stick (int player, int dir, int key);
int input_key_down_stick_sa (int player, int dir, int key);
int input_key_up_stick (int player, int dir, int key);
int input_key_up_stick_sa (int player, int dir, int key);
int input_key_down_digital_stick (int player, int dir, int key);
int input_key_up_digital_stick (int player, int dir, int key);
int input_joystick_pots_sa ( int player, int dir, int reading );
int input_joystick_pots ( int player, int dir, int reading );
int input_keypad_down ( int player, int dir, int kbcode );
int input_keypad_up ( int player, int dir, int kbcode );
int input_axis_digital_stick ( int player, int dir, int reading );
int input_modkey_down ( int player, int dir, int kbcode );
int input_modkey_up ( int player, int dir, int kbcode );
int input_joyaxis_modkey ( int player, int reading, int kbcode );
int input_switch_down ( int player, int dir, int mask );
int input_switch_up ( int player, int dir, int mask );
int input_joyaxis_switch ( int player, int reading, int mask );
int input_joyaxis_keypad ( int player, int reading, int kbcode );
int input_800keypad_down ( int player, int dir, int kbcode );
int input_800keypad_up ( int player, int dir, int kbcode );
int input_joyaxis_800keypad ( int player, int reading, int kbcode );
int input_top_button_down ( int player, int dir, int temp );
int input_top_button_up ( int player, int dir, int temp );
int input_joyaxis_top_button ( int player, int dir, int reading );
int input_joyaxis_bottom_button ( int player, int dir, int reading );
int input_stick_h_axis ( int player, int reading );
int input_stick_v_axis ( int player, int reading );
int input_mouse_axis (int player, int dir, int value);
int input_mouse_vel_axis (int player, int dir, int value);
int input_trackball_x_axis (int player, int dir, int value);
int input_trackball_y_axis (int player, int dir, int value);

/*
 * User Interface functions
 *   Called by our PC event loop to perform user actions
 */
int input_ui_debug (int key, int temp, int temp1);
int input_ui_exit (int key, int temp, int temp1);
int input_ui_back (int key, int temp, int temp1);
int input_ui_fullscreen (int key, int temp, int temp1);
int input_ui_load_state (int player, int temp, int temp1);
int input_ui_save_state (int player, int temp, int temp1);

/******************************************************************************
**  Function   :  input_set_defaults                                            
**                                                                    
**  Objective  :  This function sets up controller and input defaults
**                                                   
**  Parameters :  NONE
**                 
**  return     :  0 if success, otherwise failure
** 
******************************************************************************/ 
int input_set_defaults ( void ) {

	strcpy ( g_input.name, "Internal Defaults" );
	strcpy ( g_input.description, "Internal Defaults" );
	pc_set_controller_defaults ();
	pc_set_keyboard_defaults ();

	input_set_controls();

	return 0;

} /* end input_set_defaults */

/******************************************************************************
**  Function   :  input_set_controls                                            
**                                                                    
**  Objective  :  This function loads up the pc interface with the control
**                functions based on the input configuration
**                                                   
**  Parameters :  NONE
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_set_controls ( void ) {

	int i;
	int max_controller = MAX_CONTROLLER;

	input_preload_controls ();
	ui_setup_keys ();

	if ( g_input.machine_type == MACHINE_TYPE_XL )
		max_controller = 2;

	for ( i = 0; i < max_controller; ++i ) {
		input_setup_frame_functions ( i );
		input_setup_stick_values ( i );

		if ( g_input.machine_type == MACHINE_TYPE_5200 ) {
			if ( g_input.players[i].control_type != CTRLR_TYPE_NONE ) {
				if ( g_input.players[i].control_type == CTRLR_TYPE_TRACKBALL )
					input_setup_trackball ( i, &g_input.players[i].stick );
				else
					input_setup_stick ( i, &g_input.players[i].stick );
				input_setup_keypad ( i, g_input.players[i].keypad );
			}
		}
		else {
			switch ( g_input.players[i].control_type ) {
				case CTRLR_TYPE_JOYSTICK: 
					input_setup_digital_stick ( i, &g_input.players[i].stick );
					break;
				case CTRLR_TYPE_PADDLES: 
					input_setup_stick ( i, &g_input.players[i].paddles );
					break;
				case CTRLR_TYPE_KEYPAD: 
					input_setup_800keypad ( i );
					break;
				default:
					break;
			}
		}
		if ( g_input.players[i].control_type != CTRLR_TYPE_NONE ) {
			if ( g_input.players[i].control_type == CTRLR_TYPE_PADDLES )
				input_setup_paddle_button ( i );
			else
				input_setup_buttons ( i );
		}
	}

	if ( g_input.machine_type != MACHINE_TYPE_5200 ) {
		input_setup_keypad ( 0, g_input.keyboard );
		input_setup_switch ( );
	}

	return 0;

} /* input_set_controls */

/************************************************************************
 *
 *  Profile functions
 *
 ************************************************************************/

/******************************************************************************
**  Function   :  input_set_profile                                            
**                                                                    
**  Objective  :  This function loads up an input profile from file and puts it
**                in our global input struct
**                                                   
**  Parameters :  profile_name - filename containing input to load
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_set_profile ( char *profile_name ) {
		
	if ( *profile_name == '\0' ) {
		if ( config_get_ptr()->machine_type == MACHINE_TYPE_5200 )
			input_load_profile ( config_get_ptr()->default_input_profile, &g_input );
		else
			input_load_profile ( config_get_ptr()->default_800input_profile, &g_input );
	}
	else
		input_load_profile ( profile_name, &g_input );
	input_set_controls ();

	return 0;

} /* end input_set_profile */

/******************************************************************************
**  Function   :  input_keyboard_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  keyboard  - pointer to keyboard array
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_keyboard_callback ( void *keyboard, int argc, char **argv, char **azColName ) {

	int i;
	int row_value;
	int status;
	t_atari_common *p_keyboard = keyboard;
	char statement[50];

	if ( argc < 54 )
		return -1;

	for ( i = 1; i < 54; ++i ) {
		if ( argv[i] ) {
			row_value = 0;
			sprintf ( statement, "SELECT * FROM KeyboardCodes WHERE ROWID='%d'", i );
			status = db_if_exec_sql ( statement, db_if_id_callback, &row_value );
			input_find_common ( atoi(argv[i]), &p_keyboard[row_value] ); 
		}
	}

	return 0;

} /* end input_keyboard_callback */

/******************************************************************************
**  Function   :  input_keypad_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  keypad    - pointer to keypad array
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_keypad_callback ( void *keypad, int argc, char **argv, char **azColName ) {

	int i;
	t_atari_common *p_keypad = keypad;
	int key_values[] = {KEY_START>>1,KEY_PAUSE>>1,KEY_RESET>>1,KEY_STAR>>1,
	                    KEY_POUND>>1,KEY_0>>1,KEY_1>>1,KEY_2>>1,KEY_3>>1,KEY_4>>1,
	                    KEY_5>>1,KEY_6>>1,KEY_7>>1,KEY_8>>1,KEY_9>>1};

	if ( argc < 16 )
		return -1;

	for ( i = 1; i < 16; ++i ) {
		if ( argv[i] )
			input_find_common ( atoi(argv[i]), &p_keypad[key_values[i-1]] ); 
	}

	return 0;

} /* end input_keypad_callback */

/******************************************************************************
**  Function   :  input_player_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  player    - pointer to player struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_player_callback ( void *player, int argc, char **argv, char **azColName ) {

	int i, keypad=0, status;
	t_atari_player *p_player = player;
	char statement[50];

	if ( argc < 16 )
		return -1;

	/*
	 * THIS DEPENDS ON MATCHING THE DATABASE!
	 */
	i = 0;
	if ( argv[++i] ) p_player->control_type = atoi ( argv[i] );
	if ( argv[++i] ) p_player->stick.deadzone = atoi ( argv[i] );
	if ( argv[++i] ) p_player->stick.sensitivity = atoi ( argv[i] );
	if ( argv[++i] ) p_player->stick.simulate_analog = atoi ( argv[i] );
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_player->stick.direction[DIR_LEFT] ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_player->stick.direction[DIR_RIGHT] ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_player->stick.direction[DIR_UP] ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_player->stick.direction[DIR_DOWN] ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_player->paddles.direction[DIR_LEFT] ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_player->paddles.direction[DIR_RIGHT] ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_player->paddles.direction[DIR_UP] ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_player->paddles.direction[DIR_DOWN] ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_player->bottom_button ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_player->top_button ); 
	if ( argv[++i] ) keypad = atoi ( argv[i] );

	sprintf ( statement, "SELECT * FROM KeyPad WHERE KeyPadID='%d'", keypad );
	status = db_if_exec_sql ( statement, input_keypad_callback, p_player->keypad );

	return 0;

} /* end input_player_callback */

/******************************************************************************
**  Function   :  input_load_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  input     - pointer to input struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_load_callback ( void *input, int argc, char **argv, char **azColName ) {

	int i, status;
	int keyboard=0;
	int player[4] = {0};
	char statement[50];
	t_atari_input *p_input = input;

	if ( argc < 15 )
		return -1;

	/*
	 * THIS DEPENDS ON MATCHING THE DATABASE!
	 */
	i = -1;
	if ( argv[++i] ) strcpy ( p_input->name, argv[i] );
	if ( argv[++i] ) strcpy ( p_input->description, argv[i] );
	if ( argv[++i] ) p_input->machine_type = atoi ( argv[i] );
	if ( argv[++i] ) p_input->mouse_speed = atoi ( argv[i] );
	if ( argv[++i] ) player[0] = atoi ( argv[i] );
	if ( argv[++i] ) player[1] = atoi ( argv[i] );
	if ( argv[++i] ) player[2] = atoi ( argv[i] );
	if ( argv[++i] ) player[3] = atoi ( argv[i] );
	if ( argv[++i] ) keyboard = atoi ( argv[i] );
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_input->start_key ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_input->select_key ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_input->option_key ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_input->ctrl_key ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_input->shift_key ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_input->break_key ); 
	if ( argv[++i] ) input_find_common ( atoi(argv[i]), &p_input->reset_key ); 

	for ( i = 0; i < MAX_CONTROLLER; ++i ) {
		sprintf ( statement, "SELECT * FROM Player WHERE PlayerID='%d'", player[i] );
		status = db_if_exec_sql ( statement, input_player_callback, &p_input->players[i] );
	}

	sprintf ( statement, "SELECT * FROM Keyboard WHERE KeyboardID='%d'", keyboard );
	status = db_if_exec_sql ( statement, input_keyboard_callback, p_input->keyboard );

	return 0;

} /* end input_load_callback */

/******************************************************************************
**  Function   :  input_load_profile                                            
**                                                                    
**  Objective  :  This function loads up an input profile from the database
**                                                   
**  Parameters :  name     - name of the input profile to load
**                p_input  - input structure to load with info
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_load_profile ( const char *name, t_atari_input *p_input ) {
	
	int status;
	char statement[257];
	char *p_tmp;

	/*
	 * Create the SQL query statement
	 */
	db_if_exec_sql ( "BEGIN", 0, 0 );

	sprintf ( statement, "SELECT * FROM Input WHERE Name='%s'", 
	           p_tmp=util_replace_and_copy(name, "'", "''" ) );

	free ( p_tmp );

	status = db_if_exec_sql ( statement, input_load_callback, p_input );

	db_if_exec_sql ( "END", 0, 0 );

	return status;

} /* end input_load_profile */

/******************************************************************************
**  Function   :  input_write_keypad                                            
**                                                                    
**  Objective  :  This function writes player parameters to the profile
**                                                   
**  Parameters :  statement - SQL to check for existence of this entry
**                keypad    - struct containing values to write
**                id        - returns the id of this entry in the table
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_write_keypad ( char *statement, t_atari_common *keypad, int *id ) {

	int keyid,i;
	int status;
	char values[256];
	char tstatement[256];
	char key_strings[][20] = {"KEY_START","KEY_PAUSE","KEY_RESET", "KEY_STAR",
	                          "KEY_POUND","KEY_0","KEY_1","KEY_2","KEY_3","KEY_4",
	                          "KEY_5","KEY_6","KEY_7","KEY_8","KEY_9"};
	int key_values[] = {KEY_START>>1,KEY_PAUSE>>1,KEY_RESET>>1,KEY_STAR>>1,
	                    KEY_POUND>>1,KEY_0>>1,KEY_1>>1,KEY_2>>1,KEY_3>>1,KEY_4>>1,
	                    KEY_5>>1,KEY_6>>1,KEY_7>>1,KEY_8>>1,KEY_9>>1};
	*id = 0;

	status = db_if_exec_sql ( statement, db_if_id_callback, id );

	for ( i = 0; i < 15; ++i ) {
		sprintf ( tstatement, "SELECT %s FROM Keypad WHERE KeypadID = %d", key_strings[i], *id );
		input_write_common ( tstatement, &keypad[key_values[i]], &keyid );
		if ( i == 0 )
			sprintf ( values, "%d", keyid );
		else
			sprintf ( values, "%s, %d", values, keyid );
	}

	db_if_insert_or_update ( statement, values, "Keypad", id );

	return 0;

} /* end input_write_keypad */

/******************************************************************************
**  Function   :  input_write_player                                            
**                                                                    
**  Objective  :  This function writes player parameters to the profile
**                                                   
**  Parameters :  statement - SQL to check for existence of this entry
**                player    - struct containing values to write
**                id        - returns the id of this entry in the table
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_write_player ( e_machine_type m_type, char *statement, 
                                        t_atari_player *player, int *id ) {

	char values[512];
	char tstatement[512];
	int keyid, status;

	*id = 0;

	status = db_if_exec_sql ( statement, db_if_id_callback, id );

	sprintf ( values, "%d, %d, %d, %d", player->control_type, 
	            player->stick.deadzone, player->stick.sensitivity, player->stick.simulate_analog );

	sprintf ( tstatement, "SELECT StickLeft FROM Player WHERE PlayerID= %d", *id );
	input_write_common ( tstatement, &player->stick.direction[DIR_LEFT], &keyid );
	sprintf ( values, "%s, %d", values, keyid );

	sprintf ( tstatement, "SELECT StickRight FROM Player WHERE PlayerID= %d", *id );
	input_write_common ( tstatement, &player->stick.direction[DIR_RIGHT], &keyid );
	sprintf ( values, "%s, %d", values, keyid );

	sprintf ( tstatement, "SELECT StickUp FROM Player WHERE PlayerID= %d", *id );
	input_write_common ( tstatement, &player->stick.direction[DIR_UP], &keyid );
	sprintf ( values, "%s, %d", values, keyid );

	sprintf ( tstatement, "SELECT StickDown FROM Player WHERE PlayerID= %d", *id );
	input_write_common ( tstatement, &player->stick.direction[DIR_DOWN], &keyid );
	sprintf ( values, "%s, %d", values, keyid );

	if ( m_type != MACHINE_TYPE_5200 ) {
		sprintf ( tstatement, "SELECT Paddle0CounterClockwise FROM Player WHERE PlayerID= %d", *id );
		input_write_common ( tstatement, &player->paddles.direction[DIR_LEFT], &keyid );
		sprintf ( values, "%s, %d", values, keyid );

		sprintf ( tstatement, "SELECT Paddle0Clockwise FROM Player WHERE PlayerID= %d", *id );
		input_write_common ( tstatement, &player->paddles.direction[DIR_RIGHT], &keyid );
		sprintf ( values, "%s, %d", values, keyid );

		sprintf ( tstatement, "SELECT Paddle1CounterClockwise FROM Player WHERE PlayerID= %d", *id );
		input_write_common ( tstatement, &player->paddles.direction[DIR_UP], &keyid );
		sprintf ( values, "%s, %d", values, keyid );

		sprintf ( tstatement, "SELECT Paddle1Clockwise FROM Player WHERE PlayerID= %d", *id );
		input_write_common ( tstatement, &player->paddles.direction[DIR_DOWN], &keyid );
		sprintf ( values, "%s, %d", values, keyid );
	}
	else {
		strcat ( values, ", NULL, NULL, NULL, NULL" );
	}

	sprintf ( tstatement, "SELECT BottomButton FROM Player WHERE PlayerID= %d", *id );
	input_write_common ( tstatement, &player->bottom_button, &keyid );
	sprintf ( values, "%s, %d", values, keyid );

	if ( m_type == MACHINE_TYPE_5200 ) {
		sprintf ( tstatement, "SELECT TopButton FROM Player WHERE PlayerID= %d", *id );
		input_write_common ( tstatement, &player->top_button, &keyid );
		sprintf ( values, "%s, %d", values, keyid );
	}
	else {
		strcat ( values, ", NULL" );
	}

	sprintf ( tstatement, "SELECT KeypadID FROM Player WHERE PlayerID= %d", *id );
	input_write_keypad ( tstatement, player->keypad, &keyid );
	sprintf ( values, "%s, %d", values, keyid );

	db_if_insert_or_update ( statement, values, "Player", id );

	return 0;

} /* end input_write_player */

int input_string_callback ( void *input, int argc, char **argv, char **azColName ) {

	int i;
	char *string = input;

	if ( argc < 2 )
		return -1;

	i = 0;
	if ( argv[++i] ) strcpy ( string, argv[i] );

	return 0;

} /* end input_string_callback */

/******************************************************************************
**  Function   :  input_write_keyboard                                            
**                                                                    
**  Objective  :  This function writes player parameters to the profile
**                                                   
**  Parameters :  statement - SQL to check for existence of this entry
**                keyboard  - struct containing values to write
**                id        - returns the id of this entry in the table
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_write_keyboard ( char *statement, t_atari_common *keyboard, int *id ) {

	int keyid, row_value;
	int i, status;
	char string[50];
	char values[1024];
	char tstatement[512];

	*id = 0;

	status = db_if_exec_sql ( statement, db_if_id_callback, id );

	for ( i = 0; i < 54; ++i ) {

		sprintf ( tstatement, "SELECT * FROM KeyboardCodes WHERE ROWID= %d", i+1 );
		status = db_if_exec_sql ( tstatement, db_if_id_callback, &row_value );
		sprintf ( tstatement, "SELECT * FROM KeyboardCodes WHERE ROWID= %d", i+1 );
		status = db_if_exec_sql ( tstatement, input_string_callback, string );

		sprintf ( tstatement, "SELECT %s FROM Keyboard WHERE KeyboardID= %d", string, *id );
		input_write_common ( tstatement, &keyboard[row_value], &keyid );
		if ( i == 0 )
			sprintf ( values, "%d", keyid );
		else
			sprintf ( values, "%s, %d", values, keyid );
	}

	db_if_insert_or_update ( statement, values, "Keyboard", id );

	return 0;

} /* end input_write_keyboard */

/******************************************************************************
**  Function   :  input_save_profile                                            
**                                                                    
**  Objective  :  This function saves the input configuration to database
**                                                   
**  Parameters :  name    - name of input profile to save
**                p_input - input structure containing info to save
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_save_profile ( char *name, t_atari_input *p_input ) {

	int i,id;
	int status = 0;
	char statement[257];
	char tstatement[257];
	char *p_tmp[2];

	db_if_exec_sql ( "BEGIN", 0, 0 );

	/*
	 * Put together our update or insert string.
	 *   At the same time, update or insert players and keyboard
	 */
	sprintf ( statement, "INSERT OR REPLACE INTO Input VALUES ( '%s', '%s', '%d', '%d'",
	            p_tmp[0]=util_replace_and_copy(name, "'", "''" ), 
	            p_tmp[1]=util_replace_and_copy(p_input->description, "'", "''" ), 
	            p_input->machine_type, p_input->mouse_speed ); 

	free ( p_tmp[0] );
	free ( p_tmp[1] );

	for ( i = 0; i < 4; ++i ) {
		sprintf ( tstatement, "SELECT Player%d FROM Input WHERE Name='%s'", i, name );
		input_write_player ( p_input->machine_type, tstatement, &p_input->players[i], &id );
		sprintf ( statement, "%s, %d", statement, id );
	}

	if ( p_input->machine_type != MACHINE_TYPE_5200 ) {
		sprintf ( tstatement, "SELECT KeyboardID FROM Input WHERE Name='%s'", name );
		input_write_keyboard ( tstatement, p_input->keyboard, &id );
		sprintf ( statement, "%s, %d", statement, id );

		sprintf ( tstatement, "SELECT StartKey FROM Input WHERE Name='%s'", name );
		input_write_common ( tstatement, &p_input->start_key, &id );
		sprintf ( statement, "%s, %d", statement, id );

		sprintf ( tstatement, "SELECT SelectKey FROM Input WHERE Name='%s'", name );
		input_write_common ( tstatement, &p_input->select_key, &id );
		sprintf ( statement, "%s, %d", statement, id );
	
		sprintf ( tstatement, "SELECT OptionKey FROM Input WHERE Name='%s'", name );
		input_write_common ( tstatement, &p_input->option_key, &id );
		sprintf ( statement, "%s, %d", statement, id );
	
		sprintf ( tstatement, "SELECT CtrlKey FROM Input WHERE Name='%s'", name );
		input_write_common ( tstatement, &p_input->ctrl_key, &id );
		sprintf ( statement, "%s, %d", statement, id );

		sprintf ( tstatement, "SELECT ShiftKey FROM Input WHERE Name='%s'", name );
		input_write_common ( tstatement, &p_input->shift_key, &id );
		sprintf ( statement, "%s, %d", statement, id );

		sprintf ( tstatement, "SELECT BreakKey FROM Input WHERE Name='%s'", name );
		input_write_common ( tstatement, &p_input->break_key, &id );
		sprintf ( statement, "%s, %d", statement, id );

		sprintf ( tstatement, "SELECT ResetKey FROM Input WHERE Name='%s'", name );
		input_write_common ( tstatement, &p_input->reset_key, &id );
		sprintf ( statement, "%s, %d )", statement, id );
	}
	else {
		strcat ( statement, ", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL )" );
	}

	status = db_if_exec_sql ( statement, 0, 0 );

	db_if_exec_sql ( "END", 0, 0 );

	return status;

} /* end input_save_profile */

/******************************************************************************
**  Function   :  input_find_common_callback                                            
**                                                                    
**  Objective  :  This function is called by sqlite in response to the query
**                                                   
**  Parameters :  ctrl      - pointer to common control struct
**                argc      - number of columns returned
**                argv      - array of strings containing data
**                azColName - array of strings containing column names
**
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_find_common_callback ( void *ctrl, int argc, char **argv, char **azColName ) {

	t_atari_common *p_ctrl = ctrl;
	int i;

	if ( argc < 6 )
		return -1;

	/*
	 * THIS DEPENDS ON MATCHING THE ORDER IN THE DATABASE!
	 */
	i = 0;
	if ( argv[++i] ) p_ctrl->device = atoi ( argv[i] );
	if ( argv[++i] ) p_ctrl->device_num = atoi ( argv[i] );
	if ( argv[++i] ) p_ctrl->part_type = atoi ( argv[i] );
	if ( argv[++i] ) p_ctrl->part_num = atoi ( argv[i] );
	if ( argv[++i] ) p_ctrl->direction = atoi ( argv[i] );

	return 0;

} /* end input_find_common_callback */

/******************************************************************************
**  Function   :  input_find_common                                            
**                                                                    
**  Objective  :  This function finds common contol parameters in the profile
**                                                   
**  Parameters :  id   - id of control to find in database table
**                ctrl - struct to store found values
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_find_common ( int id, t_atari_common *ctrl ) {

	int status;
	char statement[50];

	sprintf ( statement, "SELECT * FROM InputList WHERE InputID=%d", id );
	status = db_if_exec_sql ( statement, input_find_common_callback, ctrl );

	return 0;

} /* end input_find_common */

/******************************************************************************
**  Function   :  input_write_common                                            
**                                                                    
**  Objective  :  This function writes common contol parameters to the profile
**                                                   
**  Parameters :  statement - SQL to check for existence of this entry
**                ctrl      - struct containing values to write
**                id        - returns the id of this entry in the table
**                 
**  return     :  0 if success, otherwise failure
**
******************************************************************************/ 
int input_write_common ( char *statement, t_atari_common *ctrl, int *id ) {

	char values[256];

	*id = 0;

	sprintf ( values, "%d, %d, %d, %d, %d", ctrl->device, ctrl->device_num, 
	            ctrl->part_type, ctrl->part_num, ctrl->direction );

	db_if_insert_or_update ( statement, values, "InputList", id );

	return 0;

} /* end input_write_common */

/******************************************************************************
**  Function   :  input_get_key_string                                            
**                                                                    
**  Objective  :  This function gets the name of the key from the value
**                                                   
**  Parameters :  key  - input key number
**                name - name of key to return
**                 
**  return     :  keyname
******************************************************************************/ 
char * input_get_key_string ( int key, char *name ) {

	switch ( key ) {

		case A800_KEY_L:      return strcpy(name,"key_l"); break;
		case A800_KEY_J:      return strcpy(name,"key_j"); break;
		case A800_KEY_SEMI:   return strcpy(name,"key_semicolon"); break;
		case A800_KEY_K:      return strcpy(name,"key_k"); break;
		case A800_KEY_PLUS:   return strcpy(name,"key_plus"); break;
		case A800_KEY_STAR:   return strcpy(name,"key_star"); break;
		case A800_KEY_O:      return strcpy(name,"key_o"); break;
		case A800_KEY_P:      return strcpy(name,"key_p"); break;
		case A800_KEY_U:      return strcpy(name,"key_u"); break;
		case A800_KEY_RETURN: return strcpy(name,"key_return"); break;
		case A800_KEY_I:      return strcpy(name,"key_i"); break;
		case A800_KEY_MINUS:  return strcpy(name,"key_-"); break;
		case A800_KEY_EQUALS: return strcpy(name,"key_equals"); break;
		case A800_KEY_V:      return strcpy(name,"key_v"); break;
		case A800_KEY_HELP:   return strcpy(name,"key_help"); break;
		case A800_KEY_C:      return strcpy(name,"key_c"); break;
		case A800_KEY_B:      return strcpy(name,"key_b"); break;
		case A800_KEY_X:      return strcpy(name,"key_x"); break;
		case A800_KEY_Z:      return strcpy(name,"key_z"); break;
		case A800_KEY_4:      return strcpy(name,"key_4"); break;
		case A800_KEY_3:      return strcpy(name,"key_3"); break;
		case A800_KEY_6:      return strcpy(name,"key_6"); break;
		case A800_KEY_ESC:    return strcpy(name,"key_esc"); break;
		case A800_KEY_5:      return strcpy(name,"key_5"); break;
		case A800_KEY_2:      return strcpy(name,"key_2"); break;
		case A800_KEY_1:      return strcpy(name,"key_1"); break;
		case A800_KEY_COMMA:  return strcpy(name,"key_comma"); break;
		case A800_KEY_SPACE:  return strcpy(name,"key_space"); break;
		case A800_KEY_PERIOD: return strcpy(name,"key_."); break;
		case A800_KEY_N:      return strcpy(name,"key_n"); break;
		case A800_KEY_M:      return strcpy(name,"key_m"); break;
		case A800_KEY_SLASH:  return strcpy(name,"key_slash"); break;
		case A800_KEY_FUJI:   return strcpy(name,""); break;
		case A800_KEY_R:      return strcpy(name,"key_r"); break;
		case A800_KEY_E:      return strcpy(name,"key_e"); break;
		case A800_KEY_Y:      return strcpy(name,"key_y"); break;
		case A800_KEY_TAB:    return strcpy(name,"key_tab"); break;
		case A800_KEY_T:      return strcpy(name,"key_t"); break;
		case A800_KEY_W:      return strcpy(name,"key_w"); break;
		case A800_KEY_Q:      return strcpy(name,"key_q"); break;
		case A800_KEY_9:      return strcpy(name,"key_9"); break;
		case A800_KEY_0:      return strcpy(name,"key_0"); break;
		case A800_KEY_7:      return strcpy(name,"key_7"); break;
		case A800_KEY_BKSP:   return strcpy(name,"key_bksp"); break;
		case A800_KEY_8:      return strcpy(name,"key_8"); break;
		case A800_KEY_LESST:  return strcpy(name,"key_lessthan"); break;
		case A800_KEY_MORET:  return strcpy(name,"key_greaterthan"); break;
		case A800_KEY_F:      return strcpy(name,"key_f"); break;
		case A800_KEY_H:      return strcpy(name,"key_h"); break;
		case A800_KEY_D:      return strcpy(name,"key_d"); break;
		case A800_KEY_CAPS:   return strcpy(name,"key_caps"); break;
		case A800_KEY_G:      return strcpy(name,"key_g"); break;
		case A800_KEY_S:      return strcpy(name,"key_s"); break;
		case A800_KEY_A:      return strcpy(name,"key_a"); break;
		default: return strcpy(name,""); break;

	 } /* end switch key */

} /* end input_get_key_string */

/************************************************************************
 *
 *  Assign PC inputs to Atari Inputs and setup input functions
 *
 ************************************************************************/
/******************************************************************************
**  Function   :  input_preload_controls
**                            
**  Objective  :  This function preloads all control functions with do nothing
**                frame
**
**  Parameters :  NONE
**                                                
**  return     :  0
**      
******************************************************************************/
int input_preload_controls ( void ) {
	
	int i, j, k;

	for ( i = 0; i < MAX_PC_KEY; ++i ) {
		g_pc_input.keyboard[i].key_up_function = input_do_nothing;
		g_pc_input.keyboard[i].key_down_function = input_do_nothing;
		g_pc_input.keyboard[i].player = 0;
		g_pc_input.keyboard[i].atari_key = 0;
		g_pc_input.keyboard[i].direction = 0;
	}
	for ( i = 0; i < UI_KEY_MAX; ++i ) {
		g_pc_input.ui_keyboard[i].key_up_function = input_do_nothing;
		g_pc_input.ui_keyboard[i].key_down_function = input_do_nothing;
		g_pc_input.ui_keyboard[i].modifier = 0;
		g_pc_input.ui_keyboard[i].key = 0;
	}

	for ( i = 0; i < MAX_PC_DEVICE; ++i ) {
		for ( j = 0; j < MAX_PC_PART; ++j ) {

			for ( k = 0; k < MAX_DIR; ++k ) {
				g_pc_input.pov_hat[i][j].hat_pressed_function[k] = input_do_nothing;
				g_pc_input.pov_hat[i][j].hat_not_pressed_function[k] = input_do_nothing;
			}
			g_pc_input.pov_hat[i][j].player = 0;

			g_pc_input.analog_axis[i][j].axis_pos_function = input_do_nothing;
			g_pc_input.analog_axis[i][j].axis_neg_function = input_do_nothing;
			g_pc_input.analog_axis[i][j].player = 0;

			g_pc_input.trackball[i][j].x_axis.axis_pos_function = input_do_nothing;
			g_pc_input.trackball[i][j].x_axis.axis_neg_function = input_do_nothing;
			g_pc_input.trackball[i][j].y_axis.axis_pos_function = input_do_nothing;
			g_pc_input.trackball[i][j].y_axis.axis_neg_function = input_do_nothing;
			g_pc_input.trackball[i][j].x_axis.player = 0;
			g_pc_input.trackball[i][j].y_axis.player = 0;

			g_pc_input.button[i][j].button_up_function = input_do_nothing;
			g_pc_input.button[i][j].button_down_function = input_do_nothing;
			g_pc_input.button[i][j].player = 0;
			g_pc_input.button[i][j].atari_key = 0;
			g_pc_input.button[i][j].direction = 0;
		}
	}

	g_pc_input.mouse.x_axis.axis_pos_function = input_do_nothing;
	g_pc_input.mouse.x_axis.axis_neg_function = input_do_nothing;
	g_pc_input.mouse.y_axis.axis_pos_function = input_do_nothing;
	g_pc_input.mouse.y_axis.axis_neg_function = input_do_nothing;
	g_pc_input.mouse.x_axis.player = 0;
	g_pc_input.mouse.y_axis.player = 0;
	g_pc_input.mouse.speed = 0;
	g_pc_input.mouse.x_axis.direction_pos = 0;
	g_pc_input.mouse.y_axis.direction_pos = 0;
	g_pc_input.mouse.x_axis.direction_neg = 0;
	g_pc_input.mouse.y_axis.direction_neg = 0;

	for ( i = 0; i < 4; ++i ) {
		g_pc_input.mouse_button[i].button_up_function = input_do_nothing;
		g_pc_input.mouse_button[i].button_down_function = input_do_nothing;
		g_pc_input.mouse_button[i].player = 0;
		g_pc_input.mouse_button[i].atari_key = 0;
		g_pc_input.mouse_button[i].direction = 0;
	}

	return 0;

} /* end input_preload_controls */

/******************************************************************************
**  Function   :  input_setup_stick
**                            
**  Objective  :  This function sets up controller options for the emulated
**                stick.  For 800 paddles, direction is reversed
**
**  Parameters :  player - which controller to setup for
**                stick  - player array (5200 stick or 800 paddle)
**                                                
**  return     :  0
**      
******************************************************************************/
int input_setup_stick ( int player, t_atari_stick *stick ) {

	int j, tmp_dir, tmp_device_num, tmp_part_num;
	int i = player;
	t_atari_common *ctrl = stick->direction;
	t_pc_analog_axis *tmp_axis;

	/*
	 * Check each direction of Atari stick
	 */
	for ( j = DIR_LEFT; j < MAX_DIR; ++j ) {

		/*
		 * Check for Keyboard
		 */
		if ( ctrl[j].device == DEV_KEYBOARD ) {
			if ( g_input.machine_type == MACHINE_TYPE_5200 )
				g_pc_input.keyboard[ctrl[j].part_num].direction = j;
			else
				g_pc_input.keyboard[ctrl[j].part_num].direction = (j ^ 1);
			tmp_part_num = ctrl[j].part_num; 
			if ( stick->simulate_analog ) {
					g_pc_input.keyboard[tmp_part_num].key_up_function = input_key_up_stick_sa;
					g_pc_input.keyboard[tmp_part_num].key_down_function = input_key_down_stick_sa;
			}
			else {
					g_pc_input.keyboard[tmp_part_num].key_up_function = input_key_up_stick;
					g_pc_input.keyboard[tmp_part_num].key_down_function = input_key_down_stick;
			}
			g_pc_input.keyboard[tmp_part_num].player = i;
			g_pc_input.keyboard[tmp_part_num].atari_key = 0;

		} /* end if keyboard */

		/*
		 * Check for Joystick
		 */
		tmp_device_num = ctrl[j].device_num; 
		tmp_part_num = ctrl[j].part_num; 
		tmp_dir = ctrl[j].direction; 
		if ( ctrl[j].device == DEV_JOYSTICK ) {

			if ( ctrl[j].part_type == PART_TYPE_AXIS ) {
				if ( stick[i].simulate_analog ) {
					if ( ctrl[j].direction == DIR_PLUS )
						g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_pos_function = input_joystick_pots_sa;
					else
						g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_neg_function = input_joystick_pots_sa;
				}
				else {
					if ( ctrl[j].direction == DIR_PLUS )
						g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_pos_function = input_joystick_pots;
					else
						g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_neg_function = input_joystick_pots;
				}

				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].player = i;
				if ( ctrl[j].direction == DIR_PLUS ) {
					if ( g_input.machine_type == MACHINE_TYPE_5200 )
						g_pc_input.analog_axis[tmp_device_num][tmp_part_num].direction_pos = j;
					else
						g_pc_input.analog_axis[tmp_device_num][tmp_part_num].direction_pos = (j ^ 1);
				}
				else {
					if ( g_input.machine_type == MACHINE_TYPE_5200 )
						g_pc_input.analog_axis[tmp_device_num][tmp_part_num].direction_neg = j;
					else
						g_pc_input.analog_axis[tmp_device_num][tmp_part_num].direction_neg = (j ^ 1);
				}
			}

			if ( ctrl[j].part_type == PART_TYPE_BALL ) {

				if ( g_input.machine_type == MACHINE_TYPE_5200 )
					tmp_dir = j;
				else
					tmp_dir = (j ^ 1);

				if ( !ctrl[j].part_num )
					tmp_axis = &g_pc_input.trackball[tmp_device_num][tmp_part_num].x_axis;
				else
					tmp_axis = &g_pc_input.trackball[tmp_device_num][tmp_part_num].y_axis;

				if ( ctrl[j].direction == DIR_PLUS ) {
					tmp_axis->axis_pos_function = input_mouse_axis;
					tmp_axis->direction_pos = tmp_dir;
				}
				else {
					tmp_axis->axis_neg_function = input_mouse_axis;
					tmp_axis->direction_neg = tmp_dir;
				}
				tmp_axis->player = i;
				g_pc_input.trackball[tmp_device_num][tmp_part_num].speed = g_input.mouse_speed;
			}

			if ( ctrl[j].part_type == PART_TYPE_HAT ) {

				if ( stick->simulate_analog ) {
					g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_not_pressed_function[tmp_dir] = input_key_up_stick_sa;
					g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_pressed_function[tmp_dir] = input_key_down_stick_sa;
				}
				else {
					g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_not_pressed_function[tmp_dir] = input_key_up_stick;
					g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_pressed_function[tmp_dir] = input_key_down_stick;
				}
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].player = i;
				if ( g_input.machine_type == MACHINE_TYPE_5200 )
					g_pc_input.pov_hat[tmp_device_num][tmp_part_num].direction[tmp_dir] = j;
				else
					g_pc_input.pov_hat[tmp_device_num][tmp_part_num].direction[tmp_dir] = (j ^ 1);

			} /* end if hat on joystick */

			if ( ctrl[j].part_type == PART_TYPE_BUTTON ) {

				if ( stick[i].simulate_analog ) {
					g_pc_input.button[tmp_device_num][tmp_part_num].button_up_function = input_key_up_stick_sa;
					g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = input_key_down_stick_sa;
				}
				else {
					g_pc_input.button[tmp_device_num][tmp_part_num].button_up_function = input_key_up_stick;
					g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = input_key_down_stick;
				}
				g_pc_input.button[tmp_device_num][tmp_part_num].player = i;
				if ( g_input.machine_type == MACHINE_TYPE_5200 )
					g_pc_input.button[tmp_device_num][tmp_part_num].direction = j;
				else
					g_pc_input.button[tmp_device_num][tmp_part_num].direction = (j ^ 1);

			} /* end if button on joystick */

		} /* end if joystick */

		if ( ctrl[j].device == DEV_MOUSE ) {

			if ( g_input.machine_type == MACHINE_TYPE_5200 )
				tmp_dir = j;
			else
				tmp_dir = (j ^ 1);

			if ( !ctrl[j].part_num )
				tmp_axis = &g_pc_input.mouse.x_axis;
			else
				tmp_axis = &g_pc_input.mouse.y_axis;

			if ( ctrl[j].direction == DIR_PLUS ) {
				tmp_axis->axis_pos_function = input_mouse_axis;
				tmp_axis->direction_pos = tmp_dir;
			}
			else {
				tmp_axis->axis_neg_function = input_mouse_axis;
				tmp_axis->direction_neg = tmp_dir;
			}
			tmp_axis->player = i;
			g_pc_input.mouse.speed = g_input.mouse_speed;

		} /* end if mouse */

	} /* end for stick direction */

	return 0;

} /* input_setup_stick */

/******************************************************************************
**  Function   :  input_setup_digital_stick
**                            
**  Objective  :  This function sets up controller options for the emulated
**                stick (for the 800 digital stick)
**
**  Parameters :  player - which controller to setup for
**                stick  - player array (5200 stick or 800 paddle)
**                                                
**  return     :  0
**      
******************************************************************************/
int input_setup_digital_stick ( int player, t_atari_stick *stick ) {

	int j, tmp_dir, tmp_device_num, tmp_part_num;
	int i = player;
	t_atari_common *ctrl = stick->direction;

	/*
	 * Check each direction of Atari stick
	 */
	for ( j = DIR_LEFT; j < MAX_DIR; ++j ) {

		/*
		 * Check for Keyboard
		 */
		if ( ctrl[j].device == DEV_KEYBOARD ) {
			g_pc_input.keyboard[ctrl[j].part_num].direction = j;
			g_pc_input.keyboard[ctrl[j].part_num].key_up_function = input_key_up_digital_stick;
			g_pc_input.keyboard[ctrl[j].part_num].key_down_function = input_key_down_digital_stick;
			g_pc_input.keyboard[ctrl[j].part_num].player = i;
			g_pc_input.keyboard[ctrl[j].part_num].atari_key = 0;

		} /* end if keyboard */

		/*
		 * Check for Joystick
		 */
		tmp_device_num = ctrl[j].device_num; 
		tmp_part_num = ctrl[j].part_num; 
		tmp_dir = ctrl[j].direction; 
		if ( ctrl[j].device == DEV_JOYSTICK ) {

			if ( ctrl[j].part_type == PART_TYPE_AXIS ) {
				if ( ctrl[j].direction == DIR_PLUS )
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_pos_function = input_axis_digital_stick;
				else
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_neg_function = input_axis_digital_stick;

				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].player = i;
				if ( ctrl[j].direction == DIR_PLUS )
					g_pc_input.analog_axis[tmp_device_num][ctrl[j].part_num].direction_pos = j;
				else
					g_pc_input.analog_axis[tmp_device_num][ctrl[j].part_num].direction_neg = j;
			} /* end if axis on joystick */

			if ( ctrl[j].part_type == PART_TYPE_HAT ) {
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_not_pressed_function[tmp_dir] = input_key_up_digital_stick;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_pressed_function[tmp_dir] = input_key_down_digital_stick;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].player = i;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].direction[tmp_dir] = j;
			} /* end if hat on joystick */

			if ( ctrl[j].part_type == PART_TYPE_BUTTON ) {
				g_pc_input.button[tmp_device_num][tmp_part_num].button_up_function = input_key_up_digital_stick;
				g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = input_key_down_digital_stick;
				g_pc_input.button[tmp_device_num][tmp_part_num].player = i;
				g_pc_input.button[tmp_device_num][tmp_part_num].direction = j;
			} /* end if button on joystick */

		} /* end if joystick */

	} /* end for stick direction */

	return 0;

} /* input_setup_digital_stick */

/******************************************************************************
**  Function   :  input_setup_trackball
**                            
**  Objective  :  This function sets up controller options for the emulated
**                trackball.
**
**  Parameters :  player - which controller to setup for
**                stick  - player array (5200 trackball)
**                                                
**  return     :  0
**      
******************************************************************************/
int input_setup_trackball ( int player, t_atari_stick *stick ) {

	int j, tmp_dir, tmp_device_num, tmp_part_num;
	int i = player;
	t_atari_common *ctrl = stick->direction;
	t_pc_analog_axis *tmp_axis;

	/*
	 * Check each direction of Atari stick
	 */
	for ( j = DIR_LEFT; j < MAX_DIR; ++j ) {

		/*
		 * Check for Keyboard
		 */
		if ( ctrl[j].device == DEV_KEYBOARD ) {
			g_pc_input.keyboard[ctrl[j].part_num].direction = j;
			tmp_part_num = ctrl[j].part_num; 
			g_pc_input.keyboard[tmp_part_num].key_up_function = input_key_up_stick;
			g_pc_input.keyboard[tmp_part_num].key_down_function = input_key_down_stick;
			g_pc_input.keyboard[tmp_part_num].player = i;
			g_pc_input.keyboard[tmp_part_num].atari_key = 0;

		} /* end if keyboard */

		/*
		 * Check for Joystick
		 */
		tmp_device_num = ctrl[j].device_num; 
		tmp_part_num = ctrl[j].part_num; 
		tmp_dir = ctrl[j].direction; 
		if ( ctrl[j].device == DEV_JOYSTICK ) {

			if ( ctrl[j].part_type == PART_TYPE_AXIS ) {
				if ( ctrl[j].direction == DIR_PLUS )
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_pos_function = input_joystick_pots;
				else
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_neg_function = input_joystick_pots;

				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].player = i;
				if ( ctrl[j].direction == DIR_PLUS ) {
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].direction_pos = j;
				}
				else {
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].direction_neg = j;
				}
			}

			if ( ctrl[j].part_type == PART_TYPE_BALL ) {
				if ( g_input.machine_type == MACHINE_TYPE_5200 )
					tmp_dir = j;
				else
					tmp_dir = (j ^ 1);

				if ( !ctrl[j].part_num )
					tmp_axis = &g_pc_input.trackball[tmp_device_num][tmp_part_num].x_axis;
				else
					tmp_axis = &g_pc_input.trackball[tmp_device_num][tmp_part_num].y_axis;

				if ( ctrl[j].direction == DIR_PLUS ) {
					tmp_axis->axis_pos_function = input_mouse_vel_axis;
					tmp_axis->direction_pos = tmp_dir;
				}
				else {
					tmp_axis->axis_neg_function = input_mouse_vel_axis;
					tmp_axis->direction_neg = tmp_dir;
				}
				tmp_axis->player = i;
			}

			if ( ctrl[j].part_type == PART_TYPE_HAT ) {

				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_not_pressed_function[tmp_dir] = input_key_up_stick;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_pressed_function[tmp_dir] = input_key_down_stick;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].player = i;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].direction[tmp_dir] = j;

			} /* end if hat on joystick */

			if ( ctrl[j].part_type == PART_TYPE_BUTTON ) {

				g_pc_input.button[tmp_device_num][tmp_part_num].button_up_function = input_key_up_stick;
				g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = input_key_down_stick;
				g_pc_input.button[tmp_device_num][tmp_part_num].player = i;
				g_pc_input.button[tmp_device_num][tmp_part_num].direction = j;

			} /* end if button on joystick */

		} /* end if joystick */

		if ( ctrl[j].device == DEV_MOUSE ) {

			if ( g_input.machine_type == MACHINE_TYPE_5200 )
				tmp_dir = j;
			else
				tmp_dir = (j ^ 1);

			if ( !ctrl[j].part_num )
				tmp_axis = &g_pc_input.mouse.x_axis;
			else
				tmp_axis = &g_pc_input.mouse.y_axis;

			if ( ctrl[j].direction == DIR_PLUS ) {
				tmp_axis->axis_pos_function = input_mouse_vel_axis;
				tmp_axis->direction_pos = tmp_dir;
			}
			else {
				tmp_axis->axis_neg_function = input_mouse_vel_axis;
				tmp_axis->direction_neg = tmp_dir;
			}
			tmp_axis->player = i;
			g_pc_input.mouse.speed = g_input.mouse_speed;

		} /* end if mouse */

	} /* end for stick direction */

	return 0;

} /* input_setup_trackball */

/******************************************************************************
**  Function   :  input_setup_keypad
**                            
**  Objective  :  This function sets up the emulated controller keypad
**                stick
**
**  Parameters :  player - which controller to setup for
**                ctrl   - ctrl struct array (either player keypad for 5200 or
**                           keyboard for 800)
**                                                
**  return     :  0
**      
******************************************************************************/
int input_setup_keypad ( int player, t_atari_common *ctrl ) {

	int j, tmp_part_num, tmp_device_num, tmp_dir;
	int i = player;
	int stop;

	if ( g_input.machine_type == MACHINE_TYPE_5200 )
		stop = 16;
	else
		stop = 0x40;

	for ( j = 0; j < stop; j++ ) {

		tmp_part_num = ctrl[j].part_num;
		tmp_device_num = ctrl[j].device_num;
		tmp_dir = ctrl[j].direction;

		if ( ctrl[j].device == DEV_KEYBOARD ) {
			g_pc_input.keyboard[tmp_part_num].key_up_function = input_keypad_up;
			g_pc_input.keyboard[tmp_part_num].key_down_function = input_keypad_down;
			g_pc_input.keyboard[tmp_part_num].atari_key = j;
			g_pc_input.keyboard[tmp_part_num].player = i;

			if ( (g_input.machine_type !=  MACHINE_TYPE_5200) && 
			     (pc_check_for_lr_keys(tmp_part_num)) ) {
				tmp_part_num = pc_check_for_lr_keys(tmp_part_num);
				g_pc_input.keyboard[tmp_part_num].key_up_function = input_keypad_up;
				g_pc_input.keyboard[tmp_part_num].key_down_function = input_keypad_down;
				g_pc_input.keyboard[tmp_part_num].atari_key = j;
				g_pc_input.keyboard[tmp_part_num].player = i;
			}
		}

		if ( ctrl[j].device == DEV_JOYSTICK ) {
			if ( ctrl[j].part_type == PART_TYPE_HAT ) {
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_not_pressed_function[tmp_dir] = input_keypad_up;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_pressed_function[tmp_dir] = input_keypad_down;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].atari_key = j;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].player = i;
			}
			else if ( ctrl[j].part_type == PART_TYPE_AXIS ) {
				if ( tmp_dir == DIR_PLUS )
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_pos_function = input_joyaxis_keypad;
				else
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_neg_function = input_joyaxis_keypad;
				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].player = i;
			}
			else {
				g_pc_input.button[tmp_device_num][tmp_part_num].button_up_function = input_keypad_up;
				g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = input_keypad_down;
				g_pc_input.button[tmp_device_num][tmp_part_num].atari_key = j;
				g_pc_input.button[tmp_device_num][tmp_part_num].player = i;
			}
		}

		if ( ctrl[j].device == DEV_MOUSE ) {
			g_pc_input.mouse_button[tmp_part_num].button_up_function = input_keypad_up;
			g_pc_input.mouse_button[tmp_part_num].button_down_function = input_keypad_down;
			g_pc_input.mouse_button[tmp_part_num].atari_key = j;
			g_pc_input.mouse_button[tmp_part_num].player = i;
		}
	}

	return 0;

} /* input_setup_keypad */

/******************************************************************************
**  Function   :  input_setup_800keypad
**                            
**  Objective  :  This function sets up the emulated controller 800 keypad
**
**  Parameters :  player - which controller to setup for
**                                                
**  return     :  0
**      
******************************************************************************/
int input_setup_800keypad ( int player ) {

	int j, tmp_part_num, tmp_device_num, tmp_dir;
	int i = player;
	t_atari_common *ctrl = g_input.players[player].keypad;

	for ( j = 0; j < 0x40; j++ ) {

		tmp_part_num = ctrl[j].part_num;
		tmp_device_num = ctrl[j].device_num;
		tmp_dir = ctrl[j].direction;

		if ( ctrl[j].device == DEV_KEYBOARD ) {
			g_pc_input.keyboard[tmp_part_num].key_up_function = input_800keypad_up;
			g_pc_input.keyboard[tmp_part_num].key_down_function = input_800keypad_down;
			g_pc_input.keyboard[tmp_part_num].atari_key = j;
			g_pc_input.keyboard[tmp_part_num].player = i;
		}

		if ( ctrl[j].device == DEV_JOYSTICK ) {
			if ( ctrl[j].part_type == PART_TYPE_HAT ) {
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_not_pressed_function[tmp_dir] = input_800keypad_up;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_pressed_function[tmp_dir] = input_800keypad_down;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].atari_key = j;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].player = i;
			}
			else if ( ctrl[j].part_type == PART_TYPE_AXIS ) {
				if ( tmp_dir == DIR_PLUS )
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_pos_function = input_joyaxis_800keypad;
				else
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_neg_function = input_joyaxis_800keypad;
				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].player = i;
			}
			else {
				g_pc_input.button[tmp_device_num][tmp_part_num].button_up_function = input_800keypad_up;
				g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = input_800keypad_down;
				g_pc_input.button[tmp_device_num][tmp_part_num].atari_key = j;
				g_pc_input.button[tmp_device_num][tmp_part_num].player = i;
			}
		}

		if ( ctrl[j].device == DEV_MOUSE ) {
			g_pc_input.mouse_button[tmp_part_num].button_up_function = input_800keypad_up;
			g_pc_input.mouse_button[tmp_part_num].button_down_function = input_800keypad_down;
			g_pc_input.mouse_button[tmp_part_num].atari_key = j;
			g_pc_input.mouse_button[tmp_part_num].player = i;
		}
	}

	return 0;

} /* input_setup_800keypad */

/******************************************************************************
**  Function   :  input_setup_modkeys
**                            
**  Objective  :  This function sets up the emulated modifier keys on the 800
**                keyboard (shift, control, break - well, ok break is not a
**                modifier but it fits here!)
**
**  Parameters :  NONE 
**                
**  return     :  0
**      
******************************************************************************/
int input_setup_modkeys ( void ) {

	int j, tmp_part_num, tmp_device_num, tmp_dir;
	t_atari_common *ctrl;

	for ( j = 0; j < 4; j++ ) {

		switch ( j ) {
			case 0: ctrl = &g_input.shift_key; break;
			case 1: ctrl = &g_input.ctrl_key; break;
			case 2: ctrl = &g_input.break_key; break;
			case 3: ctrl = &g_input.reset_key; break;
			default: ctrl = &g_input.shift_key; break;
		}

		tmp_part_num = ctrl->part_num;
		tmp_device_num = ctrl->device_num;
		tmp_dir = ctrl->direction;

		if ( ctrl->device == DEV_KEYBOARD ) {
			g_pc_input.keyboard[tmp_part_num].key_up_function = input_modkey_up;
			g_pc_input.keyboard[tmp_part_num].key_down_function = input_modkey_down;
			g_pc_input.keyboard[tmp_part_num].atari_key = j;
			g_pc_input.keyboard[tmp_part_num].player = 0;
		}

		if ( ctrl->device == DEV_JOYSTICK ) {
			if ( ctrl->part_type == PART_TYPE_HAT ) {
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_not_pressed_function[tmp_dir] = input_modkey_up;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_pressed_function[tmp_dir] = input_modkey_down;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].atari_key = j;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].player = 0;
			}
			else if ( ctrl->part_type == PART_TYPE_AXIS ) {
				if ( tmp_dir == DIR_PLUS )
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_pos_function = input_joyaxis_modkey;
				else
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_neg_function = input_joyaxis_modkey;
				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].player = 0;
			}
			else {
				g_pc_input.button[tmp_device_num][tmp_part_num].button_up_function = input_modkey_up;
				g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = input_modkey_down;
				g_pc_input.button[tmp_device_num][tmp_part_num].atari_key = j;
				g_pc_input.button[tmp_device_num][tmp_part_num].player = 0;
			}
		}

		if ( ctrl->device == DEV_MOUSE ) {
			g_pc_input.mouse_button[tmp_part_num].button_up_function = input_modkey_up;
			g_pc_input.mouse_button[tmp_part_num].button_down_function = input_modkey_down;
			g_pc_input.mouse_button[tmp_part_num].atari_key = j;
			g_pc_input.mouse_button[tmp_part_num].player = 0;
		}
	}

	return 0;

} /* input_setup_modkeys */

/******************************************************************************
**  Function   :  input_setup_switch
**                            
**  Objective  :  This function sets up the emulated switches on the 800
**                keyboard (start, select, option)
**
**  Parameters :  NONE 
**                
**  return     :  0
**      
******************************************************************************/
int input_setup_switch ( void ) {

	int j, tmp_part_num, tmp_device_num, tmp_dir;
	t_atari_common *ctrl;

	for ( j = 0; j < 3; j++ ) {

		switch ( j ) {
			case 0: ctrl = &g_input.start_key; break;
			case 1: ctrl = &g_input.select_key; break;
			case 2: ctrl = &g_input.option_key; break;
			default: ctrl = &g_input.start_key; break;
		}

		tmp_part_num = ctrl->part_num;
		tmp_device_num = ctrl->device_num;
		tmp_dir = ctrl->direction;

		if ( ctrl->device == DEV_KEYBOARD ) {
			g_pc_input.keyboard[tmp_part_num].key_up_function = input_switch_up;
			g_pc_input.keyboard[tmp_part_num].key_down_function = input_switch_down;
			g_pc_input.keyboard[tmp_part_num].atari_key = 1<<j;
			g_pc_input.keyboard[tmp_part_num].player = 0;
		}

		if ( ctrl->device == DEV_JOYSTICK ) {
			if ( ctrl->part_type == PART_TYPE_HAT ) {
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_not_pressed_function[tmp_dir] = input_switch_up;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_pressed_function[tmp_dir] = input_switch_down;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].atari_key = 1<<j;
				g_pc_input.pov_hat[tmp_device_num][tmp_part_num].player = 0;
			}
			else if ( ctrl->part_type == PART_TYPE_AXIS ) {
				if ( tmp_dir == DIR_PLUS )
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_pos_function = input_joyaxis_switch;
				else
					g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_neg_function = input_joyaxis_switch;
				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].player = 0;
			}
			else {
				g_pc_input.button[tmp_device_num][tmp_part_num].button_up_function = input_switch_up;
				g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = input_switch_down;
				g_pc_input.button[tmp_device_num][tmp_part_num].atari_key = 1<<j;
				g_pc_input.button[tmp_device_num][tmp_part_num].player = 0;
			}
		}

		if ( ctrl->device == DEV_MOUSE ) {
			g_pc_input.mouse_button[tmp_part_num].button_up_function = input_switch_up;
			g_pc_input.mouse_button[tmp_part_num].button_down_function = input_switch_down;
			g_pc_input.mouse_button[tmp_part_num].atari_key = 1<<j;
			g_pc_input.mouse_button[tmp_part_num].player = 0;
		}
	}

	return 0;

} /* input_setup_switch */

/******************************************************************************
**  Function   :  input_setup_buttons
**                            
**  Objective  :  This function sets up the emulated controller buttons
**
**  Parameters :  player - which controller to setup for
**                                                
**  return     :  0
**      
******************************************************************************/
int input_setup_buttons ( int player ) {

	int tmp_part_num, tmp_device_num, tmp_dir;
	int i = player;
	t_atari_common *ctrl;

	/***************************************
	 * Top Button
	 **************************************/
	ctrl = &g_input.players[i].top_button;
	tmp_device_num = ctrl->device_num;
	tmp_part_num = ctrl->part_num;
	tmp_dir = ctrl->direction;

	/*
	 * Check for Keyboard
	 */
	if (ctrl->device == DEV_KEYBOARD ) {
		g_pc_input.keyboard[tmp_part_num].key_up_function = input_top_button_up;
		g_pc_input.keyboard[tmp_part_num].key_down_function = input_top_button_down;
		g_pc_input.keyboard[tmp_part_num].player = i;
	}

	/*
	 * Check for Joystick
	 */
	if ( ctrl->device == DEV_JOYSTICK ) {

		if ( ctrl->part_type == PART_TYPE_HAT ) {
			g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_not_pressed_function[tmp_dir] = input_top_button_up;
			g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_pressed_function[tmp_dir] = input_top_button_down;
			g_pc_input.pov_hat[tmp_device_num][tmp_part_num].player = i;
		}
		else if ( ctrl->part_type == PART_TYPE_AXIS ) {
			if ( tmp_dir == DIR_PLUS )
				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_pos_function = input_joyaxis_top_button;
			else
				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_neg_function = input_joyaxis_top_button;
			g_pc_input.analog_axis[tmp_device_num][tmp_part_num].player = i;
		}
		else {
			g_pc_input.button[tmp_device_num][tmp_part_num].button_up_function = input_top_button_up;
			g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = input_top_button_down;
			g_pc_input.button[tmp_device_num][tmp_part_num].player = i;
		}
	}

	/*
	 * Check for Mouse
	 */
	if ( ctrl->device == DEV_MOUSE ) {
		g_pc_input.mouse_button[tmp_part_num].button_up_function = input_top_button_up;
		g_pc_input.mouse_button[tmp_part_num].button_down_function = input_top_button_down;
		g_pc_input.mouse_button[tmp_part_num].player = i;
	}

	/***************************************
	 * Bottom Button
	 **************************************/
	ctrl = &g_input.players[i].bottom_button;
	tmp_device_num = ctrl->device_num;
	tmp_part_num = ctrl->part_num;
	tmp_dir = ctrl->direction;

	/*
	 * Check for Keyboard
	 */
	if ( ctrl->device == DEV_KEYBOARD ) {
		g_pc_input.keyboard[tmp_part_num].key_up_function = gtia_bottom_button_up;
		g_pc_input.keyboard[tmp_part_num].key_down_function = gtia_bottom_button_down;
		g_pc_input.keyboard[tmp_part_num].player = i;
	}
	if ( ctrl->device == DEV_JOYSTICK ) {
		if ( ctrl->part_type == PART_TYPE_HAT ) {
			g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_not_pressed_function[tmp_dir] = gtia_bottom_button_up;
			g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_pressed_function[tmp_dir] = gtia_bottom_button_down;
			g_pc_input.pov_hat[tmp_device_num][tmp_part_num].player = i;
		}
		else if ( ctrl->part_type == PART_TYPE_AXIS ) {
			if ( tmp_dir == DIR_PLUS )
				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_pos_function = input_joyaxis_bottom_button;
			else
				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_neg_function = input_joyaxis_bottom_button;
			g_pc_input.analog_axis[tmp_device_num][tmp_part_num].player = i;
		}
		else {
			g_pc_input.button[tmp_device_num][tmp_part_num].button_up_function = gtia_bottom_button_up;
			g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = gtia_bottom_button_down;
			g_pc_input.button[tmp_device_num][tmp_part_num].player = i;
		}
	}

	/*
	 * Check for Mouse
	 */
	if ( ctrl->device == DEV_MOUSE ) {
		g_pc_input.mouse_button[tmp_part_num].button_up_function = gtia_bottom_button_up;
		g_pc_input.mouse_button[tmp_part_num].button_down_function = gtia_bottom_button_down;
		g_pc_input.mouse_button[tmp_part_num].player = i;
	}

	return 0;

} /* input_setup_buttons */

/******************************************************************************
**  Function   :  input_setup_paddle_button
**                            
**  Objective  :  This function sets up controller options for the emulated
**                paddle button (for the 800)
**
**  Parameters :  player - which controller to setup for
**                                                
**  return     :  0
**      
******************************************************************************/
int input_setup_paddle_button ( int player ) {

	int j, tmp_dir, tmp_device_num, tmp_part_num;
	int i;
	t_atari_common *ctrl = &g_input.players[player].bottom_button;

	/*
	 * Similiar to reading the stick via PIA.
	 *   P0 is the same as P0 left
	 *   P1 is the same as P0 right
	 *   P2 is the same as P1 left
	 *   P3 is the same as P1 right
	 */
	switch ( player ) {
		case 0: i = 0; j = DIR_LEFT; break;
		case 1: i = 0; j = DIR_RIGHT; break;
		case 2: i = 1; j = DIR_LEFT; break;
		case 3: i = 1; j = DIR_RIGHT; break;
		default: i = 0; j = DIR_LEFT; break;
	}

	/*
	 * Check for Keyboard
	 */
	if ( ctrl->device == DEV_KEYBOARD ) {
		g_pc_input.keyboard[ctrl->part_num].direction = j;
		g_pc_input.keyboard[ctrl->part_num].key_up_function = input_key_up_digital_stick;
		g_pc_input.keyboard[ctrl->part_num].key_down_function = input_key_down_digital_stick;
		g_pc_input.keyboard[ctrl->part_num].player = i;
		g_pc_input.keyboard[ctrl->part_num].atari_key = 0;

	} /* end if keyboard */

	/*
	 * Check for Joystick
	 */
	tmp_device_num = ctrl->device_num; 
	tmp_part_num = ctrl->part_num; 
	tmp_dir = ctrl->direction; 
	if ( ctrl->device == DEV_JOYSTICK ) {

		if ( ctrl->part_type == PART_TYPE_AXIS ) {
			if ( ctrl->direction == DIR_PLUS )
				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_pos_function = input_axis_digital_stick;
			else
				g_pc_input.analog_axis[tmp_device_num][tmp_part_num].axis_neg_function = input_axis_digital_stick;

			g_pc_input.analog_axis[tmp_device_num][tmp_part_num].player = i;
			if ( ctrl->direction == DIR_PLUS )
				g_pc_input.analog_axis[tmp_device_num][ctrl->part_num].direction_pos = j;
			else
				g_pc_input.analog_axis[tmp_device_num][ctrl->part_num].direction_neg = j;
		} /* end if axis on joystick */

		if ( ctrl->part_type == PART_TYPE_HAT ) {
			g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_not_pressed_function[tmp_dir] = input_key_up_digital_stick;
			g_pc_input.pov_hat[tmp_device_num][tmp_part_num].hat_pressed_function[tmp_dir] = input_key_down_digital_stick;
			g_pc_input.pov_hat[tmp_device_num][tmp_part_num].player = i;
			g_pc_input.pov_hat[tmp_device_num][tmp_part_num].direction[tmp_dir] = j;
		} /* end if hat on joystick */

		if ( ctrl->part_type == PART_TYPE_BUTTON ) {
			g_pc_input.button[tmp_device_num][tmp_part_num].button_up_function = input_key_up_digital_stick;
			g_pc_input.button[tmp_device_num][tmp_part_num].button_down_function = input_key_down_digital_stick;
			g_pc_input.button[tmp_device_num][tmp_part_num].player = i;
			g_pc_input.button[tmp_device_num][tmp_part_num].direction = j;
		} /* end if button on joystick */

	} /* end if joystick */

	/*
	 * Check for Mouse
	 */
	if ( ctrl->device == DEV_MOUSE ) {
		g_pc_input.mouse_button[tmp_part_num].button_up_function = input_key_up_digital_stick;
		g_pc_input.mouse_button[tmp_part_num].button_down_function = input_key_down_digital_stick;
		g_pc_input.mouse_button[tmp_part_num].player = i;
	}

	return 0;

} /* input_setup_paddle_button */

/******************************************************************************
**  Function   :  input_setup_frame_functions
**                            
**  Objective  :  This function sets up control functions to be called every 
**                frame
**
**  Parameters :  player - which controller to setup for
**                                                
**  return     :  0
**      
******************************************************************************/
int input_setup_frame_functions ( int player ) {

	t_atari_common *ctrl = g_input.players[player].stick.direction;

	g_pc_input.frame_x_function[player] = input_frame_do_nothing;
	g_pc_input.frame_y_function[player] = input_frame_do_nothing;

	if ( g_input.machine_type == MACHINE_TYPE_5200 ) {
		if ( ctrl[DIR_LEFT].device == DEV_MOUSE )
			g_pc_input.frame_x_function[player] = input_center_x_axis_frame;
		if ( ctrl[DIR_UP].device == DEV_MOUSE )
			g_pc_input.frame_y_function[player] = input_center_y_axis_frame;

		if ( ctrl[DIR_LEFT].device== DEV_JOYSTICK &&
		     ctrl[DIR_LEFT].part_type== PART_TYPE_BALL )
			g_pc_input.frame_x_function[player] = input_center_x_axis_frame;
		if ( ctrl[DIR_UP].device== DEV_JOYSTICK &&
		     ctrl[DIR_UP].part_type== PART_TYPE_BALL )
			g_pc_input.frame_y_function[player] = input_center_y_axis_frame;
	}

	if ( g_input.players[player].stick.simulate_analog ) {
		g_pc_input.frame_x_function[player] = input_simulate_analog_x_axis_frame;
		g_pc_input.frame_y_function[player] = input_simulate_analog_y_axis_frame;
	}

	return 0;

} /* end input_setup_frame_functions */

/******************************************************************************
**  Function   :  input_setup_stick_values
**                            
**  Objective  :  This function sets up pot values and axis idle values for
**                the emulated stick
**
**  Parameters :  player - which controller to setup for
**                                                
**  return     :  0
**      
******************************************************************************/
int input_setup_stick_values ( int player ) {

	int j;
	double deadzone, deadzone_ratio;
	int pot_high_value, pot_low_value, pot_center_value;

	if ( g_input.machine_type == MACHINE_TYPE_5200 ) {
		pot_high_value = POT_HIGH_VALUE;
		pot_low_value = POT_LOW_VALUE;
		pot_center_value = POT_CENTER_VALUE;
	}
	else {
		pot_high_value = POT_800_HIGH_VALUE;
		pot_low_value = POT_800_LOW_VALUE;
		pot_center_value = POT_800_CENTER_VALUE;
	}

	/*
	 * Use sensitivity to find upper, lower and center values
	 */
	g_stick_interface[player].pot_low = pot_low_value + (int)( (double)(pot_high_value-pot_low_value) * (double)(100-g_input.players[player].stick.sensitivity) / 200.0 );
	g_stick_interface[player].pot_high = pot_high_value - (int)( (double)(pot_high_value-pot_low_value) * (double)(100-g_input.players[player].stick.sensitivity) / 200.0 );
	g_stick_interface[player].axis_value[0] = pot_center_value;
	g_stick_interface[player].axis_value[1] = pot_center_value;
	g_stick_interface[player].sa_high_threshold = (g_stick_interface[player].pot_high+pot_center_value)>>1;
	g_stick_interface[player].sa_low_threshold = (g_stick_interface[player].pot_low+pot_center_value)>>1;

	/*
	 * Analog axis values
	 */
	deadzone = 32768*g_input.players[player].stick.deadzone/100;
	deadzone_ratio = ((double)(32768-deadzone))/((double)(g_stick_interface[player].pot_high-g_stick_interface[player].pot_low)/2.0);

	for ( j = -32768; j < 32768; ++j ) {
		if ( abs(j) < deadzone ) {
			g_stick_interface[player].pot_values[j+32768] = pot_center_value;
		}
		else {
			if ( j > 0 )
				g_stick_interface[player].pot_values[j+32768] = (unsigned char)(((j-deadzone)/deadzone_ratio)+(pot_center_value+1));
			else
				g_stick_interface[player].pot_values[j+32768] = (unsigned char)(((j+deadzone)/deadzone_ratio)+(pot_center_value-1));
		}
	}

	/*
	 * Setup both axis idle variables and the pressed variables
	 */
	g_stick_interface[player].axis_idle[0] = -1;
	g_stick_interface[player].axis_idle[1] = -1;
	for ( j = DIR_LEFT; j < MAX_DIR; ++j )
		g_stick_interface[player].pressed[j] = 0;

	/*
	 * Check for mouse on this player
	 */
	if ( (g_input.players[player].stick.direction[0].device == DEV_MOUSE && 
	      g_input.machine_type == MACHINE_TYPE_5200) ) {
		g_stick_interface[player].axis_idle[0] = 4;
	}
	return 0;

} /* end input_setup_stick_values */

int input_do_nothing (int key, int dir, int temp) {

	return 0;

} /* input_do_nothing */

int input_frame_do_nothing (int key, int dir) {

	return 0;

} /* input_frame_do_nothing */

int input_center_x_axis_frame (int player, int dir) {

	g_stick_interface[player].axis_idle[0]--;
	if ( !g_stick_interface[player].axis_idle[0] )
		input_stick_h_axis ( player, POT_CENTER_VALUE );

	return 0;

} /* end input_center_x_axis_frame */

int input_center_y_axis_frame (int player, int dir) {

	g_stick_interface[player].axis_idle[1]--;
	if ( !g_stick_interface[player].axis_idle[1] )
		input_stick_v_axis ( player, POT_CENTER_VALUE );

	return 0;

} /* end input_center_y_axis_frame */

int input_simulate_analog_x_axis_frame (int player, int tmp) {

	if ( g_stick_interface[player].pressed[DIR_LEFT] )
		input_stick_h_axis ( player, g_stick_interface[player].axis_value[0]-=3 );
	if ( g_stick_interface[player].pressed[DIR_RIGHT] )
		input_stick_h_axis ( player, g_stick_interface[player].axis_value[0]+=3 );

	return 0;

} /* end input_simulate_analog_x_axis_frame */

int input_simulate_analog_y_axis_frame (int player, int tmp) {

	if ( g_stick_interface[player].pressed[DIR_UP] )
		input_stick_v_axis ( player, g_stick_interface[player].axis_value[1]-=3 );
	if ( g_stick_interface[player].pressed[DIR_DOWN] )
		input_stick_v_axis ( player, g_stick_interface[player].axis_value[1]+=3 );

	return 0;

} /* end input_simulate_analog_y_axis_frame */

int input_key_down_stick (int player, int dir, int key) {

	g_stick_interface[player].pressed[dir]= 1;
	switch ( dir ) {
		case DIR_LEFT: 
			pokey_set_pot_value ( (player<<1), g_stick_interface[player].pot_low );
			break;
		case DIR_RIGHT: 
			pokey_set_pot_value ( (player<<1), g_stick_interface[player].pot_high );
			break;
		case DIR_UP: 
			pokey_set_pot_value ( ((player<<1) | 0x1), g_stick_interface[player].pot_low );
			break;
		case DIR_DOWN: 
			pokey_set_pot_value ( ((player<<1) | 0x1), g_stick_interface[player].pot_high );
			break;
	}

	return 0;

} /* end input_key_down_stick */

int input_key_down_stick_sa (int player, int dir, int key) {

	g_stick_interface[player].pressed[dir]= 1;

	switch ( dir ) {
		case DIR_LEFT: 
			input_stick_h_axis ( player, g_stick_interface[player].axis_value[0]-=3 );
			break;
		case DIR_RIGHT: 
			input_stick_h_axis ( player, g_stick_interface[player].axis_value[0]+=3 );
			break;
		case DIR_UP: 
			input_stick_v_axis ( player, g_stick_interface[player].axis_value[0]-=3 );
			break;
		case DIR_DOWN: 
			input_stick_v_axis ( player, g_stick_interface[player].axis_value[0]+=3 );
			break;
	}

	return 0;

} /* end input_key_down_stick_sa */

int input_key_up_stick (int player, int dir, int key) {

	int mask = (dir & 0x2) >> 1;
	g_stick_interface[player].pressed[dir]= 0;
	if ( g_stick_interface[player].pressed[dir ^ 0x1]) {
		if ( dir & 0x1 )
			pokey_set_pot_value ( (player<<1) | mask, g_stick_interface[player].pot_low );
		else
			pokey_set_pot_value ( (player<<1) | mask, g_stick_interface[player].pot_high );
	}
	else
		pokey_set_pot_value ( (player<<1) | mask, POT_CENTER_VALUE );

	return 0;

} /* end input_key_up_stick */

int input_key_up_stick_sa (int player, int dir, int key) {

	g_stick_interface[player].pressed[dir]= 0;

	return 0;

} /* end input_key_up_stick_sa */

int input_key_down_digital_stick (int player, int dir, int key) {

	int value[] = {0x4,0x8,0x1,0x2};

	g_stick_interface[player].digital_pressed[dir]= 1;
	pia_set_input ( player, value[dir], 0 );
	pia_set_input ( player, value[dir^1], value[dir^1] );

	return 0;

} /* end input_key_down_digital_stick */

int input_key_up_digital_stick (int player, int dir, int key) {

	int value[] = {0x4,0x8,0x1,0x2};

	g_stick_interface[player].digital_pressed[dir]= 0;
	pia_set_input ( player, value[dir], value[dir] );

	if ( g_stick_interface[player].digital_pressed[dir^1] )
		pia_set_input ( player, value[dir^1], 0 );

	return 0;

} /* end input_key_up_digital_stick */

int input_joystick_pots_sa ( int player, int dir, int reading ) {

	int value;
	int rate;

	if ( dir & 0x1 ) {
		value = (g_stick_interface[player].pot_values[32768+reading] > g_stick_interface[player].sa_high_threshold);
		rate = 3;
	}
	else {
		value = (g_stick_interface[player].pot_values[32768-reading] < g_stick_interface[player].sa_low_threshold);
		rate = -3;
	}

	if ( value ) {
		g_stick_interface[player].pressed[dir] = 1;
		g_stick_interface[player].pressed[dir ^ 1] = 0;
		if ( dir & 0x2 )
			input_stick_v_axis ( player, g_stick_interface[player].axis_value[0] += rate );
		else
			input_stick_h_axis ( player, g_stick_interface[player].axis_value[0] += rate );
	}
	else
		g_stick_interface[player].pressed[dir] = 0;

	return 0;

} /* input_joystick_pots_sa */

int input_joystick_pots ( int player, int dir, int reading ) {

	int value;

	//if ( dir >> 1 )printf ( "y=%d\n", reading );
	//else printf ( "x=%d\n", reading );
	if ( dir & 0x1 )
		value = 32768+reading;
	else
		value = 32768-reading;

	pokey_set_pot_value ( (player<<1 | (dir>>1)), g_stick_interface[player].pot_values[value] );

    return 0;

} /* end input_joystick_pots */

int input_axis_digital_stick ( int player, int dir, int reading ) {

	int value[] = {0x4,0x8,0x1,0x2};

	if ( reading > 16384 )
		pia_set_input ( player, value[dir], 0 );
	else
		pia_set_input ( player, value[dir], value[dir] );

    return 0;

} /* end input_axis_digital_stick */

int input_keypad_down ( int player, int dir, int kbcode ) {

	pokey_set_keypad_down ( player, kbcode, 1 );

    return 0;

} /* end input_keypad_down */

int input_keypad_up ( int player, int dir, int kbcode ) {

	pokey_set_keypad_down ( player, kbcode, 0 );

    return 0;

} /* end input_keypad_up */

int input_modkey_down ( int player, int dir, int kbcode ) {

	switch ( kbcode ) {
		case 0: pokey_set_shift_key_down ( 1 ); break;
		case 1: pokey_set_ctrl_key_down ( 1 ); break;
		case 2: pokey_set_break_key_down ( 1 ); break;
		case 3: antic_set_reset(); break;
		default: pokey_set_shift_key_down ( 1 ); break;
	}

    return 0;

} /* end input_modkey_down */

int input_modkey_up ( int player, int dir, int kbcode ) {

	switch ( kbcode ) {
		case 0: pokey_set_shift_key_down ( 0 ); break;
		case 1: pokey_set_ctrl_key_down ( 0 ); break;
		case 2: pokey_set_break_key_down ( 0 ); break;
		case 3: break;
		default: pokey_set_shift_key_down ( 0 ); break;
	}

    return 0;

} /* end input_modkey_up */

int input_joyaxis_modkey ( int player, int reading, int kbcode ) {

	int value;

	if ( reading > 16384 )
		value = 1;
	else
		value = 0;

	switch ( kbcode ) {
		case 0: pokey_set_shift_key_down ( value ); break;
		case 1: pokey_set_ctrl_key_down ( value ); break;
		case 2: pokey_set_break_key_down ( value ); break;
		default: pokey_set_shift_key_down ( value ); break;
	}

    return 0;

} /* end input_joyaxis_modkey */

int input_switch_down ( int player, int dir, int mask ) {

	gtia_set_switch ( mask, 0 );

    return 0;

} /* end input_switch_down */

int input_switch_up ( int player, int dir, int mask ) {

	gtia_set_switch ( mask, 1 );

    return 0;

} /* end input_switch_up */

int input_joyaxis_switch ( int player, int reading, int mask ) {

	if ( reading > 16384 )
		gtia_set_switch ( mask, 0 );
	else
		gtia_set_switch ( mask, 1 );

    return 0;

} /* end input_joyaxis_switch */

int input_joyaxis_keypad ( int player, int reading, int kbcode ) {

	if ( reading > 16384 )
		pokey_set_keypad_down( player, kbcode, 1 );
	else
		pokey_set_keypad_down( player, kbcode, 0 );

    return 0;

} /* end input_joyaxis_keypad */

int input_800keypad_down ( int player, int dir, int kbcode ) {

	pia_set_keypad_down ( player, kbcode, 1 );

    return 0;

} /* end input_800keypad_down */

int input_800keypad_up ( int player, int dir, int kbcode ) {

	pia_set_keypad_down ( player, kbcode, 0 );

    return 0;

} /* end input_800keypad_up */

int input_joyaxis_800keypad ( int player, int reading, int kbcode ) {

	if ( reading > 16384 )
		pia_set_keypad_down( player, kbcode, 1 );
	else
		pia_set_keypad_down( player, kbcode, 0 );

    return 0;

} /* end input_joyaxis_800keypad */

int input_top_button_down ( int player, int dir, int temp ) {

	pokey_set_top_button_down ( player, 1 );

    return 0;

} /* end pc_top_button_down */

int input_top_button_up ( int player, int dir, int temp ) {

	pokey_set_top_button_down ( player, 0 );

    return 0;

} /* end pc_top_button_up */

int input_joyaxis_top_button ( int player, int dir, int reading ) {

	if ( reading > 16384 )
		pokey_set_top_button_down ( player, 1 );
	else
		pokey_set_top_button_down ( player, 0 );

    return 0;

} /* end input_joyaxis_top_button */

int input_joyaxis_bottom_button ( int player, int dir, int reading ) {

	if ( reading > 16384 )
		gtia_bottom_button_down ( player, 0, 0 );
	else
		gtia_bottom_button_up ( player, 0, 0 );

    return 0;

} /* end input_joyaxis_bottom_button */

int input_stick_h_axis ( int player, int reading ) {

	if ( reading < g_stick_interface[player].pot_low ) 
		reading = g_stick_interface[player].pot_low;
	if ( reading > g_stick_interface[player].pot_high ) 
		reading = g_stick_interface[player].pot_high;
	pokey_set_pot_value ( player<<1, reading );

    return 0;

} /* end input_stick_h_axis */

int input_stick_v_axis ( int player, int reading ) {

	if ( reading < g_stick_interface[player].pot_low ) 
		reading = g_stick_interface[player].pot_low;
	if ( reading > g_stick_interface[player].pot_high ) 
		reading = g_stick_interface[player].pot_high;
	pokey_set_pot_value ( ((player<<1) | 0x1), reading );

    return 0;

} /* end input_stick_v_axis */

int input_mouse_axis (int player, int dir, int value ) {

	switch ( dir ) {
		case DIR_LEFT: 
			input_stick_h_axis ( player, g_stick_interface[player].axis_value[0]-=value );
			break;
		case DIR_RIGHT: 
			input_stick_h_axis ( player, g_stick_interface[player].axis_value[0]+=value );
			break;
		case DIR_UP: 
			input_stick_v_axis ( player, g_stick_interface[player].axis_value[1]-=value );
			break;
		case DIR_DOWN: 
			input_stick_v_axis ( player, g_stick_interface[player].axis_value[1]+=value );
			break;
	}

	return 0;

} /* end input_mouse_axis */

int input_mouse_vel_axis (int player, int dir, int value ) {

	switch ( dir ) {
		case DIR_LEFT: 
			g_stick_interface[player].axis_idle[0] = 4;
			input_stick_h_axis ( player, -value + POT_CENTER_VALUE );
			break;
		case DIR_RIGHT: 
			g_stick_interface[player].axis_idle[0] = 4;
			input_stick_h_axis ( player, value + POT_CENTER_VALUE );
			break;
		case DIR_UP: 
			g_stick_interface[player].axis_idle[1] = 4;
			input_stick_v_axis ( player, -value + POT_CENTER_VALUE );
			break;
		case DIR_DOWN: 
			g_stick_interface[player].axis_idle[1] = 4;
			input_stick_v_axis ( player, value + POT_CENTER_VALUE );
			break;
	}

	return 0;

} /* end input_mouse_vel_axis */

int input_trackball_x_axis (int player, int dir, int value) {

	g_stick_interface[player].axis_idle[0] = 4;
	input_stick_h_axis ( player, value );

	return 0;

} /* end input_trackball_x_axis */

int input_trackball_y_axis (int player, int dir, int value) {

	g_stick_interface[player].axis_idle[1] = 4;
	input_stick_v_axis ( player, value );

	return 0;

} /* end input_trackball_y_axis */
