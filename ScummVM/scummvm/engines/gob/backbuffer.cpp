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

#include "common/util.h"

#include "gob/backbuffer.h"
#include "gob/surface.h"

namespace Gob {

BackBuffer::BackBuffer() : _background(0), _saved(false) {
}

BackBuffer::~BackBuffer() {
	delete _background;
}

bool BackBuffer::hasBuffer() const {
	return _background != 0;
}

bool BackBuffer::hasSavedBackground() const {
	return _saved;
}

void BackBuffer::trashBuffer() {
	_saved = false;
}

void BackBuffer::resizeBuffer(uint16 width, uint16 height) {
	trashBuffer();

	if (_background && (_background->getWidth() == width) && (_background->getHeight() == height))
		return;

	delete _background;

	_background = new Surface(width, height, 1);
}

bool BackBuffer::saveScreen(const Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom) {
	if (!_background)
		return false;

	const int16 width  = MIN<int16>(right  - left + 1, _background->getWidth ());
	const int16 height = MIN<int16>(bottom - top  + 1, _background->getHeight());
	if ((width <= 0) || (height <= 0))
		return false;

	right  = left + width  - 1;
	bottom = top  + height - 1;

	_saveLeft   = left;
	_saveTop    = top;
	_saveRight  = right;
	_saveBottom = bottom;

	_background->blit(dest, _saveLeft, _saveTop, _saveRight, _saveBottom, 0, 0);

	_saved = true;

	return true;
}

bool BackBuffer::restoreScreen(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom) {
	if (!_saved)
		return false;

	left   = _saveLeft;
	top    = _saveTop;
	right  = _saveRight;
	bottom = _saveBottom;

	dest.blit(*_background, 0, 0, right - left, bottom - top, left, top);

	_saved = false;

	return true;
}

} // End of namespace Gob
