/*
Snem 0.1 by Tom Walker
PPU emulation
*/
#include <stdio.h>
#include "snes.h"


#define SNES_INLINE

#define uint unsigned int
#define uint16 unsigned short
#define uint32 unsigned int


int nmi,vbl,joyscan;
int nmienable;

int yirq,xirq,irqenable,irq;
int lines;


int global_pal;

/*DMA registers*/
unsigned short dmadest[8],dmasrc[8],dmalen[8];
unsigned long hdmaaddr[8],hdmaaddr2[8];
unsigned char dmabank[8],dmaibank[8],dmactrl[8],hdmastat[8],hdmadat[8];
int hdmacount[8];
unsigned char hdmaena;


/* layers */
enum
{
	SNES_BG1 = 0,
	SNES_BG2,
	SNES_BG3,
	SNES_BG4,
	SNES_OAM,
	SNES_COLOR
};

/* offset-per-tile modes */
enum
{
	SNES_OPT_NONE = 0,
	SNES_OPT_MODE2,
	SNES_OPT_MODE4,
	SNES_OPT_MODE6
};

struct SNES_PPU_STRUCT	/* once all the regs are saved in this structure, it would be better to reorganize it a bit... */
{
	struct
	{
		/* clipmasks */
		UINT8 window1_enabled, window1_invert;
		UINT8 window2_enabled, window2_invert;
		UINT8 wlog_mask;
		/* color math enabled */
		UINT8 color_math;

		UINT8 charmap;
		UINT8 tilemap;
		UINT8 tilemap_size;

		UINT8 tile_size;
		UINT8 mosaic_enabled;	// actually used only for layers 0->3!

		UINT8 main_window_enabled;
		UINT8 sub_window_enabled;
		UINT8 main_bg_enabled;
		UINT8 sub_bg_enabled;

		UINT16 hoffs;
		UINT16 voffs;
	} layer[6];	// this is for the BG1 - BG2 - BG3 - BG4 - OBJ - color layers

	struct
	{
		UINT8 address_low;
		UINT8 address_high;
		UINT8 saved_address_low;
		UINT8 saved_address_high;
		UINT16 address;
		UINT16 priority_rotation;
		UINT8 next_charmap;
		UINT8 next_size;
		UINT8 size_;
		UINT8 size[2];
		UINT32 next_name_select;
		UINT32 name_select;
		UINT8 first_sprite;
		UINT8 flip;
		UINT16 write_latch;
	} oam;

	struct
	{
		UINT16 horizontal[4];
		UINT16 vertical[4];
	} bgd_offset;

	struct
	{
		UINT16 latch_horz;
		UINT16 latch_vert;
		UINT16 current_horz;
		UINT16 current_vert;
		UINT8 last_visible_line;
		UINT8 interlace_count;
	} beam;

	struct
	{
		UINT8 repeat;
		UINT8 hflip;
		UINT8 vflip;
		INT16 matrix_a;
		INT16 matrix_b;
		INT16 matrix_c;
		INT16 matrix_d;
		INT16 origin_x;
		INT16 origin_y;
		UINT16 hor_offset;
		UINT16 ver_offset;
		UINT8 extbg;
	} mode7;

	UINT8 mosaic_size;
	UINT8 clip_to_black;
	UINT8 prevent_color_math;
	UINT8 sub_add_mode;
	UINT8 bg3_priority_bit;
	UINT8 direct_color;
	UINT8 ppu_last_scroll;		/* as per Anomie's doc and Theme Park, all scroll regs shares (but mode 7 ones) the same
								'previous' scroll value */
	UINT8 mode7_last_scroll;	/* as per Anomie's doc mode 7 scroll regs use a different value, shared with mode 7 matrix! */

	UINT8 ppu1_open_bus, ppu2_open_bus;
	UINT8 ppu1_version, ppu2_version;
	UINT8 window1_left, window1_right, window2_left, window2_right;

	UINT16 mosaic_table[16][4096];
	UINT8 clipmasks[6][SNES_SCR_WIDTH];
	UINT8 update_windows;
	UINT8 update_offsets;
	UINT8 update_oam_list;
	UINT8 mode;
	UINT8 interlace; //doubles the visible resolution
	UINT8 obj_interlace;
	UINT8 screen_brightness;
	UINT8 screen_disabled;
	UINT8 pseudo_hires;
	UINT8 color_modes;
	UINT8 stat77_flags;
};


static UINT16 cgram_address =0;	/* CGRAM address */
static UINT8  vram_read_offset=2;	/* VRAM read offset */


static UINT16 vram_fgr_high, vram_fgr_increment, vram_fgr_count, vram_fgr_mask, vram_fgr_shift, vram_read_buffer;
static const UINT16 vram_fgr_inctab[4] = { 1, 32, 128, 128 };
static const UINT16 vram_fgr_inccnts[4] = { 0, 32, 64, 128 };
static const UINT16 vram_fgr_shiftab[4] = { 0, 5, 6, 7 };

#define SNES_MAINSCREEN    0
#define SNES_SUBSCREEN     1
#define SNES_CLIP_NEVER    0
#define SNES_CLIP_IN       1
#define SNES_CLIP_OUT      2
#define SNES_CLIP_ALWAYS   3



static const UINT16 table_obj_offset[8][8] =
{
	{ (0*32),   (0*32)+32,   (0*32)+64,   (0*32)+96,   (0*32)+128,   (0*32)+160,   (0*32)+192,   (0*32)+224 },
	{ (16*32),  (16*32)+32,  (16*32)+64,  (16*32)+96,  (16*32)+128,  (16*32)+160,  (16*32)+192,  (16*32)+224 },
	{ (32*32),  (32*32)+32,  (32*32)+64,  (32*32)+96,  (32*32)+128,  (32*32)+160,  (32*32)+192,  (32*32)+224 },
	{ (48*32),  (48*32)+32,  (48*32)+64,  (48*32)+96,  (48*32)+128,  (48*32)+160,  (48*32)+192,  (48*32)+224 },
	{ (64*32),  (64*32)+32,  (64*32)+64,  (64*32)+96,  (64*32)+128,  (64*32)+160,  (64*32)+192,  (64*32)+224 },
	{ (80*32),  (80*32)+32,  (80*32)+64,  (80*32)+96,  (80*32)+128,  (80*32)+160,  (80*32)+192,  (80*32)+224 },
	{ (96*32),  (96*32)+32,  (96*32)+64,  (96*32)+96,  (96*32)+128,  (96*32)+160,  (96*32)+192,  (96*32)+224 },
	{ (112*32), (112*32)+32, (112*32)+64, (112*32)+96, (112*32)+128, (112*32)+160, (112*32)+192, (112*32)+224 }
};

struct SCANLINE
{
	int enable, clip;

	UINT16 buffer[SNES_SCR_WIDTH];
	UINT8  priority[SNES_SCR_WIDTH];
	UINT8  layer[SNES_SCR_WIDTH];
	UINT8  blend_exception[SNES_SCR_WIDTH];
};

UINT16 snes_cgram[SNES_CGRAM_SIZE];
UINT16 snes_oam[SNES_OAM_SIZE];
UINT8  snes_vram[SNES_VRAM_SIZE];
static UINT16 snes_ram[0x4000];
static struct SCANLINE scanlines[2];
struct SNES_PPU_STRUCT snes_ppu;

enum
{
	SNES_COLOR_DEPTH_2BPP = 0,
	SNES_COLOR_DEPTH_4BPP,
	SNES_COLOR_DEPTH_8BPP
};



/*****************************************
* snes_draw_blend()
*
* Routine for additive/subtractive blending
* between the main and sub screens.
*****************************************/

SNES_INLINE void snes_draw_blend( UINT16 offset, UINT16 *colour, UINT8 prevent_color_math, UINT8 black_pen_clip, int switch_screens )
{
	/* when color math is applied to subscreen pixels, the blending depends on the blending used by the previous mainscreen
	pixel, except for subscreen pixel 0 which has no previous mainscreen pixel, see comments in snes_refresh_scanline */
	if (switch_screens && offset > 0)
		offset -= 1;

	if ((black_pen_clip == SNES_CLIP_ALWAYS) ||
		(black_pen_clip == SNES_CLIP_IN && snes_ppu.clipmasks[SNES_COLOR][offset]) ||
		(black_pen_clip == SNES_CLIP_OUT && !snes_ppu.clipmasks[SNES_COLOR][offset]))
		*colour = 0; //clip to black before color math

	if (prevent_color_math == SNES_CLIP_ALWAYS) // blending mode 3 == always OFF
		return;

#ifdef SNES_LAYER_DEBUG
	if (!debug_options.transparency_disabled)
#endif /* SNES_LAYER_DEBUG */
		if ((prevent_color_math == SNES_CLIP_NEVER) ||
			(prevent_color_math == SNES_CLIP_IN  && !snes_ppu.clipmasks[SNES_COLOR][offset]) ||
			(prevent_color_math == SNES_CLIP_OUT && snes_ppu.clipmasks[SNES_COLOR][offset]))
		{
			UINT16 r, g, b;
			struct SCANLINE *subscreen;
			int clip_max = 0;	// if add then clip to 0x1f, if sub then clip to 0

#ifdef SNES_LAYER_DEBUG
			/* Toggle drawing of SNES_SUBSCREEN or SNES_MAINSCREEN */
			if (debug_options.draw_subscreen)
			{
				subscreen = switch_screens ? &scanlines[SNES_SUBSCREEN] : &scanlines[SNES_MAINSCREEN];
			}
			else
#endif /* SNES_LAYER_DEBUG */
			{
				subscreen = switch_screens ? &scanlines[SNES_MAINSCREEN] : &scanlines[SNES_SUBSCREEN];
			}

			if (snes_ppu.sub_add_mode) /* SNES_SUBSCREEN*/
			{
				if (!BIT(snes_ppu.color_modes, 7))
				{
					/* 0x00 add */
					r = (*colour & 0x1f) + (subscreen->buffer[offset] & 0x1f);
					g = ((*colour & 0x3e0) >> 5) + ((subscreen->buffer[offset] & 0x3e0) >> 5);
					b = ((*colour & 0x7c00) >> 10) + ((subscreen->buffer[offset] & 0x7c00) >> 10);
					clip_max = 1;
				}
				else
				{
					/* 0x80 sub */
					r = (*colour & 0x1f) - (subscreen->buffer[offset] & 0x1f);
					g = ((*colour & 0x3e0) >> 5) - ((subscreen->buffer[offset] & 0x3e0) >> 5);
					b = ((*colour & 0x7c00) >> 10) - ((subscreen->buffer[offset] & 0x7c00) >> 10);
					if (r > 0x1f) r = 0;
					if (g > 0x1f) g = 0;
					if (b > 0x1f) b = 0;
				}
				/* only halve if the color is not the back colour */
				if (BIT(snes_ppu.color_modes, 6) && (subscreen->buffer[offset] != snes_cgram[FIXED_COLOUR]))
				{
					r >>= 1;
					g >>= 1;
					b >>= 1;
				}
			}
			else /* Fixed colour */
			{
				if (!BIT(snes_ppu.color_modes, 7))
				{
					/* 0x00 add */
					r = (*colour & 0x1f) + (snes_cgram[FIXED_COLOUR] & 0x1f);
					g = ((*colour & 0x3e0) >> 5) + ((snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5);
					b = ((*colour & 0x7c00) >> 10) + ((snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10);
					clip_max = 1;
				}
				else
				{
					/* 0x80: sub */
					r = (*colour & 0x1f) - (snes_cgram[FIXED_COLOUR] & 0x1f);
					g = ((*colour & 0x3e0) >> 5) - ((snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5);
					b = ((*colour & 0x7c00) >> 10) - ((snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10);
					if (r > 0x1f) r = 0;
					if (g > 0x1f) g = 0;
					if (b > 0x1f) b = 0;
				}
				/* halve if necessary */
				if (BIT(snes_ppu.color_modes, 6))
				{
					r >>= 1;
					g >>= 1;
					b >>= 1;
				}
			}

			/* according to anomie's docs, after addition has been performed, division by 2 happens *before* clipping to max, hence we clip now */
			if (clip_max)
			{
				if (r > 0x1f) r = 0x1f;
				if (g > 0x1f) g = 0x1f;
				if (b > 0x1f) b = 0x1f;
			}

			*colour = ((r & 0x1f) | ((g & 0x1f) << 5) | ((b & 0x1f) << 10));
		}
}






/*****************************************
* snes_draw_tile()
*
* Draw tiles with variable bit planes
*****************************************/

SNES_INLINE void snes_draw_tile( UINT8 planes, UINT8 layer, UINT16 tileaddr, INT16 xpos, UINT8 priority, UINT8 flip, UINT8 direct_colors, UINT16 palNo, UINT8 hires )
{
	UINT8 mask, plane[8];
	UINT16 c;
	INT16 ii, jj;

	for (ii = 0; ii < planes / 2; ii++)
	{
		plane[2 * ii] = snes_vram[tileaddr + 16 * ii];
		plane[2 * ii + 1] = snes_vram[tileaddr + 16 * ii + 1];
	}

	if (flip)
		mask = 0x1;
	else
		mask = 0x80;

	for (ii = xpos; ii < (xpos + 8); ii++)
	{
		UINT8 colour = 0;
		if (flip)
		{
			for (jj = 0; jj < planes; jj++)
				colour |= plane[jj] & mask ? (1 << jj) : 0;

			mask <<= 1;
		}
		else
		{
			for (jj = 0; jj < planes; jj++)
				colour |= plane[jj] & mask ? (1 << jj) : 0;

			mask >>= 1;
		}


		if (!hires)
		{
			if (ii >= 0 && ii < (SNES_SCR_WIDTH << hires) && scanlines[SNES_MAINSCREEN].enable)
			{
				if (scanlines[SNES_MAINSCREEN].priority[ii] <= priority)
				{
					UINT8 clr = colour;

#ifdef SNES_LAYER_DEBUG
					if (!debug_options.windows_disabled)
#endif /* SNES_LAYER_DEBUG */
						/* Clip to windows */
						if (scanlines[SNES_MAINSCREEN].clip)
							clr &= snes_ppu.clipmasks[layer][ii];

					/* Only draw if we have a colour (0 == transparent) */
					if (clr)
					{
						if (direct_colors)
						{
							/* format is  0 | BBb00 | GGGg0 | RRRr0, HW confirms that the data is zero padded. */
							c = ((clr & 0x07) << 2) | ((clr & 0x38) << 4) | ((clr & 0xc0) << 7);
							c |= ((palNo & 0x04) >> 1) | ((palNo & 0x08) << 3) | ((palNo & 0x10) << 8);
						}
						else
							c = snes_cgram[(palNo + clr) % FIXED_COLOUR];

						if (snes_ppu.layer[SNES_MAINSCREEN].mosaic_enabled) // handle horizontal mosaic
						{
							int x_mos;

							//TODO: 512 modes has the h values doubled.
							for (x_mos = 0; x_mos < (snes_ppu.mosaic_size + 1) ; x_mos++)
							{
								scanlines[SNES_MAINSCREEN].buffer[ii + x_mos] = c;
								scanlines[SNES_MAINSCREEN].priority[ii + x_mos] = priority;
								scanlines[SNES_MAINSCREEN].layer[ii + x_mos] = layer;
							}

							ii += x_mos - 1;
						}
						else
						{
							scanlines[SNES_MAINSCREEN].buffer[ii] = c;
							scanlines[SNES_MAINSCREEN].priority[ii] = priority;
							scanlines[SNES_MAINSCREEN].layer[ii] = layer;
						}
					}
				}
			}

			if (ii >= 0 && ii < (SNES_SCR_WIDTH << hires) && scanlines[SNES_SUBSCREEN].enable)
			{
				if (scanlines[SNES_SUBSCREEN].priority[ii] <= priority)
				{
					UINT8 clr = colour;

#ifdef SNES_LAYER_DEBUG
					if (!debug_options.windows_disabled)
#endif /* SNES_LAYER_DEBUG */
						/* Clip to windows */
						if (scanlines[SNES_SUBSCREEN].clip)
							clr &= snes_ppu.clipmasks[layer][ii];

					/* Only draw if we have a colour (0 == transparent) */
					if (clr)
					{
						if (direct_colors)
						{
							/* format is  0 | BBb00 | GGGg0 | RRRr0, HW confirms that the data is zero padded. */
							c = ((clr & 0x07) << 2) | ((clr & 0x38) << 4) | ((clr & 0xc0) << 7);
							c |= ((palNo & 0x04) >> 1) | ((palNo & 0x08) << 3) | ((palNo & 0x10) << 8);
						}
						else
							c = snes_cgram[(palNo + clr) % FIXED_COLOUR];

						if (snes_ppu.layer[SNES_SUBSCREEN].mosaic_enabled) // handle horizontal mosaic
						{
							int x_mos;

							//TODO: 512 modes has the h values doubled.
							for (x_mos = 0; x_mos < (snes_ppu.mosaic_size + 1) ; x_mos++)
							{
								scanlines[SNES_SUBSCREEN].buffer[ii + x_mos] = c;
								scanlines[SNES_SUBSCREEN].priority[ii + x_mos] = priority;
								scanlines[SNES_SUBSCREEN].layer[ii + x_mos] = layer;
							}

							ii += x_mos - 1;
						}
						else
						{
							scanlines[SNES_SUBSCREEN].buffer[ii] = c;
							scanlines[SNES_SUBSCREEN].priority[ii] = priority;
							scanlines[SNES_SUBSCREEN].layer[ii] = layer;
						}
					}
				}
			}
		}
		else /* hires */
		{
			if (ii >= 0 && ii < (SNES_SCR_WIDTH << hires) && (ii & 1) && scanlines[SNES_MAINSCREEN].enable)
			{
				if (scanlines[SNES_MAINSCREEN].priority[ii >> 1] <= priority)
				{
					UINT8 clr = colour;

#ifdef SNES_LAYER_DEBUG
					if (!debug_options.windows_disabled)
#endif /* SNES_LAYER_DEBUG */
						/* Clip to windows */
						if (scanlines[SNES_MAINSCREEN].clip)
							clr &= snes_ppu.clipmasks[layer][ii >> 1];

					/* Only draw if we have a colour (0 == transparent) */
					if (clr)
					{
						if (direct_colors)
						{
							/* format is  0 | BBb00 | GGGg0 | RRRr0, HW confirms that the data is zero padded. */
							c = ((clr & 0x07) << 2) | ((clr & 0x38) << 4) | ((clr & 0xc0) << 7);
							c |= ((palNo & 0x04) >> 1) | ((palNo & 0x08) << 3) | ((palNo & 0x10) << 8);
						}
						else
							c = snes_cgram[(palNo + clr) % FIXED_COLOUR];

						if (snes_ppu.layer[layer].mosaic_enabled) // handle horizontal mosaic
						{
							int x_mos;

							//TODO: 512 modes has the h values doubled.
							for (x_mos = 0; x_mos < (snes_ppu.mosaic_size + 1) ; x_mos++)
							{
								scanlines[SNES_MAINSCREEN].buffer[(ii + x_mos) >> 1] = c;
								scanlines[SNES_MAINSCREEN].priority[(ii + x_mos) >> 1] = priority;
								scanlines[SNES_MAINSCREEN].layer[(ii + x_mos) >> 1] = layer;
							}
							ii += x_mos - 1;
						}
						else
						{
							scanlines[SNES_MAINSCREEN].buffer[ii >> 1] = c;
							scanlines[SNES_MAINSCREEN].priority[ii >> 1] = priority;
							scanlines[SNES_MAINSCREEN].layer[ii >> 1] = layer;
						}
					}
				}
			}

			if (ii >= 0 && ii < (SNES_SCR_WIDTH << hires) && !(ii & 1) && scanlines[SNES_SUBSCREEN].enable)
			{
				if (scanlines[SNES_SUBSCREEN].priority[ii >> 1] <= priority)
				{
					UINT8 clr = colour;

#ifdef SNES_LAYER_DEBUG
					if (!debug_options.windows_disabled)
#endif /* SNES_LAYER_DEBUG */
						/* Clip to windows */
						if (scanlines[SNES_SUBSCREEN].clip)
							clr &= snes_ppu.clipmasks[layer][ii >> 1];

					/* Only draw if we have a colour (0 == transparent) */
					if (clr)
					{
						if (direct_colors)
						{
							/* format is  0 | BBb00 | GGGg0 | RRRr0, HW confirms that the data is zero padded. */
							c = ((clr & 0x07) << 2) | ((clr & 0x38) << 4) | ((clr & 0xc0) << 7);
							c |= ((palNo & 0x04) >> 1) | ((palNo & 0x08) << 3) | ((palNo & 0x10) << 8);
						}
						else
							c = snes_cgram[(palNo + clr) % FIXED_COLOUR];

						if (snes_ppu.layer[layer].mosaic_enabled) // handle horizontal mosaic
						{
							int x_mos;

							//TODO: 512 modes has the h values doubled.
							for (x_mos = 0; x_mos < (snes_ppu.mosaic_size + 1) ; x_mos++)
							{
								scanlines[SNES_SUBSCREEN].buffer[(ii + x_mos) >> 1] = c;
								scanlines[SNES_SUBSCREEN].priority[(ii + x_mos) >> 1] = priority;
								scanlines[SNES_SUBSCREEN].layer[(ii + x_mos) >> 1] = layer;
							}
							ii += x_mos - 1;
						}
						else
						{
							scanlines[SNES_SUBSCREEN].buffer[ii >> 1] = c;
							scanlines[SNES_SUBSCREEN].priority[ii >> 1] = priority;
							scanlines[SNES_SUBSCREEN].layer[ii >> 1] = layer;
						}
					}
				}
			}
		}
	}
}

/*****************************************
* snes_draw_tile_x2()
*
* Draw 2 tiles with variable bit planes
*****************************************/

SNES_INLINE void snes_draw_tile_x2( UINT8 planes, UINT8 layer, UINT16 tileaddr, INT16 xpos, UINT8 priority, UINT8 flip, UINT8 direct_colors, UINT16 palNo, UINT8 hires )
{
	if (flip)
	{
		snes_draw_tile(planes, layer, tileaddr + (8 * planes), xpos, priority, flip, direct_colors, palNo, hires);
		snes_draw_tile(planes, layer, tileaddr, xpos + 8, priority, flip, direct_colors, palNo, hires);
	}
	else
	{
		snes_draw_tile(planes, layer, tileaddr, xpos, priority, flip, direct_colors, palNo, hires);
		snes_draw_tile(planes, layer, tileaddr + (8 * planes), xpos + 8, priority, flip, direct_colors, palNo, hires);
	}
}

/*****************************************
* snes_draw_tile_object()
*
* Draw tiles with 4 bit planes(16 colors)
* The same as snes_draw_tile_4() except
* that it takes a blend parameter.
*****************************************/

SNES_INLINE void snes_draw_tile_object( UINT16 tileaddr, INT16 xpos, UINT8 priority, UINT8 flip, UINT16 palNo, UINT8 blend )
{
	UINT8 mask, plane[4];
	UINT16 c;
	INT16 ii, jj;

	plane[0] = snes_vram[tileaddr];
	plane[1] = snes_vram[tileaddr + 1];
	plane[2] = snes_vram[tileaddr + 16];
	plane[3] = snes_vram[tileaddr + 17];

	if (flip)
		mask = 0x01;
	else
		mask = 0x80;

	for (ii = xpos; ii < (xpos + 8); ii++)
	{
		UINT8 colour = 0;
		if (flip)
		{
			for (jj = 0; jj < 4; jj++)
				colour |= plane[jj] & mask ? (1 << jj) : 0;

			mask <<= 1;
		}
		else
		{
			for (jj = 0; jj < 4; jj++)
				colour |= plane[jj] & mask ? (1 << jj) : 0;

			mask >>= 1;
		}

		if (ii >= 0 && ii < SNES_SCR_WIDTH && scanlines[SNES_MAINSCREEN].enable)
		{
			if (scanlines[SNES_MAINSCREEN].priority[ii] <= priority)
			{
				UINT8 clr = colour;

#ifdef SNES_LAYER_DEBUG
				if (!debug_options.windows_disabled)
#endif /* SNES_LAYER_DEBUG */
					/* Clip to windows */
					if (scanlines[SNES_MAINSCREEN].clip)
						clr &= snes_ppu.clipmasks[SNES_OAM][ii];

				/* Only draw if we have a colour (0 == transparent) */
				if (clr)
				{
					c = snes_cgram[(palNo + clr) % FIXED_COLOUR];

					scanlines[SNES_MAINSCREEN].buffer[ii] = c;
					scanlines[SNES_MAINSCREEN].priority[ii] = priority;
					scanlines[SNES_MAINSCREEN].layer[ii] = SNES_OAM;
					scanlines[SNES_MAINSCREEN].blend_exception[ii] = blend;
				}
			}
		}

		if (ii >= 0 && ii < SNES_SCR_WIDTH && scanlines[SNES_SUBSCREEN].enable)
		{
			if (scanlines[SNES_SUBSCREEN].priority[ii] <= priority)
			{
				UINT8 clr = colour;

#ifdef SNES_LAYER_DEBUG
				if (!debug_options.windows_disabled)
#endif /* SNES_LAYER_DEBUG */
					/* Clip to windows */
					if (scanlines[SNES_SUBSCREEN].clip)
						clr &= snes_ppu.clipmasks[SNES_OAM][ii];

				/* Only draw if we have a colour (0 == transparent) */
				if (clr)
				{
					c = snes_cgram[(palNo + clr) % FIXED_COLOUR];

					scanlines[SNES_SUBSCREEN].buffer[ii] = c;
					scanlines[SNES_SUBSCREEN].priority[ii] = priority;
					scanlines[SNES_SUBSCREEN].layer[ii] = SNES_OAM;
					scanlines[SNES_SUBSCREEN].blend_exception[ii] = blend;
				}
			}
		}
	}
}




/*********************************************
* snes_get_tmap_addr()
*
* Find the address in VRAM of the tile (x,y)
*********************************************/

SNES_INLINE UINT32 snes_get_tmap_addr( UINT8 layer, UINT8 tile_size, UINT32 base, UINT32 xpos, UINT32 ypos )
{
	UINT32 res = base;
	xpos  >>= (3 + tile_size);
	ypos  >>= (3 + tile_size);

	res += (snes_ppu.layer[layer].tilemap_size & 2) ? ((ypos & 0x20) << ((snes_ppu.layer[layer].tilemap_size & 1) ? 7 : 6)) : 0;
	/* Scroll vertically */
	res += (ypos & 0x1f) << 6;
	/* Offset horizontally */
	res += (snes_ppu.layer[layer].tilemap_size & 1) ? ((xpos & 0x20) << 6) : 0;
	/* Scroll horizontally */
	res += (xpos & 0x1f) << 1;

	return res;
}


/*********************************************
* snes_update_line()
*
* Update an entire line of tiles.
*********************************************/

SNES_INLINE void snes_update_line( UINT8 color_depth, UINT8 hires, UINT8 priority_a, UINT8 priority_b, UINT8 layer, UINT16 curline, UINT8 offset_per_tile, UINT8 direct_colors )
{
	UINT32 tmap, tile, xoff, yoff;
	UINT32 charaddr;
	UINT16 ii = 0, vflip, hflip, pal_col, pal_direct;
	INT8 yscroll;
	UINT8 xscroll;
	UINT8 priority;
	UINT32 addr;
	UINT16 tilemap;
	/* scrolling */
	UINT16 opt_bit = (layer == SNES_BG1) ? 13 : (layer == SNES_BG2) ? 14 : 0;
	UINT8 tile_size;
	/* variables depending on color_depth */
	UINT8 tile_divider = (color_depth == SNES_COLOR_DEPTH_8BPP) ? 4 : 2;
	UINT8 color_planes = 2 << color_depth;
	/* below we cheat to simplify the code: 8BPP should have 0 pal offset, not 0x100 (but we take care of this by later using pal % FIXED_COLOUR) */
	UINT8 color_shift = 2 << color_depth;

#ifdef SNES_LAYER_DEBUG
	if (debug_options.bg_disabled[layer])
		return;
#endif /* SNES_LAYER_DEBUG */

	scanlines[SNES_MAINSCREEN].enable = snes_ppu.layer[layer].main_bg_enabled;
	scanlines[SNES_SUBSCREEN].enable = snes_ppu.layer[layer].sub_bg_enabled;
	scanlines[SNES_MAINSCREEN].clip = snes_ppu.layer[layer].main_window_enabled;
	scanlines[SNES_SUBSCREEN].clip = snes_ppu.layer[layer].sub_window_enabled;

	if (!scanlines[SNES_MAINSCREEN].enable && !scanlines[SNES_SUBSCREEN].enable)
		return;

	/* Handle Mosaic effects */
	if (snes_ppu.layer[layer].mosaic_enabled)
		curline -= (curline % (snes_ppu.mosaic_size + 1));

	if ((snes_ppu.interlace == 2) && !hires)
		curline /= 2;

	/* Find the size of the tiles (8x8 or 16x16) */
	tile_size = snes_ppu.layer[layer].tile_size;

	/* Find scroll info */
	xoff = snes_ppu.layer[layer].hoffs;
	yoff = snes_ppu.layer[layer].voffs;

	xscroll = xoff & ((1 << (3 + tile_size)) - 1);

	/* Jump to base map address */
	tmap = snes_ppu.layer[layer].tilemap << 9;
	charaddr = snes_ppu.layer[layer].charmap << 13;

	while (ii < 256 + (8 << tile_size))
	{
		// determine the horizontal position (Bishojo Janshi Suchi Pai & Desert Figther have tile_size & hires == 1)
		UINT32 xpos = xoff + (ii << (tile_size * hires));
		UINT32 ypos = yoff + curline;

		if (offset_per_tile != SNES_OPT_NONE)
		{
			int opt_x = ii + (xoff & 7);
			UINT32 haddr = 0, vaddr = 0;
			UINT16 hval = 0, vval = 0;

			if (opt_x >= 8)
			{
				switch (offset_per_tile)
				{
				case SNES_OPT_MODE2:
				case SNES_OPT_MODE6:
					haddr = snes_get_tmap_addr(SNES_BG3, snes_ppu.layer[SNES_BG3].tile_size, snes_ppu.layer[SNES_BG3].tilemap << 9, (opt_x - 8) + ((snes_ppu.layer[SNES_BG3].hoffs & 0x3ff) & ~7), (snes_ppu.layer[SNES_BG3].voffs & 0x3ff));
					vaddr = snes_get_tmap_addr(SNES_BG3, snes_ppu.layer[SNES_BG3].tile_size, snes_ppu.layer[SNES_BG3].tilemap << 9, (opt_x - 8) + ((snes_ppu.layer[SNES_BG3].hoffs & 0x3ff) & ~7), (snes_ppu.layer[SNES_BG3].voffs & 0x3ff) + 8);
					hval = snes_vram[haddr] | (snes_vram[haddr + 1] << 8);
					vval = snes_vram[vaddr] | (snes_vram[vaddr + 1] << 8);
					if (BIT(hval, opt_bit))
						xpos = opt_x + (hval & ~7);
					if (BIT(vval, opt_bit))
						ypos = curline + vval;
					break;
				case SNES_OPT_MODE4:
					haddr = snes_get_tmap_addr(SNES_BG3, snes_ppu.layer[SNES_BG3].tile_size, snes_ppu.layer[SNES_BG3].tilemap << 9, (opt_x - 8) + ((snes_ppu.layer[SNES_BG3].hoffs & 0x3ff) & ~7), (snes_ppu.layer[SNES_BG3].voffs & 0x3ff));
					hval = snes_vram[haddr] | (snes_vram[haddr + 1] << 8);
					if (BIT(hval, opt_bit))
					{
						if (!BIT(hval, 15))
							xpos = opt_x + (hval & ~7);
						else
							ypos = curline + hval;
					}
					break;
				}
			}
		}

		addr = snes_get_tmap_addr(layer, tile_size, tmap, xpos, ypos);

		/*
		Tilemap format
		vhopppcc cccccccc

		v/h  = Vertical/Horizontal flip this tile.
		o    = Tile priority.
		ppp  = Tile palette. The number of entries in the palette depends on the Mode and the BG.
		cccccccccc = Tile number.
		*/
		tilemap = snes_vram[addr] | (snes_vram[addr + 1] << 8);
		vflip = BIT(tilemap, 15);
		hflip = BIT(tilemap, 14);
		priority = BIT(tilemap, 13) ? priority_b : priority_a;
		pal_direct = ((tilemap & 0x1c00) >> 8);
		tile = tilemap & 0x03ff;

		pal_col = ((pal_direct >> 2) << color_shift);

		/* Mode 0 palettes are layer specific */
		if (snes_ppu.mode == 0)
		{
			pal_col += (layer << 5);
		}

		/* figure out which line to draw */
		yscroll = ypos & ((8 << tile_size) - 1);

		if (yscroll > ((8 << tile_size) - 1))	/* scrolled into the next tile */
			yscroll -= (8 << tile_size);

		if (vflip)
		{
			if (tile_size)
			{
				if (yscroll > 7)
				{
					yscroll -= 8;
				}
				else
				{
					tile += 32 / tile_divider;
				}
			}
			yscroll = -yscroll + 7;
		}
		else
		{
			if (yscroll > 7)
			{
				tile += 32 / tile_divider;
				yscroll -= 8;
			}
		}
		yscroll <<= 1;

		/* below, only color_planes depends on color_depth */
		if (hires)	/* Hi-Res: 2bpp & 4bpp */
		{
			snes_draw_tile_x2(color_planes, layer, charaddr + (tile  * 8 * color_planes) + yscroll, (ii - xscroll) * 2, priority, hflip, direct_colors, direct_colors ? pal_direct : pal_col, hires);
			ii += 8;
		}
		else	/* tile_size = 0 */
		{
			if (tile_size)
			{
				snes_draw_tile_x2(color_planes, layer, charaddr + (tile  * 8 * color_planes) + yscroll, ii - xscroll, priority, hflip, direct_colors, direct_colors ? pal_direct : pal_col, hires);
				ii += 16;
			}
			else	/* No Hi-Res: 2bpp, 4bpp & 8bpp */
			{
				snes_draw_tile(color_planes, layer, charaddr + (tile  * 8 * color_planes) + yscroll, ii - xscroll, priority, hflip, direct_colors, direct_colors ? pal_direct : pal_col, hires);
				ii += 8;
			}
		}
	}
}


/*********************************************
* snes_update_line_mode7()
*
* Update an entire line of mode7 tiles.
*********************************************/

#define MODE7_CLIP(x) (((x) & 0x2000) ? ((x) | ~0x03ff) : ((x) & 0x03ff))

static void snes_update_line_mode7( UINT8 priority_a, UINT8 priority_b, UINT8 layer, UINT16 curline )
{
	UINT32 tiled;
	INT16 ma, mb, mc, md;
	INT32 xc, yc, tx, ty, sx, sy, hs, vs, xpos, xdir, x0, y0;
	UINT8 priority = priority_a;
	UINT8 colour = 0;
	UINT16 *mosaic_x, *mosaic_y;
	UINT16 c;

#ifdef SNES_LAYER_DEBUG
	if (debug_options.bg_disabled[layer])
		return;
#endif /* SNES_LAYER_DEBUG */

	scanlines[SNES_MAINSCREEN].enable = snes_ppu.layer[layer].main_bg_enabled;
	scanlines[SNES_SUBSCREEN].enable = snes_ppu.layer[layer].sub_bg_enabled;
	scanlines[SNES_MAINSCREEN].clip = snes_ppu.layer[layer].main_window_enabled;
	scanlines[SNES_SUBSCREEN].clip = snes_ppu.layer[layer].sub_window_enabled;

	if (!scanlines[SNES_MAINSCREEN].enable && !scanlines[SNES_SUBSCREEN].enable)
		return;

	ma = snes_ppu.mode7.matrix_a;
	mb = snes_ppu.mode7.matrix_b;
	mc = snes_ppu.mode7.matrix_c;
	md = snes_ppu.mode7.matrix_d;
	xc = snes_ppu.mode7.origin_x;
	yc = snes_ppu.mode7.origin_y;
	hs = snes_ppu.mode7.hor_offset;
	vs = snes_ppu.mode7.ver_offset;

	/* Sign extend */
	xc <<= 19;
	xc >>= 19;
	yc <<= 19;
	yc >>= 19;
	hs <<= 19;
	hs >>= 19;
	vs <<= 19;
	vs >>= 19;

	/* Vertical flip */
	if (snes_ppu.mode7.vflip)
		sy = 255 - curline;
	else
		sy = curline;

	/* Horizontal flip */
	if (snes_ppu.mode7.hflip)
	{
		xpos = 255;
		xdir = -1;
	}
	else
	{
		xpos = 0;
		xdir = 1;
	}

	/* MOSAIC - to be verified */
	if (layer == 1)	// BG2 use two different bits for horizontal and vertical mosaic
	{
		mosaic_x = snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG2].mosaic_enabled ? snes_ppu.mosaic_size : 0];
		mosaic_y = snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG1].mosaic_enabled ? snes_ppu.mosaic_size : 0];
	}
	else	// BG1 works as usual
	{
		mosaic_x =  snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG1].mosaic_enabled ? snes_ppu.mosaic_size : 0];
		mosaic_y =  snes_ppu.mosaic_table[snes_ppu.layer[SNES_BG1].mosaic_enabled ? snes_ppu.mosaic_size : 0];
	}

	/* Let's do some mode7 drawing huh? */
	/* These can be computed only once, since they do not depend on sx */
	x0 = ((ma * MODE7_CLIP(hs - xc)) & ~0x3f) + ((mb * mosaic_y[sy]) & ~0x3f) + ((mb * MODE7_CLIP(vs - yc)) & ~0x3f) + (xc << 8);
	y0 = ((mc * MODE7_CLIP(hs - xc)) & ~0x3f) + ((md * mosaic_y[sy]) & ~0x3f) + ((md * MODE7_CLIP(vs - yc)) & ~0x3f) + (yc << 8);

	for (sx = 0; sx < 256; sx++, xpos += xdir)
	{
		tx = (x0 + (ma * mosaic_x[sx])) >> 8;
		ty = (y0 + (mc * mosaic_x[sx])) >> 8;

		switch (snes_ppu.mode7.repeat)
		{
		case 0x00:	/* Repeat if outside screen area */
		case 0x01:	/* Repeat if outside screen area */
			tx &= 0x3ff;
			ty &= 0x3ff;
			tiled = snes_vram[(((tx >> 3) & 0x7f) + (((ty >> 3) & 0x7f) * 128)) * 2] << 7;
			colour = snes_vram[tiled + ((tx & 0x07) * 2) + ((ty & 0x07) * 16) + 1];
			break;
		case 0x02:	/* Single colour backdrop screen if outside screen area */
			if ((tx > 0) && (tx < 1024) && (ty > 0) && (ty < 1024))
			{
				tiled = snes_vram[(((tx >> 3) & 0x7f) + (((ty >> 3) & 0x7f) * 128)) * 2] << 7;
				colour = snes_vram[tiled + ((tx & 0x07) * 2) + ((ty & 0x07) * 16) + 1];
			}
			else
				colour = 0;
			break;
		case 0x03:	/* Character 0x00 repeat if outside screen area */
			if ((tx > 0) && (tx < 1024) && (ty > 0) && (ty < 1024))
				tiled = snes_vram[(((tx >> 3) & 0x7f) + (((ty >> 3) & 0x7f) * 128)) * 2] << 7;
			else
				tiled = 0;

			colour = snes_vram[tiled + ((tx & 0x07) * 2) + ((ty & 0x07) * 16) + 1];
			break;
		}

		/* The last bit is for priority in EXTBG mode (used only for BG2) */
		if (layer == 1)
		{
			priority = ((colour & 0x80) >> 7) ? priority_b : priority_a;
			colour &= 0x7f;
		}

		if (scanlines[SNES_MAINSCREEN].enable)
		{
			UINT8 clr = colour;

			if (scanlines[SNES_MAINSCREEN].clip)
				clr &= snes_ppu.clipmasks[layer][xpos];

			/* Draw pixel if appropriate */
			if (scanlines[SNES_MAINSCREEN].priority[xpos] <= priority && clr > 0)
			{
				/* Direct select, but only outside EXTBG! */
				if (snes_ppu.direct_color && layer == 0)
				{
					/* 0 | BB000 | GGG00 | RRR00, HW confirms that the data is zero padded. */
					c = ((clr & 0x07) << 2) | ((clr & 0x38) << 4) | ((clr & 0xc0) << 7);
				}
				else
					c = snes_cgram[clr];

				scanlines[SNES_MAINSCREEN].buffer[xpos] = c;
				scanlines[SNES_MAINSCREEN].priority[xpos] = priority;
				scanlines[SNES_MAINSCREEN].layer[xpos] = layer;
			}
		}

		if (scanlines[SNES_SUBSCREEN].enable)
		{
			UINT8 clr = colour;

			if (scanlines[SNES_SUBSCREEN].clip)
				clr &= snes_ppu.clipmasks[layer][xpos];

			/* Draw pixel if appropriate */
			if (scanlines[SNES_SUBSCREEN].priority[xpos] <= priority && clr > 0)
			{
				/* Direct select, but only outside EXTBG! */
				if (snes_ppu.direct_color && layer == 0)
				{
					/* 0 | BB000 | GGG00 | RRR00, HW confirms that the data is zero padded. */
					c = ((clr & 0x07) << 2) | ((clr & 0x38) << 4) | ((clr & 0xc0) << 7);
				}
				else
					c = snes_cgram[clr];

				scanlines[SNES_SUBSCREEN].buffer[xpos] = c;
				scanlines[SNES_SUBSCREEN].priority[xpos] = priority;
				scanlines[SNES_SUBSCREEN].layer[xpos] = layer;
			}
		}
	}
}


/*********************************************
* snes_update_objects()
*
* Update an entire line of sprites.
* FIXME: We need to support high priority bit
*********************************************/

struct OAM
{
	UINT16 tile;
	INT16 x, y;
	UINT8 size, vflip, hflip, priority_bits, pal;
	int height, width;
};

static struct OAM oam_list[SNES_SCR_WIDTH / 2];

#if 0

// FIXME: The following functions should be used to create sprite list with
// obsel data from the previous line. Unfortunately, some games are very picky
// about when you set up sprite attributes (e.g. Super Star Wars - The Empire
// Strikes Back and Super Street Fighter II) and they would freeze if we only build
// part of the sprite attributes at the right moment and part of them later, during
// scanline drawing. Hence, these functions are on hold until more OAM refactoring
// is done (soon-ish)
static void snes_update_obsel( void )
{
	snes_ppu.layer[SNES_OAM].charmap = snes_ppu.oam.next_charmap;
	snes_ppu.oam.name_select = snes_ppu.oam.next_name_select;

	if (snes_ppu.oam.size_ != snes_ppu.oam.next_size)
	{
		snes_ppu.oam.size_ = snes_ppu.oam.next_size;
		snes_ppu.update_oam_list = 1;
	}
}

static void snes_oam_list_build( void )
{
	UINT8 *oamram = (UINT8 *)snes_oam;
	INT16 oam = 0x1ff;
	UINT16 oam_extra = oam + 0x20;
	UINT16 extra = 0;
	int i;

	snes_ppu.update_oam_list = 0;		// eventually, we can optimize the code by only calling this function when there is a change in size

	for (i = 128; i > 0; i--)
	{
		if ((i % 4) == 0)
			extra = oamram[oam_extra--];

		oam_list[i].vflip = (oamram[oam] & 0x80) >> 7;
		oam_list[i].hflip = (oamram[oam] & 0x40) >> 6;
		oam_list[i].priority_bits = (oamram[oam] & 0x30) >> 4;
		oam_list[i].pal = 128 + ((oamram[oam] & 0x0e) << 3);
		oam_list[i].tile = (oamram[oam--] & 0x1) << 8;
		oam_list[i].tile |= oamram[oam--];
		oam_list[i].y = oamram[oam--] + 1;	/* We seem to need to add one here.... */
		oam_list[i].x = oamram[oam--];
		oam_list[i].size = (extra & 0x80) >> 7;
		extra <<= 1;
		oam_list[i].x |= ((extra & 0x80) << 1);
		extra <<= 1;
		oam_list[i].y *= snes_ppu.obj_interlace;

		/* Adjust if past maximum position */
		if (oam_list[i].y >= snes_ppu.beam.last_visible_line * snes_ppu.interlace)
			oam_list[i].y -= 256 * snes_ppu.interlace;
		if (oam_list[i].x > 255)
			oam_list[i].x -= 512;

		/* Determine object size */
		switch (snes_ppu.oam.next_size)
		{
		case 0:			/* 8x8 or 16x16 */
			oam_list[i].width  = oam_list[i].size ? 2 : 1;
			oam_list[i].height = oam_list[i].size ? 2 : 1;
			break;
		case 1:			/* 8x8 or 32x32 */
			oam_list[i].width  = oam_list[i].size ? 4 : 1;
			oam_list[i].height = oam_list[i].size ? 4 : 1;
			break;
		case 2:			/* 8x8 or 64x64 */
			oam_list[i].width  = oam_list[i].size ? 8 : 1;
			oam_list[i].height = oam_list[i].size ? 8 : 1;
			break;
		case 3:			/* 16x16 or 32x32 */
			oam_list[i].width  = oam_list[i].size ? 4 : 2;
			oam_list[i].height = oam_list[i].size ? 4 : 2;
			break;
		case 4:			/* 16x16 or 64x64 */
			oam_list[i].width  = oam_list[i].size ? 8 : 2;
			oam_list[i].height = oam_list[i].size ? 8 : 2;
			break;
		case 5:			/* 32x32 or 64x64 */
			oam_list[i].width  = oam_list[i].size ? 8 : 4;
			oam_list[i].height = oam_list[i].size ? 8 : 4;
			break;
		case 6:			/* undocumented: 16x32 or 32x64 */
			oam_list[i].width  = oam_list[i].size ? 4 : 2;
			oam_list[i].height = oam_list[i].size ? 8 : 4;
			if (snes_ppu.obj_interlace && !oam_list[i].size)
				oam_list[i].height = 2;
			break;
		case 7:			/* undocumented: 16x32 or 32x32 */
			oam_list[i].width  = oam_list[i].size ? 4 : 2;
			oam_list[i].height = oam_list[i].size ? 4 : 4;
			if (snes_ppu.obj_interlace && !oam_list[i].size)
				oam_list[i].height = 2;
			break;
		default:
			/* we should never enter here... */
			logerror("Object size unsupported: %d\n", snes_ppu.oam.size_);
			break;
		}
	}
}
#endif

static void snes_update_objects( UINT8 priority_tbl, UINT16 curline )
{
	INT8 xs, ys;
	UINT8 line;
	UINT8 range_over = 0, time_over = 0;
	UINT8 height, width, vflip, hflip, priority, palNo, blend;
	UINT16 tile;
	INT16 i, xpos, ypos;
	UINT32 name_sel = 0, charaddr;
	static const UINT8 table_obj_priority[10][4] = {
		{2, 5, 8, 11},	// mode 0
		{1, 3, 6, 9},	// mode 1
		{1, 3, 5, 7},	// mode 2
		{1, 3, 5, 7},	// mode 3
		{1, 3, 5, 7},	// mode 4
		{1, 3, 5, 7},	// mode 5
		{0, 2, 3, 5},	// mode 6
		{0, 2, 3, 4},	// mode 7
		{1, 3, 5, 6},	// mode 7 EXTBG
		{1, 2, 5, 8}	// mode 1 + BG3 priority bit
	};
	UINT8 *oamram = (UINT8 *)snes_oam;
	INT16 oam = 0x1ff;
	UINT16 oam_extra = oam + 0x20;
	UINT16 extra = 0;

#ifdef SNES_LAYER_DEBUG
	if (debug_options.bg_disabled[SNES_OAM])
		return;
#endif /* SNES_LAYER_DEBUG */

	scanlines[SNES_MAINSCREEN].enable = snes_ppu.layer[SNES_OAM].main_bg_enabled;
	scanlines[SNES_SUBSCREEN].enable = snes_ppu.layer[SNES_OAM].sub_bg_enabled;
	scanlines[SNES_MAINSCREEN].clip = snes_ppu.layer[SNES_OAM].main_window_enabled;
	scanlines[SNES_SUBSCREEN].clip = snes_ppu.layer[SNES_OAM].sub_window_enabled;

	if (!scanlines[SNES_MAINSCREEN].enable && !scanlines[SNES_SUBSCREEN].enable)
		return;

	curline /= snes_ppu.interlace;
	curline *= snes_ppu.obj_interlace;

	charaddr = snes_ppu.oam.next_charmap << 13;

	for (i = 128; i > 0; i--)
	{
		if ((i % 4) == 0)
			extra = oamram[oam_extra--];

		oam_list[i].vflip = (oamram[oam] & 0x80) >> 7;
		oam_list[i].hflip = (oamram[oam] & 0x40) >> 6;
		oam_list[i].priority_bits = (oamram[oam] & 0x30) >> 4;
		oam_list[i].pal = 128 + ((oamram[oam] & 0x0e) << 3);
		oam_list[i].tile = (oamram[oam--] & 0x1) << 8;
		oam_list[i].tile |= oamram[oam--];
		oam_list[i].y = oamram[oam--] + 1;	/* We seem to need to add one here.... */
		oam_list[i].x = oamram[oam--];
		oam_list[i].size = (extra & 0x80) >> 7;
		extra <<= 1;
		oam_list[i].x |= ((extra & 0x80) << 1);
		extra <<= 1;
		oam_list[i].y *= snes_ppu.obj_interlace;

		/* Adjust if past maximum position */
		if (oam_list[i].y >= snes_ppu.beam.last_visible_line * snes_ppu.interlace)
			oam_list[i].y -= 256 * snes_ppu.interlace;
		if (oam_list[i].x > 255)
			oam_list[i].x -= 512;
		tile = oam_list[i].tile;
		xpos = oam_list[i].x;
		ypos = oam_list[i].y;
		height = oam_list[i].height;
		width = oam_list[i].width;
		vflip = oam_list[i].vflip;
		hflip = oam_list[i].hflip;
		priority = table_obj_priority[priority_tbl][oam_list[i].priority_bits];
		palNo = oam_list[i].pal;

		/* Draw sprite if it intersects the current line */
		if (curline >= ypos && curline < (ypos + (snes_ppu.oam.size[oam_list[i].size] << 3)))
		{
			/* Only objects using palettes 4-7 can be transparent */
			blend = (palNo < 192) ? 1 : 0;

			/* Only objects using tiles over 255 use name select */
			name_sel = (tile < 256) ? 0 : snes_ppu.oam.next_name_select;

			ys = (curline - ypos) >> 3;
			line = (curline - ypos) % 8;
			if (vflip)
			{
				ys = snes_ppu.oam.size[oam_list[i].size] - ys - 1;
				line = (-1 * line) + 7;
			}
			line <<= 1;
			tile <<= 5;
			if (hflip)
			{
				UINT8 count = 0;
				for (xs = (snes_ppu.oam.size[oam_list[i].size] - 1); xs >= 0; xs--)
				{
					if ((xpos + (count << 3) < SNES_SCR_WIDTH))
					{
						snes_draw_tile_object(charaddr + name_sel + tile + table_obj_offset[ys][xs] + line, xpos + (count++ << 3), priority, hflip, palNo, blend);
					}
					time_over++;	/* Increase time_over. Should we stop drawing if exceeded 34 tiles? */
				}
			}
			else
			{
				for (xs = 0; xs < snes_ppu.oam.size[oam_list[i].size]; xs++)
				{
					if ((xpos + (xs << 3) < SNES_SCR_WIDTH))
					{
						snes_draw_tile_object(charaddr + name_sel + tile + table_obj_offset[ys][xs] + line, xpos + (xs << 3), priority, hflip, palNo, blend);
					}
					time_over++;	/* Increase time_over. Should we stop drawing if exceeded 34 tiles? */
				}
			}

			/* Increase range_over. Stop drawing if exceeded 32 objects and enforcing that limit is enabled */
			range_over++;
			if (range_over == 32) //&& (input_port_read(machine, "INTERNAL") & 0x01) )
			{
				/* Set the flag in STAT77 register */
				snes_ppu.stat77_flags |= 0x40;
				/* FIXME: This stops the SNESTest rom from drawing the object test properly.  Maybe we shouldn't stop drawing? */
				/* return; */
			}
		}
	}

	if (time_over >= 34)
	{
		/* Set the flag in STAT77 register */
		snes_ppu.stat77_flags |= 0x80;
	}
}


/*********************************************
* snes_update_mode_X()
*
* Update Mode X line.
*********************************************/

static void snes_update_mode_0( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[0])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_line(SNES_COLOR_DEPTH_2BPP, 0, 0, 3, SNES_BG4, curline, SNES_OPT_NONE, 0);
	snes_update_line(SNES_COLOR_DEPTH_2BPP, 0, 1, 4, SNES_BG3, curline, SNES_OPT_NONE, 0);
	snes_update_line(SNES_COLOR_DEPTH_2BPP, 0, 6, 9, SNES_BG2, curline, SNES_OPT_NONE, 0);
	snes_update_line(SNES_COLOR_DEPTH_2BPP, 0, 7, 10, SNES_BG1, curline, SNES_OPT_NONE, 0);
	snes_update_objects(0, curline);
}

static void snes_update_mode_1( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[1])
		return;
#endif /* SNES_LAYER_DEBUG */

	if (!snes_ppu.bg3_priority_bit)
	{
		snes_update_line(SNES_COLOR_DEPTH_2BPP, 0, 0, 2, SNES_BG3, curline, SNES_OPT_NONE, 0);
		snes_update_line(SNES_COLOR_DEPTH_4BPP, 0, 4, 7, SNES_BG2, curline, SNES_OPT_NONE, 0);
		snes_update_line(SNES_COLOR_DEPTH_4BPP, 0, 5, 8, SNES_BG1, curline, SNES_OPT_NONE, 0);
		snes_update_objects(1, curline);
	}
	else
	{
		snes_update_line(SNES_COLOR_DEPTH_2BPP, 0, 0, 9, SNES_BG3, curline, SNES_OPT_NONE, 0);
		snes_update_line(SNES_COLOR_DEPTH_4BPP, 0, 3, 6, SNES_BG2, curline, SNES_OPT_NONE, 0);
		snes_update_line(SNES_COLOR_DEPTH_4BPP, 0, 4, 7, SNES_BG1, curline, SNES_OPT_NONE, 0);
		snes_update_objects(9, curline);
	}
}

static void snes_update_mode_2( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[2])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_line(SNES_COLOR_DEPTH_4BPP, 0, 0, 4, SNES_BG2, curline, SNES_OPT_MODE2, 0);
	snes_update_line(SNES_COLOR_DEPTH_4BPP, 0, 2, 6, SNES_BG1, curline, SNES_OPT_MODE2, 0);
	snes_update_objects(2, curline);
}

static void snes_update_mode_3( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[3])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_line(SNES_COLOR_DEPTH_4BPP, 0, 0, 4, SNES_BG2, curline, SNES_OPT_NONE, 0);
	snes_update_line(SNES_COLOR_DEPTH_8BPP, 0, 2, 6, SNES_BG1, curline, SNES_OPT_NONE, snes_ppu.direct_color);
	snes_update_objects(3, curline);
}

static void snes_update_mode_4( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[4])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_line(SNES_COLOR_DEPTH_2BPP, 0, 0, 4, SNES_BG2, curline, SNES_OPT_MODE4, 0);
	snes_update_line(SNES_COLOR_DEPTH_8BPP, 0, 2, 6, SNES_BG1, curline, SNES_OPT_MODE4, snes_ppu.direct_color);
	snes_update_objects(4, curline);
}

static void snes_update_mode_5( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[5])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_line(SNES_COLOR_DEPTH_4BPP, 1, 2, 6, SNES_BG1, curline, SNES_OPT_NONE, 0);
	snes_update_line(SNES_COLOR_DEPTH_2BPP, 1, 0, 4, SNES_BG2, curline, SNES_OPT_NONE, 0);
	snes_update_objects(5, curline);
}

static void snes_update_mode_6( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[6])
		return;
#endif /* SNES_LAYER_DEBUG */

	snes_update_line(SNES_COLOR_DEPTH_4BPP, 1, 1, 4, SNES_BG1, curline, SNES_OPT_MODE6, 0);
	snes_update_objects(6, curline);
}

static void snes_update_mode_7( UINT16 curline )
{
#ifdef SNES_LAYER_DEBUG
	if (debug_options.mode_disabled[7])
		return;
#endif /* SNES_LAYER_DEBUG */

	if (!snes_ppu.mode7.extbg)
	{
		snes_update_line_mode7(1, 1, SNES_BG1, curline);
		snes_update_objects(7, curline);
	}
	else
	{
		snes_update_line_mode7(0, 4, SNES_BG2, curline);
		snes_update_line_mode7(2, 2, SNES_BG1, curline);
		snes_update_objects(8, curline);
	}
}



/*********************************************
* snes_draw_screens()
*
* Draw the whole screen (Mode 0 -> 7).
*********************************************/

static void snes_draw_screens( UINT16 curline )
{
	switch (snes_ppu.mode)
	{
	case 0: snes_update_mode_0(curline); break;		/* Mode 0 */
	case 1: snes_update_mode_1(curline); break;		/* Mode 1 */
	case 2: snes_update_mode_2(curline); break;		/* Mode 2 - Supports offset per tile */
	case 3: snes_update_mode_3(curline); break;		/* Mode 3 - Supports direct colour */
	case 4: snes_update_mode_4(curline); break;		/* Mode 4 - Supports offset per tile and direct colour */
	case 5: snes_update_mode_5(curline); break;		/* Mode 5 - Supports hires */
	case 6: snes_update_mode_6(curline); break;		/* Mode 6 - Supports offset per tile and hires */
	case 7: snes_update_mode_7(curline); break;		/* Mode 7 - Supports direct colour */
	}
}

/*********************************************
* snes_update_windowmasks()
*
* An example of how windows work:
* Win1: ...#####......
* Win2: ......#####...
*             IN                 OUT
* OR:   ...########...     ###........###
* AND:  ......##......     ######..######
* XOR:  ...###..###...     ###...##...###
* XNOR: ###...##...###     ...###..###...
*********************************************/

static void snes_update_windowmasks( void )
{
	UINT16 ii, jj;
	INT8 w1, w2;

	snes_ppu.update_windows = 0;		/* reset the flag */

	for (ii = 0; ii < SNES_SCR_WIDTH; ii++)
	{
		/* update bg 1, 2, 3, 4, obj & color windows */
		/* jj = layer */
		for (jj = 0; jj < 6; jj++)
		{
			snes_ppu.clipmasks[jj][ii] = 0xff;	/* let's start from un-masked */
			w1 = w2 = -1;

			if (snes_ppu.layer[jj].window1_enabled)
			{
				/* Default to mask area inside */
				if ((ii < snes_ppu.window1_left) || (ii > snes_ppu.window1_right))
					w1 = 0;
				else
					w1 = 1;

				/* If mask area is outside then swap */
				if (snes_ppu.layer[jj].window1_invert)
					w1 = !w1;
			}

			if (snes_ppu.layer[jj].window2_enabled)
			{
				if ((ii < snes_ppu.window2_left) || (ii > snes_ppu.window2_right))
					w2 = 0;
				else
					w2 = 1;
				if (snes_ppu.layer[jj].window2_invert)
					w2 = !w2;
			}

			/* mask if the appropriate expression is true */
			if (w1 >= 0 && w2 >= 0)
			{
				switch (snes_ppu.layer[jj].wlog_mask)
				{
				case 0x00:	/* OR */
					snes_ppu.clipmasks[jj][ii] = w1 | w2 ? 0x00 : 0xff;
					break;
				case 0x01:	/* AND */
					snes_ppu.clipmasks[jj][ii] = w1 & w2 ? 0x00 : 0xff;
					break;
				case 0x02:	/* XOR */
					snes_ppu.clipmasks[jj][ii] = w1 ^ w2 ? 0x00 : 0xff;
					break;
				case 0x03:	/* XNOR */
					snes_ppu.clipmasks[jj][ii] = !(w1 ^ w2) ? 0x00 : 0xff;
					break;
				}
			}
			else if (w1 >= 0)
				snes_ppu.clipmasks[jj][ii] = w1 ? 0x00 : 0xff;
			else if (w2 >= 0)
				snes_ppu.clipmasks[jj][ii] = w2 ? 0x00 : 0xff;
		}
	}
}

/*********************************************
* snes_update_offsets()
*
* Update the offsets with the latest changes.
*********************************************/

static void snes_update_offsets( void )
{
	int ii;
	for (ii = 0; ii < 4; ii++)
	{
	}
	snes_ppu.update_offsets = 0;
}

static SNES_INLINE unsigned char pal5bit(unsigned char bits)
{
	bits &= 0x1f;
	return (bits << 3);
}
/*********************************************
* snes_refresh_scanline()
*
* Redraw the current line.
*********************************************/
#define RGB_BLACK 0x00


#if 0
SNES_INLINE static unsigned int CalcCol(unsigned short nColour)
{
	int r, g, b;

	//r = pal5bit(nColour >> 0);
	//g = pal5bit(nColour >> 5);
	//b = pal5bit(nColour >> 10);

	r = (nColour & 0x7C00) >> 7;  // Red 
	r |= r >> 0;
	g = (nColour & 0x03E0) >> 2;	// Green
	g |= g >> 5;
	b = (nColour & 0x001F) << 3;	// Blue
	b |= b >> 10;

	return BurnHighCol(r, g, b, 0);
}
#endif



static void snes_refresh_scanline(UINT16 curline )
{
	UINT16 ii;
	int xpos;
	int fade;
	struct SCANLINE *scanline1, *scanline2;
	UINT16 c;
	unsigned short * dstbitmap = (unsigned short * )pBurnDraw;

	if (snes_ppu.screen_disabled) /* screen is forced blank */
		for (xpos = 0; xpos < SNES_SCR_WIDTH * 2; xpos++)
		{
			if (pBurnDraw)
			{
				dstbitmap[(curline*(nBurnPitch>>1))+xpos]= RGB_BLACK;
			}
		}
	else
	{
		/* Update clip window masks if necessary */
		if (snes_ppu.update_windows)
			snes_update_windowmasks();
		/* Update the offsets if necessary */
		if (snes_ppu.update_offsets)
			snes_update_offsets();

		/* Clear priority */
		memset(scanlines[SNES_MAINSCREEN].priority, 0, SNES_SCR_WIDTH);
		memset(scanlines[SNES_SUBSCREEN].priority, 0, SNES_SCR_WIDTH);

		/* Clear layers */
		memset(scanlines[SNES_MAINSCREEN].layer, SNES_COLOR, SNES_SCR_WIDTH);
		memset(scanlines[SNES_SUBSCREEN].layer, SNES_COLOR, SNES_SCR_WIDTH);

		/* Clear blend_exception (only used for OAM) */
		memset(scanlines[SNES_MAINSCREEN].blend_exception, 0, SNES_SCR_WIDTH);
		memset(scanlines[SNES_SUBSCREEN].blend_exception, 0, SNES_SCR_WIDTH);

		/* Draw back colour */
		for (ii = 0; ii < SNES_SCR_WIDTH; ii++)
		{
			if (snes_ppu.mode == 5 || snes_ppu.mode == 6)
				scanlines[SNES_SUBSCREEN].buffer[ii] = snes_cgram[0];
			else
				scanlines[SNES_SUBSCREEN].buffer[ii] = snes_cgram[FIXED_COLOUR];

			scanlines[SNES_MAINSCREEN].buffer[ii] = snes_cgram[0];
		}

		/* Draw screens */
		snes_draw_screens(curline);

#ifdef SNES_LAYER_DEBUG
		if (snes_dbg_video(machine, bitmap, curline))
		{
			profiler_mark_end();
			return;
		}

		/* Toggle drawing of SNES_SUBSCREEN or SNES_MAINSCREEN */
		if (debug_options.draw_subscreen)
		{
			scanline1 = &scanlines[SNES_SUBSCREEN];
			scanline2 = &scanlines[SNES_MAINSCREEN];
		}
		else
#endif /* SNES_LAYER_DEBUG */
		{
			scanline1 = &scanlines[SNES_MAINSCREEN];
			scanline2 = &scanlines[SNES_SUBSCREEN];
		}

		/* Phew! Draw the line to screen */
		fade = snes_ppu.screen_brightness;

		for (xpos = 0; xpos < SNES_SCR_WIDTH; xpos++)
		{
			int r, g, b, hires;
			hires = (snes_ppu.mode != 5 && snes_ppu.mode != 6) ? 0 : 1;
			c = scanline1->buffer[xpos];

			/* perform color math if the layer wants it (except if it's an object > 192) */
			if (!scanline1->blend_exception[xpos] && snes_ppu.layer[scanline1->layer[xpos]].color_math)
				snes_draw_blend(xpos, &c, snes_ppu.prevent_color_math, snes_ppu.clip_to_black, 0);

			r = ((c & 0x1f) * fade) >> 4;
			g = (((c & 0x3e0) >> 5) * fade) >> 4;
			b = (((c & 0x7c00) >> 10) * fade) >> 4;

			if (pBurnDraw)
			{
				dstbitmap[(curline * (nBurnPitch>>1))+ (xpos<<1) + 1] =BurnHighCol(pal5bit(r),pal5bit(g),pal5bit(b),0); //BurnHighCol(pal5bit(r),pal5bit(g),pal5bit(b),0);
			}

			/* in hires, the first pixel (of 512) is subscreen pixel, then the first mainscreen pixel follows, and so on... */
			if (!hires)
			{
				if (pBurnDraw)
				{
					dstbitmap[(curline * (nBurnPitch>>1))+(xpos<<1) + 0] =BurnHighCol(pal5bit(r),pal5bit(g),pal5bit(b),0); //BurnHighCol(pal5bit(r), pal5bit(g), pal5bit(b),0);
				}
			}
			else
			{
				c = scanline2->buffer[xpos];

				/* in hires, subscreen pixels are blended as well: for each subscreen pixel, color math is applied if
				it had been applied to the previous mainscreen pixel. What happens at subscreen pixel 0 (which has no
				previous mainscreen pixel) is undocumented. Until more info are discovered, we (arbitrarily) apply to it
				the same color math as the *next* mainscreen pixel (i.e. mainscreen pixel 0) */

				if (xpos == 0 && !scanline1->blend_exception[0] && snes_ppu.layer[scanline1->layer[0]].color_math)
					snes_draw_blend(0, &c, snes_ppu.prevent_color_math, snes_ppu.clip_to_black, 1);
				else if (xpos > 0  && !scanline1->blend_exception[xpos - 1] && snes_ppu.layer[scanline1->layer[xpos - 1]].color_math)
					snes_draw_blend(xpos, &c, snes_ppu.prevent_color_math, snes_ppu.clip_to_black, 1);


				r = ((c & 0x1f) * fade) >> 4;
				g = (((c & 0x3e0) >> 5) * fade) >> 4;
				b = (((c & 0x7c00) >> 10) * fade) >> 4;
				if (pBurnDraw)
				{
					dstbitmap[(curline * (nBurnPitch>>1))+ (xpos<<1) + 0] = BurnHighCol(pal5bit(r),pal5bit(g),pal5bit(b),0);//BurnHighCol(pal5bit(r), pal5bit(g), pal5bit(b),0);
				}
			}
		}
	}
}



static int hcount,vcount;


void initppu()
{
	
}

void resetppu()
{
	memset(snes_cgram,0x0000,SNES_CGRAM_SIZE*2);
	memset(snes_oam,0xff,SNES_OAM_SIZE*2);
	memset(snes_vram,0x55,SNES_VRAM_SIZE);
	memset(snes_ram,0x55,0x4000*2);

	snes_ppu.update_windows = 1;
	snes_ppu.beam.latch_vert = 0;
	snes_ppu.beam.latch_horz = 0;
	snes_ppu.beam.current_vert = 0;
	snes_ppu.beam.current_horz = 0;
	snes_ppu.beam.last_visible_line = 240;
	snes_ppu.mode = 0;
	snes_ppu.ppu1_version = 1;	// 5C77 chip version number, read by STAT77, only '1' is known
	snes_ppu.ppu2_version = 3;	// 5C78 chip version number, read by STAT78, only '2' & '3' encountered so far.
	cgram_address = 0;
	vram_read_offset = 2;

	/* Inititialize mosaic table */
	for (int j = 0; j < 16; j++)
	{
		for (int i = 0; i < 4096; i++)
		{
			snes_ppu.mosaic_table[j][i] = (i / (j + 1)) * (j + 1);
		}
	}
	snes_ram[VMAIN] = 0x80;
	snes_ram[STAT78] = SNES_NTSC;
	
	//snes_ram[HTIMEL] = 0xff;
	//snes_ram[HTIMEH] = 0x1;
	//snes_ram[VTIMEL] = 0xff;
	//snes_ram[VTIMEH] = 0x1;
	snes_ppu.interlace = 1;
	snes_ppu.obj_interlace = 1;
	snes_ppu.beam.current_vert = SNES_VTOTAL_NTSC;
}




#define CONTINUOUS 1
#define INDIRECT 2

void dohdma(int line)
{
	int c;
	for (c=0;c<8;c++)
	{
		if (!line)
		{
			hdmaaddr[c]=dmasrc[c];
			hdmacount[c]=0;
			//                                if (c==2)
			//                                printf("Reset HDMA %i loading from %02X%04X %i\n",c,dmabank[c],hdmaaddr[c],hdmacount[c]);
		}
		if (hdmaena&(1<<c) && hdmacount[c]!=-1)
		{
			if (hdmacount[c]<=0)
			{
				hdmacount[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
				//                                        printf("HDMA %i count now %04X at %02X%04X %02X %04X\n",c,hdmacount[c],dmabank[c],hdmaaddr[c],dmactrl[c],hdmadat[c]);
				if (!hdmacount[c])
				{
					hdmacount[c]--;
					break;
				}
				hdmastat[c]=0;
				if (hdmacount[c]&0x80)
				{
					if (hdmacount[c]!=0x80)
						hdmacount[c]&=0x7F;
					hdmastat[c]|=CONTINUOUS;
					//                                                printf("Continuous for %i lines\n",hdmacount[c]);
				}
				if (dmactrl[c]&0x40)
				{
					hdmastat[c]|=INDIRECT;
					hdmaaddr2[c]=readmemw((dmabank[c]<<16)|hdmaaddr[c]);
					//                                                printf("Indirect : %02X%04X\n",dmaibank[c],hdmaaddr2[c]);
					hdmaaddr[c]+=2;
				}
				//                                        printf("Channel %i now %02X%04X\n",c,dmabank[c],hdmaaddr[c]);
				//                                        if (c==5) printf("Channel 4 : dest %04X read from %02X %04X %04X stat %i\n",dmadest[c],dmabank[c],hdmaaddr[c],hdmaaddr2[c],hdmastat[c]);
				switch (dmactrl[c]&7)
				{
				case 1: /*Two registers*/
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+1,hdmadat[c]);
					break;
				case 2: /*One register write twice*/
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
				case 0: /*One register write once*/
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
					//                                                if (c==2) printf("Channel 2 now %02X%04X\n",dmabank[c],hdmaaddr[c]);
					break;
				case 3: /*Two registers write twice*/
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+1,hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+1,hdmadat[c]);
					break;
				case 4: /*Four registers*/
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+1,hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+2,hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+3,hdmadat[c]);
					break;
				default:
					//snemlog(L"Bad HDMA transfer mode %i %02X %i\n",dmactrl[c]&7,dmadest[c],hdmastat[c]);
					break;
				}
			}
			else if (hdmastat[c]&CONTINUOUS)
			{
				switch (dmactrl[c]&7)
				{
				case 1: /*Two registers*/
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+1,hdmadat[c]);
					break;
				case 2: /*One register write twice*/
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
				case 0: /*One register write once*/
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
					break;
				case 3: /*Two registers write twice*/
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+1,hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+1,hdmadat[c]);
					break;
				case 4: /*Four registers*/
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c],hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+1,hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+2,hdmadat[c]);
					if (hdmastat[c]&INDIRECT) hdmadat[c]=snes_readmem((dmaibank[c]<<16)|(hdmaaddr2[c]++));
					else                      hdmadat[c]=snes_readmem((dmabank[c]<<16)|(hdmaaddr[c]++));
					writeppu(dmadest[c]+3,hdmadat[c]);
					break;
				default:
					//snemlog(L"Bad HDMA2 transfer mode %i\n",dmactrl[c]&7);
					break;
				}
			}
			
		}
	}
}





void drawline(int line)
{
	snes_refresh_scanline(line);
	if (line<225) /*Process HDMA*/
		dohdma(line);
	return;
}

void dumphdma()
{
	int c;
	for (c=0;c<8;c++)
	{
		printf("HDMA %i %s - src %06X dest %04X mode %02X stat %i len %i\n",c,(hdmaena&(1<<c))?"on":"off",(dmabank[c]<<16)|dmasrc[c],dmadest[c],dmactrl[c],hdmastat[c],hdmacount[c]);
	}
}



void writeppu(unsigned short offset, unsigned char data)
{

	if (offset >= APU00 && offset < WMDATA)
	{
		writetospc(offset,data);
		return;
	}

	switch (offset)
	{

		case SLHV:
			vcount=lines;
			hcount=(1364-cycles)>>2;
			break;
		case INIDISP:	/* Initial settings for screen */
			if ((snes_ppu.screen_disabled & 0x80) && (!(data & 0x80))) //a 1->0 force blank transition causes a reset OAM address
			{
	//			memory_write_byte(space, OAMADDL, snes_ppu.oam.saved_address_low);
		//		memory_write_byte(space, OAMADDH, snes_ppu.oam.saved_address_high);
			}
			snes_ppu.screen_disabled = data & 0x80;
			snes_ppu.screen_brightness = (data & 0x0f) + 1;
			break;
		case OBSEL:		/* Object size and data area designation */
			snes_ppu.oam.next_charmap = (data & 0x03) << 1;
			snes_ppu.oam.next_name_select = (((data & 0x18) >> 3) * 0x1000) << 1;
			snes_ppu.oam.next_size = (data & 0xe0) >> 5;
			switch ((data & 0xe0) >> 5)
			{
				case 0:         /* 8 & 16 */
		               snes_ppu.oam.size[0] = 1;
		               snes_ppu.oam.size[1] = 2;
		               break;
		            case 1:         /* 8 & 32 */
		               snes_ppu.oam.size[0] = 1;
		               snes_ppu.oam.size[1] = 4;
		               break;
		            case 2:         /* 8 & 64 */
		               snes_ppu.oam.size[0] = 1;
		               snes_ppu.oam.size[1] = 8;
		               break;
		            case 3:         /* 16 & 32 */
		               snes_ppu.oam.size[0] = 2;
		               snes_ppu.oam.size[1] = 4;
		               break;
		            case 4:         /* 16 & 64 */
		               snes_ppu.oam.size[0] = 2;
		               snes_ppu.oam.size[1] = 8;
		               break;
		            case 5:         /* 32 & 64 */
		               snes_ppu.oam.size[0] = 4;
		               snes_ppu.oam.size[1] = 8;
		               break;
		            default:
		               /* Unknown size so default to 8 & 16 */
//		               logerror("Object size unsupported: %d\n", (data & 0xe0) >> 5);

		               snes_ppu.oam.size[0] = 1;
		               snes_ppu.oam.size[1] = 2;
				break;
			}
		case OAMADDL:	/* Address for accessing OAM (low) */
			snes_ppu.oam.address_low = data;
			snes_ppu.oam.saved_address_low = data;
			snes_ppu.oam.address = ((snes_ppu.oam.address_high & 0x1) << 8) + data;
			snes_ram[OAMDATA] = 0;
			break;
		case OAMADDH:	/* Address for accessing OAM (high) */
			snes_ppu.oam.address_high = data & 0x1;
			snes_ppu.oam.saved_address_high = data;
			snes_ppu.oam.address = ((data & 0x1) << 8) + snes_ppu.oam.address_low;
			snes_ppu.oam.priority_rotation = BIT(data, 7);
			snes_ram[OAMDATA] = 0;
			break;
		case OAMDATA:	/* Data for OAM write (DW) */
				{
				int oam_addr = snes_ppu.oam.address;

				if (oam_addr >= 0x100)
				{
					oam_addr &= 0x10f;
					if (!(snes_ram[OAMDATA]))
					{
						snes_oam[oam_addr] &= 0xff00;
						snes_oam[oam_addr] |= data;
					}
					else
					{
						snes_oam[oam_addr] &= 0x00ff;
						snes_oam[oam_addr] |= (data << 8);
					}
				}
				else
				{
					oam_addr &= 0x1ff;
					if (!(snes_ram[OAMDATA]))
					{
						snes_ppu.oam.write_latch = data;
					}
					else
					{
						snes_oam[oam_addr] = (data << 8) | snes_ppu.oam.write_latch;
					}
				}
				snes_ram[OAMDATA] = (snes_ram[OAMDATA] + 1) % 2;
				if (snes_ram[OAMDATA] == 0)
				{
					snes_ram[OAMDATA] = 0;
					snes_ppu.oam.address++;
					snes_ppu.oam.address_low = snes_ram[OAMADDL] = snes_ppu.oam.address & 0xff;
					snes_ppu.oam.address_high = snes_ram[OAMADDH] = (snes_ppu.oam.address >> 8) & 0x1;
				}
				return;
			}

		case BGMODE:	/* BG mode and character size settings */
			snes_ppu.mode = data & 0x07;
		//	snes_dynamic_res_change(space->machine);
			snes_ppu.bg3_priority_bit = BIT(data, 3);
			snes_ppu.layer[SNES_BG1].tile_size = BIT(data, 4);
			snes_ppu.layer[SNES_BG2].tile_size = BIT(data, 5);
			snes_ppu.layer[SNES_BG3].tile_size = BIT(data, 6);
			snes_ppu.layer[SNES_BG4].tile_size = BIT(data, 7);
			snes_ppu.update_offsets = 1;
			break;
		case MOSAIC:	/* Size and screen designation for mosaic */
			/* FIXME: We support horizontal mosaic only partially */
			snes_ppu.mosaic_size = (data & 0xf0) >> 4;
			snes_ppu.layer[SNES_BG1].mosaic_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].mosaic_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].mosaic_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].mosaic_enabled = BIT(data, 3);
			break;
		case BG1SC:		/* Address for storing SC data BG1 SC size designation */
		case BG2SC:		/* Address for storing SC data BG2 SC size designation  */
		case BG3SC:		/* Address for storing SC data BG3 SC size designation  */
		case BG4SC:		/* Address for storing SC data BG4 SC size designation  */
			snes_ppu.layer[offset - BG1SC].tilemap = data & 0xfc;
			snes_ppu.layer[offset - BG1SC].tilemap_size = data & 0x3;
			break;
		case BG12NBA:	/* Address for BG 1 and 2 character data */
			snes_ppu.layer[SNES_BG1].charmap = (data & 0x0f);
			snes_ppu.layer[SNES_BG2].charmap = (data & 0xf0) >> 4;
			break;
		case BG34NBA:	/* Address for BG 3 and 4 character data */
			snes_ppu.layer[SNES_BG3].charmap = (data & 0x0f);
			snes_ppu.layer[SNES_BG4].charmap = (data & 0xf0) >> 4;
			break;

		// Anomie says "H Current = (Byte<<8) | (Prev&~7) | ((Current>>8)&7); V Current = (Current<<8) | Prev;" and Prev is shared by all scrolls but in Mode 7!
		case BG1HOFS:	/* BG1 - horizontal scroll (DW) */
			/* In Mode 0->6 we use ppu_last_scroll as Prev */
			snes_ppu.layer[SNES_BG1].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG1].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			/* In Mode 7 we use mode7_last_scroll as Prev */
			snes_ppu.mode7.hor_offset = (data << 8) | (snes_ppu.mode7_last_scroll & ~7) | ((snes_ppu.mode7.hor_offset >> 8) & 7);
			snes_ppu.mode7_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG1VOFS:	/* BG1 - vertical scroll (DW) */
			/* In Mode 0->6 we use ppu_last_scroll as Prev */
			snes_ppu.layer[SNES_BG1].voffs = (data << 8) | snes_ppu.ppu_last_scroll;
			snes_ppu.ppu_last_scroll = data;
			/* In Mode 7 we use mode7_last_scroll as Prev */
			snes_ppu.mode7.ver_offset = (data << 8) | snes_ppu.mode7_last_scroll;
			snes_ppu.mode7_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG2HOFS:	/* BG2 - horizontal scroll (DW) */
			snes_ppu.layer[SNES_BG2].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG2].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG2VOFS:	/* BG2 - vertical scroll (DW) */
			snes_ppu.layer[SNES_BG2].voffs = (data << 8) | (snes_ppu.ppu_last_scroll);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG3HOFS:	/* BG3 - horizontal scroll (DW) */
			snes_ppu.layer[SNES_BG3].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG3].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG3VOFS:	/* BG3 - vertical scroll (DW) */
			snes_ppu.layer[SNES_BG3].voffs = (data << 8) | (snes_ppu.ppu_last_scroll);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG4HOFS:	/* BG4 - horizontal scroll (DW) */
			snes_ppu.layer[SNES_BG4].hoffs = (data << 8) | (snes_ppu.ppu_last_scroll & ~7) | ((snes_ppu.layer[SNES_BG4].hoffs >> 8) & 7);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case BG4VOFS:	/* BG4 - vertical scroll (DW) */
			snes_ppu.layer[SNES_BG4].voffs = (data << 8) | (snes_ppu.ppu_last_scroll);
			snes_ppu.ppu_last_scroll = data;
			snes_ppu.update_offsets = 1;
			return;
		case VMAIN:		/* VRAM address increment value designation */
			vram_fgr_high = (data & 0x80);
			vram_fgr_increment = vram_fgr_inctab[data & 3];

			if (data & 0xc)
			{
				int md = (data & 0xc) >> 2;

				vram_fgr_count = vram_fgr_inccnts[md];
				vram_fgr_mask = (vram_fgr_count * 8) - 1;
				vram_fgr_shift = vram_fgr_shiftab[md];
			}
			else
			{
				vram_fgr_count = 0;
			}
//          printf("VMAIN: high %x inc %x count %x mask %x shift %x\n", vram_fgr_high, vram_fgr_increment, vram_fgr_count, vram_fgr_mask, vram_fgr_shift);
			break;
		case VMADDL:	/* Address for VRAM read/write (low) */
		case VMADDH:	/* Address for VRAM read/write (high) */
			{
				UINT32 addr;

				snes_ram[offset] = data;
				addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				if (vram_fgr_count)
				{
					UINT32 rem = addr & vram_fgr_mask;
					UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) + ((rem & (vram_fgr_count - 1)) << 3);

					vram_read_buffer = snes_vram[(faddr << 1) & 0x1ffff] | snes_vram[((faddr << 1) + 1) & 0x1ffff] << 8;
				}
				else
				{
					vram_read_buffer = snes_vram[(addr << 1) & 0x1ffff] | snes_vram[((addr << 1) + 1) & 0x1ffff] << 8;
				}

			}
			break;
		case VMDATAL:	/* 2118: Data for VRAM write (low) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				if (vram_fgr_count)
				{
					UINT32 rem = addr & vram_fgr_mask;
					UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) + ((rem & (vram_fgr_count - 1)) << 3);

					snes_vram[(faddr << 1) & 0x1ffff] = data;
				}
				else
				{
					snes_vram[(addr << 1) & 0x1ffff] = data;
				}

				if (!vram_fgr_high)
				{
					addr += vram_fgr_increment;
					snes_ram[VMADDL] = addr & 0xff;
					snes_ram[VMADDH] = (addr >> 8) & 0xff;
				}
			}
			return;
		case VMDATAH:	/* 2119: Data for VRAM write (high) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				if (vram_fgr_count)
				{
					UINT32 rem = addr & vram_fgr_mask;
					UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) + ((rem & (vram_fgr_count - 1)) << 3);

					snes_vram[((faddr << 1) + 1) & 0x1ffff] = data;
				}
				else
				{
					snes_vram[((addr << 1) + 1) & 0x1ffff] = data;
				}

				if (vram_fgr_high)
				{
					addr += vram_fgr_increment;
					snes_ram[VMADDL] = addr & 0xff;
					snes_ram[VMADDH] = (addr >> 8) & 0xff;
				}
			}
			return;
		case M7SEL:		/* Mode 7 initial settings */
			snes_ppu.mode7.repeat = (data >> 6) & 3;
			snes_ppu.mode7.vflip  = BIT(data, 1);
			snes_ppu.mode7.hflip  = BIT(data, 0);
			break;
		/* As per Anomie's doc: Reg = (Current<<8) | Prev; and there is only one Prev, shared by these matrix regs and Mode 7 scroll regs */
		case M7A:		/* Mode 7 COS angle/x expansion (DW) */
			snes_ppu.mode7.matrix_a = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7B:		/* Mode 7 SIN angle/ x expansion (DW) */
			snes_ppu.mode7.matrix_b = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7C:		/* Mode 7 SIN angle/y expansion (DW) */
			snes_ppu.mode7.matrix_c = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7D:		/* Mode 7 COS angle/y expansion (DW) */
			snes_ppu.mode7.matrix_d = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7X:		/* Mode 7 x center position (DW) */
			snes_ppu.mode7.origin_x = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case M7Y:		/* Mode 7 y center position (DW) */
			snes_ppu.mode7.origin_y = snes_ppu.mode7_last_scroll + (data << 8);
			snes_ppu.mode7_last_scroll = data;
			break;
		case CGADD:		/* Initial address for colour RAM writing */
			/* CGRAM is 16-bit, but when reading/writing we treat it as
                 * 8-bit, so we need to double the address */
			cgram_address = data << 1;
			break;
		case CGDATA:	/* Data for colour RAM */
			((UINT8 *)snes_cgram)[cgram_address] = data;
			cgram_address = (cgram_address + 1) % (SNES_CGRAM_SIZE - 2);
			break;
		case W12SEL:	/* Window mask settings for BG1-2 */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_BG1].window1_invert  = BIT(data, 0);
				snes_ppu.layer[SNES_BG1].window1_enabled = BIT(data, 1);
				snes_ppu.layer[SNES_BG1].window2_invert  = BIT(data, 2);
				snes_ppu.layer[SNES_BG1].window2_enabled = BIT(data, 3);
				snes_ppu.layer[SNES_BG2].window1_invert  = BIT(data, 4);
				snes_ppu.layer[SNES_BG2].window1_enabled = BIT(data, 5);
				snes_ppu.layer[SNES_BG2].window2_invert  = BIT(data, 6);
				snes_ppu.layer[SNES_BG2].window2_enabled = BIT(data, 7);
				snes_ppu.update_windows = 1;
			}
			break;
		case W34SEL:	/* Window mask settings for BG3-4 */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_BG3].window1_invert  = BIT(data, 0);
				snes_ppu.layer[SNES_BG3].window1_enabled = BIT(data, 1);
				snes_ppu.layer[SNES_BG3].window2_invert  = BIT(data, 2);
				snes_ppu.layer[SNES_BG3].window2_enabled = BIT(data, 3);
				snes_ppu.layer[SNES_BG4].window1_invert  = BIT(data, 4);
				snes_ppu.layer[SNES_BG4].window1_enabled = BIT(data, 5);
				snes_ppu.layer[SNES_BG4].window2_invert  = BIT(data, 6);
				snes_ppu.layer[SNES_BG4].window2_enabled = BIT(data, 7);
				snes_ppu.update_windows = 1;
			}
			break;
		case WOBJSEL:	/* Window mask settings for objects */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_OAM].window1_invert  = BIT(data, 0);
				snes_ppu.layer[SNES_OAM].window1_enabled = BIT(data, 1);
				snes_ppu.layer[SNES_OAM].window2_invert  = BIT(data, 2);
				snes_ppu.layer[SNES_OAM].window2_enabled = BIT(data, 3);
				snes_ppu.layer[SNES_COLOR].window1_invert  = BIT(data, 4);
				snes_ppu.layer[SNES_COLOR].window1_enabled = BIT(data, 5);
				snes_ppu.layer[SNES_COLOR].window2_invert  = BIT(data, 6);
				snes_ppu.layer[SNES_COLOR].window2_enabled = BIT(data, 7);
				snes_ppu.update_windows = 1;
			}
			break;
		case WH0:		/* Window 1 left position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window1_left = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WH1:		/* Window 1 right position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window1_right = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WH2:		/* Window 2 left position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window2_left = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WH3:		/* Window 2 right position */
			if (data != snes_ram[offset])
			{
				snes_ppu.window2_right = data;
				snes_ppu.update_windows = 1;
			}
			break;
		case WBGLOG:	/* Window mask logic for BG's */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_BG1].wlog_mask = data & 0x03;
				snes_ppu.layer[SNES_BG2].wlog_mask = (data & 0x0c) >> 2;
				snes_ppu.layer[SNES_BG3].wlog_mask = (data & 0x30) >> 4;
				snes_ppu.layer[SNES_BG4].wlog_mask = (data & 0xc0) >> 6;
				snes_ppu.update_windows = 1;
			}
			break;
		case WOBJLOG:	/* Window mask logic for objects */
			if (data != snes_ram[offset])
			{
				snes_ppu.layer[SNES_OAM].wlog_mask = data & 0x03;
				snes_ppu.layer[SNES_COLOR].wlog_mask = (data & 0x0c) >> 2;
				snes_ppu.update_windows = 1;
			}
			break;
		case TM:		/* Main screen designation */
			snes_ppu.layer[SNES_BG1].main_bg_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].main_bg_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].main_bg_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].main_bg_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].main_bg_enabled = BIT(data, 4);
			break;
		case TS:		/* Subscreen designation */
			snes_ppu.layer[SNES_BG1].sub_bg_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].sub_bg_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].sub_bg_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].sub_bg_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].sub_bg_enabled = BIT(data, 4);
			break;
		case TMW:		/* Window mask for main screen designation */
			snes_ppu.layer[SNES_BG1].main_window_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].main_window_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].main_window_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].main_window_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].main_window_enabled = BIT(data, 4);
			break;
		case TSW:		/* Window mask for subscreen designation */
			snes_ppu.layer[SNES_BG1].sub_window_enabled = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].sub_window_enabled = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].sub_window_enabled = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].sub_window_enabled = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].sub_window_enabled = BIT(data, 4);
			break;
		case CGWSEL:	/* Initial settings for Fixed colour addition or screen addition */
			/* FIXME: We don't support direct select for modes 3 & 4 or subscreen window stuff */
			snes_ppu.clip_to_black = (data >> 6) & 0x03;
			snes_ppu.prevent_color_math = (data >> 4) & 0x03;
			snes_ppu.sub_add_mode = BIT(data, 1);
			snes_ppu.direct_color = BIT(data, 0);
#ifdef SNES_DBG_REG_W
			if ((data & 0x2) != (snes_ram[CGWSEL] & 0x2))
				mame_printf_debug( "Add/Sub Layer: %s\n", ((data & 0x2) >> 1) ? "Subscreen" : "Fixed colour" );
#endif
			break;
		case CGADSUB:	/* Addition/Subtraction designation for each screen */
			snes_ppu.color_modes = data & 0xc0;
			snes_ppu.layer[SNES_BG1].color_math = BIT(data, 0);
			snes_ppu.layer[SNES_BG2].color_math = BIT(data, 1);
			snes_ppu.layer[SNES_BG3].color_math = BIT(data, 2);
			snes_ppu.layer[SNES_BG4].color_math = BIT(data, 3);
			snes_ppu.layer[SNES_OAM].color_math = BIT(data, 4);
			snes_ppu.layer[SNES_COLOR].color_math = BIT(data, 5);
			break;
		case COLDATA:	/* Fixed colour data for fixed colour addition/subtraction */
			{
				/* Store it in the extra space we made in the CGRAM. It doesn't really go there, but it's as good a place as any. */
				UINT8 r, g, b;

				/* Get existing value. */
				r = snes_cgram[FIXED_COLOUR] & 0x1f;
				g = (snes_cgram[FIXED_COLOUR] & 0x3e0) >> 5;
				b = (snes_cgram[FIXED_COLOUR] & 0x7c00) >> 10;
				/* Set new value */
				if (data & 0x20)
					r = data & 0x1f;
				if (data & 0x40)
					g = data & 0x1f;
				if (data & 0x80)
					b = data & 0x1f;
				snes_cgram[FIXED_COLOUR] = (r | (g << 5) | (b << 10));
			} break;
		case SETINI:	/* Screen mode/video select */
			/* FIXME: We only support line count and interlace here */
			snes_ppu.interlace = (data & 0x01) ? 2 : 1;
			snes_ppu.obj_interlace = (data & 0x02) ? 2 : 1;
			snes_ppu.beam.last_visible_line = (data & 0x04) ? 240 : 225;
			snes_ppu.pseudo_hires = BIT(data, 3);
			snes_ppu.mode7.extbg = BIT(data, 6);
	//		snes_dynamic_res_change(space->machine);
#ifdef SNES_DBG_REG_W
			if ((data & 0x8) != (snes_ram[SETINI] & 0x8))
				mame_printf_debug( "Pseudo 512 mode: %s\n", (data & 0x8) ? "on" : "off" );
#endif
			break;
		case APU00: case APU01: case APU02: case APU03:

			writetospc(offset,data);
			break;

		default:
			printf("%x, %x",offset,data);
			break;
			}
}



static UINT8 read_ophct =0;
static UINT8 read_opvct =0;
static UINT8 temp=0;
unsigned char readppu(unsigned short offset)
{
//	snes_state *state = (snes_state *)space->machine->driver_data;
	UINT8 value = 0;

	// APU is mirrored from 2140 to 217f
	if (offset >= APU00 && offset < WMDATA)
	{
		return readfromspc(offset & 0x3);
//		return spc_port_out[offset & 0x3];
	}
	
	/* offset is from 0x000000 */
	switch (offset)
	{
		case OAMDATA:	/* 21xy for x=0,1,2 and y=4,5,6,8,9,a returns PPU1 open bus*/
		case BGMODE:
		case MOSAIC:
		case BG2SC:
		case BG3SC:
		case BG4SC:
		case BG4VOFS:
		case VMAIN:
		case VMADDL:
		case VMDATAL:
		case VMDATAH:
		case M7SEL:
		case W34SEL:
		case WOBJSEL:
		case WH0:
		case WH2:
		case WH3:
		case WBGLOG:
			return snes_ppu.ppu1_open_bus;

// According to BSNES, these should return snes_open_bus_r!
//      case OAMADDL:
//      case OAMADDH:
//      case VMADDH:
//      case CGADD:
//      case CGDATA:
//          return snes_ram[offset];
		case MPYL:		/* Multiplication result (low) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = c & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case MPYM:		/* Multiplication result (mid) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = (c >> 8) & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case MPYH:		/* Multiplication result (high) */
			{
				/* Perform 16bit * 8bit multiply */
				UINT32 c = snes_ppu.mode7.matrix_a * (INT8)(snes_ppu.mode7.matrix_b >> 8);
				snes_ppu.ppu1_open_bus = (c >> 16) & 0xff;
				return snes_ppu.ppu1_open_bus;
			}
		case SLHV:		/* Software latch for H/V counter */
			temp=vcount&0xFF;
			vcount>>=8;
			return temp;
		case ROAMDATA:	/* Read data from OAM (DR) */
			{
				int oam_addr = snes_ppu.oam.address;

				if (oam_addr & 0x100)
				{
					oam_addr &= 0x10f;
				}
				else
				{
					oam_addr &= 0x1ff;
				}

				snes_ppu.ppu1_open_bus = (snes_oam[oam_addr] >> (snes_ram[OAMDATA] << 3)) & 0xff;
				snes_ram[OAMDATA] = (snes_ram[OAMDATA] + 1) % 2;
				if (snes_ram[OAMDATA] == 0)
				{
					snes_ppu.oam.address++;
					snes_ppu.oam.address_low = snes_ram[OAMADDL] = snes_ppu.oam.address & 0xff;
					snes_ppu.oam.address_high = snes_ram[OAMADDH] = (snes_ppu.oam.address >> 8) & 0x1;
				}
				return snes_ppu.ppu1_open_bus;
			}
		case RVMDATAL:	/* Read data from VRAM (low) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				snes_ppu.ppu1_open_bus = vram_read_buffer & 0xff;

				if (!vram_fgr_high)
				{
					if (vram_fgr_count)
					{
						UINT32 rem = addr & vram_fgr_mask;
						UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) +
							       ((rem & (vram_fgr_count - 1)) << 3);

						vram_read_buffer = snes_vram[(faddr<<1)&0x1ffff] | snes_vram[((faddr<<1)+1) & 0x1ffff]<<8;
					}
					else
					{
						vram_read_buffer = snes_vram[(addr<<1)&0x1ffff] | snes_vram[((addr<<1)+1) & 0x1ffff]<<8;
					}

					addr += vram_fgr_increment;
					snes_ram[VMADDL] = addr&0xff;
					snes_ram[VMADDH] = (addr>>8)&0xff;
				}
			}
			return snes_ppu.ppu1_open_bus;
		case RVMDATAH:	/* Read data from VRAM (high) */
			{
				UINT32 addr = (snes_ram[VMADDH] << 8) | snes_ram[VMADDL];

				snes_ppu.ppu1_open_bus = (vram_read_buffer>>8) & 0xff;

				if (vram_fgr_high)
				{
					if (vram_fgr_count)
					{
						UINT32 rem = addr & vram_fgr_mask;
						UINT32 faddr = (addr & ~vram_fgr_mask) + (rem >> vram_fgr_shift) +
							       ((rem & (vram_fgr_count - 1)) << 3);

						vram_read_buffer = snes_vram[(faddr<<1)&0x1ffff] | snes_vram[((faddr<<1)+1) & 0x1ffff]<<8;
					}
					else
					{
						vram_read_buffer = snes_vram[(addr<<1)&0x1ffff] | snes_vram[((addr<<1)+1) & 0x1ffff]<<8;
					}

					addr += vram_fgr_increment;
					snes_ram[VMADDL] = addr&0xff;
					snes_ram[VMADDH] = (addr>>8)&0xff;
				}
			}
			return snes_ppu.ppu1_open_bus;
		case RCGDATA:	/* Read data from CGRAM */
				if (!(cgram_address & 0x01))
				{
					snes_ppu.ppu2_open_bus = ((UINT8 *)snes_cgram)[cgram_address] & 0xff;
				}
				else
				{
					snes_ppu.ppu2_open_bus &= 0x80;
					snes_ppu.ppu2_open_bus |= ((UINT8 *)snes_cgram)[cgram_address] & 0x7f;
				}

				cgram_address = (cgram_address + 1) % (SNES_CGRAM_SIZE - 2);
				return snes_ppu.ppu2_open_bus;
		case OPHCT:		/* Horizontal counter data by ext/soft latch */
			{
				if (read_ophct)
				{
					snes_ppu.ppu2_open_bus &= 0xfe;
					snes_ppu.ppu2_open_bus |= (snes_ppu.beam.latch_horz >> 8) & 0x01;
				}
				else
				{
					snes_ppu.ppu2_open_bus = snes_ppu.beam.latch_horz & 0xff;
				}
				read_ophct ^= 1;
				return snes_ppu.ppu2_open_bus;
			}
		case OPVCT:		/* Vertical counter data by ext/soft latch */
			{
				if (read_opvct)
				{
					snes_ppu.ppu2_open_bus &= 0xfe;
					snes_ppu.ppu2_open_bus |= (snes_ppu.beam.latch_vert >> 8) & 0x01;
				}
				else
				{
					snes_ppu.ppu2_open_bus = snes_ppu.beam.latch_vert & 0xff;
				}
				read_opvct ^= 1;
				return snes_ppu.ppu2_open_bus;
			}
		case STAT77:	/* PPU status flag and version number */
			value = snes_ppu.stat77_flags & 0xc0; // 0x80 & 0x40 are Time Over / Range Over Sprite flags, set by the video code
			// 0x20 - Master/slave mode select. Little is known about this bit. We always seem to read back 0 here.
			value |= (snes_ppu.ppu1_open_bus & 0x10);
			value |= (snes_ppu.ppu1_version & 0x0f);
			snes_ppu.stat77_flags = value;	// not sure if this is needed...
			snes_ppu.ppu1_open_bus = value;
			return snes_ppu.ppu1_open_bus;
		case STAT78:	/* PPU status flag and version number */
			read_ophct = 0;
			read_opvct = 0;
			value = snes_ram[offset];
			value |= (snes_ppu.ppu1_open_bus & 0x20);
			value |= (snes_ppu.ppu2_version & 0x0f);
			snes_ram[offset] = value;	// not sure if this is needed...
			snes_ppu.ppu2_open_bus = value;
			return snes_ppu.ppu2_open_bus;
		case WMDATA:	/* Data to read from WRAM */
			{
				UINT32 addr = ((snes_ram[WMADDH] & 0x1) << 16) | (snes_ram[WMADDM] << 8) | snes_ram[WMADDL];
				value = snes_ram[addr++];
				//value = memory_read_byte(space, 0x7e0000 + addr++);
				addr &= 0x1ffff;
				snes_ram[WMADDH] = (addr >> 16) & 0x1;
				snes_ram[WMADDM] = (addr >> 8) & 0xff;
				snes_ram[WMADDL] = addr & 0xff;
				return value;
			}
		case WMADDL:	/* Address to read/write to wram (low) */
		case WMADDM:	/* Address to read/write to wram (mid) */
		case WMADDH:	/* Address to read/write to wram (high) */
			return snes_ram[offset];
			}
			return 0xff;
}


void dumpvram()
{
}
