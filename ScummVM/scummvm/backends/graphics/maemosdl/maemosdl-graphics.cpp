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
#if defined(MAEMO)

#include "SDL_syswm.h"

#include "common/scummsys.h"

#include "backends/platform/maemo/maemo.h"
#include "backends/events/maemosdl/maemosdl-events.h"
#include "backends/graphics/maemosdl/maemosdl-graphics.h"

MaemoSdlGraphicsManager::MaemoSdlGraphicsManager(SdlEventSource *sdlEventSource)
	: SurfaceSdlGraphicsManager(sdlEventSource) {
}

bool MaemoSdlGraphicsManager::loadGFXMode() {
	bool success = SurfaceSdlGraphicsManager::loadGFXMode();

	// fix the problematic zoom key capture in Maemo5/N900
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (SDL_GetWMInfo(&info)) {
		Display *dpy = info.info.x11.display;
		Window win;
		unsigned long val = 1;
		Atom atom_zoom = XInternAtom(dpy, "_HILDON_ZOOM_KEY_ATOM", 0);
		info.info.x11.lock_func();
		win = info.info.x11.fswindow;
		if (win)
			XChangeProperty(dpy, win, atom_zoom, XA_INTEGER, 32, PropModeReplace, (unsigned char *) &val, 1); // grab zoom keys
		win = info.info.x11.wmwindow;
		if (win)
			XChangeProperty(dpy, win, atom_zoom, XA_INTEGER, 32, PropModeReplace, (unsigned char *) &val, 1); // grab zoom keys
		info.info.x11.unlock_func();
	}

	return success;
}

#endif
