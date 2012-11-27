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

#include "common/memstream.h"

#include "draci/draci.h"
#include "draci/font.h"
#include "draci/sprite.h"
#include "draci/surface.h"

namespace Draci {

const Displacement kNoDisplacement = { 0, 0, 1.0, 1.0 };

/**
 *  @brief Transforms an image from column-wise to row-wise
 *  @param img pointer to the buffer containing the image data
 *         width width of the image in the buffer
 *         height height of the image in the buffer
 */
static void transformToRows(byte *img, uint16 width, uint16 height) {
	byte *buf = new byte[width * height];
	byte *tmp = buf;
	memcpy(buf, img, width * height);

	for (uint16 i = 0; i < width; ++i) {
		for (uint16 j = 0; j < height; ++j) {
			img[j * width + i] = *tmp++;
		}
	}

	delete[] buf;
}

/**
 *  Constructor for loading sprites from a raw data buffer, one byte per pixel.
 */
Sprite::Sprite(uint16 width, uint16 height, byte *raw_data, int x, int y, bool columnwise)
    : _ownsData(true), _data(raw_data), _mirror(false) {

	 _width = width;
	 _height = height;

	_scaledWidth = _width;
	_scaledHeight = _height;

	 _x = x;
	 _y = y;

	_delay = 0;

	if (columnwise) {
		transformToRows(raw_data, _width, _height);
	}
}

/**
 *  Constructor for loading sprites from a sprite-formatted buffer, one byte per
 *  pixel.
 */
Sprite::Sprite(const byte *sprite_data, uint16 length, int x, int y, bool columnwise)
    : _ownsData(false), _data(NULL), _mirror(false) {

	Common::MemoryReadStream reader(sprite_data, length);
	_width = reader.readSint16LE();
	_height = reader.readSint16LE();

	_scaledWidth = _width;
	_scaledHeight = _height;

	 _x = x;
	 _y = y;

	_delay = 0;

	if (!columnwise) {
		_ownsData = false;
		_data = sprite_data + 4;
	} else {
		_ownsData = true;
		byte *data = new byte[_width * _height];
		memcpy(data, sprite_data + 4, _width * _height);
		transformToRows(data, _width, _height);
		_data = data;
	}
}

Sprite::~Sprite() {
	if (_ownsData) {
		delete[] _data;
	}
}

int Sprite::getPixel(int x, int y, const Displacement &displacement) const {
	Common::Rect rect = getRect(displacement);

	int dy = y - rect.top;
	int dx = x - rect.left;

	// Calculate scaling factors
	double scaleX = double(rect.width()) / _width;
	double scaleY = double(rect.height()) / _height;

	int sy = scummvm_lround(dy / scaleY);
	int sx = scummvm_lround(dx / scaleX);

	if (_mirror)
		return _data[sy * _width + (_width - sx)];
	else
		return _data[sy * _width + sx];
}


void Sprite::drawReScaled(Surface *surface, bool markDirty, const Displacement &displacement) const {
	const Common::Rect destRect(getRect(displacement));
	Common::Rect clippedDestRect(0, 0, surface->w, surface->h);
	clippedDestRect.clip(destRect);

	// Calculate by how much we need to adjust the source rectangle to account for cropping
	const Common::Point croppedBy(clippedDestRect.left - destRect.left, clippedDestRect.top - destRect.top);

	// Get pointers to source and destination buffers
	byte *dst = (byte *)surface->getBasePtr(clippedDestRect.left, clippedDestRect.top);

	const int transparent = surface->getTransparentColor();

	// Calculate how many rows and columns we need to draw
	const int rows = clippedDestRect.height();
	const int columns = clippedDestRect.width();

	// Precalculate column indexes
	int *columnIndices = new int[columns];
	if (!_mirror) {
		for (int j = 0; j < columns; ++j) {
		        columnIndices[j] = (j + croppedBy.x) * _width / destRect.width();
		}
	} else {
		// Draw the sprite mirrored if the _mirror flag is set
		for (int j = 0; j < columns; ++j) {
		        columnIndices[j] = _width - 1 - (j + croppedBy.x) * _width / destRect.width();
		}
	}

	// Blit the sprite to the surface
	for (int i = 0; i < rows; ++i) {
		// Compute the index of current row to be drawn
		const int row = (i + croppedBy.y) * _height / destRect.height();
		const byte *row_data = _data + row * _width;

		for (int j = 0; j < columns; ++j) {

			// Fetch index of current column to be drawn
			const byte src = row_data[columnIndices[j]];

			// Don't blit if the pixel is transparent on the target surface
			if (src != transparent) {
				dst[j] = src;
			}
		}

		// Advance to next row
		dst += surface->pitch;
	}

	// Mark the sprite's rectangle dirty
	if (markDirty) {
		surface->markDirtyRect(clippedDestRect);
	}

	delete[] columnIndices;
}

/**
 *  @brief Draws the sprite to a Draci::Surface
 *  @param surface Pointer to a Draci::Surface
 *
 *  Draws the sprite to a Draci::Surface and marks its rectangle on the surface as dirty.
 *  It is safe to call it for sprites that would overflow the surface.
 */
void Sprite::draw(Surface *surface, bool markDirty, int relX, int relY) const {
	const Common::Rect destRect(_x + relX, _y + relY, _x + relX + _width, _y + relY + _height);
	Common::Rect clippedDestRect(0, 0, surface->w, surface->h);
	clippedDestRect.clip(destRect);

	// Calculate by how much we need to adjust the source rectangle to account for cropping
	const Common::Point croppedBy(clippedDestRect.left - destRect.left, clippedDestRect.top - destRect.top);

	// Get pointers to source and destination buffers
	byte *dst = (byte *)surface->getBasePtr(clippedDestRect.left, clippedDestRect.top);
	const byte *src = _data + croppedBy.y * _width +
		(!_mirror ? croppedBy.x : _width - 1 - croppedBy.x);

	const int transparent = surface->getTransparentColor();

	// Calculate how many rows and columns we need to draw
	const int rows = clippedDestRect.height();
	const int columns = clippedDestRect.width();

	// Blit the sprite to the surface
	for (int i = 0; i < rows; ++i) {
		if (!_mirror) {
			for (int j = 0; j < columns; ++j) {
				if (src[j] != transparent) {
					dst[j] = src[j];
				}
			}
		} else {
			for (int j = 0; j < columns; ++j) {
				if (src[-j] != transparent) {
					dst[j] = src[-j];
				}
			}
		}

		// Advance to next row
		dst += surface->pitch;
		src += _width;
	}

	// Mark the sprite's rectangle dirty
	if (markDirty) {
		surface->markDirtyRect(clippedDestRect);
	}
}


Common::Rect Sprite::getRect(const Displacement &displacement) const {
	return Common::Rect(_x + displacement.relX, _y + displacement.relY,
	    _x + displacement.relX + scummvm_lround(_scaledWidth * displacement.extraScaleX),
	    _y + displacement.relY + scummvm_lround(_scaledHeight * displacement.extraScaleY));
}

Text::Text(const Common::String &str, const Font *font, byte fontColor,
                int x, int y, uint spacing) {
	_x = x;
	_y = y;
	_delay = 0;

	_text = str;

	_length = 0;
	for (uint i = 0; i < _text.size(); ++i) {
		if (_text[i] != '|') {
			++_length;
		}
	}

	_spacing = spacing;
	_color = fontColor;

	_font = font;

	_width = _font->getStringWidth(str, _spacing);
	_height = _font->getStringHeight(str);

	_scaledWidth = _width;
	_scaledHeight = _height;
}

void Text::setText(const Common::String &str) {
	_width = _font->getStringWidth(str, _spacing);
	_height = _font->getStringHeight(str);

	 _text = str;

	_length = 0;
	for (uint i = 0; i < _text.size(); ++i) {
		if (_text[i] != '|') {
			++_length;
		}
	}
}

void Text::draw(Surface *surface, bool markDirty, int relX, int relY) const {
	_font->drawString(surface, _text, _x + relX, _y + relY, _color, _spacing, true);
}

Common::Rect Text::getRect(const Displacement &displacement) const {
	// Texts are never scaled
	return Common::Rect(_x + displacement.relX, _y + displacement.relY, _x + displacement.relX + _width, _y + displacement.relY + _height);
}

void Text::setFont(const Font *font) {
	if (font == _font) {
		return;
	}
	_font = font;

	_width = _font->getStringWidth(_text, _spacing);
	_height = _font->getStringHeight(_text);
}

void Text::repeatedlySplitLongLines(uint maxWidth) {
	while (_width > maxWidth) {
		splitLinesLongerThan(maxWidth);
		_width = _font->getStringWidth(_text, _spacing);
		_height = _font->getStringHeight(_text);
	}
}

void Text::splitLinesLongerThan(uint maxWidth) {
	char *start = const_cast<char *> (_text.c_str());	// hacky
	while (1) {
		char *end = strchr(start, '|');
		if (end) {
			*end = 0;
		}
		uint lineWidth = _font->getStringWidth(start, _spacing);
		if (lineWidth > maxWidth) {
			int middle = end ? (end - start) / 2 : strlen(start) / 2;
			for (int i = 0; ; ++i) {
				if (start[middle + i] == ' ') {
					start[middle + i] = '|';
					break;
				}
				if (start[middle - i] == ' ') {
					start[middle - i] = '|';
					break;
				}
			}
			debugC(2, kDraciGeneralDebugLevel, "Long line of width %d split into %s", lineWidth, start);
		}
		if (end) {
			*end = '|';
			start = end + 1;
		} else {
			break;
		}
	}
}

} // End of namespace Draci
