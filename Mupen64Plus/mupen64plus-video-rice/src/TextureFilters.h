/*
Copyright (C) 2003 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


#ifndef __FILTERS_H__
#define __FILTERS_H__

#include "Config.h"
#include "TextureManager.h"

#define DWORD_MAKE(r, g, b, a)   ((uint32) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))
#define WORD_MAKE(r, g, b, a)   ((uint16) (((a) << 12) | ((r) << 8) | ((g) << 4) | (b)))

extern void InitExternalTextures(void);
extern void CloseExternalTextures(void);

void Texture2x_32( DrawInfo &srcInfo, DrawInfo &destInfo);
void Texture2x_16( DrawInfo &srcInfo, DrawInfo &destInfo);

void Texture2x_Interp_32( DrawInfo &srcInfo, DrawInfo &destInfo);
void Texture2x_Interp_16( DrawInfo &srcInfo, DrawInfo &destInfo);

void Super2xSaI_32( uint32 *srcPtr, uint32 *destPtr, uint32 width, uint32 height, uint32 pitch);
void Super2xSaI_16( uint16 *srcPtr, uint16 *destPtr, uint32 width, uint32 height, uint32 pitch);

void hq4x_16( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL );
void hq4x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL );
void hq4x_InitLUTs(void);

void SmoothFilter_32(uint32 *pdata, uint32 width, uint32 height, uint32 pitch, uint32 filter=TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1);
void SmoothFilter_16(uint16 *pdata, uint32 width, uint32 height, uint32 pitch, uint32 filter=TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1);

void SharpenFilter_32(uint32 *pdata, uint32 width, uint32 height, uint32 pitch, uint32 filter=TEXTURE_SHARPEN_ENHANCEMENT);
void SharpenFilter_16(uint16 *pdata, uint32 width, uint32 height, uint32 pitch, uint32 filter=TEXTURE_SHARPEN_ENHANCEMENT);

void hq2x_init(unsigned bits_per_pixel);
void hq2x_16(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void hq2x_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);

void lq2x_16(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void lq2x_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);

typedef enum _IMAGE_FILEFORMAT 
{
   XIFF_BMP = 0,
     XIFF_JPG = 1,
     XIFF_TGA = 2,
     XIFF_PNG = 3,
     XIFF_DDS = 4,
     XIFF_PPM = 5,
     XIFF_DIB = 6,
     XIFF_HDR = 7,
     XIFF_PFM = 8,
     XIFF_FORCE_DWORD = 0x7fffffff
} IMAGE_FILEFORMAT;

typedef struct _IMAGE_INFO
{
   unsigned int Width;
   unsigned int Height;
   unsigned int Depth;
   unsigned int MipLevels;
   int          Format;  /* SURFFORMAT */
   IMAGE_FILEFORMAT ImageFileFormat;
} IMAGE_INFO;

#endif

