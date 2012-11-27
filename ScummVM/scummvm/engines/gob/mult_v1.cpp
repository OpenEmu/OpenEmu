/*  ScummVM - Graphic Adventure Engine
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
#include "gob/inter.h"
#include "gob/scenery.h"

namespace Gob {

Mult_v1::Mult_v1(GobEngine *vm) : Mult(vm) {
}

void Mult_v1::loadMult(int16 resId) {
	debugC(4, kDebugGameFlow, "Loading mult");

	_multData = new Mult_Data;
	memset(_multData, 0, sizeof(Mult_Data));

	_multData->sndSlotsCount = 0;
	_multData->frameStart = 0;

	Resource *resource = _vm->_game->_resources->getResource(resId);
	if (!resource)
		return;

	Common::SeekableReadStream &data = *resource->stream();

	_multData->staticCount = data.readSByte() + 1;
	_multData->animCount = data.readSByte() + 1;

	for (int i = 0; i < 10; i++) {
		_multData->staticLoaded[i] = false;
		_multData->animLoaded[i] = false;
	}

	for (int i = 0; i < _multData->staticCount; i++, data.seek(14, SEEK_CUR)) {
		_multData->staticIndices[i] = _vm->_scenery->loadStatic(1);

		if (_multData->staticIndices[i] >= 100) {
			_multData->staticIndices[i] -= 100;
			_multData->staticLoaded[i] = true;
		}
	}

	for (int i = 0; i < _multData->animCount; i++, data.seek(14, SEEK_CUR)) {
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
		data.read(_multData->textKeys[i].unknown, 18);
		data.read(_multData->textKeys[i].script, 6);
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
		data.seek(26, SEEK_CUR);
		switch (_multData->sndKeys[i].cmd) {
		case 1:
		case 4:
			_multData->sndKeys[i].resId = _vm->_game->_script->peekUint16();

			for (j = 0; j < i; j++) {
				if (_multData->sndKeys[i].resId ==
				    _multData->sndKeys[j].resId) {
					_multData->sndKeys[i].soundIndex =
					    _multData->sndKeys[j].soundIndex;
					_vm->_game->_script->skip(2);
					break;
				}
			}
			if (i == j) {
				_vm->_inter->loadSound(19 - _multData->sndSlotsCount);
				_multData->sndKeys[i].soundIndex =
				    19 - _multData->sndSlotsCount;
				_multData->sndSlotsCount++;
			}
			break;

		case 3:
			_vm->_game->_script->skip(6);
			break;

		case 5:
			_vm->_game->_script->skip(_multData->sndKeys[i].freq * 2);
			break;
		}
	}

	delete resource;
}

void Mult_v1::freeMultKeys() {
	for (int i = 0; i < _multData->staticCount; i++)
		if (_multData->staticLoaded[i])
			_vm->_scenery->freeStatic(_multData->staticIndices[i]);

	for (int i = 0; i < _multData->animCount; i++)
		if (_multData->animLoaded[i])
			_vm->_scenery->freeAnim(_multData->animIndices[i]);

	delete[] _multData->staticKeys;

	for (int i = 0; i < 4; i++)
		delete[] _multData->animKeys[i];

	delete[] _multData->palFadeKeys;
	delete[] _multData->palKeys;
	delete[] _multData->textKeys;

	for (int i = 0; i < _multData->sndSlotsCount; i++)
		_vm->_game->freeSoundSlot(19 - i);

	delete[] _multData->sndKeys;

	if (_animDataAllocated) {
		clearObjectVideos();

		if (_objects)
			for (int i = 0; i < _objCount; i++) {
				delete _objects[i].pPosX;
				delete _objects[i].pPosY;
			}

		delete[] _objects;
		delete[] _renderData;
		delete _animArrayX;
		delete _animArrayY;
		delete[] _animArrayData;

		_objects = 0;
		_renderData = 0;
		_animArrayX = 0;
		_animArrayY = 0;
		_animArrayData = 0;

		_animSurf.reset();
		_vm->_draw->freeSprite(Draw::kAnimSurface);

		_animDataAllocated = false;
	}

	delete _multData;
	_multData = 0;
}

bool Mult_v1::hasMultData(uint16 multIndex) {
	error("Switching mults not supported for Gob1");
}

void Mult_v1::setMultData(uint16 multIndex) {
	error("Switching mults not supported for Gob1");
}

void Mult_v1::zeroMultData(uint16 multIndex) {
	error("Switching mults not supported for Gob1");
}

void Mult_v1::multSub(uint16 multIndex) {
	error("Switching mults not supported for Gob1");
}

void Mult_v1::playMultInit() {
	_doPalSubst = false;
	_palFadingRed = 0;
	_palFadingGreen = 0;
	_palFadingBlue = 0;

	_oldPalette = _vm->_global->_pPaletteDesc->vgaPal;

	if (!_animSurf) {
		if (_objects)
			for (int i = 0; i < _objCount; i++) {
				delete _objects[i].pPosX;
				delete _objects[i].pPosY;
			}

		delete[] _objects;

		_vm->_util->setFrameRate(_multData->frameRate);
		_animTop = 0;
		_animLeft = 0;
		_animWidth = 320;
		_animHeight = 200;
		_objCount = 4;

		delete[] _renderData;
		delete _animArrayX;
		delete _animArrayY;
		delete[] _animArrayData;

		_objects = new Mult_Object[_objCount];
		_renderData = new int16[9 * _objCount];
		_animArrayX = new VariablesLE(_objCount * 4);
		_animArrayY = new VariablesLE(_objCount * 4);
		_animArrayData = new Mult_AnimData[_objCount];

		memset(_objects, 0, _objCount * sizeof(Mult_Object));
		memset(_renderData, 0, _objCount * 9 * sizeof(int16));
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

		_animSurf = _vm->_video->initSurfDesc(320, 200);
		_vm->_draw->_spritesArray[Draw::kAnimSurface] = _animSurf;

		_animSurf->blit(*_vm->_draw->_backSurface, 0, 0, 319, 199, 0, 0);

		_animDataAllocated = true;
	} else
		_animDataAllocated = false;

	_frame = 0;
}

void Mult_v1::drawStatics(bool &stop) {
	if (_multData->staticKeys[_multData->staticKeysCount - 1].frame > _frame)
		stop = false;

	for (_counter = 0; _counter < _multData->staticKeysCount; _counter++) {
		if ((_multData->staticKeys[_counter].frame != _frame)
		    || (_multData->staticKeys[_counter].layer == -1))
			continue;

		_vm->_scenery->_curStaticLayer = _multData->staticKeys[_counter].layer;
		for (_vm->_scenery->_curStatic = 0;
				_vm->_scenery->_curStaticLayer >=
					_vm->_scenery->getStaticLayersCount(_multData->staticIndices[_vm->_scenery->_curStatic]);
				_vm->_scenery->_curStatic++) {
			_vm->_scenery->_curStaticLayer -=
					_vm->_scenery->getStaticLayersCount(_multData->staticIndices[_vm->_scenery->_curStatic]);
		}

		_vm->_scenery->_curStatic = _multData->staticIndices[_vm->_scenery->_curStatic];
		_vm->_scenery->renderStatic(_vm->_scenery->_curStatic, _vm->_scenery->_curStaticLayer);
		_animSurf->blit(*_vm->_draw->_backSurface, 0, 0, 319, 199, 0, 0);
	}
}

void Mult_v1::drawAnims(bool &stop) {
	int16 count;
	int animIndex;

	for (_index = 0; _index < 4; _index++) {
		for (_counter = 0; _counter < _multData->animKeysCount[_index]; _counter++) {
			Mult_AnimKey &key = _multData->animKeys[_index][_counter];
			Mult_Object &animObj = _objects[_index];
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

void Mult_v1::newCycleAnim(Mult_Object &animObj) {
	Mult_AnimData &animData = *(animObj.pAnimData);
	int nAnim = animData.animation;
	int nLayer = animData.layer;
	Scenery::AnimLayer *animLayer = _vm->_scenery->getAnimLayer(nAnim, nLayer);

	animData.frame++;

	if (animData.frame < animLayer->framesCount) {
		animData.newCycle = 0;
		return;
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
		animData.animation =
				animData.newAnimation;
		animData.layer =
				animData.newLayer;
		break;

	case 3:
		animData.animType = 4;
		animData.frame = 0;
		break;

	case 5:
		animData.isStatic = 1;
		animData.frame = 0;
		break;

	case 6:
		animData.frame--;
		animData.isPaused = 1;
		break;
	}
	animData.newCycle = 1;
}

void Mult_v1::animate() {
	int16 minOrder;
	int16 maxOrder;
	int16 *pCurLefts, *pCurRights, *pCurTops, *pCurBottoms;
	int16 *pDirtyLefts, *pDirtyRights, *pDirtyTops, *pDirtyBottoms;
	int16 *pNeedRedraw;
	Mult_AnimData *pAnimData;
	int16 i, j;
	int16 order;

	if (!_renderData)
		return;

	pDirtyLefts = _renderData;
	pDirtyRights = pDirtyLefts + _objCount;
	pDirtyTops = pDirtyRights + _objCount;
	pDirtyBottoms = pDirtyTops + _objCount;
	pNeedRedraw = pDirtyBottoms + _objCount;
	pCurLefts = pNeedRedraw + _objCount;
	pCurRights = pCurLefts + _objCount;
	pCurTops = pCurRights + _objCount;
	pCurBottoms = pCurTops + _objCount;
	minOrder = 100;
	maxOrder = 0;

	// Find dirty areas
	for (i = 0; i < _objCount; i++) {
		Mult_Object &animObj = _objects[i];
		Mult_AnimData &animData = *(animObj.pAnimData);

		pNeedRedraw[i] = 0;
		pDirtyTops[i] = 1000;
		pDirtyLefts[i] = 1000;
		pDirtyBottoms[i] = 1000;
		pDirtyRights[i] = 1000;

		if (!animData.isStatic && !animData.isPaused &&
		    (animObj.tick == animData.maxTick)) {
			if (animData.order < minOrder)
				minOrder = animData.order;

			if (animData.order > maxOrder)
				maxOrder = animData.order;

			pNeedRedraw[i] = 1;
			_vm->_scenery->updateAnim(animData.layer, animData.frame,
				animData.animation, 0, *(animObj.pPosX), *(animObj.pPosY), 0);

			if (animObj.lastLeft != -1) {
				pDirtyLefts[i] =
				  MIN(animObj.lastLeft, _vm->_scenery->_toRedrawLeft);
				pDirtyTops[i] =
				  MIN(animObj.lastTop, _vm->_scenery->_toRedrawTop);
				pDirtyRights[i] =
				  MAX(animObj.lastRight, _vm->_scenery->_toRedrawRight);
				pDirtyBottoms[i] =
				  MAX(animObj.lastBottom, _vm->_scenery->_toRedrawBottom);
			} else {
				pDirtyLefts[i] = _vm->_scenery->_toRedrawLeft;
				pDirtyTops[i] = _vm->_scenery->_toRedrawTop;
				pDirtyRights[i] = _vm->_scenery->_toRedrawRight;
				pDirtyBottoms[i] = _vm->_scenery->_toRedrawBottom;
			}
			pCurLefts[i] = _vm->_scenery->_toRedrawLeft;
			pCurRights[i] = _vm->_scenery->_toRedrawRight;
			pCurTops[i] = _vm->_scenery->_toRedrawTop;
			pCurBottoms[i] = _vm->_scenery->_toRedrawBottom;
		} else {
			if (animObj.lastLeft != -1) {
				if (animData.order < minOrder)
					minOrder = animData.order;

				if (animData.order > maxOrder)
					maxOrder = animData.order;

				if (animData.isStatic)
					*pNeedRedraw = 1;

				pCurLefts[i] = animObj.lastLeft;
				pDirtyLefts[i] = animObj.lastLeft;

				pCurTops[i] = animObj.lastTop;
				pDirtyTops[i] = animObj.lastTop;

				pCurRights[i] = animObj.lastRight;
				pDirtyRights[i] = animObj.lastRight;

				pCurBottoms[i] = animObj.lastBottom;
				pDirtyBottoms[i] = animObj.lastBottom;
			}
		}
	}

	// Find intersections
	for (i = 0; i < _objCount; i++) {
	Mult_AnimData &animData = *(_objects[i].pAnimData);
		animData.intersected = 200;

		if (animData.isStatic)
			continue;

		for (j = 0; j < _objCount; j++) {
			if (i == j)
				continue;

			if (_objects[j].pAnimData->isStatic)
				continue;

			if (pCurRights[i] < pCurLefts[j])
				continue;

			if (pCurRights[j] < pCurLefts[i])
				continue;

			if (pCurBottoms[i] < pCurTops[j])
				continue;

			if (pCurBottoms[j] < pCurTops[i])
				continue;

			animData.intersected = j;
			break;
		}
	}

	// Restore dirty areas
	for (i = 0; i < _objCount; i++) {

		if ((pNeedRedraw[i] == 0) || (_objects[i].lastLeft == -1))
			continue;

		_vm->_draw->_sourceSurface = Draw::kAnimSurface;
		_vm->_draw->_destSurface = Draw::kBackSurface;
		_vm->_draw->_spriteLeft = pDirtyLefts[i] - _animLeft;
		_vm->_draw->_spriteTop = pDirtyTops[i] - _animTop;
		_vm->_draw->_spriteRight = pDirtyRights[i] - pDirtyLefts[i] + 1;
		_vm->_draw->_spriteBottom = pDirtyBottoms[i] - pDirtyTops[i] + 1;
		_vm->_draw->_destSpriteX = pDirtyLefts[i];
		_vm->_draw->_destSpriteY = pDirtyTops[i];
		_vm->_draw->_transparency = 0;
		_vm->_draw->spriteOperation(DRAW_BLITSURF);
		_objects[i].lastLeft = -1;
	}

	// Update view
	for (order = minOrder; order <= maxOrder; order++) {
		for (i = 0; i < _objCount; i++) {
			Mult_Object &animObj = _objects[i];
			Mult_AnimData &animData = *(animObj.pAnimData);

			if (animData.order != order)
				continue;

			if (pNeedRedraw[i]) {
				if (!animData.isStatic) {
					_vm->_scenery->updateAnim(animData.layer, animData.frame,
						animData.animation, 2, *(animObj.pPosX),
						*(animObj.pPosY), 1);

					if (_vm->_scenery->_toRedrawLeft != -12345) {
						animObj.lastLeft = _vm->_scenery->_toRedrawLeft;
						animObj.lastTop = _vm->_scenery->_toRedrawTop;
						animObj.lastRight = _vm->_scenery->_toRedrawRight;
						animObj.lastBottom = _vm->_scenery->_toRedrawBottom;
					} else
						animObj.lastLeft = -1;
				}
				_vm->_scenery->updateStatic(order + 1);
			} else if (!animData.isStatic) {
				for (j = 0; j < _objCount; j++) {
					if (pNeedRedraw[j] == 0)
						continue;

					if (pDirtyRights[i] < pDirtyLefts[j])
						continue;

					if (pDirtyRights[j] < pDirtyLefts[i])
						continue;

					if (pDirtyBottoms[i] < pDirtyTops[j])
						continue;

					if (pDirtyBottoms[j] < pDirtyTops[i])
						continue;

					_vm->_scenery->_toRedrawLeft = pDirtyLefts[j];
					_vm->_scenery->_toRedrawRight = pDirtyRights[j];
					_vm->_scenery->_toRedrawTop = pDirtyTops[j];
					_vm->_scenery->_toRedrawBottom = pDirtyBottoms[j];

					_vm->_scenery->updateAnim(animData.layer, animData.frame,
						animData.animation, 4, *(animObj.pPosX),
						*(animObj.pPosY), 1);

					_vm->_scenery->updateStatic(order + 1);
				}
			}
		}
	}

	// Advance animations
	for (i = 0; i < _objCount; i++) {
		pAnimData = _objects[i].pAnimData;
		if (pAnimData->isStatic || pAnimData->isPaused)
			continue;

		if (_objects[i].tick == pAnimData->maxTick) {
			_objects[i].tick = 0;
			if (pAnimData->animType == 4) {
				pAnimData->isPaused = 1;
				pAnimData->frame = 0;
			} else
				newCycleAnim(_objects[i]);
		} else
			_objects[i].tick++;
	}
}

} // End of namespace Gob
