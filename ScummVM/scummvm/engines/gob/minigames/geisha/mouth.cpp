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

#include "common/util.h"

#include "gob/minigames/geisha/mouth.h"

namespace Gob {

namespace Geisha {

Mouth::Mouth(const ANIFile &ani, const CMPFile &cmp,
             uint16 mouthAnim, uint16 mouthSprite, uint16 floorSprite) : ANIObject(ani) {

	_sprite = new ANIObject(cmp);
	_sprite->setAnimation(mouthSprite);
	_sprite->setVisible(true);

	for (int i = 0; i < kFloorCount; i++) {
		_floor[i] = new ANIObject(cmp);
		_floor[i]->setAnimation(floorSprite);
		_floor[i]->setVisible(true);
	}

	_state = kStateDeactivated;

	setAnimation(mouthAnim);
	setMode(kModeOnce);
	setPause(true);
	setVisible(true);
}

Mouth::~Mouth() {
	for (int i = 0; i < kFloorCount; i++)
		delete _floor[i];

	delete _sprite;
}

void Mouth::advance() {
	if (_state != kStateActivated)
		return;

	// Animation finished, set state to dead
	if (isPaused()) {
		_state = kStateDead;
		return;
	}

	ANIObject::advance();
}

void Mouth::activate() {
	if (_state != kStateDeactivated)
		return;

	_state = kStateActivated;

	setPause(false);
}

bool Mouth::isDeactivated() const {
	return _state == kStateDeactivated;
}

void Mouth::setPosition(int16 x, int16 y) {
	ANIObject::setPosition(x, y);

	int16 floorWidth, floorHeight;
	_floor[0]->getFrameSize(floorWidth, floorHeight);

	_sprite->setPosition(x, y);

	for (int i = 0; i < kFloorCount; i++)
		_floor[i]->setPosition(x + (i * floorWidth), y);
}

bool Mouth::draw(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom) {
	// If the mouth is deactivated, draw the default mouth sprite
	if (_state == kStateDeactivated)
		return _sprite->draw(dest, left, top, right, bottom);

	// If the mouth is activated, draw the current mouth animation sprite
	if (_state == kStateActivated)
		return ANIObject::draw(dest, left, top, right, bottom);

	// If the mouth is dead, draw the floor tiles
	if (_state == kStateDead) {
		int16 fLeft, fRight, fTop, fBottom;
		bool drawn = false;

		left   = 0x7FFF;
		top    = 0x7FFF;
		right  =      0;
		bottom =      0;

		for (int i = 0; i < kFloorCount; i++) {
			if (_floor[i]->draw(dest, fLeft, fTop, fRight, fBottom)) {
				drawn  = true;
				left   = MIN(left  , fLeft);
				top    = MIN(top   , fTop);
				right  = MAX(right , fRight);
				bottom = MAX(bottom, fBottom);
			}
		}

		return drawn;
	}

	return false;
}

bool Mouth::clear(Surface &dest, int16 &left , int16 &top, int16 &right, int16 &bottom) {
	// If the mouth is deactivated, clear the default mouth sprite
	if (_state == kStateDeactivated)
		return _sprite->clear(dest, left, top, right, bottom);

	// If the mouth is activated, clear the current mouth animation sprite
	if (_state == kStateActivated)
		return ANIObject::clear(dest, left, top, right, bottom);

	// If the mouth is clear, draw the floor tiles
	if (_state == kStateDead) {
		int16 fLeft, fRight, fTop, fBottom;
		bool cleared = false;

		left   = 0x7FFF;
		top    = 0x7FFF;
		right  =      0;
		bottom =      0;

		for (int i = 0; i < kFloorCount; i++) {
			if (_floor[i]->clear(dest, fLeft, fTop, fRight, fBottom)) {
				cleared  = true;
				left   = MIN(left  , fLeft);
				top    = MIN(top   , fTop);
				right  = MAX(right , fRight);
				bottom = MAX(bottom, fBottom);
			}
		}

		return cleared;
	}

	return false;
}

} // End of namespace Geisha

} // End of namespace Gob
