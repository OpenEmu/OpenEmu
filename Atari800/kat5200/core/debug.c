/******************************************************************************
*
* FILENAME: debug.c
*
* DESCRIPTION:  This contains debug functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.5.0   02/11/07  bberlin      Creation, breakout from util
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "boom6502.h"
#include "memory.h"
#include "gtia.h"
//#include "../interface/sdl_if.h"
#include "../interface/kconfig.h"
#include "../gui/gui.h"

void debug_print_value ( char *arg );

struct debug debug;

FILE *m_cpu;
FILE *m_antic;

static struct atari *atari;
extern struct cpu cpu6502;

/******************************************************************************
**  Function   :  debug_init                                            
**                                                                    
**  Objective  :  This function initializes debugger stuff               
**                                                   
**  Parameters :  NONE
**                 
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
void debug_init ( void ) {

	int i;

	atari = console_get_ptr ();

	debug.breakpoint_flag = 0;
	debug.single_step_flag = 0;

	for ( i = 0; i < DEBUG_MAX_BK; ++i )
			debug.breakpoint[i] = BK_NONE;

	return;

} /* end debug_init */

/******************************************************************************
**  Function   :  debug_print_cpu_op
**
**  Objective  :  This function is used to print op_code debug info 
**
**  Parameters :  NONE
**                   
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
void debug_print_cpu_op ( int point_of_debug ) {

	/* PC has been incremented by 1 when we get here */
	if ( point_of_debug == BEFORE_OP ) {
		fprintf ( m_cpu, "%02x ", atari->cpu->t_reg );
		fprintf ( m_cpu, "%-12s ", atari->cpu->op_name[atari->cpu->t_reg] );
		fprintf ( m_cpu, "pc=%04x ", atari->cpu->pc-1 );
	}
	else if ( point_of_debug == AFTER_OP ) {
		fprintf ( m_cpu, "           " );
		fprintf ( m_cpu, "%-12s ", "            " );
	}

	fprintf ( m_cpu, "op=%04x ", rd_6502(atari->cpu->pc) | (rd_6502(atari->cpu->pc+1) << 8) );
	fprintf ( m_cpu, "a=%04x ", atari->cpu->a_reg );
	fprintf ( m_cpu, "x=%04x ", atari->cpu->x_reg );
	fprintf ( m_cpu, "y=%04x ", atari->cpu->y_reg );
	fprintf ( m_cpu, "t=%04x ", atari->cpu->t_reg );
	fprintf ( m_cpu, "h=%04x ", atari->cpu->h_reg );
	fprintf ( m_cpu, "sp=%04x ", atari->cpu->stack_ptr );
	fprintf ( m_cpu, "n%d", (atari->cpu->status >> 7) & 1 );
	fprintf ( m_cpu, "v%d", (atari->cpu->status >> 6) & 1 );
	fprintf ( m_cpu, "r%d", (atari->cpu->status >> 5) & 1 );
	fprintf ( m_cpu, "b%d", (atari->cpu->status >> 4) & 1 );
	fprintf ( m_cpu, "d%d", (atari->cpu->status >> 3) & 1 );
	fprintf ( m_cpu, "i%d", (atari->cpu->status >> 2) & 1 );
	fprintf ( m_cpu, "z%d", (atari->cpu->status >> 1) & 1 );
	fprintf ( m_cpu, "c%d", (atari->cpu->status >> 0) & 1 );
	fprintf ( m_cpu, "\n" );
		
} /* end debug_print_cpu_op */

/******************************************************************************
**  Function   :  print_addr_val
**
**  Objective  :  This function is used to print mem access info
**
**  Parameters :  NONE  
**
**  return     :  0 if success
**                1 if failure
******************************************************************************/ 
void print_addr_val ( int access_type, WORD addr, BYTE val ) {

		printf ( "           " );
		printf ( "%-12s ", "            " );
		printf ( "        " );

		if ( access_type == READ_MEM )
			printf ( "Read  " );
		else
			printf ( "Write " );
		printf ( "%04x, ", addr );
		printf ( "%04x\n", val );

} /* end print_addr_val */

int debug_crash_to_gui ( void ) {
	int i;

	i = gui_show_main (0,1);
	pc_game_init ();
	if ( !i )
		debug_get_input ();
	else if ( i == -1 ) {
		config_set_exit_status ();
		pc_exit ();
		exit ( 0 );
	}
	return i;
}

void debug_get_input (void) {

	char command;
	char arg[257];
	char *arg_ptr;
	int get_input_flag;
	int value;
	int i;

	pc_set_debugger_video ();

	debug.flag = 1;

	get_input_flag = 1;
	while ( get_input_flag ) {
	
		printf ( "debug>" );
		
		fflush (stdout);

		//scanf("debug>%c", &command );
		fgets(arg,256,stdin);

		if ( strlen(arg) > 0 )
			arg[strlen(arg)-1]='\0';
		
		command = arg[0];

		switch ( command ) {
			case BREAKPOINT:
				value = -1;
				if ( (arg_ptr = strstr(arg,"sl")) ) {
					value = atoi ( arg_ptr+2);
					if ( value > -1  ) {
						for ( i = 0; i < DEBUG_MAX_BK; ++i ) {
							if ( debug.breakpoint[i] == BK_NONE )
								break;
						}
						if ( i < DEBUG_MAX_BK ) {
							debug.breakpoint[i] = BK_SCANLINE;
							debug.breakpoint_place[i] = value;
							printf ( "Breakpoint %d at scanline %d\n", i+1, value );
							debug.breakpoint_flag = 1;
						}
						else
							printf ( "No more breakpoints left!\n" );					
					}
					else
						printf ( "How about you enter the Breakpoint command right!\n" );
				}
				else if ( (arg_ptr = strstr(arg,"mem")) ) {
					sscanf ( arg_ptr, "mem %x", &value );
					if ( value >= 0 ) {
						for ( i = 0; i < DEBUG_MAX_BK; ++i ) {
							if ( debug.breakpoint[i] == BK_NONE )
								break;
						}
						if ( i < DEBUG_MAX_BK ) {
							debug.breakpoint[i] = BK_MEM;
							debug.breakpoint_place[i] = value;
							printf ( "Breakpoint %d at address %4x\n", i+1, value );
							debug.breakpoint_flag = 1;
							mem_assign_ops_debug ( atari->cpu, &debug );
						}
						else
							printf ( "No more breakpoints left!\n" );					
					}
				}
				else {
					printf ( "How about you enter the Breakpoint command right!\n" );
				}
				break;
			case WATCH:
				sscanf ( &arg[1], "%x", &value );
				if ( value >= 0 ) {
					for ( i = 0; i < DEBUG_MAX_BK; ++i ) {
						if ( debug.breakpoint[i] == BK_NONE )
							break;
					}
					if ( i < DEBUG_MAX_BK ) {
						debug.breakpoint[i] = BK_WATCH;
						debug.breakpoint_place[i] = value;
						printf ( "Watch %d at address %4x\n", i+1, value );
						debug.breakpoint_flag = 1;
						mem_assign_ops_debug ( atari->cpu, &debug );
					}
					else
						printf ( "No more breakpoints left!\n" );					
				}
				break;
			case DELETE_BK:
				value = 0;
				value = atoi ( arg+1 );
				if ( value > 0  && value < DEBUG_MAX_BK+1) {
					debug.breakpoint[value-1] = BK_NONE;
					printf ( "Breakpoint %d deleted\n", value );					
					debug.breakpoint_flag = 0;
					for ( i = 0; i < DEBUG_MAX_BK; ++i ) {
						if ( debug.breakpoint[i] != BK_NONE ) {
							debug.breakpoint_flag = 1;
							break;
						}
					}
					mem_assign_ops_debug ( atari->cpu, &debug );
				}
				break;
			case TRACE:
				if ( strstr(arg,"cpu") ) {
					if ( (arg_ptr = strstr (arg,":")) ) {
						strcpy ( debug.trace_cpu_file, &arg_ptr[1] );
						debug.trace_cpu_flag = 1;
						m_cpu = fopen ( debug.trace_cpu_file, "w" );
					}
					else {
						printf ( "Error in trace command\n" );
						printf ( "t - trace processor:filename(ex. t cpu:cpu.txt)\n" );					
					}
				}
				else if ( strstr(arg,"antic" ) ) {
					if ( (arg_ptr = strstr (arg,":")) ) {
						strcpy ( debug.trace_antic_file, &arg_ptr[1] );
						debug.trace_antic_flag = 1;
					}
					else {
						printf ( "Error in trace command\n" );
						printf ( "t - trace processor:filename(ex. t antic:antic.txt)\n" );					
					}				
				}
				else {
					printf ( "Error in trace command\n" );
					printf ( "t - trace processor:filename(ex. t cpu:cpu.txt)\n" );				
				}
				break;
			case STOPTRACE:
				if ( strstr(arg,"cpu") ) {
					debug.trace_cpu_flag = 0;
					fclose ( m_cpu );
					m_cpu = 0;
				}
				else if ( strstr(arg,"antic" ) )
					debug.trace_antic_flag = 0;
				else {
					printf ( "Error in stop trace command\n" );
					printf ( "x - stop trace (ex. x cpu)\n" );				
				}
				break;
			case PRINT:
				debug_print_value ( arg );
				break;
			case DEBUG_QUIT:
				i = gui_show_main (0,0);
				pc_game_init();
				if ( i == -1 ) {
					config_set_exit_status ();
					pc_exit ();
					exit ( 0 );
				}
				if ( i ) {
					debug.flag = 0;
					get_input_flag = 0;
					debug.single_step_flag = 0;
				}
				break;
			case CONT:
				debug.flag = 0;
				get_input_flag = 0;
				debug.single_step_flag = 0;
				break;
			case STEP:
				debug.flag = 0;
				get_input_flag = 0;
				debug.single_step_flag = 1;
				break;
			case HELP:
				printf ( "Debugger Help\n" );
				printf ( "w value      - Watch address for changes\n" );
				printf ( "p type       - print value\n" );
				printf ( "b type value - set breakpoint\n" );
				printf ( "t - trace processor:filename(ex. t cpu:cpu.txt)\n" );
				printf ( "x - stop trace (ex. x cpu)\n" );
				printf ( "s - single step cpu\n" );
				printf ( "c - continue\n" );
				printf ( "h - display this help\n" );
				break;
		};
	
	} /* end while getting input */

} /* end debug_get_input */

void debug_print_value ( char *arg ) {

	int i,j;
	char msg[100];
	int address = 0;
	int items = 0;
	int temp_flag = 0;
	char *mem_cmd = 0;
	char type = 0;

	/* Make sure no breakpoints encountered when reading */
	temp_flag = debug.breakpoint_flag;
	debug.breakpoint_flag = 0;

	/***********************************************************/
	/*                        RAM                              */  
	/***********************************************************/
	if ( (mem_cmd = strstr(arg,"mem")) ) {
		sscanf ( mem_cmd, "%s %x %d %c", msg, &address, &items, &type );

		/* if items and type are set print in a list      */
		for ( i = 0; i < items; i++ ) {
			if ( type == 'b' ) {
				printf ( "%04x ", address+i );
				for ( j = 0x80; j > 0; j >>= 1 ) {
					printf ( "%1d", ((rd_6502(address+i) & j) ? 1:0) );
				}
				printf ( "\n" );
			}
			else
				printf ( "%04x %02x\n", address+i, rd_6502(address+i) );
		}

		/* if not amount for items, then do an auto print */
		if ( items == 0 ) {
			for ( i = 0; i < 10; ++i ) {
				printf ( "%04x :", address+i*16 );
				for ( j = 0; j < 16; ++j ) {
					printf ( " %02x", rd_6502(address+(i*16)+j) );
				}
				printf ( "\n" );
			}
		}
	}

	/***********************************************************/
	/*                     CPU                                 */  
	/***********************************************************/
	else if ( strstr(arg,"cpu") || strstr(arg,"CPU") ) {
		printf ( "%02x ", atari->cpu->t_reg );
		printf ( "%-12s ", atari->cpu->op_name[atari->cpu->t_reg] );
		printf ( "pc=%04x ", atari->cpu->pc-1 );
		printf ( "op=%04x ", rd_6502(atari->cpu->pc) | (rd_6502(atari->cpu->pc+1) << 8) );
		printf ( "a=%04x ", atari->cpu->a_reg );
		printf ( "x=%04x ", atari->cpu->x_reg );
		printf ( "y=%04x ", atari->cpu->y_reg );
		printf ( "t=%04x ", atari->cpu->t_reg );
		printf ( "h=%04x ", atari->cpu->h_reg );
		printf ( "sp=%04x ", atari->cpu->stack_ptr );
		printf ( "n%d", (atari->cpu->status >> 7) & 1 );
		printf ( "v%d", (atari->cpu->status >> 6) & 1 );
		printf ( "r%d", (atari->cpu->status >> 5) & 1 );
		printf ( "b%d", (atari->cpu->status >> 4) & 1 );
		printf ( "d%d", (atari->cpu->status >> 3) & 1 );
		printf ( "i%d", (atari->cpu->status >> 2) & 1 );
		printf ( "z%d", (atari->cpu->status >> 1) & 1 );
		printf ( "c%d", (atari->cpu->status >> 0) & 1 );
		printf ( "\n" );
	}

	/***********************************************************/
	/*                     GTIA REGISTERS                      */  
	/***********************************************************/
	else if ( strstr(arg,"gtia") || strstr (arg,"GTIA") ) {
		printf ( "GTIA Registers\n" );
		printf ( "HPOSP0=%2x  SIZEP0=%2x  GRAFP0=%2x  COLPM0=%2x\n", 
		         atari->gtia->hpos_p0_reg >> 1, atari->gtia->w_reg[SIZEP0],
		         atari->gtia->graph_p0_reg, atari->gtia->color_pm0_reg );
		printf ( "HPOSP1=%2x  SIZEP1=%2x  GRAFP1=%2x  COLPM1=%2x\n",
		         atari->gtia->hpos_p1_reg >> 1, atari->gtia->w_reg[SIZEP1],
		         atari->gtia->graph_p1_reg, atari->gtia->color_pm1_reg );
		printf ( "HPOSP2=%2x  SIZEP2=%2x  GRAFP2=%2x  COLPM2=%2x\n",
		         atari->gtia->hpos_p2_reg >> 1, atari->gtia->w_reg[SIZEP2],
		         atari->gtia->graph_p2_reg, atari->gtia->color_pm2_reg );
		printf ( "HPOSP3=%2x  SIZEP3=%2x  GRAFP3=%2x  COLPM3=%2x\n",
		         atari->gtia->hpos_p3_reg >> 1, atari->gtia->w_reg[SIZEP3],
		         atari->gtia->graph_p3_reg, atari->gtia->color_pm3_reg );
		printf ( "HPOSM0=%2x  SIZEM =%2x  VDELAY=%2x  COLPF0=%2x\n",
		         atari->gtia->hpos_m0_reg >> 1, atari->gtia->w_reg[SIZEM],
		         atari->gtia->vdelay_reg, atari->gtia->color_pf0_reg );
		printf ( "HPOSM1=%2x  GRAFM =%2x  PAL   =%2x  COLPF1=%2x\n",
		         atari->gtia->hpos_m1_reg >> 1, atari->gtia->graph_m_reg,
		         atari->gtia->pal_or_ntsc, atari->gtia->color_pf1_reg );
		printf ( "HPOSM2=%2x  PRIOR =%2x  CONSOL=%2x  COLPF2=%2x\n",
		         atari->gtia->hpos_m2_reg >> 1, atari->gtia->prior_reg,
		         atari->gtia->console_reg, atari->gtia->color_pf2_reg );
		printf ( "HPOSM3=%2x  GRACTL=%2x  COLBK =%2x  COLPF3=%2x\n",
		         atari->gtia->hpos_m3_reg >> 1, atari->gtia->latch_triggers | 
		         atari->gtia->player_dma | atari->gtia->missle_dma, 
		         atari->gtia->color_bk_reg, atari->gtia->color_pf3_reg );

		printf ( "M0PF  =%2x  P0PF  =%2x  M0PL  =%2x  P0PL  =%2x\n",
		         gtia_read_register(M0PF), gtia_read_register(P0PF),
				 gtia_read_register(M0PL), gtia_read_register(P0PL) );
		printf ( "M1PF  =%2x  P1PF  =%2x  M1PL  =%2x  P1PL  =%2x\n",
		         gtia_read_register(M1PF), gtia_read_register(P1PF),
				 gtia_read_register(M1PL), gtia_read_register(P1PL) );
		printf ( "M2PF  =%2x  P2PF  =%2x  M2PL  =%2x  P2PL  =%2x\n",
		         gtia_read_register(M2PF), gtia_read_register(P2PF),
				 gtia_read_register(M2PL), gtia_read_register(P2PL) );
		printf ( "M3PF  =%2x  P3PF  =%2x  M3PL  =%2x  P3PL  =%2x\n",
		         gtia_read_register(M3PF), gtia_read_register(P3PF),
				 gtia_read_register(M3PL), gtia_read_register(P3PL) );
		printf ( "TRIG0 =%2x  TRIG1 =%2x  TRIG2 =%2x  TRIG3 =%2x\n",
		         atari->gtia->trig[0], atari->gtia->trig[1],
				 atari->gtia->trig[2], atari->gtia->trig[3] );
	}

	/***********************************************************/
	/*                     ANTIC REGISTERS                     */  
	/***********************************************************/
	else if ( strstr(arg,"antic") || strstr (arg,"ANTIC") ) {
		printf ( "ANTIC Registers\n" );
		printf ( "DMACTL=%2x  HSCROL=%2x  WSYNC =%2x  NMIEN =%2x\n",
		         atari->antic->dmactl_reg, (32-atari->antic->hscroll_reg_value)>>1,
		         0, atari->antic->nmi_dli_en | atari->antic->nmi_vbi_en | 
		         atari->antic->nmi_reset_en);
		printf ( "CHACTL=%2x  VSCROL=%2x  VCOUNT=%2x  NMIRES=%2x\n",
		         atari->antic->chactl_reg, atari->antic->vscroll_reg_value,
		         atari->antic->vcount, 0);
		printf ( "DLISTL=%2x  PMBASE=%2x  PENH  =%2x  NMIST =%2x\n", 
		         atari->antic->dlistl_reg, atari->antic->pm_base_address >> 8, 0,
		         atari->antic->nmi_status_reg );
		printf ( "DLISTH=%2x  CHBASE=%2x  PENV  =%2x            \n", 
		         atari->antic->dlisth_reg, atari->antic->ch_base_address >> 8, 0 );
	}

	/***********************************************************/
	/*                     POKEY REGISTERS                     */  
	/***********************************************************/
	else if ( strstr(arg,"pokey") || strstr (arg,"POKEY") ) {
		printf ( "POKEY Registers\n" );
		printf ( "AUDF1=%2x  AUDC1 =%2x  POT0  =%2x  POT4  =%2x\n",
		         atari->pokey->w_reg[AUDF1], atari->pokey->w_reg[AUDC1],
		         pokey_read_register(POT0), pokey_read_register(POT4) ); 
		printf ( "AUDF2=%2x  AUDC2 =%2x  POT1  =%2x  POT5  =%2x\n",
		         atari->pokey->w_reg[AUDF2], atari->pokey->w_reg[AUDC2],
		         pokey_read_register(POT1), pokey_read_register(POT5) ); 
		printf ( "AUDF3=%2x  AUDC3 =%2x  POT2  =%2x  POT6  =%2x\n", 
		         atari->pokey->w_reg[AUDF3], atari->pokey->w_reg[AUDC3],
		         pokey_read_register(POT2), pokey_read_register(POT6) ); 
		printf ( "AUDF4=%2x  AUDC4 =%2x  POT3  =%2x  POT7  =%2x\n", 
		         atari->pokey->w_reg[AUDF4], atari->pokey->w_reg[AUDC4],
		         pokey_read_register(POT3), pokey_read_register(POT7) ); 
		printf ( "IRQST=%2x  SKSTAT=%2x  SERIN =%2x  STIMER=%2x\n", 
		         atari->pokey->irq_status, atari->pokey->sk_status,
		         atari->pokey->serial_in, atari->pokey->w_reg[STIMER] ); 
		printf ( "IRQEN=%2x  SKCTL =%2x  SEROUT=%2x  ALLPOT=%2x\n", 
		         atari->pokey->irq_enable, atari->pokey->sk_ctl,
		         atari->pokey->serial_out, atari->pokey->r_reg[ALLPOT] ); 
		printf ( "KBCDE=%2x                                    \n", 
		         atari->pokey->kbcode ); 
	}

	/***********************************************************/
	/*                     DISPLAY LIST                        */  
	/***********************************************************/
	else if ( strstr(arg,"dlist") || strstr (arg,"DLIST") ) {

		printf ( "ANTIC Display List\n" );
		for ( i = 0; i < 0x240; ++i ) {
			switch ( atari->antic->dl[i] & 0xf ) {
				case 0:
					if ( atari->antic->dl[i] & 0x80 )
						printf ( "%4x : %2x DLI %d Blank Lines\n", atari->antic->dl_pc[i], 
						         atari->antic->dl[i],
						       ((atari->antic->dl[i] >> 4) & 0x7)+1 );
					else
						printf ( "%4x : %2x     %d Blank Lines\n", atari->antic->dl_pc[i], 
						         atari->antic->dl[i], 
						       ((atari->antic->dl[i] >> 4) & 0x7)+1 );
					break;
				case 1:
					if ( atari->antic->dl[i] == 0x01 )
						printf ( "%4x : %2x     JMP %x\n", atari->antic->dl_pc[i], 
						       atari->antic->dl[i], 
						       atari->antic->dl[i+1] | (atari->antic->dl[i+2] << 8) );
					else if ( atari->antic->dl[i] == 0x41 ) {
						printf ( "%4x : %2x     JVB %x\n", atari->antic->dl_pc[i], 
						       atari->antic->dl[i], 
						       atari->antic->dl[i+1] | (atari->antic->dl[i+2] << 8) );
						i = 0x240;
					}
					else
						printf ( "%4x : %2x     Illegal Instruction\n", atari->antic->dl_pc[i], atari->antic->dl[i] );
					i+=2;
					break;
				default:
					printf ( "%4x : %2x %s MODE%x %s ", atari->antic->dl_pc[i],
					           atari->antic->dl[i],
					          (atari->antic->dl[i] & 0x80) ? "DLI" : "   ",
					           atari->antic->dl[i] & 0x0f,
					          (atari->antic->dl[i] & 0x40) ? "LMS" : "   ");
					if ( atari->antic->dl[i] & 0x40 ) {
						printf ( "%x\n", atari->antic->dl[i+1] | (atari->antic->dl[i+2] << 8) );
						i+=2;
					}
					else
						printf ( "\n" );
					break;
			} /* end switch instruction type */

		} /* end for display list     */

	} /* end if printing display list */

	debug.breakpoint_flag = temp_flag;

} /* end debug_print_value */

void debug_set_breakpoint ( char *arg ) {

	int i;

	if ( strstr(arg,"mem") ) {

	}
	else if ( strstr(arg,"gtia") || strstr (arg,"GTIA") ) {
		printf ( "GTIA Registers\n" );
	}
	else if ( strstr(arg,"antic") || strstr (arg,"ANTIC") ) {
		printf ( "ANTIC Registers\n" );
	}
	else if ( strstr(arg,"dlist") || strstr (arg,"DLIST") ) {

		printf ( "ANTIC Display List\n" );
		for ( i = 0; i < 0x100; ++i ) {
			switch ( atari->antic->dl[i] & 0xf ) {
				case 0:
					break;
				case 1:
					break;
				default:
					break;
			}
		}
	}

} /* end debug_set_breakpoint */

void debug_search_for_breakpoint (e_debug_breakpoint break_type, int value ) {

	int i;

	if ( !debug.breakpoint_flag )
		return;

	for ( i = 0; i < DEBUG_MAX_BK; ++i ) {
		if ( debug.breakpoint[i] == break_type && debug.breakpoint_place[i] == value ) {
			printf ( "Breakpoint %d at %d occured\n", i+1, value );
			debug_get_input ();
			return;
		}
	}
}

int debug_is_page_in_debug (int page) {

	int i;

	if ( !debug.breakpoint_flag )
		return 0;

	for ( i = 0; i < DEBUG_MAX_BK; ++i ) {
		if ( debug.breakpoint[i] == BK_MEM && debug.breakpoint_place[i>>8] == page ) {
			return 1;
		}
	}

	return 0;
}

void debug_break_for_watch (int addr, int new_value, int old_value ) {

	int i;

	if ( !debug.breakpoint_flag )
		return;

	for ( i = 0; i < DEBUG_MAX_BK; ++i ) {
		if ( debug.breakpoint[i] == BK_WATCH && 
		     debug.breakpoint_place[i] == addr &&
			 new_value != old_value ) {
			printf ( "Value at %04x is %02x, was %02x\n", addr, new_value, old_value );
			debug_get_input ();
			return;
		}
	}
}

void debug_print_string_to_trace (char *string) {
	if ( m_cpu )
		fprintf ( m_cpu, string );
}

void debug_open_default_trace (void) {
	debug.trace_cpu_flag = 1;
	m_cpu = fopen ( "cpu.txt", "w" );
}

void debug_close_default_trace (void) {
	debug.trace_cpu_flag = 0;
	fclose ( m_cpu );
	m_cpu = 0;
}
