/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   Copyright (C) 2003 MaxSt                                              *
 *   maxst@hiend3d.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "maxsthq2x.h"
#include "filterinfo.h"
#include <cstring>

static /*inline*/ unsigned long Interp1(const unsigned long c1, const unsigned long c2) {
	const unsigned long lowbits = ((c1 & 0x030303) * 3 + (c2 & 0x030303)) & 0x030303;

	return (c1 * 3 + c2 - lowbits) >> 2;
}

static /*inline*/ unsigned long Interp2(const unsigned long c1, const unsigned long c2, const unsigned long c3) {
	const unsigned long lowbits = ((c1 * 2 & 0x020202) + (c2 & 0x030303) + (c3 & 0x030303)) & 0x030303;

	return (c1 * 2 + c2 + c3 - lowbits) >> 2;
}

static /*inline*/ unsigned long Interp6(const unsigned long c1, const unsigned long c2, const unsigned long c3) {
	const unsigned long lowbits = ((c1 & 0x070707) * 5 + (c2 * 2 & 0x060606) + (c3 & 0x070707)) & 0x070707;

	return ((c1 * 5 + c2 * 2 + c3) - lowbits) >> 3;
}

static /*inline*/ unsigned long Interp7(const unsigned long c1, const unsigned long c2, const unsigned long c3) {
	const unsigned long lowbits = ((c1 & 0x070707) * 6 + (c2 & 0x070707) + (c3 & 0x070707)) & 0x070707;

	return ((c1 * 6 + c2 + c3) - lowbits) >> 3;
}

static /*inline*/ unsigned long Interp9(unsigned long c1, const unsigned long c2, const unsigned long c3) {
	const unsigned lowbits = ((c1 * 2 & 0x070707) + ((c2 & 0x070707) + (c3 & 0x070707)) * 3) & 0x070707;

	return (c1 * 2 + (c2 + c3) * 3 - lowbits) >> 3;
}

static /*inline*/ unsigned long Interp10(const unsigned long c1, const unsigned long c2, const unsigned long c3) {
	const unsigned lowbits = ((c1 & 0x0F0F0F) * 14 + (c2 & 0x0F0F0F) + (c3 & 0x0F0F0F)) & 0x0F0F0F;

	return (c1 * 14 + c2 + c3 - lowbits) >> 4;
}

#define PIXEL00_0     *pOut = w[5];
#define PIXEL00_10    *pOut = Interp1(w[5], w[1]);
#define PIXEL00_11    *pOut = Interp1(w[5], w[4]);
#define PIXEL00_12    *pOut = Interp1(w[5], w[2]);
#define PIXEL00_20    *pOut = Interp2(w[5], w[4], w[2]);
#define PIXEL00_21    *pOut = Interp2(w[5], w[1], w[2]);
#define PIXEL00_22    *pOut = Interp2(w[5], w[1], w[4]);
#define PIXEL00_60    *pOut = Interp6(w[5], w[2], w[4]);
#define PIXEL00_61    *pOut = Interp6(w[5], w[4], w[2]);
#define PIXEL00_70    *pOut = Interp7(w[5], w[4], w[2]);
#define PIXEL00_90    *pOut = Interp9(w[5], w[4], w[2]);
#define PIXEL00_100   *pOut = Interp10(w[5], w[4], w[2]);
#define PIXEL01_0     *(pOut+1) = w[5];
#define PIXEL01_10    *(pOut+1) = Interp1(w[5], w[3]);
#define PIXEL01_11    *(pOut+1) = Interp1(w[5], w[2]);
#define PIXEL01_12    *(pOut+1) = Interp1(w[5], w[6]);
#define PIXEL01_20    *(pOut+1) = Interp2(w[5], w[2], w[6]);
#define PIXEL01_21    *(pOut+1) = Interp2(w[5], w[3], w[6]);
#define PIXEL01_22    *(pOut+1) = Interp2(w[5], w[3], w[2]);
#define PIXEL01_60    *(pOut+1) = Interp6(w[5], w[6], w[2]);
#define PIXEL01_61    *(pOut+1) = Interp6(w[5], w[2], w[6]);
#define PIXEL01_70    *(pOut+1) = Interp7(w[5], w[2], w[6]);
#define PIXEL01_90    *(pOut+1) = Interp9(w[5], w[2], w[6]);
#define PIXEL01_100   *(pOut+1) = Interp10(w[5], w[2], w[6]);
#define PIXEL10_0     *(pOut+dstPitch) = w[5];
#define PIXEL10_10    *(pOut+dstPitch) = Interp1(w[5], w[7]);
#define PIXEL10_11    *(pOut+dstPitch) = Interp1(w[5], w[8]);
#define PIXEL10_12    *(pOut+dstPitch) = Interp1(w[5], w[4]);
#define PIXEL10_20    *(pOut+dstPitch) = Interp2(w[5], w[8], w[4]);
#define PIXEL10_21    *(pOut+dstPitch) = Interp2(w[5], w[7], w[4]);
#define PIXEL10_22    *(pOut+dstPitch) = Interp2(w[5], w[7], w[8]);
#define PIXEL10_60    *(pOut+dstPitch) = Interp6(w[5], w[4], w[8]);
#define PIXEL10_61    *(pOut+dstPitch) = Interp6(w[5], w[8], w[4]);
#define PIXEL10_70    *(pOut+dstPitch) = Interp7(w[5], w[8], w[4]);
#define PIXEL10_90    *(pOut+dstPitch) = Interp9(w[5], w[8], w[4]);
#define PIXEL10_100   *(pOut+dstPitch) = Interp10(w[5], w[8], w[4]);
#define PIXEL11_0     *(pOut+dstPitch+1) = w[5];
#define PIXEL11_10    *(pOut+dstPitch+1) = Interp1(w[5], w[9]);
#define PIXEL11_11    *(pOut+dstPitch+1) = Interp1(w[5], w[6]);
#define PIXEL11_12    *(pOut+dstPitch+1) = Interp1(w[5], w[8]);
#define PIXEL11_20    *(pOut+dstPitch+1) = Interp2(w[5], w[6], w[8]);
#define PIXEL11_21    *(pOut+dstPitch+1) = Interp2(w[5], w[9], w[8]);
#define PIXEL11_22    *(pOut+dstPitch+1) = Interp2(w[5], w[9], w[6]);
#define PIXEL11_60    *(pOut+dstPitch+1) = Interp6(w[5], w[8], w[6]);
#define PIXEL11_61    *(pOut+dstPitch+1) = Interp6(w[5], w[6], w[8]);
#define PIXEL11_70    *(pOut+dstPitch+1) = Interp7(w[5], w[6], w[8]);
#define PIXEL11_90    *(pOut+dstPitch+1) = Interp9(w[5], w[6], w[8]);
#define PIXEL11_100   *(pOut+dstPitch+1) = Interp10(w[5], w[6], w[8]);

static /*inline*/ bool Diff(const unsigned long w1, const unsigned long w2) {
	const unsigned rdiff = (w1 >> 16) - (w2 >> 16);
	const unsigned gdiff = (w1 >> 8 & 0xFF) - (w2 >> 8 & 0xFF);
	const unsigned bdiff = (w1 & 0xFF) - (w2 & 0xFF);

	return rdiff + gdiff + bdiff + 0xC0U > 0xC0U * 2    ||
		rdiff - bdiff + 0x1CU > 0x1CU * 2            ||
		gdiff * 2 - rdiff - bdiff + 0x30U > 0x30U * 2;
}

static void filter(Gambatte::uint_least32_t *pOut, const unsigned dstPitch,
		const Gambatte::uint_least32_t *pIn, const unsigned Xres, const unsigned Yres)
{
	unsigned long w[10];

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

	for (unsigned j = 0; j < Yres; j++) {
		const unsigned prevline = j > 0        ? Xres : 0;
		const unsigned nextline = j < Yres - 1 ? Xres : 0;

		for (unsigned i = 0; i < Xres; i++) {
			w[2] = *(pIn - prevline);
			w[5] = *(pIn);
			w[8] = *(pIn + nextline);

			if (i > 0) {
				w[1] = *(pIn - prevline - 1);
				w[4] = *(pIn - 1);
				w[7] = *(pIn + nextline - 1);
			} else {
				w[1] = w[2];
				w[4] = w[5];
				w[7] = w[8];
			}

			if (i < Xres - 1) {
				w[3] = *(pIn - prevline + 1);
				w[6] = *(pIn + 1);
				w[9] = *(pIn + nextline + 1);
			} else {
				w[3] = w[2];
				w[6] = w[5];
				w[9] = w[8];
			}

			unsigned pattern = 0;

			{
				unsigned flag = 1;

				const unsigned r1 = w[5] >> 16;
				const unsigned g1 = w[5] >> 8 & 0xFF;
				const unsigned b1 = w[5] & 0xFF;

				for (unsigned k = 1; k < 10; ++k) {
					if (k == 5) continue;

					if (w[k] != w[5]) {
						const unsigned rdiff = r1 - (w[k] >> 16);
						const unsigned gdiff = g1 - (w[k] >> 8 & 0xFF);
						const unsigned bdiff = b1 - (w[k] & 0xFF);

						if (rdiff + gdiff + bdiff + 0xC0U > 0xC0U * 2  ||
						    rdiff - bdiff + 0x1CU > 0x1CU * 2          ||
						    gdiff * 2 - rdiff - bdiff + 0x30U > 0x30U * 2)
							pattern |= flag;
					}

					flag <<= 1;
				}
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
				PIXEL01_20
				PIXEL10_20
				PIXEL11_20
				break;
				}
				case 2:
				case 34:
				case 130:
				case 162:
				{
				PIXEL00_22
				PIXEL01_21
				PIXEL10_20
				PIXEL11_20
				break;
				}
				case 16:
				case 17:
				case 48:
				case 49:
				{
				PIXEL00_20
				PIXEL01_22
				PIXEL10_20
				PIXEL11_21
				break;
				}
				case 64:
				case 65:
				case 68:
				case 69:
				{
				PIXEL00_20
				PIXEL01_20
				PIXEL10_21
				PIXEL11_22
				break;
				}
				case 8:
				case 12:
				case 136:
				case 140:
				{
				PIXEL00_21
				PIXEL01_20
				PIXEL10_22
				PIXEL11_20
				break;
				}
				case 3:
				case 35:
				case 131:
				case 163:
				{
				PIXEL00_11
				PIXEL01_21
				PIXEL10_20
				PIXEL11_20
				break;
				}
				case 6:
				case 38:
				case 134:
				case 166:
				{
				PIXEL00_22
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
				}
				case 20:
				case 21:
				case 52:
				case 53:
				{
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_21
				break;
				}
				case 144:
				case 145:
				case 176:
				case 177:
				{
				PIXEL00_20
				PIXEL01_22
				PIXEL10_20
				PIXEL11_12
				break;
				}
				case 192:
				case 193:
				case 196:
				case 197:
				{
				PIXEL00_20
				PIXEL01_20
				PIXEL10_21
				PIXEL11_11
				break;
				}
				case 96:
				case 97:
				case 100:
				case 101:
				{
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_22
				break;
				}
				case 40:
				case 44:
				case 168:
				case 172:
				{
				PIXEL00_21
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
				}
				case 9:
				case 13:
				case 137:
				case 141:
				{
				PIXEL00_12
				PIXEL01_20
				PIXEL10_22
				PIXEL11_20
				break;
				}
				case 18:
				case 50:
				{
				PIXEL00_22
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_20
				PIXEL11_21
				break;
				}
				case 80:
				case 81:
				{
				PIXEL00_20
				PIXEL01_22
				PIXEL10_21
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 72:
				case 76:
				{
				PIXEL00_21
				PIXEL01_20
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_22
				break;
				}
				case 10:
				case 138:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_22
				PIXEL11_20
				break;
				}
				case 66:
				{
				PIXEL00_22
				PIXEL01_21
				PIXEL10_21
				PIXEL11_22
				break;
				}
				case 24:
				{
				PIXEL00_21
				PIXEL01_22
				PIXEL10_22
				PIXEL11_21
				break;
				}
				case 7:
				case 39:
				case 135:
				{
				PIXEL00_11
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
				}
				case 148:
				case 149:
				case 180:
				{
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_12
				break;
				}
				case 224:
				case 228:
				case 225:
				{
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_11
				break;
				}
				case 41:
				case 169:
				case 45:
				{
				PIXEL00_12
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
				}
				case 22:
				case 54:
				{
				PIXEL00_22
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_20
				PIXEL11_21
				break;
				}
				case 208:
				case 209:
				{
				PIXEL00_20
				PIXEL01_22
				PIXEL10_21
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 104:
				case 108:
				{
				PIXEL00_21
				PIXEL01_20
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_22
				break;
				}
				case 11:
				case 139:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_22
				PIXEL11_20
				break;
				}
				case 19:
				case 51:
				{
				if (Diff(w[2], w[6]))
				{
				PIXEL00_11
				PIXEL01_10
				}
				else
				{
				PIXEL00_60
				PIXEL01_90
				}
				PIXEL10_20
				PIXEL11_21
				break;
				}
				case 146:
				case 178:
				{
				PIXEL00_22
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				PIXEL11_12
				}
				else
				{
				PIXEL01_90
				PIXEL11_61
				}
				PIXEL10_20
				break;
				}
				case 84:
				case 85:
				{
				PIXEL00_20
				if (Diff(w[6], w[8]))
				{
				PIXEL01_11
				PIXEL11_10
				}
				else
				{
				PIXEL01_60
				PIXEL11_90
				}
				PIXEL10_21
				break;
				}
				case 112:
				case 113:
				{
				PIXEL00_20
				PIXEL01_22
				if (Diff(w[6], w[8]))
				{
				PIXEL10_12
				PIXEL11_10
				}
				else
				{
				PIXEL10_61
				PIXEL11_90
				}
				break;
				}
				case 200:
				case 204:
				{
				PIXEL00_21
				PIXEL01_20
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				PIXEL11_11
				}
				else
				{
				PIXEL10_90
				PIXEL11_60
				}
				break;
				}
				case 73:
				case 77:
				{
				if (Diff(w[8], w[4]))
				{
				PIXEL00_12
				PIXEL10_10
				}
				else
				{
				PIXEL00_61
				PIXEL10_90
				}
				PIXEL01_20
				PIXEL11_22
				break;
				}
				case 42:
				case 170:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				PIXEL10_11
				}
				else
				{
				PIXEL00_90
				PIXEL10_60
				}
				PIXEL01_21
				PIXEL11_20
				break;
				}
				case 14:
				case 142:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				PIXEL01_12
				}
				else
				{
				PIXEL00_90
				PIXEL01_61
				}
				PIXEL10_22
				PIXEL11_20
				break;
				}
				case 67:
				{
				PIXEL00_11
				PIXEL01_21
				PIXEL10_21
				PIXEL11_22
				break;
				}
				case 70:
				{
				PIXEL00_22
				PIXEL01_12
				PIXEL10_21
				PIXEL11_22
				break;
				}
				case 28:
				{
				PIXEL00_21
				PIXEL01_11
				PIXEL10_22
				PIXEL11_21
				break;
				}
				case 152:
				{
				PIXEL00_21
				PIXEL01_22
				PIXEL10_22
				PIXEL11_12
				break;
				}
				case 194:
				{
				PIXEL00_22
				PIXEL01_21
				PIXEL10_21
				PIXEL11_11
				break;
				}
				case 98:
				{
				PIXEL00_22
				PIXEL01_21
				PIXEL10_12
				PIXEL11_22
				break;
				}
				case 56:
				{
				PIXEL00_21
				PIXEL01_22
				PIXEL10_11
				PIXEL11_21
				break;
				}
				case 25:
				{
				PIXEL00_12
				PIXEL01_22
				PIXEL10_22
				PIXEL11_21
				break;
				}
				case 26:
				case 31:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_22
				PIXEL11_21
				break;
				}
				case 82:
				case 214:
				{
				PIXEL00_22
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_21
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 88:
				case 248:
				{
				PIXEL00_21
				PIXEL01_22
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 74:
				case 107:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_21
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_22
				break;
				}
				case 27:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_22
				PIXEL11_21
				break;
				}
				case 86:
				{
				PIXEL00_22
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_21
				PIXEL11_10
				break;
				}
				case 216:
				{
				PIXEL00_21
				PIXEL01_22
				PIXEL10_10
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 106:
				{
				PIXEL00_10
				PIXEL01_21
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_22
				break;
				}
				case 30:
				{
				PIXEL00_10
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_22
				PIXEL11_21
				break;
				}
				case 210:
				{
				PIXEL00_22
				PIXEL01_10
				PIXEL10_21
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 120:
				{
				PIXEL00_21
				PIXEL01_22
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_10
				break;
				}
				case 75:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_10
				PIXEL11_22
				break;
				}
				case 29:
				{
				PIXEL00_12
				PIXEL01_11
				PIXEL10_22
				PIXEL11_21
				break;
				}
				case 198:
				{
				PIXEL00_22
				PIXEL01_12
				PIXEL10_21
				PIXEL11_11
				break;
				}
				case 184:
				{
				PIXEL00_21
				PIXEL01_22
				PIXEL10_11
				PIXEL11_12
				break;
				}
				case 99:
				{
				PIXEL00_11
				PIXEL01_21
				PIXEL10_12
				PIXEL11_22
				break;
				}
				case 57:
				{
				PIXEL00_12
				PIXEL01_22
				PIXEL10_11
				PIXEL11_21
				break;
				}
				case 71:
				{
				PIXEL00_11
				PIXEL01_12
				PIXEL10_21
				PIXEL11_22
				break;
				}
				case 156:
				{
				PIXEL00_21
				PIXEL01_11
				PIXEL10_22
				PIXEL11_12
				break;
				}
				case 226:
				{
				PIXEL00_22
				PIXEL01_21
				PIXEL10_12
				PIXEL11_11
				break;
				}
				case 60:
				{
				PIXEL00_21
				PIXEL01_11
				PIXEL10_11
				PIXEL11_21
				break;
				}
				case 195:
				{
				PIXEL00_11
				PIXEL01_21
				PIXEL10_21
				PIXEL11_11
				break;
				}
				case 102:
				{
				PIXEL00_22
				PIXEL01_12
				PIXEL10_12
				PIXEL11_22
				break;
				}
				case 153:
				{
				PIXEL00_12
				PIXEL01_22
				PIXEL10_22
				PIXEL11_12
				break;
				}
				case 58:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				PIXEL10_11
				PIXEL11_21
				break;
				}
				case 83:
				{
				PIXEL00_11
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				PIXEL10_21
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 92:
				{
				PIXEL00_21
				PIXEL01_11
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 202:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				PIXEL01_21
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				PIXEL11_11
				break;
				}
				case 78:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				PIXEL01_12
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				PIXEL11_22
				break;
				}
				case 154:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				PIXEL10_22
				PIXEL11_12
				break;
				}
				case 114:
				{
				PIXEL00_22
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				PIXEL10_12
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 89:
				{
				PIXEL00_12
				PIXEL01_22
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 90:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 55:
				case 23:
				{
				if (Diff(w[2], w[6]))
				{
				PIXEL00_11
				PIXEL01_0
				}
				else
				{
				PIXEL00_60
				PIXEL01_90
				}
				PIXEL10_20
				PIXEL11_21
				break;
				}
				case 182:
				case 150:
				{
				PIXEL00_22
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				PIXEL11_12
				}
				else
				{
				PIXEL01_90
				PIXEL11_61
				}
				PIXEL10_20
				break;
				}
				case 213:
				case 212:
				{
				PIXEL00_20
				if (Diff(w[6], w[8]))
				{
				PIXEL01_11
				PIXEL11_0
				}
				else
				{
				PIXEL01_60
				PIXEL11_90
				}
				PIXEL10_21
				break;
				}
				case 241:
				case 240:
				{
				PIXEL00_20
				PIXEL01_22
				if (Diff(w[6], w[8]))
				{
				PIXEL10_12
				PIXEL11_0
				}
				else
				{
				PIXEL10_61
				PIXEL11_90
				}
				break;
				}
				case 236:
				case 232:
				{
				PIXEL00_21
				PIXEL01_20
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				PIXEL11_11
				}
				else
				{
				PIXEL10_90
				PIXEL11_60
				}
				break;
				}
				case 109:
				case 105:
				{
				if (Diff(w[8], w[4]))
				{
				PIXEL00_12
				PIXEL10_0
				}
				else
				{
				PIXEL00_61
				PIXEL10_90
				}
				PIXEL01_20
				PIXEL11_22
				break;
				}
				case 171:
				case 43:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				PIXEL10_11
				}
				else
				{
				PIXEL00_90
				PIXEL10_60
				}
				PIXEL01_21
				PIXEL11_20
				break;
				}
				case 143:
				case 15:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				PIXEL01_12
				}
				else
				{
				PIXEL00_90
				PIXEL01_61
				}
				PIXEL10_22
				PIXEL11_20
				break;
				}
				case 124:
				{
				PIXEL00_21
				PIXEL01_11
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_10
				break;
				}
				case 203:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_10
				PIXEL11_11
				break;
				}
				case 62:
				{
				PIXEL00_10
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_11
				PIXEL11_21
				break;
				}
				case 211:
				{
				PIXEL00_11
				PIXEL01_10
				PIXEL10_21
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 118:
				{
				PIXEL00_22
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_12
				PIXEL11_10
				break;
				}
				case 217:
				{
				PIXEL00_12
				PIXEL01_22
				PIXEL10_10
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 110:
				{
				PIXEL00_10
				PIXEL01_12
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_22
				break;
				}
				case 155:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_22
				PIXEL11_12
				break;
				}
				case 188:
				{
				PIXEL00_21
				PIXEL01_11
				PIXEL10_11
				PIXEL11_12
				break;
				}
				case 185:
				{
				PIXEL00_12
				PIXEL01_22
				PIXEL10_11
				PIXEL11_12
				break;
				}
				case 61:
				{
				PIXEL00_12
				PIXEL01_11
				PIXEL10_11
				PIXEL11_21
				break;
				}
				case 157:
				{
				PIXEL00_12
				PIXEL01_11
				PIXEL10_22
				PIXEL11_12
				break;
				}
				case 103:
				{
				PIXEL00_11
				PIXEL01_12
				PIXEL10_12
				PIXEL11_22
				break;
				}
				case 227:
				{
				PIXEL00_11
				PIXEL01_21
				PIXEL10_12
				PIXEL11_11
				break;
				}
				case 230:
				{
				PIXEL00_22
				PIXEL01_12
				PIXEL10_12
				PIXEL11_11
				break;
				}
				case 199:
				{
				PIXEL00_11
				PIXEL01_12
				PIXEL10_21
				PIXEL11_11
				break;
				}
				case 220:
				{
				PIXEL00_21
				PIXEL01_11
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 158:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_22
				PIXEL11_12
				break;
				}
				case 234:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				PIXEL01_21
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_11
				break;
				}
				case 242:
				{
				PIXEL00_22
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				PIXEL10_12
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 59:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				PIXEL10_11
				PIXEL11_21
				break;
				}
				case 121:
				{
				PIXEL00_12
				PIXEL01_22
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 87:
				{
				PIXEL00_11
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_21
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 79:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_12
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				PIXEL11_22
				break;
				}
				case 122:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 94:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 218:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 91:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 229:
				{
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_11
				break;
				}
				case 167:
				{
				PIXEL00_11
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
				}
				case 173:
				{
				PIXEL00_12
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
				}
				case 181:
				{
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_12
				break;
				}
				case 186:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				PIXEL10_11
				PIXEL11_12
				break;
				}
				case 115:
				{
				PIXEL00_11
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				PIXEL10_12
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 93:
				{
				PIXEL00_12
				PIXEL01_11
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 206:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				PIXEL01_12
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				PIXEL11_11
				break;
				}
				case 205:
				case 201:
				{
				PIXEL00_12
				PIXEL01_20
				if (Diff(w[8], w[4]))
				{
				PIXEL10_10
				}
				else
				{
				PIXEL10_70
				}
				PIXEL11_11
				break;
				}
				case 174:
				case 46:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_10
				}
				else
				{
				PIXEL00_70
				}
				PIXEL01_12
				PIXEL10_11
				PIXEL11_20
				break;
				}
				case 179:
				case 147:
				{
				PIXEL00_11
				if (Diff(w[2], w[6]))
				{
				PIXEL01_10
				}
				else
				{
				PIXEL01_70
				}
				PIXEL10_20
				PIXEL11_12
				break;
				}
				case 117:
				case 116:
				{
				PIXEL00_20
				PIXEL01_11
				PIXEL10_12
				if (Diff(w[6], w[8]))
				{
				PIXEL11_10
				}
				else
				{
				PIXEL11_70
				}
				break;
				}
				case 189:
				{
				PIXEL00_12
				PIXEL01_11
				PIXEL10_11
				PIXEL11_12
				break;
				}
				case 231:
				{
				PIXEL00_11
				PIXEL01_12
				PIXEL10_12
				PIXEL11_11
				break;
				}
				case 126:
				{
				PIXEL00_10
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_10
				break;
				}
				case 219:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_10
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 125:
				{
				if (Diff(w[8], w[4]))
				{
				PIXEL00_12
				PIXEL10_0
				}
				else
				{
				PIXEL00_61
				PIXEL10_90
				}
				PIXEL01_11
				PIXEL11_10
				break;
				}
				case 221:
				{
				PIXEL00_12
				if (Diff(w[6], w[8]))
				{
				PIXEL01_11
				PIXEL11_0
				}
				else
				{
				PIXEL01_60
				PIXEL11_90
				}
				PIXEL10_10
				break;
				}
				case 207:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				PIXEL01_12
				}
				else
				{
				PIXEL00_90
				PIXEL01_61
				}
				PIXEL10_10
				PIXEL11_11
				break;
				}
				case 238:
				{
				PIXEL00_10
				PIXEL01_12
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				PIXEL11_11
				}
				else
				{
				PIXEL10_90
				PIXEL11_60
				}
				break;
				}
				case 190:
				{
				PIXEL00_10
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				PIXEL11_12
				}
				else
				{
				PIXEL01_90
				PIXEL11_61
				}
				PIXEL10_11
				break;
				}
				case 187:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				PIXEL10_11
				}
				else
				{
				PIXEL00_90
				PIXEL10_60
				}
				PIXEL01_10
				PIXEL11_12
				break;
				}
				case 243:
				{
				PIXEL00_11
				PIXEL01_10
				if (Diff(w[6], w[8]))
				{
				PIXEL10_12
				PIXEL11_0
				}
				else
				{
				PIXEL10_61
				PIXEL11_90
				}
				break;
				}
				case 119:
				{
				if (Diff(w[2], w[6]))
				{
				PIXEL00_11
				PIXEL01_0
				}
				else
				{
				PIXEL00_60
				PIXEL01_90
				}
				PIXEL10_12
				PIXEL11_10
				break;
				}
				case 237:
				case 233:
				{
				PIXEL00_12
				PIXEL01_20
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_100
				}
				PIXEL11_11
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
				PIXEL00_100
				}
				PIXEL01_12
				PIXEL10_11
				PIXEL11_20
				break;
				}
				case 183:
				case 151:
				{
				PIXEL00_11
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_100
				}
				PIXEL10_20
				PIXEL11_12
				break;
				}
				case 245:
				case 244:
				{
				PIXEL00_20
				PIXEL01_11
				PIXEL10_12
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_100
				}
				break;
				}
				case 250:
				{
				PIXEL00_10
				PIXEL01_10
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 123:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_10
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_10
				break;
				}
				case 95:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_10
				PIXEL11_10
				break;
				}
				case 222:
				{
				PIXEL00_10
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_10
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 252:
				{
				PIXEL00_21
				PIXEL01_11
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_100
				}
				break;
				}
				case 249:
				{
				PIXEL00_12
				PIXEL01_22
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_100
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 235:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_21
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_100
				}
				PIXEL11_11
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
				PIXEL00_100
				}
				PIXEL01_12
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_22
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
				PIXEL00_100
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_11
				PIXEL11_21
				break;
				}
				case 159:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_100
				}
				PIXEL10_22
				PIXEL11_12
				break;
				}
				case 215:
				{
				PIXEL00_11
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_100
				}
				PIXEL10_21
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 246:
				{
				PIXEL00_22
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				PIXEL10_12
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_100
				}
				break;
				}
				case 254:
				{
				PIXEL00_10
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_100
				}
				break;
				}
				case 253:
				{
				PIXEL00_12
				PIXEL01_11
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_100
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_100
				}
				break;
				}
				case 251:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				PIXEL01_10
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_100
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
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
				PIXEL00_100
				}
				PIXEL01_12
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_100
				}
				PIXEL11_11
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
				PIXEL00_100
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_20
				}
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_20
				}
				PIXEL11_10
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
				PIXEL00_100
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_100
				}
				PIXEL10_11
				PIXEL11_12
				break;
				}
				case 223:
				{
				if (Diff(w[4], w[2]))
				{
				PIXEL00_0
				}
				else
				{
				PIXEL00_20
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_100
				}
				PIXEL10_10
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_20
				}
				break;
				}
				case 247:
				{
				PIXEL00_11
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_100
				}
				PIXEL10_12
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_100
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
				PIXEL00_100
				}
				if (Diff(w[2], w[6]))
				{
				PIXEL01_0
				}
				else
				{
				PIXEL01_100
				}
				if (Diff(w[8], w[4]))
				{
				PIXEL10_0
				}
				else
				{
				PIXEL10_100
				}
				if (Diff(w[6], w[8]))
				{
				PIXEL11_0
				}
				else
				{
				PIXEL11_100
				}
				break;
				}
			}
			++pIn;
			pOut += 2;
		}
		pOut += dstPitch * 2 - Xres * 2;
	}
}

MaxSt_Hq2x::MaxSt_Hq2x() {
	buffer = NULL;
}

MaxSt_Hq2x::~MaxSt_Hq2x() {
	outit();
}

void MaxSt_Hq2x::init() {
	delete []buffer;
	buffer = new Gambatte::uint_least32_t[144 * 160];
}

void MaxSt_Hq2x::outit() {
	delete []buffer;
	buffer = NULL;
}

const Gambatte::FilterInfo& MaxSt_Hq2x::info() {
	static const Gambatte::FilterInfo fInfo = { "MaxSt's Hq2x", 160 * 2, 144 * 2 };
	return fInfo;
}

Gambatte::uint_least32_t* MaxSt_Hq2x::inBuffer() {
	return buffer;
}

unsigned MaxSt_Hq2x::inPitch() {
	return 160;
}

void MaxSt_Hq2x::filter(Gambatte::uint_least32_t *const dbuffer, const unsigned pitch) {
	::filter(dbuffer, pitch, buffer, 160, 144);
}
