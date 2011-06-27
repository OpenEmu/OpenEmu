/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

// Code for drawing save states and mooovies
// all functions should be called from the main thread
#include "main.h"
#include "video.h"
#include <string.h>
#include <trio/trio.h>

#define MK_COLOR_A(tmp_surface, r,g,b,a) ( ((a)<<tmp_surface->format->Ashift) | ((r)<<tmp_surface->format->Rshift) | ((g) << tmp_surface->format->Gshift) | ((b) << tmp_surface->format->Bshift))

static SDL_Surface *PreviewSurface = NULL, *TextSurface = NULL;
static SDL_Rect PreviewRect, TextRect;

static StateStatusStruct *StateStatus;
static uint32 StateShow;
static bool IsMovie;

void DrawStateMovieRow(SDL_Surface *surface, int *nstatus, int cur, int recently_saved, uint8 *text)
{
 uint32 *XBuf = (uint32 *)surface->pixels;
 uint32 pitch32 = surface->pitch >> 2;
 int x, y;

 for(y=0; y<40; y++)
  for(x=0;x<230;x++)
   XBuf[x + y * pitch32] = MK_COLOR_A(surface, 0x00, 0x00, 0x00, 170);

 // nstatus
 for(int i = 1; i < 11; i++)
 {
  char stringie[2];
  uint32 bordercol;

  if(cur == (i % 10))
   bordercol = MK_COLOR_A(surface, 0x60, 0x20, 0xb0, 0xFF);
  else
   bordercol = MK_COLOR_A(surface, 0, 0, 0, 0xFF);

  stringie[0] = '0' + (i % 10);
  stringie[1] = 0;

  //if(nstatus[i % 10])
  {
   uint32 rect_bg_color = MK_COLOR_A(surface, 0x00, 0x00, 0x00, 0xFF);

   if(nstatus[i % 10])
   {
    rect_bg_color = MK_COLOR_A(surface, 0x00, 0x38, 0x28, 0xFF);

    if(recently_saved == (i % 10))
     rect_bg_color = MK_COLOR_A(surface, 0x48, 0x00, 0x34, 0xFF);
   }
   MDFN_DrawRectangleFill(XBuf, pitch32, (i - 1) * 23, 0, bordercol, rect_bg_color, 23, 18 + 1);
  }
  //else
  // MDFN_DrawRectangle(XBuf, pitch32, (i - 1) * 23, 0, bordercol, 23, 18 + 1);

  DrawTextTransShadow(XBuf + (i - 1) * 23 + 7, pitch32 << 2, 230, (UTF8*)stringie, MK_COLOR_A(surface, 0xE0, 0xFF, 0xE0, 0xFF), MK_COLOR_A(surface, 0x00, 0x00, 0x00, 0xFF), FALSE);
 }
 DrawTextTransShadow(XBuf + 20 * pitch32, pitch32 << 2, 230, text, MK_COLOR_A(surface, 0xE0, 0xFF, 0xE0, 0xFF), MK_COLOR_A(surface, 0x00, 0x00, 0x00, 0xFF), TRUE);
}


bool SaveStatesActive(void)
{
 return(StateStatus);
}

static void SSCleanup(void)
{
  if(PreviewSurface)
  {
   SDL_FreeSurface(PreviewSurface);
   PreviewSurface = NULL;
  }
  if(TextSurface)
  {
   SDL_FreeSurface(TextSurface);
   TextSurface = NULL;
  }
  if(StateStatus)
  {
   if(StateStatus->gfx)
    free(StateStatus->gfx);
   free(StateStatus);
   StateStatus = NULL;
  }
}

void DrawSaveStates(SDL_Surface *screen, double exs, double eys, int rs, int gs, int bs, int as)
{
 if(StateShow < MDFND_GetTime())
 {
  SSCleanup();
 }

 if(StateStatus)
 {
  if(!PreviewSurface)
  {
   PreviewSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, StateStatus->w + 2, StateStatus->h + 2, 32, 0xFF << rs, 0xFF << gs, 0xFF << bs, 0xFF << as);

   if(!PreviewSurface)
   {
    printf("Iyee: %d %d\n", StateStatus->w, StateStatus->h);
    return;
   }

   PreviewRect.x = PreviewRect.y = 0;
   PreviewRect.w = StateStatus->w + 2;
   PreviewRect.h = StateStatus->h + 2;
   MDFN_DrawRectangleAlpha((uint32*)PreviewSurface->pixels, PreviewSurface->pitch >> 2, 0, 0, MK_COLOR_A(PreviewSurface, 0x00, 0x00, 0x9F, 0xFF), MK_COLOR_A(PreviewSurface, 0x00, 0x00, 0x00, 0x80), StateStatus->w + 2, StateStatus->h + 2);

   uint32 *psp = (uint32*)PreviewSurface->pixels;

   psp += PreviewSurface->pitch >> 2;
   psp++;

   if(StateStatus->gfx)
   {
    for(uint32 y = 0; y < StateStatus->h; y++)
    {
     uint8 *src_row = StateStatus->gfx + y * StateStatus->w * 3;

     for(uint32 x = 0; x < StateStatus->w; x++)
     {
      psp[x] = MK_COLOR_A(PreviewSurface, src_row[0], src_row[1], src_row[2], 0xFF);
      src_row += 3;
     }
     psp += PreviewSurface->pitch >> 2;
    }
   }

   if(!TextSurface)
   {
    TextSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, 230, 40, 32, 0xFF << rs, 0xFF << gs, 0xFF << bs, 0xFF << as);
    SDL_SetColorKey(TextSurface, SDL_SRCCOLORKEY, 0);
    SDL_SetAlpha(TextSurface, SDL_SRCALPHA, 0);

    TextRect.x = TextRect.y = 0;
    TextRect.w = 230;
    TextRect.h = 40;
   }

   if(IsMovie)
   {
    UTF8 text[256];

    if(StateStatus->current_movie > 0)
     trio_snprintf((char *)text, 256, _("-recording movie %d-"), StateStatus->current_movie-1);
    else if (StateStatus->current_movie < 0)
     trio_snprintf((char *)text, 256, _("-playing movie %d-"),-1 - StateStatus->current_movie);
    else
     trio_snprintf((char *)text, 256, _("-select movie-"));
 
    DrawStateMovieRow(TextSurface, StateStatus->status, StateStatus->current, StateStatus->recently_saved, text);
   }
   else
    DrawStateMovieRow(TextSurface, StateStatus->status, StateStatus->current, StateStatus->recently_saved, (UTF8 *)_("-select state-"));
  } 
 } // end if(StateStatus)

 if(PreviewSurface)
 {
  SDL_Rect tdrect, drect;

  int meow = ((screen->w / CurGame->nominal_width) + 1) / 2;
  if(!meow) meow = 1;

  tdrect.w = TextRect.w * meow;
  tdrect.h = TextRect.h * meow;
  tdrect.x = (screen->w - tdrect.w) / 2;
  tdrect.y = screen->h - tdrect.h;

  BlitRaw(TextSurface, &TextRect, &tdrect);

  drect.w = PreviewRect.w * meow;
  drect.h = PreviewRect.h * meow;
  drect.x = (screen->w - drect.w) / 2;
  drect.y = screen->h - drect.h - tdrect.h - 4;

  BlitRaw(PreviewSurface, &PreviewRect, &drect);

 }

}

void MT_SetStateStatus(StateStatusStruct *status)
{
 SSCleanup();

 IsMovie = FALSE;
 StateStatus = status;

 if(status)
  StateShow = MDFND_GetTime() + MDFN_GetSettingUI("osd.state_display_time");
 else
  StateShow = 0;
}

void MT_SetMovieStatus(StateStatusStruct *status)
{
 SSCleanup();

 IsMovie = TRUE;
 StateStatus = status;

 if(status)
  StateShow = MDFND_GetTime() + MDFN_GetSettingUI("osd.state_display_time");
 else
  StateShow = 0;
}

