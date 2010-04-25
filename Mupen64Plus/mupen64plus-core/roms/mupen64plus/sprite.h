/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - sprite.h                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Marshallh                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Functions for drawing 32-bit alpha-blended sprites. */

void drawString(Gfx** glistp, int face, int x, int y, char* inpStr, f32 scaleX, f32 scaleY, int alpha);
void drawChar(Gfx** glistp, int face, int ch, int x, int y, f32 scaleX, f32 scaleY, int alpha);
void drawSprite(Gfx** glistp, void *buf, int x, int y, int width, int height, f32 scale_x, f32 scale_y, int alphabit);
void SpriteFinish(Gfx** glistp);
void SpriteInit(Gfx** glistp);

#define NUM_template_BMS (NUM_DL(52))

static Gfx sprite_dl[NUM_DL(NUM_template_BMS)];

Sprite default_sprite = {
    0,0,             /* position: x,y */
    0,0,             /* sprite size in texels: x,y */
    1.0,1.0,         /* sprite scale: x,y */
    0,0,             /* explosion: x,y */
    3,               /* attribute: just copy :) */
    0x1234,          /* sprite depth: z */
    255,255,255,255, /* sprite coloration RGBA */
    0,0,NULL,        /* color lookup table: start_index, length, address */
    0,1,             /* sprite bitmap index: start_index, increment */
    1,               /* number of bitmaps */
    1,               /* number of dl locations allocated */
    0,0,             /* sprite bitmap height: used_height, physical height */
    G_IM_FMT_RGBA,   /* sprite bitmap format */
    G_IM_SIZ_32b,    /* sprite bitmap texel size */
    NULL,            /* pointer to bitmaps */
    sprite_dl,       /* rsp_dl: display list memory */
    NULL,            /* rsp_next_dl: dynamic_dl pointer */
    0,0              /* texture fractional offsets */
};

void drawString(Gfx** glistp, int face, int x, int y, char* inpStr, f32 scaleX, f32 scaleY, int alpha)
{
    int i = 0;
    int fontID = 0;
    int sx = x;
    int sy = y;
    char ch;

    for(i = 0; i < strlen(inpStr); i++)
        {
        fontID = 1024;
        ch = inpStr[i];
        switch(ch)
            {
            case 'a': {fontID = 0; break;}
            case 'b': {fontID = 1; break;}
            case 'c': {fontID = 2; break;}
            case 'd': {fontID = 3; break;}
            case 'e': {fontID = 4; break;}
            case 'f': {fontID = 5; break;}
            case 'g': {fontID = 6; break;}
            case 'h': {fontID = 7; break;}
            case 'i': {fontID = 8; break;}
            case 'j': {fontID = 9; break;}
            case 'k': {fontID = 10; break;}
            case 'l': {fontID = 11; break;}
            case 'm': {fontID = 12; break;}
            case 'n': {fontID = 13; break;}
            case 'o': {fontID = 14; break;}
            case 'p': {fontID = 15; break;}
            case 'q': {fontID = 16; break;}
            case 'r': {fontID = 17; break;}
            case 's': {fontID = 18; break;}
            case 't': {fontID = 19; break;}
            case 'u': {fontID = 20; break;}
            case 'v': {fontID = 21; break;}
            case 'w': {fontID = 22; break;}
            case 'x': {fontID = 23; break;}
            case 'y': {fontID = 24; break;}
            case 'z': {fontID = 25; break;}

            case '0': {fontID = 26; break;}
            case '1': {fontID = 27; break;}
            case '2': {fontID = 28; break;}
            case '3': {fontID = 29; break;}
            case '4': {fontID = 30; break;}
            case '5': {fontID = 31; break;}
            case '6': {fontID = 32; break;}
            case '7': {fontID = 33; break;}
            case '8': {fontID = 34; break;}
            case '9': {fontID = 35; break;}
            case '"': {fontID = 36; break;}
            case '-': {fontID = 37; break;}
            case '=': {fontID = 38; break;}
            case '+': {fontID = 39; break;}
            case '!': {fontID = 40; break;}
            case '@': {fontID = 41; break;}
            case ':': {fontID = 42; break;}
            case '[': {fontID = 43; break;}
            case ']': {fontID = 44; break;}
            case '&': {fontID = 45; break;}
            case '*': {fontID = 46; break;}
            case '(': {fontID = 47; break;}
            case ')': {fontID = 48; break;}
            case '.': {fontID = 49; break;}
            case ',': {fontID = 50; break;}
            case '/': {fontID = 51; break;}
            }


        if(fontID != 1024)
            {
            drawChar(glistp, face, fontID, sx, sy, scaleX, scaleY, alpha);
            /* font structures not included */
            /* sx += (f32)(font[face*52 + fontID].width + 2) * scaleX; */
            }
        else
            sx += 8.0*scaleX;

        if(ch == '\n')
            {
            sy+= (30.0)*scaleY;
            sx = x;
            }
        }
}

void drawChar(Gfx** glistp, int face, int ch, int x, int y, f32 scaleX, f32 scaleY, int alpha)
{
    /* drawSprite(glistp, font[ch+face*52].buf, x, y, 32, 32, scaleX, scaleY, alpha); */
}

void drawSprite(Gfx** glistp, void* buf, int x, int y, int width, int height, f32 scale_x, f32 scale_y, int alphabit)
{
    Sprite *sp;

    Gfx *gxp, *dl;

    Bitmap bm = {width, width, 0, 0, buf, height, 0};

    gxp = *glistp;
    sp = &default_sprite;

    sp->x         = x;
    sp->y         = y;
    sp->width     = bm.width;
    sp->height    = bm.actualHeight;
    sp->bmheight  = bm.actualHeight;
    sp->bmHreal   = bm.actualHeight;
    sp->bitmap    = &bm;

    spScale(sp, scale_x, scale_y);
    spColor(sp, 255, 255, 255, alphabit);

    dl = spDraw(sp);
    gSPDisplayList(gxp++, dl);

    *glistp = gxp;
}

void SpriteFinish(Gfx** glistp)
{
    Gfx *gxp;

    gxp = *glistp;
    spFinish(&gxp);
    *glistp = (gxp-1); /* Don't use final EndDisplayList() */
}

void SpriteInit(Gfx** glistp)
{
    Gfx* gxp;

    gxp = *glistp; 
    spInit(&gxp);

    default_sprite.rsp_dl_next = default_sprite.rsp_dl;

    *glistp = gxp;
}


