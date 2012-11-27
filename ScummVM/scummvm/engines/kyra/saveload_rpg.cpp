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

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/resource.h"
#include "kyra/script_eob.h"

#include "common/system.h"
#include "common/savefile.h"
#include "common/substream.h"

namespace Kyra {

void KyraRpgEngine::generateTempData() {
	int l = _currentLevel - 1;
	if (_lvlTempData[l]) {
		delete[] _lvlTempData[l]->wallsXorData;
		delete[] _lvlTempData[l]->flags;
		releaseMonsterTempData(_lvlTempData[l]);
		releaseFlyingObjectTempData(_lvlTempData[l]);
		releaseWallOfForceTempData(_lvlTempData[l]);
		delete _lvlTempData[l];
	}

	_lvlTempData[l] = new LevelTempData;

	_lvlTempData[l]->wallsXorData = new uint8[4096];
	_lvlTempData[l]->flags = new uint16[1024];

	const uint8 *p = getBlockFileData(_currentLevel);
	uint16 len = READ_LE_UINT16(p + 4);
	p += 6;

	memset(_lvlTempData[l]->wallsXorData, 0, 4096);
	memset(_lvlTempData[l]->flags, 0, 1024 * sizeof(uint16));
	uint8 *d = _lvlTempData[l]->wallsXorData;
	uint16 *df = _lvlTempData[l]->flags;

	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			*d++ = p[i * len + ii] ^ _levelBlockProperties[i].walls[ii];
		*df++ = _levelBlockProperties[i].flags;
	}

	_lvlTempData[l]->monsters = generateMonsterTempData(_lvlTempData[l]);
	_lvlTempData[l]->flyingObjects = generateFlyingObjectTempData(_lvlTempData[l]);
	_lvlTempData[l]->wallsOfForce = generateWallOfForceTempData(_lvlTempData[l]);

	_hasTempDataFlags |= (1 << l);
}

void KyraRpgEngine::restoreBlockTempData(int levelIndex) {
	int l = levelIndex - 1;
	const uint8 *p = getBlockFileData(levelIndex);
	uint16 len = READ_LE_UINT16(p + 4);
	p += 6;

	memset(_levelBlockProperties, 0, 1024 * sizeof(LevelBlockProperty));

	uint8 *t = _lvlTempData[l]->wallsXorData;
	uint16 *t2 = _lvlTempData[l]->flags;

	for (int i = 0; i < 1024; i++) {
		for (int ii = 0; ii < 4; ii++)
			_levelBlockProperties[i].walls[ii] = p[i * len + ii] ^ *t++;
		_levelBlockProperties[i].flags = *t2++;
	}

	restoreMonsterTempData(_lvlTempData[l]);
	restoreFlyingObjectTempData(_lvlTempData[l]);
	restoreWallOfForceTempData(_lvlTempData[l]);
}

void KyraRpgEngine::releaseTempData() {
	for (int i = 0; i < 29; i++) {
		if (_lvlTempData[i]) {
			delete[] _lvlTempData[i]->wallsXorData;
			delete[] _lvlTempData[i]->flags;
			releaseMonsterTempData(_lvlTempData[i]);
			releaseFlyingObjectTempData(_lvlTempData[i]);
			releaseWallOfForceTempData(_lvlTempData[i]);
			delete _lvlTempData[i];
			_lvlTempData[i] = 0;
		}
	}
}

void *KyraRpgEngine::generateFlyingObjectTempData(LevelTempData *tmp) {
	assert(_flyingObjectStructSize == sizeof(EoBFlyingObject));
	EoBFlyingObject *f = new EoBFlyingObject[_numFlyingObjects];
	memcpy(f, _flyingObjectsPtr,  sizeof(EoBFlyingObject) * _numFlyingObjects);
	return f;
}

void KyraRpgEngine::restoreFlyingObjectTempData(LevelTempData *tmp) {
	assert(_flyingObjectStructSize == sizeof(EoBFlyingObject));
	memcpy(_flyingObjectsPtr, tmp->flyingObjects, sizeof(EoBFlyingObject) * _numFlyingObjects);
}

void KyraRpgEngine::releaseFlyingObjectTempData(LevelTempData *tmp) {
	EoBFlyingObject *p = (EoBFlyingObject *)tmp->flyingObjects;
	delete[] p;
}

} // End of namespace Kyra

#endif // ENABLE_EOB || ENABLE_LOL
