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

#ifndef LASTEXPRESS_OBJECT_H
#define LASTEXPRESS_OBJECT_H

#include "lastexpress/shared.h"

#include "common/serializer.h"
#include "common/system.h"

namespace LastExpress {

class LastExpressEngine;

class Objects : Common::Serializable {
public:

	struct Object : Common::Serializable {                         // All fields should be saved as bytes
		EntityIndex entity;
		ObjectLocation location;
		CursorStyle cursor;
		CursorStyle cursor2;
		ObjectLocation location2;

		Object() {
			entity = kEntityPlayer;
			location = kObjectLocationNone;
			cursor = kCursorHandKnock;
			cursor2 = kCursorHandKnock;
			location2 = kObjectLocationNone;
		}

		Common::String toString();

		// Serializable
		void saveLoadWithSerializer(Common::Serializer &s) {
			s.syncAsByte(entity);
			s.syncAsByte(location);
			s.syncAsByte(cursor);
			s.syncAsByte(cursor2);
			s.syncAsByte(location2);
		}
	};

	Objects(LastExpressEngine *engine);

	const Object get(ObjectIndex index) const;
	void update(ObjectIndex index, EntityIndex entity, ObjectLocation location, CursorStyle cursor, CursorStyle cursor2);
	void updateLocation2(ObjectIndex index, ObjectLocation location2);

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s);

	/**
	 * Convert this object into a string representation.
	 *
	 * @return A string representation of this object.
	 */
	Common::String toString();

private:
	LastExpressEngine *_engine;

	Object _objects[kObjectMax];
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_OBJECT_H
