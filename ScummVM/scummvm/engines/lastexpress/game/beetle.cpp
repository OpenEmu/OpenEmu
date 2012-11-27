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

#include "lastexpress/game/beetle.h"

#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

Beetle::Beetle(LastExpressEngine *engine) : _engine(engine), _data(NULL) {}

Beetle::~Beetle() {
	SAFE_DELETE(_data);

	// Free passed pointers
	_engine = NULL;
}

void Beetle::load() {
	// Only load in chapter 2 & 3
	if (getProgress().chapter != kChapter2 &&  getProgress().chapter != kChapter3)
		return;

	// Already loaded
	if (_data)
		return;

	// Do not load if beetle is in the wrong location
	if (getInventory()->get(kItemBeetle)->location != kObjectLocation3)
		return;

	///////////////////////
	// Load Beetle data
	_data = new BeetleData();

	// Load sequences
	_data->sequences.push_back(loadSequence("BW000.seq"));        // 0
	_data->sequences.push_back(loadSequence("BT000045.seq"));
	_data->sequences.push_back(loadSequence("BT045000.seq"));
	_data->sequences.push_back(loadSequence("BW045.seq"));
	_data->sequences.push_back(loadSequence("BT045090.seq"));
	_data->sequences.push_back(loadSequence("BT090045.seq"));     // 5
	_data->sequences.push_back(loadSequence("BW090.seq"));
	_data->sequences.push_back(loadSequence("BT090135.seq"));
	_data->sequences.push_back(loadSequence("BT135090.seq"));
	_data->sequences.push_back(loadSequence("BW135.seq"));
	_data->sequences.push_back(loadSequence("BT135180.seq"));     // 10
	_data->sequences.push_back(loadSequence("BT180135.seq"));
	_data->sequences.push_back(loadSequence("BW180.seq"));
	_data->sequences.push_back(loadSequence("BT180225.seq"));
	_data->sequences.push_back(loadSequence("BT225180.seq"));
	_data->sequences.push_back(loadSequence("BW225.seq"));        // 15
	_data->sequences.push_back(loadSequence("BT225270.seq"));
	_data->sequences.push_back(loadSequence("BT270225.seq"));
	_data->sequences.push_back(loadSequence("BW270.seq"));
	_data->sequences.push_back(loadSequence("BT270315.seq"));
	_data->sequences.push_back(loadSequence("BT315270.seq"));     // 20
	_data->sequences.push_back(loadSequence("BW315.seq"));
	_data->sequences.push_back(loadSequence("BT315000.seq"));
	_data->sequences.push_back(loadSequence("BT000315.seq"));
	_data->sequences.push_back(loadSequence("BA135.seq"));
	_data->sequences.push_back(loadSequence("BL045.seq"));        // 25
	_data->sequences.push_back(loadSequence("BL000.seq"));
	_data->sequences.push_back(loadSequence("BL315.seq"));
	_data->sequences.push_back(loadSequence("BL180.seq"));

	// Init fields
	_data->field_74 = 0;

	// Check that all sequences are loaded properly
	_data->isLoaded = true;
	for (uint i = 0; i < _data->sequences.size(); i++) {
		if (!_data->sequences[i]->isLoaded()) {
			_data->isLoaded = false;
			break;
		}
	}

	_data->field_D9 = 10;
	_data->coordOffset = 5;
	_data->coordY = 178;
	_data->currentSequence = 0;
	_data->offset = 0;
	_data->frame = NULL;
	_data->field_D5 = 0;
	_data->indexes[0] = 29;
	_data->field_DD = 0;
}

void Beetle::unload() {
	// Remove sequences from display list
	if (_data)
		getScenes()->removeFromQueue(_data->frame);

	// Delete all loaded sequences
	SAFE_DELETE(_data);
}

bool Beetle::isLoaded() const {
	if (!_data)
		return false;

	return _data->isLoaded;
}

bool Beetle::catchBeetle() {
	if (!_data)
		error("[Beetle::catchBeetle] Sequences have not been loaded");

	if (getInventory()->getSelectedItem() == kItemMatchBox
	 && getInventory()->hasItem(kItemMatch)
	 && ABS((int16)(getCoords().x - _data->coordX)) < 10
	 && ABS((int16)(getCoords().y - _data->coordY)) < 10) {
		return true;
	}

	_data->field_D5 = 0;
	move();

	return false;
}

bool Beetle::isCatchable() const {
	if (!_data)
		error("[Beetle::isCatchable] Sequences have not been loaded");

	return (_data->indexes[_data->offset] >= 30);
}

void Beetle::update() {
	if (!_data)
		error("[Beetle::update] Sequences have not been loaded");

	if (!_data->isLoaded)
		return;

	move();

	if (_data->field_D5)
		_data->field_D5--;

	if (_data->currentSequence && _data->indexes[_data->offset] != 29) {
		drawUpdate();
		return;
	}

	if (getInventory()->get(kItemBeetle)->location == kObjectLocation3) {
		if ((!_data->field_DD && rnd(10) < 1)
		  || (_data->field_DD && rnd(30) < 1)
		  || rnd(100) < 1) {

			_data->field_DD++;
			if (_data->field_DD > 3)
				_data->field_DD = 0;

			updateData(24);

			_data->coordX = (int16)(rnd(250) + 190);
			_data->coordOffset = (int16)(rnd(5) + 5);

			if (_data->field_D9 > 1)
				_data->field_D9--;

			drawUpdate();
		}
	}
}

void Beetle::drawUpdate() {
	if (!_data)
		error("[Beetle::drawUpdate] Sequences have not been loaded");

	if (_data->frame != NULL) {
		getScenes()->setCoordinates(_data->frame);
		getScenes()->removeFromQueue(_data->frame);
	}

	// Update current frame
	switch (_data->indexes[_data->offset]) {
	default:
		_data->currentFrame += 10;
		break;

	case 3:
	case 6:
	case 9:
	case 12:
	case 15:
	case 18:
	case 21:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
		_data->currentFrame++;
		break;
	}

	// Update current sequence
	if (_data->currentSequence->count() <= _data->currentFrame) {
		switch (_data->indexes[_data->offset]) {
		default:
			_data->offset++;
			_data->currentSequence = _data->sequences[_data->indexes[_data->offset]];
			break;

		case 3:
		case 6:
		case 9:
		case 12:
		case 15:
		case 18:
		case 21:
			break;
		}

		_data->currentFrame = 0;
		if (_data->indexes[_data->offset] == 29) {
			SAFE_DELETE(_data->frame);
			_data->currentSequence = NULL; // pointer to existing sequence
			return;
		}
	}

	// Update coordinates
	switch (_data->indexes[_data->offset]) {
	default:
		break;

	case 0:
		_data->coordY -= _data->coordOffset;
		break;

	case 3:
		_data->coordX += _data->coordOffset;
		_data->coordY -= _data->coordOffset;
		break;

	case 6:
		_data->coordX += _data->coordOffset;
		break;

	case 9:
		_data->coordX += _data->coordOffset;
		_data->coordY += _data->coordOffset;
		break;

	case 12:
		_data->coordY += _data->coordOffset;
		break;

	case 15:
		_data->coordX -= _data->coordOffset;
		_data->coordY += _data->coordOffset;
		break;

	case 18:
		_data->coordX -= _data->coordOffset;
		break;

	case 21:
		_data->coordX -= _data->coordOffset;
		_data->coordY -= _data->coordOffset;
		break;
	}

	// Update beetle data
	int rnd = rnd(100);
	if (_data->coordX < 165 || _data->coordX > 465) {
		uint index = 0;

		if (rnd >= 30) {
			if (rnd >= 70)
				index = (_data->coordX < 165) ? 9 : 15;
			else
				index = (_data->coordX < 165) ? 6 : 18;
		} else {
			index = (_data->coordX < 165) ? 3 : 21;
		}

		updateData(index);
	}

	if (_data->coordY < 178) {
		switch (_data->indexes[_data->offset]) {
		default:
			updateData(26);
			break;

		case 3:
			updateData(25);
			break;

		case 21:
			updateData(27);
			break;
		}
	}

	if (_data->coordY > 354) {
		switch (_data->indexes[_data->offset]) {
		default:
			break;

		case 9:
		case 12:
		case 15:
			updateData(28);
			break;
		}
	}

	// Invert direction
	invertDirection();

	SequenceFrame *frame = new SequenceFrame(_data->currentSequence, (uint16)_data->currentFrame);
	updateFrame(frame);

	invertDirection();

	getScenes()->addToQueue(frame);

	SAFE_DELETE(_data->frame);
	_data->frame = frame;
}

void Beetle::invertDirection() {
	if (!_data)
		error("[Beetle::invertDirection] Sequences have not been loaded");

	switch (_data->indexes[_data->offset]) {
	default:
		break;

	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
		_data->coordY = -_data->coordY;
		break;
	}
}

void Beetle::move() {
	if (!_data)
		error("[Beetle::move] Sequences have not been loaded");

	if (_data->indexes[_data->offset] >= 24 && _data->indexes[_data->offset] <= 29)
		return;

	if (_data->field_D5)
		return;

	if (ABS((int)(getCoords().x - _data->coordX)) > 35)
		return;

	if (ABS((int)(getCoords().y - _data->coordY)) > 35)
		return;

	int32 deltaX = getCoords().x - _data->coordX;
	int32 deltaY = -getCoords().y - _data->coordY;
	uint32 index = 0;

	// FIXME: check code path
	if (deltaX >= 0) {
		if (deltaY > 0) {
			if (100 * deltaY - 241 * deltaX <= 0) {
				if (100 * deltaY  - 41 * deltaX <= 0)
					index = 18;
				else
					index = 15;
			} else {
				index = 12;
			}

			goto update_data;
		}
	}

	if (deltaX < 0) {

		if (deltaY > 0) {
			if (100 * deltaY + 241 * deltaX <= 0) {
				if (100 * deltaY + 41 * deltaX <= 0)
					index = 6;
				else
					index = 9;
			} else {
				index = 12;
			}

			goto update_data;
		}

		if (deltaY <= 0) {
			if (100 * deltaY - 41 * deltaX <= 0) {
				if (100 * deltaY - 241 * deltaX <= 0)
					index = 0;
				else
					index = 3;
			} else {
				index = 6;
			}

			goto update_data;
		}
	}

update_data:
	updateData(index);

	if (_data->coordOffset >= 15) {
		_data->field_D5 = 0;
		return;
	}

	_data->coordOffset = _data->coordOffset + (int16)(4 * rnd(100)/100 + _data->field_D9);
	_data->field_D5 = 0;
}

// Update the beetle sequence to show the correct frames in the correct place
void Beetle::updateFrame(SequenceFrame *frame) const {
	if (!_data)
		error("[Beetle::updateFrame] Sequences have not been loaded");

	if (!frame)
		return;

	// Update coordinates
	if (_data->coordX > 0)
		frame->getInfo()->xPos1 = (uint16)_data->coordX;

	if (_data->coordY > 0)
		frame->getInfo()->yPos1 = (uint16)_data->coordY;
}

void Beetle::updateData(uint32 index) {
	if (!_data)
		error("[Beetle::updateData] Sequences have not been loaded");

	if (!_data->isLoaded)
		return;

	if (index == 25 || index == 26 || index == 27 || index == 28) {
		_data->indexes[0] = index;
		_data->indexes[1] = 29;
		_data->offset = 0;

		_data->currentSequence = _data->sequences[index];
		_data->currentFrame = 0;
		_data->index = index;
	} else {
		if (!_data->sequences[index])
			return;

		if (_data->index == index)
			return;

		_data->offset = 0;

		// Special case for sequence 24
		if (index == 24) {
			_data->indexes[0] = index;
			_data->coordY = 178;
			_data->index = _data->indexes[1];
			_data->indexes[1] = (_data->coordX >= 265) ? 15 : 9;
			_data->currentFrame = 0;
			_data->currentSequence = _data->sequences[index];
		} else {
			if (index <= _data->index) {
				for (uint32 i = _data->index - 1; i > index; ++_data->offset) {
					_data->indexes[_data->offset] = i;
					i -= 3;
				}
			} else {
				for (uint32 i = _data->index + 1; i < index; ++_data->offset) {
					_data->indexes[_data->offset] = i;
					i += 3;
				}
			}

			_data->index = index;
			_data->indexes[_data->offset] = index;
			_data->currentFrame = 0;
			_data->offset = 0;
			_data->currentSequence = _data->sequences[_data->indexes[0]];
		}
	}
}

} // End of namespace LastExpress
