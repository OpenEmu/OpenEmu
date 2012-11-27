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

#include "lastexpress/fight/fighter_salko.h"

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
FighterPlayerSalko::FighterPlayerSalko(LastExpressEngine *engine) : Fighter(engine) {
	_sequences.push_back(loadSequence("2004cr.seq"));
	_sequences.push_back(loadSequence("2004cdr.seq"));
	_sequences.push_back(loadSequence("2004chj.seq"));
	_sequences.push_back(loadSequence("2004bk.seq"));

	_countdown = 2;
}

void FighterPlayerSalko::handleAction(FightAction action) {
	switch (action) {
	default:
		Fighter::handleAction(action);
		return;

	case kFightAction1:
	case kFightAction2:
		if (_sequenceIndex != 1 && checkFrame(4)) {
			_field_34 = 0;

			setSequenceAndDraw(3, kFightSequenceType1);
			_opponent->setSequenceAndDraw((action == kFightAction1 ? 3 : 4), kFightSequenceType1);

			_opponent->handleAction(kFightAction103);

			if (action == kFightAction2)
				_countdown= 0;

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
		setSequenceAndDraw(1, kFightSequenceType0);
		_field_34 = 0;
		break;

	case kFightAction131:
		setSequenceAndDraw(2, (_sequenceIndex ? kFightSequenceType2 : kFightSequenceType0));
		break;
	}
}

void FighterPlayerSalko::update() {
	Fighter::update();

	// The original doesn't check for currentSequence2 != NULL (might not happen when everything is working properly, but crashes with our current implementation)
	if (_frame && checkFrame(2)) {

		if (_opponent->getCountdown() <= 0) {
			getSoundQueue()->removeFromQueue(kEntityTables0);
			_fight->bailout(Fight::kFightEndWin);

			return;
		}

		if (_sequenceIndex == 2)
			_opponent->handleAction(kFightAction2);
	}
}

bool FighterPlayerSalko::canInteract(FightAction action) {
	if (action == kFightAction131) {
		if (_sequenceIndex == 1) {
			if (_opponent->getCountdown() <= 0)
				_engine->getCursor()->setStyle(kCursorHand);

			return true;
		}

		return false;
	}

	return Fighter::canInteract();
}

//////////////////////////////////////////////////////////////////////////
// Opponent
//////////////////////////////////////////////////////////////////////////
FighterOpponentSalko::FighterOpponentSalko(LastExpressEngine *engine) : Opponent(engine) {
	_sequences.push_back(loadSequence("2004or.seq"));
	_sequences.push_back(loadSequence("2004oam.seq"));
	_sequences.push_back(loadSequence("2004oar.seq"));
	_sequences.push_back(loadSequence("2004okr.seq"));
	_sequences.push_back(loadSequence("2004ohm.seq"));
	_sequences.push_back(loadSequence("blank.seq"));

	getSound()->playSound(kEntityTables0, "MUS035", kFlagDefault);

	_countdown = 3;
	_field_38 = 30;
}

void FighterOpponentSalko::handleAction(FightAction action) {
	if (action == kFightAction2) {
		setSequenceAndDraw(5, kFightSequenceType1);
		_opponent->handleAction(kFightAction103);
	} else {
		Fighter::handleAction(action);
	}
}

void FighterOpponentSalko::update() {
	if (!_field_38 && canInteract(kFightAction1) && !_sequenceIndex2) {

		switch (rnd(5)) {
		default:
			break;

		case 0:
			setSequenceAndDraw(1, kFightSequenceType0);
			break;

		case 1:
			setSequenceAndDraw(2, kFightSequenceType0);
			break;

		case 2:
			setSequenceAndDraw(1, kFightSequenceType0);
			setSequenceAndDraw(2, kFightSequenceType2);
			break;

		case 3:
			setSequenceAndDraw(2, kFightSequenceType0);
			setSequenceAndDraw(1, kFightSequenceType2);
			break;

		case 4:
			setSequenceAndDraw(1, kFightSequenceType0);
			setSequenceAndDraw(1, kFightSequenceType2);
			break;
		}

		// Update field_38
		_field_38 = 4 * _countdown;
	}

	if (_frame && checkFrame(2)) {
		if (_opponent->getCountdown() <= 0) {
			getSoundQueue()->removeFromQueue(kEntityTables0);
			_fight->bailout(Fight::kFightEndLost);

			// Stop processing
			return;
		}

		if (_sequenceIndex == 1 || _sequenceIndex == 2)
			_opponent->handleAction((FightAction)_sequenceIndex);
	}

	Fighter::update();
}

} // End of namespace LastExpress
