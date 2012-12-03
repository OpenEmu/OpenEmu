#ifndef _TIFF_RW_H_
#define _TIFF_RW_H_
/*
//  header file defining BMGLib libTIFF structures and functions 
//
//  Copyright 2000, 2001 Scott Heiman
//  libTIFF is Copyright Sam Leffler and SGI
//  zLib Copyright (C) 1995-1998 Jean-loup Gailly.
//
// Permission to use, copy, modify, distribute, and sell this software and 
// its documentation for any purpose is hereby granted without fee, provided
// that (i) the above copyright notices and this permission notice appear in
// all copies of the software and related documentation, and (ii) the names of
// Sam Leffler and Silicon Graphics may not be used in any advertising or
// publicity relating to the software without the specific, prior written
// permission of Sam Leffler and Silicon Graphics.
//
// THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
// WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
//
// IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
// ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
// OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
// LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
// OF THIS SOFTWARE.
*/

#include "BMGImage.h"

/* enumeration types that support libTIFF */
enum TiffCompressionEnum { NONE, CCITTRLE, CCITTFAX3, CCITTFAX4, LZW, JPEG6,
                       JPEG_DCT, NeXT, CCITTRLEW, MACINTOSH, THUNDERSCAN,
                       PIXARFILM, PIXARLOG, ZIP, KODAK, JBIG };
enum TiffPhotometricEnum { MINISWHITE, MINISBLACK, RGB, PALETTE, MASK,
                       SEPARATED, YCBCR, CIELAB, CIE_LOGL, CIE_LOGLUV };
enum TiffOrientationEnum { TOPLEFT, BOTTOMLEFT };

typedef enum TiffCompressionEnum TiffCompression;
typedef enum TiffPhotometricEnum TiffPhotometric;
typedef enum TiffOrientationEnum TiffOrientation;

#pragma pack(push,1)
struct TIFFInfoStruct
{
    TiffCompression compression;
    TiffPhotometric photometric;
    TiffOrientation orientation;
    unsigned short predictor;
};
#pragma pack(pop)

#if defined(__cplusplus)
extern "C" {
#endif

extern
BMGError  ReadTIFF( const char *filename,
                         struct BMGImageStruct *img,
                         struct TIFFInfoStruct *info );

extern
BMGError  WriteTIFF( const char *filename,
                          struct BMGImageStruct img,
                          struct TIFFInfoStruct *info );


#if defined(__cplusplus)
 }
#endif


#endif

