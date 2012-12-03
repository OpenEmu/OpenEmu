/*
//  source code for the ImageLib PNG functions
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

#include "BMGUtils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _BMG_LIBPNG_STANDALONE
#include "BMGLibPNG.h"
#else
#include "pngrw.h"
#endif
#include <png.h>

#ifndef png_jmpbuf
#  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

/****************************************************************************/
/*
* this function will expand 1 byte of a 4-color bit values to 2 bytes of
* a 16 color bit value.
*/
static void Expand2to4( char *b4, char b2, int NumBytes )
{
    struct TwoBit
    {
        unsigned char b1 : 2;
        unsigned char b2 : 2;
        unsigned char b3 : 2;
        unsigned char b4 : 2;
    } bit2;

    struct FourBit
    {
        unsigned char b1 : 4;
        unsigned char b2 : 4;
        unsigned char b3 : 4;
        unsigned char b4 : 4;
    } bit4;

    memcpy( (void *)&bit2, (void *)&b2, 1 );
    bit4.b3 = bit2.b1;
    bit4.b4 = bit2.b2;
    bit4.b1 = bit2.b3;
    bit4.b2 = bit2.b4;
    memcpy( (void *)b4, (void *)&bit4, NumBytes );
}

/* this stuff is necessary because the normal png_init_io() method crashes in Win32 */
static void user_read_data(png_structp png_read, png_bytep data, png_size_t length)
{
    FILE *fPtr = (FILE *) png_get_io_ptr(png_read);
    if (fread(data, 1, length, fPtr) != length)
        fprintf(stderr, "Failed to read %i bytes from PNG file.\n", (int) length);
}

static void user_write_data(png_structp png_write, png_bytep data, png_size_t length)
{
    FILE *fPtr = (FILE *) png_get_io_ptr(png_write);
    if (fwrite(data, 1, length, fPtr) != length)
        fprintf(stderr, "Failed to write %i bytes to PNG file.\n", (int) length);
}

static void user_flush_data(png_structp png_read)
{
    FILE *fPtr = (FILE *) png_get_io_ptr(png_read);
    fflush(fPtr);
}

/*
ReadPNG - Reads the contents of a PNG file and stores the contents into
    BMGImageStruct

Inputs:
    filename    - the name of the file to be opened

Outputs:
    img         - the BMGImageStruct containing the image data

Returns:
    BMGError - if the file could not be read or a resource error occurred
    BMG_OK   - if the file was read and the data was stored in img

Limitations:
    None.

Comments:
    2-bit images are converted to 4-bit images.
    16-bit images are converted to 8-bit images.
    gray scale images with alpha components are converted to 32-bit images
*/
BMGError ReadPNG( const char *filename,
        struct BMGImageStruct * volatile img )
{
    jmp_buf             err_jmp;
    int                 error;

    FILE * volatile     file = NULL;
    int                 BitDepth;
    int                 BitsPerPixel;
    int                 FixedBitDepth;
    int                 ColorType;
    int                 ImageChannels;
    int                 InterlaceType;
    unsigned char       signature[8];
    png_structp volatile png_ptr = NULL;
    png_infop   volatile info_ptr = NULL;
    png_infop   volatile end_info = NULL;
    png_colorp          PNGPalette = NULL;
    png_color_16       *ImageBackground = NULL;
    png_bytep           trns = NULL;
    int                 NumTrans = 0;
    int                 i, j, k;
    png_color_16p       TransColors = NULL;
    png_uint_32         Width, Height;

    unsigned char      *bits, *p, *q;
    unsigned char** volatile rows = NULL;
    int                 NumColors = 0;
    unsigned char       BgdRed = 0;
    unsigned char       BgdGreen = 0;
    unsigned char       BgdBlue = 0;
    int                 PaletteTo32 = 0;

    BMGError tmp;
    unsigned int DIBLineWidth;

    /* error handler */
    error = setjmp( err_jmp );
    if (error != 0)
    {
        if (end_info != NULL)
            png_destroy_read_struct((png_structp *) &png_ptr, (png_infop *) &info_ptr, (png_infop *) &end_info);
        else if (info_ptr != NULL)
            png_destroy_read_struct((png_structp *) &png_ptr, (png_infop *) &info_ptr, NULL);
        else if (png_ptr != NULL)
            png_destroy_read_struct((png_structp *) &png_ptr, NULL, NULL);
        if (rows)
        {
            if (rows[0])
                free(rows[0]);
            free(rows);
        }
        if (img)
            FreeBMGImage(img);
        if (file)
            fclose(file);
        SetLastBMGError((BMGError) error);
        return (BMGError) error;
    }

    if ( img == NULL )
        longjmp ( err_jmp, (int)errInvalidBMGImage );

    file = fopen( filename, "rb" );
    if ( !file || fread( signature, 1, 8, file ) != 8)
        longjmp ( err_jmp, (int)errFileOpen );

    /* check the signature */
    if ( png_sig_cmp( signature, 0, 8 ) != 0 )
        longjmp( err_jmp, (int)errUnsupportedFileFormat );

    /* create a pointer to the png read structure */
    png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
    if ( !png_ptr )
        longjmp( err_jmp, (int)errMemoryAllocation );

    /* create a pointer to the png info structure */
    info_ptr = png_create_info_struct( png_ptr );
    if ( !info_ptr )
        longjmp( err_jmp, (int)errMemoryAllocation );

    /* create a pointer to the png end-info structure */
    end_info = png_create_info_struct(png_ptr);
    if (!end_info)
        longjmp( err_jmp, (int)errMemoryAllocation );

    /* bamboozle the PNG longjmp buffer */
    /*generic PNG error handler*/
    /* error will always == 1 which == errLib */
//    error = png_setjmp(png_ptr);
    error = setjmp( png_jmpbuf( png_ptr ) );
    if ( error > 0 )
        longjmp( err_jmp, error );

    /* set function pointers in the PNG library, for read callbacks */
    png_set_read_fn(png_ptr, (png_voidp) file, user_read_data);

    /*let the read functions know that we have already read the 1st 8 bytes */
    png_set_sig_bytes( png_ptr, 8 );

    /* read all PNG data up to the image data */
    png_read_info( png_ptr, info_ptr );

    /* extract the data we need to form the HBITMAP from the PNG header */
    png_get_IHDR( png_ptr, info_ptr, &Width, &Height, &BitDepth, &ColorType,
        &InterlaceType, NULL, NULL);

    img->width = (unsigned int) Width;
    img->height = (unsigned int) Height;
    ImageChannels = png_get_channels( png_ptr, info_ptr );
    FixedBitDepth = BitDepth;

    /* convert 2 pixel images to 4 pixel images */
    if ( BitDepth == 2 )
        FixedBitDepth = 4;

/* There is logic in the following code that will
    convert 16-bit grayscale images to 8-bit paletted images and
    convert 16-bit color images to 24-bit RGB images */
    if ( BitDepth == 16 )
        FixedBitDepth = 8;
    BitsPerPixel = FixedBitDepth *
            ( ColorType & PNG_COLOR_MASK_ALPHA && !(ColorType & PNG_COLOR_MASK_COLOR)?
            ImageChannels - 1 : ImageChannels );

    img->bits_per_pixel = (unsigned char)BitsPerPixel;

    /* convert 16-bit images to 8-bit images */
    if (BitDepth == 16)
        png_set_strip_16(png_ptr);

    /* set the background color if one is found */
    if ( png_get_valid(png_ptr, info_ptr, PNG_INFO_bKGD) )
        png_get_bKGD(png_ptr, info_ptr, &ImageBackground);

    /* get the transparent color if one is there */
    if ( png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) )
        png_get_tRNS( png_ptr, info_ptr, &trns, &NumTrans, &TransColors );

    /* Get the background color if we have transparent pixels */
    if ( NumTrans > 0 )
    {
        if ( NumTrans == 1 )
        {
    // we will shove the background color into the palette array
    // or pixel location
            if ( ImageBackground )
            {
                BgdRed   = (unsigned char)ImageBackground->red;
                BgdGreen = (unsigned char)ImageBackground->green;
                BgdBlue  = (unsigned char)ImageBackground->blue;
            }
    // if the alpha component == 0 then set the transparenct index
    // and let the user deal with it
            else if ( trns[0] == 0 )
                img->transparency_index = 0;
    // otherwise we must perform blending so we will need to create
    // a 32-bit ARGB image.
            else
                PaletteTo32 = 1;
        }
    // if we have more than 1 transparent color then create a 32-bit ARGB
    // image
        else
            PaletteTo32 = 1;
    }

    /* convert all palette based images to 8-bit arrays */
    if ( BitDepth < 8 && PaletteTo32 == 1 )
    {
        BitDepth = 8;
        png_set_packing(png_ptr);
    }

    /* calculate the the scan line width */
/* 8 & 16-bit images with an alpha component are converted to 32-bit
    true color images in order to retain the alpha component in the bitmap */
    if ( ColorType & PNG_COLOR_MASK_ALPHA && BitsPerPixel == 8 )
    {
        img->bits_per_pixel = 32U;
        DIBLineWidth = 4U * (unsigned int) Width;
    }
/* paletted images with more than 1 transparency index or a non-zero alpha
    component are converted to 32-bit ABGR images */
    else if ( ColorType & PNG_COLOR_MASK_PALETTE && PaletteTo32 == 1 )
    {
        img->bits_per_pixel = 32U;
        DIBLineWidth = 4U * (unsigned int) Width;
    }
    else
    {
        DIBLineWidth = img->scan_width = ( BitsPerPixel * (unsigned int) Width + 7 ) / 8;
        if ( img->opt_for_bmp > 0 && img->scan_width % 4 )
            img->scan_width += 4 - img->scan_width % 4;
    }

/* Determine palette parameters.  We will not create a palette for
    grayscale images that have an alpha component.  Those images will be
    expanded to 32-bit true color images in order to retain the alpha
    component in the bitmap. */
    if ( BitsPerPixel <= 8 && !(ColorType & PNG_COLOR_MASK_ALPHA))
    {
        if ( ColorType & PNG_COLOR_MASK_PALETTE )
        {
            png_get_PLTE( png_ptr, info_ptr, &PNGPalette, &NumColors );
            if ( NumTrans == 1 && PaletteTo32 == 0 && ImageBackground != NULL )
            {
                PNGPalette[0].red   =
                        AlphaComp(PNGPalette[0].red,   trns[0], BgdRed);
                PNGPalette[0].green =
                        AlphaComp(PNGPalette[0].green, trns[0], BgdGreen);
                PNGPalette[0].blue  =
                        AlphaComp(PNGPalette[0].blue,  trns[0], BgdBlue);
            }
        }
        else  /* gray scale */
            NumColors = 1 << (BitDepth == 2 ? 2 : FixedBitDepth);
    }

    /* set up palette parameters */
    if ( PaletteTo32 != 1 && BitsPerPixel <= 8 &&
        !(ColorType & PNG_COLOR_MASK_ALPHA) )
    {
        img->palette_size = (unsigned short)NumColors;
        img->bytes_per_palette_entry = img->opt_for_bmp > 0 ? 4U : 3U;
    }

    tmp = AllocateBMGImage( img );
    if ( tmp != BMG_OK )
        longjmp( err_jmp, (int)tmp );


    if ( img->palette != NULL )
    {
        /* color palette */
        if ( ColorType & PNG_COLOR_MASK_PALETTE )
        {
            bits = img->palette;
            for ( i = 0; i < NumColors; 
                        i++, bits += img->bytes_per_palette_entry )
            {
                bits[2] = PNGPalette[i].red;
                bits[1] = PNGPalette[i].green;
                bits[0] = PNGPalette[i].blue;
            }
        }
        else /* Gray scale palette */
        {
            j = 255 / (NumColors - 1);
            bits = img->palette;
            for ( i = 0; i < NumColors; 
                        i++, bits += img->bytes_per_palette_entry )
            {
                memset( (void *)bits, i*j, 3 );
            }

            if ( NumTrans == 1 && ImageBackground != NULL )
            {
                img->palette[2] = BgdRed;
                img->palette[1] = BgdGreen;
                img->palette[0] = BgdBlue;
            }
        }
    }

    png_read_update_info( png_ptr, info_ptr );

    /* create buffer to read data to */
    rows = (unsigned char **)malloc(Height*sizeof(unsigned char *));
    if ( !rows )
        longjmp( err_jmp, (int)errMemoryAllocation );

    k = png_get_rowbytes( png_ptr, info_ptr );
    rows[0] = (unsigned char *)malloc( Height*k*sizeof(char));
    if ( !rows[0] )
        longjmp( err_jmp, (int)errMemoryAllocation );

    for ( i = 1; i < (int)Height; i++ )
        rows[i] = rows[i-1] + k;

    /* read the entire image into rows */
    png_read_image( png_ptr, rows );

/* extract bits
    The following logic is convoluted compared to the simple examples
    provided with the source.  I wrote the code this way to ensure that
    hBitmap cantained the minimal amount of information needed to store
    the image as well as capturing all alpha components */
    bits = img->bits + (Height - 1) * img->scan_width;
    for ( i = 0; i < (int)Height; i++ )
    {
        p = rows[i];
        switch ( BitDepth )
        {
            case 1:
            case 4:
                memcpy((void *)bits, (void *)p, DIBLineWidth);
                break;
            case 2:
                for ( j = 0; j < (int)(Width/2); j += 2 )
                    Expand2to4((char*)&bits[j], p[j/2], 2);
                if ( Width % 2 )
                    Expand2to4((char*)&bits[Width/2+1], p[Width/4+1], 1);
                break;
            case 8:
                /* 16-bit images were converted to 8-bit */
            case 16:
                /* this section for data with alpha components */
                if ( ColorType & PNG_COLOR_MASK_ALPHA )
                {
        /* expand this format to a 32-bit true color image so that
                    the alpha term is retained in the bitmap */
                    if ( BitsPerPixel == 8 )
                    {
                        for ( q = bits; q < bits + img->scan_width;
                                        q += 4, p+=2)
                        {
                            memset( (void *)q, *p, 3 );
                            q[3] = p[1];
                        }
                    }
                    else  /* BitsPerPixel == 32 */
                    {
                        for ( j = 0; j < (int)(4*Width); j += 4 )
                        {
                            bits[j+3] = p[j+3];
                            bits[j+2] = p[j  ];
                            bits[j+1] = p[j+1];
                            bits[j  ] = p[j+2];
                        }
                    }
                }
        /* this section is for paletted images that contain multiple
                transparency values or non-zero alpha transparencies */
                else if ( PaletteTo32 == 1 )
                {
                    for ( q = bits; q < bits + img->scan_width; q += 4, p++ )
                    {
                        png_colorp pal = PNGPalette + *p;
                        q[0] = pal->blue;
                        q[1] = pal->green;
                        q[2] = pal->red;
                        if ( *p >= NumTrans )
                            q[3] = 0xFF;
                        else
                            q[3] = trns[*p];
                    }
                }
                /* no alpha component */
                else
                {
                    if ( BitsPerPixel == 8 )
                        memcpy( (void *)bits, (void *)p, DIBLineWidth );
                    else /* BitsPerPixel == 24 */
                    {
                        for ( j = 0; j < (int)(3*Width); j += 3 )
                        {
                            if ( TransColors != NULL &&
                                        ImageBackground != NULL )
                            {
                                if ( TransColors->red == p[j] &&
                                        TransColors->green == p[j+1] &&
                                        TransColors->blue == p[j+2] )
                                {
                                    bits[j+2] = BgdRed;
                                    bits[j+1] = BgdGreen;
                                    bits[j  ] = BgdBlue;
                                }
                                else
                                {
                                    bits[j+2] = p[j  ];
                                    bits[j+1] = p[j+1];
                                    bits[j  ] = p[j+2];
                                }
                            }
                            else
                            {
                                bits[j+2] = p[j  ];
                                bits[j+1] = p[j+1];
                                bits[j  ] = p[j+2];
                            }
                        }
                    }
                }
                break;
        }

        bits -= img->scan_width;
    }

    free( rows[0] );
    free( rows );
    png_read_end( png_ptr, info_ptr );
    png_destroy_read_struct((png_structp *) &png_ptr, (png_infop *) &info_ptr, (png_infop *) &end_info);
    fclose( file );

    return BMG_OK;
}

/*
WritePNG - writes the contents of a BMGImageStruct to a PNG file.

Inputs:
    filename    - the name of the file to be opened
    img         - the BMGImageStruct containing the image data

Returns:
    0 - if the file could not be written or a resource error occurred
    1 - if the file was written

Comments:
        16-BPP BMG Images are converted to 24-BPP images

Limitations:
    Color Type is limited to PNG_COLOR_TYPE_GRAY, PNG_COLOR_TYPE_RGB_ALPHA,
    PNG_COLOR_TYPE_RGB, & PNG_COLOR_TYPE_PALETTE;
*/
BMGError WritePNG( const char *filename, struct BMGImageStruct img )
{
    jmp_buf     err_jmp;
    int     error = 0;
    int     BitDepth = 0;
    int     ColorType = 0;
    png_structp png_ptr = NULL;
    png_infop   info_ptr = NULL;
    png_colorp  PNGPalette = NULL;
    int     GrayScale;

    unsigned char   *bits, *p, *q;
    unsigned char   **rows = NULL;
    int     NumColors = 0;

    int     DIBScanWidth;
    int     HasPalette;
    FILE        *outfile = NULL;
    int     i;
    BMGError    tmp;

    /* error handler */
    error = setjmp( err_jmp );
    fprintf(stderr,"Writing PNG file %s.\n", filename);
    if ( error != 0 )
    {
        if ( png_ptr != NULL )
            png_destroy_write_struct( &png_ptr, NULL );
        if ( rows )
        {
            if ( rows[0] )
            {
                free( rows[0] );
            }
            free( rows );
        }
        if ( PNGPalette )
            free( PNGPalette );
        if (outfile)
        {
            fclose( outfile );
        }
        SetLastBMGError( (BMGError)error );
        return (BMGError)error;
    }

    SetLastBMGError( BMG_OK );
    /* open the file */
    if ((outfile = fopen(filename, "wb")) == NULL)
    {
        fprintf(stderr, "Error opening %s for reading.\n", filename);
        longjmp( err_jmp, (int)errFileOpen );
    }

    /* 16 BPP DIBS do not have palettes.  libPNG expects 16 BPP images to have
    a palette.  To correct this situation we must convert 16 BPP images to
    24 BPP images before saving the data to the file */
    if ( img.bits_per_pixel == 16 )
    {
        tmp = Convert16to24( &img ); 
        if (  tmp != BMG_OK )
            longjmp( err_jmp, (int)tmp );
    }

    HasPalette = img.bits_per_pixel <= 8;
    if ( HasPalette )
    {
        NumColors = img.palette_size;
        /* if this is a grayscale image then set the flag and delete the palette*/
        i = 0;
        bits = img.palette;
        while ( i < NumColors && bits[0] == bits[1] && bits[0] == bits[2] )
        {
            i++;
            bits += img.bytes_per_palette_entry;
        }
        GrayScale = i == NumColors;
    }
    else
        GrayScale = 0;

    /* dimensions */
    DIBScanWidth = ( img.width * img.bits_per_pixel + 7 ) / 8;

    /* create the png pointer */
    png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if ( !png_ptr )
        longjmp( err_jmp, (int)errMemoryAllocation );

    /* create the info pointer */
    info_ptr = png_create_info_struct( png_ptr );
    if ( !info_ptr )
        longjmp( err_jmp, (int)errMemoryAllocation );

    /* bamboozle the png error handler */
    /* error will always == 1 which equals errLib */
//    error = png_setjmp(png_ptr);
    error = setjmp( png_jmpbuf( png_ptr ) );
    if ( error > 0 )
        longjmp( err_jmp, error );

    /* set function pointers in the PNG library, for write callbacks */
    png_set_write_fn(png_ptr, (png_voidp) outfile, user_write_data, user_flush_data);

    /* prepare variables needed to create PNG header */
    BitDepth = img.bits_per_pixel < 8 ? img.bits_per_pixel : 8;

    /* determine color type */
    if ( GrayScale )
        ColorType = PNG_COLOR_TYPE_GRAY;
    else if ( img.bits_per_pixel == 32 )
        ColorType = PNG_COLOR_TYPE_RGB_ALPHA;
    else if ( img.bits_per_pixel == 24 )
        ColorType = PNG_COLOR_TYPE_RGB;
    else
        ColorType = PNG_COLOR_TYPE_PALETTE;

    /* create the PNG header */
    png_set_IHDR( png_ptr, info_ptr, img.width, img.height, BitDepth, ColorType, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );

    /* store the palette information if there is any */
    if ( img.palette != NULL && !GrayScale )
    {
        PNGPalette = (png_colorp)png_malloc( png_ptr,
            NumColors*sizeof(png_color));
        if ( PNGPalette )
        {
            bits = img.palette;
            for ( i = 0; i < NumColors; i++, bits += img.bytes_per_palette_entry )
            {
                PNGPalette[i].red   = bits[2];
                PNGPalette[i].green = bits[1];
                PNGPalette[i].blue  = bits[0];
            }
            png_set_PLTE( png_ptr, info_ptr, PNGPalette, NumColors );
        }
        else
            longjmp( err_jmp, (int)errMemoryAllocation );
    }

    /* write the file header information */
    png_write_info( png_ptr, info_ptr );

    /* create array to store data in */
    rows = (unsigned char **)malloc(sizeof(unsigned char*));
    if ( !rows )
        longjmp( err_jmp, (int)errMemoryAllocation );
    rows[0] = (unsigned char *)malloc( DIBScanWidth * sizeof(unsigned char));
    if ( !rows[0] )
        longjmp( err_jmp, (int)errMemoryAllocation );

/* point to the bottom row of the DIB data.  DIBs are stored bottom-to-top,
    PNGs are stored top-to-bottom. */
    bits = img.bits + (img.height - 1) * img.scan_width;

    /* store bits */
    for ( i = 0; i < (int)img.height; i++ )
    {
        switch ( img.bits_per_pixel )
        {
            case 1:
            case 4:
            case 8:
                memcpy( (void *)rows[0], (void *)bits, DIBScanWidth );
                break;
            case 24:
                q = bits;
                for ( p = rows[0]; p < rows[0] + DIBScanWidth; p += 3, q += 3 )
                {
                    p[0] = q[2];
                    p[1] = q[1];
                    p[2] = q[0];
                }
                break;
            case 32:
                q = bits;
                for ( p = rows[0]; p < rows[0] + DIBScanWidth; p += 4, q += 4 )
                {
                    p[3] = q[3];
                    p[0] = q[2];
                    p[1] = q[1];
                    p[2] = q[0];
                }
                break;
        }

        png_write_rows( png_ptr, rows, 1 );
        bits -= img.scan_width;
    }

    /* finish writing the rest of the file */
    png_write_end( png_ptr, info_ptr );

    /* clean up and exit */
    if ( PNGPalette )
        free( PNGPalette );
    free( rows[0] );
    free( rows );
    png_destroy_write_struct( &png_ptr, NULL );
    fclose( outfile );

    return BMG_OK;
}

#ifdef _BMG_LIBPNG_STANDALONE
#pragma message ("Creating BMGLibPNG functions")

#ifdef _WIN32
/* saves the contents of an HBITMAP to a file.  returns 1 if successfull,
                // 0 otherwise */
                BMGError SaveBitmapToPNGFile( HBITMAP hBitmap,      /* bitmap to be saved */
                const char *filename) /* name of output file */
{
    struct BMGImageStruct img;
    char msg[256], ext[4], *period;
    BMGError out = BMG_OK;

    InitBMGImage( &img );

    /* determine the file type by using the extension */
    strcpy( msg, filename );
    period = strrchr( msg, '.' );
    if ( period != NULL )
    {
        period++;
        strcpy( ext, period );
        ext[0] = toupper( ext[0] );
        ext[1] = toupper( ext[1] );
        ext[2] = toupper( ext[2] );
        ext[3] = 0;
    }
    else
    {
        strcat( msg, ".PNG" );
        strcpy( ext, "PNG" );
    }

    if ( strcmp( ext, "PNG" ) == 0 )
    {
/* extract data from the bitmap.  We assume that 32 bit images have been
// blended with the background (unless this is a DDB - see GetDataFromBitmap
        // for more details) */
        out = GetDataFromBitmap( hBitmap, &img, 1 ); 
        if (  out == BMG_OK )
        {
            out = WritePNG( msg, img );
        }
        FreeBMGImage( &img );
    }
    else
    {
        out = errInvalidFileExtension;
    }

    SetLastBMGError( out );
    return out;
}
#endif // _WIN32

/* Creates an HBITMAP to an image file.  returns an HBITMAP if successfull,
// NULL otherwise */
HBITMAP CreateBitmapFromPNGFile( const char *filename,
                int blend )
{
    char ext[4], msg[256];
    char *period;
    BMGError out = BMG_OK;
    struct BMGImageStruct img;
    HBITMAP hBitmap = NULL;

    InitBMGImage( &img );
    img.opt_for_bmp = 1;

    strcpy( msg, filename );
    period = strrchr( msg, '.' );
    if ( period != NULL )
    {
        period++;
        strncpy( ext, period, 3 );
        ext[0] = toupper( ext[0] );
        ext[1] = toupper( ext[1] );
        ext[2] = toupper( ext[2] );
        ext[3] = 0;
    }
    else
    {
        strcat( msg, ".PNG" );
        strcpy( ext, "PNG" );
    }

    if ( strcmp( ext, "PNG" ) == 0 )
    {
        out = ReadPNG( msg, &img ); 
        if (  out == BMG_OK )
        {
            hBitmap = CreateBitmapFromData( img, blend );
        }
        FreeBMGImage( &img );
    }
    else
    {
        out = errInvalidFileExtension;
    }

    SetLastBMGError( out );
    return hBitmap;
}

#endif

