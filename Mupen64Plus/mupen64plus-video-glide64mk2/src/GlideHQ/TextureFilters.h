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


#ifndef __TEXTUREFILTERS_H__
#define __TEXTUREFILTERS_H__

/* 16bpp filters are somewhat buggy and output image is not clean.
 * Since there's not much time, we'll just convert them to ARGB8888
 * and use 32bpp filters until fixed.
 * (1:enable hack, 0:disable hack) */
#define _16BPP_HACK 1

#include "TxInternal.h"

/* enhancers */
void hq4x_8888(unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL);

void hq2x_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void hq2xS_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);

void lq2x_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void lq2xS_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);

void Super2xSaI_8888(uint32 *srcPtr, uint32 *destPtr, uint32 width, uint32 height, uint32 pitch);

void Texture2x_32(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);

/* filters */
void SharpFilter_8888(uint32 *src, uint32 srcwidth, uint32 srcheight, uint32 *dest, uint32 filter);

void SmoothFilter_8888(uint32 *src, uint32 srcwidth, uint32 srcheight, uint32 *dest, uint32 filter);

/* helper */
void filter_8888(uint32 *src, uint32 srcwidth, uint32 srcheight, uint32 *dest, uint32 filter);

#if !_16BPP_HACK
void hq4x_init(void);
void hq4x_4444(unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL);
void hq4x_1555(unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL);
void hq4x_565 (unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL);

void hq2x_16(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void hq2xS_16(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);

void lq2x_16(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);
void lq2xS_16(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);

void Super2xSaI_4444(uint16 *srcPtr, uint16 *destPtr, uint32 width, uint32 height, uint32 pitch);
void Super2xSaI_1555(uint16 *srcPtr, uint16 *destPtr, uint32 width, uint32 height, uint32 pitch);
void Super2xSaI_565 (uint16 *srcPtr, uint16 *destPtr, uint32 width, uint32 height, uint32 pitch);
void Super2xSaI_8   (uint8  *srcPtr, uint8  *destPtr, uint32 width, uint32 height, uint32 pitch);

void Texture2x_16(uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height);

void SharpFilter_4444(uint16 *src, uint32 srcwidth, uint32 srcheight, uint16 *dest, uint32 filter);

void SmoothFilter_4444(uint16 *src, uint32 srcwidth, uint32 srcheight, uint16 *dest, uint32 filter);
#endif

#endif /* __TEXTUREFILTERS_H__ */
