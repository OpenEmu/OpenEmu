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

#ifndef WII_GX_H
#define WII_GX_H

#include <gccore.h>

#include "FreeTypeGX.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Structure for loading images
 */
typedef struct gx_imagedata
{ 
  u8* data;
  int width;
  int height;
} gx_imagedata;

/*
 * Draws a rectangle at the specified position
 * 
 * x        The x position
 * y        The y position
 * width    The width of the rectangle
 * height   The height of the rectangle
 * color    The color of the rectangle
 * filled   Whether the rectangle is filled
 */
void wii_gx_drawrectangle( 
  int x, int y, int width, int height, GXColor color, BOOL filled );

/*
 * Draws text at the specified position
 * 
 * x          The x position
 * y          The y position
 * pixelSize  The pixel size
 * color      The color of the text
 * textStyle  The style(s) for the text (FreeTypeGX)
 */
void wii_gx_drawtext( 
  int16_t x, int16_t y, FT_UInt pixelSize, char *text, GXColor color, 
  uint16_t textStyle );

/**
 * Returns the width of the specified text
 *
 * pixelSize  The pixel size
 * text       The text
 * return     The width of the specified text
 */
uint16_t wii_gx_gettextwidth( FT_UInt pixelSize, char *text );

/*
 * Draws the image at the specified position
 *
 * xpos     The x position
 * ypos     The y position
 * width    The image width
 * height   The image height
 * data     The image data
 * degress  The rotation degrees
 * scaleX   How much to scale the X
 * scaleY   How much to scale the Y
 * alpha    
 */
void wii_gx_drawimage(
  int xpos, int ypos, u16 width, u16 height, u8 data[],
  f32 degrees, f32 scaleX, f32 scaleY, u8 alpha );

/*
 * Loads and returns the data for the image at the specified path
 * 
 * imgpath  The path to the image
 * return   The data for the loaded image
 */
gx_imagedata* wii_gx_loadimage( char *imgpath );

/*
 * Loads image data for the specified image buffer
 * 
 * buff     The image buffer
 * return   The data for the loaded image
 */
gx_imagedata* wii_gx_loadimagefrombuff( const u8 *buff );

/*
 * Frees the specified image data information
 *
 * data   The image data to release
 */
void wii_gx_freeimage( gx_imagedata* imgdata );

#ifdef __cplusplus
}
#endif

#endif
