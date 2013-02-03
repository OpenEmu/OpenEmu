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
//
// Draw N64 frame buffer to screen.
// Created by Gonetz, 2007
//
//****************************************************************


#include "Gfx #1.3.h"
#include "FBtoScreen.h"
#include "TexCache.h"

static int SetupFBtoScreenCombiner(wxUint32 texture_size, wxUint32 opaque)
{
  int tmu;
  if (voodoo.tmem_ptr[GR_TMU0]+texture_size < voodoo.tex_max_addr[0])
  {
    tmu = GR_TMU0;
    grTexCombine( GR_TMU1,
      GR_COMBINE_FUNCTION_NONE,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_NONE,
      GR_COMBINE_FACTOR_NONE,
      FXFALSE,
      FXFALSE );
    grTexCombine( GR_TMU0,
      GR_COMBINE_FUNCTION_LOCAL,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_LOCAL,
      GR_COMBINE_FACTOR_NONE,
      FXFALSE,
      FXFALSE );
  }
  else
  {
    if (voodoo.tmem_ptr[GR_TMU1]+texture_size >= voodoo.tex_max_addr[1])
      ClearCache ();
    tmu = GR_TMU1;
    grTexCombine( GR_TMU1,
      GR_COMBINE_FUNCTION_LOCAL,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_LOCAL,
      GR_COMBINE_FACTOR_NONE,
      FXFALSE,
      FXFALSE );
    grTexCombine( GR_TMU0,
      GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      FXFALSE,
      FXFALSE );
  }
  int filter = (rdp.filter_mode!=2)?GR_TEXTUREFILTER_POINT_SAMPLED:GR_TEXTUREFILTER_BILINEAR;
  grTexFilterMode (tmu, filter, filter);
  grTexClampMode (tmu,
        GR_TEXTURECLAMP_CLAMP,
        GR_TEXTURECLAMP_CLAMP);
//  grConstantColorValue (0xFFFFFFFF);
  grColorCombine (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE,
//    GR_COMBINE_OTHER_CONSTANT,
    FXFALSE);
  grAlphaCombine (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE,
    FXFALSE);
  if (opaque)
  {
    grAlphaTestFunction (GR_CMP_ALWAYS);
    grAlphaBlendFunction( GR_BLEND_ONE,
      GR_BLEND_ZERO,
      GR_BLEND_ONE,
      GR_BLEND_ZERO);
  }
  else
  {
    grAlphaBlendFunction( GR_BLEND_SRC_ALPHA,
      GR_BLEND_ONE_MINUS_SRC_ALPHA,
      GR_BLEND_ONE,
      GR_BLEND_ZERO);
  }
  grDepthBufferFunction (GR_CMP_ALWAYS);
  grCullMode(GR_CULL_DISABLE);
  grDepthMask (FXFALSE);
  rdp.update |= UPDATE_COMBINE | UPDATE_ZBUF_ENABLED | UPDATE_CULL_MODE;
  return tmu;
}

static void DrawRE2Video(FB_TO_SCREEN_INFO & fb_info, float scale)
{
  float scale_y = (float)fb_info.width/rdp.vi_height;
  float height = settings.scr_res_x/scale_y;
  float ul_x = 0.5f;
  float ul_y = (settings.scr_res_y - height)/2.0f;
  float lr_y = settings.scr_res_y - ul_y - 1.0f;
  float lr_x = settings.scr_res_x - 1.0f;
  float lr_u = (fb_info.width - 1)*scale;
  float lr_v = (fb_info.height - 1)*scale;
  VERTEX v[4] = {
    { ul_x, ul_y, 1, 1, 0.5f, 0.5f, 0.5f, 0.5f, {0.5f, 0.5f, 0.5f, 0.5f} },
    { lr_x, ul_y, 1, 1, lr_u, 0.5f, lr_u, 0.5f, {lr_u, 0.5f, lr_u, 0.5f} },
    { ul_x, lr_y, 1, 1, 0.5f, lr_v, 0.5f, lr_v, {0.5f, lr_v, 0.5f, lr_v} },
    { lr_x, lr_y, 1, 1, lr_u, lr_v, lr_u, lr_v, {lr_u, lr_v, lr_u, lr_v} }
  };
  grDrawTriangle (&v[0], &v[2], &v[1]);
  grDrawTriangle (&v[2], &v[3], &v[1]);
}

static void DrawRE2Video256(FB_TO_SCREEN_INFO & fb_info)
{
  FRDP("DrawRE2Video256. ul_x=%d, ul_y=%d, lr_x=%d, lr_y=%d, size=%d, addr=%08lx\n", fb_info.ul_x, fb_info.ul_y, fb_info.lr_x, fb_info.lr_y, fb_info.size, fb_info.addr);
  wxUint32 * src = (wxUint32*)(gfx.RDRAM+fb_info.addr);
  GrTexInfo t_info;
  t_info.smallLodLog2 = GR_LOD_LOG2_256;
  t_info.largeLodLog2 = GR_LOD_LOG2_256;
  t_info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  wxUint16 * tex = (wxUint16*)texture_buffer;
  wxUint16 * dst = tex;
  wxUint32 col;
  wxUint8 r, g, b;
  fb_info.height = min(256, fb_info.height);
  for (wxUint32 h = 0; h < fb_info.height; h++)
  {
    for (wxUint32 w = 0; w < 256; w++)
    {
      col = *(src++);
      r = (wxUint8)((col >> 24)&0xFF);
      r = (wxUint8)((float)r / 255.0f * 31.0f);
      g = (wxUint8)((col >> 16)&0xFF);
      g = (wxUint8)((float)g / 255.0f * 63.0f);
      b = (wxUint8)((col >>  8)&0xFF);
      b = (wxUint8)((float)b / 255.0f * 31.0f);
      *(dst++) = (r << 11) | (g << 5) | b;
    }
    src += (fb_info.width - 256);
  }
  t_info.format = GR_TEXFMT_RGB_565;
  t_info.data = tex;
  int tmu = SetupFBtoScreenCombiner(grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &t_info), fb_info.opaque);
  grTexDownloadMipMap (tmu,
    voodoo.tex_min_addr[tmu]+voodoo.tmem_ptr[tmu],
    GR_MIPMAPLEVELMASK_BOTH,
    &t_info);
  grTexSource (tmu,
      voodoo.tex_min_addr[tmu]+voodoo.tmem_ptr[tmu],
      GR_MIPMAPLEVELMASK_BOTH,
      &t_info);
  DrawRE2Video(fb_info, 1.0f);
}

static void DrawFrameBufferToScreen256(FB_TO_SCREEN_INFO & fb_info)
{
  if (settings.hacks&hack_RE2)
  {
    DrawRE2Video256(fb_info);
    return;
  }
  FRDP("DrawFrameBufferToScreen256. ul_x=%d, ul_y=%d, lr_x=%d, lr_y=%d, size=%d, addr=%08lx\n", fb_info.ul_x, fb_info.ul_y, fb_info.lr_x, fb_info.lr_y, fb_info.size, fb_info.addr);
  wxUint32 width = fb_info.lr_x - fb_info.ul_x + 1;
  wxUint32 height = fb_info.lr_y - fb_info.ul_y + 1;
  GrTexInfo t_info;
  wxUint8 * image = gfx.RDRAM+fb_info.addr;
  wxUint32 width256 = ((width-1) >> 8) + 1;
  wxUint32 height256 = ((height-1) >> 8) + 1;
  t_info.smallLodLog2 = t_info.largeLodLog2 = GR_LOD_LOG2_256;
  t_info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  t_info.format = GR_TEXFMT_ARGB_1555;
  wxUint16 * tex = (wxUint16*)texture_buffer;
  t_info.data = tex;
  wxUint32 tex_size = grTexTextureMemRequired (GR_MIPMAPLEVELMASK_BOTH, &t_info);
  int tmu = SetupFBtoScreenCombiner(tex_size*width256*height256, fb_info.opaque);
  wxUint16 * src = (wxUint16*)image;
  src += fb_info.ul_x + fb_info.ul_y * fb_info.width;
  wxUint32 * src32 = (wxUint32*)image;
  src32 += fb_info.ul_x + fb_info.ul_y * fb_info.width;
  wxUint32 w_tail = width%256;
  wxUint32 h_tail = height%256;
  wxUint16 c;
  wxUint32 c32;
  wxUint32 idx;
  wxUint32 bound = BMASK+1-fb_info.addr;
  bound = fb_info.size == 2 ? bound >> 1 : bound >> 2;
  wxUint8 r, g, b, a;
  wxUint32 cur_width, cur_height, cur_tail;
  wxUint32 tex_adr = voodoo.tex_min_addr[tmu]+voodoo.tmem_ptr[tmu];
  if ((voodoo.tmem_ptr[tmu] < TEXMEM_2MB_EDGE) && (voodoo.tmem_ptr[tmu]+tex_size*width256*height256 > TEXMEM_2MB_EDGE))
  {
    tex_adr = TEXMEM_2MB_EDGE;
  }
  for (wxUint32 h = 0; h < height256; h++)
  {
    for (wxUint32 w = 0; w < width256; w++)
    {
      cur_width = (256*(w+1) < width) ? 256 : w_tail;
      cur_height = (256*(h+1) < height) ? 256 : h_tail;
      cur_tail = 256 - cur_width;
      wxUint16 * dst = tex;
      if (fb_info.size == 2)
      {
        for (wxUint32 y=0; y < cur_height; y++)
        {
          for (wxUint32 x=0; x < cur_width; x++)
          {
            idx = (x+256*w+(y+256*h)*fb_info.width)^1;
            if (idx >= bound)
              break;
            c = src[idx];
            *(dst++) = (c >> 1) | ((c&1)<<15);
          }
          dst += cur_tail;
        }
      }
      else
      {
        for (wxUint32 y=0; y < cur_height; y++)
        {
          for (wxUint32 x=0; x < cur_width; x++)
          {
            idx = (x+256*w+(y+256*h)*fb_info.width);
            if (idx >= bound)
              break;
            c32 = src32[idx];
            r = (wxUint8)((c32 >> 24)&0xFF);
            r = (wxUint8)((float)r / 255.0f * 31.0f);
            g = (wxUint8)((c32 >> 16)&0xFF);
            g = (wxUint8)((float)g / 255.0f * 63.0f);
            b = (wxUint8)((c32 >>  8)&0xFF);
            b = (wxUint8)((float)b / 255.0f * 31.0f);
            a = (c32&0xFF) ? 1 : 0;
            *(dst++) = (a<<15) | (r << 10) | (g << 5) | b;
          }
          dst += cur_tail;
        }
      }
      grTexDownloadMipMap (tmu, tex_adr, GR_MIPMAPLEVELMASK_BOTH, &t_info);
      grTexSource (tmu, tex_adr, GR_MIPMAPLEVELMASK_BOTH, &t_info);
      tex_adr += tex_size;
      float ul_x = (float)(fb_info.ul_x + 256*w);
      float ul_y = (float)(fb_info.ul_y + 256*h);
      float lr_x = (ul_x + (float)(cur_width)) * rdp.scale_x;
      float lr_y = (ul_y + (float)(cur_height)) * rdp.scale_y;
      ul_x *= rdp.scale_x;
      ul_y *= rdp.scale_y;
      ul_x += rdp.offset_x;
      ul_y += rdp.offset_y;
      lr_x += rdp.offset_x;
      lr_y += rdp.offset_y;

      float lr_u = (float)(cur_width-1);
      float lr_v = (float)(cur_height-1);
      // Make the vertices
      VERTEX v[4] = {
        { ul_x, ul_y, 1, 1, 0.5f, 0.5f, 0.5f, 0.5f, {0.5f, 0.5f, 0.5f, 0.5f} },
        { lr_x, ul_y, 1, 1, lr_u, 0.5f, lr_u, 0.5f, {lr_u, 0.5f, lr_u, 0.5f} },
        { ul_x, lr_y, 1, 1, 0.5f, lr_v, 0.5f, lr_v, {0.5f, lr_v, 0.5f, lr_v} },
        { lr_x, lr_y, 1, 1, lr_u, lr_v, lr_u, lr_v, {lr_u, lr_v, lr_u, lr_v} }
      };
      grDrawTriangle (&v[0], &v[2], &v[1]);
      grDrawTriangle (&v[2], &v[3], &v[1]);
    }
  }
}

bool DrawFrameBufferToScreen(FB_TO_SCREEN_INFO & fb_info)
{
  if (fb_info.width < 200 || fb_info.size < 2)
    return false;
  wxUint32 width = fb_info.lr_x - fb_info.ul_x + 1;
  wxUint32 height = fb_info.lr_y - fb_info.ul_y + 1;
  wxUint32 max_size = min(voodoo.max_tex_size, 512);
  if (width > (wxUint32)max_size || height > (wxUint32)max_size)
  {
    DrawFrameBufferToScreen256(fb_info);
    return true;
  }
  FRDP("DrawFrameBufferToScreen. ul_x=%d, ul_y=%d, lr_x=%d, lr_y=%d, size=%d, addr=%08lx\n", fb_info.ul_x, fb_info.ul_y, fb_info.lr_x, fb_info.lr_y, fb_info.size, fb_info.addr);
  GrTexInfo t_info;
  wxUint8 * image = gfx.RDRAM+fb_info.addr;
  wxUint32 texwidth, texheight;
  float scale;
  if (width <= 256)
  {
    texwidth = 256;
    scale = 1.0f;
    t_info.smallLodLog2 = t_info.largeLodLog2 = GR_LOD_LOG2_256;
  }
  else
  {
    texwidth = 512;
    scale = 0.5f;
    t_info.smallLodLog2 = t_info.largeLodLog2 = GR_LOD_LOG2_512;
  }

  if (height <= (texwidth>>1))
  {
    t_info.aspectRatioLog2 = GR_ASPECT_LOG2_2x1;
    texheight = texwidth>>1;
  }
  else
  {
    t_info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
    texheight = texwidth;
  }

  if (fb_info.size == 2)
  {
    wxUint16 * tex = (wxUint16*)texture_buffer;
    wxUint16 * dst = tex;
    wxUint16 * src = (wxUint16*)image;
    src += fb_info.ul_x + fb_info.ul_y * fb_info.width;
    wxUint16 c;
    wxUint32 idx;
    const wxUint32 bound = (BMASK+1-fb_info.addr) >> 1;
    bool empty = true;
    for (wxUint32 y=0; y < height; y++)
    {
      for (wxUint32 x=0; x < width; x++)
      {
        idx = (x+y*fb_info.width)^1;
        if (idx >= bound)
          break;
        c = src[idx];
        if (c) empty = false;
        *(dst++) = (c >> 1) | ((c&1)<<15);
      }
      dst += texwidth-width;
    }
    if (empty)
      return false;
    t_info.format = GR_TEXFMT_ARGB_1555;
    t_info.data = tex;
  }
  else
  {
    wxUint32 * tex = (wxUint32*)texture_buffer;
    wxUint32 * dst = tex;
    wxUint32 * src = (wxUint32*)image;
    src += fb_info.ul_x + fb_info.ul_y * fb_info.width;
    wxUint32 col;
    wxUint32 idx;
    const wxUint32 bound = (BMASK+1-fb_info.addr) >> 2;
    for (wxUint32 y=0; y < height; y++)
    {
      for (wxUint32 x=0; x < width; x++)
      {
        idx = x+y*fb_info.width;
        if (idx >= bound)
          break;
        col = src[idx];
        *(dst++) = (col >> 8) | 0xFF000000;
      }
      dst += texwidth-width;
    }
    t_info.format = GR_TEXFMT_ARGB_8888;
    t_info.data = tex;
  }

  int tmu = SetupFBtoScreenCombiner(grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &t_info), fb_info.opaque);
  grTexDownloadMipMap (tmu,
    voodoo.tex_min_addr[tmu]+voodoo.tmem_ptr[tmu],
    GR_MIPMAPLEVELMASK_BOTH,
    &t_info);
  grTexSource (tmu,
    voodoo.tex_min_addr[tmu]+voodoo.tmem_ptr[tmu],
    GR_MIPMAPLEVELMASK_BOTH,
    &t_info);
  if (settings.hacks&hack_RE2)
  {
    DrawRE2Video(fb_info, scale);
  }
  else
  {
    float ul_x = fb_info.ul_x * rdp.scale_x + rdp.offset_x;
    float ul_y = fb_info.ul_y * rdp.scale_y + rdp.offset_y;
    float lr_x = fb_info.lr_x * rdp.scale_x + rdp.offset_x;
    float lr_y = fb_info.lr_y * rdp.scale_y + rdp.offset_y;
    float lr_u = (width-1)*scale;
    float lr_v = (height-1)*scale;
    // Make the vertices
    VERTEX v[4] = {
      { ul_x, ul_y, 1, 1, 0.5f, 0.5f, 0.5f, 0.5f, {0.5f, 0.5f, 0.5f, 0.5f} },
      { lr_x, ul_y, 1, 1, lr_u, 0.5f, lr_u, 0.5f, {lr_u, 0.5f, lr_u, 0.5f} },
      { ul_x, lr_y, 1, 1, 0.5f, lr_v, 0.5f, lr_v, {0.5f, lr_v, 0.5f, lr_v} },
      { lr_x, lr_y, 1, 1, lr_u, lr_v, lr_u, lr_v, {lr_u, lr_v, lr_u, lr_v} }
    };
    grDrawTriangle (&v[0], &v[2], &v[1]);
    grDrawTriangle (&v[2], &v[3], &v[1]);
  }
  return true;
}

static void DrawDepthBufferToScreen256(FB_TO_SCREEN_INFO & fb_info)
{
  FRDP("DrawDepthBufferToScreen256. ul_x=%d, ul_y=%d, lr_x=%d, lr_y=%d, size=%d, addr=%08lx\n", fb_info.ul_x, fb_info.ul_y, fb_info.lr_x, fb_info.lr_y, fb_info.size, fb_info.addr);
  wxUint32 width = fb_info.lr_x - fb_info.ul_x + 1;
  wxUint32 height = fb_info.lr_y - fb_info.ul_y + 1;
  GrTexInfo t_info;
  wxUint8 * image = gfx.RDRAM+fb_info.addr;
  wxUint32 width256 = ((width-1) >> 8) + 1;
  wxUint32 height256 = ((height-1) >> 8) + 1;
  t_info.smallLodLog2 = t_info.largeLodLog2 = GR_LOD_LOG2_256;
  t_info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  t_info.format = GR_TEXFMT_ALPHA_INTENSITY_88;
  wxUint16 * tex = (wxUint16*)texture_buffer;
  t_info.data = tex;
  wxUint32 tex_size = grTexTextureMemRequired (GR_MIPMAPLEVELMASK_BOTH, &t_info);
  int tmu = SetupFBtoScreenCombiner(tex_size*width256*height256, fb_info.opaque);
  grConstantColorValue (rdp.fog_color);
  grColorCombine (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_CONSTANT,
    FXFALSE);
  wxUint16 * src = (wxUint16*)image;
  src += fb_info.ul_x + fb_info.ul_y * fb_info.width;
  wxUint32 w_tail = width%256;
  wxUint32 h_tail = height%256;
  wxUint32 cur_width, cur_height, cur_tail;
  wxUint32 tex_adr = voodoo.tex_min_addr[tmu]+voodoo.tmem_ptr[tmu];
  if ((voodoo.tmem_ptr[tmu] < TEXMEM_2MB_EDGE) && (voodoo.tmem_ptr[tmu]+tex_size*width256*height256 > TEXMEM_2MB_EDGE))
  {
    tex_adr = TEXMEM_2MB_EDGE;
  }
  for (wxUint32 h = 0; h < height256; h++)
  {
    for (wxUint32 w = 0; w < width256; w++)
    {
      cur_width = (256*(w+1) < width) ? 256 : w_tail;
      cur_height = (256*(h+1) < height) ? 256 : h_tail;
      cur_tail = 256 - cur_width;
      wxUint16 * dst = tex;
      for (wxUint32 y=0; y < cur_height; y++)
      {
        for (wxUint32 x=0; x < cur_width; x++)
        {
          *(dst++) = rdp.pal_8[src[(x+256*w+(y+256*h)*fb_info.width)^1]>>8];
        }
        dst += cur_tail;
      }
      grTexDownloadMipMap (tmu, tex_adr, GR_MIPMAPLEVELMASK_BOTH, &t_info);
      grTexSource (tmu, tex_adr, GR_MIPMAPLEVELMASK_BOTH, &t_info);
      tex_adr += tex_size;
      float ul_x = (float)(fb_info.ul_x + 256*w);
      float ul_y = (float)(fb_info.ul_y + 256*h);
      float lr_x = (ul_x + (float)(cur_width)) * rdp.scale_x + rdp.offset_x;
      float lr_y = (ul_y + (float)(cur_height)) * rdp.scale_y + rdp.offset_y;
      ul_x = ul_x * rdp.scale_x + rdp.offset_x;
      ul_y = ul_y * rdp.scale_y + rdp.offset_y;
      float lr_u = (float)(cur_width-1);
      float lr_v = (float)(cur_height-1);
      // Make the vertices
      VERTEX v[4] = {
        { ul_x, ul_y, 1, 1, 0.5f, 0.5f, 0.5f, 0.5f, {0.5f, 0.5f, 0.5f, 0.5f} },
        { lr_x, ul_y, 1, 1, lr_u, 0.5f, lr_u, 0.5f, {lr_u, 0.5f, lr_u, 0.5f} },
        { ul_x, lr_y, 1, 1, 0.5f, lr_v, 0.5f, lr_v, {0.5f, lr_v, 0.5f, lr_v} },
        { lr_x, lr_y, 1, 1, lr_u, lr_v, lr_u, lr_v, {lr_u, lr_v, lr_u, lr_v} }
      };
      grDrawTriangle (&v[0], &v[2], &v[1]);
      grDrawTriangle (&v[2], &v[3], &v[1]);
    }
  }
}

static void DrawHiresDepthBufferToScreen(FB_TO_SCREEN_INFO & fb_info)
{
  FRDP("DrawHiresDepthBufferToScreen. ul_x=%d, ul_y=%d, lr_x=%d, lr_y=%d, size=%d, addr=%08lx\n", fb_info.ul_x, fb_info.ul_y, fb_info.lr_x, fb_info.lr_y, fb_info.size, fb_info.addr);
  GrTexInfo t_info;
  float scale = 0.25f;
  GrLOD_t LOD = GR_LOD_LOG2_1024;
  if (settings.scr_res_x > 1024)
  {
    scale = 0.125f;
    LOD = GR_LOD_LOG2_2048;
  }
  t_info.format = GR_TEXFMT_ALPHA_INTENSITY_88;
  t_info.smallLodLog2 = t_info.largeLodLog2 = LOD;
  t_info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  grConstantColorValue (rdp.fog_color);
  grColorCombine (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE,
    FXFALSE);
  grAlphaCombine (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE,
    FXFALSE);
  grAlphaBlendFunction( GR_BLEND_SRC_ALPHA,
    GR_BLEND_ONE_MINUS_SRC_ALPHA,
    GR_BLEND_ONE,
    GR_BLEND_ZERO);
  grDepthBufferFunction (GR_CMP_ALWAYS);
  grDepthMask (FXFALSE);
  grCullMode (GR_CULL_DISABLE);
  grTexCombine( GR_TMU1,
    GR_COMBINE_FUNCTION_NONE,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_FUNCTION_NONE,
    GR_COMBINE_FACTOR_NONE,
    FXFALSE,
    FXFALSE );
  grTexCombine( GR_TMU0,
    GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    FXFALSE,
    FXFALSE);
//  grAuxBufferExt( GR_BUFFER_AUXBUFFER );
  grTexSource( rdp.texbufs[0].tmu, rdp.texbufs[0].begin, GR_MIPMAPLEVELMASK_BOTH, &(t_info) );
  float ul_x = (float)rdp.scissor.ul_x;
  float ul_y = (float)rdp.scissor.ul_y;
  float lr_x = (float)rdp.scissor.lr_x;
  float lr_y = (float)rdp.scissor.lr_y;
  float ul_u = (float)rdp.scissor.ul_x * scale;
  float ul_v = (float)rdp.scissor.ul_y * scale;
  float lr_u = (float)rdp.scissor.lr_x * scale;
  float lr_v = (float)rdp.scissor.lr_y * scale;
  // Make the vertices
  VERTEX v[4] = {
    { ul_x, ul_y, 1, 1, ul_u, ul_v, ul_u, ul_v, {ul_u, ul_v, ul_u, ul_v} },
    { lr_x, ul_y, 1, 1, lr_u, ul_v, lr_u, ul_v, {lr_u, ul_v, lr_u, ul_v} },
    { ul_x, lr_y, 1, 1, ul_u, lr_v, ul_u, lr_v, {ul_u, lr_v, ul_u, lr_v} },
    { lr_x, lr_y, 1, 1, lr_u, lr_v, lr_u, lr_v, {lr_u, lr_v, lr_u, lr_v} }
  };
  grDrawTriangle (&v[0], &v[2], &v[1]);
  grDrawTriangle (&v[2], &v[3], &v[1]);
//  grAuxBufferExt( GR_BUFFER_TEXTUREAUXBUFFER_EXT );
  rdp.update |= UPDATE_COMBINE | UPDATE_ZBUF_ENABLED | UPDATE_CULL_MODE;
}

void DrawDepthBufferToScreen(FB_TO_SCREEN_INFO & fb_info)
{
  wxUint32 width = fb_info.lr_x - fb_info.ul_x + 1;
  wxUint32 height = fb_info.lr_y - fb_info.ul_y + 1;
  if (width > (wxUint32)voodoo.max_tex_size || height > (wxUint32)voodoo.max_tex_size || width > 512)
  {
    DrawDepthBufferToScreen256(fb_info);
    return;
  }
  if (fb_hwfbe_enabled && !evoodoo)
  {
    DrawHiresDepthBufferToScreen(fb_info);
    return;
  }
  FRDP("DrawDepthBufferToScreen. ul_x=%d, ul_y=%d, lr_x=%d, lr_y=%d, size=%d, addr=%08lx\n", fb_info.ul_x, fb_info.ul_y, fb_info.lr_x, fb_info.lr_y, fb_info.size, fb_info.addr);
  GrTexInfo t_info;
  wxUint8 * image = gfx.RDRAM+fb_info.addr;
  wxUint32 texwidth, texheight;
  float scale;
  if (width <= 256)
  {
    texwidth = 256;
    scale = 1.0f;
    t_info.smallLodLog2 = t_info.largeLodLog2 = GR_LOD_LOG2_256;
  }
  else
  {
    texwidth = 512;
    scale = 0.5f;
    t_info.smallLodLog2 = t_info.largeLodLog2 = GR_LOD_LOG2_512;
  }

  if (height <= (texwidth>>1))
  {
    t_info.aspectRatioLog2 = GR_ASPECT_LOG2_2x1;
    texheight = texwidth>>1;
  }
  else
  {
    t_info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
    texheight = texwidth;
  }

  wxUint16 * tex = (wxUint16*)texture_buffer;
  wxUint16 * dst = tex;
  wxUint16 * src = (wxUint16*)image;
  src += fb_info.ul_x + fb_info.ul_y * fb_info.width;
  for (wxUint32 y=0; y < height; y++)
  {
    for (wxUint32 x=0; x < width; x++)
    {
      *(dst++) = rdp.pal_8[src[(x+y*fb_info.width)^1]>>8];
    }
    dst += texwidth-width;
  }
  t_info.format = GR_TEXFMT_ALPHA_INTENSITY_88;
  t_info.data = tex;

  int tmu = SetupFBtoScreenCombiner(grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &t_info), fb_info.opaque);
  grConstantColorValue (rdp.fog_color);
  grColorCombine (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_CONSTANT,
    FXFALSE);
  grTexDownloadMipMap (tmu,
    voodoo.tex_min_addr[tmu]+voodoo.tmem_ptr[tmu],
    GR_MIPMAPLEVELMASK_BOTH,
    &t_info);
  grTexSource (tmu,
    voodoo.tex_min_addr[tmu]+voodoo.tmem_ptr[tmu],
    GR_MIPMAPLEVELMASK_BOTH,
    &t_info);
  float ul_x = fb_info.ul_x * rdp.scale_x + rdp.offset_x;
  float ul_y = fb_info.ul_y * rdp.scale_y + rdp.offset_y;
  float lr_x = fb_info.lr_x * rdp.scale_x + rdp.offset_x;
  float lr_y = fb_info.lr_y * rdp.scale_y + rdp.offset_y;
  float lr_u = (width-1)*scale;
  float lr_v = (height-1)*scale;
  float zero = scale*0.5f;
  // Make the vertices
  VERTEX v[4] = {
    { ul_x, ul_y, 1, 1, zero, zero, zero, zero, {zero, zero, zero, zero} },
    { lr_x, ul_y, 1, 1, lr_u, zero, lr_u, zero, {lr_u, zero, lr_u, zero} },
    { ul_x, lr_y, 1, 1, zero, lr_v, zero, lr_v, {zero, lr_v, zero, lr_v} },
    { lr_x, lr_y, 1, 1, lr_u, lr_v, lr_u, lr_v, {lr_u, lr_v, lr_u, lr_v} }
  };
  grDrawTriangle (&v[0], &v[2], &v[1]);
  grDrawTriangle (&v[2], &v[3], &v[1]);
}
