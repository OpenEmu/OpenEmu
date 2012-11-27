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

#include "lastexpress/entities/kronos.h"

#include "lastexpress/entities/anna.h"
#include "lastexpress/entities/august.h"
#include "lastexpress/entities/kahina.h"
#include "lastexpress/entities/rebecca.h"
#include "lastexpress/entities/sophie.h"
#include "lastexpress/entities/tatiana.h"

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

static const struct {
	uint32 time;
	const char *sequence;
} concertData[54] = {
	{735, "201d"},   {1395, "201a"},  {1965, "201d"},  {2205, "201a"},  {3405, "201d"},
	{3750, "201a"},  {3975, "201d"},  {4365, "201a"},  {4650, "201d"},  {4770, "201a"},
	{4995, "201e"},  {5085, "201d"},  {5430, "201a"},  {5685, "201d"},  {5850, "201a"},
	{7515, "201d"},  {7620, "201a"},  {7785, "201d"},  {7875, "201a"},  {8235, "201d"},
	{8340, "201a"},  {8745, "201d"},  {8805, "201a"},  {8925, "201d"},  {8985, "201a"},
	{9765, "201d"},  {9930, "201a"},  {12375, "201e"}, {12450, "201d"}, {12705, "201c"},
	{13140, "201d"}, {13305, "201a"}, {13380, "201d"}, {13560, "201a"}, {14145, "201d"},
	{14385, "201a"}, {14445, "201c"}, {14805, "201a"}, {16485, "201d"}, {16560, "201a"},
	{16755, "201d"}, {16845, "201a"}, {17700, "201d"}, {17865, "201a"}, {18645, "201d"},
	{18720, "201a"}, {19410, "201e"}, {19500, "201a"}, {22020, "201d"}, {22185, "201a"},
	{22590, "201d"}, {22785, "201a"}, {23085, "201d"}, {23265, "201a"}
};

Kronos::Kronos(LastExpressEngine *engine) : Entity(engine, kEntityKronos) {
	ADD_CALLBACK_FUNCTION(Kronos, reset);
	ADD_CALLBACK_FUNCTION(Kronos, savegame);
	ADD_CALLBACK_FUNCTION(Kronos, updateEntity);
	ADD_CALLBACK_FUNCTION(Kronos, playSound);
	ADD_CALLBACK_FUNCTION(Kronos, updateFromTime);
	ADD_CALLBACK_FUNCTION(Kronos, updateFromTicks);
	ADD_CALLBACK_FUNCTION(Kronos, chapter1);
	ADD_CALLBACK_FUNCTION(Kronos, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Kronos, function9);
	ADD_CALLBACK_FUNCTION(Kronos, function10);
	ADD_CALLBACK_FUNCTION(Kronos, function11);
	ADD_CALLBACK_FUNCTION(Kronos, chapter2);
	ADD_CALLBACK_FUNCTION(Kronos, chapter3);
	ADD_CALLBACK_FUNCTION(Kronos, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Kronos, function15);
	ADD_CALLBACK_FUNCTION(Kronos, function16);
	ADD_CALLBACK_FUNCTION(Kronos, function17);
	ADD_CALLBACK_FUNCTION(Kronos, function18);
	ADD_CALLBACK_FUNCTION(Kronos, function19);
	ADD_CALLBACK_FUNCTION(Kronos, function20);
	ADD_CALLBACK_FUNCTION(Kronos, function21);
	ADD_CALLBACK_FUNCTION(Kronos, function22);
	ADD_CALLBACK_FUNCTION(Kronos, function23);
	ADD_CALLBACK_FUNCTION(Kronos, chapter4);
	ADD_CALLBACK_FUNCTION(Kronos, chapter5);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Kronos, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(2, Kronos, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(3, Kronos, updateEntity, CarIndex, EntityPosition)
	Entity::updateEntity(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(4, Kronos, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(5, Kronos, updateFromTime)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(6, Kronos, updateFromTicks)
	Entity::updateFromTicks(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Kronos, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Kronos, setup_chapter1Handler));
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6000;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarKronos;

		getObjects()->update(kObjectCeiling, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Kronos, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTime1489500, params->param2, WRAP_SETUP_FUNCTION(Kronos, setup_function11));
		break;

	case kAction171849314:
		params->param1 = 1;
		break;

	case kAction202621266:
		setup_function9();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Kronos, function9)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventKronosConversation);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventKronosConversation);
			getScenes()->loadSceneFromPosition(kCarKronos, 87);
			getSavePoints()->push(kEntityKronos, kEntityKahina, kAction137685712);
			setup_function10();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Kronos, function10)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTime1489500, params->param1, WRAP_SETUP_FUNCTION(Kronos, setup_function11));
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6000;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarKronos;

		getEntities()->clearSequences(kEntityKronos);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Kronos, function11)
	switch (savepoint.action) {
	default:
		break;

	case kActionEndSound:
		params->param1++;
		getSound()->playSound(kEntityKronos, (params->param1 & 1) ? "KRO1001" : "KRO1002");
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_7000;

		if (!getSoundQueue()->isBuffered(kEntityKronos))
			getSound()->playSound(kEntityKronos, "KRO1001");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Kronos, chapter2)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityKronos);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Kronos, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityKronos);

		getData()->entityPosition = kPosition_6000;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarKronos;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCeiling, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Kronos, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime1993500 && !params->param1 && !params->param2 && !params->param3)
			setup_function15();
		break;

	case kAction157159392:
		switch (savepoint.entity2) {
		case kEntityAnna:
			params->param1 = 1;
			break;

		case kEntityTatiana:
			params->param2 = 1;
			break;

		case kEntityAbbot:
			params->param3 = 1;
			break;

		default:
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Kronos, function15)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 && !getEntities()->isInSalon(kEntityBoutarel)) {
			if (Entity::updateParameter(params->param2, getState()->timeTicks, 75)) {
				setup_function16();
				break;
			}
		}

		if (params->param3 != kTimeInvalid && getState()->time > kTime2002500) {
			if (getState()->time <= kTime2052000) {
				if (!getEntities()->isInSalon(kEntityPlayer) || getEntities()->isInSalon(kEntityPlayer) || !params->param3)
					params->param3 = (uint)getState()->time + 900;

				if (params->param3 >= getState()->time)
					break;
			}

			params->param3 = kTimeInvalid;

			if (getEntities()->isInSalon(kEntityPlayer)) {
				setup_function16();
			} else {
				getSavePoints()->push(kEntityKronos, kEntityAnna, kAction101169422);
				getSavePoints()->push(kEntityKronos, kEntityTatiana, kAction101169422);
				getSavePoints()->push(kEntityKronos, kEntityAbbot, kAction101169422);

				setup_function18();
			}
		}
		break;

	case kActionDefault:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 60)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 59)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 83)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 81)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 87))
			params->param1 = 1;
		break;

	case kActionDrawScene:
		if (params->param1 && getEntities()->isPlayerPosition(kCarRestaurant, 51) && !getEntities()->isInSalon(kEntityBoutarel))
			setup_function16();
		else
			params->param1 = getEntities()->isPlayerPosition(kCarRestaurant, 60)
			              || getEntities()->isPlayerPosition(kCarRestaurant, 59)
			              || getEntities()->isPlayerPosition(kCarRestaurant, 83)
			              || getEntities()->isPlayerPosition(kCarRestaurant, 81)
			              || getEntities()->isPlayerPosition(kCarRestaurant, 87);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Kronos, function16)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventKronosVisit);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventKronosVisit);
			getSavePoints()->push(kEntityKronos, kEntityAnna, kAction101169422);
			getSavePoints()->push(kEntityKronos, kEntityTatiana, kAction101169422);
			getSavePoints()->push(kEntityKronos, kEntityAbbot, kAction101169422);
			getScenes()->loadSceneFromPosition(kCarRestaurant, 60);

			setup_function17();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Kronos, function17)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_9270);
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function18();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Kronos, function18)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime2079000 && !params->param2) {
			getObjects()->updateLocation2(kObjectCompartmentKronos, kObjectLocation3);
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
			params->param1 = 1;
			params->param2 = 1;
		}

		if (!Entity::timeCheck(kTime2106000, params->param3, WRAP_SETUP_FUNCTION(Kronos, setup_function19))) {
			if (params->param1 && getEntities()->isInKronosSanctum(kEntityPlayer)) {
				setCallback(1);
				setup_savegame(kSavegameTypeEvent, kEventKahinaPunchSuite4);
			}
		}
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6000;
		getData()->car = kCarKronos;
		getData()->location = kLocationOutsideCompartment;
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventKahinaPunchSuite4);
			getLogic()->gameOver(kSavegameTypeEvent2, kEventCathJumpDownCeiling, kSceneNone, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Kronos, function19)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);
		break;

	case kActionDrawScene:
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventKahinaPunchSuite4);
			getLogic()->gameOver(kSavegameTypeEvent2, kEventCathJumpDownCeiling, kSceneNone, true);
			break;

		case 2:
			getAction()->playAnimation(kEventConcertStart);
			getSoundQueue()->setupEntry(kSoundType7, kEntityKronos);
			getScenes()->loadSceneFromPosition(kCarKronos, 83);

			RESET_ENTITY_STATE(kEntityRebecca, Rebecca, setup_function39);
			RESET_ENTITY_STATE(kEntitySophie, Sophie, setup_chaptersHandler);
			RESET_ENTITY_STATE(kEntityAugust, August,  setup_function50);
			RESET_ENTITY_STATE(kEntityAnna, Anna, setup_function56);
			RESET_ENTITY_STATE(kEntityTatiana, Tatiana, setup_function35);

			setup_function20();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Kronos, function20)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		params->param5 = getSoundQueue()->getEntryTime(kEntityKronos)* 2;

		if (params->param6 < ARRAYSIZE(concertData) && params->param5 > concertData[params->param6].time) {

			getEntities()->drawSequenceLeft(kEntityKronos, concertData[params->param6].sequence);

			if (scumm_stricmp(concertData[params->param6].sequence, "201e")) {

				if (scumm_stricmp(concertData[params->param6].sequence, "201c")) {

					if (!scumm_stricmp(concertData[params->param6].sequence, "201d")) {
						if (getEntities()->isPlayerPosition(kCarKronos, 86))
							getScenes()->loadSceneFromPosition(kCarKronos, 83);

						getEntities()->updatePositionEnter(kEntityKronos, kCarKronos, 86);
						getEntities()->updatePositionExit(kEntityKronos, kCarKronos, 85);
					} else {
						getEntities()->updatePositionExit(kEntityKronos, kCarKronos, 85);
						getEntities()->updatePositionExit(kEntityKronos, kCarKronos, 86);
					}
				} else {
					if (getEntities()->isPlayerPosition(kCarKronos, 85))
						getScenes()->loadSceneFromPosition(kCarKronos, 83);

					getEntities()->updatePositionEnter(kEntityKronos, kCarKronos, 85);
					getEntities()->updatePositionExit(kEntityKronos, kCarKronos, 86);
				}
			} else {
				if (getEntities()->isPlayerPosition(kCarKronos, 85) || getEntities()->isPlayerPosition(kCarKronos, 86))
					getScenes()->loadSceneFromPosition(kCarKronos, 83);

				getEntities()->updatePositionEnter(kEntityKronos, kCarKronos, 85);
				getEntities()->updatePositionEnter(kEntityKronos, kCarKronos, 86);
			}

			++params->param6;
		}

		getObjects()->update(kObject76, kEntityKronos, kObjectLocationNone, kCursorNormal, getInventory()->hasItem(kItemBriefcase) ? kCursorHand : kCursorNormal);

		if (!params->param7) {
			params->param7 = (uint)getState()->time + 2700;
			params->param8 = (uint)getState()->time + 13500;
		}

		if (CURRENT_PARAM(1, 2) != kTimeInvalid && params->param7 < getState()->time) {
			if (Entity::updateParameterTime((TimeValue)params->param8, !params->param1, CURRENT_PARAM(1, 2), 450)) {
				getSavePoints()->push(kEntityKronos, kEntityKahina, kAction237555748);
			}
		}

		if (!params->param1)
			params->param2 = params->param3;

		params->param2 -= getState()->timeDelta;

		if (params->param2 < getState()->timeDelta) {

			getSavePoints()->push(kEntityKronos, kEntityKahina, kAction92186062);

			++params->param4;
			switch (params->param4) {
			default:
				break;

			case 1:
				getAction()->playAnimation(kEventCathWakingUp);
				getScenes()->processScene();
				params->param3 = 1800;
				break;

			case 2:
				getAction()->playAnimation(kEventCathWakingUp);
				getScenes()->processScene();
				params->param3 = 3600;
				break;

			case 3:
				getAction()->playAnimation(kEventCathFallingAsleep);

				while (getSoundQueue()->isBuffered("1919.LNK"))
					getSoundQueue()->updateQueue();

				getAction()->playAnimation(kEventCathWakingUp);
				getScenes()->processScene();
				params->param3 = 162000;
				break;
			}
			params->param2 = params->param3;
		}

		if (params->param5 > 23400 || CURRENT_PARAM(1, 1)) {
			if (getEntities()->isInKronosSanctum(kEntityPlayer)) {
				setCallback(1);
				setup_savegame(kSavegameTypeEvent, kEventKahinaWrongDoor);
			}
		}
		break;

	case kActionEndSound:
		getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);

		if (CURRENT_PARAM(1, 1)) {
			getSound()->playSound(kEntityPlayer, "BUMP");
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 26);

			setup_function21();
			break;
		}

		if (getEntities()->isInKronosSalon(kEntityPlayer)) {
			setCallback(3);
			setup_savegame(kSavegameTypeEvent, kEventConcertEnd);
			break;
		}

		if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarKronos)) {
			getSound()->playSound(kEntityKronos, "Kro3001");
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorNormal, kCursorNormal);
			CURRENT_PARAM(1, 1) = 1;
			break;
		}

		setup_function21();
		break;

	case kActionOpenDoor:
		setCallback(2);
		setup_savegame(kSavegameTypeEvent, kEventConcertLeaveWithBriefcase);
		break;

	case kActionDefault:
		getState()->time = kTime2115000;
		getState()->timeDelta = 3;

		params->param1 = (getEntities()->isPlayerPosition(kCarKronos, 88)
		               || getEntities()->isPlayerPosition(kCarKronos, 84)
		               || getEntities()->isPlayerPosition(kCarKronos, 85)
		               || getEntities()->isPlayerPosition(kCarKronos, 86)
		               || getEntities()->isPlayerPosition(kCarKronos, 83));

		if (getInventory()->hasItem(kItemFirebird))
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorNormal);
		else
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		getObjects()->update(kObject76, kEntityKronos, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		getProgress().field_40 = 1;
		getEntities()->drawSequenceLeft(kEntityKronos, "201a");

		params->param2 = 2700;
		params->param3 = 2700;
		break;

	case kActionDrawScene:
		params->param1 = (getEntities()->isPlayerPosition(kCarKronos, 88)
		               || getEntities()->isPlayerPosition(kCarKronos, 84)
		               || getEntities()->isPlayerPosition(kCarKronos, 85)
		               || getEntities()->isPlayerPosition(kCarKronos, 86)
		               || getEntities()->isPlayerPosition(kCarKronos, 83));

		if (getInventory()->hasItem(kItemFirebird))
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorNormal, kCursorNormal);
		else
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventKahinaWrongDoor);

			if (getInventory()->hasItem(kItemBriefcase))
				getInventory()->removeItem(kItemBriefcase);

			getSound()->playSound(kEntityPlayer, "BUMP");
			getScenes()->loadSceneFromPosition(kCarKronos, 81);
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorNormal, kCursorNormal);
			getSound()->playSound(kEntityPlayer, "LIB015");
			break;

		case 2:
			getData()->entityPosition = kPosition_6000;
			getAction()->playAnimation(kEventConcertLeaveWithBriefcase);

			RESET_ENTITY_STATE(kEntityKahina, Kahina, setup_function21);

			getScenes()->loadSceneFromPosition(kCarKronos, 87);
			break;

		case 3:
			getAction()->playAnimation(kEventConcertEnd);
			getSound()->playSound(kEntityPlayer, "BUMP");
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 26);

			setup_function21();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Kronos, function21)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInKronosSanctum(kEntityPlayer)) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventKahinaWrongDoor);
		}
		break;

	case kActionDefault:
		getProgress().field_40 = 0;
		getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorNormal, kCursorNormal);
		getSavePoints()->push(kEntityKronos, kEntityRebecca, kAction191668032);
		if (!getEvent(kEventConcertLeaveWithBriefcase))
			setup_function22();
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventKahinaWrongDoor);

			if (getInventory()->hasItem(kItemBriefcase))
				getInventory()->removeItem(kItemBriefcase);

			getSound()->playSound(kEntityPlayer, "BUMP");

			getScenes()->loadSceneFromPosition(kCarKronos, 81);

			getSound()->playSound(kEntityPlayer, "LIB015");
		}
		break;

	case kAction235599361:
		setup_function22();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Kronos, function22)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getProgress().field_44) {
			setCallback(5);
			setup_savegame(kSavegameTypeEvent, kEventKahinaPunchBaggageCarEntrance);
		} else {
			setCallback(6);
			setup_savegame(kSavegameTypeEvent, kEventKahinaWrongDoor);
		}
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (!getSoundQueue()->isBuffered(savepoint.action == kActionKnock ? "LIB012" : "LIB013", true))
			getSound()->playSound(kEntityPlayer, savepoint.action == kActionKnock ? "LIB012" : "LIB013");

		if (getEvent(kEventConcertLeaveWithBriefcase))
			getSavePoints()->call(kEntityKronos, kEntityKahina, kAction137503360);

		if (getInventory()->hasItem(kItemBriefcase)) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventKronosReturnBriefcase);
			break;
		}

		if (getInventory()->hasItem(kItemFirebird) && getEvent(kEventConcertLeaveWithBriefcase)) {
			setCallback(2);
			setup_savegame(kSavegameTypeEvent, kEventKronosBringEggCeiling);
			break;
		}

		if (getInventory()->hasItem(kItemFirebird)) {
			setCallback(3);
			setup_savegame(kSavegameTypeEvent, kEventKronosBringEggCeiling);
			break;
		}

		if (getEvent(kEventConcertLeaveWithBriefcase)) {
			setCallback(4);
			setup_savegame(kSavegameTypeEvent, kEventKronosBringNothing);
			break;
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentKronos, kEntityKronos, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventKronosReturnBriefcase);
			getScenes()->loadSceneFromPosition(kCarKronos, 87);
			getInventory()->removeItem(kItemFirebird);
			getInventory()->removeItem(kItemScarf);

			setup_function23();
			break;

		case 2:
			getAction()->playAnimation(kEventKronosBringEggCeiling);
			getScenes()->loadSceneFromPosition(kCarKronos, 87);
			getInventory()->removeItem(kItemFirebird);
			getInventory()->get(kItemFirebird)->location = kObjectLocation5;

			setup_function23();
			break;

		case 3:
			getInventory()->removeItem(kItemFirebird);
			getInventory()->get(kItemFirebird)->location = kObjectLocation5;
			getAction()->playAnimation(kEventKronosBringEgg);
			getScenes()->loadSceneFromPosition(kCarKronos, 87);
			getInventory()->addItem(kItemBriefcase);
			setup_function23();
			break;

		case 4:
			getAction()->playAnimation(kEventKronosBringNothing);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneNone, true);
			break;

		case 5:
			getAction()->playAnimation(kEventKahinaPunchSuite4);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneNone, true);
			break;

		case 6:
			getAction()->playAnimation(kEventKahinaWrongDoor);
			if (getInventory()->hasItem(kItemBriefcase))
				getInventory()->removeItem(kItemBriefcase);

			getSound()->playSound(kEntityPlayer, "BUMP");
			getScenes()->loadSceneFromPosition(kCarKronos, 81);
			getSound()->playSound(kEntityPlayer, "LIB015");
			break;
		}
		break;

	case kAction138085344:
		setup_function23();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Kronos, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInKronosSanctum(kEntityPlayer)) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventKahinaWrongDoor);
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventKahinaWrongDoor);

			if (getInventory()->hasItem(kItemBriefcase))
				getInventory()->removeItem(kItemBriefcase);

			getSound()->playSound(kEntityPlayer, "BUMP");

			getScenes()->loadSceneFromPosition(kCarKronos, 81);

			getSound()->playSound(kEntityPlayer, "LIB015");
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Kronos, chapter4)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityKronos);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Kronos, chapter5)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityKronos);
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
