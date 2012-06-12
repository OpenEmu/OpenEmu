/******************************************************************************
*
* FILENAME: pia.c
*
* DESCRIPTION:  This contains functions for emulating Port I/O functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   01/25/07  bberlin      Creation
******************************************************************************/
#include "pia.h"
#include "debug.h"
#include "pokey.h"
#include "gtia.h"
#include "console.h"
#include "memory.h"
#include "sio.h"

struct pia pia;

/******************************************************************************
**  Function   :  pia_init
**                            
**  Objective  :  This function initializes pia registers
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void pia_init ( void ) {

	int i;

	for ( i = 0; i < 2; ++i ) {
		pia.input[i] = 0xff;
		pia.port_value[i] = 0xff;
		pia.direction[i] = 0x00;
		pia.port_control[i] = 0x3f;
	}

} /* end pia_init */

/******************************************************************************
**  Function   :  pia_read_register
**                            
**  Objective  :  This function reads from the input register
**
**  Parameters :  addr - 6502 memory address of the register
**                                                
**  return     :  value at register
**      
******************************************************************************/ 
BYTE pia_read_register ( WORD addr ) {

	BYTE new_address = addr & 0x03;

	switch ( new_address ) {
		case PORTA:
		case PORTB:
			if ( pia.port_control[new_address] & 0x04 ) {
				if ( (new_address == PORTB) && (console_get_machine_type() == MACHINE_TYPE_XL) ) {
					return pia.port_value[PORTB] | (~pia.direction[PORTB] & 0xff);
				}
				else {
					return pia.input[new_address] & 
					      (pia.port_value[new_address] | (~pia.direction[new_address] & 0xff));
				}
			}
			else {
				return pia.direction[new_address];
			}	
			break;
		case PACTL:
		case PBCTL:
			return pia.port_control[new_address & 0x1] & 0x3f;
			break;
	}

	return 0xff;

} /* end pia_read_register */

BYTE pia_read_register_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return pia_read_register ( addr );
}

/******************************************************************************
**  Function   :  pia_write_register
**                            
**  Objective  :  This function performs actions caused by writes to the 
**                registers the input register
**
**  Parameters :  addr - 6502 memory address of the register
**                value - value to write to register
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void pia_write_register ( WORD addr, BYTE value ) {

	BYTE new_address = addr & 0x03;

	switch ( new_address ) {
		case PORTA:
		case PORTB:
			if ( pia.port_control[new_address] & 0x04 ) {
				if ( (console_get_machine_type() == MACHINE_TYPE_XL) && (new_address == PORTB) ) {
					mem_assign_portb_ops ( console_get_ptr()->cpu, value | ((~pia.direction[PORTB]) & 0xff) );
				}
				pia.port_value[new_address] = value;
			}
			else {
				if ( (console_get_machine_type() == MACHINE_TYPE_XL) && (new_address == PORTB) ) {
					mem_assign_portb_ops ( console_get_ptr()->cpu, pia.port_value[PORTB] | ((~value) & 0xff) );
				}
				pia.direction[new_address] = value;
			}	
			break;
		case PACTL:
			sio_set_motor ( value & 0x08 );
		case PBCTL:
			pia.port_control[new_address & 0x1] = value;
			return; 
			break;
	}

	return;

} /* end pia_write_register */

void pia_write_register_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	pia_write_register ( addr, value );
}

/******************************************************************************
**  Function   :  pia_recall_port_registers
**                            
**  Objective  :  This function sets the port registers based on value. Used
**                with recalling states.
**
**  Parameters :  addr - 6502 memory address of the register
**                value - value to write to register
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void pia_recall_port_registers ( WORD addr, BYTE value ) {

	pia.input[addr&0x1] = value;
	pia.port_value[addr&0x1] = value;

} /* end pia_recall_port_registers */

/******************************************************************************
**  Function   :  pia_set_input
**                            
**  Objective  :  This function sets the input port register based on stick
**                position
**
**  Parameters :  player - which controller port it came from
**                mask   - which bit to set
**                value  - should be set to 0 or mask
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void pia_set_input ( int player, int mask, BYTE value ) {

	switch ( player ) {
		case 0:pia.input[0] = (pia.input[0] & ~(mask)) | (value); break;
		case 1:pia.input[0] = (pia.input[0] & ~(mask<<4)) | (value<<4); break;
		case 2:pia.input[1] = (pia.input[1] & ~(mask)) | (value); break;
		case 3:pia.input[1] = (pia.input[1] & ~(mask<<4)) | (value<<4); break;
	}

} /* end pia_set_input */

/******************************************************************************
**  Function   :  pia_set_keypad_down
**                            
**  Objective  :  This function is called when input detects a key press or
**                release
**
**  Parameters :  player - which controller it came from
**                key    - which key (0-11, left to right, up to down)
**                down   - 1 for press, 0 for release
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void pia_set_keypad_down ( int player, int key, int down ) {

	int shift;

	pia.key_pressed[player][key/3][key%3] = down;

	shift = ((player&0x1)<<2) + (key/3);
	if ( (pia.direction[player>>1] >> shift) & 0x1 ) {
		if ( !((pia.port_value[player>>1] >> shift) & 0x1) )
			pia_set_keypad_column ( player );
	}

} /* end pia_set_keypad_down */

/******************************************************************************
**  Function   :  pia_set_keypad_column
**                            
**  Objective  :  This function sets the pot and trig registers based on the
**                row and port selected
**
**  Parameters :  player - which player we are checking
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void pia_set_keypad_column ( int player ) {

	int i;

	for ( i = 0+(player&0x1)*4; i < (4+(player&0x1)*4); i++ ) {
		if ( !(pia.port_value[player>>1] >> i) && ( pia.direction[player>>1] >> i )) {

			/*
			 * First two columns are tied to POTs
			 */
			if ( pia.key_pressed[player][i&0x3][0] )
				pokey_set_pot_value ( 0, 228 );
			else
				pokey_set_pot_value ( 0, 2 );

			if ( pia.key_pressed[player][i&0x3][1] )
				pokey_set_pot_value ( 1, 228 );
			else
				pokey_set_pot_value ( 1, 2 );

			/*
			 * 3rd column tied to TRIG, just like 5200 fire button
			 */
			if ( pia.key_pressed[player][i&0x3][2] )
				gtia_bottom_button_down ( player, 0, 0 );
			else
				gtia_bottom_button_up ( player, 0, 0 );
		}		
	}

} /* end pia_set_keypad_column */

