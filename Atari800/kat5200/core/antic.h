/******************************************************************************
*
* FILENAME: antic.h
*
* DESCRIPTION:  This contains function declartions for antic video functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     07/18/04  bberlin      Creation
******************************************************************************/
#ifndef antic_h
#define antic_h

#include "boom6502.h"

/* ANTIC registers */
#define DMACTL 0x00
#define PL_WD  0x03 
#define M_DMA  0x04 
#define P_DMA  0x08 
#define PM_REZ 0x10
#define DL_DMA 0x20

#define CHACTL  0x01
#define OPAQUE  0x01
#define INV_VID 0x02
#define UP_VID  0x04

#define DLISTL  0x02
#define DLISTH  0x03
#define HSCROL  0x04
#define VSCROL  0x05
#define PMBASE  0x07
#define CHBASE  0x09
#define WSYNC   0x0a
#define VCOUNT  0x0b
#define PENH    0x0c
#define PENV    0x0d

#define NMIEN     0x0e
#define NMI_RESET 0x20
#define NMI_VBI   0x40
#define NMI_DLI   0x80

#define NMIRES 0x0f
#define NMIST  0x0f

#define MAX_SCAN_LINES 262
#define MAX_MODE_LINES 17
#define MAX_INSTR      0x8
#define MAX_CHANGES    36

/* Cycle Map Defines   */
#define MODE_BLANK     0
#define MODE_2_TO_5    2
#define MODE_6_7       3
#define MODE_8_9       4
#define MODE_A_B_C     5
#define MODE_D_E       6
#define MODE_F         7

#define ML_FIRST       0
#define ML_NORMAL      1
#define ML_LAST        2
#define ML_FIRST_LAST  3

struct antic {
	WORD pc;
	WORD pc_fixed;
	WORD msc;
	WORD msc_fixed;
	void ( *do_instruct[ 0x100 ] ) ( void );
	int byte_count;
	int dli_count;
	int vblank_sl_time;
	BYTE dmactl_reg;
	BYTE chactl_reg;
	BYTE dlistl_reg;
	BYTE dlisth_reg;

	WORD character_base;

	/* Cycle map stuff */
	void ( *run_6502_ptr ) ( void );
	void ( *antic_read_char_data_ptr ) ( void );
	void ( *antic_read_char_data_mode3_ptr ) ( void );
	void ( *antic_read_data_ptr ) ( void );
	void ( *antic_read_data_mode3_ptr ) ( void );
	void ( *antic_map_read_and_draw_ptr ) ( void );
	void ( *antic_map_draw_ptr ) ( void );
	void ( *antic_blank_ptr ) ( void );
	void ( *antic_set_dli_ptr ) ( void );
	void ( *antic_read_missle_ptr ) ( void );
	void ( *antic_read_player_ptr ) ( void );
	void ( *antic_read_even_missle_ptr ) ( void );
	void ( *antic_read_even_player_ptr ) ( void );
	void ( *antic_read_odd_missle_ptr ) ( void );
	void ( *antic_read_odd_player_ptr ) ( void );
	void ( *antic_do_dl_instr_ptr ) ( void );
	void ( *antic_init_scanline_ptr ) ( void );
	void ( *antic_end_scanline_ptr ) ( void );
	void ( *antic_draw_blank_line_ptr ) ( void );

	/* Char Ctrl ptrs */
	void ( **antic_charctl_ptr ) ( void );
	void ( *antic_inv_video_ptr ) ( void );
	void ( *antic_inv_blank_ptr ) ( void );
	void ( *antic_blank_char_ptr ) ( void );
	void ( *antic_normal_char_ptr ) ( void );
	void ( *antic_bit1_char_ptr ) ( void );
	void ( *antic_char_pf0_ptr ) ( void );
	void ( *antic_char_pf1_ptr ) ( void );
	void ( *antic_char_pf2_ptr ) ( void );
	void ( *antic_char_pf3_ptr ) ( void );
	void ( *antic_char_ptr[48] ) ( void );

	/* CHBASE stuff   */
	WORD ch_base_address;
	WORD ch_base_20_mode;
	WORD ch_base_40_mode;
	WORD current_ch_base;
	WORD current_ch_base_16_line;
	WORD *ch_base_ptr;

	/* PM ptrs        */
	int *pm_counter_ptr;

	/* Cycle Map stuff*/
	void ( **dl_map[5][MAX_INSTR][MAX_MODE_LINES][MAX_CHANGES] ) ( void );
	int dl_cycles[5][MAX_INSTR][MAX_MODE_LINES][MAX_CHANGES];
	int *ml_dl_cycles[MAX_MODE_LINES];
	void ( ***ml_map[MAX_MODE_LINES] ) ( void );
	int *ml_cycles[MAX_MODE_LINES];
	void ( ***sl_map ) ( void );
	int *sl_cycles;

	int mode_line;
	int scan_change;

	unsigned int  pixel_start[5];
	unsigned int normal_pixel_start;
	unsigned int hscroll_pixel_start;
	WORD pl_width;
	BYTE dl_current_byte;
	WORD dl_scanline;
	WORD vcount;

	BYTE dl_byte_number;
	BYTE dl[0x240];
	BYTE dl_pc[0x240];

	WORD dl_mode_line;
	BYTE dl_mode_line_opp;
	BYTE *dl_mode_line_ptr;

	BYTE chactl_inverse_video;
	BYTE chactl_blanked;

	/* NMI interupt stuff                     */
	BYTE nmi_status_reg;
	BYTE nmi_dli_en;
	BYTE nmi_vbi_en;
	BYTE nmi_reset_en;

	/* Player, missle stuff                   */
	BYTE pm_resolution;
	WORD pm_base_address;
	WORD pm_size;
	WORD m_start;
	WORD p_0_start;
	WORD p_1_start;
	WORD p_2_start;
	WORD p_3_start;

	/* VSCROLL,HSCROLL stuff                  */
	BYTE hscroll_reg_value;
	BYTE vscroll_reg_value;
	BYTE vscroll_taking_place;
	int num_lines;
	BYTE width_array;

	BYTE screen_cache[192];
	BYTE screen_offset[192];

	BYTE ( *read_mem[ 0x100 ] ) ( WORD addr );
};

void antic_init ( void );
BYTE antic_read_register ( WORD addr );
void antic_write_register ( WORD addr, BYTE value );
BYTE antic_read_register_debug ( WORD addr );
void antic_write_register_debug ( WORD addr, BYTE value );
void antic_create_cycle_maps ( void );

void antic_read_missle ( void );
void antic_read_char_data_mode_2 ( void );
void antic_read_char_data_mode_3 ( void );
void antic_read_data_mode_2 ( void );
void antic_read_data_mode_3 ( void );

void antic_read_byte_mode_2 ( void );
void antic_read_byte_inv_mode_2 ( void );
void antic_read_byte_blank_mode_2 ( void );
void antic_read_byte_inv_blank_mode_2 ( void );

void antic_read_byte_mode_4_bit_0 ( void );
void antic_read_byte_mode_4_bit_1 ( void );

void antic_read_byte_mode_6_pf0 ( void );
void antic_read_byte_mode_6_pf1 ( void );
void antic_read_byte_mode_6_pf2 ( void );
void antic_read_byte_mode_6_pf3 ( void );

void antic_send_byte_mode_8 ( void );
void antic_read_and_draw_mode_8 ( void );
void antic_draw_mode_8 ( void );
void antic_send_byte_mode_9 ( void );
void antic_send_byte_mode_a ( void );
void antic_send_byte_mode_b_c ( void );
void antic_send_byte_mode_d_e ( void );
void antic_send_byte_mode_f ( void );
void antic_send_byte_mode_blank ( void );

void assign_rest_of_line ( int mode_type, int mode_line, int start ); 
int assign_start_of_line ( int mode_type, int mode_line );

void antic_do_nothing ( void );
void antic_do_nothing_to_dli ( void );
void antic_do_display_list_instruction ( void );
void antic_set_reset ( void );
void antic_set_dli ( void );
void antic_read_player ( void );
void antic_run_6502 ( void );
void antic_run_6502_to_dli ( void );

/* display list instructions */
void antic_blank_line ( void );
void antic_blank_line_dli ( void );
void antic_jump ( void );
void antic_jvb ( void );
void antic_lms ( void );
void antic_graphics_mode_2 ( void );
void antic_graphics_mode_3 ( void );
void antic_graphics_mode_4 ( void );
void antic_graphics_mode_5 ( void );
void antic_graphics_mode_6 ( void );
void antic_graphics_mode_7 ( void );
void antic_graphics_mode_8 ( void );
void antic_graphics_mode_9 ( void );
void antic_graphics_mode_a ( void );
void antic_graphics_mode_b ( void );
void antic_graphics_mode_c ( void );
void antic_graphics_mode_d ( void );
void antic_graphics_mode_e ( void );
void antic_graphics_mode_f ( void );
void antic_none ( void );

void antic_no_dma ( void );
#endif
