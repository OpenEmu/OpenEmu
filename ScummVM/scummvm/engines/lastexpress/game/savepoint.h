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

#ifndef LASTEXPRESS_SAVEPOINT_H
#define LASTEXPRESS_SAVEPOINT_H

#include "lastexpress/helpers.h"
#include "lastexpress/shared.h"

#include "common/array.h"
#include "common/list.h"
#include "common/serializer.h"

/*
	Savepoint format
	----------------

	Save point: max: 127 - FIFO list (ie. goes back and overwrites first save point when full)
	    uint32 {4}      - Entity 1
	    uint32 {4}      - Action
	    uint32 {4}      - Entity 2
	    uint32 {4}      - Parameter

	Save point Data
	    uint32 {4}      - Entity 1
	    uint32 {4}      - Action
	    uint32 {4}      - Entity 2
	    uint32 {4}      - function pointer to ??

*/

namespace LastExpress {

class LastExpressEngine;

struct SavePoint {
	EntityIndex entity1;
	ActionIndex action;
	EntityIndex entity2;
	union {
		uint32 intValue;
		char charValue[5];
	} param;

	SavePoint() {
		entity1 = kEntityPlayer;
		action = kActionNone;
		entity2 = kEntityPlayer;
		param.intValue = 0;
	}

	Common::String toString() {
		return Common::String::format("{ %s - %d - %s - %s }", ENTITY_NAME(entity1), action, ENTITY_NAME(entity2), param.charValue);
	}
};

typedef Common::Functor1<const SavePoint&, void> Callback;

class SavePoints : Common::Serializable {
public:

	struct SavePointData {
		EntityIndex entity1;
		ActionIndex action;
		EntityIndex entity2;
		uint32 param;

		SavePointData() {
			entity1 = kEntityPlayer;
			action = kActionNone;
			entity2 = kEntityPlayer;
			param = 0;
		}

		Common::String toString() {
			return Common::String::format(" { %s - %d - %s - %d }", ENTITY_NAME(entity1), action, ENTITY_NAME(entity2), param);
		}
	};

	SavePoints(LastExpressEngine *engine);
	~SavePoints();

	// Savepoints
	void push(EntityIndex entity2, EntityIndex entity1, ActionIndex action, uint32 param = 0);
	void push(EntityIndex entity2, EntityIndex entity1, ActionIndex action, const char *param);
	void pushAll(EntityIndex entity, ActionIndex action, uint32 param = 0);
	void process();
	void reset();

	// Data
	void addData(EntityIndex entity, ActionIndex action, uint32 param);

	// Callbacks
	void setCallback(EntityIndex index, Callback *callback);
	Callback *getCallback(EntityIndex entity) const;
	void call(EntityIndex entity2, EntityIndex entity1, ActionIndex action, uint32 param = 0) const;
	void call(EntityIndex entity2, EntityIndex entity1, ActionIndex action, const char *param) const;
	void callAndProcess();

	// Serializable
	void saveLoadWithSerializer(Common::Serializer &s);

	/**
	 * Convert this object into a string representation.
	 *
	 * @return A string representation of this object.
	 */
	Common::String toString();

	uint32 count() { return _savepoints.size(); }

private:
	static const uint32 _savePointsMaxSize = 128;

	LastExpressEngine *_engine;

	Common::List<SavePoint> _savepoints;    ///< could be a queue, but we need to be able to iterate on the items
	Common::Array<SavePointData> _data;
	Callback *_callbacks[40];

	SavePoint pop();
	bool updateEntityFromData(const SavePoint &point);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SAVEPOINT_H
