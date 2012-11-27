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

#include "kyra/eob.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

namespace Kyra {

EoBEngine::EoBEngine(OSystem *system, const GameFlags &flags)
	: EoBCoreEngine(system, flags) {
	_numSpells = 53;
	_menuChoiceInit = 4;

	_turnUndeadString = 0;
	_finBonusStrings = _npcStrings[1] = _npcStrings[2] = 0;
	_npcStrings[3] = _npcStrings[4] = _npcStrings[5] = _npcStrings[6] = 0;
	_npcStrings[7] = _npcStrings[8] = _npcStrings[9] = _npcStrings[10] = 0;
	_npcShpData = _npcSubShpIndex1 = _npcSubShpIndex2 = _npcSubShpY = 0;
	_dscDoorScaleMult4 = _dscDoorScaleMult5 = _dscDoorScaleMult6 = _dscDoorY3 = 0;
	_dscDoorY4 = _dscDoorY5 = _dscDoorY6 = _dscDoorY7 = _doorShapeEncodeDefs = 0;
	_doorSwitchShapeEncodeDefs = _doorSwitchCoords = 0;
	_dscDoorCoordsExt = 0;
}

EoBEngine::~EoBEngine() {
	delete[] _itemsOverlay;
}

Common::Error EoBEngine::init() {
	Common::Error err = EoBCoreEngine::init();
	if (err.getCode() != Common::kNoError)
		return err;

	initStaticResource();

	if (_configRenderMode != Common::kRenderCGA)
		_itemsOverlay = _res->fileData((_configRenderMode == Common::kRenderEGA) ? "ITEMRMP.EGA" : "ITEMRMP.VGA", 0);

	_screen->modifyScreenDim(7, 0x01, 0xB3, 0x22, 0x12);
	_screen->modifyScreenDim(9, 0x01, 0x7D, 0x26, 0x3F);
	_screen->modifyScreenDim(12, 0x01, 0x04, 0x14, 0xA0);

	_scriptTimersCount = 1;

	if (_configRenderMode == Common::kRenderEGA) {
		Palette pal(16);
		_screen->loadPalette(_egaDefaultPalette, pal, 16);
		_screen->setScreenPalette(pal);
	} else {
		_screen->loadPalette("PALETTE.COL", _screen->getPalette(0));
	}

	return Common::kNoError;
}

void EoBEngine::startupNew() {
	_currentLevel = 1;
	_currentSub = 0;
	loadLevel(1, 0);
	_currentBlock = 490;
	_currentDirection = 0;
	setHandItem(0);

	EoBCoreEngine::startupNew();
}

void EoBEngine::startupLoad() {
	_sound->loadSoundFile("ADLIB");
}

void EoBEngine::drawNpcScene(int npcIndex) {
	_screen->copyRegion(0, 0, 0, 0, 176, 120, 6, 0, Screen::CR_NO_P_CHECK);
	switch (npcIndex) {
	case 0:
		encodeDrawNpcSeqShape(2, 88, 104);
		break;

	case 1:
		if (_npcSequenceSub == -1) {
			encodeDrawNpcSeqShape(0, 88, 104);
		} else {
			encodeDrawNpcSeqShape(0, 60, 104);
			encodeDrawNpcSeqShape(5, 116, 104);
		}
		break;

	case 2:
		if (_npcSequenceSub == -1) {
			encodeDrawNpcSeqShape(3, 88, 104);
		} else {
			encodeDrawNpcSeqShape(3, 60, 104);
			encodeDrawNpcSeqShape(_npcSubShpIndex1[_npcSequenceSub], 116, 104);
			encodeDrawNpcSeqShape(_npcSubShpIndex2[_npcSequenceSub], 116, _npcSubShpY[_npcSequenceSub]);
		}
		break;

	case 3:
		encodeDrawNpcSeqShape(7, 88, 104);
		break;

	case 4:
		encodeDrawNpcSeqShape(6, 88, 104);
		break;

	case 5:
		encodeDrawNpcSeqShape(18, 88, 88);
		break;

	case 6:
		encodeDrawNpcSeqShape(17, 88, 104);
		break;

	case 7:
		encodeDrawNpcSeqShape(4, 88, 104);
		break;

	default:
		break;
	}
}

void EoBEngine::encodeDrawNpcSeqShape(int npcIndex, int drawX, int drawY) {
	const uint8 *shpDef = &_npcShpData[npcIndex << 2];
	_screen->_curPage = 2;
	const uint8 *shp = _screen->encodeShape(shpDef[0], shpDef[1], shpDef[2], shpDef[3], false, _cgaMappingDefault);
	_screen->_curPage = 0;
	_screen->drawShape(0, shp, drawX - (shp[2] << 2), drawY - shp[1], 5);
	delete[] shp;
}

#define DLG2(txt, buttonstr) (runDialogue(txt, 2, _npcStrings[buttonstr][0], _npcStrings[buttonstr][1]) - 1)
#define DLG3(txt, buttonstr) (runDialogue(txt, 3, _npcStrings[buttonstr][0], _npcStrings[buttonstr][1], _npcStrings[buttonstr][2]) - 1)
#define DLG2A3(cond, txt, buttonstr1, buttonstr2) ((cond) ? (DLG2(txt, buttonstr1) ? 2 : 0) : DLG3(txt, buttonstr2))
#define TXT(txt) _txt->printDialogueText(txt, _moreStrings[0])

void EoBEngine::runNpcDialogue(int npcIndex) {
	int r = 0;
	int a = 0;
	Item itm = 0;

	switch (npcIndex) {
	case 0:
		for (r = 1; r == 1;) {
			gui_drawDialogueBox();
			r = DLG2A3(checkScriptFlags(0x2000), 8, 2, 1);
			if (r == 1) {
				TXT(1);
				setScriptFlags(0x2000);
			} else if (r == 0) {
				npcJoinDialogue(6, 12, 23, 2);
				setScriptFlags(0x4000);
			}
		}
		break;

	case 1:
		if (!checkScriptFlags(0x10000)) {
			if (checkScriptFlags(0x8000)) {
				a = 1;
			} else {
				setScriptFlags(0x8000);
				r = DLG2(3, 3);
			}
			if (!r)
				r = DLG2(a ? 13 : 4, 4);

			if (!r) {
				for (a = 0; a < 6; a++)
					createItemOnCurrentBlock(55);
				createItemOnCurrentBlock(62);
				setScriptFlags(0x10000);
				TXT(6);
				npcJoinDialogue(7, 7, 29, 30);
			} else {
				TXT(5);
			}
			r = 1;
		}

		if (!checkScriptFlags(0x80000)) {
			for (a = 0; a < 6; a++) {
				if (testCharacter(a, 1) && _characters[a].portrait == -9)
					break;
			}
			if (a != 6) {
				TXT(25);
				TXT(26);
				setScriptFlags(0x80000);
				r = 1;
			}
		}

		if (!checkScriptFlags(0x100000)) {
			if (deletePartyItems(6, -1)) {
				//_npcSequenceSub = 0;
				//drawNpcScene(npcIndex);
				TXT(28);
				createItemOnCurrentBlock(32);
				setScriptFlags(0x100000);
				r = 1;
			}
		}

		if (!r)
			_txt->printDialogueText(_npcStrings[0][0], true);

		break;

	case 2:
		if (checkScriptFlags(0x10000)) {
			if (checkScriptFlags(0x20000)) {
				TXT(11);
			} else {
				r = DLG2A3(!countResurrectionCandidates(), 9, 5, 6);
				if (r < 2) {
					if (r == 0)
						healParty();
					else
						resurrectionSelectDialogue();
					setScriptFlags(0x20000);
				}
			}
		} else {
			TXT(24);
		}
		break;

	case 3:
		if (!DLG2(18, 7)) {
			setScriptFlags(0x8400000);
			for (a = 0; a < 30; a++) {
				if (_monsters[a].mode == 8)
					_monsters[a].mode = 5;
			}
		} else if (deletePartyItems(49, -1)) {
			TXT(20);
			setScriptFlags(0x400000);
		} else {
			TXT(19);
		}
		break;

	case 4:
		r = DLG3(14, 8);
		if (r == 0)
			setScriptFlags(0x200000);
		else if (r == 1)
			TXT(15);
		setScriptFlags(0x800000);
		break;

	case 5:
		if (!DLG2(16, 9)) {
			TXT(17);
			for (a = 0; a < 6; a++) {
				for (r = 0; r < 2; r++) {
					itm = _characters[a].inventory[r];
					if (itm && (_items[itm].type < 51 || _items[itm].type > 56)) {
						_characters[a].inventory[r] = 0;
						setItemPosition((Item *)&_levelBlockProperties[_currentBlock].drawObjects, _currentBlock, itm, _dropItemDirIndex[(_currentDirection << 2) + rollDice(1, 2, -1)]);
					}
				}
			}
		}
		setScriptFlags(0x2000000);
		break;

	case 6:
		TXT(21);
		setScriptFlags(0x1000000);
		break;

	case 7:
		r = DLG3(22, 10);
		if (r  < 2) {
			if (r == 0)
				npcJoinDialogue(8, 27, 44, 45);
			else
				TXT(31);
			setScriptFlags(0x4000000);
		}
		break;

	default:
		break;
	}
}

#undef TXT
#undef DLG2
#undef DLG3
#undef DLG2A3

void EoBEngine::updateUsedCharacterHandItem(int charIndex, int slot) {
	EoBItem *itm = &_items[_characters[charIndex].inventory[slot]];
	if (itm->type == 48) {
		int charges = itm->flags & 0x3f;
		if (--charges)
			--itm->flags;
		else
			deleteInventoryItem(charIndex, slot);
	} else if (itm->type == 34 || itm->type == 35) {
		deleteInventoryItem(charIndex, slot);
	}
}

void EoBEngine::replaceMonster(int unit, uint16 block, int pos, int dir, int type, int shpIndex, int mode, int h2, int randItem, int fixedItem) {
	if (_levelBlockProperties[block].flags & 7)
		return;

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].hitPointsCur <= 0) {
			initMonster(i, unit, block, pos, dir, type, shpIndex, mode, h2, randItem, fixedItem);
			break;
		}
	}
}

void EoBEngine::updateScriptTimersExtra() {
	int cnt = 0;
	for (int i = 1; i < 30; i++) {
		if (_monsters[i].hitPointsCur <= 0)
			cnt++;
	}

	if (!cnt) {
		for (int i = 1; i < 30; i++) {
			if (getBlockDistance(_monsters[i].block, _currentBlock) > 3) {
				killMonster(&_monsters[i], true);
				break;
			}
		}
	}
}

void EoBEngine::loadDoorShapes(int doorType1, int shapeId1, int doorType2, int shapeId2) {
	_screen->loadShapeSetBitmap("DOOR", 5, 3);
	_screen->_curPage = 2;

	if (doorType1 != 0xff) {
		for (int i = 0; i < 3; i++) {
			const uint8 *enc = &_doorShapeEncodeDefs[(doorType1 * 3 + i) << 2];
			_doorShapes[shapeId1 + i] = _screen->encodeShape(enc[0], enc[1], enc[2], enc[3], false, (_flags.gameID == GI_EOB1) ? _cgaMappingLevel[_cgaLevelMappingIndex[_currentLevel - 1]] : 0);
			enc = &_doorSwitchShapeEncodeDefs[(doorType1 * 3 + i) << 2];
			_doorSwitches[shapeId1 + i].shp = _screen->encodeShape(enc[0], enc[1], enc[2], enc[3], false, (_flags.gameID == GI_EOB1) ? _cgaMappingLevel[_cgaLevelMappingIndex[_currentLevel - 1]] : 0);
			_doorSwitches[shapeId1 + i].x = _doorSwitchCoords[doorType1 * 6 + i * 2];
			_doorSwitches[shapeId1 + i].y = _doorSwitchCoords[doorType1 * 6 + i * 2 + 1];
		}
	}

	if (doorType2 != 0xff) {
		for (int i = 0; i < 3; i++) {
			const uint8 *enc = &_doorShapeEncodeDefs[(doorType2 * 3 + i) << 2];
			_doorShapes[shapeId2 + i] = _screen->encodeShape(enc[0], enc[1], enc[2], enc[3], false, (_flags.gameID == GI_EOB1) ? _cgaMappingLevel[_cgaLevelMappingIndex[_currentLevel - 1]] : 0);
			enc = &_doorSwitchShapeEncodeDefs[(doorType2 * 3 + i) << 2];
			_doorSwitches[shapeId2 + i].shp = _screen->encodeShape(enc[0], enc[1], enc[2], enc[3], false, (_flags.gameID == GI_EOB1) ? _cgaMappingLevel[_cgaLevelMappingIndex[_currentLevel - 1]] : 0);
			_doorSwitches[shapeId2 + i].x = _doorSwitchCoords[doorType2 * 6 + i * 2];
			_doorSwitches[shapeId2 + i].y = _doorSwitchCoords[doorType2 * 6 + i * 2 + 1];
		}
	}

	_screen->_curPage = 0;
}

void EoBEngine::drawDoorIntern(int type, int index, int x, int y, int w, int wall, int mDim, int16 y1, int16 y2) {
	int shapeIndex = type + 2 - mDim;
	uint8 *shp = _doorShapes[shapeIndex];

	int d1 = 0;
	int d2 = 0;
	int v = 0;
	const ScreenDim *td = _screen->getScreenDim(5);

	switch (_currentLevel) {
	case 4:
	case 5:
	case 6:
		y = _dscDoorY6[mDim] - shp[1];
		d1 = _dscDoorCoordsExt[index << 1] >> 3;
		d2 = _dscDoorCoordsExt[(index << 1) + 1] >> 3;
		if (_shpDmX1 > d1)
			d1 = _shpDmX1;
		if (_shpDmX2 < d2)
			d2 = _shpDmX2;
		_screen->modifyScreenDim(5, d1, td->sy, d2 - d1, td->h);
		v = ((wall < 30) ? (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult3[mDim] : _dscDoorScaleMult4[mDim]) * -1;
		v -= (shp[2] << 3);
		drawBlockObject(0, 2, shp, x + v, y, 5);
		v += (shp[2] << 3);
		drawBlockObject(1, 2, shp, x - v, y, 5);
		if (_wllShapeMap[wall] == -1)
			drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w - v, _doorSwitches[shapeIndex].y, 5);
		break;

	case 7:
	case 8:
	case 9:
		y = _dscDoorY3[mDim] - _doorShapes[shapeIndex + 3][1];
		d1 = x - (_doorShapes[shapeIndex + 3][2] << 2);
		x -= (shp[2] << 2);
		drawBlockObject(0, 2, _doorShapes[shapeIndex + 3], d1, y, 5);
		setDoorShapeDim(index, y1, y2, 5);
		y = _dscDoorY3[mDim] - ((wall < 30) ? (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult1[mDim] : _dscDoorScaleMult2[mDim]);
		drawBlockObject(0, 2, shp, x, y, 5);
		if (_wllShapeMap[wall] == -1)
			drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w, _doorSwitches[shapeIndex].y, 5);
		break;

	case 10:
	case 11:
		v = ((wall < 30) ? (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult5[mDim] : _dscDoorScaleMult6[mDim]) * -1;
		x -= (shp[2] << 2);
		y = _dscDoorY4[mDim] + v;
		drawBlockObject(0, 2, shp, x, y + v, 5);
		v = (v >> 3) + (v >> 2);
		y = _dscDoorY5[mDim];
		drawBlockObject(0, 2, _doorShapes[shapeIndex + 3], x, y - v, 5);
		if (_wllShapeMap[wall] == -1)
			drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w, _doorSwitches[shapeIndex].y, 5);
		break;

	default:
		y = (_currentLevel == 12 ? _dscDoorY6[mDim] : _dscDoorY1[mDim]) - shp[1];
		x -= (shp[2] << 2);
		y -= (wall >= 30 ? _dscDoorScaleMult2[mDim] : (wall - _dscDoorScaleOffs[wall]) * _dscDoorScaleMult1[mDim]);
		drawBlockObject(0, 2, shp, x, y, 5);

		if (_wllShapeMap[wall] == -1)
			drawBlockObject(0, 2, _doorSwitches[shapeIndex].shp, _doorSwitches[shapeIndex].x + w, _doorSwitches[shapeIndex].y, 5);
		break;
	}
}

void EoBEngine::turnUndeadAuto() {
	if (_currentLevel != 2 && _currentLevel != 7)
		return;

	int oc = _openBookChar;

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 0x0d))
			continue;

		EoBCharacter *c = &_characters[i];

		if (_itemTypes[_items[c->inventory[0]].type].extraProperties != 6 && _itemTypes[_items[c->inventory[1]].type].extraProperties != 6)
			continue;

		int l = getCharacterLevelIndex(2, c->cClass);
		if (l > -1) {
			if (c->level[l] > _openBookCasterLevel) {
				_openBookCasterLevel = c->level[l];
				_openBookChar = i;
			}
		} else {
			l = getCharacterLevelIndex(4, c->cClass);
			if (l > -1) {
				if ((c->level[l] - 2) > _openBookCasterLevel) {
					_openBookCasterLevel = (c->level[l] - 2);
					_openBookChar = i;
				}
			}
		}
	}

	if (_openBookCasterLevel)
		spellCallback_start_turnUndead();

	_openBookChar = oc;
	_openBookCasterLevel = 0;
}

void EoBEngine::turnUndeadAutoHit() {
	_txt->printMessage(_turnUndeadString[0], -1, _characters[_openBookChar].name);
	sparkEffectOffensive();
}

bool EoBEngine::checkPartyStatusExtra() {
	_screen->copyPage(0, 10);
	int cd = _screen->curDimIndex();
	gui_drawBox(0, 121, 320, 80, guiSettings()->colors.frame1, guiSettings()->colors.frame2, guiSettings()->colors.fill);
	_txt->setupField(9, false);
	_txt->printMessage(_menuStringsDefeat[0]);
	while (!shouldQuit()) {
		removeInputTop();
		if (checkInput(0, false, 0) & 0xff)
			break;
	}
	_screen->copyPage(10, 0);
	_eventList.clear();
	_screen->setScreenDim(cd);
	_txt->removePageBreakFlag();
	return true;
}

int EoBEngine::resurrectionSelectDialogue() {
	gui_drawDialogueBox();
	_txt->printDialogueText(_npcStrings[0][1]);

	int r = _rrId[runDialogue(-1, 9, _rrNames[0], _rrNames[1], _rrNames[2], _rrNames[3], _rrNames[4], _rrNames[5], _rrNames[6], _rrNames[7], _rrNames[8]) - 1];

	if (r < 0) {
		r = -r;
		deletePartyItems(33, r);
		_npcSequenceSub = r - 1;
		drawNpcScene(2);
		npcJoinDialogue(_npcSequenceSub, 32 + (_npcSequenceSub << 1), -1, 33 + (_npcSequenceSub << 1));
	} else {
		_characters[r].hitPointsCur = _characters[r].hitPointsMax;
	}

	return 1;
}

void EoBEngine::healParty() {
	int cnt = rollDice(1, 3, 2);
	for (int i = 0; i < 6 && cnt; i++) {
		if (testCharacter(i, 3))
			continue;

		_characters[i].flags &= ~4;
		neutralizePoison(i);

		if (_characters[i].hitPointsCur >= _characters[i].hitPointsMax)
			continue;

		cnt--;
		_characters[i].hitPointsCur += rollDice(1, 8, 9);
		if (_characters[i].hitPointsCur > _characters[i].hitPointsMax)
			_characters[i].hitPointsCur = _characters[i].hitPointsMax;
	}
}

const KyraRpgGUISettings *EoBEngine::guiSettings() {
	return (_configRenderMode == Common::kRenderCGA || _configRenderMode == Common::kRenderEGA) ? &_guiSettingsEGA : &_guiSettingsVGA;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
