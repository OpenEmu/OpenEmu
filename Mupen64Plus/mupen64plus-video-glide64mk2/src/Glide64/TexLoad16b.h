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

static inline void load16bRGBA(uint8_t *src, uint8_t *dst, int wid_64, int height, int line, int ext)
{
  uint32_t *v6;
  uint32_t *v7;
  int v8;
  int v9;
  uint32_t v10;
  uint32_t v11;
  uint32_t *v12;
  uint32_t *v13;
  int v14;
  uint32_t v15;
  uint32_t v16;
  int v17;
  int v18;

  v6 = (uint32_t *)src;
  v7 = (uint32_t *)dst;
  v8 = height;
  do
  {
    v17 = v8;
    v9 = wid_64;
    do
    {
      v10 = bswap32(*v6);
      v11 = bswap32(v6[1]);
      ALOWORD(v10) = __ROR__((uint16_t)v10, 1);
      ALOWORD(v11) = __ROR__((uint16_t)v11, 1);
      v10 = __ROR__(v10, 16);
      v11 = __ROR__(v11, 16);
      ALOWORD(v10) = __ROR__((uint16_t)v10, 1);
      ALOWORD(v11) = __ROR__((uint16_t)v11, 1);
      *v7 = v10;
      v7[1] = v11;
      v6 += 2;
      v7 += 2;
      --v9;
    }
    while ( v9 );
    if ( v17 == 1 )
      break;
    v18 = v17 - 1;
    v12 = (uint32_t *)&src[(line + (uintptr_t)v6 - (uintptr_t)src) & 0xFFF];
    v13 = (uint32_t *)((char *)v7 + ext);
    v14 = wid_64;
    do
    {
      v15 = bswap32(v12[1]);
      v16 = bswap32(*v12);
      ALOWORD(v15) = __ROR__((uint16_t)v15, 1);
      ALOWORD(v16) = __ROR__((uint16_t)v16, 1);
      v15 = __ROR__(v15, 16);
      v16 = __ROR__(v16, 16);
      ALOWORD(v15) = __ROR__((uint16_t)v15, 1);
      ALOWORD(v16) = __ROR__((uint16_t)v16, 1);
      *v13 = v15;
      v13[1] = v16;
      v12 += 2;
      v13 += 2;
      --v14;
    }
    while ( v14 );
    v6 = (uint32_t *)&src[(line + (uintptr_t)v12 - (uintptr_t)src) & 0xFFF];
    v7 = (uint32_t *)((char *)v13 + ext);
    v8 = v18 - 1;
  }
  while ( v18 != 1 );
}

static inline void load16bIA(uint8_t *src, uint8_t *dst, int wid_64, int height, int line, int ext)
{
  uint32_t *v6;
  uint32_t *v7;
  int v8;
  int v9;
  uint32_t v10;
  uint32_t *v11;
  uint32_t *v12;
  int v13;
  uint32_t v14;
  int v15;
  int v16;

  v6 = (uint32_t *)src;
  v7 = (uint32_t *)dst;
  v8 = height;
  do
  {
    v15 = v8;
    v9 = wid_64;
    do
    {
      v10 = v6[1];
      *v7 = *v6;
      v7[1] = v10;
      v6 += 2;
      v7 += 2;
      --v9;
    }
    while ( v9 );
    if ( v15 == 1 )
      break;
    v16 = v15 - 1;
    v11 = (uint32_t *)((char *)v6 + line);
    v12 = (uint32_t *)((char *)v7 + ext);
    v13 = wid_64;
    do
    {
      v14 = *v11;
      *v12 = v11[1];
      v12[1] = v14;
      v11 += 2;
      v12 += 2;
      --v13;
    }
    while ( v13 );
    v6 = (uint32_t *)((char *)v11 + line);
    v7 = (uint32_t *)((char *)v12 + ext);
    v8 = v16 - 1;
  }
  while ( v16 != 1 );
}


//****************************************************************
// Size: 2, Format: 0
//

wxUint32 Load16bRGBA (wxUIntPtr dst, wxUIntPtr src, int wid_64, int height, int line, int real_width, int tile)
{
  if (wid_64 < 1) wid_64 = 1;
  if (height < 1) height = 1;
  int ext = (real_width - (wid_64 << 2)) << 1;

  load16bRGBA((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext);

  return (1 << 16) | GR_TEXFMT_ARGB_1555;
}

//****************************************************************
// Size: 2, Format: 3
//
// ** by Gugaman/Dave2001 **

wxUint32 Load16bIA (wxUIntPtr dst, wxUIntPtr src, int wid_64, int height, int line, int real_width, int tile)
{
  if (wid_64 < 1) wid_64 = 1;
  if (height < 1) height = 1;
  int ext = (real_width - (wid_64 << 2)) << 1;

  load16bIA((uint8_t *)src, (uint8_t *)dst, wid_64, height, line, ext);

  return (1 << 16) | GR_TEXFMT_ALPHA_INTENSITY_88;
}

//****************************************************************
// Size: 2, Format: 1
//

wxUint16 yuv_to_rgb565(wxUint8 y, wxUint8 u, wxUint8 v)
{
  //*
  float r = y + (1.370705f * (v-128));
  float g = y - (0.698001f * (v-128)) - (0.337633f * (u-128));
  float b = y + (1.732446f * (u-128));
  r *= 0.125f;
  g *= 0.25f;
  b *= 0.125f;
  //clipping the result
  if (r > 31) r = 31;
  if (g > 63) g = 63;
  if (b > 31) b = 31;
  if (r < 0) r = 0;
  if (g < 0) g = 0;
  if (b < 0) b = 0;
  wxUint16 c = (wxUint16)(((wxUint16)(r) << 11) |
    ((wxUint16)(g) << 5) |
    (wxUint16)(b) );
  return c;
  //*/
  /*
  const wxUint32 c = y - 16;
  const wxUint32 d = u - 128;
  const wxUint32 e = v - 128;

  wxUint32 r =  (298 * c           + 409 * e + 128) & 0xf800;
  wxUint32 g = ((298 * c - 100 * d - 208 * e + 128) >> 5) & 0x7e0;
  wxUint32 b = ((298 * c + 516 * d           + 128) >> 11) & 0x1f;

  WORD texel = (WORD)(r | g | b);

  return texel;
  */
}

//****************************************************************
// Size: 2, Format: 1
//

wxUint32 Load16bYUV (wxUIntPtr dst, wxUIntPtr src, int wid_64, int height, int line, int real_width, int tile)
{
  wxUint32 * mb = (wxUint32*)(gfx.RDRAM+rdp.addr[rdp.tiles[tile].t_mem]); //pointer to the macro block
  wxUint16 * tex = (wxUint16*)dst;
  wxUint16 i;
  for (i = 0; i < 128; i++)
  {
    wxUint32 t = mb[i]; //each wxUint32 contains 2 pixels
    wxUint8 y1 = (wxUint8)t&0xFF;
    wxUint8 v  = (wxUint8)(t>>8)&0xFF;
    wxUint8 y0 = (wxUint8)(t>>16)&0xFF;
    wxUint8 u  = (wxUint8)(t>>24)&0xFF;
    wxUint16 c = yuv_to_rgb565(y0, u, v);
    *(tex++) = c;
    c = yuv_to_rgb565(y1, u, v);
    *(tex++) = c;
  }
  return (1 << 16) | GR_TEXFMT_RGB_565;
}
