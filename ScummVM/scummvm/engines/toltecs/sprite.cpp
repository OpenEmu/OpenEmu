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
 *
 */

#include "toltecs/toltecs.h"
#include "toltecs/palette.h"
#include "toltecs/render.h"
#include "toltecs/resource.h"

namespace Toltecs {

class SpriteReader : public SpriteFilter {
public:
	SpriteReader(byte *source, const SpriteDrawItem &sprite) : SpriteFilter(sprite), _source(source) {
		_curWidth = _sprite->origWidth;
		_curHeight = _sprite->origHeight;
	}
	SpriteReaderStatus readPacket(PixelPacket &packet) {
		if (_sprite->flags & 0x40) {
			// shadow sprite
			packet.count = _source[0] & 0x7F;
			if (_source[0] & 0x80)
				packet.pixel = 1;
			else
				packet.pixel = 0;
			_source++;
		} else if (_sprite->flags & 0x10) {
			// 256-color sprite
			packet.pixel = *_source++;
			packet.count = *_source++;
		} else {
			// 16-color sprite
			packet.count = _source[0] & 0x0F;
			packet.pixel = (_source[0] & 0xF0) >> 4;
			_source++;
		}
		_curWidth -= packet.count;
		if (_curWidth <= 0) {
			_curHeight--;
			if (_curHeight == 0) {
				return kSrsEndOfSprite;
			} else {
				_curWidth = _sprite->origWidth;
				return kSrsEndOfLine;
			}
		} else {
			return kSrsPixelsLeft;
		}
	}
	byte *getSource() {
		return _source;
	}
	void setSource(byte *source) {
		_source = source;
		_curHeight++;
	}
protected:
	byte *_source;
	int16 _curWidth, _curHeight;
};

class SpriteFilterScaleDown : public SpriteFilter {
public:
	SpriteFilterScaleDown(const SpriteDrawItem &sprite, SpriteReader *reader) : SpriteFilter(sprite), _reader(reader) {
		_height = _sprite->height;
		_yerror = _sprite->yerror;
		_origHeight = _sprite->origHeight;
		_scalerStatus = 0;
	}
	SpriteReaderStatus readPacket(PixelPacket &packet) {
		SpriteReaderStatus status = kSrsPixelsLeft;
		if (_scalerStatus == 0) {
			_xerror = _sprite->xdelta;
			_yerror -= 100;
			while (_yerror <= 0) {
				do {
					status = _reader->readPacket(packet);
				} while (status == kSrsPixelsLeft);
				_yerror += _sprite->ydelta - 100;
			}
			if (status == kSrsEndOfSprite)
				return kSrsEndOfSprite;
			_scalerStatus = 1;
		}
		if (_scalerStatus == 1) {
			status = _reader->readPacket(packet);
			byte updcount = packet.count;
			while (updcount--) {
				_xerror -= 100;
				if (_xerror <= 0) {
					if (packet.count > 0)
						packet.count--;
					_xerror += _sprite->xdelta;
				}
			}
			if (status == kSrsEndOfLine) {
				if (--_height == 0)
					return kSrsEndOfSprite;
				_scalerStatus = 0;
				return kSrsEndOfLine;
			}
		}
		return kSrsPixelsLeft;
	}
protected:
	SpriteReader *_reader;
	int16 _xerror, _yerror;
	int16 _height;
	int16 _origHeight;
	int _scalerStatus;
};

class SpriteFilterScaleUp : public SpriteFilter {
public:
	SpriteFilterScaleUp(const SpriteDrawItem &sprite, SpriteReader *reader) : SpriteFilter(sprite), _reader(reader) {
		_height = _sprite->height;
		_yerror = _sprite->yerror;
		_origHeight = _sprite->origHeight;
		_scalerStatus = 0;
	}
	SpriteReaderStatus readPacket(PixelPacket &packet) {
		SpriteReaderStatus status;
		if (_scalerStatus == 0) {
			_xerror = _sprite->xdelta;
			_sourcep = _reader->getSource();
			_scalerStatus = 1;
		}
		if (_scalerStatus == 1) {
			status = _reader->readPacket(packet);
			byte updcount = packet.count;
			while (updcount--) {
				_xerror -= 100;
				if (_xerror <= 0) {
					packet.count++;
					_xerror += _sprite->xdelta;
				}
			}
			if (status == kSrsEndOfLine) {
				if (--_height == 0)
					return kSrsEndOfSprite;
				_yerror -= 100;
				if (_yerror <= 0) {
					_reader->setSource(_sourcep);
					_yerror += _sprite->ydelta + 100;
				}
				_scalerStatus = 0;
				return kSrsEndOfLine;
			}
		}
		return kSrsPixelsLeft;
	}
protected:
	SpriteReader *_reader;
	byte *_sourcep;
	int16 _xerror, _yerror;
	int16 _height;
	int16 _origHeight;
	int _scalerStatus;
};

bool Screen::createSpriteDrawItem(const DrawRequest &drawRequest, SpriteDrawItem &sprite) {
	int16 scaleValueX, scaleValueY;
	int16 xoffs, yoffs;
	byte *spriteData;
	int16 frameNum;

	memset(&sprite, 0, sizeof(SpriteDrawItem));

	if (drawRequest.flags == 0xFFFF)
		return false;

	frameNum = drawRequest.flags & 0x0FFF;

	sprite.flags = 0;
	sprite.baseColor = drawRequest.baseColor;
	sprite.x = drawRequest.x;
	sprite.y = drawRequest.y;
	sprite.priority = drawRequest.y;
	sprite.resIndex = drawRequest.resIndex;
	sprite.frameNum = frameNum;

	spriteData = _vm->_res->load(drawRequest.resIndex)->data;

	if (drawRequest.flags & 0x1000) {
		sprite.flags |= 4;
	}

	if (drawRequest.flags & 0x2000) {
		sprite.flags |= 0x10;
	}

	if (drawRequest.flags & 0x4000) {
		sprite.flags |= 0x40;
	}

	// First initialize the sprite item with the values from the sprite resource

	SpriteFrameEntry spriteFrameEntry(spriteData + frameNum * 12);

	if (spriteFrameEntry.w == 0 || spriteFrameEntry.h == 0)
		return false;

	sprite.offset = spriteFrameEntry.offset;

	sprite.width = spriteFrameEntry.w;
	sprite.height = spriteFrameEntry.h;
	sprite.origWidth = spriteFrameEntry.w;
	sprite.origHeight = spriteFrameEntry.h;

	if (drawRequest.flags & 0x1000) {
		xoffs = spriteFrameEntry.w - spriteFrameEntry.x;
	} else {
		xoffs = spriteFrameEntry.x;
	}

	yoffs = spriteFrameEntry.y;

	// If the sprite should be scaled we need to initialize some values now

	if (drawRequest.scaling != 0) {

		byte scaleValue = ABS(drawRequest.scaling);

		scaleValueX = scaleValue * sprite.origWidth;
		sprite.xdelta = (10000 * sprite.origWidth) / scaleValueX;
		scaleValueX /= 100;

		scaleValueY = scaleValue * sprite.origHeight;
		sprite.ydelta = (10000 * sprite.origHeight) / scaleValueY;
		scaleValueY /= 100;

		if (drawRequest.scaling > 0) {
			sprite.flags |= 2;
			sprite.width = sprite.origWidth + scaleValueX;
			sprite.height = sprite.origHeight + scaleValueY;
			xoffs += (xoffs * scaleValue) / 100;
			yoffs += (yoffs * scaleValue) / 100;
		} else {
			sprite.flags |= 1;
			sprite.width = sprite.origWidth - scaleValueX;
			sprite.height = sprite.origHeight - 1 - scaleValueY;
			if (sprite.width <= 0 || sprite.height <= 0)
				return false;
			xoffs -= (xoffs * scaleValue) / 100;
			yoffs -= (yoffs * scaleValue) / 100;
		}

	}

	sprite.x -= xoffs;
	sprite.y -= yoffs;

	sprite.yerror = sprite.ydelta;

	// Now we check if the sprite needs to be clipped

	// Clip Y
	if (sprite.y - _vm->_cameraY < 0) {

		int16 clipHeight = ABS(sprite.y - _vm->_cameraY);
		int16 skipHeight = clipHeight;
		byte *spriteFrameData;

		sprite.height -= clipHeight;
		if (sprite.height <= 0)
			return false;

		sprite.y = _vm->_cameraY;

		// If the sprite is scaled
		if (sprite.flags & 3) {
			int16 chopHeight = sprite.ydelta;
			if ((sprite.flags & 2) == 0) {
				do {
					chopHeight -= 100;
					if (chopHeight <= 0) {
						skipHeight++;
						chopHeight += sprite.ydelta;
					} else {
						clipHeight--;
					}
				} while (clipHeight > 0);
			} else {
				do {
					chopHeight -= 100;
					if (chopHeight < 0) {
						skipHeight--;
						chopHeight += sprite.ydelta + 100;
					}
					clipHeight--;
				} while (clipHeight > 0);
			}
			sprite.yerror = chopHeight;
		}

		spriteFrameData = spriteData + sprite.offset;
		// Now the sprite's offset is adjusted to point to the starting line
		if ((sprite.flags & 0x10) == 0) {
			while (skipHeight--) {
				int16 lineWidth = 0;
				while (lineWidth < sprite.origWidth) {
					sprite.offset++;
					lineWidth += spriteFrameData[0] & 0x0F;
					spriteFrameData++;
				}
			}
		} else {
			while (skipHeight--) {
				int16 lineWidth = 0;
				while (lineWidth < sprite.origWidth) {
					sprite.offset += 2;
					lineWidth += spriteFrameData[1];
					spriteFrameData += 2;
				}
			}
		}

	}

	if (sprite.y + sprite.height - _vm->_cameraY - _vm->_cameraHeight > 0)
		sprite.height -= sprite.y + sprite.height - _vm->_cameraY - _vm->_cameraHeight;
	if (sprite.height <= 0)
		return false;

	sprite.skipX = 0;

	if (drawRequest.flags & 0x1000) {
		// Left border
		if (sprite.x - _vm->_cameraX < 0) {
			sprite.width -= ABS(sprite.x - _vm->_cameraX);
			sprite.x = _vm->_cameraX;
		}
		// Right border
		if (sprite.x + sprite.width - _vm->_cameraX - 640 > 0) {
			sprite.flags |= 8;
			sprite.skipX = sprite.x + sprite.width - _vm->_cameraX - 640;
			sprite.width -= sprite.skipX;
		}
	} else {
		// Left border
		if (sprite.x - _vm->_cameraX < 0) {
			sprite.flags |= 8;
			sprite.skipX = ABS(sprite.x - _vm->_cameraX);
			sprite.width -= sprite.skipX;
			sprite.x = _vm->_cameraX;
		}
		// Right border
		if (sprite.x + sprite.width - _vm->_cameraX - 640 > 0) {
			sprite.flags |= 8;
			sprite.width -= sprite.x + sprite.width - _vm->_cameraX - 640;
		}
	}

	if (sprite.width <= 0)
		return false;

	return true;
}

void Screen::addDrawRequest(const DrawRequest &drawRequest) {
	SpriteDrawItem sprite;
	if (createSpriteDrawItem(drawRequest, sprite))
		_renderQueue->addSprite(sprite);
}

void Screen::drawSprite(const SpriteDrawItem &sprite) {

	debug(0, "Screen::drawSprite() x = %d; y = %d; flags = %04X; resIndex = %d; offset = %08X; drawX = %d; drawY = %d",
		sprite.x, sprite.y, sprite.flags, sprite.resIndex, sprite.offset,
		sprite.x - _vm->_cameraX, sprite.y - _vm->_cameraY);
	debug(0, "Screen::drawSprite() width = %d; height = %d; origWidth = %d; origHeight = %d",
		sprite.width, sprite.height, sprite.origWidth, sprite.origHeight);

	byte *source = _vm->_res->load(sprite.resIndex)->data + sprite.offset;
	byte *dest = _frontScreen + sprite.x + sprite.y * 640;

	SpriteReader spriteReader(source, sprite);

	if (sprite.flags & 0x40) {
		// Shadow sprites
		if (sprite.flags & 1) {
			SpriteFilterScaleDown spriteScaler(sprite, &spriteReader);
			drawSpriteCore(dest, spriteScaler, sprite);
		} else if (sprite.flags & 2) {
			SpriteFilterScaleUp spriteScaler(sprite, &spriteReader);
			drawSpriteCore(dest, spriteScaler, sprite);
		} else {
			drawSpriteCore(dest, spriteReader, sprite);
		}
	} else if (sprite.flags & 0x10) {
		// 256 color sprite
		drawSpriteCore(dest, spriteReader, sprite);
	} else {
		// 16 color sprite
		if (sprite.flags & 1) {
			SpriteFilterScaleDown spriteScaler(sprite, &spriteReader);
			drawSpriteCore(dest, spriteScaler, sprite);
		} else if (sprite.flags & 2) {
			SpriteFilterScaleUp spriteScaler(sprite, &spriteReader);
			drawSpriteCore(dest, spriteScaler, sprite);
		} else {
			drawSpriteCore(dest, spriteReader, sprite);
		}
	}

	debug(0, "Screen::drawSprite() ok");

}

void Screen::drawSpriteCore(byte *dest, SpriteFilter &reader, const SpriteDrawItem &sprite) {

	int16 destInc;

	if (sprite.flags & 4) {
		destInc = -1;
		dest += sprite.width;
	} else {
		destInc = 1;
	}

	SpriteReaderStatus status;
	PixelPacket packet;

	byte *destp = dest;
	int16 skipX = sprite.skipX;

	int16 w = sprite.width;
	int16 h = sprite.height;

	do {
		status = reader.readPacket(packet);

		if (skipX > 0) {
			while (skipX > 0) {
				skipX -= packet.count;
				if (skipX < 0) {
					packet.count = ABS(skipX);
					break;
				}
				status = reader.readPacket(packet);
			}
		}

		if (w - packet.count < 0)
			packet.count = w;

		w -= packet.count;

		if (((sprite.flags & 0x40) && (packet.pixel != 0)) ||
			((sprite.flags & 0x10) && (packet.pixel != 0xFF)) ||
			(!(sprite.flags & 0x10) && (packet.pixel != 0)))
		{
			if (sprite.flags & 0x40) {
				while (packet.count--) {
					*dest = _vm->_palette->getColorTransPixel(*dest);
					dest += destInc;
				}
			} else {
				if (sprite.flags & 0x10) {
					packet.pixel = ((packet.pixel << 4) & 0xF0) | ((packet.pixel >> 4) & 0x0F);
				} else {
					packet.pixel += sprite.baseColor - 1;
				}
				while (packet.count--) {
					*dest = packet.pixel;
					dest += destInc;
				}
			}
		} else {
			dest += packet.count * destInc;
		}

		if (status == kSrsEndOfLine || w <= 0) {
			if (w <= 0) {
				while (status == kSrsPixelsLeft) {
					status = reader.readPacket(packet);
				}
			}
			dest = destp + 640;
			destp = dest;
			skipX = sprite.skipX;
			w = sprite.width;
			h--;
		}

	} while (status != kSrsEndOfSprite && h > 0);

}

} // End of namespace Toltecs
