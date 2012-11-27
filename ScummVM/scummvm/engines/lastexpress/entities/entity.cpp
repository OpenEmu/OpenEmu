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

#include "lastexpress/entities/entity.h"

#include "lastexpress/data/sequence.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"
#include "lastexpress/game/scenes.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

//////////////////////////////////////////////////////////////////////////
// EntityData
//////////////////////////////////////////////////////////////////////////

EntityData::EntityCallData::~EntityCallData() {
	SAFE_DELETE(frame);
	SAFE_DELETE(frame1);

	SAFE_DELETE(sequence);
	SAFE_DELETE(sequence2);
	SAFE_DELETE(sequence3);
}

void EntityData::EntityCallData::syncString(Common::Serializer &s, Common::String &string, uint length) const {
	char seqName[13];
	memset(&seqName, 0, length);

	if (s.isSaving())
		strcpy((char *)&seqName, string.c_str());

	s.syncBytes((byte *)&seqName, length);

	if (s.isLoading())
		string = seqName;
}

void EntityData::EntityCallData::saveLoadWithSerializer(Common::Serializer &s) {
	for (uint i = 0; i < ARRAYSIZE(callbacks); i++)
		s.syncAsByte(callbacks[i]);

	s.syncAsByte(currentCall);
	s.syncAsUint16LE(entityPosition);
	s.syncAsUint16LE(location);
	s.syncAsUint16LE(car);
	s.syncAsByte(field_497);
	s.syncAsByte(entity);
	s.syncAsByte(inventoryItem);
	s.syncAsByte(direction);
	s.syncAsUint16LE(field_49B);
	s.syncAsUint16LE(currentFrame);
	s.syncAsUint16LE(currentFrame2);
	s.syncAsUint16LE(field_4A1);
	s.syncAsUint16LE(field_4A3);
	s.syncAsByte(clothes);
	s.syncAsByte(position);
	s.syncAsByte(car2);
	s.syncAsByte(doProcessEntity);
	s.syncAsByte(field_4A9);
	s.syncAsByte(field_4AA);
	s.syncAsByte(directionSwitch);

	// Sync strings
	syncString(s, sequenceName, 13);
	syncString(s, sequenceName2, 13);
	syncString(s, sequenceNamePrefix, 7);
	syncString(s, sequenceNameCopy, 13);

	// Skip pointers to frame & sequences
	// (we are using a compressed stream, so we cannot seek on load)
	if (s.isLoading()) {
		byte empty[5 * 4];
		s.syncBytes(empty, 5 * 4);
	} else {
		s.skip(5 * 4);
	}
}

//////////////////////////////////////////////////////////////////////////
// EntityData
//////////////////////////////////////////////////////////////////////////
EntityData::EntityParameters *EntityData::getParameters(uint callback, byte index) const {
	if (callback >= 9)
		error("[EntityData::getParameters] Invalid callback value (was: %d, max: 9)", callback);

	if (index >= 4)
		error("[EntityData::getParameters] Invalid index value (was: %d, max: 4)", index);

	return _parameters[callback].parameters[index];
}

byte EntityData::getCallback(uint callback) const {
	if (callback >= 16)
		error("[EntityData::getCallback] Invalid callback value (was: %d, max: 16)", callback);

	return _data.callbacks[callback];
}

void EntityData::setCallback(uint callback, byte index) {
	if (callback >= 16)
		error("[EntityData::setCallback] Invalid callback value (was: %d, max: 16)", callback);

	_data.callbacks[callback] = index;
}

void EntityData::updateParameters(uint32 index) const {
	if (index < 8)
		getParameters(8, 0)->update(index);
	else if (index < 16)
		getParameters(8, 1)->update(index - 8);
	else if (index < 24)
		getParameters(8, 2)->update(index - 16);
	else if (index < 32)
		getParameters(8, 3)->update(index - 24);
	else
		error("[EntityData::updateParameters] Invalid param index to update (was:%d, max:32)", index);
}

void EntityData::saveLoadWithSerializer(Common::Serializer &s) {
	for (uint i = 0; i < ARRAYSIZE(_parameters); i++)
		_parameters[i].saveLoadWithSerializer(s);

	_data.saveLoadWithSerializer(s);
}

//////////////////////////////////////////////////////////////////////////
// Entity
//////////////////////////////////////////////////////////////////////////
Entity::Entity(LastExpressEngine *engine, EntityIndex index) : _engine(engine), _entityIndex(index) {
	_data = new EntityData();

	// Add first empty entry to callbacks array
	_callbacks.push_back(NULL);
}

Entity::~Entity() {
	for (uint i = 0; i < _callbacks.size(); i++)
		SAFE_DELETE(_callbacks[i]);

	_callbacks.clear();

	SAFE_DELETE(_data);

	// Zero-out passed pointers
	_engine = NULL;
}

void Entity::setup(ChapterIndex index) {
	switch(index) {
	case kChapterAll:
		getSavePoints()->setCallback(_entityIndex, _callbacks[_data->getCurrentCallback()]);
		break;

	case kChapter1:
		setup_chapter1();
		break;

	case kChapter2:
		setup_chapter2();
		break;

	case kChapter3:
		setup_chapter3();
		break;

	case kChapter4:
		setup_chapter4();
		break;

	case kChapter5:
		setup_chapter5();
		break;

	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// Shared functions
//////////////////////////////////////////////////////////////////////////

void Entity::reset(const SavePoint &savepoint, bool resetClothes, bool resetItem) {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	switch (savepoint.action) {
	default:
		break;

	case kAction1:
		if (resetClothes) {
			// Select next available clothes
			getData()->clothes = (ClothesIndex)(getData()->clothes + 1);
			if (getData()->clothes > kClothes3)
				getData()->clothes = kClothesDefault;
		}
		break;

	case kActionNone:
		if (getEntities()->updateEntity(_entityIndex, kCarGreenSleeping, (EntityPosition)params->param1))
			params->param1 = (params->param1 == 10000) ? 0 : 10000;
		break;

	case kActionDefault:
		getData()->entityPosition = kPositionNone;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;

		if (resetItem)
			getData()->inventoryItem = kItemInvalid;

		params->param1 = 10000;
		break;
	}
}

void Entity::savegame(const SavePoint &savepoint) {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		callbackAction();
		break;

	case kActionDefault:
		getSaveLoad()->saveGame((SavegameType)params->param1, _entityIndex, (EventIndex)params->param2);
		callbackAction();
		break;
	}
}

void Entity::savegameBloodJacket() {
	if (getProgress().jacket == kJacketBlood
	 && getEntities()->isDistanceBetweenEntities(_entityIndex, kEntityPlayer, 1000)
	 && !getEntities()->isInsideCompartments(kEntityPlayer)
	 && !getEntities()->checkFields10(kEntityPlayer)) {
		setCallback(1);

		switch (_entityIndex) {
		default:
			break;

		case kEntityCoudert:
			setup_savegame(kSavegameTypeEvent, kEventCoudertBloodJacket);
			break;

		case kEntityMertens:
			setup_savegame(kSavegameTypeEvent, kEventCoudertBloodJacket);
			break;
		}
	}
}

void Entity::playSound(const SavePoint &savepoint, bool resetItem, SoundFlag flag) {
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS)

	switch (savepoint.action) {
	default:
		break;

	case kActionEndSound:
		callbackAction();
		break;

	case kActionDefault:
		if (resetItem)
			getData()->inventoryItem = kItemNone;

		getSound()->playSound(_entityIndex, (char *)&params->seq1, flag);
		break;
	}
}

void Entity::draw(const SavePoint &savepoint, bool handleExcuseMe) {
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS)

	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		callbackAction();
		break;

	case kActionExcuseMeCath:
		if (handleExcuseMe && !params->param4) {
			getSound()->excuseMe(_entityIndex);
			params->param4 = 1;
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(_entityIndex, (char *)&params->seq1);
		break;
	}
}

void Entity::draw2(const SavePoint &savepoint) {
	EXPOSE_PARAMS(EntityData::EntityParametersSSII)

	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		callbackAction();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(_entityIndex, (char *)&params->seq1);
		getEntities()->drawSequenceRight((EntityIndex)params->param7, (char *)&params->seq2);
		break;
	}
}

void Entity::updateFromTicks(const SavePoint &savepoint) {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::updateParameter(params->param2, getState()->timeTicks, params->param1))
			break;

		callbackAction();
		break;
	}
}

void Entity::updateFromTime(const SavePoint &savepoint) {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::updateParameter(params->param2, getState()->time, params->param1))
			break;

		callbackAction();
		break;
	}
}

void Entity::callbackActionOnDirection(const SavePoint &savepoint) {
	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		callbackAction();
		break;

	case kActionDefault:
		if (getData()->direction != kDirectionRight)
			callbackAction();
		break;
	}
}

void Entity::callbackActionRestaurantOrSalon(const SavePoint &savepoint) {
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
	case kActionDefault:
		if (getEntities()->isSomebodyInsideRestaurantOrSalon())
			callbackAction();
		break;
	}
}

void Entity::updateEntity(const SavePoint &savepoint, bool handleExcuseMe) {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	switch (savepoint.action) {
	default:
		break;

	case kActionExcuseMeCath:
		if (handleExcuseMe)
			getSound()->excuseMeCath();
		break;

	case kActionExcuseMe:
		if (handleExcuseMe)
			getSound()->excuseMe(_entityIndex);
		break;

	case kActionNone:
	case kActionDefault:
		if (getEntities()->updateEntity(_entityIndex, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;
	}
}

void Entity::callSavepoint(const SavePoint &savepoint, bool handleExcuseMe) {
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS)

	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		if (!CURRENT_PARAM(1, 1))
			getSavePoints()->call(_entityIndex, (EntityIndex)params->param4, (ActionIndex)params->param5, (char *)&params->seq2);
		callbackAction();
		break;

	case kActionExcuseMeCath:
		if (handleExcuseMe && !CURRENT_PARAM(1, 2)) {
			getSound()->excuseMe(_entityIndex);
			CURRENT_PARAM(1, 2) = 1;
		}
		break;

	case kAction10:
		if (!CURRENT_PARAM(1, 1)) {
			getSavePoints()->call(_entityIndex, (EntityIndex)params->param4, (ActionIndex)params->param5, (char *)&params->seq2);
			CURRENT_PARAM(1, 1) = 1;
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(_entityIndex, (char *)&params->seq1);
		break;
	}
}

void Entity::enterExitCompartment(const SavePoint &savepoint, EntityPosition position1, EntityPosition position2, CarIndex car, ObjectIndex compartment, bool alternate, bool updateLocation) {
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS)

	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		getEntities()->exitCompartment(_entityIndex, (ObjectIndex)params->param4);
		if (position1)
			getData()->entityPosition = position1;

		if (updateLocation)
			getData()->location = kLocationInsideCompartment;

		callbackAction();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(_entityIndex, (char *)&params->seq1);
		getEntities()->enterCompartment(_entityIndex, (ObjectIndex)params->param4);

		if (position1) {
			getData()->location = kLocationInsideCompartment;

			if (getEntities()->isInsideCompartment(kEntityPlayer, car, position1) || getEntities()->isInsideCompartment(kEntityPlayer, car, position2)) {
				getAction()->playAnimation(isNight() ? kEventCathTurningNight : kEventCathTurningDay);
				getSound()->playSound(kEntityPlayer, "BUMP");
				getScenes()->loadSceneFromObject(compartment, alternate);
			}
		}
		break;
	}
}

void Entity::goToCompartment(const SavePoint &savepoint, ObjectIndex compartmentFrom, EntityPosition positionFrom, Common::String sequenceFrom, Common::String sequenceTo) {
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = positionFrom;
		setCallback(1);
		setup_enterExitCompartment(sequenceFrom.c_str(), compartmentFrom);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment(sequenceTo.c_str(), compartmentFrom);
			break;

		case 2:
			getData()->entityPosition = positionFrom;
			getEntities()->clearSequences(_entityIndex);
			callbackAction();
			break;
		}
		break;
	}
}

void Entity::goToCompartmentFromCompartment(const SavePoint &savepoint, ObjectIndex compartmentFrom, EntityPosition positionFrom, Common::String sequenceFrom, ObjectIndex compartmentTo, EntityPosition positionTo, Common::String sequenceTo) {
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = positionFrom;
		getData()->location = kLocationOutsideCompartment;
		setCallback(1);
		setup_enterExitCompartment(sequenceFrom.c_str(), compartmentFrom);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, positionTo);
			break;

		case 2:
			setCallback(3);
			setup_enterExitCompartment(sequenceTo.c_str(), compartmentTo);
			break;

		case 3:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(_entityIndex);
			callbackAction();
			break;
		}
		break;
	}
}

void Entity::updatePosition(const SavePoint &savepoint, bool handleExcuseMe) {
	EXPOSE_PARAMS(EntityData::EntityParametersSIII)

	switch (savepoint.action) {
	default:
		break;

	case kActionExitCompartment:
		getEntities()->updatePositionExit(_entityIndex, (CarIndex)params->param4, (Position)params->param5);
		callbackAction();
		break;

	case kActionExcuseMeCath:
		if (handleExcuseMe && !params->param6) {
			getSound()->excuseMe(_entityIndex);
			params->param6 = 1;
		}
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(_entityIndex, (char *)&params->seq);
		getEntities()->updatePositionEnter(_entityIndex, (CarIndex)params->param4, (Position)params->param5);
		break;
	}
}

void Entity::callbackAction() {
	if (getData()->currentCall == 0)
		error("[Entity::callbackAction] currentCall is already 0, cannot proceed");

	getData()->currentCall--;

	getSavePoints()->setCallback(_entityIndex, _callbacks[_data->getCurrentCallback()]);

	getSavePoints()->call(_entityIndex, _entityIndex, kActionCallback);
}

//////////////////////////////////////////////////////////////////////////
// Setup functions
//////////////////////////////////////////////////////////////////////////
void Entity::setup(const char *name, uint index) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s()", name);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersIIII>();

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupI(const char *name, uint index, uint param1) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%u)", name, param1);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersIIII>();

	EntityData::EntityParametersIIII *params = (EntityData::EntityParametersIIII *)_data->getCurrentParameters();
	params->param1 = (unsigned int)param1;

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupII(const char *name, uint index, uint param1, uint param2) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%u, %u)", name, param1, param2);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersIIII>();

	EntityData::EntityParametersIIII *params = (EntityData::EntityParametersIIII *)_data->getCurrentParameters();
	params->param1 = param1;
	params->param2 = param2;

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupIII(const char *name, uint index, uint param1, uint param2, uint param3) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%u, %u, %u)", name, param1, param2, param3);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersIIII>();

	EntityData::EntityParametersIIII *params = (EntityData::EntityParametersIIII *)_data->getCurrentParameters();
	params->param1 = param1;
	params->param2 = param2;
	params->param3 = param3;

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupS(const char *name, uint index, const char *seq1) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%s)", name, seq1);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersSIIS>();

	EntityData::EntityParametersSIIS *params = (EntityData::EntityParametersSIIS*)_data->getCurrentParameters();
	strncpy(params->seq1, seq1, 12);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupSS(const char *name, uint index, const char *seq1, const char *seq2) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%s, %s)", name, seq1, seq2);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersSSII>();

	EntityData::EntityParametersSSII *params = (EntityData::EntityParametersSSII*)_data->getCurrentParameters();
	strncpy(params->seq1, seq1, 12);
	strncpy(params->seq2, seq2, 12);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupSI(const char *name, uint index, const char *seq1, uint param4) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%s, %u)", name, seq1, param4);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersSIIS>();

	EntityData::EntityParametersSIIS *params = (EntityData::EntityParametersSIIS *)_data->getCurrentParameters();
	strncpy(params->seq1, seq1, 12);
	params->param4 = param4;

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupSII(const char *name, uint index, const char *seq1, uint param4, uint param5) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%s, %u, %u)", name, seq1, param4, param5);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersSIIS>();

	EntityData::EntityParametersSIIS *params = (EntityData::EntityParametersSIIS *)_data->getCurrentParameters();
	strncpy(params->seq1, seq1, 12);
	params->param4 = param4;
	params->param5 = param5;

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupSIII(const char *name, uint index, const char *seq, uint param4, uint param5, uint param6) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%s, %u, %u, %u)", name, seq, param4, param5, param6);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersSIII>();

	EntityData::EntityParametersSIII *params = (EntityData::EntityParametersSIII *)_data->getCurrentParameters();
	strncpy(params->seq, seq, 12);
	params->param4 = param4;
	params->param5 = param5;
	params->param6 = param6;

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupSIIS(const char *name, uint index, const char *seq1, uint param4, uint param5, const char *seq2) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%s, %u, %u, %s)", name, seq1, param4, param5, seq2);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersSIIS>();

	EntityData::EntityParametersSIIS *params = (EntityData::EntityParametersSIIS *)_data->getCurrentParameters();
	strncpy(params->seq1, seq1, 12);
	params->param4 = param4;
	params->param5 = param5;
	strncpy(params->seq2, seq2, 12);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupSSI(const char *name, uint index, const char *seq1, const char *seq2, uint param7) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%s, %s, %u)", name, seq1, seq2, param7);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersSSII>();

	EntityData::EntityParametersSSII *params = (EntityData::EntityParametersSSII *)_data->getCurrentParameters();
	strncpy(params->seq1, seq1, 12);
	strncpy(params->seq2, seq2, 12);
	params->param7 = param7;

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupIS(const char *name, uint index, uint param1, const char *seq) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%u, %s)", name, param1, seq);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersISII>();

	EntityData::EntityParametersISII *params = (EntityData::EntityParametersISII *)_data->getCurrentParameters();
	params->param1 = (unsigned int)param1;
	strncpy(params->seq, seq, 12);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupISS(const char *name, uint index, uint param1, const char *seq1, const char *seq2) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%u, %s, %s)", name, param1, seq1, seq2);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersISSI>();

	EntityData::EntityParametersISSI *params = (EntityData::EntityParametersISSI *)_data->getCurrentParameters();
	params->param1 = param1;
	strncpy(params->seq1, seq1, 12);
	strncpy(params->seq2, seq2, 12);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupIIS(const char *name, uint index, uint param1, uint param2, const char *seq) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%u, %u, %s)", name, param1, param2, seq);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersIISI>();

	EntityData::EntityParametersIISI *params = (EntityData::EntityParametersIISI *)_data->getCurrentParameters();
	params->param1 = param1;
	params->param2 = param2;
	strncpy(params->seq, seq, 12);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

void Entity::setupIISS(const char *name, uint index, uint param1, uint param2, const char *seq1, const char *seq2) {
	debugC(6, kLastExpressDebugLogic, "Entity: %s(%u, %u, %s, %s)", name, param1, param2, seq1, seq2);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->setCallback(_entityIndex, _callbacks[index]);
	_data->setCurrentCallback(index);
	_data->resetCurrentParameters<EntityData::EntityParametersIISS>();

	EntityData::EntityParametersIISS *params = (EntityData::EntityParametersIISS *)_data->getCurrentParameters();
	params->param1 = param1;
	params->param2 = param2;
	strncpy(params->seq1, seq1, 12);
	strncpy(params->seq2, seq2, 12);

	_engine->getGameLogic()->getGameState()->getGameSavePoints()->call(_entityIndex, _entityIndex, kActionDefault);
}

//////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////

bool Entity::updateParameter(uint &parameter, uint timeType, uint delta) const {
	if (!parameter)
		parameter = (uint)(timeType + delta);

	if (parameter >= timeType)
		return false;

	parameter = kTimeInvalid;

	return true;
}

bool Entity::updateParameterTime(TimeValue timeValue, bool check, uint &parameter, uint delta) const {
	if (getState()->time <= timeValue) {
		if (check || !parameter)
			parameter = (uint)(getState()->time + delta);
	}

	if (parameter >= getState()->time && getState()->time <= timeValue)
		return false;

	parameter = kTimeInvalid;

	return true;
}

bool Entity::updateParameterCheck(uint &parameter, uint timeType, uint delta) const {
	if (parameter && parameter >= timeType)
		return false;

	if (!parameter)
		parameter = (uint)(timeType + delta);

	return true;
}

bool Entity::timeCheck(TimeValue timeValue, uint &parameter, Common::Functor0<void> *function) const {
	if (getState()->time > timeValue && !parameter) {
		parameter = 1;
		(*function)();

		return true;
	}

	return false;
}

bool Entity::timeCheckCallback(TimeValue timeValue, uint &parameter, byte callback, Common::Functor0<void> *function) {
	if (getState()->time > timeValue && !parameter) {
		parameter = 1;
		setCallback(callback);
		(*function)();

		return true;
	}

	return false;
}

bool Entity::timeCheckCallback(TimeValue timeValue, uint &parameter, byte callback, const char *str, Common::Functor1<const char *, void> *function) {
	if (getState()->time > timeValue && !parameter) {
		parameter = 1;
		setCallback(callback);
		(*function)(str);

		return true;
	}

	return false;
}

bool Entity::timeCheckCallback(TimeValue timeValue, uint &parameter, byte callback, bool check, Common::Functor1<bool, void> *function) {
	if (getState()->time > timeValue && !parameter) {
		parameter = 1;
		setCallback(callback);
		(*function)(check);

		return true;
	}

	return false;
}

bool Entity::timeCheckCallbackInventory(TimeValue timeValue, uint &parameter, byte callback, Common::Functor0<void> *function) {
	if (getState()->time > timeValue && !parameter) {
		parameter = 1;
		getData()->inventoryItem = kItemNone;
		setCallback(callback);
		(*function)();

		return true;
	}

	return false;
}

bool Entity::timeCheckCar(TimeValue timeValue, uint &parameter, byte callback, Common::Functor0<void> *function) {
	if ((getState()->time <= timeValue && !getEntities()->isPlayerInCar(kCarGreenSleeping)) || !parameter)
		parameter = (uint)getState()->time + 75;

	if (getState()->time > timeValue || parameter < getState()->time) {
		parameter = kTimeInvalid;
		setCallback(callback);
		(*function)();

		return true;
	}

	return false;
}

void Entity::timeCheckSavepoint(TimeValue timeValue, uint &parameter, EntityIndex entity1, EntityIndex entity2, ActionIndex action) const {
	if (getState()->time > timeValue && !parameter) {
		parameter = 1;
		getSavePoints()->push(entity1, entity2, action);
	}
}

void Entity::timeCheckObject(TimeValue timeValue, uint &parameter, ObjectIndex object, ObjectLocation location) const {
	if (getState()->time > timeValue && !parameter) {
		parameter = 1;
		getObjects()->updateLocation2(object, location);
	}
}

bool Entity::timeCheckCallbackAction(TimeValue timeValue, uint &parameter) {
	if (getState()->time > timeValue && !parameter) {
		parameter = 1;
		callbackAction();
		return true;
	}

	return false;
}

bool Entity::timeCheckPlaySoundUpdatePosition(TimeValue timeValue, uint &parameter, byte callback, const char* sound, EntityPosition position) {
	if (getState()->time > timeValue && !parameter) {
		parameter = 1;
		getData()->entityPosition = position;
		setCallback(callback);
		setup_playSound(sound);
		return true;
	}

	return false;
}


} // End of namespace LastExpress
