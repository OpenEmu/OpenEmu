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

#include "gob/minigames/geisha/submarine.h"

namespace Gob {

namespace Geisha {

enum Animation {
	kAnimationDriveS   =  4,
	kAnimationDriveE   =  5,
	kAnimationDriveN   =  6,
	kAnimationDriveW   =  7,
	kAnimationDriveSE  =  8,
	kAnimationDriveNE  =  9,
	kAnimationDriveSW  = 10,
	kAnimationDriveNW  = 11,
	kAnimationShootS   = 12,
	kAnimationShootN   = 13,
	kAnimationShootW   = 14,
	kAnimationShootE   = 15,
	kAnimationShootNE  = 16,
	kAnimationShootSE  = 17,
	kAnimationShootSW  = 18,
	kAnimationShootNW  = 19,
	kAnimationExplodeN = 28,
	kAnimationExplodeS = 29,
	kAnimationExplodeW = 30,
	kAnimationExplodeE = 31,
	kAnimationExit     = 32
};


Submarine::Submarine(const ANIFile &ani) : ANIObject(ani), _state(kStateMove), _direction(kDirectionNone) {
	turn(kDirectionN);
}

Submarine::~Submarine() {
}

Submarine::Direction Submarine::getDirection() const {
	return _direction;
}

void Submarine::turn(Direction to) {
	// Nothing to do
	if ((to == kDirectionNone) || ((_state == kStateMove) && (_direction == to)))
		return;

	_direction = to;

	move();
}

void Submarine::move() {
	uint16 frame = getFrame();
	uint16 anim  = (_state == kStateShoot) ? directionToShoot(_direction) : directionToMove(_direction);

	setAnimation(anim);
	setFrame(frame);
	setPause(false);
	setVisible(true);

	setMode((_state == kStateShoot) ? kModeOnce : kModeContinuous);
}

void Submarine::shoot() {
	_state = kStateShoot;

	setAnimation(directionToShoot(_direction));
	setMode(kModeOnce);
	setPause(false);
	setVisible(true);
}

void Submarine::die() {
	if (!canMove())
		return;

	_state = kStateDie;

	setAnimation(directionToExplode(_direction));
	setMode(kModeOnce);
	setPause(false);
	setVisible(true);
}

void Submarine::leave() {
	_state = kStateExit;

	setAnimation(kAnimationExit);
	setMode(kModeOnce);
	setPause(false);
	setVisible(true);
}

void Submarine::advance() {
	ANIObject::advance();

	switch (_state) {
	case kStateShoot:
		if (isPaused()) {
			_state = kStateMove;

			move();
		}
		break;

	case kStateExit:
		if (isPaused())
			_state = kStateExited;

		break;

	case kStateDie:
		if (isPaused())
			_state = kStateDead;
		break;

	default:
		break;
	}
}

bool Submarine::canMove() const {
	return (_state == kStateMove) || (_state == kStateShoot);
}

bool Submarine::isDead() const {
	return _state == kStateDead;
}

bool Submarine::isShooting() const {
	return _state == kStateShoot;
}

bool Submarine::hasExited() const {
	return _state == kStateExited;
}

uint16 Submarine::directionToMove(Direction direction) const {
	switch (direction) {
	case kDirectionN:
		return kAnimationDriveN;

	case kDirectionNE:
		return kAnimationDriveNE;

	case kDirectionE:
		return kAnimationDriveE;

	case kDirectionSE:
		return kAnimationDriveSE;

	case kDirectionS:
		return kAnimationDriveS;

	case kDirectionSW:
		return kAnimationDriveSW;

	case kDirectionW:
		return kAnimationDriveW;

	case kDirectionNW:
		return kAnimationDriveNW;

	default:
		break;
	}

	return 0;
}

uint16 Submarine::directionToShoot(Direction direction) const {
	switch (direction) {
	case kDirectionN:
		return kAnimationShootN;

	case kDirectionNE:
		return kAnimationShootNE;

	case kDirectionE:
		return kAnimationShootE;

	case kDirectionSE:
		return kAnimationShootSE;

	case kDirectionS:
		return kAnimationShootS;

	case kDirectionSW:
		return kAnimationShootSW;

	case kDirectionW:
		return kAnimationShootW;

	case kDirectionNW:
		return kAnimationShootNW;

	default:
		break;
	}

	return 0;
}

uint16 Submarine::directionToExplode(Direction direction) const {
	// Only 4 exploding animations (spinning clockwise)

	switch (direction) {
	case kDirectionNW:
	case kDirectionN:
		return kAnimationExplodeN;

	case kDirectionNE:
	case kDirectionE:
		return kAnimationExplodeE;

	case kDirectionSE:
	case kDirectionS:
		return kAnimationExplodeS;

	case kDirectionSW:
	case kDirectionW:
		return kAnimationExplodeW;

	default:
		break;
	}

	return 0;
}

} // End of namespace Geisha

} // End of namespace Gob
