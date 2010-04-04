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

#include "main.h"
#include "video.h"

#define MK_COLOR_A(surface, r,g,b,a) ( ((a)<<surface->format->Ashift) | ((r)<<surface->format->Rshift) | ((g) << surface->format->Gshift) |((b) << surface->format->Bshift))

static uint32 VirtualTime[128], DrawnTime[128], BlittedTime[128];
static uint32 VirtualIndex, DrawnIndex, BlittedIndex;
static SDL_Surface *FPSSurface = NULL;
static SDL_Rect FPSRect;

void FPS_Init(void)
{
 VirtualIndex = 0;
 DrawnIndex = 0;
 BlittedIndex = 0;

 memset(VirtualTime, 0, sizeof(VirtualTime));
 memset(DrawnTime, 0, sizeof(DrawnTime));
 memset(BlittedTime, 0, sizeof(BlittedTime));
}

void FPS_IncVirtual(void)
{
 VirtualTime[VirtualIndex] = SDL_GetTicks();
 VirtualIndex = (VirtualIndex + 1) & 127;
}

void FPS_IncDrawn(void)
{
 DrawnTime[DrawnIndex] = SDL_GetTicks();
 DrawnIndex = (DrawnIndex + 1) & 127;
}

void FPS_IncBlitted(void)
{
 BlittedTime[BlittedIndex] = SDL_GetTicks();
 BlittedIndex = (BlittedIndex + 1) & 127;
}

static bool isactive = 0;
bool FPS_IsActive(void)
{
 return(isactive);
}

void FPS_ToggleView(void)
{
 isactive ^= 1;
}

void FPS_Draw(SDL_Surface *screen, int rs, int gs, int bs, int as)
{
 if(!isactive) 
 {
  if(FPSSurface)
  {
   SDL_FreeSurface(FPSSurface);
   FPSSurface = NULL;
  }
  return;
 }

 if(!FPSSurface)
 {
  FPSSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, 9 * 5, 3 * 7, 32, 0xFF << rs, 0xFF << gs, 0xFF << bs, 0xFF << as);
  FPSRect.w = 9 * 5;
  FPSRect.h = 3 * 7;
  FPSRect.x = FPSRect.y = 0;
 }

 uint32 curtime = SDL_GetTicks();
 uint32 vt_frames_drawn = 0, dt_frames_drawn = 0, bt_frames_drawn = 0;
 uint32 vt_mintime, dt_mintime, bt_mintime;
 char virtfps[64], drawnfps[64], blitfps[64];

 vt_mintime = dt_mintime = bt_mintime = curtime;

 for(int x = 0; x < 128; x++)
 {
  uint32 vt = VirtualTime[x];
  uint32 dt = DrawnTime[x];
  uint32 bt = BlittedTime[x];

  if(vt >= (curtime - 1000))
  {
   if(vt < vt_mintime) vt_mintime = vt;
   vt_frames_drawn++;
  }

  if(dt >= (curtime - 1000))
  {
   if(dt < dt_mintime) dt_mintime = dt;
   dt_frames_drawn++;
  }

  if(bt >= (curtime - 1000))
  {
   if(bt < bt_mintime) bt_mintime = bt;
   bt_frames_drawn++;
  }
 }

 if(curtime - vt_mintime)
  snprintf(virtfps, 64, "%f", (double)vt_frames_drawn * 1000 / (curtime - vt_mintime));
 else
  snprintf(virtfps, 64, "?");

 if(curtime - dt_mintime)
  snprintf(drawnfps, 64, "%f", (double)dt_frames_drawn * 1000 / (curtime - dt_mintime));
 else
  snprintf(drawnfps, 64, "?");

 if(curtime - bt_mintime)
  snprintf(blitfps, 64, "%f", (double)bt_frames_drawn * 1000 / (curtime - bt_mintime));
 else
  snprintf(blitfps, 64, "?");

 SDL_FillRect(FPSSurface, NULL, MK_COLOR_A(FPSSurface, 0, 0, 0, 0x80));
 DrawTextTrans((uint32 *)FPSSurface->pixels, FPSSurface->pitch, FPSSurface->w, (UTF8*)virtfps, MK_COLOR_A(FPSSurface, 0xFF, 0xFF, 0xFF, 0xFF), FALSE, TRUE);
 DrawTextTrans((uint32 *)FPSSurface->pixels + 7 * (FPSSurface->pitch >> 2), FPSSurface->pitch, FPSSurface->w, (UTF8*)drawnfps, MK_COLOR_A(FPSSurface, 0xFF, 0xFF, 0xFF, 0xFF), FALSE, TRUE);
 DrawTextTrans((uint32 *)FPSSurface->pixels + 7 * 2 * (FPSSurface->pitch >> 2), FPSSurface->pitch, FPSSurface->w, (UTF8*)blitfps, MK_COLOR_A(FPSSurface, 0xFF, 0xFF, 0xFF, 0xFF), FALSE, TRUE);

 SDL_Rect drect;
 drect.x = 0;
 drect.y = 0;
 drect.w = FPSRect.w;
 drect.h = FPSRect.h;

 BlitRaw(FPSSurface, &FPSRect, &drect);

}
