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

#ifdef MACOSX

#include "backends/platform/sdl/macosx/macosx.h"
#include "backends/mixer/doublebuffersdl/doublebuffersdl-mixer.h"
#include "backends/platform/sdl/macosx/appmenu_osx.h"
#include "backends/updates/macosx/macosx-updates.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/fs.h"
#include "common/translation.h"

#include "ApplicationServices/ApplicationServices.h"	// for LSOpenFSRef
#include "CoreFoundation/CoreFoundation.h"	// for CF* stuff

OSystem_MacOSX::OSystem_MacOSX()
	:
	OSystem_POSIX("Library/Preferences/ScummVM Preferences") {
}

void OSystem_MacOSX::initBackend() {
	// Create the mixer manager
	if (_mixer == 0) {
		_mixerManager = new DoubleBufferSDLMixerManager();

		// Setup and start mixer
		_mixerManager->init();
	}

#ifdef USE_TRANSLATION
	// We need to initialize the translataion manager here for the following
	// call to replaceApplicationMenuItems() work correctly
	TransMan.setLanguage(ConfMan.get("gui_language").c_str());
#endif // USE_TRANSLATION

	// Replace the SDL generated menu items with our own translated ones on Mac OS X
	replaceApplicationMenuItems();

#ifdef USE_SPARKLE
	// Initialize updates manager
	_updateManager = new MacOSXUpdateManager();
#endif

	// Invoke parent implementation of this method
	OSystem_POSIX::initBackend();
}

void OSystem_MacOSX::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	// Invoke parent implementation of this method
	OSystem_POSIX::addSysArchivesToSearchSet(s, priority);

	// Get URL of the Resource directory of the .app bundle
	CFURLRef fileUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	if (fileUrl) {
		// Try to convert the URL to an absolute path
		UInt8 buf[MAXPATHLEN];
		if (CFURLGetFileSystemRepresentation(fileUrl, true, buf, sizeof(buf))) {
			// Success: Add it to the search path
			Common::String bundlePath((const char *)buf);
			s.add("__OSX_BUNDLE__", new Common::FSDirectory(bundlePath), priority);
		}
		CFRelease(fileUrl);
	}
}

void OSystem_MacOSX::setupIcon() {
	// Don't set icon on OS X, as we use a nicer external icon there.
}

bool OSystem_MacOSX::hasFeature(Feature f) {
	if (f == kFeatureDisplayLogFile)
		return true;
	return OSystem_POSIX::hasFeature(f);
}

bool OSystem_MacOSX::displayLogFile() {
	// Use LaunchServices to open the log file, if possible.

	if (_logFilePath.empty())
		return false;

    CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8 *)_logFilePath.c_str(), _logFilePath.size(), false);
    OSStatus err = LSOpenCFURLRef(url, NULL);
    CFRelease(url);

	return err != noErr;
}

Common::String OSystem_MacOSX::getSystemLanguage() const {
#if defined(USE_DETECTLANG) && defined(USE_TRANSLATION)
	CFArrayRef availableLocalizations = CFBundleCopyBundleLocalizations(CFBundleGetMainBundle());
	if (availableLocalizations) {
		CFArrayRef preferredLocalizations = CFBundleCopyPreferredLocalizationsFromArray(availableLocalizations);
		CFRelease(availableLocalizations);
		if (preferredLocalizations) {
			CFIndex localizationsSize = CFArrayGetCount(preferredLocalizations);
			// Since we have a list of sorted preferred localization, I would like here to
			// check that they are supported by the TranslationManager and take the first
			// one that is supported. The listed localizations are taken from the Bundle
			// plist file, so they should all be supported, unless the plist file is not
			// synchronized with the translations.dat file. So this is not really a big
			// issue. And because getSystemLanguage() is called from the constructor of
			// TranslationManager (therefore before the instance pointer is set), calling
			// TransMan here results in an infinite loop and creation of a lot of TransMan
			// instances.
			/*
			for (CFIndex i = 0 ; i < localizationsSize ; ++i) {
				CFStringRef language = (CFStringRef)CFArrayGetValueAtIndex(preferredLocalizations, i);
				char buffer[10];
				CFStringGetCString(language, buffer, 50, kCFStringEncodingASCII);
				int32 languageId = TransMan.findMatchingLanguage(buffer);
				if (languageId != -1) {
					CFRelease(preferredLocalizations);
					return TransMan.getLangById(languageId);
				}
			}
			*/
			if (localizationsSize > 0) {
				CFStringRef language = (CFStringRef)CFArrayGetValueAtIndex(preferredLocalizations, 0);
				char buffer[10];
				CFStringGetCString(language, buffer, 50, kCFStringEncodingASCII);
				CFRelease(preferredLocalizations);
				return buffer;
			}
			CFRelease(preferredLocalizations);
		}

	}
	// Falback to POSIX implementation
	return OSystem_POSIX::getSystemLanguage();
#else // USE_DETECTLANG
	return OSystem_POSIX::getSystemLanguage();
#endif // USE_DETECTLANG
}

#endif
