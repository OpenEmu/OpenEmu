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

#include "GeneralCombiner.h"

//Attention
// If using CUR as an argument, use it as Arg2, not Arg1. I don't know why, 
// Geforce2 seems to be picky about this

// LERP and MULTIPLYADD are actually implemented in 2 stages in video chip
// they can only be used with SEL() before it, or use 1 stage only

// SEL(SPE) only is not good for alpha channel
// In fact, Specular color does not have alpha channel

// ADDSMOOTH does not work

// When using MOD with TEX and other, TEX must be the first argument, Arg1
// When MOD the DIF and FAC, using MOD(FAC,DIF) instead of MOD(DIF,FAC)

// Don't MOD(TEX,DIF) at Alpha channel, I don't know why this does not work
// probably there is not alpha blending for DIFFUSE at alpha channel

// Modifier COMPLEMENT and ALPHAREPLICATE only works as the first argument of the MOD operate
// Modifier ALPHAREPLICATE works
// Combined modifier of COMPLEMENT and ALPHAREPLICATE also works

#define MUX_T0  MUX_TEXEL0
#define MUX_T1  MUX_TEXEL1
#define MUX_DIF MUX_SHADE
#define MUX_COM MUX_COMBINED
#define MUX_CUR MUX_COMBINED
#define MUX_PRI MUX_PRIM

#define MUX_T0A     (MUX_TEXEL0|MUX_ALPHAREPLICATE)
#define MUX_T1A     (MUX_TEXEL1|MUX_ALPHAREPLICATE)
#define MUX_DIFA    (MUX_SHADE|MUX_ALPHAREPLICATE)
#define MUX_COMA    (MUX_COMBINED|MUX_ALPHAREPLICATE)
#define MUX_CURA    (MUX_COMBINED|MUX_ALPHAREPLICATE)
#define MUX_PRIA    (MUX_PRIM|MUX_ALPHAREPLICATE)
#define MUX_ENVA    (MUX_ENV|MUX_ALPHAREPLICATE)

#define MUX_T0C     (MUX_TEXEL0|MUX_COMPLEMENT)
#define MUX_T1C     (MUX_TEXEL1|MUX_COMPLEMENT)
#define MUX_DIFC    (MUX_SHADE|MUX_COMPLEMENT)
#define MUX_COMC    (MUX_COMBINED|MUX_COMPLEMENT)
#define MUX_CURC    (MUX_COMBINED|MUX_COMPLEMENT)
#define MUX_PRIC    (MUX_PRIM|MUX_COMPLEMENT)
#define MUX_ENVC    (MUX_ENV|MUX_COMPLEMENT)

#define MUX_T0AC    (MUX_TEXEL0|MUX_COMPLEMENT|MUX_ALPHAREPLICATE)
#define MUX_T1AC    (MUX_TEXEL1|MUX_COMPLEMENT|MUX_ALPHAREPLICATE)
#define MUX_DIFAC   (MUX_SHADE|MUX_COMPLEMENT|MUX_ALPHAREPLICATE)
#define MUX_COMAC   (MUX_COMBINED|MUX_COMPLEMENT|MUX_ALPHAREPLICATE)
#define MUX_CURAC   (MUX_COMBINED|MUX_COMPLEMENT|MUX_ALPHAREPLICATE)
#define MUX_PRIAC   (MUX_PRIM|MUX_COMPLEMENT|MUX_ALPHAREPLICATE)
#define MUX_ENVAC   (MUX_ENV|MUX_COMPLEMENT|MUX_ALPHAREPLICATE)

#define ONEARGS(op, arg1)   {CM_##op, MUX_##arg1}
#define TWOARGS(op, arg1,arg2)  {CM_##op, MUX_##arg1, MUX_##arg2}
#define TRIARGS(op, arg1,arg2,arg3) {CM_##op, MUX_##arg1, MUX_##arg2, MUX_##arg3}
#define SEL(arg1)       ONEARGS(REPLACE,arg1)
#define MOD(arg1,arg2)  TWOARGS(MODULATE,arg1,arg2)
#define ADD(arg1,arg2)  TWOARGS(ADD,arg1,arg2)
#define SUB(arg1,arg2)  TWOARGS(SUBTRACT,arg1,arg2)
#define ADDSMOOTH(arg1,arg2)    TWOARGS(ADDSMOOTH,arg1,arg2)
#define LERP(arg1,arg2,arg3)    TRIARGS(INTERPOLATE,arg1,arg2,arg3)
#define MULADD(arg1,arg2,arg3)  TRIARGS(MULTIPLYADD,arg1,arg2,arg3)
#define SKIP    SEL(CUR)

GeneralCombinerInfo twostages[]=
{
/*
Stage overflow
//Mux=0x00267e60350cf37f    Overflowed in THE LEGEND OF ZELDA
Color0: (TEXEL1 - PRIM) * ENV|A + TEXEL0
Color1: (PRIM - ENV) * COMBINED + ENV
Alpha0: (0 - 0) * 0 + TEXEL0
Alpha1: (COMBINED - 0) * PRIM + 0

//Simplied Mux=0x00267e60350cf37f   Overflowed in THE LEGEND OF ZELDA
Simplied DWORDs=03470604, 00060003, 07020706, 02000000
Color0: (TEXEL1 - SHADE) * ENV|A + TEXEL0
Color1: (SHADE - ENV) * COMBINED + ENV
Alpha0: (TEXEL0 - 0) * SHADE + 0
Alpha1: (0 - 0) * 0 + COMBINED
Simplfied type: CM_FMT_TYPE_NOT_CHECKED
Shade = PRIM in color channel
Shade = PRIM in alpha channel
*/

    {
        {0x03470604, 0x00060003, 0x07020706, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000},   // constant color texture flags
        {
            {SUB(T1,DIF), SKIP,     1, true},   // Stage 0
            {MULADD(CUR,ENVA,T0), MOD(T0,DIF),          0, true},   // Stage 1
        }
    },

        /*
        //Mux=0x002527ff1ffc9238    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - TEXEL0) * PRIM|A + TEXEL0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * PRIM + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x002527ff1ffc9238   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=03460304, 03060304, 02000000, 02000000
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1

        */


    {
        {0x03460304, 0x03060304, 0x02000000, 0x02000000},   // Simplified mux
            0x002527FF, 0x1FFC9238,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0, 0,   // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T1,PRIA), MOD(T1,PRI), 1, true},   // Stage 0
            {ADD(T0,CUR), ADD(T0,CUR), 0, true},    // Stage 1
        }
    },

    {
        {0x03460304, 0x03060304, 0x02000000, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            0,      // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T1,DIFA), MOD(T1,DIF), 1, true},   // Stage 0
            {ADD(T0,CUR), ADD(T0,CUR), 0, true},    // Stage 1
        }
    },



        /*
        //Mux=0x00262a60150c937f    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - TEXEL0) * ENV|A + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x00262a60150c937f   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=03460304, 03060304, 06020605, 00020005
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (PRIM - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = ENV in color channel
        Shade = ENV in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1

        */

    {
        {0x03460304, 0x03060304, 0x06020605, 0x00020005},   // Simplified mux
            0x00262A60, 0x150C937F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0, 0, 0,    // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T1,ENVA), MOD(T1,ENV), 1, true},   // Stage 0
            {ADD(T0,CUR), ADD(T0,CUR), 0, true},    // Stage 1
        }
    },

    {
        {0x03460304, 0x03060304, 0x06020605, 0x00020005},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            0,      // Constant color
            0x00000007, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T1,DIFA), MOD(T1,DIF), 1, true},   // Stage 0
            {ADD(T0,CUR), ADD(T0,CUR), 0, true},    // Stage 1
        }
    },


        /*
        //Mux=0x00267e041ffcfdf8    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - TEXEL0) * ENV|A + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00267e041ffcfdf8   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=03460304, 01000000, 00020006, 02000000
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = ENV in alpha channel

        Generated combiners:

        Stages:2, Alpha:DISABLE_ALPHA, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1
        */

    {
        {0x03460304, 0x01000000, 0x00020006, 0x02000000},   // Simplified mux
            0x00267e04, 0x1ffcfdf8,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            //{MOD(T1,DIFA), SKIP, 1, true},    // Stage 0
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            //{MULADD(T0,CUR,DIF), SKIP, 0, true},  // Stage 1
            {LERP(T1,CUR,ENVA), SKIP, 1, true}, // Stage 1
        }
    },


        /*
        //Mux=0x00267e041f0cfdff    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - TEXEL0) * ENV|A + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x00267e041f0cfdff   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=03470304, 06000000, 00020006, 02000000
        Color0: (TEXEL1 - TEXEL0) * ENV|A + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlFacA - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Sel - SHADE, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1



        */

    {
        {0x03470304, 0x06000000, 0x00020006, 0x02000000},   // Simplified mux
            0x00267E04, 0x1F0CFDFF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T1,ENVA), SEL(DIFA), 1, true}, // Stage 0
            {MULADD(T0,DIF,CUR), SKIP, 0, true},    // Stage 1
        }
    },



        /*
        //Mux=0x00117ffffffdfc38    Overflowed in MarioTennis
        Color0: (TEXEL0 - 0) * TEXEL1 + PRIM
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x00117ffffffdfc38   Overflowed in MarioTennis
        Simplied DWORDs=00030004, 01000000, 02010006, 02000000
        Color0: (TEXEL1 - 0) * TEXEL0 + 0
        Color1: (SHADE - 0) * 1 + COMBINED
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE5_A_MOD_C_ADD_D
        Shade = PRIM in color channel

        Generated combiners:

        Stages:2, Alpha:DISABLE_ALPHA, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL1, , 
        1:Color: Mod - TEXEL0, COMBINED, 
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Sel - COMBINED, , 


        */

    {
        {0x00030004, 0x01000000, 0x02010006, 0x02000000},   // Simplified mux
            0x00117FFF, 0xFFFDFC38,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            0,      // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SEL(T0), SKIP, 0, true},   // Stage 0
            {MULADD(T1,CUR,DIF), SKIP, 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x00ffa1ffff0d923f    Overflowed in MarioTennis
        Color0: (0 - 0) * 0 + PRIM
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0

        //Simplied Mux=0x00ffa1ffff0d923f   Overflowed in MarioTennis
        Simplied DWORDs=05000000, 03060304, 02000000, 00020005
        Color0: (0 - 0) * 0 + PRIM
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (PRIM - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - PRIM, , 
        1:Color: Sel - COMBINED, ,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x05000000, 0x03060304, 0x02000000, 0x00020005},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            0,      // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SEL(PRI), MOD(T0,PRIM), 0, true},  // Stage 0
            {SKIP, TRIARGS(BLENDDIFFUSEALPHA,T1,CUR,DIFA), 1, true},    // Stage 1
        }
    },


        /*
        //Mux=0x00ffb9ffffebdbc0    Used in MarioTennis
        Color0: (0 - 0) * 0 + 0
        Color1: (0 - 0) * 0 + 0
        Alpha0: (PRIM - ENV) * SHADE + ENV
        Alpha1: (0 - COMBINED) * TEXEL1 + COMBINED

        //Simplied Mux=0x00ffb9ffffebdbc0   Used in MarioTennis
        Simplied DWORDs=00000000, 00060083, 02000000, 02000000
        Color0: (0 - 0) * 0 + 0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL0|C - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE2_A_ADD_D
        Shade = 07060705 in alpha channel
        Generated combiners:

        */


    {
        {0x00000000, 0x00060083, 0x02000000, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            DISABLE_COLOR,
            0,      // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SKIP, MOD(T0,PRIM), 0, true},  // Stage 0
            {SKIP, TRIARGS(BLENDDIFFUSEALPHA,T0,CUR,DIFA), 0, true},    // Stage 1
        }
    },

        /*
        //Mux=0x0030b2045ffefff8    Used in THE LEGEND OF ZELDA
        Color0: (PRIM - ENV) * TEXEL0 + ENV
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (PRIM - 0) * TEXEL0 + 0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x0030b2045ffefff8   Used in THE LEGEND OF ZELDA
        Simplied DWORDs=07030704, 04000000, 00020006, 00020003
        Color0: (TEXEL1 - ENV) * TEXEL0 + ENV
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (TEXEL0 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Tex 1 = PRIM



        Stages:2, Alpha:ENABLE_BOTH, Factor:MUX_ENV, Specular:MUX_0 Dif Color:0x0 Dif Alpha:0x0
        0:Color: SELECTARG1 - TEXTURE, _, _
        1:Color: LERP - TEXTURE, TFACTOR, CURRENT -Tex1
        0:Alpha: SELECTARG1 - CURRENT, _, _
        1:Alpha: SELECTARG1 - TEXTURE, _, _ -Tex1
        */

    {
        {0x07030704, 0x04000000, 0x00020006, 0x00020003},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            DISABLE_COLOR,
            0,      // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {LERP(T1,ENV,CUR), MOD(T1,CUR), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x0026a0041f1093ff    Overflowed in Perfect Dark
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0

        //Simplied Mux=0x0026a0041f1093ff   Overflowed in Perfect Dark
        Simplied DWORDs=030E0304, 03060304, 00020006, 00020006
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, LODFRAC -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x030E0304, 0x03060304, 0x00020006, 0x00020006},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            0,      // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            //{LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},    // Stage 1
            {SKIP, SKIP, 0, false}, // Stage 1
        }
    },


        /*
        //Mux=0x0026a0041ffc93fc    Overflowed in Perfect Dark
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (0 - 0) * 0 + SHADE

        //Simplied Mux=0x0026a0041ffc93fc   Overflowed in Perfect Dark
        Simplied DWORDs=030E0304, 06000000, 00020006, 02000000
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, LODFRAC -Tex1
        0:Alpha: Sel - SHADE, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x030E0304, 0x06000000, 0x00020006, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            0,      // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(DIF), 0, true},   // Stage 0
            //{LERP(T1,CUR,LODFRAC), SKIP, 1, true},    // Stage 1
            {SKIP, SKIP, 0, false}, // Stage 1
        }
    },


        /*
        //Mux=0x002526041f1093ff    Overflowed in Perfect Dark
        Color0: (TEXEL1 - TEXEL0) * PRIM|A + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * PRIM + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0

        //Simplied Mux=0x002526041f1093ff   Overflowed in Perfect Dark
        Simplied DWORDs=03450304, 03050304, 00020006, 00020006
        Color0: (TEXEL1 - TEXEL0) * PRIM|A + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * PRIM + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlFacA - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlFacA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x03450304, 0x03050304, 0x00020006, 0x00020006},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(DIF), 0, true},   // Stage 0
            {TRIARGS(BLENDDIFFUSEALPHA,T1,CUR,DIFA), SKIP, 1, true},    // Stage 1
        }
    },


        /*
        //Mux=0x0026a0041f1093fb    Overflowed in Perfect Dark
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + PRIM

        //Simplied Mux=0x0026a0041f1093fb   Overflowed in Perfect Dark
        Simplied DWORDs=030E0304, 03060304, 00020006, 05020006
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + PRIM
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, LODFRAC -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x030E0304, 0x03060304, 0x00020006, 0x05020006},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {SKIP, ADD(CUR,PRI), 0, false}, // Stage 1
        }
    },



        /*
        //Mux=0x00272c041f1093ff    Overflowed in GOLDENEYE
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0

        //Simplied Mux=0x00272c041f1093ff   Overflowed in GOLDENEYE
        Simplied DWORDs=030F0304, 00060004, 00020006, 02000000
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, PRIMLODFRAC -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Mod - TEXEL1, SHADE,  -Tex1


        */


    {
        {0x030F0304, 0x00060004, 0x00020006, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {SKIP, MOD(T1,DIF), 1, true},   // Stage 1
        }
    },


        /*
        //Mux=0x0026a0041f1493ff    Overflowed in GOLDENEYE
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x0026a0041f1493ff   Overflowed in GOLDENEYE
        Simplied DWORDs=00060003, 03060304, 02000000, 00020007
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (ENV - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Mod - TEXEL0, SHADE, 
        1:Color: Sel - COMBINED, ,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x00060003, 0x03060304, 0x02000000, 0x00020007},   // Simplified mux
            0x0026A004, 0x1F1493FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {SKIP, SKIP, 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x0030fe045ffefdf8    Overflowed in Kirby64
        Color0: (PRIM - ENV) * TEXEL0 + ENV
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x0030fe045ffefdf8   Overflowed in Kirby64
        Simplied DWORDs=07030704, 01000000, 00020006, 02000000
        Color0: (TEXEL1 - ENV) * TEXEL0 + ENV
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:DISABLE_ALPHA, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, ENV, COMBINED -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x07030704, 0x01000000, 0x00020006, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {LERP(T1,ENV,CUR), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x00309e045ffefdf8    Overflowed in Kirby64
        Color0: (PRIM - ENV) * TEXEL0 + ENV
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x00309e045ffefdf8   Overflowed in Kirby64
        Simplied DWORDs=07030704, 01000000, 00020006, 02000000
        Color0: (TEXEL1 - ENV) * TEXEL0 + ENV
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:DISABLE_ALPHA, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, ENV, COMBINED -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x07030704, 0x01000000, 0x00020006, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {LERP(T1,ENV,CUR), SKIP, 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x0026a0041ffc93f8    Overflowed in ZELDA MAJORA'S MASK
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x0026a0041ffc93f8   Overflowed in ZELDA MAJORA'S MASK
        Simplied DWORDs=030E0304, 03060304, 00020006, 02000000
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, LODFRAC -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x030E0304, 0x03060304, 0x00020006, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {SKIP, LERP(T1,CUR,LODFRAC), 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x00209c03ff0f93ff    Overflowed in ZELDA MAJORA'S MASK
        Color0: (TEXEL1 - 0) * TEXEL0 + 0
        Color1: (COMBINED - 0) * PRIM + 0
        Alpha0: (TEXEL0 - TEXEL0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0

        //Simplied Mux=0x00209c03ff0f93ff   Overflowed in ZELDA MAJORA'S MASK
        Simplied DWORDs=00050004, 00050003, 00020003, 02000000
        Color0: (TEXEL1 - 0) * PRIM + 0
        Color1: (TEXEL0 - 0) * COMBINED + 0
        Alpha0: (TEXEL0 - 0) * PRIM + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE2_A_ADD_D

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Mod - TEXEL1, PRIM,  -Tex1
        1:Color: Mod - TEXEL0, COMBINED, 
        0:Alpha: Sel - COMBINED, ,  -Tex1
        1:Alpha: Mod - TEXEL0, PRIM, 


        */


    {
        {0x00050004, 0x00050003, 0x00020003, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,PRI), MOD(T0,PRI), 0, true},    // Stage 0
            {MOD(T1,CUR), SKIP, 1, true},   // Stage 1
        }
    },


        /*
        //Mux=0x001229ffff17fe3f    Overflowed in Rayman 2
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x001229ffff17fe3f   Overflowed in Rayman 2
        Simplied DWORDs=00060003, 00060004, 02000000, 00020007
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (ENV - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE2_A_ADD_D

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Mod - TEXEL0, SHADE, 
        1:Color: Sel - COMBINED, ,  -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Mod - TEXEL1, SHADE,  -Tex1


        */


    {
        {0x00060003, 0x00060004, 0x02000000, 0x00020007},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(DIF,ENV), 0, true},   // Stage 0
            {SKIP, MOD(T1,CUR), 1, true},   // Stage 1
        }
    },


        /*
        //Mux=0x0030fe0254feff3e    Overflowed in Beetle Adventure Rac
        Color0: (PRIM - ENV) * TEXEL0 + ENV
        Color1: (COMBINED - SHADE) * TEXEL1 + SHADE
        Alpha0: (0 - 0) * 0 + 0
        Alpha1: (0 - 0) * 0 + 1

        //Simplied Mux=0x0030fe0254feff3e   Overflowed in Beetle Adventure Rac
        Simplied DWORDs=07030704, 01000000, 06030602, 02000000
        Color0: (TEXEL1 - ENV) * TEXEL0 + ENV
        Color1: (COMBINED - SHADE) * TEXEL0 + SHADE
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:DISABLE_ALPHA, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, ENV, COMBINED -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x07030704, 0x01000000, 0x06030602, 0x02000000},   // Simplified mux
            0x0030FE02, 0x54FEFF3E,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(DIFA,ENV,T0), SKIP, 0, true}, // Stage 0
            {LERP(CUR,DIF,T1), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x0015fe042ffd79fc    Overflowed in Beetle Adventure Rac
        Color0: (TEXEL0 - TEXEL1) * SHADE|A + TEXEL1
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + SHADE

        //Simplied Mux=0x0015fe042ffd79fc   Overflowed in Beetle Adventure Rac
        Simplied DWORDs=04460403, 06000000, 00020006, 02000000
        Color0: (TEXEL0 - TEXEL1) * SHADE|A + TEXEL1
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - COMBINED, TEXEL1,  -Tex1
        0:Alpha: Sel - SHADE, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x04460403, 0x06000000, 0x00020006, 0x02000000},   // Simplified mux
            0x0015FE04, 0x2FFD79FC,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            0,      // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIFA), SEL(DIF), 0, true},  // Stage 0
            {LERP(CUR,T1,DIF), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x0020fe0a14fcf938    Overflowed in Beetle Adventure Rac
        Color0: (TEXEL1 - TEXEL0) * TEXEL0 + TEXEL0
        Color1: (COMBINED - SHADE) * PRIM|A + SHADE
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x0020fe0a14fcf938   Overflowed in Beetle Adventure Rac
        Simplied DWORDs=03030304, 06000000, 06450602, 02000000
        Color0: (TEXEL1 - TEXEL0) * TEXEL0 + TEXEL0
        Color1: (COMBINED - SHADE) * PRIM|A + SHADE
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, COMBINED -Tex1
        0:Alpha: Sel - SHADE, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x03030304, 0x06000000, 0x06450602, 0x02000000},   // Simplified mux
            0x0020FE0A, 0x14FCF938,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SEL(T0), SEL(DIF), 0, true},   // Stage 0
            {LERP(CUR,DIF,PRIA), SKIP, 0, true},    // Stage 1
        }
    },

        /*
        //Mux=0x0017fe042ffd73f8    Overflowed in Beetle Adventure Rac
        Color0: (TEXEL0 - TEXEL1) * UNK + TEXEL1
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x0017fe042ffd73f8   Overflowed in Beetle Adventure Rac
        Simplied DWORDs=04100403, 03000000, 00020006, 02000000
        Color0: (TEXEL0 - TEXEL1) * UNK + TEXEL1
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - COMBINED, TEXEL1, UNK -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x04100403, 0x03000000, 0x00020006, 0x02000000},   // Simplified mux
            0x0017FE04, 0x2FFD73F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            0,      // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SEL(T0), SEL(T0), 0, true},    // Stage 0
            {LERP(CUR,T1,DIF), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x001218245531feff    Overflowed in CONKER BFD
        Color0: (TEXEL0 - ENV) * SHADE + PRIM
        Color1: (TEXEL0 - ENV) * SHADE + PRIM
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (TEXEL0 - 0) * SHADE + 0

        //Simplied Mux=0x001218245531feff   Overflowed in CONKER BFD
        Simplied DWORDs=00060703, 00060003, 02010004, 02000000
        Color0: (TEXEL0 - ENV) * SHADE + 0
        Color1: (TEXEL1 - 0) * 1 + COMBINED
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sub - TEXEL0, ENV, 
        1:Color: Mod - COMBINED, SHADE, 
        0:Alpha: Mod - TEXEL0, SHADE, 
        1:Alpha: Sel - COMBINED, , 


        */


    {
        {0x00060703, 0x00060003, 0x02010004, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {SUB(T0,ENV), MOD(T0,DIF), 0, true},    // Stage 0
            {MULADD(CUR,DIF,T1), SKIP, 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x00127e2455fdf2f9    Overflowed in CONKER BFD
        Color0: (TEXEL0 - ENV) * SHADE + PRIM
        Color1: (TEXEL0 - ENV) * SHADE + PRIM
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + TEXEL0

        //Simplied Mux=0x00127e2455fdf2f9   Overflowed in CONKER BFD
        Simplied DWORDs=00060703, 03000000, 02010004, 02000000
        Color0: (TEXEL0 - ENV) * SHADE + 0
        Color1: (TEXEL1 - 0) * 1 + COMBINED
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sub - TEXEL0, ENV, 
        1:Color: Mod - COMBINED, SHADE, 
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: Sel - COMBINED, , 


        */


    {
        {0x00060703, 0x03000000, 0x02010004, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {SUB(T0,ENV), SEL(T0), 0, true},    // Stage 0
            {MULADD(CUR,DIF,T1), SKIP, 1, true},    // Stage 1
        }
    },


        /*
        //Mux=0x0026a004151092ff    Overflowed in CONKER BFD
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - ENV) * SHADE + PRIM
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0

        //Simplied Mux=0x0026a004151092ff   Overflowed in CONKER BFD
        Simplied DWORDs=00060703, 03060304, 02010005, 00020006
        Color0: (TEXEL0 - ENV) * SHADE + 0
        Color1: (PRIM - 0) * 1 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sub - TEXEL0, ENV, 
        1:Color: Mod - COMBINED, SHADE,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x00060703, 0x03060304, 0x02010005, 0x00020006},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            //{LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},    // Stage 1
            {ADD(CUR,PRI), SKIP, 0, false}, // Stage 1
        }
    },

        /*
        //Mux=0x0026a00415fc92f8    Overflowed in CONKER BFD
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - ENV) * SHADE + PRIM
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x0026a00415fc92f8   Overflowed in CONKER BFD
        Simplied DWORDs=00060703, 03060304, 02010005, 02000000
        Color0: (TEXEL0 - ENV) * SHADE + 0
        Color1: (PRIM - 0) * 1 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sub - TEXEL0, ENV, 
        1:Color: Mod - COMBINED, SHADE,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x00060703, 0x03060304, 0x02010005, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {SKIP, LERP(T1,CUR,LODFRAC), 1, true},  // Stage 1
        }

    },


        /*
        //Mux=0x001219ff5f15fe3f    Overflowed in CONKER BFD
        Color0: (TEXEL0 - ENV) * SHADE + PRIM
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x001219ff5f15fe3f   Overflowed in CONKER BFD
        Simplied DWORDs=00060703, 00060003, 02010004, 00020007
        Color0: (TEXEL0 - ENV) * SHADE + 0
        Color1: (TEXEL1 - 0) * 1 + COMBINED
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (ENV - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sub - TEXEL0, ENV, 
        1:Color: Mod - COMBINED, SHADE, 
        0:Alpha: Mod - TEXEL0, SHADE, 
        1:Alpha: Mod - ENV, COMBINED, 


        */


    {
        {0x00060703, 0x00060003, 0x02010004, 0x00020007},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {SUB(T0,ENV), MOD(T0,DIF), 0, true},    // Stage 0
            {MULADD(CUR,DIF,T1), MOD(CUR,ENV), 1, true},    // Stage 1
        }
    },


        /*
        //Mux=0x00ff9880f514feff    Overflowed in CONKER BFD
        Color0: (0 - 0) * 0 + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + PRIM
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x00ff9880f514feff   Overflowed in CONKER BFD
        Simplied DWORDs=00030706, 00060003, 02010004, 00020007
        Color0: (SHADE - ENV) * TEXEL0 + 0
        Color1: (TEXEL1 - 0) * 1 + COMBINED
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (ENV - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED
        Shade = 00000706 in color channel
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sub - SHADE, ENV, 
        1:Color: Mod - COMBINED, TEXEL0, 
        0:Alpha: Mod - TEXEL0, SHADE, 
        1:Alpha: Mod - ENV, COMBINED, 


        */


    {
        {0x00030706, 0x00060003, 0x02010004, 0x00020007},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {SUB(T0,ENV), MOD(T0,DIF), 0, true},    // Stage 0
            {MULADD(CUR,DIF,T1), MOD(CUR,ENV), 1, true},    // Stage 1
        }
    },




        /*
        //Mux=0x0026a080151492ff    Overflowed in CONKER BFD
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + PRIM
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x0026a080151492ff   Overflowed in CONKER BFD
        Simplied DWORDs=030E0304, 03060304, 05020706, 00020007
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + PRIM
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (ENV - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, LODFRAC -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x030E0304, 0x03060304, 0x05020706, 0x00020007},   // Simplified mux
            0x0026A080, 0x151492FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000706, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            //{LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},    // Stage 1
            {ADD(CUR,PRI), SKIP, 0, false}, // Stage 1
        }
    },

        /*
        //Mux=0x0026a004151092ff    Overflowed in CONKER BFD
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - ENV) * SHADE + PRIM
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x0026a004151092ff   Overflowed in CONKER BFD
        Simplied DWORDs=030E0304, 03060304, 05060702, 00020006
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - ENV) * SHADE + PRIM
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */


    {
        {0x030E0304, 0x03060304, 0x05060702, 0x00020006},   // Simplified mux
            0x0026A004, 0x151092FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            //{LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},    // Stage 1
            {SKIP, LERP(T1,CUR,LODFRAC), 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x00ff9880f514feff    Overflowed in CONKER BFD
        Color0: (0 - 0) * 0 + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + PRIM
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x00ff9880f514feff   Overflowed in CONKER BFD
        Simplied DWORDs=00030706, 00060003, 02010004, 00020007
        Color0: (SHADE - ENV) * TEXEL0 + 0
        Color1: (TEXEL1 - 0) * 1 + COMBINED
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (ENV - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED
        Shade = 00000706 in color channel
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sub - SHADE, ENV, 
        1:Color: Mod - COMBINED, TEXEL0, 
        0:Alpha: Mod - TEXEL0, SHADE, 
        1:Alpha: Mod - ENV, COMBINED, 


        */


    {
        {0x00030706, 0x00060003, 0x02010004, 0x00020007}, // Simplified mux
            0x00FF9880, 0xF514FEFF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000706, 0x00070006, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {ADD(CUR,PRI), SKIP, 0, false}, // Stage 1
        }
    },

        /*
        //Mux=0x00262a041f0c93ff    Overflowed in JET FORCE GEMINI
        Color0: (TEXEL1 - TEXEL0) * ENV|A + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0

        //Simplied Mux=0x00262a041f0c93ff   Overflowed in JET FORCE GEMINI
        Simplied DWORDs=03460304, 03060304, 00020006, 00020005
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (PRIM - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = ENV in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x03460304, 0x03060304, 0x00020006, 0x00020005},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,ENVA), LERP(T1,CUR,ENV), 1, true}, // Stage 1
        }
    },

        /*
        //Mux=0x00262a6014fc9338    Overflowed in JET FORCE GEMINI
        Color0: (TEXEL1 - TEXEL0) * ENV|A + TEXEL0
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x00262a6014fc9338   Overflowed in JET FORCE GEMINI
        Simplied DWORDs=03460304, 03060304, 06020605, 02000000
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = ENV in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x03460304, 0x03060304, 0x06020605, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {LERP(T1,CUR,ENVA), LERP(T1,CUR,ENV), 1, true}, // Stage 1
        }
    },


        /*
        //Mux=0x00127e2455fdf8fc    Overflowed in KILLER INSTINCT GOLD
        Color0: (TEXEL0 - ENV) * SHADE + PRIM
        Color1: (TEXEL0 - ENV) * SHADE + PRIM
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + SHADE

        //Simplied Mux=0x00127e2455fdf8fc   Overflowed in KILLER INSTINCT GOLD
        Simplied DWORDs=00060703, 06000000, 02010004, 02000000
        Color0: (TEXEL0 - ENV) * SHADE + 0
        Color1: (TEXEL1 - 0) * 1 + COMBINED
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sub - TEXEL0, ENV, 
        1:Color: Mod - COMBINED, SHADE, 
        0:Alpha: Sel - SHADE, , 
        1:Alpha: Sel - COMBINED, , 


        */


    {
        {0x00060703, 0x06000000, 0x02010004, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {SUB(T0,ENV), SEL(DIF), 0, true},   // Stage 0
            {MULADD(CUR,DIF,T1), SKIP, 1, true},    // Stage 1
        }
    },


        /*
        //Mux=0x00fffe6af5fcf438    Overflowed in KILLER INSTINCT GOLD
        Color0: (0 - 0) * 0 + TEXEL0
        Color1: (PRIM - ENV) * PRIM|A + COMBINED
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x00fffe6af5fcf438   Overflowed in KILLER INSTINCT GOLD
        Simplied DWORDs=00460706, 04000000, 02010003, 02000000
        Color0: (SHADE - ENV) * SHADE|A + 0
        Color1: (TEXEL0 - 0) * 1 + COMBINED
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED
        Shade = PRIM in color channel
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sub - SHADE, ENV,  -Tex1
        1:Color: Mod - COMBINED, SHADE|A, 
        0:Alpha: Sel - TEXEL1, ,  -Tex1
        1:Alpha: Sel - COMBINED, , 


        */


    {
        {0x00460706, 0x04000000, 0x02010003, 0x02000000},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SUB(DIF,ENV), SEL(T1), 1, true},   // Stage 0
            {MULADD(CUR,DIFA,T0), SKIP, 0, true},   // Stage 1
        }
    },


        /*
        //Mux=0x00262a041f5893f8    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - TEXEL0) * ENV|A + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (TEXEL1 - 0) * 1 + COMBINED

        //Simplied Mux=0x00262a041f5893f8   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=03460304, 03060304, 00020006, 02010004
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (TEXEL1 - 0) * 1 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = ENV in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x03460304, 0x03060304, 0x00020006, 0x02010004},   // Simplified mux
            0x00262A04, 0x1F5893F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {LERP(T1,CUR,ENVA), LERP(T1,CUR,ENV), 1, true}, // Stage 1
        }
    },


        /*
        //Mux=0x00272c60350ce37f    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - 1) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0

        //Simplied Mux=0x00272c60350ce37f   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=030F0604, 00060003, 07020706, 02000000
        Color0: (TEXEL1 - SHADE) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED
        Shade = PRIM in color channel
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Mod - TEXEL1, PRIMLODFRAC,  -Tex1
        1:Color: Add - COMBINED, TEXEL0, 
        0:Alpha: Sel - COMBINED, ,  -Tex1
        1:Alpha: Mod - TEXEL0, SHADE, 


        */


    {
        {0x030F0604, 0x00060003, 0x07020706, 0x02000000},   // Simplified mux
            0x00272C60, 0x350CE37F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T1,DIF), MOD(T1,DIF), 1, true},    // Stage 0
            {MULADD(DIF,T0,CUR), MOD(T0,CUR), 0, true}, // Stage 1
        }
    },



        /*
        //Mux=0x0026a0041f1093ff    Overflowed in Perfect Dark
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0

        //Simplied Mux=0x0026a0041f1093ff   Overflowed in Perfect Dark
        Simplied DWORDs=00060003, 03060304, 02000000, 00020006
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Mod - TEXEL0, SHADE, 
        1:Color: Sel - COMBINED, ,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x00060003, 0x03060304, 0x02000000, 0x00020006}, // Simplified mux
            0x0026A004, 0x1F1093FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {SKIP, SKIP, 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x0026a08015fc937b    Overflowed in ROCKETROBOTONWHEELS
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (0 - 0) * 0 + PRIM

        //Simplied Mux=0x0026a08015fc937b   Overflowed in ROCKETROBOTONWHEELS
        Simplied DWORDs=030E0304, 06000000, 07020706, 02000000
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, LODFRAC -Tex1
        0:Alpha: Sel - SHADE, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x030E0304, 0x06000000, 0x07020706, 0x02000000}, // Simplified mux
            0x0026A080, 0x15FC937B,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x0026a0801ffc93fb    Overflowed in ROCKETROBOTONWHEELS
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (0 - 0) * 0 + PRIM

        //Simplied Mux=0x0026a0801ffc93fb   Overflowed in ROCKETROBOTONWHEELS
        Simplied DWORDs=030E0304, 06000000, 00020006, 02000000
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, LODFRAC -Tex1
        0:Alpha: Sel - SHADE, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x030E0304, 0x06000000, 0x00020006, 0x02000000}, // Simplified mux
            0x0026A080, 0x1FFC93FB,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x0025a8801f1493ff    Overflowed in ROCKETROBOTONWHEELS
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x0025a8801f1493ff   Overflowed in ROCKETROBOTONWHEELS
        Simplied DWORDs=03460304, 03060304, 00020006, 00020007
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (ENV - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x03460304, 0x03060304, 0x00020006, 0x00020007},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,ENV), 0, true},    // Stage 0
            {LERP(T1,CUR,DIFA), LERP(T1,CUR,DIF), 1, true}, // Stage 1
        }
    },


        /*
        //Mux=0x0026a0801510937f    Overflowed in ROCKETROBOTONWHEELS
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0

        //Simplied Mux=0x0026a0801510937f   Overflowed in ROCKETROBOTONWHEELS
        Simplied DWORDs=030E0304, 03060304, 07020706, 00020006
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, LODFRAC -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x030E0304, 0x03060304, 0x07020706, 0x00020006},   // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x004099ff5f0efe3f    Overflowed in ROCKETROBOTONWHEELS
        Color0: (SHADE - ENV) * TEXEL0 + ENV
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (COMBINED - 0) * PRIM + 0

        //Simplied Mux=0x004099ff5f0efe3f   Overflowed in ROCKETROBOTONWHEELS
        Simplied DWORDs=07030706, 00060003, 02000000, 00020004
        Color0: (SHADE - ENV) * TEXEL0 + ENV
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (TEXEL1 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = 00000706 in color channel
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Lerp - SHADE, ENV, TEXEL0
        1:Color: Sel - COMBINED, ,  -Tex1
        0:Alpha: Mod - TEXEL0, SHADE, 
        1:Alpha: Mod - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x07030706, 0x00060003, 0x02000000, 0x00020004}, // Simplified mux
            0x004099FF, 0x5F0EFE3F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {LERP(DIF,ENV,T0), MOD(T0,DIF), 0, true},   // Stage 0
            {SKIP, MOD(T1,CUR), 1, true},   // Stage 1
        }
    },


        /*
        //Mux=0x0025a8a01414933f    Overflowed in ROCKETROBOTONWHEELS
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (ENV - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x0025a8a01414933f   Overflowed in ROCKETROBOTONWHEELS
        Simplied DWORDs=03460304, 03060304, 06020607, 00020007
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (ENV - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (ENV - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x03460304, 0x03060304, 0x06020607, 0x00020007},   // Simplified mux
            0x0025A8A0, 0x1414933F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {LERP(ENV,DIF,T0), MOD(T0,ENV), 0, true},   // Stage 0
            {LERP(T1,CUR,DIFA), LERP(T1,CUR,DIF), 1, true}, // Stage 1
        }
    },


        /*
        //Mux=0x001298043f15ffff    Overflowed in BANJO TOOIE
        Color0: (TEXEL0 - PRIM) * ENV + PRIM
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x001298043f15ffff   Overflowed in BANJO TOOIE
        Simplied DWORDs=04070403, 00060003, 00020006, 00020007
        Color0: (TEXEL0 - TEXEL1) * ENV + TEXEL1
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (ENV - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - COMBINED, TEXEL1, ENV
        0:Alpha: Mod - TEXEL0, SHADE, 
        1:Alpha: Mod - ENV, COMBINED, 


        */


    {
        {0x04070403, 0x00060003, 0x00020006, 0x00020007}, // Simplified mux
            0x00129804, 0x3F15FFFF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(CUR,T1,ENV), MOD(CUR,ENV), 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x0062fe043f15f9ff    Overflowed in BANJO TOOIE
        Color0: (1 - PRIM) * ENV + PRIM
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x0062fe043f15f9ff   Overflowed in BANJO TOOIE
        Simplied DWORDs=03070301, 06000000, 00020006, 02000000
        Color0: (1 - TEXEL0) * ENV + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = 00070006 in alpha channel
        Tex 0 = PRIM

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: AddSmooth - TEXEL0, ENV, 
        1:Color: Mod - SHADE, COMBINED, 
        0:Alpha: Sel - SHADE, , 
        1:Alpha: Sel - COMBINED, , 


        */


    {
        {0x03070301, 0x06000000, 0x00020006, 0x02000000}, // Simplified mux
            0x0062FE04, 0x3F15F9FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000005, 0x00000000}, // constant color texture flags
        {
            {MULADD(T0C,ENV,T0), SEL(DIF), 0, true},    // Stage 0
            {MOD(CUR,DIF), SKIP, 0, true},  // Stage 1
        }
    },


        /*
        //Mux=0x0025266015fc9378    Overflowed in ZELDA MAJORA'S MASK
        Color0: (TEXEL1 - TEXEL0) * PRIM|A + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - TEXEL0) * PRIM + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x0025266015fc9378   Overflowed in ZELDA MAJORA'S MASK
        Simplied DWORDs=03460304, 03060304, 06020605, 02000000
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = ENV in color channel
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x03460304, 0x03060304, 0x06020605, 0x02000000}, // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000007, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(PRI,DIF,T0), SEL(T0), 0, true},   // Stage 0
            {LERP(T1,CUR,DIFA), SKIP, 1, true}, // Stage 1
        }
    },


        /*
        //Mux=0x0061a5ff1f10d23f    Overflowed in PAPER MARIO
        Color0: (1 - TEXEL0) * PRIM + TEXEL0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - ENV) * TEXEL1 + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0

        //Simplied Mux=0x0061a5ff1f10d23f   Overflowed in PAPER MARIO
        Simplied DWORDs=03060301, 03040704, 02000000, 00020006
        Color0: (1 - TEXEL0) * SHADE + TEXEL0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - ENV) * TEXEL1 + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE_NOT_CHECKED
        Shade = PRIM in color channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: AddSmooth - TEXEL0, SHADE, 
        1:Color: Sel - COMBINED, ,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: Mod - COMBINED, TEXEL1,  -Tex1


        */


    {
        {0x03060301, 0x03040704, 0x02000000, 0x00020006}, // Simplified mux
            0x0061A5FF, 0x1F10D23F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MULADD(T0C,DIF,T0), MOD(T0,DIF), 0, true}, // Stage 0
            {SKIP, LERP(T1,ENV,CUR), 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x00322bff5f0e923f    Overflowed in PAPER MARIO
        Color0: (PRIM - ENV) * SHADE + ENV
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0

        //Simplied Mux=0x00322bff5f0e923f   Overflowed in PAPER MARIO
        Simplied DWORDs=06000000, 03070304, 02000000, 00020006
        Color0: (0 - 0) * 0 + SHADE
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = 07060705 in color channel
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - SHADE, , 
        1:Color: Sel - COMBINED, ,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlFacA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x06000000, 0x03070304, 0x02000000, 0x00020006}, // Simplified mux
            0x00322BFF, 0x5F0E923F,     // 64bit Mux
            23, // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x07060705, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SEL(DIF), MOD(T0,DIF), 0, true},   // Stage 0
            {SKIP, LERP(T1,CUR,ENV), 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x0010e5e0230b157f    Overflowed in PAPER MARIO
        Color0: (TEXEL0 - TEXEL1) * TEXEL0 + 1
        Color1: (0 - PRIM) * COMBINED + ENV
        Alpha0: (1 - TEXEL0) * TEXEL1 + TEXEL1
        Alpha1: (COMBINED - 0) * TEXEL1 + 0

        //Simplied Mux=0x0010e5e0230b157f   Overflowed in PAPER MARIO
        Simplied DWORDs=00010600, 04830004, 02010007, 00020004
        Color0: (0 - SHADE) * 1 + 0
        Color1: (ENV - 0) * 1 + COMBINED
        Alpha0: (TEXEL1 - 0) * TEXEL0|C + TEXEL1
        Alpha1: (TEXEL1 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE7_A_SUB_B_ADD_D
        Shade = PRIM in color channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sub - 0, SHADE, 
        1:Color: Add - ENV, COMBINED,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: MulAdd - TEXEL1, COMBINED|C, TEXEL1 -Tex1


        */


    {
        {0x00010600, 0x04830004, 0x02010007, 0x00020004}, // Simplified mux
            0x0010E5E0, 0x230B157F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MULADD(T0,DIF,ENV), SEL(T0), 0, true}, // Stage 0
            {SKIP, MULADD(CURC,T1,T1), 1, true},    // Stage 1
        }
    },


        /*
        //Mux=0x00117e045ffef3f8    Overflowed in RIDGE RACER 64
        Color0: (TEXEL0 - ENV) * TEXEL1 + ENV
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x00117e045ffef3f8   Overflowed in RIDGE RACER 64
        Simplied DWORDs=07040703, 03000000, 00020006, 02000000
        Color0: (TEXEL0 - ENV) * TEXEL1 + ENV
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - COMBINED, ENV, TEXEL1 -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x07040703, 0x03000000, 0x00020006, 0x02000000}, // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T1,DIF), SKIP, 1, true},   // Stage 0
            {LERP(T0,ENV,CUR), SEL(T0), 0, true},   // Stage 1
        }
    },


        /*
        //Mux=0x0040b467f0fffe3e    Overflowed in RIDGE RACER 64
        Color0: (SHADE - 0) * TEXEL0 + 0
        Color1: (PRIM - COMBINED) * COMBINED|A + COMBINED
        Alpha0: (PRIM - 0) * TEXEL1 + 0
        Alpha1: (0 - 0) * 0 + 1

        //Simplied Mux=0x0040b467f0fffe3e   Overflowed in RIDGE RACER 64
        Simplied DWORDs=00060003, 00050004, 02420205, 01000000
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (PRIM - COMBINED) * COMBINED|A + COMBINED
        Alpha0: (TEXEL1 - 0) * PRIM + 0
        Alpha1: (0 - 0) * 0 + 1
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:DISABLE_ALPHA, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Mod - TEXEL0, SHADE, 
        1:Color: BlCurA - PRIM, COMBINED,  -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Mod - TEXEL1, PRIM,  -Tex1


        */


    {
        {0x00060003, 0x00050004, 0x02420205, 0x01000000}, // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            //{LERP(PRI,CUR,CURA), MOD(T1,PRI), 1, true},   // Stage 1
            {SKIP, MOD(T1,PRI), 1, true},   // Stage 1
        }
    },


        /*
        //Mux=0x0022aa041f0c93ff    Overflowed in RIDGE RACER 64
        Color0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0

        //Simplied Mux=0x0022aa041f0c93ff   Overflowed in RIDGE RACER 64
        Simplied DWORDs=03070304, 03070304, 00020006, 00020006
        Color0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, ENV -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlFacA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x03070304, 0x03070304, 0x00020006, 0x00020006}, // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,ENV), LERP(T1,CUR,ENV), 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x0030fe045ffef3f8    Overflowed in RIDGE RACER 64
        Color0: (PRIM - ENV) * TEXEL0 + ENV
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x0030fe045ffef3f8   Overflowed in RIDGE RACER 64
        Simplied DWORDs=07030704, 03000000, 00020006, 02000000
        Color0: (TEXEL1 - ENV) * TEXEL0 + ENV
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, ENV, COMBINED -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x07030704, 0x03000000, 0x00020006, 0x02000000}, // Simplified mux
            0x0030FE04, 0x5FFEF3F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {LERP(T1,ENV,CUR), SKIP, 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x00272c041ffc93f8    Overflowed in RIDGE RACER 64
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x00272c041ffc93f8   Overflowed in RIDGE RACER 64
        Simplied DWORDs=030F0304, 04000000, 00020006, 02000000
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, PRIMLODFRAC -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Sel - TEXEL1, ,  -Tex1


        */


    {
        {0x030F0304, 0x04000000, 0x00020006, 0x02000000}, // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), SEL(T1), 1, true},   // Stage 1
        }
    },


        /*
        //Mux=0x00272c6015fc9378    Overflowed in RIDGE RACER 64
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x00272c6015fc9378   Overflowed in RIDGE RACER 64
        Simplied DWORDs=030F0304, 04000000, 06020605, 02000000
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = ENV in color channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, PRIMLODFRAC -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Sel - TEXEL1, ,  -Tex1


        */


    {
        {0x030F0304, 0x04000000, 0x06020605, 0x02000000}, // Simplified mux
            0x00272C60, 0x15FC9378,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000705, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(DIF,T0), SKIP, 0, true},   // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), SEL(T1), 1, true},   // Stage 1
        }
    },


        /*
        //Mux=0x001516032f1125ff    Overflowed in CASTLEVANIA2
        Color0: (TEXEL0 - TEXEL1) * PRIM|A + TEXEL1
        Color1: (COMBINED - 0) * PRIM + 0
        Alpha0: (TEXEL0 - TEXEL1) * PRIM + TEXEL1
        Alpha1: (COMBINED - 0) * SHADE + 0

        //Simplied Mux=0x001516032f1125ff   Overflowed in CASTLEVANIA2
        Simplied DWORDs=04460403, 04060403, 00020006, 00020006
        Color0: (TEXEL0 - TEXEL1) * SHADE|A + TEXEL1
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL0 - TEXEL1) * SHADE + TEXEL1
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = PRIM in color channel
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - COMBINED, TEXEL1,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - COMBINED, TEXEL1,  -Tex1


        */


    {
        {0x04460403, 0x04060403, 0x00020006, 0x00020006}, // Simplified mux
            0x00151603, 0x2F1125FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,PRI), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T0,T1,PRIA), LERP(T0,T1,PRI), 1, true},   // Stage 1
        }
    },


        /*
        //Mux=0x001516602515257f    Overflowed in CASTLEVANIA2
        Color0: (TEXEL0 - TEXEL1) * PRIM|A + TEXEL1
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL0 - TEXEL1) * PRIM + TEXEL1
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x001516602515257f   Overflowed in CASTLEVANIA2
        Simplied DWORDs=04450403, 04050403, 06020605, 00020006
        Color0: (TEXEL0 - TEXEL1) * PRIM|A + TEXEL1
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL0 - TEXEL1) * PRIM + TEXEL1
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = ENV in color channel
        Shade = ENV in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlFacA - COMBINED, TEXEL1,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlFacA - COMBINED, TEXEL1,  -Tex1


        */


    {
        {0x04450403, 0x04050403, 0x06020605, 0x00020006}, // Simplified mux
            0x00151660, 0x2515257F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000007, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(PRI,DIF,T0), MOD(T0,DIF), 0, true},   // Stage 0
            {LERP(T0,T1,PRIA), LERP(T0,T1,PRI), 1, true},   // Stage 1
        }
    },


        /*
        //Mux=0x001516032f1525ff    Overflowed in CASTLEVANIA2
        Color0: (TEXEL0 - TEXEL1) * PRIM|A + TEXEL1
        Color1: (COMBINED - 0) * PRIM + 0
        Alpha0: (TEXEL0 - TEXEL1) * PRIM + TEXEL1
        Alpha1: (COMBINED - 0) * ENV + 0

        //Simplied Mux=0x001516032f1525ff   Overflowed in CASTLEVANIA2
        Simplied DWORDs=04460403, 04060403, 00020006, 00020007
        Color0: (TEXEL0 - TEXEL1) * SHADE|A + TEXEL1
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL0 - TEXEL1) * SHADE + TEXEL1
        Alpha1: (ENV - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = PRIM in color channel
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: BlDifA - COMBINED, TEXEL1,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - COMBINED, TEXEL1,  -Tex1


        */


    {
        {0x04460403, 0x04060403, 0x00020006, 0x00020007}, // Simplified mux
            0x00151603, 0x2F1525FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,ENV), 0, true},    // Stage 0
            {LERP(T0,T1,DIFA), LERP(T0,T1,DIF), 1, true},   // Stage 1
        }
    },



        /*
        //Mux=0x00ffd5fffffcf238    Overflowed in CASTLEVANIA
        Color0: (0 - 0) * 0 + TEXEL0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (ENV - 0) * TEXEL1 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x00ffd5fffffcf238   Overflowed in CASTLEVANIA
        Simplied DWORDs=03000000, 00060004, 02000000, 02010003
        Color0: (0 - 0) * 0 + TEXEL0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (TEXEL0 - 0) * 1 + COMBINED
        Simplfied type: CM_FMT_TYPE5_A_MOD_C_ADD_D
        Shade = ENV in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Sel - COMBINED, ,  -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Mod - TEXEL1, SHADE,  -Tex1


        */


    {
        {0x03000000, 0x00060004, 0x02000000, 0x02010003}, // Simplified mux
            0x00FFD5FF, 0xFFFCF238,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SKIP, MOD(T0,DIF), 1, true},   // Stage 0
            {SEL(T0), ADD(T0,CUR), 0, true},    // Stage 1
        }
    },


        /*
        //Mux=0x0026a0041f0c93ff    Overflowed in NEWTETRIS
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0

        //Simplied Mux=0x0026a0041f0c93ff   Overflowed in NEWTETRIS
        Simplied DWORDs=00060003, 03060304, 02000000, 00020005
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (PRIM - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Mod - TEXEL0, SHADE, 
        1:Color: Sel - COMBINED, ,  -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x00060003, 0x03060304, 0x02000000, 0x00020005}, // Simplified mux
            0x0026A004, 0x1F0C93FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x0017166045fe7f78    Overflowed in DOUBUTSUNOMORI
        Color0: (TEXEL0 - SHADE) * PRIMLODFRAC + SHADE
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL0 - 0) * PRIM + 0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x0017166045fe7f78   Overflowed in DOUBUTSUNOMORI
        Simplied DWORDs=060F0603, 04000000, 07020704, 00020003
        Color0: (TEXEL0 - SHADE) * PRIMLODFRAC + SHADE
        Color1: (TEXEL1 - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (TEXEL0 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Lerp - TEXEL0, SHADE, PRIMLODFRAC
        1:Color: Lerp - TEXEL1, ENV, COMBINED -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Sel - TEXEL1, ,  -Tex1


        */


    {
        {0x060F0603, 0x04000000, 0x07020704, 0x00020003}, // Simplified mux
            0x00171660, 0x45FE7F78,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T1,DIF), SEL(T1), 1, true},    // Stage 0
            {LERP(T0,CUR,PRIMLODFRAC), MOD(T0,CUR), 0, true},   // Stage 1
        }
    },


        /*
        //Mux=0x003095ff5f1af43f    Overflowed in DOUBUTSUNOMORI
        Color0: (PRIM - ENV) * TEXEL0 + ENV
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL0 - 0) * TEXEL1 + TEXEL1
        Alpha1: (COMBINED - 0) * 1 + 0

        //Simplied Mux=0x003095ff5f1af43f   Overflowed in DOUBUTSUNOMORI
        Simplied DWORDs=06030605, 00030004, 02000000, 02010004
        Color0: (PRIM - SHADE) * TEXEL0 + SHADE
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - 0) * TEXEL0 + 0
        Alpha1: (TEXEL1 - 0) * 1 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = ENV in color channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Lerp - PRIM, SHADE, TEXEL0 -Tex1
        1:Color: Sel - COMBINED, , 
        0:Alpha: Sel - TEXEL1, ,  -Tex1
        1:Alpha: Mod - TEXEL0, COMBINED, 


        */


    {
        {0x06030605, 0x00030004, 0x02000000, 0x02010004}, // Simplified mux
            0x003095FF, 0x5F1AF43F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000007, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(PRI,DIF,T0), SEL(T0), 0, true},   // Stage 0
            {SKIP, MULADD(CUR,T1,T1), 1, true}, // Stage 1
        }
    },


        /*
        //Mux=0x003717fffffefe38    Overflowed in DOUBUTSUNOMORI
        Color0: (PRIM - 0) * PRIMLODFRAC + ENV
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL0 - 0) * PRIM + 0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x003717fffffefe38   Overflowed in DOUBUTSUNOMORI
        Simplied DWORDs=000F0006, 00060003, 02010004, 02000000
        Color0: (SHADE - 0) * PRIMLODFRAC + 0
        Color1: (TEXEL1 - 0) * 1 + COMBINED
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE5_A_MOD_C_ADD_D
        Shade = PRIM in color channel
        Shade = PRIM in alpha channel
        Tex 1 = ENV

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Mod - SHADE, PRIMLODFRAC, 
        1:Color: Add - TEXEL1, COMBINED,  -Tex1
        0:Alpha: Mod - TEXEL0, SHADE, 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */

    {
        {0x000F0006, 0x00060003, 0x02010004, 0x02000000}, // Simplified mux
            0x003717FF, 0xFFFEFE38,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000005, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000007}, // constant color texture flags
        {
            {MOD(DIF,PRIMLODFRAC), MOD(T0,DIF), 0, true},   // Stage 0
            {ADD(T1,CUR), SKIP, 1, true},   // Stage 1
        }
    },


        /*
        //Mux=0x00272a8013fc92f8    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - PRIM) * COMBINED + PRIM
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x00272a8013fc92f8   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=030F0304, 03060304, 05020506, 02000000
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - PRIM) * COMBINED + PRIM
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Shade = ENV in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sel - TEXEL0, , 
        1:Color: Lerp - TEXEL1, COMBINED, PRIMLODFRAC -Tex1
        0:Alpha: Sel - TEXEL0, , 
        1:Alpha: BlDifA - TEXEL1, COMBINED,  -Tex1


        */


    {
        {0x030F0304, 0x03060304, 0x05020506, 0x02000000}, // Simplified mux
            0x00272A80, 0x13FC92F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(DIF,PRI,T0), SEL(T0), 0, true},   // Stage 0
            {SKIP, LERP(T1,CUR,DIF), 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x00127e60f5fffd78    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED

        //Simplied Mux=0x00127e60f5fffd78   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=00060003, 01000000, 07020704, 02000000
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (TEXEL1 - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_C
        Tex 1 = PRIM

        Generated combiners:

        Stages:2, Alpha:DISABLE_ALPHA, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Mod - TEXEL0, SHADE, 
        1:Color: Lerp - TEXEL1, ENV, COMBINED -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x00060003, 0x01000000, 0x07020704, 0x02000000}, // Simplified mux
            0x00127E60, 0xF5FFFD78,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {LERP(T1,ENV,CUR), SKIP, 1, true},  // Stage 1
        }
    },


        /*
        //Mux=0x0020ac60350c937f    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - PRIM) * TEXEL0 + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0

        //Simplied Mux=0x0020ac60350c937f   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=00038604, 00060004, 07020706, 02000000
        Color0: (TEXEL1 - SHADE|C) * TEXEL0 + 0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE8_A_SUB_B_MOD_C
        Shade = PRIM in color channel
        Shade = PRIM in alpha channel

        Generated combiners:

        Stages:2, Alpha:ENABLE_BOTH, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Sub - TEXEL1, SHADE|C,  -Tex1
        1:Color: Mod - COMBINED, TEXEL0, 
        0:Alpha: Mod - TEXEL1, SHADE,  -Tex1
        1:Alpha: Sel - COMBINED, , 


        */


    {
        {0x00038604, 0x00060004, 0x07020706, 0x02000000}, // Simplified mux
            0x0020AC60, 0x350C937F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(DIF,ENV,T0), MOD(T0,DIF), 0, true},   // Stage 0
            {MOD(T1,CUR), MOD(T1,DIF), 1, true},    // Stage 1
        }
    },


        /*
        //Mux=0x00177e6035fcfd7e    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL0 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + 1

        //Simplied Mux=0x00177e6035fcfd7e   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=030F0603, 01000000, 04020406, 02000000
        Color0: (TEXEL0 - SHADE) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - TEXEL1) * COMBINED + TEXEL1
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: Color0
        Shade = PRIM in color channel
        Tex 1 = ENV

        Generated combiners:

        Stages:2, Alpha:DISABLE_ALPHA, Factor:0, Specular:0 Dif Color:0xCCCCCCCC Dif Alpha:0xCCCCCCCC
        0:Color: Lerp - TEXEL0, SHADE, PRIMLODFRAC
        1:Color: Lerp - SHADE, TEXEL1, COMBINED -Tex1
        0:Alpha: Sel - COMBINED, , 
        1:Alpha: Sel - COMBINED, ,  -Tex1


        */


    {
        {0x030F0603, 0x01000000, 0x04020406, 0x02000000}, // Simplified mux
            0x00177E60, 0x35FCFD7E,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000007}, // constant color texture flags
        {
            {LERP(T0,DIF,PRIMLODFRAC), SKIP, 0, true},  // Stage 0
            {LERP(DIF,T1,CUR), SKIP, 0, true},  // Stage 1
        }
    },


        /*
        //Mux=0x00276c6035d8ed76    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (1 - 1) * 1 + 1
        Alpha1: (1 - 1) * 1 + 1
        //Simplied Mux=0x00276c6035d8ed76   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=030F0604, 01000000, 07020706, 02000000Color0: (TEXEL1 - SHADE) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */


    {
        {0x030F0604, 0x01000000, 0x07020706, 0x02000000}, // Simplified mux
            0x00276C60, 0x35D8ED76,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x00277e60150cf37f    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0

        //Simplied Mux=0x00277e60150cf37f   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=030F0304, 00060003, 06020605, 02000000Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030F0304, 0x00060003, 0x06020605, 0x02000000}, // Simplified mux
            0x00277E60, 0x150CF37F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000007, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x001596a430fdfe38    Overflowed in Diddy Kong Racing
        Color0: (TEXEL0 - PRIM) * SHADE|A + PRIM
        Color1: (ENV - COMBINED) * SHADE + COMBINED
        Alpha0: (TEXEL0 - 0) * PRIM + 0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x001596a430fdfe38   Overflowed in Diddy Kong Racing
        Simplied DWORDs=04460403, 04000000, 02060207, 00020003Color0: (TEXEL0 - TEXEL1) * SHADE|A + TEXEL1
        Color1: (ENV - COMBINED) * SHADE + COMBINED
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (TEXEL0 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x04460403, 0x04000000, 0x02060207, 0x00020003}, // Simplified mux
            0x001596A4, 0x30FDFE38,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000007}, // constant color texture flags
        {
            {LERP(T0,PRI,DIFA), MOD(T0,PRI), 0, true},  // Stage 0
            {LERP(T1,CUR,DIF), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x001218acf00ffe3f    Overflowed in Diddy Kong Racing
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (ENV - COMBINED) * ENV|A + COMBINED
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x001218acf00ffe3f   Overflowed in Diddy Kong Racing
        Simplied DWORDs=00060003, 00060003, 02470207, 00020004Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (ENV - COMBINED) * ENV|A + COMBINED
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (TEXEL1 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x00060003, 0x00060003, 0x02470207, 0x00020004}, // Simplified mux
            0x001218AC, 0xF00FFE3F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(ENV,CUR,ENVA), MOD(T1,CUR), 1, true}, // Stage 1
        }
    },

        /*
        //Mux=0x002266ac1010923f    Overflowed in Diddy Kong Racing
        Color0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Color1: (ENV - COMBINED) * ENV|A + COMBINED
        Alpha0: (1 - TEXEL0) * PRIM + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x002266ac1010923f   Overflowed in Diddy Kong Racing
        Simplied DWORDs=03060304, 03050301, 02470207, 00020006Color0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Color1: (ENV - COMBINED) * ENV|A + COMBINED
        Alpha0: (1 - TEXEL0) * PRIM + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x03060304, 0x03050301, 0x02470207, 0x00020006}, // Simplified mux
            0x002266AC, 0x1010923F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(ENV,T0,ENVA), MOD(T0,DIF), 0, true},  // Stage 0
            {LERP(T1,CUR,DIF), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x0030fe045ffefbf8    Overflowed in F-ZERO X
        Color0: (PRIM - ENV) * TEXEL0 + ENV
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + ENV
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0030fe045ffefbf8   Overflowed in F-ZERO X
        Simplied DWORDs=07030704, 06000000, 00020006, 02000000Color0: (TEXEL1 - ENV) * TEXEL0 + ENV
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x07030704, 0x06000000, 0x00020006, 0x02000000}, // Simplified mux
            0x0030FE04, 0x5FFEFBF8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(ENV), 0, true},   // Stage 0
            {LERP(T1,ENV,CUR), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x0026a0801f0c93ff    Overflowed in ROCKETROBOTONWHEELS
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x0026a0801f0c93ff   Overflowed in ROCKETROBOTONWHEELS
        Simplied DWORDs=030E0304, 03060304, 00020006, 00020005Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (PRIM - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030E0304, 0x03060304, 0x00020006, 0x00020005}, // Simplified mux
            0x0026A080, 0x1F0C93FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x00272c60150c937f    Overflowed in POKEMON STADIUM
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x00272c60150c937f   Overflowed in POKEMON STADIUM
        Simplied DWORDs=030F0304, 00060004, 06020605, 02000000Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030F0304, 0x00060004, 0x06020605, 0x02000000}, // Simplified mux
            0x00272C60, 0x150C937F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000007, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), MOD(T1,DIF), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00162a0325fe13f8    Overflowed in POKEMON STADIUM
        Color0: (TEXEL0 - TEXEL1) * ENV|A + SHADE
        Color1: (COMBINED - ENV) * PRIM + 0
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00162a0325fe13f8   Overflowed in POKEMON STADIUM
        Simplied DWORDs=06460403, 03060304, 00050702, 02000000Color0: (TEXEL0 - TEXEL1) * SHADE|A + SHADE
        Color1: (COMBINED - ENV) * PRIM + 0
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */


    {
        {0x06460403, 0x03060304, 0x00050702, 0x02000000}, // Simplified mux
            0x00162A03, 0x25FE13F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {LERP(CUR,T1,ENVA), LERP(T1,CUR,ENV), 1, true}, // Stage 1
        }
    },

        /*
        //Mux=0x00167e03f5fe77f8    Overflowed in POKEMON STADIUM
        Color0: (TEXEL0 - 0) * ENV|A + SHADE
        Color1: (COMBINED - ENV) * PRIM + 0
        Alpha0: (0 - 0) * 0 + PRIM
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00167e03f5fe77f8   Overflowed in POKEMON STADIUM
        Simplied DWORDs=06470003, 04000000, 00040702, 02000000Color0: (TEXEL0 - 0) * ENV|A + SHADE
        Color1: (COMBINED - ENV) * TEXEL1 + 0
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE8_A_SUB_B_MOD_CGenerated combiners:
        */


    {
        {0x06470003, 0x04000000, 0x00040702, 0x02000000}, // Simplified mux
            0x00167E03, 0xF5FE77F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MULADD(T0,ENVA,DIF), SKIP, 0, true},   // Stage 0
            {MOD(CUR,T1), SEL(T1), 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x002698801514feff    Overflowed in CONKER BFD
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + PRIM
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (COMBINED - 0) * ENV + 0


        //Simplied Mux=0x002698801514feff   Overflowed in CONKER BFD
        Simplied DWORDs=00030706, 00060003, 02010004, 00020007Color0: (SHADE - ENV) * TEXEL0 + 0
        Color1: (TEXEL1 - 0) * 1 + COMBINED
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (ENV - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDShade = 00000706 in color channelGenerated combiners:
        */


    {
        {0x00030706, 0x00060003, 0x02010004, 0x00020007}, // Simplified mux
            0x00269880, 0x1514FEFF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000706, 0x00070006, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,LODFRAC), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x00127e04f513f4ff    Overflowed in CONKER BFD
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (COMBINED - ENV) * SHADE + PRIM
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x00127e04f513f4ff   Overflowed in CONKER BFD
        Simplied DWORDs=00060003, 00060004, 05060702, 02000000Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (COMBINED - ENV) * SHADE + PRIM
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */


    {
        {0x00060003, 0x00060004, 0x05060702, 0x02000000}, // Simplified mux
            0x00127E04, 0xF513F4FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {MULADD(CUR,DIF,PRI), MOD(T1,DIF), 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x00277e60350cf37f    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL1 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x00277e60350cf37f   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=030F0604, 00060003, 07020706, 02000000Color0: (TEXEL1 - SHADE) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */


    {
        {0x030F0604, 0x00060003, 0x07020706, 0x02000000}, // Simplified mux
            0x00277E60, 0x350CF37F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000005, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {MULADD(T1,PRIMLODFRAC,CUR), SKIP, 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x00272c041f0c93ff    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x00272c041f0c93ff   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=030F0304, 00060004, 00020006, 02000000Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030F0304, 0x00060004, 0x00020006, 0x02000000}, // Simplified mux
            0x00272C04, 0x1F0C93FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), MOD(T1,DIF), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x002714041f0cffff    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL0 - 0) * TEXEL1 + 0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x002714041f0cffff   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=030F0304, 00060003, 00020006, 00020004Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (TEXEL1 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030F0304, 0x00060003, 0x00020006, 0x00020004}, // Simplified mux
            0x00271404, 0x1F0CFFFF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), MOD(T1,CUR), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x002722041f0cffff    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - 0) * TEXEL0 + 0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x002722041f0cffff   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=030F0304, 00060004, 00020006, 00020003Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (TEXEL0 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030F0304, 0x00060004, 0x00020006, 0x00020003}, // Simplified mux
            0x00272204, 0x1F0CFFFF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), MOD(T1,CUR), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00272c603510f37f    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL1 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - 0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x00272c603510f37f   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=030F0604, 04010003, 07020706, 00020006Color0: (TEXEL1 - SHADE) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL0 - 0) * 1 + TEXEL1
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */


    {
        {0x030F0604, 0x04010003, 0x07020706, 0x00020006}, // Simplified mux
            0x00272C60, 0x3510F37F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {MULADD(T1,PRIMLODFRAC,CUR), MOD(T1,CUR), 1, true}, // Stage 1
        }
    },

        /*
        //Mux=0x00209204ff0fffff    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL1 - 0) * TEXEL0 + 0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL0 - 0) * TEXEL0 + 0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x00209204ff0fffff   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=00060004, 00030003, 00020003, 00020005Color0: (TEXEL1 - 0) * SHADE + 0
        Color1: (TEXEL0 - 0) * COMBINED + 0
        Alpha0: (TEXEL0 - 0) * TEXEL0 + 0
        Alpha1: (PRIM - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE2_A_ADD_DGenerated combiners:
        */


    {
        {0x00060004, 0x00030003, 0x00020003, 0x00020005}, // Simplified mux
            0x00209204, 0xFF0FFFFF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,T0), 0, true}, // Stage 0
            {MOD(T1,CUR), MOD(PRI,CUR), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x002714031f0cffff    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (COMBINED - 0) * PRIM + 0
        Alpha0: (TEXEL0 - 0) * TEXEL1 + 0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x002714031f0cffff   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=030F0304, 00060003, 00020006, 00020004Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (TEXEL1 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030F0304, 0x00060003, 0x00020006, 0x00020004}, // Simplified mux
            0x00271403, 0x1F0CFFFF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000005, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,T0,PRIMLODFRAC), MOD(T1,CUR), 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x00272c031f1093ff    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (COMBINED - 0) * PRIM + 0
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x00272c031f1093ff   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=030F0304, 00060004, 00020006, 02000000Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030F0304, 0x00060004, 0x00020006, 0x02000000}, // Simplified mux
            0x00272C03, 0x1F1093FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), MOD(T1,DIF), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x0012fe043ffe77f8    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL0 - PRIM) * ENV + SHADE
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + PRIM
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0012fe043ffe77f8   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=06070403, 04000000, 00020006, 02000000Color0: (TEXEL0 - TEXEL1) * ENV + SHADE
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */


    {
        {0x06070403, 0x04000000, 0x00020006, 0x02000000}, // Simplified mux
            0x0012FE04, 0x3FFE77F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MULADD(T0,ENV,DIF), SEL(DIF), 0, true},    // Stage 0
            {MOD(CUR,DIF), SKIP, 0, false}, // Stage 1
        }
    },

        /*
        //Mux=0x0020fe05f3fff738    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL1 - 0) * TEXEL0 + 0
        Color1: (COMBINED - PRIM) * ENV + SHADE
        Alpha0: (0 - 0) * 0 + PRIM
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0020fe05f3fff738   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=00030004, 06000000, 06070502, 02000000Color0: (TEXEL1 - 0) * TEXEL0 + 0
        Color1: (COMBINED - PRIM) * ENV + SHADE
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */


    {
        {0x00030004, 0x06000000, 0x06070502, 0x02000000}, // Simplified mux
            0x0020FE05, 0xF3FFF738,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,ENV), SEL(DIF), 0, true},   // Stage 0
            {MULADD(T1,CUR,DIF), SKIP, 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x0012fe043f1677ff    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL0 - PRIM) * ENV + SHADE
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + PRIM
        Alpha1: (COMBINED - 0) * ENV + 0


        //Simplied Mux=0x0012fe043f1677ff   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=06070403, 00070004, 00020006, 02000000Color0: (TEXEL0 - TEXEL1) * ENV + SHADE
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - 0) * ENV + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */


    {
        {0x06070403, 0x00070004, 0x00020006, 0x02000000}, // Simplified mux
            0x0012FE04, 0x3F1677FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MULADD(T0,ENV,DIF), MOD(DIF,ENV), 0, true},    // Stage 0
            {MOD(CUR,DIF), SKIP, 0, false}, // Stage 1
        }
    },

        /*
        //Mux=0x0011fe04ff17f7ff    Overflowed in THE MASK OF MUJURA
        Color0: (TEXEL0 - 0) * PRIM + 0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + PRIM
        Alpha1: (COMBINED - 0) * ENV + 0


        //Simplied Mux=0x0011fe04ff17f7ff   Overflowed in THE MASK OF MUJURA
        Simplied DWORDs=00030006, 00070004, 00020004, 02000000Color0: (SHADE - 0) * TEXEL0 + 0
        Color1: (TEXEL1 - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - 0) * ENV + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE2_A_ADD_DGenerated combiners:
        */


    {
        {0x00030006, 0x00070004, 0x00020004, 0x02000000}, // Simplified mux
            0x0011FE04, 0xFF17F7FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {MOD(T1,CUR), MOD(T1,DIF), 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x00272c031f0c93ff    Overflowed in MULTI RACING
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (COMBINED - 0) * PRIM + 0
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x00272c031f0c93ff   Overflowed in MULTI RACING
        Simplied DWORDs=030F0304, 00060004, 00020006, 02000000Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030F0304, 0x00060004, 0x00020006, 0x02000000}, // Simplified mux
            0x00272C03, 0x1F0C93FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000005, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), MOD(T1,DIF), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00272c041f0c93ff    Overflowed in MULTI RACING
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x00272c041f0c93ff   Overflowed in MULTI RACING
        Simplied DWORDs=030F0304, 00060004, 00020006, 02000000Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030F0304, 0x00060004, 0x00020006, 0x02000000}, // Simplified mux
            0x00272C04, 0x1F0C93FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), MOD(T1,DIF), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x0030fe045ffef7f8    Overflowed in MULTI RACING
        Color0: (PRIM - ENV) * TEXEL0 + ENV
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + PRIM
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0030fe045ffef7f8   Overflowed in MULTI RACING
        Simplied DWORDs=07030704, 04000000, 00020006, 02000000Color0: (TEXEL1 - ENV) * TEXEL0 + ENV
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x07030704, 0x04000000, 0x00020006, 0x02000000}, // Simplified mux
            0x0030FE04, 0x5FFEF7F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {LERP(T1,ENV,CUR), SEL(T1), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x0026a08015fc93f8    Overflowed in Monaco Grand Prix
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0026a08015fc93f8   Overflowed in Monaco Grand Prix
        Simplied DWORDs=030E0304, 03060304, 00020706, 02000000Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE8_A_SUB_B_MOD_CGenerated combiners:
        */


    {
        {0x030E0304, 0x03060304, 0x00020706, 0x02000000}, // Simplified mux
            0x0026A080, 0x15FC93F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x0050fe043ffdf3f8    Overflowed in KING HILL 64
        Color0: (ENV - PRIM) * TEXEL0 + PRIM
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0050fe043ffdf3f8   Overflowed in KING HILL 64
        Simplied DWORDs=04030407, 03000000, 00020006, 02000000Color0: (ENV - TEXEL1) * TEXEL0 + TEXEL1
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x04030407, 0x03000000, 0x00020006, 0x02000000}, // Simplified mux
            0x0050FE04, 0x3FFDF3F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000007}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {LERP(T1,PRI,CUR), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x00209a04ffcfffc8    Overflowed in HSV ADVENTURE RACING
        Color0: (TEXEL1 - 0) * TEXEL0 + 0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL0 - 0) * ENV + 0
        Alpha1: (1 - TEXEL0) * PRIM + COMBINED


        //Simplied Mux=0x00209a04ffcfffc8   Overflowed in HSV ADVENTURE RACING
        Simplied DWORDs=00060004, 00070003, 00020003, 02060083Color0: (TEXEL1 - 0) * SHADE + 0
        Color1: (TEXEL0 - 0) * COMBINED + 0
        Alpha0: (TEXEL0 - 0) * ENV + 0
        Alpha1: (TEXEL0|C - 0) * SHADE + COMBINED
        Simplfied type: CM_FMT_TYPE5_A_MOD_C_ADD_DGenerated combiners:
        */


    {
        {0x00060004, 0x00070003, 0x00020003, 0x02060083}, // Simplified mux
            0x00209A04, 0xFFCFFFC8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,ENV), 0, true},    // Stage 0
            {MOD(T1,CUR), MULADD(T0C,DIF,CUR), 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x00ffa1ffff12123f    Overflowed in HSV ADVENTURE RACING
        Color0: (0 - 0) * 0 + SHADE
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x00ffa1ffff12123f   Overflowed in HSV ADVENTURE RACING
        Simplied DWORDs=06000000, 03060304, 02000000, 00020006Color0: (0 - 0) * 0 + SHADE
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x06000000, 0x03060304, 0x02000000, 0x00020006}, // Simplified mux
            0x00FFA1FF, 0xFF12123F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SEL(DIF), SEL(T0), 0, true},   // Stage 0
            {SKIP, LERP(T1,T0,DIF), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x0020980a14fcff38    Overflowed in HSV ADVENTURE RACING
        Color0: (TEXEL1 - TEXEL0) * TEXEL0 + TEXEL0
        Color1: (COMBINED - SHADE) * PRIM|A + SHADE
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0020980a14fcff38   Overflowed in HSV ADVENTURE RACING
        Simplied DWORDs=03030304, 00060003, 06450602, 02000000Color0: (TEXEL1 - TEXEL0) * TEXEL0 + TEXEL0
        Color1: (COMBINED - SHADE) * PRIM|A + SHADE
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x03030304, 0x00060003, 0x06450602, 0x02000000}, // Simplified mux
            0x0020980A, 0x14FCFF38,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MULADD(T0,PRIA,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,T0,CUR), SKIP, 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00247ec0f2fffebe    Overflowed in HSV ADVENTURE RACING
        Color0: (TEXEL1 - 0) * TEXEL0|A + 0
        Color1: (1 - TEXEL1) * COMBINED + TEXEL1
        Alpha0: (0 - 0) * 0 + 0
        Alpha1: (0 - 0) * 0 + 1


        //Simplied Mux=0x00247ec0f2fffebe   Overflowed in HSV ADVENTURE RACING
        Simplied DWORDs=00430004, 01000000, 03020301, 02000000Color0: (TEXEL1 - 0) * TEXEL0|A + 0
        Color1: (1 - TEXEL0) * COMBINED + TEXEL0
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x00430004, 0x01000000, 0x03020301, 0x02000000}, // Simplified mux
            0x00247EC0, 0xF2FFFEBE,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            0,      // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SEL(T0), SKIP, 0, true},   // Stage 0
            {MULADD(T1C,CUR,T1), SKIP, 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x0020fe0411fd7ebe    Overflowed in HSV ADVENTURE RACING
        Color0: (TEXEL1 - TEXEL0) * TEXEL0 + TEXEL1
        Color1: (COMBINED - TEXEL0) * SHADE + TEXEL1
        Alpha0: (0 - 0) * 0 + 0
        Alpha1: (0 - 0) * 0 + 1


        //Simplied Mux=0x0020fe0411fd7ebe   Overflowed in HSV ADVENTURE RACING
        Simplied DWORDs=04030304, 01000000, 03060302, 02000000Color0: (TEXEL1 - TEXEL0) * TEXEL0 + TEXEL1
        Color1: (COMBINED - TEXEL0) * SHADE + TEXEL0
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: Color0Generated combiners:
        */


    {
        {0x04030304, 0x01000000, 0x03060302, 0x02000000}, // Simplified mux
            0x0020FE04, 0x11FD7EBE,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            0,      // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SKIP, 0, true},   // Stage 0
            {MOD(T1,CUR), SKIP, 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00111480f513ff7f    Overflowed in HSV ADVENTURE RACING
        Color0: (TEXEL0 - 0) * TEXEL1 + 0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL0 - 0) * TEXEL1 + 0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x00111480f513ff7f   Overflowed in HSV ADVENTURE RACING
        Simplied DWORDs=00040003, 00060003, 07020706, 00020004Color0: (TEXEL0 - 0) * TEXEL1 + 0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (TEXEL1 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x00040003, 0x00060003, 0x07020706, 0x00020004}, // Simplified mux
            0x00111480, 0xF513FF7F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(DIF,ENV,T0), MOD(T0,DIF), 0, true},   // Stage 0
            {MOD(T1,CUR), MOD(T1,CUR), 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x00117e042ffd79f8    Overflowed in HSV ADVENTURE RACING
        Color0: (TEXEL0 - TEXEL1) * TEXEL1 + TEXEL1
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00117e042ffd79f8   Overflowed in HSV ADVENTURE RACING
        Simplied DWORDs=04040403, 06000000, 00020006, 02000000Color0: (TEXEL0 - TEXEL1) * TEXEL1 + TEXEL1
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x04040403, 0x06000000, 0x00020006, 0x02000000}, // Simplified mux
            0x00117E04, 0x2FFD79F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            0,      // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(DIF), 0, true},   // Stage 0
            {MOD(T1,CUR), SKIP, 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00257e041ffcf3f8    Overflowed in G.A.S.P!!Fighters'NE
        Color0: (TEXEL1 - TEXEL0) * PRIM|A + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00257e041ffcf3f8   Overflowed in G.A.S.P!!Fighters'NE
        Simplied DWORDs=03460304, 03000000, 00020006, 02000000
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x03460304, 0x03000000, 0x00020006, 0x02000000}, // Simplified mux
            0x00257E04, 0x1FFCF3F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {LERP(T1,CUR,PRI), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x00ff9480fffcfff8    Overflowed in G.A.S.P!!Fighters'NE
        Color0: (0 - 0) * 0 + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL0 - 0) * TEXEL1 + 0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00ff9480fffcfff8   Overflowed in G.A.S.P!!Fighters'NE
        Simplied DWORDs=00060003, 04000000, 02000000, 00020003
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (TEXEL0 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE2_A_ADD_DGenerated combiners:
        */


    {
        {0x00060003, 0x04000000, 0x02000000, 0x00020003}, // Simplified mux
            0, 0,       // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            0,      // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {SKIP, MOD(T1,CUR), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00612680fffcf3f8    Overflowed in G.A.S.P!!Fighters'NE
        Color0: (1 - 0) * TEXEL1 + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - 0) * PRIM + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00612680fffcf3f8   Overflowed in G.A.S.P!!Fighters'NE
        Simplied DWORDs=03010004, 00060004, 00020006, 02010003Color0: (TEXEL1 - 0) * 1 + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (TEXEL0 - 0) * 1 + COMBINED
        Simplfied type: CM_FMT_TYPE5_A_MOD_C_ADD_DGenerated combiners:
        */


    {
        {0x03010004, 0x00060004, 0x00020006, 0x02010003}, // Simplified mux
            0x00612680, 0xFFFCF3F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T1,DIF), MOD(T1,PRI), 1, true},    // Stage 0
            {MOD(T0,CUR), ADD(T0,CUR), 0, true},    // Stage 1
        }
    },

        /*
        //Mux=0x0026a06015fc9378    Overflowed in FIFA Soccer 64
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0026a06015fc9378   Overflowed in FIFA Soccer 64
        Simplied DWORDs=030E0304, 03060304, 06020605, 02000000Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030E0304, 0x03060304, 0x06020605, 0x02000000}, // Simplified mux
            0x0026A060, 0x15FC9378,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000005, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(DIF,DIFA,T0), SEL(T0), 0, true},  // Stage 0
            {LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x0026a0041ffc93fe    Overflowed in Taz Express
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (0 - 0) * 0 + 1


        //Simplied Mux=0x0026a0041ffc93fe   Overflowed in Taz Express
        Simplied DWORDs=030E0304, 01000000, 00020006, 02000000
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030E0304, 0x01000000, 0x00020006, 0x02000000}, // Simplified mux
            0x0026A004, 0x1FFC93FE,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_LODFRAC,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(T0), 0, true},    // Stage 0
            {LERP(T1,CUR,LODFRAC), LERP(T1,CUR,LODFRAC), 1, true},  // Stage 1
        }
    },
        /*
        //Mux=0x003716041ffcfff8    Overflowed in GAUNTLET LEGENDS
        Color0: (PRIM - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL0 - 0) * PRIM + 0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x003716041ffcfff8   Overflowed in GAUNTLET LEGENDS
        Simplied DWORDs=030F0304, 04000000, 00020006, 00020003
        Color0: (TEXEL1 - TEXEL0) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + TEXEL1
        Alpha1: (TEXEL0 - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x030F0304, 0x04000000, 0x00020006, 0x00020003}, // Simplified mux
            0x00371604, 0x1FFCFFF8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,PRIMLODFRAC), SKIP, 1, true},  // Stage 1
        }
    },
        /*
        //Mux=0x00157e602ffd77f8    Overflowed in MarioTennis
        Color0: (TEXEL0 - TEXEL1) * PRIM|A + TEXEL1
        Color1: (PRIM - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + PRIM
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00157e602ffd77f8   Overflowed in MarioTennis
        Simplied DWORDs=04460403, 06000000, 00020006, 02000000
        Color0: (TEXEL0 - TEXEL1) * SHADE|A + TEXEL1
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x04460403, 0x06000000, 0x00020006, 0x02000000}, // Simplified mux
            0x00157E60, 0x2FFD77F8,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000005, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(DIF), 0, true},   // Stage 0
            {LERP(T1,CUR,DIFA), SKIP, 1, true}, // Stage 1
        }
    },
        /*
        //Mux=0x00157e6025fd7778    Overflowed in MarioTennis
        Color0: (TEXEL0 - TEXEL1) * PRIM|A + TEXEL1
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + PRIM
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00157e6025fd7778   Overflowed in MarioTennis
        Simplied DWORDs=04460403, 06000000, 06020605, 02000000
        Color0: (TEXEL0 - TEXEL1) * SHADE|A + TEXEL1
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x04460403, 0x06000000, 0x06020605, 0x02000000}, // Simplified mux
            0x00157E60, 0x25FD7778,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x000000007, 0x00000000, 0, // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(PRI,DIF,T0), SEL(DIF), 0, true},  // Stage 0
            {LERP(CUR,T1,DIFA), SKIP, 1, true}, // Stage 1
        }
    },

        /*
        //Mux=0x00fffe80f514f8ff    Overflowed in CONKER BFD
        Color0: (0 - 0) * 0 + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + PRIM
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (COMBINED - 0) * ENV + 0
        */

    {
        {0x00030706, 0x06000000, 0x02010004, 0x02000000}, // Simplified mux
            0x00FFFE80, 0xF514F8FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000706, 0x00070006, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), SEL(DIF), 0, true},   // Stage 0
            {ADD(PRI,CUR), SKIP, 0, false}, // Stage 1
        }
    },

        /*
        //Mux=0x0017166035fcff78    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL0 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL0 - 0) * PRIM + 0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0017166035fcff78   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=030F0603, 00060003, 04020406, 02000000
        Color0: (TEXEL0 - SHADE) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - TEXEL1) * COMBINED + TEXEL1
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: Color0Generated combiners:
        */

    {
        {0x030F0603, 0x00060003, 0x04020406, 0x02000000}, // Simplified mux
            0x00171660, 0x35FCFF78,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIMLODFRAC,        // Constant color
            0x00000005, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000007}, // constant color texture flags
        {
            {LERP(T0,DIF,PRIMLODFRAC), MOD(T0,DIF), 0, true},   // Stage 0
            {LERP(DIF,T1,CUR), SKIP, 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x00262a041f1093ff    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - TEXEL0) * ENV|A + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x00262a041f1093ff   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=03470304, 03070304, 00020006, 00020006
        Color0: (TEXEL1 - TEXEL0) * ENV|A + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (TEXEL1 - TEXEL0) * ENV + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */

    {
        {0x03470304, 0x03070304, 0x00020006, 0x00020006}, // Simplified mux
            0x00262A04, 0x1F1093FF,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(T1,CUR,ENVA), LERP(T1,CUR,ENV), 1, true}, // Stage 1
        }
    },

        /*
        //Mux=0x00267e051ffcfdf8    Overflowed in THE LEGEND OF ZELDA
        Color0: (TEXEL1 - TEXEL0) * ENV|A + TEXEL0
        Color1: (COMBINED - 0) * ENV + 0
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00267e051ffcfdf8   Overflowed in THE LEGEND OF ZELDA
        Simplied DWORDs=03460304, 01000000, 00020006, 02000000
        Color0: (TEXEL1 - TEXEL0) * SHADE|A + TEXEL0
        Color1: (SHADE - 0) * COMBINED + 0
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x03460304, 0x01000000, 0x00020006, 0x02000000}, // Simplified mux
            0x00267E05, 0x1FFCFDF8,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            MUX_ENV,        // Constant color
            0x00000007, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,ENV), SKIP, 0, true},   // Stage 0
            {LERP(T1,CUR,ENVA), SKIP, 1, true}, // Stage 1
        }
    },

        /*
        //Mux=0x0026a0041f1093fb    Overflowed in GOLDENEYE
        Color0: (TEXEL1 - TEXEL0) * LODFRAC + TEXEL0
        Color1: (COMBINED - 0) * SHADE + 0
        Alpha0: (TEXEL1 - TEXEL0) * COMBINED + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + PRIM


        //Simplied Mux=0x0026a0041f1093fb   Overflowed in GOLDENEYE
        Simplied DWORDs=00060003, 03060304, 02000000, 05020006
        Color0: (TEXEL0 - 0) * SHADE + 0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - TEXEL0) * SHADE + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + PRIM
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */


    {
        {0x00060003, 0x03060304, 0x02000000, 0x05020006}, // Simplified mux
            0x0026A004, 0x1F1093FB,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {SKIP, ADD(CUR,PRI), 0, false}, // Stage 1
        }
    },

        /*
        //Mux=0x0017666025fd7f78    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL0 - TEXEL1) * PRIMLODFRAC + TEXEL1
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (1 - 0) * PRIM + 0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0017666025fd7f78   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=040F0403, 06000000, 06020605, 02000000
        Color0: (TEXEL0 - TEXEL1) * PRIMLODFRAC + TEXEL1
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */

    {
        {0x040F0403, 0x06000000, 0x06020605, 0x02000000}, // Simplified mux
            0x00176660, 0x25FD7F78,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(DIF,ENV,T0), SEL(DIF), 0, true},  // Stage 0
            {SKIP, SKIP, 0, false}, // Stage 1
        }
    },

        /*
        //Mux=0x0077666045fd7f78    Overflowed in POKEMON STADIUM 2
        Color0: (COMBALPHA - SHADE) * PRIMLODFRAC + TEXEL1
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (1 - 0) * PRIM + 0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0077666045fd7f78   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=040F0608, 03000000, 07020703, 02000000
        Color0: (COMBALPHA - SHADE) * PRIMLODFRAC + TEXEL1
        Color1: (TEXEL0 - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDShade = 000F0608 in color channelGenerated combiners:
        */

    {
        {0x040F0608, 0x03000000, 0x07020703, 0x02000000}, // Simplified mux
            0x00776660, 0x45FD7F78,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000005, 0x00000000}, // constant color texture flags
        {
            {MOD(T1,DIF), SKIP, 1, true},   // Stage 0
            {LERP(T1,ENV,CUR), SEL(T0), 0, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00457fff3ffcfe3f    Overflowed in POKEMON STADIUM 2
        Color0: (SHADE - PRIM) * PRIM|A + TEXEL0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (0 - 0) * 0 + 0
        Alpha1: (0 - 0) * 0 + 0


        //Simplied Mux=0x00457fff3ffcfe3f   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=00460506, 00000000, 02010003, 02000000
        Color0: (SHADE - PRIM) * SHADE|A + 0
        Color1: (TEXEL0 - 0) * 1 + COMBINED
        Alpha0: (0 - 0) * 0 + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDShade = 00460506 in color channelGenerated combiners:
        */

    {
        {0x00460506, 0x00000000, 0x02010003, 0x02000000}, // Simplified mux
            0x00457FFF, 0x3FFCFE3F,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SUB(DIF,PRI), SKIP, 0, false}, // Stage 0
            {MULADD(CUR,PRIA,T0), SKIP, 0, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00272c603510e37f    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL1 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - 1) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x00272c603510e37f   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=030F0604, 00060003, 07020706, 02000000
        Color0: (TEXEL1 - SHADE) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */

    {
        {0x030F0604, 0x00060003, 0x07020706, 0x02000000}, // Simplified mux
            0x00272C60, 0x3510E37F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(DIF,ENV,T0), MOD(T0,DIF), 0, true},   // Stage 0
            {SKIP, MULADD(T1,DIF,CUR), 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x0025a660f510f37f    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL1 - 0) * SHADE|A + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - 0) * PRIM + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x0025a660f510f37f   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=03460004, 03050004, 07020705, 00020006
        Color0: (TEXEL1 - 0) * SHADE|A + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - 0) * PRIM + TEXEL0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */

    {
        {0x03460004, 0x03050004, 0x07020705, 0x00020006}, // Simplified mux
            0x0025A660, 0xF510F37F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(DIF,ENV,T0), MOD(T0,DIF), 0, true},   // Stage 0
            {MULADD(T1,DIFA,CUR), MULADD(T1,DIF,CUR), 1, true}, // Stage 1
        }
    },

        /*
        //Mux=0x00171607f511a97f    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL0 - 0) * PRIMLODFRAC + PRIM
        Color1: (COMBINED - ENV) * COMBINED|A + ENV
        Alpha0: (TEXEL0 - TEXEL1) * PRIM + SHADE
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x00171607f511a97f   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=050F0003, 06050403, 06420602, 00020006
        Color0: (TEXEL0 - 0) * PRIMLODFRAC + PRIM
        Color1: (COMBINED - SHADE) * COMBINED|A + SHADE
        Alpha0: (TEXEL0 - TEXEL1) * PRIM + SHADE
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */

    {
        {0x050F0003, 0x06050403, 0x06420602, 0x00020006}, // Simplified mux
            0x00171607, 0xF511A97F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {ADD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0
            {LERP(CUR,ENV,CURA), SUB(CUR,T1), 1, true}, // Stage 1
        }
    },

        /*
        //Mux=0x00177e6025fd7378    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL0 - TEXEL1) * PRIMLODFRAC + TEXEL1
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00177e6025fd7378   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=040F0403, 03000000, 06020605, 02000000
        Color0: (TEXEL0 - TEXEL1) * PRIMLODFRAC + TEXEL1
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */

    {
        {0x040F0403, 0x03000000, 0x06020605, 0x02000000}, // Simplified mux
            0x00177E60, 0x25FD7378,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(DIF,ENV,T0), SEL(T0), 0, true},   // Stage 0
            {SKIP, SKIP, 0, true},  // Stage 1
        }
    },

        /*
        //Mux=0x0017666025fd7f78    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL0 - TEXEL1) * PRIMLODFRAC + TEXEL1
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (1 - 0) * PRIM + 0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x0017666025fd7f78   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=040F0403, 06000000, 06020605, 02000000
        Color0: (TEXEL0 - TEXEL1) * PRIMLODFRAC + TEXEL1
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (0 - 0) * 0 + SHADE
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */

    {
        {0x040F0403, 0x06000000, 0x06020605, 0x02000000}, // Simplified mux
            0x00176660, 0x25FD7F78,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000007, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(PRI,DIF,T0), SEL(PRI), 0, true},  // Stage 0
            {SKIP, SKIP, 0, false}, // Stage 1
        }
    },

        /*
        //Mux=0x00177e6025fd7378    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL0 - TEXEL1) * PRIMLODFRAC + TEXEL1
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00177e6025fd7378   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=040F0403, 03000000, 06020605, 02000000
        Color0: (TEXEL0 - TEXEL1) * PRIMLODFRAC + TEXEL1
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (0 - 0) * 0 + TEXEL0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */

    {
        {0x040F0403, 0x03000000, 0x06020605, 0x02000000}, // Simplified mux
            0x00177E60, 0x25FD7378,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000007, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(PRI,DIF,T0), SEL(T0), 0, true},   // Stage 0
            {SKIP, SKIP, 0, false}, // Stage 1
        }
    },

        /*
        //Mux=0x00457fff3ffcfe3f    Overflowed in POKEMON STADIUM 2
        Color0: (SHADE - PRIM) * PRIM|A + TEXEL0
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (0 - 0) * 0 + 0
        Alpha1: (0 - 0) * 0 + 0


        //Simplied Mux=0x00457fff3ffcfe3f   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=00460506, 00000000, 02010003, 02000000
        Color0: (SHADE - PRIM) * SHADE|A + 0
        Color1: (TEXEL0 - 0) * 1 + COMBINED
        Alpha0: (0 - 0) * 0 + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDShade = 00460506 in color channelGenerated combiners:
        */

    {
        {0x00460506, 0x00000000, 0x02010003, 0x02000000}, // Simplified mux
            0x00457FFF, 0x3FFCFE3F,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            MUX_ENV,        // Constant color
            0x00460506, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {ADD(T0,DIF), SKIP, 0, true},   // Stage 0
            {SKIP, SKIP, 0, false}, // Stage 1
        }
    },

        /*
        //Mux=0x00272c60350c937f    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL1 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * PRIM + 0


        //Simplied Mux=0x00272c60350c937f   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=030F0604, 00060004, 07020706, 02000000
        Color0: (TEXEL1 - SHADE) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */

    {
        {0x030F0604, 0x00060004, 0x07020706, 0x02000000}, // Simplified mux
            0x00272C60, 0x350C937F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000007, 0x00000005, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(PRI,DIF,T0),SKIP, 0, true},   // Stage 0
            {SKIP, MOD(T1,DIF), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00272c603510e37f    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL1 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - 1) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x00272c603510e37f   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=030F0604, 00060003, 07020706, 02000000
        Color0: (TEXEL1 - SHADE) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL0 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */

    {
        {0x030F0604, 0x00060003, 0x07020706, 0x02000000}, // Simplified mux
            0x00272C60, 0x3510E37F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SKIP,MOD(T1,DIF), 1, true},    // Stage 0
            {LERP(DIF,ENV,T0), MULADD(T0,DIF,CUR), 0, true},    // Stage 1
        }
    },

        /*
        //Mux=0x0030e5ff5f16f63f    Overflowed in POKEMON STADIUM 2
        Color0: (PRIM - ENV) * TEXEL0 + ENV
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (1 - 0) * TEXEL1 + PRIM
        Alpha1: (COMBINED - 0) * ENV + 0


        //Simplied Mux=0x0030e5ff5f16f63f   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=06030605, 05010004, 02000000, 00020006
        Color0: (PRIM - SHADE) * TEXEL0 + SHADE
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL1 - 0) * 1 + PRIM
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */

    {
        {0x06030605, 0x05010004, 0x02000000, 0x00020006}, // Simplified mux
            0x0030E5FF, 0x5F16F63F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000007, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(PRI,DIF,T0), MOD(PRI,DIF), 0, true},  // Stage 0
            {SKIP, MULADD(T1,DIF,CUR), 1, true},    // Stage 1
        }
    },

        /*
        //Mux=0x003117ff5f16fe3f    Overflowed in POKEMON STADIUM 2
        Color0: (PRIM - ENV) * TEXEL1 + ENV
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL0 - 0) * PRIM + 0
        Alpha1: (COMBINED - 0) * ENV + 0


        //Simplied Mux=0x003117ff5f16fe3f   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=06040605, 00050003, 02000000, 00020006
        Color0: (PRIM - SHADE) * TEXEL1 + SHADE
        Color1: (0 - 0) * 0 + COMBINED
        Alpha0: (TEXEL0 - 0) * PRIM + 0
        Alpha1: (SHADE - 0) * COMBINED + 0
        Simplfied type: CM_FMT_TYPE6_A_LERP_B_CGenerated combiners:
        */

    {
        {0x06040605, 0x00050003, 0x02000000, 0x00020006}, // Simplified mux
            0x003117FF, 0x5F16FE3F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000007, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {SKIP, MOD(T0,PRI), 0, true},   // Stage 0
            {LERP(PRI,DIF,T1), MOD(DIF,CUR), 1, true},  // Stage 1
        }
    },

        /*
        //Mux=0x00272c603410933f    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL1 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x00272c603410933f   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=030F0504, 00060004, 06020605, 02000000
        Color0: (TEXEL1 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - SHADE) * COMBINED + SHADE
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */

    {
        {0x030F0504, 0x00060004, 0x06020605, 0x02000000}, // Simplified mux
            0x00272C60, 0x3410933F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_PRIM,       // Constant color
            0x00000000, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(PRI,DIF,T0), SKIP, 0, true},  // Stage 0
            {SKIP, MOD(T1,DIF), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00272c603510937f    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL1 - PRIM) * PRIMLODFRAC + TEXEL0
        Color1: (PRIM - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - TEXEL0) * 1 + TEXEL0
        Alpha1: (COMBINED - 0) * SHADE + 0


        //Simplied Mux=0x00272c603510937f   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=030F0604, 00060004, 07020706, 02000000
        Color0: (TEXEL1 - SHADE) * PRIMLODFRAC + TEXEL0
        Color1: (SHADE - ENV) * COMBINED + ENV
        Alpha0: (TEXEL1 - 0) * SHADE + 0
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE_NOT_CHECKEDGenerated combiners:
        */

    {
        {0x030F0604, 0x00060004, 0x07020706, 0x02000000}, // Simplified mux
            0x00272C60, 0x3510937F,     // 64bit Mux
            2,  // number of stages
            ENABLE_BOTH,
            MUX_ENV,        // Constant color
            0x00000005, 0x00000000, 0,  // Shade and specular color flags
            {0x00000000, 0x00000000}, // constant color texture flags
        {
            {LERP(DIF,ENV,T0), SKIP, 0, true},  // Stage 0
            {SKIP, MOD(T1,DIF), 1, true},   // Stage 1
        }
    },

        /*
        //Mux=0x00167e835ffffc38    Overflowed in POKEMON STADIUM 2
        Color0: (TEXEL0 - ENV) * ENV|A + 0
        Color1: (SHADE - 0) * PRIM + COMBINED
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED


        //Simplied Mux=0x00167e835ffffc38   Overflowed in POKEMON STADIUM 2
        Simplied DWORDs=00460703, 01000000, 02060004, 02000000
        Color0: (TEXEL0 - ENV) * SHADE|A + 0
        Color1: (TEXEL1 - 0) * SHADE + COMBINED
        Alpha0: (0 - 0) * 0 + 1
        Alpha1: (0 - 0) * 0 + COMBINED
        Simplfied type: CM_FMT_TYPE8_A_SUB_B_MOD_CGenerated combiners:
        */

    {
        {0x00460703, 0x01000000, 0x02060004, 0x02000000}, // Simplified mux
            0x00167E83, 0x5FFFFC38,     // 64bit Mux
            2,  // number of stages
            DISABLE_ALPHA,
            MUX_ENV,        // Constant color
            0x00050006, 0x00000007, 0,  // Shade and specular color flags
            {0x00000000, 0x00000005}, // constant color texture flags
        {
            {SUB(T0,ENV), SKIP, 0, true},   // Stage 0
            {MULADD(CUR,ENVA,DIF), SKIP, 1, true},  // Stage 1
        }
    },
};

int noOfTwoStages = sizeof(twostages)/sizeof(GeneralCombinerInfo);

