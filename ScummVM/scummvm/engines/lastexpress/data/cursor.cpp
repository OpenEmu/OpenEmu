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

#include "lastexpress/data/cursor.h"

#include "lastexpress/lastexpress.h"

#include "common/stream.h"
#include "common/system.h"
#include "graphics/cursorman.h"

namespace LastExpress {

uint16 brigthnessData[4] = { 0, 0x7BDE, 0x739C, 0x6318 };

Cursor::Cursor() : _current(kCursorMAX) {
	memset(&_cursors, 0, sizeof(_cursors));
}

bool Cursor::load(Common::SeekableReadStream *stream) {
	if (!stream)
		return false;

	// Load the whole file to memory
	Common::SeekableReadStream *data = stream->readStream((uint32) stream->size());
	delete stream;
	if (!data)
		return false;

	// Read the hotspot data
	for (int i = 0; i < kCursorMAX; i++) {
		_cursors[i].hotspotX = data->readUint16LE();
		_cursors[i].hotspotY = data->readUint16LE();
		debugC(15, kLastExpressDebugCursor | kLastExpressDebugAll,
			"Cursor %d hotspot x: %d, hotspot y: %d",
			i, _cursors[i].hotspotX, _cursors[i].hotspotY);
	}

	// Read the pixel data
	for (int i = 0; i < kCursorMAX; i++)
		for (int pix = 0; pix < 32 * 32; pix++)
			_cursors[i].image[pix] = data->readUint16LE();

	delete data;
	return true;
}

void Cursor::show(bool visible) const {
	CursorMan.showMouse(visible);
}

bool Cursor::checkStyle(CursorStyle style) const {
	if (style >= kCursorMAX) {
		debugC(2, kLastExpressDebugGraphics, "Trying to use an invalid cursor style: was %d, max %d", (int)style, kCursorMAX);
		return false;
	}

	return true;
}

void Cursor::setStyle(CursorStyle style) {
	if (!checkStyle(style))
		return;

	if (style == _current)
		return;

	debugC(10, kLastExpressDebugCursor | kLastExpressDebugAll, "Cursor: setting style: %d", style);

	// Save the new cursor
	_current = style;

	// Reuse the screen pixel format
	Graphics::PixelFormat pf = g_system->getScreenFormat();
	CursorMan.replaceCursor(getCursorImage(style),
	                        32, 32, _cursors[style].hotspotX, _cursors[style].hotspotY,
	                        0, false, &pf);
}

const uint16 *Cursor::getCursorImage(CursorStyle style) const {
	if (!checkStyle(style))
		return NULL;

	return _cursors[style].image;
}


Icon::Icon(CursorStyle style) : _style(style), _x(0), _y(0), _brightnessIndex(-1) {}

void Icon::setPosition(int16 x, int16 y) {
	_x = x;
	_y = y;
}

void Icon::setBrightness(int16 brightnessIndex) {
	assert(brightnessIndex < ARRAYSIZE(brigthnessData));

	_brightnessIndex = brightnessIndex;
}

Common::Rect Icon::draw(Graphics::Surface *surface) {
	const uint16 *image = ((LastExpressEngine *)g_engine)->getCursor()->getCursorImage((CursorStyle)_style);
	if (!image)
		return Common::Rect();

	// TODO adjust brightness. The original game seems to be using a table for that (at least in the highlighting case)
	for (int j = 0; j < 32; j++) {
		uint16 *s = (uint16 *)surface->getBasePtr(_x, _y + j);
		for (int i = 0; i < 32; i++) {

			// Adjust brightness
			if (_brightnessIndex == -1 || _brightnessIndex >= ARRAYSIZE(brigthnessData))
				*s = *image;
			else
				*s = (*image & brigthnessData[_brightnessIndex]) >> _brightnessIndex;

			// Update the image and surface pointers
			image++;
			s++;
		}
	}

	return Common::Rect(_x, _y, _x + 32, _y + 32);
}

} // End of namespace LastExpress
