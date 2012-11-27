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

#include "lastexpress/fight/fighter.h"

#include "lastexpress/data/sequence.h"

#include "lastexpress/game/scenes.h"

#include "lastexpress/sound/sound.h"

#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"

namespace LastExpress {

Fighter::Fighter(LastExpressEngine *engine) : _engine(engine) {
	_opponent = NULL;
	_fight = NULL;

	_sequenceIndex = 0;
	_sequence = NULL;
	_frame = NULL;
	_frameIndex = 0;

	_field_24 = 0;

	_action = kFightAction101;
	_sequenceIndex2 = 0;

	_countdown = 1;

	_field_34 = 0;
}

Fighter::~Fighter() {
	// The original game resets the function pointers to default values, just before deleting the struct

	getScenes()->removeAndRedraw(&_frame, false);

	// Free sequences
	for (uint i = 0; i < _sequences.size(); i++)
		SAFE_DELETE(_sequences[i]);

	// Zero-out passed pointers
	_sequence = NULL;
	_opponent = NULL;
	_fight = NULL;

	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Drawing
//////////////////////////////////////////////////////////////////////////
void Fighter::setSequenceAndDraw(uint32 sequenceIndex, FightSequenceType type) {
	if (_sequences.size() < sequenceIndex)
		return;

	switch (type) {
	default:
		break;

	case kFightSequenceType0:
		if (_sequenceIndex)
			return;

		_sequence = _sequences[sequenceIndex];
		_sequenceIndex = sequenceIndex;
		draw();
		break;

	case kFightSequenceType1:
		_sequence = _sequences[sequenceIndex];
		_sequenceIndex = sequenceIndex;
		_sequenceIndex2 = 0;
		draw();
		break;

	case kFightSequenceType2:
		_sequenceIndex2 = sequenceIndex;
		break;
	}
}

void Fighter::draw() {
	getScenes()->removeAndRedraw(&_frame, false);

	_frameIndex = 0;
	_field_24 = 0;
}

//////////////////////////////////////////////////////////////////////////
// Processing
//////////////////////////////////////////////////////////////////////////
void Fighter::process() {
	if (!_fight)
		error("[Fighter::handleAction] Fighter not initialized properly");

	if (!_sequence) {
		if (_frame) {
			getScenes()->removeFromQueue(_frame);
			getScenes()->setCoordinates(_frame);
		}
		SAFE_DELETE(_frame);
		return;
	}

	if (_sequence->count() <= _frameIndex) {
		switch(_action) {
		default:
			break;

		case kFightAction101:
			setSequenceAndDraw(_sequenceIndex2, kFightSequenceType1);
			_sequenceIndex2 = 0;
			break;

		case kFightActionResetFrame:
			_frameIndex = 0;
			break;

		case kFightAction103:
			setSequenceAndDraw(0, kFightSequenceType1);
			handleAction(kFightAction101);
			_opponent->setSequenceAndDraw(0, kFightSequenceType1);
			_opponent->handleAction(kFightAction101);
			_opponent->update();
			break;

		case kFightActionWin:
			_fight->bailout(Fight::kFightEndWin);
			break;

		case kFightActionLost:
			_fight->bailout(Fight::kFightEndLost);
			break;
		}
	}

	if (_fight->isRunning()) {

		// Get the current sequence frame
		SequenceFrame *frame = new SequenceFrame(_sequence, (uint16)_frameIndex);
		frame->getInfo()->location = 1;

		if (_frame == frame) {
			delete frame;
			return;
		}

		getSound()->playFightSound(frame->getInfo()->soundAction, frame->getInfo()->field_31);

		// Add current frame to queue and advance
		getScenes()->addToQueue(frame);
		_frameIndex++;

		if (_frame) {
			getScenes()->removeFromQueue(_frame);

			if (!frame->getInfo()->field_2E)
				getScenes()->setCoordinates(_frame);
		}

		// Replace by new frame
		delete _frame;
		_frame = frame;
	}
}

//////////////////////////////////////////////////////////////////////////
// Default actions
//////////////////////////////////////////////////////////////////////////
void Fighter::handleAction(FightAction action) {
	if (!_opponent || !_fight)
		error("[Fighter::handleAction] Fighter not initialized properly");

	switch (action) {
	default:
		return;

	case kFightAction101:
		break;

	case kFightActionResetFrame:
		_countdown--;
		break;

	case kFightAction103:
		_opponent->handleAction(kFightActionResetFrame);
		break;

	case kFightActionWin:
		_fight->setEndType(Fight::kFightEndWin);
		_opponent->handleAction(kFightActionResetFrame);
		break;

	case kFightActionLost:
		_fight->setEndType(Fight::kFightEndLost);
		_opponent->handleAction(kFightActionResetFrame);
		break;
	}

	// Update action
	_action = action;
}

bool Fighter::canInteract(FightAction /*action = kFightActionNone*/ ) {
	return (_action == kFightAction101 && !_sequenceIndex);
}

void Fighter::update() {
	process();

	if (_frame)
		_frame->getInfo()->location = (_action == kFightActionResetFrame ? 2 : 0);
}

void Opponent::update() {
	process();

	if (_field_38 && !_sequenceIndex)
		_field_38--;

	if (_frame)
		_frame->getInfo()->location = 1;
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
bool Fighter::checkFrame(uint32 val) {
	if (!_frame)
		error("[Fighter::checkFrame] Invalid current frame");

	return (bool)(_frame->getInfo()->field_33 & val);
}

} // End of namespace LastExpress
