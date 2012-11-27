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


#if !defined(ENABLE_EOB)
#include "kyra/screen.h"
#endif

#ifdef ENABLE_EOB

#include "kyra/eobcommon.h"
#include "kyra/resource.h"

#include "common/system.h"

#include "graphics/cursorman.h"
#include "graphics/palette.h"

namespace Kyra {

Screen_EoB::Screen_EoB(EoBCoreEngine *vm, OSystem *system) : Screen(vm, system, _screenDimTable, _screenDimTableCount) {
	_shapeFadeMode[0] = _shapeFadeMode[1] = 0;
	_shapeFadeInternal = 0;
	_fadeData = 0;
	_fadeDataIndex = 0;
	_dsX1 = _dsX2 = _dsY1 = _dsY2 = 0;
	_gfxX = _gfxY = 0;
	_gfxCol = 0;
	_dsTempPage = 0;
	_dsDiv = 0;
	_dsRem = 0;
	_dsScaleTrans = 0;
	_cgaScaleTable = 0;
	_gfxMaxY = 0;
	_egaDitheringTable = 0;
	_egaDitheringTempPage = 0;
	_cgaMappingDefault = 0;
	_cgaDitheringTables[0] = _cgaDitheringTables[1] = 0;
	_useHiResEGADithering = false;
}

Screen_EoB::~Screen_EoB() {
	delete[] _fadeData;
	delete[] _dsTempPage;
	delete[] _cgaScaleTable;
	delete[] _egaDitheringTable;
	delete[] _egaDitheringTempPage;
	delete[] _cgaDitheringTables[0];
	delete[] _cgaDitheringTables[1];
}

bool Screen_EoB::init() {
	if (Screen::init()) {
		int temp;
		_gfxMaxY = _vm->staticres()->loadRawData(kEoBBaseExpObjectY, temp);

		if (_renderMode != Common::kRenderCGA && _renderMode != Common::kRenderEGA)
			_fadeData = _vm->resource()->fileData("FADING.DAT", 0);

		if (!_fadeData) {
			_fadeData = new uint8[0x700];
			memset(_fadeData, 0, 0x700);
			if (_renderMode != Common::kRenderCGA && _renderMode != Common::kRenderEGA) {
				uint8 *pal = _vm->resource()->fileData("PALETTE1.PAL", 0);
				for (int i = 0; i < 7; i++)
					createFadeTable(pal, &_fadeData[i << 8], 18, (i + 1) * 36);
				delete[] pal;
			}
		}

		_dsTempPage = new uint8[12000];

		if (_vm->gameFlags().useHiRes && _renderMode == Common::kRenderEGA) {
			_useHiResEGADithering = true;
			_egaDitheringTable = new uint8[256];
			_egaDitheringTempPage = new uint8[SCREEN_W * 2 * SCREEN_H * 2];
			for (int i = 0; i < 256; i++)
				_egaDitheringTable[i] = i & 0x0f;
		} else if (_renderMode == Common::kRenderCGA) {
			_cgaMappingDefault = _vm->staticres()->loadRawData(kEoB1CgaMappingDefault, temp);
			_cgaDitheringTables[0] = new uint16[256];
			memset(_cgaDitheringTables[0], 0, 256 * sizeof(uint16));
			_cgaDitheringTables[1] = new uint16[256];
			memset(_cgaDitheringTables[1], 0, 256 * sizeof(uint16));

			_cgaScaleTable = new uint8[256];
			memset(_cgaScaleTable, 0, 256 * sizeof(uint8));
			for (int i = 0; i < 256; i++)
				_cgaScaleTable[i] = ((i & 0xf0) >> 2) | (i & 0x03);
		}

		return true;
	}
	return false;
}

void Screen_EoB::setClearScreenDim(int dim) {
	setScreenDim(dim);
	clearCurDim();
}

void Screen_EoB::clearCurDim() {
	fillRect(_curDim->sx << 3, _curDim->sy, ((_curDim->sx + _curDim->w) << 3) - 1, (_curDim->sy + _curDim->h) - 1, _curDim->unkA);
}

void Screen_EoB::setMouseCursor(int x, int y, const byte *shape) {
	setMouseCursor(x, y, shape, 0);
}

void Screen_EoB::setMouseCursor(int x, int y, const byte *shape, const uint8 *ovl) {
	if (!shape)
		return;

	int mouseW = (shape[2] << 3);
	int mouseH = (shape[3]);
	int colorKey = (_renderMode == Common::kRenderCGA) ? 0 : _cursorColorKey;

	int scaleFactor = _useHiResEGADithering ? 2 : 1;

	uint8 *cursor = new uint8[mouseW * scaleFactor * mouseH * scaleFactor];
	// We use memset and copyBlockToPage instead of fillRect to make sure that the
	// color key 0xFF doesn't get converted into EGA color
	memset(cursor, colorKey, mouseW * scaleFactor * mouseH * scaleFactor);
	copyBlockToPage(6, 0, 0, mouseW * scaleFactor, mouseH * scaleFactor, cursor);
	drawShape(6, shape, 0, 0, 0, 2, ovl);
	CursorMan.showMouse(false);

	if (_useHiResEGADithering)
		ditherRect(getCPagePtr(6), cursor, mouseW * scaleFactor, mouseW, mouseH, colorKey);
	else
		copyRegionToBuffer(6, 0, 0, mouseW, mouseH, cursor);

	// Mouse cursor post processing for CGA mode. Unlike the original (which uses drawShape for the mouse cursor)
	// the cursor manager cannot know whether a pixel value of 0 is supposed to be black or transparent. Thus, we
	// go over the transparency mask again and turn the black pixels to color 4.
	if (_renderMode == Common::kRenderCGA) {
		const uint8 *maskTbl = shape + 4 + ((mouseW * mouseH) >> 2);
		uint8 *dst = cursor;
		uint8 trans = 0;
		uint8 shift = 6;

		uint16 mH = mouseH;
		while (mH--) {
			uint16 mW = mouseW;
			while (mW--) {
				if (shift == 6)
					trans = *maskTbl++;
				if (!*dst && !((trans >> shift) & 3))
					*dst = 4;
				dst++;
				shift = (shift - 2) & 7;
			}
		}
	}

	CursorMan.replaceCursor(cursor, mouseW * scaleFactor, mouseH * scaleFactor, x, y, colorKey);
	if (isMouseVisible())
		CursorMan.showMouse(true);
	delete[] cursor;

	// makes sure that the cursor is drawn
	// we do not use Screen::updateScreen here
	// so we can be sure that changes to page 0
	// are NOT updated on the real screen here
	_system->updateScreen();
}

void Screen_EoB::loadFileDataToPage(Common::SeekableReadStream *s, int pageNum, uint32 size) {
	s->read(_pagePtrs[pageNum], size);
}

void Screen_EoB::printShadedText(const char *string, int x, int y, int col1, int col2) {
	printText(string, x - 1, y, 12, col2);
	printText(string, x, y + 1, 12, 0);
	printText(string, x - 1, y + 1, 12, 0);
	printText(string, x, y, col1, 0);
}

void Screen_EoB::loadShapeSetBitmap(const char *file, int tempPage, int destPage) {
	loadEoBBitmap(file, _cgaMappingDefault, tempPage, destPage, -1);
	_curPage = 2;
}

void Screen_EoB::loadEoBBitmap(const char *file, const uint8 *cgaMapping, int tempPage, int destPage, int convertToPage) {
	const char *filePattern = (_vm->game() == GI_EOB1 && (_renderMode == Common::kRenderEGA || _renderMode == Common::kRenderCGA)) ? "%s.EGA" : "%s.CPS";
	Common::String tmp = Common::String::format(filePattern, file);
	Common::SeekableReadStream *s = _vm->resource()->createReadStream(tmp);
	bool loadAlternative = false;
	if (s) {
		// This additional check is necessary since some localized versions of EOB II seem to contain invalid (size zero) cps files
		if (s->size())
			loadBitmap(tmp.c_str(), tempPage, destPage, 0);
		else
			loadAlternative = true;

		delete s;
	} else {
		loadAlternative = true;
	}

	if (loadAlternative) {
		if (_vm->game() == GI_EOB1) {
			tmp.insertChar('1', tmp.size() - 4);
			loadBitmap(tmp.c_str(), tempPage, destPage, 0);
		} else {
			tmp.setChar('X', 0);
			s = _vm->resource()->createReadStream(tmp);
			if (!s)
				error("Screen_EoB::loadEoBBitmap(): Failed to load file '%s'", file);
			s->seek(768);
			loadFileDataToPage(s, destPage, 64000);
			delete s;
		}
	}

	if (convertToPage == -1) {
		return;
	} else if (convertToPage == 2 && _renderMode == Common::kRenderCGA) {
		convertPage(destPage, 4, cgaMapping);
		copyRegion(0, 0, 0, 0, 320, 200, 4, 2, Screen::CR_NO_P_CHECK);
	} else if (convertToPage == 0) {
		convertPage(destPage, 2, cgaMapping);
		copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
	} else {
		convertPage(destPage, convertToPage, cgaMapping);
	}
}

void Screen_EoB::convertPage(int srcPage, int dstPage, const uint8 *cgaMapping) {
	uint8 *src = getPagePtr(srcPage);
	uint8 *dst = getPagePtr(dstPage);
	if (src == dst)
		return;

	if (_renderMode == Common::kRenderCGA) {
		if (cgaMapping)
			generateCGADitheringTables(cgaMapping);

		uint16 *d = (uint16 *)dst;
		uint8 tblSwitch = 0;
		for (int height = SCREEN_H; height; height--) {
			const uint16 *table = _cgaDitheringTables[(tblSwitch++) & 1];
			for (int width = SCREEN_W / 2; width; width--) {
				WRITE_LE_UINT16(d++, table[((src[1] & 0x0f) << 4) | (src[0] & 0x0f)]);
				src += 2;
			}
		}
	} else if (_renderMode == Common::kRenderEGA && !_useHiResEGADithering) {
		uint32 len = SCREEN_W * SCREEN_H;
		while (len--)
			*dst++ = *src++ & 0x0f;
	} else {
		copyPage(srcPage, dstPage);
	}

	if (dstPage == 0 || dstPage == 1)
		_forceFullUpdate = true;
}

void Screen_EoB::setScreenPalette(const Palette &pal) {
	if (_useHiResEGADithering && pal.getNumColors() != 16) {
		generateEGADitheringTable(pal);
	} else if (_renderMode == Common::kRenderEGA && pal.getNumColors() == 16) {
		_screenPalette->copy(pal);
		_system->getPaletteManager()->setPalette(_screenPalette->getData(), 0, _screenPalette->getNumColors());
	} else if (_renderMode != Common::kRenderCGA && _renderMode != Common::kRenderEGA) {
		Screen::setScreenPalette(pal);
	}
}

void Screen_EoB::getRealPalette(int num, uint8 *dst) {
	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderEGA) {
		const uint8 *pal = _screenPalette->getData();
		for (int i = 0; i < 16; ++i) {
			dst[0] = (pal[0] << 2) | (pal[0] & 3);
			dst[1] = (pal[1] << 2) | (pal[1] & 3);
			dst[2] = (pal[2] << 2) | (pal[2] & 3);
			dst += 3;
			pal += 3;
		}
	} else {
		Screen::getRealPalette(num, dst);
	}
}

uint8 *Screen_EoB::encodeShape(uint16 x, uint16 y, uint16 w, uint16 h, bool encode8bit, const uint8 *cgaMapping) {
	uint8 *shp = 0;
	uint16 shapesize = 0;

	uint8 *srcLineStart = getPagePtr(_curPage | 1) + y * 320 + (x << 3);
	uint8 *src = srcLineStart;

	if (_renderMode == Common::kRenderEGA && !_useHiResEGADithering)
		encode8bit = false;

	if (_renderMode == Common::kRenderCGA) {
		if (cgaMapping)
			generateCGADitheringTables(cgaMapping);
		shapesize = h * (w << 2) + 4;
		shp = new uint8[shapesize];
		memset(shp, 0, shapesize);
		uint8 *dst = shp;

		*dst++ = 4;
		*dst++ = (h & 0xff);
		*dst++ = (w & 0xff);
		*dst++ = (h & 0xff);

		uint8 *dst2 = dst + (h * (w << 1));

		uint8 tblSwitch = 0;
		uint16 h1 = h;
		while (h1--) {
			uint16 w1 = w << 1;
			const uint16 *table = _cgaDitheringTables[(tblSwitch++) & 1];

			while (w1--) {
				uint16 p0 = table[((src[1] & 0x0f) << 4) | (src[0] & 0x0f)];
				uint16 p1 = table[((src[3] & 0x0f) << 4) | (src[2] & 0x0f)];

				*dst++ = ((p0 & 0x0003) << 6) | ((p0 & 0x0300) >> 4) | ((p1 & 0x0003) << 2) | ((p1 & 0x0300) >> 8);

				uint8 msk = 0;
				for (int i = 0; i < 4; i++) {
					if (!src[3 - i])
						msk |= (3 << (i << 1));
				}
				*dst2++ = msk;
				src += 4;
			}
			srcLineStart += SCREEN_W;
			src = srcLineStart;
		}

	} else if (encode8bit) {
		uint16 h1 = h;
		while (h1--) {
			uint8 *lineEnd = src + (w << 3);
			do {
				if (!*src++) {
					shapesize++;
					uint8 *startZeroPos = src;
					while (src != lineEnd && *src == 0)
						src++;

					uint16 numZero = src - startZeroPos + 1;
					if (numZero >> 8)
						shapesize += 2;
				}
				shapesize++;
			} while (src != lineEnd);

			srcLineStart += SCREEN_W;
			src = srcLineStart;
		}

		shapesize += 4;

		shp = new uint8[shapesize];
		memset(shp, 0, shapesize);
		uint8 *dst = shp;

		*dst++ = 8;
		*dst++ = (h & 0xff);
		*dst++ = (w & 0xff);
		*dst++ = (h & 0xff);

		srcLineStart = getPagePtr(_curPage | 1) + y * 320 + (x << 3);
		src = srcLineStart;

		h1 = h;
		while (h1--) {
			uint8 *lineEnd = src + (w << 3);
			do {
				uint8 val = *src++;
				if (!val) {
					*dst++ = 0;
					uint8 *startZeroPos = src;

					while (src != lineEnd && *src == 0)
						src++;

					uint16 numZero = src - startZeroPos + 1;
					if (numZero >> 8) {
						*dst++ = 255;
						*dst++ = 0;
						numZero -= 255;
					}
					val = numZero & 0xff;
				}
				*dst++ = val;
			} while (src != lineEnd);

			srcLineStart += SCREEN_W;
			src = srcLineStart;
		}

	} else {
		uint8 nib = 0, col = 0;
		uint8 *colorMap = 0;

		if (_renderMode != Common::kRenderEGA || _useHiResEGADithering) {
			colorMap = new uint8[0x100];
			memset(colorMap, 0xff, 0x100);
		}

		shapesize = h * (w << 2) + 20;
		shp = new uint8[shapesize];
		memset(shp, 0, shapesize);
		uint8 *dst = shp;

		*dst++ = 2;
		*dst++ = (h & 0xff);
		*dst++ = (w & 0xff);
		*dst++ = (h & 0xff);

		if (_renderMode != Common::kRenderEGA || _useHiResEGADithering) {
			memset(dst, 0xff, 0x10);
		} else {
			for (int i = 0; i < 16; i++)
				dst[i] = i;
		}

		uint8 *pal = dst;
		dst += 16;
		nib = col = 0;

		uint16 h1 = h;
		while (h1--) {
			uint16 w1 = w << 3;
			while (w1--) {
				uint8 s = *src++;
				uint8 c = s & 0x0f;
				if (colorMap) {
					c = colorMap[s];
					if (c == 0xff) {
						if (col < 0x10) {
							*pal++ = s;
							c = colorMap[s] = col++;
							if (!col)
								c = 0;
						} else {
							c = 0;
						}
					}
				}

				if (++nib & 1)
					*dst = c << 4;
				else
					*dst++ |= c;
			}
			srcLineStart += SCREEN_W;
			src = srcLineStart;
		}
		delete[] colorMap;
	}

	return shp;
}

void Screen_EoB::drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, ...) {
	uint8 *dst = getPagePtr(pageNum);
	const uint8 *src = shapeData;

	if (!src)
		return;

	va_list args;
	va_start(args, flags);
	uint8 *ovl = (flags & 2) ? va_arg(args, uint8 *) : 0;
	va_end(args);

	if (sd != -1) {
		const ScreenDim *dm = getScreenDim(sd);
		setShapeFrame(dm->sx, dm->sy, dm->sx + dm->w, dm->sy + dm->h);
		x += (_dsX1 << 3);
		y += _dsY1;
	}

	dst += (_dsX1 << 3);
	int16 dX = x - (_dsX1 << 3);
	int16 dY = y;
	int16 dW = _dsX2 - _dsX1;
	uint8 pixelsPerByte = *src++ ;

	uint16 dH = *src++;
	uint16 width = (*src++) << 3;
	uint16 transOffset = (pixelsPerByte == 4) ? (dH * width) >> 2 : 0;
	src++;

	int rX = x;
	int rY = y;
	int rW = width + 8;
	int rH = dH;

	uint16 w2 = width;
	int d = dY - _dsY1;

	int pixelStep = (flags & 1) ? -1 : 1;

	if (pixelsPerByte == 8)  {
		uint16 marginLeft = 0;
		uint16 marginRight = 0;

		if (d < 0) {
			dH += d;
			if (dH <= 0)
				return;
			d = -d;

			for (int i = 0; i < d; i++) {
				marginLeft = width;
				for (int ii = 0; ii < marginLeft; ii++) {
					if (!*src++)
						marginLeft = marginLeft + 1 - *src++;
				}
			}
			dY = _dsY1;
		}

		d = _dsY2 - dY;

		if (d < 1)
			return;

		if (d < dH)
			dH = d;

		marginLeft = 0;

		if (dX < 0) {
			width += dX;
			marginLeft = -dX;

			if (marginLeft >= w2)
				return;

			dX = 0;
		}

		marginRight = 0;
		d = (dW << 3) - dX;

		if (d < 1)
			return;

		if (d < width) {
			width = d;
			marginRight = w2 - marginLeft - width;
		}

		dst += (dY * SCREEN_W + dX);
		uint8 *dstL = dst;

		if (pageNum == 0 || pageNum == 1)
			addDirtyRect(rX, rY, rW, rH);

		while (dH--) {
			int16 xpos = (int16) marginLeft;

			if (flags & 1) {
				for (int i = 0; i < w2; i++) {
					if (*src++ == 0) {
						i += (*src - 1);
						src += (*src - 1);
					}
				}
				src--;
			}
			const uint8 *src2 = src;

			if (xpos) {
				do {
					uint8 val = (flags & 1) ? *(src - 1) : *src;
					while (val && xpos) {
						src += pixelStep;
						xpos--;
						val = (flags & 1) ? *(src - 1) : *src;
					}

					val = (flags & 1) ? *(src - 1) : *src;
					if (!val) {
						src += pixelStep;
						uint8 bt = (flags & 1) ? src[1] : src[0];
						src += pixelStep;
						xpos = xpos - bt;
					}
				} while (xpos > 0);
			}

			dst -= xpos;
			xpos += width;

			while (xpos > 0) {
				uint8 c = *src;
				uint8 m = (flags & 1) ? *(src - 1) : c;
				src += pixelStep;

				if (m) {
					drawShapeSetPixel(dst, c);
					dst++;
					xpos--;
				} else {
					uint8 len = (flags & 1) ? src[1] : src[0];
					dst += len;
					xpos -= len;
					src += pixelStep;
				}
			}
			xpos += marginRight;

			if (xpos) {
				do {
					uint8 val = (flags & 1) ? *(src - 1) : *src;
					while (val && xpos) {
						src += pixelStep;
						xpos--;
						val = (flags & 1) ? *(src - 1) : *src;
					}

					val = (flags & 1) ? *(src - 1) : *src;
					if (!val) {
						src += pixelStep;
						uint8 bt = (flags & 1) ? src[1] : src[0];
						src += pixelStep;
						xpos = xpos - bt;
					}
				} while (xpos > 0);
			}

			dstL += SCREEN_W;
			dst = dstL;
			if (flags & 1)
				src = src2 + 1;
		}
	} else {
		const uint8 *pal = 0;
		uint8 cgaPal[4];
		memset(cgaPal, 0, 4);

		if (pixelsPerByte == 2) {
			pal = ovl ? ovl : src;
			src += 16;
		} else {
			static const uint8 cgaDefOvl[] = { 0x00, 0x55, 0xaa, 0xff };
			pal = ovl ? ovl : cgaDefOvl;
			for (int i = 0; i < 4; i++)
				cgaPal[i] = pal[i] & 3;
			pal = cgaPal;
		}

		if (d < 0) {
			d = -d;
			if (d >= dH)
				return;
			src += (d * (width / pixelsPerByte));
			d = dY + dH - _dsY1;
			if (d >= 0) {
				dH = d;
				dY = _dsY1;
				d = _dsY2 - dY;
			}
		} else {
			d = _dsY2 - dY;
		}

		if (d < 1)
			return;

		if (d < dH)
			dH = d;

		bool trimL = false;
		uint8 dXbitAlign = dX & (pixelsPerByte - 1);

		if (dX < 0) {
			width += dX;
			d = -dX;
			if (flags & 1)
				src -= (d / pixelsPerByte);
			else
				src += (d / pixelsPerByte);

			if (d >= w2)
				return;

			dX = 0;
			trimL = true;
		}

		d = (dW << 3) - dX;

		if (d < 1)
			return;

		if (d < width)
			width = d;

		dst += (dY * SCREEN_W + dX);

		if (pageNum == 0 || pageNum == 1)
			addDirtyRect(rX, rY, rW, rH);

		int pitch = SCREEN_W - width;
		int16 lineSrcStep = (w2 - width) / pixelsPerByte;
		uint8 lineSrcStepRemainder = (w2 - width) % pixelsPerByte;

		w2 /= pixelsPerByte;
		if (flags & 1)
			src += (w2 - 1);

		uint8 pixelPacking = 8 / pixelsPerByte;
		uint8 pixelPackingMask = 0;

		for (int i = 0; i < pixelPacking; i++)
			pixelPackingMask |= (1 << i);

		if (trimL && (dXbitAlign > lineSrcStepRemainder))
			lineSrcStep--;

		uint8 bitShDef = 8 - pixelPacking;
		if (flags & 1) {
			lineSrcStep = (w2 << 1) - lineSrcStep;
			bitShDef = 0;
		}

		uint8 bitShLineStart = bitShDef;
		if (trimL)
			bitShLineStart -= (dXbitAlign * pixelStep * pixelPacking);

		while (dH--) {
			int16 wd = width;
			uint8 in = 0;
			uint8 trans = 0;
			uint8 shift = bitShLineStart;
			uint8 shSwtch = bitShLineStart;

			while (wd--) {
				if (shift == shSwtch) {
					in = *src;
					trans = src[transOffset];
					src += pixelStep;
					shSwtch = bitShDef;
				}
				uint8 col = (pixelsPerByte == 2) ? pal[(in >> shift) & pixelPackingMask] : (*dst & ((trans >> shift) & (pixelPackingMask))) | pal[(in >> shift) & pixelPackingMask];
				if (col || pixelsPerByte == 4)
					drawShapeSetPixel(dst, col);
				dst++;
				shift = ((shift - (pixelStep * pixelPacking)) & 7);
			}
			src += lineSrcStep;
			dst += pitch;
		}
	}
}

const uint8 *Screen_EoB::scaleShape(const uint8 *shapeData, int steps) {
	setShapeFadeMode(1, steps ? true : false);

	while (shapeData && steps--)
		shapeData = scaleShapeStep(shapeData);

	return shapeData;
}

const uint8 *Screen_EoB::scaleShapeStep(const uint8 *shp) {
	uint8 *dst = (shp != _dsTempPage) ? _dsTempPage : _dsTempPage + 6000;
	uint8 *d = dst;
	uint8 pixelsPerByte = *d++ = *shp++;
	assert(pixelsPerByte > 1);

	uint16 h = shp[0] + 1;
	d[0] = d[2] = (h << 1) / 3;

	uint16 w = shp[1];
	uint16 w2 = (w << 3) / pixelsPerByte;
	uint16 t = ((w << 1) % 3) ? 1 : 0;
	d[1] = ((w << 1) / 3) + t;

	uint32 transOffsetSrc = (pixelsPerByte == 4) ? (shp[0] * shp[1]) << 1 : 0;
	uint32 transOffsetDst = (pixelsPerByte == 4) ? (d[0] * d[1]) << 1 : 0;
	shp += 3;
	d += 3;

	if (pixelsPerByte == 2) {
		int i = 0;
		while (i < 16) {
			if (!shp[i]) {
				i = -i;
				break;
			}
			i++;
		}

		if (i >= 0)
			i = 0;
		else
			i = -i;

		_dsScaleTrans = (i << 4) | (i & 0x0f);
		for (int ii = 0; ii < 16; ii++)
			*d++ = *shp++;
	}

	_dsDiv = w2 / 3;
	_dsRem = w2 % 3;

	while (--h) {
		if (pixelsPerByte == 2)
			scaleShapeProcessLine4Bit(d, shp);
		else
			scaleShapeProcessLine2Bit(d, shp, transOffsetDst, transOffsetSrc);
		if (!--h)
			break;
		if (pixelsPerByte == 2)
			scaleShapeProcessLine4Bit(d, shp);
		else
			scaleShapeProcessLine2Bit(d, shp, transOffsetDst, transOffsetSrc);
		if (!--h)
			break;
		shp += w2;
	}

	return (const uint8 *)dst;
}

const uint8 *Screen_EoB::generateShapeOverlay(const uint8 *shp, int paletteOverlayIndex) {
	if (*shp != 2)
		return 0;

	shp += 4;
	uint8 *ovl = getFadeTable(paletteOverlayIndex);
	for (int i = 0; i < 16; i++)
		_shapeOverlay[i] = ovl[shp[i]];
	return _shapeOverlay;
}

void Screen_EoB::setShapeFrame(int x1, int y1, int x2, int y2) {
	_dsX1 = x1;
	_dsY1 = y1;
	_dsX2 = x2;
	_dsY2 = y2;
}

void Screen_EoB::setShapeFadeMode(uint8 i, bool b) {
	if (!i || i == 1)
		_shapeFadeMode[i] = b;
}

void Screen_EoB::setGfxParameters(int x, int y, int col) {
	_gfxX = x;
	_gfxY = y;
	_gfxCol = col;
}

void Screen_EoB::drawExplosion(int scale, int radius, int numElements, int stepSize, int aspectRatio, const uint8 *colorTable, int colorTableSize) {
	int ymin = 0;
	int ymax = _gfxMaxY[scale];
	int xmin = -100;
	int xmax = 276;

	if (scale)
		--scale;

	hideMouse();

	const ScreenDim *dm = getScreenDim(5);
	int rX1 = dm->sx << 3;
	int rY1 = dm->sy;
	int rX2 = rX1 + (dm->w << 3);
	int rY2 = rY1 + dm->h - 1;

	int16 gx2 = _gfxX;
	int16 gy2 = _gfxY;

	int16 *ptr2 = (int16 *)_dsTempPage;
	int16 *ptr3 = (int16 *)&_dsTempPage[300];
	int16 *ptr4 = (int16 *)&_dsTempPage[600];
	int16 *ptr5 = (int16 *)&_dsTempPage[900];
	int16 *ptr6 = (int16 *)&_dsTempPage[1200];
	int16 *ptr7 = (int16 *)&_dsTempPage[1500];
	int16 *ptr8 = (int16 *)&_dsTempPage[1800];

	if (numElements > 150)
		numElements = 150;

	for (int i = 0; i < numElements; i++) {
		ptr2[i] = ptr3[i] = 0;
		ptr4[i] = _vm->_rnd.getRandomNumberRng(0, radius) - (radius >> 1);
		ptr5[i] = _vm->_rnd.getRandomNumberRng(0, radius) - (radius >> 1) - (radius >> (8 - aspectRatio));
		ptr7[i] = _vm->_rnd.getRandomNumberRng(1024 / stepSize, 2048 / stepSize);
		ptr8[i] = scale << 8;
	}

	for (int l = 2; l;) {
		if (l != 2) {
			for (int i = numElements - 1; i >= 0; i--) {
				int16 px = ((ptr2[i] >> 6) >> scale) + gx2;
				int16 py = ((ptr3[i] >> 6) >> scale) + gy2;
				if (py > ymax)
					py = ymax;
				if (posWithinRect(px, py, rX1, rY1, rX2, rY2))
					setPagePixel(0, px, py, ptr6[i]);
			}
		}

		l = 0;

		for (int i = 0; i < numElements; i++) {
			uint32 end = _system->getMillis() + 1;
			if (ptr4[i] <= 0)
				ptr4[i]++;
			else
				ptr4[i]--;
			ptr2[i] += ptr4[i];
			ptr5[i] += 5;
			ptr3[i] += ptr5[i];
			ptr8[i] += ptr7[i];

			int16 px = ((ptr2[i] >> 6) >> scale) + gx2;
			int16 py = ((ptr3[i] >> 6) >> scale) + gy2;
			if (py >= ymax || py < ymin)
				ptr5[i] = -(ptr5[i] >> 1);
			if (px >= xmax || px < xmin)
				ptr4[i] = -(ptr4[i] >> 1);

			if (py > ymax)
				py = ymax;

			int pxVal1 = 0;
			if (posWithinRect(px, py, 0, 0, 319, 199)) {
				pxVal1 = getPagePixel(2, px, py);
				ptr6[i] = getPagePixel(0, px, py);
			}

			assert((ptr8[i] >> 8) < colorTableSize);
			int pxVal2 = colorTable[ptr8[i] >> 8];
			if (pxVal2) {
				l = 1;
				if (pxVal1 == _gfxCol && posWithinRect(px, py, rX1, rY1, rX2, rY2)) {
					setPagePixel(0, px, py, pxVal2);
					if (i % 5 == 0)  {
						updateScreen();
						uint32 cur = _system->getMillis();
						if (end > cur)
							_system->delayMillis(end - cur);
					}
				}
			} else {
				ptr7[i] = 0;
			}
		}
	}

	showMouse();
}

void Screen_EoB::drawVortex(int numElements, int radius, int stepSize, int, int disorder, const uint8 *colorTable, int colorTableSize) {
	int16 *xCoords = (int16 *)_dsTempPage;
	int16 *yCoords = (int16 *)&_dsTempPage[300];
	int16 *xMod = (int16 *)&_dsTempPage[600];
	int16 *yMod = (int16 *)&_dsTempPage[900];
	int16 *pixBackup = (int16 *)&_dsTempPage[1200];
	int16 *colTableStep = (int16 *)&_dsTempPage[1500];
	int16 *colTableIndex = (int16 *)&_dsTempPage[1800];
	int16 *pixDelay = (int16 *)&_dsTempPage[2100];

	hideMouse();
	int cp = _curPage;

	if (numElements > 150)
		numElements = 150;

	int cx = 88;
	int cy = 48;
	radius <<= 6;

	for (int i = 0; i < numElements; i++) {
		int16 v38 = _vm->_rnd.getRandomNumberRng(radius >> 2, radius);
		int16 stepSum = 0;
		int16 sqsum = 0;
		while (sqsum < v38) {
			stepSum += stepSize;
			sqsum += stepSum;
		}

		switch (_vm->_rnd.getRandomNumber(255) & 3) {
		case 0:
			xCoords[i] = 32;
			yCoords[i] = sqsum;
			xMod[i] = stepSum;
			yMod[i] = 0;
			break;

		case 1:
			xCoords[i] = sqsum;
			yCoords[i] = 32;
			xMod[i] = 0;
			yMod[i] = stepSum;
			break;

		case 2:
			xCoords[i] = 32;
			yCoords[i] = -sqsum;
			xMod[i] = stepSum;
			yMod[i] = 0;
			break;

		default:
			xCoords[i] = -sqsum;
			yCoords[i] = 32;
			xMod[i] = 0;
			yMod[i] = stepSum;
			break;
		}

		if (_vm->_rnd.getRandomBit()) {
			xMod[i] *= -1;
			yMod[i] *= -1;
		}

		colTableStep[i] = _vm->_rnd.getRandomNumberRng(1024 / disorder, 2048 / disorder);
		colTableIndex[i] = 0;
		pixDelay[i] = _vm->_rnd.getRandomNumberRng(0, disorder >> 2);
	}

	int d = 0;
	for (int i = 2; i;) {
		if (i != 2) {
			for (int ii = numElements - 1; ii >= 0; ii--) {
				int16 px = CLIP((xCoords[ii] >> 6) + cx, 0, SCREEN_W - 1);
				int16 py = CLIP((yCoords[ii] >> 6) + cy, 0, SCREEN_H - 1);
				setPagePixel(0, px, py, pixBackup[ii]);
			}
		}

		i = 0;
		int r = (stepSize >> 1) + (stepSize >> 2) + (stepSize >> 3);
		uint32 nextDelay = _system->getMillis() + 1;

		for (int ii = 0; ii < numElements; ii++) {
			if (pixDelay[ii] == 0) {
				if (xCoords[ii] > 0) {
					xMod[ii] -= ((xMod[ii] > 0) ? stepSize : r);
				} else {
					xMod[ii] += ((xMod[ii] < 0) ? stepSize : r);
				}

				if (yCoords[ii] > 0) {
					yMod[ii] -= ((yMod[ii] > 0) ? stepSize : r);
				} else {
					yMod[ii] += ((yMod[ii] < 0) ? stepSize : r);
				}

				xCoords[ii] += xMod[ii];
				yCoords[ii] += yMod[ii];
				colTableIndex[ii] += colTableStep[ii];

			} else {
				pixDelay[ii]--;
			}

			int16 px = CLIP((xCoords[ii] >> 6) + cx, 0, SCREEN_W - 1);
			int16 py = CLIP((yCoords[ii] >> 6) + cy, 0, SCREEN_H - 1);

			uint8 tc1 = ((disorder >> 2) <= d) ? getPagePixel(2, px, py) : 0;
			pixBackup[ii] = getPagePixel(0, px, py);
			uint8 tblIndex = CLIP(colTableIndex[ii] >> 8, 0, colorTableSize - 1);
			uint8 tc2 = colorTable[tblIndex];

			if (tc2) {
				i = 1;
				if (tc1 == _gfxCol && !pixDelay[ii]) {
					setPagePixel(0, px, py, tc2);
					if (ii % 15 == 0)  {
						updateScreen();
						uint32 cur = _system->getMillis();
						if (nextDelay > cur)
							_system->delayMillis(nextDelay - cur);
						nextDelay += 1;
					}
				}
			} else {
				colTableStep[ii] = 0;
			}
		}
		d++;
	}

	_curPage = cp;
	showMouse();
}

void Screen_EoB::fadeTextColor(Palette *pal, int color1, int rate) {
	uint8 *col = pal->getData();

	for (bool loop = true; loop;) {
		loop = true;
		uint32 end = _system->getMillis() + _vm->tickLength();

		loop = false;
		for (int ii = 0; ii < 3; ii++) {
			uint8 c = col[color1 * 3 + ii];
			if (c > rate) {
				col[color1 * 3 + ii] -= rate;
				loop = true;
			} else if (c) {
				col[color1 * 3 + ii] = 0;
				loop = true;
			}
		}

		if (loop) {
			setScreenPalette(*pal);
			updateScreen();
			uint32 cur = _system->getMillis();
			if (end > cur)
				_system->delayMillis(end - cur);
		}
	}
}

bool Screen_EoB::delayedFadePalStep(Palette *fadePal, Palette *destPal, int rate) {
	bool res = false;

	uint8 *s = fadePal->getData();
	uint8 *d = destPal->getData();

	for (int i = 0; i < 765; i++) {
		int fadeVal = *s++;
		int dstCur = *d;
		int diff = ABS(fadeVal - dstCur);

		if (diff == 0) {
			d++;
			continue;
		}

		res = true;
		diff = MIN(diff, rate);

		if (dstCur < fadeVal)
			*d += diff;
		else
			*d -= diff;
		d++;
	}

	return res;
}

int Screen_EoB::getRectSize(int w, int h) {
	return w * h;
}

void Screen_EoB::setFadeTableIndex(int index) {
	_fadeDataIndex = (CLIP(index, 0, 7) << 8);
}

void Screen_EoB::createFadeTable(uint8 *palData, uint8 *dst, uint8 rootColor, uint8 weight) {
	if (!palData)
		return;

	uint8 *src = palData + 3 * rootColor;
	uint8 r = *src++;
	uint8 g = *src++;
	uint8 b = *src;
	uint8 tr, tg, tb;
	src = palData + 3;

	*dst++ = 0;
	weight >>= 1;

	for (uint8 i = 1; i; i++) {
		uint16 tmp = (uint16)((*src - r) * weight) << 1;
		tr = *src++ - ((tmp >> 8) & 0xff);
		tmp = (uint16)((*src - g) * weight) << 1;
		tg = *src++ - ((tmp >> 8) & 0xff);
		tmp = (uint16)((*src - b) * weight) << 1;
		tb = *src++ - ((tmp >> 8) & 0xff);

		uint8 *d = palData + 3;
		uint16 v = 0xffff;
		uint8 col = rootColor;

		for (uint8 ii = 1; ii; ii++) {
			int a = *d++ - tr;
			int t = a * a;
			a = *d++ - tg;
			t += (a * a);
			a = *d++ - tb;
			t += (a * a);

			if (t <= v && (ii == rootColor || ii != i)) {
				v = t;
				col = ii ;
			}
		}
		*dst++ = col;
	}
}

uint8 *Screen_EoB::getFadeTable(int index) {
	return (index >= 0 && index < 5) ? &_fadeData[index << 8] : 0;
}

const uint16 *Screen_EoB::getCGADitheringTable(int index) {
	return !(index & ~1) ? _cgaDitheringTables[index] : 0;
}

const uint8 *Screen_EoB::getEGADitheringTable() {
	return _egaDitheringTable;
}

void Screen_EoB::updateDirtyRects() {
	if (!_useHiResEGADithering) {
		Screen::updateDirtyRects();
		return;
	}

	if (_forceFullUpdate) {
		ditherRect(getCPagePtr(0), _egaDitheringTempPage, SCREEN_W * 2, SCREEN_W, SCREEN_H);
		_system->copyRectToScreen(_egaDitheringTempPage, SCREEN_W * 2, 0, 0, SCREEN_W * 2, SCREEN_H * 2);
	} else {
		const byte *page0 = getCPagePtr(0);
		Common::List<Common::Rect>::iterator it;
		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			ditherRect(page0 + it->top * SCREEN_W + it->left, _egaDitheringTempPage, SCREEN_W * 2, it->width(), it->height());
			_system->copyRectToScreen(_egaDitheringTempPage, SCREEN_W * 2, it->left * 2, it->top * 2, it->width() * 2, it->height() * 2);
		}
	}
	_forceFullUpdate = false;
	_dirtyRects.clear();
}

void Screen_EoB::ditherRect(const uint8 *src, uint8 *dst, int dstPitch, int srcW, int srcH, int colorKey) {
	while (srcH--) {
		uint8 *dst2 = dst + dstPitch;
		for (int i = 0; i < srcW; i++) {
			int in = *src++;
			if (in != colorKey) {
				in = _egaDitheringTable[in];
				*dst++ = *dst2++ = in >> 4;
				*dst++ = *dst2++ = in & 0x0f;
			} else {
				dst[0] = dst[1] = dst2[0] = dst2[1] = colorKey;
				dst += 2;
				dst2 += 2;
			}
		}
		src += (SCREEN_W - srcW);
		dst += ((dstPitch - srcW) * 2);
	}
}

void Screen_EoB::drawShapeSetPixel(uint8 *dst, uint8 col) {
	if ((_renderMode != Common::kRenderCGA && _renderMode != Common::kRenderEGA) || _useHiResEGADithering) {
		if (_shapeFadeMode[0]) {
			if (_shapeFadeMode[1]) {
				col = *dst;
			} else {
				_shapeFadeInternal &= 7;
				col = *(dst + _shapeFadeInternal++);
			}
		}

		if (_shapeFadeMode[1]) {
			uint8 cnt = _shapeFadeMode[1];
			while (cnt--)
				col = _fadeData[_fadeDataIndex + col];
		}
	}

	*dst = col;
}

void Screen_EoB::scaleShapeProcessLine2Bit(uint8 *&shpDst, const uint8 *&shpSrc, uint32 transOffsetDst, uint32 transOffsetSrc) {
	for (int i = 0; i < _dsDiv; i++) {
		shpDst[0] = (_cgaScaleTable[shpSrc[0]] << 2) | (shpSrc[1] >> 6);
		shpDst[1] = ((shpSrc[1] & 0x0f) << 4) | ((shpSrc[2] >> 2) & 0x0f);
		shpDst[transOffsetDst] = (_cgaScaleTable[shpSrc[transOffsetSrc]] << 2) | (shpSrc[transOffsetSrc + 1] >> 6);
		shpDst[transOffsetDst + 1] = ((shpSrc[transOffsetSrc + 1] & 0x0f) << 4) | ((shpSrc[transOffsetSrc + 2] >> 2) & 0x0f);
		shpSrc += 3;
		shpDst += 2;
	}

	if (_dsRem == 1) {
		shpDst[0] = _cgaScaleTable[shpSrc[0]] << 2;
		shpDst[1] = 0;
		shpDst[transOffsetDst] = (_cgaScaleTable[shpSrc[transOffsetSrc]] << 2) | 3;
		shpDst[transOffsetDst + 1] = 0xff;
		shpSrc++;
		shpDst += 2;

	} else if (_dsRem == 2) {
		shpDst[0] = (_cgaScaleTable[shpSrc[0]] << 2) | (shpSrc[1] >> 6);
		shpDst[1] = (shpSrc[1] & 0x3f) << 2;
		shpDst[transOffsetDst] = (_cgaScaleTable[shpSrc[transOffsetSrc]] << 2) | (shpSrc[transOffsetSrc + 1] >> 6);
		shpDst[transOffsetDst + 1] = ((shpSrc[transOffsetSrc + 1] & 0x3f) << 2) | 3;
		shpSrc += 2;
		shpDst += 2;
	}
}

void Screen_EoB::scaleShapeProcessLine4Bit(uint8 *&dst, const uint8 *&src) {
	for (int i = 0; i < _dsDiv; i++) {
		*dst++ = *src++;
		*dst++ = (READ_BE_UINT16(src) >> 4) & 0xff;
		src += 2;
	}

	if (_dsRem == 1) {
		*dst++ = *src++;
		*dst++ = _dsScaleTrans;
	} else if (_dsRem == 2) {
		*dst++ = (src[0] & 0xf0) | (src[1] >> 4);
		src += 2;
		*dst++ = _dsScaleTrans;
		*dst++ = _dsScaleTrans;
		*dst++ = _dsScaleTrans;
	}
}

bool Screen_EoB::posWithinRect(int posX, int posY, int x1, int y1, int x2, int y2) {
	if (posX < x1 || posX > x2 || posY < y1 || posY > y2)
		return false;
	return true;
}

void Screen_EoB::generateEGADitheringTable(const Palette &pal) {
	assert(_egaDitheringTable);
	const uint8 *src = pal.getData();
	uint8 *dst = _egaDitheringTable;

	for (int i = 256; i; i--) {
		int r = *src++;
		int g = *src++;
		int b = *src++;

		uint8 col = 0;
		uint16 min = 0x2E83;

		for (int ii = 256; ii; ii--) {
			const uint8 *palEntry = _egaMatchTable + (ii - 1) * 3;
			if (*palEntry == 0xff)
				continue;

			int e_r = palEntry[0] - r;
			int e_g = palEntry[1] - g;
			int e_b = palEntry[2] - b;

			uint16 s = (e_r * e_r) + (e_g * e_g) + (e_b * e_b);

			if (s <= min) {
				min = s;
				col = ii - 1;
			}
		}
		*dst++ = col;
	}
}

void Screen_EoB::generateCGADitheringTables(const uint8 *mappingData) {
	for (int i = 0; i < 256; i++) {
		_cgaDitheringTables[0][i] = (mappingData[(i >> 4) + 16] << 8) | mappingData[i & 0x0f];
		_cgaDitheringTables[1][i] = (mappingData[i >> 4] << 8) | mappingData[(i & 0x0f) + 16];
	}
}

const uint8 Screen_EoB::_egaMatchTable[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x1E, 0x00, 0x00, 0x1E, 0x1E, 0x1E, 0x00, 0x00, 0x1E,
	0x00, 0x1E, 0x1E, 0x0F, 0x00, 0x1E, 0x1E, 0x1E, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x2D, 0x0F, 0x2D,
	0x0F, 0x0F, 0x2D, 0x2D, 0x2D, 0x0F, 0x0F, 0x2D, 0x0F, 0x2D, 0x2D, 0x2D, 0x0F, 0x2D, 0x2D, 0x2D,
	0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x2A, 0x00, 0x1E, 0x1E, 0x00, 0x1E, 0x2A, 0x1E, 0x00, 0x1E, 0x1E,
	0x00, 0x2A, 0x1E, 0x0F, 0x1E, 0x1E, 0x1E, 0x2A, 0x0F, 0x0F, 0x21, 0x0F, 0x0F, 0x36, 0x0F, 0x2D,
	0x21, 0x0F, 0x2D, 0x36, 0x2D, 0x0F, 0x21, 0x2D, 0x0F, 0x36, 0x2D, 0x2D, 0x21, 0x2D, 0x2D, 0x36,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x2A, 0x00, 0x00, 0x2A, 0x1E, 0x1E, 0x1E, 0x00, 0x1E,
	0x1E, 0x1E, 0x1E, 0x21, 0x00, 0x1E, 0x2A, 0x1E, 0x0F, 0x21, 0x0F, 0x0F, 0x21, 0x2D, 0x0F, 0x36,
	0x0F, 0x0F, 0x36, 0x2D, 0x2D, 0x21, 0x0F, 0x2D, 0x21, 0x2D, 0x2D, 0x36, 0x0F, 0x2D, 0x36, 0x2D,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x2A, 0x2A, 0x1E, 0x1E, 0x1E, 0x1E,
	0x1E, 0x2A, 0x1E, 0x21, 0x1E, 0x1E, 0x2A, 0x2A, 0x0F, 0x21, 0x21, 0x0F, 0x21, 0x36, 0x0F, 0x36,
	0x21, 0x0F, 0x36, 0x36, 0x2D, 0x21, 0x21, 0x2D, 0x21, 0x36, 0x2D, 0x36, 0x21, 0x2D, 0x36, 0x36,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2A, 0x00, 0x00, 0x2A,
	0x00, 0x1E, 0x2A, 0x0F, 0x00, 0x2A, 0x1E, 0x1E, 0x21, 0x0F, 0x0F, 0x21, 0x0F, 0x2D, 0x21, 0x2D,
	0x0F, 0x21, 0x2D, 0x2D, 0x36, 0x0F, 0x0F, 0x36, 0x0F, 0x2D, 0x36, 0x2D, 0x0F, 0x36, 0x2D, 0x2D,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2A,
	0x00, 0x2A, 0x2A, 0x0F, 0x1E, 0x2A, 0x1E, 0x2A, 0x21, 0x0F, 0x21, 0x21, 0x0F, 0x36, 0x21, 0x2D,
	0x21, 0x21, 0x2D, 0x36, 0x36, 0x0F, 0x21, 0x36, 0x0F, 0x36, 0x36, 0x2D, 0x21, 0x36, 0x2D, 0x36,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0x2A, 0x15, 0x00, 0x2A, 0x21, 0x1E, 0x21, 0x15, 0x0F, 0x21, 0x15, 0x2D, 0x21, 0x2F,
	0x0F, 0x21, 0x2F, 0x2D, 0x36, 0x15, 0x0F, 0x36, 0x15, 0x2D, 0x36, 0x2F, 0x0F, 0x36, 0x2F, 0x2D,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x2A, 0x2A, 0x2A, 0x21, 0x21, 0x21, 0x21, 0x21, 0x36, 0x21, 0x36,
	0x21, 0x21, 0x36, 0x36, 0x36, 0x21, 0x21, 0x36, 0x21, 0x36, 0x36, 0x36, 0x21, 0x36, 0x36, 0x36,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x15, 0x15, 0x15, 0x15, 0x15, 0x2F, 0x15, 0x2F,
	0x15, 0x15, 0x2F, 0x2F, 0x2F, 0x15, 0x15, 0x2F, 0x15, 0x2F, 0x2F, 0x2F, 0x15, 0x2F, 0x2F, 0x2F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x15, 0x15, 0x3F, 0x15, 0x2F,
	0x2F, 0x15, 0x2F, 0x3F, 0x2F, 0x15, 0x2F, 0x2F, 0x15, 0x3F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x3F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x15, 0x3F,
	0x15, 0x15, 0x3F, 0x2F, 0x2F, 0x2F, 0x15, 0x2F, 0x2F, 0x2F, 0x2F, 0x3F, 0x15, 0x2F, 0x3F, 0x2F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x15, 0x3F, 0x3F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x3F, 0x2F, 0x3F, 0x2F, 0x2F, 0x3F, 0x3F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x15, 0x15, 0x3F, 0x15, 0x2F, 0x3F, 0x2F, 0x15, 0x3F, 0x2F, 0x2F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x15, 0x3F, 0x3F, 0x2F, 0x2F, 0x3F, 0x2F, 0x3F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x3F, 0x15, 0x3F, 0x3F, 0x2F,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x3F, 0x3F
};

uint16 *OldDOSFont::_cgaDitheringTable = 0;
int OldDOSFont::_numRef = 0;

OldDOSFont::OldDOSFont(Common::RenderMode mode) : _renderMode(mode) {
	_data = 0;
	_width = _height = _numGlyphs = 0;
	_bitmapOffsets = 0;

	_numRef++;
	if (!_cgaDitheringTable && _numRef == 1) {
		_cgaDitheringTable = new uint16[256];
		memset(_cgaDitheringTable, 0, 256 * sizeof(uint16));
		static const uint bits[] = { 0, 3, 12, 15 };
		for (int i = 0; i < 256; i++)
			_cgaDitheringTable[i] = (bits[i & 3] << 8) | (bits[(i >> 2) & 3] << 12) | (bits[(i >> 4) & 3] << 0) | (bits[(i >> 6) & 3] << 4);
	}
}

OldDOSFont::~OldDOSFont() {
	unload();

	if (_numRef)
		--_numRef;

	if (_cgaDitheringTable && !_numRef) {
		delete[] _cgaDitheringTable;
		_cgaDitheringTable = 0;
	}
}

bool OldDOSFont::load(Common::SeekableReadStream &file) {
	unload();

	_data = new uint8[file.size()];
	assert(_data);

	file.read(_data, file.size());
	if (file.err())
		return false;

	if (file.size() - 2 != READ_LE_UINT16(_data))
		return false;

	_width = _data[0x103];
	_height = _data[0x102];
	_numGlyphs = 255;

	_bitmapOffsets = (uint16 *)(_data + 2);

	for (int i = 0; i < _numGlyphs; ++i)
		_bitmapOffsets[i] = READ_LE_UINT16(&_bitmapOffsets[i]);

	return true;
}

int OldDOSFont::getCharWidth(uint16 c) const {
	if (c >= _numGlyphs)
		return 0;
	return _width;
}

void OldDOSFont::drawChar(uint16 c, byte *dst, int pitch) const {
	static const uint8 renderMaskTable6[] = { 0xFC, 0x00, 0x7E, 0x00, 0x3F, 0x00, 0x1F, 0x80, 0x0F, 0xC0, 0x07, 0xE0, 0x03, 0xF0, 0x01, 0xF8 };
	static const uint8 renderMaskTable8[] = { 0xFF, 0x00, 0x7F, 0x80, 0x3F, 0xC0, 0x1F, 0xE0, 0x0F, 0xF0, 0x07, 0xF8, 0x03, 0xFC, 0x01, 0xFE };

	if (_width != 8 && _width != 6)
		error("EOB font rendering not implemented for other font widths than 6 and 8.");

	if (_width == 6) {
		switch (c) {
		case 0x81:
		case 0x9a:
			c = 0x5d;
			break;
		case 0x84:
		case 0x8e:
			c = 0x5b;
			break;
		case 0x94:
		case 0x99:
			c = 0x40;
		case 0xe1:
			// TODO: recheck this: no conversion for 'ß' ?
			break;
		}
	} else if (_width == 8) {
		switch (c) {
		case 0x81:
		case 0x9a:
		case 0x5d:
			c = 0x1d;
			break;
		case 0x84:
		case 0x5b:
			c = 0x1e;
			break;
		case 0x94:
		case 0x40:
			c = 0x1f;
			break;
		case 0x8e:
			c = 0x1b;
			break;
		case 0x99:
			c = 0x1c;
			break;
		case 0xe1:
			c = 0x19;
			break;
		}
	}

	const uint8 *src = &_data[_bitmapOffsets[c]];
	uint8 *dst2 = dst + pitch;

	int w = (_width - 1) >> 3;
	pitch -= _width;

	uint8 color1 = _colorMap[1];
	uint8 color2 = _colorMap[0];

	static const uint16 cgaColorMask[] = { 0, 0x5555, 0xAAAA, 0xFFFF };
	uint16 cgaMask1 = cgaColorMask[color1 & 3];
	uint16 cgaMask2 = cgaColorMask[color2 & 3];

	if (_renderMode == Common::kRenderCGA || _renderMode == Common::kRenderEGA) {
		color1 &= 0x0f;
		color2 &= 0x0f;
	}

	int cH = _height;
	while (cH--) {
		int cW = w;
		uint8 last = 0;
		const uint8 *mtbl = _width == 8 ? renderMaskTable8 : renderMaskTable6;

		if (_renderMode == Common::kRenderCGA) {
			uint8 s = *src++;
			uint8 m = *mtbl++;

			uint8 in = s | last;
			uint16 cmp1 = 0;
			uint16 cmp2 = 0;

			if (color1) {
				in &= m;
				cmp1 = _cgaDitheringTable[in];
			}

			if (color2) {
				in = ~in & m;
				cmp2 = _cgaDitheringTable[in];
			}

			uint16 cDst = 0;
			uint8 sh = 6;
			for (int i = 0; i < _width; i++) {
				cDst |= ((dst[i] & 3) << sh);
				sh = (sh - 2) & 0x0f;
			}

			uint16 out = (~(cmp1 | cmp2) & cDst) | (cmp1 & cgaMask1) | (cmp2 & cgaMask2);

			sh = 6;
			for (int i = 0; i < _width; i++) {
				*dst++ = (out >> sh) & 3;
				sh = (sh - 2) & 0x0f;
			}

			last = s;

		} else {
			for (bool runWidthLoop = true; runWidthLoop;) {
				uint8 s = *src++;
				uint8 m = *mtbl++;

				for (uint8 i = 0x80; i; i >>= 1) {
					if (!(m & i)) {
						runWidthLoop = false;
						break;
					}

					if (s & i) {
						if (color1)
							*dst = color1;
					} else if (color2) {
						*dst = color2;
					}
					dst++;
				}

				if (cW)
					cW--;
				else
					runWidthLoop = false;
			}
		}

		dst += pitch;
		dst2 += pitch;
	}
}

void OldDOSFont::unload() {
	delete[] _data;
	_data = 0;
	_width = _height = _numGlyphs = 0;
	_bitmapOffsets = 0;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
