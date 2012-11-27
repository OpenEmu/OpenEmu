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

#include "backends/platform/wince/wince-sdl.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/util.h"
#include "common/textconsole.h"
#include "common/timer.h"
#include "common/translation.h"

#include "engines/engine.h"

#include "base/main.h"
#include "base/plugins.h"

#include "audio/mixer_intern.h"
#include "audio/fmopl.h"

#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/timer/sdl/sdl-timer.h"

#include "gui/Actions.h"
#include "gui/KeysDialog.h"
#include "gui/message.h"

#include "backends/platform/wince/CEActionsPocket.h"
#include "backends/platform/wince/CEActionsSmartphone.h"
#include "backends/platform/wince/CEgui/ItemAction.h"

#include "graphics/scaler/downscaler.h"
#include "graphics/scaler/aspect.h"

#include "backends/platform/wince/CEException.h"
#include "backends/platform/wince/CEScaler.h"

#include "backends/graphics/wincesdl/wincesdl-graphics.h"
#include "backends/events/wincesdl/wincesdl-events.h"
#include "backends/mixer/wincesdl/wincesdl-mixer.h"

#ifdef DYNAMIC_MODULES
#include <malloc.h>
#include "backends/plugins/win32/win32-provider.h"
#endif

#ifdef __GNUC__
extern "C" _CRTIMP FILE *__cdecl   _wfreopen(const wchar_t *, const wchar_t *, FILE *);
#endif

#ifdef WRAP_MALLOC

extern "C" void *__real_malloc(size_t size);
extern "C" void __real_free(void *ptr);

extern "C" void *__wrap_malloc(size_t size) {
/*
	void *ptr = __real_malloc(size);
	printf("malloc(%d) = %p\n", size, ptr);
	return ptr;
*/
	if (size < 64 * 1024) {
		void *ptr = __real_malloc(size+4);
//		printf("malloc(%d) = %p\n", size, ptr);
		if (ptr != NULL) {
			*((HANDLE *)ptr) = 0;
			return 4+(char *)ptr;
		}
		return NULL;
	}
	HANDLE H = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, size+4, 0);
	void *ptr = MapViewOfFile(H, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	*((HANDLE *)ptr) = H;
	return 4+(char *)ptr;
}

extern "C" void __wrap_free(void *ptr) {
/*
	__real_free(ptr);
	printf("free(%p)\n", ptr);
*/
	if (ptr != NULL) {
		HANDLE H = *(HANDLE *)((char *)ptr-4);
		if (H == 0) {
			__real_free((char *)ptr-4);
			return;
		}
		UnmapViewOfFile((char *)ptr-4);
		CloseHandle(H);
	}
}

#endif

using namespace CEGUI;

// ********************************************************************************************

// stdin/err redirection
#define STDOUT_FNAME "\\scummvm_stdout.txt"
#define STDERR_FNAME "\\scummvm_stderr.txt"
static FILE *stdout_file = NULL, *stderr_file = NULL;
static char stdout_fname[MAX_PATH], stderr_fname[MAX_PATH];

// Static member inits
typedef void (*SoundProc)(void *param, byte *buf, int len);
bool OSystem_WINCE3::_soundMaster = true;

bool _isSmartphone = false;
bool _hasSmartphoneResolution = false;

#define DEFAULT_CONFIG_FILE "scummvm.ini"

// ********************************************************************************************

bool isSmartphone() {
	//return _isSmartphone;
	return _hasSmartphoneResolution;
}

const TCHAR *ASCIItoUnicode(const char *str) {
	static TCHAR ustr[MAX_PATH];    // size good enough

	MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, ustr, sizeof(ustr) / sizeof(TCHAR));
	return ustr;
}

// MAIN
#ifndef __GNUC__
int handleException(EXCEPTION_POINTERS *exceptionPointers) {
	CEException::writeException(TEXT("\\scummvmCrash"), exceptionPointers);
	drawError("Unrecoverable exception occurred - see crash dump in latest \\scummvmCrash file");
	fclose(stdout_file);
	fclose(stderr_file);
	CEDevice::end();
	SDL_Quit();
	exit(0);
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

extern "C" char *getcwd(char *buf, int size);
int SDL_main(int argc, char **argv) {
	FILE *newfp = NULL;
#ifdef __GNUC__
	// Due to incomplete crt0.o implementation, we go through the constructor function
	// list provided by the linker and init all of them
	// thanks to joostp and DJWillis
	extern void (*__CTOR_LIST__)();
	void (**constructor)() = &__CTOR_LIST__;
	constructor++;  // First item in list of constructors has special meaning (platform dependent), ignore it.
	while (*constructor) {
		(*constructor)();
		constructor++;
	}
#endif

	CEDevice::init();

	/* Redirect standard input and standard output */
	strcpy(stdout_fname, getcwd(NULL, MAX_PATH));
	strcpy(stderr_fname, getcwd(NULL, MAX_PATH));
	strcat(stdout_fname, STDOUT_FNAME);
	strcat(stderr_fname, STDERR_FNAME);
#ifndef __GNUC__
	stdout_file = fopen(stdout_fname, "w");
	stderr_file = fopen(stderr_fname, "w");
#else
	stdout_file = newfp = _wfreopen(ASCIItoUnicode(stdout_fname), TEXT("w"), stdout);
	if (newfp == NULL) {
#if !defined(stdout)
		stdout = fopen(stdout_fname, "w");
		stdout_file = stdout;
#else
		newfp = fopen(stdout_fname, "w");
		if (newfp) {
			//*stdout = *newfp;
			stdout_file = stdout;
		}
#endif
	}
	stderr_file = newfp = _wfreopen(ASCIItoUnicode(stderr_fname), TEXT("w"), stderr);
	if (newfp == NULL) {
#if !defined(stderr)
		stderr = fopen(stderr_fname, "w");
		stderr_file = stderr;
#else
		newfp = fopen(stderr_fname, "w");
		if (newfp) {
			//*stderr = *newfp;
			stderr_file = stderr;
		}
#endif
	}
#endif

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new Win32PluginProvider());
#endif


	int res = 0;
#if !defined(DEBUG) && !defined(__GNUC__)
	__try {
#endif
		g_system = new OSystem_WINCE3();
		assert(g_system);

		// Pre initialize the backend
		((OSystem_WINCE3 *)g_system)->init();

		// Invoke the actual ScummVM main entry point:
		res = scummvm_main(argc, argv);

		// Free OSystem
		delete(OSystem_WINCE3 *)g_system;
#if !defined(DEBUG) && !defined(__GNUC__)
	}
	__except(handleException(GetExceptionInformation())) {
	}
#endif

	return res;
}

#ifdef DYNAMIC_MODULES

/* This is the OS startup code in the case of a plugin-enabled build.
 * It contains copied and slightly modified parts of SDL's win32/ce startup functions.
 * We copy these here because the calling stub already has a WinMain procedure
 * which overrides SDL's one and hence we essentially re-implement the startup procedure.
 * Note also that this has to be here and not in the stub because SDL is statically
 * linked in the scummvm.dll archive.
 * Take a look at the comments in stub.cpp as well.
 */

int console_main(int argc, char *argv[]) {
	int n;
	char *bufp, *appname;

	appname = argv[0];
	if ((bufp = strrchr(argv[0], '\\')) != NULL)
		appname = bufp + 1;
	else if ((bufp = strrchr(argv[0], '/')) != NULL)
		appname = bufp + 1;

	if ((bufp = strrchr(appname, '.')) == NULL)
		n = strlen(appname);
	else
		n = (bufp - appname);

	bufp = (char *) alloca(n + 1);
	strncpy(bufp, appname, n);
	bufp[n] = '\0';
	appname = bufp;

	if (SDL_Init(SDL_INIT_NOPARACHUTE) < 0) {
		error("WinMain() error: %d", SDL_GetError());
		return(FALSE);
	}

	SDL_SetModuleHandle(GetModuleHandle(NULL));

	// Run the application main() code
	SDL_main(argc, argv);

	return(0);
}

static int ParseCommandLine(char *cmdline, char **argv) {
	char *bufp;
	int argc;

	argc = 0;
	for (bufp = cmdline; *bufp;) {
		// Skip leading whitespace
		while (isspace(*bufp))
			++bufp;

		// Skip over argument
		if (*bufp == '"') {
			++bufp;
			if (*bufp) {
				if (argv)
					argv[argc] = bufp;
				++argc;
			}
			// Skip over word
			while (*bufp && (*bufp != '"'))
				++bufp;
		} else {
			if (*bufp) {
				if (argv)
					argv[argc] = bufp;
				++argc;
			}
			// Skip over word
			while (*bufp && ! isspace(*bufp))
				++bufp;
		}
		if (*bufp) {
			if (argv)
				*bufp = '\0';
			++bufp;
		}
	}
	if (argv)
		argv[argc] = NULL;

	return(argc);
}

int dynamic_modules_main(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR szCmdLine, int sw) {
	HINSTANCE handle;
	char **argv;
	int argc;
	char *cmdline;
	wchar_t *bufp;
	int nLen;

	if (wcsncmp(szCmdLine, TEXT("\\"), 1)) {
		nLen = wcslen(szCmdLine) + 128 + 1;
		bufp = (wchar_t *) alloca(nLen * 2);
		wcscpy(bufp, TEXT("\""));
		GetModuleFileName(NULL, bufp + 1, 128 - 3);
		wcscpy(bufp + wcslen(bufp), TEXT("\" "));
		wcsncpy(bufp + wcslen(bufp), szCmdLine, nLen - wcslen(bufp));
	} else
		bufp = szCmdLine;

	nLen = wcslen(bufp) + 1;
	cmdline = (char *) alloca(nLen);
	WideCharToMultiByte(CP_ACP, 0, bufp, -1, cmdline, nLen, NULL, NULL);

	// Parse command line into argv and argc
	argc = ParseCommandLine(cmdline, NULL);
	argv = (char **) alloca((argc + 1) * (sizeof * argv));
	ParseCommandLine(cmdline, argv);

	// fix gdb-emulator combo
	while (argc > 1 && !strstr(argv[0], ".exe")) {
		OutputDebugString(TEXT("SDL: gdb argv[0] fixup\n"));
		*(argv[1] - 1) = ' ';
		int i;
		for (i = 1; i < argc; i++)
			argv[i] = argv[i + 1];
		argc--;
	}

	// Run the main program (after a little SDL initialization)
	return(console_main(argc, argv));

}
#endif

// ********************************************************************************************

// ********************************************************************************************

void pumpMessages() {
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void drawError(char *error) {
	TCHAR errorUnicode[200];
	MultiByteToWideChar(CP_ACP, 0, error, strlen(error) + 1, errorUnicode, sizeof(errorUnicode));
	pumpMessages();
	MessageBox(GetActiveWindow(), errorUnicode, TEXT("ScummVM error"), MB_OK | MB_ICONERROR);
	pumpMessages();
}

// ********************************************************************************************
static Uint32 timer_handler_wrapper(Uint32 interval) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return interval;
}

void OSystem_WINCE3::initBackend() {

	assert(!_inited);

	// Create the backend custom managers
	if (_eventSource == 0)
		_eventSource = new WINCESdlEventSource();

	if (_mixerManager == 0) {
		_mixerManager = new WINCESdlMixerManager();

		// Setup and start mixer
		_mixerManager->init();
	}

	if (_graphicsManager == 0)
		_graphicsManager = new WINCESdlGraphicsManager(_eventSource);

	((WINCESdlEventSource *)_eventSource)->init((WINCESdlGraphicsManager *)_graphicsManager);

	// Call parent implementation of this method
	OSystem_SDL::initBackend();

	// Initialize global key mapping
	GUI::Actions::init();
	GUI_Actions::Instance()->initInstanceMain(this);
	if (!GUI_Actions::Instance()->loadMapping()) {  // error during loading means not present/wrong version
		warning("Setting default action mappings");
		GUI_Actions::Instance()->saveMapping(); // write defaults
	}

	_inited = true;
}

int OSystem_WINCE3::getScreenWidth() {
	return _platformScreenWidth;
}

void OSystem_WINCE3::initScreenInfos() {
	// sdl port ensures that we use correctly full screen
	_isOzone = 0;
	SDL_Rect **r;
	r = SDL_ListModes(NULL, 0);
	_platformScreenWidth = r[0]->w;
	_platformScreenHeight = r[0]->h;
}

int OSystem_WINCE3::getScreenHeight() {
	return _platformScreenHeight;
}

bool OSystem_WINCE3::isOzone() {
	return _isOzone;
}

Common::String OSystem_WINCE3::getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];
	strcpy(configFile, getcwd(NULL, MAX_PATH));
	strcat(configFile, "\\");
	strcat(configFile, DEFAULT_CONFIG_FILE);
	return configFile;
}

// ********************************************************************************************


OSystem_WINCE3::OSystem_WINCE3() : OSystem_SDL(),
	_forcePanelInvisible(false) {
	// Initialze File System Factory
	_fsFactory = new WindowsFilesystemFactory();
	_mixer = 0;
}

OSystem_WINCE3::~OSystem_WINCE3() {
	delete _mixer;
}

void OSystem_WINCE3::swap_sound_master() {
	_soundMaster = !_soundMaster;

	//WINCESdlGraphicsManager _graphicsManager

	if (((WINCESdlGraphicsManager *)_graphicsManager)->_toolbarHandler.activeName() == NAME_MAIN_PANEL)
		((WINCESdlGraphicsManager *)_graphicsManager)->_toolbarHandler.forceRedraw(); // redraw sound icon
}


void OSystem_WINCE3::engineInit() {
	check_mappings(); // called here to initialize virtual keys handling

	((WINCESdlGraphicsManager *)_graphicsManager)->update_game_settings();
	// finalize mixer init
	_mixerManager->init();
}

void OSystem_WINCE3::check_mappings() {
	CEActionsPocket *instance;

	Common::String gameid(ConfMan.get("gameid"));

	if (gameid.empty() || GUI_Actions::Instance()->initialized())
		return;

	GUI_Actions::Instance()->initInstanceGame();
	instance = (CEActionsPocket *)GUI_Actions::Instance();

	// Some games need to map the right click button, signal it here if it wasn't done
	if (instance->needsRightClickMapping()) {
		GUI::KeysDialog *keysDialog = new GUI::KeysDialog(_("Map right click action"));
		while (!instance->getMapping(POCKET_ACTION_RIGHTCLICK)) {
			keysDialog->runModal();
			if (!instance->getMapping(POCKET_ACTION_RIGHTCLICK)) {
				GUI::MessageDialog alert(_("You must map a key to the 'Right Click' action to play this game"));
				alert.runModal();
			}
		}
		delete keysDialog;
	}

	// Map the "hide toolbar" action if needed
	if (instance->needsHideToolbarMapping()) {
		GUI::KeysDialog *keysDialog = new GUI::KeysDialog(_("Map hide toolbar action"));
		while (!instance->getMapping(POCKET_ACTION_HIDE)) {
			keysDialog->runModal();
			if (!instance->getMapping(POCKET_ACTION_HIDE)) {
				GUI::MessageDialog alert(_("You must map a key to the 'Hide toolbar' action to play this game"));
				alert.runModal();
			}
		}
		delete keysDialog;
	}

	// Map the "zoom" actions if needed
	if (instance->needsZoomMapping()) {
		GUI::KeysDialog *keysDialog = new GUI::KeysDialog(_("Map Zoom Up action (optional)"));
		keysDialog->runModal();
		delete keysDialog;
		keysDialog = new GUI::KeysDialog(_("Map Zoom Down action (optional)"));
		keysDialog->runModal();
		delete keysDialog;
	}

	// Extra warning for Zak Mc Kracken
	if (strncmp(gameid.c_str(), "zak", 3) == 0 &&
	        !GUI_Actions::Instance()->getMapping(POCKET_ACTION_HIDE)) {
		GUI::MessageDialog alert(_("Don't forget to map a key to 'Hide Toolbar' action to see the whole inventory"));
		alert.runModal();
	}

}

void OSystem_WINCE3::setGraphicsModeIntern() {
	// Scalers have been pre-selected for the desired mode.
	// No further tuning required.
}

void OSystem_WINCE3::initSDL() {
	// Check if SDL has not been initialized
	if (!_initedSDL) {
		uint32 sdlFlags = SDL_INIT_EVENTTHREAD;
		if (ConfMan.hasKey("disable_sdl_parachute"))
			sdlFlags |= SDL_INIT_NOPARACHUTE;

		if (ConfMan.hasKey("use_GDI") && ConfMan.getBool("use_GDI")) {
			SDL_VideoInit("windib", 0);
			sdlFlags ^= SDL_INIT_VIDEO;
		}

		// Initialize SDL (SDL Subsystems are initiliazed in the corresponding sdl managers)
		if (SDL_Init(sdlFlags) == -1)
			error("Could not initialize SDL: %s", SDL_GetError());

		// Enable unicode support if possible
		SDL_EnableUNICODE(1);

		_initedSDL = true;
	}
}

void OSystem_WINCE3::init() {
	// Create SdlMutexManager instance as the TimerManager relies on the
	// MutexManager being already initialized
	if (_mutexManager == 0)
		_mutexManager = new SdlMutexManager();

	// Create the timer. CE SDL does not support multiple timers (SDL_AddTimer).
	// We work around this by using the SetTimer function, since we only use
	// one timer in scummvm (for the time being)
	if (_timerManager == 0) {
		_timerManager = new DefaultTimerManager();
		SDL_SetTimer(10, &timer_handler_wrapper);
	}

	// Call parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_WINCE3::quit() {
	fclose(stdout_file);
	fclose(stderr_file);
	if (gDebugLevel <= 0) {
		DeleteFile(ASCIItoUnicode(stdout_fname));
		DeleteFile(ASCIItoUnicode(stderr_fname));
	}
	CEDevice::end();
	OSystem_SDL::quit();
}

void OSystem_WINCE3::getTimeAndDate(TimeDate &t) const {
	SYSTEMTIME systime;

	GetLocalTime(&systime);
	t.tm_year   = systime.wYear - 1900;
	t.tm_mon    = systime.wMonth - 1;
	t.tm_mday   = systime.wDay;
	t.tm_hour   = systime.wHour;
	t.tm_min    = systime.wMinute;
	t.tm_sec    = systime.wSecond;
	t.tm_wday   = systime.wDayOfWeek;
}

Common::String OSystem_WINCE3::getSystemLanguage() const {
#ifdef USE_DETECTLANG
	// We can not use "setlocale" (at least not for MSVC builds), since it
	// will return locales like: "English_USA.1252", thus we need a special
	// way to determine the locale string for Win32.
	char langName[9];
	char ctryName[9];
	TCHAR langNameW[32];
	TCHAR ctryNameW[32];
	int i = 0;
	bool localeFound = false;
	Common::String localeName;

	// Really not nice, but the only way to map Windows CE language/country codes to posix NLS names,
	// because Windows CE doesn't support LOCALE_SISO639LANGNAME and LOCALE_SISO3166CTRYNAME,
	// according to this: http://msdn.microsoft.com/en-us/library/aa912934.aspx
	//
	// See http://msdn.microsoft.com/en-us/goglobal/bb896001.aspx for a translation table
	// This table has to be updated manually when new translations are added
	const char *posixMappingTable[][3] = {
		{"CAT", "ESP", "ca_ES"},
		{"CSY", "CZE", "cs_CZ"},
		{"DAN", "DNK", "da_DA"},
		{"DEU", "DEU", "de_DE"},
		{"ESN", "ESP", "es_ES"},
		{"ESP", "ESP", "es_ES"},
		{"FRA", "FRA", "fr_FR"},
		{"HUN", "HUN", "hu_HU"},
		{"ITA", "ITA", "it_IT"},
		{"NOR", "NOR", "nb_NO"},
		{"NON", "NOR", "nn_NO"},
		{"PLK", "POL", "pl_PL"},
		{"PTB", "BRA", "pt_BR"},
		{"RUS", "RUS", "ru_RU"},
		{"SVE", "SWE", "se_SE"},
		{"UKR", "UKR", "uk_UA"},
		{NULL, NULL, NULL}
	};

	if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, langNameW, sizeof(langNameW)) != 0 &&
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVCTRYNAME, ctryNameW, sizeof(ctryNameW)) != 0) {
		WideCharToMultiByte(CP_ACP, 0, langNameW, -1, langName, (wcslen(langNameW) + 1), NULL, NULL);
		WideCharToMultiByte(CP_ACP, 0, ctryNameW, -1, ctryName, (wcslen(ctryNameW) + 1), NULL, NULL);

		debug(1, "Trying to find posix locale name for %s_%s", langName, ctryName);
		while (posixMappingTable[i][0] && !localeFound) {
			if ( (!strcmp(posixMappingTable[i][0], langName) || !strcmp(posixMappingTable[i][0], "*")) &&
				 (!strcmp(posixMappingTable[i][1], ctryName) || !strcmp(posixMappingTable[i][0], "*")) ) {
				localeFound = true;
				localeName = posixMappingTable[i][2];
			}
			i++;
		}
		if (!localeFound) warning("No posix locale name found for %s_%s", langName, ctryName);
	}

	if (localeFound) {
		debug(1, "Found posix locale name: %s", localeName.c_str());
		return localeName;
	} else {
		return ModularBackend::getSystemLanguage();
	}
#else // USE_DETECTLANG
	return ModularBackend::getSystemLanguage();
#endif // USE_DETECTLANG
}

int OSystem_WINCE3::_platformScreenWidth;
int OSystem_WINCE3::_platformScreenHeight;
bool OSystem_WINCE3::_isOzone;
