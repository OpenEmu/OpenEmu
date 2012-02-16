
#include "shared.h"

/* Bit 0 : BG enable, Bit 1 : OBJ enable */
int plane_enable = -1;

/* VCE color data to VGA 8-bit pixel table */
uint8 *xlat[2];

/* VCE color data to 16-bit pixel table */
uint16 pixel[2][0x100];

/* Precalculated 16-bit pixel values */
uint16 pixel_lut[0x200];

/* Pointer to 8 or 16-bit version of render_line() */
void (*render_line)(int line) = NULL;

/* List of precalculated sprite data */
t_sprite sprite_list[0x40];

/* Bitplane to packed pixel lookup table */
uint32 bp_lut[0x10000];

/* Used sprite data */
uint8 used_sprite_list[0x40];
uint8 used_sprite_index;

/*--------------------------------------------------------------------------*/
/* Init, reset, shutdown functions                                          */
/*--------------------------------------------------------------------------*/


int render_init(void)
{
    int i, j, x;
    uint8 *ptr;

    /* Make bitplane to pixel lookup table */
    for(i = 0; i < 0x100; i += 1)
    for(j = 0; j < 0x100; j += 1)
    {
        uint32 out = 0;
        for(x = 0; x < 8; x += 1)
        {
            out |= (j & (0x80 >> x)) ? (uint32)(8 << (x << 2)) : 0;
            out |= (i & (0x80 >> x)) ? (uint32)(4 << (x << 2)) : 0;
        }
#ifdef LSB_FIRST
        bp_lut[(j << 8) | (i)] = out;
#else
        bp_lut[(i << 8) | (j)] = out;
#endif
    }

    /* Set up pixel color remap tables, the alignment is DOS specific */
    ptr = malloc(0x400);
    if(!ptr) return (0);
    memset(ptr, 0, 0x400);
    xlat[0] = (uint8 *)(((uint32)ptr + 0x200) & ~0x1FF);
    xlat[1] = xlat[0] + 0x100;

    /* Make VCE data to raw pixel look-up table */
    for(i = 0; i < 0x200; i += 1)
    {
        int r = (i >> 3) & 7;
        int g = (i >> 6) & 7;
        int b = (i >> 0) & 7;
        pixel_lut[i] = (r << 13 | g << 8 | b << 2) & 0xE71C;
    }

    render_line = (bitmap.depth == 8) ? render_line_8 : render_line_16;

    return (1);
}


void render_reset(void)
{
    /* Hack for Mac port */
    render_line = (bitmap.depth == 8) ? render_line_8 : render_line_16;
}


void render_shutdown(void)
{
    /* todo: free xlat pointer */
}


int make_sprite_list(void)
{
    uint16 *sat = &objramw[0];
    int xpos, ypos, name, attr;
    int cgx, xflip, cgy, yflip;
    int width, height;
    int i;
    uint32 flip;

    used_sprite_index = 0;
    memset(&used_sprite_list, 0, sizeof(used_sprite_list));

    memset(&sprite_list, 0, sizeof(sprite_list));

    for(i = 0; i < 0x40; i += 1)
    {
        ypos = sat[(i << 2) | (0)];
        xpos = sat[(i << 2) | (1)];
        name = sat[(i << 2) | (2)];
        attr = sat[(i << 2) | (3)];

        ypos &= 0x3FF;
        xpos &= 0x3FF;

        if(xpos && ypos)
        {
            ypos -= 64;
            if(ypos >= 0x100) continue;
            cgy = (attr >> 12) & 3;
            cgy |= (cgy >> 1);
            height = (cgy + 1) << 4;
            if((ypos + height) < 0) continue;

            xpos -= 32;
            if(xpos >= 0x200) continue;
            cgx = (attr >> 8) & 1;
            width  = (cgx) ? 32 : 16;
            if((xpos + width) < 0) continue;

            xflip = (attr >> 11) & 1;
            yflip = (attr >> 15) & 1;
            flip = ((xflip << 9) | (yflip << 10)) & 0x600;

            name = (name >> 1) & 0x1FF;
            name &= ~((cgy << 1) | cgx);
            name |= flip;
            if(xflip && cgx) name ^= 1;

            sprite_list[i].top = ypos;
            sprite_list[i].bottom = ypos + height;
            sprite_list[i].xpos = xpos;
            sprite_list[i].name_left = name;
            sprite_list[i].name_right = name ^ 1;
            sprite_list[i].height = (height - 1);
            sprite_list[i].palette = (attr & 0x0F) << 4;

            if(yflip)
                sprite_list[i].flags |= FLAG_YFLIP;

            if(cgx)
                sprite_list[i].flags |= FLAG_CGX;

            if(!(attr & 0x80))
                sprite_list[i].flags |= FLAG_PRIORITY;

            used_sprite_list[used_sprite_index] = (i);
            used_sprite_index += 1;
        }
    }

    return (used_sprite_index);
}


/*--------------------------------------------------------------------------*/
/* Pattern and object cache update routines                                 */
/*--------------------------------------------------------------------------*/

void update_bg_pattern_cache(void)
{
    int i;
    uint8 x, y, c;
    uint16 name, index1, index2;
    uint32 temp;

    if(!bg_list_index) return;

    for(i = 0; i < bg_list_index; i += 1)
    {
        name = bg_name_list[i];
        bg_name_list[i] = 0;

        for(y = 0; y < 8; y += 1)
        {
            if(bg_name_dirty[name] & (1 << y))
            {
                index1 = vramw[(name << 4) | (y)];
                index2 = vramw[(name << 4) | (y) | (8)];

                temp = (bp_lut[index1] >> 2) | bp_lut[index2];

                for(x = 0; x < 8; x += 1)
                {
                    c = (temp >> (x << 2)) & 0x0F;
                    bg_pattern_cache[(name << 6) | (y << 3) | (x)] = (c);
                }
            }
        }
        bg_name_dirty[name] = 0;
    }

    bg_list_index = 0;
}



void update_obj_pattern_cache(void)
{
    int i;
    uint16 name;
    uint16 b0, b1, b2, b3;
    uint8 i0, i1, i2, i3;           
    uint8 x, y, c;

    if(!obj_list_index) return;

    for(i = 0; i < obj_list_index; i += 1)
    {
        name = obj_name_list[i];
        obj_name_list[i] = 0;

        for(y = 0; y < 0x10; y += 1)
        {
            if(obj_name_dirty[name] & (1 << y))
            {
                b0 = vramw[(name << 6) + (y) + (0x00)];
                b1 = vramw[(name << 6) + (y) + (0x10)];
                b2 = vramw[(name << 6) + (y) + (0x20)];
                b3 = vramw[(name << 6) + (y) + (0x30)];

                for(x = 0; x < 0x10; x += 1)
                {
                    i0 = (b0 >> (x ^ 0x0F)) & 1;
                    i1 = (b1 >> (x ^ 0x0F)) & 1;
                    i2 = (b2 >> (x ^ 0x0F)) & 1;
                    i3 = (b3 >> (x ^ 0x0F)) & 1;

                    c = (i3 << 3 | i2 << 2 | i1 << 1 | i0);

                    obj_pattern_cache[(name << 8) | (y << 4) | (x)] = (c);
                    obj_pattern_cache[0x20000 | (name << 8) | (y << 4) | (x ^ 0x0F)] = (c);
                    obj_pattern_cache[0x40000 | (name << 8) | ((y ^ 0x0F) << 4) | (x)] = (c);
                    obj_pattern_cache[0x60000 | (name << 8) | ((y ^ 0x0F) << 4) | (x ^ 0x0F)] = (c);
                }
            }
        }
        obj_name_dirty[name] = 0;
    }
    obj_list_index = 0;
}


/*--------------------------------------------------------------------------*/
/* Render functions                                                         */
/*--------------------------------------------------------------------------*/


void render_line_8(int line)
{
    if((reg[0x05] & 0x80) && (plane_enable & 1))
    {
        update_bg_pattern_cache();
        render_bg_8(line);
    }
    else
    {
        memset(&bitmap.data[(line * bitmap.pitch) + (bitmap.viewport.x * bitmap.granularity)], xlat[0][0], disp_width);
    }

    if((reg[0x05] & 0x40) && (plane_enable & 2))
    {
        update_obj_pattern_cache();
        render_obj_8(line);
    }
}


void render_line_16(int line)
{
    if((reg[0x05] & 0x80) && (plane_enable & 1))
    {
        update_bg_pattern_cache();
        render_bg_16(line);
    }
    else
    {
        int i;
        uint16 *ptr = (uint16 *)&bitmap.data[(line * bitmap.pitch) + (bitmap.viewport.x * bitmap.granularity)];
        for(i = 0; i < disp_width; i += 1) ptr[i] = pixel[0][0];            
    }

    if((reg[0x05] & 0x40) && (plane_enable & 2))
    {
        update_obj_pattern_cache();
        render_obj_16(line);
    }
}


void render_bg_8(int line)
{
    uint16 *nt;
    uint8 *src, *dst, palette;
    int column, name, attr, x, shift, v_line, nt_scroll;
    int xscroll = (reg[7] & 0x03FF);
    int end = disp_nt_width;

    /* Offset in pattern, in lines */
    v_line = (y_offset & 7);

    /* Offset in name table, in columns */
    nt_scroll = (xscroll >> 3);

    /* Offset in column, in pixels */
    shift = (xscroll & 7);

    /* Draw an extra tile for the last column */
    if(shift) end += 1;

    /* Point to current offset within name table */
    nt = (uint16 *)&vram[(y_offset >> 3) << playfield_shift];

    /* Point to start in line buffer */
    dst = &bitmap.data[(line * bitmap.pitch) + 0x20 + (0 - shift)];

    /* Draw columns */
    for(column = 0; column < end; column += 1)
    {
        /* Get attribute */
        attr = nt[(column + nt_scroll) & playfield_row_mask];

        /* Extract name and palette bits */
        name = (attr & 0x07FF);
        palette = (attr >> 8) & 0xF0;

        /* Point to current pattern line */
        src = &bg_pattern_cache[(name << 6) + (v_line << 3)];

        for(x = 0; x < 8; x += 1)
        {
            dst[(column << 3) | (x)] = xlat[0][(src[x] | palette)];
        }
    }
}


void render_bg_16(int line)
{
    uint16 *nt;
    uint8 *src, palette;
    uint16 *dst;
    int column, name, attr, x, shift, v_line, nt_scroll;
    int xscroll = (reg[7] & 0x03FF);
    int end = disp_nt_width;

    /* Offset in pattern, in lines */
    v_line = (y_offset & 7);

    /* Offset in name table, in columns */
    nt_scroll = (xscroll >> 3);

    /* Offset in column, in pixels */
    shift = (xscroll & 7);

    /* Draw an extra tile for the last column */
    if(shift) end += 1;

    /* Point to current offset within name table */
    nt = (uint16 *)&vram[(y_offset >> 3) << playfield_shift];

    /* Point to start in line buffer */
    dst = (uint16 *)&bitmap.data[(line * bitmap.pitch) + ((0x20 + (0 - shift)) << 1)];

    /* Draw columns */
    for(column = 0; column < end; column += 1)
    {
        /* Get attribute */
        attr = nt[(column + nt_scroll) & playfield_row_mask];

        /* Extract name and palette bits */
        name = (attr & 0x07FF);
        palette = (attr >> 8) & 0xF0;

        /* Point to current pattern line */
        src = &bg_pattern_cache[(name << 6) + (v_line << 3)];

        /* Draw column */
        for(x = 0; x < 8; x += 1)
        {
            dst[(column << 3) | (x)] = pixel[0][(src[x] | palette)];
        }
    }
}


void render_obj_8(int line)
{
    t_sprite *p;
    int j, i, x, c;
    int name, name_mask;
    int v_line;
    uint8 *src;
    int nt_line;
    uint8 *dst;

    for(j = (used_sprite_index - 1); j >= 0; j -= 1)
    {
        i = used_sprite_list[j];
        p = &sprite_list[i];

        if( (line >= p->top) && (line < p->bottom))
        {
            v_line = (line - p->top) & p->height;
            nt_line = v_line;
            if(p->flags & FLAG_YFLIP) nt_line = (p->height - nt_line);
            name_mask = ((nt_line >> 4) & 3) << 1;
            name = (p->name_left | name_mask);
            v_line &= 0x0F;

            src = &obj_pattern_cache[(name << 8) | ((v_line & 0x0f) << 4)];
            dst = &bitmap.data[(line * bitmap.pitch) + ((0x20+p->xpos) & 0x1ff)];

            for(x = 0; x < 0x10; x += 1)
            {
                c = src[x];
                if(c) dst[x] = xlat[1][((c) | p->palette)];
            }

            if(p->flags & FLAG_CGX)
            {
                name = (p->name_right | name_mask);
                src = &obj_pattern_cache[(name << 8) | ((v_line & 0x0f) << 4)];
                dst += 0x10;

                for(x = 0; x < 0x10; x += 1)
                {
                    c = src[x];
                    if(c) dst[x] = xlat[1][((c) | p->palette)];
                }
            }
        }
    }
}


void render_obj_16(int line)
{
    t_sprite *p;
    int j, i, x, c;
    int name, name_mask;
    int v_line;
    uint8 *src;
    int nt_line;
    uint16 *dst;

    for(j = (used_sprite_index - 1); j >= 0; j -= 1)
    {
        i = used_sprite_list[j];
        p = &sprite_list[i];

        if( (line >= p->top) && (line < p->bottom))
        {
            v_line = (line - p->top) & p->height;
            nt_line = v_line;
            if(p->flags & FLAG_YFLIP) nt_line = (p->height - nt_line);
            name_mask = ((nt_line >> 4) & 3) << 1;
            name = (p->name_left | name_mask);
            v_line &= 0x0F;

            src = &obj_pattern_cache[(name << 8) | ((v_line & 0x0f) << 4)];
            dst = (uint16 *)&bitmap.data[(line * bitmap.pitch) + (((0x20+p->xpos) & 0x1ff) * (bitmap.granularity))];

            for(x = 0; x < 0x10; x += 1)
            {
                c = src[x];
                if(c) dst[x] = pixel[1][((c) | p->palette)];
            }

            if(p->flags & FLAG_CGX)
            {
                name = (p->name_right | name_mask);
                src = &obj_pattern_cache[(name << 8) | ((v_line & 0x0f) << 4)];
                dst += 0x10;

                for(x = 0; x < 0x10; x += 1)
                {
                    c = src[x];
                    if(c) dst[x] = pixel[1][((c) | p->palette)];
                }
            }
        }
    }
}

