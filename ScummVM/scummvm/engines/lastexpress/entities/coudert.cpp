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

#include "lastexpress/entities/coudert.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Coudert::Coudert(LastExpressEngine *engine) : Entity(engine, kEntityCoudert) {
	ADD_CALLBACK_FUNCTION(Coudert, reset);
	ADD_CALLBACK_FUNCTION(Coudert, bloodJacket);
	ADD_CALLBACK_FUNCTION(Coudert, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(Coudert, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(Coudert, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION(Coudert, playSound);
	ADD_CALLBACK_FUNCTION(Coudert, playSound16);
	ADD_CALLBACK_FUNCTION(Coudert, savegame);
	ADD_CALLBACK_FUNCTION(Coudert, updateEntity);
	ADD_CALLBACK_FUNCTION(Coudert, updateFromTime);
	ADD_CALLBACK_FUNCTION(Coudert, updateFromTicks);
	ADD_CALLBACK_FUNCTION(Coudert, excuseMe);
	ADD_CALLBACK_FUNCTION(Coudert, function13);
	ADD_CALLBACK_FUNCTION(Coudert, function14);
	ADD_CALLBACK_FUNCTION(Coudert, function15);
	ADD_CALLBACK_FUNCTION(Coudert, function16);
	ADD_CALLBACK_FUNCTION(Coudert, function17);
	ADD_CALLBACK_FUNCTION(Coudert, function18);
	ADD_CALLBACK_FUNCTION(Coudert, function19);
	ADD_CALLBACK_FUNCTION(Coudert, function20);
	ADD_CALLBACK_FUNCTION(Coudert, function21);
	ADD_CALLBACK_FUNCTION(Coudert, function22);
	ADD_CALLBACK_FUNCTION(Coudert, function23);
	ADD_CALLBACK_FUNCTION(Coudert, visitCompartmentF);
	ADD_CALLBACK_FUNCTION(Coudert, function25);
	ADD_CALLBACK_FUNCTION(Coudert, function26);
	ADD_CALLBACK_FUNCTION(Coudert, function27);
	ADD_CALLBACK_FUNCTION(Coudert, visitCompartmentB);
	ADD_CALLBACK_FUNCTION(Coudert, visitCompartmentA);
	ADD_CALLBACK_FUNCTION(Coudert, function30);
	ADD_CALLBACK_FUNCTION(Coudert, function31);
	ADD_CALLBACK_FUNCTION(Coudert, function32);
	ADD_CALLBACK_FUNCTION(Coudert, function33);
	ADD_CALLBACK_FUNCTION(Coudert, function34);
	ADD_CALLBACK_FUNCTION(Coudert, function35);
	ADD_CALLBACK_FUNCTION(Coudert, chapter1);
	ADD_CALLBACK_FUNCTION(Coudert, function37);
	ADD_CALLBACK_FUNCTION(Coudert, function38);
	ADD_CALLBACK_FUNCTION(Coudert, function39);
	ADD_CALLBACK_FUNCTION(Coudert, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Coudert, function41);
	ADD_CALLBACK_FUNCTION(Coudert, chapter2);
	ADD_CALLBACK_FUNCTION(Coudert, function43);
	ADD_CALLBACK_FUNCTION(Coudert, chapter3);
	ADD_CALLBACK_FUNCTION(Coudert, function45);
	ADD_CALLBACK_FUNCTION(Coudert, function46);
	ADD_CALLBACK_FUNCTION(Coudert, function47);
	ADD_CALLBACK_FUNCTION(Coudert, function48);
	ADD_CALLBACK_FUNCTION(Coudert, function49);
	ADD_CALLBACK_FUNCTION(Coudert, function50);
	ADD_CALLBACK_FUNCTION(Coudert, function51);
	ADD_CALLBACK_FUNCTION(Coudert, chapter4);
	ADD_CALLBACK_FUNCTION(Coudert, function53);
	ADD_CALLBACK_FUNCTION(Coudert, function54);
	ADD_CALLBACK_FUNCTION(Coudert, function55);
	ADD_CALLBACK_FUNCTION(Coudert, function56);
	ADD_CALLBACK_FUNCTION(Coudert, chapter5);
	ADD_CALLBACK_FUNCTION(Coudert, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Coudert, function59);
	ADD_CALLBACK_FUNCTION(Coudert, function60);
	ADD_CALLBACK_FUNCTION(Coudert, function61);
	ADD_CALLBACK_FUNCTION(Coudert, function62);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Coudert, reset)
	Entity::reset(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Coudert, bloodJacket)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		break;

	case kActionExitCompartment:
		callbackAction();
		break;

	case kActionDefault:
		getEntities()->drawSequenceRight(kEntityCoudert, (char *)&params->seq1);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(3, Coudert, enterExitCompartment, ObjectIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		return;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		return;
	}

	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(4, Coudert, callbackActionOnDirection)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getData()->direction != kDirectionRight) {
			callbackAction();
			break;
		}

		Entity::savegameBloodJacket();
		break;

	case kActionExitCompartment:
		callbackAction();
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SIII(5, Coudert, enterExitCompartment2, ObjectIndex, EntityPosition, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		return;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		return;
	}

	Entity::enterExitCompartment(savepoint, (EntityPosition)params->param5, (EntityPosition)params->param6, kCarRedSleeping, (ObjectIndex)params->param4, false);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(6, Coudert, playSound)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		break;

	case kActionEndSound:
		callbackAction();
		break;

	case kActionDefault:
		getSound()->playSound(kEntityCoudert, (char *)&params->seq1);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(7, Coudert, playSound16)
	EXPOSE_PARAMS(EntityData::EntityParametersSIIS);

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		break;

	case kActionEndSound:
		callbackAction();
		break;

	case kActionDefault:
		getSound()->playSound(kEntityCoudert, (char *)&params->seq1, kFlagDefault);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(8, Coudert, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(9, Coudert, updateEntity, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param3 && getEntities()->isDistanceBetweenEntities(kEntityCoudert, kEntityPlayer, 2000))
			getData()->inventoryItem = kItemInvalid;
		else
			getData()->inventoryItem = kItemNone;

		if (getProgress().jacket != kJacketBlood
		 || !getEntities()->isDistanceBetweenEntities(kEntityCoudert, kEntityPlayer, 1000)
		 || getEntities()->isInsideCompartments(kEntityPlayer)
		 || getEntities()->checkFields10(kEntityPlayer)) {
			if (getEntities()->updateEntity(kEntityCoudert, (CarIndex)params->param1, (EntityPosition)params->param2)) {
				getData()->inventoryItem = kItemNone;

				callbackAction();
			}
			break;
		}

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventMertensBloodJacket);
		break;

	case kAction1:
		params->param3 = 0;
		getData()->inventoryItem = kItemNone;

		setCallback(2);
		setup_savegame(kSavegameTypeEvent, kEventCoudertAskTylerCompartment);
		break;

	case kActionExcuseMeCath:
		if (getData()->clothes == kClothes1)
			getSound()->playSound(kEntityPlayer, "ZFX1003", getSound()->getSoundFlag(kEntityCoudert));
		else if (!getSoundQueue()->isBuffered(kEntityCoudert))
			getSound()->playSound(kEntityPlayer, "JAC1112", getSound()->getSoundFlag(kEntityCoudert));
		break;

	case kActionExcuseMe:
		if (getData()->clothes == kClothes1)
			getSound()->playSound(kEntityPlayer, "ZFX1003", getSound()->getSoundFlag(kEntityCoudert));
		else
			getSound()->excuseMe(kEntityCoudert);
		break;

	case kActionDefault:
		if (!getProgress().eventCorpseFound && !getEvent(kEventCoudertAskTylerCompartment))
			params->param3 = kItemInvalid;

		if (getEntities()->updateEntity(kEntityCoudert, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			break;

		case 2:
			getAction()->playAnimation(kEventCoudertAskTylerCompartment);

			if (getData()->direction != kDirectionUp)
				getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + 750));
			else
				getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition - 750), true);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(10, Coudert, updateFromTime, uint32)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();

		if (!Entity::updateParameter(params->param2, getState()->time, params->param1))
			break;

		callbackAction();
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(11, Coudert, updateFromTicks, uint32)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();

		if (!Entity::updateParameter(params->param2, getState()->timeTicks, params->param1))
			break;

		callbackAction();
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
// Parameters
//  - EntityIndex
IMPLEMENT_FUNCTION_I(12, Coudert, excuseMe, EntityIndex)
	if (savepoint.action != kActionDefault)
		return;

	if (getSoundQueue()->isBuffered(kEntityCoudert)) {
		callbackAction();
		return;
	}

	if (isNight()) {
		if (Entities::isFemale((EntityIndex)params->param1)) {
			getSound()->playSound(kEntityCoudert, Entities::isMarried((EntityIndex)params->param1) ? "JAC1112C" : "JAC1112F");
		} else {
			if (!params->param1 && getProgress().field_18 == 2) {
				switch (rnd(4)) {
				default:
					break;

				case 0:
					getSound()->playSound(kEntityCoudert, "JAC1013");
					break;

				case 1:
					getSound()->playSound(kEntityCoudert, "JAC1013A");
					break;

				case 2:
					getSound()->playSound(kEntityCoudert, "JAC1113");
					break;

				case 3:
					getSound()->playSound(kEntityCoudert, "JAC1113A");
					break;
				}
			} else {
				getSound()->playSound(kEntityCoudert, "JAC1112D");
			}
		}
	} else {
		if (Entities::isFemale((EntityIndex)params->param1))
			getSound()->playSound(kEntityCoudert, Entities::isMarried((EntityIndex)params->param1) ? "JAC1112B" : "JAC1112G");
		else
			getSound()->playSound(kEntityCoudert, "JAC1112E");
	}

	callbackAction();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(13, Coudert, function13, bool, EntityIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();

		if (!params->param2 && !params->param3) {

			if (!params->param4) {
				params->param4 = getState()->timeTicks + 75;

				if (!params->param4) {
					getData()->inventoryItem = kItemNone;
					setCallback(4);
					setup_function19(true);
					break;
				}
			}

			if (params->param4 < getState()->timeTicks) {
				params->param4 = kTimeInvalid;

				getData()->inventoryItem = kItemNone;
				setCallback(4);
				setup_function19(true);
				break;
			}
		}

		if (!Entity::updateParameter(params->param5, getState()->timeTicks, 225))
			break;

		getData()->inventoryItem = kItemNone;
		setCallback(5);
		setup_function19(true);
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;

		setCallback(9);
		setup_savegame(kSavegameTypeEvent, kEventCoudertAskTylerCompartment);
		break;

	case kAction11:
		++params->param3;

		setCallback(8);
		setup_excuseMe(savepoint.entity2);
		break;

	case kActionDefault:
		if (params->param2)
			params->param3 = 1;

		setCallback(1);
		setup_excuseMe((EntityIndex)params->param2);
		break;

	case kAction16:
		--params->param3;

		if (params->param2 && !params->param3) {
			getData()->inventoryItem = kItemNone;
			setCallback(7);
			setup_function19(true);
		}
		break;

	case kActionDrawScene:
		if (!params->param3) {
			getData()->inventoryItem = kItemNone;
			setCallback(6);
			setup_function19(true);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function17(true);
			break;

		case 2:
			if (getProgress().chapter == kChapter1 && !getProgress().eventCorpseFound && !getEvent(kEventCoudertAskTylerCompartment))
				getData()->inventoryItem = kItemInvalid;

			getEntities()->drawSequenceLeft(kEntityCoudert, params->param1 ? "667I" : "667H");
			break;

		case 3:
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			// BUG: the original game continues executing code here
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			callbackAction();
			break;

		case 9:
			getAction()->playAnimation(kEventCoudertAskTylerCompartment);
			getScenes()->loadSceneFromPosition(kCarRedSleeping, 25);
			break;
		}
		break;

	case kAction201439712:
		getEntities()->drawSequenceLeft(kEntityCoudert, "627K");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(14, Coudert, function14, EntityIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::savegameBloodJacket();
		break;

	case kActionDefault:
		if (ENTITY_PARAM(2, 1)) {
			ENTITY_PARAM(2, 1) = 0;

			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_1500);
		} else {
			setCallback(1);
			setup_updateFromTime(15);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityCoudert, (EntityIndex)params->param1, kAction202558662);

			setCallback(2);
			setup_function17(false);
			break;

		case 2:
			getSavePoints()->push(kEntityCoudert, (EntityIndex)params->param1, kAction155853632);
			getEntities()->drawSequenceLeft(kEntityCoudert, "627K");
			break;

		case 3:
			getSavePoints()->push(kEntityCoudert, (EntityIndex)params->param1, kAction202558662);
			getSavePoints()->push(kEntityCoudert, (EntityIndex)params->param1, kAction155853632);
			getEntities()->drawSequenceLeft(kEntityCoudert, "627K");
			getScenes()->loadSceneFromItemPosition(kItem5);
			break;

		case 4:
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			break;

		case 5:
			callbackAction();
			break;
		}
		break;

	case kAction125499160:
		switch (params->param1) {
		default:
			break;

		case kEntityVerges:
			ENTITY_PARAM(0, 3) = 0;
			break;

		case kEntityMmeBoutarel:
			ENTITY_PARAM(0, 4) = 0;
			break;

		case kEntityMertens:
			ENTITY_PARAM(0, 5) = 0;
			break;
		}

		setCallback(5);
		setup_function19(false);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(15, Coudert, function15, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		ENTITY_PARAM(0, 8) = 0;
		ENTITY_PARAM(1, 1) = 0;

		setCallback(1);
		setup_function16();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_7500);
			break;

		case 2:
			if (params->param1)
				getSound()->playSound(kEntityCoudert, "Tat3163");
			else
				getSound()->playSound(kEntityCoudert, (getProgress().chapter != kChapter3 || getState()->time > kTime1449000) ? "Tat3162A" : "Tat3161A");

			setCallback(3);
			setup_enterExitCompartment("627Xb", kObjectCompartmentB);
			break;

		case 3:
			getSavePoints()->push(kEntityCoudert, kEntityTatiana, kAction69239528);
			getData()->entityPosition = kPosition_7250;

			setCallback(4);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 4:
			setCallback(5);
			setup_function18();
			break;

		case 5:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Coudert, function16)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (ENTITY_PARAM(2, 1)) {
			ENTITY_PARAM(2, 1) = 0;
			getInventory()->setLocationAndProcess(kItem5, kObjectLocation1);

			callbackAction();
			break;
		}

		setCallback(ENTITY_PARAM(0, 2) ? 1 : 2);
		setup_bloodJacket(ENTITY_PARAM(0, 2) ? "627C" : "627F");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			getInventory()->setLocationAndProcess(kItem5, kObjectLocation1);
			if (!getEntities()->isPlayerPosition(kCarRedSleeping, 2))
				getData()->entityPosition = kPosition_2088;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(17, Coudert, function17, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getScenes()->loadSceneFromItemPosition(kItem5);

		if (ENTITY_PARAM(2, 1)) {
			ENTITY_PARAM(2, 1) = 0;
			callbackAction();
			break;
		}

		if (params->param1) {
			setCallback(1);
			setup_bloodJacket("627H");
			break;
		}

		if (params->param2) {
			setCallback(2);
			setup_bloodJacket("627C");
			break;
		}

		setCallback(3);
		setup_bloodJacket("627F");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
		case 3:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Coudert, function18)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (ENTITY_PARAM(0, 6) || ENTITY_PARAM(0, 8)
		 || ENTITY_PARAM(1, 1) || ENTITY_PARAM(1, 2) || ENTITY_PARAM(1, 3) || ENTITY_PARAM(1, 5) || ENTITY_PARAM(1, 6) || ENTITY_PARAM(1, 7) || ENTITY_PARAM(1, 8)
		 || ENTITY_PARAM(2, 4) || ENTITY_PARAM(2, 6)) {
			getInventory()->setLocationAndProcess(kItem5, kObjectLocation1);

			setCallback(1);
			setup_updateEntity(kCarRedSleeping, kPosition_540);
			break;
		}

		if (ENTITY_PARAM(0, 3) || ENTITY_PARAM(0, 5) || ENTITY_PARAM(0, 4)) {
			getEntities()->drawSequenceLeft(kEntityCoudert, "627K");
			getScenes()->loadSceneFromItemPosition(kItem5);

			callbackAction();
			break;
		}

		getEntities()->drawSequenceRight(kEntityCoudert, ENTITY_PARAM(0, 2) ? "627A" : "627D");
		getScenes()->loadSceneFromItemPosition(kItem5);

		if (getEntities()->isPlayerPosition(kCarRedSleeping, 68)) {
			if (!getSoundQueue()->isBuffered(kEntityCoudert))
				getSound()->playSound(kEntityCoudert, "JAC1111");

			getScenes()->loadSceneFromPosition(kCarRedSleeping, 25);
		}

		setCallback(3);
		setup_callbackActionOnDirection();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->clearSequences(kEntityCoudert);
			ENTITY_PARAM(2, 1) = 1;

			setCallback(2);
			setup_updateFromTime(75);
			break;

		case 2:
			callbackAction();
			break;

		case 3:
			getEntities()->drawSequenceLeft(kEntityCoudert, ENTITY_PARAM(0, 2) ? "627B" : "627E");
			ENTITY_PARAM(0, 1) = 0;
			getSavePoints()->push(kEntityCoudert, kEntityCoudert, kActionDrawScene);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(19, Coudert, function19, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (ENTITY_PARAM(0, 6) || ENTITY_PARAM(0, 8)
		 || ENTITY_PARAM(1, 1) || ENTITY_PARAM(1, 2) || ENTITY_PARAM(1, 3) || ENTITY_PARAM(1, 5) || ENTITY_PARAM(1, 6) || ENTITY_PARAM(1, 7) || ENTITY_PARAM(1, 8)
		 || ENTITY_PARAM(2, 4) || ENTITY_PARAM(2, 6)) {
			getInventory()->setLocationAndProcess(kItem5, kObjectLocation1);
			ENTITY_PARAM(2, 1) = 1;
			callbackAction();
			break;
		}

		if (ENTITY_PARAM(0, 3) || ENTITY_PARAM(0, 5) || ENTITY_PARAM(0, 4)) {
			getScenes()->loadSceneFromItemPosition(kItem5);
			ENTITY_PARAM(2, 1) = 1;
			callbackAction();
			break;
		}

		if (params->param1)
			getEntities()->drawSequenceRight(kEntityCoudert, "697H");
		else
			getEntities()->drawSequenceRight(kEntityCoudert, ENTITY_PARAM(0, 2) ? "627A" : "627D");

		getScenes()->loadSceneFromItemPosition(kItem5);

		setCallback(1);
		setup_callbackActionOnDirection();
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getEntities()->drawSequenceLeft(kEntityCoudert, ENTITY_PARAM(0, 2) ? "627B" : "627E");
			ENTITY_PARAM(0, 1) = 0;

			callbackAction();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(20, Coudert, function20, ObjectIndex, ObjectIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::updateParameter(CURRENT_PARAM(1, 3), getState()->time, 300)) {
			getSound()->playSound(kEntityPlayer, "ZFX1004", getSound()->getSoundFlag(kEntityCoudert));
		}

		if (!Entity::updateParameter(CURRENT_PARAM(1, 4), getState()->time, 900))
			break;

		getObjects()->updateLocation2((ObjectIndex)params->param1, kObjectLocation1);

		if (params->param4 != kObjectLocation2)
			getObjects()->update((ObjectIndex)params->param1, (EntityIndex)params->param3, (ObjectLocation)params->param4, (CursorStyle)params->param5, (CursorStyle)params->param6);

		if (params->param2)
			getObjects()->update((ObjectIndex)params->param2, (EntityIndex)params->param7, (ObjectLocation)params->param8, (CursorStyle)CURRENT_PARAM(1, 1), (CursorStyle)CURRENT_PARAM(1, 2));

		callbackAction();
		break;

	case kActionKnock:
	case kActionOpenDoor:
		getObjects()->update((ObjectIndex)params->param1, kEntityCoudert, kObjectLocation1, kCursorNormal, kCursorNormal);
		if (params->param2)
			getObjects()->update((ObjectIndex)params->param2, kEntityCoudert, kObjectLocation1, kCursorNormal, kCursorNormal);

		setCallback(savepoint.action == kActionKnock ? 1 : 2);
		setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		break;

	case kActionDefault:
		params->param3 = getObjects()->get((ObjectIndex)params->param1).entity;
		params->param4 = getObjects()->get((ObjectIndex)params->param1).location;
		params->param5 = getObjects()->get((ObjectIndex)params->param1).cursor;
		params->param6 = getObjects()->get((ObjectIndex)params->param1).cursor2;

		if (params->param2) {
			params->param7 = getObjects()->get((ObjectIndex)params->param2).entity;
			params->param8 = getObjects()->get((ObjectIndex)params->param2).location;
			CURRENT_PARAM(1, 1) = getObjects()->get((ObjectIndex)params->param2).cursor;
			CURRENT_PARAM(1, 2) = getObjects()->get((ObjectIndex)params->param2).cursor2;

			getObjects()->update((ObjectIndex)params->param2, kEntityCoudert, kObjectLocation1, kCursorHandKnock, kCursorHand);
		}

		if (params->param4 != kObjectLocation2)
			getObjects()->update((ObjectIndex)params->param1, kEntityCoudert, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			if (params->param1 == kObjectCompartmentA || params->param1 == kObjectCompartmentC
			 || params->param1 == kObjectCompartmentG || params->param1 == kObjectCompartmentH) {
				setCallback(3);
				setup_playSound("Jac1001B");
			} else {
				setCallback(4);
				setup_playSound("Jac1001A");
			}
			break;

		case 3:
		case 4:
			getObjects()->update((ObjectIndex)params->param1, kEntityCoudert, kObjectLocation1, kCursorHandKnock, kCursorHand);

			if (params->param2)
				getObjects()->update((ObjectIndex)params->param2, kEntityCoudert, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Coudert, function21)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1) {
			if (!Entity::updateParameter(params->param2, getState()->timeTicks, 75))
				break;

			setCallback(3);
			setup_enterExitCompartment("627Zh", kObjectCompartmentH);
		}
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_2740);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("627Vh", kObjectCompartmentH);
			break;

		case 2:
			getSavePoints()->push(kEntityCoudert, kEntityIvo, kAction221683008);
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Wh");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentH, true);
			break;

		case 3:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentH, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(4);
			setup_function20(kObjectCompartmentH, kObjectNone);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("697Ah", kObjectCompartmentH);
			break;

		case 5:
			getData()->location = kLocationOutsideCompartment;

			callbackAction();
			break;

		case 6:
			getSavePoints()->push(kEntityCoudert, kEntityIvo, kAction122865568);
			break;

		case 7:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentH, true);
			getObjects()->update(kObjectCompartmentH, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(8);
			setup_function20(kObjectCompartmentH, kObjectNone);
			break;

		case 8:
			getSound()->playSound(kEntityCoudert, "JAC1013A");
			getObjects()->update(kObjectCompartmentH, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(9);
			setup_enterExitCompartment("667Uh", kObjectCompartmentH);
			break;

		case 9:
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityCoudert, kEntityIvo, kAction123852928);

			callbackAction();
			break;
		}
		break;

	case kAction88652208:
		setCallback(7);
		setup_enterExitCompartment("667Th", kObjectCompartmentH);
		break;

	case kAction123199584:
		params->param1 = 1;

		setCallback(6);
		setup_playSound("JAC1012");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Coudert, function22)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1) {
			if (!Entity::updateParameter(params->param2, getState()->timeTicks, 75))
				break;

			setCallback(3);
			setup_enterExitCompartment("627Rg", kObjectCompartmentG);
		}
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_3050);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("627Mg", kObjectCompartmentG);
			break;

		case 2:
			getSavePoints()->push(kEntityCoudert, kEntityMilos, kAction221683008);
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Ng");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentG, true);
			break;

		case 3:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentG, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(4);
			setup_function20(kObjectCompartmentG, kObjectNone);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("627Sg", kObjectCompartmentG);
			break;

		case 5:
			getData()->location = kLocationOutsideCompartment;

			callbackAction();
			break;

		case 6:
			getSavePoints()->push(kEntityCoudert, kEntityMilos, kAction122865568);
			break;

		case 7:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentG, true);
			getObjects()->update(kObjectCompartmentG, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(8);
			setup_function20(kObjectCompartmentG, kObjectNone);
			break;

		case 8:
			getSound()->playSound(kEntityCoudert, "JAC1013A");
			getObjects()->update(kObjectCompartmentG, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(9);
			setup_enterExitCompartment("627Ug", kObjectCompartmentG);
			break;

		case 9:
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityCoudert, kEntityMilos, kAction123852928);

			callbackAction();
			break;
		}
		break;

	case kAction88652208:
		setCallback(7);
		setup_enterExitCompartment("627Tg", kObjectCompartmentG);
		break;

	case kAction123199584:
		params->param1 = 1;

		setCallback(6);
		setup_playSound("JAC1030");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Coudert, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_4070);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("627Vf", kObjectCompartmentF);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Wf");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentF, true);
			getSavePoints()->push(kEntityCoudert, kEntityMax, kAction158007856);

			setCallback(3);
			setup_updateFromTime(150);
			break;

		case 3:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentF, true);
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Coudert, visitCompartmentF)
	visitCompartment(savepoint, kPosition_4070, "627Vf", kObjectCompartmentF, "627Wf", "627Zf", kPosition_4455, kObject53, "697Af");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Coudert, function25)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_4840);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("627Me", kObjectCompartmentE);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Ne");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentE, true);

			setCallback(3);
			setup_updateFromTime(45);
			break;

		case 3:
			getSavePoints()->push(kEntityCoudert, kEntityRebecca, kAction254915200);

			setCallback(4);
			setup_updateFromTime(450);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment2("627Re", kObjectCompartmentE, kPosition_4840, kPosition_4455);
			break;

		case 5:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentE, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(6);
			setup_function20(kObjectCompartmentE, kObject52);
			break;

		case 6:
			setCallback(7);
			setup_enterExitCompartment("627Se", kObjectCompartmentE);
			break;

		case 7:
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityCoudert, kEntityRebecca, kAction123852928);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Coudert, function26)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			if (!Entity::updateParameter(params->param2, getState()->timeTicks, 75))
				break;

			setCallback(3);
			setup_enterExitCompartment2("627Zd", kObjectCompartmentD, kPosition_5790, kPosition_6130);
		}
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_5790);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("627Vd", kObjectCompartmentD);
			break;

		case 2:
			getSavePoints()->push(kEntityCoudert, kEntityMmeBoutarel, kAction221683008);
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Wd");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentD, true);
			break;

		case 3:
		case 7:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentD, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(getCallback() + 1);
			setup_function20(kObjectCompartmentD, kObject51);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("697Ad", kObjectCompartmentD);
			break;

		case 5:
			getData()->location = kLocationOutsideCompartment;

			callbackAction();
			break;

		case 6:
			getSavePoints()->push(kEntityCoudert, kEntityMmeBoutarel, kAction122865568);
			break;

		case 8:
			getSound()->playSound(kEntityCoudert, "JAC1013");

			setCallback(9);
			setup_enterExitCompartment("697Ad", kObjectCompartmentD);
			break;

		case 9:
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityCoudert, kEntityMmeBoutarel, kAction123852928);

			callbackAction();
			break;
		}
		break;

	case kAction88652208:
		setCallback(7);
		setup_enterExitCompartment2("627Zd", kObjectCompartmentD, kPosition_5790, kPosition_6130);
		break;

	case kAction123199584:
		params->param1 = 1;

		setCallback(6);
		setup_playSound("JAC1012");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Coudert, function27)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param1) {
			if (!Entity::updateParameter(params->param2, getState()->timeTicks, 75))
				break;

			setCallback(3);
			setup_enterExitCompartment2("627Rc", kObjectCompartmentC, kPosition_6470, kPosition_6130);
		}
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_6470);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("627Mc", kObjectCompartmentC);
			break;

		case 2:
			getSavePoints()->push(kEntityCoudert, kEntityBoutarel, kAction221683008);
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Nc");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentC, true);
			break;

		case 3:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentC, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(4);
			setup_function20(kObjectCompartmentC, kObject50);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("627Sc", kObjectCompartmentC);
			break;

		case 5:
			getData()->location = kLocationOutsideCompartment;
			callbackAction();
			break;

		case 6:
			getSavePoints()->push(kEntityCoudert, kEntityBoutarel, kAction122865568);
			break;

		case 7:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentC, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(8);
			setup_function20(kObjectCompartmentC, kObject50);
			break;

		case 8:
			getSound()->playSound(kEntityCoudert, "JAC1013");

			setCallback(9);
			setup_enterExitCompartment("627Uc", kObjectCompartmentC);
			break;

		case 9:
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kEntityCoudert, kEntityBoutarel, kAction123852928);

			callbackAction();
			break;
		}
		break;

	case kAction88652208:
		setCallback(7);
		setup_enterExitCompartment2("627Rc", kObjectCompartmentC, kPosition_6470, kPosition_6130);
		break;

	case kAction123199584:
		params->param1 = 1;

		setCallback(6);
		setup_playSound("JAC1012");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Coudert, visitCompartmentB)
	visitCompartment(savepoint, kPosition_7500, "627Vb", kObjectCompartmentB, "627Wb", "627Zb", kPosition_7850, kObject49, "697Ab");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Coudert, visitCompartmentA)
	visitCompartment(savepoint, kPosition_8200, "627Ma", kObjectCompartmentA, "627Na", "627Ra", kPosition_7850, kObject48, "627Sa");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(30, Coudert, function30, ObjectIndex)
	// Expose parameters as IIIIIS and ignore the default exposed parameters
	EntityData::EntityParametersI5S  *parameters  = (EntityData::EntityParametersI5S*)_data->getCurrentParameters();
	EntityData::EntityParametersSIIS *parameters1 = (EntityData::EntityParametersSIIS*)_data->getCurrentParameters(1);

	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		switch (parameters->param1) {
		default:
			callbackAction();
			// Stop processing here
			return;

		case kObjectCompartmentA:
			parameters->param2 = kPosition_8200;
			parameters->param3 = kPosition_7850;
			strcpy((char *)&parameters->seq, "627Ma");
			strcpy((char *)&parameters1->seq1, "627Na");
			break;

		case kObjectCompartmentB:
			parameters->param2 = kPosition_7500;
			parameters->param3 = kPosition_7850;
			parameters->param4 = true;
			strcpy((char *)&parameters->seq, "627Vb");
			strcpy((char *)&parameters1->seq1, "627Wb");
			break;

		case kObjectCompartmentC:
			parameters->param2 = kPosition_6470;
			parameters->param3 = kPosition_6130;
			strcpy((char *)&parameters->seq, "627Mc");
			strcpy((char *)&parameters1->seq1, "627Nc");
			break;

		case kObjectCompartmentD:
			parameters->param2 = kPosition_5790;
			parameters->param3 = kPosition_6130;
			parameters->param4 = true;
			strcpy((char *)&parameters->seq, "627Vd");
			strcpy((char *)&parameters1->seq1, "627Wd");
			break;

		case kObjectCompartmentE:
			parameters->param2 = kPosition_4840;
			parameters->param3 = kPosition_4455;
			parameters->param4 = true;
			strcpy((char *)&parameters->seq, "627Me");
			strcpy((char *)&parameters1->seq1, "627Ne");
			break;

		case kObjectCompartmentF:
			parameters->param2 = kPosition_4070;
			parameters->param3 = kPosition_4455;
			parameters->param4 = true;
			strcpy((char *)&parameters->seq, "627Vf");
			strcpy((char *)&parameters1->seq1, "627Wf");
			break;
		}

		setCallback(1);
		setup_function16();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, (EntityPosition)parameters->param2);
			break;

		case 2:
			if (getEntities()->checkFields19(kEntityPlayer, kCarRedSleeping, (EntityPosition)parameters->param3)
			 || ((parameters->param1 == kObjectCompartmentE || parameters->param1 == kObjectCompartmentF) && getEntities()->isOutsideAnnaWindow())) {
				getObjects()->update((ObjectIndex)parameters->param1, kEntityPlayer, getObjects()->get((ObjectIndex)parameters->param1).location, kCursorNormal, kCursorNormal);
				parameters->param5 = true;
			}

			setCallback(3);
			setup_enterExitCompartment((char *)&parameters->seq, (ObjectIndex)parameters->param1);
			break;

		case 3:
			getEntities()->drawSequenceLeft(kEntityCoudert, (char *)&parameters1->seq1);
			getEntities()->enterCompartment(kEntityCoudert, (ObjectIndex)parameters->param1, true);

			setCallback(4);
			setup_playSound(parameters->param4 ? "JAC3020" : "JAC3021");
			break;

		case 4:
			if (parameters->param5)
				getObjects()->update((ObjectIndex)parameters->param1, kEntityPlayer, getObjects()->get((ObjectIndex)parameters->param1).location, kCursorHandKnock, kCursorHand);

			getEntities()->exitCompartment(kEntityCoudert, (ObjectIndex)parameters->param1, true);

			setCallback(5);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 5:
			setCallback(6);
			setup_function18();
			break;

		case 6:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(31, Coudert, function31, uint32)
	switch (savepoint.action) {
	default:
		break;

	case kActionEndSound:
		setCallback(3);
		setup_function19(true);
		break;

	case kActionDefault:
		setCallback(1);
		setup_bloodJacket("627G");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getSoundQueue()->isBuffered(kEntityCoudert)) {
				getEntities()->drawSequenceLeft(kEntityCoudert, "627K");
			} else {
				setCallback(2);
				setup_function19(true);
			}
			break;

		case 2:
		case 3:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Coudert, function32)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function16();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_9460);
			break;

		case 2:
			getEntities()->clearSequences(kEntityCoudert);
			setCallback(3);
			setup_updateFromTime(900);
			break;

		case 3:
			setCallback(4);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 4:
			setCallback(5);
			setup_function18();
			break;

		case 5:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Coudert, function33)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (ENTITY_PARAM(0, 3) || ENTITY_PARAM(0, 4) || ENTITY_PARAM(0, 5) || ENTITY_PARAM(0, 6) || ENTITY_PARAM(0, 7)
		 || ENTITY_PARAM(1, 2) || ENTITY_PARAM(1, 7)
		 || ENTITY_PARAM(2, 2)) {
			ENTITY_PARAM(2, 6) = 1;

			if (ENTITY_PARAM(0, 3) || ENTITY_PARAM(0, 4) || ENTITY_PARAM(0, 5)) {
				setCallback(1);
				setup_updateEntity(kCarRedSleeping, kPosition_1500);
			} else {
				setCallback(5);
				setup_updateEntity(kCarRedSleeping, kPosition_540);
			}
		} else {
			callbackAction();
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			ENTITY_PARAM(2, 1) = 1;
			if (ENTITY_PARAM(0, 3)) {
				setCallback(2);
				setup_function14(kEntityVerges);
				break;
			}
			// Fallback to next case

		case 2:
			if (ENTITY_PARAM(0, 5)) {
				setCallback(3);
				setup_function14(kEntityMertens);
				break;
			}
			// Fallback to next case

		case 3:
			if (ENTITY_PARAM(0, 4)) {
				setCallback(4);
				setup_function14(kEntityMmeBoutarel);
				break;
			}
			// Fallback to next case

		case 4:
			ENTITY_PARAM(2, 6) = 0;

			callbackAction();
			break;

		case 5:
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(6);
			setup_updateFromTime(75);
			break;

		case 6:
			if (ENTITY_PARAM(0, 6) || ENTITY_PARAM(0, 7)) {
				setCallback(7);
				setup_function37();
				break;
			}
			// Fallback to next case

		case 7:
			if (ENTITY_PARAM(2, 2)) {
				setCallback(8);
				setup_function39();
				break;
			}
			// Fallback to next case

		case 8:
			if (ENTITY_PARAM(1, 2)) {
				setCallback(9);
				setup_function55();
				break;
			}
			// Fallback to next case

		case 9:
			if (ENTITY_PARAM(1, 7)) {
				setCallback(10);
				setup_function34(false);
				break;
			}
			// Fallback to next case

		case 10:
			ENTITY_PARAM(2, 6) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(34, Coudert, function34, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function16();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_4070);
			break;

		case 2:
			if (!params->param1) {
				getSound()->playSound(kEntityCoudert, "Ann3124");

				ENTITY_PARAM(1, 7) = 0;
				ENTITY_PARAM(1, 4) = 0;

				setCallback(7);
				setup_function35((bool)params->param1);
			} else {
				getEntities()->drawSequenceLeft(kEntityCoudert, "627Vf");
				getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentF, true);

				setCallback(3);
				setup_playSound("LIB012");
			}
			break;

		case 3:
			setCallback(4);
			setup_playSound("Jac1001");
			break;

		case 4:
			getSound()->playSound(kEntityCoudert, "Ann3125");

			setCallback(5);
			setup_enterExitCompartment("629Bf", kObjectCompartmentF);
			break;

		case 5:
			setCallback(6);
			setup_enterExitCompartment("629Ff", kObjectCompartmentF);
			break;

		case 6:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentF, true);

			ENTITY_PARAM(1, 7) = 0;
			ENTITY_PARAM(1, 4) = 0;

			setCallback(7);
			setup_function35((bool)params->param1);
			break;

		case 7:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(35, Coudert, function35, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarBaggage)) {
			getAction()->playAnimation(kEventCoudertBaggageCar);
			getSound()->playSound(kEntityPlayer, "BUMP");
			getScenes()->loadSceneFromPosition(kCarRestaurant, 65);
		}

		if (!Entity::updateParameter(params->param2, getState()->time, 2700))
			break;

		getSavePoints()->push(kEntityCoudert, kEntityMax, kActionMaxFreeFromCage);

		getData()->clothes = kClothesDefault;

		setCallback(3);
		setup_updateEntity(kCarRedSleeping, kPosition_2000);
		break;

	case kActionDefault:
		if (params->param1)
			getSavePoints()->push(kEntityCoudert, kEntityAnna, kAction156049968);

		getSavePoints()->push(kEntityCoudert, kEntityMax, kAction122358304);

		getData()->clothes = kClothes1;
		getData()->entityPosition = kPosition_4370;

		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_8200);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (!getSoundQueue()->isBuffered(kEntityCoudert))
				getSound()->playSound(kEntityCoudert, "Ann3124");

			if (params->param1)
				getSavePoints()->push(kEntityCoudert, kEntityAnna, kAction123733488);

			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_9460);
			break;

		case 2:
			getEntities()->clearSequences(kEntityCoudert);
			break;

		case 3:
			setCallback(4);
			setup_function18();
			break;

		case 4:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(36, Coudert, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheckCallback(kTimeChapter1, params->param1, 1, WRAP_SETUP_FUNCTION(Coudert, setup_chapter1Handler));
		break;

	case kActionDefault:
		getSavePoints()->addData(kEntityCoudert, kAction292048641, 7);
		getSavePoints()->addData(kEntityCoudert, kAction326348944, 8);
		getSavePoints()->addData(kEntityCoudert, kAction171394341, 2);
		getSavePoints()->addData(kEntityCoudert, kAction154005632, 4);
		getSavePoints()->addData(kEntityCoudert, kAction169557824, 3);
		getSavePoints()->addData(kEntityCoudert, kAction226031488, 5);
		getSavePoints()->addData(kEntityCoudert, kAction339669520, 6);
		getSavePoints()->addData(kEntityCoudert, kAction189750912, 10);
		getSavePoints()->addData(kEntityCoudert, kAction185737168, 12);
		getSavePoints()->addData(kEntityCoudert, kAction185671840, 13);
		getSavePoints()->addData(kEntityCoudert, kAction205033696, 15);
		getSavePoints()->addData(kEntityCoudert, kAction157026693, 14);
		getSavePoints()->addData(kEntityCoudert, kAction189026624, 11);
		getSavePoints()->addData(kEntityCoudert, kAction168254872, 17);
		getSavePoints()->addData(kEntityCoudert, kAction201431954, 18);
		getSavePoints()->addData(kEntityCoudert, kAction188570113, 19);

		ENTITY_PARAM(0, 1) = 0;
		ENTITY_PARAM(0, 2) = 1;

		getData()->entityPosition = kPosition_1500;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->updateLocation2(kObject111, kObjectLocation1);
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_chapter1Handler();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(37, Coudert, function37)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (getSoundQueue()->isBuffered(kEntityCoudert))
			getSoundQueue()->processEntry(kEntityCoudert);

		if (ENTITY_PARAM(0, 7)) {
			getData()->entityPosition = kPosition_8200;

			setCallback(4);
			setup_enterExitCompartment2("698Ha", kObjectCompartmentA, kPosition_8200, kPosition_7850);
		} else {
			setCallback(1);
			setup_function16();
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_5790);
			break;

		case 2:
			getSavePoints()->push(kEntityCoudert, kEntityAnna, kAction238358920);
			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_8200);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment2("698Ha", kObjectCompartmentA, kPosition_8200, kPosition_7850);
			break;

		case 4:
			getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);
			setup_function38();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(38, Coudert, function38)
switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			ENTITY_PARAM(0, 6) = 0;
			ENTITY_PARAM(0, 7) = 0;

			setCallback(2);
			setup_function18();
			break;

		case 2:
			setup_chapter1Handler();
			break;
		}
		break;

	case kAction191477936:
		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationOutsideCompartment;
		getObjects()->update(kObjectCompartment4, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_2000);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(39, Coudert, function39)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_playSound("LIB070");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function16();
			break;

		case 2:
			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_5790);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment("627Vd", kObjectCompartmentD);
			break;

		case 4:
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Wd");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentD, true);

			setCallback(5);
			setup_playSound("MME1151A");
			break;

		case 5:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentD, true);

			setCallback(6);
			setup_enterExitCompartment("627Zd", kObjectCompartmentD);
			break;

		case 6:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(7);
			setup_playSound("MME1151");
			break;

		case 7:
			setCallback(8);
			setup_enterExitCompartment("MME1151", kObjectCompartmentD);
			break;

		case 8:
			getSavePoints()->push(kEntityCoudert, kEntityMmeBoutarel, kAction223068211);
			getData()->location = kLocationOutsideCompartment;

			setCallback(9);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 9:
			setCallback(10);
			setup_function18();
			break;

		case 10:
			getSavePoints()->push(kEntityCoudert, kEntityVerges, kAction167854368);
			ENTITY_PARAM(2, 2) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(40, Coudert, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(2, 3)) {
			ENTITY_PARAM(0, 1) = 1;
			ENTITY_PARAM(0, 3) = 0;
			ENTITY_PARAM(0, 4) = 0;
			ENTITY_PARAM(0, 5) = 0;
			ENTITY_PARAM(0, 8) = 0;

			ENTITY_PARAM(1, 1) = 0;

			ENTITY_PARAM(2, 1) = 0;
			ENTITY_PARAM(2, 2) = 0;

			getEntities()->drawSequenceLeft(kEntityCoudert, "697F");

			params->param1 = 1;
			params->param2 = 1;

			ENTITY_PARAM(2, 3) = 0;
		}

		getData()->inventoryItem = (getProgress().eventCorpseFound || getEvent(kEventCoudertAskTylerCompartment)) ? kItemNone : kItemInvalid;

		if (ENTITY_PARAM(0, 8)) {
			getData()->inventoryItem = kItemNone;

			setCallback(4);
			setup_function15(true);
			break;
		}

label_callback_4:
		if (ENTITY_PARAM(1, 1)) {
			getData()->inventoryItem = kItemNone;

			setCallback(5);
			setup_function15(false);
			break;
		}

label_callback_5:
		if (ENTITY_PARAM(0, 6) || ENTITY_PARAM(0, 7)) {
			getData()->inventoryItem = kItemNone;
			setup_function37();
			break;
		}

		if (ENTITY_PARAM(0, 3)) {
			getData()->inventoryItem = kItemNone;

			setCallback(6);
			setup_function14(kEntityVerges);
			break;
		}

label_callback_6:
		if (ENTITY_PARAM(0, 5)) {
			getData()->inventoryItem = kItemNone;

			setCallback(7);
			setup_function14(kEntityMertens);
			break;
		}

label_callback_7:
		if (ENTITY_PARAM(0, 4)) {
			getData()->inventoryItem = kItemNone;

			setCallback(8);
			setup_function14(kEntityMmeBoutarel);
			break;
		}

label_callback_8:
		if (ENTITY_PARAM(2, 2)) {
			getData()->inventoryItem = kItemNone;

			setCallback(9);
			setup_function39();
			break;
		}

label_callback_9:
		if (ENTITY_PARAM(0, 1) && !getSoundQueue()->isBuffered(kEntityCoudert))
			getSound()->playSound(kEntityCoudert, rnd(2) ? "JAC1065" : "JAC1065A");

		if (getState()->time > kTime1107000 && !ENTITY_PARAM(0, 1) && !getEvent(kEventVassiliSeizure)) {
			getData()->inventoryItem = kItemNone;

			setCallback(10);
			setup_function41();
			break;
		}

label_callback_10:
		if (getState()->time > kTime1189800 && !ENTITY_PARAM(0, 1) && !ENTITY_PARAM(2, 1)) {
			if (Entity::updateParameter(params->param3, getState()->time, 2700)) {
				ENTITY_PARAM(0, 2) = 1;
				ENTITY_PARAM(0, 1) = 1;

				getEntities()->drawSequenceLeft(kEntityCoudert, "697F");

				params->param3 = 0;
			}
		}

		if (!ENTITY_PARAM(0, 2))
			break;

		timeCheckObject(kTime1107000, params->param4, kObject111, kObjectLocation2);
		timeCheckObject(kTime1161000, params->param5, kObject111, kObjectLocation3);
		timeCheckObject(kTime1206000, params->param6, kObject111, kObjectLocation4);
		break;

	case kAction1:
		getData()->inventoryItem = kItemNone;

		setCallback(11);
		setup_savegame(kSavegameTypeEvent, kEventCoudertAskTylerCompartment);
		break;

	case kAction11:
		if (!ENTITY_PARAM(0, 1) && !ENTITY_PARAM(2, 1)) {
			getData()->inventoryItem = kItemNone;

			setCallback(13);
			setup_function13((bool)savepoint.param.intValue, savepoint.entity2);
		}
		break;

	case kActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_1500;
		getData()->location = kLocationOutsideCompartment;

		getScenes()->loadSceneFromItemPosition(kItem5);
		break;

	case kActionDrawScene:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {

			if (!getEntities()->isPlayerPosition(kCarRedSleeping, 1) && !getEntities()->isPlayerPosition(kCarRedSleeping, 23))
				break;

			if (getProgress().jacket == kJacketBlood) {
				setCallback(1);
				setup_savegame(kSavegameTypeEvent, kEventCoudertBloodJacket);
			} else {
				setCallback(getEntities()->isPlayerPosition(kCarRedSleeping, 1) ? 2 : 3);
				setup_function13(true, kEntityPlayer);
			}
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			break;

		case 4:
			goto label_callback_4;

		case 5:
			goto label_callback_5;

		case 6:
			goto label_callback_6;

		case 7:
			goto label_callback_7;

		case 8:
			goto label_callback_8;

		case 9:
			goto label_callback_9;

		case 10:
			params->param1 = 1;
			goto label_callback_10;

		case 11:
			getAction()->playAnimation(kEventCoudertAskTylerCompartment);
			getEntities()->drawSequenceRight(kEntityCoudert, ENTITY_PARAM(0, 2) ? "627A" : "627D");
			getScenes()->loadSceneFromItemPosition(kItem5);

			ENTITY_PARAM(0, 1) = 0;

			getScenes()->loadSceneFromPosition(kCarRedSleeping, 25);

			setCallback(12);
			setup_callbackActionOnDirection();
			break;

		case 12:
			getEntities()->drawSequenceLeft(kEntityCoudert, ENTITY_PARAM(0, 2) ? "627B" : "627E");
			break;

		case 14:
			setCallback(15);
			setup_function18();
			break;
		}
		break;

	case kAction168253822:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			getData()->inventoryItem = kItemNone;
			getSound()->playSound(kEntityCoudert, "JAC1120");

			setCallback(14);
			setup_bloodJacket("697D");
		}
		break;

	case kAction225358684:
		if (!ENTITY_PARAM(0, 1)) {
			getData()->inventoryItem = kItemNone;
			setCallback(16);
			setup_function30((ObjectIndex)savepoint.param.intValue);
		}
		break;

	case kAction225932896:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1))
			getSavePoints()->push(kEntityCoudert, kEntityFrancois, kAction205346192);
		break;

	case kAction305159806:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			getData()->inventoryItem = kItemNone;
			setCallback(17);
			setup_function31(savepoint.param.intValue);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(41, Coudert, function41)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function16();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_visitCompartmentA();
			break;

		case 2:
			setCallback(3);
			setup_function33();
			break;

		case 3:
			setCallback(4);
			setup_visitCompartmentB();
			break;

		case 4:
			setCallback(5);
			setup_function33();
			break;

		case 5:
			setCallback(6);
			setup_function27();
			break;

		case 6:
			getSavePoints()->push(kEntityCoudert, kEntityRebecca, kAction285528346);

			setCallback(7);
			setup_function33();
			break;

		case 7:
			setCallback(8);
			setup_function26();
			break;

		case 8:
			setCallback(9);
			setup_function33();
			break;

		case 9:
			setCallback(10);
			setup_function25();
			break;

		case 10:
			setCallback(11);
			setup_function33();
			break;

		case 11:
			setCallback(12);
			setup_function23();
			break;

		case 12:
			setCallback(13);
			setup_function33();
			break;

		case 13:
			setCallback(14);
			setup_function22();
			break;

		case 14:
			setCallback(15);
			setup_function33();
			break;

		case 15:
			setCallback(16);
			setup_function21();
			break;

		case 16:
			setCallback(17);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 17:
			setCallback(18);
			setup_function18();
			break;

		case 18:
			getSavePoints()->push(kEntityCoudert, kEntityMilos, kAction208228224);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(42, Coudert, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setCallback(1);
		setup_function18();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityCoudert);

		getData()->entityPosition = kPosition_1500;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 2) = 0;
		ENTITY_PARAM(0, 3) = 0;
		ENTITY_PARAM(0, 4) = 0;
		ENTITY_PARAM(0, 5) = 0;
		ENTITY_PARAM(0, 6) = 0;
		ENTITY_PARAM(0, 8) = 0;

		ENTITY_PARAM(1, 1) = 0;
		ENTITY_PARAM(1, 2) = 0;
		ENTITY_PARAM(1, 3) = 0;
		ENTITY_PARAM(1, 5) = 0;
		ENTITY_PARAM(1, 6) = 0;
		ENTITY_PARAM(1, 7) = 0;
		ENTITY_PARAM(1, 8) = 0;

		ENTITY_PARAM(2, 4) = 0;

		getObjects()->updateLocation2(kObject111, kObjectLocation5);
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function43();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(43, Coudert, function43)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(0, 8)) {
			setCallback(1);
			setup_function15(true);
			break;
		}

label_callback1:
		if (!ENTITY_PARAM(1, 1)) {
			setCallback(2);
			setup_function15(false);
			break;
		}

label_callback2:
		if (ENTITY_PARAM(0, 3)) {
			setCallback(3);
			setup_function14(kEntityVerges);
		}
		break;

	case kAction11:
		if (!ENTITY_PARAM(2, 1)) {
			setCallback(4);
			setup_function13((bool)savepoint.param.intValue, savepoint.entity2);
		}
		break;

	case kActionDrawScene:
		if (ENTITY_PARAM(2, 1))
			break;

		if (getEntities()->isPlayerPosition(kCarRedSleeping, 1)) {
			setCallback(5);
			setup_function13(true, kEntityPlayer);

		} else if (getEntities()->isPlayerPosition(kCarRedSleeping, 23)) {
			setCallback(6);
			setup_function13(false, kEntityPlayer);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback1;

		case 2:
			goto label_callback2;

		case 7:
			setCallback(8);
			setup_function18();
			break;
		}
		break;

	case kAction225358684:
		if (!ENTITY_PARAM(0, 1)) {
			setCallback(9);
			setup_function30((ObjectIndex)savepoint.param.intValue);
		}
		break;

	case kAction226078300:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			getSound()->playSound(kEntityCoudert, "JAC2020");

			setCallback(7);
			setup_bloodJacket("697D");
		}
		break;

	case kAction305159806:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			setCallback(10);
			setup_function31(savepoint.param.intValue);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(44, Coudert, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setCallback(1);
		setup_function18();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityCoudert);

		getData()->entityPosition = kPosition_1500;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 2) = 0;
		ENTITY_PARAM(0, 3) = 0;
		ENTITY_PARAM(0, 4) = 0;
		ENTITY_PARAM(0, 5) = 0;
		ENTITY_PARAM(0, 8) = 0;

		ENTITY_PARAM(1, 1) = 0;
		ENTITY_PARAM(1, 2) = 0;
		ENTITY_PARAM(1, 3) = 0;
		ENTITY_PARAM(1, 4) = 0;
		ENTITY_PARAM(1, 5) = 0;
		ENTITY_PARAM(1, 6) = 0;
		ENTITY_PARAM(1, 7) = 0;
		ENTITY_PARAM(1, 8) = 0;

		ENTITY_PARAM(2, 4) = 0;
		ENTITY_PARAM(2, 5) = 0;

		getObjects()->updateLocation2(kObject111, kObjectLocation6);
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function45();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(45, Coudert, function45)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(0, 8)) {
			setCallback(1);
			setup_function15(true);
			break;
		}

label_callback_1:
		if (ENTITY_PARAM(1, 1)) {
			setCallback(2);
			setup_function15(false);
			break;
		}

label_callback_2:
		if (ENTITY_PARAM(0, 3)) {
			setCallback(3);
			setup_function14(kEntityVerges);
			break;
		}

label_callback_3:
		if (ENTITY_PARAM(0, 5)) {
			setCallback(4);
			setup_function14(kEntityMertens);
			break;
		}

label_callback_4:
		if (ENTITY_PARAM(1, 3)) {
			setCallback(5);
			setup_function46();
			break;
		}

label_callback_5:
		if (ENTITY_PARAM(1, 5)) {
			setCallback(6);
			setup_function47(true);
			break;
		}

label_callback_6:
		if (ENTITY_PARAM(1, 6)) {
			setCallback(7);
			setup_function47(false);
			break;
		}

label_callback_7:
		if (ENTITY_PARAM(1, 8)) {
			setCallback(8);
			setup_function48();
			break;
		}

label_callback_8:
		if (ENTITY_PARAM(2, 4)) {
			setCallback(9);
			setup_function49();
			break;
		}

label_callback_9:
		if (ENTITY_PARAM(1, 4)) {
			setCallback(10);
			setup_function34(true);
			break;
		}

label_callback_10:
		if (ENTITY_PARAM(1, 7)) {
			setCallback(11);
			setup_function34(false);
			break;
		}

label_callback_11:
		if (ENTITY_PARAM(0, 4)) {
			setCallback(12);
			setup_function14(kEntityMmeBoutarel);
			break;
		}

label_callback_12:
		// BUG: Can never be called... FAIL!
		if (ENTITY_PARAM(2, 5) && getState()->time > kTime2056500 && getState()->time < kTime1417500) {
			setCallback(13);
			setup_function50();
			break;
		}

label_callback_13:
		if (Entity::timeCheckCallback(kTime2088900, params->param1, 14, WRAP_SETUP_FUNCTION(Coudert, setup_function32)))
			break;

label_callback_14:
		if (Entity::timeCheckCallback(kTime2119500, params->param2, 15, WRAP_SETUP_FUNCTION(Coudert, setup_function32)))
			break;

label_callback_15:
		if (Entity::timeCheckCallback(kTime2138400, params->param3, 16, WRAP_SETUP_FUNCTION(Coudert, setup_function32)))
			break;

label_callback_16:
		if (Entity::timeCheckCallback(kTime2147400, params->param4, 17, WRAP_SETUP_FUNCTION(Coudert, setup_function32)))
			break;

label_callback_17:
		if (Entity::timeCheckCallback(kTime2160000, params->param5, 18, WRAP_SETUP_FUNCTION(Coudert, setup_function32)))
			break;

label_callback_18:
		if (Entity::timeCheckCallback(kTime2205000, params->param6, 19, WRAP_SETUP_FUNCTION(Coudert, setup_function32)))
			break;

label_callback_19:
		if (ENTITY_PARAM(0, 2)) {
			timeCheckObject(kTime2025000, params->param7, kObject111, kObjectLocation7);
			timeCheckObject(kTime2133000, params->param8, kObject111, kObjectLocation8);
			timeCheckObject(kTime2173500, CURRENT_PARAM(1, 1), kObject111, kObjectLocation9);
		}
		break;

	case kAction11:
		if (!ENTITY_PARAM(2, 1)) {
			setCallback(20);
			setup_function13((bool)savepoint.param.intValue, savepoint.entity2);
		}
		break;

	case kActionDrawScene:
		if (!ENTITY_PARAM(2, 1)) {
			if (getEntities()->isPlayerPosition(kCarRedSleeping, 1)) {
				setCallback(21);
				setup_function13(true, kEntityPlayer);
			} else if (getEntities()->isPlayerPosition(kCarRedSleeping, 23)) {
				setCallback(22);
				setup_function13(false, kEntityPlayer);
			}
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback_1;

		case 2:
			goto label_callback_2;

		case 3:
			goto label_callback_3;

		case 4:
			goto label_callback_4;

		case 5:
			goto label_callback_5;

		case 6:
			goto label_callback_6;

		case 7:
			goto label_callback_7;

		case 8:
			goto label_callback_8;

		case 9:
			goto label_callback_9;

		case 10:
			goto label_callback_10;

		case 11:
			goto label_callback_11;

		case 12:
			getSavePoints()->push(kEntityCoudert, kEntityVerges, kAction168255788);
			goto label_callback_12;

		case 13:
			goto label_callback_13;

		case 14:
			goto label_callback_14;

		case 15:
			goto label_callback_15;

		case 16:
			goto label_callback_16;

		case 17:
			goto label_callback_17;

		case 18:
			goto label_callback_18;

		case 19:
			goto label_callback_19;

		case 23:
			setCallback(24);
			setup_function18();
			break;
		}
		break;

	case kAction225358684:
		if (!ENTITY_PARAM(0, 1)) {
			setCallback(25);
			setup_function30((ObjectIndex)savepoint.param.intValue);
		}
		break;

	case kAction226078300:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			getSound()->playSound(kEntityCoudert, "JAC2020");

			setCallback(23);
			setup_bloodJacket("697D");
		}
		break;

	case kAction305159806:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			setCallback(26);
			setup_function31(savepoint.param.intValue);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(46, Coudert, function46)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function16();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_4070);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Vf");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentF, true);
			getSavePoints()->push(kEntityCoudert, kEntityAnna, kAction253868128);

			setCallback(3);
			setup_playSound("LIB012");
			break;

		case 3:
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Wf");

			setCallback(4);
			setup_playSound("Ann1016A");
			break;

		case 4:
			setCallback(5);
			setup_playSound("Ann4150");
			break;

		case 5:
			getSound()->playSound(kEntityCoudert, "Ann3121");

			setCallback(6);
			setup_enterExitCompartment("629Bf", kObjectCompartmentF);
			break;

		case 6:
			getEntities()->drawSequenceLeft(kEntityCoudert, "629Cf");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentF, true);
			// Fallback to next case

		case 7:
			if (getSoundQueue()->isBuffered(kEntityCoudert)) {
				setCallback(7);
				setup_updateFromTime(75);
			} else {
				setCallback(8);
				setup_playSound("Ann3122");
			}
			break;

		case 8:
			getSound()->playSound(kEntityCoudert, "Ann3123");

			setCallback(9);
			setup_updateFromTicks(75);
			break;

		case 9:
			setCallback(10);
			setup_enterExitCompartment("629Ff", kObjectCompartmentF);
			break;

		case 10:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentF, true);
			ENTITY_PARAM(1, 3) = 0;

			setCallback(11);
			setup_function35(true);
			break;

		case 11:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(47, Coudert, function47, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function16();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_4070);
			break;

		case 2:
			setCallback(3);
			setup_enterExitCompartment("627Xf", kObjectCompartmentF);
			break;

		case 3:
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Wf");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentF);
			// Fallback to next case

		case 4:
			if (getSoundQueue()->isBuffered(kEntityCoudert)) {
				setCallback(4);
				setup_updateFromTime(225);
			} else {
				setCallback(5);
				setup_playSound(params->param1 ? "Ann3149" : "Ann3147a");
			}
			break;

		case 5:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentF, true);
			getSavePoints()->push(kEntityCoudert, kEntityAnna, kAction157894320);

			setCallback(6);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 6:
			ENTITY_PARAM(1, 5) = 0;
			ENTITY_PARAM(1, 6) = 0;

			setCallback(7);
			setup_function18();
			break;

		case 7:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(48, Coudert, function48)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function16();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSound()->playSound(kEntityCoudert, "Ann3148A");
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_4070);
			break;

		case 2:
			getSound()->playSound(kEntityCoudert, rnd(2) ? "Ann3148B" : "Ann3148");
			setCallback(3);
			setup_enterExitCompartment("627Xf", kObjectCompartmentF);
			break;

		case 3:
			getSavePoints()->push(kEntityCoudert, kEntityAnna, kAction192063264);
			setCallback(4);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 4:
			ENTITY_PARAM(1, 8) = 0;
			setCallback(5);
			setup_function18();
			break;

		case 5:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(49, Coudert, function49)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function16();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_7500);
			break;

		case 2:
			setCallback(3);
			setup_enterExitCompartment("627Vb", kObjectCompartmentB);
			break;

		case 3:
			if (getEntities()->isInsideCompartment(kEntityTatiana, kCarRedSleeping, kPosition_7500)) {
				getEntities()->drawSequenceLeft(kEntityCoudert, "627Wb");

				setCallback(4);
				setup_playSound("Jac3006");
			} else {
				getEntities()->drawSequenceLeft(kEntityCoudert, "627Wb");
				getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentB, true);

				setCallback(8);
				setup_playSound("LIB012");
			}
			break;

		case 4:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentB, true);

			setCallback(5);
			setup_enterExitCompartment("627Zb", kObjectCompartmentB);
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(6);
			setup_playSound("Jac3006A");
			break;

		case 6:
			setCallback(7);
			setup_enterExitCompartment("697Ab", kObjectCompartmentB);
			break;

		case 7:
			getData()->location = kLocationOutsideCompartment;

			setCallback(10);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 8:
			setCallback(9);
			setup_updateFromTime(150);
			break;

		case 9:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentB, true);

			setCallback(10);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 10:
			getSavePoints()->push(kEntityCoudert, kEntityMmeBoutarel, kAction242526416);
			ENTITY_PARAM(2, 4) = 0;
			ENTITY_PARAM(2, 5) = 1;

			setCallback(11);
			setup_function18();
			break;

		case 11:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(50, Coudert, function50)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function16();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_4840);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Me");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentE, true);

			setCallback(3);
			setup_playSound("LIB012");
			break;

		case 3:
			if (!getEntities()->isInsideCompartment(kEntityRebecca, kCarRedSleeping, kPosition_4840)) {
				getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentE, true);

				setCallback(8);
				setup_updateEntity(kCarRedSleeping, kPosition_2000);
			} else {
				getEntities()->drawSequenceLeft(kEntityCoudert, "627Ne");

				setCallback(4);
				setup_playSound("Jac3005");
			}
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("627Re", kObjectCompartmentE);
			break;

		case 5:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentE, true);

			getData()->location = kLocationInsideCompartment;

			getEntities()->clearSequences(kEntityCoudert);

			setCallback(6);
			setup_playSound("Jac3005A");
			break;

		case 6:
			setCallback(7);
			setup_enterExitCompartment("627Se", kObjectCompartmentE);
			break;

		case 7:
			getData()->location = kLocationOutsideCompartment;

			setCallback(8);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 8:
			ENTITY_PARAM(2, 5) = 0;

			setCallback(9);
			setup_function18();
			break;

		case 9:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(51, Coudert, function51)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime2133000 && !getProgress().field_40) {
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentB);
			getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

			setCallback(1);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
		}
		break;

	case kActionOpenDoor:
		if (savepoint.param.intValue == kObjectCompartmentB)
			getData()->entityPosition = kPosition_7500;

		getSound()->playSound(kEntityPlayer, "LIB014");
		getAction()->playAnimation(kEventCoudertGoingOutOfVassiliCompartment);
		getEntities()->updateEntity(kEntityCoudert, kCarRedSleeping, kPosition_2000);
		getScenes()->loadSceneFromObject(savepoint.param.intValue == kObjectCompartmentB ? kObjectCompartmentB : kObjectCompartmentA);
		getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentB, true);
		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		setCallback(3);
		setup_updateEntity(kCarRedSleeping, kPosition_2000);
		break;

	case kActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationOutsideCompartment;

		getSavePoints()->push(kEntityCoudert, kEntityMax, kActionMaxFreeFromCage);

		if (ENTITY_PARAM(0, 5)) {
			ENTITY_PARAM(0, 5) = 0;

			getSavePoints()->push(kEntityCoudert, kEntityMertens, kAction155853632);
			getSavePoints()->push(kEntityCoudert, kEntityMertens, kActionEndSound);
		}

		if (ENTITY_PARAM(0, 3)) {
			ENTITY_PARAM(0, 3) = 0;

			getSavePoints()->push(kEntityCoudert, kEntityVerges, kAction155853632);
			getSavePoints()->push(kEntityCoudert, kEntityVerges, kActionEndSound);
		}

		getEntities()->drawSequenceLeft(kEntityCoudert, "627Wb");
		getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentB, true);
		getSavePoints()->push(kEntityCoudert, kEntityTatiana, kAction154071333);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function18();
			break;

		case 2:
		case 4:
		case 6:
			setup_function45();
			break;

		case 3:
			setCallback(4);
			setup_function18();
			break;

		case 5:
			setCallback(5);
			setup_function18();
			break;
		}
		break;

	case kAction168316032:
		getObjects()->update(kObjectCompartmentA, kEntityCoudert, kObjectLocationNone, kCursorNormal, kCursorHand);
		getObjects()->update(kObjectCompartmentB, kEntityCoudert, kObjectLocation1, kCursorNormal, kCursorHand);
		break;

	case kAction235061888:
		getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentB, true);
		getObjects()->update(kObjectCompartmentA, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartmentB, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

		setCallback(5);
		setup_updateEntity(kCarRedSleeping, kPosition_2000);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(52, Coudert, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setCallback(1);
		setup_function18();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityCoudert);

		getData()->entityPosition = kPosition_1500;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 2) = 0;
		ENTITY_PARAM(0, 3) = 0;
		ENTITY_PARAM(0, 4) = 0;
		ENTITY_PARAM(0, 5) = 0;
		ENTITY_PARAM(0, 6) = 0;
		ENTITY_PARAM(0, 8) = 0;

		ENTITY_PARAM(1, 1) = 0;
		ENTITY_PARAM(1, 3) = 0;
		ENTITY_PARAM(1, 5) = 0;
		ENTITY_PARAM(1, 6) = 0;
		ENTITY_PARAM(1, 7) = 0;
		ENTITY_PARAM(1, 8) = 0;

		ENTITY_PARAM(2, 3) = 0;
		ENTITY_PARAM(2, 4) = 0;

		getObjects()->updateLocation2(kObject111, kObjectLocation10);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			ENTITY_PARAM(1, 2) = 1;
			setup_function53();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(53, Coudert, function53)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(2, 3)) {
			ENTITY_PARAM(1, 2) = 0;
			ENTITY_PARAM(1, 7) = 0;

			params->param1 = 1;

			getObjects()->updateLocation2(kObjectCompartmentA, kObjectLocation1);
			getObjects()->updateLocation2(kObjectCompartmentB, kObjectLocation1);
			getObjects()->updateLocation2(kObjectCompartmentC, kObjectLocation1);
			getObjects()->updateLocation2(kObjectCompartmentD, kObjectLocation1);
			getObjects()->updateLocation2(kObjectCompartmentE, kObjectLocation1);
			getObjects()->updateLocation2(kObjectCompartmentF, kObjectLocation1);
			getObjects()->updateLocation2(kObjectCompartmentG, kObjectLocation1);
			getObjects()->updateLocation2(kObjectCompartmentH, kObjectLocation1);

			ENTITY_PARAM(2, 3) = 0;

			setCallback(1);
			setup_function54();
			break;
		}

label_callback_1:
		if (ENTITY_PARAM(1, 2)) {
			if (!params->param2)
				params->param2 = (uint)(getState()->time + 4500);

			if (params->param3 != kTimeInvalid) {
				if (Entity::updateParameterTime((TimeValue)params->param2, !getEntities()->isPlayerInCar(kCarRedSleeping), params->param3, 0)) {
					setCallback(2);
					setup_function55();
					break;
				}
			}
		}

label_callback_2:
		if (ENTITY_PARAM(1, 7)) {
			setCallback(3);
			setup_function34(false);
			break;
		}

label_callback_3:
		if (!params->param1) {
			if (Entity::timeCheckCallback(kTime2394000, params->param4, 4, WRAP_SETUP_FUNCTION(Coudert, setup_function56)))
				break;

label_callback_4:
			if (Entity::timeCheckCallback(kTime2434500, params->param5, 5, WRAP_SETUP_FUNCTION(Coudert, setup_function32)))
				break;

label_callback_5:
			if (Entity::timeCheckCallback(kTime2448000, params->param6, 6, WRAP_SETUP_FUNCTION(Coudert, setup_function32)))
				break;
		}

label_callback_6:
		if (getState()->time > kTime2538000 && !ENTITY_PARAM(0, 1) && !ENTITY_PARAM(2, 1)) {
			if (!Entity::updateParameter(params->param7, getState()->time, 2700))
				break;

			ENTITY_PARAM(0, 2) = 0;
			ENTITY_PARAM(0, 1) = 1;

			getEntities()->drawSequenceLeft(kEntityCoudert, "697F");

			params->param7 = 0;
		}
		break;

	case kAction11:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			setCallback(7);
			setup_function13((bool)savepoint.param.intValue, savepoint.entity2);
		}
		break;

	case kActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_1500;
		getData()->location = kLocationOutsideCompartment;

		getScenes()->loadSceneFromItemPosition(kItem5);
		break;

	case kActionDrawScene:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			if (getEntities()->isPlayerPosition(kCarRedSleeping, 1)) {
				setCallback(8);
				setup_function13(true, kEntityPlayer);
			} else if (getEntities()->isPlayerPosition(kCarRedSleeping, 23)) {
				setCallback(9);
				setup_function13(false, kEntityPlayer);
			}
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback_1;

		case 2:
			goto label_callback_2;

		case 3:
			goto label_callback_3;

		case 4:
			goto label_callback_4;

		case 5:
			goto label_callback_5;

		case 6:
			goto label_callback_6;

		case 10:
			setCallback(11);
			setup_function18();
			break;
		}
		break;

	case kAction225358684:
		if (!ENTITY_PARAM(0, 1)) {
			setCallback(12);
			setup_function30((ObjectIndex)savepoint.param.intValue);
		}
		break;

	case kAction226078300:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			getSound()->playSound(kEntityCoudert, "JAC2020");

			setCallback(10);
			setup_bloodJacket("697D");
		}
		break;

	case kAction305159806:
		if (!ENTITY_PARAM(2, 1) && !ENTITY_PARAM(0, 1)) {
			setCallback(13);
			setup_function31(savepoint.param.intValue);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(54, Coudert, function54)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (getEntities()->hasValidFrame(kEntityCoudert)) {
			getData()->location = kLocationOutsideCompartment;

			setCallback(1);
			setup_updateEntity(kCarRedSleeping, kPosition_540);
		} else {
			getData()->car = kCarLocomotive;
			getData()->entityPosition = kPosition_540;
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->clearSequences(kEntityCoudert);
			getData()->car = kCarLocomotive;
			break;

		case 2:
			setCallback(3);
			setup_function18();
			break;

		case 3:
			callbackAction();
			break;
		}
		break;

	case kAction191001984:
		getData()->car = kCarRedSleeping;
		setCallback(2);
		setup_updateEntity(kCarRedSleeping, kPosition_1500);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(55, Coudert, function55)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_playSound("LIB070");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function16();
			break;

		case 2:
			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_4070);
			break;

		case 3:
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Wf");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentF, true);

			setCallback(4);
			setup_playSound("Ann4150A");
			break;

		case 4:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentF, true);
			getSavePoints()->push(kEntityCoudert, kEntityAnna, kAction219971920);
			getSavePoints()->push(kEntityCoudert, kEntityPascale, kAction101824388);

			setCallback(5);
			setup_updateEntity(kCarRedSleeping, kPosition_9460);
			break;

		case 5:
			getEntities()->clearSequences(kEntityCoudert);
			getSavePoints()->push(kEntityCoudert, kEntityPascale, kAction136059947);
			break;

		case 6:
			ENTITY_PARAM(1, 2) = 0;

			setCallback(7);
			setup_function18();
			break;

		case 7:
			callbackAction();
			break;
		}
		break;

	case kAction123712592:
		setCallback(6);
		setup_updateEntity(kCarRedSleeping, kPosition_2000);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(56, Coudert, function56)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function16();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function21();
			break;

		case 2:
			setCallback(3);
			setup_function33();
			break;

		case 3:
			setCallback(4);
			setup_function22();
			break;

		case 4:
			setCallback(5);
			setup_function33();
			break;

		case 5:
			setCallback(6);
			setup_visitCompartmentF();
			break;

		case 6:
			setCallback(7);
			setup_function33();
			break;

		case 7:
			setCallback(8);
			setup_function25();
			break;

		case 8:
			setCallback(9);
			setup_function33();
			break;

		case 9:
			setCallback(10);
			setup_function26();
			break;

		case 10:
			setCallback(11);
			setup_function33();
			break;

		case 11:
			setCallback(12);
			setup_function27();
			break;

		case 12:
			setCallback(13);
			setup_function33();
			break;

		case 13:
			setCallback(14);
			setup_visitCompartmentB();
			break;

		case 14:
			setCallback(15);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 15:
			setCallback(16);
			setup_function18();
			break;

		case 16:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(57, Coudert, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityCoudert);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(58, Coudert, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_function59();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(59, Coudert, function59)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		getSound()->playSound(kEntityCoudert, "Jac5010"); // Situation is under control, please remain in your compartment

		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_2000);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getEntities()->drawSequenceLeft(kEntityCoudert, "627K");
			setup_function60();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(60, Coudert, function60)
	switch (savepoint.action) {
	default:
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function61();
		break;

	case kAction155991520:
		setCallback(1);
		setup_updateFromTime(225);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(61, Coudert, function61)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_2088;

		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_4840);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("627Me", kObjectCompartmentE);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityCoudert, "627Ne");
			getEntities()->enterCompartment(kEntityCoudert, kObjectCompartmentE, true);

			setCallback(3);
			setup_updateFromTime(75);
			break;

		case 3:
			getEntities()->exitCompartment(kEntityCoudert, kObjectCompartmentE, true);

			setCallback(4);
			setup_enterExitCompartment("627Re", kObjectCompartmentE);
			break;

		case 4:
			getData()->location = kLocationInsideCompartment;

			getEntities()->clearSequences(kEntityCoudert);
			getObjects()->update(kObjectCompartmentE, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);

			setCallback(5);
			setup_playSound("Reb5010");
			break;

		case 5:
			setCallback(6);
			setup_enterExitCompartment("627Se", kObjectCompartmentE);
			break;

		case 6:
			getSavePoints()->push(kEntityCoudert, kEntityRebecca, kAction155604840);

			getData()->location = kLocationOutsideCompartment;

			setCallback(7);
			setup_updateEntity(kCarRedSleeping, kPosition_2740);
			break;

		case 7:
			setCallback(8);
			setup_enterExitCompartment("627Zh", kObjectCompartmentH);
			break;

		case 8:
			getData()->location = kLocationInsideCompartment;

			getEntities()->clearSequences(kEntityCoudert);
			getSavePoints()->push(kEntityCoudert, kEntityPascale, kAction169750080);

			setup_function62();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(62, Coudert, function62)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			if (!Entity::updateParameter(params->param4, getState()->timeTicks, 75))
				break;

			params->param1 = 0;
			params->param2 = 1;

			getObjects()->update(kObjectCompartmentH, kEntityCoudert, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param4 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (params->param1) {
			getObjects()->update(kObjectCompartmentH, kEntityCoudert, kObjectLocation1, kCursorNormal, kCursorNormal);
			params->param1 = 0;

			setCallback(1);
			setup_playSound(getSound()->justCheckingCath());
		} else {
			setCallback(savepoint.action == kActionKnock ? 2 : 3);
			setup_playSound(savepoint.action == kActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentH, kEntityCoudert, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (params->param1 || params->param2) {
			params->param1 = 0;
			params->param2 = 0;
			params->param3 = 0;

			getObjects()->update(kObjectCompartmentH, kEntityCoudert, kObjectLocation1, kCursorHandKnock, kCursorHand);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentH, kEntityCoudert, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;

		case 2:
		case 3:
			++params->param3;

			if (params->param3 == 1 || params->param2) {
				getObjects()->update(kObjectCompartmentH, kEntityCoudert, kObjectLocation1, kCursorNormal, kCursorNormal);
				setCallback(params->param3 == 1 ? 4 : 5);
				setup_playSound(params->param3 == 1 ? "Jac5002" : "Jac5002A");
			}
			break;

		case 4:
			params->param1 = 1;
			getObjects()->update(kObjectCompartmentH, kEntityCoudert, kObjectLocation1, kCursorTalk, kCursorNormal);
			break;

		case 5:
			params->param2 = 1;
			break;
		}
		break;

	case kAction135800432:
		setup_nullfunction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(63, Coudert)


//////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////
void Coudert::visitCompartment(const SavePoint &savepoint, EntityPosition position, const char *seq1, ObjectIndex compartment, const char *seq2, const char *seq3, EntityPosition sittingPosition, ObjectIndex object, const char *seq4) {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, position);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment(seq1, compartment);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityCoudert, seq2);
			getEntities()->enterCompartment(kEntityCoudert, compartment, true);

			setCallback(3);
			setup_updateFromTime(150);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment2(seq3, compartment, position, sittingPosition);
			break;

		case 4:
			getEntities()->exitCompartment(kEntityCoudert, compartment, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityCoudert);

			setCallback(5);
			setup_function20(compartment, object);
			break;

		case 5:
			setCallback(6);
			setup_enterExitCompartment(seq4, compartment);
			break;

		case 6:
			getData()->location = kLocationOutsideCompartment;
			callbackAction();
			break;
		}
		break;
	}
}

} // End of namespace LastExpress
