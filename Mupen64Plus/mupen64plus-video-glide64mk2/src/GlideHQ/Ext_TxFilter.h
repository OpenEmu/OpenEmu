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

#ifndef __EXT_TXFILTER_H__
#define __EXT_TXFILTER_H__

#ifdef WIN32
#include <windows.h>
#define TXHMODULE HMODULE
#define DLOPEN(a) LoadLibraryW(a)
#define DLCLOSE(a) FreeLibrary(a)
#define DLSYM(a, b) GetProcAddress(a, b)
#define GETCWD(a, b) GetCurrentDirectoryW(a, b)
#define CHDIR(a) SetCurrentDirectoryW(a)
#else
#include <iostream>
#include <dlfcn.h> 
#define MAX_PATH 4095
#define TXHMODULE void*
#define DLOPEN(a) dlopen(a, RTLD_LAZY|RTLD_GLOBAL)
#define DLCLOSE(a) dlclose(a)
#define DLSYM(a, b) dlsym(a, b)
#define GETCWD(a, b) getcwd(b, a)
#define CHDIR(a) chdir(a)
#endif

#ifdef __MSC__
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
#include <stdint.h>
typedef int64_t int64;
typedef uint64_t uint64;
typedef unsigned char boolean;
#endif

#define NO_OPTIONS          0x00000000

#define FILTER_MASK         0x000000ff
#define NO_FILTER           0x00000000
#define SMOOTH_FILTER_MASK  0x0000000f
#define NO_SMOOTH_FILTER    0x00000000
#define SMOOTH_FILTER_1     0x00000001
#define SMOOTH_FILTER_2     0x00000002
#define SMOOTH_FILTER_3     0x00000003
#define SMOOTH_FILTER_4     0x00000004
#define SHARP_FILTER_MASK   0x000000f0
#define NO_SHARP_FILTER     0x00000000
#define SHARP_FILTER_1      0x00000010
#define SHARP_FILTER_2      0x00000020

#define ENHANCEMENT_MASK    0x00000f00
#define NO_ENHANCEMENT      0x00000000
#define X2_ENHANCEMENT      0x00000100
#define X2SAI_ENHANCEMENT   0x00000200
#define HQ2X_ENHANCEMENT    0x00000300
#define LQ2X_ENHANCEMENT    0x00000400
#define HQ4X_ENHANCEMENT    0x00000500
#define HQ2XS_ENHANCEMENT   0x00000600
#define LQ2XS_ENHANCEMENT   0x00000700

#define COMPRESSION_MASK    0x0000f000
#define NO_COMPRESSION      0x00000000
#define FXT1_COMPRESSION    0x00001000
#define NCC_COMPRESSION     0x00002000
#define S3TC_COMPRESSION    0x00003000

#define HIRESTEXTURES_MASK  0x000f0000
#define NO_HIRESTEXTURES    0x00000000
#define GHQ_HIRESTEXTURES   0x00010000
#define RICE_HIRESTEXTURES  0x00020000
#define JABO_HIRESTEXTURES  0x00030000

#define COMPRESS_TEX        0x00100000
#define COMPRESS_HIRESTEX   0x00200000
#define GZ_TEXCACHE         0x00400000
#define GZ_HIRESTEXCACHE    0x00800000
#define DUMP_TEXCACHE       0x01000000
#define DUMP_HIRESTEXCACHE  0x02000000
#define TILE_HIRESTEX       0x04000000
#define UNDEFINED_0         0x08000000
#define FORCE16BPP_HIRESTEX 0x10000000
#define FORCE16BPP_TEX      0x20000000
#define LET_TEXARTISTS_FLY  0x40000000 /* a little freedom for texture artists */
#define DUMP_TEX            0x80000000

#ifndef __GLIDE_H__ /* GLIDE3 */
/* from 3Dfx Interactive Inc. glide.h */
#define GR_TEXFMT_ALPHA_8           0x2
#define GR_TEXFMT_INTENSITY_8       0x3

#define GR_TEXFMT_ALPHA_INTENSITY_44 0x4
#define GR_TEXFMT_P_8                0x5

#define GR_TEXFMT_RGB_565            0xa
#define GR_TEXFMT_ARGB_1555          0xb
#define GR_TEXFMT_ARGB_4444          0xc
#define GR_TEXFMT_ALPHA_INTENSITY_88 0xd

/* from 3Dfx Interactive Inc. g3ext.h */
#define GR_TEXFMT_ARGB_CMP_FXT1      0x11

#define GR_TEXFMT_ARGB_8888          0x12

#define GR_TEXFMT_ARGB_CMP_DXT1      0x16
#define GR_TEXFMT_ARGB_CMP_DXT3      0x18
#define GR_TEXFMT_ARGB_CMP_DXT5      0x1A
#endif /* GLIDE3 */

struct GHQTexInfo {
  unsigned char *data;
  int width;
  int height;
  unsigned short format;

  int smallLodLog2;
  int largeLodLog2;
  int aspectRatioLog2;

  int tiles;
  int untiled_width;
  int untiled_height;

  unsigned char is_hires_tex;
};

/* Callback to display hires texture info.
 * Gonetz <gonetz(at)ngs.ru>
 *
 * void DispInfo(const char *format, ...)
 * {
 *   va_list args;
 *   char buf[INFO_BUF];
 *
 *   va_start(args, format);
 *   vsprintf(buf, format, args);
 *   va_end(args);
 *
 *   printf(buf);
 * }
 */
#define INFO_BUF 4095
typedef void (*dispInfoFuncExt)(const wchar_t *format, ...);

#ifndef TXFILTER_DLL
boolean ext_ghq_init(int maxwidth, /* maximum texture width supported by hardware */
                     int maxheight,/* maximum texture height supported by hardware */
                     int maxbpp,   /* maximum texture bpp supported by hardware */
                     int options,  /* options */
                     int cachesize,/* cache textures to system memory */
                     wchar_t *path,   /* plugin directory. must be smaller than MAX_PATH */
                     wchar_t *ident,  /* name of ROM. must be no longer than 64 in character. */
                     dispInfoFuncExt callback /* callback function to display info */
                     );

void ext_ghq_shutdown(void);

boolean ext_ghq_txfilter(unsigned char *src,        /* input texture */
                         int srcwidth,              /* width of input texture */
                         int srcheight,             /* height of input texture */
                         unsigned short srcformat,  /* format of input texture */
                         uint64 g64crc,             /* glide64 crc */
                         GHQTexInfo *info           /* output */
                         );

boolean ext_ghq_hirestex(uint64 g64crc,             /* glide64 crc */
                         uint64 r_crc64,            /* checksum hi:palette low:texture */
                         unsigned short *palette,   /* palette for CI textures */
                         GHQTexInfo *info           /* output */
                         );

uint64 ext_ghq_checksum(unsigned char *src, /* input texture */
                        int width,          /* width of texture */
                        int height,         /* height of texture */
                        int size,           /* type of texture pixel */
                        int rowStride,      /* row stride in bytes */
                        unsigned char *palette /* palette */
                        );

boolean ext_ghq_dmptx(unsigned char *src,   /* input texture (must be in 3Dfx Glide format) */
                      int width,            /* width of texture */
                      int height,           /* height of texture */
                      int rowStridePixel,   /* row stride of input texture in pixels */
                      unsigned short gfmt,  /* glide format of input texture */
                      unsigned short n64fmt,/* N64 format hi:format low:size */
                      uint64 r_crc64        /* checksum hi:palette low:texture */
                      );

boolean ext_ghq_reloadhirestex();
#endif /* TXFILTER_DLL */

#endif /* __EXT_TXFILTER_H__ */
