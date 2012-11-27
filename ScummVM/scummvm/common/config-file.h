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

#ifndef COMMON_CONFIG_FILE_H
#define COMMON_CONFIG_FILE_H

#include "common/hash-str.h"
#include "common/list.h"
#include "common/str.h"

namespace Common {

class SeekableReadStream;
class WriteStream;

/**
 * This class allows reading/writing INI style config files.
 * It is used by the ConfigManager for storage, but can also
 * be used by other code if it needs to read/write custom INI
 * files.
 *
 * Lines starting with a '#' are ignored (i.e. treated as comments).
 * Some effort is made to preserve comments, though.
 *
 * This class makes no attempts to provide fast access to key/value pairs.
 * In particular, it stores all sections and k/v pairs in lists, not
 * in dictionaries/maps. This makes it very easy to read/write the data
 * from/to files, but of course is not appropriate for fast access.
 * The main reason is that this class is indeed geared toward doing precisely
 * that!
 * If you need fast access to the game config, use higher level APIs, like the
 * one provided by ConfigManager.
 */
class ConfigFile {
public:
	struct KeyValue {
		String key;
		String value;
		String comment;
	};

	typedef List<KeyValue> SectionKeyList;

	/** A section in a config file. I.e. corresponds to something like this:
	 *   [mySection]
	 *   key=value
	 *
	 * Comments are also stored, to keep users happy who like editing their
	 * config files manually.
	 */
	struct Section {
		String name;
		List<KeyValue> keys;
		String comment;

		bool hasKey(const String &key) const;
		const KeyValue* getKey(const String &key) const;
		void setKey(const String &key, const String &value);
		void removeKey(const String &key);
		const SectionKeyList getKeys() const { return keys; }
	};

	typedef List<Section> SectionList;

public:
	ConfigFile();
	~ConfigFile();

	// TODO: Maybe add a copy constructor etc.?

	/**
	 * Check whether the given string is a valid section or key name.
	 * For that, it must only consist of letters, numbers, dashes and
	 * underscores. In particular, white space and "#", "=", "[", "]"
	 * are not valid!
	 */
	static bool isValidName(const String &name);

	/** Reset everything stored in this config file. */
	void	clear();

	bool	loadFromFile(const String &filename);
	bool	loadFromSaveFile(const char *filename);
	bool	loadFromStream(SeekableReadStream &stream);
	bool	saveToFile(const String &filename);
	bool	saveToSaveFile(const char *filename);
	bool	saveToStream(WriteStream &stream);

	bool	hasSection(const String &section) const;
	void	removeSection(const String &section);
	void	renameSection(const String &oldName, const String &newName);

	bool	hasKey(const String &key, const String &section) const;
	bool	getKey(const String &key, const String &section, String &value) const;
	void	setKey(const String &key, const String &section, const String &value);
	void	removeKey(const String &key, const String &section);

	const SectionList getSections() const { return _sections; }
	const SectionKeyList getKeys(const String &section) const;

	void listKeyValues(StringMap &kv);

private:
	SectionList _sections;

	Section *getSection(const String &section);
	const Section *getSection(const String &section) const;
};

/*
- ConfigMan owns a config file
- allow direct access to that config file (for the launcher)
- simplify and unify the regular ConfigMan API in exchange


*/

}	// End of namespace Common

#endif
