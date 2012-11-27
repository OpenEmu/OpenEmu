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


// Enable getenv, mkdir and time.h stuff
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h	//On IRIX, sys/stat.h includes sys/time.h

#include "common/scummsys.h"

#if defined(POSIX) && !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)

#include "backends/saves/posix/posix-saves.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/textconsole.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>


#ifdef MACOSX
#define DEFAULT_SAVE_PATH "Documents/ScummVM Savegames"
#else
#define DEFAULT_SAVE_PATH ".scummvm"
#endif

POSIXSaveFileManager::POSIXSaveFileManager() {
	// Register default savepath based on HOME
#if defined(SAMSUNGTV)
	ConfMan.registerDefault("savepath", "/mtd_wiselink/scummvm savegames");
#else
	Common::String savePath;
	const char *home = getenv("HOME");
	if (home && *home && strlen(home) < MAXPATHLEN) {
		savePath = home;
		savePath += "/" DEFAULT_SAVE_PATH;
		ConfMan.registerDefault("savepath", savePath);
	}

	// The user can override the savepath with the SCUMMVM_SAVEPATH
	// environment variable. This is weaker than a --savepath on the
	// command line, but overrides the default savepath.
	//
	// To ensure that the command line option (if given) has precedence,
	// we only set the value in the transient domain if it is not
	// yet present there.
	if (!ConfMan.hasKey("savepath", Common::ConfigManager::kTransientDomain)) {
		const char *dir = getenv("SCUMMVM_SAVEPATH");
		if (dir && *dir && strlen(dir) < MAXPATHLEN) {
			Common::FSNode saveDir(dir);
			if (!saveDir.exists()) {
				warning("Ignoring non-existent SCUMMVM_SAVEPATH '%s'", dir);
			} else if (!saveDir.isWritable()) {
				warning("Ignoring non-writable SCUMMVM_SAVEPATH '%s'", dir);
			} else {
				ConfMan.set("savepath", dir, Common::ConfigManager::kTransientDomain);
			}
		}
	}
#endif
}

void POSIXSaveFileManager::checkPath(const Common::FSNode &dir) {
	const Common::String path = dir.getPath();
	clearError();

	struct stat sb;

	// Check whether the dir exists
	if (stat(path.c_str(), &sb) == -1) {
		// The dir does not exist, or stat failed for some other reason.
		// If the problem was that the path pointed to nothing, try
		// to create the dir (ENOENT case).
		switch (errno) {
		case EACCES:
			setError(Common::kWritePermissionDenied, "Search or write permission denied: "+path);
			break;
		case ELOOP:
			setError(Common::kUnknownError, "Too many symbolic links encountered while traversing the path: "+path);
			break;
		case ENAMETOOLONG:
			setError(Common::kUnknownError, "The path name is too long: "+path);
			break;
		case ENOENT:
			if (mkdir(path.c_str(), 0755) != 0) {
				// mkdir could fail for various reasons: The parent dir doesn't exist,
				// or is not writeable, the path could be completly bogus, etc.
				warning("mkdir for '%s' failed", path.c_str());
				perror("mkdir");

				switch (errno) {
				case EACCES:
					setError(Common::kWritePermissionDenied, "Search or write permission denied: "+path);
					break;
				case EMLINK:
					setError(Common::kUnknownError, "The link count of the parent directory would exceed {LINK_MAX}: "+path);
					break;
				case ELOOP:
					setError(Common::kUnknownError, "Too many symbolic links encountered while traversing the path: "+path);
					break;
				case ENAMETOOLONG:
					setError(Common::kUnknownError, "The path name is too long: "+path);
					break;
				case ENOENT:
					setError(Common::kPathDoesNotExist, "A component of the path does not exist, or the path is an empty string: "+path);
					break;
				case ENOTDIR:
					setError(Common::kPathDoesNotExist, "A component of the path prefix is not a directory: "+path);
					break;
				case EROFS:
					setError(Common::kWritePermissionDenied, "The parent directory resides on a read-only file system:"+path);
					break;
				}
			}
			break;
		case ENOTDIR:
			setError(Common::kPathDoesNotExist, "A component of the path prefix is not a directory: "+path);
			break;
		}
	} else {
		// So stat() succeeded. But is the path actually pointing to a directory?
		if (!S_ISDIR(sb.st_mode)) {
			setError(Common::kPathDoesNotExist, "The given savepath is not a directory: "+path);
		}
	}
}

#endif
