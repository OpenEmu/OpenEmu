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

#include "wii_video.h"

/*
 * Converts RGB to Y1CbY2Cr format
 *
 * r1       Red
 * g1       Green
 * b1       Blue
 * return   Color in Y1CbY2Cr format
 */
u32 wii_video_rgb_to_y1cby2cr( u8 r1, u8 g1, u8 b1 )
{
  int y1, cb1, cr1, y2, cb2, cr2, cb, cr;
  u8 r2, g2, b2;

  r2 = r1;
  g2 = g1;
  b2 = b1;

  y1 = (299 * r1 + 587 * g1 + 114 * b1) / 1000;
  cb1 = (-16874 * r1 - 33126 * g1 + 50000 * b1 + 12800000) / 100000;
  cr1 = (50000 * r1 - 41869 * g1 - 8131 * b1 + 12800000) / 100000;

  y2 = (299 * r2 + 587 * g2 + 114 * b2) / 1000;
  cb2 = (-16874 * r2 - 33126 * g2 + 50000 * b2 + 12800000) / 100000;
  cr2 = (50000 * r2 - 41869 * g2 - 8131 * b2 + 12800000) / 100000;

  cb = (cb1 + cb2) >> 1;
  cr = (cr1 + cr2) >> 1;

  return ((y1 << 24) | (cb << 16) | (y2 << 8) | cr);
}

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
void wii_video_draw_line( 
  u32* xfb, int startx, int endx, int y, u8 r, u8 g, u8 b )
{
  int i;

  u32 offset = (y * 320) + (startx >> 1);
  u32 color = wii_video_rgb_to_y1cby2cr( r, g, b );
  int width = (endx - startx) >> 1;

  for( i = 0; i < width; i++ )
  {
    xfb[offset++] = color;
  }
}

/*
 * Draws the image specified to the frame buffer
 *
 * props      The image properties
 * source     The bytes of the image
 * framebuff  The destination frame buffer
 * x          The x location to display the image in the buffer
 * y          The y location to disploay the image in the buffer
 */
void wii_video_draw_image( 
  PNGUPROP *props, u32 *source, u32 *framebuff, u16 x, u16 y )
{
  u16 pix = 0, row = 0, col = 0;
  u16 offset = y * 320;	

  x >>= 1;

  for( row = 0; row < props->imgHeight; row++ )
  {
    for( col = 0; col < ( props->imgWidth >> 1 ); col++ )
    {
      framebuff[offset + col + x] = source[pix++];
    }
    offset += 320;
  }
}