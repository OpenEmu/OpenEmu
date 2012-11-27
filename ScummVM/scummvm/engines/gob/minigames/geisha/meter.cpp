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

#include "gob/surface.h"

#include "gob/minigames/geisha/meter.h"

namespace Gob {

namespace Geisha {

Meter::Meter(int16 x, int16 y, int16 width, int16 height, uint8 frontColor,
             uint8 backColor, int32 maxValue, Direction direction) :
	_x(x), _y(y), _width(width), _height(height), _frontColor(frontColor),
	_backColor(backColor), _value(0), _maxValue(maxValue), _direction(direction),
	_needUpdate(true), _surface(0) {

}

Meter::~Meter() {
	delete _surface;
}

int32 Meter::getMaxValue() const {
	return _maxValue;
}

int32 Meter::getValue() const {
	return _value;
}

void Meter::setValue(int32 value) {
	value = CLIP<int32>(value, 0, _maxValue);
	if (_value == value)
		return;

	_value = value;
	_needUpdate = true;
}

void Meter::setMaxValue() {
	setValue(_maxValue);
}

int32 Meter::increase(int32 n) {
	if (n < 0)
		return decrease(-n);

	int32 overflow = MAX<int32>(0, (_value + n) - _maxValue);

	int32 value = CLIP<int32>(_value + n, 0, _maxValue);
	if (_value == value)
		return overflow;

	_value = value;
	_needUpdate = true;

	return overflow;
}

int32 Meter::decrease(int32 n) {
	if (n < 0)
		return increase(-n);

	int32 underflow = -MIN<int32>(0, _value - n);

	int32 value = CLIP<int32>(_value - n, 0, _maxValue);
	if (_value == value)
		return underflow;

	_value = value;
	_needUpdate = true;

	return underflow;
}

void Meter::draw(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom) {
	if (!_surface) {
		_surface = new Surface(_width, _height, dest.getBPP());
		_needUpdate = true;
	}

	update();

	left   = CLIP<int16>(_x              , 0, dest.getWidth () - 1);
	top    = CLIP<int16>(_y              , 0, dest.getHeight() - 1);
	right  = CLIP<int16>(_x + _width  - 1, 0, dest.getWidth () - 1);
	bottom = CLIP<int16>(_y + _height - 1, 0, dest.getHeight() - 1);

	dest.blit(*_surface, left - _x, top - _y, _width, _height, left, top);
}

void Meter::update() {
	if (!_needUpdate)
		return;

	_needUpdate = false;

	_surface->fill(_backColor);

	int32 n = (int32)floor((((float) _width) / _maxValue * _value) + 0.5);
	if (n <= 0)
		return;

	if (_direction == kFillToLeft)
		_surface->fillRect(_width - n, 0, _width - 1, _height - 1, _frontColor);
	else
		_surface->fillRect(0         , 0, n - 1, _height - 1, _frontColor);
}

} // End of namespace Geisha

} // End of namespace Gob
