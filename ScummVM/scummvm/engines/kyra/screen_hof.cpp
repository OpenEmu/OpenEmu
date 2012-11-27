/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "kyra/screen_hof.h"
#include "kyra/kyra_hof.h"

namespace Kyra {

Screen_HoF::Screen_HoF(KyraEngine_HoF *vm, OSystem *system)
	: Screen_v2(vm, system, _screenDimTable, _screenDimTableCount), _vm(vm) {
}

void Screen_HoF::generateGrayOverlay(const Palette &srcPal, uint8 *grayOverlay, int factor, int addR, int addG, int addB, int lastColor, bool flag) {
	Palette tmpPal(lastColor);

	for (int i = 0; i != lastColor; i++) {
		if (flag) {
			int v = ((((srcPal[3 * i] & 0x3f) + (srcPal[3 * i + 1] & 0x3f)
				+ (srcPal[3 * i + 2] & 0x3f)) / 3) * factor) / 0x40;
			tmpPal[3 * i] = tmpPal[3 * i + 1] = tmpPal[3 * i + 2] = v & 0xff;
		} else {
			int v = (((srcPal[3 * i] & 0x3f) * factor) / 0x40) + addR;
			tmpPal[3 * i] = (v > 0x3f) ? 0x3f : v & 0xff;
			v = (((srcPal[3 * i + 1] & 0x3f) * factor) / 0x40) + addG;
			tmpPal[3 * i + 1] = (v > 0x3f) ? 0x3f : v & 0xff;
			v = (((srcPal[3 * i + 2] & 0x3f) * factor) / 0x40) + addB;
			tmpPal[3 * i + 2] = (v > 0x3f) ? 0x3f : v & 0xff;
		}
	}

	for (int i = 0; i < lastColor; i++)
		grayOverlay[i] = findLeastDifferentColor(tmpPal.getData() + 3 * i, srcPal, 0, lastColor);
}

void Screen_HoF::cmpFadeFrameStep(int srcPage, int srcW, int srcH, int srcX, int srcY, int dstPage, int dstW,
	int dstH, int dstX, int dstY, int cmpW, int cmpH, int cmpPage) {

	if (!cmpW || !cmpH)
		return;

	int r1, r2, r3, r4, r5, r6;

	int X1 = srcX;
	int Y1 = srcY;
	int W1 = cmpW;
	int H1 = cmpH;

	if (!calcBounds(srcW, srcH, X1, Y1, W1, H1, r1, r2, r3))
		return;

	int X2 = dstX;
	int Y2 = dstY;
	int W2 = W1;
	int H2 = H1;

	if (!calcBounds(dstW, dstH, X2, Y2, W2, H2, r4, r5, r6))
		return;

	const uint8 *src = getPagePtr(srcPage) + srcW * (Y1 + r5);
	uint8 *dst = getPagePtr(dstPage) + dstW * (Y2 + r2);
	const uint8 *cmp = getPagePtr(cmpPage);

	while (H2--) {
		const uint8 *s = src + r4 + X1;
		uint8 *d = dst + r1 + X2;

		for (int i = 0; i < W2; i++) {
			int ix = (*s++ << 8) + *d;
			*d++ = cmp[ix];
		}

		src += W1;
		dst += W2;
	}
}

} // End of namespace Kyra
