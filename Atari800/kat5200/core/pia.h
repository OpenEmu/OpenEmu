/******************************************************************************
*
* FILENAME: pia.h
*
* DESCRIPTION:  This contains function declarations for Port I/O functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   01/24/07  bberlin      Creation
******************************************************************************/
#ifndef pia_h
#define pia_h

#include "boom6502.h"

/*
 * PIA Registers
 */
#define PORTA   0x00
#define PORTB   0x01
#define PACTL   0x02
#define PBCTL   0x03

struct pia {
	BYTE port_control[2];
	BYTE port_value[2];
	BYTE input[2];
	BYTE direction[2];
	int key_pressed[4][4][3];
};

void pia_init ( void );
BYTE pia_read_register ( WORD addr );
void pia_write_register ( WORD addr, BYTE value );
BYTE pia_read_register_debug ( WORD addr );
void pia_write_register_debug ( WORD addr, BYTE value );
void pia_recall_port_registers ( WORD addr, BYTE value );
void pia_set_input ( int player, int mask, BYTE value );
void pia_set_keypad_down ( int player, int key, int down );
void pia_set_keypad_column ( int player );

#endif
