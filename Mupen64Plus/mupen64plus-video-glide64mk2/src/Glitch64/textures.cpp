/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2002  Dave2001
* Copyright (c) 2003-2009  Sergey 'Gonetz' Lipski
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef _WIN32
#include <windows.h>
#else // _WIN32
#include <stdlib.h>
#endif // _WIN32
#include "glide.h"
#include "main.h"
#include <stdio.h>

/* Napalm extensions to GrTextureFormat_t */
#define GR_TEXFMT_ARGB_CMP_FXT1           0x11
#define GR_TEXFMT_ARGB_8888               0x12
#define GR_TEXFMT_YUYV_422                0x13
#define GR_TEXFMT_UYVY_422                0x14
#define GR_TEXFMT_AYUV_444                0x15
#define GR_TEXFMT_ARGB_CMP_DXT1           0x16
#define GR_TEXFMT_ARGB_CMP_DXT2           0x17
#define GR_TEXFMT_ARGB_CMP_DXT3           0x18
#define GR_TEXFMT_ARGB_CMP_DXT4           0x19
#define GR_TEXFMT_ARGB_CMP_DXT5           0x1A
#define GR_TEXTFMT_RGB_888                0xFF

int TMU_SIZE = 8*2048*2048;
static unsigned char* texture = NULL;

int packed_pixels_support = -1;
int ati_sucks = -1;
float largest_supported_anisotropy = 1.0f;

#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif

int tex0_width, tex0_height, tex1_width, tex1_height;
float lambda;

static int min_filter0, mag_filter0, wrap_s0, wrap_t0;
static int min_filter1, mag_filter1, wrap_s1, wrap_t1;

unsigned char *filter(unsigned char *source, int width, int height, int *width2, int *height2);

typedef struct _texlist
{
  unsigned int id;
  struct _texlist *next;
} texlist;

static int nbTex = 0;
static texlist *list = NULL;

#ifdef _WIN32
extern PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB;
#endif
void remove_tex(unsigned int idmin, unsigned int idmax)
{
  unsigned int *t;
  int n = 0;
  texlist *aux = list;
  int sz = nbTex;
  if (aux == NULL) return;
  t = (unsigned int*)malloc(sz * sizeof(int));
  while (aux && aux->id >= idmin && aux->id < idmax)
  {
    if (n >= sz)
      t = (unsigned int *) realloc(t, ++sz*sizeof(int));
    t[n++] = aux->id;
    aux = aux->next;
    free(list);
    list = aux;
    nbTex--;
  }
  while (aux != NULL && aux->next != NULL)
  {
    if (aux->next->id >= idmin && aux->next->id < idmax)
    {
      texlist *aux2 = aux->next->next;
      if (n >= sz)
        t = (unsigned int *) realloc(t, ++sz*sizeof(int));
      t[n++] = aux->next->id;
      free(aux->next);
      aux->next = aux2;
      nbTex--;
    }
    aux = aux->next;
  }
  glDeleteTextures(n, t);
  free(t);
  //printf("RMVTEX nbtex is now %d (%06x - %06x)\n", nbTex, idmin, idmax);
}


void add_tex(unsigned int id)
{
  texlist *aux = list;
  texlist *aux2;
  //printf("ADDTEX nbtex is now %d (%06x)\n", nbTex, id);
  if (list == NULL || id < list->id)
  {
    nbTex++;
    list = (texlist*)malloc(sizeof(texlist));
    list->next = aux;
    list->id = id;
    return;
  }
  while (aux->next != NULL && aux->next->id < id) aux = aux->next;
  // ZIGGY added this test so that add_tex now accept re-adding an existing texture
  if (aux->next != NULL && aux->next->id == id) return;
  nbTex++;
  aux2 = aux->next;
  aux->next = (texlist*)malloc(sizeof(texlist));
  aux->next->id = id;
  aux->next->next = aux2;
}

void init_textures()
{
  tex0_width = tex0_height = tex1_width = tex1_height = 2;
  // ZIGGY because remove_tex isn't called (Pj64 doesn't like it), it's better
  // to leave these so that they'll be reused (otherwise we have a memory leak)
  // 	list = NULL;
  // 	nbTex = 0;

  if (!texture)	texture = (unsigned char*)malloc(2048*2048*4);
}

void free_textures()
{
#ifndef WIN32
  // ZIGGY for some reasons, Pj64 doesn't like remove_tex on exit
  remove_tex(0x00000000, 0xFFFFFFFF);
#endif
  if (texture != NULL) {
    free(texture);
    texture = NULL;
  }
}

FX_ENTRY FxU32 FX_CALL
grTexMinAddress( GrChipID_t tmu )
{
  LOG("grTexMinAddress(%d)\r\n", tmu);
  if (UMAmode)
    return 0;
  else
    return tmu*TMU_SIZE;
}

FX_ENTRY FxU32 FX_CALL
grTexMaxAddress( GrChipID_t tmu )
{
  LOG("grTexMaxAddress(%d)\r\n", tmu);
  if (UMAmode)
    return TMU_SIZE*2 - 1;
  else
    return tmu*TMU_SIZE + TMU_SIZE - 1;
}

FX_ENTRY FxU32 FX_CALL
grTexTextureMemRequired( FxU32     evenOdd,
                        GrTexInfo *info   )
{
  int width, height;
  LOG("grTextureMemRequired(%d)\r\n", evenOdd);
  if (info->largeLodLog2 != info->smallLodLog2) display_warning("grTexTextureMemRequired : loading more than one LOD");

  if (info->aspectRatioLog2 < 0)
  {
    height = 1 << info->largeLodLog2;
    width = height >> -info->aspectRatioLog2;
  }
  else
  {
    width = 1 << info->largeLodLog2;
    height = width >> info->aspectRatioLog2;
  }

  switch(info->format)
  {
  case GR_TEXFMT_ALPHA_8:
  case GR_TEXFMT_INTENSITY_8: // I8 support - H.Morii
  case GR_TEXFMT_ALPHA_INTENSITY_44:
    return width*height;
    break;
  case GR_TEXFMT_ARGB_1555:
  case GR_TEXFMT_ARGB_4444:
  case GR_TEXFMT_ALPHA_INTENSITY_88:
  case GR_TEXFMT_RGB_565:
    return (width*height)<<1;
    break;
  case GR_TEXFMT_ARGB_8888:
    return (width*height)<<2;
    break;
  case GR_TEXFMT_ARGB_CMP_DXT1:  // FXT1,DXT1,5 support - H.Morii
    return ((((width+0x3)&~0x3)*((height+0x3)&~0x3))>>1);
  case GR_TEXFMT_ARGB_CMP_DXT3:
    return ((width+0x3)&~0x3)*((height+0x3)&~0x3);
  case GR_TEXFMT_ARGB_CMP_DXT5:
    return ((width+0x3)&~0x3)*((height+0x3)&~0x3);
  case GR_TEXFMT_ARGB_CMP_FXT1:
    return ((((width+0x7)&~0x7)*((height+0x3)&~0x3))>>1);
  default:
    display_warning("grTexTextureMemRequired : unknown texture format: %x", info->format);
  }
  return 0;
}

FX_ENTRY FxU32 FX_CALL
grTexCalcMemRequired(
                     GrLOD_t lodmin, GrLOD_t lodmax,
                     GrAspectRatio_t aspect, GrTextureFormat_t fmt)
{
  int width, height;
  LOG("grTexCalcMemRequired(%d, %d, %d, %d)\r\n", lodmin, lodmax, aspect, fmt);
  if (lodmax != lodmin) display_warning("grTexCalcMemRequired : loading more than one LOD");

  if (aspect < 0)
  {
    height = 1 << lodmax;
    width = height >> -aspect;
  }
  else
  {
    width = 1 << lodmax;
    height = width >> aspect;
  }

  switch(fmt)
  {
  case GR_TEXFMT_ALPHA_8:
  case GR_TEXFMT_INTENSITY_8: // I8 support - H.Morii
  case GR_TEXFMT_ALPHA_INTENSITY_44:
    return width*height;
    break;
  case GR_TEXFMT_ARGB_1555:
  case GR_TEXFMT_ARGB_4444:
  case GR_TEXFMT_ALPHA_INTENSITY_88:
  case GR_TEXFMT_RGB_565:
    return (width*height)<<1;
    break;
  case GR_TEXFMT_ARGB_8888:
    return (width*height)<<2;
    break;
  case GR_TEXFMT_ARGB_CMP_DXT1:  // FXT1,DXT1,5 support - H.Morii
    return ((((width+0x3)&~0x3)*((height+0x3)&~0x3))>>1);
  case GR_TEXFMT_ARGB_CMP_DXT3:
    return ((width+0x3)&~0x3)*((height+0x3)&~0x3);
  case GR_TEXFMT_ARGB_CMP_DXT5:
    return ((width+0x3)&~0x3)*((height+0x3)&~0x3);
  case GR_TEXFMT_ARGB_CMP_FXT1:
    return ((((width+0x7)&~0x7)*((height+0x3)&~0x3))>>1);
  default:
    display_warning("grTexTextureMemRequired : unknown texture format: %x", fmt);
  }
  return 0;
}

int grTexFormatSize(int fmt)
{
  int factor = -1;
  switch(fmt) {
  case GR_TEXFMT_ALPHA_8:
  case GR_TEXFMT_INTENSITY_8: // I8 support - H.Morii
    factor = 1;
    break;
  case GR_TEXFMT_ALPHA_INTENSITY_44:
    factor = 1;
    break;
  case GR_TEXFMT_RGB_565:
    factor = 2;
    break;
  case GR_TEXFMT_ARGB_1555:
    factor = 2;
    break;
  case GR_TEXFMT_ALPHA_INTENSITY_88:
    factor = 2;
    break;
  case GR_TEXFMT_ARGB_4444:
    factor = 2;
    break;
  case GR_TEXFMT_ARGB_8888:
    factor = 4;
    break;
  case GR_TEXFMT_ARGB_CMP_DXT1:  // FXT1,DXT1,5 support - H.Morii
    factor = 8;                  // HACKALERT: factor holds block bytes
    break;
  case GR_TEXFMT_ARGB_CMP_DXT3:  // FXT1,DXT1,5 support - H.Morii
    factor = 16;                  // HACKALERT: factor holds block bytes
    break;
  case GR_TEXFMT_ARGB_CMP_DXT5:
    factor = 16;
    break;
  case GR_TEXFMT_ARGB_CMP_FXT1:
    factor = 8;
    break;
  default:
    display_warning("grTexFormatSize : unknown texture format: %x", fmt);
  }
  return factor;
}

int grTexFormat2GLPackedFmt(int fmt, int * gltexfmt, int * glpixfmt, int * glpackfmt)
{
  int factor = -1;
  switch(fmt) {
  case GR_TEXFMT_ALPHA_8:
    factor = 1;
    *gltexfmt = GL_INTENSITY8;
    *glpixfmt = GL_LUMINANCE;
    *glpackfmt = GL_UNSIGNED_BYTE;
    break;
  case GR_TEXFMT_INTENSITY_8: // I8 support - H.Morii
    factor = 1;
    *gltexfmt = GL_LUMINANCE8;
    *glpixfmt = GL_LUMINANCE;
    *glpackfmt = GL_UNSIGNED_BYTE;
    break;
  case GR_TEXFMT_ALPHA_INTENSITY_44:
    break;
  case GR_TEXFMT_RGB_565:
    factor = 2;
    *gltexfmt = GL_RGB;
    *glpixfmt = GL_RGB;
    *glpackfmt = GL_UNSIGNED_SHORT_5_6_5;
    break;
  case GR_TEXFMT_ARGB_1555:
    if (ati_sucks > 0) return -1; // ATI sucks as usual (fixes slowdown on ATI)
    factor = 2;
    *gltexfmt = GL_RGB5_A1;
    *glpixfmt = GL_BGRA;
    *glpackfmt = GL_UNSIGNED_SHORT_1_5_5_5_REV;
    break;
  case GR_TEXFMT_ALPHA_INTENSITY_88:
    factor = 2;
    *gltexfmt = GL_LUMINANCE8_ALPHA8;
    *glpixfmt = GL_LUMINANCE_ALPHA;
    *glpackfmt = GL_UNSIGNED_BYTE;
    break;
  case GR_TEXFMT_ARGB_4444:
    factor = 2;
    *gltexfmt = GL_RGBA4;
    *glpixfmt = GL_BGRA;
    *glpackfmt = GL_UNSIGNED_SHORT_4_4_4_4_REV;
    break;
  case GR_TEXFMT_ARGB_8888:
    factor = 4;
    *gltexfmt = GL_RGBA8;
    *glpixfmt = GL_BGRA;
    *glpackfmt = GL_UNSIGNED_INT_8_8_8_8_REV;
    break;
  case GR_TEXFMT_ARGB_CMP_DXT1:  // FXT1,DXT1,5 support - H.Morii
    // HACKALERT: 3Dfx Glide uses GR_TEXFMT_ARGB_CMP_DXT1 for both opaque DXT1 and DXT1 with 1bit alpha.
    // GlideHQ compiled with GLIDE64_DXTN option enabled, uses opaqe DXT1 only.
    factor = 8; // HACKALERT: factor holds block bytes
    *gltexfmt = GL_COMPRESSED_RGB_S3TC_DXT1_EXT; // these variables aren't used
    *glpixfmt = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    *glpackfmt = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    break;
  case GR_TEXFMT_ARGB_CMP_DXT3:
    factor = 16;
    *gltexfmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    *glpixfmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    *glpackfmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    break;
  case GR_TEXFMT_ARGB_CMP_DXT5:
    factor = 16;
    *gltexfmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    *glpixfmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    *glpackfmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    break;
  case GR_TEXFMT_ARGB_CMP_FXT1:
    factor = 8;
    *gltexfmt = 0x86B1; // GL_COMPRESSED_RGBA_FXT1_3DFX
    *glpixfmt = 0x86B1; // GL_COMPRESSED_RGBA_FXT1_3DFX
    *glpackfmt = 0x86B1; // XXX: what should we do about GL_COMPRESSED_RGB_FXT1_3DFX?
    break;
  default:
    display_warning("grTexFormat2GLPackedFmt : unknown texture format: %x", fmt);
  }
  return factor;
}

FX_ENTRY void FX_CALL
grTexDownloadMipMap( GrChipID_t tmu,
                    FxU32      startAddress,
                    FxU32      evenOdd,
                    GrTexInfo  *info )
{
  int width, height, i, j;
  int factor;
  int glformat = 0;
  int gltexfmt, glpixfmt, glpackfmt;
  LOG("grTexDownloadMipMap(%d,%d,%d)\r\n", tmu, startAddress, evenOdd);
  if (info->largeLodLog2 != info->smallLodLog2) display_warning("grTexDownloadMipMap : loading more than one LOD");

  if (info->aspectRatioLog2 < 0)
  {
    height = 1 << info->largeLodLog2;
    width = height >> -info->aspectRatioLog2;
  }
  else
  {
    width = 1 << info->largeLodLog2;
    height = width >> info->aspectRatioLog2;
  }

  if (!packed_pixels_support)
    factor = -1;
  else
    factor = grTexFormat2GLPackedFmt(info->format, &gltexfmt, &glpixfmt, &glpackfmt);

  if (factor < 0) {

    // VP fixed the texture conversions to be more accurate, also swapped
    // the for i/j loops so that is is less likely to break the memory cache
    register int n = 0, m = 0;
    switch(info->format)
    {
    case GR_TEXFMT_ALPHA_8:
      for (i=0; i<height; i++)
      {
        for (j=0; j<width; j++)
        {
          unsigned int texel = (unsigned int)((unsigned char*)info->data)[m];
          texel |= (texel << 8);
          texel |= (texel << 16);
          ((unsigned int*)texture)[n] = texel;
          m++;
          n++;
        }
      }
      factor = 1;
      glformat = GL_INTENSITY8;
      break;
    case GR_TEXFMT_INTENSITY_8: // I8 support - H.Morii
      for (i=0; i<height; i++)
      {
        for (j=0; j<width; j++)
        {
          unsigned int texel = (unsigned int)((unsigned char*)info->data)[m];
          texel |= (0xFF000000 | (texel << 16) | (texel << 8));
          ((unsigned int*)texture)[n] = texel;
          m++;
          n++;
        }
      }
      factor = 1;
      glformat = GL_LUMINANCE8;
      break;
    case GR_TEXFMT_ALPHA_INTENSITY_44:
#if 1
      for (i=0; i<height; i++)
      {
        for (j=0; j<width; j++)
        {
          unsigned int texel = (unsigned int)((unsigned char*)info->data)[m];
#if 1
          /* accurate conversion */
          unsigned int texel_hi = (texel & 0x000000F0) << 20;
          unsigned int texel_low = texel & 0x0000000F;
          texel_low |= (texel_low << 4);
          texel_hi |= ((texel_hi << 4) | (texel_low << 16) | (texel_low << 8) | texel_low);
#else
          unsigned int texel_hi = (texel & 0x000000F0) << 24;
          unsigned int texel_low = (texel & 0x0000000F) << 4;
          texel_hi |= ((texel_low << 16) | (texel_low << 8) | texel_low);
#endif
          ((unsigned int*)texture)[n] = texel_hi;
          m++;
          n++;
        }
      }
      factor = 1;
      glformat = GL_LUMINANCE4_ALPHA4;
#endif
      break;
    case GR_TEXFMT_RGB_565:
      for (i=0; i<height; i++)
      {
        for (j=0; j<width; j++)
        {
          unsigned int texel = (unsigned int)((unsigned short*)info->data)[m];
          unsigned int B = texel & 0x0000F800;
          unsigned int G = texel & 0x000007E0;
          unsigned int R = texel & 0x0000001F;
#if 0
          /* accurate conversion */
          ((unsigned int*)texture)[n] = 0xFF000000 | (R << 19) | ((R >> 2) << 16) | (G << 5) | ((G >> 9) << 8) | (B >> 8) | (B >> 13);
#else
          ((unsigned int*)texture)[n] = 0xFF000000 | (R << 19) | (G << 5) | (B >> 8);
#endif
          m++;
          n++;
        }
      }
      factor = 2;
      glformat = GL_RGB;
      break;
    case GR_TEXFMT_ARGB_1555:
      for (i=0; i<height; i++)
      {
        for (j=0; j<width; j++)
        {
          unsigned int texel = (unsigned int)((unsigned short*)info->data)[m];
          unsigned int A = texel & 0x00008000 ? 0xFF000000 : 0;
          unsigned int B = texel & 0x00007C00;
          unsigned int G = texel & 0x000003E0;
          unsigned int R = texel & 0x0000001F;
#if 0
          /* accurate conversion */
          ((unsigned int*)texture)[n] = A | (R << 19) | ((R >> 2) << 16) | (G << 6) | ((G >> 8) << 8) | (B >> 7) | (B >> 12);
#else
          ((unsigned int*)texture)[n] = A | (R << 19) | (G << 6) | (B >> 7);
#endif
          m++;
          n++;
        }
      }
      factor = 2;
      glformat = GL_RGB5_A1;
      break;
    case GR_TEXFMT_ALPHA_INTENSITY_88:
      for (i=0; i<height; i++)
      {
        for (j=0; j<width; j++)
        {
          unsigned int AI = (unsigned int)((unsigned short*)info->data)[m];
          unsigned int I = (unsigned int)(AI & 0x000000FF);
          ((unsigned int*)texture)[n] = (AI << 16) | (I << 8) | I;
          m++;
          n++;
        }
      }
      factor = 2;
      glformat = GL_LUMINANCE8_ALPHA8;
      break;
    case GR_TEXFMT_ARGB_4444:

      for (i=0; i<height; i++)
      {
        for (j=0; j<width; j++)
        {
          unsigned int texel = (unsigned int)((unsigned short*)info->data)[m];
          unsigned int A = texel & 0x0000F000;
          unsigned int B = texel & 0x00000F00;
          unsigned int G = texel & 0x000000F0;
          unsigned int R = texel & 0x0000000F;
#if 0
          /* accurate conversion */
          ((unsigned int*)texture)[n] = (A << 16) | (A << 12) | (R << 20) | (R << 16) | (G << 8) | (G << 4) | (B >> 4) | (B >> 8);
#else
          ((unsigned int*)texture)[n] = (A << 16) | (R << 20) | (G << 8) | (B >> 4);
#endif
          m++;
          n++;
        }
      }
      factor = 2;
      glformat = GL_RGBA4;
      break;
    case GR_TEXFMT_ARGB_8888:
      for (i=0; i<height; i++)
      {
        for (j=0; j<width; j++)
        {
          unsigned int texel = ((unsigned int*)info->data)[m];
          unsigned int A = texel & 0xFF000000;
          unsigned int B = texel & 0x00FF0000;
          unsigned int G = texel & 0x0000FF00;
          unsigned int R = texel & 0x000000FF;
          ((unsigned int*)texture)[n] = A | (R << 16) | G | (B >> 16);
          m++;
          n++;
        }
      }
      factor = 4;
      glformat = GL_RGBA8;
      break;
    case GR_TEXFMT_ARGB_CMP_DXT1: // FXT1,DXT1,5 support - H.Morii
      factor = 8;                 // HACKALERT: factor holds block bytes
      glformat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
      break;
    case GR_TEXFMT_ARGB_CMP_DXT3: // FXT1,DXT1,5 support - H.Morii
      factor = 16;                 // HACKALERT: factor holds block bytes
      glformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      break;
    case GR_TEXFMT_ARGB_CMP_DXT5:
      factor = 16;
      glformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
      break;
    case GR_TEXFMT_ARGB_CMP_FXT1:
      factor = 8;
      glformat = 0x86B1; // GL_COMPRESSED_RGBA_FXT1_3DFX
      break;
    default:
      display_warning("grTexDownloadMipMap : unknown texture format: %x", info->format);
      factor = 0;
    }
  }

  if (nbTextureUnits <= 2)
    glActiveTextureARB(GL_TEXTURE1_ARB);
  else
    glActiveTextureARB(GL_TEXTURE2_ARB);

  switch(info->format)
  {
  case GR_TEXFMT_ARGB_CMP_DXT1:
  case GR_TEXFMT_ARGB_CMP_DXT3:
  case GR_TEXFMT_ARGB_CMP_DXT5:
  case GR_TEXFMT_ARGB_CMP_FXT1:
    remove_tex(startAddress+1, startAddress+1+((width*height*factor)>>4));
    break;
  default:
    remove_tex(startAddress+1, startAddress+1+(width*height*factor));
  }

  add_tex(startAddress+1);
  glBindTexture(GL_TEXTURE_2D, startAddress+1);

  if (largest_supported_anisotropy > 1.0f)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);

  switch(info->format)
  {
  case GR_TEXFMT_ARGB_CMP_DXT1:
  case GR_TEXFMT_ARGB_CMP_DXT3:
  case GR_TEXFMT_ARGB_CMP_DXT5:
  case GR_TEXFMT_ARGB_CMP_FXT1:
    glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, (glformat ? glformat : gltexfmt), width, height, 0, (width*height*factor)>>4, info->data);
    break;
  default:
    if (glformat) {
      glTexImage2D(GL_TEXTURE_2D, 0, glformat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);
    } else
      glTexImage2D(GL_TEXTURE_2D, 0, gltexfmt, width, height, 0, glpixfmt, glpackfmt, info->data);
  }

  glBindTexture(GL_TEXTURE_2D, default_texture);
}

int CheckTextureBufferFormat(GrChipID_t tmu, FxU32 startAddress, GrTexInfo *info );

FX_ENTRY void FX_CALL
grTexSource( GrChipID_t tmu,
            FxU32      startAddress,
            FxU32      evenOdd,
            GrTexInfo  *info )
{
  LOG("grTexSource(%d,%d,%d)\r\n", tmu, startAddress, evenOdd);

  if (tmu == GR_TMU1 || nbTextureUnits <= 2)
  {
    if (tmu == GR_TMU1 && nbTextureUnits <= 2) return;
    glActiveTextureARB(GL_TEXTURE0_ARB);

    if (info->aspectRatioLog2 < 0)
    {
      tex0_height = 256;
      tex0_width = tex0_height >> -info->aspectRatioLog2;
    }
    else
    {
      tex0_width = 256;
      tex0_height = tex0_width >> info->aspectRatioLog2;
    }

    glBindTexture(GL_TEXTURE_2D, startAddress+1);
#ifdef VPDEBUG
    dump_tex(startAddress+1);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t0);
  }
  else
  {
    glActiveTextureARB(GL_TEXTURE1_ARB);

    if (info->aspectRatioLog2 < 0)
    {
      tex1_height = 256;
      tex1_width = tex1_height >> -info->aspectRatioLog2;
    }
    else
    {
      tex1_width = 256;
      tex1_height = tex1_width >> info->aspectRatioLog2;
    }

    glBindTexture(GL_TEXTURE_2D, startAddress+1);
#ifdef VPDEBUG
    dump_tex(startAddress+1);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t1);
  }
  if(!CheckTextureBufferFormat(tmu, startAddress+1, info))
  {
    if(tmu == 0 && blackandwhite1 != 0)
    {
      blackandwhite1 = 0;
      need_to_compile = 1;
    }
    if(tmu == 1 && blackandwhite0 != 0)
    {
      blackandwhite0 = 0;
      need_to_compile = 1;
    }
  }

#if 0
  extern int auxbuffer;
  static int oldbuffer;
  FX_ENTRY void FX_CALL grAuxBufferExt( GrBuffer_t buffer );
  if (auxbuffer == GR_BUFFER_AUXBUFFER && auxbuffer != oldbuffer)
    grAuxBufferExt(auxbuffer);
  oldbuffer = auxbuffer;
#endif
}

FX_ENTRY void FX_CALL
grTexDetailControl(
                   GrChipID_t tmu,
                   int lod_bias,
                   FxU8 detail_scale,
                   float detail_max
                   )
{
  LOG("grTexDetailControl(%d,%d,%d,%d)\r\n", tmu, lod_bias, detail_scale, detail_max);
  if (lod_bias != 31 && detail_scale != 7)
  {
    if (!lod_bias && !detail_scale && !detail_max) return;
    else
      display_warning("grTexDetailControl : %d, %d, %f", lod_bias, detail_scale, detail_max);
  }
  lambda = detail_max;
  if(lambda > 1.0f)
  {
    lambda = 1.0f - (255.0f - lambda);
  }
  if(lambda > 1.0f) display_warning("lambda:%f", lambda);

  set_lambda();
}

FX_ENTRY void FX_CALL
grTexLodBiasValue(GrChipID_t tmu, float bias )
{
  LOG("grTexLodBiasValue(%d,%f)\r\n", tmu, bias);
}

FX_ENTRY void FX_CALL
grTexFilterMode(
                GrChipID_t tmu,
                GrTextureFilterMode_t minfilter_mode,
                GrTextureFilterMode_t magfilter_mode
                )
{
  LOG("grTexFilterMode(%d,%d,%d)\r\n", tmu, minfilter_mode, magfilter_mode);
  if (tmu == GR_TMU1 || nbTextureUnits <= 2)
  {
    if (tmu == GR_TMU1 && nbTextureUnits <= 2) return;
    if (minfilter_mode == GR_TEXTUREFILTER_POINT_SAMPLED) min_filter0 = GL_NEAREST;
    else min_filter0 = GL_LINEAR;

    if (magfilter_mode == GR_TEXTUREFILTER_POINT_SAMPLED) mag_filter0 = GL_NEAREST;
    else mag_filter0 = GL_LINEAR;

    glActiveTextureARB(GL_TEXTURE0_ARB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter0);
  }
  else
  {
    if (minfilter_mode == GR_TEXTUREFILTER_POINT_SAMPLED) min_filter1 = GL_NEAREST;
    else min_filter1 = GL_LINEAR;

    if (magfilter_mode == GR_TEXTUREFILTER_POINT_SAMPLED) mag_filter1 = GL_NEAREST;
    else mag_filter1 = GL_LINEAR;

    glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter1);
  }
}

FX_ENTRY void FX_CALL
grTexClampMode(
               GrChipID_t tmu,
               GrTextureClampMode_t s_clampmode,
               GrTextureClampMode_t t_clampmode
               )
{
  LOG("grTexClampMode(%d, %d, %d)\r\n", tmu, s_clampmode, t_clampmode);
  if (tmu == GR_TMU1 || nbTextureUnits <= 2)
  {
    if (tmu == GR_TMU1 && nbTextureUnits <= 2) return;
    switch(s_clampmode)
    {
    case GR_TEXTURECLAMP_WRAP:
      wrap_s0 = GL_REPEAT;
      break;
    case GR_TEXTURECLAMP_CLAMP:
      wrap_s0 = GL_CLAMP_TO_EDGE;
      break;
    case GR_TEXTURECLAMP_MIRROR_EXT:
      wrap_s0 = GL_MIRRORED_REPEAT_ARB;
      break;
    default:
      display_warning("grTexClampMode : unknown s_clampmode : %x", s_clampmode);
    }
    switch(t_clampmode)
    {
    case GR_TEXTURECLAMP_WRAP:
      wrap_t0 = GL_REPEAT;
      break;
    case GR_TEXTURECLAMP_CLAMP:
      wrap_t0 = GL_CLAMP_TO_EDGE;
      break;
    case GR_TEXTURECLAMP_MIRROR_EXT:
      wrap_t0 = GL_MIRRORED_REPEAT_ARB;
      break;
    default:
      display_warning("grTexClampMode : unknown t_clampmode : %x", t_clampmode);
    }
    glActiveTextureARB(GL_TEXTURE0_ARB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t0);
  }
  else
  {
    switch(s_clampmode)
    {
    case GR_TEXTURECLAMP_WRAP:
      wrap_s1 = GL_REPEAT;
      break;
    case GR_TEXTURECLAMP_CLAMP:
      wrap_s1 = GL_CLAMP_TO_EDGE;
      break;
    case GR_TEXTURECLAMP_MIRROR_EXT:
      wrap_s1 = GL_MIRRORED_REPEAT_ARB;
      break;
    default:
      display_warning("grTexClampMode : unknown s_clampmode : %x", s_clampmode);
    }
    switch(t_clampmode)
    {
    case GR_TEXTURECLAMP_WRAP:
      wrap_t1 = GL_REPEAT;
      break;
    case GR_TEXTURECLAMP_CLAMP:
      wrap_t1 = GL_CLAMP_TO_EDGE;
      break;
    case GR_TEXTURECLAMP_MIRROR_EXT:
      wrap_t1 = GL_MIRRORED_REPEAT_ARB;
      break;
    default:
      display_warning("grTexClampMode : unknown t_clampmode : %x", t_clampmode);
    }
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t1);
  }
}
