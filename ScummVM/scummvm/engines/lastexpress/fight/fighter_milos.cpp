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

#include "lastexpress/fight/fighter_milos.h"

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
FighterPlayerMilos::FighterPlayerMilos(LastExpressEngine *engine) : Fighter(engine) {
	_sequences.push_back(loadSequence("2001cr.seq"));
	_sequences.push_back(loadSequence("2001cdl.seq"));
	_sequences.push_back(loadSequence("2001cdr.seq"));
	_sequences.push_back(loadSequence("2001cdm.seq"));
	_sequences.push_back(loadSequence("2001csgr.seq"));
	_sequences.push_back(loadSequence("2001csgl.seq"));
	_sequences.push_back(loadSequence("2001dbk.seq"));
}

void FighterPlayerMilos::handleAction(FightAction action) {
	switch (action) {
	default:
		Fighter::handleAction(action);
		return;

	case kFightAction1:
		if (_sequenceIndex != 1 || checkFrame(4)) {
			setSequenceAndDraw(6, kFightSequenceType1);
			_opponent->setSequenceAndDraw(3, kFightSequenceType1);

			_opponent->handleAction(kFightAction103);
			update();
		} else {
			_field_34++;
		}
		break;

	case kFightAction2:
		if ((_sequenceIndex != 2 && _sequenceIndex != 3) || checkFrame(4)) {
			setSequenceAndDraw(6, kFightSequenceType1);
			_opponent->setSequenceAndDraw(4, kFightSequenceType1);

			_opponent->handleAction(kFightAction103);
			update();
		} else {
			_field_34++;
		}
		break;

	case kFightAction128:
		if (_sequenceIndex != 1 || checkFrame(4) || _opponent->getSequenceIndex() != 1) {
			switch (_opponent->getSequenceIndex()) {
			default:
				setSequenceAndDraw(rnd(3) + 1, kFightSequenceType0);
				break;

			case 1:
				setSequenceAndDraw(1, kFightSequenceType0);
				break;

			case 2:
				setSequenceAndDraw(3, kFightSequenceType0);
				break;
			}
		} else {
			setSequenceAndDraw(4, kFightSequenceType1);
			update();
		}
		break;
	}
}

void FighterPlayerMilos::update() {
	if (_frame && checkFrame(2)) {

		// Draw sequences
		if (_opponent->getCountdown() <= 0) {
			setSequenceAndDraw(5, kFightSequenceType1);
			_opponent->setSequenceAndDraw(6, kFightSequenceType1);

			getSoundQueue()->removeFromQueue(kEntityTables0);
			getSound()->playSound(kEntityTrain, "MUS029", kFlagDefault);

			handleAction(kFightActionWin);
		}

		if (_sequenceIndex == 4) {
			_opponent->handleAction(kFightAction4);
			_fight->setEndType(Fight::kFightEndLost);
		}
	}

	Fighter::update();
}

bool FighterPlayerMilos::canInteract(FightAction action) {
	if (action != kFightAction128
	 || _sequenceIndex != 1
	 || !_frame
	 || checkFrame(4)
	 || _opponent->getSequenceIndex() != 1) {
		 return Fighter::canInteract();
	}

	_engine->getCursor()->setStyle(kCursorHand);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Opponent
//////////////////////////////////////////////////////////////////////////
FighterOpponentMilos::FighterOpponentMilos(LastExpressEngine *engine) : Opponent(engine) {
	_sequences.push_back(loadSequence("2001or.seq"));
	_sequences.push_back(loadSequence("2001oal.seq"));
	_sequences.push_back(loadSequence("2001oam.seq"));
	_sequences.push_back(loadSequence("2001okl.seq"));
	_sequences.push_back(loadSequence("2001okm.seq"));
	_sequences.push_back(loadSequence("2001dbk.seq"));
	_sequences.push_back(loadSequence("2001wbk.seq"));

	getSound()->playSound(kEntityTables0, "MUS027", kFlagDefault);

	_field_38 = 35;
}

void FighterOpponentMilos::handleAction(FightAction action) {
	if (action == kFightAction4) {
		setSequenceAndDraw(5, kFightSequenceType1);
		_opponent->handleAction(kFightAction103);
	} else {
		if (action != kFightAction131)
			Fighter::handleAction(action);
	}
}

void FighterOpponentMilos::update() {
	if (!_field_38 && canInteract(kFightAction1) && !_sequenceIndex2) {

		if (_opponent->getField34() >= 2) {
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
				setSequenceAndDraw(2, kFightSequenceType0);
				setSequenceAndDraw(2, kFightSequenceType1);
				break;

			case 3:
				setSequenceAndDraw(1, kFightSequenceType0);
				setSequenceAndDraw(2, kFightSequenceType2);
				break;

			case 4:
				setSequenceAndDraw(1, kFightSequenceType0);
				setSequenceAndDraw(1, kFightSequenceType2);
				break;
			}
		} else {
			setSequenceAndDraw(2, kFightSequenceType0);
		}

		// Update field_38
		if (_opponent->getField34() < 5)
			_field_38 = 6 * (5 - _opponent->getField34());
		else
			_field_38 = 0;
	}

	if (_frame && checkFrame(2)) {
		if (_sequenceIndex == 1 || _sequenceIndex == 2)
			_opponent->handleAction((FightAction)_sequenceIndex);

		if (_opponent->getCountdown() <= 0) {
			getSoundQueue()->removeFromQueue(kEntityTables0);
			handleAction(kFightActionLost);
		}
	}

	Fighter::update();
}

} // End of namespace LastExpress
