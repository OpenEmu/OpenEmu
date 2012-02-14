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

#include "FreeTypeGX.h"

#include "pngu.h"

#include "wii_app.h"
#include "wii_gx.h"
#include "wii_sdl.h"

extern Mtx gx_view;

static gx_imagedata* getimagedata( IMGCTX ctx );

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
  int x, int y, int width, int height, GXColor color, BOOL filled )
{
  u8 fmt;
  long n;
  int i;
  int x2 = x+width;
  int y2 = y-height;
  if( !filled ) { x+=1; y-=1; }
  guVector v[] = {{x,y,0}, {x2,y,0}, {x2,y2,0}, {x,y2,0}, {x,y+1,0}};

  if(!filled)
  {
    fmt = GX_LINESTRIP;
    n = 5;
  }
  else
  {
    fmt = GX_TRIANGLEFAN;
    n = 4;
  }

  GX_Begin( fmt, GX_VTXFMT0, n );
  for(i=0; i<n; i++)
  {
    GX_Position3s16( v[i].x, v[i].y, v[i].z );
    GX_Color4u8( color.r, color.g, color.b, color.a );
  }
  GX_End();
}

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
  uint16_t textStyle )
{
  FT_DrawText( x, y, pixelSize, text, color, textStyle );
}

/**
 * Returns the width of the specified text
 *
 * pixelSize  The pixel size
 * text       The text
 * return     The width of the specified text
 */
uint16_t wii_gx_gettextwidth( FT_UInt pixelSize, char *text )
{
  return FT_GetWidth( pixelSize, text );
}

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
  f32 degrees, f32 scaleX, f32 scaleY, u8 alpha )
{
  if( data == NULL )
    return;

  GXTexObj texObj;
  
  GX_InitTexObj( &texObj, data, width, height, GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE );
  GX_LoadTexObj( &texObj, GX_TEXMAP0 );
  GX_InvalidateTexAll();

  GX_SetTevOp( GX_TEVSTAGE0, GX_MODULATE );
  GX_SetVtxDesc( GX_VA_TEX0, GX_DIRECT );

  Mtx m, m1, m2, mv;

  int x2 = xpos+width;
  int y2 = ypos-height;

  guMtxIdentity( m1 );
  guMtxScaleApply( m1, m1, scaleX, scaleY, 1.0 );
  guVector axis = (guVector) { 0, 0, 1 };
  guMtxRotAxisDeg( m2, &axis, degrees );
  guMtxConcat( m2, m1, m );

  guMtxTransApply( m, m, 0, 0, -100 );
  guMtxConcat( gx_view, m, mv );
  GX_LoadPosMtxImm( mv, GX_PNMTX0 );

  GX_Begin( GX_QUADS, GX_VTXFMT0,4 );
  GX_Position3s16( xpos, ypos, 0 );
  GX_Color4u8( 0xFF, 0xFF, 0xFF, alpha );
  GX_TexCoord2f32( 0, 0 );

  GX_Position3s16( x2, ypos, 0 );
  GX_Color4u8( 0xFF, 0xFF, 0xFF, alpha );
  GX_TexCoord2f32( 1, 0 );

  GX_Position3s16( x2, y2, 0);
  GX_Color4u8( 0xFF, 0xFF, 0xFF, alpha );
  GX_TexCoord2f32( 1, 1 );

  GX_Position3s16( xpos, y2, 0 );
  GX_Color4u8( 0xFF, 0xFF, 0xFF, alpha );
  GX_TexCoord2f32( 0, 1 );
  GX_End();  

  GX_SetTevOp( GX_TEVSTAGE0, GX_PASSCLR );
  GX_SetVtxDesc( GX_VA_TEX0, GX_NONE );
}

/*
 * Loads and returns the data for the image at the specified path
 * 
 * imgpath  The path to the image
 * return   The data for the loaded image
 */
gx_imagedata* wii_gx_loadimage( char *imgpath )
{  
  if( imgpath )
  {    
    IMGCTX ctx = PNGU_SelectImageFromDevice( imgpath );
    return getimagedata( ctx );
  }
  
  return NULL;
}

/*
 * Loads image data for the specified image buffer
 * 
 * buff     The image buffer
 * return   The data for the loaded image
 */
gx_imagedata* wii_gx_loadimagefrombuff( const u8 *buff )
{  
  if( buff )
  {    
    IMGCTX ctx = PNGU_SelectImageFromBuffer( buff );
    return getimagedata( ctx );
  }
  
  return NULL;
}

/*
 * Loads image data for the specified image context
 *
 * ctx    The image context
 * return The image data for the specified context
 */
static gx_imagedata* getimagedata( IMGCTX ctx )
{    
  if( !ctx )
    return NULL;

  gx_imagedata imgdata;
  memset( &imgdata, 0, sizeof( gx_imagedata ) );
  PNGUPROP imgProp;

  int res = PNGU_GetImageProperties( ctx, &imgProp );
  if( res == PNGU_OK )
  {
    int len = imgProp.imgWidth * imgProp.imgHeight * 4;
    if( len % 32 ) len += ( 32 - len % 32 );
    imgdata.data = (u8*)memalign( 32, len );
    if( imgdata.data )
    {
      res = 
        PNGU_DecodeTo4x4RGBA8( ctx, imgProp.imgWidth, imgProp.imgHeight,
          imgdata.data, 255 );

      if( res == PNGU_OK )
      {
        imgdata.width = imgProp.imgWidth;
        imgdata.height = imgProp.imgHeight;
        DCFlushRange( imgdata.data, len );
      }
      else
      {
        free( imgdata.data );
        imgdata.data = NULL;
      }
    }
  }

  PNGU_ReleaseImageContext( ctx );  

  gx_imagedata* ret = NULL;
  if( imgdata.data )
  {
    ret = (gx_imagedata*)malloc( sizeof( gx_imagedata ) );
    *ret = imgdata;
  }

  return ret;
}

/*
 * Frees the specified image data information
 *
 * data   The image data to release
 */
void wii_gx_freeimage( gx_imagedata* imgdata )
{
  if( imgdata )
  {
    free( imgdata->data );
    imgdata->data = NULL;
    free( imgdata );
  }
}
