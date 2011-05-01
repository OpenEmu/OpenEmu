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

#include "../mednafen.h"
#include "selblur.h"
#include <math.h>

#if 0
static INLINE void GetSourcePixel(const SelBlurImage *spec, int32 x, int32 y, unsigned int &r, unsigned int &g, unsigned int &b)
{
 uint32 pixel = spec->source[x + y * spec->source_pitch32];
 r = (pixel >> spec->red_shift) & 0xFF;
 g = (pixel >> spec->green_shift) & 0xFF;
 b = (pixel >> spec->blue_shift) & 0xFF;
}

static INLINE void SetDestPixel(const SelBlurImage *spec, int32 x, int32 y, const unsigned int r, const unsigned int g, const unsigned int b)
{
 spec->dest[x + y * spec->dest_pitch32] = (r << spec->red_shift) | (g << spec->green_shift) | (b << spec->blue_shift);
}
#endif

#define GetSourcePixel(_x, _y, _r, _g, _b) { uint32 mypixel = source[_x + _y * source_pitch32]; _r = (mypixel >> red_shift) & 0xFF; 	\
	_g = (mypixel >> green_shift) & 0xFF; _b = (mypixel >> blue_shift) & 0xFF; }

#define SetDestPixel(_x, _y, _r, _g, _b) dest[_x + _y * dest_pitch32] = (_r << red_shift) | (_g << green_shift) | (_b << blue_shift);


void MDFN_SelBlur(SelBlurImage *spec)
{
 unsigned int red_shift = spec->red_shift;
 unsigned int green_shift = spec->green_shift;
 unsigned int blue_shift = spec->blue_shift;
 uint32 *dest = spec->dest;
 uint32 *source = spec->source;
 uint32 source_pitch32 = spec->source_pitch32;
 uint32 dest_pitch32 = spec->dest_pitch32;
 uint8 r_thresh[512];
 uint8 g_thresh[512];
 uint8 b_thresh[512];

 for(int i = 0; i < 512; i++)
 {
  if((unsigned int)abs(i - 256) <= spec->red_threshold)
   r_thresh[i] = 0;
  else r_thresh[i] = 8;

  if((unsigned int)abs(i - 256) <= spec->green_threshold)
   g_thresh[i] = 0;
  else g_thresh[i] = 8;

  if((unsigned int)abs(i - 256) <= spec->blue_threshold)
   b_thresh[i] = 0;
  else b_thresh[i] = 8;
 }

 for(int32 y = 0; y < spec->height; y++)
 {
  int32 y_min, y_max;

  y_min = y - spec->radius;
  if(y_min < 0) y_min = 0;
  y_max = y + spec->radius;
  if(y_max >= spec->height) y_max = spec->height - 1;

  for(int32 x = 0; x < spec->width; x++)
  {
   unsigned int red, green, blue;
   unsigned int red_blur, green_blur, blue_blur;
   unsigned int blurdiv = 0;

   GetSourcePixel(x, y, red, green, blue);

   red_blur = green_blur = blue_blur = 0;

   for(int32 y_sub = y_min; y_sub <= y_max; y_sub++)
   {
    int32 x_magic, x_min, x_max;

    x_magic = spec->radius - abs(y - y_sub);

    x_min = x - x_magic;
    x_max = x + x_magic;

    if(x_max >= spec->width) x_max = spec->width - 1;
    if(x_min < 0) x_min = 0;

    for(int32 x_sub = x_min; x_sub <= x_max; x_sub++)
    {
     unsigned int red_other, green_other, blue_other;
     unsigned int smashing;

     GetSourcePixel(x_sub, y_sub, red_other, green_other, blue_other);

     smashing = r_thresh[256 + red_other - red] | g_thresh[256 + green_other - green] | b_thresh[256 + blue_other - blue];

     red_blur += red_other >> smashing;
     green_blur += green_other >> smashing;
     blue_blur += blue_other >> smashing;
     blurdiv += 1 >> smashing;
    }
   }
   SetDestPixel(x, y, red_blur / blurdiv , green_blur / blurdiv, blue_blur / blurdiv);
  }
 }

}
