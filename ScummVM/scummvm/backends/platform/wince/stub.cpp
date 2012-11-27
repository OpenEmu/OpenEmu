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

#include <windows.h>

extern int dynamic_modules_main(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR szCmdLine, int sw);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR szCmdLine, int sw) {

	/* Hello!
	 * This thing looks trivial, right? Guess again :-)
	 * Observe:
	 * 1) Because of the way plugins are done within the scummvm core
	 *    (read: slightly dirty) it is required that the plugins
	 *    (built as dlls in win32 systems) have to "backlink" into the
	 *    main executable. More specifically, the dlls have to call
	 *    functions from the (separately built) main module.
	 *    This means trouble for wince, as the dynamic linker does not
	 *    resolve imported symbols to a dll from an executable.
	 * 2) But wait! DLLs can have any way of importing/exporting symbols
	 *    between them. Hmmm...
	 *
	 * Right! The solution is thus: We build everything as a dll. This means
	 * the individual engines (plugins) and also the scummvm core application.
	 * It is in fact "The sub-DLL solution" described over at edll
	 * (ref. http://edll.sourceforge.net/).
	 *
	 * The last thing to watch out for is that in a plugin build, we already
	 * supply a WinMain function (this one, right here) which supersedes
	 * SDL's one. So we need to do the startup things SDL does in scummvm
	 * before running SDL_main.
	 *
	 * All this leaves us, for this source file, to do this crummy little... */

	dynamic_modules_main(hInst, hPrev, szCmdLine, sw);
}
