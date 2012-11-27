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

#include "gob/iniconfig.h"

namespace Gob {

INIConfig::INIConfig() {
}

INIConfig::~INIConfig() {
	for (ConfigMap::iterator c = _configs.begin(); c != _configs.end(); ++c)
		delete c->_value.config;
}

bool INIConfig::getValue(Common::String &result, const Common::String &file,
		const Common::String &section, const Common::String &key,
		const Common::String &def) {

	Config config;
	if (!getConfig(file, config)) {
		if (!openConfig(file, config)) {
			result = def;
			return false;
		}
	}

	if (!config.config->getKey(key, section, result)) {
		result = def;
		return false;
	}

	return true;
}

bool INIConfig::setValue(const Common::String &file, const Common::String &section,
		const Common::String &key, const Common::String &value) {

	Config config;
	if (!getConfig(file, config))
		if (!createConfig(file, config))
			return false;

	config.config->setKey(key, section, value);
	return true;
}

bool INIConfig::getConfig(const Common::String &file, Config &config) {
	if (!_configs.contains(file))
		return false;

	config = _configs.getVal(file);
	return true;
}

bool INIConfig::openConfig(const Common::String &file, Config &config) {
	config.config  = new Common::ConfigFile();
	config.created = false;

	if (!config.config->loadFromFile(file)) {
		delete config.config;
		config.config = 0;
		return false;
	}


	_configs.setVal(file, config);

	return true;
}

bool INIConfig::createConfig(const Common::String &file, Config &config) {
	config.config  = new Common::ConfigFile();
	config.created = true;

	_configs.setVal(file, config);

	return true;
}

} // End of namespace Gob
