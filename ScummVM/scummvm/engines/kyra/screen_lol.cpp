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

#ifdef ENABLE_LOL

#include "kyra/screen_lol.h"
#include "kyra/lol.h"

#include "common/system.h"

#include "graphics/palette.h"

namespace Kyra {

Screen_LoL::Screen_LoL(LoLEngine *vm, OSystem *system) : Screen_v2(vm, system,  vm->gameFlags().use16ColorMode ? _screenDimTable16C : _screenDimTable256C, _screenDimTableCount) {
	_paletteOverlay1 = new uint8[0x100];
	_paletteOverlay2 = new uint8[0x100];
	_grayOverlay = new uint8[0x100];
	memset(_paletteOverlay1, 0, 0x100);
	memset(_paletteOverlay2, 0, 0x100);
	memset(_grayOverlay, 0, 0x100);

	for (int i = 0; i < 8; i++)
		_levelOverlays[i] = new uint8[256];

	_fadeFlag = 2;
}

Screen_LoL::~Screen_LoL() {
	for (int i = 0; i < 8; i++)
		delete[] _levelOverlays[i];

	delete[] _paletteOverlay1;
	delete[] _paletteOverlay2;
	delete[] _grayOverlay;
}

void Screen_LoL::fprintString(const char *format, int x, int y, uint8 col1, uint8 col2, uint16 flags, ...) {
	if (!format)
		return;

	char string[240];
	va_list vaList;
	va_start(vaList, flags);
	vsnprintf(string, sizeof(string), format, vaList);
	va_end(vaList);

	if (flags & 1)
		x -= (getTextWidth(string) >> 1);

	if (flags & 2)
		x -= getTextWidth(string);

	if (_use16ColorMode) {
		if (flags & 12) {
			printText(string, x - 1, y, 0x44, col2);
			printText(string, x, y + 1, 0x44, col2);
		}
	} else {
		if (flags & 4) {
			printText(string, x - 1, y, 1, col2);
			printText(string, x, y + 1, 1, col2);
		}

		if (flags & 8) {
			printText(string, x - 1, y, 227, col2);
			printText(string, x, y + 1, 227, col2);
		}
	}

	printText(string, x, y, col1, col2);
}

void Screen_LoL::fprintStringIntro(const char *format, int x, int y, uint8 c1, uint8 c2, uint8 c3, uint16 flags, ...) {
	char buffer[400];

	va_list args;
	va_start(args, flags);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	if ((flags & 0x0F00) == 0x100)
		x -= getTextWidth(buffer) >> 1;
	if ((flags & 0x0F00) == 0x200)
		x -= getTextWidth(buffer);

	if ((flags & 0x00F0) == 0x20) {
		printText(buffer, x - 1, y, c3, c2);
		printText(buffer, x, y + 1, c3, c2);
	}

	printText(buffer, x, y, c1, c2);
}

void Screen_LoL::drawShadedBox(int x1, int y1, int x2, int y2, int color1, int color2) {
	assert(x1 >= 0 && y1 >= 0);
	hideMouse();

	fillRect(x1, y1, x2, y1 + 1, color1);
	fillRect(x1, y1, x1 + 1, y2, color1);

	drawClippedLine(x2, y1, x2, y2, color2);
	drawClippedLine(x2 - 1, y1 + 1, x2 - 1, y2 - 1, color2);
	drawClippedLine(x1 + 1, y2 - 1, x2, y2 - 1, color2);
	drawClippedLine(x1, y2, x2, y2, color2);

	if (_use16ColorMode && color1 > color2)
		drawBox(x1, y1, x2, y2, 0x44);

	showMouse();
}

void Screen_LoL::generateGrayOverlay(const Palette &srcPal, uint8 *grayOverlay, int factor, int addR, int addG, int addB, int lastColor, bool skipSpecialColors) {
	Palette tmpPal(lastColor);

	for (int i = 0; i != lastColor; i++) {
		int v = (((srcPal[3 * i] & 0x3f) * factor) / 0x40) + addR;
		tmpPal[3 * i] = (v > 0x3f) ? 0x3f : v & 0xff;
		v = (((srcPal[3 * i + 1] & 0x3f) * factor) / 0x40) + addG;
		tmpPal[3 * i + 1] = (v > 0x3f) ? 0x3f : v & 0xff;
		v = (((srcPal[3 * i + 2] & 0x3f) * factor) / 0x40) + addB;
		tmpPal[3 * i + 2] = (v > 0x3f) ? 0x3f : v & 0xff;
	}

	for (int i = 0; i < lastColor; i++)
		grayOverlay[i] = findLeastDifferentColor(tmpPal.getData() + 3 * i, srcPal, 0, lastColor, skipSpecialColors);
}

void Screen_LoL::createTransparencyTablesIntern(const uint8 *ovl, int a, const uint8 *fxPal1, const uint8 *fxPal2, uint8 *outTable1, uint8 *outTable2, int b) {
	Palette screenPal(256);
	screenPal.copy(fxPal2, 0, 256);

	memset(outTable1, 0xff, 256);

	for (int i = 0; i < a; i++)
		outTable1[ovl[i]] = i;

	for (int i = 0; i < a; i++) {
		if (ovl[i]) {
			uint8 tcol[3];
			uint16 fcol[3];
			uint16 scol[3];

			uint16 t1 = (b << 6) / 100;
			uint16 t2 = 64 - t1;

			uint8 c = ovl[i];
			fcol[0] = fxPal1[3 * c];
			fcol[1] = fxPal1[3 * c + 1];
			fcol[2] = fxPal1[3 * c + 2];

			uint8 *o = &outTable2[i << 8];

			for (int ii = 0; ii < 256; ii++) {
				scol[0] = screenPal[3 * ii];
				scol[1] = screenPal[3 * ii + 1];
				scol[2] = screenPal[3 * ii + 2];

				tcol[0] = CLIP(((fcol[0] * t2) >> 6) + ((scol[0] * t1) >> 6), 0, 63);
				tcol[1] = CLIP(((fcol[1] * t2) >> 6) + ((scol[1] * t1) >> 6), 0, 63);
				tcol[2] = CLIP(((fcol[2] * t2) >> 6) + ((scol[2] * t1) >> 6), 0, 63);

				o[ii] = findLeastDifferentColor(tcol, screenPal, 0, 255);
			}

		} else {
			memset(&outTable2[i << 8], 0, 256);
		}
	}
}

void Screen_LoL::drawGridBox(int x, int y, int w, int h, int col) {
	if (w <= 0 || x >= 320 || h <= 0 || y >= 200)
		return;

	if (x < 0) {
		x += w;
		if (x <= 0)
			return;
		w = x;
		x = 0;
	}

	int tmp = x + w;
	if (tmp >= 320) {
		w = 320 - x;
	}

	int pitch = 320 - w;

	if (y < 0) {
		y += h;
		if (y <= 0)
			return;
		h = y;
		y = 0;
	}

	tmp = y + h;
	if (tmp >= 200) {
		h = 200 - y;
	}

	tmp = (y + x) & 1;
	uint8 *p = getPagePtr(_curPage) + y * 320 + x;
	uint8 s = (tmp >> 8) & 1;

	w >>= 1;
	int w2 = w;

	while (h--) {
		if (w) {
			while (w--) {
				*(p + tmp) = col;
				p += 2;
			}
		}

		if (s == 1) {
			if (tmp == 0)
				*p = col;
			p++;
		}
		tmp ^= 1;
		p += pitch;
		w = w2;
	}
}

void Screen_LoL::fadeClearSceneWindow(int delay) {
	if (_fadeFlag == 1)
		return;

	if (_use16ColorMode) {
		fadeToBlack(delay);
		fillRect(112, 0, 288, 120, 0x44);
	} else {
		Palette tpal(getPalette(0).getNumColors());
		tpal.copy(getPalette(0), 128);

		loadSpecialColors(tpal);
		fadePalette(tpal, delay);

		fillRect(112, 0, 288, 120, 0);
	}

	_fadeFlag = 1;
}

void Screen_LoL::backupSceneWindow(int srcPageNum, int dstPageNum) {
	uint8 *src = getPagePtr(srcPageNum) + 112;
	uint8 *dst = getPagePtr(dstPageNum) + 0xa500;

	for (int h = 0; h < 120; h++) {
		for (int w = 0; w < 176; w++)
			*dst++ = *src++;
		src += 144;
	}
}

void Screen_LoL::restoreSceneWindow(int srcPageNum, int dstPageNum) {
	uint8 *src = getPagePtr(srcPageNum) + 0xa500;
	uint8 *dst = getPagePtr(dstPageNum) + 112;

	for (int h = 0; h < 120; h++) {
		memcpy(dst, src, 176);
		src += 176;
		dst += 320;
	}

	if (!dstPageNum)
		addDirtyRect(112, 0, 176, 120);
}

void Screen_LoL::clearGuiShapeMemory(int pageNum) {
	uint8 *dst = getPagePtr(pageNum) + 0x79b0;
	for (int i = 0; i < 23; i++) {
		memset(dst, 0, 176);
		dst += 320;
	}
}

void Screen_LoL::copyGuiShapeFromSceneBackupBuffer(int srcPageNum,  int dstPageNum) {
	uint8 *src = getPagePtr(srcPageNum) + 0x79c3;
	uint8 *dst = getPagePtr(dstPageNum);

	for (int i = 0; i < 23; i++) {
		uint8 len = 0;
		uint8 v = 0;

		do {
			v = *src++;
			len++;
		} while (!v);

		*dst++ = len;

		len = 69 - len;
		memcpy(dst, src, len);
		src += (len + 251);
		dst += len;
	}
}

void Screen_LoL::copyGuiShapeToSurface(int srcPageNum, int dstPageNum) {
	uint8 *src = getPagePtr(srcPageNum);
	uint8 *dst = getPagePtr(dstPageNum) + 0xe7c3;

	for (int i = 0; i < 23; i++) {
		uint8 v = *src++;
		uint8 len = 69 - v;
		dst += v;
		memcpy(dst, src, len);
		src += (len - 1);
		dst += len;

		for (int ii = 0; ii < len; ii++)
			*dst++ = *src--;

		src += (len + 1);
		dst += (v + 38);
	}
}

void Screen_LoL::smoothScrollZoomStepTop(int srcPageNum, int dstPageNum, int x, int y) {
	uint8 *src = getPagePtr(srcPageNum) + 0xa500 + y * 176 + x;
	uint8 *dst = getPagePtr(dstPageNum) + 0xa500;

	x <<= 1;
	uint16 width = 176 - x;
	uint16 scaleX = (((x + 1) << 8) / width + 0x100);
	uint16 cntW = scaleX >> 8;
	scaleX <<= 8;
	width--;
	uint16 widthCnt = width;

	uint16 height = 46 - y;
	uint16 scaleY = (((y + 1) << 8) / height + 0x100);
	scaleY <<= 8;

	uint32 scaleYc = 0;
	while (height) {
		uint32 scaleXc = 0;
		do {
			scaleXc += scaleX;
			int numbytes = cntW + (scaleXc >> 16);
			scaleXc &= 0xffff;
			memset(dst, *src++, numbytes);
			dst += numbytes;
		} while (--widthCnt);

		*dst++ = *src++;
		widthCnt = width;

		src += x;
		scaleYc += scaleY;

		if (scaleYc >> 16) {
			scaleYc = 0;
			src -= 176;
			continue;
		}

		height--;
	}
}

void Screen_LoL::smoothScrollZoomStepBottom(int srcPageNum, int dstPageNum, int x, int y) {
	uint8 *src = getPagePtr(srcPageNum) + 0xc4a0 + x;
	uint8 *dst = getPagePtr(dstPageNum) + 0xc4a0;

	x <<= 1;
	uint16 width = 176 - x;
	uint16 scaleX = (((x + 1) << 8) / width + 0x100);
	uint16 cntW = scaleX >> 8;
	scaleX <<= 8;
	width--;
	uint16 widthCnt = width;

	uint16 height = 74 - y;
	uint16 scaleY = (((y + 1) << 8) / height + 0x100);
	scaleY <<= 8;

	uint32 scaleYc = 0;
	while (height) {
		uint32 scaleXc = 0;
		do {
			scaleXc += scaleX;
			int numbytes = cntW + (scaleXc >> 16);
			scaleXc &= 0xffff;
			memset(dst, *src++, numbytes);
			dst += numbytes;
		} while (--widthCnt);

		*dst++ = *src++;
		widthCnt = width;

		src += x;
		scaleYc += scaleY;

		if (scaleYc >> 16) {
			scaleYc = 0;
			src -= 176;
			continue;
		}

		height--;
	}
}

void Screen_LoL::smoothScrollHorizontalStep(int pageNum, int srcX, int dstX, int w) {
	uint8 *d = getPagePtr(pageNum);
	uint8 *s = d + 112 + srcX;

	int w2 = srcX + w - dstX;
	int pitchS = 320 + w2 - (w << 1);

	int pitchD = 320 - w;
	int h = 120;

	while (h--) {
		for (int i = 0; i < w; i++)
			*d++ = *s++;
		d -= w;
		s -= w2;

		for (int i = 0; i < w; i++)
			*s++ = *d++;

		s += pitchS;
		d += pitchD;
	}
}

void Screen_LoL::smoothScrollTurnStep1(int srcPage1Num, int srcPage2Num, int dstPageNum) {
	uint8 *s = getPagePtr(srcPage1Num) + 273;
	uint8 *d = getPagePtr(dstPageNum) + 0xa500;

	for (int i = 0; i < 120; i++) {
		uint8 a = *s++;
		*d++ = a;
		*d++ = a;

		for (int ii = 0; ii < 14; ii++) {
			a = *s++;
			*d++ = a;
			*d++ = a;
			*d++ = a;
		}

		s += 305;
		d += 132;
	}

	s = getPagePtr(srcPage2Num) + 112;
	d = getPagePtr(dstPageNum)  + 0xa52c;

	for (int i = 0; i < 120; i++) {
		for (int ii = 0; ii < 33; ii++) {
			*d++ = *s++;
			*d++ = *s++;
			uint8 a = *s++;
			*d++ = a;
			*d++ = a;
		}

		s += 221;
		d += 44;
	}
}

void Screen_LoL::smoothScrollTurnStep2(int srcPage1Num, int srcPage2Num, int dstPageNum) {
	uint8 *s = getPagePtr(srcPage1Num) + 244;
	uint8 *d = getPagePtr(dstPageNum) + 0xa500;

	for (int k = 0; k < 2; k++) {
		for (int i = 0; i < 120; i++) {
			for (int ii = 0; ii < 44; ii++) {
				uint8 a = *s++;
				*d++ = a;
				*d++ = a;
			}

			s += 276;
			d += 88;
		}

		s = getPagePtr(srcPage2Num) + 112;
		d = getPagePtr(dstPageNum) + 0xa558;
	}
}

void Screen_LoL::smoothScrollTurnStep3(int srcPage1Num, int srcPage2Num, int dstPageNum) {
	uint8 *s = getPagePtr(srcPage1Num) + 189;
	uint8 *d = getPagePtr(dstPageNum) + 0xa500;

	for (int i = 0; i < 120; i++) {
		for (int ii = 0; ii < 33; ii++) {
			*d++ = *s++;
			*d++ = *s++;
			uint8 a = *s++;
			*d++ = a;
			*d++ = a;
		}

		s += 221;
		d += 44;
	}

	s = getPagePtr(srcPage2Num) + 112;
	d = getPagePtr(dstPageNum)  + 0xa584;

	for (int i = 0; i < 120; i++) {
		for (int ii = 0; ii < 14; ii++) {
			uint8 a = *s++;
			*d++ = a;
			*d++ = a;
			*d++ = a;
		}

		uint8 a = *s++;
		*d++ = a;
		*d++ = a;

		s += 305;
		d += 132;
	}
}

void Screen_LoL::copyRegionSpecial(int page1, int w1, int h1, int x1, int y1, int page2, int w2, int h2, int x2, int y2, int w3, int h3, int mode, ...) {
	if (!w3 || !h3)
		return;

	uint8 *table1 = 0;
	uint8 *table2 = 0;

	if (mode == 2) {
		va_list args;
		va_start(args, mode);
		table1 = va_arg(args, uint8 *);
		table2 = va_arg(args, uint8 *);
		va_end(args);
	}

	int na = 0, nb = 0, nc = w3;
	if (!calcBounds(w1, h1, x1, y1, w3, h3, na, nb, nc))
		return;

	int iu5_1 = na;
	int iu6_1 = nb;
	int ibw_1 = w3;
	int dx_1 = x1;
	int dy_1 = y1;

	if (!calcBounds(w2, h2, x2, y2, w3, h3, na, nb, nc))
		return;

	int iu5_2 = na;
	int iu6_2 = nb;
	int ibw_2 = w3;
	int ibh_2 = h3;
	int dx_2 = x2;
	int dy_2 = y2;

	uint8 *src = getPagePtr(page1) + (dy_1 + iu6_2) * w1;
	uint8 *dst = getPagePtr(page2) + (dy_2 + iu6_1) * w2;

	for (int i = 0; i < ibh_2; i++) {
		uint8 *s = src + iu5_2 + dx_1;
		uint8 *d = dst + iu5_1 + dx_2;

		if (mode == 0) {
			memcpy(d, s, ibw_2);

		} else if (mode == 1) {
			if (!(i & 1)) {
				s++;
				d++;
			}

			for (int ii = (i & 1) ^ 1; ii < ibw_2; ii += 2) {
				*d = *s;
				d += 2;
				s += 2;
			}

		} else if (mode == 2) {
			for (int ii = 0; ii < ibw_2; ii++) {
				uint8 cmd = *s++;
				uint8 offs = table1[cmd];
				if (!(offs & 0x80))
					cmd = table2[(offs << 8) | *d];
				*d++ = cmd;
			}

		} else if (mode == 3) {
			s = s - iu5_2 + ibw_1;
			s = s - iu5_2 - 1;
			for (int ii = 0; ii < ibw_2; ii++)
				*d++ = *s--;
		}

		dst += w2;
		src += w1;
	}

	if (!page2)
		addDirtyRect(x2, y2, w2, h2);
}

void Screen_LoL::copyBlockAndApplyOverlay(int page1, int x1, int y1, int page2, int x2, int y2, int w, int h, int dim, uint8 *ovl) {
	if (!w || !h || !ovl)
		return;

	const ScreenDim *cdim = getScreenDim(dim);
	int ix = cdim->sx << 3;
	int iy = cdim->sy;
	int iw = cdim->w << 3;
	int ih = cdim->h;

	int na = 0, nb = 0, nc = w;
	if (!calcBounds(iw, ih, x2, y2, w, h, na, nb, nc))
		return;

	uint8 *src = getPagePtr(page1) + y1 * 320 + x1;
	uint8 *dst = getPagePtr(page2) + (y2 + iy) * 320;

	for (int i = 0; i < h; i++) {
		uint8 *s = src + na;
		uint8 *d = dst + (x2 + ix);

		for (int ii = 0; ii < w; ii++) {
			uint8 p = ovl[*s++];
			if (p)
				*d = p;
			d++;
		}

		dst += 320;
		src += 320;
	}

	if (!page2)
		addDirtyRect(x2 + ix, y2 + iy, w, h);
}

void Screen_LoL::applyOverlaySpecial(int page1, int x1, int y1, int page2, int x2, int y2, int w, int h, int dim, int flag, uint8 *ovl) {
	if (!w || !h || !ovl)
		return;

	const ScreenDim *cdim = getScreenDim(dim);
	int ix = cdim->sx << 3;
	int iy = cdim->sy;
	int iw = cdim->w << 3;
	int ih = cdim->h;

	int na = 0, nb = 0, nc = w;
	if (!calcBounds(iw, ih, x2, y2, w, h, na, nb, nc))
		return;

	uint8 *src = getPagePtr(page1) + y1 * 320 + x1;
	uint8 *dst = getPagePtr(page2) + (y2 + iy) * 320;

	for (int i = 0; i < h; i++) {
		uint8 *s = src + na;
		uint8 *d = dst + (x2 + ix);

		if (flag)
			d += (i >> 1);

		for (int ii = 0; ii < w; ii++) {
			if (*s++)
				*d = ovl[*d];
			d++;
		}

		dst += 320;
		src += 320;
	}

	if (!page2)
		addDirtyRect(x2 + ix, y2 + iy, w, h);
}

void Screen_LoL::copyBlockAndApplyOverlayOutro(int srcPage, int dstPage, const uint8 *ovl) {
	if (!ovl)
		return;

	const byte *src = getCPagePtr(srcPage);
	byte *dst = getPagePtr(dstPage);

	for (int y = 0; y < 200; ++y) {
		for (int x = 0; x < 80; ++x) {
			uint32 srcData = READ_LE_UINT32(src); src += 4;
			uint32 dstData = READ_LE_UINT32(dst);
			uint16 offset = 0;

			offset = ((srcData & 0xFF) << 8) + (dstData & 0xFF);
			*dst++ = ovl[offset];

			offset = (srcData & 0xFF00) + ((dstData & 0xFF00) >> 8);
			*dst++ = ovl[offset];

			srcData >>= 16;
			dstData >>= 16;

			offset = ((srcData & 0xFF) << 8) + (dstData & 0xFF);
			*dst++ = ovl[offset];

			offset = (srcData & 0xFF00) + ((dstData & 0xFF00) >> 8);
			*dst++ = ovl[offset];
		}
	}
}

void Screen_LoL::fadeToBlack(int delay, const UpdateFunctor *upFunc) {
	Screen::fadeToBlack(delay, upFunc);
	_fadeFlag = 2;
}

void Screen_LoL::fadeToPalette1(int delay) {
	loadSpecialColors(getPalette(1));
	fadePalette(getPalette(1), delay);
	_fadeFlag = 0;
}

void Screen_LoL::loadSpecialColors(Palette &dst) {
	if (_use16ColorMode)
		return;

	dst.copy(*_screenPalette, 192, 4);
}

void Screen_LoL::copyColor(int dstColorIndex, int srcColorIndex) {
	uint8 *s = _screenPalette->getData() + srcColorIndex * 3;
	uint8 *d = _screenPalette->getData() + dstColorIndex * 3;
	memcpy(d, s, 3);

	uint8 ci[3];
	ci[0] = (d[0] << 2) | (d[0] & 3);
	ci[1] = (d[1] << 2) | (d[1] & 3);
	ci[2] = (d[2] << 2) | (d[2] & 3);

	_system->getPaletteManager()->setPalette(ci, dstColorIndex, 1);
}

bool Screen_LoL::fadeColor(int dstColorIndex, int srcColorIndex, uint32 elapsedTicks, uint32 totalTicks) {
	if (_use16ColorMode)
		return false;

	const uint8 *dst = _screenPalette->getData() + 3 * dstColorIndex;
	const uint8 *src = _screenPalette->getData() + 3 * srcColorIndex;
	uint8 *p = getPalette(1).getData() + 3 * dstColorIndex;

	bool res = false;

	int16 srcV = 0;
	int16 dstV = 0;
	int32 outV = 0;

	uint8 tmpPalEntry[3];

	for (int i = 0; i < 3; i++) {
		if (elapsedTicks < totalTicks) {
			srcV = *src & 0x3f;
			dstV = *dst & 0x3f;

			outV = srcV - dstV;
			if (outV)
				res = true;

			outV = dstV + ((((outV << 8) / (int32)totalTicks) * (int32)elapsedTicks) >> 8);
		} else {
			*p = outV = *src;
			res = false;
		}

		tmpPalEntry[i] = outV & 0xff;
		src++;
		dst++;
		p++;
	}

	_internFadePalette->copy(*_screenPalette);
	_internFadePalette->copy(tmpPalEntry, 0, 1, dstColorIndex);
	setScreenPalette(*_internFadePalette);
	updateScreen();

	return res;
}

Palette **Screen_LoL::generateFadeTable(Palette **dst, Palette *src1, Palette *src2, int numTabs) {
	int len = _use16ColorMode ? 48 : 768;
	if (!src1)
		src1 = _screenPalette;

	uint8 *p1 = (*dst++)->getData();
	uint8 *p2 = src1->getData();
	uint8 *p3 = src2->getData();
	uint8 *p4 = p1;
	uint8 *p5 = p2;

	for (int i = 0; i < len; i++) {
		int8 val = (int8)*p3++ - (int8)*p2++;
		*p4++ = (uint8)val;
	}

	int16 t = 0;
	int16 d = 256 / numTabs;

	for (int i = 1; i < numTabs - 1; i++) {
		p2 = p5;
		p3 = p1;
		t += d;
		p4 = (*dst++)->getData();

		for (int ii = 0; ii < len; ii++) {
			int16 val = (((int8)*p3++ * t) >> 8) + (int8)*p2++;
			*p4++ = (uint8)val;
		}
	}

	memcpy(p1, p5, len);
	(*dst)->copy(*src2);

	return ++dst;
}

uint8 Screen_LoL::getShapePaletteSize(const uint8 *shp) {
	return shp[10];
}

void Screen_LoL::mergeOverlay(int x, int y, int w, int h) {
	// For now we convert to 16 colors on overlay merging. If that gives
	// any problems, like Screen functionallity not prepared for the
	// format PC98 16 color uses, we'll need to think of a better way.
	//
	// We must do this before merging the overlay, else the font colors
	// will be wrong.
	if (_use16ColorMode)
		convertPC98Gfx(_sjisOverlayPtrs[0] + y * 640 + x, w, h, 640);

	Screen_v2::mergeOverlay(x, y, w, h);
}

void Screen_LoL::convertPC98Gfx(uint8 *data, int w, int h, int pitch) {
	while (h--) {
		for (int i = 0; i < w; ++i) {
			*data = (*data >> 4) & (*data & 0x0F);
			++data;
		}

		data += pitch - w;
	}
}

void Screen_LoL::postProcessCursor(uint8 *data, int w, int h, int pitch) {
	if (!_use16ColorMode)
		return;

	while (h--) {
		for (int i = 0; i < w; ++i) {
			if (*data != _cursorColorKey)
				*data = (*data >> 4) & (*data & 0x0F);
			++data;
		}

		data += pitch - w;
	}
}

} // End of namespace Kyra

#endif // ENABLE_LOL
