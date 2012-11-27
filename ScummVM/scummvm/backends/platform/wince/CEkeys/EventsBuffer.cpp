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

#include "backends/platform/sdl/sdl-sys.h"

#include "EventsBuffer.h"

namespace CEKEYS {

bool EventsBuffer::simulateKey(GUI::Key *key, bool pushed) {
	SDL_Event ev = {0};

	if (!key->keycode())
		key->setKey(key->ascii(), key->ascii());
	else if (!key->ascii())
		key->setKey(key->keycode());

	ev.type = (pushed ? SDL_KEYDOWN : SDL_KEYUP);
	ev.key.keysym.unicode = (SDLMod)key->flags();   // HACK: put the flags into the unused unicode field
	ev.key.keysym.sym = (SDLKey)key->keycode();
	ev.key.keysym.mod = KMOD_RESERVED;
	return (SDL_PushEvent(&ev) == 0);
}

bool EventsBuffer::simulateMouseMove(int x, int y) {
	SDL_Event ev = {0};

	ev.type = SDL_MOUSEMOTION;
	ev.motion.x = x;
	ev.motion.y = y;
	return (SDL_PushEvent(&ev) == 0);
}

bool EventsBuffer::simulateMouseLeftClick(int x, int y, bool pushed) {
	SDL_Event ev = {0};
	static bool state = false;

	if (pushed == state) return 0;
	state = pushed;
	ev.type = (pushed ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP);
	ev.button.button = SDL_BUTTON_LEFT;
	ev.button.x = x;
	ev.button.y = y;
	return (SDL_PushEvent(&ev) == 0);
}

bool EventsBuffer::simulateMouseRightClick(int x, int y, bool pushed) {
	SDL_Event ev = {0};
	static bool state = false;

	if (pushed == state) return 0;
	state = pushed;
	ev.type = (pushed ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP);
	ev.button.button = SDL_BUTTON_RIGHT;
	ev.button.x = x;
	ev.button.y = y;
	return (SDL_PushEvent(&ev) == 0);
}
}
