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

#include "gob/gob.h"
#include "gob/mult.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/palanim.h"
#include "gob/scenery.h"
#include "gob/video.h"
#include "gob/videoplayer.h"
#include "gob/inter.h"
#include "gob/sound/sound.h"

namespace Gob {

Mult::Mult(GobEngine *vm) : _vm(vm) {
	_multData = 0;

	_frame = 0;

	_objCount = 0;
	_objects = 0;

	_renderData = 0;
	_renderObjs = 0;

	_orderArray = 0;

	_index = 0;
	_counter = 0;
	_animDataAllocated = false;

	for (int i = 0; i < 8; i++)
		_multDatas[i] = 0;

	_doPalSubst = false;

	_animArrayX = 0;
	_animArrayY = 0;
	_animArrayData = 0;

	_palKeyIndex = 0;
	_oldPalette = 0;
	for (int i = 0; i < 256; i++) {
		_palAnimPalette[i].red = 0;
		_palAnimPalette[i].green = 0;
		_palAnimPalette[i].blue = 0;
	}

	_palAnimKey = 0;
	for (int i = 0; i < 4; i++) {
		_palAnimRed[i] = 0;
		_palAnimGreen[i] = 0;
		_palAnimBlue[i] = 0;
	}

	_palFadingRed = 0;
	_palFadingGreen = 0;
	_palFadingBlue = 0;

	_animLeft = 0;
	_animTop = 0;
	_animWidth = 0;
	_animHeight = 0;
}

Mult::~Mult() {
	if (_objects)
		for (int i = 0; i < _objCount; i++) {
			delete _objects[i].pPosX;
			delete _objects[i].pPosY;
		}

	delete[] _objects;
	delete[] _orderArray;
	delete[] _renderData;
	delete[] _renderObjs;
	delete _animArrayX;
	delete _animArrayY;
	delete[] _animArrayData;
	delete _multData;
}

void Mult::initAll() {
	_objects = 0;
	_animSurf.reset();
	_renderData = 0;

	_vm->_scenery->init();
}

void Mult::freeAll() {
	freeMult();

	for (int i = 0; i < 10; i++) {
		_vm->_scenery->freeAnim(i);
		_vm->_scenery->freeStatic(i);
	}
}

void Mult::freeMult() {
	clearObjectVideos();

	if (_objects)
		for (int i = 0; i < _objCount; i++) {
			delete _objects[i].pPosX;
			delete _objects[i].pPosY;
		}

	delete[] _objects;
	delete[] _renderData;
	delete[] _renderObjs;
	delete[] _orderArray;

	_objects = 0;
	_renderData = 0;
	_renderObjs = 0;
	_orderArray = 0;

	_animSurf.reset();
	_vm->_draw->freeSprite(Draw::kAnimSurface);
}

void Mult::checkFreeMult() {
	if (_multData)
		freeMultKeys();
}

void Mult::zeroMultData() {
	_multData = 0;
}

void Mult::playMult(int16 startFrame, int16 endFrame, char checkEscape,
	    char handleMouse) {
	bool stopNoClear;
	bool stop;

	if (!_multData)
		return;

	stopNoClear = false;
	_frame = startFrame;
	if (endFrame == -1)
		endFrame = 32767;

	if (_frame == -1)
		playMultInit();

	do {
		stop = true;

		if (VAR(58) == 0) {
			drawStatics(stop);
			drawAnims(stop);
		}

		animate();
		if (handleMouse)
			_vm->_draw->animateCursor(-1);
		else
			_vm->_draw->blitInvalidated();

		if (VAR(58) == 0)
			drawText(stop, stopNoClear);

		prepPalAnim(stop);
		doPalAnim();

		doFadeAnim(stop);
		doSoundAnim(stop, _frame);

		if (_frame >= endFrame)
			stopNoClear = true;

		if (_vm->_sound->blasterPlayingSound())
			stop = false;

		_vm->_util->processInput();
		if (checkEscape && (_vm->_util->checkKey() == kKeyEscape))
			stop = true;

		_frame++;
		_vm->_util->waitEndFrame();
	} while (!stop && !stopNoClear && !_vm->shouldQuit());

	if (!stopNoClear) {
		if (_animDataAllocated) {
			clearObjectVideos();

			if (_objects)
				for (int i = 0; i < _objCount; i++) {
					delete _objects[i].pPosX;
					delete _objects[i].pPosY;
				}

			delete[] _objects;
			delete[] _renderData;
			delete[] _renderObjs;
			delete _animArrayX;
			delete _animArrayY;
			delete[] _animArrayData;
			delete[] _orderArray;

			_objects = 0;
			_renderObjs = 0;
			_renderData = 0;
			_animArrayX = 0;
			_animArrayY = 0;
			_animArrayData = 0;
			_orderArray = 0;

			_animSurf.reset();
			_vm->_draw->freeSprite(Draw::kAnimSurface);

			_animDataAllocated = false;
		}

		if (_vm->_sound->blasterPlayingSound())
			_vm->_sound->blasterStop(10);

		WRITE_VAR(57, (uint32) -1);
	} else
		WRITE_VAR(57, _frame - _multData->frameStart - 1);
}

void Mult::drawText(bool &stop, bool &stopNoClear) {
	int16 cmd;
	for (_index = 0; _index < _multData->textKeysCount; _index++) {
		if (_multData->textKeys[_index].frame != _frame)
			continue;

		cmd = _multData->textKeys[_index].cmd;
		if (cmd == 0) {
			stop = false;
		} else if (cmd == 1) {
			stopNoClear = true;
			_multData->frameStart = 0;
		} else if (cmd == 3) {
			warning("Mult::drawText, cmd == 3");
			stop = false;
//			uint32 startPos = _vm->_game->_script->pos();
//			_vm->_global->_inter_execPtr = _multData->textKeys[_index].script;
		}
	}
}

void Mult::prepPalAnim(bool &stop) {
	_palKeyIndex = -1;
	do {
		_palKeyIndex++;
		if (_palKeyIndex >= _multData->palKeysCount)
			return;
	} while (_multData->palKeys[_palKeyIndex].frame != _frame);

	if (_multData->palKeys[_palKeyIndex].cmd == -1) {
		stop = false;
		_doPalSubst = false;
		_vm->_global->_pPaletteDesc->vgaPal = _oldPalette;
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	} else {
		stop = false;
		_doPalSubst = true;
		_palAnimKey = _palKeyIndex;

		_multData->palAnimIndices[0] = 0;
		_multData->palAnimIndices[1] = 0;
		_multData->palAnimIndices[2] = 0;
		_multData->palAnimIndices[3] = 0;

		memcpy((char *)_palAnimPalette,
				(char *)_vm->_global->_pPaletteDesc->vgaPal, 768);
		_vm->_global->_pPaletteDesc->vgaPal = _palAnimPalette;
	}
}

void Mult::doPalAnim() {
	int16 off;
	int16 off2;
	Video::Color *palPtr;
	Mult_PalKey *palKey;

	if (!_doPalSubst)
		return;

	for (_index = 0; _index < 4; _index++) {
		palKey = &_multData->palKeys[_palAnimKey];

		if ((_frame % palKey->rates[_index]) != 0)
			continue;

		_palAnimRed[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].red;
		_palAnimGreen[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].green;
		_palAnimBlue[_index] =
		    _vm->_global->_pPaletteDesc->vgaPal[palKey->subst[0][_index] - 1].blue;

		while (1) {
			off = palKey->subst[(_multData->palAnimIndices[_index] + 1) % 16][_index];
			if (off == 0) {
				off = palKey->subst[_multData->palAnimIndices[_index]][_index] - 1;

				_vm->_global->_pPaletteDesc->vgaPal[off].red = _palAnimRed[_index];
				_vm->_global->_pPaletteDesc->vgaPal[off].green = _palAnimGreen[_index];
				_vm->_global->_pPaletteDesc->vgaPal[off].blue = _palAnimBlue[_index];
			} else {
				off = palKey->subst[(_multData->palAnimIndices[_index] + 1) % 16][_index] - 1;
				off2 = palKey->subst[_multData->palAnimIndices[_index]][_index] - 1;

				_vm->_global->_pPaletteDesc->vgaPal[off2].red =
					_vm->_global->_pPaletteDesc->vgaPal[off].red;
				_vm->_global->_pPaletteDesc->vgaPal[off2].green =
					_vm->_global->_pPaletteDesc->vgaPal[off].green;
				_vm->_global->_pPaletteDesc->vgaPal[off2].blue =
					_vm->_global->_pPaletteDesc->vgaPal[off].blue;
			}

			_multData->palAnimIndices[_index] = (_multData->palAnimIndices[_index] + 1) % 16;

			off = palKey->subst[_multData->palAnimIndices[_index]][_index];

			if (off == 0) {
				_multData->palAnimIndices[_index] = 0;
				off = palKey->subst[0][_index] - 1;

				_palAnimRed[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].red;
				_palAnimGreen[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].green;
				_palAnimBlue[_index] = _vm->_global->_pPaletteDesc->vgaPal[off].blue;
			}
			if (_multData->palAnimIndices[_index] == 0)
				break;
		}
	}

	if (_vm->_global->_colorCount == 256) {
		_vm->_video->waitRetrace();

		palPtr = _vm->_global->_pPaletteDesc->vgaPal;
		for (_counter = 0; _counter < 16; _counter++, palPtr++)
			_vm->_video->setPalElem(_counter, palPtr->red, palPtr->green,
					palPtr->blue, 0, 0x13);

		palPtr = _vm->_global->_pPaletteDesc->vgaPal;
		for (_counter = 0; _counter < 16; _counter++, palPtr++) {
			_vm->_global->_redPalette[_counter] = palPtr->red;
			_vm->_global->_greenPalette[_counter] = palPtr->green;
			_vm->_global->_bluePalette[_counter] = palPtr->blue;
		}

	} else
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
}

void Mult::doFadeAnim(bool &stop) {
	Mult_PalFadeKey *fadeKey;

	for (_index = 0; _index < _multData->palFadeKeysCount; _index++) {
		fadeKey = &_multData->palFadeKeys[_index];

		if (fadeKey->frame != _frame)
			continue;

		stop = false;
		if (!(fadeKey->flag & 1)) {
			if (fadeKey->fade == 0) {
				_vm->_global->_pPaletteDesc->vgaPal =
					_multData->fadePal[fadeKey->palIndex];
				_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			} else {
				_vm->_global->_pPaletteDesc->vgaPal =
					_multData->fadePal[fadeKey->palIndex];
				_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, fadeKey->fade, 0);
			}
		} else {
			_vm->_global->_pPaletteDesc->vgaPal =
				_multData->fadePal[fadeKey->palIndex];
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, fadeKey->fade, -1);

			_palFadingRed = (fadeKey->flag >> 1) & 1;
			_palFadingGreen = (fadeKey->flag >> 2) & 1;
			_palFadingBlue = (fadeKey->flag >> 3) & 1;
		}
	}

	if (_palFadingRed) {
		_palFadingRed = !_vm->_palAnim->fadeStep(1);
		stop = false;
	}
	if (_palFadingGreen) {
		_palFadingGreen = !_vm->_palAnim->fadeStep(2);
		stop = false;
	}
	if (_palFadingBlue) {
		_palFadingBlue = !_vm->_palAnim->fadeStep(3);
		stop = false;
	}
}

void Mult::doSoundAnim(bool &stop, int16 frame) {
	Mult_SndKey *sndKey;
	for (_index = 0; _index < _multData->sndKeysCount; _index++) {
		sndKey = &_multData->sndKeys[_index];
		if (sndKey->frame != frame)
			continue;

		if (sndKey->cmd != -1) {
			if ((sndKey->cmd == 1) || (sndKey->cmd == 4)) {
				SoundDesc *sample = _vm->_sound->sampleGetBySlot(sndKey->soundIndex);

				_vm->_sound->blasterStop(0);
				if (sample && !sample->empty())
					_vm->_sound->blasterPlay(sample, sndKey->repCount,
							sndKey->freq, sndKey->fadeLength);
			}
		} else {
			if (_vm->_sound->blasterPlayingSound())
				_vm->_sound->blasterStop(sndKey->fadeLength);
		}
	}
}

void Mult::clearObjectVideos() {
	if (!_objects)
		return;

	for (int i = 0; i < _objCount; i++)
		if (_objects[i].videoSlot > 0)
			_vm->_vidPlayer->closeVideo(_objects[i].videoSlot - 1);
}

} // End of namespace Gob
