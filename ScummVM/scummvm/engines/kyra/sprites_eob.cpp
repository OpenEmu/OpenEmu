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

#include "kyra/eobcommon.h"
#include "kyra/script_eob.h"
#include "kyra/resource.h"
#include "kyra/timer.h"

#include "common/system.h"


namespace Kyra {

void EoBCoreEngine::loadMonsterShapes(const char *filename, int monsterIndex, bool hasDecorations, int encodeTableIndex) {
	_screen->loadShapeSetBitmap(filename, 3, 3);
	const uint16 *enc = &_encodeMonsterShpTable[encodeTableIndex << 2];

	for (int i = 0; i < 6; i++, enc += 4)
		_monsterShapes[monsterIndex + i] = _screen->encodeShape(enc[0], enc[1], enc[2], enc[3], false, _cgaMappingDefault);

	generateMonsterPalettes(filename, monsterIndex);

	if (hasDecorations)
		loadMonsterDecoration(filename, monsterIndex);

	_screen->_curPage = 0;
}

void EoBCoreEngine::releaseMonsterShapes(int first, int num) {
	for (int i = first; i < first + num; i++) {
		delete[] _monsterShapes[i];
		_monsterShapes[i] = 0;
		delete[] _monsterDecorations[i].shp;
		_monsterDecorations[i].shp = 0;
	}
}

const uint8 *EoBCoreEngine::loadMonsterProperties(const uint8 *data) {
	uint8 cmd = *data++;
	while (cmd != 0xff) {
		EoBMonsterProperty *d = &_monsterProps[cmd];
		d->armorClass = (int8)*data++;
		d->hitChance = (int8)*data++;
		d->level = (int8)*data++;
		d->hpDcTimes = *data++;
		d->hpDcPips = *data++;
		d->hpDcBase = *data++;
		d->attacksPerRound = *data++;
		d->dmgDc[0].times = *data++;
		d->dmgDc[0].pips = *data++;
		d->dmgDc[0].base = (int8)*data++;
		d->dmgDc[1].times = *data++;
		d->dmgDc[1].pips = *data++;
		d->dmgDc[1].base = (int8)*data++;
		d->dmgDc[2].times = *data++;
		d->dmgDc[2].pips = *data++;
		d->dmgDc[2].base = (int8)*data++;
		d->immunityFlags = READ_LE_UINT16(data);
		data += 2;
		d->capsFlags = READ_LE_UINT16(data);
		data += 2;
		d->typeFlags = READ_LE_UINT16(data);
		data += 2;
		d->experience = READ_LE_UINT16(data);
		data += 2;

		d->u30 = *data++;
		d->sound1 = (int8)*data++;
		d->sound2 = (int8)*data++;
		d->numRemoteAttacks = *data++;

		if (*data++ != 0xff) {
			d->remoteWeaponChangeMode = *data++;
			d->numRemoteWeapons = *data++;

			for (int i = 0; i < d->numRemoteWeapons; i++) {
				d->remoteWeapons[i] = (int8)*data;
				data += 2;
			}
		}

		d->tuResist = (int8)*data++;
		d->dmgModifierEvade = *data++;

		for (int i = 0; i < 3; i++)
			d->decorations[i] = *data++;

		cmd = *data++;
	}

	return data;
}

const uint8 *EoBCoreEngine::loadActiveMonsterData(const uint8 *data, int level) {
	for (uint8 p = *data++; p != 0xff; p = *data++) {
		uint8 v = *data++;
		_timer->setCountdown(0x20 + (p << 1), v);
		_timer->setCountdown(0x21 + (p << 1), v);
	}

	uint32 ct = _system->getMillis();
	for (int i = 0x20; i < 0x24; i++) {
		int32 del = _timer->getDelay(i);
		_timer->setNextRun(i, (i & 1) ? ct + (del >> 1) * _tickLength : ct + del * _tickLength);
	}
	_timer->resetNextRun();

	if (_hasTempDataFlags & (1 << (level - 1)))
		return data + 420;

	memset(_monsters, 0, 30 * sizeof(EoBMonsterInPlay));

	for (int i = 0; i < 30; i++, data += 14) {
		if (*data == 0xff)
			continue;

		initMonster(data[0], data[1], READ_LE_UINT16(&data[2]), data[4], (int8)data[5], data[6], data[7], data[8], data[9], READ_LE_UINT16(&data[10]), READ_LE_UINT16(&data[12]));
		_monsters[data[0]].flags |= 0x40;
	}

	return data;
}

void EoBCoreEngine::initMonster(int index, int unit, uint16 block, int pos, int dir, int type, int shpIndex, int mode, int i, int randItem, int fixedItem) {
	EoBMonsterInPlay *m = &_monsters[index];
	EoBMonsterProperty *p = &_monsterProps[type];
	memset(m, 0, sizeof(EoBMonsterInPlay));

	if (!block)
		return;

	unit <<= 1;
	if (index & 1)
		unit++;

	m->stepsTillRemoteAttack = _flags.gameID == GI_EOB2 ? rollDice(1, 3, 0) : 5;
	m->type = type;
	m->numRemoteAttacks = p->numRemoteAttacks;
	m->curRemoteWeapon = 0;
	m->unit = unit;
	m->pos = pos;
	m->shpIndex = shpIndex;
	m->mode = mode;
	m->spellStatusLeft = i;
	m->dir = dir;
	m->palette = _flags.gameID == GI_EOB2 ? (index % 3) : 0;
	m->hitPointsCur = m->hitPointsMax = _flags.gameID == GI_EOB2 ? rollDice(p->hpDcTimes, p->hpDcPips, p->hpDcBase) : (p->level == -1 ? rollDice(1, 4, 0) : rollDice(p->level, 8, 0));
	m->randItem = randItem;
	m->fixedItem = fixedItem;
	m->sub = _currentSub;

	placeMonster(m, block, dir);
}

void EoBCoreEngine::placeMonster(EoBMonsterInPlay *m, uint16 block, int dir) {
	if (block != 0xffff) {
		checkSceneUpdateNeed(m->block);
		if (_levelBlockProperties[m->block].flags & 7) {
			_levelBlockProperties[m->block].flags--;
			if (_flags.gameID == GI_EOB2)
				runLevelScript(m->block, 0x400);
		}
		m->block = block;
		_levelBlockProperties[block].flags++;
		if (_flags.gameID == GI_EOB2)
			runLevelScript(m->block, 0x200);
	}

	if (dir != -1) {
		m->dir = dir;
		block = m->block;
	}

	checkSceneUpdateNeed(block);
}

void EoBCoreEngine::killMonster(EoBMonsterInPlay *m, bool giveExperience) {
	m->hitPointsCur = -1;
	int pos = (m->pos == 4) ? rollDice(1, 4, -1) : m->pos;

	if (m->randItem) {
		if (rollDice(1, 10, 0) == 1)
			setItemPosition((Item *)&_levelBlockProperties[m->block & 0x3ff].drawObjects, m->block, duplicateItem(m->randItem), pos);
	}

	if (m->fixedItem)
		setItemPosition((Item *)&_levelBlockProperties[m->block & 0x3ff].drawObjects, m->block, duplicateItem(m->fixedItem), pos);

	if (giveExperience)
		increasePartyExperience(_monsterProps[m->type].experience);

	if (killMonsterExtra(m)) {
		placeMonster(m, 0, -1);

		if ((_flags.gameID == GI_EOB1) && (m->type == 21)) {
			_playFinale = true;
			_runFlag = false;
		}

		if (m->mode == 8)
			updateAttackingMonsterFlags();
	}
}

bool EoBCoreEngine::killMonsterExtra(EoBMonsterInPlay *) {
	return true;
}

int EoBCoreEngine::countSpecificMonsters(int type) {
	int res = 0;
	for (int i = 0; i < 30; i++) {
		if (_monsters[i].type != type || _monsters[i].sub != _currentSub || _monsters[i].hitPointsCur < 0)
			continue;
		res++;
	}
	return res;
}

void EoBCoreEngine::updateAttackingMonsterFlags() {
	EoBMonsterInPlay *m2 = 0;
	for (EoBMonsterInPlay *m = _monsters; m < &_monsters[30]; m++) {
		if (m->mode != 8)
			continue;
		m->mode = 0;
		m->dest = _currentBlock;
		m2 = m;
	}

	if (m2->type == 7)
		setScriptFlags(4);

	if (m2->type == 12)
		setScriptFlags(0x800);
}

const int8 *EoBCoreEngine::getMonstersOnBlockPositions(uint16 block) {
	memset(_monsterBlockPosArray, -1, sizeof(_monsterBlockPosArray));
	for (int8 i = 0; i < 30; i++) {
		if (_monsters[i].block != block)
			continue;
		assert(_monsters[i].pos < sizeof(_monsterBlockPosArray));
		_monsterBlockPosArray[_monsters[i].pos] = i;
	}
	return _monsterBlockPosArray;
}

int EoBCoreEngine::getClosestMonster(int charIndex, int block) {
	const int8 *pos = getMonstersOnBlockPositions(block);
	if (pos[4] != -1)
		return pos[4];

	const uint8 *p = &_monsterProximityTable[(_currentDirection << 3) + ((charIndex & 1) << 2)];
	for (int i = 0; i < 4; i++) {
		if (pos[p[i]] != -1)
			return pos[p[i]];
	}
	return -1;
}

bool EoBCoreEngine::blockHasMonsters(uint16 block) {
	return _levelBlockProperties[block].flags & 7 ? true : false;
}

bool EoBCoreEngine::isMonsterOnPos(EoBMonsterInPlay *m, uint16 block, int pos, int checkPos4) {
	return (m->block == block && (m->pos == pos || (m->pos == 4 && checkPos4))) ? true : false;
}

const int16 *EoBCoreEngine::findBlockMonsters(uint16 block, int pos, int dir, int blockDamage, int singleTargetCheckAdjacent) {
	static const uint8 cpos4[] = { 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1 };
	int include4 = (pos < 4) ? cpos4[(dir << 2) + pos] : 1;
	int16 *dst = _foundMonstersArray;

	if (blockDamage) {
		for (int i = 0; i < 30; i++) {
			if (_monsters[i].block == block && (_monsters[i].pos != 4 || include4))
				*dst++ = i;
		}

	} else if (singleTargetCheckAdjacent) {
		int16 r = -1;
		int f = 5;

		for (int i = 0; i < 30; i++) {
			const uint8 *tbl = &_findBlockMonstersTable[(dir << 4) + (pos << 2)];

			if (_monsters[i].block != block)
				continue;

			if (_monsters[i].pos == pos) {
				r = i;
				break;
			}

			for (int ii = 0; ii < 4; ii++) {
				if (_monsters[i].pos == tbl[ii] && ii < f) {
					f = ii;
					r = i;
				}
			}
		}

		*dst++ = r;

	} else {
		for (int i = 0; i < 30; i++) {
			if (isMonsterOnPos(&_monsters[i], block, pos, include4))
				*dst++ = i;
		}
	}

	*dst = -1;
	return _foundMonstersArray;
}

void EoBCoreEngine::drawBlockObject(int flipped, int page, const uint8 *shape, int x, int y, int sd, uint8 *ovl) {
	const ScreenDim *d = _screen->getScreenDim(sd);
	if (_flags.gameID == GI_EOB1)
		x &= ~1;
	_screen->drawShape(page, shape, x - (d->sx << 3), y - d->sy, sd, flipped | (ovl ? 2 : 0), ovl);
}

void EoBCoreEngine::drawMonsterShape(const uint8 *shape, int x, int y, int flipped, int flags, int palIndex) {
	uint8 *ovl = 0;

	if (flags & 2)
		ovl = _monsterFlashOverlay;
	else if (_flags.gameID == GI_EOB2 && flags & 0x20)
		ovl = _monsterStoneOverlay;
	else if (palIndex != -1)
		ovl = _monsterPalettes[palIndex];

	drawBlockObject(flipped, 2, shape, x, y, 5, ovl);
}

void EoBCoreEngine::flashMonsterShape(EoBMonsterInPlay *m) {
	disableSysTimer(2);
	_flashShapeTimer = 0;
	drawScene(1);
	m->flags &= 0xfd;
	_flashShapeTimer  = _system->getMillis() + _tickLength;
	enableSysTimer(2);

	_sceneUpdateRequired = true;
}

void EoBCoreEngine::updateAllMonsterShapes() {
	drawScene(1);
	bool updateShp = false;

	for (EoBMonsterInPlay *m = _monsters; m < &_monsters[30]; m++) {
		if (m->flags & 2) {
			m->flags &= ~2;
			updateShp = true;
			if (m->hitPointsCur <= 0)
				killMonster(m, true);
		}
	}

	if (updateShp) {
		_sceneUpdateRequired = true;
		_flashShapeTimer = _system->getMillis() + _tickLength;
	} else {
		_sceneUpdateRequired = false;
	}
	_preventMonsterFlash = false;
}

void EoBCoreEngine::drawBlockItems(int index) {
	uint16 o = _visibleBlocks[index]->drawObjects;
	uint8 w = _visibleBlocks[index]->walls[_sceneDrawVarDown];
	uint8 flg = (index == 16) ? 0x80 : _wllWallFlags[w];

	if (_wllVmpMap[w] && !(flg & 0x80))
		return;

	uint16 o2 = o = _items[o].next;
	bool forceLoop = true;
	static const int8 itemPosYNiche[] = { 0x25, 0x31, 0x38, 0x00 };
	static const int8 itemPosFin[] = { 0, -2, 1, -1, 2, 0, 1, -1 };
	int tile2 = 0;

	while (o != o2 || forceLoop) {
		EoBItem *itm = &_items[o];
		if (itm->pos == 8 || itm->pos < 4) {
			tile2 = -1;

			uint8 ps = (itm->pos == 8) ? 4 : _dscItemPosIndex[(_currentDirection << 2) + (itm->pos & 3)];
			uint16 wo = (index * 5 + ps) << 1;
			int x = _dscShapeCoords[wo] + 88;
			int y = 0;

			if (itm->pos == 8) {
				x = _dscItemShpX[index];
				y = itemPosYNiche[_dscDimMap[index]];
				ps = 0;
			} else {
				y = _dscShapeCoords[wo + 1] + 124;
			}

			int8 scaleSteps = (int8)_dscItemScaleIndex[(_dscDimMap[index] << 2) + ps];
			if ((flg & 8) && ps < 2 && scaleSteps) {
				tile2 = _dscItemTileIndex[index];
				if (tile2 != -1)
					setLevelShapesDim(tile2, _shpDmX1, _shpDmX2, 5);
				y -= 4;
			}

			if (scaleSteps >= 0) {
				const uint8 *shp = _screen->scaleShape(_dscItemShapeMap[itm->icon] < _numLargeItemShapes ? _largeItemShapes[_dscItemShapeMap[itm->icon]] : (_dscItemShapeMap[itm->icon] < 15 ? 0 : _smallItemShapes[_dscItemShapeMap[itm->icon] - 15]), scaleSteps);
				x = x + (itemPosFin[o & 7] << 1) - ((shp[2] << 3) >> 1);
				y -= shp[1];

				if (itm->pos != 8)
					y += itemPosFin[(o >> 1) & 7];

				drawBlockObject(0, 2, shp, x, y, 5);
				_screen->setShapeFadeMode(1, false);
			}
		}

		o = itm->next;
		forceLoop = false;
		if (tile2 != -1)
			setLevelShapesDim(index, _shpDmX1, _shpDmX2, 5);
	}
}

void EoBCoreEngine::drawDoor(int index) {
	int s = _visibleBlocks[index]->walls[_sceneDrawVarDown];

	if (_flags.gameID == GI_EOB1 && s == 0x85)
		s = 0;

	if (s >= _dscDoorShpIndexSize)
		return;

	int type = _dscDoorShpIndex[s];
	int d = _dscDimMap[index];
	int w = _dscShapeCoords[(index * 5 + 4) << 1];

	int x = 88 + w;
	int y = 0;

	int16 y1 = 0;
	int16 y2 = 0;
	setDoorShapeDim(index, y1, y2, 5);
	drawDoorIntern(type, index, x, y, w, s, d, y1, y2);
	drawLevelModifyScreenDim(5, _shpDmX1, 0, _shpDmX2, 15);
}

void EoBCoreEngine::drawMonsters(int index) {
	static const uint8 distMap[] = { 2, 1, 0, 4 };
	static const uint8 yAdd[] = { 20, 12, 4, 4, 2, 0, 0 };

	int blockDistance = distMap[_dscDimMap[index]];

	uint16 bl = _visibleBlockIndex[index];
	if (!bl)
		return;

	int drawObjDirIndex = _currentDirection * 5;
	int cDirOffs = _currentDirection << 2;

	EoBMonsterInPlay *drawObj[5];
	memset(drawObj, 0, 5 * sizeof(EoBMonsterInPlay *));

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].block != bl)
			continue;
		drawObj[_drawObjPosIndex[drawObjDirIndex + _monsters[i].pos]] = &_monsters[i];
	}

	for (int i = 0; i < 5; i++) {
		EoBMonsterInPlay *d = drawObj[i];
		if (!d)
			continue;

		EoBMonsterProperty *p = &_monsterProps[d->type];

		if (_flags.gameID == GI_EOB2 && (p->capsFlags & 0x100) && !(_partyEffectFlags & 0x220) && !(d->flags & 2))
			continue;

		int f = (d->animStep << 4) + cDirOffs + d->dir;
		f = (p->capsFlags & 2) ? _monsterFrmOffsTable1[f] : _monsterFrmOffsTable2[f];

		if (!blockDistance && d->curAttackFrame < 0)
			f = d->curAttackFrame + 7;

		int subFrame = ABS(f);
		int shpIndex = d->shpIndex ? 18 : 0;
		int palIndex = d->palette ? ((((shpIndex == 18) ? subFrame + 5 : subFrame - 1) << 1) + (d->palette - 1)) : -1;

		const uint8 *shp = _screen->scaleShape(_monsterShapes[subFrame + shpIndex - 1], blockDistance);

		int v30 = (subFrame == 1 || subFrame > 3) ? 1 : 0;
		int v1e = (d->pos == 4) ? 4 : _dscItemPosIndex[cDirOffs + d->pos];
		int posIndex = (index * 5 + v1e) << 1;

		int x = _dscShapeCoords[posIndex] + 88;
		int y = _dscShapeCoords[posIndex + 1] + 127;

		if (p->u30 == 1) {
			if (v30) {
				if (_flags.gameID == GI_EOB2)
					posIndex = ((posIndex >> 1) - v1e) << 1;
				y = _dscShapeCoords[posIndex + 1] + 127 + yAdd[blockDistance + ((v1e == 4 || _flags.gameID == GI_EOB1) ? 0 : 3)];
			} else {
				if (_flags.gameID == GI_EOB2)
					posIndex = ((posIndex >> 1) - v1e + 4) << 1;
				x = _dscShapeCoords[posIndex] + 88;
			}
		}

		int w = shp[2] << 3;
		int h = shp[1];

		x = x - (w >> 1) + (d->idleAnimState >> 4);
		y = y - h + (d->idleAnimState & 0x0f);

		drawMonsterShape(shp, x, y, f >= 0 ? 0 : 1, d->flags, palIndex);

		if (_flags.gameID == GI_EOB1) {
			_screen->setShapeFadeMode(1, false);
			continue;
		}

		for (int ii = 0; ii < 3; ii++) {
			if (!p->decorations[ii])
				continue;

			SpriteDecoration *dcr = &_monsterDecorations[(p->decorations[ii] - 1) * 6 + subFrame + shpIndex - 1];

			if (!dcr)
				continue;
			if (!dcr->shp)
				continue;

			shp = _screen->scaleShape(dcr->shp, blockDistance);
			int dx = dcr->x;
			int dy = dcr->y;

			for (int iii = 0; iii < blockDistance; iii++) {
				dx = (dx << 1) / 3;
				dy = (dy << 1) / 3;
			}

			drawMonsterShape(shp, x + ((f < 0) ? (w - dx - (shp[2] << 3)) : dx), y + dy, f >= 0 ? 0 : 1, d->flags, -1);
		}
		_screen->setShapeFadeMode(1, false);
	}
}

void EoBCoreEngine::drawWallOfForce(int index) {
	int d = _dscDimMap[index];
	assert(d < 3);
	int dH = _wallOfForceDsNumH[d];
	int dW = _wallOfForceDsNumW[d];
	int y = _wallOfForceDsY[d];
	int shpId = _wallOfForceShpId[d] + _teleporterPulse;
	int h = _wallOfForceShapes[shpId][1];
	int w = _wallOfForceShapes[shpId][2] << 3;

	for (int i = 0; i < dH; i++) {
		int x = _wallOfForceDsX[index];
		for (int ii = 0; ii < dW; ii++) {
			drawBlockObject(0, 2, _wallOfForceShapes[shpId], x, y, 5);
			x += w;
		}
		y += h;
		shpId ^= 1;
	}
}

void EoBCoreEngine::drawFlyingObjects(int index) {
	LevelBlockProperty *bl = _visibleBlocks[index];
	int blockIndex = _visibleBlockIndex[index];
	int w = bl->walls[_sceneDrawVarDown];

	if (_wllVmpMap[w] && !(_wllWallFlags[w] & 0x80))
		return;

	EoBFlyingObject *drawObj[5];
	memset(drawObj, 0, 5 * sizeof(EoBFlyingObject *));

	for (int i = 0; i < 10; i++) {
		if (!_flyingObjects[i].enable || blockIndex != _flyingObjects[i].curBlock)
			continue;
		drawObj[_drawObjPosIndex[_currentDirection * 5 + (_flyingObjects[i].curPos & 3)]] = &_flyingObjects[i];
	}

	for (int i = 0; i < 5; i++) {
		EoBFlyingObject *fo = drawObj[i];
		if (!fo)
			continue;

		int p = _dscItemPosIndex[(_currentDirection << 2) + (fo->curPos & 3)];
		int x = _dscShapeCoords[(index * 5 + p) << 1] + 88;
		int y = 39;

		int sclValue = _flightObjSclIndex[(index << 2) + p];
		int flipped = 0;

		if (sclValue < 0) {
			_screen->setShapeFadeMode(1, false);
			continue;
		}

		const uint8 *shp = 0;
		bool rstFade = false;

		if (fo->enable == 1) {
			int shpIx = _dscItemShapeMap[_items[fo->item].icon];
			int dirOffs = (fo->direction == _currentDirection) ? 0 : ((fo->direction == (_currentDirection ^ 2)) ? 1 : -1);

			if (dirOffs == -1 || _flightObjShpMap[shpIx] == -1) {
				shp = shpIx < _numLargeItemShapes ? _largeItemShapes[shpIx] : (shpIx < 15 ? 0 : _smallItemShapes[shpIx - 15]);
				flipped = fo->direction == ((_currentDirection + 1) & 3) ? 1 : 0;
			} else {
				shp = (_flightObjShpMap[shpIx] + dirOffs) < _numThrownItemShapes ? _thrownItemShapes[_flightObjShpMap[shpIx] + dirOffs] : _spellShapes[_flightObjShpMap[shpIx - _numThrownItemShapes] + dirOffs];
				flipped = _flightObjFlipIndex[(fo->direction << 2) + (fo->curPos & 3)];
			}

		} else {
			rstFade = true;
			shp = (fo->objectType < _numThrownItemShapes) ? _thrownItemShapes[fo->objectType] : _spellShapes[fo->objectType - _numThrownItemShapes];
			flipped = _flightObjFlipIndex[(fo->direction << 2) + (fo->curPos & 3)];

			if (fo->flags & 0x40) {
				x = _dscShapeCoords[(index * 5 + 4) << 1] + 88;
				y = 44;
			}
		}

		shp = _screen->scaleShape(shp, sclValue);

		if (rstFade) {
			_screen->setShapeFadeMode(1, false);
			rstFade = false;
		}

		x -= (shp[2] << 2);
		y -= (y == 44 ? (shp[1]  >> 1) : shp[1]);

		drawBlockObject(flipped, 2, shp, x, y, 5);
		_screen->setShapeFadeMode(1, false);
	}
}

void EoBCoreEngine::drawTeleporter(int index) {
	static const uint8 telprtX[] = { 0x28, 0x1C, 0x12 };
	static const uint8 telprtY[] = { 0x0D, 0x15, 0x1A };

	int t = 2 - _dscDimMap[index];
	if (t < 0)
		return;

	int16 x1 = _dscItemShpX[index] - telprtX[t];
	int16 y1 = telprtY[t];

	for (int i = 0; i < 2; i++) {

		int16 x2 = 0;
		int16 y2 = 0;
		int d = (t << 1) + i;
		if (!d)
			x2 = y2 = -4;

		const uint8 *shp = _teleporterShapes[d ^ _teleporterPulse];

		for (int ii = 0; ii < 13; ii++)
			drawBlockObject(0, 2, shp, x1 + x2 + _teleporterShapeCoords[d * 26 + ii * 2], y1 + y2 + _teleporterShapeCoords[d * 26 + ii * 2 + 1], 5);
	}
}

void EoBCoreEngine::updateMonsters(int unit) {
	for (int i = 0; i < 30; i++) {
		EoBMonsterInPlay *m = &_monsters[i];
		if (m->unit == unit) {
			if (m->hitPointsCur <= 0 || m->flags & 0x20)
				continue;
			if (m->directionChanged) {
				m->directionChanged = 0;
				continue;
			}

			updateMonsterDest(m);

			if (m->mode > 0)
				updateMonsterAttackMode(m);

			switch (m->mode) {
			case 0:
				updateMoveMonster(m);
				break;
			case 1:
				updateMonsterFollowPath(m, 2);
				break;
			case 2:
				updateMonsterFollowPath(m, -1);
				break;
			case 3:
				updateMonsterFollowPath(m, 1);
				break;
			case 5:
				updateMonstersStraying(m, -1);
				break;
			case 6:
				updateMonstersStraying(m, 1);
				break;
			case 7:
			case 10:
				updateMonstersSpellStatus(m);
				break;
			default:
				break;
			}

			if (m->mode != 4 && m->mode != 7 && m->mode != 8)
				m->animStep ^= 1;

			if (_monsterProps[m->type].u30 == 1)
				setBlockMonsterDirection(m->block, m->dir);
		}
	}
	checkFlyingObjects();
}

void EoBCoreEngine::updateMonsterDest(EoBMonsterInPlay *m) {
	if (m->mode >= 7 && m->mode <= 10)
		return;
	int dist = getBlockDistance(m->block, _currentBlock);
	if (dist >= 4)
		return;

	int s = getNextMonsterDirection(m->block, _currentBlock) - (m->dir << 1) - 3;

	if (s < 0)
		s += 8;

	if (s <= 2 && dist >= 2)
		return;

	m->mode = 0;
	m->dest = _currentBlock;
}

void EoBCoreEngine::updateMonsterAttackMode(EoBMonsterInPlay *m) {
	if (!(m->flags & 1) || m->mode == 10)
		return;
	if (m->mode == 8) {
		turnFriendlyMonstersHostile();
		return;
	}
	m->mode = 0;
	m->dest = _currentBlock;
}

void EoBCoreEngine::updateAllMonsterDests() {
	for (int i = 0; i < 30; i++)
		updateMonsterDest(&_monsters[i]);
}

void EoBCoreEngine::turnFriendlyMonstersHostile() {
	EoBMonsterInPlay *m = 0;
	for (int i = 0; i < 30; i++) {
		if (_monsters[i].mode == 8) {
			_monsters[i].mode = 0;
			_monsters[i].dest = _currentBlock;
			m = &_monsters[i];
		}
	}

	if (m) {
		if (m->type == 7)
			setScriptFlags(0x40000);
		else if (m->type == 12)
			setScriptFlags(0x8000000);
	}
}

int EoBCoreEngine::getNextMonsterDirection(int curBlock, int destBlock) {
	uint8 c = destBlock % 32;
	uint8 d = destBlock / 32;
	uint8 e = curBlock % 32;
	uint8 f = curBlock / 32;

	int r = 0;

	int s1 = f - d;
	int d1 = ABS(s1);
	s1 <<= 1;
	int s2 = c - e;
	int d2 = ABS(s2);
	s2 <<= 1;

	if (s1 >= d2)
		r |= 8;
	if (-s1 >= d2)
		r |= 4;
	if (s2 >= d1)
		r |= 2;
	if (-s2 >= d1)
		r |= 1;

	return _monsterDirChangeTable[r];
}

int EoBCoreEngine::getNextMonsterPos(EoBMonsterInPlay *m, int block) {
	if ((_flags.gameID == GI_EOB1 && _monsterProps[m->type].u30 != 0) || (_flags.gameID == GI_EOB2 && _monsterProps[m->type].u30 == 2))
		return -1;
	int d = findFreeMonsterPos(block, _monsterProps[m->type].u30);
	if (d < 0)
		return -1;

	int dir = m->dir;
	if (_flags.gameID == GI_EOB2) {
		if (_monsterProps[m->type].u30 == 1) {
			if (d == 9)
				return -1;

			int v = _monsterCloseAttUnkTable[d];
			if (v != -1)
				//////
				m->dir = 0;
			return v;
		}
	} else {
		dir &= 1;
	}

	for (int i = 0; i < 4; i++) {
		int v = m->dir ^ _monsterCloseAttPosTable2[(dir << 2) + i];
		if (!(d & (1 << v)))
			return v;
	}

	return -1;
}

int EoBCoreEngine::findFreeMonsterPos(int block, int size) {
	int nm = _levelBlockProperties[block].flags & 7;
	if (nm == 4)
		return -2;

	int res = 0;

	for (int i = 0; i < 30; i++) {
		EoBMonsterInPlay *m = &_monsters[i];
		if (m->block != block)
			continue;
		if (_monsterProps[m->type].u30 != size)
			return -1;

		if (m->pos == 4 && !(_flags.gameID == GI_EOB2 && m->flags & 0x20))
			m->pos = (_flags.gameID == GI_EOB2 && _monsterProps[m->type].u30 == 1) ? 0 : _monsterCloseAttPosTable1[m->dir];

		res |= (1 << m->pos);
		if (--nm == 0)
			break;
	}

	return res;
}

void EoBCoreEngine::updateMoveMonster(EoBMonsterInPlay *m) {
	EoBMonsterProperty *p = &_monsterProps[m->type];
	int d = getNextMonsterDirection(m->block, _currentBlock);

	if ((_flags.gameID == GI_EOB2) && (p->capsFlags & 0x800) && !(d & 1))
		d >>= 1;
	else
		d = m->dir;

	d = calcNewBlockPosition(m->block, d);

	if (m->dest == d && _currentBlock != d) {
		m->mode = rollDice(1, 2, -1) + 5;
		return;
	}

	if (updateMonsterTryDistanceAttack(m))
		return;

	if (updateMonsterTryCloseAttack(m, d))
		return;

	m->curAttackFrame = 0;
	walkMonster(m, m->dest);

	if (p->capsFlags & 8)
		updateMonsterTryCloseAttack(m, -1);
}

bool EoBCoreEngine::updateMonsterTryDistanceAttack(EoBMonsterInPlay *m) {
	EoBMonsterProperty *p = &_monsterProps[m->type];
	if (!m->numRemoteAttacks || ((_flags.gameID == GI_EOB1) && !(p->capsFlags & 0x40)))
		return false;

	if ((_flags.gameID == GI_EOB1 && m->stepsTillRemoteAttack < 5) || (_flags.gameID == GI_EOB2 && (rollDice(1, 3) > m->stepsTillRemoteAttack))) {
		m->stepsTillRemoteAttack++;
		return false;
	}

	if (getBlockDistance(m->block, _currentBlock) > 3 || getNextMonsterDirection(m->block, _currentBlock) != (m->dir << 1))
		return false;

	int d = m->dir;
	int bl = calcNewBlockPosition(m->block, d);

	while (bl != _currentBlock) {
		if (!(_wllWallFlags[_levelBlockProperties[bl].walls[d ^ 2]] & 3) || (_levelBlockProperties[bl].flags & 7))
			return false;
		bl = calcNewBlockPosition(bl, d);
	}

	Item itm = 0;
	if (_flags.gameID == GI_EOB1) {
		switch (m->type - 4) {
		case 0:
			launchMagicObject(-1, 9, m->block, m->pos, m->dir);
			snd_processEnvironmentalSoundEffect(31, m->block);
			break;
		case 10:
			launchMagicObject(-1, _enemyMageSpellList[m->numRemoteAttacks], m->block, m->pos, m->dir);
			snd_processEnvironmentalSoundEffect(_enemyMageSfx[m->numRemoteAttacks], m->block);
			break;
		case 11:
			itm = duplicateItem(60);
			if (itm) {
				if (!launchObject(-1, itm, m->block, m->pos, m->dir, _items[itm].type))
					_items[itm].block = -1;
			}
			break;
		case 12:
			launchMagicObject(-1, 0, m->block, m->pos, m->dir);
			snd_processEnvironmentalSoundEffect(85, m->block);
			break;
		case 13:
			snd_processEnvironmentalSoundEffect(83, m->block);
			_txt->printMessage(_monsterSpecAttStrings[1]);
			for (int i = 0; i < 6; i++)
				statusAttack(i, 4, _monsterSpecAttStrings[2], 1, 5, 9, 1);
			break;
		case 17:
			d = rollDice(1, 4, -1);
			if (d >= 3) {
				for (int i = 0; i < 6; i++) {
					if (!testCharacter(i, 3))
						continue;
					_txt->printMessage(_monsterSpecAttStrings[0], -1, _characters[i].name);
					inflictCharacterDamage(i, rollDice(2, 8, 1));
				}
				snd_processEnvironmentalSoundEffect(108, m->block);
			} else {
				launchMagicObject(-1, _beholderSpellList[d], m->block, m->pos, m->dir);
				snd_processEnvironmentalSoundEffect(_beholderSfx[d], m->block);
			}
			break;
		default:
			break;
		}

	} else {
		int cw = 0;
		if (p->remoteWeaponChangeMode == 1) {
			cw = m->curRemoteWeapon++;
			if (m->curRemoteWeapon == p->numRemoteWeapons)
				m->curRemoteWeapon = 0;
		} else if (p->remoteWeaponChangeMode == 2) {
			cw = rollDice(1, p->numRemoteWeapons, -1);
		}

		int s = p->remoteWeapons[cw];
		if (s >= 0) {
			if (s < 20) {
				monsterSpellCast(m, s);
			} else if (s == 20) {
				snd_processEnvironmentalSoundEffect(103, m->block);
				_txt->printMessage(_monsterSpecAttStrings[0]);
				for (int i = 0; i < 6; i++)
					statusAttack(i, 4, _monsterSpecAttStrings[1], 1, 5, 9, 1);
			}
		} else {
			itm = duplicateItem(-s);
			if (itm) {
				if (!launchObject(-1, itm, m->block, m->pos, m->dir, _items[itm].type))
					_items[itm].block = -1;
			}
		}
	}

	if (m->numRemoteAttacks != 255)
		m->numRemoteAttacks--;
	m->stepsTillRemoteAttack = 0;
	return true;
}

bool EoBCoreEngine::updateMonsterTryCloseAttack(EoBMonsterInPlay *m, int block) {
	if (block == -1)
		block = calcNewBlockPosition(m->block, m->dir);

	if (block != _currentBlock)
		return false;

	int r = (m->pos == 4 || (_flags.gameID == GI_EOB2 && _monsterProps[m->type].u30 == 1)) ? 1 : _monsterCloseAttChkTable1[(m->dir << 2) + m->pos];

	if (r) {
		m->flags ^= 4;
		if (!(m->flags & 4))
			return true;

		bool facing = (m->block == _visibleBlockIndex[13]);

		if (facing) {
			disableSysTimer(2);
			if (m->type == 4)
				updateEnvironmentalSfx(_monsterProps[m->type].sound1);
			m->curAttackFrame = -2;
			_flashShapeTimer = 0;
			drawScene(1);
			m->curAttackFrame = -1;
			if (m->type != 4)
				updateEnvironmentalSfx(_monsterProps[m->type].sound1);
			_flashShapeTimer = _system->getMillis() + 8 * _tickLength;
			drawScene(1);
		} else {
			updateEnvironmentalSfx(_monsterProps[m->type].sound1);
		}

		monsterCloseAttack(m);

		if (facing) {
			m->curAttackFrame = 0;
			m->animStep ^= 1;
			_sceneUpdateRequired = 1;
			enableSysTimer(2);
			_flashShapeTimer = _system->getMillis() + 8 * _tickLength;
		}
	} else {
		int b = m->block;
		if ((_levelBlockProperties[b].flags & 7) == 1) {
			m->pos = 4;
		} else {
			b = getNextMonsterPos(m, b);
			if (b >= 0)
				m->pos = b;
		}
		checkSceneUpdateNeed(m->block);
	}

	return true;
}

void EoBCoreEngine::walkMonster(EoBMonsterInPlay *m, int destBlock) {
	if (++_monsterStepCounter > 10) {
		_monsterStepCounter = 0;
		_monsterStepMode ^= 1;
	}

	const int8 *tbl = _monsterStepMode ? _monsterStepTable3 : _monsterStepTable2;

	int s = m->dir << 1;
	int b = m->block;
	int d = getNextMonsterDirection(b, destBlock);
	if (d == -1)
		return;

	if (m->flags & 8) {
		// Interestingly, the fear spell in EOB 1 does not expire.
		// I don't know whether this is intended or not.
		if (_flags.gameID == GI_EOB1) {
			d ^= 4;
		} else if (m->spellStatusLeft > 0) {
			if (--m->spellStatusLeft == 0)
				m->flags &= ~8;
			else
				d ^= 4;
		}
	}

	int d2 = (d - s) & 7;

	if (_flags.gameID == GI_EOB1) {
		if ((b + _monsterStepTable0[d >> 1] == _currentBlock) && !(d & 1)) {
			if (d2 >= 5)
				s = m->dir - 1;
			else if (d2 != 0)
				s = m->dir + 1;
			walkMonsterNextStep(m, -1, s & 3);
			return;
		}
	} else if (_flags.gameID == GI_EOB2) {
		if (b + _monsterStepTable0[d] == destBlock) {
			if (d & 1) {
				int e = _monsterStepTable1[((d - 1) << 1) + m->dir];
				if (e && (!(_monsterProps[m->type].capsFlags & 0x200) || (rollDice(1, 4) < 4))) {
					if (walkMonsterNextStep(m, b + e, -1))
						return;
				}
			} else {
				walkMonsterNextStep(m, -1, d >> 1);
				return;
			}
		}
	}

	if (d2) {
		if (d2 >= 5)
			s -= (1 + ((d & 1) ^ 1));
		else
			s += (1 + ((d & 1) ^ 1));
		s &= 7;
	}

	for (int i = 7; i > -1; i--) {
		s = (s + tbl[i]) & 7;
		uint16 b2 = (s & 1) ? 0 : calcNewBlockPosition(b, s >> 1);
		if (!b2)
			continue;
		if (walkMonsterNextStep(m, b2, s >> 1))
			return;
	}
}

bool EoBCoreEngine::walkMonsterNextStep(EoBMonsterInPlay *m, int destBlock, int direction) {
	EoBMonsterProperty *p = &_monsterProps[m->type];
	int obl = m->block;

	if (destBlock != m->block && destBlock != -1) {
		if (m->flags & 8) {
			if (getBlockDistance(destBlock, _currentBlock) < getBlockDistance(m->block, _currentBlock))
				return false;
		}

		if (destBlock == _currentBlock)
			return false;

		if (direction == -1)
			direction = m->dir;

		LevelBlockProperty *l = &_levelBlockProperties[destBlock];
		uint8 w = l->walls[direction ^ 2];

		if (!(_wllWallFlags[w] & 4)) {
			if (_flags.gameID == GI_EOB1 || !(p->capsFlags & 0x1000) || _wllShapeMap[w] != -1)
				return false;

			if (_wllWallFlags[w] & 0x20) {
				if (p->capsFlags & 4 && m->type == 1)
					l->walls[direction] = l->walls[direction ^ 2] = 72;
				else
					openDoor(destBlock);
			}

			if (direction != -1) {
				m->dir = direction;
				checkSceneUpdateNeed(m->block);
			}
			return true;
		}

		if ((l->flags & 7) && destBlock) {
			int pos = getNextMonsterPos(m, destBlock);
			if (pos == -1)
				return false;
			m->pos = pos;
		}

		placeMonster(m, destBlock, direction);
		direction = -1;
	}

	if (direction != -1)
		m->dir = direction;

	checkSceneUpdateNeed(obl);
	if (!_partyResting && p->sound2 > 0)
		snd_processEnvironmentalSoundEffect(p->sound2, m->block);

	return true;
}

void EoBCoreEngine::updateMonsterFollowPath(EoBMonsterInPlay *m, int turnSteps) {
	if (!walkMonsterNextStep(m, calcNewBlockPosition(m->block, m->dir), -1)) {
		m->dir = (m->dir + turnSteps) & 3;
		walkMonsterNextStep(m, -1, m->dir);
	}
}

void EoBCoreEngine::updateMonstersStraying(EoBMonsterInPlay *m, int a) {
	if (m->f_9 >= 0) {
		if (m->f_9 == 0)
			updateMonsterFollowPath(m, -a);

		int8 d = (m->dir + a) & 3;
		uint16 bl = calcNewBlockPosition(m->block, d);
		uint8 flg = _wllWallFlags[_levelBlockProperties[bl].walls[_dscBlockMap[d]]] & 4;

		if (m->f_9 == 0) {
			if (!flg)
				m->f_9 = -1;
			return;
		}

		if (flg) {
			walkMonsterNextStep(m, -1, d);
			m->f_9 = -1;
			return;
		}
	}

	if (walkMonsterNextStep(m, calcNewBlockPosition(m->block, m->dir), -1)) {
		m->f_9 = 1;
	} else {
		walkMonsterNextStep(m, -1, (m->dir - a) & 3);
		m->f_9 = 0;
	}
}

void EoBCoreEngine::updateMonstersSpellStatus(EoBMonsterInPlay *m) {
	if (m->spellStatusLeft) {
		if (!--m->spellStatusLeft)
			m->mode = 0;
	}
}

void EoBCoreEngine::setBlockMonsterDirection(int block, int dir) {
	for (int i = 0; i < 30; i++) {
		if (_monsters[i].block != block || _monsters[i].dir == dir)
			continue;
		_monsters[i].dir = dir;
		_monsters[i].directionChanged = 1;
	}
}

} // End of namespace Kyra

#endif // ENABLE_EOB
