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
// Software rendering into N64 depth buffer
// Idea and N64 depth value format by Orkin
// Polygon rasterization algorithm is taken from FATMAP2 engine by Mats Byggmastar, mri@penti.sit.fi
//
// Created by Gonetz, Dec 2004
//
//****************************************************************

#include "Gfx_1.3.h"
#include "rdp.h"
#include "DepthBufferRender.h"

wxUint16 * zLUT = 0;

void ZLUT_init()
{
  if (zLUT)
    return;
  zLUT = new wxUint16[0x40000];
  for(int i=0; i<0x40000; i++)
  {
    wxUint32 exponent = 0;
    wxUint32 testbit = 1 << 17;
    while((i & testbit) && (exponent < 7))
    {
      exponent++;
      testbit = 1 << (17 - exponent);
    }
    
    wxUint32 mantissa = (i >> (6 - (6 < exponent ? 6 : exponent))) & 0x7ff;
    zLUT[i] = (wxUint16)(((exponent << 11) | mantissa) << 2);
  }
}

void ZLUT_release()
{
  delete[] zLUT;
  zLUT = 0;
}

static vertexi * max_vtx;                   // Max y vertex (ending vertex)
static vertexi * start_vtx, * end_vtx;      // First and last vertex in array
static vertexi * right_vtx, * left_vtx;     // Current right and left vertex

static int right_height, left_height;
static int right_x, right_dxdy, left_x, left_dxdy;
static int left_z, left_dzdy;

__inline int imul16(int x, int y)        // (x * y) >> 16
{
    return (((long long)x) * ((long long)y)) >> 16;
}

__inline int imul14(int x, int y)        // (x * y) >> 14
{
    return (((long long)x) * ((long long)y)) >> 14;
}
__inline int idiv16(int x, int y)        // (x << 16) / y
{
    //x = (((long long)x) << 16) / ((long long)y);
 /*   
  eax = x;
  ebx = y;
  edx = x;
  (x << 16) | ()
   */ 
#if !defined(__GNUC__) && !defined(NO_ASM)
  __asm {
        mov   eax, x
        mov   ebx, y
        mov   edx,eax   
        sar   edx,16
        shl   eax,16    
        idiv  ebx  
        mov   x, eax
    }
#elif !defined(NO_ASM)
    int reminder;
    asm ("idivl %[divisor]"
          : "=a" (x), "=d" (reminder)
          : [divisor] "g" (y), "d" (x >> 16), "a" (x << 16));
#else
	x = (((long long)x) << 16) / ((long long)y);
#endif
    return x;
}

__inline int iceil(int x)
{
  x +=  0xffff;
  return (x >> 16);
}

static void RightSection(void)
{
  // Walk backwards trough the vertex array
  
  vertexi * v2, * v1 = right_vtx;
  if(right_vtx > start_vtx) v2 = right_vtx-1;     
  else                      v2 = end_vtx;         // Wrap to end of array
  right_vtx = v2;
  
  // v1 = top vertex
  // v2 = bottom vertex 
  
  // Calculate number of scanlines in this section
  
  right_height = iceil(v2->y) - iceil(v1->y);
  if(right_height <= 0) return;
  
  // Guard against possible div overflows
  
  if(right_height > 1) {
    // OK, no worries, we have a section that is at least
    // one pixel high. Calculate slope as usual.
    
    int height = v2->y - v1->y;
    right_dxdy  = idiv16(v2->x - v1->x, height);
  }
  else {
    // Height is less or equal to one pixel.
    // Calculate slope = width * 1/height
    // using 18:14 bit precision to avoid overflows.
    
    int inv_height = (0x10000 << 14) / (v2->y - v1->y);  
    right_dxdy = imul14(v2->x - v1->x, inv_height);
  }
  
  // Prestep initial values
  
  int prestep = (iceil(v1->y) << 16) - v1->y;
  right_x = v1->x + imul16(prestep, right_dxdy);
}

static void LeftSection(void)
{
  // Walk forward trough the vertex array
  
  vertexi * v2, * v1 = left_vtx;
  if(left_vtx < end_vtx) v2 = left_vtx+1;
  else                   v2 = start_vtx;      // Wrap to start of array
  left_vtx = v2;
  
  // v1 = top vertex
  // v2 = bottom vertex 
  
  // Calculate number of scanlines in this section
  
  left_height = iceil(v2->y) - iceil(v1->y);
  if(left_height <= 0) return;
  
  // Guard against possible div overflows
  
  if(left_height > 1) {
    // OK, no worries, we have a section that is at least
    // one pixel high. Calculate slope as usual.
    
    int height = v2->y - v1->y;
    left_dxdy = idiv16(v2->x - v1->x, height);
    left_dzdy = idiv16(v2->z - v1->z, height);
  }
  else {
    // Height is less or equal to one pixel.
    // Calculate slope = width * 1/height
    // using 18:14 bit precision to avoid overflows.
    
    int inv_height = (0x10000 << 14) / (v2->y - v1->y);
    left_dxdy = imul14(v2->x - v1->x, inv_height);
    left_dzdy = imul14(v2->z - v1->z, inv_height);
  }
  
  // Prestep initial values
  
  int prestep = (iceil(v1->y) << 16) - v1->y;
  left_x = v1->x + imul16(prestep, left_dxdy);
  left_z = v1->z + imul16(prestep, left_dzdy);
}


void Rasterize(vertexi * vtx, int vertices, int dzdx)
{
  start_vtx = vtx;        // First vertex in array
  
  // Search trough the vtx array to find min y, max y
  // and the location of these structures.
  
  vertexi * min_vtx = vtx;
  max_vtx = vtx;
  
  int min_y = vtx->y;
  int max_y = vtx->y;
  
  vtx++;
  
  for(int n=1; n<vertices; n++) {
    if(vtx->y < min_y) {
      min_y = vtx->y;
      min_vtx = vtx;
    }
    else
      if(vtx->y > max_y) {
        max_y = vtx->y;
        max_vtx = vtx;
      }
      vtx++;
  }
  
  // OK, now we know where in the array we should start and
  // where to end while scanning the edges of the polygon
  
  left_vtx  = min_vtx;    // Left side starting vertex
  right_vtx = min_vtx;    // Right side starting vertex
  end_vtx   = vtx-1;      // Last vertex in array
  
  // Search for the first usable right section
  
  do {
    if(right_vtx == max_vtx) return;
    RightSection();
  } while(right_height <= 0);
  
  // Search for the first usable left section
  
  do {
    if(left_vtx == max_vtx) return;
    LeftSection();
  } while(left_height <= 0);
  
  wxUint16 * destptr = (wxUint16*)(gfx.RDRAM+rdp.zimg);
  int y1 = iceil(min_y);
  if (y1 >= (int)rdp.scissor_o.lr_y) return;
  int shift;
  
  for(;;)
  {
    int x1 = iceil(left_x);
    if (x1 < (int)rdp.scissor_o.ul_x)
      x1 = rdp.scissor_o.ul_x;
    int width = iceil(right_x) - x1;
    if (x1+width >= (int)rdp.scissor_o.lr_x)
      width = rdp.scissor_o.lr_x - x1 - 1;
    
    if(width > 0 && y1 >= (int)rdp.scissor_o.ul_y) {
      
      // Prestep initial z
      
      int prestep = (x1 << 16) - left_x;
      int z = left_z + imul16(prestep, dzdx);
      
      shift = x1 + y1*rdp.zi_width;
      //draw to depth buffer
      int trueZ;
      int idx;
      wxUint16 encodedZ;
      for (int x = 0; x < width; x++)
      {
        trueZ = z/8192;
        if (trueZ < 0) trueZ = 0;
        else if (trueZ > 0x3FFFF) trueZ = 0x3FFFF;
        encodedZ = zLUT[trueZ];
        idx = (shift+x)^1;
        if(encodedZ < destptr[idx]) 
          destptr[idx] = encodedZ;
        z += dzdx;
      }
    }
    
    //destptr += rdp.zi_width;
    y1++;
    if (y1 >= (int)rdp.scissor_o.lr_y) return;
    
    // Scan the right side
    
    if(--right_height <= 0) {               // End of this section?
      do {
        if(right_vtx == max_vtx) return;
        RightSection();
      } while(right_height <= 0);
    }
    else 
      right_x += right_dxdy;
    
    // Scan the left side
    
    if(--left_height <= 0) {                // End of this section?
      do {
        if(left_vtx == max_vtx) return;
        LeftSection();
      } while(left_height <= 0);
    }
    else {
      left_x += left_dxdy;
      left_z += left_dzdy;
    }
  }
}
