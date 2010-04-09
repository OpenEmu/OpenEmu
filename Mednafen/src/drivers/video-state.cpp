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
static StateStatusStruct *StateStatus, *MovieStatus;
static uint32 StateShow, MovieShow;

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
   bordercol = MK_COLOR_A(surface, 0x00, 0x00, 0x00, 0xFF);

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


void DrawSaveStates(SDL_Surface *screen, double exs, double eys, int rs, int gs, int bs, int as)
{
 StateStatusStruct *tmps;

 if(StateShow < MDFND_GetTime())
 {
  if(PreviewSurface)
  {
   SDL_FreeSurface(PreviewSurface);
   PreviewSurface = NULL;
  }
  if(StateStatus)
  {
   if(StateStatus->gfx)
    free(StateStatus->gfx);
   free(StateStatus);
   StateStatus = NULL;
  }
 }

 if(MovieShow < MDFND_GetTime())
 {
  if(PreviewSurface)
  {
   SDL_FreeSurface(PreviewSurface);
   PreviewSurface = NULL;
  }
  if(MovieStatus)
  {
   if(MovieStatus->gfx)
    free(MovieStatus->gfx);
   free(MovieStatus);
   MovieStatus = NULL;
  }
 }

 tmps = MovieStatus;
 if(StateStatus)
  tmps = StateStatus;

 if(tmps)
 {
  if(PreviewSurface)
  {
   SDL_FreeSurface(PreviewSurface);
   PreviewSurface = NULL;
  }
  PreviewSurface = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, tmps->w + 2, tmps->h + 2, 32, 0xFF << rs, 0xFF << gs, 0xFF << bs, 0xFF << as);
  PreviewRect.x = PreviewRect.y = 0;
  PreviewRect.w = tmps->w + 2;
  PreviewRect.h = tmps->h + 2;
  MDFN_DrawRectangleAlpha((uint32*)PreviewSurface->pixels, PreviewSurface->pitch >> 2, 0, 0, MK_COLOR_A(PreviewSurface, 0x00, 0x00, 0x9F, 0xFF), tmps->w + 2, tmps->h + 2);

  uint32 *psp = (uint32*)PreviewSurface->pixels;

  psp += PreviewSurface->pitch >> 2;
  psp++;

  if(tmps->gfx)
   for(int y = 0; y < tmps->h; y++)
   {
    memcpy(psp, tmps->gfx + y * tmps->pitch, tmps->w * sizeof(uint32));
    psp += PreviewSurface->pitch >> 2;
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
  if(tmps == MovieStatus)
  {
   UTF8 text[256];

   if(tmps->current_movie > 0)
    trio_snprintf((char *)text, 256, _("-recording movie %d-"), tmps->current_movie-1);
   else if (tmps->current_movie < 0)
    trio_snprintf((char *)text, 256, _("-playing movie %d-"),-1 - tmps->current_movie);
   else
    trio_snprintf((char *)text, 256, _("-select movie-"));

   DrawStateMovieRow(TextSurface, tmps->status, tmps->current, tmps->recently_saved, text);
  }
  else
   DrawStateMovieRow(TextSurface, tmps->status, tmps->current, tmps->recently_saved, (UTF8 *)_("-select state-"));
 } 

 if(PreviewSurface)
 {
  SDL_Rect tdrect, drect;

  int meow = ((screen->w / CurGame->width) + 1) / 2;
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
 if(StateStatus)
 {
  if(StateStatus->gfx)
   free(StateStatus->gfx);
  free(StateStatus);
 }
 StateStatus = status;

 if(status)
  StateShow = MDFND_GetTime() + MDFN_GetSettingUI("osd.state_display_time");
 else
  StateShow = 0;
}

void MT_SetMovieStatus(StateStatusStruct *status)
{
 if(MovieStatus)
 {
  if(MovieStatus->gfx)
   free(MovieStatus->gfx);
  free(MovieStatus);
 }
 MovieStatus = status;

 if(status)
  MovieShow = MDFND_GetTime() + MDFN_GetSettingUI("osd.state_display_time");
 else
  MovieShow = 0;
}

