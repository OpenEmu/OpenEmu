#ifndef __MDFN_SELBLUR_H
#define __MDFN_SELBLUR_H

#include "../video.h"

typedef struct
{
 unsigned int red_threshold; // 0 - 255
 unsigned int green_threshold;
 unsigned int blue_threshold;
 int32 radius;

 uint32 *source;
 uint32 source_pitch32;
 uint32 *dest;
 uint32 dest_pitch32;

 int32 width;
 int32 height;

 uint32 red_shift;
 uint32 green_shift;
 uint32 blue_shift;

} SelBlurImage;

void MDFN_SelBlur(SelBlurImage *spec);

#endif
