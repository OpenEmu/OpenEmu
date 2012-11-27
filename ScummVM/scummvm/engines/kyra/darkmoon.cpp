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

#ifdef ENABLE_EOB

#include "kyra/darkmoon.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

namespace Kyra {

DarkMoonEngine::DarkMoonEngine(OSystem *system, const GameFlags &flags) : EoBCoreEngine(system, flags) {
	_animIntro = _animFinale = 0;
	_shapesIntro = _shapesFinale = 0;
	_dscDoorType5Offs = 0;
	_numSpells = 70;
	_menuChoiceInit = 4;

	_introStrings = _cpsFilesIntro = _cpsFilesFinale = _finaleStrings = _kheldranStrings = _npcStrings[0] = _npcStrings[1] = _hornStrings = 0;
	_shapesIntro = _shapesFinale = 0;
	_creditsData = _npcShpData = _dscDoorType5Offs = _hornSounds = 0;
	_dreamSteps = 0;
}

DarkMoonEngine::~DarkMoonEngine() {
	delete[] _animIntro;
	delete[] _animFinale;
	delete[] _shapesIntro;
	delete[] _shapesFinale;
}

Common::Error DarkMoonEngine::init() {
	Common::Error err = EoBCoreEngine::init();
	if (err.getCode() != Common::kNoError)
		return err;

	initStaticResource();

	_monsterProps = new EoBMonsterProperty[10];

	if (_configRenderMode == Common::kRenderEGA) {
		Palette pal(16);
		_screen->loadPalette(_egaDefaultPalette, pal, 16);
		_screen->setScreenPalette(pal);
	}

	_screen->loadPalette("PALETTE.COL", _screen->getPalette(0));
	_screen->setScreenPalette(_screen->getPalette(0));

	return Common::kNoError;
}

void DarkMoonEngine::startupNew() {
	_currentLevel = 4;
	_currentSub = 0;
	loadLevel(4, 0);
	_currentBlock = 171;
	_currentDirection = 2;
	setHandItem(0);
	EoBCoreEngine::startupNew();
}

void DarkMoonEngine::drawNpcScene(int npcIndex) {
	const uint8 *shpDef = &_npcShpData[npcIndex << 3];
	for (int i = npcIndex; i != 255; i = shpDef[7]) {
		shpDef = &_npcShpData[i << 3];
		_screen->_curPage = 2;
		const uint8 *shp = _screen->encodeShape(READ_LE_UINT16(shpDef), shpDef[2], shpDef[3], shpDef[4]);
		_screen->_curPage = 0;
		_screen->drawShape(0, shp, 88 + shpDef[5] - (shp[2] << 2), 104 + shpDef[6] - shp[1], 5);
		delete[] shp;
	}
}

void DarkMoonEngine::runNpcDialogue(int npcIndex) {
	if (npcIndex == 0) {
		snd_playSoundEffect(57);
		if (npcJoinDialogue(0, 1, 3, 2))
			setScriptFlags(0x40);
	} else if (npcIndex == 1) {
		snd_playSoundEffect(53);
		gui_drawDialogueBox();

		_txt->printDialogueText(4, 0);
		int r = runDialogue(-1, 2, _npcStrings[0][0], _npcStrings[0][1]) - 1;

		if (r == 0) {
			snd_stopSound();
			delay(3 * _tickLength);
			snd_playSoundEffect(91);
			npcJoinDialogue(1, 5, 6, 7);
		} else if (r == 1) {
			setScriptFlags(0x20);
		}

	} else if (npcIndex == 2) {
		snd_playSoundEffect(55);
		gui_drawDialogueBox();

		_txt->printDialogueText(8, 0);
		int r = runDialogue(-1, 2, _npcStrings[1][0], _npcStrings[1][1]) - 1;

		if (r == 0) {
			if (rollDice(1, 2, -1))
				_txt->printDialogueText(9, _okStrings[0]);
			else
				npcJoinDialogue(2, 102, 103, 104);
			setScriptFlags(8);
		} else if (r == 1) {
			_currentDirection = 0;
		}
	}
}

void DarkMoonEngine::updateUsedCharacterHandItem(int charIndex, int slot) {
	EoBItem *itm = &_items[_characters[charIndex].inventory[slot]];
	if (itm->type == 48 || itm->type == 62) {
		if (itm->value == 5)
			return;
		int charges = itm->flags & 0x3f;
		if (--charges)
			--itm->flags;
		else
			deleteInventoryItem(charIndex, slot);
	} else if (itm->type == 26 || itm->type == 34 || itm->type == 35) {
		deleteInventoryItem(charIndex, slot);
	}
}

void DarkMoonEngine::generateMonsterPalettes(const char *file, int16 monsterIndex) {
	int cp = _screen->setCurPage(2);
	_screen->loadShapeSetBitmap(file, 3, 3);
	uint8 tmpPal[16];
	uint8 newPal[16];

	for (int i = 0; i < 6; i++) {
		int dci = monsterIndex + i;
		memcpy(tmpPal, _monsterShapes[dci] + 4, 16);
		int colx = 302 + 3 * i;

		for (int ii = 0; ii < 16; ii++) {
			uint8 col = _screen->getPagePixel(_screen->_curPage, colx, 184 + ii);
			int iii = 0;
			for (; iii < 16; iii++) {
				if (tmpPal[iii] == col) {
					newPal[ii] = iii;
					break;
				}
			}

			if (iii == 16)
				newPal[ii] = 0;
		}

		for (int ii = 1; ii < 3; ii++) {
			memcpy(tmpPal, _monsterShapes[dci] + 4, 16);

			for (int iii = 0; iii < 16; iii++) {
				uint8 col = _screen->getPagePixel(_screen->_curPage, colx + ii, 184 + iii);
				if (newPal[iii])
					tmpPal[newPal[iii]] = col;
			}

			int c = i;
			if (monsterIndex >= 18)
				c += 6;

			c = (c << 1) + (ii - 1);
			assert(c < 24);
			memcpy(_monsterPalettes[c], tmpPal, 16);
		}
	}

	_screen->setCurPage(cp);
}

void DarkMoonEngine::loadMonsterDecoration(const char *file, int16 monsterIndex) {
	Common::SeekableReadStream *s = _res->createReadStream(Common::String::format("%s.dcr", file));
	if (!s)
		return;

	int len = s->readUint16LE();

	for (int i = 0; i < len; i++) {
		for (int ii = 0; ii < 6; ii++) {
			uint8 dc[6];
			s->read(dc, 6);
			if (!dc[2] || !dc[3])
				continue;

			SpriteDecoration *m = &_monsterDecorations[i * 6 + ii + monsterIndex];

			m->shp = _screen->encodeShape(dc[0], dc[1], dc[2], dc[3]);
			m->x = (int8)dc[4];
			m->y = (int8)dc[5];
		}
	}

	delete s;
}

void DarkMoonEngine::replaceMonster(int unit, uint16 block, int pos, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem) {
	uint8 flg = _levelBlockProperties[block].flags & 7;

	if (flg == 7 || _currentBlock == block || (flg && (_monsterProps[type].u30 || pos == 4)))
		return;

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].block != block)
			continue;
		if (_monsters[i].pos == 4 || _monsterProps[_monsters[i].type].u30)
			return;
	}

	int index = -1;
	int maxDist = 0;

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].hitPointsCur <= 0) {
			index = i;
			break;
		}

		if (_monsters[i].flags & 0x40)
			continue;

		int dist = getBlockDistance(_monsters[i].block, _currentBlock);

		if (dist > maxDist) {
			maxDist = dist;
			index = i;
		}
	}

	if (index == -1)
		return;

	if (_monsters[index].hitPointsCur > 0)
		killMonster(&_monsters[index], false);

	initMonster(index, unit, block, pos, dir, type, shpIndex, mode, h2, randItem, fixedItem);
}

bool DarkMoonEngine::killMonsterExtra(EoBMonsterInPlay *m) {
	if (_currentLevel == 16 && _currentSub == 1 && (_monsterProps[m->type].capsFlags & 4)) {
		if (m->type) {
			_playFinale = true;
			_runFlag = false;
			delay(850);
		} else {
			m->hitPointsCur = 150;
			m->curRemoteWeapon = 0;
			m->numRemoteAttacks = 255;
			m->shpIndex++;
			m->type++;
			seq_dranDragonTransformation();
		}
		return false;
	}
	return true;
}

const uint8 *DarkMoonEngine::loadDoorShapes(const char *filename, int doorIndex, const uint8 *shapeDefs) {
	_screen->loadShapeSetBitmap(filename, 3, 3);
	for (int i = 0; i < 3; i++) {
		_doorShapes[doorIndex * 3 + i] = _screen->encodeShape(READ_LE_UINT16(shapeDefs), READ_LE_UINT16(shapeDefs + 2), READ_LE_UINT16(shapeDefs + 4), READ_LE_UINT16(shapeDefs + 6));
		shapeDefs += 8;
	}

	for (int i = 0; i < 2; i++) {
		_doorSwitches[doorIndex * 3 + i].shp = _screen->encodeShape(READ_LE_UINT16(shapeDefs), READ_LE_UINT16(shapeDefs + 2), READ_LE_UINT16(shapeDefs + 4), READ_LE_UINT16(shapeDefs + 6));
		shapeDefs += 8;
		_doorSwitches[doorIndex * 3 + i].x = *shapeDefs;
		shapeDefs += 2;
		_doorSwitches[doorIndex * 3 + i].y = *shapeDefs;
		shapeDefs += 2;
	}
	_screen->_curPage = 0;
	return shapeDefs;
}

void DarkMoonEngine::drawDoorIntern(int type, int, int x, int y, int w, int wall, int mDim, int16, int16) {
	int shapeIndex = type * 3 + 2 - mDim;
	uint8 *shp = _doorShapes[shapeIndex];
	if (!shp)
		return;

	if ((_doorType[type] == 0) || (_doorType[type] == 1)) {
		y = _dscDoorY1[mDim] - shp[1];
		x -= (shp[2] << 2);

		if (_doorType[type] == 1) {
			drawBlockObject(0, 2, shp, x, y, 5);
			shp = _doorShapes[3 + shapeIndex];
		}

		y -= ((wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult1[mDim]);

		if (_specialWallTypes[wall] == 5)
			y -= _dscDoorType5Offs[shapeIndex];

	} else if (_doorType[type] == 2) {
		x -= (shp[2] << 2);
		y = _dscDoorY2[mDim] - ((wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult3[mDim]);
	}

	drawBlockObject(0, 2, shp, x, y, 5);

	if (_wllShapeMap[wall] == -1 && !_noDoorSwitch[type])
		drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w, _doorSwitches[shapeIndex].y, 5);
}

void DarkMoonEngine::restParty_npc() {
	int insalId = -1;
	int numChar = 0;

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;
		if (testCharacter(i, 2) && _characters[i].portrait == -1)
			insalId = i;
		numChar++;
	}

	if (insalId == -1 || numChar < 5)
		return;

	removeCharacterFromParty(insalId);
	if (insalId < 4)
		exchangeCharacters(insalId, testCharacter(5, 1) ? 5 : 4);

	clearScriptFlags(6);

	if (!stripPartyItems(1, 1, 1, 1))
		stripPartyItems(2, 1, 1, 1);
	stripPartyItems(31, 0, 1, 3);
	stripPartyItems(39, 1, 0, 3);
	stripPartyItems(47, 0, 1, 2);

	_items[createItemOnCurrentBlock(28)].value = 26;

	gui_drawPlayField(false);
	gui_drawAllCharPortraitsWithStats();

	_screen->setClearScreenDim(10);
	_gui->messageDialogue2(11, 63, 6);
	_gui->messageDialogue2(11, 64, 6);
}

bool DarkMoonEngine::restParty_extraAbortCondition() {
	if (_currentLevel != 3)
		return false;

	seq_nightmare();

	return true;
}

void DarkMoonEngine::useHorn(int charIndex, int weaponSlot) {
	int v = _items[_characters[charIndex].inventory[weaponSlot]].value - 1;
	_txt->printMessage(_hornStrings[v]);
	snd_playSoundEffect(_hornSounds[v]);
}

bool DarkMoonEngine::checkPartyStatusExtra() {
	if (checkScriptFlags(0x100000))
		seq_kheldran();
	return _gui->confirmDialogue2(14, 67, 1);
}

void DarkMoonEngine::drawLightningColumn() {
	int f = rollDice(1, 2, -1);
	int y = 0;

	for (int i = 0; i < 6; i++) {
		f ^= 1;
		drawBlockObject(f, 2, _lightningColumnShape, 72, y, 5);
		y += 64;
	}
}

int DarkMoonEngine::resurrectionSelectDialogue() {
	countResurrectionCandidates();

	_rrNames[_rrCount] = _abortStrings[0];
	_rrId[_rrCount++] = 99;

	int r = _rrId[runDialogue(-1, 9, _rrNames[0], _rrNames[1], _rrNames[2], _rrNames[3], _rrNames[4], _rrNames[5], _rrNames[6], _rrNames[7], _rrNames[8]) - 1];
	if (r == 99)
		return 0;

	if (r < 0) {
		r = -r;
		if (prepareForNewPartyMember(33, r))
			initNpc(r - 1);
	} else {
		_characters[r].hitPointsCur = 1;
	}

	return 1;
}

int DarkMoonEngine::charSelectDialogue() {
	int cnt = 0;
	const char *namesList[7];
	memset(namesList, 0, 7 * sizeof(const char *));

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 3))
			continue;
		namesList[cnt++] = _characters[i].name;
	}

	namesList[cnt++] = _abortStrings[0];

	int r = runDialogue(-1, 7, namesList[0], namesList[1], namesList[2], namesList[3], namesList[4], namesList[5], namesList[6]) - 1;
	if (r == cnt - 1)
		return 99;

	for (cnt = 0; cnt < 6; cnt++) {
		if (!testCharacter(cnt, 3))
			continue;
		if (--r < 0)
			break;
	}
	return cnt;
}

void DarkMoonEngine::characterLevelGain(int charIndex) {
	EoBCharacter *c = &_characters[charIndex];
	int s = _numLevelsPerClass[c->cClass];
	for (int i = 0; i < s; i++) {
		uint32 er = getRequiredExperience(c->cClass, i, c->level[i] + 1);
		if (er == 0xffffffff)
			continue;

		increaseCharacterExperience(charIndex, er - c->experience[i] + 1);
	}
}

const KyraRpgGUISettings *DarkMoonEngine::guiSettings() {
	return &_guiSettings;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
