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

#include "common/endian.h"
#include "common/str.h"

#include "gob/gob.h"
#include "gob/draw.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/game.h"
#include "gob/resources.h"
#include "gob/script.h"
#include "gob/inter.h"
#include "gob/video.h"
#include "gob/palanim.h"

namespace Gob {

Draw::Draw(GobEngine *vm) : _vm(vm) {
	_renderFlags = 0;

	_fontIndex = 0;
	_spriteLeft = 0;
	_spriteTop = 0;
	_spriteRight = 0;
	_spriteBottom = 0;
	_destSpriteX = 0;
	_destSpriteY = 0;
	_backColor = 0;
	_frontColor = 0;
	_transparency = 0;

	_sourceSurface = 0;
	_destSurface = 0;

	_letterToPrint = 0;
	_textToPrint = 0;
	_hotspotText = 0;

	_backDeltaX = 0;
	_backDeltaY = 0;

	for (int i = 0; i < kFontCount; i++)
		_fonts[i] = 0;

	_spritesArray.resize(kSpriteCount);

	_invalidatedCount = 0;
	for (int i = 0; i < 30; i++) {
		_invalidatedTops[i] = 0;
		_invalidatedLefts[i] = 0;
		_invalidatedRights[i] = 0;
		_invalidatedBottoms[i] = 0;
	}

	_noInvalidated = false;
	_noInvalidated57 = false;
	_paletteCleared = false;
	_applyPal = false;

	for (int i = 0; i < 18; i++)
		_unusedPalette1[i] = 0;
	for (int i = 0; i < 16; i++)
		_unusedPalette2[i] = 0;
	for (int i = 0; i < 256; i++) {
		_vgaPalette[i].red = 0;
		_vgaPalette[i].blue = 0;
		_vgaPalette[i].green = 0;
	}
	for (int i = 0; i < 16; i++) {
		_vgaSmallPalette[i].red = 0;
		_vgaSmallPalette[i].blue = 0;
		_vgaSmallPalette[i].green = 0;
	}

	_showCursor = 0;
	_cursorIndex = 0;
	_transparentCursor = 0;
	_cursorTimeKey = 0;

	_cursorX = 0;
	_cursorY = 0;
	_cursorWidth = 0;
	_cursorHeight = 0;

	_cursorHotspotXVar = -1;
	_cursorHotspotYVar = -1;

	_cursorHotspotX = -1;
	_cursorHotspotY = -1;

	_cursorAnim = 0;
	for (int i = 0; i < 40; i++) {
		_cursorAnimLow[i] = 0;
		_cursorAnimHigh[i] = 0;
		_cursorAnimDelays[i] = 0;
	}

	_cursorCount         = 0;
	_doCursorPalettes    = 0;
	_cursorPalettes      = 0;
	_cursorKeyColors     = 0;
	_cursorPaletteStarts = 0;
	_cursorPaletteCounts = 0;
	_cursorHotspotsX     = 0;
	_cursorHotspotsY     = 0;

	_palLoadData1[0] = 0;
	_palLoadData1[1] = 17;
	_palLoadData1[2] = 34;
	_palLoadData1[3] = 51;
	_palLoadData2[0] = 0;
	_palLoadData2[1] = 68;
	_palLoadData2[2] = 136;
	_palLoadData2[3] = 204;

	_needAdjust = 2;
	_scrollOffsetX = 0;
	_scrollOffsetY = 0;

	_pattern = 0;
}

Draw::~Draw() {
	delete[] _cursorPalettes;
	delete[] _doCursorPalettes;
	delete[] _cursorKeyColors;
	delete[] _cursorPaletteStarts;
	delete[] _cursorPaletteCounts;
	delete[] _cursorHotspotsX;
	delete[] _cursorHotspotsY;

	for (int i = 0; i < kFontCount; i++)
		delete _fonts[i];
}

void Draw::invalidateRect(int16 left, int16 top, int16 right, int16 bottom) {
	if (_renderFlags & RENDERFLAG_NOINVALIDATE) {
		_vm->_video->dirtyRectsAll();
		return;
	}

	if (left > right)
		SWAP(left, right);
	if (top > bottom)
		SWAP(top, bottom);

	if ((left > (_vm->_video->_surfWidth - 1)) || (right < 0) ||
	    (top > (_vm->_video->_surfHeight - 1)) || (bottom < 0))
		return;

	_noInvalidated = false;

	if (_invalidatedCount >= 30) {
		_invalidatedLefts[0] = 0;
		_invalidatedTops[0] = 0;
		_invalidatedRights[0] = _vm->_video->_surfWidth - 1;
		_invalidatedBottoms[0] = _vm->_video->_surfHeight - 1;
		_invalidatedCount = 1;
		return;
	}

	if (left < 0)
		left = 0;

	if (right > (_vm->_video->_surfWidth - 1))
		right = _vm->_video->_surfWidth - 1;

	if (top < 0)
		top = 0;

	if (bottom > (_vm->_video->_surfHeight - 1))
		bottom = _vm->_video->_surfHeight - 1;

	left &= 0xFFF0;
	right |= 0x000F;

	for (int rect = 0; rect < _invalidatedCount; rect++) {

		if (_invalidatedTops[rect] > top) {
			if (_invalidatedTops[rect] > bottom) {
				for (int i = _invalidatedCount; i > rect; i--) {
					_invalidatedLefts[i] = _invalidatedLefts[i - 1];
					_invalidatedTops[i] = _invalidatedTops[i - 1];
					_invalidatedRights[i] = _invalidatedRights[i - 1];
					_invalidatedBottoms[i] = _invalidatedBottoms[i - 1];
				}
				_invalidatedLefts[rect] = left;
				_invalidatedTops[rect] = top;
				_invalidatedRights[rect] = right;
				_invalidatedBottoms[rect] = bottom;
				_invalidatedCount++;
				return;
			}
			if (_invalidatedBottoms[rect] < bottom)
				_invalidatedBottoms[rect] = bottom;

			if (_invalidatedLefts[rect] > left)
				_invalidatedLefts[rect] = left;

			if (_invalidatedRights[rect] < right)
				_invalidatedRights[rect] = right;

			_invalidatedTops[rect] = top;
			return;
		}

		if (_invalidatedBottoms[rect] < top)
			continue;

		if (_invalidatedBottoms[rect] < bottom)
			_invalidatedBottoms[rect] = bottom;

		if (_invalidatedLefts[rect] > left)
			_invalidatedLefts[rect] = left;

		if (_invalidatedRights[rect] < right)
			_invalidatedRights[rect] = right;

		return;
	}

	_invalidatedLefts[_invalidatedCount] = left;
	_invalidatedTops[_invalidatedCount] = top;
	_invalidatedRights[_invalidatedCount] = right;
	_invalidatedBottoms[_invalidatedCount] = bottom;
	_invalidatedCount++;
}

void Draw::blitInvalidated() {
	if (_noInvalidated57 &&
			((_vm->_global->_videoMode == 5) || (_vm->_global->_videoMode == 7)))
		return;

	if (_cursorIndex == 4)
		blitCursor();

	if (_vm->_inter && _vm->_inter->_terminate)
		return;

	if (_noInvalidated && !_applyPal)
		return;

	if (_vm->isTrueColor())
		_applyPal = false;

	if (_noInvalidated) {
		setPalette();
		_applyPal = false;
		return;
	}

	if (_cursorSprites)
		_showCursor = (_showCursor & ~2) | ((_showCursor & 1) << 1);

	if (_applyPal) {
		clearPalette();
		forceBlit();
		setPalette();
		_invalidatedCount = 0;
		_noInvalidated = true;
		_applyPal = false;
		return;
	}

	_vm->_video->_doRangeClamp = false;
	for (int i = 0; i < _invalidatedCount; i++) {
		_frontSurface->blit(*_backSurface,
		    _invalidatedLefts[i], _invalidatedTops[i],
		    _invalidatedRights[i], _invalidatedBottoms[i],
		    _invalidatedLefts[i], _invalidatedTops[i]);
		_vm->_video->dirtyRectsAdd(_invalidatedLefts[i], _invalidatedTops[i],
				_invalidatedRights[i], _invalidatedBottoms[i]);
	}
	_vm->_video->_doRangeClamp = true;

	_invalidatedCount = 0;
	_noInvalidated = true;
	_applyPal = false;
}

void Draw::setPalette() {
	_vm->validateVideoMode(_vm->_global->_videoMode);

	_vm->_global->_pPaletteDesc->unused1 = _unusedPalette1;
	_vm->_global->_pPaletteDesc->unused2 = _unusedPalette2;
	_vm->_global->_pPaletteDesc->vgaPal = _vgaPalette;
	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	_paletteCleared = false;
}

void Draw::clearPalette() {
	if (!_paletteCleared) {
		_vm->_util->clearPalette();
		_paletteCleared = true;
	}
}

uint32 Draw::getColor(uint8 index) const {
	if (!_vm->isTrueColor())
		return index;

	return _vm->getPixelFormat().RGBToColor(_vgaPalette[index].red   << 2,
	                                        _vgaPalette[index].green << 2,
	                                        _vgaPalette[index].blue  << 2);
}

void Draw::dirtiedRect(int16 surface,
		int16 left, int16 top, int16 right, int16 bottom) {

	dirtiedRect(_spritesArray[surface], left, top, right, bottom);
}

void Draw::dirtiedRect(SurfacePtr surface,
		int16 left, int16 top, int16 right, int16 bottom) {

	if (surface == _backSurface)
		invalidateRect(left, top, right, bottom);
	else if (surface == _frontSurface)
		_vm->_video->dirtyRectsAdd(left, top, right, bottom);
	else if (_vm->_video->_splitSurf && (surface == _vm->_video->_splitSurf))
		_vm->_video->retrace();
}

void Draw::initSpriteSurf(int16 index, int16 width, int16 height,
		int16 flags) {

	_spritesArray[index] = _vm->_video->initSurfDesc(width, height, flags);
	_spritesArray[index]->clear();
}

void Draw::freeSprite(int16 index) {
	assert(index < kSpriteCount);

	_spritesArray[index].reset();

	if (index == kFrontSurface)
		_spritesArray[index] = _frontSurface;
	if (index == kBackSurface)
		_spritesArray[index] = _backSurface;
}

void Draw::adjustCoords(char adjust, int16 *coord1, int16 *coord2) {
	if (_needAdjust == 2)
		return;

	switch (adjust) {
		case 0:
			if (coord2)
				*coord2 *= 2;
			if (coord1)
				*coord1 *= 2;
			break;

		case 1:
			if (coord2)
				*coord2 = (signed) ((unsigned) (*coord2 + 1) / 2);
			if (coord1)
				*coord1 = (signed) ((unsigned) (*coord1 + 1) / 2);
			break;

		case 2:
			if (coord2)
				*coord2 = *coord2 * 2 + 1;
			if (coord1)
				*coord1 = *coord1 * 2 + 1;
			break;
	}
}

int Draw::stringLength(const char *str, uint16 fontIndex) {
	static const int8 japaneseExtraCharLen[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	if (fontIndex >= kFontCount) {
		warning("Draw::stringLength(): Font %d > Count %d", fontIndex, kFontCount);
		return 0;
	}

	if (!_fonts[fontIndex])
		return 0;

	Font &font = *_fonts[fontIndex];

	int len = 0;

	if (_vm->_global->_language == 10) {

		for (int i = 0; str[i] != 0; i++) {
			if (((unsigned char) str[i+1]) < 128) {
				len += japaneseExtraCharLen[4];
				i++;
			} else
				len += font.getCharWidth();
		}

	} else {

		if (!font.isMonospaced())
			while (*str != '\0')
				len += font.getCharWidth(*str++);
		else
			len = strlen(str) * font.getCharWidth();

	}

	return len;
}

void Draw::printTextCentered(int16 id, int16 left, int16 top, int16 right,
		int16 bottom, const char *str, int16 fontIndex, int16 color) {

	adjustCoords(1, &left, &top);
	adjustCoords(1, &right, &bottom);

	uint16 centerOffset = _vm->_game->_script ? _vm->_game->_script->getFunctionOffset(TOTFile::kFunctionCenter) : 0;
	if (centerOffset != 0) {
		_vm->_game->_script->call(centerOffset);

		WRITE_VAR(17, (uint32) id);
		WRITE_VAR(18, (uint32) left);
		WRITE_VAR(19, (uint32) top);
		WRITE_VAR(20, (uint32) (right - left + 1));
		WRITE_VAR(21, (uint32) (bottom - top + 1));
		_vm->_inter->funcBlock(0);

		_vm->_game->_script->pop();
	}

	if (str[0] == '\0')
		return;

	if (fontIndex >= kFontCount) {
		warning("Draw::printTextCentered(): Font %d > Count %d", fontIndex, kFontCount);
		return;
	}

	if (!_fonts[fontIndex])
		return;

	_transparency = 1;
	_destSpriteX = left;
	_destSpriteY = top;
	_fontIndex = fontIndex;
	_frontColor = color;
	_textToPrint = str;

	Font &font = *_fonts[fontIndex];

	int16 width = 0;
	if (!font.isMonospaced()) {
		const char *s = str;
		while (*s != '\0')
			width += font.getCharWidth(*s++);
	}
	else
		width = strlen(str) * font.getCharWidth();

	adjustCoords(1, &width, 0);
	_destSpriteX += (right - left + 1 - width) / 2;

	spriteOperation(DRAW_PRINTTEXT);
}

void Draw::oPlaytoons_sub_F_1B(uint16 id, int16 left, int16 top, int16 right, int16 bottom,
		char *paramStr, int16 fontIndex, int16 var4, int16 shortId) {

	int16 width;
	char tmpStr[128];

	strcpy(tmpStr, paramStr);
	adjustCoords(1, &left, &top);
	adjustCoords(1, &right,  &bottom);

	uint16 centerOffset = _vm->_game->_script ? _vm->_game->_script->getFunctionOffset(TOTFile::kFunctionCenter) : 0;
	if (centerOffset != 0) {
		_vm->_game->_script->call(centerOffset);

		WRITE_VAR(17, (uint32) id & 0x7FFF);
		WRITE_VAR(18, (uint32) left);
		WRITE_VAR(19, (uint32) top);
		WRITE_VAR(20, (uint32) (right - left + 1));
		WRITE_VAR(21, (uint32) (bottom - top + 1));

		if (_vm->_game->_script->peekUint16(41) >= '4') {
			WRITE_VAR(22, (uint32) fontIndex);
			WRITE_VAR(23, (uint32) var4);
			if (id & 0x8000)
				WRITE_VAR(24, (uint32) 1);
			else
				WRITE_VAR(24, (uint32) 0);
			WRITE_VAR(25, (uint32) shortId);
			if (_hotspotText)
				Common::strlcpy(_hotspotText, paramStr, 40);
		}
		_vm->_inter->funcBlock(0);
		_vm->_game->_script->pop();
	}

	strcpy(paramStr, tmpStr);

	if (fontIndex >= kFontCount) {
		warning("Draw::oPlaytoons_sub_F_1B(): Font %d > Count %d", fontIndex, kFontCount);
		return;
	}

	if (!_fonts[fontIndex])
		return;

	if (*paramStr) {
		_transparency = 1;
		_fontIndex = fontIndex;
		_frontColor = var4;
		if (_vm->_game->_script->peekUint16(41) >= '4' && strchr(paramStr, 92)) {
			char str[80];
			char *str2;
			int16 strLen= 0;
			int16 offY, deltaY;

			str2 = paramStr;
			do {
				strLen++;
				str2++;
				str2 = strchr(str2, 92);
			} while (str2);
			deltaY = (bottom - right + 1 - (strLen * _fonts[fontIndex]->getCharHeight())) / (strLen + 1);
			offY = right + deltaY;
			for (int i = 0; paramStr[i]; i++) {
				int j = 0;
				while (paramStr[i] && paramStr[i] != 92)
					str[j++] = paramStr[i++];
				str[j] = 0;
				_destSpriteX = left;
				_destSpriteY = offY;
				_textToPrint = str;
				width = stringLength(str, fontIndex);
				adjustCoords(1, &width, NULL);
				_destSpriteX += (top - left + 1 - width) / 2;
				spriteOperation(DRAW_PRINTTEXT);
				offY += deltaY + _fonts[fontIndex]->getCharHeight();
			}
		} else {
			_destSpriteX = left;
			if (_vm->_game->_script->peekUint16(41) >= '4')
				_destSpriteY = right + (bottom - right + 1 - _fonts[fontIndex]->getCharHeight()) / 2;
			else
				_destSpriteY = right;
			_textToPrint = paramStr;
			width = stringLength(paramStr, fontIndex);
			adjustCoords(1, &width, NULL);
			_destSpriteX += (top - left + 1 - width) / 2;
			spriteOperation(DRAW_PRINTTEXT);
		}
	}

	return;
}

int32 Draw::getSpriteRectSize(int16 index) {
	if (!_spritesArray[index])
		return 0;

	return _spritesArray[index]->getWidth() * _spritesArray[index]->getHeight();
}

void Draw::forceBlit(bool backwards) {
	if (!_frontSurface || !_backSurface)
		return;
	if (_frontSurface == _backSurface)
		return;
	/*
	if (_spritesArray[kFrontSurface] != _frontSurface)
		return;
	if (_spritesArray[kBackSurface] != _backSurface)
		return;
	*/

	if (!backwards) {
		_frontSurface->blit(*_backSurface);
		_vm->_video->dirtyRectsAll();
	} else
		_backSurface->blit(*_frontSurface);

}

const int16 Draw::_wobbleTable[360] = {
	 0x0000,  0x011D,  0x023B,  0x0359,  0x0476,  0x0593,  0x06B0,  0x07CC,  0x08E8,
	 0x0A03,  0x0B1D,  0x0C36,  0x0D4E,  0x0E65,  0x0F7B,  0x1090,  0x11A4,  0x12B6,
	 0x13C6,  0x14D6,  0x15E3,  0x16EF,  0x17F9,  0x1901,  0x1A07,  0x1B0C,  0x1C0E,
	 0x1D0E,  0x1E0B,  0x1F07,  0x2000,  0x20F6,  0x21EA,  0x22DB,  0x23C9,  0x24B5,
	 0x259E,  0x2684,  0x2766,  0x2846,  0x2923,  0x29FC,  0x2AD3,  0x2BA5,  0x2C75,
	 0x2D41,  0x2E09,  0x2ECE,  0x2F8F,  0x304D,  0x3106,  0x31BC,  0x326E,  0x331C,
	 0x33C6,  0x346C,  0x350E,  0x35AC,  0x3646,  0x36DB,  0x376C,  0x37F9,  0x3882,
	 0x3906,  0x3985,  0x3A00,  0x3A77,  0x3AE9,  0x3B56,  0x3BBF,  0x3C23,  0x3C83,
	 0x3CDE,  0x3D34,  0x3D85,  0x3DD1,  0x3E19,  0x3E5C,  0x3E99,  0x3ED2,  0x3F07,
	 0x3F36,  0x3F60,  0x3F85,  0x3FA6,  0x3FC1,  0x3FD8,  0x3FE9,  0x3FF6,  0x3FFD,
	 0x4000,  0x3FFD,  0x3FF6,  0x3FE9,  0x3FD8,  0x3FC1,  0x3FA6,  0x3F85,  0x3F60,
	 0x3F36,  0x3F07,  0x3ED2,  0x3E99,  0x3E5C,  0x3E19,  0x3DD1,  0x3D85,  0x3D34,
	 0x3CDE,  0x3C83,  0x3C23,  0x3BBF,  0x3B56,  0x3AE9,  0x3A77,  0x3A00,  0x3985,
	 0x3906,  0x3882,  0x37F9,  0x376C,  0x36DB,  0x3646,  0x35AC,  0x350E,  0x346C,
	 0x33C6,  0x331C,  0x326E,  0x31BC,  0x3106,  0x304D,  0x2F8F,  0x2ECE,  0x2E09,
	 0x2D41,  0x2C75,  0x2BA5,  0x2AD3,  0x29FC,  0x2923,  0x2846,  0x2766,  0x2684,
	 0x259E,  0x24B5,  0x23C9,  0x22DB,  0x21EA,  0x20F6,  0x1FFF,  0x1F07,  0x1E0B,
	 0x1D0E,  0x1C0E,  0x1B0C,  0x1A07,  0x1901,  0x17F9,  0x16EF,  0x15E3,  0x14D6,
	 0x13C6,  0x12B6,  0x11A4,  0x1090,  0x0F7B,  0x0E65,  0x0D4E,  0x0C36,  0x0B1D,
	 0x0A03,  0x08E8,  0x07CC,  0x06B0,  0x0593,  0x0476,  0x0359,  0x023B,  0x011D
	-0x0000, -0x011D, -0x023B, -0x0359, -0x0476, -0x0593, -0x06B0, -0x07CC, -0x08E8,
	-0x0A03, -0x0B1D, -0x0C36, -0x0D4E, -0x0E65, -0x0F7B, -0x1090, -0x11A4, -0x12B6,
	-0x13C6, -0x14D6, -0x15E3, -0x16EF, -0x17F9, -0x1901, -0x1A07, -0x1B0C, -0x1C0E,
	-0x1D0E, -0x1E0B, -0x1F07, -0x2000, -0x20F6, -0x21EA, -0x22DB, -0x23C9, -0x24B5,
	-0x259E, -0x2684, -0x2766, -0x2846, -0x2923, -0x29FC, -0x2AD3, -0x2BA5, -0x2C75,
	-0x2D41, -0x2E09, -0x2ECE, -0x2F8F, -0x304D, -0x3106, -0x31BC, -0x326E, -0x331C,
	-0x33C6, -0x346C, -0x350E, -0x35AC, -0x3646, -0x36DB, -0x376C, -0x37F9, -0x3882,
	-0x3906, -0x3985, -0x3A00, -0x3A77, -0x3AE9, -0x3B56, -0x3BBF, -0x3C23, -0x3C83,
	-0x3CDE, -0x3D34, -0x3D85, -0x3DD1, -0x3E19, -0x3E5C, -0x3E99, -0x3ED2, -0x3F07,
	-0x3F36, -0x3F60, -0x3F85, -0x3FA6, -0x3FC1, -0x3FD8, -0x3FE9, -0x3FF6, -0x3FFD,
	-0x4000, -0x3FFD, -0x3FF6, -0x3FE9, -0x3FD8, -0x3FC1, -0x3FA6, -0x3F85, -0x3F60,
	-0x3F36, -0x3F07, -0x3ED2, -0x3E99, -0x3E5C, -0x3E19, -0x3DD1, -0x3D85, -0x3D34,
	-0x3CDE, -0x3C83, -0x3C23, -0x3BBF, -0x3B56, -0x3AE9, -0x3A77, -0x3A00, -0x3985,
	-0x3906, -0x3882, -0x37F9, -0x376C, -0x36DB, -0x3646, -0x35AC, -0x350E, -0x346C,
	-0x33C6, -0x331C, -0x326E, -0x31BC, -0x3106, -0x304D, -0x2F8F, -0x2ECE, -0x2E09,
	-0x2D41, -0x2C75, -0x2BA5, -0x2AD3, -0x29FC, -0x2923, -0x2846, -0x2766, -0x2684,
	-0x259E, -0x24B5, -0x23C9, -0x22DB, -0x21EA, -0x20F6, -0x1FFF, -0x1F07, -0x1E0B,
	-0x1D0E, -0x1C0E, -0x1B0C, -0x1A07, -0x1901, -0x17F9, -0x16EF, -0x15E3, -0x14D6,
	-0x13C6, -0x12B6, -0x11A4, -0x1090, -0x0F7B, -0x0E65, -0x0D4E, -0x0C36, -0x0B1D,
	-0x0A03, -0x08E8, -0x07CC, -0x06B0, -0x0593, -0x0476, -0x0359, -0x023B, -0x011D
};

void Draw::wobble(Surface &surfDesc) {
	int16 amplitude = 32;
	uint16 curFrame = 0;
	uint16 frameWobble = 0;
	uint16 rowWobble = 0;
	int8 *offsets = new int8[_vm->_height];

	_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, -1);

	while (amplitude > 0) {
		rowWobble = frameWobble;
		frameWobble = (frameWobble + 20) % 360;

		for (uint16 y = 0; y < _vm->_height; y++) {
			offsets[y] = amplitude +
				((_wobbleTable[rowWobble] * amplitude) / 0x4000);

			rowWobble = (rowWobble + 20) % 360;
		}

		if (curFrame++ & 16)
			amplitude--;

		for (uint16 y = 0; y < _vm->_height; y++)
			_frontSurface->blit(surfDesc, 0, y, _vm->_width - 1, y, offsets[y], y);

		_vm->_palAnim->fadeStep(0);
		_vm->_video->dirtyRectsAll();
		_vm->_video->waitRetrace();
	}

	_frontSurface->blit(surfDesc);

	_applyPal = false;
	_invalidatedCount = 0;
	_noInvalidated = true;
	_vm->_video->dirtyRectsAll();

	delete[] offsets;
}

Font *Draw::loadFont(const char *path) const {
	if (!_vm->_dataIO->hasFile(path))
		return 0;

	int32 size;
	byte *data = _vm->_dataIO->getFile(path, size);

	return new Font(data);
}

bool Draw::loadFont(uint16 fontIndex, const char *path) {
	if (fontIndex >= kFontCount) {
		warning("Draw::loadFont(): Font %d > Count %d (\"%s\")", fontIndex, kFontCount, path);
		return false;
	}

	delete _fonts[fontIndex];

	_fonts[fontIndex] = loadFont(path);

	return _fonts[fontIndex] != 0;
}

} // End of namespace Gob
