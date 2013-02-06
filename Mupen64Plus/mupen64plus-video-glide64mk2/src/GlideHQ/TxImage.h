/*
 * Texture Filtering
 * Version:  1.0
 *
 * Copyright (C) 2007  Hiroshi Morii   All Rights Reserved.
 * Email koolsmoky(at)users.sourceforge.net
 * Web   http://www.3dfxzone.it/koolsmoky
 *
 * this is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * this is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Make; see the file COPYING.  If not, write to
 * the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __TXIMAGE_H__
#define __TXIMAGE_H__

#include <stdio.h>
#include <png.h>
#include "TxInternal.h"

#ifndef WIN32
typedef struct tagBITMAPFILEHEADER {
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  uint32_t biSize;
  uint32_t biWidth;
  uint32_t biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  uint32_t biXPelsPerMeter;
  uint32_t biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
} BITMAPINFOHEADER;
#else
typedef struct tagBITMAPFILEHEADER BITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER BITMAPINFOHEADER;
#endif

#define DDSD_CAPS	0x00000001
#define DDSD_HEIGHT	0x00000002
#define DDSD_WIDTH	0x00000004
#define DDSD_PITCH	0x00000008
#define DDSD_PIXELFORMAT	0x00001000
#define DDSD_MIPMAPCOUNT	0x00020000
#define DDSD_LINEARSIZE	0x00080000
#define DDSD_DEPTH	0x00800000

#define DDPF_ALPHAPIXELS	0x00000001
#define DDPF_FOURCC	0x00000004
#define DDPF_RGB	0x00000040

#define DDSCAPS_COMPLEX	0x00000008
#define DDSCAPS_TEXTURE	0x00001000
#define DDSCAPS_MIPMAP	0x00400000

typedef struct tagDDSPIXELFORMAT {
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwFourCC;
  uint32_t dwRGBBitCount;
  uint32_t dwRBitMask;
  uint32_t dwGBitMask;
  uint32_t dwBBitMask;
  uint32_t dwRGBAlphaBitMask;
} DDSPIXELFORMAT;

typedef struct tagDDSFILEHEADER {
  uint32_t dwMagic;
  uint32_t dwSize;
  uint32_t dwFlags;
  uint32_t dwHeight;
  uint32_t dwWidth;
  uint32_t dwLinearSize;
  uint32_t dwDepth;
  uint32_t dwMipMapCount;
  uint32_t dwReserved1[11];
  DDSPIXELFORMAT ddpf;
  uint32_t dwCaps1;
  uint32_t dwCaps2;
} DDSFILEHEADER;

class TxImage
{
private:
  boolean getPNGInfo(FILE *fp, png_structp *png_ptr, png_infop *info_ptr);
  boolean getBMPInfo(FILE *fp, BITMAPFILEHEADER *bmp_fhdr, BITMAPINFOHEADER *bmp_ihdr);
  boolean getDDSInfo(FILE *fp, DDSFILEHEADER *dds_fhdr);
public:
  TxImage() {}
  ~TxImage() {}
  uint8* readPNG(FILE* fp, int* width, int* height, uint16* format);
  boolean writePNG(uint8* src, FILE* fp, int width, int height, int rowStride, uint16 format, uint8 *palette);
  uint8* readBMP(FILE* fp, int* width, int* height, uint16* format);
  uint8* readDDS(FILE* fp, int* width, int* height, uint16* format);
};

#endif /* __TXIMAGE_H__ */
