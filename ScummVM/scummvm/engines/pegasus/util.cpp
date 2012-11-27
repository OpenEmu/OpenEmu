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

#include "common/random.h"
#include "common/system.h"
#include "common/util.h"

#include "pegasus/util.h"

namespace Pegasus {

IDObject::IDObject(const int32 id) {
	_objectID = id;
}

IDObject::~IDObject() {
}

int32 IDObject::getObjectID() const {
	return _objectID;
}

int operator==(const IDObject &arg1, const IDObject &arg2) {
	return arg1.getObjectID() == arg2.getObjectID();
}

int operator!=(const IDObject &arg1, const IDObject &arg2) {
	return arg1.getObjectID() != arg2.getObjectID();
}

int32 pegasusRound(const int32 a, const int32 b) {
	if (b < 0)
		if (a < 0)
			return -((a - (-b >> 1)) / -b);
		else
			return -((a + (-b >> 1)) / -b);
	else
		if (a < 0)
			return (a - (b >> 1)) / b;
		else
			return (a + (b >> 1)) / b;
}

int32 linearInterp(const int32 start1, const int32 stop1, const int32 current1, const int32 start2, const int32 stop2) {
	if (start2 == stop2)
		return start2;
	else
		return start2 + pegasusRound((current1 - start1) * (stop2 - start2), (stop1 - start1));
}

uint32 tickCount() {
	return g_system->getMillis() * 60 / 1000;
}

} // End of namespace Pegasus
