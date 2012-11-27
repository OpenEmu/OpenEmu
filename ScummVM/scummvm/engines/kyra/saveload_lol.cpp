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

#include "common/savefile.h"
#include "common/substream.h"
#include "common/memstream.h"

#include "graphics/scaler.h"

namespace Kyra {

Common::Error LoLEngine::loadGameState(int slot) {
	const uint16 *cdf[] = { _charDefsMan, _charDefsWoman, _charDefsKieran, _charDefsMan, _charDefsAkshel };

	const char *fileName = getSavegameFilename(slot);

	SaveHeader header;
	Common::InSaveFile *saveFile = openSaveForReading(fileName, header);
	if (!saveFile) {
		_txt->printMessage(2, "%s", getLangString(0x425d));
		return Common::kNoError;
	}

	if (header.originalSave)
		warning("Trying to load savegame from original interpreter, while this is possible, it is not officially supported");

	_screen->fadeClearSceneWindow(10);
	completeDoorOperations();
	_screen->fillRect(112, 0, 287, 119, 0, 0);
	_screen->updateScreen();

	Common::SeekableSubReadStreamEndian in(saveFile, saveFile->pos(), saveFile->size(), !header.originalSave, DisposeAfterUse::YES);

	for (int i = 0; i < 4; i++) {
		LoLCharacter *c = &_characters[i];
		c->flags = in.readUint16();
		in.read(c->name, 11);
		c->raceClassSex = in.readByte();
		c->id = in.readSint16();
		c->curFaceFrame = in.readByte();
		c->tempFaceFrame = in.readByte();
		c->screamSfx = in.readByte();
		if (header.originalSave)
			in.skip(4);
		for (int ii = 0; ii < 8; ii++)
			c->itemsMight[ii] = in.readUint16();
		for (int ii = 0; ii < 8; ii++)
			c->protectionAgainstItems[ii] = in.readUint16();
		c->itemProtection = in.readUint16();
		c->hitPointsCur = in.readSint16();
		c->hitPointsMax = in.readUint16();
		c->magicPointsCur = in.readSint16();
		c->magicPointsMax = in.readUint16();
		c->field_41 = in.readByte();
		c->damageSuffered = in.readUint16();
		c->weaponHit = in.readUint16();
		c->totalMightModifier = in.readUint16();
		c->totalProtectionModifier = in.readUint16();
		c->might = in.readUint16();
		c->protection = in.readUint16();
		c->nextAnimUpdateCountdown = in.readSint16();
		for (int ii = 0; ii < 11; ii++)
			c->items[ii] = in.readUint16();
		for (int ii = 0; ii < 3; ii++)
			c->skillLevels[ii] = in.readByte();
		for (int ii = 0; ii < 3; ii++)
			c->skillModifiers[ii] = in.readSByte();
		for (int ii = 0; ii < 3; ii++)
			c->experiencePts[ii] = in.readUint32();
		for (int ii = 0; ii < 5; ii++)
			c->characterUpdateEvents[ii] = in.readByte();
		for (int ii = 0; ii < 5; ii++)
			c->characterUpdateDelay[ii] = in.readByte();

		if (c->flags & 1) {
			loadCharFaceShapes(i, c->id);
			c->defaultModifiers = cdf[c->raceClassSex];
		}
	}

	if (header.version < 17)
		in.skip(80);

	_currentBlock = in.readUint16();
	_partyPosX = in.readUint16();
	_partyPosY = in.readUint16();
	_updateFlags = in.readUint16();
	_scriptDirection = in.readByte();
	_selectedSpell = in.readByte();

	if (header.originalSave)
		in.skip(2);

	_sceneDefaultUpdate = in.readByte();
	_compassBroken = in.readByte();
	_drainMagic = in.readByte();
	_currentDirection = in.readUint16();
	_compassDirection = in.readUint16();
	_selectedCharacter = in.readSByte();

	if (header.originalSave)
		in.skip(1);

	_currentLevel = in.readByte();
	for (int i = 0; i < 48; i++)
		_inventory[i] = in.readSint16();
	_inventoryCurItem = in.readSint16();
	_itemInHand = in.readSint16();
	_lastMouseRegion = in.readSint16();

	if (header.originalSave || header.version <= 15) {
		uint16 flags[40];
		memset(flags, 0, sizeof(flags));

		if (header.version == 14) {
			for (int i = 0; i < 16; i++)
				flags[i] = in.readUint16();
			flags[26] = in.readUint16();
			flags[36] = in.readUint16();
		} else if (header.originalSave || header.version == 15) {
			for (int i = 0; i < 40; i++)
				flags[i] = in.readUint16();
		}

		memset(_flagsTable, 0, sizeof(_flagsTable));
		for (uint i = 0; i < ARRAYSIZE(flags); ++i) {
			for (uint k = 0; k < 16; ++k) {
				if (flags[i] & (1 << k))
					setGameFlag(((i << 4) & 0xFFF0) | (k & 0x000F));
			}
		}
	} else {
		uint32 flagsSize = in.readUint32();
		assert(flagsSize <= sizeof(_flagsTable));
		in.read(_flagsTable, flagsSize);
	}

	if (header.originalSave)
		in.skip(120);

	for (int i = 0; i < 24; i++)
		_globalScriptVars[i] = in.readUint16();

	if (header.originalSave)
		in.skip(152);

	_brightness = in.readByte();
	_lampOilStatus = in.readByte();
	_lampEffect = in.readSByte();

	if (header.originalSave)
		in.skip(1);

	_credits = in.readUint16();
	for (int i = 0; i < 8; i++)
		_globalScriptVars2[i] = in.readUint16();
	in.read(_availableSpells, 7);
	_hasTempDataFlags = in.readUint32();

	uint8 *origCmp = 0;
	if (header.originalSave) {
		in.skip(6);
		origCmp = new uint8[2496];
	}

	for (int i = 0; i < 400; i++) {
		LoLItem *t = &_itemsInPlay[i];
		t->nextAssignedObject = in.readUint16();
		t->nextDrawObject = in.readUint16();
		t->flyingHeight = in.readByte();
		t->block = in.readUint16();
		t->x = in.readUint16();
		t->y = in.readUint16();
		t->level = in.readSByte();
		t->itemPropertyIndex = in.readUint16();
		t->shpCurFrame_flg = in.readUint16();
		if (header.version < 17)
			in.skip(4);
	}

	for (int i = 0; i < 1024; i++) {
		LevelBlockProperty *l = &_levelBlockProperties[i];
		l->assignedObjects = l->drawObjects = 0;
		l->direction = 5;
	}

	for (int i = 0; i < 29; i++) {
		if (!(_hasTempDataFlags & (1 << i))) {
			if (header.originalSave) {
				if (in.size() - in.pos() >= 2500)
					in.skip(2500);
			}
			continue;
		}

		if (_lvlTempData[i]) {
			delete[] _lvlTempData[i]->wallsXorData;
			delete[] _lvlTempData[i]->flags;
			releaseMonsterTempData(_lvlTempData[i]);
			releaseFlyingObjectTempData(_lvlTempData[i]);
			releaseWallOfForceTempData(_lvlTempData[i]);
			delete _lvlTempData[i];
		}

		_lvlTempData[i] = new LevelTempData;
		_lvlTempData[i]->wallsXorData = new uint8[4096];
		_lvlTempData[i]->flags = new uint16[1024];
		LoLMonster *lm = new LoLMonster[30];
		_lvlTempData[i]->monsters = lm;
		FlyingObject *lf = new FlyingObject[_numFlyingObjects];
		_lvlTempData[i]->flyingObjects = lf;
		LevelTempData *l = _lvlTempData[i];

		uint32 next = in.pos() + 2500;

		if (header.originalSave) {
			in.skip(4);
			in.read(origCmp, in.readUint16());
			_screen->decodeFrame4(origCmp, _tempBuffer5120, 5120);
			memcpy(l->wallsXorData, _tempBuffer5120, 4096);
			for (int ii = 0; ii < 1024; ii++)
				l->flags[ii] = _tempBuffer5120[4096 + ii];
		} else {
			in.read(l->wallsXorData, 4096);
			for (int ii = 0; ii < 1024; ii++)
				l->flags[ii] = in.readByte();
		}

		if (header.originalSave)
			l->monsterDifficulty =  in.readUint16();

		for (int ii = 0; ii < 30; ii++) {
			LoLMonster *m = &lm[ii];
			m->nextAssignedObject = in.readUint16();
			m->nextDrawObject = in.readUint16();
			m->flyingHeight = in.readByte();
			m->block = in.readUint16();
			m->x = in.readUint16();
			m->y = in.readUint16();
			m->shiftStep = in.readSByte();
			m->destX = in.readUint16();
			m->destY = in.readUint16();
			m->destDirection = in.readByte();
			m->hitOffsX = in.readSByte();
			m->hitOffsY = in.readSByte();
			m->currentSubFrame = in.readByte();
			m->mode = in.readByte();
			m->fightCurTick = in.readSByte();
			m->id = in.readByte();
			m->direction = in.readByte();
			m->facing = in.readByte();
			m->flags = in.readUint16();
			m->damageReceived = in.readUint16();
			m->hitPoints = in.readSint16();
			m->speedTick = in.readByte();
			m->type = in.readByte();

			if (header.originalSave)
				in.skip(4);

			m->numDistAttacks = in.readByte();
			m->curDistWeapon = in.readByte();
			m->distAttackTick = in.readSByte();
			m->assignedItems = in.readUint16();
			m->properties = &_monsterProperties[m->type];
			in.read(m->equipmentShapes, 4);
		}

		for (int ii = 0; ii < _numFlyingObjects; ii++) {
			FlyingObject *m = &lf[ii];
			m->enable = in.readByte();
			m->objectType = in.readByte();
			m->attackerId = in.readUint16();
			m->item = in.readSint16();
			m->x = in.readUint16();
			m->y = in.readUint16();
			m->flyingHeight = in.readByte();
			m->direction = in.readByte();
			m->distance = in.readByte();
			m->field_D = in.readSByte();
			m->c = in.readByte();
			m->flags = in.readByte();
			m->wallFlags = in.readByte();
		}

		if (header.originalSave)
			in.seek(next, SEEK_SET);
		else
			l->monsterDifficulty =  in.readByte();
	}

	delete[] origCmp;

	calcCharPortraitXpos();
	memset(_moneyColumnHeight, 0, sizeof(_moneyColumnHeight));
	int t = _credits;
	_credits = 0;
	giveCredits(t, 0);
	setHandItem(_itemInHand);
	loadLevel(_currentLevel);
	gui_drawPlayField();
	timerSpecialCharacterUpdate(0);
	_flagsTable[73] |= 0x08;

	while (!_screen->isMouseVisible())
		_screen->showMouse();

	return Common::kNoError;
}

Common::Error LoLEngine::saveGameStateIntern(int slot, const char *saveName, const Graphics::Surface *thumbnail) {
	const char *fileName = getSavegameFilename(slot);

	Common::OutSaveFile *out = openSaveForWriting(fileName, saveName, thumbnail);
	if (!out)
		return _saveFileMan->getError();

	completeDoorOperations();
	generateTempData();

	for (int i = 0; i < 4; i++) {
		LoLCharacter *c = &_characters[i];
		out->writeUint16BE(c->flags);
		out->write(c->name, 11);
		out->writeByte(c->raceClassSex);
		out->writeSint16BE(c->id);
		out->writeByte(c->curFaceFrame);
		out->writeByte(c->tempFaceFrame);
		out->writeByte(c->screamSfx);
		for (int ii = 0; ii < 8; ii++)
			out->writeUint16BE(c->itemsMight[ii]);
		for (int ii = 0; ii < 8; ii++)
			out->writeUint16BE(c->protectionAgainstItems[ii]);
		out->writeUint16BE(c->itemProtection);
		out->writeSint16BE(c->hitPointsCur);
		out->writeUint16BE(c->hitPointsMax);
		out->writeSint16BE(c->magicPointsCur);
		out->writeUint16BE(c->magicPointsMax);
		out->writeByte(c->field_41);
		out->writeUint16BE(c->damageSuffered);
		out->writeUint16BE(c->weaponHit);
		out->writeUint16BE(c->totalMightModifier);
		out->writeUint16BE(c->totalProtectionModifier);
		out->writeUint16BE(c->might);
		out->writeUint16BE(c->protection);
		out->writeSint16BE(c->nextAnimUpdateCountdown);
		for (int ii = 0; ii < 11; ii++)
			out->writeUint16BE(c->items[ii]);
		for (int ii = 0; ii < 3; ii++)
			out->writeByte(c->skillLevels[ii]);
		for (int ii = 0; ii < 3; ii++)
			out->writeSByte(c->skillModifiers[ii]);
		for (int ii = 0; ii < 3; ii++)
			out->writeUint32BE(c->experiencePts[ii]);
		for (int ii = 0; ii < 5; ii++)
			out->writeByte(c->characterUpdateEvents[ii]);
		for (int ii = 0; ii < 5; ii++)
			out->writeByte(c->characterUpdateDelay[ii]);
	}

	out->writeUint16BE(_currentBlock);
	out->writeUint16BE(_partyPosX);
	out->writeUint16BE(_partyPosY);
	out->writeUint16BE(_updateFlags);
	out->writeByte(_scriptDirection);
	out->writeByte(_selectedSpell);
	out->writeByte(_sceneDefaultUpdate);
	out->writeByte(_compassBroken);
	out->writeByte(_drainMagic);
	out->writeUint16BE(_currentDirection);
	out->writeUint16BE(_compassDirection);
	out->writeSByte(_selectedCharacter);
	out->writeByte(_currentLevel);
	for (int i = 0; i < 48; i++)
		out->writeSint16BE(_inventory[i]);
	out->writeSint16BE(_inventoryCurItem);
	out->writeSint16BE(_itemInHand);
	out->writeSint16BE(_lastMouseRegion);
	out->writeUint32BE(ARRAYSIZE(_flagsTable));
	out->write(_flagsTable, ARRAYSIZE(_flagsTable));
	for (int i = 0; i < 24; i++)
		out->writeUint16BE(_globalScriptVars[i]);
	out->writeByte(_brightness);
	out->writeByte(_lampOilStatus);
	out->writeSByte(_lampEffect);
	out->writeUint16BE(_credits);
	for (int i = 0; i < 8; i++)
		out->writeUint16BE(_globalScriptVars2[i]);
	out->write(_availableSpells, 7);
	out->writeUint32BE(_hasTempDataFlags);

	resetItems(0);

	for (int i = 0; i < 400; i++) {
		LoLItem *t = &_itemsInPlay[i];
		out->writeUint16BE(t->nextAssignedObject);
		out->writeUint16BE(t->nextDrawObject);
		out->writeByte(t->flyingHeight);
		out->writeUint16BE(t->block);
		out->writeUint16BE(t->x);
		out->writeUint16BE(t->y);
		out->writeSByte(t->level);
		out->writeUint16BE(t->itemPropertyIndex);
		out->writeUint16BE(t->shpCurFrame_flg);
	}

	addLevelItems();

	for (int i = 0; i < 29; i++) {
		LevelTempData *l = _lvlTempData[i];
		if (!l || !(_hasTempDataFlags & (1 << i)))
			continue;

		out->write(l->wallsXorData, 4096);
		for (int ii = 0; ii < 1024; ii++)
			out->writeByte(l->flags[ii] & 0xff);

		LoLMonster *lm = (LoLMonster *)_lvlTempData[i]->monsters;
		FlyingObject *lf = (FlyingObject *)_lvlTempData[i]->flyingObjects;

		for (int ii = 0; ii < 30; ii++) {
			LoLMonster *m = &lm[ii];
			out->writeUint16BE(m->nextAssignedObject);
			out->writeUint16BE(m->nextDrawObject);
			out->writeByte(m->flyingHeight);
			out->writeUint16BE(m->block);
			out->writeUint16BE(m->x);
			out->writeUint16BE(m->y);
			out->writeSByte(m->shiftStep);
			out->writeUint16BE(m->destX);
			out->writeUint16BE(m->destY);
			out->writeByte(m->destDirection);
			out->writeSByte(m->hitOffsX);
			out->writeSByte(m->hitOffsY);
			out->writeByte(m->currentSubFrame);
			out->writeByte(m->mode);
			out->writeSByte(m->fightCurTick);
			out->writeByte(m->id);
			out->writeByte(m->direction);
			out->writeByte(m->facing);
			out->writeUint16BE(m->flags);
			out->writeUint16BE(m->damageReceived);
			out->writeSint16BE(m->hitPoints);
			out->writeByte(m->speedTick);
			out->writeByte(m->type);
			out->writeByte(m->numDistAttacks);
			out->writeByte(m->curDistWeapon);
			out->writeSByte(m->distAttackTick);
			out->writeUint16BE(m->assignedItems);
			out->write(m->equipmentShapes, 4);
		}

		for (int ii = 0; ii < _numFlyingObjects; ii++) {
			FlyingObject *m = &lf[ii];
			out->writeByte(m->enable);
			out->writeByte(m->objectType);
			out->writeUint16BE(m->attackerId);
			out->writeSint16BE(m->item);
			out->writeUint16BE(m->x);
			out->writeUint16BE(m->y);
			out->writeByte(m->flyingHeight);
			out->writeByte(m->direction);
			out->writeByte(m->distance);
			out->writeSByte(m->field_D);
			out->writeByte(m->c);
			out->writeByte(m->flags);
			out->writeByte(m->wallFlags);
		}
		out->writeByte(l->monsterDifficulty);
	}

	out->finalize();

	// check for errors
	if (out->err()) {
		warning("Can't write file '%s'. (Disk full?)", fileName);
		return Common::kUnknownError;
	} else {
		debugC(1, kDebugLevelMain, "Saved game '%s.'", saveName);
	}

	delete out;
	return Common::kNoError;
}

Graphics::Surface *LoLEngine::generateSaveThumbnail() const {
	if (_flags.platform != Common::kPlatformPC98)
		return 0;

	uint8 *screenPal = new uint8[16 * 3];
	assert(screenPal);
	_screen->getRealPalette(0, screenPal);

	uint8 *screenBuf = new uint8[Screen::SCREEN_W * Screen::SCREEN_H];
	assert(screenBuf);

	Graphics::Surface *dst = new Graphics::Surface();
	assert(dst);

	_screen->copyRegionToBuffer(0, 0, 0, 320, 200, screenBuf);
	Screen_LoL::convertPC98Gfx(screenBuf, Screen::SCREEN_W, Screen::SCREEN_H, Screen::SCREEN_W);
	::createThumbnail(dst, screenBuf, Screen::SCREEN_W, Screen::SCREEN_H, screenPal);

	delete[] screenBuf;
	delete[] screenPal;
	return dst;
}

void LoLEngine::restoreBlockTempData(int levelIndex) {
	memset(_tempBuffer5120, 0, 5120);
	KyraRpgEngine::restoreBlockTempData(levelIndex);
	restoreTempDataAdjustMonsterStrength(levelIndex - 1);
}

void *LoLEngine::generateMonsterTempData(LevelTempData *tmp) {
	LoLMonster *m = new LoLMonster[30];
	memcpy(m, _monsters,  sizeof(LoLMonster) * 30);
	tmp->monsterDifficulty = _monsterDifficulty;
	return m;
}

void LoLEngine::restoreTempDataAdjustMonsterStrength(int index) {
	if (_lvlTempData[index]->monsterDifficulty == _monsterDifficulty)
		return;

	uint16 d = (_monsterModifiers[_lvlTempData[index]->monsterDifficulty] << 8) / _monsterModifiers[_monsterDifficulty];

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].mode >= 14 || _monsters[i].block == 0 || _monsters[i].hitPoints <= 0)
			continue;

		_monsters[i].hitPoints = (d * _monsters[i].hitPoints) >> 8;
		if (_monsterDifficulty < _lvlTempData[index]->monsterDifficulty)
			_monsters[i].hitPoints++;
		if (_monsters[i].hitPoints == 0)
			_monsters[i].hitPoints = 1;
	}
}

void LoLEngine::restoreMonsterTempData(LevelTempData *tmp) {
	memcpy(_monsters, tmp->monsters, sizeof(LoLMonster) * 30);

	for (int i = 0; i < 30; i++) {
		if (_monsters[i].block) {
			_monsters[i].block = 0;
			_monsters[i].properties = &_monsterProperties[_monsters[i].type];
			placeMonster(&_monsters[i], _monsters[i].x, _monsters[i].y);
		}
	}
}

void LoLEngine::releaseMonsterTempData(LevelTempData *tmp) {
	LoLMonster *p = (LoLMonster *)tmp->monsters;
	delete[] p;
}

} // End of namespace Kyra

#endif // ENABLE_LOL
