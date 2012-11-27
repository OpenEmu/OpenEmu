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

#include "backends/saves/windows/windows-saves.h"

#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)

#if defined(ARRAYSIZE)
#undef ARRAYSIZE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE // winnt.h defines ARRAYSIZE, but we want our own one...

#include "common/config-manager.h"
#include "common/savefile.h"

WindowsSaveFileManager::WindowsSaveFileManager() {
	char defaultSavepath[MAXPATHLEN];

	OSVERSIONINFO win32OsVersion;
	ZeroMemory(&win32OsVersion, sizeof(OSVERSIONINFO));
	win32OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&win32OsVersion);
	// Check for non-9X version of Windows.
	if (win32OsVersion.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
		// Use the Application Data directory of the user profile.
		if (win32OsVersion.dwMajorVersion >= 5) {
			if (!GetEnvironmentVariable("APPDATA", defaultSavepath, sizeof(defaultSavepath)))
				error("Unable to access application data directory");
		} else {
			if (!GetEnvironmentVariable("USERPROFILE", defaultSavepath, sizeof(defaultSavepath)))
				error("Unable to access user profile directory");

			strcat(defaultSavepath, "\\Application Data");

			// If the directory already exists (as it should in most cases),
			// we don't want to fail, but we need to stop on other errors (such as ERROR_PATH_NOT_FOUND)
			if (!CreateDirectory(defaultSavepath, NULL)) {
				if (GetLastError() != ERROR_ALREADY_EXISTS)
					error("Cannot create Application data folder");
			}
		}

		strcat(defaultSavepath, "\\ScummVM");
		if (!CreateDirectory(defaultSavepath, NULL)) {
			if (GetLastError() != ERROR_ALREADY_EXISTS)
				error("Cannot create ScummVM application data folder");
		}

		strcat(defaultSavepath, "\\Saved games");
		if (!CreateDirectory(defaultSavepath, NULL)) {
			if (GetLastError() != ERROR_ALREADY_EXISTS)
				error("Cannot create ScummVM Saved games folder");
		}

		ConfMan.registerDefault("savepath", defaultSavepath);
	}
}

#endif
