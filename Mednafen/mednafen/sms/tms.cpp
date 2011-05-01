/*
    tms.c --
    TMS9918 and legacy video mode support.
*/
#include "shared.h"

namespace MDFN_IEN_SMS
{

int text_counter;               /* Text offset counter */
uint8 tms_lookup[16][256][2];   /* Expand BD, PG data into 8-bit pixels (G1,G2) */
uint8 mc_lookup[16][256][8];    /* Expand BD, PG data into 8-bit pixels (MC) */
uint8 txt_lookup[256][2];       /* Expand BD, PG data into 8-bit pixels (TX) */
uint8 bp_expand[256][8];        /* Expand PG data into 8-bit pixels */
uint8 tms_obj_lut[16*256];      /* Look up priority between SG and display pixels */

static const uint8 diff_mask[]  = {0x07, 0x07, 0x0F, 0x0F};
static const uint8 name_mask[]  = {0xFF, 0xFF, 0xFC, 0xFC};
static const uint8 diff_shift[] = {0, 1, 0, 1};
static const uint8 size_tab[]   = {8, 16, 16, 32};

/* Internally latched sprite data in the VDP */
typedef struct {
    int xpos;
    uint8 attr;
    uint8 sg[2];
} tms_sprite;

tms_sprite sprites[4];
int sprites_found;

void parse_line(int line)
{
    int yp, i;
    int mode = vdp.reg[1] & 3;
    int size = size_tab[mode];
    int diff, name;
    uint8 *sa, *sg;
    tms_sprite *p;

    /* Reset # of sprites found */
    sprites_found = 0;

    /* Parse sprites */
    for(i = 0; i < 32; i++)
    {
        /* Point to current sprite in SA and our current sprite record */
        p = &sprites[sprites_found];
        sa = &vdp.vram[vdp.sa + (i << 2)];

        /* Fetch Y coordinate */
        yp = sa[0];

        /* Check for end marker */
        if(yp == 0xD0)
            goto parse_end;

        /* Wrap Y position */
        if(yp > 0xE0)
            yp -= 256;

        /* Check if sprite falls on following line */
        if(line >= yp && line < (yp + size))
        {
            /* Sprite overflow on this line */
            if(sprites_found == 4)
            {
                /* Set 5S and abort parsing */
                vdp.status |= 0x40;
                goto parse_end;
            }

            /* Fetch X position */
            p->xpos = sa[1];

            /* Fetch name */
            name = sa[2] & name_mask[mode];

            /* Load attribute into attribute storage */
            p->attr = sa[3];

            /* Apply early clock bit */
            if(p->attr & 0x80)
                p->xpos -= 32;

            /* Calculate offset in pattern */
            diff = ((line - yp) >> diff_shift[mode]) & diff_mask[mode];

            /* Insert additional name bit for 16-pixel tall sprites */
            if(diff & 8)
                name |= 1;

            /* Fetch SG data */
            sg = &vdp.vram[vdp.sg | (name << 3) | (diff & 7)];
            p->sg[0] = sg[0x00];
            p->sg[1] = sg[0x10];

            /* Bump found sprite count */
            ++sprites_found;
        }
    }
parse_end:

    /* Insert number of last sprite entry processed */
    vdp.status = (vdp.status & 0xE0) | (i & 0x1F);
}

void render_obj_tms(int line)
{
    int i, x = 0;
    int size, start, end, mode;
    uint8 *lb, *obj_lut, *ex[2];
    tms_sprite *p;

    mode = vdp.reg[1] & 3;
    size = size_tab[mode];

    /* Render sprites */
    for(i = 0; i < sprites_found; i++)
    {
        p = &sprites[i];
        lb = &linebuf[p->xpos];
        obj_lut = &tms_obj_lut[(p->attr & 0x0F) << 8];

        /* Point to expanded PG data */
        ex[0] = bp_expand[p->sg[0]];
        ex[1] = bp_expand[p->sg[1]];

        /* Clip left edge */
        if(p->xpos < 0)
            start = 0 - p->xpos;
        else
            start = 0;

        /* Clip right edge */
        if(p->xpos > 256 - size)
            end = 256 - p->xpos;
        else
            end = size;

        /* Render sprite line */
        switch(mode)
        {
            case 0: /* 8x8 */
                for(x = start; x < end; x++) {
                    if(ex[0][x])
                        lb[x] = obj_lut[lb[x]];
                }
                break;

            case 1: /* 8x8 zoomed */
                for(x = start; x < end; x++) {                   
                    if(ex[0][x >> 1])
                        lb[x] = obj_lut[lb[x]];
                }
                break;

            case 2: /* 16x16 */
                for(x = start; x < end; x++) {
                    if(ex[(x >> 3) & 1][x & 7])
                        lb[x] = obj_lut[lb[x]];
                }
                break;

            case 3: /* 16x16 zoomed */
                for(x = start; x < end; x++) {
                    if(ex[(x >> 4) & 1][(x >> 1) & 7])
                        lb[x] = obj_lut[lb[x]];
                }
                break;
        }
    }
}

/****
1.) NOTE: xpos can be negative, but the 'start' value that is added
    to xpos will ensure it is positive.
    
    For an EC sprite that is offscreen, 'start' will be larger
    than 'end' and the for-loop used for rendering will abort
    on the first pass.
***/


#define RENDER_TX_LINE \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ];

#define RENDER_TX_BORDER \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; \
        *lb++ = 0x10 | clut[0]; 

#define RENDER_GR_LINE \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ]; \
        *lb++ = 0x10 | clut[ *bpex++ ];

#define RENDER_MC_LINE \
        *lb++ = 0x10 | *mcex++; \
        *lb++ = 0x10 | *mcex++; \
        *lb++ = 0x10 | *mcex++; \
        *lb++ = 0x10 | *mcex++; \
        *lb++ = 0x10 | *mcex++; \
        *lb++ = 0x10 | *mcex++; \
        *lb++ = 0x10 | *mcex++; \
        *lb++ = 0x10 | *mcex++;

void make_tms_tables(void)
{
    int i, j, x;
    int bd, pg, ct;
    int sx, bx;

    for(sx = 0; sx < 16; sx++)
    {
        for(bx = 0; bx < 256; bx++)
        {
//          uint8 bd = (bx & 0x0F);
            uint8 bs = (bx & 0x40);
//          uint8 bt = (bd == 0) ? 1 : 0;
            uint8 sd = (sx & 0x0F);
//          uint8 st = (sd == 0) ? 1 : 0;

            // opaque sprite pixel, choose 2nd pal and set sprite marker
            if(sd && !bs)
            {
                tms_obj_lut[(sx<<8)|(bx)] = sd | 0x10 | 0x40;
            }
            else
            if(sd && bs)
            {
                // writing over a sprite
                tms_obj_lut[(sx<<8)|(bx)] = bx;
            }
            else
            {
                tms_obj_lut[(sx<<8)|(bx)] = bx;
            }
        }
    }


    /* Text lookup table */
    for(bd = 0; bd < 256; bd++)
    {
        uint8 bg = (bd >> 0) & 0x0F;
        uint8 fg = (bd >> 4) & 0x0F;

        /* If foreground is transparent, use background color */
        if(fg == 0) fg = bg;

        txt_lookup[bd][0] = bg;
        txt_lookup[bd][1] = fg;
    }

    /* Multicolor lookup table */
    for(bd = 0; bd < 16; bd++)
    {
        for(pg = 0; pg < 256; pg++)
        {
            int l = (pg >> 4) & 0x0F;
            int r = (pg >> 0) & 0x0F;

            /* If foreground is transparent, use background color */
            if(l == 0) l = bd;
            if(r == 0) r = bd;

            /* Unpack 2 nibbles across eight pixels */
            for(x = 0; x < 8; x++)
            {
                int c = (x & 4) ? r : l;

                mc_lookup[bd][pg][x] = c;
            }
        }
    }

    /* Make bitmap data expansion table */
    memset(bp_expand, 0, sizeof(bp_expand));
    for(i = 0; i < 256; i++)
    {
        for(j = 0; j < 8; j++)
        {
            int c = (i >> (j ^ 7)) & 1;
            bp_expand[i][j] = c;
        }
    }

    /* Graphics I/II lookup table */
    for(bd = 0; bd < 0x10; bd++)
    {
        for(ct = 0; ct < 0x100; ct++)
        {
            int backdrop = (bd & 0x0F);
            int background = (ct >> 0) & 0x0F;
            int foreground = (ct >> 4) & 0x0F;

            /* If foreground is transparent, use background color */
            if(background == 0) background = backdrop;
            if(foreground == 0) foreground = backdrop;

            tms_lookup[bd][ct][0] = background;
            tms_lookup[bd][ct][1] = foreground;
        }
    }
}


void render_bg_tms(int line)
{
    switch(vdp.mode & 7)
    {
        case 0: /* Graphics I */
            render_bg_m0(line);
            break;

        case 1: /* Text */
            render_bg_m1(line);
            break;

        case 2: /* Graphics II */
            render_bg_m2(line);
            break;

        case 3: /* Text (Extended PG) */
            render_bg_m1x(line);
            break;

        case 4: /* Multicolor */
            render_bg_m3(line);
            break;

        case 5: /* Invalid (1+3) */
            render_bg_inv(line);
            break;

        case 6: /* Multicolor (Extended PG) */
            render_bg_m3x(line);
            break;

        case 7: /* Invalid (1+2+3) */
            render_bg_inv(line);
            break;
    }
}

/* Graphics I */
void render_bg_m0(int line)
{
    int v_row  = (line & 7);
    int column;
    int name;

    uint8 *clut;
    uint8 *bpex;
    uint8 *lb = &linebuf[0];
    uint8 *pn = &vdp.vram[vdp.pn + ((line >> 3) << 5)];
    uint8 *ct = &vdp.vram[vdp.ct];
    uint8 *pg = &vdp.vram[vdp.pg | (v_row)];

    for(column = 0; column < 32; column++)
    {
        name = pn[column];
        clut = &tms_lookup[vdp.bd][ct[name >> 3]][0];
        bpex = &bp_expand[pg[name << 3]][0];
        RENDER_GR_LINE
    }
}

/* Text */
void render_bg_m1(int line)
{
    int v_row  = (line & 7);
    int column;

    uint8 *clut;
    uint8 *bpex;
    uint8 *lb = &linebuf[0];
//  uint8 *pn = &vdp.vram[vdp.pn + ((line >> 3) * 40)];

    uint8 *pn = &vdp.vram[vdp.pn + text_counter];

    uint8 *pg = &vdp.vram[vdp.pg | (v_row)];
    uint8 bk = vdp.reg[7];

    clut = &txt_lookup[bk][0];

    for(column = 0; column < 40; column++)
    {
        bpex = &bp_expand[pg[pn[column] << 3]][0];
        RENDER_TX_LINE        
    }

    /* V3 */
    if((vdp.line & 7) == 7)
        text_counter += 40;

    RENDER_TX_BORDER
}

/* Text + extended PG */
void render_bg_m1x(int line)
{
    int v_row  = (line & 7);
    int column;

    uint8 *clut;
    uint8 *bpex;
    uint8 *lb = &linebuf[0];
    uint8 *pn = &vdp.vram[vdp.pn + ((line >> 3) * 40)];
    uint8 *pg = &vdp.vram[vdp.pg + (v_row) + ((line & 0xC0) << 5)];
    uint8 bk = vdp.reg[7];

    clut = &tms_lookup[0][bk][0];

    for(column = 0; column < 40; column++)
    {
        bpex = &bp_expand[pg[pn[column] << 3]][0];
        RENDER_TX_LINE
    }
    RENDER_TX_BORDER
}

/* Invalid (2+3/1+2+3) */
void render_bg_inv(int line)
{
    int column;
    uint8 *clut;
    uint8 *bpex;
    uint8 *lb = &linebuf[0];
    uint8 bk = vdp.reg[7];

    clut = &txt_lookup[bk][0];

    for(column = 0; column < 40; column++)
    {
        bpex = &bp_expand[0xF0][0];
        RENDER_TX_LINE
    }
}

/* Multicolor */
void render_bg_m3(int line)
{
    int column;
    uint8 *mcex;
    uint8 *lb = &linebuf[0];

    uint8 *pn = &vdp.vram[vdp.pn + ((line >> 3) << 5)];
    uint8 *pg = &vdp.vram[vdp.pg + ((line >> 2) & 7)];

    for(column = 0; column < 32; column++)
    {
        mcex = &mc_lookup[vdp.bd][pg[pn[column]<<3]][0];
        RENDER_MC_LINE
    }
}

/* Multicolor + extended PG */
void render_bg_m3x(int line)
{
    int column;
    uint8 *mcex;
    uint8 *lb = &linebuf[0];
    uint8 *pn = &vdp.vram[vdp.pn + ((line >> 3) << 5)];
    uint8 *pg = &vdp.vram[vdp.pg + ((line >> 2) & 7) + ((line & 0xC0) << 5)];

    for(column = 0; column < 32; column++)
    {
        mcex = &mc_lookup[vdp.bd][pg[pn[column]<<3]][0];
        RENDER_MC_LINE
    }
}

/* Graphics II */
void render_bg_m2(int line)
{
    int v_row  = (line & 7);
    int column;
    int name;

    uint8 *clut;
    uint8 *bpex;
    uint8 *lb = &linebuf[0];
    uint8 *pn = &vdp.vram[vdp.pn | ((line & 0xF8) << 2)];
    uint8 *ct = &vdp.vram[(vdp.ct & 0x2000) | (v_row) | ((line & 0xC0) << 5)];
    uint8 *pg = &vdp.vram[(vdp.pg & 0x2000) | (v_row) | ((line & 0xC0) << 5)];

    for(column = 0; column < 32; column++)
    {
        name = pn[column] << 3;
        clut = &tms_lookup[vdp.bd][ct[name]][0];
        bpex = &bp_expand[pg[name]][0];
        RENDER_GR_LINE
    }
}

}
