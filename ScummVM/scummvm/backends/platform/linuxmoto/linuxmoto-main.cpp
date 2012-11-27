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

#include "backends/platform/linuxmoto/linuxmoto-sdl.h"
#include "base/main.h"

int main(int argc, char *argv[]) {

	// Create our OSystem instance
	g_system = new OSystem_LINUXMOTO();
	assert(g_system);

	// Pre initialize the backend
	((OSystem_POSIX *)g_system)->init();

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);

	// Free OSystem
	delete (OSystem_LINUXMOTO *)g_system;

	return res;
}
