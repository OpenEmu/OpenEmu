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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "kyra/screen_v2.h"

#include "common/endian.h"

namespace Kyra {

Screen_v2::Screen_v2(KyraEngine_v1 *vm, OSystem *system, const ScreenDim *dimTable, const int dimTableSize)
	: Screen(vm, system, dimTable, dimTableSize), _wsaFrameAnimBuffer(0) {
	_wsaFrameAnimBuffer = new uint8[1024];
	assert(_wsaFrameAnimBuffer);
}

Screen_v2::~Screen_v2() {
	delete[] _wsaFrameAnimBuffer;
}

uint8 *Screen_v2::generateOverlay(const Palette &pal, uint8 *buffer, int opColor, uint weight, int maxColor) {
	if (!buffer)
		return buffer;

	weight = MIN<uint>(weight, 255) >> 1;

	const byte opR = pal[opColor * 3 + 0];
	const byte opG = pal[opColor * 3 + 1];
	const byte opB = pal[opColor * 3 + 2];

	uint8 *dst = buffer;
	*dst++ = 0;

	int maxIndex = maxColor;
	if (maxIndex == -1) {
		if (_vm->game() == GI_LOL) {
			if (_use16ColorMode)
				maxIndex = 255;
			else
				maxIndex = 127;
		} else {
			maxIndex = 255;
		}
	}

	for (int i = 1; i != 256; ++i) {
		const byte curR = pal[i * 3 + 0] - (((pal[i * 3 + 0] - opR) * weight) >> 7);
		const byte curG = pal[i * 3 + 1] - (((pal[i * 3 + 1] - opG) * weight) >> 7);
		const byte curB = pal[i * 3 + 2] - (((pal[i * 3 + 2] - opB) * weight) >> 7);

		uint16 idxSum = _use16ColorMode ? 0xFFFF : 0x7FFF;
		byte index = opColor;

		for (int curIdx = 1; curIdx <= maxIndex; ++curIdx) {
			if (!_use16ColorMode && i == curIdx)
				continue;

			int diff = 0;
			uint16 sum = 0;

			diff = pal[curIdx * 3 + 0] - curR;
			sum += diff * diff;
			diff = pal[curIdx * 3 + 1] - curG;
			sum += diff * diff;
			diff = pal[curIdx * 3 + 2] - curB;
			sum += diff * diff;

			if (!sum) {
				index = curIdx;
				break;
			}

			if (sum <= idxSum) {
				if (!_use16ColorMode || (curIdx == opColor || curIdx != i)) {
					idxSum = sum;
					index = curIdx;
				}
			}
		}

		*dst++ = index;
	}

	return buffer;
}

void Screen_v2::applyOverlay(int x, int y, int w, int h, int pageNum, const uint8 *overlay) {
	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, w, h);

	uint8 *dst = getPagePtr(pageNum) + y * 320 + x;
	while (h--) {
		for (int wi = 0; wi < w; ++wi) {
			uint8 index = *dst;
			*dst++ = overlay[index];
		}
		dst += 320 - w;
	}
}

int Screen_v2::findLeastDifferentColor(const uint8 *paletteEntry, const Palette &pal, uint8 firstColor, uint16 numColors, bool skipSpecialColors) {
	int m = 0x7fff;
	int r = 0x101;

	for (int i = 0; i < numColors; i++) {
		if (skipSpecialColors && i >= 0xc0 && i <= 0xc3)
			continue;

		int v = paletteEntry[0] - pal[(i + firstColor) * 3 + 0];
		int c = v * v;
		v = paletteEntry[1] - pal[(i + firstColor) * 3 + 1];
		c += (v * v);
		v = paletteEntry[2] - pal[(i + firstColor) * 3 + 2];
		c += (v * v);

		if (c <= m) {
			m = c;
			r = i;
		}
	}

	return r;
}

void Screen_v2::getFadeParams(const Palette &pal, int delay, int &delayInc, int &diff) {
	int maxDiff = 0;
	diff = 0;
	for (int i = 0; i < pal.getNumColors() * 3; ++i) {
		diff = ABS(pal[i] - (*_screenPalette)[i]);
		maxDiff = MAX(maxDiff, diff);
	}

	delayInc = delay << 8;
	if (maxDiff != 0) {
		delayInc /= maxDiff;
		delayInc = MIN(delayInc, 0x7FFF);
	}

	delay = delayInc;
	for (diff = 1; diff <= maxDiff; ++diff) {
		if (delayInc >= 256)
			break;
		delayInc += delay;
	}
}

bool Screen_v2::timedPaletteFadeStep(uint8 *pal1, uint8 *pal2, uint32 elapsedTime, uint32 totalTime) {
	Palette &p1 = getPalette(1);

	bool res = false;
	for (int i = 0; i < p1.getNumColors() * 3; i++) {
		uint8 out = 0;

		if (elapsedTime < totalTime) {
			int32 d = ((pal2[i] & 0x3f) - (pal1[i] & 0x3f));
			if (d)
				res = true;

			int32 val = ((((d << 8) / (int32)totalTime) * (int32)elapsedTime) >> 8);
			out = ((pal1[i] & 0x3f) + (int8)val);
		} else {
			out = p1[i] = (pal2[i] & 0x3f);
			res = false;
		}

		(*_internFadePalette)[i] = out;
	}

	setScreenPalette(*_internFadePalette);
	updateScreen();

	return res;
}

const uint8 *Screen_v2::getPtrToShape(const uint8 *shpFile, int shape) {
	uint16 shapes = READ_LE_UINT16(shpFile);

	if (shapes <= shape)
		return 0;

	uint32 offset = READ_LE_UINT32(shpFile + (shape << 2) + 2);

	return shpFile + offset + 2;
}

uint8 *Screen_v2::getPtrToShape(uint8 *shpFile, int shape) {
	uint16 shapes = READ_LE_UINT16(shpFile);

	if (shapes <= shape)
		return 0;

	uint32 offset = READ_LE_UINT32(shpFile + (shape << 2) + 2);

	return shpFile + offset + 2;
}

int Screen_v2::getShapeScaledWidth(const uint8 *shpFile, int scale) {
	if (!shpFile)
		return 0;
	int width = READ_LE_UINT16(shpFile + 3);
	return (width * scale) >> 8;
}

int Screen_v2::getShapeScaledHeight(const uint8 *shpFile, int scale) {
	if (!shpFile)
		return 0;
	int height = shpFile[2];
	return (height * scale) >> 8;
}

uint16 Screen_v2::getShapeSize(const uint8 *shp) {
	if (!shp)
		return 0;
	return READ_LE_UINT16(shp + 6);
}

uint8 *Screen_v2::makeShapeCopy(const uint8 *src, int index) {
	const uint8 *shape = getPtrToShape(src, index);
	if (!shape)
		return 0;

	int size = getShapeSize(shape);

	uint8 *copy = new uint8[size];
	assert(copy);
	memcpy(copy, shape, size);

	return copy;
}

int Screen_v2::getLayer(int x, int y) {
	if (x < 0)
		x = 0;
	else if (x >= 320)
		x = 319;
	if (y < 0)
		y = 0;
	else if (y >= 144)
		y = 143;

	uint8 pixel = *(getCPagePtr(5) + y * 320 + x);
	pixel &= 0x7F;
	pixel >>= 3;

	if (pixel < 1)
		pixel = 1;
	else if (pixel > 15)
		pixel = 15;
	return pixel;
}

int Screen_v2::getRectSize(int w, int h) {
	if (w > 320 || h > 200)
		return 0;
	return w * h;
}

void Screen_v2::setTextColorMap(const uint8 *cmap) {
	setTextColor(cmap, 0, 15);
}

void Screen_v2::wsaFrameAnimationStep(int x1, int y1, int x2, int y2,
                                      int w1, int h1, int w2, int h2, int srcPage, int dstPage, int dim) {

	if (!(w1 || h1 || w2 || h2))
		return;

	ScreenDim cdm = *getScreenDim(dim);
	cdm.sx <<= 3;
	cdm.w <<= 3;

	int na = 0, nb = 0, nc = w2;

	if (!calcBounds(cdm.w, cdm.h, x2, y2, w2, h2, na, nb, nc))
		return;

	const uint8 *src = getPagePtr(srcPage) + y1 * 320;
	uint8 *dst = getPagePtr(dstPage) + (y2 + cdm.sy) * 320;

	int u = -1;

	do {
		int t = (nb * h1) / h2;
		if (t != u) {
			u = t;
			const uint8 *s = src + x1 + t * 320;
			uint8 *dt = (uint8 *)_wsaFrameAnimBuffer;

			t = w2 - w1;
			if (!t) {
				memcpy(dt, s, w2);
			} else if (t > 0) {
				if (w1 == 1) {
					memset(dt, *s, w2);
				} else {
					t = ((((((w2 - w1 + 1) & 0xffff) << 8) / w1) + 0x100) & 0xffff) << 8;
					int bp = 0;
					for (int i = 0; i < w1; i++) {
						int cnt = (t >> 16);
						bp += (t & 0xffff);
						if (bp > 0xffff) {
							bp -= 0xffff;
							cnt++;
						}
						memset(dt, *s++, cnt);
						dt += cnt;
					}
				}
			} else {
				if (w2 == 1) {
					*dt = *s;
				} else {
					t = (((((w1 - w2) & 0xffff) << 8) / w2) & 0xffff) << 8;
					int bp = 0;
					for (int i = 0; i < w2; i++) {
						*dt++ = *s++;
						bp += (t & 0xffff);
						if (bp > 0xffff) {
							bp -= 0xffff;
							s++;
						}
						s += (t >> 16);
					}
				}
			}
		}
		memcpy(dst + x2 + cdm.sx, _wsaFrameAnimBuffer + na, w2);
		dst += 320;
	} while (++nb < h2);

	if (!dstPage)
		addDirtyRect(x2, y2, w2, h2);
}

void Screen_v2::copyPageMemory(int srcPage, int srcPos, int dstPage, int dstPos, int numBytes) {
	const uint8 *src = getPagePtr(srcPage) + srcPos;
	uint8 *dst = getPagePtr(dstPage) + dstPos;
	memcpy(dst, src, numBytes);
}

void Screen_v2::copyRegionEx(int srcPage, int srcW, int srcH, int dstPage, int dstX, int dstY, int dstW, int dstH, const ScreenDim *dim, bool flag) {
	int x0 = dim->sx << 3;
	int y0 = dim->sy;
	int w0 = dim->w << 3;
	int h0 = dim->h;

	int x1 = dstX;
	int y1 = dstY;
	int w1 = dstW;
	int h1 = dstH;

	int x2, y2, w2;

	calcBounds(w0, h0, x1, y1, w1, h1, x2, y2, w2);

	const uint8 *src = getPagePtr(srcPage) + (320 * srcH) + srcW;
	uint8 *dst = getPagePtr(dstPage) + 320 * (y0 + y1);

	for (int y = 0; y < h1; y++) {
		const uint8 *s = src + x2;
		uint8 *d = dst + x0 + x1;

		if (flag)
			d += (h1 >> 1);

		for (int x = 0; x < w1; x++) {
			if (*s)
				*d = *s;
			s++;
			d++;
		}
		dst += 320;
		src += 320;
	}
}

bool Screen_v2::calcBounds(int w0, int h0, int &x1, int &y1, int &w1, int &h1, int &x2, int &y2, int &w2) {
	x2 = 0;
	y2 = 0;
	w2 = w1;

	int t = x1 + w1;
	if (t < 1) {
		w1 = h1 = -1;
	} else {
		if (t <= x1) {
			x2 = w1 - t;
			w1 = t;
			x1 = 0;
		}
		t = w0 - x1;
		if (t < 1) {
			w1 = h1 = -1;
		} else {
			if (t <= w1) {
				w1 = t;
			}
			w2 -= w1;
			t = h1 + y1;
			if (t < 1) {
				w1 = h1 = -1;
			} else {
				if (t <= y1) {
					y2 = h1 - t;
					h1 = t;
					y1 = 0;
				}
				t = h0 - y1;
				if (t < 1) {
					w1 = h1 = -1;
				} else {
					if (t <= h1) {
						h1 = t;
					}
				}
			}
		}
	}

	return w1 != -1;
}

void Screen_v2::checkedPageUpdate(int srcPage, int dstPage) {
	const uint32 *src = (const uint32 *)getPagePtr(srcPage);
	uint32 *dst = (uint32 *)getPagePtr(dstPage);
	uint32 *page0 = (uint32 *)getPagePtr(0);

	bool updated = false;

	for (int y = 0; y < 200; ++y) {
		for (int x = 0; x < 80; ++x, ++src, ++dst, ++page0) {
			if (*src != *dst) {
				updated = true;
				*dst = *page0 = *src;
			}
		}
	}

	if (updated)
		addDirtyRect(0, 0, 320, 200);
}

} // End of namespace Kyra
