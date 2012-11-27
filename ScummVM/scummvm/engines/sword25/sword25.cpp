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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/algorithm.h"
#include "common/array.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/singleton.h"
#include "common/str-array.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/util.h"

#include "sword25/sword25.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/persistenceservice.h"
#include "sword25/package/packagemanager.h"
#include "sword25/script/script.h"

#include "sword25/gfx/animationtemplateregistry.h"	// Needed so we can destroy the singleton
#include "sword25/gfx/renderobjectregistry.h"		// Needed so we can destroy the singleton
namespace Common {
DECLARE_SINGLETON(Sword25::RenderObjectRegistry);
}
#include "sword25/math/regionregistry.h"			// Needed so we can destroy the singleton

namespace Sword25 {

const char *const PACKAGE_MANAGER = "archiveFS";
const char *const DEFAULT_SCRIPT_FILE = "/system/boot.lua";

Sword25Engine::Sword25Engine(OSystem *syst, const ADGameDescription *gameDesc):
	Engine(syst),
	_gameDescription(gameDesc) {
	// Setup mixer
	syncSoundSettings();

	DebugMan.addDebugChannel(kDebugScript, "Script", "Script debug level");
	DebugMan.addDebugChannel(kDebugScript, "Scripts", "Script debug level");
	DebugMan.addDebugChannel(kDebugSound, "Sound", "Sound debug level");

	_console = new Sword25Console(this);
}

Sword25Engine::~Sword25Engine() {
	DebugMan.clearAllDebugChannels();
	delete _console;
}

Common::Error Sword25Engine::run() {
	// Engine initialisation
	Common::Error error = appStart();
	if (error.getCode() != Common::kNoError) {
		appEnd();
		return error;
	}

	// Run the game
	bool runSuccess = appMain();

	// Engine de-initialisation
	bool deinitSuccess = appEnd();

	return (runSuccess && deinitSuccess) ? Common::kNoError : Common::kUnknownError;
}

Common::Error Sword25Engine::appStart() {
	// Initialize the graphics mode to ARGB8888
	Graphics::PixelFormat format = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
	initGraphics(800, 600, true, &format);
	if (format != g_system->getScreenFormat())
		return Common::kUnsupportedColorMode;

	// Kernel initialization
	if (!Kernel::getInstance()->getInitSuccess()) {
		error("Kernel initialization failed.");
		return Common::kUnknownError;
	}

	// Load packages
	PackageManager *packageManagerPtr = Kernel::getInstance()->getPackage();
	if (getGameFlags() & GF_EXTRACTED) {
		if (!packageManagerPtr->loadDirectoryAsPackage(ConfMan.get("path"), "/"))
			return Common::kUnknownError;
	} else {
		if (!loadPackages())
			return Common::kUnknownError;
	}

	// Pass the command line to the script engine.
	ScriptEngine *scriptPtr = Kernel::getInstance()->getScript();
	if (!scriptPtr) {
		error("Script intialization failed.");
		return Common::kUnknownError;
	}

	// Set the game target for use in savegames
	setGameTarget(_targetName.c_str());

	Common::StringArray commandParameters;
	scriptPtr->setCommandLine(commandParameters);

	return Common::kNoError;
}

bool Sword25Engine::appMain() {
	// The main script start. This script loads all the other scripts and starts the actual game.
	ScriptEngine *scriptPtr = Kernel::getInstance()->getScript();
	assert(scriptPtr);
	scriptPtr->executeFile(DEFAULT_SCRIPT_FILE);

	return true;
}

bool Sword25Engine::appEnd() {
	// The kernel is shutdown, and un-initializes all subsystems
	Kernel::deleteInstance();

	AnimationTemplateRegistry::destroy();
	RenderObjectRegistry::destroy();
	RegionRegistry::destroy();

	return true;
}

bool Sword25Engine::loadPackages() {
	PackageManager *packageManagerPtr = Kernel::getInstance()->getPackage();
	assert(packageManagerPtr);

	// Load the main package
	if (!packageManagerPtr->loadPackage("data.b25c", "/"))
		return false;

	// Get the contents of the main program directory and sort them alphabetically
	Common::FSNode dir(ConfMan.get("path"));
	Common::FSList files;
	if (!dir.isDirectory() || !dir.getChildren(files, Common::FSNode::kListAll)) {
		warning("Game data path does not exist or is not a directory");
		return false;
	}

	Common::sort(files.begin(), files.end());

	// Identify all patch packages
	// The filename of patch packages must have the form patch??.b25c, with the question marks
	// are placeholders for numbers.
	// Since the filenames have been sorted, patches are mounted with low numbers first, through
	// to ones with high numbers. This is important, because newly mount packages overwrite
	// existing files in the virtual file system, if they include files with the same name.
	for (Common::FSList::const_iterator it = files.begin(); it != files.end(); ++it) {
		if (it->getName().matchString("patch???.b25c", true))
			if (!packageManagerPtr->loadPackage(it->getName(), "/"))
				return false;
	}

	// Identify and mount all language packages
	// The filename of the packages have the form lang_*.b25c (eg. lang_de.b25c)
	for (Common::FSList::const_iterator it = files.begin(); it != files.end(); ++it) {
		if (it->getName().matchString("lang_*.b25c", true))
			if (!packageManagerPtr->loadPackage(it->getName(), "/"))
				return false;
	}

	return true;
}

bool Sword25Engine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
	// TODO: Implement more of these features?!
#if 0
	return
		(f == kSupportsSubtitleOptions) ||
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
#endif
}

} // End of namespace Sword25
