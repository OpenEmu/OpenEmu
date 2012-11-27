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
#include "graphics/cursorman.h"

#include "gob/gob.h"
#include "gob/draw.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/hotspots.h"
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/video.h"

namespace Gob {

Draw_v2::Draw_v2(GobEngine *vm) : Draw_v1(vm) {
	_mayorWorkaroundStatus = 0;
}

void Draw_v2::initScreen() {
	_vm->_game->_preventScroll = false;

	_scrollOffsetX = 0;
	_scrollOffsetY = 0;

	initSpriteSurf(kBackSurface, _vm->_video->_surfWidth, _vm->_video->_surfHeight, 0);
	_backSurface = _spritesArray[kBackSurface];
	_backSurface->clear();

	if (!_spritesArray[kCursorSurface]) {
		initSpriteSurf(kCursorSurface, 32, 16, 2);
		_cursorSpritesBack = _spritesArray[kCursorSurface];
		_cursorSprites = _cursorSpritesBack;
		_scummvmCursor = _vm->_video->initSurfDesc(16, 16, SCUMMVM_CURSOR);
	}

	_spritesArray[kFrontSurface] = _frontSurface;
	_spritesArray[kBackSurface ] = _backSurface;

	_vm->_video->dirtyRectsAll();
}

void Draw_v2::closeScreen() {
	//freeSprite(kCursorSurface);
	//_cursorSprites = 0;
	//_cursorSpritesBack = 0;
	//_scummvmCursor = 0;
	freeSprite(kBackSurface);
}

void Draw_v2::blitCursor() {
	if (!_cursorSprites || (_cursorIndex == -1))
		return;

	_showCursor = (_showCursor & ~2) | ((_showCursor & 1) << 1);
}

void Draw_v2::animateCursor(int16 cursor) {
	if (!_cursorSprites)
		return;

	int16 cursorIndex = cursor;
	int16 newX = 0, newY = 0;
	uint16 hotspotX, hotspotY;

	_showCursor |= 1;

	// .-- _draw_animateCursorSUB1 ---
	if (cursorIndex == -1) {
		cursorIndex =
			_vm->_game->_hotspots->findCursor(_vm->_global->_inter_mouseX,
			                                  _vm->_global->_inter_mouseY);

		if (_cursorAnimLow[cursorIndex] == -1)
			cursorIndex = 1;
	}
	// '------

	if (_cursorAnimLow[cursorIndex] != -1) {
		// .-- _draw_animateCursorSUB2 ---
		if (cursorIndex == _cursorIndex) {
			if ((_cursorAnimDelays[_cursorIndex] != 0) &&
					((_cursorTimeKey + (_cursorAnimDelays[_cursorIndex] * 10)) <=
						_vm->_util->getTimeKey())) {
				_cursorAnim++;
				if ((_cursorAnimHigh[_cursorIndex] < _cursorAnim) ||
						(_cursorAnimLow[_cursorIndex] > _cursorAnim))
					_cursorAnim = _cursorAnimLow[_cursorIndex];
				_cursorTimeKey = _vm->_util->getTimeKey();
			} else {
				if (_noInvalidated && (_vm->_global->_inter_mouseX == _cursorX) &&
						(_vm->_global->_inter_mouseY == _cursorY)) {
					_vm->_video->waitRetrace();
					return;
				}
			}
		} else {
			_cursorIndex = cursorIndex;
			if (_cursorAnimDelays[cursorIndex] != 0) {
				_cursorAnim = _cursorAnimLow[cursorIndex];
				_cursorTimeKey = _vm->_util->getTimeKey();
			}
		}

		if (_cursorAnimDelays[_cursorIndex] != 0) {
			if ((_cursorAnimHigh[_cursorIndex] < _cursorAnim) ||
					(_cursorAnimLow[_cursorIndex] > _cursorAnim))
				_cursorAnim = _cursorAnimLow[_cursorIndex];

			cursorIndex = _cursorAnim;
		}
		// '------

		hotspotX = 0;
		hotspotY = 0;

		if (_cursorHotspotXVar != -1) {
			hotspotX = (uint16) VAR(_cursorIndex + _cursorHotspotXVar);
			hotspotY = (uint16) VAR(_cursorIndex + _cursorHotspotYVar);
		} else if (_cursorHotspotX != -1) {
			hotspotX = _cursorHotspotX;
			hotspotY = _cursorHotspotY;
		} else if (_cursorHotspotsX != 0) {
			hotspotX = _cursorHotspotsX[_cursorIndex];
			hotspotY = _cursorHotspotsY[_cursorIndex];
		}

		newX = _vm->_global->_inter_mouseX - hotspotX;
		newY = _vm->_global->_inter_mouseY - hotspotY;

		_scummvmCursor->clear();
		_scummvmCursor->blit(*_cursorSprites,
				cursorIndex * _cursorWidth, 0,
				(cursorIndex + 1) * _cursorWidth - 1,
				_cursorHeight - 1, 0, 0);

		uint32 keyColor = 0;
		if (_doCursorPalettes && _cursorKeyColors && _doCursorPalettes[cursorIndex])
			keyColor = _cursorKeyColors[cursorIndex];

		CursorMan.replaceCursor(_scummvmCursor->getData(),
				_cursorWidth, _cursorHeight, hotspotX, hotspotY, keyColor, false, &_vm->getPixelFormat());

		if (_doCursorPalettes && _doCursorPalettes[cursorIndex]) {
			CursorMan.replaceCursorPalette(_cursorPalettes + (cursorIndex * 256 * 3),
					_cursorPaletteStarts[cursorIndex], _cursorPaletteCounts[cursorIndex]);
			CursorMan.disableCursorPalette(false);
		} else
			CursorMan.disableCursorPalette(true);

		if (_frontSurface != _backSurface) {
			if (!_noInvalidated) {
				int16 tmp = _cursorIndex;
				_cursorIndex = -1;
				blitInvalidated();
				_cursorIndex = tmp;
			} else {
				_showCursor = 3;
				_vm->_video->waitRetrace();
				if (MIN(newY, _cursorY) < 50)
					_vm->_util->delay(5);
			}
		}
	} else {
		blitCursor();
		_cursorX = newX;
		_cursorY = newY;
	}

	_showCursor &= ~1;
}

void Draw_v2::printTotText(int16 id) {
	byte *dataPtr;
	byte *ptr, *ptrEnd;
	byte cmd;
	int16 savedFlags;
	int16 destX, destY;
	int16 spriteRight, spriteBottom;
	int16 val;
	int16 rectLeft, rectTop, rectRight, rectBottom;
	int16 size;

	id &= 0xFFF;

	_vm->validateLanguage();

	// WORKAROUND: In the scripts of some Gobliins 2 versions, the dialog text IDs
	// for Fingus and the mayor are swapped.
	if ((_vm->getGameType() == kGameTypeGob2) && !_vm->isCD() &&
	     _vm->isCurrentTot("gob07.tot")) {

		if (id == 24) {
			if (_mayorWorkaroundStatus == 1) {
				_mayorWorkaroundStatus = 0;
				id = 31;
			} else
				_mayorWorkaroundStatus = 2;
		} else if (id == 31) {
			if (_mayorWorkaroundStatus == 0) {
				_mayorWorkaroundStatus = 1;
				id = 24;
			} else
				_mayorWorkaroundStatus = 0;
		}

	}

	TextItem *textItem = _vm->_game->_resources->getTextItem(id);
	if (!textItem)
		return;

	size    = textItem->getSize();
	dataPtr = textItem->getData();
	ptr     = dataPtr;
	_pattern = 0;

	bool isSubtitle = (ptr[1] & 0x80) != 0;

	if (isSubtitle && !_vm->_global->_doSubtitles) {
		delete textItem;
		return;
	}

	if (_renderFlags & RENDERFLAG_DOUBLECOORDS) {
		destX = (READ_LE_UINT16(ptr) & 0x7FFF) * 2;
		spriteRight = READ_LE_UINT16(ptr + 4) * 2 + 1;
	} else {
// No mask used for Fascination
		destX = READ_LE_UINT16(ptr);
		if (_vm->getGameType() != kGameTypeFascination)
			destX &= 0x7FFF;
		spriteRight = READ_LE_UINT16(ptr + 4);
	}

	if (_renderFlags & RENDERFLAG_FROMSPLIT) {
		int16 start;

		start = _vm->_video->_splitStart;

		destY = start;
		spriteBottom = READ_LE_UINT16(ptr + 6) - READ_LE_UINT16(ptr + 2);

		if (_renderFlags & RENDERFLAG_DOUBLECOORDS)
			spriteBottom *= 3;

		spriteBottom += start;

		if (_renderFlags & RENDERFLAG_DOUBLECOORDS) {
			spriteBottom += _backDeltaY;
			destY += _backDeltaY;
		}
	} else {
		destY = READ_LE_UINT16(ptr + 2);
		spriteBottom = READ_LE_UINT16(ptr + 6);

		if (_renderFlags & RENDERFLAG_DOUBLECOORDS) {
			destY *= 2;
			spriteBottom *= 2;
		}
	}

	ptr += 8;

	if (_renderFlags & RENDERFLAG_CAPTUREPUSH) {
		_vm->_game->capturePush(destX, destY,
				spriteRight - destX + 1, spriteBottom - destY + 1);
		(*_vm->_scenery->_pCaptureCounter)++;
	}

	_destSpriteX = destX;
	_destSpriteY = destY;
	_spriteRight = spriteRight;
	_spriteBottom = spriteBottom;
	_destSurface = kBackSurface;

	_backColor = *ptr++;
	_transparency = 1;

	spriteOperation(DRAW_CLEARRECT);

	_backColor = 0;
	savedFlags = _renderFlags;
	_renderFlags &= ~RENDERFLAG_NOINVALIDATE;

	while ((_destSpriteX = READ_LE_UINT16(ptr)) != -1) {
		_destSpriteX += destX;
		_destSpriteY = READ_LE_UINT16(ptr + 2) + destY;
		_spriteRight = READ_LE_UINT16(ptr + 4) + destX;
		_spriteBottom = READ_LE_UINT16(ptr + 6) + destY;
		ptr += 8;

		cmd = *ptr++;
		switch ((cmd & 0xF0) >> 4) {
		case 0:
			_frontColor = cmd & 0xF;
			spriteOperation(DRAW_DRAWLINE);
			break;
		case 1:
			_frontColor = cmd & 0xF;
			spriteOperation(DRAW_DRAWBAR);
			break;
		case 2:
			_backColor = cmd & 0xF;
			spriteOperation(DRAW_FILLRECTABS);
			break;
		}
	}
	ptr += 2;

	ptrEnd = ptr;
	while (((ptrEnd - dataPtr) < size) && (*ptrEnd != 1)) {
		// Converting to unknown commands/characters to spaces
		if ((_vm->_game->_script->getVersionMinor() < 2) && (*ptrEnd > 3) && (*ptrEnd < 32))
			*ptrEnd = 32;

		switch (*ptrEnd) {
		case 1:
			break;

		case 2:
		case 5:
			ptrEnd += 5;
			break;

		case 3:
		case 4:
			ptrEnd += 2;
			break;

		case 6:
			ptrEnd++;
			switch (*ptrEnd & 0xC0) {
			case 0x40:
				ptrEnd += 9;
				break;
			case 0x80:
				ptrEnd += 3;
				break;
			case 0xC0:
				ptrEnd += 11;
				break;
			default:
				ptrEnd++;
				break;
			}
			break;

		case 10:
			ptrEnd += (ptrEnd[1] * 2) + 2;
			break;

		default:
			ptrEnd++;
			break;
		}
	}
	ptrEnd++;

	int16 fontIndex = 0, frontColor = 0;
	int16 strPos = 0, strPos2 = -1, strPosBak;
	int16 offX = 0, offY = 0;
	int16 colId = 0;
	int16 colCmd = 0;
	int16 width;
	int16 maskChar = 0;
	char mask[80], str[80], buf[50];

	memset(mask, 0, 80);
	memset(str, ' ', 80);
	_backColor = 0;
	_transparency = 1;

	while (true) {
		if ((((*ptr >= 1) && (*ptr <= 7)) || (*ptr == 10)) && (strPos != 0)) {
			str[MAX(strPos, strPos2)] = 0;
			strPosBak = strPos;
			width = strlen(str) * _fonts[fontIndex]->getCharWidth();
			adjustCoords(1, &width, 0);

			if (colCmd & 0x0F) {
				rectLeft = offX - 2;
				rectTop = offY - 2;
				rectRight = offX + width + 1;
				rectBottom = _fonts[fontIndex]->getCharHeight();
				adjustCoords(1, &rectBottom, 0);
				rectBottom += offY + 1;
				adjustCoords(0, &rectLeft, &rectTop);
				adjustCoords(2, &rectRight, &rectBottom);

				if (colId != -1)
					_vm->_game->_hotspots->add(colId + 0xD000, rectLeft, rectTop,
							rectRight, rectBottom, (uint16) Hotspots::kTypeClick, 0, 0, 0, 0);

				if (_needAdjust != 2)
					printTextCentered(colCmd & 0x0F, rectLeft + 4, rectTop + 4,
							rectRight - 4, rectBottom - 4, str, fontIndex, frontColor);
				else
					printTextCentered(colCmd & 0x0F, rectLeft + 2, rectTop + 2,
							rectRight - 2, rectBottom - 2, str, fontIndex, frontColor);

			} else {
				_destSpriteX = offX;
				_destSpriteY = offY;
				_fontIndex   = fontIndex;
				_frontColor  = frontColor;
				_textToPrint = str;

				if (isSubtitle) {
					_fontIndex  = _subtitleFont;
					_frontColor = _subtitleColor;
				}

				if (_needAdjust != 2) {
					if ((_destSpriteX >= destX) && (_destSpriteY >= destY) &&
					    (((_fonts[_fontIndex]->getCharHeight() / 2) + _destSpriteY - 1) <= spriteBottom)) {
						while (((_destSpriteX + width - 1) > spriteRight) && (width > 0)) {
							width -= _fonts[_fontIndex]->getCharWidth() / 2;
							str[strlen(str) - 1] = '\0';
						}
						spriteOperation(DRAW_PRINTTEXT);
					}
				} else
					spriteOperation(DRAW_PRINTTEXT);

				width = strlen(str);
				for (strPos = 0; strPos < width; strPos++) {
					if (mask[strPos] == '\0')
						continue;

					rectLeft = _fonts[fontIndex]->getCharWidth();
					rectTop = _fonts[fontIndex]->getCharHeight();
					adjustCoords(1, &rectLeft, &rectTop);
					_destSpriteX = strPos * rectLeft + offX;
					_spriteRight = _destSpriteX + rectLeft - 1;
					_spriteBottom = offY + rectTop;
					_destSpriteY = _spriteBottom;
					spriteOperation(DRAW_DRAWLINE);
				}
			}

			rectLeft = 0;
			for (int i = 0; i < strPosBak; i++)
				rectLeft += _fonts[_fontIndex]->getCharWidth(str[i]);

			adjustCoords(1, &rectLeft, 0);
			offX += rectLeft;
			strPos = 0;
			strPos2 = -1;
			memset(mask, 0, 80);
			memset(str, ' ', 80);
		}

		if (*ptr == 1)
			break;

		cmd = *ptr;
		switch ((uint8) cmd) {
		case 2:
		case 5:
			ptr++;
			offX = destX + (int16)READ_LE_UINT16(ptr);
			offY = destY + (int16)READ_LE_UINT16(ptr + 2);
			if (_renderFlags & RENDERFLAG_DOUBLECOORDS) {
				offX += (int16)READ_LE_UINT16(ptr);
				offY += (int16)READ_LE_UINT16(ptr + 2);
			}
			ptr += 4;
			break;

		case 3:
			ptr++;
			fontIndex = ((*ptr & 0xF0) >> 4) & 7;
			frontColor = *ptr & 0x0F;
			ptr++;

			if (isSubtitle) {
				_subtitleFont  = fontIndex;
				_subtitleColor = frontColor;
			}
			break;

		case 4:
			ptr++;
			frontColor = *ptr++;

			if (isSubtitle)
				_subtitleColor = frontColor;
			break;

		case 6:
			ptr++;
			colCmd = *ptr++;
			colId = -1;
			if (colCmd & 0x80) {
				colId = (int16)READ_LE_UINT16(ptr);
				ptr += 2;
			}
			if (colCmd & 0x40) {
				rectLeft = destX + (int16)READ_LE_UINT16(ptr);
				rectRight = destX + (int16)READ_LE_UINT16(ptr + 2);
				rectTop = destY + (int16)READ_LE_UINT16(ptr + 4);
				rectBottom = destY + (int16)READ_LE_UINT16(ptr + 6);
				adjustCoords(2, &rectLeft, &rectTop);
				adjustCoords(2, &rectRight, &rectBottom);
				_vm->_game->_hotspots->add(colId + 0x0D000, rectLeft, rectTop,
						rectRight, rectBottom, (uint16) Hotspots::kTypeClick, 0, 0, 0, 0);
				ptr += 8;
			}
			break;

		case 7:
			ptr++;
			colCmd = 0;
			break;

		case 8:
			ptr++;
			maskChar = 1;
			break;

		case 9:
			ptr++;
			maskChar = 0;
			break;

		case 10:
			str[0] = (char) 255;
			WRITE_LE_UINT16(str + 1, ptr - _vm->_game->_resources->getTexts());
			str[3] = 0;
			ptr++;
			for (int i = *ptr++; i > 0; i--) {
				mask[strPos++] = maskChar;
				ptr += 2;
			}
			break;

		default:
			str[strPos] = (char) cmd;
		case 32:
			mask[strPos++] = maskChar;
			ptr++;
			break;

		case 186:
			cmd = ptrEnd[17] & 0x7F;
			if (cmd == 0) {
				val = READ_LE_UINT16(ptrEnd + 18) * 4;
				sprintf(buf, "%d", (int32)VAR_OFFSET(val));
			} else if (cmd == 1) {
				val = READ_LE_UINT16(ptrEnd + 18) * 4;
				Common::strlcpy(buf, GET_VARO_STR(val), 20);
			} else {
				val = READ_LE_UINT16(ptrEnd + 18) * 4;
				sprintf(buf, "%d", (int32)VAR_OFFSET(val));
				if (buf[0] == '-') {
					while (strlen(buf) - 1 < (uint32)ptrEnd[17]) {
						_vm->_util->insertStr("0", buf, 1);
					}
				} else {
					while (strlen(buf) - 1 < (uint32)ptrEnd[17]) {
						_vm->_util->insertStr("0", buf, 0);
					}
				}
				if (_vm->_global->_language == 2)
					_vm->_util->insertStr(".", buf, strlen(buf) + 1 - ptrEnd[17]);
				else
					_vm->_util->insertStr(",", buf, strlen(buf) + 1 - ptrEnd[17]);
			}
			memcpy(str + strPos, buf, strlen(buf));
			memset(mask, maskChar, strlen(buf));
			if (ptrEnd[17] & 0x80) {
				strPos2 = strPos + strlen(buf);
				strPos++;
				ptrEnd += 23;
				ptr++;
			} else {
				strPos += strlen(buf);
				if (ptr[1] != ' ') {
					if ((ptr[1] == 2) &&
							(((int16)READ_LE_UINT16(ptr + 4)) == _destSpriteY)) {
						ptr += 5;
						str[strPos] = ' ';
						mask[strPos++] = maskChar;
					}
				} else {
					str[strPos] = ' ';
					mask[strPos++] = maskChar;
					while (ptr[1] == ' ')
						ptr++;
					if ((ptr[1] == 2) &&
							(((int16)READ_LE_UINT16(ptr + 4)) == _destSpriteY))
						ptr += 5;
				}
				ptrEnd += 23;
				ptr++;
			}
			break;
		}
	}

	delete textItem;
	_renderFlags = savedFlags;

	if (!(_renderFlags & RENDERFLAG_COLLISIONS))
		return;

	_vm->_game->_hotspots->check(0, 0);

	if (*_vm->_scenery->_pCaptureCounter != 0) {
		(*_vm->_scenery->_pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}
}

void Draw_v2::spriteOperation(int16 operation) {
	int16 len;
	int16 x, y;
	SurfacePtr sourceSurf, destSurf;
	bool deltaVeto;
	int16 left;
	int16 ratio;
	Resource *resource;

	deltaVeto = (operation & 0x10) != 0;
	operation &= 0x0F;

	if (_sourceSurface >= 100)
		_sourceSurface -= 80;
	if (_destSurface >= 100)
		_destSurface -= 80;

	if ((_renderFlags & RENDERFLAG_USEDELTAS) && !deltaVeto) {
		if ((_sourceSurface == kBackSurface) && (operation != DRAW_LOADSPRITE)) {
			_spriteLeft += _backDeltaX;
			_spriteTop += _backDeltaY;
		}

		if (_destSurface == kBackSurface) {
			_destSpriteX += _backDeltaX;
			_destSpriteY += _backDeltaY;
			if ((operation == DRAW_DRAWLINE) ||
			   ((operation >= DRAW_DRAWBAR) && (operation <= DRAW_FILLRECTABS))) {
				_spriteRight += _backDeltaX;
				_spriteBottom += _backDeltaY;
			}
		}
	}

	int16 spriteLeft = _spriteLeft;
	int16 spriteTop = _spriteTop;
	int16 spriteRight = _spriteRight;
	int16 spriteBottom = _spriteBottom;
	int16 destSpriteX = _destSpriteX;
	int16 destSpriteY = _destSpriteY;
	int16 destSurface = _destSurface;
	int16 sourceSurface = _sourceSurface;

	if (_vm->_video->_splitSurf && ((_destSurface == kFrontSurface) || (_destSurface == kBackSurface))) {
		if ((_destSpriteY >= _vm->_video->_splitStart)) {
			_destSpriteY -= _vm->_video->_splitStart;
			if ((operation == DRAW_DRAWLINE) ||
				 ((operation >= DRAW_DRAWBAR) && (operation <= DRAW_FILLRECTABS)))
				_spriteBottom -= _vm->_video->_splitStart;

			_destSurface += 4;
		}

		if ((_spriteTop >= _vm->_video->_splitStart) && (operation == DRAW_BLITSURF)) {
			_spriteTop -= _vm->_video->_splitStart;
			if (_destSurface < 24)
				_destSurface += 4;
		}

	}

	adjustCoords(0, &_destSpriteX, &_destSpriteY);
	if ((operation != DRAW_LOADSPRITE) && (_needAdjust != 2)) {
		adjustCoords(0, &_spriteRight, &_spriteBottom);
		adjustCoords(0, &_spriteLeft, &_spriteTop);

		if (operation == DRAW_DRAWLINE) {
			if ((_spriteRight == _destSpriteX) || (_spriteBottom == _destSpriteY)) {
				operation = DRAW_FILLRECTABS;
				_backColor = _frontColor;
			}
		} else if (operation == DRAW_DRAWLETTER)
			operation = DRAW_BLITSURF;

		if (operation == DRAW_DRAWLINE) {
			if (_spriteBottom < _destSpriteY) {
				SWAP(_spriteBottom, _destSpriteY);
				SWAP(_spriteRight, _destSpriteX);
			}
		} else if ((operation == DRAW_LOADSPRITE) ||
		           (operation > DRAW_PRINTTEXT)) {
			if (_spriteBottom < _destSpriteY)
				SWAP(_spriteBottom, _destSpriteY);
			if (_spriteRight < _destSpriteX)
				SWAP(_spriteRight, _destSpriteX);
			_spriteRight++;
			_spriteBottom++;
		}
	}

	sourceSurf = _spritesArray[_sourceSurface];
	destSurf = _spritesArray[_destSurface];

	if (!destSurf) {
		warning("Can't do operation %d on surface %d: nonexistent", operation, _destSurface);
		return;
	}

	switch (operation) {
	case DRAW_BLITSURF:
	case DRAW_DRAWLETTER:
		if (!sourceSurf || !destSurf)
			break;

		_spritesArray[_destSurface]->blit(*_spritesArray[_sourceSurface],
				_spriteLeft, spriteTop,
				_spriteLeft + _spriteRight - 1,
				_spriteTop + _spriteBottom - 1,
				_destSpriteX, _destSpriteY, (_transparency == 0) ? -1 : 0);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
		break;

	case DRAW_PUTPIXEL:
		_spritesArray[_destSurface]->putPixel(_destSpriteX, _destSpriteY, _frontColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _destSpriteX, _destSpriteY);
		break;

	case DRAW_FILLRECT:
		if (!(_backColor & 0xFF00) || !(_backColor & 0x0100)) {
			_spritesArray[_destSurface]->fillRect(destSpriteX,
					_destSpriteY, _destSpriteX + _spriteRight - 1,
					_destSpriteY + _spriteBottom - 1, getColor(_backColor));
		} else {
			uint8 strength = 16 - (((uint16) _backColor) >> 12);

			_spritesArray[_destSurface]->shadeRect(destSpriteX,
					_destSpriteY, _destSpriteX + _spriteRight - 1,
					_destSpriteY + _spriteBottom - 1, getColor(_backColor), strength);
		}

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
		break;

	case DRAW_DRAWLINE:
		_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _frontColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_INVALIDATE:
		_spritesArray[_destSurface]->drawCircle(_destSpriteX,
				_destSpriteY, _spriteRight, _frontColor);

		dirtiedRect(_destSurface, _destSpriteX - _spriteRight, _destSpriteY - _spriteBottom,
				_destSpriteX + _spriteRight, _destSpriteY + _spriteBottom);
		break;

	case DRAW_LOADSPRITE:
		resource = _vm->_game->_resources->getResource((uint16) _spriteLeft,
			                                             &_spriteRight, &_spriteBottom);

		if (!resource)
			break;

		_vm->_video->drawPackedSprite(resource->getData(),
				_spriteRight, _spriteBottom, _destSpriteX, _destSpriteY,
				_transparency, *_spritesArray[_destSurface]);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);

		delete resource;
		break;

	case DRAW_PRINTTEXT:
		// WORKAROUND: There's mistakes in Little Red's animal names.
		//             See this function for details.
		fixLittleRedStrings();

		len = strlen(_textToPrint);
		left = _destSpriteX;

		if ((_fontIndex >= 4) || (_fontToSprite[_fontIndex].sprite == -1)) {
			if ((_fontIndex >= kFontCount) || !_fonts[_fontIndex]) {
				warning("Trying to print \"%s\" with undefined font %d", _textToPrint, _fontIndex);
				break;
			}

			Font *font = _fonts[_fontIndex];
			if (font->isMonospaced()) {
				if (((int8) _textToPrint[0]) == -1) {
					_vm->validateLanguage();

					byte *dataBuf = _vm->_game->_resources->getTexts() + _textToPrint[1] + 1;
					len = *dataBuf++;
					for (int i = 0; i < len; i++, dataBuf += 2) {
						font->drawLetter(*_spritesArray[_destSurface], READ_LE_UINT16(dataBuf),
								_destSpriteX, _destSpriteY, getColor(_frontColor),
								getColor(_backColor), _transparency);
					}
				} else {
					font->drawString(_textToPrint, _destSpriteX, _destSpriteY, getColor(_frontColor),
							getColor(_backColor), _transparency, *_spritesArray[_destSurface]);
					_destSpriteX += len * font->getCharWidth();
				}
			} else {
				for (int i = 0; i < len; i++) {
					font->drawLetter(*_spritesArray[_destSurface], _textToPrint[i],
								_destSpriteX, _destSpriteY, getColor(_frontColor),
								getColor(_backColor), _transparency);
					_destSpriteX += font->getCharWidth(_textToPrint[i]);
				}
			}

		} else {
			sourceSurf = _spritesArray[_fontToSprite[_fontIndex].sprite];
			ratio = ((sourceSurf == _frontSurface) || (sourceSurf == _backSurface)) ?
				320 : sourceSurf->getWidth();
			ratio /= _fontToSprite[_fontIndex].width;
			for (int i = 0; i < len; i++) {
				y = ((_textToPrint[i] - _fontToSprite[_fontIndex].base) / ratio)
					* _fontToSprite[_fontIndex].height;
				x = ((_textToPrint[i] - _fontToSprite[_fontIndex].base) % ratio)
					* _fontToSprite[_fontIndex].width;
				_spritesArray[_destSurface]->blit(*_spritesArray[_fontToSprite[_fontIndex].sprite], x, y,
						x + _fontToSprite[_fontIndex].width - 1,
						y + _fontToSprite[_fontIndex].height - 1,
						_destSpriteX, _destSpriteY, (_transparency == 0) ? -1 : 0);
				_destSpriteX += _fontToSprite[_fontIndex].width;
			}
		}

		dirtiedRect(_destSurface, left, _destSpriteY,
				_destSpriteX - 1, _destSpriteY + _fonts[_fontIndex]->getCharHeight() - 1);
		break;

	case DRAW_DRAWBAR:
		if (_needAdjust != 2) {
			_spritesArray[_destSurface]->fillRect(_destSpriteX, _spriteBottom - 1,
					_spriteRight, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->fillRect(_destSpriteX, _destSpriteY,
					_destSpriteX + 1, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->fillRect(_spriteRight - 1, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->fillRect(_destSpriteX, _destSpriteY,
					_spriteRight, _destSpriteY + 1, _frontColor);
		} else {
			_spritesArray[_destSurface]->drawLine(_destSpriteX, _spriteBottom,
					_spriteRight, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
					_destSpriteX, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->drawLine(_spriteRight, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);

			_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
					_spriteRight, _destSpriteY, _frontColor);
		}

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_CLEARRECT:
		if ((_backColor != 16) && (_backColor != 144)) {
			_spritesArray[_destSurface]->fillRect(_destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom,
			    getColor(_backColor));
		}

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_FILLRECTABS:
		_spritesArray[_destSurface]->fillRect(_destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, getColor(_backColor));

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;
	}

	_spriteLeft = spriteLeft;
	_spriteTop = spriteTop;
	_spriteRight = spriteRight;
	_spriteBottom = spriteBottom;
	if (operation != DRAW_PRINTTEXT)
		_destSpriteX = destSpriteX;
	_destSpriteY = destSpriteY;
	_destSurface = destSurface;
	_sourceSurface = sourceSurface;

	if ((_renderFlags & RENDERFLAG_USEDELTAS) && !deltaVeto) {
		if (_sourceSurface == kBackSurface) {
			_spriteLeft -= _backDeltaX;
			_spriteTop -= _backDeltaY;
		}

		if (_destSurface == kBackSurface) {
			_destSpriteX -= _backDeltaX;
			_destSpriteY -= _backDeltaY;
		}
	}
}

/* WORKAROUND: Fix wrong German animal names in Once Upon A Time: Little Red Riding Hood.
 *
 * The DOS, Amiga and Atari version of Little Red come with a small screen, accessible
 * through the main menu, that lets children read and listen to animal names in 5
 * languages: French, German, English, Spanish and Italian.
 * Unfortunately, the German names are partially wrong. This is especially tragic
 * because this is a game for small children and they're supposed to learn something
 * here. We fix this.
 *
 * However, there's also problems with the recorded spoken German names:
 * - "Der Rabe" has a far too short "a", sounding more like "Rabbe"
 * - The wrong article for "Schmetterling" is very audible
 * - In general, the words are way too overpronounced
 * These are, of course, way harder to fix.
 */

static const char *kLittleRedStrings[][2] = {
	{"die Heule"          , "die Eule"},
	{"das Schmetterling"  , "der Schmetterling"},
	{"die Vespe"          , "die Wespe"},
	{"das Eich\224rnchen" , "das Eichh\224rnchen"}
};

void Draw_v2::fixLittleRedStrings() {
	if (!_textToPrint || (_vm->getGameType() != kGameTypeLittleRed))
		return;

	for (int i = 0; i < ARRAYSIZE(kLittleRedStrings); i++) {
		if (!strcmp(_textToPrint, kLittleRedStrings[i][0])) {
			_textToPrint = kLittleRedStrings[i][1];
			return;
		}
	}
}

} // End of namespace Gob
