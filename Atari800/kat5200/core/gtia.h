/******************************************************************************
*
* FILENAME: gtia.h
*
* DESCRIPTION:  This contains function declartions for gtia video functions
*
* AUTHORS:  bberlin
*
* CHANGE LOG
*
*  VER     DATE     CHANGED BY   DESCRIPTION OF CHANGE
* ------  --------  ----------   ------------------------------------
* 0.1     07/18/04  bberlin      Creation
* 0.4.0   07/11/06  bberlin      Added hi-res artifact variables
* 0.5.0   08/25/06  bberlin      Added 'gtia_bottom_button' functions here
* 0.5.0   08/28/06  bberlin      Changed all color vars to unsigned in case we
*                                  want to support 32-bit pixels
******************************************************************************/
#ifndef gtia_h
#define gtia_h

#include "boom6502.h"

/*#define NEW_VIDEO_METHOD 1*/

#define HPOSP0  0x00
#define HPOSP1  0x01
#define HPOSP2  0x02
#define HPOSP3  0x03

#define HPOSM0  0x04
#define HPOSM1  0x05
#define HPOSM2  0x06
#define HPOSM3  0x07

#define SIZEP0  0x08
#define SIZEP1  0x09
#define SIZEP2  0x0a
#define SIZEP3  0x0b
#define SIZEM   0x0c

#define GRAFP0  0x0d
#define GRAFP1  0x0e
#define GRAFP2  0x0f
#define GRAFP3  0x10
#define GRAFM   0x11

/* overlap color */
#define COLOVP1  0x00
#define COLOVP2  0x01

#define COLPM0  0x12
#define COLPM1  0x13
#define COLPM2  0x14
#define COLPM3  0x15

#define COLPF0  0x16
#define COLPF1  0x17
#define COLPF2  0x18
#define COLPF3  0x19
#define COLBK   0x1a

#define M0PF    0x00
#define M1PF    0x01
#define M2PF    0x02
#define M3PF    0x03

#define P0PF    0x04
#define P1PF    0x05
#define P2PF    0x06
#define P3PF    0x07

#define M0PL    0x08
#define M1PL    0x09
#define M2PL    0x0a
#define M3PL    0x0b

#define P0PL    0x0c
#define P1PL    0x0d
#define P2PL    0x0e
#define P3PL    0x0f

#define PRIOR   0x1b
#define VDELAY  0x1c

#define PALR    0x14
#define CONSOL  0x1f

#define GRACTL  0x1d
#define HITCLR  0x1e
#define TRIG0   0x10
#define TRIG1   0x11
#define TRIG2   0x12
#define TRIG3   0x13

#define NTSC   0xf
#define PAL    0x1

/* Color PRIOR Masks */
#define BK_MASK  0x001
#define PF3_MASK 0x002
#define PF2_MASK 0x004
#define PF1_MASK 0x008
#define PF0_MASK 0x010
#define PM3_MASK 0x020
#define PM2_MASK 0x040
#define PM1_MASK 0x080
#define PM0_MASK 0x100
#define MODE2_MASK 0x2000
#define PRIOR_SHIFT 9 

#define BAK_BYTE  0
#define PF3_BYTE  4
#define PF2_BYTE  3
#define PF1_BYTE  2
#define PF0_BYTE  1

struct gtia_surface {
	UINT8 *start;
	UINT8 *current;
	int width;
	int bytes_per_pixel;
	int line_only;
};

struct gtia {
	BYTE r_reg[0x20];
	BYTE w_reg[0x20];

	BYTE pal_or_ntsc;

	/* CONSOL register                  */
	BYTE  keypad;
	BYTE pot_enable;
	BYTE console_reg;
	BYTE console_mask;
	BYTE option_pressed;
	BYTE select_pressed;
	BYTE start_pressed;
	BYTE internal_speaker;

	/* player-missle graphics registers */
	BYTE graph_p0_reg;
	BYTE graph_p1_reg;
	BYTE graph_p2_reg;
	BYTE graph_p3_reg;
	BYTE graph_m_reg;

	BYTE graph_p0_line[0x200];
	BYTE graph_p1_line[0x200];
	BYTE graph_p2_line[0x200];
	BYTE graph_p3_line[0x200];
	BYTE graph_m0_line[0x200];
	BYTE graph_m1_line[0x200];
	BYTE graph_m2_line[0x200];
	BYTE graph_m3_line[0x200];

	int hpos_p0_reg;
	int hpos_p1_reg;
	int hpos_p2_reg;
	int hpos_p3_reg;
	int hpos_m0_reg;
	int hpos_m1_reg;
	int hpos_m2_reg;
	int hpos_m3_reg;

	BYTE size_p0_reg;
	BYTE size_p1_reg;
	BYTE size_p2_reg;
	BYTE size_p3_reg;
	BYTE size_m0_reg;
	BYTE size_m1_reg;
	BYTE size_m2_reg;
	BYTE size_m3_reg;

	/* vertical delay                        */
	BYTE vdelay_reg;
	void ( *transfer_p0_ptr[2] ) ( void );
	void ( *transfer_p1_ptr[2] ) ( void );
	void ( *transfer_p2_ptr[2] ) ( void );
	void ( *transfer_p3_ptr[2] ) ( void );
	void ( *transfer_m0_ptr[2] ) ( void );
	void ( *transfer_m1_ptr[2] ) ( void );
	void ( *transfer_m2_ptr[2] ) ( void );
	void ( *transfer_m3_ptr[2] ) ( void );

	/* need backup of the actual register value  */
	BYTE color_pm0_reg;
	BYTE color_pm1_reg;
	BYTE color_pm2_reg;
	BYTE color_pm3_reg;
	BYTE color_pf0_reg;
	BYTE color_pf1_reg;
	BYTE color_pf2_reg;
	BYTE color_pf3_reg;
	BYTE color_bk_reg;

	/* Use the overlap color for players - PRIOR */
	BYTE prior_overlap_flag;

	struct gtia_surface d_surface;
	unsigned int *color_array;
	unsigned int col_blk;
	unsigned int col_bk;
	unsigned int col_pm0;
	unsigned int col_pm1;
	unsigned int col_pm2;
	unsigned int col_pm3;
	unsigned int col_pf0;
	unsigned int col_pf1;
	unsigned int col_pf2;
	unsigned int col_pf3;
	unsigned int col_ov01;
	unsigned int col_ov23;

	/* Antic Mode 2,3,f colors for lum only                    */
	unsigned int col_pm0_mode2_set;
	unsigned int col_pm1_mode2_set;
	unsigned int col_pm2_mode2_set;
	unsigned int col_pm3_mode2_set;
	unsigned int col_pm0_mode2_clear;
	unsigned int col_pm1_mode2_clear;
	unsigned int col_pm2_mode2_clear;
	unsigned int col_pm3_mode2_clear;

	unsigned int col_pf3_mode2_set;
	unsigned int col_pf3_mode2_clear;
	unsigned int col_pf1_mode2;
	unsigned int hi_res_current_table[8];
	unsigned int **hi_res_current_ptr_array;

	/* GTIA Mode 1 or 3 colors                                 */
	unsigned int *gtia_mode_col_ptr;
	void ( *gtia_set_byte_mode_2_ptr ) ( void );
	unsigned int gtia_mode_col[2][16];

	/* Priority array containing pointers to colrs       */
	/* The mask will indicate PRIOR, GTIA Mode, Overlap, Mode 2 */
	BYTE prior_reg;
	unsigned int *prior_ptr_array[0x4000];
	unsigned int prior_mask;
	int prior_overlap;

	unsigned int missle0_mask;
	unsigned int missle1_mask;
	unsigned int missle2_mask;
	unsigned int missle3_mask;
	int gtia_mode;

	/* collisions will have array that is updated on pixel write       */
	BYTE collision_p0pf[4];
	BYTE collision_p1pf[4];
	BYTE collision_p2pf[4];
	BYTE collision_p3pf[4];
	BYTE collision_p0pl[4];
	BYTE collision_p1pl[4];
	BYTE collision_p2pl[4];
	BYTE collision_p3pl[4];

	BYTE collision_m0pf[4];
	BYTE collision_m1pf[4];
	BYTE collision_m2pf[4];
	BYTE collision_m3pf[4];
	BYTE collision_m0pl[4];
	BYTE collision_m1pl[4];
	BYTE collision_m2pl[4];
	BYTE collision_m3pl[4];

	/* collision registers              */
	BYTE p0pl_reg;
	BYTE p1pl_reg;
	BYTE p2pl_reg;
	BYTE p3pl_reg;
	BYTE p0pf_reg;
	BYTE p1pf_reg;
	BYTE p2pf_reg;
	BYTE p3pf_reg;
	BYTE m0pl_reg;
	BYTE m1pl_reg;
	BYTE m2pl_reg;
	BYTE m3pl_reg;
	BYTE m0pf_reg;
	BYTE m1pf_reg;
	BYTE m2pf_reg;
	BYTE m3pf_reg;

	/* graphics control, triggers       */
	BYTE latch_triggers;
	BYTE player_dma;
	BYTE missle_dma;

	BYTE trig[4];
	BYTE trig_held_down[4];

	/* Playfield bytes set by antic     */
	BYTE pf_byte[5];

	BYTE pf_x_byte;
	int *pf_x_start_ptr;
	int  pf_x;
};

void gtia_transfer_nothing (void);
void gtia_transfer_p0 (void);
void gtia_transfer_p1 (void);
void gtia_transfer_p2 (void);
void gtia_transfer_p3 (void);
void gtia_transfer_m0 (void);
void gtia_transfer_m1 (void);
void gtia_transfer_m2 (void);
void gtia_transfer_m3 (void);

void gtia_init_prior_array ( void );

void gtia_set_blank_line (void);

void gtia_set_byte_blank (void);
void gtia_set_byte_mode_2 (void);
void gtia_set_byte_mode_2_g1g3 (void);
void gtia_set_byte_mode_2_g2 (void);
void gtia_set_byte_mode_4 (void);

void gtia_init ( int system_type );
void gtia_set_screen ( void *surface, unsigned int *color_array, int width, 
                        int bpp, int line_only );
void gtia_set_mode_2_draw_ptr ( void );
BYTE gtia_read_register ( WORD addr );
void gtia_write_register ( WORD addr, BYTE value );
BYTE gtia_read_register_debug ( WORD addr );
void gtia_write_register_debug ( WORD addr, BYTE value );
int gtia_bottom_button_down ( int player, int temp, int temp1 );
int gtia_bottom_button_up ( int player, int temp, int temp1 );
int gtia_set_switch ( int mask, int value );
void gtia_write_console ( BYTE value );
void gtia_write_collision ( WORD addr, BYTE value );
void gtia_init_scanline ( void );
void gtia_end_scanline ( void );
void gtia_draw_scanline ( void );

#endif
