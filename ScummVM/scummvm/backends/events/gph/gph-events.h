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

#if !defined(BACKEND_EVENTS_GPH_H) && !defined(DISABLE_DEFAULT_EVENTMANAGER)
#define BACKEND_EVENTS_GPH_H

#include "backends/events/sdl/sdl-events.h"

/*
 * SDL Events manager for GPH devices.
 */

class GPHEventSource : public SdlEventSource {
public:
	GPHEventSource();

protected:
	/**
	 * Button state for L button modifier
	 */
	bool _buttonStateL;

	bool handleJoyButtonDown(SDL_Event &ev, Common::Event &event);
	bool handleJoyButtonUp(SDL_Event &ev, Common::Event &event);
	bool handleMouseButtonDown(SDL_Event &ev, Common::Event &event);
	bool handleMouseButtonUp(SDL_Event &ev, Common::Event &event);
	bool remapKey(SDL_Event &ev, Common::Event &event);
};

#endif /* BACKEND_EVENTS_GPH_H */
