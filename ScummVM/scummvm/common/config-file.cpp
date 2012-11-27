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

#include "common/config-file.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

namespace Common {

bool ConfigFile::isValidName(const String &name) {
	const char *p = name.c_str();
	while (*p && (isAlnum(*p) || *p == '-' || *p == '_' || *p == '.'))
		p++;
	return *p == 0;
}

ConfigFile::ConfigFile() {
}

ConfigFile::~ConfigFile() {
}

void ConfigFile::clear() {
	_sections.clear();
}

bool ConfigFile::loadFromFile(const String &filename) {
	File file;
	if (file.open(filename))
		return loadFromStream(file);
	else
		return false;
}

bool ConfigFile::loadFromSaveFile(const char *filename) {
	assert(g_system);
	SaveFileManager *saveFileMan = g_system->getSavefileManager();
	SeekableReadStream *loadFile;

	assert(saveFileMan);
	if (!(loadFile = saveFileMan->openForLoading(filename)))
		return false;

	bool status = loadFromStream(*loadFile);
	delete loadFile;
	return status;
}

bool ConfigFile::loadFromStream(SeekableReadStream &stream) {
	Section section;
	KeyValue kv;
	String comment;
	int lineno = 0;

	// TODO: Detect if a section occurs multiple times (or likewise, if
	// a key occurs multiple times inside one section).

	while (!stream.eos() && !stream.err()) {
		lineno++;

		// Read a line
		String line = stream.readLine();

		if (line.size() == 0) {
			// Do nothing
		} else if (line[0] == '#' || line[0] == ';' || line.hasPrefix("//")) {
			// Accumulate comments here. Once we encounter either the start
			// of a new section, or a key-value-pair, we associate the value
			// of the 'comment' variable with that entity. The semicolon and
			// C++-style comments are used for Living Books games in Mohawk.
			comment += line;
			comment += "\n";
		} else if (line[0] == '(') {
			// HACK: The following is a hack added by Kirben to support the
			// "map.ini" used in the HE SCUMM game "SPY Fox in Hold the Mustard".
			//
			// It would be nice if this hack could be restricted to that game,
			// but the current design of this class doesn't allow to do that
			// in a nice fashion (a "isMustard" parameter is *not* a nice
			// solution).
			comment += line;
			comment += "\n";
		} else if (line[0] == '[') {
			// It's a new section which begins here.
			const char *p = line.c_str() + 1;
			// Get the section name, and check whether it's valid (that
			// is, verify that it only consists of alphanumerics,
			// periods, dashes and underscores). Mohawk Living Books games
			// can have periods in their section names.
			while (*p && (isAlnum(*p) || *p == '-' || *p == '_' || *p == '.'))
				p++;

			if (*p == '\0')
				error("ConfigFile::loadFromStream: missing ] in line %d", lineno);
			else if (*p != ']')
				error("ConfigFile::loadFromStream: Invalid character '%c' occurred in section name in line %d", *p, lineno);

			// Previous section is finished now, store it.
			if (!section.name.empty())
				_sections.push_back(section);

			section.name = String(line.c_str() + 1, p);
			section.keys.clear();
			section.comment = comment;
			comment.clear();

			assert(isValidName(section.name));
		} else {
			// This line should be a line with a 'key=value' pair, or an empty one.

			// Skip leading whitespaces
			const char *t = line.c_str();
			while (isSpace(*t))
				t++;

			// Skip empty lines / lines with only whitespace
			if (*t == 0)
				continue;

			// If no section has been set, this config file is invalid!
			if (section.name.empty()) {
				error("ConfigFile::loadFromStream: Key/value pair found outside a section in line %d", lineno);
			}

			// Split string at '=' into 'key' and 'value'. First, find the "=" delimeter.
			const char *p = strchr(t, '=');
			if (!p)
				error("Config file buggy: Junk found in line line %d: '%s'", lineno, t);

			// Extract the key/value pair
			kv.key = String(t, p);
			kv.value = String(p + 1);

			// Trim of spaces
			kv.key.trim();
			kv.value.trim();

			// Store comment
			kv.comment = comment;
			comment.clear();

			assert(isValidName(kv.key));

			section.keys.push_back(kv);
		}
	}

	// Save last section
	if (!section.name.empty())
		_sections.push_back(section);

	return (!stream.err() || stream.eos());
}

bool ConfigFile::saveToFile(const String &filename) {
	DumpFile file;
	if (file.open(filename))
		return saveToStream(file);
	else
		return false;
}

bool ConfigFile::saveToSaveFile(const char *filename) {
	assert(g_system);
	SaveFileManager *saveFileMan = g_system->getSavefileManager();
	WriteStream *saveFile;

	assert(saveFileMan);
	if (!(saveFile = saveFileMan->openForSaving(filename)))
		return false;

	bool status = saveToStream(*saveFile);
	delete saveFile;
	return status;
}

bool ConfigFile::saveToStream(WriteStream &stream) {
	for (List<Section>::iterator i = _sections.begin(); i != _sections.end(); ++i) {
		// Write out the section comment, if any
		if (! i->comment.empty()) {
			stream.writeString(i->comment);
		}

		// Write out the section name
		stream.writeByte('[');
		stream.writeString(i->name);
		stream.writeByte(']');
		stream.writeByte('\n');

		// Write out the key/value pairs
		for (List<KeyValue>::iterator kv = i->keys.begin(); kv != i->keys.end(); ++kv) {
			// Write out the comment, if any
			if (! kv->comment.empty()) {
				stream.writeString(kv->comment);
			}
			// Write out the key/value pair
			stream.writeString(kv->key);
			stream.writeByte('=');
			stream.writeString(kv->value);
			stream.writeByte('\n');
		}
	}

	stream.flush();
	return !stream.err();
}


void ConfigFile::removeSection(const String &section) {
	assert(isValidName(section));
	for (List<Section>::iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (section.equalsIgnoreCase(i->name)) {
			_sections.erase(i);
			return;
		}
	}
}

bool ConfigFile::hasSection(const String &section) const {
	assert(isValidName(section));
	const Section *s = getSection(section);
	return s != 0;
}

void ConfigFile::renameSection(const String &oldName, const String &newName) {
	assert(isValidName(oldName));
	assert(isValidName(newName));

	Section *os = getSection(oldName);
	const Section *ns = getSection(newName);
	if (os) {
		// HACK: For now we just print a warning, for more info see the TODO
		// below.
		if (ns)
			warning("ConfigFile::renameSection: Section name \"%s\" already used", newName.c_str());
		else
			os->name = newName;
	}
	// TODO: Check here whether there already is a section with the
	// new name. Not sure how to cope with that case, we could:
	// - simply remove the existing "newName" section
	// - error out
	// - merge the two sections "oldName" and "newName"
}


bool ConfigFile::hasKey(const String &key, const String &section) const {
	assert(isValidName(key));
	assert(isValidName(section));

	const Section *s = getSection(section);
	if (!s)
		return false;
	return s->hasKey(key);
}

void ConfigFile::removeKey(const String &key, const String &section) {
	assert(isValidName(key));
	assert(isValidName(section));

	Section *s = getSection(section);
	if (s)
		 s->removeKey(key);
}

bool ConfigFile::getKey(const String &key, const String &section, String &value) const {
	assert(isValidName(key));
	assert(isValidName(section));

	const Section *s = getSection(section);
	if (!s)
		return false;
	const KeyValue *kv = s->getKey(key);
	if (!kv)
		return false;
	value = kv->value;
	return true;
}

void ConfigFile::setKey(const String &key, const String &section, const String &value) {
	assert(isValidName(key));
	assert(isValidName(section));
	// TODO: Verify that value is valid, too. In particular, it shouldn't
	// contain CR or LF...

	Section *s = getSection(section);
	if (!s) {
		KeyValue newKV;
		newKV.key = key;
		newKV.value = value;

		Section newSection;
		newSection.name = section;
		newSection.keys.push_back(newKV);

		_sections.push_back(newSection);
	} else {
		s->setKey(key, value);
	}
}

const ConfigFile::SectionKeyList ConfigFile::getKeys(const String &section) const {
	const Section *s = getSection(section);

	return s->getKeys();
}

ConfigFile::Section *ConfigFile::getSection(const String &section) {
	for (List<Section>::iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (section.equalsIgnoreCase(i->name)) {
			return &(*i);
		}
	}
	return 0;
}

const ConfigFile::Section *ConfigFile::getSection(const String &section) const {
	for (List<Section>::const_iterator i = _sections.begin(); i != _sections.end(); ++i) {
		if (section.equalsIgnoreCase(i->name)) {
			return &(*i);
		}
	}
	return 0;
}

bool ConfigFile::Section::hasKey(const String &key) const {
	return getKey(key) != 0;
}

const ConfigFile::KeyValue* ConfigFile::Section::getKey(const String &key) const {
	for (List<KeyValue>::const_iterator i = keys.begin(); i != keys.end(); ++i) {
		if (key.equalsIgnoreCase(i->key)) {
			return &(*i);
		}
	}
	return 0;
}

void ConfigFile::Section::setKey(const String &key, const String &value) {
	for (List<KeyValue>::iterator i = keys.begin(); i != keys.end(); ++i) {
		if (key.equalsIgnoreCase(i->key)) {
			i->value = value;
			return;
		}
	}

	KeyValue newKV;
	newKV.key = key;
	newKV.value = value;
	keys.push_back(newKV);
}

void ConfigFile::Section::removeKey(const String &key) {
	for (List<KeyValue>::iterator i = keys.begin(); i != keys.end(); ++i) {
		if (key.equalsIgnoreCase(i->key)) {
			keys.erase(i);
			return;
		}
	}
}

}	// End of namespace Common
