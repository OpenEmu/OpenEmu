/*
//  source code for the BMGLib Utility functions
//
//  Copyright (C) 2001 M. Scott Heiman
//  All Rights Reserved
//
// You may use the software for any purpose you see fit. You may modify
// it, incorporate it in a commercial application, use it for school,
// even turn it in as homework. You must keep the Copyright in the
// header and source files. This software is not in the "Public Domain".
// You may use this software at your own risk. I have made a reasonable
// effort to verify that this software works in the manner I expect it to;
// however,...
//
// THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS" AND
// WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE, INCLUDING
// WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR FITNESS FOR A
// PARTICULAR PURPOSE. IN NO EVENT SHALL MICHAEL S. HEIMAN BE LIABLE TO
// YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL, INDIRECT OR
// CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER, INCLUDING
// WITHOUT LIMITATION, LOSS OF PROFIT, LOSS OF USE, SAVINGS OR REVENUE,
// OR THE CLAIMS OF THIRD PARTIES, WHETHER OR NOT MICHAEL S. HEIMAN HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
// POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdlib.h>
#include "BMGUtils.h"

#ifndef _WIN32
#include <string.h>
#endif // _WIN32

/* error strings for all BMG errors */
static char BMGErrorStrings[17][128] = {
"No Error",
"Corrupted file or invalid file format",
"Invalid bits per pixel for this file format",
"Memory allocation error",
"Invalid requested image size",
"Invalid bitmap handle",
"Windows API Error",  /* this will be overwritten */
"Unable to open file",
"Unsupported file format option",
"Invalid pointer to a BMG image",
"Unsupported file extension",
"Error reading file",
"Error writing to the output file",
"Invalid pointer to a GeoTIFF structure",
"The background image is undefined",
"The background image is too small",
"Corrupt File"
};

/* stores last BMG error */
static BMGError LastBMGError;

/* sets the last BMG error */
void SetLastBMGError( BMGError err )
{
    LastBMGError = err;
}

/* returns the last error state */
BMGError GetLastBMGError(void)
{
    return LastBMGError;
}
/* gets the error message */
void GetLastBMGErrorMessage( const char **msg )
{
    if ( LastBMGError == errWindowsAPI )
    {
       char* lpMsgBuf = "Erreur BMG\n";

        /* copy the string. */
          strcpy( BMGErrorStrings[(int)LastBMGError], (char *)lpMsgBuf );
    }

    *msg = BMGErrorStrings[(int)LastBMGError];
}

/* Global background color variables */
static unsigned char BackgroundColor[4];
static struct BMGImageStruct BackgroundImage;

/* this function simply initializes the background info.  It is called from
   the DllEntryPoint function */
void InitBackground(void)
{
    memset( (void *)BackgroundColor, 0xFF, 3 ); /* white */
    BackgroundColor[3] = 0; /* ignored */
    InitBMGImage( &BackgroundImage );
}

unsigned char *GetBackgroundColor(void)
{
    return &BackgroundColor[0];
}

struct BMGImageStruct *GetBackgroundImage(void)
{
    return &BackgroundImage;
}

/* converts an array of 1-bit scanlines to 8-bit scanlines */
void Convert1to8( struct BMGImageStruct img,
                  unsigned char *out )
{
    unsigned char *p, *q, *r, *s, *end;
    int i;

    q = out;

    for ( s = img.bits; s < img.bits + img.scan_width * img.height;
          s += img.scan_width, q += img.width )
    {
        i = img.width % 8;
        end = q + img.width - i;
        p = s;
        for ( r = q; r < end; p++ )
        {
            *r++ = (unsigned char)((*p & 0x80) ? 1 : 0);
            *r++ = (unsigned char)((*p & 0x40) ? 1 : 0);
            *r++ = (unsigned char)((*p & 0x20) ? 1 : 0);
            *r++ = (unsigned char)((*p & 0x10) ? 1 : 0);
            *r++ = (unsigned char)((*p & 0x08) ? 1 : 0);
            *r++ = (unsigned char)((*p & 0x04) ? 1 : 0);
            *r++ = (unsigned char)((*p & 0x02) ? 1 : 0);
            *r++ = (unsigned char)(*p & 0x01);
        }

        if ( i-- )
        {
            *r++ =  (unsigned char)((*p & 0x80) ? 1 : 0);
            if ( i-- )
            {
                *r++ =  (unsigned char)((*p & 0x40) ? 1 : 0);
                if ( i-- )
                {
                    *r++ =  (unsigned char)((*p & 0x20) ? 1 : 0);
                    if ( i-- )
                    {
                        *r++ =  (unsigned char)((*p & 0x10) ? 1 : 0);
                        if ( i-- )
                        {
                            *r++ =  (unsigned char)((*p & 0x08) ? 1 : 0);
                            if ( i-- )
                            {
                                *r++ =  (unsigned char)((*p & 0x04) ? 1 : 0);
                                if ( i )
                                    *r   =  (unsigned char)((*p & 0x02) ? 1:0);
                            }
                        }
                    }
                }
            }
        }
    }
}

/* converts an array of 4-bit scanlines to 8-bit scanlines */
void Convert4to8( struct BMGImageStruct img,
                  unsigned char *out )
{
    unsigned char *p, *q, *r, *s, *end;
    int i;

    q = out;

    for ( s = img.bits; s < img.bits + img.scan_width * img.height;
          s += img.scan_width, q += img.width )
    {
        i = img.width % 2;
        end = q + img.width - i;
        p = s;
        for ( r = q; r < end; p++ )
        {
            *r++ = (unsigned char)((*p >> 4) & 0x0F);
            *r++ = (unsigned char)(*p & 0x0F);
        }

        if ( i )
            *r =  (unsigned char)((*p >> 4) & 0x0F);
    }
}

/****************************************************************************/
/* this function performs alpha blending.  It is a variation of a function
  that I found in the PNG example code */
unsigned char AlphaComp( unsigned char fg,
                         unsigned char alpha,
                         unsigned char bg )
{
  unsigned char out;
  unsigned short temp;

  switch ( alpha )
  {
    case 0:
      out = bg;
      break;
    case 255:
      out = fg;
      break;
    default:
      temp = ((unsigned short)(fg)*(unsigned short)(alpha) +
                    (unsigned short)(bg)*(unsigned short)(255 -
                    (unsigned short)(alpha)) + (unsigned short)128);
      out = (unsigned char)((temp + (temp >> 8)) >> 8);
      break;
  }
  return out;
}

/****************************************************************************
// Converts a 16 BPP image to a 24 BPP image 
// returns 1 if successfull, 0 otherwise */
BMGError Convert16to24( struct BMGImageStruct *img )
{
    unsigned int i;
    unsigned int new_scan_width;
    unsigned char *new_bits;

    /* this function will only work with 16 BBP images */
    if ( img->bits_per_pixel != 16 )
        return errInvalidPixelFormat;

    /* calculate the new scan width */
    new_scan_width = 3 * img->width;
    if ( new_scan_width % 4 && img->opt_for_bmp )
        new_scan_width += 4 - new_scan_width % 4;

    /* allocate memory for the new pixel values */
    new_bits = (unsigned char *)calloc( new_scan_width * img->height, sizeof(unsigned char) );
    if ( new_bits == NULL )
        return errMemoryAllocation;

    /* convert the 16 BPP pixel values to the equivalent 24 BPP values  */
    for ( i = 0; i < img->height; i++ )
    {
        unsigned char *p24;
        unsigned short *p16 = (unsigned short *)(img->bits + i * img->scan_width);
        unsigned char *start = new_bits + i * new_scan_width;
        unsigned char *end = start + new_scan_width;
        for ( p24 = start; p24 < end; p24 += 3, p16++ ) 
        {
            p24[0] = (unsigned char)( (*p16 & 0x001F) << 3 );
            p24[1] = (unsigned char)( (*p16 & 0x03E0) >> 2 );
            p24[2] = (unsigned char)( (*p16 & 0x7C00) >> 7 );
        }
    }

    free( img->bits );
    img->bits = new_bits;
    img->scan_width = new_scan_width;
    img->bits_per_pixel = 24;

    return BMG_OK;
}

/****************************************************************************/
/* this function undoes alpha blending - kind-a-sort-of ;-) */
unsigned char InverseAlphaComp( unsigned char fg, unsigned char alpha,
                               unsigned char bg )
{
  unsigned char out;
  short temp;

  switch ( alpha )
  {
    case 0:
      out = bg;
      break;
    case 255:
      out = fg;
      break;
    default:
      temp = (255*fg - bg*(255-alpha))/alpha;
      if ( temp < 0 )
        temp = 0;
      out = (unsigned char)temp;
      break;
  }
  return out;
}

/****************************************************************************/
/*
// Creates a BITMAPINFOHEADER for the given width, height, bit count, and
// compression.  Compression must = BI_RGB, BI_BITFIELDS, BI_RLE4, or BI_RLE8.
*/
BITMAPINFO InternalCreateBMI( unsigned int dwWidth,  /* width */
                              unsigned int dwHeight, /* height */
                              unsigned short wBitCount, /* bit count */
                              int compression )  /* compression type */
{
   BITMAPINFO bi;         /* bitmap header */
   unsigned int dwBytesPerLine;        /* Number of bytes per scanline */

   /* clear the bitmapinfo structure */
   memset(&bi, 0, sizeof(BITMAPINFO));

   /* Make sure bits per pixel is valid */
   if (wBitCount <= 1)
      wBitCount = 1;
   else if (wBitCount <= 4)
      wBitCount = 4;
   else if (wBitCount <= 8)
      wBitCount = 8;
   else if (wBitCount <= 16)
      wBitCount = 16;
   else if (wBitCount <= 24)
      wBitCount = 24;
   else if (wBitCount <= 32)
      wBitCount = 32;
   else
      wBitCount = 8;  /* set default value to 8 if parameter is bogus */

   dwBytesPerLine =   (((wBitCount * dwWidth) + 31) / 32 * 4);

   /* initialize BITMAPINFO */
   bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
   bi.bmiHeader.biWidth = dwWidth;
   bi.bmiHeader.biHeight = dwHeight;
   bi.bmiHeader.biPlanes = 1;              /* must be 1 */
   bi.bmiHeader.biBitCount = wBitCount;
   bi.bmiHeader.biCompression =  compression;
   bi.bmiHeader.biSizeImage = dwBytesPerLine*dwHeight;
   bi.bmiHeader.biXPelsPerMeter = 0;
   bi.bmiHeader.biYPelsPerMeter = 0;
   bi.bmiHeader.biClrUsed = wBitCount <= 8 ? 1U << wBitCount : 0;
   bi.bmiHeader.biClrImportant = bi.bmiHeader.biClrUsed;

   return bi;
}

short SwapShort( short in )
{
    char sin[2];
    char sout[2];

    memcpy( (char *)sin, (char *)&in, 2 );

    sout[0] = sin[1];
    sout[1] = sin[0];

    return *((short *)sout);
}

unsigned short SwapUShort( unsigned short in )
{
    char sin[2];
    char sout[2];

    memcpy( (char *)sin, (char *)&in, 2 );

    sout[0] = sin[1];
    sout[1] = sin[0];

    return *((unsigned short *)sout);
}

int SwapLong( int in )
{
    char sin[4];
    char sout[4];

    memcpy( (char *)sin, (char *)&in, 4 );

    sout[0] = sin[3];
    sout[1] = sin[2];
    sout[2] = sin[1];
    sout[3] = sin[0];

    return *((int *)sout);
}

unsigned int SwapULong( unsigned int in )
{
    char sin[4];
    char sout[4];

    memcpy( (char *)sin, (char *)&in, 4 );

    sout[0] = sin[3];
    sout[1] = sin[2];
    sout[2] = sin[1];
    sout[3] = sin[0];

    return *((unsigned int *)sout);
}

