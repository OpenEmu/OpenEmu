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

#include "gob/sound/sound.h"

#include "gob/minigames/geisha/oko.h"

namespace Gob {

namespace Geisha {

enum kOkoAnimation {
	kOkoAnimationEnter   =  0,
	kOkoAnimationSwim    =  1,
	kOkoAnimationSink    =  8,
	kOkoAnimationRaise   =  7,
	kOkoAnimationBreathe =  2,
	kOkoAnimationPick    =  3,
	kOkoAnimationHurt    =  4,
	kOkoAnimationDie0    = 17,
	kOkoAnimationDie1    = 18,
	kOkoAnimationDie2    = 19
};

static const int16 kOkoPositionX = 110;

static const uint kLevelCount = 3;
static const int16 kLevelPositionX[kLevelCount] = { 44, 84, 124 };


Oko::Oko(const ANIFile &ani, Sound &sound, SoundDesc &breathe) :
	ANIObject(ani), _sound(&sound), _breathe(&breathe), _state(kStateEnter), _level(0) {

	setAnimation(kOkoAnimationEnter);
	setVisible(true);
}

Oko::~Oko() {
}

void Oko::advance() {
	bool wasLastFrame = lastFrame();

	if ((_state == kStateDead) && wasLastFrame) {
		setPause(true);
		return;
	}

	ANIObject::advance();

	switch (_state) {
		case kStateEnter:
			if (wasLastFrame) {
				setAnimation(kOkoAnimationSwim);
				setPosition(kOkoPositionX, kLevelPositionX[_level]);
				_state = kStateSwim;
			}
			break;

		case kStateBreathe:
			if ((getFrame() == 6) || (getFrame() == 23))
			_sound->blasterPlay(_breathe, 1, 0);
		case kStateSink:
		case kStateRaise:
		case kStateHurt:
			if (wasLastFrame) {
				setAnimation(kOkoAnimationSwim);
				setPosition(kOkoPositionX, kLevelPositionX[_level]);
				_state = kStateSwim;
			}
			break;

		case kStatePick:
			if (wasLastFrame) {
				_level = 1;
				setAnimation(kOkoAnimationSwim);
				setPosition(kOkoPositionX, kLevelPositionX[_level]);
				_state = kStateSwim;
			}
			break;

		default:
			break;
	}
}

void Oko::sink() {
	if (_state != kStateSwim)
		return;

	if (_level >= (kLevelCount - 1)) {
		setAnimation(kOkoAnimationPick);
		_state = kStatePick;
		return;
	}

	setAnimation(kOkoAnimationSink);
	setPosition(kOkoPositionX, kLevelPositionX[_level]);
	_state = kStateSink;

	_level++;
}

void Oko::raise() {
	if (_state != kStateSwim)
		return;

	if (_level == 0) {
		setAnimation(kOkoAnimationBreathe);
		_state = kStateBreathe;
		return;
	}

	setAnimation(kOkoAnimationRaise);
	setPosition(kOkoPositionX, kLevelPositionX[_level]);
	_state = kStateSink;

	_level--;
}

void Oko::hurt() {
	if (_state != kStateSwim)
		return;

	setAnimation(kOkoAnimationHurt);
	_state = kStateHurt;
}

void Oko::die() {
	if (_state != kStateSwim)
		return;

	setAnimation(kOkoAnimationDie0 + _level);
	_state = kStateDead;
}

Oko::State Oko::getState() const {
	return _state;
}

bool Oko::isBreathing() const {
	return (_state == kStateBreathe) && ((getFrame() >= 9) && (getFrame() <= 30));
}

bool Oko::isMoving() const {
	return (_state != kStateBreathe) && (_state != kStateHurt) && (_state != kStateDead);
}

} // End of namespace Geisha

} // End of namespace Gob
