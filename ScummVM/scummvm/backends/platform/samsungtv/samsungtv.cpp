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

#if defined(SAMSUNGTV)

#include "backends/platform/samsungtv/samsungtv.h"
#include "backends/events/samsungtvsdl/samsungtvsdl-events.h"
#include "backends/graphics/samsungtvsdl/samsungtvsdl-graphics.h"
#include "common/textconsole.h"

OSystem_SDL_SamsungTV::OSystem_SDL_SamsungTV()
	:
	OSystem_POSIX("/mtd_rwarea/.scummvmrc") {
}

void OSystem_SDL_SamsungTV::initBackend() {
	// Create the events manager
	if (_eventSource == 0)
		_eventSource = new SamsungTVSdlEventSource();

	if (_graphicsManager == 0)
		_graphicsManager = new SamsungTVSdlGraphicsManager(_eventSource);

	// Call parent implementation of this method
	OSystem_POSIX::initBackend();
}

void OSystem_SDL_SamsungTV::quit() {
	delete this;
}

void OSystem_SDL_SamsungTV::fatalError() {
	delete this;
	// FIXME
	warning("fatal error");
	for (;;) {}
}

#endif
