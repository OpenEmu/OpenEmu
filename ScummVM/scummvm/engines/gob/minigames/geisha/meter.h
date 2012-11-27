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

#ifndef GOB_MINIGAMES_GEISHA_METER_H
#define GOB_MINIGAMES_GEISHA_METER_H

#include "gob/aniobject.h"

namespace Gob {

class Surface;

namespace Geisha {

/** A meter measuring a value. */
class Meter {
public:
	enum Direction {
		kFillToLeft,
		kFillToRight
	};

	Meter(int16 x, int16 y, int16 width, int16 height,
	      uint8 frontColor, uint8 backColor, int32 maxValue,
	      Direction direction);
	~Meter();

	/** Return the max value the meter is measuring. */
	int32 getMaxValue() const;
	/** Return the current value the meter is measuring. */
	int32 getValue() const;

	/** Set the current value the meter is measuring. */
	void setValue(int32 value);

	/** Set the current value the meter is measuring to the max value. */
	void setMaxValue();

	/** Increase the current value the meter is measuring, returning the overflow. */
	int32 increase(int32 n = 1);
	/** Decrease the current value the meter is measuring, returning the underflow. */
	int32 decrease(int32 n = 1);

	/** Draw the meter onto the surface and return the affected rectangle. */
	void draw(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom);

private:
	int16 _x;
	int16 _y;
	int16 _width;
	int16 _height;

	uint8 _frontColor;
	uint8 _backColor;

	int32 _value;
	int32 _maxValue;

	Direction _direction;

	bool _needUpdate;

	Surface *_surface;

	void update();
};

} // End of namespace Geisha

} // End of namespace Gob

#endif // GOB_MINIGAMES_GEISHA_METER_H
