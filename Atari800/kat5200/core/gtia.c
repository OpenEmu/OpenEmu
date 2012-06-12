/******************************************************************************
*
* FILENAME: gtia.c
*
* DESCRIPTION:  This contains functions for emulating the GTIA video chip
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     07/18/04  bberlin      Creation
* 0.2     12/09/05  bberlin      Update GRAFx register to also transfer data
*                                  (Kaboom fix)
* 0.2     03/01/06  bberlin      Changed so that when there is no dma the PM
*                                  transfer functions are used instead of do
*                                  nothing (Pitfall II Logo fix)
* 0.3.0   04/07/06  bberlin      Changed option_pressed to 0 in 'gtia_init'
*                                  (Xari Arena fix)
* 0.4.0   04/22/06  bberlin      Implemented PRIOR=3 in 'gtia_init_prior_array'
*                                  (Castle Crises fix)
* 0.4.0   04/26/06  bberlin      Changed read CONSOL to use reg value & 0xb in
*                                  'gtia_read_register' (Xari Arena and BC)
* 0.4.0   05/29/06  bberlin      Changed 'gtia_init_prior_array' not to use pm
*                                  set and clear variables in lum-only modes
*                                   (AE PM graphics)
* 0.4.0   07/11/06  bberlin      Added 'gtia_set_byte_mode_2_artif' to handle
*                                  hi res mode colors
* 0.5.0   08/25/06  bberlin      Added 'gtia_bottom_button' functions here
* 0.5.0   08/28/06  bberlin      Changed all color vars to unsigned in case we
*                                  want to support 32-bit pixels
* 0.6.0   09/22/08  bberlin      Put back in set and clear variables but 
*                                  changed clear (Bounty Bob Counter)
* 0.6.0   09/24/08  bberlin      Use correct color array for gtia mode 3
******************************************************************************/
#include <string.h>
#include "console.h"
#include "gtia.h"
#include "antic.h"
#include "debug.h"

/*
 * Defines for our Put Pixel functions
 */
#define GTIA_PUT_PIXEL(color) 	switch ( gtia.d_surface.bytes_per_pixel ) { \
		case 1: \
			*(gtia.d_surface.current++) = (color); \
			break; \
		case 2: \
			*((UINT16 *)(gtia.d_surface.current)) = (color); \
			gtia.d_surface.current+=2; \
			break; \
		case 4: \
			*(UINT32 *)(gtia.d_surface.current) = (color); \
			gtia.d_surface.current+=4; \
			break; \
	} \

#define GTIA_PUT_PIXEL_32(color) *((UINT32 *)(gtia.d_surface.current)) = (color); \
	                              gtia.d_surface.current+=4; \

#define GTIA_PUT_PIXEL_16(color) *((UINT16 *)(gtia.d_surface.current)) = (color); \
	                              gtia.d_surface.current+=2; \

#define GTIA_PUT_PIXEL_8(color) *(UINT8 *)(gtia.d_surface.current++) = (color);

#define BITS_TO_DRAW 32
#define BIT_SHIFT 0x80000000
#define BIT_DIVIDE 5

struct gtia gtia;
extern struct antic antic;

/******************************************************************************
**  Function   :  gtia_init
**                            
**  Objective  :  This function initializes GTIA registers
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_init ( int system_type ) {

	int i = 0;

	for ( i = 0; i < 0x20; ++i ) {
		gtia.w_reg[i] = 0x0f;
		gtia.r_reg[i] = 0x0f;
	}

	gtia.pal_or_ntsc = system_type;

	gtia.option_pressed = 1;
	gtia.select_pressed = 1;
	gtia.start_pressed = 1;
	gtia.internal_speaker = 1;

	if ( console_get_machine_type() == MACHINE_TYPE_XL )
		gtia.option_pressed = console_get_ptr()->basic_enable;

	if ( console_get_ptr()->mapping == CART_CASSETTE )
		gtia.start_pressed = 0;

	gtia.keypad = 0;
	gtia.pot_enable = 0;
	gtia.console_reg = 0;
	gtia.console_mask = 0xf;

	/*
	 * Initialize PRIOR stuff
	 */
	gtia_init_prior_array ( );
	gtia.prior_reg = 0;
	gtia.prior_mask = 0;

	/*
	 * Initialize pm-graphics array
	 */
	gtia.graph_p0_reg = 0;
	gtia.graph_p1_reg = 0;
	gtia.graph_p2_reg = 0;
	gtia.graph_p3_reg = 0;
	gtia.graph_m_reg = 0;

	/*
	 * Initialize playfield start stuff
	 */
	gtia.pf_x_start_ptr = &antic.normal_pixel_start;
	
	/*
	 * Initialize collision arrays
	 */
	memset ( gtia.collision_p0pf, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_p1pf, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_p2pf, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_p3pf, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_p0pl, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_p1pl, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_p2pl, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_p3pl, 0, sizeof(BYTE)*4 );

	memset ( gtia.collision_m0pf, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_m1pf, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_m2pf, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_m3pf, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_m0pl, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_m1pl, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_m2pl, 0, sizeof(BYTE)*4 );
	memset ( gtia.collision_m3pl, 0, sizeof(BYTE)*4 );

	/*
	 * Initialize player missle lines
	 */
	memset ( gtia.graph_p0_line, 0, sizeof(BYTE)*65 );
	memset ( gtia.graph_p1_line, 0, sizeof(BYTE)*65 );
	memset ( gtia.graph_p2_line, 0, sizeof(BYTE)*65 );
	memset ( gtia.graph_p3_line, 0, sizeof(BYTE)*65 );
	memset ( gtia.graph_m0_line, 0, sizeof(BYTE)*65 );
	memset ( gtia.graph_m1_line, 0, sizeof(BYTE)*65 );
	memset ( gtia.graph_m2_line, 0, sizeof(BYTE)*65 );
	memset ( gtia.graph_m3_line, 0, sizeof(BYTE)*65 );

	/*
	 * Initialize graphics control and triggers
	 */
	gtia.latch_triggers = 0;
	gtia.player_dma = 0;
	gtia.missle_dma = 0;
	gtia.vdelay_reg = 0;
	for ( i = 0; i < 4; ++i ) {
		gtia.trig[i] = 1;
		gtia.trig_held_down[i] = 0;
	}
	if ( console_get_ptr()->machine_type == MACHINE_TYPE_XL )
		gtia.trig[3] = console_get_ptr()->cart->loaded;

	/*
	 * Initialize vertical delay for no delay
	 */
	gtia.transfer_p0_ptr[0] = gtia_transfer_p0;
	gtia.transfer_p1_ptr[0] = gtia_transfer_p1;
	gtia.transfer_p2_ptr[0] = gtia_transfer_p2;
	gtia.transfer_p3_ptr[0] = gtia_transfer_p3;
	gtia.transfer_m0_ptr[0] = gtia_transfer_m0;
	gtia.transfer_m1_ptr[0] = gtia_transfer_m1;
	gtia.transfer_m2_ptr[0] = gtia_transfer_m2;
	gtia.transfer_m3_ptr[0] = gtia_transfer_m3;
	gtia.transfer_p0_ptr[1] = gtia_transfer_p0;
	gtia.transfer_p1_ptr[1] = gtia_transfer_p1;
	gtia.transfer_p2_ptr[1] = gtia_transfer_p2;
	gtia.transfer_p3_ptr[1] = gtia_transfer_p3;
	gtia.transfer_m0_ptr[1] = gtia_transfer_m0;
	gtia.transfer_m1_ptr[1] = gtia_transfer_m1;
	gtia.transfer_m2_ptr[1] = gtia_transfer_m2;
	gtia.transfer_m3_ptr[1] = gtia_transfer_m3;

	/*
	 * Initialize missle color register
	 */
	gtia.missle0_mask = PM0_MASK;
	gtia.missle1_mask = PM1_MASK;
	gtia.missle2_mask = PM2_MASK;
	gtia.missle3_mask = PM3_MASK;

	/*
	 * GTIA Mode Stuff
	 * FIXME: Init function pointer
	 */
	gtia.gtia_mode = 0;
	gtia.gtia_mode_col_ptr = gtia.gtia_mode_col[0];
	gtia_set_mode_2_draw_ptr ();

	gtia.prior_overlap = 0;

	return;

} /* end gtia_init */

/******************************************************************************
**  Function   :  gtia_set_screen
**                            
**  Objective  :  This function is called by the PC interface to set up the 
**                video surface to write to
**
**  Parameters :  surface     - memory for gtia to write pixel data to
**                color_array - color palette
**                width       - width of line in bytes
**                bpp         - bytes per pixel
**                line_only   - set if surface is a single line
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_set_screen ( void *surface, unsigned int *color_array, int width, int bpp, 
                         int line_only ) {

	gtia.d_surface.start = surface;
	gtia.d_surface.line_only = line_only;
	gtia.d_surface.bytes_per_pixel = bpp;
	gtia.d_surface.width = width;
	gtia.color_array = color_array;

} /* end gtia_set_screen */

/******************************************************************************
**  Function   :  gtia_set_mode_2_draw_ptr
**                            
**  Objective  :  This function sets up the mode 2 draw pointer.  Needed for
**                 the GTIA modes.
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/
void gtia_set_mode_2_draw_ptr ( void ) {

	switch ( gtia.gtia_mode ) {
		case 0: 
			gtia.gtia_set_byte_mode_2_ptr = gtia_set_byte_mode_2;
			break;
		case 1: 
			gtia.gtia_mode_col_ptr = gtia.gtia_mode_col[0];
			gtia.gtia_set_byte_mode_2_ptr = gtia_set_byte_mode_2_g1g3;
			break;
		case 2: 
			gtia.gtia_set_byte_mode_2_ptr = gtia_set_byte_mode_2_g2;
			break;
		case 3: 
			gtia.gtia_mode_col_ptr = gtia.gtia_mode_col[1];
			gtia.gtia_set_byte_mode_2_ptr = gtia_set_byte_mode_2_g1g3;
			break;
	}

} /* end gtia_set_mode_2_draw_ptr */

/******************************************************************************
**  Function   :  gtia_read_register
**                            
**  Objective  :  This function reads from the input register
**
**  Parameters :  addr - 6502 memory address of the register
**                                                
**  return     :  value at register
**      
******************************************************************************/ 
BYTE gtia_read_register ( WORD addr ) {

	BYTE new_address = addr & 0x1f;

	switch ( new_address ) {

		case M0PF:
			gtia.m0pf_reg = gtia.collision_m0pf[0] ? 0x1 : 0;
			gtia.m0pf_reg |= gtia.collision_m0pf[1] ? 0x2 : 0;
			gtia.m0pf_reg |= gtia.collision_m0pf[2] ? 0x4 : 0;
			gtia.m0pf_reg |= gtia.collision_m0pf[3] ? 0x8 : 0;
			return gtia.m0pf_reg;
			break;
		case M1PF:
			gtia.m1pf_reg = gtia.collision_m1pf[0] ? 0x1 : 0;
			gtia.m1pf_reg |= gtia.collision_m1pf[1] ? 0x2 : 0;
			gtia.m1pf_reg |= gtia.collision_m1pf[2] ? 0x4 : 0;
			gtia.m1pf_reg |= gtia.collision_m1pf[3] ? 0x8 : 0;
			return gtia.m1pf_reg;
			break;
		case M2PF:
			gtia.m2pf_reg = gtia.collision_m2pf[0] ? 0x1 : 0;
			gtia.m2pf_reg |= gtia.collision_m2pf[1] ? 0x2 : 0;
			gtia.m2pf_reg |= gtia.collision_m2pf[2] ? 0x4 : 0;
			gtia.m2pf_reg |= gtia.collision_m2pf[3] ? 0x8 : 0;
			return gtia.m2pf_reg;
			break;
		case M3PF:
			gtia.m3pf_reg = gtia.collision_m3pf[0] ? 0x1 : 0;
			gtia.m3pf_reg |= gtia.collision_m3pf[1] ? 0x2 : 0;
			gtia.m3pf_reg |= gtia.collision_m3pf[2] ? 0x4 : 0;
			gtia.m3pf_reg |= gtia.collision_m3pf[3] ? 0x8 : 0;
			return gtia.m3pf_reg;
			break;

		case M0PL:
			gtia.m0pl_reg = gtia.collision_m0pl[0] ? 0x1 : 0;
			gtia.m0pl_reg |= gtia.collision_m0pl[1] ? 0x2 : 0;
			gtia.m0pl_reg |= gtia.collision_m0pl[2] ? 0x4 : 0;
			gtia.m0pl_reg |= gtia.collision_m0pl[3] ? 0x8 : 0;
			return gtia.m0pl_reg;
			break;
		case M1PL:
			gtia.m1pl_reg = gtia.collision_m1pl[0] ? 0x1 : 0;
			gtia.m1pl_reg |= gtia.collision_m1pl[1] ? 0x2 : 0;
			gtia.m1pl_reg |= gtia.collision_m1pl[2] ? 0x4 : 0;
			gtia.m1pl_reg |= gtia.collision_m1pl[3] ? 0x8 : 0;
			return gtia.m1pl_reg;
			break;
		case M2PL:
			gtia.m2pl_reg = gtia.collision_m2pl[0] ? 0x1 : 0;
			gtia.m2pl_reg |= gtia.collision_m2pl[1] ? 0x2 : 0;
			gtia.m2pl_reg |= gtia.collision_m2pl[2] ? 0x4 : 0;
			gtia.m2pl_reg |= gtia.collision_m2pl[3] ? 0x8 : 0;
			return gtia.m2pl_reg;
			break;
		case M3PL:
			gtia.m3pl_reg = gtia.collision_m3pl[0] ? 0x1 : 0;
			gtia.m3pl_reg |= gtia.collision_m3pl[1] ? 0x2 : 0;
			gtia.m3pl_reg |= gtia.collision_m3pl[2] ? 0x4 : 0;
			gtia.m3pl_reg |= gtia.collision_m3pl[3] ? 0x8 : 0;
			return gtia.m3pl_reg;
			break;

		case P0PF:
			gtia.p0pf_reg = gtia.collision_p0pf[0] ? 0x1 : 0;
			gtia.p0pf_reg |= gtia.collision_p0pf[1] ? 0x2 : 0;
			gtia.p0pf_reg |= gtia.collision_p0pf[2] ? 0x4 : 0;
			gtia.p0pf_reg |= gtia.collision_p0pf[3] ? 0x8 : 0;
			return gtia.p0pf_reg;
			break;
		case P1PF:
			gtia.p1pf_reg = gtia.collision_p1pf[0] ? 0x1 : 0;
			gtia.p1pf_reg |= gtia.collision_p1pf[1] ? 0x2 : 0;
			gtia.p1pf_reg |= gtia.collision_p1pf[2] ? 0x4 : 0;
			gtia.p1pf_reg |= gtia.collision_p1pf[3] ? 0x8 : 0;
			return gtia.p1pf_reg;
			break;
		case P2PF:
			gtia.p2pf_reg = gtia.collision_p2pf[0] ? 0x1 : 0;
			gtia.p2pf_reg |= gtia.collision_p2pf[1] ? 0x2 : 0;
			gtia.p2pf_reg |= gtia.collision_p2pf[2] ? 0x4 : 0;
			gtia.p2pf_reg |= gtia.collision_p2pf[3] ? 0x8 : 0;
			return gtia.p2pf_reg;
			break;
		case P3PF:
			gtia.p3pf_reg = gtia.collision_p3pf[0] ? 0x1 : 0;
			gtia.p3pf_reg |= gtia.collision_p3pf[1] ? 0x2 : 0;
			gtia.p3pf_reg |= gtia.collision_p3pf[2] ? 0x4 : 0;
			gtia.p3pf_reg |= gtia.collision_p3pf[3] ? 0x8 : 0;
			return gtia.p3pf_reg;
			break;

		case P0PL:
			gtia.p0pl_reg = 0;
			gtia.p0pl_reg |= gtia.collision_p0pl[1] ? 0x2 : 0;
			gtia.p0pl_reg |= gtia.collision_p0pl[2] ? 0x4 : 0;
			gtia.p0pl_reg |= gtia.collision_p0pl[3] ? 0x8 : 0;
			return gtia.p0pl_reg;
			break;
		case P1PL:
			gtia.p1pl_reg = gtia.collision_p1pl[0] ? 0x1 : 0;
			gtia.p1pl_reg |= gtia.collision_p1pl[2] ? 0x4 : 0;
			gtia.p1pl_reg |= gtia.collision_p1pl[3] ? 0x8 : 0;
			return gtia.p1pl_reg;
			break;
		case P2PL:
			gtia.p2pl_reg = gtia.collision_p2pl[0] ? 0x1 : 0;
			gtia.p2pl_reg |= gtia.collision_p2pl[1] ? 0x2 : 0;
			gtia.p2pl_reg |= gtia.collision_p2pl[3] ? 0x8 : 0;
			return gtia.p2pl_reg;
			break;
		case P3PL:
			gtia.p3pl_reg = gtia.collision_p3pl[0] ? 0x1 : 0;
			gtia.p3pl_reg |= gtia.collision_p3pl[1] ? 0x2 : 0;
			gtia.p3pl_reg |= gtia.collision_p3pl[2] ? 0x4 : 0;
			return gtia.p3pl_reg;
			break;

		/*
		 * Triggers set by outside.  Must consider latch trigger
		 */
		case TRIG0:
			return gtia.trig[0];
			break;
		case TRIG1:
			return gtia.trig[1];
			break;
		case TRIG2:
			return gtia.trig[2];
			break;
		case TRIG3:
			return gtia.trig[3];
			break;

		/*
		 * PAL or NTSC television  (set by outside)
		 */
		case PALR:
			return gtia.pal_or_ntsc;
			break;

		/*
		 * Button Inputs/Speaker
		 */
		case CONSOL:
			if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
				return gtia.console_reg & 0xb;
			}
			else {
				gtia.console_reg = gtia.start_pressed ? 0x1 : 0;
				gtia.console_reg |= gtia.select_pressed ? 0x2 : 0;
				gtia.console_reg |= gtia.option_pressed ? 0x4 : 0;
				gtia.console_reg |= gtia.internal_speaker ? 0x8 : 0;

				if ( console_get_ptr()->option_key_pressed ) {
					console_get_ptr()->option_key_pressed--;
					gtia.option_pressed = 1;
				}

				if ( console_get_ptr()->start_key_pressed ) {
					console_get_ptr()->start_key_pressed--;
					if ( !console_get_ptr()->start_key_pressed )
						gtia.start_pressed = 1;
				}

				return gtia.console_reg & gtia.console_mask;
			}
			break;

		default: return 0x0f; break;
	}

	return 0x0f;

} /* end gtia_read_register */

BYTE gtia_read_register_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return gtia_read_register ( addr );
}

/******************************************************************************
**  Function   :  gtia_write_register
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
void gtia_write_register ( WORD addr, BYTE value ) {

	BYTE new_address = addr & 0x1f;
	BYTE temp_value;
	int i;

	gtia.w_reg[new_address] = value;

	switch ( new_address ) {

		/*
		 * Control of P-M DMA and latch triggers
		 */
		case GRACTL:
			gtia.latch_triggers = value & 0x04;
			gtia.player_dma = value & 0x02;
			gtia.missle_dma = value & 0x01;

			/*
			 * if latch cleared, reset triggers
			 */
			if ( !gtia.latch_triggers ) {
				for ( i = 0; i < 4; ++i ) {
					if ( !gtia.trig_held_down[i] )
						gtia.trig[i] = 1;
				}
			}

			if ( console_get_ptr()->machine_type == MACHINE_TYPE_XL )
				gtia.trig[3] = console_get_ptr()->cart->loaded;
			
			/*
			 * Assign pm transfer functions
			 */
			//if ( gtia.player_dma ) {
				if ( gtia.vdelay_reg & 0x10 )
					gtia.transfer_p0_ptr[0] = gtia_transfer_nothing;
				else
					gtia.transfer_p0_ptr[0] = gtia_transfer_p0;
				if ( gtia.vdelay_reg & 0x20 )
					gtia.transfer_p1_ptr[0] = gtia_transfer_nothing;
				else
					gtia.transfer_p1_ptr[0] = gtia_transfer_p1;
				if ( gtia.vdelay_reg & 0x40 )
					gtia.transfer_p2_ptr[0] = gtia_transfer_nothing;
				else
					gtia.transfer_p2_ptr[0] = gtia_transfer_p2;
				if ( gtia.vdelay_reg & 0x80 )
					gtia.transfer_p3_ptr[0] = gtia_transfer_nothing;
				else
					gtia.transfer_p3_ptr[0] = gtia_transfer_p3;

				gtia.transfer_p0_ptr[1] = gtia_transfer_p0;
				gtia.transfer_p1_ptr[1] = gtia_transfer_p1;
				gtia.transfer_p2_ptr[1] = gtia_transfer_p2;
				gtia.transfer_p3_ptr[1] = gtia_transfer_p3;

			//} /* end if player dma enabled */

			/*
			else {
				gtia.transfer_p0_ptr[0] = gtia_transfer_p0;
				gtia.transfer_p1_ptr[0] = gtia_transfer_p1;
				gtia.transfer_p2_ptr[0] = gtia_transfer_p2;
				gtia.transfer_p3_ptr[0] = gtia_transfer_p3;
				gtia.transfer_p0_ptr[1] = gtia_transfer_p0;
				gtia.transfer_p1_ptr[1] = gtia_transfer_p1;
				gtia.transfer_p2_ptr[1] = gtia_transfer_p2;
				gtia.transfer_p3_ptr[1] = gtia_transfer_p3;

			}*/ /* end else player dma not enabled */

			//if ( gtia.missle_dma ) {
				if ( gtia.vdelay_reg & 0x01 )
					gtia.transfer_m0_ptr[0] = gtia_transfer_nothing;
				else
					gtia.transfer_m0_ptr[0] = gtia_transfer_m0;
				if ( gtia.vdelay_reg & 0x02 )
					gtia.transfer_m1_ptr[0] = gtia_transfer_nothing;
				else
					gtia.transfer_m1_ptr[0] = gtia_transfer_m1;
				if ( gtia.vdelay_reg & 0x04 )
					gtia.transfer_m2_ptr[0] = gtia_transfer_nothing;
				else
					gtia.transfer_m2_ptr[0] = gtia_transfer_m2;
				if ( gtia.vdelay_reg & 0x08 )
					gtia.transfer_m3_ptr[0] = gtia_transfer_nothing;
				else
					gtia.transfer_m3_ptr[0] = gtia_transfer_m3;

				gtia.transfer_m0_ptr[1] = gtia_transfer_m0;
				gtia.transfer_m1_ptr[1] = gtia_transfer_m1;
				gtia.transfer_m2_ptr[1] = gtia_transfer_m2;
				gtia.transfer_m3_ptr[1] = gtia_transfer_m3;

			//} /* end if missle dma enabled */

			/*
			else {
				gtia.transfer_m0_ptr[0] = gtia_transfer_m0;
				gtia.transfer_m1_ptr[0] = gtia_transfer_m1;
				gtia.transfer_m2_ptr[0] = gtia_transfer_m2;
				gtia.transfer_m3_ptr[0] = gtia_transfer_m3;
				gtia.transfer_m0_ptr[1] = gtia_transfer_m0;
				gtia.transfer_m1_ptr[1] = gtia_transfer_m1;
				gtia.transfer_m2_ptr[1] = gtia_transfer_m2;
				gtia.transfer_m3_ptr[1] = gtia_transfer_m3;

			}*/ /* end else missle dma not enabled */

			break;

		/*
		 * Vertical Delay             
		 *   Every odd scanline if set 
		 */
		case VDELAY:
			gtia.vdelay_reg = value;

			if ( value & 0x10 )
				gtia.transfer_p0_ptr[0] = gtia_transfer_nothing;
			else
				gtia.transfer_p0_ptr[0] = gtia_transfer_p0;

			if ( value & 0x20 )
				gtia.transfer_p1_ptr[0] = gtia_transfer_nothing;
			else
				gtia.transfer_p1_ptr[0] = gtia_transfer_p1;
		
			if ( value & 0x40 )
				gtia.transfer_p2_ptr[0] = gtia_transfer_nothing;
			else
				gtia.transfer_p2_ptr[0] = gtia_transfer_p2;

			if ( value & 0x80 )
				gtia.transfer_p3_ptr[0] = gtia_transfer_nothing;
			else
				gtia.transfer_p3_ptr[0] = gtia_transfer_p3;

			if ( value & 0x01 )
				gtia.transfer_m0_ptr[0] = gtia_transfer_nothing;
			else
				gtia.transfer_m0_ptr[0] = gtia_transfer_m0;

			if ( value & 0x02 )
				gtia.transfer_m1_ptr[0] = gtia_transfer_nothing;
			else
				gtia.transfer_m1_ptr[0] = gtia_transfer_m1;

			if ( value & 0x04 )
				gtia.transfer_m2_ptr[0] = gtia_transfer_nothing;
			else
				gtia.transfer_m2_ptr[0] = gtia_transfer_m2;

			if ( value & 0x08 )
				gtia.transfer_m3_ptr[0] = gtia_transfer_nothing;
			else
				gtia.transfer_m3_ptr[0] = gtia_transfer_m3;

			break;

		/*
		 * Player-missle graphics registers 
		 *   when doing direct writes here going ahead
		 *   and transfer to gtia draw array
		 */
		case GRAFP0:
			gtia.graph_p0_reg = value;
			gtia.transfer_p0_ptr[antic.dl_scanline & 0x1] (); 
			break;
		case GRAFP1:
			gtia.graph_p1_reg = value;
			gtia.transfer_p1_ptr[antic.dl_scanline & 0x1] (); 
			break;
		case GRAFP2:
			gtia.graph_p2_reg = value;
			gtia.transfer_p2_ptr[antic.dl_scanline & 0x1] (); 
			break;
		case GRAFP3:
			gtia.graph_p3_reg = value;
			gtia.transfer_p3_ptr[antic.dl_scanline & 0x1] (); 
			break;
		case GRAFM:
			gtia.graph_m_reg = value;
			gtia.transfer_m0_ptr[antic.dl_scanline & 0x1] (); 
			gtia.transfer_m1_ptr[antic.dl_scanline & 0x1] (); 
			gtia.transfer_m2_ptr[antic.dl_scanline & 0x1] (); 
			gtia.transfer_m3_ptr[antic.dl_scanline & 0x1] (); 
			break;

		/*
		 * Horizontal position of players and missles
		 *   multiply by 2 to go from color clocks to pixels
		 */
		case HPOSP0:
			gtia.hpos_p0_reg = value << 1;
			gtia.transfer_p0_ptr[antic.dl_scanline & 0x1] (); 
			break;
		case HPOSP1:
			gtia.hpos_p1_reg = value << 1;
			gtia.transfer_p1_ptr[antic.dl_scanline & 0x1] (); 
			break;
		case HPOSP2:
			gtia.hpos_p2_reg = value << 1;
			gtia.transfer_p2_ptr[antic.dl_scanline & 0x1] (); 
			break;
		case HPOSP3:
			gtia.hpos_p3_reg = value << 1;
			gtia.transfer_p3_ptr[antic.dl_scanline & 0x1] (); 
			break;
		case HPOSM0:
			gtia.hpos_m0_reg = value << 1;
			gtia.transfer_m0_ptr[antic.dl_scanline & 0x1] ();
			break;
		case HPOSM1:
			gtia.hpos_m1_reg = value << 1;
			gtia.transfer_m1_ptr[antic.dl_scanline & 0x1] ();
			break;
		case HPOSM2:
			gtia.hpos_m2_reg = value << 1;
			gtia.transfer_m2_ptr[antic.dl_scanline & 0x1] ();
			break;
		case HPOSM3:
			gtia.hpos_m3_reg = value << 1;
			gtia.transfer_m3_ptr[antic.dl_scanline & 0x1] ();
			break;

		/*
		 * Horizontal size of players and missles
		 *   multiply by 2 to go from color clocks to pixels
		 */
		case SIZEP0:
			switch ( value & 0x3 ) {
				case 0: gtia.size_p0_reg = 2; break;
				case 1: gtia.size_p0_reg = 4; break;
				case 2: gtia.size_p0_reg = 2; break;
				case 3: gtia.size_p0_reg = 8; break;
			}
			break;
		case SIZEP1:
			switch ( value & 0x3 ) {
				case 0: gtia.size_p1_reg = 2; break;
				case 1: gtia.size_p1_reg = 4; break;
				case 2: gtia.size_p1_reg = 2; break;
				case 3: gtia.size_p1_reg = 8; break;
			}
			break;
		case SIZEP2:
			switch ( value & 0x3 ) {
				case 0: gtia.size_p2_reg = 2; break;
				case 1: gtia.size_p2_reg = 4; break;
				case 2: gtia.size_p2_reg = 2; break;
				case 3: gtia.size_p2_reg = 8; break;
			}
			break;
		case SIZEP3:
			switch ( value & 0x3 ) {
				case 0: gtia.size_p3_reg = 2; break;
				case 1: gtia.size_p3_reg = 4; break;
				case 2: gtia.size_p3_reg = 2; break;
				case 3: gtia.size_p3_reg = 8; break;
			}
			break;
		case SIZEM:
			switch ( value & 0x3 ) {
				case 0: gtia.size_m0_reg = 2; break;
				case 1: gtia.size_m0_reg = 4; break;
				case 2: gtia.size_m0_reg = 2; break;
				case 3: gtia.size_m0_reg = 8; break;
			}
			switch ( (value >> 2) & 0x3 ) {
				case 0: gtia.size_m1_reg = 2; break;
				case 1: gtia.size_m1_reg = 4; break;
				case 2: gtia.size_m1_reg = 2; break;
				case 3: gtia.size_m1_reg = 8; break;
			}
			switch ( (value >> 4) & 0x3 ) {
				case 0: gtia.size_m2_reg = 2; break;
				case 1: gtia.size_m2_reg = 4; break;
				case 2: gtia.size_m2_reg = 2; break;
				case 3: gtia.size_m2_reg = 8; break;
			}
			switch ( (value >> 6) & 0x3 ) {
				case 0: gtia.size_m3_reg = 2; break;
				case 1: gtia.size_m3_reg = 4; break;
				case 2: gtia.size_m3_reg = 2; break;
				case 3: gtia.size_m3_reg = 8; break;
			}

			break;

		/*
		 * Color registers
		 */
		case COLPM0:
			gtia.color_pm0_reg = value & 0xfe;
			gtia.col_pm0 = gtia.color_array[gtia.color_pm0_reg];
			gtia.col_pm0_mode2_set = gtia.color_array[(value & 0xf0) | (gtia.color_pf1_reg & 0x0f)];
			gtia.col_pm0_mode2_clear = gtia.color_array[(gtia.color_pf2_reg) | value ];

			if ( gtia.prior_overlap )
				gtia.col_ov01 = gtia.color_array[gtia.color_pm0_reg | gtia.color_pm1_reg];
			else 
				gtia.col_ov01 = gtia.color_array[gtia.color_pm0_reg]; 
			break;

		case COLPM1:
			gtia.color_pm1_reg = value & 0xfe;
			gtia.col_pm1 = gtia.color_array[gtia.color_pm1_reg];
			gtia.col_pm1_mode2_set = gtia.color_array[(value & 0xf0) | (gtia.color_pf1_reg & 0x0f)];
			gtia.col_pm1_mode2_clear = gtia.color_array[gtia.color_pf2_reg | value ];

			if ( gtia.prior_overlap )
				gtia.col_ov01 = gtia.color_array[gtia.color_pm1_reg | gtia.color_pm0_reg];
			else 
				gtia.col_ov01 = gtia.color_array[gtia.color_pm0_reg]; 
			break;

		case COLPM2:
			gtia.color_pm2_reg = value & 0xfe;
			gtia.col_pm2 = gtia.color_array[gtia.color_pm2_reg];
			gtia.col_pm2_mode2_set = gtia.color_array[(value & 0xf0) | (gtia.color_pf1_reg & 0x0f)];
			gtia.col_pm2_mode2_clear = gtia.color_array[gtia.color_pf2_reg | value ];

			if ( gtia.prior_overlap )
				gtia.col_ov23 = gtia.color_array[gtia.color_pm2_reg | gtia.color_pm3_reg];
			else 
				gtia.col_ov23 = gtia.color_array[gtia.color_pm2_reg]; 
			break;

		case COLPM3:
			gtia.color_pm3_reg = value & 0xfe;
			gtia.col_pm3 = gtia.color_array[gtia.color_pm3_reg];
			gtia.col_pm3_mode2_set = gtia.color_array[((value) & 0xf0) | (gtia.color_pf1_reg & 0x0f)];
			gtia.col_pm3_mode2_clear = gtia.color_array[gtia.color_pf2_reg | value ];

			if ( gtia.prior_overlap )
				gtia.col_ov23 = gtia.color_array[gtia.color_pm3_reg | gtia.color_pm2_reg];
			else 
				gtia.col_ov23 = gtia.color_array[gtia.color_pm2_reg]; 
			break;

		case COLPF0:
			gtia.color_pf0_reg = value & 0xfe;
			gtia.col_pf0 = gtia.color_array[gtia.color_pf0_reg]; 
			break;

		case COLPF1:
			gtia.color_pf1_reg = value & 0xfe;
			gtia.col_pf1 = gtia.color_array[gtia.color_pf1_reg]; 
			gtia.col_pf1_mode2 = gtia.color_array[(gtia.color_pf2_reg & 0xf0) | (gtia.color_pf1_reg & 0x0f)]; 

			gtia.col_pm0_mode2_set = gtia.color_array[(gtia.color_pm0_reg & 0xf0) | (gtia.color_pf1_reg & 0x0f)];
			gtia.col_pm1_mode2_set = gtia.color_array[(gtia.color_pm1_reg & 0xf0) | (gtia.color_pf1_reg & 0x0f)];
			gtia.col_pm2_mode2_set = gtia.color_array[(gtia.color_pm2_reg & 0xf0) | (gtia.color_pf1_reg & 0x0f)];
			gtia.col_pm3_mode2_set = gtia.color_array[(gtia.color_pm3_reg & 0xf0) | (gtia.color_pf1_reg & 0x0f)];
			break;

		case COLPF2:
			gtia.color_pf2_reg = value & 0xfe;
			gtia.col_pf2 = gtia.color_array[gtia.color_pf2_reg]; 
			gtia.col_pf1_mode2 = gtia.color_array[(gtia.color_pf2_reg & 0xf0) | (gtia.color_pf1_reg & 0x0f)]; 

			gtia.col_pm0_mode2_clear = gtia.color_array[gtia.color_pf2_reg | gtia.color_pm0_reg ];
			gtia.col_pm1_mode2_clear = gtia.color_array[gtia.color_pf2_reg | gtia.color_pm1_reg ];
			gtia.col_pm2_mode2_clear = gtia.color_array[gtia.color_pf2_reg | gtia.color_pm2_reg ];
			gtia.col_pm3_mode2_clear = gtia.color_array[gtia.color_pf2_reg | gtia.color_pm3_reg ];
			break;

		case COLPF3:
			gtia.color_pf3_reg = value & 0xfe;
			gtia.col_pf3 = gtia.color_array[gtia.color_pf3_reg]; 
			
			/*
			 * In case of Player 5, set this up for PF3
			 */
			gtia.col_pf3_mode2_set = gtia.color_array[(value & 0xf0) | (gtia.color_pf1_reg & 0x0f)];
			gtia.col_pf3_mode2_clear = gtia.color_array[(value & 0xf0) | (gtia.color_pf2_reg & 0x0f)];
			break;

		case COLBK:
			gtia.color_bk_reg = value & 0xfe;
			gtia.col_bk = gtia.color_array[gtia.color_bk_reg]; 

			/*
			 * Colors for gtia modes one and three
			 * Mode 1
			 */
			for ( i = 0; i < 16; ++i )
				gtia.gtia_mode_col[0][i] = gtia.color_array[gtia.color_bk_reg | i];

			/*
			 * Mode 3
			 */
			for ( i = 0; i < 16; ++i )
				gtia.gtia_mode_col[1][i] = gtia.color_array[gtia.color_bk_reg | (i<<4)];

			break;

		/*
		 * Priority register - the biggie!
		 */
		case PRIOR:
			
			gtia.prior_reg = value;
			
			/*
			 * Fifth player
			 */
			if ( value & 0x10 ) {
				gtia.missle0_mask = PF3_MASK;
				gtia.missle1_mask = PF3_MASK;
				gtia.missle2_mask = PF3_MASK;
				gtia.missle3_mask = PF3_MASK;
			}
			else {
				gtia.missle0_mask = PM0_MASK;
				gtia.missle1_mask = PM1_MASK;
				gtia.missle2_mask = PM2_MASK;
				gtia.missle3_mask = PM3_MASK;
			}

			/*
			 * Overlap color
			 */
			if ( (gtia.prior_overlap = (value & 0x20)) ) {
				gtia.col_ov23 = gtia.color_pm3_reg | gtia.color_pm2_reg;
				gtia.col_ov01 = gtia.color_pm1_reg | gtia.color_pm0_reg;
			}
			else {
				gtia.col_ov23 = gtia.color_pm2_reg; 
				gtia.col_ov01 = gtia.color_pm0_reg; 
			}

			/*
			 * GTIA mode
			 */
			gtia.gtia_mode = (value & 0xc0) >> 6;
			gtia_set_mode_2_draw_ptr ( );

			gtia.col_bk = gtia.color_bk_reg; 

			/*
			 * Figure out PRIOR mask
			 */
			gtia.prior_mask = (value & 0xf) << PRIOR_SHIFT;
			temp_value = antic.dl_current_byte & 0xf;
			if ( (temp_value == 0x2 || temp_value == 0x3 || temp_value == 0xf) && (gtia.gtia_mode == 0) )
				gtia.prior_mask |= MODE2_MASK;

			break;

		case HITCLR:
			/*
			 * Initialize collision arrays
			 */
			memset ( gtia.collision_p0pf, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_p1pf, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_p2pf, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_p3pf, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_p0pl, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_p1pl, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_p2pl, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_p3pl, 0, sizeof(BYTE)*4 );

			memset ( gtia.collision_m0pf, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_m1pf, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_m2pf, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_m3pf, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_m0pl, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_m1pl, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_m2pl, 0, sizeof(BYTE)*4 );
			memset ( gtia.collision_m3pl, 0, sizeof(BYTE)*4 );
			break;

		/*
		 * FIXME: Not going to act on internal speaker
		 */
		case CONSOL:
			gtia.console_reg = value;
			gtia.console_mask = (~value) & 0xf;
			gtia.internal_speaker = value & 0x08;

			if ( console_get_machine_type() == MACHINE_TYPE_5200 ) {
				gtia.keypad = value & 0x01;
				gtia.pot_enable = value & 0x04;
			}

			break;
	}

	return;

} /* end gtia_write_register */

void gtia_write_register_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	gtia_write_register ( addr, value );
}

/******************************************************************************
**  Function   :  gtia_botton_button_down  
**                            
**  Objective  :  This function acts like the bottom button was pressed
**
**  Parameters :  player - which contoller event happened on
**                                                
**  return     :  1 if an interupt occured
**      
******************************************************************************/
int gtia_bottom_button_down ( int player, int temp, int temp1 ) {

	gtia.trig[player] = 0;
	gtia.trig_held_down[player] = 1;

    return 0;

} /* end gtia_bottom_button_down */

/******************************************************************************
**  Function   :  gtia_bottom_button_up  
**                            
**  Objective  :  This function acts like the bottom button was released
**
**  Parameters :  player - which contoller event happened on
**                                                
**  return     :  1 if an interupt occured
**      
******************************************************************************/
int gtia_bottom_button_up ( int player, int temp, int temp1 ) {

	gtia.trig_held_down[player] = 0;
	if ( !gtia.latch_triggers ) {
		gtia.trig[player] = 1;
	}

    return 0;

} /* end gtia_bottom_button_up */

/******************************************************************************
**  Function   :  gtia_set_switch  
**                            
**  Objective  :  This function sets the switch position
**
**  Parameters :  mask - describes which switch was pressed.
**                value - 0 for down, 1 for up
**                                                
**  return     :  0
**      
******************************************************************************/
int gtia_set_switch ( int mask, int value ) {

	switch ( mask ) {
		case 0x1: gtia.start_pressed = value; break;
		case 0x2: gtia.select_pressed = value; break;
		case 0x4: gtia.option_pressed = value; break;
	}

    return 0;

} /* end gtia_set_switch */

/******************************************************************************
**  Function   :  gtia_write_console
**                            
**  Objective  :  This function sets up cosole variables based on value. Useful
**                for recalling save states   
**
**  Parameters :  value - value to write to consol variables
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_write_console ( BYTE value ) {

	gtia.start_pressed = value & 0x1 ? 1 : 0;
	gtia.select_pressed = value & 0x2 ? 1 : 0;
	gtia.option_pressed = value & 0x4 ? 1 : 0;
	gtia.internal_speaker = value & 0x8 ? 1 : 0;

} /* end gtia_write_console */

/******************************************************************************
**  Function   :  gtia_write_collision
**                            
**  Objective  :  This function sets up collision arrays based on value. Useful
**                for recalling save states   
**
**  Parameters :  addr - 6502 memory address of the register
**                value - value to write to collsion arrays
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_write_collision ( WORD addr, BYTE value ) {

	BYTE new_address = addr & 0x1f;

	switch ( new_address ) {

		case M0PF:
			gtia.collision_m0pf[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_m0pf[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_m0pf[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_m0pf[3] = (value & 0x8) ? 1 : 0;
			break;
		case M1PF:
			gtia.collision_m1pf[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_m1pf[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_m1pf[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_m1pf[3] = (value & 0x8) ? 1 : 0;
			break;
		case M2PF:
			gtia.collision_m2pf[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_m2pf[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_m2pf[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_m2pf[3] = (value & 0x8) ? 1 : 0;
			break;
		case M3PF:
			gtia.collision_m3pf[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_m3pf[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_m3pf[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_m3pf[3] = (value & 0x8) ? 1 : 0;
			break;
		case M0PL:
			gtia.collision_m0pl[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_m0pl[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_m0pl[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_m0pl[3] = (value & 0x8) ? 1 : 0;
			break;
		case M1PL:
			gtia.collision_m1pl[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_m1pl[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_m1pl[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_m1pl[3] = (value & 0x8) ? 1 : 0;
			break;
		case M2PL:
			gtia.collision_m2pl[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_m2pl[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_m2pl[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_m2pl[3] = (value & 0x8) ? 1 : 0;
			break;
		case M3PL:
			gtia.collision_m3pl[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_m3pl[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_m3pl[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_m3pl[3] = (value & 0x8) ? 1 : 0;
			break;
		case P0PF:
			gtia.collision_p0pf[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_p0pf[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_p0pf[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_p0pf[3] = (value & 0x8) ? 1 : 0;
			break;
		case P1PF:
			gtia.collision_p1pf[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_p1pf[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_p1pf[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_p1pf[3] = (value & 0x8) ? 1 : 0;
			break;
		case P2PF:
			gtia.collision_p2pf[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_p2pf[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_p2pf[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_p2pf[3] = (value & 0x8) ? 1 : 0;
			break;
		case P3PF:
			gtia.collision_p3pf[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_p3pf[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_p3pf[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_p3pf[3] = (value & 0x8) ? 1 : 0;
			break;
		case P0PL:
			gtia.collision_p0pl[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_p0pl[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_p0pl[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_p0pl[3] = (value & 0x8) ? 1 : 0;
			break;
		case P1PL:
			gtia.collision_p1pl[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_p1pl[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_p1pl[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_p1pl[3] = (value & 0x8) ? 1 : 0;
			break;
		case P2PL:
			gtia.collision_p2pl[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_p2pl[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_p2pl[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_p2pl[3] = (value & 0x8) ? 1 : 0;
			break;
		case P3PL:
			gtia.collision_p3pl[0] = (value & 0x1) ? 1 : 0;
			gtia.collision_p3pl[1] = (value & 0x2) ? 1 : 0;
			gtia.collision_p3pl[2] = (value & 0x4) ? 1 : 0;
			gtia.collision_p3pl[3] = (value & 0x8) ? 1 : 0;
			break;
	}

} /* end gtia_write_collision */

/******************************************************************************
**  Function   :  gtia_init_scanline
**                            
**  Objective  :  This initializes the scanline for use            
**
**  Parameters :  addr - 6502 memory address of the register
**                value - value to write to register
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_init_scanline ( void ) {

	BYTE line;
	UINT8 *start;
	int i;
	unsigned int  temp_pf_x_byte = 0;

	/*
	 * Re-initialize Background in case we are
	 *   in GTIA modes 1 and 3
	 */
	gtia.col_bk = gtia.color_array[gtia.color_bk_reg]; 
	gtia.col_pf0 = gtia.color_array[gtia.color_pf0_reg]; 

	/*
	 * Playfield scanline init
	 */
	gtia.d_surface.current = start = gtia.d_surface.start + 
	                            ((antic.dl_scanline-8) * gtia.d_surface.width *
	                            (gtia.d_surface.line_only ? 0 : 1));
	for ( i = 0; i < 64; ++i ) {
		GTIA_PUT_PIXEL(gtia.col_bk)
	}	
	
	gtia.pf_x = *gtia.pf_x_start_ptr;
	gtia.pf_x_byte = (gtia.pf_x / BITS_TO_DRAW) + 1;

	/*
	 * Transfer player-missle data to priority array
	 */
	line = antic.dl_scanline & 0x1;
	gtia.transfer_p0_ptr[line] ();
	gtia.transfer_p1_ptr[line] ();
	gtia.transfer_p2_ptr[line] ();
	gtia.transfer_p3_ptr[line] ();
	gtia.transfer_m0_ptr[line] ();
	gtia.transfer_m1_ptr[line] ();
	gtia.transfer_m2_ptr[line] ();
	gtia.transfer_m3_ptr[line] ();

	/*
	 * Draw player-missle data outside of playfield
	 */
	temp_pf_x_byte = gtia.pf_x_byte;
	gtia.pf_x = 64 + (gtia.pf_x % 32);
	gtia.pf_x_byte = (gtia.pf_x / BITS_TO_DRAW) + 1;
	gtia.d_surface.current = start + ((gtia.pf_x - 64) * gtia.d_surface.bytes_per_pixel);

	while ( gtia.pf_x_byte < temp_pf_x_byte ) {
		gtia_set_byte_blank ();
	}

	return;

} /* end gtia_init_scanline */

/******************************************************************************
**  Function   :  gtia_end_scanline
**                            
**  Objective  :  This ends the scanline and writes background to rest of line
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_end_scanline ( void ) {

	UINT8 *end_ptr;

	/*
	 * Re-initialize Background in case we are
	 *   in GTIA modes 1 and 3
	 */
	gtia.col_bk = gtia.color_array[gtia.color_bk_reg]; 

	/*
	 * Find out end of line
	 */
	end_ptr = gtia.d_surface.start + ((antic.dl_scanline-8) * gtia.d_surface.width * 
	            (gtia.d_surface.line_only ? 0 : 1));
	end_ptr += (416 - BITS_TO_DRAW) * gtia.d_surface.bytes_per_pixel;

	/*
	 * Write blanks for as long as we can
	 */
	while ( gtia.d_surface.current <= end_ptr ) {
		gtia_set_byte_blank ();
	}

	return;

} /* end gtia_end_scanline */

/******************************************************************************
**  Function   :  gtia_set_blank_line
**                            
**  Objective  :  This writes a blank scanline to the screen       
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_set_blank_line (void) {

	int x;

	gtia.d_surface.current = gtia.d_surface.start + ((antic.dl_scanline-8) * gtia.d_surface.width * 
	                            (gtia.d_surface.line_only ? 0 : 1));

	switch ( gtia.d_surface.bytes_per_pixel ) {
		case 1:
			for ( x = 0; x < 416; ++x ) {
				GTIA_PUT_PIXEL_8(gtia.col_bk)
			}
			break;
		case 2:
			for ( x = 0; x < 416; ++x ) {
				GTIA_PUT_PIXEL_16(gtia.col_bk)
			}
			break;
		case 4:
			for ( x = 0; x < 416; ++x ) {
				GTIA_PUT_PIXEL_32(gtia.col_bk)
			}
			break;
	}

} /* end gtia_set_blank_line */

/******************************************************************************
**  Function   :  gtia_transfer_pxmx 
**                            
**  Objective  :  These functions transfer a byte from grafx registers to 
**                the drawing queue
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_transfer_nothing (void) {

} /* end gtia_transfer_nothing */

void gtia_transfer_p0 (void) {

	int k,n,shift;
	BYTE temp_value,j;
	int pixel_offset;

	memset ( gtia.graph_p0_line, 0, sizeof(BYTE)*65 );

	if ( !gtia.graph_p0_reg )
		return;

	/*
	 * Take graph reg from hscoll reg to find where in byte we start 
	 */
	pixel_offset = *gtia.pf_x_start_ptr - gtia.hpos_p0_reg;
	shift = (pixel_offset % BITS_TO_DRAW) - 1;
	if ( shift < 0 )
		shift += BITS_TO_DRAW;

	/*
	 * Setup 9 (8 + 1 extra for shifts) bytes and spread for size
	 */
	n = (*gtia.pf_x_start_ptr/BITS_TO_DRAW) + 1 + ((0-pixel_offset) >> BIT_DIVIDE);
	for ( k = 0x80; k > 0; k >>= 1 ) {
		temp_value = (gtia.graph_p0_reg & k ? 0x1 : 0x0);
		for ( j = 0; j < gtia.size_p0_reg; ++j ) {
			gtia.graph_p0_line[n] |= temp_value << shift;
			if ( --shift < 0 ) {
				n++;
				shift = BITS_TO_DRAW-1;
			}
		}
	}

} /* end gtia_transfer_p0 */

void gtia_transfer_p1 (void) {

	int k,n,shift;
	BYTE temp_value,j;
	int pixel_offset;

	memset ( gtia.graph_p1_line, 0, sizeof(BYTE)*65 );

	if ( !gtia.graph_p1_reg )
		return;

	/*
	 * Take graph reg from hscoll reg to find where in byte we start
	 */
	pixel_offset = *gtia.pf_x_start_ptr - gtia.hpos_p1_reg;
	shift = (pixel_offset % BITS_TO_DRAW) - 1;
	if ( shift < 0 )
		shift += BITS_TO_DRAW;

	/*
	 * Setup 9 (8 + 1 extra for shifts) bytes and spread for size
	 */
	n = (*gtia.pf_x_start_ptr/BITS_TO_DRAW) + 1 + ((0-pixel_offset) >> BIT_DIVIDE);
	for ( k = 0x80; k > 0; k >>= 1 ) {
		temp_value = (gtia.graph_p1_reg & k ? 0x1 : 0x0);
		for ( j = 0; j < gtia.size_p1_reg; ++j ) {
			gtia.graph_p1_line[n] |= temp_value << shift;
			if ( --shift < 0 ) {
				n++;
				shift = BITS_TO_DRAW-1;
			}
		}
	}

} /* end gtia_transfer_p1 */

void gtia_transfer_p2 (void) {

	int k,n,shift;
	BYTE temp_value,j;
	int pixel_offset;

	memset ( gtia.graph_p2_line, 0, sizeof(BYTE)*65 );

	if ( !gtia.graph_p2_reg )
		return;

	/*
	 * Take graph reg from hscoll reg to find where in byte we start
	 */
	pixel_offset = *gtia.pf_x_start_ptr - gtia.hpos_p2_reg;
	shift = (pixel_offset % BITS_TO_DRAW) - 1;
	if ( shift < 0 )
		shift += BITS_TO_DRAW;

	/*
	 * Setup 9 (8 + 1 extra for shifts) bytes and spread for size
	 */
	n = (*gtia.pf_x_start_ptr/BITS_TO_DRAW) + 1 + ((0-pixel_offset) >> BIT_DIVIDE);
	for ( k = 0x80; k > 0; k >>= 1 ) {
		temp_value = (gtia.graph_p2_reg & k ? 0x1 : 0x0);
		for ( j = 0; j < gtia.size_p2_reg; ++j ) {
			gtia.graph_p2_line[n] |= temp_value << shift;
			if ( --shift < 0 ) {
				n++;
				shift = BITS_TO_DRAW-1;
			}
		}
	}

} /* end gtia_transfer_p2 */

void gtia_transfer_p3 (void) {

	int k,n,shift;
	BYTE temp_value,j;
	int pixel_offset;

	memset ( gtia.graph_p3_line, 0, sizeof(BYTE)*65 );

	if ( !gtia.graph_p3_reg )
		return;

	/*
	 * Take graph reg from hscoll reg to find where in byte we start
	 */
	pixel_offset = *gtia.pf_x_start_ptr - gtia.hpos_p3_reg;
	shift = (pixel_offset % BITS_TO_DRAW) - 1;
	if ( shift < 0 )
		shift += BITS_TO_DRAW;

	/*
	 * Setup 9 (8 + 1 extra for shifts) bytes and spread for size
	 */
	n = (*gtia.pf_x_start_ptr/BITS_TO_DRAW) + 1 + ((0-pixel_offset) >> BIT_DIVIDE);
	for ( k = 0x80; k > 0; k >>= 1 ) {
		temp_value = (gtia.graph_p3_reg & k ? 0x1 : 0x0);
		for ( j = 0; j < gtia.size_p3_reg; ++j ) {
			gtia.graph_p3_line[n] |= temp_value << shift;
			if ( --shift < 0 ) {
				n++;
				shift = BITS_TO_DRAW-1;
			}
		}
	}

} /* end gtia_transfer_p3 */

void gtia_transfer_m0 (void) {

	int k,n,shift;
	BYTE temp_value,j;
	int pixel_offset;

	memset ( gtia.graph_m0_line, 0, sizeof(BYTE)*65 );

	if ( !gtia.graph_m_reg )
		return;

	/*
	 * Take graph reg from hscoll reg to find where in byte we start
	 */
	pixel_offset = *gtia.pf_x_start_ptr - gtia.hpos_m0_reg;
	shift = (pixel_offset % BITS_TO_DRAW) - 1;
	if ( shift < 0 )
		shift += BITS_TO_DRAW;

	/*
	 * Setup 9 (8 + 1 extra for shifts) bytes and spread for size
	 */
	n = (*gtia.pf_x_start_ptr/BITS_TO_DRAW) + 1 + ((0-pixel_offset) >> BIT_DIVIDE);
	for ( k = 0x02; k > 0x00; k >>= 1 ) {
		temp_value = (gtia.graph_m_reg & k ? 0x1 : 0x0);
		for ( j = 0; j < gtia.size_m0_reg; ++j ) {
			gtia.graph_m0_line[n] |= temp_value << shift;
			if ( --shift < 0 ) {
				n++;
				shift = BITS_TO_DRAW-1;
			}
		}
	}

} /* end gtia_transfer_m0 */

void gtia_transfer_m1 (void) {

	int k,n,shift;
	BYTE temp_value,j;
	int pixel_offset;

	memset ( gtia.graph_m1_line, 0, sizeof(BYTE)*65 );

	if ( !gtia.graph_m_reg )
		return;

	/*
	 * Take graph reg from hscoll reg to find where in byte we start
	 */
	pixel_offset = *gtia.pf_x_start_ptr - gtia.hpos_m1_reg;
	shift = (pixel_offset % BITS_TO_DRAW) - 1;
	if ( shift < 0 )
		shift += BITS_TO_DRAW;

	/*
	 * Setup 9 (8 + 1 extra for shifts) bytes and spread for size
	 */
	n = (*gtia.pf_x_start_ptr/BITS_TO_DRAW) + 1 + ((0-pixel_offset) >> BIT_DIVIDE);
	for ( k = 0x08; k > 0x02; k >>= 1 ) {
		temp_value = (gtia.graph_m_reg & k ? 0x1 : 0x0);
		for ( j = 0; j < gtia.size_m1_reg; ++j ) {
			gtia.graph_m1_line[n] |= temp_value << shift;
			if ( --shift < 0 ) {
				n++;
				shift = BITS_TO_DRAW-1;
			}
		}
	}

} /* end gtia_transfer_m1 */

void gtia_transfer_m2 (void) {

	int k,n,shift;
	BYTE temp_value,j;
	int pixel_offset;

	memset ( gtia.graph_m2_line, 0, sizeof(BYTE)*65 );

	if ( !gtia.graph_m_reg )
		return;

	/*
	 * Take graph reg from hscoll reg to find where in byte we start
	 */
	pixel_offset = *gtia.pf_x_start_ptr - gtia.hpos_m2_reg;
	shift = (pixel_offset % BITS_TO_DRAW) - 1;
	if ( shift < 0 )
		shift += BITS_TO_DRAW;

	/*
	 * Setup 9 (8 + 1 extra for shifts) bytes and spread for size
	 */
	n = (*gtia.pf_x_start_ptr/BITS_TO_DRAW) + 1 + ((0-pixel_offset) >> BIT_DIVIDE);
	for ( k = 0x20; k > 0x08; k >>= 1 ) {
		temp_value = (gtia.graph_m_reg & k ? 0x1 : 0x0);
		for ( j = 0; j < gtia.size_m2_reg; ++j ) {
			gtia.graph_m2_line[n] |= temp_value << shift;
			if ( --shift < 0 ) {
				n++;
				shift = BITS_TO_DRAW-1;
			}
		}
	}

} /* end gtia_transfer_m2 */

void gtia_transfer_m3 (void) {

	int k,n,shift;
	BYTE temp_value,j;
	int pixel_offset;

	memset ( gtia.graph_m3_line, 0, sizeof(BYTE)*65 );

	if ( !gtia.graph_m_reg )
		return;

	/*
	 * Take graph reg from hscoll reg to find where in byte we start
	 */
	pixel_offset = *gtia.pf_x_start_ptr - gtia.hpos_m3_reg;
	shift = (pixel_offset % BITS_TO_DRAW) - 1;
	if ( shift < 0 )
		shift += BITS_TO_DRAW;

	/*
	 * Setup 9 (8 + 1 extra for shifts) bytes and spread for size
	 */
	n = (*gtia.pf_x_start_ptr/BITS_TO_DRAW) + 1 + ((0-pixel_offset) >> BIT_DIVIDE);
	for ( k = 0x80; k > 0x20; k >>= 1 ) {
		temp_value = (gtia.graph_m_reg & k ? 0x1 : 0x0);
		for ( j = 0; j < gtia.size_m3_reg; ++j ) {
			gtia.graph_m3_line[n] |= temp_value << shift;
			if ( --shift < 0 ) {
				n++;
				shift = BITS_TO_DRAW-1;
			}
		}
	}

} /* end gtia_transfer_m3 */

/******************************************************************************
**  Function   :  gtia_set_byte_blank 
**                            
**  Objective  :  This writes player missle during blank line     
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_set_byte_blank (void) {

	unsigned int i, temp_prior_lookup;
	unsigned int p0_byte,p1_byte,p2_byte,p3_byte;
	unsigned int m0_byte,m1_byte,m2_byte,m3_byte;
	int check_flag;

	/*
	 * Pre-Calculate bytes
	 */
	p0_byte = gtia.graph_p0_line[gtia.pf_x_byte];
	p1_byte = gtia.graph_p1_line[gtia.pf_x_byte];
	p2_byte = gtia.graph_p2_line[gtia.pf_x_byte];
	p3_byte = gtia.graph_p3_line[gtia.pf_x_byte];
	m0_byte = gtia.graph_m0_line[gtia.pf_x_byte];
	m1_byte = gtia.graph_m1_line[gtia.pf_x_byte];
	m2_byte = gtia.graph_m2_line[gtia.pf_x_byte];
	m3_byte = gtia.graph_m3_line[gtia.pf_x_byte];
	check_flag = 0;

	/*
	 * 4 bytes come in from antic, check collisions
	 * Player - Player
	 */
	if ( p0_byte ) {
		check_flag = 1;
		gtia.collision_p0pl[1] |= p0_byte & p1_byte;
		gtia.collision_p0pl[2] |= p0_byte & p2_byte;
		gtia.collision_p0pl[3] |= p0_byte & p3_byte;
	}
	if ( p1_byte ) {
		check_flag = 1;
		gtia.collision_p1pl[0] |= p1_byte & p0_byte;
		gtia.collision_p1pl[2] |= p1_byte & p2_byte;
		gtia.collision_p1pl[3] |= p1_byte & p3_byte;
	}
	if ( p2_byte ) {
		check_flag = 1;
		gtia.collision_p2pl[0] |= p2_byte & p0_byte;
		gtia.collision_p2pl[1] |= p2_byte & p1_byte;
		gtia.collision_p2pl[3] |= p2_byte & p3_byte;
	}
	if ( p3_byte ) {
		check_flag = 1;
		gtia.collision_p3pl[0] |= p3_byte & p0_byte;
		gtia.collision_p3pl[1] |= p3_byte & p1_byte;
		gtia.collision_p3pl[2] |= p3_byte & p2_byte;
	}

	/*
	 * Missle - Player
	 */
	if ( m0_byte ) {
		check_flag = 1;
		gtia.collision_m0pl[0] |= m0_byte & p0_byte;
		gtia.collision_m0pl[1] |= m0_byte & p1_byte;
		gtia.collision_m0pl[2] |= m0_byte & p2_byte;
		gtia.collision_m0pl[3] |= m0_byte & p3_byte;
	}
	if ( m1_byte ) {
		check_flag = 1;
		gtia.collision_m1pl[0] |= m1_byte & p0_byte;
		gtia.collision_m1pl[1] |= m1_byte & p1_byte;
		gtia.collision_m1pl[2] |= m1_byte & p2_byte;
		gtia.collision_m1pl[3] |= m1_byte & p3_byte;
	}
	if ( m2_byte ) {
		check_flag = 1;
		gtia.collision_m2pl[0] |= m2_byte & p0_byte;
		gtia.collision_m2pl[1] |= m2_byte & p1_byte;
		gtia.collision_m2pl[2] |= m2_byte & p2_byte;
		gtia.collision_m2pl[3] |= m2_byte & p3_byte;
	}
	if ( m3_byte ) {
		check_flag = 1;
		gtia.collision_m3pl[0] |= m3_byte & p0_byte;
		gtia.collision_m3pl[1] |= m3_byte & p1_byte;
		gtia.collision_m3pl[2] |= m3_byte & p2_byte;
		gtia.collision_m3pl[3] |= m3_byte & p3_byte;
	}

	if ( check_flag ) {

		/*
		 * Now set up PRIOR and get screen data for 32
		 *  Mode2(GTIA=0) PRIOR P0 P1 P2 P3 PF0 PF1 PF2 PF3 BK
		 *     1            4    1  1  1  1  1   1   1   1   1
		 */
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 ) {
			temp_prior_lookup = gtia.prior_mask | BK_MASK;
	
			temp_prior_lookup |= (p0_byte & i) ? PM0_MASK : 0x0;
			temp_prior_lookup |= (p1_byte & i) ? PM1_MASK : 0x0;
			temp_prior_lookup |= (p2_byte & i) ? PM2_MASK : 0x0;
			temp_prior_lookup |= (p3_byte & i) ? PM3_MASK : 0x0;
	
			temp_prior_lookup |= (m0_byte & i) ?  gtia.missle0_mask : 0x0;
			temp_prior_lookup |= (m1_byte & i) ?  gtia.missle1_mask : 0x0;
			temp_prior_lookup |= (m2_byte & i) ?  gtia.missle2_mask : 0x0;
			temp_prior_lookup |= (m3_byte & i) ?  gtia.missle3_mask : 0x0;
	
			GTIA_PUT_PIXEL(*gtia.prior_ptr_array[temp_prior_lookup])
		}
	}
	else {
		switch ( gtia.d_surface.bytes_per_pixel ) {
			case 1:
				for ( i = 0; i < BITS_TO_DRAW; ++i ) {
					GTIA_PUT_PIXEL_8(gtia.col_bk)
				}
				break;
			case 2:
				for ( i = 0; i < BITS_TO_DRAW; ++i ) {
					GTIA_PUT_PIXEL_16(gtia.col_bk)
				}
				break;
			case 4:
				for ( i = 0; i < BITS_TO_DRAW; ++i ) {
					GTIA_PUT_PIXEL_32(gtia.col_bk)
				}
				break;
		}
	}

	++gtia.pf_x_byte;

}/* gtia_set_byte_blank */

/******************************************************************************
**  Function   :  gtia_set_byte_mode_2 
**                            
**  Objective  :  This takes a byte from antic and writes to frame
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_set_byte_mode_2 (void) {

	unsigned int i, temp_prior_lookup;
	unsigned int p0_byte,p1_byte,p2_byte,p3_byte;
	unsigned int m0_byte,m1_byte,m2_byte,m3_byte;

	/*
	 * Pre-Calculate bytes
	 */
	p0_byte = gtia.graph_p0_line[gtia.pf_x_byte];
	p1_byte = gtia.graph_p1_line[gtia.pf_x_byte];
	p2_byte = gtia.graph_p2_line[gtia.pf_x_byte];
	p3_byte = gtia.graph_p3_line[gtia.pf_x_byte];
	m0_byte = gtia.graph_m0_line[gtia.pf_x_byte];
	m1_byte = gtia.graph_m1_line[gtia.pf_x_byte];
	m2_byte = gtia.graph_m2_line[gtia.pf_x_byte];
	m3_byte = gtia.graph_m3_line[gtia.pf_x_byte];

	/*
	 * 4 bytes come in from antic, check collisions
	 * Player - Player
	 */
	if ( p0_byte ) {
		gtia.collision_p0pl[1] |= p0_byte & p1_byte;
		gtia.collision_p0pl[2] |= p0_byte & p2_byte;
		gtia.collision_p0pl[3] |= p0_byte & p3_byte;
	}
	if ( p1_byte ) {
		gtia.collision_p1pl[0] |= p1_byte & p0_byte;
		gtia.collision_p1pl[2] |= p1_byte & p2_byte;
		gtia.collision_p1pl[3] |= p1_byte & p3_byte;
	}
	if ( p2_byte ) {
		gtia.collision_p2pl[0] |= p2_byte & p0_byte;
		gtia.collision_p2pl[1] |= p2_byte & p1_byte;
		gtia.collision_p2pl[3] |= p2_byte & p3_byte;
	}
	if ( p3_byte ) {
		gtia.collision_p3pl[0] |= p3_byte & p0_byte;
		gtia.collision_p3pl[1] |= p3_byte & p1_byte;
		gtia.collision_p3pl[2] |= p3_byte & p2_byte;
	}

	/*
	 * Missle - Player
	 */
	if ( m0_byte ) {
		gtia.collision_m0pl[0] |= m0_byte & p0_byte;
		gtia.collision_m0pl[1] |= m0_byte & p1_byte;
		gtia.collision_m0pl[2] |= m0_byte & p2_byte;
		gtia.collision_m0pl[3] |= m0_byte & p3_byte;
	}
	if ( m1_byte ) {
		gtia.collision_m1pl[0] |= m1_byte & p0_byte;
		gtia.collision_m1pl[1] |= m1_byte & p1_byte;
		gtia.collision_m1pl[2] |= m1_byte & p2_byte;
		gtia.collision_m1pl[3] |= m1_byte & p3_byte;
	}
	if ( m2_byte ) {
		gtia.collision_m2pl[0] |= m2_byte & p0_byte;
		gtia.collision_m2pl[1] |= m2_byte & p1_byte;
		gtia.collision_m2pl[2] |= m2_byte & p2_byte;
		gtia.collision_m2pl[3] |= m2_byte & p3_byte;
	}
	if ( m3_byte ) {
		gtia.collision_m3pl[0] |= m3_byte & p0_byte;
		gtia.collision_m3pl[1] |= m3_byte & p1_byte;
		gtia.collision_m3pl[2] |= m3_byte & p2_byte;
		gtia.collision_m3pl[3] |= m3_byte & p3_byte;
	}

	/*
	 * Player - Playfield
	 */
	gtia.collision_p0pf[2] |= p0_byte;
	gtia.collision_p1pf[2] |= p1_byte;
	gtia.collision_p2pf[2] |= p2_byte;
	gtia.collision_p3pf[2] |= p3_byte;

	/*
	 * Missle - Playfield
	 */
	gtia.collision_m0pf[2] |= m0_byte;
	gtia.collision_m1pf[2] |= m1_byte;
	gtia.collision_m2pf[2] |= m2_byte;
	gtia.collision_m3pf[2] |= m3_byte;

	/*
	 * Now set up PRIOR and get screen data for 8
     *  Mode2(GTIA=0) PRIOR P0 P1 P2 P3 PF0 PF1 PF2 PF3 BK
     *     1            4    1  1  1  1  1   1   1   1   1
	 */
	for ( i = BIT_SHIFT; i > 0x00; i >>= 1 ) {
		temp_prior_lookup = gtia.prior_mask;

		temp_prior_lookup |= (gtia.pf_byte[PF2_BYTE] & i) ? PF1_MASK : PF2_MASK;

		temp_prior_lookup |= (p0_byte & i) ? PM0_MASK : 0x0;
		temp_prior_lookup |= (p1_byte & i) ? PM1_MASK : 0x0;
		temp_prior_lookup |= (p2_byte & i) ? PM2_MASK : 0x0;
		temp_prior_lookup |= (p3_byte & i) ? PM3_MASK : 0x0;

		temp_prior_lookup |= (m0_byte & i) ?  gtia.missle0_mask : 0x0;
		temp_prior_lookup |= (m1_byte & i) ?  gtia.missle1_mask : 0x0;
		temp_prior_lookup |= (m2_byte & i) ?  gtia.missle2_mask : 0x0;
		temp_prior_lookup |= (m3_byte & i) ?  gtia.missle3_mask : 0x0;

		GTIA_PUT_PIXEL(*gtia.prior_ptr_array[temp_prior_lookup])
	}

	++gtia.pf_x_byte;

}/* gtia_set_byte_mode_2 */

/******************************************************************************
**  Function   :  gtia_set_byte_mode_2_g1g3 
**                            
**  Objective  :  This takes a byte from antic and writes to frame in GTIA 1,3
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_set_byte_mode_2_g1g3 (void) {

	unsigned int i, j, temp_prior_lookup;
	unsigned int temp;
	int  k;
	unsigned int pf_byte;
	unsigned int p0_byte,p1_byte,p2_byte,p3_byte;
	unsigned int m0_byte,m1_byte,m2_byte,m3_byte;

	/*
	 * Pre-Calculate bytes
	 */
	pf_byte = gtia.pf_byte[PF2_BYTE];
	p0_byte = gtia.graph_p0_line[gtia.pf_x_byte];
	p1_byte = gtia.graph_p1_line[gtia.pf_x_byte];
	p2_byte = gtia.graph_p2_line[gtia.pf_x_byte];
	p3_byte = gtia.graph_p3_line[gtia.pf_x_byte];
	m0_byte = gtia.graph_m0_line[gtia.pf_x_byte];
	m1_byte = gtia.graph_m1_line[gtia.pf_x_byte];
	m2_byte = gtia.graph_m2_line[gtia.pf_x_byte];
	m3_byte = gtia.graph_m3_line[gtia.pf_x_byte];

	/*
	 * A byte comes in from antic, check collisions
	 * Player - Player
	 */
	if ( p0_byte ) {
		gtia.collision_p0pl[1] |= p0_byte & p1_byte;
		gtia.collision_p0pl[2] |= p0_byte & p2_byte;
		gtia.collision_p0pl[3] |= p0_byte & p3_byte;
	}
	if ( p1_byte ) {
		gtia.collision_p1pl[0] |= p1_byte & p0_byte;
		gtia.collision_p1pl[2] |= p1_byte & p2_byte;
		gtia.collision_p1pl[3] |= p1_byte & p3_byte;
	}
	if ( p2_byte ) {
		gtia.collision_p2pl[0] |= p2_byte & p0_byte;
		gtia.collision_p2pl[1] |= p2_byte & p1_byte;
		gtia.collision_p2pl[3] |= p2_byte & p3_byte;
	}
	if ( p3_byte ) {
		gtia.collision_p3pl[0] |= p3_byte & p0_byte;
		gtia.collision_p3pl[1] |= p3_byte & p1_byte;
		gtia.collision_p3pl[2] |= p3_byte & p2_byte;
	}

	/*
	 * Missle - Player 
	 */
	if ( m0_byte ) {
		gtia.collision_m0pl[0] |= m0_byte & p0_byte;
		gtia.collision_m0pl[1] |= m0_byte & p1_byte;
		gtia.collision_m0pl[2] |= m0_byte & p2_byte;
		gtia.collision_m0pl[3] |= m0_byte & p3_byte;
	}
	if ( m1_byte ) {
		gtia.collision_m1pl[0] |= m1_byte & p0_byte;
		gtia.collision_m1pl[1] |= m1_byte & p1_byte;
		gtia.collision_m1pl[2] |= m1_byte & p2_byte;
		gtia.collision_m1pl[3] |= m1_byte & p3_byte;
	}
	if ( m2_byte ) {
		gtia.collision_m2pl[0] |= m2_byte & p0_byte;
		gtia.collision_m2pl[1] |= m2_byte & p1_byte;
		gtia.collision_m2pl[2] |= m2_byte & p2_byte;
		gtia.collision_m2pl[3] |= m2_byte & p3_byte;
	}
	if ( m3_byte ) {
		gtia.collision_m3pl[0] |= m3_byte & p0_byte;
		gtia.collision_m3pl[1] |= m3_byte & p1_byte;
		gtia.collision_m3pl[2] |= m3_byte & p2_byte;
		gtia.collision_m3pl[3] |= m3_byte & p3_byte;
	}

	/*
	 * Player - Playfield -ALL Playfield is BKGND
	 */
	gtia.collision_p0pf[0] |= p0_byte;
	gtia.collision_p1pf[0] |= p1_byte;
	gtia.collision_p2pf[0] |= p2_byte;
	gtia.collision_p3pf[0] |= p3_byte;

	/*
	 * Missle - Playfield
	 */
	gtia.collision_m0pf[0] |= m0_byte;
	gtia.collision_m1pf[0] |= m1_byte;
	gtia.collision_m2pf[0] |= m2_byte;
	gtia.collision_m3pf[0] |= m3_byte;

	/*
	 * Now set up PRIOR and get screen data for 32
     *  Mode2(GTIA=0) PRIOR P0 P1 P2 P3 PF0 PF1 PF2 PF3 BK
     *     1            4    1  1  1  1  1   1   1   1   1
	 */
	i = BIT_SHIFT;
	for ( k = BITS_TO_DRAW-4; k >= 0; k -= 4 ) {
		temp = (pf_byte & (0xf << k)) >> k;
		gtia.col_bk = gtia.gtia_mode_col_ptr[temp];
		for ( j = 0; j < 4; ++j ) {
			
			temp_prior_lookup = gtia.prior_mask | BK_MASK;

			temp_prior_lookup |= (p0_byte & i) ? PM0_MASK : 0x0;
			temp_prior_lookup |= (p1_byte & i) ? PM1_MASK : 0x0;
			temp_prior_lookup |= (p2_byte & i) ? PM2_MASK : 0x0;
			temp_prior_lookup |= (p3_byte & i) ? PM3_MASK : 0x0;

			temp_prior_lookup |= (m0_byte & i) ?  gtia.missle0_mask : 0x0;
			temp_prior_lookup |= (m1_byte & i) ?  gtia.missle1_mask : 0x0;
			temp_prior_lookup |= (m2_byte & i) ?  gtia.missle2_mask : 0x0;
			temp_prior_lookup |= (m3_byte & i) ?  gtia.missle3_mask : 0x0;

			GTIA_PUT_PIXEL(*gtia.prior_ptr_array[temp_prior_lookup])

			i >>= 1;

		} /* end set these 4 bits */

	} /* end for every 4 bits reset background color */

	++gtia.pf_x_byte;

}/* gtia_set_byte_mode_2_g1g3 */

/******************************************************************************
**  Function   :  gtia_set_byte_mode_2_g2 
**                            
**  Objective  :  This takes a byte from antic and writes to frame in GTIA mode 2
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_set_byte_mode_2_g2 (void) {

	unsigned int i, j, shift, temp, temp_prior_lookup[BITS_TO_DRAW];
	int k;
	unsigned int p0_byte,p1_byte,p2_byte,p3_byte;
	unsigned int pf0_byte=0,pf1_byte=0,pf2_byte=0,pf3_byte=0,bak_byte=0;
	unsigned int m0_byte,m1_byte,m2_byte,m3_byte;
	unsigned int pm0_byte=0,pm1_byte=0,pm2_byte=0,pm3_byte=0, temp_byte;

	temp_byte = gtia.pf_byte[PF2_BYTE];

	i = BIT_SHIFT;
	for ( k = BITS_TO_DRAW-4; k >= 0; k -= 4 ) {
		shift = 0xf << k;
		temp = (temp_byte & shift) >> k;
		switch ( temp ) {
			case 0:  pm0_byte |= shift; break;
			case 1:  pm1_byte |= shift; break;
			case 2:  pm2_byte |= shift; break;
			case 3:  pm3_byte |= shift; break;
			case 4:  pf0_byte |= shift; break;
			case 5:  pf1_byte |= shift; break;
			case 6:  pf2_byte |= shift; break;
			case 7:  pf3_byte |= shift; break;
			case 8:  bak_byte |= shift; break;
			case 9:  bak_byte |= shift; break;
			case 10:  bak_byte |= shift; break;
			case 11:  bak_byte |= shift; break;
			case 12:  pf0_byte |= shift; break;
			case 13:  pf1_byte |= shift; break;
			case 14:  pf2_byte |= shift; break;
			case 15:  pf3_byte |= shift; break;
		}
	} 

	/*
	 * Pre-Calculate bytes
	 */
	p0_byte = gtia.graph_p0_line[gtia.pf_x_byte];
	p1_byte = gtia.graph_p1_line[gtia.pf_x_byte];
	p2_byte = gtia.graph_p2_line[gtia.pf_x_byte];
	p3_byte = gtia.graph_p3_line[gtia.pf_x_byte];
	m0_byte = gtia.graph_m0_line[gtia.pf_x_byte];
	m1_byte = gtia.graph_m1_line[gtia.pf_x_byte];
	m2_byte = gtia.graph_m2_line[gtia.pf_x_byte];
	m3_byte = gtia.graph_m3_line[gtia.pf_x_byte];

	for ( j = 0; j < BITS_TO_DRAW; j++  )
		temp_prior_lookup[j] = gtia.prior_mask;

	/*
	 * A byte comes in from antic, check collisions
	 * Player - Player
	 */
	if ( p0_byte ) {
		gtia.collision_p0pl[1] |= p0_byte & p1_byte & pm1_byte;
		gtia.collision_p0pl[2] |= p0_byte & p2_byte & pm2_byte;
		gtia.collision_p0pl[3] |= p0_byte & p3_byte & pm3_byte;
		gtia.collision_p0pf[0] |= p0_byte & pf0_byte;
		gtia.collision_p0pf[1] |= p0_byte & pf1_byte;
		gtia.collision_p0pf[2] |= p0_byte & pf2_byte;
		gtia.collision_p0pf[3] |= p0_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (p0_byte & i) ? PM0_MASK : 0;
	}
	if ( p1_byte ) {
		gtia.collision_p1pl[0] |= p1_byte & p0_byte & pm0_byte;
		gtia.collision_p1pl[2] |= p1_byte & p2_byte & pm2_byte;
		gtia.collision_p1pl[3] |= p1_byte & p3_byte & pm3_byte;
		gtia.collision_p1pf[0] |= p1_byte & pf0_byte;
		gtia.collision_p1pf[1] |= p1_byte & pf1_byte;
		gtia.collision_p1pf[2] |= p1_byte & pf2_byte;
		gtia.collision_p1pf[3] |= p1_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (p1_byte & i) ? PM1_MASK : 0;
	}
	if ( p2_byte ) {
		gtia.collision_p2pl[0] |= p2_byte & p0_byte & pm0_byte;
		gtia.collision_p2pl[1] |= p2_byte & p1_byte & pm1_byte;
		gtia.collision_p2pl[3] |= p2_byte & p3_byte & pm3_byte;
		gtia.collision_p2pf[0] |= p2_byte & pf0_byte;
		gtia.collision_p2pf[1] |= p2_byte & pf1_byte;
		gtia.collision_p2pf[2] |= p2_byte & pf2_byte;
		gtia.collision_p2pf[3] |= p2_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (p2_byte & i) ? PM2_MASK : 0;
	}
	if ( p3_byte ) {
		gtia.collision_p3pl[0] |= p3_byte & p0_byte & pm0_byte;
		gtia.collision_p3pl[1] |= p3_byte & p1_byte & pm1_byte;
		gtia.collision_p3pl[2] |= p3_byte & p2_byte & pm2_byte;
		gtia.collision_p3pf[0] |= p3_byte & pf0_byte;
		gtia.collision_p3pf[1] |= p3_byte & pf1_byte;
		gtia.collision_p3pf[2] |= p3_byte & pf2_byte;
		gtia.collision_p3pf[3] |= p3_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (p3_byte & i) ? PM3_MASK : 0;
	}

	/*
	 * Missle - Player
	 */
	if ( m0_byte ) {
		gtia.collision_m0pl[0] |= m0_byte & p0_byte & pm0_byte;
		gtia.collision_m0pl[1] |= m0_byte & p1_byte & pm1_byte;
		gtia.collision_m0pl[2] |= m0_byte & p2_byte & pm2_byte;
		gtia.collision_m0pl[3] |= m0_byte & p3_byte & pm3_byte;
		gtia.collision_m0pf[0] |= m0_byte & pf0_byte;
		gtia.collision_m0pf[1] |= m0_byte & pf1_byte;
		gtia.collision_m0pf[2] |= m0_byte & pf2_byte;
		gtia.collision_m0pf[3] |= m0_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (m0_byte & i) ?  gtia.missle0_mask : 0x0;
	}
	if ( m1_byte ) {
		gtia.collision_m1pl[0] |= m1_byte & p0_byte & pm0_byte;
		gtia.collision_m1pl[1] |= m1_byte & p1_byte & pm1_byte;
		gtia.collision_m1pl[2] |= m1_byte & p2_byte & pm2_byte;
		gtia.collision_m1pl[3] |= m1_byte & p3_byte & pm3_byte;
		gtia.collision_m1pf[0] |= m1_byte & pf0_byte;
		gtia.collision_m1pf[1] |= m1_byte & pf1_byte;
		gtia.collision_m1pf[2] |= m1_byte & pf2_byte;
		gtia.collision_m1pf[3] |= m1_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (m1_byte & i) ?  gtia.missle1_mask : 0x0;
	}
	if ( m2_byte ) {
		gtia.collision_m2pl[0] |= m2_byte & p0_byte & pm0_byte;
		gtia.collision_m2pl[1] |= m2_byte & p1_byte & pm1_byte;
		gtia.collision_m2pl[2] |= m2_byte & p2_byte & pm2_byte;
		gtia.collision_m2pl[3] |= m2_byte & p3_byte & pm3_byte;
		gtia.collision_m2pf[0] |= m2_byte & pf0_byte;
		gtia.collision_m2pf[1] |= m2_byte & pf1_byte;
		gtia.collision_m2pf[2] |= m2_byte & pf2_byte;
		gtia.collision_m2pf[3] |= m2_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (m2_byte & i) ?  gtia.missle2_mask : 0x0;
	}
	if ( m3_byte ) {
		gtia.collision_m3pl[0] |= m3_byte & p0_byte & pm0_byte;
		gtia.collision_m3pl[1] |= m3_byte & p1_byte & pm1_byte;
		gtia.collision_m3pl[2] |= m3_byte & p2_byte & pm2_byte;
		gtia.collision_m3pl[3] |= m3_byte & p3_byte & pm3_byte;
		gtia.collision_m3pf[0] |= m3_byte & pf0_byte;
		gtia.collision_m3pf[1] |= m3_byte & pf1_byte;
		gtia.collision_m3pf[2] |= m3_byte & pf2_byte;
		gtia.collision_m3pf[3] |= m3_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (m3_byte & i) ?  gtia.missle3_mask : 0x0;
	}

	if ( bak_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (bak_byte & i) ?  BK_MASK : 0x0;
	}

	if ( pf0_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pf0_byte & i) ?  PF0_MASK : 0x0;
	}

	if ( pf1_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pf1_byte & i) ?  PF1_MASK : 0x0;
	}

	if ( pf2_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pf2_byte & i) ?  PF2_MASK : 0x0;
	}

	if ( pf3_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pf3_byte & i) ?  PF3_MASK : 0x0;
	}

	if ( pm0_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pm0_byte & i) ?  PM0_MASK : 0x0;
	}

	if ( pm1_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pm1_byte & i) ?  PM1_MASK : 0x0;
	}

	if ( pm2_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pm2_byte & i) ?  PM2_MASK : 0x0;
	}

	if ( pm3_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pm3_byte & i) ?  PM3_MASK : 0x0;
	}

	/*
	 * Now set up PRIOR and get screen data for 32
     *  Mode2(GTIA=0) PRIOR P0 P1 P2 P3 PF0 PF1 PF2 PF3 BK
     *     1            4    1  1  1  1  1   1   1   1   1
	 */
	switch ( gtia.d_surface.bytes_per_pixel ) {
		case 1:
			for ( i = 0; i < BITS_TO_DRAW; ++i ) {
					GTIA_PUT_PIXEL_8(*gtia.prior_ptr_array[temp_prior_lookup[i]])
			}
			break;
		case 2:
			for ( i = 0; i < BITS_TO_DRAW; ++i ) {
					GTIA_PUT_PIXEL_16(*gtia.prior_ptr_array[temp_prior_lookup[i]])
			}
			break;
		case 4:
			for ( i = 0; i < BITS_TO_DRAW; ++i ) {
					GTIA_PUT_PIXEL_32(*gtia.prior_ptr_array[temp_prior_lookup[i]])
			}
			break;
	}

	++gtia.pf_x_byte;

}/* gtia_set_byte_mode_2_g2 */

/******************************************************************************
**  Function   :  gtia_set_byte_mode_4 
**                            
**  Objective  :  This takes a byte from antic and writes to frame
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_set_byte_mode_4 (void) {

	unsigned int i, j, temp_prior_lookup[BITS_TO_DRAW];
	unsigned int p0_byte,p1_byte,p2_byte,p3_byte;
	unsigned int pf0_byte,pf1_byte,pf2_byte,pf3_byte,bak_byte;
	unsigned int m0_byte,m1_byte,m2_byte,m3_byte;

	/*
	 * Pre-Calculate bytes
	 */
	bak_byte = gtia.pf_byte[BAK_BYTE];
	pf0_byte = gtia.pf_byte[PF0_BYTE];
	pf1_byte = gtia.pf_byte[PF1_BYTE];
	pf2_byte = gtia.pf_byte[PF2_BYTE];
	pf3_byte = gtia.pf_byte[PF3_BYTE];
	p0_byte = gtia.graph_p0_line[gtia.pf_x_byte];
	p1_byte = gtia.graph_p1_line[gtia.pf_x_byte];
	p2_byte = gtia.graph_p2_line[gtia.pf_x_byte];
	p3_byte = gtia.graph_p3_line[gtia.pf_x_byte];
	m0_byte = gtia.graph_m0_line[gtia.pf_x_byte];
	m1_byte = gtia.graph_m1_line[gtia.pf_x_byte];
	m2_byte = gtia.graph_m2_line[gtia.pf_x_byte];
	m3_byte = gtia.graph_m3_line[gtia.pf_x_byte];

	for ( j = 0; j < BITS_TO_DRAW; j++  )
		temp_prior_lookup[j] = gtia.prior_mask;

	/*
	 * A byte comes in from antic, check collisions
	 * Player - Player
	 */
	if ( p0_byte ) {
		gtia.collision_p0pl[1] |= p0_byte & p1_byte;
		gtia.collision_p0pl[2] |= p0_byte & p2_byte;
		gtia.collision_p0pl[3] |= p0_byte & p3_byte;
		gtia.collision_p0pf[0] |= p0_byte & pf0_byte;
		gtia.collision_p0pf[1] |= p0_byte & pf1_byte;
		gtia.collision_p0pf[2] |= p0_byte & pf2_byte;
		gtia.collision_p0pf[3] |= p0_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (p0_byte & i) ? PM0_MASK : 0;
	}
	if ( p1_byte ) {
		gtia.collision_p1pl[0] |= p1_byte & p0_byte;
		gtia.collision_p1pl[2] |= p1_byte & p2_byte;
		gtia.collision_p1pl[3] |= p1_byte & p3_byte;
		gtia.collision_p1pf[0] |= p1_byte & pf0_byte;
		gtia.collision_p1pf[1] |= p1_byte & pf1_byte;
		gtia.collision_p1pf[2] |= p1_byte & pf2_byte;
		gtia.collision_p1pf[3] |= p1_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (p1_byte & i) ? PM1_MASK : 0;
	}
	if ( p2_byte ) {
		gtia.collision_p2pl[0] |= p2_byte & p0_byte;
		gtia.collision_p2pl[1] |= p2_byte & p1_byte;
		gtia.collision_p2pl[3] |= p2_byte & p3_byte;
		gtia.collision_p2pf[0] |= p2_byte & pf0_byte;
		gtia.collision_p2pf[1] |= p2_byte & pf1_byte;
		gtia.collision_p2pf[2] |= p2_byte & pf2_byte;
		gtia.collision_p2pf[3] |= p2_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (p2_byte & i) ? PM2_MASK : 0;
	}
	if ( p3_byte ) {
		gtia.collision_p3pl[0] |= p3_byte & p0_byte;
		gtia.collision_p3pl[1] |= p3_byte & p1_byte;
		gtia.collision_p3pl[2] |= p3_byte & p2_byte;
		gtia.collision_p3pf[0] |= p3_byte & pf0_byte;
		gtia.collision_p3pf[1] |= p3_byte & pf1_byte;
		gtia.collision_p3pf[2] |= p3_byte & pf2_byte;
		gtia.collision_p3pf[3] |= p3_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (p3_byte & i) ? PM3_MASK : 0;
	}

	/*
	 * Missle - Player
	 */
	if ( m0_byte ) {
		gtia.collision_m0pl[0] |= m0_byte & p0_byte;
		gtia.collision_m0pl[1] |= m0_byte & p1_byte;
		gtia.collision_m0pl[2] |= m0_byte & p2_byte;
		gtia.collision_m0pl[3] |= m0_byte & p3_byte;
		gtia.collision_m0pf[0] |= m0_byte & pf0_byte;
		gtia.collision_m0pf[1] |= m0_byte & pf1_byte;
		gtia.collision_m0pf[2] |= m0_byte & pf2_byte;
		gtia.collision_m0pf[3] |= m0_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (m0_byte & i) ?  gtia.missle0_mask : 0x0;
	}
	if ( m1_byte ) {
		gtia.collision_m1pl[0] |= m1_byte & p0_byte;
		gtia.collision_m1pl[1] |= m1_byte & p1_byte;
		gtia.collision_m1pl[2] |= m1_byte & p2_byte;
		gtia.collision_m1pl[3] |= m1_byte & p3_byte;
		gtia.collision_m1pf[0] |= m1_byte & pf0_byte;
		gtia.collision_m1pf[1] |= m1_byte & pf1_byte;
		gtia.collision_m1pf[2] |= m1_byte & pf2_byte;
		gtia.collision_m1pf[3] |= m1_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (m1_byte & i) ?  gtia.missle1_mask : 0x0;
	}
	if ( m2_byte ) {
		gtia.collision_m2pl[0] |= m2_byte & p0_byte;
		gtia.collision_m2pl[1] |= m2_byte & p1_byte;
		gtia.collision_m2pl[2] |= m2_byte & p2_byte;
		gtia.collision_m2pl[3] |= m2_byte & p3_byte;
		gtia.collision_m2pf[0] |= m2_byte & pf0_byte;
		gtia.collision_m2pf[1] |= m2_byte & pf1_byte;
		gtia.collision_m2pf[2] |= m2_byte & pf2_byte;
		gtia.collision_m2pf[3] |= m2_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (m2_byte & i) ?  gtia.missle2_mask : 0x0;
	}
	if ( m3_byte ) {
		gtia.collision_m3pl[0] |= m3_byte & p0_byte;
		gtia.collision_m3pl[1] |= m3_byte & p1_byte;
		gtia.collision_m3pl[2] |= m3_byte & p2_byte;
		gtia.collision_m3pl[3] |= m3_byte & p3_byte;
		gtia.collision_m3pf[0] |= m3_byte & pf0_byte;
		gtia.collision_m3pf[1] |= m3_byte & pf1_byte;
		gtia.collision_m3pf[2] |= m3_byte & pf2_byte;
		gtia.collision_m3pf[3] |= m3_byte & pf3_byte;
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (m3_byte & i) ?  gtia.missle3_mask : 0x0;
	}

	if ( bak_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (bak_byte & i) ?  BK_MASK : 0x0;
	}

	if ( pf0_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pf0_byte & i) ?  PF0_MASK : 0x0;
	}

	if ( pf1_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pf1_byte & i) ?  PF1_MASK : 0x0;
	}

	if ( pf2_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pf2_byte & i) ?  PF2_MASK : 0x0;
	}

	if ( pf3_byte ) {
		j = -1;
		for ( i = BIT_SHIFT; i > 0x00; i >>= 1 )
			temp_prior_lookup[++j] |= (pf3_byte & i) ?  PF3_MASK : 0x0;
	}

	/*
	 * Now set up PRIOR and get screen data for 32
     *  Mode2(GTIA=0) PRIOR P0 P1 P2 P3 PF0 PF1 PF2 PF3 BK
     *     1            4    1  1  1  1  1   1   1   1   1
	 */
	switch ( gtia.d_surface.bytes_per_pixel ) {
		case 1:
			for ( i = 0; i < BITS_TO_DRAW; ++i ) {
				GTIA_PUT_PIXEL_8(*gtia.prior_ptr_array[temp_prior_lookup[i]])
			}
			break;
		case 2:
			for ( i = 0; i < BITS_TO_DRAW; ++i ) {
				GTIA_PUT_PIXEL_16(*gtia.prior_ptr_array[temp_prior_lookup[i]])
			}
			break;
		case 4:
			for ( i = 0; i < BITS_TO_DRAW; ++i ) {
				GTIA_PUT_PIXEL_32(*gtia.prior_ptr_array[temp_prior_lookup[i]])
			}
			break;
	}

	++gtia.pf_x_byte;

}/* gtia_set_byte_mode_4 */

/******************************************************************************
**  Function   :  gtia_init_prior_array
**                            
**  Objective  :  This initializes an array of pointers to color register     
**                so that prior points to right color when drawn
**                First index corresponds to prior
**                The mode2 stuff depends on not being in GTIA modes
**                      Mode2(GTIA=0) PRIOR P0 P1 P2 P3 PF0 PF1 PF2 PF3 BK
**                         1            4    1  1  1  1  1   1   1   1   1
**  Parameters :  NONE
**              
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void gtia_init_prior_array ( void ) {

		int i;

		/*
		 * Overlap Colors Assigned at reg write
		 * GTIA color will have to be set up by reg 
		 */

		/*
		 * First, init all to prior 1
		 */
		for ( i = 0; i < 0x4000; ++i )
			gtia.prior_ptr_array[i]  = &gtia.col_bk;

		gtia.prior_ptr_array[0]  = &gtia.col_blk;
		gtia.prior_ptr_array[1]  = &gtia.col_bk;

		/*
		 * Prior 0b0001
		 */
		for ( i = 0x200; i < 0x400; ++i ) {
			if ( i & PM0_MASK ) {
				if ( i & PM1_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_ov01;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pm0;
			}
			else if ( i & PM1_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pm1;
			else if ( i & PM2_MASK ) {
				if ( i & PM3_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_ov23;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pm2;
			}
			else if ( i & PM3_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pm3;
			else if ( i & PF3_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf3;
			else if ( i & PF0_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf0;
			else if ( i & PF1_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf1;
			else if ( i & PF2_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf2;
			else
				gtia.prior_ptr_array[i]  = &gtia.col_bk;
		}

		/*
		 * Prior 0b0000
		 */
		for ( i = 1; i < 0x200; ++i ) {
			gtia.prior_ptr_array[i]  = gtia.prior_ptr_array[i+0x200];
		}

		/*
		 * Prior 0b1111
		 */
		for ( i = 0x1e00; i < 0x2000; ++i ) {
			gtia.prior_ptr_array[i]  = gtia.prior_ptr_array[i-0x1c00];
		}

		/*
		 * Prior 0b0010
		 */
		for ( i = 0x400; i < 0x600; ++i ) {
			if ( i & PM0_MASK ) {
				if ( i & PM1_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_ov01;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pm0;
			}
			else if ( i & PM1_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pm1;
			else if ( i & PF3_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf3;
			else if ( i & PF0_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf0;
			else if ( i & PF1_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf1;
			else if ( i & PF2_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf2;
			else if ( i & PM2_MASK ) {
				if ( i & PM3_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_ov23;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pm2;
			}
			else if ( i & PM3_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pm3;
			else
				gtia.prior_ptr_array[i]  = &gtia.col_bk;
		}

		/*
		 * Prior 0b0011
		 */
		for ( i = 0x600; i < 0x800; ++i ) {
			if ( i & PM0_MASK ) {
				if ( i & PM1_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_ov01;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pm0;
			}
			else if ( i & PM1_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pm1;
			else if ( i & PF3_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf3;
			else if ( i & PF0_MASK ) {
				if ( i & PM2_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_blk;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pf0;
			}
			else if ( i & PF1_MASK ) {
				if ( i & PM3_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_blk;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pf1;
			}
			else if ( i & PF2_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf2;
			else if ( i & PM2_MASK ) {
				if ( i & PF2_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_blk;
				else {
					if ( i & PM3_MASK )
						gtia.prior_ptr_array[i]  = &gtia.col_ov23;
					else
						gtia.prior_ptr_array[i]  = &gtia.col_pm2;
				}
			}
			else if ( i & PM3_MASK ) {
				if ( i & PF3_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_blk;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pm3;
			}
			else
				gtia.prior_ptr_array[i]  = &gtia.col_bk;
		}

		/*
		 * Prior 0b0100
		 */
		for ( i = 0x800; i < 0xa00; ++i ) {
			if ( i & PF3_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf3;
			else if ( i & PF0_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf0;
			else if ( i & PF1_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf1;
			else if ( i & PF2_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf2;
			else if ( i & PM0_MASK ) {
				if ( i & PM1_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_ov01;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pm0;
			}
			else if ( i & PM1_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pm1;
			else if ( i & PM2_MASK ) {
				if ( i & PM3_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_ov23;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pm2;
			}
			else if ( i & PM3_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pm3;
			else
				gtia.prior_ptr_array[i]  = &gtia.col_bk;
		}

		/*
		 * Prior 0b1000
		 */
		for ( i = 0x1000; i < 0x1200; ++i ) {
			if ( i & PF0_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf0;
			else if ( i & PF1_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf1;
			else if ( i & PM0_MASK ) {
				if ( i & PM1_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_ov01;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pm0;
			}
			else if ( i & PM1_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pm1;
			else if ( i & PM2_MASK ) {
				if ( i & PM3_MASK )
					gtia.prior_ptr_array[i]  = &gtia.col_ov23;
				else
					gtia.prior_ptr_array[i]  = &gtia.col_pm2;
			}
			else if ( i & PM3_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pm3;
			else if ( i & PF3_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf3;
			else if ( i & PF2_MASK )
				gtia.prior_ptr_array[i]  = &gtia.col_pf2;
			else
				gtia.prior_ptr_array[i]  = &gtia.col_bk;
		}

		/****************/
		/* Mode 2       */
		/****************/
		/*
		 * First assign the same as priority
		 */
		for ( i = 0x2000; i < 0x4000; ++i ) {
				gtia.prior_ptr_array[i]  = gtia.prior_ptr_array[i&0x1fff];
		}

		/*
		 * Then replace col reg with new mode2 ones
		 */
		for ( i = 0x2000; i < 0x4000; ++i ) {
			if ( gtia.prior_ptr_array[i] == &gtia.col_pf1 )
				gtia.prior_ptr_array[i] = &gtia.col_pf1_mode2;

			else if ( gtia.prior_ptr_array[i] == &gtia.col_pm0 ) {
				if ( i & PF1_MASK )
					gtia.prior_ptr_array[i] = &gtia.col_pm0_mode2_set;
				else
					gtia.prior_ptr_array[i] = &gtia.col_pm0_mode2_clear;
			}

			else if ( gtia.prior_ptr_array[i] == &gtia.col_pm1 ) {
				if ( i & PF1_MASK )
					gtia.prior_ptr_array[i] = &gtia.col_pm1_mode2_set;
				else
					gtia.prior_ptr_array[i] = &gtia.col_pm1_mode2_clear;
			}

			else if ( gtia.prior_ptr_array[i] == &gtia.col_pm2 ) {
				if ( i & PF1_MASK )
					gtia.prior_ptr_array[i] = &gtia.col_pm2_mode2_set;
				else
					gtia.prior_ptr_array[i] = &gtia.col_pm2_mode2_clear;
			}

			else if ( gtia.prior_ptr_array[i] == &gtia.col_pm3 ) {
				if ( i & PF1_MASK )
					gtia.prior_ptr_array[i] = &gtia.col_pm3_mode2_set;
				else
					gtia.prior_ptr_array[i] = &gtia.col_pm3_mode2_clear;
			}

			else if ( gtia.prior_ptr_array[i] == &gtia.col_pf3 ) {
				if ( i & PF1_MASK )
					gtia.prior_ptr_array[i] = &gtia.col_pf3_mode2_set;
				else
					gtia.prior_ptr_array[i] = &gtia.col_pf3_mode2_clear;
			}
			
		}

} /* end gtia_init_prior_array */
