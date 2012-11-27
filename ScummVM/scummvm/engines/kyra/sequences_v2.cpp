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

#include "kyra/kyra_v2.h"
#include "kyra/resource.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_v2::runAnimationScript(const char *filename, int allowSkip, int resetChar, int newShapes, int shapeUnload) {
	memset(&_animationScriptData, 0, sizeof(_animationScriptData));
	memset(&_animationScriptState, 0, sizeof(_animationScriptState));

	if (!_emc->load(filename, &_animationScriptData, &_opcodesAnimation))
		error("Couldn't load temporary script '%s'", filename);

	_emc->init(&_animationScriptState, &_animationScriptData);
	_emc->start(&_animationScriptState, 0);

	_animResetFrame = -1;

	if (_animShapeFiledata && newShapes) {
		uninitAnimationShapes(_animShapeCount, _animShapeFiledata);
		_animShapeFiledata = 0;
		_animShapeCount = 0;
	}

	while (_emc->isValid(&_animationScriptState))
		_emc->run(&_animationScriptState);

	uint8 *fileData = 0;

	if (newShapes)
		_animShapeFiledata = _res->fileData(_animShapeFilename, 0);

	fileData = _animShapeFiledata;

	if (!fileData) {
		_emc->unload(&_animationScriptData);
		return;
	}

	if (newShapes)
		_animShapeCount = initAnimationShapes(fileData);

	processAnimationScript(allowSkip, resetChar);

	if (shapeUnload) {
		uninitAnimationShapes(_animShapeCount, fileData);
		_animShapeCount = 0;
		_animShapeFiledata = 0;
	}

	_emc->unload(&_animationScriptData);
}

void KyraEngine_v2::processAnimationScript(int allowSkip, int resetChar) {
	setCharacterAnimDim(_animShapeWidth, _animShapeHeight);

	_emc->init(&_animationScriptState, &_animationScriptData);
	_emc->start(&_animationScriptState, 1);

	resetSkipFlag();

	while (_emc->isValid(&_animationScriptState)) {
		_animNeedUpdate = false;
		while (_emc->isValid(&_animationScriptState) && !_animNeedUpdate)
			_emc->run(&_animationScriptState);

		if (_animNewFrame < 0)
			continue;

		_mainCharacter.animFrame = _animNewFrame + _desc.animScriptFrameAdd;
		updateCharacterAnim(0);
		if (_chatText)
			updateWithText();
		else
			update();

		uint32 delayEnd = _system->getMillis() + _animDelayTime * _tickLength;

		while ((!skipFlag() || !allowSkip) && _system->getMillis() < delayEnd)
			delay(10, true);

		if (skipFlag()) {
			resetSkipFlag();
			if (allowSkip)
				break;
		}
	}

	if (resetChar) {
		if (_animResetFrame >= 0) {
			_mainCharacter.animFrame = _animResetFrame + _desc.animScriptFrameAdd;
			updateCharacterAnim(0);
			if (_chatText)
				updateWithText();
			else
				update();
		}

		_mainCharacter.animFrame = _desc.characterFrameTable[_mainCharacter.facing];
		updateCharacterAnim(0);
	}

	_animResetFrame = -1;
	resetCharacterAnimDim();
}

} // End of namespace Kyra
