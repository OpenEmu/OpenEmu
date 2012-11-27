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

#include "gob/surface.h"
#include "gob/anifile.h"

#include "gob/pregob/onceupon/chargenchild.h"

enum Animation {
	kAnimWalkLeft  =  0,
	kAnimWalkRight =  1,
	kAnimJumpLeft  =  2,
	kAnimJumpRight =  3,
	kAnimTapFoot   = 14
};

namespace Gob {

namespace OnceUpon {

CharGenChild::CharGenChild(const ANIFile &ani) : ANIObject(ani) {
	setPosition(265, 110);
	setAnimation(kAnimWalkLeft);
	setVisible(true);
	setPause(false);
}

CharGenChild::~CharGenChild() {
}

void CharGenChild::advance() {
	bool wasLastFrame = lastFrame();

	ANIObject::advance();

	int16 x, y, left, top, width, height;
	getPosition(x, y);
	getFramePosition(left, top);
	getFrameSize(width, height);

	const int16 right = left + width - 1;

	switch (getAnimation()) {
	case kAnimWalkLeft:
		if (left <= 147)
			setAnimation(kAnimWalkRight);
		break;

	case kAnimWalkRight:
		if (right >= 290) {
			setAnimation(kAnimJumpLeft);

			setPosition(x, y - 14);
		}
		break;

	case kAnimJumpLeft:
		if (wasLastFrame) {
			setAnimation(kAnimTapFoot);

			setPosition(x, y - 10);
		}
		break;

	case kAnimTapFoot:
		if (wasLastFrame) {
			setAnimation(kAnimJumpRight);

			setPosition(x, y + 10);
		}
		break;

	case kAnimJumpRight:
		if (wasLastFrame) {
			setAnimation(kAnimWalkLeft);

			setPosition(x, y + 14);
		}
		break;
	}
}

CharGenChild::Sound CharGenChild::shouldPlaySound() const {
	const uint16 anim  = getAnimation();
	const uint16 frame = getFrame();

	if (((anim == kAnimWalkLeft) || (anim == kAnimWalkRight)) && ((frame == 1) || (frame == 6)))
		return kSoundWalk;

	if (((anim == kAnimJumpLeft) || (anim == kAnimJumpRight)) &&  (frame == 0))
		return kSoundJump;

	return kSoundNone;
}

} // End of namespace OnceUpon

} // End of namespace Gob
