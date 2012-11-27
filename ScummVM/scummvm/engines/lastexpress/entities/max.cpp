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

#include "lastexpress/entities/max.h"

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

Max::Max(LastExpressEngine *engine) : Entity(engine, kEntityMax) {
	ADD_CALLBACK_FUNCTION(Max, reset);
	ADD_CALLBACK_FUNCTION(Max, playSound);
	ADD_CALLBACK_FUNCTION(Max, draw);
	ADD_CALLBACK_FUNCTION(Max, enterExitCompartment);
	ADD_CALLBACK_FUNCTION(Max, savegame);
	ADD_CALLBACK_FUNCTION(Max, chapter12_handler);
	ADD_CALLBACK_FUNCTION(Max, function7);
	ADD_CALLBACK_FUNCTION(Max, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Max, function9);
	ADD_CALLBACK_FUNCTION(Max, chapter1);
	ADD_CALLBACK_FUNCTION(Max, chapter2);
	ADD_CALLBACK_FUNCTION(Max, chapter3);
	ADD_CALLBACK_FUNCTION(Max, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Max, freeFromCage);
	ADD_CALLBACK_FUNCTION(Max, function15);
	ADD_CALLBACK_FUNCTION(Max, chapter4);
	ADD_CALLBACK_FUNCTION(Max, function17);
	ADD_CALLBACK_FUNCTION(Max, chapter5);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Max, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Max, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(3, Max, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(4, Max, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(5, Max, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Max, chapter12_handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param2, getState()->time, params->param1))
			break;

		if (!getSoundQueue()->isBuffered(kEntityMax))
			getSound()->playSound(kEntityMax, "Max1122");

		params->param1 = 255 * (4 * rnd(20) + 40);
		params->param2 = 0;
		break;

	case kActionDefault:
		params->param1 = 255 * (4 * rnd(20) + 40);
		break;

	case kAction71277948:
		setCallback(1);
		setup_function7();
		break;

	case kAction158007856:
		if (!getSoundQueue()->isBuffered(kEntityMax)) {
			getSound()->playSound(kEntityMax, "Max1122");
			params->param1 = 255 * (4 * rnd(20) + 40);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Max, function7)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param2, getState()->time, params->param1))
			break;

		if (!getSoundQueue()->isBuffered(kEntityMax))
			getSound()->playSound(kEntityMax, "Max1122");

		params->param1 = 255 * (4 * rnd(20) + 40);
		params->param2 = 0;
		break;

	case kActionKnock:
	case kActionOpenDoor:
		getObjects()->update(kObjectCompartmentF, kEntityMax, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject53, kEntityMax, kObjectLocation1, kCursorNormal, kCursorNormal);

		if (getSoundQueue()->isBuffered(kEntityMax))
			getSoundQueue()->processEntry(kEntityMax);

		setCallback((savepoint.action == kActionKnock) ? 1 : 2);
		setup_playSound((savepoint.action == kActionKnock) ? "LIB012" : "LIB013");
		break;

	case kActionDefault:
		params->param1 = 255 * (4 * rnd(20) + 40);

		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentF, kEntityMax, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject53, kEntityMax, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kActionDrawScene:
		if (!getSoundQueue()->isBuffered(kEntityMax)) {
			if (getEntities()->isPlayerPosition(kCarRedSleeping, 56) || getEntities()->isPlayerPosition(kCarRedSleeping, 78))
				getSound()->playSound(kEntityMax, "Max1120");
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		case 0:
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_playSound("Max1122");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentF, kEntityMax, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject53, kEntityMax, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;
		}
		break;

	case kAction101687594:
		getEntities()->clearSequences(kEntityMax);

		callbackAction();
		break;

	case kAction122358304:
	case kActionMaxFreeFromCage:
		getSavePoints()->push(kEntityMax, kEntityMax, kActionMaxFreeFromCage);
		getObjects()->update(kObjectCompartmentF, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject53, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		callbackAction();
		break;

	case kAction158007856:
		if (!getSoundQueue()->isBuffered(kEntityMax)) {
			getSound()->playSound(kEntityMax, "Max1122");
			params->param1 = 255 * (4 * rnd(20) + 40);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Max, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param3, getState()->time, params->param2))
			break;

		if (!getSoundQueue()->isBuffered(kEntityMax))
			getSound()->playSound(kEntityMax, "Max3101");

		params->param2 = 255 * (4 * rnd(20) + 40);
		params->param3 = 0;
		break;

	case kActionOpenDoor:
		if (params->param1) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventCathMaxLickHand);
			break;
		}

		if (getSoundQueue()->isBuffered(kEntityMax))
			getSoundQueue()->processEntry(kEntityMax);

		getAction()->playAnimation(kEventCathMaxLickHand);
		getScenes()->processScene();

		params->param1 = 1;
		break;

	case kActionDefault:
		params->param2 = 255 * (4 * rnd(20) + 40);

		getObjects()->update(kObjectCageMax, kEntityMax, kObjectLocationNone, kCursorNormal, kCursorHand);
		getEntities()->clearSequences(kEntityMax);

		getData()->entityPosition = kPosition_8000;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarBaggage;

		if (!getSoundQueue()->isBuffered(kEntityMax))
			getSound()->playSound(kEntityMax, "Max3101");
		break;

	case kActionCallback:
		if (getCallback() != 1)
			break;

		if (getSoundQueue()->isBuffered(kEntityMax))
			getSoundQueue()->processEntry(kEntityMax);

		getSound()->playSound(kEntityPlayer, "LIB026");
		getAction()->playAnimation(kEventCathMaxFree);
		getScenes()->loadSceneFromPosition(kCarBaggage, 92);
		getObjects()->update(kObjectCageMax, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);
		setup_function9();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Max, function9)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param2 == kTimeInvalid || !getState()->time)
			break;

		if (params->param1 >= getState()->time) {
			if (!getEntities()->hasValidFrame(kEntityMax) || !params->param2) {

				params->param2 = (uint)getState()->time;
				if (!params->param2)
					goto setup_functions;
			}

			if (params->param2 >= getState()->time)
				break;
		}

		params->param2 = kTimeInvalid;

setup_functions:
		if (getProgress().chapter == kChapter3)
			setup_function15();

		if (getProgress().chapter == kChapter4)
			setup_function17();
		break;

	//////////////////////////////////////////////////////////////////////////
	// Draw Max outside of cage
	case kActionDefault:
		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->drawSequenceLeft(kEntityMax, "630Af");
		getEntities()->enterCompartment(kEntityMax, kObjectCompartmentF, true);

		params->param1 = (uint)(getState()->time + 2700);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Max, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Max, setup_chapter12_handler));
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Max, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter12_handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMax);

		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Max, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMax);

		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Max, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param2) {
			getData()->entityPosition = getEntityData(kEntityCoudert)->entityPosition;
			break;
		}

		if (!Entity::updateParameter(params->param3, getState()->time, params->param1))
			break;

		if (!getSoundQueue()->isBuffered(kEntityMax))
			getSound()->playSound(kEntityMax, "Max1122");

		params->param1 = 255 * (4 * rnd(20) + 40);
		params->param3 = 0;
		break;

	case kActionDefault:
		params->param1 = 255 * (4 * rnd(20) + 40);

		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		break;

	case kAction71277948:
		setCallback(1);
		setup_function7();
		break;

	case kAction122358304:
		params->param2 = 1;
		break;

	case kActionMaxFreeFromCage:
		setup_freeFromCage();
		break;

	case kAction158007856:
		if (params->param2)
			break;

		if (!getSoundQueue()->isBuffered(kEntityMax)) {
			getSound()->playSound(kEntityMax, "Max1122");
			params->param1 = 255 * (4 * rnd(20) + 40);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Max, freeFromCage)
	switch (savepoint.action) {
	default:
		break;

	case kActionEndSound:
		getSound()->playSound(kEntityMax, "Max1122");
		break;

	//////////////////////////////////////////////////////////////////////////
	// Save game after freeing Max from his cage
	case kActionOpenDoor:
		if (getEvent(kEventCathMaxCage)) {
			if (getEvent(kEventCathMaxFree)) {
				setCallback(2);
				setup_savegame(kSavegameTypeEvent, kEventCathMaxFree);
			}

		} else {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventCathMaxCage);
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCageMax, kEntityMax, kObjectLocationNone, kCursorNormal, kCursorHand);

		getData()->entityPosition = kPosition_8000;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarBaggage;

		if (!getSoundQueue()->isBuffered(kEntityMax))
			getSound()->playSound(kEntityMax, "Max1122");
		break;

	//////////////////////////////////////////////////////////////////////////
	// Play animation for Max in the cage and after opening it
	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getSoundQueue()->isBuffered(kEntityMax))
				getSoundQueue()->removeFromQueue(kEntityMax);

			getAction()->playAnimation(kEventCathMaxCage);
			getSoundQueue()->setupEntry(kSoundType7, kEntityMax);
			getScenes()->processScene();
			break;

		case 2:
			if (getSoundQueue()->isBuffered(kEntityMax))
				getSoundQueue()->processEntry(kEntityMax);

			getSound()->playSound(kEntityPlayer, "LIB026");
			getAction()->playAnimation(kEventCathMaxFree);
			getScenes()->loadSceneFromPosition(kCarBaggage, 92);
			getObjects()->update(kObjectCageMax, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);
			setup_function9();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Max, function15)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param2) {
			getData()->entityPosition = getEntityData(kEntityCoudert)->entityPosition;
			getData()->car = getEntityData(kEntityCoudert)->car;
		}

		if (!params->param1) {
			if (!Entity::updateParameter(params->param3, getState()->time, 900))
				break;

			getSavePoints()->push(kEntityMax, kEntityCoudert, kAction157026693);
		}
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		if (!getSoundQueue()->isBuffered(kEntityMax))
			getSound()->playSound(kEntityMax, "Max3010");

		setCallback(1);
		setup_enterExitCompartment("630Bf", kObjectCompartment4);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getEntities()->drawSequenceLeft(kEntityMax, "630Af");
			getEntities()->enterCompartment(kEntityMax, kObjectCompartmentF, true);
			getSavePoints()->push(kEntityMax, kEntityAnna, kAction156622016);
		}
		break;

	case kAction122358304:
		(savepoint.entity2 == kEntityAnna) ? (params->param1 = 1) : (params->param2 = 1);
		getEntities()->exitCompartment(kEntityMax, kObjectCompartmentF, true);
		getEntities()->drawSequenceLeft(kEntityMax, "BLANK");
		break;

	case kActionMaxFreeFromCage:
		getEntities()->exitCompartment(kEntityMax, kObjectCompartmentF, true);
		setup_chapter4Handler();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Max, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityMax);

		getData()->entityPosition = kPosition_8000;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarBaggage;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Max, function17)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			getData()->entityPosition = getEntityData(kEntityCoudert)->entityPosition;
			getData()->car = getEntityData(kEntityCoudert)->car;
		}
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->drawSequenceLeft(kEntityMax, "630Af");
		getSavePoints()->push(kEntityMax, kEntityCoudert, kAction157026693);
		break;

	case kAction122358304:
		params->param1 = 1;
		getEntities()->exitCompartment(kEntityMax, kObjectCompartmentF, true);
		getEntities()->drawSequenceLeft(kEntityMax, "BLANK");
		break;

	case kActionMaxFreeFromCage:
		getEntities()->exitCompartment(kEntityMax, kObjectCompartmentF, true);
		setup_chapter4Handler();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Max, chapter5)
	if (savepoint.action == kActionDefault) {
		getEntities()->clearSequences(kEntityMax);

		getData()->entityPosition = kPositionNone;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarNone;

		getObjects()->update(kObjectCageMax, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);
	}
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
