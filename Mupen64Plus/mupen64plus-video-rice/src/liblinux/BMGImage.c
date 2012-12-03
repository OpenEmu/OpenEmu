/*
// source code for the BMGImage functions
//
// Copyright (C) 2001 Michael S. Heiman
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

#include <string.h>
#include <setjmp.h>
#include <stdlib.h>

#include "BMGUtils.h"

/* initializes a BMGImage to default values */
void InitBMGImage( struct BMGImageStruct *img )
{
    img->width = img->height = 0;
    img->bits_per_pixel = 0;
    img->palette_size = 0;
    img->bytes_per_palette_entry = 0;
    img->bits = NULL;
    img->palette = NULL;
    img->opt_for_bmp = 0;
    img->scan_width = 0;
    img->transparency_index = -1;
}

/* frees memory allocated to a BMGImage */
void FreeBMGImage( struct BMGImageStruct *img )
{
    if ( img->bits != NULL )
    {
        free( img->bits );
        img->bits = NULL;
    }
    if ( img->palette != NULL )
    {
        free( img->palette );
        img->palette = NULL;
    }
    img->bits_per_pixel = 0;
    img->palette_size = 0;
    img->bytes_per_palette_entry = 0;
    img->width = img->height = 0;
    img->opt_for_bmp = 0;
    img->scan_width = 0;
    img->transparency_index = -1;
}

/* allocates memory for the bits & palette.  Assigned values to scan_line
   & bits_per_palette_entry as well.  Assumes opt_for_bmp has been set before
   this function is called. Assumes that all images with bits_per_pixel <= 8
   require a palette.
 */
BMGError AllocateBMGImage( struct BMGImageStruct *img )
{
    unsigned int mempal;

    SetLastBMGError( BMG_OK );

    /* make sure that all REQUIRED parameters are valid */
    if ( img->width * img->height <= 0 )
    {
        SetLastBMGError(errInvalidSize);
        return errInvalidSize;
    }

    switch( img->bits_per_pixel )
    {
        case  1:
        case  4:
        case  8:
        case 16:
        case 24:
        case 32:
            break;
        default:
            SetLastBMGError( errInvalidPixelFormat );
            return errInvalidPixelFormat;
    }

    /* delete old memory */
    if ( img->bits != NULL )
    {
        free( img->bits );
        img->bits = NULL;
    }
    if ( img->palette != NULL )
    {
        free( img->palette );
        img->palette = NULL;
    }

    /* allocate memory for the palette */
    if ( img->bits_per_pixel <= 8 )
    {
        if ( img->opt_for_bmp > 0 )
            img->bytes_per_palette_entry = 4U;
        else
        {
            /* we only support 3-byte and 4-byte palettes */
            if ( img->bytes_per_palette_entry <= 3U )
                img->bytes_per_palette_entry = 3U;
            else
                img->bytes_per_palette_entry = 4U;
        }
        /*
           use bits_per_pixel to determine palette_size if none was
           specified
        */
        if ( img->palette_size == 0 )
            img->palette_size = (unsigned short)(1 << img->bits_per_pixel);

        mempal = img->bytes_per_palette_entry * img->palette_size;
        img->palette = (unsigned char *)calloc( mempal, sizeof(unsigned char) );
        if ( img->palette == NULL )
        {
            SetLastBMGError(errMemoryAllocation);
            return errMemoryAllocation;
        }
    }
    else
    {
        img->bytes_per_palette_entry = 0;
        img->palette_size = 0;
    }

    /*
       set the scan width.  Bitmaps optimized for windows have scan widths that
       are evenly divisible by 4.
    */
    img->scan_width = ( img->bits_per_pixel * img->width + 7 ) / 8;
    if ( img->opt_for_bmp && img->scan_width % 4 )
        img->scan_width += 4 - img->scan_width % 4;

    /* allocate memory for the bits */
    mempal = img->scan_width * img->height;
    if ( mempal > 0 )
    {
        img->bits = (unsigned char *)calloc( mempal, sizeof( unsigned char) );
        if ( img->bits == NULL )
        {
            if ( img->palette != NULL )
            {
                free( img->palette );
                img->palette = NULL;
            }
            SetLastBMGError(errMemoryAllocation);
            return errMemoryAllocation;
        }
    }
    else
    {
        SetLastBMGError(errInvalidSize);
        return errInvalidSize;
    }

    return BMG_OK;
}

/*******************************************************************************
 A utility function for compressing paletted images.  Will automatically
 convert 8-bit paletted images to 1-bit or 4-bit paletted images based
 upon palette_size.  Assumes that indices in img->bits are valid.  That is,
 0 <= img->bits[i] <= 1 for all i if 1-bit compression is desired, and
 0 <= img->bits[i] <= 15 for all i if 4-bit compression is desired  Returns
 BMG_OK if successful, or an error code otherwise.
*******************************************************************************/
BMGError CompressBMGImage( struct BMGImageStruct *img )
{
    unsigned char new_bits_per_pixel;
    unsigned int new_scan_width;
    unsigned char *new_bits = NULL;
    unsigned int new_bit_size;
    unsigned char *new_row, *old_row, *p, *q;
    unsigned char *end;
    unsigned short scale;

    SetLastBMGError( BMG_OK );

    /* if we cannot compress it then do no harm and return "true" */
    if ( img->palette == NULL ||
         img->palette_size > 16 ||
         img->bits_per_pixel != 8 )
    {
        return BMG_OK;
    }

    /* calculate new dimensions */
    new_bits_per_pixel = img->palette_size <= 2 ? 1U : 4U;
    new_scan_width = ( new_bits_per_pixel * img->width + 7 ) / 8;
    if ( img->opt_for_bmp > 0 && new_scan_width % 4 )
        new_scan_width += 4 - new_scan_width % 4;
    new_bit_size = new_scan_width * img->height;

    /* allocate & test memory */
    new_bits = (unsigned char *)calloc( new_bit_size, sizeof(unsigned char) );
    if ( new_bits == NULL )
    {
        SetLastBMGError( errMemoryAllocation );
        return errMemoryAllocation;
    }

    old_row = img->bits;
    for ( new_row = new_bits; new_row < new_bits + new_bit_size;
          new_row += new_scan_width, old_row += img->scan_width )
    {
        scale = 8 / new_bits_per_pixel;
        end = new_row + img->width / scale;
        p = old_row;
        if ( new_bits_per_pixel == 1 )
        {
            for ( q = new_row; q < end; q++, p += scale )
            {
                *q = (unsigned char)( (p[0] << 7) | (p[1] << 6) |
                                      (p[2] << 5) | (p[3] << 4) |
                                      (p[4] << 3) | (p[5] << 2) |
                                      (p[6] << 1) | p[7] );
            }
            scale = img->width % scale;
            if  ( scale-- > 0 )
            {
                *q = (unsigned char)(p[0] << 7);
                if ( scale-- )
                {
                    *q |= (unsigned char)(p[1] << 6);
                    if ( scale-- )
                    {
                        *q |= (unsigned char)(p[2] << 5);
                        if ( scale-- )
                        {
                            *q |= (unsigned char)(p[3] << 4);
                            if ( scale-- )
                            {
                                *q |= (unsigned char)(p[4] << 3);
                                if ( scale-- )
                                {
                                    *q |= (unsigned char)(p[5] << 2);
                                    if ( scale-- )
                                        *q |= (unsigned char)(p[6] << 1);
                                }
                            }
                        }
                    }
                }
            }
        }
        else /* new_bits_per_pixel == 4 */
        {
            for ( q = new_row; q < end; q++, p += scale )
            {
                *q = (unsigned char)( (p[0] << 4) | (p[1] & 0x0F) );
            }
            if  ( img->width % scale )
                *q = (unsigned char)(p[0] << 4);
        }
    }

    /* replace old values with new values */
    free( img->bits );
    img->bits = new_bits;
    img->scan_width = new_scan_width;
    img->bits_per_pixel = new_bits_per_pixel;

    return BMG_OK;
}

/* this function simply frees memory that was allocated by any function
   in the BMGLib.  This was required because acces violations occurred
   when I tried to delete memory created by CreateRGBAArray in the demo
   applications */
void FreeBMGMemory( unsigned char *mem )
{
    if ( mem != NULL )
        free( mem );
}

/* converts a BGR to a gray scale
// color[0] = blue, color[1] = green, color[2] = red */
static unsigned char CreateGrayScale( unsigned char *color )
{
    return (unsigned char)( 0.299f * color[2] + 0.587f * color[1]
                               + 0.114f * color[0] + 0.5f );
}

/*
// converts a color image to a gray scale image.  If img is a 16 or
// 24-BPP image then it is converted to a 256 color grayscale bitmap.
// If img is a 1, 4, or 8 BPP image, then it will have the same number
// of grayscales as it has palette entries.  If it is a 32-BPP bitmap then
// it will remain a 32-BPP bitmap to preserve the alpha channel.
//
// This function returns BMG_OK if successfull, or an error state
// otherwise.
*/
BMGError ConvertToGrayScale( struct BMGImageStruct *img )
{
    unsigned char *p, *q, *r, *end, gray;

    SetLastBMGError( BMG_OK );

    /* if this is a paletted image then we simply need to convert the
    // palette entries */
    switch ( img->bits_per_pixel )
    {
    default:
        end = img->palette + img->palette_size * img->bytes_per_palette_entry;
        for ( p = img->palette; p < end; p += img->bytes_per_palette_entry )
        {
            gray = CreateGrayScale( p );
            memset( (void *)p, gray, 3 );
        }
        break;
    /* 16 BPP image are converted to 24 BPP images */
    case 16:
    {
        BMGError tmp = Convert16to24( img );
        if ( tmp != BMG_OK )
        {
            SetLastBMGError( tmp );
            return tmp;
        }
    }
    case 24:
    {
        unsigned char *new_bits;
        unsigned char *s, *s_end;
        unsigned short i;

        /* calculate the new scan width */
        unsigned int new_scan_width = img->width;
        if ( new_scan_width % 4 && img->opt_for_bmp )
            new_scan_width += 4 - new_scan_width % 4;

        /* allocate memory for the new pixel values */
        new_bits = (unsigned char *)calloc( new_scan_width * img->height,
                    sizeof(unsigned char) );
        if ( new_bits == NULL )
        {
            SetLastBMGError( errMemoryAllocation );
            return errMemoryAllocation;
        }

        /* allocate memory for a 256 gray scale palette */
        img->bytes_per_palette_entry = img->opt_for_bmp == 1 ? 4 : 3;
        img->palette_size = 256;
        img->palette =
            (unsigned char *)calloc(img->bytes_per_palette_entry *
                                    img->palette_size,
                                    sizeof(unsigned char) );
        if ( img->palette == NULL )
        {
            free( new_bits );
            img->bytes_per_palette_entry = 0;
            img->palette_size = 0;
            SetLastBMGError( errMemoryAllocation );
            return errMemoryAllocation;
        }

        /* assign values to the gray scale palette */
        for ( i = 0; i < 256; i++ )
        {
            p = img->palette + i * img->bytes_per_palette_entry;
            memset( (void *)p, i, 3 );
            if ( img->bytes_per_palette_entry == 4 )
                p[3] = 0;
        }

        /* cycle through the pixels and convert them to gray scale values */
        q = new_bits;
        end = img->bits + img->scan_width * img->height;

        for ( p = img->bits; p < end; p += img->scan_width, q += new_scan_width )
        {
            s_end = p + 3 * img->width;
            r = q;
            for ( s = p; s < s_end; s += 3, r++ )
                *r = CreateGrayScale( s );
        }

        free( img->bits );
        img->bits = new_bits;
        img->scan_width = new_scan_width;
        img->bits_per_pixel = 8;

        break;
    }
    case 32:
        end = img->bits + img->scan_width * img->height;
        for ( p = img->bits; p < end; p += img->scan_width )
        {
            r = p + img->scan_width;
            for ( q = p; q < r; q += 4 )
            {
                gray = CreateGrayScale( q );
                memset( (void *)q, gray, 3 );
            }
        }
        break;
    }

    return BMG_OK;
}

/*
// converts a color image to a pseudo-gray scale image.  This is a implementation
// is based upon the code published by Rich Franzen
// <http://rocq.home.att.net/pseudoGrey.html>. I have "simplified" the 2 functions
// he published into a single function.  This implementation creates 1786 gray
// scales from a 24-bit image. 16-BPP images are converted to 24-BPP images.  24
// and 32-BPP images will keep the same bitdepth. Paletted images and 16-BPP images
// are not supported.
//
// This function returns BMK_OK if successfull,
// errInvalidPixelFormat otherwise
*/
BMGError ConvertToPseudoGrayScale( struct BMGImageStruct *img )
{
    unsigned char *p, *p_end;
    unsigned char *q, *q_end;
    unsigned char gray;
    unsigned int bytes_per_pixel;

    SetLastBMGError( errMemoryAllocation );

    if ( img->bits_per_pixel <= 16 )
    {
        SetLastBMGError( errInvalidPixelFormat );
        return errInvalidPixelFormat;
    }

    bytes_per_pixel = img->bits_per_pixel / 8;
    p_end = img->bits + img->scan_width * img->height;

    for ( p = img->bits; p < p_end; p += img->scan_width )
    {
        q_end = p + bytes_per_pixel * img->width;
        for ( q = p; q < q_end; q += bytes_per_pixel )
        {
        /* Rich's code has 1 function that converts an RGB triplet to a float
        // bounded by 0 and 1.  He has a second function that converts a
        // float to a pseudo gray value.  Pseudo gray values are RGB triplets
        // whose red, green and blue values differ by no more than 1.  I have
        // combined these two functions into a single function that simply
        // looks for pseudo gray RGB triplets.  If an RGB triplet meets this
        // criteria, I leave it unchanged; otherwise, I use the common intensity
        // conversion to create a grayscale value */
            unsigned char cmin, cmax;

            cmin = q[0];
            if ( q[1] < cmin )
                cmin = q[1];
            if ( q[2] < cmin )
                cmin = q[2];

            cmax = q[0];
            if ( q[1] > cmax )
                cmax = q[1];
            if ( q[2] > cmax )
                cmax = q[2];

            if ( cmax - cmin > 2 )
            {
                gray = CreateGrayScale( q );
                memset( (void *)q, gray, 3 );
            }
        }
    }

    return BMG_OK;
}

#ifdef _WIN32
/*******************************************************************************
// extracts the dimensional information, pixel array, and color table from an
// HBITMAP.
// hBitmap can be a handle to a DIB or a DDB.  This function assumes that DDBs
// will not have a palette.  If you create a DDB on a 256-color graphics card,
// then the DDB will have a palette and this function will fail.
//
// returns BMK_OK if successfull, and error state otherwise.
********************************************************************************/
BMGError GetDataFromBitmap( HBITMAP hBitmap,
                            struct BMGImageStruct *img,
                            int remove_alpha )
{
    unsigned int        DIBScanWidth;
    DIBSECTION          DS;
    HWND                hWnd = GetForegroundWindow();
    HDC                 hDC = NULL;
    HDC                 hMemDC = NULL;
    unsigned char       red, green, blue;
    int                 FreelpBits = 0;
    unsigned int        numBytes;
    size_t              soDIBSECTION = sizeof(DIBSECTION);
    size_t              soBITMAP = sizeof(BITMAP);

    unsigned char *p, *q, *lpBits, alpha;

    jmp_buf err_jmp;
    int error;
    BMGError bmgerr;

    /* error handler */
    error = setjmp( err_jmp );
    if ( error != 0 )
    {
        if ( hMemDC != NULL )
            DeleteDC( hMemDC );
        if ( hDC != NULL )
            ReleaseDC( hWnd, hDC );
        if ( FreelpBits )
            free( lpBits );
        FreeBMGImage( img );
        SetLastBMGError( (BMGError)error );
        return (BMGError)error;
    }

    SetLastBMGError( BMG_OK );
    /* check for valid bitmap*/
    if ( !hBitmap )
        longjmp( err_jmp, (int)errInvalidBitmapHandle );

    /* Extract DIBSECTION info from the HBITMAP.  numBytes will equal
    // soDIBSECTION (84) if hBitmap is a handle to a DIBSECTION (DIB).
    // numBytes will equal soBITMAP (24) if hBitmap is a handle to a
    // BITMAP (DDB). */
    numBytes = GetObject( hBitmap, sizeof(DIBSECTION), &DS );
    if ( numBytes == 0 )
        longjmp( err_jmp, (int)errWindowsAPI );

    img->opt_for_bmp = 1;
    if ( numBytes == soDIBSECTION )
    {
        img->width = DS.dsBmih.biWidth;
        img->height = DS.dsBmih.biHeight;
        img->bits_per_pixel = (unsigned char)DS.dsBmih.biBitCount;
        if ( img->bits_per_pixel <= 8 && DS.dsBmih.biClrUsed > 0 )
            img->palette_size = (unsigned short)DS.dsBmih.biClrUsed;
        lpBits = (unsigned char *)DS.dsBm.bmBits;
    }
    /* this may be a DDB which must be handled differently */
    else if ( numBytes == soBITMAP )
    {
        BITMAP bm;
        BITMAPINFO bmi;

        if ( GetObject( hBitmap, sizeof(BITMAP), &bm ) == 0 )
            longjmp( err_jmp, (int)errWindowsAPI );

        /* DDB with a palette */
        if ( bm.bmBitsPixel <= 8 )
            longjmp( err_jmp, (int)errInvalidPixelFormat );

        img->width = bm.bmWidth;
        img->height = bm.bmHeight;
        img->bits_per_pixel = (unsigned char)bm.bmBitsPixel;
        bmi = InternalCreateBMI( bm.bmWidth, bm.bmHeight, bm.bmBitsPixel,
                                 BI_RGB );

        lpBits = (unsigned char *)calloc( bm.bmHeight * bm.bmWidthBytes,
                                          sizeof(unsigned char) );
        if ( lpBits == 0 )
            longjmp( err_jmp, (int)errMemoryAllocation );
        FreelpBits = 1;
        hDC = GetDC( hWnd );
        if ( GetDIBits(hDC, hBitmap, 0, bm.bmHeight, (void *)lpBits, &bmi,
                       DIB_RGB_COLORS ) == 0 )
            longjmp( err_jmp, (int)errWindowsAPI );
        ReleaseDC( hWnd, hDC );
        hDC = NULL;
    }
    else /* I have no idea what this is */
        longjmp( err_jmp, (int)errInvalidBitmapHandle );

    /* allocate memory */
    bmgerr = AllocateBMGImage( img );
    if ( bmgerr != BMG_OK )
        longjmp( err_jmp, (int)bmgerr );

    /* dimensions */
    DIBScanWidth = ( img->width * img->bits_per_pixel + 7 )/8;
    if ( DIBScanWidth % 4 )
        DIBScanWidth += 4 - DIBScanWidth % 4;

    p = img->bits;
    for ( q = lpBits; q < lpBits + DIBScanWidth * img->height;
            p += img->scan_width, q += DIBScanWidth )
    {
        memcpy( (void *)p, (void *)q, DIBScanWidth );
    }

    /* "un-blend" the image if requested.  NOTE: unblending only works with
    // bland backgrounds */
    if ( remove_alpha > 0 &&
         img->bits_per_pixel == 32 &&
         numBytes == soDIBSECTION )
    {
        unsigned char *color = GetBackgroundColor();
        red   = color[2];
        green = color[1];
        blue  = color[0];

        for ( p = img->bits; p < img->bits + img->scan_width * img->height;
              p += 4 )
        {
            alpha = p[3];
            p[2] = InverseAlphaComp( p[2], alpha, blue);
            p[1] = InverseAlphaComp( p[1], alpha, green);
            p[0] = InverseAlphaComp( p[0], alpha, red);
        }
    }

    /* 32-bit DDBs must have the alpha channel set to 0xFF before they are
    // saved to a file. This may not be true for all devices that generate
    // 32-bit DDBs.  I have only created 32-bit DDBs using an Intense3D Wildcat
    // 4110 card.  The alpha channel was always 0. */
    if (img->bits_per_pixel == 32 && numBytes == soBITMAP )
    {
        for ( p = img->bits + 3; p < img->bits + img->scan_width * img->height;
                                 p += 4 )
        {
            *p = 0xFF;
        }
    }

    /* create palette if necessary */
    if ( img->bits_per_pixel <= 8 )
    {
        hDC = GetDC( hWnd );
        hMemDC = CreateCompatibleDC( hDC );
        SelectObject( hMemDC, hBitmap );
        if ( !GetDIBColorTable( hMemDC, 0, img->palette_size,
                                (RGBQUAD *)img->palette ) )
        {
            longjmp( err_jmp, (int)errWindowsAPI );
        }
        DeleteDC( hMemDC );
        ReleaseDC( hWnd, hDC );
    }

    if ( FreelpBits )
        free( lpBits );

    return BMG_OK;
}

/*******************************************************************************
// this function creates a bitmap from raw data. Returns an HBITMAP if it
// succeeds, otherwise NULL */
HBITMAP CreateBitmapFromData( struct BMGImageStruct img,
                              int alpha_blend )
{
    HBITMAP hBitmap = NULL;
    HDC hMemDC = NULL;
    HWND hWnd = GetForegroundWindow();
    HDC hDC = NULL;
    RGBQUAD *pColor = NULL;
    BITMAPINFO bmi;
    unsigned char *rbits;
    unsigned char *bits;
    unsigned char *lpBits;
    unsigned char alpha;
    unsigned int DIBScanWidth;
    int i;

    jmp_buf err_jmp;
    int error;

    /* error handler */
    error = setjmp( err_jmp );
    if ( error != 0 )
    {
        if ( hMemDC != NULL )
            DeleteDC( hMemDC );
        if ( hDC != NULL )
            ReleaseDC( hWnd, hDC );
        if ( pColor != NULL && img.bytes_per_palette_entry == 3U )
            free( pColor );
        SetLastBMGError( (BMGError)error );
        return 0;
    }

    SetLastBMGError( BMG_OK );

    /* create the DIB section that will hold this bitmap */
    bmi = InternalCreateBMI( (unsigned int)img.width, (unsigned int)img.height,
                              (unsigned short)img.bits_per_pixel, BI_RGB );
    bmi.bmiHeader.biClrUsed = bmi.bmiHeader.biClrImportant =
         img.palette_size;
    hDC = GetDC( hWnd );
    hBitmap = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS,
                                   (void **)&lpBits, NULL, 0 );

    if ( !hBitmap || !lpBits )
        longjmp( err_jmp, (int)errWindowsAPI );

    /* create a palette if needed */
    if ( img.palette != NULL )
    {
        /* copy pixel data to pColor */
        if ( img.bytes_per_palette_entry == 4U )
            pColor = (RGBQUAD *)img.palette;
        else /* bytes_per_palette_entry === 3 */
        {
            pColor = (RGBQUAD *)calloc(img.palette_size, sizeof(RGBQUAD) );
            if ( pColor == NULL )
                longjmp( err_jmp, (int)errMemoryAllocation );

            bits = img.palette;
            for ( i = 0; i < (int)bmi.bmiHeader.biClrUsed; i++, bits += 3 )
            {
                pColor[i].rgbRed   = bits[0];
                pColor[i].rgbGreen = bits[1];
                pColor[i].rgbBlue  = bits[2];
            }
        }

        if ( img.transparency_index > -1 )
        {
            unsigned char *color = GetBackgroundColor();
            rbits = img.palette + img.bytes_per_palette_entry *
                    img.transparency_index;
            rbits[0] = color[2];
            rbits[1] = color[1];
            rbits[2] = color[0];
        }
        /* save color table in bitmap */
        hMemDC = CreateCompatibleDC( hDC );
        SelectObject( hMemDC, hBitmap );
        if ( !SetDIBColorTable( hMemDC, 0, img.palette_size, pColor ) )
            longjmp( err_jmp, (int)errWindowsAPI );

        DeleteDC( hMemDC );
        hMemDC = NULL;
        if ( img.bytes_per_palette_entry == 3U )
            free( pColor );
        pColor = NULL;
    }

    /* calculate the scan line width */
    DIBScanWidth = img.scan_width;
    if ( DIBScanWidth % 4 )
        DIBScanWidth += 4 - DIBScanWidth % 4;

    if ( img.opt_for_bmp == 0 )
    {
        /* store bits into hBitmap */
        rbits = img.bits;
        for ( bits = lpBits;
              bits < lpBits + img.height * DIBScanWidth;
              bits += DIBScanWidth, rbits += img.scan_width )
        {
            memcpy( (void *)bits, (void *)rbits, img.scan_width );
        }
    }
    else
        memcpy( (void *)lpBits, (void *)img.bits, img.scan_width * img.height );

    /* blend the image with the window background if alpha pixels
    // are present */
    if ( img.bits_per_pixel == 32 )
    {
        /* blend with a bland background */
        if ( alpha_blend == 1 )
        {
            unsigned char *color = GetBackgroundColor();
            unsigned char red   = color[2];
            unsigned char green = color[1];
            unsigned char blue  = color[0];

            for ( rbits = lpBits;
                  rbits < lpBits + img.height*DIBScanWidth;
                  rbits += DIBScanWidth )
            {
                for ( bits = rbits; bits < rbits + DIBScanWidth; bits += 4 )
                {
                    alpha = bits[3];
                    bits[2] = AlphaComp( bits[2], alpha, blue );
                    bits[1] = AlphaComp( bits[1], alpha, green );
                    bits[0] = AlphaComp( bits[0], alpha, red );
                }
            }
        }
        /* blend with a background image */
        else if ( alpha_blend == 2 )
        {
            unsigned char *bg_bits;
            unsigned char *bg_bits_2;
            unsigned int bg_bytes_per_pixel;
            struct BMGImageStruct *bg = GetBackgroundImage();

            /* make sure we can blend with a background image
            // I assume that the background image is invalid if it does not
            // have a valid width */
            if ( bg->width <= 0 || bg->height <= 0 )
                longjmp( err_jmp, (int)errUndefinedBGImage );

            /* I cannot blend a foreground image with a background image that
            // is smaller than it */
            if ( bg->width < img.width || bg->height < img.height )
                longjmp( err_jmp, (int)errBGImageTooSmall );

            /* the background image was forced to be a 24 or 32-BPP image;
            // therefore, we can safely divide by 8 to determined the
            // bytes per pixel*/
            bg_bytes_per_pixel = bg->bits_per_pixel / 8;

            /* I will assume that the upper left corner of the input image
            // must be aligned with the upper left corner of the background
            // image.  This allows me to have background images that are bigger
            // than the input image. */
            bg_bits = bg->bits;
            for ( rbits = lpBits;
                  rbits < lpBits + img.height*DIBScanWidth;
                  rbits += DIBScanWidth, bg_bits += bg->scan_width )
            {
                bg_bits_2 = bg_bits;
                for ( bits = rbits; bits < rbits + DIBScanWidth;
                      bits += 4, bg_bits_2 += bg_bytes_per_pixel )
                {
                    alpha = bits[3];
                    bits[2] = AlphaComp( bits[2], alpha, bg_bits_2[2] );
                    bits[1] = AlphaComp( bits[1], alpha, bg_bits_2[1] );
                    bits[0] = AlphaComp( bits[0], alpha, bg_bits_2[0] );
                }
            }

        }
    }

    ReleaseDC( hWnd, hDC );

    return hBitmap;
}
#endif // _WIN32
/******************************************************************************
//  ConvertPaletteToRGB converts paletted and 16-BPP images that do not have
// transparent pixels to 24-BPP images.  Paletted images with transparent pixels
// are converted to 32-BPP images.  24-BPP and 32-BPP images are simply copied
// to the output structure
//
// INPUTS:
//  img_in
// OUTPUTS:
//  img_out
//
// returns BMG_OK if no errors occur, an error code otherwise
******************************************************************************/
BMGError ConvertPaletteToRGB( struct BMGImageStruct img_in,
                              struct BMGImageStruct *img_out )
{
    jmp_buf err_jmp;
    int error;

    /* error handler */
    error = setjmp( err_jmp );
    if ( error != 0 )
    {
        FreeBMGImage( img_out );
        SetLastBMGError( (BMGError)error );
        return (BMGError)error;
    }

    SetLastBMGError( BMG_OK );

    if ( img_in.height == 0 || img_in.width == 0 )
        longjmp( err_jmp, (int)errInvalidSize );

    InitBMGImage( img_out );

    // copy 16, 24, and 32-BPP images into the output image
    if ( img_in.bits_per_pixel > 8 )
    {
        BMGError out;
        img_out->bits_per_pixel = img_in.bits_per_pixel;
        out = CopyBMGImage( img_in, img_out );
        if ( out != BMG_OK )
            longjmp( err_jmp, (int)out );

        // 16-BPP are converted to 24-BPP images
        if ( img_out->bits_per_pixel == 16 )
        {
            out = Convert16to24( img_out );
            if ( out != BMG_OK )
                longjmp( err_jmp, (int)out );
        }
    }
    else // convert paletted images to 24-BPP BGR or 32-BPP BGRA images
    {
        BMGError out;
        unsigned char *buf;
        unsigned int scan_width;
        int dealloc;
        unsigned char *q0, *q1, *p0, *p1;
        unsigned int bpp;

        // allocate memory for the 24-BPP output image
        img_out->width  = img_in.width;
        img_out->height = img_in.height;
        img_out->opt_for_bmp = img_in.opt_for_bmp;
        img_out->bits_per_pixel = img_in.transparency_index > -1 ? 32 : 24;

        out = AllocateBMGImage( img_out );
        if ( out != BMG_OK )
            longjmp( err_jmp, (int)out );

        // 1-BPP and 4-BPP images are packed, so we need to unpack them
        if ( img_in.bits_per_pixel < 8 )
        {
            dealloc = 1;
            scan_width = img_in.width;
            buf = (unsigned char *)malloc(scan_width * img_in.height);
            if ( buf == NULL )
                longjmp( err_jmp, (int)errMemoryAllocation );

            if ( img_in.bits_per_pixel == 1 )
                Convert1to8( img_in, buf );
            else
                Convert4to8( img_in, buf );
        }
        else // simply point to the bits array if we have a 8-BPP image
        {
            dealloc = 0;
            buf = img_in.bits;
            scan_width = img_in.scan_width;
        }

        // convert palette indices to BGR pixels
        bpp = img_out->bits_per_pixel / 8;
        q0 = img_out->bits;
        for ( p0 = buf; p0 < buf + scan_width * img_in.height;
                    p0 += scan_width, q0 += img_out->scan_width )
        {
            q1 = q0;
            for ( p1 = p0; p1 < p0 + img_in.width; p1++, q1 += bpp )
            {
                memcpy( (void *)q1,
                    (void *)(img_in.palette + *p1 * img_in.bytes_per_palette_entry), 3 );
                if ( bpp == 4 )
                {
                    q1[3] = *p1 == img_in.transparency_index ? 0 : 0xFF;
                }
            }
        }

        if ( dealloc == 1 )
            free( buf );
    }

    return BMG_OK;
}

/******************************************************************************
// CopyBMG copies the contents of img_in into img_out.
//
// CopyBMG returns BMG_OK if successful, otherwise, it returns an error code
******************************************************************************/
BMGError CopyBMGImage( struct BMGImageStruct img_in,
                       struct BMGImageStruct *img_out )
{
    BMGError out = BMG_OK;
    SetLastBMGError( out );

    FreeBMGImage( img_out );

    img_out->height = img_in.height;
    img_out->width = img_in.width;
    img_out->bits_per_pixel = img_in.bits_per_pixel;
    img_out->palette_size = img_in.palette_size;
    img_out->opt_for_bmp = img_in.opt_for_bmp;

    if ( img_in.width > 0 && img_in.height > 0 )
    {
        out = AllocateBMGImage( img_out );
        if ( out == BMG_OK )
        {
            memcpy( (void *)img_out->bits, (void *)img_in.bits,
                img_in.scan_width * img_in.height );
            if ( img_in.palette_size > 0 )
                memcpy( (void *)img_out->palette, (void *)img_in.palette,
                    img_in.palette_size * img_in.bytes_per_palette_entry );
        }
    }

    return out;
}

/* sets the background color for alpha blending
  color points to an array of 4 unsigned chars
  color[0] = blue, color[1] = green, color[2] = red, color[3] = unused */
void SetBMGBackgroundColor( unsigned char *color )
{
    memcpy( (void *)GetBackgroundColor(), (void *)color,
            4*sizeof(unsigned char) );
}

#ifdef _WIN32
/* defines the background bitmap that is used for alpha blending & transparent
   pixels */
BMGError SetBMGBackgroundBitmap( HBITMAP hBitmap )
{
    BMGError out;
    struct BMGImageStruct tmp;
    InitBMGImage( &tmp );

    /* first we extract the data from the HBITMAP */
    out = GetDataFromBitmap( hBitmap, &tmp, 0 );
    if ( out == BMG_OK )
    {
        /* clean up the old background image */
        FreeBMGImage( GetBackgroundImage() );

        /* next, we convert paletted & 16-BPP images to 24 or 32-BPP images.
        // this will simplify the alpha blending. */
        out = ConvertPaletteToRGB( tmp, GetBackgroundImage() );
    }

    return out;
}
#endif // _WIN32

/* defines the background image that is used for alpha blending & transparent
   pixels */
BMGError SetBMGBackgroundImage( struct BMGImageStruct img )
{
    /* clean up the old background image */
    FreeBMGImage( GetBackgroundImage() );

    /* convert paletted and 16-BPP images to 24-BPP or 32-BPP images.  This
    // will simplify the alpha blending logic*/
    return ConvertPaletteToRGB( img, GetBackgroundImage() );
}

