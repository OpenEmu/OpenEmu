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

#include "lastexpress/entities/gendarmes.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Gendarmes::Gendarmes(LastExpressEngine *engine) : Entity(engine, kEntityGendarmes) {
	ADD_CALLBACK_FUNCTION(Gendarmes, reset);
	ADD_CALLBACK_FUNCTION(Gendarmes, chapter1);
	ADD_CALLBACK_FUNCTION(Gendarmes, arrestDraw);
	ADD_CALLBACK_FUNCTION(Gendarmes, arrestPlaysound);
	ADD_CALLBACK_FUNCTION(Gendarmes, arrestPlaysound16);
	ADD_CALLBACK_FUNCTION(Gendarmes, arrestCallback);
	ADD_CALLBACK_FUNCTION(Gendarmes, savegame);
	ADD_CALLBACK_FUNCTION(Gendarmes, arrestUpdateEntity);
	ADD_CALLBACK_FUNCTION(Gendarmes, function9);
	ADD_CALLBACK_FUNCTION(Gendarmes, function10);
	ADD_CALLBACK_FUNCTION(Gendarmes, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Gendarmes, function12);
	ADD_CALLBACK_FUNCTION(Gendarmes, function13);
	ADD_CALLBACK_FUNCTION(Gendarmes, chapter2);
	ADD_CALLBACK_FUNCTION(Gendarmes, chapter3);
	ADD_CALLBACK_FUNCTION(Gendarmes, chapter4);
	ADD_CALLBACK_FUNCTION(Gendarmes, chapter5);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Gendarmes, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(2, Gendarmes, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Gendarmes, setup_chapter1Handler));
		break;

	case kActionDefault:
		getData()->car = kCarNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, Gendarmes, arrestDraw)
	arrest(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(4, Gendarmes, arrestPlaysound)
	arrest(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(5, Gendarmes, arrestPlaysound16)
	arrest(savepoint, true, kFlagDefault);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(6, Gendarmes, arrestCallback, uint32)
	arrest(savepoint, true, kFlagInvalid, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(7, Gendarmes, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(8, Gendarmes, arrestUpdateEntity, CarIndex, EntityPosition)
	arrest(savepoint, true, kFlagInvalid, false, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_IISS(9, Gendarmes, function9, CarIndex, EntityPosition)
	EntityData::EntityParametersSSS *parameters1 = (EntityData::EntityParametersSSS*)_data->getCurrentParameters(1);
	EntityData::EntityParametersISII *parameters2 = (EntityData::EntityParametersISII*)_data->getCurrentParameters(2);

	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (params->param2 <= kPosition_3050) {
			if (params->param2 != kPosition_3050) {
				if (params->param2 == kPosition_2740)
					parameters2->param5 = kObjectCompartment8;
			} else {
				parameters2->param5 = kObjectCompartment7;
				parameters2->param6 = true;
			}
		} else if (params->param2 <= kPosition_4840) {
			if (params->param2 != kPosition_4840) {
				if (params->param2 == kPosition_4070) {
					parameters2->param5 = kObjectCompartment6;
					parameters2->param7 = kPosition_4455;
				}
			} else {
				parameters2->param5 = kObjectCompartment5;
				parameters2->param6 = true;
				parameters2->param7 = kPosition_4455;
			}
		} else if (params->param2 <= kPosition_6470) {
			if (params->param2 != kPosition_6470) {
				if (params->param2 == kPosition_5790) {
					parameters2->param5 = kObjectCompartment4;
					parameters2->param7 = kPosition_6130;
				}
			} else {
				parameters2->param5 = kObjectCompartment3;
				parameters2->param6 = true;
				parameters2->param7 = kPosition_6130;
			}
		} else if (params->param2 != kPosition_7500) {
			if (params->param2 == kPosition_8200) {
				parameters2->param5 = kObjectCompartment1;
				parameters2->param6 = true;
				parameters2->param7 = kPosition_7850;
			}
		} else {
			parameters2->param5 = kObjectCompartment2;
			parameters2->param7 = kPosition_7850;
		}

		if (params->param1 == kCarBaggageRear)
			parameters2->param5 += 31; // Switch to next compartment car

		if (parameters2->param6) {
			strcpy((char *)&parameters1->seq1, "632A");
			strcpy((char *)&parameters1->seq2, "632B");
			strcpy((char *)&parameters1->seq3, "632C");
		} else {
			strcpy((char *)&parameters1->seq1, "632D");
			strcpy((char *)&parameters1->seq2, "632E");
			strcpy((char *)&parameters1->seq3, "632F");
		}

		strcat((char *)&parameters1->seq1, (char *)&params->seq1);
		strcat((char *)&parameters1->seq2, (char *)&params->seq1);
		strcat((char *)&parameters1->seq3, (char *)&params->seq1);

		if ((getEntities()->isInsideCompartment(kEntityPlayer, (CarIndex)params->param1, (EntityPosition)params->param2)
		  || getEntities()->isInsideCompartment(kEntityPlayer, (CarIndex)params->param1, (EntityPosition)parameters2->param7)
		  || (params->param1 == kCarGreenSleeping && params->param2 == kPosition_8200 && getEntities()->isOutsideAlexeiWindow()))
		 && !getEntities()->isInsideCompartment(kEntityPlayer, kCarRedSleeping, kPosition_7850)) {
			setCallback(1);
			setup_function10((CarIndex)params->param1, (EntityPosition)params->param2, (ObjectIndex)parameters2->param5);
		} else {
			getEntities()->drawSequenceLeft(kEntityGendarmes, (char *)&parameters1->seq1);
			getEntities()->enterCompartment(kEntityGendarmes, (ObjectIndex)CURRENT_PARAM(2, 5));

			setCallback(parameters2->param6 ? 2 : 3);
			setup_arrestPlaysound(parameters2->param6 ? "POL1044A" : "POL1044B");
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			callbackAction();
			break;

		case 2:
		case 3:
			getEntities()->drawSequenceLeft(kEntityGendarmes, (char *)&parameters1->seq2);
			if (getEntities()->isNobodyInCompartment((CarIndex)params->param1, (EntityPosition)params->param2) || !strcmp(params->seq2, "NODIALOG")) {
				setCallback(4);
				setup_arrestCallback(150);
			} else {
				char *arrestSound = (char *)&parameters2->seq;
				strcpy(arrestSound, "POL1045");
				strcat(arrestSound, (char *)&params->seq2);

				setCallback(5);
				setup_arrestPlaysound(arrestSound);
			}
			break;

		case 4:
		case 5:
			if (!getEntities()->isNobodyInCompartment((CarIndex)params->param1, (EntityPosition)params->param2) && strcmp(params->seq2, "NODIALOG")) {
				char *arrestSound = (char *)&parameters2->seq;
				strcpy(arrestSound, "POL1043");
				strcat(arrestSound, (char *)&params->seq2);

				getSound()->playSound(kEntityGendarmes, arrestSound, kFlagInvalid, 30);
			}

			getData()->location = kLocationInsideCompartment;

			setCallback(6);
			setup_arrestDraw((char *)&parameters1->seq3);
			break;

		case 6:
			getData()->location = kLocationOutsideCompartment;
			getEntities()->exitCompartment(kEntityGendarmes, (ObjectIndex)parameters2->param5);
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_III(10, Gendarmes, function10, CarIndex, EntityPosition, ObjectIndex)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!params->param5 || getState()->timeTicks > (uint32)params->param5) {
			if (!params->param5)
				params->param5 = getState()->timeTicks + 75;

			if (!getEntities()->isOutsideAlexeiWindow() && getObjects()->get((ObjectIndex)params->param3).location != kObjectLocation1) {
				setCallback(2);
				setup_savegame(kSavegameTypeEvent, kEventGendarmesArrestation);
				break;
			}
		}

		if (!params->param6)
			params->param6 = getState()->timeTicks + 150;

		if (params->param6 == 0 || getState()->timeTicks > (uint32)params->param6) {
			params->param6 = kTimeInvalid;

			getSound()->playSound(kEntityGendarmes, "POL1046A", kFlagDefault);
		}

		if (!Entity::updateParameter(params->param7, getState()->timeTicks, 300))
			break;

		if (!params->param4 && getEntities()->isOutsideAlexeiWindow()) {
			getObjects()->update((ObjectIndex)params->param3, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			callbackAction();
		} else {
			if (getEntities()->isOutsideAlexeiWindow())
				getScenes()->loadSceneFromPosition(kCarGreenSleeping, 49);

			getSound()->playSound(kEntityGendarmes, "LIB017", kFlagDefault);

			setCallback(getProgress().jacket == kJacketBlood ? 3 : 4);
			setup_savegame(kSavegameTypeEvent, getProgress().jacket == kJacketBlood ? kEventMertensBloodJacket : kEventGendarmesArrestation);
		}
		break;

	case kActionKnock:
		getObjects()->update((ObjectIndex)params->param3, kEntityGendarmes, getObjects()->get((ObjectIndex)params->param3).location, kCursorNormal, kCursorNormal);

		setCallback(5);
		setup_arrestPlaysound16("POL1046B");
		break;

	case kActionOpenDoor:
		setCallback(6);
		setup_savegame(kSavegameTypeEvent, kEventGendarmesArrestation);
		break;

	case kActionDefault:
		getObjects()->update((ObjectIndex)params->param3, kEntityGendarmes, getObjects()->get((ObjectIndex)params->param3).location, kCursorNormal, kCursorNormal);

		setCallback(1);
		setup_arrestPlaysound16("POL1046");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update((ObjectIndex)params->param3, kEntityGendarmes, getObjects()->get((ObjectIndex)params->param3).location, kCursorTalk, kCursorNormal);
			break;

		case 2:
			getSound()->playSound(kEntityGendarmes, "LIB014", kFlagDefault);
			getAction()->playAnimation(kEventGendarmesArrestation);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverPolice1, true);
			break;

		case 3:
			getAction()->playAnimation((params->param1 < kCarRedSleeping) ? kEventMertensBloodJacket : kEventCoudertBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverBloodJacket, true);

			getObjects()->update((ObjectIndex)params->param3, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			callbackAction();
			break;

		case 4:
			getAction()->playAnimation(kEventGendarmesArrestation);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverPolice1, true);

			getObjects()->update((ObjectIndex)params->param3, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			callbackAction();
			break;

		case 5:
			getObjects()->update((ObjectIndex)params->param3, kEntityGendarmes, getObjects()->get((ObjectIndex)params->param3).location, kCursorNormal, kCursorHand);
			break;

		case 6:
			getSound()->playSound(kEntityGendarmes, "LIB014", kFlagDefault);
			getAction()->playAnimation(kEventGendarmesArrestation);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverPolice1, true);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Gendarmes, chapter1Handler)
	if (savepoint.action == kAction169499649) {
		getSavePoints()->push(kEntityGendarmes, kEntityMertens, kAction190082817);
		setup_function12();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Gendarmes, function12)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_540;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;

		getProgress().field_14 = 29;

		setCallback(1);
		setup_arrestUpdateEntity(kCarGreenSleeping, kPosition_5540);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function9(kCarGreenSleeping, kPosition_5790, "d", "A");
			break;

		case 2:
			setCallback(3);
			setup_arrestUpdateEntity(kCarGreenSleeping, kPosition_6220);
			break;

		case 3:
			setCallback(4);
			setup_function9(kCarGreenSleeping, kPosition_6470, "c", "B");
			break;

		case 4:
			setCallback(5);
			setup_arrestUpdateEntity(kCarGreenSleeping, kPosition_7250);
			break;

		case 5:
			setCallback(6);
			setup_function9(kCarGreenSleeping, kPosition_7500, "b", "C");
			break;

		case 6:
			setCallback(7);
			setup_arrestUpdateEntity(kCarGreenSleeping, kPosition_7950);
			break;

		case 7:
			setCallback(8);
			setup_function9(kCarGreenSleeping, kPosition_8200, "a", "NODIALOG");
			break;

		case 8:
			setCallback(9);
			setup_arrestUpdateEntity(kCarGreenSleeping, kPosition_9460);
			break;

		case 9:
			if (getEntityData(kEntityPlayer)->car == kCarGreenSleeping) {
				getProgress().field_14 = 0;
				getEntities()->clearSequences(kEntityGendarmes);
				getSavePoints()->push(kEntityGendarmes, kEntityVerges, kAction168710784);
				setup_function13();
				break;
			}

			setCallback(10);
			setup_arrestUpdateEntity(kCarRedSleeping, kPosition_2490);
			break;

		case 10:
			setCallback(11);
			setup_function9(kCarRedSleeping, kPosition_2740, "h", "NODIALOG");
			break;

		case 11:
			setCallback(12);
			setup_arrestUpdateEntity(kCarRedSleeping, kPosition_3820);
			break;

		case 12:
			setCallback(13);
			setup_function9(kCarRedSleeping, kPosition_4070, "f", "E");
			break;

		case 13:
			setCallback(14);
			setup_arrestUpdateEntity(kCarRedSleeping, kPosition_4590);
			break;

		case 14:
			setCallback(15);
			setup_function9(kCarRedSleeping, kPosition_4840, "e", "F");
			break;

		case 15:
			setCallback(16);
			setup_arrestUpdateEntity(kCarRedSleeping, kPosition_5540);
			break;

		case 16:
			setCallback(17);
			setup_function9(kCarRedSleeping, kPosition_5790, "d", "G");
			break;

		case 17:
			setCallback(18);
			setup_arrestUpdateEntity(kCarRedSleeping, kPosition_6220);
			break;

		case 18:
			setCallback(19);
			setup_function9(kCarRedSleeping, kPosition_6470, "c", "H");
			break;

		case 19:
			setCallback(20);
			setup_arrestUpdateEntity(kCarRedSleeping, kPosition_7250);
			break;

		case 20:
			setCallback(21);
			setup_function9(kCarRedSleeping, kPosition_7500, "b", "J");
			break;

		case 21:
			setCallback(22);
			setup_arrestUpdateEntity(kCarRedSleeping, kPosition_7950);
			break;

		case 22:
			setCallback(23);
			setup_function9(kCarRedSleeping, kPosition_8200, "a", "NODIALOG");
			break;

		case 23:
			setCallback(24);
			setup_arrestUpdateEntity(kCarRedSleeping, kPosition_9460);
			break;

		case 24:
			getProgress().field_14 = 0;
			getEntities()->clearSequences(kEntityGendarmes);
			getSavePoints()->push(kEntityGendarmes, kEntityVerges, kAction168710784);
			setup_function13();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Gendarmes, function13)
	if (savepoint.action == kActionDefault)
		getData()->car = kCarNone;
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Gendarmes, chapter2)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityGendarmes);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Gendarmes, chapter3)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityGendarmes);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Gendarmes, chapter4)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityGendarmes);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Gendarmes, chapter5)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityGendarmes);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////
void Gendarmes::arrest(const SavePoint &savepoint, bool shouldPlaySound, SoundFlag flag, bool checkCallback, bool shouldUpdateEntity) {
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (checkCallback) {
			EXPOSE_PARAMS(EntityData::EntityParametersIIII);
			if (Entity::timeCheckCallbackAction((TimeValue)params->param1, params->param2))
				break;
		}

		if (shouldUpdateEntity) {
			EXPOSE_PARAMS(EntityData::EntityParametersIIII);
			if (getEntities()->updateEntity(kEntityGendarmes, (CarIndex)params->param1, (EntityPosition)params->param2)) {
				callbackAction();
				break;
			}
		}
		// Fallback to next action

	case kActionDrawScene:
		if (!ENTITY_PARAM(0, 1) && getEntities()->hasValidFrame(kEntityGendarmes)) {
			getSound()->playSound(kEntityPlayer, "MUS007");
			ENTITY_PARAM(0, 1) = 1;
		}

		if (getEntities()->isDistanceBetweenEntities(kEntityGendarmes, kEntityPlayer, 1000) && !getEntityData(kEntityPlayer)->location) {

			if (shouldUpdateEntity)
				if (getEntities()->isPlayerPosition(kCarRedSleeping, 22) && !getEntities()->isDistanceBetweenEntities(kEntityGendarmes, kEntityPlayer, 250))
					break;

			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventGendarmesArrestation);
		}
		break;

	case kActionExitCompartment:
		callbackAction();
		break;

	case kActionDefault:
		// Only handle when passing SIIS params
		if (!checkCallback) {
			EXPOSE_PARAMS(EntityData::EntityParametersSIIS);

			if (!shouldPlaySound)
				getEntities()->drawSequenceRight(kEntityGendarmes, (char *)&params->seq1);
			else
				getSound()->playSound(kEntityGendarmes, (char *)&params->seq1, flag);
		}

		if (shouldUpdateEntity) {
			EXPOSE_PARAMS(EntityData::EntityParametersIIII);
			if (getEntities()->updateEntity(kEntityGendarmes, (CarIndex)params->param1, (EntityPosition)params->param2)) {
				callbackAction();
				break;
			}
		}
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventGendarmesArrestation);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneGameOverPolice1, true);
		}
		break;
	}
}

} // End of namespace LastExpress
