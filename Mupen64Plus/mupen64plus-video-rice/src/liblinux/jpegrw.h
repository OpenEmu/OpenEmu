#ifndef _JPEG_RW_H_
#define _JPEG_RW_H_
/*
//  header file for the BMGLib JPEG functions
//
//  Copyright 2000, 2001 M. Scott Heiman
//  All Rights Reserved
//  libJPEG is Copyright (C) 1991-1998, Thomas G. Lane and is part of the
//      Independent JPEG Group's software.
//
// We are releasing this software for both noncommercial and commercial use.
// Companies are welcome to use it as the basis for JPEG-related products.
// We do not ask a royalty, although we do ask for an acknowledgement in
// product literature (see the README file in the distribution for details).
// We hope to make this software industrial-quality --- although, as with
// anything that's free, we offer no warranty and accept no liability.
*/
#include "BMGImage.h"

#if defined(__cplusplus)
extern "C" {
#endif

extern BMGError  ReadJPEG( const char *filename,
                         struct BMGImageStruct *img );

extern BMGError  WriteJPEG( const char *filename,
                          struct BMGImageStruct img,
                          int quality );

#if defined(__cplusplus)
 }
#endif

#endif

