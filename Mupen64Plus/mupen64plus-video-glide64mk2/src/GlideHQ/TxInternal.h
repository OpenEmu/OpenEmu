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

#ifndef __INTERNAL_H__
#define __INTERNAL_H__

#include "Ext_TxFilter.h"

/* dll exports */
#ifdef TXFILTER_DLL
#define TAPI __declspec(dllexport)
#define TAPIENTRY
#else
#define TAPI
#define TAPIENTRY
#endif

#include <stdint.h>

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t  uint32;

#ifdef WIN32
#define KBHIT(key) ((GetAsyncKeyState(key) & 0x8001) == 0x8001)
#else
#define KBHIT(key) (0)
#endif

/* from OpenGL glext.h */
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3

/* for explicit fxt1 compression */
#define CC_CHROMA 0x0
#define CC_HI     0x1
#define CC_ALPHA  0x2

/* in-memory zlib texture compression */
#define GR_TEXFMT_GZ                 0x8000

#if 0 /* this is here to remind me of other formats */
/* from 3Dfx Interactive Inc. glide.h */
#define GR_TEXFMT_8BIT                  0x0
#define GR_TEXFMT_RGB_332               GR_TEXFMT_8BIT
#define GR_TEXFMT_YIQ_422               0x1
#define GR_TEXFMT_ALPHA_8               0x2 /* (0..0xFF) alpha     */
#define GR_TEXFMT_INTENSITY_8           0x3 /* (0..0xFF) intensity */
#define GR_TEXFMT_ALPHA_INTENSITY_44    0x4
#define GR_TEXFMT_P_8                   0x5 /* 8-bit palette */
#define GR_TEXFMT_RSVD0                 0x6 /* GR_TEXFMT_P_8_RGBA */
#define GR_TEXFMT_P_8_6666              GR_TEXFMT_RSVD0
#define GR_TEXFMT_P_8_6666_EXT          GR_TEXFMT_RSVD0
#define GR_TEXFMT_RSVD1                 0x7
#define GR_TEXFMT_16BIT                 0x8
#define GR_TEXFMT_ARGB_8332             GR_TEXFMT_16BIT
#define GR_TEXFMT_AYIQ_8422             0x9
#define GR_TEXFMT_RGB_565               0xa
#define GR_TEXFMT_ARGB_1555             0xb
#define GR_TEXFMT_ARGB_4444             0xc
#define GR_TEXFMT_ALPHA_INTENSITY_88    0xd
#define GR_TEXFMT_AP_88                 0xe /* 8-bit alpha 8-bit palette */
#define GR_TEXFMT_RSVD2                 0xf
#define GR_TEXFMT_RSVD4                 GR_TEXFMT_RSVD2

/* from 3Dfx Interactive Inc. g3ext.h */
#define GR_TEXFMT_ARGB_CMP_FXT1        0x11
#define GR_TEXFMT_ARGB_8888            0x12
#define GR_TEXFMT_YUYV_422             0x13
#define GR_TEXFMT_UYVY_422             0x14
#define GR_TEXFMT_AYUV_444             0x15
#define GR_TEXFMT_ARGB_CMP_DXT1        0x16
#define GR_TEXFMT_ARGB_CMP_DXT2        0x17
#define GR_TEXFMT_ARGB_CMP_DXT3        0x18
#define GR_TEXFMT_ARGB_CMP_DXT4        0x19
#define GR_TEXFMT_ARGB_CMP_DXT5        0x1A
#define GR_TEXTFMT_RGB_888             0xFF
#endif

#endif /* __INTERNAL_H__ */
