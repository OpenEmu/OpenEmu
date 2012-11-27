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

#include "toon/drew.h"

namespace Toon {

CharacterDrew::CharacterDrew(ToonEngine *vm) : Character(vm) {
	_id = 0;
	_blockingWalk = true;
	_animationInstance = vm->getAnimationManager()->createNewInstance(kAnimationCharacter);
	_animationInstance->setUseMask(true);
	vm->getAnimationManager()->addInstance(_animationInstance);
	_currentScale = 1024;
}

CharacterDrew::~CharacterDrew() {
}

bool CharacterDrew::setupPalette() {
	debugC(1, kDebugCharacter, "setupPalette()");

	if (_walkAnim) {
		_walkAnim->applyPalette(129, 129 * 3, 63);
		return true;
	}
	return false;
}

void CharacterDrew::setPosition(int16 x, int16 y) {
	debugC(5, kDebugCharacter, "setPosition(%d, %d)", x, y);

	_z = _vm->getLayerAtPoint(x, y);
	int newScale = _vm->getScaleAtPoint(x, y);
	if (newScale > 0)
		_scale = newScale;

	// work out position and scale of the character sprite
	int32 width = _walkAnim->getWidth() * _scale / 1024;
	int32 height = 210 * _scale / 1024;
	_animationInstance->setPosition(x - width / 2, y - height, _z , false);
	_animationInstance->setScale(_scale);

	// work out position and scale of the shadow below character
	int32 shadowWidth = _shadowAnim->getWidth() * _scale / 1024;
	int32 shadowHeight = _shadowAnim->getHeight() * _scale / 1024;

	_shadowAnimationInstance->setPosition(x - shadowWidth / 2, y - shadowHeight / 2 - 4 , _z , false);
	_shadowAnimationInstance->setScale(_scale);

	_x = x;
	_y = y;
	_animationInstance->setLayerZ(_y);
}

void CharacterDrew::playStandingAnim() {
	debugC(4, kDebugCharacter, "playStandingAnim()");

	stopSpecialAnim();
	_animationInstance->setAnimation(_walkAnim);
	_animationInstance->setFrame(_facing * 2);
	_shadowAnimationInstance->setFrame(_facing);
	_animationInstance->setAnimationRange(_facing * 2, _facing * 2);
	_animationInstance->stopAnimation();
	_animationInstance->setLooping(true);
	//setVisible(true);
}

void CharacterDrew::playWalkAnim(int32 start, int32 end) {
	debugC(4, kDebugCharacter, "playWalkAnim(%d, %d)", start, end);

	stopSpecialAnim();
	_animationInstance->setAnimation(_walkAnim);
	_shadowAnimationInstance->setFrame(_facing);
	_animationInstance->setAnimationRange(16 + _facing * 14, 16 + _facing * 14 + 13);
	_animationInstance->playAnimation();
	_animationInstance->setFps(16);
	_animationInstance->setLooping(true);

	//setVisible(true);
}

void CharacterDrew::update(int32 timeIncrement) {
	debugC(5, kDebugCharacter, "update(%d)", timeIncrement);
	Character::update(timeIncrement);
	if (_currentScale > _scale) {
		_scale += timeIncrement * 2;
		if (_scale > _currentScale)
			_scale = _currentScale;
	} else if (_currentScale < _scale) {
		_scale -= timeIncrement * 2;
		if (_scale < _currentScale)
			_scale = _currentScale;
	}
	setPosition(_x, _y);
}

int32 CharacterDrew::getRandomIdleAnim() {
	debugC(3, kDebugCharacter, "getRandomIdleAnim()");

	static const int32 idle[] = { 6, 9, 10, 11, 12 };
	return idle[_vm->randRange(0, 4)];
}

void CharacterDrew::resetScale()
{
	_scale = _currentScale;
	setPosition(_x, _y);
}
} // End of namespace Toon
