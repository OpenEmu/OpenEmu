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

#include "lastexpress/game/object.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Common::String Objects::Object::toString() {
	return Common::String::format("{ %s - %d - %d - %d - %d }", ENTITY_NAME(entity), location, cursor, cursor2, location2);
}

Objects::Objects(LastExpressEngine *engine) : _engine(engine) {}

const Objects::Object Objects::get(ObjectIndex index) const {
	if (index >= kObjectMax)
		error("[Objects::get] Invalid object index (%d)", index);

	return _objects[index];
}

void Objects::update(ObjectIndex index, EntityIndex entity, ObjectLocation location, CursorStyle cursor, CursorStyle cursor2) {
	if (index >= kObjectMax)
		return;

	Object *object = &_objects[index];

	// Store original location
	ObjectLocation original_location = object->location;

	// Update entity
	object->entity = entity;
	object->location = location;

	if (cursor != kCursorKeepValue || cursor2 != kCursorKeepValue) {
		if (cursor != kCursorKeepValue)
			object->cursor = cursor;
		if (cursor2 != kCursorKeepValue)
			object->cursor2 = cursor2;

		getLogic()->updateCursor();
	}

	getFlags()->flag_3 = true;

	// Compartments
	if (original_location != location && (original_location == kObjectLocation2 || location == kObjectLocation2))
		if ((index >= kObjectCompartment1 && index <= kObjectCompartment8)
	     || (index >= kObjectCompartmentA && index <= kObjectCompartmentF)) {
		 	getScenes()->updateDoorsAndClock();
		}
}

void Objects::updateLocation2(ObjectIndex index, ObjectLocation location2) {
	if (index >= kObjectMax)
		return;

	_objects[index].location2 = location2;
}

//////////////////////////////////////////////////////////////////////////
// Serializable
//////////////////////////////////////////////////////////////////////////
void Objects::saveLoadWithSerializer(Common::Serializer &s) {
	for (int i = 0; i < ARRAYSIZE(_objects); i++)
		_objects[i].saveLoadWithSerializer(s);
}

//////////////////////////////////////////////////////////////////////////
// toString
//////////////////////////////////////////////////////////////////////////
Common::String Objects::toString() {
	Common::String ret = "";

	for (int i = 0; i < ARRAYSIZE(_objects); i++)
		ret += Common::String::format("%d : %s\n", i, _objects[i].toString().c_str());

	return ret;
}

} // End of namespace LastExpress
