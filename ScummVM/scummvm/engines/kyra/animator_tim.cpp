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

#include "kyra/script_tim.h"
#include "kyra/wsamovie.h"
#include "kyra/screen_lol.h"

#ifdef ENABLE_LOL
#include "kyra/lol.h"
#else
#include "kyra/screen_v2.h"
#endif

#include "common/system.h"

namespace Kyra {

#ifdef ENABLE_LOL
TimAnimator::TimAnimator(LoLEngine *engine, Screen_v2 *screen_v2, OSystem *system, bool useParts) : _vm(engine), _screen(screen_v2), _system(system), _useParts(useParts) {
#else
TimAnimator::TimAnimator(KyraEngine_v1 *engine, Screen_v2 *screen_v2, OSystem *system, bool useParts) : _vm(engine), _screen(screen_v2), _system(system), _useParts(useParts) {
#endif
	_animations = new Animation[TIM::kWSASlots];
	memset(_animations, 0, TIM::kWSASlots * sizeof(Animation));

	if (_useParts) {
		for (int i = 0; i < TIM::kWSASlots; i++) {
			_animations[i].parts = new AnimPart[TIM::kAnimParts];
			memset(_animations[i].parts, 0, TIM::kAnimParts * sizeof(AnimPart));
		}
	}
}

TimAnimator::~TimAnimator() {
	for (int i = 0; i < TIM::kWSASlots; i++) {
		delete _animations[i].wsa;
		if (_useParts)
			delete[] _animations[i].parts;
	}

	delete[] _animations;
}

void TimAnimator::init(int animIndex, Movie *wsa, int x, int y, int wsaCopyParams, int frameDelay) {
	Animation *anim = &_animations[animIndex];
	anim->wsa = wsa;
	anim->x = x;
	anim->y = y;
	anim->wsaCopyParams = wsaCopyParams;
	anim->frameDelay = frameDelay;
	anim->enable = 0;
	anim->lastPart = -1;
}

void TimAnimator::reset(int animIndex, bool clearStruct) {
	Animation *anim = &_animations[animIndex];
	if (!anim)
		return;
	anim->field_D = 0;
	anim->enable = 0;
	delete anim->wsa;
	anim->wsa = 0;

	if (clearStruct) {
		if (_useParts)
			delete[] anim->parts;

		memset(anim, 0, sizeof(Animation));

		if (_useParts) {
			anim->parts = new AnimPart[TIM::kAnimParts];
			memset(anim->parts, 0, TIM::kAnimParts * sizeof(AnimPart));
		}
	}
}

void TimAnimator::displayFrame(int animIndex, int page, int frame, int flags) {
	Animation *anim = &_animations[animIndex];
	if ((anim->wsaCopyParams & 0x4000) != 0)
		page = 2;
	// WORKAROUND for some bugged scripts that will try to display frames of non-existent animations
	if (anim->wsa)
		anim->wsa->displayFrame(frame, page, anim->x, anim->y, (flags == -1) ? (anim->wsaCopyParams & 0xF0FF) : flags, 0, 0);
	if (!page)
		_screen->updateScreen();
}

#ifdef ENABLE_LOL
void TimAnimator::setupPart(int animIndex, int part, int firstFrame, int lastFrame, int cycles, int nextPart, int partDelay, int f, int sfxIndex, int sfxFrame) {
	AnimPart *a = &_animations[animIndex].parts[part];
	a->firstFrame = firstFrame;
	a->lastFrame = lastFrame;
	a->cycles = cycles;
	a->nextPart = nextPart;
	a->partDelay = partDelay;
	a->field_A = f;
	a->sfxIndex = sfxIndex;
	a->sfxFrame = sfxFrame;
}

void TimAnimator::start(int animIndex, int part) {
	if (!_vm || !_system || !_screen)
		return;

	Animation *anim = &_animations[animIndex];
	anim->curPart = part;
	AnimPart *p = &anim->parts[part];
	anim->enable = 1;
	anim->nextFrame = _system->getMillis() + anim->frameDelay * _vm->_tickLength;
	anim->curFrame = p->firstFrame;
	anim->cyclesCompleted = 0;

	// WORKAROUND for some bugged scripts that will try to display frames of non-existent animations
	if (anim->wsa)
		anim->wsa->displayFrame(anim->curFrame - 1, 0, anim->x, anim->y, 0, 0, 0);
}

void TimAnimator::stop(int animIndex) {
	Animation *anim = &_animations[animIndex];
	anim->enable = 0;
	anim->field_D = 0;
	if (animIndex == 5) {
		delete anim->wsa;
		anim->wsa = 0;
	}
}

void TimAnimator::update(int animIndex) {
	if (!_vm || !_system || !_screen)
		return;

	Animation *anim = &_animations[animIndex];
	if (!anim->enable || anim->nextFrame >= _system->getMillis())
		return;

	AnimPart *p = &anim->parts[anim->curPart];
	anim->nextFrame = 0;

	int step = 0;
	if (p->lastFrame >= p->firstFrame) {
		step = 1;
		anim->curFrame++;
	} else {
		step = -1;
		anim->curFrame--;
	}

	if (anim->curFrame == (p->lastFrame + step)) {
		anim->cyclesCompleted++;

		if ((anim->cyclesCompleted > p->cycles) || anim->field_D) {
			anim->lastPart = anim->curPart;

			if ((p->nextPart == -1) || (anim->field_D && p->field_A)) {
				anim->enable = 0;
				anim->field_D = 0;
				return;
			}

			anim->nextFrame += (p->partDelay * _vm->_tickLength);
			anim->curPart = p->nextPart;

			p = &anim->parts[anim->curPart];
			anim->curFrame = p->firstFrame;
			anim->cyclesCompleted = 0;

		} else {
			anim->curFrame = p->firstFrame;
		}
	}

	if (p->sfxIndex != -1 && p->sfxFrame == anim->curFrame)
		_vm->snd_playSoundEffect(p->sfxIndex, -1);

	anim->nextFrame += (anim->frameDelay * _vm->_tickLength);

	anim->wsa->displayFrame(anim->curFrame - 1, 0, anim->x, anim->y, 0, 0, 0);
	anim->nextFrame += _system->getMillis();
}

void TimAnimator::playPart(int animIndex, int firstFrame, int lastFrame, int delay) {
	if (!_vm || !_system || !_screen)
		return;

	Animation *anim = &_animations[animIndex];

	int step = (lastFrame >= firstFrame) ? 1 : -1;
	for (int i = firstFrame; i != (lastFrame + step); i += step) {
		uint32 next = _system->getMillis() + delay * _vm->_tickLength;
		if (anim->wsaCopyParams & 0x4000) {
			_screen->copyRegion(112, 0, 112, 0, 176, 120, 6, 2);
			anim->wsa->displayFrame(i - 1, 2, anim->x, anim->y, anim->wsaCopyParams & 0x1000 ? 0x5000 : 0x4000, _vm->_transparencyTable1, _vm->_transparencyTable2);
			_screen->copyRegion(112, 0, 112, 0, 176, 120, 2, 0);
			_screen->updateScreen();
		} else {
			anim->wsa->displayFrame(i - 1, 0, anim->x, anim->y, 0, 0, 0);
			_screen->updateScreen();
		}
		int32 del  = (int32)(next - _system->getMillis());
		if (del > 0)
			_vm->delay(del, true);
	}
}

int TimAnimator::resetLastPart(int animIndex) {
	Animation *anim = &_animations[animIndex];
	int8 res = -1;
	SWAP(res, anim->lastPart);
	return res;
}
#endif

} // End of namespace Kyra
