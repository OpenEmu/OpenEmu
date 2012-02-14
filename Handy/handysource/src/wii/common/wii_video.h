/*
Copyright (C) 2010
raz0red (www.twitchasylum.com)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.
*/

#ifndef WII_VIDEO_H
#define WII_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gctypes.h>

#include "pngu.h"

/*
 * Converts RGB to Y1CbY2Cr format
 *
 * r1       Red
 * g1       Green
 * b1       Blue
 * return   Color in Y1CbY2Cr format
 */
extern u32 wii_video_rgb_to_y1cby2cr( u8 r1, u8 g1, u8 b1 );

/*
 * Draws a line
 *
 * xfb      The frame buffer
 * startx   The starting x position
 * endx     The ending x position
 * y        The y location
 * r        Red
 * g        Green
 * b        Blue 
 */
extern void wii_video_draw_line( 
  u32* xfb, int startx, int endx, int y, u8 r, u8 g, u8 b );

/*
 * Draws the image specified to the frame buffer
 *
 * props      The image properties
 * source     The bytes of the image
 * framebuff  The destination frame buffer
 * x          The x location to display the image in the buffer
 * y          The y location to disploay the image in the buffer
 */
extern void wii_video_draw_image( 
  PNGUPROP *props, u32 *source, u32 *framebuff, u16 x, u16 y );

#ifdef __cplusplus
}
#endif

#endif
