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

#ifndef GOB_INICONFIG_H
#define GOB_INICONFIG_H

#include "common/str.h"
#include "common/config-file.h"
#include "common/hashmap.h"

namespace Gob {

class INIConfig {
public:
	INIConfig();
	~INIConfig();

	bool getValue(Common::String &result, const Common::String &file,
			const Common::String &section, const Common::String &key,
			const Common::String &def = "");

	bool setValue(const Common::String &file, const Common::String &section,
			const Common::String &key, const Common::String &value);

private:
	struct Config {
		Common::ConfigFile *config;
		bool created;
	};

	typedef Common::HashMap<Common::String, Config, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ConfigMap;

	ConfigMap _configs;

	bool getConfig(const Common::String &file, Config &config);

	bool openConfig(const Common::String &file, Config &config);
	bool createConfig(const Common::String &file, Config &config);
};

} // End of namespace Gob

#endif // GOB_INICONFIG_H
