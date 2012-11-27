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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <FUiCtrlMessageBox.h>
#include <FLocales.h>

#include "common/config-manager.h"
#include "common/file.h"
#include "engines/engine.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/fonts/bdf.h"
#include "backends/saves/default/default-saves.h"
#include "backends/events/default/default-events.h"
#include "backends/audiocd/default/default-audiocd.h"
#include "backends/mutex/mutex.h"
#include "backends/fs/fs-factory.h"
#include "backends/timer/bada/timer.h"

#include "backends/platform/bada/form.h"
#include "backends/platform/bada/system.h"
#include "backends/platform/bada/graphics.h"
#include "backends/platform/bada/audio.h"

using namespace Osp::Base;
using namespace Osp::Base::Runtime;
using namespace Osp::Locales;
using namespace Osp::Ui::Controls;
using namespace Osp::System;

#define DEFAULT_CONFIG_FILE "/Home/scummvm.ini"
#define RESOURCE_PATH       "/Res"
#define MUTEX_BUFFER_SIZE 5

//
// BadaFilesystemFactory
//
class BadaFilesystemFactory : public FilesystemFactory {
	AbstractFSNode *makeRootFileNode() const;
	AbstractFSNode *makeCurrentDirectoryFileNode() const;
	AbstractFSNode *makeFileNodePath(const Common::String &path) const;
};

AbstractFSNode *BadaFilesystemFactory::makeRootFileNode() const {
	return new BadaFilesystemNode("/");
}

AbstractFSNode *BadaFilesystemFactory::makeCurrentDirectoryFileNode() const {
	return new BadaFilesystemNode("/Home");
}

AbstractFSNode *BadaFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	AppAssert(!path.empty());
	return new BadaFilesystemNode(path);
}

//
// BadaSaveFileManager
//
struct BadaSaveFileManager : public DefaultSaveFileManager {
	bool removeSavefile(const Common::String &filename);
};

bool BadaSaveFileManager::removeSavefile(const Common::String &filename) {
	Common::String savePathName = getSavePath();

	checkPath(Common::FSNode(savePathName));
	if (getError().getCode() != Common::kNoError) {
		return false;
	}

	// recreate FSNode since checkPath may have changed/created the directory
	Common::FSNode savePath(savePathName);
	Common::FSNode file = savePath.getChild(filename);

	String unicodeFileName;
	StringUtil::Utf8ToString(file.getPath().c_str(), unicodeFileName);

	switch (Osp::Io::File::Remove(unicodeFileName)) {
	case E_SUCCESS:
		return true;

	case E_ILLEGAL_ACCESS:
		setError(Common::kWritePermissionDenied, "Search or write permission denied: " +
						 file.getName());
		break;

	default:
		setError(Common::kPathDoesNotExist, "removeSavefile: '" + file.getName() +
						 "' does not exist or path is invalid");
		break;
	}

	return false;
}

//
// BadaMutexManager
//
struct BadaMutexManager : public MutexManager {
	BadaMutexManager();
	~BadaMutexManager();
	OSystem::MutexRef createMutex();
	void lockMutex(OSystem::MutexRef mutex);
	void unlockMutex(OSystem::MutexRef mutex);
	void deleteMutex(OSystem::MutexRef mutex);
private:
	Mutex *buffer[MUTEX_BUFFER_SIZE];
};

BadaMutexManager::BadaMutexManager() {
	for (int i = 0; i < MUTEX_BUFFER_SIZE; i++) {
		buffer[i] = NULL;
	}
}

BadaMutexManager::~BadaMutexManager() {
	for (int i = 0; i < MUTEX_BUFFER_SIZE; i++) {
		if (buffer[i] != NULL) {
			delete buffer[i];
		}
	}
}

OSystem::MutexRef BadaMutexManager::createMutex() {
	Mutex *mutex = new Mutex();
	mutex->Create();

	for (int i = 0; i < MUTEX_BUFFER_SIZE; i++) {
		if (buffer[i] == NULL) {
			buffer[i] = mutex;
			break;
		}
	}

	return (OSystem::MutexRef) mutex;
}

void BadaMutexManager::lockMutex(OSystem::MutexRef mutex) {
	Mutex *m = (Mutex *)mutex;
	m->Acquire();
}

void BadaMutexManager::unlockMutex(OSystem::MutexRef mutex) {
	Mutex *m = (Mutex *)mutex;
	m->Release();
}

void BadaMutexManager::deleteMutex(OSystem::MutexRef mutex) {
	Mutex *m = (Mutex *)mutex;

	for (int i = 0; i < MUTEX_BUFFER_SIZE; i++) {
		if (buffer[i] == m) {
			buffer[i] = NULL;
		}
	}

	delete m;
}

//
// BadaEventManager
//
struct BadaEventManager : public DefaultEventManager {
	BadaEventManager(Common::EventSource *boss);
	void init();
	int shouldQuit() const;
};

BadaEventManager::BadaEventManager(Common::EventSource *boss) :
	DefaultEventManager(boss) {
}

void BadaEventManager::init() {
	DefaultEventManager::init();

	// theme and vkbd should have now loaded - clear the splash screen
	BadaSystem *system = (BadaSystem *)g_system;
	BadaGraphicsManager *graphics = system->getGraphics();
	if (graphics) {
		graphics->setReady();
		graphics->updateScreen();
	}
}

int BadaEventManager::shouldQuit() const {
	BadaSystem *system = (BadaSystem *)g_system;
	return DefaultEventManager::shouldQuit() || system->isClosing();
}

//
// BadaSystem
//
BadaSystem::BadaSystem(BadaAppForm *appForm) :
	_appForm(appForm),
	_audioThread(0),
	_epoch(0) {
}

result BadaSystem::Construct(void) {
	logEntered();

	_fsFactory = new BadaFilesystemFactory();
	if (!_fsFactory) {
		return E_OUT_OF_MEMORY;
	}

	return E_SUCCESS;
}

BadaSystem::~BadaSystem() {
	logEntered();
}

result BadaSystem::initModules() {
	logEntered();

	_mutexManager = new BadaMutexManager();
	if (!_mutexManager) {
		return E_OUT_OF_MEMORY;
	}

	_timerManager = new BadaTimerManager();
	if (!_timerManager) {
		return E_OUT_OF_MEMORY;
	}

	_savefileManager = new BadaSaveFileManager();
	if (!_savefileManager) {
		return E_OUT_OF_MEMORY;
	}

	_graphicsManager = (GraphicsManager *)new BadaGraphicsManager(_appForm);
	if (!_graphicsManager) {
		return E_OUT_OF_MEMORY;
	}

	// depends on _graphicsManager when ENABLE_VKEYBD enabled
	_eventManager = new BadaEventManager(this);
	if (!_eventManager) {
		return E_OUT_OF_MEMORY;
	}

	_audioThread = new AudioThread();
	if (!_audioThread) {
		return E_OUT_OF_MEMORY;
	}

	_mixer = _audioThread->Construct(this);
	if (!_mixer) {
		return E_OUT_OF_MEMORY;
	}

	_audiocdManager = (AudioCDManager *)new DefaultAudioCDManager();
	if (!_audiocdManager) {
		return E_OUT_OF_MEMORY;
	}

	if (IsFailed(_audioThread->Start())) {
		AppLog("Failed to start audio thread");
		return E_OUT_OF_MEMORY;
	}

	logLeaving();
	return E_SUCCESS;
}

void BadaSystem::initBackend() {
	logEntered();

	// use the mobile device theme
	ConfMan.set("gui_theme", "/Res/scummmobile");

	// allow bada virtual keypad pack to be found
	ConfMan.set("vkeybdpath", "/Res/vkeybd_bada");
	ConfMan.set("vkeybd_pack_name", "vkeybd_bada");

	// set default save path to writable area
	if (!ConfMan.hasKey("savepath")) {
		ConfMan.set("savepath", "/Home/Share");
	}

	// default to no auto-save
	if (!ConfMan.hasKey("autosave_period")) {
		ConfMan.setInt("autosave_period", 0);
	}

	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", false);
	ConfMan.setBool("confirm_exit", false);

	SystemTime::GetTicks(_epoch);

	if (E_SUCCESS != initModules()) {
		AppLog("initModules failed");
	} else {
		OSystem::initBackend();
	}

	// replace kBigGUIFont using the large font from the scummmobile theme
	Common::File fontFile;
	Common::String fileName = "/Res/scummmobile/helvB14-iso-8859-1.fcc";
	BadaFilesystemNode file(fileName);
	if (file.exists()) {
		Common::SeekableReadStream *stream = file.createReadStream();
		if (stream) {
			if (fontFile.open(stream, fileName)) {
				Graphics::BdfFont *font = Graphics::BdfFont::loadFromCache(fontFile);
				if (font) {
					// use this font for the vkbd and on-screen messages
					FontMan.setFont(Graphics::FontManager::kBigGUIFont, font);
				}
			}
		}
	}

	logLeaving();
}

void BadaSystem::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	// allow translations.dat and game .DAT files to be found
	s.addDirectory(RESOURCE_PATH, RESOURCE_PATH, priority);
}

void BadaSystem::destroyBackend() {
	closeAudio();

	delete _graphicsManager;
	_graphicsManager = 0;

	delete _savefileManager;
	_savefileManager = 0;

	delete _fsFactory;
	_fsFactory = 0;

	delete _mixer;
	_mixer = 0;

	delete _audiocdManager;
	_audiocdManager = 0;

	delete _timerManager;
	_timerManager = 0;

	delete _eventManager;
	_eventManager = 0;

	delete _mutexManager;
	_mutexManager = 0;
}

bool BadaSystem::pollEvent(Common::Event &event) {
	return _appForm->pollEvent(event);
}

uint32 BadaSystem::getMillis() {
	long long result, ticks = 0;
	SystemTime::GetTicks(ticks);
	result = ticks - _epoch;
	return result;
}

void BadaSystem::delayMillis(uint msecs) {
	if (!_appForm->isClosing()) {
		Thread::Sleep(msecs);
	}
}

void BadaSystem::updateScreen() {
	if (_graphicsManager != NULL) {
		_graphicsManager->updateScreen();
	}
}

void BadaSystem::getTimeAndDate(TimeDate &td) const {
	DateTime currentTime;

	if (E_SUCCESS == SystemTime::GetCurrentTime(WALL_TIME, currentTime)) {
		td.tm_sec = currentTime.GetSecond();
		td.tm_min = currentTime.GetMinute();
		td.tm_hour = currentTime.GetHour();
		td.tm_mday = currentTime.GetDay();
		td.tm_mon = currentTime.GetMonth();
		td.tm_year = currentTime.GetYear();

		Calendar *calendar = Calendar::CreateInstanceN(CALENDAR_GREGORIAN);
		calendar->SetTime(currentTime);
		td.tm_wday = calendar->GetTimeField(TIME_FIELD_DAY_OF_WEEK) - 1;
		delete calendar;
	}
}

void BadaSystem::fatalError() {
	systemError("ScummVM: Fatal internal error.");
}

void BadaSystem::exitSystem() {
	if (_appForm) {
		closeAudio();
		closeGraphics();
		_appForm->exitSystem();
	}
}

void BadaSystem::logMessage(LogMessageType::Type type, const char *message) {
	if (type == LogMessageType::kError) {
		systemError(message);
	} else {
		AppLog(message);
	}
}

Common::SeekableReadStream *BadaSystem::createConfigReadStream() {
	BadaFilesystemNode file(DEFAULT_CONFIG_FILE);
	return file.createReadStream();
}

Common::WriteStream *BadaSystem::createConfigWriteStream() {
	BadaFilesystemNode file(DEFAULT_CONFIG_FILE);
	return file.createWriteStream();
}

void BadaSystem::closeAudio() {
	if (_audioThread) {
		_audioThread->Stop();
		_audioThread->Join();
		delete _audioThread;
		_audioThread = 0;
	}
}

void BadaSystem::closeGraphics() {
	if (_graphicsManager) {
		delete _graphicsManager;
		_graphicsManager = 0;
	}
}

void BadaSystem::setMute(bool on) {
	// only change mute after eventManager init() has completed
	if (_audioThread) {
		BadaGraphicsManager *graphics = getGraphics();
		if (graphics && graphics->isReady()) {
			_audioThread->setMute(on);
		}
	}
}

int BadaSystem::setVolume(bool up, bool minMax) {
	int level = -1;
	if (_audioThread) {
		level = _audioThread->setVolume(up, minMax);
	}
	return level;
}

//
// create the ScummVM system
//
BadaAppForm *systemStart(Osp::App::Application *app) {
	logEntered();

	BadaAppForm *appForm = new BadaAppForm();
	if (!appForm) {
		AppLog("Failed to create appForm");
		return NULL;
	}

	if (E_SUCCESS != appForm->Construct() ||
			E_SUCCESS != app->GetAppFrame()->GetFrame()->AddControl(*appForm)) {
		delete appForm;
		AppLog("Failed to construct appForm");
		return NULL;
	}

	return appForm;
}

//
// display a fatal error notification
//
void systemError(const char *message) {
	AppLog("Fatal system error: %s", message);

	ArrayList *args = new ArrayList();
	args->Construct();
	args->Add(*(new String(message)));
	Application::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT_ERR, args);

	if (g_system) {
		BadaSystem *system = (BadaSystem *)g_system;
		system->exitSystem();
	}
}
