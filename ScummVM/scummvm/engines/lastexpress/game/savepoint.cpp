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

#include "lastexpress/game/savepoint.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"


namespace LastExpress {

SavePoints::SavePoints(LastExpressEngine *engine) : _engine(engine) {
	for (int i = 0; i < 40; i++)
		_callbacks[i] = NULL;
}

SavePoints::~SavePoints() {
	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Savepoints
//////////////////////////////////////////////////////////////////////////
void SavePoints::push(EntityIndex entity2, EntityIndex entity1, ActionIndex action, uint32 param) {
	if (_savepoints.size() >= _savePointsMaxSize)
		return;

	SavePoint point;
	point.entity1 = entity1;
	point.action = action;
	point.entity2 = entity2;
	point.param.intValue = param;

	_savepoints.push_back(point);
}

void SavePoints::push(EntityIndex entity2, EntityIndex entity1, ActionIndex action, const char *param) {
	if (_savepoints.size() >= _savePointsMaxSize)
		return;

	SavePoint point;
	point.entity1 = entity1;
	point.action = action;
	point.entity2 = entity2;
	strcpy((char *)&point.param.charValue, param);

	_savepoints.push_back(point);
}

SavePoint SavePoints::pop() {
	SavePoint point = _savepoints.front();
	_savepoints.pop_front();
	return point;
}


void SavePoints::pushAll(EntityIndex entity, ActionIndex action, uint32 param) {
	for (uint32 index = 1; index < 40; index++) {
		if ((EntityIndex)index != entity)
			push(entity, (EntityIndex)index, action, param);
	}
}

// Process all savepoints
void SavePoints::process() {
	while (_savepoints.size() > 0 && getFlags()->isGameRunning) {
		SavePoint savepoint = pop();

		// If this is a data savepoint, update the entity
		// otherwise, execute the callback
		if (!updateEntityFromData(savepoint)) {

			// Call requested callback
			Callback *callback = getCallback(savepoint.entity1);
			if (callback && callback->isValid()) {
				debugC(8, kLastExpressDebugLogic, "Savepoint: entity1=%s, action=%s, entity2=%s", ENTITY_NAME(savepoint.entity1), ACTION_NAME(savepoint.action), ENTITY_NAME(savepoint.entity2));
				(*callback)(savepoint);
			}
		}
	}
}

void SavePoints::reset() {
	_savepoints.clear();
}

//////////////////////////////////////////////////////////////////////////
// Data
//////////////////////////////////////////////////////////////////////////
void SavePoints::addData(EntityIndex entity, ActionIndex action, uint32 param) {
	if (_data.size() >= _savePointsMaxSize)
		return;

	SavePointData data;
	data.entity1 = entity;
	data.action = action;
	data.param = param;

	_data.push_back(data);
}

//////////////////////////////////////////////////////////////////////////
// Callbacks
//////////////////////////////////////////////////////////////////////////
void SavePoints::setCallback(EntityIndex index, Callback *callback) {
	if (index >= 40)
		error("[SavePoints::setCallback] Attempting to use an invalid entity index. Valid values 0-39, was %d", index);

	if (!callback || !callback->isValid())
		error("[SavePoints::setCallback] Attempting to set an invalid callback for entity %s", ENTITY_NAME(index));

	_callbacks[index] = callback;
}

Callback *SavePoints::getCallback(EntityIndex index) const {
	if (index >= 40)
		error("[SavePoints::getCallback] Attempting to use an invalid entity index. Valid values 0-39, was %d", index);

	return _callbacks[index];
}

void SavePoints::call(EntityIndex entity2, EntityIndex entity1, ActionIndex action, uint32 param) const {
	SavePoint point;
	point.entity1 = entity1;
	point.action = action;
	point.entity2 = entity2;
	point.param.intValue = param;

	Callback *callback = getCallback(entity1);
	if (callback != NULL && callback->isValid()) {
		debugC(8, kLastExpressDebugLogic, "Savepoint: entity1=%s, action=%s, entity2=%s, param=%d", ENTITY_NAME(entity1), ACTION_NAME(action), ENTITY_NAME(entity2), param);
		(*callback)(point);
	}
}

void SavePoints::call(EntityIndex entity2, EntityIndex entity1, ActionIndex action, const char *param) const {
	SavePoint point;
	point.entity1 = entity1;
	point.action = action;
	point.entity2 = entity2;
	strcpy((char *)&point.param.charValue, param);

	Callback *callback = getCallback(entity1);
	if (callback != NULL && callback->isValid()) {
		debugC(8, kLastExpressDebugLogic, "Savepoint: entity1=%s, action=%s, entity2=%s, param=%s", ENTITY_NAME(entity1), ACTION_NAME(action), ENTITY_NAME(entity2), param);
		(*callback)(point);
	}
}

void SavePoints::callAndProcess() {
	SavePoint savepoint; // empty parameters

	// We ignore the kEntityPlayer callback in the list
	EntityIndex index = kEntityAnna;

	// Call all callbacks with empty parameters
	bool isRunning = getFlags()->isGameRunning;
	while (isRunning) {

		Callback *callback = getCallback(index);
		if (callback != NULL && callback->isValid()) {
			(*callback)(savepoint);
			isRunning = getFlags()->isGameRunning;
		}

		index = (EntityIndex)(index + 1);

		// Process all savepoints when done
		if (index >= 40) {
			if (isRunning)
				process();

			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
bool SavePoints::updateEntityFromData(const SavePoint &savepoint) {
	for (uint i = 0; i < _data.size(); i++) {

		// Not a data savepoint!
		if (!_data[i].entity1)
			return false;

		// Found our data!
		if (_data[i].entity1 == savepoint.entity1 && _data[i].action == savepoint.action) {
			debugC(8, kLastExpressDebugLogic, "Update entity from data: entity1=%s, action=%s, param=%u", ENTITY_NAME(_data[i].entity1), ACTION_NAME(_data[i].action), _data[i].param);

			// the SavePoint param value is the index of the entity call parameter to update
			getEntities()->get(_data[i].entity1)->getParamData()->updateParameters(_data[i].param);

			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Serializable
//////////////////////////////////////////////////////////////////////////
void SavePoints::saveLoadWithSerializer(Common::Serializer &s) {

	// Serialize savepoint data
	uint32 dataSize = (s.isLoading() ? _savePointsMaxSize : _data.size());
	for (uint i = 0; i < dataSize; i++) {
		if (s.isLoading()) {
			SavePointData data;
			_data.push_back(data);
		}

		s.syncAsUint32LE(_data[i].entity1);
		s.syncAsUint32LE(_data[i].action);
		s.syncAsUint32LE(_data[i].entity2);
		s.syncAsUint32LE(_data[i].param);
	}

	// Skip uninitialized data if any
	// (we are using a compressed stream, so we cannot seek on load)
	uint32 unusedDataSize = (_savePointsMaxSize - dataSize) * 16;
	if (s.isLoading()) {
		byte *empty = (byte *)malloc(unusedDataSize);
		s.syncBytes(empty, unusedDataSize);
		free(empty);
	} else {
		s.skip(unusedDataSize);
	}

	// Number of savepoints
	uint32 numSavepoints = _savepoints.size();
	s.syncAsUint32LE(numSavepoints);

	// Savepoints
	if (s.isLoading()) {
		for (uint i = 0; i < numSavepoints; i++) {
			SavePoint point;
			s.syncAsUint32LE(point.entity1);
			s.syncAsUint32LE(point.action);
			s.syncAsUint32LE(point.entity2);
			s.syncAsUint32LE(point.param.intValue);

			_savepoints.push_back(point);

			if (_savepoints.size() >= _savePointsMaxSize)
				break;
		}
	} else {
		for (Common::List<SavePoint>::iterator it = _savepoints.begin(); it != _savepoints.end(); ++it) {
			s.syncAsUint32LE((*it).entity1);
			s.syncAsUint32LE((*it).action);
			s.syncAsUint32LE((*it).entity2);
			s.syncAsUint32LE((*it).param.intValue);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// toString
//////////////////////////////////////////////////////////////////////////
Common::String SavePoints::toString() {
	Common::String ret = "";

	ret += "Savepoint Data\n";
	for (uint i = 0; i < _data.size(); i++)
		ret += _data[i].toString() + "\n";

	ret += "\nSavepoints\n";
	for (Common::List<SavePoint>::iterator it = _savepoints.begin(); it != _savepoints.end(); ++it)
		ret += (*it).toString() + "\n";

	return ret;
}

} // End of namespace LastExpress
