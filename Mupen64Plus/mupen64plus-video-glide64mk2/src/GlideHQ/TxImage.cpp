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

/* use power of 2 texture size
 * (0:disable, 1:enable, 2:3dfx) */
#define POW2_TEXTURES 0

/* check 8 bytes. use a larger value if needed. */
#define PNG_CHK_BYTES 8

#include "TxImage.h"
#include "TxReSample.h"
#include "TxDbg.h"
#include <stdlib.h>

boolean
TxImage::getPNGInfo(FILE *fp, png_structp *png_ptr, png_infop *info_ptr)
{
  unsigned char sig[PNG_CHK_BYTES];

  /* check for valid file pointer */
  if (!fp)
    return 0;

  /* check if file is PNG */
  if (fread(sig, 1, PNG_CHK_BYTES, fp) != PNG_CHK_BYTES)
    return 0;

  if (png_sig_cmp(sig, 0, PNG_CHK_BYTES) != 0)
    return 0;

  /* get PNG file info */
  *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!*png_ptr)
    return 0;

  *info_ptr = png_create_info_struct(*png_ptr);
  if (!*info_ptr) {
    png_destroy_read_struct(png_ptr, NULL, NULL);
    return 0;
  }

  if (setjmp(png_jmpbuf(*png_ptr))) {
    DBG_INFO(80, L"error reading png!\n");
    png_destroy_read_struct(png_ptr, info_ptr, NULL);
    return 0;
  }

  png_init_io(*png_ptr, fp);
  png_set_sig_bytes(*png_ptr, PNG_CHK_BYTES);
  png_read_info(*png_ptr, *info_ptr);

  return 1;
}

uint8*
TxImage::readPNG(FILE* fp, int* width, int* height, uint16* format)
{
  /* NOTE: returned image format is GR_TEXFMT_ARGB_8888 */

  png_structp png_ptr;
  png_infop info_ptr;
  uint8 *image = NULL;
  int bit_depth, color_type, interlace_type, compression_type, filter_type,
      row_bytes, o_width, o_height, num_pas;

  /* initialize */
  *width  = 0;
  *height = 0;
  *format = 0;

  /* check if we have a valid png file */
  if (!fp)
    return NULL;

  if (!getPNGInfo(fp, &png_ptr, &info_ptr)) {
    INFO(80, L"error reading png file! png image is corrupt.\n");
    return NULL;
  }

  png_get_IHDR(png_ptr, info_ptr,
               (png_uint_32*)&o_width, (png_uint_32*)&o_height, &bit_depth, &color_type,
               &interlace_type, &compression_type, &filter_type);

  DBG_INFO(80, L"png format %d x %d bitdepth:%d color:%x interlace:%x compression:%x filter:%x\n",
           o_width, o_height, bit_depth, color_type,
           interlace_type, compression_type, filter_type);

  /* transformations */

  /* Rice hi-res textures
   * _all.png
   * _rgb.png, _a.png
   * _ciByRGBA.png
   * _allciByRGBA.png
   */

  /* strip if color channel is larger than 8 bits */
  if (bit_depth > 8) {
    png_set_strip_16(png_ptr);
    bit_depth = 8;
  }

#if 1
  /* These are not really required per Rice format spec,
   * but is done just in case someone uses them.
   */
  /* convert palette color to rgb color */
  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png_ptr);
    color_type = PNG_COLOR_TYPE_RGB;
  }

  /* expand 1,2,4 bit gray scale to 8 bit gray scale */
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png_ptr);

  /* convert gray scale or gray scale + alpha to rgb color */
  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    png_set_gray_to_rgb(png_ptr);
    color_type = PNG_COLOR_TYPE_RGB;
  }
#endif

  /* add alpha channel if any */
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png_ptr);
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
  }

  /* convert rgb to rgba */
  if (color_type == PNG_COLOR_TYPE_RGB) {
    png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
  }

  /* punt invalid formats */
  if (color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    DBG_INFO(80, L"Error: not PNG_COLOR_TYPE_RGB_ALPHA format!\n");
    return NULL;
  }

  /*png_color_8p sig_bit;
  if (png_get_sBIT(png_ptr, info_ptr, &sig_bit))
    png_set_shift(png_ptr, sig_bit);*/

  /* convert rgba to bgra */
  png_set_bgr(png_ptr);

  /* turn on interlace handling to cope with the weirdness
   * of texture authors using interlaced format */
  num_pas = png_set_interlace_handling(png_ptr);

  /* update info structure */
  png_read_update_info(png_ptr, info_ptr);

  /* we only get here if ARGB8888 */
  row_bytes = png_get_rowbytes(png_ptr, info_ptr);

  /* allocate memory to read in image */
  image = (uint8*)malloc(row_bytes * o_height);

  /* read in image */
  if (image) {
    int pas, i;
    uint8* tmpimage;

    for (pas = 0; pas < num_pas; pas++) { /* deal with interlacing */
      tmpimage = image;

      for (i = 0; i < o_height; i++) {
        /* copy row */
        png_read_rows(png_ptr, &tmpimage, NULL, 1);
        tmpimage += row_bytes;
      }
    }

    /* read rest of the info structure */
    png_read_end(png_ptr, info_ptr);

    *width = (row_bytes >> 2);
    *height = o_height;
    *format = GR_TEXFMT_ARGB_8888;

#if POW2_TEXTURES
    /* next power of 2 size conversions */
    /* NOTE: I can do this in the above loop for faster operations, but some
     * texture packs require a workaround. see HACKALERT in nextPow2().
     */

    TxReSample txReSample = new TxReSample; // XXX: temporary. move to a better place.

#if (POW2_TEXTURES == 2)
    if (!txReSample->nextPow2(&image, width, height, 32, 1)) {
#else
    if (!txReSample->nextPow2(&image, width, height, 32, 0)) {
#endif
      if (image) {
        free(image);
        image = NULL;
      }
      *width = 0;
      *height = 0;
      *format = 0;
    }

    delete txReSample;

#endif /* POW2_TEXTURES */
  }

  /* clean up */
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

#ifdef DEBUG
  if (!image) {
    DBG_INFO(80, L"Error: failed to load png image!\n");
  }
#endif

  return image;
}

boolean
TxImage::writePNG(uint8* src, FILE* fp, int width, int height, int rowStride, uint16 format, uint8 *palette)
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_color_8 sig_bit;
  png_colorp palette_ptr;
  png_bytep trans_ptr;//, tex_ptr;
  int bit_depth, color_type, row_bytes, num_palette;
  int i;
  //uint16 srcfmt, destfmt;

  if (!src || !fp)
    return 0;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL)
    return 0;

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    png_destroy_write_struct(&png_ptr, NULL);
    return 0;
  }

  if (png_jmpbuf(png_ptr)) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 0;
  }

  png_init_io(png_ptr, fp);

  /* TODO: images must be converted to RGBA8888 or CI8,
   * palettes need to be separated to A and RGB. */

  /* N64 formats
   * Format: 0 - RGBA, 1 - YUV, 2 - CI, 3 - IA, 4 - I
   * Size:   0 - 4bit, 1 - 8bit, 2 - 16bit, 3 - 32 bit
   * format = (Format << 8 | Size);
   */

  /* each channel is saved in 8bits for consistency */
  switch (format) {
  case 0x0002:/* RGBA5551 */
    bit_depth = 8;
    sig_bit.red   = 5;
    sig_bit.green = 5;
    sig_bit.blue  = 5;
    sig_bit.alpha = 1;
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    break;
  case 0x0003:/* RGBA8888 */
  case 0x0302:/* IA88 */
    bit_depth = 8;
    sig_bit.red   = 8;
    sig_bit.green = 8;
    sig_bit.blue  = 8;
    sig_bit.alpha = 8;
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    break;
  case 0x0300:/* IA31 */
    bit_depth = 8;
    sig_bit.red   = 3;
    sig_bit.green = 3;
    sig_bit.blue  = 3;
    sig_bit.alpha = 1;
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    break;
  case 0x0301:/* IA44 */
    bit_depth = 8;
    sig_bit.red   = 4;
    sig_bit.green = 4;
    sig_bit.blue  = 4;
    sig_bit.alpha = 4;
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    break;
  case 0x0400:/* I4 */
    bit_depth = 8;
    sig_bit.red   = 4;
    sig_bit.green = 4;
    sig_bit.blue  = 4;
    color_type = PNG_COLOR_TYPE_RGB;
    break;
  case 0x0401:/* I8 */
  case 0x0402:/* I16 */
    bit_depth = 8;
    sig_bit.red   = 8;
    sig_bit.green = 8;
    sig_bit.blue  = 8;
    color_type = PNG_COLOR_TYPE_RGB;
    break;
  case 0x0200:/* CI4 */
    bit_depth = 8;
    num_palette = 16;
    color_type = PNG_COLOR_TYPE_PALETTE;
    break;
  case 0x0201:/* CI8 */
    bit_depth = 8;
    num_palette = 256;
    color_type = PNG_COLOR_TYPE_PALETTE;
    break;
  case 0x0102:/* YUV ? */
  case 0x0103:
  default:
    /* unsupported format */
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return 0;
  }

  switch (color_type) {
  case PNG_COLOR_TYPE_RGB_ALPHA:
  case PNG_COLOR_TYPE_RGB:
    //row_bytes = (bit_depth * width) >> 1;
    row_bytes = rowStride;
    png_set_bgr(png_ptr);
    png_set_sBIT(png_ptr, info_ptr, &sig_bit);
    break;
  case PNG_COLOR_TYPE_PALETTE:
    //row_bytes = (bit_depth * width) >> 3;
    row_bytes = rowStride;
    png_set_PLTE(png_ptr, info_ptr, palette_ptr, num_palette);
    png_set_tRNS(png_ptr, info_ptr, trans_ptr, num_palette, 0);
  }

  //png_set_filter(png_ptr, 0, PNG_ALL_FILTERS);

  //if (bit_depth == 16)
  //  png_set_swap(png_ptr);

  //if (bit_depth < 8)
  //  png_set_packswap(png_ptr);

  png_set_IHDR(png_ptr, info_ptr, width, height,
               bit_depth, color_type, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  //png_set_gAMA(png_ptr, info_ptr, 1.0);

  png_write_info(png_ptr, info_ptr);
  for (i = 0; i < height; i++) {
    png_write_row(png_ptr, (png_bytep)src);
    src += row_bytes;
  }
  png_write_end(png_ptr, info_ptr);

  png_destroy_write_struct(&png_ptr, &info_ptr);

  //if (tex_ptr) delete [] tex_ptr;

  return 1;
}

boolean
TxImage::getBMPInfo(FILE* fp, BITMAPFILEHEADER* bmp_fhdr, BITMAPINFOHEADER* bmp_ihdr)
{
  /*
   * read in BITMAPFILEHEADER
   */

  /* is this a BMP file? */
  if (fread(&bmp_fhdr->bfType, 2, 1, fp) != 1)
    return 0;

  if (memcmp(&bmp_fhdr->bfType, "BM", 2) != 0)
    return 0;

  /* get file size */
  if (fread(&bmp_fhdr->bfSize, 4, 1, fp) != 1)
    return 0;

  /* reserved 1 */
  if (fread(&bmp_fhdr->bfReserved1, 2, 1, fp) != 1)
    return 0;

  /* reserved 2 */
  if (fread(&bmp_fhdr->bfReserved2, 2, 1, fp) != 1)
    return 0;

  /* offset to the image data */
  if (fread(&bmp_fhdr->bfOffBits, 4, 1, fp) != 1)
    return 0;

  /*
   * read in BITMAPINFOHEADER
   */

  /* size of BITMAPINFOHEADER */
  if (fread(&bmp_ihdr->biSize, 4, 1, fp) != 1)
    return 0;

  /* is this a Windows BMP? */
  if (bmp_ihdr->biSize != 40)
    return 0;

  /* width of the bitmap in pixels */
  if (fread(&bmp_ihdr->biWidth, 4, 1, fp) != 1)
    return 0;

  /* height of the bitmap in pixels */
  if (fread(&bmp_ihdr->biHeight, 4, 1, fp) != 1)
    return 0;

  /* number of planes (always 1) */
  if (fread(&bmp_ihdr->biPlanes, 2, 1, fp) != 1)
    return 0;

  /* number of bits-per-pixel. (1, 4, 8, 16, 24, 32) */
  if (fread(&bmp_ihdr->biBitCount, 2, 1, fp) != 1)
    return 0;

  /* compression for a compressed bottom-up bitmap
   *   0 : uncompressed format
   *   1 : run-length encoded 4 bpp format
   *   2 : run-length encoded 8 bpp format
   *   3 : bitfield
   */
  if (fread(&bmp_ihdr->biCompression, 4, 1, fp) != 1)
    return 0;

  /* size of the image in bytes */
  if (fread(&bmp_ihdr->biSizeImage, 4, 1, fp) != 1)
    return 0;

  /* horizontal resolution in pixels-per-meter */
  if (fread(&bmp_ihdr->biXPelsPerMeter, 4, 1, fp) != 1)
    return 0;

  /* vertical resolution in pixels-per-meter */
  if (fread(&bmp_ihdr->biYPelsPerMeter, 4, 1, fp) != 1)
    return 0;

  /* number of color indexes in the color table that are actually used */
  if (fread(&bmp_ihdr->biClrUsed, 4, 1, fp) != 1)
    return 0;

  /*  the number of color indexes that are required for displaying */
  if (fread(&bmp_ihdr->biClrImportant, 4, 1, fp) != 1)
    return 0;

  return 1;
}

uint8*
TxImage::readBMP(FILE* fp, int* width, int* height, uint16* format)
{
  /* NOTE: returned image format;
   *       4, 8bit palette bmp -> GR_TEXFMT_P_8
   *       24, 32bit bmp -> GR_TEXFMT_ARGB_8888
   */

  uint8 *image = NULL;
  uint8 *image_row = NULL;
  uint8 *tmpimage = NULL;
  int row_bytes, pos, i, j;
  /* Windows Bitmap */
  BITMAPFILEHEADER bmp_fhdr;
  BITMAPINFOHEADER bmp_ihdr;

  /* initialize */
  *width  = 0;
  *height = 0;
  *format = 0;

  /* check if we have a valid bmp file */
  if (!fp)
    return NULL;

  if (!getBMPInfo(fp, &bmp_fhdr, &bmp_ihdr)) {
    INFO(80, L"error reading bitmap file! bitmap image is corrupt.\n");
    return NULL;
  }

  DBG_INFO(80, L"bmp format %d x %d bitdepth:%d compression:%x offset:%d\n",
           bmp_ihdr.biWidth, bmp_ihdr.biHeight, bmp_ihdr.biBitCount,
           bmp_ihdr.biCompression, bmp_fhdr.bfOffBits);

  /* rowStride in bytes */
  row_bytes = (bmp_ihdr.biWidth * bmp_ihdr.biBitCount) >> 3;
  /* align to 4bytes boundary */
  row_bytes = (row_bytes + 3) & ~3;

  /* Rice hi-res textures */
  if (!(bmp_ihdr.biBitCount == 8 || bmp_ihdr.biBitCount == 4 || bmp_ihdr.biBitCount == 32 || bmp_ihdr.biBitCount == 24) ||
      bmp_ihdr.biCompression != 0) {
    DBG_INFO(80, L"Error: incompatible bitmap format!\n");
    return NULL;
  }

  switch (bmp_ihdr.biBitCount) {
  case 8:
  case 32:
    /* 8 bit, 32 bit bitmap */
    image = (uint8*)malloc(row_bytes * bmp_ihdr.biHeight);
    if (image) {
      tmpimage = image;
      pos = bmp_fhdr.bfOffBits + row_bytes * (bmp_ihdr.biHeight - 1);
      for (i = 0; i < bmp_ihdr.biHeight; i++) {
        /* read in image */
        fseek(fp, pos, SEEK_SET);
        fread(tmpimage, row_bytes, 1, fp);
        tmpimage += row_bytes;
        pos -= row_bytes;
      }
    }
    break;
  case 4:
    /* 4bit bitmap */
    image = (uint8*)malloc((row_bytes * bmp_ihdr.biHeight) << 1);
    image_row = (uint8*)malloc(row_bytes);
    if (image && image_row) {
      tmpimage = image;
      pos = bmp_fhdr.bfOffBits + row_bytes * (bmp_ihdr.biHeight - 1);
      for (i = 0; i < bmp_ihdr.biHeight; i++) {
        /* read in image */
        fseek(fp, pos, SEEK_SET);
        fread(image_row, row_bytes, 1, fp);
        /* expand 4bpp to 8bpp. stuff 4bit values into 8bit comps. */
        for (j = 0; j < row_bytes; j++) {
          tmpimage[j << 1] = image_row[j] & 0x0f;
          tmpimage[(j << 1) + 1] = (image_row[j] & 0xf0) >> 4;
        }
        tmpimage += (row_bytes << 1);
        pos -= row_bytes;
      }
      free(image_row);
    } else {
      if (image_row) free(image_row);
      if (image) free(image);
      image = NULL;
    }
    break;
  case 24:
    /* 24 bit bitmap */
    image = (uint8*)malloc((bmp_ihdr.biWidth * bmp_ihdr.biHeight) << 2);
    image_row = (uint8*)malloc(row_bytes);
    if (image && image_row) {
      tmpimage = image;
      pos = bmp_fhdr.bfOffBits + row_bytes * (bmp_ihdr.biHeight - 1);
      for (i = 0; i < bmp_ihdr.biHeight; i++) {
        /* read in image */
        fseek(fp, pos, SEEK_SET);
        fread(image_row, row_bytes, 1, fp);
        /* convert 24bpp to 32bpp. */
        for (j = 0; j < bmp_ihdr.biWidth; j++) {
          tmpimage[(j << 2)]     = image_row[j * 3];
          tmpimage[(j << 2) + 1] = image_row[j * 3 + 1];
          tmpimage[(j << 2) + 2] = image_row[j * 3 + 2];
          tmpimage[(j << 2) + 3] = 0xFF;
        }
        tmpimage += (bmp_ihdr.biWidth << 2);
        pos -= row_bytes;
      }
      free(image_row);
    } else {
      if (image_row) free(image_row);
      if (image) free(image);
      image = NULL;
    }
  }

  if (image) {
    *width = (row_bytes << 3) / bmp_ihdr.biBitCount;
    *height = bmp_ihdr.biHeight;

    switch (bmp_ihdr.biBitCount) {
    case 8:
    case 4:
      *format = GR_TEXFMT_P_8;
      break;
    case 32:
    case 24:
      *format = GR_TEXFMT_ARGB_8888;
    }

#if POW2_TEXTURES
    /* next power of 2 size conversions */
    /* NOTE: I can do this in the above loop for faster operations, but some
     * texture packs require a workaround. see HACKALERT in nextPow2().
     */

    TxReSample txReSample = new TxReSample; // XXX: temporary. move to a better place.

#if (POW2_TEXTURES == 2)
    if (!txReSample->nextPow2(&image, width, height, 8, 1)) {
#else
    if (!txReSample->nextPow2(&image, width, height, 8, 0)) {
#endif
      if (image) {
        free(image);
        image = NULL;
      }
      *width = 0;
      *height = 0;
      *format = 0;
    }

    delete txReSample;

#endif /* POW2_TEXTURES */
  }

#ifdef DEBUG
  if (!image) {
    DBG_INFO(80, L"Error: failed to load bmp image!\n");
  }
#endif

  return image;
}

boolean
TxImage::getDDSInfo(FILE *fp, DDSFILEHEADER *dds_fhdr)
{
  /*
   * read in DDSFILEHEADER
   */

  /* is this a DDS file? */
  if (fread(&dds_fhdr->dwMagic, 4, 1, fp) != 1)
    return 0;

  if (memcmp(&dds_fhdr->dwMagic, "DDS ", 4) != 0)
    return 0;

  if (fread(&dds_fhdr->dwSize, 4, 1, fp) != 1)
    return 0;

  /* get file flags */
  if (fread(&dds_fhdr->dwFlags, 4, 1, fp) != 1)
    return 0;

  /* height of dds in pixels */
  if (fread(&dds_fhdr->dwHeight, 4, 1, fp) != 1)
    return 0;

  /* width of dds in pixels */
  if (fread(&dds_fhdr->dwWidth, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->dwLinearSize, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->dwDepth, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->dwMipMapCount, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->dwReserved1, 4 * 11, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->ddpf.dwSize, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->ddpf.dwFlags, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->ddpf.dwFourCC, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->ddpf.dwRGBBitCount, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->ddpf.dwRBitMask, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->ddpf.dwGBitMask, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->ddpf.dwBBitMask, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->ddpf.dwRGBAlphaBitMask, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->dwCaps1, 4, 1, fp) != 1)
    return 0;

  if (fread(&dds_fhdr->dwCaps2, 4, 1, fp) != 1)
    return 0;

  return 1;
}

uint8*
TxImage::readDDS(FILE* fp, int* width, int* height, uint16* format)
{
  uint8 *image = NULL;
  DDSFILEHEADER dds_fhdr;
  uint16 tmpformat = 0;

  /* initialize */
  *width  = 0;
  *height = 0;
  *format = 0;

  /* check if we have a valid dds file */
  if (!fp)
    return NULL;

  if (!getDDSInfo(fp, &dds_fhdr)) {
    INFO(80, L"error reading dds file! dds image is corrupt.\n");
    return NULL;
  }

  DBG_INFO(80, L"dds format %d x %d HeaderSize %d LinearSize %d\n",
           dds_fhdr.dwWidth, dds_fhdr.dwHeight, dds_fhdr.dwSize, dds_fhdr.dwLinearSize);

  if (!(dds_fhdr.dwFlags & (DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT|DDSD_LINEARSIZE))) {
    DBG_INFO(80, L"Error: incompatible dds format!\n");
    return NULL;
  }

  if ((dds_fhdr.dwFlags & DDSD_MIPMAPCOUNT) && dds_fhdr.dwMipMapCount != 1) {
    DBG_INFO(80, L"Error: mipmapped dds not supported!\n");
    return NULL;
  }

  if (!((dds_fhdr.ddpf.dwFlags & DDPF_FOURCC) && dds_fhdr.dwCaps2 == 0)) {
    DBG_INFO(80, L"Error: not fourcc standard texture!\n");
    return NULL;
  }

  if (memcmp(&dds_fhdr.ddpf.dwFourCC, "DXT1", 4) == 0) {
    DBG_INFO(80, L"DXT1 format\n");
    /* compensate for missing LinearSize */
    dds_fhdr.dwLinearSize = (dds_fhdr.dwWidth * dds_fhdr.dwHeight) >> 1;
    tmpformat = GR_TEXFMT_ARGB_CMP_DXT1;
  } else if (memcmp(&dds_fhdr.ddpf.dwFourCC, "DXT3", 4) == 0) {
    DBG_INFO(80, L"DXT3 format\n");
    dds_fhdr.dwLinearSize = dds_fhdr.dwWidth * dds_fhdr.dwHeight;
    tmpformat = GR_TEXFMT_ARGB_CMP_DXT3;
  } else if (memcmp(&dds_fhdr.ddpf.dwFourCC, "DXT5", 4) == 0) {
    DBG_INFO(80, L"DXT5 format\n");
    dds_fhdr.dwLinearSize = dds_fhdr.dwWidth * dds_fhdr.dwHeight;
    tmpformat = GR_TEXFMT_ARGB_CMP_DXT5;
  } else {
    DBG_INFO(80, L"Error: not DXT1 or DXT3 or DXT5 format!\n");
    return NULL;
  }

  /* read in image */
  image = (uint8*)malloc(dds_fhdr.dwLinearSize);
  if (image) {
    *width  = dds_fhdr.dwWidth;
    *height = dds_fhdr.dwHeight;
    *format = tmpformat;

    fseek(fp, 128, SEEK_SET); /* size of header is 128 bytes */
    fread(image, dds_fhdr.dwLinearSize, 1, fp);
  }

  return image;
}
