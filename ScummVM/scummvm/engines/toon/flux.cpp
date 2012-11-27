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

#include "common/debug.h"

#include "toon/flux.h"

namespace Toon {

CharacterFlux::CharacterFlux(ToonEngine *vm) : Character(vm) {
	_id = 1;
	_animationInstance = vm->getAnimationManager()->createNewInstance(kAnimationCharacter);
	_animationInstance->setUseMask(true);
	vm->getAnimationManager()->addInstance(_animationInstance);
}

CharacterFlux::~CharacterFlux() {
}

void CharacterFlux::playStandingAnim() {
	debugC(4, kDebugCharacter, "playStandingAnim()");

	_animationInstance->setAnimation(_walkAnim);
	_animationInstance->setFrame(_facing * 3);
	_animationInstance->setAnimationRange(_facing * 3, _facing * 3);
	_animationInstance->stopAnimation();
	_animationInstance->setLooping(true);

	//setVisible(true);
}

void CharacterFlux::setVisible(bool visible) {
	if (_vm->state()->_currentChapter == 2) {
		Character::setVisible(false);
	} else {
		Character::setVisible(visible);
	}
}

void CharacterFlux::playWalkAnim(int32 start, int32 end) {
	debugC(4, kDebugCharacter, "playWalkAnim(%d, %d)", start, end);

	_animationInstance->setAnimation(_walkAnim);
	_animationInstance->setAnimationRange(24 + _facing * 10, 24 + _facing * 10 + 9);
	_animationInstance->playAnimation();
	_animationInstance->setFps(16);
	_animationInstance->setLooping(true);
}

int32 CharacterFlux::fixFacingForAnimation(int32 originalFacing, int32 animationId) {

	static const byte fixFluxAnimationFacing[] = {
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xee, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x55,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	byte animFacingFlag = fixFluxAnimationFacing[animationId];
	int32 v5 = 1 << originalFacing;
	int32 v6 = 1 << originalFacing;
	int32 facingMask = 0;
	do {
		if (v6 & animFacingFlag) {
			facingMask = v6;
		} else if (v5 & animFacingFlag) {
			facingMask = v5;
		}
		v5 >>= 1;
		v6 <<= 1;
	}
	while (!facingMask);

	int32 finalFacing = 0;
	for (finalFacing = 0; ; ++finalFacing) {
		facingMask >>= 1;
		if (!facingMask)
			break;
	}

	return finalFacing;
}

void CharacterFlux::setPosition(int16 x, int16 y) {
	debugC(5, kDebugCharacter, "setPosition(%d, %d)", x, y);

	_z = _vm->getLayerAtPoint(x, y);
	_scale = _vm->getScaleAtPoint(x, y);
	int32 width = _walkAnim->getWidth() * _scale / 1024;
	int32 height = 165 * _scale / 1024;
	_animationInstance->setPosition(x - width / 2, y - height, _z , false);
	_animationInstance->setScale(_scale);

	// in original code, flux shadow scale is 3/4 of real scale
	int32 shadowScale = _scale * 3 / 4;

	// work out position and scale of the shadow below character
	int32 shadowWidth = _shadowAnim->getWidth() * shadowScale / 1024;
	int32 shadowHeight = _shadowAnim->getHeight() * shadowScale / 1024;
	_shadowAnimationInstance->setPosition(x - shadowWidth / 2, y - shadowHeight / 2 , _z , false);
	_shadowAnimationInstance->setScale(shadowScale);
	_x = x;
	_y = y;
	_finalX = x;
	_finalY = y;
	_animationInstance->setLayerZ(_y);
}

void CharacterFlux::update(int32 timeIncrement) {
	debugC(5, kDebugCharacter, "update(%d)", timeIncrement);
	Character::update(timeIncrement);
	setPosition(_x, _y);
}

int32 CharacterFlux::getRandomIdleAnim() {
	debugC(3, kDebugCharacter, "getRandomIdleAnim()");
	static const int32 idle[] = { 0xe, 0xf, 0x21, 0x22, 0x24, 0x25, 0x27 };
	return idle[_vm->randRange(0, 6)];
}

} // End of namespace Toon
