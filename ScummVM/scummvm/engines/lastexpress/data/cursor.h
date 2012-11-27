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

#ifndef LASTEXPRESS_CURSOR_H
#define LASTEXPRESS_CURSOR_H

/*
	Cursor format (CURSORS.TBM)

	style table:
	    (for each cursor)
	    uint16 {2}       - hotspot X
	    uint16 {2}       - hotspot Y

	data:
	    (for each cursor)
	    uint16 {32*32}   - cursor data
*/

#include "lastexpress/drawable.h"

#include "lastexpress/shared.h"

namespace Common {
class SeekableReadStream;
}

namespace LastExpress {

class Icon : public Drawable {
public:
	Icon(CursorStyle style);

	void setPosition(int16 x, int16 y);
	void setBrightness(int16 brightnessIndex);
	Common::Rect draw(Graphics::Surface *surface);

private:
	CursorStyle _style;
	int16 _x, _y;
	int16 _brightnessIndex;
};

class Cursor {
public:
	Cursor();

	bool load(Common::SeekableReadStream *stream);
	void show(bool visible) const;

	void setStyle(CursorStyle style);
	CursorStyle getStyle() const { return _current; }

private:
	// Style
	CursorStyle _current;

	// Cursors data
	struct {
		uint16 image[32 * 32];
		uint16 hotspotX, hotspotY;
	} _cursors[kCursorMAX];

	bool checkStyle(CursorStyle style) const;
	const uint16 *getCursorImage(CursorStyle style) const;

	// Only allow full access for drawing (needed for getCursorImage)
	friend Common::Rect Icon::draw(Graphics::Surface *surface);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_CURSOR_H
