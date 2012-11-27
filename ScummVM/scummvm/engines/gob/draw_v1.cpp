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
#include "gob/resources.h"
#include "gob/hotspots.h"
#include "gob/scenery.h"
#include "gob/inter.h"
#include "gob/sound/sound.h"

namespace Gob {

Draw_v1::Draw_v1(GobEngine *vm) : Draw(vm) {
}

void Draw_v1::initScreen() {
	_backSurface  = _vm->_video->initSurfDesc(320, 200);
	_frontSurface = _vm->_global->_primarySurfDesc;

	_cursorSprites = _vm->_video->initSurfDesc(_cursorWidth * 2, _cursorHeight, 2);
	_scummvmCursor = _vm->_video->initSurfDesc(_cursorWidth    , _cursorHeight, SCUMMVM_CURSOR);
}

void Draw_v1::closeScreen() {
}

void Draw_v1::blitCursor() {
	if (_cursorIndex == -1)
		return;

	if (_showCursor == 2)
		_showCursor = 0;
}

void Draw_v1::animateCursor(int16 cursor) {
	int16 cursorIndex = cursor;
	int16 newX = 0, newY = 0;
	uint16 hotspotX = 0, hotspotY = 0;

	_showCursor = 2;

	if (cursorIndex == -1) {
		cursorIndex =
			_vm->_game->_hotspots->findCursor(_vm->_global->_inter_mouseX,
			                                  _vm->_global->_inter_mouseY);

		if (_cursorAnimLow[cursorIndex] == -1)
			cursorIndex = 1;
	}

	if (_cursorAnimLow[cursorIndex] != -1) {
		if (cursorIndex == _cursorIndex) {
			if (_cursorAnimDelays[_cursorIndex] != 0 &&
			    _cursorAnimDelays[_cursorIndex] * 10 +
			    _cursorTimeKey <= _vm->_util->getTimeKey()) {
				_cursorAnim++;
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
			if (_cursorAnimDelays[_cursorIndex] != 0) {
				_cursorAnim =
				    _cursorAnimLow[_cursorIndex];
				_cursorTimeKey = _vm->_util->getTimeKey();
			} else {
				_cursorAnim = _cursorIndex;
			}
		}

		if (_cursorAnimDelays[_cursorIndex] != 0 &&
		    (_cursorAnimHigh[_cursorIndex] < _cursorAnim ||
			_cursorAnimLow[_cursorIndex] >
			_cursorAnim)) {
			_cursorAnim = _cursorAnimLow[_cursorIndex];
		}

		newX = _vm->_global->_inter_mouseX;
		newY = _vm->_global->_inter_mouseY;
		if (_cursorHotspotXVar != -1) {
			newX -= hotspotX = (uint16) VAR(_cursorIndex + _cursorHotspotXVar);
			newY -= hotspotY = (uint16) VAR(_cursorIndex + _cursorHotspotYVar);
		} else if (_cursorHotspotX != -1) {
			newX -= hotspotX = _cursorHotspotX;
			newY -= hotspotY = _cursorHotspotY;
		}

		_scummvmCursor->clear();
		_scummvmCursor->blit(*_cursorSprites,
				cursorIndex * _cursorWidth, 0,
				(cursorIndex + 1) * _cursorWidth - 1,
				_cursorHeight - 1, 0, 0);
		CursorMan.replaceCursor(_scummvmCursor->getData(),
				_cursorWidth, _cursorHeight, hotspotX, hotspotY, 0, false, &_vm->getPixelFormat());

		if (_frontSurface != _backSurface) {
			_showCursor = 3;
			if (!_noInvalidated) {
				int16 tmp = _cursorIndex;
				_cursorIndex = -1;
				blitInvalidated();
				_cursorIndex = tmp;
			} else {
				_vm->_video->waitRetrace();
				if (MIN(newY, _cursorY) < 50)
					_vm->_util->delay(5);
				_showCursor = 0;
			}
		}
	} else
		blitCursor();

	_cursorX = newX;
	_cursorY = newY;
}

void Draw_v1::printTotText(int16 id) {
	byte *dataPtr;
	byte *ptr, *ptrEnd;
	byte cmd;
	int16 destX, destY;
	int16 val;
	int16 savedFlags;
	int16 destSpriteX;
	int16 spriteRight, spriteBottom;
	char buf[20];

	_vm->_sound->cdPlayMultMusic();

	TextItem *textItem = _vm->_game->_resources->getTextItem(id);
	if (!textItem)
		return;

	dataPtr = textItem->getData();
	ptr     = dataPtr;

	destX = READ_LE_UINT16(ptr) & 0x7FFF;
	destY = READ_LE_UINT16(ptr + 2);
	spriteRight = READ_LE_UINT16(ptr + 4);
	spriteBottom = READ_LE_UINT16(ptr + 6);
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

	for (ptrEnd = ptr; *ptrEnd != 1; ptrEnd++) {
		if (*ptrEnd == 3)
			ptrEnd++;

		if (*ptrEnd == 2)
			ptrEnd += 4;
	}
	ptrEnd++;

	while (*ptr != 1) {
		cmd = *ptr;
		if (cmd == 3) {
			ptr++;
			_fontIndex = (*ptr & 0xF0) >> 4;
			_frontColor = *ptr & 0xF;
			ptr++;
			continue;
		} else if (cmd == 2) {
			ptr++;
			_destSpriteX = destX + READ_LE_UINT16(ptr);
			_destSpriteY = destY + READ_LE_UINT16(ptr + 2);
			ptr += 4;
			continue;
		}

		if (*ptr != 0xBA) {
			_letterToPrint = (char) *ptr;
			spriteOperation(DRAW_DRAWLETTER);
			_destSpriteX += _fonts[_fontIndex]->getCharWidth();
			ptr++;
		} else {
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

				_vm->_util->insertStr(",", buf, strlen(buf) + 1 - ptrEnd[17]);
			}

			_textToPrint = buf;
			destSpriteX = _destSpriteX;
			spriteOperation(DRAW_PRINTTEXT);
			if (ptrEnd[17] & 0x80) {
				if (ptr[1] == ' ') {
					_destSpriteX += _fonts[_fontIndex]->getCharWidth();
					while (ptr[1] == ' ')
						ptr++;
					if (ptr[1] == 2) {
						if (READ_LE_UINT16(ptr + 4) == _destSpriteY)
							ptr += 5;
					}
				} else if (ptr[1] == 2 && READ_LE_UINT16(ptr + 4) == _destSpriteY) {
					ptr += 5;
					_destSpriteX += _fonts[_fontIndex]->getCharWidth();
				}
			} else {
				_destSpriteX = destSpriteX + _fonts[_fontIndex]->getCharWidth();
			}
			ptrEnd += 23;
			ptr++;
		}
	}

	delete textItem;
	_renderFlags = savedFlags;

	if (_renderFlags & RENDERFLAG_COLLISIONS)
		_vm->_game->_hotspots->check(0, 0);

	if ((_renderFlags & RENDERFLAG_CAPTUREPOP) && *_vm->_scenery->_pCaptureCounter != 0) {
		(*_vm->_scenery->_pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}
}

void Draw_v1::spriteOperation(int16 operation) {
	int16 len;
	int16 x, y;
	int16 perLine;
	Resource *resource;

	operation &= 0x0F;

	if (_sourceSurface >= 100)
		_sourceSurface -= 80;
	if (_destSurface >= 100)
		_destSurface -= 80;

	if (_renderFlags & RENDERFLAG_USEDELTAS) {
		if (_sourceSurface == kBackSurface) {
			_spriteLeft += _backDeltaX;
			_spriteTop += _backDeltaY;
		}

		if (_destSurface == kBackSurface) {
			_destSpriteX += _backDeltaX;
			_destSpriteY += _backDeltaY;
			if ((operation == DRAW_DRAWLINE) ||
			   ((operation >= DRAW_DRAWBAR) &&
			    (operation <= DRAW_FILLRECTABS))) {
				_spriteRight += _backDeltaX;
				_spriteBottom += _backDeltaY;
			}
		}
	}

	Font *font = 0;
	switch (operation) {
	case DRAW_BLITSURF:
		_spritesArray[_destSurface]->blit(*_spritesArray[_sourceSurface],
		    _spriteLeft, _spriteTop,
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
		_spritesArray[_destSurface]->fillRect(_destSpriteX, _destSpriteY,
		    _destSpriteX + _spriteRight - 1,
		    _destSpriteY + _spriteBottom - 1, _backColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
		break;

	case DRAW_DRAWLINE:
		_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _frontColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_INVALIDATE:
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
		if ((_fontIndex >= kFontCount) || !_fonts[_fontIndex]) {
			warning("Trying to print \"%s\" with undefined font %d", _textToPrint, _fontIndex);
			break;
		}

		font = _fonts[_fontIndex];
		len = strlen(_textToPrint);
		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + len * font->getCharWidth() - 1,
				_destSpriteY + font->getCharHeight() - 1);

		for (int i = 0; i < len; i++) {
			font->drawLetter(*_spritesArray[_destSurface], _textToPrint[i],
					_destSpriteX, _destSpriteY, _frontColor, _backColor, _transparency);

			_destSpriteX += font->getCharWidth();
		}
		break;

	case DRAW_DRAWBAR:
		_spritesArray[_destSurface]->drawLine(_destSpriteX, _spriteBottom,
		    _spriteRight, _spriteBottom, _frontColor);

		_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
		    _destSpriteX, _spriteBottom, _frontColor);

		_spritesArray[_destSurface]->drawLine(_spriteRight, _destSpriteY,
		    _spriteRight, _spriteBottom, _frontColor);

		_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
		    _spriteRight, _destSpriteY, _frontColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_CLEARRECT:
		if (_backColor < 16) {
			_spritesArray[_destSurface]->fillRect(_destSpriteX, _destSpriteY,
			    _spriteRight, _spriteBottom,
			    _backColor);
		}
		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_FILLRECTABS:
		_spritesArray[_destSurface]->fillRect(_destSpriteX, _destSpriteY,
		    _spriteRight, _spriteBottom, _backColor);

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY, _spriteRight, _spriteBottom);
		break;

	case DRAW_DRAWLETTER:
		if ((_fontIndex >= kFontCount) || !_fonts[_fontIndex]) {
			warning("Trying to print \'%c\' with undefined font %d", _letterToPrint, _fontIndex);
			break;
		}

		font = _fonts[_fontIndex];
		if (_fontToSprite[_fontIndex].sprite == -1) {
			dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
					_destSpriteX + font->getCharWidth()  - 1,
					_destSpriteY + font->getCharHeight() - 1);
			font->drawLetter(*_spritesArray[_destSurface], _letterToPrint,
					_destSpriteX, _destSpriteY, _frontColor, _backColor, _transparency);
			break;
		}

		perLine =
			_spritesArray[(int16)_fontToSprite[_fontIndex].sprite]->getWidth() /
			_fontToSprite[_fontIndex].width;

		y = (_letterToPrint - _fontToSprite[_fontIndex].base) / perLine *
			_fontToSprite[_fontIndex].height;

		x = (_letterToPrint - _fontToSprite[_fontIndex].base) % perLine *
			_fontToSprite[_fontIndex].width;

		dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
				_destSpriteX + _fontToSprite[_fontIndex].width,
				_destSpriteY + _fontToSprite[_fontIndex].height);

		_spritesArray[_destSurface]->blit(*_spritesArray[(int16)_fontToSprite[_fontIndex].sprite], x, y,
		    x + _fontToSprite[_fontIndex].width,
		    y + _fontToSprite[_fontIndex].height,
		    _destSpriteX, _destSpriteY, (_transparency == 0) ? -1 : 0);

		break;
	}

	if (_renderFlags & RENDERFLAG_USEDELTAS) {
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

} // End of namespace Gob
