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

#ifdef ENABLE_LOL

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/timer.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {

void LoLEngine::runInitScript(const char *filename, int optionalFunc) {
	_suspendScript = true;
	EMCData scriptData;
	EMCState scriptState;
	memset(&scriptData, 0, sizeof(EMCData));
	_emc->unload(&_scriptData);
	_emc->load(filename, &scriptData, &_opcodes);

	_emc->init(&scriptState, &scriptData);
	_emc->start(&scriptState, 0);
	while (_emc->isValid(&scriptState))
		_emc->run(&scriptState);

	if (optionalFunc) {
		_emc->init(&scriptState, &scriptData);
		_emc->start(&scriptState, optionalFunc);
		while (_emc->isValid(&scriptState))
			_emc->run(&scriptState);
	}

	_emc->unload(&scriptData);
	_suspendScript = false;
}

void LoLEngine::runInfScript(const char *filename) {
	_emc->unload(&_scriptData);
	_emc->load(filename, &_scriptData, &_opcodes);
	runLevelScript(0x400, -1);
}

void LoLEngine::runLevelScript(int block, int flags) {
	runLevelScriptCustom(block, flags, -1, 0, 0, 0);
}

void LoLEngine::runLevelScriptCustom(int block, int flags, int charNum, int item, int reg3, int reg4) {
	EMCState scriptState;
	memset(&scriptState, 0, sizeof(EMCState));

	if (!_suspendScript) {
		_emc->init(&scriptState, &_scriptData);
		_emc->start(&scriptState, block);

		scriptState.regs[0] = flags;
		scriptState.regs[1] = charNum;
		scriptState.regs[2] = item;
		scriptState.regs[3] = reg3;
		scriptState.regs[4] = reg4;
		scriptState.regs[5] = block;
		scriptState.regs[6] = _scriptDirection;

		if (_emc->isValid(&scriptState)) {
			if (*(scriptState.ip - 1) & flags) {
				while (_emc->isValid(&scriptState))
					_emc->run(&scriptState);
			}
		}
	}

	checkSceneUpdateNeed(block);
}

int LoLEngine::olol_setWallType(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setWallType(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	if (stackPos(2) != -1) {
		if (_wllWallFlags[stackPos(2)] & 4)
			deleteMonstersFromBlock(stackPos(0));
	}
	setWallType(stackPos(0), stackPos(1), stackPos(2));
	return 1;
}

int LoLEngine::olol_getWallType(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getWallType(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return (int8)_levelBlockProperties[stackPos(0)].walls[stackPos(1) & 3];
}

int LoLEngine::olol_drawScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_drawScene(%p) (%d)", (const void *)script, stackPos(0));
	drawScene(stackPos(0));
	return 1;
}

int LoLEngine::olol_rollDice(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_rollDice(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return rollDice(stackPos(0), stackPos(1));
}

int LoLEngine::olol_moveParty(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_moveParty(%p) (%d)", (const void *)script, stackPos(0));
	int mode = stackPos(0);
	if (mode > 5 && mode < 10)
		mode = (mode - 6 - _currentDirection) & 3;

	Button b;
	b.data0Val2 = b.data1Val2 = b.data2Val2 = 0xfe;
	b.data0Val3 = b.data1Val3 = b.data2Val3 = 0x01;

	switch (mode) {
	case 0:
		clickedUpArrow(&b);
		break;

	case 1:
		clickedRightArrow(&b);
		break;

	case 2:
		clickedDownArrow(&b);
		break;

	case 3:
		clickedLeftArrow(&b);
		break;

	case 4:
		clickedTurnLeftArrow(&b);
		break;

	case 5:
		clickedTurnRightArrow(&b);
		break;

	case 10:
	case 11:
	case 12:
	case 13:
		mode = ABS(mode - 10 - _currentDirection);
		if (mode > 2)
			mode = (mode ^ 2) * -1;

		while (mode) {
			if (mode > 0) {
				clickedTurnRightArrow(&b);
				mode--;
			} else {
				clickedTurnLeftArrow(&b);
				mode++;
			}
		}
		break;

	default:
		break;
	}

	return 1;
}


int LoLEngine::olol_delay(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_delay(%p) (%d)", (const void *)script, stackPos(0));
	delay(stackPos(0) * _tickLength, true);
	return 1;
}

int LoLEngine::olol_setGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setGameFlag(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));

	if (stackPos(1))
		setGameFlag(stackPos(0));
	else
		resetGameFlag(stackPos(0));

	return 1;
}

int LoLEngine::olol_testGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_testGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	if (stackPos(0) < 0)
		return 0;

	return queryGameFlag(stackPos(0));
}

int LoLEngine::olol_loadLevelGraphics(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadLevelGraphics(%p) (%s, %d, %d, %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	loadLevelGraphics(stackPosString(0), stackPos(1), stackPos(2), stackPos(3) == -1 ? -1 : (uint16)stackPos(3), stackPos(4) == -1 ? -1 : (uint16)stackPos(4), (stackPos(5) == -1) ? 0 : stackPosString(5));
	return 1;
}

int LoLEngine::olol_loadBlockProperties(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadBlockProperties(%p) (%s)", (const void *)script, stackPosString(0));
	loadBlockProperties(stackPosString(0));
	return 1;
}

int LoLEngine::olol_loadMonsterShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadMonsterShapes(%p) (%s, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2));
	loadMonsterShapes(stackPosString(0), stackPos(1), stackPos(2));
	return 1;
}

int LoLEngine::olol_deleteHandItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_deleteHandItem(%p) ()", (const void *)script);
	int r = _itemInHand;
	deleteItem(_itemInHand);
	setHandItem(0);
	return r;
}

int LoLEngine::olol_allocItemPropertiesBuffer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_allocItemPropertiesBuffer(%p) (%d)", (const void *)script, stackPos(0));
	delete[] _itemProperties;
	_itemProperties = new ItemProperty[stackPos(0)];
	return 1;
}

int LoLEngine::olol_setItemProperty(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setItemProperty(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));
	ItemProperty *tmp = &_itemProperties[stackPos(0)];

	tmp->nameStringId = stackPos(1);
	tmp->shpIndex = stackPos(2);
	tmp->type = stackPos(3);

	// WORKAROUND for unpatched early floppy versions.
	// The Vaelan's cube should not be able to be equipped in a weapon slot.
	if (stackPos(0) == 264 && tmp->type == 5)
		tmp->type = 0;

	tmp->itemScriptFunc = stackPos(4);
	tmp->might = stackPos(5);
	tmp->skill = stackPos(6);
	tmp->protection = stackPos(7);
	tmp->flags = stackPos(8);
	tmp->unkB = stackPos(9);
	return 1;
}

int LoLEngine::olol_makeItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_makeItem(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	return makeItem(stackPos(0), stackPos(1), stackPos(2));
}

int LoLEngine::olol_placeMoveLevelItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_placeMoveLevelItem(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	placeMoveLevelItem(stackPos(0), stackPos(1), stackPos(2), stackPos(3) & 0xff, stackPos(4) & 0xff, stackPos(5));
	return 1;
}

int LoLEngine::olol_createLevelItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_createLevelItem(%p) (%d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	int item = makeItem(stackPos(0), stackPos(1), stackPos(2));
	if (item == -1)
		return item;
	placeMoveLevelItem(item, stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	return item;
}

int LoLEngine::olol_getItemPara(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getItemPara(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (!stackPos(0))
		return 0;

	LoLItem *i = &_itemsInPlay[stackPos(0)];
	ItemProperty *p = &_itemProperties[i->itemPropertyIndex];

	switch (stackPos(1)) {
	case 0:
		return i->block;
	case 1:
		return i->x;
	case 2:
		return i->y;
	case 3:
		return i->level;
	case 4:
		return i->itemPropertyIndex;
	case 5:
		return i->shpCurFrame_flg;
	case 6:
		return p->nameStringId;
	case 7:
		break;
	case 8:
		return p->shpIndex;
	case 9:
		return p->type;
	case 10:
		return p->itemScriptFunc;
	case 11:
		return p->might;
	case 12:
		return p->skill;
	case 13:
		return p->protection;
	case 14:
		return p->unkB;
	case 15:
		return i->shpCurFrame_flg & 0x1fff;
	case 16:
		return p->flags;
	case 17:
		return (p->skill << 8) | ((uint8)p->might);
	default:
		break;
	}

	return -1;
}

int LoLEngine::olol_getCharacterStat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getCharacterStat(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	LoLCharacter *c = &_characters[stackPos(0)];
	int d = stackPos(2);

	switch (stackPos(1)) {
	case 0:
		return c->flags;

	case 1:
		return c->raceClassSex;

	case 5:
		return c->hitPointsCur;

	case 6:
		return c->hitPointsMax;

	case 7:
		return c->magicPointsCur;

	case 8:
		return c->magicPointsMax;

	case 9:
		return c->itemProtection;

	case 10:
		return c->items[d];

	case 11:
		return c->skillLevels[d] + c->skillModifiers[d];

	case 12:
		return c->protectionAgainstItems[d];

	case 13:
		return (d & 0x80) ? c->itemsMight[7] : c->itemsMight[d];

	case 14:
		return c->skillModifiers[d];

	case 15:
		return c->id;

	default:
		break;
	}

	return 0;
}

int LoLEngine::olol_setCharacterStat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setCharacterStat(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	LoLCharacter *c = &_characters[stackPos(0)];
	int d = stackPos(2);
	int e = stackPos(3);

	switch (stackPos(1)) {
	case 0:
		c->flags = e;
		break;

	case 1:
		c->raceClassSex = e & 0x0f;
		break;

	case 5:
		setCharacterMagicOrHitPoints(stackPos(0), 0, e, 0);
		break;

	case 6:
		c->hitPointsMax = e;
		break;

	case 7:
		setCharacterMagicOrHitPoints(stackPos(0), 1, e, 0);
		break;

	case 8:
		c->magicPointsMax = e;
		break;

	case 9:
		c->itemProtection = e;
		break;

	case 10:
		c->items[d] = 0;
		break;

	case 11:
		c->skillLevels[d] = e;
		break;

	case 12:
		c->protectionAgainstItems[d] = e;
		break;

	case 13:
		if (d & 0x80)
			c->itemsMight[7] = e;
		else
			c->itemsMight[d] = e;
		break;

	case 14:
		c->skillModifiers[d] = e;
		break;

	default:
		break;
	}

	return 0;
}

int LoLEngine::olol_loadLevelShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadLevelShapes(%p) (%s, %s)", (const void *)script, stackPosString(0), stackPosString(1));
	loadLevelShpDat(stackPosString(0), stackPosString(1), true);
	return 1;
}

int LoLEngine::olol_closeLevelShapeFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_closeLevelShapeFile(%p) ()", (const void *)script);
	delete _lvlShpFileHandle;
	_lvlShpFileHandle = 0;
	return 1;
}

int LoLEngine::olol_loadDoorShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadDoorShapes(%p) (%s, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2));
	_screen->loadBitmap(stackPosString(0), 3, 3, 0);
	const uint8 *p = _screen->getCPagePtr(2);
	if (_doorShapes[0])
		delete[] _doorShapes[0];
	_doorShapes[0] = _screen->makeShapeCopy(p, stackPos(1));
	if (_doorShapes[1])
		delete[] _doorShapes[1];
	_doorShapes[1] = _screen->makeShapeCopy(p, stackPos(2));

	for (int i = 0; i < 20; i++) {
		_wllWallFlags[i + 3] |= 7;
		int t = i % 5;
		if (t == 4)
			_wllWallFlags[i + 3] &= 0xf8;
		if (t == 3)
			_wllWallFlags[i + 3] &= 0xfd;
	}

	if (stackPos(3)) {
		for (int i = 3; i < 13; i++)
			_wllWallFlags[i] &= 0xfd;
	}

	if (stackPos(4)) {
		for (int i = 13; i < 23; i++)
			_wllWallFlags[i] &= 0xfd;
	}

	return 1;
}

int LoLEngine::olol_initAnimStruct(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_initAnimStruct(%p) (%s, %d, %d, %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	if (_tim->initAnimStruct(stackPos(1), stackPosString(0), stackPos(2), stackPos(3), stackPos(4), 0, stackPos(5)))
		return 1;
	return 0;
}

int LoLEngine::olol_playAnimationPart(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playAnimationPart(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_tim->animator()->playPart(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return 1;
}

int LoLEngine::olol_freeAnimStruct(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_freeAnimStruct(%p) (%d)", (const void *)script, stackPos(0));
	if (_tim->freeAnimStruct(stackPos(0)))
		return 1;
	return 0;
}

int LoLEngine::olol_getDirection(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getDirection(%p)", (const void *)script);
	return _currentDirection;
}

int LoLEngine::olol_characterSurpriseFeedback(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_characterSurpriseFeedback(%p)", (const void *)script);
	for (int i = 0; i < 4; i++) {
		if (!(_characters[i].flags & 1) || _characters[i].id >= 0)
			continue;
		int s = -_characters[i].id;
		int sfx = (s == 1) ? 136 : ((s == 5) ? 50 : ((s == 8) ? 49 : ((s == 9) ? 48 : 0)));
		if (sfx)
			snd_playSoundEffect(sfx, -1);
		return 1;
	}
	return 1;
}

int LoLEngine::olol_setMusicTrack(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setMusicTrack(%p) (%d)", (const void *)script, stackPos(0));
	_curMusicTheme = stackPos(0);
	return 1;
}

int LoLEngine::olol_setSequenceButtons(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setSequenceButtons(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	setSequenceButtons(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	return 1;
}

int LoLEngine::olol_setDefaultButtonState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setDefaultButtonState(%p)", (const void *)script);
	setDefaultButtonState();
	return 1;
}

int LoLEngine::olol_checkRectForMousePointer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_checkRectForMousePointer(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return posWithinRect(_mouseX, _mouseY, stackPos(0), stackPos(1), stackPos(2), stackPos(3)) ? 1 : 0;
}

int LoLEngine::olol_clearDialogueField(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_clearDialogueField(%p) (%d)", (const void *)script, stackPos(0));
	if (_currentControlMode && (!textEnabled()))
		return 1;

	_screen->setScreenDim(5);
	const ScreenDim *d = _screen->getScreenDim(5);
	_screen->fillRect(d->sx, d->sy, d->sx + d->w - (_flags.use16ColorMode ? 3 : 2), d->sy + d->h - 2, d->unkA);
	_txt->clearDim(4);
	_txt->resetDimTextPositions(4);

	return 1;
}

int LoLEngine::olol_setupBackgroundAnimationPart(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setupBackgroundAnimationPart(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));
	_tim->animator()->setupPart(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));
	return 0;
}

int LoLEngine::olol_startBackgroundAnimation(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_startBackgroundAnimation(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_tim->animator()->start(stackPos(0), stackPos(1));
	return 1;
}

int LoLEngine::olol_fadeToBlack(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_fadeToBlack(%p) (%d)", (const void *)script, stackPos(0));
	_screen->fadeToBlack(10);
	return 1;
}

int LoLEngine::olol_fadePalette(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_fadePalette(%p)", (const void *)script);
	if (_flags.use16ColorMode)
		setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
	else
		_screen->fadePalette(_screen->getPalette(3), 10);
	_screen->_fadeFlag = 0;
	return 1;
}

int LoLEngine::olol_loadBitmap(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadBitmap(%p) (%s, %d)", (const void *)script, stackPosString(0), stackPos(1));
	_screen->loadBitmap(stackPosString(0), 3, 3, &_screen->getPalette(3));
	if (stackPos(1) != 2)
		_screen->copyPage(3, stackPos(1));
	return 1;
}

int LoLEngine::olol_stopBackgroundAnimation(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_stopBackgroundAnimation(%p) (%d)", (const void *)script, stackPos(0));
	_tim->animator()->stop(stackPos(0));
	return 1;
}

int LoLEngine::olol_getGlobalScriptVar(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getGlobalScriptVar(%p) (%d)", (const void *)script, stackPos(0));
	assert(stackPos(0) < 24);
	return _globalScriptVars[stackPos(0)];
}

int LoLEngine::olol_setGlobalScriptVar(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setGlobalScriptVar(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) < 24);
	_globalScriptVars[stackPos(0)] = stackPos(1);
	return 1;
}

int LoLEngine::olol_getGlobalVar(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getGlobalVar(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));

	switch (stackPos(0)) {
	case 0:
		return _currentBlock;
	case 1:
		return _currentDirection;
	case 2:
		return _currentLevel;
	case 3:
		return _itemInHand;
	case 4:
		return _brightness;
	case 5:
		return _credits;
	case 6:
		return _globalScriptVars2[stackPos(1)];
	case 8:
		return _updateFlags;
	case 9:
		return _lampOilStatus;
	case 10:
		return _sceneDefaultUpdate;
	case 11:
		return _compassBroken;
	case 12:
		return _drainMagic;
	case 13:
		return getVolume(kVolumeSpeech) - 2;
	case 14:
		return _tim->_abortFlag;
	default:
		break;
	}

	return 0;
}

int LoLEngine::olol_setGlobalVar(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setGlobalVar(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	uint16 a = stackPos(1);
	uint16 b = stackPos(2);

	switch (stackPos(0)) {
	case 0:
		_currentBlock = b;
		calcCoordinates(_partyPosX, _partyPosY, _currentBlock, 0x80, 0x80);
		updateAutoMap(_currentBlock);
		break;

	case 1:
		_currentDirection = b;
		break;

	case 2:
		_currentLevel = b & 0xff;
		break;

	case 3:
		setHandItem(b);
		break;

	case 4:
		_brightness = b & 0xff;
		break;

	case 5:
		_credits = b;
		break;

	case 6:
		_globalScriptVars2[a] = b;
		break;

	case 7:
		break;

	case 8:
		_updateFlags = b;
		if (b == 1) {
			if (!textEnabled() || (!(_currentControlMode & 2)))
				timerUpdatePortraitAnimations(1);
			disableSysTimer(2);
		} else {
			enableSysTimer(2);
		}
		break;

	case 9:
		_lampOilStatus = b & 0xff;
		break;

	case 10:
		_sceneDefaultUpdate = b & 0xff;
		gui_toggleButtonDisplayMode(0, 0);
		break;

	case 11:
		_compassBroken = a & 0xff;
		break;

	case 12:
		_drainMagic = a & 0xff;
		break;

	default:
		break;
	}

	return 1;
}

int LoLEngine::olol_triggerDoorSwitch(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_triggerDoorSwitch(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	processDoorSwitch(stackPos(0)/*, (_wllWallFlags[_levelBlockProperties[stackPos(0)].walls[0]] & 8) ? 0 : 1*/, stackPos(1));
	return 1;
}

int LoLEngine::olol_checkEquippedItemScriptFlags(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_checkEquippedItemScriptFlags(%p)", (const void *)script);
	for (int i = 0; i < 4; i++) {
		if (!(_characters[i].flags & 1))
			continue;
		for (int ii = 0; ii < 4; ii++) {
			uint8 f = _itemProperties[_itemsInPlay[_characters[i].items[ii]].itemPropertyIndex].itemScriptFunc;
			if (f == 0 || f == 2)
				return 1;
		}
	}
	return 0;
}

int LoLEngine::olol_setDoorState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setDoorState(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (stackPos(1))
		_levelBlockProperties[stackPos(0)].flags = (_levelBlockProperties[stackPos(0)].flags & 0xef) | 0x20;
	else
		_levelBlockProperties[stackPos(0)].flags &= 0xdf;
	return 1;
}

int LoLEngine::olol_updateBlockAnimations(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_updateBlockAnimations(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int block = stackPos(0);
	int wall = stackPos(1);
	setWallType(block, wall, _levelBlockProperties[block].walls[(wall == -1) ? 0 : wall] == stackPos(2) ? stackPos(3) : stackPos(2));
	return 0;
}

int LoLEngine::olol_assignLevelDecorationShape(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_assignLevelDecorationShape(%p) (%d)", (const void *)script, stackPos(0));
	return assignLevelDecorationShapes(stackPos(0));
}

int LoLEngine::olol_resetBlockShapeAssignment(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_resetBlockShapeAssignment(%p) (%d)", (const void *)script, stackPos(0));
	uint8 v = stackPos(0) & 0xff;
	memset(_wllShapeMap + 3, v, 5);
	memset(_wllShapeMap + 13, v, 5);
	return 1;
}

int LoLEngine::olol_copyRegion(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_copyRegion(%p) (%d, %d, %d, %d, %d, %d, %d, %d)",
	       (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	_screen->copyRegion(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), Screen::CR_NO_P_CHECK);
	if (!stackPos(7))
		_screen->updateScreen();
	return 1;
}

int LoLEngine::olol_initMonster(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_initMonster(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script,
	       stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9), stackPos(10));
	uint16 x = 0;
	uint16 y = 0;
	calcCoordinates(x, y, stackPos(0), stackPos(1), stackPos(2));
	uint16 w = _monsterProperties[stackPos(4)].maxWidth;

	if (checkBlockBeforeObjectPlacement(x, y, w, 7, 7))
		return -1;

	for (uint8 i = 0; i < 30; i++) {
		LoLMonster *l = &_monsters[i];
		if (l->hitPoints || l->mode == 13)
			continue;

		memset(l, 0, sizeof(LoLMonster));
		l->id = i;
		l->x = x;
		l->y = y;
		l->facing = stackPos(3);
		l->type = stackPos(4);
		l->properties = &_monsterProperties[l->type];
		l->direction = l->facing << 1;
		l->hitPoints = (l->properties->hitPoints * _monsterModifiers[_monsterDifficulty]) >> 8;

		if (_currentLevel != 12 || l->type != 2)
			l->hitPoints = (l->hitPoints * (rollDice(1, 128) + 192)) >> 8;

		l->numDistAttacks = l->properties->numDistAttacks;
		l->distAttackTick = rollDice(1, calcMonsterSkillLevel(l->id | 0x8000, 8)) - 1;
		l->flyingHeight = 2;
		l->flags = stackPos(5);
		l->assignedItems = 0;

		setMonsterMode(l, stackPos(6));
		placeMonster(l, l->x, l->y);

		l->destX = l->x;
		l->destY = l->y;
		l->destDirection = l->direction;

		for (int ii = 0; ii < 4; ii++)
			l->equipmentShapes[ii] = stackPos(7 + ii) & 0xff;

		checkSceneUpdateNeed(l->block);
		return i;
	}

	return -1;
}

int LoLEngine::olol_fadeClearSceneWindow(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_fadeClearSceneWindow(%p)", (const void *)script);
	_screen->fadeClearSceneWindow(10);
	return 1;
}

int LoLEngine::olol_fadeSequencePalette(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_fadeSequencePalette(%p)", (const void *)script);
	if (_flags.use16ColorMode) {
		setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
	} else {
		_screen->getPalette(3).copy(_screen->getPalette(0), 128);
		_screen->loadSpecialColors(_screen->getPalette(3));
		_screen->fadePalette(_screen->getPalette(3), 10);
	}
	_screen->_fadeFlag = 0;
	return 1;
}

int LoLEngine::olol_redrawPlayfield(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_redrawPlayfield(%p)", (const void *)script);
	if (_screen->_fadeFlag != 2)
		_screen->fadeClearSceneWindow(10);
	gui_drawPlayField();
	setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
	_screen->_fadeFlag = 0;
	return 1;
}

int LoLEngine::olol_loadNewLevel(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadNewLevel(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	_screen->fadeClearSceneWindow(10);
	_screen->fillRect(112, 0, 288, 120, 0);
	disableSysTimer(2);

	for (int i = 0; i < 8; i++) {
		if (!_flyingObjects[i].enable || _flyingObjects[i].objectType)
			continue;
		endObjectFlight(&_flyingObjects[i], _flyingObjects[i].x, _flyingObjects[i].y, 1);
	}

	completeDoorOperations();

	generateTempData();

	_currentBlock = stackPos(1);
	_currentDirection = stackPos(2);
	calcCoordinates(_partyPosX, _partyPosY, _currentBlock, 0x80, 0x80);

	loadLevel(stackPos(0));

	enableSysTimer(2);

	script->ip = 0;
	return 1;
}

int LoLEngine::olol_getNearestMonsterFromCharacter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getNearestMonsterFromCharacter(%p) (%d)", (const void *)script, stackPos(0));
	return getNearestMonsterFromCharacter(stackPos(0));
}

int LoLEngine::olol_dummy0(EMCState *script) {
	return 0;
}

int LoLEngine::olol_loadMonsterProperties(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadMonsterProperties(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)",
	       (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5),
	       stackPos(6), stackPos(7), stackPos(8), stackPos(9), stackPos(10), stackPos(11), stackPos(12), stackPos(13),
	       stackPos(14), stackPos(15), stackPos(16), stackPos(17), stackPos(18), stackPos(19), stackPos(20),
	       stackPos(21), stackPos(22), stackPos(23), stackPos(24), stackPos(25), stackPos(26), stackPos(27),
	       stackPos(28), stackPos(29), stackPos(30), stackPos(31), stackPos(32), stackPos(33), stackPos(34),
	       stackPos(35), stackPos(36), stackPos(37), stackPos(38), stackPos(39), stackPos(40), stackPos(41));

	LoLMonsterProperty *l = &_monsterProperties[stackPos(0)];
	l->shapeIndex = stackPos(1) & 0xff;

	int shpWidthMax = 0;

	for (int i = 0; i < 16; i++) {
		uint8 m = _monsterShapes[(l->shapeIndex << 4) + i][3];
		if (m > shpWidthMax)
			shpWidthMax = m;
	}

	l->maxWidth = shpWidthMax;

	l->fightingStats[0] = (stackPos(2) << 8) / 100;     // hit chance
	l->fightingStats[1] = 256;                          //
	l->fightingStats[2] = (stackPos(3) << 8) / 100;     // protection
	l->fightingStats[3] = stackPos(4);                  // evade chance
	l->fightingStats[4] = (stackPos(5) << 8) / 100;     // speed
	l->fightingStats[5] = (stackPos(6) << 8) / 100;     //
	l->fightingStats[6] = (stackPos(7) << 8) / 100;     //
	l->fightingStats[7] = (stackPos(8) << 8) / 100;     //
	l->fightingStats[8] = 0;

	for (int i = 0; i < 8; i++) {
		l->itemsMight[i] = stackPos(9 + i);
		l->protectionAgainstItems[i] = (stackPos(17 + i) << 8) / 100;
	}

	l->itemProtection = stackPos(25);
	l->hitPoints = stackPos(26);
	l->speedTotalWaitTicks = 1;
	l->flags = stackPos(27);
	// This is what the original does here (setting the value first to stackPos(28) and then to stackPos(29):
	//l->unk5 = stackPos(28);
	l->unk5 = stackPos(29);

	l->numDistAttacks = stackPos(30);
	l->numDistWeapons = stackPos(31);
	for (int i = 0; i < 3; i++)
		l->distWeapons[i] = stackPos(32 + i);

	l->attackSkillChance = stackPos(35);
	l->attackSkillType = stackPos(36);
	l->defenseSkillChance = stackPos(37);
	l->defenseSkillType = stackPos(38);

	for (int i = 0; i < 3; i++)
		l->sounds[i] = stackPos(39 + i);

	return 1;
}

int LoLEngine::olol_battleHitSkillTest(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_battleHitSkillTest(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	return battleHitSkillTest(stackPos(0), stackPos(1), stackPos(2));
}

int LoLEngine::olol_inflictDamage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_inflictDamage(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	if (stackPos(0) == -1) {
		for (int i = 0; i < 4; i++)
			inflictDamage(i, stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	} else {
		inflictDamage(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	}

	return 1;
}

int LoLEngine::olol_moveMonster(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_moveMonster(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	LoLMonster *m = &_monsters[stackPos(0)];

	if (m->mode == 1 || m->mode == 2) {
		calcCoordinates(m->destX, m->destY, stackPos(1), stackPos(2), stackPos(3));
		m->destDirection = stackPos(4) << 1;
		if (m->x != m->destX || m->y != m->destY)
			setMonsterDirection(m, calcMonsterDirection(m->x, m->y, m->destX, m->destY));
	}

	return 1;
}

int LoLEngine::olol_setupDialogueButtons(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setupDialogueButtons(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	setupDialogueButtons(stackPos(0), getLangString(stackPos(1)), getLangString(stackPos(2)), getLangString(stackPos(3)));
	return 1;
}

int LoLEngine::olol_giveTakeMoney(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_giveTakeMoney(%p) (%d)", (const void *)script, stackPos(0));
	int c = stackPos(0);
	if (c >= 0)
		giveCredits(c, 1);
	else
		takeCredits(-c, 1);

	return 1;
}

int LoLEngine::olol_checkMoney(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_checkMoney(%p) (%d)", (const void *)script, stackPos(0));
	return (stackPos(0) > _credits) ? 0 : 1;
}

int LoLEngine::olol_setScriptTimer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setScriptTimer(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	uint8 id = 0x50 + stackPos(0);

	if (stackPos(1)) {
		_timer->enable(id);
		_timer->setCountdown(id, stackPos(1));

	} else {
		_timer->disable(id);
	}

	return 1;
}

int LoLEngine::olol_createHandItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_createHandItem(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	if (_itemInHand)
		return 0;

	setHandItem(makeItem(stackPos(0), stackPos(1), stackPos(2)));
	return 1;
}

int LoLEngine::olol_playAttackSound(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playAttackSound(%p) (%d)", (const void *)script, stackPos(0));

	static const uint8 sounds[] = { 12, 62, 63 };
	int d = stackPos(0);

	if ((d < 70 || d > 74) && (d < 81 || d > 89) && (d < 93 || d > 97) && (d < 102 || d > 106))
		snd_playSoundEffect(sounds[_itemProperties[d].skill & 3], -1);
	else
		snd_playSoundEffect(12, -1);

	return 1;
}

int LoLEngine::olol_addRemoveCharacter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_addRemoveCharacter(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));

	int16 id = stackPos(0);
	if (id < 0) {
		id = -id;
		for (int i = 0; i < 4; i++) {
			if (!(_characters[i].flags & 1) || _characters[i].id != id)
				continue;

			_characters[i].flags &= 0xfffe;
			calcCharPortraitXpos();

			if (_selectedCharacter == i)
				_selectedCharacter = 0;
			break;
		}
	} else {
		addCharacter(id);
	}

	if (!_updateFlags) {
		gui_enableDefaultPlayfieldButtons();
		gui_drawPlayField();
	}

	return 1;
}
int LoLEngine::olol_giveItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_giveItem(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	int item = makeItem(stackPos(0), stackPos(1), stackPos(2));
	if (addItemToInventory(item))
		return 1;

	deleteItem(item);
	return 0;
}

int LoLEngine::olol_loadTimScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadTimScript(%p) (%d, %s)", (const void *)script, stackPos(0), stackPosString(1));
	if (_activeTim[stackPos(0)])
		return 1;
	Common::String file = Common::String::format("%s.TIM", stackPosString(1));
	_activeTim[stackPos(0)] = _tim->load(file.c_str(), &_timIngameOpcodes);
	return 1;
}

int LoLEngine::olol_runTimScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_runTimScript(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return _tim->exec(_activeTim[stackPos(0)], stackPos(1));
}

int LoLEngine::olol_releaseTimScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_releaseTimScript(%p) (%d)", (const void *)script, stackPos(0));
	_tim->unload(_activeTim[stackPos(0)]);
	return 1;
}

int LoLEngine::olol_initSceneWindowDialogue(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_initSceneWindowDialogue(%p) (%d)", (const void *)script, stackPos(0));
	initSceneWindowDialogue(stackPos(0));
	return 1;
}

int LoLEngine::olol_restoreAfterSceneWindowDialogue(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_restoreAfterSceneWindowDialogue(%p) (%d)", (const void *)script, stackPos(0));
	restoreAfterSceneWindowDialogue(stackPos(0));
	return 1;
}

int LoLEngine::olol_getItemInHand(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getItemInHand(%p))", (const void *)script);
	return _itemInHand;
}

int LoLEngine::olol_checkMagic(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_checkMagic(%p )(%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	return checkMagic(stackPos(0), stackPos(1), stackPos(2));
}

int LoLEngine::olol_giveItemToMonster(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_giveItemToMonster(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (stackPos(0) == -1)
		return 0;
	giveItemToMonster(&_monsters[stackPos(0)], stackPos(1));
	return 1;
}

int LoLEngine::olol_loadLangFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadLangFile(%p) (%s)", (const void *)script, stackPosString(0));
	Common::String filename = Common::String::format("%s.%s", stackPosString(0), _languageExt[_lang]);
	delete[] _levelLangFile;
	_levelLangFile = _res->fileData(filename.c_str(), 0);
	return 1;
}

int LoLEngine::olol_playSoundEffect(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playSoundEffect(%p) (%d)", (const void *)script, stackPos(0));
	snd_playSoundEffect(stackPos(0), -1);
	return 1;
}

int LoLEngine::olol_processDialogue(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_processDialogue(%p)", (const void *)script);
	return processDialogue();
}

int LoLEngine::olol_stopTimScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_stopTimScript(%p) (%d)", (const void *)script, stackPos(0));
	_tim->stopAllFuncs(_activeTim[stackPos(0)]);
	return 1;
}

int LoLEngine::olol_getWallFlags(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getWallFlags(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return _wllWallFlags[_levelBlockProperties[stackPos(0)].walls[stackPos(1) & 3]];
}

int LoLEngine::olol_changeMonsterStat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_changeMonsterStat(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	if (stackPos(0) == -1)
		return 1;

	LoLMonster *m = &_monsters[stackPos(0) & 0x7fff];

	int16 d = stackPos(2);
	uint16 x = 0;
	uint16 y = 0;

	switch (stackPos(1)) {
	case 0:
		setMonsterMode(m, d);
		break;

	case 1:
		m->hitPoints = d;
		break;

	case 2:
		calcCoordinates(x, y, d, m->x & 0xff, m->y & 0xff);
		if (!walkMonsterCheckDest(x, y, m, 7))
			placeMonster(m, x, y);
		break;

	case 3:
		setMonsterDirection(m, d << 1);
		break;

	case 6:
		m->flags |= d;
		break;

	default:
		break;
	}

	return 1;
}

int LoLEngine::olol_getMonsterStat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getMonsterStat(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (stackPos(0) == -1)
		return 0;

	LoLMonster *m = &_monsters[stackPos(0) & 0x7fff];
	int d = stackPos(1);

	switch (d) {
	case 0:
		return m->mode;
	case 1:
		return m->hitPoints;
	case 2:
		return m->block;
	case 3:
		return m->facing;
	case 4:
		return m->type;
	case 5:
		return m->properties->hitPoints;
	case 6:
		return m->flags;
	case 7:
		return m->properties->flags;
	case 8:
		return _monsterAnimType[m->properties->shapeIndex];
	default:
		break;
	}

	return 0;
}

int LoLEngine::olol_releaseMonsterShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_releaseMonsterShapes(%p)", (const void *)script);
	for (int i = 0; i < 3; i++)
		releaseMonsterShapes(i);
	return 0;
}

int LoLEngine::olol_playCharacterScriptChat(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playCharacterScriptChat(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	if (_flags.isTalkie) {
		snd_stopSpeech(1);
		stopPortraitSpeechAnim();
	}
	return playCharacterScriptChat(stackPos(0), stackPos(1), 1, getLangString(stackPos(2)), script, 0, 3);
}

int LoLEngine::olol_playEnvironmentalSfx(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playEnvironmentalSfx(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	uint16 block = (stackPos(1) == -1) ? _currentBlock : stackPos(1);
	snd_processEnvironmentalSoundEffect(stackPos(0), block);
	return 1;
}

int LoLEngine::olol_update(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_update(%p)", (const void *)script);
	update();
	return 1;
}

int LoLEngine::olol_healCharacter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_healCharacter(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	if (stackPos(3)) {
		processMagicHeal(stackPos(0), stackPos(1));
	} else {
		increaseCharacterHitpoints(stackPos(0), stackPos(1), true);
		if (stackPos(2))
			gui_drawCharPortraitWithStats(stackPos(0));
	}
	return 1;
}

int LoLEngine::olol_drawExitButton(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_drawExitButton(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));

	static const uint8 printPara[] = { 0x90, 0x78, 0x0C, 0x9F, 0x80, 0x1E };

	int cp = _screen->setCurPage(0);
	Screen::FontId cf = _screen->setFont(Screen::FID_6_FNT);
	int x = printPara[3 * stackPos(0)] << 1;
	int y = printPara[3 * stackPos(0) + 1];
	int offs = printPara[3 * stackPos(0) + 2];

	char *str = getLangString(0x4033);
	int w = _screen->getTextWidth(str);

	if (_flags.use16ColorMode) {
		gui_drawBox(x - offs - w, y - 9, w + offs, 9, 0xee, 0xcc, 0x11);
		_screen->printText(str, x - (offs >> 1) - w, y - 7, 0xbb, 0);
	} else {
		gui_drawBox(x - offs - w, y - 9, w + offs, 9, 136, 251, 252);
		_screen->printText(str, x - (offs >> 1) - w, y - 7, 144, 0);
	}

	if (stackPos(1))
		_screen->drawGridBox(x - offs - w + 1, y - 8, w + offs - 2, 7, 1);

	_screen->setFont(cf);
	_screen->setCurPage(cp);
	return 1;
}

int LoLEngine::olol_loadSoundFile(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_loadSoundFile(%p) (%d)", (const void *)script, stackPos(0));
	snd_loadSoundFile(stackPos(0));
	return 1;
}

int LoLEngine::olol_playMusicTrack(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playMusicTrack(%p) (%d)", (const void *)script, stackPos(0));
	return snd_playTrack(stackPos(0));
}

int LoLEngine::olol_deleteMonstersFromBlock(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_deleteMonstersFromBlock(%p) (%d)", (const void *)script, stackPos(0));
	deleteMonstersFromBlock(stackPos(0));
	return 1;
}

int LoLEngine::olol_countBlockItems(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_countBlockItems(%p) (%d)", (const void *)script, stackPos(0));
	uint16 o = _levelBlockProperties[stackPos(0)].assignedObjects;
	int res = 0;

	while (o) {
		if (!(o & 0x8000))
			res++;
		o = findObject(o)->nextAssignedObject;
	}

	return res;
}

int LoLEngine::olol_characterSkillTest(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_characterSkillTest(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	int skill = stackPos(0);
	int n = countActiveCharacters();
	int m = 0;
	int c = 0;

	for (int i = 0; i < n; i++) {
		int v = _characters[i].skillModifiers[skill] + _characters[i].skillLevels[skill] + 25;
		if (v > m) {
			m = v;
			c = i;
		}
	}

	return (rollDice(1, 100) > m) ? -1 : c;
}

int LoLEngine::olol_countAllMonsters(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_countAllMonsters(%p)", (const void *)script);
	int res = 0;

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].hitPoints > 0 && _monsters[i].mode != 13)
			res++;
	}

	return res;
}

int LoLEngine::olol_playEndSequence(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playEndSequence(%p)", (const void *)script);

	int c = 0;
	if (_characters[0].id == -9)
		c = 1;
	else if (_characters[0].id == -5)
		c = 3;
	else if (_characters[0].id == -1)
		c = 2;

	while (snd_updateCharacterSpeech())
		delay(_tickLength);

	_eventList.clear();
	_screen->hideMouse();
	_screen->getPalette(1).clear();

	showOutro(c, (_monsterDifficulty == 2));
	// Don't call quitGame() on a RTL request (because this would
	// make the next game launched from the launcher quit instantly.
	if (!shouldQuit())
		quitGame();

	return 0;
}

int LoLEngine::olol_stopPortraitSpeechAnim(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_stopPortraitSpeechAnim(%p)", (const void *)script);
	if (_flags.isTalkie)
		snd_stopSpeech(1);
	stopPortraitSpeechAnim();
	return 1;
}

int LoLEngine::olol_setPaletteBrightness(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setPaletteBrightness(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	uint16 old = _brightness;
	_brightness = stackPos(0);
	if (stackPos(1) == 1)
		setPaletteBrightness(_screen->getPalette(0), stackPos(0), _lampEffect);
	return old;
}

int LoLEngine::olol_calcInflictableDamage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_calcInflictableDamage(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	return calcInflictableDamage(stackPos(0), stackPos(1), stackPos(2));
}

int LoLEngine::olol_getInflictedDamage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getInflictedDamage(%p) (%d)", (const void *)script, stackPos(0));
	int mx = stackPos(0);
	return rollDice(2, mx);
}

int LoLEngine::olol_checkForCertainPartyMember(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_checkForCertainPartyMember(%p) (%d)", (const void *)script, stackPos(0));
	for (int i = 0; i < 4; i++) {
		if (_characters[i].flags & 9 && _characters[i].id == stackPos(0))
			return 1;
	}
	return 0;
}

int LoLEngine::olol_printMessage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_printMessage(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));
	int snd = stackPos(2);
	_txt->printMessage(stackPos(0), getLangString(stackPos(1)), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8), stackPos(9));

	if (snd >= 0)
		snd_playSoundEffect(snd, -1);

	return 1;
}

int LoLEngine::olol_deleteLevelItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_deleteLevelItem(%p) (%d)", (const void *)script, stackPos(0));
	if (_itemsInPlay[stackPos(0)].block)
		removeLevelItem(stackPos(0), _itemsInPlay[stackPos(0)].block);

	deleteItem(stackPos(0));

	return 1;
}

int LoLEngine::olol_calcInflictableDamagePerItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_calcInflictableDamagePerItem(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	return calcInflictableDamagePerItem(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
}

int LoLEngine::olol_distanceAttack(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_distanceAttack(%p) (%d, %d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7), stackPos(8));

	uint16 fX = stackPos(3);
	uint16 fY = stackPos(4);

	if (!(stackPos(8) & 0x8000))
		fX = fY = 0x80;

	uint16 x = 0;
	uint16 y = 0;
	calcCoordinates(x, y, stackPos(2), fX, fY);

	if (launchObject(stackPos(0), stackPos(1), x, y, stackPos(5), stackPos(6) << 1, stackPos(7), stackPos(8), 0x3f))
		return 1;

	deleteItem(stackPos(1));
	return 0;
}

int LoLEngine::olol_removeCharacterEffects(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_removeCharacterEffects(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	removeCharacterEffects(&_characters[stackPos(0)], stackPos(1), stackPos(2));
	return 1;
}

int LoLEngine::olol_checkInventoryFull(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_checkInventoryFull(%p)", (const void *)script);
	for (int i = 0; i < 48; i++) {
		if (_inventory[i])
			return 0;
	}
	return 1;
}

int LoLEngine::olol_moveBlockObjects(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_moveBlockObjects(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	int o = _levelBlockProperties[stackPos(0)].assignedObjects;
	int res = 0;
	int level = stackPos(2);
	int destBlock = stackPos(1);
	int runScript = stackPos(4);
	int includeMonsters = stackPos(3);
	int includeItems = stackPos(5);

	// WORKAROUND for script bug
	// Items would vanish when thrown towards the stairs
	// in white tower level 3.
	if (_currentLevel == 21 && level == 21 && destBlock == 0x3e0) {
		level = 20;
		destBlock = 0x0247;
	}

	while (o) {
		int l = o;
		o = findObject(o)->nextAssignedObject;
		if (l & 0x8000) {
			if (!includeMonsters)
				continue;

			l &= 0x7fff;

			LoLMonster *m = &_monsters[l];

			setMonsterMode(m, 14);
			checkSceneUpdateNeed(m->block);
			placeMonster(m, 0, 0);

			res = 1;

		} else {
			if (!(_itemsInPlay[l].shpCurFrame_flg & 0x4000) || !includeItems)
				continue;

			placeMoveLevelItem(l, level, destBlock, _itemsInPlay[l].x & 0xff, _itemsInPlay[l].y & 0xff, _itemsInPlay[l].flyingHeight);
			res = 1;

			if (!runScript || level != _currentLevel)
				continue;

			runLevelScriptCustom(destBlock, 0x80, -1, l, 0, 0);
		}
	}

	return res;
}

int LoLEngine::olol_addSpellToScroll(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_addSpellToScroll(%p) (%d)", (const void *)script, stackPos(0));
	addSpellToScroll(stackPos(0), stackPos(1));
	return 1;
}

int LoLEngine::olol_playDialogueText(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playDialogueText(%p) (%d)", (const void *)script, stackPos(0));
	_txt->printDialogueText(3, getLangString(stackPos(0)), script, 0, 1);
	return 1;
}

int LoLEngine::olol_playDialogueTalkText(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_playDialogueTalkText(%p) (%d)", (const void *)script, stackPos(0));
	int track = stackPos(0);

	if (!snd_playCharacterSpeech(track, 0, 0) || textEnabled()) {
		char *s = getLangString(track);
		_txt->printDialogueText(4, s, script, 0, 1);
	}

	return 1;
}

int LoLEngine::olol_checkMonsterTypeHostility(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_checkMonsterTypeHostility(%p) (%d)", (const void *)script, stackPos(0));
	for (int i = 0; i < 30; i++) {
		if (stackPos(0) != _monsters[i].type && stackPos(0) != -1)
			continue;
		return (_monsters[i].mode == 1) ? 0 : 1;
	}
	return 1;
}

int LoLEngine::olol_setNextFunc(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setNextFunc(%p) (%d)", (const void *)script, stackPos(0));
	_nextScriptFunc = stackPos(0);
	return 1;
}

int LoLEngine::olol_dummy1(EMCState *script) {
	return 1;
}

int LoLEngine::olol_suspendMonster(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_suspendMonster(%p) (%d)", (const void *)script, stackPos(0));
	LoLMonster *m = &_monsters[stackPos(0) & 0x7fff];
	setMonsterMode(m, 14);
	checkSceneUpdateNeed(m->block);
	placeMonster(m, 0, 0);
	return 1;
}

int LoLEngine::olol_setScriptTextParameter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setScriptTextParameter(%p) (%d)", (const void *)script, stackPos(0));
	_txt->_scriptTextParameter = stackPos(0);
	return 1;
}

int LoLEngine::olol_triggerEventOnMouseButtonClick(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_triggerEventOnMouseButtonClick(%p) (%d)", (const void *)script, stackPos(0));
	gui_notifyButtonListChanged();
	snd_updateCharacterSpeech();

	int f = checkInput(0);
	removeInputTop();
	if (f == 0 || (f & 0x800))
		return 0;

	int evt = stackPos(0);
	if (evt) {
		gui_triggerEvent(evt);
		_seqTrigger = 1;
	} else {
		removeInputTop();
	}

	return 1;
}

int LoLEngine::olol_printWindowText(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_printWindowText(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	int dim = stackPos(0);
	int flg = stackPos(1);
	_screen->setScreenDim(dim);
	if (flg & 1)
		_txt->clearCurDim();
	if (flg & 3)
		_txt->resetDimTextPositions(dim);
	_txt->printDialogueText(dim, getLangString(stackPos(2)), script, 0, 3);
	return 1;
}

int LoLEngine::olol_countSpecificMonsters(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_countSpecificMonsters(%p) (%d, ...)", (const void *)script, stackPos(0));
	uint16 types = 0;
	int res = 0;
	int cnt = 0;

	while (stackPos(cnt) != -1)
		types |= (1 << stackPos(cnt++));

	for (int i = 0; i < 30; i++) {
		if (((1 << _monsters[i].type) & types) && _monsters[i].mode < 14)
			res++;
	}

	return res;
}

int LoLEngine::olol_updateBlockAnimations2(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_updateBlockAnimations2(%p) (%d, %d, %d, %d, ...)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int numFrames = stackPos(3);
	assert(numFrames <= 97);
	int curFrame = stackPos(2) % numFrames;
	setWallType(stackPos(0), stackPos(1), stackPos(4 + curFrame));
	return 0;
}

int LoLEngine::olol_checkPartyForItemType(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_checkPartyForItemType(%p) (%d, %d, %d))", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	int p = stackPos(1);

	if (!stackPos(2)) {
		for (int i = 0; i < 48; i++) {
			if (!_inventory[i] || _itemsInPlay[_inventory[i]].itemPropertyIndex != p)
				continue;
			return 1;
		}

		if (_itemsInPlay[_itemInHand].itemPropertyIndex == p)
			return 1;
	}

	int last = (stackPos(0) == -1) ? 3 : stackPos(0);
	int first = (stackPos(0) == -1) ? 0 : stackPos(0);

	for (int i = first; i <= last; i++) {
		if (itemEquipped(i, p))
			return 1;
	}

	return 0;
}

int LoLEngine::olol_blockDoor(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_blockDoor(%p) (%d)", (const void *)script, stackPos(0));
	_blockDoor = stackPos(0);
	return _blockDoor;
}

int LoLEngine::olol_resetTimDialogueState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_resetTimDialogueState(%p) (%d)", (const void *)script, stackPos(0));
	_tim->resetDialogueState(_activeTim[stackPos(0)]);
	return 1;
}

int LoLEngine::olol_getItemOnPos(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getItemOnPos(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	int pX = stackPos(1);
	if (pX != -1)
		pX &= 0xff;

	int pY = stackPos(2);
	if (pY != -1)
		pY &= 0xff;

	int o = (stackPos(3) || _emcLastItem == -1) ? stackPos(0) : _emcLastItem;

	_emcLastItem = _levelBlockProperties[o].assignedObjects;

	while (_emcLastItem) {
		if (_emcLastItem & 0x8000) {
			o = _emcLastItem & 0x7fff;
			_emcLastItem = _levelBlockProperties[o].assignedObjects;
			continue;
		}

		if (pX != -1 && (_itemsInPlay[_emcLastItem].x & 0xff) != pX) {
			o = _emcLastItem & 0x7fff;
			_emcLastItem = _levelBlockProperties[o].assignedObjects;
			continue;
		}

		if (pY != -1 && (_itemsInPlay[_emcLastItem].y & 0xff) != pY) {
			o = _emcLastItem & 0x7fff;
			_emcLastItem = _levelBlockProperties[o].assignedObjects;
			continue;
		}

		return _emcLastItem;
	}

	return 0;
}

int LoLEngine::olol_removeLevelItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_removeLevelItem(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	removeLevelItem(stackPos(0), stackPos(1));
	return 1;
}

int LoLEngine::olol_savePage5(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_savePage5(%p)", (const void *)script);
	// Not implemented: The original code uses this to back up and restore page 5 which is used
	// to load WSA files. Since our WSA player provides its own memory buffers we don't
	// need to use page 5.
	return 1;
}

int LoLEngine::olol_restorePage5(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_restorePage5(%p)", (const void *)script);
	// Not implemented: The original code uses this to back up and restore page 5 which is used
	// to load WSA files. Since our WSA player provides its own memory buffers we don't
	// need to use page 5.
	for (int i = 0; i < 6; i++)
		_tim->freeAnimStruct(i);
	return 1;
}

int LoLEngine::olol_initDialogueSequence(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_initDialogueSequence(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	initDialogueSequence(stackPos(0), stackPos(1));
	return 1;
}

int LoLEngine::olol_restoreAfterDialogueSequence(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_restoreAfterDialogueSequence(%p) (%d)", (const void *)script, stackPos(0));
	restoreAfterDialogueSequence(stackPos(0));
	return 1;
}

int LoLEngine::olol_setSpecialSceneButtons(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setSpecialSceneButtons(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	setSpecialSceneButtons(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	return 1;
}

int LoLEngine::olol_restoreButtonsAfterSpecialScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_restoreButtonsAfterSpecialScene(%p)", (const void *)script);
	gui_specialSceneRestoreButtons();
	return 1;
}

int LoLEngine::olol_prepareSpecialScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_prepareSpecialScene(%p) (%d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	prepareSpecialScene(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	return 1;
}

int LoLEngine::olol_restoreAfterSpecialScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_restoreAfterSpecialScene(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	return restoreAfterSpecialScene(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
}

int LoLEngine::olol_assignCustomSfx(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_assignCustomSfx(%p) (%s, %d)", (const void *)script, stackPosString(0), stackPos(1));
	const char *c = stackPosString(0);
	int i = stackPos(1);

	if (!c || i > 250)
		return 0;

	uint16 t = READ_LE_UINT16(&_ingameSoundIndex[i << 1]);
	if (t == 0xffff)
		return 0;

	strcpy(_ingameSoundList[t], c);

	return 0;
}

int LoLEngine::olol_findAssignedMonster(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_findAssignedMonster(%p)  (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	uint16 o = stackPos(1) == -1 ? _levelBlockProperties[stackPos(0)].assignedObjects : findObject(stackPos(1))->nextAssignedObject;
	while (o) {
		if (o & 0x8000)
			return o & 0x7fff;
		o = findObject(o)->nextAssignedObject;
	}
	return -1;
}

int LoLEngine::olol_checkBlockForMonster(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_checkBlockForMonster(%p)  (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	uint16 block = stackPos(0);
	uint16 id = stackPos(1) | 0x8000;

	uint16 o = _levelBlockProperties[block].assignedObjects;
	while (o & 0x8000) {
		if (id == 0xffff || id == o)
			return o & 0x7fff;
		o = findObject(o)->nextAssignedObject;
	}
	return -1;
}

int LoLEngine::olol_crossFadeRegion(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_crossFadeRegion(%p) (%d, %d, %d, %d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	_screen->crossFadeRegion(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5), stackPos(6), stackPos(7));
	return 1;
}

int LoLEngine::olol_calcCoordinatesAddDirectionOffset(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_calcCoordinatesAddDirectionOffset(%p) (%d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	uint16 x = stackPos(0);
	uint16 y = stackPos(1);
	calcCoordinatesAddDirectionOffset(x, y, stackPos(2));
	return stackPos(3) ? x : y;
}

int LoLEngine::olol_resetPortraitsAndDisableSysTimer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_resetPortraitsAndDisableSysTimer(%p)", (const void *)script);
	resetPortraitsAndDisableSysTimer();
	return 1;
}

int LoLEngine::olol_enableSysTimer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_enableSysTimer(%p)", (const void *)script);
	_needSceneRestore = 0;
	enableSysTimer(2);
	return 1;
}

int LoLEngine::olol_checkNeedSceneRestore(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_checkNeedSceneRestore(%p)", (const void *)script);
	return _needSceneRestore;
}

int LoLEngine::olol_getNextActiveCharacter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getNextActiveCharacter(%p)  (%d)", (const void *)script, stackPos(0));
	if (stackPos(0))
		_scriptCharacterCycle = 0;
	else
		_scriptCharacterCycle++;

	while (_scriptCharacterCycle < 4) {
		if (_characters[_scriptCharacterCycle].flags & 1)
			return _scriptCharacterCycle;
		_scriptCharacterCycle++;
	}
	return -1;
}

int LoLEngine::olol_paralyzePoisonCharacter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_paralyzePoisonCharacter(%p)  (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	return paralyzePoisonCharacter(stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
}

int LoLEngine::olol_drawCharPortrait(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_drawCharPortrait(%p) (%d)", (const void *)script, stackPos(0));
	int charNum = stackPos(0);
	if (charNum == -1)
		gui_drawAllCharPortraitsWithStats();
	else
		gui_drawCharPortraitWithStats(charNum);
	return 1;
}

int LoLEngine::olol_removeInventoryItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_removeInventoryItem(%p) (%d)", (const void *)script, stackPos(0));
	int itemType = stackPos(0);
	for (int i = 0; i < 48; i++) {
		if (!_inventory[i] || _itemsInPlay[_inventory[i]].itemPropertyIndex != itemType)
			continue;
		_inventory[i] = 0;
		gui_drawInventory();
		return 1;
	}

	for (int i = 0; i < 4; i++) {
		if (!(_characters[i].flags & 1))
			continue;

		for (int ii = 0; ii < 11; ii++) {
			if (!_characters[i].items[ii] || _itemsInPlay[_characters[i].items[ii]].itemPropertyIndex != itemType)
				continue;
			_characters[i].items[ii] = 0;
			return 1;
		}
	}
	return 0;
}

int LoLEngine::olol_getAnimationLastPart(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getAnimationLastPart(%p) (%d)", (const void *)script, stackPos(0));
	return _tim->animator()->resetLastPart(stackPos(0));
}

int LoLEngine::olol_assignSpecialGuiShape(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_assignSpecialGuiShape(%p) (%d, %d, %d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	if (stackPos(0)) {
		_specialGuiShape = _levelDecorationShapes[_levelDecorationProperties[_wllShapeMap[stackPos(0)]].shapeIndex[stackPos(1)]];
		_specialGuiShapeX = stackPos(2);
		_specialGuiShapeY = stackPos(3);
		_specialGuiShapeMirrorFlag = stackPos(4);

	} else {
		_specialGuiShape = 0;
		_specialGuiShapeX = _specialGuiShapeY = _specialGuiShapeMirrorFlag = 0;
	}
	return 1;
}

int LoLEngine::olol_findInventoryItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_findInventoryItem(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	if (stackPos(0) == 0) {
		for (int i = 0; i < 48; i++) {
			if (!_inventory[i])
				continue;
			if (_itemsInPlay[_inventory[i]].itemPropertyIndex == stackPos(2))
				return 0;
		}
	}
	int cur = stackPos(1);
	int last = cur;
	if (stackPos(1) == -1) {
		cur = 0;
		last = 4;
	}
	for (; cur < last; cur++) {
		if (!(_characters[cur].flags & 1))
			continue;
		for (int i = 0; i < 11; i++) {
			if (!_characters[cur].items[i])
				continue;
			if (_itemsInPlay[_characters[cur].items[i]].itemPropertyIndex == stackPos(2))
				return cur;
		}
	}
	return -1;
}

int LoLEngine::olol_restoreFadePalette(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_restoreFadePalette(%p)", (const void *)script);
	_screen->getPalette(0).copy(_screen->getPalette(1), 0, _flags.use16ColorMode ? 16 : 128);
	_screen->fadePalette(_screen->getPalette(0), 10);
	_screen->_fadeFlag = 0;
	return 1;
}

int LoLEngine::olol_getSelectedCharacter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getSelectedCharacter(%p)", (const void *)script);
	return _selectedCharacter;
}

int LoLEngine::olol_setHandItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setHandItem(%p) (%d)", (const void *)script, stackPos(0));
	setHandItem(stackPos(0));
	return 1;
}

int LoLEngine::olol_drinkBezelCup(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_drinkBezelCup(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	drinkBezelCup(3 - stackPos(0), stackPos(1));
	return 1;
}

int LoLEngine::olol_changeItemTypeOrFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_changeItemTypeOrFlag(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	if (stackPos(0) < 1)
		return 0;

	LoLItem *i = &_itemsInPlay[stackPos(0)];
	int16 val = stackPos(2);

	if (stackPos(1) == 4)
		i->itemPropertyIndex = val;
	else if (stackPos(1) == 15)
		i->shpCurFrame_flg = (i->shpCurFrame_flg & 0xe000) | (val & 0x1fff);
	else
		val = -1;

	return val;
}

int LoLEngine::olol_placeInventoryItemInHand(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_placeInventoryItemInHand(%p)  (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	int itemType = stackPos(0);
	int i = 0;
	for (; i < 48; i++) {
		if (!_inventory[i])
			continue;
		if (_itemsInPlay[_inventory[i]].itemPropertyIndex == itemType)
			break;
	}

	if (i == 48)
		return -1;

	_inventoryCurItem = i;
	int r = _itemInHand;
	setHandItem(_inventory[i]);
	_inventory[i] = r;

	if (stackPos(1))
		gui_drawInventory();

	return r;
}

int LoLEngine::olol_castSpell(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_castSpell(%p)  (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	return castSpell(stackPos(0), stackPos(1), stackPos(2));
}

int LoLEngine::olol_pitDrop(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_pitDrop(%p)  (%d)", (const void *)script, stackPos(0));
	int m = stackPos(0);
	_screen->updateScreen();
	if (m) {
		gui_drawScene(2);
		pitDropScroll(9);
		snd_playSoundEffect(-1, -1);
		shakeScene(30, 4, 0, 1);

	} else {
		int t = -1;
		for (int i = 0; i < 4; i++) {
			if (!(_characters[i].flags & 1) || (_characters[i].id >= 0))
				continue;
			if (_characters[i].id == -1)
				t = 54;
			else if (_characters[i].id == -5)
				t = 53;
			else if (_characters[i].id == -8)
				t = 52;
			else if (_characters[i].id == -9)
				t = 51;
		}

		_screen->fillRect(112, 0, 288, 120, 0, 2);
		snd_playSoundEffect(t, -1);
		pitDropScroll(12);
	}

	return 1;
}

int LoLEngine::olol_increaseSkill(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_increaseSkill(%p)  (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	LoLCharacter *c = &_characters[stackPos(0)];
	int s = stackPos(1);
	int l = c->skillLevels[s];
	increaseExperience(stackPos(0), s, _expRequirements[l] - c->experiencePts[s]);
	return c->skillLevels[s] - l;
}

int LoLEngine::olol_paletteFlash(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_paletteFlash(%p) (%d)", (const void *)script, stackPos(0));
	Palette &p1 = _screen->getPalette(1);

	if (_flags.use16ColorMode) {
		Palette p2(16);
		p2.copy(p1);
		uint8 *d = p2.getData();

		for (int i = 0; i < 16; i++)
			d[i * 3] = 0x3f;

		_screen->setScreenPalette(p2);
		_screen->updateScreen();

		delay(4 * _tickLength);

		_screen->setScreenPalette(p1);
		if (_smoothScrollModeNormal)
			_screen->copyRegion(112, 0, 112, 0, 176, 120, 2, 0);

		_screen->updateScreen();

	} else {
		Palette &p2 = _screen->getPalette(3);

		uint8 ovl[256];
		generateFlashPalette(p1, p2, stackPos(0));
		_screen->loadSpecialColors(p1);
		_screen->loadSpecialColors(p2);

		if (_smoothScrollModeNormal) {
			for (int i = 0; i < 256; i++)
				ovl[i] = i;
			ovl[1] = 6;

			_screen->copyRegion(112, 0, 112, 0, 176, 120, 0, 2);
			_screen->applyOverlay(112, 0, 176, 120, 0, ovl);
		}

		_screen->setScreenPalette(p2);
		_screen->updateScreen();

		delay(2 * _tickLength);

		_screen->setScreenPalette(p1);
		if (_smoothScrollModeNormal)
			_screen->copyRegion(112, 0, 112, 0, 176, 120, 2, 0);

		_screen->updateScreen();
	}

	return 0;
}

int LoLEngine::olol_restoreMagicShroud(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_restoreMagicShroud(%p)", (const void *)script);

	WSAMovie_v2 *mov = new WSAMovie_v2(this);
	mov->open("DARKLITE.WSA", 2, 0);
	if (!mov->opened()) {
		delete mov;
		warning("LoLEngine::olol_restoreMagicShroud: Could not open file: \"DARKLITE.WSA\"");
		return 1;
	}

	_screen->hideMouse();

	Palette *fadeTab[28];
	for (int i = 0; i < 28; i++)
		fadeTab[i] = new Palette(_flags.use16ColorMode ? 16 : 256);

	Palette **tpal1 = &fadeTab[0];
	Palette **tpal2 = &fadeTab[1];
	Palette **tpal3 = &fadeTab[2];
	Palette **tpal4 = 0;

	int len = _flags.use16ColorMode ? 48 : 768;
	_res->loadFileToBuf("LITEPAL1.COL", (*tpal1)->getData(), len);
	tpal2 = _screen->generateFadeTable(tpal3, 0, *tpal1, 21);

	_res->loadFileToBuf("LITEPAL2.COL", (*tpal2)->getData(), len);
	tpal4 = tpal2++;

	_res->loadFileToBuf("LITEPAL3.COL", (*tpal1)->getData(), len);
	_screen->generateFadeTable(tpal2, *tpal4, *tpal1, 4);

	for (int i = 0; i < 21; i++) {
		uint32 etime = _system->getMillis() + 20 * _tickLength;
		mov->displayFrame(i, 0, 0, 0, 0, 0, 0);
		_screen->setScreenPalette(**tpal3++);
		_screen->updateScreen();

		if (i == 2 || i == 5 || i == 8 || i == 11 || i == 13 || i == 15 || i == 17 || i == 19)
			snd_playSoundEffect(95, -1);

		delayUntil(etime);
	}

	snd_playSoundEffect(91, -1);
	_screen->fadePalette(**tpal3++, 300);

	for (int i = 22; i < 38; i++) {
		uint32 etime = _system->getMillis() + 12 * _tickLength;
		mov->displayFrame(i, 0, 0, 0, 0, 0, 0);
		if (i == 22 || i == 24 || i == 28 || i == 32) {
			snd_playSoundEffect(131, -1);
			_screen->setScreenPalette(**tpal3++);
		}
		_screen->updateScreen();
		delayUntil(etime);
	}

	mov->close();
	delete mov;

	for (int i = 0; i < 28; i++)
		delete fadeTab[i];

	_screen->showMouse();

	return 1;
}

int LoLEngine::olol_disableControls(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_disableControls(%p) (%d)", (const void *)script, stackPos(0));
	return gui_disableControls(stackPos(0));
}

int LoLEngine::olol_enableControls(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_enableControls(%p)", (const void *)script);
	return gui_enableControls();
}

int LoLEngine::olol_shakeScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_shakeScene(%p)  (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	shakeScene(stackPos(0), stackPos(1), stackPos(2), 1);
	return 1;
}

int LoLEngine::olol_gasExplosion(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_gasExplosion(%p) (%d)", (const void *)script, stackPos(0));
	processGasExplosion(stackPos(0));
	return 1;
}

int LoLEngine::olol_calcNewBlockPosition(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_calcNewBlockPosition(%p)  (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	return calcNewBlockPosition(stackPos(0), stackPos(1));
}

int LoLEngine::olol_crossFadeScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_crossFadeScene(%p)", (const void *)script);
	gui_drawScene(2);
	_screen->crossFadeRegion(112, 0, 112, 0, 176, 120, 2, 0);
	updateDrawPage2();
	return 1;
}

int LoLEngine::olol_updateDrawPage2(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_updateDrawPage2(%p)", (const void *)script);
	updateDrawPage2();
	return 1;
}

int LoLEngine::olol_setMouseCursor(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_setMouseCursor(%p) (%d)", (const void *)script, stackPos(0));
	if (stackPos(0) == 1)
		setMouseCursorToIcon(133);
	else
		setMouseCursorToItemInHand();
	return 1;
}

int LoLEngine::olol_characterSays(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_characterSays(%p)  (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));

	if (!_flags.isTalkie)
		return 0;

	if (stackPos(0) == -1) {
		snd_stopSpeech(true);
		return 1;
	}

	if (stackPos(0) != -2)
		return characterSays(stackPos(0), stackPos(1), stackPos(2));
	else
		return snd_updateCharacterSpeech();
}

int LoLEngine::olol_queueSpeech(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_queueSpeech(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	if (stackPos(0) && stackPos(1)) {
		_nextSpeechId = stackPos(0) + 1000;
		_nextSpeaker = stackPos(1);
	}
	return 1;
}

int LoLEngine::olol_getItemPrice(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getItemPrice(%p) (%d)", (const void *)script, stackPos(0));
	int c = stackPos(0);
	if (c < 0) {
		c = -c;
		if (c < 50)
			return 50;
		c = (c + 99) / 100;
		return c * 100;

	} else {
		for (int i = 0; i < 46; i++) {
			if (_itemCost[i] >= c)
				return _itemCost[i];
		}
	}

	return 0;
}

int LoLEngine::olol_getLanguage(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::olol_getLanguage(%p)", (const void *)script);
	return _lang;
}

#pragma mark -

int LoLEngine::tlol_setupPaletteFade(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_setupPaletteFade(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	_screen->getFadeParams(_screen->getPalette(0), param[0], _tim->_palDelayInc, _tim->_palDiff);
	_tim->_palDelayAcc = 0;
	return 1;
}

int LoLEngine::tlol_loadPalette(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_loadPalette(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	const char *palFile = (const char *)(tim->text + READ_LE_UINT16(tim->text + (param[0] << 1)));
	_screen->loadPalette(palFile, _screen->getPalette(0));
	return 1;
}

int LoLEngine::tlol_setupPaletteFadeEx(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_setupPaletteFadeEx(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	_screen->copyPalette(0, 1);

	_screen->getFadeParams(_screen->getPalette(0), param[0], _tim->_palDelayInc, _tim->_palDiff);
	_tim->_palDelayAcc = 0;
	return 1;
}

int LoLEngine::tlol_processWsaFrame(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_processWsaFrame(%p, %p) (%d, %d, %d, %d, %d)",
	       (const void *)tim, (const void *)param, param[0], param[1], param[2], param[3], param[4]);

	const int animIndex = tim->wsa[param[0]].anim - 1;
	const int frame = param[1];
	const int x2 = param[2];
	const int y2 = param[3];
	const int factor = MAX<int>(0, (int16)param[4]);

	const int x1 = _tim->animator()->getAnimX(animIndex);
	const int y1 = _tim->animator()->getAnimY(animIndex);
	const Movie *wsa = _tim->animator()->getWsaCPtr(animIndex);

	int w1 = wsa->width();
	int h1 = wsa->height();
	int w2 = (w1 * factor) / 100;
	int h2 = (h1 * factor) / 100;

	_tim->animator()->displayFrame(animIndex, 2, frame);
	_screen->wsaFrameAnimationStep(x1, y1, x2, y2, w1, h1, w2, h2, 2, _flags.isDemo && _flags.platform != Common::kPlatformPC98 ? 0 : 8, 0);
	if (!_flags.isDemo && _flags.platform != Common::kPlatformPC98)
		_screen->checkedPageUpdate(8, 4);
	_screen->updateScreen();

	return 1;
}

int LoLEngine::tlol_displayText(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_displayText(%p, %p) (%d, %d)", (const void *)tim, (const void *)param, param[0], (int16)param[1]);
	if (tim->isLoLOutro)
		_tim->displayText(param[0], param[1], param[2]);
	else
		_tim->displayText(param[0], param[1]);
	return 1;
}

int LoLEngine::tlol_initSceneWindowDialogue(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_initSceneWindowDialogue(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	initSceneWindowDialogue(param[0]);
	return 1;
}

int LoLEngine::tlol_restoreAfterSceneWindowDialogue(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_restoreAfterSceneWindowDialogue(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	restoreAfterSceneWindowDialogue(param[0]);
	return 1;
}

int LoLEngine::tlol_giveItem(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_giveItem(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	int item = makeItem(param[0], param[1], param[2]);
	if (addItemToInventory(item))
		return 1;

	deleteItem(item);
	return 0;
}

int LoLEngine::tlol_setPartyPosition(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_setPartyPosition(%p, %p) (%d, %d)", (const void *)tim, (const void *)param, param[0], param[1]);
	if (param[0] == 1) {
		_currentDirection = param[1];
	} else if (param[0] == 0) {
		_currentBlock = param[1];
		calcCoordinates(_partyPosX, _partyPosY, _currentBlock, 0x80, 0x80);
	}

	return 1;
}

int LoLEngine::tlol_fadeClearWindow(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_fadeClearWindow(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);

	switch (param[0]) {
	case 0:
		_screen->fadeClearSceneWindow(10);
		break;

	case 1:
		if (_flags.use16ColorMode) {
			_screen->fadePalette(_screen->getPalette(1), 10);
		} else {
			_screen->getPalette(3).copy(_screen->getPalette(0), 128);
			_screen->loadSpecialColors(_screen->getPalette(3));
			_screen->fadePalette(_screen->getPalette(3), 10);
		}
		_screen->_fadeFlag = 0;
		break;

	case 2:
		_screen->fadeToBlack(10);
		break;

	case 3:
		_screen->loadSpecialColors(_screen->getPalette(3));
		_screen->fadePalette(_screen->getPalette(_flags.use16ColorMode ? 1 : 3), 10);
		_screen->_fadeFlag = 0;
		break;

	case 4:
		if (_screen->_fadeFlag != 2)
			_screen->fadeClearSceneWindow(10);
		gui_drawPlayField();
		setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
		_screen->_fadeFlag = 0;
		break;

	case 5:
		_screen->loadSpecialColors(_screen->getPalette(3));
		_screen->fadePalette(_screen->getPalette(1), 10);
		_screen->_fadeFlag = 0;
		break;

	default:
		break;
	}

	return 1;
}

int LoLEngine::tlol_copyRegion(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_copyRegion(%p, %p) (%d, %d, %d, %d, %d, %d, %d, %d)", (const void *)tim, (const void *)param, param[0], param[1], param[2], param[3], param[4], param[5], param[6], param[7]);
	_screen->copyRegion(param[0], param[1], param[2], param[3], param[4], param[5], param[6], param[7], Screen::CR_NO_P_CHECK);
	if (!param[7])
		_screen->updateScreen();
	return 1;
}

int LoLEngine::tlol_characterChat(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_characterChat(%p, %p) (%d, %d, %d)", (const void *)tim, (const void *)param, param[0], param[1], param[2]);
	playCharacterScriptChat(param[0], param[1], 1, getLangString(param[2]), 0, param, 3);
	return 1;
}

int LoLEngine::tlol_drawScene(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_drawScene(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	gui_drawScene(param[0]);
	//if (_sceneDrawPage2 != 2 && param[0] == 2)
	//	_screen->copyRegion(112 << 3, 0, 112 << 3, 0, 176 << 3, 120, _sceneDrawPage2, 2, Screen::CR_NO_P_CHECK);
	return 1;
}

int LoLEngine::tlol_update(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_update(%p, %p)", (const void *)tim, (const void *)param);
	update();
	return 1;
}

int LoLEngine::tlol_clearTextField(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_clearTextField(%p, %p)", (const void *)tim, (const void *)param);
	if (_currentControlMode && !textEnabled())
		return 1;
	_screen->setScreenDim(5);
	const ScreenDim *d = _screen->_curDim;
	_screen->fillRect(d->sx, d->sy, d->sx + d->w - (_flags.use16ColorMode ? 3 : 2), d->sy + d->h - 2, d->unkA);
	_txt->clearDim(4);
	_txt->resetDimTextPositions(4);
	return 1;
}

int LoLEngine::tlol_loadSoundFile(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_loadSoundFile(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	snd_loadSoundFile(param[0]);
	return 1;
}

int LoLEngine::tlol_playMusicTrack(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_playMusicTrack(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	snd_playTrack(param[0]);
	return 1;
}

int LoLEngine::tlol_playDialogueTalkText(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_playDialogueTalkText(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	if (!snd_playCharacterSpeech(param[0], 0, 0) || textEnabled())
		_txt->printDialogueText(4, getLangString(param[0]), 0, param, 1);
	return 1;
}

int LoLEngine::tlol_playSoundEffect(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_playSoundEffect(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	snd_playSoundEffect(param[0], -1);
	return 1;
}

int LoLEngine::tlol_startBackgroundAnimation(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_startBackgroundAnimation(%p, %p) (%d, %d)", (const void *)tim, (const void *)param, param[0], param[1]);
	_tim->animator()->start(param[0], param[1]);
	return 1;
}

int LoLEngine::tlol_stopBackgroundAnimation(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_stopBackgroundAnimation(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	_tim->animator()->stop(param[0]);
	return 1;
}

int LoLEngine::tlol_fadeInScene(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_fadeInScene(%p, %p) (%d, %d)", (const void *)tim, (const void *)param, param[0], param[1]);
	const char *sceneFile = (const char *)(tim->text + READ_LE_UINT16(tim->text + (param[0] << 1)));
	const char *overlayFile = (const char *)(tim->text + READ_LE_UINT16(tim->text + (param[1] << 1)));

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 0, 2, Screen::CR_NO_P_CHECK);

	char filename[32];
	strcpy(filename, sceneFile);
	strcat(filename, ".CPS");

	_screen->loadBitmap(filename, 7, 5, &_screen->getPalette(0));

	uint8 *overlay = 0;
	if (!_flags.use16ColorMode) {
		filename[0] = 0;

		if (_flags.isTalkie) {
			strcpy(filename, _languageExt[_lang]);
			strcat(filename, "/");
		}

		strcat(filename, overlayFile);
		overlay = _res->fileData(filename, 0);

		for (int i = 0; i < 3; ++i) {
			uint32 endTime = _system->getMillis() + 10 * _tickLength;
			_screen->copyBlockAndApplyOverlayOutro(4, 2, overlay);
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
			delayUntil(endTime);
		}
	}

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 4, 0, Screen::CR_NO_P_CHECK);

	if (_flags.use16ColorMode) {
		_screen->fadePalette(_screen->getPalette(0), 5);
	} else {
		_screen->updateScreen();
		delete[] overlay;
	}

	return 1;
}

int LoLEngine::tlol_unusedResourceFunc(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_unusedResourceFunc(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	// The original used 0x6 / 0x7 for some resource caching, we don't need this.
	return 1;
}

int LoLEngine::tlol_fadeInPalette(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_fadeInPalette(%p, %p) (%d, %d)", (const void *)tim, (const void *)param, param[0], param[1]);
	const char *bitmap = (const char *)(tim->text + READ_LE_UINT16(tim->text + (param[0] << 1)));

	Palette pal(_screen->getPalette(0).getNumColors());
	_screen->loadBitmap(bitmap, 3, 3, &pal);

	if (_flags.use16ColorMode) {
		_screen->getPalette(0).clear();
		_screen->setScreenPalette(_screen->getPalette(0));
		_screen->copyPage(2, 0);
	}

	_screen->fadePalette(pal, param[1]);

	return 1;
}

int LoLEngine::tlol_fadeOutSound(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_fadeOutSound(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	_sound->beginFadeOut();
	return 1;
}

int LoLEngine::tlol_displayAnimFrame(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_displayAnimFrame(%p, %p) (%d, %d)", (const void *)tim, (const void *)param, param[0], param[1]);

	const int animIndex = tim->wsa[param[0]].anim - 1;
	const Movie *wsa = _tim->animator()->getWsaCPtr(animIndex);

	if (param[1] == 0xFFFF) {
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 0, 2, Screen::CR_NO_P_CHECK);
	} else {
		_tim->animator()->displayFrame(animIndex, 2, param[1], 0);
		_screen->copyRegion(wsa->xAdd(), wsa->yAdd(), wsa->xAdd(), wsa->yAdd(), wsa->width(), wsa->height(), 2, 0);
	}

	return 1;
}

int LoLEngine::tlol_delayForChat(const TIM *tim, const uint16 *param) {
	debugC(3, kDebugLevelScriptFuncs, "LoLEngine::tlol_delayForChat(%p, %p) (%d)", (const void *)tim, (const void *)param, param[0]);
	if (!speechEnabled())
		delay(param[0]);
	return 1;
}

#pragma mark -

typedef Common::Functor1Mem<EMCState *, int, LoLEngine> OpcodeV2;
#define SetOpcodeTable(x) table = &x;
#define Opcode(x) table->push_back(new OpcodeV2(this, &LoLEngine::x))
#define OpcodeUnImpl() table->push_back(new OpcodeV2(this, 0))

typedef Common::Functor2Mem<const TIM *, const uint16 *, int, LoLEngine> TIMOpcodeLoL;
#define SetTimOpcodeTable(x) timTable = &x;
#define OpcodeTim(x) timTable->push_back(new TIMOpcodeLoL(this, &LoLEngine::x))
#define OpcodeTimUnImpl() timTable->push_back(new TIMOpcodeLoL(this, 0))

void LoLEngine::setupOpcodeTable() {
	Common::Array<const Opcode *> *table = 0;

	_opcodes.reserve(192);
	SetOpcodeTable(_opcodes);
	// 0x00
	Opcode(olol_setWallType);
	Opcode(olol_getWallType);
	Opcode(olol_drawScene);
	Opcode(olol_rollDice);

	// 0x04
	Opcode(olol_moveParty);
	OpcodeUnImpl();
	Opcode(olol_delay);
	Opcode(olol_setGameFlag);

	// 0x08
	Opcode(olol_testGameFlag);
	Opcode(olol_loadLevelGraphics);
	Opcode(olol_loadBlockProperties);
	Opcode(olol_loadMonsterShapes);

	// 0x0C
	Opcode(olol_deleteHandItem);
	Opcode(olol_allocItemPropertiesBuffer);
	Opcode(olol_setItemProperty);
	Opcode(olol_makeItem);

	// 0x10
	Opcode(olol_placeMoveLevelItem);
	Opcode(olol_createLevelItem);
	Opcode(olol_getItemPara);
	Opcode(olol_getCharacterStat);

	// 0x14
	Opcode(olol_setCharacterStat);
	Opcode(olol_loadLevelShapes);
	Opcode(olol_closeLevelShapeFile);
	OpcodeUnImpl();

	// 0x18
	Opcode(olol_loadDoorShapes);
	Opcode(olol_initAnimStruct);
	Opcode(olol_playAnimationPart);
	Opcode(olol_freeAnimStruct);

	// 0x1C
	Opcode(olol_getDirection);
	Opcode(olol_characterSurpriseFeedback);
	Opcode(olol_setMusicTrack);
	Opcode(olol_setSequenceButtons);

	// 0x20
	Opcode(olol_setDefaultButtonState);
	Opcode(olol_checkRectForMousePointer);
	Opcode(olol_clearDialogueField);
	Opcode(olol_setupBackgroundAnimationPart);

	// 0x24
	Opcode(olol_startBackgroundAnimation);
	Opcode(o1_hideMouse);
	Opcode(o1_showMouse);
	Opcode(olol_fadeToBlack);

	// 0x28
	Opcode(olol_fadePalette);
	Opcode(olol_loadBitmap);
	Opcode(olol_stopBackgroundAnimation);
	OpcodeUnImpl();

	// 0x2C
	OpcodeUnImpl();
	Opcode(olol_getGlobalScriptVar);
	Opcode(olol_setGlobalScriptVar);
	Opcode(olol_getGlobalVar);

	// 0x30
	Opcode(olol_setGlobalVar);
	Opcode(olol_triggerDoorSwitch);
	Opcode(olol_checkEquippedItemScriptFlags);
	Opcode(olol_setDoorState);

	// 0x34
	Opcode(olol_updateBlockAnimations);
	Opcode(olol_assignLevelDecorationShape);
	Opcode(olol_resetBlockShapeAssignment);
	Opcode(olol_copyRegion);

	// 0x38
	Opcode(olol_initMonster);
	Opcode(olol_fadeClearSceneWindow);
	Opcode(olol_fadeSequencePalette);
	Opcode(olol_redrawPlayfield);

	// 0x3C
	Opcode(olol_loadNewLevel);
	Opcode(olol_getNearestMonsterFromCharacter);
	Opcode(olol_dummy0);
	Opcode(olol_loadMonsterProperties);

	// 0x40
	Opcode(olol_battleHitSkillTest);
	Opcode(olol_inflictDamage);
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x44
	Opcode(olol_moveMonster);
	Opcode(olol_setupDialogueButtons);
	Opcode(olol_giveTakeMoney);
	Opcode(olol_checkMoney);

	// 0x48
	Opcode(olol_setScriptTimer);
	Opcode(olol_createHandItem);
	Opcode(olol_playAttackSound);
	Opcode(olol_addRemoveCharacter);

	// 0x4C
	Opcode(olol_giveItem);
	OpcodeUnImpl();
	Opcode(olol_loadTimScript);
	Opcode(olol_runTimScript);

	// 0x50
	Opcode(olol_releaseTimScript);
	Opcode(olol_initSceneWindowDialogue);
	Opcode(olol_restoreAfterSceneWindowDialogue);
	Opcode(olol_getItemInHand);

	// 0x54
	Opcode(olol_checkMagic);
	Opcode(olol_giveItemToMonster);
	Opcode(olol_loadLangFile);
	Opcode(olol_playSoundEffect);

	// 0x58
	Opcode(olol_processDialogue);
	Opcode(olol_stopTimScript);
	Opcode(olol_getWallFlags);
	Opcode(olol_changeMonsterStat);

	// 0x5C
	Opcode(olol_getMonsterStat);
	Opcode(olol_releaseMonsterShapes);
	Opcode(olol_playCharacterScriptChat);
	Opcode(olol_update);

	// 0x60
	Opcode(olol_playEnvironmentalSfx);
	Opcode(olol_healCharacter);
	Opcode(olol_drawExitButton);
	Opcode(olol_loadSoundFile);

	// 0x64
	Opcode(olol_playMusicTrack);
	Opcode(olol_deleteMonstersFromBlock);
	Opcode(olol_countBlockItems);
	Opcode(olol_characterSkillTest);

	// 0x68
	Opcode(olol_countAllMonsters);
	Opcode(olol_playEndSequence);
	Opcode(olol_stopPortraitSpeechAnim);
	Opcode(olol_setPaletteBrightness);

	// 0x6C
	Opcode(olol_calcInflictableDamage);
	Opcode(olol_getInflictedDamage);
	Opcode(olol_checkForCertainPartyMember);
	Opcode(olol_printMessage);

	// 0x70
	Opcode(olol_deleteLevelItem);
	Opcode(olol_calcInflictableDamagePerItem);
	Opcode(olol_distanceAttack);
	Opcode(olol_removeCharacterEffects);

	// 0x74
	Opcode(olol_checkInventoryFull);
	Opcode(olol_moveBlockObjects);
	OpcodeUnImpl();
	OpcodeUnImpl();

	// 0x78
	Opcode(olol_addSpellToScroll);
	Opcode(olol_playDialogueText);
	Opcode(olol_playDialogueTalkText);
	Opcode(olol_checkMonsterTypeHostility);

	// 0x7C
	Opcode(olol_setNextFunc);
	Opcode(olol_dummy1);
	OpcodeUnImpl();
	Opcode(olol_suspendMonster);

	// 0x80
	Opcode(olol_setScriptTextParameter);
	Opcode(olol_triggerEventOnMouseButtonClick);
	Opcode(olol_printWindowText);
	Opcode(olol_countSpecificMonsters);

	// 0x84
	Opcode(olol_updateBlockAnimations2);
	Opcode(olol_checkPartyForItemType);
	Opcode(olol_blockDoor);
	Opcode(olol_resetTimDialogueState);

	// 0x88
	Opcode(olol_getItemOnPos);
	Opcode(olol_removeLevelItem);
	Opcode(olol_savePage5);
	Opcode(olol_restorePage5);

	// 0x8C
	Opcode(olol_initDialogueSequence);
	Opcode(olol_restoreAfterDialogueSequence);
	Opcode(olol_setSpecialSceneButtons);
	Opcode(olol_restoreButtonsAfterSpecialScene);

	// 0x90
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_prepareSpecialScene);
	Opcode(olol_restoreAfterSpecialScene);

	// 0x94
	Opcode(olol_assignCustomSfx);
	OpcodeUnImpl();
	Opcode(olol_findAssignedMonster);
	Opcode(olol_checkBlockForMonster);

	// 0x98
	Opcode(olol_crossFadeRegion);
	Opcode(olol_calcCoordinatesAddDirectionOffset);
	Opcode(olol_resetPortraitsAndDisableSysTimer);
	Opcode(olol_enableSysTimer);

	// 0x9C
	Opcode(olol_checkNeedSceneRestore);
	Opcode(olol_getNextActiveCharacter);
	Opcode(olol_paralyzePoisonCharacter);
	Opcode(olol_drawCharPortrait);

	// 0xA0
	Opcode(olol_removeInventoryItem);
	OpcodeUnImpl();
	OpcodeUnImpl();
	Opcode(olol_getAnimationLastPart);

	// 0xA4
	Opcode(olol_assignSpecialGuiShape);
	Opcode(olol_findInventoryItem);
	Opcode(olol_restoreFadePalette);
	Opcode(olol_calcNewBlockPosition);

	// 0xA8
	Opcode(olol_getSelectedCharacter);
	Opcode(olol_setHandItem);
	Opcode(olol_drinkBezelCup);
	Opcode(olol_changeItemTypeOrFlag);

	// 0xAC
	Opcode(olol_placeInventoryItemInHand);
	Opcode(olol_castSpell);
	Opcode(olol_pitDrop);
	Opcode(olol_increaseSkill);

	// 0xB0
	Opcode(olol_paletteFlash);
	Opcode(olol_restoreMagicShroud);
	Opcode(olol_dummy1); // anim buffer select?
	Opcode(olol_disableControls);

	// 0xB4
	Opcode(olol_enableControls);
	Opcode(olol_shakeScene);
	Opcode(olol_gasExplosion);
	Opcode(olol_calcNewBlockPosition);

	// 0xB8
	Opcode(olol_crossFadeScene);
	Opcode(olol_updateDrawPage2);
	Opcode(olol_setMouseCursor);
	Opcode(olol_characterSays);

	// 0xBC
	Opcode(olol_queueSpeech);
	Opcode(olol_getItemPrice);
	Opcode(olol_getLanguage);
	Opcode(olol_dummy0);

	Common::Array<const TIMOpcode *> *timTable = 0;

	_timIntroOpcodes.reserve(8);
	SetTimOpcodeTable(_timIntroOpcodes);

	// 0x00
	OpcodeTim(tlol_setupPaletteFade);
	OpcodeTimUnImpl();
	OpcodeTim(tlol_loadPalette);
	OpcodeTim(tlol_setupPaletteFadeEx);

	// 0x04
	OpcodeTim(tlol_processWsaFrame);
	OpcodeTim(tlol_displayText);
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();

	_timOutroOpcodes.reserve(16);
	SetTimOpcodeTable(_timOutroOpcodes);

	// 0x00
	OpcodeTim(tlol_setupPaletteFade);
	OpcodeTimUnImpl();
	OpcodeTim(tlol_loadPalette);
	OpcodeTim(tlol_setupPaletteFadeEx);

	// 0x04
	OpcodeTimUnImpl();
	OpcodeTim(tlol_fadeInScene);
	OpcodeTim(tlol_unusedResourceFunc);
	OpcodeTim(tlol_unusedResourceFunc);

	// 0x08
	OpcodeTim(tlol_fadeInPalette);
	OpcodeTimUnImpl();
	OpcodeTimUnImpl();
	OpcodeTim(tlol_fadeOutSound);

	// 0x0C
	OpcodeTim(tlol_displayAnimFrame);
	OpcodeTim(tlol_delayForChat);
	OpcodeTim(tlol_displayText);
	OpcodeTimUnImpl();

	_timIngameOpcodes.reserve(17);
	SetTimOpcodeTable(_timIngameOpcodes);

	// 0x00
	OpcodeTim(tlol_initSceneWindowDialogue);
	OpcodeTim(tlol_restoreAfterSceneWindowDialogue);
	OpcodeTimUnImpl();
	OpcodeTim(tlol_giveItem);

	// 0x04
	OpcodeTim(tlol_setPartyPosition);
	OpcodeTim(tlol_fadeClearWindow);
	OpcodeTim(tlol_copyRegion);
	OpcodeTim(tlol_characterChat);

	// 0x08
	OpcodeTim(tlol_drawScene);
	OpcodeTim(tlol_update);
	OpcodeTim(tlol_clearTextField);
	OpcodeTim(tlol_loadSoundFile);

	// 0x0C
	OpcodeTim(tlol_playMusicTrack);
	OpcodeTim(tlol_playDialogueTalkText);
	OpcodeTim(tlol_playSoundEffect);
	OpcodeTim(tlol_startBackgroundAnimation);

	// 0x10
	OpcodeTim(tlol_stopBackgroundAnimation);
}

} // End of namespace Kyra

#endif // ENABLE_LOL
