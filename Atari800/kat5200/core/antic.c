/******************************************************************************
*
* FILENAME: antic.c
*
* DESCRIPTION:  This contains functions for emulating the ANTIC video chip
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   --------------------------------------------
* 0.1     07/18/04  bberlin      Creation
* 0.2     12/09/05  bberlin      Changed PM to write values directly to
*                                  GTIA instead of using write_register.  This
*                                  will allow the write register to do 
*                                  transfers (Kaboom fix)
* 0.2     02/22/06  bberlin      Initialize VBI Enable to 0 to prevent crashes
* 0.2.3   03/19/06  bberlin      Increase dli_count to 14 in 'antic_init' 
* 0.3.0   04/06/06  bberlin      Make dli_count 14 for Football only
* 0.4.0   04/23/06  bberlin      Take out dli_count 14 for Football.
* 0.4.0   04/23/06  bberlin      Change JVB to use blank line cycle map.
******************************************************************************/
#include <string.h>
#include "antic.h"
#include "gtia.h"
#include "memory.h"
#include "debug.h"

extern struct cpu cpu6502;
extern struct gtia gtia;
struct antic antic;

static int m_shift;

/******************************************************************************
**  Function   :  antic_init
**                            
**  Objective  :  This function initializes ANTIC registers
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void antic_init ( void ) {

	int i = 0;

	/*
	 * Initialize Registers
	 */
	antic.pixel_start[0] = 128;
	antic.pixel_start[1] = 128;
	antic.pixel_start[2] = 96;
	antic.pixel_start[3] = 64;
	antic.pixel_start[4] = 64;
	antic.hscroll_pixel_start = 96;
	antic.normal_pixel_start = 96;

	antic.dli_count = 13;

	antic.nmi_vbi_en = 0;

	antic.vscroll_taking_place = 0;
	antic.vblank_sl_time = 114;

	/*
	 * Initialize Function Pointers
	 */
	antic.run_6502_ptr = antic_run_6502;
	antic.antic_read_char_data_ptr = antic_read_char_data_mode_2;
	antic.antic_read_data_ptr = antic_read_data_mode_2;
	antic.antic_map_read_and_draw_ptr = antic_read_and_draw_mode_8;
	antic.antic_map_draw_ptr = antic_draw_mode_8;
	antic.antic_blank_ptr = antic_send_byte_mode_blank;
	antic.antic_bit1_char_ptr = antic_read_byte_mode_4_bit_1;
	antic.antic_char_pf0_ptr = antic_read_byte_mode_6_pf0;
	antic.antic_char_pf1_ptr = antic_read_byte_mode_6_pf1;
	antic.antic_char_pf2_ptr = antic_read_byte_mode_6_pf2;
	antic.antic_char_pf3_ptr = antic_read_byte_mode_6_pf3;
	antic.antic_set_dli_ptr = antic_do_nothing;
	antic.antic_read_even_missle_ptr = antic_run_6502;
	antic.antic_read_even_player_ptr = antic_run_6502;
	antic.antic_read_missle_ptr = antic.antic_read_even_missle_ptr;
	antic.antic_read_player_ptr = antic.antic_read_even_player_ptr;
	antic.antic_read_odd_missle_ptr = antic_run_6502;
	antic.antic_read_odd_player_ptr = antic_run_6502;
	antic.antic_do_dl_instr_ptr = antic_no_dma;
	antic.antic_init_scanline_ptr = gtia_init_scanline;
	antic.antic_end_scanline_ptr = gtia_end_scanline;
	antic.antic_draw_blank_line_ptr = gtia_set_blank_line;

	/*
	 * Initialize Variable Pointers
	 */
	antic.dl_mode_line_ptr = &antic.dl_mode_line;
	antic.ch_base_ptr = &antic.ch_base_40_mode; 

	/*
	 * Initialize OP Code Array
	 */

	/* 
	 * Blank Line
	 */
	for ( i = 0x00; i < 0x100; ++i ) {
		antic.do_instruct[ i & 0xf0 ] = antic_blank_line;
	}

	/* 
	 * Jump and JVB
	 */
	for ( i = 0x01; i < 0x100; ++i ) {
		if ( (i & 0x0f) == 0x1 ) {
			if ( i & 0x40 )
				antic.do_instruct[ i ] = antic_jvb;
			else
				antic.do_instruct[ i ] = antic_jump;
		}
	}

	/* 
	 * Graphics Line
	 */
	for ( i = 0x02; i < 0x100; ++i ) {

		if ( (i & 0x0f) == 0x0 )
			continue;
		if ( (i & 0x0f) == 0x1 )
			continue;

		if ( (i & 0x0f) == 0x2 )
			antic.do_instruct[ i ] = antic_graphics_mode_2;
		if ( (i & 0x0f) == 0x3 )
			antic.do_instruct[ i ] = antic_graphics_mode_3;
		if ( (i & 0x0f) == 0x4 )
			antic.do_instruct[ i ] = antic_graphics_mode_4;
		if ( (i & 0x0f) == 0x5 )
			antic.do_instruct[ i ] = antic_graphics_mode_5;
		if ( (i & 0x0f) == 0x6 )
			antic.do_instruct[ i ] = antic_graphics_mode_6;
		if ( (i & 0x0f) == 0x7 )
			antic.do_instruct[ i ] = antic_graphics_mode_7;
		if ( (i & 0x0f) == 0x8 )
			antic.do_instruct[ i ] = antic_graphics_mode_8;
		if ( (i & 0x0f) == 0x9 )
			antic.do_instruct[ i ] = antic_graphics_mode_9;
		if ( (i & 0x0f) == 0xa )
			antic.do_instruct[ i ] = antic_graphics_mode_a;
		if ( (i & 0x0f) == 0xb )
			antic.do_instruct[ i ] = antic_graphics_mode_b;
		if ( (i & 0x0f) == 0xc )
			antic.do_instruct[ i ] = antic_graphics_mode_c;
		if ( (i & 0x0f) == 0xd )
			antic.do_instruct[ i ] = antic_graphics_mode_d;
		if ( (i & 0x0f) == 0xe )
			antic.do_instruct[ i ] = antic_graphics_mode_e;
		if ( (i & 0x0f) == 0xf )
			antic.do_instruct[ i ] = antic_graphics_mode_f;
		if (i & 0x40)
			antic.do_instruct[ i ] = antic_lms;
	}

	/*
	 * Create Cycle Maps for each mode
	 */
	antic_create_cycle_maps ( );

	/*
	 * Assign Read Memory functions for DMA
	 *   These can change for bank switched RAM in the XE
	 */
	for ( i = 0; i < 0x100; ++i )
		antic.read_mem[i] = cpu6502.read_mem[i];

	return;

} /* end antic_init */

/******************************************************************************
**  Function   :  antic_read_register
**                            
**  Objective  :  This function reads from the input register
**
**  Parameters :  addr - 6502 memory address of the register
**                                                
**  return     :  value at register
**      
******************************************************************************/ 
BYTE antic_read_register ( WORD addr ) {

	switch ( addr & 0x0f ) {
		case VCOUNT: 
			return antic.vcount;
		   	break;

		case PENH: return 0x00; break;
		case PENV: return 0x00; break;
		case NMIST: return antic.nmi_status_reg; break;
	}

	return 0xff;

} /* end antic_read_register */

BYTE antic_read_register_debug ( WORD addr ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	return antic_read_register ( addr );
}

/******************************************************************************
**  Function   :  antic_write_register
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
void antic_write_register ( WORD addr, BYTE value ) {

	WORD temp_pm_base_address;
	BYTE temp;

	switch ( addr & 0x0f ) {
		case DMACTL: 

			antic.dmactl_reg = value;

			/*
			 * Check Playfield DMA
			 */
			if ( value & DL_DMA )
				antic.antic_do_dl_instr_ptr = antic_do_display_list_instruction;
			else
				antic.antic_do_dl_instr_ptr = antic_no_dma;

			/*
			 * Check PM DMA
			 */
			if ( value & P_DMA ) {
				antic.antic_read_even_missle_ptr = antic_read_missle;
				antic.antic_read_even_player_ptr = antic_read_player;
			}
			else if ( value & M_DMA ) {
				antic.antic_read_even_missle_ptr = antic_read_missle;
				antic.antic_read_even_player_ptr = antic_run_6502;
				gtia_write_register ( GRAFP0, 0 );
				gtia_write_register ( GRAFP1, 0 );
				gtia_write_register ( GRAFP2, 0 );
				gtia_write_register ( GRAFP3, 0 );
			}
			else {
				antic.antic_read_even_missle_ptr = antic_run_6502;
				antic.antic_read_even_player_ptr = antic_run_6502;
				gtia_write_register ( GRAFP0, 0 );
				gtia_write_register ( GRAFP1, 0 );
				gtia_write_register ( GRAFP2, 0 );
				gtia_write_register ( GRAFP3, 0 );
				gtia_write_register ( GRAFM, 0 );
			}

			/*
			 * Setup VBLANK time
			 * FIXME: Will be different for PAL
			 */
			if ( value & (DL_DMA | P_DMA | M_DMA) )
				antic.vblank_sl_time = 105;
			else
				antic.vblank_sl_time = 114;

			/*
			 * Go and set Player Missile resolution
			 * FIXME: Have 2 different functions for single,double line
			 */
			if ( value & PM_REZ ) {

				if ( antic.antic_read_even_missle_ptr == antic_read_missle )
					antic.antic_read_odd_missle_ptr = antic_read_missle;
				else
					antic.antic_read_odd_missle_ptr = antic_run_6502;

				if ( antic.antic_read_even_player_ptr == antic_read_player )
					antic.antic_read_odd_player_ptr = antic_read_player;
				else
					antic.antic_read_odd_player_ptr = antic_run_6502;
			}
			else {
				antic.antic_read_odd_missle_ptr = antic_run_6502;
				antic.antic_read_odd_player_ptr = antic_run_6502;
			}

			/*
			 * Update player-missle start based on size
			 */
			antic.pm_resolution = (value & PM_REZ) >> 4;

			if ( antic.pm_resolution ) {
				antic.pm_counter_ptr = &antic.dl_scanline;
				temp_pm_base_address = antic.pm_base_address & 0xf800;
				antic.m_start = temp_pm_base_address | 0x0300;
				antic.p_0_start = temp_pm_base_address | 0x0400;
				antic.p_1_start = temp_pm_base_address | 0x0500;
				antic.p_2_start = temp_pm_base_address | 0x0600;
				antic.p_3_start = temp_pm_base_address | 0x0700;
			}
			else {
				antic.pm_counter_ptr = &antic.vcount;
				temp_pm_base_address = antic.pm_base_address & 0xfc00;
				antic.m_start = temp_pm_base_address | 0x0180;
				antic.p_0_start = temp_pm_base_address | 0x0200;
				antic.p_1_start = temp_pm_base_address | 0x0280;
				antic.p_2_start = temp_pm_base_address | 0x0300;
				antic.p_3_start = temp_pm_base_address | 0x0380;
			}

			/*
			 * Use playfield width to set pointer to correct cycle array
			 * FIXME: I am assuming this won't happen in a mode line??  
			 */
			temp = antic.pl_width;
			antic.pl_width = value & PL_WD;
			antic.normal_pixel_start = antic.pixel_start[antic.pl_width];
			antic.hscroll_pixel_start = antic.pixel_start[antic.pl_width+1] + 
			                                 antic.hscroll_reg_value;

			/*
			 * Update PM function pointers
			 */
			if ( antic.dl_scanline & 0x1 ) {
				antic.antic_read_missle_ptr = antic.antic_read_odd_missle_ptr;
				antic.antic_read_player_ptr = antic.antic_read_odd_player_ptr;
			}
			else {
				antic.antic_read_missle_ptr = antic.antic_read_even_missle_ptr;
				antic.antic_read_player_ptr = antic.antic_read_even_player_ptr;
			}

			break;

		case CHACTL: 

			antic.chactl_reg = value;

			/*
			 * This affects character with bit 7 set in modes 2 and 3
			 * FIXME: I assume this won't happen in a mode line?
			 */
			if ( (value & (OPAQUE | INV_VID)) == (OPAQUE | INV_VID) )
				antic.antic_charctl_ptr = &antic.antic_inv_blank_ptr;
			else if ( value & OPAQUE )
				antic.antic_charctl_ptr = &antic.antic_blank_char_ptr;
			else if ( value & INV_VID )
				antic.antic_charctl_ptr = &antic.antic_inv_video_ptr;
			else
				antic.antic_charctl_ptr = &antic.antic_normal_char_ptr;

			/*
			 * Setup for up side down characters
			 */
			if ( value & UP_VID )
				antic.dl_mode_line_ptr = &antic.dl_mode_line_opp;
			else
				antic.dl_mode_line_ptr = &antic.dl_mode_line;

			break;
		case DLISTL: 
			antic.dlistl_reg = value;
			antic.pc = (antic.pc & 0xff00) | value;
			break;
		case DLISTH: 
			antic.dlisth_reg = value;
			antic.pc = (antic.pc & 0x00ff) | (value << 8);
			antic.pc_fixed = antic.pc & 0xfc00;
			break;
		case HSCROL: 
			/*
			 * Shift by one for pixels    
			 * Subtract from 32 for actual horizontal start 
			 * FIXME:  Test for vblank so we know to change PM transfer start 
			 */
			antic.hscroll_reg_value = (value & 0xf) << 1;
			antic.hscroll_pixel_start = antic.pixel_start[antic.pl_width+1] + 
			                                 antic.hscroll_reg_value;
			break;
		case VSCROL: 
			antic.vscroll_reg_value = value;
			break;
		case PMBASE: 
			antic.pm_base_address = value << 8;

			/*
			 * Update Player-Missle start based on size
			 */
			if ( antic.pm_resolution ) {
				temp_pm_base_address = antic.pm_base_address & 0xf800;
				antic.m_start = temp_pm_base_address | 0x0300;
				antic.p_0_start = temp_pm_base_address | 0x0400;
				antic.p_1_start = temp_pm_base_address | 0x0500;
				antic.p_2_start = temp_pm_base_address | 0x0600;
				antic.p_3_start = temp_pm_base_address | 0x0700;
			}
			else {
				temp_pm_base_address = antic.pm_base_address & 0xfc00;
				antic.m_start = temp_pm_base_address | 0x0180;
				antic.p_0_start = temp_pm_base_address | 0x0200;
				antic.p_1_start = temp_pm_base_address | 0x0280;
				antic.p_2_start = temp_pm_base_address | 0x0300;
				antic.p_3_start = temp_pm_base_address | 0x0380;
			}

			break;
		case CHBASE:
			antic.ch_base_address = value << 8;
			antic.ch_base_20_mode = antic.ch_base_address & 0xfe00;
			antic.ch_base_40_mode = antic.ch_base_address & 0xfc00;
			antic.current_ch_base = *antic.ch_base_ptr | (*antic.dl_mode_line_ptr & 0x7);
			antic.current_ch_base_16_line = *antic.ch_base_ptr | 
			                                     ((*antic.dl_mode_line_ptr>>1) & 0x7);
			break;
		case WSYNC: 

			/*
			 * Replace any 6502 function with do nothing               
			 */
			antic.run_6502_ptr = antic_do_nothing;

			/*
			 * Make sure no more ops are done  
			 */ 
			cpu6502.i_cnt = -1;

			if (antic.antic_read_missle_ptr == antic_run_6502 )
				antic.antic_read_missle_ptr = antic_do_nothing;
			if (antic.antic_read_player_ptr == antic_run_6502 )
				antic.antic_read_player_ptr = antic_do_nothing;
			break;
		case NMIEN: 

			antic.nmi_dli_en = value & 0x80;
			if ( antic.nmi_dli_en )
				antic.antic_set_dli_ptr = antic_set_dli;
			else
				antic.antic_set_dli_ptr = antic_do_nothing;

			antic.nmi_vbi_en = value & 0x40;
			antic.nmi_reset_en = value & 0x20;
			break;
		case NMIRES: 
			antic.nmi_status_reg = 0x1f;
			break;
	}

} /* end antic_write_register */

void antic_write_register_debug ( WORD addr, BYTE value ) {

	debug_search_for_breakpoint (BK_MEM, addr );

	antic_write_register ( addr, value );
}

/******************************************************************************
**  Function   :  antic_do_display_list_instruction
**                            
**  Objective  :  This function runs the next instructions in the antic 
**                display list.  It controls the cycle map used in main. 
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void antic_do_display_list_instruction ( void ) {

	int lines_for_mode;
	BYTE vscroll_mask;

	/*
	 * Read the instruction
	 */
	antic.dl_current_byte = rd_antic(antic.pc);

	/*
	 * Figure out mode lines for this mode
	 */
	switch ( antic.dl_current_byte & 0xf ) {
		case 0: 
			lines_for_mode = (antic.dl_current_byte >> 4) & 0x7;
			vscroll_mask = 0x7;
			break;
		case 1: lines_for_mode = 1; vscroll_mask = 0x7; break;
		case 2: lines_for_mode = 8; vscroll_mask = 0x7; break;
		case 3: lines_for_mode = 10; vscroll_mask = 0x7; break;
		case 4: lines_for_mode = 8; vscroll_mask = 0x7; break;
		case 5: lines_for_mode = 16; vscroll_mask = 0xf; break;
		case 6: lines_for_mode = 8; vscroll_mask = 0x7; break;
		case 7: lines_for_mode = 16; vscroll_mask = 0xf; break;
		case 8: lines_for_mode = 8; vscroll_mask = 0x7; break;
		case 9: lines_for_mode = 4; vscroll_mask = 0x7; break;
		case 0xa: lines_for_mode = 4; vscroll_mask = 0x7; break;
		case 0xb: lines_for_mode = 2; vscroll_mask = 0x7; break;
		case 0xc: lines_for_mode = 1; vscroll_mask = 0x7; break;
		case 0xd: lines_for_mode = 2; vscroll_mask = 0x7; break;
		case 0xe: lines_for_mode = 1; vscroll_mask = 0x7; break;
		case 0xf: lines_for_mode = 1; vscroll_mask = 0x7; break;
		default: lines_for_mode = 1; vscroll_mask = 0x7; break;
	}

	/*
	 * If this is a regular graphics mode
	 */
	if ( (antic.dl_current_byte & 0xf) > 1 ) {

		/*
		 * Check for HSRCOLL - GTIA will handle this
		 */
		if ( antic.dl_current_byte & 0x10 ) {
			gtia.pf_x_start_ptr = &antic.hscroll_pixel_start;
			antic.width_array = antic.pl_width + 1;
		}
		else {
			gtia.pf_x_start_ptr = &antic.normal_pixel_start;
			antic.width_array = antic.pl_width;
		}
	
		/*
		 * Check for VSCROLL and whether or not to skip mode line 
		 */
		if ( antic.dl_current_byte & 0x20 ) {
			if ( antic.vscroll_taking_place ) {
				antic.dl_mode_line = 0;
				antic.dl_mode_line_opp = lines_for_mode-1;
				antic.num_lines = lines_for_mode;
				if ( antic.num_lines <= 0 )
					antic.num_lines += lines_for_mode;
				else
					antic.num_lines = lines_for_mode;
			}
			else {
				antic.num_lines = lines_for_mode - 
				           (antic.vscroll_reg_value & vscroll_mask);
				antic.vscroll_taking_place = 1;
				antic.dl_mode_line = antic.vscroll_reg_value & vscroll_mask;
				antic.dl_mode_line_opp = lines_for_mode - (antic.vscroll_reg_value & vscroll_mask) - 1;
			}
		}

		else {
			if ( antic.vscroll_taking_place ) {
				antic.num_lines = (antic.vscroll_reg_value & vscroll_mask) + 1;
				antic.vscroll_taking_place = 0;
			}
			else {
				antic.num_lines = lines_for_mode;
			}
			antic.dl_mode_line = 0;
			antic.dl_mode_line_opp = lines_for_mode-1;
		}

	} /* end if this is a regular mode */

	else {
		if ( antic.vscroll_taking_place ) {
			antic.num_lines = (antic.vscroll_reg_value & vscroll_mask);
			antic.vscroll_taking_place = 0;
		}
		else {
			antic.num_lines = lines_for_mode;
		}
	}

	/*
	 * Used for debug information
	 */
	antic.dl_pc[antic.dl_byte_number] = antic.pc;
	antic.dl[antic.dl_byte_number++] = antic.dl_current_byte;

	/*
	 * Do the instruction
	 */
	antic.do_instruct [ antic.dl_current_byte ] ();

} /* end antic_do_display_list_instruction */

/******************************************************************************
**  Function   :  antic_do_nothing
**                            
**  Objective  :  This function is a placeholder for the cycle map
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void antic_do_nothing ( void ) {

} /* end antic_do_nothing */

/******************************************************************************
**  Function   :  antic_set_dli   
**                            
**  Objective  :  This function sets up a display list interupt   
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void antic_set_dli ( void ) {

	/*
	 * Interupt 6502 after DLI cycles
	 */
	/*antic.nmi_status_reg |= 0x80;*/
	antic.nmi_status_reg = 0x9f;
	set_int_6502 ( INT_NMI );
	int_6502 ();

	/*
	 * Make sure i count is 0 if cpu not running
	 */
	if ( antic.run_6502_ptr == antic_do_nothing )
		cpu6502.i_cnt = 0;

} /* end antic_set_dli */

/******************************************************************************
**  Function   :  antic_set_reset 
**                            
**  Objective  :  This function sets up a reset interupt   
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void antic_set_reset ( void ) {

	/*
	 * Interupt 6502 after DLI cycles
	 */
	antic.nmi_status_reg = 0x3f;
	set_int_6502 ( INT_NMI );
	int_6502 ();

	/*
	 * Make sure i count is 0 if cpu not running
	 */
	if ( antic.run_6502_ptr == antic_do_nothing )
		cpu6502.i_cnt = 0;

} /* end antic_set_reset */

/******************************************************************************
**  Function   :  antic_run_6502  
**                            
**  Objective  :  This function sets cycle count and runs 6502   
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void antic_run_6502 ( void ) {

	cpu6502.i_cnt += antic.ml_cycles[antic.mode_line][antic.scan_change];

	run_6502 ();

} /* end antic_run_6502 */

/******************************************************************************
**  Function   :  antic_set_vbi   
**                            
**  Objective  :  This function sets up a vertical blank interupt 
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void antic_set_vbi ( void ) {

	antic.nmi_status_reg = 0x5f;
	/*antic.nmi_status_reg |= 0x40;*/

	set_int_6502 ( INT_NMI );

	int_6502 ();

} /* end antic_set_vbi */

/**********************************
 * ANITC graphics mode functions  *
 **********************************/
void antic_read_player ( void ) {

	/*
	 * Read pm bytes into gtia grafp registers
	 */
	if ( gtia.player_dma ) {
		gtia.graph_p0_reg = rd_antic (antic.p_0_start | *antic.pm_counter_ptr);
		gtia.graph_p1_reg = rd_antic (antic.p_1_start | *antic.pm_counter_ptr);
		gtia.graph_p2_reg = rd_antic (antic.p_2_start | *antic.pm_counter_ptr);
		gtia.graph_p3_reg = rd_antic (antic.p_3_start | *antic.pm_counter_ptr);
	}

} /* antic_read_player */

/*
 * If missle DMA set but not player read only missle 
 * If player DMA is set both are read 
 */

void antic_read_missle ( void ) {

	/*
	 * Read pm bytes into gtia grafp registers 
	 */
	if ( gtia.missle_dma ) {
		gtia.graph_m_reg = rd_antic( antic.m_start | *antic.pm_counter_ptr);
	}

} /* antic_read_missle */

void antic_read_char_data_mode_2 ( void ) {

		int i;
		WORD address;

		/*
		 *  40 character mode is special read all on line 1
		 */
		for ( i = 0; i < (antic.ml_cycles[antic.mode_line][antic.scan_change]>>2); ++i ) {

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read number of bytes from RAM and store in cache
		 *  Also, go ahead and figure out addresses 
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			address = (antic.msc++ & 0x0fff) | antic.msc_fixed;
			antic.screen_cache[++antic.byte_count] = rd_antic ( address );

			/*
			 * Setup for bit 7 set
			 * FIXME:  Make char_ptr and address in case charctl changes 
			 */
			if ( (antic.screen_cache[antic.byte_count] & 0x80) )
				antic.antic_char_ptr[antic.byte_count] = antic.antic_charctl_ptr[0];
			else
				antic.antic_char_ptr[antic.byte_count] = antic.antic_normal_char_ptr;

			antic.screen_offset[antic.byte_count] = 
				(antic.screen_cache[antic.byte_count] & 0x7f) << 3;

			/*
			 * Read the data
			 */
			antic.antic_char_ptr[antic.byte_count] ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia.gtia_set_byte_mode_2_ptr ( );

		}

} /* antic_read_char_data_mode_2 */

void antic_read_char_data_mode_3 ( void ) {

		int i;
		WORD address;

		/*
		 *  40 character mode is special read all on line 1
		 */
		for ( i = 0; i < (antic.ml_cycles[antic.mode_line][antic.scan_change]>>2); ++i ) {

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read number of bytes from RAM and store in cache
		 *  Also, go ahead and figure out addresses 
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			address = (antic.msc++ & 0x0fff) | antic.msc_fixed;
			antic.screen_cache[++antic.byte_count] = rd_antic ( address );

			/*
			 * Setup for bit 7 set
			 * FIXME:  Make char_ptr and address in case charctl changes 
			 */
			if ( (antic.screen_cache[antic.byte_count] & 0x80) )
				antic.antic_char_ptr[antic.byte_count] = antic.antic_charctl_ptr[0];
			else
				antic.antic_char_ptr[antic.byte_count] = antic.antic_normal_char_ptr;

			antic.screen_offset[antic.byte_count] = 
				(antic.screen_cache[antic.byte_count] & 0x7f) << 3;

			/*
			 * Check for lower case on first two lines, upper on last two 
			 * Don't worry about last two lines for lower 
			 * ch_base is set by dl_mode_line & 0x7 so last 2 are first 2
			 */
			if ( (antic.screen_cache[antic.byte_count] & 0x60) == 0x60 ) {
				if ( *antic.dl_mode_line_ptr < 2 )
					antic_read_byte_blank_mode_2 ( );
				else
					antic.antic_char_ptr[antic.byte_count] ();
			}
			else {
				if ( *antic.dl_mode_line_ptr & 0x08 )
					antic_read_byte_blank_mode_2 ( );
				else
					antic.antic_char_ptr[antic.byte_count] ();
			}
		}

		/*
		 * Now write 32 bits to gtia 
		 */
		gtia.gtia_set_byte_mode_2_ptr ( );

	}

} /* antic_read_char_data_mode_3 */

void antic_read_char_data_mode_4 ( void ) {

		int i;
		WORD address;

		/*
		 *  40 character mode is special read all on line 1
		 */
		for ( i = 0; i < (antic.ml_cycles[antic.mode_line][antic.scan_change]>>2); ++i ) {

		/*
		 * Reset gtia bytes  
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read number of bytes from RAM and store in cache
		 *  Also, go ahead and figure out addresses
		 */ 
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			address = (antic.msc++ & 0x0fff) | antic.msc_fixed;
			antic.screen_cache[++antic.byte_count] = rd_antic ( address );

			/*
			 * Setup for bit 7 set
			 */
			if ( (antic.screen_cache[antic.byte_count] & 0x80) )
				antic.antic_char_ptr[antic.byte_count] = antic.antic_bit1_char_ptr;
			else
				antic.antic_char_ptr[antic.byte_count] = antic.antic_normal_char_ptr;

			antic.screen_offset[antic.byte_count] = 
				(antic.screen_cache[antic.byte_count] & 0x7f) << 3;

			/*
			 * Read the data
			 */
			antic.antic_char_ptr[antic.byte_count] ();
		}

		/*
		 * Now write 32 bits to gtia 
		 */
		gtia_set_byte_mode_4 ( );

	}

} /* antic_read_char_data_mode_4 */

void antic_read_char_data_mode_6 ( void ) {

		WORD address;

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read number of bytes from RAM and store in cache
		 *  Also, go ahead and figure out addresses 
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			address = (antic.msc++ & 0x0fff) | antic.msc_fixed;
			antic.screen_cache[++antic.byte_count] = rd_antic ( address );

			/*
			 * Setup for bits 6 and 7
			 */
			switch ( antic.screen_cache[antic.byte_count] & 0xc0 ) {
				case 0x00:
					antic.antic_char_ptr[antic.byte_count] = antic.antic_char_pf0_ptr;
					break;
				case 0x40:
					antic.antic_char_ptr[antic.byte_count] = antic.antic_char_pf1_ptr;
					break;
				case 0x80:
					antic.antic_char_ptr[antic.byte_count] = antic.antic_char_pf2_ptr;
					break;
				case 0xc0:
					antic.antic_char_ptr[antic.byte_count] = antic.antic_char_pf3_ptr;
					break;
			}

			antic.screen_offset[antic.byte_count] = 
				(antic.screen_cache[antic.byte_count] & 0x3f) << 3;

			/*
			 * Read the data 
			 */
			antic.antic_char_ptr[antic.byte_count] ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_read_char_data_mode_6 */

void antic_read_data_mode_2 ( void ) {

		/*
		 * Reset gtia bytes 
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read from cache
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {
			antic.antic_char_ptr[++antic.byte_count] ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia.gtia_set_byte_mode_2_ptr ( );

} /* antic_read_data_mode_2 */

void antic_read_data_mode_4 ( void ) {

		/*
		 * Reset gtia bytes 
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read from cache
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {
			antic.antic_char_ptr[++antic.byte_count] ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_read_data_mode_4 */

void antic_read_data_mode_3 ( void ) {

		/*
		 * Reset gtia bytes 
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read from cache
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			/*
			 * Check for lower case on first two lines, upper on last two
			 * Don't worry about last two lines for lower 
			 * ch_base is set by dl_mode_line & 0x7 so last 2 are first 2
			 */
			if ( (antic.screen_cache[++antic.byte_count] & 0x60) == 0x60 ) {
				if ( *antic.dl_mode_line_ptr < 2 )
					antic_read_byte_blank_mode_2 ( );
				else
					antic.antic_char_ptr[antic.byte_count] ();
			}
			else {
				if ( *antic.dl_mode_line_ptr & 0x08 )
					antic_read_byte_blank_mode_2 ( );
				else
					antic.antic_char_ptr[antic.byte_count] ();
			}
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia.gtia_set_byte_mode_2_ptr ( );

} /* antic_read_data_mode_3 */

void antic_read_byte_mode_2 ( void ) {

		/*
		 * Read current screen byte , 8 pixels
		 */
		gtia.pf_byte[PF2_BYTE] |= (rd_antic ( antic.current_ch_base | 
		                      antic.screen_offset[antic.byte_count] )) << m_shift;
			
} /* antic_read_byte_mode_2 */

void antic_read_byte_inv_mode_2 ( void ) {

		/*
		 * Read current screen byte , then invert
		 */
		gtia.pf_byte[PF2_BYTE] |= (~(rd_antic ( antic.current_ch_base | 
		                      antic.screen_offset[antic.byte_count] )) & 0xff) << m_shift;
			
} /* antic_read_byte_inv_mode_2 */

void antic_read_byte_blank_mode_2 ( void ) {

} /* antic_read_byte_blank_mode_2 */

void antic_read_byte_inv_blank_mode_2 ( void ) {

		/*
		 * Gtia byte = ff for inverse blank 
		 */
		gtia.pf_byte[PF2_BYTE] |= (0xff) << m_shift;
			
} /* antic_read_byte_blank_mode_2 */

void antic_read_byte_mode_4_bit_0 ( void ) {

		BYTE temp_byte;

		/*
		 * Read current screen byte , 8 pixels
		 */
		temp_byte = rd_antic ( antic.current_ch_base | 
		                      antic.screen_offset[antic.byte_count] );

		/*
		 * Assign GTIA colors 
		 */
		gtia.pf_byte[temp_byte & 0x03] |= 0x03 << m_shift;
		gtia.pf_byte[(temp_byte & 0x0c) >> 2] |= 0x0c << m_shift;
		gtia.pf_byte[(temp_byte & 0x30) >> 4] |= 0x30 << m_shift;
		gtia.pf_byte[(temp_byte & 0xc0) >> 6] |= 0xc0 << m_shift;

} /* antic_read_byte_mode_4_bit_0 */

void antic_read_byte_mode_4_bit_1 ( void ) {

		BYTE temp_byte;
		BYTE pixel_0,pixel_1,pixel_2,pixel_3;

		/*
		 * Read current screen byte , 8 pixels 
		 */
		temp_byte = rd_antic ( antic.current_ch_base | 
		                      antic.screen_offset[antic.byte_count] );

		/*
		 * Assign pixels and test for 0x3
		 */
		pixel_0 = temp_byte & 0x03;
		pixel_1 = temp_byte & 0x0c;
		pixel_2 = temp_byte & 0x30;
		pixel_3 = temp_byte & 0xc0;

		/*
		 * Assign GTIA colors
		 */
		gtia.pf_byte[pixel_0 + (pixel_0 == 0x03 ? 1:0)] |= 0x03 << m_shift;
		gtia.pf_byte[(pixel_1 >> 2) + (pixel_1 == 0x0c ? 1:0)] |= 0x0c << m_shift;
		gtia.pf_byte[(pixel_2 >> 4) + (pixel_2 == 0x30 ? 1:0)] |= 0x30 << m_shift;
		gtia.pf_byte[(pixel_3 >> 6) + (pixel_3 == 0xc0 ? 1:0)] |= 0xc0 << m_shift;

} /* antic_read_byte_mode_4_bit_1 */

void antic_read_byte_mode_5_bit_0 ( void ) {

		BYTE temp_byte;

		/*
		 * Read current screen byte , 8 pixels
		 */
		temp_byte = rd_antic ( antic.current_ch_base_16_line | 
		                      antic.screen_offset[antic.byte_count] );

		/*
		 * Assign GTIA colors 
		 */
		gtia.pf_byte[temp_byte & 0x03] |= 0x03 << m_shift;
		gtia.pf_byte[(temp_byte & 0x0c) >> 2] |= 0x0c << m_shift;
		gtia.pf_byte[(temp_byte & 0x30) >> 4] |= 0x30 << m_shift;
		gtia.pf_byte[(temp_byte & 0xc0) >> 6] |= 0xc0 << m_shift;

} /* antic_read_byte_mode_5_bit_0 */

void antic_read_byte_mode_5_bit_1 ( void ) {

		BYTE temp_byte;
		BYTE pixel_0,pixel_1,pixel_2,pixel_3;

		/*
		 * Read current screen byte , 8 pixels
		 */
		temp_byte = rd_antic ( antic.current_ch_base_16_line | 
		                      antic.screen_offset[antic.byte_count] );

		/*
		 * Assign pixels and test for 0x3
		 */
		pixel_0 = temp_byte & 0x03;
		pixel_1 = temp_byte & 0x0c;
		pixel_2 = temp_byte & 0x30;
		pixel_3 = temp_byte & 0xc0;

		/*
		 * Assign GTIA colors
		 */
		gtia.pf_byte[pixel_0 + (pixel_0 == 0x03 ? 1:0)] |= 0x03 << m_shift;
		gtia.pf_byte[(pixel_1 >> 2) + (pixel_1 == 0x0c ? 1:0)] |= 0x0c << m_shift;
		gtia.pf_byte[(pixel_2 >> 4) + (pixel_2 == 0x30 ? 1:0)] |= 0x30 << m_shift;
		gtia.pf_byte[(pixel_3 >> 6) + (pixel_3 == 0xc0 ? 1:0)] |= 0xc0 << m_shift;

} /* antic_read_byte_mode_5_bit_1 */

void antic_read_byte_mode_6_pf0 ( void ) {

		BYTE temp_byte;

		/*
		 * Read current screen byte , 8 pixels
		 */
		temp_byte = rd_antic ( antic.current_ch_base | 
		                      antic.screen_offset[antic.byte_count] );

		/*
		 * Go through first 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x80) ? PF0_BYTE : BAK_BYTE)] |= 0xc0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x40) ? PF0_BYTE : BAK_BYTE)] |= 0x30 << m_shift;
		gtia.pf_byte[((temp_byte & 0x20) ? PF0_BYTE : BAK_BYTE)] |= 0x0c << m_shift;
		gtia.pf_byte[((temp_byte & 0x10) ? PF0_BYTE : BAK_BYTE)] |= 0x03 << m_shift;

		m_shift -=8;

		/*
		 * Go through last 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x08) ? PF0_BYTE : BAK_BYTE)] |= 0xc0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x04) ? PF0_BYTE : BAK_BYTE)] |= 0x30 << m_shift;
		gtia.pf_byte[((temp_byte & 0x02) ? PF0_BYTE : BAK_BYTE)] |= 0x0c << m_shift;
		gtia.pf_byte[((temp_byte & 0x01) ? PF0_BYTE : BAK_BYTE)] |= 0x03 << m_shift;

} /* antic_read_byte_mode_6_pf0 */

void antic_read_byte_mode_6_pf1 ( void ) {

		BYTE temp_byte;

		/*
		 * Read current screen byte , 8 pixels
		 */
		temp_byte = rd_antic ( antic.current_ch_base | 
		                      antic.screen_offset[antic.byte_count] );

		/*
		 * Go through first 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x80) ? PF1_BYTE : BAK_BYTE)] |= 0xc0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x40) ? PF1_BYTE : BAK_BYTE)] |= 0x30 << m_shift;
		gtia.pf_byte[((temp_byte & 0x20) ? PF1_BYTE : BAK_BYTE)] |= 0x0c << m_shift;
		gtia.pf_byte[((temp_byte & 0x10) ? PF1_BYTE : BAK_BYTE)] |= 0x03 << m_shift;

		m_shift -= 8;

		/*
		 * Go through last 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x08) ? PF1_BYTE : BAK_BYTE)] |= 0xc0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x04) ? PF1_BYTE : BAK_BYTE)] |= 0x30 << m_shift;
		gtia.pf_byte[((temp_byte & 0x02) ? PF1_BYTE : BAK_BYTE)] |= 0x0c << m_shift;
		gtia.pf_byte[((temp_byte & 0x01) ? PF1_BYTE : BAK_BYTE)] |= 0x03 << m_shift;

} /* antic_read_byte_mode_6_pf1 */

void antic_read_byte_mode_6_pf2 ( void ) {

		BYTE temp_byte;

		/*
		 * Read current screen byte , 8 pixels
		 */
		temp_byte = rd_antic ( antic.current_ch_base | 
		                      antic.screen_offset[antic.byte_count] );

		/*
		 * Go through first 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x80) ? PF2_BYTE : BAK_BYTE)] |= 0xc0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x40) ? PF2_BYTE : BAK_BYTE)] |= 0x30 << m_shift;
		gtia.pf_byte[((temp_byte & 0x20) ? PF2_BYTE : BAK_BYTE)] |= 0x0c << m_shift;
		gtia.pf_byte[((temp_byte & 0x10) ? PF2_BYTE : BAK_BYTE)] |= 0x03 << m_shift;

		m_shift -= 8;

		/*
		 * Go through last 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x08) ? PF2_BYTE : BAK_BYTE)] |= 0xc0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x04) ? PF2_BYTE : BAK_BYTE)] |= 0x30 << m_shift;
		gtia.pf_byte[((temp_byte & 0x02) ? PF2_BYTE : BAK_BYTE)] |= 0x0c << m_shift;
		gtia.pf_byte[((temp_byte & 0x01) ? PF2_BYTE : BAK_BYTE)] |= 0x03 << m_shift;

} /* antic_read_byte_mode_6_pf2 */

void antic_read_byte_mode_6_pf3 ( void ) {

		BYTE temp_byte;

		/*
		 * Read current screen byte , 8 pixels
		 */
		temp_byte = rd_antic ( antic.current_ch_base | 
		                      antic.screen_offset[antic.byte_count] );

		/*
		 * Go through first 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x80) ? PF3_BYTE : BAK_BYTE)] |= 0xc0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x40) ? PF3_BYTE : BAK_BYTE)] |= 0x30 << m_shift;
		gtia.pf_byte[((temp_byte & 0x20) ? PF3_BYTE : BAK_BYTE)] |= 0x0c << m_shift;
		gtia.pf_byte[((temp_byte & 0x10) ? PF3_BYTE : BAK_BYTE)] |= 0x03 << m_shift;

		m_shift -=8;

		/*
		 * Go through last 4 bits 
		 */
		gtia.pf_byte[((temp_byte & 0x08) ? PF3_BYTE : BAK_BYTE)] |= 0xc0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x04) ? PF3_BYTE : BAK_BYTE)] |= 0x30 << m_shift;
		gtia.pf_byte[((temp_byte & 0x02) ? PF3_BYTE : BAK_BYTE)] |= 0x0c << m_shift;
		gtia.pf_byte[((temp_byte & 0x01) ? PF3_BYTE : BAK_BYTE)] |= 0x03 << m_shift;

} /* antic_read_byte_mode_6_pf3 */

void antic_read_byte_mode_7_pf0 ( void ) {

		BYTE temp_byte;

		/* Read current screen byte , 8 pixels               */
		temp_byte = rd_antic ( antic.current_ch_base_16_line | 
		                      antic.screen_offset[antic.byte_count] );

		/* Go through first 4 bits                           */
		gtia.pf_byte[((temp_byte & 0x80) ? PF0_BYTE : BAK_BYTE)] |= (0xc0 << m_shift);
		gtia.pf_byte[((temp_byte & 0x40) ? PF0_BYTE : BAK_BYTE)] |= (0x30 << m_shift);
		gtia.pf_byte[((temp_byte & 0x20) ? PF0_BYTE : BAK_BYTE)] |= (0x0c << m_shift);
		gtia.pf_byte[((temp_byte & 0x10) ? PF0_BYTE : BAK_BYTE)] |= (0x03 << m_shift);

		m_shift -= 8;

		/* Go through last 4 bits                            */
		gtia.pf_byte[((temp_byte & 0x08) ? PF0_BYTE : BAK_BYTE)] |= (0xc0 << m_shift);
		gtia.pf_byte[((temp_byte & 0x04) ? PF0_BYTE : BAK_BYTE)] |= (0x30 << m_shift);
		gtia.pf_byte[((temp_byte & 0x02) ? PF0_BYTE : BAK_BYTE)] |= (0x0c << m_shift);
		gtia.pf_byte[((temp_byte & 0x01) ? PF0_BYTE : BAK_BYTE)] |= (0x03 << m_shift);

} /* antic_read_byte_mode_7_pf0 */

void antic_read_byte_mode_7_pf1 ( void ) {

		BYTE temp_byte;

		/*
		 * Read current screen byte , 8 pixels
		 */
		temp_byte = rd_antic ( antic.current_ch_base_16_line | 
		                      antic.screen_offset[antic.byte_count] );

		/*
		 * Go through first 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x80) ? PF1_BYTE : BAK_BYTE)] |= (0xc0 << m_shift);
		gtia.pf_byte[((temp_byte & 0x40) ? PF1_BYTE : BAK_BYTE)] |= (0x30 << m_shift);
		gtia.pf_byte[((temp_byte & 0x20) ? PF1_BYTE : BAK_BYTE)] |= (0x0c << m_shift);
		gtia.pf_byte[((temp_byte & 0x10) ? PF1_BYTE : BAK_BYTE)] |= (0x03 << m_shift);

		m_shift -= 8;

		/*
		 * Go through last 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x08) ? PF1_BYTE : BAK_BYTE)] |= (0xc0 << m_shift);
		gtia.pf_byte[((temp_byte & 0x04) ? PF1_BYTE : BAK_BYTE)] |= (0x30 << m_shift);
		gtia.pf_byte[((temp_byte & 0x02) ? PF1_BYTE : BAK_BYTE)] |= (0x0c << m_shift);
		gtia.pf_byte[((temp_byte & 0x01) ? PF1_BYTE : BAK_BYTE)] |= (0x03 << m_shift);

} /* antic_read_byte_mode_7_pf1 */

void antic_read_byte_mode_7_pf2 ( void ) {

		BYTE temp_byte;

		/*
		 * Read current screen byte , 8 pixels 
		 */
		temp_byte = rd_antic ( antic.current_ch_base_16_line | 
		                      antic.screen_offset[antic.byte_count] );

		/*
		 * Go through first 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x80) ? PF2_BYTE : BAK_BYTE)] |= (0xc0 << m_shift);
		gtia.pf_byte[((temp_byte & 0x40) ? PF2_BYTE : BAK_BYTE)] |= (0x30 << m_shift);
		gtia.pf_byte[((temp_byte & 0x20) ? PF2_BYTE : BAK_BYTE)] |= (0x0c << m_shift);
		gtia.pf_byte[((temp_byte & 0x10) ? PF2_BYTE : BAK_BYTE)] |= (0x03 << m_shift);

		m_shift -= 8;

		/*
		 * Go through last 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x08) ? PF2_BYTE : BAK_BYTE)] |= (0xc0 << m_shift);
		gtia.pf_byte[((temp_byte & 0x04) ? PF2_BYTE : BAK_BYTE)] |= (0x30 << m_shift);
		gtia.pf_byte[((temp_byte & 0x02) ? PF2_BYTE : BAK_BYTE)] |= (0x0c << m_shift);
		gtia.pf_byte[((temp_byte & 0x01) ? PF2_BYTE : BAK_BYTE)] |= (0x03 << m_shift);

} /* antic_read_byte_mode_7_pf2 */

void antic_read_byte_mode_7_pf3 ( void ) {

		BYTE temp_byte;

		/*
		 * Read current screen byte , 8 pixels
		 */
		temp_byte = rd_antic ( antic.current_ch_base_16_line | 
		                      antic.screen_offset[antic.byte_count] );

		/*
		 * Go through first 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x80) ? PF3_BYTE : BAK_BYTE)] |= (0xc0 << m_shift);
		gtia.pf_byte[((temp_byte & 0x40) ? PF3_BYTE : BAK_BYTE)] |= (0x30 << m_shift);
		gtia.pf_byte[((temp_byte & 0x20) ? PF3_BYTE : BAK_BYTE)] |= (0x0c << m_shift);
		gtia.pf_byte[((temp_byte & 0x10) ? PF3_BYTE : BAK_BYTE)] |= (0x03 << m_shift);

		m_shift -= 8;

		/*
		 * Go through last 4 bits
		 */
		gtia.pf_byte[((temp_byte & 0x08) ? PF3_BYTE : BAK_BYTE)] |= (0xc0 << m_shift);
		gtia.pf_byte[((temp_byte & 0x04) ? PF3_BYTE : BAK_BYTE)] |= (0x30 << m_shift);
		gtia.pf_byte[((temp_byte & 0x02) ? PF3_BYTE : BAK_BYTE)] |= (0x0c << m_shift);
		gtia.pf_byte[((temp_byte & 0x01) ? PF3_BYTE : BAK_BYTE)] |= (0x03 << m_shift);

} /* antic_read_byte_mode_7_pf3 */

void antic_read_and_draw_mode_8 ( void ) {

		WORD address;

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read number of bytes from RAM and store in cache
		 *  Also, go ahead and figure out addresses
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			address = (antic.msc++ & 0x0fff) | antic.msc_fixed;
			antic.screen_cache[++antic.byte_count] = rd_antic ( address );

			/*
			 * Transfer data to gtia
			 */
			antic_send_byte_mode_8 ();
		}

		/*
		 * Now write 32 bits to gtia 
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_read_and_draw_mode_8 */

void antic_draw_mode_8 ( void ) {

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read bytes from cache
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			++antic.byte_count;

			antic_send_byte_mode_8 ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_draw_mode_8 */

void antic_read_and_draw_mode_9 ( void ) {

		WORD address;

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read number of bytes from RAM and store in cache
		 *  Also, go ahead and figure out addresses
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			address = (antic.msc++ & 0x0fff) | antic.msc_fixed;
			antic.screen_cache[++antic.byte_count] = rd_antic ( address );

			antic_send_byte_mode_9 ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_read_and_draw_mode_9 */

void antic_draw_mode_9 ( void ) {

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read bytes from cache
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			++antic.byte_count;

			antic_send_byte_mode_9 ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_draw_mode_9 */

void antic_read_and_draw_mode_a ( void ) {

		WORD address;

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read number of bytes from RAM and store in cache
		 *  Also, go ahead and figure out addresses 
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			address = (antic.msc++ & 0x0fff) | antic.msc_fixed;
			antic.screen_cache[++antic.byte_count] = rd_antic ( address );

			antic_send_byte_mode_a ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_read_and_draw_mode_a */

void antic_draw_mode_a ( void ) {

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read bytes from cache
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			++antic.byte_count;

			antic_send_byte_mode_a ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_draw_mode_a */

void antic_read_and_draw_mode_b_c ( void ) {

		WORD address;

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read number of bytes from RAM and store in cache
		 *  Also, go ahead and figure out addresses 
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			address = (antic.msc++ & 0x0fff) | antic.msc_fixed;
			antic.screen_cache[++antic.byte_count] = rd_antic ( address );

			antic_send_byte_mode_b_c ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_read_and_draw_mode_b_c */

void antic_draw_mode_b_c ( void ) {

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read bytes from cache
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			++antic.byte_count;

			antic_send_byte_mode_b_c ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_draw_mode_b_c */

void antic_read_and_draw_mode_d_e ( void ) {

		WORD address;

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read number of bytes from RAM and store in cache
		 *  Also, go ahead and figure out addresses 
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			address = (antic.msc++ & 0x0fff) | antic.msc_fixed;
			antic.screen_cache[++antic.byte_count] = rd_antic ( address );

			antic_send_byte_mode_d_e ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_read_and_draw_mode_d_e */

void antic_draw_mode_d_e ( void ) {

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read bytes from cache
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			++antic.byte_count;

			antic_send_byte_mode_d_e ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia_set_byte_mode_4 ( );

} /* antic_draw_mode_d_e */

void antic_read_and_draw_mode_f ( void ) {

		WORD address;

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read number of bytes from RAM and store in cache
		 *  Also, go ahead and figure out addresses 
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			address = (antic.msc++ & 0x0fff) | antic.msc_fixed;
			antic.screen_cache[++antic.byte_count] = rd_antic ( address );

			antic_send_byte_mode_f ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia.gtia_set_byte_mode_2_ptr ( );

} /* antic_read_and_draw_mode_f */

void antic_draw_mode_f ( void ) {

		/*
		 * Reset gtia bytes
		 */
		memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

		/*
		 * Read bytes from cache
		 */
		for ( m_shift = 24; m_shift >= 0; m_shift-=8 ) {

			++antic.byte_count;

			/* Transfer data to gtia          */
			antic_send_byte_mode_f ();
		}

		/*
		 * Now write 32 bits to gtia
		 */
		gtia.gtia_set_byte_mode_2_ptr ( );

} /* antic_draw_mode_f */

void antic_send_byte_mode_8 ( void ) {

		BYTE temp_byte = antic.screen_cache[antic.byte_count];

		/*
		 * Write 4 bytes based on each set of 2 bits 
		 */
		gtia.pf_byte[((temp_byte & 0xc0) >> 6)] |= 0xff << m_shift;
		m_shift -= 8;

		gtia.pf_byte[((temp_byte & 0x30) >> 4)] |= 0xff << m_shift;
		m_shift -= 8;

		gtia.pf_byte[((temp_byte & 0x0c) >> 2)] |= 0xff << m_shift;
		m_shift -= 8;

		gtia.pf_byte[(temp_byte & 0x03)] |= 0xff << m_shift;

} /* antic_send_byte_mode_8 */

void antic_send_byte_mode_9 ( void ) {

		BYTE temp_byte = antic.screen_cache[antic.byte_count];

		/*
		 * Every 2 bits is a byte to be sent
		 */
		gtia.pf_byte[(temp_byte & 0x80) >> 7] |= 0xf0 << m_shift;
		gtia.pf_byte[(temp_byte & 0x40) >> 6] |= 0x0f << m_shift;
		m_shift -= 8;

		gtia.pf_byte[(temp_byte & 0x20) >> 5] |= 0xf0 << m_shift;
		gtia.pf_byte[(temp_byte & 0x10) >> 4] |= 0x0f << m_shift;
		m_shift -= 8;

		gtia.pf_byte[(temp_byte & 0x08) >> 3] |= 0xf0 << m_shift;
		gtia.pf_byte[(temp_byte & 0x04) >> 2] |= 0x0f << m_shift;
		m_shift -= 8;

		gtia.pf_byte[(temp_byte & 0x02) >> 1] |= 0xf0 << m_shift;
		gtia.pf_byte[(temp_byte & 0x01) >> 0] |= 0x0f << m_shift;

} /* antic_send_byte_mode_9 */

void antic_send_byte_mode_a ( void ) {

		BYTE temp_byte = antic.screen_cache[antic.byte_count];

		/*
		 * Write 2 bytes based on each set of 2 bits
		 */
		gtia.pf_byte[((temp_byte & 0xc0) >> 6)] |= 0xf0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x30) >> 4)] |= 0x0f << m_shift;
		m_shift -= 8;

		gtia.pf_byte[((temp_byte & 0x0c) >> 2)] |= 0xf0 << m_shift;
		gtia.pf_byte[(temp_byte & 0x03)] = 0x0f << m_shift;

} /* antic_send_byte_mode_a */

void antic_send_byte_mode_b_c ( void ) {

		BYTE temp_byte = antic.screen_cache[antic.byte_count];

		/*
		 * Write 2 bytes based on each bit
		 */
		gtia.pf_byte[((temp_byte & 0x80) >> 7)] |= 0xc0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x40) >> 6)] |= 0x30 << m_shift;
		gtia.pf_byte[((temp_byte & 0x20) >> 5)] |= 0x0c << m_shift;
		gtia.pf_byte[((temp_byte & 0x10) >> 4)] |= 0x03 << m_shift;
		m_shift -= 8;

		gtia.pf_byte[((temp_byte & 0x08) >> 3)] |= 0xc0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x04) >> 2)] |= 0x30 << m_shift;
		gtia.pf_byte[((temp_byte & 0x02) >> 1)] |= 0x0c << m_shift;
		gtia.pf_byte[((temp_byte & 0x01) >> 0)] |= 0x03 << m_shift;

} /* antic_send_byte_mode_b_c */

void antic_send_byte_mode_d_e ( void ) {

		BYTE temp_byte = antic.screen_cache[antic.byte_count];

		/*
		 * Write 1 byte based on each every 2 bits
		 */
		gtia.pf_byte[((temp_byte & 0xc0) >> 6)] |= 0xc0 << m_shift;
		gtia.pf_byte[((temp_byte & 0x30) >> 4)] |= 0x30 << m_shift;
		gtia.pf_byte[((temp_byte & 0x0c) >> 2)] |= 0x0c << m_shift;
		gtia.pf_byte[((temp_byte & 0x03) >> 0)] |= 0x03 << m_shift;

} /* antic_send_byte_mode_d_e */

void antic_send_byte_mode_f ( void ) {

		gtia.pf_byte[PF2_BYTE] |= (antic.screen_cache[antic.byte_count]) << m_shift;

} /* antic_send_byte_mode_f */

void antic_send_byte_mode_blank ( void ) {

		gtia_set_byte_blank ( );

} /* antic_send_byte_mode_blank */

/******************************************************************************
**  Function   :  antic_no_dma    
**                            
**  Objective  :  This function is when antic cannot do DMA, so set up 6502 for
**                everything
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void antic_no_dma ( void ) {

	/*
	 * We are pretending no DMA is instruction 0x1
	 */
	antic.ml_map[0] = antic.dl_map[antic.pl_width][1][0];
	antic.ml_map[1] = 0;

	antic.ml_cycles[0] = antic.dl_cycles[antic.pl_width][1][0];

	/*
	 * FIXME: Tell GTIA to do background?
	 */

} /* end antic_no_dma */

/********************************
 * ANITC op code functions      *
 ********************************/
void antic_blank_line ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Tell gtia we are not mode 2 for PRIOR
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	gtia.pf_x_start_ptr = &antic.normal_pixel_start;

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.pl_width][0][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.pl_width][0][ML_FIRST];
	antic.sl_map = antic.ml_map[0];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < antic.num_lines; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.pl_width][0][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.pl_width][0][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines == 0 ) {
			antic.ml_map[0] = antic.dl_map[antic.pl_width][0][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.pl_width][0][ML_FIRST_LAST];
			antic.sl_map = antic.ml_map[0];
		}
		else {
			antic.ml_map[antic.num_lines] = antic.dl_map[antic.pl_width][0][ML_LAST];
			antic.ml_cycles[antic.num_lines] = antic.dl_cycles[antic.pl_width][0][ML_LAST];
		}
	}

	/*
	 * Last line if not DLI
	 */
	else if ( antic.num_lines != 0 ) {
		antic.ml_map[antic.num_lines] = antic.dl_map[antic.pl_width][0][ML_NORMAL];
		antic.ml_cycles[antic.num_lines] = antic.dl_cycles[antic.pl_width][0][ML_NORMAL];
	}
	antic.ml_map[antic.num_lines+1] = 0;

} /* end antic_blank_line */

void antic_jump ( void ) {

	/*
	 * Add to display list for debug
	 */
	antic.dl[antic.dl_byte_number++] = rd_antic(antic.pc+1);
	antic.dl[antic.dl_byte_number++] = rd_antic(antic.pc+2);

	antic.pc = rd_antic(antic.pc+1) | (rd_antic(antic.pc+2) << 8);
	antic.pc_fixed = antic.pc & 0xfc00;

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	/*
	 * Do a blank line
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		antic.ml_map[0] = antic.dl_map[antic.pl_width][0][ML_FIRST_LAST];
		antic.ml_cycles[0] = antic.dl_cycles[antic.pl_width][0][ML_FIRST_LAST];
	}
	else {
		antic.ml_map[0] = antic.dl_map[antic.pl_width][0][ML_FIRST];
		antic.ml_cycles[0] = antic.dl_cycles[antic.pl_width][0][ML_FIRST];
	}

	antic.ml_map[1] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_jump */

void antic_jvb ( void ) {

	/*
	 * Add to display list for debug
	 */
	antic.dl[antic.dl_byte_number++] = rd_antic(antic.pc+1);
	antic.dl[antic.dl_byte_number++] = rd_antic(antic.pc+2);

	antic.pc = rd_antic(antic.pc+1) | (rd_antic(antic.pc+2) << 8);
	antic.pc_fixed = antic.pc & 0xfc00;

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	/*
	 * Do all 6502 and let main proc find vertical blank
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		antic.ml_map[0] = antic.dl_map[0][1][ML_LAST];
		antic.ml_cycles[0] = antic.dl_cycles[0][1][ML_LAST];
	}
	else {
		antic.ml_map[0] = antic.dl_map[0][1][ML_NORMAL];
		antic.ml_cycles[0] = antic.dl_cycles[0][1][ML_NORMAL];
	}
	antic.sl_map = antic.ml_map[0];

	antic.ml_map[1] = 0;

} /* end antic_jvb */

void antic_lms ( void ) {

	/*
	 * Add to display list for debug
	 */
	antic.dl[antic.dl_byte_number++] = rd_antic(antic.pc+1);
	antic.dl[antic.dl_byte_number++] = rd_antic(antic.pc+2);

	/*
	 * This is called when LMS bit set
	 * It then calls the instruction
	 */
	antic.msc = rd_antic(antic.pc+1) | (rd_antic(antic.pc+2) << 8);
	antic.msc_fixed = antic.msc & 0xf000;

	antic.dl_current_byte = rd_antic(antic.pc);
	antic.pc = antic.pc_fixed | ((antic.pc + 2) & 0x03ff );

	antic.do_instruct [ antic.dl_current_byte & 0xbf ] ();

	/*
	 * Lets take two cycles of 6502 count since
	 * this is a 3 byte instr instead of 1
	 */
	cpu6502.i_cnt -= 2;

} /* end antic_lms */

void antic_none ( void ) {

	antic.pc++;

} /* end antic_none */

void antic_graphics_mode_2 ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * We only use one color, so just reset all here
	 */
	memset (gtia.pf_byte, 0x0, 5*sizeof(BYTE) );

	/*
	 * Tell gtia we are mode 2 for PRIOR
	 */
	if ( gtia.gtia_mode == 0 ) {
		gtia.prior_mask |= MODE2_MASK;
	}

	antic.ch_base_ptr = &antic.ch_base_40_mode;

	/*
	 * Assign read char and data function ptrs
	 */
	antic.antic_read_char_data_ptr = antic_read_char_data_mode_2;
	antic.antic_read_data_ptr = antic_read_data_mode_2;

	/*
	 * Assign char ctrl function pointers
	 */
	antic.antic_normal_char_ptr = antic_read_byte_mode_2;
	antic.antic_inv_video_ptr = antic_read_byte_inv_mode_2;
	antic.antic_blank_char_ptr = antic_read_byte_blank_mode_2;
	antic.antic_inv_blank_ptr = antic_read_byte_inv_blank_mode_2;

	/*
	 * Recalculate ch base
	 */
	antic.current_ch_base = *antic.ch_base_ptr | 
	                             (*antic.dl_mode_line_ptr & 0x7);

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 8; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_2 */

void antic_graphics_mode_3 ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Tell gtia we are mode 2 for PRIOR 
	 */
	if ( gtia.gtia_mode == 0 ) {
		gtia.prior_mask |= MODE2_MASK;
	}

	antic.ch_base_ptr = &antic.ch_base_40_mode;

	/*
	 * Assign read char and data function ptrs
	 */
	antic.antic_read_char_data_ptr = antic_read_char_data_mode_3;
	antic.antic_read_data_ptr = antic_read_data_mode_3;

	/*
	 * Assign char ctrl function pointers
	 */
	antic.antic_normal_char_ptr = antic_read_byte_mode_2;
	antic.antic_inv_video_ptr = antic_read_byte_inv_mode_2;
	antic.antic_blank_char_ptr = antic_read_byte_blank_mode_2;
	antic.antic_inv_blank_ptr = antic_read_byte_inv_blank_mode_2;

	/*
	 * Recalculate ch base
	 */
	antic.current_ch_base = *antic.ch_base_ptr | 
	                             (*antic.dl_mode_line_ptr & 0x7);

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 10; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_3 */

void antic_graphics_mode_4 ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	antic.ch_base_ptr = &antic.ch_base_40_mode;

	/*
	 * Assign read char and data function ptrs
	 */
	antic.antic_read_char_data_ptr = antic_read_char_data_mode_4;
	antic.antic_read_data_ptr = antic_read_data_mode_4;

	/*
	 * Assign char ctrl function pointers
	 */
	antic.antic_normal_char_ptr = antic_read_byte_mode_4_bit_0;
	antic.antic_bit1_char_ptr = antic_read_byte_mode_4_bit_1;

	/*
	 * Recalculate ch base
	 */
	antic.current_ch_base = *antic.ch_base_ptr | 
	                             (*antic.dl_mode_line_ptr & 0x7);

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 8; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_4 */

void antic_graphics_mode_5 ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	antic.ch_base_ptr = &antic.ch_base_40_mode;

	/*
	 * Assign read char and data function ptrs
	 */
	antic.antic_read_char_data_ptr = antic_read_char_data_mode_4;
	antic.antic_read_data_ptr = antic_read_data_mode_4;

	/*
	 * Assign char ctrl function pointers
	 */
	antic.antic_normal_char_ptr = antic_read_byte_mode_5_bit_0;
	antic.antic_bit1_char_ptr = antic_read_byte_mode_5_bit_1;

	/*
	 * Recalculate ch base
	 */
	antic.current_ch_base_16_line = *antic.ch_base_ptr | 
			                             ((*antic.dl_mode_line_ptr>>1) & 0x7);

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 16; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_2_TO_5][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_2_TO_5][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_5 */

void antic_graphics_mode_6 ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	antic.ch_base_ptr = &antic.ch_base_20_mode;

	/*
	 * Assign read char and data function ptrs
	 */
	antic.antic_read_char_data_ptr = antic_read_char_data_mode_6;
	antic.antic_read_data_ptr = antic_read_data_mode_4;

	/*
	 * Assign char ctrl function pointers
	 */
	antic.antic_char_pf0_ptr = antic_read_byte_mode_6_pf0;
	antic.antic_char_pf1_ptr = antic_read_byte_mode_6_pf1;
	antic.antic_char_pf2_ptr = antic_read_byte_mode_6_pf2;
	antic.antic_char_pf3_ptr = antic_read_byte_mode_6_pf3;

	/*
	 * Recalculate ch base
	 */
	antic.current_ch_base = *antic.ch_base_ptr | 
	                             (*antic.dl_mode_line_ptr & 0x7);

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_6_7][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_6_7][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 8; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_6_7][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_6_7][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_6_7][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_6_7][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_6_7][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_6_7][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_6 */

void antic_graphics_mode_7 ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	antic.ch_base_ptr = &antic.ch_base_20_mode;

	/*
	 * Assign read char and data function ptrs
	 */
	antic.antic_read_char_data_ptr = antic_read_char_data_mode_6;
	antic.antic_read_data_ptr = antic_read_data_mode_4;

	/*
	 * Assign char ctrl function pointers
	 */
	antic.antic_char_pf0_ptr = antic_read_byte_mode_7_pf0;
	antic.antic_char_pf1_ptr = antic_read_byte_mode_7_pf1;
	antic.antic_char_pf2_ptr = antic_read_byte_mode_7_pf2;
	antic.antic_char_pf3_ptr = antic_read_byte_mode_7_pf3;

	/*
	 * Recalculate ch base
	 */
	antic.current_ch_base_16_line = *antic.ch_base_ptr | 
			                             ((*antic.dl_mode_line_ptr>>1) & 0x7);

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_6_7][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_6_7][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 16; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_6_7][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_6_7][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_6_7][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_6_7][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_6_7][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_6_7][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_7 */

void antic_graphics_mode_8 ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Check for number of lines <= 0
	 */
	if ( antic.num_lines <= 0 ) {
		antic_do_display_list_instruction ();
		return;
	}

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	/*
	 * Assign read data function ptrs
	 */
	antic.antic_map_read_and_draw_ptr = antic_read_and_draw_mode_8;
	antic.antic_map_draw_ptr = antic_draw_mode_8;

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_8_9][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_8_9][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 8; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_8_9][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_8_9][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_8_9][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_8_9][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_8_9][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_8_9][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_8 */

void antic_graphics_mode_9 ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Check for number of lines <= 0 
	 */
	if ( antic.num_lines <= 0 ) {
		antic_do_display_list_instruction ();
		return;
	}

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	/*
	 * Assign read data function ptrs
	 */
	antic.antic_map_read_and_draw_ptr = antic_read_and_draw_mode_9;
	antic.antic_map_draw_ptr = antic_draw_mode_9;

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_8_9][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_8_9][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 8; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_8_9][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_8_9][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_8_9][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_8_9][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_8_9][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_8_9][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_9 */

void antic_graphics_mode_a ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Check for number of lines <= 0
	 */
	if ( antic.num_lines <= 0 ) {
		antic_do_display_list_instruction ();
		return;
	}

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	/*
	 * Assign read data function ptrs
	 */
	antic.antic_map_read_and_draw_ptr = antic_read_and_draw_mode_a;
	antic.antic_map_draw_ptr = antic_draw_mode_a;

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 8; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode */

void antic_graphics_mode_b ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Check for number of lines <= 0 
	 */
	if ( antic.num_lines <= 0 ) {
		antic_do_display_list_instruction ();
		return;
	}

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	/*
	 * Assign read data function ptrs
	 */
	antic.antic_map_read_and_draw_ptr = antic_read_and_draw_mode_b_c;
	antic.antic_map_draw_ptr = antic_draw_mode_b_c;

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 8; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_b */

void antic_graphics_mode_c ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Check for number of lines <= 0
	 */
	if ( antic.num_lines <= 0 ) {
		antic_do_display_list_instruction ();
		return;
	}

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	/*
	 * Assign read data function ptrs
	 */
	antic.antic_map_read_and_draw_ptr = antic_read_and_draw_mode_b_c;
	antic.antic_map_draw_ptr = antic_draw_mode_b_c;

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 8; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_A_B_C][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_A_B_C][ML_FIRST_LAST];
		}
	}

	antic.sl_map = antic.ml_map[0];
	antic.ml_map[antic.num_lines] = 0;

} /* end antic_graphics_mode_c */

void antic_graphics_mode_d ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Check for number of lines <= 0
	 */
	if ( antic.num_lines <= 0 ) {
		antic_do_display_list_instruction ();
		return;
	}

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	/*
	 * Assign read char and data function ptrs
	 */
	antic.antic_map_read_and_draw_ptr = antic_read_and_draw_mode_d_e;
	antic.antic_map_draw_ptr = antic_draw_mode_d_e;

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_D_E][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_D_E][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 8; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_D_E][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_D_E][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_D_E][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_D_E][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_D_E][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_D_E][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_d */

void antic_graphics_mode_e ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Check for number of lines <= 0
	 */
	if ( antic.num_lines <= 0 ) {
		antic_do_display_list_instruction ();
		return;
	}

	/*
	 * Tell gtia we are not mode 2 for PRIOR 
	 */
	gtia.prior_mask &= ~(MODE2_MASK);

	/*
	 * Assign read data function ptrs
	 */
	antic.antic_map_read_and_draw_ptr = antic_read_and_draw_mode_d_e;
	antic.antic_map_draw_ptr = antic_draw_mode_d_e;

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_D_E][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_D_E][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 8; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_D_E][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_D_E][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_D_E][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_D_E][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_D_E][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_D_E][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_e */

void antic_graphics_mode_f ( void ) {

	int i;

	antic.pc = antic.pc_fixed | ((antic.pc + 1) & 0x03ff );

	/*
	 * Check for number of lines <= 0
	 */
	if ( antic.num_lines <= 0 ) {
		antic_do_display_list_instruction ();
		return;
	}

	/*
	 * Tell gtia we are mode 2 for PRIOR 
	 */
	if ( gtia.gtia_mode == 0 ) {
		gtia.prior_mask |= MODE2_MASK;
	}

	/*
	 * Assign read data function ptrs
	 */
	antic.antic_map_read_and_draw_ptr = antic_read_and_draw_mode_f;
	antic.antic_map_draw_ptr = antic_draw_mode_f;

	/*
	 * First line
	 */
	antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_F][ML_FIRST];
	antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_F][ML_FIRST];

	/*
	 * Middle lines
	 */
	for ( i = 1; i < 8; ++i ) {
		antic.ml_map[i] = antic.dl_map[antic.width_array][MODE_F][ML_NORMAL];
		antic.ml_cycles[i] = antic.dl_cycles[antic.width_array][MODE_F][ML_NORMAL];
	}

	/*
	 * Last line if DLI
	 */
	if ( antic.dl_current_byte & 0x80 ) {
		if ( antic.num_lines > 1 ) {
			antic.ml_map[antic.num_lines-1] = antic.dl_map[antic.width_array][MODE_F][ML_LAST];
			antic.ml_cycles[antic.num_lines-1] = antic.dl_cycles[antic.width_array][MODE_F][ML_LAST];
		}
		else {
			antic.ml_map[0] = antic.dl_map[antic.width_array][MODE_F][ML_FIRST_LAST];
			antic.ml_cycles[0] = antic.dl_cycles[antic.width_array][MODE_F][ML_FIRST_LAST];
		}
	}

	antic.ml_map[antic.num_lines] = 0;
	antic.sl_map = antic.ml_map[0];

} /* end antic_graphics_mode_f */

/******************************************************************************
**  Function   :  antic_create_cycle_maps
**                            
**  Objective  :  This function creates scanline maps based on mode line
**
**  Parameters :  NONE
**                                                
**  return     :  NONE
**      
******************************************************************************/ 
void antic_create_cycle_maps ( void ) {

	int j;
	int index;

		/*
		 * Blank Line
		 */
		index = assign_start_of_line ( MODE_BLANK, ML_FIRST );
		assign_rest_of_line ( MODE_BLANK, ML_FIRST, index );

		index = assign_start_of_line ( MODE_BLANK, ML_NORMAL );
		assign_rest_of_line ( MODE_BLANK, ML_NORMAL, index );

		index = assign_start_of_line ( MODE_BLANK, ML_LAST );
		assign_rest_of_line ( MODE_BLANK, ML_LAST, index );

		index = assign_start_of_line ( MODE_BLANK, ML_FIRST_LAST );
		assign_rest_of_line ( MODE_BLANK, ML_FIRST_LAST, index );

		/*
		 * No DMA / JVB
		 */
		index = assign_start_of_line ( 1, ML_FIRST );
		for ( j = 0; j < 5; ++j ) {
			antic.dl_cycles[j][1][0][index-2] = 101;
			antic.dl_map[j][1][0][index-1] = &antic.antic_draw_blank_line_ptr;
		}

		index = assign_start_of_line ( 1, ML_NORMAL );
		assign_rest_of_line ( 1, ML_NORMAL, index );

		/*
		 * Add for Race in Space - DLI JVB
		 */
		index = assign_start_of_line ( 1, ML_LAST );
		assign_rest_of_line ( 1, ML_LAST, index );
		//for ( j = 0; j < 5; ++j ) {
	//		antic.dl_cycles[j][1][1][index-2] = 92;
	//		antic.dl_map[j][1][1][index-1] = &antic.antic_draw_blank_line_ptr;
	//	}

		/*
		 * Modes 2,3,4,5 (40 character modes)
		 */
		index = assign_start_of_line ( MODE_2_TO_5, ML_FIRST );

		for ( j = 0; j < 5; ++j ) {
			antic.dl_map[j][2][0][index] = &antic.antic_read_char_data_ptr;
			antic.dl_map[j][2][0][index+1] = &antic.run_6502_ptr;
			antic.dl_map[j][2][0][index+2] = &antic.antic_end_scanline_ptr;
			antic.dl_map[j][2][0][index+3] = 0;

			switch (j) {
				case 0:
					antic.dl_cycles[j][2][0][index] = 32;
					antic.dl_cycles[j][2][0][index+1] = 16;
					break;

				case 1:
					antic.dl_cycles[j][2][0][index] = 32;
					antic.dl_cycles[j][2][0][index+1] = 16;
					break;

				case 2:
					antic.dl_cycles[j][2][0][index] = 40;
					antic.dl_cycles[j][2][0][index+1] = 8;
					break;
				case 3:
					antic.dl_cycles[j][2][0][index] = 48;
					antic.dl_cycles[j][2][0][index+1] = 0;
					break;
				case 4:
					antic.dl_cycles[j][2][0][index] = 48;
					antic.dl_cycles[j][2][0][index+1] = 0;
					break;
			}
		}

		index = assign_start_of_line ( MODE_2_TO_5, ML_NORMAL );
		assign_rest_of_line ( MODE_2_TO_5, ML_NORMAL, index );

		index = assign_start_of_line ( MODE_2_TO_5, ML_LAST );
		assign_rest_of_line ( MODE_2_TO_5, ML_LAST, index );

		index = assign_start_of_line ( MODE_2_TO_5, ML_FIRST_LAST );
		assign_rest_of_line ( MODE_2_TO_5, ML_FIRST_LAST, index );

		/*
		 * Modes 6,7 (20 character modes)
		 */
		index = assign_start_of_line ( MODE_6_7, ML_FIRST );
		assign_rest_of_line ( MODE_6_7, ML_FIRST, index );

		index = assign_start_of_line ( MODE_6_7, ML_NORMAL );
		assign_rest_of_line ( MODE_6_7, ML_NORMAL, index );

		index = assign_start_of_line ( MODE_6_7, ML_LAST );
		assign_rest_of_line ( MODE_6_7, ML_LAST, index );

		index = assign_start_of_line ( MODE_6_7, ML_FIRST_LAST );
		assign_rest_of_line ( MODE_6_7, ML_FIRST_LAST, index );

		/*
		 * Modes 8,9 (10 byte map modes)
		 */
		index = assign_start_of_line ( MODE_8_9, ML_FIRST );
		assign_rest_of_line ( MODE_8_9, ML_FIRST, index );

		index = assign_start_of_line ( MODE_8_9, ML_NORMAL );
		assign_rest_of_line ( MODE_8_9, ML_NORMAL, index );

		index = assign_start_of_line ( MODE_8_9, ML_LAST );
		assign_rest_of_line ( MODE_8_9, ML_LAST, index );

		index = assign_start_of_line ( MODE_8_9, ML_FIRST_LAST );
		assign_rest_of_line ( MODE_8_9, ML_FIRST_LAST, index );

		/*
		 * Modes a,b,c (20 byte map modes)
		 */
		index = assign_start_of_line ( MODE_A_B_C, ML_FIRST );
		assign_rest_of_line ( MODE_A_B_C, ML_FIRST, index );

		index = assign_start_of_line ( MODE_A_B_C, ML_NORMAL );
		assign_rest_of_line ( MODE_A_B_C, ML_NORMAL, index );

		index = assign_start_of_line ( MODE_A_B_C, ML_LAST );
		assign_rest_of_line ( MODE_A_B_C, ML_LAST, index );

		index = assign_start_of_line ( MODE_A_B_C, ML_FIRST_LAST );
		assign_rest_of_line ( MODE_A_B_C, ML_FIRST_LAST, index );

		/*
		 * Modes d,e (40 byte map modes)
		 */
		index = assign_start_of_line ( MODE_D_E, ML_FIRST );
		assign_rest_of_line ( MODE_D_E, ML_FIRST, index );

		index = assign_start_of_line ( MODE_D_E, ML_NORMAL );
		assign_rest_of_line ( MODE_D_E, ML_NORMAL, index );

		index = assign_start_of_line ( MODE_D_E, ML_LAST );
		assign_rest_of_line ( MODE_D_E, ML_LAST, index );

		index = assign_start_of_line ( MODE_D_E, ML_FIRST_LAST );
		assign_rest_of_line ( MODE_D_E, ML_FIRST_LAST, index );

		/*
		 * Modes f (40 byte map mode)
		 */
		index = assign_start_of_line ( MODE_F, ML_FIRST );
		assign_rest_of_line ( MODE_F, ML_FIRST, index );

		index = assign_start_of_line ( MODE_F, ML_NORMAL );
		assign_rest_of_line ( MODE_F, ML_NORMAL, index );

		index = assign_start_of_line ( MODE_F, ML_LAST );
		assign_rest_of_line ( MODE_F, ML_LAST, index );

		index = assign_start_of_line ( MODE_F, ML_FIRST_LAST );
		assign_rest_of_line ( MODE_F, ML_FIRST_LAST, index );

} /* end antic_create_cycle_maps */

int assign_start_of_line ( int mode_type, int mode_line ) {

	int m_t = mode_type, m_l = mode_line;
	int num_changes=0;
	int j;

	switch ( m_l ) {

		/*
		 * First Line
		 */
		case ML_FIRST:
		num_changes = 6;
		for ( j = 0; j < 5; ++j ) {
			antic.dl_map[j][m_t][m_l][0] = &antic.run_6502_ptr;
			antic.dl_map[j][m_t][m_l][1] = &antic.antic_read_missle_ptr;
			antic.dl_map[j][m_t][m_l][2] = &antic.antic_do_dl_instr_ptr;
			antic.dl_map[j][m_t][m_l][3] = &antic.antic_read_player_ptr;
			antic.dl_map[j][m_t][m_l][4] = &antic.run_6502_ptr;
			antic.dl_map[j][m_t][m_l][5] = &antic.antic_init_scanline_ptr;
			antic.dl_cycles[j][m_t][m_l][0] = 7;
			antic.dl_cycles[j][m_t][m_l][1] = 1;
			antic.dl_cycles[j][m_t][m_l][2] = 1;
			antic.dl_cycles[j][m_t][m_l][3] = 4;
			antic.dl_cycles[j][m_t][m_l][5] = 0;
			switch ( j ) {
				case 0: antic.dl_cycles[j][m_t][m_l][4] = 20; break;
				case 1: antic.dl_cycles[j][m_t][m_l][4] = 20; break;
				case 2: antic.dl_cycles[j][m_t][m_l][4] = 13; break;
				case 3: antic.dl_cycles[j][m_t][m_l][4] = 5; break;
				case 4: antic.dl_cycles[j][m_t][m_l][4] = 5; break;
			}
		}

		/*
		 * Mode 2 first line is special since little refresh
		 */
		if ( m_t == MODE_2_TO_5 ) {
			antic.dl_cycles[0][m_t][m_l][4] = 19;
			antic.dl_cycles[1][m_t][m_l][4] = 19;
			antic.dl_cycles[2][m_t][m_l][4] = 12;
			antic.dl_cycles[3][m_t][m_l][4] = 5;
			antic.dl_cycles[4][m_t][m_l][4] = 5;
		}
		break;

		/*
		 * Normal Line
		 */
		case ML_NORMAL:
		num_changes = 5;
		for ( j = 0; j < 5; ++j ) {
			antic.dl_map[j][m_t][m_l][0] = &antic.run_6502_ptr;
			antic.dl_map[j][m_t][m_l][1] = &antic.antic_read_missle_ptr;
			antic.dl_map[j][m_t][m_l][2] = &antic.antic_read_player_ptr;
			antic.dl_map[j][m_t][m_l][3] = &antic.run_6502_ptr;
			antic.dl_map[j][m_t][m_l][4] = &antic.antic_init_scanline_ptr;
			antic.dl_cycles[j][m_t][m_l][0] = 7;
			antic.dl_cycles[j][m_t][m_l][1] = 1;
			antic.dl_cycles[j][m_t][m_l][2] = 4;
			antic.dl_cycles[j][m_t][m_l][4] = 0;
			switch ( j ) {
				case 0: antic.dl_cycles[j][m_t][m_l][3] = 21; break;
				case 1: antic.dl_cycles[j][m_t][m_l][3] = 21; break;
				case 2: antic.dl_cycles[j][m_t][m_l][3] = 14; break;
				case 3: antic.dl_cycles[j][m_t][m_l][3] = 6; break;
				case 4: antic.dl_cycles[j][m_t][m_l][3] = 6; break;
			}
		}
		break;

		/*
		 * Last Line
		 */
		case ML_LAST:
		num_changes = 7;
		for ( j = 0; j < 5; ++j ) {
			antic.dl_map[j][m_t][m_l][0] = &antic.run_6502_ptr;
			antic.dl_map[j][m_t][m_l][1] = &antic.antic_read_missle_ptr;
			antic.dl_map[j][m_t][m_l][2] = &antic.antic_read_player_ptr;
			antic.dl_map[j][m_t][m_l][3] = &antic.run_6502_ptr;
			antic.dl_map[j][m_t][m_l][4] = &antic.antic_set_dli_ptr;
			antic.dl_map[j][m_t][m_l][5] = &antic.run_6502_ptr;
			antic.dl_map[j][m_t][m_l][6] = &antic.antic_init_scanline_ptr;
			antic.dl_cycles[j][m_t][m_l][0] = 7;
			antic.dl_cycles[j][m_t][m_l][1] = 1;
			antic.dl_cycles[j][m_t][m_l][2] = 4;
			antic.dl_cycles[j][m_t][m_l][3] = antic.dli_count;
			antic.dl_cycles[j][m_t][m_l][5] = 0;
			antic.dl_cycles[j][m_t][m_l][6] = 0;
			switch ( j ) {
				case 0: antic.dl_cycles[j][m_t][m_l][5] = 21-antic.dli_count; break;
				case 1: antic.dl_cycles[j][m_t][m_l][5] = 21-antic.dli_count; break;
				case 2: antic.dl_cycles[j][m_t][m_l][5] = 14-antic.dli_count; break;
				case 3: antic.dl_cycles[j][m_t][m_l][5] = 6-antic.dli_count; break;
				case 4: antic.dl_cycles[j][m_t][m_l][5] = 6-antic.dli_count; break;
			}
		}
		break;
		
		/*
		 * Combo First / Last Line
		 */
		case ML_FIRST_LAST:
		num_changes = 8;
		for ( j = 0; j < 5; ++j ) {
			antic.dl_map[j][m_t][m_l][0] = &antic.run_6502_ptr;
			antic.dl_map[j][m_t][m_l][1] = &antic.antic_read_missle_ptr;
			antic.dl_map[j][m_t][m_l][2] = &antic.antic_do_dl_instr_ptr;
			antic.dl_map[j][m_t][m_l][3] = &antic.antic_read_player_ptr;
			antic.dl_map[j][m_t][m_l][4] = &antic.run_6502_ptr;
			antic.dl_map[j][m_t][m_l][5] = &antic.antic_set_dli_ptr;
			antic.dl_map[j][m_t][m_l][6] = &antic.run_6502_ptr;
			antic.dl_map[j][m_t][m_l][7] = &antic.antic_init_scanline_ptr;
			antic.dl_cycles[j][m_t][m_l][0] = 7;
			antic.dl_cycles[j][m_t][m_l][1] = 1;
			antic.dl_cycles[j][m_t][m_l][2] = 1;
			antic.dl_cycles[j][m_t][m_l][3] = 4;
			antic.dl_cycles[j][m_t][m_l][4] = antic.dli_count;
			antic.dl_cycles[j][m_t][m_l][5] = 0;
			antic.dl_cycles[j][m_t][m_l][7] = 0;
			switch ( j ) {
				case 0: antic.dl_cycles[j][m_t][m_l][6] = 20-antic.dli_count; break;
				case 1: antic.dl_cycles[j][m_t][m_l][6] = 20-antic.dli_count; break;
				case 2: antic.dl_cycles[j][m_t][m_l][6] = 13-antic.dli_count; break;
				case 3: antic.dl_cycles[j][m_t][m_l][6] = 5-antic.dli_count; break;
				case 4: antic.dl_cycles[j][m_t][m_l][6] = 5-antic.dli_count; break;
			}
		}
		break;
	}

	return num_changes;

} /* end assign_start_of_line */

void assign_rest_of_line ( int mode_type, int mode_line, int start ) {

	void ( **antic_func) (void);
	int antic_bytes=0, cpu_bytes=0;
	int cpu_add_on[5] = {16,16,8,0,0};
	int cycle_changes[5] = {16,16,20,24,24};
	int m_t = mode_type, m_l = mode_line;
	int j,k;

	/*
	 * We set this up to draw 32 pixels (16 color clocks) at a time
	 */

	/*
	 * Assign bytes based on mode type
	 */
	switch ( m_t ) {
		case MODE_BLANK: 
			antic_func = &antic.antic_blank_ptr;
			antic_bytes=1; cpu_bytes = 8; 
			break;
		case 1: 
			antic_func = &antic.antic_blank_ptr;
			antic_bytes=1; cpu_bytes = 8; 
			break;
		case MODE_2_TO_5: 
			if ( m_l == 0 || m_l == ML_FIRST_LAST ) {
				antic_func = &antic.antic_read_char_data_ptr;
				antic_bytes=4; cpu_bytes = 0; 
			}
			else {
				antic_func = &antic.antic_read_data_ptr;
				antic_bytes=4; cpu_bytes = 4; 
			}
			break;
		case MODE_6_7: 
			if ( m_l == 0 || m_l == ML_FIRST_LAST ) {
				antic_func = &antic.antic_read_char_data_ptr;
				antic_bytes=2; cpu_bytes = 4; 
			}
			else {
				antic_func = &antic.antic_read_data_ptr;
				antic_bytes=2; cpu_bytes = 6; 
			}
			break;
		case MODE_8_9: 
			if ( m_l == 0 || m_l == ML_FIRST_LAST ) {
				antic_func = &antic.antic_map_read_and_draw_ptr;
				antic_bytes=1; cpu_bytes = 7; 
			}
			else {
				antic_func = &antic.antic_map_draw_ptr;
				antic_bytes=1; cpu_bytes = 8; 
			}
			break;
		case MODE_A_B_C: 
			if ( m_l == 0 || m_l == ML_FIRST_LAST ) {
				antic_func = &antic.antic_map_read_and_draw_ptr;
				antic_bytes=2; cpu_bytes = 6; 
			}
			else {
				antic_func = &antic.antic_map_draw_ptr;
				antic_bytes=2; cpu_bytes = 8; 
			}
			break;
		case MODE_D_E: 
		case MODE_F: 
			if ( m_l == 0 || m_l == ML_FIRST_LAST ) {
				antic_func = &antic.antic_map_read_and_draw_ptr;
				antic_bytes=4; cpu_bytes = 4; 
			}
			else {
				antic_func = &antic.antic_map_draw_ptr;
				antic_bytes=4; cpu_bytes = 8; 
			}
			break;
		default:
			antic_func = 0;
			antic_bytes=0; cpu_bytes = 0; 
			break;
	}

	/*
	 * Assign functions
	 */
	for ( j = 0; j < 5; ++j ) {
		for ( k = 0; k < cycle_changes[j]; k+=2 ) {
			antic.dl_map[j][m_t][m_l][start+k] = antic_func;
			antic.dl_map[j][m_t][m_l][start+k+1] = &antic.run_6502_ptr;
			antic.dl_cycles[j][m_t][m_l][start+k] = antic_bytes;
			antic.dl_cycles[j][m_t][m_l][start+k+1] = cpu_bytes;
		}

		/*
		 * End the list with an end scanline ptr and then 0
		 */
		antic.dl_map[j][m_t][m_l][cycle_changes[j]+start] = &antic.antic_end_scanline_ptr;
		antic.dl_map[j][m_t][m_l][cycle_changes[j]+start+1] = 0;

		/*
		 * Lets add the cpu cycles to the end of the line
		 */
		antic.dl_cycles[j][m_t][m_l][cycle_changes[j]+start-1] += cpu_add_on[j];

		/*
		 * Lets take out cpu cycles for memory refresh
		 */
		switch ( j ) {

			/*
			 * Narrow Screen take 2 out of the first 4 cycles
			 */
			case 0:
			case 1:
				for ( k = 0; k < 8; k+=2 )
					antic.dl_cycles[j][m_t][m_l][start+k+1] -= 2;
				break;

			/*
			 * Standard Screen take 1 out of the first cycle
			 *   then 2 out of the next 4                  
			 */
			case 2:
				antic.dl_cycles[j][m_t][m_l][start+1] -= 1;
				for ( k = 2; k < 10; k+=2 )
					antic.dl_cycles[j][m_t][m_l][start+k+1] -= 2;
				break;

			/*
			 * Wide Screen take 1 out of the second cycle
			 *   then 2 out of the next 4
			 */
			case 3:
			case 4:
				antic.dl_cycles[j][m_t][m_l][start+3] -= 1;
				for ( k = 2; k < 10; k+=2 )
					antic.dl_cycles[j][m_t][m_l][start+k+1] -= 2;
				break;

		} /* end take out cycles for memory refresh */

	} /* end for playfield width */

} /* end assign_rest_of_line */
