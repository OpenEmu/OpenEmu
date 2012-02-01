/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - fpu.c                                                   *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2010 Ari64                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <math.h>

#include "r4300.h"

#ifdef _MSC_VER
  #define M64P_FPU_INLINE static __inline
  #include <float.h>
  typedef enum { FE_TONEAREST = 0, FE_TOWARDZERO, FE_UPWARD, FE_DOWNWARD } eRoundType;
  static void fesetround(eRoundType RoundType)
  {
    static const unsigned int msRound[4] = { _RC_NEAR, _RC_CHOP, _RC_UP, _RC_DOWN };
    unsigned int oldX87, oldSSE2;
    __control87_2(msRound[RoundType], _MCW_RC, &oldX87, &oldSSE2);
  }
  static __inline double round(double x) { return floor(x + 0.5); }
  static __inline float roundf(float x) { return (float) floor(x + 0.5); }
  static __inline double trunc(double x) { return (double) (int) x; }
  static __inline float truncf(float x) { return (float) (int) x; }
  #define isnan _isnan
#else
  #define M64P_FPU_INLINE static inline
  #include <fenv.h>
#endif


M64P_FPU_INLINE void set_rounding(void)
{
  switch(rounding_mode) {
  case 0x33F:
    fesetround(FE_TONEAREST);
    break;
  case 0xF3F:
    fesetround(FE_TOWARDZERO);
    break;
  case 0xB3F:
    fesetround(FE_UPWARD);
    break;
  case 0x73F:
    fesetround(FE_DOWNWARD);
    break;
  }
}

M64P_FPU_INLINE void cvt_s_w(int *source,float *dest)
{
  set_rounding();
  *dest = (float) *source;
}
M64P_FPU_INLINE void cvt_d_w(int *source,double *dest)
{
  set_rounding();
  *dest = (double) *source;
}
M64P_FPU_INLINE void cvt_s_l(long long *source,float *dest)
{
  set_rounding();
  *dest = (float) *source;
}
M64P_FPU_INLINE void cvt_d_l(long long *source,double *dest)
{
  set_rounding();
  *dest = (double) *source;
}
M64P_FPU_INLINE void cvt_d_s(float *source,double *dest)
{
  set_rounding();
  *dest = (double) *source;
}
M64P_FPU_INLINE void cvt_s_d(double *source,float *dest)
{
  set_rounding();
  *dest = (float) *source;
}

M64P_FPU_INLINE void round_l_s(float *source,long long *dest)
{
  *dest = (long long) roundf(*source);
}
M64P_FPU_INLINE void round_w_s(float *source,int *dest)
{
  *dest = (int) roundf(*source);
}
M64P_FPU_INLINE void trunc_l_s(float *source,long long *dest)
{
  *dest = (long long) truncf(*source);
}
M64P_FPU_INLINE void trunc_w_s(float *source,int *dest)
{
  *dest = (int) truncf(*source);
}
M64P_FPU_INLINE void ceil_l_s(float *source,long long *dest)
{
  *dest = (long long) ceilf(*source);
}
M64P_FPU_INLINE void ceil_w_s(float *source,int *dest)
{
  *dest = (int) ceilf(*source);
}
M64P_FPU_INLINE void floor_l_s(float *source,long long *dest)
{
  *dest = (long long) floorf(*source);
}
M64P_FPU_INLINE void floor_w_s(float *source,int *dest)
{
  *dest = (int) floorf(*source);
}

M64P_FPU_INLINE void round_l_d(double *source,long long *dest)
{
  *dest = (long long) round(*source);
}
M64P_FPU_INLINE void round_w_d(double *source,int *dest)
{
  *dest = (int) round(*source);
}
M64P_FPU_INLINE void trunc_l_d(double *source,long long *dest)
{
  *dest = (long long) trunc(*source);
}
M64P_FPU_INLINE void trunc_w_d(double *source,int *dest)
{
  *dest = (int) trunc(*source);
}
M64P_FPU_INLINE void ceil_l_d(double *source,long long *dest)
{
  *dest = (long long) ceil(*source);
}
M64P_FPU_INLINE void ceil_w_d(double *source,int *dest)
{
  *dest = (int) ceil(*source);
}
M64P_FPU_INLINE void floor_l_d(double *source,long long *dest)
{
  *dest = (long long) floor(*source);
}
M64P_FPU_INLINE void floor_w_d(double *source,int *dest)
{
  *dest = (int) floor(*source);
}

M64P_FPU_INLINE void cvt_w_s(float *source,int *dest)
{
  set_rounding();
  switch(FCR31&3)
  {
    case 0: round_w_s(source,dest);return;
    case 1: trunc_w_s(source,dest);return;
    case 2: ceil_w_s(source,dest);return;
    case 3: floor_w_s(source,dest);return;
  }
}
M64P_FPU_INLINE void cvt_w_d(double *source,int *dest)
{
  set_rounding();
  switch(FCR31&3)
  {
    case 0: round_w_d(source,dest);return;
    case 1: trunc_w_d(source,dest);return;
    case 2: ceil_w_d(source,dest);return;
    case 3: floor_w_d(source,dest);return;
  }
}
M64P_FPU_INLINE void cvt_l_s(float *source,long long *dest)
{
  set_rounding();
  switch(FCR31&3)
  {
    case 0: round_l_s(source,dest);return;
    case 1: trunc_l_s(source,dest);return;
    case 2: ceil_l_s(source,dest);return;
    case 3: floor_l_s(source,dest);return;
  }
}
M64P_FPU_INLINE void cvt_l_d(double *source,long long *dest)
{
  set_rounding();
  switch(FCR31&3)
  {
    case 0: round_l_d(source,dest);return;
    case 1: trunc_l_d(source,dest);return;
    case 2: ceil_l_d(source,dest);return;
    case 3: floor_l_d(source,dest);return;
  }
}

M64P_FPU_INLINE void c_f_s()
{
  FCR31 &= ~0x800000;
}
M64P_FPU_INLINE void c_un_s(float *source,float *target)
{
  FCR31=(isnan(*source) || isnan(*target)) ? FCR31|0x800000 : FCR31&~0x800000;
}
                          
M64P_FPU_INLINE void c_eq_s(float *source,float *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31&=~0x800000;return;}
  FCR31 = *source==*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_ueq_s(float *source,float *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31|=0x800000;return;}
  FCR31 = *source==*target ? FCR31|0x800000 : FCR31&~0x800000;
}

M64P_FPU_INLINE void c_olt_s(float *source,float *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31&=~0x800000;return;}
  FCR31 = *source<*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_ult_s(float *source,float *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31|=0x800000;return;}
  FCR31 = *source<*target ? FCR31|0x800000 : FCR31&~0x800000;
}

M64P_FPU_INLINE void c_ole_s(float *source,float *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31&=~0x800000;return;}
  FCR31 = *source<=*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_ule_s(float *source,float *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31|=0x800000;return;}
  FCR31 = *source<=*target ? FCR31|0x800000 : FCR31&~0x800000;
}

M64P_FPU_INLINE void c_sf_s(float *source,float *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31&=~0x800000;
}
M64P_FPU_INLINE void c_ngle_s(float *source,float *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31&=~0x800000;
}

M64P_FPU_INLINE void c_seq_s(float *source,float *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source==*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_ngl_s(float *source,float *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source==*target ? FCR31|0x800000 : FCR31&~0x800000;
}

M64P_FPU_INLINE void c_lt_s(float *source,float *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source<*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_nge_s(float *source,float *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source<*target ? FCR31|0x800000 : FCR31&~0x800000;
}

M64P_FPU_INLINE void c_le_s(float *source,float *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source<=*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_ngt_s(float *source,float *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source<=*target ? FCR31|0x800000 : FCR31&~0x800000;
}

M64P_FPU_INLINE void c_f_d()
{
  FCR31 &= ~0x800000;
}
M64P_FPU_INLINE void c_un_d(double *source,double *target)
{
  FCR31=(isnan(*source) || isnan(*target)) ? FCR31|0x800000 : FCR31&~0x800000;
}
                          
M64P_FPU_INLINE void c_eq_d(double *source,double *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31&=~0x800000;return;}
  FCR31 = *source==*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_ueq_d(double *source,double *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31|=0x800000;return;}
  FCR31 = *source==*target ? FCR31|0x800000 : FCR31&~0x800000;
}

M64P_FPU_INLINE void c_olt_d(double *source,double *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31&=~0x800000;return;}
  FCR31 = *source<*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_ult_d(double *source,double *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31|=0x800000;return;}
  FCR31 = *source<*target ? FCR31|0x800000 : FCR31&~0x800000;
}

M64P_FPU_INLINE void c_ole_d(double *source,double *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31&=~0x800000;return;}
  FCR31 = *source<=*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_ule_d(double *source,double *target)
{
  if (isnan(*source) || isnan(*target)) {FCR31|=0x800000;return;}
  FCR31 = *source<=*target ? FCR31|0x800000 : FCR31&~0x800000;
}

M64P_FPU_INLINE void c_sf_d(double *source,double *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31&=~0x800000;
}
M64P_FPU_INLINE void c_ngle_d(double *source,double *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31&=~0x800000;
}

M64P_FPU_INLINE void c_seq_d(double *source,double *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source==*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_ngl_d(double *source,double *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source==*target ? FCR31|0x800000 : FCR31&~0x800000;
}

M64P_FPU_INLINE void c_lt_d(double *source,double *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source<*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_nge_d(double *source,double *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source<*target ? FCR31|0x800000 : FCR31&~0x800000;
}

M64P_FPU_INLINE void c_le_d(double *source,double *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source<=*target ? FCR31|0x800000 : FCR31&~0x800000;
}
M64P_FPU_INLINE void c_ngt_d(double *source,double *target)
{
  //if (isnan(*source) || isnan(*target)) // FIXME - exception
  FCR31 = *source<=*target ? FCR31|0x800000 : FCR31&~0x800000;
}


M64P_FPU_INLINE void add_s(float *source1,float *source2,float *target)
{
  set_rounding();
  *target=(*source1)+(*source2);
}
M64P_FPU_INLINE void sub_s(float *source1,float *source2,float *target)
{
  set_rounding();
  *target=(*source1)-(*source2);
}
M64P_FPU_INLINE void mul_s(float *source1,float *source2,float *target)
{
  set_rounding();
  *target=(*source1)*(*source2);
}
M64P_FPU_INLINE void div_s(float *source1,float *source2,float *target)
{
  set_rounding();
  *target=(*source1)/(*source2);
}
M64P_FPU_INLINE void sqrt_s(float *source,float *target)
{
  set_rounding();
  *target=sqrtf(*source);
}
M64P_FPU_INLINE void abs_s(float *source,float *target)
{
  set_rounding();
  *target=fabsf(*source);
}
M64P_FPU_INLINE void mov_s(float *source,float *target)
{
  set_rounding();
  *target=*source;
}
M64P_FPU_INLINE void neg_s(float *source,float *target)
{
  set_rounding();
  *target=-(*source);
}
M64P_FPU_INLINE void add_d(double *source1,double *source2,double *target)
{
  set_rounding();
  *target=(*source1)+(*source2);
}
M64P_FPU_INLINE void sub_d(double *source1,double *source2,double *target)
{
  set_rounding();
  *target=(*source1)-(*source2);
}
M64P_FPU_INLINE void mul_d(double *source1,double *source2,double *target)
{
  set_rounding();
  *target=(*source1)*(*source2);
}
M64P_FPU_INLINE void div_d(double *source1,double *source2,double *target)
{
  set_rounding();
  *target=(*source1)/(*source2);
}
M64P_FPU_INLINE void sqrt_d(double *source,double *target)
{
  set_rounding();
  *target=sqrt(*source);
}
M64P_FPU_INLINE void abs_d(double *source,double *target)
{
  set_rounding();
  *target=fabs(*source);
}
M64P_FPU_INLINE void mov_d(double *source,double *target)
{
  set_rounding();
  *target=*source;
}
M64P_FPU_INLINE void neg_d(double *source,double *target)
{
  set_rounding();
  *target=-(*source);
}
