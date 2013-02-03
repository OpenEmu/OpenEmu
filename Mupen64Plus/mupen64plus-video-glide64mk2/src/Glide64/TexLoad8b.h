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
#include <stdint.h>

extern "C" void asmLoad8bCI(uint8_t *src, uint8_t *dst, int wid_64, int height, int line, int ext, uint16_t *pal);
extern "C" void asmLoad8bIA8(uint8_t *src, uint8_t *dst, int wid_64, int height, int line, int ext, uint16_t *pal);
extern "C" void asmLoad8bIA4(uint8_t *src, uint8_t *dst, int wid_64, int height, int line, int ext);
extern "C" void asmLoad8bI(uint8_t *src, uint8_t *dst, int wid_64, int height, int line, int ext);

static inline void load8bCI(uint8_t *src, uint8_t *dst, int wid_64, int height, int line, int ext, uint16_t *pal)
{
  uint8_t *v7;
  uint32_t *v8;
  int v9;
  int v10;
  int v11;
  uint32_t v12;
  uint32_t *v13;
  uint32_t v14;
  uint32_t *v15;
  uint32_t v16;
  uint32_t *v17;
  uint32_t *v18;
  int v19;
  int v20;
  uint32_t v21;
  uint32_t v22;
  uint32_t *v23;
  uint32_t v24;
  int v25;
  int v26;

  v7 = src;
  v8 = (uint32_t *)dst;
  v9 = height;
  do
  {
    v25 = v9;
    v10 = wid_64;
    do
    {
      v11 = v10;
      v12 = bswap32(*(uint32_t *)v7);
      v13 = (uint32_t *)(v7 + 4);
      ALOWORD(v10) = __ROR__(*(uint16_t *)((char *)pal + ((v12 >> 15) & 0x1FE)), 1);
      v14 = v10 << 16;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + ((v12 >> 23) & 0x1FE)), 1);
      *v8 = v14;
      v15 = v8 + 1;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + (2 * (uint16_t)v12 & 0x1FE)), 1);
      v14 <<= 16;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + ((v12 >> 7) & 0x1FE)), 1);
      *v15 = v14;
      ++v15;
      v16 = bswap32(*v13);
      v7 = (uint8_t *)(v13 + 1);
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + ((v16 >> 15) & 0x1FE)), 1);
      v14 <<= 16;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + ((v16 >> 23) & 0x1FE)), 1);
      *v15 = v14;
      ++v15;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + (2 * (uint16_t)v16 & 0x1FE)), 1);
      v14 <<= 16;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + ((v16 >> 7) & 0x1FE)), 1);
      *v15 = v14;
      v8 = v15 + 1;
      v10 = v11 - 1;
    }
    while ( v11 != 1 );
    if ( v25 == 1 )
      break;
    v26 = v25 - 1;
    v17 = (uint32_t *)&src[(line + (uintptr_t)v7 - (uintptr_t)src) & 0x7FF];
    v18 = (uint32_t *)((char *)v8 + ext);
    v19 = wid_64;
    do
    {
      v20 = v19;
      v21 = bswap32(v17[1]);
      ALOWORD(v19) = __ROR__(*(uint16_t *)((char *)pal + ((v21 >> 15) & 0x1FE)), 1);
      v22 = v19 << 16;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + ((v21 >> 23) & 0x1FE)), 1);
      *v18 = v22;
      v23 = v18 + 1;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + (2 * (uint16_t)v21 & 0x1FE)), 1);
      v22 <<= 16;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + ((v21 >> 7) & 0x1FE)), 1);
      *v23 = v22;
      ++v23;
      v24 = bswap32(*v17);
      v17 = (uint32_t *)&src[((uintptr_t)v17 + 8 - (uintptr_t)src) & 0x7FF];
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + ((v24 >> 15) & 0x1FE)), 1);
      v22 <<= 16;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + ((v24 >> 23) & 0x1FE)), 1);
      *v23 = v22;
      ++v23;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + (2 * (uint16_t)v24 & 0x1FE)), 1);
      v22 <<= 16;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + ((v24 >> 7) & 0x1FE)), 1);
      *v23 = v22;
      v18 = v23 + 1;
      v19 = v20 - 1;
    }
    while ( v20 != 1 );
    v7 = &src[(line + (uintptr_t)v17 - (uintptr_t)src) & 0x7FF];
    v8 = (uint32_t *)((char *)v18 + ext);
    v9 = v26 - 1;
  }
  while ( v26 != 1 );
}

static inline void load8bIA8(uint8_t *src, uint8_t *dst, int wid_64, int height, int line, int ext, uint16_t *pal)
{
  uint32_t *v7;
  uint32_t *v8;
  int v9;
  int v10;
  int v11;
  uint32_t v12;
  uint32_t *v13;
  uint32_t v14;
  uint32_t *v15;
  uint32_t v16;
  uint32_t *v17;
  uint32_t *v18;
  int v19;
  int v20;
  uint32_t v21;
  uint32_t v22;
  uint32_t *v23;
  uint32_t v24;
  int v25;
  int v26;

  v7 = (uint32_t *)src;
  v8 = (uint32_t *)dst;
  v9 = height;
  do
  {
    v25 = v9;
    v10 = wid_64;
    do
    {
      v11 = v10;
      v12 = bswap32(*v7);
      v13 = v7 + 1;
      ALOWORD(v10) = __ROR__(*(uint16_t *)((char *)pal + ((v12 >> 15) & 0x1FE)), 8);
      v14 = v10 << 16;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + ((v12 >> 23) & 0x1FE)), 8);
      *v8 = v14;
      v15 = v8 + 1;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + (2 * (uint16_t)v12 & 0x1FE)), 8);
      v14 <<= 16;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + ((v12 >> 7) & 0x1FE)), 8);
      *v15 = v14;
      ++v15;
      v16 = bswap32(*v13);
      v7 = v13 + 1;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + ((v16 >> 15) & 0x1FE)), 8);
      v14 <<= 16;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + ((v16 >> 23) & 0x1FE)), 8);
      *v15 = v14;
      ++v15;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + (2 * (uint16_t)v16 & 0x1FE)), 8);
      v14 <<= 16;
      ALOWORD(v14) = __ROR__(*(uint16_t *)((char *)pal + ((v16 >> 7) & 0x1FE)), 8);
      *v15 = v14;
      v8 = v15 + 1;
      v10 = v11 - 1;
    }
    while ( v11 != 1 );
    if ( v25 == 1 )
      break;
    v26 = v25 - 1;
    v17 = (uint32_t *)((char *)v7 + line);
    v18 = (uint32_t *)((char *)v8 + ext);
    v19 = wid_64;
    do
    {
      v20 = v19;
      v21 = bswap32(v17[1]);
      ALOWORD(v19) = __ROR__(*(uint16_t *)((char *)pal + ((v21 >> 15) & 0x1FE)), 8);
      v22 = v19 << 16;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + ((v21 >> 23) & 0x1FE)), 8);
      *v18 = v22;
      v23 = v18 + 1;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + (2 * (uint16_t)v21 & 0x1FE)), 8);
      v22 <<= 16;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + ((v21 >> 7) & 0x1FE)), 8);
      *v23 = v22;
      ++v23;
      v24 = bswap32(*v17);
      v17 += 2;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + ((v24 >> 15) & 0x1FE)), 8);
      v22 <<= 16;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + ((v24 >> 23) & 0x1FE)), 8);
      *v23 = v22;
      ++v23;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + (2 * (uint16_t)v24 & 0x1FE)), 8);
      v22 <<= 16;
      ALOWORD(v22) = __ROR__(*(uint16_t *)((char *)pal + ((v24 >> 7) & 0x1FE)), 8);
      *v23 = v22;
      v18 = v23 + 1;
      v19 = v20 - 1;
    }
    while ( v20 != 1 );
    v7 = (uint32_t *)((char *)v17 + line);
    v8 = (uint32_t *)((char *)v18 + ext);
    v9 = v26 - 1;
  }
  while ( v26 != 1 );
}

static inline void load8bIA4(uint8_t *src, uint8_t *dst, int wid_64, int height, int line, int ext)
{
  uint32_t *v6;
  uint32_t *v7;
  int v8;
  int v9;
  uint32_t v10;
  uint32_t v11;
  uint32_t *v12;
  uint32_t *v13;
  uint32_t v14;
  uint32_t v15;
  uint32_t *v16;
  uint32_t *v17;
  int v18;
  uint32_t *v19;
  uint32_t v20;
  int v21;
  int v22;

  v6 = (uint32_t *)src;
  v7 = (uint32_t *)dst;
  v8 = height;
  do
  {
    v21 = v8;
    v9 = wid_64;
    do
    {
      v10 = *v6;
      v11 = (*v6 >> 4) & 0xF0F0F0F;
      v12 = v6 + 1;
      *v7 = 16 * v10 & 0xF0F0F0F0 | v11;
      v13 = v7 + 1;
      v14 = (*v12 >> 4) & 0xF0F0F0F;
      v15 = 16 * *v12 & 0xF0F0F0F0;
      v6 = v12 + 1;
      *v13 = v15 | v14;
      v7 = v13 + 1;
      --v9;
    }
    while ( v9 );
    if ( v21 == 1 )
      break;
    v22 = v21 - 1;
    v16 = (uint32_t *)((char *)v6 + line);
    v17 = (uint32_t *)((char *)v7 + ext);
    v18 = wid_64;
    do
    {
      *v17 = 16 * v16[1] & 0xF0F0F0F0 | (v16[1] >> 4) & 0xF0F0F0F;
      v19 = v17 + 1;
      v20 = *v16;
      v16 += 2;
      *v19 = 16 * v20 & 0xF0F0F0F0 | (v20 >> 4) & 0xF0F0F0F;
      v17 = v19 + 1;
      --v18;
    }
    while ( v18 );
    v6 = (uint32_t *)((char *)v16 + line);
    v7 = (uint32_t *)((char *)v17 + ext);
    v8 = v22 - 1;
  }
  while ( v22 != 1 );
}

static inline void load8bI(uint8_t *src, uint8_t *dst, int wid_64, int height, int line, int ext)
{
  uint32_t *v6;
  uint32_t *v7;
  int v8;
  int v9;
  uint32_t v10;
  uint32_t *v11;
  uint32_t *v12;
  uint32_t v13;
  uint32_t *v14;
  uint32_t *v15;
  int v16;
  uint32_t *v17;
  uint32_t v18;
  int v19;
  int v20;

  v6 = (uint32_t *)src;
  v7 = (uint32_t *)dst;
  v8 = height;
  do
  {
    v19 = v8;
    v9 = wid_64;
    do
    {
      v10 = *v6;
      v11 = v6 + 1;
      *v7 = v10;
      v12 = v7 + 1;
      v13 = *v11;
      v6 = v11 + 1;
      *v12 = v13;
      v7 = v12 + 1;
      --v9;
    }
    while ( v9 );
    if ( v19 == 1 )
      break;
    v20 = v19 - 1;
    v14 = (uint32_t *)((char *)v6 + line);
    v15 = (uint32_t *)((char *)v7 + ext);
    v16 = wid_64;
    do
    {
      *v15 = v14[1];
      v17 = v15 + 1;
      v18 = *v14;
      v14 += 2;
      *v17 = v18;
      v15 = v17 + 1;
      --v16;
    }
    while ( v16 );
    v6 = (uint32_t *)((char *)v14 + line);
    v7 = (uint32_t *)((char *)v15 + ext);
    v8 = v20 - 1;
  }
  while ( v20 != 1 );
}

//****************************************************************
// Size: 1, Format: 2
//

wxUint32 Load8bCI (wxUIntPtr dst, wxUIntPtr src, int wid_64, int height, int line, int real_width, int tile)
{
  if (wid_64 < 1) wid_64 = 1;
  if (height < 1) height = 1;
  int ext = (real_width - (wid_64 << 3));
  unsigned short * pal = rdp.pal_8;

  switch (rdp.tlut_mode) {
    case 0: //palette is not used
      //in tlut DISABLE mode load CI texture as plain intensity texture instead of palette dereference.
      //Thanks to angrylion for the advice
#ifdef OLDASM_asmLoad8bI
      asmLoad8bI ((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext);
#else
      load8bI ((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext);
#endif
      return /*(0 << 16) | */GR_TEXFMT_ALPHA_8;
    case 2: //color palette
      ext <<= 1;
#ifdef OLDASM_asmLoad8bCI
      asmLoad8bCI ((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext, pal);
#else
      load8bCI ((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext, pal);
#endif
      return (1 << 16) | GR_TEXFMT_ARGB_1555;
    default: //IA palette
      ext <<= 1;
#ifdef OLDASM_asmLoad8bIA8
      asmLoad8bIA8 ((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext, pal);
#else
      load8bIA8 ((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext, pal);
#endif
      return (1 << 16) | GR_TEXFMT_ALPHA_INTENSITY_88;
  }
}

//****************************************************************
// Size: 1, Format: 3
//
// ** by Gugaman **

wxUint32 Load8bIA (wxUIntPtr dst, wxUIntPtr src, int wid_64, int height, int line, int real_width, int tile)
{
  if (rdp.tlut_mode != 0)
    return Load8bCI (dst, src, wid_64, height, line, real_width, tile);

  if (wid_64 < 1) wid_64 = 1;
  if (height < 1) height = 1;
  int ext = (real_width - (wid_64 << 3));
#ifdef OLDASM_asmLoad8bIA4
  asmLoad8bIA4 ((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext);
#else
  load8bIA4 ((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext);
#endif
  return /*(0 << 16) | */GR_TEXFMT_ALPHA_INTENSITY_44;
} 

//****************************************************************
// Size: 1, Format: 4
//
// ** by Gugaman **

wxUint32 Load8bI (wxUIntPtr dst, wxUIntPtr src, int wid_64, int height, int line, int real_width, int tile)
{
  if (rdp.tlut_mode != 0)
    return Load8bCI (dst, src, wid_64, height, line, real_width, tile);

  if (wid_64 < 1) wid_64 = 1;
  if (height < 1) height = 1;
  int ext = (real_width - (wid_64 << 3));
#ifdef OLDASM_asmLoad8bI
  asmLoad8bI ((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext);
#else
  load8bI ((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext);
#endif
  return /*(0 << 16) | */GR_TEXFMT_ALPHA_8;
}

