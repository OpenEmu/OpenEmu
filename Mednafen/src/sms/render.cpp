/*
    render.c --
    Display rendering.
*/

#include "shared.h"

static void remap_8_to_32(int line);

uint8 sms_cram_expand_table[4];
uint8 gg_cram_expand_table[16];

/* Background drawing function */
void (*render_bg)(int line) = NULL;
void (*render_obj)(int line) = NULL;

/* Pointer to output buffer */
uint8 *linebuf;

/* Internal buffer for drawing non 8-bit displays */
uint8 internal_buffer[0x100];

/* Precalculated pixel table */
uint32 pixel[PALETTE_SIZE];

/* Dirty pattern info */
uint8 bg_name_dirty[0x200];     /* 1= This pattern is dirty */
uint16 bg_name_list[0x200];     /* List of modified pattern indices */
uint16 bg_list_index;           /* # of modified patterns in list */
uint8 bg_pattern_cache[0x20000];/* Cached and flipped patterns */

/* Pixel look-up table */
uint8 lut[0x10000];

/* Bitplane to packed pixel LUT */
uint32 bp_lut[0x10000];


void render_shutdown(void)
{
}

/* Initialize the rendering data */
void render_init(void)
{
    int i, j;
    int bx, sx, b, s, bp, bf, sf, c;

    make_tms_tables();

    /* Generate 64k of data for the look up table */
    for(bx = 0; bx < 0x100; bx++)
    {
        for(sx = 0; sx < 0x100; sx++)
        {
            /* Background pixel */
            b  = (bx & 0x0F);

            /* Background priority */
            bp = (bx & 0x20) ? 1 : 0;

            /* Full background pixel + priority + sprite marker */
            bf = (bx & 0x7F);

            /* Sprite pixel */
            s  = (sx & 0x0F);

            /* Full sprite pixel, w/ palette and marker bits added */
            sf = (sx & 0x0F) | 0x10 | 0x40;

            /* Overwriting a sprite pixel ? */
            if(bx & 0x40)
            {
                /* Return the input */
                c = bf;
            }
            else
            {
                /* Work out priority and transparency for both pixels */
                if(bp)
                {
                    /* Underlying pixel is high priority */
                    if(b)
                    {
                        c = bf | 0x40;
                    }
                    else
                    {
                        
                        if(s)
                        {
                            c = sf;
                        }
                        else
                        {
                            c = bf;
                        }
                    }
                }
                else
                {
                    /* Underlying pixel is low priority */
                    if(s)
                    {
                        c = sf;
                    }
                    else
                    {
                        c = bf;
                    }
                }
            }

            /* Store result */
            lut[(bx << 8) | (sx)] = c;
        }
    }

    /* Make bitplane to pixel lookup table */
    for(i = 0; i < 0x100; i++)
    for(j = 0; j < 0x100; j++)
    {
        int x;
        uint32 out = 0;
        for(x = 0; x < 8; x++)
        {
            out |= (j & (0x80 >> x)) ? (uint32)(8 << (x << 2)) : 0;
            out |= (i & (0x80 >> x)) ? (uint32)(4 << (x << 2)) : 0;
        }
#if LSB_FIRST
        bp_lut[(j << 8) | (i)] = out;
#else
        bp_lut[(i << 8) | (j)] = out;
#endif
    }

    for(i = 0; i < 4; i++)
    {
        sms_cram_expand_table[i] = i * 85;
    }

    for(i = 0; i < 16; i++)
    {
        gg_cram_expand_table[i] = i * 17;
    }

    render_reset();

}


/* Reset the rendering data */
void render_reset(void)
{
    int i;

    /* Clear display bitmap */
    memset(bitmap.data, 0, bitmap.pitch * bitmap.height);

    /* Clear palette */
    for(i = 0; i < PALETTE_SIZE; i++)
    {
        palette_sync(i, 1);
    }

    /* Invalidate pattern cache */
    memset(bg_name_dirty, 0, sizeof(bg_name_dirty));
    memset(bg_name_list, 0, sizeof(bg_name_list));
    bg_list_index = 0;
    memset(bg_pattern_cache, 0, sizeof(bg_pattern_cache));

    /* Pick render routine */
    render_bg = render_bg_sms;
    render_obj = render_obj_sms;
}


/* Draw a line of the display */
void render_line(int line, int skip)
{
    int meow_line = 0xFFFF;
    //	printf("%02x\n", vdp.extended);
    switch(vdp.extended)
    {
     case 0: if(line < (192 + 24))
	      meow_line = 24 + line;
	     else if(line >= (vdp.lines_per_frame - 24))
	      meow_line = line - (vdp.lines_per_frame - 24);
	     break;

     case 1: if(line < (224 + 8))
	      meow_line = 8 + line;
	     else if(line >= (vdp.lines_per_frame - 8))
	      meow_line = line - (vdp.lines_per_frame - 8);

	     break;
     case 2: if(line < 240)
	      meow_line = line;
	     break;
    }

    /* Ensure we're within the viewport range */
    if(line >= vdp.height)
    {
	if(meow_line < 240)
	{
         memset(linebuf, BACKDROP_COLOR, 256);
	 remap_8_to_32(meow_line);
	}
        return;
    }

    /* Point to current line in output buffer */
    linebuf = internal_buffer;

    /* Update pattern cache */
    update_bg_pattern_cache();

    /* Blank line (full width) */
    if(!(vdp.reg[1] & 0x40))
    {
        memset(linebuf, BACKDROP_COLOR, 256);
    }
    else
    {
        /* Draw background */
        if(render_bg != NULL)
            render_bg(line);

        /* Draw sprites */
        if(render_obj != NULL)
            render_obj(line);

        /* Blank leftmost column of display */
        if(vdp.reg[0] & 0x20)
        {
            memset(linebuf, BACKDROP_COLOR, 8);
        }
    }

    remap_8_to_32(meow_line);
}


/* Draw the Master System background */
void render_bg_sms(int line)
{
    int locked = 0;
    int yscroll_mask = (vdp.extended) ? 256 : 224;
    int v_line = (line + vdp.reg[9]) % yscroll_mask;
    int v_row  = (v_line & 7) << 3;
    int hscroll = ((vdp.reg[0] & 0x40) && (line < 0x10)) ? 0 : (0x100 - vdp.reg[8]);
    int column = 0;
    uint16 attr;
    uint8 *nt = &vdp.vram[vdp.ntab + ((v_line >> 3) << 6)];
    int nt_scroll = (hscroll >> 3);
    int shift = (hscroll & 7);
    uint8 atex_mask;

    uint8 *cache_ptr;
    uint8 *linebuf_ptr = &linebuf[0 - shift];

    /* Draw first column (clipped) */
    if(shift)
    {
        int x;

        for(x = shift; x < 8; x++)
            linebuf[(0 - shift) + (x)] = 0;

        column++;
    }

    /* Draw a line of the background */
    for(; column < 32; column++)
    {
        /* Stop vertical scrolling for leftmost eight columns */
        if((vdp.reg[0] & 0x80) && (!locked) && (column >= 24))
        {
            locked = 1;
            v_row = (line & 7) << 3;
            nt = &vdp.vram[((vdp.reg[2] << 10) & 0x3800) + ((line >> 3) << 6)];
        }

        /* Get name table attribute word */
        attr = nt[(((column + nt_scroll) & 0x1F) << 1) | 0] | (nt[(((column + nt_scroll) & 0x1F) << 1) | 1] << 8);

        /* Expand priority and palette bits */
        atex_mask = (attr >> 7) & 0x30;

        /* Point to a line of pattern data in cache */
        cache_ptr = &bg_pattern_cache[((attr & 0x7FF) << 6) | (v_row)];
        
	for(int i = 0; i < 8; i++)
	{
	 linebuf_ptr[column * 8 + i] = cache_ptr[i] | atex_mask;
	}
    }

    /* Draw last column (clipped) */
    if(shift)
    {
        int x, c, a;

        uint8 *p = &linebuf[(0 - shift)+(column << 3)];

	attr = nt[(((column + nt_scroll) & 0x1F) << 1) | 0] | (nt[(((column + nt_scroll) & 0x1F) << 1) | 1] << 8);

        a = (attr >> 7) & 0x30;

        for(x = 0; x < shift; x++)
        {
            c = bg_pattern_cache[((attr & 0x7FF) << 6) | (v_row) | (x)];
            p[x] = ((c) | (a));
        }
    }
}




/* Draw sprites */
void render_obj_sms(int line)
{
    int i;
    uint8 collision_buffer = 0;

    /* Sprite count for current line (8 max.) */
    int count = 0;

    /* Sprite dimensions */
    int width = 8;
    int height = (vdp.reg[1] & 0x02) ? 16 : 8;

    /* Pointer to sprite attribute table */
    uint8 *st = (uint8 *)&vdp.vram[vdp.satb];

    /* Adjust dimensions for double size sprites */
    if(vdp.reg[1] & 0x01)
    {
        width *= 2;
        height *= 2;
    }

    /* Draw sprites in front-to-back order */
    for(i = 0; i < 64; i++)
    {
        /* Sprite Y position */
        int yp = st[i];

        /* Found end of sprite list marker for non-extended modes? */
        if(vdp.extended == 0 && yp == 208)
            goto end;

        /* Actual Y position is +1 */
        yp++;

        /* Wrap Y coordinate for sprites > 240 */
        if(yp > 240) yp -= 256;

        /* Check if sprite falls on current line */
        if((line >= yp) && (line < (yp + height)))
        {
            uint8 *linebuf_ptr;

            /* Width of sprite */
            int start = 0;
            int end = width;

            /* Sprite X position */
            int xp = st[0x80 + (i << 1)];

            /* Pattern name */
            int n = st[0x81 + (i << 1)];

            /* Bump sprite count */
            count++;

            /* Too many sprites on this line ? */
            if(count == 9)
            {
                vdp.status |= 0x40;                
                goto end;
            }

            /* X position shift */
            if(vdp.reg[0] & 0x08) xp -= 8;

            /* Add MSB of pattern name */
            if(vdp.reg[6] & 0x04) n |= 0x0100;

            /* Mask LSB for 8x16 sprites */
            if(vdp.reg[1] & 0x02) n &= 0x01FE;

            /* Point to offset in line buffer */
            linebuf_ptr = (uint8 *)&linebuf[xp];

            /* Clip sprites on left edge */
            if(xp < 0)
            {
                start = (0 - xp);
            }

            /* Clip sprites on right edge */
            if((xp + width) > 256)        
            {
                end = (256 - xp);
            }

            /* Draw double size sprite */
            if(vdp.reg[1] & 0x01)
            {
                int x;
                uint8 *cache_ptr = (uint8 *)&bg_pattern_cache[(n << 6) | (((line - yp) >> 1) << 3)];

                /* Draw sprite line */
                for(x = start; x < end; x++)
                {
                    /* Source pixel from cache */
                    uint8 sp = cache_ptr[(x >> 1)];
    
                    /* Only draw opaque sprite pixels */
                    if(sp)
                    {
                        /* Background pixel from line buffer */
                        uint8 bg = linebuf_ptr[x];
    
                        /* Look up result */
                        linebuf_ptr[x] = lut[(bg << 8) | (sp)];
    
                        /* Update collision buffer */
                        collision_buffer |= bg;
                    }
                }
            }
            else /* Regular size sprite (8x8 / 8x16) */
            {
                int x;
                uint8 *cache_ptr = (uint8 *)&bg_pattern_cache[(n << 6) | ((line - yp) << 3)];

                /* Draw sprite line */
                for(x = start; x < end; x++)
                {
                    /* Source pixel from cache */
                    uint8 sp = cache_ptr[x];
    
                    /* Only draw opaque sprite pixels */
                    if(sp)
                    {
                        /* Background pixel from line buffer */
                        uint8 bg = linebuf_ptr[x];
    
                        /* Look up result */
                        linebuf_ptr[x] = lut[(bg << 8) | (sp)];
    
                        /* Update collision buffer */
                        collision_buffer |= bg;
                    }
                }
            }
        }
    }
end:
    /* Set sprite collision flag */
    if(collision_buffer & 0x40)
        vdp.status |= 0x20;
}



void update_bg_pattern_cache(void)
{
    int i;
    uint8 x, y;
    uint16 name;

    if(!bg_list_index) return;

    for(i = 0; i < bg_list_index; i++)
    {
        name = bg_name_list[i];
        bg_name_list[i] = 0;

        for(y = 0; y < 8; y++)
        {
            if(bg_name_dirty[name] & (1 << y))
            {
                uint8 *dst = &bg_pattern_cache[name << 6];

                uint16 bp01 = *(uint16 *)&vdp.vram[(name << 5) | (y << 2) | (0)];
                uint16 bp23 = *(uint16 *)&vdp.vram[(name << 5) | (y << 2) | (2)];
                uint32 temp = (bp_lut[bp01] >> 2) | (bp_lut[bp23]);

                for(x = 0; x < 8; x++)
                {
                    uint8 c = (temp >> (x << 2)) & 0x0F;
                    dst[0x00000 | (y << 3) | (x)] = (c);
                    dst[0x08000 | (y << 3) | (x ^ 7)] = (c);
                    dst[0x10000 | ((y ^ 7) << 3) | (x)] = (c);
                    dst[0x18000 | ((y ^ 7) << 3) | (x ^ 7)] = (c);
                }
            }
        }
        bg_name_dirty[name] = 0;
    }
    bg_list_index = 0;
}


/* Update a palette entry */
void palette_sync(int index, int force)
{
    int r, g, b;

    // unless we are forcing an update,
    // if not in mode 4, exit


    if(IS_SMS && !force && ((vdp.reg[0] & 4) == 0) )
        return;

    if(IS_GG)
    {
        /* ----BBBBGGGGRRRR */
        r = (vdp.cram[(index << 1) | (0)] >> 0) & 0x0F;
        g = (vdp.cram[(index << 1) | (0)] >> 4) & 0x0F;
        b = (vdp.cram[(index << 1) | (1)] >> 0) & 0x0F;
    
        r = gg_cram_expand_table[r];
        g = gg_cram_expand_table[g];
        b = gg_cram_expand_table[b];
    }
    else
    {
        /* --BBGGRR */
        r = (vdp.cram[index] >> 0) & 3;
        g = (vdp.cram[index] >> 2) & 3;
        b = (vdp.cram[index] >> 4) & 3;
    
        r = sms_cram_expand_table[r];
        g = sms_cram_expand_table[g];
        b = sms_cram_expand_table[b];
    }
    
    bitmap.pal.color[index][0] = r;
    bitmap.pal.color[index][1] = g;
    bitmap.pal.color[index][2] = b;

    pixel[index] = MAKE_PIXEL(r, g, b);

    bitmap.pal.dirty[index] = bitmap.pal.update = 1;
}

static void remap_8_to_32(int line)
{
    int i;
    uint32 *p = (uint32 *)&bitmap.data[(line * bitmap.pitch)];

    for(i = 0; i < 256; i++)
    {
       p[i] = pixel[ internal_buffer[i] & PIXEL_MASK ];
    }
}

void SMS_VDPSetPixelFormat(int, int, int)
{

}

