/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "pegasus/elements.h"
#include "pegasus/items/autodragger.h"

namespace Pegasus {

AutoDragger::AutoDragger() {
	_draggingElement = NULL;
	_lastTime = 0;
	initCallBack(this, kCallBackAtExtremes);
}

void AutoDragger::autoDrag(DisplayElement *dragElement, const Common::Point &startPoint, const Common::Point &stopPoint,
		TimeValue dragTime, TimeScale dragScale) {
	_draggingElement = dragElement;

	if (_draggingElement) {
		_startLocation = startPoint;
		_stopLocation = stopPoint;
		_lastTime = 0;
		_done = false;
		_draggingElement->moveElementTo(_startLocation.x, _startLocation.y);
		setScale(dragScale);
		setSegment(0, dragTime);
		setTime(0);
		scheduleCallBack(kTriggerAtStop, 0, 0);
		startIdling();
		start();
	} else {
		stopDragging();
	}
}

void AutoDragger::stopDragging() {
	cancelCallBack();
	stopIdling();
	_draggingElement = 0;
	_startLocation = Common::Point();
	_stopLocation = Common::Point();
	_lastTime = 0;
	_done = true;
}

bool AutoDragger::isDragging() {
	return isIdling();
}

void AutoDragger::useIdleTime() {
	TimeValue thisTime = getTime();

	if (thisTime != _lastTime) {
		int32 offsetX = (_stopLocation.x - _startLocation.x) * (int32)thisTime / (int32)getDuration();
		int32 offsetY = (_stopLocation.y - _startLocation.y) * (int32)thisTime / (int32)getDuration();
		_draggingElement->moveElementTo(_startLocation.x + offsetX, _startLocation.y + offsetY);
		_lastTime = thisTime;
	}

	if (_done)
		stopDragging();
}

void AutoDragger::callBack() {
	if (isIdling())
		_done = true;
}

} // End of namespace Pegasus
