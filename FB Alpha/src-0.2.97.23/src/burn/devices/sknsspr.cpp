/* Super Kaneko Nova System Sprites

   "CG24173 6186" & "CG24143 4181" (always used as a pair?)

  - used by suprnova.c
            galpani3.c
            jchan.c

  - ToDo:
    Get rid of sprite position kludges
    Fix zooming precision/rounding (most noticeable on jchan backgrounds)

	Ported from MAME 0.144u4
*/

#include "tiles_generic.h"

#define cliprect_min_y 0
#define cliprect_max_y (nScreenHeight-1)
#define cliprect_min_x 0
#define cliprect_max_x (nScreenWidth - 1)

#define SUPRNOVA_DECODE_BUFFER_SIZE	0x2000

static int sprite_kludge_x, sprite_kludge_y;
static UINT8 decodebuffer[0x2000];

static int skns_rle_decode ( int romoffset, int size, UINT8*gfx_source, INT32 gfx_length )
{
	UINT8 *src = gfx_source;
	INT32 srcsize = gfx_length;
	UINT8 *dst = decodebuffer;
	int decodeoffset = 0;

	while(size>0) {
		UINT8 code = src[(romoffset++)%srcsize];
		size -= (code & 0x7f) + 1;
		if(code & 0x80) { /* (code & 0x7f) normal values will follow */
			code &= 0x7f;
			do {
				dst[(decodeoffset++)%SUPRNOVA_DECODE_BUFFER_SIZE] = src[(romoffset++)%srcsize];
				code--;
			} while(code != 0xff);
		} else {  /* repeat next value (code & 0x7f) times */
			UINT8 val = src[(romoffset++)%srcsize];
			do {
				dst[(decodeoffset++)%SUPRNOVA_DECODE_BUFFER_SIZE] = val;
				code--;
			} while(code != 0xff);
		}
	}
	return &src[romoffset%srcsize]-gfx_source;
}

void skns_sprite_kludge(int x, int y)
{
	sprite_kludge_x = x;
	sprite_kludge_y = y;
}

/* Zooming blitter, zoom is by way of both source and destination offsets */
/* We are working in .6 fixed point if you hadn't guessed */

#define z_decls(step)				\
	UINT16 zxs = 0x40-(zx_m>>2);			\
	UINT16 zxd = 0x40-(zx_s>>2);		\
	UINT16 zys = 0x40-(zy_m>>2);			\
	UINT16 zyd = 0x40-(zy_s>>2);		\
	int xs, ys, xd, yd, old, old2;		\
	int step_spr = step;				\
	int bxs = 0, bys = 0;				\
	INT32 clip_min_x = cliprect_min_x<<6;		\
	INT32 clip_max_x = (cliprect_max_x+1)<<6;	\
	INT32 clip_min_y = cliprect_min_y<<6;		\
	INT32 clip_max_y = (cliprect_max_y+1)<<6;	\
	sx <<= 6;					\
	sy <<= 6;					\
	x <<= 6;					\
	y <<= 6;

#define z_clamp_x_min()			\
	if(x < clip_min_x) {					\
		do {					\
			bxs += zxs;				\
			x += zxd;					\
		} while(x < clip_min_x);				\
	}

#define z_clamp_x_max()			\
	if(x > clip_max_x) {				\
		do {					\
			bxs += zxs;				\
			x -= zxd;					\
		} while(x > clip_max_x);				\
	}

#define z_clamp_y_min()			\
	if(y < clip_min_y) {					\
		do {					\
			bys += zys;				\
			y += zyd;					\
		} while(y < clip_min_y);				\
		src += (bys>>6)*step_spr;			\
	}

#define z_clamp_y_max()			\
	if(y > clip_max_y) {				\
		do {					\
			bys += zys;				\
			y -= zyd;					\
		} while(y > clip_max_y);				\
		src += (bys>>6)*step_spr;			\
	}

#define z_loop_x()			\
	xs = bxs;					\
	xd = x;					\
	while(xs < sx && xd <= clip_max_x)

#define z_loop_x_flip()			\
	xs = bxs;					\
	xd = x;					\
	while(xs < sx && xd >= clip_min_x)

#define z_loop_y()			\
	ys = bys;					\
	yd = y;					\
	while(ys < sy && yd <= clip_max_y)

#define z_loop_y_flip()			\
	ys = bys;					\
	yd = y;					\
	while(ys < sy && yd >= clip_min_y)

#define z_draw_pixel()				\
	UINT8 val = src[xs >> 6];			\
	if(val)					\
		if ((yd>>6) < nScreenHeight && (xd>>6) < nScreenWidth)	\
			bitmap[(yd>>6) * nScreenWidth + (xd>>6)] = val + colour;

#define z_x_dst(op)			\
	old = xd;					\
	do {						\
		xs += zxs;					\
		xd op zxd;					\
	} while(!((xd^old) & ~0x3f));

#define z_y_dst(op)			\
	old = yd;					\
	old2 = ys;					\
	do {						\
		ys += zys;					\
		yd op zyd;					\
	} while(!((yd^old) & ~0x3f));			\
	while((ys^old2) & ~0x3f) {			\
		src += step_spr;				\
		old2 += 0x40;				\
	}

static void blit_nf_z(UINT16 *bitmap, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx_m, UINT16 zx_s, UINT16 zy_m, UINT16 zy_s, int colour)
{
	z_decls(sx);
	z_clamp_x_min();
	z_clamp_y_min();
	z_loop_y() {
		z_loop_x() {
			z_draw_pixel();
			z_x_dst(+=);
		}
		z_y_dst(+=);
	}
}

static void blit_fy_z(UINT16 *bitmap, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx_m, UINT16 zx_s, UINT16 zy_m, UINT16 zy_s, int colour)
{
	z_decls(sx);
	z_clamp_x_min();
	z_clamp_y_max();
	z_loop_y_flip() {
		z_loop_x() {
			z_draw_pixel();
			z_x_dst(+=);
		}
		z_y_dst(-=);
	}
}

static void blit_fx_z(UINT16 *bitmap, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx_m, UINT16 zx_s, UINT16 zy_m, UINT16 zy_s, int colour)
{
	z_decls(sx);
	z_clamp_x_max();
	z_clamp_y_min();
	z_loop_y() {
		z_loop_x_flip() {
			z_draw_pixel();
			z_x_dst(-=);
		}
		z_y_dst(+=);
	}
}

static void blit_fxy_z(UINT16 *bitmap, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx_m, UINT16 zx_s, UINT16 zy_m, UINT16 zy_s, int colour)
{
	z_decls(sx);
	z_clamp_x_max();
	z_clamp_y_max();
	z_loop_y_flip() {
		z_loop_x_flip() {
			z_draw_pixel();
			z_x_dst(-=);
		}
		z_y_dst(-=);
	}
}

static void (*const blit_z[4])(UINT16 *bitmap, const UINT8 *src, int x, int y, int sx, int sy, UINT16 zx_m, UINT16 zx_s, UINT16 zy_m, UINT16 zy_s, int colour) = {
	blit_nf_z,
	blit_fy_z,
	blit_fx_z,
	blit_fxy_z,
};

// disable_priority is a hack to make jchan drawing a bit quicker (rather than moving the sprites around different bitmaps and adding colors
void skns_draw_sprites(UINT16 *bitmap, UINT32* spriteram_source, INT32 spriteram_size, UINT8* gfx_source, INT32 gfx_length, UINT32* sprite_regs, INT32 disable_priority)
{
	/*- SPR RAM Format -**

      16 bytes per sprite

	0x00  --ss --SS  z--- ----  jjjg g-ff  ppcc cccc

      s = y size
      S = x size
      j = joint
      g = group sprite is part of (if groups are enabled)
      f = flip
      p = priority
      c = palette

	0x04  ---- -aaa  aaaa aaaa  aaaa aaaa  aaaa aaaa

      a = ROM address of sprite data

	0x08  ZZZZ ZZ--  zzzz zz--  xxxx xxxx  xx-- ----

      Z = horizontal zoom table
      z = horizontal zoom subtable
      x = x position

	0x0C  ZZZZ ZZ--  zzzz zz--  yyyy yyyy  yy-- ----

      Z = vertical zoom table
      z = vertical zoom subtable
      x = y position

	**- End of Comments -*/

	UINT32 *source = spriteram_source;
	UINT32 *finish = source + spriteram_size/4;

	int group_x_offset[4];
	int group_y_offset[4];
	int group_enable;
	int group_number;
	int sprite_flip;
	int sprite_x_scroll;
	int sprite_y_scroll;
	int disabled = sprite_regs[0x04/4] & 0x08; // RWR1
	int xsize,ysize, size, xpos=0,ypos=0, pri=0, romoffset, colour=0, xflip,yflip, joint;
	int sx,sy;
	int endromoffs=0, gfxlen;
	int grow;
	UINT16 zoomx_m, zoomx_s, zoomy_m, zoomy_s;

	if ((!disabled)){

		group_enable    = (BURN_ENDIAN_SWAP_INT32(sprite_regs[0x00/4]) & 0x0040) >> 6; // RWR0

		/* Sengekis uses global flip */
		sprite_flip = (BURN_ENDIAN_SWAP_INT32(sprite_regs[0x04/4]) & 0x03); // RWR1

		sprite_y_scroll = ((BURN_ENDIAN_SWAP_INT32(sprite_regs[0x08/4]) & 0x7fc0) >> 6); // RWR2
		sprite_x_scroll = ((BURN_ENDIAN_SWAP_INT32(sprite_regs[0x10/4]) & 0x7fc0) >> 6); // RWR4
		if (sprite_y_scroll&0x100) sprite_y_scroll -= 0x200; // Signed
		if (sprite_x_scroll&0x100) sprite_x_scroll -= 0x200; // Signed

		group_x_offset[0] = (BURN_ENDIAN_SWAP_INT32(sprite_regs[0x18/4]) & 0xffc0) >> 6; // RWR6
		group_y_offset[0] = (BURN_ENDIAN_SWAP_INT32(sprite_regs[0x1c/4]) & 0xffc0) >> 6; // RWR7
		if (group_x_offset[0]&0x200) group_x_offset[0] -= 0x400; // Signed
		if (group_y_offset[0]&0x200) group_y_offset[0] -= 0x400; // Signed

		group_x_offset[1] = (BURN_ENDIAN_SWAP_INT32(sprite_regs[0x20/4]) & 0xffc0) >> 6; // RWR8
		group_y_offset[1] = (BURN_ENDIAN_SWAP_INT32(sprite_regs[0x24/4]) & 0xffc0) >> 6; // RWR9
		if (group_x_offset[1]&0x200) group_x_offset[1] -= 0x400; // Signed
		if (group_y_offset[1]&0x200) group_y_offset[1] -= 0x400; // Signed

		group_x_offset[2] = (BURN_ENDIAN_SWAP_INT32(sprite_regs[0x28/4]) & 0xffc0) >> 6; // RWR10
		group_y_offset[2] = (BURN_ENDIAN_SWAP_INT32(sprite_regs[0x2c/4]) & 0xffc0) >> 6; // RWR11
		if (group_x_offset[2]&0x200) group_x_offset[2] -= 0x400; // Signed
		if (group_y_offset[2]&0x200) group_y_offset[2] -= 0x400; // Signed

		group_x_offset[3] = (BURN_ENDIAN_SWAP_INT32(sprite_regs[0x30/4]) & 0xffc0) >> 6; // RWR12
		group_y_offset[3] = (BURN_ENDIAN_SWAP_INT32(sprite_regs[0x34/4]) & 0xffc0) >> 6; // RWR13
		if (group_x_offset[3]&0x200) group_x_offset[3] -= 0x400; // Signed
		if (group_y_offset[3]&0x200) group_y_offset[3] -= 0x400; // Signed

		/* Seems that sprites are consistently off by a fixed no. of pixels in different games
           (Patterns emerge through Manufacturer/Date/Orientation) */
		sprite_x_scroll += sprite_kludge_x;
		sprite_y_scroll += sprite_kludge_y;


		gfxlen = gfx_length;
		while( source<finish )
		{
			xflip = (BURN_ENDIAN_SWAP_INT32(source[0]) & 0x00000200) >> 9;
			yflip = (BURN_ENDIAN_SWAP_INT32(source[0]) & 0x00000100) >> 8;

			ysize = (BURN_ENDIAN_SWAP_INT32(source[0]) & 0x30000000) >> 28;
			xsize = (BURN_ENDIAN_SWAP_INT32(source[0]) & 0x03000000) >> 24;
			xsize ++;
			ysize ++;

			xsize *= 16;
			ysize *= 16;

			size = xsize * ysize;

			joint = (BURN_ENDIAN_SWAP_INT32(source[0]) & 0x0000e000) >> 13;

			if (!(joint & 1))
			{
				xpos =  (BURN_ENDIAN_SWAP_INT32(source[2]) & 0x0000ffc0) >> 6;
				ypos =  (BURN_ENDIAN_SWAP_INT32(source[3]) & 0x0000ffc0) >> 6;

				xpos += sprite_x_scroll; // Global offset
				ypos += sprite_y_scroll;

				if (group_enable)
				{
					group_number = (BURN_ENDIAN_SWAP_INT32(source[0]) & 0x00001800) >> 11;

					/* the group positioning doesn't seem to be working as i'd expect,
					if I apply the x position the cursor on galpani4 ends up moving
					from the correct position to too far right, also the y offset
					seems to cause the position to be off by one in galpans2 even if
					it fixes the position in galpani4?

					even if I take into account the global sprite scroll registers
					it isn't right

					global offset kludged using game specific offset -pjp */

					xpos += group_x_offset[group_number];
					ypos += group_y_offset[group_number];
				}
			}
			else
			{
				xpos +=  (BURN_ENDIAN_SWAP_INT32(source[2]) & 0x0000ffc0) >> 6;
				ypos +=  (BURN_ENDIAN_SWAP_INT32(source[3]) & 0x0000ffc0) >> 6;
			}

			if (xpos > 0x1ff) xpos -= 0x400;
			if (ypos > 0x1ff) ypos -= 0x400;

			/* Local sprite offset (for taking flip into account and drawing offset) */
			sx = xpos;
			sy = ypos;

			/* Global Sprite Flip (sengekis) */
			if (sprite_flip&2)
			{
				xflip ^= 1;
				sx = nScreenWidth - sx;
			}
			if (sprite_flip&1)
			{
				yflip ^= 1;
				sy = nScreenHeight - sy;
			}

			/* Palette linking */
			if (!(joint & 2))
			{
				colour = (BURN_ENDIAN_SWAP_INT32(source[0]) & 0x0000003f) >> 0;
			}

			/* Priority and Tile linking */
			if (!(joint & 4))
			{
				romoffset = (BURN_ENDIAN_SWAP_INT32(source[1]) & 0x07ffffff) >> 0;
				pri = (BURN_ENDIAN_SWAP_INT32(source[0]) & 0x000000c0) >> 6;
			} else {
				romoffset = endromoffs;
			}

			grow = (BURN_ENDIAN_SWAP_INT32(source[0])>>23) & 1;

			if (!grow)
			{
				zoomx_m = (BURN_ENDIAN_SWAP_INT32(source[2]) >> 24)&0x00fc;
				zoomx_s = (BURN_ENDIAN_SWAP_INT32(source[2]) >> 16)&0x00fc;
				zoomy_m = (BURN_ENDIAN_SWAP_INT32(source[3]) >> 24)&0x00fc;
				zoomy_s = (BURN_ENDIAN_SWAP_INT32(source[3]) >> 16)&0x00fc;
			}
			else
			{
				// sengekis uses this on sprites which are shrinking as they head towards the ground
				// it's also used on the input test of Gals Panic S2
				//
				// it appears to offer a higher precision 'shrink' mode (although I'm not entirely
				//  convinced this implementation is correct because we simply end up ignoring
				//  part of the data)
				zoomx_m = 0;
				zoomx_s = (BURN_ENDIAN_SWAP_INT32(source[2]) >> 24)&0x00fc;
				zoomy_m = 0;
				zoomy_s = (BURN_ENDIAN_SWAP_INT32(source[3]) >> 24)&0x00fc;


			}

			romoffset &= gfxlen-1;

			endromoffs = skns_rle_decode ( romoffset, size, gfx_source, gfx_length );

			// in Cyvern

			//  train in tunnel pri = 0x00
			//  nothing?         = 0x01
			//  players etc. pri = 0x02
			//  pickups etc. pri = 0x03

			{
				int NewColour = (colour<<8);
				if (disable_priority) {
					NewColour += disable_priority; // jchan hack
				} else {
					NewColour += (pri << 14);
				}

				if(zoomx_m || zoomx_s || zoomy_m || zoomy_s)
				{
					blit_z[ (xflip<<1) | yflip ](bitmap, decodebuffer, sx, sy, xsize, ysize, zoomx_m, zoomx_s, zoomy_m, zoomy_s, NewColour);
				}
				else
				{
					if (!xflip && !yflip) {
						int xx,yy;

						for (xx = 0; xx<xsize; xx++)
						{
							if ((sx+xx < (cliprect_max_x+1)) && (sx+xx >= cliprect_min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+yy < (cliprect_max_y+1)) && (sy+yy >= cliprect_min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											bitmap[(sy+yy) * nScreenWidth + (sx+xx)] = pix+ NewColour; // change later
									}
								}
							}
						}
					} else if (!xflip && yflip) {
						int xx,yy;
						sy -= ysize;

						for (xx = 0; xx<xsize; xx++)
						{
							if ((sx+xx < (cliprect_max_x+1)) && (sx+xx >= cliprect_min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+(ysize-1-yy) < (cliprect_max_y+1)) && (sy+(ysize-1-yy) >= cliprect_min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											bitmap[(sy+(ysize-1-yy)) * nScreenWidth + (sx+xx)] = pix+ NewColour; // change later
									}
								}
							}
						}
					} else if (xflip && !yflip) {
						int xx,yy;
						sx -= xsize;

						for (xx = 0; xx<xsize; xx++)
						{
							if ( (sx+(xsize-1-xx) < (cliprect_max_x+1)) && (sx+(xsize-1-xx) >= cliprect_min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+yy < (cliprect_max_y+1)) && (sy+yy >= cliprect_min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											bitmap[(sy+yy) * nScreenWidth + (sx+(xsize-1-xx))] = pix+ NewColour; // change later
									}
								}
							}
						}
					} else if (xflip && yflip) {
						int xx,yy;
						sx -= xsize;
						sy -= ysize;

						for (xx = 0; xx<xsize; xx++)
						{
							if ((sx+(xsize-1-xx) < (cliprect_max_x+1)) && (sx+(xsize-1-xx) >= cliprect_min_x))
							{
								for (yy = 0; yy<ysize; yy++)
								{
									if ((sy+(ysize-1-yy) < (cliprect_max_y+1)) && (sy+(ysize-1-yy) >= cliprect_min_y))
									{
										int pix;
										pix = decodebuffer[xsize*yy+xx];
										if (pix)
											bitmap[(sy+(ysize-1-yy)) * nScreenWidth + (sx+(xsize-1-xx))] = pix+ NewColour; // change later
									}
								}
							}
						}
					}
				}
			}

			source+=4;
		}
	}
}

