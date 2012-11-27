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

#include "common/scummsys.h"

#ifdef SAMSUNGTV

#include "backends/events/samsungtvsdl/samsungtvsdl-events.h"

bool SamsungTVSdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {
	switch (ev.type) {
		case SDL_KEYDOWN:{
			if (ev.key.keysym.sym == SDLK_POWER) {
				event.type = Common::EVENT_QUIT;
				return true;
			} else if (ev.key.keysym.sym == SDLK_F1 && ev.key.keysym.scancode == 20) {
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_F5;
				event.kbd.ascii = Common::ASCII_F5;
				return true;
			} else if (ev.key.keysym.sym == SDLK_F2 && ev.key.keysym.scancode == 21) {
				event.type = Common::EVENT_KEYDOWN;
				event.kbd.keycode = Common::KEYCODE_F7;
				event.kbd.ascii = Common::ASCII_F7;
				return true;
			}
			break;
		}
		case SDL_KEYUP: {
			if (ev.key.keysym.sym == SDLK_POWER) {
				event.type = Common::EVENT_QUIT;
				return true;
			} else if (ev.key.keysym.sym == SDLK_F1 && ev.key.keysym.scancode == 20) {
				event.type = Common::EVENT_KEYUP;
				event.kbd.keycode = Common::KEYCODE_F5;
				event.kbd.ascii = Common::ASCII_F5;
				return true;
			} else if (ev.key.keysym.sym == SDLK_F2 && ev.key.keysym.scancode == 21) {
				event.type = Common::EVENT_KEYUP;
				event.kbd.keycode = Common::KEYCODE_F7;
				event.kbd.ascii = Common::ASCII_F7;
				return true;
			}
			break;
		}
	}

	// Invoke parent implementation of this method
	return SdlEventSource::remapKey(ev, event);
}

#endif
