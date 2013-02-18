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
// Hardware frame buffer emulation
// Dec 2003 created by Gonetz
//
//****************************************************************

#include "Gfx_1.3.h"
#include "TexBuffer.h"
#include "CRC.h"

static TBUFF_COLOR_IMAGE * AllocateTextureBuffer(COLOR_IMAGE & cimage)
{
  TBUFF_COLOR_IMAGE texbuf;
  texbuf.addr = cimage.addr;
  texbuf.end_addr = cimage.addr + ((cimage.width*cimage.height)<<cimage.size>>1);
  texbuf.width = cimage.width;
  texbuf.height = cimage.height;
  texbuf.format = cimage.format;
  texbuf.size = cimage.size;
  texbuf.scr_width = min(cimage.width * rdp.scale_x, settings.scr_res_x);
  float height = min(rdp.vi_height,cimage.height);
  if (cimage.status == ci_copy_self || (cimage.status == ci_copy && cimage.width == rdp.frame_buffers[rdp.main_ci_index].width))
    height = rdp.vi_height;
  texbuf.scr_height = height * rdp.scale_y;
//  texbuf.scr_height = texbuf.height * rdp.scale_y;

  wxUint16 max_size = max((wxUint16)texbuf.scr_width, (wxUint16)texbuf.scr_height);
  if (max_size > voodoo.max_tex_size) //texture size is too large
    return 0;
  wxUint32 tex_size;
  //calculate LOD
  switch ((max_size-1) >> 6)
  {
  case 0:
    texbuf.info.smallLodLog2 = texbuf.info.largeLodLog2 = GR_LOD_LOG2_64;
    tex_size = 64;
    break;
  case 1:
    texbuf.info.smallLodLog2 = texbuf.info.largeLodLog2 = GR_LOD_LOG2_128;
    tex_size = 128;
    break;
  case 2:
  case 3:
    texbuf.info.smallLodLog2 = texbuf.info.largeLodLog2 = GR_LOD_LOG2_256;
    tex_size = 256;
    break;
  case 4:
  case 5:
  case 6:
  case 7:
    texbuf.info.smallLodLog2 = texbuf.info.largeLodLog2 = GR_LOD_LOG2_512;
    tex_size = 512;
    break;
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
    texbuf.info.smallLodLog2 = texbuf.info.largeLodLog2 = GR_LOD_LOG2_1024;
    tex_size = 1024;
    break;
  default:
    texbuf.info.smallLodLog2 = texbuf.info.largeLodLog2 = GR_LOD_LOG2_2048;
    tex_size = 2048;
  }
  //calculate aspect
  if (texbuf.scr_width >= texbuf.scr_height)
  {
    if ((texbuf.scr_width/texbuf.scr_height) >= 2)
    {
      texbuf.info.aspectRatioLog2 = GR_ASPECT_LOG2_2x1;
      texbuf.tex_width = tex_size;
      texbuf.tex_height = tex_size >> 1;
    }
    else
    {
      texbuf.info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
      texbuf.tex_width = texbuf.tex_height = tex_size;
    }
  }
  else
  {
    if ((texbuf.scr_height/texbuf.scr_width) >= 2)
    {
      texbuf.info.aspectRatioLog2 = GR_ASPECT_LOG2_1x2;
      texbuf.tex_width = tex_size >> 1;
      texbuf.tex_height = tex_size;
    }
    else
    {
      texbuf.info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
      texbuf.tex_width = texbuf.tex_height = tex_size;
    }
  }
  if ((cimage.format != 0))// && (cimage.width <= 64))
    texbuf.info.format = GR_TEXFMT_ALPHA_INTENSITY_88;
  else
    texbuf.info.format = GR_TEXFMT_RGB_565;

  texbuf.lr_u = 256.0f * texbuf.scr_width / (float)tex_size;// + 1.0f;
  texbuf.lr_v = 256.0f * texbuf.scr_height / (float)tex_size;// + 1.0f;
  texbuf.tile = 0;
  texbuf.tile_uls = 0;
  texbuf.tile_ult = 0;
  texbuf.u_shift = 0;
  texbuf.v_shift = 0;
  texbuf.drawn = FALSE;
  texbuf.u_scale = texbuf.lr_u / (float)(texbuf.width);
  texbuf.v_scale = texbuf.lr_v / (float)(texbuf.height);
  texbuf.cache = 0;
  texbuf.crc = 0;
  texbuf.t_mem = 0;

  FRDP("\nAllocateTextureBuffer. width: %d, height: %d, scr_width: %f, scr_height: %f, vi_width: %f, vi_height:%f, scale_x: %f, scale_y: %f, lr_u: %f, lr_v: %f, u_scale: %f, v_scale: %f\n", texbuf.width, texbuf.height, texbuf.scr_width, texbuf.scr_height, rdp.vi_width, rdp.vi_height, rdp.scale_x, rdp.scale_y, texbuf.lr_u, texbuf.lr_v, texbuf.u_scale, texbuf.v_scale);

  wxUint32 required = grTexCalcMemRequired(texbuf.info.smallLodLog2, texbuf.info.largeLodLog2,
    texbuf.info.aspectRatioLog2, texbuf.info.format);
  //find free space
  for (int i = 0; i < voodoo.num_tmu; i++)
  {
    wxUint32 available = 0;
    wxUint32 top = 0;
    if (rdp.texbufs[i].count)
    {
      TBUFF_COLOR_IMAGE & t = rdp.texbufs[i].images[rdp.texbufs[i].count - 1];
      if (rdp.read_whole_frame || rdp.motionblur)
      {
        if ((cimage.status == ci_aux) && (rdp.cur_tex_buf == i))
        {
          top = t.tex_addr + t.tex_width * (int)(t.scr_height+1) * 2;
          if (rdp.texbufs[i].end - top < required)
            return 0;
        }
        else
          top = rdp.texbufs[i].end;
      }
      else
        top = t.tex_addr + t.tex_width * t.tex_height * 2;
      available	 = rdp.texbufs[i].end - top;
    }
    else
    {
      available	 = rdp.texbufs[i].end - rdp.texbufs[i].begin;
      top = rdp.texbufs[i].begin;
    }
    if (available >= required)
    {
      rdp.texbufs[i].count++;
      rdp.texbufs[i].clear_allowed = FALSE;
      texbuf.tex_addr = top;
      rdp.cur_tex_buf = i;
      texbuf.tmu = rdp.texbufs[i].tmu;
      rdp.texbufs[i].images[rdp.texbufs[i].count - 1] = texbuf;
      return &(rdp.texbufs[i].images[rdp.texbufs[i].count - 1]);
    }
  }
  //not found. keep recently accessed bank, clear second one
  if (!rdp.texbufs[rdp.cur_tex_buf^1].clear_allowed) //can't clear => can't allocate
    return 0;
  rdp.cur_tex_buf ^= 1;
  rdp.texbufs[rdp.cur_tex_buf].count = 1;
  rdp.texbufs[rdp.cur_tex_buf].clear_allowed = FALSE;
  texbuf.tmu = rdp.texbufs[rdp.cur_tex_buf].tmu;
  texbuf.tex_addr = rdp.texbufs[rdp.cur_tex_buf].begin;
  rdp.texbufs[rdp.cur_tex_buf].images[0] = texbuf;
  return &(rdp.texbufs[rdp.cur_tex_buf].images[0]);
}

int OpenTextureBuffer(COLOR_IMAGE & cimage)
{
  FRDP("OpenTextureBuffer. cur_tex_buf: %d, addr: %08lx, width: %d, height: %d", rdp.cur_tex_buf, cimage.addr, cimage.width, cimage.height);
  if (!fullscreen) return FALSE;

  int found = FALSE, search = TRUE;
  TBUFF_COLOR_IMAGE *texbuf = 0;
  wxUint32 addr = cimage.addr;
  if ((settings.hacks&hack_Banjo2) && cimage.status == ci_copy_self)
    addr = rdp.frame_buffers[rdp.copy_ci_index].addr;
  wxUint32 end_addr = addr + ((cimage.width*cimage.height)<<cimage.size>>1);
  if (rdp.motionblur)
  {
//    if (cimage.format != 0)
//      return FALSE;
    search = FALSE;
  }
  if (rdp.read_whole_frame)
  {
    if (settings.hacks&hack_PMario) //motion blur effects in Paper Mario
    {
      rdp.cur_tex_buf = rdp.acc_tex_buf;
      FRDP("\nread_whole_frame. last allocated bank: %d\n", rdp.acc_tex_buf);
    }
    else
    {
      if (!rdp.texbufs[0].clear_allowed || !rdp.texbufs[1].clear_allowed)
      {
        if (cimage.status == ci_main)
        {
          texbuf = &(rdp.texbufs[rdp.cur_tex_buf].images[0]);
          found = TRUE;
        }
        else
        {
          for (int t = 0; (t < rdp.texbufs[rdp.cur_tex_buf].count) && !found; t++)
          {
            texbuf = &(rdp.texbufs[rdp.cur_tex_buf].images[t]);
            if (addr == texbuf->addr && cimage.width == texbuf->width)
            {
              texbuf->drawn = FALSE;
              found = TRUE;
            }
          }
        }
      }
      search = FALSE;
    }
  }
  if (search)
  {
    for (int i = 0; (i < voodoo.num_tmu) && !found; i++)
    {
      for (int j = 0; (j < rdp.texbufs[i].count) && !found; j++)
      {
        texbuf = &(rdp.texbufs[i].images[j]);
        if (addr == texbuf->addr && cimage.width == texbuf->width)
        {
          //texbuf->height = cimage.height;
          //texbuf->end_addr = end_addr;
          texbuf->drawn = FALSE;
          texbuf->format = (wxUint16)cimage.format;
          if ((cimage.format != 0))
            texbuf->info.format = GR_TEXFMT_ALPHA_INTENSITY_88;
          else
            texbuf->info.format = GR_TEXFMT_RGB_565;
          texbuf->crc = 0;
          texbuf->t_mem = 0;
          texbuf->tile = 0;
          found = TRUE;
          rdp.cur_tex_buf = i;
          rdp.texbufs[i].clear_allowed = FALSE;
        }
        else //check intersection
        {
          if (!((end_addr <= texbuf->addr) || (addr >= texbuf->end_addr))) //intersected, remove
          {
            grRenderBuffer( GR_BUFFER_TEXTUREBUFFER_EXT );
            grTextureBufferExt( texbuf->tmu, texbuf->tex_addr, texbuf->info.smallLodLog2, texbuf->info.largeLodLog2,
              texbuf->info.aspectRatioLog2, texbuf->info.format, GR_MIPMAPLEVELMASK_BOTH );
            grDepthMask (FXFALSE);
            grBufferClear (0, 0, 0xFFFF);
            grDepthMask (FXTRUE);
            grRenderBuffer( GR_BUFFER_BACKBUFFER );
            rdp.texbufs[i].count--;
            if (j < rdp.texbufs[i].count)
              memcpy(&(rdp.texbufs[i].images[j]), &(rdp.texbufs[i].images[j+1]), sizeof(TBUFF_COLOR_IMAGE)*(rdp.texbufs[i].count-j));
          }
        }
      }
    }
  }
  else
  {
    LRDP("  not searched");
  }

  if (!found)
  {
    LRDP("  not found");
    texbuf = AllocateTextureBuffer(cimage);
  }
  else
  {
    LRDP("  found");
  }

  if (!texbuf)
  {
    LRDP("  KO\n");
    return FALSE;
  }

  rdp.acc_tex_buf = rdp.cur_tex_buf;
  rdp.cur_image = texbuf;
  grRenderBuffer( GR_BUFFER_TEXTUREBUFFER_EXT );
  grTextureBufferExt( rdp.cur_image->tmu, rdp.cur_image->tex_addr, rdp.cur_image->info.smallLodLog2, rdp.cur_image->info.largeLodLog2,
    rdp.cur_image->info.aspectRatioLog2, rdp.cur_image->info.format, GR_MIPMAPLEVELMASK_BOTH );
  ///*
  if (rdp.cur_image->clear && (settings.frame_buffer&fb_hwfbe_buf_clear) && cimage.changed)
  {
    rdp.cur_image->clear = FALSE;
    grDepthMask (FXFALSE);
    grBufferClear (0, 0, 0xFFFF);
    grDepthMask (FXTRUE);
  }
  //*/
  //  memset(gfx.RDRAM+cimage.addr, 0, cimage.width*cimage.height*cimage.size);
  FRDP("  texaddr: %08lx, tex_width: %d, tex_height: %d, cur_tex_buf: %d, texformat: %d, motionblur: %d\n", rdp.cur_image->tex_addr, rdp.cur_image->tex_width, rdp.cur_image->tex_height, rdp.cur_tex_buf, rdp.cur_image->info.format, rdp.motionblur);
  if (!rdp.offset_x_bak)
  {
    rdp.offset_x_bak = rdp.offset_x;
    rdp.offset_x = 0;
  }
  if (!rdp.offset_y_bak)
  {
    rdp.offset_y_bak = rdp.offset_y;
    rdp.offset_y = 0;
  }
  rdp.update |= UPDATE_VIEWPORT | UPDATE_SCISSOR;
  return TRUE;
}

static GrTextureFormat_t TexBufSetupCombiner(int force_rgb = FALSE)
{
  grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
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
//  grConstantColorValue (0xFFFFFFFF);
  grAlphaBlendFunction (GR_BLEND_ONE,	// use alpha compare, but not T0 alpha
    GR_BLEND_ZERO,
    GR_BLEND_ONE,
    GR_BLEND_ZERO);
  grClipWindow (0, 0, settings.scr_res_x, settings.scr_res_y);
  grDepthBufferFunction (GR_CMP_ALWAYS);
  grDepthMask (FXFALSE);
  grCullMode (GR_CULL_DISABLE);
  grFogMode (GR_FOG_DISABLE);
  GrTextureFormat_t buf_format = (rdp.tbuff_tex) ? rdp.tbuff_tex->info.format : GR_TEXFMT_RGB_565;
  GrCombineFunction_t color_source = GR_COMBINE_FUNCTION_LOCAL;
  if  (!force_rgb && rdp.black_ci_index > 0 && rdp.black_ci_index <= rdp.copy_ci_index)
  {
    color_source = GR_COMBINE_FUNCTION_LOCAL_ALPHA;
    buf_format = GR_TEXFMT_ALPHA_INTENSITY_88;
  }
  if (rdp.tbuff_tex->tmu == GR_TMU0)
  {
    grTexCombine( GR_TMU1,
      GR_COMBINE_FUNCTION_NONE,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_NONE,
      GR_COMBINE_FACTOR_NONE,
      FXFALSE,
      FXFALSE );
    grTexCombine( GR_TMU0,
      color_source,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_ZERO,
      GR_COMBINE_FACTOR_NONE,
      FXFALSE,
      FXTRUE );
  }
  else
  {
    grTexCombine( GR_TMU1,
      color_source,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_ZERO,
      GR_COMBINE_FACTOR_NONE,
      FXFALSE,
      FXTRUE );
    grTexCombine( GR_TMU0,
      GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      FXFALSE,
      FXFALSE );
  }
  return buf_format;
}

int CloseTextureBuffer(int draw)
{
  if (!fullscreen || !rdp.cur_image)
  {
    LRDP("CloseTextureBuffer KO\n");
    return FALSE;
  }
  grRenderBuffer( GR_BUFFER_BACKBUFFER );
  rdp.offset_x = rdp.offset_x_bak;
  rdp.offset_y = rdp.offset_y_bak;
  rdp.offset_x_bak = rdp.offset_y_bak = 0;
  rdp.update |= UPDATE_VIEWPORT | UPDATE_SCISSOR;
  if (!draw)
  {
    LRDP("CloseTextureBuffer no draw, OK\n");
    rdp.cur_image = 0;
    return TRUE;
  }
  rdp.tbuff_tex = rdp.cur_image;
  rdp.cur_image = 0;
  rdp.tbuff_tex->info.format = TexBufSetupCombiner();
  float zero = 0.0f;
  float ul_x = rdp.offset_x;
  float ul_y = rdp.offset_y;
  float lr_x = rdp.tbuff_tex->scr_width + rdp.offset_x;
  float lr_y = rdp.tbuff_tex->scr_height + rdp.offset_y;
  float lr_u = rdp.tbuff_tex->lr_u;
  float lr_v = rdp.tbuff_tex->lr_v;
  FRDP("lr_x: %f, lr_y: %f, lr_u: %f, lr_v: %f\n", lr_x, lr_y, lr_u, lr_v);


  // Make the vertices
  VERTEX v[4] = {
    { ul_x, ul_y, 1, 1, zero, zero, zero, zero, {zero, zero, zero, zero} },
    { lr_x, ul_y, 1, 1, lr_u, zero, lr_u, zero, {lr_u, zero, lr_u, zero} },
    { ul_x, lr_y, 1, 1, zero, lr_v, zero, lr_v, {zero, lr_v, zero, lr_v} },
    { lr_x, lr_y, 1, 1, lr_u, lr_v, lr_u, lr_v, {lr_u, lr_v, lr_u, lr_v} }
  };

  grTexSource( rdp.tbuff_tex->tmu, rdp.tbuff_tex->tex_addr, GR_MIPMAPLEVELMASK_BOTH, &(rdp.tbuff_tex->info) );
  grClipWindow (0, 0, settings.res_x, settings.res_y);
  grDrawTriangle (&v[0], &v[2], &v[1]);
  grDrawTriangle (&v[2], &v[3], &v[1]);
  rdp.update |= UPDATE_ZBUF_ENABLED | UPDATE_COMBINE | UPDATE_TEXTURE | UPDATE_ALPHA_COMPARE;
  if (settings.fog && (rdp.flags & FOG_ENABLED))
  {
    grFogMode (GR_FOG_WITH_TABLE_ON_FOGCOORD_EXT);
  }
  LRDP("CloseTextureBuffer draw, OK\n");
  rdp.tbuff_tex = 0;
  return TRUE;
}

int CopyTextureBuffer(COLOR_IMAGE & fb_from, COLOR_IMAGE & fb_to)
{
  if (!fullscreen)
    return FALSE;
  FRDP("CopyTextureBuffer from %08x to %08x\n", fb_from.addr, fb_to.addr);
  if (rdp.cur_image)
  {
    rdp.cur_image->crc = 0;
    if (rdp.cur_image->addr == fb_to.addr)
      return CloseTextureBuffer(TRUE);
    rdp.tbuff_tex = rdp.cur_image;
  }
  else if (!FindTextureBuffer(fb_from.addr, (wxUint16)fb_from.width))
  {
    LRDP("Can't find 'from' buffer.\n");
    return FALSE;
  }
  if (!OpenTextureBuffer(fb_to))
  {
    LRDP("Can't open new buffer.\n");
    return CloseTextureBuffer(TRUE);
  }
  rdp.tbuff_tex->crc = 0;
  GrTextureFormat_t buf_format = rdp.tbuff_tex->info.format;
  rdp.tbuff_tex->info.format = GR_TEXFMT_RGB_565;
  TexBufSetupCombiner(TRUE);
  float ul_x = 0.0f;
  float ul_y = 0.0f;
  float lr_x = rdp.tbuff_tex->scr_width;
  float lr_y = rdp.tbuff_tex->scr_height;
  float zero = 0.0f;
  float lr_u = rdp.tbuff_tex->lr_u;
  float lr_v = rdp.tbuff_tex->lr_v;
  FRDP("lr_x: %f, lr_y: %f\n", lr_x, lr_y);


  // Make the vertices
  VERTEX v[4] = {
    { ul_x, ul_y, 1, 1, zero, zero, zero, zero, {zero, zero, zero, zero} },
    { lr_x, ul_y, 1, 1, lr_u, zero, lr_u, zero, {lr_u, zero, lr_u, zero} },
    { ul_x, lr_y, 1, 1, zero, lr_v, zero, lr_v, {zero, lr_v, zero, lr_v} },
    { lr_x, lr_y, 1, 1, lr_u, lr_v, lr_u, lr_v, {lr_u, lr_v, lr_u, lr_v} }
  };

  grTexSource( rdp.tbuff_tex->tmu, rdp.tbuff_tex->tex_addr, GR_MIPMAPLEVELMASK_BOTH, &(rdp.tbuff_tex->info) );
  grDrawTriangle (&v[0], &v[2], &v[1]);
  grDrawTriangle (&v[2], &v[3], &v[1]);
  grRenderBuffer( GR_BUFFER_BACKBUFFER );
  rdp.offset_x = rdp.offset_x_bak;
  rdp.offset_y = rdp.offset_y_bak;
  rdp.offset_x_bak = rdp.offset_y_bak = 0;
  AddOffset(v, 4);
  grClipWindow (0, 0, settings.res_x, settings.res_y);
  grDrawTriangle (&v[0], &v[2], &v[1]);
  grDrawTriangle (&v[2], &v[3], &v[1]);
  rdp.tbuff_tex->info.format = buf_format;

  rdp.update |= UPDATE_ZBUF_ENABLED | UPDATE_COMBINE | UPDATE_TEXTURE | UPDATE_ALPHA_COMPARE;
  rdp.update |= UPDATE_VIEWPORT | UPDATE_SCISSOR;
  if (settings.fog && (rdp.flags & FOG_ENABLED))
    grFogMode (GR_FOG_WITH_TABLE_ON_FOGCOORD_EXT);
  LRDP("CopyTextureBuffer draw, OK\n");
  rdp.tbuff_tex = 0;
  rdp.cur_image = 0;
  return TRUE;
}

int CopyDepthBuffer()
{
  if (!fullscreen)
    return FALSE;
  LRDP("CopyDepthBuffer. ");
  float bound = 1024.0f;
  GrLOD_t LOD = GR_LOD_LOG2_1024;
  if (settings.scr_res_x > 1024)
  {
    bound = 2048.0f;
    LOD = GR_LOD_LOG2_2048;
  }
  rdp.tbuff_tex = &(rdp.texbufs[0].images[0]);
  rdp.tbuff_tex->tmu = rdp.texbufs[0].tmu;
  rdp.tbuff_tex->info.format = GR_TEXFMT_RGB_565;
  rdp.tbuff_tex->info.smallLodLog2 = rdp.tbuff_tex->info.largeLodLog2 = LOD;
  rdp.tbuff_tex->info.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
  TexBufSetupCombiner(TRUE);
  float ul_x = 0.0f;
  float ul_y = 0.0f;
  float lr_x = bound;
  float lr_y = bound;
  float zero = 0.0f;
  float lr_u = 255.5f;
  float lr_v = 255.5f;
  FRDP("lr_x: %f, lr_y: %f\n", lr_x, lr_y);


  // Make the vertices
  VERTEX v[4] = {
    { ul_x, ul_y, 1, 1, zero, zero, zero, zero, {zero, zero, zero, zero} },
    { lr_x, ul_y, 1, 1, lr_u, zero, lr_u, zero, {lr_u, zero, lr_u, zero} },
    { ul_x, lr_y, 1, 1, zero, lr_v, zero, lr_v, {zero, lr_v, zero, lr_v} },
    { lr_x, lr_y, 1, 1, lr_u, lr_v, lr_u, lr_v, {lr_u, lr_v, lr_u, lr_v} }
  };

  grAuxBufferExt( GR_BUFFER_AUXBUFFER );
  grTexSource( rdp.texbufs[0].tmu, rdp.texbufs[0].begin, GR_MIPMAPLEVELMASK_BOTH, &(rdp.tbuff_tex->info) );
  grRenderBuffer( GR_BUFFER_TEXTUREBUFFER_EXT );
  grTextureBufferExt( rdp.texbufs[1].tmu, rdp.texbufs[1].begin, LOD, LOD,
    GR_ASPECT_LOG2_1x1, GR_TEXFMT_RGB_565, GR_MIPMAPLEVELMASK_BOTH );
  grDrawTriangle (&v[0], &v[2], &v[1]);
  grDrawTriangle (&v[2], &v[3], &v[1]);
  grRenderBuffer( GR_BUFFER_BACKBUFFER );
  grTextureAuxBufferExt( rdp.texbufs[1].tmu, rdp.texbufs[1].begin, LOD, LOD,
    GR_ASPECT_LOG2_1x1, GR_TEXFMT_RGB_565, GR_MIPMAPLEVELMASK_BOTH );
  grAuxBufferExt( GR_BUFFER_TEXTUREAUXBUFFER_EXT );

  rdp.update |= UPDATE_ZBUF_ENABLED | UPDATE_COMBINE | UPDATE_TEXTURE | UPDATE_ALPHA_COMPARE;
  if (settings.fog && (rdp.flags & FOG_ENABLED))
    grFogMode (GR_FOG_WITH_TABLE_ON_FOGCOORD_EXT);
  LRDP("CopyDepthBuffer draw, OK\n");
  rdp.tbuff_tex = 0;
  return TRUE;
}

int SwapTextureBuffer()
{
  if (!fullscreen || !rdp.tbuff_tex)
    return FALSE;
  LRDP("SwapTextureBuffer.");
  COLOR_IMAGE ci;
  ci.addr = rdp.tbuff_tex->addr;
  ci.format = rdp.tbuff_tex->format;
  ci.width = rdp.tbuff_tex->width;
  ci.height = rdp.tbuff_tex->height;
  ci.size = 2;
  ci.status = ci_main;
  ci.changed = FALSE;
  TBUFF_COLOR_IMAGE * texbuf = AllocateTextureBuffer(ci);
  if (!texbuf)
  {
    LRDP("Failed!\n");
    return FALSE;
  }
  TexBufSetupCombiner();

  float ul_x = 0.0f;
  float ul_y = 0.0f;
  float lr_x = rdp.tbuff_tex->scr_width;
  float lr_y = rdp.tbuff_tex->scr_height;
  float zero = 0.0f;
  float lr_u = rdp.tbuff_tex->lr_u;
  float lr_v = rdp.tbuff_tex->lr_v;

  // Make the vertices
  VERTEX v[4] = {
    { ul_x, ul_y, 1, 1, zero, zero, zero, zero, {zero, zero, zero, zero} },
    { lr_x, ul_y, 1, 1, lr_u, zero, lr_u, zero, {lr_u, zero, lr_u, zero} },
    { ul_x, lr_y, 1, 1, zero, lr_v, zero, lr_v, {zero, lr_v, zero, lr_v} },
    { lr_x, lr_y, 1, 1, lr_u, lr_v, lr_u, lr_v, {lr_u, lr_v, lr_u, lr_v} }
  };

  grTexSource( rdp.tbuff_tex->tmu, rdp.tbuff_tex->tex_addr, GR_MIPMAPLEVELMASK_BOTH, &(rdp.tbuff_tex->info) );
  texbuf->tile_uls = rdp.tbuff_tex->tile_uls;
  texbuf->tile_ult = rdp.tbuff_tex->tile_ult;
  texbuf->v_shift = rdp.tbuff_tex->v_shift;
  grRenderBuffer( GR_BUFFER_TEXTUREBUFFER_EXT );
  grTextureBufferExt( texbuf->tmu, texbuf->tex_addr, texbuf->info.smallLodLog2, texbuf->info.largeLodLog2,
    texbuf->info.aspectRatioLog2, texbuf->info.format, GR_MIPMAPLEVELMASK_BOTH );
  grDrawTriangle (&v[0], &v[2], &v[1]);
  grDrawTriangle (&v[2], &v[3], &v[1]);
  rdp.texbufs[rdp.tbuff_tex->tmu].clear_allowed = TRUE;
  rdp.texbufs[rdp.tbuff_tex->tmu].count = 0;
  texbuf->tile_uls = rdp.tbuff_tex->tile_uls;
  texbuf->tile_ult = rdp.tbuff_tex->tile_ult;
  texbuf->u_shift = rdp.tbuff_tex->u_shift;
  texbuf->v_shift = rdp.tbuff_tex->v_shift;
  rdp.tbuff_tex = texbuf;
  if (rdp.cur_image)
  {
    grTextureBufferExt( rdp.cur_image->tmu, rdp.cur_image->tex_addr, rdp.cur_image->info.smallLodLog2, rdp.cur_image->info.largeLodLog2,
    rdp.cur_image->info.aspectRatioLog2, rdp.cur_image->info.format, GR_MIPMAPLEVELMASK_BOTH );
  }
  else
  {
    grRenderBuffer( GR_BUFFER_BACKBUFFER );
    rdp.offset_x = rdp.offset_x_bak;
    rdp.offset_y = rdp.offset_y_bak;
    rdp.offset_x_bak = rdp.offset_y_bak = 0;
    rdp.update |= UPDATE_VIEWPORT | UPDATE_SCISSOR;
  }
  rdp.update |= UPDATE_ZBUF_ENABLED | UPDATE_COMBINE | UPDATE_TEXTURE | UPDATE_ALPHA_COMPARE;
  if (settings.fog && (rdp.flags & FOG_ENABLED))
  {
    grFogMode (GR_FOG_WITH_TABLE_ON_FOGCOORD_EXT);
  }
  LRDP("SwapTextureBuffer draw, OK\n");
  return TRUE;
}

static wxUint32 CalcCRC(TBUFF_COLOR_IMAGE * pTCI)
{
  wxUint32 result = 0;
  if ((settings.frame_buffer&fb_ref) > 0)
    pTCI->crc = 0; //Since fb content changes each frame, crc check is meaningless.
  else if (settings.fb_crc_mode == SETTINGS::fbcrcFast)
    result = *((wxUint32*)(gfx.RDRAM + pTCI->addr + (pTCI->end_addr-pTCI->addr)/2));
  else if (settings.fb_crc_mode == SETTINGS::fbcrcSafe)
  {
    wxUint8 * pSrc = gfx.RDRAM + pTCI->addr;
    const wxUint32 nSize = pTCI->end_addr-pTCI->addr;
    result = CRC32(0xFFFFFFFF, pSrc, 32);
    result = CRC32(result, pSrc + (nSize>>1), 32);
    result = CRC32(result, pSrc + nSize - 32, 32);
  }
  return result;
}

int FindTextureBuffer(wxUint32 addr, wxUint16 width)
{
  if (rdp.skip_drawing)
    return FALSE;
  FRDP("FindTextureBuffer. addr: %08lx, width: %d, scale_x: %f\n", addr, width, rdp.scale_x);
  int found = FALSE;
  wxUint32 shift = 0;
  for (int i = 0; i < voodoo.num_tmu && !found; i++)
  {
    wxUint8 index = rdp.cur_tex_buf^i;
    for (int j = 0; j < rdp.texbufs[index].count && !found; j++)
    {
      rdp.tbuff_tex = &(rdp.texbufs[index].images[j]);
      if(addr >= rdp.tbuff_tex->addr && addr < rdp.tbuff_tex->end_addr)// && rdp.timg.format == 0)
      {
        bool bCorrect;
        if (rdp.tbuff_tex->crc == 0)
        {
          rdp.tbuff_tex->crc = CalcCRC(rdp.tbuff_tex);
          bCorrect = width == 1 || rdp.tbuff_tex->width == width || (rdp.tbuff_tex->width > 320 && rdp.tbuff_tex->width == width*2);
        }
        else
          bCorrect = rdp.tbuff_tex->crc == CalcCRC(rdp.tbuff_tex);
        if (bCorrect)
        {
          shift = addr - rdp.tbuff_tex->addr;
          // if (!rdp.motionblur)
          if (!rdp.cur_image)
            rdp.cur_tex_buf = index;
          found = TRUE;
          //    FRDP("FindTextureBuffer, found in TMU%d buffer: %d\n", rdp.tbuff_tex->tmu, j);
        }
        else //new texture is loaded into this place, texture buffer is not valid anymore
        {
          rdp.texbufs[index].count--;
          if (j < rdp.texbufs[index].count)
            memcpy(&(rdp.texbufs[index].images[j]), &(rdp.texbufs[index].images[j+1]), sizeof(TBUFF_COLOR_IMAGE)*(rdp.texbufs[index].count-j));
        }
      }
    }
  }
  if (found)
  {
    rdp.tbuff_tex->tile_uls = 0;
    rdp.tbuff_tex->tile_ult = 0;
    if (shift > 0)
    {
      shift >>= 1;
      rdp.tbuff_tex->v_shift = shift / rdp.tbuff_tex->width;
      rdp.tbuff_tex->u_shift = shift % rdp.tbuff_tex->width;
    }
    else
    {
      rdp.tbuff_tex->v_shift = 0;
      rdp.tbuff_tex->u_shift = 0;
    }
    FRDP("FindTextureBuffer, found, u_shift: %d,  v_shift: %d, format: %s\n", rdp.tbuff_tex->u_shift, rdp.tbuff_tex->v_shift, str_format[rdp.tbuff_tex->format]);
    //FRDP("Buffer, addr=%08lx, end_addr=%08lx, width: %d, height: %d\n", rdp.tbuff_tex->addr, rdp.tbuff_tex->end_addr, rdp.tbuff_tex->width, rdp.tbuff_tex->height);
    return TRUE;
  }
  rdp.tbuff_tex = 0;
  LRDP("FindTextureBuffer, not found\n");
  return FALSE;
}
