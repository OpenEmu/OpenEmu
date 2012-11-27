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
#include "kyra/screen_v2.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/system.h"

namespace Kyra {

KyraEngine_v2::KyraEngine_v2(OSystem *system, const GameFlags &flags, const EngineDesc &desc) : KyraEngine_v1(system, flags), _desc(desc) {
	memset(&_sceneAnims, 0, sizeof(_sceneAnims));
	memset(&_sceneAnimMovie, 0, sizeof(_sceneAnimMovie));

	_lastProcessedSceneScript = 0;
	_specialSceneScriptRunFlag = false;

	_itemList = 0;
	_itemListSize = 0;

	_characterShapeFile = -1;

	_updateCharPosNextUpdate = 0;

	memset(&_sceneScriptState, 0, sizeof(_sceneScriptState));
	memset(&_sceneScriptData, 0, sizeof(_sceneScriptData));

	_animObjects = 0;

	_runFlag = true;
	_showOutro = false;
	_deathHandler = -1;
	_animNeedUpdate = false;

	_animShapeCount = 0;
	_animShapeFiledata = 0;

	_vocHigh = -1;
	_chatVocHigh = -1;
	_chatVocLow = -1;
	_chatText = 0;
	_chatObject = -1;
	_chatTextEnabled = false;

	memset(_hiddenItems, -1, sizeof(_hiddenItems));

	_screenBuffer = 0;

	memset(&_mainCharacter, 0, sizeof(_mainCharacter));
	memset(&_mainCharacter.inventory, -1, sizeof(_mainCharacter.inventory));

	_pauseStart = 0;

	_pathfinderFlag = 0;
	_smoothingPath = false;

	_lang = 0;
	Common::Language lang = Common::parseLanguage(ConfMan.get("language"));
	if (lang == _flags.fanLang && _flags.replacedLang != Common::UNK_LANG)
		lang = _flags.replacedLang;

	switch (lang) {
	case Common::EN_ANY:
	case Common::EN_USA:
	case Common::EN_GRB:
		_lang = 0;
		break;

	case Common::FR_FRA:
		_lang = 1;
		break;

	case Common::DE_DEU:
		_lang = 2;
		break;

	case Common::JA_JPN:
		_lang = 3;
		break;

	default:
		warning("unsupported language, switching back to English");
		_lang = 0;
	}
}

KyraEngine_v2::~KyraEngine_v2() {
	if (!(_flags.isDemo && !_flags.isTalkie)) {
		for (ShapeMap::iterator i = _gameShapes.begin(); i != _gameShapes.end(); ++i) {
			delete[] i->_value;
			i->_value = 0;
		}
		_gameShapes.clear();
	}

	delete[] _itemList;
	delete[] _sceneList;

	_emc->unload(&_sceneScriptData);

	delete[] _animObjects;

	for (Common::Array<const Opcode *>::iterator i = _opcodesAnimation.begin(); i != _opcodesAnimation.end(); ++i)
		delete *i;
	_opcodesAnimation.clear();

	delete[] _screenBuffer;
}

void KyraEngine_v2::pauseEngineIntern(bool pause) {
	KyraEngine_v1::pauseEngineIntern(pause);

	if (!pause) {
		uint32 pausedTime = _system->getMillis() - _pauseStart;

		for (int i = 0; i < ARRAYSIZE(_sceneSpecialScriptsTimer); ++i) {
			if (_sceneSpecialScriptsTimer[i])
				_sceneSpecialScriptsTimer[i] += pausedTime;
		}

	} else {
		_pauseStart = _system->getMillis();
	}
}

void KyraEngine_v2::delay(uint32 amount, bool updateGame, bool isMainLoop) {
	uint32 start = _system->getMillis();
	do {
		if (updateGame) {
			if (_chatText)
				updateWithText();
			else
				update();
		} else {
			updateInput();
		}

		if (amount > 0)
			_system->delayMillis(amount > 10 ? 10 : amount);
	} while (!skipFlag() && _system->getMillis() < start + amount && !shouldQuit());
}

bool KyraEngine_v2::checkSpecialSceneExit(int num, int x, int y) {
	if (_specialExitTable[0 + num] > x || _specialExitTable[5 + num] > y ||
	        _specialExitTable[10 + num] < x || _specialExitTable[15 + num] < y)
		return false;
	return true;
}

void KyraEngine_v2::addShapeToPool(const uint8 *data, int realIndex, int shape) {
	remShapeFromPool(realIndex);
	_gameShapes[realIndex] = screen_v2()->makeShapeCopy(data, shape);
}

void KyraEngine_v2::addShapeToPool(uint8 *shpData, int index) {
	remShapeFromPool(index);
	_gameShapes[index] = shpData;
}

void KyraEngine_v2::remShapeFromPool(int idx) {
	ShapeMap::iterator iter = _gameShapes.find(idx);
	if (iter != _gameShapes.end()) {
		delete[] iter->_value;
		iter->_value = 0;
	}
}

uint8 *KyraEngine_v2::getShapePtr(int shape) const {
	ShapeMap::iterator iter = _gameShapes.find(shape);
	if (iter == _gameShapes.end())
		return 0;
	return iter->_value;
}

void KyraEngine_v2::moveCharacter(int facing, int x, int y) {
	x &= ~3;
	y &= ~1;
	_mainCharacter.facing = facing;

	switch (facing) {
	case 0:
		while (_mainCharacter.y1 > y)
			updateCharPosWithUpdate();
		break;

	case 2:
		while (_mainCharacter.x1 < x)
			updateCharPosWithUpdate();
		break;

	case 4:
		while (_mainCharacter.y1 < y)
			updateCharPosWithUpdate();
		break;

	case 6:
		while (_mainCharacter.x1 > x)
			updateCharPosWithUpdate();
		break;

	default:
		break;
	}
}

void KyraEngine_v2::updateCharPosWithUpdate() {
	updateCharPos(0, 0);
	update();
}

int KyraEngine_v2::updateCharPos(int *table, int force) {
	if (_updateCharPosNextUpdate > _system->getMillis() && !force)
		return 0;
	_mainCharacter.x1 += _charAddXPosTable[_mainCharacter.facing];
	_mainCharacter.y1 += _charAddYPosTable[_mainCharacter.facing];
	updateCharAnimFrame(table);
	_updateCharPosNextUpdate = _system->getMillis() + getCharacterWalkspeed() * _tickLength;
	return 1;
}

} // End of namespace Kyra
