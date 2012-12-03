/*
//  source code for the ImageLib BMP functions
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

#include "BMGDLL.h"
#include "BMGUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#ifndef _WIN32
#include <string.h>
#endif // _WIN32

static const unsigned short BMP_ID = 0x4D42;

/*
    ReadBMP - reads the image data from a BMP files and stores it in a
              BMGImageStruct.

    Inputs:
        filename    - the name of the file to be opened

    Outputs:
        img         - the BMGImageStruct containing the image data

    Returns:
        BMGError - if the file could not be read or a resource error occurred
        BMG_OK   - if the file was read and the data was stored in img

    Limitations:
        will not read BMP files using BI_RLE8, BI_RLE4, or BI_BITFIELDS
*/
BMGError ReadBMP( const char *filename,
              struct BMGImageStruct *img )
{
    FILE *file = NULL;
    int error;
    BMGError tmp;
    unsigned char *p, *q; /*, *q_end; */
/*    unsigned int cnt; */
    int i;
/*    int EOBMP; */

    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
/*
    unsigned int mask[3];
*/

    unsigned int DIBScanWidth;
    unsigned int bit_size, rawbit_size;
    unsigned char *rawbits = NULL;

    SetLastBMGError( BMG_OK );

    if ( img == NULL )
        { error = (int) errInvalidBMGImage; goto err_jmp; }


    file = fopen( filename, "rb" );
    if  ( file == NULL )
        { error = (int) errFileOpen; goto err_jmp; }

        /* read the file header */
    if ( fread( (void *)&bmfh, sizeof(BITMAPFILEHEADER), 1, file ) != 1 )
        { error = (int) errFileRead; goto err_jmp; }

    /* confirm that this is a BMP file */
    if ( bmfh.bfType != BMP_ID )
        { error = (int) errUnsupportedFileFormat; goto err_jmp; }

    /* read the bitmap info header */
    if ( fread( (void *)&bmih, sizeof(BITMAPINFOHEADER), 1, file ) != 1 )
        { error = (int) errFileRead; goto err_jmp; }

    /* abort if this is an unsupported format */
    if ( bmih.biCompression != BI_RGB )
        { printf("planes: %i  bits: %i  type: %i   ", bmih.biPlanes, bmih.biBitCount, bmih.biCompression); error = (int) errUnsupportedFileFormat; goto err_jmp; }

    img->bits_per_pixel = (unsigned char)bmih.biBitCount;
    img->width  = bmih.biWidth;
    img->height = bmih.biHeight;
    if ( img->bits_per_pixel <= 8 )
    {
        img->palette_size = (unsigned short)bmih.biClrUsed;
        img->bytes_per_palette_entry = 4U;
    }

    tmp = AllocateBMGImage( img );
    if ( tmp != BMG_OK )
        { error = (int) tmp; goto err_jmp; }

    /* read palette if necessary */
    if ( img->bits_per_pixel <= 8 )
    {
        if ( fread( (void *)img->palette, sizeof(RGBQUAD), img->palette_size,
                file ) != (unsigned int)img->palette_size )
        {
          error = (int) errFileRead;
          goto err_jmp;
        }
    }

    /* dimensions */
    DIBScanWidth = ( img->bits_per_pixel * img->width + 7 ) / 8;
    if ( DIBScanWidth %4 )
        DIBScanWidth += 4 - DIBScanWidth % 4;

    bit_size = img->scan_width * img->height;

    /* allocate memory for the raw bits */
    if ( bmih.biCompression != BI_RGB )
        rawbit_size = bmfh.bfSize - bmfh.bfOffBits;
    else
        rawbit_size = DIBScanWidth * img->height;

    rawbits = (unsigned char *)calloc( rawbit_size, 1 );
    if ( rawbits == NULL )
        { error = (int) errMemoryAllocation; goto err_jmp; }

    if ( fread( (void *)rawbits, sizeof(unsigned char), rawbit_size, file )
                   != rawbit_size )
    {
        error = (int) errFileRead;
        goto err_jmp;
    }

    if ( bmih.biCompression == BI_RGB )
    {
        p = rawbits;
        for ( q = img->bits; q < img->bits + bit_size;
                         q += img->scan_width, p += DIBScanWidth )
        {
            memcpy( (void *)q, (void *)p, img->scan_width );
        }
    }

    /* swap rows if necessary */
    if ( bmih.biHeight < 0 )
    {
        for ( i = 0; i < (int)(img->height) / 2; i++ )
        {
            p = img->bits + i * img->scan_width;
            q = img->bits + ((img->height) - i - 1 ) * img->scan_width;
            memcpy( (void *)rawbits, (void *)p, img->scan_width );
            memcpy( (void *)p, (void *)q, img->scan_width );
            memcpy( (void *)q, (void *)rawbits, img->scan_width );
        }
    }

    fclose( file );
    free( rawbits );
    return BMG_OK;

  /* error handler */
err_jmp:
    if ( file != NULL )
        fclose( file );
    if ( rawbits != NULL )
        free( rawbits );
    FreeBMGImage( img );
    SetLastBMGError( (BMGError)error );
    return (BMGError)error;

}

/*
    WriteBMP - writes the contents of an BMGImageStruct to a bmp file.

    Inputs:
        filename    - the name of the file to be opened
        img         - the BMGImageStruct containing the image data

    Returns:
        BMGError - if a write error or a resource error occurred
        BMG_OK   - if the data was successfilly stored in filename

    Limitations:
        will not write BMP files using BI_RLE8, BI_RLE4, or BI_BITFIELDS
*/
BMGError WriteBMP( const char *filename,
                   struct BMGImageStruct img )
{
    FILE * volatile file = NULL;
    jmp_buf err_jmp;
    int error;

    unsigned char * volatile bits = NULL;
    unsigned int DIBScanWidth;
    unsigned int BitsPerPixel;
    unsigned int bit_size; /*, new_bit_size; */
/*    unsigned int rawbit_size; */
    unsigned char *p, *q, *r, *t;
/*    unsigned int cnt;  */
    unsigned char * volatile pColor = NULL;

    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;

    SetLastBMGError( BMG_OK );

    /* error handler */
    error = setjmp(err_jmp);
    if (error != 0)
    {
        if (file != NULL)
            fclose(file);
        if (bits != NULL)
            free(bits);
        if (pColor != NULL)
            free(pColor);
        SetLastBMGError((BMGError)error);
        return (BMGError) error;
    }

    if ( img.bits == NULL )
        longjmp( err_jmp, (int)errInvalidBMGImage );

    file = fopen( filename, "wb" );
    if ( file == NULL )
        longjmp( err_jmp, (int)errFileOpen );

    /* abort if we do not support the data */
    if ( img.palette != NULL && img.bytes_per_palette_entry < 3 )
        longjmp( err_jmp, (int)errInvalidBMGImage );

    /* calculate dimensions */
    BitsPerPixel = img.bits_per_pixel < 32 ? img.bits_per_pixel : 24U;
    DIBScanWidth = ( BitsPerPixel * img.width + 7 ) / 8;
    if ( DIBScanWidth % 4 )
        DIBScanWidth += 4 - DIBScanWidth % 4;
    bit_size = DIBScanWidth * img.height;
/*    rawbit_size = BITScanWidth * img.height; */

    /* allocate memory for bit array - assume that compression will
    // actually compress the bitmap */
    bits = (unsigned char *)calloc( bit_size, 1 );
    if ( bits == NULL )
        longjmp( err_jmp, (int)errMemoryAllocation );

    /* some initialization */
    memset( (void *)&bmih, 0, sizeof(BITMAPINFOHEADER) );
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = img.width;
    bmih.biHeight = img.height;
    bmih.biPlanes = 1;
    /* 32-bit images will be stored as 24-bit images to save space.  The BMP
       format does not use the high word and I do not want to store alpha
       components in an image format that does not recognize it */
    bmih.biBitCount = BitsPerPixel;
    bmih.biCompression = BI_RGB; // assumed
    bmih.biSizeImage = bit_size; // assumed
    bmih.biClrUsed = img.palette == NULL ? 0 : img.palette_size;
    bmih.biClrImportant = img.palette == NULL ? 0 : img.palette_size;

    /* if we are not compressed then copy the raw bits to bits */
    if ( bmih.biCompression == BI_RGB )
    {
        p = img.bits;
        /* simple memcpy's for images containing < 32-bits per pixel */
        if ( img.bits_per_pixel < 32 )
        {
            for ( q = bits; q < bits + bit_size; q += DIBScanWidth,
                                                 p += img.scan_width )
            {
                memcpy( (void *)q, (void *)p, img.scan_width );
            }
        }
        /* store 32-bit images as 24-bit images to save space. alpha terms
           are lost */
        else
        {
            DIBScanWidth = 3 * img.width;
            if ( DIBScanWidth % 4 )
                DIBScanWidth += 4 - DIBScanWidth % 4;

            for ( q = bits; q < bits + bit_size; q += DIBScanWidth,
                                                 p += img.scan_width )
            {
                t = p;
                for ( r = q; r < q + DIBScanWidth; r += 3, t += 4 )
                    memcpy( (void *)r, (void *)t, 3 );
            }
        }
    }

    /* create the palette if necessary */
    if ( img.palette != NULL )
    {
        pColor = (unsigned char *)calloc( img.palette_size, sizeof(RGBQUAD) );
        if ( pColor == NULL )
            longjmp( err_jmp, (int)errMemoryAllocation );

        if ( img.bytes_per_palette_entry == 3 )
        {
            p = img.palette;
            for ( q = pColor + 1; q < pColor +img.palette_size*sizeof(RGBQUAD);
                            q += sizeof(RGBQUAD), p += 3 )
            {
                memcpy( (void *)pColor, (void *)p, 3 );
            }
        }
        else /* img.bytes_per_palette_entry == 4 */
        {
            memcpy( (void *)pColor, (void *)img.palette,
                img.palette_size * sizeof(RGBQUAD) );
        }
    }

    /* now that we know how big everything is let's write the file */
    memset( (void *)&bmfh, 0, sizeof(BITMAPFILEHEADER) );
    bmfh.bfType = BMP_ID;
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
                     img.palette_size * sizeof(RGBQUAD);
    bmfh.bfSize = bmfh.bfOffBits + bit_size;

    if ( fwrite( (void *)&bmfh, sizeof(BITMAPFILEHEADER), 1, file ) != 1 )
        longjmp( err_jmp, (int)errFileWrite );

    if ( fwrite( (void *)&bmih, sizeof(BITMAPINFOHEADER), 1, file ) != 1 )
        longjmp( err_jmp, (int)errFileWrite );

    if ( pColor != NULL )
    {
        if ( fwrite( (void *)pColor, sizeof(RGBQUAD), img.palette_size, file )
                              != (unsigned int)img.palette_size )
        {
            longjmp( err_jmp, (int)errFileWrite );
        }
    }

    if ( fwrite( (void *)bits, sizeof(unsigned char), bit_size, file )
                    != bit_size )
    {
        longjmp( err_jmp, (int)errFileWrite );
    }

    fclose( file );
    free( bits );
    if ( pColor != NULL )
        free( pColor );
    return BMG_OK;
}

#ifdef _NEVER_DEFINE_THIS_DEF_
/* following code is not tested.  I keep it here in case I ever find a BMP
  file that is compressed and I want to test it */
    else if ( bmih.biCompression == BI_RLE8 )
    {
        bmih.biCompression = BI_RGB;
        bmih.biSizeImage = DIBScanWidth * img.height;
        p = rawbits;
        q = img.bits;
        EOBMP = 1;
        while ( q < img.bits + bit_size && p < rawbits + rawbit_size && EOBMP )
        {
            cnt = (unsigned int)*p;
            p++;

            /* encoded mode */
            if ( cnt == 0 )
            {
                cnt = (unsigned int)*p;
                if ( cnt < 3U )
                {
                    p++;
                    /* EOL */
                    if ( *p == 0 )
                        p++;
                    /* end of bitmap */
                    else if ( *p == 1 )
                        EOBMP = 0;
                    /* delta */
                    else if ( *p == 2 )
                    {
                        p++;
                        q += *p;  /* columns */
                        p++;
                        q += (*p)*BITScanWidth; /* rows */
                        p++;
                    }
                }
                /* copy *p duplicates of *(p++) into the bit array */
                else
                {
                    cnt = (unsigned int)*p;
                    p++;
                    q_end = q + cnt;
                    while ( q < q_end )
                        *q++ = *p;
                    p++;
                }
            }
            /* absolute mode */
            else
            {
                q_end = q + cnt;
                while ( q < q_end )
                    *q++ = *p++;
            }
        }
    }

    /* if compression is desired && possible then attempt compression.  The
    // following logic will try to compress the data.  If the compressed data
    // requires more space than the uncompressed data then the bits will be
    // stored in an uncompressed format */
    if ( try_compression != 0 && img.bits_per_pixel == 8 )
    {
        p = rawbits;
        r = bits;
        new_bit_size = 0;
        cnt = 0;
        while ( p < rawbits + rawbit_size && new_bit_size < bit_size )
        {
            q = p;
            while ( q < p + BITScanWidth )
            {
                t = q;
                while ( t < q + 255 && t < p + BITScanWidth )
                {
                    /* look for non-repeats - absolute mode */
                    if ( *t != *(t+1) )
                    {
                        while ( *t != *(t+1) &&
                                cnt < 255 &&
                                t < p + BITScanWidth )
                        {
                            t++;
                            cnt++;
                        }
                        cnt++;
                        *r++ = (unsigned char)cnt;
                        memcpy( (void *)r, (void *)q, cnt );
                        r += cnt;
                        q += cnt;
                        new_bit_size += 1 + cnt;
                        cnt = 0;
                    }
                    /* else look for repeats */
                    else
                    {
                        while ( *t == *(t+1) &&
                                cnt < 255 &&
                                t < p + BITScanWidth )
                        {
                            t++;
                            cnt++;
                        }
                        cnt++;
                        if ( cnt > 2 )
                        {
                            *r++ = 0;
                            *r++ = (unsigned char)cnt;
                            *r++ = *(t-1);
                            new_bit_size += 3;
                            q = t;
                            cnt = 0;
                        }
                        /* use absolute mode if we have reached the EOL &&
                        // cnt <= 2 */
                        else if ( t >= p + BITScanWidth )
                        {
                            *r++ = (unsigned char)cnt;
                            memcpy( (void *)r, (void *)q, cnt );
                            r += cnt;
                            q += cnt;
                            new_bit_size += 1 + cnt;
                            cnt = 0;
                        }
                    }
                }

                /* put an EOL marker here */
                *r++ = 0;
                *r++ = 0;
                new_bit_size += 2;
                cnt = 0;
            }

            p += BITScanWidth;
        }

        /* put the EOB marker here */
        if ( new_bit_size < bit_size - 2 )
        {
            *r++ = 0;
            *r = 1;
            new_bit_size += 2;
        }
        else
            new_bit_size = bit_size + 1;

        /* if the compressed image will take less space then use it */
        if ( new_bit_size < bit_size )
        {
            bmih.biCompression = BI_RLE8;
            bmih.biSizeImage = bit_size = new_bit_size;
        }
    }

#endif

