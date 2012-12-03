#ifndef _BMG_IMAGE_H_
#define _BMG_IMAGE_H_
/*
//  header file for the BMGImage functions
//
//  Copyright 2000, 2001 M. Scott Heiman
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

#include "../osal_preproc.h"

#if !defined(WIN32)
    typedef struct tagRGBQUAD
    {
       unsigned char rgbBlue;
       unsigned char rgbGreen;
       unsigned char rgbRed;
       unsigned char rgbReserved;
    } RGBQUAD;
#endif

enum BMG_Error 
{ 
    BMG_OK = 0,
    errLib = 1,
    errInvalidPixelFormat = 2,
    errMemoryAllocation = 3,
    errInvalidSize = 4,
    errInvalidBitmapHandle = 5,
    errWindowsAPI = 6,
    errFileOpen = 7,
    errUnsupportedFileFormat = 8,
    errInvalidBMGImage = 9,
    errInvalidFileExtension = 10,
    errFileRead = 11,
    errFileWrite = 12,
    errInvalidGeoTIFFPointer = 13,
    errUndefinedBGImage = 14,
    errBGImageTooSmall = 15,
    errCorruptFile = 16
};

typedef enum BMG_Error BMGError;

#pragma pack(push,1)
struct BMGImageStruct
{
    unsigned int width;
    unsigned int height;
    unsigned char bits_per_pixel;
    unsigned char *bits;
    unsigned short palette_size;
    unsigned char bytes_per_palette_entry;
    unsigned char *palette;
    unsigned int scan_width;
    int opt_for_bmp; /*= 1 if memory has been sized for HBITMAP, 0 otherwise*/
    short transparency_index;
};
#pragma pack(pop)

#if defined(__cplusplus)
extern "C" {
#endif

/* initializes a BMGImage to default values */
extern
void  InitBMGImage(struct BMGImageStruct *img );

/* frees memory allocated to a BMGImage */
extern
void  FreeBMGImage( struct BMGImageStruct *img );

/* allocates memory (bits & palette) for a BMGImage.
   returns 1 if successfull, 0 otherwise.
   width, height, bits_per_pixel, palette_size, & opt_for_bmp must have valid
   values before this function is called.
   Assumes that all images with bits_per_pixel <= 8 requires a palette.
   will set bits_per_palette_entry, scan_width, bits, & palette */
extern
BMGError  AllocateBMGImage( struct BMGImageStruct *img );

/* compresses 8 BPP paletted images to 1 BPP or 4 BPP paletted images if
   possible */
extern
BMGError  CompressBMGImage( struct BMGImageStruct *img );


/* a utility function for freeing memory created in BMGLib */
extern
void  FreeBMGMemory( unsigned char *mem );

/* converts a color image to a gray scale image */
extern
BMGError  ConvertToGrayScale( struct BMGImageStruct *img );

/* converts a color image to a pseudo-gray scale image */
extern
BMGError  ConvertToPseudoGrayScale( struct BMGImageStruct *img );

/* stores the contents of a bitmap into a BMGImageStruct */
extern
BMGError  GetDataFromBitmap( HBITMAP hBitmap,
                                  struct BMGImageStruct *img,
                                  int remove_alpha );

/* creates an HBITMAP from a BMGImageStruct */
extern
HBITMAP  CreateBitmapFromData( struct BMGImageStruct img,
                                         int alpha_blend );

/* sets the background color for alpha blending
  color points to an array of 4 unsigned chars
  color[0] = blue, color[1] = green, color[2] = red, color[3] = unused */
extern
void  SetBMGBackgroundColor( unsigned char *color );

/* defines the background bitmap that is used for alpha blending & transparent
   pixels */
extern
BMGError  SetBMGBackgroundBitmap( HBITMAP hBitmap );

/* defines the background image that is used for alpha blending & transparent
   pixels */
extern
BMGError  SetBMGBackgroundImage( struct BMGImageStruct img );

/* Converts paletted images and 16-BPP images to 24-BPP images */
extern
BMGError  ConvertPaletteToRGB( struct BMGImageStruct img_in,
                                         struct BMGImageStruct *img_out );

/* copies the contents of the input image into the output image */
extern
BMGError  CopyBMGImage( struct BMGImageStruct img_in,
                                  struct BMGImageStruct *img_out );

/* returns the last error state */
extern
BMGError  GetLastBMGError();

/* gets the error message */
extern
void  GetLastBMGErrorMessage( const char **msg );

#if defined(__cplusplus)
 }
#endif

#endif

