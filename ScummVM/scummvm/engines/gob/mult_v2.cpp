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

#include "common/endian.h"
#include "common/stream.h"

#include "gob/gob.h"
#include "gob/mult.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/goblin.h"
#include "gob/inter.h"
#include "gob/scenery.h"
#include "gob/map.h"
#include "gob/video.h"
#include "gob/videoplayer.h"

namespace Gob {

Mult_v2::Mult_v2(GobEngine *vm) : Mult_v1(vm) {
	_renderObjs = 0;
	_multData = 0;
	for (int i = 0; i < 8; i++)
		_multDatas[i] = 0;
}

Mult_v2::~Mult_v2() {
	freeMultKeys();
	for (int i = 0; i < 8; i++) {
		_multData = _multDatas[i];
		freeMultKeys();
	}
}

void Mult_v2::loadMult(int16 resId) {
	int8 index;
	uint8 staticCount;
	uint8 animCount;
	bool hasImds;

	index = (resId & 0x8000) ? _vm->_game->_script->readByte() : 0;
	resId &= 0x7FFF;

	debugC(4, kDebugGameFlow, "Loading mult %d", index);

	_multData = new Mult_Data;
	memset(_multData, 0, sizeof(Mult_Data));

	_multDatas[index] = _multData;

	for (int i = 0; i < 4; i++)
		_multData->animObjs[0][i] = i;

	_multData->sndSlotsCount = 0;
	_multData->frameStart = 0;

	Resource *resource = _vm->_game->_resources->getResource(resId);
	if (!resource)
		return;

	Common::SeekableReadStream &data = *resource->stream();

	_multData->staticCount = staticCount = data.readSByte();
	_multData->animCount = animCount = data.readSByte();
	staticCount++;
	animCount++;

	hasImds = (staticCount & 0x80) != 0;
	staticCount &= 0x7F;

	debugC(7, kDebugGraphics, "statics: %u, anims: %u, imds: %u",
			staticCount, animCount, hasImds);

	for (int i = 0; i < 10; i++) {
		_multData->staticLoaded[i] = false;
		_multData->animLoaded[i] = false;
	}

	for (int i = 0; i < staticCount; i++, data.seek(14, SEEK_CUR)) {
		_multData->staticIndices[i] = _vm->_scenery->loadStatic(1);

		if (_multData->staticIndices[i] >= 100) {
			_multData->staticIndices[i] -= 100;
			_multData->staticLoaded[i] = true;
		}
	}

	for (int i = 0; i < animCount; i++, data.seek(14, SEEK_CUR)) {
		_multData->animIndices[i] = _vm->_scenery->loadAnim(1);

		if (_multData->animIndices[i] >= 100) {
			_multData->animIndices[i] -= 100;
			_multData->animLoaded[i] = true;
		}
	}

	_multData->frameRate = data.readSint16LE();
	_multData->staticKeysCount = data.readSint16LE();
	_multData->staticKeys = new Mult_StaticKey[_multData->staticKeysCount];
	for (int i = 0; i < _multData->staticKeysCount; i++) {
		_multData->staticKeys[i].frame = data.readSint16LE();
		_multData->staticKeys[i].layer = data.readSint16LE();
	}

	for (int i = 0; i < 4; i++) {
		_multData->imdKeysCount[i] = 0;
		_multData->imdKeys[i] = 0;
		_multData->imdIndices[i] = -1;

		for (int j = 0; j < 4; j++) {
			_multData->animKeysIndices[i][j] = 0;
			_multData->imdKeysIndices[i][j] = 0;
		}

		_multData->animKeysFrames[i] = -1;
		_multData->animKeysCount[i] = data.readSint16LE();
		_multData->animKeys[i] = new Mult_AnimKey[_multData->animKeysCount[i]];
		for (int j = 0; j < _multData->animKeysCount[i]; j++) {
			_multData->animKeys[i][j].frame = data.readSint16LE();
			_multData->animKeys[i][j].layer = data.readSint16LE();
			_multData->animKeys[i][j].posX = data.readSint16LE();
			_multData->animKeys[i][j].posY = data.readSint16LE();
			_multData->animKeys[i][j].order = data.readSint16LE();
		}
	}

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 16; j++) {
			_multData->fadePal[i][j].red = data.readByte();
			_multData->fadePal[i][j].green = data.readByte();
			_multData->fadePal[i][j].blue = data.readByte();
		}
	}

	_multData->palFadeKeysCount = data.readSint16LE();
	_multData->palFadeKeys = new Mult_PalFadeKey[_multData->palFadeKeysCount];
	for (int i = 0; i < _multData->palFadeKeysCount; i++) {
		_multData->palFadeKeys[i].frame = data.readSint16LE();
		_multData->palFadeKeys[i].fade = data.readSint16LE();
		_multData->palFadeKeys[i].palIndex = data.readSint16LE();
		_multData->palFadeKeys[i].flag = data.readSByte();
	}

	_multData->palKeysCount = data.readSint16LE();
	_multData->palKeys = new Mult_PalKey[_multData->palKeysCount];
	for (int i = 0; i < _multData->palKeysCount; i++) {
		_multData->palKeys[i].frame = data.readSint16LE();
		_multData->palKeys[i].cmd = data.readSint16LE();
		_multData->palKeys[i].rates[0] = data.readSint16LE();
		_multData->palKeys[i].rates[1] = data.readSint16LE();
		_multData->palKeys[i].rates[2] = data.readSint16LE();
		_multData->palKeys[i].rates[3] = data.readSint16LE();
		_multData->palKeys[i].unknown0 = data.readSint16LE();
		_multData->palKeys[i].unknown1 = data.readSint16LE();
		data.read(_multData->palKeys[i].subst, 64);
	}

	_multData->textKeysCount = data.readSint16LE();
	_multData->textKeys = new Mult_TextKey[_multData->textKeysCount];
	for (int i = 0; i < _multData->textKeysCount; i++) {
		_multData->textKeys[i].frame = data.readSint16LE();
		_multData->textKeys[i].cmd = data.readSint16LE();
		if (!hasImds)
			data.seek(24, SEEK_CUR);
	}

	_multData->sndKeysCount = data.readSint16LE();
	_multData->sndKeys = new Mult_SndKey[_multData->sndKeysCount];
	for (int i = 0; i < _multData->sndKeysCount; i++) {
		int j;

		_multData->sndKeys[i].frame = data.readSint16LE();
		_multData->sndKeys[i].cmd = data.readSint16LE();
		_multData->sndKeys[i].freq = data.readSint16LE();
		_multData->sndKeys[i].fadeLength = data.readSint16LE();
		_multData->sndKeys[i].repCount = data.readSint16LE();
		_multData->sndKeys[i].soundIndex = -1;
		_multData->sndKeys[i].resId = -1;
		data.seek(2, SEEK_CUR);
		if (!hasImds)
			data.seek(24, SEEK_CUR);

		switch (_multData->sndKeys[i].cmd) {
		case 1:
		case 4:
			_multData->sndKeys[i].resId = _vm->_game->_script->peekUint16();
			for (j = 0; j < i; j++) {
				if (_multData->sndKeys[j].resId ==
						_multData->sndKeys[i].resId) {
					_multData->sndKeys[i].soundIndex =
						_multData->sndKeys[j].soundIndex;
					_vm->_game->_script->skip(2);
					break;
				}
			}
			if (i == j) {
				_multData->sndSlot[_multData->sndSlotsCount] =
					_vm->_inter->loadSound(1);
				_multData->sndKeys[i].soundIndex =
					_multData->sndSlot[_multData->sndSlotsCount] & 0x7FFF;
				_multData->sndSlotsCount++;
			}
			break;
		case 3:
			_vm->_game->_script->skip(4);
			break;

		case -1:
			break;

		default:
			warning("Mult_v2::loadMult(): Unknown sound key command (%d)",
					_multData->sndKeys[i].cmd);
		}
	}

	_multData->imdFiles = 0;
	_multData->somepointer10 = 0;

	if (hasImds)
		loadImds(data);

	delete resource;
}

void Mult_v2::loadImds(Common::SeekableReadStream &data) {
	int16 size;

	size = _vm->_game->_script->readInt16();
	_multData->execPtr = _vm->_game->_script->getData() + _vm->_game->_script->pos();
	_vm->_game->_script->skip(size * 2);

	if (_vm->_game->_script->getVersionMinor() < 3)
		return;

	size = data.readSint16LE();
	if (size > 0) {
		_multData->somepointer10 = new char[size * 20];
		data.read(_multData->somepointer10, size * 20);
	}

	size = _vm->_game->_script->readInt16();
	if (size <= 0)
		return;

	_multData->imdFiles = new char[size * 14];
	memcpy(_multData->imdFiles,
			_vm->_game->_script->getData() + _vm->_game->_script->pos(), size * 14);

	// WORKAROUND: The Windows versions of Lost in Time and Gob3 have VMD not
	//             IMD files, but they are still referenced as IMD.
	if (((_vm->getGameType() == kGameTypeLostInTime) || (_vm->getGameType() == kGameTypeGob3)) &&
	    (_vm->getPlatform() == Common::kPlatformWindows)) {

		for (int i = 0; i < size; i++) {
			char *dot = strrchr(_multData->imdFiles + (i * 14), '.');
			if (dot)
				*dot = '\0';
		}
	}

	_vm->_game->_script->skip(size * 14);
	data.seek(2, SEEK_CUR);
	for (int i = 0; i < 4; i++) {
		_multData->imdKeysCount[i] = data.readSint16LE();
		_multData->imdKeys[i] = new Mult_ImdKey[_multData->imdKeysCount[i]];
		for (int j = 0; j < _multData->imdKeysCount[i]; j++) {
			_multData->imdKeys[i][j].frame = data.readSint16LE();
			_multData->imdKeys[i][j].imdFile = data.readSint16LE();
			_multData->imdKeys[i][j].field_4 = data.readSint16LE();
			_multData->imdKeys[i][j].field_6 = data.readSint16LE();
			_multData->imdKeys[i][j].flags = data.readUint16LE();
			_multData->imdKeys[i][j].palFrame = data.readSint16LE();
			_multData->imdKeys[i][j].lastFrame = data.readSint16LE();
			_multData->imdKeys[i][j].palStart = data.readSByte();
			_multData->imdKeys[i][j].palEnd = data.readSByte();
		}
	}
}

void Mult_v2::freeMultKeys() {
	uint8 animCount;
	uint8 staticCount;

	if (!_multData)
		return;

	staticCount = (_multData->staticCount + 1) & 0x7F;
	animCount = _multData->animCount + 1;

	for (int i = 0; i < staticCount; i++)
		if (_multData->staticLoaded[i])
			_vm->_scenery->freeStatic(_multData->staticIndices[i]);

	for (int i = 0; i < animCount; i++)
		if (_multData->animLoaded[i])
			_vm->_scenery->freeAnim(_multData->animIndices[i]);

	delete[] _multData->staticKeys;

	for (int i = 0; i < 4; i++) {
		delete[] _multData->animKeys[i];
		delete[] _multData->imdKeys[i];
	}

	delete[] _multData->palFadeKeys;
	delete[] _multData->palKeys;
	delete[] _multData->textKeys;

	for (int i = 0; i < _multData->sndSlotsCount; i++)
		if (!(_multData->sndSlot[i] & 0x8000))
			_vm->_game->freeSoundSlot(_multData->sndSlot[i]);

	delete[] _multData->sndKeys;

	delete[] _multData->imdFiles;
	delete[] _multData->somepointer10;

	if (_animDataAllocated) {
		freeMult();

		delete _animArrayX;
		delete _animArrayY;
		delete[] _animArrayData;

		_animArrayX = 0;
		_animArrayY = 0;
		_animArrayData = 0;

		_animDataAllocated = false;
	}

	for (int i = 0; i < 8; i++)
		if (_multDatas[i] == _multData)
			_multDatas[i] = 0;

	delete _multData;
	_multData = 0;
}

bool Mult_v2::hasMultData(uint16 multIndex) {
	if (multIndex > 7)
		error("Multindex out of range");

	return _multDatas[multIndex] != 0;
}

void Mult_v2::setMultData(uint16 multIndex) {
	if (multIndex > 7)
		error("Multindex out of range");

	debugC(4, kDebugGameFlow, "Switching to mult %d", multIndex);
	_multData = _multDatas[multIndex];
}

void Mult_v2::zeroMultData(uint16 multIndex) {
	if (multIndex > 7)
		error("Multindex out of range");

	_multDatas[multIndex] = 0;
}

void Mult_v2::multSub(uint16 multIndex) {
	uint16 flags;
	int16 expr;
	int16 index;
	int16 startFrame, stopFrame, firstFrame;

	flags = multIndex;
	multIndex = (multIndex >> 12) & 0xF;

	if (multIndex > 7)
		error("Multindex out of range");

	_vm->_util->notifyNewAnim();

	debugC(4, kDebugGameFlow, "Sub mult %d", multIndex);
	_multData = _multDatas[multIndex];

	if (!_multData) {
		_vm->_game->_script->readValExpr();
		_vm->_game->_script->readValExpr();
		_vm->_game->_script->readValExpr();
		_vm->_game->_script->readValExpr();
		return;
	}

	if (flags & 0x200)
		index = 3;
	else if (flags & 0x100)
		index = 2;
	else if (flags & 0x80)
		index = 1;
	else
		index = 0;

	if (flags & 0x400) {
		flags = 0x400;
		_multData->animDirection = -1;
	} else {
		_multData->animDirection = 1;
		flags &= 0x7F;
	}

	_multData->animObjs[index][0] = flags;
	for (int i = 1; i < 4; i++)
		_multData->animObjs[index][i] = _vm->_game->_script->readValExpr();

	expr = _vm->_game->_script->readValExpr();
	_multData->animKeysFrames[index] = expr;
	_multData->animKeysStartFrames[index] = expr;

	WRITE_VAR(18 + index, expr);
	if (expr == -1) {
		if (!_objects)
			return;

		for (int i = 0; i < 4; i++) {
			int obj = _multData->animObjs[index][i];

			if ((obj == -1) || (obj == 1024))
				continue;

			Mult_AnimData &animData = *(_objects[obj].pAnimData);
			animData.animType = animData.animTypeBak;
		}

		return;
	}

	startFrame = _multData->animKeysStartFrames[index];
	stopFrame = _multData->animKeysStopFrames[index];

	if (_multData->animDirection == 1) {
		stopFrame = 32000;
		for (int i = 0; i < _multData->textKeysCount; i++) {
			int16 textFrame = _multData->textKeys[i].frame;

			if ((textFrame > startFrame) && (textFrame < stopFrame))
				stopFrame = textFrame;
		}
	} else {
		stopFrame = 0;
		for (int i = 0; i < _multData->textKeysCount; i++) {
			int16 textFrame = _multData->textKeys[i].frame;

			if ((textFrame < startFrame) && (textFrame > stopFrame))
				stopFrame = textFrame;
		}
	}

	if (_objects) {
		for (int i = 0; i < 4; i++) {
			int obj = _multData->animObjs[index][i];

			if ((obj != -1) && (obj != 1024))
				_objects[obj].pAnimData->animTypeBak = _objects[obj].pAnimData->animType;
		}
	}

	for (int i = 0; i < 4; i++) {
		_multData->animKeysIndices[index][i] = 0;

		for (int j = 0; j < _multData->animKeysCount[i]; j++)
			if (_multData->animKeys[i][j].frame >= startFrame) {
				_multData->animKeysIndices[index][i] = j;
				break;
			}
	}

	if (_multData->animDirection == -1) {
		int i = 0;
		while (_multData->imdKeys[index][i].frame <= startFrame)
			i++;

		_multData->imdIndices[index] = i - 1;
	}

	firstFrame = (_multData->animDirection == 1) ? startFrame : stopFrame;
	for (int i = 0; i < 4; i++) {
		_multData->imdKeysIndices[index][i] = 0;

		for (int j = 0; j < _multData->imdKeysCount[i]; j++)
			if (_multData->imdKeys[i][j].frame >= firstFrame) {
				_multData->imdKeysIndices[index][i] = j;
				break;
			}
	}

	_multData->animKeysStartFrames[index] = startFrame;
	_multData->animKeysStopFrames[index] = stopFrame;
}

void Mult_v2::playMultInit() {
	_doPalSubst = false;
	_palFadingRed = 0;
	_palFadingGreen = 0;
	_palFadingBlue = 0;

	_oldPalette = _vm->_global->_pPaletteDesc->vgaPal;

	if (!_animSurf) {
		int16 width, height;

		if (_objects)
			for (int i = 0; i < _objCount; i++) {
				delete _objects[i].pPosX;
				delete _objects[i].pPosY;
			}

		delete[] _objects;

		_vm->_util->setFrameRate(_multData->frameRate);
		_animTop = 0;
		_animLeft = 0;
		_animWidth = _vm->_video->_surfWidth;
		_animHeight = _vm->_video->_surfHeight;
		_objCount = 4;

		delete[] _orderArray;
		delete[] _renderObjs;
		delete _animArrayX;
		delete _animArrayY;
		delete[] _animArrayData;

		_objects = new Mult_Object[_objCount];
		_orderArray = new int8[_objCount];
		_renderObjs = new Mult_Object*[_objCount];
		_animArrayX = new VariablesLE(_objCount * 4);
		_animArrayY = new VariablesLE(_objCount * 4);
		_animArrayData = new Mult_AnimData[_objCount];

		memset(_objects, 0, _objCount * sizeof(Mult_Object));
		memset(_orderArray, 0, _objCount * sizeof(int8));
		memset(_renderObjs, 0, _objCount * sizeof(Mult_Object *));
		memset(_animArrayData, 0, _objCount * sizeof(Mult_AnimData));

		for (_counter = 0; _counter < _objCount; _counter++) {
			Mult_Object &multObj = _objects[_counter];
			Mult_AnimData &animData = _animArrayData[_counter];

			multObj.pPosX = new VariableReference(*_animArrayX, _counter * 4);
			multObj.pPosY = new VariableReference(*_animArrayY, _counter * 4);
			multObj.pAnimData = &animData;

			animData.isStatic = 1;

			multObj.lastLeft = -1;
			multObj.lastTop = -1;
			multObj.lastRight = -1;
			multObj.lastBottom = -1;
		}

		width = _animWidth;
		height = _animHeight;
		_vm->_draw->adjustCoords(0, &width, &height);
		_vm->_draw->initSpriteSurf(Draw::kAnimSurface, width, height, 0);
		_animSurf = _vm->_draw->_spritesArray[Draw::kAnimSurface];

		_vm->_draw->_spritesArray[Draw::kAnimSurface]->blit(*_vm->_draw->_spritesArray[Draw::kBackSurface],
				0, 0, _vm->_video->_surfWidth, _vm->_video->_surfHeight, 0, 0);

		for (_counter = 0; _counter < _objCount; _counter++)
			_multData->palAnimIndices[_counter] = _counter;

		_animDataAllocated = true;
	} else
		_animDataAllocated = false;

	_frame = 0;
}

void Mult_v2::drawStatics(bool &stop) {
	int staticIndex;

	if (_multData->staticKeys[_multData->staticKeysCount - 1].frame > _frame)
		stop = false;

	for (_counter = 0; _counter < _multData->staticKeysCount; _counter++) {
		if ((_multData->staticKeys[_counter].frame != _frame)
		    || (_multData->staticKeys[_counter].layer == -1))
			continue;

		if (_multData->staticKeys[_counter].layer >= 0) {
			int i = 0;
			_vm->_scenery->_curStatic = 0;
			_vm->_scenery->_curStaticLayer =
				_multData->staticKeys[_counter].layer;

			staticIndex = _multData->staticIndices[i];
			while (_vm->_scenery->getStaticLayersCount(staticIndex) <=
					_vm->_scenery->_curStaticLayer) {
				_vm->_scenery->_curStaticLayer -=
					_vm->_scenery->getStaticLayersCount(staticIndex);

				staticIndex = _multData->staticIndices[++i];
				_vm->_scenery->_curStatic++;
			}
			_vm->_scenery->_curStatic =
				_multData->staticIndices[_vm->_scenery->_curStatic];
			_vm->_scenery->renderStatic(_vm->_scenery->_curStatic,
					_vm->_scenery->_curStaticLayer);
		} else {
			int layer = -_multData->staticKeys[_counter].layer - 2;

			_vm->_draw->_spriteLeft =
				READ_LE_UINT16(_multData->execPtr + layer * 2);
			_vm->_draw->_destSpriteX = 0;
			_vm->_draw->_destSpriteY = 0;
			_vm->_draw->_destSurface = Draw::kBackSurface;
			_vm->_draw->_transparency = 0;
			_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
			_vm->_scenery->_curStatic = -1;
		}

		_vm->_draw->_spritesArray[Draw::kAnimSurface]->blit(*_vm->_draw->_spritesArray[Draw::kBackSurface],
				0, 0, _vm->_video->_surfWidth, _vm->_video->_surfHeight, 0, 0);
	}
}

void Mult_v2::drawAnims(bool &stop) {
	int16 count;
	int animIndex;

	for (int i = 0; i < 4; i++) {
		int16 animKeysCount = _multData->animKeysCount[i];
		if ((animKeysCount > 0) && ((uint16) _multData->animKeys[i][animKeysCount - 1].frame > _frame))
			stop = false;
	}

	for (_index = 0; _index < 4; _index++) {
		int16 animKeysCount = _multData->animKeysCount[_index];
		for (_counter = 0; _counter < animKeysCount; _counter++) {
			Mult_AnimKey &key = _multData->animKeys[_index][_counter];
			Mult_Object &animObj = _objects[_multData->animObjs[0][_index]];
			Mult_AnimData &animData = *(animObj.pAnimData);

			if (key.frame != _frame)
				continue;

			if (key.layer != -1) {
				*(animObj.pPosX) = key.posX;
				*(animObj.pPosY) = key.posY;

				animData.frame = 0;
				animData.order = key.order;
				animData.animType = 1;

				animData.isPaused = 0;
				animData.isStatic = 0;
				animData.maxTick = 0;
				animObj.tick = 0;
				animData.layer = key.layer;

				int i = 0;
				animIndex = _multData->animIndices[i];
				count = _vm->_scenery->getAnimLayersCount(animIndex);
				while (animData.layer >= count) {
					animData.layer -= count;
					animIndex = _multData->animIndices[++i];

					count = _vm->_scenery->getAnimLayersCount(animIndex);
				}
				animData.animation = animIndex;

			} else
				animData.isStatic = 1;
		}
	}
}

void Mult_v2::newCycleAnim(Mult_Object &animObj) {
	Mult_AnimData &animData = *(animObj.pAnimData);
	Scenery::AnimLayer *animLayer = 0;

	if (animData.animation >= 0) {
		int nAnim = animData.animation, nLayer = animData.layer;

		if (_vm->_scenery->getAnimLayersCount(nAnim) <= nLayer)
			return;

		animLayer = _vm->_scenery->getAnimLayer(nAnim, nLayer);
	} else {
		if (animObj.videoSlot > 0) {
			_vm->_video->retrace();
			_vm->_vidPlayer->waitEndFrame(animObj.videoSlot - 1, true);
		}
	}

	if (animData.animType == 4) {
		animData.frame = 0;
		animData.isPaused = 1;
		if ((animData.animation < 0) && (animObj.videoSlot > 0)) {
			_vm->_vidPlayer->closeVideo(animObj.videoSlot - 1);
			animObj.videoSlot = 0;
		}
		return;
	}

	if (animData.animType == 12)
		animData.animType = 11;

	if (animData.animType == 11) {
		if (animData.isBusy != 0)
			warning("Woodruff Stub: AnimType 11");
		return;
	}

	if (animData.animType != 8)
		animData.frame++;

	if (animData.animation < 0) {
		if ((animObj.videoSlot > 0) &&
		    ((_vm->_vidPlayer->getCurrentFrame(animObj.videoSlot - 1) + 1) <
		      _vm->_vidPlayer->getFrameCount(animObj.videoSlot - 1))) {
			animData.newCycle = 0;
			return;
		}
	} else {
		if (animData.frame < animLayer->framesCount) {
			animData.newCycle = 0;
			return;
		}
	}


	switch (animData.animType) {
	case 0:
		animData.frame = 0;
		break;

	case 1:
		animData.frame = 0;
		*(animObj.pPosX) += animLayer->animDeltaX;
		*(animObj.pPosY) += animLayer->animDeltaY;
		break;

	case 2:
		animData.frame = 0;
		animData.animation = animData.newAnimation;
		animData.layer = animData.newLayer;
		break;

	case 3:
		animData.animType = 4;
		animData.frame = 0;
		break;

	case 5:
		animData.isStatic = 1;
		animData.frame = 0;
		if ((animData.animation < 0) && (animObj.videoSlot > 0)) {
			_vm->_vidPlayer->closeVideo(animObj.videoSlot - 1);
			animObj.videoSlot = 0;
		}

		break;

	case 6:
	case 7:
		animData.frame--;
		animData.isPaused = 1;
/*
		if ((animData.animation < 0) && (animObj.videoSlot > 0)) {
			if (_vm->_vidPlayer->getFlags(animObj.videoSlot - 1) & 0x1000) {
				_vm->_vidPlayer->closeVideo(animObj.videoSlot - 1);
				animObj.videoSlot = 0;
			}
		}
*/
		break;

	case 10:
		warning("Woodruff Stub: AnimType 10");
		break;
	}

	animData.newCycle = 1;
}

void Mult_v2::animate() {
	int8 minOrder = 100;
	int8 maxOrder = 0;
	int8 *orderArray;
	int orderArrayPos = 0;
	int8 animIndices[150];
	int numAnims = 0;

	if (!_objects)
		return;

	if (_objCount > 0) {
		if (!_orderArray)
			return;
		orderArray = _orderArray;
	} else
		orderArray = 0;

	advanceAllObjects();

	// Find relevant objects
	for (int i = 0; i < _objCount; i++) {
		Mult_Object &animObj = _objects[i];
		Mult_AnimData &animData = *(animObj.pAnimData);

		if (_vm->_map->_mapUnknownBool) {
			// TODO!
		}

		animData.intersected = 200;
		if (animData.isStatic != 2) {
			if ((animData.isStatic == 0) || (animObj.lastLeft != -1)) {
				animIndices[numAnims] = i;
				_renderObjs[numAnims] = &animObj;
				numAnims++;
			}
		}
	}

	// Find dirty areas
	for (int i = 0; i < numAnims; i++) {
		Mult_Object &animObj = *_renderObjs[i];
		Mult_AnimData &animData = *(animObj.pAnimData);

		animObj.needRedraw = 0;
		animObj.newTop     = 1000;
		animObj.newLeft    = 1000;
		animObj.newBottom  = 0;
		animObj.newRight   = 0;

		if (animData.isStatic == 2)
			continue;

		if (!animData.isStatic && !animData.isPaused &&
				(animData.maxTick == animObj.tick)) {

			animObj.needRedraw = 1;
			_vm->_scenery->updateAnim(animData.layer, animData.frame,
					animData.animation, 8, *animObj.pPosX, *animObj.pPosY, 0);
			if (animObj.lastLeft == -1) {
				animObj.newLeft = _vm->_scenery->_toRedrawLeft;
				animObj.newTop = _vm->_scenery->_toRedrawTop;
				animObj.newRight = _vm->_scenery->_toRedrawRight;
				animObj.newBottom = _vm->_scenery->_toRedrawBottom;
			} else {
				animObj.newLeft =
					MIN(animObj.lastLeft, _vm->_scenery->_toRedrawLeft);
				animObj.newTop =
					MIN(animObj.lastTop, _vm->_scenery->_toRedrawTop);
				animObj.newRight =
					MAX(animObj.lastRight, _vm->_scenery->_toRedrawRight);
				animObj.newBottom =
					MAX(animObj.lastBottom, _vm->_scenery->_toRedrawBottom);

				if ((_vm->_game->_script->getVersionMinor() > 2) &&
						(animObj.newLeft == animObj.lastLeft) &&
						(animObj.newTop == animObj.lastTop) &&
						(animObj.newRight == animObj.lastRight) &&
						(animObj.newBottom == animObj.lastBottom) &&
						(animData.redrawLayer == animData.layer) &&
						(animData.redrawFrame == animData.frame) &&
						(animData.redrawAnimation == animData.animation)) {
					animObj.needRedraw = 0;
				}
			}

		} else if (!animData.isStatic) {

			if (animObj.lastLeft == -1) {
				animObj.needRedraw = 1;
				_vm->_scenery->updateAnim(animData.layer, animData.frame,
					animData.animation, 8, *animObj.pPosX, *animObj.pPosY, 0);

				animObj.newLeft = _vm->_scenery->_toRedrawLeft;
				animObj.newTop = _vm->_scenery->_toRedrawTop;
				animObj.newRight = _vm->_scenery->_toRedrawRight;
				animObj.newBottom = _vm->_scenery->_toRedrawBottom;
			} else {
				animObj.newLeft = animObj.lastLeft;
				animObj.newTop = animObj.lastTop;
				animObj.newRight = animObj.lastRight;
				animObj.newBottom = animObj.lastBottom;
			}

		} else if (animObj.lastLeft != -1) {
			animObj.needRedraw = 1;
			animObj.newLeft = animObj.lastLeft;
			animObj.newTop = animObj.lastTop;
			animObj.newRight = animObj.lastRight;
			animObj.newBottom = animObj.lastBottom;
		}

		animData.redrawLayer = animData.layer;
		animData.redrawFrame = animData.frame;
		animData.redrawAnimation = animData.animation;
		if (animObj.needRedraw || !animData.isStatic) {
			minOrder = MIN(minOrder, animData.order);
			maxOrder = MAX(maxOrder, animData.order);
		}
	}

	// Restore dirty areas
	for (int i = 0; i < numAnims; i++) {
		Mult_Object &animObj = *_renderObjs[i];

		if (!animObj.needRedraw || (animObj.lastLeft == -1))
			continue;

		animObj.lastLeft = -1;

		int maxleft = MAX(animObj.newLeft, _animLeft);
		int maxtop = MAX(animObj.newTop, _animTop);
		int right = animObj.newRight - maxleft + 1;
		int bottom = animObj.newBottom - maxtop + 1;

		if ((right <= 0) || (bottom <= 0))
			continue;

		_vm->_draw->_sourceSurface = Draw::kAnimSurface;
		_vm->_draw->_destSurface = Draw::kBackSurface;
		_vm->_draw->_spriteLeft = maxleft - _animLeft;
		_vm->_draw->_spriteTop = maxtop - _animTop;
		_vm->_draw->_spriteRight = right;
		_vm->_draw->_spriteBottom = bottom;
		_vm->_draw->_destSpriteX = maxleft;
		_vm->_draw->_destSpriteY = maxtop;
		_vm->_draw->_transparency = 0;
		_vm->_draw->spriteOperation(DRAW_BLITSURF);
	}

	// Figure out the correct drawing order
	for (int i = minOrder; i <= maxOrder; i++) {
		for (int j = 0; j < numAnims; j++) {
			Mult_Object &animObj = *_renderObjs[j];
			Mult_AnimData &animData = *(animObj.pAnimData);

			if (animData.order == i)
				if (animObj.needRedraw || !animData.isStatic)
					orderArray[orderArrayPos++] = j;
		}
	}

	// Put the goblins in correct drawing order as well
	if (_vm->_goblin->_gobsCount >= 0) {
		for (int i = 0; i < orderArrayPos; i++) {
			Mult_Object &animObj1 = *_renderObjs[orderArray[i]];
			Mult_AnimData &animData1 = *(animObj1.pAnimData);

			if (!animObj1.goblinStates)
				continue;

			for (int j = i+1; j < orderArrayPos; j++) {
				Mult_Object &animObj2 = *_renderObjs[orderArray[j]];
				Mult_AnimData &animData2 = *(animObj2.pAnimData);

				if ((animData1.order == animData2.order) &&
						((animObj1.newBottom > animObj2.newBottom) ||
						((animObj1.newBottom == animObj2.newBottom) &&
						 (animData1.isBusy == 1))))
						SWAP(orderArray[i], orderArray[j]);
			}
		}
	}

	// Update view
	for (int i = 0; i < orderArrayPos; i++) {
		Mult_Object &animObj1 = *_renderObjs[orderArray[i]];
		Mult_AnimData &animData1 = *(animObj1.pAnimData);

		if (!animObj1.needRedraw) {

			if (!animData1.isStatic) {
				for (int j = 0; j < orderArrayPos; j++) {
					Mult_Object &animObj2 = *_renderObjs[orderArray[j]];

					if (!animObj2.needRedraw)
						continue;

					if ((animObj1.newRight >= animObj2.newLeft) &&
							(animObj2.newRight >= animObj1.newLeft) &&
							(animObj1.newBottom >= animObj2.newTop) &&
							(animObj2.newBottom >= animObj1.newTop)) {

						_vm->_scenery->_toRedrawLeft = animObj2.newLeft;
						_vm->_scenery->_toRedrawRight = animObj2.newRight;
						_vm->_scenery->_toRedrawTop = animObj2.newTop;
						_vm->_scenery->_toRedrawBottom = animObj2.newBottom;

						_vm->_scenery->updateAnim(animData1.layer, animData1.frame,
								animData1.animation, 12, *animObj1.pPosX, *animObj1.pPosY, 1);
						_vm->_scenery->updateStatic(animData1.order + 1);
					}
				}
			}

		} else {

			if (animData1.isStatic != 0) {
				_vm->_scenery->_toRedrawLeft = animObj1.newLeft;
				_vm->_scenery->_toRedrawRight = animObj1.newRight;
				_vm->_scenery->_toRedrawTop = animObj1.newTop;
				_vm->_scenery->_toRedrawBottom = animObj1.newBottom;
			} else {
				_vm->_scenery->updateAnim(animData1.layer, animData1.frame,
						animData1.animation, 10, *animObj1.pPosX, *animObj1.pPosY, 1);

				if (_vm->_scenery->_toRedrawLeft != -12345) {
					animObj1.lastLeft = _vm->_scenery->_toRedrawLeft;
					animObj1.lastRight = _vm->_scenery->_toRedrawRight;
					animObj1.lastTop = _vm->_scenery->_toRedrawTop;
					animObj1.lastBottom = _vm->_scenery->_toRedrawBottom;
				} else {
					animObj1.lastLeft = -1;
				}

			}

			_vm->_scenery->updateStatic(animData1.order + 1);

		}

	}

	// Advance animations
	for (int i = 0; i < numAnims; i++) {
		Mult_Object &animObj = *_renderObjs[i];
		Mult_AnimData &animData = *(animObj.pAnimData);

		if (animData.isStatic)
			continue;

		if ((animData.animType == 7) && (animData.newState != -1)) {
			animData.layer = animData.newState;
			animData.frame = 0;
			animData.newState = -1;
			animData.isPaused = 0;
		}
		if (animData.isPaused)
			continue;

		if (animData.maxTick == animObj.tick) {
			animObj.tick = 0;
			if ((animData.animType < 100) || (_vm->_goblin->_gobsCount < 0))
				newCycleAnim(animObj);
			else if (animData.animType == 100)
				_vm->_goblin->moveAdvance(&animObj, 0, 0, 0);
			else if (animData.animType == 101)
				_vm->_goblin->animate(&animObj);
		} else
			animObj.tick++;
	}

	// Find intersections
	for (int i = 0; i < numAnims; i++) {
		Mult_Object &animObj1 = *_renderObjs[i];
		Mult_AnimData &animData1 = *(animObj1.pAnimData);

		if (animData1.isStatic || (animObj1.lastLeft == -1))
			continue;

		for (int j = 0; j < numAnims; j++) {
			Mult_Object &animObj2 = *_renderObjs[j];
			Mult_AnimData &animData2 = *(animObj2.pAnimData);

			if (i == j)
				continue;
			if ((animData2.isStatic) || (animObj2.lastLeft == -1))
				continue;

			if ((animObj2.lastRight >= animObj1.lastLeft) &&
			    (animObj2.lastLeft <= animObj1.lastRight) &&
			    (animObj2.lastBottom >= animObj1.lastTop) &&
			    (animObj2.lastTop <= animObj1.lastBottom))
				animData2.intersected = animIndices[i];
		}
	}

}

void Mult_v2::playImd(const char *imdFile, Mult::Mult_ImdKey &key, int16 dir,
		int16 startFrame) {

	VideoPlayer::Properties props;

	if (_vm->_draw->_renderFlags & 0x100) {
		props.x = VAR(55);
		props.y = VAR(56);
	}

	if (key.imdFile == -1) {
		_vm->_vidPlayer->closeVideo();
		return;
	}

	props.flags = (key.flags >> 8) & 0xFF;
	if (props.flags & 0x20)
		props.flags = (props.flags & 0x9F) | 0x80;

	props.palStart  = key.palStart;
	props.palEnd    = key.palEnd;
	props.palFrame  = key.palFrame;
	props.lastFrame = key.lastFrame;

	if ((props.palFrame != -1) && (props.lastFrame != -1))
		if ((props.lastFrame - props.palFrame) < props.startFrame)
			if (!(key.flags & 0x4000)) {
				_vm->_vidPlayer->closeVideo();
				return;
			}

	_vm->_vidPlayer->evaluateFlags(props);

	int slot;
	if ((slot = _vm->_vidPlayer->openVideo(true, imdFile, props)) < 0)
		return;

	if (props.palFrame == -1)
		props.palFrame = 0;

	if (props.lastFrame == -1)
		props.lastFrame = _vm->_vidPlayer->getFrameCount() - 1;

	uint32 baseFrame = startFrame % (props.lastFrame - props.palFrame + 1);

	props.endFrame   = props.lastFrame;
	props.startFrame = baseFrame + props.palFrame;
	props.lastFrame  = baseFrame + props.palFrame;

	props.flags &= 0x7F;

	debugC(2, kDebugVideo, "Playing mult video \"%s\" @ %d+%d, frame %d, "
			"paletteCmd %d (%d - %d; %d), flags %X", imdFile,
			props.x, props.y, props.startFrame,
			props.palCmd, props.palStart, props.palEnd, props.endFrame, props.flags);

	_vm->_vidPlayer->play(slot, props);
}

void Mult_v2::advanceObjects(int16 index) {
	int16 frame;
	bool stop = false;

	frame = _multData->animKeysFrames[index];
	if (frame == -1)
		return;

	for (int i = 0; i < 4; i++) {
		int obj = _multData->animObjs[index][i];

		if ((obj != -1) && (obj != 1024)) {
			int keyIndex = _multData->animKeysIndices[index][i];
			int count = _multData->animKeysCount[i];

			for (int j = keyIndex; j < count; j++) {
				Mult_AnimKey &key = _multData->animKeys[i][j];
				Mult_Object &animObj = _objects[obj];
				Mult_AnimData &animData = *(animObj.pAnimData);

				if (key.frame > frame)
					break;
				else if (key.frame < frame)
					continue;

				if (key.layer > -1) {
					int16 layer;
					int16 layers;
					int16 curAnim;

					_multData->animKeysIndices[index][i] = j;
					*(animObj.pPosX) = key.posX;
					*(animObj.pPosY) = key.posY;
					animData.frame = 0;
					animData.animType = 1;
					animData.isStatic = 0;
					animData.isPaused = 0;
					animData.maxTick = 0;
					animData.animation = 0;
					animObj.tick = 0;

					curAnim = _multData->animIndices[0];
					layer = key.layer;
					layers = _vm->_scenery->getAnimLayersCount(curAnim);
					while (layer >= layers) {
						layer -= layers;
						animData.animation++;
						curAnim = _multData->animIndices[animData.animation];
						layers = _vm->_scenery->getAnimLayersCount(curAnim);
					}
					animData.layer = layer;
					animData.animation =
						_multData->animIndices[animData.animation];
					break;
				} else
					animData.isStatic = 1;
			}
		}

		if (obj != -1) {
			int keyIndex = _multData->imdKeysIndices[index][i];
			int count = _multData->imdKeysCount[i];

			for (int j = keyIndex; j < count; j++) {
				Mult_ImdKey &key1 = _multData->imdKeys[i][j];
				Mult_ImdKey &key2 = _multData->imdKeys[i][j - 1];

				if (key1.frame > frame)
					break;
				else if (key1.frame < frame)
					continue;

				if (key1.imdFile != -1) {
					_multData->imdIndices[0] = -1;
					_multData->imdIndices[1] = -1;
					_multData->imdIndices[2] = -1;
					_multData->imdIndices[3] = -1;
					if ((_multData->animDirection == 1) || (key2.imdFile == -1))
						_multData->imdIndices[i] = j;
					else if (_multData->animKeysStopFrames[index] == frame)
						_multData->imdIndices[i] = -1;
					else
						_multData->imdIndices[i] = j - 1;
				} else
					_multData->imdIndices[i] = -1;
			}
		}

		if (_multData->imdIndices[i] != -1) {
			int fileN;
			char *imdFile;
			int dir;
			int startFrame;

			Mult_ImdKey &key = _multData->imdKeys[i][_multData->imdIndices[i]];

			fileN = -key.imdFile - 2;
			if (fileN < 0)
				return;

			imdFile = _multData->imdFiles + fileN * 14;
			dir = _multData->animDirection;
			startFrame = frame - key.frame;

			if ((dir != 1) && (--startFrame < 0))
				startFrame = 0;

			playImd(imdFile, key, dir, startFrame);
		}
	}

	doSoundAnim(stop, frame);
	WRITE_VAR(22, frame);

	if (_multData->animKeysStopFrames[index] == frame) {
		_multData->imdIndices[0] = -1;
		_multData->imdIndices[1] = -1;
		_multData->imdIndices[2] = -1;
		_multData->imdIndices[3] = -1;
		frame = -1;
		for (int i = 0; i < 4; i++) {
			int obj = _multData->animObjs[index][i];

			if ((obj == -1) || (obj == 1024))
				continue;

			Mult_Object &animObj = _objects[_multData->animObjs[index][i]];
			animObj.pAnimData->animType = animObj.pAnimData->animTypeBak;
		}
	} else if (_multData->animDirection == 1)
		frame++;
	else
		frame--;

	_multData->animKeysFrames[index] = frame;
	WRITE_VAR(18 + index, frame);
}

void Mult_v2::advanceAllObjects() {
	Mult_Data *multData = _multData;

	for (int i = 0; i < 8; i++) {
		if (_multDatas[i]) {
			_multData = _multDatas[i];
			for (int j = 0; j < 4; j++)
				advanceObjects(j);
		}
	}

	_multData = multData;
}

} // End of namespace Gob
