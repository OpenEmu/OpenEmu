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
extern "C" {
#ifndef NOSSE
#include <xmmintrin.h>
#endif
}

#include <math.h>
#include "3dmath.h"

void calc_light (VERTEX *v)
{
  float light_intensity = 0.0f;
  register float color[3] = {rdp.light[rdp.num_lights].r, rdp.light[rdp.num_lights].g, rdp.light[rdp.num_lights].b};
  for (wxUint32 l=0; l<rdp.num_lights; l++)
  {
    light_intensity = DotProduct (rdp.light_vector[l], v->vec);
    
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

//*
void calc_linear (VERTEX *v)
{
  if (settings.force_calc_sphere)
  {
    calc_sphere(v);
    return;
  }
  DECLAREALIGN16VAR(vec[3]);
  
  TransformVector (v->vec, vec, rdp.model);
  //    TransformVector (v->vec, vec, rdp.combined);
  NormalizeVector (vec);
  float x, y;
  if (!rdp.use_lookat)
  {
    x = vec[0];
    y = vec[1];
  }
  else
  {
    x = DotProduct (rdp.lookat[0], vec);
    y = DotProduct (rdp.lookat[1], vec);
  }
  
  if (x > 1.0f)
    x = 1.0f;
  else if (x < -1.0f)
    x = -1.0f;
  if (y > 1.0f)
    y = 1.0f;
  else if (y < -1.0f)
    y = -1.0f;
  
  if (rdp.cur_cache[0])
  {
    // scale >> 6 is size to map to
    v->ou = (acosf(x)/3.141592654f) * (rdp.tiles[rdp.cur_tile].org_s_scale >> 6);
    v->ov = (acosf(y)/3.141592654f) * (rdp.tiles[rdp.cur_tile].org_t_scale >> 6);
  }
  v->uv_scaled = 1;
#ifdef EXTREME_LOGGING
  FRDP ("calc linear u: %f, v: %f\n", v->ou, v->ov);
#endif
}

void calc_sphere (VERTEX *v)
{
//  LRDP("calc_sphere\n");
  DECLAREALIGN16VAR(vec[3]);
  int s_scale, t_scale;
  if (settings.hacks&hack_Chopper)
  {
    s_scale = min(rdp.tiles[rdp.cur_tile].org_s_scale >> 6, rdp.tiles[rdp.cur_tile].lr_s);
    t_scale = min(rdp.tiles[rdp.cur_tile].org_t_scale >> 6, rdp.tiles[rdp.cur_tile].lr_t);
  }
  else
  {
    s_scale = rdp.tiles[rdp.cur_tile].org_s_scale >> 6;
    t_scale = rdp.tiles[rdp.cur_tile].org_t_scale >> 6;
  }
  TransformVector (v->vec, vec, rdp.model);
  //    TransformVector (v->vec, vec, rdp.combined);
  NormalizeVector (vec);
  float x, y;
  if (!rdp.use_lookat)
  {
    x = vec[0];
    y = vec[1];
  }
  else
  {
    x = DotProduct (rdp.lookat[0], vec);
    y = DotProduct (rdp.lookat[1], vec);
  }
  v->ou = (x * 0.5f + 0.5f) * s_scale;
  v->ov = (y * 0.5f + 0.5f) * t_scale;
  v->uv_scaled = 1;
#ifdef EXTREME_LOGGING
  FRDP ("calc sphere u: %f, v: %f\n", v->ou, v->ov);
#endif
}

float DotProductC(register float *v1, register float *v2)
{
    register float result;
    result = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
    return(result);
}

void NormalizeVectorC(float *v)
{
    register float len;
    len = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (len > 0.0f)
    {
        v[0] /= len;
        v[1] /= len;
        v[2] /= len;
    }
}

void TransformVectorC(float *src, float *dst, float mat[4][4])
{
  dst[0] = mat[0][0]*src[0] + mat[1][0]*src[1] + mat[2][0]*src[2];
  dst[1] = mat[0][1]*src[0] + mat[1][1]*src[1] + mat[2][1]*src[2];
  dst[2] = mat[0][2]*src[0] + mat[1][2]*src[1] + mat[2][2]*src[2];
}

void InverseTransformVectorC (float *src, float *dst, float mat[4][4])
{
  dst[0] = mat[0][0]*src[0] + mat[0][1]*src[1] + mat[0][2]*src[2];
  dst[1] = mat[1][0]*src[0] + mat[1][1]*src[1] + mat[1][2]*src[2];
  dst[2] = mat[2][0]*src[0] + mat[2][1]*src[1] + mat[2][2]*src[2];
}

void MulMatricesC(float m1[4][4],float m2[4][4],float r[4][4])
{
  for (int i=0; i<4; i++)
  {
    for (int j=0; j<4; j++)
    {
      r[i][j] = m1[i][0] * m2[0][j] +
                m1[i][1] * m2[1][j] +
                m1[i][2] * m2[2][j] +
                m1[i][3] * m2[3][j];
    }
  }
}

// 2008.03.29 H.Morii - added SSE 3DNOW! 3x3 1x3 matrix multiplication
//                      and 3DNOW! 4x4 4x4 matrix multiplication
// 2011-01-03 Balrog - removed because is in NASM format and not 64-bit compatible
// This will need fixing.
MULMATRIX MulMatrices = MulMatricesC;
TRANSFORMVECTOR TransformVector = TransformVectorC;
TRANSFORMVECTOR InverseTransformVector = InverseTransformVectorC;
DOTPRODUCT DotProduct = DotProductC;
NORMALIZEVECTOR NormalizeVector = NormalizeVectorC;

void MulMatricesSSE(float m1[4][4],float m2[4][4],float r[4][4])
{
#if defined(__GNUC__) && !defined(NO_ASM) && !defined(NOSSE)
   /* [row][col]*/
  typedef float v4sf __attribute__ ((vector_size (16)));
  v4sf row0 = _mm_loadu_ps(m2[0]);
  v4sf row1 = _mm_loadu_ps(m2[1]);
  v4sf row2 = _mm_loadu_ps(m2[2]);
  v4sf row3 = _mm_loadu_ps(m2[3]);

  for (int i = 0; i < 4; ++i)
  {
    v4sf leftrow = _mm_loadu_ps(m1[i]);

    // Fill tmp with four copies of leftrow[0]
    v4sf tmp = leftrow;
    tmp = _mm_shuffle_ps (tmp, tmp, 0);
    // Calculate the four first summands
    v4sf destrow = tmp * row0;

    // Fill tmp with four copies of leftrow[1]
    tmp = leftrow;
    tmp = _mm_shuffle_ps (tmp, tmp, 1 + (1 << 2) + (1 << 4) + (1 << 6));
    destrow += tmp * row1;

    // Fill tmp with four copies of leftrow[2]
    tmp = leftrow;
    tmp = _mm_shuffle_ps (tmp, tmp, 2 + (2 << 2) + (2 << 4) + (2 << 6));
    destrow += tmp * row2;

    // Fill tmp with four copies of leftrow[3]
    tmp = leftrow;
    tmp = _mm_shuffle_ps (tmp, tmp, 3 + (3 << 2) + (3 << 4) + (3 << 6));
    destrow += tmp * row3;

    __builtin_ia32_storeups(r[i], destrow);
  }
 #elif !defined(NO_ASM) && !defined(NOSSE)
  __asm
  {
    mov     eax, dword ptr [r]  
      mov     ecx, dword ptr [m1]
      mov     edx, dword ptr [m2]

      movaps  xmm0,[edx]
      movaps  xmm1,[edx+16]
      movaps  xmm2,[edx+32]
      movaps  xmm3,[edx+48]

// r[0][0],r[0][1],r[0][2],r[0][3]

      movaps  xmm4,xmmword ptr[ecx]
      movaps  xmm5,xmm4
      movaps  xmm6,xmm4
      movaps  xmm7,xmm4

      shufps  xmm4,xmm4,00000000b
      shufps  xmm5,xmm5,01010101b
      shufps  xmm6,xmm6,10101010b
      shufps  xmm7,xmm7,11111111b

      mulps   xmm4,xmm0
      mulps   xmm5,xmm1
      mulps   xmm6,xmm2
      mulps   xmm7,xmm3

      addps   xmm4,xmm5
      addps   xmm4,xmm6
      addps   xmm4,xmm7

      movaps  xmmword ptr[eax],xmm4

// r[1][0],r[1][1],r[1][2],r[1][3]

      movaps  xmm4,xmmword ptr[ecx+16]
      movaps  xmm5,xmm4
      movaps  xmm6,xmm4
      movaps  xmm7,xmm4

      shufps  xmm4,xmm4,00000000b
      shufps  xmm5,xmm5,01010101b
      shufps  xmm6,xmm6,10101010b
      shufps  xmm7,xmm7,11111111b

      mulps   xmm4,xmm0
      mulps   xmm5,xmm1
      mulps   xmm6,xmm2
      mulps   xmm7,xmm3

      addps   xmm4,xmm5
      addps   xmm4,xmm6
      addps   xmm4,xmm7

      movaps  xmmword ptr[eax+16],xmm4


// r[2][0],r[2][1],r[2][2],r[2][3]

      movaps  xmm4,xmmword ptr[ecx+32]
      movaps  xmm5,xmm4
      movaps  xmm6,xmm4
      movaps  xmm7,xmm4

      shufps  xmm4,xmm4,00000000b
      shufps  xmm5,xmm5,01010101b
      shufps  xmm6,xmm6,10101010b
      shufps  xmm7,xmm7,11111111b

      mulps   xmm4,xmm0
      mulps   xmm5,xmm1
      mulps   xmm6,xmm2
      mulps   xmm7,xmm3

      addps   xmm4,xmm5
      addps   xmm4,xmm6
      addps   xmm4,xmm7

      movaps  xmmword ptr[eax+32],xmm4

// r[3][0],r[3][1],r[3][2],r[3][3]

      movaps  xmm4,xmmword ptr[ecx+48]
      movaps  xmm5,xmm4
      movaps  xmm6,xmm4
      movaps  xmm7,xmm4

      shufps  xmm4,xmm4,00000000b
      shufps  xmm5,xmm5,01010101b
      shufps  xmm6,xmm6,10101010b
      shufps  xmm7,xmm7,11111111b

      mulps   xmm4,xmm0
      mulps   xmm5,xmm1
      mulps   xmm6,xmm2
      mulps   xmm7,xmm3

      addps   xmm4,xmm5
      addps   xmm4,xmm6
      addps   xmm4,xmm7

      movaps  xmmword ptr[eax+48],xmm4
    }
#endif // _WIN32
  }



  void math_init()
  {
#ifndef _DEBUG
    int IsSSE = FALSE;
    int edx, eax;
#if defined(__GNUC__) && !defined(NO_ASM) && !defined(NOSSE)
    GLIDE64_TRY
    {
  #if defined(__x86_64__)
      asm volatile(" cpuid;        "
        : "=a"(eax), "=d"(edx)
        : "0"(1)
        : "rbx", "rcx"
        );
  #else
      asm volatile(" push %%ebx;   "
        " push %%ecx;   "
        " cpuid;        "
        " pop %%ecx;    "
        " pop %%ebx;    "
        : "=a"(eax), "=d"(edx)
        : "0"(1)
        :
      );
  #endif
    }
    GLIDE64_CATCH
      { return; }
    // Check for SSE
    if (edx & (1 << 25))
      IsSSE = TRUE;
#elif !defined(NO_ASM) && !defined(NOSSE)
    DWORD dwEdx;
    __try
    {
      __asm 
      {
        mov  eax,1
          cpuid
          mov dwEdx,edx
        }  
      }
      __except(EXCEPTION_EXECUTE_HANDLER)
      {
        return;
      }

      if (dwEdx & (1<<25)) 
      {
        if (dwEdx & (1<<24))
        {      
          __try
          {
            __asm xorps xmm0, xmm0
              IsSSE = TRUE;
          }
          __except(EXCEPTION_EXECUTE_HANDLER)
          {
            return;
          }
        }
      }
#endif // _WIN32
      if (IsSSE)
      {
        MulMatrices = MulMatricesSSE;
        LOG("3DNOW! detected.\n");
      }

#endif //_DEBUG
    }
