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

#include "gob/draw.h"
#include "gob/game.h"
#include "gob/resources.h"

namespace Gob {

Draw_Playtoons::Draw_Playtoons(GobEngine *vm) : Draw_v2(vm) {
}

void Draw_Playtoons::spriteOperation(int16 operation) {
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
		switch (_pattern & 0xFF) {
		case 0xFF:
			warning("oPlaytoons_spriteOperation: operation DRAW_PUTPIXEL, pattern -1");
			break;
		case 1:
			_spritesArray[_destSurface]->fillRect(destSpriteX,
					_destSpriteY, _destSpriteX + 1,
					_destSpriteY + 1, _frontColor);
			break;
		case 2:
			_spritesArray[_destSurface]->fillRect(destSpriteX - 1,
					_destSpriteY - 1, _destSpriteX + 1,
					_destSpriteY + 1, _frontColor);
			break;
		case 3:
			_spritesArray[_destSurface]->fillRect(destSpriteX - 1,
					_destSpriteY - 1, _destSpriteX + 2,
					_destSpriteY + 2, _frontColor);
			break;
		default:
			_spritesArray[_destSurface]->putPixel(_destSpriteX, _destSpriteY, _frontColor);
			break;
		}
		dirtiedRect(_destSurface, _destSpriteX - (_pattern / 2),
			                      _destSpriteY - (_pattern / 2),
								  _destSpriteX + (_pattern + 1) / 2,
								  _destSpriteY + (_pattern + 1) / 2);
		break;
	case DRAW_FILLRECT:
		switch (_pattern & 0xFF) {
		case 1:
		case 2:
		case 3:
		case 4:
			warning("oPlaytoons_spriteOperation: operation DRAW_FILLRECT, pattern %d", _pattern & 0xFF);
			break;
		case 0:
			_spritesArray[_destSurface]->fillRect(destSpriteX,
					_destSpriteY, _destSpriteX + _spriteRight - 1,
					_destSpriteY + _spriteBottom - 1, _backColor);

			dirtiedRect(_destSurface, _destSpriteX, _destSpriteY,
					_destSpriteX + _spriteRight - 1, _destSpriteY + _spriteBottom - 1);
			break;
		default:
			warning("oPlaytoons_spriteOperation: operation DRAW_FILLRECT, unexpected pattern %d", _pattern & 0xFF);
			break;
		}
		break;

	case DRAW_DRAWLINE:
		if ((_needAdjust != 2) && (_needAdjust < 10)) {
			warning ("oPlaytoons_spriteOperation: operation DRAW_DRAWLINE, draw multiple lines");
				_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);
				_spritesArray[_destSurface]->drawLine(_destSpriteX + 1, _destSpriteY,
					_spriteRight + 1, _spriteBottom, _frontColor);
				_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY + 1,
					_spriteRight, _spriteBottom + 1, _frontColor);
				_spritesArray[_destSurface]->drawLine(_destSpriteX + 1, _destSpriteY + 1,
					_spriteRight + 1, _spriteBottom + 1, _frontColor);
		} else {
			switch (_pattern & 0xFF) {
			case 0:
				_spritesArray[_destSurface]->drawLine(_destSpriteX, _destSpriteY,
					_spriteRight, _spriteBottom, _frontColor);

				break;
			default:
				warning("oPlaytoons_spriteOperation: operation DRAW_DRAWLINE, draw %d lines", (_pattern & 0xFF) * (_pattern & 0xFF));
				for (int16 i = 0; i <= _pattern; i++)
					for (int16 j = 0; j <= _pattern; j++)
						_spritesArray[_destSurface]->drawLine(
								_destSpriteX  - (_pattern / 2) + i,
								_destSpriteY  - (_pattern / 2) + j,
								_spriteRight  - (_pattern / 2) + i,
								_spriteBottom - (_pattern / 2) + j,
								_frontColor);
				break;
			}
		}
		dirtiedRect(_destSurface, MIN(_destSpriteX, _spriteRight)  - _pattern,
								  MIN(_destSpriteY, _spriteBottom) - _pattern,
								  MAX(_destSpriteX, _spriteRight)  + _pattern + 1,
								  MAX(_destSpriteY, _spriteBottom) + _pattern + 1);
		break;

	case DRAW_INVALIDATE:
		if ((_pattern & 0xFF) != 0)
			warning("oPlaytoons_spriteOperation: operation DRAW_INVALIDATE, pattern %d", _pattern & 0xFF);

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
		len = strlen(_textToPrint);
		left = _destSpriteX;

		if ((_fontIndex >= 4) || (_fontToSprite[_fontIndex].sprite == -1)) {
			Font *font = _fonts[_fontIndex];
			if (!font) {
				warning("oPlaytoons_spriteOperation: Trying to print \"%s\" with undefined font %d", _textToPrint, _fontIndex);
				break;
			}

			if (font->isMonospaced()) {
				if (((int8) _textToPrint[0]) == -1) {
					_vm->validateLanguage();

					byte *dataBuf = _vm->_game->_resources->getTexts() + _textToPrint[1] + 1;
					len = *dataBuf++;
					for (int i = 0; i < len; i++, dataBuf += 2) {
						font->drawLetter(*_spritesArray[_destSurface], READ_LE_UINT16(dataBuf),
								_destSpriteX, _destSpriteY, _frontColor, _backColor, _transparency);
					}
				} else {
					font->drawString(_textToPrint, _destSpriteX, _destSpriteY, _frontColor,
							_backColor, _transparency, *_spritesArray[_destSurface]);
					_destSpriteX += len * font->getCharWidth();
				}
			} else {
				for (int i = 0; i < len; i++) {
					font->drawLetter(*_spritesArray[_destSurface], _textToPrint[i],
							_destSpriteX, _destSpriteY, _frontColor, _backColor, _transparency);
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
		if ((_needAdjust != 2) && (_needAdjust < 10)){
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
		warning ("oPlaytoons_spriteOperation: DRAW_CLEARRECT uses _backColor %d", _backColor);
		if (_backColor != -1) {
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

	default:
		warning ("oPlaytoons_spriteOperation: Unhandled operation %d", operation);
		break;
	}

	_spriteLeft = spriteLeft;
	_spriteTop = spriteTop;
	_spriteRight = spriteRight;
	_spriteBottom = spriteBottom;
	_destSpriteX = destSpriteX;
	_destSpriteY = destSpriteY;
	_destSurface = destSurface;
	_sourceSurface = sourceSurface;

	if (operation == DRAW_PRINTTEXT) {
		len = _fonts[_fontIndex]->getCharWidth();
		adjustCoords(1, &len, 0);
		_destSpriteX += len * strlen(_textToPrint);
	}

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

} // End of namespace Gob
