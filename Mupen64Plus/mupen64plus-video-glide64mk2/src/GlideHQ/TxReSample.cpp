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

#include "TxReSample.h"
#include "TxDbg.h"
#include <stdlib.h>
#include <memory.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int
TxReSample::nextPow2(int num)
{
  num = num - 1;
  num = num | (num >> 1);
  num = num | (num >> 2);
  num = num | (num >> 4);
  num = num | (num >> 8);
  num = num | (num >> 16);
  /*num = num | (num >> 32);*//* for 64bit architecture */
  num = num + 1;

  return num;
}

boolean
TxReSample::nextPow2(uint8** image, int* width, int* height, int bpp, boolean use_3dfx = 0)
{
  /* NOTE: bpp must be one of the follwing: 8, 16, 24, 32 bits per pixel */

  if (!*image || !*width || !*height || !bpp)
    return 0;

  int row_bytes = ((*width * bpp) >> 3);
  int o_row_bytes = row_bytes;
  int o_width = *width;
  int n_width = *width;
  int o_height = *height;
  int n_height = *height;

  /* HACKALERT: I have explicitly subtracted (n) from width/height to
   * adjust textures that have (n) pixel larger width/height than
   * power of 2 size. This is a dirty hack for textures that have
   * munged aspect ratio by (n) pixel to the original.
   */
  if      (n_width  > 64) n_width  -= 4;
  else if (n_width  > 16) n_width  -= 2;
  else if (n_width  >  4) n_width  -= 1;

  if      (n_height > 64) n_height -= 4;
  else if (n_height > 16) n_height -= 2;
  else if (n_height >  4) n_height -= 1;

  n_width = nextPow2(n_width);
  n_height = nextPow2(n_height);
  row_bytes = (n_width * bpp) >> 3;

  /* 3dfx Glide3 format, W:H aspect ratio range (8:1 - 1:8) */
  if (use_3dfx) {
    if (n_width > n_height) {
      if (n_width > (n_height << 3))
        n_height = n_width >> 3;
    } else {
      if (n_height > (n_width << 3)) {
        n_width = n_height >> 3;
        row_bytes = (n_width * bpp) >> 3;
      }
    }
    DBG_INFO(80, L"using 3dfx W:H aspect ratio range (8:1 - 1:8).\n");
  }

  /* do we really need to do this ? */
  if (o_width == n_width && o_height == n_height)
    return 1; /* nope */

  DBG_INFO(80, L"expand image to next power of 2 dimensions. %d x %d -> %d x %d\n",
           o_width, o_height, n_width, n_height);

  if (o_width > n_width)
    o_width = n_width;

  if (o_height > n_height)
    o_height = n_height;

  /* allocate memory to read in image */
  uint8 *pow2image = (uint8*)malloc(row_bytes * n_height);

  /* read in image */
  if (pow2image) {
    int i, j;
    uint8 *tmpimage = *image, *tmppow2image = pow2image;

    for (i = 0; i < o_height; i++) {
      /* copy row */
      memcpy(tmppow2image, tmpimage, ((o_width * bpp) >> 3));

      /* expand to pow2 size by replication */
      for(j = ((o_width * bpp) >> 3); j < row_bytes; j++)
        tmppow2image[j] = tmppow2image[j - (bpp >> 3)];

      tmppow2image += row_bytes;
      tmpimage += o_row_bytes;
    }
    /* expand to pow2 size by replication */
    for (i = o_height; i < n_height; i++)
      memcpy(&pow2image[row_bytes * i], &pow2image[row_bytes * (i - 1)], row_bytes);

    free(*image);

    *image = pow2image;
    *height = n_height;
    *width = n_width;

    return 1;
  }

  return 0;
}

/* Ken Turkowski
 * Filters for Common Resampling Tasks
 * Apple Computer 1990
 */
double
TxReSample::tent(double x)
{
  if (x < 0.0) x = -x;
  if (x < 1.0) return (1.0 - x);
  return 0.0;
}

double
TxReSample::gaussian(double x)
{
  if (x < 0) x = -x;
  if (x < 2.0) return pow(2.0, -2.0 * x * x);
  return 0.0;
}

double 
TxReSample::sinc(double x)
{
  if (x == 0) return 1.0;
  x *= M_PI;
  return (sin(x) / x);
}

double 
TxReSample::lanczos3(double x)
{
  if (x < 0) x = -x;
  if (x < 3.0) return (sinc(x) * sinc(x/3.0));
  return 0.0;
}

/* Don P. Mitchell and Arun N. Netravali
 * Reconstruction Filters in Computer Graphics
 * SIGGRAPH '88
 * Proceedings of the 15th annual conference on Computer 
 * graphics and interactive techniques, pp221-228, 1988
 */
double
TxReSample::mitchell(double x)
{
  if (x < 0) x = -x;
  if (x < 2.0) {
    const double B = 1.0 / 3.0;
    const double C = 1.0 / 3.0;
    if (x < 1.0) {
      x = (((12.0 - 9.0 * B - 6.0 * C) * (x * x * x))
           + ((-18.0 + 12.0 * B + 6.0 * C) * (x * x))
           + (6.0 - 2.0 * B));
    } else {
      x = (((-1.0 * B - 6.0 * C) * (x * x * x))
           + ((6.0 * B + 30.0 * C) * (x * x))
           + ((-12.0 * B - 48.0 * C) * x)
           + (8.0 * B + 24.0 * C));
    }
    return (x / 6.0);
  }
  return 0.0;
}

/* J. F. Kaiser and W. A. Reed
 * Data smoothing using low-pass digital filters
 * Rev. Sci. instrum. 48 (11), pp1447-1457, 1977
 */
double
TxReSample::besselI0(double x)
{
  /* zero-order modified bessel function of the first kind */
  const double eps_coeff = 1E-16; /* small enough */
  double xh, sum, pow, ds;
  xh = 0.5 * x;
  sum = 1.0;
  pow = 1.0;
  ds = 1.0;
  int k = 0;
  while (ds > sum * eps_coeff) {
    k++;
    pow *= (xh / k);
    ds = pow * pow;
    sum = sum + ds;
  }
  return sum;
}

double
TxReSample::kaiser(double x)
{
  const double alpha = 4.0;
  const double half_window = 5.0;
  const double ratio = x / half_window;
  return sinc(x) * besselI0(alpha * sqrt(1 - ratio * ratio)) / besselI0(alpha);
}

boolean
TxReSample::minify(uint8 **src, int *width, int *height, int ratio)
{
  /* NOTE: src must be ARGB8888, ratio is the inverse representation */

#if 0
  if (!*src || ratio < 2) return 0;

  /* Box filtering.
   * It would be nice to do Kaiser filtering.
   * N64 uses narrow strip textures which makes it hard to filter effectively.
   */

  int x, y, x2, y2, offset, numtexel;
  uint32 A, R, G, B, texel;

  int tmpwidth = *width / ratio;
  int tmpheight = *height / ratio;

  uint8 *tmptex = (uint8*)malloc((tmpwidth * tmpheight) << 2);

  if (tmptex) {
    numtexel = ratio * ratio;
    for (y = 0; y < tmpheight; y++) {
      offset = ratio * y * *width;
      for (x = 0; x < tmpwidth; x++) {
        A = R = G = B = 0;
        for (y2 = 0; y2 < ratio; y2++) {
          for (x2 = 0; x2 < ratio; x2++) {
            texel = ((uint32*)*src)[offset + *width * y2 + x2];
            A += (texel >> 24);
            R += ((texel >> 16) & 0x000000ff);
            G += ((texel >> 8) & 0x000000ff);
            B += (texel & 0x000000ff);
          }
        }
        A = (A + ratio) / numtexel;
        R = (R + ratio) / numtexel;
        G = (G + ratio) / numtexel;
        B = (B + ratio) / numtexel;
        ((uint32*)tmptex)[y * tmpwidth + x] = ((A << 24) | (R << 16) | (G << 8) | B);
        offset += ratio;
      }
    }
    free(*src);
    *src = tmptex;
    *width = tmpwidth;
    *height = tmpheight;

    DBG_INFO(80, L"minification ratio:%d -> %d x %d\n", ratio, *width, *height);

    return 1;
  }

  DBG_INFO(80, L"Error: failed minification!\n");

  return 0;

#else

  if (!*src || ratio < 2) return 0;

  /* Image Resampling */
  
  /* half width of filter window.
   * NOTE: must be 1.0 or larger. 
   *
   * kaiser-bessel 5, lanczos3 3, mitchell 2, gaussian 1.5, tent 1
   */
  double half_window = 5.0;

  int x, y, x2, y2, z;
  double A, R, G, B;
  uint32 texel;

  int tmpwidth = *width / ratio;
  int tmpheight = *height / ratio;

  /* resampled destination */
  uint8 *tmptex = (uint8*)malloc((tmpwidth * tmpheight) << 2);
  if (!tmptex) return 0;

  /* work buffer. single row */
  uint8 *workbuf = (uint8*)malloc(*width << 2);
  if (!workbuf) {
    free(tmptex);
    return 0;
  }

  /* prepare filter lookup table. only half width required for symetric filters. */
  double *weight = (double*)malloc((int)((half_window * ratio) * sizeof(double)));
  if (!weight) {
    free(tmptex);
    free(workbuf);
    return 0;
  }
  for (x = 0; x < half_window * ratio; x++) {
    //weight[x] = tent((double)x / ratio) / ratio;
    //weight[x] = gaussian((double)x / ratio) / ratio;
    //weight[x] = lanczos3((double)x / ratio) / ratio;
    //weight[x] = mitchell((double)x / ratio) / ratio;
    weight[x] = kaiser((double)x / ratio) / ratio;
  }

  /* linear convolution */
  for (y = 0; y < tmpheight; y++) {
    for (x = 0; x < *width; x++) {
      texel = ((uint32*)*src)[y * ratio * *width + x];
      A = (double)(texel >> 24) * weight[0];
      R = (double)((texel >> 16) & 0xff) * weight[0];
      G = (double)((texel >>  8) & 0xff) * weight[0];
      B = (double)((texel      ) & 0xff) * weight[0];
      for (y2 = 1; y2 < half_window * ratio; y2++) {
        z = y * ratio + y2;
        if (z >= *height) z = *height - 1;
        texel = ((uint32*)*src)[z * *width + x];
        A += (double)(texel >> 24) * weight[y2];
        R += (double)((texel >> 16) & 0xff) * weight[y2];
        G += (double)((texel >>  8) & 0xff) * weight[y2];
        B += (double)((texel      ) & 0xff) * weight[y2];
        z = y * ratio - y2;
        if (z < 0) z = 0;
        texel = ((uint32*)*src)[z * *width + x];
        A += (double)(texel >> 24) * weight[y2];
        R += (double)((texel >> 16) & 0xff) * weight[y2];
        G += (double)((texel >>  8) & 0xff) * weight[y2];
        B += (double)((texel      ) & 0xff) * weight[y2];
      }
      if (A < 0) A = 0; else if (A > 255) A = 255;
      if (R < 0) R = 0; else if (R > 255) R = 255;
      if (G < 0) G = 0; else if (G > 255) G = 255;
      if (B < 0) B = 0; else if (B > 255) B = 255;
      ((uint32*)workbuf)[x] = (((uint32)A << 24) | ((uint32)R << 16) | ((uint32)G << 8) | (uint32)B);
    }
    for (x = 0; x < tmpwidth; x++) {
      texel = ((uint32*)workbuf)[x * ratio];
      A = (double)(texel >> 24) * weight[0];
      R = (double)((texel >> 16) & 0xff) * weight[0];
      G = (double)((texel >>  8) & 0xff) * weight[0];
      B = (double)((texel      ) & 0xff) * weight[0];
      for (x2 = 1; x2 < half_window * ratio; x2++) {
        z = x * ratio + x2;
        if (z >= *width) z = *width - 1;
        texel = ((uint32*)workbuf)[z];
        A += (double)(texel >> 24) * weight[x2];
        R += (double)((texel >> 16) & 0xff) * weight[x2];
        G += (double)((texel >>  8) & 0xff) * weight[x2];
        B += (double)((texel      ) & 0xff) * weight[x2];
        z = x * ratio - x2;
        if (z < 0) z = 0;
        texel = ((uint32*)workbuf)[z];
        A += (double)(texel >> 24) * weight[x2];
        R += (double)((texel >> 16) & 0xff) * weight[x2];
        G += (double)((texel >>  8) & 0xff) * weight[x2];
        B += (double)((texel      ) & 0xff) * weight[x2];
      }
      if (A < 0) A = 0; else if (A > 255) A = 255;
      if (R < 0) R = 0; else if (R > 255) R = 255;
      if (G < 0) G = 0; else if (G > 255) G = 255;
      if (B < 0) B = 0; else if (B > 255) B = 255;
      ((uint32*)tmptex)[y * tmpwidth + x] = (((uint32)A << 24) | ((uint32)R << 16) | ((uint32)G << 8) | (uint32)B);
    }
  }

  free(*src);
  *src = tmptex;
  free(weight);
  free(workbuf);
  *width = tmpwidth;
  *height = tmpheight;

  DBG_INFO(80, L"minification ratio:%d -> %d x %d\n", ratio, *width, *height);

  return 1;
#endif
}
