/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - TextureFilters_hq4x.cpp                                 *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C)  2003 MaxSt ( maxst@hiend3d.com )                       *
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

#include <stdlib.h>

#include "typedefs.h"

static int   RGBtoYUV[4096];
//#define RGB32toYUV(val) (RGBtoYUV[((val&0x00FF0000)>>20)+((val&0x0000FF00)>>12)+((val&0x000000FF)>>4)])
inline int RGB32toYUV(uint32 val)
{
    int a,r,g,b,Y,u,v;
    //r = (val&0x00FF0000)>>16;
    //g = (val&0x0000FF00)>>8;
    //b = (val&0x000000FF);
    a = (val&0xFF000000);
    r = (val&0x00FF0000)>>16;
    g = (val&0x0000FF00)>>8;
    b = (val&0x000000FF);
    //r = (val&0x00F80000)>>16;
    //g = (val&0x0000FC00)>>8;
    //b = (val&0x000000F8);
    Y = (r + g + b) >> 2;
    u = 128 + ((r - b) >> 2);
    v = 128 + ((-r + 2*g -b)>>3);
    return a + (Y<<16) + (u<<8) + v;
}
#define RGB16toYUV(val) (RGBtoYUV[(val&0x0FFF)])
static int   YUV1, YUV2;
const  int   Amask = 0xFF000000;
const  int   Ymask = 0x00FF0000;
const  int   Umask = 0x0000FF00;
const  int   Vmask = 0x000000FF;
const  int   trA   = 0x20000000;
const  int   trY   = 0x00300000;
const  int   trU   = 0x00000700;
const  int   trV   = 0x00000006;
//const  int   trU   = 0x00001800;
//const  int   trV   = 0x00000018;

#define INTERP_16_MASK_1_3(v) ((v)&0x0F0F)
#define INTERP_16_MASK_SHIFT_2_4(v) (((v)&0xF0F0)>>4)
#define INTERP_16_MASK_SHIFTBACK_2_4(v) ((INTERP_16_MASK_1_3(v))<<4)

inline void hq4x_Interp1_16(unsigned char * pc, uint16 p1, uint16 p2)
{
    *((uint16*)pc) = INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*3 + INTERP_16_MASK_1_3(p2)) / 4)
        | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*3 + INTERP_16_MASK_SHIFT_2_4(p2)) / 4 );
}

inline void hq4x_Interp2_16(unsigned char * pc, uint16 p1, uint16 p2, uint16 p3)
{
    *((uint16*)pc) =  INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*2 + INTERP_16_MASK_1_3(p2) + INTERP_16_MASK_1_3(p3)) / 4)
        | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*2 + INTERP_16_MASK_SHIFT_2_4(p2) + INTERP_16_MASK_SHIFT_2_4(p3)) / 4);
}

inline void hq4x_Interp3_16(unsigned char * pc, uint16 p1, uint16 p2)
{
    *((uint16*)pc) =  INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*7 + INTERP_16_MASK_1_3(p2)) / 8)
        | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*7 + INTERP_16_MASK_SHIFT_2_4(p2)) / 8);
}

inline void hq4x_Interp5_16(unsigned char * pc, uint16 p1, uint16 p2)
{
    *((uint16*)pc) =  INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1) + INTERP_16_MASK_1_3(p2)) / 2)
        | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1) + INTERP_16_MASK_SHIFT_2_4(p2)) / 2);
}

inline void hq4x_Interp6_16(unsigned char * pc, uint16 p1, uint16 p2, uint16 p3)
{
    *((uint16*)pc) =  INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*5 + INTERP_16_MASK_1_3(p2)*2 + INTERP_16_MASK_1_3(p3)*1) / 8)
        | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*5 + INTERP_16_MASK_SHIFT_2_4(p2)*2 + INTERP_16_MASK_SHIFT_2_4(p3)*1) / 8);
}

inline void hq4x_Interp7_16(unsigned char * pc, uint16 p1, uint16 p2, uint16 p3)
{
    *((uint16*)pc) =   INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*6 + INTERP_16_MASK_1_3(p2) + INTERP_16_MASK_1_3(p3)) / 8)
        | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*6 + INTERP_16_MASK_SHIFT_2_4(p2) + INTERP_16_MASK_SHIFT_2_4(p3)) / 8);
}

inline void hq4x_Interp8_16(unsigned char * pc, uint16 p1, uint16 p2)
{
    //*((int*)pc) = (c1*5+c2*3)/8;
    *((uint16*)pc) =   INTERP_16_MASK_1_3((INTERP_16_MASK_1_3(p1)*5 + INTERP_16_MASK_1_3(p2)*3) / 8)
        | INTERP_16_MASK_SHIFTBACK_2_4((INTERP_16_MASK_SHIFT_2_4(p1)*5 + INTERP_16_MASK_SHIFT_2_4(p2)*3) / 8);
}

#define INTERP_32_MASK_1_3(v) ((v)&0x00FF00FF)
#define INTERP_32_MASK_SHIFT_2_4(v) (((v)&0xFF00FF00)>>8)
#define INTERP_32_MASK_SHIFTBACK_2_4(v) (((INTERP_32_MASK_1_3(v))<<8))

inline void hq4x_Interp1_32(unsigned char * pc, uint32 p1, uint32 p2)
{
    *((uint32*)pc) = INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*3 + INTERP_32_MASK_1_3(p2)) / 4)
        | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*3 + INTERP_32_MASK_SHIFT_2_4(p2)) / 4 );
}

inline void hq4x_Interp2_32(unsigned char * pc, uint32 p1, uint32 p2, uint32 p3)
{
    *((uint32*)pc) =  INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*2 + INTERP_32_MASK_1_3(p2) + INTERP_32_MASK_1_3(p3)) / 4)
        | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*2 + INTERP_32_MASK_SHIFT_2_4(p2) + INTERP_32_MASK_SHIFT_2_4(p3)) / 4);
}

inline void hq4x_Interp3_32(unsigned char * pc, uint32 p1, uint32 p2)
{
    *((uint32*)pc) =  INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*7 + INTERP_32_MASK_1_3(p2)) / 8)
        | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*7 + INTERP_32_MASK_SHIFT_2_4(p2)) / 8);
}

inline void hq4x_Interp5_32(unsigned char * pc, uint32 p1, uint32 p2)
{
    *((uint32*)pc) =  INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1) + INTERP_32_MASK_1_3(p2)) / 2)
        | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1) + INTERP_32_MASK_SHIFT_2_4(p2)) / 2);
}

inline void hq4x_Interp6_32(unsigned char * pc, uint32 p1, uint32 p2, uint32 p3)
{
    *((uint32*)pc) =  INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*5 + INTERP_32_MASK_1_3(p2)*2 + INTERP_32_MASK_1_3(p3)*1) / 8)
        | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*5 + INTERP_32_MASK_SHIFT_2_4(p2)*2 + INTERP_32_MASK_SHIFT_2_4(p3)*1) / 8);
}

inline void hq4x_Interp7_32(unsigned char * pc, uint32 p1, uint32 p2, uint32 p3)
{
    *((uint32*)pc) =   INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*6 + INTERP_32_MASK_1_3(p2) + INTERP_32_MASK_1_3(p3)) / 8)
        | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*6 + INTERP_32_MASK_SHIFT_2_4(p2) + INTERP_32_MASK_SHIFT_2_4(p3)) / 8);
}

inline void hq4x_Interp8_32(unsigned char * pc, uint32 p1, uint32 p2)
{
    //*((int*)pc) = (c1*5+c2*3)/8;
    *((uint32*)pc) =   INTERP_32_MASK_1_3((INTERP_32_MASK_1_3(p1)*5 + INTERP_32_MASK_1_3(p2)*3) / 8)
        | INTERP_32_MASK_SHIFTBACK_2_4((INTERP_32_MASK_SHIFT_2_4(p1)*5 + INTERP_32_MASK_SHIFT_2_4(p2)*3) / 8);
}

#define PIXEL00_0     *((int*)(pOut)) = c[5];
#define PIXEL00_11    hq4x_Interp1(pOut, c[5], c[4]);
#define PIXEL00_12    hq4x_Interp1(pOut, c[5], c[2]);
#define PIXEL00_20    hq4x_Interp2(pOut, c[5], c[2], c[4]);
#define PIXEL00_50    hq4x_Interp5(pOut, c[2], c[4]);
#define PIXEL00_80    hq4x_Interp8(pOut, c[5], c[1]);
#define PIXEL00_81    hq4x_Interp8(pOut, c[5], c[4]);
#define PIXEL00_82    hq4x_Interp8(pOut, c[5], c[2]);
#define PIXEL01_0     *((int*)(pOut+BPP)) = c[5];
#define PIXEL01_10    hq4x_Interp1(pOut+BPP, c[5], c[1]);
#define PIXEL01_12    hq4x_Interp1(pOut+BPP, c[5], c[2]);
#define PIXEL01_14    hq4x_Interp1(pOut+BPP, c[2], c[5]);
#define PIXEL01_21    hq4x_Interp2(pOut+BPP, c[2], c[5], c[4]);
#define PIXEL01_31    hq4x_Interp3(pOut+BPP, c[5], c[4]);
#define PIXEL01_50    hq4x_Interp5(pOut+BPP, c[2], c[5]);
#define PIXEL01_60    hq4x_Interp6(pOut+BPP, c[5], c[2], c[4]);
#define PIXEL01_61    hq4x_Interp6(pOut+BPP, c[5], c[2], c[1]);
#define PIXEL01_82    hq4x_Interp8(pOut+BPP, c[5], c[2]);
#define PIXEL01_83    hq4x_Interp8(pOut+BPP, c[2], c[4]);
#define PIXEL02_0     *((int*)(pOut+BPP2)) = c[5];
#define PIXEL02_10    hq4x_Interp1(pOut+BPP2, c[5], c[3]);
#define PIXEL02_11    hq4x_Interp1(pOut+BPP2, c[5], c[2]);
#define PIXEL02_13    hq4x_Interp1(pOut+BPP2, c[2], c[5]);
#define PIXEL02_21    hq4x_Interp2(pOut+BPP2, c[2], c[5], c[6]);
#define PIXEL02_32    hq4x_Interp3(pOut+BPP2, c[5], c[6]);
#define PIXEL02_50    hq4x_Interp5(pOut+BPP2, c[2], c[5]);
#define PIXEL02_60    hq4x_Interp6(pOut+BPP2, c[5], c[2], c[6]);
#define PIXEL02_61    hq4x_Interp6(pOut+BPP2, c[5], c[2], c[3]);
#define PIXEL02_81    hq4x_Interp8(pOut+BPP2, c[5], c[2]);
#define PIXEL02_83    hq4x_Interp8(pOut+BPP2, c[2], c[6]);
#define PIXEL03_0     *((int*)(pOut+BPP3)) = c[5];
#define PIXEL03_11    hq4x_Interp1(pOut+BPP3, c[5], c[2]);
#define PIXEL03_12    hq4x_Interp1(pOut+BPP3, c[5], c[6]);
#define PIXEL03_20    hq4x_Interp2(pOut+BPP3, c[5], c[2], c[6]);
#define PIXEL03_50    hq4x_Interp5(pOut+BPP3, c[2], c[6]);
#define PIXEL03_80    hq4x_Interp8(pOut+BPP3, c[5], c[3]);
#define PIXEL03_81    hq4x_Interp8(pOut+BPP3, c[5], c[2]);
#define PIXEL03_82    hq4x_Interp8(pOut+BPP3, c[5], c[6]);
#define PIXEL10_0     *((int*)(pOut+BpL)) = c[5];
#define PIXEL10_10    hq4x_Interp1(pOut+BpL, c[5], c[1]);
#define PIXEL10_11    hq4x_Interp1(pOut+BpL, c[5], c[4]);
#define PIXEL10_13    hq4x_Interp1(pOut+BpL, c[4], c[5]);
#define PIXEL10_21    hq4x_Interp2(pOut+BpL, c[4], c[5], c[2]);
#define PIXEL10_32    hq4x_Interp3(pOut+BpL, c[5], c[2]);
#define PIXEL10_50    hq4x_Interp5(pOut+BpL, c[4], c[5]);
#define PIXEL10_60    hq4x_Interp6(pOut+BpL, c[5], c[4], c[2]);
#define PIXEL10_61    hq4x_Interp6(pOut+BpL, c[5], c[4], c[1]);
#define PIXEL10_81    hq4x_Interp8(pOut+BpL, c[5], c[4]);
#define PIXEL10_83    hq4x_Interp8(pOut+BpL, c[4], c[2]);
#define PIXEL11_0     *((int*)(pOut+BpL+BPP)) = c[5];
#define PIXEL11_30    hq4x_Interp3(pOut+BpL+BPP, c[5], c[1]);
#define PIXEL11_31    hq4x_Interp3(pOut+BpL+BPP, c[5], c[4]);
#define PIXEL11_32    hq4x_Interp3(pOut+BpL+BPP, c[5], c[2]);
#define PIXEL11_70    hq4x_Interp7(pOut+BpL+BPP, c[5], c[4], c[2]);
#define PIXEL12_0     *((int*)(pOut+BpL+BPP2)) = c[5];
#define PIXEL12_30    hq4x_Interp3(pOut+BpL+BPP2, c[5], c[3]);
#define PIXEL12_31    hq4x_Interp3(pOut+BpL+BPP2, c[5], c[2]);
#define PIXEL12_32    hq4x_Interp3(pOut+BpL+BPP2, c[5], c[6]);
#define PIXEL12_70    hq4x_Interp7(pOut+BpL+BPP2, c[5], c[6], c[2]);
#define PIXEL13_0     *((int*)(pOut+BpL+BPP3)) = c[5];
#define PIXEL13_10    hq4x_Interp1(pOut+BpL+BPP3, c[5], c[3]);
#define PIXEL13_12    hq4x_Interp1(pOut+BpL+BPP3, c[5], c[6]);
#define PIXEL13_14    hq4x_Interp1(pOut+BpL+BPP3, c[6], c[5]);
#define PIXEL13_21    hq4x_Interp2(pOut+BpL+BPP3, c[6], c[5], c[2]);
#define PIXEL13_31    hq4x_Interp3(pOut+BpL+BPP3, c[5], c[2]);
#define PIXEL13_50    hq4x_Interp5(pOut+BpL+BPP3, c[6], c[5]);
#define PIXEL13_60    hq4x_Interp6(pOut+BpL+BPP3, c[5], c[6], c[2]);
#define PIXEL13_61    hq4x_Interp6(pOut+BpL+BPP3, c[5], c[6], c[3]);
#define PIXEL13_82    hq4x_Interp8(pOut+BpL+BPP3, c[5], c[6]);
#define PIXEL13_83    hq4x_Interp8(pOut+BpL+BPP3, c[6], c[2]);
#define PIXEL20_0     *((int*)(pOut+BpL+BpL)) = c[5];
#define PIXEL20_10    hq4x_Interp1(pOut+BpL+BpL, c[5], c[7]);
#define PIXEL20_12    hq4x_Interp1(pOut+BpL+BpL, c[5], c[4]);
#define PIXEL20_14    hq4x_Interp1(pOut+BpL+BpL, c[4], c[5]);
#define PIXEL20_21    hq4x_Interp2(pOut+BpL+BpL, c[4], c[5], c[8]);
#define PIXEL20_31    hq4x_Interp3(pOut+BpL+BpL, c[5], c[8]);
#define PIXEL20_50    hq4x_Interp5(pOut+BpL+BpL, c[4], c[5]);
#define PIXEL20_60    hq4x_Interp6(pOut+BpL+BpL, c[5], c[4], c[8]);
#define PIXEL20_61    hq4x_Interp6(pOut+BpL+BpL, c[5], c[4], c[7]);
#define PIXEL20_82    hq4x_Interp8(pOut+BpL+BpL, c[5], c[4]);
#define PIXEL20_83    hq4x_Interp8(pOut+BpL+BpL, c[4], c[8]);
#define PIXEL21_0     *((int*)(pOut+BpL+BpL+BPP)) = c[5];
#define PIXEL21_30    hq4x_Interp3(pOut+BpL+BpL+BPP, c[5], c[7]);
#define PIXEL21_31    hq4x_Interp3(pOut+BpL+BpL+BPP, c[5], c[8]);
#define PIXEL21_32    hq4x_Interp3(pOut+BpL+BpL+BPP, c[5], c[4]);
#define PIXEL21_70    hq4x_Interp7(pOut+BpL+BpL+BPP, c[5], c[4], c[8]);
#define PIXEL22_0     *((int*)(pOut+BpL+BpL+BPP2)) = c[5];
#define PIXEL22_30    hq4x_Interp3(pOut+BpL+BpL+BPP2, c[5], c[9]);
#define PIXEL22_31    hq4x_Interp3(pOut+BpL+BpL+BPP2, c[5], c[6]);
#define PIXEL22_32    hq4x_Interp3(pOut+BpL+BpL+BPP2, c[5], c[8]);
#define PIXEL22_70    hq4x_Interp7(pOut+BpL+BpL+BPP2, c[5], c[6], c[8]);
#define PIXEL23_0     *((int*)(pOut+BpL+BpL+BPP3)) = c[5];
#define PIXEL23_10    hq4x_Interp1(pOut+BpL+BpL+BPP3, c[5], c[9]);
#define PIXEL23_11    hq4x_Interp1(pOut+BpL+BpL+BPP3, c[5], c[6]);
#define PIXEL23_13    hq4x_Interp1(pOut+BpL+BpL+BPP3, c[6], c[5]);
#define PIXEL23_21    hq4x_Interp2(pOut+BpL+BpL+BPP3, c[6], c[5], c[8]);
#define PIXEL23_32    hq4x_Interp3(pOut+BpL+BpL+BPP3, c[5], c[8]);
#define PIXEL23_50    hq4x_Interp5(pOut+BpL+BpL+BPP3, c[6], c[5]);
#define PIXEL23_60    hq4x_Interp6(pOut+BpL+BpL+BPP3, c[5], c[6], c[8]);
#define PIXEL23_61    hq4x_Interp6(pOut+BpL+BpL+BPP3, c[5], c[6], c[9]);
#define PIXEL23_81    hq4x_Interp8(pOut+BpL+BpL+BPP3, c[5], c[6]);
#define PIXEL23_83    hq4x_Interp8(pOut+BpL+BpL+BPP3, c[6], c[8]);
#define PIXEL30_0     *((int*)(pOut+BpL+BpL+BpL)) = c[5];
#define PIXEL30_11    hq4x_Interp1(pOut+BpL+BpL+BpL, c[5], c[8]);
#define PIXEL30_12    hq4x_Interp1(pOut+BpL+BpL+BpL, c[5], c[4]);
#define PIXEL30_20    hq4x_Interp2(pOut+BpL+BpL+BpL, c[5], c[8], c[4]);
#define PIXEL30_50    hq4x_Interp5(pOut+BpL+BpL+BpL, c[8], c[4]);
#define PIXEL30_80    hq4x_Interp8(pOut+BpL+BpL+BpL, c[5], c[7]);
#define PIXEL30_81    hq4x_Interp8(pOut+BpL+BpL+BpL, c[5], c[8]);
#define PIXEL30_82    hq4x_Interp8(pOut+BpL+BpL+BpL, c[5], c[4]);
#define PIXEL31_0     *((int*)(pOut+BpL+BpL+BpL+BPP)) = c[5];
#define PIXEL31_10    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP, c[5], c[7]);
#define PIXEL31_11    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP, c[5], c[8]);
#define PIXEL31_13    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP, c[8], c[5]);
#define PIXEL31_21    hq4x_Interp2(pOut+BpL+BpL+BpL+BPP, c[8], c[5], c[4]);
#define PIXEL31_32    hq4x_Interp3(pOut+BpL+BpL+BpL+BPP, c[5], c[4]);
#define PIXEL31_50    hq4x_Interp5(pOut+BpL+BpL+BpL+BPP, c[8], c[5]);
#define PIXEL31_60    hq4x_Interp6(pOut+BpL+BpL+BpL+BPP, c[5], c[8], c[4]);
#define PIXEL31_61    hq4x_Interp6(pOut+BpL+BpL+BpL+BPP, c[5], c[8], c[7]);
#define PIXEL31_81    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP, c[5], c[8]);
#define PIXEL31_83    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP, c[8], c[4]);
#define PIXEL32_0     *((int*)(pOut+BpL+BpL+BpL+BPP2)) = c[5];
#define PIXEL32_10    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP2, c[5], c[9]);
#define PIXEL32_12    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP2, c[5], c[8]);
#define PIXEL32_14    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP2, c[8], c[5]);
#define PIXEL32_21    hq4x_Interp2(pOut+BpL+BpL+BpL+BPP2, c[8], c[5], c[6]);
#define PIXEL32_31    hq4x_Interp3(pOut+BpL+BpL+BpL+BPP2, c[5], c[6]);
#define PIXEL32_50    hq4x_Interp5(pOut+BpL+BpL+BpL+BPP2, c[8], c[5]);
#define PIXEL32_60    hq4x_Interp6(pOut+BpL+BpL+BpL+BPP2, c[5], c[8], c[6]);
#define PIXEL32_61    hq4x_Interp6(pOut+BpL+BpL+BpL+BPP2, c[5], c[8], c[9]);
#define PIXEL32_82    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP2, c[5], c[8]);
#define PIXEL32_83    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP2, c[8], c[6]);
#define PIXEL33_0     *((int*)(pOut+BpL+BpL+BpL+BPP3)) = c[5];
#define PIXEL33_11    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP3, c[5], c[6]);
#define PIXEL33_12    hq4x_Interp1(pOut+BpL+BpL+BpL+BPP3, c[5], c[8]);
#define PIXEL33_20    hq4x_Interp2(pOut+BpL+BpL+BpL+BPP3, c[5], c[8], c[6]);
#define PIXEL33_50    hq4x_Interp5(pOut+BpL+BpL+BpL+BPP3, c[8], c[6]);
#define PIXEL33_80    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP3, c[5], c[9]);
#define PIXEL33_81    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP3, c[5], c[6]);
#define PIXEL33_82    hq4x_Interp8(pOut+BpL+BpL+BpL+BPP3, c[5], c[8]);



inline bool Diff_16(uint16 w1, uint16 w2)
{
    YUV1 = RGB16toYUV(w1);
    YUV2 = RGB16toYUV(w2);
    return ( ( abs((YUV1 & Amask) - (YUV2 & Amask)) > trA ) ||
        ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
        ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
        ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) );
}
inline bool Diff_32(uint32 w1, uint32 w2)
{
    YUV1 = RGB32toYUV(w1);
    YUV2 = RGB32toYUV(w2);
    return ( ( abs((YUV1 & Amask) - (YUV2 & Amask)) > trA ) ||
        ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
        ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
        ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) );
}

void hq4x_16( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL )
{
#define hq4x_Interp1 hq4x_Interp1_16
#define hq4x_Interp2 hq4x_Interp2_16
#define hq4x_Interp3 hq4x_Interp3_16
#define hq4x_Interp4 hq4x_Interp4_16
#define hq4x_Interp5 hq4x_Interp5_16
#define hq4x_Interp6 hq4x_Interp6_16
#define hq4x_Interp7 hq4x_Interp7_16
#define hq4x_Interp8 hq4x_Interp8_16
#define Diff Diff_16
#define BPP   2
#define BPP2  4
#define BPP3  6

    int  i, j, k;
    int  prevline, nextline;
    uint16  w[10];
    uint16  c[10];

    //   +----+----+----+
    //   |    |    |    |
    //   | w1 | w2 | w3 |
    //   +----+----+----+
    //   |    |    |    |
    //   | w4 | w5 | w6 |
    //   +----+----+----+
    //   |    |    |    |
    //   | w7 | w8 | w9 |
    //   +----+----+----+

    for (j=0; j<Yres; j++)
    {
        if (j>0)      prevline = -SrcPPL*2; else prevline = 0;
        if (j<Yres-1) nextline =  SrcPPL*2; else nextline = 0;

        for (i=0; i<Xres; i++)
        {
            w[2] = *((uint16*)(pIn + prevline));
            w[5] = *((uint16*)pIn);
            w[8] = *((uint16*)(pIn + nextline));

            if (i>0)
            {
                w[1] = *((uint16*)(pIn + prevline - 2));
                w[4] = *((uint16*)(pIn - 2));
                w[7] = *((uint16*)(pIn + nextline - 2));
            }
            else
            {
                w[1] = w[2];
                w[4] = w[5];
                w[7] = w[8];
            }

            if (i<Xres-1)
            {
                w[3] = *((uint16*)(pIn + prevline + 2));
                w[6] = *((uint16*)(pIn + 2));
                w[9] = *((uint16*)(pIn + nextline + 2));
            }
            else
            {
                w[3] = w[2];
                w[6] = w[5];
                w[9] = w[8];
            }

            int pattern = 0;
            int flag = 1;

            YUV1 = RGB16toYUV(w[5]);

            for (k=1; k<=9; k++)
            {
                if (k==5) continue;

                if ( w[k] != w[5] )
                {
                    YUV2 = RGB16toYUV(w[k]);
                    if ( ( abs((YUV1 & Amask) - (YUV2 & Amask)) > trA ) ||
                        ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
                        ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
                        ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
                        pattern |= flag;
                }
                flag <<= 1;
            }

            for (k=1; k<=9; k++)
                c[k] = w[k];

#include "TextureFilters_hq4x.h"

            pIn+=2;
            pOut+=8;
        }
        pIn += 2*(SrcPPL-Xres);
        pOut+= 8*(SrcPPL-Xres);
        pOut+=BpL;
        pOut+=BpL;
        pOut+=BpL;
    }

#undef BPP
#undef BPP2
#undef BPP3
#undef Diff
#undef hq4x_Interp1
#undef hq4x_Interp2
#undef hq4x_Interp3
#undef hq4x_Interp4
#undef hq4x_Interp5
#undef hq4x_Interp6
#undef hq4x_Interp7
#undef hq4x_Interp8
}

void hq4x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int SrcPPL, int BpL )
{
#define hq4x_Interp1 hq4x_Interp1_32
#define hq4x_Interp2 hq4x_Interp2_32
#define hq4x_Interp3 hq4x_Interp3_32
#define hq4x_Interp4 hq4x_Interp4_32
#define hq4x_Interp5 hq4x_Interp5_32
#define hq4x_Interp6 hq4x_Interp6_32
#define hq4x_Interp7 hq4x_Interp7_32
#define hq4x_Interp8 hq4x_Interp8_32
#define Diff Diff_32
#define BPP  4
#define BPP2  8
#define BPP3  12

    int  i, j, k;
    int  prevline, nextline;
    uint32  w[10];
    uint32  c[10];

    //   +----+----+----+
    //   |    |    |    |
    //   | w1 | w2 | w3 |
    //   +----+----+----+
    //   |    |    |    |
    //   | w4 | w5 | w6 |
    //   +----+----+----+
    //   |    |    |    |
    //   | w7 | w8 | w9 |
    //   +----+----+----+

    for (j=0; j<Yres; j++)
    {
        if (j>0)      prevline = -SrcPPL*4; else prevline = 0;
        if (j<Yres-1) nextline =  SrcPPL*4; else nextline = 0;

        for (i=0; i<Xres; i++)
        {
            w[2] = *((uint32*)(pIn + prevline));
            w[5] = *((uint32*)pIn);
            w[8] = *((uint32*)(pIn + nextline));

            if (i>0)
            {
                w[1] = *((uint32*)(pIn + prevline - 4));
                w[4] = *((uint32*)(pIn - 4));
                w[7] = *((uint32*)(pIn + nextline - 4));
            }
            else
            {
                w[1] = w[2];
                w[4] = w[5];
                w[7] = w[8];
            }

            if (i<Xres-1)
            {
                w[3] = *((uint32*)(pIn + prevline + 4));
                w[6] = *((uint32*)(pIn + 4));
                w[9] = *((uint32*)(pIn + nextline + 4));
            }
            else
            {
                w[3] = w[2];
                w[6] = w[5];
                w[9] = w[8];
            }

            int pattern = 0;
            int flag = 1;

            YUV1 = RGB32toYUV(w[5]);

            for (k=1; k<=9; k++)
            {
                if (k==5) continue;

                if ( w[k] != w[5] )
                {
                    YUV2 = RGB32toYUV(w[k]);
                    if ( ( abs((YUV1 & Amask) - (YUV2 & Amask)) > trA ) ||
                        ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
                        ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
                        ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
                        pattern |= flag;
                }
                flag <<= 1;
            }

            for (k=1; k<=9; k++)
                c[k] = w[k];

#include "TextureFilters_hq4x.h"

            pIn+=4;
            pOut+=16;
        }

        pIn += 4*(SrcPPL-Xres);
        pOut+= 16*(SrcPPL-Xres);
        pOut+=BpL;
        pOut+=BpL;
        pOut+=BpL;
    }

#undef BPP
#undef BPP2
#undef BPP3
#undef Diff
#undef hq4x_Interp1
#undef hq4x_Interp2
#undef hq4x_Interp3
#undef hq4x_Interp4
#undef hq4x_Interp5
#undef hq4x_Interp6
#undef hq4x_Interp7
#undef hq4x_Interp8
}

void hq4x_InitLUTs(void)
{
    static bool done = false;
    int i, j, k, r, g, b, Y, u, v;

    if( !done )
    {
        for (i=0; i<16; i++)
        {
            for (j=0; j<16; j++)
            {
                for (k=0; k<16; k++)
                {
                    r = i << 4;
                    g = j << 4;
                    b = k << 4;
                    Y = (r + g + b) >> 2;
                    u = 128 + ((r - b) >> 2);
                    v = 128 + ((-r + 2*g -b)>>3);
                    RGBtoYUV[ (i << 8) + (j << 4) + k ] = (Y<<16) + (u<<8) + v;
                }
            }
        }
        done = true;
    }
}

