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
#include "Util.h"
#include "Debugger.h"

GLIDE64_DEBUGGER _debugger;

#define SX(x) ((x)*rdp.scale_1024)
#define SY(x) ((x)*rdp.scale_768)

#ifdef COLORED_DEBUGGER
#define COL_CATEGORY()  grConstantColorValue(0xD288F400)
#define COL_UCC()   grConstantColorValue(0xFF000000)
#define COL_CC()    grConstantColorValue(0x88C3F400)
#define COL_UAC()   grConstantColorValue(0xFF808000)
#define COL_AC()    grConstantColorValue(0x3CEE5E00)
#define COL_TEXT()    grConstantColorValue(0xFFFFFF00)
#define COL_SEL(x)    grConstantColorValue((x)?0x00FF00FF:0x800000FF)
#else
#define COL_CATEGORY()
#define COL_UCC()
#define COL_CC()
#define COL_UAC()
#define COL_AC()
#define COL_TEXT()
#define COL_SEL(x)
#endif

#define COL_GRID    0xFFFFFF80

int  grid = 0;
static const char *tri_type[4] = { "TRIANGLE", "TEXRECT", "FILLRECT", "BACKGROUND" };

//Platform-specific stuff
#ifndef WIN32
typedef struct dbgPOINT {
   int x;
   int y;
} POINT;
#endif
void DbgCursorPos(POINT * pt)
{
#ifdef __WINDOWS__
  GetCursorPos (pt);
#else //!todo find a way to get cursor position on Unix
  pt->x = pt->y = 0;
#endif
}

//
// debug_init - initialize the debugger
//

void debug_init ()
{
  _debugger.capture = 0;
  _debugger.selected = SELECTED_TRI;
  _debugger.screen = NULL;
  _debugger.tri_list = NULL;
  _debugger.tri_last = NULL;
  _debugger.tri_sel = NULL;
  _debugger.tmu = 0;

  _debugger.tex_scroll = 0;
  _debugger.tex_sel = 0;

  _debugger.draw_mode = 0;
}

//
// debug_cacheviewer - views the debugger's cache
//

void debug_cacheviewer ()
{
  grCullMode (GR_CULL_DISABLE);

  int i;
  for (i=0; i<2; i++)
  {
    grTexFilterMode (i,
      (settings.filter_cache)?GR_TEXTUREFILTER_BILINEAR:GR_TEXTUREFILTER_POINT_SAMPLED,
      (settings.filter_cache)?GR_TEXTUREFILTER_BILINEAR:GR_TEXTUREFILTER_POINT_SAMPLED);
    grTexClampMode (i,
      GR_TEXTURECLAMP_CLAMP,
      GR_TEXTURECLAMP_CLAMP);
  }

  switch (_debugger.draw_mode)
  {
  case 0:
    grColorCombine (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE,
      FXFALSE);
    grAlphaCombine (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE,
      FXFALSE);
    break;
  case 1:
    grColorCombine (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE,
      FXFALSE);
    grAlphaCombine (GR_COMBINE_FUNCTION_LOCAL,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_NONE,
      FXFALSE);
    grConstantColorValue (0xFFFFFFFF);
    break;
  case 2:
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
    grConstantColorValue (0xFFFFFFFF);
  }

  if (_debugger.tmu == 1)
  {
    grTexCombine (GR_TMU1,
      GR_COMBINE_FUNCTION_LOCAL,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_LOCAL,
      GR_COMBINE_FACTOR_NONE,
      FXFALSE,
      FXFALSE);

    grTexCombine (GR_TMU0,
      GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      FXFALSE,
      FXFALSE);
  }
  else
  {
    grTexCombine (GR_TMU0,
      GR_COMBINE_FUNCTION_LOCAL,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_LOCAL,
      GR_COMBINE_FACTOR_NONE,
      FXFALSE,
      FXFALSE);
  }

  grAlphaBlendFunction (GR_BLEND_SRC_ALPHA,
    GR_BLEND_ONE_MINUS_SRC_ALPHA,
    GR_BLEND_ONE,
    GR_BLEND_ZERO);

  // Draw texture memory
  for (i=0; i<4; i++)
  {
    for (wxUint32 x=0; x<16; x++)
    {
      wxUint32 y = i+_debugger.tex_scroll;
      if (x+y*16 >= (wxUint32)rdp.n_cached[_debugger.tmu]) break;
      CACHE_LUT * cache = voodoo.tex_UMA?rdp.cache[0]:rdp.cache[_debugger.tmu];

      VERTEX v[4] = {
          { SX(x*64.0f), SY(512+64.0f*i), 1, 1,       0, 0, 0, 0, {0, 0, 0, 0} },
          { SX(x*64.0f+64.0f*cache[x+y*16].scale_x), SY(512+64.0f*i), 1, 1,    255*cache[x+y*16].scale_x, 0, 0, 0, {0, 0, 0, 0} },
          { SX(x*64.0f), SY(512+64.0f*i+64.0f*cache[x+y*16].scale_y), 1, 1,    0, 255*cache[x+y*16].scale_y, 0, 0, {0, 0, 0, 0} },
          { SX(x*64.0f+64.0f*cache[x+y*16].scale_x), SY(512+64.0f*i+64.0f*cache[x+y*16].scale_y), 1, 1, 255*cache[x+y*16].scale_x, 255*cache[x+y*16].scale_y, 0, 0, {0, 0, 0, 0} }
          };
      for
      (int i=0; i<4; i++)
      {
        v[i].u1 = v[i].u0;
        v[i].v1 = v[i].v0;
      }

      ConvertCoordsConvert (v, 4);

      grTexSource(_debugger.tmu,
        voodoo.tex_min_addr[_debugger.tmu] + cache[x+y*16].tmem_addr,
        GR_MIPMAPLEVELMASK_BOTH,
        &cache[x+y*16].t_info);

      grDrawTriangle (&v[2], &v[1], &v[0]);
      grDrawTriangle (&v[2], &v[3], &v[1]);
    }
  }

}

//
// debug_capture - does a frame capture event (for debugging)
//

void debug_capture ()
{
  wxUint32 i,j;

  if (_debugger.tri_list == NULL) goto END;
  _debugger.tri_sel = _debugger.tri_list;
  _debugger.selected = SELECTED_TRI;

  // Connect the list
  _debugger.tri_last->pNext = _debugger.tri_list;

  while (!CheckKeyPressed(G64_VK_INSERT, 0x0001)) //INSERT
  {
    // Check for clicks
    if (CheckKeyPressed(G64_VK_LBUTTON, 0x0001)) //LBUTTON
    {
      POINT pt;
      DbgCursorPos(&pt);

      //int diff = settings.scr_res_y-settings.res_y;

      if (pt.y <= (int)settings.res_y)
      {
        int x = pt.x;
        int y = pt.y;//settings.res_y - (pt.y - diff);

        TRI_INFO *start;
        TRI_INFO *tri;
        if (_debugger.tri_sel == NULL) tri = _debugger.tri_list, start = _debugger.tri_list;
        else tri = _debugger.tri_sel->pNext, start = _debugger.tri_sel;

        // Select a triangle (start from the currently selected one)
        do {
          if (tri->v[0].x == tri->v[1].x &&
            tri->v[0].y == tri->v[1].y)
          {
            tri = tri->pNext;
            continue;
          }

          for (i=0; i<tri->nv; i++)
          {
            j=i+1;
            if (j==tri->nv) j=0;

            if ((y-tri->v[i].y)*(tri->v[j].x-tri->v[i].x) -
              (x-tri->v[i].x)*(tri->v[j].y-tri->v[i].y) < 0)
              break;    // It's outside
          }

          if (i==tri->nv) // all lines passed
          {
            _debugger.tri_sel = tri;
            break;
          }

          for (i=0; i<tri->nv; i++)
          {
            j=i+1;
            if (j==tri->nv) j=0;

            if ((y-tri->v[i].y)*(tri->v[j].x-tri->v[i].x) -
              (x-tri->v[i].x)*(tri->v[j].y-tri->v[i].y) > 0)
              break;    // It's outside
          }

          if (i==tri->nv) // all lines passed
          {
            _debugger.tri_sel = tri;
            break;
          }

          tri = tri->pNext;
        } while (tri != start);
      }
      else
      {
        // on a texture
        _debugger.tex_sel = (((wxUint32)((pt.y-SY(512))/SY(64))+_debugger.tex_scroll)*16) +
          (wxUint32)(pt.x/SX(64));
      }
    }

    debug_keys ();

    grBufferClear (0, 0, 0xFFFF);

    // Copy the screen capture back to the screen:
    grLfbWriteRegion(GR_BUFFER_BACKBUFFER,
      (wxUint32)rdp.offset_x,
      (wxUint32)rdp.offset_y,
      GR_LFB_SRC_FMT_565,
      settings.res_x,
      settings.res_y,
      FXFALSE,
      settings.res_x<<1,
      _debugger.screen);

    // Do the cacheviewer
    debug_cacheviewer ();

    // **
    // 3/16/02: Moved texture viewer out of loop, remade it.  Now it's simpler, and
    //   supports TMU1. [Dave2001]
    // Original by Gugaman

    CACHE_LUT * cache = voodoo.tex_UMA?rdp.cache[0]:rdp.cache[_debugger.tmu];
    if (_debugger.page == PAGE_TEX_INFO)
    {
      grTexSource(_debugger.tmu,
        voodoo.tex_min_addr[_debugger.tmu] + cache[_debugger.tex_sel].tmem_addr,
        GR_MIPMAPLEVELMASK_BOTH,
        &cache[_debugger.tex_sel].t_info);

#ifdef SHOW_FULL_TEXVIEWER
      float scx = 1.0f;
      float scy = 1.0f;
#else
      float scx = cache[_debugger.tex_sel].scale_x;
      float scy = cache[_debugger.tex_sel].scale_y;
#endif
      VERTEX v[4] = {
              { SX(704.0f), SY(221.0f), 1, 1, 0, 0,  0, 0, {0, 0, 0, 0} },
              { SX(704.0f+256.0f*scx), SY(221.0f), 1, 1, 255*scx, 0, 255*scx, 0, {0, 0, 0, 0} },
              { SX(704.0f), SY(221.0f+256.0f*scy), 1, 1, 0, 255*scy, 0, 255*scy, {0, 0, 0, 0} },
              { SX(704.0f+256.0f*scx), SY(221.0f+256.0f*scy), 1, 1, 255*scx, 255*scy, 255*scx, 255*scy, {0, 0, 0, 0} }
              };
      ConvertCoordsConvert (v, 4);
      VERTEX *varr[4] = { &v[0], &v[1], &v[2], &v[3] };
      grDrawVertexArray (GR_TRIANGLE_STRIP, 4, varr);
    }

    // **

    grTexFilterMode (GR_TMU0,
      GR_TEXTUREFILTER_BILINEAR,
      GR_TEXTUREFILTER_BILINEAR);

    grColorCombine (GR_COMBINE_FUNCTION_LOCAL,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_NONE,
      FXFALSE);

    grAlphaCombine (GR_COMBINE_FUNCTION_LOCAL,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_NONE,
      FXFALSE);

    grConstantColorValue (0x0000FFFF);

    VERTEX *v[8];
    if (_debugger.tri_sel)
    {
      // Draw the outline around the selected triangle
      for (i=0; i<_debugger.tri_sel->nv; i++)
      {
        j=i+1;
        if (j>=_debugger.tri_sel->nv) j=0;

        grDrawLine (&_debugger.tri_sel->v[i], &_debugger.tri_sel->v[j]);

        v[i] = &_debugger.tri_sel->v[i];
      }
    }

    // and the selected texture
    wxUint32 t_y = ((_debugger.tex_sel & 0xFFFFFFF0) >> 4) - _debugger.tex_scroll;
    wxUint32 t_x = _debugger.tex_sel & 0xF;
    VERTEX vt[4] = {
      { SX(t_x*64.0f), SY(512+64.0f*t_y), 1, 1 },
      { SX(t_x*64.0f+64.0f), SY(512+64.0f*t_y), 1, 1 },
      { SX(t_x*64.0f), SY(512+64.0f*t_y+64.0f), 1, 1 },
      { SX(t_x*64.0f+64.0f), SY(512+64.0f*t_y+64.0f), 1, 1 } };
    if (t_y < 4)
    {
      grDrawLine (&vt[0], &vt[1]);
      grDrawLine (&vt[1], &vt[3]);
      grDrawLine (&vt[3], &vt[2]);
      grDrawLine (&vt[2], &vt[0]);
    }

    grConstantColorValue (0xFF000020);

    if (t_y < 4)
    {
      grDrawTriangle (&vt[2], &vt[1], &vt[0]);
      grDrawTriangle (&vt[2], &vt[3], &vt[1]);
    }

    if (_debugger.tri_sel)
      grDrawVertexArray (GR_TRIANGLE_FAN, _debugger.tri_sel->nv, &v);

    // Draw the outline of the cacheviewer
    if (_debugger.page == PAGE_TEX_INFO)
    {
      float scx = cache[_debugger.tex_sel].scale_x;
      float scy = cache[_debugger.tex_sel].scale_y;

      // And the grid
      if (grid)
      {
        grConstantColorValue (COL_GRID);

        float scale_y = (256.0f * scy) / (float)cache[_debugger.tex_sel].height;
        for (int y=0; y<=(int)cache[_debugger.tex_sel].height; y++)
        {
          float y_val = SY(221.0f+y*scale_y);
          VERTEX vh[2] = {
            { SX(704.0f), y_val, 1, 1 },
            { SX(704.0f+255.0f*scx), y_val, 1, 1 } };
          grDrawLine (&vh[0], &vh[1]);
        }

        float scale_x = (256.0f * scx) / (float)cache[_debugger.tex_sel].width;
        for (int x=0; x<=(int)cache[_debugger.tex_sel].width; x++)
        {
          float x_val = SX(704.0f+x*scale_x);
          VERTEX vv[2] = {
            { x_val, SX(221.0f), 1, 1 },
            { x_val, SX(221.0f+256.0f*scy), 1, 1 } };
          grDrawLine (&vv[0], &vv[1]);
        }
      }
    }

    grTexCombine (GR_TMU0,
        GR_COMBINE_FUNCTION_LOCAL,
        GR_COMBINE_FACTOR_NONE,
        GR_COMBINE_FUNCTION_LOCAL,
        GR_COMBINE_FACTOR_NONE,
        FXFALSE,
        FXFALSE);

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

    grConstantColorValue (0xFFFFFF00);

    // Output the information about the selected triangle
    grTexSource(GR_TMU0,      // Text
      voodoo.tex_min_addr[_debugger.tmu]+ offset_font,
      GR_MIPMAPLEVELMASK_BOTH,
      &fontTex);

    static const char *cycle_mode_s[4] = { "1 cycle (0)", "2 cycle (1)", "copy (2)", "fill (3)" };

#define OUTPUT(fmt,other) output(642,(float)i,1,fmt,other); i-=16;
#define OUTPUT1(fmt,other,other1) output(642,(float)i,1,fmt,other,other1); i-=16;
#define OUTPUT_(fmt,cc) COL_SEL(cc); x=642; output(x,(float)i,1,fmt,0); x+=8*(strlen(fmt)+1)
#define _OUTPUT(fmt,cc) COL_SEL(cc); output(x,(float)i,1,fmt,0); x+=8*(strlen(fmt)+1)
    i = 740;
    float x;
    if (_debugger.page == PAGE_GENERAL && _debugger.tri_sel)
    {
      COL_CATEGORY();
      OUTPUT ("GENERAL (page 1):",0);
      COL_TEXT();
      OUTPUT ("tri #%d", _debugger.tri_sel->tri_n);
      OUTPUT ("type: %s", tri_type[_debugger.tri_sel->type]);
      OUTPUT ("geom:   0x%08lx", _debugger.tri_sel->geom_mode);
      OUTPUT ("othermode_h: 0x%08lx", _debugger.tri_sel->othermode_h);
      OUTPUT ("othermode_l: 0x%08lx", _debugger.tri_sel->othermode_l);
      OUTPUT ("flags: 0x%08lx", _debugger.tri_sel->flags);
      OUTPUT ("",0);
      COL_CATEGORY();
      OUTPUT ("COMBINE:",0);
      COL_TEXT();
      OUTPUT ("cycle_mode: %s", cycle_mode_s[_debugger.tri_sel->cycle_mode]);
      OUTPUT ("cycle1: 0x%08lx", _debugger.tri_sel->cycle1);
      OUTPUT ("cycle2: 0x%08lx", _debugger.tri_sel->cycle2);
      if (_debugger.tri_sel->uncombined & 1)
        COL_UCC();
      else
        COL_CC();
      OUTPUT ("a0: %s", Mode0[(_debugger.tri_sel->cycle1)&0x0000000F]);
      OUTPUT ("b0: %s", Mode1[(_debugger.tri_sel->cycle1>>4)&0x0000000F]);
      OUTPUT ("c0: %s", Mode2[(_debugger.tri_sel->cycle1>>8)&0x0000001F]);
      OUTPUT ("d0: %s", Mode3[(_debugger.tri_sel->cycle1>>13)&0x00000007]);
      if (_debugger.tri_sel->uncombined & 2)
        COL_UAC();
      else
        COL_AC();
      OUTPUT ("Aa0: %s", Alpha0[(_debugger.tri_sel->cycle1>>16)&0x00000007]);
      OUTPUT ("Ab0: %s", Alpha1[(_debugger.tri_sel->cycle1>>19)&0x00000007]);
      OUTPUT ("Ac0: %s", Alpha2[(_debugger.tri_sel->cycle1>>22)&0x00000007]);
      OUTPUT ("Ad0: %s", Alpha3[(_debugger.tri_sel->cycle1>>25)&0x00000007]);
      if (_debugger.tri_sel->uncombined & 1)
        COL_UCC();
      else
        COL_CC();
      OUTPUT ("a1: %s", Mode0[(_debugger.tri_sel->cycle2)&0x0000000F]);
      OUTPUT ("b1: %s", Mode1[(_debugger.tri_sel->cycle2>>4)&0x0000000F]);
      OUTPUT ("c1: %s", Mode2[(_debugger.tri_sel->cycle2>>8)&0x0000001F]);
      OUTPUT ("d1: %s", Mode3[(_debugger.tri_sel->cycle2>>13)&0x00000007]);
      if (_debugger.tri_sel->uncombined & 2)
        COL_UAC();
      else
        COL_AC();
      OUTPUT ("Aa1: %s", Alpha0[(_debugger.tri_sel->cycle2>>16)&0x00000007]);
      OUTPUT ("Ab1: %s", Alpha1[(_debugger.tri_sel->cycle2>>19)&0x00000007]);
      OUTPUT ("Ac1: %s", Alpha2[(_debugger.tri_sel->cycle2>>22)&0x00000007]);
      OUTPUT ("Ad1: %s", Alpha3[(_debugger.tri_sel->cycle2>>25)&0x00000007]);
    }
    if ((_debugger.page == PAGE_TEX1 || _debugger.page == PAGE_TEX2) && _debugger.tri_sel)
    {
      COL_CATEGORY ();
      OUTPUT1 ("TEXTURE %d (page %d):", _debugger.page-PAGE_TEX1, 2+_debugger.page-PAGE_TEX1);
      COL_TEXT();
      int tmu = _debugger.page - PAGE_TEX1;
      OUTPUT1 ("cur cache: %d,%d", _debugger.tri_sel->t[tmu].cur_cache[tmu]&0x0F, _debugger.tri_sel->t[tmu].cur_cache[tmu]>>4);
      OUTPUT ("tex_size: %d", _debugger.tri_sel->t[tmu].size);
      OUTPUT ("tex_format: %d", _debugger.tri_sel->t[tmu].format);
      OUTPUT ("width: %d", _debugger.tri_sel->t[tmu].width);
      OUTPUT ("height: %d", _debugger.tri_sel->t[tmu].height);
      OUTPUT ("palette: %d", _debugger.tri_sel->t[tmu].palette);
      OUTPUT ("clamp_s: %d", _debugger.tri_sel->t[tmu].clamp_s);
      OUTPUT ("clamp_t: %d", _debugger.tri_sel->t[tmu].clamp_t);
      OUTPUT ("mirror_s: %d", _debugger.tri_sel->t[tmu].mirror_s);
      OUTPUT ("mirror_t: %d", _debugger.tri_sel->t[tmu].mirror_t);
      OUTPUT ("mask_s: %d", _debugger.tri_sel->t[tmu].mask_s);
      OUTPUT ("mask_t: %d", _debugger.tri_sel->t[tmu].mask_t);
      OUTPUT ("shift_s: %d", _debugger.tri_sel->t[tmu].shift_s);
      OUTPUT ("shift_t: %d", _debugger.tri_sel->t[tmu].shift_t);
      OUTPUT ("ul_s: %d", _debugger.tri_sel->t[tmu].ul_s);
      OUTPUT ("ul_t: %d", _debugger.tri_sel->t[tmu].ul_t);
      OUTPUT ("lr_s: %d", _debugger.tri_sel->t[tmu].lr_s);
      OUTPUT ("lr_t: %d", _debugger.tri_sel->t[tmu].lr_t);
      OUTPUT ("t_ul_s: %d", _debugger.tri_sel->t[tmu].t_ul_s);
      OUTPUT ("t_ul_t: %d", _debugger.tri_sel->t[tmu].t_ul_t);
      OUTPUT ("t_lr_s: %d", _debugger.tri_sel->t[tmu].t_lr_s);
      OUTPUT ("t_lr_t: %d", _debugger.tri_sel->t[tmu].t_lr_t);
      OUTPUT ("scale_s: %f", _debugger.tri_sel->t[tmu].scale_s);
      OUTPUT ("scale_t: %f", _debugger.tri_sel->t[tmu].scale_t);
      OUTPUT ("s_mode: %s", str_cm[((_debugger.tri_sel->t[tmu].clamp_s << 1) | _debugger.tri_sel->t[tmu].mirror_s)&3]);
      OUTPUT ("t_mode: %s", str_cm[((_debugger.tri_sel->t[tmu].clamp_t << 1) | _debugger.tri_sel->t[tmu].mirror_t)&3]);
    }
    if (_debugger.page == PAGE_COLORS && _debugger.tri_sel)
    {
      COL_CATEGORY();
      OUTPUT ("COLORS (page 4)", 0);
      COL_TEXT();
      OUTPUT ("fill:  %08lx", _debugger.tri_sel->fill_color);
      OUTPUT ("prim:  %08lx", _debugger.tri_sel->prim_color);
      OUTPUT ("blend: %08lx", _debugger.tri_sel->blend_color);
      OUTPUT ("env:   %08lx", _debugger.tri_sel->env_color);
      OUTPUT ("fog: %08lx", _debugger.tri_sel->fog_color);
      OUTPUT ("prim_lodmin:  %d", _debugger.tri_sel->prim_lodmin);
      OUTPUT ("prim_lodfrac: %d", _debugger.tri_sel->prim_lodfrac);
    }
    if (_debugger.page == PAGE_FBL && _debugger.tri_sel)
    {
      COL_CATEGORY();
      OUTPUT ("BLENDER", 0);
      COL_TEXT();
      OUTPUT ("fbl_a0: %s", FBLa[(_debugger.tri_sel->othermode_l>>30)&0x3]);
      OUTPUT ("fbl_b0: %s", FBLb[(_debugger.tri_sel->othermode_l>>26)&0x3]);
      OUTPUT ("fbl_c0: %s", FBLc[(_debugger.tri_sel->othermode_l>>22)&0x3]);
      OUTPUT ("fbl_d0: %s", FBLd[(_debugger.tri_sel->othermode_l>>18)&0x3]);
      OUTPUT ("fbl_a1: %s", FBLa[(_debugger.tri_sel->othermode_l>>28)&0x3]);
      OUTPUT ("fbl_b1: %s", FBLb[(_debugger.tri_sel->othermode_l>>24)&0x3]);
      OUTPUT ("fbl_c1: %s", FBLc[(_debugger.tri_sel->othermode_l>>20)&0x3]);
      OUTPUT ("fbl_d1: %s", FBLd[(_debugger.tri_sel->othermode_l>>16)&0x3]);
      OUTPUT ("", 0);
      OUTPUT ("fbl:    %08lx", _debugger.tri_sel->othermode_l&0xFFFF0000);
      OUTPUT ("fbl #1: %08lx", _debugger.tri_sel->othermode_l&0xCCCC0000);
      OUTPUT ("fbl #2: %08lx", _debugger.tri_sel->othermode_l&0x33330000);
    }
    if (_debugger.page == PAGE_OTHERMODE_L && _debugger.tri_sel)
    {
      wxUint32 othermode_l = _debugger.tri_sel->othermode_l;
      COL_CATEGORY ();
      OUTPUT ("OTHERMODE_L: %08lx", othermode_l);
      OUTPUT_ ("AC_NONE", (othermode_l & 3) == 0);
      _OUTPUT ("AC_THRESHOLD", (othermode_l & 3) == 1);
      _OUTPUT ("AC_DITHER", (othermode_l & 3) == 3);
      i -= 16;
      OUTPUT_ ("ZS_PIXEL", !(othermode_l & 4));
      _OUTPUT ("ZS_PRIM", (othermode_l & 4));
      i -= 32;
      COL_CATEGORY ();
      OUTPUT ("RENDERMODE: %08lx", othermode_l);
      OUTPUT_ ("AA_EN", othermode_l & 0x08);
      i -= 16;
      OUTPUT_ ("Z_CMP", othermode_l & 0x10);
      i -= 16;
      OUTPUT_ ("Z_UPD", othermode_l & 0x20);
      i -= 16;
      OUTPUT_ ("IM_RD", othermode_l & 0x40);
      i -= 16;
      OUTPUT_ ("CLR_ON_CVG", othermode_l & 0x80);
      i -= 16;
      OUTPUT_ ("CVG_DST_CLAMP", (othermode_l & 0x300) == 0x000);
      _OUTPUT ("CVG_DST_WRAP", (othermode_l & 0x300) == 0x100);
      _OUTPUT (".._FULL", (othermode_l & 0x300) == 0x200);
      _OUTPUT (".._SAVE", (othermode_l & 0x300) == 0x300);
      i -= 16;
      OUTPUT_ ("ZM_OPA", (othermode_l & 0xC00) == 0x000);
      _OUTPUT ("ZM_INTER", (othermode_l & 0xC00) == 0x400);
      _OUTPUT ("ZM_XLU", (othermode_l & 0xC00) == 0x800);
      _OUTPUT ("ZM_DEC", (othermode_l & 0xC00) == 0xC00);
      i -= 16;
      OUTPUT_ ("CVG_X_ALPHA", othermode_l & 0x1000);
      i -= 16;
      OUTPUT_ ("ALPHA_CVG_SEL", othermode_l & 0x2000);
      i -= 16;
      OUTPUT_ ("FORCE_BL", othermode_l & 0x4000);
    }
    if (_debugger.page == PAGE_OTHERMODE_H && _debugger.tri_sel)
    {
      wxUint32 othermode_h = _debugger.tri_sel->othermode_h;
      COL_CATEGORY ();
      OUTPUT ("OTHERMODE_H: %08lx", othermode_h);
      OUTPUT_ ("CK_NONE", (othermode_h & 0x100) == 0);
      _OUTPUT ("CK_KEY", (othermode_h & 0x100) == 1);
      i -= 16;
      OUTPUT_  ("TC_CONV", (othermode_h & 0xE00) == 0x200);
      _OUTPUT ("TC_FILTCONV", (othermode_h & 0xE00) == 0xA00);
      _OUTPUT ("TC_FILT", (othermode_h & 0xE00) == 0xC00);
      i -= 16;
      OUTPUT_ ("TF_POINT", (othermode_h & 0x3000) == 0x0000);
      _OUTPUT ("TF_AVERAGE", (othermode_h & 0x3000) == 0x3000);
      _OUTPUT ("TF_BILERP", (othermode_h & 0x3000) == 0x2000);
      i -= 16;
      OUTPUT_ ("TT_NONE", (othermode_h & 0xC000) == 0x0000);
      _OUTPUT ("TT_RGBA16", (othermode_h & 0xC000) == 0x8000);
      _OUTPUT ("TT_IA16", (othermode_h & 0xC000) == 0xC000);
      i -= 16;
      OUTPUT_ ("TL_TILE", (othermode_h & 0x10000) == 0x00000);
      _OUTPUT ("TL_LOD", (othermode_h & 0x10000) == 0x10000);
      i -= 16;
      OUTPUT_ ("TD_CLAMP", (othermode_h & 0x60000) == 0x00000);
      _OUTPUT ("TD_SHARPEN", (othermode_h & 0x60000) == 0x20000);
      _OUTPUT ("TD_DETAIL", (othermode_h & 0x60000) == 0x40000);
      i -= 16;
      OUTPUT_ ("TP_NONE", (othermode_h & 0x80000) == 0x00000);
      _OUTPUT ("TP_PERSP", (othermode_h & 0x80000) == 0x80000);
      i -= 16;
      OUTPUT_ ("1CYCLE", (othermode_h & 0x300000) == 0x000000);
      _OUTPUT ("2CYCLE", (othermode_h & 0x300000) == 0x100000);
      _OUTPUT ("COPY", (othermode_h & 0x300000) == 0x200000);
      _OUTPUT ("FILL", (othermode_h & 0x300000) == 0x300000);
      i -= 16;
      OUTPUT_ ("PM_1PRIM", (othermode_h & 0x400000) == 0x000000);
      _OUTPUT ("PM_NPRIM", (othermode_h & 0x400000) == 0x400000);
    }
    if (_debugger.page == PAGE_TEXELS && _debugger.tri_sel)
    {
      // change these to output whatever you need, ou for triangles, or u0 for texrects
      COL_TEXT();
      OUTPUT ("n: %d", _debugger.tri_sel->nv);
      OUTPUT ("",0);
      for (j=0; j<_debugger.tri_sel->nv; j++)
      {
        OUTPUT1 ("v[%d].s0: %f", j, _debugger.tri_sel->v[j].ou);
        OUTPUT1 ("v[%d].t0: %f", j, _debugger.tri_sel->v[j].ov);
      }
      OUTPUT ("",0);
      for (j=0; j<_debugger.tri_sel->nv; j++)
      {
        OUTPUT1 ("v[%d].s1: %f", j, _debugger.tri_sel->v[j].u0);
        OUTPUT1 ("v[%d].t1: %f", j, _debugger.tri_sel->v[j].v0);
      }
    }
    if (_debugger.page == PAGE_COORDS && _debugger.tri_sel)
    {
      COL_TEXT();
      OUTPUT ("n: %d", _debugger.tri_sel->nv);
      for (j=0; j<_debugger.tri_sel->nv; j++)
      {
        OUTPUT1 ("v[%d].x: %f", j, _debugger.tri_sel->v[j].x);
        OUTPUT1 ("v[%d].y: %f", j, _debugger.tri_sel->v[j].y);
        OUTPUT1 ("v[%d].z: %f", j, _debugger.tri_sel->v[j].z);
        OUTPUT1 ("v[%d].w: %f", j, _debugger.tri_sel->v[j].w);
        OUTPUT1 ("v[%d].f: %f", j, 1.0f/_debugger.tri_sel->v[j].f);
        OUTPUT1 ("v[%d].r: %d", j, _debugger.tri_sel->v[j].r);
        OUTPUT1 ("v[%d].g: %d", j, _debugger.tri_sel->v[j].g);
        OUTPUT1 ("v[%d].b: %d", j, _debugger.tri_sel->v[j].b);
        OUTPUT1 ("v[%d].a: %d", j, _debugger.tri_sel->v[j].a);
      }
    }
    if (_debugger.page == PAGE_TEX_INFO && _debugger.tex_sel < (wxUint32)rdp.n_cached[_debugger.tmu])
    {
      COL_CATEGORY();
      OUTPUT ("CACHE (page 0)", 0);
      COL_TEXT();
      //OUTPUT ("t_mem: %08lx", rdp.cache[0][_debugger.tex_sel].t_mem);
      //OUTPUT ("crc: %08lx", rdp.cache[0][_debugger.tex_sel].crc);
      OUTPUT ("addr: %08lx", cache[_debugger.tex_sel].addr);
      OUTPUT ("scale_x: %f", cache[_debugger.tex_sel].scale_x);
      OUTPUT ("scale_y: %f", cache[_debugger.tex_sel].scale_y);
      OUTPUT ("tmem_addr: %08lx", cache[_debugger.tex_sel].tmem_addr);
      OUTPUT ("palette: %08lx", cache[_debugger.tex_sel].palette);
      OUTPUT ("set_by: %08lx", cache[_debugger.tex_sel].set_by);
      OUTPUT ("texrecting: %d", cache[_debugger.tex_sel].texrecting);

      OUTPUT ("mod: %08lx", cache[_debugger.tex_sel].mod);
      OUTPUT ("mod_col: %08lx", cache[_debugger.tex_sel].mod_color);
      OUTPUT ("mod_col1: %08lx", cache[_debugger.tex_sel].mod_color1);
      i=740;
      output(800,(float)i,1,"width: %d", cache[_debugger.tex_sel].width);
      i-=16;
      output(800,(float)i,1,"height: %d", cache[_debugger.tex_sel].height);
      i-=16;
      output(800,(float)i,1,"format: %d", cache[_debugger.tex_sel].format);
      i-=16;
      output(800,(float)i,1,"size: %d", cache[_debugger.tex_sel].size);
      i-=16;
      output(800,(float)i,1,"crc: %08lx", cache[_debugger.tex_sel].crc);
      i-=16;
#ifdef TEXTURE_FILTER
      output(800,(float)i,1,"RiceCrc: %08lx", (wxUint32)(rdp.cache[_debugger.tmu][_debugger.tex_sel].ricecrc&0xFFFFFFFF));
      i-=16;
      output(800,(float)i,1,"RicePalCrc: %08lx", (wxUint32)(rdp.cache[_debugger.tmu][_debugger.tex_sel].ricecrc>>32));
      i-=16;
#endif
      output(800,(float)i,1,"flags: %08lx", cache[_debugger.tex_sel].flags);
      i-=16;
      output(800,(float)i,1,"line: %d", cache[_debugger.tex_sel].line);
      i-=16;
      output(800,(float)i,1,"mod_factor: %08lx", cache[_debugger.tex_sel].mod_factor);
      i-=32;

      output(800,(float)i,1,"lod: %s", str_lod[cache[_debugger.tex_sel].lod]);
      i-=16;
      output(800,(float)i,1,"aspect: %s", str_aspect[cache[_debugger.tex_sel].aspect + 3]);

//  debug_texture(_debugger.tmu, cache[_debugger.tex_sel].addr, _debugger.tex_sel);
    }

    // Draw the vertex numbers
    if (_debugger.tri_sel)
    {
      for (i=0; i<_debugger.tri_sel->nv; i++)
      {
        grConstantColorValue (0x000000FF);
        output (_debugger.tri_sel->v[i].x+1, settings.scr_res_y-_debugger.tri_sel->v[i].y+1, 1,
          "%d", i);
        grConstantColorValue (0xFFFFFFFF);
        output (_debugger.tri_sel->v[i].x, settings.scr_res_y-_debugger.tri_sel->v[i].y, 1,
          "%d", i);
      }
    }

    // Draw the cursor
    debug_mouse ();

    grBufferSwap (1);
  }

END:
  // Release all data
  delete [] _debugger.screen;
  TRI_INFO *tri;
  for (tri=_debugger.tri_list; tri != _debugger.tri_last;)
  {
    TRI_INFO *tmp = tri;
    tri = tri->pNext;
    delete [] tmp->v;
    delete tmp;
  }
  delete [] tri->v;
  delete tri;

  // Reset all values
  _debugger.capture = 0;
  _debugger.selected = SELECTED_TRI;
  _debugger.screen = NULL;
  _debugger.tri_list = NULL;
  _debugger.tri_last = NULL;
  _debugger.tri_sel = NULL;
  _debugger.tex_sel = 0;
}

//
// debug_mouse - draws the debugger mouse
//

void debug_mouse ()
{
  grColorCombine (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE,
    FXFALSE);

  grAlphaCombine (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE,
    FXFALSE);

  // Draw the cursor
  POINT pt;
  DbgCursorPos(&pt);
  float cx = (float)pt.x;
  float cy = (float)pt.y;

  VERTEX v[4] = {
    { cx,       cy, 1, 1,   0,   0,   0, 0, {0, 0, 0, 0} },
    { cx+32,    cy, 1, 1, 255,   0,   0, 0, {0, 0, 0, 0} },
    { cx,    cy+32, 1, 1,   0, 255,   0, 0, {0, 0, 0, 0} },
    { cx+32, cy+32, 1, 1, 255, 255,   0, 0, {0, 0, 0, 0} }
    };

  ConvertCoordsKeep (v, 4);

  grTexSource(GR_TMU0,
    voodoo.tex_min_addr[GR_TMU0] + offset_cursor,
    GR_MIPMAPLEVELMASK_BOTH,
    &cursorTex);

  if (voodoo.num_tmu >= 3)
    grTexCombine (GR_TMU2,
      GR_COMBINE_FUNCTION_NONE,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_NONE,
      GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE);
  if (voodoo.num_tmu >= 2)
    grTexCombine (GR_TMU1,
      GR_COMBINE_FUNCTION_NONE,
      GR_COMBINE_FACTOR_NONE,
      GR_COMBINE_FUNCTION_NONE,
      GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE);
  grTexCombine (GR_TMU0,
    GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE);

  grDrawTriangle (&v[0], &v[1], &v[2]);
  grDrawTriangle (&v[1], &v[3], &v[2]);
}

//
// debug_keys - receives debugger key input
//

void debug_keys ()
{
  if (CheckKeyPressed(G64_VK_RIGHT, 0x0001) && _debugger.tri_sel)
  {
    TRI_INFO *start = _debugger.tri_sel;

    while (_debugger.tri_sel->pNext != start)
      _debugger.tri_sel = _debugger.tri_sel->pNext;
  }

  if (CheckKeyPressed(G64_VK_LEFT, 0x0001) && _debugger.tri_sel)
    _debugger.tri_sel = _debugger.tri_sel->pNext;

  // Check for page changes
  if (CheckKeyPressed(G64_VK_1, 0x0001))
    _debugger.page = PAGE_GENERAL;
  if (CheckKeyPressed(G64_VK_2, 0x0001))
    _debugger.page = PAGE_TEX1;
  if (CheckKeyPressed(G64_VK_3, 0x0001))
    _debugger.page = PAGE_TEX2;
  if (CheckKeyPressed(G64_VK_4, 0x0001))
    _debugger.page = PAGE_COLORS;
  if (CheckKeyPressed(G64_VK_5, 0x0001))
    _debugger.page = PAGE_FBL;
  if (CheckKeyPressed(G64_VK_6, 0x0001))
    _debugger.page = PAGE_OTHERMODE_L;
  if (CheckKeyPressed(G64_VK_7, 0x0001))
    _debugger.page = PAGE_OTHERMODE_H;
  if (CheckKeyPressed(G64_VK_8, 0x0001))
    _debugger.page = PAGE_TEXELS;
  if (CheckKeyPressed(G64_VK_9, 0x0001))
    _debugger.page = PAGE_COORDS;
  if (CheckKeyPressed(G64_VK_0, 0x0001))
    _debugger.page = PAGE_TEX_INFO;
  if (CheckKeyPressed(G64_VK_Q, 0x0001))
    _debugger.tmu = 0;
  if (CheckKeyPressed(G64_VK_W, 0x0001))
    _debugger.tmu = 1;

  if (CheckKeyPressed(G64_VK_G, 0x0001))
    grid = !grid;

  // Go to texture
  if (CheckKeyPressed(G64_VK_SPACE, 0x0001))
  {
    int tile = -1;
    if (_debugger.page == PAGE_TEX2)
      tile = 1;
    else
      tile = 0;
    if (tile != -1)
    {
      _debugger.tmu = _debugger.tri_sel->t[tile].tmu;
      _debugger.tex_sel = _debugger.tri_sel->t[tile].cur_cache[_debugger.tmu];
      _debugger.tex_scroll = (_debugger.tri_sel->t[tile].cur_cache[_debugger.tmu] >> 4) - 1;
    }
  }

  // Go to triangle
  CACHE_LUT * cache = voodoo.tex_UMA?rdp.cache[0]:rdp.cache[_debugger.tmu];
  if (CheckKeyPressed(G64_VK_CONTROL, 0x0001))
  {
    int count = rdp.debug_n - cache[_debugger.tex_sel].uses - 1;
    if (cache[_debugger.tex_sel].last_used == frame_count)
    {
      TRI_INFO *t = _debugger.tri_list;
      while (count && t) {
        t = t->pNext;
        count --;
      }
      _debugger.tri_sel = t;
    }
    else
      _debugger.tri_sel = NULL;
  }

  if (CheckKeyPressed(G64_VK_A, 0x0001))
    _debugger.draw_mode = 0;  // texture & texture alpha
  if (CheckKeyPressed(G64_VK_S, 0x0001))
    _debugger.draw_mode = 1;  // texture
  if (CheckKeyPressed(G64_VK_D, 0x0001))
    _debugger.draw_mode = 2;  // texture alpha

  // Check for texture scrolling
  if (CheckKeyPressed(G64_VK_DOWN, 0x0001))
    _debugger.tex_scroll ++;
  if (CheckKeyPressed(G64_VK_UP, 0x0001))
    _debugger.tex_scroll --;
}

//
// output - output debugger text
//

void output (float x, float y, int scale, const char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  vsprintf(out_buf, fmt, ap);
  va_end(ap);

  wxUint8 c,r;
  for (wxUint32 i=0; i<strlen(out_buf); i++)
  {
    c = ((out_buf[i]-32) & 0x1F) * 8;//<< 3;
    r = (((out_buf[i]-32) & 0xE0) >> 5) * 16;//<< 4;
    VERTEX v[4] = { { SX(x), SY(768-y), 1, 1,   (float)c, r+16.0f, 0, 0, {0, 0, 0, 0} },
      { SX(x+8), SY(768-y), 1, 1,   c+8.0f, r+16.0f, 0, 0, {0, 0, 0, 0} },
      { SX(x), SY(768-y-16), 1, 1,  (float)c, (float)r, 0, 0, {0, 0, 0, 0} },
      { SX(x+8), SY(768-y-16), 1, 1,  c+8.0f, (float)r, 0, 0, {0, 0, 0, 0} }
      };
    if (!scale)
    {
      v[0].x = x;
      v[0].y = y;
      v[1].x = x+8;
      v[1].y = y;
      v[2].x = x;
      v[2].y = y-16;
      v[3].x = x+8;
      v[3].y = y-16;
    }

    ConvertCoordsKeep (v, 4);

    grDrawTriangle (&v[0], &v[1], &v[2]);
    grDrawTriangle (&v[1], &v[3], &v[2]);

    x+=8;
  }
}
