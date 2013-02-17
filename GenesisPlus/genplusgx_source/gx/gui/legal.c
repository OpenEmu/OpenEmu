/****************************************************************************
 *  legal.c
 *
 *  Genesis Plus GX Disclaimer
 *
 *  Copyright Eke-Eke (2009-2012)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#include "shared.h"
#include "font.h"
#include "gui.h"

extern const u8 Bg_intro_c1_png[];
extern const u8 Bg_intro_c2_png[];
extern const u8 Bg_intro_c3_png[];
extern const u8 Bg_intro_c4_png[];

/* 
 * This is the legal stuff - which must be shown at program startup 
 * Any derivative work MUST include the same textual output.
 *
 */

static void show_disclaimer(int ypos)
{
  FONT_writeCenter ("DISCLAIMER",22,0,640,ypos,(GXColor)WHITE);
  ypos += 32;
  FONT_writeCenter ("This is a free software, and you are welcome",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("to redistribute it under the conditions of the",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("license that you should have received with this",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("program. You may not sell, lease, rent or generally",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("use this software in any commercial product or activity.",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("Authors can not be held responsible for any damage or",20,0,640,ypos,(GXColor)WHITE);
  ypos += 20;
  FONT_writeCenter ("or dysfunction that could occur while using this port.",20,0,640,ypos,(GXColor)WHITE);
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
  gx_texture *logo = gxTextureOpenPNG(Bg_intro_c4_png,0);

  gxClearScreen((GXColor)BLACK);
  show_disclaimer(56);
  gxDrawTexture(logo, (640-logo->width)/2, 480-24-logo->height, logo->width, logo->height,255);
  gxSetScreen();
  sleep(1);

  while (!m_input.keys && count)
  {
    gxClearScreen((GXColor)BLACK);
    show_disclaimer(56);
    if (count%25 == 0) vis^=1;
    if (vis)
    {
      FONT_writeCenter("Press    button to continue.",24,0,640,366,(GXColor)SKY_BLUE);
      gxDrawTexture(button, 220, 366-24+(24-button->height)/2,  button->width, button->height,255);
    }
    gxDrawTexture(logo, (640-logo->width)/2, 480-24-logo->height, logo->width, logo->height,255);
    gxSetScreen();
    count--;
  }

  gxTextureClose(&button);
  gxTextureClose(&logo);

  if (count > 0)
  {
    ASND_Pause(0);
    int voice = ASND_GetFirstUnusedVoice();
    ASND_SetVoice(voice,VOICE_MONO_16BIT,44100,0,(u8 *)button_select_pcm,button_select_pcm_size,200,200,NULL);
    GUI_FadeOut();
    ASND_Pause(1);
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
