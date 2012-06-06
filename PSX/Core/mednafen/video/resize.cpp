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

// I don't think this code works entirely correctly for reducing image size.
// Also, it won't interpolate quite correctly when resizing vertically and there are multiple horizontal resolutions(represented
// by LineWidths).

#include "video-common.h"
#include <math.h>

#define INT(x) ((int)(x))
#define FRACT(x) ((x) - floor(x))

#define READ_PIXEL(src_x, src_y, components)	\
{	\
   double source_components[2][2][4];	\
	\
   for(unsigned int xc = 0; xc < 2; xc++)	\
   {						\
    for(unsigned int yc = 0; yc < 2; yc++)	\
    {						\
     uint32 tmp;				\
     uint32 eff_x = (int)(src_x) + xc, eff_y = (int)(src_y) + yc;			\
     if(eff_x >= x_ip_limit) eff_x = x_ip_limit - 1;    \
     if(eff_y >= y_ip_limit) eff_y = y_ip_limit - 1;    \
     tmp = pixels[eff_x + eff_y * src->pitch32];	\
     source_components[xc][yc][0] = (tmp >> 0) & 0xFF;	\
     source_components[xc][yc][1] = (tmp >> 8) & 0xFF;	\
     source_components[xc][yc][2] = (tmp >> 16) & 0xFF;	\
     source_components[xc][yc][3] = (tmp >> 24) & 0xFF;	\
    }						\
   }						\
						\
   for(unsigned int i = 0; i < 4; i++)		\
   {						\
    components[i] += (1.0 - FRACT(src_y)) * (source_components[0][0][i] * (1.0 - FRACT(src_x)) + (source_components[1][0][i] * FRACT(src_x)));	\
    components[i] += FRACT(src_y) * (source_components[0][1][i] * (1.0 - FRACT(src_x)) + (source_components[1][1][i] * FRACT(src_x)));	\
   }	\
}

bool MDFN_ResizeSurface(const MDFN_Surface *src, const MDFN_Rect *src_rect, const MDFN_Rect *LineWidths, MDFN_Surface *dest, const MDFN_Rect *dest_rect)
{
 double src_x_inc, src_y_inc;
 double src_x, src_y;
 uint32 *pixels = src->pixels;
 uint32 x_ip_limit, y_ip_limit;

 //printf("%ld, %d %d, %d %d\n", (long)pixels, src_rect->w, dest_rect->w, src_rect->h, dest_rect->h);

 src_x_inc = (double)src_rect->w / dest_rect->w;
 src_y_inc = (double)src_rect->h / dest_rect->h;

 x_ip_limit = src_rect->x + src_rect->w;
 y_ip_limit = src_rect->y + src_rect->h;

 src_y = src_rect->y;

 for(int32 y = 0; y < dest_rect->h; y++)
 {
  uint32 *dest_pix = dest->pixels + dest_rect->x + (dest_rect->y + y) * dest->pitch32;
  bool ZeroDimensionSource = false;

  if(src_rect->h == 0)
  {
   ZeroDimensionSource = true;
  }
  else
  {
   if(LineWidths)
   {
    src_x = LineWidths[INT(src_y)].x;
    src_x_inc = (double)LineWidths[INT(src_y)].w / dest_rect->w;
    x_ip_limit = LineWidths[INT(src_y)].x + LineWidths[INT(src_y)].w;

    if(LineWidths[INT(src_y)].w == 0)
     ZeroDimensionSource = true;
   }
   else
   {
    src_x = src_rect->x;

    if(src_rect->w == 0)
     ZeroDimensionSource = true;
   }
  }

  if(ZeroDimensionSource)
  {
   puts("All Hail Lelouch!");

   for(int32 x = 0; x < dest_rect->w; x++)
    dest_pix[x] = 0;
  }
  else for(int32 x = 0; x < dest_rect->w; x++)
  {
   double components[4] = {0, 0, 0, 0};
   double div_fib = 1;

   //printf("%d %d, %d %d\n", (int)src_x, (int)src_y, x_ip_limit, y_ip_limit);

   READ_PIXEL(src_x, src_y, components);

   if(INT(src_x_inc) > 1)
   {
    for(double subx = 1; subx < (INT(src_x_inc) - 1); subx++)
    {
     READ_PIXEL(INT(src_x) + subx, INT(src_y), components);
     div_fib++;
    }

    READ_PIXEL(src_x + src_x_inc - 1, src_y, components);
    div_fib++;
   }

   if(INT(src_y_inc) > 1)
   {
    for(double suby = 1; suby < (INT(src_y_inc) - 1); suby++)
    {
     READ_PIXEL(INT(src_x), INT(src_y) + suby, components);
     div_fib++;
    }

    READ_PIXEL(src_x, src_y + src_y_inc - 1, components);
    div_fib++;
   }

   for(unsigned int i = 0; i < 4; i++)
   {
    components[i] /= div_fib;
    if(components[i] < 0) components[i] = 0;
    if(components[i] > 255) components[i] = 255;
   }

   dest_pix[x] = ((int)components[0] << 0) | ((int)components[1] << 8) | ((int)components[2] << 16) | ((int)components[3] << 24);
   src_x += src_x_inc;
  }
  src_y += src_y_inc;
 }


 return(TRUE);
}
