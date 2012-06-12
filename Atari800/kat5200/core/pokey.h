/******************************************************************************
*
* FILENAME: pokey.h
*
* DESCRIPTION:  This contains function declartions for pokey sound functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     05/11/05  bberlin      Creation
* 0.5.0   08/25/06  bberlin      Added 'pokey_set_controller_type' function
******************************************************************************/
#ifndef pokey_h
#define pokey_h

#include "boom6502.h"

/* Pokey Write Registers */
#define AUDF1   0x00
#define AUDC1   0x01
#define AUDF2   0x02
#define AUDC2   0x03
#define AUDF3   0x04
#define AUDC3   0x05
#define AUDF4   0x06
#define AUDC4   0x07
#define AUDCTL  0x08
#define STIMER  0x09
#define SKREST  0x0a
#define POTGO   0x0b
#define SEROUT  0x0d
#define IRQEN   0x0e
#define SKCTL   0x0f

/* Pokey Read Registers  */
#define POT0    0x00
#define POT1    0x01
#define POT2    0x02
#define POT3    0x03
#define POT4    0x04
#define POT5    0x05
#define POT6    0x06
#define POT7    0x07
#define ALLPOT  0x08
#define KBCODE  0x09
#define RANDOM  0x0a
#define SERIN   0x0d
#define IRQST   0x0e
#define SKSTAT  0x0f

/* 5200 Keyboard codes   */
#define KEY_NONE  0x00
#define KEY_POUND 0x03
#define KEY_0     0x05
#define KEY_STAR  0x07
#define KEY_RESET 0x09
#define KEY_9     0x0b
#define KEY_8     0x0d
#define KEY_7     0x0f
#define KEY_PAUSE 0x11
#define KEY_6     0x13
#define KEY_5     0x15
#define KEY_4     0x17
#define KEY_START 0x19
#define KEY_3     0x1b
#define KEY_2     0x1d
#define KEY_1     0x1f

/* IRQ Bits              */
#define IRQ_TIMER1    0x01
#define IRQ_TIMER2    0x02
#define IRQ_TIMER4    0x04
#define IRQ_SER_DONE  0x08
#define IRQ_SER_NEED  0x10
#define IRQ_SER_RED   0x20
#define IRQ_KEY_PRESS 0x40
#define IRQ_BK_PRESS  0x80
#define NOT_IRQ_TIMER1    0xfe
#define NOT_IRQ_TIMER2    0xfd
#define NOT_IRQ_TIMER4    0xfb
#define NOT_IRQ_SER_DONE  0xf7
#define NOT_IRQ_SER_NEED  0xef
#define NOT_IRQ_SER_RED   0xdf
#define NOT_IRQ_KEY_PRESS 0xbf
#define NOT_IRQ_BK_PRESS  0x7f

/* SKCTL Bits            */
#define SK_KEY_DEB   0x01
#define SK_KEY_SCAN  0x02
#define SK_FAST_POT  0x04
#define SK_TWO_TONE  0x08
#define SK_SER_MODE  0x70
#define SK_SER_BREAK 0x80

/* SKSTAT Bits           */
#define SK_SERIN_BUSY    0x02
#define SK_KEY_PRESS     0x04
#define SK_SHIFT_KEY     0x08
#define SK_DIRECT_SERIN  0x10
#define SK_KEY_OVERRUN   0x20
#define SK_SERIN_OVERRUN 0x40
#define SK_SERIN_ERROR   0x80
#define NOT_SK_SERIN_BUSY    0xfd
#define NOT_SK_KEY_PRESS     0xfb
#define NOT_SK_SHIFT_KEY     0xf7
#define NOT_SK_DIRECT_SERIN  0xef
#define NOT_SK_KEY_OVERRUN   0xdf
#define NOT_SK_SERIN_OVERRUN 0xbf
#define NOT_SK_SERIN_ERROR   0x7f

/* KBCODE Bits           */
#define KB_SHIFT_KEY     0x40
#define KB_CTRL_KEY      0x80
#define NOT_KB_SHIFT_KEY 0xbf
#define NOT_KB_CTRL_KEY  0x7f

/* Controller Type       */
#define CTRL_STICK  0
#define CTRL_BALL   1

/*
 * Atari 800 keycodes
 */
#define A800_KEY_L      0x00
#define A800_KEY_J      0x01
#define A800_KEY_SEMI   0x02
#define A800_KEY_K      0x05
#define A800_KEY_PLUS   0x06
#define A800_KEY_STAR   0x07
#define A800_KEY_O      0x08
#define A800_KEY_P      0x0a
#define A800_KEY_U      0x0b
#define A800_KEY_RETURN 0x0c
#define A800_KEY_I      0x0d
#define A800_KEY_MINUS  0x0e
#define A800_KEY_EQUALS 0x0f
#define A800_KEY_V      0x10
#define A800_KEY_HELP   0x11
#define A800_KEY_C      0x12
#define A800_KEY_B      0x15
#define A800_KEY_X      0x16
#define A800_KEY_Z      0x17
#define A800_KEY_4      0x18
#define A800_KEY_3      0x1a
#define A800_KEY_6      0x1b
#define A800_KEY_ESC    0x1c
#define A800_KEY_5      0x1d
#define A800_KEY_2      0x1e
#define A800_KEY_1      0x1f
#define A800_KEY_COMMA  0x20
#define A800_KEY_SPACE  0x21
#define A800_KEY_PERIOD 0x22
#define A800_KEY_N      0x23
#define A800_KEY_M      0x25
#define A800_KEY_SLASH  0x26
#define A800_KEY_FUJI   0x27
#define A800_KEY_R      0x28
#define A800_KEY_E      0x2a
#define A800_KEY_Y      0x2b
#define A800_KEY_TAB    0x2c
#define A800_KEY_T      0x2d
#define A800_KEY_W      0x2e
#define A800_KEY_Q      0x2f
#define A800_KEY_9      0x30
#define A800_KEY_0      0x32
#define A800_KEY_7      0x33
#define A800_KEY_BKSP   0x34
#define A800_KEY_8      0x35
#define A800_KEY_LESST  0x36
#define A800_KEY_MORET  0x37
#define A800_KEY_F      0x38
#define A800_KEY_H      0x39
#define A800_KEY_D      0x3a
#define A800_KEY_CAPS   0x3c
#define A800_KEY_G      0x3d
#define A800_KEY_S      0x3e
#define A800_KEY_A      0x3f

/*
 * Pot Digital Values
 */
#define POT_CENTER_VALUE 112
#define POT_LOW_VALUE 2
#define POT_HIGH_VALUE 226
#define POT_800_CENTER_VALUE 124
#define POT_800_LOW_VALUE 4
#define POT_800_HIGH_VALUE 228

/*
 * Serial Port Hardware Status
 *   We have the SEROUT register and the output shift register
 */
#define SER_OUT_OCCUPIED 0x1
#define SER_REG_OCCUPIED 0x2
#define NOT_SER_OUT_OCCUPIED 0xe
#define NOT_SER_REG_OCCUPIED 0xd

/*
 * Serial Mode Status
 */
#define SER_MODE_OFF    0  
#define SER_MODE_QUEUED 1   
#define SER_MODE_ON     2

struct pokey {
	
	/*
	 * Pots 
	 */
	BYTE pot[8];
	BYTE pc_pot[8];
	BYTE pot_scanline;

	BYTE kbcode;

	/*
	 * The Serial Port
	 */
	BYTE serial_in;
	BYTE serial_out;
	BYTE serial_shift;
	unsigned int serial_hw_status;
	unsigned int serial_irq_time;
	unsigned int serial_out_time;

	/*
	 * IRQ Status - 0 means interupt occurred
	 */
	BYTE irq_status;
	BYTE irq_enable;
	BYTE sk_ctl;
	BYTE sk_status;

	BYTE w_reg[0x10];
	BYTE r_reg[0x10];

	/*
	 * The key counter 
	 */
	BYTE current_key_code;

	int key_pressed[4][0x40];
	int shift_pressed[4];
	int ctrl_pressed;
	int break_pressed;
	int serial_mode_status;

	/*
	 * The controller type (stick or ball)
	 */
	int controller_type[4];
};

void pokey_init ( int freq );
void pokey_set_pot_value ( int pot, BYTE value );
void pokey_set_keypad_down ( int player, int kbcode, int down );
void pokey_set_top_button_down ( int player, int down );
void pokey_set_shift_key_down ( int down );
void pokey_set_ctrl_key_down ( int down );
void pokey_set_break_key_down ( int down );
void pokey_set_controller_type ( int player, int type );
BYTE pokey_read_register ( WORD addr );
void pokey_write_register ( WORD addr, BYTE value );
BYTE pokey_read_register_debug ( WORD addr );
void pokey_write_register_debug ( WORD addr, BYTE value );
void pokey_init_scanline( void );
void pokey_receive_SIO ( BYTE data );

#endif
