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

#ifndef WII_FREETYPE_H
#define WII_FREETYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initializes the FreeType library
 *
 * returns  If an error occurred (non-zero)
 */
extern int wii_ft_init();

/*
 * Sets the font size
 *
 * pixelsize    The font size
 * returns      If an error occurred (non-zero)
 */
extern int wii_ft_set_fontsize( int pixelsize );

/*
 * Sets the font color
 *
 * r  Red
 * g  Green
 * b  Blue
 */
extern void wii_ft_set_fontcolor( u8 r, u8 g, u8 b );

/*
 * Draws the specified text
 *
 * xfb    The framebuffer
 * x      The x location 
 *          (-1 auto center, -2 left of center, -3 right of center)
 * y      The y location
 * text   The text to draw
 */
extern void wii_ft_drawtext( u32* xfb, int x, int y, char *text );

/*
 * Calculates the width of the specified text
 *
 * text     The text
 * return   The width of the text 
 */
extern int wii_ft_get_textwidth( char *text );

#ifdef __cplusplus
}
#endif

#endif

