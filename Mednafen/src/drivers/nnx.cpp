/* Nearest-neighbor simple scalers */

#include "main.h"
#include "nnx.h"

void nnx(int factor, SDL_Surface *src, SDL_Rect *src_rect, SDL_Surface *dest, SDL_Rect *dest_rect)
{
 uint32 *source_pixies = (uint32 *)((uint8 *)src->pixels + src_rect->y * src->pitch);
 source_pixies += src_rect->x;
 int source_pitch = src->pitch / sizeof(uint32);
 int source_pitch_diff = source_pitch - src_rect->w;

 uint32 *dest_pixies = (uint32 *)((uint8 *)dest->pixels + dest_rect->y * dest->pitch);
 dest_pixies += dest_rect->x;
 int dest_pitch = dest->pitch / sizeof(uint32);
 int dest_pitch_diff = dest_pitch - dest_rect->w + dest_pitch * (factor - 1);

 int max_x = src_rect->w;

 switch(factor)
 {
  case 2:
  for(int y = src_rect->h; y; y--)
  {
   for(int x = max_x; x; x--)
   {
    dest_pixies[0] = dest_pixies[1] = dest_pixies[dest_pitch] = dest_pixies[dest_pitch + 1] = *source_pixies;
    source_pixies++;
    dest_pixies += 2;
   }
   dest_pixies += dest_pitch_diff;
   source_pixies += source_pitch_diff;
  }
  break;

  case 3:
  for(int y = src_rect->h; y; y--)
  {
   for(int x = max_x; x; x--)
   {
    dest_pixies[0] = dest_pixies[1] = dest_pixies[2] = 
    dest_pixies[dest_pitch] = dest_pixies[dest_pitch + 1] = dest_pixies[dest_pitch + 2] = 
    dest_pixies[dest_pitch << 1] = dest_pixies[(dest_pitch << 1) + 1] = dest_pixies[(dest_pitch << 1) + 2] = *source_pixies;
    source_pixies++;
    dest_pixies += 3;
   }
   dest_pixies += dest_pitch_diff;
   source_pixies += source_pitch_diff;
  }
  break;

  case 4:
  for(int y = src_rect->h; y; y--)
  {
   for(int x = max_x; x; x--)
   {
    dest_pixies[0] = dest_pixies[1] = dest_pixies[2] = dest_pixies[3] =
    dest_pixies[dest_pitch] = dest_pixies[dest_pitch + 1] = dest_pixies[dest_pitch + 2] = dest_pixies[dest_pitch + 3] =
    dest_pixies[dest_pitch << 1] = dest_pixies[(dest_pitch << 1) + 1] = dest_pixies[(dest_pitch << 1) + 2] = dest_pixies[(dest_pitch << 1) + 3] = 
    dest_pixies[(dest_pitch << 1) + dest_pitch] = dest_pixies[(dest_pitch << 1) + dest_pitch + 1] = dest_pixies[(dest_pitch << 1) + dest_pitch + 2] = dest_pixies[(dest_pitch << 1) + dest_pitch + 3] = *source_pixies;
    source_pixies++;
    dest_pixies += 4;
   }
   dest_pixies += dest_pitch_diff;
   source_pixies += source_pitch_diff;
  }
  break;

 }
}

void nnyx(int factor, SDL_Surface *src, SDL_Rect *src_rect, SDL_Surface *dest, SDL_Rect *dest_rect)
{
 uint32 *source_pixies = (uint32 *)((uint8 *)src->pixels + src_rect->y * src->pitch);
 source_pixies += src_rect->x;
 int source_pitch = src->pitch / sizeof(uint32);
 int source_pitch_diff = source_pitch - src_rect->w;

 uint32 *dest_pixies = (uint32 *)((uint8 *)dest->pixels + dest_rect->y * dest->pitch);
 dest_pixies += dest_rect->x;
 int dest_pitch = dest->pitch / sizeof(uint32);
 int dest_pitch_diff = dest_pitch - dest_rect->w + dest_pitch * (factor - 1);

 int max_x = src_rect->w;

 switch(factor)
 {
  case 2:
  for(int y = src_rect->h; y; y--)
  {
   for(int x = max_x; x; x--)
   {
    dest_pixies[0] = dest_pixies[dest_pitch] = *source_pixies;
    source_pixies++;
    dest_pixies ++;
   }
   dest_pixies += dest_pitch_diff;
   source_pixies += source_pitch_diff;
  }
  break;

  case 3:
  for(int y = src_rect->h; y; y--)
  {
   for(int x = max_x; x; x--)
   {
    dest_pixies[0] =
    dest_pixies[dest_pitch] = 
    dest_pixies[dest_pitch << 1] = *source_pixies;
    source_pixies++;
    dest_pixies ++;
   }
   dest_pixies += dest_pitch_diff;
   source_pixies += source_pitch_diff;
  }
  break;

  case 4:
  for(int y = src_rect->h; y; y--)
  {
   for(int x = max_x; x; x--)
   {
    dest_pixies[0] = 
    dest_pixies[dest_pitch] = 
    dest_pixies[dest_pitch << 1] = 
    dest_pixies[(dest_pitch << 1) + dest_pitch] = *source_pixies;
    source_pixies++;
    dest_pixies ++;
   }
   dest_pixies += dest_pitch_diff;
   source_pixies += source_pitch_diff;
  }
  break;

 }
}

