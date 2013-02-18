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
#include "Gfx_1.3.h"

//****************************************************************
// Size: 2, Format: 0
//
// Load 32bit RGBA texture
// Based on sources of angrylion's software plugin.
//
wxUint32 Load32bRGBA (wxUIntPtr dst, wxUIntPtr src, int wid_64, int height, int line, int real_width, int tile)
{
  if (height < 1) height = 1;
  const wxUint16 *tmem16 = (wxUint16*)rdp.tmem;
  const wxUint32 tbase = (src - (wxUIntPtr)rdp.tmem) >> 1;
  const wxUint32 width = max(1, wid_64 << 1);
  const int ext = real_width - width;
  line = width + (line>>2);
  wxUint32 s, t, c;
  wxUint32 * tex = (wxUint32*)dst;
  wxUint16 rg, ba;
  for (t = 0; t < (wxUint32)height; t++)
  {
    wxUint32 tline = tbase + line * t;
    wxUint32 xorval = (t & 1) ? 3 : 1;
    for (s = 0; s < width; s++)
    {
      wxUint32 taddr = ((tline + s) ^ xorval) & 0x3ff;
      rg = tmem16[taddr];
      ba = tmem16[taddr|0x400];
      c = ((ba&0xFF)<<24) | (rg << 8) | (ba>>8);
      *tex++ = c;
    }
    tex += ext;
  }
  int id = tile - rdp.cur_tile;
  wxUint32 mod = (id == 0) ? cmb.mod_0 : cmb.mod_1;
  if (mod || !voodoo.sup_32bit_tex)
  {
    //convert to ARGB_4444
    const wxUint32 tex_size = real_width * height;
    tex = (wxUint32 *)dst;
    wxUint16 *tex16 = (wxUint16*)dst;
    wxUint16 a, r, g, b;
    for (wxUint32 i = 0; i < tex_size; i++) {
      c = tex[i];
      a = (c >> 28) & 0xF;
      r = (c >> 20) & 0xF;
      g = (c >> 12) & 0xF;
      b = (c >> 4)  & 0xF;
      tex16[i] = (a <<12) | (r << 8) | (g << 4) | b;
    }
    return (1 << 16) | GR_TEXFMT_ARGB_4444;
  }
  return (2 << 16) | GR_TEXFMT_ARGB_8888;
}

//****************************************************************
// LoadTile for 32bit RGBA texture
// Based on sources of angrylion's software plugin.
//
void LoadTile32b (wxUint32 tile, wxUint32 ul_s, wxUint32 ul_t, wxUint32 width, wxUint32 height)
{
  const wxUint32 line = rdp.tiles[tile].line << 2;
  const wxUint32 tbase = rdp.tiles[tile].t_mem << 2;
  const wxUint32 addr = rdp.timg.addr >> 2;
  const wxUint32* src = (const wxUint32*)gfx.RDRAM;
  wxUint16 *tmem16 = (wxUint16*)rdp.tmem;
  wxUint32 c, ptr, tline, s, xorval;

  for (wxUint32 j = 0; j < height; j++)
  {
    tline = tbase + line * j;
    s = ((j + ul_t) * rdp.timg.width) + ul_s;
    xorval = (j & 1) ? 3 : 1;				
    for (wxUint32 i = 0; i < width; i++)
    {
      c = src[addr + s + i];
      ptr = ((tline + i) ^ xorval) & 0x3ff;
      tmem16[ptr] = c >> 16;
      tmem16[ptr|0x400] = c & 0xffff;
    }
  }
}

//****************************************************************
// LoadBlock for 32bit RGBA texture
// Based on sources of angrylion's software plugin.
//
void LoadBlock32b(wxUint32 tile, wxUint32 ul_s, wxUint32 ul_t, wxUint32 lr_s, wxUint32 dxt)
{
  const wxUint32 * src = (const wxUint32*)gfx.RDRAM;
  const wxUint32 tb = rdp.tiles[tile].t_mem << 2;
  const wxUint32 tiwindwords = rdp.timg.width;
  const wxUint32 slindwords = ul_s;
  const wxUint32 line = rdp.tiles[tile].line << 2;

  wxUint16 *tmem16 = (wxUint16*)rdp.tmem;
  wxUint32 addr = rdp.timg.addr >> 2;
  wxUint32 width = (lr_s - ul_s + 1) << 2;
  if (width & 7)
    width = (width & (~7)) + 8;

  if (dxt != 0)
  {
    wxUint32 j= 0;
    wxUint32 t = 0;
    wxUint32 oldt = 0;
    wxUint32 ptr;

    addr += (ul_t * tiwindwords) + slindwords;
    wxUint32 c = 0;
    for (wxUint32 i = 0; i < width; i += 2)
    {
      oldt = t;
      t = ((j >> 11) & 1) ? 3 : 1;
      if (t != oldt)
        i += line;
      ptr = ((tb + i) ^ t) & 0x3ff;
      c = src[addr + i];
      tmem16[ptr] = c >> 16;
      tmem16[ptr|0x400] = c & 0xffff;
      ptr = ((tb+ i + 1) ^ t) & 0x3ff;
      c = src[addr + i + 1];
      tmem16[ptr] = c >> 16;
      tmem16[ptr|0x400] = c & 0xffff;
      j += dxt;
    }
  }
  else
  {
    addr += (ul_t * tiwindwords) + slindwords;
    wxUint32 c, ptr;
    for (wxUint32 i = 0; i < width; i ++)
    {
      ptr = ((tb + i) ^ 1) & 0x3ff;
      c = src[addr + i];
      tmem16[ptr] = c >> 16;
      tmem16[ptr|0x400] = c & 0xffff;
    }
  }
}
