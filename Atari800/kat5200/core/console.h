/******************************************************************************
*
* FILENAME: console.h
*
* DESCRIPTION:  This contains console struct and functions used in main
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   02/06/07  bberlin      Creation, breakout from main
******************************************************************************/
#ifndef console_h
#define console_h

#include "cart.h"
#include "boom6502.h"
#include "antic.h"
#include "gtia.h"
#include "pokey.h"
#include "pia.h"

typedef enum {
	MACHINE_TYPE_MIN,
	MACHINE_TYPE_5200,
	MACHINE_TYPE_800,
	MACHINE_TYPE_XL,
} e_machine_type;

struct atari {
	struct cpu *cpu;
	struct antic *antic;
	struct gtia *gtia;
	struct pokey *pokey;
	struct pia *pia;
	t_cart *cart;
	t_cart *cart_right;
	unsigned int frame_number;
	e_machine_type machine_type;
	e_cart_type mapping;
	int frames_per_sec;
	unsigned int scanlines;
	int debug_mode;
	int throttle;
	int ram_size;
	int basic_enable;
	int option_key_pressed;
	int start_key_pressed;
};

void console_init ( void );
void console_reset (e_machine_type type, int system_type, int ram, int freq);
struct atari * console_get_ptr ( void );
void console_set_machine_type ( e_machine_type type );
e_machine_type console_get_machine_type ( void );
void console_set_media_type ( e_cart_type type );
void console_set_throttle ( int throttle );
void console_set_debug_mode ( int debug_mode );

#endif
