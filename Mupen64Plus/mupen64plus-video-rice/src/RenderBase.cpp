/*
Copyright (C) 2003 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <cmath>
#include <vector>

#include "osal_preproc.h"
#include "float.h"
#include "DeviceBuilder.h"
#include "VertexShaderConstantDef.h"
#include "Render.h"
#include "Timing.h"

extern FiddledVtx * g_pVtxBase;

#define ENABLE_CLIP_TRI
#define X_CLIP_MAX  0x1
#define X_CLIP_MIN  0x2
#define Y_CLIP_MAX  0x4
#define Y_CLIP_MIN  0x8
#define Z_CLIP_MAX  0x10
#define Z_CLIP_MIN  0x20

#ifdef ENABLE_CLIP_TRI

inline void RSP_Vtx_Clipping(int i)
{
    g_clipFlag[i] = 0;
    g_clipFlag2[i] = 0;
    if( g_vecProjected[i].w > 0 )
    {
        /*
        if( gRSP.bRejectVtx )
        {
            if( g_vecProjected[i].x > 1 )   
            {
                g_clipFlag2[i] |= X_CLIP_MAX;
                if( g_vecProjected[i].x > gRSP.real_clip_ratio_posx )   
                    g_clipFlag[i] |= X_CLIP_MAX;
            }

            if( g_vecProjected[i].x < -1 )  
            {
                g_clipFlag2[i] |= X_CLIP_MIN;
                if( g_vecProjected[i].x < gRSP.real_clip_ratio_negx )   
                    g_clipFlag[i] |= X_CLIP_MIN;
            }

            if( g_vecProjected[i].y > 1 )   
            {
                g_clipFlag2[i] |= Y_CLIP_MAX;
                if( g_vecProjected[i].y > gRSP.real_clip_ratio_posy )   
                    g_clipFlag[i] |= Y_CLIP_MAX;
            }

            if( g_vecProjected[i].y < -1 )  
            {
                g_clipFlag2[i] |= Y_CLIP_MIN;
                if( g_vecProjected[i].y < gRSP.real_clip_ratio_negy )   
                    g_clipFlag[i] |= Y_CLIP_MIN;
            }

            //if( g_vecProjected[i].z > 1.0f )  
            //{
            //  g_clipFlag2[i] |= Z_CLIP_MAX;
            //  g_clipFlag[i] |= Z_CLIP_MAX;
            //}

            //if( gRSP.bNearClip && g_vecProjected[i].z < -1.0f )   
            //{
            //  g_clipFlag2[i] |= Z_CLIP_MIN;
            //  g_clipFlag[i] |= Z_CLIP_MIN;
            //}
        }
        else
        */
        {
            if( g_vecProjected[i].x > 1 )   g_clipFlag2[i] |= X_CLIP_MAX;
            if( g_vecProjected[i].x < -1 )  g_clipFlag2[i] |= X_CLIP_MIN;
            if( g_vecProjected[i].y > 1 )   g_clipFlag2[i] |= Y_CLIP_MAX;
            if( g_vecProjected[i].y < -1 )  g_clipFlag2[i] |= Y_CLIP_MIN;
            //if( g_vecProjected[i].z > 1.0f )  g_clipFlag2[i] |= Z_CLIP_MAX;
            //if( gRSP.bNearClip && g_vecProjected[i].z < -1.0f )   g_clipFlag2[i] |= Z_CLIP_MIN;
        }

    }
}

#else
inline void RSP_Vtx_Clipping(int i) {}
#endif

/*
 *  Global variables
 */
ALIGN(16,RSP_Options gRSP)
ALIGN(16,RDP_Options gRDP)

static ALIGN(16,XVECTOR4 g_normal)
//static int norms[3];

ALIGN(16,XVECTOR4 g_vtxNonTransformed[MAX_VERTS])
ALIGN(16,XVECTOR4 g_vecProjected[MAX_VERTS])
ALIGN(16,XVECTOR4 g_vtxTransformed[MAX_VERTS])

float       g_vtxProjected5[1000][5];
float       g_vtxProjected5Clipped[2000][5];

//uint32        g_dwVtxFlags[MAX_VERTS];            // Z_POS Z_NEG etc
VECTOR2     g_fVtxTxtCoords[MAX_VERTS];
uint32      g_dwVtxDifColor[MAX_VERTS];
uint32      g_clipFlag[MAX_VERTS];
uint32      g_clipFlag2[MAX_VERTS];
RenderTexture g_textures[MAX_TEXTURES];
float       g_fFogCoord[MAX_VERTS];

EXTERNAL_VERTEX g_vtxForExternal[MAX_VERTS];

TLITVERTEX          g_vtxBuffer[1000];
TLITVERTEX          g_clippedVtxBuffer[2000];
uint8               g_oglVtxColors[1000][4];
int                 g_clippedVtxCount=0;
TLITVERTEX          g_texRectTVtx[4];
unsigned int        g_vtxIndex[1000];
unsigned int        g_minIndex, g_maxIndex;

float               gRSPfFogMin;
float               gRSPfFogMax;
float               gRSPfFogDivider;

uint32          gRSPnumLights;
Light   gRSPlights[16];

ALIGN(16,Matrix  gRSPworldProjectTransported)
ALIGN(16,Matrix  gRSPworldProject)
ALIGN(16,Matrix  gRSPmodelViewTop)
ALIGN(16,Matrix  gRSPmodelViewTopTranspose)
ALIGN(16,Matrix  dkrMatrixTransposed)

N64Light        gRSPn64lights[16];


void (*ProcessVertexData)(uint32 dwAddr, uint32 dwV0, uint32 dwNum)=NULL;

/*
 *  
 */


/*n.x = (g_normal.x * matWorld.m00) + (g_normal.y * matWorld.m10) + (g_normal.z * matWorld.m20);
n.y = (g_normal.x * matWorld.m01) + (g_normal.y * matWorld.m11) + (g_normal.z * matWorld.m21);
n.z = (g_normal.x * matWorld.m02) + (g_normal.y * matWorld.m12) + (g_normal.z * matWorld.m22);*/

// Multiply (x,y,z,0) by matrix m, then normalize
#if defined(__INTEL_COMPILER) && !defined(NO_ASM)
#define Vec3TransformNormal(vec, m) __asm                   \
{                                                       \
    __asm fld       dword ptr [vec + 0]                         \
    __asm fmul  dword ptr [m + 0]           /* x m00*/      \
    __asm fld       dword ptr [vec + 0]                         \
    __asm fmul  dword ptr [m + 4]   /* x m01  x m00*/           \
    __asm fld       dword ptr [vec + 0]                             \
    __asm fmul  dword ptr [m + 8]   /* x m02  x m01  x m00*/    \
    \
    __asm fld       dword ptr [vec + 4]                             \
    __asm fmul  dword ptr [m + 16]  /* y m10  x m02  x m01  x m00*/ \
    __asm fld       dword ptr [vec + 4]                                 \
    __asm fmul  dword ptr [m + 20]  /* y m11  y m10  x m02  x m01  x m00*/      \
    __asm fld       dword ptr [vec + 4]                                             \
    __asm fmul  dword ptr [m + 24]  /* y m12  y m11  y m10  x m02  x m01  x m00*/   \
    \
    __asm fxch  st(2)               /* y m10  y m11  y m12  x m02  x m01  x m00*/           \
    __asm faddp st(5), st(0)        /* y m11  y m12  x m02  x m01  (x m00 + y m10)*/        \
    __asm faddp st(3), st(0)        /* y m12  x m02  (x m01 + ym11)  (x m00 + y m10)*/  \
    __asm faddp st(1), st(0)        /* (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/    \
    \
    __asm fld       dword ptr [vec + 8]                                                     \
    __asm fmul  dword ptr [m + 32] /* z m20  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/  \
    __asm fld       dword ptr [vec + 8]                                                             \
    __asm fmul  dword ptr [m + 36] /* z m21  z m20  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/               \
    __asm fld       dword ptr [vec + 8]                                                                             \
    __asm fmul  dword ptr [m + 40] /* z m22  z m21  z m20  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/        \
    \
    __asm fxch  st(2)               /* z m20  z m21  z m22  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/       \
    __asm faddp st(5), st(0)        /* z m21  z m22  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10 + z m20)*/  \
    __asm faddp st(3), st(0)        /* z m22  (x m02 + y m12) (x m01 + ym11 + z m21)  (x m00 + y m10 + z m20)*/ \
    __asm faddp st(1), st(0)        /* (x m02 + y m12 + z m 22) (x m01 + ym11 + z m21)  (x m00 + y m10 + z m20)*/   \
    \
    __asm fxch  st(2)               /* (x m00 + y m10 + z m20) (x m01 + ym11 + z m21) (x m02 + y m12 + z m 22) */   \
    \
    __asm fld1                      /* 1 x y z */ \
    __asm fld   st(1)               /* x 1 x y z */ \
    __asm fmul  st(0),st(0)         /* xx 1 x y z */  \
    __asm fld   st(3)               /* y xx 1 x y z */ \
    __asm fmul  st(0),st(0)         /* yy xx 1 x y z */ \
    __asm fld   st(5)               /* z yy xx 1 x y z */ \
    __asm fmul  st(0),st(0)         /* zz yy xx 1 x y z */ \
    \
    __asm fxch  st(2)               /* xx yy zz 1 x y z */ \
    \
    __asm faddp st(1),st(0)         /* (xx+yy) zz 1 x y z */ \
    __asm faddp st(1),st(0)         /* (xx+yy+zz) 1 x y z */ \
    \
    __asm ftst                      /* Compare ST to 0  */              \
    __asm fstsw ax                  /* Store FPU status word in a   */  \
    __asm sahf                      /* Transfer ax to flags register */ \
    __asm jz        l2              /* Skip if length is zero   */      \
    \
    __asm fsqrt                     /* l 1 x y z */ \
    \
    __asm fdivp st(1),st(0)         /* (1/l) x y z */ \
    \
    __asm fmul  st(3),st(0)         /* f x y fz */                                      \
    __asm fmul  st(2),st(0)         /* f x fy fz */                                     \
    __asm fmulp st(1),st(0)         /* fx fy fz */                                      \
    \
    __asm fstp  dword ptr [vec + 0] /* fy fz*/                          \
    __asm fstp  dword ptr [vec + 4] /* fz   */          \
    __asm fstp  dword ptr [vec + 8] /* done */          \
    __asm jmp   l3  \
__asm l2:   \
    __asm mov dword ptr [vec + 0], 0    \
    __asm mov dword ptr [vec + 4], 0    \
    __asm mov dword ptr [vec + 8], 0    \
__asm l3:   \
}       \

#else  // use C code in other cases, this is probably faster anyway
#define Vec3TransformNormal(vec, m) \
   VECTOR3 temp; \
   temp.x = (vec.x * m._11) + (vec.y * m._21) + (vec.z * m._31); \
   temp.y = (vec.x * m._12) + (vec.y * m._22) + (vec.z * m._32); \
   temp.z = (vec.x * m._13) + (vec.y * m._23) + (vec.z * m._33); \
   float norm = sqrt(temp.x*temp.x+temp.y*temp.y+temp.z*temp.z); \
   if (norm == 0.0) { vec.x = 0.0; vec.y = 0.0; vec.z = 0.0;} else \
   { vec.x = temp.x/norm; vec.y = temp.y/norm; vec.z = temp.z/norm; }
#endif


#if !defined(__GNUC__) && !defined(NO_ASM)
__declspec( naked ) void  __fastcall SSEVec3Transform(int i)
{
    __asm
    {
        shl     ecx,4;      // ecx = i

        movaps  xmm1,   DWORD PTR g_vtxNonTransformed [ecx];        // xmm1 as original vector

        movaps  xmm4,   DWORD PTR gRSPworldProjectTransported;          // row1
        movaps  xmm5,   DWORD PTR gRSPworldProjectTransported[0x10];    // row2
        movaps  xmm6,   DWORD PTR gRSPworldProjectTransported[0x20];    // row3
        movaps  xmm7,   DWORD PTR gRSPworldProjectTransported[0x30];    // row4

        mulps   xmm4, xmm1;     // row 1
        mulps   xmm5, xmm1;     // row 2
        mulps   xmm6, xmm1;     // row 3
        mulps   xmm7, xmm1;     // row 4

        movhlps xmm0, xmm4;     // xmm4 high to xmm0 low
        movlhps xmm0, xmm5;     // xmm5 low to xmm0 high

        addps   xmm4, xmm0;     // result of add are in xmm4 low
        addps   xmm5, xmm0;     // result of add are in xmm5 high

        shufps  xmm0, xmm4, 0x44;   // move xmm4 low DWORDs to xmm0 high
        shufps  xmm4, xmm5, 0xe4;   // move xmm5 high DWORS to xmm4
        movhlps xmm5, xmm0;         // xmm4, xmm5 are mirrored

        shufps  xmm4, xmm4, 0x08;   // move xmm4's 3rd uint32 to its 2nd uint32
        shufps  xmm5, xmm5, 0x0d;   // move xmm5's 4th uint32 to its 2nd uint32, 
                                    // and move its 2nd uint32 to its 1st uint32
        
        addps   xmm4, xmm5;     // results are in 1st and 2nd uint32


        movhlps xmm0, xmm6;     // xmm6 high to xmm0 low
        movlhps xmm0, xmm7;     // xmm7 low to xmm0 high

        addps   xmm6, xmm0;     // result of add are in xmm6 low
        addps   xmm7, xmm0;     // result of add are in xmm7 high

        shufps  xmm0, xmm6, 0x44;   // move xmm6 low DWORDs to xmm0 high
        shufps  xmm6, xmm7, 0xe4;   // move xmm7 high DWORS to xmm6
        movhlps xmm7, xmm0;         // xmm6, xmm7 are mirrored

        shufps  xmm6, xmm6, 0x08;   // move xmm6's 3rd uint32 to its 2nd uint32
        shufps  xmm7, xmm7, 0x0d;   // move xmm7's 4th uint32 to its 2nd uint32, 
                                    // and move its 2nd uint32 to its 1st uint32
        
        addps   xmm6, xmm7;     // results are in 1st and 2nd uint32
        
        movlhps xmm4, xmm6;     // final result is in xmm4
        movaps  DWORD PTR g_vtxTransformed [ecx], xmm4;

        movaps  xmm0,xmm4;
        shufps  xmm0,xmm0,0xff;
        divps   xmm4,xmm0;
        rcpps   xmm0,xmm0;
        movhlps xmm0,xmm4;
        shufps  xmm0,xmm0,0xe8;
        movlhps xmm4,xmm0;

        movaps  DWORD PTR g_vecProjected [ecx], xmm4;

        emms;
        ret;
    }
}

// Only used by DKR
__declspec( naked ) void  __fastcall SSEVec3TransformDKR(XVECTOR4 &pOut, const XVECTOR4 &pV)
{
    __asm
    {
        movaps  xmm1,   DWORD PTR [edx];        // xmm1 as original vector

        movaps  xmm4,   DWORD PTR dkrMatrixTransposed;  // row1
        movaps  xmm5,   DWORD PTR dkrMatrixTransposed[0x10];    // row2
        movaps  xmm6,   DWORD PTR dkrMatrixTransposed[0x20];    // row3
        movaps  xmm7,   DWORD PTR dkrMatrixTransposed[0x30];    // row4

        mulps   xmm4, xmm1;     // row 1
        mulps   xmm5, xmm1;     // row 2
        mulps   xmm6, xmm1;     // row 3
        mulps   xmm7, xmm1;     // row 4

        movhlps xmm0, xmm4;     // xmm4 high to xmm0 low
        movlhps xmm0, xmm5;     // xmm5 low to xmm0 high

        addps   xmm4, xmm0;     // result of add are in xmm4 low
        addps   xmm5, xmm0;     // result of add are in xmm5 high

        shufps  xmm0, xmm4, 0x44;   // move xmm4 low DWORDs to xmm0 high
        shufps  xmm4, xmm5, 0xe4;   // move xmm5 high DWORS to xmm4
        movhlps xmm5, xmm0;         // xmm4, xmm5 are mirrored

        shufps  xmm4, xmm4, 0x08;   // move xmm4's 3rd uint32 to its 2nd uint32
        shufps  xmm5, xmm5, 0x0d;   // move xmm5's 4th uint32 to its 2nd uint32, 
        // and move its 2nd uint32 to its 1st uint32

        addps   xmm4, xmm5;     // results are in 1st and 2nd uint32


        movhlps xmm0, xmm6;     // xmm6 high to xmm0 low
        movlhps xmm0, xmm7;     // xmm7 low to xmm0 high

        addps   xmm6, xmm0;     // result of add are in xmm6 low
        addps   xmm7, xmm0;     // result of add are in xmm7 high

        shufps  xmm0, xmm6, 0x44;   // move xmm6 low DWORDs to xmm0 high
        shufps  xmm6, xmm7, 0xe4;   // move xmm7 high DWORS to xmm6
        movhlps xmm7, xmm0;         // xmm6, xmm7 are mirrored

        shufps  xmm6, xmm6, 0x08;   // move xmm6's 3rd uint32 to its 2nd uint32
        shufps  xmm7, xmm7, 0x0d;   // move xmm7's 4th uint32 to its 2nd uint32, 
        // and move its 2nd uint32 to its 1st uint32

        addps   xmm6, xmm7;     // results are in 1st and 2nd uint32

        movlhps xmm4, xmm6;     // final result is in xmm4
        movaps  DWORD PTR [ecx], xmm4;

        emms;
        ret;
    }
}
#elif defined(__GNUC__) && defined(__x86_64__) && !defined(NO_ASM)
void SSEVec3Transform(int i)
{
  asm volatile(" shl               $4,      %0   \n"
               " movslq           %k0,     %q0   \n"
               " movaps      (%1,%q0),  %%xmm1   \n"
               " movaps         0(%2),  %%xmm4   \n"
               " movaps        16(%2),  %%xmm5   \n"
               " movaps        32(%2),  %%xmm6   \n"
               " movaps        48(%2),  %%xmm7   \n"
               " mulps         %%xmm1,  %%xmm4   \n"
               " mulps         %%xmm1,  %%xmm5   \n"
               " mulps         %%xmm1,  %%xmm6   \n"
               " mulps         %%xmm1,  %%xmm7   \n"
               " movhlps       %%xmm4,  %%xmm0   \n"
               " movlhps       %%xmm5,  %%xmm0   \n"
               " addps         %%xmm0,  %%xmm4   \n"
               " addps         %%xmm0,  %%xmm5   \n"
               " shufps $0x44, %%xmm4,  %%xmm0   \n"
               " shufps $0xe4, %%xmm5,  %%xmm4   \n"
               " movhlps       %%xmm0,  %%xmm5   \n"
               " shufps $0x08, %%xmm4,  %%xmm4   \n"
               " shufps $0x0d, %%xmm5,  %%xmm5   \n"
               " addps         %%xmm5,  %%xmm4   \n"
               " movhlps       %%xmm6,  %%xmm0   \n"
               " movlhps       %%xmm7,  %%xmm0   \n"
               " addps         %%xmm0,  %%xmm6   \n"
               " addps         %%xmm0,  %%xmm7   \n"
               " shufps $0x44, %%xmm6,  %%xmm0   \n"
               " shufps $0xe4, %%xmm7,  %%xmm6   \n"
               " movhlps       %%xmm0,  %%xmm7   \n"
               " shufps $0x08, %%xmm6,  %%xmm6   \n"
               " shufps $0x0d, %%xmm7,  %%xmm7   \n"
               " addps         %%xmm7,  %%xmm6   \n"
               " movlhps       %%xmm6,  %%xmm4   \n"
               " movaps        %%xmm4, (%3,%q0)  \n"
               " movaps        %%xmm4,  %%xmm0   \n"
               " shufps $0xff, %%xmm0,  %%xmm0   \n"
               " divps         %%xmm0,  %%xmm4   \n"
               " rcpps         %%xmm0,  %%xmm0   \n"
               " movhlps       %%xmm4,  %%xmm0   \n"
               " shufps $0xe8, %%xmm0,  %%xmm0   \n"
               " movlhps       %%xmm0,  %%xmm4   \n"
               " movaps        %%xmm4, (%4,%q0)  \n"
               : "+r"(i)
               : "r"(g_vtxNonTransformed), "r"(&gRSPworldProjectTransported.m[0][0]), "r"(g_vtxTransformed), "r"(g_vecProjected)
               : "memory", "%xmm0", "%xmm1", "%xmm4", "%xmm5", "%xmm6", "%xmm7"
               );
}
#elif !defined(NO_ASM) // 32-bit GCC assumed
void SSEVec3Transform(int i)
{
  asm volatile(" shl               $4,      %0   \n"
               " movaps       (%1,%0),  %%xmm1   \n"
               " movaps         0(%2),  %%xmm4   \n"
               " movaps        16(%2),  %%xmm5   \n"
               " movaps        32(%2),  %%xmm6   \n"
               " movaps        48(%2),  %%xmm7   \n"
               " mulps         %%xmm1,  %%xmm4   \n"
               " mulps         %%xmm1,  %%xmm5   \n"
               " mulps         %%xmm1,  %%xmm6   \n"
               " mulps         %%xmm1,  %%xmm7   \n"
               " movhlps       %%xmm4,  %%xmm0   \n"
               " movlhps       %%xmm5,  %%xmm0   \n"
               " addps         %%xmm0,  %%xmm4   \n"
               " addps         %%xmm0,  %%xmm5   \n"
               " shufps $0x44, %%xmm4,  %%xmm0   \n"
               " shufps $0xe4, %%xmm5,  %%xmm4   \n"
               " movhlps       %%xmm0,  %%xmm5   \n"
               " shufps $0x08, %%xmm4,  %%xmm4   \n"
               " shufps $0x0d, %%xmm5,  %%xmm5   \n"
               " addps         %%xmm5,  %%xmm4   \n"
               " movhlps       %%xmm6,  %%xmm0   \n"
               " movlhps       %%xmm7,  %%xmm0   \n"
               " addps         %%xmm0,  %%xmm6   \n"
               " addps         %%xmm0,  %%xmm7   \n"
               " shufps $0x44, %%xmm6,  %%xmm0   \n"
               " shufps $0xe4, %%xmm7,  %%xmm6   \n"
               " movhlps       %%xmm0,  %%xmm7   \n"
               " shufps $0x08, %%xmm6,  %%xmm6   \n"
               " shufps $0x0d, %%xmm7,  %%xmm7   \n"
               " addps         %%xmm7,  %%xmm6   \n"
               " movlhps       %%xmm6,  %%xmm4   \n"
               " movaps        %%xmm4,  (%3,%0)  \n"
               " movaps        %%xmm4,  %%xmm0   \n"
               " shufps $0xff, %%xmm0,  %%xmm0   \n"
               " divps         %%xmm0,  %%xmm4   \n"
               " rcpps         %%xmm0,  %%xmm0   \n"
               " movhlps       %%xmm4,  %%xmm0   \n"
               " shufps $0xe8, %%xmm0,  %%xmm0   \n"
               " movlhps       %%xmm0,  %%xmm4   \n"
               " movaps        %%xmm4,  (%4,%0)  \n"
               : "+r"(i)
               : "r"(g_vtxNonTransformed), "r"(&gRSPworldProjectTransported.m[0][0]), "r"(g_vtxTransformed), "r"(g_vecProjected)
               : "memory", "%xmm0", "%xmm1", "%xmm4", "%xmm5", "%xmm6", "%xmm7"
               );
}
#endif
float real255 = 255.0f;
float real128 = 128.0f;

#if !defined(__GNUC__) && !defined(NO_ASM)
__declspec( naked ) void  __fastcall SSEVec3TransformNormal()
{
    __asm
    {
        mov     DWORD PTR [g_normal][12], 0;

        movaps  xmm4,   DWORD PTR gRSPmodelViewTopTranspose;    // row1
        movaps  xmm5,   DWORD PTR gRSPmodelViewTopTranspose[0x10];  // row2
        movaps  xmm1,   DWORD PTR [g_normal];       // xmm1 as the normal vector
        movaps  xmm6,   DWORD PTR gRSPmodelViewTopTranspose[0x20];  // row3

        mulps   xmm4, xmm1;     // row 1
        mulps   xmm5, xmm1;     // row 2
        mulps   xmm6, xmm1;     // row 3

        movhlps xmm0, xmm4;     // xmm4 high to xmm0 low
        movlhps xmm0, xmm5;     // xmm5 low to xmm0 high

        addps   xmm4, xmm0;     // result of add are in xmm4 low
        addps   xmm5, xmm0;     // result of add are in xmm5 high

        shufps  xmm0, xmm4, 0x44;   // move xmm4 low DWORDs to xmm0 high
        shufps  xmm4, xmm5, 0xe4;   // move xmm5 high DWORS to xmm4
        movhlps xmm5, xmm0;         // xmm4, xmm5 are mirrored

        shufps  xmm4, xmm4, 0x08;   // move xmm4's 3rd uint32 to its 2nd uint32
        shufps  xmm5, xmm5, 0x0d;   // move xmm5's 4th uint32 to its 2nd uint32, 

        addps   xmm4, xmm5;     // results are in 1st and 2nd uint32

        movaps  xmm1,xmm4;
        mulps   xmm1,xmm1;  //square
        movlhps xmm7, xmm1;
        shufps  xmm7, xmm7,0x03;
        addss   xmm7, xmm1;

        movhlps xmm0, xmm6;     // xmm6 high to xmm0 low
        addps   xmm6, xmm0;     // result of add are in xmm6 low

        movlhps xmm0, xmm6;
        shufps  xmm0, xmm0, 0x03;
        addss   xmm0, xmm6;     // result of add is at xmm0's 1st uint32

        movlhps xmm4, xmm0;

        mulss   xmm0,xmm0;
        addss   xmm7,xmm0;      // xmm7 1st uint32 is the sum of squares

#ifdef DEBUGGER
        movaps  DWORD PTR [g_normal], xmm4;
        movss  DWORD PTR [g_normal][12], xmm7;
#endif
        xorps   xmm0,xmm0;
        ucomiss xmm0,xmm7;
        jz      l2

        rsqrtss xmm7,xmm7;
        shufps  xmm7,xmm7,0;
#ifdef DEBUGGER
        movss  DWORD PTR [g_normal][12], xmm7;
#endif
        mulps   xmm4,xmm7;

        movaps  DWORD PTR [g_normal], xmm4;     // Normalized
        mov     DWORD PTR [g_normal][12], 0;

        emms;
        ret;
l2:
        movss   DWORD PTR [g_normal], xmm0;
        movss   DWORD PTR [g_normal][12], xmm0;
        emms;
        ret;
    }
}
#elif defined(__GNUC__) && !defined(NO_ASM)  // this code should compile for both 64-bit and 32-bit architectures
void SSEVec3TransformNormal(void)
{
  asm volatile(" movl              $0,  12(%0)    \n"
           " movaps          (%1),  %%xmm4    \n"
           " movaps        16(%1),  %%xmm5    \n"
           " movaps          (%0),  %%xmm1    \n"
           " movaps        32(%1),  %%xmm6    \n"
           " mulps         %%xmm1,  %%xmm4    \n"
           " mulps         %%xmm1,  %%xmm5    \n"
           " mulps         %%xmm1,  %%xmm6    \n"
           " movhlps       %%xmm4,  %%xmm0    \n"
           " movlhps       %%xmm5,  %%xmm0    \n"
           " addps         %%xmm0,  %%xmm4    \n"
           " addps         %%xmm0,  %%xmm5    \n"
           " shufps $0x44, %%xmm4,  %%xmm0    \n"
           " shufps $0xe4, %%xmm5,  %%xmm4    \n"
           " movhlps       %%xmm0,  %%xmm5    \n"
           " shufps $0x08, %%xmm4,  %%xmm4    \n"
           " shufps $0x0d, %%xmm5,  %%xmm5    \n"
           " addps         %%xmm5,  %%xmm4    \n"
           " movaps        %%xmm4,  %%xmm1    \n"
           " mulps         %%xmm1,  %%xmm1    \n"
           " movlhps       %%xmm1,  %%xmm7    \n"
           " shufps $0x03, %%xmm7,  %%xmm7    \n"
           " addss         %%xmm1,  %%xmm7    \n"
           " movhlps       %%xmm6,  %%xmm0    \n"
           " addps         %%xmm0,  %%xmm6    \n"
           " movlhps       %%xmm6,  %%xmm0    \n"
           " shufps $0x03, %%xmm0,  %%xmm0    \n"
           " addss         %%xmm6,  %%xmm0    \n"
           " movlhps       %%xmm0,  %%xmm4    \n"
           " mulss         %%xmm0,  %%xmm0    \n"
           " addss         %%xmm0,  %%xmm7    \n"
#ifdef DEBUGGER
           " movaps        %%xmm4,    (%0)    \n"
           " movss         %%xmm7,  12(%0)    \n"
#endif
           " xorps         %%xmm0,  %%xmm0    \n"
           " ucomiss       %%xmm7,  %%xmm0    \n"
           " jz                0f             \n"
           " rsqrtss       %%xmm7,  %%xmm7    \n"
           " shufps $0x00, %%xmm7,  %%xmm7    \n"
#ifdef DEBUGGER
           " movss         %%xmm7,  12(%0)    \n"
#endif
               " mulps         %%xmm7,  %%xmm4    \n"
               " movaps        %%xmm4,    (%0)    \n"
               " movl              $0,  12(%0)    \n"
               " jmp               1f             \n"
               "0:                                \n"
               " movss         %%xmm0,    (%0)    \n"
               " movss         %%xmm0,  12(%0)    \n"
               "1:                                \n"
               :
               : "r"(&g_normal.x), "r"(&gRSPmodelViewTopTranspose.m[0][0])
               : "memory", "cc", "%xmm0", "%xmm1", "%xmm4", "%xmm5", "%xmm6", "%xmm7"
               );
}
#endif

void NormalizeNormalVec()
{
    float w = 1/sqrtf(g_normal.x*g_normal.x + g_normal.y*g_normal.y + g_normal.z*g_normal.z);
    g_normal.x *= w;
    g_normal.y *= w;
    g_normal.z *= w;
}


void InitRenderBase()
{
#if !defined(NO_ASM)
    if( status.isSSEEnabled && !g_curRomInfo.bPrimaryDepthHack && options.enableHackForGames != HACK_FOR_NASCAR)
    {
        ProcessVertexData = ProcessVertexDataSSE;
    }
    else
#endif
    {
        ProcessVertexData = ProcessVertexDataNoSSE;
    }

    gRSPfFogMin = gRSPfFogMax = 0.0f;
    windowSetting.fMultX = windowSetting.fMultY = 2.0f;
    windowSetting.vpLeftW = windowSetting.vpTopW = 0;
    windowSetting.vpRightW = windowSetting.vpWidthW = 640;
    windowSetting.vpBottomW = windowSetting.vpHeightW = 480;
    gRSP.maxZ = 0;
    gRSP.nVPLeftN = gRSP.nVPTopN = 0;
    gRSP.nVPRightN = 640;
    gRSP.nVPBottomN = 640;
    gRSP.nVPWidthN = 640;
    gRSP.nVPHeightN = 640;
    gRDP.scissor.left=gRDP.scissor.top=0;
    gRDP.scissor.right=gRDP.scissor.bottom=640;
    
    gRSP.bLightingEnable = gRSP.bTextureGen = false;
    gRSP.curTile=gRSPnumLights=gRSP.ambientLightColor=gRSP.ambientLightIndex= 0;
    gRSP.fAmbientLightR=gRSP.fAmbientLightG=gRSP.fAmbientLightB=0;
    gRSP.projectionMtxTop = gRSP.modelViewMtxTop = 0;
    gRDP.fogColor = gRDP.primitiveColor = gRDP.envColor = gRDP.primitiveDepth = gRDP.primLODMin = gRDP.primLODFrac = gRDP.LODFrac = 0;
    gRDP.fPrimitiveDepth = 0;
    gRSP.numVertices = 0;
    gRSP.maxVertexID = 0;
    gRSP.bCullFront=false;
    gRSP.bCullBack=true;
    gRSP.bFogEnabled=gRDP.bFogEnableInBlender=false;
    gRSP.bZBufferEnabled=true;
    gRSP.shadeMode=SHADE_SMOOTH;
    gRDP.keyR=gRDP.keyG=gRDP.keyB=gRDP.keyA=gRDP.keyRGB=gRDP.keyRGBA = 0;
    gRDP.fKeyA = 0;
    gRSP.DKRCMatrixIndex = gRSP.dwDKRVtxAddr = gRSP.dwDKRMatrixAddr = 0;
    gRSP.DKRBillBoard = false;

    gRSP.fTexScaleX = 1/32.0f;
    gRSP.fTexScaleY = 1/32.0f;
    gRSP.bTextureEnabled = FALSE;

    gRSP.clip_ratio_left = 0;
    gRSP.clip_ratio_top = 0;
    gRSP.clip_ratio_right = 640;
    gRSP.clip_ratio_bottom = 480;
    gRSP.clip_ratio_negx = 1;
    gRSP.clip_ratio_negy = 1;
    gRSP.clip_ratio_posx = 1;
    gRSP.clip_ratio_posy = 1;
    gRSP.real_clip_scissor_left = 0;
    gRSP.real_clip_scissor_top = 0;
    gRSP.real_clip_scissor_right = 640;
    gRSP.real_clip_scissor_bottom = 480;
    windowSetting.clipping.left = 0;
    windowSetting.clipping.top = 0;
    windowSetting.clipping.right = 640;
    windowSetting.clipping.bottom = 480;
    windowSetting.clipping.width = 640;
    windowSetting.clipping.height = 480;
    windowSetting.clipping.needToClip = false;
    gRSP.real_clip_ratio_negx = 1;
    gRSP.real_clip_ratio_negy = 1;
    gRSP.real_clip_ratio_posx = 1;
    gRSP.real_clip_ratio_posy = 1;

    gRSP.DKRCMatrixIndex=0;
    gRSP.DKRVtxCount=0;
    gRSP.DKRBillBoard = false;
    gRSP.dwDKRVtxAddr=0;
    gRSP.dwDKRMatrixAddr=0;


    gRDP.geometryMode   = 0;
    gRDP.otherModeL     = 0;
    gRDP.otherModeH     = 0;
    gRDP.fillColor      = 0xFFFFFFFF;
    gRDP.originalFillColor  =0;

    gRSP.ucode      = 1;
    gRSP.vertexMult = 10;
    gRSP.bNearClip  = false;
    gRSP.bRejectVtx = false;

    gRDP.texturesAreReloaded = false;
    gRDP.textureIsChanged = false;
    gRDP.colorsAreReloaded = false;

    memset(&gRDP.otherMode,0,sizeof(RDP_OtherMode));
    memset(&gRDP.tiles,0,sizeof(Tile)*8);


        int i;
    for( i=0; i<MAX_VERTS; i++ )
        g_clipFlag[i] = 0;
    for( i=0; i<MAX_VERTS; i++ )
        g_vtxNonTransformed[i].w = 1;

    memset(gRSPn64lights, 0, sizeof(N64Light)*16);
}

void SetFogMinMax(float fMin, float fMax, float fMul, float fOffset)
{
    if( fMin > fMax )
    {
        float temp = fMin;
        fMin = fMax;
        fMax = temp;
    }

    {
        gRSPfFogMin = max(0,fMin/500-1);
        gRSPfFogMax = fMax/500-1;
    }

    gRSPfFogDivider = 255/(gRSPfFogMax-gRSPfFogMin);
    CRender::g_pRender->SetFogMinMax(fMin, fMax);
}

void InitVertexColors()
{
}

void InitVertexTextureConstants()
{
    float scaleX;
    float scaleY;

    RenderTexture &tex0 = g_textures[gRSP.curTile];
    //CTexture *surf = tex0.m_pCTexture;
    Tile &tile0 = gRDP.tiles[gRSP.curTile];

    scaleX = gRSP.fTexScaleX;
    scaleY = gRSP.fTexScaleY;

    gRSP.tex0scaleX = scaleX * tile0.fShiftScaleS/tex0.m_fTexWidth;
    gRSP.tex0scaleY = scaleY * tile0.fShiftScaleT/tex0.m_fTexHeight;

    gRSP.tex0OffsetX = tile0.fhilite_sl/tex0.m_fTexWidth;
    gRSP.tex0OffsetY = tile0.fhilite_tl/tex0.m_fTexHeight;

    if( CRender::g_pRender->IsTexel1Enable() )
    {
        RenderTexture &tex1 = g_textures[(gRSP.curTile+1)&7];
        //CTexture *surf = tex1.m_pCTexture;
        Tile &tile1 = gRDP.tiles[(gRSP.curTile+1)&7];

        gRSP.tex1scaleX = scaleX * tile1.fShiftScaleS/tex1.m_fTexWidth;
        gRSP.tex1scaleY = scaleY * tile1.fShiftScaleT/tex1.m_fTexHeight;

        gRSP.tex1OffsetX = tile1.fhilite_sl/tex1.m_fTexWidth;
        gRSP.tex1OffsetY = tile1.fhilite_tl/tex1.m_fTexHeight;
    }

    gRSP.texGenXRatio = tile0.fShiftScaleS;
    gRSP.texGenYRatio = gRSP.fTexScaleX/gRSP.fTexScaleY*tex0.m_fTexWidth/tex0.m_fTexHeight*tile0.fShiftScaleT;
}

void TexGen(float &s, float &t)
{
    if (gRDP.geometryMode & G_TEXTURE_GEN_LINEAR)
    {   
        s = acosf(g_normal.x) / 3.14159f;
        t = acosf(g_normal.y) / 3.14159f;
    }
    else
    {
        s = 0.5f * ( 1.0f + g_normal.x);
        t = 0.5f * ( 1.0f - g_normal.y);
    }
}

void ComputeLOD(bool openGL)
{
    TLITVERTEX &v0 = g_vtxBuffer[0];
    TLITVERTEX &v1 = g_vtxBuffer[1];
    RenderTexture &tex0 = g_textures[gRSP.curTile];

    float d,dt;
    if( openGL )
    {
        float x = g_vtxProjected5[0][0] / g_vtxProjected5[0][4] - g_vtxProjected5[1][0] / g_vtxProjected5[1][4];
        float y = g_vtxProjected5[0][1] / g_vtxProjected5[0][4] - g_vtxProjected5[1][1] / g_vtxProjected5[1][4];

        x = windowSetting.vpWidthW*x/windowSetting.fMultX/2;
        y = windowSetting.vpHeightW*y/windowSetting.fMultY/2;
        d = sqrtf(x*x+y*y);
    }
    else
    {
        float x = (v0.x - v1.x)/ windowSetting.fMultX;
        float y = (v0.y - v1.y)/ windowSetting.fMultY;
        d = sqrtf(x*x+y*y);
    }

    float s0 = v0.tcord[0].u * tex0.m_fTexWidth;
    float t0 = v0.tcord[0].v * tex0.m_fTexHeight;
    float s1 = v1.tcord[0].u * tex0.m_fTexWidth;
    float t1 = v1.tcord[0].v * tex0.m_fTexHeight;

    dt = sqrtf((s0-s1)*(s0-s1)+(t0-t1)*(t0-t1));

    float lod = dt/d;
    float frac = log10f(lod)/log10f(2.0f);
    //DEBUGGER_IF_DUMP(pauseAtNext,{DebuggerAppendMsg("LOD frac = %f", frac);});
    frac = (lod / powf(2.0f,floorf(frac)));
    frac = frac - floorf(frac);
    //DEBUGGER_IF_DUMP(pauseAtNext,{DebuggerAppendMsg("LOD = %f, frac = %f", lod, frac);});
    gRDP.LODFrac = (uint32)(frac*255);
    CRender::g_pRender->SetCombinerAndBlender();
}

bool bHalfTxtScale=false;
extern uint32 lastSetTile;

void InitVertex(uint32 dwV, uint32 vtxIndex, bool bTexture, bool openGL)
{
    VTX_DUMP(TRACE2("Init vertex (%d) to vtx buf[%d]:", dwV, vtxIndex));

    TLITVERTEX &v = g_vtxBuffer[vtxIndex];
    VTX_DUMP(TRACE4("  Trans: x=%f, y=%f, z=%f, w=%f",  g_vtxTransformed[dwV].x,g_vtxTransformed[dwV].y,g_vtxTransformed[dwV].z,g_vtxTransformed[dwV].w));
    if( openGL )
    {
        g_vtxProjected5[vtxIndex][0] = g_vtxTransformed[dwV].x;
        g_vtxProjected5[vtxIndex][1] = g_vtxTransformed[dwV].y;
        g_vtxProjected5[vtxIndex][2] = g_vtxTransformed[dwV].z;
        g_vtxProjected5[vtxIndex][3] = g_vtxTransformed[dwV].w;
        g_vtxProjected5[vtxIndex][4] = g_vecProjected[dwV].z;

        if( g_vtxTransformed[dwV].w < 0 )   g_vtxProjected5[vtxIndex][4] = 0;
        g_vtxIndex[vtxIndex] = vtxIndex;
    }

    if( !openGL || options.bOGLVertexClipper == TRUE )
    {
        v.x = g_vecProjected[dwV].x*gRSP.vtxXMul+gRSP.vtxXAdd;
        v.y = g_vecProjected[dwV].y*gRSP.vtxYMul+gRSP.vtxYAdd;
        v.z = (g_vecProjected[dwV].z + 1.0f) * 0.5f;    // DirectX minZ=0, maxZ=1
        //v.z = g_vecProjected[dwV].z;  // DirectX minZ=0, maxZ=1
        v.rhw = g_vecProjected[dwV].w;
        VTX_DUMP(TRACE4("  Proj : x=%f, y=%f, z=%f, rhw=%f",  v.x,v.y,v.z,v.rhw));

        if( gRSP.bProcessSpecularColor )
        {
            v.dcSpecular = CRender::g_pRender->PostProcessSpecularColor();
            if( gRSP.bFogEnabled )
            {
                v.dcSpecular &= 0x00FFFFFF;
                uint32  fogFct = 0xFF-(uint8)((g_fFogCoord[dwV]-gRSPfFogMin)*gRSPfFogDivider);
                v.dcSpecular |= (fogFct<<24);
            }
        }
        else if( gRSP.bFogEnabled )
        {
            uint32  fogFct = 0xFF-(uint8)((g_fFogCoord[dwV]-gRSPfFogMin)*gRSPfFogDivider);
            v.dcSpecular = (fogFct<<24);
        }
    }
    VTX_DUMP(TRACE2("  (U,V): %f, %f",  g_fVtxTxtCoords[dwV].x,g_fVtxTxtCoords[dwV].y));

    v.dcDiffuse = g_dwVtxDifColor[dwV];
    if( gRDP.otherMode.key_en )
    {
        v.dcDiffuse &= 0x00FFFFFF;
        v.dcDiffuse |= (gRDP.keyA<<24);
    }
    else if( gRDP.otherMode.aa_en && gRDP.otherMode.clr_on_cvg==0 )
    {
        v.dcDiffuse |= 0xFF000000;
    }

    if( gRSP.bProcessDiffuseColor )
    {
        v.dcDiffuse = CRender::g_pRender->PostProcessDiffuseColor(v.dcDiffuse);
    }
    if( options.bWinFrameMode )
    {
        v.dcDiffuse = g_dwVtxDifColor[dwV];
    }

    if( openGL )
    {
        g_oglVtxColors[vtxIndex][0] = v.r;
        g_oglVtxColors[vtxIndex][1] = v.g;
        g_oglVtxColors[vtxIndex][2] = v.b;
        g_oglVtxColors[vtxIndex][3] = v.a;
    }

    if( bTexture )
    {
        // If the vert is already lit, then there is no normal (and hence we can't generate tex coord)
        // Only scale if not generated automatically
        if (gRSP.bTextureGen && gRSP.bLightingEnable)
        {
            // Correction for texGen result
            float u0,u1,v0,v1;
            RenderTexture &tex0 = g_textures[gRSP.curTile];
            u0 = g_fVtxTxtCoords[dwV].x * 32 * 1024 * gRSP.fTexScaleX / tex0.m_fTexWidth;
            v0 = g_fVtxTxtCoords[dwV].y * 32 * 1024 * gRSP.fTexScaleY / tex0.m_fTexHeight;
            u0 *= (gRDP.tiles[gRSP.curTile].fShiftScaleS);
            v0 *= (gRDP.tiles[gRSP.curTile].fShiftScaleT);

            if( CRender::g_pRender->IsTexel1Enable() )
            {
                RenderTexture &tex1 = g_textures[(gRSP.curTile+1)&7];
                u1 = g_fVtxTxtCoords[dwV].x * 32 * 1024 * gRSP.fTexScaleX / tex1.m_fTexWidth;
                v1 = g_fVtxTxtCoords[dwV].y * 32 * 1024 * gRSP.fTexScaleY / tex1.m_fTexHeight;
                u1 *= gRDP.tiles[(gRSP.curTile+1)&7].fShiftScaleS;
                v1 *= gRDP.tiles[(gRSP.curTile+1)&7].fShiftScaleT;
                CRender::g_pRender->SetVertexTextureUVCoord(v, u0, v0, u1, v1);
            }
            else
            {
                CRender::g_pRender->SetVertexTextureUVCoord(v, u0, v0);
            }
        }
        else
        {
            float tex0u = g_fVtxTxtCoords[dwV].x *gRSP.tex0scaleX - gRSP.tex0OffsetX ;
            float tex0v = g_fVtxTxtCoords[dwV].y *gRSP.tex0scaleY - gRSP.tex0OffsetY ;

            if( CRender::g_pRender->IsTexel1Enable() )
            {
                float tex1u = g_fVtxTxtCoords[dwV].x *gRSP.tex1scaleX - gRSP.tex1OffsetX ;
                float tex1v = g_fVtxTxtCoords[dwV].y *gRSP.tex1scaleY - gRSP.tex1OffsetY ;

                CRender::g_pRender->SetVertexTextureUVCoord(v, tex0u, tex0v, tex1u, tex1v);
                VTX_DUMP(TRACE2("  (tex0): %f, %f",  tex0u,tex0v));
                VTX_DUMP(TRACE2("  (tex1): %f, %f",  tex1u,tex1v));
            }
            else
            {
                CRender::g_pRender->SetVertexTextureUVCoord(v, tex0u, tex0v);
                VTX_DUMP(TRACE2("  (tex0): %f, %f",  tex0u,tex0v));
            }
        }

        // Check for txt scale hack
        if( !bHalfTxtScale && g_curRomInfo.bTextureScaleHack &&
            (gRDP.tiles[lastSetTile].dwSize == TXT_SIZE_32b || gRDP.tiles[lastSetTile].dwSize == TXT_SIZE_4b ) )
        {
            int width = ((gRDP.tiles[lastSetTile].sh-gRDP.tiles[lastSetTile].sl+1)<<1);
            int height = ((gRDP.tiles[lastSetTile].th-gRDP.tiles[lastSetTile].tl+1)<<1);
            if( g_fVtxTxtCoords[dwV].x*gRSP.fTexScaleX == width || g_fVtxTxtCoords[dwV].y*gRSP.fTexScaleY == height )
            {
                bHalfTxtScale=true;
            }
        }
    }

    if( g_curRomInfo.bEnableTxtLOD && vtxIndex == 1 && gRDP.otherMode.text_lod )
    {
        if( CRender::g_pRender->IsTexel1Enable() && CRender::g_pRender->m_pColorCombiner->m_pDecodedMux->isUsed(MUX_LODFRAC) )
        {
            ComputeLOD(openGL);
        }
        else
        {
            gRDP.LODFrac = 0;
        }
    }

    VTX_DUMP(TRACE2("  DIF(%08X), SPE(%08X)",   v.dcDiffuse, v.dcSpecular));
    VTX_DUMP(TRACE0(""));
}

uint32 LightVert(XVECTOR4 & norm, int vidx)
{
    float fCosT;

    // Do ambient
    register float r = gRSP.fAmbientLightR;
    register float g = gRSP.fAmbientLightG;
    register float b = gRSP.fAmbientLightB;

    if( options.enableHackForGames != HACK_FOR_ZELDA_MM )
    {
        for (register unsigned int l=0; l < gRSPnumLights; l++)
        {
            fCosT = norm.x*gRSPlights[l].x + norm.y*gRSPlights[l].y + norm.z*gRSPlights[l].z; 

            if (fCosT > 0 )
            {
                r += gRSPlights[l].fr * fCosT;
                g += gRSPlights[l].fg * fCosT;
                b += gRSPlights[l].fb * fCosT;
            }
        }
    }
    else
    {
        XVECTOR4 v;
        bool transformed = false;

        for (register unsigned int l=0; l < gRSPnumLights; l++)
        {
            if( gRSPlights[l].range == 0 )
            {
                // Regular directional light
                fCosT = norm.x*gRSPlights[l].x + norm.y*gRSPlights[l].y + norm.z*gRSPlights[l].z; 

                if (fCosT > 0 )
                {
                    r += gRSPlights[l].fr * fCosT;
                    g += gRSPlights[l].fg * fCosT;
                    b += gRSPlights[l].fb * fCosT;
                }
            }
            else //if( (gRSPlights[l].col&0x00FFFFFF) != 0x00FFFFFF )
            {
                // Point light
                if( !transformed )
                {
                    Vec3Transform(&v, (XVECTOR3*)&g_vtxNonTransformed[vidx], &gRSPmodelViewTop);    // Convert to w=1
                    transformed = true;
                }

                XVECTOR3 dir(gRSPlights[l].x - v.x, gRSPlights[l].y - v.y, gRSPlights[l].z - v.z);
                //XVECTOR3 dir(v.x-gRSPlights[l].x, v.y-gRSPlights[l].y, v.z-gRSPlights[l].z);
                float d2 = sqrtf(dir.x*dir.x+dir.y*dir.y+dir.z*dir.z);
                dir.x /= d2;
                dir.y /= d2;
                dir.z /= d2;

                fCosT = norm.x*dir.x + norm.y*dir.y + norm.z*dir.z; 

                if (fCosT > 0 )
                {
                    //float f = d2/gRSPlights[l].range*50;
                    float f = d2/15000*50;
                    f = 1 - min(f,1);
                    fCosT *= f*f;

                    r += gRSPlights[l].fr * fCosT;
                    g += gRSPlights[l].fg * fCosT;
                    b += gRSPlights[l].fb * fCosT;
                }

            }
        }
    }

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    return ((0xff000000)|(((uint32)r)<<16)|(((uint32)g)<<8)|((uint32)b));
}

uint32 LightVertNew(XVECTOR4 & norm)
{
    float fCosT;

    // Do ambient
    register float r = gRSP.fAmbientLightR;
    register float g = gRSP.fAmbientLightG;
    register float b = gRSP.fAmbientLightB;


    for (register unsigned int l=0; l < gRSPnumLights; l++)
    {
        fCosT = norm.x*gRSPlights[l].tx + norm.y*gRSPlights[l].ty + norm.z*gRSPlights[l].tz; 

        if (fCosT > 0 )
        {
            r += gRSPlights[l].fr * fCosT;
            g += gRSPlights[l].fg * fCosT;
            b += gRSPlights[l].fb * fCosT;
        }
    }

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    return ((0xff000000)|(((uint32)r)<<16)|(((uint32)g)<<8)|((uint32)b));
}


float zero = 0.0f;
float onef = 1.0f;
float fcosT;

#if !defined(__GNUC__) && !defined(NO_ASM)
__declspec( naked ) uint32  __fastcall SSELightVert()
{
    __asm
    {
        movaps      xmm3, DWORD PTR gRSP;   // loading Ambient colors, xmm3 is the result color
        movaps      xmm4, DWORD PTR [g_normal]; // xmm4 is the normal

        mov         ecx, 0;
loopback:
        cmp         ecx, DWORD PTR gRSPnumLights;
        jae         breakout;
        mov         eax,ecx;
        imul        eax,0x44;
        movups      xmm5, DWORD PTR gRSPlights[eax];        // Light Dir
        movups      xmm1, DWORD PTR gRSPlights[0x14][eax];  // Light color
        mulps       xmm5, xmm4;                 // Lightdir * normals

        movhlps     xmm0,xmm5;
        addps       xmm0,xmm5;
        shufps      xmm5,xmm0,0x01;
        addps       xmm0,xmm5;

        comiss      xmm0,zero;
        jc          endloop

        shufps      xmm0,xmm0,0;                    // fcosT
        mulps       xmm1,xmm0; 
        addps       xmm3,xmm1; 
endloop:
        inc         ecx;
        jmp         loopback;
breakout:

        movss       xmm0,DWORD PTR real255;
        shufps      xmm0,xmm0,0;
        minps       xmm0,xmm3;

        // Without using a memory
        cvtss2si    eax,xmm0;   // move the 1st uint32 to eax
        shl         eax,10h;
        or          eax,0FF000000h;
        shufps      xmm0,xmm0,0E5h; // move the 2nd uint32 to the 1st uint32
        cvtss2si    ecx,xmm0;   // move the 1st uint32 to ecx
        shl         ecx,8;
        or          eax,ecx;
        shufps      xmm0,xmm0,0E6h; // Move the 3rd uint32 to the 1st uint32
        cvtss2si    ecx,xmm0;
        or          eax,ecx;

        ret;
    }
}
#elif defined(__GNUC__) && defined(__x86_64__) && !defined(NO_ASM)
uint32 SSELightVert(void)
{
  uint32 rval;
  float f255 = 255.0, fZero = 0.0;
  
  asm volatile(" movaps            %1,  %%xmm3    \n" // xmm3 == gRSP.fAmbientLight{RGBA}
           " movaps            %2,  %%xmm4    \n"     // xmm4 == g_normal.{xyz}
           " xor            %%rcx,   %%rcx    \n"
           "0:                                \n"
           " cmpl              %3,   %%ecx    \n"
           " jae               2f             \n"
           " mov            %%rcx,   %%rax    \n"
           " imul    $0x44, %%rax,   %%rax    \n"
           " movups   (%4,%%rax,),  %%xmm5    \n"  // xmm5 == gRSPlights[l].{xyzr}
           " movups 20(%4,%%rax,),  %%xmm1    \n"  // xmm1 == gRSPlights[l].{frfgfbfa}
           " mulps         %%xmm4,  %%xmm5    \n"
           " movhlps       %%xmm5,  %%xmm0    \n"
           " addps         %%xmm5,  %%xmm0    \n"
           " shufps $0x01, %%xmm0,  %%xmm5    \n"
           " addps         %%xmm5,  %%xmm0    \n"
           " comiss            %6,  %%xmm0    \n"
           " jc                1f             \n"
           " shufps $0x00, %%xmm0,  %%xmm0    \n"
           " mulps         %%xmm0,  %%xmm1    \n"
           " addps         %%xmm1,  %%xmm3    \n"
           "1:                                \n"
           " inc            %%rcx             \n"
           " jmp               0b             \n"
           "2:                                \n"
           " movss             %5,  %%xmm0    \n"
           " shufps $0x00, %%xmm0,  %%xmm0    \n"
           " minps         %%xmm3,  %%xmm0    \n"
           " cvtss2si      %%xmm0,   %%eax    \n"
           " shll           $0x10,   %%eax    \n"
           " orl      $0xff000000,   %%eax    \n"
           " shufps $0xe5, %%xmm0,  %%xmm0    \n"
           " cvtss2si      %%xmm0,   %%ecx    \n"
           " shll              $8,   %%ecx    \n"
           " orl            %%ecx,   %%eax    \n"
           " shufps $0xe6, %%xmm0,  %%xmm0    \n"
           " cvtss2si      %%xmm0,   %%ecx    \n"
           " orl            %%ecx,   %%eax    \n"
           : "=&a"(rval)
           : "m"(gRSP), "m"(g_normal), "m"(gRSPnumLights), "r"(gRSPlights), "m"(f255), "m"(fZero)
           : "%rcx", "memory", "cc", "%xmm0", "%xmm1", "%xmm3", "%xmm4", "%xmm5"
           );
  return rval;
}
#elif !defined(NO_ASM) // 32-bit GCC assumed
uint32 SSELightVert(void)
{
  uint32 rval;
  float f255 = 255.0, fZero = 0.0;

  asm volatile(" movaps            %1,  %%xmm3    \n"
               " movaps            %2,  %%xmm4    \n"
               " xor            %%ecx,   %%ecx    \n"
               "0:                                \n"
               " cmpl              %3,   %%ecx    \n"
               " jae               2f             \n"
               " mov            %%ecx,   %%eax    \n"
               " imul    $0x44, %%eax,   %%eax    \n"
               " movups   (%4,%%eax,),  %%xmm5    \n"
               " movups 20(%4,%%eax,),  %%xmm1    \n"
               " mulps         %%xmm4,  %%xmm5    \n"
               " movhlps       %%xmm5,  %%xmm0    \n"
               " addps         %%xmm5,  %%xmm0    \n"
               " shufps $0x01, %%xmm0,  %%xmm5    \n"
               " addps         %%xmm5,  %%xmm0    \n"
               " comiss            %6,  %%xmm0    \n"
               " jc                1f             \n"
               " shufps $0x00, %%xmm0,  %%xmm0    \n"
               " mulps         %%xmm0,  %%xmm1    \n"
               " addps         %%xmm1,  %%xmm3    \n"
               "1:                                \n"
               " inc            %%ecx             \n"
               " jmp               0b             \n"
               "2:                                \n"
               " movss             %5,  %%xmm0    \n"
               " shufps $0x00, %%xmm0,  %%xmm0    \n"
               " minps         %%xmm3,  %%xmm0    \n"
               " cvtss2si      %%xmm0,   %%eax    \n"
               " shll           $0x10,   %%eax    \n"
               " orl      $0xff000000,   %%eax    \n"
               " shufps $0xe5, %%xmm0,  %%xmm0    \n"
               " cvtss2si      %%xmm0,   %%ecx    \n"
               " shll              $8,   %%ecx    \n"
               " orl            %%ecx,   %%eax    \n"
               " shufps $0xe6, %%xmm0,  %%xmm0    \n"
               " cvtss2si      %%xmm0,   %%ecx    \n"
               " orl            %%ecx,   %%eax    \n"
               : "=&a"(rval)
               : "m"(gRSP), "m"(g_normal), "m"(gRSPnumLights), "r"(gRSPlights), "m"(f255), "m"(fZero)
               : "%rcx", "memory", "cc", "%xmm0", "%xmm1", "%xmm3", "%xmm4", "%xmm5"
               );
  return rval;
}
#endif

inline void ReplaceAlphaWithFogFactor(int i)
{
    if( gRDP.geometryMode & G_FOG )
    {
        // Use fog factor to replace vertex alpha
        if( g_vecProjected[i].z > 1 )
            *(((uint8*)&(g_dwVtxDifColor[i]))+3) = 0xFF;
        if( g_vecProjected[i].z < 0 )
            *(((uint8*)&(g_dwVtxDifColor[i]))+3) = 0;
        else
            *(((uint8*)&(g_dwVtxDifColor[i]))+3) = (uint8)(g_vecProjected[i].z*255);    
    }
}


// Bits
// +-+-+-
// xxyyzz
#define Z_NEG  0x01
#define Z_POS  0x02
#define Y_NEG  0x04
#define Y_POS  0x08
#define X_NEG  0x10
#define X_POS  0x20

// Assumes dwAddr has already been checked! 
// Don't inline - it's too big with the transform macros

#if !defined(NO_ASM)
void ProcessVertexDataSSE(uint32 dwAddr, uint32 dwV0, uint32 dwNum)
{
    UpdateCombinedMatrix();

    // This function is called upon SPvertex
    // - do vertex matrix transform
    // - do vertex lighting
    // - do texture cooridinate transform if needed
    // - calculate normal vector

    // Output:  - g_vecProjected[i]             -> transformed vertex x,y,z
    //          - g_vecProjected[i].w                       -> saved vertex 1/w
    //          - g_dwVtxFlags[i]               -> flags
    //          - g_dwVtxDifColor[i]            -> vertex color
    //          - g_fVtxTxtCoords[i]                -> vertex texture cooridinates

    FiddledVtx * pVtxBase = (FiddledVtx*)(g_pRDRAMu8 + dwAddr);
    g_pVtxBase = pVtxBase;

    uint32 i;
    for (i = dwV0; i < dwV0 + dwNum; i++)
    {
        SP_Timing(RSP_GBI0_Vtx);

        FiddledVtx & vert = pVtxBase[i - dwV0];

        g_vtxNonTransformed[i].x = (float)vert.x;
        g_vtxNonTransformed[i].y = (float)vert.y;
        g_vtxNonTransformed[i].z = (float)vert.z;

        SSEVec3Transform(i);

        if( gRSP.bFogEnabled )
        {
            g_fFogCoord[i] = g_vecProjected[i].z;
            if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < 0 || g_fFogCoord[i] < gRSPfFogMin )
                g_fFogCoord[i] = gRSPfFogMin;
        }

        ReplaceAlphaWithFogFactor(i);


        VTX_DUMP( 
        {
            uint32 *dat = (uint32*)(&vert);
            DebuggerAppendMsg("vtx %d: %08X %08X %08X %08X", i, dat[0],dat[1],dat[2],dat[3]); 
            DebuggerAppendMsg("      : %f, %f, %f, %f", 
                g_vtxTransformed[i].x,g_vtxTransformed[i].y,g_vtxTransformed[i].z,g_vtxTransformed[i].w);
            DebuggerAppendMsg("      : %f, %f, %f, %f", 
                g_vecProjected[i].x,g_vecProjected[i].y,g_vecProjected[i].z,g_vecProjected[i].w);
        });

        RSP_Vtx_Clipping(i);

        if( gRSP.bLightingEnable )
        {
            g_normal.x = (float)vert.norma.nx;
            g_normal.y = (float)vert.norma.ny;
            g_normal.z = (float)vert.norma.nz;

            SSEVec3TransformNormal();
            if( options.enableHackForGames != HACK_FOR_ZELDA_MM )
                g_dwVtxDifColor[i] = SSELightVert();
            else
                g_dwVtxDifColor[i] = LightVert(g_normal, i);
            *(((uint8*)&(g_dwVtxDifColor[i]))+3) = vert.rgba.a; // still use alpha from the vertex
        }
        else
        {
            if( (gRDP.geometryMode & G_SHADE) == 0 && gRSP.ucode < 5 )  //Shade is disabled
            {
                //FLAT shade
                g_dwVtxDifColor[i] = gRDP.primitiveColor;
            }
            else
            {
                register IColor &color = *(IColor*)&g_dwVtxDifColor[i];
                color.b = vert.rgba.r;
                color.g = vert.rgba.g;
                color.r = vert.rgba.b;
                color.a = vert.rgba.a;
            }
        }

        if( options.bWinFrameMode )
        {
            g_dwVtxDifColor[i] = COLOR_RGBA(vert.rgba.r, vert.rgba.g, vert.rgba.b, vert.rgba.a);
        }

        // Update texture coords n.b. need to divide tu/tv by bogus scale on addition to buffer

        // If the vert is already lit, then there is no normal (and hence we
        // can't generate tex coord)
        if (gRSP.bTextureGen && gRSP.bLightingEnable )
        {
            TexGen(g_fVtxTxtCoords[i].x, g_fVtxTxtCoords[i].y);
        }
        else
        {
            g_fVtxTxtCoords[i].x = (float)vert.tu;
            g_fVtxTxtCoords[i].y = (float)vert.tv; 
        }
    }

    VTX_DUMP(TRACE2("Setting Vertexes: %d - %d\n", dwV0, dwV0+dwNum-1));
    DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at Vertex Cmd");});
}
#endif

void ProcessVertexDataNoSSE(uint32 dwAddr, uint32 dwV0, uint32 dwNum)
{

    UpdateCombinedMatrix();

    // This function is called upon SPvertex
    // - do vertex matrix transform
    // - do vertex lighting
    // - do texture cooridinate transform if needed
    // - calculate normal vector

    // Output:  - g_vecProjected[i]             -> transformed vertex x,y,z
    //          - g_vecProjected[i].w                       -> saved vertex 1/w
    //          - g_dwVtxFlags[i]               -> flags
    //          - g_dwVtxDifColor[i]            -> vertex color
    //          - g_fVtxTxtCoords[i]                -> vertex texture cooridinates

    FiddledVtx * pVtxBase = (FiddledVtx*)(g_pRDRAMu8 + dwAddr);
    g_pVtxBase = pVtxBase;

    uint32 i;
    for (i = dwV0; i < dwV0 + dwNum; i++)
    {
        SP_Timing(RSP_GBI0_Vtx);

        FiddledVtx & vert = pVtxBase[i - dwV0];

        g_vtxNonTransformed[i].x = (float)vert.x;
        g_vtxNonTransformed[i].y = (float)vert.y;
        g_vtxNonTransformed[i].z = (float)vert.z;

        Vec3Transform(&g_vtxTransformed[i], (XVECTOR3*)&g_vtxNonTransformed[i], &gRSPworldProject); // Convert to w=1

        g_vecProjected[i].w = 1.0f / g_vtxTransformed[i].w;
        g_vecProjected[i].x = g_vtxTransformed[i].x * g_vecProjected[i].w;
        g_vecProjected[i].y = g_vtxTransformed[i].y * g_vecProjected[i].w;
        if ((g_curRomInfo.bPrimaryDepthHack || options.enableHackForGames == HACK_FOR_NASCAR ) && gRDP.otherMode.depth_source )
        {
            g_vecProjected[i].z = gRDP.fPrimitiveDepth;
            g_vtxTransformed[i].z = gRDP.fPrimitiveDepth*g_vtxTransformed[i].w;
        }
        else
        {
            g_vecProjected[i].z = g_vtxTransformed[i].z * g_vecProjected[i].w;
        }

        if( gRSP.bFogEnabled )
        {
            g_fFogCoord[i] = g_vecProjected[i].z;
            if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < 0 || g_fFogCoord[i] < gRSPfFogMin )
                g_fFogCoord[i] = gRSPfFogMin;
        }

        VTX_DUMP( 
        {
            uint32 *dat = (uint32*)(&vert);
            DebuggerAppendMsg("vtx %d: %08X %08X %08X %08X", i, dat[0],dat[1],dat[2],dat[3]); 
            DebuggerAppendMsg("      : %f, %f, %f, %f", 
                g_vtxTransformed[i].x,g_vtxTransformed[i].y,g_vtxTransformed[i].z,g_vtxTransformed[i].w);
            DebuggerAppendMsg("      : %f, %f, %f, %f", 
                g_vecProjected[i].x,g_vecProjected[i].y,g_vecProjected[i].z,g_vecProjected[i].w);
        });

        RSP_Vtx_Clipping(i);

        if( gRSP.bLightingEnable )
        {
            g_normal.x = (float)vert.norma.nx;
            g_normal.y = (float)vert.norma.ny;
            g_normal.z = (float)vert.norma.nz;

            Vec3TransformNormal(g_normal, gRSPmodelViewTop);
            g_dwVtxDifColor[i] = LightVert(g_normal, i);
            *(((uint8*)&(g_dwVtxDifColor[i]))+3) = vert.rgba.a; // still use alpha from the vertex
        }
        else
        {
            if( (gRDP.geometryMode & G_SHADE) == 0 && gRSP.ucode < 5 )  //Shade is disabled
            {
                //FLAT shade
                g_dwVtxDifColor[i] = gRDP.primitiveColor;
            }
            else
            {
                register IColor &color = *(IColor*)&g_dwVtxDifColor[i];
                color.b = vert.rgba.r;
                color.g = vert.rgba.g;
                color.r = vert.rgba.b;
                color.a = vert.rgba.a;
            }
        }

        if( options.bWinFrameMode )
        {
            g_dwVtxDifColor[i] = COLOR_RGBA(vert.rgba.r, vert.rgba.g, vert.rgba.b, vert.rgba.a);
        }

        ReplaceAlphaWithFogFactor(i);

        // Update texture coords n.b. need to divide tu/tv by bogus scale on addition to buffer

        // If the vert is already lit, then there is no normal (and hence we
        // can't generate tex coord)
        if (gRSP.bTextureGen && gRSP.bLightingEnable )
        {
            TexGen(g_fVtxTxtCoords[i].x, g_fVtxTxtCoords[i].y);
        }
        else
        {
            g_fVtxTxtCoords[i].x = (float)vert.tu;
            g_fVtxTxtCoords[i].y = (float)vert.tv; 
        }
    }

    VTX_DUMP(TRACE2("Setting Vertexes: %d - %d\n", dwV0, dwV0+dwNum-1));
    DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at Vertex Cmd");});
}

bool PrepareTriangle(uint32 dwV0, uint32 dwV1, uint32 dwV2)
{
    if( status.isVertexShaderEnabled || status.bUseHW_T_L )
    {
        g_vtxIndex[gRSP.numVertices++] = dwV0;
        g_vtxIndex[gRSP.numVertices++] = dwV1;
        g_vtxIndex[gRSP.numVertices++] = dwV2;
        status.dwNumTrisRendered++;
        gRSP.maxVertexID = max(gRSP.maxVertexID,max(dwV0,max(dwV1,dwV2)));
    }
    else
    {
        SP_Timing(SP_Each_Triangle);

        bool textureFlag = (CRender::g_pRender->IsTextureEnabled() || gRSP.ucode == 6 );
        bool openGL = CDeviceBuilder::m_deviceGeneralType == OGL_DEVICE;

        InitVertex(dwV0, gRSP.numVertices, textureFlag, openGL);
        InitVertex(dwV1, gRSP.numVertices+1, textureFlag, openGL);
        InitVertex(dwV2, gRSP.numVertices+2, textureFlag, openGL);

        gRSP.numVertices += 3;
        status.dwNumTrisRendered++;
    }

    return true;
}



// Returns TRUE if it thinks the triangle is visible
// Returns FALSE if it is clipped
bool IsTriangleVisible(uint32 dwV0, uint32 dwV1, uint32 dwV2)
{
    //return true;  //fix me

    if( status.isVertexShaderEnabled || status.bUseHW_T_L ) return true;    // We won't have access to transformed vertex data

    DEBUGGER_ONLY_IF( (!debuggerEnableTestTris || !debuggerEnableCullFace), {return TRUE;});
    
#ifdef DEBUGGER
    // Check vertices are valid!
    if (dwV0 >= MAX_VERTS || dwV1 >= MAX_VERTS || dwV2 >= MAX_VERTS)
        return false;
#endif

    // Here we AND all the flags. If any of the bits is set for all
    // 3 vertices, it means that all three x, y or z lie outside of
    // the current viewing volume.
    // Currently disabled - still seems a bit dodgy
    if ((gRSP.bCullFront || gRSP.bCullBack) && gRDP.otherMode.zmode != 3)
    {
        XVECTOR4 & v0 = g_vecProjected[dwV0];
        XVECTOR4 & v1 = g_vecProjected[dwV1];
        XVECTOR4 & v2 = g_vecProjected[dwV2];

        // Only try to clip if the tri is onscreen. For some reason, this
        // method doesnt' work well when the z value is outside of screenspace
        //if (v0.z < 1 && v1.z < 1 && v2.z < 1)
        {
            float V1 = v2.x - v0.x;
            float V2 = v2.y - v0.y;
            
            float W1 = v2.x - v1.x;
            float W2 = v2.y - v1.y;

            float fDirection = (V1 * W2) - (V2 * W1);
            fDirection = fDirection * v1.w * v2.w * v0.w;
            //float fDirection = v0.x*v1.y-v1.x*v0.y+v1.x*v2.y-v2.x*v1.y+v2.x*v0.y-v0.x*v2.y;
            /*
            */

            if (fDirection < 0 && gRSP.bCullBack)
            {
                status.dwNumTrisClipped++;
                return false;
            }
            else if (fDirection > 0 && gRSP.bCullFront)
            {
                status.dwNumTrisClipped++;
                return false;
            }
        }
    }
    
#ifdef ENABLE_CLIP_TRI
    //if( gRSP.bRejectVtx && (g_clipFlag[dwV0]|g_clipFlag[dwV1]|g_clipFlag[dwV2]) ) 
    //  return;
    if( g_clipFlag2[dwV0]&g_clipFlag2[dwV1]&g_clipFlag2[dwV2] )
    {
        //DebuggerAppendMsg("Clipped");
        return false;
    }
#endif

    return true;
}


void SetPrimitiveColor(uint32 dwCol, uint32 LODMin, uint32 LODFrac)
{
    gRDP.colorsAreReloaded = true;
    gRDP.primitiveColor = dwCol;
    gRDP.primLODMin = LODMin;
    gRDP.primLODFrac = LODFrac;
    if( gRDP.primLODFrac < gRDP.primLODMin )
    {
        gRDP.primLODFrac = gRDP.primLODMin;
    }

    gRDP.fvPrimitiveColor[0] = ((dwCol>>16)&0xFF)/255.0f;       //r
    gRDP.fvPrimitiveColor[1] = ((dwCol>>8)&0xFF)/255.0f;        //g
    gRDP.fvPrimitiveColor[2] = ((dwCol)&0xFF)/255.0f;           //b
    gRDP.fvPrimitiveColor[3] = ((dwCol>>24)&0xFF)/255.0f;       //a
}

void SetPrimitiveDepth(uint32 z, uint32 dwDZ)
{
    gRDP.primitiveDepth = z & 0x7FFF;
    gRDP.fPrimitiveDepth = (float)(gRDP.primitiveDepth)/(float)0x8000;

    //gRDP.fPrimitiveDepth = gRDP.fPrimitiveDepth*2-1;  
    /*
    z=0xFFFF    ->  1   the farest
    z=0         ->  -1  the nearest
    */

    //how to use dwDZ?

#ifdef DEBUGGER
    if( (pauseAtNext && (eventToPause == NEXT_VERTEX_CMD || eventToPause == NEXT_FLUSH_TRI )) )//&& logTriangles ) 
    {
        DebuggerAppendMsg("Set prim Depth: %f, (%08X, %08X)", gRDP.fPrimitiveDepth, z, dwDZ); 
    }
#endif
}

void SetVertexXYZ(uint32 vertex, float x, float y, float z)
{
    g_vecProjected[vertex].x = x;
    g_vecProjected[vertex].y = y;
    g_vecProjected[vertex].z = z;

    g_vtxTransformed[vertex].x = x*g_vtxTransformed[vertex].w;
    g_vtxTransformed[vertex].y = y*g_vtxTransformed[vertex].w;
    g_vtxTransformed[vertex].z = z*g_vtxTransformed[vertex].w;
}

void ModifyVertexInfo(uint32 where, uint32 vertex, uint32 val)
{
    switch (where)
    {
    case RSP_MV_WORD_OFFSET_POINT_RGBA:     // Modify RGBA
        {
            uint32 r = (val>>24)&0xFF;
            uint32 g = (val>>16)&0xFF;
            uint32 b = (val>>8)&0xFF;
            uint32 a = val&0xFF;
            g_dwVtxDifColor[vertex] = COLOR_RGBA(r, g, b, a);
            LOG_UCODE("Modify vert %d color, 0x%08x", vertex, g_dwVtxDifColor[vertex]);
        }
        break;
    case RSP_MV_WORD_OFFSET_POINT_XYSCREEN:     // Modify X,Y
        {
            uint16 nX = (uint16)(val>>16);
            short x = *((short*)&nX);
            x /= 4;

            uint16 nY = (uint16)(val&0xFFFF);
            short y = *((short*)&nY);
            y /= 4;

            // Should do viewport transform.


            x -= windowSetting.uViWidth/2;
            y = windowSetting.uViHeight/2-y;

            if( options.bEnableHacks && ((*g_GraphicsInfo.VI_X_SCALE_REG)&0xF) != 0 )
            {
                // Tarzan
                // I don't know why Tarzan is different
                SetVertexXYZ(vertex, x/windowSetting.fViWidth, y/windowSetting.fViHeight, g_vecProjected[vertex].z);
            }
            else
            {
                // Toy Story 2 and other games
                SetVertexXYZ(vertex, x*2/windowSetting.fViWidth, y*2/windowSetting.fViHeight, g_vecProjected[vertex].z);
            }

            LOG_UCODE("Modify vert %d: x=%d, y=%d", vertex, x, y);
            VTX_DUMP(TRACE3("Modify vert %d: (%d,%d)", vertex, x, y));
        }
        break;
    case RSP_MV_WORD_OFFSET_POINT_ZSCREEN:      // Modify C
        {
            int z = val>>16;

            SetVertexXYZ(vertex, g_vecProjected[vertex].x, g_vecProjected[vertex].y, (((float)z/0x03FF)+0.5f)/2.0f );
            LOG_UCODE("Modify vert %d: z=%d", vertex, z);
            VTX_DUMP(TRACE2("Modify vert %d: z=%d", vertex, z));
        }
        break;
    case RSP_MV_WORD_OFFSET_POINT_ST:       // Texture
        {
            short tu = short(val>>16);
            short tv = short(val & 0xFFFF);
            float ftu = tu / 32.0f;
            float ftv = tv / 32.0f;
            LOG_UCODE("      Setting vertex %d tu/tv to %f, %f", vertex, (float)tu, (float)tv);
            CRender::g_pRender->SetVtxTextureCoord(vertex, ftu/gRSP.fTexScaleX, ftv/gRSP.fTexScaleY);
        }
        break;
    }
    DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at ModVertex Cmd");});
}

void ProcessVertexDataDKR(uint32 dwAddr, uint32 dwV0, uint32 dwNum)
{
    UpdateCombinedMatrix();

    long pVtxBase = (long) (g_pRDRAMu8 + dwAddr);
    g_pVtxBase = (FiddledVtx*)pVtxBase;

    Matrix &matWorldProject = gRSP.DKRMatrixes[gRSP.DKRCMatrixIndex];

    uint32 i;
    int nOff;

    bool addbase=false;
    if ((!gRSP.DKRBillBoard) || (gRSP.DKRCMatrixIndex != 2) )
        addbase = false;
    else
        addbase = true;

    if( addbase && gRSP.DKRVtxCount == 0 && dwNum > 1 )
    {
        gRSP.DKRVtxCount++;
    }

    LOG_UCODE("    ProcessVertexDataDKR, CMatrix = %d, Add base=%s", gRSP.DKRCMatrixIndex, gRSP.DKRBillBoard?"true":"false");
    VTX_DUMP(TRACE2("DKR Setting Vertexes\nCMatrix = %d, Add base=%s", gRSP.DKRCMatrixIndex, gRSP.DKRBillBoard?"true":"false"));

    nOff = 0;
    uint32 end = dwV0 + dwNum;
    for (i = dwV0; i < end; i++)
    {
        XVECTOR3 w;

        g_vtxNonTransformed[i].x = (float)*(short*)((pVtxBase+nOff + 0) ^ 2);
        g_vtxNonTransformed[i].y = (float)*(short*)((pVtxBase+nOff + 2) ^ 2);
        g_vtxNonTransformed[i].z = (float)*(short*)((pVtxBase+nOff + 4) ^ 2);

        //if( status.isSSEEnabled )
        //  SSEVec3TransformDKR(g_vtxTransformed[i], g_vtxNonTransformed[i]);
        //else
            Vec3Transform(&g_vtxTransformed[i], (XVECTOR3*)&g_vtxNonTransformed[i], &matWorldProject);  // Convert to w=1

        if( gRSP.DKRVtxCount == 0 && dwNum==1 )
        {
            gRSP.DKRBaseVec.x = g_vtxTransformed[i].x;
            gRSP.DKRBaseVec.y = g_vtxTransformed[i].y;
            gRSP.DKRBaseVec.z = g_vtxTransformed[i].z;
            gRSP.DKRBaseVec.w = g_vtxTransformed[i].w;
        }
        else if( addbase )
        {
            g_vtxTransformed[i].x += gRSP.DKRBaseVec.x;
            g_vtxTransformed[i].y += gRSP.DKRBaseVec.y;
            g_vtxTransformed[i].z += gRSP.DKRBaseVec.z;
            g_vtxTransformed[i].w  = gRSP.DKRBaseVec.w;
        }

        g_vecProjected[i].w = 1.0f / g_vtxTransformed[i].w;
        g_vecProjected[i].x = g_vtxTransformed[i].x * g_vecProjected[i].w;
        g_vecProjected[i].y = g_vtxTransformed[i].y * g_vecProjected[i].w;
        g_vecProjected[i].z = g_vtxTransformed[i].z * g_vecProjected[i].w;

        gRSP.DKRVtxCount++;

        VTX_DUMP(TRACE5("vtx %d: %f, %f, %f, %f", i, 
            g_vtxTransformed[i].x,g_vtxTransformed[i].y,g_vtxTransformed[i].z,g_vtxTransformed[i].w));

        if( gRSP.bFogEnabled )
        {
            g_fFogCoord[i] = g_vecProjected[i].z;
            if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < 0 || g_fFogCoord[i] < gRSPfFogMin )
                g_fFogCoord[i] = gRSPfFogMin;
        }

        RSP_Vtx_Clipping(i);

        short wA = *(short*)((pVtxBase+nOff + 6) ^ 2);
        short wB = *(short*)((pVtxBase+nOff + 8) ^ 2);

        s8 r = (s8)(wA >> 8);
        s8 g = (s8)(wA);
        s8 b = (s8)(wB >> 8);
        s8 a = (s8)(wB);

        if (gRSP.bLightingEnable)
        {
            g_normal.x = (char)r; //norma.nx;
            g_normal.y = (char)g; //norma.ny;
            g_normal.z = (char)b; //norma.nz;

            Vec3TransformNormal(g_normal, matWorldProject)
#if !defined(NO_ASM)
            if( status.isSSEEnabled )
                g_dwVtxDifColor[i] = SSELightVert();
            else
#endif
                g_dwVtxDifColor[i] = LightVert(g_normal, i);
        }
        else
        {
            int nR, nG, nB, nA;

            nR = r;
            nG = g;
            nB = b;
            nA = a;
            // Assign true vert colour after lighting/fogging
            g_dwVtxDifColor[i] = COLOR_RGBA(nR, nG, nB, nA);
        }

        ReplaceAlphaWithFogFactor(i);

        g_fVtxTxtCoords[i].x = g_fVtxTxtCoords[i].y = 1;

        nOff += 10;
    }


    DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{DebuggerAppendMsg("Paused at DKR Vertex Cmd, v0=%d, vn=%d, addr=%08X", dwV0, dwNum, dwAddr);});
}


extern uint32 dwPDCIAddr;
void ProcessVertexDataPD(uint32 dwAddr, uint32 dwV0, uint32 dwNum)
{
    UpdateCombinedMatrix();

    N64VtxPD * pVtxBase = (N64VtxPD*)(g_pRDRAMu8 + dwAddr);
    g_pVtxBase = (FiddledVtx*)pVtxBase; // Fix me

    for (uint32 i = dwV0; i < dwV0 + dwNum; i++)
    {
        N64VtxPD &vert = pVtxBase[i - dwV0];

        g_vtxNonTransformed[i].x = (float)vert.x;
        g_vtxNonTransformed[i].y = (float)vert.y;
        g_vtxNonTransformed[i].z = (float)vert.z;

#if !defined(NO_ASM)
        if( status.isSSEEnabled )
            SSEVec3Transform(i);
        else
#endif
        {
            Vec3Transform(&g_vtxTransformed[i], (XVECTOR3*)&g_vtxNonTransformed[i], &gRSPworldProject); // Convert to w=1
            g_vecProjected[i].w = 1.0f / g_vtxTransformed[i].w;
            g_vecProjected[i].x = g_vtxTransformed[i].x * g_vecProjected[i].w;
            g_vecProjected[i].y = g_vtxTransformed[i].y * g_vecProjected[i].w;
            g_vecProjected[i].z = g_vtxTransformed[i].z * g_vecProjected[i].w;
        }

        g_fFogCoord[i] = g_vecProjected[i].z;
        if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < 0 || g_fFogCoord[i] < gRSPfFogMin )
            g_fFogCoord[i] = gRSPfFogMin;

        RSP_Vtx_Clipping(i);

        uint8 *addr = g_pRDRAMu8+dwPDCIAddr+ (vert.cidx&0xFF);
        uint32 a = addr[0];
        uint32 r = addr[3];
        uint32 g = addr[2];
        uint32 b = addr[1];

        if( gRSP.bLightingEnable )
        {
            g_normal.x = (char)r;
            g_normal.y = (char)g;
            g_normal.z = (char)b;
#if !defined(NO_ASM)
            if( status.isSSEEnabled )
            {
                SSEVec3TransformNormal();
                g_dwVtxDifColor[i] = SSELightVert();
            }
            else
#endif
            {
                Vec3TransformNormal(g_normal, gRSPmodelViewTop);
                g_dwVtxDifColor[i] = LightVert(g_normal, i);
            }
            *(((uint8*)&(g_dwVtxDifColor[i]))+3) = (uint8)a;    // still use alpha from the vertex
        }
        else
        {
            if( (gRDP.geometryMode & G_SHADE) == 0 && gRSP.ucode < 5 )  //Shade is disabled
            {
                g_dwVtxDifColor[i] = gRDP.primitiveColor;
            }
            else    //FLAT shade
            {
                g_dwVtxDifColor[i] = COLOR_RGBA(r, g, b, a);
            }
        }

        if( options.bWinFrameMode )
        {
            g_dwVtxDifColor[i] = COLOR_RGBA(r, g, b, a);
        }

        ReplaceAlphaWithFogFactor(i);

        VECTOR2 & t = g_fVtxTxtCoords[i];
        if (gRSP.bTextureGen && gRSP.bLightingEnable )
        {
            // Not sure if we should transform the normal here
            //Matrix & matWV = gRSP.projectionMtxs[gRSP.projectionMtxTop];
            //Vec3TransformNormal(g_normal, matWV);

            TexGen(g_fVtxTxtCoords[i].x, g_fVtxTxtCoords[i].y);
        }
        else
        {
            t.x = vert.s;
            t.y = vert.t; 
        }


        VTX_DUMP( 
        {
            DebuggerAppendMsg("vtx %d: %d %d %d", i, vert.x,vert.y,vert.z); 
            DebuggerAppendMsg("      : %f, %f, %f, %f", 
                g_vtxTransformed[i].x,g_vtxTransformed[i].y,g_vtxTransformed[i].z,g_vtxTransformed[i].w);
            DebuggerAppendMsg("      : %X, %X, %X, %X", r,g,b,a);
            DebuggerAppendMsg("      : u=%f, v=%f", t.x, t.y);
        });
    }

    VTX_DUMP(TRACE2("Setting Vertexes: %d - %d\n", dwV0, dwV0+dwNum-1));
    DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at Vertex Cmd");});
}

extern uint32 dwConkerVtxZAddr;
void ProcessVertexDataConker(uint32 dwAddr, uint32 dwV0, uint32 dwNum)
{
    UpdateCombinedMatrix();

    FiddledVtx * pVtxBase = (FiddledVtx*)(g_pRDRAMu8 + dwAddr);
    g_pVtxBase = pVtxBase;
    //short *vertexColoraddr = (short*)(g_pRDRAMu8+dwConkerVtxZAddr);

    uint32 i;
    for (i = dwV0; i < dwV0 + dwNum; i++)
    {
        SP_Timing(RSP_GBI0_Vtx);

        FiddledVtx & vert = pVtxBase[i - dwV0];

        g_vtxNonTransformed[i].x = (float)vert.x;
        g_vtxNonTransformed[i].y = (float)vert.y;
        g_vtxNonTransformed[i].z = (float)vert.z;

#if !defined(NO_ASM)
        if( status.isSSEEnabled )
            SSEVec3Transform(i);
        else
#endif
        {
            Vec3Transform(&g_vtxTransformed[i], (XVECTOR3*)&g_vtxNonTransformed[i], &gRSPworldProject); // Convert to w=1
            g_vecProjected[i].w = 1.0f / g_vtxTransformed[i].w;
            g_vecProjected[i].x = g_vtxTransformed[i].x * g_vecProjected[i].w;
            g_vecProjected[i].y = g_vtxTransformed[i].y * g_vecProjected[i].w;
            g_vecProjected[i].z = g_vtxTransformed[i].z * g_vecProjected[i].w;
        }

        g_fFogCoord[i] = g_vecProjected[i].z;
        if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < 0 || g_fFogCoord[i] < gRSPfFogMin )
            g_fFogCoord[i] = gRSPfFogMin;

        VTX_DUMP( 
        {
            uint32 *dat = (uint32*)(&vert);
            DebuggerAppendMsg("vtx %d: %08X %08X %08X %08X", i, dat[0],dat[1],dat[2],dat[3]); 
            DebuggerAppendMsg("      : %f, %f, %f, %f", 
                g_vtxTransformed[i].x,g_vtxTransformed[i].y,g_vtxTransformed[i].z,g_vtxTransformed[i].w);
            DebuggerAppendMsg("      : %f, %f, %f, %f", 
                g_vecProjected[i].x,g_vecProjected[i].y,g_vecProjected[i].z,g_vecProjected[i].w);
        });

        RSP_Vtx_Clipping(i);

        if( gRSP.bLightingEnable )
        {
            {
                uint32 r= ((gRSP.ambientLightColor>>16)&0xFF);
                uint32 g= ((gRSP.ambientLightColor>> 8)&0xFF);
                uint32 b= ((gRSP.ambientLightColor    )&0xFF);
                for( uint32 k=1; k<=gRSPnumLights; k++)
                {
                    r += gRSPlights[k].r;
                    g += gRSPlights[k].g;
                    b += gRSPlights[k].b;
                }
                if( r>255 ) r=255;
                if( g>255 ) g=255;
                if( b>255 ) b=255;
                r *= vert.rgba.r ;
                g *= vert.rgba.g ;
                b *= vert.rgba.b ;
                r >>= 8;
                g >>= 8;
                b >>= 8;
                g_dwVtxDifColor[i] = 0xFF000000;
                g_dwVtxDifColor[i] |= (r<<16);
                g_dwVtxDifColor[i] |= (g<< 8);
                g_dwVtxDifColor[i] |= (b    );          
            }

            *(((uint8*)&(g_dwVtxDifColor[i]))+3) = vert.rgba.a; // still use alpha from the vertex
        }
        else
        {
            if( (gRDP.geometryMode & G_SHADE) == 0 && gRSP.ucode < 5 )  //Shade is disabled
            {
                g_dwVtxDifColor[i] = gRDP.primitiveColor;
            }
            else    //FLAT shade
            {
                g_dwVtxDifColor[i] = COLOR_RGBA(vert.rgba.r, vert.rgba.g, vert.rgba.b, vert.rgba.a);
            }
        }

        if( options.bWinFrameMode )
        {
            //g_vecProjected[i].z = 0;
            g_dwVtxDifColor[i] = COLOR_RGBA(vert.rgba.r, vert.rgba.g, vert.rgba.b, vert.rgba.a);
        }

        ReplaceAlphaWithFogFactor(i);

        // Update texture coords n.b. need to divide tu/tv by bogus scale on addition to buffer
        //VECTOR2 & t = g_fVtxTxtCoords[i];

        // If the vert is already lit, then there is no normal (and hence we
        // can't generate tex coord)
        if (gRSP.bTextureGen && gRSP.bLightingEnable )
        {
                g_normal.x = (float)*(char*)(g_pRDRAMu8+ (((i<<1)+0)^3)+dwConkerVtxZAddr);
                g_normal.y = (float)*(char*)(g_pRDRAMu8+ (((i<<1)+1)^3)+dwConkerVtxZAddr);
                g_normal.z = (float)*(char*)(g_pRDRAMu8+ (((i<<1)+2)^3)+dwConkerVtxZAddr);
                Vec3TransformNormal(g_normal, gRSPmodelViewTop);
                TexGen(g_fVtxTxtCoords[i].x, g_fVtxTxtCoords[i].y);
        }
        else
        {
            g_fVtxTxtCoords[i].x = (float)vert.tu;
            g_fVtxTxtCoords[i].y = (float)vert.tv; 
        }
    }

    VTX_DUMP(TRACE2("Setting Vertexes: %d - %d\n", dwV0, dwV0+dwNum-1));
    DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{DebuggerAppendMsg("Paused at Vertex Cmd");});
}


typedef struct{
    short y;
    short x;
    short flag;
    short z;
} RS_Vtx_XYZ;

typedef union {
    struct {
        uint8 a;
        uint8 b;
        uint8 g;
        uint8 r;
    };
    struct {
        char na;
        char nz;    //b
        char ny;    //g
        char nx;    //r
    };
} RS_Vtx_Color;


void ProcessVertexData_Rogue_Squadron(uint32 dwXYZAddr, uint32 dwColorAddr, uint32 dwXYZCmd, uint32 dwColorCmd)
{
    UpdateCombinedMatrix();

    uint32 dwV0 = 0;
    uint32 dwNum = (dwXYZCmd&0xFF00)>>10;

    RS_Vtx_XYZ * pVtxXYZBase = (RS_Vtx_XYZ*)(g_pRDRAMu8 + dwXYZAddr);
    RS_Vtx_Color * pVtxColorBase = (RS_Vtx_Color*)(g_pRDRAMu8 + dwColorAddr);

    uint32 i;
    for (i = dwV0; i < dwV0 + dwNum; i++)
    {
        RS_Vtx_XYZ & vertxyz = pVtxXYZBase[i - dwV0];
        RS_Vtx_Color & vertcolors = pVtxColorBase[i - dwV0];

        g_vtxNonTransformed[i].x = (float)vertxyz.x;
        g_vtxNonTransformed[i].y = (float)vertxyz.y;
        g_vtxNonTransformed[i].z = (float)vertxyz.z;

#if !defined(NO_ASM)
        if( status.isSSEEnabled )
            SSEVec3Transform(i);
        else
#endif
        {
            Vec3Transform(&g_vtxTransformed[i], (XVECTOR3*)&g_vtxNonTransformed[i], &gRSPworldProject); // Convert to w=1
            g_vecProjected[i].w = 1.0f / g_vtxTransformed[i].w;
            g_vecProjected[i].x = g_vtxTransformed[i].x * g_vecProjected[i].w;
            g_vecProjected[i].y = g_vtxTransformed[i].y * g_vecProjected[i].w;
            g_vecProjected[i].z = g_vtxTransformed[i].z * g_vecProjected[i].w;
        }

        VTX_DUMP( 
        {
            DebuggerAppendMsg("      : %f, %f, %f, %f", 
                g_vtxTransformed[i].x,g_vtxTransformed[i].y,g_vtxTransformed[i].z,g_vtxTransformed[i].w);
            DebuggerAppendMsg("      : %f, %f, %f, %f", 
                g_vecProjected[i].x,g_vecProjected[i].y,g_vecProjected[i].z,g_vecProjected[i].w);
        });

        g_fFogCoord[i] = g_vecProjected[i].z;
        if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < 0 || g_fFogCoord[i] < gRSPfFogMin )
            g_fFogCoord[i] = gRSPfFogMin;

        RSP_Vtx_Clipping(i);

        if( gRSP.bLightingEnable )
        {
            g_normal.x = (float)vertcolors.nx;
            g_normal.y = (float)vertcolors.ny;
            g_normal.z = (float)vertcolors.nz;

#if !defined(NO_ASM)
            if( status.isSSEEnabled )
            {
                SSEVec3TransformNormal();
                g_dwVtxDifColor[i] = SSELightVert();
            }
            else
#endif
            {
                Vec3TransformNormal(g_normal, gRSPmodelViewTop);
                g_dwVtxDifColor[i] = LightVert(g_normal, i);
            }
            *(((uint8*)&(g_dwVtxDifColor[i]))+3) = vertcolors.a;    // still use alpha from the vertex
        }
        else
        {
            if( (gRDP.geometryMode & G_SHADE) == 0 && gRSP.ucode < 5 )  //Shade is disabled
            {
                g_dwVtxDifColor[i] = gRDP.primitiveColor;
            }
            else    //FLAT shade
            {
                g_dwVtxDifColor[i] = COLOR_RGBA(vertcolors.r, vertcolors.g, vertcolors.b, vertcolors.a);
            }
        }

        if( options.bWinFrameMode )
        {
            g_dwVtxDifColor[i] = COLOR_RGBA(vertcolors.r, vertcolors.g, vertcolors.b, vertcolors.a);
        }

        ReplaceAlphaWithFogFactor(i);

        /*
        // Update texture coords n.b. need to divide tu/tv by bogus scale on addition to buffer
        VECTOR2 & t = g_fVtxTxtCoords[i];

        // If the vert is already lit, then there is no normal (and hence we
        // can't generate tex coord)
        if (gRSP.bTextureGen && gRSP.bLightingEnable && g_textures[gRSP.curTile].m_bTextureEnable )
        {
            TexGen(g_fVtxTxtCoords[i].x, g_fVtxTxtCoords[i].y);
        }
        else
        {
            t.x = (float)vert.tu;
            t.y = (float)vert.tv; 
        }
        */
    }

    VTX_DUMP(TRACE2("Setting Vertexes: %d - %d\n", dwV0, dwV0+dwNum-1));
    DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at Vertex Cmd");});
}

void SetLightCol(uint32 dwLight, uint32 dwCol)
{
    gRSPlights[dwLight].r = (uint8)((dwCol >> 24)&0xFF);
    gRSPlights[dwLight].g = (uint8)((dwCol >> 16)&0xFF);
    gRSPlights[dwLight].b = (uint8)((dwCol >>  8)&0xFF);
    gRSPlights[dwLight].a = 255;    // Ignore light alpha
    gRSPlights[dwLight].fr = (float)gRSPlights[dwLight].r;
    gRSPlights[dwLight].fg = (float)gRSPlights[dwLight].g;
    gRSPlights[dwLight].fb = (float)gRSPlights[dwLight].b;
    gRSPlights[dwLight].fa = 255;   // Ignore light alpha

    //TRACE1("Set light %d color", dwLight);
    LIGHT_DUMP(TRACE2("Set Light %d color: %08X", dwLight, dwCol));
}

void SetLightDirection(uint32 dwLight, float x, float y, float z, float range)
{
    //gRSP.bLightIsUpdated = true;

    //gRSPlights[dwLight].ox = x;
    //gRSPlights[dwLight].oy = y;
    //gRSPlights[dwLight].oz = z;

    register float w = range == 0 ? (float)sqrt(x*x+y*y+z*z) : 1;

    gRSPlights[dwLight].x = x/w;
    gRSPlights[dwLight].y = y/w;
    gRSPlights[dwLight].z = z/w;
    gRSPlights[dwLight].range = range;
    DEBUGGER_PAUSE_AND_DUMP(NEXT_SET_LIGHT,TRACE5("Set Light %d dir: %.4f, %.4f, %.4f, %.4f", dwLight, x, y, z, range));
}

static float maxS0, maxT0;
static float maxS1, maxT1;
static bool validS0, validT0;
static bool validS1, validT1;

void LogTextureCoords(float fTex0S, float fTex0T, float fTex1S, float fTex1T)
{
    if( validS0 )
    {
        if( fTex0S<0 || fTex0S>maxS0 )  validS0 = false;
    }
    if( validT0 )
    {
        if( fTex0T<0 || fTex0T>maxT0 )  validT0 = false;
    }
    if( validS1 )
    {
        if( fTex1S<0 || fTex1S>maxS1 )  validS1 = false;
    }
    if( validT1 )
    {
        if( fTex1T<0 || fTex1T>maxT1 )  validT1 = false;
    }
}

bool CheckTextureCoords(int tex)
{
    if( tex==0 )
    {
        return validS0&&validT0;
    }
    else
    {
        return validS1&&validT1;
    }
}

void ResetTextureCoordsLog(float maxs0, float maxt0, float maxs1, float maxt1)
{
    maxS0 = maxs0;
    maxT0 = maxt0;
    maxS1 = maxs1;
    maxT1 = maxt1;
    validS0 = validT0 = true;
    validS1 = validT1 = true;
}

void ForceMainTextureIndex(int dwTile) 
{
    if( dwTile == 1 && !(CRender::g_pRender->IsTexel0Enable()) && CRender::g_pRender->IsTexel1Enable() )
    {
        // Hack
        gRSP.curTile = 0;
    }
    else
        gRSP.curTile = dwTile;
}

float HackZ2(float z)
{
    z = (z+9)/10;
    return z;
}

float HackZ(float z)
{
    return HackZ2(z);

    if( z < 0.1 && z >= 0 )
        z = (.1f+z)/2;
    else if( z < 0 )
        //return (10+z)/100;
        z = (expf(z)/20);
    return z;
}

void HackZ(std::vector<XVECTOR3>& points)
{
    int size = points.size();
    for( int i=0; i<size; i++)
    {
        XVECTOR3 &v = points[i];
        v.z = (float)HackZ(v.z);
    }
}

void HackZAll()
{
    if( CDeviceBuilder::m_deviceGeneralType == DIRECTX_DEVICE )
    {
        for( uint32 i=0; i<gRSP.numVertices; i++)
        {
            g_vtxBuffer[i].z = HackZ(g_vtxBuffer[i].z);
        }
    }
    else
    {
        for( uint32 i=0; i<gRSP.numVertices; i++)
        {
            float w = g_vtxProjected5[i][3];
            g_vtxProjected5[i][2] = HackZ(g_vtxProjected5[i][2]/w)*w;
        }
    }
}


extern XMATRIX reverseXY;
extern XMATRIX reverseY;

void UpdateCombinedMatrix()
{
    if( gRSP.bMatrixIsUpdated )
    {
        gRSPworldProject = gRSP.modelviewMtxs[gRSP.modelViewMtxTop] * gRSP.projectionMtxs[gRSP.projectionMtxTop];
        gRSP.bMatrixIsUpdated = false;
        gRSP.bCombinedMatrixIsUpdated = true;
    }

    if( gRSP.bCombinedMatrixIsUpdated )
    {
        if( options.enableHackForGames == HACK_REVERSE_XY_COOR )
        {
            gRSPworldProject = gRSPworldProject * reverseXY;
        }
        if( options.enableHackForGames == HACK_REVERSE_Y_COOR )
        {
            gRSPworldProject = gRSPworldProject * reverseY;
        }
#if !defined(NO_ASM)
        if( status.isSSEEnabled )
        {
            MatrixTranspose(&gRSPworldProjectTransported, &gRSPworldProject);
        }
#endif
        gRSP.bCombinedMatrixIsUpdated = false;
    }

    //if( gRSP.bWorldMatrixIsUpdated || gRSP.bLightIsUpdated )
    //{
    //  // Update lights with transported world matrix
    //  for( unsigned int l=0; l<gRSPnumLights; l++)
    //  {
    //      Vec3TransformCoord(&gRSPlights[l].td, &gRSPlights[l].od, &gRSPmodelViewTopTranspose);
    //      Vec3Normalize(&gRSPlights[l].td,&gRSPlights[l].td);
    //  }

    //  gRSP.bWorldMatrixIsUpdated = false;
    //  gRSP.bLightIsUpdated = false;
    //}
}

