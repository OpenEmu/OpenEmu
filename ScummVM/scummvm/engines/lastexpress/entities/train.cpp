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

#include "lastexpress/entities/train.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Train::Train(LastExpressEngine *engine) : Entity(engine, kEntityTrain) {
	ADD_CALLBACK_FUNCTION(Train, savegame);
	ADD_CALLBACK_FUNCTION(Train, chapter1);
	ADD_CALLBACK_FUNCTION(Train, chapter2);
	ADD_CALLBACK_FUNCTION(Train, chapter3);
	ADD_CALLBACK_FUNCTION(Train, chapter4);
	ADD_CALLBACK_FUNCTION(Train, chapter5);
	ADD_CALLBACK_FUNCTION(Train, harem);
	ADD_CALLBACK_FUNCTION(Train, process);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(1, Train, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(2, Train, chapter1)
	if (savepoint.action == kActionDefault)
		setup_process();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(3, Train, chapter2)
	if (savepoint.action == kActionDefault)
		setup_process();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(4, Train, chapter3)
	if (savepoint.action == kActionDefault)
		setup_process();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(5, Train, chapter4)
	if (savepoint.action == kActionDefault)
		setup_process();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Train, chapter5)
	if (savepoint.action == kActionDefault)
		setup_process();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(7, Train, harem, ObjectIndex, uint32)
	if (savepoint.action != kActionDefault)
		return;

	switch (params->param1) {
	default:
		error("[Train::harem] Invalid value for parameter 1: %d", params->param1);
		break;

	case kObjectCompartment5:
		params->param3 = kPosition_4840;
		break;

	case kObjectCompartment6:
		params->param3 = kPosition_4070;
		break;

	case kObjectCompartment7:
		params->param3 = kPosition_3050;
		break;

	case kObjectCompartment8:
		params->param3 = kPosition_2740;
		break;
	}

	params->param4 = getEntities()->isInsideCompartment(kEntityAlouan, kCarGreenSleeping, (EntityPosition)params->param3);
	params->param5 = (ENTITY_PARAM(0, 7) - params->param3) < 1 ? true : false;
	params->param6 = getEntities()->isInsideCompartment(kEntityYasmin, kCarGreenSleeping, (EntityPosition)params->param3);
	params->param7 = getEntities()->isInsideCompartment(kEntityHadija, kCarGreenSleeping, (EntityPosition)params->param3);

	getObjects()->update((ObjectIndex)params->param1, kEntityTrain, kObjectLocation3, kCursorNormal, kCursorNormal);

	// Knock / closed door sound
	getSound()->playSound(kEntityTables5, (params->param2 == 8) ? "LIB012" : "LIB013", kFlagDefault);

	if (params->param4 && params->param5) {

		ENTITY_PARAM(0, 5)++;

		switch (ENTITY_PARAM(0, 5)) {
		default:
			params->param8 = 1;
			break;

		case 1:
			getSound()->playSound(kEntityTables5, "Har1014", kFlagDefault, 15);
			break;

		case 2:
			getSound()->playSound(kEntityTables5, "Har1013", kFlagDefault, 15);
			getSound()->playSound(kEntityTables5, "Har1016", kFlagDefault, 150);
			break;

		case 3:
			getSound()->playSound(kEntityTables5, "Har1015A", kFlagDefault, 15);
			getSound()->playSound(kEntityTables5, "Har1015", kFlagDefault, 150);
			break;
		}

		// Update progress
		getProgress().field_DC = 1;
		getProgress().field_E0 = 1;

		handleCompartmentAction();

		// Done with it!
		return;
	}

	if (params->param6 && params->param7) {

		ENTITY_PARAM(0, 6)++;

		switch(ENTITY_PARAM(0, 6)) {
		default:
			params->param8 = 1;
			break;

		case 1:
			getSound()->playSound(kEntityTables5, "Har1014", kFlagDefault, 15);
			break;

		case 2:
			getSound()->playSound(kEntityTables5, "Har1013", kFlagDefault, 15);
			break;

		case 3:
			getSound()->playSound(kEntityTables5, "Har1013A", kFlagDefault, 15);
			break;
		}

		handleCompartmentAction();
		return;
	}

	if (!params->param5) {

		if (params->param6) {
			ENTITY_PARAM(0, 3)++;

			switch(ENTITY_PARAM(0, 3)) {
			default:
				params->param8 = 1;
				break;

			case 1:
				getSound()->playSound(kEntityTables5, "Har1012", kFlagDefault, 15);
				break;

			case 2:
				getSound()->playSound(kEntityTables5, "Har1012A", kFlagDefault, 15);
				break;
			}

			handleCompartmentAction();
			return;
		} else {

			if (params->param4) {
				ENTITY_PARAM(0, 1)++;

				if (ENTITY_PARAM(0, 1) <= 1)
					getSound()->playSound(kEntityTables5, "Har1014", kFlagDefault, 15);
				else
					params->param8 = 1;

				getProgress().field_DC = 1;

				handleCompartmentAction();
				return;
			}

			if (params->param7) {
				ENTITY_PARAM(0, 4)++;

				if (ENTITY_PARAM(0, 4) <= 1) {
					getSound()->playSound(kEntityTables5, "Har1011", kFlagDefault, 15);
					handleCompartmentAction();
					return;
				}
			}
		}

		params->param8 = 1;
		handleCompartmentAction();
		return;
	}

	ENTITY_PARAM(0, 2) += 1;

	switch (ENTITY_PARAM(0, 2)) {
	default:
		params->param8 = 1;
		break;

	case 1:
		getSound()->playSound(kEntityTables5, "Har1013", kFlagDefault, 15);
		break;

	case 2:
		getSound()->playSound(kEntityTables5, "Har1013A", kFlagDefault, 15);
		break;
	}

	getProgress().field_E0 = 1;

	handleCompartmentAction();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Train, process)
	EntityData::EntityParametersIIIS *params1 = (EntityData::EntityParametersIIIS*)_data->getCurrentParameters(1);

	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		// Play smoke animation
		if ((getEntities()->isPlayerInCar(kCarGreenSleeping) || getEntities()->isPlayerInCar(kCarRedSleeping))
		  && params->param4 && !params->param5) {

			params->param4 -= 1;

			if (!params->param4 && getProgress().jacket == kJacketGreen) {

				getAction()->playAnimation(isNight() ? kEventCathSmokeNight : kEventCathSmokeDay);
				params->param5 = 1;
				getScenes()->processScene();
			}
		}

		if (params->param6) {
			if (!Entity::updateParameter(params1->param7, getState()->time, 900))
				goto label_process;

			getScenes()->loadSceneFromPosition(kCarRestaurant, 58);
		}

		params1->param7 = 0;

label_process:
		if (params->param7) {
			if (!params1->param8) {
				params1->param8 = (uint)(getState()->time + 4500);

				if (!params1->param8)
					params->param7 = 0;
			}

			if (params1->param8 && params1->param8 < getState()->time) {
				params->param7 = 0;
				params1->param8 = 0;
			}
		}

		// Update object
		if (ENTITY_PARAM(0, 8) && !getSoundQueue()->isBuffered(kEntityTables5)) {
			getObjects()->update((ObjectIndex)ENTITY_PARAM(0, 8), getObjects()->get((ObjectIndex)ENTITY_PARAM(0, 8)).entity, kObjectLocation3, kCursorHandKnock, kCursorHand);
			ENTITY_PARAM(0, 8) = 0;
		}

		// Play clock sound
		if (params->param6 && !getSoundQueue()->isBuffered("ZFX1001", true))
			getSound()->playSound(kEntityPlayer, "ZFX1001");

		break;

	case kActionKnock:
	case kActionOpenDoor: {
		// Handle opening harem compartments
		ObjectIndex compartment = (ObjectIndex)savepoint.param.intValue;
		if (compartment == kObjectCompartment5 || compartment == kObjectCompartment6 || compartment == kObjectCompartment7 || compartment == kObjectCompartment8) {
			setCallback(savepoint.action == kActionKnock ? 3 : 4);
			setup_harem(compartment, savepoint.action);
		}
		break;
	}

	case kActionDefault:
		params->param3 = 1;
		if (getProgress().chapter < kChapter5) {
			getObjects()->update(kObjectCompartment5, kEntityTrain, kObjectLocation3, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectCompartment6, kEntityTrain, kObjectLocation3, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectCompartment7, kEntityTrain, kObjectLocation3, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectCompartment8, kEntityTrain, kObjectLocation3, kCursorHandKnock, kCursorHand);
		}
		getData()->entityPosition = kPosition_30000;
		break;

	case kActionDrawScene:
		getData()->car = getEntityData(kEntityPlayer)->car;

		// Play clock sound
		if (getEntities()->isPlayerPosition(kCarRestaurant, 81)) {
			params->param6 = 1;
			if (!getSoundQueue()->isBuffered("ZFX1001"))
				getSound()->playSound(kEntityPlayer, "ZFX1001");
		} else {
			params->param6 = 0;
			if (getSoundQueue()->isBuffered("ZFX1001", true))
				getSoundQueue()->removeFromQueue("ZFX1001");
		}

		// Draw moving background behind windows
		if (params->param3) {
			if (getEntityData(kEntityPlayer)->car != (CarIndex)params->param1 || isNight() != (bool)(params->param2)) {
				switch (getEntityData(kEntityPlayer)->car) {
				default:
					getEntities()->clearSequences(kEntityTrain);
					break;

				case kCarBaggageRear:
				case kCarBaggage:
					if (getProgress().isNightTime)
						getEntities()->drawSequenceLeft(kEntityTrain, "B1WNM");
					else
						getEntities()->drawSequenceLeft(kEntityTrain, isNight() ? "B1WNN" : "B1WND");
					break;

				case kCarGreenSleeping:
				case kCarRedSleeping:
					if (getProgress().isNightTime)
						getEntities()->drawSequenceLeft(kEntityTrain, "S1WNM");
					else
						getEntities()->drawSequenceLeft(kEntityTrain, isNight() ? "S1WNN" : "S1WND");
					break;

				case kCarRestaurant:
					getEntities()->drawSequenceLeft(kEntityTrain, isNight() ? "RCWNN" : "RCWND");
					break;
				}

				// Set parameters so we do not get called twice
				params->param1 = getEntityData(kEntityPlayer)->car;
				params->param2 = isNight();
			}
		}

		if (!params->param5) {
			params->param4 = 2700;	// this is the sound file name
			params->param5 = 0;
		}

		if (getProgress().jacket == kJacketBlood) {
			if (getEntities()->isPlayerPosition(kCarRedSleeping, 18)) {
				setCallback(1);
				setup_savegame(kSavegameTypeEvent, kEventMertensBloodJacket);
				break;
			}

			if (getEntities()->isPlayerPosition(kCarGreenSleeping, 22)) {
				setCallback(2);
				setup_savegame(kSavegameTypeEvent, kEventMertensBloodJacket);
				break;
			}
		}

		resetParam8();
		break;


	case kActionCallback: {
		int action = getCallback();
		switch(action) {
		default:
			break;

		case 1:
		case 2:
			getAction()->playAnimation(action == 1 ? kEventCoudertBloodJacket : kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);
			resetParam8();
			break;

		case 5:
			getAction()->playAnimation(kEventLocomotiveConductorsDiscovered);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverPolice2, true);
			break;

		case 6:
			getAction()->playAnimation(kEventCathBreakCeiling);
			getObjects()->update(kObjectCeiling, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getScenes()->processScene();
			break;

		case 7:
			getAction()->playAnimation(kEventCathJumpDownCeiling);
			getScenes()->loadSceneFromPosition(kCarKronos, 89);
			break;

		case 8:
			getAction()->playAnimation(kEventCloseMatchbox);
			getScenes()->loadSceneFromPosition(kCarRestaurant, 51);
			break;
		}
		break;
	}

	case kAction191070912:
		ENTITY_PARAM(0, 7) = savepoint.param.intValue;
		break;

	case kActionTrainStopRunning:
		params->param3 = 0;
		getEntities()->clearSequences(kEntityTrain);
		break;

	case kActionCatchBeetle:
		setCallback(8);
		setup_savegame(kSavegameTypeEvent, kEventCloseMatchbox);
		break;

	case kAction203339360:
		if (params->param7) {
			setCallback(5);
			setup_savegame(kSavegameTypeEvent, kEventLocomotiveConductorsDiscovered);
		} else {
			params->param7 = 1;
			getAction()->playAnimation(kEventLocomotiveConductorsLook);
			getScenes()->loadSceneFromPosition(kCarCoalTender, 2);
		}
		break;

	case kActionTrainStartRunning:
		if (!params->param3) {
			params->param1 = 0;
			params->param3 = 1;
			getSavePoints()->push(kEntityTrain, kEntityTrain, kActionDrawScene);
		}
		break;

	case kAction203863200:
		if (!strcmp(savepoint.param.charValue, "")) {
			params->param8 = 1;
			strcpy((char *)&params1->seq, savepoint.param.charValue);	// this is the sound file name
		}
		break;

	case kAction222746496:
		switch(savepoint.param.intValue) {
		default:
			break;

		case kObjectCompartment1:
		case kObjectCompartment2:
		case kObjectCompartmentA:
		case kObjectCompartmentB:
			params1->param1 = (savepoint.param.intValue == kObjectCompartment1 || savepoint.param.intValue == kObjectCompartment2) ? kCarGreenSleeping : kCarRedSleeping;
			params1->param2 = (savepoint.param.intValue == kObjectCompartment1 || savepoint.param.intValue == kObjectCompartmentA) ? kPosition_8200 : kPosition_7500;
			params1->param3 = kPosition_7850;
			break;

		case kObjectCompartment3:
		case kObjectCompartment4:
		case kObjectCompartmentC:
		case kObjectCompartmentD:
			params1->param1 = (savepoint.param.intValue == kObjectCompartment1 || savepoint.param.intValue == kObjectCompartment2) ? kCarGreenSleeping : kCarRedSleeping;
			params1->param2 = (savepoint.param.intValue == kObjectCompartment3 || savepoint.param.intValue == kObjectCompartmentC) ? kPosition_6470 : kPosition_5790;
			params1->param3 = kPosition_6130;
			break;

		case kObjectCompartment5:
		case kObjectCompartment6:
		case kObjectCompartmentE:
		case kObjectCompartmentF:
			params1->param1 = (savepoint.param.intValue == kObjectCompartment1 || savepoint.param.intValue == kObjectCompartment2) ? kCarGreenSleeping : kCarRedSleeping;
			params1->param2 = (savepoint.param.intValue == kObjectCompartment5 || savepoint.param.intValue == kObjectCompartmentE) ? kPosition_4840 : kPosition_4070;
			params1->param3 = kPosition_4455;
			break;

		case kObjectCompartment7:
		case kObjectCompartment8:
		case kObjectCompartmentG:
		case kObjectCompartmentH:
			params1->param1 = (savepoint.param.intValue == kObjectCompartment1 || savepoint.param.intValue == kObjectCompartment2) ? kCarGreenSleeping : kCarRedSleeping;
			params1->param2 = (savepoint.param.intValue == kObjectCompartment7 || savepoint.param.intValue == kObjectCompartmentG) ? kPosition_3050 : kPosition_2740;
			params1->param3 = kPositionNone;
			break;
		}
		break;

	case kActionBreakCeiling:
		setCallback(6);
		setup_savegame(kSavegameTypeEvent, kEventCathBreakCeiling);
		break;

	case kActionJumpDownCeiling:
		setCallback(7);
		setup_savegame(kSavegameTypeEvent, kEventCathJumpDownCeiling);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////
void Train::handleCompartmentAction() {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)

	if (params->param8)
		getSavePoints()->push(kEntityTrain, kEntityMahmud, kAction290410610, params->param1);

	getAction()->handleOtherCompartment((ObjectIndex)params->param1, false, !params->param8);

	ENTITY_PARAM(0, 8) = params->param1;

	callbackAction();
}

//////////////////////////////////////////////////////////////////////////
void Train::resetParam8() {
	EXPOSE_PARAMS(EntityData::EntityParametersIIII)
	EntityData::EntityParametersIIIS *params1 = (EntityData::EntityParametersIIIS*)_data->getCurrentParameters(1);

	if (params->param8
	 && !getEntities()->isInsideCompartment(kEntityPlayer, (CarIndex)params1->param1, (EntityPosition)params1->param2)
	 && !getEntities()->isInsideCompartment(kEntityPlayer, (CarIndex)params1->param1, (EntityPosition)params1->param3)) {

		if (getSoundQueue()->isBuffered((const char *)&params1->seq))
			getSoundQueue()->processEntry((const char *)&params1->seq);

		params->param8 = 0;
	}
}

} // End of namespace LastExpress
