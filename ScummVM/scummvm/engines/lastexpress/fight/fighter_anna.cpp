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

#include "lastexpress/fight/fighter_anna.h"

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
FighterPlayerAnna::FighterPlayerAnna(LastExpressEngine *engine) : Fighter(engine) {
	_sequences.push_back(loadSequence("2002cr.seq"));
	_sequences.push_back(loadSequence("2002cdl.seq"));
	_sequences.push_back(loadSequence("2002cdr.seq"));
	_sequences.push_back(loadSequence("2002cdm.seq"));
	_sequences.push_back(loadSequence("2002lbk.seq"));
}

void FighterPlayerAnna::handleAction(FightAction action) {
	switch (action) {
	default:
		Fighter::handleAction(action);
		return;

	case kFightAction1:
		if ((_sequenceIndex != 1 && _sequenceIndex != 3) || checkFrame(4)) {
			setSequenceAndDraw(4, kFightSequenceType1);
			_opponent->setSequenceAndDraw(4, kFightSequenceType1);

			_opponent->handleAction(kFightAction103);
			update();
		} else {
			_field_34++;
		}
		break;

	case kFightAction2:
		if ((_sequenceIndex != 2 && _sequenceIndex != 3) || checkFrame(4)) {
			setSequenceAndDraw(4, kFightSequenceType1);
			_opponent->setSequenceAndDraw(5, kFightSequenceType1);

			_opponent->handleAction(kFightAction103);
			update();
		} else {
			_field_34++;
		}
		break;

	case kFightAction3:
		if ((_sequenceIndex != 2 && _sequenceIndex != 1) || checkFrame(4)) {
			setSequenceAndDraw(4, kFightSequenceType1);
			_opponent->setSequenceAndDraw(6, kFightSequenceType1);

			_opponent->handleAction(kFightAction103);
			update();
		} else {
			_field_34++;
		}
		break;

	case kFightAction128:
		switch (_opponent->getSequenceIndex()) {
		default:
			setSequenceAndDraw(3, kFightSequenceType0);
			break;

		case 1:
			setSequenceAndDraw(1, kFightSequenceType0);
			break;

		case 2:
			setSequenceAndDraw(3, kFightSequenceType0);
			break;

		case 3:
			setSequenceAndDraw(2, kFightSequenceType0);
			break;
		}
		break;
	}

	if (_field_34 > 4) {
		getSoundQueue()->removeFromQueue(kEntityTables0);
		_fight->bailout(Fight::kFightEndWin);
	}
}

//////////////////////////////////////////////////////////////////////////
// Opponent
//////////////////////////////////////////////////////////////////////////
FighterOpponentAnna::FighterOpponentAnna(LastExpressEngine *engine) : Opponent(engine) {
	_sequences.push_back(loadSequence("2002or.seq"));
	_sequences.push_back(loadSequence("2002oal.seq"));
	_sequences.push_back(loadSequence("2002oam.seq"));
	_sequences.push_back(loadSequence("2002oar.seq"));
	_sequences.push_back(loadSequence("2002okr.seq"));
	_sequences.push_back(loadSequence("2002okml.seq"));
	_sequences.push_back(loadSequence("2002okm.seq"));

	getSound()->playSound(kEntityTables0, "MUS030", kFlagDefault);

	_field_38 = 30;
}

void FighterOpponentAnna::update() {
	if (!_field_38 && canInteract(kFightAction1) && !_sequenceIndex2) {

		if (_opponent->getField34() >= 2) {
			switch (rnd(6)) {
			default:
				break;

			case 0:
				setSequenceAndDraw(1, kFightSequenceType0);
				break;

			case 1:
				setSequenceAndDraw(2, kFightSequenceType0);
				break;

			case 2:
				setSequenceAndDraw(3, kFightSequenceType0);
				break;

			case 3:
				setSequenceAndDraw(3, kFightSequenceType0);
				setSequenceAndDraw(2, kFightSequenceType2);
				break;

			case 4:
				setSequenceAndDraw(1, kFightSequenceType0);
				setSequenceAndDraw(2, kFightSequenceType2);
				break;

			case 5:
				setSequenceAndDraw(3, kFightSequenceType0);
				setSequenceAndDraw(2, kFightSequenceType2);
				break;
			}
		}

		// Update field_38
		_field_38 = (int32)rnd(15);
	}

	if (_frame && checkFrame(2)) {
		if (_sequenceIndex == 1 || _sequenceIndex == 2 || _sequenceIndex == 3)
			_opponent->handleAction((FightAction)_sequenceIndex);

		if (_opponent->getCountdown() <= 0) {
			getSoundQueue()->removeFromQueue(kEntityTables0);
			handleAction(kFightActionLost);
		}
	}

	Fighter::update();
}

} // End of namespace LastExpress
