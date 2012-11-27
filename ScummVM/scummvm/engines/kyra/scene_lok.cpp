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

#include "kyra/kyra_lok.h"
#include "kyra/resource.h"
#include "kyra/sound.h"
#include "kyra/sprites.h"
#include "kyra/animator_lok.h"
#include "kyra/timer.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_LoK::enterNewScene(int sceneId, int facing, int unk1, int unk2, int brandonAlive) {
	int unkVar1 = 1;
	_screen->hideMouse();

	// TODO: Check how the original handled sfx still playing
	_sound->stopAllSoundEffects();

	if (_flags.platform == Common::kPlatformFMTowns) {
		int newSfxFile = -1;
		if (_currentCharacter->sceneId == 7 && sceneId == 24)
			newSfxFile = 2;
		else if (_currentCharacter->sceneId == 25 && sceneId == 109)
			newSfxFile = 3;
		else if (_currentCharacter->sceneId == 120 && sceneId == 37)
			newSfxFile = 4;
		else if (_currentCharacter->sceneId == 52 && sceneId == 199)
			newSfxFile = 5;
		else if (_currentCharacter->sceneId == 37 && sceneId == 120)
			newSfxFile = 3;
		else if (_currentCharacter->sceneId == 109 && sceneId == 25)
			newSfxFile = 2;
		else if (_currentCharacter->sceneId == 24 && sceneId == 7)
			newSfxFile = 1;

		if (newSfxFile != -1) {
			_curSfxFile = newSfxFile;
			_sound->loadSoundFile(_curSfxFile);
		}
	}

	switch (_currentCharacter->sceneId) {
	case 1:
		if (sceneId == 0) {
			moveCharacterToPos(0, 0, _currentCharacter->x1, 84);
			unkVar1 = 0;
		}
		break;

	case 3:
		if (sceneId == 2) {
			moveCharacterToPos(0, 6, 155, _currentCharacter->y1);
			unkVar1 = 0;
		}
		break;

	case 26:
		if (sceneId == 27) {
			moveCharacterToPos(0, 6, 155, _currentCharacter->y1);
			unkVar1 = 0;
		}
		break;

	case 44:
		if (sceneId == 45) {
			moveCharacterToPos(0, 2, 192, _currentCharacter->y1);
			unkVar1 = 0;
		}
		break;

	default:
		break;
	}

	if (unkVar1 && unk1) {
		int xpos = _currentCharacter->x1;
		int ypos = _currentCharacter->y1;
		switch (facing) {
		case 0:
			ypos = _currentCharacter->y1 - 6;
			break;

		case 2:
			xpos = 336;
			break;

		case 4:
			ypos = 143;
			break;

		case 6:
			xpos = -16;
			break;

		default:
			break;
		}

		moveCharacterToPos(0, facing, xpos, ypos);
	}

	for (int i = 0; i < ARRAYSIZE(_movieObjects); ++i)
		_movieObjects[i]->close();

	if (!brandonAlive) {
		_emc->init(&_scriptClick, &_scriptClickData);
		_emc->start(&_scriptClick, 5);
		while (_emc->isValid(&_scriptClick))
			_emc->run(&_scriptClick);
	}

	memset(_entranceMouseCursorTracks, 0xFF, sizeof(_entranceMouseCursorTracks));
	_currentCharacter->sceneId = sceneId;

	assert(sceneId < _roomTableSize);
	assert(_roomTable[sceneId].nameIndex < _roomFilenameTableSize);

	Room *currentRoom = &_roomTable[sceneId];

	setupSceneResource(sceneId);

	_currentRoom = sceneId;

	int tableId = _roomTable[sceneId].nameIndex;
	char fileNameBuffer[32];
	strcpy(fileNameBuffer, _roomFilenameTable[tableId]);
	strcat(fileNameBuffer, ".DAT");
	_sprites->loadDat(fileNameBuffer, _sceneExits);
	_sprites->setupSceneAnims();
	_emc->unload(&_scriptClickData);
	loadSceneMsc();

	_walkBlockNorth = currentRoom->northExit;
	_walkBlockEast = currentRoom->eastExit;
	_walkBlockSouth = currentRoom->southExit;
	_walkBlockWest = currentRoom->westExit;

	if (_walkBlockNorth == 0xFFFF)
		_screen->blockOutRegion(0, 0, 320, (_northExitHeight & 0xFF) + 3);
	if (_walkBlockEast == 0xFFFF)
		_screen->blockOutRegion(312, 0, 8, 139);
	if (_walkBlockSouth == 0xFFFF)
		_screen->blockOutRegion(0, 135, 320, 8);
	if (_walkBlockWest == 0xFFFF)
		_screen->blockOutRegion(0, 0, 8, 139);

	if (!brandonAlive)
		updatePlayerItemsForScene();

	startSceneScript(brandonAlive);
	setupSceneItems();

	initSceneData(facing, unk2, brandonAlive);

	_loopFlag2 = 0;
	_screen->showMouse();
	if (!brandonAlive)
		seq_poisonDeathNow(0);
	updateMousePointer(true);
	_changedScene = true;
}

void KyraEngine_LoK::transcendScenes(int roomIndex, int roomName) {
	assert(roomIndex < _roomTableSize);

	if (_flags.isTalkie) {
		char file[32];
		assert(roomIndex < _roomTableSize);
		int tableId = _roomTable[roomIndex].nameIndex;
		assert(tableId < _roomFilenameTableSize);
		strcpy(file, _roomFilenameTable[tableId]);
		strcat(file, ".VRM");
		_res->unloadPakFile(file);
	}

	_roomTable[roomIndex].nameIndex = roomName;
	_unkScreenVar2 = 1;
	_unkScreenVar3 = 1;
	_unkScreenVar1 = 0;
	_brandonPosX = _currentCharacter->x1;
	_brandonPosY = _currentCharacter->y1;
	enterNewScene(roomIndex, _currentCharacter->facing, 0, 0, 0);
	_unkScreenVar1 = 1;
	_unkScreenVar2 = 0;
	_unkScreenVar3 = 0;
}

void KyraEngine_LoK::setSceneFile(int roomIndex, int roomName) {
	assert(roomIndex < _roomTableSize);
	_roomTable[roomIndex].nameIndex = roomName;
}

void KyraEngine_LoK::moveCharacterToPos(int character, int facing, int xpos, int ypos) {
	Character *ch = &_characterList[character];
	ch->facing = facing;
	_screen->hideMouse();
	xpos = (int16)(xpos & 0xFFFC);
	ypos = (int16)(ypos & 0xFFFE);
	_timer->disable(19);
	_timer->disable(14);
	_timer->disable(18);
	uint32 nextFrame = 0;

	switch (facing) {
	case 0:
		while (ypos < ch->y1) {
			nextFrame = _timer->getDelay(5 + character) * _tickLength + _system->getMillis();
			setCharacterPositionWithUpdate(character);
			delayUntil(nextFrame, true);
		}
		break;

	case 2:
		while (ch->x1 < xpos) {
			nextFrame = _timer->getDelay(5 + character) * _tickLength + _system->getMillis();
			setCharacterPositionWithUpdate(character);
			delayUntil(nextFrame, true);
		}
		break;

	case 4:
		while (ypos > ch->y1) {
			nextFrame = _timer->getDelay(5 + character) * _tickLength + _system->getMillis();
			setCharacterPositionWithUpdate(character);
			delayUntil(nextFrame, true);
		}
		break;

	case 6:
		while (ch->x1 > xpos) {
			nextFrame = _timer->getDelay(5 + character) * _tickLength + _system->getMillis();
			setCharacterPositionWithUpdate(character);
			delayUntil(nextFrame, true);
		}
		break;

	default:
		break;
	}

	_timer->enable(19);
	_timer->enable(14);
	_timer->enable(18);
	_screen->showMouse();
}

void KyraEngine_LoK::setCharacterPositionWithUpdate(int character) {
	setCharacterPosition(character, 0);
	_sprites->updateSceneAnims();
	_timer->update();
	_animator->updateAllObjectShapes();
	updateTextFade();

	if (_currentCharacter->sceneId == 210)
		updateKyragemFading();
}

int KyraEngine_LoK::setCharacterPosition(int character, int *facingTable) {
	if (character == 0) {
		_currentCharacter->x1 += _charAddXPosTable[_currentCharacter->facing];
		_currentCharacter->y1 += _charAddYPosTable[_currentCharacter->facing];
		setCharacterPositionHelper(0, facingTable);
		return 1;
	} else {
		_characterList[character].x1 += _charAddXPosTable[_characterList[character].facing];
		_characterList[character].y1 += _charAddYPosTable[_characterList[character].facing];
		if (_characterList[character].sceneId == _currentCharacter->sceneId)
			setCharacterPositionHelper(character, 0);
	}
	return 0;
}

void KyraEngine_LoK::setCharacterPositionHelper(int character, int *facingTable) {
	Character *ch = &_characterList[character];
	++ch->currentAnimFrame;
	int facing = ch->facing;
	if (facingTable) {
		if (*facingTable != *(facingTable - 1)) {
			if (*(facingTable - 1) == *(facingTable + 1)) {
				facing = getOppositeFacingDirection(*(facingTable - 1));
				*facingTable = *(facingTable - 1);
			}
		}
	}

	if (facing == 0) {
		++_characterFacingZeroCount[character];
	} else {
		bool resetTables = false;
		if (facing != 7) {
			if (facing - 1 != 0) {
				if (facing != 4) {
					if (facing == 3 || facing == 5) {
						if (_characterFacingFourCount[character] > 2)
							facing = 4;
						resetTables = true;
					}
				} else {
					++_characterFacingFourCount[character];
				}
			} else {
				if (_characterFacingZeroCount[character] > 2)
					facing = 0;
				resetTables = true;
			}
		} else {
			if (_characterFacingZeroCount[character] > 2)
				facing = 0;
			resetTables = true;
		}

		if (resetTables) {
			_characterFacingZeroCount[character] = 0;
			_characterFacingFourCount[character] = 0;
		}
	}

	static const uint16 maxAnimationFrame[] = {
		0x000F, 0x0031, 0x0055, 0x0000, 0x0000, 0x0000,
		0x0008, 0x002A, 0x004E, 0x0000, 0x0000, 0x0000,
		0x0022, 0x0046, 0x006A, 0x0000, 0x0000, 0x0000,
		0x001D, 0x0041, 0x0065, 0x0000, 0x0000, 0x0000,
		0x001F, 0x0043, 0x0067, 0x0000, 0x0000, 0x0000,
		0x0028, 0x004C, 0x0070, 0x0000, 0x0000, 0x0000,
		0x0023, 0x0047, 0x006B, 0x0000, 0x0000, 0x0000
	};

	if (facing == 0) {
		if (maxAnimationFrame[36 + character] > ch->currentAnimFrame)
			ch->currentAnimFrame = maxAnimationFrame[36 + character];
		if (maxAnimationFrame[30 + character] < ch->currentAnimFrame)
			ch->currentAnimFrame = maxAnimationFrame[36 + character];
	} else if (facing == 4) {
		if (maxAnimationFrame[18 + character] > ch->currentAnimFrame)
			ch->currentAnimFrame = maxAnimationFrame[18 + character];
		if (maxAnimationFrame[12 + character] < ch->currentAnimFrame)
			ch->currentAnimFrame = maxAnimationFrame[18 + character];
	} else {
		if (maxAnimationFrame[18 + character] < ch->currentAnimFrame)
			ch->currentAnimFrame = maxAnimationFrame[30 + character];
		if (maxAnimationFrame[character] == ch->currentAnimFrame)
			ch->currentAnimFrame = maxAnimationFrame[6 + character];
		if (maxAnimationFrame[character] < ch->currentAnimFrame)
			ch->currentAnimFrame = maxAnimationFrame[6 + character] + 2;
	}

	if (character == 0 && (_brandonStatusBit & 0x10))
		ch->currentAnimFrame = 88;

	_animator->animRefreshNPC(character);
}

void KyraEngine_LoK::loadSceneMsc() {
	assert(_currentCharacter->sceneId < _roomTableSize);
	int tableId = _roomTable[_currentCharacter->sceneId].nameIndex;
	assert(tableId < _roomFilenameTableSize);
	char fileNameBuffer[32];
	strcpy(fileNameBuffer, _roomFilenameTable[tableId]);
	strcat(fileNameBuffer, ".MSC");
	_screen->fillRect(0, 0, 319, 199, 0, 5);
	_res->exists(fileNameBuffer, true);
	_screen->loadBitmap(fileNameBuffer, 3, 5, 0);
}

void KyraEngine_LoK::startSceneScript(int brandonAlive) {
	assert(_currentCharacter->sceneId < _roomTableSize);
	int tableId = _roomTable[_currentCharacter->sceneId].nameIndex;
	assert(tableId < _roomFilenameTableSize);
	char fileNameBuffer[32];
	strcpy(fileNameBuffer, _roomFilenameTable[tableId]);
	strcat(fileNameBuffer, ".CPS");
	_screen->clearPage(3);
	_res->exists(fileNameBuffer, true);
	// FIXME: check this hack for amiga version
	_screen->loadBitmap(fileNameBuffer, 3, 3, (_flags.platform == Common::kPlatformAmiga ? &_screen->getPalette(0) : 0));
	_sprites->loadSceneShapes();
	_exitListPtr = 0;

	_scaleMode = 1;
	for (int i = 0; i < 145; ++i)
		_scaleTable[i] = 256;

	clearNoDropRects();
	_emc->init(&_scriptClick, &_scriptClickData);
	strcpy(fileNameBuffer, _roomFilenameTable[tableId]);
	strcat(fileNameBuffer, ".EMC");
	_res->exists(fileNameBuffer, true);
	_emc->unload(&_scriptClickData);
	_emc->load(fileNameBuffer, &_scriptClickData, &_opcodes);
	_emc->start(&_scriptClick, 0);
	_scriptClick.regs[0] = _currentCharacter->sceneId;
	_scriptClick.regs[7] = brandonAlive;

	while (_emc->isValid(&_scriptClick))
		_emc->run(&_scriptClick);
}

void KyraEngine_LoK::initSceneData(int facing, int unk1, int brandonAlive) {
	int16 xpos2 = 0;
	int setFacing = 1;

	int16 xpos = 0, ypos = 0;

	if (_brandonPosX == -1 && _brandonPosY == -1) {
		switch (facing + 1) {
		case 0:
			xpos = ypos = -1;
			break;

		case 1: case 2: case 8:
			xpos = _sceneExits.southXPos;
			ypos = _sceneExits.southYPos;
			break;

		case 3:
			xpos = _sceneExits.westXPos;
			ypos = _sceneExits.westYPos;
			break;

		case 4: case 5: case 6:
			xpos = _sceneExits.northXPos;
			ypos = _sceneExits.northYPos;
			break;

		case 7:
			xpos = _sceneExits.eastXPos;
			ypos = _sceneExits.eastYPos;
			break;

		default:
			break;
		}

		if ((uint8)(_northExitHeight & 0xFF) + 2 >= ypos)
			ypos = (_northExitHeight & 0xFF) + 4;
		if (xpos >= 308)
			xpos = 304;
		if ((uint8)(_northExitHeight >> 8) - 2 <= ypos)
			ypos = (_northExitHeight >> 8) - 4;
		if (xpos <= 12)
			xpos = 16;
	}

	if (_brandonPosX > -1)
		xpos = _brandonPosX;
	if (_brandonPosY > -1)
		ypos = _brandonPosY;

	int16 ypos2 = 0;
	if (_brandonPosX > -1 && _brandonPosY > -1) {
		switch (_currentCharacter->sceneId) {
		case 1:
			_currentCharacter->x1 = xpos;
			_currentCharacter->x2 = xpos;
			_currentCharacter->y1 = ypos;
			_currentCharacter->y2 = ypos;
			facing = 4;
			xpos2 = 192;
			ypos2 = 104;
			setFacing = 0;
			unk1 = 1;
			break;

		case 3:
			_currentCharacter->x1 = xpos;
			_currentCharacter->x2 = xpos;
			_currentCharacter->y1 = ypos;
			_currentCharacter->y2 = ypos;
			facing = 2;
			xpos2 = 204;
			ypos2 = 94;
			setFacing = 0;
			unk1 = 1;
			break;

		case 26:
			_currentCharacter->x1 = xpos;
			_currentCharacter->x2 = xpos;
			_currentCharacter->y1 = ypos;
			_currentCharacter->y2 = ypos;
			facing = 2;
			xpos2 = 192;
			ypos2 = 128;
			setFacing = 0;
			unk1 = 1;
			break;

		case 44:
			_currentCharacter->x1 = xpos;
			_currentCharacter->x2 = xpos;
			_currentCharacter->y1 = ypos;
			_currentCharacter->y2 = ypos;
			facing = 6;
			xpos2 = 156;
			ypos2 = 96;
			setFacing = 0;
			unk1 = 1;
			break;

		case 37:
			_currentCharacter->x1 = xpos;
			_currentCharacter->x2 = xpos;
			_currentCharacter->y1 = ypos;
			_currentCharacter->y2 = ypos;
			facing = 2;
			xpos2 = 148;
			ypos2 = 114;
			setFacing = 0;
			unk1 = 1;
			break;

		default:
			break;
		}
	}

	_brandonPosX = _brandonPosY = -1;

	if (unk1 && setFacing) {
		ypos2 = ypos;
		xpos2 = xpos;
		switch (facing) {
		case 0:
			ypos = 142;
			break;

		case 2:
			xpos = -16;
			break;

		case 4:
			ypos = (uint8)(_northExitHeight & 0xFF) - 4;
			break;

		case 6:
			xpos = 336;
			break;

		default:
			break;
		}
	}

	xpos2 = (int16)(xpos2 & 0xFFFC);
	ypos2 = (int16)(ypos2 & 0xFFFE);
	xpos = (int16)(xpos & 0xFFFC);
	ypos = (int16)(ypos & 0xFFFE);
	_currentCharacter->facing = facing;
	_currentCharacter->x1 = xpos;
	_currentCharacter->x2 = xpos;
	_currentCharacter->y1 = ypos;
	_currentCharacter->y2 = ypos;

	initSceneObjectList(brandonAlive);

	if (unk1 && brandonAlive == 0)
		moveCharacterToPos(0, facing, xpos2, ypos2);

	_scriptClick.regs[4] = _itemInHand;
	_scriptClick.regs[7] = brandonAlive;
	_emc->start(&_scriptClick, 3);
	while (_emc->isValid(&_scriptClick))
		_emc->run(&_scriptClick);
}

void KyraEngine_LoK::initSceneObjectList(int brandonAlive) {
	for (int i = 0; i < 28; ++i)
		_animator->actors()[i].active = 0;

	int startAnimFrame = 0;

	Animator_LoK::AnimObject *curAnimState = _animator->actors();
	curAnimState->active = 1;
	curAnimState->drawY = _currentCharacter->y1;
	curAnimState->sceneAnimPtr = _shapes[_currentCharacter->currentAnimFrame];
	curAnimState->animFrameNumber = _currentCharacter->currentAnimFrame;
	startAnimFrame = _currentCharacter->currentAnimFrame - 7;
	int xOffset = _defaultShapeTable[startAnimFrame].xOffset;
	int yOffset = _defaultShapeTable[startAnimFrame].yOffset;

	if (_scaleMode) {
		curAnimState->x1 = _currentCharacter->x1;
		curAnimState->y1 = _currentCharacter->y1;

		_animator->_brandonScaleX = _scaleTable[_currentCharacter->y1];
		_animator->_brandonScaleY = _scaleTable[_currentCharacter->y1];

		curAnimState->x1 += (_animator->_brandonScaleX * xOffset) >> 8;
		curAnimState->y1 += (_animator->_brandonScaleY * yOffset) >> 8;
	} else {
		curAnimState->x1 = _currentCharacter->x1 + xOffset;
		curAnimState->y1 = _currentCharacter->y1 + yOffset;
	}

	curAnimState->x2 = curAnimState->x1;
	curAnimState->y2 = curAnimState->y1;
	curAnimState->refreshFlag = 1;
	curAnimState->bkgdChangeFlag = 1;
	_animator->clearQueue();
	_animator->addObjectToQueue(curAnimState);

	int listAdded = 0;
	int addedObjects = 1;

	for (int i = 1; i < 5; ++i) {
		Character *ch = &_characterList[i];
		curAnimState = &_animator->actors()[addedObjects];
		if (ch->sceneId != _currentCharacter->sceneId) {
			curAnimState->active = 0;
			curAnimState->refreshFlag = 0;
			curAnimState->bkgdChangeFlag = 0;
			++addedObjects;
			continue;
		}

		curAnimState->drawY = ch->y1;
		curAnimState->sceneAnimPtr = _shapes[ch->currentAnimFrame];
		curAnimState->animFrameNumber = ch->currentAnimFrame;
		startAnimFrame = ch->currentAnimFrame - 7;
		xOffset = _defaultShapeTable[startAnimFrame].xOffset;
		yOffset = _defaultShapeTable[startAnimFrame].yOffset;
		if (_scaleMode) {
			curAnimState->x1 = ch->x1;
			curAnimState->y1 = ch->y1;

			_animator->_brandonScaleX = _scaleTable[ch->y1];
			_animator->_brandonScaleY = _scaleTable[ch->y1];

			curAnimState->x1 += (_animator->_brandonScaleX * xOffset) >> 8;
			curAnimState->y1 += (_animator->_brandonScaleY * yOffset) >> 8;
		} else {
			curAnimState->x1 = ch->x1 + xOffset;
			curAnimState->y1 = ch->y1 + yOffset;
		}
		curAnimState->x2 = curAnimState->x1;
		curAnimState->y2 = curAnimState->y1;
		curAnimState->active = 1;
		curAnimState->refreshFlag = 1;
		curAnimState->bkgdChangeFlag = 1;

		if (ch->facing >= 1 && ch->facing <= 3)
			curAnimState->flags |= 1;
		else if (ch->facing >= 5 && ch->facing <= 7)
			curAnimState->flags &= 0xFFFFFFFE;

		_animator->addObjectToQueue(curAnimState);

		++addedObjects;
		++listAdded;
		if (listAdded < 2)
			i = 5;
	}

	for (int i = 0; i < 11; ++i) {
		curAnimState = &_animator->sprites()[i];

		if (_sprites->_anims[i].play) {
			curAnimState->active = 1;
			curAnimState->refreshFlag = 1;
			curAnimState->bkgdChangeFlag = 1;
		} else {
			curAnimState->active = 0;
			curAnimState->refreshFlag = 0;
			curAnimState->bkgdChangeFlag = 0;
		}
		curAnimState->height = _sprites->_anims[i].height;
		curAnimState->height2 = _sprites->_anims[i].height2;
		curAnimState->width = _sprites->_anims[i].width + 1;
		curAnimState->width2 = _sprites->_anims[i].width2;
		curAnimState->drawY = _sprites->_anims[i].drawY;
		curAnimState->x1 = curAnimState->x2 = _sprites->_anims[i].x;
		curAnimState->y1 = curAnimState->y2 = _sprites->_anims[i].y;
		curAnimState->background = _sprites->_anims[i].background;
		curAnimState->sceneAnimPtr = _sprites->_sceneShapes[_sprites->_anims[i].sprite];

		curAnimState->disable = _sprites->_anims[i].disable;

		if (_sprites->_anims[i].unk2)
			curAnimState->flags = 0x800;
		else
			curAnimState->flags = 0;

		if (_sprites->_anims[i].flipX)
			curAnimState->flags |= 0x1;

		_animator->addObjectToQueue(curAnimState);
	}

	for (int i = 0; i < 12; ++i) {
		curAnimState = &_animator->items()[i];
		Room *curRoom = &_roomTable[_currentCharacter->sceneId];
		byte curItem = curRoom->itemsTable[i];
		if (curItem != 0xFF) {
			curAnimState->drawY = curRoom->itemsYPos[i];
			curAnimState->sceneAnimPtr = _shapes[216 + curItem];
			curAnimState->animFrameNumber = (int16)0xFFFF;
			curAnimState->y1 = curRoom->itemsYPos[i];
			curAnimState->x1 = curRoom->itemsXPos[i];

			curAnimState->x1 -= (_animator->fetchAnimWidth(curAnimState->sceneAnimPtr, _scaleTable[curAnimState->drawY])) >> 1;
			curAnimState->y1 -= _animator->fetchAnimHeight(curAnimState->sceneAnimPtr, _scaleTable[curAnimState->drawY]);

			curAnimState->x2 = curAnimState->x1;
			curAnimState->y2 = curAnimState->y1;

			curAnimState->active = 1;
			curAnimState->refreshFlag = 1;
			curAnimState->bkgdChangeFlag = 1;

			_animator->addObjectToQueue(curAnimState);
		} else {
			curAnimState->active = 0;
			curAnimState->refreshFlag = 0;
			curAnimState->bkgdChangeFlag = 0;
		}
	}

	_animator->preserveAnyChangedBackgrounds();
	curAnimState = _animator->actors();
	curAnimState->bkgdChangeFlag = 1;
	curAnimState->refreshFlag = 1;
	for (int i = 1; i < 28; ++i) {
		curAnimState = &_animator->objects()[i];
		if (curAnimState->active) {
			curAnimState->bkgdChangeFlag = 1;
			curAnimState->refreshFlag = 1;
		}
	}
	_animator->restoreAllObjectBackgrounds();
	_animator->preserveAnyChangedBackgrounds();
	_animator->prepDrawAllObjects();
	initSceneScreen(brandonAlive);
	_animator->copyChangedObjectsForward(0);
}

void KyraEngine_LoK::initSceneScreen(int brandonAlive) {
	if (_flags.platform == Common::kPlatformAmiga) {
		if (_unkScreenVar1 && !queryGameFlag(0xF0)) {
			_screen->getPalette(2).clear();
			if (_currentCharacter->sceneId != 117 || !queryGameFlag(0xB3))
				_screen->setScreenPalette(_screen->getPalette(2));
		}

		if (_unkScreenVar2 == 1)
			_screen->shuffleScreen(8, 8, 304, 128, 2, 0, _unkScreenVar3, false);
		else
			_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0, Screen::CR_NO_P_CHECK);

		if (_unkScreenVar1 && !queryGameFlag(0xA0)) {
			if (_currentCharacter->sceneId == 45 && _cauldronState)
				_screen->getPalette(0).copy(_screen->getPalette(4), 12, 1);

			if (_currentCharacter->sceneId >= 229 && _currentCharacter->sceneId <= 245 && (_brandonStatusBit & 1))
				_screen->copyPalette(0, 10);

			_screen->setScreenPalette(_screen->getPalette(0));
		}
	} else {
		if (_unkScreenVar1 && !queryGameFlag(0xA0)) {
			for (int i = 0; i < 60; ++i) {
				uint16 col = _screen->getPalette(0)[684 + i];
				col += _screen->getPalette(1)[684 + i] << 1;
				col >>= 2;
				_screen->getPalette(0)[684 + i] = col;
			}
			_screen->setScreenPalette(_screen->getPalette(0));
		}

		if (_unkScreenVar2 == 1)
			_screen->shuffleScreen(8, 8, 304, 128, 2, 0, _unkScreenVar3, false);
		else
			_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0);

		if (_unkScreenVar1 && _paletteChanged) {
			if (!queryGameFlag(0xA0)) {
				_screen->getPalette(0).copy(_screen->getPalette(1), 228, 20);
				_screen->setScreenPalette(_screen->getPalette(0));
			} else {
				_screen->getPalette(0).clear();
			}
		}
	}

	if (!_emc->start(&_scriptClick, 2))
		error("Could not start script function 2 of scene script");

	_scriptClick.regs[7] = brandonAlive;

	while (_emc->isValid(&_scriptClick))
		_emc->run(&_scriptClick);

	setTextFadeTimerCountdown(-1);

	if (_currentCharacter->sceneId == 210) {
		if (_itemInHand != kItemNone)
			magicOutMouseItem(2, -1);

		_screen->hideMouse();
		for (int i = 0; i < 10; ++i) {
			if (_currentCharacter->inventoryItems[i] != kItemNone)
				magicOutMouseItem(2, i);
		}
		_screen->showMouse();
	}
}

int KyraEngine_LoK::handleSceneChange(int xpos, int ypos, int unk1, int frameReset) {
	if (queryGameFlag(0xEF))
		unk1 = 0;

	int sceneId = _currentCharacter->sceneId;
	_pathfinderFlag = 0;

	if (xpos < 12) {
		if (_roomTable[sceneId].westExit != 0xFFFF) {
			xpos = 12;
			ypos = _sceneExits.westYPos;
			_pathfinderFlag = 7;
		}
	} else if (xpos >= 308) {
		if (_roomTable[sceneId].eastExit != 0xFFFF) {
			xpos = 307;
			ypos = _sceneExits.eastYPos;
			_pathfinderFlag = 13;
		}
	}

	if (ypos <= (_northExitHeight & 0xFF) + 2) {
		if (_roomTable[sceneId].northExit != 0xFFFF) {
			xpos = _sceneExits.northXPos;
			ypos = _northExitHeight & 0xFF;
			_pathfinderFlag = 14;
		}
	} else if (ypos >= 136) {
		if (_roomTable[sceneId].southExit != 0xFFFF) {
			xpos = _sceneExits.southXPos;
			ypos = 136;
			_pathfinderFlag = 11;
		}
	}

	int temp = xpos - _currentCharacter->x1;
	if (ABS(temp) < 4) {
		temp = ypos - _currentCharacter->y1;
		if (ABS(temp) < 2)
			return 0;
	}

	int x = (int16)(_currentCharacter->x1 & 0xFFFC);
	int y = (int16)(_currentCharacter->y1 & 0xFFFE);
	xpos = (int16)(xpos & 0xFFFC);
	ypos = (int16)(ypos & 0xFFFE);

	int ret = findWay(x, y, xpos, ypos, _movFacingTable, 150);
	_pathfinderFlag = 0;

	if (ret >= _lastFindWayRet)
		_lastFindWayRet = ret;

	if (ret == 0x7D00 || ret == 0)
		return 0;

	return processSceneChange(_movFacingTable, unk1, frameReset);
}

int KyraEngine_LoK::processSceneChange(int *table, int unk1, int frameReset) {
	if (queryGameFlag(0xEF))
		unk1 = 0;

	int *tableStart = table;
	_sceneChangeState = 0;
	_loopFlag2 = 0;
	bool running = true;
	int returnValue = 0;
	uint32 nextFrame = 0;

	while (running) {
		bool forceContinue = false;
		switch (*table) {
		case 0: case 1: case 2:
		case 3: case 4: case 5:
		case 6: case 7:
			_currentCharacter->facing = getOppositeFacingDirection(*table);
			break;

		case 8:
			forceContinue = true;
			running = false;
			break;

		default:
			++table;
			forceContinue = true;
		}

		returnValue = changeScene(_currentCharacter->facing);
		if (returnValue)
			running = false;

		if (unk1) {
			if (skipFlag()) {
				resetSkipFlag(false);
				running = false;
				_sceneChangeState = 1;
			}
		}

		if (forceContinue || !running)
			continue;

		int temp = 0;
		if (table == tableStart || table[1] == 8)
			temp = setCharacterPosition(0, 0);
		else
			temp = setCharacterPosition(0, table);

		if (temp)
			++table;

		nextFrame = _timer->getDelay(5) * _tickLength + _system->getMillis();
		while (_system->getMillis() < nextFrame) {
			_timer->update();

			if (_currentCharacter->sceneId == 210) {
				updateKyragemFading();
				if (seq_playEnd() || _beadStateVar == 4 || _beadStateVar == 5) {
					*table = 8;
					running = false;
					break;
				}
			}

			if ((nextFrame - _system->getMillis()) >= 10)
				delay(10, true);
		}
	}

	if (frameReset && !(_brandonStatusBit & 2))
		_currentCharacter->currentAnimFrame = 7;

	_animator->animRefreshNPC(0);
	_animator->updateAllObjectShapes();
	return returnValue;
}

int KyraEngine_LoK::changeScene(int facing) {
	if (queryGameFlag(0xEF)) {
		if (_currentCharacter->sceneId == 5)
			return 0;
	}

	int xpos = _charAddXPosTable[facing] + _currentCharacter->x1;
	int ypos = _charAddYPosTable[facing] + _currentCharacter->y1;

	if (xpos >= 12 && xpos <= 308) {
		if (!lineIsPassable(xpos, ypos))
			return false;
	}

	if (_exitListPtr) {
		int16 *ptr = _exitListPtr;
		// this loop should be only entered one time, seems to be some hack in the original
		while (true) {
			if (*ptr == -1)
				break;

			if (*ptr > _currentCharacter->x1 || _currentCharacter->y1 < ptr[1] || _currentCharacter->x1 > ptr[2] || _currentCharacter->y1 > ptr[3]) {
				ptr += 10;
				break;
			}

			_brandonPosX = ptr[6];
			_brandonPosY = ptr[7];
			uint16 sceneId = ptr[5];
			facing = ptr[4];
			int unk1 = ptr[8];
			int unk2 = ptr[9];

			if (sceneId == 0xFFFF) {
				switch (facing) {
				case 0:
					sceneId = _roomTable[_currentCharacter->sceneId].northExit;
					break;

				case 2:
					sceneId = _roomTable[_currentCharacter->sceneId].eastExit;
					break;

				case 4:
					sceneId = _roomTable[_currentCharacter->sceneId].southExit;
					break;

				case 6:
					sceneId = _roomTable[_currentCharacter->sceneId].westExit;
					break;

				default:
					break;
				}
			}

			_currentCharacter->facing = facing;
			_animator->animRefreshNPC(0);
			_animator->updateAllObjectShapes();
			enterNewScene(sceneId, facing, unk1, unk2, 0);
			resetGameFlag(0xEE);
			return 1;
		}
	}

	int returnValue = 0;
	facing = 0;

	if ((_northExitHeight & 0xFF) + 2 >= ypos || (_northExitHeight & 0xFF) + 2 >= _currentCharacter->y1) {
		facing = 0;
		returnValue = 1;
	}

	if (xpos >= 308 || (_currentCharacter->x1 + 4) >= 308) {
		facing = 2;
		returnValue = 1;
	}

	if (((_northExitHeight >> 8) & 0xFF) - 2 < ypos || ((_northExitHeight >> 8) & 0xFF) - 2 < _currentCharacter->y1) {
		facing = 4;
		returnValue = 1;
	}

	if (xpos <= 12 || _currentCharacter->y1 <= 12) {
		facing = 6;
		returnValue = 1;
	}

	if (!returnValue)
		return 0;

	uint16 sceneId = 0xFFFF;
	switch (facing) {
	case 0:
		sceneId = _roomTable[_currentCharacter->sceneId].northExit;
		break;

	case 2:
		sceneId = _roomTable[_currentCharacter->sceneId].eastExit;
		break;

	case 4:
		sceneId = _roomTable[_currentCharacter->sceneId].southExit;
		break;

	default:
		sceneId = _roomTable[_currentCharacter->sceneId].westExit;
	}

	if (sceneId == 0xFFFF)
		return 0;

	enterNewScene(sceneId, facing, 1, 1, 0);
	return returnValue;
}

void KyraEngine_LoK::setCharactersInDefaultScene() {
	static const uint32 defaultSceneTable[][4] = {
		{ 0xFFFF, 0x0004, 0x0003, 0xFFFF },
		{ 0xFFFF, 0x0022, 0xFFFF, 0x0000 },
		{ 0xFFFF, 0x001D, 0x0021, 0xFFFF },
		{ 0xFFFF, 0x0000, 0x0000, 0xFFFF }
	};

	for (int i = 1; i < 5; ++i) {
		Character *cur = &_characterList[i];
		//cur->field_20 = 0;

		const uint32 *curTable = defaultSceneTable[i - 1];
		cur->sceneId = curTable[0];

		if (cur->sceneId == _currentCharacter->sceneId)
			//++cur->field_20;
			cur->sceneId = curTable[1/*cur->field_20*/];

		//cur->field_23 = curTable[cur->field_20+1];
	}
}

void KyraEngine_LoK::setCharactersPositions(int character) {
	static const uint16 initXPosTable[] = {
		0x3200, 0x0024, 0x2230, 0x2F00, 0x0020, 0x002B,
		0x00CA, 0x00F0, 0x0082, 0x00A2, 0x0042
	};
	static const uint8 initYPosTable[] = {
		0x00, 0xA2, 0x00, 0x42, 0x00,
		0x67, 0x67, 0x60, 0x5A, 0x71,
		0x76
	};

	assert(character < ARRAYSIZE(initXPosTable));
	Character *edit = &_characterList[character];
	edit->x1 = edit->x2 = initXPosTable[character];
	edit->y1 = edit->y2 = initYPosTable[character];
}

#pragma mark -
#pragma mark - Pathfinder
#pragma mark -

int KyraEngine_LoK::findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize) {
	int ret = KyraEngine_v1::findWay(x, y, toX, toY, moveTable, moveTableSize);
	if (ret == 0x7D00)
		return 0;
	return getMoveTableSize(moveTable);
}

bool KyraEngine_LoK::lineIsPassable(int x, int y) {
	if (queryGameFlag(0xEF)) {
		if (_currentCharacter->sceneId == 5)
			return true;
	}

	if (_pathfinderFlag & 2) {
		if (x >= 312)
			return false;
	}

	if (_pathfinderFlag & 4) {
		if (y >= 136)
			return false;
	}

	if (_pathfinderFlag & 8) {
		if (x < 8)
			return false;
	}

	if (_pathfinderFlag2) {
		if (x <= 8 || x >= 312)
			return true;
		if (y < (_northExitHeight & 0xFF) || y > 135)
			return true;
	}

	if (y > 137)
		return false;

	if (y < 0)
		y = 0;

	int ypos = 8;
	if (_scaleMode) {
		ypos = (_scaleTable[y] >> 5) + 1;
		if (8 < ypos)
			ypos = 8;
	}

	x -= (ypos >> 1);

	int xpos = x;
	int xtemp = xpos + ypos - 1;
	if (x < 0)
		xpos = 0;

	if (xtemp > 319)
		xtemp = 319;

	for (; xpos < xtemp; ++xpos) {
		if (!_screen->getShapeFlag1(xpos, y))
			return false;
	}
	return true;
}

#pragma mark -

void KyraEngine_LoK::setupZanthiaPalette(int pal) {
	uint8 r, g, b;

	switch (pal - 17) {
	case 0:
		// 0x88F
		r = 33;
		g = 33;
		b = 63;
		break;

	case 1:
		// 0x00F
		r = 0;
		g = 0;
		b = 63;
		break;

	case 2:
		// 0xF88
		r = 63;
		g = 33;
		b = 33;
		break;

	case 3:
		// 0xF00
		r = 63;
		g = 0;
		b = 0;
		break;

	case 4:
		// 0xFF9
		r = 63;
		g = 63;
		b = 37;
		break;

	case 5:
		// 0xFF1
		r = 63;
		g = 63;
		b = 4;
		break;

	default:
		// 0xFFF
		r = 63;
		g = 63;
		b = 63;
	}

	_screen->getPalette(4)[12 * 3 + 0] = r;
	_screen->getPalette(4)[12 * 3 + 1] = g;
	_screen->getPalette(4)[12 * 3 + 2] = b;
}

#pragma mark -

void KyraEngine_LoK::setupSceneResource(int sceneId) {
	if (!_flags.isTalkie)
		return;

	if (_currentRoom != 0xFFFF) {
		assert(_currentRoom < _roomTableSize);
		int tableId = _roomTable[_currentRoom].nameIndex;
		assert(tableId < _roomFilenameTableSize);

		// unload our old room
		char file[64];
		strcpy(file, _roomFilenameTable[tableId]);
		strcat(file, ".VRM");
		_res->unloadPakFile(file);

		strcpy(file, _roomFilenameTable[tableId]);
		strcat(file, ".PAK");
		_res->unloadPakFile(file);

		strcpy(file, _roomFilenameTable[tableId]);
		strcat(file, ".APK");
		_res->unloadPakFile(file);
	}

	assert(sceneId < _roomTableSize);
	int tableId = _roomTable[sceneId].nameIndex;
	assert(tableId < _roomFilenameTableSize);

	// load our new room
	char file[64];
	strcpy(file, _roomFilenameTable[tableId]);
	strcat(file, ".VRM");
	if (_res->exists(file))
		_res->loadPakFile(file);

	strcpy(file, _roomFilenameTable[tableId]);
	strcat(file, ".PAK");
	if (_res->exists(file))
		_res->loadPakFile(file);

	strcpy(file, _roomFilenameTable[tableId]);
	strcat(file, ".APK");
	if (_res->exists(file))
		_res->loadPakFile(file);
}

} // End of namespace Kyra
