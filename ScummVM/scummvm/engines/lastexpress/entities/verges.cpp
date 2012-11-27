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

#include "lastexpress/entities/verges.h"

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

Verges::Verges(LastExpressEngine *engine) : Entity(engine, kEntityVerges) {
	ADD_CALLBACK_FUNCTION(Verges, reset);
	ADD_CALLBACK_FUNCTION(Verges, draw);
	ADD_CALLBACK_FUNCTION(Verges, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(Verges, playSound);
	ADD_CALLBACK_FUNCTION(Verges, playSound16);
	ADD_CALLBACK_FUNCTION(Verges, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION(Verges, savegame);
	ADD_CALLBACK_FUNCTION(Verges, updateEntity);
	ADD_CALLBACK_FUNCTION(Verges, walkBetweenCars);
	ADD_CALLBACK_FUNCTION(Verges, makeAnnouncement);
	ADD_CALLBACK_FUNCTION(Verges, function11);
	ADD_CALLBACK_FUNCTION(Verges, function12);
	ADD_CALLBACK_FUNCTION(Verges, baggageCar);
	ADD_CALLBACK_FUNCTION(Verges, updateFromTime);
	ADD_CALLBACK_FUNCTION(Verges, dialog);
	ADD_CALLBACK_FUNCTION(Verges, dialog2);
	ADD_CALLBACK_FUNCTION(Verges, talkAboutPassengerList);
	ADD_CALLBACK_FUNCTION(Verges, chapter1);
	ADD_CALLBACK_FUNCTION(Verges, talkHarem);
	ADD_CALLBACK_FUNCTION(Verges, talkPassengerList);
	ADD_CALLBACK_FUNCTION(Verges, talkGendarmes);
	ADD_CALLBACK_FUNCTION(Verges, askMertensToRelayAugustInvitation);
	ADD_CALLBACK_FUNCTION(Verges, function23);
	ADD_CALLBACK_FUNCTION(Verges, policeGettingOffTrain);
	ADD_CALLBACK_FUNCTION(Verges, policeSearch);
	ADD_CALLBACK_FUNCTION(Verges, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Verges, chapter2);
	ADD_CALLBACK_FUNCTION(Verges, chapter2Handler);
	ADD_CALLBACK_FUNCTION(Verges, chapter3);
	ADD_CALLBACK_FUNCTION(Verges, function30);
	ADD_CALLBACK_FUNCTION(Verges, talkAboutMax);
	ADD_CALLBACK_FUNCTION(Verges, function32);
	ADD_CALLBACK_FUNCTION(Verges, function33);
	ADD_CALLBACK_FUNCTION(Verges, function34);
	ADD_CALLBACK_FUNCTION(Verges, organizeConcertInvitations);
	ADD_CALLBACK_FUNCTION(Verges, chapter4);
	ADD_CALLBACK_FUNCTION(Verges, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Verges, resetState);
	ADD_CALLBACK_FUNCTION(Verges, chapter5);
	ADD_CALLBACK_FUNCTION(Verges, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Verges, askPassengersToStayInCompartments);
	ADD_CALLBACK_FUNCTION(Verges, end);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Verges, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Verges, draw)
	Entity::draw(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(3, Verges, callbackActionOnDirection)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getData()->direction != kDirectionRight)
			callbackAction();
		break;

	case kActionExitCompartment:
		callbackAction();
 		break;

	case kActionExcuseMeCath:
		if (!params->param1) {
			getSound()->excuseMe(kEntityVerges);
			params->param1 = 1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(4, Verges, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(5, Verges, playSound16)
	Entity::playSound(savepoint, false, kFlagDefault);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Verges, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(7, Verges, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(8, Verges, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kActionExcuseMeCath) {
		if (!getSoundQueue()->isBuffered(kEntityVerges))
			getSound()->playSound(kEntityPlayer, "TRA1113", getSound()->getSoundFlag(kEntityVerges));

		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(9, Verges, walkBetweenCars)
switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getObjects()->update(kObject104, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);
		getObjects()->update(kObject105, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);

		if (getEntities()->isInBaggageCar(kEntityPlayer) || getEntities()->isInKitchen(kEntityPlayer)) {
			getAction()->playAnimation(getEntities()->isInBaggageCar(kEntityPlayer) ? kEventVergesBaggageCarOffLimits : kEventVergesCanIHelpYou);
			getSound()->playSound(kEntityPlayer, "BUMP");
			getScenes()->loadSceneFromPosition(kCarRestaurant, 65);
		}

		getScenes()->loadSceneFromItemPosition(kItem9);
		getData()->car = kCarRestaurant;
		getData()->entityPosition = kPosition_5900;

		setCallback(1);
		setup_callbackActionRestaurantOrSalon();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;
			getSound()->playSound(kEntityVerges, (char *)&params->seq1);

			setCallback(2);
			setup_draw("813DD");
			break;

		case 2:
			if (!getSoundQueue()->isBuffered(kEntityVerges))
				getSound()->playSound(kEntityVerges, (char *)&params->seq1);

			getEntities()->drawSequenceRight(kEntityVerges, "813DS");

			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityVerges);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			setCallback(4);
			setup_makeAnnouncement(kCarGreenSleeping, kPosition_540, (char *)&params->seq1);
			break;

		case 4:
			getEntities()->clearSequences(kEntityVerges);

			setCallback(5);
			setup_updateFromTime(225);
			break;

		case 5:
			setCallback(6);
			setup_function11();
			break;

		case 6:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_IIS(10, Verges, makeAnnouncement, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param7) {
			if (!getSoundQueue()->isBuffered(kEntityVerges)) {
				getSound()->playSound(kEntityVerges, (char *)&params->seq);
				params->param7 = 1;
			}
		}

		if (getEntities()->updateEntity(kEntityVerges, (CarIndex)params->param1, (EntityPosition)params->param2)) {
			callbackAction();
			break;
		}

		if (params->param6) {
			if (!Entity::updateParameter(params->param8, getState()->timeTicks, 75))
				break;

			getSound()->playSound(kEntityVerges, (char *)&params->seq);

			params->param6 = 0;
			params->param8 = 0;
		}
		break;

	case kActionEndSound:
		params->param6 = 1;
		break;

	case kActionDefault:
		if (!getSoundQueue()->isBuffered(kEntityVerges)) {
			getSound()->playSound(kEntityVerges, (char *)&params->seq);
			params->param7 = 1;
		}

		if (getEntities()->updateEntity(kEntityVerges, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Verges, function11)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRestaurant, kPosition_540);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 2:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(3);
			setup_draw("813US");
			break;

		case 3:
			getEntities()->drawSequenceRight(kEntityVerges, "813UD");

			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityVerges);

			setCallback(4);
			setup_callbackActionOnDirection();
			break;

		case 4: {
			getEntities()->clearSequences(kEntityVerges);

			bool loadscene = true;

			if (getEntities()->isInBaggageCarEntrance(kEntityPlayer))
				getAction()->playAnimation(kEventVergesEscortToDiningCar);
			else if (getEntities()->isInBaggageCar(kEntityPlayer))
				getAction()->playAnimation(kEventVergesBaggageCarOffLimits);
			else if (getEntities()->isInKitchen(kEntityPlayer))
				getAction()->playAnimation(kEventVergesCanIHelpYou);
			else
				loadscene = false;

			if (loadscene) {
				getSound()->playSound(kEntityPlayer, "BUMP");
				getScenes()->loadSceneFromPosition(kCarRestaurant, 65);
			}

			getInventory()->setLocationAndProcess(kItem9, kObjectLocation1);

			getData()->car = kCarBaggage;
			getData()->entityPosition = kPosition_5000;

			getObjects()->update(kObject104, kEntityVerges, kObjectLocationNone, kCursorNormal, kCursorHand);
			getObjects()->update(kObject105, kEntityVerges, kObjectLocationNone, kCursorNormal, kCursorHand);

			callbackAction();
			break;
			}
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Verges, function12)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getObjects()->update(kObject104, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);
		getObjects()->update(kObject105, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);

		if (getEntities()->isInBaggageCar(kEntityPlayer) || getEntities()->isInKitchen(kEntityPlayer)) {
			getAction()->playAnimation(getEntities()->isInBaggageCar(kEntityPlayer) ? kEventVergesBaggageCarOffLimits : kEventVergesCanIHelpYou);
			getSound()->playSound(kEntityPlayer, "BUMP");
			getScenes()->loadSceneFromPosition(kCarRestaurant, 65);
		}

		getScenes()->loadSceneFromItemPosition(kItem9);

		getData()->car = kCarRestaurant;
		getData()->entityPosition = kPosition_5900;

		setCallback(1);
		setup_callbackActionRestaurantOrSalon();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_draw("813DD");
			break;

		case 2:
			getEntities()->drawSequenceRight(kEntityVerges, "813DS");

			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityVerges);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			getData()->entityPosition = kPosition_850;
			getEntities()->clearSequences(kEntityVerges);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(13, Verges, baggageCar, bool)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventVergesSuitcase);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			if (getEvent(kEventVergesSuitcase) || getEvent(kEventVergesSuitcaseNight) || getEvent(kEventVergesSuitcaseOtherEntry) || getEvent(kEventVergesSuitcaseNightOtherEntry))
				params->param2 = 1;

			if (isNight() && getProgress().chapter != kChapter1)
				params->param2 = 1;

			if (params->param1) {
				if (isNight())
					getAction()->playAnimation(params->param2 ? kEventVergesSuitcaseNightOtherEntryStart : kEventVergesSuitcaseNightOtherEntry);
				else
					getAction()->playAnimation(params->param2 ? kEventVergesSuitcaseOtherEntryStart : kEventVergesSuitcaseOtherEntry);
			} else {
				if (isNight())
					getAction()->playAnimation(params->param2 ? kEventVergesSuitcaseNightStart : kEventVergesSuitcaseNight);
				else
					getAction()->playAnimation(params->param2 ? kEventVergesSuitcaseStart : kEventVergesSuitcase);
			}

			getEntities()->clearSequences(kEntityVerges);
			getScenes()->loadSceneFromPosition(kCarBaggage, 91);

			callbackAction();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(14, Verges, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_IS(15, Verges, dialog, EntityIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param5 && params->param6) {
			getSavePoints()->push(kEntityVerges, (EntityIndex)params->param1, kAction125499160);

			if (!getEntities()->isPlayerPosition(kCarGreenSleeping, 2) && !getEntities()->isPlayerPosition(kCarRedSleeping, 2))
				getData()->entityPosition = kPosition_2088;

			callbackAction();
		}
		break;

	case kActionEndSound:
		params->param5 = 1;
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityVerges, "620F");
		getSavePoints()->push(kEntityVerges, (EntityIndex)params->param1, kAction171394341);
		break;

	case kAction155853632:
		params->param6 = 1;
		break;

	case kAction202558662:
		getEntities()->drawSequenceLeft(kEntityVerges, "620E");
		getSound()->playSound(kEntityVerges, (char *)&params->seq);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_ISS(16, Verges, dialog2, EntityIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (CURRENT_PARAM(1, 1) && params->param8) {
			getSavePoints()->push(kEntityVerges, (EntityIndex)params->param1, kAction125499160);

			if (!getEntities()->isPlayerPosition(kCarGreenSleeping, 2) && !getEntities()->isPlayerPosition(kCarRedSleeping, 2))
				getData()->entityPosition = kPosition_2088;

			callbackAction();
		}
		break;

	case kActionEndSound:
		CURRENT_PARAM(1, 1)++;

		if (CURRENT_PARAM(1, 1) == 1)
			getSound()->playSound(kEntityVerges, (char *)&params->seq2);
		break;

	case kActionDefault:
		getEntities()->drawSequenceLeft(kEntityVerges, "620F");
		getSavePoints()->push(kEntityVerges, (EntityIndex)params->param1, kAction171394341);
		break;

	case kAction155853632:
		params->param8 = 1;
		break;

	case kAction202558662:
		getEntities()->drawSequenceLeft(kEntityVerges, "620E");
		getSound()->playSound(kEntityVerges, (char *)&params->seq1);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Verges, talkAboutPassengerList)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function12();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 2:
			setCallback(3);
			setup_dialog(kEntityMertens, "TRA1291");
			break;

		case 3:
			setCallback(4);
			setup_function11();
			break;

		case 4:
			ENTITY_PARAM(0, 3) = 0;
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Verges, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Verges, setup_chapter1Handler));
		break;

	case kActionDefault:
		getSavePoints()->addData(kEntityVerges, kActionDeliverMessageToTyler, 0);
		getSavePoints()->addData(kEntityVerges, kAction226031488, 1);
		getSavePoints()->addData(kEntityVerges, kAction339669520, 1);
		getSavePoints()->addData(kEntityVerges, kAction167854368, 4);
		getSavePoints()->addData(kEntityVerges, kAction158617345, 2);
		getSavePoints()->addData(kEntityVerges, kAction168255788, 3);
		getSavePoints()->addData(kEntityVerges, kAction201431954, 5);
		getSavePoints()->addData(kEntityVerges, kAction168187490, 6);

		getObjects()->update(kObject104, kEntityVerges, kObjectLocationNone, kCursorNormal, kCursorHand);
		getObjects()->update(kObject105, kEntityVerges, kObjectLocationNone, kCursorNormal, kCursorHand);

		getData()->entityPosition = kPosition_5000;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarBaggage;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(19, Verges, talkHarem)
	talk(savepoint, "TRA1202", "TRA1201");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Verges, talkPassengerList)
	talk(savepoint, "TRA1205", "TRA1206");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Verges, talkGendarmes)
	talk(savepoint, "TRA1250", "TRA1251");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Verges, askMertensToRelayAugustInvitation)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function12();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 2:
			if (getEvent(kEventMertensAskTylerCompartment) || getEvent(kEventMertensAskTylerCompartmentD) || getEvent(kEventMertensAugustWaiting)) {
				setCallback(3);
				setup_dialog2(kEntityMertens, "TRA1200", "TRA1201");
			} else {
				setCallback(4);
				setup_dialog2(kEntityMertens, "TRA1200A", "TRA1201");
			}
			break;

		case 3:
		case 4:
			getSavePoints()->push(kEntityVerges, kEntityMertens, kAction169633856);

			setCallback(5);
			setup_function11();
			break;

		case 5:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Verges, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getScenes()->loadSceneFromItemPosition(kItem9);

		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		break;

	case kAction191477936:
		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		setCallback(1);
		setup_function11();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Verges, policeGettingOffTrain)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isDistanceBetweenEntities(kEntityVerges, kEntityPlayer, 1000) && getEntityData(kEntityPlayer)->location == kLocationOutsideCompartment) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventGendarmesArrestation);
		}
		break;

	case kActionEndSound:
		callbackAction();
		break;

	case kActionDefault:
		getSound()->playSound(kEntityVerges, "POL1101", kFlagDefault);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getSoundQueue()->processEntry(kEntityVerges);
			getAction()->playAnimation(kEventGendarmesArrestation);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverPolice1, true);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Verges, policeSearch)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_savegame(kSavegameTypeTime, kTimeNone);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getScenes()->loadSceneFromItemPosition(kItem9);

			if (!getEntities()->isInKronosSalon(kEntityPlayer)) {

				if (getEntityData(kEntityPlayer)->car > kCarRedSleeping
				 || (getEntityData(kEntityPlayer)->car == kCarRedSleeping && getEntityData(kEntityPlayer)->entityPosition > kPosition_9270)) {
					getSound()->playSound(kEntityPlayer, "BUMP");
					getScenes()->loadSceneFromPosition(kCarRedSleeping, 40);

					getData()->car = kCarRedSleeping;
					getData()->entityPosition = kPosition_9270;
				} else {
					if (getEntityData(kEntityPlayer)->car > kCarGreenSleeping
					 || (getEntityData(kEntityPlayer)->car == kCarGreenSleeping && getEntityData(kEntityPlayer)->entityPosition < kPosition_4840)) {
						getSound()->playSound(kEntityPlayer, "BUMP");
						getScenes()->loadSceneFromObject(kObjectCompartment5, true);
					}

					getData()->car = kCarGreenSleeping;
					getData()->entityPosition = kPosition_850;
				}

				getData()->location = kLocationOutsideCompartment;

				getObjects()->update(kObjectRestaurantCar, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorForward);
				getObjects()->update(kObjectCompartmentE, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);

				if (getEntities()->isOutsideAnnaWindow())
					getScenes()->loadSceneFromPosition(kCarRedSleeping, 49);

				if (getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_4840)
				 || getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_4455)) {
					getAction()->playAnimation(isNight() ? kEventCathTurningNight : kEventCathTurningDay);
					getSound()->playSound(kEntityPlayer, "BUMP");
					getScenes()->loadSceneFromObject(kObjectCompartmentE, true);
				}

				getSavePoints()->push(kEntityVerges, kEntityGendarmes, kAction169499649);

				getProgress().field_3C = 1;
				getState()->timeDelta = 1;

				if (getData()->car == kCarRedSleeping) {
					setCallback(6);
					setup_makeAnnouncement(kCarGreenSleeping, kPosition_540, "TRA1005");
				} else {
					setCallback(7);
					setup_makeAnnouncement(kCarRedSleeping, kPosition_9460, "TRA1006");
				}
				break;
			}
			// Fallback to next case

		case 2:
			if (getEvent(kEventKronosConversation)) {
				getProgress().field_3C = 1;
				getData()->car = kCarGreenSleeping;
				getData()->entityPosition = kPosition_540;
				getData()->location = kLocationOutsideCompartment;

				getState()->timeDelta = 3;
				getSavePoints()->push(kEntityVerges, kEntityChapters, kAction169629818);

				setCallback(3);
				setup_policeGettingOffTrain();
			} else {
				setCallback(2);
				setup_updateFromTime(150);
			}
			break;

		case 3:
			getSavePoints()->push(kEntityVerges, kEntityCoudert, kAction168254872);

			setCallback(4);
			setup_makeAnnouncement(kCarRedSleeping, kPosition_9460, "TRA1006");
			break;

		case 4:
			setCallback(5);
			setup_function11();
			break;

		case 5:
		case 11:
			ENTITY_PARAM(0, 7) = 0;

			callbackAction();
			break;

		case 6:
		case 7:
			getEntities()->clearSequences(kEntityVerges);
			break;

		case 8:
			getSavePoints()->push(kEntityVerges, kEntityChapters, kAction169629818);

			setCallback(9);
			setup_policeGettingOffTrain();
			break;

		case 9:
			getObjects()->update(kObjectRestaurantCar, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorForward);
			getObjects()->update(kObjectCompartmentE, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getSavePoints()->push(kEntityVerges, kEntityCoudert, kAction168254872);

			setCallback(10);
			setup_makeAnnouncement(kCarGreenSleeping, kPosition_540, "TRA1006");
			break;

		case 10:
			setCallback(11);
			setup_function11();
			break;
		}
		break;

	case kAction168710784:
		getData()->car = kCarGreenSleeping;

		if (!(getEntityData(kEntityPlayer)->car == kCarGreenSleeping))
			getData()->car = kCarRedSleeping;

		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationOutsideCompartment;

		getState()->timeDelta = 3;

		setCallback(8);
		setup_savegame(kSavegameTypeTime, kTimeNone);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Verges, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (ENTITY_PARAM(0, 6)) {
			params->param1 = 1;
			params->param2 = 1;
			params->param3 = 1;
			params->param4 = 1;
			params->param5 = 1;
			params->param6 = 1;

			ENTITY_PARAM(0, 6) = 0;
		}

		if (ENTITY_PARAM(0, 2)) {
			setCallback(1);
			setup_function23();
			break;
		}

label_callback1:
		if (getEntities()->isInBaggageCarEntrance(kEntityPlayer)) {
			setCallback(2);
			setup_baggageCar(false);
			break;
		}

label_callback2:
		if (ENTITY_PARAM(0, 7)) {
			setCallback(3);
			setup_policeSearch();
			break;
		}

label_callback3:
		if (params->param6)
			goto label_callback12;

		if (Entity::timeCheckCallback(kTimeChapter1, params->param7, 4, "TRA1001", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
			break;

label_callback4:
		if (Entity::timeCheckCallback(kTime1089000, params->param8, 5, WRAP_SETUP_FUNCTION(Verges, setup_function12)))
			break;

		params->param8 = 1;

		if (!params->param5) {
			setCallback(5);
			setup_function12();
			break;
		}

label_callback8:
		if (Entity::timeCheckCallback(kTime1107000, CURRENT_PARAM(1, 1), 9, "TRA1001A", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
			break;

label_callback9:
		if (Entity::timeCheckCallback(kTime1134000, CURRENT_PARAM(1, 2), 10, "TRA1002", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
			break;

label_callback10:
		if (Entity::timeCheckCallback(kTime1165500, CURRENT_PARAM(1, 3), 11, "TRA1003", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
			break;

label_callback11:
		if (Entity::timeCheckCallback(kTime1225800, CURRENT_PARAM(1, 4), 12, "TRA1004", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
			break;

label_callback12:
		if (ENTITY_PARAM(0, 5) && !params->param2) {
			setCallback(13);
			setup_talkGendarmes();
			break;
		}

label_callback13:
		if (getInventory()->hasItem(kItemPassengerList) && !params->param3 && (getState()->time < kTime1134000 || getState()->time > kTime1156500)) {
			setCallback(14);
			setup_talkPassengerList();
			break;
		}

label_callback14:
		if (ENTITY_PARAM(0, 3) && !params->param4 && (getState()->time < kTime1134000 || getState()->time > kTime1156500)) {
			setCallback(15);
			setup_talkAboutPassengerList();
			break;
		}

label_callback15:
		if (ENTITY_PARAM(0, 1) && !params->param5) {
			if (getState()->time < kTime1134000 || getState()->time > kTime1156500) {
				setCallback(16);
				setup_askMertensToRelayAugustInvitation();
			}
		}
		break;

	case kActionOpenDoor:
		setCallback(17);
		setup_baggageCar(savepoint.param.intValue < 106 ? true : false);
		break;

	case kActionDefault:
		getData()->car = kCarBaggage;
		getData()->entityPosition = kPosition_5000;
		getData()->location = kLocationOutsideCompartment;

		getEntities()->clearSequences(kEntityVerges);
		getInventory()->setLocationAndProcess(kItem9, kObjectLocation1);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback1;

		case 2:
			goto label_callback2;

		case 3:
			goto label_callback3;

		case 4:
			goto label_callback4;

		case 5:
			setCallback(6);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 6:
			setCallback(7);
			setup_dialog(kEntityMertens, "TRA1202");
			break;

		case 7:
			setCallback(8);
			setup_function11();
			break;

		case 8:
			goto label_callback8;

		case 9:
			goto label_callback9;

		case 10:
			goto label_callback10;

		case 11:
			goto label_callback11;

		case 12:
			goto label_callback12;

		case 13:
			params->param2 = 1;
			goto label_callback13;

		case 14:
			params->param3 = 1;
			goto label_callback14;

		case 15:
			params->param4 = 1;
			goto label_callback15;

		case 16:
			params->param5 = 1;
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Verges, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter2Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityVerges);

		getData()->entityPosition = kPosition_5000;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarBaggage;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObject104, kEntityVerges, kObjectLocationNone, kCursorNormal, kCursorHand);
		getObjects()->update(kObject105, kEntityVerges, kObjectLocationNone, kCursorNormal, kCursorHand);

		ENTITY_PARAM(0, 3) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Verges, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInBaggageCarEntrance(kEntityPlayer)) {
			setCallback(1);
			setup_baggageCar(false);
		}

label_callback_1:
		if (Entity::timeCheckCallback(kTime1818900, params->param1, 2, "Tra2177", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
			break;

label_callback_2:
		if (params->param2 == kTimeInvalid || !getState()->time)
			goto label_callback_6;

		if (getState()->time > kTime1836000) {
			params->param2 = kTimeInvalid;
			setCallback(3);
			setup_function12();
			break;
		}

		if (!getEntities()->isPlayerInCar(kCarRedSleeping) || !params->param2) {
			params->param2 = (uint)getState()->time;

			if (!params->param2) {
				setCallback(3);
				setup_function12();
				break;
			}
		}

		if (params->param2 >= getState()->time) {
label_callback_6:

			if (ENTITY_PARAM(0, 3)) {
				setCallback(7);
				setup_talkAboutPassengerList();
			}

			break;
		}

		params->param2 = kTimeInvalid;
		setCallback(3);
		setup_function12();
		break;

	case kActionOpenDoor:
		setCallback(8);
		setup_baggageCar(savepoint.param.intValue < 106);
		break;

	case kActionDefault:
		getInventory()->setLocationAndProcess(kItem9, kObjectLocation1);
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
			setCallback(4);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 4:
			setCallback(5);
			setup_dialog(kEntityCoudert, "TRA2100");
			break;

		case 5:
			setCallback(6);
			setup_function11();
			break;

		case 6:
			goto label_callback_6;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Verges, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_function33();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityVerges);

		getData()->entityPosition = kPosition_540;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObject104, kEntityVerges, kObjectLocationNone, kCursorNormal, kCursorHand);
		getObjects()->update(kObject105, kEntityVerges, kObjectLocationNone, kCursorNormal, kCursorHand);

		ENTITY_PARAM(0, 3) = 0;
		ENTITY_PARAM(0, 4) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(30, Verges, function30)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function12();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 2:
			setCallback(3);
			setup_dialog(kEntityCoudert, (char *)&params->seq1);
			break;

		case 3:
			setCallback(4);
			setup_function11();
			break;

		case 4:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Verges, talkAboutMax)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function12();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 2:
			setCallback(3);
			setup_dialog(kEntityCoudert, "TRA3015");
			break;

		case 3:
			setCallback(4);
			setup_function11();
			break;

		case 4:
			getProgress().field_48 = 1;
			ENTITY_PARAM(0, 4) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Verges, function32)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime2263500 && !params->param1) {
			params->param1 = 1;
			setCallback(5);
			setup_makeAnnouncement(kCarRedSleeping, kPosition_9460, "TRA3006");
			break;
		}
		break;

	case kActionDefault:
		getObjects()->update(kObject104, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);
		getObjects()->update(kObject105, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);

		if (getEntities()->isInBaggageCar(kEntityPlayer) || getEntities()->isInKitchen(kEntityPlayer)) {
			getAction()->playAnimation(getEntities()->isInBaggageCar(kEntityPlayer) ? kEventVergesBaggageCarOffLimits : kEventVergesCanIHelpYou);
			getSound()->playSound(kEntityPlayer, "BUMP");
			getScenes()->loadSceneFromPosition(kCarRestaurant, 65);
		}

		getScenes()->loadSceneFromItemPosition(kItem9);
		getData()->car = kCarRestaurant;
		getData()->entityPosition = kPosition_5900;

		setCallback(1);
		setup_callbackActionRestaurantOrSalon();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->entityPosition = kPosition_8500;
			getData()->location = kLocationOutsideCompartment;
			getSound()->playSound(kEntityVerges, "TRA3004");

			setCallback(2);
			setup_draw("813DD");
			break;

		case 2:
			if (!getSoundQueue()->isBuffered(kEntityVerges))
				getSound()->playSound(kEntityVerges, "TRA3004");

			getEntities()->drawSequenceRight(kEntityVerges, "813DS");

			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityVerges);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			setCallback(4);
			setup_makeAnnouncement(kCarGreenSleeping, kPosition_540, "TRA3004");
			break;

		case 4:
			getEntities()->clearSequences(kEntityVerges);
			break;

		case 5:
			setCallback(6);
			setup_function11();
			break;

		case 6:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Verges, function33)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_callbackActionRestaurantOrSalon();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_draw("813US");
			break;

		case 2:
			getEntities()->drawSequenceRight(kEntityVerges, "813UD");
			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityVerges);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			getEntities()->clearSequences(kEntityVerges);
			getData()->location = kLocationInsideCompartment;
			getData()->entityPosition = kPosition_5799;

			setCallback(getProgress().field_3C ? 4 : 5);
			setup_playSound(getProgress().field_3C ? "ABB3035A" : "ABB3035");
			break;

		case 4:
			setCallback(5);
			setup_playSound("ABB3035");
			break;

		case 5:
			getSavePoints()->push(kEntityVerges, kEntityAbbot, kAction192054567);

			setCallback(6);
			setup_walkBetweenCars("Tra3010");
			break;

		case 6:
			setup_function34();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(34, Verges, function34)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInBaggageCarEntrance(kEntityPlayer)) {
			setCallback(1);
			setup_baggageCar(false);
			break;
		}

label_callback_1:
		if (ENTITY_PARAM(0, 4)) {
			setCallback(2);
			setup_talkAboutMax();
			break;
		}

label_callback_2:
		if (ENTITY_PARAM(0, 3)) {
			setCallback(3);
			setup_talkAboutPassengerList();
			break;
		}

label_callback_3:
		if (Entity::timeCheckCallback(kTime1971000, params->param1, 4, "Tra3001", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
			break;

label_callback_4:
		if (Entity::timeCheckCallback(kTime1998000, params->param2, 5, "Tra3010a", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
			break;

label_callback_5:
		if (Entity::timeCheckCallback(kTime2016000, params->param3, 6, WRAP_SETUP_FUNCTION(Verges, setup_organizeConcertInvitations)))
			break;

label_callback_6:
		if (Entity::timeCheckCallback(kTime2070000, params->param4, 7, "Tra3002", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
			break;

label_callback_7:
		if (Entity::timeCheckCallback(kTime2142000, params->param5, 8, "Tra3003", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
			break;

label_callback_8:
		if (Entity::timeCheckCallback(kTime2173500, params->param6, 9, "Tra3012", WRAP_SETUP_FUNCTION_S(Verges, setup_function30)))
			break;

label_callback_9:
		Entity::timeCheckCallback(kTime2218500, params->param7, 10, WRAP_SETUP_FUNCTION(Verges, setup_function32));
		break;

	case kActionOpenDoor:
		setCallback(11);
		setup_baggageCar(savepoint.param.intValue < 106);
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
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(35, Verges, organizeConcertInvitations)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function12();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 2:
			setCallback(3);
			setup_dialog(kEntityMertens, "Tra3011A");
			break;

		case 3:
			getSavePoints()->push(kEntityVerges, kEntityCoudert, kAction188570113);

			setCallback(4);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 4:
			setCallback(5);
			setup_dialog(kEntityMertens, "Tra3011");
			break;

		case 5:
			getSavePoints()->push(kEntityVerges, kEntityMertens, kAction188635520);

			setCallback(6);
			setup_function11();
			break;

		case 6:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(36, Verges, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter4Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityVerges);

		getData()->entityPosition = kPosition_5000;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarBaggage;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObject104, kEntityVerges, kObjectLocationNone, kCursorNormal, kCursorHand);
		getObjects()->update(kObject105, kEntityVerges, kObjectLocationNone, kCursorNormal, kCursorHand);

		ENTITY_PARAM(0, 3) = 0;
		ENTITY_PARAM(0, 6) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(37, Verges, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInBaggageCarEntrance(kEntityPlayer)) {
			setCallback(1);
			setup_baggageCar(false);
			break;
		}

label_callback_1:
		if (ENTITY_PARAM(0, 6)) {
			if (ENTITY_PARAM(0, 3)) {
				setCallback(2);
				setup_talkAboutPassengerList();
				break;
			}

label_callback_2:
			if (Entity::timeCheckCallback(kTime2349000, params->param1, 3, "Tra1001", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
				break;

label_callback_3:
			if (Entity::timeCheckCallback(kTime2378700, params->param2, 4, "Tra4001", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
				break;

label_callback_4:
			if (Entity::timeCheckCallback(kTime2403000, params->param3, 5, "Tra1001A", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
				break;

label_callback_5:
			if (Entity::timeCheckCallback(kTime2414700, params->param4, 6, "Tra4002", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
				break;

label_callback_6:
			if (Entity::timeCheckCallback(kTime2484000, params->param5, 7, "Tra4003", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
				break;

label_callback_7:
			if (Entity::timeCheckCallback(kTime2511000, params->param6, 8, "Tra4004", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars)))
				break;
		}

label_callback_8:
		Entity::timeCheckCallback(kTime2538000, params->param7, 9, "Tra4005", WRAP_SETUP_FUNCTION_S(Verges, setup_walkBetweenCars));
		break;

	case kActionOpenDoor:
		setCallback(10);
		setup_baggageCar(savepoint.param.intValue < 106);
		break;

	case kActionDefault:
		getData()->car = kCarBaggage;
		getData()->entityPosition = kPosition_5000;
		getData()->location = kLocationOutsideCompartment;

		getInventory()->setLocationAndProcess(kItem9, kObjectLocation1);
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
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(38, Verges, resetState)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getObjects()->update(kObject104, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);
		getObjects()->update(kObject105, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);
		getScenes()->loadSceneFromItemPosition(kItem9);
		getEntities()->clearSequences(kEntityVerges);

		getData()->entityPosition = kPosition_6469;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->clearSequences(kEntityVerges);
			setCallback(2);
			setup_updateFromTime(1800);
			break;

		case 2:
			setCallback(3);
			setup_function11();
			break;

		case 3:
			setup_chapter4Handler();
			break;
		}
		break;

	case kAction125233040:
		getData()->entityPosition = kPosition_5790;

		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_540);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(39, Verges, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityVerges);

		getData()->entityPosition = kPosition_3650;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObject104, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);
		getObjects()->update(kObject105, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(40, Verges, chapter5Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInSalon(kEntityPlayer) && !getSoundQueue()->isBuffered(kEntityVerges))
			getSound()->playSound(kEntityVerges, "WAT5000");
		break;

	case kActionOpenDoor:
		if (getSoundQueue()->isBuffered(kEntityVerges))
			getSoundQueue()->processEntry(kEntityVerges);

		if (getSoundQueue()->isBuffered("MUS050"))
			getSoundQueue()->processEntry("MUS050");

		getObjects()->update(kObject65, kEntityPlayer, kObjectLocationNone, kCursorNormal, kCursorForward);

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventCathFreePassengers);
		break;

	case kActionDefault:
		getScenes()->loadSceneFromItemPosition(kItem9);
		getObjects()->update(kObject65, kEntityVerges, kObjectLocation1, kCursorNormal, kCursorForward);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventCathFreePassengers);
			getSavePoints()->pushAll(kEntityVerges, kActionProceedChapter5);
			getScenes()->loadSceneFromPosition(kCarRedSleeping, 40);
			setup_askPassengersToStayInCompartments();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(41, Verges, askPassengersToStayInCompartments)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getObjects()->updateLocation2(kObjectRestaurantCar, kObjectLocation3);
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_9460;
		getData()->location = kLocationInsideCompartment;

		setCallback(1);
		setup_makeAnnouncement(kCarRedSleeping, kPosition_2000, "Tra5001");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kEntityVerges, "620E");
			// Fallback to next case

		case 2:
			if (getSoundQueue()->isBuffered(kEntityVerges)) {
				setCallback(2);
				setup_updateFromTime(225);
			} else {
				setCallback(3);
				setup_playSound("Con5001");
			}
			break;

		case 3:
			getSavePoints()->push(kEntityVerges, kEntityCoudert, kAction155991520);

			setCallback(4);
			setup_updateEntity(kCarBaggageRear, kPosition_9460);
			break;

		case 4:
			setup_end();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(42, Verges, end)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityVerges);
IMPLEMENT_FUNCTION_END


//////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////
void Verges::talk(const SavePoint &savepoint, const char *sound1, const char *sound2) {
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_function12();
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 2:
			setCallback(3);
			setup_dialog(kEntityCoudert, sound1);
			break;

		case 3:
			setCallback(4);
			setup_updateEntity(kCarGreenSleeping, kPosition_2000);
			break;

		case 4:
			setCallback(5);
			setup_dialog(kEntityMertens, sound2);
			break;

		case 5:
			setup_function11();
			break;

		case 6:
			callbackAction();
			break;
		}
		break;
	}
}

} // End of namespace LastExpress
