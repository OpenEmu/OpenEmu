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

#include <gccore.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef WII_BIN2O
#include "asot_ttf.h"
#endif

#include "wii_freetype.h"
#include "wii_main.h"
#include "wii_video.h"

static FT_Library ftlibrary;
static FT_Face face;
static FT_GlyphSlot slot;
static int fonthi, fontlo;

/*
 * Draws the specified character
 *
 * xfb    The frame buffer
 * bmp    The character
 * x      The x location
 * y      The y location
 */
static void draw_character( u32* xfb, FT_Bitmap* bmp, FT_Int x, FT_Int y )
{
  FT_Int i, j, p, q;
  FT_Int x_max = x + bmp->width;
  FT_Int y_max = y + bmp->rows;

  int spos;
  unsigned int pixel;
  int c;

  for( i = x, p = 0; i < x_max; i++, p++ )
  {
    for( j = y, q = 0; j < y_max; j++, q++ )
    {
      if( i < 0 || j < 0 || i >= WII_WIDTH || j >= WII_HEIGHT )
      {
        continue;
      }

      // Convert pixel position to GC int sizes
      spos = (j * 320) + (i >> 1);
      pixel = xfb[spos];
      c = bmp->buffer[q * bmp->width + p];

      // Anti-Aliasing doesn't work too well at hires on GC
      if( c > 128 )
      {
        if( i & 1 )
        {
          pixel = (pixel & 0xffff0000) | fontlo;
        }
        else
        {
          pixel = ((pixel & 0xffff) | fonthi);
        }

        xfb[spos] = pixel;
      }
    }
  }
}

/*
 * Initializes the FreeType library
 *
 * returns  If an error occurred (non-zero)
 */
int wii_ft_init()
{
  int err = FT_Init_FreeType( &ftlibrary );
  if( err )
  {
    return 1;
  }

  err = 
    FT_New_Memory_Face( 
      ftlibrary, (FT_Byte *)asot_ttf, asot_ttf_size, 0, &face );

  if( err )
  {
    return 1;
  }

  wii_ft_set_fontsize( 14 );
  wii_ft_set_fontcolor( 0xff, 0xff, 0xff );

  slot = face->glyph;

  return 0;
}

/*
 * Sets the font size
 *
 * pixelsize    The font size
 * returns      If an error occurred (non-zero)
 */
int wii_ft_set_fontsize( int pixelsize )
{
  return FT_Set_Pixel_Sizes( face, 0, pixelsize );
}

/*
 * Sets the font color
 *
 * r  Red
 * g  Green
 * b  Blue
 */
void wii_ft_set_fontcolor( u8 r, u8 g, u8 b )
{
  u32 fontcolor;
  fontcolor = wii_video_rgb_to_y1cby2cr( r, g, b );
  fonthi = fontcolor & 0xffff0000;
  fontlo = fontcolor & 0xffff;
}

/*
 * Calculates the width of the specified text
 *
 * text     The text
 * return   The width of the text 
 */
int wii_ft_get_textwidth( char *text )
{
  int err;  
  int i;
  int px = 0;

  int n = strlen( text );
  for( i = 0; i < n; i++ )
  {
    err = FT_Load_Char( face, text[i], FT_LOAD_RENDER );

    if( err )
    {
      continue;	// Skip unprintable characters
    }

    px += slot->advance.x >> 6;
  }

  return px;
}

/*
 * Draws the specified text
 *
 * xfb    The framebuffer
 * x      The x location 
 *          (-1 auto center, -2 left of center, -3 right of center)
 * y      The y location
 * text   The text to draw
 */
void wii_ft_drawtext( u32* xfb, int x, int y, char *text )
{
  int px;
  int i;
  int err;

  int n = strlen( text );
  if( n == 0 )
  {
    return;
  }
 
  if( x < 0 )
  {
    px = wii_ft_get_textwidth( text );
    if( x == -1 )
    {
      px = (WII_WIDTH - px) >> 1;
    }
    else if( x == -2 )
    {
       px = ( ( WII_WIDTH >> 1 ) - px );
    }
    else if( x == -3 )
    {
      px = ( WII_WIDTH >> 1 );
    }
  }
  else
  {
    px = x;
  }

  // Draw the string
  for( i = 0; i < n; i++ )
  {
    err = FT_Load_Char( face, text[i], FT_LOAD_RENDER );

    if( err )
    {
      continue;	// Skip unprintable characters
    }

    draw_character( xfb, &slot->bitmap, 
      px + slot->bitmap_left, y - slot->bitmap_top );

    px += slot->advance.x >> 6;
  }
}
