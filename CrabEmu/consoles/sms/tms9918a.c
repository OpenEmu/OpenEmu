/*
    This file is part of CrabEmu.

    Copyright (C) 2005, 2006, 2007, 2008, 2009 Lawrence Sebald

    CrabEmu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 
    as published by the Free Software Foundation.

    CrabEmu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CrabEmu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "sms.h"
#include "smsvdp.h"
#include "tms9918a.h"

#include <string.h>

/* The hardwired palette when used in a TMS9918 mode */
/* These constants are taken from Sean Young's TMS9918A Documentation */
#ifndef _arch_dreamcast
static const sms_vdp_color_t tms9918_pal[16] = {
    0xFF000000, /* Transparent */
    0xFF000000, /* Black */
    0xFF21C842, /* Medium Green */
    0xFF5EDC78, /* Light Green */
    0xFF5455ED, /* Dark Blue */
    0xFF7D76FC, /* Light Blue */
    0xFFD4524D, /* Dark Red */
    0xFF42EBF5, /* Cyan */
    0xFFFC5554, /* Medium Red */
    0xFFFF7978, /* Light Red */
    0xFFD4C154, /* Dark Yellow */
    0xFFE6CE80, /* Light Yellow */
    0xFF21B03B, /* Dark Green */
    0xFFC95BBA, /* Magenta */
    0xFFCCCCCC, /* Gray */
    0xFFFFFFFF, /* White */
};
#else
static const sms_vdp_color_t tms9918_pal[16] = {
    0x0000, /* Transparent */ 
    0x0000, /* Black */
    0x2648, /* Medium Green */
    0x5EEF, /* Light Green */
    0x52BD, /* Dark Blue */
    0x7BBF, /* Light Blue */
    0xD289, /* Dark Red */
    0x475E, /* Cyan */
    0xFAAA, /* Medium Red */
    0xFBCF, /* Light Red */
    0xD60A, /* Dark Yellow */
    0xE670, /* Light Yellow */
    0x2587, /* Dark Green */
    0xCAD7, /* Magenta */
    0xCE79, /* Gray */
    0xFFFF  /* White */
};
#endif

#define DRAW_COLOR(color, line, pixel) { \
    sms_vdp_color_t *_tmp = (smsvdp.framebuffer) + ((line) << 8) + (pixel); \
    *_tmp = color; \
}

#define DRAW_BACKGROUND(pixel) { \
    DRAW_COLOR(bg, line, (i << 3) + pixel); \
}

#define DRAW_FOREGROUND(pixel) { \
    DRAW_COLOR(fg, line, (i << 3) + pixel); \
}

void tms9918a_m0_draw_bg(int line, sms_vdp_color_t *px __UNUSED__)  {
    sms_vdp_color_t bg, fg;
    uint8 *name_table;
    uint8 *pattern_gen;
    uint8 *color_table;
    uint8 pixels, pattern, color;
    int i, row;

    name_table = &smsvdp.vram[(smsvdp.regs[2] & 0x0F) << 10];
    pattern_gen = &smsvdp.vram[(smsvdp.regs[4] & 0x07) << 11];
    color_table = &smsvdp.vram[smsvdp.regs[3] << 6];

    row = line >> 3;

    for(i = 0; i < 32; ++i) {
        pattern = name_table[(row << 5) + i];
        pixels = *(pattern_gen + (pattern << 3) + (line & 0x07));
        color = *(color_table + (pattern >> 3));

        if(color & 0x0F)
            bg = tms9918_pal[color & 0x0F];
        else
            bg = tms9918_pal[smsvdp.regs[7] & 0x0F];

        if(color >> 4)
            fg = tms9918_pal[color >> 4];
        else
            fg = tms9918_pal[smsvdp.regs[7] & 0x0F];

        if(pixels & 0x80)
            DRAW_FOREGROUND(0)
        else
            DRAW_BACKGROUND(0)

        if(pixels & 0x40)
            DRAW_FOREGROUND(1)
        else
            DRAW_BACKGROUND(1)

        if(pixels & 0x20)
            DRAW_FOREGROUND(2)
        else
            DRAW_BACKGROUND(2)

        if(pixels & 0x10)
            DRAW_FOREGROUND(3)
        else
            DRAW_BACKGROUND(3)

        if(pixels & 0x08)
            DRAW_FOREGROUND(4)
        else
            DRAW_BACKGROUND(4)

        if(pixels & 0x04)
            DRAW_FOREGROUND(5)
        else
            DRAW_BACKGROUND(5)

        if(pixels & 0x02)
            DRAW_FOREGROUND(6)
        else
            DRAW_BACKGROUND(6)

        if(pixels & 0x01)
            DRAW_FOREGROUND(7)
        else
            DRAW_BACKGROUND(7)
    }
}

#undef DRAW_BACKGROUND
#undef DRAW_FOREGROUND

#define DRAW_BACKDROP(pixel) { \
    DRAW_COLOR(bd, line, (i * 6) + pixel + 8); \
}

#define DRAW_TEXT(pixel) { \
    DRAW_COLOR(tc, line, (i * 6) + pixel + 8); \
}

void tms9918a_m1_draw_bg(int line, sms_vdp_color_t *px __UNUSED__)  {
    sms_vdp_color_t tc, bd;
    uint8 *name_table;
    uint8 *pattern_gen;
    uint8 pixels;
    int i, row;

    bd = tms9918_pal[smsvdp.regs[7] & 0x0F];
    tc = tms9918_pal[(smsvdp.regs[7] >> 4) & 0x0F];

    name_table = &smsvdp.vram[(smsvdp.regs[2] & 0x0F) << 10];
    pattern_gen = &smsvdp.vram[(smsvdp.regs[4] & 0x07) << 11];

    row = line >> 3;

    for(i = 0; i < 8; ++i)  {
        DRAW_COLOR(bd, line, i);
        DRAW_COLOR(bd, line, i + 248);
    }

    for(i = 0; i < 40; ++i) {
        pixels = *(pattern_gen + (name_table[row * 40 + i] << 3) +
                   (line & 0x07));

        if(pixels & 0x80)
            DRAW_TEXT(0)
        else
            DRAW_BACKDROP(0)

        if(pixels & 0x40)
            DRAW_TEXT(1)
        else
            DRAW_BACKDROP(1)

        if(pixels & 0x20)
            DRAW_TEXT(2)
        else
            DRAW_BACKDROP(2)

        if(pixels & 0x10)
            DRAW_TEXT(3)
        else
            DRAW_BACKDROP(3)

        if(pixels & 0x08)
            DRAW_TEXT(4)
        else
            DRAW_BACKDROP(4)

        if(pixels & 0x04)
            DRAW_TEXT(5)
        else
            DRAW_BACKDROP(5)
    }
}

#undef DRAW_BACKDROP
#undef DRAW_TEXT

#define DRAW_BACKGROUND(pixel) { \
    DRAW_COLOR(bg, line, (i << 3) + pixel); \
}

#define DRAW_FOREGROUND(pixel) { \
    DRAW_COLOR(fg, line, (i << 3) + pixel); \
}

void tms9918a_m2_draw_bg(int line, sms_vdp_color_t *px __UNUSED__)  {
    sms_vdp_color_t bg, fg;
    uint8 *name_table;
    uint8 *pattern_gen;
    uint8 *color_table;
    uint8 pixels, color;
    int pattern;
    int i, row, mask, mask2;

    row = line >> 3;    
    mask = ((smsvdp.regs[4] & 0x03) << 8) | 0xFF;
    mask2 = ((smsvdp.regs[3] & 0x7F) << 3) | 0x07;

    name_table = &smsvdp.vram[(smsvdp.regs[2] & 0x0F) << 10];
    pattern_gen = &smsvdp.vram[(smsvdp.regs[4] & 0x04) << 11];
    color_table = &smsvdp.vram[(smsvdp.regs[3] & 0x80) << 6];

    for(i = 0; i < 32; ++i) {
        pattern = name_table[(row << 5) + i] + ((row & 0x18) << 5);
        pixels = *(pattern_gen + ((pattern & mask) << 3) + (line & 0x07));
        color = *(color_table + ((pattern & mask2) << 3) + (line & 0x07));

        if(color & 0x0F)
            bg = tms9918_pal[color & 0x0F];
        else
            bg = tms9918_pal[smsvdp.regs[7] & 0x0F];

        if(color >> 4)
            fg = tms9918_pal[color >> 4];
        else
            fg = tms9918_pal[smsvdp.regs[7] & 0x0F];

        if(pixels & 0x80)
            DRAW_FOREGROUND(0)
        else
            DRAW_BACKGROUND(0)

        if(pixels & 0x40)
            DRAW_FOREGROUND(1)
        else
            DRAW_BACKGROUND(1)

        if(pixels & 0x20)
            DRAW_FOREGROUND(2)
        else
            DRAW_BACKGROUND(2)

        if(pixels & 0x10)
            DRAW_FOREGROUND(3)
        else
            DRAW_BACKGROUND(3)

        if(pixels & 0x08)
            DRAW_FOREGROUND(4)
        else
            DRAW_BACKGROUND(4)

        if(pixels & 0x04)
            DRAW_FOREGROUND(5)
        else
            DRAW_BACKGROUND(5)

        if(pixels & 0x02)
            DRAW_FOREGROUND(6)
        else
            DRAW_BACKGROUND(6)

        if(pixels & 0x01)
            DRAW_FOREGROUND(7)
        else
            DRAW_BACKGROUND(7)
    }
}

#undef DRAW_BACKGROUND
#undef DRAW_FOREGROUND

#define DRAW_PIXEL(pixel) { \
    DRAW_COLOR(c, line, (i << 3) + pixel); \
}

void tms9918a_m3_draw_bg(int line, sms_vdp_color_t *px __UNUSED__)  {
    uint8 *name_table;
    uint8 *pattern_gen;
    int row, i;
    uint8 pattern, color;
    sms_vdp_color_t c;

    name_table = &smsvdp.vram[(smsvdp.regs[2] & 0x0F) << 10];
    pattern_gen = &smsvdp.vram[(smsvdp.regs[4] & 0x07) << 11];

    row = line >> 3;

    for(i = 0; i < 32; ++i) {
        pattern = name_table[(row << 5) + i];

        if(!(line & 0x04))
            color = *(pattern_gen + (pattern << 3) + ((row & 0x03) << 1));
        else
            color = *(pattern_gen + (pattern << 3) + ((row & 0x03) << 1) + 1);

        if(color & 0x0F)
            c = tms9918_pal[color & 0x0F];
        else
            c = tms9918_pal[smsvdp.regs[7] & 0x0F];

        DRAW_PIXEL(0)
        DRAW_PIXEL(1)
        DRAW_PIXEL(2)
        DRAW_PIXEL(3)

        if(color >> 4)
            c = tms9918_pal[color & 0x0F];
        else
            c = tms9918_pal[smsvdp.regs[7] & 0x0F];

        DRAW_PIXEL(4)
        DRAW_PIXEL(5)
        DRAW_PIXEL(6)
        DRAW_PIXEL(7)
    }
}

#undef DRAW_PIXEL

#define CHECK_PIXEL(i)  { \
    if(col_tab[(i << size_shift)]) \
        smsvdp.status |= 0x20; \
\
    col_tab[(i << size_shift)] = 1; \
\
    if(size_shift)  { \
        if(col_tab[(i << 1) + 1]) \
            smsvdp.status |= 0x20; \
\
        col_tab[(i << 1) + 1] = 1; \
    } \
}

#define DRAW_PIXEL(i) { \
    DRAW_COLOR(c, line, (i << size_shift) + x); \
\
    if(size_shift) { \
        DRAW_COLOR(c, line, (i << 1) + x + 1); \
    } \
}

void tms9918a_m023_draw_spr(int line, sms_vdp_color_t *px __UNUSED__)   {
    uint8 *sat, *sprite_gen;
    int i, pattern_size, size_shift, tmp, pixels, pixels2 = 0;
    int count = 0, x, y = 0, pattern, color;
    sms_vdp_color_t c;
    static uint8 col_tab[0xFF];

    /* First of all, clear out our colision table */
    memset(col_tab, 0, 0xFF);

    sat = &smsvdp.vram[(smsvdp.regs[5] & 0x7F) << 7];
    sprite_gen = &smsvdp.vram[(smsvdp.regs[6] & 0x07) << 11];

    if(smsvdp.regs[1] & 0x01)
        size_shift = 1;
    else
        size_shift = 0;

    if(smsvdp.regs[1] & 0x02)
        pattern_size = 16;
    else
        pattern_size = 8;

    for(i = 0; i < 32 && count < 5; ++i) {
        y = sat[i << 2] + 1;

        if(y == 0xD1)   {
            /* End of list marker */
            break;
        }
        else if(line < y)  {
            continue;
        }
        else if(y + (pattern_size << size_shift) - 1 < line)    {
            continue;
        }
        else if(++count == 5)   {
            break;
        }

        x = sat[(i << 2) + 1];
        pattern = sat[(i << 2) + 2];
        color = sat[(i << 2) + 3];

        if(color & 0x80)    {
            x -= 32;
        }

        tmp = (line - y) >> size_shift;

        if(pattern_size == 8)   {
            pixels = *(sprite_gen + (pattern << 3) + tmp);
        }
        else    {
            pixels = *(sprite_gen + ((pattern & 0xFC) << 3) + tmp);
            pixels2 = *(sprite_gen + ((pattern & 0xFC) << 3) + tmp + 16);
        }

        if(color & 0x0F)    {
            c = tms9918_pal[color & 0x0F];
        }
        else    {
            if(pattern_size == 8)   {
                if(pixels & 0x80 && x >= 0)
                    CHECK_PIXEL(0)

                if(pixels & 0x40 && x + 1 >= 0)
                    CHECK_PIXEL(1)

                if(pixels & 0x20 && x + 2 >= 0)
                    CHECK_PIXEL(2)

                if(pixels & 0x10 && x + 3 >= 0)
                    CHECK_PIXEL(3)

                if(pixels & 0x08 && x + 4 >= 0)
                    CHECK_PIXEL(4)

                if(pixels & 0x04 && x + 5 >= 0)
                    CHECK_PIXEL(5)

                if(pixels & 0x02 && x + 6 >= 0)
                    CHECK_PIXEL(6)

                if(pixels & 0x01 && x + 7 >= 0)
                    CHECK_PIXEL(7)
            }
            else    {
                if(pixels & 0x80 && x >= 0)
                    CHECK_PIXEL(0)

                if(pixels & 0x40 && x + 1 >= 0)
                    CHECK_PIXEL(1)

                if(pixels & 0x20 && x + 2 >= 0)
                    CHECK_PIXEL(2)

                if(pixels & 0x10 && x + 3 >= 0)
                    CHECK_PIXEL(3)

                if(pixels & 0x08 && x + 4 >= 0)
                    CHECK_PIXEL(4)

                if(pixels & 0x04 && x + 5 >= 0)
                    CHECK_PIXEL(5)

                if(pixels & 0x02 && x + 6 >= 0)
                    CHECK_PIXEL(6)

                if(pixels & 0x01 && x + 7 >= 0)
                    CHECK_PIXEL(7)

                if(pixels2 & 0x80 && x + 8 >= 0)
                    CHECK_PIXEL(8)

                if(pixels2 & 0x40 && x + 9 >= 0)
                    CHECK_PIXEL(9)

                if(pixels2 & 0x20 && x + 10 >= 0)
                    CHECK_PIXEL(10)

                if(pixels2 & 0x10 && x + 11 >= 0)
                    CHECK_PIXEL(11)

                if(pixels2 & 0x08 && x + 12 >= 0)
                    CHECK_PIXEL(12)

                if(pixels2 & 0x04 && x + 13 >= 0)
                    CHECK_PIXEL(13)

                if(pixels2 & 0x02 && x + 14 >= 0)
                    CHECK_PIXEL(14)

                if(pixels2 & 0x01 && x + 15 >= 0)
                    CHECK_PIXEL(15)
            }
            continue;
        }

        if(pattern_size == 8)   {
            if(pixels & 0x80 && x >= 0)
                DRAW_PIXEL(0)

            if(pixels & 0x40 && x + 1 >= 0)
                DRAW_PIXEL(1)

            if(pixels & 0x20 && x + 2 >= 0)
                DRAW_PIXEL(2)

            if(pixels & 0x10 && x + 3 >= 0)
                DRAW_PIXEL(3)

            if(pixels & 0x08 && x + 4 >= 0)
                DRAW_PIXEL(4)

            if(pixels & 0x04 && x + 5 >= 0)
                DRAW_PIXEL(5)

            if(pixels & 0x02 && x + 6 >= 0)
                DRAW_PIXEL(6)

            if(pixels & 0x01 && x + 7 >= 0)
                DRAW_PIXEL(7)
        }
        else    {
            if(pixels & 0x80 && x >= 0)
                DRAW_PIXEL(0)

            if(pixels & 0x40 && x + 1 >= 0)
                DRAW_PIXEL(1)

            if(pixels & 0x20 && x + 2 >= 0)
                DRAW_PIXEL(2)

            if(pixels & 0x10 && x + 3 >= 0)
                DRAW_PIXEL(3)

            if(pixels & 0x08 && x + 4 >= 0)
                DRAW_PIXEL(4)

            if(pixels & 0x04 && x + 5 >= 0)
                DRAW_PIXEL(5)

            if(pixels & 0x02 && x + 6 >= 0)
                DRAW_PIXEL(6)

            if(pixels & 0x01 && x + 7 >= 0)
                DRAW_PIXEL(7)

            if(pixels2 & 0x80 && x + 8 >= 0)
                DRAW_PIXEL(8)

            if(pixels2 & 0x40 && x + 9 >= 0)
                DRAW_PIXEL(9)

            if(pixels2 & 0x20 && x + 10 >= 0)
                DRAW_PIXEL(10)

            if(pixels2 & 0x10 && x + 11 >= 0)
                DRAW_PIXEL(11)

            if(pixels2 & 0x08 && x + 12 >= 0)
                DRAW_PIXEL(12)

            if(pixels2 & 0x04 && x + 13 >= 0)
                DRAW_PIXEL(13)

            if(pixels2 & 0x02 && x + 14 >= 0)
                DRAW_PIXEL(14)

            if(pixels2 & 0x01 && x + 15 >= 0)
                DRAW_PIXEL(15)
        }
    }

    if(!(smsvdp.status & 0x40))  {
        if(count == 5)  {
            /* Set the 5 sprites flag and the fifth sprite bits */
            smsvdp.status |= 0x40 | ((i - 1) & 0x1F);
        }
        else if(y == 0xD0)  {
            /* Set the fifth sprite bits to the last sprite displayed */
            smsvdp.status |= (i & 0x1F);
        }
        else    {
            /* Otherwise, set the fifth sprite bits to the last sprite */
            smsvdp.status |= 0x1F;
        }
    }
}

#undef DRAW_PIXEL
#undef CHECK_PIXEL
