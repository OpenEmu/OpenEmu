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

#ifndef DRACI_SPRITE_H
#define DRACI_SPRITE_H

#include "common/scummsys.h"
#include "common/rect.h"

namespace Draci {

enum DrawableType {
	kDrawableText,
	kDrawableSprite
};

struct Displacement {
	int relX, relY;
	double extraScaleX, extraScaleY;
};

extern const Displacement kNoDisplacement;

class Surface;
class Font;

class Drawable {
public:
	virtual void draw(Surface *surface, bool markDirty, int relX, int relY) const = 0;
	virtual void drawReScaled(Surface *surface, bool markDirty, const Displacement &displacement) const = 0;

	virtual ~Drawable() {}

	uint getWidth() const { return _width; }
	uint getHeight() const { return _height; }

	uint getScaledWidth() const { return _scaledWidth; }
	uint getScaledHeight() const { return _scaledHeight; }

	void setScaled(uint width, uint height) {
		_scaledWidth = width;
		_scaledHeight = height;
	}

	int getX() const { return _x; }
	int getY() const { return _y; }

	void setX(int x) { _x = x; }
	void setY(int y) { _y = y; }

	void setDelay(int delay) { _delay = delay; }
	int getDelay() const { return _delay; }

	virtual Common::Rect getRect(const Displacement &displacement) const = 0;

	virtual DrawableType getType() const = 0;

protected:
	uint _width;        ///< Width of the sprite
	uint _height;       ///< Height of the sprite
	uint _scaledWidth;  ///< Scaled width of the sprite
	uint _scaledHeight; ///< Scaled height of the sprite
	int _x, _y;         ///< Sprite coordinates

	/** The time a drawable should stay on the screen
	 *  before being replaced by another or deleted
	 */
	int _delay;
};

/**
 *  Represents a Draci Historie sprite. Supplies two constructors; one for
 *  loading a sprite from a raw data buffer and one for loading a sprite in
 *  the Draci sprite format. Supports loading the sprite from a column-wise
 *  format (transforming them to row-wise) since that is the way the sprites
 *  are stored in the original game files.
 *
 *  Sprite format:
 *  [uint16LE] sprite width
 *  [uint16LE] sprite height
 *  [height * width bytes] image pixels stored column-wise, one byte per pixel
 */

class Sprite : public Drawable {
public:
	// Takes ownership of raw_data.
	Sprite(uint16 width, uint16 height, byte *raw_data, int x, int y, bool columnwise);

	// It doesn't take ownership of sprite_data.  If columnwise is false,
	// it internally points to the original buffer (which has lifetime at
	// least as long as this sprite, assumming that it comes from a cached
	// BArchive file), otherwise it allocates its own buffer with the
	// transposed image.
	Sprite(const byte *sprite_data, uint16 length, int x, int y, bool columnwise);

	~Sprite();

	void draw(Surface *surface, bool markDirty, int relX, int relY) const;
	void drawReScaled(Surface *surface, bool markDirty, const Displacement &displacement) const;

	void setMirrorOn() { _mirror = true; }
	void setMirrorOff() { _mirror = false; }

	Common::Rect getRect(const Displacement &displacement) const;

	const byte *getBuffer() const { return _data; }
	int getPixel(int x, int y, const Displacement &displacement) const;

	DrawableType getType() const { return kDrawableSprite; }

private:
	bool _ownsData;
	const byte *_data;  ///< Pointer to a buffer containing raw sprite data (row-wise)
	bool _mirror;
};

class Text : public Drawable {

public:
	Text(const Common::String &str, const Font *font, byte fontColor,
	    int x, int y, uint spacing);
	~Text() {}

	void setText(const Common::String &str);
	void setColor(byte fontColor) { _color = fontColor; }
	void setSpacing(uint spacing) { _spacing = spacing; }
	void setFont(const Font *font);

	void repeatedlySplitLongLines(uint maxWidth);

	uint getLength() const { return _length; }

	void draw(Surface *surface, bool markDirty, int relX, int relY) const;

	// drawReScaled just calls draw so that Text can be accessed through a Drawable pointer.
	// Text scaling does not need to be handled.
	void drawReScaled(Surface *surface, bool markDirty, const Displacement &displacement) const { draw(surface, markDirty, displacement.relX, displacement.relY); }
	Common::Rect getRect(const Displacement &displacement) const;

	DrawableType getType() const { return kDrawableText; }
private:
	void splitLinesLongerThan(uint maxWidth);

	Common::String _text;
	uint _length;
	uint8 _color;
	uint _spacing;
	const Font *_font;
};

} // End of namespace Draci

#endif // DRACI_SPRITE_H
