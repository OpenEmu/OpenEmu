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

#include "lastexpress/entities/salko.h"

#include "lastexpress/fight/fight.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Salko::Salko(LastExpressEngine *engine) : Entity(engine, kEntitySalko) {
	ADD_CALLBACK_FUNCTION(Salko, reset);
	ADD_CALLBACK_FUNCTION(Salko, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(Salko, draw);
	ADD_CALLBACK_FUNCTION(Salko, updateEntity);
	ADD_CALLBACK_FUNCTION(Salko, updateFromTime);
	ADD_CALLBACK_FUNCTION(Salko, savegame);
	ADD_CALLBACK_FUNCTION(Salko, function7);
	ADD_CALLBACK_FUNCTION(Salko, function8);
	ADD_CALLBACK_FUNCTION(Salko, chapter1);
	ADD_CALLBACK_FUNCTION(Salko, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Salko, function11);
	ADD_CALLBACK_FUNCTION(Salko, chapter2);
	ADD_CALLBACK_FUNCTION(Salko, function13);
	ADD_CALLBACK_FUNCTION(Salko, chapter3);
	ADD_CALLBACK_FUNCTION(Salko, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Salko, function16);
	ADD_CALLBACK_FUNCTION(Salko, function17);
	ADD_CALLBACK_FUNCTION(Salko, chapter4);
	ADD_CALLBACK_FUNCTION(Salko, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Salko, function20);
	ADD_CALLBACK_FUNCTION(Salko, function21);
	ADD_CALLBACK_FUNCTION(Salko, function22);
	ADD_CALLBACK_FUNCTION(Salko, chapter5);
	ADD_CALLBACK_FUNCTION(Salko, chapter5Handler);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Salko, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(2, Salko, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(3, Salko, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(4, Salko, updateEntity, CarIndex, EntityPosition)
	Entity::updateEntity(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(5, Salko, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(6, Salko, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(7, Salko, function7, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone: {
		params->param3 = 0;

		EntityDirection direction = getData()->direction;
		CarIndex carSalko = getData()->car;
		CarIndex carIvo   = getEntityData(kEntityIvo)->car;
		EntityPosition positionSalko = getData()->entityPosition;
		EntityPosition positionIvo   = getEntityData(kEntityIvo)->entityPosition;

		if (getEntities()->isDistanceBetweenEntities(kEntitySalko, kEntityIvo, 500)
		 || (direction == kDirectionUp   && (carSalko > carIvo || (carSalko == carIvo && positionSalko > positionIvo)))
		 || (direction == kDirectionDown && (carSalko < carIvo || (carSalko == carIvo && positionSalko < positionIvo)))) {
			 getData()->field_49B = 0;
			 params->param3 = 1;
		}

		if (!params->param3)
			getEntities()->updateEntity(kEntitySalko, (CarIndex)params->param1, (EntityPosition)params->param2);

		}
		break;

	case kActionExcuseMeCath:
	case kActionExcuseMe:
		getSound()->playSound(kEntityPlayer, "ZFX1002", getSound()->getSoundFlag(kEntitySalko));
		getSound()->playSound(kEntityPlayer, "CAT1127A");
		break;

	case kActionDefault:
		getEntities()->updateEntity(kEntitySalko, (CarIndex)params->param1, (EntityPosition)params->param2);
		break;

	case kAction123668192:
		callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Salko, function8)
	if (savepoint.action == kActionDefault) {
		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Salko, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Salko, setup_chapter1Handler));
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_4691;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Salko, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		getData()->entityPosition = getEntityData(kEntityIvo)->entityPosition;
		getData()->location = getEntityData(kEntityIvo)->location;
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getEntities()->clearSequences(kEntitySalko);
			setup_function8();
		}
		break;

	case kAction125242096:
		setCallback(1);
		setup_function7(kCarRedSleeping, kPosition_2740);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Salko, function11)
	if (savepoint.action == kActionDefault) {
		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->clearSequences(kEntitySalko);
		getObjects()->update(kObjectCompartmentH, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Salko, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntitySalko);

		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;

	case kAction136184016:
		setCallback(1);
		setup_function13();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Salko, function13)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_enterExitCompartment("612DH", kObjectCompartmentH);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			getSavePoints()->push(kEntitySalko, kEntityIvo, kAction102675536);
			getEntities()->clearSequences(kEntitySalko);
			break;

		case 3:
			getEntities()->drawSequenceLeft(kEntitySalko, "BLANK");
			getData()->location = kLocationInsideCompartment;

			setup_function8();
			break;
		}
		break;

	case kAction125242096:
		setCallback(3);
		setup_function7(kCarRedSleeping, kPosition_2740);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Salko, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntitySalko);

		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Salko, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time < kTime2200500) {
			if (!Entity::updateParameter(params->param1, getState()->time, 81000))
				break;

			setCallback(1);
			setup_function16();
		}
		break;

	case kActionCallback:
		if (getCallback() == 1)
			params->param1 = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Salko, function16)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->hasValidFrame(kEntitySalko) && getEntities()->isDistanceBetweenEntities(kEntitySalko, kEntityPlayer, 5000)) {
			getSavePoints()->push(kEntitySalko, kEntityMax, kAction158007856);

			setCallback(3);
			setup_updateFromTime(75);
			break;
		}

label_callback3:
		if (!Entity::updateParameter(params->param1, getState()->time, 4500))
			break;

		getSavePoints()->push(kEntitySalko, kEntitySalko, kAction101169464);
		break;

	case kActionDefault:
		setCallback(1);
		setup_enterExitCompartment("612DH", kObjectCompartmentH);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;

			if (getData()->entityPosition < kPosition_2087)
				getData()->entityPosition = kPosition_2088;

			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_4070);
			break;

		case 2:
			break;

		case 3:
			getSavePoints()->push(kEntitySalko, kEntitySalko, kAction101169464);
			goto label_callback3;

		case 4:
			getEntities()->exitCompartment(kEntitySalko, kObjectCompartmentF, true);

			setCallback(5);
			setup_updateEntity(kCarRedSleeping, kPosition_9460);
			break;

		case 5:
			setCallback(6);
			setup_updateFromTime(4500);
			break;

		case 6:
			setCallback(7);
			setup_updateEntity(kCarRedSleeping, kPosition_2740);
			break;

		case 7:
			setCallback(8);
			setup_enterExitCompartment("612Ch", kObjectCompartmentH);
			break;

		case 8:
			getData()->location = kLocationInsideCompartment;
			getData()->entityPosition = kPosition_2740;
			getEntities()->clearSequences(kEntitySalko);

			callbackAction();
			break;
		}
		break;

	case kAction101169464:
		setCallback(4);
		setup_enterExitCompartment("612Bf", kObjectCompartmentF);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Salko, function17)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_2740);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("612Ch", kObjectCompartmentH);
			break;

		case 2:
			getEntities()->clearSequences(kEntitySalko);
			getData()->entityPosition = kPosition_2740;
			getData()->location = kLocationInsideCompartment;
			getSavePoints()->push(kEntitySalko, kEntityMilos, kAction157691176);

			setup_chapter3Handler();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Salko, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntitySalko);

		getData()->entityPosition = kPosition_5420;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Salko, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getEntities()->drawSequenceLeft(kEntitySalko, "BLANK");

			getData()->location = kLocationInsideCompartment;

			setup_function20();
		}
		break;

	case kAction125242096:
		setCallback(1);
		setup_function7(kCarRedSleeping, kPosition_2740);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Salko, function20)
	switch (savepoint.action) {
	default:
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			if (getData()->entityPosition < kPosition_2087)
				getData()->entityPosition = kPosition_2088;

			setCallback(2);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			getEntities()->clearSequences(kEntitySalko);
			setup_function21();
			break;
		}
		break;

	case kAction55996766:
		setCallback(1);
		setup_enterExitCompartment("612Dh", kObjectCompartmentH);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Salko, function21)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime2422800 && !params->param1) {
			params->param1 = 1;
			setCallback(1);
			setup_updateEntity(kCarRedSleeping, kPosition_2740);
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("612Ch", kObjectCompartmentH);
			break;

		case 2:
			setup_function22();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Salko, function22)
	if (savepoint.action == kActionDefault) {
		getEntities()->clearSequences(kEntitySalko);
		getObjects()->update(kObjectCompartmentH, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->inventoryItem = kItemNone;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Salko, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntitySalko);

		getData()->entityPosition = kPosition_9460;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Salko, chapter5Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getSoundQueue()->isBuffered("MUS050"))
				getSoundQueue()->processEntry("MUS050");

			getAction()->playAnimation(kEventCathSalkoTrainTopFight);

			setCallback(2);
			setup_savegame(kSavegameTypeTime, kTimeNone);
			break;

		case 2:
			params->param1 = getFight()->setup(kFightSalko);

			if (params->param1 == Fight::kFightEndWin) {
				getState()->time = (TimeValue)(getState()->time + 1800);
				setCallback(3);
				setup_savegame(kSavegameTypeEvent, kEventCathSalkoTrainTopWin);
			} else {
				getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, params->param1 == Fight::kFightEndLost);
			}
			break;

		case 3:
			getAction()->playAnimation(kEventCathSalkoTrainTopWin);
			getSavePoints()->push(kEntitySalko, kEntityVesna, kAction134427424);

			getScenes()->loadSceneFromPosition(kCarRestaurant, 10);
			setup_nullfunction();
			break;
		}
		break;

	case kAction167992577:
		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventCathSalkoTrainTopFight);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(25, Salko)

} // End of namespace LastExpress
