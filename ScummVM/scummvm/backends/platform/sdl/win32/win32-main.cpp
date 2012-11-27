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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#ifdef WIN32

// Fix for bug #2895217 "MSVC compilation broken with r47595":
// We need to keep this on top of the "common/scummsys.h"(base/main.h) include,
// otherwise we will get errors about the windows headers redefining
// "ARRAYSIZE" for example.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE // winnt.h defines ARRAYSIZE, but we want our own one...

#include "backends/platform/sdl/win32/win32.h"
#include "backends/plugins/sdl/sdl-provider.h"
#include "base/main.h"

int __stdcall WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/,  LPSTR /*lpCmdLine*/, int /*iShowCmd*/) {
	SDL_SetModuleHandle(GetModuleHandle(NULL));
	return main(__argc, __argv);
}

int main(int argc, char *argv[]) {
	// Create our OSystem instance
	g_system = new OSystem_Win32();
	assert(g_system);

	// Pre initialize the backend
	((OSystem_Win32 *)g_system)->init();

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new SDLPluginProvider());
#endif

	// Invoke the actual ScummVM main entry point:
	int res = scummvm_main(argc, argv);

	// Free OSystem
	delete (OSystem_Win32 *)g_system;

	return res;
}

#endif
