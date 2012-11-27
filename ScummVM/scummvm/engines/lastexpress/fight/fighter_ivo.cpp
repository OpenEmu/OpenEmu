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

#include "lastexpress/fight/fighter_ivo.h"

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
FighterPlayerIvo::FighterPlayerIvo(LastExpressEngine *engine) : Fighter(engine) {
	_sequences.push_back(loadSequence("2003cr.seq"));
	_sequences.push_back(loadSequence("2003car.seq"));
	_sequences.push_back(loadSequence("2003cal.seq"));
	_sequences.push_back(loadSequence("2003cdr.seq"));
	_sequences.push_back(loadSequence("2003cdm.seq"));
	_sequences.push_back(loadSequence("2003chr.seq"));
	_sequences.push_back(loadSequence("2003chl.seq"));
	_sequences.push_back(loadSequence("2003ckr.seq"));
	_sequences.push_back(loadSequence("2003lbk.seq"));
	_sequences.push_back(loadSequence("2003fbk.seq"));

	_countdown = 5;
}

void FighterPlayerIvo::handleAction(FightAction action) {
	switch (action) {
	default:
		Fighter::handleAction(action);
		return;

	case kFightAction1:
		if (_sequenceIndex != 1 || checkFrame(4)) {
			setSequenceAndDraw(7, kFightSequenceType1);
			_opponent->setSequenceAndDraw(4, kFightSequenceType1);

			_opponent->handleAction(kFightAction103);
			update();
		}
		break;

	case kFightAction2:
		if ((_sequenceIndex != 2 && _sequenceIndex != 3) || checkFrame(4)) {
			setSequenceAndDraw(7, kFightSequenceType1);
			_opponent->setSequenceAndDraw(5, kFightSequenceType1);

			_opponent->handleAction(kFightAction103);
			update();
		}
		break;

	case kFightAction128:
		switch (_opponent->getSequenceIndex()) {
		default:
		case 1:
			setSequenceAndDraw(1, kFightSequenceType0);
			break;

		case 2:
			setSequenceAndDraw(2, kFightSequenceType0);
			break;
		}
		break;

	case kFightAction129:
		setSequenceAndDraw((_opponent->getCountdown() > 1) ? 4 : 3, _sequenceIndex ? kFightSequenceType2 : kFightSequenceType0);
		break;

	case kFightAction130:
		setSequenceAndDraw(3, _sequenceIndex ? kFightSequenceType2 : kFightSequenceType0);
		break;
	}
}

void FighterPlayerIvo::update() {

	if ((_sequenceIndex == 3 || _sequenceIndex == 4) && !_frameIndex)
		_opponent->handleAction(kFightAction131);

	if (_frame && checkFrame(2)) {

		// Draw sequences
		if (_opponent->getCountdown() <= 0) {
			setSequenceAndDraw(9, kFightSequenceType1);
			_opponent->setSequenceAndDraw(8, kFightSequenceType1);
			getSoundQueue()->removeFromQueue(kEntityTables0);

			handleAction(kFightActionWin);
			return;
		}

		if (_sequenceIndex == 3 || _sequenceIndex == 4)
			_opponent->handleAction((FightAction)_sequenceIndex);
	}

	Fighter::update();
}

bool FighterPlayerIvo::canInteract(FightAction action) {
	if (action == kFightAction129 || action == kFightAction130)
		return (_sequenceIndex >= 8);

	return Fighter::canInteract();
}

//////////////////////////////////////////////////////////////////////////
// Opponent
//////////////////////////////////////////////////////////////////////////
FighterOpponentIvo::FighterOpponentIvo(LastExpressEngine *engine) : Opponent(engine) {
	_sequences.push_back(loadSequence("2003or.seq"));
	_sequences.push_back(loadSequence("2003oal.seq"));
	_sequences.push_back(loadSequence("2003oar.seq"));
	_sequences.push_back(loadSequence("2003odm.seq"));
	_sequences.push_back(loadSequence("2003okl.seq"));
	_sequences.push_back(loadSequence("2003okj.seq"));
	_sequences.push_back(loadSequence("blank.seq"));
	_sequences.push_back(loadSequence("csdr.seq"));
	_sequences.push_back(loadSequence("2003l.seq"));

	getSound()->playSound(kEntityTables0, "MUS032", kFlagDefault);

	_countdown = 5;
	_field_38 = 15;
}

void FighterOpponentIvo::handleAction(FightAction action) {
	switch (action) {
	default:
		Fighter::handleAction(action);
		break;

	case kFightAction3:
		if ((_sequenceIndex != 1 && _sequenceIndex != 3) || checkFrame(4)) {
			setSequenceAndDraw(6, kFightSequenceType1);
			_opponent->setSequenceAndDraw(6, kFightSequenceType1);
			_opponent->handleAction(kFightAction103);
		}
		break;

	case kFightAction4:
		if ((_sequenceIndex != 2 && _sequenceIndex != 3) || checkFrame(4)) {
			setSequenceAndDraw(6, kFightSequenceType1);
			_opponent->setSequenceAndDraw(5, kFightSequenceType1);
			_opponent->handleAction(kFightAction103);
		}
		break;

	case kFightAction131:
		if (_sequenceIndex)
			break;

		if (rnd(100) <= (unsigned int)(_countdown > 2 ? 60 : 75)) {
			setSequenceAndDraw(3 , kFightSequenceType1);
			if (_opponent->getSequenceIndex() == 4)
				setSequenceAndDraw(2, kFightSequenceType2);
		}
		break;
	}
}

void FighterOpponentIvo::update() {
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
				setSequenceAndDraw(1, kFightSequenceType0);
				setSequenceAndDraw(2, kFightSequenceType2);
				break;

			case 3:
				setSequenceAndDraw(0, kFightSequenceType2);
				setSequenceAndDraw(1, kFightSequenceType2);
				break;

			case 4:
				setSequenceAndDraw(0, kFightSequenceType1);
				setSequenceAndDraw(1, kFightSequenceType2);
				break;
			}
		}

		// Update field_38
		_field_38 = 3 * _countdown + (int32)rnd(10);
	}

	if (_frame && checkFrame(2)) {

		if (_opponent->getCountdown() <= 0) {
			setSequenceAndDraw(7, kFightSequenceType1);
			_opponent->setSequenceAndDraw(8, kFightSequenceType1);
			getSoundQueue()->removeFromQueue(kEntityTables0);

			_opponent->handleAction(kFightActionWin);

			return;
		}

		if (_sequenceIndex == 1 || _sequenceIndex == 2)
			_opponent->handleAction((FightAction)_sequenceIndex);
	}

	Fighter::update();
}

} // End of namespace LastExpress
