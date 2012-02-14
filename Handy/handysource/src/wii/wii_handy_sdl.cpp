/*
WiiHandy : Port of the Handy Emulator for the Wii

Copyright (C) 2011
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

#include "wii_sdl.h"

#include "wii_handy.h"

// The surface for horizontal ROMS
static SDL_Surface *horiz_surface = NULL;
// The surface for vertical ROMS
static SDL_Surface *vert_surface = NULL;


/*
 * Set the surface to be horizontal or vertical
 *
 * isHorizontal Whether the surface is to be horizontal or vertical
 */
void wii_handy_set_surface( BOOL isHorizontal )
{
  blit_surface = ( isHorizontal ? horiz_surface : vert_surface );
}

/*
 * Initializes the SDL
 */
int wii_sdl_handle_init()
{
  if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0) {
    return 0;
  }

  if( SDL_InitSubSystem( SDL_INIT_VIDEO ) < 0 ) {
    return 0;
  }

  back_surface = 
    SDL_SetVideoMode(
    WII_WIDTH,
    WII_HEIGHT, 
    16, 
    SDL_DOUBLEBUF|SDL_HWSURFACE
    );

  if( !back_surface) 
  {
    return 0;
  }

  horiz_surface = 
    SDL_CreateRGBSurface(
    SDL_SWSURFACE, 
    HANDY_WIDTH, 
    HANDY_HEIGHT,
    back_surface->format->BitsPerPixel,
    back_surface->format->Rmask,
    back_surface->format->Gmask,
    back_surface->format->Bmask, 0);

  vert_surface = 
    SDL_CreateRGBSurface(
    SDL_SWSURFACE, 
    HANDY_HEIGHT, 
    HANDY_WIDTH,
    back_surface->format->BitsPerPixel,
    back_surface->format->Rmask,
    back_surface->format->Gmask,
    back_surface->format->Bmask, 0);

  // Set the default surface as horizontal
  wii_handy_set_surface( TRUE );

  return 1;
}
