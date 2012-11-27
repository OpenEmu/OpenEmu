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

#ifndef BACKEND_EVENTS_SDL_H
#define BACKEND_EVENTS_SDL_H

#include "backends/platform/sdl/sdl-sys.h"
#include "backends/graphics/sdl/sdl-graphics.h"

#include "common/events.h"


/**
 * The SDL event source.
 */
class SdlEventSource : public Common::EventSource {
public:
	SdlEventSource();
	virtual ~SdlEventSource();

	void setGraphicsManager(SdlGraphicsManager *gMan) { _graphicsManager = gMan; }

	/**
	 * Gets and processes SDL events.
	 */
	virtual bool pollEvent(Common::Event &event);

	/**
	 * Resets keyboard emulation after a video screen change
	 */
	virtual void resetKeyboadEmulation(int16 x_max, int16 y_max);

	/**
	 * Toggles mouse input grab
	 */
	virtual void toggleMouseGrab();

protected:
	/** @name Keyboard mouse emulation
	 * Disabled by fingolfin 2004-12-18.
	 * I am keeping the rest of the code in for now, since the joystick
	 * code (or rather, "hack") uses it, too.
	 */
	//@{

	struct KbdMouse {
		int16 x, y, x_vel, y_vel, x_max, y_max, x_down_count, y_down_count;
		uint32 last_time, delay_time, x_down_time, y_down_time;
	};
	KbdMouse _km;

	//@}

	/** Scroll lock state - since SDL doesn't track it */
	bool _scrollLock;

	/** Joystick */
	SDL_Joystick *_joystick;

	/** Last screen id for checking if it was modified */
	int _lastScreenID;

	/**
	 * The associated graphics manager.
	 */
	SdlGraphicsManager *_graphicsManager;

	/**
	 * Pre process an event before it is dispatched.
	 */
	virtual void preprocessEvents(SDL_Event *event) {}

	/**
	 * Dispatchs SDL events for each handler.
	 */
	virtual bool dispatchSDLEvent(SDL_Event &ev, Common::Event &event);


	/** @name Event Handlers
	 * Handlers for specific SDL events, called by SdlEventSource::dispatchSDLEvent().
	 * This way, if a managers inherits fromt this SDL events manager, it can
	 * change the behavior of only a single event, without having to override all
	 * of SdlEventSource::dispatchSDLEvent().
	 */
	//@{

	virtual bool handleKeyDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleKeyUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseMotion(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyButtonDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyButtonUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleJoyAxisMotion(SDL_Event &ev, Common::Event &event);
	virtual void handleKbdMouse();

	//@}

	/**
	 * Assigns the mouse coords to the mouse event. Furthermore notify the
	 * graphics manager about the position change.
	 */
	virtual void processMouseEvent(Common::Event &event, int x, int y);

	/**
	 * Remaps key events. This allows platforms to configure
	 * their custom keys.
	 */
	virtual bool remapKey(SDL_Event &ev, Common::Event &event);

	/**
	 * Maps the ASCII value of key
	 */
	virtual int mapKey(SDLKey key, SDLMod mod, Uint16 unicode);

	/**
	 * Configures the key modifiers flags status
	 */
	virtual void SDLModToOSystemKeyFlags(SDLMod mod, Common::Event &event);

	/**
	 * Translates SDL key codes to OSystem key codes
	 */
	Common::KeyCode SDLToOSystemKeycode(const SDLKey key);
};

#endif
