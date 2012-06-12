#ifndef INPUT_H_
#define INPUT_H_

/* Keyboard AKEY_* are in akey.h */

/* INPUT_key_consol masks */
/* Note: INPUT_key_consol should be INPUT_CONSOL_NONE if no consol key is pressed.
   When a consol key is pressed, corresponding bit should be cleared.
 */
#define INPUT_CONSOL_NONE		0x07
#define INPUT_CONSOL_START	0x01
#define INPUT_CONSOL_SELECT	0x02
#define INPUT_CONSOL_OPTION	0x04

extern int INPUT_key_code;	/* regular Atari key code */
extern int INPUT_key_shift;	/* Shift key pressed */
extern int INPUT_key_consol;	/* Start, Select and Option keys */

/* Joysticks ----------------------------------------------------------- */

/* joystick position */
#define	INPUT_STICK_LL		0x09
#define	INPUT_STICK_BACK		0x0d
#define	INPUT_STICK_LR		0x05
#define	INPUT_STICK_LEFT		0x0b
#define	INPUT_STICK_CENTRE	0x0f
#define	INPUT_STICK_RIGHT		0x07
#define	INPUT_STICK_UL		0x0a
#define	INPUT_STICK_FORWARD	0x0e
#define	INPUT_STICK_UR		0x06

/* joy_autofire values */
#define INPUT_AUTOFIRE_OFF	0
#define INPUT_AUTOFIRE_FIRE	1	/* Fire dependent */
#define INPUT_AUTOFIRE_CONT	2	/* Continuous */

extern int INPUT_joy_autofire[4];		/* autofire mode for each Atari port */

extern int INPUT_joy_block_opposite_directions;	/* can't move joystick left
											   and right simultaneously */

extern int INPUT_joy_multijoy;	/* emulate MultiJoy4 interface */

/* 5200 joysticks values */
extern int INPUT_joy_5200_min;
extern int INPUT_joy_5200_center;
extern int INPUT_joy_5200_max;

/* Mouse --------------------------------------------------------------- */

/* INPUT_mouse_mode values */
#define INPUT_MOUSE_OFF		0
#define INPUT_MOUSE_PAD		1	/* Paddles */
#define INPUT_MOUSE_TOUCH	2	/* Atari touch tablet */
#define INPUT_MOUSE_KOALA	3	/* Koala pad */
#define INPUT_MOUSE_PEN		4	/* Light pen */
#define INPUT_MOUSE_GUN		5	/* Light gun */
#define INPUT_MOUSE_AMIGA	6	/* Amiga mouse */
#define INPUT_MOUSE_ST		7	/* Atari ST mouse */
#define INPUT_MOUSE_TRAK	8	/* Atari CX22 Trak-Ball */
#define INPUT_MOUSE_JOY		9	/* Joystick */

extern int INPUT_mouse_mode;			/* device emulated with mouse */
extern int INPUT_mouse_port;			/* Atari port, to which the emulated device is attached */
extern int INPUT_mouse_delta_x;		/* x motion since last frame */
extern int INPUT_mouse_delta_y;		/* y motion since last frame */
extern int INPUT_mouse_buttons;		/* buttons pressed (b0: left, b1: right, b2: middle */
extern int INPUT_mouse_speed;			/* how fast the mouse pointer moves */
extern int INPUT_mouse_pot_min;		/* min. value of POKEY's POT register */
extern int INPUT_mouse_pot_max;		/* max. value of POKEY's POT register */
extern int INPUT_mouse_pen_ofs_h;		/* light pen/gun horizontal offset (for calibration) */
extern int INPUT_mouse_pen_ofs_v;		/* light pen/gun vertical offset (for calibration) */
extern int INPUT_mouse_joy_inertia;	/* how long the mouse pointer can move (time in Atari frames)
								   after a fast motion of mouse */
extern int INPUT_direct_mouse;      /* When true, convert the mouse pointer
													position directly into POKEY POT values */

extern int INPUT_cx85;      /* emulate CX85 numeric keypad */
/* Functions ----------------------------------------------------------- */

int INPUT_Initialise(int *argc, char *argv[]);
void INPUT_Exit(void);
void INPUT_Frame(void);
void INPUT_Scanline(void);
void INPUT_SelectMultiJoy(int no);
void INPUT_CenterMousePointer(void);
void INPUT_DrawMousePointer(void);
int INPUT_Recording(void);
int INPUT_Playingback(void);
void INPUT_RecordInt(int i);
int INPUT_PlaybackInt(void);

#endif /* INPUT_H_ */
