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

#if !defined(BACKEND_EVENTS_SDL_WEBOS_H) && !defined(DISABLE_DEFAULT_EVENTMANAGER)
#define BACKEND_EVENTS_SDL_WEBOS_H

#include "backends/events/sdl/sdl-events.h"

/**
 * SDL events manager for WebOS
 */
class WebOSSdlEventSource : public SdlEventSource {
public:
	enum {
		DOUBLETAP_LIMIT = 400,
		MAX_FINGERS = 3,
		MOUSE_DEADZONE_PIXELS = 5,
		QUEUED_DRAG_DELAY = 500,
		QUEUED_KEY_DELAY = 250,
		QUEUED_RUP_DELAY = 50,
		SWIPE_PERCENT_HORIZ = 15,
		SWIPE_PERCENT_VERT = 20
	};
	WebOSSdlEventSource() :
			_gestureDown(false),
			_dragStartTime(0), _dragging(false),
			_curX(0), _curY(0),
			_screenX(0), _screenY(0),
			_trackpadMode(false), _autoDragMode(true),
			_doClick(true),
			_queuedDragTime(0), _queuedEscapeUpTime(0), _queuedSpaceUpTime(0),
			_queuedRUpTime(0),
			_firstPoll(true) {
		for (int i = 0; i < MAX_FINGERS; i++) {
			_fingerDown[i] = false;
			_screenDownTime[i] = _dragDiffX[i] = _dragDiffY[i] = 0;
		}
	};
protected:
	// Inidicates if gesture area is pressed down or not.
	bool _gestureDown;

	// The timestamp when screen was pressed down for each finger.
	uint32 _screenDownTime[MAX_FINGERS];

	// The timestamp when a possible drag operation was triggered.
	uint32 _dragStartTime;

	// The distance each finger traveled from touch to release.
	int _dragDiffX[MAX_FINGERS], _dragDiffY[MAX_FINGERS];

	// Indicates if we are in drag mode.
	bool _dragging;

	// The current mouse position on the screen.
	int _curX, _curY;

	// The current screen dimensions
	int _screenX, _screenY;

	// The drag distance for linear gestures
	int _swipeDistX, _swipeDistY;

	// Indicates if we're in trackpad mode or tap-to-move mode.
	bool _trackpadMode;

	// Indicates if we're in automatic drag mode.
	bool _autoDragMode;

	// Tracks which fingers are currently touching the screen.
	bool _fingerDown[MAX_FINGERS];

	// Indicates if a click should be executed when the first finger is lifted
	bool _doClick;

	// Indicates whether the event poll has been run before
	bool _firstPoll;

	// Event queues
	uint32 _queuedDragTime, _queuedEscapeUpTime, _queuedSpaceUpTime,
		_queuedRUpTime;

	// SDL overrides
	virtual void SDLModToOSystemKeyFlags(SDLMod mod, Common::Event &event);
	virtual bool handleKeyDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleKeyUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonDown(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseButtonUp(SDL_Event &ev, Common::Event &event);
	virtual bool handleMouseMotion(SDL_Event &ev, Common::Event &event);
	virtual bool pollEvent(Common::Event &event);

	// Utility functions
	void calculateDimensions();
};

#endif
