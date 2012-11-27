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
#include "kyra/resource.h"
#include "kyra/timer.h"

#include "common/endian.h"
#include "common/system.h"

namespace Kyra {

void LoLEngine::loadLevel(int index) {
	_flagsTable[73] |= 0x08;
	setMouseCursorToIcon(0x85);
	_nextScriptFunc = 0;

	snd_stopMusic();

	stopPortraitSpeechAnim();

	for (int i = 0; i < 400; i++) {
		delete[] _levelDecorationShapes[i];
		_levelDecorationShapes[i] = 0;
	}
	_emc->unload(&_scriptData);

	resetItems(1);
	disableMonsters();
	resetBlockProperties();

	releaseMonsterShapes(0);
	releaseMonsterShapes(1);

	for (int i = 0x50; i < 0x53; i++)
		_timer->disable(i);

	_currentLevel = index;
	_updateFlags = 0;

	setDefaultButtonState();

	loadTalkFile(index);

	loadLevelWallData(index, true);
	_loadLevelFlag = 1;

	Common::String filename = Common::String::format("LEVEL%d.INI", index);

	int f = _hasTempDataFlags & (1 << (index - 1));

	runInitScript(filename.c_str(), f ? 0 : 1);

	if (f)
		restoreBlockTempData(index);

	filename = Common::String::format("LEVEL%d.INF", index);
	runInfScript(filename.c_str());

	addLevelItems();
	deleteMonstersFromBlock(_currentBlock);

	if (!_flags.use16ColorMode)
		_screen->generateGrayOverlay(_screen->getPalette(0), _screen->_grayOverlay, 32, 16, 0, 0, 128, true);

	_sceneDefaultUpdate = 0;
	if (_screen->_fadeFlag == 3)
		_screen->fadeToBlack(10);

	gui_drawPlayField();

	setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
	setMouseCursorToItemInHand();

	if (_flags.use16ColorMode)
		_screen->fadeToPalette1(10);

	snd_playTrack(_curMusicTheme);
}

void LoLEngine::addLevelItems() {
	for (int i = 0; i < 400; i++) {
		if (_itemsInPlay[i].level != _currentLevel)
			continue;

		assignBlockItem(&_levelBlockProperties[_itemsInPlay[i].block], i);

		_levelBlockProperties[_itemsInPlay[i].block].direction = 5;
		_itemsInPlay[i].nextDrawObject = 0;
	}
}

void LoLEngine::assignBlockItem(LevelBlockProperty *l, uint16 item) {
	uint16 *index = &l->assignedObjects;
	LoLObject *tmp = 0;

	while (*index & 0x8000) {
		tmp = findObject(*index);
		index = &tmp->nextAssignedObject;
	}

	tmp = findObject(item);
	((LoLItem *)tmp)->level = -1;

	uint16 ix = *index;

	if (ix == item)
		return;

	*index = item;
	index = &tmp->nextAssignedObject;

	while (*index) {
		tmp = findObject(*index);
		index = &tmp->nextAssignedObject;
	}

	*index = ix;
}

void LoLEngine::loadLevelWallData(int index, bool mapShapes) {
	Common::String filename = Common::String::format("LEVEL%d.WLL", index);

	uint32 size;
	uint8 *file = _res->fileData(filename.c_str(), &size);

	uint16 c = READ_LE_UINT16(file);
	loadLevelShpDat(_levelShpList[c], _levelDatList[c], false);

	uint8 *d = file + 2;
	size = (size - 2) / 12;
	for (uint32 i = 0; i < size; i++) {
		c = READ_LE_UINT16(d);
		d += 2;
		_wllVmpMap[c] = *d;
		d += 2;

		if (mapShapes) {
			int16 sh = (int16) READ_LE_UINT16(d);
			if (sh > 0)
				_wllShapeMap[c] = assignLevelDecorationShapes(sh);
			else
				_wllShapeMap[c] = *d;
		}
		d += 2;
		_specialWallTypes[c] = *d;
		d += 2;
		_wllWallFlags[c] = *d;
		d += 2;
		_wllAutomapData[c] = *d;
		d += 2;
	}

	delete[] file;

	delete _lvlShpFileHandle;
	_lvlShpFileHandle = 0;
}

int LoLEngine::assignLevelDecorationShapes(int index) {
	uint16 *p1 = (uint16 *)_tempBuffer5120;
	uint16 *p2 = (uint16 *)(_tempBuffer5120 + 4000);

	uint16 r = p2[index];
	if (r)
		return r;

	uint16 o = _mappedDecorationsCount++;

	memcpy(&_levelDecorationProperties[o], &_levelDecorationData[index], sizeof(LevelDecorationProperty));

	for (int i = 0; i < 10; i++) {
		uint16 t = _levelDecorationProperties[o].shapeIndex[i];
		if (t == 0xffff)
			continue;

		uint16 pv = p1[t];
		if (pv) {
			_levelDecorationProperties[o].shapeIndex[i] = pv;
		} else {
			releaseDecorations(_lvlShapeIndex, 1);
			_levelDecorationShapes[_lvlShapeIndex] = getLevelDecorationShapes(t);
			p1[t] = _lvlShapeIndex;
			_levelDecorationProperties[o].shapeIndex[i] = _lvlShapeIndex++;
		}
	}

	p2[index] = o;
	if (_levelDecorationProperties[o].next)
		_levelDecorationProperties[o].next = assignLevelDecorationShapes(_levelDecorationProperties[o].next);

	return o;
}

uint8 *LoLEngine::getLevelDecorationShapes(int shapeIndex) {
	if (_decorationCount <= shapeIndex)
		return 0;

	_lvlShpFileHandle->seek(shapeIndex * 4 + 2, SEEK_SET);
	uint32 offs = _lvlShpFileHandle->readUint32LE() + 2;
	_lvlShpFileHandle->seek(offs, SEEK_SET);

	uint8 tmp[16];
	_lvlShpFileHandle->read(tmp, 16);
	uint16 size = _screen->getShapeSize(tmp);

	_lvlShpFileHandle->seek(offs, SEEK_SET);
	uint8 *res = new uint8[size];
	_lvlShpFileHandle->read(res, size);

	return res;
}

void LoLEngine::releaseDecorations(int first, int num) {
	for (int i = first; i < (first + num); i++) {
		delete[] _levelDecorationShapes[i];
		_levelDecorationShapes[i] = 0;
	}
}

void LoLEngine::loadBlockProperties(const char *cmzFile) {
	memset(_levelBlockProperties, 0, 1024 * sizeof(LevelBlockProperty));
	_screen->loadBitmap(cmzFile, 2, 2, 0);
	const uint8 *h = _screen->getCPagePtr(2);
	uint16 len = READ_LE_UINT16(&h[4]);
	const uint8 *p = h + 6;

	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			_levelBlockProperties[i].walls[ii] = p[i * len + ii];

		_levelBlockProperties[i].direction = 5;

		if (_wllAutomapData[_levelBlockProperties[i].walls[0]] == 17) {
			_levelBlockProperties[i].flags &= 0xef;
			_levelBlockProperties[i].flags |= 0x20;
		}
	}
}

const uint8 *LoLEngine::getBlockFileData(int levelIndex) {
	_screen->loadBitmap(Common::String::format("LEVEL%d.CMZ", levelIndex).c_str(), 15, 15, 0);
	return _screen->getCPagePtr(14);
}

void LoLEngine::loadLevelShpDat(const char *shpFile, const char *datFile, bool flag) {
	memset(_tempBuffer5120, 0, 5120);

	_lvlShpFileHandle = _res->createReadStream(shpFile);
	_decorationCount = _lvlShpFileHandle->readUint16LE();

	Common::SeekableReadStream *s = _res->createReadStream(datFile);

	_levelDecorationDataSize = s->readUint16LE();
	delete[] _levelDecorationData;
	_levelDecorationData = new LevelDecorationProperty[_levelDecorationDataSize];
	for (int i = 0; i < _levelDecorationDataSize; i++) {
		LevelDecorationProperty *l = &_levelDecorationData[i];
		for (int ii = 0; ii < 10; ii++)
			l->shapeIndex[ii] = s->readUint16LE();
		for (int ii = 0; ii < 10; ii++)
			l->scaleFlag[ii] = s->readByte();
		for (int ii = 0; ii < 10; ii++)
			l->shapeX[ii] = s->readSint16LE();
		for (int ii = 0; ii < 10; ii++)
			l->shapeY[ii] = s->readSint16LE();
		l->next = s->readByte();
		l->flags = s->readByte();
	}

	delete s;

	if (!flag) {
		_mappedDecorationsCount = 1;
		_lvlShapeIndex = 1;
	}
}

void LoLEngine::loadLevelGraphics(const char *file, int specialColor, int weight, int vcnLen, int vmpLen, const char *palFile) {
	if (file) {
		_lastSpecialColor = specialColor;
		_lastSpecialColorWeight = weight;
		strcpy(_lastBlockDataFile, file);
		if (palFile) {
			strcpy(_lastOverridePalFile, palFile);
			_lastOverridePalFilePtr = _lastOverridePalFile;
		} else {
			_lastOverridePalFilePtr = 0;
		}
	}

	if (_flags.use16ColorMode) {
		if (_lastSpecialColor == 1)
			_lastSpecialColor = 0x44;
		else if (_lastSpecialColor == 0x66)
			_lastSpecialColor = scumm_stricmp(file, "YVEL2") ? 0xcc : 0x44;
		else if (_lastSpecialColor == 0x6b)
			_lastSpecialColor = 0xcc;
		else
			_lastSpecialColor = 0x44;
	}

	Common::String fname;
	const uint8 *v = 0;
	int tlen = 0;

	if (_flags.use16ColorMode) {
		fname = Common::String::format("%s.VCF", _lastBlockDataFile);
		_screen->loadBitmap(fname.c_str(), 3, 3, 0);
		v = _screen->getCPagePtr(2);
		tlen = READ_LE_UINT16(v) << 5;
		v += 2;

		delete[] _vcfBlocks;
		_vcfBlocks = new uint8[tlen];

		memcpy(_vcfBlocks, v, tlen);
	}

	fname = Common::String::format("%s.VCN", _lastBlockDataFile);
	_screen->loadBitmap(fname.c_str(), 3, 3, 0);
	v = _screen->getCPagePtr(2);
	tlen = READ_LE_UINT16(v);
	v += 2;

	if (vcnLen == -1)
		vcnLen = tlen << 5;

	delete[] _vcnBlocks;
	_vcnBlocks = new uint8[vcnLen];

	if (!_flags.use16ColorMode) {
		delete[] _vcnShift;
		_vcnShift = new uint8[tlen];

		memcpy(_vcnShift, v, tlen);
		v += tlen;

		memcpy(_vcnColTable, v, 128);
		v += 128;

		if (_lastOverridePalFilePtr) {
			_res->loadFileToBuf(_lastOverridePalFilePtr, _screen->getPalette(0).getData(), 384);
		} else {
			_screen->getPalette(0).copy(v, 0, 128);
		}

		v += 384;
	}

	if (_currentLevel == 11) {
		if (_flags.use16ColorMode) {
			_screen->loadPalette("LOLICE.NOL", _screen->getPalette(2));

		} else {
			_screen->loadPalette("SWAMPICE.COL", _screen->getPalette(2));
			_screen->getPalette(2).copy(_screen->getPalette(0), 128);
		}

		if (_flagsTable[52] & 0x04) {
			uint8 *pal0 = _screen->getPalette(0).getData();
			uint8 *pal2 = _screen->getPalette(2).getData();
			for (int i = 1; i < _screen->getPalette(0).getNumColors() * 3; i++)
				SWAP(pal0[i], pal2[i]);
		}
	}

	memcpy(_vcnBlocks, v, vcnLen);
	v += vcnLen;

	fname = Common::String::format("%s.VMP", _lastBlockDataFile);
	_screen->loadBitmap(fname.c_str(), 3, 3, 0);
	v = _screen->getCPagePtr(2);

	if (vmpLen == -1)
		vmpLen = READ_LE_UINT16(v);
	v += 2;

	delete[] _vmpPtr;
	_vmpPtr = new uint16[vmpLen];

	for (int i = 0; i < vmpLen; i++)
		_vmpPtr[i] = READ_LE_UINT16(&v[i << 1]);

	Palette tpal(256);
	if (_flags.use16ColorMode) {
		uint8 *dst = tpal.getData();
		_res->loadFileToBuf("LOL.NOL", dst, 48);
		for (int i = 1; i < 16; i++) {
			int s = ((i << 4) | i) * 3;
			SWAP(dst[s], dst[i * 3]);
			SWAP(dst[s + 1], dst[i * 3 + 1]);
			SWAP(dst[s + 2], dst[i * 3 + 2]);
		}
	} else {
		tpal.copy(_screen->getPalette(0));
	}

	for (int i = 0; i < 7; i++) {
		weight = 100 - (i * _lastSpecialColorWeight);
		weight = (weight > 0) ? (weight * 255) / 100 : 0;
		_screen->generateOverlay(tpal, _screen->getLevelOverlay(i), _lastSpecialColor, weight);

		int l = _flags.use16ColorMode ? 256 : 128;
		uint8 *levelOverlay = _screen->getLevelOverlay(i);
		for (int ii = 0; ii < l; ii++) {
			if (levelOverlay[ii] == 255)
				levelOverlay[ii] = 0;
		}

		for (int ii = l; ii < 256; ii++)
			levelOverlay[ii] = ii & 0xff;
	}

	uint8 *levelOverlay = _screen->getLevelOverlay(7);
	for (int i = 0; i < 256; i++)
		levelOverlay[i] = i & 0xff;

	if (_flags.use16ColorMode) {
		_screen->getLevelOverlay(6)[0xee] = 0xee;
		if (_lastSpecialColor == 0x44)
			_screen->getLevelOverlay(5)[0xee] = 0xee;

		for (int i = 0; i < 7; i++)
			memcpy(_screen->getLevelOverlay(i), _screen->getLevelOverlay(i + 1), 256);

		_screen->loadPalette("LOL.NOL", _screen->getPalette(0));

		for (int i = 0; i < 8; i++) {
			uint8 *pl = _screen->getLevelOverlay(7 - i);
			for (int ii = 0; ii < 16; ii++)
				_vcnColTable[(i << 4) + ii] = pl[(ii << 4) | ii];
		}
	}

	_loadSuppFilesFlag = 0;
	generateBrightnessPalette(_screen->getPalette(0), _screen->getPalette(1), _brightness, _lampEffect);

	if (_flags.isTalkie) {
		Common::SeekableReadStream *s = _res->createReadStream(Common::String::format("LEVEL%.02d.TLC", _currentLevel));
		s->read(_transparencyTable1, 256);
		s->read(_transparencyTable2, 5120);
		delete s;
	} else {
		createTransparencyTables();
	}

	_loadSuppFilesFlag = 1;
}

void LoLEngine::resetItems(int flag) {
	for (int i = 0; i < 1024; i++) {
		_levelBlockProperties[i].direction = 5;
		uint16 id = _levelBlockProperties[i].assignedObjects;
		LoLObject *r = 0;

		while (id & 0x8000) {
			r = findObject(id);
			id = r->nextAssignedObject;
		}

		if (!id)
			continue;

		LoLItem *it = &_itemsInPlay[id];
		it->level = _currentLevel;
		it->block = i;
		if (r)
			r->nextAssignedObject = 0;
	}

	if (flag)
		memset(_flyingObjects, 0, 8 * sizeof(FlyingObject));
}

void LoLEngine::disableMonsters() {
	memset(_monsters, 0, 30 * sizeof(LoLMonster));
	for (int i = 0; i < 30; i++)
		_monsters[i].mode = 0x10;
}

void LoLEngine::resetBlockProperties() {
	for (int i = 0; i < 1024; i++) {
		LevelBlockProperty *l = &_levelBlockProperties[i];
		if (l->flags & 0x10) {
			l->flags &= 0xef;
			if (testWallInvisibility(i, 0) && testWallInvisibility(i, 1))
				l->flags |= 0x40;
		} else {
			if (l->flags & 0x40)
				l->flags &= 0xbf;
			else if (l->flags & 0x80)
				l->flags &= 0x7f;
		}
	}
}

bool LoLEngine::testWallFlag(int block, int direction, int flag) {
	if (_levelBlockProperties[block].flags & 0x10)
		return true;

	if (direction != -1)
		return (_wllWallFlags[_levelBlockProperties[block].walls[direction ^ 2]] & flag) ? true : false;

	for (int i = 0; i < 4; i++) {
		if (_wllWallFlags[_levelBlockProperties[block].walls[i]] & flag)
			return true;
	}

	return false;
}

bool LoLEngine::testWallInvisibility(int block, int direction) {
	uint8 w = _levelBlockProperties[block].walls[direction];
	if (_wllVmpMap[w] || _wllShapeMap[w] || _levelBlockProperties[block].flags & 0x80)
		return false;
	return true;
}

void LoLEngine::resetLampStatus() {
	_flagsTable[31] |= 0x04;
	_lampEffect = -1;
	updateLampStatus();
}

void LoLEngine::setLampMode(bool lampOn) {
	_flagsTable[31] &= 0xFB;
	if (!(_flagsTable[31] & 0x08) || !lampOn)
		return;

	_screen->drawShape(0, _gameShapes[_flags.isTalkie ? 43 : 41], 291, 56, 0, 0);
	_lampEffect = 8;
}

void LoLEngine::updateLampStatus() {
	int8 newLampEffect = 0;
	uint8 tmpOilStatus = 0;

	if ((_updateFlags & 4) || !(_flagsTable[31] & 0x08))
		return;

	if (!_brightness || !_lampOilStatus) {
		newLampEffect = 8;
		if (newLampEffect != _lampEffect && _screen->_fadeFlag == 0)
			setPaletteBrightness(_screen->getPalette(0), _brightness, newLampEffect);
	} else {
		tmpOilStatus = (_lampOilStatus < 100) ? _lampOilStatus : 100;
		newLampEffect = (3 - ((tmpOilStatus - 1) / 25)) << 1;

		if (_lampEffect == -1) {
			if (_screen->_fadeFlag == 0)
				setPaletteBrightness(_screen->getPalette(0), _brightness, newLampEffect);
			_lampStatusTimer = _system->getMillis() + (10 + rollDice(1, 30)) * _tickLength;
		} else {
			if ((_lampEffect & 0xfe) == (newLampEffect & 0xfe)) {
				if (_system->getMillis() <= _lampStatusTimer) {
					newLampEffect = _lampEffect;
				} else {
					newLampEffect = _lampEffect ^ 1;
					_lampStatusTimer = _system->getMillis() + (10 + rollDice(1, 30)) * _tickLength;
				}
			} else {
				if (_screen->_fadeFlag == 0)
					setPaletteBrightness(_screen->getPalette(0), _brightness, newLampEffect);
			}
		}
	}

	if (newLampEffect == _lampEffect)
		return;

	_screen->hideMouse();

	_screen->drawShape(_screen->_curPage, _gameShapes[(_flags.isTalkie ? 35 : 33) + newLampEffect], 291, 56, 0, 0);
	_screen->showMouse();

	_lampEffect = newLampEffect;
}

void LoLEngine::updateCompass() {
	if (!(_flagsTable[31] & 0x40) || (_updateFlags & 4))
		return;

	if (_compassDirection == -1) {
		_compassStep = 0;
		gui_drawCompass();
		return;
	}

	if (_compassTimer >= _system->getMillis())
		return;

	if ((_currentDirection << 6) == _compassDirection && (!_compassStep))
		return;

	_compassTimer = _system->getMillis() + 3 * _tickLength;
	int dir = _compassStep >= 0 ? 1 : -1;
	if (_compassStep)
		_compassStep -= (((ABS(_compassStep) >> 4) + 2) * dir);

	int16 d = _compassBroken ? (int8(_rnd.getRandomNumber(255)) - _compassDirection) : (_currentDirection << 6) - _compassDirection;
	if (d <= -128)
		d += 256;
	if (d >= 128)
		d -= 256;

	d >>= 2;
	_compassStep += d;
	_compassStep = CLIP(_compassStep, -24, 24);
	_compassDirection += _compassStep;

	if (_compassDirection < 0)
		_compassDirection += 256;
	if (_compassDirection > 255)
		_compassDirection -= 256;

	if ((_compassDirection >> 6) == _currentDirection && _compassStep < 2) {
		int16 d2 = d >> 16;
		d ^= d2;
		d -= d2;
		if (d < 4) {
			_compassDirection = _currentDirection << 6;
			_compassStep = 0;
		}
	}

	gui_drawCompass();
}

void LoLEngine::moveParty(uint16 direction, int unk1, int unk2, int buttonShape) {
	if (buttonShape)
		gui_toggleButtonDisplayMode(buttonShape, 1);

	uint16 opos = _currentBlock;
	uint16 npos = calcNewBlockPosition(_currentBlock, direction);

	if (!checkBlockPassability(npos, direction)) {
		notifyBlockNotPassable(unk2 ? 0 : 1);
		gui_toggleButtonDisplayMode(buttonShape, 0);
		return;
	}

	_scriptDirection = direction;
	_currentBlock = npos;
	_sceneDefaultUpdate = 1;

	calcCoordinates(_partyPosX, _partyPosY, _currentBlock, 0x80, 0x80);
	_flagsTable[73] &= 0xFD;

	runLevelScript(opos, 4);
	runLevelScript(npos, 1);

	if (!(_flagsTable[73] & 0x02)) {
		initTextFading(2, 0);

		if (_sceneDefaultUpdate) {
			switch (unk2) {
			case 0:
				movePartySmoothScrollUp(2);
				break;
			case 1:
				movePartySmoothScrollDown(2);
				break;
			case 2:
				movePartySmoothScrollLeft(1);
				break;
			case 3:
				movePartySmoothScrollRight(1);
				break;
			default:
				break;
			}
		} else {
			gui_drawScene(0);
		}

		gui_toggleButtonDisplayMode(buttonShape, 0);

		if (npos == _currentBlock) {
			runLevelScript(opos, 8);
			runLevelScript(npos, 2);

			if (_levelBlockProperties[npos].walls[0] == 0x1a)
				memset(_levelBlockProperties[npos].walls, 0, 4);
		}
	}

	updateAutoMap(_currentBlock);
}

uint16 LoLEngine::calcBlockIndex(uint16 x, uint16 y) {
	return (((y & 0xff00) >> 3) | (x >> 8)) & 0x3ff;
}

void LoLEngine::calcCoordinates(uint16 &x, uint16 &y, int block, uint16 xOffs, uint16 yOffs) {
	x = (block & 0x1f) << 8 | xOffs;
	y = ((block & 0xffe0) << 3) | yOffs;
}

void LoLEngine::calcCoordinatesForSingleCharacter(int charNum, uint16 &x, uint16 &y) {
	static const uint8 xOffsets[] = {  0x80, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x40, 0x80, 0xC0 };
	int c = countActiveCharacters();
	if (!c)
		return;

	c = (c - 1) * 3 + charNum;

	x = xOffsets[c];
	y = 0x80;

	calcCoordinatesAddDirectionOffset(x, y, _currentDirection);

	x |= (_partyPosX & 0xff00);
	y |= (_partyPosY & 0xff00);
}

void LoLEngine::calcCoordinatesAddDirectionOffset(uint16 &x, uint16 &y, int direction) {
	if (!direction)
		return;

	int tx = x;
	int ty = y;

	if (direction & 1)
		SWAP(tx, ty);

	if (direction != 1)
		ty = (ty - 256) * -1;

	if (direction != 3) {
		tx = (tx - 256) * -1;
	}

	x = tx;
	y = ty;
}

bool LoLEngine::checkBlockPassability(uint16 block, uint16 direction) {
	if (testWallFlag(block, direction, 1))
		return false;

	uint16 d = _levelBlockProperties[block].assignedObjects;

	while (d) {
		if (d & 0x8000)
			return false;
		d = findObject(d)->nextAssignedObject;
	}

	return true;
}

void LoLEngine::notifyBlockNotPassable(int scrollFlag) {
	if (scrollFlag)
		movePartySmoothScrollBlocked(2);

	snd_stopSpeech(true);
	_txt->printMessage(0x8002, "%s", getLangString(0x403f));
	snd_playSoundEffect(19, -1);
}

int LoLEngine::clickedDoorSwitch(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v))
		return 0;

	snd_playSoundEffect(78, -1);
	_blockDoor = 0;
	runLevelScript(block, 0x40);

	if (!_blockDoor) {
		delay(15 * _tickLength);
		processDoorSwitch(block, 0);
	}

	return 1;
}

int LoLEngine::clickedNiche(uint16 block, uint16 direction) {
	uint8 v = _wllShapeMap[_levelBlockProperties[block].walls[direction]];
	if (!clickedShape(v) || !_itemInHand)
		return 0;

	uint16 x = 0x80;
	uint16 y = 0xff;
	calcCoordinatesAddDirectionOffset(x, y, _currentDirection);
	calcCoordinates(x, y, block, x, y);
	setItemPosition(_itemInHand, x, y, 8, 1);
	setHandItem(0);
	return 1;
}

void LoLEngine::movePartySmoothScrollBlocked(int speed) {
	if (!_smoothScrollingEnabled || (_smoothScrollingEnabled && _needSceneRestore))
		return;

	_screen->backupSceneWindow(_sceneDrawPage2 == 2 ? 2 : 6, 6);

	for (int i = 0; i < 2; i++) {
		uint32 delayTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);
		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(delayTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	for (int i = 2; i; i--) {
		uint32 delayTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);
		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(delayTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	if (_sceneDefaultUpdate != 2) {
		_screen->restoreSceneWindow(6, 0);
		_screen->updateScreen();
	}

	updateDrawPage2();
}

void LoLEngine::movePartySmoothScrollUp(int speed) {
	if (!_smoothScrollingEnabled || (_smoothScrollingEnabled && _needSceneRestore))
		return;

	int d = 0;

	if (_sceneDrawPage2 == 2) {
		d = smoothScrollDrawSpecialGuiShape(6);
		gui_drawScene(6);
		_screen->backupSceneWindow(6, 12);
		_screen->backupSceneWindow(2, 6);
	} else {
		d = smoothScrollDrawSpecialGuiShape(2);
		gui_drawScene(2);
		_screen->backupSceneWindow(2, 12);
		_screen->backupSceneWindow(6, 6);
	}

	for (int i = 0; i < 5; i++) {
		uint32 delayTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);

		if (d)
			_screen->copyGuiShapeToSurface(14, 2);

		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(delayTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	if (d)
		_screen->copyGuiShapeToSurface(14, 12);

	if (_sceneDefaultUpdate != 2) {
		_screen->restoreSceneWindow(12, 0);
		_screen->updateScreen();
	}

	updateDrawPage2();
}

void LoLEngine::movePartySmoothScrollDown(int speed) {
	if (!_smoothScrollingEnabled)
		return;

	int d = smoothScrollDrawSpecialGuiShape(2);
	gui_drawScene(2);
	_screen->backupSceneWindow(2, 6);

	for (int i = 4; i >= 0; i--) {
		uint32 delayTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollZoomStepTop(6, 2, _scrollXTop[i], _scrollYTop[i]);
		_screen->smoothScrollZoomStepBottom(6, 2, _scrollXBottom[i], _scrollYBottom[i]);

		if (d)
			_screen->copyGuiShapeToSurface(14, 2);

		_screen->restoreSceneWindow(2, 0);
		_screen->updateScreen();
		fadeText();
		delayUntil(delayTimer);
		if (!_smoothScrollModeNormal)
			i++;
	}

	if (d)
		_screen->copyGuiShapeToSurface(14, 12);

	if (_sceneDefaultUpdate != 2) {
		_screen->restoreSceneWindow(6, 0);
		_screen->updateScreen();
	}

	updateDrawPage2();
}

void LoLEngine::movePartySmoothScrollLeft(int speed) {
	if (!_smoothScrollingEnabled)
		return;

	speed <<= 1;

	gui_drawScene(_sceneDrawPage1);

	for (int i = 88, d = 88; i > 22; i -= 22, d += 22) {
		uint32 delayTimer = _system->getMillis() + speed * _tickLength;
		_screen->smoothScrollHorizontalStep(_sceneDrawPage2, 66, d, i);
		_screen->copyRegion(112 + i, 0, 112, 0, d, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		fadeText();
		delayUntil(delayTimer);
	}

	if (_sceneDefaultUpdate != 2) {
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}

	SWAP(_sceneDrawPage1, _sceneDrawPage2);
}

void LoLEngine::movePartySmoothScrollRight(int speed) {
	if (!_smoothScrollingEnabled)
		return;

	speed <<= 1;

	gui_drawScene(_sceneDrawPage1);

	uint32 delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->copyRegion(112, 0, 222, 0, 66, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollHorizontalStep(_sceneDrawPage2, 22, 0, 66);
	_screen->copyRegion(112, 0, 200, 0, 88, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollHorizontalStep(_sceneDrawPage2, 44, 0, 22);
	_screen->copyRegion(112, 0, 178, 0, 110, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	if (_sceneDefaultUpdate != 2) {
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}

	SWAP(_sceneDrawPage1, _sceneDrawPage2);
}

void LoLEngine::movePartySmoothScrollTurnLeft(int speed) {
	if (!_smoothScrollingEnabled)
		return;

	speed <<= 1;

	int d = smoothScrollDrawSpecialGuiShape(_sceneDrawPage1);
	gui_drawScene(_sceneDrawPage1);
	int dp = _sceneDrawPage2 == 2 ? _sceneDrawPage2 : _sceneDrawPage1;

	uint32 delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep1(_sceneDrawPage1, _sceneDrawPage2, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep2(_sceneDrawPage1, _sceneDrawPage2, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep3(_sceneDrawPage1, _sceneDrawPage2, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	if (_sceneDefaultUpdate != 2) {
		drawSpecialGuiShape(_sceneDrawPage1);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}
}

void LoLEngine::movePartySmoothScrollTurnRight(int speed) {
	if (!_smoothScrollingEnabled)
		return;

	speed <<= 1;

	int d = smoothScrollDrawSpecialGuiShape(_sceneDrawPage1);
	gui_drawScene(_sceneDrawPage1);
	int dp = _sceneDrawPage2 == 2 ? _sceneDrawPage2 : _sceneDrawPage1;

	uint32 delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep3(_sceneDrawPage2, _sceneDrawPage1, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep2(_sceneDrawPage2, _sceneDrawPage1, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	delayTimer = _system->getMillis() + speed * _tickLength;
	_screen->smoothScrollTurnStep1(_sceneDrawPage2, _sceneDrawPage1, dp);
	if (d)
		_screen->copyGuiShapeToSurface(14, dp);
	_screen->restoreSceneWindow(dp, 0);
	_screen->updateScreen();
	fadeText();
	delayUntil(delayTimer);

	if (_sceneDefaultUpdate != 2) {
		drawSpecialGuiShape(_sceneDrawPage1);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}
}

void LoLEngine::pitDropScroll(int numSteps) {
	_screen->copyRegionSpecial(0, 320, 200, 112, 0, 6, 176, 120, 0, 0, 176, 120, 0);
	uint32 etime = 0;

	for (int i = 0; i < numSteps; i++) {
		etime = _system->getMillis() + _tickLength;
		int ys = ((30720 / numSteps) * i) >> 8;
		_screen->copyRegionSpecial(6, 176, 120, 0, ys, 0, 320, 200, 112, 0, 176, 120 - ys, 0);
		_screen->copyRegionSpecial(2, 320, 200, 112, 0, 0, 320, 200, 112, 120 - ys, 176, ys, 0);
		_screen->updateScreen();

		delayUntil(etime);
	}

	etime = _system->getMillis() + _tickLength;

	_screen->copyRegionSpecial(2, 320, 200, 112, 0, 0, 320, 200, 112, 0, 176, 120, 0);
	_screen->updateScreen();

	delayUntil(etime);

	updateDrawPage2();
}

void LoLEngine::shakeScene(int duration, int width, int height, int restore) {
	_screen->copyRegion(112, 0, 112, 0, 176, 120, 0, 6, Screen::CR_NO_P_CHECK);
	uint32 endTime = _system->getMillis() + duration * _tickLength;

	while (endTime > _system->getMillis()) {
		uint32 delayTimer = _system->getMillis() + 2 * _tickLength;

		int s1 = width ? (_rnd.getRandomNumber(255) % (width << 1)) - width : 0;
		int s2 = height ? (_rnd.getRandomNumber(255) % (height << 1)) - height : 0;

		int x1, y1, x2, y2, w, h;
		if (s1 >= 0) {
			x1 = 112;
			x2 = 112 + s1;
			w = 176 - s1;
		} else {
			x1 = 112 - s1;
			x2 = 112;
			w = 176 + s1;
		}

		if (s2 >= 0) {
			y1 = 0;
			y2 = s2;
			h = 120 - s2;
		} else {
			y1 = -s2;
			y2 = 0;
			h = 120 + s2;
		}

		_screen->copyRegion(x1, y1, x2, y2, w, h, 6, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		delayUntil(delayTimer);
	}

	if (restore) {
		_screen->copyRegion(112, 0, 112, 0, 176, 120, 6, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		updateDrawPage2();
	}
}

void LoLEngine::processGasExplosion(int soundId) {
	int cp = _screen->setCurPage(2);
	_screen->copyPage(0, 12);

	static const uint8 sounds[] = { 0x62, 0xA7, 0xA7, 0xA8 };
	snd_playSoundEffect(sounds[soundId], -1);

	uint16 targetBlock = 0;
	int dist = getSpellTargetBlock(_currentBlock, _currentDirection, 3, targetBlock);

	uint8 *p1 = _screen->getPalette(1).getData();
	uint8 *p2 = _screen->getPalette(3).getData();

	if (dist) {
		WSAMovie_v2 *mov = new WSAMovie_v2(this);
		Common::String file = Common::String::format("gasexp%0d.wsa", dist);
		mov->open(file.c_str(), 1, 0);
		if (!mov->opened())
			error("Gas: Unable to load gasexp.wsa");

		playSpellAnimation(mov, 0, 6, 1, (176 - mov->width()) / 2 + 112, (120 - mov->height()) / 2, 0, 0, 0, 0, false);

		mov->close();
		delete mov;

	} else {
		memcpy(p2, p1, 768);

		for (int i = 1; i < 128; i++)
			p2[i * 3] = 0x3f;

		uint32 ctime = _system->getMillis();
		while (_screen->timedPaletteFadeStep(_screen->getPalette(0).getData(), p2, _system->getMillis() - ctime, 10))
			updateInput();

		ctime = _system->getMillis();
		while (_screen->timedPaletteFadeStep(p2, _screen->getPalette(0).getData(), _system->getMillis() - ctime, 50))
			updateInput();
	}

	_screen->copyPage(12, 2);
	_screen->setCurPage(cp);

	updateDrawPage2();
	_sceneUpdateRequired = true;
	gui_drawScene(0);
}

int LoLEngine::smoothScrollDrawSpecialGuiShape(int pageNum) {
	if (!_specialGuiShape)
		return 0;

	_screen->clearGuiShapeMemory(pageNum);
	_screen->drawShape(pageNum, _specialGuiShape, _specialGuiShapeX, _specialGuiShapeY, 2, 0);
	_screen->copyGuiShapeFromSceneBackupBuffer(pageNum, 14);
	return 1;
}

void LoLEngine::drawScene(int pageNum) {
	if (pageNum && pageNum != _sceneDrawPage1) {
		SWAP(_sceneDrawPage1, _sceneDrawPage2);
		updateDrawPage2();
	}

	if (pageNum && pageNum != _sceneDrawPage1) {
		SWAP(_sceneDrawPage1, _sceneDrawPage2);
		updateDrawPage2();
	}

	generateBlockDrawingBuffer();
	drawVcnBlocks();
	drawSceneShapes();

	if (!pageNum) {
		drawSpecialGuiShape(_sceneDrawPage1);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(112, 0, 112, 0, 176, 120, _sceneDrawPage1, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		SWAP(_sceneDrawPage1, _sceneDrawPage2);
	}

	updateEnvironmentalSfx(0);
	gui_drawCompass();

	_sceneUpdateRequired = false;
}


void LoLEngine::setWallType(int block, int wall, int val) {
	if (wall == -1) {
		for (int i = 0; i < 4; i++)
			_levelBlockProperties[block].walls[i] = val;
		if (_wllAutomapData[val] == 17) {
			_levelBlockProperties[block].flags &= 0xef;
			_levelBlockProperties[block].flags |= 0x20;
		} else {
			_levelBlockProperties[block].flags &= 0xdf;
		}
	} else {
		_levelBlockProperties[block].walls[wall] = val;
	}

	checkSceneUpdateNeed(block);
}

void LoLEngine::updateDrawPage2() {
	_screen->copyRegion(112, 0, 112, 0, 176, 120, 0, _sceneDrawPage2, Screen::CR_NO_P_CHECK);
}

void LoLEngine::prepareSpecialScene(int fieldType, int hasDialogue, int suspendGui, int allowSceneUpdate, int controlMode, int fadeFlag) {
	resetPortraitsAndDisableSysTimer();
	if (fieldType) {
		if (suspendGui)
			gui_specialSceneSuspendControls(1);

		if (!allowSceneUpdate)
			_sceneDefaultUpdate = 0;

		if (hasDialogue)
			initDialogueSequence(fieldType, 0);

		if (fadeFlag) {
			if (_flags.use16ColorMode)
				setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
			else
				_screen->fadePalette(_screen->getPalette(3), 10);
			_screen->_fadeFlag = 0;
		}

		setSpecialSceneButtons(0, 0, 320, 130, controlMode);

	} else {
		if (suspendGui)
			gui_specialSceneSuspendControls(0);

		if (!allowSceneUpdate)
			_sceneDefaultUpdate = 0;

		gui_disableControls(controlMode);

		if (fadeFlag) {
			if (_flags.use16ColorMode) {
				setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
			} else {
				_screen->getPalette(3).copy(_screen->getPalette(0), 128);
				_screen->loadSpecialColors(_screen->getPalette(3));
				_screen->fadePalette(_screen->getPalette(3), 10);
			}
			_screen->_fadeFlag = 0;
		}

		if (hasDialogue)
			initDialogueSequence(fieldType, 0);

		setSpecialSceneButtons(112, 0, 176, 120, controlMode);
	}
}

int LoLEngine::restoreAfterSpecialScene(int fadeFlag, int redrawPlayField, int releaseTimScripts, int sceneUpdateMode) {
	if (!_needSceneRestore)
		return 0;

	_needSceneRestore = 0;
	enableSysTimer(2);

	if (_dialogueField)
		restoreAfterDialogueSequence(_currentControlMode);

	if (_specialSceneFlag)
		gui_specialSceneRestoreControls(_currentControlMode);

	int l = _currentControlMode;
	_currentControlMode = 0;

	gui_specialSceneRestoreButtons();
	calcCharPortraitXpos();

	_currentControlMode = l;

	if (releaseTimScripts) {
		for (int i = 0; i < TIM::kWSASlots; i++)
			_tim->freeAnimStruct(i);

		for (int i = 0; i < 10; i++)
			_tim->unload(_activeTim[i]);
	}

	gui_enableControls();

	if (fadeFlag) {
		if ((_screen->_fadeFlag != 1 && _screen->_fadeFlag != 2) || (_screen->_fadeFlag == 1 && _currentControlMode)) {
			if (_currentControlMode)
				_screen->fadeToBlack(10);
			else
				_screen->fadeClearSceneWindow(10);
		}

		_currentControlMode = 0;
		calcCharPortraitXpos();

		if (redrawPlayField)
			gui_drawPlayField();

		setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);

	} else {
		_currentControlMode = 0;
		calcCharPortraitXpos();

		if (redrawPlayField)
			gui_drawPlayField();
	}

	_sceneDefaultUpdate = sceneUpdateMode;
	return 1;
}

void LoLEngine::setSequenceButtons(int x, int y, int w, int h, int enableFlags) {
	gui_enableSequenceButtons(x, y, w, h, enableFlags);
	_seqWindowX1 = x;
	_seqWindowY1 = y;
	_seqWindowX2 = x + w;
	_seqWindowY2 = y + h;
	int offs = _itemInHand ? 10 : 0;
	_screen->setMouseCursor(offs, offs, getItemIconShapePtr(_itemInHand));
	_currentFloatingCursor = -1;
	if (w == 320) {
		setLampMode(0);
		_lampStatusSuspended = true;
	}
}

void LoLEngine::setSpecialSceneButtons(int x, int y, int w, int h, int enableFlags) {
	gui_enableSequenceButtons(x, y, w, h, enableFlags);
	_spsWindowX = x;
	_spsWindowY = y;
	_spsWindowW = w;
	_spsWindowH = h;
}

void LoLEngine::setDefaultButtonState() {
	gui_enableDefaultPlayfieldButtons();
	_seqWindowX1 = _seqWindowY1 = _seqWindowX2 = _seqWindowY2 = _seqTrigger = 0;
	if (_lampStatusSuspended)
		resetLampStatus();
	_lampStatusSuspended = false;
}

void LoLEngine::drawSceneShapes(int) {
	for (int i = 0; i < 18; i++) {
		uint8 t = _dscTileIndex[i];
		uint8 s = _visibleBlocks[t]->walls[_sceneDrawVarDown];

		_shpDmX1 = 0;
		_shpDmX2 = 0;

		int16 dimY1 = 0;
		int16 dimY2 = 0;

		setLevelShapesDim(t, _shpDmX1, _shpDmX2, _sceneShpDim);

		if (_shpDmX2 <= _shpDmX1)
			continue;

		drawDecorations(t);

		uint16 w = _wllWallFlags[s];

		if (t == 16)
			w |= 0x80;

		drawBlockEffects(t, 0);

		if (_visibleBlocks[t]->assignedObjects && (w & 0x80))
			drawBlockObjects(t);

		drawBlockEffects(t, 1);

		if (!(w & 8))
			continue;

		uint16 v = 20 * (s - (s < 23 ? _dscUnk2[s] : 0));
		if (v > 80)
			v = 80;

		setDoorShapeDim(t, dimY1, dimY2, _sceneShpDim);
		drawDoor(_doorShapes[(s < 23 ? _dscDoorShpIndex[s] : 0)], 0, t, 10, 0, -v, 2);
		setLevelShapesDim(t, dimY1, dimY2, _sceneShpDim);
	}
}

void LoLEngine::drawDecorations(int index) {
	for (int i = 1; i >= 0; i--) {
		int s = index * 2 + i;
		uint16 scaleW = _dscShapeScaleW[s];
		uint16 scaleH = _dscShapeScaleH[s];
		int8 ix = _dscShapeIndex[s];
		uint8 shpIx = ABS(ix);
		uint8 ovlIndex = _dscShapeOvlIndex[4 + _dscDimMap[index] * 5] + 2;
		if (ovlIndex > 7)
			ovlIndex = 7;

		if (!scaleW || !scaleH)
			continue;

		uint8 d = (_currentDirection + _dscWalls[s]) & 3;
		int8 l = _wllShapeMap[_visibleBlocks[index]->walls[d]];

		uint8 *shapeData = 0;

		int x = 0;
		int y = 0;
		int flags = 0;

		while (l > 0) {
			if ((_levelDecorationProperties[l].flags & 8) && index != 3 && index != 9 && index != 13) {
				l = _levelDecorationProperties[l].next;
				continue;
			}

			if (_dscOvlMap[shpIx] == 1 && ((_levelDecorationProperties[l].flags & 2) || ((_levelDecorationProperties[l].flags & 4) && _wllProcessFlag)))
				ix = -ix;

			int xOffs = 0;
			int yOffs = 0;
			uint8 *ovl = 0;

			if (_levelDecorationProperties[l].scaleFlag[shpIx] & 1) {
				xOffs = _levelDecorationProperties[l].shapeX[shpIx];
				yOffs = _levelDecorationProperties[l].shapeY[shpIx];
				shpIx = _dscOvlMap[shpIx];
				int ov = ovlIndex;
				if (_flags.use16ColorMode) {
					uint8 bb = _blockBrightness >> 4;
					if (ov > bb)
						ov -= bb;
					else
						ov = 0;
				}
				ovl = _screen->getLevelOverlay(ov);
			} else if (_levelDecorationProperties[l].shapeIndex[shpIx] != 0xffff) {
				scaleW = scaleH = 0x100;
				int ov = 7;
				if (_flags.use16ColorMode) {
					uint8 bb = _blockBrightness >> 4;
					if (ov > bb)
						ov -= bb;
					else
						ov = 0;
				}
				ovl = _screen->getLevelOverlay(ov);
			}

			if (_levelDecorationProperties[l].shapeIndex[shpIx] != 0xffff) {
				shapeData = _levelDecorationShapes[_levelDecorationProperties[l].shapeIndex[shpIx]];
				if (shapeData) {
					if (ix < 0) {
						x = _dscShapeX[s] + xOffs + ((_levelDecorationProperties[l].shapeX[shpIx] * scaleW) >> 8);
						if (ix == _dscShapeIndex[s]) {
							x = _dscShapeX[s] - ((_levelDecorationProperties[l].shapeX[shpIx] * scaleW) >> 8) -
							     _screen->getShapeScaledWidth(shapeData, scaleW) - xOffs;
						}
						flags = 0x105;
					} else {
						x = _dscShapeX[s] + xOffs + ((_levelDecorationProperties[l].shapeX[shpIx] * scaleW) >> 8);
						flags = 0x104;
					}

					y = _dscShapeY[s] + yOffs + ((_levelDecorationProperties[l].shapeY[shpIx] * scaleH) >> 8);
					_screen->drawShape(_sceneDrawPage1, shapeData, x + 112, y, _sceneShpDim, flags, ovl, 1, scaleW, scaleH);

					if ((_levelDecorationProperties[l].flags & 1) && shpIx < 4) {
						//draw shadow
						x += (_screen->getShapeScaledWidth(shapeData, scaleW));
						flags ^= 1;
						_screen->drawShape(_sceneDrawPage1, shapeData, x + 112, y, _sceneShpDim, flags, ovl, 1, scaleW, scaleH);
					}
				}
			}

			l = _levelDecorationProperties[l].next;
			shpIx = (_dscShapeIndex[s] < 0) ? -_dscShapeIndex[s] : _dscShapeIndex[s];
		}
	}
}

void LoLEngine::drawBlockEffects(int index, int type) {
	static const uint16 yOffs[] = { 0xff, 0xff, 0x80, 0x80 };
	uint8 flg = _visibleBlocks[index]->flags;
	// flags: 0x10 = ice wall, 0x20 = teleporter, 0x40 = blue slime spot, 0x80 = blood spot
	if (!(flg & 0xf0))
		return;

	type = (type == 0) ? 2 : 0;

	for (int i = 0; i < 2; i++, type++) {
		if (!((0x10 << type) & flg))
			continue;

		uint16 x = 0x80;
		uint16 y = yOffs[type];
		uint16 drawFlag = (type == 3) ? 0x80 : 0x20;
		uint8 *ovl = (type == 3) ? _screen->_grayOverlay : 0;

		if (_flags.use16ColorMode) {
			ovl = 0;
			drawFlag = (type == 0 || type == 3) ? 0 : 0x20;
		}

		calcCoordinatesAddDirectionOffset(x, y, _currentDirection);

		x |= ((_visibleBlockIndex[index] & 0x1f) << 8);
		y |= ((_visibleBlockIndex[index] & 0xffe0) << 3);

		drawItemOrMonster(_effectShapes[type], ovl, x, y, 0, (type == 1) ? -20 : 0, drawFlag, -1, false);
	}
}

void LoLEngine::drawSpecialGuiShape(int pageNum) {
	if (!_specialGuiShape)
		return;

	_screen->drawShape(pageNum, _specialGuiShape, _specialGuiShapeX, _specialGuiShapeY, 2, 0);

	if (_specialGuiShapeMirrorFlag & 1)
		_screen->drawShape(pageNum, _specialGuiShape, _specialGuiShapeX + _specialGuiShape[3], _specialGuiShapeY, 2, 1);
}

} // End of namespace Kyra

#endif // ENABLE_LOL
