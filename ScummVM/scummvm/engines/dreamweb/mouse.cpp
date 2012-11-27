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

#include "common/events.h"
#include "dreamweb/dreamweb.h"

namespace DreamWeb {

void DreamWebEngine::mouseCall(uint16 *x, uint16 *y, uint16 *state) {
	processEvents();
	Common::Point pos = _eventMan->getMousePos();
	*x = CLIP<int16>(pos.x, 15, 298);
	*y = CLIP<int16>(pos.y, 15, 184);

	unsigned newState = _eventMan->getButtonState();
	*state = (newState == _oldMouseState ? 0 : newState);
	_oldMouseState = newState;
}

void DreamWebEngine::readMouse() {
	_oldButton = _mouseButton;
	_mouseButton = readMouseState();
}

uint16 DreamWebEngine::readMouseState() {
	_oldX = _mouseX;
	_oldY = _mouseY;
	uint16 x, y, state;
	mouseCall(&x, &y, &state);
	_mouseX = x;
	_mouseY = y;
	return state;
}

void DreamWebEngine::dumpPointer() {
	dumpBlink();
	multiDump(_delHereX, _delHereY, _delXS, _delYS);
	if ((_oldPointerX != _delHereX) || (_oldPointerY != _delHereY))
		multiDump(_oldPointerX, _oldPointerY, _pointerXS, _pointerYS);
}

void DreamWebEngine::showPointer() {
	showBlink();
	uint16 x = _mouseX;
	_oldPointerX = _mouseX;
	uint16 y = _mouseY;
	_oldPointerY = _mouseY;
	if (_pickUp == 1) {
		const GraphicsFile *frames;
		if (_objectType != kExObjectType)
			frames = &_freeFrames;
		else
			frames = &_exFrames;
		const Frame *frame = &frames->_frames[(3 * _itemFrame + 1)];

		uint8 width = MAX<uint8>(frame->width, 12);
		uint8 height = MAX<uint8>(frame->height, 12);
		_pointerXS = width;
		_pointerYS = height;
		uint16 xMin = (x >= width / 2) ? x - width / 2 : 0;
		uint16 yMin = (y >= height / 2) ? y - height / 2 : 0;
		_oldPointerX = xMin;
		_oldPointerY = yMin;
		multiGet(_pointerBack, xMin, yMin, width, height);
		showFrame(*frames, x, y, 3 * _itemFrame + 1, 128);
		showFrame(_icons1, x, y, 3, 128);
	} else {
		const Frame *frame = &_icons1._frames[_pointerFrame + 20];
		uint8 width = MAX<uint8>(frame->width, 12);
		uint8 height = MAX<uint8>(frame->height, 12);
		_pointerXS = width;
		_pointerYS = height;
		multiGet(_pointerBack, x, y, width, height);
		showFrame(_icons1, x, y, _pointerFrame + 20, 0);
	}
}

void DreamWebEngine::delPointer() {
	if (_oldPointerX == 0xffff)
		return;
	_delHereX = _oldPointerX;
	_delHereY = _oldPointerY;
	_delXS = _pointerXS;
	_delYS = _pointerYS;
	multiPut(_pointerBack, _delHereX, _delHereY, _pointerXS, _pointerYS);
}

void DreamWebEngine::animPointer() {
	if (_pointerMode == 2) {
		_pointerFrame = 0;
		if ((_realLocation == 14) && (_commandType == 211))
			_pointerFrame = 5;
		return;
	} else if (_pointerMode == 3) {
		if (_pointerSpeed != 0) {
			--_pointerSpeed;
		} else {
			_pointerSpeed = 5;
			++_pointerCount;
			if (_pointerCount == 16)
				_pointerCount = 0;
		}
		_pointerFrame = (_pointerCount <= 8) ? 1 : 2;
		return;
	}
	if (_vars._watchingTime != 0) {
		_pointerFrame = 11;
		return;
	}
	_pointerFrame = 0;
	if (_inMapArea == 0)
		return;
	if (_pointerFirstPath == 0)
		return;
	uint8 flag, flagEx;
	getFlagUnderP(&flag, &flagEx);
	if (flag < 2)
		return;
	if (flag >= 128)
		return;
	if (flag & 4) {
		_pointerFrame = 3;
		return;
	}
	if (flag & 16) {
		_pointerFrame = 4;
		return;
	}
	if (flag & 2) {
		_pointerFrame = 5;
		return;
	}
	if (flag & 8) {
		_pointerFrame = 6;
		return;
	}
	_pointerFrame = 8;
}

void DreamWebEngine::checkCoords(const RectWithCallback *rectWithCallbacks) {
	if (_newLocation != 0xff)
		return;
	const RectWithCallback *r;
	for (r = rectWithCallbacks; r->_xMin != 0xffff; ++r) {
		if (r->contains(_mouseX, _mouseY)) {
			(this->*(r->_callback))();
			return;
		}
	}
}

} // End of namespace DreamWeb
