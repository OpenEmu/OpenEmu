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

/*
 * OpenPandora: Device Specific Event Handling.
 */

#if defined(OPENPANDORA)

#include "backends/events/openpandora/op-events.h"
#include "backends/graphics/openpandora/op-graphics.h"
#include "backends/platform/openpandora/op-sdl.h"
#include "backends/platform/openpandora/op-options.h"

#include "common/translation.h"
#include "common/util.h"
#include "common/events.h"

/* Quick default button states for modifiers. */
int BUTTON_STATE_L                  =   false;

enum {
	/* Touchscreen TapMode */
	TAPMODE_LEFT        = 0,
	TAPMODE_RIGHT       = 1,
	TAPMODE_HOVER       = 2,
	TAPMODE_HOVER_DPAD  = 3
};

OPEventSource::OPEventSource()
	: _buttonStateL(false) {
}

/* Custom handleMouseButtonDown/handleMouseButtonUp to deal with 'Tap Mode' for the touchscreen */

bool OPEventSource::handleMouseButtonDown(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT) {
		if (BUTTON_STATE_L == true) /* BUTTON_STATE_L = Left Trigger Held, force Right Click */
			event.type = Common::EVENT_RBUTTONDOWN;
		else if (OP::tapmodeLevel == TAPMODE_LEFT) /* TAPMODE_LEFT = Left Click Tap Mode */
			event.type = Common::EVENT_LBUTTONDOWN;
		else if (OP::tapmodeLevel == TAPMODE_RIGHT) /* TAPMODE_RIGHT = Right Click Tap Mode */
			event.type = Common::EVENT_RBUTTONDOWN;
		else if (OP::tapmodeLevel == TAPMODE_HOVER) /* TAPMODE_HOVER = Hover (No Click) Tap Mode */
			event.type = Common::EVENT_MOUSEMOVE;
		else if (OP::tapmodeLevel == TAPMODE_HOVER_DPAD) /* TAPMODE_HOVER_DPAD = Hover (DPad Clicks) Tap Mode */
			event.type = Common::EVENT_MOUSEMOVE;
		else
			event.type = Common::EVENT_LBUTTONDOWN; /* For normal mice etc. */
	} else if (ev.button.button == SDL_BUTTON_RIGHT)
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

bool OPEventSource::handleMouseButtonUp(SDL_Event &ev, Common::Event &event) {
	if (ev.button.button == SDL_BUTTON_LEFT) {
		if (BUTTON_STATE_L == true) /* BUTTON_STATE_L = Left Trigger Held, force Right Click */
			event.type = Common::EVENT_RBUTTONUP;
		else if (OP::tapmodeLevel == TAPMODE_LEFT) /* TAPMODE_LEFT = Left Click Tap Mode */
			event.type = Common::EVENT_LBUTTONUP;
		else if (OP::tapmodeLevel == TAPMODE_RIGHT) /* TAPMODE_RIGHT = Right Click Tap Mode */
			event.type = Common::EVENT_RBUTTONUP;
		else if (OP::tapmodeLevel == TAPMODE_HOVER) /* TAPMODE_HOVER = Hover (No Click) Tap Mode */
			event.type = Common::EVENT_MOUSEMOVE;
		else if (OP::tapmodeLevel == TAPMODE_HOVER_DPAD) /* TAPMODE_HOVER_DPAD = Hover (DPad Clicks) Tap Mode */
			event.type = Common::EVENT_MOUSEMOVE;
		else
			event.type = Common::EVENT_LBUTTONUP; /* For normal mice etc. */
	} else if (ev.button.button == SDL_BUTTON_RIGHT)
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

/* On the OpenPandora by default the ABXY and L/R Trigger buttons are returned by SDL as
   (A): SDLK_HOME (B): SDLK_END (X): SDLK_PAGEDOWN (Y): SDLK_PAGEUP (L): SDLK_RSHIFT (R): SDLK_RCTRL
*/

bool OPEventSource::remapKey(SDL_Event &ev, Common::Event &event) {

	if (OP::tapmodeLevel == TAPMODE_HOVER_DPAD) {
		switch (ev.key.keysym.sym) {
		case SDLK_LEFT:
			event.type = (ev.type == SDL_KEYDOWN) ? Common::EVENT_LBUTTONDOWN : Common::EVENT_LBUTTONUP;
			processMouseEvent(event, _km.x, _km.y);
			return true;
			break;
		case SDLK_RIGHT:
			event.type = (ev.type == SDL_KEYDOWN) ? Common::EVENT_RBUTTONDOWN : Common::EVENT_RBUTTONUP;
			processMouseEvent(event, _km.x, _km.y);
			return true;
			break;
#if defined(SDL_BUTTON_MIDDLE)
		case SDLK_UP:
			event.type = (ev.type == SDL_KEYDOWN) ? Common::EVENT_MBUTTONDOWN : Common::EVENT_MBUTTONUP;
			processMouseEvent(event, _km.x, _km.y);
			return true;
			break;
#endif
		default:
		  break;
		}
	}

	if (ev.type == SDL_KEYDOWN) {
		switch (ev.key.keysym.sym) {
		case SDLK_HOME:
			event.type = Common::EVENT_LBUTTONDOWN;
			processMouseEvent(event, _km.x, _km.y);
			return true;
			break;
		case SDLK_END:
			event.type = Common::EVENT_RBUTTONDOWN;
			processMouseEvent(event, _km.x, _km.y);
			return true;
			break;
		case SDLK_PAGEDOWN:
			event.type = Common::EVENT_MAINMENU;
			return true;
			break;
		case SDLK_PAGEUP:
			OP::ToggleTapMode();
			if (OP::tapmodeLevel == TAPMODE_LEFT) {
				g_system->displayMessageOnOSD(_("Touchscreen 'Tap Mode' - Left Click"));
			} else if (OP::tapmodeLevel == TAPMODE_RIGHT) {
				g_system->displayMessageOnOSD(_("Touchscreen 'Tap Mode' - Right Click"));
			} else if (OP::tapmodeLevel == TAPMODE_HOVER) {
				g_system->displayMessageOnOSD(_("Touchscreen 'Tap Mode' - Hover (No Click)"));
			} else if (OP::tapmodeLevel == TAPMODE_HOVER_DPAD) {
				g_system->displayMessageOnOSD(_("Touchscreen 'Tap Mode' - Hover (DPad Clicks)"));
			}
			break;
		case SDLK_RSHIFT:
			BUTTON_STATE_L = true;
			break;
		case SDLK_RCTRL:
			break;
		default:
			return false;
			break;
		}
		return false;
	} else {
		switch (ev.key.keysym.sym) {
		case SDLK_HOME:
			event.type = Common::EVENT_LBUTTONUP;
			processMouseEvent(event, _km.x, _km.y);
			return true;
			break;
		case SDLK_END:
			event.type = Common::EVENT_RBUTTONUP;
			processMouseEvent(event, _km.x, _km.y);
			return true;
			break;
		case SDLK_PAGEDOWN:
			event.type = Common::EVENT_MAINMENU;
			return true;
			break;
		case SDLK_PAGEUP:
			break;
		case SDLK_RSHIFT:
			BUTTON_STATE_L = false;
			break;
		case SDLK_RCTRL:
			break;
		default:
			return false;
			break;
		}
		return false;
	}
	return false;
}

#endif
