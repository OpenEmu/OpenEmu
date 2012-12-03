#ifndef _BMG_UTILS_H_
#define _BMG_UTILS_H_
/*
    some handy utilities used in several units

    Copyright 2001
    M. Scott Heiman
    All Rights Reserved
*/

#include "BMGImage.h"
#include "../osal_preproc.h"

/* the following 3 functions are used to access the background color
// and the background image */
void InitBackground(void);

unsigned char *GetBackgroundColor(void);

struct BMGImageStruct *GetBackgroundImage(void);

/* creates a 24 bpp image from a 16 bpp image */
BMGError Convert16to24( struct BMGImageStruct *img );

/* converts an array of 1-bit scanlines to 8-bit scanlines */
void Convert1to8( struct BMGImageStruct img,
                  unsigned char *out );

/* converts an array of 4-bit scanlines to 8-bit scanlines */
void Convert4to8( struct BMGImageStruct img,
                  unsigned char *out );

unsigned char AlphaComp( unsigned char fg,
                         unsigned char alpha,
                         unsigned char bg );

unsigned char InverseAlphaComp( unsigned char fg,
                                unsigned char alpha,
                                unsigned char bg );

BITMAPINFO InternalCreateBMI( unsigned int dwWidth,  /* width */
                              unsigned int dwHeight, /* height */
                              unsigned short wBitCount, /* bit count */
                              int compression );  /* compression type */

void SetLastBMGError( BMGError err );

/* the following 4 functions are for dealing with file formats
   that store data in big endian format */
short SwapShort( short in );
unsigned short SwapUShort( unsigned short in );
int SwapLong( int in );
unsigned int SwapULong( unsigned int in );

#endif

