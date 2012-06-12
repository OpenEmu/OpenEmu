/******************************************************************************
*
* FILENAME: input.h
*
* DESCRIPTION:  This contains function declarations for input handling
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   01/10/06  bberlin      Creation, break out from config
******************************************************************************/
#ifndef input_h
#define input_h

#define MAX_PC_DEVICE 8
#define MAX_PC_PART 32
#define MAX_PC_KEY 255
//#include "sdl_if.h"
#include "../core/console.h"

#define MAX_CONTROLLER 4
#define MAX_KEYS 64
#define MAX_KEYPAD_KEYS 16

typedef enum {
	DEV_KEYBOARD,
	DEV_JOYSTICK,
	DEV_MOUSE,
	DEV_MAX
} e_dev_type;

typedef enum {
	PART_TYPE_HAT,
	PART_TYPE_AXIS,
	PART_TYPE_BALL,
	PART_TYPE_BUTTON,
	PART_TYPE_KEY,
	PART_TYPE_MAX
} e_part_type;

typedef enum {
	DIR_LEFT,
	DIR_RIGHT,
	DIR_UP,
	DIR_DOWN,
	MAX_DIR,
	DIR_PLUS,
	DIR_MINUS,
	DIR_PRESS,
	DIR_RELEASE
} e_direction;

typedef enum {
	CTRLR_TYPE_NONE,
	CTRLR_TYPE_JOYSTICK,
	CTRLR_TYPE_TRACKBALL,
	CTRLR_TYPE_PADDLES,
	CTRLR_TYPE_KEYPAD,
	CTRLR_TYPE_MAX
} e_ctrlr_type;

/*
 * Declare Atari specific structs
 */
typedef struct {
	e_dev_type device;
	int device_num;
	e_part_type part_type;
	int part_num;
	e_direction direction;
} t_atari_common;

typedef struct {
	t_atari_common direction[MAX_DIR];
	int deadzone;
	int sensitivity;
	int simulate_analog;
} t_atari_stick;

typedef struct {
	e_ctrlr_type control_type;
	t_atari_stick stick;
	t_atari_stick paddles;
	t_atari_common top_button;
	t_atari_common bottom_button;
	t_atari_common keypad[MAX_KEYPAD_KEYS];
} t_atari_player;

typedef struct {
	char name[257];
	char description[257];
	e_machine_type machine_type;
	int mouse_speed;
	t_atari_player players[MAX_CONTROLLER];
	t_atari_common keyboard[MAX_KEYS];
	t_atari_common ctrl_key;
	t_atari_common shift_key;
	t_atari_common break_key;
	t_atari_common start_key;
	t_atari_common select_key;
	t_atari_common option_key;
	t_atari_common reset_key;
} t_atari_input;

/*
 * Declare PC specific structs
 */
typedef struct {
	int axis_idle[2];
	int axis_value[2];
	int pot_low;
	int pot_high;
	int sa_high_threshold;
	int sa_low_threshold;
	int pressed[MAX_DIR];
	unsigned char pot_values[0x10000];
	int ( *x_axis_function ) ( int,int,int );
	int ( *y_axis_function ) ( int,int,int );
} t_pc_stick;

typedef struct {
	int ( *key_up_function ) ( int,int,int );
	int ( *key_down_function ) ( int,int,int );
	int modifier;
	int key;
	int player;
} t_pc_mod_keyboard_key;

typedef struct {
	int ( *key_up_function ) ( int,int,int );
	int ( *key_down_function ) ( int,int,int );
	int player;
	int atari_key;
	int direction;
} t_pc_keyboard_key;

typedef struct {
	int ( *hat_pressed_function [MAX_DIR] ) ( int,int,int );
	int ( *hat_not_pressed_function [MAX_DIR] ) ( int,int,int );
	int direction[MAX_DIR];
	int player;
	int atari_key;
} t_pc_digital_hat;

typedef struct {
	int ( *axis_pos_function ) ( int,int,int );
	int ( *axis_neg_function ) ( int,int,int );
	int player;
	int direction_pos;
	int direction_neg;
	int sa_high_threshold;
	int sa_low_threshold;
} t_pc_analog_axis;

typedef struct {
	t_pc_analog_axis x_axis;
	t_pc_analog_axis y_axis;
	int speed;
} t_pc_ball;

typedef struct {
	int ( *button_up_function ) ( int,int,int );
	int ( *button_down_function ) ( int,int,int );
	int player;
	int atari_key;
	int direction;
} t_pc_button;

typedef struct {
	t_pc_mod_keyboard_key ui_keyboard[30];
	t_pc_keyboard_key keyboard[MAX_PC_KEY];
	t_pc_digital_hat pov_hat[MAX_PC_DEVICE][MAX_PC_PART];
	t_pc_analog_axis analog_axis[MAX_PC_DEVICE][MAX_PC_PART];
	t_pc_ball trackball[MAX_PC_DEVICE][MAX_PC_PART];
	t_pc_ball mouse;
	t_pc_button button[MAX_PC_DEVICE][MAX_PC_PART];
	t_pc_button mouse_button[4];
	int ( *frame_x_function[MAX_CONTROLLER] ) ( int,int );
	int ( *frame_y_function[MAX_CONTROLLER] ) ( int,int );
} t_pc_input;

/*
 * Function Declarations
 */
int input_set_defaults ( void );
int input_set_profile ( char *filename );
int input_load_profile ( const char *filename, t_atari_input *p_input );
int input_save_profile ( char *name, t_atari_input *p_input );
int input_find_common ( int id, t_atari_common *ctrl );
int input_write_common ( char *statement, t_atari_common *ctrl, int *id );
#endif
