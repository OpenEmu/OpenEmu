////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

switch
(
	(b.w[4] != b.w[0] && ((yuv5 - lut.yuv[b.w[0]]) & Lut::YUV_MASK) ? 0x01U : 0x0U) |
	(b.w[4] != b.w[1] && ((yuv5 - lut.yuv[b.w[1]]) & Lut::YUV_MASK) ? 0x02U : 0x0U) |
	(b.w[4] != b.w[2] && ((yuv5 - lut.yuv[b.w[2]]) & Lut::YUV_MASK) ? 0x04U : 0x0U) |
	(b.w[4] != b.w[3] && ((yuv5 - lut.yuv[b.w[3]]) & Lut::YUV_MASK) ? 0x08U : 0x0U) |
	(b.w[4] != b.w[5] && ((yuv5 - lut.yuv[b.w[5]]) & Lut::YUV_MASK) ? 0x10U : 0x0U) |
	(b.w[4] != b.w[6] && ((yuv5 - lut.yuv[b.w[6]]) & Lut::YUV_MASK) ? 0x20U : 0x0U) |
	(b.w[4] != b.w[7] && ((yuv5 - lut.yuv[b.w[7]]) & Lut::YUV_MASK) ? 0x40U : 0x0U) |
	(b.w[4] != b.w[8] && ((yuv5 - lut.yuv[b.w[8]]) & Lut::YUV_MASK) ? 0x80U : 0x0U)
)
#define PIXEL00_0     dst[0][0] = b.c[4];
#define PIXEL00_11    dst[0][0] = Interpolate1<R,G,B>( b.c[4], b.c[3] );
#define PIXEL00_12    dst[0][0] = Interpolate1<R,G,B>( b.c[4], b.c[1] );
#define PIXEL00_20    dst[0][0] = Interpolate2<R,G,B>( b.c[4], b.c[1], b.c[3] );
#define PIXEL00_50    dst[0][0] = Interpolate5<R,G,B>( b.c[1], b.c[3] );
#define PIXEL00_80    dst[0][0] = Interpolate8<R,G,B>( b.c[4], b.c[0] );
#define PIXEL00_81    dst[0][0] = Interpolate8<R,G,B>( b.c[4], b.c[3] );
#define PIXEL00_82    dst[0][0] = Interpolate8<R,G,B>( b.c[4], b.c[1] );
#define PIXEL01_0     dst[0][1] = b.c[4];
#define PIXEL01_10    dst[0][1] = Interpolate1<R,G,B>( b.c[4], b.c[0] );
#define PIXEL01_12    dst[0][1] = Interpolate1<R,G,B>( b.c[4], b.c[1] );
#define PIXEL01_14    dst[0][1] = Interpolate1<R,G,B>( b.c[1], b.c[4] );
#define PIXEL01_21    dst[0][1] = Interpolate2<R,G,B>( b.c[1], b.c[4], b.c[3] );
#define PIXEL01_31    dst[0][1] = Interpolate3<R,G,B>( b.c[4], b.c[3] );
#define PIXEL01_50    dst[0][1] = Interpolate5<R,G,B>( b.c[1], b.c[4] );
#define PIXEL01_60    dst[0][1] = Interpolate6<R,G,B>( b.c[4], b.c[1], b.c[3] );
#define PIXEL01_61    dst[0][1] = Interpolate6<R,G,B>( b.c[4], b.c[1], b.c[0] );
#define PIXEL01_82    dst[0][1] = Interpolate8<R,G,B>( b.c[4], b.c[1] );
#define PIXEL01_83    dst[0][1] = Interpolate8<R,G,B>( b.c[1], b.c[3] );
#define PIXEL02_0     dst[0][2] = b.c[4];
#define PIXEL02_10    dst[0][2] = Interpolate1<R,G,B>( b.c[4], b.c[2] );
#define PIXEL02_11    dst[0][2] = Interpolate1<R,G,B>( b.c[4], b.c[1] );
#define PIXEL02_13    dst[0][2] = Interpolate1<R,G,B>( b.c[1], b.c[4] );
#define PIXEL02_21    dst[0][2] = Interpolate2<R,G,B>( b.c[1], b.c[4], b.c[5] );
#define PIXEL02_32    dst[0][2] = Interpolate3<R,G,B>( b.c[4], b.c[5] );
#define PIXEL02_50    dst[0][2] = Interpolate5<R,G,B>( b.c[1], b.c[4] );
#define PIXEL02_60    dst[0][2] = Interpolate6<R,G,B>( b.c[4], b.c[1], b.c[5] );
#define PIXEL02_61    dst[0][2] = Interpolate6<R,G,B>( b.c[4], b.c[1], b.c[2] );
#define PIXEL02_81    dst[0][2] = Interpolate8<R,G,B>( b.c[4], b.c[1] );
#define PIXEL02_83    dst[0][2] = Interpolate8<R,G,B>( b.c[1], b.c[5] );
#define PIXEL03_0     dst[0][3] = b.c[4];
#define PIXEL03_11    dst[0][3] = Interpolate1<R,G,B>( b.c[4], b.c[1] );
#define PIXEL03_12    dst[0][3] = Interpolate1<R,G,B>( b.c[4], b.c[5] );
#define PIXEL03_20    dst[0][3] = Interpolate2<R,G,B>( b.c[4], b.c[1], b.c[5] );
#define PIXEL03_50    dst[0][3] = Interpolate5<R,G,B>( b.c[1], b.c[5] );
#define PIXEL03_80    dst[0][3] = Interpolate8<R,G,B>( b.c[4], b.c[2] );
#define PIXEL03_81    dst[0][3] = Interpolate8<R,G,B>( b.c[4], b.c[1] );
#define PIXEL03_82    dst[0][3] = Interpolate8<R,G,B>( b.c[4], b.c[5] );
#define PIXEL10_0     dst[1][0] = b.c[4];
#define PIXEL10_10    dst[1][0] = Interpolate1<R,G,B>( b.c[4], b.c[0] );
#define PIXEL10_11    dst[1][0] = Interpolate1<R,G,B>( b.c[4], b.c[3] );
#define PIXEL10_13    dst[1][0] = Interpolate1<R,G,B>( b.c[3], b.c[4] );
#define PIXEL10_21    dst[1][0] = Interpolate2<R,G,B>( b.c[3], b.c[4], b.c[1]);
#define PIXEL10_32    dst[1][0] = Interpolate3<R,G,B>( b.c[4], b.c[1] );
#define PIXEL10_50    dst[1][0] = Interpolate5<R,G,B>( b.c[3], b.c[4] );
#define PIXEL10_60    dst[1][0] = Interpolate6<R,G,B>( b.c[4], b.c[3], b.c[1] );
#define PIXEL10_61    dst[1][0] = Interpolate6<R,G,B>( b.c[4], b.c[3], b.c[0] );
#define PIXEL10_81    dst[1][0] = Interpolate8<R,G,B>( b.c[4], b.c[3] );
#define PIXEL10_83    dst[1][0] = Interpolate8<R,G,B>( b.c[3], b.c[1] );
#define PIXEL11_0     dst[1][1] = b.c[4];
#define PIXEL11_30    dst[1][1] = Interpolate3<R,G,B>( b.c[4], b.c[0] );
#define PIXEL11_31    dst[1][1] = Interpolate3<R,G,B>( b.c[4], b.c[3] );
#define PIXEL11_32    dst[1][1] = Interpolate3<R,G,B>( b.c[4], b.c[1] );
#define PIXEL11_70    dst[1][1] = Interpolate7<R,G,B>( b.c[4], b.c[3], b.c[1] );
#define PIXEL12_0     dst[1][2] = b.c[4];
#define PIXEL12_30    dst[1][2] = Interpolate3<R,G,B>( b.c[4], b.c[2] );
#define PIXEL12_31    dst[1][2] = Interpolate3<R,G,B>( b.c[4], b.c[1] );
#define PIXEL12_32    dst[1][2] = Interpolate3<R,G,B>( b.c[4], b.c[5] );
#define PIXEL12_70    dst[1][2] = Interpolate7<R,G,B>( b.c[4], b.c[5], b.c[1] );
#define PIXEL13_0     dst[1][3] = b.c[4];
#define PIXEL13_10    dst[1][3] = Interpolate1<R,G,B>( b.c[4], b.c[2] );
#define PIXEL13_12    dst[1][3] = Interpolate1<R,G,B>( b.c[4], b.c[5] );
#define PIXEL13_14    dst[1][3] = Interpolate1<R,G,B>( b.c[5], b.c[4] );
#define PIXEL13_21    dst[1][3] = Interpolate2<R,G,B>( b.c[5], b.c[4], b.c[1] );
#define PIXEL13_31    dst[1][3] = Interpolate3<R,G,B>( b.c[4], b.c[1] );
#define PIXEL13_50    dst[1][3] = Interpolate5<R,G,B>( b.c[5], b.c[4] );
#define PIXEL13_60    dst[1][3] = Interpolate6<R,G,B>( b.c[4], b.c[5], b.c[1] );
#define PIXEL13_61    dst[1][3] = Interpolate6<R,G,B>( b.c[4], b.c[5], b.c[2] );
#define PIXEL13_82    dst[1][3] = Interpolate8<R,G,B>( b.c[4], b.c[5] );
#define PIXEL13_83    dst[1][3] = Interpolate8<R,G,B>( b.c[5], b.c[1] );
#define PIXEL20_0     dst[2][0] = b.c[4];
#define PIXEL20_10    dst[2][0] = Interpolate1<R,G,B>( b.c[4], b.c[6] );
#define PIXEL20_12    dst[2][0] = Interpolate1<R,G,B>( b.c[4], b.c[3] );
#define PIXEL20_14    dst[2][0] = Interpolate1<R,G,B>( b.c[3], b.c[4] );
#define PIXEL20_21    dst[2][0] = Interpolate2<R,G,B>( b.c[3], b.c[4], b.c[7] );
#define PIXEL20_31    dst[2][0] = Interpolate3<R,G,B>( b.c[4], b.c[7] );
#define PIXEL20_50    dst[2][0] = Interpolate5<R,G,B>( b.c[3], b.c[4] );
#define PIXEL20_60    dst[2][0] = Interpolate6<R,G,B>( b.c[4], b.c[3], b.c[7] );
#define PIXEL20_61    dst[2][0] = Interpolate6<R,G,B>( b.c[4], b.c[3], b.c[6] );
#define PIXEL20_82    dst[2][0] = Interpolate8<R,G,B>( b.c[4], b.c[3] );
#define PIXEL20_83    dst[2][0] = Interpolate8<R,G,B>( b.c[3], b.c[7] );
#define PIXEL21_0     dst[2][1] = b.c[4];
#define PIXEL21_30    dst[2][1] = Interpolate3<R,G,B>( b.c[4], b.c[6] );
#define PIXEL21_31    dst[2][1] = Interpolate3<R,G,B>( b.c[4], b.c[7] );
#define PIXEL21_32    dst[2][1] = Interpolate3<R,G,B>( b.c[4], b.c[3] );
#define PIXEL21_70    dst[2][1] = Interpolate7<R,G,B>( b.c[4], b.c[3], b.c[7] );
#define PIXEL22_0     dst[2][2] = b.c[4];
#define PIXEL22_30    dst[2][2] = Interpolate3<R,G,B>( b.c[4], b.c[8] );
#define PIXEL22_31    dst[2][2] = Interpolate3<R,G,B>( b.c[4], b.c[5] );
#define PIXEL22_32    dst[2][2] = Interpolate3<R,G,B>( b.c[4], b.c[7] );
#define PIXEL22_70    dst[2][2] = Interpolate7<R,G,B>( b.c[4], b.c[5], b.c[7]);
#define PIXEL23_0     dst[2][3] = b.c[4];
#define PIXEL23_10    dst[2][3] = Interpolate1<R,G,B>( b.c[4], b.c[8]);
#define PIXEL23_11    dst[2][3] = Interpolate1<R,G,B>( b.c[4], b.c[5]);
#define PIXEL23_13    dst[2][3] = Interpolate1<R,G,B>( b.c[5], b.c[4]);
#define PIXEL23_21    dst[2][3] = Interpolate2<R,G,B>( b.c[5], b.c[4], b.c[7] );
#define PIXEL23_32    dst[2][3] = Interpolate3<R,G,B>( b.c[4], b.c[7] );
#define PIXEL23_50    dst[2][3] = Interpolate5<R,G,B>( b.c[5], b.c[4] );
#define PIXEL23_60    dst[2][3] = Interpolate6<R,G,B>( b.c[4], b.c[5], b.c[7] );
#define PIXEL23_61    dst[2][3] = Interpolate6<R,G,B>( b.c[4], b.c[5], b.c[8] );
#define PIXEL23_81    dst[2][3] = Interpolate8<R,G,B>( b.c[4], b.c[5] );
#define PIXEL23_83    dst[2][3] = Interpolate8<R,G,B>( b.c[5], b.c[7] );
#define PIXEL30_0     dst[3][0] = b.c[4];
#define PIXEL30_11    dst[3][0] = Interpolate1<R,G,B>( b.c[4], b.c[7] );
#define PIXEL30_12    dst[3][0] = Interpolate1<R,G,B>( b.c[4], b.c[3] );
#define PIXEL30_20    dst[3][0] = Interpolate2<R,G,B>( b.c[4], b.c[7], b.c[3] );
#define PIXEL30_50    dst[3][0] = Interpolate5<R,G,B>( b.c[7], b.c[3] );
#define PIXEL30_80    dst[3][0] = Interpolate8<R,G,B>( b.c[4], b.c[6] );
#define PIXEL30_81    dst[3][0] = Interpolate8<R,G,B>( b.c[4], b.c[7] );
#define PIXEL30_82    dst[3][0] = Interpolate8<R,G,B>( b.c[4], b.c[3] );
#define PIXEL31_0     dst[3][1] =  b.c[4];
#define PIXEL31_10    dst[3][1] = Interpolate1<R,G,B>( b.c[4], b.c[6] );
#define PIXEL31_11    dst[3][1] = Interpolate1<R,G,B>( b.c[4], b.c[7] );
#define PIXEL31_13    dst[3][1] = Interpolate1<R,G,B>( b.c[7], b.c[4] );
#define PIXEL31_21    dst[3][1] = Interpolate2<R,G,B>( b.c[7], b.c[4], b.c[3] );
#define PIXEL31_32    dst[3][1] = Interpolate3<R,G,B>( b.c[4], b.c[3] );
#define PIXEL31_50    dst[3][1] = Interpolate5<R,G,B>( b.c[7], b.c[4] );
#define PIXEL31_60    dst[3][1] = Interpolate6<R,G,B>( b.c[4], b.c[7], b.c[3] );
#define PIXEL31_61    dst[3][1] = Interpolate6<R,G,B>( b.c[4], b.c[7], b.c[6] );
#define PIXEL31_81    dst[3][1] = Interpolate8<R,G,B>( b.c[4], b.c[7] );
#define PIXEL31_83    dst[3][1] = Interpolate8<R,G,B>( b.c[7], b.c[3] );
#define PIXEL32_0     dst[3][2] = b.c[4];
#define PIXEL32_10    dst[3][2] = Interpolate1<R,G,B>( b.c[4], b.c[8] );
#define PIXEL32_12    dst[3][2] = Interpolate1<R,G,B>( b.c[4], b.c[7] );
#define PIXEL32_14    dst[3][2] = Interpolate1<R,G,B>( b.c[7], b.c[4] );
#define PIXEL32_21    dst[3][2] = Interpolate2<R,G,B>( b.c[7], b.c[4], b.c[5] );
#define PIXEL32_31    dst[3][2] = Interpolate3<R,G,B>( b.c[4], b.c[5] );
#define PIXEL32_50    dst[3][2] = Interpolate5<R,G,B>( b.c[7], b.c[4] );
#define PIXEL32_60    dst[3][2] = Interpolate6<R,G,B>( b.c[4], b.c[7], b.c[5] );
#define PIXEL32_61    dst[3][2] = Interpolate6<R,G,B>( b.c[4], b.c[7], b.c[8] );
#define PIXEL32_82    dst[3][2] = Interpolate8<R,G,B>( b.c[4], b.c[7] );
#define PIXEL32_83    dst[3][2] = Interpolate8<R,G,B>( b.c[7], b.c[5] );
#define PIXEL33_0     dst[3][3] = b.c[4];
#define PIXEL33_11    dst[3][3] = Interpolate1<R,G,B>( b.c[4], b.c[5] );
#define PIXEL33_12    dst[3][3] = Interpolate1<R,G,B>( b.c[4], b.c[7] );
#define PIXEL33_20    dst[3][3] = Interpolate2<R,G,B>( b.c[4], b.c[7], b.c[5] );
#define PIXEL33_50    dst[3][3] = Interpolate5<R,G,B>( b.c[7], b.c[5] );
#define PIXEL33_80    dst[3][3] = Interpolate8<R,G,B>( b.c[4], b.c[8] );
#define PIXEL33_81    dst[3][3] = Interpolate8<R,G,B>( b.c[4], b.c[5] );
#define PIXEL33_82    dst[3][3] = Interpolate8<R,G,B>( b.c[4], b.c[7] );
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

	case 2:
	case 34:
	case 130:
	case 162:

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

	case 16:
	case 17:
	case 48:
	case 49:

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

	case 64:
	case 65:
	case 68:
	case 69:

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

	case 8:
	case 12:
	case 136:
	case 140:

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

	case 3:
	case 35:
	case 131:
	case 163:

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

	case 6:
	case 38:
	case 134:
	case 166:

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

	case 20:
	case 21:
	case 52:
	case 53:

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

	case 144:
	case 145:
	case 176:
	case 177:

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

	case 192:
	case 193:
	case 196:
	case 197:

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

	case 96:
	case 97:
	case 100:
	case 101:

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

	case 40:
	case 44:
	case 168:
	case 172:

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

	case 9:
	case 13:
	case 137:
	case 141:

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

	case 18:
	case 50:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

	case 80:
	case 81:

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

		if (Diff( b.w[5], b.w[7] ))
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

	case 72:
	case 76:

		PIXEL00_80
		PIXEL01_61
		PIXEL02_60
		PIXEL03_20
		PIXEL10_10
		PIXEL11_30
		PIXEL12_70
		PIXEL13_60

		if (Diff( b.w[7], b.w[3] ))
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

	case 10:
	case 138:

		if (Diff( b.w[3], b.w[1] ))
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

	case 66:

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

	case 24:

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

	case 7:
	case 39:
	case 135:

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

	case 148:
	case 149:
	case 180:

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

	case 224:
	case 228:
	case 225:

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

	case 41:
	case 169:
	case 45:

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

	case 22:
	case 54:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

	case 208:
	case 209:

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

		if (Diff( b.w[5], b.w[7] ))
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

	case 104:
	case 108:

		PIXEL00_80
		PIXEL01_61
		PIXEL02_60
		PIXEL03_20
		PIXEL10_10
		PIXEL11_30
		PIXEL12_70
		PIXEL13_60

		if (Diff( b.w[7], b.w[3] ))
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

	case 11:
	case 139:

		if (Diff( b.w[3], b.w[1] ))
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

	case 19:
	case 51:

		if (Diff( b.w[1], b.w[5] ))
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

	case 146:
	case 178:
	{
		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

		PIXEL00_20
		PIXEL01_60
		PIXEL02_81

		if (Diff( b.w[5], b.w[7] ))
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

	case 112:
	case 113:

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

		if (Diff( b.w[5], b.w[7] ))
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

	case 200:
	case 204:

		PIXEL00_80
		PIXEL01_61
		PIXEL02_60
		PIXEL03_20
		PIXEL10_10
		PIXEL11_30
		PIXEL12_70
		PIXEL13_60

		if (Diff( b.w[7], b.w[3] ))
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

	case 73:
	case 77:

		if (Diff( b.w[7], b.w[3] ))
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

	case 42:
	case 170:

		if (Diff( b.w[3], b.w[1] ))
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

	case 14:
	case 142:

		if (Diff( b.w[3], b.w[1] ))
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

	case 67:

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

	case 70:

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

	case 28:

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

	case 152:

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

	case 194:

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

	case 98:

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

	case 56:

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

	case 25:

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

	case 26:
	case 31:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

	case 82:
	case 214:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 88:
	case 248:

		PIXEL00_80
		PIXEL01_61
		PIXEL02_61
		PIXEL03_80
		PIXEL10_10
		PIXEL11_30
		PIXEL12_30
		PIXEL13_10

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 74:
	case 107:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

	case 27:

		if (Diff( b.w[3], b.w[1] ))
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

	case 86:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

	case 216:

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

		if (Diff( b.w[5], b.w[7] ))
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

	case 106:

		PIXEL00_80
		PIXEL01_10
		PIXEL02_10
		PIXEL03_80
		PIXEL10_10
		PIXEL11_30
		PIXEL12_30
		PIXEL13_61

		if (Diff( b.w[7], b.w[3] ))
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

	case 30:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

	case 210:

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

		if (Diff( b.w[5], b.w[7] ))
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

	case 120:

		PIXEL00_80
		PIXEL01_61
		PIXEL02_61
		PIXEL03_80
		PIXEL10_10
		PIXEL11_30
		PIXEL12_30
		PIXEL13_10

		if (Diff( b.w[7], b.w[3] ))
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

	case 75:

		if (Diff( b.w[3], b.w[1] ))
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

	case 29:

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

	case 198:

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

	case 184:

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

	case 99:

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

	case 57:

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

	case 71:

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

	case 156:

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

	case 226:

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

	case 60:

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

	case 195:

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

	case 102:

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

	case 153:

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

	case 58:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

	case 83:

		PIXEL00_81
		PIXEL01_31

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 92:

		PIXEL00_80
		PIXEL01_61
		PIXEL02_81
		PIXEL03_81
		PIXEL10_10
		PIXEL11_30
		PIXEL12_31
		PIXEL13_31

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 202:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

	case 78:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

	case 154:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

	case 114:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 89:

		PIXEL00_82
		PIXEL01_82
		PIXEL02_61
		PIXEL03_80
		PIXEL10_32
		PIXEL11_32
		PIXEL12_30
		PIXEL13_10

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 90:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 55:
	case 23:

		if (Diff( b.w[1], b.w[5] ))
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

	case 182:
	case 150:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

	case 213:
	case 212:

		PIXEL00_20
		PIXEL01_60
		PIXEL02_81

		if (Diff( b.w[5], b.w[7] ))
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

	case 241:
	case 240:

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

		if (Diff( b.w[5], b.w[7] ))
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

	case 236:
	case 232:

		PIXEL00_80
		PIXEL01_61
		PIXEL02_60
		PIXEL03_20
		PIXEL10_10
		PIXEL11_30
		PIXEL12_70
		PIXEL13_60

		if (Diff( b.w[7], b.w[3] ))
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

	case 109:
	case 105:

		if (Diff( b.w[7], b.w[3] ))
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

	case 171:
	case 43:

		if (Diff( b.w[3], b.w[1] ))
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

	case 143:
	case 15:

		if (Diff( b.w[3], b.w[1] ))
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

	case 124:

		PIXEL00_80
		PIXEL01_61
		PIXEL02_81
		PIXEL03_81
		PIXEL10_10
		PIXEL11_30
		PIXEL12_31
		PIXEL13_31

		if (Diff( b.w[7], b.w[3] ))
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

	case 203:

		if (Diff( b.w[3], b.w[1] ))
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

	case 62:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

	case 211:

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

		if (Diff( b.w[5], b.w[7] ))
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

	case 118:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

	case 217:

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

		if (Diff( b.w[5], b.w[7] ))
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

	case 110:

		PIXEL00_80
		PIXEL01_10
		PIXEL02_32
		PIXEL03_82
		PIXEL10_10
		PIXEL11_30
		PIXEL12_32
		PIXEL13_82

		if (Diff( b.w[7], b.w[3] ))
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

	case 155:

		if (Diff( b.w[3], b.w[1] ))
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

	case 188:

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

	case 185:

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

	case 61:

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

	case 157:

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

	case 103:

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

	case 227:

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

	case 230:

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

	case 199:

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

	case 220:

		PIXEL00_80
		PIXEL01_61
		PIXEL02_81
		PIXEL03_81
		PIXEL10_10
		PIXEL11_30
		PIXEL12_31
		PIXEL13_31

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 158:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

	case 234:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

	case 242:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 59:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

	case 121:

		PIXEL00_82
		PIXEL01_82
		PIXEL02_61
		PIXEL03_80
		PIXEL10_32
		PIXEL11_32
		PIXEL12_30
		PIXEL13_10

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 87:

		PIXEL00_81
		PIXEL01_31

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 79:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

	case 122:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 94:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 218:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 91:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 229:

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

	case 167:

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

	case 173:

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

	case 181:

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

	case 186:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

	case 115:

		PIXEL00_81
		PIXEL01_31

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 93:

		PIXEL00_82
		PIXEL01_82
		PIXEL02_81
		PIXEL03_81
		PIXEL10_32
		PIXEL11_32
		PIXEL12_31
		PIXEL13_31

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 206:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

	case 205:
	case 201:

		PIXEL00_82
		PIXEL01_82
		PIXEL02_60
		PIXEL03_20
		PIXEL10_32
		PIXEL11_32
		PIXEL12_70
		PIXEL13_60

		if (Diff( b.w[7], b.w[3] ))
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

	case 174:
	case 46:

		if (Diff( b.w[3], b.w[1] ))
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

	case 179:
	case 147:

		PIXEL00_81
		PIXEL01_31

		if (Diff( b.w[1], b.w[5] ))
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

	case 117:
	case 116:

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

		if (Diff( b.w[5], b.w[7] ))
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

	case 189:

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

	case 231:

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

	case 126:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

	case 219:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 125:

		if (Diff( b.w[7], b.w[3] ))
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

	case 221:

		PIXEL00_82
		PIXEL01_82
		PIXEL02_81

		if (Diff( b.w[5], b.w[7] ))
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

	case 207:

		if (Diff( b.w[3], b.w[1] ))
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

	case 238:

		PIXEL00_80
		PIXEL01_10
		PIXEL02_32
		PIXEL03_82
		PIXEL10_10
		PIXEL11_30
		PIXEL12_32
		PIXEL13_82

		if (Diff( b.w[7], b.w[3] ))
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

	case 190:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

	case 187:

		if (Diff( b.w[3], b.w[1] ))
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

	case 243:

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

		if (Diff( b.w[5], b.w[7] ))
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

	case 119:

		if (Diff( b.w[1], b.w[5] ))
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

	case 237:
	case 233:

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

		if (Diff( b.w[7], b.w[3] ))
			PIXEL30_0
		else
			PIXEL30_20

		PIXEL31_0
		PIXEL32_31
		PIXEL33_81
		break;

	case 175:
	case 47:

		if (Diff( b.w[3], b.w[1] ))
			PIXEL00_0
		else
			PIXEL00_20

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

	case 183:
	case 151:

		PIXEL00_81
		PIXEL01_31
		PIXEL02_0

		if (Diff( b.w[1], b.w[5] ))
			PIXEL03_0
		else
			PIXEL03_20

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

	case 245:
	case 244:

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

		if (Diff( b.w[5], b.w[7] ))
			PIXEL33_0
		else
			PIXEL33_20
		break;

	case 250:

		PIXEL00_80
		PIXEL01_10
		PIXEL02_10
		PIXEL03_80
		PIXEL10_10
		PIXEL11_30
		PIXEL12_30
		PIXEL13_10

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 123:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

	case 95:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
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

	case 222:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

	case 252:

		PIXEL00_80
		PIXEL01_61
		PIXEL02_81
		PIXEL03_81
		PIXEL10_10
		PIXEL11_30
		PIXEL12_31
		PIXEL13_31

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
			PIXEL33_0
		else
			PIXEL33_20

		break;

	case 249:

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

		if (Diff( b.w[5], b.w[7] ))
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

		if (Diff( b.w[7], b.w[3] ))
			PIXEL30_0
		else
			PIXEL30_20

		PIXEL31_0
		break;

	case 235:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[7], b.w[3] ))
			PIXEL30_0
		else
			PIXEL30_20

		PIXEL31_0
		PIXEL32_31
		PIXEL33_81
		break;

	case 111:

		if (Diff( b.w[3], b.w[1] ))
			PIXEL00_0
		else
			PIXEL00_20

		PIXEL01_0
		PIXEL02_32
		PIXEL03_82
		PIXEL10_0
		PIXEL11_0
		PIXEL12_32
		PIXEL13_82

		if (Diff( b.w[7], b.w[3] ))
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

	case 63:

		if (Diff( b.w[3], b.w[1] ))
			PIXEL00_0
		else
			PIXEL00_20

		PIXEL01_0

		if (Diff( b.w[1], b.w[5] ))
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

	case 159:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
			PIXEL03_0
		else
			PIXEL03_20

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

	case 215:

		PIXEL00_81
		PIXEL01_31
		PIXEL02_0

		if (Diff( b.w[1], b.w[5] ))
			PIXEL03_0
		else
			PIXEL03_20

		PIXEL10_81
		PIXEL11_31
		PIXEL12_0
		PIXEL13_0
		PIXEL20_61
		PIXEL21_30
		PIXEL22_0

		if (Diff( b.w[5], b.w[7] ))
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

	case 246:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[5], b.w[7] ))
			PIXEL33_0
		else
			PIXEL33_20

		break;

	case 254:

		PIXEL00_80
		PIXEL01_10

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

		if (Diff( b.w[5], b.w[7] ))
			PIXEL33_0
		else
			PIXEL33_20

		break;

	case 253:

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

		if (Diff( b.w[7], b.w[3] ))
			PIXEL30_0
		else
			PIXEL30_20

		PIXEL31_0
		PIXEL32_0

		if (Diff( b.w[5], b.w[7] ))
			PIXEL33_0
		else
			PIXEL33_20

		break;

	case 251:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[5], b.w[7] ))
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

		if (Diff( b.w[7], b.w[3] ))
			PIXEL30_0
		else
			PIXEL30_20

		PIXEL31_0
		break;

	case 239:

		if (Diff( b.w[3], b.w[1] ))
			PIXEL00_0
		else
			PIXEL00_20

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

		if (Diff( b.w[7], b.w[3] ))
			PIXEL30_0
		else
			PIXEL30_20

		PIXEL31_0
		PIXEL32_31
		PIXEL33_81
		break;

	case 127:

		if (Diff( b.w[3], b.w[1] ))
			PIXEL00_0
		else
			PIXEL00_20

		PIXEL01_0

		if (Diff( b.w[1], b.w[5] ))
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

		if (Diff( b.w[7], b.w[3] ))
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

	case 191:

		if (Diff( b.w[3], b.w[1] ))
			PIXEL00_0
		else
			PIXEL00_20

		PIXEL01_0
		PIXEL02_0

		if (Diff( b.w[1], b.w[5] ))
			PIXEL03_0
		else
			PIXEL03_20

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

	case 223:

		if (Diff( b.w[3], b.w[1] ))
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

		if (Diff( b.w[1], b.w[5] ))
			PIXEL03_0
		else
			PIXEL03_20

		PIXEL11_0
		PIXEL12_0
		PIXEL13_0
		PIXEL20_10
		PIXEL21_30
		PIXEL22_0

		if (Diff( b.w[5], b.w[7] ))
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

	case 247:

		PIXEL00_81
		PIXEL01_31
		PIXEL02_0

		if (Diff( b.w[1], b.w[5] ))
			PIXEL03_0
		else
			PIXEL03_20

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

		if (Diff( b.w[5], b.w[7] ))
			PIXEL33_0
		else
			PIXEL33_20

		break;

	case 255:

		if (Diff( b.w[3], b.w[1] ))
			PIXEL00_0
		else
			PIXEL00_20

		PIXEL01_0
		PIXEL02_0

		if (Diff( b.w[1], b.w[5] ))
			PIXEL03_0
		else
			PIXEL03_20

		PIXEL10_0
		PIXEL11_0
		PIXEL12_0
		PIXEL13_0
		PIXEL20_0
		PIXEL21_0
		PIXEL22_0
		PIXEL23_0

		if (Diff( b.w[7], b.w[3] ))
			PIXEL30_0
		else
			PIXEL30_20

		PIXEL31_0
		PIXEL32_0

		if (Diff( b.w[5], b.w[7] ))
			PIXEL33_0
		else
			PIXEL33_20

		break;

	default: NST_UNREACHABLE();
}
#undef PIXEL00_0
#undef PIXEL00_11
#undef PIXEL00_12
#undef PIXEL00_20
#undef PIXEL00_50
#undef PIXEL00_80
#undef PIXEL00_81
#undef PIXEL00_82
#undef PIXEL01_0
#undef PIXEL01_10
#undef PIXEL01_12
#undef PIXEL01_14
#undef PIXEL01_21
#undef PIXEL01_31
#undef PIXEL01_50
#undef PIXEL01_60
#undef PIXEL01_61
#undef PIXEL01_82
#undef PIXEL01_83
#undef PIXEL02_0
#undef PIXEL02_10
#undef PIXEL02_11
#undef PIXEL02_13
#undef PIXEL02_21
#undef PIXEL02_32
#undef PIXEL02_50
#undef PIXEL02_60
#undef PIXEL02_61
#undef PIXEL02_81
#undef PIXEL02_83
#undef PIXEL03_0
#undef PIXEL03_11
#undef PIXEL03_12
#undef PIXEL03_20
#undef PIXEL03_50
#undef PIXEL03_80
#undef PIXEL03_81
#undef PIXEL03_82
#undef PIXEL10_0
#undef PIXEL10_10
#undef PIXEL10_11
#undef PIXEL10_13
#undef PIXEL10_21
#undef PIXEL10_32
#undef PIXEL10_50
#undef PIXEL10_60
#undef PIXEL10_61
#undef PIXEL10_81
#undef PIXEL10_83
#undef PIXEL11_0
#undef PIXEL11_30
#undef PIXEL11_31
#undef PIXEL11_32
#undef PIXEL11_70
#undef PIXEL12_0
#undef PIXEL12_30
#undef PIXEL12_31
#undef PIXEL12_32
#undef PIXEL12_70
#undef PIXEL13_0
#undef PIXEL13_10
#undef PIXEL13_12
#undef PIXEL13_14
#undef PIXEL13_21
#undef PIXEL13_31
#undef PIXEL13_50
#undef PIXEL13_60
#undef PIXEL13_61
#undef PIXEL13_82
#undef PIXEL13_83
#undef PIXEL20_0
#undef PIXEL20_10
#undef PIXEL20_12
#undef PIXEL20_14
#undef PIXEL20_21
#undef PIXEL20_31
#undef PIXEL20_50
#undef PIXEL20_60
#undef PIXEL20_61
#undef PIXEL20_82
#undef PIXEL20_83
#undef PIXEL21_0
#undef PIXEL21_30
#undef PIXEL21_31
#undef PIXEL21_32
#undef PIXEL21_70
#undef PIXEL22_0
#undef PIXEL22_30
#undef PIXEL22_31
#undef PIXEL22_32
#undef PIXEL22_70
#undef PIXEL23_0
#undef PIXEL23_10
#undef PIXEL23_11
#undef PIXEL23_13
#undef PIXEL23_21
#undef PIXEL23_32
#undef PIXEL23_50
#undef PIXEL23_60
#undef PIXEL23_61
#undef PIXEL23_81
#undef PIXEL23_83
#undef PIXEL30_0
#undef PIXEL30_11
#undef PIXEL30_12
#undef PIXEL30_20
#undef PIXEL30_50
#undef PIXEL30_80
#undef PIXEL30_81
#undef PIXEL30_82
#undef PIXEL31_0
#undef PIXEL31_10
#undef PIXEL31_11
#undef PIXEL31_13
#undef PIXEL31_21
#undef PIXEL31_32
#undef PIXEL31_50
#undef PIXEL31_60
#undef PIXEL31_61
#undef PIXEL31_81
#undef PIXEL31_83
#undef PIXEL32_0
#undef PIXEL32_10
#undef PIXEL32_12
#undef PIXEL32_14
#undef PIXEL32_21
#undef PIXEL32_31
#undef PIXEL32_50
#undef PIXEL32_60
#undef PIXEL32_61
#undef PIXEL32_82
#undef PIXEL32_83
#undef PIXEL33_0
#undef PIXEL33_11
#undef PIXEL33_12
#undef PIXEL33_20
#undef PIXEL33_50
#undef PIXEL33_80
#undef PIXEL33_81
#undef PIXEL33_82
