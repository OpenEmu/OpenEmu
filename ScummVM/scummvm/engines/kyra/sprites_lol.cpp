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

namespace Kyra {

void LoLEngine::loadMonsterShapes(const char *file, int monsterIndex, int animType) {
	releaseMonsterShapes(monsterIndex);
	_screen->loadBitmap(file, 3, 3, 0);

	const uint8 *p = _screen->getCPagePtr(2);
	const uint8 *ts[16];

	for (int i = 0; i < 16; i++) {
		ts[i] = _screen->getPtrToShape(p, i);

		bool replaced = false;
		int pos = monsterIndex << 4;

		for (int ii = 0; ii < i; ii++) {
			if (ts[i] != ts[ii])
				continue;

			_monsterShapes[pos + i] = _monsterShapes[pos + ii];
			replaced = true;
			break;
		}

		if (!replaced)
			_monsterShapes[pos + i] = _screen->makeShapeCopy(p, i);

		int size = _screen->getShapePaletteSize(_monsterShapes[pos + i]) << 3;
		_monsterPalettes[pos + i] = new uint8[size];
		memset(_monsterPalettes[pos + i], 0, size);
	}

	for (int i = 0; i < 4; i++) {
		for (int ii = 0; ii < 16; ii++) {
			uint8 **of = &_monsterDecorationShapes[monsterIndex * 192 + i * 48 + ii * 3];
			int s = (i << 4) + ii + 17;
			of[0] = _screen->makeShapeCopy(p, s);
			of[1] = _screen->makeShapeCopy(p, s + 1);
			of[2] = _screen->makeShapeCopy(p, s + 2);
		}
	}
	_monsterAnimType[monsterIndex] = animType & 0xff;

	uint8 *palShape = _screen->makeShapeCopy(p, 16);

	_screen->clearPage(3);
	_screen->drawShape(2, palShape, 0, 0, 0, 0);

	uint8 *tmpPal1 = new uint8[64];
	uint8 *tmpPal2 = new uint8[256];
	uint16 *tmpPal3 = new uint16[256];
	memset(tmpPal1, 0, 64);

	for (int i = 0; i < 64; i++) {
		tmpPal1[i] = *p;
		p += 320;
	}

	p = _screen->getCPagePtr(2);

	for (int i = 0; i < 16; i++) {
		int pos = (monsterIndex << 4) + i;
		uint16 sz = MIN(_screen->getShapeSize(_monsterShapes[pos]) - 10, 256);
		memset(tmpPal2, 0, 256);
		memcpy(tmpPal2, _monsterShapes[pos] + 10, sz);
		memset(tmpPal3, 0xff, 256 * sizeof(uint16));
		uint8 numCol = *tmpPal2;

		for (int ii = 0; ii < numCol; ii++) {
			uint8 *cl = (uint8 *)memchr(tmpPal1, tmpPal2[1 + ii], 64);
			if (!cl)
				continue;
			tmpPal3[ii] = (uint16)(cl - tmpPal1);
		}

		for (int ii = 0; ii < 8; ii++) {
			memset(tmpPal2, 0, 256);
			memcpy(tmpPal2, _monsterShapes[pos] + 10, sz);
			for (int iii = 0; iii < numCol; iii++) {
				if (tmpPal3[iii] == 0xffff)
					continue;
				if (p[tmpPal3[iii] * 320 + ii + 1])
					tmpPal2[1 + iii] = p[tmpPal3[iii] * 320 + ii + 1];
			}
			memcpy(_monsterPalettes[pos] + ii * numCol, &tmpPal2[1], numCol);
		}
	}

	delete[] tmpPal1;
	delete[] tmpPal2;
	delete[] tmpPal3;
	delete[] palShape;
}

void LoLEngine::releaseMonsterShapes(int monsterIndex) {
	for (int i = 0; i < 16; i++) {
		int pos = (monsterIndex << 4) + i;
		int pos2 = (monsterIndex << 4) + 16;
		if (_monsterShapes[pos]) {
			uint8 *t = _monsterShapes[pos];
			delete[] _monsterShapes[pos];
			for (int ii = pos; ii < pos2; ii++) {
				if (_monsterShapes[ii] == t)
					_monsterShapes[ii] = 0;
			}
		}

		if (_monsterPalettes[pos]) {
			delete[] _monsterPalettes[pos];
			_monsterPalettes[pos] = 0;
		}
	}

	for (int i = 0; i < 192; i++) {
		int pos = (monsterIndex * 192) + i;
		if (_monsterDecorationShapes[pos]) {
			delete[] _monsterDecorationShapes[pos];
			_monsterDecorationShapes[pos] = 0;
		}
	}
}

int LoLEngine::deleteMonstersFromBlock(int block) {
	int i = _levelBlockProperties[block].assignedObjects;
	int cnt = 0;
	uint16 next = 0;

	while (i) {
		next = findObject(i)->nextAssignedObject;
		if (!(i & 0x8000)) {
			i = next;
			continue;
		}

		LoLMonster *m = &_monsters[i & 0x7fff];

		cnt++;
		setMonsterMode(m, 14);

		checkSceneUpdateNeed(m->block);

		placeMonster(m, 0, 0);

		i = next;
	}
	return cnt;
}

void LoLEngine::setMonsterMode(LoLMonster *monster, int mode) {
	if (monster->mode == 13 && mode != 14)
		return;

	if (mode == 7) {
		monster->destX = _partyPosX;
		monster->destY = _partyPosY;
	}

	if (monster->mode == 1 && mode == 7) {
		for (int i = 0; i < 30; i++) {
			if (monster->mode != 1)
				continue;
			monster->mode = mode;
			monster->fightCurTick = 0;
			monster->destX = _partyPosX;
			monster->destY = _partyPosY;
			setMonsterDirection(monster, calcMonsterDirection(monster->x, monster->y, monster->destX, monster->destY));
		}
	} else {
		monster->mode = mode;
		monster->fightCurTick = 0;
		if (mode == 14)
			monster->hitPoints = 0;
		if (mode == 13 && (monster->flags & 0x20)) {
			monster->mode = 0;
			monsterDropItems(monster);
			if (_currentLevel != 29)
				setMonsterMode(monster, 14);
			runLevelScriptCustom(0x404, -1, monster->id, monster->id, 0, 0);
			checkSceneUpdateNeed(monster->block);
			if (monster->mode == 14)
				placeMonster(monster, 0, 0);
		}
	}
}

bool LoLEngine::updateMonsterAdjustBlocks(LoLMonster *monster) {
	static const uint8 dims[] = { 0, 13, 9, 3 };
	if (monster->properties->flags & 8)
		return true;

	uint16 x1 = (monster->x & 0xff00) | 0x80;
	uint16 y1 = (monster->y & 0xff00) | 0x80;
	int x2 = _partyPosX;
	int y2 = _partyPosY;

	uint16 dir = 0;
	if (monster->properties->flags & 1) {
		dir = monster->direction >> 1;
	} else {
		dir = calcMonsterDirection(x1, y1, x2, y2);
		if ((monster->properties->flags & 2) && (dir == (monster->direction ^ 4)))
			return false;
		dir >>= 1;
	}

	calcSpriteRelPosition(x1, y1, x2, y2, dir);
	x2 >>= 8;
	y2 >>= 8;

	if (y2 < 0 || y2 > 3)
		return false;

	int t = (x2 < 0) ? -x2 : x2;
	if (t > y2)
		return false;

	for (int i = 0; i < 18; i++)
		_visibleBlocks[i] = &_levelBlockProperties[(monster->block + _dscBlockIndex[dir + i]) & 0x3ff];

	int16 fx1 = 0;
	int16 fx2 = 0;
	setLevelShapesDim(x2 + dims[y2], fx1, fx2, 13);

	return fx1 < fx2;
}

void LoLEngine::placeMonster(LoLMonster *monster, uint16 x, uint16 y) {
	bool cont = true;
	int t = monster->block;
	if (monster->block) {
		removeAssignedObjectFromBlock(&_levelBlockProperties[t], ((uint16)monster->id) | 0x8000);
		_levelBlockProperties[t].direction = 5;
		checkSceneUpdateNeed(t);
	} else {
		cont = false;
	}

	monster->block = calcBlockIndex(x, y);

	if (monster->x != x || monster->y != y) {
		monster->x = x;
		monster->y = y;
		monster->currentSubFrame = (monster->currentSubFrame + 1) & 3;
	}

	if (monster->block == 0)
		return;

	assignObjectToBlock(&_levelBlockProperties[monster->block].assignedObjects, ((uint16)monster->id) | 0x8000);
	_levelBlockProperties[monster->block].direction = 5;
	checkSceneUpdateNeed(monster->block);

	// WORKAROUND: Some monsters in the white tower have sound id's of 0xff. This is definitely a bug, since the
	// last valid track number is 249 and there is no specific handling for 0xff. Nonetheless this wouldn't
	// cause problems in the original code, because it just so happens that the invalid memory address points
	// to an entry in _ingameGMSoundIndex which just so happens to have a value of -1
	if (monster->properties->sounds[0] == 0 || monster->properties->sounds[0] == 255 || cont == false)
		return;

	if ((!(monster->properties->flags & 0x100) || ((monster->currentSubFrame & 1) == 0)) && monster->block == t)
		return;

	if (monster->block != t)
		runLevelScriptCustom(monster->block, 0x800, -1, monster->id, 0, 0);

	if (_updateFlags & 1)
		return;

	snd_processEnvironmentalSoundEffect(monster->properties->sounds[0], monster->block);
}

int LoLEngine::calcMonsterDirection(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int16 r = 0;

	int16 t1 = y1 - y2;
	if (t1 < 0) {
		r++;
		t1 = -t1;
	}

	r <<= 1;

	int16 t2 = x2 - x1;
	if (t2 < 0) {
		r++;
		t2 = -t2;
	}

	uint8 f = (t1 > t2) ? 1 : 0;

	if (t2 >= t1)
		SWAP(t1, t2);

	r = (r << 1) | f;

	t1 = (t1 + 1) >> 1;

	f = (t1 > t2) ? 1 : 0;
	r = (r << 1) | f;

	static const uint8 retVal[] = { 1, 2, 1, 0, 7, 6, 7, 0, 3, 2, 3, 4, 5, 6, 5, 4};
	return retVal[r];
}

void LoLEngine::setMonsterDirection(LoLMonster *monster, int dir) {
	monster->direction = dir;

	if (!(dir & 1) || ((monster->direction - (monster->facing << 1)) >= 2))
		monster->facing = monster->direction >> 1;

	checkSceneUpdateNeed(monster->block);
}

void LoLEngine::monsterDropItems(LoLMonster *monster) {
	uint16 a = monster->assignedItems;
	while (a) {
		uint16 b = a;
		a = _itemsInPlay[a].nextAssignedObject;
		setItemPosition(b, monster->x, monster->y, 0, 1);
	}
}

int LoLEngine::checkBlockBeforeObjectPlacement(uint16 x, uint16 y, uint16 objectWidth, uint16 testFlag, uint16 wallFlag) {
	_objectLastDirection = 0;
	uint16 x2 = 0;
	uint16 y2 = 0;
	int xOffs = 0;
	int yOffs = 0;
	int flag = 0;

	int r = testBlockPassability(calcBlockIndex(x, y), x, y, objectWidth, testFlag, wallFlag);
	if (r)
		return r;

	r = checkBlockOccupiedByParty(x, y, testFlag);
	if (r)
		return 4;

	if (x & 0x80) {
		if (((x & 0xff) + objectWidth) & 0xff00) {
			xOffs = 1;
			_objectLastDirection = 2;
			x2 = x + objectWidth;

			r = testBlockPassability(calcBlockIndex(x2, y), x, y, objectWidth, testFlag, wallFlag);
			if (r)
				return r;

			r = checkBlockOccupiedByParty(x + xOffs, y, testFlag);
			if (r)
				return 4;

			flag = 1;
		}
	} else {
		if (((x & 0xff) - objectWidth) & 0xff00) {
			xOffs = -1;
			_objectLastDirection = 6;
			x2 = x - objectWidth;

			r = testBlockPassability(calcBlockIndex(x2, y), x, y, objectWidth, testFlag, wallFlag);
			if (r)
				return r;

			r = checkBlockOccupiedByParty(x + xOffs, y, testFlag);
			if (r)
				return 4;

			flag = 1;
		}
	}

	if (y & 0x80) {
		if (((y & 0xff) + objectWidth) & 0xff00) {
			yOffs = 1;
			_objectLastDirection = 4;
			y2 = y + objectWidth;

			r = testBlockPassability(calcBlockIndex(x, y2), x, y, objectWidth, testFlag, wallFlag);
			if (r)
				return r;

			r = checkBlockOccupiedByParty(x, y + yOffs, testFlag);
			if (r)
				return 4;
			flag &= 1;
		} else {
			flag = 0;
		}
	} else {
		if (((y & 0xff) - objectWidth) & 0xff00) {
			yOffs = -1;
			_objectLastDirection = 0;
			y2 = y - objectWidth;

			r = testBlockPassability(calcBlockIndex(x, y2), x, y, objectWidth, testFlag, wallFlag);
			if (r)
				return r;

			r = checkBlockOccupiedByParty(x, y + yOffs, testFlag);
			if (r)
				return 4;
			flag &= 1;
		} else {
			flag = 0;
		}
	}

	if (!flag)
		return 0;

	r = testBlockPassability(calcBlockIndex(x2, y2), x, y, objectWidth, testFlag, wallFlag);
	if (r)
		return r;

	r = checkBlockOccupiedByParty(x + xOffs, y + yOffs, testFlag);
	if (r)
		return 4;

	return 0;
}

int LoLEngine::testBlockPassability(int block, int x, int y, int objectWidth, int testFlag, int wallFlag) {
	if (block == _currentBlock)
		testFlag &= 0xfffe;

	if (testFlag & 1) {
		_monsterCurBlock = block;
		if (testWallFlag(block, -1, wallFlag))
			return 1;
		_monsterCurBlock = 0;
	}

	if (!(testFlag & 2))
		return 0;

	uint16 obj = _levelBlockProperties[block].assignedObjects;
	while (obj & 0x8000) {
		LoLMonster *monster = &_monsters[obj & 0x7fff];

		if (monster->mode < 13) {
			int r = checkDrawObjectSpace(x, y, monster->x, monster->y);
			if ((objectWidth + monster->properties->maxWidth) > r)
				return 2;
		}

		obj = findObject(obj)->nextAssignedObject;
	}

	return 0;
}

int LoLEngine::calcMonsterSkillLevel(int id, int a) {
	const uint16 *c = getCharacterOrMonsterStats(id);
	int r = (a << 8) / c[4];

	/*
	if (!(id & 0x8000))
		r = (r * _monsterModifiers[3 + _monsterDifficulty]) >> 8;

	id &= 0x7fff;

	if (_characters[id].skillLevels[1] <= 3)
		return r;
	else if (_characters[id].skillLevels[1] <= 7)
		return (r- (r >> 2));*/

	if (id & 0x8000) {
		r = (r * _monsterModifiers[3 + _monsterDifficulty]) >> 8;
	} else {
		if (_characters[id].skillLevels[1] > 7)
			r = (r - (r >> 1));
		else if (_characters[id].skillLevels[1] > 3 && _characters[id].skillLevels[1] <= 7)
			r = (r - (r >> 2));
	}

	return r;
}

int LoLEngine::checkBlockOccupiedByParty(int x, int y, int testFlag) {
	if ((testFlag & 4) && (_currentBlock == calcBlockIndex(x, y)))
		return 1;

	return 0;
}

void LoLEngine::drawBlockObjects(int blockArrayIndex) {
	LevelBlockProperty *l = _visibleBlocks[blockArrayIndex];
	uint16 s = l->assignedObjects;
	LoLObject *obj = findObject(s);

	if (l->direction != _currentDirection) {
		l->drawObjects = 0;
		l->direction = _currentDirection;

		while (s) {
			reassignDrawObjects(_currentDirection, s, l, true);
			obj = findObject(s);
			s = obj->nextAssignedObject;
		}
	}

	s = l->drawObjects;
	while (s) {
		if (s & 0x8000) {
			s &= 0x7fff;
			if (blockArrayIndex < 15)
				drawMonster(s);
			s = _monsters[s].nextDrawObject;
		} else {
			LoLItem *i = &_itemsInPlay[s];
			int fx = _sceneItemOffs[s & 7] << 1;
			int fy = _sceneItemOffs[(s >> 1) & 7] + 5;

			if (i->flyingHeight >= 2 && blockArrayIndex >= 15) {
				s = i->nextDrawObject;
				continue;
			}

			uint8 *shp = 0;
			uint16 flg = 0;

			if (i->flyingHeight >= 2)
				fy -= ((i->flyingHeight - 1) * 6);

			if ((_itemProperties[i->itemPropertyIndex].flags & 0x1000) && !(i->shpCurFrame_flg & 0xC000)) {
				int shpIndex = _itemProperties[i->itemPropertyIndex].flags & 0x800 ? 7 : _itemProperties[i->itemPropertyIndex].shpIndex;
				int ii = 0;
				for (; ii < 8; ii++) {
					if (!_flyingObjects[ii].enable)
						continue;

					if (_flyingObjects[ii].item == s)
						break;
				}

				if (_flyingItemShapes[shpIndex].flipFlags && ((i->x ^ i->y) & 0x20))
					flg |= 0x20;

				flg |= _flyingItemShapes[shpIndex].drawFlags;

				if (ii != 8) {
					switch (_currentDirection - (_flyingObjects[ii].direction >> 1) + 3) {
					case 1:
					case 5:
						shpIndex = _flyingItemShapes[shpIndex].shapeFront;
						break;
					case 3:
						shpIndex = _flyingItemShapes[shpIndex].shapeBack;
						break;
					case 2:
					case 6:
						flg |= 0x10;
					case 0:
					case 4:
						shpIndex = _flyingItemShapes[shpIndex].shapeLeft;
						break;
					default:
						break;
					}

					shp = _thrownShapes[shpIndex];
				}

				if (shp)
					fy += (shp[2] >> 2);

			} else {
				shp = (_itemProperties[i->itemPropertyIndex].flags & 0x40) ? _gameShapes[_itemProperties[i->itemPropertyIndex].shpIndex] :
				      _itemShapes[_gameShapeMap[_itemProperties[i->itemPropertyIndex].shpIndex << 1]];
			}

			if (shp)
				drawItemOrMonster(shp, 0, i->x, i->y, fx, fy, flg, -1, false);
			s = i->nextDrawObject;
		}
	}
}

void LoLEngine::drawMonster(uint16 id) {
	LoLMonster *m = &_monsters[id];
	int16 flg = _monsterDirFlags[(_currentDirection << 2) + m->facing];
	int curFrm = getMonsterCurFrame(m, flg & 0xffef);
	uint8 *shp = 0;

	if (curFrm == -1) {
		shp = _monsterShapes[m->properties->shapeIndex << 4];
		calcDrawingLayerParameters(m->x + _monsterShiftOffs[m->shiftStep << 1], m->y + _monsterShiftOffs[(m->shiftStep << 1) + 1], _shpDmX, _shpDmY, _dmScaleW, _dmScaleH, shp, 0);
	} else {
		int d = m->flags & 7;
		bool flip = m->properties->flags & 0x200 ? true : false;
		flg &= 0x10;
		shp = _monsterShapes[(m->properties->shapeIndex << 4) + curFrm];

		if (m->properties->flags & 0x800)
			flg |= 0x20;

		uint8 *monsterPalette = d ? _monsterPalettes[(m->properties->shapeIndex << 4) + (curFrm & 0x0f)] + (shp[10] * (d - 1)) : 0;
		uint8 *brightnessOverlay = drawItemOrMonster(shp, monsterPalette, m->x + _monsterShiftOffs[m->shiftStep << 1], m->y + _monsterShiftOffs[(m->shiftStep << 1) + 1], 0, 0, flg | 1, -1, flip);

		for (int i = 0; i < 4; i++) {
			int v = m->equipmentShapes[i] - 1;
			if (v == -1)
				break;

			uint8 *shp2 = _monsterDecorationShapes[m->properties->shapeIndex * 192 + v * 48 + curFrm * 3];
			if (!shp2)
				continue;

			drawDoorOrMonsterEquipment(shp2, 0, _shpDmX, _shpDmY, flg | 1, brightnessOverlay);
		}
	}

	if (!m->damageReceived)
		return;

	int dW = _screen->getShapeScaledWidth(shp, _dmScaleW) >> 1;
	int dH = _screen->getShapeScaledHeight(shp, _dmScaleH) >> 1;

	int bloodAmount = (m->mode == 13) ? (m->fightCurTick << 1) : (m->properties->hitPoints / (m->damageReceived & 0x7fff));

	shp = _gameShapes[6];

	int bloodType = m->properties->flags & 0xc000;
	if (bloodType == 0x4000)
		bloodType = _flags.use16ColorMode ? 0xbb : 63;
	else if (bloodType == 0x8000)
		bloodType = _flags.use16ColorMode ? 0x55 : 15;
	else if (bloodType == 0xc000)
		bloodType = _flags.use16ColorMode ? 0x33 : 74;
	else
		bloodType = 0;

	uint8 *tbl = new uint8[256];
	if (bloodType) {
		for (int i = 0; i < 256; i++) {
			tbl[i] = i;
			if (i < 2 || i > 7)
				continue;
			tbl[i] += bloodType;
		}
	}

	dW += m->hitOffsX;
	dH += m->hitOffsY;

	bloodAmount = CLIP(bloodAmount, 1, 4);

	int sW = _dmScaleW / bloodAmount;
	int sH = _dmScaleH / bloodAmount;

	_screen->drawShape(_sceneDrawPage1, shp, _shpDmX + dW, _shpDmY + dH, 13, 0x124, tbl, bloodType ? 1 : 0, sW, sH);

	delete[] tbl;
}

int LoLEngine::getMonsterCurFrame(LoLMonster *m, uint16 dirFlags) {
	int tmp = 0;
	switch (_monsterAnimType[m->properties->shapeIndex]) {
	case 0:
		// default
		if (dirFlags) {
			return (m->mode == 13) ? -1 : (dirFlags + m->currentSubFrame);
		} else {
			if (m->damageReceived)
				return 12;

			switch (m->mode - 5) {
			case 0:
				return (m->properties->flags & 4) ? 13 : 0;
			case 3:
				return (m->fightCurTick + 13);
			case 6:
				return 14;
			case 8:
				return -1;
			default:
				return m->currentSubFrame;
			}
		}
		break;
	case 1:
		// monsters whose outward appearance reflects the damage they have taken
		tmp = (m->properties->hitPoints * _monsterModifiers[_monsterDifficulty]) >> 8;
		if (m->hitPoints > (tmp >> 1))
			tmp = 0;
		else if (m->hitPoints > (tmp >> 2))
			tmp = 4;
		else
			tmp = 8;

		switch (m->mode) {
		case 8:
			return (m->fightCurTick + tmp);
		case 11:
			return 12;
		case 13:
			return (m->fightCurTick + 12);
		default:
			return tmp;
		}

		break;
	case 2:
		return (m->fightCurTick >= 13) ? 13 : m->fightCurTick;
	case 3:
		switch (m->mode) {
		case 5:
			return m->damageReceived ? 5 : 6;
		case 8:
			return (m->fightCurTick + 6);
		case 11:
			return 5;
		default:
			return m->damageReceived ? 5 : m->currentSubFrame;
		}

		break;
	default:
		break;
	}

	return 0;
}

void LoLEngine::reassignDrawObjects(uint16 direction, uint16 itemIndex, LevelBlockProperty *l, bool flag) {
	if (l->direction != direction) {
		l->direction = 5;
		return;
	}

	LoLObject *newObject = findObject(itemIndex);
	int r = calcObjectPosition(newObject, direction);
	uint16 *b = &l->drawObjects;
	LoLObject *lastObject = 0;

	while (*b) {
		lastObject = findObject(*b);

		if (flag) {
			if (calcObjectPosition(lastObject, direction) >= r)
				break;
		} else {
			if (calcObjectPosition(lastObject, direction) > r)
				break;
		}

		b = &lastObject->nextDrawObject;
	}

	newObject->nextDrawObject = *b;
	*b = itemIndex;
}

void LoLEngine::redrawSceneItem() {
	assignVisibleBlocks(_currentBlock, _currentDirection);
	_screen->fillRect(112, 0, 287, 119, 0);

	static const uint8 sceneClickTileIndex[] = { 13, 16};

	int16 x1 = 0;
	int16 x2 = 0;

	for (int i = 0; i < 2; i++) {
		uint8 tile = sceneClickTileIndex[i];
		setLevelShapesDim(tile, x1, x2, 13);
		uint16 s = _visibleBlocks[tile]->drawObjects;

		int t = (i << 7) + 1;
		while (s) {
			if (s & 0x8000) {
				s = _monsters[s & 0x7fff].nextDrawObject;
			} else {
				LoLItem *item = &_itemsInPlay[s];

				if (item->shpCurFrame_flg & 0x4000) {
					if (checkDrawObjectSpace(item->x, item->y, _partyPosX, _partyPosY) < 320) {
						int fx = _sceneItemOffs[s & 7] << 1;
						int fy = _sceneItemOffs[(s >> 1) & 7] + 5;
						if (item->flyingHeight > 1)
							fy -= ((item->flyingHeight - 1) * 6);

						uint8 *shp = (_itemProperties[item->itemPropertyIndex].flags & 0x40) ? _gameShapes[_itemProperties[item->itemPropertyIndex].shpIndex] :
						             _itemShapes[_gameShapeMap[_itemProperties[item->itemPropertyIndex].shpIndex << 1]];

						drawItemOrMonster(shp, 0, item->x, item->y, fx, fy, 0, t, 0);
						_screen->updateScreen();
					}
				}

				s = item->nextDrawObject;
				t++;
			}
		}
	}
}

void LoLEngine::calcSpriteRelPosition(uint16 x1, uint16 y1, int &x2, int &y2, uint16 direction) {
	int a = x2 - x1;
	int b = y1 - y2;

	if (direction) {
		if (direction != 2)
			SWAP(a, b);
		if (direction != 3) {
			a = -a;
			if (direction != 1)
				b = -b;
		} else {
			b = -b;
		}
	}

	x2 = a;
	y2 = b;
}

void LoLEngine::drawDoor(uint8 *shape, uint8 *doorPalette, int index, int unk2, int w, int h, int flags) {
	if (!shape)
		return;

	uint8 c = _dscDoorY2[(_currentDirection << 5) + unk2];
	int r = (c / 5) + 5 * _dscDimMap[index];
	uint16 d = _dscShapeOvlIndex[r];
	uint16 t = (index << 5) + c;

	_shpDmY = _dscDoorMonsterY[t] + 120;

	if (flags & 1) {
		// TODO / UNUSED
		flags |= 1;
	}

	int u = 0;

	if (flags & 2) {
		uint8 dimW = _dscDimMap[index];
		_dmScaleW = _dscDoorMonsterScaleTable[dimW << 1];
		_dmScaleH = _dscDoorMonsterScaleTable[(dimW << 1) + 1];
		u = _dscDoor4[dimW];
	}

	d += 2;

	if (!_dmScaleW || !_dmScaleH)
		return;

	int s = _screen->getShapeScaledHeight(shape, _dmScaleH) >> 1;

	if (w)
		w = (w * _dmScaleW) >> 8;

	if (h)
		h = (h * _dmScaleH) >> 8;

	_shpDmX = _dscDoorMonsterX[t] + w + 200;
	_shpDmY = _shpDmY + 4 - s + h - u;

	if (d > 7)
		d = 7;

	if (_flags.use16ColorMode) {
		uint8 bb = _blockBrightness >> 4;
		if (d > bb)
			d -= bb;
		else
			d = 0;
	}

	uint8 *brightnessOverlay = _screen->getLevelOverlay(d);
	int doorScaledWitdh = _screen->getShapeScaledWidth(shape, _dmScaleW);

	_shpDmX -= (doorScaledWitdh >> 1);
	_shpDmY -= s;

	drawDoorOrMonsterEquipment(shape, doorPalette, _shpDmX, _shpDmY, flags, brightnessOverlay);
}

void LoLEngine::drawDoorOrMonsterEquipment(uint8 *shape, uint8 *objectPalette, int x, int y, int flags, const uint8 *brightnessOverlay) {
	int flg = 0;

	if (flags & 0x10)
		flg |= 1;

	if (flags & 0x20)
		flg |= 0x1000;

	if (flags & 0x40)
		flg |= 2;

	if (flg & 0x1000) {
		if (objectPalette)
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x9104, objectPalette, brightnessOverlay, 1, _transparencyTable1, _transparencyTable2, _dmScaleW, _dmScaleH);
		else
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x1104, brightnessOverlay, 1, _transparencyTable1, _transparencyTable2, _dmScaleW, _dmScaleH);
	} else {
		if (objectPalette)
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x8104, objectPalette, brightnessOverlay, 1, _dmScaleW, _dmScaleH);
		else
			_screen->drawShape(_sceneDrawPage1, shape, x, y, 13, flg | 0x104, brightnessOverlay, 1, _dmScaleW, _dmScaleH);
	}
}

uint8 *LoLEngine::drawItemOrMonster(uint8 *shape, uint8 *monsterPalette, int x, int y, int fineX, int fineY, int flags, int tblValue, bool vflip) {
	uint8 *ovl2 = 0;
	uint8 *brightnessOverlay = 0;
	uint8 tmpOvl[16];

	if (flags & 0x80) {
		flags &= 0xff7f;
		ovl2 = monsterPalette;
		monsterPalette = 0;
	} else {
		ovl2 = _screen->getLevelOverlay(_flags.use16ColorMode ? 5 : 4);
	}

	int r = calcDrawingLayerParameters(x, y, _shpDmX, _shpDmY, _dmScaleW, _dmScaleH, shape, vflip);

	if (tblValue == -1) {
		r = 7 - ((r / 3) - 1);
		r = CLIP(r, 0, 7);
		if (_flags.use16ColorMode) {
			uint8 bb = _blockBrightness >> 4;
			if (r > bb)
				r -= bb;
			else
				r = 0;
		}
		brightnessOverlay = _screen->getLevelOverlay(r);
	} else {
		memset(tmpOvl + 1, tblValue, 15);
		tmpOvl[0] = 0;
		monsterPalette = tmpOvl;
		brightnessOverlay = _screen->getLevelOverlay(7);
	}

	int flg = flags & 0x10 ? 1 : 0;
	if (flags & 0x20)
		flg |= 0x1000;
	if (flags & 0x40)
		flg |= 2;

	if (_flags.use16ColorMode) {
		if (_currentLevel != 22)
			flg &= 0xdfff;

	} else {
		if (_currentLevel == 22) {
			if (brightnessOverlay)
				brightnessOverlay[255] = 0;
		} else {
			flg |= 0x2000;
		}
	}

	_shpDmX += ((_dmScaleW * fineX) >> 8);
	_shpDmY += ((_dmScaleH * fineY) >> 8);

	int dH = _screen->getShapeScaledHeight(shape, _dmScaleH) >> 1;

	if (flg & 0x1000) {
		if (monsterPalette)
			_screen->drawShape(_sceneDrawPage1, shape, _shpDmX, _shpDmY, 13, flg | 0x8124, monsterPalette, brightnessOverlay, 0, _transparencyTable1, _transparencyTable2, _dmScaleW, _dmScaleH, ovl2);
		else
			_screen->drawShape(_sceneDrawPage1, shape, _shpDmX, _shpDmY, 13, flg | 0x124, brightnessOverlay, 0, _transparencyTable1, _transparencyTable2, _dmScaleW, _dmScaleH, ovl2);
	} else {
		if (monsterPalette)
			_screen->drawShape(_sceneDrawPage1, shape, _shpDmX, _shpDmY, 13, flg | 0x8124, monsterPalette, brightnessOverlay, 1, _dmScaleW, _dmScaleH, ovl2);
		else
			_screen->drawShape(_sceneDrawPage1, shape, _shpDmX, _shpDmY, 13, flg | 0x124, brightnessOverlay, 1, _dmScaleW, _dmScaleH, ovl2);
	}

	_shpDmX -= (_screen->getShapeScaledWidth(shape, _dmScaleW) >> 1);
	_shpDmY -= dH;

	return brightnessOverlay;
}

int LoLEngine::calcDrawingLayerParameters(int x1, int y1, int &x2, int &y2, uint16 &w, uint16 &h, uint8 *shape, int vflip) {
	calcSpriteRelPosition(_partyPosX, _partyPosY, x1, y1, _currentDirection);

	if (y1 < 0) {
		w = h = x2 = y2 = 0;
		return 0;
	}

	int l = y1 >> 5;
	y2 = _monsterScaleY[l];
	x2 = ((_monsterScaleX[l] * x1) >> 8) + 200;
	w = h = (_shpDmY > 120) ? 0x100 : _monsterScaleWH[_shpDmY - 56];

	if (vflip)
		// objects aligned to the ceiling (like the "lobsters" in the mines)
		y2 = ((120 - y2) >> 1) + (_screen->getShapeScaledHeight(shape, _dmScaleH) >> 1);
	else
		y2 -= (_screen->getShapeScaledHeight(shape, _dmScaleH) >> 1);

	return l;
}

void LoLEngine::updateMonster(LoLMonster *monster) {
	static const uint8 flags[] = { 1, 0, 1, 3, 3, 0, 0, 3, 4, 1, 0, 0, 4, 0, 0 };
	if (monster->mode > 14)
		return;

	int f = flags[monster->mode];
	if ((monster->speedTick++ < monster->properties->speedTotalWaitTicks) && (!(f & 4)))
		return;

	monster->speedTick = 0;

	if (monster->properties->flags & 0x40) {
		monster->hitPoints += rollDice(1, 8);
		if (monster->hitPoints > monster->properties->hitPoints)
			monster->hitPoints = monster->properties->hitPoints;
	}

	if (monster->flags & 8) {
		monster->destX = _partyPosX;
		monster->destY = _partyPosY;
	}

	if (f & 2) {
		if (updateMonsterAdjustBlocks(monster)) {
			setMonsterMode(monster, 7);
			f &= 6;
		}
	}

	if ((f & 1) && (monster->flags & 0x10))
		setMonsterMode(monster, 7);

	if ((monster->mode != 11) && (monster->mode != 14)) {
		if (!(_rnd.getRandomNumber(255) & 3)) {
			monster->shiftStep = (monster->shiftStep + 1) & 0x0f;
			checkSceneUpdateNeed(monster->block);
		}
	}

	switch (monster->mode) {
	case 0:
	case 1:
		// friendly mode
		if (monster->flags & 0x10) {
			for (int i = 0; i < 30; i++) {
				if (_monsters[i].mode == 1)
					setMonsterMode(&_monsters[i], 7);
			}
		} else if (monster->mode == 1) {
			moveMonster(monster);
		}
		break;

	case 2:
		moveMonster(monster);
		break;

	case 3:
		if (updateMonsterAdjustBlocks(monster))
			setMonsterMode(monster, 7);
		for (int i = 0; i < 4; i++) {
			if (calcNewBlockPosition(monster->block, i) == _currentBlock)
				setMonsterMode(monster, 7);
		}
		break;

	case 4:
		// straying around not tracing the party
		moveStrayingMonster(monster);
		break;

	case 5:
		// second recovery phase after delivering an attack
		// monsters will rearrange positions in this phase so as to allow a maximum
		// number of monsters possible attacking at the same time
		_partyAwake = true;
		monster->fightCurTick--;
		if ((monster->fightCurTick <= 0) || (checkDrawObjectSpace(_partyPosX, _partyPosY, monster->x, monster->y) > 256) || (monster->flags & 8))
			setMonsterMode(monster, 7);
		else
			alignMonsterToParty(monster);
		break;

	case 6:
		// same as mode 5, but without rearranging
		if (--monster->fightCurTick <= 0)
			setMonsterMode(monster, 7);
		break;

	case 7:
		// monster destination is set to current party position
		// depending on the flag setting this gets updated each round
		// monster can't change mode before arriving at destination and/or attacking the party
		if (!chasePartyWithDistanceAttacks(monster))
			chasePartyWithCloseAttacks(monster);
		checkSceneUpdateNeed(monster->block);
		break;

	case 8:
		// first recovery phase after delivering an attack
		if (++monster->fightCurTick > 2) {
			setMonsterMode(monster, 5);
			monster->fightCurTick = (int8)((((8 << 8) / monster->properties->fightingStats[4]) * _monsterModifiers[6 + _monsterDifficulty]) >> 8);
		}
		checkSceneUpdateNeed(monster->block);
		break;

	case 9:
		if (--monster->fightCurTick) {
			chasePartyWithCloseAttacks(monster);
		} else {
			setMonsterMode(monster, 7);
			monster->flags &= 0xfff7;
		}
		break;

	case 12:
		checkSceneUpdateNeed(monster->block);
		if (++monster->fightCurTick > 13)
			runLevelScriptCustom(0x404, -1, monster->id, monster->id, 0, 0);
		break;

	case 13:
		// monster death
		if (++monster->fightCurTick > 2)
			killMonster(monster);
		checkSceneUpdateNeed(monster->block);
		break;

	case 14:
		monster->damageReceived = 0;
		break;

	default:
		break;
	}

	if (monster->damageReceived) {
		if (monster->damageReceived & 0x8000)
			monster->damageReceived &= 0x7fff;
		else
			monster->damageReceived = 0;
		checkSceneUpdateNeed(monster->block);
	}

	monster->flags &= 0xffef;
}

void LoLEngine::moveMonster(LoLMonster *monster) {
	static const int8 turnPos[] = { 0, 2, 6, 6, 0, 2, 4, 4, 2, 2, 4, 6, 0, 0, 4, 6, 0 };
	if (monster->x != monster->destX || monster->y != monster->destY) {
		walkMonster(monster);
	} else if (monster->direction != monster->destDirection) {
		int i = (monster->facing << 2) + (monster->destDirection >> 1);
		setMonsterDirection(monster, turnPos[i]);
	}
}

void LoLEngine::walkMonster(LoLMonster *monster) {
	if (monster->properties->flags & 0x400)
		return;

	int s = walkMonsterCalcNextStep(monster);

	if (s == -1) {
		if (walkMonsterCheckDest(monster->x, monster->y, monster, 4) != 1)
			return;

		_objectLastDirection ^= 4;
		setMonsterDirection(monster, _objectLastDirection);
	} else {
		setMonsterDirection(monster, s);
		if (monster->numDistAttacks) {
			if (getBlockDistance(monster->block, _currentBlock) >= 2) {
				if (checkForPossibleDistanceAttack(monster->block, monster->direction, 3, _currentBlock) != 5) {
					if (monster->distAttackTick)
						return;
				}
			}
		}
	}

	int fx = 0;
	int fy = 0;

	getNextStepCoords(monster->x, monster->y, fx, fy, (s == -1) ? _objectLastDirection : s);
	placeMonster(monster, fx, fy);
}

bool LoLEngine::chasePartyWithDistanceAttacks(LoLMonster *monster) {
	if (!monster->numDistAttacks)
		return false;

	if (monster->distAttackTick > 0) {
		monster->distAttackTick--;
		return false;
	}

	int dir = checkForPossibleDistanceAttack(monster->block, monster->facing, 4, _currentBlock);
	if (dir == 5)
		return false;

	int s = 0;

	if (monster->flags & 0x10) {
		s = monster->properties->numDistWeapons ? rollDice(1, monster->properties->numDistWeapons) : 0;
	} else {
		s = monster->curDistWeapon++;
		if (monster->curDistWeapon >= monster->properties->numDistWeapons)
			monster->curDistWeapon = 0;
	}

	int flyingObject = monster->properties->distWeapons[s];

	if (flyingObject & 0xc000) {
		if (getBlockDistance(monster->block, _currentBlock) > 1) {
			int type = flyingObject & 0x4000 ? 0 : 1;
			flyingObject = makeItem(flyingObject & 0x3fff, 0, 0);

			if (flyingObject) {
				if (!launchObject(type, flyingObject, monster->x, monster->y, 12, dir << 1, -1, monster->id | 0x8000, 0x3f))
					deleteItem(flyingObject);
			}
		}
	} else if (!(flyingObject & 0x2000)) {
		if (getBlockDistance(monster->block, _currentBlock) > 1)
			return false;

		if (flyingObject == 1) {
			snd_playSoundEffect(147, -1);
			shakeScene(10, 2, 2, 1);

			for (int i = 0; i < 4; i++) {
				if (!(_characters[i].flags & 1))
					continue;

				int item = removeCharacterItem(i, 15);
				if (item)
					setItemPosition(item, _partyPosX, _partyPosY, 0, 1);

				inflictDamage(i, 20, 0xFFFF, 0, 2);
			}

		} else if (flyingObject == 3) {
			// shriek
			for (int i = 0; i < 30; i++) {
				if (getBlockDistance(monster->block, _monsters[i].block) < 7)
					setMonsterMode(monster, 7);
			}
			_txt->printMessage(2, "%s", getLangString(0x401a));

		} else if (flyingObject == 4) {
			launchMagicViper();

		} else {
			return false;
		}
	}

	if (monster->numDistAttacks != 255)
		monster->numDistAttacks--;

	monster->distAttackTick = (monster->properties->fightingStats[4] * 8) >> 8;

	return true;
}

void LoLEngine::chasePartyWithCloseAttacks(LoLMonster *monster) {
	if (!(monster->flags & 8)) {
		int dir = calcMonsterDirection(monster->x & 0xff00, monster->y & 0xff00, _partyPosX & 0xff00, _partyPosY & 0xff00);
		int x1 = _partyPosX;
		int y1 = _partyPosY;

		calcSpriteRelPosition(monster->x, monster->y, x1, y1, dir >> 1);

		int t = (x1 < 0) ? -x1 : x1;
		if (y1 <= 160 && t <= 80) {
			if ((monster->direction == dir) && (monster->facing == (dir >> 1))) {
				int dst = getNearestPartyMemberFromPos(monster->x, monster->y);
				snd_playSoundEffect(monster->properties->sounds[1], -1);
				int m = monster->id | 0x8000;
				int hit = battleHitSkillTest(m, dst, 0);

				if (hit) {
					int mx = calcInflictableDamage(m, dst, hit);
					int dmg = rollDice(2, mx);
					inflictDamage(dst, dmg, m, 0, 0);
					applyMonsterAttackSkill(monster, dst, dmg);
				}

				setMonsterMode(monster, 8);
				checkSceneUpdateNeed(monster->block);

			} else {
				setMonsterDirection(monster, dir);
				checkSceneUpdateNeed(monster->block);
			}
			return;
		}
	}

	if (monster->x != monster->destX || monster->y != monster->destY) {
		walkMonster(monster);
	} else {
		setMonsterDirection(monster, monster->destDirection);
		setMonsterMode(monster, (rollDice(1, 100) <= 50) ? 4 : 3);
	}
}

int LoLEngine::walkMonsterCalcNextStep(LoLMonster *monster) {
	static const int8 walkMonsterTable1[] = { 7, -6, 5, -4, 3, -2, 1, 0 };
	static const int8 walkMonsterTable2[] = { -7, 6, -5, 4, -3, 2, -1, 0 };

	if (++_monsterStepCounter > 10) {
		_monsterStepCounter = 0;
		_monsterStepMode ^= 1;
	}

	const int8 *tbl = _monsterStepMode ? walkMonsterTable2 : walkMonsterTable1;

	int sx = monster->x;
	int sy = monster->y;
	int s = monster->direction;
	int d = calcMonsterDirection(monster->x, monster->y, monster->destX, monster->destY);

	if (monster->flags & 8)
		d ^= 4;

	d = (d - s) & 7;

	if (d >= 5)
		s = (s - 1) & 7;
	else if (d)
		s = (s + 1) & 7;

	for (int i = 7; i > -1; i--) {
		s = (s + tbl[i]) & 7;

		int fx = 0;
		int fy = 0;
		getNextStepCoords(sx, sy, fx, fy, s);
		d = walkMonsterCheckDest(fx, fy, monster, 4);

		if (!d)
			return s;

		if ((d != 1) || (s & 1) || (!(monster->properties->flags & 0x80)))
			continue;

		uint8 w = _levelBlockProperties[_monsterCurBlock].walls[(s >> 1) ^ 2];

		if (_wllWallFlags[w] & 0x20) {
			if (_specialWallTypes[w] == 5) {
				openCloseDoor(_monsterCurBlock, 1);
				return -1;
			}
		}

		if (_wllWallFlags[w] & 8)
			return -1;
	}

	return -1;
}

int LoLEngine::checkForPossibleDistanceAttack(uint16 monsterBlock, int direction, int distance, uint16 curBlock) {
	int mdist = getBlockDistance(curBlock, monsterBlock);

	if (mdist > distance)
		return 5;

	int dir = calcMonsterDirection(monsterBlock & 0x1f, monsterBlock >> 5, curBlock & 0x1f, curBlock >> 5);
	if ((dir & 1) || (dir != (direction << 1)))
		return 5;

	if (((monsterBlock & 0x1f) != (curBlock & 0x1f)) && ((monsterBlock & 0xffe0) != (curBlock & 0xffe0)))
		return 5;

	if (distance < 0)
		return 5;

	int p = monsterBlock;

	for (int i = 0; i < distance; i++) {
		p = calcNewBlockPosition(p, direction);

		if (p == curBlock)
			return direction;

		if (_wllWallFlags[_levelBlockProperties[p].walls[direction ^ 2]] & 2)
			return 5;

		if (_levelBlockProperties[p].assignedObjects & 0x8000)
			return 5;
	}

	return 5;
}

int LoLEngine::walkMonsterCheckDest(int x, int y, LoLMonster *monster, int unk) {
	uint8 m = monster->mode;
	monster->mode = 15;

	int objType = checkBlockBeforeObjectPlacement(x, y, monster->properties->maxWidth, 7, monster->properties->flags & 0x1000 ? 32 : unk);

	monster->mode = m;
	return objType;
}

void LoLEngine::getNextStepCoords(int16 srcX, int16 srcY, int &newX, int &newY, uint16 direction) {
	static const int8 stepAdjustX[] = { 0, 32, 32, 32, 0, -32, -32, -32 };
	static const int8 stepAdjustY[] = { -32, -32, 0, 32, 32, 32, 0, -32 };

	newX = (srcX + stepAdjustX[direction]) & 0x1fff;
	newY = (srcY + stepAdjustY[direction]) & 0x1fff;
}

void LoLEngine::alignMonsterToParty(LoLMonster *monster) {
	uint8 mdir = monster->direction >> 1;
	uint16 mx = monster->x;
	uint16 my = monster->y;
	uint16 *pos = (mdir & 1) ? &my : &mx;
	bool centered = (*pos & 0x7f) == 0;

	bool posFlag = true;
	if (monster->properties->maxWidth <= 63) {
		if (centered) {
			bool r = false;

			if (monster->nextAssignedObject & 0x8000) {
				r = true;
			} else {
				uint16 id = _levelBlockProperties[monster->block].assignedObjects;
				id = (id & 0x8000) ? (id & 0x7fff) : 0xffff;

				if (id != monster->id) {
					r = true;
				} else {
					for (int i = 0; i < 3; i++) {
						mdir = (mdir + 1) & 3;
						id = _levelBlockProperties[calcNewBlockPosition(monster->block, mdir)].assignedObjects;
						id = (id & 0x8000) ? (id & 0x7fff) : 0xffff;
						if (id != 0xffff) {
							r = true;
							break;
						}
					}
				}
			}

			if (r)
				posFlag = false;
		} else {
			posFlag = false;
		}
	}

	if (centered && posFlag)
		return;

	if (posFlag) {
		if (*pos & 0x80)
			*pos -= 32;
		else
			*pos += 32;
	} else {
		if (*pos & 0x80)
			*pos += 32;
		else
			*pos -= 32;
	}

	if (walkMonsterCheckDest(mx, my, monster, 4))
		return;

	int fx = _partyPosX;
	int fy = _partyPosY;
	calcSpriteRelPosition(mx, my, fx, fy, monster->direction >> 1);

	if (fx < 0)
		fx = -fx;

	if (fy > 160 || fx > 80)
		return;

	placeMonster(monster, mx, my);
}

void LoLEngine::moveStrayingMonster(LoLMonster *monster) {
	int x = 0;
	int y = 0;

	if (monster->fightCurTick) {
		uint8 d = (monster->direction - monster->fightCurTick) & 6;
		uint8 id = monster->id;

		for (int i = 0; i < 7; i++) {
			getNextStepCoords(monster->x, monster->y, x, y, d);

			if (!walkMonsterCheckDest(x, y, monster, 4)) {
				placeMonster(monster, x, y);
				setMonsterDirection(monster, d);
				if (!i) {
					if (++id > 3)
						monster->fightCurTick = 0;
				}
				return;
			}

			d = (d + monster->fightCurTick) & 6;
		}
		setMonsterMode(monster, 3);

	} else {
		monster->direction &= 6;
		getNextStepCoords(monster->x, monster->y, x, y, monster->direction);
		if (!walkMonsterCheckDest(x, y, monster, 4)) {
			placeMonster(monster, x, y);
		} else {
			monster->fightCurTick = _rnd.getRandomBit() ? 2 : -2;
			monster->direction = (monster->direction + monster->fightCurTick) & 6;
		}
	}
}

void LoLEngine::killMonster(LoLMonster *monster) {
	setMonsterMode(monster, 14);
	monsterDropItems(monster);
	checkSceneUpdateNeed(monster->block);

	uint8 w = _levelBlockProperties[monster->block].walls[0];
	uint16 f = _levelBlockProperties[monster->block].flags;
	if (_wllVmpMap[w] == 0 && _wllShapeMap[w] == 0 && !(f & 0x40) && !(monster->properties->flags & 0x1000))
		_levelBlockProperties[monster->block].flags |= 0x80;

	placeMonster(monster, 0, 0);
}

} // End of namespace Kyra

#endif // ENABLE_LOL
