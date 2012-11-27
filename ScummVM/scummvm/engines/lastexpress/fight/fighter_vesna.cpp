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

#include "lastexpress/fight/fighter_vesna.h"

#include "lastexpress/data/cursor.h"
#include "lastexpress/data/sequence.h"

#include "lastexpress/sound/queue.h"
#include "lastexpress/sound/sound.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

//////////////////////////////////////////////////////////////////////////
// Player
//////////////////////////////////////////////////////////////////////////
FighterPlayerVesna::FighterPlayerVesna(LastExpressEngine *engine) : Fighter(engine) {
	_sequences.push_back(loadSequence("2005cr.seq"));
	_sequences.push_back(loadSequence("2005cdr.seq"));
	_sequences.push_back(loadSequence("2005cbr.seq"));
	_sequences.push_back(loadSequence("2005bk.seq"));
	_sequences.push_back(loadSequence("2005cdm1.seq"));
	_sequences.push_back(loadSequence("2005chl.seq"));
}

void FighterPlayerVesna::handleAction(FightAction action) {
	switch (action) {
	default:
		Fighter::handleAction(action);
		return;

	case kFightAction1:
		if (_sequenceIndex != 1) {
			_opponent->handleAction(kFightAction103);
			update();
		} else {
			_field_34++;
		}
		break;

	case kFightAction2:
		if (_sequenceIndex != 2) {
			_opponent->handleAction(kFightAction103);
			update();
		} else {
			_field_34++;
		}
		break;

	case kFightAction5:
		if (_sequenceIndex != 3) {
			_opponent->handleAction(kFightAction103);
			update();
		}
		break;

	case kFightAction128:
		if (_sequenceIndex == 1 && _opponent->getSequenceIndex() == 1 && checkFrame(4)) {
			setSequenceAndDraw(5, kFightSequenceType1);
		} else {
			setSequenceAndDraw((_opponent->getSequenceIndex() == 5) ? 3 : 1, kFightSequenceType0);
		}
		break;

	case kFightAction132:
		setSequenceAndDraw(2, kFightSequenceType0);
		break;
	}

	if (_field_34 > 10) {
		_opponent->setSequenceAndDraw(5, kFightSequenceType2);
		_opponent->setCountdown(1);
		_field_34 = 0;
	}
}

void FighterPlayerVesna::update() {
	if (_frame && checkFrame(2)) {

		if (_sequenceIndex == 3)
			_opponent->handleAction(kFightAction3);

		if (_opponent->getCountdown() <= 0) {
			getSoundQueue()->removeFromQueue(kEntityTables0);
			_fight->bailout(Fight::kFightEndWin);
			return;
		}

		if (_sequenceIndex == 5)
			_opponent->handleAction(kFightAction5);
	}

	Fighter::update();
}

bool FighterPlayerVesna::canInteract(FightAction action) {
	if (action != kFightAction128)
		return Fighter::canInteract();

	if (_sequenceIndex != 1) {

		if (_opponent->getSequenceIndex() == 5) {
			_engine->getCursor()->setStyle(kCursorDown);
			return true;
		}

		return Fighter::canInteract();
	}

	if (_opponent->getSequenceIndex() == 1 && checkFrame(4)) {
		_engine->getCursor()->setStyle(kCursorPunchLeft);
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Opponent
//////////////////////////////////////////////////////////////////////////
FighterOpponentVesna::FighterOpponentVesna(LastExpressEngine *engine) : Opponent(engine) {
	_sequences.push_back(loadSequence("2005or.seq"));
	_sequences.push_back(loadSequence("2005oam.seq"));
	_sequences.push_back(loadSequence("2005oar.seq"));
	_sequences.push_back(loadSequence("2005okml.seq"));
	_sequences.push_back(loadSequence("2005okr.seq"));
	_sequences.push_back(loadSequence("2005odm1.seq"));
	_sequences.push_back(loadSequence("2005csbm.seq"));
	_sequences.push_back(loadSequence("2005oam4.seq"));

	getSound()->playSound(kEntityTables0, "MUS038", kFlagDefault);

	_countdown = 4;
	_field_38 = 30;
}

void FighterOpponentVesna::handleAction(FightAction action) {
	switch (action) {
	default:
		Fighter::handleAction(action);
		break;

	case kFightAction3:
		_opponent->handleAction(kFightAction103);
		break;

	case kFightAction5:
		setSequenceAndDraw(7, kFightSequenceType1);
		_opponent->handleAction(kFightAction103);
		if (_countdown <= 1)
			_countdown = 1;
		break;

	case kFightAction131:
		break;
	}
}

void FighterOpponentVesna::update() {
	if (!_field_38 && canInteract(kFightAction1) && !_sequenceIndex2) {

		if (_opponent->getField34() == 1) {
			setSequenceAndDraw(2, kFightSequenceType0);
		} else {
			switch (rnd(6)) {
			default:
				break;

			case 0:
				setSequenceAndDraw(1, kFightSequenceType0);
				break;

			case 1:
				setSequenceAndDraw(1, kFightSequenceType0);
				setSequenceAndDraw(1, kFightSequenceType2);
				break;

			case 2:
				setSequenceAndDraw(2, kFightSequenceType0);
				break;

			case 3:
				setSequenceAndDraw(2, kFightSequenceType0);
				setSequenceAndDraw(2, kFightSequenceType2);
				break;

			case 4:
				setSequenceAndDraw(1, kFightSequenceType0);
				setSequenceAndDraw(2, kFightSequenceType2);
				break;

			case 5:
				setSequenceAndDraw(2, kFightSequenceType0);
				setSequenceAndDraw(1, kFightSequenceType2);
				break;
			}
		}

		// Update field_38
		_field_38 = 4 * _countdown;
	}

	if (_frame && checkFrame(2)) {
		if (_sequenceIndex == 1 || _sequenceIndex == 2 || _sequenceIndex == 5)
			_opponent->handleAction((FightAction)_sequenceIndex);

		if (_opponent->getCountdown() <= 0) {

			switch (_sequenceIndex) {
			default:
				break;

			case 1:
				setSequenceAndDraw(3, kFightSequenceType1);
				break;

			case 2:
				setSequenceAndDraw(4, kFightSequenceType1);
				break;

			case 5:
				setSequenceAndDraw(6, kFightSequenceType1);
				break;
			}

			_opponent->setSequenceAndDraw(4, kFightSequenceType1);

			handleAction(kFightActionLost);
			_opponent->update();
			Fighter::update();

			getSoundQueue()->removeFromQueue(kEntityTables0);

			// Stop processing
			return;
		}
	}

	Fighter::update();
}

} // End of namespace LastExpress
