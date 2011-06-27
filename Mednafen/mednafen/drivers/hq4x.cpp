//hq4x filter demo program
//----------------------------------------------------------
//Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )

//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 2.1 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this program; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


/* Modified for usage in Mednafen */

#include "main.h"

#define HQXX_INTERNAL
#include "hqxx-common.h"

static int YUV1, YUV2;

inline void Interp1(unsigned char * pc, int c1, int c2)
{
  *((int*)pc) = (c1*3+c2) >> 2;
}

inline void Interp2(unsigned char * pc, int c1, int c2, int c3)
{
  *((int*)pc) = (c1*2+c2+c3) >> 2;
}

inline void Interp3(unsigned char * pc, int c1, int c2)
{
  //*((int*)pc) = (c1*7+c2)/8;

  *((int*)pc) = ((((c1 & 0x00FF00)*7 + (c2 & 0x00FF00) ) & 0x0007F800) +
                 (((c1 & 0xFF00FF)*7 + (c2 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
}

inline void Interp5(unsigned char * pc, int c1, int c2)
{
  *((int*)pc) = (c1+c2) >> 1;
}

inline void Interp6(unsigned char * pc, int c1, int c2, int c3)
{
  //*((int*)pc) = (c1*5+c2*2+c3)/8;

  *((int*)pc) = ((((c1 & 0x00FF00)*5 + (c2 & 0x00FF00)*2 + (c3 & 0x00FF00) ) & 0x0007F800) +
                 (((c1 & 0xFF00FF)*5 + (c2 & 0xFF00FF)*2 + (c3 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
}

inline void Interp7(unsigned char * pc, int c1, int c2, int c3)
{
  //*((int*)pc) = (c1*6+c2+c3)/8;

  *((int*)pc) = ((((c1 & 0x00FF00)*6 + (c2 & 0x00FF00) + (c3 & 0x00FF00) ) & 0x0007F800) +
                 (((c1 & 0xFF00FF)*6 + (c2 & 0xFF00FF) + (c3 & 0xFF00FF) ) & 0x07F807F8)) >> 3;
}

inline void Interp8(unsigned char * pc, int c1, int c2)
{
  //*((int*)pc) = (c1*5+c2*3)/8;

  *((int*)pc) = ((((c1 & 0x00FF00)*5 + (c2 & 0x00FF00)*3 ) & 0x0007F800) +
                 (((c1 & 0xFF00FF)*5 + (c2 & 0xFF00FF)*3 ) & 0x07F807F8)) >> 3;
}

#define PIXEL00_0     *((int*)(pOut)) = w[5];
#define PIXEL00_11    Interp1(pOut, w[5], w[4]);
#define PIXEL00_12    Interp1(pOut, w[5], w[2]);
#define PIXEL00_20    Interp2(pOut, w[5], w[2], w[4]);
#define PIXEL00_50    Interp5(pOut, w[2], w[4]);
#define PIXEL00_80    Interp8(pOut, w[5], w[1]);
#define PIXEL00_81    Interp8(pOut, w[5], w[4]);
#define PIXEL00_82    Interp8(pOut, w[5], w[2]);
#define PIXEL01_0     *((int*)(pOut+4)) = w[5];
#define PIXEL01_10    Interp1(pOut+4, w[5], w[1]);
#define PIXEL01_12    Interp1(pOut+4, w[5], w[2]);
#define PIXEL01_14    Interp1(pOut+4, w[2], w[5]);
#define PIXEL01_21    Interp2(pOut+4, w[2], w[5], w[4]);
#define PIXEL01_31    Interp3(pOut+4, w[5], w[4]);
#define PIXEL01_50    Interp5(pOut+4, w[2], w[5]);
#define PIXEL01_60    Interp6(pOut+4, w[5], w[2], w[4]);
#define PIXEL01_61    Interp6(pOut+4, w[5], w[2], w[1]);
#define PIXEL01_82    Interp8(pOut+4, w[5], w[2]);
#define PIXEL01_83    Interp8(pOut+4, w[2], w[4]);
#define PIXEL02_0     *((int*)(pOut+8)) = w[5];
#define PIXEL02_10    Interp1(pOut+8, w[5], w[3]);
#define PIXEL02_11    Interp1(pOut+8, w[5], w[2]);
#define PIXEL02_13    Interp1(pOut+8, w[2], w[5]);
#define PIXEL02_21    Interp2(pOut+8, w[2], w[5], w[6]);
#define PIXEL02_32    Interp3(pOut+8, w[5], w[6]);
#define PIXEL02_50    Interp5(pOut+8, w[2], w[5]);
#define PIXEL02_60    Interp6(pOut+8, w[5], w[2], w[6]);
#define PIXEL02_61    Interp6(pOut+8, w[5], w[2], w[3]);
#define PIXEL02_81    Interp8(pOut+8, w[5], w[2]);
#define PIXEL02_83    Interp8(pOut+8, w[2], w[6]);
#define PIXEL03_0     *((int*)(pOut+12)) = w[5];
#define PIXEL03_11    Interp1(pOut+12, w[5], w[2]);
#define PIXEL03_12    Interp1(pOut+12, w[5], w[6]);
#define PIXEL03_20    Interp2(pOut+12, w[5], w[2], w[6]);
#define PIXEL03_50    Interp5(pOut+12, w[2], w[6]);
#define PIXEL03_80    Interp8(pOut+12, w[5], w[3]);
#define PIXEL03_81    Interp8(pOut+12, w[5], w[2]);
#define PIXEL03_82    Interp8(pOut+12, w[5], w[6]);
#define PIXEL10_0     *((int*)(pOut+BpL)) = w[5];
#define PIXEL10_10    Interp1(pOut+BpL, w[5], w[1]);
#define PIXEL10_11    Interp1(pOut+BpL, w[5], w[4]);
#define PIXEL10_13    Interp1(pOut+BpL, w[4], w[5]);
#define PIXEL10_21    Interp2(pOut+BpL, w[4], w[5], w[2]);
#define PIXEL10_32    Interp3(pOut+BpL, w[5], w[2]);
#define PIXEL10_50    Interp5(pOut+BpL, w[4], w[5]);
#define PIXEL10_60    Interp6(pOut+BpL, w[5], w[4], w[2]);
#define PIXEL10_61    Interp6(pOut+BpL, w[5], w[4], w[1]);
#define PIXEL10_81    Interp8(pOut+BpL, w[5], w[4]);
#define PIXEL10_83    Interp8(pOut+BpL, w[4], w[2]);
#define PIXEL11_0     *((int*)(pOut+BpL+4)) = w[5];
#define PIXEL11_30    Interp3(pOut+BpL+4, w[5], w[1]);
#define PIXEL11_31    Interp3(pOut+BpL+4, w[5], w[4]);
#define PIXEL11_32    Interp3(pOut+BpL+4, w[5], w[2]);
#define PIXEL11_70    Interp7(pOut+BpL+4, w[5], w[4], w[2]);
#define PIXEL12_0     *((int*)(pOut+BpL+8)) = w[5];
#define PIXEL12_30    Interp3(pOut+BpL+8, w[5], w[3]);
#define PIXEL12_31    Interp3(pOut+BpL+8, w[5], w[2]);
#define PIXEL12_32    Interp3(pOut+BpL+8, w[5], w[6]);
#define PIXEL12_70    Interp7(pOut+BpL+8, w[5], w[6], w[2]);
#define PIXEL13_0     *((int*)(pOut+BpL+12)) = w[5];
#define PIXEL13_10    Interp1(pOut+BpL+12, w[5], w[3]);
#define PIXEL13_12    Interp1(pOut+BpL+12, w[5], w[6]);
#define PIXEL13_14    Interp1(pOut+BpL+12, w[6], w[5]);
#define PIXEL13_21    Interp2(pOut+BpL+12, w[6], w[5], w[2]);
#define PIXEL13_31    Interp3(pOut+BpL+12, w[5], w[2]);
#define PIXEL13_50    Interp5(pOut+BpL+12, w[6], w[5]);
#define PIXEL13_60    Interp6(pOut+BpL+12, w[5], w[6], w[2]);
#define PIXEL13_61    Interp6(pOut+BpL+12, w[5], w[6], w[3]);
#define PIXEL13_82    Interp8(pOut+BpL+12, w[5], w[6]);
#define PIXEL13_83    Interp8(pOut+BpL+12, w[6], w[2]);
#define PIXEL20_0     *((int*)(pOut+BpL+BpL)) = w[5];
#define PIXEL20_10    Interp1(pOut+BpL+BpL, w[5], w[7]);
#define PIXEL20_12    Interp1(pOut+BpL+BpL, w[5], w[4]);
#define PIXEL20_14    Interp1(pOut+BpL+BpL, w[4], w[5]);
#define PIXEL20_21    Interp2(pOut+BpL+BpL, w[4], w[5], w[8]);
#define PIXEL20_31    Interp3(pOut+BpL+BpL, w[5], w[8]);
#define PIXEL20_50    Interp5(pOut+BpL+BpL, w[4], w[5]);
#define PIXEL20_60    Interp6(pOut+BpL+BpL, w[5], w[4], w[8]);
#define PIXEL20_61    Interp6(pOut+BpL+BpL, w[5], w[4], w[7]);
#define PIXEL20_82    Interp8(pOut+BpL+BpL, w[5], w[4]);
#define PIXEL20_83    Interp8(pOut+BpL+BpL, w[4], w[8]);
#define PIXEL21_0     *((int*)(pOut+BpL+BpL+4)) = w[5];
#define PIXEL21_30    Interp3(pOut+BpL+BpL+4, w[5], w[7]);
#define PIXEL21_31    Interp3(pOut+BpL+BpL+4, w[5], w[8]);
#define PIXEL21_32    Interp3(pOut+BpL+BpL+4, w[5], w[4]);
#define PIXEL21_70    Interp7(pOut+BpL+BpL+4, w[5], w[4], w[8]);
#define PIXEL22_0     *((int*)(pOut+BpL+BpL+8)) = w[5];
#define PIXEL22_30    Interp3(pOut+BpL+BpL+8, w[5], w[9]);
#define PIXEL22_31    Interp3(pOut+BpL+BpL+8, w[5], w[6]);
#define PIXEL22_32    Interp3(pOut+BpL+BpL+8, w[5], w[8]);
#define PIXEL22_70    Interp7(pOut+BpL+BpL+8, w[5], w[6], w[8]);
#define PIXEL23_0     *((int*)(pOut+BpL+BpL+12)) = w[5];
#define PIXEL23_10    Interp1(pOut+BpL+BpL+12, w[5], w[9]);
#define PIXEL23_11    Interp1(pOut+BpL+BpL+12, w[5], w[6]);
#define PIXEL23_13    Interp1(pOut+BpL+BpL+12, w[6], w[5]);
#define PIXEL23_21    Interp2(pOut+BpL+BpL+12, w[6], w[5], w[8]);
#define PIXEL23_32    Interp3(pOut+BpL+BpL+12, w[5], w[8]);
#define PIXEL23_50    Interp5(pOut+BpL+BpL+12, w[6], w[5]);
#define PIXEL23_60    Interp6(pOut+BpL+BpL+12, w[5], w[6], w[8]);
#define PIXEL23_61    Interp6(pOut+BpL+BpL+12, w[5], w[6], w[9]);
#define PIXEL23_81    Interp8(pOut+BpL+BpL+12, w[5], w[6]);
#define PIXEL23_83    Interp8(pOut+BpL+BpL+12, w[6], w[8]);
#define PIXEL30_0     *((int*)(pOut+BpL+BpL+BpL)) = w[5];
#define PIXEL30_11    Interp1(pOut+BpL+BpL+BpL, w[5], w[8]);
#define PIXEL30_12    Interp1(pOut+BpL+BpL+BpL, w[5], w[4]);
#define PIXEL30_20    Interp2(pOut+BpL+BpL+BpL, w[5], w[8], w[4]);
#define PIXEL30_50    Interp5(pOut+BpL+BpL+BpL, w[8], w[4]);
#define PIXEL30_80    Interp8(pOut+BpL+BpL+BpL, w[5], w[7]);
#define PIXEL30_81    Interp8(pOut+BpL+BpL+BpL, w[5], w[8]);
#define PIXEL30_82    Interp8(pOut+BpL+BpL+BpL, w[5], w[4]);
#define PIXEL31_0     *((int*)(pOut+BpL+BpL+BpL+4)) = w[5];
#define PIXEL31_10    Interp1(pOut+BpL+BpL+BpL+4, w[5], w[7]);
#define PIXEL31_11    Interp1(pOut+BpL+BpL+BpL+4, w[5], w[8]);
#define PIXEL31_13    Interp1(pOut+BpL+BpL+BpL+4, w[8], w[5]);
#define PIXEL31_21    Interp2(pOut+BpL+BpL+BpL+4, w[8], w[5], w[4]);
#define PIXEL31_32    Interp3(pOut+BpL+BpL+BpL+4, w[5], w[4]);
#define PIXEL31_50    Interp5(pOut+BpL+BpL+BpL+4, w[8], w[5]);
#define PIXEL31_60    Interp6(pOut+BpL+BpL+BpL+4, w[5], w[8], w[4]);
#define PIXEL31_61    Interp6(pOut+BpL+BpL+BpL+4, w[5], w[8], w[7]);
#define PIXEL31_81    Interp8(pOut+BpL+BpL+BpL+4, w[5], w[8]);
#define PIXEL31_83    Interp8(pOut+BpL+BpL+BpL+4, w[8], w[4]);
#define PIXEL32_0     *((int*)(pOut+BpL+BpL+BpL+8)) = w[5];
#define PIXEL32_10    Interp1(pOut+BpL+BpL+BpL+8, w[5], w[9]);
#define PIXEL32_12    Interp1(pOut+BpL+BpL+BpL+8, w[5], w[8]);
#define PIXEL32_14    Interp1(pOut+BpL+BpL+BpL+8, w[8], w[5]);
#define PIXEL32_21    Interp2(pOut+BpL+BpL+BpL+8, w[8], w[5], w[6]);
#define PIXEL32_31    Interp3(pOut+BpL+BpL+BpL+8, w[5], w[6]);
#define PIXEL32_50    Interp5(pOut+BpL+BpL+BpL+8, w[8], w[5]);
#define PIXEL32_60    Interp6(pOut+BpL+BpL+BpL+8, w[5], w[8], w[6]);
#define PIXEL32_61    Interp6(pOut+BpL+BpL+BpL+8, w[5], w[8], w[9]);
#define PIXEL32_82    Interp8(pOut+BpL+BpL+BpL+8, w[5], w[8]);
#define PIXEL32_83    Interp8(pOut+BpL+BpL+BpL+8, w[8], w[6]);
#define PIXEL33_0     *((int*)(pOut+BpL+BpL+BpL+12)) = w[5];
#define PIXEL33_11    Interp1(pOut+BpL+BpL+BpL+12, w[5], w[6]);
#define PIXEL33_12    Interp1(pOut+BpL+BpL+BpL+12, w[5], w[8]);
#define PIXEL33_20    Interp2(pOut+BpL+BpL+BpL+12, w[5], w[8], w[6]);
#define PIXEL33_50    Interp5(pOut+BpL+BpL+BpL+12, w[8], w[6]);
#define PIXEL33_80    Interp8(pOut+BpL+BpL+BpL+12, w[5], w[9]);
#define PIXEL33_81    Interp8(pOut+BpL+BpL+BpL+12, w[5], w[6]);
#define PIXEL33_82    Interp8(pOut+BpL+BpL+BpL+12, w[5], w[8]);


inline bool Diff(unsigned int w1, unsigned int w2)
{
  YUV1 = hqxx_RGB_to_YUV(w1);
  YUV2 = hqxx_RGB_to_YUV(w2);
  return ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
           ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
           ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) );
}

void hq4x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int srcBpL, int BpL)
{
  int  i, j, k;
  int  prevline, nextline;
  int  w[10];

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
    if (j>0)      prevline = -srcBpL; else prevline = 0;
    if (j<Yres-1) nextline =  srcBpL; else nextline = 0;

    for (i=0; i<Xres; i++)
    {
      w[2] = *((unsigned int*)(pIn + prevline)) & 0xFCFCFC;
      w[5] = *((unsigned int*)pIn) & 0xFCFCFC;
      w[8] = *((unsigned int*)(pIn + nextline)) & 0xFCFCFC;

      if (i>0)
      {
        w[1] = *((unsigned int*)(pIn + prevline - sizeof(uint32))) & 0xFCFCFC;
        w[4] = *((unsigned int*)(pIn - sizeof(uint32))) & 0xFCFCFC;
        w[7] = *((unsigned int*)(pIn + nextline - sizeof(uint32))) & 0xFCFCFC;
      }
      else
      {
        w[1] = w[2];
        w[4] = w[5];
        w[7] = w[8];
      }

      if (i<Xres-1)
      {
        w[3] = *((unsigned int*)(pIn + prevline + sizeof(uint32))) & 0xFCFCFC;
        w[6] = *((unsigned int*)(pIn + sizeof(uint32))) & 0xFCFCFC;
        w[9] = *((unsigned int*)(pIn + nextline + sizeof(uint32))) & 0xFCFCFC;
      }
      else
      {
        w[3] = w[2];
        w[6] = w[5];
        w[9] = w[8];
      }

      int pattern = 0;
      int flag = 1;

      YUV1 = hqxx_RGB_to_YUV(w[5]);

      for (k=1; k<=9; k++)
      {
        if (k==5) continue;

        if ( w[k] != w[5] )
        {
          YUV2 = hqxx_RGB_to_YUV(w[k]);
          if ( ( abs((YUV1 & Ymask) - (YUV2 & Ymask)) > trY ) ||
               ( abs((YUV1 & Umask) - (YUV2 & Umask)) > trU ) ||
               ( abs((YUV1 & Vmask) - (YUV2 & Vmask)) > trV ) )
            pattern |= flag;
        }
        flag <<= 1;
      }

      switch (pattern)
      {
        case 0:
        case 1:
        case 4:
        case 32:
        case 128:
        case 5:
        case 132:
        case 160:
        case 33:
        case 129:
        case 36:
        case 133:
        case 164:
        case 161:
        case 37:
        case 165:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 2:
        case 34:
        case 130:
        case 162:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 16:
        case 17:
        case 48:
        case 49:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 64:
        case 65:
        case 68:
        case 69:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 8:
        case 12:
        case 136:
        case 140:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 3:
        case 35:
        case 131:
        case 163:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 6:
        case 38:
        case 134:
        case 166:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 20:
        case 21:
        case 52:
        case 53:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 144:
        case 145:
        case 176:
        case 177:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 192:
        case 193:
        case 196:
        case 197:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 96:
        case 97:
        case 100:
        case 101:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 40:
        case 44:
        case 168:
        case 172:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 9:
        case 13:
        case 137:
        case 141:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 18:
        case 50:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL12_0
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 80:
        case 81:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 72:
        case 76:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_50
            PIXEL21_0
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 10:
        case 138:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
            PIXEL11_0
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 66:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 24:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 7:
        case 39:
        case 135:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 148:
        case 149:
        case 180:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 224:
        case 228:
        case 225:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 41:
        case 169:
        case 45:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 22:
        case 54:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 208:
        case 209:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 104:
        case 108:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 11:
        case 139:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 19:
        case 51:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL00_81
            PIXEL01_31
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL00_12
            PIXEL01_14
            PIXEL02_83
            PIXEL03_50
            PIXEL12_70
            PIXEL13_21
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 146:
        case 178:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
            PIXEL23_32
            PIXEL33_82
          }
          else
          {
            PIXEL02_21
            PIXEL03_50
            PIXEL12_70
            PIXEL13_83
            PIXEL23_13
            PIXEL33_11
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          break;
        }
        case 84:
        case 85:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          if (Diff(w[6], w[8]))
          {
            PIXEL03_81
            PIXEL13_31
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL03_12
            PIXEL13_14
            PIXEL22_70
            PIXEL23_83
            PIXEL32_21
            PIXEL33_50
          }
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL20_61
          PIXEL21_30
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 112:
        case 113:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL30_82
            PIXEL31_32
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_70
            PIXEL23_21
            PIXEL30_11
            PIXEL31_13
            PIXEL32_83
            PIXEL33_50
          }
          break;
        }
        case 200:
        case 204:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
            PIXEL32_31
            PIXEL33_81
          }
          else
          {
            PIXEL20_21
            PIXEL21_70
            PIXEL30_50
            PIXEL31_83
            PIXEL32_14
            PIXEL33_12
          }
          PIXEL22_31
          PIXEL23_81
          break;
        }
        case 73:
        case 77:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL00_82
            PIXEL10_32
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL00_11
            PIXEL10_13
            PIXEL20_83
            PIXEL21_70
            PIXEL30_50
            PIXEL31_21
          }
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 42:
        case 170:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
            PIXEL20_31
            PIXEL30_81
          }
          else
          {
            PIXEL00_50
            PIXEL01_21
            PIXEL10_83
            PIXEL11_70
            PIXEL20_14
            PIXEL30_12
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 14:
        case 142:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL02_32
            PIXEL03_82
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_50
            PIXEL01_83
            PIXEL02_13
            PIXEL03_11
            PIXEL10_21
            PIXEL11_70
          }
          PIXEL12_32
          PIXEL13_82
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 67:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 70:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 28:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 152:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 194:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 98:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 56:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 25:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 26:
        case 31:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 82:
        case 214:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 88:
        case 248:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 74:
        case 107:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 27:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 86:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 216:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 106:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 30:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 210:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 120:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 75:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 29:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_61
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 198:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 184:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_61
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 99:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 57:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 71:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_30
          PIXEL23_61
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 156:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 226:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_61
          PIXEL11_30
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 60:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 195:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 102:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 153:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 58:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 83:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_61
          PIXEL21_30
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 92:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 202:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 78:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 154:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 114:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 89:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 90:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 55:
        case 23:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL00_81
            PIXEL01_31
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
          }
          else
          {
            PIXEL00_12
            PIXEL01_14
            PIXEL02_83
            PIXEL03_50
            PIXEL12_70
            PIXEL13_21
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_30
          PIXEL23_10
          PIXEL30_20
          PIXEL31_60
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 182:
        case 150:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
            PIXEL23_32
            PIXEL33_82
          }
          else
          {
            PIXEL02_21
            PIXEL03_50
            PIXEL12_70
            PIXEL13_83
            PIXEL23_13
            PIXEL33_11
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          break;
        }
        case 213:
        case 212:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          if (Diff(w[6], w[8]))
          {
            PIXEL03_81
            PIXEL13_31
            PIXEL22_0
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL03_12
            PIXEL13_14
            PIXEL22_70
            PIXEL23_83
            PIXEL32_21
            PIXEL33_50
          }
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL20_61
          PIXEL21_30
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 241:
        case 240:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_61
          PIXEL03_80
          PIXEL10_60
          PIXEL11_70
          PIXEL12_30
          PIXEL13_10
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_0
            PIXEL23_0
            PIXEL30_82
            PIXEL31_32
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL22_70
            PIXEL23_21
            PIXEL30_11
            PIXEL31_13
            PIXEL32_83
            PIXEL33_50
          }
          break;
        }
        case 236:
        case 232:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_60
          PIXEL03_20
          PIXEL10_10
          PIXEL11_30
          PIXEL12_70
          PIXEL13_60
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
            PIXEL32_31
            PIXEL33_81
          }
          else
          {
            PIXEL20_21
            PIXEL21_70
            PIXEL30_50
            PIXEL31_83
            PIXEL32_14
            PIXEL33_12
          }
          PIXEL22_31
          PIXEL23_81
          break;
        }
        case 109:
        case 105:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL00_82
            PIXEL10_32
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL00_11
            PIXEL10_13
            PIXEL20_83
            PIXEL21_70
            PIXEL30_50
            PIXEL31_21
          }
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 171:
        case 43:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
            PIXEL11_0
            PIXEL20_31
            PIXEL30_81
          }
          else
          {
            PIXEL00_50
            PIXEL01_21
            PIXEL10_83
            PIXEL11_70
            PIXEL20_14
            PIXEL30_12
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 143:
        case 15:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL02_32
            PIXEL03_82
            PIXEL10_0
            PIXEL11_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_83
            PIXEL02_13
            PIXEL03_11
            PIXEL10_21
            PIXEL11_70
          }
          PIXEL12_32
          PIXEL13_82
          PIXEL20_10
          PIXEL21_30
          PIXEL22_70
          PIXEL23_60
          PIXEL30_80
          PIXEL31_61
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 124:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 203:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_10
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 62:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 211:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_10
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 118:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_10
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 217:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 110:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_10
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 155:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 188:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 185:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 61:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 157:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 103:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_61
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 227:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_61
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 230:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_61
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 199:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_61
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 220:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 158:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 234:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_61
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 242:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 59:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL11_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 121:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 87:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL20_61
          PIXEL21_30
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 79:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 122:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 94:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL12_0
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 218:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 91:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL11_0
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 229:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_60
          PIXEL03_20
          PIXEL10_60
          PIXEL11_70
          PIXEL12_70
          PIXEL13_60
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 167:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_60
          PIXEL21_70
          PIXEL22_70
          PIXEL23_60
          PIXEL30_20
          PIXEL31_60
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 173:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 181:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 186:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 115:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 93:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          break;
        }
        case 206:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 205:
        case 201:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          if (Diff(w[8], w[4]))
          {
            PIXEL20_10
            PIXEL21_30
            PIXEL30_80
            PIXEL31_10
          }
          else
          {
            PIXEL20_12
            PIXEL21_0
            PIXEL30_20
            PIXEL31_11
          }
          PIXEL22_31
          PIXEL23_81
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 174:
        case 46:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_80
            PIXEL01_10
            PIXEL10_10
            PIXEL11_30
          }
          else
          {
            PIXEL00_20
            PIXEL01_12
            PIXEL10_11
            PIXEL11_0
          }
          PIXEL02_32
          PIXEL03_82
          PIXEL12_32
          PIXEL13_82
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 179:
        case 147:
        {
          PIXEL00_81
          PIXEL01_31
          if (Diff(w[2], w[6]))
          {
            PIXEL02_10
            PIXEL03_80
            PIXEL12_30
            PIXEL13_10
          }
          else
          {
            PIXEL02_11
            PIXEL03_20
            PIXEL12_0
            PIXEL13_12
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 117:
        case 116:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_30
            PIXEL23_10
            PIXEL32_10
            PIXEL33_80
          }
          else
          {
            PIXEL22_0
            PIXEL23_11
            PIXEL32_12
            PIXEL33_20
          }
          PIXEL30_82
          PIXEL31_32
          break;
        }
        case 189:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 231:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_32
          PIXEL03_82
          PIXEL10_81
          PIXEL11_31
          PIXEL12_32
          PIXEL13_82
          PIXEL20_82
          PIXEL21_32
          PIXEL22_31
          PIXEL23_81
          PIXEL30_82
          PIXEL31_32
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 126:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 219:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 125:
        {
          if (Diff(w[8], w[4]))
          {
            PIXEL00_82
            PIXEL10_32
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL00_11
            PIXEL10_13
            PIXEL20_83
            PIXEL21_70
            PIXEL30_50
            PIXEL31_21
          }
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 221:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          if (Diff(w[6], w[8]))
          {
            PIXEL03_81
            PIXEL13_31
            PIXEL22_0
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL03_12
            PIXEL13_14
            PIXEL22_70
            PIXEL23_83
            PIXEL32_21
            PIXEL33_50
          }
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL20_10
          PIXEL21_30
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 207:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL02_32
            PIXEL03_82
            PIXEL10_0
            PIXEL11_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_83
            PIXEL02_13
            PIXEL03_11
            PIXEL10_21
            PIXEL11_70
          }
          PIXEL12_32
          PIXEL13_82
          PIXEL20_10
          PIXEL21_30
          PIXEL22_31
          PIXEL23_81
          PIXEL30_80
          PIXEL31_10
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 238:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_32
          PIXEL03_82
          PIXEL10_10
          PIXEL11_30
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL21_0
            PIXEL30_0
            PIXEL31_0
            PIXEL32_31
            PIXEL33_81
          }
          else
          {
            PIXEL20_21
            PIXEL21_70
            PIXEL30_50
            PIXEL31_83
            PIXEL32_14
            PIXEL33_12
          }
          PIXEL22_31
          PIXEL23_81
          break;
        }
        case 190:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
            PIXEL23_32
            PIXEL33_82
          }
          else
          {
            PIXEL02_21
            PIXEL03_50
            PIXEL12_70
            PIXEL13_83
            PIXEL23_13
            PIXEL33_11
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          break;
        }
        case 187:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
            PIXEL11_0
            PIXEL20_31
            PIXEL30_81
          }
          else
          {
            PIXEL00_50
            PIXEL01_21
            PIXEL10_83
            PIXEL11_70
            PIXEL20_14
            PIXEL30_12
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL12_30
          PIXEL13_10
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 243:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_10
          PIXEL03_80
          PIXEL10_81
          PIXEL11_31
          PIXEL12_30
          PIXEL13_10
          PIXEL20_82
          PIXEL21_32
          if (Diff(w[6], w[8]))
          {
            PIXEL22_0
            PIXEL23_0
            PIXEL30_82
            PIXEL31_32
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL22_70
            PIXEL23_21
            PIXEL30_11
            PIXEL31_13
            PIXEL32_83
            PIXEL33_50
          }
          break;
        }
        case 119:
        {
          if (Diff(w[2], w[6]))
          {
            PIXEL00_81
            PIXEL01_31
            PIXEL02_0
            PIXEL03_0
            PIXEL12_0
            PIXEL13_0
          }
          else
          {
            PIXEL00_12
            PIXEL01_14
            PIXEL02_83
            PIXEL03_50
            PIXEL12_70
            PIXEL13_21
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL20_82
          PIXEL21_32
          PIXEL22_30
          PIXEL23_10
          PIXEL30_82
          PIXEL31_32
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 237:
        case 233:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_60
          PIXEL03_20
          PIXEL10_32
          PIXEL11_32
          PIXEL12_70
          PIXEL13_60
          PIXEL20_0
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 175:
        case 47:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_32
          PIXEL03_82
          PIXEL10_0
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          PIXEL20_31
          PIXEL21_31
          PIXEL22_70
          PIXEL23_60
          PIXEL30_81
          PIXEL31_81
          PIXEL32_60
          PIXEL33_20
          break;
        }
        case 183:
        case 151:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL13_0
          PIXEL20_60
          PIXEL21_70
          PIXEL22_32
          PIXEL23_32
          PIXEL30_20
          PIXEL31_60
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 245:
        case 244:
        {
          PIXEL00_20
          PIXEL01_60
          PIXEL02_81
          PIXEL03_81
          PIXEL10_60
          PIXEL11_70
          PIXEL12_31
          PIXEL13_31
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          PIXEL23_0
          PIXEL30_82
          PIXEL31_32
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 250:
        {
          PIXEL00_80
          PIXEL01_10
          PIXEL02_10
          PIXEL03_80
          PIXEL10_10
          PIXEL11_30
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          break;
        }
        case 123:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 95:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_30
          PIXEL23_10
          PIXEL30_80
          PIXEL31_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 222:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 252:
        {
          PIXEL00_80
          PIXEL01_61
          PIXEL02_81
          PIXEL03_81
          PIXEL10_10
          PIXEL11_30
          PIXEL12_31
          PIXEL13_31
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 249:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_61
          PIXEL03_80
          PIXEL10_32
          PIXEL11_32
          PIXEL12_30
          PIXEL13_10
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          break;
        }
        case 235:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_61
          PIXEL20_0
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 111:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_32
          PIXEL03_82
          PIXEL10_0
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_61
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 63:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_30
          PIXEL23_10
          PIXEL30_81
          PIXEL31_81
          PIXEL32_61
          PIXEL33_80
          break;
        }
        case 159:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_32
          PIXEL23_32
          PIXEL30_80
          PIXEL31_61
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 215:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL13_0
          PIXEL20_61
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 246:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_61
          PIXEL11_30
          PIXEL12_0
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          PIXEL23_0
          PIXEL30_82
          PIXEL31_32
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 254:
        {
          PIXEL00_80
          PIXEL01_10
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_10
          PIXEL11_30
          PIXEL12_0
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 253:
        {
          PIXEL00_82
          PIXEL01_82
          PIXEL02_81
          PIXEL03_81
          PIXEL10_32
          PIXEL11_32
          PIXEL12_31
          PIXEL13_31
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 251:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_10
          PIXEL03_80
          PIXEL11_0
          PIXEL12_30
          PIXEL13_10
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          break;
        }
        case 239:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_32
          PIXEL03_82
          PIXEL10_0
          PIXEL11_0
          PIXEL12_32
          PIXEL13_82
          PIXEL20_0
          PIXEL21_0
          PIXEL22_31
          PIXEL23_81
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_31
          PIXEL33_81
          break;
        }
        case 127:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          if (Diff(w[2], w[6]))
          {
            PIXEL02_0
            PIXEL03_0
            PIXEL13_0
          }
          else
          {
            PIXEL02_50
            PIXEL03_50
            PIXEL13_50
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          if (Diff(w[8], w[4]))
          {
            PIXEL20_0
            PIXEL30_0
            PIXEL31_0
          }
          else
          {
            PIXEL20_50
            PIXEL30_50
            PIXEL31_50
          }
          PIXEL21_0
          PIXEL22_30
          PIXEL23_10
          PIXEL32_10
          PIXEL33_80
          break;
        }
        case 191:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_31
          PIXEL21_31
          PIXEL22_32
          PIXEL23_32
          PIXEL30_81
          PIXEL31_81
          PIXEL32_82
          PIXEL33_82
          break;
        }
        case 223:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
            PIXEL01_0
            PIXEL10_0
          }
          else
          {
            PIXEL00_50
            PIXEL01_50
            PIXEL10_50
          }
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_10
          PIXEL21_30
          PIXEL22_0
          if (Diff(w[6], w[8]))
          {
            PIXEL23_0
            PIXEL32_0
            PIXEL33_0
          }
          else
          {
            PIXEL23_50
            PIXEL32_50
            PIXEL33_50
          }
          PIXEL30_80
          PIXEL31_10
          break;
        }
        case 247:
        {
          PIXEL00_81
          PIXEL01_31
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_81
          PIXEL11_31
          PIXEL12_0
          PIXEL13_0
          PIXEL20_82
          PIXEL21_32
          PIXEL22_0
          PIXEL23_0
          PIXEL30_82
          PIXEL31_32
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
        case 255:
        {
          if (Diff(w[4], w[2]))
          {
            PIXEL00_0
          }
          else
          {
            PIXEL00_20
          }
          PIXEL01_0
          PIXEL02_0
          if (Diff(w[2], w[6]))
          {
            PIXEL03_0
          }
          else
          {
            PIXEL03_20
          }
          PIXEL10_0
          PIXEL11_0
          PIXEL12_0
          PIXEL13_0
          PIXEL20_0
          PIXEL21_0
          PIXEL22_0
          PIXEL23_0
          if (Diff(w[8], w[4]))
          {
            PIXEL30_0
          }
          else
          {
            PIXEL30_20
          }
          PIXEL31_0
          PIXEL32_0
          if (Diff(w[6], w[8]))
          {
            PIXEL33_0
          }
          else
          {
            PIXEL33_20
          }
          break;
        }
      }
      pIn += sizeof(uint32);
      pOut+=16;
    }
    pOut += BpL * 4 - Xres * 4 * sizeof(uint32);
    pIn += srcBpL - Xres * sizeof(uint32);
  }
}
