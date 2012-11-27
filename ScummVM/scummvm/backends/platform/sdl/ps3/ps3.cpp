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

#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h	// sys/stat.h includes sys/time.h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "backends/platform/sdl/ps3/ps3.h"
#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"
#include "backends/saves/default/default-saves.h"
#include "backends/fs/ps3/ps3-fs-factory.h"
#include "backends/events/ps3sdl/ps3sdl-events.h"
#include "backends/mixer/sdl13/sdl13-mixer.h"

#include <dirent.h>
#include <sys/stat.h>

int access(const char *pathname, int mode) {
	struct stat sb;

	if (stat(pathname, &sb) == -1) {
		return -1;
	}

	return 0;
}

OSystem_PS3::OSystem_PS3(Common::String baseConfigName)
	: _baseConfigName(baseConfigName) {
}

void OSystem_PS3::init() {
	// Initialze File System Factory
	_fsFactory = new PS3FilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_PS3::initBackend() {
	ConfMan.set("joystick_num", 0);
	ConfMan.set("vkeybdpath", PREFIX "/data");
	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", true);

	// Create the savefile manager
	if (_savefileManager == 0)
		_savefileManager = new DefaultSaveFileManager(PREFIX "/saves");

	// Create the mixer manager
	if (_mixer == 0) {
		_mixerManager = new Sdl13MixerManager();

		// Setup and start mixer
		_mixerManager->init();
	}

	// Event source
	if (_eventSource == 0)
		_eventSource = new PS3SdlEventSource();

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();
}

Common::String OSystem_PS3::getDefaultConfigFileName() {
	return PREFIX "/" + _baseConfigName;
}

Common::WriteStream *OSystem_PS3::createLogFile() {
	Common::FSNode file(PREFIX "/scummvm.log");
	return file.createWriteStream();
}
