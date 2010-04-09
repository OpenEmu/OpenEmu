#include <math.h>
#include "main.h"
#include "video.h"
#include "nongl.h"
#include "nnx.h"

void BlitRotated(SDL_Surface *src_surface, const SDL_Rect *src_rect, const SDL_Rect *original_src_rect, const SDL_Rect *dest_rect, SDL_Surface *dest_surface, int scanlines, int rotated);

void BlitNonGL(SDL_Surface *src_surface, const SDL_Rect *src_rect, const SDL_Rect *original_src_rect, const SDL_Rect *dest_rect, SDL_Surface *dest_surface, int scanlines, int rotated)
{
 if(rotated != MDFN_ROTATE0)
 {
  BlitRotated(src_surface, src_rect, original_src_rect, dest_rect, dest_surface, scanlines, rotated);
  return;
 }

 SDL_Rect sr, dr, o_sr;

 sr = *src_rect;
 o_sr = *original_src_rect;
 dr = *dest_rect;

 if(dr.x < 0) 
  dr.x = 0;

 if(dr.y < 0) 
  dr.y = 0;

 if((dr.w + dr.x) > dest_surface->w)
 {
  dr.w = dest_surface->w - dr.x;
 }

 if((dr.h + dr.y) > dest_surface->h)
 {
  dr.h = dest_surface->h - dr.y;
 }

 if(sr.w == dr.w && sr.h == dr.h)
 {
  SDL_BlitSurface(src_surface, &sr, dest_surface, &dr);
  return;
 }
 //printf("%d\n", dr.x);

 if(SDL_MUSTLOCK(src_surface))
  SDL_LockSurface(src_surface);
 if(SDL_MUSTLOCK(dest_surface))
  SDL_LockSurface(dest_surface);

 if(!scanlines && sr.w * 2 == dr.w && sr.h * 2 == dr.h)
  nnx(2, src_surface, &sr, dest_surface, &dr);
 else if(!scanlines && sr.w * 3 == dr.w && sr.h * 3 == dr.h)
  nnx(3, src_surface, &sr, dest_surface, &dr);
 else if(!scanlines && sr.w * 4 == dr.w && sr.h * 4 == dr.h)
  nnx(4, src_surface, &sr, dest_surface, &dr);
 else if(scanlines)
 {
  uint32 sl_mult = 65536 - 65536 / scanlines;
  uint32 src_x = sr.x * 65536;
  uint32 src_x_inc = 65536 * sr.w / dr.w;
  uint32 src_y = sr.y * 65536;
  uint32 src_y_inc = 65536 * sr.h / dr.h;

  uint32 sl_y = o_sr.y * 65536 * 2;
  uint32 sl_y_inc = 65536 * o_sr.h / dr.h * 2;

  for(unsigned int y = 0; y < dr.h; y++)
  {
   uint32 *dest_row_ptr = (uint32 *)dest_surface->pixels + dr.x + ((dr.y + y) * (dest_surface->pitch >> 2));
   uint32 *src_row_ptr = (uint32 *)src_surface->pixels + (src_y >> 16) * (src_surface->pitch >> 2);

   src_x = sr.x << 16;

   if(!(sl_y & 0x10000))
   {
    for(unsigned int x = 0; x < dr.w; x++)
    {
     uint32 pixel = src_row_ptr[(src_x >> 16)];
     uint32 p[4];

     p[0] = pixel & 0xFF;
     p[1] = (pixel >> 8) & 0xFF;
     p[2] = (pixel >> 16) & 0xFF;
     p[3] = (pixel >> 24) & 0xFF;

     p[0] = (p[0] * sl_mult) >> 16;
     p[1] = (p[1] * sl_mult) >> 16;
     p[2] = (p[2] * sl_mult) >> 16;
     p[3] = (p[3] * sl_mult) >> 16;
     
     dest_row_ptr[x] = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
     src_x += src_x_inc;
    }
   }
   else
    for(unsigned int x = 0; x < dr.w; x++)
    {
     dest_row_ptr[x] = src_row_ptr[(src_x >> 16)];
     src_x += src_x_inc;
    }
   src_y += src_y_inc;
   sl_y += sl_y_inc;
  }
 }
 else
 {
  uint32 src_x = sr.x * 65536;
  uint32 src_x_inc = 65536 * sr.w / dr.w;
  uint32 src_y = sr.y * 65536;
  uint32 src_y_inc = 65536 * sr.h / dr.h;

  for(unsigned int y = 0; y < dr.h; y++)
  {
   uint32 *dest_row_ptr = (uint32 *)dest_surface->pixels + dr.x + ((dr.y + y) * (dest_surface->pitch >> 2));
   uint32 *src_row_ptr = (uint32 *)src_surface->pixels + (src_y >> 16) * (src_surface->pitch >> 2);

   src_x = sr.x << 16;

   for(unsigned int x = 0; x < dr.w; x++)
   {
    dest_row_ptr[x] = src_row_ptr[(src_x >> 16)];
    src_x += src_x_inc;
   }
   src_y += src_y_inc;
  }
 }

 if(SDL_MUSTLOCK(src_surface))
  SDL_UnlockSurface(src_surface);
 if(SDL_MUSTLOCK(dest_surface))
  SDL_UnlockSurface(dest_surface);
}


// This function handles rotation of either 90 or 270 degrees
void BlitRotated(SDL_Surface *src_surface, const SDL_Rect *src_rect, const SDL_Rect *original_src_rect, const SDL_Rect *dest_rect, SDL_Surface *dest_surface, int scanlines, int rotated)
{
 SDL_Rect sr, dr, o_sr;

 sr = *src_rect;
 o_sr = *original_src_rect;
 dr = *dest_rect;

 if(dr.x < 0)
  dr.x = 0;

 if(dr.y < 0)
  dr.y = 0;

 //printf("%d:%d %d:%d\n", dr.w, dest_surface->w, dr.h, dest_surface->h);

 if((dr.w + dr.x) > dest_surface->w)
 {
  dr.w = dest_surface->w - dr.x;
 }

 if((dr.h + dr.y) > dest_surface->h)
 {
  dr.h = dest_surface->h - dr.y;
 }

 if(SDL_MUSTLOCK(src_surface))
  SDL_LockSurface(src_surface);
 if(SDL_MUSTLOCK(dest_surface))
  SDL_LockSurface(dest_surface);

 {
  int32 src_x, src_x_init;
  int32 src_x_inc;
  int32 src_y;
  int32 src_y_inc;
  int32 src_pitch32 = (src_surface->pitch >> 2);

  if(rotated == MDFN_ROTATE90)
  {
   src_x_init = sr.x * 65536;
   src_x_inc = 65536 * sr.h / dr.w;
   src_y_inc = -65536 * sr.w / dr.h;
   src_y = (sr.y + sr.w) * 65536 + src_y_inc;
  }
  else //if(rotated == MDFN_ROTATE270)
  {
   src_x_inc = -65536 * sr.h / dr.w;
   src_x_init = (sr.x + sr.h) * 65536 + src_x_inc;

   src_y = sr.y * 65536;
   src_y_inc = 65536 * sr.w / dr.h;
  }

  //printf("%d\n", src_pitch32);

  if(src_pitch32 == 256) // Yay, optimization
   for(unsigned int y = 0; y < dr.h; y++)
   {
    uint32 *dest_row_ptr = (uint32 *)dest_surface->pixels + dr.x + ((dr.y + y) * (dest_surface->pitch >> 2));
    uint32 *src_col_ptr = (uint32*)src_surface->pixels + (src_y >> 16);
    src_x = src_x_init;
    for(unsigned int x = 0; x < dr.w; x++)
    {
     dest_row_ptr[x] = src_col_ptr[(src_x >> 8) &~0xFF];
     src_x += src_x_inc;
    }
    src_y += src_y_inc;
   }  
  else
   for(unsigned int y = 0; y < dr.h; y++)
   {
    uint32 *dest_row_ptr = (uint32 *)dest_surface->pixels + dr.x + ((dr.y + y) * (dest_surface->pitch >> 2));
    uint32 *src_col_ptr = (uint32*)src_surface->pixels + (src_y >> 16);
    src_x = src_x_init;
    for(unsigned int x = 0; x < dr.w; x++)
    {
     dest_row_ptr[x] = src_col_ptr[(src_x >> 16) * src_pitch32];
     src_x += src_x_inc;
    }
    src_y += src_y_inc;
   }
 }

 if(SDL_MUSTLOCK(src_surface))
  SDL_UnlockSurface(src_surface);
 if(SDL_MUSTLOCK(dest_surface))
  SDL_UnlockSurface(dest_surface);

}
