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

#include "Gfx #1.3.h"
#include "Util.h"
#include "Combine.h"

#define FASTSEARCH  // Enable fast combine mode searching algorithm

float percent_org, percent, r, g, b;
wxUint32 lod_frac;

wxUint32 cc_lookup[257];
wxUint32 ac_lookup[257];
COMBINE cmb;

//****************************************************************
// Macros
//****************************************************************

#define MOD_0(mode)     cmb.mod_0 = mode
#define MOD_0_COL(color)  cmb.modcolor_0 = color
#define MOD_0_COL1(color) cmb.modcolor1_0 = color
#define MOD_0_COL2(color) cmb.modcolor2_0 = color
#define MOD_0_FAC(factor) cmb.modfactor_0 = factor
#define MOD_1(mode)     cmb.mod_1 = mode
#define MOD_1_COL(color)  cmb.modcolor_1 = color
#define MOD_1_COL1(color) cmb.modcolor1_1 = color
#define MOD_1_COL2(color) cmb.modcolor2_1 = color
#define MOD_1_FAC(factor) cmb.modfactor_1 = factor

#define A_BLEND(f1,f2) cmb.abf1=f1,cmb.abf2=f2

// To make a color or alpha combine
#define CCMB(fnc,fac,loc,oth) \
  cmb.c_fnc = fnc, \
  cmb.c_fac = fac, \
  cmb.c_loc = loc, \
  cmb.c_oth = oth
#define ACMB(fnc,fac,loc,oth) \
  cmb.a_fnc = fnc, \
  cmb.a_fac = fac, \
  cmb.a_loc = loc, \
  cmb.a_oth = oth
#define CCMBEXT(a,a_mode,b,b_mode,c,c_invert,d,d_invert) \
  cmb.c_ext_a = a, \
  cmb.c_ext_a_mode = a_mode, \
  cmb.c_ext_b = b, \
  cmb.c_ext_b_mode = b_mode, \
  cmb.c_ext_c = c, \
  cmb.c_ext_c_invert = c_invert, \
  cmb.c_ext_d= d, \
  cmb.c_ext_d_invert = d_invert, \
  cmb.cmb_ext_use |= COMBINE_EXT_COLOR
#define ACMBEXT(a,a_mode,b,b_mode,c,c_invert,d,d_invert) \
  cmb.a_ext_a = a, \
  cmb.a_ext_a_mode = a_mode, \
  cmb.a_ext_b = b, \
  cmb.a_ext_b_mode = b_mode, \
  cmb.a_ext_c = c, \
  cmb.a_ext_c_invert = c_invert, \
  cmb.a_ext_d= d, \
  cmb.a_ext_d_invert = d_invert, \
  cmb.cmb_ext_use |= COMBINE_EXT_ALPHA
#define T0CCMBEXT(a,a_mode,b,b_mode,c,c_invert,d,d_invert) \
  cmb.t0c_ext_a = a, \
  cmb.t0c_ext_a_mode = a_mode, \
  cmb.t0c_ext_b = b, \
  cmb.t0c_ext_b_mode = b_mode, \
  cmb.t0c_ext_c = c, \
  cmb.t0c_ext_c_invert = c_invert, \
  cmb.t0c_ext_d= d, \
  cmb.t0c_ext_d_invert = d_invert, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_COLOR
#define T0ACMBEXT(a,a_mode,b,b_mode,c,c_invert,d,d_invert) \
  cmb.t0a_ext_a = a, \
  cmb.t0a_ext_a_mode = a_mode, \
  cmb.t0a_ext_b = b, \
  cmb.t0a_ext_b_mode = b_mode, \
  cmb.t0a_ext_c = c, \
  cmb.t0a_ext_c_invert = c_invert, \
  cmb.t0a_ext_d= d, \
  cmb.t0a_ext_d_invert = d_invert, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_ALPHA
#define T1CCMBEXT(a,a_mode,b,b_mode,c,c_invert,d,d_invert) \
  cmb.t1c_ext_a = a, \
  cmb.t1c_ext_a_mode = a_mode, \
  cmb.t1c_ext_b = b, \
  cmb.t1c_ext_b_mode = b_mode, \
  cmb.t1c_ext_c = c, \
  cmb.t1c_ext_c_invert = c_invert, \
  cmb.t1c_ext_d= d, \
  cmb.t1c_ext_d_invert = d_invert, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_COLOR
#define T1ACMBEXT(a,a_mode,b,b_mode,c,c_invert,d,d_invert) \
  cmb.t1a_ext_a = a, \
  cmb.t1a_ext_a_mode = a_mode, \
  cmb.t1a_ext_b = b, \
  cmb.t1a_ext_b_mode = b_mode, \
  cmb.t1a_ext_c = c, \
  cmb.t1a_ext_c_invert = c_invert, \
  cmb.t1a_ext_d= d, \
  cmb.t1a_ext_d_invert = d_invert, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_ALPHA

// To use textures
#define USE_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 1, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_LOCAL
#define USE_T1() \
  if (voodoo.num_tmu > 1) { \
  rdp.best_tex = 1; \
  cmb.tex |= 2, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_ONE; \
  } \
  else { \
  USE_T0(); \
}
#define T0_ADD_T1() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_ONE
#define T0_MUL_T1() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_LOCAL
#define T0_MUL_T1_ADD_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_LOCAL
#define T0A_MUL_T1() \
  rdp.best_tex = 1; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_LOCAL_ALPHA
#define T0_MUL_T1A() \
  rdp.best_tex = 1; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL_ALPHA, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_LOCAL
#define T0_INTER_T1_USING_FACTOR(factor) \
  if (factor == 0xFF) { \
  USE_T1(); \
  } \
  else if (factor == 0x00) { \
  USE_T0(); \
}\
  else {\
  if (factor <= 0x80) rdp.best_tex = 0; \
  else rdp.best_tex = 1; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_BLEND, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR, \
  percent = (float)factor / 255.0f, \
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent; \
}
#define T1_INTER_T0_USING_FACTOR(factor)  /* inverse of above */\
  if (factor == 0xFF) { \
  USE_T0(); \
  } \
  else if (factor == 0x00) { \
  USE_T1(); \
}\
  else {\
  if (factor <= 0x80) rdp.best_tex = 0; \
  else rdp.best_tex = 1; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_BLEND, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR, \
  percent = (255 - factor) / 255.0f, \
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent; \
}
#define T0_INTER_T1_USING_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_BLEND, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_LOCAL
#define T1_INTER_T0_USING_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_BLEND, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_ONE_MINUS_LOCAL
#define T0_INTER_T1_USING_T1() \
  if (!cmb.combine_ext) { \
  T0_INTER_T1_USING_FACTOR(0x7F); \
  }\
  else {\
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.t1c_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_a_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_b_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_c = GR_CMBX_ZERO, \
  cmb.t1c_ext_c_invert = 0, \
  cmb.t1c_ext_d= GR_CMBX_B, \
  cmb.t1c_ext_d_invert = 0, \
  cmb.t0c_ext_a = GR_CMBX_OTHER_TEXTURE_RGB, \
  cmb.t0c_ext_a_mode = GR_FUNC_MODE_X, \
  cmb.t0c_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t0c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X, \
  cmb.t0c_ext_c = GR_CMBX_OTHER_TEXTURE_RGB, \
  cmb.t0c_ext_c_invert = 0, \
  cmb.t0c_ext_d= GR_CMBX_B, \
  cmb.t0c_ext_d_invert = 0, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_COLOR; \
}
#define T0_INTER_T1_USING_T1A() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_BLEND, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_OTHER_ALPHA
#define T0_INTER_T1_USING_PRIM() \
  if (!cmb.combine_ext) { \
  T0_INTER_T1_USING_FACTOR ((rdp.prim_color&0xFF)); \
  }\
  else {\
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.t1c_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_a_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_b_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_c = GR_CMBX_ZERO, \
  cmb.t1c_ext_c_invert = 0, \
  cmb.t1c_ext_d= GR_CMBX_B, \
  cmb.t1c_ext_d_invert = 0, \
  cmb.t0c_ext_a = GR_CMBX_OTHER_TEXTURE_RGB, \
  cmb.t0c_ext_a_mode = GR_FUNC_MODE_X, \
  cmb.t0c_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t0c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X, \
  cmb.t0c_ext_c = GR_CMBX_TMU_CCOLOR, \
  cmb.t0c_ext_c_invert = 0, \
  cmb.t0c_ext_d= GR_CMBX_B, \
  cmb.t0c_ext_d_invert = 0, \
  cmb.tex_ccolor = rdp.prim_color, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_COLOR; \
}
#define T1_INTER_T0_USING_PRIM() /* inverse of above */\
  if (!cmb.combine_ext) { \
  T1_INTER_T0_USING_FACTOR ((rdp.prim_color&0xFF)); \
  }\
  else {\
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.t1c_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_a_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_b_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_c = GR_CMBX_ZERO, \
  cmb.t1c_ext_c_invert = 0, \
  cmb.t1c_ext_d= GR_CMBX_B, \
  cmb.t1c_ext_d_invert = 0, \
  cmb.t0c_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t0c_ext_a_mode = GR_FUNC_MODE_X, \
  cmb.t0c_ext_b = GR_CMBX_OTHER_TEXTURE_RGB, \
  cmb.t0c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X, \
  cmb.t0c_ext_c = GR_CMBX_TMU_CCOLOR, \
  cmb.t0c_ext_c_invert = 0, \
  cmb.t0c_ext_d= GR_CMBX_B, \
  cmb.t0c_ext_d_invert = 0, \
  cmb.tex_ccolor = rdp.prim_color, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_COLOR; \
}
#define T0_INTER_T1_USING_ENV() \
  if (!cmb.combine_ext) { \
  T0_INTER_T1_USING_FACTOR ((rdp.env_color&0xFF)); \
  }\
  else {\
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.t1c_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_a_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_b_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_c = GR_CMBX_ZERO, \
  cmb.t1c_ext_c_invert = 0, \
  cmb.t1c_ext_d= GR_CMBX_B, \
  cmb.t1c_ext_d_invert = 0, \
  cmb.t0c_ext_a = GR_CMBX_OTHER_TEXTURE_RGB, \
  cmb.t0c_ext_a_mode = GR_FUNC_MODE_X, \
  cmb.t0c_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t0c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X, \
  cmb.t0c_ext_c = GR_CMBX_TMU_CCOLOR, \
  cmb.t0c_ext_c_invert = 0, \
  cmb.t0c_ext_d= GR_CMBX_B, \
  cmb.t0c_ext_d_invert = 0, \
  cmb.tex_ccolor = rdp.env_color, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_COLOR; \
}
#define T1_INTER_T0_USING_ENV() /* inverse of above */\
  if (!cmb.combine_ext) { \
  T1_INTER_T0_USING_FACTOR ((rdp.env_color&0xFF)); \
  }\
  else {\
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.t1c_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_a_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_b_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_c = GR_CMBX_ZERO, \
  cmb.t1c_ext_c_invert = 0, \
  cmb.t1c_ext_d= GR_CMBX_B, \
  cmb.t1c_ext_d_invert = 0, \
  cmb.t0c_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t0c_ext_a_mode = GR_FUNC_MODE_X, \
  cmb.t0c_ext_b = GR_CMBX_OTHER_TEXTURE_RGB, \
  cmb.t0c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X, \
  cmb.t0c_ext_c = GR_CMBX_TMU_CCOLOR, \
  cmb.t0c_ext_c_invert = 0, \
  cmb.t0c_ext_d= GR_CMBX_B, \
  cmb.t0c_ext_d_invert = 0, \
  cmb.tex_ccolor = rdp.env_color, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_COLOR; \
}
#define T0_INTER_T1_USING_SHADEA() \
  if (!cmb.combine_ext) { \
  T0_INTER_T1_USING_FACTOR (0x7F); \
  }\
  else {\
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.t1c_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_a_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_b_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_c = GR_CMBX_ZERO, \
  cmb.t1c_ext_c_invert = 0, \
  cmb.t1c_ext_d= GR_CMBX_B, \
  cmb.t1c_ext_d_invert = 0, \
  cmb.t0c_ext_a = GR_CMBX_OTHER_TEXTURE_RGB, \
  cmb.t0c_ext_a_mode = GR_FUNC_MODE_X, \
  cmb.t0c_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t0c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X, \
  cmb.t0c_ext_c = GR_CMBX_ITALPHA, \
  cmb.t0c_ext_c_invert = 0, \
  cmb.t0c_ext_d= GR_CMBX_B, \
  cmb.t0c_ext_d_invert = 0, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_COLOR; \
}
#define T1_INTER_T0_USING_SHADEA() \
  if (!cmb.combine_ext) { \
  T0_INTER_T1_USING_FACTOR (0x7F); \
  }\
  else {\
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.t1c_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_a_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t1c_ext_b_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1c_ext_c = GR_CMBX_ZERO, \
  cmb.t1c_ext_c_invert = 0, \
  cmb.t1c_ext_d= GR_CMBX_B, \
  cmb.t1c_ext_d_invert = 0, \
  cmb.t0c_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t0c_ext_a_mode = GR_FUNC_MODE_X, \
  cmb.t0c_ext_b = GR_CMBX_OTHER_TEXTURE_RGB, \
  cmb.t0c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X, \
  cmb.t0c_ext_c = GR_CMBX_ITALPHA, \
  cmb.t0c_ext_c_invert = 0, \
  cmb.t0c_ext_d= GR_CMBX_B, \
  cmb.t0c_ext_d_invert = 0, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_COLOR; \
}
#define T1_SUB_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_ONE
#define T1_SUB_T0_MUL_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_LOCAL
#define T1_MUL_PRIMLOD_ADD_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR, \
  percent = (float)(lod_frac) / 255.0f, \
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent
#define T1_MUL_PRIMA_ADD_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR, \
  percent = (float)(rdp.prim_color&0xFF) / 255.0f, \
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent
#define T1_MUL_ENVA_ADD_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, \
  cmb.tmu0_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR, \
  percent = (float)(rdp.env_color&0xFF) / 255.0f, \
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent
#define T0_SUB_PRIM_MUL_PRIMLOD_ADD_T1() \
  T0_ADD_T1 (); \
  MOD_0 (TMOD_TEX_SUB_COL_MUL_FAC); \
  MOD_0_COL (rdp.prim_color & 0xFFFFFF00); \
  MOD_0_FAC (lod_frac & 0xFF);
#define T1_SUB_PRIM_MUL_PRIMLOD_ADD_T0() \
  if (cmb.combine_ext) \
{ \
  T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X, \
  GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X, \
  GR_CMBX_DETAIL_FACTOR, 0, \
  GR_CMBX_ZERO, 0); \
  T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X, \
  GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X, \
  GR_CMBX_ZERO, 1, \
  GR_CMBX_ZERO, 0); \
  cmb.tex_ccolor = rdp.prim_color; \
  cmb.tex |= 3; \
  percent = (float)(lod_frac) / 255.0f; \
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent; \
} \
  else \
{  \
  T0_ADD_T1 (); \
  MOD_1 (TMOD_TEX_SUB_COL_MUL_FAC); \
  MOD_1_COL (rdp.prim_color & 0xFFFFFF00); \
  MOD_1_FAC (lod_frac & 0xFF); \
}
#define PRIM_INTER_T0_USING_SHADEA() \
  if (!cmb.combine_ext) { \
  USE_T0 (); \
  }\
  else {\
  rdp.best_tex = 0; \
  cmb.tex |= 1, \
  cmb.t0c_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB, \
  cmb.t0c_ext_a_mode = GR_FUNC_MODE_X, \
  cmb.t0c_ext_b = GR_CMBX_TMU_CCOLOR, \
  cmb.t0c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X, \
  cmb.t0c_ext_c = GR_CMBX_ITALPHA, \
  cmb.t0c_ext_c_invert = 0, \
  cmb.t0c_ext_d= GR_CMBX_B, \
  cmb.t0c_ext_d_invert = 0, \
  cmb.tex_ccolor = rdp.prim_color, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_COLOR; \
}

#define A_USE_T0() \
  cmb.tex |= 1, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_LOCAL
#define A_USE_T1() \
  if (voodoo.num_tmu > 1) { \
  cmb.tex |= 2, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_OTHER, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_ONE; \
  } \
  else { \
  A_USE_T0(); \
}
#define A_T0_ADD_T1() \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_ONE
#define A_T1_SUB_T0() \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_ONE
#define A_T0_SUB_T1() \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_BLEND_LOCAL, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_OTHER_ALPHA
#define A_T0_MUL_T1() \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_OTHER, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_LOCAL
#define A_T0_INTER_T1_USING_T0A() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_BLEND, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_LOCAL_ALPHA
#define A_T1_INTER_T0_USING_T0A() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_BLEND, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA
#define A_T0_INTER_T1_USING_T1A() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_BLEND, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_OTHER_ALPHA
#define A_T0_INTER_T1_USING_FACTOR(factor) \
  if (factor == 0xFF) { \
  A_USE_T1(); \
  } \
  else if (factor == 0x00) { \
  A_USE_T0(); \
}\
  else { \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_BLEND, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR, \
  percent = (float)factor / 255.0f, \
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent; \
}
#define A_T1_INTER_T0_USING_FACTOR(factor) /* inverse of above */\
  if (factor == 0xFF) { \
  A_USE_T0(); \
  } \
  else if (factor == 0x00) { \
  A_USE_T1(); \
}\
  else { \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_BLEND, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR, \
  percent = (255 - factor) / 255.0f, \
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent; \
}
#define A_T0_INTER_T1_USING_SHADEA() \
  if (!cmb.combine_ext) { \
  A_T0_INTER_T1_USING_FACTOR (0x7F); \
  }\
  else {\
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.t1a_ext_a = GR_CMBX_LOCAL_TEXTURE_ALPHA, \
  cmb.t1a_ext_a_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1a_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA, \
  cmb.t1a_ext_b_mode = GR_FUNC_MODE_ZERO, \
  cmb.t1a_ext_c = GR_CMBX_ZERO, \
  cmb.t1a_ext_c_invert = 0, \
  cmb.t1a_ext_d= GR_CMBX_B, \
  cmb.t1a_ext_d_invert = 0, \
  cmb.t0a_ext_a = GR_CMBX_OTHER_TEXTURE_ALPHA, \
  cmb.t0a_ext_a_mode = GR_FUNC_MODE_X, \
  cmb.t0a_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA, \
  cmb.t0a_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X, \
  cmb.t0a_ext_c = GR_CMBX_ITALPHA, \
  cmb.t0a_ext_c_invert = 0, \
  cmb.t0a_ext_d= GR_CMBX_B, \
  cmb.t0a_ext_d_invert = 0, \
  cmb.tex_cmb_ext_use |= TEX_COMBINE_EXT_ALPHA; \
}
#define A_T1_MUL_PRIMLOD_ADD_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR, \
  percent = (float)(lod_frac) / 255.0f, \
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent
#define A_T1_MUL_PRIMA_ADD_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR, \
  percent = (float)(rdp.prim_color&0xFF) / 255.0f, \
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent
#define A_T1_MUL_ENVA_ADD_T0() \
  rdp.best_tex = 0; \
  cmb.tex |= 3, \
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL, \
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL, \
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR, \
  percent = (float)(rdp.env_color&0xFF) / 255.0f, \
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent


// UNIMP - writes to the unimplemented log, if it's enabled
#ifdef UNIMP_LOG
#define UNIMPMODE() { \
  std::ofstream unimp; \
  unimp.open("unimp.txt", std::ios::app); \
  unimp << out_buf; \
  unimp.close(); \
}
#else
#define UNIMPMODE()
#endif

// Bright red, sets up a bright red combine
#ifdef BRIGHT_RED
// Bright red, sets up a bright red combine during the alpha stage
#define BrightRed() { \
  CCMB (GR_COMBINE_FUNCTION_LOCAL, \
  GR_COMBINE_FACTOR_NONE, \
  GR_COMBINE_LOCAL_CONSTANT, \
  GR_COMBINE_OTHER_NONE); \
  ACMB (GR_COMBINE_FUNCTION_LOCAL, \
  GR_COMBINE_FACTOR_NONE, \
  GR_COMBINE_LOCAL_CONSTANT, \
  GR_COMBINE_OTHER_NONE); \
  cmb.ccolor = 0xFF0000FF; \
}
#else
#define BrightRed()
#endif

#define CC(color) cmb.ccolor=(color)&0xFFFFFF00
#define CC_BYTE(byte) { cmb.ccolor=(byte<<8)|(byte<<16)|(byte<<24); }
#define CC_C1MULC2(color1, color2) { \
  cmb.ccolor=(wxUint8)( ((color1 & 0xFF000000) >> 24) * (((color2 & 0xFF000000) >> 24) /255.0f) ) <<  24 | \
  (wxUint8)( ((color1 & 0x00FF0000) >> 16) * (((color2 & 0x00FF0000) >> 16) /255.0f) ) <<  16 | \
  (wxUint8)( ((color1 & 0x0000FF00) >>  8) * (((color2 & 0x0000FF00) >>  8) /255.0f) ) <<   8 ; \
}
#define CC_C1SUBC2(color1, color2) { \
  cmb.ccolor=(wxUint8)( max(0, (int)((color1 & 0xFF000000) >> 24) - (int)((color2 & 0xFF000000) >> 24)) ) << 24 | \
  (wxUint8)( max(0, (int)((color1 & 0x00FF0000) >> 16) - (int)((color2 & 0x00FF0000) >> 16)) ) << 16 | \
  (wxUint8)( max(0, (int)((color1 & 0x0000FF00) >>  8) - (int)((color2 & 0x0000FF00) >>  8)) ) <<  8 ; \
}
#define CC_COLMULBYTE(color, byte) { \
    float factor = byte/255.0f; \
    cmb.ccolor = (wxUint8)( ((color & 0xFF000000) >> 24) * factor ) <<  24 | \
      (wxUint8)( ((color & 0x00FF0000) >> 16) * factor ) <<  16 | \
      (wxUint8)( ((color & 0x0000FF00) >>  8) * factor ) <<   8 ; \
}
#define CC_PRIM() CC(rdp.prim_color)
#define CC_ENV() CC(rdp.env_color)
#define CC_1SUBPRIM() CC((~rdp.prim_color))
#define CC_1SUBENV() CC((~rdp.env_color))
#define CC_PRIMA() CC_BYTE((rdp.prim_color&0xFF))
#define CC_ENVA() CC_BYTE((rdp.env_color&0xFF))
#define CC_1SUBPRIMA() CC_BYTE(((~rdp.prim_color)&0xFF))
#define CC_1SUBENVA() CC_BYTE(((~rdp.env_color)&0xFF))
#define CC_PRIMLOD() CC_BYTE(rdp.prim_lodfrac)
#define CC_K5() CC_BYTE(rdp.K5)
#define CC_PRIMMULENV() CC_C1MULC2(rdp.prim_color, rdp.env_color)
#define CC_PRIMSUBENV() CC_C1SUBC2(rdp.prim_color, rdp.env_color)

#define XSHADE(color, flag) { \
  rdp.col[0] *= (float)((color & 0xFF000000) >> 24) / 255.0f; \
  rdp.col[1] *= (float)((color & 0x00FF0000) >> 16) / 255.0f; \
  rdp.col[2] *= (float)((color & 0x0000FF00) >> 8) / 255.0f; \
  rdp.cmb_flags |= flag; \
}
#define XSHADE1M(color, flag) { \
  rdp.col[0] *= 1.0f-((float)((color & 0xFF000000) >> 24)/255.0f); \
  rdp.col[1] *= 1.0f-((float)((color & 0x00FF0000) >> 16)/255.0f); \
  rdp.col[2] *= 1.0f-((float)((color & 0x0000FF00) >> 8)/255.0f); \
  rdp.cmb_flags |= flag; \
}
#define XSHADEC1MC2(color1, color2, flag) { \
  rdp.col[0] *= (float)( max(0, (int)((color1 & 0xFF000000) >> 24) - (int)((color2 & 0xFF000000) >> 24)) )/255.0f; \
  rdp.col[1] *= (float)( max(0, (int)((color1 & 0x00FF0000) >> 16) - (int)((color2 & 0x00FF0000) >> 16)) )/255.0f; \
  rdp.col[2] *= (float)( max(0, (int)((color1 & 0x0000FF00) >> 8)  - (int)((color2 & 0x0000FF00) >> 8)) )/255.0f; \
  rdp.cmb_flags |= flag; \
}
#define XSHADE_BYTE(byte, flag) { \
  float tmpcol = (float)byte / 255.0f; \
  rdp.col[0] *= tmpcol; \
  rdp.col[1] *= tmpcol; \
  rdp.col[2] *= tmpcol; \
  rdp.cmb_flags |= flag; \
}
#define MULSHADE(color) XSHADE(color, CMB_MULT)
#define MULSHADE_PRIM() MULSHADE(rdp.prim_color)
#define MULSHADE_ENV() MULSHADE(rdp.env_color)
#define MULSHADE_1MPRIM() XSHADE1M(rdp.prim_color, CMB_MULT)
#define MULSHADE_1MENV() XSHADE1M(rdp.env_color, CMB_MULT)
#define MULSHADE_PRIMSUBENV() XSHADEC1MC2(rdp.prim_color, rdp.env_color, CMB_MULT)
#define MULSHADE_ENVSUBPRIM() XSHADEC1MC2(rdp.env_color, rdp.prim_color, CMB_MULT)
#define MULSHADE_BYTE(byte) XSHADE_BYTE(byte, CMB_MULT)
#define MULSHADE_PRIMA() MULSHADE_BYTE((rdp.prim_color & 0xFF))
#define MULSHADE_ENVA() MULSHADE_BYTE((rdp.env_color & 0xFF))
#define MULSHADE_1MENVA() MULSHADE_BYTE(((~rdp.env_color) & 0xFF))
#define MULSHADE_PRIMLOD() MULSHADE_BYTE((rdp.prim_lodfrac & 0xFF))
#define MULSHADE_K5() MULSHADE_BYTE(rdp.K5)

#define SETSHADE(color) XSHADE(color, CMB_SET)
#define SETSHADE_PRIM() SETSHADE(rdp.prim_color)
#define SETSHADE_ENV() SETSHADE(rdp.env_color)
#define SETSHADE_BYTE(byte) XSHADE_BYTE(byte, CMB_SET)
#define SETSHADE_PRIMA() SETSHADE_BYTE((rdp.prim_color & 0xFF))
#define SETSHADE_ENVA() SETSHADE_BYTE((rdp.env_color & 0xFF))
#define SETSHADE_1MPRIMA() SETSHADE_BYTE(((~rdp.prim_color) & 0xFF))
#define SETSHADE_PRIMLOD() SETSHADE_BYTE((rdp.prim_lodfrac & 0xFF))
#define SETSHADE_1MPRIMLOD() SETSHADE_BYTE(((~rdp.prim_lodfrac) & 0xFF))

#define SETSHADE_1MPRIM() XSHADE1M(rdp.prim_color, CMB_SET)
#define SETSHADE_1MENV() XSHADE1M(rdp.env_color, CMB_SET)
#define SETSHADE_PRIMSUBENV() XSHADEC1MC2(rdp.prim_color, rdp.env_color, CMB_SET)
#define SETSHADE_ENVSUBPRIM() XSHADEC1MC2(rdp.env_color, rdp.prim_color, CMB_SET)
#define SETSHADE_SHADE_A() { \
  rdp.cmb_flags = CMB_SETSHADE_SHADEALPHA; \
}

#define XSHADEADD(color, flag) { \
  rdp.coladd[0] *= (float)((color & 0xFF000000) >> 24) / 255.0f; \
  rdp.coladd[1] *= (float)((color & 0x00FF0000) >> 16) / 255.0f; \
  rdp.coladd[2] *= (float)((color & 0x0000FF00) >> 8) / 255.0f; \
  rdp.cmb_flags |= flag; \
}
#define XSHADEC1MC2ADD(color1, color2, flag) { \
  rdp.coladd[0] *= (float)( max(0, (int)((color1 & 0xFF000000) >> 24) - (int)((color2 & 0xFF000000) >> 24)) )/255.0f; \
  rdp.coladd[1] *= (float)( max(0, (int)((color1 & 0x00FF0000) >> 16) - (int)((color2 & 0x00FF0000) >> 16)) )/255.0f; \
  rdp.coladd[2] *= (float)( max(0, (int)((color1 & 0x0000FF00) >> 8)  - (int)((color2 & 0x0000FF00) >> 8)) )/255.0f; \
  rdp.cmb_flags |= flag; \
}
#define SUBSHADE_PRIM() XSHADEADD(rdp.prim_color, CMB_SUB)
#define SUBSHADE_ENV() XSHADEADD(rdp.env_color, CMB_SUB)
#define SUBSHADE_PRIMSUBENV() XSHADEC1MC2ADD(rdp.prim_color, rdp.env_color, CMB_SUB)
#define ADDSHADE_PRIM() XSHADEADD(rdp.prim_color, CMB_ADD)
#define ADDSHADE_ENV() XSHADEADD(rdp.env_color, CMB_ADD)
#define ADDSHADE_PRIMSUBENV() XSHADEC1MC2ADD(rdp.prim_color, rdp.env_color, CMB_ADD)
#define SUBSHADE_PRIMMULENV() { \
  rdp.coladd[0] *= (float)( ((rdp.prim_color & 0xFF000000) >> 24) * ((rdp.env_color & 0xFF000000) >> 24) )/255.0f/255.0f; \
  rdp.coladd[1] *= (float)( ((rdp.prim_color & 0x00FF0000) >> 16) * ((rdp.env_color & 0x00FF0000) >> 16) )/255.0f/255.0f; \
  rdp.coladd[2] *= (float)( ((rdp.prim_color & 0x0000FF00) >> 8) * ((rdp.env_color & 0x0000FF00) >> 8) )/255.0f/255.0f; \
  rdp.cmb_flags |= CMB_SUB; \
}

#define COLSUBSHADE_PRIM() { \
  rdp.coladd[0] *= (float)((rdp.prim_color & 0xFF000000) >> 24) / 255.0f; \
  rdp.coladd[1] *= (float)((rdp.prim_color & 0x00FF0000) >> 16) / 255.0f; \
  rdp.coladd[2] *= (float)((rdp.prim_color & 0x0000FF00) >> 8) / 255.0f; \
  rdp.cmb_flags |= CMB_COL_SUB_OWN; \
}

#define INTERSHADE_2(color,factor) { \
  rdp.col_2[0] = (((color) >> 24) & 0xFF) / 255.0f; \
  rdp.col_2[1] = (((color) >> 16) & 0xFF) / 255.0f; \
  rdp.col_2[2] = (((color) >> 8) & 0xFF) / 255.0f; \
  rdp.shade_factor = (factor) / 255.0f; \
  rdp.cmb_flags_2 = CMB_INTER; \
}

#define MULSHADE_SHADEA() rdp.cmb_flags |= CMB_MULT_OWN_ALPHA;

#define CA(color) cmb.ccolor|=(color)&0xFF
#define CA_PRIM() CA(rdp.prim_color)
#define CA_ENV() CA(rdp.env_color)
#define CA_INVPRIM() cmb.ccolor|=0xFF-(rdp.prim_color&0xFF)
#define CA_INVENV() cmb.ccolor|=0xFF-(rdp.env_color&0xFF)
#define CA_ENV1MPRIM() cmb.ccolor|= (wxUint32)(((rdp.env_color&0xFF)/255.0f) * (((~(rdp.prim_color&0xFF)) & 0xff)/255.0f) * 255.0f);
#define CA_PRIMENV() cmb.ccolor |= (wxUint32)(((rdp.env_color&0xFF)/255.0f) * ((rdp.prim_color&0xFF)/255.0f) * 255.0f);
#define CA_PRIMLOD() cmb.ccolor |= rdp.prim_lodfrac;
#define CA_PRIM_MUL_PRIMLOD() cmb.ccolor |= (int)(((rdp.prim_color&0xFF) * rdp.prim_lodfrac) / 255.0f);
#define CA_ENV_MUL_PRIMLOD() cmb.ccolor |= (int)(((rdp.env_color&0xFF) * rdp.prim_lodfrac) / 255.0f);

#define XSHADE_A(color, flag) { \
  rdp.col[3] *= (float)(color & 0xFF) / 255.0f; \
  rdp.cmb_flags |= flag; \
}
#define XSHADE1M_A(color, flag) { \
  rdp.col[3] *= 1.0f-((float)(color & 0xFF) / 255.0f); \
  rdp.cmb_flags |= flag; \
}
#define XSHADEC1MC2_A(color1, color2, flag) { \
  rdp.col[3] *= (float)( max(0, (int)(color1 & 0xFF) - (int)(color2 & 0xFF)) ) / 255.0f; \
  rdp.cmb_flags |= flag; \
}
#define MULSHADE_A_PRIM() XSHADE_A(rdp.prim_color, CMB_A_MULT)
#define MULSHADE_A_1MPRIM() XSHADE1M_A(rdp.prim_color, CMB_A_MULT)
#define MULSHADE_A_ENV() XSHADE_A(rdp.env_color, CMB_A_MULT)
#define MULSHADE_A_PRIMSUBENV() XSHADEC1MC2_A(rdp.prim_color, rdp.env_color, CMB_A_MULT)
#define MULSHADE_A_ENVSUBPRIM() XSHADEC1MC2_A(rdp.env_color, rdp.prim_color, CMB_A_MULT)
#define SETSHADE_A(color) XSHADE_A(color, CMB_A_SET)
#define SETSHADE_A_PRIM() SETSHADE_A(rdp.prim_color)
#define SETSHADE_A_ENV() SETSHADE_A(rdp.env_color)
#define SETSHADE_A_PRIMSUBENV() XSHADEC1MC2_A(rdp.prim_color, rdp.env_color, CMB_A_SET)
#define SETSHADE_A_INVENV() XSHADE1M_A(rdp.env_color, CMB_A_SET)

#define XSHADEADD_A(color, flag) { \
  rdp.coladd[3] *= (float)(color & 0xFF) / 255.0f; \
  rdp.cmb_flags |= flag; \
}
#define SUBSHADE_A_PRIM() XSHADEADD_A(rdp.prim_color, CMB_A_SUB)
#define SUBSHADE_A_ENV() XSHADEADD_A(rdp.env_color, CMB_A_SUB)
#define ADDSHADE_A_PRIM() XSHADEADD_A(rdp.prim_color, CMB_A_ADD)
#define ADDSHADE_A_ENV() XSHADEADD_A(rdp.env_color, CMB_A_ADD)

//****************************************************************
// Combine Functions
//****************************************************************

// These are in a somewhat ordered way, using the A constants below.  T0 comes before
//  T1 comes before PRIM, ... except for CMB, which always comes at the end, where
//  the CMB comes first in the name.  T0 and T1 are always interleaved, because they use the
//  same function.
// Keep going in alphabetical order, but do not break the order of variables!
//  ex: A*C + B*C -> T0_MUL_PRIM_ADD_ENV_MUL_PRIM,
// Although prim comes before env, we have already used prim as C, so it must stay as C
//  and would NOT become T0_MUL_PRIM_ADD_PRIM_MUL_ENV
//
// New version ordered by:
// t0
// prim
// env
// shade

static void cc_one ()
{
  CCMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  //  CC (0xFFFFFF00);
  CC (0xFFFFFF00);
}

static void cc_zero ()
{
  CCMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  CC (0x00000000);
}

static void cc_t0 ()
{
  if ((rdp.othermode_l & 0x4000) && (rdp.cycle_mode < 2))
  {
    wxUint32 blend_mode = (rdp.othermode_l >> 16);
    if (blend_mode == 0xa500)
    {
      CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
        GR_COMBINE_FACTOR_ONE,
        GR_COMBINE_LOCAL_CONSTANT,
        GR_COMBINE_OTHER_TEXTURE);
      float fog = (rdp.fog_color&0xFF)/255.0f;
      wxUint32 R = (wxUint32)(((rdp.blend_color>>24)&0xFF)*fog);
      wxUint32 G = (wxUint32)(((rdp.blend_color>>16)&0xFF)*fog);
      wxUint32 B = (wxUint32)(((rdp.blend_color>> 8)&0xFF)*fog);
      CC((R<<24)|(G<<16)|(B<<8));
    }
    else if (blend_mode == 0x55f0) //cmem*afog + cfog*1ma
    {
      CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
        GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA,
        GR_COMBINE_LOCAL_NONE,
        GR_COMBINE_OTHER_CONSTANT);
      CC(rdp.fog_color);
      A_USE_T0 ();
    }
    else
    {
      CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
        GR_COMBINE_FACTOR_ONE,
        GR_COMBINE_LOCAL_NONE,
        GR_COMBINE_OTHER_TEXTURE);
    }
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
  }
  USE_T0 ();
}

static void cc_t0a ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_CONSTANT);
  USE_T0 ();
  A_USE_T0 ();
  CC (0xFFFFFF00);
}

static void cc_t1 () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T1 ();
}

static void cc_t0_mul_t1 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  T0_MUL_T1 ();
}

static void cc_t0_mul_t1_add_t0 () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  T0_MUL_T1_ADD_T0 ();
}

/*
static void cc_t1_inter__env_inter_t0_using_k5__using_t1a ()
{
CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
GR_COMBINE_FACTOR_ONE,
GR_COMBINE_LOCAL_NONE,
GR_COMBINE_OTHER_TEXTURE);
wxUint32 col1 = (rdp.K5<<24) | (rdp.K5<<16) | (rdp.K5<<8);
MOD_0 (TMOD_COL_INTER_TEX_USING_COL1);
MOD_0_COL (rdp.env_color & 0xFFFFFF00);
MOD_0_COL1 (col1 & 0xFFFFFF00);
rdp.best_tex = 0;
cmb.tex |= 3;
cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL;
cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL;
cmb.tmu0_func = GR_COMBINE_FUNCTION_BLEND;
cmb.tmu0_fac = GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA;
}
*/

static void cc_t1_inter_t0_using_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  T1_INTER_T0_USING_ENV ();
}

static void cc_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  CC_PRIM ();
}

static void cc_env ()
{
  CCMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  CC_ENV ();
}

static void cc_scale ()
{
  CCMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  CC (rdp.SCALE);
}

static void cc_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_NONE);
}

static void cc_one_mul_shade ()
{
  if ((settings.hacks&hack_Knockout) && (rdp.aTBuffTex[0] || rdp.aTBuffTex[1] || rdp.cur_image)) //hack for boxer shadow
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CC (0x20202000);
    USE_T0 ();
  }
  else
  {
    cc_shade ();
  }
}

static void cc_shadea ()
{
  CCMB (GR_COMBINE_FUNCTION_LOCAL_ALPHA,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_NONE);
}

static void cc_t0_mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  USE_T0 ();
}

static void cc_t0_mul_prima ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIMA ();
  USE_T0 ();
}

static void cc_t1_mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  if ((rdp.cycle1 & 0xFFFF) == (rdp.cycle2 & 0xFFFF)) // 1 cycle, use t0
  {
    USE_T0 ();
  }
  else
  {
    USE_T1 ();
  }
}

static void cc_t0a_mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  A_USE_T0 ();
}

//Added by Gonetz
static void cc__t1_inter_t0_using_enva__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T1_INTER_T0_USING_FACTOR (factor);
}

static void cc__t0_inter_one_using_t1__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  rdp.best_tex = 0;
  cmb.tex |= 3;
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL;
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL;
  cmb.tmu0_fac = GR_COMBINE_FACTOR_ONE_MINUS_LOCAL;
}

static void cc__t0_inter_one_using_primlod__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    MOD_0 (TMOD_TEX_INTER_COLOR_USING_FACTOR);
    MOD_0_COL (0xFFFFFF00);
    MOD_0_FAC (lod_frac);
    USE_T0 ();
  }
}

static void cc__t1_inter_one_using_env__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 1,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 2;
    cmb.tex_ccolor = rdp.env_color;
  }
  else
  {
    USE_T1 ();
  }
}

static void cc__t1_inter_one_using_enva__mul_t0 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 3;
    cmb.tex_ccolor = 0xFFFFFF00 | (rdp.env_color&0xFF);
  }
  else
  {
    if ((rdp.env_color&0xFF) == 0xFF)
    {
      USE_T0 ();
    }
    else
    {
      T0_MUL_T1 ();
    }
  }
}

//Added by Gonetz
static void cc_prim_mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_PRIM ();
}

static void cc_prim_mul_prima ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  SETSHADE_PRIMA ();
}

static void cc_t1_mul_prima ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIMA ();
  USE_T1 ();
}

static void cc_t1_mul_enva ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENVA ();
  USE_T1 ();
}

static void cc_t0_mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  USE_T0 ();
}

static void cc_t1_mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  USE_T1 ();
}

//Added by Gonetz
static void cc_t0_mul_enva ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENVA ();
  USE_T0 ();
}

static void cc_t0_mul_scale ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC (rdp.SCALE);
  USE_T0 ();
}

static void cc_t0_mul_enva_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  SETSHADE_PRIM ();
  CC_ENVA ();
  USE_T0 ();
}

static void cc_t0_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();
}

static void cc_f1_sky ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  MULSHADE_SHADEA ();
  MULSHADE_ENVSUBPRIM ();
  ADDSHADE_PRIM();
  CC(0xFFFFFFFF);
}

static void cc_t0_mul_shadea ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_SHADE_A ();
  USE_T0 ();
}

static void cc_t0_mul_k5 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_K5 ();
  USE_T0 ();
}

static void cc_t1_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T1 ();
}

//Added by Gonetz
static void cc__t0_add_t1__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_ADD_T1 ();
}

static void cc__t0_mul_shade__add__t1_mul_shade ()
{
  //combiner is used in Spiderman. It seems that t0 is used instead of t1
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_B, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    USE_T0 ();
  }
}

static void cc__t0_mul_prim__inter_env_using_enva ()
{
  wxUint32 enva  = rdp.env_color&0xFF;
  if (enva == 0xFF)
    cc_env ();
  else if (enva == 0)
    cc_t0_mul_prim ();
  else if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.prim_color;
    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    SETSHADE_ENV();
    CC_ENVA();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    SETSHADE_PRIM();
    INTERSHADE_2 (rdp.env_color & 0xFFFFFF00, rdp.env_color & 0xFF);
    USE_T0 ();
    MOD_0 (TMOD_TEX_INTER_COLOR_USING_FACTOR);
    MOD_0_COL (rdp.env_color & 0xFFFFFF00);
    MOD_0_FAC (rdp.env_color & 0xFF);
  }
}


static void cc__t1_inter_t0_using_t1__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_B, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
  }
  else
  {
    T0_INTER_T1_USING_FACTOR (0x7F);
  }
}

//Added by Gonetz
static void cc__t1_inter_t0_using_enva__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T1_INTER_T0_USING_FACTOR (factor);
}

//Added by Gonetz
static void cc__t1_inter_t0_using_shadea__mul_shade ()
{
  if (!cmb.combine_ext) {
    cc_t0_mul_shade ();
    return;
  }
  CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_ZERO, 0);
  T1_INTER_T0_USING_SHADEA ();
}

//Added by Gonetz
static void cc__t0_inter_one_using_prim__mul_shade ()
{
  // (1-t0)*prim+t0, (cmb-0)*shade+0
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.prim_color;
  }
  else
  {
    USE_T0 ();
    MOD_0 (TMOD_TEX_INTER_COL_USING_COL1);
    MOD_0_COL (0xFFFFFF00);
    MOD_0_COL1 (rdp.prim_color & 0xFFFFFF00);
  }
}

static void cc__t0_inter_one_using_primlod__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    MOD_0 (TMOD_TEX_INTER_COLOR_USING_FACTOR);
    MOD_0_COL (0xFFFFFF00);
    MOD_0_FAC (lod_frac);
    USE_T0 ();
  }
}

//Added by Gonetz
static void cc__t0_inter_env_using_enva__mul_shade ()
{
  // (env-t0)*env_a+t0, (cmb-0)*shade+0
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.env_color;
  }
  else
  {
    USE_T0 ();
    MOD_0 (TMOD_TEX_INTER_COLOR_USING_FACTOR);
    MOD_0_COL (rdp.env_color & 0xFFFFFF00);
    MOD_0_FAC (rdp.env_color&0xFF);
  }
}

//Added by Gonetz
static void cc__t0_inter_env_using_shadea__mul_shade ()
{
  // (env-t0)*shade_a+t0, (cmb-0)*shade+0
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.env_color;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    cc_t0_mul_shade ();
  }
}

static void cc__t0_mul_prim_add_env__mul_shade ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.prim_color;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MOD_0 (TMOD_TEX_SCALE_COL_ADD_COL);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    MOD_0_COL1 (rdp.env_color & 0xFFFFFF00);
	USE_T0 ();
  }
}

static void cc__t1_sub_t0_mul_primlod_add_prim__mul_shade ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 3;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    T0_INTER_T1_USING_FACTOR (lod_frac);
  }
}

static void cc__t1_sub_prim_mul_t0__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 3;
  }
  else
  {
    T0_MUL_T1 ();
  }
}

static void cc__t1_sub_t0_mul_t0_add_shade__mul_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_ITRGB, 0);
    cmb.tex |= 3;
  }
  else
  {
    T1_SUB_T0_MUL_T0 ();
  }
}

static void cc__one_sub_shade_mul_t0_add_shade__mul_shade ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
	  GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    USE_T0 ();
  }
}

static void cc__t0_sub_prim_mul_t1_add_t1__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (rdp.prim_color & 0xFFFFFF00)
  {
    MOD_0 (TMOD_TEX_SUB_COL);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
  }
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
  }
  else
  {
    T0_MUL_T1 ();
  }
}

static void cc__t1_sub_env_mul_t0_add_t0__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 3;
  }
  else
  {
    MOD_1 (TMOD_TEX_SUB_COL);
    MOD_1_COL (rdp.env_color & 0xFFFFFF00);
    T0_MUL_T1_ADD_T0 ();
  }
}

static void cc__t0_mul_prima_add_prim_mul__shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
  }
  else
  {
    MOD_0 (TMOD_TEX_SCALE_FAC_ADD_COL);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    MOD_0_FAC (rdp.prim_color & 0xFF);
    USE_T0 ();
  }
}

static void cc__t0_inter_prim_using_prima__inter_env_using_enva ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    CC_ENVA ();
    SETSHADE_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
    CC_1SUBENVA ();
    SETSHADE_ENV ();
    SETSHADE_ENVA ();
    MOD_0 (TMOD_TEX_INTER_COLOR_USING_FACTOR);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    MOD_0_FAC (rdp.prim_color & 0xFF);
    USE_T0 ();
  }
}

static void cc_prim_inter_t1_mul_shade_using_texa ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 3;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TEXTURE_ALPHA, 0,
      GR_CMBX_B, 0);
  }
  else
  {
    cc_t1_mul_shade ();
  }
}

static void cc__prim_inter_t0_using_t0a__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
  }
  else
  {
    MOD_0 (TMOD_COL_INTER_TEX_USING_TEXA);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    USE_T0 ();
  }
}

static void cc__prim_inter_t0_using_t0a__inter_env_using_enva ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    CC_ENVA ();
    SETSHADE_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
    CC_1SUBENVA ();
    SETSHADE_ENV ();
    SETSHADE_ENVA ();
    MOD_0 (TMOD_COL_INTER_TEX_USING_TEXA);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    USE_T0 ();
  }
}

// ** A*B **

static void cc__prim_inter_t0_using_shadea__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  PRIM_INTER_T0_USING_SHADEA ();
}

static void cc_t0_sub_shade_mul_shadea_add_shade ();
static void cc__shade_inter_t0_using_shadea__mul_shade ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    cc_t0_sub_shade_mul_shadea_add_shade ();
  }
}

static void cc__prim_inter_env_using_enva__mul_shade ()
{
  const float ea = ((float)(rdp.env_color&0xFF)) / 255.0f;
  const float ea_i = 1.0f - ea;
  wxUint32 pr = (rdp.prim_color >> 24)&0xFF;
  wxUint32 pg = (rdp.prim_color >> 16)&0xFF;
  wxUint32 pb = (rdp.prim_color >>  8)&0xFF;
  wxUint32 er = (rdp.env_color >> 24)&0xFF;
  wxUint32 eg = (rdp.env_color >> 16)&0xFF;
  wxUint32 eb = (rdp.env_color >>  8)&0xFF;
  wxUint32 r = min(255, (wxUint32)(er*ea + pr*ea_i));
  wxUint32 g = min(255, (wxUint32)(eg*ea + pg*ea_i));
  wxUint32 b = min(255, (wxUint32)(eb*ea + pb*ea_i));
  wxUint32 col = (r << 24) | (g << 16) | (b << 8) | 0xFF;
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC (col);
}

//Added by Gonetz
static void cc_prim_mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
}

static void cc_prim_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
}

static void cc_prim_mul_shadea ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  SETSHADE_SHADE_A ();
  CC_PRIM ();
}

static void cc_env_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
}

static void cc_env_mul_enva ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_OTHER_ALPHA,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  CA_ENV ();
}

static void cc_scale_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC (rdp.SCALE);
}

// ** A+B **

static void cc_t0_add_prim () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  USE_T0 ();
}

static void cc__t0_mul_t1__add_prim () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  T0_MUL_T1 ();
}

static void cc_t0_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  USE_T0 ();
}

//Added by Gonetz
static void cc__t0_mul_t1__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  T0_MUL_T1 ();
}

static void cc__t0_mul_t1__add_env_mul__t0_mul_t1__add_env ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 3;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
      GR_CMBX_TEXTURE_RGB, 0,
      GR_CMBX_ZERO, 0);
  }
  else
    cc__t0_mul_t1__add_env();
}

static void cc_t0_add_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();
}

static void cc__t0_mul_t1__add_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_MUL_T1 ();
}

static void cc_prim_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
}

static void cc_t0_add_prim_mul_one_sub_t0_add_t0 () //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, 1,
      GR_CMBX_B, 0);
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
  }
  else
  {
    cc_t0_add_prim ();
  }
}

static void cc_one_sub_prim_mul_t0_add_prim();
static void cc__one_sub_prim_mul_t0_add_prim__mul_prima_add__one_sub_prim_mul_t0_add_prim () //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 1,
      GR_CMBX_B, 0);
    CCMBEXT(GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    CC_PRIMA();
    cmb.tex |= 3; //hw frame buffer allocated as tile1, but not used in combiner
  }
  else
  {
    cc_one_sub_prim_mul_t0_add_prim();
    //    cc_t0 ();
  }
}

static void cc_prim_add_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
}

static void cc_env_add_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
}

static void cc_shade_add_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_ITERATED);
}

// ** A-B **
static void cc__t0_inter_t1_using_enva__sub_env () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

static void cc_t0_sub__shade_mul_center ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE(rdp.CENTER);
  USE_T0 ();
}

// ** A-B*C **
static void cc_env_sub__t0_sub_t1_mul_primlod__mul_prim () //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 3;
    CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    SETSHADE_PRIM ();
    SETSHADE_PRIMLOD ();
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    SETSHADE_PRIM ();
    CC_ENV ();
    T1_INTER_T0_USING_FACTOR (lod_frac);
  }
}

static void cc_env_sub__t0_mul_scale_add_env__mul_prim ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.SCALE;
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    SETSHADE_ENV ();
    CC_PRIM ();
  }
  else
    cc_t0_add_env ();
}

static void cc_one_sub__one_sub_t0_mul_enva_add_prim__mul_prim () //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
    percent = (float)(rdp.env_color&0xFF) / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    CCMBEXT(GR_CMBX_ZERO, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_ZERO, 1);
    CC_PRIM ();
  }
  else
  {
    cc_one ();
  }
}

// ** A+B*C **
//Aded by Gonetz
static void cc_t0_add_env_mul_k5 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  float scale = rdp.K5 / 255.0f;
  wxUint8 r = (wxUint8)(rdp.env_color >> 24) & 0xFF;
  r = (wxUint8)(r*scale);
  wxUint8 g = (wxUint8)(rdp.env_color >> 16) & 0xFF;
  g = (wxUint8)(g*scale);
  wxUint8 b = (wxUint8)(rdp.env_color >>  8) & 0xFF;
  b = (wxUint8)(b*scale);
  CC((r<<24)|(g<<16)|(b<<8));
  USE_T0 ();
}

static void cc_t0_add_shade_mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_ENV ();
  USE_T0 ();
}

static void cc__t1_mul_t0_add_t0__add_prim_mul_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_PRIM ();
  rdp.best_tex = 0;
  cmb.tex |= 3;
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL;
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL;
  cmb.tmu0_fac = GR_COMBINE_FACTOR_LOCAL;
}

static void cc__t0_sub_env_mul_enva__add_prim_mul_shade ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 1;
    percent = (float)(rdp.env_color&0xFF) / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;

    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    CC_PRIM ();
  }
  else {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
	  GR_COMBINE_FACTOR_ONE,
	  GR_COMBINE_LOCAL_ITERATED,
	  GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_PRIM ();
	  MOD_0 (TMOD_TEX_SUB_COL_MUL_FAC);
	  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
	  MOD_0_FAC (rdp.env_color & 0xFF);
	USE_T0 ();
  }
}

// ** A*B+C **
//Added by Gonetz
static void cc_t0_mul_prim_add_t1 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
    cmb.tex_ccolor = rdp.prim_color;
  }
  else
  {
    MOD_0 (TMOD_TEX_MUL_COL);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    T0_ADD_T1 ();
  }
}

static void cc_shirt ()
{
  // (t1-0)*prim+0, (1-t0)*t1+cmb
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    /*
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_TMU_CCOLOR, 0,
    GR_CMBX_ZERO, 0);
    //*/
    //*
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    //*/
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 1,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
    cmb.tex_ccolor = rdp.prim_color;
  }
  else
  {
    MOD_1 (TMOD_TEX_MUL_COL);
    MOD_1_COL (rdp.prim_color & 0xFFFFFF00);
    T0_ADD_T1 ();
  }
}

static void cc_t1_mul_prim_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_PRIM ();
  CC_PRIM ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_t0_mul_prim_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_PRIM ();
  CC_ENV ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_t1_mul_prim_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_PRIM ();
  CC_ENV ();
  USE_T1 ();
}

static void cc__t0_add_primlod__mul_prim_add_env ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    CC_PRIMLOD ();
    cmb.tex_ccolor = cmb.ccolor;
    CC_ENV ();
    SETSHADE_PRIM ();
    cmb.tex |= 1;
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    CC_PRIMLOD ();
    MOD_0 (TMOD_TEX_ADD_COL);
    MOD_0_COL (cmb.ccolor & 0xFFFFFF00);
    SETSHADE_PRIM ();
    CC_ENV ();
    USE_T0 ();
  }
}

//Added by Gonetz
static void cc_t0_mul_prim_mul_shade_add_prim_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_PRIM ();
  USE_T0 ();
}

//Added by Gonetz
static void cc__t0_inter_t1_using_primlod__mul_prim_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_PRIM ();
  CC_ENV ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc__t1_sub_prim_mul_enva_add_t0__mul_prim_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_ZERO, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 3;
    percent = (float)(rdp.env_color&0xFF) / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    MOD_1 (TMOD_TEX_SUB_COL_MUL_FAC);
    MOD_1_COL (rdp.prim_color & 0xFFFFFF00);
    MOD_1_FAC (rdp.env_color & 0xFF);
    T0_ADD_T1 ();
  }
}

//Added by Gonetz
static void cc__t0_inter_t1_using_primlod__mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

//Added by Gonetz
static void cc__t1_sub_prim_mul_primlod_add_t0__mul_prim_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_PRIM ();
  CC_ENV ();
  T1_SUB_PRIM_MUL_PRIMLOD_ADD_T0 ();
}

//Aded by Gonetz
static void cc__t0_mul_t1__mul_prim_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  MULSHADE_PRIM ();
  T0_MUL_T1 ();
}

//Aded by Gonetz
static void cc__t0_mul_t1__sub_prim_mul_env_add_shade ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_ZERO, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 3;
    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_TEXTURE_RGB, 0);
    CC_PRIMMULENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
    CC_ENV ();
    T0_MUL_T1 ();
  }
}

static void cc__t0_sub_prim_mul_t1_add_t1__mul_env_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  if (rdp.prim_color & 0xFFFFFF00)
  {
    MOD_0 (TMOD_TEX_SUB_COL);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
  }
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
  }
  else
  {
    T0_MUL_T1 ();
  }
}

static void cc__t0_mul_t1__mul_shade_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  T0_MUL_T1 ();
}

static void cc__t0_mul_shadea_add_env__mul_shade_add_prim ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    MULSHADE_SHADEA ();
    CC_PRIM ();
    USE_T0 ();
  }
}

static void cc__t0_mul_t1__mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  T0_MUL_T1 ();
}

//Added by Gonetz
static void cc__t0_add_t1__mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  T0_ADD_T1 ();
}

static void cc__t1_mul_prima_add_t0__mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  T1_MUL_PRIMA_ADD_T0 ();
}

static void cc__t0_inter_t1_using_enva__mul_shade_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

static void cc__t0_inter_t1_using_enva__mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

//Added by Gonetz
static void cc_t0_mul_primlod_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  SETSHADE_PRIM ();
  CC_PRIMLOD ();
  USE_T0 ();
}

static void cc__t0_mul_primlod__add__prim_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  MULSHADE_PRIM ();
  CC_PRIMLOD ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_t0_mul_primlod_add_prim_mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  MULSHADE_PRIM ();
  ADDSHADE_ENV ();
  CC_PRIMLOD ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_t1_mul_primlod_add_prim_mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  MULSHADE_PRIM ();
  ADDSHADE_ENV ();
  CC_PRIMLOD ();
  USE_T1 ();
}

static void cc__t0_inter_t1_using_primlod__mul_shade_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc__t1_inter_t0_using_primlod__mul_shade_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  T1_INTER_T0_USING_FACTOR (lod_frac);
}

//Added by Gonetz
static void cc__t1_sub_t0_mul_primlod_add_prim__mul_shade_add_shade ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 3;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ITRGB, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    T0_INTER_T1_USING_FACTOR (lod_frac);
  }
}

//Added by Gonetz
static void cc__t0_inter_t1_using_half__mul_prim_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_PRIM ();
  CC_ENV ();
  T0_INTER_T1_USING_FACTOR (0x7F);
}

//Added by Gonetz
static void cc__t0_inter_t1_using_t1__mul_prim_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  T0_INTER_T1_USING_T1 ();
}

//Added by Gonetz
static void cc_one_sub_t1_mul_t0a_add_t0_mul_env_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  MOD_0 (TMOD_TEX_MUL_COL);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  rdp.best_tex = 0;
  cmb.tex |= 3;
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL;
  cmb.tmu1_invert = 1;
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL;
  cmb.tmu0_fac = GR_COMBINE_FACTOR_LOCAL_ALPHA;
}

//Added by Gonetz
static void cc__t0_inter_t1_using_t1__mul_shade_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  T0_INTER_T1_USING_T1 ();
}

//Added by Gonetz
static void cc_t0_mul_prim_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  USE_T0 ();
}

static void cc_t1_mul_prim_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  USE_T1 ();
}

//Added by Gonetz
static void cc_t0_mul_env_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_ENV ();
  CC_PRIM ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_t1_mul_env_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_ENV ();
  CC_PRIM ();
  USE_T1 ();
}

static void cc_t0_mul_scale_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE (rdp.SCALE);
  CC_PRIM ();
  USE_T0 ();
}

//Added by Gonetz
static void cc__t0_mul_t1__mul_env_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_ENV ();
  CC_PRIM ();
  T0_MUL_T1 ();
}

//Added by Gonetz
static void cc__t0_add__t1_mul_scale__mul_env_sub_center_add_prim ()
{
  // (t1-0)*scale+t0, (env-center)*cmb+prim
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_C1SUBC2(rdp.env_color, rdp.CENTER);
  SETSHADE_PRIM ();
  MOD_1 (TMOD_TEX_MUL_COL);
  MOD_1_COL (rdp.SCALE & 0xFFFFFF00);
  T0_ADD_T1 ();
}

//Added by Gonetz
static void cc__t1_sub_t0__mul_env_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_ENV ();
  CC_PRIM ();
  T1_SUB_T0 ();
}

//Added by Gonetz
static void cc_t0_mul_env_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  USE_T0 ();
}

static void cc_t0_mul_shade_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  USE_T0 ();
}

static void cc__t0_mul_enva_add_t1__mul_shade_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
    percent = (float)(rdp.env_color&0xFF) / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    T0_ADD_T1 ();
  }
}

static void cc_t0_mul_shade_add_prima ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIMA ();
  USE_T0 ();
}

static void cc_t1_mul_shade_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  USE_T1 ();
}

static void cc_t0_mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  USE_T0 ();
}

static void cc__t0_add_prim__mul_shade_add_t0 ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    //    MOD_0 (TMOD_TEX_ADD_COL);
    //    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
  }
  USE_T0 ();
}

static void cc__t0_add_prim__mul_shade_add_t1 ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
    CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_PRIM ();
    T0_ADD_T1 ();
  }
}

static void cc__t0_add_primlod__mul_shade_add_env ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    CC_PRIMLOD ();
    cmb.tex_ccolor = cmb.ccolor;
    CC_ENV ();
    cmb.tex |= 1;
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    wxUint32 color = (lod_frac<<24) | (lod_frac<<16) | (lod_frac<<8);
    MOD_0 (TMOD_TEX_ADD_COL);
    MOD_0_COL (color & 0xFFFFFF00);
    CC_ENV ();
    USE_T0 ();
  }
}

static void cc__t0_mul_prima_add_prim_mul__shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
  }
  else
  {
    MOD_0 (TMOD_TEX_SCALE_FAC_ADD_COL);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    MOD_0_FAC (rdp.prim_color & 0xFF);
    USE_T0 ();
  }
}

//Added by Gonetz
static void cc_t0_mul_shadea_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();
}

static void cc_prim_mul_prima_add_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  CA_PRIM ();
  SETSHADE_PRIM ();
}

static void cc_prim_mul_prima_add_t0 () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIM ();
  SETSHADE_PRIMA ();
  USE_T0 ();
}

static void cc_prim_mul_env_add_t0 () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIM ();
  SETSHADE_ENV ();
  USE_T0 ();
}

static void cc_prim_mul_shade_add_t0 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_PRIM ();
  USE_T0 ();
}

static void cc_prim_mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  MULSHADE_PRIM ();
}

static void cc_env_mul_shade_add_env ()  //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  MULSHADE_ENV ();
}

// ** A*B+C*D **
static void cc_t0_mul_prim_add_one_sub_prim_mul_shade () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  MULSHADE_1MPRIM ();
  USE_T0 ();
}

static void cc_t0_mul_prim_add_shade_sub_env_mul_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SUBSHADE_ENV ()
    MULSHADE_PRIM ();
  USE_T0 ();
}

static void cc_t0_mul_prim_add_shade_mul_shadea_mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  MULSHADE_PRIM ();
  MULSHADE_SHADEA ();
  USE_T0 ();
}

static void cc__t0_mul_t1__mul_prim_add_prim_mul_shade () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  MULSHADE_PRIM ();
  CC_PRIM ();
  T0_MUL_T1 ();
}

static void cc_t0_mul_env_add_prim_mul_shade () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  MULSHADE_PRIM ();
  CC_ENV ();
  USE_T0 ();
}

static void cc_t0_mul_enva_add_prim_mul_shade () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  MULSHADE_PRIM ();
  CC_ENVA ();
  USE_T0 ();
}

static void cc_t0_mul_shade_add_prim_mul_env () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIMMULENV ();
  USE_T0 ();
}

static void cc_prim_mul_env_add_one_sub_prim_mul_shade () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  MULSHADE_1MPRIM ();
  CC_PRIMMULENV ();
}

// ** A*B*C **

static void cc_t0_mul_prim_mul_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIM ();
  SETSHADE_PRIM ();
  USE_T0 ();
}

static void cc_t0_mul_prim_mul_prima () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIM ();
  SETSHADE_PRIMA ();
  USE_T0 ();
}

static void cc_t0_mul_enva_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_ENVA ();
  USE_T0 ();
}

static void cc_t0_mul_primlod_mul_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_COLMULBYTE (rdp.prim_color, rdp.prim_lodfrac);
  USE_T0 ();
}

static void cc_t0_mul_primlod_mul_shade () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_PRIMLOD ();
  USE_T0 ();
}

static void cc__t0_mul_t1__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  T0_MUL_T1 ();
}

static void cc__t1_mul_t1_add_t0__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_OTHER_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
  }
  else
  {
    T0_ADD_T1 ();
  }
}

static void cc__t0_mul_t1__mul_prima () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIMA ();
  T0_MUL_T1 ();
}

static void cc__t0_mul_t1__mul_env () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  T0_MUL_T1 ();
}

static void cc__t0_mul_t1__mul_enva () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENVA ();
  T0_MUL_T1 ();
}

static void cc__t0_mul_t1__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_MUL_T1 ();
}

static void cc__t0a_mul_t1__mul_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  T0A_MUL_T1 ();
}

static void cc__t0_mul_t1a__mul_shade () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_MUL_T1A ();
}

static void cc__t0a_mul_t1__mul_shade () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0A_MUL_T1 ();
}

static void cc_t0_mul_prim_mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIM ();
  SETSHADE_ENV ();  // notice that setshade multiplies
  USE_T0 ();
}

static void cc_t0_mul_prim_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_PRIM ();
  USE_T0 ();
}

static void cc_t0_mul_prim_mul_shadea ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIM ();
  MULSHADE_SHADEA();
  USE_T0 ();
}

static void cc_t0_mul_prima_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_PRIMA ();
  USE_T0 ();
}

static void cc_t1_mul__one_sub_prim_mul_shade_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_1MPRIM ();
  ADDSHADE_PRIM ();
  USE_T1 ();
}

static void cc_t0_mul_one_sub_env_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_1MENV ();
  USE_T0 ();
}

static void cc_t1_mul_prim_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_PRIM ();
  USE_T1 ();
}

//Added by Gonetz
static void cc_t0_mul_1mprim_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_1MPRIM ();
  USE_T0 ();
}

static void cc_t0_mul_env_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_ENV ();
  USE_T0 ();
}

static void cc_t0_mul_scale_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE (rdp.SCALE);
  USE_T0 ();
}

static void cc_t0_mul_shade_mul_shadea ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_SHADEA ();
  USE_T0 ();
}

static void cc_prim_mul_env_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  MULSHADE_PRIM ();
}

static void cc_prim_mul_one_sub_env_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_1SUBENV ();
  MULSHADE_PRIM ();
}

// ** A*B*C+D **
static void cc_t0_mul_prim_mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  MULSHADE_PRIM ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_t0_mul_prim_mul_shadea_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  SETSHADE_ENV ();
  MULSHADE_A_PRIM ();
  USE_T0 ();
}

// (A*B+C)*D
static void cc__t0_mul_prim_add_shade__mul_env ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.prim_color;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_ZERO, 0);
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_ENV ();
    MOD_0 (TMOD_TEX_MUL_COL);
    CC_PRIMMULENV ();
    MOD_0_COL (cmb.ccolor & 0xFFFFFF00);
    USE_T0 ();
  }
}

static void cc__t0a_mul_prim_add_t0__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MOD_0 (TMOD_COL_MUL_TEXA_ADD_TEX);
  MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
  USE_T0 ();
}

static void cc__t0a_mul_env_add_t0__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MOD_0 (TMOD_COL_MUL_TEXA_ADD_TEX);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  USE_T0 ();
}

static void cc__prim_mul_shade_add_env__mul_shade () //Aded by Gonetz
{
  if (!cmb.combine_ext)
  {
    cc_prim_mul_shade_add_env ();
    return;
  }
  T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
    GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_ZERO, 0);
  cmb.tex |= 1;
  cmb.tex_ccolor = rdp.prim_color;
  CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_ZERO, 0);
  CC_ENV ();
}

// ** A*B*C+D*E **
//Added by Gonetz
static void cc__t0_sub_t1__mul_prim_mul_shade_add_prim_mul_env ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 3;
  }
  else
  {
    USE_T0 ();
  }

  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIMMULENV ();
  MULSHADE_PRIM ();
}

static void cc__t0_mul_prim_mul_env__add__prim_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIMMULENV ();
  MULSHADE_PRIM ();
  USE_T0 ();
}

static void cc__t1_mul_prim_mul_env__add__prim_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIMMULENV ();
  MULSHADE_PRIM ();
  USE_T1 ();
}

//Added by Gonetz
static void cc_t0_mul_one_sub_prim_mul_shade_add_prim_mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIMMULENV ();
  MULSHADE_1MPRIM ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_t0_mul_one_sub_prim_mul_shadea_add_prim_mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIMMULENV ();
  SETSHADE_1MPRIM ();
  MULSHADE_SHADEA ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_t0_mul_one_sub_env_mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  MULSHADE_1MENV ();
  USE_T0 ();
}

static void cc_t0_mul_prima_mul_shade_add_prim_mul_one_sub_prima ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  MULSHADE_PRIMA ();
  USE_T0 ();
  wxUint8 fac = 255 - (wxUint8)(rdp.prim_color&0xFF);
  float col[3];
  col[0] = (float)((rdp.prim_color & 0xFF000000) >> 24) / 255.0f;
  col[1] = (float)((rdp.prim_color & 0x00FF0000) >> 16) / 255.0f;
  col[2] = (float)((rdp.prim_color & 0x0000FF00) >> 8) / 255.0f;
  CC ( ((wxUint8)(col[0]*fac))<<24 | ((wxUint8)(col[1]*fac))<<16 | ((wxUint8)(col[2]*fac))<<8 | fac );
}

// ** A*(1-B)+C **
static void cc_t0_mul_1menv_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  SETSHADE_1MENV ();
  USE_T0 ();
}

// ** (A+B)*C **
static void cc_t0_mul_scale_add_prim__mul_shade () //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.SCALE;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MOD_0 (TMOD_TEX_ADD_COL);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    USE_T0 ();
  }
}

static void cc__t0_mul_t1_add_prim__mul_shade () //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_PRIM ();
  }
  T0_MUL_T1 ();
}

static void cc_t0_mul__prim_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIM ();
  ADDSHADE_ENV ();
  USE_T0 ();
}

static void cc_t0_mul__prim_mul_primlod_add_env () //Aded by Gonetz
{
  // forest behind window, Dobutsu no Mori.
  // (prim-0)*prim_lod+env, (t1-0)*cmb+0
  //actually, the game uses t0 instead of t1 here. t1 does not set at all this moment.
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  float prim_lod = rdp.prim_lodfrac / 65025.0f;
  rdp.col[0] *= ((rdp.prim_color & 0xFF000000) >> 24) * prim_lod;
  rdp.col[1] *= ((rdp.prim_color & 0x00FF0000) >> 16) * prim_lod;
  rdp.col[2] *= ((rdp.prim_color & 0x0000FF00) >> 8) * prim_lod;
  rdp.cmb_flags = CMB_SET;
  ADDSHADE_ENV ();
  USE_T0 ();
}

// ** (A-B)*C **
static void cc__t0_mul_prim_add_shade__sub_env_mul_shade ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_ENV ();
  }
  else
  {
    cc_t0_mul_prim_mul_shade ();
  }
}

static void cc_t0_sub_prim_mul_shadea ()  //Aded by Gonetz
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIM ();
  USE_T0 ();
}

static void cc__t0_sub_env_mul_shade__sub_prim_mul_shade ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ITRGB, 0);
    CC_PRIM ();
  }
  else
  {
    cc_t0_mul_shade ();
  }
}

static void cc_t0_sub_prim_mul_shade ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    if (rdp.prim_color & 0xFFFFFF00)
    {
      MOD_0 (TMOD_TEX_SUB_COL);
      MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    }
  }
  USE_T0 ();
}

static void cc__t0_mul_t1__sub_prim_mul_shade ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
  }
  T0_MUL_T1 ();
}

static void cc_t0_sub_env_mul_shade ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    if (rdp.env_color & 0xFFFFFF00)
    {
      MOD_0 (TMOD_TEX_SUB_COL);
      MOD_0_COL (rdp.env_color & 0xFFFFFF00);
    }
  }
  USE_T0 ();
}

static void cc__t0_mul_prima_add_t0__sub_center_mul_scale ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_B, 0);
    wxUint32 prima = rdp.prim_color&0xFF;
    cmb.tex_ccolor = (prima<<24)|(prima<<16)|(prima<<8)|prima;
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC(rdp.CENTER);
    SETSHADE(rdp.SCALE);
  }
  else
  {
    cc_t0_mul_prima();
  }
}

static void cc__t1_inter_t0_using_primlod__sub_shade_mul_prim ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_PRIM ();
  }
  T1_INTER_T0_USING_FACTOR (lod_frac);
}

static void cc__t0_inter_t1_using_enva__sub_shade_mul_prim ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_PRIM ();
  }
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

static void cc_t0_sub_shade_mul_shadea ()  //Aded by Gonetz
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();
}

static void cc_one_sub_t0_mul_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_BLEND_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  CC_PRIM ();
  USE_T0 ();
}

static void cc_one_sub_prim_mul_prima () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC (~rdp.prim_color);
  SETSHADE_PRIMA ();
}

static void cc_shade_sub_prim_mul_t0 () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  USE_T0 ();
}

static void cc_shade_sub_prim_mul_env () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SUBSHADE_PRIM ();
}

static void cc_shade_sub_env_mul_t0 () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  USE_T0 ();
}

static void cc_shade_sub_prim_mul__t0_inter_t1_using_primlod () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc_shade_sub_env_mul__t0_inter_t1_using_primlod () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc_shade_sub_env_mul_prim () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM();
  SUBSHADE_ENV ();
}

static void cc_shade_sub__prim_mul_prima () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_C1MULC2 (rdp.prim_color, (rdp.prim_color&0xFF));
}

static void cc_one_sub__t0_mul_t1__mul_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC (0);
  T0_MUL_T1 ();
}

static void cc_one_sub__t0_mul_shadea__mul_shade () //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;

    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    USE_T0 ();
    cmb.tmu0_invert = TRUE;
  }
}

static void cc_one_sub_env_mul_t0 () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  USE_T0 ();
}

static void cc_one_sub_env_mul__t0_inter_t1_using_primlod () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc_one_sub_env_mul_prim () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
}

static void cc_one_sub_env_mul_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
}

// ** (1-A)*B + A*C **
static void cc_t0_mul_env_add_1mt0_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  USE_T0 ();
}

// ** (1-A)*B+C **
static void cc_one_sub_shade_mul__t1_sub_prim_mul_primlod_add_t0__add_shade () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T1_SUB_PRIM_MUL_PRIMLOD_ADD_T0 ();
}

// ** (1-A)*B*C **
static void cc_one_sub_t0_mul_prim_mul_shade () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_BLEND_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_NONE);
  MULSHADE_PRIM ();
  USE_T0 ();
}

// ** (A-B)*C*D **
static void cc_prim_sub_env_mul_t0_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_ITERATED);
  MULSHADE_PRIMSUBENV ();
  USE_T0 ();
}

// ** (A-B)*C+D **
static void cc_t0_sub_t1_mul_prim_mul_shade_add_t1 ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    MULSHADE_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CC_PRIM ();
    T0_ADD_T1 ();
  }
}

static void cc_t0_sub_prim_mul_t0a_add_prim ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  USE_T0 ();
}

static void cc_t0_sub_prim_mul_t1_add_shade ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 3;
    cmb.tex_ccolor = rdp.prim_color;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_B, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MOD_0 (TMOD_TEX_SUB_COL);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    T0_MUL_T1 ();
  }
}

static void cc_t0_sub_prim_mul_primlod_add_prim ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    SETSHADE_PRIM ();
    CC_PRIMLOD ();
  }
  else
  {
    // * not guaranteed to work if another iterated alpha is set
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
    SETSHADE_PRIM ();
    SETSHADE_1MPRIMLOD ();
    CC_PRIMLOD ();
  }
  USE_T0 ();
}

static void cc_t0_sub_prim_mul_prima_add_prim ()  //Aded by Gonetz
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  SETSHADE_PRIM ();
  SETSHADE_1MPRIMA ();
  CC_PRIMA ();
  USE_T0 ();
}

static void cc_t0_sub_prim_mul_shadea_add_prim ()  //Aded by Gonetz
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIM ();
  USE_T0 ();
}

static void cc_t0_sub_prim_mul_env_add_shade ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.prim_color;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    CC_ENV ();
  }
  else
  {
    cc_t0_mul_env_add_shade ();
  }
}

static void cc__t0_inter_t1_using_shadea__sub_prim_mul_env_add_shade ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    //have to pass shade alpha to combiner
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
  }
  CC_ENV ();
  SUBSHADE_PRIMMULENV ();
  T0_INTER_T1_USING_SHADEA ();
}


static void cc_t0_sub_prim_mul_env_add_prim ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  SETSHADE_1MENV ();
  USE_T0 ();
}

static void cc_t0_sub_prim_mul_enva_add_prim ()  //Aded by Gonetz41
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    SETSHADE_PRIM ();
    CC_ENVA ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CC_PRIM ();
    MOD_0 (TMOD_TEX_SUB_COL_MUL_FAC);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    MOD_0_FAC (rdp.env_color & 0xFF);
  }
  USE_T0 ();
}

static void cc_t0_sub_prim_mul_primlod_add_env ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  MOD_0 (TMOD_TEX_SUB_COL_MUL_FAC);
  MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
  MOD_0_FAC (lod_frac & 0xFF);
  USE_T0 ();
}

static void cc_t0_sub__prim_mul_env ()  //Aded by Gonetz
{
  if ( (rdp.prim_color & 0xFFFFFF00) == 0xFFFFFF00 && (rdp.env_color & 0xFFFFFF00) == 0xFFFFFF00)
  {
    CCMB (GR_COMBINE_FUNCTION_BLEND_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_NONE);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    SETSHADE_PRIM ();
    SETSHADE_ENV ();
  }
  USE_T0 ();
}

static void cc__t0_mul_t1__sub_prim_mul__t0t1a__add_prim ()  //Aded by Gonetz
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  T0_MUL_T1 ();
  A_T0_MUL_T1 ();
}

static void cc__t1_inter_t0_using_enva__sub_prim_mul_prima_add_prim ()  //Aded by Gonetz
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  CA_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T1_INTER_T0_USING_FACTOR (factor);
}

static void cc_t0_sub_prim_mul_shade_add_env ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.prim_color;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    CC_ENV ();
    MOD_0 (TMOD_TEX_SUB_COL);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    USE_T0 ();
  }
}

static void cc_t1_sub_prim_mul_shade_add_env ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 2;
    cmb.tex_ccolor = rdp.prim_color;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    CC_ENV ();
    MOD_1 (TMOD_TEX_SUB_COL);
    MOD_1_COL (rdp.prim_color & 0xFFFFFF00);
    USE_T1 ();
  }
}

static void cc_t1_sub_k4_mul_prima_add_t0 ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 1,
      GR_CMBX_ZERO, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 3;
    CC_BYTE (rdp.K4);
    cmb.tex_ccolor = cmb.ccolor;
    percent = (float)(rdp.prim_color&0xFF) / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    CCMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
    T0_ADD_T1 ();
  }
}

static void cc__t0_sub_prim_mul_shade_add_env__mul_shade ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.prim_color;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    CC_ENV ();
    MOD_0 (TMOD_TEX_SUB_COL);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    USE_T0 ();
  }
}

static void cc__t0_sub_prim_mul_shade_add_env__mul_shadea ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.prim_color;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    MULSHADE_SHADEA();
    CC_ENV ();
    MOD_0 (TMOD_TEX_SUB_COL);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    USE_T0 ();
  }
}

static void cc__t0_mul_shade__sub_env_mul_shadea_add_env ()  //Aded by Gonetz
{
  if (rdp.tiles[rdp.cur_tile].format == 4)
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
      GR_COMBINE_FACTOR_OTHER_ALPHA,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    CC_ENV ();
  }
  else if (rdp.tiles[rdp.cur_tile].format == 2)
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    USE_T0 ();
  }
  else
  {
    cc_t0 ();
  }
}

static void cc_t0_sub_env_mul_k5_add_prim ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  MOD_0 (TMOD_TEX_SUB_COL_MUL_FAC);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  MOD_0_FAC (rdp.K5);
  USE_T0 ();
}

static void cc_t0_sub_k4_mul_k5_add_t0 ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    wxUint32 temp = rdp.prim_lodfrac;
    rdp.prim_lodfrac = rdp.K4;
    SETSHADE_PRIMLOD ();
    rdp.prim_lodfrac = temp;
    CC_K5 ();
    USE_T0 ();
  }
  else
  {
    cc_t0 ();
  }
}

static void cc__t0_inter_t1_using_t0__sub_shade_mul_prima_add_shade ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    cmb.tex |= 3;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    CC_PRIMA();
    T0_INTER_T1_USING_T0 ();
  }
  else
  {
    // * not guaranteed to work if another iterated alpha is set
    CCMB (GR_COMBINE_FUNCTION_BLEND,
      GR_COMBINE_FACTOR_LOCAL_ALPHA,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    SETSHADE_A_PRIM ();
    T1_INTER_T0_USING_T0 ();  //strange, but this one looks better
  }
}

static void cc_t0_sub__prim_mul_shade__mul_enva_add__prim_mul_shade ()  //Aded by Gonetz
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_PRIM ();
  SETSHADE_A_ENV ();
  USE_T0 ();
}

static void cc_t0_sub_env_mul_t0_add_env ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  USE_T0 ();
  //(t0-env)*t0+env = t0*t0 + (1-t0)*env
}

static void cc_t0_sub_env_mul_prima_add_env ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    percent = (rdp.prim_color&0xFF) / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    MOD_0 (TMOD_COL_INTER_TEX_USING_COL1);
    MOD_0_COL (rdp.env_color & 0xFFFFFF00);
    wxUint32 prima = rdp.prim_color & 0xFF;
    MOD_0_COL1 ((prima<<24)|(prima|16)|(prima<<8));
    USE_T0 ();
  }
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
}

static void cc_t0_sub_env_mul_k5_add_env ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  cmb.ccolor = (rdp.env_color&0xFFFFFF00) | rdp.K5;
  USE_T0 ();
}

static void cc_t0_sub_env_mul_prim_add_shade ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  MOD_0 (TMOD_TEX_SUB_COL);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  USE_T0 ();
}

static void cc_t0_sub_env_mul_shade_add_prim ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.env_color;
  }
  else
  {
    MOD_0 (TMOD_TEX_SUB_COL);
    MOD_0_COL (rdp.env_color & 0xFFFFFF00);
    USE_T0 ();
  }
}

static void cc__t0_sub_t1_mul_enva_add_shade__sub_env_mul_prim ()
// (t0-t1)*env_a+shade, (cmb-env)*prim+0
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_ITRGB, 0);
    cmb.tex |= 3;
    CC_COLMULBYTE(rdp.prim_color, (rdp.env_color&0xFF));
    cmb.tex_ccolor = cmb.ccolor;
    CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_TEXTURE_RGB, 0);
    MULSHADE_PRIM ();
    CC_PRIMMULENV ();
  }
  else
  {
    cc_t0_sub_env_mul_prim_add_shade();
  }
}

static void cc__t0_inter_t1_using_primlod__sub_env_mul_shade_add_prim ()  //Aded by Gonetz
{
  if (!(rdp.env_color&0xFFFFFF00))
  {
    cc__t0_inter_t1_using_primlod__mul_shade_add_prim ();
    return;
  }
  if (!(rdp.prim_color&0xFFFFFF00))
  {
    if (!cmb.combine_ext)
    {
      cc_t0_sub_env_mul_shade ();
      return;
    }
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_ENV ();
    T0_INTER_T1_USING_FACTOR (lod_frac);
    return;
  }
  cc__t0_inter_t1_using_primlod__mul_shade_add_prim ();
}

static void cc__t0_sub_env_mul_shade_add_prim__mul_shade ()  //Aded by Gonetz
{
  if (!cmb.combine_ext)
  {
    cc_t0_sub_env_mul_shade_add_prim ();
    return;
  }
  T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_ZERO, 0);
  cmb.tex_ccolor = rdp.env_color;
  cmb.tex |= 1;
  CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_ZERO, 0);
  CC_PRIM ();
}

static void cc__t0_sub_env_mul_shade_add_prim__mul_shadea ()  //Aded by Gonetz
{
  if (!cmb.combine_ext)
  {
    cc_t0_sub_env_mul_shade_add_prim ();
    return;
  }
  T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_B, 0);
  cmb.tex_ccolor = rdp.prim_color;
  cmb.tex |= 1;
  MOD_0 (TMOD_TEX_SUB_COL);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);

  CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITALPHA, 0,
    GR_CMBX_ZERO, 0);
}

static void cc__t0_inter_t1_using_primlod__sub_env_mul_shade_add_env ()
{
  // (t1-t0)*primlod+t0, (cmb-env)*shade+env
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
  }
  CC_ENV ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}


static void cc_t0_sub_env_mul_enva_add_prim ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  MOD_0 (TMOD_TEX_SUB_COL_MUL_FAC);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  MOD_0_FAC (rdp.env_color & 0xFF);
  USE_T0 ();
}

static void cc_one_sub_t0_mul_prim_add_t0 ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  USE_T0 ();
  //(1-t)*prim+t == (1-prim)*t+prim
}

static void cc_one_sub_t1_mul_prim_add_t1 ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  USE_T1 ();
  //(1-t)*prim+t == (1-prim)*t+prim
}

static void cc_one_sub_t1_mul_env_add_t1 ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  USE_T1 ();
  //(1-t)*env+t == (1-env)*t+env
}

static void cc_one_sub_t0_mul_primlod_add_t0 ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIMLOD ();
  USE_T0 ();
  //(1-t)*primlod+t == (1-primlod)*t+primlod
}

static void cc_one_sub_t0_mul_prima_add_t0 ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  USE_T0 ();
  //(1-t)*prima+t == (1-prima)*t+prima
}

static void cc_one_sub__t0_inter_t1_using_enva__mul_prim_add__t0_inter_t1_using_enva ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
  //(1-t)*prim+t == (1-prim)*t+prim
}

static void cc_one_sub_t0_mul_shade_add_t0 ()
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC (0xFFFFFFFF);
  USE_T0 ();
}

static void cc_one_sub_prim_mul_t0_add_prim () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  USE_T0 ();
}

static void cc_one_sub_prim_mul_t0a_add_prim ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_TEXTURE_ALPHA, 0,
      GR_CMBX_B, 0);
    CC_PRIM ();
  } else {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
    MOD_0 (TMOD_COL_INTER_COL1_USING_TEXA);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    MOD_0_COL1 (0xFFFFFF00);
  }
  USE_T0 ();
}

static void cc_one_sub_prim_mul__t0_inter_t1_using_primlod__add_prim () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc__one_sub_prim_mul_shade__mul_t0_add__prim_mul_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_PRIM ();
  USE_T0 ();
}

static void cc_one_sub_shade_mul__t0_inter_t1_using_primlod__add_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc_one_sub_prim_mul_t1_add_prim () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  USE_T1 ();
}

static void cc_one_sub_prim_mul_env_add_prim ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  SETSHADE_ENV ();
}

static void cc_t0_sub_prim_mul_shade_add_shade ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ITRGB, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    if (rdp.prim_color & 0xFFFFFF00)
    {
      MOD_0 (TMOD_TEX_SUB_COL);
      MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    }
  }
  USE_T0 ();
}

static void cc__t0_mul_t0__sub_prim_mul_shade_add_shade ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ITRGB, 0);
    CC_PRIM ();
  }
  else
    cc_t0_sub_prim_mul_shade_add_shade();
}

static void cc__t0_mul_t1__sub_prim_mul_shade_add_shade ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ITRGB, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
  }
  T0_MUL_T1 ();
}

static void cc__t0_mul_t1__sub_env_mul_shade_add_shade ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ITRGB, 0);
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
  }
  T0_MUL_T1 ();
}

static void cc_one_sub_prim_mul_shade_add_shade ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_1SUBPRIM ();
}

static void cc_t0_inter_env_using_prima ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();

  MOD_0 (TMOD_TEX_INTER_COLOR_USING_FACTOR);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  MOD_0_FAC (rdp.prim_color & 0xFF);
}

static void cc_t0_inter_env_using_enva ()
{
  //(env-t0)*env_a+t0
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 1;
  }
  else
  {
    USE_T0 ();
    MOD_0 (TMOD_TEX_INTER_COLOR_USING_FACTOR);
    MOD_0_COL (rdp.env_color & 0xFFFFFFFF);
    MOD_0_FAC (rdp.env_color & 0xFF);
  }
}

static void cc_t0_inter_noise_using_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();

  MOD_0 (TMOD_TEX_INTER_NOISE_USING_COL);
  MOD_0_COL (rdp.prim_color);
  rdp.noise = RDP::noise_texture;
}

static void cc_t0_inter_noise_using_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();

  MOD_0 (TMOD_TEX_INTER_NOISE_USING_COL);
  MOD_0_COL (rdp.env_color);
  rdp.noise = RDP::noise_texture;
}

static void cc_t0_sub_env_mul_enva_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  CA_ENV ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_one_sub_prim_mul__t0_mul_t1__add__prim_mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_1SUBPRIM ();
  SETSHADE_PRIM ();
  SETSHADE_ENV ();
  T0_MUL_T1 ();
}

//Added by Gonetz
static void cc_one_sub_prim_mul__t0_mul_t1__add__prim_mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_1SUBPRIM ();
  MULSHADE_PRIM ();
  T0_MUL_T1 ();
}

//Added by Gonetz
static void cc_one_sub_prim_mul__t0_inter_t1_using_enva__add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_1SUBPRIM ();
  SETSHADE_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

static void cc_one_sub_env_mul__t0_inter_t1_using_primlod__add_env () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc_one_sub_env_mul__t1_sub_prim_mul_primlod_add_t0__add_env () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  T1_SUB_PRIM_MUL_PRIMLOD_ADD_T0 ();
}

static void cc_one_sub_env_mul_t0_add_prim_mul_env () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_1SUBENV ();
  SETSHADE_PRIM ();
  SETSHADE_ENV ();
  USE_T0 ();
}

static void cc_one_sub_env_mul_t0_add_env () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  USE_T0 ();
}

static void cc_one_sub_env_mul_t0_add_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_1SUBENV ();
  USE_T0 ();
}

static void cc_one_sub_env_mul_prim_add_env () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
}

static void cc_one_sub_env_mul_prim_add_shade () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_1SUBENV ();
  CC_C1MULC2 (rdp.prim_color, cmb.ccolor);
}

static void cc_one_sub_env_mul_shade_add_env () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
}

static void cc_one_sub_env_mul_prim_add__t0_inter_t1_using_env () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIM ();
  SETSHADE_1MENV ();
  T0_INTER_T1_USING_ENV ();
}

static void cc_one_sub_shade_mul_t0_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();
}

static void cc_one_sub_shade_mul__t0_mul_shadea__add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
  }
  else
  {
    USE_T0 ();
  }
}

static void cc_one_sub_shade_mul_env_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
}

static void cc_one_sub_shade_mul_shadea_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC (0xFFFFFFFF);
}

///*
static void cc_t0_sub_env_mul_prim_add_env ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  SETSHADE_1MPRIM();
  SETSHADE_ENV();
  CC_PRIM ();
  USE_T0 ();
  //(t0-env)*prim+env == t0*prim + env*(1-prim)
}
//*/
static void cc__t0_inter_t1_using_t1a__sub_env_mul_enva_add_env ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  cmb.ccolor = rdp.env_color;
  T0_INTER_T1_USING_T1A ();
}

static void cc_t0_sub_shade_mul_t0a_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();
  A_USE_T0 ();
}

static void cc_t0_sub_shade_mul_prima_add_shade ()  //Aded by Gonetz
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_A_PRIM ();
  USE_T0 ();
}

static void cc_t0_sub_shade_mul_shadea_add_shade ()  //Aded by Gonetz
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();
}

static void cc__t0_mul_t1_add_env__mul_shadea_add_shade ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 3;
  }
  else
  {
    T0_MUL_T1 ();
  }
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
}

static void cc_prim_sub_t0_mul_env_add_t0 ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_1SUBENV ();
  SETSHADE_PRIM ();
  SETSHADE_ENV ();
  USE_T0 ();
  //(prim-t0)*env+t0 == prim*env + t0*(1-env)
}

static void cc_prim_sub_t0_mul_t1_add_t0 ()  //Aded by Gonetz
{
  if (!cmb.combine_ext)
  {
    cc_t0_mul_t1 ();
    return;
  }
  T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_ZERO, 0,
    GR_CMBX_B, 0);
  T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
    GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_OTHER_TEXTURE_RGB, 0,
    GR_CMBX_B, 0);
  cmb.tex_ccolor = rdp.prim_color;
  cmb.tex |= 3;
  CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_ZERO, 0,
    GR_CMBX_B, 0);
}

static void cc_env_sub_t0_mul_prim_add_t0 ()  //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_1SUBPRIM ();
  SETSHADE_PRIM ();
  SETSHADE_ENV ();
  USE_T0 ();
  //(env-t0)*prim+t0 == prim*env + t0*(1-prim)
}

static void cc_env_sub_t0_mul_shade_add_t0 ()  //Aded by Gonetz
{
  if (!cmb.combine_ext)
  {
    cc_t0_mul_shade ();
    return;
  }
  CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
    GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_B, 0);
  CC_ENV ();
  USE_T0 ();
}

static void cc_prim_sub_env_mul_t0_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  SETSHADE_PRIMSUBENV ();
  USE_T0 ();
}

static void cc_prim_sub_env_mul_t0_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  USE_T0 ();
}

static void cc__prim_sub_env_mul_t0_add_env__add_primlod ()
{
  if (!cmb.combine_ext)
  {
    cc_prim_sub_env_mul_t0_add_env ();
    return;
  }
  T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_B, 0);
  cmb.tex_ccolor = rdp.env_color;
  cmb.tex |= 1;
  SETSHADE_PRIMSUBENV ();
  CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
    GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_ZERO, 1,
    GR_CMBX_TEXTURE_RGB, 0);
  CC_PRIMLOD ();
}

static void cc__prim_sub_env_mul_t0_add_env__add_shadea ()
{
  if (!cmb.combine_ext)
  {
    cc_prim_sub_env_mul_t0_add_env ();
    return;
  }
  T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_B, 0);
  cmb.tex_ccolor = rdp.env_color;
  cmb.tex |= 1;
  SETSHADE_PRIMSUBENV ();
  CCMBEXT(GR_CMBX_ITALPHA, GR_FUNC_MODE_X,
    GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_ZERO, 1,
    GR_CMBX_TEXTURE_RGB, 0);
}

static void cc_prim_sub_env_mul__t0_mul_t1a__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  T0_MUL_T1A ();
}

static void cc_prim_sub_env_mul__t0_mul_prim__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
  }
  else
  {
    USE_T0 ();
  }
}

static void cc_prim_sub_env_mul_t0_mul_shade_add_env ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIMSUBENV ();
    cmb.tex_ccolor = cmb.ccolor;
    cmb.tex |= 1;
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    CC_ENV ();
  }
  else
  {
    cc_t0_mul_prim_mul_shade ();
  }
}

static void cc_prim_sub_env_mul__t0_sub_t0_mul_prima__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  rdp.best_tex = 0;
  cmb.tex |= 1;
  cmb.tmu0_func = GR_COMBINE_FUNCTION_BLEND_LOCAL;
  cmb.tmu0_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR;
  percent = (float)(rdp.prim_color&0xFF) / 255.0f;
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
}

static void cc_prim_sub_env_mul__one_sub_t0_mul_primlod_add_prim__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
    cmb.dc0_detailmax = cmb.dc1_detailmax = (float)lod_frac / 255.0f;
  }
  else
  {
    USE_T0 ();
  }
}

static void cc_prim_sub_env_mul__t0_add_t1a__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  rdp.best_tex = 0;
  cmb.tex |= 3;
  cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL_ALPHA;
  cmb.tmu0_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL;
  cmb.tmu0_fac = GR_COMBINE_FACTOR_ONE;
}

static void cc_prim_sub_env_mul__t0_sub_prim_mul_enva_add_t0__add_env ()
{
  // (t0-prim)*env_a+t0, (prim-env)*cmb+env
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  USE_T0 ();

  MOD_0 (TMOD_TEX_SUB_COL_MUL_FAC_ADD_TEX);
  MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
  MOD_0_FAC (rdp.env_color & 0xFF);
}

static void cc_prim_sub_env_mul__t1_sub_prim_mul_enva_add_t0__add_env ()
{
  //(t1-prim)*env_a+t0, (prim-env)*cmb+env
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  if (cmb.combine_ext)
  {
    if (rdp.tiles[rdp.cur_tile].format > 2)
    {
      T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
        GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
        GR_CMBX_ZERO, 0,
        GR_CMBX_B, 0);
      T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
        GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
        GR_CMBX_DETAIL_FACTOR, 0,
        GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    }
    else
    {
      T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
        GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
        GR_CMBX_DETAIL_FACTOR, 0,
        GR_CMBX_ZERO, 0);
      T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
        GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
        GR_CMBX_ZERO, 1,
        GR_CMBX_ZERO, 0);
    }
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 3;
    cmb.dc0_detailmax = cmb.dc1_detailmax = (float)(rdp.env_color&0xFF) / 255.0f;
  }
  else
  {
    MOD_1 (TMOD_TEX_SUB_COL_MUL_FAC);
    MOD_1_COL (rdp.prim_color & 0xFFFFFF00);
    MOD_1_FAC (rdp.env_color & 0xFF);
    T0_ADD_T1 ();
  }
}

static void cc_prim_sub_env_mul__t1_sub_prim_mul_prima_add_t0__add_env ()
{
  // (t1-prim)*prim_a+t0, (prim-env)*cmb+env
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 3;
    cmb.dc0_detailmax = cmb.dc1_detailmax = (float)(rdp.prim_color&0xFF) / 255.0f;
  }
  else
  {
    MOD_1 (TMOD_TEX_SUB_COL_MUL_FAC);
    MOD_1_COL (rdp.prim_color & 0xFFFFFF00);
    MOD_1_FAC (rdp.prim_color & 0xFF);
    T0_ADD_T1 ();
  }
}

static void cc__prim_sub_env_mul_t0_add_env__mul_primlod ()
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  float factor = (float)rdp.prim_lodfrac / 255.0f;
  wxUint8 r = (wxUint8)((rdp.prim_color >> 24) & 0xFF);
  r = (wxUint8)((float)r * factor);
  wxUint8 g = (wxUint8)((rdp.prim_color >> 16) & 0xFF);
  g = (wxUint8)((float)g * factor);
  wxUint8 b = (wxUint8)((rdp.prim_color >>  8) & 0xFF);
  b = (wxUint8)((float)b * factor);
  CC ((r<<24) | (g<<16) | (b<<8));
  SETSHADE_ENV ();
  MULSHADE_PRIMLOD ();
  USE_T0 ();
}

static void cc__prim_sub_env_mul_t0_add_env__mul_k5 ()
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  float factor = (float)rdp.K5 / 255.0f;
  wxUint8 r = (wxUint8)((rdp.prim_color >> 24) & 0xFF);
  r = (wxUint8)((float)r * factor);
  wxUint8 g = (wxUint8)((rdp.prim_color >> 16) & 0xFF);
  g = (wxUint8)((float)g * factor);
  wxUint8 b = (wxUint8)((rdp.prim_color >>  8) & 0xFF);
  b = (wxUint8)((float)b * factor);
  CC ((r<<24) | (g<<16) | (b<<8));
  SETSHADE_ENV ();
  MULSHADE_K5 ();
  USE_T0 ();
}

static void cc_prim_sub_env_mul_t1_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  if (rdp.cycle_mode == 0 || ((settings.hacks&hack_KI) && (rdp.cycle2 & 0x0FFFFFFF) == 0x01FF1FFF))
  {
    USE_T0 ();
  }
  else
  {
    USE_T1 ();
  }
}

static void cc_prim_sub_env_mul_t1_add_env_mul_t0 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  MOD_1 (TMOD_COL_INTER_COL1_USING_TEX);
  MOD_1_COL (rdp.env_color & 0xFFFFFF00);
  MOD_1_COL1 (rdp.prim_color & 0xFFFFFF00);
  T0_MUL_T1 ();
}

static void cc_prim_sub_env_mul_t0a_add_t0 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIMSUBENV ();
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = cmb.ccolor;
    cmb.tex |= 1;
  }
  else
  {
    MOD_0 (TMOD_COL_MUL_TEXA_ADD_TEX);
    MOD_0_COL (cmb.ccolor & 0xFFFFFF00);
    USE_T0 ();
  }
}

//Added by Gonetz
static void cc_prim_sub_env_mul_t0a_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul_t1a_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  USE_T1 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul__t0_mul_t1__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  T0_MUL_T1 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul__t0_add_t1__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  T0_ADD_T1 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul__t0_mul_enva__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIMSUBENV ();
  SETSHADE_ENVA ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul__t0_mul_shade__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  MULSHADE_PRIMSUBENV ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul__prim_inter_t0_using_shadea__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  if (cmb.combine_ext)
  {
    SETSHADE_PRIM ();
    PRIM_INTER_T0_USING_SHADEA ();
  }
  else
  {
    SETSHADE_PRIMSUBENV ();
    MULSHADE_SHADEA ();
    USE_T0 ();
  }
}

//Added by Gonetz
static void cc_prim_sub_env_mul__t0_sub_prim_mul_primlod_add_t0__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
    percent = (float)(lod_frac) / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    USE_T0 ();
    MOD_0 (TMOD_TEX_SUB_COL_MUL_FAC_ADD_TEX);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    MOD_0_FAC (lod_frac & 0xFF);
  }
}

static void cc_prim_sub_env_mul__t0_sub_prim_mul_primlod_add_shade__add_env ()
{
  if (!cmb.combine_ext)
  {
    cc_prim_sub_env_mul_t0_add_env ();
    return;
  }
  T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_DETAIL_FACTOR, 0,
    GR_CMBX_ITRGB, 0);
  CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
    GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_TEXTURE_RGB, 0,
    GR_CMBX_B, 0);
  cmb.tex_ccolor = rdp.prim_color;
  CC_PRIM ();
  SETSHADE_ENV ();
  cmb.tex |= 1;
  percent = (float)(lod_frac) / 255.0f;
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
}

static void cc_prim_sub_env_mul__t0_sub_shade_mul_primlod_add_shade__add_env ()
{
  if (!cmb.combine_ext)
  {
    cc_prim_sub_env_mul_t0_add_env ();
    return;
  }
  T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_DETAIL_FACTOR, 0,
    GR_CMBX_B, 0);
  CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
    GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_TEXTURE_RGB, 0,
    GR_CMBX_B, 0);
  CC_PRIM ();
  SETSHADE_ENV ();
  cmb.tex |= 1;
  percent = (float)(lod_frac) / 255.0f;
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
}

//Added by Gonetz
static void cc_lavatex_sub_prim_mul_shade_add_lavatex ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    CC_PRIM ();
    T0_SUB_PRIM_MUL_PRIMLOD_ADD_T1 ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
    T0_ADD_T1 ();
  }
}

//Added by Gonetz
static void cc_prim_sub_env_mul__t0_sub_prim_mul_primlod_add_t1__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  T0_SUB_PRIM_MUL_PRIMLOD_ADD_T1 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul__t1_sub_prim_mul_primlod_add_t0__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  T1_SUB_PRIM_MUL_PRIMLOD_ADD_T0 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul__t0_inter_t1_using_t1__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  T0_INTER_T1_USING_T1 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul__t0_inter_t1_using_enva_alpha__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  SETSHADE_ENV ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
  A_T0_INTER_T1_USING_FACTOR (factor);
}

//Added by Gonetz
static void cc__env_inter_prim_using_t0__sub_shade_mul_t0a_add_shade ()
{
  if (!cmb.combine_ext)
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_ALPHA,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MOD_0 (TMOD_COL_INTER_COL1_USING_TEX);
    MOD_0_COL (rdp.env_color & 0xFFFFFF00);
    MOD_0_COL1 (rdp.prim_color & 0xFFFFFF00);
    USE_T0 ();
    A_USE_T0 ();
  }
  else
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TEXTURE_ALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 1;
    wxUint32 pse = (rdp.prim_color>>24) - (rdp.env_color>>24);
    percent = (float)(pse) / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
}

//Added by Gonetz
static void cc_prim_sub_env_mul_shade_add_t0 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_PRIMSUBENV ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul_prima_add_t0 ()
{
  if (rdp.prim_color != 0x000000ff)
  {
    if (cmb.combine_ext)
    {
      CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
        GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
        GR_CMBX_CONSTANT_ALPHA, 0,
        GR_CMBX_TEXTURE_RGB, 0);
      CC_PRIM ();
      SETSHADE_ENV ();
    }
    else
    {
      CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
        GR_COMBINE_FACTOR_ONE,
        GR_COMBINE_LOCAL_ITERATED,
        GR_COMBINE_OTHER_TEXTURE);
      SETSHADE_PRIMSUBENV ();
      SETSHADE_PRIMA ();
    }
  }
  else if ((rdp.prim_color&0xFFFFFF00) - (rdp.env_color&0xFFFFFF00) == 0)
  {
    cc_t0 ();
    return;
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CC_ENV ();
  }
  USE_T0 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul_shade_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  MULSHADE_PRIMSUBENV ();
}

static void cc_prim_sub_env_mul_shadea_add_env ()
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_OTHER_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul__t0_inter_t1_using_prima__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

//Added by Gonetz
static void cc_prim_sub_env_mul__t1_inter_t0_using_prima__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  T1_INTER_T0_USING_FACTOR (factor);
}

static void cc_prim_sub_env_mul__t0_inter_t1_using_enva__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

static void cc_prim_sub_center_mul__t0_inter_t1_using_enva__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_C1SUBC2 (rdp.prim_color, rdp.CENTER);
  SETSHADE_ENV ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

static void cc_prim_sub_env_mul__t1_inter_t0_using_enva__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T1_INTER_T0_USING_FACTOR (factor);
}

static void cc_prim_sub_env_mul__t0_mul_enva_add_t1__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
    percent = (float)(rdp.env_color&0xFF) / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    T0_ADD_T1 ();
  }
}

static void cc_prim_sub_env_mul__t1_mul_enva_add_t0__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  T1_MUL_ENVA_ADD_T0 ();
}

//Added by Gonetz
static void cc_prim_sub_env_mul_primlod_add__t0_inter_t1_using_primlod ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIMSUBENV();
  CC_COLMULBYTE(cmb.ccolor, rdp.prim_lodfrac);
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc_prim_sub_env_mul__t0_inter_t1_using_primlod__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc_prim_sub_env_mul__t1_inter_t0_using_primlod__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  T1_INTER_T0_USING_FACTOR (lod_frac);
}

static void cc_prim_sub_env_mul__t1_mul_primlod_add_t0__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  T1_MUL_PRIMLOD_ADD_T0 ();
}

static void cc_prim_sub_env_mul__t1_sub_prim_mul_t0_add_t0__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_PRIM ();
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 3;
  }
  else
  {
    MOD_1 (TMOD_TEX_SUB_COL);
    MOD_1_COL (rdp.prim_color & 0xFFFFFF00);
    T0_MUL_T1_ADD_T0 ();
  }
}

//Added by Gonetz
static void cc__prim_sub_env_mul_prim_add_t0__mul_prim ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    SETSHADE_PRIMSUBENV ();
    SETSHADE_PRIM ();
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIM() ;
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    SETSHADE_PRIMSUBENV ();
    SETSHADE_PRIM ();
    USE_T0 ();
  }
}

//Added by Gonetz
static void cc_prim_sub_env_mul_prim_add_env ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_B, 0);
    SETSHADE_ENV();
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    SETSHADE_PRIMSUBENV ();
    SETSHADE_PRIM ();
    CC_ENV ();
  }
}

static void cc_prim_sub_env_mul_primlod_add_env ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    CC_PRIMLOD ();
    cmb.tex_ccolor = cmb.ccolor;
    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    SETSHADE_PRIM();
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    SETSHADE_PRIMSUBENV ();
    SETSHADE_PRIMLOD ();
    CC_ENV ();
  }
}

//Added by Gonetz
static void cc_prim_sub_env_mul_enva_add_t0 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIMSUBENV ();
  SETSHADE_ENVA ();
  USE_T0 ();
}

static void cc_prim_sub_env_mul_enva_add_env ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    CC_ENVA ();
    cmb.tex_ccolor = cmb.ccolor;
    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    SETSHADE_PRIM();
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    SETSHADE_PRIMSUBENV ();
    SETSHADE_ENVA ();
    CC_ENV ();
  }
}

//Added by Gonetz
static void cc_prim_sub_shade_mul_t0_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  COLSUBSHADE_PRIM ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_prim_sub_shade_mul__t1_sub_prim_mul_primlod_add_t0__add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  T1_SUB_PRIM_MUL_PRIMLOD_ADD_T0 ();
}

static void cc_prim_sub_shade_mul_t1a_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  USE_T1 ();
}

//Added by Gonetz
static void cc_prim_sub_shade_mul_t0_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_prim_sub_shade_mul_t1_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  USE_T1 ();
}

//Added by Gonetz
static void cc_prim_sub_shade_mul__t0a_mul_t1__add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  T0A_MUL_T1();
}

//Added by Gonetz
static void cc_prim_sub_shade_mul__t0_inter_t1_using_enva__add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

//Added by Gonetz
static void cc_prim_sub_shade_mul__t0_inter_t1_using_shadea__add_shade ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
  }
  CC_PRIM ();
  T0_INTER_T1_USING_SHADEA ();
}

//Added by Gonetz
static void cc_prim_sub_shade_mul_prima_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_OTHER_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  CA_PRIM ();
}

//Added by Gonetz
static void cc_prim_sub_shade_mul_env_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIMMULENV ();
  MULSHADE_1MENV ();
}

//Added by Gonetz
static void cc_prim_sub_shade_mul_shadea_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
}

static void cc_env_sub_prim_mul_t0_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  USE_T0 ();
}

static void cc_env_sub_prim_mul_t1_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  USE_T1 ();
}

static void cc_env_sub_prim_mul_t0a_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  A_USE_T0 ();
}

static void cc_env_sub_prim_mul_t1a_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  A_USE_T1 ();
}

static void cc_env_sub_prim_mul__t0_add_t1__add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  T0_ADD_T1 ();
}

static void cc_env_sub_prim_mul__t0_mul_t1__add_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  T0_MUL_T1 ();
}

static void cc_env_sub_prim_mul__t0t1a__add_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  A_T0_MUL_T1 ();
}

static void cc_env_sub_prim_mul__t0_inter_t1_using_t1__add_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  T0_INTER_T1_USING_T1 ();
}

static void cc_env_sub_prim_mul__t0_inter_t1_using_half__add_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  T0_INTER_T1_USING_FACTOR (0x7F);
}

static void cc_env_sub_prim_mul__t1_inter_t0_using_t0__add_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  T1_INTER_T0_USING_T0 ();
}

static void cc_env_sub_shade_mul__t0_mul_t1__add_shade () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  T0_MUL_T1 ();
}

static void cc_env_sub_prim_mul__t0a_mul_t1a__add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  SETSHADE_ENV ();
  A_T0_MUL_T1 ();
}


static void cc_env_sub_prim_mul_prima_add_prim ()  //Aded by Gonetz
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  CA_PRIM ();
  SETSHADE_ENV ();
}

static void cc_env_sub_prim_mul_enva_add_prim ()  //Aded by Gonetz
{
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_OTHER_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  CA_ENV ();
  SETSHADE_PRIM ();
}

static void cc__t0_sub_env_mul_shade__sub_prim_mul_shade_add_prim ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    CC_PRIM ();
  }
  else
  {
    cc_t0_mul_shade ();
  }
}

static void cc_env_sub_prim_mul_shade_add_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  MULSHADE_ENVSUBPRIM ();
}

static void cc_env_sub_prim_mul_shadea_add_prim () //Added by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  SETSHADE_ENVSUBPRIM ();
  MULSHADE_SHADEA ();
}

static void cc_env_sub_prim_mul__t0_inter_t1_using_prima__add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

static void cc_env_sub_prim_mul__t0_inter_t1_using_primlod__add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_PRIM ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc_env_sub_primshade_mul_t0_add_primshade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  MULSHADE_PRIM ();
  USE_T0 ();
}

static void cc_env_sub_primshade_mul_t1_add_primshade ()
{
  //  cc_prim_mul_shade();
  //  return;
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  MULSHADE_PRIM ();
  USE_T0 ();
}

static void cc_env_sub_shade_mul_t0_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  USE_T0 ();
}

static void cc__env_sub_shade_mul_t0_add_shade__mul_prim ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = rdp.prim_color;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIM() ;
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
    CC_ENV ();
    MULSHADE_PRIM ();
    USE_T0 ();
  }
}

static void cc_env_sub_shade_mul_t1_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  USE_T1 ();
}

//Added by Gonetz
static void cc_env_sub_shade_mul__t0_inter_t1_using_shadea__add_shade ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
  }
  CC_ENV ();
  T0_INTER_T1_USING_SHADEA ();
}

//Added by Gonetz
static void cc_env_sub_shade_mul_enva_add_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_OTHER_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  cmb.ccolor = rdp.env_color;
}

//Added by Gonetz
static void cc_shade_sub_t0_mul_shadea_add_t0 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_SHADEA ();
  USE_T0 ();
}


static void cc__t0_mul_shade_mul_shadea__add__t1_mul_one_sub_shadea ()
{
  // (t0-0)*shade+0, (cmb-t0)*shadea+t0
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITALPHA, 1,
      GR_CMBX_ZERO, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    MULSHADE_SHADEA ();
    cmb.tex |= 3;
    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_TEXTURE_RGB, 0);
  }
  else
  {
    cc_t0_mul_shade ();
  }
}

static void cc_shade_sub_prim_mul__t0_inter_t1_using_primlod__add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc_shade_sub_prim_mul_t0_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_shade_sub_prim_mul_t1_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  USE_T1 ();
}

//Added by Gonetz
static void cc_shade_sub_env_mul__t0_mul_t1__add__t0_mul_t1 ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TEXTURE_RGB, 0,
      GR_CMBX_TEXTURE_RGB, 0);
    CC_ENV ();
    T0_MUL_T1 ();
  }
  else
  {
    cc_t0_mul_t1 ();
  }
}

//Added by Gonetz
static void cc_shade_sub_env_mul_t0_add_prim ()
{
  if (rdp.cur_image && (rdp.cur_image->format != 0))
  {
    cc_prim ();
    return;
  }

  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  SUBSHADE_ENV ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_shade_sub_env_mul__t0_inter_t1_using_primlod__add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIM ();
  SUBSHADE_ENV ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

//Added by Gonetz
static void cc_shade_sub_env_mul__t0_inter_t1_using_primlod__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

//Added by Gonetz
static void cc_shade_sub_env_mul__t0_mul_t1__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  T0_MUL_T1 ();
}

//Added by Gonetz
static void cc_shade_sub_env_mul__t1_sub_prim_mul_primlod_add_t0__add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  SETSHADE_ENV ();
  T1_SUB_PRIM_MUL_PRIMLOD_ADD_T0 ();
}

//Added by Gonetz
static void cc_shade_sub_env_mul_t0_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  USE_T0 ();
}

//Added by Gonetz
static void cc_shade_sub_env_mul_t0_mul_prim_add_prim_mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_PRIMMULENV ();
  SUBSHADE_ENV ();
  MULSHADE_PRIM()
    USE_T0 ();
}

//Added by Gonetz
static void cc_shade_sub_env_mul_t1_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENV ();
  USE_T1 ();
}

//Added by Gonetz
static void cc_shade_sub_env_mul_prim_add_t0 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SUBSHADE_ENV ();
  MULSHADE_PRIM ();
  USE_T0 ();
}

static void cc__t0_add_prim_mul_shade__mul_shade_add_env ()
{
  if (!cmb.combine_ext)
  {
    cc_shade_sub_env_mul_prim_add_t0 ();
    return;
  }
  T1CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
    GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_B, 0);
  T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
    GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_B, 0);
  CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_B, 0);
  cmb.tex_ccolor = rdp.prim_color;
  CC_ENV ();
  cmb.tex |= 1;
}

static void cc__t0_add_prim_mul_shade__mul_shade ()
{
  if (!cmb.combine_ext)
  {
    cc_shade_sub_env_mul_prim_add_t0 ();
    return;
  }
  T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
    GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_B, 0);
  CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_ZERO, 0);
  cmb.tex_ccolor = rdp.prim_color;
  cmb.tex |= 1;
}

//Added by Gonetz
static void cc_shade_sub_env_mul_prim_add_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SUBSHADE_ENV ();
  MULSHADE_PRIM ();
  CC_ENV ();
}

//Added by Gonetz
static void cc_shade_sub_env_mul_prima_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SUBSHADE_ENV ();
  MULSHADE_PRIMA ();
  CC_PRIM ();
}

static void cc_shade_sub_env_mul_k5_add_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SUBSHADE_ENV ();
  wxUint32 temp = rdp.prim_color;
  rdp.prim_color = rdp.K5;
  MULSHADE_PRIMA ();
  rdp.prim_color = temp;
  CC_PRIM ();
}

// ** A inter B using C **
static void cc_t0_inter_t1_using_t1a ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_T1A ();
}

static void cc_t0_inter_t1_using_prima ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

static void cc_t1_inter_t0_using_prima ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  T1_INTER_T0_USING_FACTOR (factor);
}

static void cc_t1_inter_t0_using_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  T1_INTER_T0_USING_PRIM ();
}

static void cc_t0_inter_t1_using_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_ENV ();
}

static void cc_t0_inter_t1_using_enva ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

static void cc__t0_inter_t1_using_prim__inter_env_using_enva ()
{
  // (t1-t0)*prim+t0, (env-cmb)*env_a+cmb
  if (!cmb.combine_ext)
  {
    cc_t0_inter_t1_using_prima ();
    return;
  }
  T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_ZERO, 0,
    GR_CMBX_B, 0);
  T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_TMU_CCOLOR, 0,
    GR_CMBX_B, 0);
  cmb.tex_ccolor = rdp.prim_color;
  cmb.tex |= 3;
  CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
    GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_CONSTANT_ALPHA, 0,
    GR_CMBX_B, 0);
  cmb.ccolor = rdp.env_color;
}

static void cc__t0_inter_t1_using_shade__inter_env_using_enva ()
{
  // (t1-t0)*shade+t0, (env-cmb)*env_a+cmb
  if (!cmb.combine_ext)
  {
    cc_t0_inter_t1_using_enva ();
    return;
  }
  T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
    GR_CMBX_ZERO, 0,
    GR_CMBX_B, 0);
  T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_B, 0);
  cmb.tex |= 3;
  CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
    GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_CONSTANT_ALPHA, 0,
    GR_CMBX_B, 0);
  cmb.ccolor = rdp.env_color;
}

//Added by Gonetz
static void cc_t0_inter_t1_using_shade ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
    T0_INTER_T1_USING_FACTOR (0x7F);
  }
}

//Added by Gonetz
static void cc_t1_inter_t0_using_shade ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
    T0_INTER_T1_USING_FACTOR (0x7F);
  }
}

//Added by Gonetz
static void cc_t1_inter_t0_using_shadea ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
  }
  T1_INTER_T0_USING_SHADEA ();
}

//Added by Gonetz
static void cc_t0_inter_t1_using_primlod ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

//Added by Gonetz
static void cc_t1_inter_t0_using_primlod ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  T1_INTER_T0_USING_FACTOR (lod_frac);
}

//Added by Gonetz
static void cc_t1_inter_t0_using_t0 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  T1_INTER_T0_USING_T0 ();
}

//Added by Gonetz
static void cc_t0_inter_t1_using_k5 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_FACTOR (rdp.K5);
}

static void cc_t0_inter_env_using_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();

  MOD_0 (TMOD_TEX_INTER_COL_USING_COL1);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  MOD_0_COL1 (rdp.prim_color & 0xFFFFFF00);
}

//Added by Gonetz
static void cc_t0_inter_prim_using_primlod ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  USE_T0 ();

  MOD_0 (TMOD_TEX_INTER_COLOR_USING_FACTOR);
  MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
  MOD_0_FAC (lod_frac & 0xFF);
}

static void cc_t0_inter_shade_using_t0a ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TEXTURE_ALPHA, 0,
      GR_CMBX_B, 0);
    USE_T0();
    A_USE_T0();
  }
  else
  {
    //(shade-t0)*t0a+t0 = t0*(1-t0a)+shade*t0a
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    rdp.best_tex = 1;
    cmb.tex = 1;
    cmb.tmu0_func = GR_COMBINE_FUNCTION_BLEND_LOCAL;
    cmb.tmu0_fac = GR_COMBINE_FACTOR_LOCAL_ALPHA;
  }
}

static void cc_t0_inter_shade_using_primlod ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIMLOD();
  cmb.ccolor=(~cmb.ccolor)&0xFFFFFF00;
  MULSHADE_PRIMLOD ();
  USE_T0 ();
  //(shade-t0)*primlod+t0 = t0*(1-primlod)+shade*primlod
}

//Added by Gonetz
static void cc__env_inter_t0_using_primlod__mul_prim ()
{
  //((t0-env)*primlod+env)*prim = t0*prim*primlod+env*prim*(1-primlod);
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_PRIM ();
  cmb.ccolor = ((((cmb.ccolor & 0xFF000000) >> 24) * (lod_frac & 0xFF))<<24) | ((((cmb.ccolor & 0x00FF0000) >> 16) * (lod_frac & 0xFF))<<16) | ((((cmb.ccolor & 0x0000FF00) >> 8) * (lod_frac & 0xFF))<<8);
  SETSHADE_PRIM ();
  SETSHADE_ENV ();
  SETSHADE_1MPRIMLOD ();
  USE_T0 ();
}

//Added by Gonetz
static void cc__env_inter_t0_using_shadea__mul_shade ()
{
  //((t0-env)*shadea+env)*shade
  if (!cmb.combine_ext)
  {
    cc_t0_mul_shade ();
    return;
  }
  T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_ITALPHA, 0,
    GR_CMBX_B, 0);
  cmb.tex_ccolor = rdp.env_color;
  cmb.tex |= 1;
  CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
    GR_CMBX_ITRGB, 0,
    GR_CMBX_ZERO, 0);
}

//Added by Gonetz
static void cc_env_inter_prim_using_primlod ()
{
  if (rdp.prim_color&0xFFFFFF00)
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    SETSHADE_PRIMSUBENV ();
    SETSHADE_PRIMLOD ();
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
    SETSHADE_ENV ();
    SETSHADE_PRIMLOD ();
    CC_ENV ();
  }
}

static void cc_prim_inter__t0_mul_t1_add_env__using_shadea ()
{
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 3;
  }
  else
  {
    T0_MUL_T1 ();
  }
  // * not guaranteed to work if another iterated alpha is set
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_PRIM ();
}

static void cc_env_inter__prim_inter_shade_using_t0__using_shadea ()
{
  if (!cmb.combine_ext)
  {
    cc_shade_sub_prim_mul_t0_add_prim ();
    return;
  }
  T0CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_X,
    GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_LOCAL_TEXTURE_RGB, 0,
    GR_CMBX_B, 0);
  cmb.tex_ccolor = rdp.prim_color;
  cmb.tex |= 1;
  CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_ITALPHA, 0,
    GR_CMBX_B, 0);
  CC_ENV ();
}

static void cc_shade_inter__prim_inter_shade_using_t0__using_shadea ()
{
  if (!cmb.combine_ext)
  {
    cc_shade_sub_prim_mul_t0_add_prim ();
    return;
  }
  T0CCMBEXT(GR_CMBX_ITRGB, GR_FUNC_MODE_X,
    GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_LOCAL_TEXTURE_RGB, 0,
    GR_CMBX_B, 0);
  cmb.tex_ccolor = rdp.prim_color;
  cmb.tex |= 1;
  CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
    GR_CMBX_ITRGB, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_ITALPHA, 0,
    GR_CMBX_B, 0);
}

// ** (A-B)*C+D*E **
static void cc_one_sub_env_mul_prim_add__t0_mul_env () //Aded by Gonetz
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  SETSHADE_1MENV ();
  SETSHADE_PRIM ();
  USE_T0 ();
}

// ** ((A-B)*C+D)*E **
static void cc_t0_sub_env_mul_prim_mul_shade_add_prim_mul_shade () //Aded by Gonetz
{
  //(t0-env)*shade+shade, (cmb-0)*prim+0
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rdp.env_color;
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    if (rdp.env_color & 0xFFFFFF00)
    {
      MOD_0 (TMOD_TEX_SUB_COL);
      MOD_0_COL (rdp.env_color & 0xFFFFFF00);
    }
    MULSHADE_PRIM ();
    USE_T0 ();
  }
}

static void cc__t1_sub_prim_mul_t0_add_env__mul_shade () //Aded by Gonetz
{
  // (t1-prim)*t0+env, (cmb-0)*shade+0
  if (cmb.combine_ext)
  {
    T1CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 3;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    if (rdp.prim_color & 0xFFFFFF00)
    {
      MOD_1 (TMOD_TEX_SUB_COL);
      MOD_1_COL (rdp.prim_color & 0xFFFFFF00);
    }
    T0_MUL_T1 ();
  }
}

// ** (A inter B using C) * D **
//Added by Gonetz
static void cc__t0_inter_t1_using_prima__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
  CC_PRIM ();
}

//Added by Gonetz
static void cc__t1_inter_t0_using_prima__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  T1_INTER_T0_USING_FACTOR (factor);
  CC_PRIM ();
}

//Added by Gonetz
static void cc__t0_inter_t1_using_prim__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_PRIM ();
}

//Added by Gonetz
static void cc__t0_inter_t1_using_prima__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

//Added by Gonetz
static void cc__t1_inter_t0_using_prima__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  T1_INTER_T0_USING_FACTOR (factor);
}

static void cc__t0_inter_t1_using_env__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_ENV ();
}

static void cc__t0_inter_t1_using_enva__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

static void cc__t0_inter_t1_using_enva__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

//Added by Gonetz
static void cc__t0_inter_t1_using_enva__mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  T0_INTER_T1_USING_FACTOR (factor);
}

//Added by Gonetz
static void cc__t0_inter_t1_using_primlod__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

//Added by Gonetz
static void cc__t0_inter_t1_using_primlod__mul_prima ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIMA ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

//Added by Gonetz
static void cc__t1_mul_primlod_add_t0__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  T1_MUL_PRIMLOD_ADD_T0 ();
}

//Added by Gonetz
static void cc__t0_inter_t1_using_primlod__mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

//Added by Gonetz
static void cc__t1_mul_primlod_add_t0__mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  T1_MUL_PRIMLOD_ADD_T0 ();
}

//Added by Gonetz
static void cc__t1_inter_t0_using_prim__mul_env ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
  T1_INTER_T0_USING_PRIM ();
}

static void cc__one_sub_shade_mul_t0_add_shade__mul_prim ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
	  GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
	  GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_ZERO, 0);
	CC_PRIM ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    USE_T0 ();
  }
}

static void cc__one_sub_shade_mul_t0_add_shade__mul_env ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
	  GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
	  GR_CMBX_CONSTANT_COLOR, 0,
      GR_CMBX_ZERO, 0);
	CC_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    USE_T0 ();
  }
}

static void cc__t1_inter_t0_using_prim__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T1_INTER_T0_USING_PRIM ();
}

static void cc__t0_inter_t1_using_primlod__mul_shade ()
{
  //*
  if (rdp.LOD_en && (rdp.mipmap_level == 0) && !(settings.hacks&hack_Fifa98))
  {
    cc_t0_mul_shade ();
    return;
  }
  //*/
  if (settings.ucode == 7)
    lod_frac = rdp.prim_lodfrac;
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void cc__t1_inter_t0_using_primlod__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T1_INTER_T0_USING_FACTOR (lod_frac);
}

static void cc__t0_inter_t1_using_half__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_FACTOR (0x7F);
}

static void cc__t0_inter_t1_using_t0__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_T0();
}

static void cc__t0_inter_t1_using_t1a__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_T1A();
}

static void cc__t0_inter_t1_using_shadea__mul_shade ()
{
  if (cmb.combine_ext)
  {
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
  }
  T0_INTER_T1_USING_SHADEA ();
}

static void cc__t0_inter_t1_using_k5__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T0_INTER_T1_USING_FACTOR (rdp.K5);
}

static void cc__t1_inter_t0_using_k5__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  T1_INTER_T0_USING_FACTOR (rdp.K5);
}

static void cc_t0_inter_prim_using_prima ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
    CC_1SUBPRIMA ();
    SETSHADE_PRIM ();
    SETSHADE_PRIMA ();
    USE_T0 ();
  }
}

static void cc__t0_inter_prim_using_t0a__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MOD_0 (TMOD_TEX_INTER_COL_USING_TEXA);
  MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
  USE_T0 ();
}

static void cc__env_inter_prim_using_t0__mul_prim ()
{
  // (prim-env)*t0+env, (cmb-0)*prim+0
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  MOD_0 (TMOD_COL_INTER_COL1_USING_TEX);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  MOD_0_COL1 (rdp.prim_color & 0xFFFFFF00);
  USE_T0 ();
}

static void cc__env_inter_prim_using_t0__mul_shade ()
{
  // amazing... mace actually uses the blender as part of the combine
  if ((rdp.othermode_l & 0xFFFF0000) == 0x03820000 ||
    (rdp.othermode_l & 0xFFFF0000) == 0x00910000)
  {
    // blender:
    //  1ST = CLR_IN * A_IN + CLR_BL * 1MA
    //  OUT = 1ST * 0 + 1ST * 1

    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MOD_0 (TMOD_COL2_INTER__COL_INTER_COL1_USING_TEX__USING_TEXA);
    MOD_0_COL (rdp.env_color & 0xFFFFFF00);
    MOD_0_COL1 (rdp.prim_color & 0xFFFFFF00);
    MOD_0_COL2 (rdp.blend_color & 0xFFFFFF00);
    USE_T0 ();
    return;
  }
  //(prim-env)*t0+env, (shade-0)*cmb+0
  MOD_0 (TMOD_COL_INTER_COL1_USING_TEX);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  MOD_0_COL1 (rdp.prim_color & 0xFFFFFF00);
  USE_T0 ();
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
}

static void cc__env_inter_one_using_t0__mul_shade ()
{
  //(one-env)*t0+env, (cmb-0)*shade+0
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.env_color&0xFFFFFF00;
    cmb.tex |= 1;
  }
  else
  {
    MOD_0 (TMOD_COL_INTER_COL1_USING_TEX);
    MOD_0_COL (rdp.env_color & 0xFFFFFF00);
    MOD_0_COL1 (0xFFFFFF00);
    USE_T0 ();
  }
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
}

static void cc_env_inter_one_using__one_sub_t0_mul_primlod ()
{
  if (cmb.combine_ext)
  {
    // (noise-t0)*primlod+0, (1-env)*cmb+env
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rand()&0xFFFFFF00;
    cmb.tex |= 1;
    percent = (float)(lod_frac) / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    cmb.tex |= 1;
  }
  else
  {
    USE_T0 ();
  }
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_ENV ();
}

static void cc__env_inter_prim_using_prima__mul_shade ()
{
  int primr = (rdp.prim_color>>24)&0xFF;
  int primg = (rdp.prim_color>>16)&0xFF;
  int primb = (rdp.prim_color>>8)&0xFF;
  int prima = rdp.prim_color&0xFF;
  int envr = (rdp.env_color>>24)&0xFF;
  int envg = (rdp.env_color>>16)&0xFF;
  int envb = (rdp.env_color>>8)&0xFF;
  int r = (((primr-envr)*prima)/256)+envr;
  int g = (((primg-envg)*prima)/256)+envg;
  int b = (((primb-envb)*prima)/256)+envb;
  cmb.ccolor = (r<<24) | (g<<16) | (b<<8);
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
}

static void cc__prim_inter_t0_using_env__mul_shade ()
{
  // (t0-prim)*env+prim, (cmb-0)*shade+0
  if ((rdp.prim_color & 0xFFFFFF00) == 0)
  {
    cc_t0_mul_env_mul_shade ();
  }
  else if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CCOLOR, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rdp.env_color & 0xFFFFFF00;
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    wxUint32 onesubenv = ~rdp.env_color;
    CC_C1MULC2(rdp.prim_color, onesubenv);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MOD_0 (TMOD_COL_INTER_TEX_USING_COL1);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    MOD_0_COL1 (rdp.env_color & 0xFFFFFF00);
    USE_T0 ();
  }
}

static void cc__one_inter_prim_using_t1__mul_shade ()
{
  if (cmb.combine_ext)
  {
    if ((settings.hacks&hack_BAR) && rdp.cur_tile == 1)
    {
      T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
        GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
        GR_CMBX_LOCAL_TEXTURE_RGB, 0,
        GR_CMBX_ZERO, 1);
      cmb.tex |= 1;
    }
    else
    {
      T1CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
        GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
        GR_CMBX_LOCAL_TEXTURE_RGB, 0,
        GR_CMBX_ZERO, 1);
      T0CCMBEXT(GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
        GR_CMBX_OTHER_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
        GR_CMBX_ZERO, 0,
        GR_CMBX_B, 0);
      cmb.tex |= 2;
    }
    cmb.tex_ccolor = rdp.prim_color | 0xFF;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    if ((settings.hacks&hack_BAR) && rdp.cur_tile == 1)
    {
      MOD_0 (TMOD_COL_INTER_COL1_USING_TEX);
      MOD_0_COL (0xFFFFFF00);
      MOD_0_COL1 (rdp.prim_color & 0xFFFFFF00);
      USE_T0 ();
    }
    else
    {
      MOD_1 (TMOD_COL_INTER_COL1_USING_TEX);
      MOD_1_COL (0xFFFFFF00);
      MOD_1_COL1 (rdp.prim_color & 0xFFFFFF00);
      USE_T1 ();
    }
  }
}

static void cc_prim_sub__prim_sub_t0_mul_prima__mul_shade ()
{
  // (prim-t0)*prim_a+0, (prim-cmb)*shade+0
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = rdp.prim_color;
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_CONSTANT_COLOR, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    CC_PRIM();
  }
  else
  {
    if ((rdp.prim_color & 0xFFFFFF00) == 0)
    {
      cc_t0_mul_prima_mul_shade ();
      return;
    }
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MOD_0 (TMOD_COL_INTER_TEX_USING_COL1);
    MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
    wxUint8 prima = (wxUint8)(rdp.prim_color&0xFF);
    MOD_0_COL1 ((prima<<24)|(prima<<16)|(prima<<8));
    USE_T0 ();
  }
}

static void cc__prim_inter_env_using_t0__mul_shade ()
{
  // (env-prim)*t0+prim, (cmb-0)*shade+0
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MOD_0 (TMOD_COL_INTER_COL1_USING_TEX);
  MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
  MOD_0_COL1 (rdp.env_color & 0xFFFFFF00);
  USE_T0 ();
}

static void cc__prim_inter_one_using_env__mul_shade ()
{
  // (one-prim)*env+prim, (cmb-0)*shade+0
  if ((rdp.prim_color&0xFFFFFF00) == 0)
  {
    cc_env_mul_shade ();
    return;
  }
  if ((rdp.env_color&0xFFFFFF00) == 0)
  {
    cc_prim_mul_shade ();
    return;
  }
  if ((rdp.prim_color&0xFFFFFF00) == 0xFFFFFF00 || (rdp.env_color&0xFFFFFF00) == 0xFFFFFF00)
  {
    cc_shade ();
    return;
  }
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_1SUBPRIM ();
  CC_C1MULC2 (cmb.ccolor, rdp.env_color);
  cmb.ccolor=(wxUint8)( min(255, (int)((cmb.ccolor & 0xFF000000) >> 24) + (int)((rdp.prim_color & 0xFF000000) >> 24)) ) << 24 |
  (wxUint8)( min(255, (int)((cmb.ccolor & 0x00FF0000) >> 16) + (int)((rdp.prim_color & 0x00FF0000) >> 16)) ) << 16 |
  (wxUint8)( min(255, (int)((cmb.ccolor & 0x0000FF00) >>  8) + (int)((rdp.prim_color & 0x0000FF00) >>  8)) ) <<  8 ;
}

static void cc__env_inter_prim_using_t0a__mul_t0 ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  MOD_0 (TMOD_COL_INTER_COL1_USING_TEXA__MUL_TEX);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  MOD_0_COL1 (rdp.prim_color & 0xFFFFFF00);
  USE_T0 ();
}

static void cc__env_inter_prim_using_t0a__mul_prim ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CC_PRIM ();
  MOD_0 (TMOD_COL_INTER_COL1_USING_TEXA);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  MOD_0_COL1 (rdp.prim_color & 0xFFFFFF00);
  USE_T0 ();
}

static void cc__env_inter_prim_using__t0_sub_shade_mul_primlod_add_env ()
{
  // (t0-shade)*lodf+env, (prim-env)*cmb+env
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = rdp.env_color;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    cmb.tex |= 1;
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_RGB,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
    CC_PRIM ();
    SETSHADE_ENV ();
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
      GR_COMBINE_FACTOR_ONE,//TEXTURE_RGB,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);//CONSTANT);
    MOD_0 (TMOD_COL_INTER_COL1_USING_TEX);
    MOD_0_COL (rdp.env_color & 0xFFFFFF00);
    MOD_0_COL1 (rdp.prim_color & 0xFFFFFF00);
    USE_T0 ();
    MULSHADE_PRIMSUBENV ();
    MULSHADE_PRIMLOD();
    SUBSHADE_PRIMSUBENV ();
  }
}

static void cc__prim_inter_t0_using_t0__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MOD_0 (TMOD_COL_INTER_TEX_USING_TEX);
  MOD_0_COL (rdp.prim_color & 0xFFFFFF00);
  USE_T0 ();
}

static void cc__env_inter_t0_using_t0a__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MOD_0 (TMOD_COL_INTER_TEX_USING_TEXA);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  USE_T0 ();
}

static void cc__env_inter_t0_using_prima__mul_shade ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MOD_0 (TMOD_COL_INTER_TEX_USING_COL1);
  MOD_0_COL (rdp.env_color & 0xFFFFFF00);
  wxUint32 prima = rdp.prim_color & 0xFF;
  MOD_0_COL1 ((prima<<24)|(prima|16)|(prima<<8));
  USE_T0 ();
}

static void cc_shade_mul_prima ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_ITERATED);
  MULSHADE_PRIMA ();
}

static void cc_shade_mul_shadea ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_ITERATED);
  MULSHADE_SHADEA ();
}

static void cc__t0_mul_shade__inter_env_using_enva ()
{
  // (t0-0)*shade+0, (env-cmb)*env_a+cmb    ** INC **
  wxUint32 enva  = rdp.env_color&0xFF;
  if (enva == 0xFF)
    cc_env ();
  else if (enva == 0)
    cc_t0_mul_shade ();
  else if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CCOLOR, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    MULSHADE_1MENVA ();
    CC_COLMULBYTE(rdp.env_color, (rdp.env_color&0xFF));
    cmb.tex_ccolor = cmb.ccolor;
  }
  else
  {
    CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    INTERSHADE_2 (rdp.env_color & 0xFFFFFF00, rdp.env_color & 0xFF);
    USE_T0 ();
    MOD_0 (TMOD_TEX_INTER_COLOR_USING_FACTOR);
    MOD_0_COL (rdp.env_color & 0xFFFFFF00);
    MOD_0_FAC (rdp.env_color & 0xFF);
  }
}

static void cc__t0_mul_shade__inter_one_using_enva ()
{
  CCMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_RGB,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CC_ENVA ();
  MULSHADE_1MENVA ();
  USE_T0 ();
}

static void cc__t0_mul_shade__inter_one_using_shadea ()
{
  if (cmb.combine_ext)
  {
    T0CCMBEXT(GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
      GR_CMBX_ITRGB, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    CCMBEXT(GR_CMBX_ZERO, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_B, 0);
  }
  else
  {
    cc_t0_mul_shade ();
  }
}

static void cc__prim_mul_shade__inter_env_using_enva ()
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  MULSHADE_PRIM ();
  SETSHADE_A_ENV ();
}

static void cc__prim_mul_shade__inter_env_using__prim_mul_shade_alpha ()
{
  CCMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CC_ENV ();
  MULSHADE_PRIM ();
  MULSHADE_A_PRIM ();
}


//****************************************************************

static void ac_one ()
{
  ACMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  cmb.ccolor |= 0xFF;
}

static void ac_t0 ()
{
  if ((rdp.othermode_l & 0x4000) && (rdp.cycle_mode < 2))
  {
    wxUint32 blend_mode = (rdp.othermode_l >> 16);
    if (blend_mode == 0x0550)
    {
      ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
        GR_COMBINE_FACTOR_LOCAL,
        GR_COMBINE_LOCAL_CONSTANT,
        GR_COMBINE_OTHER_TEXTURE);
      CA(rdp.fog_color);
    }
    else if (blend_mode == 0x55f0) //cmem*afog + cfog*1ma
    {
      ACMB (GR_COMBINE_FUNCTION_LOCAL,
        GR_COMBINE_FACTOR_ONE,
        GR_COMBINE_LOCAL_CONSTANT,
        GR_COMBINE_OTHER_NONE);
      CA(~rdp.fog_color);
    }
    else
    {
      ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
        GR_COMBINE_FACTOR_ONE,
        GR_COMBINE_LOCAL_NONE,
        GR_COMBINE_OTHER_TEXTURE);
    }
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
  }
  A_USE_T0 ();
}

static void ac_zero ()
{
  if (cmb.tex > 0)
  {
    ac_t0 ();
    return;
  }
  ACMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  cmb.ccolor &= 0xFFFFFF00;
}

static void ac_t1 ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  if ((settings.hacks&hack_BAR) && rdp.tiles[rdp.cur_tile].format == 3)
    A_USE_T0 ();
  else
    A_USE_T1 ();
}

static void ac_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  CA_PRIM ();
}

static void ac_primlod ()
{
  ACMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  CA_PRIMLOD ();
}

static void ac_one_sub_t0 ()
{
    ACMB (GR_COMBINE_FUNCTION_BLEND_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_ALPHA,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_NONE);
    CA (0xFF);
    A_USE_T0 ();
}

static void ac_one_sub_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  CA_INVPRIM ();
}

static void ac_env ()
{
  ACMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  CA_ENV ();
}

static void ac_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_LOCAL,
    GR_COMBINE_FACTOR_NONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_NONE);
}

// ** A+B **
static void ac_t0_add_t1 ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  A_T0_ADD_T1 ();
}

static void ac__t0_mul_prim__add__t1_mul_primlod ()  //Aded by Gonetz
{
  if (lod_frac == 0)
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CA_PRIM ();
    A_USE_T0 ();
  }
  else if ((rdp.prim_color&0xFF) == 0)
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CA_PRIMLOD ();
    A_USE_T1 ();
  }
  else if ((rdp.prim_color&0xFF) == 0xFF)
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
    A_T1_MUL_PRIMLOD_ADD_T0();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CA_PRIM ();
    A_T0_ADD_T1 ();
  }
}

static void ac_t0_add_prim () //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_USE_T0 ();
}

static void ac_t0_add_env () //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
  A_USE_T0 ();
}

static void ac_t1_add_env ()  //Added by Gonetz
{
ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
GR_COMBINE_FACTOR_ONE,
GR_COMBINE_LOCAL_CONSTANT,
GR_COMBINE_OTHER_TEXTURE);
CA_ENV ();
A_USE_T1 ();
}

static void ac__t0_add_t1__add_prim () //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_T0_ADD_T1 ();
}

static void ac_prim_add_shade () //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_PRIM ();
}

static void ac_env_add_shade () //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_ENV ();
}

// ** A*B **
static void ac_t0_mul_t0 () //Added by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  A_USE_T0 ();
}

static void ac_t0_mul_t1 ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  A_T0_MUL_T1 ();
}

static void ac_t0_mul_t1_add_t1 ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
  }
  else
  {
    A_T0_MUL_T1 ();
  }
}

static void ac_t0_mul_t1_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_T0_MUL_T1 ();
}

static void ac_t0_mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_USE_T0 ();
}

static void ac_t0_mul_prim_mul_primlod ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM_MUL_PRIMLOD ();
  A_USE_T0 ();
}

static void ac_t1_mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (rdp.cycle_mode == 0)
    A_USE_T0 ();
  else
    A_USE_T1 ();
}

//Added by Gonetz
static void ac__t1_sub_one_mul_primlod_add_t0__mul_prim ()
{
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (0xFF) ;
    percent = (float)lod_frac / 255.0f;
  }
  else
  {
    cmb.tmu1_a_func = GR_COMBINE_FUNCTION_BLEND_LOCAL;
    cmb.tmu1_a_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR;
    percent = (255 - lod_frac) / 255.0f;
    cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA;
    cmb.tmu0_a_fac = GR_COMBINE_FACTOR_OTHER_ALPHA;
  }
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  cmb.tex |= 3;
}

static void ac__t0_sub_t1_mul_enva_add_t0__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.env_color&0xFF) ;
    cmb.tex |= 3;
  }
  else
  {
    A_T0_MUL_T1 ();
  }
}

static void ac__t0_sub_one_mul_enva_add_t0__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_ITALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    SETSHADE_A(0xFF);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.env_color&0xFF) ;
    cmb.tex |= 1;
  }
  else
  {
    A_USE_T0 ();
  }
}

static void ac__t0_sub_t1_mul_primlod_add_t0__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex |= 3;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    A_T0_INTER_T1_USING_FACTOR (lod_frac);
  }
}

static void ac__t1_sub_prim_mul_primlod_add_t0__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex |= 3;
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.prim_color&0xFF);
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    A_T0_INTER_T1_USING_FACTOR (lod_frac);
  }
}

static void ac__t1_sub_t0_mul_enva_add_t1__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.env_color&0xFF);
  }
  else
  {
    wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
    A_T0_INTER_T1_USING_FACTOR (factor);
  }
}

static void ac__t1_sub_t0_mul_primlod__mul_env_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (wxUint32)((float)(rdp.env_color&0xFF)*(float)rdp.prim_lodfrac/255.0f);
  }
  else
  {
    cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL;
    cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL;
    cmb.tmu0_a_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR;
    percent = (rdp.prim_lodfrac * (rdp.env_color&0xFF)) / 65025.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent; \
  }
  cmb.tex |= 3;
}

static void ac__t0_sub_one_mul_enva_add_t1__mul_prim ()
{
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.env_color&0xFF) ;
    cmb.tex |= 3;
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    CA_ENV ();
    SETSHADE_A_PRIM ();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    SETSHADE_A_PRIM ();
    SETSHADE_A_ENV ();
    A_T0_MUL_T1 ();
  }
}

static void ac__t1_mul_prima_add_t0__mul_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
  A_T1_MUL_PRIMA_ADD_T0 ();
}

static void ac__t1_mul_enva_add_t0__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_T1_MUL_ENVA_ADD_T0 ();
}

static void ac_t0_mul_primlod ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIMLOD ();
  A_USE_T0 ();
}

static void ac_t1_mul_primlod ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIMLOD ();
  A_USE_T1 ();
}

//Added by Gonetz
static void ac__t0_add_t1__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_T0_ADD_T1 ();
}

//Added by Gonetz
static void ac__t0_add_t1__mul_primlod ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIMLOD ();
  A_T0_ADD_T1 ();
}

//Added by Gonetz
static void ac__t0_mul_t1__mul_primlod ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIMLOD ();
  A_T0_MUL_T1 ();
}

static void ac_t0_mul_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
  A_USE_T0 ();
}

static void ac_t0_mul_env_mul_primlod ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV_MUL_PRIMLOD ();
  A_USE_T0 ();
}

static void ac_t1_mul_env () //Added by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
  //  if ((settings.hacks&hack_Powerpuff) && (rdp.last_tile == 0))
  if (rdp.cycle_mode == 0)
    A_USE_T0 ();
  else
    A_USE_T1 ();
}

static void ac__t1_sub_one_mul_primlod_add_t0__mul_env ()
{
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (0xFF) ;
    percent = (float)lod_frac / 255.0f;
  }
  else
  {
    cmb.tmu1_a_func = GR_COMBINE_FUNCTION_BLEND_LOCAL;
    cmb.tmu1_a_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR;
    percent = (255 - lod_frac) / 255.0f;
    cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA;
    cmb.tmu0_a_fac = GR_COMBINE_FACTOR_OTHER_ALPHA;
  }
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  cmb.tex |= 3;
}

static void ac_t0_mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  A_USE_T0 ();
}

static void ac_t1_mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  A_USE_T1 ();
}

//Added by Gonetz
static void ac__t0_add_t1__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  A_T0_ADD_T1 ();
}

static void ac__t0_mul_primlod_add_t0__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex |= 1;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    A_USE_T0 ();
  }
}

static void ac__t1_mul_prima_add_t0__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  A_T1_MUL_PRIMA_ADD_T0 ();
}

//Added by Gonetz
static void ac__t0_sub_t1__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 3;
  }
  else
  {
    A_T0_SUB_T1 ();
  }
}

static void ac__t1_mul_t1_add_t1__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 2;
  }
  else
  {
    A_USE_T1 ();
  }
}

static void ac__t1_mul_enva_add_t0__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  A_T1_MUL_ENVA_ADD_T0 ();
}

static void ac__t1_sub_one_mul_primlod_add_t0__mul_shade ()
{
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (0xFF) ;
    percent = (float)lod_frac / 255.0f;
  }
  else
  {
    cmb.tmu1_a_func = GR_COMBINE_FUNCTION_BLEND_LOCAL;
    cmb.tmu1_a_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR;
    percent = (255 - lod_frac) / 255.0f;
    cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA;
    cmb.tmu0_a_fac = GR_COMBINE_FACTOR_OTHER_ALPHA;
  }
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  cmb.tex |= 3;
}

static void ac__t1_sub_shade_mul_primlod_add_t0__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex |= 3;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    A_T0_INTER_T1_USING_FACTOR (lod_frac);
  }
}

//Added by Gonetz
static void ac_prim_mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_PRIM ();
  SETSHADE_A_PRIM ();
}

//Added by Gonetz
static void ac_prim_mul_primlod ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_PRIMLOD ();
  SETSHADE_A_PRIM ();
}

static void ac_prim_mul_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_ENV ();
  SETSHADE_A_PRIM ();
}

static void ac__prim_sub_one_mul_primlod_add_t0__mul_env ()
{
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_ITALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    SETSHADE_A_PRIM ();
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (0xFF) ;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    cmb.tex |= 1;
  }
  else
  {
    A_USE_T0 ();
  }
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
}

static void ac_prim_mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_PRIM ();
}

static void ac_env_mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_ENV ();
}

static void ac_primlod_mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_PRIMLOD ();
}

// ** A-B **
static void ac_prim_sub_t0 ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  A_USE_T0 ();

  MOD_0 (TMOD_FULL_COLOR_SUB_TEX);
  MOD_0_COL (rdp.prim_color);
}

// ** A*B+C **
static void ac_t0_mul_prim_add_t0 ()
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_ALPHA, 0,
      GR_CMBX_B, 0);
    CA_PRIM ();
    A_USE_T0 ();
  }
  else
    ac_t0();
}

static void ac_t1_mul_prim_add_t0 ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  A_T1_MUL_PRIMA_ADD_T0 ();
}

static void ac__t0_inter_t1_using_t1a__mul_prim_add__t0_inter_t1_using_t1a ()
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_ALPHA, 0,
      GR_CMBX_B, 0);
    CA_PRIM ();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CA_PRIM ();
  }
  A_T0_INTER_T1_USING_T1A ();
}

static void ac__t1_inter_t0_using_t0a__mul_prim_add__t1_inter_t0_using_t0a ()
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_ALPHA, 0,
      GR_CMBX_B, 0);
    CA_PRIM ();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CA_PRIM ();
  }
  A_T1_INTER_T0_USING_T0A ();
}

//Added by Gonetz
static void ac_t0_mul_prim_add_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_A_PRIM ();
  CA_ENV ();
  A_USE_T0 ();
}

//Added by Gonetz
static void ac__t0_add_t1__mul_prim_add_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_A_PRIM ();
  CA_ENV ();
  A_T0_ADD_T1 ();
}

//Aded by Gonetz
static void ac__t0_inter_t1_using_enva__mul_prim_add_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_A_PRIM ();
  CA_ENV ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  A_T0_INTER_T1_USING_FACTOR (factor);
}

//Aded by Gonetz
static void ac_t0_mul_primlod_add_t0 ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex |= 1;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
  }
  else
  {
    A_USE_T0 ();
  }
}

//Aded by Gonetz
static void ac_t1_mul_primlod_add_t0 ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  A_T1_MUL_PRIMLOD_ADD_T0 ();
}

//Aded by Gonetz
static void ac_t0_mul_primlod_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_PRIMLOD ();
  SETSHADE_A_PRIM ();
  A_USE_T0 ();
}

static void ac_t0_mul_primlod_add_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_PRIMLOD ();
  SETSHADE_A_ENV ();
  A_USE_T0 ();
}

//Aded by Gonetz
static void ac__t0_add_t1__mul_primlod_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_PRIMLOD ();
  SETSHADE_A_PRIM ();
  A_T0_ADD_T1 ();
}

//Added by Gonetz
static void ac_t0_mul_env_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_A_ENV ();
  CA_PRIM ();
  A_USE_T0 ();
}

//Added by Gonetz
static void ac_t1_mul_prim_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_A_PRIM ();
  CA_PRIM ();
  A_USE_T1 ();
}

//Added by Gonetz
static void ac_prim_mul_shade_add_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_PRIM ();
}

//Added by Gonetz
static void ac_t0_mul_shade_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_PRIM ();
  A_USE_T0 ();
}

static void ac_t0_mul_shade_add_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_ENV ();
  A_USE_T0 ();
}

static void ac_one_sub_prim_mul__t0_mul_t1__add__prim_mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_INVPRIM ();
  MULSHADE_A_PRIM ();
  A_T0_MUL_T1 ();
}

// ** A*B+C*D **
static void ac_t0_mul_prim_add_shade_mul_one_minus_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  MULSHADE_A_1MPRIM ();
  CA_PRIM ();
  A_USE_T0 ();
}

// ** (A*B+C)*D **
static void ac__t0_mul_primlod_add_shade__mul_shade ()
{
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    A_USE_T0 ();
  }
}

static void ac__t1_mul_primlod_add_shade__mul_shade ()
{
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 2;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    A_USE_T1 ();
  }
}

// ** ((A-B)*C+D)+E **
static void ac__t0_sub_t1_mul_prim_add_shade__mul_shade ()
 //(t0-t1)*prim+shade, (cmb-0)*shade+0
{
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 3;
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.prim_color&0xFF);
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA,
      GR_COMBINE_FACTOR_TEXTURE_ALPHA,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
    CA_PRIM ();
    A_T1_SUB_T0 ();
  }
}

static void ac__t1_sub_t0_mul_prim_add_shade__mul_shade ()
 //(t1-t0)*prim+shade, (cmb-0)*shade+0
{
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 3;
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.prim_color&0xFF);
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA,
      GR_COMBINE_FACTOR_TEXTURE_ALPHA,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_CONSTANT);
    CA_PRIM ();
    A_T1_SUB_T0 ();
  }
}

// ** A*B*C **
static void ac__t0_mul_t1__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (voodoo.sup_large_tex || rdp.tiles[1].lr_s < 256) //hack for RR64 pause screen
  {
    A_T0_MUL_T1 ();
  }
  else
  {
    A_USE_T0 ();
  }
}

static void ac__t0_mul_t1__mul_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
  A_T0_MUL_T1 ();
}

static void ac__t0_mul_t1__mul_env_mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_A_ENV ();
  A_T0_MUL_T1 ();
}

static void ac__t0_mul_t1__mul_prim_mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_A_PRIM ();
  A_T0_MUL_T1 ();
}

static void ac__t0_mul_t1__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  A_T0_MUL_T1 ();
}

static void ac__t0_add_prim_mul_shade__mul_shade ()
{
  // (shade-0)*prim+t0, (cmb-0)*shade+0
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_ITALPHA, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.prim_color&0xFF);
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_A_PRIM ();
    A_USE_T0();
  }
}

//Added by Gonetz
static void ac_t0_mul_prim_mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_A_PRIM ();
  SETSHADE_A_PRIM ();
  A_USE_T0 ();
}

static void ac_t0_mul_prim_mul_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIMENV();
  A_USE_T0 ();
}

static void ac_t0_mul_prim_mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_A_PRIM ();
  A_USE_T0 ();
}

static void ac_t1_mul_prim_mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_A_PRIM ();
  A_USE_T1 ();
}

static void ac_t0_mul_env_mul_shade ()
{
  if (rdp.cur_image && (rdp.cur_image->format != 0))
  {
    ac_shade ();
    return;
  }

  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_A_ENV ();
  A_USE_T0 ();
}

static void ac_t1_mul_env_mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  MULSHADE_A_ENV ();
  A_USE_T1 ();
}

static void ac_t0_mul_primlod_mul_prim () //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  cmb.ccolor |= (wxUint32)(lod_frac * (rdp.prim_color&0xFF) / 255);
  A_USE_T0 ();
}

// ** (A+B)*C **
static void ac_prim_add_env_mul_t0 () //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  SETSHADE_A_PRIM ();
  ADDSHADE_A_ENV ();
  A_USE_T0 ();
}

static void ac_t1_add_prim_mul_env () //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_ENV ();
  SETSHADE_A_PRIM ();
  SETSHADE_A_ENV ();
  A_USE_T1 ();
  //(t1+prim)*env = t1*env + prim*env
}

// ** (A-B)*C **
static void ac_t0_sub_prim_mul_shade ()
{
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.prim_color&0xFF);
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
  } else {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_A_PRIM ();
    A_USE_T0 ();
  }
}

static void ac_t0_sub_prim_mul_shade_mul_env ()
{
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.prim_color&0xFF);
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_ALPHA, 0,
      GR_CMBX_ZERO, 0);
    CA_ENV ();
  } else {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_A_PRIM ();
    MULSHADE_A_ENV ();
    A_USE_T0 ();
  }
}

static void ac_t0_sub_shade_mul_prim ()
{
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex |= 1;
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.prim_color&0xFF);
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
  } else {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_A_PRIM ();
    A_USE_T0 ();
  }
}

static void ac__t0_mul_t1__sub_prim_mul_shade ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    CA_PRIM();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
  }
  A_T0_MUL_T1 ();
}

static void ac__one_sub_t1_mul_t0_add_shade__sub_prim_mul_shade ()  //Aded by Gonetz
{
  // (1-t1)*t0+shade, (cmb-prim)*shade+0
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex |= 3;
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    CA_PRIM();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    A_T0_MUL_T1 ();
  }
}

static void ac__t1_mul_primlod_add_t0__sub_prim_mul_shade ()
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    CA_PRIM ();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
  }
  A_T1_MUL_PRIMLOD_ADD_T0 ();
}

static void ac__t1_mul_primlod_add_t0__sub_env_mul_prim ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    CA_ENV ();
    SETSHADE_A_PRIM ();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CA_PRIM ();
  }
  A_T1_MUL_PRIMLOD_ADD_T0 ();
}

static void ac__t1_mul_prima_add_t0__sub_env_mul_shade ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    CA_ENV ();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
  }
  A_T1_MUL_PRIMA_ADD_T0 ();
}

static void ac_one_sub_t0_mul_prim ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_BLEND_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_NONE);
  CA_PRIM ();
  A_USE_T0 ();
}

static void ac_one_sub_t0_mul_shade ()  //Aded by Gonetz
{
  if (rdp.aTBuffTex[0] || rdp.aTBuffTex[1])
  {
    ACMB (GR_COMBINE_FUNCTION_BLEND_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_ALPHA,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_NONE);
    A_USE_T0 ();
  }
  else
    ac_zero();
}

static void ac_one_sub_prim_mul_t0 ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_USE_T0 ();
}

static void ac_one_sub_env_mul_t0 ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
  A_USE_T0 ();
}

static void ac_one_sub_shade_mul_t0 ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  A_USE_T0 ();
}

static void ac_one_sub_shade_mul_env ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_ENV ();
}

static void ac_prim_sub_shade_mul_t0 ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_PRIM ();
  A_USE_T0 ();
}

static void ac_prim_sub_shade_mul_prim ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_CONSTANT_ALPHA, 0,
      GR_CMBX_ZERO, 0);
    CA_PRIM();
  }
  else
  {
    if (!(rdp.prim_color & 0xFF))
    {
      ac_zero();
    }
    else
    {
      ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
        GR_COMBINE_FACTOR_ONE,
        GR_COMBINE_LOCAL_ITERATED,
        GR_COMBINE_OTHER_CONSTANT);
      CA_PRIM();
    }
  }
}

static void ac_shade_sub_env_mul_t0 ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_ENV ();
  A_USE_T0 ();
}

// ** (A-B)*C*D **
static void ac_one_sub_t0_mul_prim_mul_shade ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_BLEND_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_NONE);
  MULSHADE_A_PRIM ();
  A_USE_T0 ();
}

// ** (A+B)*C*D **
static void ac_one_plus_env_mul_prim_mul_shade ()
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_ZERO, GR_FUNC_MODE_ONE_MINUS_X,
      GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    MULSHADE_A_PRIM ();
    CA_ENV();
  }
  else
    ac_prim_mul_shade ();
}

// ** (A-B)*C+A **
static void ac__t0_mul_t1__sub_env_mul_prim_add__t0_mul_t1 ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_TEXTURE_ALPHA, 0);
    CA_ENV();
    SETSHADE_A_PRIM ();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
  }
  A_T0_MUL_T1 ();
}

// ** (A-B)*C+D **
static void ac__t0_sub_prim_mul_shade_add_shade__mul_env ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ITALPHA, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.prim_color&0xFF) ;
    cmb.tex |= 1;
    ACMBEXT(GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
      GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, 0,
      GR_CMBX_ZERO, 0);
    CA_ENV();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_A_ENV ();
    MOD_0 (TMOD_TEX_SUB_COL);
    MOD_0_COL (rdp.prim_color & 0xFF);
    A_USE_T0 ();
  }
}

static void ac_t0_sub_t1_mul_env_add_env ()  //Aded by Gonetz
{
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CA_ENV();
    A_T0_SUB_T1 ();
}

static void ac_t0_sub_one_mul_enva_add_t1 ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.env_color&0xFF) ;
    cmb.tex |= 3;
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CA_ENV();
  }
  else
  {
    ac__t0_mul_t1__mul_env ();
  }
}

static void ac_t1_sub_one_mul_enva_add_t0 ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_B, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.env_color&0xFF) ;
    SETSHADE_A (0xFF);
    cmb.tex |= 3;
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
    A_USE_T0 ();
  }
}

static void ac_t1_sub_one_mul_primlod_add_t0 ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (0xFF) ;
    percent = (float)lod_frac / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    cmb.tex |= 3;
  }
  else
  {
    //    A_T0_MUL_T1 ();
    //    A_T1_MUL_PRIMLOD_ADD_T0 ();
    cmb.tmu1_a_func = GR_COMBINE_FUNCTION_BLEND_LOCAL;
    cmb.tmu1_a_fac = GR_COMBINE_FACTOR_DETAIL_FACTOR;
    percent = (255 - lod_frac) / 255.0f;
    cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA;
    cmb.tmu0_a_fac = GR_COMBINE_FACTOR_OTHER_ALPHA;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    cmb.tex |= 3;
  }
}

static void ac_t1_sub_prim_mul_shade_add_prim ()  //Aded by Gonetz
{
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_B, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.prim_color&0xFF) ;
    cmb.tex |= 2;
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
      GR_COMBINE_FACTOR_TEXTURE_ALPHA,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    CA_PRIM ();
    MOD_1 (TMOD_TEX_SUB_COL);
    MOD_1_COL (rdp.prim_color & 0xFF);
    A_USE_T1 ();
  }
}

static void ac_t0_sub_env_mul_prim_add_env ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_A_PRIM ();
  CA_ENV1MPRIM ();
  A_USE_T0 ();
  //(t0-env)*prim+env == t0*prim + env*(1-prim)
}

static void ac_t0_sub_env_mul_shadea_add_env ()  //Aded by Gonetz
{
  if (!cmb.combine_ext)
  {
    ac_t0_mul_shade ();
    return;
  }
  T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
    GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
    GR_CMBX_ZERO, 1,
    GR_CMBX_ZERO, 0);
  cmb.tex |= 1;
  ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
    GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
    GR_CMBX_ITALPHA, 0,
    GR_CMBX_B, 0);
  CA_ENV ();
}

static void ac__one_sub_t0_mul_t1_add_t0__mul_prim ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  rdp.best_tex = 0;
  cmb.tex |= 3;
  cmb.tmu1_a_func = GR_COMBINE_FUNCTION_LOCAL;
  cmb.tmu0_a_func = GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL;
  cmb.tmu0_a_fac = GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA;
}

static void ac_one_sub_t0_mul_prim_add_t0 ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  SETSHADE_A_PRIM ();
  CA (0xFF);
  A_USE_T0 ();
}

static void ac_one_sub_t0_mul_env_add_t0 ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_BLEND,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  SETSHADE_A_ENV ();
  CA (0xFF);
  A_USE_T0 ();
}

static void ac_one_sub_t0_mul_primlod_add_prim ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  SETSHADE_A_PRIM ();
  CA_PRIMLOD();
  A_USE_T0 ();
}

static void ac_prim_sub_t0_mul_env_add_t0 ()  //Aded by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_INVENV ();
  SETSHADE_A_PRIM ();
  SETSHADE_A_ENV ();
  A_USE_T0 ();
  //(prim-t0)*env+t0 = prim*env + t0*(1-env)
}

static void ac_prim_sub_env_mul_t0_add_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_PRIM ();
  SETSHADE_A_ENV ();
  A_USE_T0 ();
}

static void ac_prim_sub_env_mul_t1_add_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_PRIM ();
  SETSHADE_A_ENV ();
  A_USE_T1 ();
}

static void ac_prim_sub_env_mul_t0_add_one ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA (0xFF);
  SETSHADE_A_PRIMSUBENV ();
  A_USE_T0 ();
}

//Added by Gonetz
static void ac_prim_sub_env_mul_shade_add_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_ENV ();
  MULSHADE_A_PRIMSUBENV ();
}

//Added by Gonetz
static void ac_prim_sub_env_mul_shade_add_env_mul_t1 ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_ENV ();
  MULSHADE_A_PRIMSUBENV ();
  A_USE_T1 ();
}

//Added by Gonetz
static void ac_prim_sub_shade_mul_t0_add_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_PRIM ();
  A_USE_T0 ();
}

//Added by Gonetz
static void ac_one_sub_shade_mul_t1_add_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  A_USE_T1 ();
}

//Added by Gonetz
static void ac_one_sub_env_mul_shade_add_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_ENV ();
}

//Added by Gonetz
static void ac_env_sub_prim_mul_t0_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_ENV ();
  SETSHADE_A_PRIM ();
  A_USE_T0 ();
}

static void ac_one_sub_t1_add_t0_mul_env ()
{
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_B, 1);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.env_color&0xFF);
    cmb.tex |= 3;
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    CA_ENV ();
    A_T0_ADD_T1();
    cmb.tmu1_a_invert = FXTRUE;
  }
}

static void ac_env_sub_prim_mul_shade_add_prim () //Added by Gonetz
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_PRIM ();
  MULSHADE_A_ENVSUBPRIM ();
}

static void ac_env_sub_primshade_mul_t1_add_primshade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_ENV ();
  MULSHADE_A_PRIM ();
  A_USE_T1 ();
}

static void ac_one_sub_prim_mul_t0_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA (0xFF);
  SETSHADE_A_PRIM ();
  A_USE_T0 ();
}

static void ac_one_sub_prim_mul_t0_add__prim_mul_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_CONSTANT);
  CA_INVPRIM ();
  SETSHADE_A_PRIM ();
  SETSHADE_A_ENV ();
  A_USE_T0 ();
}

static void ac_shade_sub_t0_mul_primlod_add_prim ()
{
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_ITALPHA, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_TMU_CALPHA, 0,
      GR_CMBX_ZERO, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (lod_frac&0xFF);
    cmb.tex |= 1;
    ACMBEXT(GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_TEXTURE_ALPHA, 0);
    CA_PRIM ();
  }
  else
    ac_t0();
}

static void ac_shade_sub_env_mul_t0_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SUBSHADE_A_ENV ();
  CA_PRIM ();
  A_USE_T0 ();
}

// ** A inter B using C **
static void ac_t0_inter_t1_using_prima ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  A_T0_INTER_T1_USING_FACTOR (factor);
}

static void ac_t1_inter_t0_using_prima ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  A_T1_INTER_T0_USING_FACTOR (factor);
}

static void ac_t0_inter_t1_using_primlod ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  A_T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void ac_t0_inter_t1_using_enva ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  A_T0_INTER_T1_USING_FACTOR (factor);
}

static void ac_t1_inter_t0_using_enva ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  A_T1_INTER_T0_USING_FACTOR (factor);
}

//Added by Gonetz
static void ac_t0_inter_t1_using_t0a ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  A_T0_INTER_T1_USING_T0A ();
}

//Added by Gonetz
static void ac_t0_inter_t1_using_t1a ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_ONE,
    GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE);
  A_T0_INTER_T1_USING_T1A ();
}

//Added by Gonetz
static void ac_t0_inter_t1_using_shadea ()
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_ONE,
      GR_COMBINE_LOCAL_NONE,
      GR_COMBINE_OTHER_TEXTURE);
  }
  A_T0_INTER_T1_USING_SHADEA ();
}

// ** (A inter B using C) * D **

static void ac__t0_inter_t1_using_primlod__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void ac__t1_mul_primlod_add_t0__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_T1_MUL_PRIMLOD_ADD_T0 ();
}

static void ac__t0_inter_t1_using_primlod__mul_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
  A_T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void ac__t1_mul_primlod_add_t0__mul_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
  A_T1_MUL_PRIMLOD_ADD_T0 ();
}

static void ac__t0_inter_t1_using_primlod__mul_shade ()
{
  if (settings.hacks & hack_Makers)
  {
    //rolling rock issue - it has zero shade alpha and thus rejected by alpha compare
    ac_t0_inter_t1_using_primlod();
    return;
  }
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  A_T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void ac__t1_mul_primlod_add_t0__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  A_T1_MUL_PRIMLOD_ADD_T0 ();
}

//Added by Gonetz
static void ac__t0_inter_t1_using_prima__mul_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  A_T0_INTER_T1_USING_FACTOR (factor);
}

//Added by Gonetz
static void ac__t1_inter_t0_using_t0a__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_T1_INTER_T0_USING_T0A ();
}

static void ac__t1_inter_t0_using_primlod__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void ac__t1_inter_t0_using_prima__mul_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_ENV ();
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  A_T1_INTER_T0_USING_FACTOR (factor);
}

//Added by Gonetz
static void ac__t0_inter_t1_using_prima__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  A_T0_INTER_T1_USING_FACTOR (factor);
}

static void ac__t1_inter_t0_using_prima__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.prim_color&0xFF);
  A_T1_INTER_T0_USING_FACTOR (factor);
}

static void ac__t0_inter_t1_using_enva__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  A_T0_INTER_T1_USING_FACTOR (factor);
}

static void ac__env_sub_one_mul_t1_add_t0__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0,
      GR_CMBX_ZERO, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ZERO, 1,
      GR_CMBX_ZERO, 0);
    SETSHADE_A(0xFF);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (rdp.env_color&0xFF) ;
    cmb.tex |= 3;
  }
  else
  {
    wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
    A_T0_INTER_T1_USING_FACTOR (factor);
  }
}

static void ac__t0_inter_t1_using_enva__mul_primlod ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIMLOD ();
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  A_T0_INTER_T1_USING_FACTOR (factor);
}

static void ac__t1_mul_enva_add_t0__sub_prim_mul_shade ()
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ZERO, 0);
    CA_PRIM ();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_ITERATED,
      GR_COMBINE_OTHER_TEXTURE);
    MULSHADE_A_PRIM ();
  }
  A_T1_MUL_ENVA_ADD_T0 ();
}

//Added by Gonetz
static void ac__t0_inter_t1_using_t0a__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_T0_INTER_T1_USING_T0A ();
}

//Added by Gonetz
static void ac__t0_inter_t1_using_t1a__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  A_T0_INTER_T1_USING_T1A ();
}

static void ac__t0_inter_t1_using_t1a__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  A_T0_INTER_T1_USING_T1A ();
}

//Added by Gonetz
static void ac__t0_inter_t1_using_shadea__mul_prim ()
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_ALPHA, 0,
      GR_CMBX_ZERO, 0);
    A_T0_INTER_T1_USING_SHADEA ();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    A_T0_INTER_T1_USING_FACTOR (0x7F);
  }
  CA_PRIM ();
}

//Added by Gonetz
static void ac__t0_inter_t1_using_shadea__mul_env ()
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_ITALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_ALPHA, 0,
      GR_CMBX_ZERO, 0);
    A_T0_INTER_T1_USING_SHADEA ();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
      GR_COMBINE_FACTOR_LOCAL,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_TEXTURE);
    A_T0_INTER_T1_USING_FACTOR (0x7F);
  }
  CA_ENV ();
}

static void ac__t0_inter_t1_using_primlod__sub_env_mul_shade_add_shade ()
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_ITALPHA, 0,
      GR_CMBX_ALOCAL, 0);
    CA_ENV ();
    A_T0_INTER_T1_USING_FACTOR (lod_frac);
  }
  else
    ac__t0_inter_t1_using_primlod__mul_shade ();
}

//Added by Gonetz
static void ac__t0_inter_t1_using_enva__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
  A_T0_INTER_T1_USING_FACTOR (factor);
}

static void ac__t0_inter_t1_using_primlod__mul_prim_add_env ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_ENV ();
  SETSHADE_A_PRIM ();
  A_T0_INTER_T1_USING_FACTOR (lod_frac);
}

//Added by Gonetz
static void ac__t0_inter_t1_using_primlod__mul_shade_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  CA_PRIM ();
  A_T0_INTER_T1_USING_FACTOR (lod_frac);
}

//Added by Gonetz
static void ac__t0_inter_t1_using_primlod__mul_env_add__t0_inter_t1_using_primlod ()
{
  if (cmb.combine_ext)
  {
    ACMBEXT(GR_CMBX_ZERO, GR_FUNC_MODE_ZERO,
      GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, 0,
      GR_CMBX_B, 0);
    CA_ENV ();
  }
  else
  {
    ACMB (GR_COMBINE_FUNCTION_BLEND,
      GR_COMBINE_FACTOR_TEXTURE_ALPHA,
      GR_COMBINE_LOCAL_CONSTANT,
      GR_COMBINE_OTHER_ITERATED);
    SETSHADE_A_ENV ();
    CA (0xFF);
  }
  A_T0_INTER_T1_USING_FACTOR (lod_frac);
}

static void ac__t1_sub_one_mul_enva_add_t0__mul_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (0xFF) ;
    cmb.tex |= 3;
    cmb.dc0_detailmax = cmb.dc1_detailmax = (float)(rdp.env_color&0xFF) / 255.0f;
  }
  else
  {
    // (t1-1)*env+t0, (cmb-0)*prim+0
    A_T0_MUL_T1 ();

    MOD_1 (TMOD_TEX_SCALE_FAC_ADD_FAC);
    MOD_1_FAC (rdp.env_color & 0xFF);
  }
}

static void ac__one_inter_t0_using_prim__mul_env ()
{
  if (cmb.combine_ext)
  {
    T0ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_B, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | (0xFF) ;
    cmb.tex |= 1;
    cmb.dc0_detailmax = cmb.dc1_detailmax = (float)(rdp.prim_color&0xFF) / 255.0f;
    ACMBEXT(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_CONSTANT_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_CONSTANT_ALPHA, 0,
      GR_CMBX_ZERO, 0);
    CA_ENV ();
  }
  else
  {
	ac_t0_mul_prim_add_env ();
  }
}

static void ac__t1_sub_one_mul_enva_add_t0__mul_shade ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER,
    GR_COMBINE_FACTOR_LOCAL,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_TEXTURE);
  CA_PRIM ();
  if (cmb.combine_ext)
  {
    T1ACMBEXT(GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_ZERO,
      GR_CMBX_ZERO, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    T0ACMBEXT(GR_CMBX_OTHER_TEXTURE_ALPHA, GR_FUNC_MODE_X,
      GR_CMBX_TMU_CALPHA, GR_FUNC_MODE_NEGATIVE_X,
      GR_CMBX_DETAIL_FACTOR, 0,
      GR_CMBX_LOCAL_TEXTURE_ALPHA, 0);
    cmb.tex_ccolor = (cmb.tex_ccolor&0xFFFFFF00) | 0xFF ;
    percent = (rdp.env_color&0xFF) / 255.0f;
    cmb.dc0_detailmax = cmb.dc1_detailmax = percent;
    cmb.tex |= 3;
  }
  else
  {
    wxUint8 factor = (wxUint8)(rdp.env_color&0xFF);
    A_T0_INTER_T1_USING_FACTOR (factor);
  }
}

static void ac_zero_sub_prim_mul_t0_add_prim ()
{
  ACMB (GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_CONSTANT,
    GR_COMBINE_OTHER_ITERATED);
  SETSHADE_A (0);
  CA_PRIM ();
  A_USE_T0 ();
}

static void ac_one_sub_t0_mul_primshade ()
{
  ACMB (GR_COMBINE_FUNCTION_BLEND_LOCAL,
    GR_COMBINE_FACTOR_TEXTURE_ALPHA,
    GR_COMBINE_LOCAL_ITERATED,
    GR_COMBINE_OTHER_NONE);
  MULSHADE_A_PRIM ();
  A_USE_T0 ();
}

//****************************************************************
// Combine List
//
// 3/13/02: I have converted the combine descriptions, now using
//  the correct values for each slot, instead of a one-for-all-
//  slot version.  All of the descriptions marked with 'z' have
//  not yet been converted or checked.  I have not totally redone
//  the modes, because they should be for the most part correct
//  as they are, even with the wrong descriptions. [Dave2001]
//****************************************************************

typedef void (*cmb_func)();
typedef struct {
  wxUint32 key;
  cmb_func func;
} COMBINER;

static COMBINER color_cmb_list[] = {
  // { #CCSTART }
  // intro, Aidyn Chronicles. Added by Gonetz
  // (0-cmb)*env+cmb, (t1-t0)*0+t0
  {0x05083812, cc_t0},
  //terminal, Spacestation Silicon Valley. Added by Gonetz
  // (0-0)*0+cmb, (0-0)*0+prim
  {0x1fff7fff, cc_prim},
  //chip in Spacestation Silicon Valley intro. Added by Gonetz
  // (0-0)*0+cmb, (prim-0)*shade+0
  {0x1fffe4f3, cc_prim_mul_shade},
  // car, beetle adventure racing. Added by Gonetz
  // (t1-t0)*t0+t0, (cmb-shade)*prima+shade **can work incorrect**
  {0x21128a40, cc__t0_inter_t1_using_t0__sub_shade_mul_prima_add_shade},
  // Treasure opening, zelda
  // (t1-prim)*t0+t0, (prim-env)*cmb+env
  {0x2132a053, cc_prim_sub_env_mul__t1_sub_prim_mul_t0_add_t0__add_env},
  // yellow carpet, Pokemon Stadium 2
  // (t1-env)*t0+t0, (cmb-0)*shade+0
  {0x2152e4f0, cc__t1_sub_env_mul_t0_add_t0__mul_shade},
  // Water, doubut no mori
  // (t1-0)*t0+t0, (prim-0)*shade+cmb
  {0x21f204f3, cc__t1_mul_t0_add_t0__add_prim_mul_shade},
  // enemy transparent, paper mario. Addd by Gonetz
  // (t1-t0)*t1+t0, (env-prim)*cmb+prim
  {0x22126035, cc_env_sub_prim_mul__t0_inter_t1_using_t1__add_prim},
  // snowhead temple, zelda 2. Addd by Gonetz
  // (t1-t0)*t1+t0, (cmb-0)*shade+prim
  {0x221264f0, cc__t0_inter_t1_using_t1__mul_shade_add_prim},
  // snowhead temple entrance, zelda 2. Addd by Gonetz
  // (t1-t0)*t1+t0, (cmb-0)*prim+shade
  {0x221283f0, cc__t0_inter_t1_using_t1__mul_prim_add_shade},
  // teleportation, Spacestation Silicon Valley. Added by Gonetz
  // (t1-t0)*t1+t0, (prim-env)*cmb+env
  {0x2212a053, cc_prim_sub_env_mul__t0_inter_t1_using_t1__add_env},
  // pokemon fainted, Pokemon Stadium 2
  // (prim-t0)*t1+t0
  {0x22132213, cc_prim_sub_t0_mul_t1_add_t0},
  // attack, Ogre Battle 64
  // (1-t0)*t1+t0, (cmb-0)*prim+0
  {0x2216e3f0, cc__t0_inter_one_using_t1__mul_prim},
  // Some gannon spell, zelda
  // (t1-0)*t1+t0, (prim-0)*cmb+0
  {0x22f2e0f3, cc__t1_mul_t1_add_t0__mul_prim},
  // battle tanks 2 [Ogy]
  // (1-0)*t1+t0, (env-prim)*cmb+prim
  {0x22f66035, cc_env_sub_prim_mul__t0_add_t1__add_prim},
  // GASP Fighters
  // (1-0)*t1+t0, (shade-0)*cmb+0
  {0x22f6e0f4, cc__t0_add_t1__mul_shade},
  // parts of a car, F1 World Grand Prix. Added by Gonetz
  // (1-0)*t1+t0, (cmb-0)*shade+0
  {0x22f6e4f0, cc__t0_add_t1__mul_shade},
  // ???, zelda
  // (noise-0)*t1+t0, (prim-env)*cmb+env
  {0x22f7a053, cc_prim_sub_env_mul__t0_add_t1__add_env},
  // flashing arrow over buoy, wave race. Added by Gonetz
  // (t1-t0)*prim+t0, (env-cmb)*enva+cmb     ** INC **
  {0x23120c05, cc__t0_inter_t1_using_prim__inter_env_using_enva},
  // ground, zelda2. Added by Gonetz
  // (t1-t0)*prim+t0, (cmb-0)*shade+0
  {0x2312e4f0, cc__t0_inter_t1_using_prim__mul_shade},
  // wwf rules
  // (env-t0)*prim+t0
  {0x23152315, cc_t0_inter_env_using_prim},
  // Paper Mario
  // (1-t0)*prim+t0, (1-t0)*t0+cmb ** INC **
  {0x23160116, cc_t0_add_prim_mul_one_sub_t0_add_t0},
  // intro, castlevania. Added by Gonetz
  // (1-t0)*prim+t0
  {0x23162316, cc_one_sub_t0_mul_prim_add_t0},
  // Explosions, aerofighter's assault
  // (1-t0)*prim+t0, (shade-0)*cmb+0
  {0x2316e0f4, cc_t0_mul_shade},
  //beetle adventure racing. Added by Gonetz
  // (1-t0)*prim+t0, (cmb-0)*shade+0  **INC**
  {0x2316e4f0, cc__t0_inter_one_using_prim__mul_shade},
  // Unknown player background, smash bros
  // (noise-t0)*prim+t0   ** INC **
  //    {0x23172317, cc_t0},
  {0x23172317, cc_t0_inter_noise_using_prim},
  // paper mario. Added by Gonetz
  // (noise-prim)*prim+t0   ** INC **
  {0x23372337, cc_t0_add_prim},
  // strange mirror in stone temple, zelda 2. Added by Gonetz
  // (prim-env)*prim+t0, (cmb-0)*prim+0  ** INC **
  {0x2353e3f0, cc__prim_sub_env_mul_prim_add_t0__mul_prim},
  // Gilded sword, zelda 2. Added by Gonetz
  // (shade-env)*prim+t0, (cmb-0)*shade+env  ** INC **
  {0x2354a4f0, cc__t0_add_prim_mul_shade__mul_shade_add_env},
  // Razor sword, zelda 2. Added by Gonetz
  // (shade-env)*prim+t0, (cmb-0)*shade+0  ** INC **
  {0x2354e4f0, cc__t0_add_prim_mul_shade__mul_shade},
  // menu, Mischief Makers. Added by Gonetz
  // (0-env)*prim+t0, (cmb-0)*shade+0
  {0x235f235f, cc_t0_sub__prim_mul_env},
  // Deadly Arts logo. Added by Gonetz
  // (t0-0)*prim+t0
  {0x23f123f1, cc_t0_mul_prim},
  // pokemon attack, Pokemon Stadium 2. Added by Gonetz
  // (shade-0)*prim+t0, (cmb-0)*shade+0  ** INC **
  {0x23f4e4f0, cc_t0_mul_shade},
  // Mischief Makers logo. Added by Gonetz
  // (env-0)*prim+t0
  {0x23f523f5, cc_prim_mul_env_add_t0},
  // Taken out bomb, zelda
  // (1-0)*prim+t0
  {0x23f623f6, cc_t0_add_prim},
  // waterfall,  Dobutsu_no_Mori
  // (1-0)*prim+t0, (cmb-0)*shade+t0
  {0x23f624f0, cc__t0_add_prim__mul_shade_add_t0},
  // waterfall,  Dobutsu_no_Mori
  // (1-0)*prim+t0, (cmb-0)*shade+t1
  {0x23f644f0, cc__t0_add_prim__mul_shade_add_t1},
  // Jabu-Jabu's Belly, zelda
  // (noise-0)*prim+t0
  {0x23f723f7, cc_t0_add_prim},
  // carmagedon
  // (0-0)*prim+t0
  {0x23ff23ff, cc_t0},
  // water, diddy kong racing. Added by Gonetz
  // (t1-t0)*shade+t0, (env-cmb)*env_a+cmb **INC**
  {0x24120c05, cc__t0_inter_t1_using_shade__inter_env_using_enva},
  // Advertisement hoarding, Mia Soccer. Added by Gonetz
  // (t1-t0)*shade+t0, (1-0)*cmb+0
  {0x2412e0f6, cc_t0_inter_t1_using_shade},
  // ground, f-zero x
  // (prim-t0)*shade+t0 ** INC **
  {0x24132413, cc__one_sub_prim_mul_shade__mul_t0_add__prim_mul_shade},
  // intro, F1 Racing Championship. Added by Gonetz
  // (env-t0)*shade+t0  ** INC *
  {0x24152415, cc_one_sub_t0_mul_shade_add_t0},
  // Sky, pilotwings
  // (1-t0)*shade+t0
  {0x24162416, cc_one_sub_t0_mul_shade_add_t0},
  // zelda 2 [Ogy]. Added by Gonetz
  // (prim-env)*shade+t0, (prim-prim)*shade+cmb ** INC ** ?
  {0x24530433, cc_prim_sub_env_mul_shade_add_t0},
  // waves, Dr. Mario
  // (0-center)*shade+t0
  {0x246f246f, cc_t0_sub__shade_mul_center},
  // lums, Rayman2. Added by Gonetz
  // (t0-0)*shade+t0  ** INC **
  {0x24f124f1, cc_t0}, //this one works better
  //    {0x24f124f1, cc_t0_mul_shade},
  // Goemon, mystical ninja. Added by Gonetz
  // (prim-0)*shade+t0
  {0x24f324f3, cc_prim_mul_shade_add_t0},
  // Sky, waverace
  //z (t1-t0)*env+t0      ** INC **
  {0x25122512, cc_t0_inter_t1_using_env},
  // Rare logo, Jet Force. Added by Gonetz
  // (t1-t0)*env+t0, (cmb-0)*prim+0      ** INC **
  {0x2512e3f0, cc__t0_inter_t1_using_enva__mul_prim},
  // ridge recer, unimp log. Added by Gonetz
  // (t1-t0)*env+t0, (cmb-0)*shade+0      ** INC **
  {0x2512e4f0, cc__t0_inter_t1_using_env__mul_shade},
  // menu, Mischief Makers. Added by Gonetz
  //(prim-t0)*env+t0      ** INC **
  {0x25132513, cc_one_sub_env_mul_t0_add_prim_mul_env},
  // Battle border, quest64
  // (1-t0)*env+t0
  {0x25162516, cc_one_sub_env_mul_t0_add_env},
  // Paper Mario
  // (noise-t0)*env+t0
  {0x25172517, cc_t0_inter_noise_using_env},
  // the lamp in the bomb shop in town, zelda 2 [Ogy]. Added by Gonetz
  // (t0-t1)*env+t0, (1-env)*prim+cmb      ** INC **
  {0x25210356, cc_one_sub_env_mul_prim_add__t0_inter_t1_using_env},
  // Darmani's necklace, zelda 2 [Ogy]. Added by Gonetz
  // (prim-shade)*env+t0, (cmb-0)*shade+0      ** INC **
  {0x2543e4f0, cc_t0_mul_shade_add_prim_mul_env},
  //    {0x2543e4f0, cc_t0_mul_shade},
  // mystical ninja. Added by Gonetz
  // (1-0)*env+t0
  {0x25f625f6, cc_t0_add_env},
  // smoke, Starshot. Added by Gonetz
  // (1-0)*env+t0, (1-0)*cmb+0
  {0x25f6e0f6, cc_t0_add_env},
  // mega shock, Paper Mario. Added by Gonetz
  // (t1-0)*scale+t0, (env-center)*cmb+prim
  {0x26f26065, cc__t0_add__t1_mul_scale__mul_env_sub_center_add_prim},
  // character select, Duck Dodgers. Added by Gonetz
  // (prim-t0)*t0_alpha+t0, (cmb-0)*shade+0   **INC**
  {0x2813e4f0, cc__t0_inter_prim_using_t0a__mul_shade},
  // intro, Duck Dodgers. Added by Gonetz
  // (shade-t0)*t0_alpha+t0   **INC**
  {0x28142814, cc_t0_inter_shade_using_t0a},
  // vermilion gym torches, Pokemon Stadium 2.
  // (prim-env)*t0_a+t0, (cmb-cmb)*cmb+cmb
  {0x28530000, cc_prim_sub_env_mul_t0a_add_t0},
  // F1 World Grand Prix. Added by Gonetz
  // (prim-0)*t0_a+t0, (cmb-0)*shade+0   ** INC **
  {0x28f3e4f0, cc__t0a_mul_prim_add_t0__mul_shade},
  // battle tanks 2 [Ogy]
  // (env-0)*t0_a+t0, (cmb-0)*shade+0
  {0x28f5e4f0, cc__t0a_mul_env_add_t0__mul_shade},
  // blastcorps, unimp log. Added by Gonetz
  // (t1-t0)*t1_alpha+t0
  {0x29122912, cc_t0_inter_t1_using_t1a},
  // paper mario. Added by Gonetz
  // (t1-t0)*t1_alpha+t0, (cmb-env)*env_a+env
  {0x2912ac50, cc__t0_inter_t1_using_t1a__sub_env_mul_enva_add_env},
  // Rally 2000. Added by Gonetz
  // (t1-t0)*t1_alpha+t0, (cmb-0)*shade+0
  {0x2912e4f0, cc__t0_inter_t1_using_t1a__mul_shade},
  // ??? in zelda ending, zelda
  // (1-0)*t1_alpha+t0, (prim-env)*cmb+env
  {0x29f6a053, cc_prim_sub_env_mul__t0_add_t1a__add_env},
  // Sky, zelda
  //z (t1-t0)*prim_a+t0
  {0x2a122a12, cc_t0_inter_t1_using_prima},
  // battle tanks [Ogy]
  // (t1-t0)*prim_a+t0, (env-prim)*cmb+prim
  {0x2a126035, cc_env_sub_prim_mul__t0_inter_t1_using_prima__add_prim},
  // clothes, zelda 2. Added by Gonetz
  // (t1-t0)*prim_a+t0, (prim-env)*cmb+env
  {0x2a12a053, cc_prim_sub_env_mul__t0_inter_t1_using_prima__add_env},
  // N64 BIOS
  // (t1-t0)*prim_a+t0, (cmb-0)*shade+0
  {0x2a12e0f4, cc__t0_inter_t1_using_prima__mul_shade},
  // flame, Doraemon 2. Added by Gonetz
  // (t1-t0)*prim_a+t0, (cmb-0)*prim+0
  {0x2a12e3f0, cc__t0_inter_t1_using_prima__mul_prim},
  // logo, PD. Added by Gonetz
  // (t1-t0)*prim_a+t0, (cmb-0)*shade+0
  {0x2a12e4f0, cc__t0_inter_t1_using_prima__mul_shade},
  // Pikachu
  // (prim-t0)*prim_a+t0, (env-cmb)*enva+cmb
  {0x2a130c05, cc__t0_inter_prim_using_prima__inter_env_using_enva},
  // 1080 snowboarding [Ogy] - 7/03/02 fixed by Dave2001. 15 Mar 2005 fixed by Gonetz.
  // (prim-t0)*prim_a+t0
  {0x2a132a13, cc_t0_inter_prim_using_prima},
  // menu background, Paper Mario
  // (prim-t0)*prim_a+t0, (prim-t1)*prim_a+t1
  {0x2a134a23, cc_t0_inter_prim_using_prima},
  //    {0x2a134a23, cc_t0},
  // Mickey USA
  // (prim-t0)*prim_a+t0, (cmb-0)*shade+0  ** INC **
  {0x2a13e4f0, cc_t0_mul_shade},
  // gunfire, Sin and Punishmen. Added by Gonetz
  // (env-t0)*prima+t0 **INC**
  {0x2a152a15, cc_t0_inter_env_using_prima},
  // Mystical Ninja
  // (0-t0)*prima+t0, (prim-env)*cmb+env ** INC **
  {0x2a1fa053, cc_prim_sub_env_mul__t0_sub_t0_mul_prima__add_env},
  // foresight attack, Pokemon Stadium 2.
  // (t1-prim)*prim_a+t0, (prim-env)*cmb+env
  {0x2a32a053, cc_prim_sub_env_mul__t1_sub_prim_mul_prima_add_t0__add_env},
  // arena, Pokemon Stadium 2. Added by Gonetz
  // (shade-prim)*prim_a+t0  ** INC **
  {0x2a342a34, cc_t0_mul_shade},
  // Torches, Paper Mario
  // (t1-k4)*prim_a+t0, (t1-k4)*cmb_a+cmb  ** INC **
  {0x2a720772, cc_t1_sub_k4_mul_prima_add_t0},
  // GASP Fighters. Added by Gonetz
  // (t0-0)*prim_a+t0, (cmb-center)*scale+0  ** INC **
  {0x2af1e660, cc__t0_mul_prima_add_t0__sub_center_mul_scale},
  // F1 World Grand Prix. Added by Gonetz
  // (t1-0)*prim_a+t0, (cmb-0)*shade+env
  {0x2af2a4f0, cc__t1_mul_prima_add_t0__mul_shade_add_env},
  // tidal wave, Paper Mario. Added by Gonetz
  // (prim-0)*prim_a+t0
  {0x2af32af3, cc_prim_mul_prima_add_t0},
  //Spacestation Silicon Valley intro. Added by Gonetz
  // (t1-t0)*shade_alpha+t0, (prim-shade)*cmb+shade  ** INC **
  {0x2b128043, cc_prim_sub_shade_mul__t0_inter_t1_using_shadea__add_shade},
  // water, Rocket Robot in Wheels
  // (t1-t0)*shade_alpha+t0, (env-shade)*cmb+shade  ** INC **
  {0x2b128045, cc_env_sub_shade_mul__t0_inter_t1_using_shadea__add_shade},
  // arena, Pokemon Stadium 2
  // (t1-t0)*shade_alpha+t0, (cmb-prim)*env+shade  ** INC **
  {0x2b128530, cc__t0_inter_t1_using_shadea__sub_prim_mul_env_add_shade},
  // Rocket Robot in Wheels intro
  // (t1-t0)*shade_a+t0, (shade-0)*cmb+0  ** INC **
  {0x2b12e0f4, cc__t0_inter_t1_using_shadea__mul_shade},
  // water, Mickey USA
  // (t1-t0)*shade_a+t0, (cmb-0)*shade+0  ** INC **
  {0x2b12e4f0, cc__t0_inter_t1_using_shadea__mul_shade},
  // Extreme G. Added by Gonetz
  // (shade-t0)*shade_alpha+t0
  {0x2b142b14, cc_shade_sub_t0_mul_shadea_add_t0},
  // Jet Force Gemini. Added by Gonetz
  // (shade-t0)*shade_alpha+t0, (cmb-0)*prim+0  ** INC **
  {0x2b14e3f0, cc_t0_mul_prim_add_shade_mul_shadea_mul_prim},
  // V8-2
  // (env-t0)*shade_alpha+t0, (cmb-0)*shade+0  ** INC **
  {0x2b15e4f0, cc__t0_inter_env_using_shadea__mul_shade},
  // Earthquake pokemon attack, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t1-0)*shade_alpha+t0, (prim-env)*cmb+env ** INC **
  {0x2bf2a053, cc_prim_sub_env_mul__t0_add_t1__add_env},
  // pads, Pokemon Stadium 2. Added by Gonetz
  // (0-0)*shade_alpha+t0, (prim-env)*cmba+env
  {0x2bffa753, cc_prim_sub_env_mul_t0a_add_env},
  // paper mario. Added by Gonetz
  // (t1-t0)*env_a+t0, (1-cmb)*prim+cmb
  {0x2c120306, cc_one_sub__t0_inter_t1_using_enva__mul_prim_add__t0_inter_t1_using_enva},
  // Amoeba boss, water temple, zelda
  // (t1-t0)*env_a+t0, (cmb-env)*prim+t0  ** INC **
  {0x2c122350, cc__t0_inter_t1_using_enva__sub_env},
  // paper mario. Added by Gonetz
  // (t1-t0)*env_a+t0
  {0x2c122c12, cc_t0_inter_t1_using_enva},
  // paper mario. Added by Gonetz
  // (t1-t0)*env_a+t0, (1-prim)*cmb+prim
  {0x2c126036, cc_one_sub_prim_mul__t0_inter_t1_using_enva__add_prim},
  //Arena, Pokemon Stadium 2
  // (t1-t0)*env_a+t0, (cmb-0)*shade+prim
  {0x2c1264f0, cc__t0_inter_t1_using_enva__mul_shade_add_prim},
  // water, jet force. Added by Gonetz
  // (t1-t0)*env_a+t0, (prim-shade)*cmb+shade
  {0x2c128043, cc_prim_sub_shade_mul__t0_inter_t1_using_enva__add_shade},
  // Faries, zelda
  //z (t1-t0)*env_a+t0, (prim-env)*cmb+env
  {0x2c12a053, cc_prim_sub_env_mul__t0_inter_t1_using_enva__add_env},
  // paper mario. Added by Gonetz
  // (t1-t0)*env_a+t0, (prim-center)*cmb+env
  {0x2c12a063, cc_prim_sub_center_mul__t0_inter_t1_using_enva__add_env},
  // pads, Pokemon Stadium 2. Added by Gonetz
  // (t1-t0)*env_a+t0, (cmb-prim)*shade+env  ** INC **
  {0x2c12a430, cc__t0_inter_t1_using_enva__mul_shade_add_env},
  // Scary dead thing boss, zelda
  // (t1-t0)*env_a+t0, (cmb-t1)*cmb_a+env
  {0x2c12a720, cc__t0_inter_t1_using_enva__mul_env},
  // something in a menu, PokemonStadium2, [Raziel64]
  // (t1-t0)*env_a+t0, (prim-env)*cmb_a+env
  {0x2c12a753, cc_prim_sub_env_mul__t0_inter_t1_using_enva_alpha__add_env},
  // Arena, pokemon Stadium
  // (t1-t0)*env_a+t0, (cmb-shade)*prim+0
  {0x2c12e340, cc__t0_inter_t1_using_enva__sub_shade_mul_prim},
  // Water in zora's place, zelda
  // (t1-t0)*env_a+t0, (cmb-0)*prim+0
  {0x2c12e3f0, cc__t0_inter_t1_using_enva__mul_prim},
  // Ground, zelda
  //z (t1-t0)*env_a+t0, (cmb-k5)*shade+cmb_a
  {0x2c12e4f0, cc__t0_inter_t1_using_enva__mul_shade},
  // zelda, uninmp log.  Added by Gonetz
  //(t1-t0)*env_a+t0, (cmb-0)*env+0
  {0x2c12e5f0, cc__t0_inter_t1_using_enva__mul_env},
  // Spheres, waverace
  //z (env-t0)*env_a+t0
  {0x2c152c15, cc_t0_inter_env_using_enva},//cc_t0},
  // backgrounds, Mario Golf. Added by Gonetz
  // (env-t0)*env_a+t0, (shade-0)*cmb+0
  {0x2c15e0f4, cc__t0_inter_env_using_enva__mul_shade},
  // ground on Volcano level, DKR, [Raziel64]
  // (env-t0)*env_a+t0, (cmb-0)*shade+0
  {0x2c15e4f0, cc__t0_inter_env_using_enva__mul_shade},
  // Nintendo 'N', zelda
  //z (t0-prim)*env_a+t0, (prim-env)*cmb+env
  {0x2c31a053, cc_prim_sub_env_mul__t0_sub_prim_mul_enva_add_t0__add_env},
  // Nintendo title & saria's song, zelda
  //z (t1-prim)*env_a+t0, (prim-env)*cmb+env
  {0x2c32a053, cc_prim_sub_env_mul__t1_sub_prim_mul_enva_add_t0__add_env},
  // Hover boots flying, zelda
  // (t1-prim)*env_a+t0, (prim-0)*cmb+env
  {0x2c32a0f3, cc__t1_sub_prim_mul_enva_add_t0__mul_prim_add_env},
  // star beam, paper mario
  // (prim-env)*env_a+t0
  {0x2c532c53, cc_prim_sub_env_mul_enva_add_t0},
  // Kotake & koume's hair, zelda
  // (t1-0)*env_a+t0, (prim-env)*cmb+env
  {0x2cf2a053, cc_prim_sub_env_mul__t1_mul_enva_add_t0__add_env},
  //Goldeneye, [Jeremy]. Added by Gonetz
  // (t0-t0)*lodf+t0, (cmb-0)*prim+0
  {0x2d11e3f0, cc_t0_mul_prim},
  // Pilot wings
  // (t1-t0)*lodf+t0, (one-cmb)*prim+cmb
  {0x2d120306, cc_one_sub_prim_mul__t0_inter_t1_using_primlod__add_prim},
  // Pilot wings
  // (t1-t0)*lodf+t0, (one-cmb)*shade+cmb
  {0x2d120406, cc_one_sub_shade_mul__t0_inter_t1_using_primlod__add_shade},
  // Indy Racing 2000. Added by Gonetz
  // (t1-t0)*lodf+t0, (env-cmb)*prima+cmb  ** INC **
  {0x2d120a05, cc_t0_inter_t1_using_primlod},
  // (t1-t0)*lodf+t0
  {0x2d122d12, cc_t0_inter_t1_using_primlod},
  //broken wall, beetle adventure racing. Added by Gonetz
  // (t1-t0)*lodf+t0, (shade-prim)*cmb+prim
  {0x2d126034, cc_shade_sub_prim_mul__t0_inter_t1_using_primlod__add_prim},
  //Intro, CBFD. Added by Gonetz
  // (t1-t0)*lodf+t0, (shade-env)*cmb+prim
  //    {0x2d126054, cc_shade_sub_env_mul_t0_add_prim},
  {0x2d126054, cc_shade_sub_env_mul__t0_inter_t1_using_primlod__add_prim},
  // bassmasters 2000 [Ogy]
  // (t1-t0)*lodf+t0, (env-0)*cmb+prim  ** INC **
  {0x2d1260f5, cc_t0_mul_env_add_prim},
  // sign, CBFD. Added by Gonetz
  // (t1-t0)*lodf+t0, (cmb-env)*shade+prim ** INC **
  {0x2d126450, cc__t0_inter_t1_using_primlod__sub_env_mul_shade_add_prim},
  //    {0x2d126450, cc_t0_sub_env_mul_shade_add_prim},
  // landscape, Cruis'n Exotica. Added by Gonetz
  // (t1-t0)*lodf+t0, (cmb-0)*shade+prim
  {0x2d1264f0, cc__t0_inter_t1_using_primlod__mul_shade_add_prim},
  // blast corps [Ogy]
  // (t1-t0)*lodf+t0, (0-0)*0+shade
  {0x2d129fff, cc__t0_inter_t1_using_primlod__mul_shade},
  // End of level, zelda
  // (t1-t0)*lodf+t0, (prim-env)*cmb+env
  {0x2d12a053, cc_prim_sub_env_mul__t0_inter_t1_using_primlod__add_env},
  // Rocket Robot in Wheels intro
  // (t1-t0)*lodf+t0, (shade-env)*cmb+env
  {0x2d12a054, cc_shade_sub_env_mul__t0_inter_t1_using_primlod__add_env},
  // basket, Fox Sport
  // (t1-t0)*lodf+t0, (prim-env)*t0+env
  {0x2d12a153, cc_prim_sub_env_mul__t0_inter_t1_using_primlod__add_env},
  // paper mario. Added by Gonetz
  // (t1-t0)*lodf+t0, (cmb-0)*prim+env    ** INC **
  {0x2d12a3f0, cc__t0_inter_t1_using_primlod__mul_prim_add_env},
  // Tony Hawk Pro Skater
  // (t1-t0)*lodf+t0, (cmb-0)*shade+env
  {0x2d12a4f0, cc__t0_inter_t1_using_primlod__mul_shade_add_env},
  // part of a building, Spiderman. Added by Gonetz
  // (t1-t0)*lodf+t0, (cmb-env)*cmba+env    ** INC **
  {0x2d12a750, cc_t0_inter_t1_using_primlod},
  // Mike Piazza's Strike Zone
  // (t1-t0)*lodf+t0, (shade-prim)*cmb+0
  {0x2d12e034, cc_shade_sub_prim_mul__t0_inter_t1_using_primlod},
  // intro, F1 Racing Championship. Added by Gonetz
  // (t1-t0)*lodf+t0, (shade-env)*cmb+0
  {0x2d12e054, cc_shade_sub_env_mul__t0_inter_t1_using_primlod},
  // stands, F1 Racing Championship. Added by Gonetz
  // (t1-t0)*lodf+t0, (1-env)*cmb+0
  {0x2d12e056, cc_one_sub_env_mul__t0_inter_t1_using_primlod},
  // court, Mario Tennis. Added by Gonetz
  // (t1-t0)*lodf+t0, (prim-0)*cmb+0
  {0x2d12e0f3, cc__t0_inter_t1_using_primlod__mul_prim},
  // Rocket Robot in Wheels intro
  // (t1-t0)*lodf+t0, (shade-0)*cmb+0
  {0x2d12e0f4, cc__t0_inter_t1_using_primlod__mul_shade},
  // Pilot wings
  // (t1-t0)*lodf+t0, (cmb-0)*t0+0 ** INC **
  {0x2d12e1f0, cc_t0_inter_t1_using_primlod},
  // cars wheels, SF Rush 2049. Added by Gonetz
  // (t1-t0)*lodf+t0, (cmb-0)*prim+0
  {0x2d12e3f0, cc__t0_inter_t1_using_primlod__mul_prim},
  // Bridge, sf rush
  // (t1-t0)*lodf+t0, (cmb-0)*shade+0
  //    {0x2d12e4f0, cc_t0_mul_shade},
  {0x2d12e4f0, cc__t0_inter_t1_using_primlod__mul_shade},
  // blast corps [Ogy]
  // (t1-t0)*lodf+t0, (t0-0)*shade+0
  {0x2d12e4f1, cc_t0_mul_shade},
  // field, Mike Piazza's Strike Zone
  // (t1-t0)*lodf+t0, (cmb-prim)*env+0   ** INC **
  {0x2d12e530, cc__t0_inter_t1_using_primlod__mul_env},
  // radar, Perfect Dark
  // (t1-t0)*lodf+t0, (cmb-0)*env+0
  {0x2d12e5f0, cc__t0_inter_t1_using_primlod__mul_env},
  // planet, Blast Corps
  // (t1-t0)*lodf+t0, (cmb-0)*prima+0
  {0x2d12eaf0, cc__t0_inter_t1_using_primlod__mul_prima},
  // zelda 2. Added by Gonetz
  // (t0-t0)*primlod+t0, (prim-env)*cmb+env
  {0x2e11a053, cc_prim_sub_env_mul_t0_add_env},
  // zelda 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (0-0)*shade+cmb
  {0x2e1204ff, cc_t0_inter_t1_using_primlod},
  // zelda 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (env-prim)*primlod+cmb
  {0x2e120d35, cc_prim_sub_env_mul_primlod_add__t0_inter_t1_using_primlod},
  // lamppost, Ridge Racer. Added by Gonetz
  // (t1-t0)*primlod+t0
  {0x2e122e12, cc_t0_inter_t1_using_primlod},
  // Hearts, zelda
  //z (t1-t0)*primlod+t0, (shade-prim)*cmb+prim
  {0x2e126034, cc_shade_sub_prim_mul__t0_inter_t1_using_primlod__add_prim},
  // Sunny Day, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t1-t0)*primlod+t0, (env-prim)*cmb+prim
  {0x2e126035, cc_env_sub_prim_mul__t0_inter_t1_using_primlod__add_prim},
  // snowhead temple, zelda 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (cmb-env)*shade+prim  ** INC **
  {0x2e126450, cc__t0_inter_t1_using_primlod__mul_shade_add_prim},
  // snow on a wall, snowhead temple, zelda 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (cmb-0)*shade+prim
  {0x2e1264f0, cc__t0_inter_t1_using_primlod__mul_shade_add_prim},
  // Morning Sun, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t1-t0)*primlod+t0, (cmb-0)*0+prim
  {0x2e127ff0, cc_prim},
  // arena, Pokemon Stadium 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (cmb-prim)*shade+shade  ** INC **
  {0x2e128430, cc__t0_inter_t1_using_primlod__mul_shade},
  // Pokemon Stadium 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (cmb-env)*cmb+env  ** INC **
  {0x2e12a050, cc_t0_inter_t1_using_primlod},
  // End of level heart, zelda
  // (t1-t0)*primlod+t0, (prim-env)*cmb+env
  {0x2e12a053, cc_prim_sub_env_mul__t0_inter_t1_using_primlod__add_env},
  // Huge turtle appearance, zelda 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (1-env)*cmb+env
  {0x2e12a056, cc_one_sub_env_mul__t0_inter_t1_using_primlod__add_env},
  // frozen octorok, zelda 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (prim-env)*t1+env
  {0x2e12a253, cc_prim_sub_env_mul_t1_add_env},
  // fall headwaters, zelda 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (cmb-env)*shade+env  ** INC **
  {0x2e12a450, cc__t0_inter_t1_using_primlod__sub_env_mul_shade_add_env},
  // Fissure attack, pokemon stadium 2
  // (t1-t0)*primlod+t0, (prim-env)*cmb_a+env
  {0x2e12a753, cc_prim_sub_env_mul__t0_inter_t1_using_primlod__add_env},
  // zelda 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (cmb-0)*t1+0  ** INC ** ?
  {0x2e12e2f0, cc_t0_inter_t1_using_primlod},
  // zelda 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (cmb-0)*prim+0
  {0x2e12e3f0, cc__t0_inter_t1_using_primlod__mul_prim},
  // sky, PGA European Tour
  // (t1-t0)*primlod+t0, (cmb-env)*shade+0  ** INC **
  {0x2e12e450, cc__t0_inter_t1_using_primlod__mul_shade},
  // Kirby's pool, smash bros
  // (t1-t0)*primlod+t0, (cmb-0)*shade+0
  {0x2e12e4f0, cc__t0_inter_t1_using_primlod__mul_shade},
  //Spacestation Silicon Valley intro. Added by Gonetz
  // (prim-t0)*primlod+t0, (cmb-0)*shade+0  **INC**
  {0x2e132e13, cc_t0_inter_prim_using_primlod},
  // explosions, daikatana. Added by Gonetz
  // (prim-t0)*primlod+t0, (cmb-0)*shade+0  **INC**
  {0x2e13e4f0, cc_t0_mul_shade},
  //Mike Piazza's Strike Zone logo. Added by Gonetz
  // (shade-t0)*primlod+t0
  {0x2e142e14, cc_t0_inter_shade_using_primlod},
  // Cartridge color (transfer pak}, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (1-t0)*primlod+t0
  {0x2e162e16, cc_one_sub_t0_mul_primlod_add_t0},
  // pokemon attack, Pokemon Stadium 2. Added by Gonetz
  // (1-t0)*primlod+t0, (prim-0)*cmb+0
  {0x2e16e0f3, cc__t0_inter_one_using_primlod__mul_prim},
  // Spider Web attack, Pokemon Stadium 2.
  // (1-t0)*primlod+t0, (cmb-0)*prim+0
  {0x2e16e3f0, cc__t0_inter_one_using_primlod__mul_prim},
  // pokemon attack, Pokemon Stadium 2. Added by Gonetz
  // (1-t0)*primlod+t0, (cmb-0)*shade+0
  {0x2e16e4f0, cc__t0_inter_one_using_primlod__mul_shade},
  // zelda 2. Added by Gonetz
  // (t1-t1)*primlod+t0, (prim-env)*cmb+env
  {0x2e22a053, cc_prim_sub_env_mul_t0_add_env},
  // Shadow Ball, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (0-t1)*primlod+t0, (prim-env)*cmb+env  ** INC **
  {0x2e2fa053, cc_prim_sub_env_mul_t0_add_env},
  // Skulltula coin solid, zelda
  // (t0-prim)*primlod+t0, (prim-env)*cmb+env
  {0x2e31a053, cc_prim_sub_env_mul__t0_sub_prim_mul_primlod_add_t0__add_env},
  // Triforce lines, zelda
  // (t1-prim)*primlod+t0, (prim-shade)*cmb+shade
  {0x2e328043, cc_prim_sub_shade_mul__t1_sub_prim_mul_primlod_add_t0__add_shade},
  // moon when majora defeated, zelda 2. Added by Gonetz
  // (t1-prim)*primlod+t0, (1-shade)*cmb+shade
  {0x2e328046, cc_one_sub_shade_mul__t1_sub_prim_mul_primlod_add_t0__add_shade},
  // Fire, zelda
  //z (t1-prim)*primlod+t0, (prim-env)*cmb+env    ** INC **
  {0x2e32a053, cc_prim_sub_env_mul__t1_sub_prim_mul_primlod_add_t0__add_env},
  // zelda 2 [Ogy]. Added by Gonetz
  // (t1-prim)*primlod+t0, (shade-env)*cmb+env
  {0x2e32a054, cc_shade_sub_env_mul__t1_sub_prim_mul_primlod_add_t0__add_env},
  // Scary face, pokemon stadium 2
  // (t1-prim)*primlod+t0, (1-env)*cmb+env
  {0x2e32a056, cc_one_sub_env_mul__t1_sub_prim_mul_primlod_add_t0__add_env},
  // zelda 2. Added by Gonetz
  // (t1-prim)*primlod+t0, (prim-0)*cmb+env
  {0x2e32a0f3, cc__t1_sub_prim_mul_primlod_add_t0__mul_prim_add_env},
  // zelda 2. Added by Gonetz
  // (t1-0)*primlod+t0, (prim-env)*cmb+env
  {0x2ef2a053, cc_prim_sub_env_mul__t1_mul_primlod_add_t0__add_env},
  // zelda 2. Added by Gonetz
  // (t1-0)*primlod+t0, (cmb-0)*prim+0
  {0x2ef2e3f0, cc__t1_mul_primlod_add_t0__mul_prim},
  // zelda 2. Added by Gonetz
  // (t1-0)*primlod+t0, (cmb-0)*env+0
  {0x2ef2e5f0, cc__t1_mul_primlod_add_t0__mul_env},
  // gun, Doom64. Added by Gonetz
  // (1-0)*primlod+t0, (cmb-0)*prim+env
  {0x2ef6a3f0, cc__t0_add_primlod__mul_prim_add_env},
  // walls, Doom64. Added by Gonetz
  // (1-0)*primlod+t0, (cmb-0)*shade+env
  {0x2ef6a4f0, cc__t0_add_primlod__mul_shade_add_env},
  // Pokemon Stadium 2. Added by Gonetz
  // (noise-0)*primlod+t0, (prim-env)*cmb+env  ** INC **
  {0x2ef7a053, cc_prim_sub_env_mul_t0_add_env},
  // Tony Hawk's Pro Skater. Added by Gonetz
  // (t1-t0)*k5+t0
  {0x2f122f12, cc_t0_inter_t1_using_k5},
  // F1 World Grand Prix. Added by Gonetz
  // (t1-t0)*k5+t0, (cmb-0)*shade+0    **INC**
  {0x2f12e4f0, cc__t0_inter_t1_using_k5__mul_shade},
  // Turok 3 [scorpiove]. Added by Gonetz
  // (t0-k4)*k5+t0
  {0x2f712f71, cc_t0},
  // THPS 3
  // (env-0)*k5+t0,
  {0x2ff52ff5, cc_t0_add_env_mul_k5},
  // super bowling
  // (0-0)*k5+t0,
  {0x2fff0000, cc_t0},
  // super bowling
  // (0-0)*k5+t0
  {0x2fff2fff, cc_t0},
  // Moonlight attack, pokemon stadium 2
  // (t1-t0)*0+t0, (prim-env)*cmb+env
  {0x3f12a053, cc_prim_sub_env_mul_t0_add_env},
  //C&C shadows
  //(1-env)*0+t0
  {0x3f563f56, cc_t0},
  // RARE logo, blast corps. Added by Gonetz
  // (t0-0)*0+t0
  {0x3ff13ff1, cc_t0},
  // the ground below the scarecrow in the trading post in town, zelda 2 [Ogy]. Added by Gonetz
  // (t1-0)*0+t0, (cmb-0)*shade+0
  {0x3ff2e4f0, cc_t0_mul_shade},
  // intro, background, Dezaemon 3D
  // (1-0)*0+t0
  {0x3ff63ff6, cc_t0},
  // intro of WWF WrestleMania 2000
  // ((0-0)*0+t0, (env-cmb)*prim+cmb
  {0x3fff0305, cc_env_sub_t0_mul_prim_add_t0},
  // pistol fire, Turok
  // ((0-0)*0+t0, (env-cmb)*shade+cmb
  {0x3fff0405, cc_env_sub_t0_mul_shade_add_t0},
  // Tony Hawk's Pro Skater. Added by Gonetz
  // ((0-0)*0+t0, (t1-0)*shade+cmb ** INC **
  {0x3fff04f2, cc_t0},
  // Dr. Mario [Ogy]. Added by Gonetz
  // ((0-0)*0+t0, (prim-cmb)*env+cmb
  {0x3fff0503, cc_prim_sub_t0_mul_env_add_t0},
  // Stained glass, quest64
  // (0-0)*0+t0, (1-0)*env+cmb
  {0x3fff05f6, cc_t0_add_env},
  // Health bar, killer instinct gold
  // (0-0)*0+t0, (prim-env)*prim_a+cmb
  {0x3fff0a53, cc_prim_sub_env_mul_prima_add_t0},
  // Runes, Turok - Dinosaur Hunter. Added by Gonetz
  // (0-0)*0+t0, (env-cmb)*env_a+cmb
  {0x3fff0c05, cc_t0_inter_env_using_enva},
  // intro, Mission Impossible. Added by Gonetz
  // (k5-k5)*0+t0, (0-0)*scale+t0
  {0x3fff26ff, cc_t0},
  // V8-2
  // (0-0)*0+t0, (t0-k4)*k5+t0
  {0x3fff2f71, cc_t0_sub_k4_mul_k5_add_t0},
  // TM, mario
  //z (k5-k5)*0+t0
  {0x3fff3fff, cc_t0},
  // Intro, CBFD. Added by Gonetz
  // ((0-0)*0+t0, (shade-env)*cmb+prim
  {0x3fff6054, cc_shade_sub_env_mul_t0_add_prim},
  // Text, Mia Soccer. Added by Gonetz
  // ((0-0)*0+t0, (0-0)*0+prim
  {0x3fff7fff, cc_t0},
  // paper mario. Added by Gonetz
  // ((0-0)*0+t0, (prim-env)*cmb+env
  {0x3fffa053, cc_prim_sub_env_mul_t0_add_env},
  // Objects in arena, pokemon stadium 2
  // (0-0)*0+t0, (cmb-prim)*shade+env
  {0x3fffa430, cc_t0_mul_prim},
  // intro, F1 Racing Championship. Added by Gonetz
  // (0-0)*0+t0, (shade-env)*cmb+0
  {0x3fffe054, cc_shade_sub_env_mul_t0},
  // stands, F1 Racing Championship. Added by Gonetz
  // (0-0)*0+t0, (1-env)*cmb+0
  {0x3fffe056, cc_one_sub_env_mul_t0},
  // ? (from log)
  // (0-0)*0+t0, (prim-0)*cmb+0
  {0x3fffe0f3, cc_t0_mul_prim},
  // background, GASP Fighters
  // (0-0)*0+t0, (shade-0)*cmb+0
  {0x3fffe0f4, cc_t0_mul_shade},
  // zelda 2 [Ogy]. Added by Gonetz
  // (0-0)*0+t0, (env-0)*cmb+0
  {0x3fffe0f5, cc_t0_mul_env},
  // logo, v-rally 99
  // (0-0)*0+t0, (prim-0)*t0+0
  {0x3fffe1f3, cc_t0_mul_prim},
  // target hit, zelda 2. Added by Gonetz
  // (0-0)*0+t0, (cmb-0)*prim+0
  {0x3fffe3f0, cc_t0_mul_prim},
  // Ms. Pac-Man intro background. Added by Gonetz
  // (0-0)*0+t0, (cmb-0)*shade+0
  {0x3fffe4f0, cc_t0_mul_shade},
  //  Wonder Project J2 logo. Added by Gonetz
  // (0-0)*0+t0, (t0-0)*shade+0
  {0x3fffe4f1, cc_t0_mul_shade},
  // tire trace, Monster truck madness. Added by Gonetz
  // (0-0)*0+t0, (cmb-0)*env+0
  {0x3fffe5f0, cc_t0_mul_env},
  // Gauntlet Legends intro. Added by Gonetz
  // (0-0)*0+t0, (cmb-0)*ecale+0
  {0x3fffe6f0, cc_t0},
  // tire trace, beetle adventure racing. Added by Gonetz
  // (t1-t0)*t0+t1, (cmb-t0)*shade+t1   **INC**
  {0x41124410, cc__t0_inter_t1_using_t0__mul_shade},
  // Paper Mario. Added by Gonetz
  // (t0-t1)*t0+t1   **INC**
  {0x41214121, cc_t1_inter_t0_using_t0},
  // Powered Star Beam, Paper Mario. Added by Gonetz
  // (t0-t1)*t0+t1, (env-prim)*cmb+prim   **INC**
  {0x41216035, cc_env_sub_prim_mul__t1_inter_t0_using_t0__add_prim},
  // wetrix raiseland [Raziel64]. Added by Gonetz
  // (prim-t1)*t0+t1, (env-t0)*cmb+cmb   **INC**
  {0x41230015, cc_env_sub_prim_mul__t0_mul_t1__add_prim},
  // SCARS. Added by Gonetz
  // (t1-t0)*t0+t1, (cmb-t0)*shade+t1   **INC**
  {0x41250b03, cc__t0_inter_t1_using_half__mul_shade},
  //beetle adventure racing. Added by Gonetz
  //(t0-t1)*t1+t1, (cmb-0)*shade+0  **INC**
  {0x4221e4f0, cc__t1_inter_t0_using_t1__mul_shade},
  // cianwood gym walls, pokemon stadium 2
  //(t0-prim)*t1+t1, (cmb-0)*env+shade
  {0x423185f0, cc__t0_sub_prim_mul_t1_add_t1__mul_env_add_shade},
  // cianwood gym walls, pokemon stadium 2
  //(t0-prim)*t1+t1, (cmb-0)*shade+0
  {0x4231e4f0, cc__t0_sub_prim_mul_t1_add_t1__mul_shade},
  // paper mario. Added by Gonetz
  // (t0-t0)*prim+t1, (t1-cmb)*cmb+env  **INC** weird
  {0x4311a002, cc_env},
  // background, Wetrix level 1, [Raziel64]. Added by Gonetz
  // (t0-t1)*prim+t1
  {0x43214321, cc_t1_inter_t0_using_prim},
  // Mario Party3 Tidal Toss
  // (t0-t1)*prim+t1, (cmb-0)*shade+0  **INC**
  {0x4321e4f0, cc__t1_inter_t0_using_prim__mul_shade},
  // grass, ISS 2k. Added by Gonetz
  // (t0-t1)*prim+t1, (cmb-0)*env+0  **INC**
  {0x4321e5f0, cc__t1_inter_t0_using_prim__mul_env},
  // intro, Paper Mario
  // (t0-0)*prim+t1
  {0x43f143f1, cc_t0_mul_prim_add_t1},
  // F1 World Grand Prix. Added by Gonetz
  // (t0-0)*prim+t1, (cmb-0)*shade+env  **INC**
  {0x43f1a4f0, cc__t0_add_t1__mul_shade_add_env},
  // field, ISS64. Added by Gonetz
  // (t0-t1)*shade+t1, (cmb-t1)*prim+t1  ** INC **
  {0x44214320, cc_t0_sub_t1_mul_prim_mul_shade_add_t1},
  //    {0x44214320, cc__t0_add_t1__mul_prim},
  // field, Top gear hyper-bike
  // (t0-t1)*shade+t1
  {0x44214421, cc_t1_inter_t0_using_shade},
  // water, goemon great adventure
  // (t0-t1)*env+t1 ** INC **
  {0x45214521, cc_t1_inter_t0_using_env},
  // characters, Ogre Battle. Added by Gonetz
  // (1-t1)*env+t1, (1-cmb)*prim+cmb  ** INC **
  {0x45260306, cc_one_sub_t1_mul_prim_add_t1},
  // characters, Ogre Battle. Added by Gonetz
  // (1-t1)*env+t1
  {0x45264526, cc_one_sub_t1_mul_env_add_t1},
  // characters, Ogre Battle. Added by Gonetz
  // (1-t1)*env+t1, (cmb-0)*prim+0  ** INC **
  {0x4526e3f0, cc__t1_inter_one_using_env__mul_prim},
  // explosion, body harvest. Added by Gonetz
  // (t0-t1)*scale+t1, (env-prim)*cmb+prim  ** INC **
  {0x46216035, cc_env_sub_prim_mul__t0_inter_t1_using_half__add_prim},
  // Water, AeroGauge. Added by Gonetz
  // (t0-t1)*prima+t1, (0-0)*0+cmb
  {0x4a214a21, cc_t1_inter_t0_using_prima},
  // flame, castlevania 2. Added by Gonetz
  // (t0-t1)*prima+t1, (prim-env)*cmb+env
  {0x4a21a053, cc_prim_sub_env_mul__t1_inter_t0_using_prima__add_env},
  // shadows, Mario Tennis. Added by Gonetz
  // (t0-t1)*prima+t1, (prim-0)*cmb+0
  {0x4a21e0f3, cc__t1_inter_t0_using_prima__mul_prim},
  // menu, Mario Golf. Added by Gonetz
  // (t0-t1)*prima+t1, (shade-0)*cmb+0
  {0x4a21e0f4, cc__t1_inter_t0_using_prima__mul_shade},
  // intro, castlevania 2. Added by Gonetz
  // (t0-t1)*prima+t1, (cmb-0)*prim+0
  {0x4a21e3f0, cc__t1_inter_t0_using_prima__mul_prim},
  // water on map, Ogre Battle64. Added by Gonetz
  // (t0-t1)*prima+t1, (cmb-0)*shade+0
  {0x4a21e4f0, cc__t1_inter_t0_using_prima__mul_shade},
  // Ice, Paper Mario
  // (t0-t1)*shade_a+t1
  {0x4b214b21, cc_t1_inter_t0_using_shadea},
  // Grass, Beetle Adventure Racing
  // (t0-t1)*shade_a+t1, (cmb-0)*shade+0
  {0x4b21e4f0, cc__t1_inter_t0_using_shadea__mul_shade},
  // Ground at kotake & koume, zelda
  // (t1-t0)*env_a+t0, (prim-env)*cmb+env
  {0x4c12a053, cc_prim_sub_env_mul__t0_inter_t1_using_enva__add_env},
  // Tony Hawk's Pro Skater. Added by Gonetz
  // (t0-t1)*env_a+t1, (cmb-0)*shade+cmb  ** INC **
  {0x4c2104f0, cc__t1_inter_t0_using_enva__mul_shade},
  // bikes, xg2. Added by Gonetz
  // (t0-t1)*env_a+t1, (cmb-prim)*prima+prim
  {0x4c216a30, cc__t1_inter_t0_using_enva__sub_prim_mul_prima_add_prim},
  // Yoshi Story
  // (t0-t1)*env_a+t1, (prim-env)*cmb+env
  {0x4c21a053, cc_prim_sub_env_mul__t1_inter_t0_using_enva__add_env},
  // arena, Pokemon Stadium 1. Added by Gonetz
  // (t0-t1)*env_a+t1, (cmb-0)*prim+0
  {0x4c21e3f0, cc__t1_inter_t0_using_enva__mul_prim},
  // "end of chapter" text, paper mario. Added by Gonetz
  // (1-t1)*env_a+t1, (cmb-0)*t1+0
  {0x4c26e2f0, cc__t1_inter_one_using_enva__mul_t0},
  // Zelda opening door, zelda
  // (t0-prim)*env_a+t1, (prim-env)*t0+env
  {0x4c31a053, cc_prim_sub_env_mul_t0_add_env},
  // arena, Pokemon Stadium 2
  // (t0-0)*env_a+t1, (cmb-0)*shade+prim
  {0x4cf164f0, cc__t0_mul_enva_add_t1__mul_shade_add_prim},
  // Kotake & koume magic poof, zelda
  // (t0-0)*env_a+t1, (prim-env)*cmb+env
  {0x4cf1a053, cc_prim_sub_env_mul__t0_mul_enva_add_t1__add_env},
  // ground in stone temple, zelda 2. Added by Gonetz
  // (t1-t0)*primlod+t1, (cmb-0)*prim+0
  {0x4e12e3f0, cc__t0_inter_t1_using_primlod__mul_prim},
  // pokemon attack, Pokemon Stadium 2. Added by Gonetz
  // (noise-t0)*primlod+t1, (prim-env)*cmb+env  ** INC **
  {0x4e17a053, cc_prim_sub_env_mul__t0_inter_t1_using_primlod__add_env},
  // menu, pokemon stadium 1, [Raziel64]
  // (t0-t1)*lodf+t1, (prim-env)*cmb+env
  {0x4e214e21, cc_t1_inter_t0_using_primlod},
  // Pokemon backgrounds, pokemon stadium 2
  // (t0-t1)*primlod+t1, (cmb-0)*shade+prim
  {0x4e2164f0, cc__t1_inter_t0_using_primlod__mul_shade_add_prim},
  // Pokemon backgrounds, pokemon stadium 2
  // (t0-t1)*lodf+t1, (prim-env)*cmb+env
  {0x4e21a053, cc_prim_sub_env_mul__t1_inter_t0_using_primlod__add_env},
  // zelda 2 [Ogy]. Added by Gonetz
  // (t0-t1)*primlod+t1, (t1-cmb)*prim+env     ** INC **
  {0x4e21a302, cc_env_sub__t0_sub_t1_mul_primlod__mul_prim},
  // Magnitude, pokemon stadium 2
  // (t0-t1)*primlod+t1, (prim-env)*cmb_a+env
  {0x4e21a753, cc_prim_sub_env_mul__t1_inter_t0_using_primlod__add_env},
  // Arena, pokemon stadium 2
  // (t0-t1)*primlod+t1, (cmb-shade)*prim+0
  {0x4e21e340, cc__t1_inter_t0_using_primlod__sub_shade_mul_prim},
  // zelda 2 [Ogy]. Added by Gonetz
  // (t0-t1)*primlod+t1, (cmb-0)*shade+0
  {0x4e21e4f0, cc__t1_inter_t0_using_primlod__mul_shade},
  // lava in snowhead temple, zelda 2. Added by Gonetz
  // (t0-prim)*primlod+t1, (cmb-prim)*shade+cmb     ** INC **
  {0x4e310430, cc_lavatex_sub_prim_mul_shade_add_lavatex},
  // Skulltula coin, zelda
  // (t0-prim)*primlod+t1, (prim-env)*cmb+env
  {0x4e31a053, cc_prim_sub_env_mul__t0_sub_prim_mul_primlod_add_t1__add_env},
  // Pokemon background, pokemon stadium 2
  // (noise-shade)*primlod+t1, (prim-env)*cmb+env
  {0x4e47a053, cc_prim_sub_env_mul_t1_add_env},
  // Reflect, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t0-0)*primlod+t1, (prim-env)*cmb+env
  {0x4ef1a053, cc_prim_sub_env_mul__t0_add_t1__add_env},
  //beetle adventure racing. Added by Gonetz
  //(t0-t1)*k5+t1, (cmb-0)*shade+0
  {0x4f21e4f0, cc__t1_inter_t0_using_k5__mul_shade},
  // Spiderman. Added by Gonetz
  //(t0-t1)*k5+t1, (cmb-0)*env+0
  {0x4f21e5f0, cc_t1_mul_env},
  // N64 logo, Ogre Battle. Added by Gonetz
  //(0-0)*0+t1
  {0x5fff5fff, cc_t1},
  // reversing light, Monster truck madness. Added by Gonetz
  //(0-0)*0+t0, (0-0)*0+prim
  {0x5fff7fff, cc_prim},
  // battle tanks [Ogy]
  // (0-0)*0+t1, (env-shade)*cmb+shade
  {0x5fff8045, cc_env_sub_shade_mul_t1_add_shade},
  // minigame, pokemon stadium 1. Added by Gonetz
  // (0-0)*0+t1, (prim-env)*cmb+env
  {0x5fffa053, cc_prim_sub_env_mul_t1_add_env},
  // F1 World Grand Prix. Added by Gonetz
  // (t0-prim)*t0+prim, (cmb-0)*shade
  {0x6131e4f0, cc__prim_inter_t0_using_t0__mul_shade},
  // aerofighter's assault [Ogy]
  // (shade-prim)*t0+prim
  {0x61346134, cc_shade_sub_prim_mul_t0_add_prim},
  // pilot wings
  // (shade-prim)*t0+prim, (cmb-shade)*shadea+shade
  {0x61348b40, cc_shade_inter__prim_inter_shade_using_t0__using_shadea},
  // club blow, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (shade-prim)*t0+prim, (cmb-env)*cmb_a+env ** INC **
  {0x6134a750, cc_shade_sub_prim_mul_t0_add_prim},
  // sky, Killer Instinct
  // (shade-prim)*t0+prim, (cmb-env)*shade_a+env ** INC **
  {0x6134ab50, cc_env_inter__prim_inter_shade_using_t0__using_shadea},
  // lava, beetle adventure racing
  // (shade-prim)*t0+prim, (cmb-0)*t1+0 ** INC **
  {0x6134e2f0, cc_shade_sub_prim_mul_t0_add_prim},
  // Monster truck madness intro. Added by Gonetz
  // (env-prim)*t0+prim, (cmb-0)*scale+cmb ** INC **
  {0x613506f0, cc_env_sub_prim_mul_t0_add_prim},
  // pokemon attack, Pokemon stadium 1
  // (env-prim)*t0+prim, (cmb-0)*0+cmb
  {0x61351ff0, cc_env_sub_prim_mul_t0_add_prim},
  // Paper Mario, fortune teller spheres
  // (env-prim)*t0+prim, (cmb-0)*t1+t0  ** INC **
  {0x613522f0, cc_t0_mul_t1_add_t0},
  // Later hearts, zelda
  // (env-prim)*t0+prim
  {0x61356135, cc_env_sub_prim_mul_t0_add_prim},
  // Mission Impossible. Added by Gonetz
  // (env-prim)*t0+prim, (shade-0)*cmb+0  ** INC **
  {0x6135e0f4, cc__prim_inter_env_using_t0__mul_shade},
  // crashing other vehicle, Monster truck madness [Raziel64]. Added by Gonetz
  // (env-prim)*t0+prim, (cmb-0)*t0+0  ** INC **
  {0x6135e1f0, cc_env_sub_prim_mul_t0_add_prim},
  // Tony Hawk's Pro Skater. Added by Gonetz
  // (env-prim)*t0+prim, (cmb-0)*t1+0  ** INC **
  {0x6135e2f0, cc_env_sub_prim_mul_t0_add_prim},
  // aerofighter's assault [Ogy]
  // (env-prim)*t0+prim, (cmb-0)*shade+0  ** INC **
  {0x6135e4f0, cc__prim_inter_env_using_t0__mul_shade},
  // "time out", paper mario. Added by Gonetz
  // (1-prim)*t0+prim, (1-cmb)*enva+cmb  ** INC **
  {0x61360c06, cc_one_sub_prim_mul_t0_add_prim},
  // intro, paper mario. Added by Gonetz
  // (1-prim)*t0+prim, (cmb-0)*prima+t0  ** INC **
  {0x61362af0, cc__one_sub_prim_mul_t0_add_prim__mul_prima_add__one_sub_prim_mul_t0_add_prim},
  // paper mario. Added by Gonetz
  // (1-prim)*t0+prim
  {0x61366136, cc_one_sub_prim_mul_t0_add_prim},
  // arena, Pokemon Stadium 2. Added by Gonetz
  // (1-prim)*t0+prim, (cmb-env)*shade+shade  ** INC **
  {0x61368450, cc_t0_mul_shade},
  // F1 World Grand Prix. Added by Gonetz
  // (1-prim)*t0+prim, (cmb-0)*shade+0 ** INC **
  {0x6136e4f0, cc_t0_mul_shade},
  // Xena. Added by Gonetz
  // (0-prim)*t0+prim
  {0x613f613f, cc_one_sub_t0_mul_prim},
  // Kirby64 end [Raziel64]. Added by Gonetz
  // (prim-env)*t0+prim
  {0x61536153, cc_prim_sub_env_mul_t0_add_prim},
  // Xena. Added by Gonetz
  // (shade-env)*t0+prim
  {0x61546154, cc_shade_sub_env_mul_t0_add_prim},
  // Karts, mario kart
  //z (one-env)*t0+prim
  {0x61566156, cc_t0_mul_1menv_add_prim},
  // Famista64. Added by Gonetz
  //(t0-0)*t0+prim
  {0x61f161f1, cc_t0_mul_prim},
  // Pokemon Stadium 2. Added by Gonetz
  //(shade-0)*t0+prim
  {0x61f461f4, cc_t0_mul_shade_add_prim},
  // Doom. Added by Gonetz
  //(1-0)*t0+prim
  {0x61f661f6, cc_t0_add_prim},
  // tire trace, beetle adventure racing. Added by Gonetz
  // (shade-prim)*t1+prim, (cmb-0)*t1+0  **INC**
  {0x6234e2f0, cc_shade_sub_prim_mul_t1_add_prim},
  // Text, turok
  // (env-prim)*t1+prim
  {0x62356235, cc_env_sub_prim_mul_t1_add_prim},
  // Pokemon Stadium 2, [gokuss4]. Added by Gonetz
  // (env-prim)*t1+prim, (cmb-0)*t1+0
  // Hack alert!
  {0x6235e2f0, cc_t1},
  // bike trace, xg2 intro. Added by Gonetz
  // (1-prim)*t1+prim
  {0x62366236, cc_one_sub_prim_mul_t1_add_prim},
  // aerofighter's assault [Ogy]
  // (1-prim)*t1+prim, (cmb-0)*0+env
  {0x6236bff0, cc_one_sub_prim_mul_t1_add_prim},
  // Tennis court, mario tennis
  // (t0-0)*t1+prim
  {0x62f162f1, cc__t0_mul_t1__add_prim},
  // Arena, Pokemon Stadium 2
  // (t0-0)*t1+prim, (cmb-0)*shade+0
  {0x62f1e4f0, cc__t0_mul_t1_add_prim__mul_shade},
  // Rush2. Added by Gonetz
  // (prim-prim)*prim+prim
  {0x63336333, cc_prim},
  //Bowser in final battle, Paper Mario. Added by Gonetz
  // (t1-0)*prim+prim
  {0x63f263f2, cc_t1_mul_prim_add_prim},
  // wetrix, icelayer, [Raziel64]. Added by Gonetz
  // (t0-prim)*shade+prim  ** INC **
  {0x64316431, cc_t0_mul_shade},
  // KI. Added by Gonetz
  // (env-prim)*shade+prim
  {0x64356435, cc_env_sub_prim_mul_shade_add_prim},
  // xg2. Added by Gonetz
  // (1-prim)*shade+prim, (t0-0)*cmb+0  ** INC **
  {0x6436e0f1, cc_t1_mul__one_sub_prim_mul_shade_add_prim},
  // Intro, CBFD. Added by Gonetz
  // (t0-env)*shade+prim
  {0x64516451, cc_t0_sub_env_mul_shade_add_prim},
  // sword in final battle, zelda 2. Added by Gonetz
  // (t0-env)*shade+prim, (cmb-0)*shade+0 ** INC **
  {0x6451e4f0, cc__t0_sub_env_mul_shade_add_prim__mul_shade},
  // attack, Pokemon Stadium 2.
  // (t0-env)*shade+prim, (cmb-0)*shade_a+0 ** INC **
  {0x6451ebf0, cc__t0_sub_env_mul_shade_add_prim__mul_shadea},
  // Road Rush. Added by Gonetz
  // (t0-0)*shade+prim
  {0x64f164f1, cc_t0_mul_shade_add_prim},
  // paper mario. Added by Gonetz
  // (1-0)*shade+prim
  {0x64f664f6, cc_prim_add_shade},
  // Character select, smash bros
  // (t0-prim)*env+prim
  {0x65316531, cc_t0_sub_prim_mul_env_add_prim},
  // Clear screen intro, banjo kazooie
  // (t0-prim)*env+prim, (cmb-0)*shade+0
  //    {0x6531e4f0, cc_t0_mul_env_mul_shade},
  {0x6531e4f0, cc__prim_inter_t0_using_env__mul_shade},
  // Dragonfly feet, banjo kazooie
  // (1-prim)*env+prim, (cmb-0)*shade+0
  {0x6536e4f0, cc__prim_inter_one_using_env__mul_shade},
  // Lava piranha atack, Paper Mario
  // (t1-k4)*env+prim       ** INC **
  {0x65726572, cc_t1_mul_env_add_prim},
  // zelda 2 [Ogy]. Added by Gonetz
  // (t0-0)*env+prim, (1-t1)*t0a+cmb  ** INC **
  {0x65f10826, cc_one_sub_t1_mul_t0a_add_t0_mul_env_add_prim},
  // clocks while warping through time, zelda 2
  // (t0-0)*env+prim, (cmb-0)*0+cmb
  {0x65f11ff0, cc_t0_mul_env_add_prim},
  // Helicopter, Nuclear Strike. Added by Gonetz
  // (t0-0)*env+prim
  {0x65f165f1, cc_t0_mul_env_add_prim},
  // Mystical Ninja
  // (1-0)*env+prim
  {0x65f665f6, cc_prim_add_env},
  // duke nukem: zero hour [Ogy]
  // (noise-0)*env+prim     ** INC **
  {0x65f765f7, cc_prim_add_env},
  // "terminator", CBFD
  // (0-0)*env+prim
  {0x65ff65ff, cc_prim},
  // Cliffs, Taz express. Added by Gonetz
  // (t0-0)*scale+prim
  {0x66f166f1, cc_t0_mul_scale_add_prim},
  // Taz express. Added by Gonetz
  // (t0-0)*scale+prim, (cmb-0)*shade+0
  {0x66f1e4f0, cc_t0_mul_scale_add_prim__mul_shade},
  // NFL Quarterback Club 98 Menu [CpUMasteR]
  // (prim-0)*scale+prim
  {0x66f366f3, cc_prim},
  // Pikachu
  // (t0-prim)*t0_a+prim, (env-cmb)*enva+cmb
  {0x68310c05, cc__prim_inter_t0_using_t0a__inter_env_using_enva},
  // Character, dual heroes
  // (t0-prim)*t0_a+prim
  {0x68316831, cc_t0_sub_prim_mul_t0a_add_prim},
  // Indy Racing 2000. Added by Gonetz
  // (t0-prim)*t0_a+prim, (cmb-0)*shade+0    ** INC **
  {0x6831e4f0, cc__prim_inter_t0_using_t0a__mul_shade},
  // text, Sin and Punishmen. Added by Gonetz
  // (env-prim)*t0_a+prim    ** INC **
  {0x68356835, cc_env_sub_prim_mul_t0a_add_prim},
  // arena, Pokemon Stadium 2
  // (1-prim)*t0_a+prim
  {0x68366836, cc_one_sub_prim_mul_t0a_add_prim},
  // menu, PD. Added by Gonetz
  // (env-prim)*t1_a+prim
  {0x69356935, cc_env_sub_prim_mul_t1a_add_prim},
  //    {0x69356935, cc_t1},
  //xg2. Added by Gonetz
  // (t0-prim)*prima+prim
  {0x6a316a31, cc_t0_sub_prim_mul_prima_add_prim},
  // menu, battle phoenix 64. Added by Gonetz
  // (env-prim)*prima+prim
  {0x6a356a35, cc_env_sub_prim_mul_prima_add_prim},
  // ground, KI. Added by Gonetz
  // (shade-env)*prima+prim
  {0x6a546a54, cc_shade_sub_env_mul_prima_add_prim},
  // F1 World Grand Prix. Added by Gonetz
  // (t0-0)*prima+prim, (shade-0)*cmb+env **INC**
  {0x6af1a0f4, cc__t0_mul_prima_add_prim_mul__shade_add_env},
  //broken wall, beetle adventure racing. Added by Gonetz
  // (t0-0)*prima+prim, (cmb-0)*shade+0 **INC**
  {0x6af1e4f0, cc__t0_mul_prima_add_prim_mul__shade},
  // Genie, diddy kong racing
  // (t0-prim)*shade_alpha+prim, (env-cmb)*shade+cmb
  //    {0x6b310405, cc_env_sub__prim_inter_t0_using_shadea__mul_shade_add_env},
  {0x6b310405, cc_t0_mul_shadea},
  // Extreme G. Added by Gonetz
  // (t0-prim)*shade_alpha+prim  ** INC **
  {0x6b316b31, cc_t0_sub_prim_mul_shadea_add_prim},
  // water block, Paper Mario. Added by Gonetz
  // (t0-prim)*shade_alpha+prim, (prim-env)*cmb+env  ** INC **
  {0x6b31a053, cc_prim_sub_env_mul__prim_inter_t0_using_shadea__add_env},
  // water, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (t0-prim)*shade_alpha+prim, (cmb-0)*shade+0  ** INC **
  {0x6b31e4f0, cc__prim_inter_t0_using_shadea__mul_shade},
  // F1 World Grand Prix sky. Added by Gonetz
  // (env-prim)*shade_alpha+prim, (shade-cmb)*cmb_a+cmb ** INC **
  {0x6b350704, cc_f1_sky},
  // lullaby, Paper Mario. Added by Gonetz
  // (env-prim)*shade_alpha+prim
  {0x6b356b35, cc_env_sub_prim_mul_shadea_add_prim},
  // Some gannon spell, zelda
  // (noise-t0)*env_a+prim, (0-prim)*cmb+1  ** INC **
  {0x6c17c03f, cc_one_sub__one_sub_t0_mul_enva_add_prim__mul_prim},
  //Goldeneye, [Jeremy]. Added by Gonetz
  // (t0-prim)*env_a+prim
  {0x6c316c31, cc_t0_sub_prim_mul_enva_add_prim},
  // button, Sin and Punishmen. Added by Gonetz
  // (env-prim)*env_a+prim
  {0x6c356c35, cc_env_sub_prim_mul_enva_add_prim},
  // frame buffer effect, Glover2
  // (env-prim)*env_a+prim, (cmb-0)*shade+0
  {0x6c35e4f0, cc__prim_inter_env_using_enva__mul_shade},
  // fallen stars at star summit, Paper Mario. Added by Gonetz
  // (t0-env)*env_a+prim, (1-0)*primlod+cmb
  {0x6c510ef6, cc_t0_sub_env_mul_enva_add_prim},
  // focus, Paper Mario. Added by Gonetz
  // (t0-env)*env_a+prim, (cmb-shade)*shadea+shade  ** INC **
  {0x6c518b40, cc_t0_sub_shade_mul_shadea_add_shade},
  // Ring, pokemon stadium 2
  // (t0-0)*env_a+prim, (1-0)*cmb+0
  {0x6cf1e0f6, cc_t0_mul_enva_add_prim},
  // Jet Force
  // (noise-0)*env_a+prim
  {0x6cf76cf7, cc_prim},
  // snowhead temple, zelda 2. Added by Gonetz
  // (t1-t0)*primlod+prim, (cmb-0)*shade+shade
  {0x6e1284f0, cc__t1_sub_t0_mul_primlod_add_prim__mul_shade_add_shade},
  // zelda 2. Added by Gonetz
  // (t1-t0)*primlod+prim, (cmb-0)*shade+0  ** INC **
  {0x6e12e4f0, cc__t1_sub_t0_mul_primlod_add_prim__mul_shade},
  // mini games quiz monitor backround, Pokemon Stadium 2
  // (noise-t0)*primlod+prim, (prim-env)*cmb+env  ** INC **
  {0x6e17a053, cc_prim_sub_env_mul__one_sub_t0_mul_primlod_add_prim__add_env},
  // Morning Sun attack, pokemon stadium 2
  // (t0-prim)*primlod+prim, (prim-env)*0+cmb
  {0x6e311f53, cc_t0_sub_prim_mul_primlod_add_prim},
  // sky, daikatana. Added by Gonetz
  // (t0-prim)*primlod+prim, (cmb-0)*shade+0
  {0x6e31e4f0, cc_t0_mul_shade},
  // ball's track, NFL Blitz. Added by Gonetz
  // (t0-0)*primlod+prim
  {0x6ef16ef1, cc_t0_mul_primlod_add_prim},
  // Earthquake pokemon attack, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t0-0)*primlod+prim, (cmb-env)*cmb_a+env ** INC **
  {0x6ef1a750, cc_t0_mul_primlod_add_prim},
  // mini games quiz monitor backround, Pokemon Stadium 2
  // (noise-0)*primlod+prim, (env-cmb)*cmb_a+cmb ** INC **
  // use cmb_a which is ac_t0_mul_t1
  {0x6ef70705, cc_env_sub_prim_mul__t0a_mul_t1a__add_prim},
  // rope, CBFD
  // (t0-env)*k5+prim
  {0x6f516f51, cc_t0_sub_env_mul_k5_add_prim},
  // super bowling
  // (0-0)*k5+prim
  {0x6fff6fff, cc_prim},
  // intro, Aidyn Chronicles. Added by Gonetz
  // (0-0)*0+prim, (0-0)*0+prim
  {0x79fb7788, cc_prim},
  // Encore attack, Pokemon Stadium 2
  // (t0-0)*0+prim, (cmb-0)*shade+0
  {0x7ff1e4f0, cc_prim_mul_shade},
  // Menu, megaman
  // (1-0)*0+prim
  {0x7ff67ff6, cc_prim},
  // sky, PGA European Tour
  // (0-0)*0+prim, (env-0)*t0+cmb
  {0x7fff01f5, cc_t1_mul_env_add_prim},
  // WWF No Mercy?
  // ((0-0)*0+prim, (env-cmb)*shade+cmb
  {0x7fff0405, cc_env_sub_prim_mul_shade_add_prim},
  // sky, Spiderman. Added by Gonetz
  // (0-0)*0+prim, (t1-0)*shade+cmb
  {0x7fff04f2, cc_t1_mul_shade_add_prim},
  // ball's shadow, ISS 2k. Added by Gonetz
  // (0-0)*0+prim, (1-cmb)*env+cmb
  {0x7fff0506, cc_one_sub_prim_mul_env_add_prim},
  // Necklace, quest64
  // (0-0)*0+prim, (1-0)*env+cmb
  {0x7fff05f6, cc_prim_add_env},
  // Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (0-0)*0+prim, (1-cmb)*cmba+cmb ** INC **
  {0x7fff0706, cc_prim},
  // Dobutsu no Mori. Added by Gonetz
  //(k5-k5)*0+prim, (cmb-0)*0+cmb
  {0x7fff1ff0, cc_prim},
  // Intro background, starfox
  //z (k5-k5)*0+prim
  {0x7fff7fff, cc_prim},
  // train smoke, Dobutsu No Mori. Added by Gonetz
  //(0-0)*0+prim, (shade-0)*cmb+0
  {0x7fffe0f4, cc_prim_mul_shade},
  // Donald Duck intro. Added by Gonetz
  //(0-0)*0+prim, (cmb-0)*prim+0
  {0x7fffe3f0, cc_prim_mul_prim},
  // Ms. Pac-Man intro. Added by Gonetz
  //(0-0)*0+prim, (cmb-0)*shade+0
  {0x7fffe4f0, cc_prim_mul_shade},
  // zelda 2.  Added by Gonetz
  //(t1-t0)*t0+shade, (cmb-0)*shade+0
  {0x8112e4f0, cc__t1_sub_t0_mul_t0_add_shade__mul_shade},
  // branches, Beetle Adventure Racing
  //(t0-shade)*t0+shade, (t0-cmb)*prim+cmb **INC**
  {0x81410301, cc_t0_mul_prim},
  // Namco logo, Famista 64
  //(prim-shade)*t0+shade, (env-cmb)*t0+cmb **INC**
  {0x81430105, cc_prim_sub_shade_mul_t0_add_shade},
  // pikachu, hey you pikachu
  //(prim-shade)*t0+shade, (env-cmb)*enva+cmb **INC**
  {0x81430c05, cc_prim_sub_shade_mul_t0_add_shade},
  // Mario's head, mario //Added by Gonetz
  //(prim-shade)*t0+shade
  {0x81438143, cc_prim_sub_shade_mul_t0_add_shade},
  // Iguana background, turok
  // (env-shade)*t0+shade
  {0x81458145, cc_env_sub_shade_mul_t0_add_shade},
  //attack, Pokemon Stadium 2
  // (env-shade)*t0+shade, (cmb-0)*prim+0
  {0x8145e3f0, cc__env_sub_shade_mul_t0_add_shade__mul_prim},
  // Bubbles in Jabu-Jabu's belly, zelda
  // (1-shade)*t0+shade
  {0x81468146, cc_one_sub_shade_mul_t0_add_shade},
  // saffron city, Pokemon Stadium 2
  // (1-shade)*t0+shade, (cmb-0)*prim+0
  {0x8146e3f0, cc__one_sub_shade_mul_t0_add_shade__mul_prim},
  // duck dodgers intro. Added by Gonetz
  // (1-shade)*t0+shade, (cmb-0)*shade+0
  {0x8146e4f0, cc__one_sub_shade_mul_t0_add_shade__mul_shade},
  // saffron city, Pokemon Stadium 2
  // (1-shade)*t0+shade, (cmb-0)*prima+0
  {0x8146eaf0, cc__one_sub_shade_mul_t0_add_shade__mul_env},
  // intro, Madden Footbal
  // (1-env)*t0+shade
  {0x81568156, cc_one_sub_env_mul_t0_add_shade},
  // sky in doom. Added by Gonetz
  // (prim-0)*t0+shade, (cmb-0)*primlod+env  **INC**
  {0x81f3aef0, cc_t0_mul_prim_add_shade},
  // commercial? in IIS98. Added by Gonetz
  // (1-0)*t0+shade
  {0x81f681f6, cc_t0_add_shade},
  //attack, Pokemon Stadium 2
  //(t0-prim)*t1+shade
  {0x82318231, cc_t0_sub_prim_mul_t1_add_shade},
  //beetle adventure racing. Added by Gonetz
  //(prim-shade)*t1+shade, (cmb-0)*t1+0  **INC**
  {0x8243e2f0, cc_prim_sub_shade_mul_t1_add_shade},
  //Arena, Pokemon Stadium 2
  //(t0-0)*t1+shade
  {0x82f182f1, cc__t0_mul_t1__add_shade},
  //Arena, Pokemon Stadium 2
  //(t0-0)*t1+shade, (cmb-0)*prim+0
  {0x82f1e3f0, cc__t0_mul_t1__mul_prim_add_prim_mul_shade},
  // Scorpion fire breath, MK4 [Jeremy]. Added by Gonetz
  // (t0-shade)*prim+shade
  {0x83418341, cc_t0_mul_prim_add_one_sub_prim_mul_shade},
  // Menu background, wwf no mercy
  // (env-shade)*prim+shade
  {0x83458345, cc_prim_mul_env_add_one_sub_prim_mul_shade},
  // Pokemon selection window background, pokemon stadium 2
  // (noise-shade)*prim+shade
  {0x83478347, cc_shade},
  // crown of king of ikana, zelda 2. Added by Gonetz
  // (t0-env)*prim+shade
  {0x83518351, cc_t0_sub_env_mul_prim_add_shade},
  // crown of king of ikana, zelda 2. Added by Gonetz
  // (t0-env)*prim+shade, (cmb-0)*cmb+0  ** INC **
  {0x8351e0f0, cc_t0_sub_env_mul_prim_add_shade},
  // salesman's shirt in the bomb shop in town, zelda 2 [Ogy]. Added by Gonetz
  // (t0-env)*prim+shade, (cmb-0)*shade+0  ** INC **
  {0x8351e4f0, cc_t0_mul_prim_mul_shade},
  // intro, Madden Footbal
  // (1-env)*prim+shade
  {0x83568356, cc_one_sub_env_mul_prim_add_shade},
  // Buss hunter 64. Added by Gonetz
  // (t0-0)*prim+shade
  {0x83f183f1, cc_t0_mul_prim_add_shade},
  // huge water lilies, zelda 2 [Ogy]. Added by Gonetz
  // (t0-0)*prim+shade, (cmb-env)*shade+0  ** INC **
  {0x83f1e450, cc__t0_mul_prim_add_shade__sub_env_mul_shade},
  // cynnabar gym fire shield, pokemon stadium 2
  // (t0-0)*prim+shade, (cmb-0)*env+0  ** INC **
  {0x83f1e5f0, cc__t0_mul_prim_add_shade__mul_env},
  // Objects in arena, pokemon stadium 2
  // (t1-0)*prim+shade, (cmb-0)*prim_a+0  - not going to bother with prim_a since it is FF
  {0x83f2eaf0, cc_t1_mul_prim_add_shade},
  // Pokemon Stadium 2. Added by Gonetz
  // (t0-prim)*shade+shade  ** INC **
  {0x84318431, cc_t0_mul_shade},
  // big N, Pokemon Stadium 2. Added by Gonetz
  // (1-prim)*shade+shade  ** INC **
  {0x84368436, cc_one_sub_prim_mul_shade_add_shade},
  //Arena, Pokemon Stadium 2
  //(t0-env)*shade+shade
  {0x84518451, cc_t0_sub_prim_mul_shade_add_shade},
  //Arena, Pokemon Stadium 2
  //(t0-env)*shade+shade, (cmb-0)*prim+0
  {0x8451e3f0, cc_t0_sub_env_mul_prim_mul_shade_add_prim_mul_shade},
  // arena, PokemonStadium2, [Raziel64]
  // (t0-0)*shade+shade, (cmb-0)*prim+0
  {0x84f1e3f0, cc_t0_mul_prim_mul_shade_add_prim_mul_shade},
  // Spiderman. Added by Gonetz
  // (1-0)*shade+shade
  {0x84f684f6, cc_shade_add_shade},
  // the "gekko" ( a monster in a room above the 3rd room of woodfall temple }, zelda 2 [Ogy]. Added by Gonetz
  // (t0-prim)*env+shade  ** INC **
  {0x85318531, cc_t0_sub_prim_mul_env_add_shade},
  // flower, zelda 2. Added by Gonetz
  // (t0-prim)*env+shade, (cmb-0)*shade+0  ** INC **
  {0x8531e4f0, cc_t0_sub_prim_mul_env_add_shade},
  // Robotron 64, [scorpiove]
  // (env-shade)*env+shade  ** INC **
  {0x85458545, cc_one_sub_env_mul_shade_add_env},
  // Enemy dying, quest64
  // (1-shade)*env+shade  **changed by Gonetz
  {0x85468546, cc_one_sub_shade_mul_env_add_shade},
  // Arena, Pokemon Stadium
  // (t0-0)*env+shade, (cmb-0)*prim+0
  {0x85f1e3f0, cc__t0_mul_prim_mul_env__add__prim_mul_shade},
  // Clouds, Pokemon Stadium
  // (t1-0)*env+shade, (cmb-0)*prim+0
  {0x85f2e3f0, cc__t1_mul_prim_mul_env__add__prim_mul_shade},
  // Sky, Beetle Adventure Racing ** INC **
  //(t0-shade)*t0_a+shade, (env-cmb)*enva+cmb
  {0x88410c05, cc_t0_sub_shade_mul_t0a_add_shade},
  // Mario's eyes, mario
  //z (t0-shade)*t0_a+shade
  {0x88418841, cc_t0_sub_shade_mul_t0a_add_shade},
  //beetle adventure racing. Added by Gonetz
  // (prim-shade)*t0_a+shade, (t1-0)*cmb+0  **INC**
  {0x8843e0f2, cc_prim_sub_shade_mul__t0a_mul_t1__add_shade},
  // blast corps [Ogy]
  // (prim-shade)*t1_a+shade
  {0x89438943, cc_prim_sub_shade_mul_t1a_add_shade},
  //broken wall, beetle adventure racing. Added by Gonetz
  // (t0-shade)*prima+shade, (1-0)*0+cmb
  {0x8a411ff6, cc_t0_sub_shade_mul_prima_add_shade},
  // menu, battle phoenix 64. Added by Gonetz
  // (t0-shade)*prima+shade
  {0x8a418a41, cc_t0_add_shade},
  // intro, castlevania 2. Added by Gonetz
  // (prim-shade)*prim_a+shade
  {0x8a438a43, cc_prim_sub_shade_mul_prima_add_shade},
  // Pilot wings
  // (t0-shade)*shade_a+shade, (cmb-0)*shade+0
  {0x8b41e4f0, cc__shade_inter_t0_using_shadea__mul_shade},
  // ?
  // (1-shade)*shade_a+shade
  {0x8b468b46, cc_one_sub_shade_mul_shadea_add_shade},
  // Pilot wings, sky in congratulations
  // (t0-0)*shade_a+shade,
  {0x8bf18bf1, cc_t0_mul_shadea_add_shade},
  // arena, Pokemon Stadium. Added by Gonetz
  // (t0-t1)*env_a+shade, (cmb-env)*prim+0  ** INC **
  {0x8c21e350, cc__t0_sub_t1_mul_enva_add_shade__sub_env_mul_prim},
  //diddy kong racing background fill. Added by Gonetz ** Modified by Dave2001
  // (env-shade)*env_a+shade, (cmb-0)*prim+0
  {0x8c458c45, cc_shade},  // note: previous combiner used other_alpha; doesn't work
  //diddy kong racing. Added by Gonetz
  // (env-shade)*env_a+shade, (cmb-0)*prim+0  ** INC **
  {0x8c45e3f0, cc_prim_mul_shade},
  // sky, Pokemon Stadium, [Raziel64]
  // (t0-0)*env_a+shade, (cmb-env)*prim+0  ** INC **
  {0x8cf1e350, cc_t0_mul_prim_add_shade_sub_env_mul_prim},
  // zelda 2 [Ogy]. Added by Gonetz
  // (t0-prim)*primlod+shade, (prim-env)*cmb+env  ** INC **
  {0x8e31a053, cc_prim_sub_env_mul__t0_sub_prim_mul_primlod_add_shade__add_env},
  // fallen leaves, Dobutsu no Mori. Added by Gonetz
  // (t0-shade)*primlod+shade, (prim-env)*cmb+env  ** INC **
  {0x8e41a053, cc_prim_sub_env_mul__t0_sub_shade_mul_primlod_add_shade__add_env},
  // the icicle above the part just before the entrance to the mountain village, zelda 2 [Ogy]. Added by Gonetz
  // (t0-prim)*0+shade, (prim-env)*cmb+env   ** INC ** ?
  {0x9f31a053, cc_prim_sub_env_mul_shade_add_env},
  // background on level 3-1, kirby 64 [Raziel64]. Added by Gonetz
  // (0-env)*0+shade
  {0x9f5f9f5f, cc_shade},
  // Spotlight, smash bros
  // (1-0)*0+shade
  {0x9ff69ff6, cc_shade},
  // water, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (0-0)*0+shade, (cmb-cmb)*cmb+cmb
  {0x9fff0000, cc_shade},
  // menu, Dr.Mario. Added by Gonetz
  // (0-0)*0+shade, (prim-cmb)*env+cmb
  {0x9fff0503, cc_prim_sub_shade_mul_env_add_shade},
  // pikachu, hey you pikachu. Added by Gonetz
  // (0-0)*0+shade, (env-cmb)*enva+cmb
  {0x9fff0c05, cc_env_sub_shade_mul_enva_add_shade},
  // mega shock, paper mario
  //(0-0)*0+shade, (env-prim)*cmb+prim
  {0x9fff6035, cc_env_sub_prim_mul_shade_add_prim},
  // Super Mario 64 logo background
  //z (k5-k5)*0+shade
  {0x9fff9fff, cc_shade},
  // Zelda 2 final movie. Added by Gonetz
  // (0-0)*0+shade, (prim-0)*cmb+0
  {0x9fffe0f3, cc_prim_mul_shade},
  // tree shadow, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (0-0)*0+shade, (env-0)*cmb+0
  {0x9fffe0f5, cc_env_mul_shade},
  // N64 logo, Aidyn Chronicles. Added by Gonetz
  // (0-0)*0+shade, (cmb-0)*prim+0
  {0x9fffe3f0, cc_prim_mul_shade},
  // Hand, smash bros
  // (0-0)*0+shade, (cmb-0)*env+0
  {0x9fffe5f0, cc_env_mul_shade},
  // Lave piranha atack, Paper Mario
  // (t1-t0)*t0+env, (cmb-t1)*t0+prim  ** INC **
  {0xa1126120, cc__t0_mul_t1__mul_env_add_prim},
  //Arena, Pokemon Stadium 2
  // (t1-prim)*t0+env, (cmb-0)*shade+0
  {0xa132e4f0, cc__t1_sub_prim_mul_t0_add_env__mul_shade},
  // Kirby64 end [Raziel64]. Added by Gonetz
  // (prim-shade)*t0+env
  {0xa143a143, cc_prim_sub_shade_mul_t0_add_env},
  // Superman [scorpiove]. Added by Gonetz
  // (t0-env)*t0+env
  {0xa151a151, cc_t0_sub_env_mul_t0_add_env},
  // powder keg, zelda 2. Added by Gonetz
  // (prim-env)*t0+env, (0-0)*shade_a+cmb
  {0xa1530bff, cc__prim_sub_env_mul_t0_add_env__add_shadea},
  // pokemon attack, Pokemon Stadium 2. Added by Gonetz
  // (prim-env)*t0+env, (0-0)*prim_lod+cmb
  {0xa1530ef6, cc__prim_sub_env_mul_t0_add_env__add_primlod},
  //attack, Pokemon Stadium 2
  // (prim-env)*t0+env, (0-0)*prim_lod+cmb
  {0xa1530eff, cc_prim_sub_env_mul_t0_add_env},
  // Kotake & koume defeated, going into sky, zelda
  // (prim-env)*t0+env, (prim-env)*0+cmb
  {0xa1531f53, cc_prim_sub_env_mul_t0_add_env},
  // water, Dobutsu no Mori. Added by Gonetz
  // (prim-env)*t0+env, (cmb-0)*shade+t0
  {0xa15324f0, cc_t0_add_shade_mul_env},
  //sky, beetle adventure racing. Added by Gonetz
  // (prim-env)*t0+env, (cmb-shade)*t1+shade **INC** can't be done in one step
  {0xa1538240, cc__env_inter_prim_using_t0__sub_shade_mul_t0a_add_shade},
  //couple's mask, zelda2. Added by Gonetz
  // (prim-env)*t0+env, (prim-cmb)*shade+shade **INC** can't be done in one step
  {0xa1538403, cc_t0_mul_shade},
  // stadium, Pokemon Stadium 2. Added by Gonetz
  // (prim-env)*t0+env, (cmb-0)*shade+shade **INC** can't be done in one step
  {0xa15384f0, cc_t0_mul_shade},
  //clothes on girl in inn, zelda2. Added by Gonetz
  // (prim-env)*t0+env, (cmb-prim)*env+shade **INC** can't be done in one step
  {0xa1538530, cc_t0_mul_env_add_shade},
  // Getting light arrows for the first time, zelda
  // (prim-env)*t0+env, (prim-env)*cmb+env  ** INC **
  {0xa153a053, cc_prim_sub_env_mul_t0_add_env},
  // Fire, starfox
  // (prim-env)*t0+env
  {0xa153a153, cc_prim_sub_env_mul_t0_add_env},
  // a spell, Fushigi no Dungeon: Fuurai no Shiren 2
  // (prim-env)*t0+env, (cmb-env)*enva+env
  {0xa153ac50, cc_prim_sub_env_mul__t0_mul_enva__add_env},
  // wizrobe's attack, zelda 2. Added by Gonetz.
  // (prim-env)*t0+env, (cmb-0)*cmb+0
  {0xa153e0f0, cc_prim_sub_env_mul_t0_add_env},
  // dress, zelda 2. Added by Gonetz.
  // also for Great Farie's hair - changed to use texture mod by Dave2001.
  // (prim-env)*t0+env, (shade-0)*cmb+0
  {0xa153e0f4, cc__env_inter_prim_using_t0__mul_shade},
  // Start menu, paper mario
  // (prim-env)*t0+env, (cmb-0)*t0+0
  {0xa153e1f0, cc_prim_sub_env_mul_t0_add_env},
  //    {0xa153e0f4, cc_prim_sub_env_mul_t0_add_env},
  // Jellyfish tentacles in Jabu-Jabu's belly, zelda
  // (prim-env)*t0+env, (cmb-0)*prim+0
  {0xa153e3f0, cc__env_inter_prim_using_t0__mul_prim},
  // Dust, zelda
  //z (prim-env)*t0+env, (cmb-0)*shade+0   ** INC **
  {0xa153e4f0, cc__env_inter_prim_using_t0__mul_shade},
  //{0xa153e4f0, cc_prim_sub_env_mul_t0_add_env},
  // roof, Kirby 64. Added by Gonetz
  // (prim-env)*t0+env, (cmb-0)*env+0   ** INC **
  {0xa153e5f0, cc_prim_sub_env_mul_t0_add_env},
  // hall of fame, Pokemon Stadium
  // (prim-env)*t0+env, (cmb-0)*primlod+0
  {0xa153eef0, cc__prim_sub_env_mul_t0_add_env__mul_primlod},
  // Something weird in intro, monster truck madness
  // (prim-env)*t0+env, (cmb-0)*k5+0
  {0xa153eff0, cc__prim_sub_env_mul_t0_add_env__mul_k5},
  // clothes, kirby 64. Added by Gonetz
  // (shade-env)*t0+env
  {0xa154a154, cc_shade_sub_env_mul_t0_add_env},
  // field, Derby Stallion
  // (shade-env)*t0+env, (cmb-0)*prim+0 ** INC **
  {0xa154e3f0, cc_shade_sub_env_mul_t0_mul_prim_add_prim_mul_env},
  // background, level 3-5, kirby 64, [Raziel64]
  // (shade-env)*t0+env, (cmb-0)*shade+0 ** INC **
  {0xa154e4f0, cc_shade_sub_env_mul_t0_add_env},
  // pokemon attack, Pokemon Stadium 2. Added by Gonetz
  // (one-env)*t0+env
  {0xa156a156, cc_one_sub_env_mul_t0_add_env},
  // Arena, Pokemon Stadium 2.
  // (one-env)*t0+env, (cmb-0)*shade+0
  {0xa156e4f0, cc__env_inter_one_using_t0__mul_shade},
  //Arena, Pokemon Stadium 2
  // (t1-0)*t0+env, (cmb-0)*cmb+0  ** INC **
  {0xa1f2e0f0, cc__t0_mul_t1__add_env_mul__t0_mul_t1__add_env},
  // quake 2 intro
  // (prim-0)*t0+env, (prim-0)*primlod+cmb  ** INC **
  {0xa1f30ef3, cc_t0_mul_prim_add_env},
  // Kotake or koume's hair, zelda
  // (prim-0)*t0+env
  {0xa1f3a1f3, cc_t0_mul_prim_add_env},
  // track, ridge racer. Added by Gonetz
  // (t0-env)*t1+env, (cmb-0)*shade+0   ** INC **
  {0xa251e4f0, cc__t0_mul_t1__mul_shade},
  // lava, beetle adventure racing
  // (t0-env)*t1+env, (cmb-0)*enva+0   ** INC **
  {0xa251ecf0, cc__t0_mul_t1__mul_enva},
  // Ded Moroz, Paper Mario
  // (prim-env)*t1+env, (1-cmb)*t1+cmb  ** INC **
  {0xa2530206, cc_prim_sub_env_mul_t1_add_env},
  // text, monster truck madness
  // (prim-env)*t1+env
  {0xa253a253, cc_prim_sub_env_mul_t1_add_env},
  // car position, Top Gear Rally. Added by Gonetz
  // (prim-env)*t1+env, (cmb-t0)*t1+0  ** INC **
  {0xa253e210, cc_prim_sub_env_mul_t1_add_env_mul_t0},
  // text, Top Gear Rally. Added by Gonetz
  // (prim-env)*t1+env, (cmb-0)*t1+0  ** INC **
  {0xa253e2f0, cc_prim_sub_env_mul_t1_add_env_mul_t0},
  // {0xa253e2f0, cc_prim_sub_env_mul_t1_add_env},
  // a pole in the cut-scene that appears after you receive odolwa's mask, zelda 2 [Ogy]. Added by Gonetz
  // (prim-env)*t1+env, (cmb-0)*shade+0  ** INC **
  {0xa253e4f0, cc_t1_mul_prim_mul_shade},
  // Quake 2 intro. Added by Gonetz
  // (t0-0)*t1+env, (t0-0)*primlod+cmb  ** INC **
  {0xa2f10ef1, cc__t0_mul_t1__add_env},
  // silver cave, pokemon stadium 2
  // (t0-0)*t1+env, (cmb-prim)*shadea+prim
  {0xa2f16b30, cc_prim_inter__t0_mul_t1_add_env__using_shadea},
  // silver cave, pokemon stadium 2
  // (t0-0)*t1+env, (cmb-0)*shadea+shade
  {0xa2f18bf0, cc__t0_mul_t1_add_env__mul_shadea_add_shade},
  // Quake64. Added by Gonetz
  // (t0-0)*t1+env
  {0xa2f1a2f1, cc__t0_mul_t1__add_env},
  // Quake II. Added by Gonetz   ** INC **
  // (t0-0)*t1+env, (cmb-0)*prim+env
  {0xa2f1a3f0, cc__t0_mul_t1__mul_prim_add_env},
  // Dr Mario [Ogy]. Added by Gonetz
  // (t0-env)*prim+env
  //    {0xa351a351, cc_t0_mul_prim_add_env},
  {0xa351a351, cc_t0_sub_env_mul_prim_add_env},
  // menu, Dr.Mario. Added by Gonetz
  // (prim-env)*prim+env
  {0xa353a353, cc_prim_sub_env_mul_prim_add_env},
  // Razor sword, zelda 2. Added by Gonetz
  // (shade-env)*prim+env, (cmb-0)*shade+0  ** INC **
  {0xa354e4f0, cc_shade_sub_env_mul_prim_add_env},
  // bomberman 64-2 intro. Added by Gonetz
  // (1-env)*prim+env
  {0xa356a356, cc_one_sub_env_mul_prim_add_env},
  // thing that escapes from the well, zelda
  // (noise-env)*prim+env
  {0xa357a357, cc_prim_add_env},
  // Bongo Bongo, zelda
  // (noise-env)*prim+env, (cmb-0)*shade+0
  {0xa357e4f0, cc_env_mul_shade},
  // paper mario. Added by Gonetz
  // (t0-0)*prim+env
  {0xa3f1a3f1, cc_t0_mul_prim_add_env},
  // paper mario. Added by Gonetz
  // (t0-0)*prim+env, (t0-env)*prim+0
  {0xa3f1e351, cc_t0_mul_prim_add_env},
  // paper mario. Added by Gonetz
  // (t0-0)*prim+env, (t0-0)*prim+0
  {0xa3f1e3f1, cc_t0_mul_prim},
  // mahogany town statue, Pokemon Stadium 2
  // (t0-0)*prim+env, (cmb-0)*shade+0
  {0xa3f1e4f0, cc__t0_mul_prim_add_env__mul_shade},
  // squirt, paper mario. Added by Gonetz
  // (t1-0)*prim+env, (1-cmb)*t1+cmb
  {0xa3f20206, cc_t1_mul_prim_add_env},
  // paper mario. Added by Gonetz
  // (shade-0)*prim+env
  {0xa3f4a3f4, cc_prim_mul_shade_add_env},
  // Sharpen attack, pokemon stadium 2
  // (shade-0)*prim+env, (cmb-0)*shade+0
  {0xa3f4e4f0, cc__prim_mul_shade_add_env__mul_shade},
  // Doraemon 2. Added by Gonetz
  // (1-0)*prim+env
  {0xa3f6a3f6, cc_prim_add_env},
  // Pokemon Stadium 2, [Jeremy]. Added by Gonetz
  // (noise-0)*prim+env  ** INC ** ?
  {0xa3f7a3f7, cc_prim_add_env},
  // monsters, Pokemon Stadium. Added by Gonetz
  // (t0-t1)*shade+env, (cmb-0)*prim+0 ** INC **
  {0xa421e3f0, cc__t0_sub_t1__mul_prim_mul_shade_add_prim_mul_env},
  // background, pokemon stadium 2
  // (t0-prim)*shade+env
  {0xa431a431, cc_t0_sub_prim_mul_shade_add_env},
  // Arena, pokemon stadium 2
  // (t0-prim)*shade+env, (cmb-0)*shade+0
  {0xa431e4f0, cc__t0_sub_prim_mul_shade_add_env__mul_shade},
  // Trophy, pokemon stadium 2
  // (t0-prim)*shade+env, (cmb-0)*shade_a+0
  {0xa431ebf0, cc__t0_sub_prim_mul_shade_add_env__mul_shadea},
  // Buildings, pokemon stadium 2
  // (t1-prim)*shade+env
  {0xa432a432, cc_t1_sub_prim_mul_shade_add_env},
  // bomberman 64 [Ogy]
  // (t0-env)*shade+env
  {0xa451a451, cc_t0_mul_shade_add_env},
  // kirby drill, kirby 64. Added by Gonetz
  // (prim-env)*shade+env
  {0xa453a453, cc_prim_sub_env_mul_shade_add_env},
  // ball, ISS98 intro. Added by Gonetz
  // (t0-0)*shade+env
  {0xa4f1a4f1, cc_t0_mul_shade_add_env},
  // waterfall,  Dobutsu_no_Mori
  // (prim-0)*shade+env, (t0-0)*primlod+cmb
  {0xa4f30ef1, cc_t0_mul_primlod_add_prim_mul_shade_add_env},
  // waterfall,  Dobutsu_no_Mori
  // (prim-0)*shade+env, (t1-0)*primlod+cmb
  {0xa4f30ef2, cc_t1_mul_primlod_add_prim_mul_shade_add_env},
  // score, ISS98 intro. Added by Gonetz
  // (prim-0)*shade+env
  {0xa4f3a4f3, cc_prim_mul_shade_add_env},
  // magic fist, Rayman2. Added by Gonetz
  // (env-0)*shade+env
  {0xa4f5a4f5, cc_env_mul_shade_add_env},
  // gunfire, Quake64. Added by Gonetz
  // (1-0)*shade+env
  {0xa4f6a4f6, cc_env_add_shade},
  // flame, Paper Mario. Added by Gonetz
  // (t0-center)*scale+env, (0-prim)*cmb+env
  {0xa661a03f, cc_env_sub__t0_mul_scale_add_env__mul_prim},
  // N64 BIOS
  // (t0-env)*t0_a+env, cmb*shade
  {0xa851e0f4, cc__env_inter_t0_using_t0a__mul_shade},
  // pink car, f-zero x
  // (t0-env)*t0_a+env, cmb*shade
  {0xa851e4f0, cc__env_inter_t0_using_t0a__mul_shade},
  // PokemonStadium1, [Raziel64]
  // (prim-env)*t0_a+env, (cmb-cmb)*cmb+cmb
  {0xa8530000, cc_prim_sub_env_mul_t0a_add_env},
  // N64 logo, Ogre Battle
  // (prim-env)*t0_a+env
  {0xa853a853, cc_prim_sub_env_mul_t0a_add_env},
  // Mud Slap, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (prim-env)*t0_a+env, (cmb-0)*cmb+0
  {0xa853e0f0, cc_prim_sub_env_mul_t0a_add_env},
  // Tree background, mace
  // (prim-env)*t0_a+env, (cmb-0)*t0+0
  {0xa853e1f0, cc__env_inter_prim_using_t0a__mul_t0},
  //attack, Pokemon Stadium 2
  // (prim-env)*t0_a+env, (cmb-0)*prim+0
  {0xa853e3f0, cc__env_inter_prim_using_t0a__mul_prim},
  // logo, Deadly Arts. Added by Gonetz
  // (prim-env)*t1_a+env
  {0xa953a953, cc_prim_sub_env_mul_t1a_add_env},
  // MarioGolf  text "Birdie Put"
  // (t0-env)*prim_a+env
  {0xaa51aa51, cc_t0_sub_env_mul_prima_add_env},
  // N64 BIOS
  // (t0-env)*prim_a+env, (shade-0)*cmb+0
  {0xaa51e0f4, cc__env_inter_t0_using_prima__mul_shade},
  // N64 BIOS
  // (prim-env)*prima+env, (shade-0)*cmb+0
  {0xaa53e0f4, cc__env_inter_prim_using_prima__mul_shade},
  // Girl, PD intro. Added by Gonetz
  // (t0-env)*shade_alpha+env, (cmb-0)*shade+0 ** INC **
  {0xab51e4f0, cc__env_inter_t0_using_shadea__mul_shade},
  // Some gannon spell, zelda
  // (prim-env)*shade_alpha+env
  {0xab53ab53, cc_prim_sub_env_mul_shadea_add_env},
  //Arena, Pokemon Stadium 2
  // (t0-0)*shade_alpha+env, (cmb-0)*shade+prim
  {0xabf164f0, cc__t0_mul_shadea_add_env__mul_shade_add_prim},
  // Boxes, Taz express. Added by Gonetz
  // (t0-env)*env_a+env
  {0xac51ac51, cc_t0_sub_env_mul_enva_add_env},
  // paper mario. Added by Gonetz
  // (t0-env)*env_a+env, (cmb-0)*shade+0  **INC**
  {0xac51e4f0, cc_t0_mul_env_mul_shade},
  // goal, Monster Truck Madness 64
  // (noise-0)*env_a+env, (cmb-0)*t1+0  **INC**
  {0xacf7e2f0, cc_t1_mul_env},
  // sword on forge, zelda 2. Added by Gonetz
  // (t1-t1)*lodf+env, (t1-t0)*cmb+prim
  {0xae226012, cc__t1_sub_t0__mul_env_add_prim},
  // menu background, Pokemon Stadium 2, [Raziel64]
  // (t0-prim)*lodf+env
  {0xae31ae31, cc_t0_sub_prim_mul_primlod_add_env},
  // odd mushroom, zelda oot. Added by Gonetz
  // (t0-shade)*lodf+env, (prim-env)*cmb+env  ** INC **
  {0xae41a053, cc__env_inter_prim_using__t0_sub_shade_mul_primlod_add_env},
  //    {0xae41a053, cc_prim_sub_env_mul__t0_mul_shade__add_env},
  // Morning Sun, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t0-env)*lodf+env, (cmb-0)*prim+0
  {0xae51e3f0, cc__env_inter_t0_using_primlod__mul_prim},
  //Spacestation Silicon Valley intro. Added by Gonetz
  // (prim-env)*lodf+env
  {0xae53ae53, cc_env_inter_prim_using_primlod},
  // Doom, intro. Added by Gonetz
  // (t0-0)*lodf+env, (t0-0)*lodf+env
  {0xaef1aef1, cc_t0_add_env},
  // Dobutsu no Mori. Added by Gonetz
  // (prim-0)*lodf+env
  {0xaef3aef3, cc_prim_add_env},
  // forest behind window, Dobutsu no Mori. Added by Gonetz
  // (prim-0)*prim_lod+env, (t1-0)*cmb+0
  {0xaef3e0f2, cc_t0_mul__prim_mul_primlod_add_env },
  // tony hawks 2 menu
  // (t0-rnv)*k5+env, (cmb-t1)*t1_a+t1      ** INC ** (correct combiner does not work because of black t1)
  {0xaf514920, cc_t0_sub_env_mul_k5_add_env},
  // intro, Mission Impossible. Added by Gonetz
  // (k5-k5)*0+env, (0-0)*scale+env
  {0xbfffa6ff, cc_env},
  // Something blocking the screen, waverace
  //z (k5-k5)*0+env
  {0xbfffbfff, cc_env},
  // Derby Stallion . Added by Gonetz
  // (0-0)*0+env, (cmb-0)*prim+0
  {0xbfffe3f0, cc_prim_mul_env},
  // zelda 2 [Ogy]. Added by Gonetz
  // (k5-k5)*0+env, ((cmb-0)*shade+0
  {0xbfffe4f0, cc_env_mul_shade},
  // flame, paper mario. Added by Gonetz
  // (t0-t1)*t0+1, (0-prim)*cmb+env  **INC** weird
  {0xc121a03f, cc__t0_inter_t1_using_half__mul_prim_add_env},
  // tube near big monster on level 5, Kirby64 [Raziel64]
  // (prim-env)*t0+1, (cmb-0)*shade+0  ** INC **
  {0xc153e4f0, cc_prim_sub_env_mul_t0_mul_shade},
  // paper mario. Added by Gonetz
  // (0-env)*t0+1, (prim-cmb)*t0+prim  **INC**
  {0xc15f6103, cc_env_sub_prim_mul_t0_add_prim},
  // HAL, smash bros
  // (0-0)*0+1
  {0xdfffdfff, cc_one},
  // arena, Pokemon Stadium 1, [Raziel64]
  // (0-0)*0+1, (cmb-0)*prim+0
  {0xdfffe3f0, cc_prim},
  // skis, Spacestation Silicon Valley. Added by Gonetz
  // (shade-0)*cmb+0, (t1-t0)*primlod+t0
  {0xe0f42d12, cc_t0_inter_t1_using_primlod},
  // paper mario. Added by Gonetz
  // (1-t1)*t0+0, (env-prim)*cmb+prim  ** INC **
  {0xe1266035, cc_env_sub_prim_mul__t0_mul_t1__add_prim},
  // ground, zelda 2. Added by Gonetz.
  // (t1-prim)*t0+0, (cmb-0)*shade+0
  {0xe132e4f0, cc__t1_sub_prim_mul_t0__mul_shade},
  // carmagedon
  // (shade-prim)*t0+0
  {0xe134e134, cc_shade_sub_prim_mul_t0},
  // skeleton, castlevania 2. Added by Gonetz
  // (1-prim)*t0+0, (cmb-0)*shade+0
  {0xe136e4f0, cc_t0_mul_1mprim_mul_shade},
  // Starshot logo. Added by Gonetz
  // (shade-env)*t0+0, (1-0)*cmb+cmb
  {0xe15400f6, cc_shade_sub_env_mul_t0},
  // Kirby morfing, smash bros. Added by Gonetz
  // (shade-env)*t0+0
  {0xe154e154, cc_shade_sub_env_mul_t0},
  // menu, PGA euro tour. Added by Gonetz
  // (1-env)*t0+0
  {0xe156e156, cc_one_sub_env_mul_t0},
  // paper mario. Added by Gonetz
  // (t0-0)*t0+0, (1-cmb)*prim+cmb
  {0xe1f10306, cc_one_sub_t0_mul_prim_add_t0},
  // F1 World Grand Prix. Added by Gonetz
  // (t0-0)*t0+0, (shade-prim)*cmb+prim
  {0xe1f16034, cc_shade_sub_prim_mul_t0_add_prim},
  // paper mario. Added by Gonetz
  // (t0-0)*t0+0, (env-prim)*cmb+prim
  {0xe1f16035, cc_env_sub_prim_mul_t0_add_prim},
  // sparkles, F1 World Grand Prix. Added by Gonetz
  // (t0-0)*t0+0, (1-prim)*cmb+prim
  {0xe1f16036, cc_one_sub_prim_mul_t0_add_prim},
  // rocket team basket, Pokemon Stadium 2
  // (t0-0)*t0+0, (cmb-prim)*shade+shade
  {0xe1f18430, cc__t0_mul_t0__sub_prim_mul_shade_add_shade},
  // Tony Hawk's Pro Skater. Added by Gonetz
  // (t0-0)*t0+0, (cmb-0)*t0+0
  {0xe1f1e1f0, cc_t0},
  // something in upper left corner, mario tennis
  // (t0-0)*t0+0
  {0xe1f1e1f1, cc_t0},
  // zelda 2. Added by Gonetz
  // (t0-0)*t0+0, (cmb-0)*prim+0
  {0xe1f1e3f0, cc_t0_mul_prim},
  // zelda 2 final movie. Added by Gonetz
  // (t0-0)*t0+0, (cmb-0)*shade+0
  {0xe1f1e4f0, cc_t0_mul_shade},
  // paper mario. Added by Gonetz
  // (t0-t1)*t0+1, (env-cmb)*prima+cmb  ** INC **
  {0xe1f20a05, cc_t1_mul_prima},
  // terrain, SCARS. Added by Gonetz
  // (t1-0)*t0+0, (env-prim)*cmb+prim
  {0xe1f26035, cc_env_sub_prim_mul__t0_mul_t1__add_prim},
  // Trees, Zelda 2
  // (t1-0)*t0+0, (cmb-0)*shade+prim
  {0xe1f264f0, cc__t0_mul_t1__mul_shade_add_prim},
  // terrain, SCARS. Added by Gonetz
  // (t1-0)*t0+0, (env-shade)*cmb+shade
  {0xe1f28045, cc_env_sub_shade_mul__t0_mul_t1__add_shade},
  // arena, Pokemon Stadium 2. Added by Gonetz
  // (t1-0)*t0+0, (cmb-prim)*shade+shade  ** INC **
  {0xe1f28430, cc__t0_mul_t1__sub_prim_mul_shade_add_shade},
  // arena, Pokemon Stadium 2
  // (t1-0)*t0+0, (cmb-env)*shade+shade
  {0xe1f28450, cc__t0_mul_t1__sub_env_mul_shade_add_shade},
  // Zelda 2, [Ogy]. Added by Gonetz
  // (t1-0)*t0+0, (cmb-prim)*env+shade  ** INC **
  {0xe1f28530, cc__t0_mul_t1__sub_prim_mul_env_add_shade},
  // pokemon attack, Pokemon Stadium 2. Added by Gonetz
  // (t1-0)*t0+0, (prim-env)*cmb+env
  {0xe1f2a053, cc_prim_sub_env_mul__t0_mul_t1__add_env},
  // paper mario. Added by Gonetz
  // (t1-0)*t0+0, (cmb-0)*prim+env
  {0xe1f2a3f0, cc__t0_mul_t1__mul_prim_add_env},
  // Sand, pokemon stadium 2
  // (t1-0)*t0+0, (cmb-prim)*shade+env  ** INC **
  {0xe1f2a430, cc__t0_mul_t1__mul_shade},
  // grass, Mission Impossible. Added by Gonetz
  // (t1-0)*t0+0, (shade-0)*cmb+0
  {0xe1f2e0f4, cc__t0_mul_t1__mul_shade},
  // flag, Monako Grand Prix
  // (t1-0)*t0+0
  {0xe1f2e1f2, cc_t0_mul_t1},
  // lighthouse's beam, zelda 2. Added by Gonetz
  // (t1-0)*t0+0, (cmb-0)*prim+0
  {0xe1f2e3f0, cc__t0_mul_t1__mul_prim},
  // Bottom of wings, pilotwings
  // (t1-0)*t0+0, (cmb-0)*shade+0
  {0xe1f2e4f0, cc__t0_mul_t1__mul_shade},
  // zelda 2. Added by Gonetz
  // (t1-0)*t0+0, (cmb-0)*prima+0
  {0xe1f2eaf0, cc__t0_mul_t1__mul_prima},
  // lava, Roadsters. Added by Gonetz
  // (prim-0)*t0+0, (1-prim)*t0+cmb
  {0xe1f30136, cc_t0},
  // sky, Pokemon Stadium 2. Added by Gonetz
  // (prim-0)*t0+0, (cmb-0)*shadea+env
  {0xe1f3abf0, cc_t0_mul_prim_mul_shadea_add_env},
  // cars, Indy Racing 2000. Added by Gonetz
  // (prim-0)*t0+0, (shade-0)*cmb+0
  {0xe1f3e0f4, cc_t0_mul_prim_mul_shade},
  // Sign shadows, zelda
  //z (prim-k5)*t0+cmb_a
  {0xe1f3e1f3, cc_t0_mul_prim},
  // Table, mace
  // (prim-0)*t0+0, (cmb-0)*shade+0
  {0xe1f3e4f0, cc_t0_mul_prim_mul_shade},
  // Gauntlet Legends intro
  // (prim-0)*t0+0, (cmb-0)*prima+0
  {0xe1f3eaf0, cc_t0_mul_prim_mul_prima},
  // walls, beetle adventure racing. Added by Gonetz
  // (shade-0)*t0+0, (prim-0)*t0+cmb
  {0xe1f401f3, cc_t0_mul_shade},
  // cars, ridge racer. Added by Gonetz
  // (shade-0)*t0+0, (prim-cmb)*cmb_a+cmb **INC**
  {0xe1f40703, cc_t0_mul_shade},
  // water block, Paper Mario. Added by Gonetz
  // (shade-0)*t0+0, (prim-env)*cmb+env
  {0xe1f4a053, cc_prim_sub_env_mul__t0_mul_shade__add_env},
  // a lot in TWINE. Added by Gonetz
  // (shade-0)*t0+0, (cmb-0)*prim+env
  {0xe1f4a3f0, cc_t0_mul_prim_mul_shade_add_env},
  // Xena. Added by Gonetz
  // (shade-0)*t0+0, (env-0)*cmb+0
  {0xe1f4e0f5, cc_t0_mul_env_mul_shade},
  // Starshot logo. Added by Gonetz
  // (shade-0)*t0+0, (1-0)*cmb+0
  {0xe1f4e0f6, cc_t0_mul_shade},
  // Duck Dodgers intro. Added by Gonetz
  // (shade-0)*t0+0
  {0xe1f4e1f4, cc_t0_mul_shade},
  // shadow, Mission Impossible. Added by Gonetz
  // (shade-0)*t0+0, (cmb-0)*prim+0
  {0xe1f4e3f0, cc_t0_mul_prim_mul_shade},
  // Tony Hawk's Pro Skater 3. Added by Gonetz
  // (env-0)*t0+0, (t1-0)*shade+cmb  ** INC **
  {0xe1f504f2, cc__t0_add_t1__mul_shade},
  // text, tonic trouble. Added by Gonetz
  // (env-0)*t0+0
  {0xe1f5e1f5, cc_t0_mul_env},
  // powder keg, zelda 2. Added by Gonetz
  // (env-0)*t0+0, (cmb-0)*shade+0
  {0xe1f5e4f0, cc_t0_mul_env_mul_shade},
  // Buss rush
  // (1-0)*t0+0, (0-cmb)*0+cmb
  {0xe1f61f0f, cc_t0},
  // water, Starshot. Added by Gonetz
  // (1-0)*t0+0, (1-0)*cmb+0
  {0xe1f6e0f6, cc_t0},
  // bomberman 64 [Ogy]
  // (1-0)*t0+0
  {0xe1f6e1f6, cc_t0},
  // Mermaid attack, Mystical Ninja
  // (noise-0)*t0+0
  {0xe1f7e1f7, cc_t0},
  // paper mario. Added by Gonetz * changed because of odd palette copy
  // (t0-0)*t1+0, (shade-env)*cmb+cmb  **INC** ?
  {0xe2f10054, cc_shade_sub_env_mul__t0_mul_t1__add__t0_mul_t1},
  // Duck Dodgers Starring Daffy Duck text background
  // (t0-0)*t1+0, (shade-cmb)*prim+cmb
  {0xe2f10304, cc_one_sub_prim_mul__t0_mul_t1__add__prim_mul_shade},
  // water, PGA European Tour
  // (t0-0)*t1+0, (env-cmb)*prim+cmb
  {0xe2f10305, cc_one_sub_prim_mul__t0_mul_t1__add__prim_mul_env},
  // Grass, mario golf
  // (t0-0)*t1+0, (cmb-t0)*cmb_a+t0
  {0xe2f12710, cc_t0_mul_t1},
  // xg2, Added by Gonetz
  // (t0-0)*t1+0, (env-prim)*cmb+prim
  {0xe2f16035, cc_env_sub_prim_mul__t0_mul_t1__add_prim},
  // poo, CBFD, Added by Gonetz
  // (t0-0)*t1+0, (cmb-env)*shade+prim ** INC **
  {0xe2f16450, cc__t0_mul_t1__mul_shade_add_prim},
  // the champion stage, Pokemon Stadium 2
  // (t0-0)*t1+0, (cmb-0)*shade+prim
  {0xe2f164f0, cc__t0_mul_t1__mul_shade_add_prim},
  // sky, xg2, Added by Gonetz
  // (t0-0)*t1+0, (cmb-prim)*cmb_a+prim
  {0xe2f16730, cc__t0_mul_t1__sub_prim_mul__t0t1a__add_prim },
  // Sin and Punishment, [scorpiove], Added by Gonetz
  // (t0-0)*t1+0, (env-prim)*cmb_a+prim
  {0xe2f16735, cc_env_sub_prim_mul__t0t1a__add_prim},
  // cianwood gym walls, pokemon stadium 2
  // (t0-0)*t1+0, (cmb-prim)*shade+shade
  {0xe2f18430, cc__t0_mul_t1__sub_prim_mul_shade_add_shade},
  // light, Ridge Racer. Added by Gonetz
  // (t0-0)*t1+0, (prim-env)*cmb+env
  {0xe2f1a053, cc_prim_sub_env_mul__t0_mul_t1__add_env},
  // Waterfall, duck dodgers. Added by Gonetz
  // (t0-0)*t1+0, (shade-env)*cmb+env
  {0xe2f1a054, cc_shade_sub_env_mul__t0_mul_t1__add_env},
  // Arena, Pokemon Stadium 2 ** INC **
  // (t0-0)*t1+0, (cmb-prim)*shade+env
  {0xe2f1a430, cc__t0_mul_t1__mul_shade_add_env},
  // bikes, xg2
  // (t0-0)*t1+0, (shade-0)*cmb+0
  {0xe2f1e0f4, cc__t0_mul_t1__mul_shade},
  // Sky background, xg2
  // (t0-0)*t1+0
  {0xe2f1e2f1, cc_t0_mul_t1},
  // statistics, Banjo 2. Added by Gonetz
  // (t0-0)*t1+0, (cmb-0)*prim+0
  {0xe2f1e3f0, cc__t0_mul_t1__mul_prim},
  // the champion stage, Pokemon Stadium 2
  // (t0-0)*t1+0, (cmb-prim)*shade+0
  {0xe2f1e430, cc__t0_mul_t1__sub_prim_mul_shade},
  // Water, pilotwings
  // (t0-0)*t1+0, (cmb-0)*shade+0
  {0xe2f1e4f0, cc__t0_mul_t1__mul_shade},
  //beetle adventure racing. A dded by Gonetz
  // (t0-0)*t1+0, (cmb-0)*env+0
  {0xe2f1e5f0, cc__t0_mul_t1__mul_env},
  //fall headwaters, zelda 2. Added by Gonetz
  // (t1-0)*t1+0, (cmb-0)*shade+0
  {0xe2f2e4f0, cc_t1_mul_shade},
  //text, Paper Mario
  // (prim-0)*t1+0
  {0xe2f3e2f3, cc_t1_mul_prim},
  //terrain, Beetle Adventure Racing. Added by Gonetz
  // (shade-0)*t1+0
  {0xe2f4e2f4, cc_t1_mul_shade},
  // Transfer pack, Pokemon Stadium 2
  // (noise-0)*t1+0, (prim-env)*cmb+env
  {0xe2f7a053, cc_prim_sub_env_mul_t1_add_env},
  // lens of truth, zelda 2 [Ogy]. Added by Gonetz
  // (1-t0)*prim+0
  {0xe316e316, cc_one_sub_t0_mul_prim},
  //C&C pointer
  //(shade-env)*prim+0
  {0xe354e354, cc_shade_sub_env_mul_prim},
  //C&C shadows
  //(1-env)*prim+0
  {0xe356e356, cc_one_sub_env_mul_prim},
  // Magnitude, pokemon stadium 2
  // (t0-0)*prim+0, (t0-0)*env+cmb
  {0xe3f105f1, cc_t0_mul__prim_add_env},
  // night vision, jet force gemini
  // (t0-0)*prim+0, (noise-0)*env+cmb
  {0xe3f105f7, cc_t0_mul_prim_add_env},
  // Smoke, diddy kong racing
  // (t0-0)*prim+0, (env-cmb)*env_alpha+cmb
  {0xe3f10c05, cc__t0_mul_prim__inter_env_using_enva},
  // battle menu, Paper Mario. Added by Gonetz
  // (t0-0)*prim+0, (t0-env)*env_alpha+cmb   ** INC **
  {0xe3f10c51, cc_t0_mul_prim},
  // stalactites, Beetle adventure Racing. Added by Gonetz
  // (t0-0)*prim+0, (cmb-shade)*t1_alpha+shade   ** INC **
  {0xe3f18940, cc_t0_mul_prim_add_shade },
  // ? in Jabu-Jabu's belly, submitted by gokuss4
  // {0xe4f1a053, (t0-0)*prim+0, (prim-env)*cmb+env
  {0xe3f1a053, cc_prim_sub_env_mul__t0_mul_prim__add_env},
  // kirby drill, kirby 64. Added by Gonetz
  // (t0-0)*prim+0, (cmb-env)*shade+env  **INC**
  {0xe3f1a450, cc_t0_mul_prim_mul_shade_add_env},
  // ? sign, zelda 2. Added by Gonetz
  // (t0-0)*prim+0, (cmb-0)*cmb+0 ** INC **
  {0xe3f1e0f0, cc_t0_mul_prim},
  // vehicle, Star Wars Ep.1 Racer, [Raziel64]. Added by Gonetz
  // (t0-0)*prim+0, (shade-0)*cmb+0
  {0xe3f1e0f4, cc_t0_mul_prim_mul_shade},
  // mini game, Pokemon Stadium 2
  // (t0-0)*prim+0, (1-0)*cmb+0
  {0xe3f1e0f6, cc_t0_mul_prim},
  // magic stuff, buck bumble. Added by Gonetz
  // (t0-0)*prim+0, (cmb-0)*prim+0
  {0xe3f1e3f0, cc_t0_mul_prim_mul_prim},
  // The mario face, mario
  //z (t0-k5)*prim+cmb_a
  {0xe3f1e3f1, cc_t0_mul_prim},
  // Butterflies at Jabu-Jabu's lake, zelda
  // (t0-0)*prim+0, (cmb-0)*shade+0
  {0xe3f1e4f0, cc_t0_mul_prim_mul_shade},
  // Sports shirt, Mia Soccer. Added by Gonetz
  // (t1-0)*prim+0, (1-t0)*t1+cmb **INC**
  //    {0xe3f20216, cc_t0_mul_prim_add_t1},
  {0xe3f20216, cc_shirt},
  // Sprites, Ogre Battle. Added by Gonetz
  // (t1-0)*prim+0
  {0xe3f2e3f2, cc_t1_mul_prim},
  // F1 World Grand Prix. Added by Gonetz
  // (t1-0)*prim+0, (cmb-0)*shade+0
  {0xe3f2e4f0, cc_t1_mul_prim_mul_shade},
  // intro background, bio freaks. Added by Gonetz
  // (prim-0)*prim+0
  {0xe3f3e3f3, cc_prim_mul_prim},
  // player, Ohzumou2
  // (shade-0)*prim+0, (env-cmb)*t0+cmb
  {0xe3f40105, cc_env_sub_primshade_mul_t0_add_primshade},
  // floor in pyramides, beetle adventure racing.
  // (shade-0)*prim+0, (t1-0)*cmb+0
  {0xe3f4e0f2, cc_t1_mul_prim_mul_shade},
  // Slingshot string, zelda
  // (shade-0)*prim+0
  {0xe3f4e3f4, cc_prim_mul_shade},
  // ?
  // (shade-0)*prim+0, (cmb-0)*shade+0 ** INC **
  {0xe3f4e4f0, cc_prim_mul_shade},
  // ???, zelda
  // (env-0)*prim+0, (0-0)*0+cmb
  {0xe3f5e3f5, cc_prim_mul_env},
  // Option selection, zelda
  //z (1-0)*prim+0
  {0xe3f6e3f6, cc_prim},
  // ranco monster, zelda 2. Added by Gonetz
  // (noise-0)*prim+0, (cmb-0)*prim_a+prim
  {0xe3f76af0, cc_prim_mul_prima_add_prim},
  // F-1_World_Grand_Prix_II, olivieryuyu
  // (noise-0)*prim+0, (0-cmb)*prim_a+shade
  {0xe3f78a0f, cc_shade_sub__prim_mul_prima},
  // zelda 2 [Ogy]. Added by Gonetz
  // (noise-0)*prim+0
  {0xe3f7e3f7, cc_prim},
  // Road rush. Added by Gonetz
  // (0-0)*prim+0  ** INC ** ?
  {0xe3ffe3ff, cc_prim},
  // Letter to Kafei's mom, zelda 2. Added by Gonetz
  // (0-0)*prim+0, (cmb-0)*shade+0
  {0xe3ffe4f0, cc_prim_mul_shade},
  // Jabu-Jabu's Belly, zelda. Added by Gonetz
  // (1-t0)*shade+0, (cmb-0)*prim+0
  {0xe416e3f0, cc_one_sub_t0_mul_prim_mul_shade},
  // Arena, Pokemon Stadium 2
  // (t0-prim)*shade+0
  {0xe431e431, cc_t0_sub_prim_mul_shade},
  // silver cave, pokemon stadium 2
  // (t0-env)*shade+0, (cmb-prim)*shade+prim
  {0xe4516430, cc__t0_sub_env_mul_shade__sub_prim_mul_shade_add_prim},
  // bomb mask, zelda 2. Added by Gonetz
  // (t0-env)*shade+0, (cmb-prim)*shade+shade ** INC **
  {0xe4518430, cc__t0_sub_env_mul_shade__sub_prim_mul_shade},
  // terrain, Top Gear Rally  2. Added by Gonetz
  // (t0-env)*shade+0
  {0xe451e451, cc_t0_sub_env_mul_shade},
  // closes, Nightmire Creatures
  // (1-env)*shade+0
  {0xe456e456, cc_one_sub_env_mul_shade},
  // water, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (t0-0)*shade+0, (cmb-cmb)*cmb+cmb
  {0xe4f10000, cc_t0_mul_shade},
  // Monster truck madness intro. Added by Gonetz
  // (t0-0)*shade+0, (1-0)*cmb+cmb    ** INC **
  {0xe4f100f6, cc_t0_mul_shade},
  // terrain, SCARS. Added by Gonetz
  // (t0-0)*shade+0, (prim-cmb)*t0+cmb  ** INC **
  {0xe4f10103, cc_t0_mul_shade},
  // Boomerang circle, zelda
  // (t0-0)*shade+0, (1-cmb)*t0+cmb
  {0xe4f10106, cc_t0_mul_shade},
  // THPS3.
  // (t0-0)*shade+0, (1-0)*t0+cmb
  {0xe4f101f6, cc_t0_mul_shade},
  // ???, WWF No Mercy [CpuMaster]
  // (t0-0)*shade+0, (env-cmb)*prim+cmb
  {0xe4f10305, cc_t0_mul_one_sub_prim_mul_shade_add_prim_mul_env},
  // magic bubble, zelda2. Added by Gonetz
  // (t0-0)*shade+0, (t1-0)*shade+cmb
  {0xe4f104f2, cc__t0_mul_shade__add__t1_mul_shade},
  // bike select, xg2. Added by Gonetz
  // (t0-0)*shade+0, (1-cmb)*env+cmb  ** INC **
  {0xe4f10506, cc_t0_mul_shade},
  // a bugs life [Ogy]
  // (t0-0)*shade+0, (cmb-0)*env+cmb
  //    {0xe4f105f0, cc_t0_mul_env_mul_shade},
  {0xe4f105f0, cc_t0_mul_shade},
  // Wall, quest64
  // (t0-0)*shade+0, (1-0)*env+cmb
  {0xe4f105f6, cc_t0_mul_shade_add_env},
  //lava, beetle adventure racing. Added by Gonetz
  // (t0-0)*shade+0, (prim-cmb)*cmb_a+cmb **INC**
  {0xe4f10703, cc_t0_mul_shade},
  // course map, Ridge Racer. Added by Gonetz
  // (t0-0)*shade+0, (prim-cmb)*prima+cmb **INC**
  {0xe4f10a03, cc_t0_mul_shade},
  // arena, custom robo. Added by Gonetz
  // (t0-0)*shade+0, (noise-cmb)*prima+cmb **INC**
  {0xe4f10a07, cc_t0_mul_shade},
  // arena, custom robo 2. Added by Gonetz
  // (t0-0)*shade+0, (0-cmb)*prima+cmb **INC**
  {0xe4f10a0f, cc_t0_mul_shade},
  //floor in a cave, Paper mario. Added by Gonetz
  // (t0-0)*shade+0, (cmb-prim)*prima+cmb **INC**
  {0xe4f10a30, cc_t0_mul_shade},
  //beetle adventure racing. Added by Gonetz
  // (t0-0)*shade+0, (t1-prim)*prima+cmb **INC**
  {0xe4f10a32, cc_t0_mul_shade},
  // Monster truck madness intro. Added by Gonetz
  // (t0-0)*shade+0, (shade-cmb)*shade_a+cmb    ** INC **
  {0xe4f10b04, cc_t0_mul_shade},
  // xg2 intro. Added by Gonetz
  // (t0-0)*shade+0, (1-cmb)*shade_a+cmb    ** INC **
  {0xe4f10b06, cc__t0_mul_shade__inter_one_using_shadea},
  // Link's bomb, smash bros
  // (t0-0)*shade+0, (env-cmb)*env_a+cmb    ** INC **
  {0xe4f10c05, cc__t0_mul_shade__inter_env_using_enva},
  // language selection, Extreme-G XG2 (E)
  // (t0-0)*shade+0, (1-cmb)*env_a+cmb
  {0xe4f10c06, cc__t0_mul_shade__inter_one_using_enva},
  // A Bugs Life, [Raziel64]
  // (t0-0)*shade+0, (cmb-0)*k5+cmb
  {0xe4f10ff0, cc_t0_mul_shade},
  // Bass Rush
  // (t0-0)*shade+0, (cmb-0)*0+cmb
  {0xe4f11f0f, cc_t0_mul_shade},
  // car, Top Gear Rally. Added by Gonetz
  // (t0-0)*shade+0, (cmb-t0)*t0a+t0  **INC**
  {0xe4f12810, cc_t0_mul_shade},
  // logo, SCARS. Added by Gonetz
  // (t0-0)*shade+0, (cmb-t0)*shadea+t0  **INC**
  {0xe4f12b10, cc__t0_mul_shade_mul_shadea__add__t1_mul_one_sub_shadea},
  // ? sign, Spiderman. Added by Gonetz
  // (t0-0)*shade+0, (0-0)*0+t1
  {0xe4f15fff, cc_t0_mul_shade},
  // Major League Baseball Featuring Ken Griffey Jr.
  // (t0-0)*shade+0, (1-0)*cmb+prim  ** INC **
  {0xe4f160f6, cc_t0_mul_shade_add_prim},
  // plants, CBFD. Added by Gonetz
  // (t0-0)*shade+0, (cmb-env)*shade+prim  ** INC **
  {0xe4f16450, cc_t0_sub_env_mul_shade_add_prim},
  // Kirby64. Added by Gonetz
  // (t0-0)*shade+0, (cmb-prim)*prima+prim
  {0xe4f16a30, cc_t0_mul_prima_mul_shade_add_prim_mul_one_sub_prima},
  // building shadow, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (t0-0)*shade+0, (0-0)*0+prim
  {0xe4f17fff, cc_prim},
  // tire trace, beetle adventure racing. Added by Gonetz
  // (t0-0)*shade+0, (env-cmb)*t1+shade  **INC**
  {0xe4f18205, cc_env_sub_shade_mul_t0_add_shade},
  // Gold Skulltula, zelda
  // (t0-0)*shade+0, (prim-env)*cmb+env
  {0xe4f1a053, cc_prim_sub_env_mul_t0_mul_shade_add_env},
  //    {0xe4f1a053, cc_t0_mul_prim_mul_shade},
  // fighters, GASP Fighters
  // (t0-0)*shade+0, (1-env)*cmb+env
  {0xe4f1a056, cc_t0_mul_one_sub_env_mul_shade_add_env},
  // Brian, quest64
  // (t0-0)*shade+0, (cmb-0)*prim+env
  {0xe4f1a3f0, cc_t0_mul_prim_mul_shade_add_env},
  // Objects in arena, pokemon stadium 2
  // (t0-0)*shade+0
  // (cmb-prim)*shade+env
  {0xe4f1a430, cc_t0_mul_shade},
  // Monster truck madness intro. Added by Gonetz
  // (t0-0)*shade+0, (cmb-env)*shadea+env  **INC**
  //    {0xe4f1ab50, cc_t0_mul_shade_add_env},
  {0xe4f1ab50, cc__t0_mul_shade__sub_env_mul_shadea_add_env},
  // Taz express. Added by Gonetz
  // (t0-0)*shade+0, (cmb-env)*enva+env  **INC**
  {0xe4f1ac50, cc_t0_mul_shade_add_env},
  // sky in doom. Added by Gonetz
  // (t0-0)*shade+0, (cmb-0)*primlod+env  **INC**
  {0xe4f1aef0, cc_t0_mul_shade_add_env},
  // fighters, GASP Fighters
  // (t0-0)*shade+0, (1-env)*cmb+0
  {0xe4f1e056, cc_t0_mul_one_sub_env_mul_shade},
  // walls, beetle adventure racing. Added by Gonetz
  // (t0-0)*shade+0, (t0-0)*cmb+0  **INC**
  {0xe4f1e0f1, cc_t0_mul_shade},
  // Link's face, zelda
  //z (t0-k5)*shade+cmb_a, (prim-k5)*cmb+cmb_a
  {0xe4f1e0f3, cc_t0_mul_prim_mul_shade},
  // Link's suit, zelda
  //z (t0-k5)*shade+cmb_a, (env-k5)*cmb+cmb_a
  {0xe4f1e0f5, cc_t0_mul_env_mul_shade},
  // Window, starfox
  //z (t0-k5)*shade+cmb_a, (cmb-k5)*prim+cmb_a
  {0xe4f1e3f0, cc_t0_mul_prim_mul_shade},
  // crystal, Doraemon 2
  //(t0-0)*shade+0, (t0-0)*prim+0
  {0xe4f1e3f1, cc_t0_mul_prim},
  // Characters, mace
  // (t0-0)*shade+0, (cmb-0)*shade+0
  {0xe4f1e4f0, cc_t0_mul_shade},
  // Super Mario 64 logo
  //z (t0-k5)*shade+cmb_a
  {0xe4f1e4f1, cc_t0_mul_shade},
  // Kokiri's hat, zelda
  // (t0-0)*shade+0, (cmb-0)*env+0
  {0xe4f1e5f0, cc_t0_mul_env_mul_shade},
  // Gauntlet Legends intro
  // (t0-0)*shade+0, (cmb-0)*scale+0
  {0xe4f1e6f0, cc_t0_mul_scale_mul_shade},
  // Something on a tree, Paper Mario. Added by Gonetz
  // (t0-0)*shade+0, (cmb-0)*prima+0
  {0xe4f1eaf0, cc_t0_mul_prima_mul_shade},
  // Course map, Ridge Racer. Added by Gonetz
  // (t0-0)*shade+0, (cmb-0)*shadea+0
  {0xe4f1ebf0, cc_t0_mul_shade_mul_shadea},
  // Dodongo skull's eyes, zelda
  // (t0-0)*shade+0, (cmb-0)*env_alpha+0
  {0xe4f1ecf0, cc_t0_mul_enva_mul_shade},
  // lava, beetle adventure racing. Added by Gonetz
  // (t1-0)*shade+0, (cmb-prim)*cmb_a+prim  **INC**
  {0xe4f26730, cc_prim_inter_t1_mul_shade_using_texa},
  // headlight, beetle adventure racing. Added by Gonetz
  // (t1-0)*shade+0, (env-cmb)*t0+shade  **INC**
  {0xe4f28105, cc_one_sub__t0_mul_t1__mul_shade},
  // bubble, Banjo-Kazooie. Added by Gonetz
  // (t1-0)*shade+0
  {0xe4f2e4f2, cc_t1_mul_shade},
  // water, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (prim-0)*shade+0, (cmb-cmb)*cmb+cmb
  {0xe4f30000, cc_prim_mul_shade},
  // lamp shadow, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (prim-0)*shade+0, (cmb-cmb)*t0+cmb
  {0xe4f30100, cc_prim_mul_shade},
  // Yoshi, mario golf
  // (prim-0)*shade+0, (env-cmb)*t0+cmb
  {0xe4f30105, cc_env_sub_primshade_mul_t0_add_primshade},
  //Spacestation Silicon Valley intro. Added by Gonetz
  // (prim-0)*shade+0, (env-cmb)*t1+cmb
  {0xe4f30205, cc_env_sub_primshade_mul_t1_add_primshade},
  // Tip of brian's hair, quest64
  // (prim-0)*shade+0, (1-0)*env+cmb
  {0xe4f305f6, cc_prim_mul_shade_add_env},
  // V8-2 menu
  // (prim-0)*shade+0, (env-cmb)*cmb_a+cmb
  {0xe4f30705, cc__prim_mul_shade__inter_env_using__prim_mul_shade_alpha},
  // Background circle, xg2
  // (prim-0)*shade+0, (1-cmb)*shade_a+cmb
  {0xe4f30b06, cc_prim_mul_shade},
  // circle, waverace. Added by Gonetz
  // (prim-0)*shade+0, (t0-cmb)*enva+cmb
  {0xe4f30c01, cc_t0_sub__prim_mul_shade__mul_enva_add__prim_mul_shade},
  // enemy hit, Glover2
  // (prim-0)*shade+0, (env-cmb)*enva+cmb
  {0xe4f30c05, cc__prim_mul_shade__inter_env_using_enva},
  // player, super bowling
  // (prim-0)*shade+0, (0-0)*k5+cmb
  {0xe4f30fff, cc_prim_mul_shade},
  //Lure, bass rush
  // (prim-0)*shade+0, (0-cmb)*0+cmb
  {0xe4f31f0f, cc_prim_mul_shade},
  // walls, beetle adventure racing. Added by Gonetz
  // (prim-0)*shade+0, (cmb-shade)*t1+shade   **INC**
  {0xe4f38240, cc__one_inter_prim_using_t1__mul_shade},
  // GASP fighters
  //(prim-0)*shade+0, (1-env)*cmb+0
  {0xe4f3e056, cc_prim_mul_one_sub_env_mul_shade},
  // Flag, mario kart
  //z (prim-k5)*shade+cmb_a
  {0xe4f3e4f3, cc_prim_mul_shade},
  // Characters, smash bros
  // (prim-0)*shade+0, (cmb-0)*env+0
  {0xe4f3e5f0, cc_prim_mul_env_mul_shade},
  // N64 logo, ridge race. Added by Gonetz
  // (shade-0)*shade+0, (prim-cmb)*prima+cmb **INC**
  {0xe4f40a03, cc_shade},
  // fighter, shield mode, bio freaks. Added by Gonetz
  // (shade-0)*shade+0
  {0xe4f4e4f4, cc_shade},
  // truck crush, Monster truck madness. Added by Gonetz
  // (env-0)*shade+0, (env-0)*shade+cmb
  {0xe4f504f5, cc_env_mul_shade},
  // Course map, Ridge Racer. Added by Gonetz
  // (env-0)*shade+0
  {0xe4f5e4f5, cc_env_mul_shade},
  // lava, beetle adventure racing
  // (1-0)*shade+0, (prim-cmb)*cmb_a+cmb
  {0xe4f60703, cc_prim_sub_shade_mul_shadea_add_shade},
  // the wings in the song of soaring cut-scene, zelda2 [Ogy]. Added by Gonetz
  // (1-0)*shade+0, (prim-0)*cmb+0
  {0xe4f6e0f3, cc_prim_mul_shade},
  // parts of vehicle, Star Wars Ep.I Racer. Added by Gonetz
  // (1-0)*shade+0, (cmb-0)*prim+0
  {0xe4f6e3f0, cc_prim_mul_shade},
  // Snowflakes???, mario kart. Boxer shadow (fb effect}, Knockout Kings 2000
  // (1-0)*shade+0, (1-0)*shade+0
  {0xe4f6e4f6, cc_one_mul_shade},
  // ???
  // (noise-0)*shade+0
  {0xe4f7e4f7, cc_shade},
  // quest64 [Ogy]
  // (prim-t0)*env+0, (0-0)*0+prim
  {0xe5137fff, cc_prim},
  // field, Mike Piazza's Strike Zone
  // (t0-prim)*env+0 ** INC **
  {0xe531e531, cc_t0_mul_env},
  // Mike Piazza's Strike Zone
  // (shade-prim)*env+0
  {0xe534e534, cc_shade_sub_prim_mul_env},
  // rope, CBFD. Added by Gonetz
  // (t0-0)*env+0, (1-env)*prim+cmb
  {0xe5f10356, cc_one_sub_env_mul_prim_add__t0_mul_env},
  // Bell, Pokemon Stadium 2. Added by Gonetz
  // (t0-0)*env+0, (shade-0)*prim+cmb
  {0xe5f103f4, cc_t0_mul_env_add_prim_mul_shade},
  // aerofighter's assault [Ogy]
  // (t0-0)*env+0, (1-t0)*shade+cmb
  {0xe5f10416, cc_t0_mul_env_add_1mt0_mul_shade},
  // foto, Armorines - Project S.W.A.R.M.  Added by Gonetz
  // (t0-0)*env+0, (noise-0)*scale+cmb
  {0xe5f106f7, cc_t0_mul_env},
  // Extreme G2, score.  Added by Gonetz
  // (t0-0)*env+0, (1-cmb)*enva+cmb  ** INC **
  {0xe5f10c06, cc_t0_mul_env},
  // many objects in Tonic Trouble
  // (t0-0)*env+0, (shade-0)*cmb+0
  {0xe5f1e0f4, cc_t0_mul_env_mul_shade},
  // Flying skull's eyes, zelda
  // (t0-0)*env+0, (cmb-0)*prim+0
  {0xe5f1e3f0, cc_t0_mul_prim_mul_env},
  // Rock spell, quest64
  // (t0-0)*env+0, (cmb-0)*shade+0
  {0xe5f1e4f0, cc_t0_mul_env_mul_shade},
  // Text, mario
  //z (t0-k5)*env+cmb_a
  {0xe5f1e5f1, cc_t0_mul_env},
  // kirby 64. Added by Gonetz
  // (prim-0)*env+0, (cmb-0)*shade+0
  {0xe5f3e4f0, cc_prim_mul_env_mul_shade},
  // wings, kirby 64. Added by Gonetz
  // (prim-0)*env+0
  {0xe5f3e5f3, cc_prim_mul_env},
  // Text, xg2
  // (shade-0)*env+0, (1-cmb)*env_a+cmb
  {0xe5f40c06, cc_env_mul_shade},
  // Text box, mario
  //z (shade-k5)*env+cmb_a
  {0xe5f4e5f4, cc_env_mul_shade},
  // bomberman 64 [Ogy]
  // (1-0)*env+0
  {0xe5f6e5f6, cc_env},
  // Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (1-t0)*scale+0
  {0xe616e616, cc_zero},
  // Gauntlet Legends intro. Added by Gonetz
  // (t0-0)*scale+0, (cmb-0)*shade+0
  {0xe6f1e4f0, cc_t0_mul_scale_mul_shade},
  // shadows, Taz express. Added by Gonetz
  // (t0-0)*scale+0
  {0xe6f1e6f1, cc_t0_mul_scale},
  // shadows, Knockout Kings 2000. Added by Gonetz
  // (shade-0)*scale+0
  {0xe6f4e6f4, cc_scale_mul_shade},
  // bomberman 64 2 [Ogy]. Added by Gonetz
  // (1-0)*scale+0
  {0xe6f6e6f6, cc_scale},
  // walls, beetle adventure racing. Added by Gonetz
  // (t1-0)*t0_a+0, (1-t1)*cmb+t1  **INC**
  {0xe8f24026, cc_t1},
  // house on rancho, zelda2. Added by Gonetz
  // (t1-0)*t0_a+0, (cmb-0)*prim+0
  {0xe8f2e3f0, cc__t0a_mul_t1__mul_prim},
  // zelda2 [Ogy]. Added by Gonetz
  // (t1-0)*t0_a+0, (cmb-0)*shade+0
  {0xe8f2e4f0, cc__t0a_mul_t1__mul_shade},
  // mini quiz, Pokemon Stadium 2
  // (prim-0)*t0_a+0, (cmb-t1)*primlod+t1 ** INC **
  {0xe8f34e20, cc_t0},
  // Major League Baseball Featuring Ken Griffey Jr.
  // (prim-0)*t0_a+0
  {0xe8f3e8f3, cc_t0a_mul_prim},
  // Top Gear Hyper-Bike
  // (1-0)*t0_a+0
  {0xe8f6e8f6, cc_t0a},
  // waterfall,  Dobutsu_no_Mori
  // (t0-0)*t1_a+0, (prim-env)*cmb+env
  {0xe9f1a053, cc_prim_sub_env_mul__t0_mul_t1a__add_env},
  // logo, Deadly Arts. Added by Gonetz
  // (t0-0)*t1_a+0, (cmb-0)*shade+0
  {0xe9f1e4f0, cc__t0_mul_t1a__mul_shade},
  // car, Roadsters. Added by Gonetz
  // (prim-t0)*prim_a+0, (prim-cmb)*shade+0  ** INC **
  {0xea13e403, cc_prim_sub__prim_sub_t0_mul_prima__mul_shade},
  // arena, Pokemon Stadium 2. Added by Gonetz
  // (1-t0)*prim_a+0, (0-prim)*cmb+prim  ** INC **
  {0xea16603f, cc_t0_mul_prim},
  // V8-2
  // (1-prim)*prim_a+0
  {0xea36ea36, cc_one_sub_prim_mul_prima},
  // match start, Mario Tennis. Added by Gonetz
  // (t0-0)*prim_a+0, (1-t0)*cmb+t0  ** INC **
  {0xeaf12016, cc_one_sub_t0_mul_prima_add_t0},
  // blast corps [Ogy]
  // (t0-0)*prim_a+0
  {0xeaf1eaf1, cc_t0_mul_prima},
  // final battle, CBFD. Added by Gonetz
  // (prim-0)*prim_a+0
  {0xeaf3eaf3, cc_prim_mul_prima},
  // flower's stalk, Paper Mario. Added by Gonetz
  // (shade-0)*prim_a+0
  {0xeaf4eaf4, cc_shade_mul_prima},
  // blast corps [Ogy]
  // (noise-0)*prim_a+0, (t1-0)*shade+cmb   ** INC **
  {0xeaf704f2, cc_t0_mul_shade_add_prima},
  // F1 World Grand Prix. Added by Gonetz
  // (noise-0)*prim_a+0, (t1-0)*env_a+cmb   ** INC **
  {0xeaf70cf2, cc_t1_mul_enva},
  // shadows, killer instinct gold
  // (0-0)*prim_a+0
  {0xeaffeaff, cc_zero},
  // background, killer instinct gold
  // (t0-prim)*shade_a+0
  {0xeb31eb31, cc_t0_sub_prim_mul_shadea},
  // ground, C&C
  // (t0-shade)*shade_a+0
  {0xeb41eb41, cc_t0_sub_shade_mul_shadea},
  // Wreslters, WWF No Mercy, [CpUMasteR]
  // (t0-0)*shade_alpha+0, (env-cmb)*prim+cmb
  {0xebf10305, cc_t0_mul_one_sub_prim_mul_shadea_add_prim_mul_env},
  // map, Pilot wings. Added by Gonetz
  // (t0-0)*shade_alpha+0, (1-cmb)*shade+cmb
  {0xebf10406, cc_one_sub_shade_mul__t0_mul_shadea__add_shade},
  // Indy Racing 2000. Added by Gonetz
  // (t0-0)*shade_alpha+0, (1-0)*shade+cmb
  {0xebf104f6, cc_t0_mul_shadea_add_shade},
  // logo, WCW-nWo Revenge
  // (t0-0)*shade_alpha+0, (cmb-0)*prim+0
  {0xebf1e3f0, cc_t0_mul_prim_mul_shadea},
  // sky, pilot wings
  // (t0-0)*shade_alpha+0, (1-cmb)*shade+0
  {0xebf1e406, cc_one_sub__t0_mul_shadea__mul_shade},
  // Wrestlers in Game, WWF No mercy [CpUMasteR]
  // (t0-0)*shade_alpha+0
  {0xebf1ebf1, cc_t0_mul_shadea},
  // flag, top gear overdrive
  // (prim-0)*shade_alpha+0
  {0xebf3ebf3, cc_prim_mul_shadea},
  // Ropes, WWF games
  // (shade-0)*shade_alpha+0, (env-cmb)*prim+cmb
  {0xebf40305, cc_shade_mul_shadea},
  // Ropes, WWF games
  // (shade-0)*shade_alpha+0
  {0xebf4ebf4, cc_shade_mul_shadea},
  // arena, custom robo 2
  // (noise-0)*shade_alpha+0
  {0xebf7ebf7, cc_shadea},
  // Baton Pass attack, Pokemon Stadium 2
  // (t0-env)*enva+0, (shade-0)*prim+cmb
  {0xec5103f4, cc__t0_sub_env_mul_enva__add_prim_mul_shade},
  // Bell, Pokemon Stadium 2. Added by Gonetz
  // (t0-0)*enva+0, (shade-0)*prim+cmb
  {0xecf103f4, cc_t0_mul_enva_add_prim_mul_shade},
  // blastcorps, unimp log. Added by Gonetz
  // (t0-0)*enva+0
  {0xecf1ecf1, cc_t0_mul_enva},
  // car, Top Gear Rally. Added by Gonetz
  // (env-0)*enva+0
  {0xecf5ecf5, cc_env_mul_enva},
  // Sand attack, pokemon Stadium (J)
  // (noise-0)*enva+0, (prim-env)*cmb+env
  {0xecf7a053, cc_prim_sub_env_mul_enva_add_env},
  // Walls of well through lens of truth, zelda
  // (prim-t0)*primlod+0      ** INC **
  {0xee13ee13, cc_t0},   // JUST t0 b/c the other combiner handles the subtraction
  // Pokemon attack, Pokemon Stadium 2
  // (noise-t0)*primlod+0, (1-env)*cmb+env ** INC **
  {0xee17a056, cc_env_inter_one_using__one_sub_t0_mul_primlod},
  // barrage attack, Pokemon Stadium 2
  // (t0-0)*primlod+0, (prim-0)*shade+cmb
  {0xeef104f3, cc__t0_mul_primlod__add__prim_mul_shade},
  // something on a flor in stone temple, zelda 2. Added by Gonetz
  // (t0-0)*primlod+0, (cmb-0)*prim+0
  {0xeef1e3f0, cc_t0_mul_primlod_mul_prim},
  // entrance to oceanside spider house, zelda 2. Added by Gonetz
  // (t0-0)*primlod+0, (cmb-0)*shade+0
  {0xeef1e4f0, cc_t0_mul_primlod_mul_shade},
  // Haze/(all powder status changers}, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (noise-0)*primlod+0, (prim-env)*cmb+env  ** INC **
  {0xeef7a053, cc_prim_sub_env_mul_primlod_add_env},
  // pokemon attack, Pokemon Stadium 2. Added by Gonetz
  // (noise-0)*primlod+0, (prim-cmb)*cmb+0  ** INC **
  {0xeef7e003, cc_zero},
  // Night trees, Monster truck madness. Added by Gonetz
  // (t0-0)*k5+0
  {0xeff1eff1, cc_t0_mul_k5},
  // submitted by gokuss4
  // (0-0)*0+0, (0-0)*0+prim
  {0xfffd5fe6, cc_prim},
  // intro, Bettle Adventure Racing, [Raziel64]
  // (0-0)*0+0, (0-0)*0+t0
  {0xffff3fff, cc_t0},
  // Conker's face, CBFD
  // (0-0)*0+0, (shade-env)*k5+prim
  {0xffff6f54, cc_shade_sub_env_mul_k5_add_prim},
  // Boost, Beetle Adventure Racing. Added by Gonetz
  // (0-0)*0+0, (0-0)*0+prim
  {0xffff7fff, cc_prim},
  // headlight, beetle adventure racing. Added by Gonetz
  // (0-0)*0+0, (0-0)*0+shade
  {0xffff9fff, cc_shade},
  // intro, Bettle Adventure Racing, [Raziel64]
  // (0-0)*0+0, (shade-env)*t1+env
  {0xffffa254, cc_shade_sub_env_mul_t1_add_env},
  // Fly Swooping in, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (0-0)*0+0, (1-env)*cmb_a+env
  {0xffffa756, cc_env},
  // Waterfall, Donkey Kong 64
  // (0-0)*0+0, (t0-0)*t1+0
  {0xffffe1f2, cc_t0_mul_t1},
  // Screen clear, banjo kazooie
  // (0-0)*0+0
  {0xffffffff, cc_zero},
  // { #CCEND }
};

static COMBINER alpha_cmb_list[] = {
  // { #ACSTART }
  //Tony Hawk's Pro Skater. Added by Gonetz
  // (0-0)*0+0
  {0x01ff01ff, ac_zero},
  //terminal, Spacestation Silicon Valley. Added by Gonetz
  // (0-0)*0+0, (0-0)*0+prim
  {0x01ff07ff, ac_prim},
  // kirby drill, kirby 64. Added by Gonetz
  // (0-0)*0+cmb, (0-0)*0+1
  {0x01ff0dff, ac_one},
  //chip in Spacestation Silicon Valley intro. Added by Gonetz
  // (0-0)*0+cmb, (prim-0)*shade+0
  {0x01ff0f3b, ac_prim_mul_shade},
  //Goldeneye, [Jeremy]. Added by Gonetz
  // (t0-t0)*lodf+t0, (cmb-0)*prim+0
  {0x02090ef8, ac_t0_mul_prim},
  // Indy Racing 2000. Added by Gonetz
  // (t1-t0)*lodf+t0, (env-cmb)*prim+cmb  ** INC **
  {0x020a00c5, ac_t0_inter_t1_using_primlod},
  // water, Spacestation Silicon Valley. Added by Gonetz
  // (t1-t0)*lodf+t0, (0-shade)*0+cmb
  {0x020a01e7, ac_t0_inter_t1_using_primlod},
  // Bridge, sf rush
  //z (t1-t0)*lodf+t0
  {0x020a020a, ac_t0_inter_t1_using_primlod},
  // explosion, body harvest. Added by Gonetz
  //(t1-t0)*lodf+t0, (0-0)*0+t0
  {0x020a03ff, ac_t0},
  // cars, PD intro. Added by Gonetz
  // (t1-t0)*lodf+t0, (cmb-0)*shade+prim
  {0x020a0738, ac__t0_inter_t1_using_primlod__mul_shade_add_prim},
  // Rocket Robot in Wheels intro
  //(t1-t0)*lodf+t0, (0-0)*0+prim
  {0x020a07ff, ac_prim},
  // Iguana background ground, turok
  // (t1-t0)*lodf+t0, (0-0)*0+shade
  {0x020a09ff, ac_shade},
  // Ground, monster truck madness
  // (t1-t0)*lodf+t0, (0-0)*0+env
  {0x020a0bff, ac_env},
  // Taz express. Added by Gonetz
  // (t1-t0)*lodf+t0, (0-0)*0+1
  {0x020a0dff, ac_one},
  // Mike Piazza's Strike Zone
  // (t1-t0)*lodf+t0, (cmb-0)*t0+0
  {0x020a0e78, ac_t0_inter_t1_using_primlod},
  // N64 logo, tetrisphere. Added by Gonetz
  // (t1-t0)*lodf+t0, (cmb-0)*prim+0
  {0x020a0ef8, ac__t0_inter_t1_using_primlod__mul_prim},
  // Ground, mace
  // (t1-t0)*lodf+t0, (cmb-0)*shade+0
  //    {0x020a0f38, ac_t0_mul_shade},
  {0x020a0f38, ac__t0_inter_t1_using_primlod__mul_shade},
  // blast corps [Ogy]
  // (t1-t0)*lodf+t0, (cmb-0)*env+0
  {0x020a0f78, ac__t0_inter_t1_using_primlod__mul_env},
  // blast corps [Ogy]
  // (t1-t0)*lodf+t0, (t0-0)*env+0
  {0x020a0f79, ac_t0_mul_env},
  // blast corps. Added by Gonetz
  // (t1-t0)*lodf+t0, (shade-0)*env+0
  {0x020a0f7c, ac_env_mul_shade},
  // field, Mike Piazza's Strike Zone
  // (t1-t0)*lodf+t0, (0-0)*0+0
  {0x020a0fff, ac_t0_inter_t1_using_primlod},
  // blast corps, unimp log. Added by Gonetz
  // (t1-t0)*t0+t0
  {0x024a024a, ac_t0_inter_t1_using_t0a},
  // zelda 2 [Ogy]. Added by Gonetz
  // (t1-t0)*t0+t0, (cmb-0)*prim+0    **INC**
  {0x024a0ef8, ac__t0_inter_t1_using_t0a__mul_prim},
  // text in a menu, Twisted_Edge_Extreme_Snowboarding [Razeil64]. Added by Gonetz
  // (prim-t0)*t0+t0    **INC**
  {0x024b024b, ac_t0},
  // enemy's shot, battle tanks 2
  // (env-prim)*t0+t0  **INC**
  {0x025d025d, ac_t0},
  //Bowser in final battle, Paper Mario. Added by Gonetz
  // (t1-env)*t0+t0, (cmb-env)*prim+0  ** INC **
  {0x026a0ee8, ac__t0_mul_t1__mul_prim},
  // paper mario. Added by Gonetz
  // (t1-env)*t0+t0, (cmb-0)*prim+0  ** INC **
  {0x026a0ef8, ac__t0_mul_t1__mul_prim},
  // V8-2
  // (prim-0)*t0+t0
  {0x027b027b, ac_t0_mul_prim_add_t0},
  // THPS3. Added by Gonetz
  // (0-0)*t0+t0
  {0x027f027f, ac_t0},
  // zelda 2. Added by Gonetz
  // (0-0)*t0+t0, (cmb-0)*prim+0
  {0x027f0ef8, ac_t0_mul_prim},
  // Spider Web attack, Pokemon Stadium 2.
  // (t1-t0)*t1+t0, (cmb-0)*prim+cmb
  {0x028a00f8, ac__t0_inter_t1_using_t1a__mul_prim_add__t0_inter_t1_using_t1a},
  // teleportation, Spacestation Silicon Valley. Added by Gonetz
  // (t1-t0)*t1+t0
  {0x028a028a, ac_t0_inter_t1_using_t1a},
  // mega shock, paper mario. Added by Gonetz
  // (t1-t0)*t1+t0, (cmb-0)*prim+0
  {0x028a0ef8, ac__t0_inter_t1_using_t1a__mul_prim},
  // mini game, Pokemon Stadium 2
  // (t1-t0)*t1+t0, (cmb-0)*shade+0
  {0x028a0f38, ac__t0_inter_t1_using_t1a__mul_shade},
  // Magnitude, pokemon stadium 2
  // (shade-t0)*t1+t0, (cmb-0)*shade+env
  {0x028c0b38, ac__t0_mul_t1__mul_shade},
  // paper mario. Added by Gonetz
  // (1-t0)*t1+t0, (t1-0)*prim+0  ** INC **
  {0x028e0efa, ac__one_sub_t0_mul_t1_add_t0__mul_prim},
  //    {0x028e0efa, ac_t1_mul_prim},
  // Spider Web attack, Pokemon Stadium 2.
  // (1-t0)*t1+t0, (cmb-0)*shade+0  ** INC **
  {0x028e0f38, ac__one_sub_t0_mul_t1_add_t0__mul_prim},
  // paper mario. Added by Gonetz
  // (t1-env)*t1+t0, (cmb-0)*shade+0
  {0x02aa0f38, ac__t0_inter_t1_using_enva__mul_shade},
  // Scary dead boss thing, zelda
  // (env-1)*t1+t0, (cmb-0)*prim+0  * MAY need t1_inter_t0 instead...
  {0x02b50ef8, ac__env_sub_one_mul_t1_add_t0__mul_prim},
  // first screen, castlevania. Added by Gonetz
  // (env-0)*t1+t0    **INC**
  {0x02bd02bd, ac_t0},
  // enemy's shot, battle tanks 2 [Flash]
  // (1-0)*t1+t0, (0-0)*0+env
  {0x02be0bff, ac_env},
  // battle tanks 2 [Ogy]
  // (1-0)*t1+t0, (0-0)*0+1
  {0x02be0dff, ac_one},
  // menu screen, Rayman2. Added by Gonetz
  // (1-0)*t1+t0, (cmb-0)*shade+0
  {0x02be0f38, ac__t0_add_t1__mul_shade},
  // Sky, zelda
  //z (t1-t0)*prim+t0
  {0x02ca02ca, ac_t0_inter_t1_using_prima},
  // F1 World Grand Prix. Added by Gonetz
  // (t1-t0)*prim+t0, (0-0)*0+1
  {0x02ca0dff, ac_t0_inter_t1_using_prima},
  // logo, PD. Added by Gonetz
  // (t1-t0)*prim+t0, (cmb-0)*shade+0
  {0x02ca0f38, ac__t0_inter_t1_using_prima__mul_shade},
  // battle tanks [Ogy]
  // (t1-t0)*prim+t0, (cmb-0)*env+0
  {0x02ca0f78, ac__t0_inter_t1_using_prima__mul_env},
  // logo, Deadly Arts. Added by Gonetz
  // (env-t0)*prim+t0
  {0x02cd02cd, ac_one_sub_prim_mul_t0_add__prim_mul_env},
  // intro, castlevania 2. Added by Gonetz
  // (1-t0)*prim+t0
  {0x02ce02ce, ac_one_sub_t0_mul_prim_add_t0},
  // intro, diddy kong racing. Added by Gonetz
  // (1-t0)*prim+t0, (cmb-0)*shade+0   **INC**
  {0x02ce0f38, ac_t0_mul_shade},
  // submitted by Scorpiove, mario party 1
  // (0-t0)*prim+t0
  {0x02cf02cf, ac_one_sub_prim_mul_t0},
  // Pokemon attack, pokemon Stadium (J)
  // (t1-t1)*prim+t0, (prim-0)*lod_f+env **INC**
  {0x02d20a3b, ac_env},
  // Ground, pokemon stadium 2
  // (t0-0)*prim+t0
  {0x02f902f9, ac_t0_mul_prim},
  // GASP Fighters
  // (t1-0)*prim+t0,  ** INC **
  {0x02fa02fa, ac_t1_mul_prim_add_t0},
  // foresight attack, Pokemon Stadium 2
  // (t1-0)*prim+t0, (cmb-env)*shade+0
  {0x02fa0f28, ac__t1_mul_prima_add_t0__sub_env_mul_shade},
  // Earthquake pokemon attack, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t1-0)*prim+t0, (cmb-0)*shade+0
  {0x02fa0f38, ac__t1_mul_prima_add_t0__mul_shade},
  // Paper Mario, fortune teller
  // (t1-0)*prim+t0, (cmb-0)*env+0
  {0x02fa0f78, ac__t1_mul_prima_add_t0__mul_env},
  // Hydro Pump Attack, Pokemon Stadium.
  // (shade-0)*prim+t0, (cmb-0)*shade+0
  {0x02fc0f38, ac__t0_add_prim_mul_shade__mul_shade},
  // map, Ogre Battle 64. Added by Gonetz
  // (1-0)*prim+t0
  {0x02fe02fe, ac_t0_add_prim},
  // borders, Tony Hawk's Pro Skater 2. Added by Gonetz
  // (t1-t0)*shade+t0 ** INC **
  {0x030a030a, ac_t0_inter_t1_using_shadea},
  // Mickey USA
  // (t1-t0)*shade+t0, (cmb-0)*prim+0 ** INC **
  {0x030a0ef8, ac__t0_inter_t1_using_shadea__mul_prim},
  // Rocket Robot in Wheels intro
  // (t1-t0)*shade+t0, (cmb-0)*env+0 ** INC **
  {0x030a0f78, ac__t0_inter_t1_using_shadea__mul_env},
  // water, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (1-t0)*shade+t0, (cmb-0)*shade+0 ** INC **
  {0x030e0f38, ac_t0_mul_shade},
  // sky, f-zero x
  // (0-t0)*shade+t0
  {0x030f030f, ac_one_sub_shade_mul_t0},
  // Deku tree from kokiri villiage, zelda
  //z (t1-t0)*env+t0, (t1-0)*primlod+cmb
  {0x034a01ba, ac_t0_inter_t1_using_enva},
  // Hearts, zelda
  //z (t1-t0)*env+t0
  {0x034a034a, ac_t0_inter_t1_using_enva},
  // Faries, zelda
  //z (t1-t0)*env+t0, (cmb-0)*prim+0
  {0x034a0ef8, ac__t0_inter_t1_using_enva__mul_prim},
  // zelda, waterfall. Added by Gonetz
  //z (t1-t0)*env+t0, (cmb-0)*shade+0
  {0x034a0f38, ac__t0_inter_t1_using_enva__mul_shade},
  // pokemon stadium 1. Added by Gonetz
  //(t1-t0)*env+t0, (cmb-0)*primlod+0
  {0x034a0fb8, ac__t0_inter_t1_using_enva__mul_primlod},
  // fruits, Yoshi Story. Added by Gonetz
  //(prim-t0)*env+t0
  {0x034b034b, ac_prim_sub_t0_mul_env_add_t0},
  // window, Rayman2. Added by Gonetz
  //(1-t0)*env+t0
  {0x034e034e, ac_one_sub_t0_mul_env_add_t0},
  // menu, PokemonStadium1, [Raziel64]
  //(1-t0)*env+t0, (cmb-0)*shade+0  ** INC **
  {0x034e0f38, ac_t0_mul_shade},
  // Ganon's sword swinging, zelda
  // (t0-t1)*env+t0, (cmb-0)*prim+0 ** INC **
  {0x03510ef8, ac__t0_sub_t1_mul_enva_add_t0__mul_prim},
  // Lave piranha atack, Paper Mario
  // (t1-prim)*env+t0, (0-cmb)*t1+0 ** INC **
  {0x035a0e87, ac_t0_mul_t1},
  // Reflected fire at kotake & koume's, zelda
  // (t0-1)*env+t0, (cmb-0)*prim+0  ** INC **
  {0x03710ef8, ac__t0_sub_one_mul_enva_add_t0__mul_prim},
  // thing that escapes from the well, zelda
  // (t1-1)*env+t0  ** INC **
  {0x03720372, ac_t1_sub_one_mul_enva_add_t0},
  // Sword charge, zelda
  // (t1-1)*env+t0, (cmb-0)*prim+0
  {0x03720ef8, ac__t1_sub_one_mul_enva_add_t0__mul_prim},
  // Gannon hitting the ground, zelda
  // (t1-1)*env+t0, (cmb-0)*shade+0 ** INC **
  {0x03720f38, ac__t1_sub_one_mul_enva_add_t0__mul_shade},
  // Tony Hawk's Pro Skater 3. Added by Gonetz
  // (t0-0)*env+t0
  {0x03790379, ac_t0_mul_env},
  // paper mario. Added by Gonetz
  // (t0-0)*env+t0, (cmb-0)*prim+0
  {0x03790ef8, ac_t0_mul_prim},
  // pads, Pokemon Stadium 2. Added by Gonetz
  // (t1-0)*env+t0, (cmb-0)*prim+env ** INC **
  {0x037a0af8, ac__t0_inter_t1_using_enva__mul_prim_add_env},
  // attack, Pokemon Stadium 2
  // (t1-0)*env+t0, (cmb-t0)*prim+0  ** INC **
  {0x037a0ec8, ac__t1_mul_enva_add_t0__mul_prim},
  // Ice arrow gfx, zelda
  // (t1-0)*env+t0, (cmb-0)*prim+0
  {0x037a0ef8, ac__t1_mul_enva_add_t0__mul_prim},
  // Scary face move, pokemon stadium 2
  // (t1-0)*env+t0, (cmb-prim)*shade+0
  {0x037a0f18, ac__t1_mul_enva_add_t0__sub_prim_mul_shade},
  // Saria's song, zelda
  // (t1-0)*env+t0, (cmb-0)*shade+0
  {0x037a0f38, ac__t1_mul_enva_add_t0__mul_shade},
  // eye drops bottle, zelda
  // (t0-t0)*prim_lodfrac+t0
  {0x03890389, ac_t0},
  // lighthouse's beam, zelda 2. Added by Gonetz
  // (t0-t0)*prim_lodfrac+t0, (cmb-0)*prim+0
  {0x03890ef8, ac_t0_mul_prim},
  // zelda 2. Added by Gonetz
  // (t1-t0)*primlod+t0, (cmb-0)*env+cmb  ** INC **
  {0x038a0178, ac__t0_inter_t1_using_primlod__mul_env_add__t0_inter_t1_using_primlod},
  // Enter name letter background, zelda
  //z (t1-t0)*primlod+t0
  {0x038a038a, ac_t0_inter_t1_using_primlod},
  // Sunny Day, Pokemon Stadium 2
  // (t1-t0)*primlod+t0, (cmb-0)*0+prim
  {0x038a07f8, ac_prim},
  //attack, Pokemon Stadium 2
  // (t1-t0)*primlod+t0, (cmb-env)*shade+shade ** INC **
  {0x038a0928, ac__t0_inter_t1_using_primlod__sub_env_mul_shade_add_shade},
  // blastcorps, unimp log. Added by Gonetz
  // (t1-t0)*primlod+t0, (0-0)*0+shade  **INC**?
  {0x038a09ff, ac_t0_inter_t1_using_primlod},
  // pokemon attack, pokemon monsters (J)
  // (t1-t0)*primlod+t0, (cmb-0)*prim+env
  {0x038a0af8, ac__t0_inter_t1_using_primlod__mul_prim_add_env},
  // sky, PGA European Tour
  // (t1-t0)*primlod+t0, (0-0)*0+1
  {0x038a0dff, ac_one},
  // Ice surrounding enemy, zelda
  // (t1-t0)*primlod+t0, (env-0)*lodf+0
  {0x038a0e3d, ac__t0_inter_t1_using_primlod__mul_env},
  // the bridge out side the mountain smithy shop, zelda 2 [Ogy]. Added by Gonetz
  // (t1-t0)*primlod+t0, (cmb-0)*t0+0
  {0x038a0e78, ac_t0_inter_t1_using_primlod},
  // zelda 2, [Ogy]. Added by Gonetz
  // (t1-t0)*primlod+t0, (cmb-0)*t1+0
  {0x038a0eb8, ac_t0_inter_t1_using_primlod},
  // Kirby's pool, smash bros
  // (t1-t0)*primlod+t0, (cmb-0)*prim+0
  {0x038a0ef8, ac__t0_inter_t1_using_primlod__mul_prim},
  // Samus stage fire, smash bros
  // (t1-t0)*primlod+t0, (cmb-0)*shade+0
  {0x038a0f38, ac__t0_inter_t1_using_primlod__mul_shade},
  // something about ice, zelda
  // (t1-t0)*primlod+t0, (cmb-0)*env+0
  {0x038a0f78, ac__t0_inter_t1_using_primlod__mul_env},
  // Blast Corps. Added by Gonetz
  // (t1-t0)*primlod+t0, (shade-0)*env+0
  {0x038a0f7c, ac_env_mul_shade},
  // goals, J. League Tactics Soccer. Added by Gonetz
  // (prim-t0)*primlod+t0  ** INC **
  {0x038b038b, ac_t0},
  // zelda 2, [Ogy]. Added by Gonetz
  // (t0-t1)*primlod+t0, (cmb-0)*prim+0
  {0x03910ef8, ac__t0_sub_t1_mul_primlod_add_t0__mul_prim},
  // a plane in the entrance to the mountain village zelda 2, [Ogy]. Added by Gonetz
  // (t1-t1)*primlod+t0, (cmb-0)*prim+0  ** INC **?
  {0x03920ef8, ac_t0_mul_prim},
  // zelda 2. Added by Gonetz
  // (t1-prim)*primlod+t0, (cmb-0)*prim+0  ** INC **
  {0x039a0ef8, ac__t1_sub_prim_mul_primlod_add_t0__mul_prim},
  // zelda 2. Added by Gonetz
  // (t1-shade)*primlod+t0, (cmb-0)*shade+0  ** INC **
  {0x03a20f38, ac__t1_sub_shade_mul_primlod_add_t0__mul_shade},
  // saffron city, Pokemon Stadium 2
  // (t1-1)*primlod+t0, (cmb-0)*0+cmb
  {0x03b201f8, ac_t1_sub_one_mul_primlod_add_t0},
  // Candle flame in ganon's castle, zelda
  // (t1-1)*primlod+t0
  {0x03b203b2, ac_t1_sub_one_mul_primlod_add_t0},
  // Fire, zelda
  //z (t1-1)*primlod+t0, (cmb-0)*prim+0   ** INC **
  {0x03b20ef8, ac__t1_sub_one_mul_primlod_add_t0__mul_prim},
  // explosion, zelda 2. Added by Gonetz
  // (t1-1)*primlod+t0, (t0-0)*prim+0   ** INC **
  {0x03b20ef9, ac_t0_mul_prim},
  // Din's fire, zelda
  // (t1-1)*prim_lodfrac+t0, (cmb-0)*shade+0  ** INC **
  {0x03b20f38, ac__t1_sub_one_mul_primlod_add_t0__mul_shade},
  // Fire cloud, zelda
  // (t1-1)*prim_lodfrac+t0, (cmb-0)*env+0  ** INC **
  {0x03b20f78, ac__t1_sub_one_mul_primlod_add_t0__mul_env},
  // zelda 2 [Ogy]. Added by Gonetz
  // (prim-1)*prim_lodfrac+t0, (cmb-0)*env+0  ** INC **
  {0x03b30f78, ac__prim_sub_one_mul_primlod_add_t0__mul_env},
  // fairy's spirit, zelda oot
  // (t0-0)*primlod+t0
  {0x03b903b9, ac_t0_mul_primlod_add_t0},
  // Scary face, pokemon stadium 2
  // (t0-0)*primlod+t0, (cmb-0)*prim+0
  {0x03b90ef8, ac_t0_mul_prim},
  // Magnitude attack, Pokemon Stadium 2
  // (t0-0)*primlod+t0, (cmb-0)*shade+0
  {0x03b90f38, ac__t0_mul_primlod_add_t0__mul_shade},
  // Leftovers Recovery, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t1-0)*prim_lodfrac+t0, (cmb-env)*prim+0  ** INC **
  {0x03ba0ee8, ac__t1_mul_primlod_add_t0__sub_env_mul_prim},
  // zelda 2 [Ogy]. Added by Gonetz
  // (t1-0)*prim_lodfrac+t0, (cmb-0)*prim+0
  {0x03ba0ef8, ac__t1_mul_primlod_add_t0__mul_prim},
  // Mega punch attack, Pokemon Stadium 2
  // (t1-0)*prim_lodfrac+t0, (cmb-prim)*shade+0
  {0x03ba0f18, ac__t1_mul_primlod_add_t0__sub_prim_mul_shade},
  // zelda 2 [Ogy]. Added by Gonetz
  // (t1-0)*prim_lodfrac+t0, (cmb-0)*shade+0
  {0x03ba0f38, ac__t1_mul_primlod_add_t0__mul_shade},
  // chuchu monsters, zelda 2 [Ogy]. Added by Gonetz
  // (t1-0)*prim_lodfrac+t0, (cmb-0)*env+0
  {0x03ba0f78, ac__t1_mul_primlod_add_t0__mul_env},
  // Scary face, pokemon stadium 2
  // (env-0)*primlod+t0, (cmb-0)*prim+0
  {0x03bd0ef8, ac_t0_mul_prim},
  // ground, zelda 2. Added by Gonetz
  // (t1-t0)*0+t0, (cmb-0)*0+cmb
  {0x03ca01f8, ac_t0},
  // zelda 2. Added by Gonetz
  // (t1-t0)*0+t0, (cmb-0)*prim+0
  {0x03ca0ef8, ac_t0_mul_prim},
  // smoke in a night, zelda 2. Added by Gonetz
  // (t1-t0)*0+t0, (cmb-0)*shade+0
  {0x03ca0f38, ac_t0_mul_shade},
  //the ice plane out side the mountain smithy shop, zelda 2 [Ogy]. Added by Gonetz
  //(t1-1)*0+t0, (cmb-0)*env+0
  {0x03f20f78, ac_t0_mul_env},
  //something on level 5, Kirby64 [Raziel64]
  //(t0-0)*0+t0
  {0x03f903f9, ac_t0},
  //spider house, zelda 2 [Ogy]. Added by Gonetz
  //(t0-0)*0+t0, (cmb-0)*prim+0
  {0x03f90ef8, ac_t0_mul_prim},
  //Darmani's fire spin, zelda 2 [Ogy]. Added by Gonetz
  //(t1-0)*0+t0, (cmb-0)*prim+0
  {0x03fa0ef8, ac_t0_mul_prim},
  // headlight, beetle adventure racing. Added by Gonetz
  //(1-0)*0+t0
  {0x03fe03fe, ac_t0},
  // player, super bowling
  // (0-0)*0+t0,
  {0x03ff0000, ac_t0},
  // Ghost's lantern, zelda
  // (0-0)*0+t0, (t1-0)*prim_lod+cmb
  {0x03ff01ba, ac_t1_mul_primlod_add_t0},
  // Hand cursor, mario
  //z (0-0)*0+t0
  {0x03ff03ff, ac_t0},
  // Taz express. Added by Gonetz
  // (0-0)*0+t0, (0-0)*0+t1
  {0x03ff05ff, ac_t0},
  // powder keg, zelda2. Added by Gonetz
  // (0-0)*0+t0, (0-0)*0+prim
  {0x03ff07ff, ac_t0},
  // water, Spacestation Silicon Valley. Added by Gonetz
  // (0-0)*0+t0, (0-0)*0+shade
  {0x03ff09ff, ac_t0},
  // Characters, Ogre Battle. Added by Gonetz.
  // (0-0)*0+t0, (cmb-0)*prim+env
  {0x03ff0af8, ac_t0_mul_prim_add_env},
  // Monster truck madness intro. Added by Gonetz
  // (0-0)*0+t0, (0-0)*0+env
  {0x03ff0bff, ac_t0},
  // Battlezone
  // (0-0)*0+t0, (0-0)*0+1
  {0x03ff0dff, ac_t0},
  // Zoras, zelda
  // (0-0)*0+t0, (env-0)*lodf+0
  {0x03ff0e3d, ac_env},
  // logo, v-rally 99
  // (0-0)*0+t0, (prim-0)*t0+0
  {0x03ff0e7b, ac_t0_mul_prim},
  // intro, WWF-War Zone
  // (0-0)*0+t0, (env-0)*t0+0
  {0x03ff0e7d, ac_t0_mul_env},
  // Window, starfox
  //z (0-0)*0+t0, (cmb-0)*prim+0
  {0x03ff0ef8, ac_t0_mul_prim},
  //beetle adventure racing. Added by Gonetz
  // (0-0)*0+t0, (cmb-0)*shade+0
  {0x03ff0f38, ac_t0_mul_shade},
  //  Wonder Project J2 logo. Added by Gonetz
  // (0-0)*0+t0, (t0-0)*shade+0
  {0x03ff0f39, ac_t0_mul_shade},
  // Saria's suit, zelda
  // (0-0)*0+t0, (cmb-0)*env+0
  {0x03ff0f78, ac_t0_mul_env},
  // Pokemon Stadium 2, [Jeremy]. Added by Gonetz
  // (0-0)*0+t0, (cmb-0)*primlod+0
  {0x03ff0fb8, ac_t0_mul_primlod},
  // Tony Hawk's Pro Skater. Added by Gonetz
  // (0-0)*0+t0, (0-0)*0+0
  {0x03ff0fff, ac_zero},
  // Spider Web attack, Pokemon Stadium 2.
  // (t0-t1)*t0+t1, (cmb-0)*prim+cmb   **INC**
  {0x045100f8, ac__t1_inter_t0_using_t0a__mul_prim_add__t1_inter_t0_using_t0a},
  // Powered Star Beam, Paper Mario. Added by Gonetz
  // (t0-t1)*t0+t1, (cmb-0)*prim+0   **INC**
  {0x04510ef8, ac__t1_inter_t0_using_t0a__mul_prim},
  // Deadly Arts logo. Added by Gonetz
  // (1-0)*t0+t1, (1-0)*prim+cmb
  {0x047e00fe, ac__t0_add_t1__add_prim},
  // Spiderman. Added by Gonetz
  // (1-0)*t0+t1
  {0x047e047e, ac_t0_add_t1},
  // water, Dobutsu no Mori. Added by Gonetz
  // (1-0)*t0+t1, (cmb-0)*primlod+prim
  {0x047e07b8, ac__t0_add_t1__mul_primlod_add_prim},
  // paper mario. Added by Gonetz
  // (1-t0)*t1+t1, (cmb-0)*t1+0  **INC**
  {0x048e0eb8, ac_t0_mul_t1},
  // Pokemon Stadium 2. Added by Gonetz
  // (t0-prim)*t1+t1, (cmb-0)*shade+0  **INC**
  {0x04990f38, ac_t1_mul_shade},
  // waterfall, Dobutsu no Mori. Added by Gonetz
  // (t0-0)*t1+t1
  {0x04b904b9, ac_t0_mul_t1_add_t1},
  // light, Dobutsu no Mori. Added by Gonetz
  // (t0-0)*t1+t1, (cmb-0)*primlod+0  ** INC **
  {0x04b90fb8, ac__t0_add_t1__mul_primlod},
  // lava, beetle adventure racing
  // (t1-0)*t1+t1, (cmb-0)*shade+0  ** INC **
  {0x04ba0f38, ac__t1_mul_t1_add_t1__mul_shade},
  // wheels, F1 World Grand Prix. Added by Gonetz
  // (t0-t1)*prim+t1
  {0x04d104d1, ac_t1_inter_t0_using_prima},
  // intro, castlevania 2. Added by Gonetz
  // (t0-t1)*prim+t1, (cmb-0)*shade+0
  {0x04d10f38, ac__t1_inter_t0_using_prima__mul_shade},
  // flame, castlevania 2. Added by Gonetz
  // (t0-t1)*prim+t1, (cmb-0)*env+0
  {0x04d10f78, ac__t1_inter_t0_using_prima__mul_env},
  // walls, beetle adventure racing. Added by Gonetz
  // (t0-0)*prim+t1  **INC**
  {0x04f904f9, ac_t0_mul_prim},
  // Reflect pokemon attack, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t0-0)*prim+t1, (cmb-0)*prim+env  **INC**
  {0x04f90af8, ac__t0_add_t1__mul_prim_add_env},
  // Psychic pokemon attack, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t0-0)*prim+t1, (cmb-0)*shade+0  **INC**
  {0x04f90f38, ac__t0_add_t1__mul_shade},
  // Rayman2. Added by Gonetz
  // (0-0)*shade+t1, (cmb-0)*env+0
  {0x053f0f78, ac_t1_mul_env},
  // Ground at kotake & koume, zelda
  // (t1-t0)*env+t1, (cmb-0)*prim+0 ** INC **
  {0x054a0ef8, ac__t1_sub_t0_mul_enva_add_t1__mul_prim},
  // Tony Hawk's Pro Skater. Added by Gonetz
  // (t0-t1)*env+t1
  {0x05510551, ac_t1_inter_t0_using_enva},
  // Shiek's disappearance, zelda
  // (t0-1)*env+t1
  {0x05710571, ac_t0_sub_one_mul_enva_add_t1},
  // Kotake or koume's magic poof, zelda
  // (t0-1)*env+t1, (cmb-0)*prim+0  ** INC **
  {0x05710ef8, ac__t0_sub_one_mul_enva_add_t1__mul_prim},
  // Gauntlet Legends intro
  // (t0-0)*env+t1, (cmb-0)*prim+0  ** INC **
  {0x05790ef8, ac__t0_add_t1__mul_prim},
  // Zelda opening door, zelda
  // (t0-0)*env+t1, (cmb-0)*shade+0
  {0x05790f38, ac_t1_mul_shade},
  // paper mario. Added by Gonetz
  // (t1-0)*env+t1, (cmb-0)*prim+0  ** INC **
  {0x057a0ef8, ac_t1_mul_prim},
  // pokemon attack, Pokemon Stadium 2. Added by Gonetz
  // (t0-t1)*prim_lod+t1, (cmb-0)*prim+0
  {0x05910ef8, ac__t1_inter_t0_using_primlod__mul_prim},
  // Skulltula coin, zelda
  // (t0-1)*primlod+t1              ** INC **
  {0x05b105b1, ac_t0_mul_t1},
  // Bell, Pokemon Stadium 2. Added by Gonetz
  // (t0-0)*primlod+t1, (cmb-env)*prim  ** INC **
  {0x05b90ee8, ac__t0_add_t1__mul_prim},
  // intro, Aidyn Chronicles. Added by Gonetz
  // (0-cmb)*0+t1, (t1-1)*0+cmb
  {0x05c701f2, ac_t1},
  // zelda 2 [Ogy]. Added by Gonetz
  // (t1-t0)*0+t1, (cmb-0)*prim+0
  {0x05ca0ef8, ac_t1_mul_prim},
  // beaver's river, zelda 2. Added by Gonetz
  // (t1-0)*0+t1, (cmb-0)*prim+0
  {0x05fa0ef8, ac_t1_mul_prim},
  // Arena, pokemon stadium 2
  // (0-0)*0+t1, (0-0)*t0+cmb
  {0x05ff007f, ac_t1},
  // Ogre Battle, unimp log. Added by Gonetz
  // (0-0)*0+t1, (0-0)*0+cmb
  {0x05ff05ff, ac_t1},
  // lullaby, Paper Mario. Added by Gonetz
  // (0-0)*0+t1, (cmb-0)*prim+0
  {0x05ff0ef8, ac_t1_mul_prim},
  // aerofighter's assault [Ogy]
  // (0-0)*0+t1, (cmb-0)*shade+0
  {0x05ff0f38, ac_t1_mul_shade},
  // magic fist, Rayman2. Added by Gonetz
  // (0-0)*0+t1, (cmb-0)*env+0
  {0x05ff0f78, ac_t1_mul_env},
  // Pokemon selection background, Pokemon stadium 2
  // (env-prim)*t0+prim
  {0x065d065d, ac_env_sub_prim_mul_t0_add_prim},
  // text background, Ganbare Goemon - Mononoke Sugoroku
  // (1-prim)*t0+prim
  {0x065e065e, ac_one_sub_prim_mul_t0_add_prim},
  // shadows, star wars: ep1 racer
  // (0-prim)*t0+prim
  {0x065f065f, ac_zero_sub_prim_mul_t0_add_prim},
  // lava, beetle adventure racing
  // (0-1)*t0+prim, (cmb-0)*prim+0  ** INC **
  {0x06770ef8, ac_t0_mul_prim},
  // menu, Ganbare Goemon - Mononoke Sugoroku
  // (t0-0)*t0+prim
  {0x06790679, ac_t0_add_prim},
  // Water, pokemon stadium 2
  // (t1-0)*t0+prim
  {0x067a067a, ac_t0_mul_t1_add_prim},
  // Smackdown Mall Menu, WWF No Mercy
  // (shade-0)*t0+prim
  {0x067c067c, ac_t0_mul_shade_add_prim},
  // flag, Top Gear Rally  2. Added by Gonetz
  // (env-0)*t0+prim
  {0x067d067d, ac_t0_mul_env_add_prim},
  // Mario Tennis. Added by Gonetz
  // (1-0)*t0+prim
  {0x067e067e, ac_t0_add_prim},
  // sky, PGA European Tour
  // (t0-0)*t1+prim
  {0x06b906b9, ac_t0_mul_t1_add_prim},
  // lava, beetle adventure racing
  // (t0-0)*t1+prim, (0-0)*0+1 **INC**?
  {0x06b90dff, ac_one},
  // Pokemon Stadium 2, [Jeremy]. Added by Gonetz
  // (prim-0)*t1+prim
  {0x06bb06bb, ac_t1_mul_prim_add_prim},
  // pokemon psyattack, Pokemon Stadium 2. Added by Gonetz
  // (1-0)*t1+prim, (cmb-0)*env+0
  {0x06be0f78, ac_t1_add_prim_mul_env},
  // Rush2 2. Added by Gonetz
  // (prim-prim)*prim+prim
  {0x06db06db, ac_prim},
  //Spacestation Silicon Valley intro. Added by Gonetz
  // (t1-prim)*shade+prim
  //    {0x071a071a, ac_t1_mul_shade},
  {0x071a071a, ac_t1_sub_prim_mul_shade_add_prim},
  //KI logos. Added by Gonetz
  // (env-prim)*shade+prim
  {0x071d071d, ac_env_sub_prim_mul_shade_add_prim},
  // Deadly Arts, arena. Added by Gonetz
  // (1-0)*shade+prim
  {0x073e073e, ac_prim_add_shade},
  // Phantom Gannon's portal, zelda
  // (t1-t0)*env+prim, (cmb-0)*shade+0  ** INC **
  {0x074a0f38, ac__t0_mul_t1__mul_prim_mul_shade},
  // Road rush. Added by Gonetz
  // (t0-0)*env+prim
  {0x07790779, ac_t0_mul_env_add_prim},
  // arena, Pokemon Stadium 2
  // (shade-t0)*primlod+prim, (cmb-t0)*shade ** INC **
  {0x078c0f08, ac_shade_sub_t0_mul_primlod_add_prim},
  // telescope, zelda 2. Added by Gonetz
  // (1-t0)*primlod+prim
  {0x078e078e, ac_one_sub_t0_mul_primlod_add_prim},
  // zelda 2 [Ogy]. Added by Gonetz
  // (t0-t1)*primlod+prim, (cmb-0)*t0+0
  {0x07910e78, ac_t0_inter_t1_using_primlod},
  // Dobutsu no Mori. Added by Gonetz
  // (t0-0)*primlod+prim
  {0x07b907b9, ac_t0_mul_primlod_add_prim},
  // Lock-On attack, Pokemon Stadium 2
  // (t1-t0)*0+prim, (cmb-0)*0+cmb
  {0x07ca01f8, ac_prim},
  // water, DK64
  // (0-0)*0+0, (0-t1)*0+prim
  {0x07d707d7, ac_prim},
  // Menu, megaman
  // (1-0)*0+prim
  {0x07fe07fe, ac_prim},
  // super bowling
  //(0-0)*0+prim,
  {0x07ff0000, ac_prim},
  // menu, Ganbare Goemon - Mononoke Sugoroku
  // (0-0)*0+prim, (0-0)*0+t0
  {0x07ff03ff, ac_t0},
  // Intro background, starfox
  //z (0-0)*0+prim
  {0x07ff07ff, ac_prim},
  // velva boss, JFG
  //(0-0)*0+prim, (0-0)*0+env
  {0x07ff0bff, ac_env},
  // gem, castlevania 2. Added by Gonetz
  // (0-0)*0+prim, (cmb-0)*t0+0
  {0x07ff0e78, ac_t0_mul_prim},
  // text, Tony Hawk's Pro Skater. Added by Gonetz
  // (0-0)*0+prim, (cmb-0)*t1+0
  {0x07ff0eb8, ac_t1_mul_prim}, //weird, but implementing this makes text unreadable
  // zelda 2. Added by Gonetz
  // (0-0)*0+prim, (cmb-0)*prim+0
  {0x07ff0ef8, ac_prim_mul_prim},
  // explosion, Blast Corps. Added by Gonetz
  // (0-0)*0+prim, (t0-0)*prim+0
  {0x07ff0ef9, ac_t0_mul_prim},
  // zelda 2, [Ogy]. Added by Gonetz
  // (0-0)*0+prim, (cmb-0)*shade+0
  {0x07ff0f38, ac_prim_mul_shade},
  // Fox's ears and arms, smash bros
  // (0-0)*0+prim, (cmb-0)*env+0
  {0x07ff0f78, ac_prim_mul_env},
  // monsters, Pokemon Stadium. Added by Gonetz
  // (0-0)*0+prim, (cmb-0)*primlod+0
  {0x07ff0fb8, ac_prim_mul_primlod},
  // Hydro Pump Attack, Pokemon Stadium.
  // (1-t1)*t0+shade, (cmb-prim)*shade+0
  {0x08560f18, ac__one_sub_t1_mul_t0_add_shade__sub_prim_mul_shade},
  // focus, Paper Mario. Added by Gonetz
  //(t0-shade)*t0+shade, (cmb-0)*prim+0  * INC **
  {0x08610ef8, ac_t0_mul_prim},
  // Mario's head, mario //Added by Gonetz
  //(prim-shade)*t0+shade
  {0x08630863, ac_prim_sub_shade_mul_t0_add_shade},
  // Fissure attack, pokemon stadium 2
  //(t1-t0)*prim+shade, (cmb-0)*shade+0
  {0x08ca0f38, ac__t1_sub_t0_mul_prim_add_shade__mul_shade},
  // Earthquake pokemon attack, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  //(t0-t1)*prim+shade, (cmb-0)*shade+0 ** INC **
  {0x08d10f38, ac__t0_sub_t1_mul_prim_add_shade__mul_shade},
  // ?
  //(t0-shade)*prim+shade
  {0x08e108e1, ac_t0_mul_prim_add_shade_mul_one_minus_prim},
  // Paper Mario
  // (t0-prim)*shade+shade, (cmb-0)*env+0
  {0x09190f78, ac__t0_sub_prim_mul_shade_add_shade__mul_env},
  // pads, Pokemon Stadium 2. Added by Gonetz
  // (0-t0)*env+shade, (cmb-0)*prim+0  ** INC **
  {0x094f0ef8, ac_one_sub_t0_mul_prim_mul_shade},
  // sun rays, Pokemon Stadium 2.
  // (shade-0)*env+shade, (cmb-0)*prim+0
  {0x097c0ef8, ac_one_plus_env_mul_prim_mul_shade},
  // attack, Pokemon Stadium 2.
  // (t0-0)*primlod+shade, (cmb-0)*shade+0
  {0x09b90f38, ac__t0_mul_primlod_add_shade__mul_shade},
  // Huge turtle appearance, zelda 2. Added by Gonetz
  // (t1-0)*primlod+shade, (cmb-0)*shade+0  ** INC **
  {0x09ba0f38, ac__t1_mul_primlod_add_shade__mul_shade},
  // roof, Kirby 64. Added by Gonetz
  // (t0-0)*0+shade
  {0x09f909f9, ac_shade},
  // water, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (0-0)*0+shade, (cmb-cmb)*lodf+cmb
  {0x09ff0000, ac_shade},
  // water temple, zelda 2. Added by Gonetz
  // (0-0)*0+shade, (cmb-0)*prim+cmb
  {0x09ff00f8, ac_prim_mul_shade_add_shade},
  // damaged car, SCARS. Added by Gonetz
  // (0-0)*0+shade, (t0-t1)*primlod+prim  ** INC **
  {0x09ff0791, ac_t0_mul_primlod_add_prim},
  // Hyrule castle gate, zelda
  //z (0-0)*0+shade, (0-0)*0+prim
  {0x09ff07ff, ac_prim},
  // Super Mario 64 logo
  //z (0-0)*0+shade
  {0x09ff09ff, ac_shade},
  // terrain, SCARS. Added by Gonetz
  // (0-0)*0+shade, (0-0)*0+1
  {0x09ff0dff, ac_one},
  // terrain, SCARS. Added by Gonetz
  // (0-0)*0+shade, (t0-0)*t1+0
  {0x09ff0eb9, ac_t0_mul_t1},
  // N64 logo, Aidyn Chronicles. Added by Gonetz
  // (0-0)*0+shade, (cmb-0)*prim+0
  {0x09ff0ef8, ac_prim_mul_shade},
  // birds?, custom robo. Added by Gonetz
  // (0-0)*0+shade, (cmb-0)*shade+0
  {0x09ff0f38, ac_shade},
  // sky, Glover, [Raziel64]. Added by Gonetz
  // (0-0)*0+shade, (t0-0)*shade+0
  {0x09ff0f39, ac_t0_mul_shade},
  // Hand, smash bros
  // (0-0)*0+shade, (cmb-0)*env+0
  {0x09ff0f78, ac_env_mul_shade},
  // Conker's helicopter tail, CBFD
  // (0-0)*0+shade, (shade-0)*env+0
  {0x09ff0f7c, ac_env_mul_shade},
  // menu, PokemonStadium1, [Raziel64]
  // (0-0)*0+shade, (cmb-0)*primlod+0
  {0x09ff0fb8, ac_primlod_mul_shade},
  // Link's sword slashing, smash bros
  // (prim-env)*t0+env
  {0x0a6b0a6b, ac_prim_sub_env_mul_t0_add_env},
  // Reflected beam at kotake & koume's, zelda
  // (prim-env)*t0+env, (cmb-0)*prim+0  ** INC **
  {0x0a6b0ef8, ac_t0_mul_prim},
  // teleporter, Spacestation Silicon Valley. Added by Gonetz
  // (prim-env)*t0+env, (cmb-0)*shade+0  ** INC **
  {0x0a6b0f38, ac_t0_mul_shade},
  // Ridge Racer, unimp log. Added by Gonetz
  // (prim-env)*t0+env, (cmb-0)*primlod+0
  {0x0a6b0fb8, ac_prim_sub_env_mul_t0_add_env},
  // Kotake or koume's hair, zelda
  // (prim-0)*t0+env
  {0x0a7b0a7b, ac_t0_mul_prim_add_env},
  // menu, doubut no mori
  // (1-0)*t0+env
  {0x0a7e0a7e, ac_t0_add_env},
  // Grass, mario golf
  // (env-shade)*t1+env, (0-0)*0+1
  {0x0aa50dff, ac_one},
  // Ridge Racer, cars select. Added by Gonetz
  // (prim-env)*t1+env
  {0x0aab0aab, ac_prim_sub_env_mul_t1_add_env},
  // text, monster truck madness
  // (prim-env)*t1+env, (cmb-0)*t1+0
  {0x0aab0eb8, ac_t1_mul_env},
  // zelda 2 [Ogy]. Added by Gonetz
  // (1-0)*t1+env, (0-0)*0+cmb
  //{0x0abe0abe, ac_one},
  {0x0abe0abe, ac_t1_add_env},
  // arena, Pokemon Stadium 2. Added by Gonetz
  // (1-t0)*prim+env, (cmb-0)*shade+0
  {0x0ace0f38, ac_one_sub_t0_mul_prim_mul_shade},
  // intro, Bomberman 64 - 2. Added by Gonetz
  // (t0-env)*prim+env
  {0x0ae90ae9, ac_t0_sub_env_mul_prim_add_env},
  // N64 logo, Ogre Battle. Added by Gonetz
  // (t0-0)*prim+env
  {0x0af90af9, ac_t0_mul_prim_add_env},
  // girls, PD intro. Added by Gonetz
  // (t0-env)*shade+env  ** INC **
  {0x0b290b29, ac_t0_sub_env_mul_shadea_add_env},
  // Text, Mia Soccer. Added by Gonetz
  // (t0-env)*shade+env, (cmb-0)*lod_fraction+0  ** INC **
  {0x0b290e38, ac_t0_sub_env_mul_shadea_add_env},
  // shadows, Mario Tennis. Added by Gonetz
  // (prim-env)*shade+env, (0-cmb)*t1+cmb ** INC **
  {0x0b2b0087, ac_prim_sub_env_mul_shade_add_env_mul_t1},
  // lamppost?, Ridge Racer. Added by Gonetz
  // (prim-env)*shade+env, (0-0)*0+cmb
  {0x0b2b0b2b, ac_prim_sub_env_mul_shade_add_env},
  // ground, zelda2. Added by Gonetz
  // (1-env)*shade+env, (t1-0)*prim+0
  {0x0b2e0efa, ac_t1_mul_prim},
  // GASP Fighters
  // (t0-0)*shade+env
  {0x0b390b39, ac_t0_mul_shade_add_env},
  // destroying stuff, golden eye
  // (1-0)*shade+env
  {0x0b3e0b3e, ac_env_add_shade},
  // Torches, Paper Mario. Added by Gonetz
  // (t0-t1)*env+env, (0-0)*0+1
  {0x0b510dff, ac_t0_sub_t1_mul_env_add_env},
  // Mini Racers
  // (t0-0)*primlod+env
  {0x0bb90bb9, ac_t0_mul_primlod_add_env},
  // International Track and Field 2000. Added by Gonetz
  // (t0-0)*0+env
  {0x0bf90bf9, ac_env},
  // TM, mario
  //z (0-0)*0+env
  {0x0bff0bff, ac_env},
  // rancho monster, zelda2. Added by Gonetz
  // (0-0)*0+env, (cmb-0)*t1+0
  {0x0bff0eb8, ac_t1_mul_env},
  // Rocket Robot in Wheels intro
  // (0-0)*0+env, (cmb-0)*prim+0
  {0x0bff0ef8, ac_prim_mul_env},
  // Background, Pokemon Snap   
  // (prim-env)*t0+1
  {0x0c6b0c6b, ac_prim_sub_env_mul_t0_add_one},
  // Mario Golf
  // (0-1)*t0+1
  {0x0c770c77, ac_one_sub_t0},
  // flame, paper mario. Added by Gonetz
  // (1-t0)*t1+1, (cmb-t1)*t1+t1
  {0x0c8e0490, ac_t0_mul_t1},
  // hall of fame, Pokemon Stadium
  // (t0-1)*prim+1, (cmb-0)*env+0
  {0x0cf10f78, ac__one_inter_t0_using_prim__mul_env},
  // Ring boundary, dual heroes
  // (0-1)*prim+1
  {0x0cf70cf7, ac_one_sub_prim},
  // Kirby64, level 6, [Raziel64]
  // (0-0)*prim+1
  {0x0cff0cff, ac_one},
  // Mystical Ninja
  // (0-1)*env+1
  {0x0d770d77, ac_one},
  // Deku shield in shop, zelda
  // (1-1)*primlod+1
  {0x0db60db6, ac_one},
  // water near gorons willage. Added by Gonetz
  // (t1-t0)*0+1, (cmb-0)*prim+0
  {0x0dca0ef8, ac_prim},
  // background, kirby 64. Added by Gonetz
  // (t0-0)*0+1
  {0x0df90df9, ac_one},
  // kirby 64. Added by Gonetz
  // (1-0)*0+1
  {0x0dfe0dfe, ac_one},
  // background on level 2-1, kirby 64 [Raziel64]. Added by Gonetz
  // (1-0)*0+1, (0-0)*0+1
  {0x0dfe0dff, ac_one},
  // duck dodgers intro. Added by Gonetz
  // (0-0)*0+1, (cmb-cmb)*primlod+cmb
  {0x0dff0000, ac_one},
  // duck dodgers intro. Added by Gonetz
  // (0-0)*0+1, (0-0)*0+t1 **INC**?
  {0x0dff05ff, ac_t1},
  // ?
  // (0-0)*0+1, (0-0)*0+prim
  {0x0dff07ff, ac_prim},
  // arena, custom robo. Added by Gonetz
  // (0-0)*0+1, (0-0)*0+shade
  {0x0dff09ff, ac_shade},
  // field, Mario Golf
  // (0-0)*0+1, (1-env)*shade+env
  {0x0dff0b2e, ac_one_sub_env_mul_shade_add_env},
  // battle tanks 2 [Ogy]
  // (0-0)*0+1, (0-0)*0+env
  {0x0dff0bff, ac_env},
  // helmet, F1 World Grand Prix. Added by Gonetz
  // (0-0)*0+1, (0-1)*0+1
  {0x0dff0df7, ac_one},
  // secret in level 3-4, Kirby64, [Raziel64]
  // (0-0)*0+1, (cmb-0)*0+1
  {0x0dff0df8, ac_one},
  // Menu options, starfox
  // (0-0)*0+1
  {0x0dff0dff, ac_one},
  // Water, zelda
  //z (0-0)*0+primlod, (cmb-0)*prim+0
  {0x0dff0ef8, ac_prim},
  // Desert ground, zelda
  // (0-0)*0+1, (cmb-0)*shade+0
  {0x0dff0f38, ac_shade},
  // Characters, smash bros
  // (0-0)*0+1, (cmb-0)*env+0
  {0x0dff0f78, ac_env},
  // end of level 3-4, Kirby64, [Raziel64]
  // (0-0)*0+1, (cmb-0)*0+0
  {0x0dff0ff8, ac_zero},
  // Kirby64
  // (0-0)*0+1, (0-0)*0+0
  {0x0dff0fff, ac_zero},
  // floor, Spiderman [Raziel64]. Added by Gonetz
  // (env-t1)*t0+0  ** INC **
  {0x0e550e55, ac_t0_mul_env},
  // skeleton, castlevania 2. Added by Gonetz
  // (1-prim)*t0+0
  {0x0e5e0e5e, ac_one_sub_prim_mul_t0},
  // player select, Forsaken [Raziel64]. Added by Gonetz
  // (prim-shade)*t0+0
  {0x0e630e63, ac_prim_sub_shade_mul_t0},
  // castlevania 2 [Ogy]. Added by Gonetz
  // (1-shade)*t0+0
  {0x0e660e66, ac_one_sub_shade_mul_t0},
  // GoldenEye: Helicopter rotors
  // (shade-env)*t0+0, (1-0)*prim+cmb
  {0x0e6c00fe, ac_shade_sub_env_mul_t0_add_prim},
  // background, level3-4, Kirby64, [Raziel64]
  // (shade-env)*t0+0
  {0x0e6c0e6c, ac_shade_sub_env_mul_t0},
  // Goemon, mystical ninja. Added by Gonetz
  // (1-env)*t0+0
  {0x0e6e0e6e, ac_one_sub_env_mul_t0},
  // fist attack, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (t0-0)*t0+0, (t1-cmb)*prim+cmb
  {0x0e7900c2, ac_t0_inter_t1_using_prima},
  // Clay Fighter [Ogy]. Added by Gonetz
  // (t0-0)*t0+0
  {0x0e790e79, ac_t0_mul_t0},
  // Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (t0-0)*t0+0, (prim-0)*t1+0
  {0x0e790ebb, ac_t1_mul_prim},
  // zelda 2 [Ogy]. Added by Gonetz
  // (t0-0)*t0+0, (cmb-0)*prim+0
  {0x0e790ef8, ac_t0_mul_prim},
  // zelda 2. Added by Gonetz
  // (t0-0)*t0+0, (cmb-0)*shade+0
  {0x0e790f38, ac_t0_mul_shade},
  // zelda 2. Added by Gonetz
  // (t0-0)*t0+0, (cmb-0)*env+0
  {0x0e790f78, ac_t0_mul_env},
  // the ice plane just before the entrance to gorons village (where tingle is}, zelda 2 [Ogy]. Added by Gonetz
  // (t1-0)*t0+0, (cmb-0)*0+cmb
  {0x0e7a01f8, ac_t0_mul_t1},
  // paper mario. Added by Gonetz
  // (t1-0)*t0+0, (cmb-env)*prim+env  ** INC **
  {0x0e7a0ae8, ac_t1_mul_prim},
  // mini games quiz monitor backround, Pokemon Stadium 2
  // (t1-0)*t0+0, (0-0)*0+1
  {0x0e7a0dff, ac_one},
  // Tony Hawk's Pro Skater. Added by Gonetz
  // (t1-0)*t0+0, (cmb-0)*t0+0
  {0x0e7a0e78, ac_t0_mul_t1},
  // bike trace, xg2. Added by Gonetz
  // (t1-0)*t0+0
  {0x0e7a0e7a, ac_t0_mul_t1},
  // Kotake & koume defeated, zelda
  // (t1-0)*t0+0, (cmb-0)*prim+0
  {0x0e7a0ef8, ac__t0_mul_t1__mul_prim},
  // Magnitude, pokemon stadium 2
  // (t1-0)*t0+0, (cmb-env)*shade+0
  {0x0e7a0f28, ac__t0_mul_t1__mul_env_mul_shade},
  // Bongo Bongo, zelda
  // (t1-0)*t0+0, (cmb-0)*shade+0
  {0x0e7a0f38, ac__t0_mul_t1__mul_shade},
  //  Dobutsu_no_Mori, waterfall
  // (t1-0)*t0+0, (cmb-0)*prim_lod+0
  {0x0e7a0fb8, ac__t0_mul_t1__mul_primlod},
  // Back of doors, megaman
  // (prim-0)*t0+0, (cmb-0)*lodfrac+0
  {0x0e7b0e38, ac_t0_mul_prim},
  // Karts, mario kart
  //z (prim-0)*t0+0
  {0x0e7b0e7b, ac_t0_mul_prim},
  // paper mario. Added by Gonetz
  // (prim-0)*t0+0, (t0-0)*prim+0
  {0x0e7b0ef9, ac_t0_mul_prim},
  // Table, mace
  // (prim-0)*t0+0, (cmb-0)*shade+0
  {0x0e7b0f38, ac_t0_mul_prim_mul_shade},
  // lamp shadow, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (shade-0)*t0+0, (cmb-cmb)*lodf+cmb
  {0x0e7c0000, ac_t0_mul_shade},
  // Game logo, Aerofighters Assault [Raziel64]
  //(shade-0)*t0+0, (0-0)*0+1
  {0x0e7c0dff, ac_one},
  // Higher sky, waverace
  //z (shade-0)*t0+0
  {0x0e7c0e7c, ac_t0_mul_shade},
  // duck dodgers, intro. Added by Gonetz
  // (shade-0)*t0+0, (cmb-0)*prim+0
  {0x0e7c0ef8, ac_t0_mul_prim_mul_shade},
  // waterwheel in water temple, zelda 2. Added by Gonetz
  // (shade-0)*t0+0, (cmb-0)*env+0
  {0x0e7c0f78, ac_t0_mul_env_mul_shade},
  // Blowing up mine at bowser's, mario
  // (env-0)*t0+0
  {0x0e7d0e7d, ac_t0_mul_env},
  // castlevania 2, intro. Added by Gonetz
  // (1-0)*t0+0
  {0x0e7e0e7e, ac_t0},
  // moon, castlevania 2. Added by Gonetz
  // (1-0)*t0+0, (cmb-0)*prim+0
  {0x0e7e0ef8, ac_t0_mul_prim},
  //beetle adventure racing. Added by Gonetz
  // (1-0)*t0+0, (cmb-0)*shade+0
  {0x0e7e0f38, ac_t0_mul_shade},
  // lava, beetle adventure racing
  // (t0-prim)*t1+0, (0-0)*0+shade   ** INC **
  {0x0e9909ff, ac_shade},
  // Rain Dance, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t0-env)*t1+0, (cmb-0)*0+prim   ** INC **
  {0x0ea90ef8, ac__t0_mul_t1__mul_prim},
  // Duck Dodgers Starring Daffy Duck text background
  // (t0-0)*t1+0, (shade-cmb)*prim+cmb
  {0x0eb900c4, ac_one_sub_prim_mul__t0_mul_t1__add__prim_mul_shade},
  // torches, pokemon stadium 2
  // (t0-0)*t1+0, (cmb-env)*prim+cmb
  {0x0eb900e8, ac__t0_mul_t1__sub_env_mul_prim_add__t0_mul_t1},
  // airboarder 64 [Ogy]
  // (t0-0)*t1+0, (0-0)*0+prim
  {0x0eb907ff, ac_prim},
  // explosion, body harvest. Added by Gonetz
  // (t0-0)*t1+0, (0-0)*0+shade
  {0x0eb909ff, ac_shade},
  // Text off top, banjo kazooie
  // (t0-0)*t1+0
  {0x0eb90eb9, ac_t0_mul_t1},
  // smoke, daikatana. Added by Gonetz
  // (t0-0)*t1+0, (cmb-0)*prim+0
  {0x0eb90ef8, ac__t0_mul_t1__mul_prim},
  // Arena, Pokemon Stadium 2.
  // (t0-0)*t1+0, (cmb-prim)*shade+0
  {0x0eb90f18, ac__t0_mul_t1__sub_prim_mul_shade},
  // Water, pilotwings
  // (t0-0)*t1+0, (cmb-0)*shade+0
  {0x0eb90f38, ac__t0_mul_t1__mul_shade},
  // Tony Hawk's Pro Skater. Added by Gonetz
  // (t0-0)*t1+0, (cmb-0)*env+0
  {0x0eb90f78, ac__t0_mul_t1__mul_env},
  // light from window, Dobutsu no Mori. Added by Gonetz
  // (t0-0)*t1+0, (cmb-0)*primlod+0
  {0x0eb90fb8, ac__t0_mul_t1__mul_primlod},
  // chandelier in spider house, zelda 2. Added by Gonetz
  // (t1-0)*t1+0, (cmb-0)*prim+0
  {0x0eba0ef8, ac_t1_mul_prim},
  // cars, ridge racer. Added by Gonetz
  // (prim-0)*t1+0, (0-0)*0+1
  {0x0ebb0dff, ac_t1_mul_prim},
  // aerofighter's assault [Ogy]
  // (prim-0)*t1+0
  {0x0ebb0ebb, ac_t1_mul_prim},
  // tire trace, beetle adventure racing. Added by Gonetz
  // (shade-0)*t1+0
  {0x0ebc0ebc, ac_t1_mul_shade},
  // smoke, Starshot. Added by Gonetz
  // (env-0)*t1+0
  {0x0ebd0ebd, ac_t1_mul_env},
  // lots of things, goldeneye
  // (1-0)*t1+0, (0-0)*0+shade
  {0x0ebe09ff, ac_shade},
  // zelda 2 [Ogy]. Added by Gonetz
  // (1-0)*t1+0, (cmb-0)*prim+0
  {0x0ebe0ef8, ac_t1_mul_prim},
  // walls, perfect dark. Added by Gonetz
  // (1-0)*t1+0, (cmb-0)*shade+0
  {0x0ebe0f38, ac_t1_mul_shade},
  // sand, perfect dark. Added by Gonetz
  // (1-0)*t1+0, (cmb-0)*env+0
  {0x0ebe0f78, ac_t1_mul_env},
  // light, Ridge Racer. Added by Gonetz
  // (1-t0)*prim+0
  {0x0ece0ece, ac_one_sub_t0_mul_prim},
  // exaust, star wars ep1 racer
  // (1-t0)*prim+0, (cmb-0)*shade+0
  {0x0ece0f38, ac_one_sub_t0_mul_primshade},
  // iguana, Forsaken, [Raziel64]. Added by Gonetz
  // (t0-shade)*prim+0
  {0x0ee10ee1, ac_t0_sub_shade_mul_prim},
  // stands, NASCAR 2000
  // (prim-shade)*prim+0
  {0x0ee30ee3, ac_prim_sub_shade_mul_prim},
  // arena, Pokemon Stadium 2. Added by Gonetz
  // (t0-env)*prim+0    ** INC **
  {0x0ee90ee9, ac_t0_mul_prim},
  // lure, bass rush
  // (t0-0)*prim+0, (cmb-cmb)*lodf+cmb
  {0x0ef90000, ac_t0_mul_prim},
  // explosion, body harvest. Added by Gonetz
  // (t0-0)*prim+0, (t0-0)*env+cmb
  {0x0ef90179, ac_prim_add_env_mul_t0},
  // frog's eyes, zelda
  // (t0-0)*prim+0, (1-1)*prim_lod+cmb
  {0x0ef901b6, ac_t0_mul_prim},
  // Monster truck madness intro. Added by Gonetz
  // (t0-0)*prim+0, (cmb-0)*prim_lod+cmb  ** INC **
  {0x0ef901b8, ac_t0_mul_prim},
  // Road, zelda
  //z (t0-0)*prim+0, (t1-0)*primlod+cmb
  {0x0ef901ba, ac__t0_mul_prim__add__t1_mul_primlod},
  // Track, wipeout. Addded by Gonetz
  // (t0-0)*prim+0, (0-0)*0+prim
  {0x0ef907ff, ac_t0_mul_prim},
  // magic stuff, buck bumble. Added by Gonetz
  // (t0-0)*prim+0, (cmb-0)*prim+0
  {0x0ef90ef8, ac_t0_mul_prim_mul_prim},
  // The mario face, mario
  //z (t0-0)*prim+0
  {0x0ef90ef9, ac_t0_mul_prim},
  // paper mario. Added by Gonetz
  // (t0-0)*prim+0, (cmb-0)*shade+0
  {0x0ef90f38, ac_t0_mul_prim_mul_shade},
  // Pikachu's mouth, smash bros
  // (t0-0)*prim+0, (cmb-0)*env+0
  {0x0ef90f78, ac_t0_mul_prim_mul_env},
  // bomb mask, zelda 2. Added by Gonetz
  // (t0-0)*prim+0, (1-0)*env+0
  {0x0ef90f7e, ac_t0_mul_prim},
  // Charmander's tail, pokemon stadium 2
  // (t0-0)*prim+0, (cmb-0)*primlod+0
  {0x0ef90fb8, ac_t0_mul_prim_mul_primlod},
  // stalactites, Beetle adventure Racing. Added by Gonetz
  // (t1-0)*prim+0, (1-cmb)*shade+cmb
  {0x0efa0106, ac_one_sub_shade_mul_t1_add_shade},
  // Sprites, Ogre Battle. Added by Gonetz
  // (t1-0)*prim+0, (0-0)*0+cmb
  {0x0efa0efa, ac_t1_mul_prim},
  // Something about kotake & koume's combined attack, zelda
  // (t1-0)*prim+0, (cmb-0)*shade+0
  {0x0efa0f38, ac_t1_mul_prim_mul_shade},
  // intro background, bio freaks. Added by Gonetz
  // (prim-0)*prim+0
  {0x0efb0efb, ac_prim_mul_prim},
  // sky, xg2. Added by Gonetz
  // (shade-0)*prim+0, (0-0)*0+1
  {0x0efc0dff, ac_one},
  // Zelda, unimp log. Added by Gonetz
  // (shade-0)*prim+0
  {0x0efc0efc, ac_prim_mul_shade},
  // ?
  // (shade-0)*prim+0, (cmb-0)*shade+0 ** INC **
  {0x0efc0f38, ac_prim_mul_shade},
  // Baby mario's hat shadow, mario golf
  // (env-0)*prim+0
  {0x0efd0efd, ac_prim_mul_env},
  // Menu, doom
  // (1-0)*prim+0
  {0x0efe0efe, ac_prim},
  // Peris Song attack, Pokemin Stadium 2
  // (1-0)*prim+0, (cmb-0)*shade+0
  {0x0efe0f38, ac_prim_mul_shade},
  // Conker's shadow, CBFD. Added by Gonetz
  // (1-t0)*shade+0
  {0x0f0e0f0e, ac_one_sub_t0_mul_shade},
  // Rock smash, pokemon stadium 2
  // (1-t0)*shade+0
  {0x0f0f0ee8, ac_one_sub_t0_mul_shade},
  //waterfall, Paper Mario
  // (t0-t1)*shade+0
  {0x0f110f11, ac__t0_sub_t1__mul_shade},
  // mahogany town statue, Pokemon Stadium 2
  // (t0-prim)*shade+0
  {0x0f190f19, ac_t0_sub_prim_mul_shade},
  // silver cave, pokemon stadium 2
  // (t0-prim)*shade+0, (cmb-0)*env+0
  {0x0f190f78, ac_t0_sub_prim_mul_shade_mul_env},
  // Boomerang circle, zelda
  // (t0-0)*shade+0, (1-cmb)*t0+cmb
  {0x0f390046, ac_t0_mul_shade},
  // THPS3
  // (t0-0)*shade+0, (1-0)*t0+cmb
  {0x0f39007e, ac_t0_mul_shade},
  // ???
  // (t0-0)*shade+0, (env-0)*t1+cmb
  {0x0f3900bd, ac_t0_mul_shade},
  // Forest temple doorway, zelda
  // (t0-0)*shade+0, (t1-0)*primlod+cmb
  {0x0f3901ba, ac_t0_mul_shade},
  // skis, Spacestation Silicon Valley. Added by Gonetz
  // (t0-0)*shade+0, (0-0)*0+t0
  {0x0f3903ff, ac_t0},
  // paper mario. Added by Gonetz
  // (t0-0)*shade+0, (cmb-t0)*prim+0
  {0x0f390ec8, ac_t0_mul_prim_mul_shade},
  // House windows, zelda intro
  //z (t0-0)*shade+0, (cmb-0)*prim+0
  {0x0f390ef8, ac_t0_mul_prim_mul_shade},
  // Characters, mace
  // (t0-0)*shade+0, (cmb-0)*shade+0
  {0x0f390f38, ac_t0_mul_shade},
  // Shadows, mario
  //z (t0-0)*shade+0
  {0x0f390f39, ac_t0_mul_shade},
  // Clear screen intro, banjo kazooie
  // (t0-0)*shade+0, (cmb-0)*env+0
  {0x0f390f78, ac_t0_mul_env_mul_shade},
  // ridge racer, unimp log. Added by Gonetz
  // (t0-0)*shade+0, (cmb-0)*primlod+0  **INC**?
  {0x0f390fb8, ac_t0_mul_shade},
  // Reflecting combined attack at kotake & koume's, zelda
  // (t1-0)*shade+0, (cmb-0)*prim+0
  {0x0f3a0ef8, ac_t1_mul_prim_mul_shade},
  // aerofighter's assault [Ogy]
  // (t1-0)*shade+0
  {0x0f3a0f3a, ac_t1_mul_shade},
  //beetle adventure racing. Added by Gonetz
  //(t1-0)*shade+0, (cmb-0)*env+0
  {0x0f3a0f78, ac_t1_mul_env_mul_shade},
  // building shadow, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (prim-0)*shade+0, (cmb-cmb)*lodf+cmb
  {0x0f3b0000, ac_prim_mul_shade},
  //chip in Spacestation Silicon Valley intro. Added by Gonetz
  // (prim-0)*shade+0, (env-cmb)*t1+cmb
  {0x0f3b0085, ac_env_sub_primshade_mul_t1_add_primshade},
  // N64 logo, tetrisphere. Added by Gonetz
  // (prim-0)*shade+0, (prim-0)*shade+0
  {0x0f3b0f3b, ac_prim_mul_shade},
  // rays, Fushigi no Dungeon - Furai no Shiren 2. Added by Gonetz
  // (shade-0)*shade+0, (cmb-0)*prim+0
  {0x0f3c0ef8, ac_prim_mul_shade},
  // light, dracula resurrection, castlevania 2. Added by Gonetz
  // (env-0)*shade+0
  {0x0f3d0f3d, ac_env_mul_shade},
  // zelda 2 [Ogy]. Added by Gonetz
  // (1-0)*shade+0
  {0x0f3e0f3e, ac_shade},
  // surf pokemon attack, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (1-t0)*env+0, (1-cmb)*shade+0  ** INC **
  {0x0f4e0f06, ac_t0_mul_shade},
  // GE, boxes
  // (1-shade)*env+0, (cmb-0)*shade+0 ** INC **
  {0x0f660f38, ac_one_sub_shade_mul_env},
  //beetle adventure racing. Added by Gonetz
  //(t0-0)*env+0, (1-t0)*prim+cmb
  //{0x0f7900ce, ac_t0_mul_env}, //this one looks better
  //{0x0f7900ce, ac_env_sub_prim_mul_t0_add_prim},
  {0x0f7900ce, ac_one_sub_t1_add_t0_mul_env},
  //Zelda, logo ? Added by Gonetz
  //(t0-0)*env+0, (t1-0)*primlod+0    **INC** changed to mul_env for gannon's organ disappearing [Dave2001]
  {0x0f7901ba, ac__t0_inter_t1_using_primlod__mul_env},
  // V8-2 menu
  // (t0-0)*env+0, (0-0)*0+prim
  {0x0f7907ff, ac_prim},
  // Skeleton guy's eyes, zelda
  // (t0-0)*env+0, (cmb-0)*prim+0
  {0x0f790ef8, ac_t0_mul_prim_mul_env},
  // Dust from rock spell, quest64
  // (t0-0)*env+0, (cmb-0)*shade+0
  {0x0f790f38, ac_t0_mul_env_mul_shade},
  // eyes of poe, zelda
  // (t0-0)*env+0, (cmb-0)*env+0
  {0x0f790f78, ac_t0_mul_env},
  // Text, mario
  //z (t0-0)*env+0
  {0x0f790f79, ac_t0_mul_env},
  // Shadows, pokemon stadeom 2
  // (t0-0)*env+0, (cmb-0)*primlod+0
  {0x0f790fb8, ac_t0_mul_env_mul_primlod},
  //gauge, PGA
  // (t1-0)*env+0, (cmb-0)*t1+0 ** INC **
  {0x0f7a0eb8, ac_t1_mul_env},
  //text and shadows, Rayman2. Added by Gonetz
  // (t1-0)*env+0, (cmb-0)*shade+0
  {0x0f7a0f38, ac_t1_mul_env_mul_shade},
  // shadows, tom and jerry. Added by Gonetz
  // (t1-0)*env+0
  {0x0f7a0f7a, ac_t1_mul_env},
  // Bomberman64-2 intro. Added by Gonetz
  // (prim-0)*env+0
  {0x0f7b0f7b, ac_prim_mul_env},
  // Text box, mario
  //z (shade-0)*env+0
  {0x0f7c0f7c, ac_env_mul_shade},
  // Ogre battle 64
  // (env-0)*env+0
  {0x0f7d0f7d, ac_env},
  //Goldeneye, [Jeremy]. Added by Gonetz
  // (1-0)*env+0, (cmb-0)*shade+0
  {0x0f7e0f38, ac_env_mul_shade},
  // Status items, megaman
  // (1-0)*env+0
  {0x0f7e0f7e, ac_env},
  // gun fire, Beast_Wars_Transmetal [Raziel64]
  // (0-0)*env+0
  {0x0f7f0f7f, ac_zero},
  // Pokemon attack, Pokemon stadium (J). Added by Gonetz
  // (t1-t0)*primlod+0, (cmb-0)*env+prim  ** INC **
  {0x0f8a0778, ac__t1_sub_t0_mul_primlod__mul_env_add_prim},
  // Shadow Ball, Pokemon Stadium 2 [gokuss4]. Added by Gonetz
  // (t1-t0)*primlod+0, (t1-cmb)*prim+0  ** INC **
  {0x0f8a0ec2, ac_t0_mul_prim},
  // Walls of well through lens of truth, zelda
  // (prim-t0)*primlod+0
  {0x0f8b0f8b, ac_prim_sub_t0},
  // N64 logo, ridge racer. Added by Gonetz
  // (1-prim)*primlod+0  **INC**
  {0x0f9e0f9e, ac_zero},
  // Vines that covers a door in the third room of woodfall temple, zelda 2 [Ogy]. Added by Gonetz
  // (t0-0)*primlod+0, (cmb-0)*prim+0
  {0x0fb90ef8, ac_t0_mul_primlod_mul_prim},
  // zelda 2. Added by Gonetz
  // (t0-0)*primlod+0
  {0x0fb90fb9, ac_t0_mul_primlod},
  // NFL Blitz logo. Added by Gonetz
  // (t1-0)*primlod+0
  {0x0fba0fba, ac_t1_mul_primlod}, //causes issues
  // fallen stars at star summit, Paper Mario. Added by Gonetz
  // (shade-0)*primlod+0
  {0x0fbc0fbc, ac_primlod_mul_shade},
  // expansion pack, Jeremy McGrath Supercross 2000. Added by Gonetz
  // (1-0)*primlod+0
  {0x0fbe0fbe, ac_primlod},
  // intro, Aidyn Chronicles. Added by Gonetz
  // (0-0)*primlod+0, (prim-env)*t0+prim  **INC**
  {0x0fbf066b, ac_t0_mul_prim},
  // sky, Rayman2. Added by Gonetz
  // (0-shade)*0+0
  {0x0fe70fe7, ac_zero},
  // flame, PokemonStadium1 [Raziel64]
  // (t0-0)*0+0
  {0x0ff90ff9, ac_zero},
  //BAR
  // (0-0)*0+0, (0-0)*0+TEXEL1
  {0x0fff05ff, ac_t0},
  // Screen clear, banjo kazooie
  // (0-0)*0+0
  {0x0fff0fff, ac_zero},
  // { #ACEND }
};

// CountCombine - count the # of entries in the combine lists
void CountCombine ()
{
  int size = sizeof(color_cmb_list) / sizeof(COMBINER);
  int i=0, index=0, a, b;
  do {
    a = color_cmb_list[index].key >> 24;
    for (; i<=a; i++)
      cc_lookup[i] = index;

    while (index < size)
    {
      b = color_cmb_list[index].key >> 24;
      if (b != a) break;
      index ++;
    }
  } while (index < size);
  for (; i<257; i++) cc_lookup[i] = index;

  size = sizeof(alpha_cmb_list) / sizeof(COMBINER);
  i=0, index=0;
  do {
    a = (alpha_cmb_list[index].key >> 20) & 0xFF;
    for (; i<=a; i++)
      ac_lookup[i] = index;

    while (index < size)
    {
      b = (alpha_cmb_list[index].key >> 20) & 0xFF;
      if (b != a) break;
      index ++;
    }
  } while (index < size);
  for (; i<257; i++) ac_lookup[i] = index;

  //color_cmb_list_count = sizeof(color_cmb_list) >> 3; // #bytes/4/2
  //alpha_cmb_list_count = sizeof(alpha_cmb_list) >> 3;
}

//****************************************************************
// Main Combine
//****************************************************************

void Combine ()
{
  FRDP (" | |- color combine: %08lx, #1: (%s-%s)*%s+%s, #2: (%s-%s)*%s+%s\n",
    ((rdp.cycle1 & 0xFFFF) << 16) | (rdp.cycle2 & 0xFFFF),
    Mode0[rdp.cycle1&0xF], Mode1[(rdp.cycle1>>4)&0xF], Mode2[(rdp.cycle1>>8)&0x1F], Mode3[(rdp.cycle1>>13)&7],
    Mode0[rdp.cycle2&0xF], Mode1[(rdp.cycle2>>4)&0xF], Mode2[(rdp.cycle2>>8)&0x1F], Mode3[(rdp.cycle2>>13)&7]);
  FRDP (" | |- alpha combine: %08lx, #1: (%s-%s)*%s+%s, #2: (%s-%s)*%s+%s\n",
    (rdp.cycle1 & 0x0FFF0000) | ((rdp.cycle2 & 0x0FFF0000) >> 16),
    Alpha0[(rdp.cycle1>>16)&7], Alpha1[(rdp.cycle1>>19)&7], Alpha2[(rdp.cycle1>>22)&7], Alpha3[(rdp.cycle1>>25)&7],
    Alpha0[(rdp.cycle2>>16)&7], Alpha1[(rdp.cycle2>>19)&7], Alpha2[(rdp.cycle2>>22)&7], Alpha3[(rdp.cycle2>>25)&7]);
  if (!rdp.LOD_en || rdp.cur_tile == rdp.mipmap_level)
    lod_frac = rdp.prim_lodfrac;
  else if (settings.lodmode == 0)
    lod_frac = 0;
  else
    lod_frac = 10;

  rdp.noise = RDP::noise_none;

  wxUint32 found = TRUE;

  rdp.col[0] = rdp.col[1] = rdp.col[2] = rdp.col[3] =
    rdp.coladd[0] = rdp.coladd[1] = rdp.coladd[2] = rdp.coladd[3] = 1.0f;
  rdp.cmb_flags = rdp.cmb_flags_2 = 0;

  rdp.uncombined = 0;

  cmb.tex = 0;
  cmb.tmu0_func = cmb.tmu1_func = cmb.tmu0_a_func = cmb.tmu1_a_func = GR_COMBINE_FUNCTION_ZERO;
  cmb.tmu0_fac = cmb.tmu1_fac = cmb.tmu0_a_fac = cmb.tmu1_a_fac = GR_COMBINE_FACTOR_NONE;
  cmb.tmu0_invert = cmb.tmu0_a_invert = cmb.tmu1_invert = cmb.tmu1_a_invert = FXFALSE;

  cmb.dc0_detailmax = cmb.dc1_detailmax = 0;

  cmb.mod_0 = cmb.mod_1 = 0;    // remove all modifications
  cmb.modcolor_0 = cmb.modcolor1_0 = cmb.modcolor2_0 = cmb.modcolor_1 = cmb.modcolor1_1 = cmb.modcolor2_1
    = cmb.modfactor_0 = cmb.modfactor_1 = 0;

  cmb.ccolor = cmb.tex_ccolor = 0;
  if (cmb.cmb_ext_use || cmb.tex_cmb_ext_use)
  {
    //have to draw something to allow use of standard combine functions
    if (fullscreen)
    {
      VERTEX v;
      memset(&v,0,sizeof(v));
      grDrawPoint(&v);
    }
    cmb.cmb_ext_use = 0;
    cmb.tex_cmb_ext_use = 0;
  }

  wxUint32 cmb_mode_c = (rdp.cycle1 << 16) | (rdp.cycle2 & 0xFFFF);
  wxUint32 cmb_mode_a = (rdp.cycle1 & 0x0FFF0000) | ((rdp.cycle2 >> 16) & 0x00000FFF);

  cmb.abf1 = GR_BLEND_SRC_ALPHA;
  cmb.abf2 = GR_BLEND_ONE_MINUS_SRC_ALPHA;

#ifdef FASTSEARCH
  // Fast, ordered search
  int current=0x7FFFFFFF, last;
  wxUint32 actual_combine, current_combine, color_combine, alpha_combine;
  int left, right;

  actual_combine = cmb_mode_c;
  color_combine = actual_combine;
  if ((rdp.cycle2 & 0xFFFF) == 0x1FFF)
    actual_combine = (rdp.cycle1 << 16) | (rdp.cycle1 & 0xFFFF);

  left = cc_lookup[actual_combine>>24];
  right = cc_lookup[(actual_combine>>24)+1];

  while (1)
  {
    last = current;
    current = left + ((right-left) >> 1);
    if (current == last)
      break;  // can't be found!

    current_combine = color_cmb_list[current].key;
    if (current_combine < actual_combine)
      left = current;
    else if (current_combine > actual_combine)
      right = current;
    else
      break;  // found it!
  }

  // Check if we didn't find it
  if (actual_combine != current_combine)
  {
    rdp.uncombined |= 1;
#ifdef UNIMP_LOG
    if (settings.log_unk)
    {
      sprintf (out_buf, "COLOR combine not found: %08x, #1: (%s-%s)*%s+%s, #2: (%s-%s)*%s+%s\n",
        actual_combine,
        Mode0[rdp.cycle1&0xF], Mode1[(rdp.cycle1>>4)&0xF], Mode2[(rdp.cycle1>>8)&0x1F], Mode3[(rdp.cycle1>>13)&7],
        Mode0[rdp.cycle2&0xF], Mode1[(rdp.cycle2>>4)&0xF], Mode2[(rdp.cycle2>>8)&0x1F], Mode3[(rdp.cycle2>>13)&7]);
      UNIMPMODE();
    }
#endif
    found = FALSE;
    //tex |= 3;

    // use t0 as default
    cc_t0 ();
  }
  else
    color_cmb_list[current].func();

  LRDP(" | |- Color done\n");

  // Now again for alpha
  current = 0x7FFFFFFF;
  actual_combine = cmb_mode_a;
  alpha_combine = actual_combine;
  if ((rdp.cycle2 & 0x0FFF0000) == 0x01FF0000)
    actual_combine = (rdp.cycle1 & 0x0FFF0000) | ((rdp.cycle1 >> 16) & 0x00000FFF);
  if ((rdp.cycle1 & 0x0FFF0000) == 0x0FFF0000)
    actual_combine = (rdp.cycle2 & 0x0FFF0000) | ((rdp.cycle2 >> 16) & 0x00000FFF);

  left = ac_lookup[(actual_combine>>20)&0xFF];
  right = ac_lookup[((actual_combine>>20)&0xFF)+1];

  while (1)
  {
    last = current;
    current = left + ((right-left) >> 1);
    if (current == last)
      break;  // can't be found!

    current_combine = alpha_cmb_list[current].key;
    if (current_combine < actual_combine)
      left = current;
    else if (current_combine > actual_combine)
      right = current;
    else
      break;  // found it!
  }

  // Check if we didn't find it
  if (actual_combine != current_combine || !found)
  {
    if (actual_combine != current_combine)
    {
      rdp.uncombined |= 2;
#ifdef UNIMP_LOG
      if (settings.log_unk)
      {
        sprintf (out_buf, "ALPHA combine not found: %08x, #1: (%s-%s)*%s+%s, #2: (%s-%s)*%s+%s\n",
          actual_combine,
          Alpha0[(rdp.cycle1>>16)&7], Alpha1[(rdp.cycle1>>19)&7], Alpha2[(rdp.cycle1>>22)&7], Alpha3[(rdp.cycle1>>25)&7],
          Alpha0[(rdp.cycle2>>16)&7], Alpha1[(rdp.cycle2>>19)&7], Alpha2[(rdp.cycle2>>22)&7], Alpha3[(rdp.cycle2>>25)&7]);
        UNIMPMODE();
      }
#endif
    }
    if (settings.unk_as_red)
    {
      BrightRed ();
    }
    else
    {
      // use full alpha as default
      ac_t0 ();
    }
    //tex |= 3;
  }
  else
    alpha_cmb_list[current].func();


  if (color_combine == 0x69351fff) //text, PD, need to change texture alpha
  {
    A_USE_T1();
  }
  else if ((color_combine == 0x3fff1fff) && (alpha_combine == 0x03ff03ff) && (rdp.last_tile > rdp.cur_tile))//Dr. Mario
  {
    cc_t0();
    ac_t1();
  }
  else if (color_combine == 0x613522f0 && (settings.hacks&hack_PMario)) //Paper Mario fortune teller spheres
  {
    ac_t0();
  }

  LRDP(" | |- Alpha done\n");
#endif // FASTSEARCH

  CombineBlender ();
  //*
  // Update textures?
  //    if (tex == 2 && rdp.texrecting && (cmb.tmu1_func != GR_COMBINE_FUNCTION_ZERO) && (rdp.last_tile_size == 0))
  if (cmb.tex == 2 && rdp.texrecting && (rdp.cur_tile == rdp.last_tile_size))
  {
    cmb.tex = 0;
    USE_T0();
    A_USE_T0();
  }
  //*/
  rdp.tex = cmb.tex;

  if (fullscreen)
  {
    TBUFF_COLOR_IMAGE * aTBuff[2] = {0, 0};
    if (rdp.aTBuffTex[0])
      aTBuff[rdp.aTBuffTex[0]->tile] = rdp.aTBuffTex[0];
    if (rdp.aTBuffTex[1])
      aTBuff[rdp.aTBuffTex[1]->tile] = rdp.aTBuffTex[1];
    if (cmb.tex && (aTBuff[0] || aTBuff[1]))
    {
      if (aTBuff[0] && (settings.frame_buffer&fb_read_alpha))
      {
        if ((settings.hacks&hack_PMario) && aTBuff[0]->width == rdp.ci_width)
          ;
        else
        {
          grChromakeyValue(0);
          grChromakeyMode(GR_CHROMAKEY_ENABLE);
        }
      }
      else
        grChromakeyMode(GR_CHROMAKEY_DISABLE);

      if (aTBuff[0] && aTBuff[0]->info.format == GR_TEXFMT_ALPHA_INTENSITY_88)
      {
        if (cmb.tex_cmb_ext_use & TEX_COMBINE_EXT_COLOR)
        {
          if (cmb.t0c_ext_a == GR_CMBX_LOCAL_TEXTURE_RGB)
            cmb.t0c_ext_a = GR_CMBX_LOCAL_TEXTURE_ALPHA;
          if (cmb.t0c_ext_b == GR_CMBX_LOCAL_TEXTURE_RGB)
            cmb.t0c_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA;
          if (cmb.t0c_ext_c == GR_CMBX_LOCAL_TEXTURE_RGB)
            cmb.t0c_ext_c = GR_CMBX_LOCAL_TEXTURE_ALPHA;
        }
        else
          cmb.tmu0_func = GR_COMBINE_FUNCTION_LOCAL_ALPHA;
      }

      if (aTBuff[1] && aTBuff[1]->info.format == GR_TEXFMT_ALPHA_INTENSITY_88)
      {
        if (cmb.tex_cmb_ext_use & TEX_COMBINE_EXT_COLOR)
        {
          if (cmb.t1c_ext_a == GR_CMBX_LOCAL_TEXTURE_RGB)
            cmb.t1c_ext_a = GR_CMBX_LOCAL_TEXTURE_ALPHA;
          if (cmb.t1c_ext_b == GR_CMBX_LOCAL_TEXTURE_RGB)
            cmb.t1c_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA;
          if (cmb.t1c_ext_c == GR_CMBX_LOCAL_TEXTURE_RGB)
            cmb.t1c_ext_c = GR_CMBX_LOCAL_TEXTURE_ALPHA;
        }
        else
          cmb.tmu1_func = GR_COMBINE_FUNCTION_LOCAL_ALPHA;
      }
    }
    else
      grChromakeyMode(GR_CHROMAKEY_DISABLE);
  }
  cmb.shade_mod_hash = (rdp.cmb_flags + rdp.cmb_flags_2) * (rdp.prim_color + rdp.env_color + rdp.K5);

  LRDP(" | + Combine end\n");
}

void CombineBlender ()
{
  wxUint32 blendmode = rdp.othermode_l >> 16;
  // Check force-blending
  if ((rdp.othermode_l & 0x4000) && (rdp.cycle_mode < 2))
  {
    switch (blendmode)
    {
      // Mace objects
    case 0x0382:
    case 0x0091:
      // 1080 sky
    case 0x0c08:
      // Mario kart player select
      // clr_in * 0 + clr_in * 1
      //  - or just clr_in, no matter what alpha
    case 0x0f0a:
      //DK64 blue prints
    case 0x0302:
      //Sin and Punishment
    case 0xcb02:
      // Battlezone
      // clr_in * a + clr_in * (1-a)
    case 0xc800:
    case 0x00c0:
      //ISS64
    case 0xc302:
      A_BLEND (GR_BLEND_ONE, GR_BLEND_ZERO);
      break;

      //Space Invaders
    case 0x0448:
    case 0x055a:
      A_BLEND (GR_BLEND_ONE, GR_BLEND_ONE);
      break;

      // LOT in Zelda: MM
    case 0xaf50:
    case 0x0f5a: //clr_in * 0 + clr_mem * 1
      A_BLEND (GR_BLEND_ZERO, GR_BLEND_ONE);
      break;

    case 0x5f50: //clr_mem * 0 + clr_mem * (1-a)
      A_BLEND (GR_BLEND_ZERO, GR_BLEND_ONE_MINUS_SRC_ALPHA);
      break;

      /*
      case 0xc410: // Perfect Dark Mauler
      {
      MOD_0 (TMOD_TEX_INTER_COLOR_USING_FACTOR);
      MOD_0_COL (rdp.fog_color & 0xFFFFFF00);
      MOD_0_FAC (rdp.fog_color & 0xFF);
      INTERSHADE_2 (rdp.fog_color & 0xFFFFFF00, rdp.fog_color & 0xFF);

      float percent = (rdp.fog_color & 0xFF) / 255.0f;
      cmb.ccolor =
      ((wxUint32)(((cmb.ccolor >> 24) & 0xFF) * (1.0f-percent) + ((rdp.fog_color>>24) & 0xFF) * percent) << 24) |
      ((wxUint32)(((cmb.ccolor >> 16) & 0xFF) * (1.0f-percent) + ((rdp.fog_color>>16) & 0xFF) * percent) << 16) |
      ((wxUint32)(((cmb.ccolor >> 8) & 0xFF) * (1.0f-percent) + ((rdp.fog_color>>8) & 0xFF) * percent) << 8) |
      (cmb.ccolor & 0xFF);

      rdp.col[0] = rdp.col[0] * (1.0f-percent) + ((rdp.fog_color>>24) & 0xFF) / 255.0f * percent;
      rdp.col[1] = rdp.col[1] * (1.0f-percent) + ((rdp.fog_color>>16) & 0xFF) / 255.0f * percent;
      rdp.col[2] = rdp.col[2] * (1.0f-percent) + ((rdp.fog_color>>8) & 0xFF) / 255.0f * percent;
      A_BLEND (GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA);
      }
      break;
      */
    case 0xf550: //clr_fog * a_fog + clr_mem * (1-a)
      A_BLEND (GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA);
      {
        wxUint32 prim = rdp.prim_color;
        rdp.prim_color = rdp.fog_color;
        cc_prim();
        ac_prim();
        rdp.prim_color = prim;
      }
      break;

    case 0x0150: //spiderman
    case 0x0d18: //clr_in * a_fog + clr_mem * (1-a)
      A_BLEND (GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA);
      if (rdp.cycle_mode == 1 && rdp.cycle2 != 0x01ff1fff)
      {
        wxUint32 prim = rdp.prim_color;
        rdp.prim_color = rdp.fog_color;
        ac_prim();
        rdp.prim_color = prim;
      }
      break;

    case 0xc912: //40 winks, clr_in * a_fog + clr_mem * 1
      {
        wxUint32 prim = rdp.prim_color;
        rdp.prim_color = rdp.fog_color;
        ac_prim();
        rdp.prim_color = prim;
      }
      A_BLEND (GR_BLEND_SRC_ALPHA, GR_BLEND_ONE);
      break;

    default:
      A_BLEND (GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA);
    }
  }
  /*
  else if (blendmode == 0xc411) // Super Smash Bros, faked fog for flashing characters
  {
  MOD_0 (TMOD_TEX_INTER_COLOR_USING_FACTOR);
  MOD_0_COL (rdp.fog_color & 0xFFFFFF00);
  MOD_0_FAC (rdp.fog_color & 0xFF);
  INTERSHADE_2 (rdp.fog_color & 0xFFFFFF00, rdp.fog_color & 0xFF);

  float percent = (rdp.fog_color & 0xFF) / 255.0f;
  cmb.ccolor =
  ((wxUint32)(((cmb.ccolor >> 24) & 0xFF) * (1.0f-percent) + ((rdp.fog_color>>24) & 0xFF) * percent) << 24) |
  ((wxUint32)(((cmb.ccolor >> 16) & 0xFF) * (1.0f-percent) + ((rdp.fog_color>>16) & 0xFF) * percent) << 16) |
  ((wxUint32)(((cmb.ccolor >> 8) & 0xFF) * (1.0f-percent) + ((rdp.fog_color>>8) & 0xFF) * percent) << 8) |
  (cmb.ccolor & 0xFF);

  rdp.col[0] = rdp.col[0] * (1.0f-percent) + ((rdp.fog_color>>24) & 0xFF) / 255.0f * percent;
  rdp.col[1] = rdp.col[1] * (1.0f-percent) + ((rdp.fog_color>>16) & 0xFF) / 255.0f * percent;
  rdp.col[2] = rdp.col[2] * (1.0f-percent) + ((rdp.fog_color>>8) & 0xFF) / 255.0f * percent;
  A_BLEND (GR_BLEND_ONE, GR_BLEND_ZERO);
  }
  */
  else if (blendmode == 0x0040) // Mia Soccer Lights
    A_BLEND (GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA);
  else if ((settings.hacks&hack_Pilotwings) && (rdp.othermode_l & 0x80)) //CLR_ON_CVG without FORCE_BL
    A_BLEND (GR_BLEND_ZERO, GR_BLEND_ONE);
  else
    A_BLEND (GR_BLEND_ONE, GR_BLEND_ZERO);

  // ALPHA_CVG_SEL means full alpha
  // The reason it wasn't working before was because I wasn't handling rdp:setothermode
  //  if (rdp.othermode_l & 0x2000)
  if ((rdp.othermode_l & 0x2000) && ((rdp.othermode_l & 0x7000) != 0x7000))
  {
    if ((settings.hacks&hack_PMario) && (blendmode == 0x5055))
    {
      A_BLEND (GR_BLEND_ZERO, GR_BLEND_ONE);
    }
    else if (blendmode == 0x4055) // Mario Golf
    {
      A_BLEND (GR_BLEND_ZERO, GR_BLEND_ONE);
    }
    else
    {
      A_BLEND (GR_BLEND_ONE, GR_BLEND_ZERO);
    }
  }

  //hack
  //*
  if (settings.hacks&hack_ISS64)
  {
    if (rdp.othermode_l == 0xff5a6379)
    {
      A_BLEND (GR_BLEND_ZERO, GR_BLEND_SRC_ALPHA);
    }
    else if (rdp.othermode_l == 0x00504dd9) //players shadows. CVG_DST_WRAP
    {
      A_BLEND (GR_BLEND_ZERO, GR_BLEND_ONE);
    }
  }
  else if (settings.hacks&hack_TGR)
  {
    if (rdp.othermode_l == 0x0f0a0235)
    {
      A_BLEND (GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA);
    }
  }
  //*/
}

void InitCombine ()
{
  LOG ("InitCombine() ");
  memset(&cmb, 0, sizeof(cmb));
  const char *extensions = grGetString (GR_EXTENSION);
  if (const char * extstr = strstr(extensions, "COMBINE")) {
    if (!strncmp(extstr, "COMBINE", 7)) {
      LOG ("extensions ");
      char strColorCombineExt[] = "grColorCombineExt";
      cmb.grColorCombineExt = (GRCOLORCOMBINEEXT) grGetProcAddress(strColorCombineExt);
      char strAlphaCombineExt[] = "grAlphaCombineExt";
      cmb.grAlphaCombineExt = (GRCOLORCOMBINEEXT) grGetProcAddress(strAlphaCombineExt);
      char strTexColorCombineExt[] = "grTexColorCombineExt";
      cmb.grTexColorCombineExt = (GRTEXCOLORCOMBINEEXT) grGetProcAddress(strTexColorCombineExt);
      char strTexAlphaCombineExt[] = "grTexAlphaCombineExt";
      cmb.grTexAlphaCombineExt = (GRTEXCOLORCOMBINEEXT) grGetProcAddress(strTexAlphaCombineExt);
      char strConstantColorValueExt[] = "grConstantColorValueExt";
      cmb.grConstantColorValueExt = (GRCONSTANTCOLORVALUEEXT) grGetProcAddress(strConstantColorValueExt);
      if (cmb.grColorCombineExt && cmb.grAlphaCombineExt &&
        cmb.grTexColorCombineExt && cmb.grTexAlphaCombineExt)
      {
        cmb.combine_ext = TRUE;
        LOG ("initialized.");
      }
      else
      {
        cmb.combine_ext = FALSE;
      }
    }
  }
  cmb.dc0_lodbias = cmb.dc1_lodbias = 31;
  cmb.dc0_detailscale = cmb.dc1_detailscale = 7;
  cmb.lodbias0 = cmb.lodbias1 = 1.0f;
  LOG ("\n");
}

void ColorCombinerToExtension ()
{
  wxUint32 ext_local, ext_local_a, ext_other, ext_other_a;
  switch (cmb.c_loc)
  {
  case GR_COMBINE_LOCAL_ITERATED:
    ext_local = GR_CMBX_ITRGB;
    ext_local_a = GR_CMBX_ITALPHA;
    break;
  case GR_COMBINE_LOCAL_CONSTANT:
    ext_local = GR_CMBX_CONSTANT_COLOR;
    ext_local_a = GR_CMBX_CONSTANT_ALPHA;
    break;
  };
  switch (cmb.c_oth)
  {
  case GR_COMBINE_OTHER_ITERATED:
    ext_other = GR_CMBX_ITRGB;
    ext_other_a = GR_CMBX_ITALPHA;
    break;
  case GR_COMBINE_OTHER_TEXTURE:
    ext_other = GR_CMBX_TEXTURE_RGB;
    ext_other_a = GR_CMBX_TEXTURE_ALPHA;
    break;
  case GR_COMBINE_OTHER_CONSTANT:
    ext_other = GR_CMBX_CONSTANT_COLOR;
    ext_other_a = GR_CMBX_CONSTANT_ALPHA;
    break;
  };
  switch (cmb.c_fac)
  {
  case GR_COMBINE_FACTOR_ZERO:
    cmb.c_ext_c = GR_CMBX_ZERO;
    cmb.c_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_ONE:
    cmb.c_ext_c = GR_CMBX_ZERO;
    cmb.c_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_LOCAL:
    cmb.c_ext_c = ext_local;
    cmb.c_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_LOCAL_ALPHA:
    cmb.c_ext_c = ext_local_a;
    cmb.c_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_OTHER_ALPHA:
    cmb.c_ext_c = ext_other_a;
    cmb.c_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_TEXTURE_RGB:
    cmb.c_ext_c = GR_CMBX_TEXTURE_RGB;
    cmb.c_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_TEXTURE_ALPHA:
    cmb.c_ext_c = GR_CMBX_TEXTURE_ALPHA;
    cmb.c_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL:
    cmb.c_ext_c = ext_local;
    cmb.c_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA:
    cmb.c_ext_c = ext_local_a;
    cmb.c_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA:
    cmb.c_ext_c = ext_other_a;
    cmb.c_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA:
    cmb.c_ext_c = GR_CMBX_TEXTURE_ALPHA;
    cmb.c_ext_c_invert = 1;
    break;
  }

  switch (cmb.c_fnc)
  {
  case GR_COMBINE_FUNCTION_ZERO:
    cmb.c_ext_a = GR_CMBX_ZERO;
    cmb.c_ext_a_mode = GR_FUNC_MODE_X;
    cmb.c_ext_b = GR_CMBX_ZERO;
    cmb.c_ext_b_mode = GR_FUNC_MODE_X;
    cmb.c_ext_c = GR_CMBX_ZERO;
    cmb.c_ext_c_invert = 0;
    cmb.c_ext_d = GR_CMBX_ZERO;
    cmb.c_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_LOCAL:
    cmb.c_ext_a = ext_local;
    cmb.c_ext_a_mode = GR_FUNC_MODE_X;
    cmb.c_ext_b = GR_CMBX_ZERO;
    cmb.c_ext_b_mode = GR_FUNC_MODE_X;
    cmb.c_ext_c = GR_CMBX_ZERO;
    cmb.c_ext_c_invert = 1;
    cmb.c_ext_d = GR_CMBX_ZERO;
    cmb.c_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
    cmb.c_ext_a = ext_local_a;
    cmb.c_ext_a_mode = GR_FUNC_MODE_X;
    cmb.c_ext_b = GR_CMBX_ZERO;
    cmb.c_ext_b_mode = GR_FUNC_MODE_X;
    cmb.c_ext_c = GR_CMBX_ZERO;
    cmb.c_ext_c_invert = 1;
    cmb.c_ext_d = GR_CMBX_ZERO;
    cmb.c_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER:
    cmb.c_ext_a = ext_other;
    cmb.c_ext_a_mode = GR_FUNC_MODE_X;
    cmb.c_ext_b = GR_CMBX_ZERO;
    cmb.c_ext_b_mode = GR_FUNC_MODE_ZERO;
    cmb.c_ext_d = GR_CMBX_ZERO;
    cmb.c_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
    cmb.c_ext_a = ext_other;
    cmb.c_ext_a_mode = GR_FUNC_MODE_X;
    cmb.c_ext_b = ext_local;
    cmb.c_ext_b_mode = GR_FUNC_MODE_ZERO;
    cmb.c_ext_d = GR_CMBX_B;
    cmb.c_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
    cmb.c_ext_a = ext_other;
    cmb.c_ext_a_mode = GR_FUNC_MODE_X;
    cmb.c_ext_b = ext_local_a;
    cmb.c_ext_b_mode = GR_FUNC_MODE_ZERO;
    cmb.c_ext_d = GR_CMBX_B;
    cmb.c_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
    cmb.c_ext_a = ext_other;
    cmb.c_ext_a_mode = GR_FUNC_MODE_X;
    cmb.c_ext_b = ext_local;
    cmb.c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    cmb.c_ext_d = GR_CMBX_ZERO;
    cmb.c_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
    cmb.c_ext_a = ext_other;
    cmb.c_ext_a_mode = GR_FUNC_MODE_X;
    cmb.c_ext_b = ext_local;
    cmb.c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    cmb.c_ext_d = GR_CMBX_B;
    cmb.c_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    cmb.c_ext_a = ext_other;
    cmb.c_ext_a_mode = GR_FUNC_MODE_X;
    cmb.c_ext_b = ext_local;
    cmb.c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    cmb.c_ext_d = GR_CMBX_ALOCAL;
    cmb.c_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
    cmb.c_ext_a = GR_CMBX_ZERO;
    cmb.c_ext_a_mode = GR_FUNC_MODE_ZERO;
    cmb.c_ext_b = ext_local;
    cmb.c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    cmb.c_ext_d = GR_CMBX_B;
    cmb.c_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    cmb.c_ext_a = GR_CMBX_ZERO;
    cmb.c_ext_a_mode = GR_FUNC_MODE_ZERO;
    cmb.c_ext_b = ext_local;
    cmb.c_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    cmb.c_ext_d = GR_CMBX_ALOCAL;
    cmb.c_ext_d_invert = 0;
    break;
  }
}

void AlphaCombinerToExtension ()
{
  wxUint32 ext_local, ext_other;
  switch (cmb.a_loc)
  {
  case GR_COMBINE_LOCAL_ITERATED:
    ext_local = GR_CMBX_ITALPHA;
    break;
  case GR_COMBINE_LOCAL_CONSTANT:
    ext_local = GR_CMBX_CONSTANT_ALPHA;
    break;
  default:
    ext_local = GR_CMBX_ZERO;
  };
  switch (cmb.a_oth)
  {
  case GR_COMBINE_OTHER_ITERATED:
    ext_other = GR_CMBX_ITALPHA;
    break;
  case GR_COMBINE_OTHER_TEXTURE:
    ext_other = GR_CMBX_TEXTURE_ALPHA;
    break;
  case GR_COMBINE_OTHER_CONSTANT:
    ext_other = GR_CMBX_CONSTANT_ALPHA;
    break;
  default:
    ext_other = GR_CMBX_ZERO;
  };
  switch (cmb.a_fac)
  {
  case GR_COMBINE_FACTOR_ZERO:
    cmb.a_ext_c = GR_CMBX_ZERO;
    cmb.a_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_ONE:
    cmb.a_ext_c = GR_CMBX_ZERO;
    cmb.a_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_LOCAL:
  case GR_COMBINE_FACTOR_LOCAL_ALPHA:
    cmb.a_ext_c = ext_local;
    cmb.a_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_OTHER_ALPHA:
    cmb.a_ext_c = ext_other;
    cmb.a_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_TEXTURE_ALPHA:
    cmb.a_ext_c = GR_CMBX_TEXTURE_ALPHA;
    cmb.a_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL:
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA:
    cmb.a_ext_c = ext_local;
    cmb.a_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA:
    cmb.a_ext_c = ext_other;
    cmb.a_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA:
    cmb.a_ext_c = GR_CMBX_TEXTURE_ALPHA;
    cmb.a_ext_c_invert = 1;
    break;
  default:
    cmb.a_ext_c = GR_CMBX_ZERO;
    cmb.a_ext_c_invert = 0;
  }

  switch (cmb.a_fnc)
  {
  case GR_COMBINE_FUNCTION_ZERO:
    cmb.a_ext_a = GR_CMBX_ZERO;
    cmb.a_ext_a_mode = GR_FUNC_MODE_X;
    cmb.a_ext_b = GR_CMBX_ZERO;
    cmb.a_ext_b_mode = GR_FUNC_MODE_X;
    cmb.a_ext_c = GR_CMBX_ZERO;
    cmb.a_ext_c_invert = 0;
    cmb.a_ext_d = GR_CMBX_ZERO;
    cmb.a_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_LOCAL:
  case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
    cmb.a_ext_a = GR_CMBX_ZERO;
    cmb.a_ext_a_mode = GR_FUNC_MODE_ZERO;
    cmb.a_ext_b = ext_local;
    cmb.a_ext_b_mode = GR_FUNC_MODE_X;
    cmb.a_ext_c = GR_CMBX_ZERO;
    cmb.a_ext_c_invert = 1;
    cmb.a_ext_d = GR_CMBX_ZERO;
    cmb.a_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER:
    cmb.a_ext_a = ext_other;
    cmb.a_ext_a_mode = GR_FUNC_MODE_X;
    cmb.a_ext_b = GR_CMBX_ZERO;
    cmb.a_ext_b_mode = GR_FUNC_MODE_ZERO;
    cmb.a_ext_d = GR_CMBX_ZERO;
    cmb.a_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
    cmb.a_ext_a = ext_other;
    cmb.a_ext_a_mode = GR_FUNC_MODE_X;
    cmb.a_ext_b = ext_local;
    cmb.a_ext_b_mode = GR_FUNC_MODE_ZERO;
    cmb.a_ext_d = GR_CMBX_B;
    cmb.a_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
    cmb.a_ext_a = ext_other;
    cmb.a_ext_a_mode = GR_FUNC_MODE_X;
    cmb.a_ext_b = ext_local;
    cmb.a_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    cmb.a_ext_d = GR_CMBX_ZERO;
    cmb.a_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    cmb.a_ext_a = ext_other;
    cmb.a_ext_a_mode = GR_FUNC_MODE_X;
    cmb.a_ext_b = ext_local;
    cmb.a_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    cmb.a_ext_d = GR_CMBX_B;
    cmb.a_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    cmb.a_ext_a = GR_CMBX_ZERO;
    cmb.a_ext_a_mode = GR_FUNC_MODE_ZERO;
    cmb.a_ext_b = ext_local;
    cmb.a_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    cmb.a_ext_d = GR_CMBX_B;
    cmb.a_ext_d_invert = 0;
    break;
  }
}

void TexColorCombinerToExtension (GrChipID_t tmu)
{
  wxUint32 tc_ext_a, tc_ext_a_mode, tc_ext_b, tc_ext_b_mode, tc_ext_c, tc_ext_d;
  int  tc_ext_c_invert, tc_ext_d_invert;
  wxUint32 tmu_func, tmu_fac;

  if (tmu == GR_TMU0)
  {
    tmu_func = cmb.tmu0_func;
    tmu_fac = cmb.tmu0_fac;
  }
  else
  {
    tmu_func = cmb.tmu1_func;
    tmu_fac = cmb.tmu1_fac;
  }

  switch (tmu_fac)
  {
  case GR_COMBINE_FACTOR_ZERO:
    tc_ext_c = GR_CMBX_ZERO;
    tc_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_LOCAL:
    tc_ext_c = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_LOCAL_ALPHA:
    tc_ext_c = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    tc_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_OTHER_ALPHA:
    tc_ext_c = GR_CMBX_OTHER_TEXTURE_ALPHA;
    tc_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_DETAIL_FACTOR:
    tc_ext_c = GR_CMBX_DETAIL_FACTOR;
    tc_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_ONE:
    tc_ext_c = GR_CMBX_ZERO;
    tc_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL:
    tc_ext_c = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA:
    tc_ext_c = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    tc_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA:
    tc_ext_c = GR_CMBX_OTHER_TEXTURE_ALPHA;
    tc_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR:
    tc_ext_c = GR_CMBX_DETAIL_FACTOR;
    tc_ext_c_invert = 1;
    break;
  }

  switch (tmu_func)
  {
  case GR_COMBINE_FUNCTION_ZERO:
    tc_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_a_mode = GR_FUNC_MODE_ZERO;
    tc_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_b_mode = GR_FUNC_MODE_ZERO;
    tc_ext_c = GR_CMBX_ZERO;
    tc_ext_c_invert = 0;
    tc_ext_d = GR_CMBX_ZERO;
    tc_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_LOCAL:
    tc_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_a_mode = GR_FUNC_MODE_X;
    tc_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_b_mode = GR_FUNC_MODE_ZERO;
    tc_ext_c = GR_CMBX_ZERO;
    tc_ext_c_invert = 1;
    tc_ext_d = GR_CMBX_ZERO;
    tc_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
    tc_ext_a = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    tc_ext_a_mode = GR_FUNC_MODE_X;
    tc_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_b_mode = GR_FUNC_MODE_ZERO;
    tc_ext_c = GR_CMBX_ZERO;
    tc_ext_c_invert = 1;
    tc_ext_d = GR_CMBX_ZERO;
    tc_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER:
    tc_ext_a = GR_CMBX_OTHER_TEXTURE_RGB;
    tc_ext_a_mode = GR_FUNC_MODE_X;
    tc_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_b_mode = GR_FUNC_MODE_ZERO;
    tc_ext_d = GR_CMBX_ZERO;
    tc_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
    tc_ext_a = GR_CMBX_OTHER_TEXTURE_RGB;
    tc_ext_a_mode = GR_FUNC_MODE_X;
    tc_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_b_mode = GR_FUNC_MODE_ZERO;
    tc_ext_d = GR_CMBX_B;
    tc_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
    tc_ext_a = GR_CMBX_OTHER_TEXTURE_RGB;
    tc_ext_a_mode = GR_FUNC_MODE_X;
    tc_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    tc_ext_b_mode = GR_FUNC_MODE_ZERO;
    tc_ext_d = GR_CMBX_B;
    tc_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
    tc_ext_a = GR_CMBX_OTHER_TEXTURE_RGB;
    tc_ext_a_mode = GR_FUNC_MODE_X;
    tc_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    tc_ext_d = GR_CMBX_ZERO;
    tc_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
    tc_ext_a = GR_CMBX_OTHER_TEXTURE_RGB;
    tc_ext_a_mode = GR_FUNC_MODE_X;
    tc_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    tc_ext_d = GR_CMBX_B;
    tc_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    tc_ext_a = GR_CMBX_OTHER_TEXTURE_RGB;
    tc_ext_a_mode = GR_FUNC_MODE_X;
    tc_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    tc_ext_d = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    tc_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
    tc_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_a_mode = GR_FUNC_MODE_ZERO;
    tc_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    tc_ext_d = GR_CMBX_B;
    tc_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    tc_ext_a = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_a_mode = GR_FUNC_MODE_ZERO;
    tc_ext_b = GR_CMBX_LOCAL_TEXTURE_RGB;
    tc_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    tc_ext_d = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    tc_ext_d_invert = 0;
    break;
  }

  if (tmu == GR_TMU0)
  {
    cmb.t0c_ext_a = tc_ext_a;
    cmb.t0c_ext_a_mode = tc_ext_a_mode;
    cmb.t0c_ext_b = tc_ext_b;
    cmb.t0c_ext_b_mode = tc_ext_b_mode;
    cmb.t0c_ext_c = tc_ext_c;
    cmb.t0c_ext_c_invert = tc_ext_c_invert;
    cmb.t0c_ext_d = tc_ext_d;
    cmb.t0c_ext_d_invert = tc_ext_d_invert;
  }
  else
  {
    cmb.t1c_ext_a = tc_ext_a;
    cmb.t1c_ext_a_mode = tc_ext_a_mode;
    cmb.t1c_ext_b = tc_ext_b;
    cmb.t1c_ext_b_mode = tc_ext_b_mode;
    cmb.t1c_ext_c = tc_ext_c;
    cmb.t1c_ext_c_invert = tc_ext_c_invert;
    cmb.t1c_ext_d = tc_ext_d;
    cmb.t1c_ext_d_invert = tc_ext_d_invert;
  }
}

void TexAlphaCombinerToExtension (GrChipID_t tmu)
{
  wxUint32 ta_ext_a, ta_ext_a_mode, ta_ext_b, ta_ext_b_mode, ta_ext_c, ta_ext_d;
  int  ta_ext_c_invert, ta_ext_d_invert;
  wxUint32 tmu_a_func, tmu_a_fac;

  if (tmu == GR_TMU0)
  {
    tmu_a_func = cmb.tmu0_a_func;
    tmu_a_fac = cmb.tmu0_a_fac;
  }
  else
  {
    tmu_a_func = cmb.tmu1_a_func;
    tmu_a_fac = cmb.tmu1_a_fac;
  }

  switch (tmu_a_fac)
  {
  case GR_COMBINE_FACTOR_ZERO:
    ta_ext_c = GR_CMBX_ZERO;
    ta_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_LOCAL:
  case GR_COMBINE_FACTOR_LOCAL_ALPHA:
    ta_ext_c = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_OTHER_ALPHA:
    ta_ext_c = GR_CMBX_OTHER_TEXTURE_ALPHA;
    ta_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_DETAIL_FACTOR:
    ta_ext_c = GR_CMBX_DETAIL_FACTOR;
    ta_ext_c_invert = 0;
    break;
  case GR_COMBINE_FACTOR_ONE:
    ta_ext_c = GR_CMBX_ZERO;
    ta_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL:
  case GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA:
    ta_ext_c = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA:
    ta_ext_c = GR_CMBX_OTHER_TEXTURE_ALPHA;
    ta_ext_c_invert = 1;
    break;
  case GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR:
    ta_ext_c = GR_CMBX_DETAIL_FACTOR;
    ta_ext_c_invert = 1;
    break;
  }

  switch (tmu_a_func)
  {
  case GR_COMBINE_FUNCTION_ZERO:
    ta_ext_a = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_a_mode = GR_FUNC_MODE_ZERO;
    ta_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_b_mode = GR_FUNC_MODE_ZERO;
    ta_ext_c = GR_CMBX_ZERO;
    ta_ext_c_invert = 0;
    ta_ext_d = GR_CMBX_ZERO;
    ta_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_LOCAL:
  case GR_COMBINE_FUNCTION_LOCAL_ALPHA:
    ta_ext_a = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_a_mode = GR_FUNC_MODE_X;
    ta_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_b_mode = GR_FUNC_MODE_ZERO;
    ta_ext_c = GR_CMBX_ZERO;
    ta_ext_c_invert = 1;
    ta_ext_d = GR_CMBX_ZERO;
    ta_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER:
    ta_ext_a = GR_CMBX_OTHER_TEXTURE_ALPHA;
    ta_ext_a_mode = GR_FUNC_MODE_X;
    ta_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_b_mode = GR_FUNC_MODE_ZERO;
    ta_ext_d = GR_CMBX_ZERO;
    ta_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL:
  case GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA:
    ta_ext_a = GR_CMBX_OTHER_TEXTURE_ALPHA;
    ta_ext_a_mode = GR_FUNC_MODE_X;
    ta_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_b_mode = GR_FUNC_MODE_ZERO;
    ta_ext_d = GR_CMBX_B;
    ta_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL:
    ta_ext_a = GR_CMBX_OTHER_TEXTURE_ALPHA;
    ta_ext_a_mode = GR_FUNC_MODE_X;
    ta_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    ta_ext_d = GR_CMBX_ZERO;
    ta_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL:
  case GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    ta_ext_a = GR_CMBX_OTHER_TEXTURE_ALPHA;
    ta_ext_a_mode = GR_FUNC_MODE_X;
    ta_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    ta_ext_d = GR_CMBX_B;
    ta_ext_d_invert = 0;
    break;
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL:
  case GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA:
    ta_ext_a = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_a_mode = GR_FUNC_MODE_ZERO;
    ta_ext_b = GR_CMBX_LOCAL_TEXTURE_ALPHA;
    ta_ext_b_mode = GR_FUNC_MODE_NEGATIVE_X;
    ta_ext_d = GR_CMBX_B;
    ta_ext_d_invert = 0;
    break;
  }

  if (tmu == GR_TMU0)
  {
    cmb.t0a_ext_a = ta_ext_a;
    cmb.t0a_ext_a_mode = ta_ext_a_mode;
    cmb.t0a_ext_b = ta_ext_b;
    cmb.t0a_ext_b_mode = ta_ext_b_mode;
    cmb.t0a_ext_c = ta_ext_c;
    cmb.t0a_ext_c_invert = ta_ext_c_invert;
    cmb.t0a_ext_d = ta_ext_d;
    cmb.t0a_ext_d_invert = ta_ext_d_invert;
  }
  else
  {
    cmb.t1a_ext_a = ta_ext_a;
    cmb.t1a_ext_a_mode = ta_ext_a_mode;
    cmb.t1a_ext_b = ta_ext_b;
    cmb.t1a_ext_b_mode = ta_ext_b_mode;
    cmb.t1a_ext_c = ta_ext_c;
    cmb.t1a_ext_c_invert = ta_ext_c_invert;
    cmb.t1a_ext_d = ta_ext_d;
    cmb.t1a_ext_d_invert = ta_ext_d_invert;
  }
}
