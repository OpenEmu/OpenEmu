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

#ifndef SWORD25_SCUMMVM_FILE_H
#define SWORD25_SCUMMVM_FILE_H

#include "common/str.h"

namespace Sword25 {

/**
 * The following class acts as a proxy interface to the I/O code, pretending that the ScummVM
 * settings are a properly formatted 'config.lua' file
 */
class Sword25FileProxy {
private:
	Common::String _readData;
	uint _readPos;
	Common::String _settings;

	Common::String formatDouble(double value);
	void setupConfigFile();
	Common::String getLanguage();
	void setLanguage(const Common::String &lang);
	void writeSettings();
	void updateSetting(const Common::String &setting, const Common::String &value);
public:
	Sword25FileProxy(const Common::String &filename, const Common::String &mode);
	~Sword25FileProxy();

	bool eof() const { return _readPos >= _readData.size(); }
	size_t read(void *ptr, size_t size, size_t count);
	size_t write(const char *ptr, size_t count);
};

} // End of namespace Sword25

#endif
