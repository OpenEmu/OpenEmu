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
// December 2008 Created by Gonetz (Gonetz@ngs.ru)
//
//****************************************************************

void uc9_rpdcmd ();

typedef float M44[4][4];

struct ZSORTRDP {
  float view_scale[2];
  float view_trans[2];
  float scale_x;
  float scale_y;
} zSortRdp = {{0, 0}, {0, 0}, 0, 0};

//RSP command VRCPL
static int Calc_invw (int w) {
	int count, neg;
  union {
	wxInt32		  W;
	wxUint32	  UW;
	wxInt16			HW[2];
	wxUint16		UHW[2];
  } Result;
	Result.W = w;
	if (Result.UW == 0) {
		Result.UW = 0x7FFFFFFF;
	} else {
		if (Result.W < 0) {
			neg = TRUE;
			if (Result.UHW[1] == 0xFFFF && Result.HW[0] < 0) {
				Result.W = ~Result.W + 1;
			} else {
				Result.W = ~Result.W;
			}
		} else {
			neg = FALSE;
		}
		for (count = 31; count > 0; count--) {
			if ((Result.W & (1 << count))) {
				Result.W &= (0xFFC00000 >> (31 - count) );
				count = 0;
			}
		}
		Result.W = 0x7FFFFFFF / Result.W;
		for (count = 31; count > 0; count--) {
			if ((Result.W & (1 << count))) {
				Result.W &= (0xFFFF8000 >> (31 - count) );
				count = 0;
			}
		}
		if (neg == TRUE) {
			Result.W = ~Result.W;
		}
	}
  return Result.W;
}

static void uc9_draw_object (wxUint8 * addr, wxUint32 type)
{
  wxUint32 textured, vnum, vsize;
  switch (type) {
    case 0: //null
      textured = vnum = vsize = 0;
      break;
    case 1: //sh tri
      textured = 0;
      vnum = 3;
      vsize = 8;
      break;
    case 2: //tx tri
      textured = 1;
      vnum = 3;
      vsize = 16;
      break;
    case 3: //sh quad
      textured = 0;
      vnum = 4;
      vsize = 8;
      break;
    case 4: //tx quad
      textured = 1;
      vnum = 4;
      vsize = 16;
      break;
  }
  VERTEX vtx[4];
  for (wxUint32 i = 0; i < vnum; i++)
  {
    VERTEX &v = vtx[i];
    v.sx = zSortRdp.scale_x * ((short*)addr)[0^1];
    v.sy = zSortRdp.scale_y * ((short*)addr)[1^1];
    v.sz = 1.0f;
    v.r = addr[4^3];
    v.g = addr[5^3];
    v.b = addr[6^3];
    v.a = addr[7^3];
    v.flags = 0;
    v.uv_scaled = 0;
    v.uv_calculated = 0xFFFFFFFF;
    v.shade_mod = 0;
    v.scr_off = 0;
    v.screen_translated = 2;
    if (textured)
    {
      v.ou = ((short*)addr)[4^1];
      v.ov = ((short*)addr)[5^1];
      v.w = Calc_invw(((int*)addr)[3]) / 31.0f;
      v.oow = 1.0f / v.w;
      FRDP ("v%d - sx: %f, sy: %f ou: %f, ov: %f, w: %f, r=%d, g=%d, b=%d, a=%d\n", i, v.sx/rdp.scale_x, v.sy/rdp.scale_y, v.ou*rdp.tiles[rdp.cur_tile].s_scale, v.ov*rdp.tiles[rdp.cur_tile].t_scale, v.w, v.r, v.g, v.b, v.a);
    }
    else
    {
      v.oow = v.w = 1.0f;
      FRDP ("v%d - sx: %f, sy: %f r=%d, g=%d, b=%d, a=%d\n", i, v.sx/rdp.scale_x, v.sy/rdp.scale_y, v.r, v.g, v.b, v.a);
    }
    addr += vsize;
  }
  //*
  VERTEX *pV[4] = {
    &vtx[0],
    &vtx[1],
    &vtx[2],
    &vtx[3]
  };
  if (vnum == 3)
  {
    FRDP("uc9:Tri #%d, #%d\n", rdp.tri_n, rdp.tri_n+1);
    draw_tri (pV, 0);
    rdp.tri_n ++;
  }
  else
  {
    FRDP("uc9:Quad #%d, #%d\n", rdp.tri_n, rdp.tri_n+1);
    draw_tri (pV, 0);
    draw_tri (pV+1, 0);
    rdp.tri_n += 2;
  }
}

static wxUint32 uc9_load_object (wxUint32 zHeader, wxUint32 * rdpcmds)
{
  wxUint32 type = zHeader & 7;
  wxUint8 * addr = gfx.RDRAM + (zHeader&0xFFFFFFF8);
  switch (type) {
    case 1: //sh tri
    case 3: //sh quad
      {
        rdp.cmd1 = ((wxUint32*)addr)[1];
        if (rdp.cmd1 != rdpcmds[0])
        {
          rdpcmds[0] = rdp.cmd1;
          uc9_rpdcmd ();
        }
        update ();
        uc9_draw_object(addr + 8, type);
      }
      break;
    case 0: //null
    case 2: //tx tri
    case 4: //tx quad
      {
        rdp.cmd1 = ((wxUint32*)addr)[1];
        if (rdp.cmd1 != rdpcmds[0])
        {
          rdpcmds[0] = rdp.cmd1;
          uc9_rpdcmd ();
        }
        rdp.cmd1 = ((wxUint32*)addr)[2];
        if (rdp.cmd1 != rdpcmds[1])
        {
          uc9_rpdcmd ();
          rdpcmds[1] = rdp.cmd1;
        }
        rdp.cmd1 = ((wxUint32*)addr)[3];
        if (rdp.cmd1 != rdpcmds[2])
        {
          uc9_rpdcmd ();
          rdpcmds[2] = rdp.cmd1;
        }
        if (type)
        {
          update ();
          uc9_draw_object(addr + 16, type);
        }
      }
      break;
  }
  return segoffset(((wxUint32*)addr)[0]);
}

static void uc9_object ()
{
  LRDP("uc9:object\n");
  wxUint32 rdpcmds[3] = {0, 0, 0};
  wxUint32 cmd1 = rdp.cmd1;
  wxUint32 zHeader = segoffset(rdp.cmd0);
  while (zHeader)
    zHeader = uc9_load_object(zHeader, rdpcmds);
  zHeader = segoffset(cmd1);
  while (zHeader)
    zHeader = uc9_load_object(zHeader, rdpcmds);
}

static void uc9_mix ()
{
  LRDP("uc9:mix IGNORED\n");
}

static void uc9_fmlight ()
{
  int mid = rdp.cmd0&0xFF;
  rdp.num_lights = 1 + ((rdp.cmd1>>12)&0xFF);
  wxUint32 a = -1024 + (rdp.cmd1&0xFFF);
  FRDP ("uc9:fmlight matrix: %d, num: %d, dmem: %04lx\n", mid, rdp.num_lights, a);

  M44 *m;
  switch (mid) {
  case 4:
    m = (M44*)rdp.model;
    break;
  case 6:
    m = (M44*)rdp.proj;
    break;
  case 8:
    m = (M44*)rdp.combined;
    break;
  }

  rdp.light[rdp.num_lights].r = (float)(((wxUint8*)gfx.DMEM)[(a+0)^3]) / 255.0f;
  rdp.light[rdp.num_lights].g = (float)(((wxUint8*)gfx.DMEM)[(a+1)^3]) / 255.0f;
  rdp.light[rdp.num_lights].b = (float)(((wxUint8*)gfx.DMEM)[(a+2)^3]) / 255.0f;
  rdp.light[rdp.num_lights].a = 1.0f;
  FRDP ("ambient light: r: %.3f, g: %.3f, b: %.3f\n", rdp.light[rdp.num_lights].r, rdp.light[rdp.num_lights].g, rdp.light[rdp.num_lights].b);
  a += 8;
  wxUint32 i;
  for (i = 0; i < rdp.num_lights; i++)
  {
    rdp.light[i].r = (float)(((wxUint8*)gfx.DMEM)[(a+0)^3]) / 255.0f;
    rdp.light[i].g = (float)(((wxUint8*)gfx.DMEM)[(a+1)^3]) / 255.0f;
    rdp.light[i].b = (float)(((wxUint8*)gfx.DMEM)[(a+2)^3]) / 255.0f;
    rdp.light[i].a = 1.0f;
    rdp.light[i].dir_x = (float)(((char*)gfx.DMEM)[(a+8)^3]) / 127.0f;
    rdp.light[i].dir_y = (float)(((char*)gfx.DMEM)[(a+9)^3]) / 127.0f;
    rdp.light[i].dir_z = (float)(((char*)gfx.DMEM)[(a+10)^3]) / 127.0f;
    FRDP ("light: n: %d, r: %.3f, g: %.3f, b: %.3f, x: %.3f, y: %.3f, z: %.3f\n",
      i, rdp.light[i].r, rdp.light[i].g, rdp.light[i].b,
      rdp.light[i].dir_x, rdp.light[i].dir_y, rdp.light[i].dir_z);
//    TransformVector(&rdp.light[i].dir_x, rdp.light_vector[i], *m);
    InverseTransformVector(&rdp.light[i].dir_x, rdp.light_vector[i], *m);
    NormalizeVector (rdp.light_vector[i]);
    FRDP ("light vector: n: %d, x: %.3f, y: %.3f, z: %.3f\n",
      i, rdp.light_vector[i][0], rdp.light_vector[i][1], rdp.light_vector[i][2]);
    a += 24;
  }
  for (i = 0; i < 2; i++)
  {
    float dir_x = (float)(((char*)gfx.DMEM)[(a+8)^3]) / 127.0f;
    float dir_y = (float)(((char*)gfx.DMEM)[(a+9)^3]) / 127.0f;
    float dir_z = (float)(((char*)gfx.DMEM)[(a+10)^3]) / 127.0f;
    if (sqrt(dir_x*dir_x + dir_y*dir_y + dir_z*dir_z) < 0.98)
    {
      rdp.use_lookat = FALSE;
      return;
    }
    rdp.lookat[i][0] = dir_x;
    rdp.lookat[i][1] = dir_y;
    rdp.lookat[i][2] = dir_z;
    a += 24;
  }
  rdp.use_lookat = TRUE;
}

static void uc9_light ()
{
  wxUint32 csrs = -1024 + ((rdp.cmd0>>12)&0xFFF);
  wxUint32 nsrs = -1024 + (rdp.cmd0&0xFFF);
  wxUint32 num = 1 + ((rdp.cmd1>>24)&0xFF);
  wxUint32 cdest = -1024 + ((rdp.cmd1>>12)&0xFFF);
  wxUint32 tdest = -1024 + (rdp.cmd1&0xFFF);
  int use_material = (csrs != 0x0ff0);
  tdest >>= 1;
  FRDP ("uc9:light n: %d, colsrs: %04lx, normales: %04lx, coldst: %04lx, texdst: %04lx\n", num, csrs, nsrs, cdest, tdest);
  VERTEX v;
  for (wxUint32 i = 0; i < num; i++)
  {
    v.vec[0] = ((char*)gfx.DMEM)[(nsrs++)^3];
    v.vec[1] = ((char*)gfx.DMEM)[(nsrs++)^3];
    v.vec[2] = ((char*)gfx.DMEM)[(nsrs++)^3];
    calc_sphere (&v);
//    calc_linear (&v);
    NormalizeVector (v.vec);
    calc_light (&v);
    v.a = 0xFF;
    if (use_material)
    {
      v.r = (wxUint8)(((wxUint32)v.r * gfx.DMEM[(csrs++)^3])>>8);
      v.g = (wxUint8)(((wxUint32)v.g * gfx.DMEM[(csrs++)^3])>>8);
      v.b = (wxUint8)(((wxUint32)v.b * gfx.DMEM[(csrs++)^3])>>8);
      v.a = gfx.DMEM[(csrs++)^3];
    }
    gfx.DMEM[(cdest++)^3] = v.r;
    gfx.DMEM[(cdest++)^3] = v.g;
    gfx.DMEM[(cdest++)^3] = v.b;
    gfx.DMEM[(cdest++)^3] = v.a;
    ((short*)gfx.DMEM)[(tdest++)^1] = (short)v.ou;
    ((short*)gfx.DMEM)[(tdest++)^1] = (short)v.ov;
  }
}

static void uc9_mtxtrnsp ()
{
  LRDP("uc9:mtxtrnsp - ignored\n");
  /*
  LRDP("uc9:mtxtrnsp ");
  M44 *s;
  switch (rdp.cmd1&0xF) {
  case 4:
    s = (M44*)rdp.model;
    LRDP("Model\n");
    break;
  case 6:
    s = (M44*)rdp.proj;
    LRDP("Proj\n");
    break;
  case 8:
    s = (M44*)rdp.combined;
    LRDP("Comb\n");
    break;
  }
  float m = *s[1][0];
  *s[1][0] = *s[0][1];
  *s[0][1] = m;
  m = *s[2][0];
  *s[2][0] = *s[0][2];
  *s[0][2] = m;
  m = *s[2][1];
  *s[2][1] = *s[1][2];
  *s[1][2] = m;
  */
}

static void uc9_mtxcat ()
{
  LRDP("uc9:mtxcat ");
  M44 *s;
  M44 *t;
  wxUint32 S = rdp.cmd0&0xF;
  wxUint32 T = (rdp.cmd1>>16)&0xF;
  wxUint32 D = rdp.cmd1&0xF;
  switch (S) {
  case 4:
    s = (M44*)rdp.model;
    LRDP("Model * ");
    break;
  case 6:
    s = (M44*)rdp.proj;
    LRDP("Proj * ");
    break;
  case 8:
    s = (M44*)rdp.combined;
    LRDP("Comb * ");
    break;
  }
  switch (T) {
  case 4:
    t = (M44*)rdp.model;
    LRDP("Model -> ");
    break;
  case 6:
    t = (M44*)rdp.proj;
    LRDP("Proj -> ");
    break;
  case 8:
    LRDP("Comb -> ");
    t = (M44*)rdp.combined;
    break;
  }
  DECLAREALIGN16VAR(m[4][4]);
  MulMatrices(*s, *t, m);

  switch (D) {
  case 4:
    memcpy (rdp.model, m, 64);;
    LRDP("Model\n");
    break;
  case 6:
    memcpy (rdp.proj, m, 64);;
    LRDP("Proj\n");
    break;
  case 8:
    memcpy (rdp.combined, m, 64);;
    LRDP("Comb\n");
    break;
  }
#ifdef EXTREME_LOGGING
  FRDP ("\nmodel\n{%f,%f,%f,%f}\n", rdp.model[0][0], rdp.model[0][1], rdp.model[0][2], rdp.model[0][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.model[1][0], rdp.model[1][1], rdp.model[1][2], rdp.model[1][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.model[2][0], rdp.model[2][1], rdp.model[2][2], rdp.model[2][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.model[3][0], rdp.model[3][1], rdp.model[3][2], rdp.model[3][3]);
  FRDP ("\nproj\n{%f,%f,%f,%f}\n", rdp.proj[0][0], rdp.proj[0][1], rdp.proj[0][2], rdp.proj[0][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.proj[1][0], rdp.proj[1][1], rdp.proj[1][2], rdp.proj[1][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.proj[2][0], rdp.proj[2][1], rdp.proj[2][2], rdp.proj[2][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.proj[3][0], rdp.proj[3][1], rdp.proj[3][2], rdp.proj[3][3]);
  FRDP ("\ncombined\n{%f,%f,%f,%f}\n", rdp.combined[0][0], rdp.combined[0][1], rdp.combined[0][2], rdp.combined[0][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.combined[1][0], rdp.combined[1][1], rdp.combined[1][2], rdp.combined[1][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.combined[2][0], rdp.combined[2][1], rdp.combined[2][2], rdp.combined[2][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.combined[3][0], rdp.combined[3][1], rdp.combined[3][2], rdp.combined[3][3]);
#endif
}

typedef struct  {
  short sy;
  short sx;
  int   invw;
  short yi;
  short xi;
  short wi;
  wxUint8 fog;
  wxUint8 cc;
} zSortVDest;

static void uc9_mult_mpmtx ()
{
  //int id = rdp.cmd0&0xFF;
  int num = 1+ ((rdp.cmd1>>24)&0xFF);
  int src = -1024 + ((rdp.cmd1>>12)&0xFFF);
  int dst = -1024 + (rdp.cmd1&0xFFF);
  FRDP ("uc9:mult_mpmtx from: %04lx  to: %04lx n: %d\n", src, dst, num);
  short * saddr = (short*)(gfx.DMEM+src);
  zSortVDest * daddr = (zSortVDest*)(gfx.DMEM+dst);
  int idx = 0;
  zSortVDest v;
  memset(&v, 0, sizeof(zSortVDest));
  //float scale_x = 4.0f/rdp.scale_x;
  //float scale_y = 4.0f/rdp.scale_y;
  for (int i = 0; i < num; i++)
  {
    short sx   = saddr[(idx++)^1];
    short sy   = saddr[(idx++)^1];
    short sz   = saddr[(idx++)^1];
    float x = sx*rdp.combined[0][0] + sy*rdp.combined[1][0] + sz*rdp.combined[2][0] + rdp.combined[3][0];
    float y = sx*rdp.combined[0][1] + sy*rdp.combined[1][1] + sz*rdp.combined[2][1] + rdp.combined[3][1];
    float z = sx*rdp.combined[0][2] + sy*rdp.combined[1][2] + sz*rdp.combined[2][2] + rdp.combined[3][2];
    float w = sx*rdp.combined[0][3] + sy*rdp.combined[1][3] + sz*rdp.combined[2][3] + rdp.combined[3][3];
    v.sx = (short)(zSortRdp.view_trans[0] + x / w * zSortRdp.view_scale[0]);
    v.sy = (short)(zSortRdp.view_trans[1] + y / w * zSortRdp.view_scale[1]);

    v.xi = (short)x;
    v.yi = (short)y;
    v.wi = (short)w;
    v.invw = Calc_invw((int)(w * 31.0));

    if (w < 0.0f)
      v.fog = 0;
    else
    {
      int fog = (int)(z / w * rdp.fog_multiplier + rdp.fog_offset);
      if (fog > 255)
        fog = 255;
      v.fog = (fog >= 0) ? (wxUint8)fog : 0;
    }

    v.cc = 0;
    if (x < -w) v.cc |= 0x10;
    if (x > w) v.cc |= 0x01;
    if (y < -w) v.cc |= 0x20;
    if (y > w) v.cc |= 0x02;
    if (w < 0.1f) v.cc |= 0x04;

    daddr[i] = v;
    //memcpy(gfx.DMEM+dst+sizeof(zSortVDest)*i, &v, sizeof(zSortVDest));
//    FRDP("v%d x: %d, y: %d, z: %d -> sx: %d, sy: %d, w: %d, xi: %d, yi: %d, wi: %d, fog: %d\n", i, sx, sy, sz, v.sx, v.sy, v.invw, v.xi, v.yi, v.wi, v.fog);
    FRDP("v%d x: %d, y: %d, z: %d -> sx: %04lx, sy: %04lx, invw: %08lx - %f, xi: %04lx, yi: %04lx, wi: %04lx, fog: %04lx\n", i, sx, sy, sz, v.sx, v.sy, v.invw, w, v.xi, v.yi, v.wi, v.fog);
  }
}

static void uc9_link_subdl ()
{
  LRDP("uc9:link_subdl IGNORED\n");
}

static void uc9_set_subdl ()
{
  LRDP("uc9:set_subdl IGNORED\n");
}

static void uc9_wait_signal ()
{
  LRDP("uc9:wait_signal IGNORED\n");
}

static void uc9_send_signal ()
{
  LRDP("uc9:send_signal IGNORED\n");
}

void uc9_movemem ()
{
  LRDP("uc9:movemem\n");
  int idx = rdp.cmd0 & 0x0E;
  int ofs = ((rdp.cmd0>>6)&0x1ff)<<3;
  int len = (1 + ((rdp.cmd0>>15)&0x1ff))<<3;
  FRDP ("uc9:movemem ofs: %d, len: %d. ", ofs, len);
  int flag = rdp.cmd0 & 0x01;
  wxUint32 addr = segoffset(rdp.cmd1);
  switch (idx)
  {

  case 0: //save/load
    if (flag == 0)
    {
      int dmem_addr = (idx<<3) + ofs;
      FRDP ("Load to DMEM. %08lx -> %08lx\n", addr, dmem_addr);
      memcpy(gfx.DMEM + dmem_addr, gfx.RDRAM + addr, len);
    }
    else
    {
      int dmem_addr = (idx<<3) + ofs;
      FRDP ("Load from DMEM. %08lx -> %08lx\n", dmem_addr, addr);
      memcpy(gfx.RDRAM + addr, gfx.DMEM + dmem_addr, len);
    }
    break;

  case 4:  // model matrix
  case 6:  // projection matrix
  case 8:  // combined matrix
    {
      DECLAREALIGN16VAR(m[4][4]);
      load_matrix(m, addr);
      switch (idx)
      {
      case 4:  // model matrix
        LRDP("Modelview load\n");
        modelview_load (m);
        break;
      case 6:  // projection matrix
        LRDP("Projection load\n");
        projection_load (m);
        break;
      case 8:  // projection matrix
        LRDP("Combined load\n");
        rdp.update &= ~UPDATE_MULT_MAT;
        memcpy (rdp.combined, m, 64);;
        break;
      }
#ifdef EXTREME_LOGGING
  FRDP ("{%f,%f,%f,%f}\n", m[0][0], m[0][1], m[0][2], m[0][3]);
  FRDP ("{%f,%f,%f,%f}\n", m[1][0], m[1][1], m[1][2], m[1][3]);
  FRDP ("{%f,%f,%f,%f}\n", m[2][0], m[2][1], m[2][2], m[2][3]);
  FRDP ("{%f,%f,%f,%f}\n", m[3][0], m[3][1], m[3][2], m[3][3]);
  FRDP ("\nmodel\n{%f,%f,%f,%f}\n", rdp.model[0][0], rdp.model[0][1], rdp.model[0][2], rdp.model[0][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.model[1][0], rdp.model[1][1], rdp.model[1][2], rdp.model[1][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.model[2][0], rdp.model[2][1], rdp.model[2][2], rdp.model[2][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.model[3][0], rdp.model[3][1], rdp.model[3][2], rdp.model[3][3]);
  FRDP ("\nproj\n{%f,%f,%f,%f}\n", rdp.proj[0][0], rdp.proj[0][1], rdp.proj[0][2], rdp.proj[0][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.proj[1][0], rdp.proj[1][1], rdp.proj[1][2], rdp.proj[1][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.proj[2][0], rdp.proj[2][1], rdp.proj[2][2], rdp.proj[2][3]);
  FRDP ("{%f,%f,%f,%f}\n", rdp.proj[3][0], rdp.proj[3][1], rdp.proj[3][2], rdp.proj[3][3]);
#endif
    }
    break;

  case 10:
    LRDP("Othermode - IGNORED\n");
    break;

  case 12:   // VIEWPORT
    {
      wxUint32 a = addr >> 1;
      short scale_x = ((short*)gfx.RDRAM)[(a+0)^1] >> 2;
      short scale_y = ((short*)gfx.RDRAM)[(a+1)^1] >> 2;
      short scale_z = ((short*)gfx.RDRAM)[(a+2)^1];
      rdp.fog_multiplier = ((short*)gfx.RDRAM)[(a+3)^1];
      short trans_x = ((short*)gfx.RDRAM)[(a+4)^1] >> 2;
      short trans_y = ((short*)gfx.RDRAM)[(a+5)^1] >> 2;
      short trans_z = ((short*)gfx.RDRAM)[(a+6)^1];
      rdp.fog_offset = ((short*)gfx.RDRAM)[(a+7)^1];
      rdp.view_scale[0] = scale_x * rdp.scale_x;
      rdp.view_scale[1] = scale_y * rdp.scale_y;
      rdp.view_scale[2] = 32.0f * scale_z;
      rdp.view_trans[0] = trans_x * rdp.scale_x;
      rdp.view_trans[1] = trans_y * rdp.scale_y;
      rdp.view_trans[2] = 32.0f * trans_z;
      zSortRdp.view_scale[0] = (float)(scale_x*4);
      zSortRdp.view_scale[1] = (float)(scale_y*4);
      zSortRdp.view_trans[0] = (float)(trans_x*4);
      zSortRdp.view_trans[1] = (float)(trans_y*4);
      zSortRdp.scale_x = rdp.scale_x / 4.0f;
      zSortRdp.scale_y = rdp.scale_y / 4.0f;

      rdp.update |= UPDATE_VIEWPORT;

      rdp.mipmap_level = 0;
      rdp.cur_tile = 0;
      TILE *tmp_tile = &rdp.tiles[0];
      tmp_tile->on = 1;
      tmp_tile->org_s_scale = 0xFFFF;
      tmp_tile->org_t_scale = 0xFFFF;
      tmp_tile->s_scale = 0.031250f;
      tmp_tile->t_scale = 0.031250f;

      rdp.geom_mode |= 0x0200;

      FRDP ("viewport scale(%d, %d, %d), trans(%d, %d, %d), from:%08lx\n", scale_x, scale_y, scale_z,
        trans_x, trans_y, trans_z, a);
      FRDP ("fog: multiplier: %f, offset: %f\n", rdp.fog_multiplier, rdp.fog_offset);
    }
    break;

  default:
    FRDP ("** UNKNOWN %d\n", idx);
  }

}

static void uc9_setscissor()
{
  rdp_setscissor();

  if ((rdp.scissor_o.lr_x - rdp.scissor_o.ul_x) > (zSortRdp.view_scale[0] - zSortRdp.view_trans[0]))
  {
    float w = (rdp.scissor_o.lr_x - rdp.scissor_o.ul_x) / 2.0f;
    float h = (rdp.scissor_o.lr_y - rdp.scissor_o.ul_y) / 2.0f;
    rdp.view_scale[0] = w * rdp.scale_x;
    rdp.view_scale[1] = h * rdp.scale_y;
    rdp.view_trans[0] = w * rdp.scale_x;
    rdp.view_trans[1] = h * rdp.scale_y;
    zSortRdp.view_scale[0] = w * 4.0f;
    zSortRdp.view_scale[1] = h * 4.0f;
    zSortRdp.view_trans[0] = w * 4.0f;
    zSortRdp.view_trans[1] = h * 4.0f;
    zSortRdp.scale_x = rdp.scale_x / 4.0f;
    zSortRdp.scale_y = rdp.scale_y / 4.0f;
    rdp.update |= UPDATE_VIEWPORT;

    rdp.mipmap_level = 0;
    rdp.cur_tile = 0;
    TILE *tmp_tile = &rdp.tiles[0];
    tmp_tile->on = 1;
    tmp_tile->org_s_scale = 0xFFFF;
    tmp_tile->org_t_scale = 0xFFFF;
    tmp_tile->s_scale = 0.031250f;
    tmp_tile->t_scale = 0.031250f;

    rdp.geom_mode |= 0x0200;
  }
}
