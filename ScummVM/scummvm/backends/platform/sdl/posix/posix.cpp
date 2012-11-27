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

#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_exit
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h	//On IRIX, sys/stat.h includes sys/time.h

#include "common/scummsys.h"

#ifdef POSIX

#include "backends/platform/sdl/posix/posix.h"
#include "backends/saves/posix/posix-saves.h"
#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/taskbar/unity/unity-taskbar.h"

#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>


OSystem_POSIX::OSystem_POSIX(Common::String baseConfigName)
	:
	_baseConfigName(baseConfigName) {
}

void OSystem_POSIX::init() {
	// Initialze File System Factory
	_fsFactory = new POSIXFilesystemFactory();

#if defined(USE_TASKBAR) && defined(USE_TASKBAR_UNITY)
	// Initialize taskbar manager
	_taskbarManager = new UnityTaskbarManager();
#endif

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_POSIX::initBackend() {
	// Create the savefile manager
	if (_savefileManager == 0)
		_savefileManager = new POSIXSaveFileManager();

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();

#if defined(USE_TASKBAR) && defined(USE_TASKBAR_UNITY)
	// Register the taskbar manager as an event source (this is necessary for the glib event loop to be run)
	_eventManager->getEventDispatcher()->registerSource((UnityTaskbarManager *)_taskbarManager, false);
#endif
}

bool OSystem_POSIX::hasFeature(Feature f) {
	if (f == kFeatureDisplayLogFile)
		return true;
	return OSystem_SDL::hasFeature(f);
}

Common::String OSystem_POSIX::getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];

	// On POSIX type systems, by default we store the config file inside
	// to the HOME directory of the user.
	const char *home = getenv("HOME");
	if (home != NULL && strlen(home) < MAXPATHLEN)
		snprintf(configFile, MAXPATHLEN, "%s/%s", home, _baseConfigName.c_str());
	else
		strcpy(configFile, _baseConfigName.c_str());

	return configFile;
}

Common::WriteStream *OSystem_POSIX::createLogFile() {
	// Start out by resetting _logFilePath, so that in case
	// of a failure, we know that no log file is open.
	_logFilePath.clear();

	const char *home = getenv("HOME");
	if (home == NULL)
		return 0;

	Common::String logFile(home);
#ifdef MACOSX
	logFile += "/Library";
#else
	logFile += "/.scummvm";
#endif
#ifdef SAMSUNGTV
	logFile = "/mtd_ram";
#endif

	struct stat sb;

	// Check whether the dir exists
	if (stat(logFile.c_str(), &sb) == -1) {
		// The dir does not exist, or stat failed for some other reason.
		if (errno != ENOENT)
			return 0;

		// If the problem was that the path pointed to nothing, try
		// to create the dir.
		if (mkdir(logFile.c_str(), 0755) != 0)
			return 0;
	} else if (!S_ISDIR(sb.st_mode)) {
		// Path is no directory. Oops
		return 0;
	}

#ifdef MACOSX
	logFile += "/Logs";
#else
	logFile += "/logs";
#endif

	// Check whether the dir exists
	if (stat(logFile.c_str(), &sb) == -1) {
		// The dir does not exist, or stat failed for some other reason.
		if (errno != ENOENT)
			return 0;

		// If the problem was that the path pointed to nothing, try
		// to create the dir.
		if (mkdir(logFile.c_str(), 0755) != 0)
			return 0;
	} else if (!S_ISDIR(sb.st_mode)) {
		// Path is no directory. Oops
		return 0;
	}

	logFile += "/scummvm.log";

	Common::FSNode file(logFile);
	Common::WriteStream *stream = file.createWriteStream();
	if (stream)
		_logFilePath = logFile;
	return stream;
}

bool OSystem_POSIX::displayLogFile() {
	if (_logFilePath.empty())
		return false;

	// FIXME: This may not work perfectly when in fullscreen mode.
	// On my system it drops from fullscreen without ScummVM noticing,
	// so the next Alt-Enter does nothing, going from windowed to windowed.
	// (wjp, 20110604)

	pid_t pid = fork();
	if (pid < 0) {
		// failed to fork
		return false;
	} else if (pid == 0) {

		// Try xdg-open first
		execlp("xdg-open", "xdg-open", _logFilePath.c_str(), (char *)0);

		// If we're here, that clearly failed.

		// TODO: We may also want to try detecting the case where
		// xdg-open is successfully executed but returns an error code.

		// Try xterm+less next

		execlp("xterm", "xterm", "-e", "less", _logFilePath.c_str(), (char *)0);

		// TODO: If less does not exist we could fall back to 'more'.
		// However, we'll have to use 'xterm -hold' for that to prevent the
		// terminal from closing immediately (for short log files) or
		// unexpectedly.

		exit(127);
	}

	int status;
	// Wait for viewer to close.
	// (But note that xdg-open may have spawned a viewer in the background.)

	// FIXME: We probably want the viewer to always open in the background.
	// This may require installing a SIGCHLD handler.
	pid = waitpid(pid, &status, 0);

	if (pid < 0) {
		// Probably nothing sensible to do in this error situation
		return false;
	}

	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}


#endif
