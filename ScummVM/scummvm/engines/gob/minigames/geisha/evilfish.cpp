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

#include "gob/minigames/geisha/evilfish.h"

namespace Gob {

namespace Geisha {

EvilFish::EvilFish(const ANIFile &ani, uint16 screenWidth,
                   uint16 animSwimLeft, uint16 animSwimRight,
                   uint16 animTurnLeft, uint16 animTurnRight, uint16 animDie) :
	ANIObject(ani), _screenWidth(screenWidth),
	_animSwimLeft(animSwimLeft), _animSwimRight(animSwimRight),
	_animTurnLeft(animTurnLeft), _animTurnRight(animTurnRight), _animDie(animDie),
	_shouldLeave(false), _state(kStateNone) {

}

EvilFish::~EvilFish() {
}

void EvilFish::enter(Direction from, int16 y) {
	_shouldLeave = false;

	bool left = from == kDirectionLeft;

	setAnimation(left ? _animSwimLeft : _animSwimRight);

	int16 width, height;
	getFrameSize(width, height);

	setPosition(left ? -width : _screenWidth, y);
	setVisible(true);

	_state = left ? kStateSwimLeft : kStateSwimRight;
}

void EvilFish::leave() {
	if (_state == kStateNone)
		return;

	_shouldLeave = true;
}

void EvilFish::die() {
	if ((_state == kStateNone) || (_state == kStateDie))
		return;

	int16 x, y;
	getFramePosition(x, y);

	setAnimation(_animDie);
	setPosition(x, y);

	_state = kStateDie;
}

void EvilFish::advance() {
	if (_state == kStateNone)
		return;

	bool wasLastFrame = lastFrame();

	int16 oldX, oldY;
	getPosition(oldX, oldY);

	ANIObject::advance();

	int16 x, y, width, height;
	getFramePosition(x, y);
	getFrameSize(width, height);

	switch (_state) {
	case kStateNone:
		break;

	case kStateSwimLeft:
		if (!_shouldLeave && (x >= _screenWidth - width)) {
			setAnimation(_animTurnRight);
			setPosition(x, oldY);
			_state = kStateTurnRight;
		}

		if (_shouldLeave && (x >= _screenWidth)) {
			setVisible(false);

			_shouldLeave = false;
			_state       = kStateNone;
		}
		break;

	case kStateSwimRight:
		if (!_shouldLeave && (x <= 0)) {
			setAnimation(_animTurnLeft);
			setPosition(x, oldY);
			_state = kStateTurnLeft;
		}

		if (_shouldLeave && (x < -width)) {
			setVisible(false);

			_shouldLeave = false;
			_state       = kStateNone;
		}
		break;

	case kStateTurnLeft:
		if (wasLastFrame) {
			setAnimation(_animSwimLeft);
			_state = kStateSwimLeft;
		}
		break;

	case kStateTurnRight:
		if (wasLastFrame) {
			setAnimation(_animSwimRight);
			_state = kStateSwimRight;
		}
		break;

	case kStateDie:
		if (wasLastFrame) {
			setVisible(false);

			_state = kStateNone;
		}
		break;
	}
}

void EvilFish::mutate(uint16 animSwimLeft, uint16 animSwimRight,
                      uint16 animTurnLeft, uint16 animTurnRight, uint16 animDie) {

	_animSwimLeft  = animSwimLeft;
	_animSwimRight = animSwimRight;
	_animTurnLeft  = animTurnLeft;
	_animTurnRight = animTurnRight;
	_animDie       = animDie;

	switch (_state) {
	case kStateSwimLeft:
		setAnimation(_animSwimLeft);
		break;

	case kStateSwimRight:
		setAnimation(_animSwimRight);
		break;

	default:
		break;
	}
}

bool EvilFish::isDead() const {
	return !isVisible() || (_state == kStateNone) || (_state == kStateDie);
}

} // End of namespace Geisha

} // End of namespace Gob
