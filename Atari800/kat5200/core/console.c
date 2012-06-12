/******************************************************************************
*
* FILENAME: console.c
*
* DESCRIPTION:  This contains functions for running the system
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   02/06/07  bberlin      Added call to 'config_update'
******************************************************************************/
#include <stdio.h>
#include "console.h"
#include "memory.h"
#include "sio.h"
#include "debug.h"

/*********************************/
/* Declare the console variable  */
/*********************************/
static struct atari console;
extern struct antic antic;
extern struct gtia gtia;
extern struct pokey pokey;
extern struct pia pia;
extern struct cpu cpu6502;
extern struct debug debug;
extern t_cart cart;
extern t_cart cart_right;

/******************************************************************************
**  Function   :  console_init                
**                                         
**  Objective  :  Assign chip structs to the console
**                                       
**  Parameters :  NONE                  
**                                     
**  return     :  NONE
**             
******************************************************************************/ 
void console_init (void) {

	console.frame_number = 0;
	console.debug_mode = 0;
	console.throttle = 100;
	console.ram_size = 64;
	console.frames_per_sec = 60;
	console.scanlines = 262;

	console.cpu = &cpu6502;
	console.antic = &antic;
	console.gtia = &gtia;
	console.pokey = &pokey;
	console.pia = &pia;
	console.cart = &cart;
	console.cart_right = &cart_right;

	console.cart->loaded = 0;
	console.cart_right->loaded = 0;
	console.basic_enable = 0;
	console.option_key_pressed = 0;
	console.start_key_pressed = 0;

} /* end console_init */

/******************************************************************************
**  Function   :  console_reset                
**                                         
**  Objective  :  Reset the system
**                                       
**  Parameters :  type        - machine type 5200, 800, XL/XE
**                system_type - PAL or NTSC
**                ram         - amount of memory to emulate
**                freq        - frequency to run Pokey Sound at
**                                     
**  return     :  NONE
**
******************************************************************************/ 
void console_reset ( e_machine_type type, int system_type, int ram, int freq) {

	console.machine_type = type;
	console.ram_size = ram;
	mem_reset ( );
	antic_init ();
	gtia_init ( system_type );
	pokey_init ( freq );
	pia_init ();
	sio_reset ();
	reset_6502 ();

	if ( system_type == PAL ) {
		console.frames_per_sec = 50;
		console.scanlines = 312;
	}
	else {
		console.frames_per_sec = 60;
		console.scanlines = 262;
	}

	console.option_key_pressed = 0;
	console.start_key_pressed = 0;

	if ( !console.basic_enable && (console.machine_type == MACHINE_TYPE_XL) )
		console.option_key_pressed = 1;

	if ( console.mapping == CART_CASSETTE ) {
		console.start_key_pressed = 2;
	}

} /* end console_reset */

/******************************************************************************
**  Function   :  console_get_ptr
**                                                
**  Objective  :  This grabs the address of the console variable
**                                               
**  Parameters :  NONE                          
**                                             
**  return     :  pointer to atari
**
******************************************************************************/ 
struct atari *console_get_ptr ( void ) {

	return &console;

} /* end console_get_ptr */

/******************************************************************************
**  Function   :  console_get_set_functions
**                                                
**  Objective  :  These functions get and set certain parameters
**                                               
**  Parameters :  Varies
**                                             
**  return     :  Varies
**
******************************************************************************/ 
void console_set_machine_type ( e_machine_type type ) {

	console.machine_type = type;

} /* end console_set_machine_type */

e_machine_type console_get_machine_type ( void ) {

	return console.machine_type;

} /* end console_set_machine_type */

void console_set_media_type ( e_cart_type type ) {

	console.mapping = type;

} /* end console_set_cart_type */

void console_set_throttle ( int throttle ) {

	console.throttle = throttle;

} /* end console_set_throttle */

void console_set_debug_mode ( int debug_mode ) {

	console.debug_mode = debug_mode;

} /* end console_set_debug_mode */
