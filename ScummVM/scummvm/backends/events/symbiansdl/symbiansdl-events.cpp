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

#ifdef __SYMBIAN32__

#include "backends/events/symbiansdl/symbiansdl-events.h"
#include "backends/platform/symbian/src/SymbianActions.h"
#include "gui/message.h"
#include "common/translation.h"

#include <bautils.h>

SymbianSdlEventSource::zoneDesc SymbianSdlEventSource::_zones[TOTAL_ZONES] = {
        { 0, 0, 320, 145 },
        { 0, 145, 150, 55 },
        { 150, 145, 170, 55 }
};

SymbianSdlEventSource::SymbianSdlEventSource()
	: _currentZone(0) {
	for (int i = 0; i < TOTAL_ZONES; i++) {
		_mouseXZone[i] = (_zones[i].x + (_zones[i].width / 2));
		_mouseYZone[i] = (_zones[i].y + (_zones[i].height / 2));
	}
}

bool SymbianSdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {
	if (GUI::Actions::Instance()->mappingActive() || ev.key.keysym.sym <= SDLK_UNKNOWN)
		return false;

	for (TInt loop = 0; loop < GUI::ACTION_LAST; loop++) {
		if (GUI::Actions::Instance()->getMapping(loop) == (uint)ev.key.keysym.sym &&
			GUI::Actions::Instance()->isEnabled(loop)) {
			// Create proper event instead
			switch (loop) {
			case GUI::ACTION_UP:
				if (ev.type == SDL_KEYDOWN) {
					_km.y_vel = -1;
					_km.y_down_count = 1;
				} else {
					_km.y_vel = 0;
					_km.y_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				processMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_DOWN:
				if (ev.type == SDL_KEYDOWN) {
					_km.y_vel = 1;
					_km.y_down_count = 1;
				} else {
					_km.y_vel = 0;
					_km.y_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				processMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_LEFT:
				if (ev.type == SDL_KEYDOWN) {
					_km.x_vel = -1;
					_km.x_down_count = 1;
				} else {
					_km.x_vel = 0;
					_km.x_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				processMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_RIGHT:
				if (ev.type == SDL_KEYDOWN) {
					_km.x_vel = 1;
					_km.x_down_count = 1;
				} else {
					_km.x_vel = 0;
					_km.x_down_count = 0;
				}
				event.type = Common::EVENT_MOUSEMOVE;
				processMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_LEFTCLICK:
				event.type = (ev.type == SDL_KEYDOWN ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP);
				processMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_RIGHTCLICK:
				event.type = (ev.type == SDL_KEYDOWN ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP);
				processMouseEvent(event, _km.x, _km.y);

				return true;

			case GUI::ACTION_ZONE:
				if (ev.type == SDL_KEYDOWN) {
					for (int i = 0; i < TOTAL_ZONES; i++)
						if (_km.x >= _zones[i].x && _km.y >= _zones[i].y &&
							_km.x <= _zones[i].x + _zones[i].width && _km.y <= _zones[i].y + _zones[i].height
							) {
							_mouseXZone[i] = _km.x;
							_mouseYZone[i] = _km.y;
							break;
						}
						_currentZone++;
						if (_currentZone >= TOTAL_ZONES)
							_currentZone = 0;
						event.type = Common::EVENT_MOUSEMOVE;
						processMouseEvent(event, _mouseXZone[_currentZone], _mouseYZone[_currentZone]);
						SDL_WarpMouse(event.mouse.x, event.mouse.y);
				}

				return true;
			case GUI::ACTION_MULTI: {
				GUI::Key &key = GUI::Actions::Instance()->getKeyAction(loop);
				// if key code is pause, then change event to interactive or just fall through
				if (key.keycode() == SDLK_PAUSE) {
					event.type = Common::EVENT_PREDICTIVE_DIALOG;
					return true;
					}
				}
			case GUI::ACTION_SAVE:
			case GUI::ACTION_SKIP:
			case GUI::ACTION_SKIP_TEXT:
			case GUI::ACTION_PAUSE:
			case GUI::ACTION_SWAPCHAR:
			case GUI::ACTION_FASTMODE:
			case GUI::ACTION_DEBUGGER:
			case GUI::ACTION_MAINMENU:
			case GUI::ACTION_VKB:
			case GUI::ACTION_KEYMAPPER:{
					GUI::Key &key = GUI::Actions::Instance()->getKeyAction(loop);
					ev.key.keysym.sym = (SDLKey) key.keycode();
					ev.key.keysym.scancode = 0;
					ev.key.keysym.mod = (SDLMod) key.flags();

					// Translate from SDL keymod event to Scummvm Key Mod Common::Event.
					// This codes is also present in GP32 backend and in SDL backend as a static function
					// Perhaps it should be shared.
					if (key.flags() != 0) {
						event.kbd.flags = 0;

						if (ev.key.keysym.mod & KMOD_SHIFT)
							event.kbd.flags |= Common::KBD_SHIFT;

						if (ev.key.keysym.mod & KMOD_ALT)
							event.kbd.flags |= Common::KBD_ALT;

						if (ev.key.keysym.mod & KMOD_CTRL)
							event.kbd.flags |= Common::KBD_CTRL;
					}

					return false;
				}

			case GUI::ACTION_QUIT:
				{
					GUI::MessageDialog alert(_("Do you want to quit ?"), _("Yes"), _("No"));
					if (alert.runModal() == GUI::kMessageOK)
						g_system->quit();

					return true;
				}
			}
		}
	}

	return false;
}

#endif
