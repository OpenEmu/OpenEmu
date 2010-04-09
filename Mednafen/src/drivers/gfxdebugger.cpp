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
#include "gfxdebugger.h"
#include "debugger.h"

static bool IsActive = 0;
static const char *LayerNames[16];
static int LayerScanline[16] = { 0 };
static int LayerScroll[16] = { 0 };
static int LayerPBN[16] = { 0 };
static int LayerCount = 0;
static int CurLayer = 0;

static void RedoSGD(bool instant = 0)
{
 if(IsActive)
  CurGame->Debugger->SetGraphicsDecode(instant ? 0xB00B13 : LayerScanline[CurLayer], CurLayer, 128, 128, 0, LayerScroll[CurLayer], LayerPBN[CurLayer]);
 else
  CurGame->Debugger->SetGraphicsDecode(0, 0, 0, 0, 0, 0, 0);
}

// Call this function from either thread.
void GfxDebugger_SetActive(bool newia)
{
 if(CurGame->Debugger)
 {
  LockGameMutex(1);

  IsActive = newia;

  if(IsActive && !LayerCount)
  {
   LayerCount = 0;

   int clen;
   const char *lnp = CurGame->LayerNames;
   while((clen = strlen(lnp)))
   {
    LayerNames[LayerCount] = lnp;
    LayerCount++;
    lnp += clen + 1;
   }
  }

  RedoSGD();
  LockGameMutex(0);
 }
}

#define MK_COLOR_A(r,g,b,a) ( ((a)<<surface->format->Ashift) | ((r)<<surface->format->Rshift) | ((g) << surface->format->Gshift) | ((b) << surface->format->Bshift))

// Call this function from the main thread
void GfxDebugger_Draw(SDL_Surface *surface, const SDL_Rect *rect, const SDL_Rect *screen_rect)
{
 if(!IsActive) return;

 uint32 * pixels = (uint32 *)surface->pixels;
 uint32 pitch32 = surface->pitch >> 2;
 bool ism;

 LockGameMutex(1);

 ism = InSteppingMode;

 if(ism)
 {
  RedoSGD(TRUE);
 }

 uint32 *src_pixels = CurGame->Debugger->GetGraphicsDecodeBuffer(); // Returns NULL if graphics decoding is off.

 if(!src_pixels)
 {
  LockGameMutex(0);
  return;
 }

 for(unsigned int y = 0; y < 128; y++)
 {
  uint32 *row = pixels + ((rect->w - 256) / 2) + y * pitch32 * 2;
  for(unsigned int x = 0; x < 128; x++)
  {
   //printf("%d %d %d\n", y, x, pixels);
   row[x*2] = row[x*2 + 1] = row[pitch32 + x*2] = row[pitch32 + x*2 + 1] = src_pixels[x + y * 128 * 3];
   //row[x] = MK_COLOR_A(0, 0, 0, 0xc0);
   //row[x] = MK_COLOR_A(0x00, 0x00, 0x00, 0x7F);
  }
 }


 // Draw layer name
 {
  for(unsigned int y = 256; y < 256 + 18; y++)
   for(unsigned int x = 0; x < rect->w; x++)
   {
    pixels[y * pitch32 + x] = MK_COLOR_A(0x00, 0x00, 0x00, 0xC0);
   }
  char buf[256];

  if(ism)
   snprintf(buf, 256, "%s, PBN: %d, Scroll: %d, Instant", LayerNames[CurLayer], LayerPBN[CurLayer], LayerScroll[CurLayer]);
  else
   snprintf(buf, 256, "%s, PBN: %d, Scroll: %d, Line: %d", LayerNames[CurLayer], LayerPBN[CurLayer], LayerScroll[CurLayer], LayerScanline[CurLayer]);
  DrawTextTransShadow(pixels + 256 * pitch32, surface->pitch, rect->w, (UTF8*)buf, MK_COLOR_A(0xF0, 0xF0, 0xF0, 0xFF), MK_COLOR_A(0, 0, 0, 0xFF), 1, FALSE);
 }

 int mousex, mousey;
 SDL_GetMouseState(&mousex, &mousey);
 int vx, vy;

 vx = (mousex - screen_rect->x) * rect->w / screen_rect->w - ((rect->w - 256) / 2);
 vy = (mousey - screen_rect->y) * rect->h / screen_rect->h;

 vx /= 2;
 vy /= 2;

 if(vx < 128 && vy < 128 && vx >= 0 && vy >= 0)
 {
  if(src_pixels[vx + vy * 128 * 3] & surface->format->Amask)
  {
   for(unsigned int y = 278; y < 278 + 18; y++)
    for(unsigned int x = 0; x < rect->w; x++)
    {
     pixels[y * pitch32 + x] = MK_COLOR_A(0x00, 0x00, 0x00, 0xC0);
    }
   char buf[256];

   snprintf(buf, 256, "Tile: %08x, Address: %08x", src_pixels[128 + vx + vy * 128 * 3], src_pixels[256 + vx + vy * 128 * 3]);

   DrawTextTransShadow(pixels + 278 * pitch32, surface->pitch, rect->w, (UTF8*)buf, MK_COLOR_A(0xF0, 0xF0, 0xF0, 0xFF), MK_COLOR_A(0, 0, 0, 0xFF), 1, FALSE);  
  }
 }

 LockGameMutex(0);
}

// Call this from the main thread
int GfxDebugger_Event(const SDL_Event *event)
{
 switch(event->type)
 {
  case SDL_KEYDOWN:
	switch(event->key.keysym.sym)
	{
	 default: break;

	 case SDLK_MINUS:
		       LockGameMutex(1);
		       if(LayerScanline[CurLayer])
		       {
			LayerScanline[CurLayer]--;
			RedoSGD();
		       }
		       LockGameMutex(0);
		       break;
	 case SDLK_EQUALS:
		       LockGameMutex(1);
		       LayerScanline[CurLayer]++;
		       RedoSGD();
		       LockGameMutex(0);
		       break;
         case SDLK_UP: LockGameMutex(1);
		       if(LayerScroll[CurLayer])
		       {
                        LayerScroll[CurLayer]--;
                        RedoSGD();
		       }
                       LockGameMutex(0);
                       break;

         case SDLK_PAGEUP:
                         LockGameMutex(1);
                         LayerScroll[CurLayer] -= 8;
			 if(LayerScroll[CurLayer] < 0)
			  LayerScroll[CurLayer] = 0;
                         RedoSGD();
                         LockGameMutex(0);
                         break;

	 case SDLK_PAGEDOWN:
			 LockGameMutex(1);
			 LayerScroll[CurLayer] += 8;
			 RedoSGD();
			 LockGameMutex(0);
			 break;
	 case SDLK_DOWN: LockGameMutex(1);
			 LayerScroll[CurLayer]++;
			 RedoSGD();
			 LockGameMutex(0);
			 break;
	 case SDLK_LEFT: LockGameMutex(1);
			 CurLayer = (CurLayer - 1);

			 if(CurLayer < 0) CurLayer = LayerCount - 1;

			 RedoSGD();
			 LockGameMutex(0);
			 break;
	 case SDLK_RIGHT: LockGameMutex(1);
			  CurLayer = (CurLayer + 1) % LayerCount;
			  RedoSGD();
			  LockGameMutex(0);
			  break;


	 case SDLK_COMMA: LockGameMutex(1);
			  if(LayerPBN[CurLayer] >= 0)
			   LayerPBN[CurLayer]--;
			  RedoSGD();
			  LockGameMutex(0);
			  break;

	 case SDLK_PERIOD:
			  LockGameMutex(1);
			  LayerPBN[CurLayer]++;
			  RedoSGD();
			  LockGameMutex(0);
			  break;
	}
	break;
 }
 return(1);
}

