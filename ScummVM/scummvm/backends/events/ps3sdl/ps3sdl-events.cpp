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

#if defined(PLAYSTATION3)

#include "backends/events/ps3sdl/ps3sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "engines/engine.h"

#include "common/util.h"
#include "common/events.h"

enum {
	BTN_LEFT		= 0,
	BTN_DOWN		= 1,
	BTN_RIGHT		= 2,
	BTN_UP			= 3,

	BTN_START		= 4,
	BTN_R3			= 5,
	BTN_L3			= 6,
	BTN_SELECT		= 7,

	BTN_SQUARE		= 8,
	BTN_CROSS		= 9,
	BTN_CIRCLE		= 10,
	BTN_TRIANGLE	= 11,

	BTN_R1			= 12,
	BTN_L1			= 13,
	BTN_R2			= 14,
	BTN_L2			= 15
};

bool PS3SdlEventSource::handleJoyButtonDown(SDL_Event &ev, Common::Event &event) {

	event.kbd.flags = 0;

	switch (ev.jbutton.button) {
	case BTN_CROSS: // Left mouse button
		event.type = Common::EVENT_LBUTTONDOWN;
		processMouseEvent(event, _km.x, _km.y);
		break;
	case BTN_CIRCLE: // Right mouse button
		event.type = Common::EVENT_RBUTTONDOWN;
		processMouseEvent(event, _km.x, _km.y);
		break;
	case BTN_TRIANGLE: // Game menu
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_F5;
		event.kbd.ascii = mapKey(SDLK_F5, (SDLMod) ev.key.keysym.mod, 0);
		break;
	case BTN_SELECT: // Virtual keyboard
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_F7;
		event.kbd.ascii = mapKey(SDLK_F7, (SDLMod) ev.key.keysym.mod, 0);
		break;
	case BTN_SQUARE: // Escape
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_ESCAPE;
		event.kbd.ascii = mapKey(SDLK_ESCAPE, (SDLMod) ev.key.keysym.mod, 0);
		break;
	case BTN_L1: // Predictive input dialog
		event.type = Common::EVENT_PREDICTIVE_DIALOG;
		break;
	case BTN_START: // ScummVM in game menu
		event.type = Common::EVENT_MAINMENU;
		break;
	}
	return true;
}

bool PS3SdlEventSource::handleJoyButtonUp(SDL_Event &ev, Common::Event &event) {

	event.kbd.flags = 0;

	switch (ev.jbutton.button) {
	case BTN_CROSS: // Left mouse button
		event.type = Common::EVENT_LBUTTONUP;
		processMouseEvent(event, _km.x, _km.y);
		break;
	case BTN_CIRCLE: // Right mouse button
		event.type = Common::EVENT_RBUTTONUP;
		processMouseEvent(event, _km.x, _km.y);
		break;
	case BTN_TRIANGLE: // Game menu
		event.type = Common::EVENT_KEYUP;
		event.kbd.keycode = Common::KEYCODE_F5;
		event.kbd.ascii = mapKey(SDLK_F5, (SDLMod) ev.key.keysym.mod, 0);
		break;
	case BTN_SELECT: // Virtual keyboard
		event.type = Common::EVENT_KEYUP;
		event.kbd.keycode = Common::KEYCODE_F7;
		event.kbd.ascii = mapKey(SDLK_F7, (SDLMod) ev.key.keysym.mod, 0);
		break;
	case BTN_SQUARE: // Escape
		event.type = Common::EVENT_KEYUP;
		event.kbd.keycode = Common::KEYCODE_ESCAPE;
		event.kbd.ascii = mapKey(SDLK_ESCAPE, (SDLMod) ev.key.keysym.mod, 0);
		break;
	}
	return true;
}

/**
 * The XMB (PS3 in game menu) needs the screen buffers to be constantly flip while open.
 * This pauses execution and keeps redrawing the screen until the XMB is closed.
 */
void PS3SdlEventSource::preprocessEvents(SDL_Event *event) {
	if (event->type == SDL_ACTIVEEVENT) {
		if (event->active.state == SDL_APPMOUSEFOCUS && !event->active.gain) {
			// XMB opened
			if (g_engine)
				g_engine->pauseEngine(true);

			for (;;) {
				if (!SDL_PollEvent(event)) {
					// Locking the screen forces a full redraw
					Graphics::Surface* screen = g_system->lockScreen();
					if (screen) {
						g_system->unlockScreen();
						g_system->updateScreen();
					}
					SDL_Delay(10);
					continue;
				}
				if (event->type == SDL_QUIT)
					return;
				if (event->type != SDL_ACTIVEEVENT)
					continue;
				if (event->active.state == SDL_APPMOUSEFOCUS && event->active.gain) {
					// XMB closed
					if (g_engine)
						g_engine->pauseEngine(false);
					return;
				}
			}
		}
	}
}

#endif
