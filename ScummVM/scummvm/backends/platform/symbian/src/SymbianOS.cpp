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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <sdlapp.h> // for CSDLApp::GetExecutablePathCStr() @ Symbian::GetExecutablePath()
#include <bautils.h>
#include <eikenv.h> // for CEikonEnv::Static()
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#define FORBIDDEN_SYMBOL_EXCEPTION_fopen

#include "backends/platform/symbian/src/SymbianOS.h"
#include "backends/platform/symbian/src/SymbianActions.h"
#include "common/config-manager.h"
#include "common/scummsys.h"
#include "common/translation.h"

#include "gui/message.h"

#include "backends/fs/symbian/symbian-fs-factory.h"
#include "backends/saves/default/default-saves.h"
#include "backends/events/symbiansdl/symbiansdl-events.h"
#include "backends/graphics/symbiansdl/symbiansdl-graphics.h"
#include "backends/mixer/symbiansdl/symbiansdl-mixer.h"

#define DEFAULT_CONFIG_FILE "scummvm.ini"
#define DEFAULT_SAVE_PATH "Savegames"

////////// extern "C" ///////////////////////////////////////////////////
namespace Symbian {

// make this easily available everywhere
char *GetExecutablePath() {
	return CSDLApp::GetExecutablePathCStr();
}

} // namespace Symbian {

////////// OSystem_SDL_Symbian //////////////////////////////////////////

OSystem_SDL_Symbian::OSystem_SDL_Symbian()
	:
	_RFs(0) {

}

void OSystem_SDL_Symbian::init() {
	_RFs = &CEikonEnv::Static()->FsSession();
	_fsFactory = new SymbianFilesystemFactory();
	OSystem_SDL::init();
}

void OSystem_SDL_Symbian::initBackend() {
	// Calculate the default savepath
	Common::String savePath;
	savePath = Symbian::GetExecutablePath();
	savePath += DEFAULT_SAVE_PATH "\\";
	_savefileManager = new DefaultSaveFileManager(savePath);

	// If savepath has not already been set then set it
	if (!ConfMan.hasKey("savepath")) {
		ConfMan.set("savepath", savePath);
	}

	// Ensure that the current set path (might have been altered by the user) exists
	Common::String currentPath = ConfMan.get("savepath");
	TFileName fname;
	TPtrC8 ptr((const unsigned char*)currentPath.c_str(), currentPath.size());
	fname.Copy(ptr);
	BaflUtils::EnsurePathExistsL(static_cast<OSystem_SDL_Symbian *>(g_system)->FsSession(), fname);

	ConfMan.setBool("FM_high_quality", false);
#if !defined(S60) || defined(S60V3) // S60 has low quality as default
	ConfMan.setBool("FM_medium_quality", true);
#else
	ConfMan.setBool("FM_medium_quality", false);
#endif
	// Symbian OS  should have joystick_num set to 0 in the ini file,
	// but uiq devices might refuse opening the joystick
	ConfMan.setInt("joystick_num", 0);
	ConfMan.flushToDisk();

	GUI::Actions::init();

	// Creates the backend managers
	if (_eventSource == 0)
		_eventSource = new SymbianSdlEventSource();
	if (_mixerManager == 0) {
		_mixerManager = new SymbianSdlMixerManager();

		// Setup and start mixer
		_mixerManager->init();
	}
	if (_graphicsManager == 0)
		_graphicsManager = new SymbianSdlGraphicsManager(_eventSource);

	// Call parent implementation of this method
	OSystem_SDL::initBackend();

	// Initialize global key mapping for Smartphones
	GUI::Actions* actions = GUI::Actions::Instance();

	actions->initInstanceMain(this);
	actions->loadMapping();
}

void OSystem_SDL_Symbian::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	Common::FSNode pluginsNode(Symbian::GetExecutablePath());
	if (pluginsNode.exists() && pluginsNode.isDirectory()) {
			s.add("SYMBIAN_DATAFOLDER", new Common::FSDirectory(Symbian::GetExecutablePath()), priority);
		}
}

void OSystem_SDL_Symbian::quitWithErrorMsg(const char * /*aMsg*/) {
	CEikonEnv::Static()->AlertWin(_L("quitWithErrorMsg()")) ;

	if (g_system)
		g_system->quit();
}

void OSystem_SDL_Symbian::quit() {
	delete GUI_Actions::Instance();

	// Call parent implementation of this method
	OSystem_SDL::quit();
}

void OSystem_SDL_Symbian::engineInit() {
	// Check mappings for the engine just started
	checkMappings();
}

void OSystem_SDL_Symbian::engineDone() {
	// Need to reset engine to basic state after an engine has been running
	GUI::Actions::Instance()->initInstanceMain(this);
}

void OSystem_SDL_Symbian::checkMappings() {
	if (ConfMan.get("gameid").empty() || GUI::Actions::Instance()->initialized())
		return;

	GUI::Actions::Instance()->initInstanceGame();
}

// make sure we always go to normal, even if the string might be set wrong!
bool OSystem_SDL_Symbian::setGraphicsMode(const char * /*name*/) {
	return _graphicsManager->setGraphicsMode(0);
}

Common::String OSystem_SDL_Symbian::getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];
	strcpy(configFile, Symbian::GetExecutablePath());
	strcat(configFile, DEFAULT_CONFIG_FILE);
	return configFile;
}

void OSystem_SDL_Symbian::setupIcon() {
	// Don't for Symbian: it uses the EScummVM.aif file for the icon.
}

RFs& OSystem_SDL_Symbian::FsSession() {
	return *_RFs;
}

// Symbian bsearch implementation is flawed
void* scumm_bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *)) {
	// Perform binary search
	size_t lo = 0;
	size_t hi = nmemb;
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		const void *p = ((const char *)base) + mid * size;
		int tmp = (*compar)(key, p);
		if (tmp < 0)
			hi = mid;
		else if (tmp > 0)
			lo = mid + 1;
		else
			return (void *)p;
	}

	return NULL;
}
