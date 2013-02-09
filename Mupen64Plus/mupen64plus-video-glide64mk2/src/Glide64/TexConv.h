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

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators
// Project started on December 29th, 2001
//
// Authors:
// Dave2001, original author, founded the project in 2001, left it in 2002
// Gugaman, joined the project in 2002, left it in 2002
// Sergey 'Gonetz' Lipski, joined the project in 2002, main author since fall of 2002
// Hiroshi 'KoolSmoky' Morii, joined the project in 2007
//
//****************************************************************
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
//****************************************************************

static inline void texConv_ARGB1555_ARGB4444(uint8_t *src, uint8_t *dst, int size)
{
  uint32_t *v3;
  uint32_t *v4;
  int v5;
  uint32_t v6;
  uint32_t v7;

  v3 = (uint32_t *)src;
  v4 = (uint32_t *)dst;
  v5 = size;
  do
  {
    v6 = *v3;
    ++v3;
    v7 = v6;
    *v4 = ((v7 & 0x1E001E) >> 1) | ((v6 & 0x3C003C0) >> 2) | ((v6 & 0x78007800) >> 3) | ((v6 & 0x80008000) >> 3) | ((v6 & 0x80008000) >> 2) | ((v6 & 0x80008000) >> 1) | v6 & 0x80008000;
    ++v4;
    --v5;
  }
  while ( v5 );
}

static inline void texConv_AI88_ARGB4444(uint8_t *src, uint8_t *dst, int size)
{
  uint32_t *v3;
  uint32_t *v4;
  int v5;
  uint32_t v6;
  uint32_t v7;

  v3 = (uint32_t *)src;
  v4 = (uint32_t *)dst;
  v5 = size;
  do
  {
    v6 = *v3;
    ++v3;
    v7 = v6;
    *v4 = (16 * (v7 & 0xF000F0) >> 8) | v7 & 0xF000F0 | 16 * (v7 & 0xF000F0) | v6 & 0xF000F000;
    ++v4;
    --v5;
  }
  while ( v5 );
}

static inline void texConv_AI44_ARGB4444(uint8_t *src, uint8_t *dst, int size)
{
  uint32_t *v3;
  uint32_t *v4;
  int v5;
  uint32_t v6;
  uint32_t *v7;

  v3 = (uint32_t *)src;
  v4 = (uint32_t *)dst;
  v5 = size;
  do
  {
    v6 = *v3;
    ++v3;
    *v4 = ((((uint16_t)v6 << 8) & 0xFF00 & 0xF00u) >> 8) | ((((uint16_t)v6 << 8) & 0xFF00 & 0xF00u) >> 4) | (uint16_t)(((uint16_t)v6 << 8) & 0xFF00) | (((v6 << 16) & 0xF000000) >> 8) | (((v6 << 16) & 0xF000000) >> 4) | (v6 << 16) & 0xFF000000;
    v7 = v4 + 1;
    *v7 = (((v6 >> 8) & 0xF00) >> 8) | (((v6 >> 8) & 0xF00) >> 4) | (v6 >> 8) & 0xFF00 | ((v6 & 0xF000000) >> 8) | ((v6 & 0xF000000) >> 4) | v6 & 0xFF000000;
    v4 = v7 + 1;
    --v5;
  }
  while ( v5 );
}

static inline void texConv_A8_ARGB4444(uint8_t *src, uint8_t *dst, int size)
{
  uint32_t *v3;
  uint32_t *v4;
  int v5;
  uint32_t v6;
  uint32_t v7;
  uint32_t *v8;

  v3 = (uint32_t *)src;
  v4 = (uint32_t *)dst;
  v5 = size;
  do
  {
    v6 = *v3;
    ++v3;
    v7 = v6;
    *v4 = ((v6 & 0xF0) << 8 >> 12) | (uint8_t)(v6 & 0xF0) | 16 * (uint8_t)(v6 & 0xF0) & 0xFFFFFFF | ((uint8_t)(v6 & 0xF0) << 8) | 16 * (uint16_t)(v6 & 0xF000) & 0xFFFFF | ((uint16_t)(v6 & 0xF000) << 8) & 0xFFFFFF | ((uint16_t)(v6 & 0xF000) << 12) & 0xFFFFFFF | ((uint16_t)(v6 & 0xF000) << 16);
    v8 = v4 + 1;
    *v8 = ((v7 & 0xF00000) >> 20) | ((v7 & 0xF00000) >> 16) | ((v7 & 0xF00000) >> 12) | ((v7 & 0xF00000) >> 8) | ((v6 & 0xF0000000) >> 12) | ((v6 & 0xF0000000) >> 8) | ((v6 & 0xF0000000) >> 4) | v6 & 0xF0000000;
    v4 = v8 + 1;
    --v5;
  }
  while ( v5 );
}

void TexConv_ARGB1555_ARGB4444 (unsigned char * src, unsigned char * dst, int width, int height)
{
  int size = (width * height) >> 1;	// Hiroshi Morii <koolsmoky@users.sourceforge.net>
  // 2 pixels are converted in one loop
  // NOTE: width * height must be a multiple of 2
  texConv_ARGB1555_ARGB4444(src, dst, size);
}

void TexConv_AI88_ARGB4444 (unsigned char * src, unsigned char * dst, int width, int height)
{
  int size = (width * height) >> 1;	// Hiroshi Morii <koolsmoky@users.sourceforge.net>
  // 2 pixels are converted in one loop
  // NOTE: width * height must be a multiple of 2
  texConv_AI88_ARGB4444(src, dst, size);
}

void TexConv_AI44_ARGB4444 (unsigned char * src, unsigned char * dst, int width, int height)
{
  int size = (width * height) >> 2;	// Hiroshi Morii <koolsmoky@users.sourceforge.net>
  // 4 pixels are converted in one loop
  // NOTE: width * height must be a multiple of 4
  texConv_AI44_ARGB4444(src, dst, size);
}

void TexConv_A8_ARGB4444 (unsigned char * src, unsigned char * dst, int width, int height)
{
  int size = (width * height) >> 2;	// Hiroshi Morii <koolsmoky@users.sourceforge.net>
  // 4 pixels are converted in one loop
  // NOTE: width * height must be a multiple of 4
  texConv_A8_ARGB4444(src, dst, size);
}

