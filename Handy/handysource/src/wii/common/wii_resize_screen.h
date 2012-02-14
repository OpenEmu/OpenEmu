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

#ifndef WII_RESIZE_SCREEN_H
#define WII_RESIZE_SCREEN_H

#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Information about the resize operation
 */
typedef struct resize_info {
  float defaultX; 
  float defaultY; 
  float currentX; 
  float currentY;
} resize_info;

/*
 * Displays the resize user interface
 *
 * rinfo  Information for the resize operation
 */
extern void wii_resize_screen_gui( resize_info* rinfo );

/*
 * Draws a border around the surface that is to be scaled.
 *
 * surface  The surface to scale
 * startY   The Y offset into the surface to scale
 * height   The height to scale
 */
extern void wii_resize_screen_draw_border( SDL_Surface* surface, int startY, int height );

#ifdef __cplusplus
}
#endif

#endif
