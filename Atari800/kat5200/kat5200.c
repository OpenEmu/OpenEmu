/******************************************************************************
*
* FILENAME: kat5200.c
*
* DESCRIPTION:  This contains main and functions for running the system
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     07/18/04  bberlin      Creation
* 0.2     03/09/06  bberlin      Updated to use GUI on loading failure
* 0.3.0   04/08/06  bberlin      Added call to 'config_update'
******************************************************************************/
#include <stdio.h>
#include <strings.h>
#include <zlib.h>
#include "kat5200.h"
#include "core/antic.h"
#include "core/memory.h"
#include "core/debug.h"
#include "core/sio.h"
#include "interface/util.h"
#include "interface/media.h"
#include "interface/kconfig.h"
#include "interface/input.h"
#include "interface/video.h"
#include "interface/sound.h"
#include "interface/ui.h"
#include "interface/db_if.h"
#include "gui/gui.h"
#include "interface/logger.h"
#include "interface/states.h"

extern t_atari_sound g_sound;
extern struct antic antic;
extern struct cpu cpu6502;
extern struct debug debug;

double frametime = 1.0;
unsigned int *g_frame;

/******************************************************************************
**  Function   :  main
**                                                
**  Objective  :  This is the main used to run kat5200
**                                               
**  Parameters :  NONE                          
**                                             
**  return     :  0 if success                
**                1 if failure               
******************************************************************************/ 
int main ( int argc, char *argv[]  ) {

	t_config *p_config = config_get_ptr();
	struct atari *p_console = console_get_ptr();
	int checking_files = 1;
	int start_with_gui;
	int status=0;
	int patch=0;
	int os_status=0;
	int ram_size = 64;
	int state_load = STATE_LOAD_NONE;

	/*
	 * Initialize our interface
	 */
	util_init ( argv[0] );
	logger_init();
	debug_init();
	console_init();
	mem_init();
	db_if_init();
	state_load = config_init ( argc, argv );
	sio_init();
//	pc_init();

	input_set_defaults();
	video_set_defaults();
	sound_set_defaults();
	ui_set_defaults();

	/*
	 * Setup patches
	 */
	if ( p_config->sio_patch )
		patch |= ESC_CODE_SIO;

	/*
	 * FIXME: Do we need this here?
	 *   For now, we need so GUI loads up something
	 */
	if ( p_config->machine_type != MACHINE_TYPE_5200 )
		input_set_profile( p_config->default_800input_profile );
	else
		input_set_profile( p_config->default_input_profile );
	video_set_profile( p_config->default_video_profile );
	sound_set_profile( p_config->default_sound_profile );
	ui_set_profile( p_config->default_ui_profile );

	start_with_gui = p_config->start_with_gui;

	/*
	 * Check for both OS and ROM, else go to GUI
	 */
	while ( checking_files ) {

		checking_files = 0;

		/*
		 * Check for state load
		 */
		if ( state_load == STATE_LOAD_FILE ) {
			if ( recall_state ( p_config->state, p_console ) )
				checking_files = 1;
			mem_assign_ops ( p_console->cpu );
		}
		else if ( state_load > 0 && state_load < 10 ) {
			if ( recall_quick_state ( state_load, p_console ) )
				checking_files = 1;
			mem_assign_ops ( p_console->cpu );
		}

		/*
		 * Else load the OS and image
		 */
		else if ( (status=media_open_rom (p_config->rom_file, 0, 0, 1 )) || 
		          (os_status=load_os ( config_get_os_file (), p_config->machine_type )) || 
		          p_config->exit_status || start_with_gui ) {
			checking_files = 1;
		}

		if ( checking_files ) {
			start_with_gui = 0;
			state_load = 0;
			p_config->exit_status = 0;
			if ( status ) {
				if ( !strlen(p_config->rom_file) )
					status = 0;
				status = gui_show_main (status, 0);
			}
			else
				status = gui_show_main (os_status, 0);

			/*
			 * FIXME: If GUI loads something break the loop so we don't load twice
			 */
			if ( status == -1 ) {
				return 0;
			}
			else if ( status == 2 ) { /* we loaded a state in the gui */
				state_load = 1;
				checking_files = 0;
			}
		}

	} /* end while checking files */

	/*
	 * Load cart specific controls
	 */
	input_set_profile ( media_get_ptr()->input_profile );
	video_set_profile ( media_get_ptr()->video_profile );
	sound_set_profile ( media_get_ptr()->sound_profile );
	ui_set_profile( p_config->default_ui_profile );

	/*
	 * Initialize Sub-Systems
	 */
	if ( !state_load ) {
		if ( media_get_ptr()->ram_size )
			ram_size = media_get_ptr()->ram_size;
		else
			ram_size = p_config->ram_size[media_get_ptr()->machine_type];
		console_reset ( p_config->machine_type, p_config->system_type, ram_size, g_sound.freq );
	}
	
//	pc_game_init ();

	/*
	 * GO!
	 */
	run_system ( );

	/*
	 * Goodbye
	 */
	sio_unmount_all();
	config_set_exit_status ();
//	pc_exit ();
	logger_exit();
	db_if_close();

	return 0;

} /* end main */

/******************************************************************************
**  Function   :  run_system                
**                                         
**  Objective  :  Run the console system  
**                                       
**  Parameters :  NONE                  
**                                     
**  return     :  0 if success        
**                1 if failure       
******************************************************************************/ 
int run_system (void) {

	double alpha = 0.2;
	double throttle_time = 0;
	unsigned int getticks,frametimedelta,frametimelast = 1;
	struct atari *p_console = console_get_ptr();
	t_config *p_config = config_get_ptr();

	int run = 1;
	int status;
	int start,stop;

	char msg[257];

	/*
	 * Check for debug
	 */
	if ( p_console->debug_mode )
		debug_get_input ();

	antic.dl_scanline = NTSC_SCAN_LINES;

	/*
	 * Calculate throttle_time and start timing
	 */
	throttle_time = ((1.0/p_console->frames_per_sec) * 1000.0) * 
	                (1/((double)p_config->throttle/100.0));

	start = pc_get_ticks ();

	/*
	 * Begin the main loop
	 */
	while ( run ) {

		/*
		 * Run CPU for VBLANK time - memory refresh if DMA
		 */
		//antic.run_6502_ptr = run_6502;
		while ( antic.dl_scanline < p_console->scanlines ) {
			antic.vcount = antic.dl_scanline >> 1;
			pokey_init_scanline ();
			cpu6502.i_cnt = antic.vblank_sl_time;
			//antic_5200.run_6502_ptr();
			run_6502 ();
			antic.dl_scanline++;
		}

		antic.dl_scanline = 0;;

		/*
		 * Now check for PC events
		 */
		if ( (status = pc_poll_events ()) ) {
			if ( status < 0 )
				break;
			else {
				throttle_time = ((1.0/p_console->frames_per_sec) * 1000.0) * 
				                (1/((double)p_config->throttle/100.0));

				start = pc_get_ticks ();
			}
		}

		/*
		 * Do speed throttling if desired
		 */
		if ( p_config->throttle ) {
			stop = pc_get_ticks ();
			while ( (stop-start) < throttle_time ) {
				stop = pc_get_ticks ();
			}
			start = pc_get_ticks ();
			/*start = pc_get_ticks () + (stop-start-throttle_time);*/
		}

		/*
		 * First 8 scanlines no drawing?
		 */
		while ( antic.dl_scanline < 8 ) {
			antic.vcount = antic.dl_scanline >> 1;
			pokey_init_scanline ();
			cpu6502.i_cnt = antic.vblank_sl_time;
			//antic_5200.run_6502_ptr();
			run_6502 ();
			antic.dl_scanline++;
		}

		/*
		 * Reset cycle map pointers
		 */
		antic.dl_byte_number = 0;
		antic.ml_map[0] = antic.dl_map[antic.pl_width][1][0];
		antic.ml_map[1] = 0;

		antic.ml_cycles[0] = antic.dl_cycles[antic.pl_width][1][0];
		antic.vscroll_taking_place = 0;

		/*
		 * Read Display List from ANTIC until JVB
		 */
		while ( antic.dl_scanline < NTSC_SCAN_LINES+8 ) {

			if ( debug.breakpoint_flag )
				debug_search_for_breakpoint (BK_SCANLINE, antic.dl_scanline );

			/*
			 * Do Mode Lines
			 */
			antic.mode_line = -1;
			while ( antic.ml_map[++antic.mode_line] ) {

				antic.sl_map = antic.ml_map[antic.mode_line];

				/*
				 * Every Scan line do pokey stuff
				 */
				pokey_init_scanline ();

				/*
				 * Every scanline reassign 6502 in case of WSYNC
				 */
				antic.run_6502_ptr = antic_run_6502;

				/*
				 * Every scanline update counters
				 */
				antic.vcount = antic.dl_scanline >> 1;
				antic.byte_count = -1;
				antic.current_ch_base = *antic.ch_base_ptr | (*antic.dl_mode_line_ptr & 0x7);
				antic.current_ch_base_16_line = *antic.ch_base_ptr | ((*antic.dl_mode_line_ptr>>1) & 0x7);

				/*
				 * Make it so we skip first 7 6502 cycles on line 1
				 * FIXME: Then make up 7 at VBLANK - is that right?
				 */
				if ( antic.dl_scanline == 8 )
					antic.scan_change = 0;
				else
					antic.scan_change = -1;

				/*
				 * Reassign player missle stuff
				 */
				if ( antic.dl_scanline & 0x1 ) {
					antic.antic_read_missle_ptr = antic.antic_read_odd_missle_ptr;
					antic.antic_read_player_ptr = antic.antic_read_odd_player_ptr;
				}
				else {
					antic.antic_read_missle_ptr = antic.antic_read_even_missle_ptr;
					antic.antic_read_player_ptr = antic.antic_read_even_player_ptr;
				}

				/*
				 * Run through the scanline
				 */
				while ( antic.sl_map[++antic.scan_change] ) {

					antic.sl_map[antic.scan_change][0] ();

				} /* end scan line */

				/*
				 * Update mode line counters
				 */
				antic.dl_scanline++;
				antic.dl_mode_line++;
				antic.dl_mode_line_opp--;

				/*
				 * Check for going over scanline limit
				 *       Fixes several games (xevious)
				 */
				if ( antic.dl_scanline >= NTSC_SCAN_LINES+8 )
					break;

				if ( debug.breakpoint_flag )
					debug_search_for_breakpoint (BK_SCANLINE, antic.dl_scanline );

			} /* end mode line */

		} /* end while doing scanlines */
			
		/*
		 * If ANTIC allows, interupt the 6502 for VBLANK
		 */
		if ( antic.nmi_vbi_en ) {
			/*antic.nmi_status_reg |= 0x40;*/
			antic.nmi_status_reg = 0x5f;
			cpu6502.i_req = INT_NMI;
			int_6502 ();
		}

		/*
		 * For frame timing calculations
		 */
		p_console->frame_number++;
		getticks = pc_get_ticks();
		frametimedelta = getticks - frametimelast;
		frametimelast = getticks;
		frametime = alpha * frametimedelta + (1.0 - alpha) * frametime;

	} /* end main loop                                   */

	sprintf ( msg, "Console: frames/sec = %lf", 1000.0/frametime ); 
	logger_log_message ( LOG_ERROR, msg, "" );

	return 0;
	
} /* end run_system */

/******************************************************************************
**  Function   :  load_os                                            
**                                                                    
**  Objective  :  This function opens the OS file and puts contents in memory
**                                                   
**  Parameters :  filename - file to open
**                type     - machine type used for where to load in memory
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
int load_os ( char *filename, e_machine_type type ) {

	gzFile zip_fp;
	t_config *p_config = config_get_ptr();
	int bytes;
	char msg[1100];
	char *buffer;
	unsigned char sio_patch_bytes[3] = {0xf2,ESC_CODE_SIO,0x60};
	unsigned char disable_chksm_bytes[4] = {0x8e,0xff,0x8e,0xff};
	int patch = 0;

	if ( p_config->sio_patch )
		patch |= ESC_CODE_SIO;

	buffer = malloc(sizeof(char)*131072);

	util_set_file_to_program_dir ( filename );

	zip_fp = gzopen ( filename, "rb" );

	if ( zip_fp ) {
		bytes = gzread ( zip_fp, buffer, 131072 );
		sprintf ( msg, "load_os: Opened BIOS File \"%s\"", filename );
		logger_log_message ( LOG_INFO, msg, "" );
	}
	else {
		sprintf ( msg, "load_os: Unable to Open BIOS File \"%s\"", filename );
		logger_log_message ( LOG_ERROR, msg, "" );
		free ( buffer );
		return ERR_BIOS_LOAD; 
	}

	switch ( type ) {
		case MACHINE_TYPE_MIN: 
			memory_write_rom_bytes ( 0xf800, buffer, 0x800 );
			break;
		case MACHINE_TYPE_5200: 
			memory_write_rom_bytes ( 0xf800, buffer, 0x800 );
			break;
		case MACHINE_TYPE_800: 
			memory_write_rom_bytes ( 0xd800, buffer, 0x2800 );
			if ( patch & ESC_CODE_SIO )
				memory_write_rom_bytes ( 0xe459, sio_patch_bytes, 3 );
			if ( (p_config->machine_type == MACHINE_TYPE_XL) && patch ) {
				memory_write_rom_bytes ( 0xc314, disable_chksm_bytes, 2 );
				memory_write_rom_bytes ( 0xc319, disable_chksm_bytes+2, 2 );
			}
			break;
		case MACHINE_TYPE_XL: 
			memory_write_os_bytes ( 0xc000, buffer, 0x4000 );
			if ( patch & ESC_CODE_SIO ) 
				memory_write_os_bytes ( 0xe459, sio_patch_bytes, 3 );
			if ( (p_config->machine_type == MACHINE_TYPE_XL) && patch ) {
				memory_write_os_bytes ( 0xc314, disable_chksm_bytes, 2 );
				memory_write_os_bytes ( 0xc319, disable_chksm_bytes+2, 2 );
			}
			break;
	}

	free ( buffer );

	gzclose (zip_fp);

	return 0;

} /* end load_os */

