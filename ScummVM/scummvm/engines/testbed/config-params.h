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

#ifndef TESTBED_CONFIG_PARAMS_H
#define TESTBED_CONFIG_PARAMS_H

#include "common/singleton.h"
#include "common/stream.h"
#include "graphics/fontman.h"

class TestbedConfigManager;

namespace Testbed {

class ConfigParams : public Common::Singleton<ConfigParams> {
private:
	friend class Common::Singleton<SingletonBaseType>;
	ConfigParams();

	/**
	 * Private variables related to log files.
	 */
	Common::String _logDirectory;
	Common::String _logFilename;
	Common::WriteStream *_ws;

	/**
	 * Private variable used for font.
	 */
	Graphics::FontManager::FontUsage _displayFont;

	/**
	 * Determines if the user initiated testing session is interactive or not.
	 * Used by various tests to respond accordingly.
	 */
	bool _isInteractive;
	bool _isGameDataFound;
	bool _rerunTests;
	TestbedConfigManager *_testbedConfMan;

public:

	bool isRerunRequired();
	void setRerunFlag(bool flag) { _rerunTests = flag; }

	bool isSessionInteractive() { return _isInteractive; }
	void setSessionAsInteractive(bool status) { _isInteractive = status; }

	bool isGameDataFound() { return _isGameDataFound; }
	void setGameDataFound(bool status) { _isGameDataFound = status; }

	TestbedConfigManager *getTestbedConfigManager() { return _testbedConfMan; }
	void setTestbedConfigManager(TestbedConfigManager* confMan) { _testbedConfMan = confMan; }

	Common::String &getLogDirectory() {	return _logDirectory; }
	void setLogDirectory(const Common::String &dirname) { _logDirectory = dirname; }
	Common::String &getLogFilename() { return _logFilename; }
	void setLogFilename(const Common::String &filename) { _logFilename = filename; }

	Common::WriteStream *getLogWriteStream() { return _ws; }
	Graphics::FontManager::FontUsage getCurrentFontUsageType() { return _displayFont; }
	void setCurrentFontUsageType(Graphics::FontManager::FontUsage f) { _displayFont = f; }

	/**
	 * Note: To enable logging, this function must be called once first.
	 */
	void initLogging(const char *dirname, const char *filename, bool enable = true);
	void initLogging(bool enable = true);

	void deleteWriteStream();
};

/** Shortcut for accessing ConfigParams */
#define ConfParams ConfigParams::instance()

} // End of Namespace Testbed

#endif
