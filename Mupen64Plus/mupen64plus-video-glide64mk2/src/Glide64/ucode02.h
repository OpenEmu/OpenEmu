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

static void calc_point_light (VERTEX *v, float * vpos)
{
  float light_intensity = 0.0f;
  register float color[3] = {rdp.light[rdp.num_lights].r, rdp.light[rdp.num_lights].g, rdp.light[rdp.num_lights].b};
  for (wxUint32 l=0; l<rdp.num_lights; l++)
  {
    if (rdp.light[l].nonblack)
    {
      float lvec[3] = {rdp.light[l].x, rdp.light[l].y, rdp.light[l].z};
   			lvec[0] -= vpos[0];
        lvec[1] -= vpos[1];
        lvec[2] -= vpos[2];
        float light_len2 = lvec[0]*lvec[0] + lvec[1]*lvec[1] + lvec[2]*lvec[2];
        float light_len = sqrtf(light_len2);
#ifdef EXTREME_LOGGING
        FRDP ("calc_point_light: len: %f, len2: %f\n", light_len, light_len2);
#endif
        float at = rdp.light[l].ca + light_len/65535.0f*rdp.light[l].la + light_len2/65535.0f*rdp.light[l].qa;
        if (at > 0.0f)
          light_intensity = 1/at;//DotProduct (lvec, nvec) / (light_len * normal_len * at);
        else
          light_intensity = 0.0f;
    }
    else
    {
      light_intensity = 0.0f;
    }
    if (light_intensity > 0.0f)
    {
      color[0] += rdp.light[l].r * light_intensity;
      color[1] += rdp.light[l].g * light_intensity;
      color[2] += rdp.light[l].b * light_intensity;
    }
  }
  if (color[0] > 1.0f) color[0] = 1.0f;
  if (color[1] > 1.0f) color[1] = 1.0f;
  if (color[2] > 1.0f) color[2] = 1.0f;

  v->r = (wxUint8)(color[0]*255.0f);
  v->g = (wxUint8)(color[1]*255.0f);
  v->b = (wxUint8)(color[2]*255.0f);
}

static void uc6_obj_rectangle();

static void uc2_vertex ()
{
  if (!(rdp.cmd0 & 0x00FFFFFF))
  {
    uc6_obj_rectangle();
    return;
  }

  // This is special, not handled in update(), but here
  // * Matrix Pre-multiplication idea by Gonetz (Gonetz@ngs.ru)
  if (rdp.update & UPDATE_MULT_MAT)
  {
    rdp.update ^= UPDATE_MULT_MAT;
    MulMatrices(rdp.model, rdp.proj, rdp.combined);
  }
  if (rdp.update & UPDATE_LIGHTS)
  {
    rdp.update ^= UPDATE_LIGHTS;

    // Calculate light vectors
    for (wxUint32 l=0; l<rdp.num_lights; l++)
    {
      InverseTransformVector(&rdp.light[l].dir_x, rdp.light_vector[l], rdp.model);
      NormalizeVector (rdp.light_vector[l]);
    }
  }

  wxUint32 addr = segoffset(rdp.cmd1);
  int v0, i, n;
  float x, y, z;

  rdp.vn = n = (rdp.cmd0 >> 12) & 0xFF;
  rdp.v0 = v0 = ((rdp.cmd0 >> 1) & 0x7F) - n;

  FRDP ("uc2:vertex n: %d, v0: %d, from: %08lx\n", n, v0, addr);

  if (v0 < 0)
  {
    RDP_E ("** ERROR: uc2:vertex v0 < 0\n");
    LRDP("** ERROR: uc2:vertex v0 < 0\n");
    return;
  }

  wxUint32 geom_mode = rdp.geom_mode;
  if ((settings.hacks&hack_Fzero) && (rdp.geom_mode & 0x40000))
  {
    if (((short*)gfx.RDRAM)[(((addr) >> 1) + 4)^1] || ((short*)gfx.RDRAM)[(((addr) >> 1) + 5)^1])
      rdp.geom_mode ^= 0x40000;
  }
  for (i=0; i < (n<<4); i+=16)
  {
    VERTEX *v = &rdp.vtx[v0 + (i>>4)];
    x   = (float)((short*)gfx.RDRAM)[(((addr+i) >> 1) + 0)^1];
    y   = (float)((short*)gfx.RDRAM)[(((addr+i) >> 1) + 1)^1];
    z   = (float)((short*)gfx.RDRAM)[(((addr+i) >> 1) + 2)^1];
    v->flags  = ((wxUint16*)gfx.RDRAM)[(((addr+i) >> 1) + 3)^1];
    v->ou   = (float)((short*)gfx.RDRAM)[(((addr+i) >> 1) + 4)^1];
    v->ov   = (float)((short*)gfx.RDRAM)[(((addr+i) >> 1) + 5)^1];
    v->uv_scaled = 0;
    v->a    = ((wxUint8*)gfx.RDRAM)[(addr+i + 15)^3];

    v->x = x*rdp.combined[0][0] + y*rdp.combined[1][0] + z*rdp.combined[2][0] + rdp.combined[3][0];
    v->y = x*rdp.combined[0][1] + y*rdp.combined[1][1] + z*rdp.combined[2][1] + rdp.combined[3][1];
    v->z = x*rdp.combined[0][2] + y*rdp.combined[1][2] + z*rdp.combined[2][2] + rdp.combined[3][2];
    v->w = x*rdp.combined[0][3] + y*rdp.combined[1][3] + z*rdp.combined[2][3] + rdp.combined[3][3];

    if (fabs(v->w) < 0.001) v->w = 0.001f;
    v->oow = 1.0f / v->w;
    v->x_w = v->x * v->oow;
    v->y_w = v->y * v->oow;
    v->z_w = v->z * v->oow;
    CalculateFog (v);

    v->uv_calculated = 0xFFFFFFFF;
    v->screen_translated = 0;
    v->shade_mod = 0;

    v->scr_off = 0;
    if (v->x < -v->w) v->scr_off |= 1;
    if (v->x > v->w) v->scr_off |= 2;
    if (v->y < -v->w) v->scr_off |= 4;
    if (v->y > v->w) v->scr_off |= 8;
    if (v->w < 0.1f) v->scr_off |= 16;
//    if (v->z_w > 1.0f) v->scr_off |= 32;

    if (rdp.geom_mode & 0x00020000)
    {
      v->vec[0] = ((char*)gfx.RDRAM)[(addr+i + 12)^3];
      v->vec[1] = ((char*)gfx.RDRAM)[(addr+i + 13)^3];
      v->vec[2] = ((char*)gfx.RDRAM)[(addr+i + 14)^3];
      //	  FRDP("Calc light. x: %f, y: %f z: %f\n", v->vec[0], v->vec[1], v->vec[2]);
      //      if (!(rdp.geom_mode & 0x800000))
      {
        if (rdp.geom_mode & 0x40000)
        {
          if (rdp.geom_mode & 0x80000)
          {
            calc_linear (v);
#ifdef EXTREME_LOGGING
            FRDP ("calc linear: v%d - u: %f, v: %f\n", i>>4, v->ou, v->ov);
#endif
          }
          else
          {
            calc_sphere (v);
#ifdef EXTREME_LOGGING
            FRDP ("calc sphere: v%d - u: %f, v: %f\n", i>>4, v->ou, v->ov);
#endif
          }
        }
      }
      if (rdp.geom_mode & 0x00400000)
      {
        float tmpvec[3] = {x, y, z};
        calc_point_light (v, tmpvec);
      }
      else
      {
        NormalizeVector (v->vec);
        calc_light (v);
      }
    }
    else
    {
      v->r = ((wxUint8*)gfx.RDRAM)[(addr+i + 12)^3];
      v->g = ((wxUint8*)gfx.RDRAM)[(addr+i + 13)^3];
      v->b = ((wxUint8*)gfx.RDRAM)[(addr+i + 14)^3];
    }
#ifdef EXTREME_LOGGING
    FRDP ("v%d - x: %f, y: %f, z: %f, w: %f, u: %f, v: %f, f: %f, z_w: %f, r=%d, g=%d, b=%d, a=%d\n", i>>4, v->x, v->y, v->z, v->w, v->ou*rdp.tiles[rdp.cur_tile].s_scale, v->ov*rdp.tiles[rdp.cur_tile].t_scale, v->f, v->z_w, v->r, v->g, v->b, v->a);
#endif
  }
  rdp.geom_mode = geom_mode;
}

static void uc2_modifyvtx ()
{
  wxUint8 where = (wxUint8)((rdp.cmd0 >> 16) & 0xFF);
  wxUint16 vtx = (wxUint16)((rdp.cmd0 >> 1) & 0xFFFF);

  FRDP ("uc2:modifyvtx: vtx: %d, where: 0x%02lx, val: %08lx - ", vtx, where, rdp.cmd1);
  uc0_modifyvtx(where, vtx, rdp.cmd1);
}

static void uc2_culldl ()
{
  wxUint16 vStart = (wxUint16)(rdp.cmd0 & 0xFFFF) >> 1;
  wxUint16 vEnd = (wxUint16)(rdp.cmd1 & 0xFFFF) >> 1;
  wxUint32 cond = 0;
  FRDP ("uc2:culldl start: %d, end: %d\n", vStart, vEnd);

  if (vEnd < vStart) return;
  for (wxUint16 i=vStart; i<=vEnd; i++)
  {
  /*
  VERTEX v = &rdp.vtx[i];
  // Check if completely off the screen (quick frustrum clipping for 90 FOV)
  if (v->x >= -v->w)
  cond |= 0x01;
  if (v->x <= v->w)
  cond |= 0x02;
  if (v->y >= -v->w)
  cond |= 0x04;
  if (v->y <= v->w)
  cond |= 0x08;
  if (v->w >= 0.1f)
  cond |= 0x10;

    if (cond == 0x1F)
    return;
    //*/

#ifdef EXTREME_LOGGING
    FRDP (" v[%d] = (%02f, %02f, %02f, 0x%02lx)\n", i, rdp.vtx[i].x, rdp.vtx[i].y, rdp.vtx[i].w, rdp.vtx[i].scr_off);
#endif

    cond |= (~rdp.vtx[i].scr_off) & 0x1F;
    if (cond == 0x1F)
      return;
  }

  LRDP(" - ");  // specify that the enddl is not a real command
  uc0_enddl ();
}

static void uc6_obj_loadtxtr ();

static void uc2_tri1()
{
  if ((rdp.cmd0 & 0x00FFFFFF) == 0x17)
  {
    uc6_obj_loadtxtr ();
    return;
  }
  if (rdp.skip_drawing)
  {
    LRDP("uc2:tri1. skipped\n");
    return;
  }

  FRDP("uc2:tri1 #%d - %d, %d, %d\n", rdp.tri_n,
    ((rdp.cmd0 >> 17) & 0x7F),
    ((rdp.cmd0 >> 9) & 0x7F),
    ((rdp.cmd0 >> 1) & 0x7F));

  VERTEX *v[3] = {
    &rdp.vtx[(rdp.cmd0 >> 17) & 0x7F],
      &rdp.vtx[(rdp.cmd0 >> 9) & 0x7F],
      &rdp.vtx[(rdp.cmd0 >> 1) & 0x7F]
  };

  rsp_tri1(v);
}

static void uc6_obj_ldtx_sprite ();
static void uc6_obj_ldtx_rect ();

static void uc2_quad ()
{
  if ((rdp.cmd0 & 0x00FFFFFF) == 0x2F)
  {
    wxUint32 command = rdp.cmd0>>24;
    if (command == 0x6)
    {
      uc6_obj_ldtx_sprite ();
      return;
    }
    if (command == 0x7)
    {
      uc6_obj_ldtx_rect ();
      return;
    }
  }

  if (rdp.skip_drawing)
  {
    LRDP("uc2_quad. skipped\n");
    return;
  }

  LRDP("uc2:quad");

  FRDP(" #%d, #%d - %d, %d, %d - %d, %d, %d\n", rdp.tri_n, rdp.tri_n+1,
    ((rdp.cmd0 >> 17) & 0x7F),
    ((rdp.cmd0 >> 9) & 0x7F),
    ((rdp.cmd0 >> 1) & 0x7F),
    ((rdp.cmd1 >> 17) & 0x7F),
    ((rdp.cmd1 >> 9) & 0x7F),
    ((rdp.cmd1 >> 1) & 0x7F));

  VERTEX *v[6] = {
    &rdp.vtx[(rdp.cmd0 >> 17) & 0x7F],
    &rdp.vtx[(rdp.cmd0 >> 9) & 0x7F],
    &rdp.vtx[(rdp.cmd0 >> 1) & 0x7F],
    &rdp.vtx[(rdp.cmd1 >> 17) & 0x7F],
    &rdp.vtx[(rdp.cmd1 >> 9) & 0x7F],
    &rdp.vtx[(rdp.cmd1 >> 1) & 0x7F]
  };

  rsp_tri2(v);
}

static void uc6_ldtx_rect_r ();

static void uc2_line3d ()
{
  if ( (rdp.cmd0&0xFF) == 0x2F )
    uc6_ldtx_rect_r ();
  else
  {
    FRDP("uc2:line3d #%d, #%d - %d, %d\n", rdp.tri_n, rdp.tri_n+1,
      (rdp.cmd0 >> 17) & 0x7F,
      (rdp.cmd0 >> 9) & 0x7F);

    VERTEX *v[3] = {
      &rdp.vtx[(rdp.cmd0 >> 17) & 0x7F],
        &rdp.vtx[(rdp.cmd0 >> 9) & 0x7F],
        &rdp.vtx[(rdp.cmd0 >> 9) & 0x7F]
    };
    wxUint16 width = (wxUint16)(rdp.cmd0 + 3)&0xFF;
    wxUint32 cull_mode = (rdp.flags & CULLMASK) >> CULLSHIFT;
    rdp.flags |= CULLMASK;
    rdp.update |= UPDATE_CULL_MODE;
    rsp_tri1(v, width);
    rdp.flags ^= CULLMASK;
    rdp.flags |= cull_mode << CULLSHIFT;
    rdp.update |= UPDATE_CULL_MODE;
  }
}

static void uc2_special3 ()
{
  LRDP("uc2:special3\n");
}

static void uc2_special2 ()
{
  LRDP("uc2:special2\n");
}

static void uc2_dma_io ()
{
  LRDP("uc2:dma_io\n");
}

static void uc2_pop_matrix ()
{
  FRDP ("uc2:pop_matrix %08lx, %08lx\n", rdp.cmd0, rdp.cmd1);

  // Just pop the modelview matrix
  modelview_pop (rdp.cmd1 >> 6);
}

static void uc2_geom_mode ()
{
  // Switch around some things
  wxUint32 clr_mode = (rdp.cmd0 & 0x00DFC9FF) |
    ((rdp.cmd0 & 0x00000600) << 3) |
    ((rdp.cmd0 & 0x00200000) >> 12) | 0xFF000000;
  wxUint32 set_mode = (rdp.cmd1 & 0xFFDFC9FF) |
    ((rdp.cmd1 & 0x00000600) << 3) |
    ((rdp.cmd1 & 0x00200000) >> 12);

  FRDP("uc2:geom_mode c:%08lx, s:%08lx ", clr_mode, set_mode);

  rdp.geom_mode &= clr_mode;
  rdp.geom_mode |= set_mode;

  FRDP ("result:%08lx\n", rdp.geom_mode);

  if (rdp.geom_mode & 0x00000001) // Z-Buffer enable
  {
    if (!(rdp.flags & ZBUF_ENABLED))
    {
      rdp.flags |= ZBUF_ENABLED;
      rdp.update |= UPDATE_ZBUF_ENABLED;
    }
  }
  else
  {
    if ((rdp.flags & ZBUF_ENABLED))
    {
      if (!settings.flame_corona || (rdp.rm != 0x00504341)) //hack for flame's corona
        rdp.flags ^= ZBUF_ENABLED;
      rdp.update |= UPDATE_ZBUF_ENABLED;
    }
  }
  if (rdp.geom_mode & 0x00001000) // Front culling
  {
    if (!(rdp.flags & CULL_FRONT))
    {
      rdp.flags |= CULL_FRONT;
      rdp.update |= UPDATE_CULL_MODE;
    }
  }
  else
  {
    if (rdp.flags & CULL_FRONT)
    {
      rdp.flags ^= CULL_FRONT;
      rdp.update |= UPDATE_CULL_MODE;
    }
  }
  if (rdp.geom_mode & 0x00002000) // Back culling
  {
    if (!(rdp.flags & CULL_BACK))
    {
      rdp.flags |= CULL_BACK;
      rdp.update |= UPDATE_CULL_MODE;
    }
  }
  else
  {
    if (rdp.flags & CULL_BACK)
    {
      rdp.flags ^= CULL_BACK;
      rdp.update |= UPDATE_CULL_MODE;
    }
  }

  //Added by Gonetz
  if (rdp.geom_mode & 0x00010000)      // Fog enable
  {
    if (!(rdp.flags & FOG_ENABLED))
    {
      rdp.flags |= FOG_ENABLED;
      rdp.update |= UPDATE_FOG_ENABLED;
    }
  }
  else
  {
    if (rdp.flags & FOG_ENABLED)
    {
      rdp.flags ^= FOG_ENABLED;
      rdp.update |= UPDATE_FOG_ENABLED;
    }
  }
}

static void uc6_obj_rectangle_r ();

static void uc2_matrix ()
{
  if (!(rdp.cmd0 & 0x00FFFFFF))
  {
    uc6_obj_rectangle_r();
    return;
  }
  LRDP("uc2:matrix\n");

  DECLAREALIGN16VAR(m[4][4]);
  load_matrix(m, segoffset(rdp.cmd1));

  wxUint8 command = (wxUint8)((rdp.cmd0 ^ 1) & 0xFF);
  switch (command)
  {
  case 0: // modelview mul nopush
    LRDP("modelview mul\n");
    modelview_mul (m);
    break;

  case 1: // modelview mul push
    LRDP("modelview mul push\n");
    modelview_mul_push (m);
    break;

  case 2: // modelview load nopush
    LRDP("modelview load\n");
    modelview_load (m);
    break;

  case 3: // modelview load push
    LRDP("modelview load push\n");
    modelview_load_push (m);
    break;

  case 4: // projection mul nopush
  case 5: // projection mul push, can't push projection
    LRDP("projection mul\n");
    projection_mul (m);
    break;

  case 6: // projection load nopush
  case 7: // projection load push, can't push projection
    LRDP("projection load\n");
    projection_load (m);
    break;

  default:
    FRDP_E ("Unknown matrix command, %02lx", command);
    FRDP ("Unknown matrix command, %02lx", command);
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

static void uc2_moveword ()
{
  wxUint8 index = (wxUint8)((rdp.cmd0 >> 16) & 0xFF);
  wxUint16 offset = (wxUint16)(rdp.cmd0 & 0xFFFF);
  wxUint32 data = rdp.cmd1;

  FRDP ("uc2:moveword ");

  switch (index)
  {
    // NOTE: right now it's assuming that it sets the integer part first.  This could
    //  be easily fixed, but only if i had something to test with.

  case 0x00:  // moveword matrix
    {
      // do matrix pre-mult so it's re-updated next time
      if (rdp.update & UPDATE_MULT_MAT)
      {
        rdp.update ^= UPDATE_MULT_MAT;
        MulMatrices(rdp.model, rdp.proj, rdp.combined);
      }

      if (rdp.cmd0 & 0x20)  // fractional part
      {
        int index_x = (rdp.cmd0 & 0x1F) >> 1;
        int index_y = index_x >> 2;
        index_x &= 3;

        float fpart = (rdp.cmd1>>16)/65536.0f;
        rdp.combined[index_y][index_x] = (float)(int)rdp.combined[index_y][index_x];
        rdp.combined[index_y][index_x] += fpart;

        fpart = (rdp.cmd1&0xFFFF)/65536.0f;
        rdp.combined[index_y][index_x+1] = (float)(int)rdp.combined[index_y][index_x+1];
        rdp.combined[index_y][index_x+1] += fpart;
      }
      else
      {
        int index_x = (rdp.cmd0 & 0x1F) >> 1;
        int index_y = index_x >> 2;
        index_x &= 3;

        float fpart = (float)fabs(rdp.combined[index_y][index_x] - (int)rdp.combined[index_y][index_x]);
        rdp.combined[index_y][index_x] = (short)(rdp.cmd1>>16);

        fpart = (float)fabs(rdp.combined[index_y][index_x+1] - (int)rdp.combined[index_y][index_x+1]);
        rdp.combined[index_y][index_x+1] = (short)(rdp.cmd1&0xFFFF);
      }

      LRDP("matrix\n");
    }
    break;

  case 0x02:
    rdp.num_lights = data / 24;
    rdp.update |= UPDATE_LIGHTS;
    FRDP ("numlights: %d\n", rdp.num_lights);
    break;

  case 0x04:
    if (offset == 0x04)
    {
      rdp.clip_ratio = sqrt((float)rdp.cmd1);
      rdp.update |= UPDATE_VIEWPORT;
    }
    FRDP ("mw_clip %08lx, %08lx\n", rdp.cmd0, rdp.cmd1);
    break;

  case 0x06:  // moveword SEGMENT
    {
      FRDP ("SEGMENT %08lx -> seg%d\n", data, offset >> 2);
      if ((data&BMASK)<BMASK)
        rdp.segment[(offset >> 2) & 0xF] = data;
    }
    break;


  case 0x08:
    {
      rdp.fog_multiplier = (short)(rdp.cmd1 >> 16);
      rdp.fog_offset = (short)(rdp.cmd1 & 0x0000FFFF);
      FRDP ("fog: multiplier: %f, offset: %f\n", rdp.fog_multiplier, rdp.fog_offset);

      //offset must be 0 for move_fog, but it can be non zero in Nushi Zuri 64 - Shiokaze ni Notte
      //low-level display list has setothermode commands in this place, so this is obviously not move_fog.
      if (offset == 0x04)
        rdp.tlut_mode = (data == 0xffffffff) ? 0 : 2; 
    }
    break;

  case 0x0a:  // moveword LIGHTCOL
    {
      int n = offset / 24;
      FRDP ("lightcol light:%d, %08lx\n", n, data);

      rdp.light[n].r = (float)((data >> 24) & 0xFF) / 255.0f;
      rdp.light[n].g = (float)((data >> 16) & 0xFF) / 255.0f;
      rdp.light[n].b = (float)((data >> 8) & 0xFF) / 255.0f;
      rdp.light[n].a = 255;
    }
    break;

  case 0x0c:
    RDP_E ("uc2:moveword forcemtx - IGNORED\n");
    LRDP("forcemtx - IGNORED\n");
    break;

  case 0x0e:
    LRDP("perspnorm - IGNORED\n");
    break;

  default:
    FRDP_E("uc2:moveword unknown (index: 0x%08lx, offset 0x%08lx)\n", index, offset);
    FRDP ("unknown (index: 0x%08lx, offset 0x%08lx)\n", index, offset);
  }
}

static void uc6_obj_movemem ();

static void uc2_movemem ()
{
  int idx = rdp.cmd0 & 0xFF;
  wxUint32 addr = segoffset(rdp.cmd1);
  int ofs = (rdp.cmd0 >> 5) & 0x7F8;

  FRDP ("uc2:movemem ofs:%d ", ofs);

  switch (idx)
  {
  case 0:
  case 2:
    uc6_obj_movemem ();
    break;

  case 8:   // VIEWPORT
    {
      wxUint32 a = addr >> 1;
      short scale_x = ((short*)gfx.RDRAM)[(a+0)^1] >> 2;
      short scale_y = ((short*)gfx.RDRAM)[(a+1)^1] >> 2;
      short scale_z = ((short*)gfx.RDRAM)[(a+2)^1];
      short trans_x = ((short*)gfx.RDRAM)[(a+4)^1] >> 2;
      short trans_y = ((short*)gfx.RDRAM)[(a+5)^1] >> 2;
      short trans_z = ((short*)gfx.RDRAM)[(a+6)^1];
      rdp.view_scale[0] = scale_x * rdp.scale_x;
      rdp.view_scale[1] = -scale_y * rdp.scale_y;
      rdp.view_scale[2] = 32.0f * scale_z;
      rdp.view_trans[0] = trans_x * rdp.scale_x;
      rdp.view_trans[1] = trans_y * rdp.scale_y;
      rdp.view_trans[2] = 32.0f * trans_z;

      rdp.update |= UPDATE_VIEWPORT;

      FRDP ("viewport scale(%d, %d, %d), trans(%d, %d, %d), from:%08lx\n", scale_x, scale_y, scale_z,
        trans_x, trans_y, trans_z, a);
    }
    break;

  case 10:  // LIGHT
    {
      int n = ofs / 24;

      if (n < 2)
      {
        char dir_x = ((char*)gfx.RDRAM)[(addr+8)^3];
        rdp.lookat[n][0] = (float)(dir_x) / 127.0f;
        char dir_y = ((char*)gfx.RDRAM)[(addr+9)^3];
        rdp.lookat[n][1] = (float)(dir_y) / 127.0f;
        char dir_z = ((char*)gfx.RDRAM)[(addr+10)^3];
        rdp.lookat[n][2] = (float)(dir_z) / 127.0f;
        rdp.use_lookat = TRUE;
        if (n == 1)
        {
          if (!dir_x && !dir_y)
            rdp.use_lookat = FALSE;
        }
        FRDP("lookat_%d (%f, %f, %f)\n", n, rdp.lookat[n][0], rdp.lookat[n][1], rdp.lookat[n][2]);
        return;
      }
      n -= 2;
      if (n > 7) return;

      // Get the data
      wxUint8 col = gfx.RDRAM[(addr+0)^3];
      rdp.light[n].r = (float)col / 255.0f;
      rdp.light[n].nonblack = col;
      col = gfx.RDRAM[(addr+1)^3];
      rdp.light[n].g = (float)col / 255.0f;
      rdp.light[n].nonblack += col;
      col = gfx.RDRAM[(addr+2)^3];
      rdp.light[n].b = (float)col / 255.0f;
      rdp.light[n].nonblack += col;
      rdp.light[n].a = 1.0f;
      // ** Thanks to Icepir8 for pointing this out **
      // Lighting must be signed byte instead of byte
      rdp.light[n].dir_x = (float)(((char*)gfx.RDRAM)[(addr+8)^3]) / 127.0f;
      rdp.light[n].dir_y = (float)(((char*)gfx.RDRAM)[(addr+9)^3]) / 127.0f;
      rdp.light[n].dir_z = (float)(((char*)gfx.RDRAM)[(addr+10)^3]) / 127.0f;
      wxUint32 a = addr >> 1;
      rdp.light[n].x = (float)(((short*)gfx.RDRAM)[(a+4)^1]);
      rdp.light[n].y = (float)(((short*)gfx.RDRAM)[(a+5)^1]);
      rdp.light[n].z = (float)(((short*)gfx.RDRAM)[(a+6)^1]);
      rdp.light[n].ca = (float)(gfx.RDRAM[(addr+3)^3]) / 16.0f;
      rdp.light[n].la = (float)(gfx.RDRAM[(addr+7)^3]);
      rdp.light[n].qa = (float)(gfx.RDRAM[(addr+14)^3]) / 8.0f;
#ifdef EXTREME_LOGGING
      FRDP ("light: n: %d, pos: x: %f, y: %f, z: %f, ca: %f, la:%f, qa: %f\n",
        n, rdp.light[n].x, rdp.light[n].y, rdp.light[n].z, rdp.light[n].ca, rdp.light[n].la, rdp.light[n].qa);
#endif
      FRDP ("light: n: %d, r: %.3f, g: %.3f, b: %.3f. dir: x: %.3f, y: %.3f, z: %.3f\n",
        n, rdp.light[n].r, rdp.light[n].g, rdp.light[n].b,
        rdp.light[n].dir_x, rdp.light[n].dir_y, rdp.light[n].dir_z);
    }
    break;

  case 14:  // matrix
    {
      // do not update the combined matrix!
      rdp.update &= ~UPDATE_MULT_MAT;
      load_matrix(rdp.combined, segoffset(rdp.cmd1));

#ifdef EXTREME_LOGGING
      FRDP ("{%f,%f,%f,%f}\n", rdp.combined[0][0], rdp.combined[0][1], rdp.combined[0][2], rdp.combined[0][3]);
      FRDP ("{%f,%f,%f,%f}\n", rdp.combined[1][0], rdp.combined[1][1], rdp.combined[1][2], rdp.combined[1][3]);
      FRDP ("{%f,%f,%f,%f}\n", rdp.combined[2][0], rdp.combined[2][1], rdp.combined[2][2], rdp.combined[2][3]);
      FRDP ("{%f,%f,%f,%f}\n", rdp.combined[3][0], rdp.combined[3][1], rdp.combined[3][2], rdp.combined[3][3]);
#endif
    }
    break;

  default:
    FRDP ("uc2:matrix unknown (%d)\n", idx);
    FRDP ("** UNKNOWN %d\n", idx);
  }
}

static void uc2_load_ucode ()
{
  LRDP("uc2:load_ucode\n");
}

static void uc2_rdphalf_2 ()
{
  LRDP("uc2:rdphalf_2\n");
}

static void uc2_dlist_cnt ()
{
  wxUint32 addr = segoffset(rdp.cmd1) & BMASK;
  int count = rdp.cmd0 & 0x000000FF;
  FRDP ("dl_count - addr: %08lx, count: %d\n", addr, count);
  if (addr == 0)
    return;

  if (rdp.pc_i >= 9) {
    RDP_E ("** DL stack overflow **\n");
    LRDP("** DL stack overflow **\n");
    return;
  }
  rdp.pc_i ++;  // go to the next PC in the stack
  rdp.pc[rdp.pc_i] = addr;  // jump to the address
  rdp.dl_count = count + 1;
}
