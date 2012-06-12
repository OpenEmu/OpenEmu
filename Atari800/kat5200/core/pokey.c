/******************************************************************************
*
* FILENAME: pokey.c
*
* DESCRIPTION:  This contains functions for emulating the Pokey sound chip
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     05/11/05  bberlin      Creation
* 0.2     03/09/06  bberlin      Added a timer to 'pokey_init_scanline' for 
*                                  use with the pokey sound module
* 0.2.3   03/17/06  bberlin      Change 'Pokey_sound_init' function to use 
*                                  sound_freq parameter.
* 0.2.3   03/18/06  bberlin      Change 'pokey_init_scanline' to only send irq
*                                  when 6502 is running
* 0.2.3   03/19/06  bberlin      Took out Pokey_sound_init from here and put
*                                  in sdl_if.c
* 0.4.0   04/16/06  bberlin      Added Pokey_sound_init back to here
* 0.5.0   08/25/06  bberlin      Added 'pokey_get_pc_pot_ptr' function
* 0.5.0   08/25/06  bberlin      Added 'pokey_set_controller_type' function
******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "console.h"
#include "pokey.h"
#include "gtia.h"
#include "antic.h"
#include "pokeysound.h"
#include "debug.h"
#include "sio.h"

#define SER_PORT_IRQ_DELAY  114

struct pokey pokey;
extern struct gtia gtia;
extern struct antic antic;
extern t_sio_interface sio;

unsigned int pokey_cpu_clock;
static int key_is_pressed = 0;
static int irq_sent_flag = 0;

void pokey_check_SIO ( void );
void pokey_check_keys_5200 ( void );
void pokey_check_keys_800 ( void );

void pokey_set_pot_value ( int pot, BYTE value ) {
	pokey.pc_pot[pot] = value;
}

void pokey_set_keypad_down ( int player, int kbcode, int down ) {
	pokey.key_pressed[player][kbcode] = down;
}

void pokey_set_top_button_down ( int player, int down ) {
	pokey.shift_pressed[player] = down;
}

void pokey_set_shift_key_down ( int down ) {
	pokey.shift_pressed[0] = down;
}

void pokey_set_ctrl_key_down ( int down ) {
	pokey.ctrl_pressed = down;
}

void pokey_set_break_key_down ( int down ) {
	pokey.break_pressed = down;
}

void pokey_set_controller_type ( int player, int type ) {
	pokey.controller_type[player] = type;
}

/******************************************************************************
**  Function   :  pokey_init
**                            
**  Objective  :  This function initializes pokey registers
**
**  Parameters :  freq - what frequency to set Pokey Sound to
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void pokey_init ( int freq ) {

	int i;

	/*
	 * Seed Random Gen
	 */
	srand((int)time(NULL));

	pokey_cpu_clock = 0;

	/*
	 * Initialize Pot stuff
	 */
	for ( i = 0; i < 8; ++i ) {
		pokey.pot[i] = 228;
		if ( console_get_machine_type() == MACHINE_TYPE_5200 )
			pokey.pc_pot[i] = POT_CENTER_VALUE;
		else
			pokey.pc_pot[i] = POT_800_CENTER_VALUE;
		pokey.controller_type[i>>1] = 0;
	}

	memset ( pokey.w_reg, 0, sizeof(BYTE)*0x10 );
	memset ( pokey.r_reg, 0, sizeof(BYTE)*0x10 );
	pokey.kbcode = 0;
	pokey.irq_status = 0xff;
	pokey.sk_status = 0xff;

	pokey.irq_enable = 0x00;
	pokey.sk_ctl = 0x01;

	pokey.serial_in = 0x0;
	pokey.serial_out = 0x0;
	pokey.serial_hw_status = 0;
	pokey.serial_mode_status = 0;
	pokey.serial_irq_time = 1;
	pokey.serial_out_time = 1;
	
	/*
	 * Initialize Pokey Sound
	 */
	Pokey_sound_init (FREQ_17_APPROX, (unsigned short)freq, 1);

	return;

} /* end pokey_init */

/******************************************************************************
**  Function   :  pokey_read_register
**                            
**  Objective  :  This function reads from the input register
**
**  Parameters :  addr - 6502 memory address of the register
**                                                
**  return     :  value at register
**      
******************************************************************************/ 
BYTE pokey_read_register ( WORD addr ) {

	BYTE new_address = addr & 0x0f;
	BYTE value;
	int i;

	switch ( new_address ) {
		case POT0:
		case POT1:
		case POT2:
		case POT3:
		case POT4:
		case POT5:
		case POT6:
		case POT7:
			if ( !gtia.pot_enable && console_get_machine_type() == MACHINE_TYPE_5200) {
				if ( pokey.controller_type[new_address>>1] )
					return pokey.r_reg[new_address] = 114;
				else
					return pokey.r_reg[new_address] = 228;
			}
			if ( pokey.pot[new_address] < pokey.pot_scanline) {
				return pokey.r_reg[new_address] = pokey.pot[new_address];
			}
			else {
				return pokey.r_reg[new_address] = pokey.pot_scanline;
			}

			break;
		case ALLPOT:
			value = 0x00;
			for ( i = 0; i < 8; ++i ) {
				if ( pokey.pot[i] > pokey.pot_scanline )
					value |= (1<i);
			}
			return pokey.r_reg[ALLPOT] = value;
			break;
		case KBCODE:
			if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
				if ( pokey.sk_ctl & SK_KEY_DEB )
					return 0xff;
				else
					return pokey.kbcode | ((rand() & 0x1)<<5);
			}
			else
				return pokey.kbcode;
			break;
		case RANDOM:
			return rand() & 0xff;
			break;
		case SERIN:
			return pokey.serial_in;
			break;
		case IRQST:
			return pokey.irq_status;
			break;
		case SKSTAT:
			pokey.sk_status &= NOT_SK_DIRECT_SERIN;
			pokey.sk_status |= sio_update_serial_in_bit();
			return pokey.sk_status;
			break;
	}

	return 0xff;

} /* end pokey_read_register */

BYTE pokey_read_register_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return pokey_read_register ( addr );
}

/******************************************************************************
**  Function   :  pokey_write_register
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
void pokey_write_register ( WORD addr, BYTE value ) {

	BYTE new_address = addr & 0x0f;

	switch ( new_address ) {

		/*
		 * Use Pokey Sound for audio
		 */
		case AUDF1:
		case AUDC1:
		case AUDF2:
		case AUDC2:
		case AUDF3:
		case AUDC3:
		case AUDF4:
		case AUDC4:
		case AUDCTL:
		case STIMER:
			Update_pokey_sound ( (uint16) addr, (uint8) value, 0, 2 );
			pokey.w_reg[new_address] = value;
			break;

		case SKREST:
			pokey.sk_status |= 0xe0;
			break;
		case POTGO:
			if ( pokey.sk_ctl & SK_FAST_POT )
				pokey.pot_scanline = 226;
			else
				pokey.pot_scanline = 1;
			break;
		case SEROUT:
			pokey.serial_out = value;
			pokey.serial_hw_status |= SER_OUT_OCCUPIED;
			if ( !(pokey.serial_hw_status & SER_REG_OCCUPIED) )
				pokey.serial_irq_time = pokey_cpu_clock + SER_PORT_IRQ_DELAY;
			break;
		case IRQEN:
			pokey.irq_enable = value;
			pokey.irq_status |= ~value & NOT_IRQ_SER_DONE;
			if ( (pokey.irq_enable & ~pokey.irq_status) == 0 )
				clear_irq_flag_6502 ( );

			break;
		case SKCTL:
			if ( (value & 0x70) && !(pokey.sk_ctl & 0x70) ) {
				if ( sio.current_device->status == SIO_STATUS_READ )
					pokey.serial_mode_status = SER_MODE_QUEUED;
				else
					pokey.serial_mode_status = SER_MODE_ON;
			}
			else if ( !(value & 0x70) )
				pokey.serial_mode_status = SER_MODE_OFF;

			pokey.sk_ctl = value;
			if ( value & SK_FAST_POT )
				pokey.pot_scanline = 226;

			pokey.current_key_code = 0;
			break;
	}

	return;

} /* end pokey_write_register */

void pokey_write_register_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	pokey_write_register ( addr, value );
}

/******************************************************************************
**  Function   :  pokey_init_scanline 
**                            
**  Objective  :  This function performs scanline actions required by pokey
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void pokey_init_scanline ( void ) {

	pokey_cpu_clock += 114;

	/*
	 * Handle pot updates per scanline
	 */
	if ( pokey.pot_scanline < 228 )
		pokey.pot_scanline++;

	memcpy ( pokey.pot, pokey.pc_pot, sizeof(BYTE)*8 ); 

	/*
	 * Reset keyboard scanning on counter reset
	 */
	if ( pokey.current_key_code == 0 ) {
		if ( !key_is_pressed ) {
			pokey.sk_status |= SK_KEY_PRESS;
			pokey.irq_status |= IRQ_KEY_PRESS;
			irq_sent_flag = 0;
		}
		key_is_pressed = 0;
	}

	/*
	 * Check for keys pressed and/or SIO ready
	 */
	if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
		pokey_check_keys_5200 ( );
	}
	else {
		pokey_check_SIO ( );
		sio_check();
		pokey_check_keys_800 ( );
	}

	return;

} /* end pokey_init_scanline  */

/******************************************************************************
**  Function   :  pokey_receive_SIO
**                            
**  Objective  :  This function takes the byte from SIO and sets interupts
**
**  Parameters :  data - byte from SIO
**                                                
**  return     :  NONE
**      
******************************************************************************/
void pokey_receive_SIO ( BYTE data ) {

	/*
	 * Check if we just turned on the serial mode (SKCTL)
	 *    Need this check for cassettes
	 */
	if ( pokey.serial_mode_status == SER_MODE_QUEUED ) {
		pokey.serial_mode_status = SER_MODE_ON;
		return;
	}
	else if ( !pokey.serial_mode_status )
		return;

	/*
	 * Check for Serial Byte already here and set overrun if so
	 */
	if ( !(pokey.irq_status & IRQ_SER_RED) ) {
		pokey.sk_status &= NOT_SK_SERIN_OVERRUN;
	}

	/*
	 * Get Next Byte and Interrupt if desired
	 */
	pokey.serial_in = data;
	if ( (pokey.irq_enable & IRQ_SER_RED) ) {
		pokey.irq_status &= NOT_IRQ_SER_RED;
		//if ( antic.run_6502_ptr != antic_do_nothing ) {
			set_irq_flag_6502 ( );
		//}
	}

} /* end pokey_receive_SIO  */

/******************************************************************************
**  Function   :  pokey_check_SIO
**                            
**  Objective  :  This function checks for needed action on the Serial I/O
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void pokey_check_SIO ( void ) {

	if ( !pokey.serial_mode_status )
		return;

	/*
	 * Data is started out the serial shift register, meaning the SEROUT
	 *   is empty and ready for more data
	 */
	if ( (pokey_cpu_clock == pokey.serial_irq_time) ) {
		if ( pokey.serial_hw_status & SER_OUT_OCCUPIED ) {
			pokey.serial_shift = pokey.serial_out;
			pokey.serial_out_time = pokey_cpu_clock + sio.current_device->baud - SER_PORT_IRQ_DELAY;
			pokey.serial_hw_status = SER_REG_OCCUPIED;
			if ( (pokey.irq_enable & IRQ_SER_NEED) ) {
				pokey.irq_status &= NOT_IRQ_SER_NEED;
				//if ( antic.run_6502_ptr != antic_do_nothing ) {
					set_irq_flag_6502 ( );
				//}
			}
		}
		else {
			pokey.serial_hw_status = 0;
			pokey.serial_out_time = 1;
			pokey.serial_irq_time = 1;
			if ( (pokey.irq_enable & IRQ_SER_DONE) ) {
				pokey.irq_status &= NOT_IRQ_SER_DONE;
				//if ( antic.run_6502_ptr != antic_do_nothing ) {
					set_irq_flag_6502 ( );
				//}
			}
		}

	} /* end if irq time */

	/*
	 * Send data from the output shift register
	 */
	if ( (pokey_cpu_clock == pokey.serial_out_time) ) {
		sio_receive ( pokey.serial_shift );
		pokey.serial_hw_status &= NOT_SER_REG_OCCUPIED;
		pokey.serial_irq_time = pokey_cpu_clock + SER_PORT_IRQ_DELAY;
	}

		/*
		 * Send BREAK
		 */
		if ( pokey.sk_ctl & SK_SER_BREAK ) {

		}

} /* end pokey_check_SIO  */

/******************************************************************************
**  Function   :  pokey_check_keys_5200
**                            
**  Objective  :  This function checks key interrupts on the 5200
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void pokey_check_keys_5200 ( void ) {

	if ( pokey.sk_ctl & SK_KEY_SCAN ) {
		if ( pokey.key_pressed[gtia.keypad][pokey.current_key_code>>1] ) {
			key_is_pressed = 1;
			pokey.sk_status &= NOT_SK_KEY_PRESS;
			pokey.kbcode = pokey.current_key_code | 0x1;
			if ( (pokey.irq_enable & IRQ_KEY_PRESS) && !irq_sent_flag ) {
				pokey.irq_status &= NOT_IRQ_KEY_PRESS;
				if ( antic.run_6502_ptr != antic_do_nothing ) {
					set_irq_flag_6502 ( );
					irq_sent_flag = 1;
				}
			}

		} /* end if normal key is pressed */

		/*
		 * Check for Shift key (top button for 5200) at keycode 0x3d
		 */
		if ( pokey.current_key_code == 0x3d ) {
			if ( pokey.shift_pressed[gtia.keypad] ) {
				pokey.sk_status &= NOT_SK_SHIFT_KEY; 
				if ( pokey.irq_enable & IRQ_BK_PRESS ) { 
					pokey.irq_status &= NOT_IRQ_BK_PRESS;
					if ( antic.run_6502_ptr != antic_do_nothing )
						set_irq_flag_6502 ( );
				}
			}
			else {
				pokey.sk_status |= SK_SHIFT_KEY;
				pokey.irq_status |= IRQ_BK_PRESS;
			}

		} /* end if shift key checked */

		pokey.current_key_code++;
		pokey.current_key_code &= 0x3f;

	} /* end if scanning enabled */

} /* end pokey_check_keys_5200  */

/******************************************************************************
**  Function   :  pokey_check_keys_800
**                            
**  Objective  :  This function checks key interrupts on the 5200
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void pokey_check_keys_800 ( void ) {

	if ( pokey.sk_ctl & SK_KEY_SCAN ) {

		/*
		 * Normal Key
		 */
		if ( pokey.key_pressed[0][pokey.current_key_code] ) {
			key_is_pressed = 1;
			pokey.sk_status &= NOT_SK_KEY_PRESS;
			pokey.kbcode = pokey.current_key_code;
			pokey.kbcode |= pokey.shift_pressed[0] ? KB_SHIFT_KEY : 0;
			pokey.kbcode |= pokey.ctrl_pressed ? KB_CTRL_KEY : 0;
			if ( (pokey.irq_enable & IRQ_KEY_PRESS) && !irq_sent_flag ) {
				pokey.irq_status &= NOT_IRQ_KEY_PRESS;
				if ( antic.run_6502_ptr != antic_do_nothing ) {
					set_irq_flag_6502 ( );
					irq_sent_flag = 1;
				}
			}

		} /* end if normal key is pressed */

		/*
		 * Check for Shift key 
		 */
		if ( pokey.current_key_code == 0x3d ) {
			if ( pokey.shift_pressed[0] ) {
				pokey.sk_status &= NOT_SK_SHIFT_KEY; 
				pokey.kbcode |= KB_SHIFT_KEY;
			}
			else {
				pokey.sk_status |= SK_SHIFT_KEY;
				pokey.kbcode &= NOT_KB_SHIFT_KEY;
			}

		} /* end if shift key checked */

		/*
		 * Check for Ctrl key 
		 */
		if ( pokey.current_key_code == 0x3f ) {
			if ( pokey.ctrl_pressed ) {
				pokey.kbcode |= KB_CTRL_KEY;
			}
			else {
				pokey.kbcode &= NOT_KB_CTRL_KEY;
			}
		}

		/*
		 * Check for Break key 
		 */
		if ( pokey.current_key_code == 0x3c ) {
			if ( pokey.break_pressed ) {
				if ( pokey.irq_enable & IRQ_BK_PRESS ) { 
					pokey.irq_status &= NOT_IRQ_BK_PRESS;
					if ( antic.run_6502_ptr != antic_do_nothing )
						set_irq_flag_6502 ( );
				}
			}
			else {
				pokey.irq_status |= IRQ_BK_PRESS;
			}
		}

		pokey.current_key_code++;
		pokey.current_key_code &= 0x3f;

	} /* end if scanning enabled */

} /* end pokey_check_keys_800  */
