/****************************************************************************
 *  legal.c
 *
 *  Genesis Plus GX Disclaimer
 *
 *  Softdev (2006)
 *  Eke-Eke (2007,2008,2009,2010)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ***************************************************************************/

#include "shared.h"
#include "font.h"
#include "gui.h"

extern const u8 Bg_intro_c1_png[];
extern const u8 Bg_intro_c2_png[];
extern const u8 Bg_intro_c3_png[];
extern const u8 Bg_intro_c4_png[];
extern const u8 Bg_intro_c5_png[];

/* 
 * This is the legal stuff - which must be shown at program startup 
 * Any derivative work MUST include the same textual output.
 *
 * In other words, play nice and give credit where it's due.
 */

static void show_disclaimer(int ypos)
{
  FONT_writeCenter ("DISCLAIMER",22,0,640,ypos,(GXColor)WHITE);
  ypos += 32;
  FONT_writeCenter ("This is free software, and you are welcome to",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("redistribute it under the conditions of the",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("GNU GENERAL PUBLIC LICENSE Version 2.",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("Authors can not be held responsible for any damage or",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("or dysfunction that could occur while using this port.",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("You may not sell, lease, rent or generally use",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("this software in any commercial product or activity.",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("You may not distribute this software with any ROM image",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("unless you have the legal right to distribute them.",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("This software is not endorsed by or affiliated",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("with Sega Enterprises Ltd or Nintendo Co Ltd.",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("All trademarks and registered trademarks are",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("the property of their respective owners.",20,0,640,ypos,(GXColor)WHITE);
  ypos += 38;
}

void legal ()
{
  int count = 2000;
  int vis = 0;

#ifdef HW_RVL
  gx_texture *button = gxTextureOpenPNG(Key_A_wii_png,0);
#else
  gx_texture *button = gxTextureOpenPNG(Key_A_gcn_png,0);
#endif

  gx_texture *logo_left= gxTextureOpenPNG(Bg_intro_c5_png,0);
  gx_texture *logo_right = gxTextureOpenPNG(Bg_intro_c4_png,0);

  gxClearScreen((GXColor)BLACK);
  show_disclaimer(56);
  gxDrawTexture(logo_left, (640-logo_left->width-logo_right->width -32)/2, 480-logo_left->height-24, logo_left->width, logo_left->height,255);
  gxDrawTexture(logo_right, (640-logo_left->width-logo_right->width -32)/2+logo_left->width+32, 480-logo_right->height-24, logo_right->width, logo_right->height,255);
  gxSetScreen();

  sleep(1);

  while (!(m_input.keys & PAD_BUTTON_A) && (count > 0))
  {
    gxClearScreen((GXColor)BLACK);
    show_disclaimer(56);
    if (count%25 == 0) vis^=1;
    if (vis)
    {
      FONT_writeCenter("Press    button to continue.",24,0,640,366,(GXColor)SKY_BLUE);
      gxDrawTexture(button, 220, 366-24+(24-button->height)/2,  button->width, button->height,255);
    }
    gxDrawTexture(logo_left, (640-logo_left->width-logo_right->width -32)/2, 480-logo_left->height-24, logo_left->width, logo_left->height,255);
    gxDrawTexture(logo_right, (640-logo_left->width-logo_right->width -32)/2+logo_left->width+32, 480-logo_right->height-24, logo_right->width, logo_right->height,255);
    gxSetScreen();
    count--;
  }

  gxTextureClose(&button);
  gxTextureClose(&logo_left);
  gxTextureClose(&logo_right);

  if (count > 0)
  {
    ASND_Init();
    ASND_Pause(0);
    int voice = ASND_GetFirstUnusedVoice();
    ASND_SetVoice(voice,VOICE_MONO_16BIT,44100,0,(u8 *)button_select_pcm,button_select_pcm_size,200,200,NULL);
    GUI_FadeOut();
    ASND_Pause(1);
    ASND_End();
    return;
  }

  gxClearScreen((GXColor)BLACK);
  gx_texture *texture = gxTextureOpenPNG(Bg_intro_c1_png,0);
  if (texture)
  {
    gxDrawTexture(texture, (640-texture->width)/2, (480-texture->height)/2,  texture->width, texture->height,255);
    if (texture->data) free(texture->data);
    free(texture);
  }
  gxSetScreen();

  sleep (1);

  gxClearScreen((GXColor)WHITE);
  texture = gxTextureOpenPNG(Bg_intro_c2_png,0);
  if (texture)
  {
    gxDrawTexture(texture, (640-texture->width)/2, (480-texture->height)/2,  texture->width, texture->height,255);
    if (texture->data) free(texture->data);
    free(texture);
  }
  gxSetScreen();

  sleep (1);

  gxClearScreen((GXColor)BLACK);
  texture = gxTextureOpenPNG(Bg_intro_c3_png,0);
  if (texture)
  {
    gxDrawTexture(texture, (640-texture->width)/2, (480-texture->height)/2,  texture->width, texture->height,255);
    if (texture->data) free(texture->data);
    free(texture);
  }
  gxSetScreen();

  ASND_Pause(0);
  int voice = ASND_GetFirstUnusedVoice();
  ASND_SetVoice(voice,VOICE_MONO_16BIT,44100,0,(u8 *)intro_pcm,intro_pcm_size,200,200,NULL);
  sleep (2);
  ASND_Pause(1);
}
