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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "teenagent/actor.h"
#include "teenagent/objects.h"
#include "teenagent/resources.h"
#include "teenagent/teenagent.h"

#include "common/random.h"
#include "common/textconsole.h"

namespace TeenAgent {

Actor::Actor(TeenAgentEngine *vm) : _vm(vm), headIndex(0), idleType(0) {}

Common::Rect Actor::renderIdle(Graphics::Surface *surface, const Common::Point &position, uint8 orientation, int deltaFrame, uint zoom, Common::RandomSource &rnd) {
	if (index == 0) {
		idleType = rnd.getRandomNumber(2);
		debugC(kDebugActor, "switched to idle animation %u", idleType);
	}

	byte *framesIdle;
	do {
		framesIdle = _vm->res->dseg.ptr(_vm->res->dseg.get_word(dsAddr_idleAnimationListPtr + idleType * 2)) + index;
		index += deltaFrame;
		if (*framesIdle == 0) {
			idleType = rnd.getRandomNumber(2);
			debugC(kDebugActor, "switched to idle animation %u[loop]", idleType);
			index = 3; //put 4th frame (base 1) if idle animation loops
		}
	} while (*framesIdle == 0);

	bool mirror = orientation == kActorLeft;
	Surface *s = frames + *framesIdle - 1;

	//TODO: remove copy-paste here and below
	int xp = position.x - s->w * zoom / 512 - s->x, yp = position.y - 62 * zoom / 256 - s->y; //hardcoded in original game
	return s->render(surface, xp, yp, mirror, Common::Rect(), zoom);
}

Common::Rect Actor::render(Graphics::Surface *surface, const Common::Point &position, uint8 orientation, int deltaFrame, bool renderHead, uint zoom) {
	const uint8 framesLeftRight[] = {0, 1, 2, 3, 4, 5, /* step */ 6, 7, 8, 9};
	const uint8 framesUp[] = {18, 19, 20, 21, 22, 23, 24, 25, };
	const uint8 framesDown[] = {10, 11, 12, 13, 14, 15, 16, 17, };

	const uint8 framesHeadLeftRight[] = {
		0x27, 0x1a, 0x1b,
		0x27, 0x1c, 0x1d,
		0x27, 0x1a,
		0x27, 0x1e, 0x1f,
		0x27, 0x1a, 0x1b,
		0x27, 0x1c,
		0x27, 0x1e,
		0x27, 0x1a,
	};

	const uint8 framesHeadUp[] = {
		0x29, 0x25, 0x29, 0x29,
		0x26, 0x29, 0x26, 0x29,
		0x29, 0x25, 0x29, 0x25,
		0x29, 0x29, 0x29, 0x25,
		0x25, 0x29, 0x29, 0x26
	};
	const uint8 framesHeadDown[] = {
		0x20, 0x21, 0x22, 0x23,
		0x28, 0x24, 0x28, 0x28,
		0x24, 0x28, 0x20, 0x21,
		0x20, 0x23, 0x28, 0x20,
		0x28, 0x28, 0x20, 0x28
	};

	Surface *s = NULL, *head = NULL;

	bool mirror = orientation == kActorLeft;
	index += deltaFrame;

	switch (orientation) {
	case kActorLeft:
	case kActorRight:
		if (renderHead) {
			if (headIndex >= ARRAYSIZE(framesHeadLeftRight))
				headIndex = 0;
			head = frames + framesHeadLeftRight[headIndex];
			++headIndex;
		}

		if (index >= ARRAYSIZE(framesLeftRight))
			index = 1;
		s = frames + framesLeftRight[index];
		break;
	case kActorUp:
		if (renderHead) {
			if (headIndex >= ARRAYSIZE(framesHeadUp))
				headIndex = 0;
			head = frames + framesHeadUp[headIndex];
			++headIndex;
		}

		if (index >= ARRAYSIZE(framesUp))
			index = 1;
		s = frames + framesUp[index];
		break;
	case kActorDown:
		if (renderHead) {
			if (headIndex >= ARRAYSIZE(framesHeadDown))
				headIndex = 0;
			head = frames + framesHeadDown[headIndex];
			++headIndex;
		}

		if (index >= ARRAYSIZE(framesDown))
			index = 1;
		s = frames + framesDown[index];
		break;
	default:
		return Common::Rect();
	}
	if (s == NULL) {
		warning("no surface, skipping");
		return Common::Rect();
	}

	Common::Rect dirty;
	Common::Rect clip(0, 0, s->w, s->h);
	if (head != NULL)
		clip.top = head->h;

	int xp = position.x - s->w * zoom / 512 - s->x, yp = position.y - s->h * zoom / 256 - s->y;
	dirty = s->render(surface, xp, yp + clip.top * zoom / 256, mirror, clip, zoom);

	if (head != NULL)
		dirty.extend(head->render(surface, xp, yp, orientation == kActorLeft, Common::Rect(), zoom));

	return dirty;
}

} // End of namespace TeenAgent
