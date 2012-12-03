/*
Copyright (C) 2002 Rice1964

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

#ifndef _COMBINER_DEFS_H_
#define _COMBINER_DEFS_H_

#include "typedefs.h"

#define MUX_MASK            0x1F
#define MUX_MASK_WITH_ALPHA 0x5F
#define MUX_MASK_WITH_NEG   0x3F
#define MUX_MASK_WITH_COMP  0x9F
enum
{
    MUX_0 = 0,
    MUX_1,
    MUX_COMBINED,
    MUX_TEXEL0,
    MUX_TEXEL1,
    MUX_PRIM,
    MUX_SHADE,
    MUX_ENV,
    MUX_COMBALPHA,
    MUX_T0_ALPHA,
    MUX_T1_ALPHA,
    MUX_PRIM_ALPHA,
    MUX_SHADE_ALPHA,
    MUX_ENV_ALPHA,
    MUX_LODFRAC,
    MUX_PRIMLODFRAC,
    MUX_K5,
    MUX_UNK,            //Use this if you want to factor to be set to 0

    // Don't change value of these three flags, then need to be within 1 uint8
    MUX_NEG             = 0x20, //Support by NVidia register combiner
    MUX_ALPHAREPLICATE = 0x40,
    MUX_COMPLEMENT = 0x80,
    MUX_FORCE_0 = 0xFE,
    MUX_ERR = 0xFF,
};


enum CombinerFormatType
{
    CM_FMT_TYPE_NOT_USED,
    CM_FMT_TYPE_D,                  // = A          can mapped to SEL(arg1)
    CM_FMT_TYPE_A_MOD_C,            // = A*C        can mapped to MOD(arg1,arg2)
    CM_FMT_TYPE_A_ADD_D,            // = A+D        can mapped to ADD(arg1,arg2)
    CM_FMT_TYPE_A_SUB_B,            // = A-B        can mapped to SUB(arg1,arg2)
    CM_FMT_TYPE_A_MOD_C_ADD_D,      // = A*C+D      can mapped to MULTIPLYADD(arg1,arg2,arg0)
    CM_FMT_TYPE_A_LERP_B_C,         // = (A-B)*C+B  can mapped to LERP(arg1,arg2,arg0)
                                    //              or mapped to BLENDALPHA(arg1,arg2) if C is
                                    //              alpha channel or DIF, TEX, FAC, CUR
    CM_FMT_TYPE_A_SUB_B_ADD_D,      // = A-B+C      can not map very well in 1 stage
    CM_FMT_TYPE_A_SUB_B_MOD_C,      // = (A-B)*C    can not map very well in 1 stage
    CM_FMT_TYPE_A_ADD_B_MOD_C,      // = (A+B)*C    can not map very well in 1 stage
    CM_FMT_TYPE_A_B_C_D,            // = (A-B)*C+D  can not map very well in 1 stage
    CM_FMT_TYPE_A_B_C_A,            // = (A-B)*C+D  can not map very well in 1 stage

    // Don't use these two types in default functions
    CM_FMT_TYPE_AB_ADD_CD,          // = A*B+C*D    Use by nvidia video cards
    CM_FMT_TYPE_AB_SUB_CD,          // = A*B-C*D    Use by nvidia video cards
    CM_FMT_TYPE_AB_ADD_C,           // = A*B+C      Use by ATI video cards
    CM_FMT_TYPE_AB_SUB_C,           // = A*B-C      Use by ATI video cards
    CM_FMT_TYPE_NOT_CHECKED = 0xFF,
};


typedef enum {
    ENABLE_BOTH,
    DISABLE_ALPHA,
    DISABLE_COLOR,
    DISABLE_BOTH,
    COLOR_ONE,
    ALPHA_ONE,
} BlendingFunc;


typedef enum {
    COLOR_CHANNEL,
    ALPHA_CHANNEL,
} CombineChannel;



typedef struct {
    uint8 a;
    uint8 b;
    uint8 c;
    uint8 d;
} N64CombinerType;

#define CONST_FLAG4(a,b,c,d)    (a|(b<<8)|(c<<16)|(d<<24))  //(A-B)*C+D
#define CONST_MOD(a,b)          (a|(b<<16))             //A*B       
#define CONST_SEL(a)            (a<<24)                 //=D
#define CONST_ADD(a,b)          (a|b<<24)               //A+D
#define CONST_SUB(a,b)          (a|b<<8)                //A-B
#define CONST_MULADD(a,b,c)     (a|b<<16|c<<24)         //A*C+D

#define G_CCMUX_TEXEL1      2
#define G_ACMUX_TEXEL1      2

#define NOTUSED MUX_0

enum { TEX_0=0, TEX_1=1};



typedef struct {
    uint32 op;
    uint32 Arg1;
    uint32 Arg2;
    uint32 Arg0;
} StageOperate;

#endif



