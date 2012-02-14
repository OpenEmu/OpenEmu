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

#ifndef WII_SDL_H
#define WII_SDL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL.h>
#include <SDL_ttf.h>

#include <gctypes.h>

// The Wii surface
extern SDL_Surface* back_surface;
// The Coleco surface
extern SDL_Surface* blit_surface;

// Fonts
extern TTF_Font *sdl_font_18;
extern TTF_Font *sdl_font_14;
extern TTF_Font *sdl_font_13;
extern TTF_Font *sdl_font_12;

// Colors
extern SDL_Color SDL_COLOR_WHITE;
extern SDL_Color SDL_COLOR_BLACK;
extern SDL_Color SDL_COLOR_RED;

/*
 * Initializes the SDL
 */
extern int wii_sdl_init();

/*
 * Maps the specified color into the back surface.
 *
 * return   The index of the specified color
 */
extern uint wii_sdl_rgb( u8 R, u8 G, u8 B );

/*
 * Renders the current back surface
 */
extern void wii_sdl_flip();

/*
 * Renders black to both blit and back surfaces
 */
extern void wii_sdl_black_screen();

/*
 * Blacks the back surface
 */
extern void wii_sdl_black_back_surface();

/*
 * Renders the Coleco surface to the Wii surface. 
 * TODO: replace this lame scaling method w/ GX based scaler
 *
 * scale  The scale to render the surface (1 or 2)
 */
extern void wii_sdl_put_image_normal( int scale );

/*
 * Renders text to the Wii surface
 *
 * font       The font to render with
 * text       The text to display
 * destRect   The location to render the text
 * colorFG    The forground color (optional)
 * colorBG    The background color (optional)
 */
extern void wii_sdl_render_text( 
  TTF_Font *font, const char *text, SDL_Rect *destRect, 
  SDL_Color *colorFG, SDL_Color *colorBG );

/*
 * Determines the size of the text displayed with the specified font
 *
 * font     The font
 * text     The text
 * w        The width (return value)
 * h        The height (return value)
 */
extern void wii_sdl_get_text_size( 
  TTF_Font *font, const char *text, int *w, int *h );

/*
 * Renders a filled rectangle to the back (Wii) surface
 *
 * surface  The surface to render to
 * x        The x location
 * y        The y location
 * w        The width
 * h        The height
 * color    The color
 */
extern void wii_sdl_fill_rectangle( 
  SDL_Surface* surface, int x, int y, int w, int h, u32 color );

/*
 * Renders a rectangle to the back (Wii) surface
 *
 * surface  The surface to render to
 * x        The x location
 * y        The y location
 * w        The width
 * h        The height
 * border   The border color
 * exor     Whether to exclusive or the rectangle's lines
 */
extern void wii_sdl_draw_rectangle( 
  SDL_Surface* surface, int x, int y, int w, int h, u32 border, BOOL exor );

/*
 * Frees the SDL resources
 */
extern void wii_sdl_free_resources();

//
// Methods to be implemented by application
//

/*
 * Initializes the SDL
 */
extern int wii_sdl_handle_init();

#ifdef __cplusplus
}
#endif

#endif
