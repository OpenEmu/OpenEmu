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
#include "overlay.h"
#include "nnx.h"
#include <math.h>

static SDL_Overlay *ov = NULL;


void OV_Blit(SDL_Surface *src_surface, const SDL_Rect *src_rect, const SDL_Rect *original_src_rect,
	const SDL_Rect *dest_rect, SDL_Surface *dest_surface, int softscale, int scanlines, int rotated)
{
 const uint32 spitch32 = src_surface->pitch >> 2;
 SDL_Rect drect = *dest_rect;
 uint16 *dpixels;
 uint32 dpitch16;
 int32 need_w, need_h;

 if(softscale < 1)
  softscale = 1;

 if(!rotated)
 {
  need_w = src_rect->w;
  need_h = src_rect->h;
 }
 else
 {
  need_w = src_rect->h;
  need_h = src_rect->w;
 }

 need_w *= softscale;
 need_h *= softscale;

 need_w = (need_w + 1) & ~1;

 if(!ov || ov->w != need_w || ov->h != need_h)
 {
  if(ov)
  {
   SDL_FreeYUVOverlay(ov);
   ov = NULL;
  } 
  if(!(ov = SDL_CreateYUVOverlay(need_w, need_h, SDL_YUY2_OVERLAY, dest_surface)))
  {
   puts("Overlay creation failure");
   return;
  }
  printf("Overlay Created: %d %d %d\n", ov->hw_overlay, ov->w, ov->h);
 }

 if(SDL_LockYUVOverlay(ov) != 0)
 {
  puts("Lock failure");
  return;
 }
 dpixels = (uint16 *)ov->pixels[0];

 assert(!(ov->pitches[0] & 1));

 dpitch16 = ov->pitches[0] >> 1;

 if(rotated == MDFN_ROTATE90)
 {
  uint16 *drow = &dpixels[(need_h - 1) * dpitch16];
  
  for(int y = 0; y < src_rect->w; y++)
  {
   uint32 *scol = (uint32 *)((uint8 *)src_surface->pixels + src_rect->y * src_surface->pitch) + src_rect->x + y;

   for(int x = 0; x < src_rect->h; x += 2)
   {
    unsigned int cb, cr;

    cb = ((scol[0] & 0x00FF00) + (scol[spitch32] & 0x00FF00)) >> 9;
    cr = ((scol[0] & 0xFF0000) + (scol[spitch32] & 0xFF0000)) >> 17;

    drow[x + 0] = (scol[0] & 0xFF) | (cb << 8);
    drow[x + 1] = (scol[spitch32] & 0xFF) | (cr << 8);

    scol += spitch32 * 2;
   }
   drow -= dpitch16;
  }
 }
 else if(rotated == MDFN_ROTATE270)
 {
  uint16 *drow = &dpixels[0];

  for(int y = 0; y < src_rect->w; y++)
  {
   uint32 *scol = (uint32 *)((uint8 *)src_surface->pixels + src_rect->y * src_surface->pitch) + src_rect->x + y;

   scol += spitch32 * (need_w - 1);

   for(int x = 0; x < src_rect->h; x += 2)
   {
    unsigned int cb, cr;

    cb = ((scol[0] & 0x00FF00) + (scol[-(int)spitch32] & 0x00FF00)) >> 9;
    cr = ((scol[0] & 0xFF0000) + (scol[-(int)spitch32] & 0xFF0000)) >> 17;

    drow[x + 0] = (scol[0] & 0xFF) | (cb << 8);
    drow[x + 1] = (scol[-(int)spitch32] & 0xFF) | (cr << 8);

    scol -= spitch32 * 2;
   }
   drow += dpitch16;
  }
 }
 else // Plain copy
 {
  //uint32 dest_pitch_diff = dpitch16 - (((src_rect->w + 1) & ~1) * softscale) + dpitch16 * (softscale - 1);
  uint32 dest_pitch_diff = dpitch16 - (src_rect->w * softscale) + dpitch16 * (softscale - 1);
  uint32 src_pitch_diff = (src_surface->pitch >> 2);
  uint16 *drow = dpixels;
  uint32 *srow = (uint32 *)((uint8 *)src_surface->pixels + (src_rect->y) * src_surface->pitch) + src_rect->x;

  // Dest pointer is always incremented by 2.
  dest_pitch_diff &= ~1;

  if(softscale == 2)
  {
   for(int y = 0; y < src_rect->h; y++)
   {
    for(int x = 0; x < src_rect->w; x++)
    {
     const unsigned int cb = (srow[x] & 0x00FF00) >> 8;
     const unsigned int cr = (srow[x] & 0xFF0000) >> 16;
     const unsigned int tmp0 = (srow[x] & 0xFF) | (cb << 8);
     const unsigned int tmp1 = (srow[x] & 0xFF) | (cr << 8);

     drow[0] = tmp0;
     drow[1] = tmp1;
     drow[dpitch16 + 0] = tmp0;
     drow[dpitch16 + 1] = tmp1;

     drow += 2;
    }
    srow += src_pitch_diff;
    drow += dest_pitch_diff;
   }
  }
  else if(softscale == 3)
  {
   unsigned int weasel_cb = 0, weasel_cr = 0;

   for(int y = 0; y < src_rect->h; y++)
   {
    for(int x = 0; x < src_rect->w; x++)
    {
     const unsigned int cb = (srow[x] & 0x00FF00);
     const unsigned int cr = (srow[x] & 0xFF0000) >> 8;
     const unsigned int y_c = srow[x] & 0xFF;
     unsigned int tmp0, tmp1, tmp2;

     tmp0 = y_c;
     tmp1 = y_c;
     tmp2 = y_c;

     if(x & 1)
     {
      tmp0 |= weasel_cr;
      tmp1 |= cb;
      tmp2 |= cr;
     }
     else
     {
      const unsigned int cb_next = (srow[x + 1] & 0x00FF00);
      const unsigned int cr_next = (srow[x + 1] & 0xFF0000) >> 8;

      weasel_cb = ((cb + cb_next) >> 1) & 0xFF00;
      weasel_cr = ((cr + cr_next) >> 1) & 0xFF00;

      tmp0 |= cb;
      tmp1 |= cr;
      tmp2 |= weasel_cb;
     }

     drow[0] = tmp0;
     drow[1] = tmp1;
     drow[2] = tmp2;

     drow[dpitch16 + 0] = tmp0;
     drow[dpitch16 + 1] = tmp1;
     drow[dpitch16 + 2] = tmp2;

     drow[(dpitch16 << 1) + 0] = tmp0;
     drow[(dpitch16 << 1) + 1] = tmp1;
     drow[(dpitch16 << 1) + 2] = tmp2;

     drow += 3;
    }
    srow += src_pitch_diff;
    drow += dest_pitch_diff;
   }
  }
  else if(softscale == 4)
  {
   for(int y = 0; y < src_rect->h; y++)
   {
    for(int x = 0; x < src_rect->w; x++)
    {
     const unsigned int cb = (srow[x] & 0x00FF00) >> 8;
     const unsigned int cr = (srow[x] & 0xFF0000) >> 16;
     const unsigned int tmp0 = (srow[x] & 0xFF) | (cb << 8);
     const unsigned int tmp1 = (srow[x] & 0xFF) | (cr << 8);


     drow[0] = tmp0;
     drow[1] = tmp1;
     drow[2] = tmp0;
     drow[3] = tmp1;
     drow[dpitch16 + 0] = tmp0;
     drow[dpitch16 + 1] = tmp1;
     drow[dpitch16 + 2] = tmp0;
     drow[dpitch16 + 3] = tmp1;
     drow[(dpitch16 << 1) + 0] = tmp0;
     drow[(dpitch16 << 1) + 1] = tmp1;
     drow[(dpitch16 << 1) + 2] = tmp0;
     drow[(dpitch16 << 1) + 3] = tmp1;
     drow[(dpitch16 << 1) + dpitch16 + 0] = tmp0;
     drow[(dpitch16 << 1) + dpitch16 + 1] = tmp1;
     drow[(dpitch16 << 1) + dpitch16 + 2] = tmp0;
     drow[(dpitch16 << 1) + dpitch16 + 3] = tmp1;

     drow += 4; 
    }
    srow += src_pitch_diff;
    drow += dest_pitch_diff;
   }
  }
  else
  {
   //static bool room = 0;
   //room ^= 1; //rand() & 1;
   //printf("%d %d %d %d\n", src_rect->w, dpitch16, src_pitch_diff, dest_pitch_diff);

   for(int y = 0; y < src_rect->h; y++)
   {
    uint32 lastpixcow = srow[0]; //0x808000;

    for(int x = 0; x < src_rect->w; x += 2)
    {
     //unsigned int cb = ((srow[x + 0] & 0x00FF00) + (srow[x + 1] & 0x00FF00)) >> 9;
     //unsigned int cr = ((srow[x + 0] & 0xFF0000) + (srow[x + 1] & 0xFF0000)) >> 17;
     //if(room)
     //{
     // cb = ((srow[x - 1] & 0x00FF00) + (srow[x + 0] & 0x00FF00)) >> 9;
     // cr = ((srow[x - 1] & 0xFF0000) + (srow[x + 0] & 0xFF0000)) >> 17;
     //}
     //     cb = (((srow[x + 0] >> 8) & 0xFF)*2 + ((srow[x + 1] >> 8) & 0xFF)  + ((srow[x - 1] >> 8) & 0xFF) ) / 4;
     //     cr = (((srow[x + 0] >> 16) & 0xFF)*2 + ((srow[x + 1] >> 16) & 0xFF)  + ((srow[x - 1] >> 16) & 0xFF) ) / 4;
     unsigned int cb = ((lastpixcow & 0x00FF00) + ((srow[x + 0] & 0x00FF00) << 1) + (srow[x + 1] & 0x00FF00)) >> 10;
     unsigned int cr = ((lastpixcow & 0xFF0000) + ((srow[x + 0] & 0xFF0000) << 1) + (srow[x + 1] & 0xFF0000)) >> 18;

	
     drow[0] = (srow[x + 0] & 0xFF) | (cb << 8);
     drow[1] = (srow[x + 1] & 0xFF) | (cr << 8);

     lastpixcow = srow[x + 1];

     drow += 2;
    }
    srow += src_pitch_diff;
    drow += dest_pitch_diff;
   }
  } // End else to softscale
 }
 SDL_UnlockYUVOverlay(ov);

 if(SDL_DisplayYUVOverlay(ov, &drect) != 0)
 {
  puts("Blit error");
 }
}

void OV_Kill(void)
{
 if(ov)
 {
  SDL_FreeYUVOverlay(ov);
  ov = NULL;
 }
}
