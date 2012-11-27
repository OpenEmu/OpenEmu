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

#include "common/config-manager.h"
#include "common/fs.h"

#include "testbed/config-params.h"

namespace Common {
DECLARE_SINGLETON(Testbed::ConfigParams);
}

namespace Testbed {

ConfigParams::ConfigParams() {
	_logDirectory = "";
	_logFilename = "";
	_ws = 0;
	_displayFont = Graphics::FontManager::kGUIFont;
	_isInteractive = true;
	_isGameDataFound = true;
	_rerunTests = false;
}

void ConfigParams::initLogging(const char *dirname, const char *filename, bool enable) {
	setLogDirectory(dirname);
	setLogFilename(filename);
	if (enable) {
		_ws = Common::FSNode(_logDirectory).getChild(_logFilename).createWriteStream();
	} else {
		_ws = 0;
	}
}

void ConfigParams::initLogging(bool enable) {
	// Default Log Directory is game-data directory and filename is 'testbed.log'.
	initLogging(ConfMan.get("path").c_str(), "testbed.log", enable);
}

bool ConfigParams::isRerunRequired() {
	if (_rerunTests) {
		_rerunTests = false;
		return true;
	}
	return false;
}

void ConfigParams::deleteWriteStream() {
	if (_ws) {
		delete _ws;
	}
}

}	// End of namespace Testbed
