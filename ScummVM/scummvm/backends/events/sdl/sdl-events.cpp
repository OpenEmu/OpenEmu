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

#if defined(SDL_BACKEND)

#include "backends/events/sdl/sdl-events.h"
#include "backends/platform/sdl/sdl.h"
#include "backends/graphics/graphics.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

// FIXME move joystick defines out and replace with confile file options
// we should really allow users to map any key to a joystick button
#define JOY_DEADZONE 3200

#ifndef __SYMBIAN32__ // Symbian wants dialog joystick i.e cursor for movement/selection
	#define JOY_ANALOG
#endif

// #define JOY_INVERT_Y
#define JOY_XAXIS 0
#define JOY_YAXIS 1
// buttons
#define JOY_BUT_LMOUSE 0
#define JOY_BUT_RMOUSE 2
#define JOY_BUT_ESCAPE 3
#define JOY_BUT_PERIOD 1
#define JOY_BUT_SPACE 4
#define JOY_BUT_F5 5

SdlEventSource::SdlEventSource()
    : EventSource(), _scrollLock(false), _joystick(0), _lastScreenID(0), _graphicsManager(0) {
	// Reset mouse state
	memset(&_km, 0, sizeof(_km));

	int joystick_num = ConfMan.getInt("joystick_num");
	if (joystick_num > -1) {
		// Initialize SDL joystick subsystem
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1) {
			error("Could not initialize SDL: %s", SDL_GetError());
		}

		// Enable joystick
		if (SDL_NumJoysticks() > 0) {
			debug("Using joystick: %s", SDL_JoystickName(0));
			_joystick = SDL_JoystickOpen(joystick_num);
		}
	}
}

SdlEventSource::~SdlEventSource() {
	if (_joystick)
		SDL_JoystickClose(_joystick);
}

int SdlEventSource::mapKey(SDLKey key, SDLMod mod, Uint16 unicode) {
	if (key >= SDLK_F1 && key <= SDLK_F9) {
		return key - SDLK_F1 + Common::ASCII_F1;
	} else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
		return key - SDLK_KP0 + '0';
	} else if (key >= SDLK_UP && key <= SDLK_PAGEDOWN) {
		return key;
	} else if (unicode) {
		return unicode;
	} else if (key >= 'a' && key <= 'z' && (mod & KMOD_SHIFT)) {
		return key & ~0x20;
	} else if (key >= SDLK_NUMLOCK && key <= SDLK_EURO) {
		return 0;
	}
	return key;
}

void SdlEventSource::processMouseEvent(Common::Event &event, int x, int y) {
	event.mouse.x = x;
	event.mouse.y = y;

	if (_graphicsManager) {
		_graphicsManager->notifyMousePos(Common::Point(x, y));
		_graphicsManager->transformMouseCoordinates(event.mouse);
	}

	// Update the "keyboard mouse" coords
	_km.x = x;
	_km.y = y;
}

void SdlEventSource::handleKbdMouse() {
	uint32 curTime = g_system->getMillis();
	if (curTime >= _km.last_time + _km.delay_time) {
		_km.last_time = curTime;
		if (_km.x_down_count == 1) {
			_km.x_down_time = curTime;
			_km.x_down_count = 2;
		}
		if (_km.y_down_count == 1) {
			_km.y_down_time = curTime;
			_km.y_down_count = 2;
		}

		if (_km.x_vel || _km.y_vel) {
			if (_km.x_down_count) {
				if (curTime > _km.x_down_time + _km.delay_time * 12) {
					if (_km.x_vel > 0)
						_km.x_vel++;
					else
						_km.x_vel--;
				} else if (curTime > _km.x_down_time + _km.delay_time * 8) {
					if (_km.x_vel > 0)
						_km.x_vel = 5;
					else
						_km.x_vel = -5;
				}
			}
			if (_km.y_down_count) {
				if (curTime > _km.y_down_time + _km.delay_time * 12) {
					if (_km.y_vel > 0)
						_km.y_vel++;
					else
						_km.y_vel--;
				} else if (curTime > _km.y_down_time + _km.delay_time * 8) {
					if (_km.y_vel > 0)
						_km.y_vel = 5;
					else
						_km.y_vel = -5;
				}
			}

			_km.x += _km.x_vel;
			_km.y += _km.y_vel;

			if (_km.x < 0) {
				_km.x = 0;
				_km.x_vel = -1;
				_km.x_down_count = 1;
			} else if (_km.x > _km.x_max) {
				_km.x = _km.x_max;
				_km.x_vel = 1;
				_km.x_down_count = 1;
			}

			if (_km.y < 0) {
				_km.y = 0;
				_km.y_vel = -1;
				_km.y_down_count = 1;
			} else if (_km.y > _km.y_max) {
				_km.y = _km.y_max;
				_km.y_vel = 1;
				_km.y_down_count = 1;
			}

			SDL_WarpMouse((Uint16)_km.x, (Uint16)_km.y);
		}
	}
}

void SdlEventSource::SDLModToOSystemKeyFlags(SDLMod mod, Common::Event &event) {

	event.kbd.flags = 0;

#ifdef LINUPY
	// Yopy has no ALT key, steal the SHIFT key
	// (which isn't used much anyway)
	if (mod & KMOD_SHIFT)
		event.kbd.flags |= Common::KBD_ALT;
#else
	if (mod & KMOD_SHIFT)
		event.kbd.flags |= Common::KBD_SHIFT;
	if (mod & KMOD_ALT)
		event.kbd.flags |= Common::KBD_ALT;
#endif
	if (mod & KMOD_CTRL)
		event.kbd.flags |= Common::KBD_CTRL;
	if (mod & KMOD_META)
		event.kbd.flags |= Common::KBD_META;

	// Sticky flags
	if (mod & KMOD_NUM)
		event.kbd.flags |= Common::KBD_NUM;
	if (mod & KMOD_CAPS)
		event.kbd.flags |= Common::KBD_CAPS;
}

Common::KeyCode SdlEventSource::SDLToOSystemKeycode(const SDLKey key) {
	switch (key) {
	case SDLK_BACKSPACE: return Common::KEYCODE_BACKSPACE;
	case SDLK_TAB: return Common::KEYCODE_TAB;
	case SDLK_CLEAR: return Common::KEYCODE_CLEAR;
	case SDLK_RETURN: return Common::KEYCODE_RETURN;
	case SDLK_PAUSE: return Common::KEYCODE_PAUSE;
	case SDLK_ESCAPE: return Common::KEYCODE_ESCAPE;
	case SDLK_SPACE: return Common::KEYCODE_SPACE;
	case SDLK_EXCLAIM: return Common::KEYCODE_EXCLAIM;
	case SDLK_QUOTEDBL: return Common::KEYCODE_QUOTEDBL;
	case SDLK_HASH: return Common::KEYCODE_HASH;
	case SDLK_DOLLAR: return Common::KEYCODE_DOLLAR;
	case SDLK_AMPERSAND: return Common::KEYCODE_AMPERSAND;
	case SDLK_QUOTE: return Common::KEYCODE_QUOTE;
	case SDLK_LEFTPAREN: return Common::KEYCODE_LEFTPAREN;
	case SDLK_RIGHTPAREN: return Common::KEYCODE_RIGHTPAREN;
	case SDLK_ASTERISK: return Common::KEYCODE_ASTERISK;
	case SDLK_PLUS: return Common::KEYCODE_PLUS;
	case SDLK_COMMA: return Common::KEYCODE_COMMA;
	case SDLK_MINUS: return Common::KEYCODE_MINUS;
	case SDLK_PERIOD: return Common::KEYCODE_PERIOD;
	case SDLK_SLASH: return Common::KEYCODE_SLASH;
	case SDLK_0: return Common::KEYCODE_0;
	case SDLK_1: return Common::KEYCODE_1;
	case SDLK_2: return Common::KEYCODE_2;
	case SDLK_3: return Common::KEYCODE_3;
	case SDLK_4: return Common::KEYCODE_4;
	case SDLK_5: return Common::KEYCODE_5;
	case SDLK_6: return Common::KEYCODE_6;
	case SDLK_7: return Common::KEYCODE_7;
	case SDLK_8: return Common::KEYCODE_8;
	case SDLK_9: return Common::KEYCODE_9;
	case SDLK_COLON: return Common::KEYCODE_COLON;
	case SDLK_SEMICOLON: return Common::KEYCODE_SEMICOLON;
	case SDLK_LESS: return Common::KEYCODE_LESS;
	case SDLK_EQUALS: return Common::KEYCODE_EQUALS;
	case SDLK_GREATER: return Common::KEYCODE_GREATER;
	case SDLK_QUESTION: return Common::KEYCODE_QUESTION;
	case SDLK_AT: return Common::KEYCODE_AT;
	case SDLK_LEFTBRACKET: return Common::KEYCODE_LEFTBRACKET;
	case SDLK_BACKSLASH: return Common::KEYCODE_BACKSLASH;
	case SDLK_RIGHTBRACKET: return Common::KEYCODE_RIGHTBRACKET;
	case SDLK_CARET: return Common::KEYCODE_CARET;
	case SDLK_UNDERSCORE: return Common::KEYCODE_UNDERSCORE;
	case SDLK_BACKQUOTE: return Common::KEYCODE_BACKQUOTE;
	case SDLK_a: return Common::KEYCODE_a;
	case SDLK_b: return Common::KEYCODE_b;
	case SDLK_c: return Common::KEYCODE_c;
	case SDLK_d: return Common::KEYCODE_d;
	case SDLK_e: return Common::KEYCODE_e;
	case SDLK_f: return Common::KEYCODE_f;
	case SDLK_g: return Common::KEYCODE_g;
	case SDLK_h: return Common::KEYCODE_h;
	case SDLK_i: return Common::KEYCODE_i;
	case SDLK_j: return Common::KEYCODE_j;
	case SDLK_k: return Common::KEYCODE_k;
	case SDLK_l: return Common::KEYCODE_l;
	case SDLK_m: return Common::KEYCODE_m;
	case SDLK_n: return Common::KEYCODE_n;
	case SDLK_o: return Common::KEYCODE_o;
	case SDLK_p: return Common::KEYCODE_p;
	case SDLK_q: return Common::KEYCODE_q;
	case SDLK_r: return Common::KEYCODE_r;
	case SDLK_s: return Common::KEYCODE_s;
	case SDLK_t: return Common::KEYCODE_t;
	case SDLK_u: return Common::KEYCODE_u;
	case SDLK_v: return Common::KEYCODE_v;
	case SDLK_w: return Common::KEYCODE_w;
	case SDLK_x: return Common::KEYCODE_x;
	case SDLK_y: return Common::KEYCODE_y;
	case SDLK_z: return Common::KEYCODE_z;
	case SDLK_DELETE: return Common::KEYCODE_DELETE;
#if SDL_VERSION_ATLEAST(1, 3, 0)
	case SDL_SCANCODE_TO_KEYCODE(SDL_SCANCODE_GRAVE): return Common::KEYCODE_TILDE;
#else
	case SDLK_WORLD_16: return Common::KEYCODE_TILDE;
#endif
	case SDLK_KP0: return Common::KEYCODE_KP0;
	case SDLK_KP1: return Common::KEYCODE_KP1;
	case SDLK_KP2: return Common::KEYCODE_KP2;
	case SDLK_KP3: return Common::KEYCODE_KP3;
	case SDLK_KP4: return Common::KEYCODE_KP4;
	case SDLK_KP5: return Common::KEYCODE_KP5;
	case SDLK_KP6: return Common::KEYCODE_KP6;
	case SDLK_KP7: return Common::KEYCODE_KP7;
	case SDLK_KP8: return Common::KEYCODE_KP8;
	case SDLK_KP9: return Common::KEYCODE_KP9;
	case SDLK_KP_PERIOD: return Common::KEYCODE_KP_PERIOD;
	case SDLK_KP_DIVIDE: return Common::KEYCODE_KP_DIVIDE;
	case SDLK_KP_MULTIPLY: return Common::KEYCODE_KP_MULTIPLY;
	case SDLK_KP_MINUS: return Common::KEYCODE_KP_MINUS;
	case SDLK_KP_PLUS: return Common::KEYCODE_KP_PLUS;
	case SDLK_KP_ENTER: return Common::KEYCODE_KP_ENTER;
	case SDLK_KP_EQUALS: return Common::KEYCODE_KP_EQUALS;
	case SDLK_UP: return Common::KEYCODE_UP;
	case SDLK_DOWN: return Common::KEYCODE_DOWN;
	case SDLK_RIGHT: return Common::KEYCODE_RIGHT;
	case SDLK_LEFT: return Common::KEYCODE_LEFT;
	case SDLK_INSERT: return Common::KEYCODE_INSERT;
	case SDLK_HOME: return Common::KEYCODE_HOME;
	case SDLK_END: return Common::KEYCODE_END;
	case SDLK_PAGEUP: return Common::KEYCODE_PAGEUP;
	case SDLK_PAGEDOWN: return Common::KEYCODE_PAGEDOWN;
	case SDLK_F1: return Common::KEYCODE_F1;
	case SDLK_F2: return Common::KEYCODE_F2;
	case SDLK_F3: return Common::KEYCODE_F3;
	case SDLK_F4: return Common::KEYCODE_F4;
	case SDLK_F5: return Common::KEYCODE_F5;
	case SDLK_F6: return Common::KEYCODE_F6;
	case SDLK_F7: return Common::KEYCODE_F7;
	case SDLK_F8: return Common::KEYCODE_F8;
	case SDLK_F9: return Common::KEYCODE_F9;
	case SDLK_F10: return Common::KEYCODE_F10;
	case SDLK_F11: return Common::KEYCODE_F11;
	case SDLK_F12: return Common::KEYCODE_F12;
	case SDLK_F13: return Common::KEYCODE_F13;
	case SDLK_F14: return Common::KEYCODE_F14;
	case SDLK_F15: return Common::KEYCODE_F15;
	case SDLK_NUMLOCK: return Common::KEYCODE_NUMLOCK;
	case SDLK_CAPSLOCK: return Common::KEYCODE_CAPSLOCK;
	case SDLK_SCROLLOCK: return Common::KEYCODE_SCROLLOCK;
	case SDLK_RSHIFT: return Common::KEYCODE_RSHIFT;
	case SDLK_LSHIFT: return Common::KEYCODE_LSHIFT;
	case SDLK_RCTRL: return Common::KEYCODE_RCTRL;
	case SDLK_LCTRL: return Common::KEYCODE_LCTRL;
	case SDLK_RALT: return Common::KEYCODE_RALT;
	case SDLK_LALT: return Common::KEYCODE_LALT;
	case SDLK_LSUPER: return Common::KEYCODE_LSUPER;
	case SDLK_RSUPER: return Common::KEYCODE_RSUPER;
	case SDLK_MODE: return Common::KEYCODE_MODE;
	case SDLK_COMPOSE: return Common::KEYCODE_COMPOSE;
	case SDLK_HELP: return Common::KEYCODE_HELP;
	case SDLK_PRINT: return Common::KEYCODE_PRINT;
	case SDLK_SYSREQ: return Common::KEYCODE_SYSREQ;
	case SDLK_BREAK: return Common::KEYCODE_BREAK;
	case SDLK_MENU: return Common::KEYCODE_MENU;
	case SDLK_POWER: return Common::KEYCODE_POWER;
	case SDLK_UNDO: return Common::KEYCODE_UNDO;
	default: return Common::KEYCODE_INVALID;
	}
}

bool SdlEventSource::pollEvent(Common::Event &event) {
	handleKbdMouse();

	// If the screen changed, send an Common::EVENT_SCREEN_CHANGED
	int screenID = ((OSystem_SDL *)g_system)->getGraphicsManager()->getScreenChangeID();
	if (screenID != _lastScreenID) {
		_lastScreenID = screenID;
		event.type = Common::EVENT_SCREEN_CHANGED;
		return true;
	}

	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		preprocessEvents(&ev);
		if (dispatchSDLEvent(ev, event))
			return true;
	}
	return false;
}

bool SdlEventSource::dispatchSDLEvent(SDL_Event &ev, Common::Event &event) {
	switch (ev.type) {
	case SDL_KEYDOWN:
		return handleKeyDown(ev, event);
	case SDL_KEYUP:
		return handleKeyUp(ev, event);
	case SDL_MOUSEMOTION:
		return handleMouseMotion(ev, event);
	case SDL_MOUSEBUTTONDOWN:
		return handleMouseButtonDown(ev, event);
	case SDL_MOUSEBUTTONUP:
		return handleMouseButtonUp(ev, event);
	case SDL_JOYBUTTONDOWN:
		return handleJoyButtonDown(ev, event);
	case SDL_JOYBUTTONUP:
		return handleJoyButtonUp(ev, event);
	case SDL_JOYAXISMOTION:
		return handleJoyAxisMotion(ev, event);

	case SDL_VIDEOEXPOSE:
		if (_graphicsManager)
			_graphicsManager->notifyVideoExpose();
		return false;

	case SDL_VIDEORESIZE:
		if (_graphicsManager)
			_graphicsManager->notifyResize(ev.resize.w, ev.resize.h);
		return false;

	case SDL_QUIT:
		event.type = Common::EVENT_QUIT;
		return true;

	}

	return false;
}


bool SdlEventSource::handleKeyDown(SDL_Event &ev, Common::Event &event) {

	SDLModToOSystemKeyFlags(SDL_GetModState(), event);

	// Handle scroll lock as a key modifier
	if (ev.key.keysym.sym == SDLK_SCROLLOCK)
		_scrollLock = !_scrollLock;

	if (_scrollLock)
		event.kbd.flags |= Common::KBD_SCRL;

	// Ctrl-m toggles mouse capture
	if (event.kbd.hasFlags(Common::KBD_CTRL) && ev.key.keysym.sym == 'm') {
		toggleMouseGrab();
		return false;
	}

#if defined(MACOSX)
	// On Macintosh, Cmd-Q quits
	if ((ev.key.keysym.mod & KMOD_META) && ev.key.keysym.sym == 'q') {
		event.type = Common::EVENT_QUIT;
		return true;
	}
#elif defined(POSIX)
	// On other *nix systems, Control-Q quits
	if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == 'q') {
		event.type = Common::EVENT_QUIT;
		return true;
	}
#else
	// Ctrl-z and Alt-X quit
	if ((event.kbd.hasFlags(Common::KBD_CTRL) && ev.key.keysym.sym == 'z') || (event.kbd.hasFlags(Common::KBD_ALT) && ev.key.keysym.sym == 'x')) {
		event.type = Common::EVENT_QUIT;
		return true;
	}

	#ifdef WIN32
	// On Windows, also use the default Alt-F4 quit combination
	if ((ev.key.keysym.mod & KMOD_ALT) && ev.key.keysym.sym == SDLK_F4) {
		event.type = Common::EVENT_QUIT;
		return true;
	}
	#endif
#endif

	// Ctrl-u toggles mute
	if ((ev.key.keysym.mod & KMOD_CTRL) && ev.key.keysym.sym == 'u') {
		event.type = Common::EVENT_MUTE;
		return true;
	}

	if (remapKey(ev, event))
		return true;

	event.type = Common::EVENT_KEYDOWN;
	event.kbd.keycode = SDLToOSystemKeycode(ev.key.keysym.sym);
	event.kbd.ascii = mapKey(ev.key.keysym.sym, (SDLMod)ev.key.keysym.mod, (Uint16)ev.key.keysym.unicode);

	return true;
}

bool SdlEventSource::handleKeyUp(SDL_Event &ev, Common::Event &event) {
	if (remapKey(ev, event))
		return true;

	SDLMod mod = SDL_GetModState();

	// Check if this is an event handled by handleKeyDown(), and stop if it is

	// Check if the Ctrl key is down, so that we can trap cases where the
	// user has the Ctrl key down, and has just released a special key
	if (mod & KMOD_CTRL) {
		if (ev.key.keysym.sym == 'm' ||	// Ctrl-m toggles mouse capture
#if defined(MACOSX)
			// Meta - Q, handled below
#elif defined(POSIX)
			ev.key.keysym.sym == 'q' ||	// On other *nix systems, Control-Q quits
#else
			ev.key.keysym.sym == 'z' ||	// Ctrl-z quit
#endif
			ev.key.keysym.sym == 'u')	// Ctrl-u toggles mute
			return false;
	}

	// Same for other keys (Meta and Alt)
#if defined(MACOSX)
	if ((mod & KMOD_META) && ev.key.keysym.sym == 'q')
		return false;	// On Macintosh, Cmd-Q quits
#elif defined(POSIX)
	// Control Q has already been handled above
#else
	if ((mod & KMOD_ALT) && ev.key.keysym.sym == 'x')
		return false;	// Alt-x quit
#endif

	// If we reached here, this isn't an event handled by handleKeyDown(), thus
	// continue normally

	event.type = Common::EVENT_KEYUP;
	event.kbd.keycode = SDLToOSystemKeycode(ev.key.keysym.sym);
	event.kbd.ascii = mapKey(ev.key.keysym.sym, (SDLMod)ev.key.keysym.mod, (Uint16)ev.key.keysym.unicode);

	// Ctrl-Alt-<key> will change the GFX mode
	SDLModToOSystemKeyFlags(mod, event);

	// Set the scroll lock sticky flag
	if (_scrollLock)
		event.kbd.flags |= Common::KBD_SCRL;

	return true;
}

bool SdlEventSource::handleMouseMotion(SDL_Event &ev, Common::Event &event) {
	event.type = Common::EVENT_MOUSEMOVE;
	processMouseEvent(event, ev.motion.x, ev.motion.y);

	return true;
}

bool SdlEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT)
		event.type = Common::EVENT_LBUTTONDOWN;
	else if (ev.button.button == SDL_BUTTON_RIGHT)
		event.type = Common::EVENT_RBUTTONDOWN;
#if defined(SDL_BUTTON_WHEELUP) && defined(SDL_BUTTON_WHEELDOWN)
	else if (ev.button.button == SDL_BUTTON_WHEELUP)
		event.type = Common::EVENT_WHEELUP;
	else if (ev.button.button == SDL_BUTTON_WHEELDOWN)
		event.type = Common::EVENT_WHEELDOWN;
#endif
#if defined(SDL_BUTTON_MIDDLE)
	else if (ev.button.button == SDL_BUTTON_MIDDLE)
		event.type = Common::EVENT_MBUTTONDOWN;
#endif
	else
		return false;

	processMouseEvent(event, ev.button.x, ev.button.y);

	return true;
}

bool SdlEventSource::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT)
		event.type = Common::EVENT_LBUTTONUP;
	else if (ev.button.button == SDL_BUTTON_RIGHT)
		event.type = Common::EVENT_RBUTTONUP;
#if defined(SDL_BUTTON_MIDDLE)
	else if (ev.button.button == SDL_BUTTON_MIDDLE)
		event.type = Common::EVENT_MBUTTONUP;
#endif
	else
		return false;
	processMouseEvent(event, ev.button.x, ev.button.y);

	return true;
}

bool SdlEventSource::handleJoyButtonDown(SDL_Event &ev, Common::Event &event) {
	if (ev.jbutton.button == JOY_BUT_LMOUSE) {
		event.type = Common::EVENT_LBUTTONDOWN;
		processMouseEvent(event, _km.x, _km.y);
	} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
		event.type = Common::EVENT_RBUTTONDOWN;
		processMouseEvent(event, _km.x, _km.y);
	} else {
		event.type = Common::EVENT_KEYDOWN;
		switch (ev.jbutton.button) {
		case JOY_BUT_ESCAPE:
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = mapKey(SDLK_ESCAPE, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_PERIOD:
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = mapKey(SDLK_PERIOD, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_SPACE:
			event.kbd.keycode = Common::KEYCODE_SPACE;
			event.kbd.ascii = mapKey(SDLK_SPACE, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_F5:
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = mapKey(SDLK_F5, (SDLMod)ev.key.keysym.mod, 0);
			break;
		}
	}
	return true;
}

bool SdlEventSource::handleJoyButtonUp(SDL_Event &ev, Common::Event &event) {
	if (ev.jbutton.button == JOY_BUT_LMOUSE) {
		event.type = Common::EVENT_LBUTTONUP;
		processMouseEvent(event, _km.x, _km.y);
	} else if (ev.jbutton.button == JOY_BUT_RMOUSE) {
		event.type = Common::EVENT_RBUTTONUP;
		processMouseEvent(event, _km.x, _km.y);
	} else {
		event.type = Common::EVENT_KEYUP;
		switch (ev.jbutton.button) {
		case JOY_BUT_ESCAPE:
			event.kbd.keycode = Common::KEYCODE_ESCAPE;
			event.kbd.ascii = mapKey(SDLK_ESCAPE, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_PERIOD:
			event.kbd.keycode = Common::KEYCODE_PERIOD;
			event.kbd.ascii = mapKey(SDLK_PERIOD, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_SPACE:
			event.kbd.keycode = Common::KEYCODE_SPACE;
			event.kbd.ascii = mapKey(SDLK_SPACE, (SDLMod)ev.key.keysym.mod, 0);
			break;
		case JOY_BUT_F5:
			event.kbd.keycode = Common::KEYCODE_F5;
			event.kbd.ascii = mapKey(SDLK_F5, (SDLMod)ev.key.keysym.mod, 0);
			break;
		}
	}
	return true;
}

bool SdlEventSource::handleJoyAxisMotion(SDL_Event &ev, Common::Event &event) {
	int axis = ev.jaxis.value;
	if ( axis > JOY_DEADZONE) {
		axis -= JOY_DEADZONE;
		event.type = Common::EVENT_MOUSEMOVE;
	} else if ( axis < -JOY_DEADZONE ) {
		axis += JOY_DEADZONE;
		event.type = Common::EVENT_MOUSEMOVE;
	} else
		axis = 0;

	if ( ev.jaxis.axis == JOY_XAXIS) {
#ifdef JOY_ANALOG
		_km.x_vel = axis / 2000;
		_km.x_down_count = 0;
#else
		if (axis != 0) {
			_km.x_vel = (axis > 0) ? 1:-1;
			_km.x_down_count = 1;
		} else {
			_km.x_vel = 0;
			_km.x_down_count = 0;
		}
#endif

	} else if (ev.jaxis.axis == JOY_YAXIS) {
#ifndef JOY_INVERT_Y
		axis = -axis;
#endif
#ifdef JOY_ANALOG
		_km.y_vel = -axis / 2000;
		_km.y_down_count = 0;
#else
		if (axis != 0) {
			_km.y_vel = (-axis > 0) ? 1: -1;
			_km.y_down_count = 1;
		} else {
			_km.y_vel = 0;
			_km.y_down_count = 0;
		}
#endif
	}

	processMouseEvent(event, _km.x, _km.y);

	return true;
}

bool SdlEventSource::remapKey(SDL_Event &ev, Common::Event &event) {
#ifdef LINUPY
	// On Yopy map the End button to quit
	if ((ev.key.keysym.sym == 293)) {
		event.type = Common::EVENT_QUIT;
		return true;
	}
	// Map menu key to f5 (scumm menu)
	if (ev.key.keysym.sym == 306) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_F5;
		event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
		return true;
	}
	// Map action key to action
	if (ev.key.keysym.sym == 291) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_TAB;
		event.kbd.ascii = mapKey(SDLK_TAB, ev.key.keysym.mod, 0);
		return true;
	}
	// Map OK key to skip cinematic
	if (ev.key.keysym.sym == 292) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_ESCAPE;
		event.kbd.ascii = mapKey(SDLK_ESCAPE, ev.key.keysym.mod, 0);
		return true;
	}
#endif

#ifdef QTOPIA
	// Quit on fn+backspace on zaurus
	if (ev.key.keysym.sym == 127) {
		event.type = Common::EVENT_QUIT;
		return true;
	}

	// Map menu key (f11) to f5 (scumm menu)
	if (ev.key.keysym.sym == SDLK_F11) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_F5;
		event.kbd.ascii = mapKey(SDLK_F5, ev.key.keysym.mod, 0);
	}
	// Nap center (space) to tab (default action )
	// I wanted to map the calendar button but the calendar comes up
	//
	else if (ev.key.keysym.sym == SDLK_SPACE) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_TAB;
		event.kbd.ascii = mapKey(SDLK_TAB, ev.key.keysym.mod, 0);
	}
	// Since we stole space (pause) above we'll rebind it to the tab key on the keyboard
	else if (ev.key.keysym.sym == SDLK_TAB) {
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = Common::KEYCODE_SPACE;
		event.kbd.ascii = mapKey(SDLK_SPACE, ev.key.keysym.mod, 0);
	} else {
	// Let the events fall through if we didn't change them, this may not be the best way to
	// set it up, but i'm not sure how sdl would like it if we let if fall through then redid it though.
	// and yes i have an huge terminal size so i dont wrap soon enough.
		event.type = Common::EVENT_KEYDOWN;
		event.kbd.keycode = ev.key.keysym.sym;
		event.kbd.ascii = mapKey(ev.key.keysym.sym, ev.key.keysym.mod, ev.key.keysym.unicode);
	}
#endif
	return false;
}

void SdlEventSource::toggleMouseGrab() {
	if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_OFF)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
}

void SdlEventSource::resetKeyboadEmulation(int16 x_max, int16 y_max) {
	_km.x_max = x_max;
	_km.y_max = y_max;
	_km.delay_time = 25;
	_km.last_time = 0;
}

#endif
