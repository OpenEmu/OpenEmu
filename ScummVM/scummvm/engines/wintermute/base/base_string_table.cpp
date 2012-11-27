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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_string_table.h"
#include "common/str.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
BaseStringTable::BaseStringTable(BaseGame *inGame) : BaseClass(inGame) {

}


//////////////////////////////////////////////////////////////////////////
BaseStringTable::~BaseStringTable() {
	// delete strings
	_strings.clear();

}


//////////////////////////////////////////////////////////////////////////
bool BaseStringTable::addString(const char *key, const char *val, bool reportDuplicities) {
	if (key == NULL || val == NULL) {
		return STATUS_FAILED;
	}

	if (scumm_stricmp(key, "@right-to-left") == 0) {
		_gameRef->_textRTL = true;
		return STATUS_OK;
	}

	Common::String finalKey = key;
	finalKey.toLowercase();

	StringsIter it = _strings.find(finalKey);
	if (it != _strings.end() && reportDuplicities) {
		_gameRef->LOG(0, "  Warning: Duplicate definition of string '%s'.", finalKey.c_str());
	}

	_strings[finalKey] = val;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
char *BaseStringTable::getKey(const char *str) const {
	if (str == NULL || str[0] != '/') {
		return NULL;
	}

	const char *value = strchr(str + 1, '/');
	if (value == NULL) {
		return NULL;
	}

	char *key = new char[value - str];
	Common::strlcpy(key, str + 1, (size_t)(value - str));

	BasePlatform::strlwr(key);

	char *newStr;

	StringsIter it = _strings.find(key);
	if (it != _strings.end()) {
		newStr = new char[it->_value.size() + 1];
		strcpy(newStr, it->_value.c_str());
		if (strlen(newStr) > 0 && newStr[0] == '/' && strchr(newStr + 1, '/')) {
			delete[] key;
			char *ret = getKey(newStr);
			delete[] newStr;
			return ret;
		} else {
			delete[] newStr;
			return key;
		}
	} else {
		return key;
	}
}

//////////////////////////////////////////////////////////////////////////
void BaseStringTable::expand(char **str) const {
	if (str == NULL || *str == NULL || *str[0] != '/') {
		return;
	}

	char *value = strchr(*str + 1, '/');
	if (value == NULL) {
		return;
	}

	char *key = new char[value - *str];
	Common::strlcpy(key, *str + 1, (size_t)(value - *str));

	BasePlatform::strlwr(key);

	value++;

	char *newStr;

	StringsIter it = _strings.find(key);
	if (it != _strings.end()) {
		newStr = new char[it->_value.size() + 1];
		strcpy(newStr, it->_value.c_str());
	} else {
		newStr = new char[strlen(value) + 1];
		strcpy(newStr, value);
	}

	delete[] key;
	delete[] *str;
	*str = newStr;

	if (strlen(*str) > 0 && *str[0] == '/') {
		expand(str);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *BaseStringTable::expandStatic(const char *string) const {
	if (string == NULL || string[0] == '\0' || string[0] != '/') {
		return string;
	}

	const char *value = strchr(string + 1, '/');
	if (value == NULL) {
		return string;
	}

	char *key = new char[value - string];
	Common::strlcpy(key, string + 1, (size_t)(value - string - 1));
	BasePlatform::strlwr(key);

	value++;

	const char *newStr;

	StringsIter it = _strings.find(key);
	if (it != _strings.end()) {
		newStr = it->_value.c_str();
	} else {
		newStr = value;
	}

	delete[] key;

	if (strlen(newStr) > 0 && newStr[0] == '/') {
		return expandStatic(newStr);
	} else {
		return newStr;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseStringTable::loadFile(const char *filename, bool clearOld) {
	_gameRef->LOG(0, "Loading string table...");

	if (clearOld) {
		_strings.clear();
	}

	uint32 size;
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename, &size);
	if (buffer == NULL) {
		_gameRef->LOG(0, "BaseStringTable::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	uint32 pos = 0;

	if (size > 3 && buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) {
		pos += 3;
		if (_gameRef->_textEncoding != TEXT_UTF8) {
			_gameRef->_textEncoding = TEXT_UTF8;
			//_gameRef->_textEncoding = TEXT_ANSI;
			_gameRef->LOG(0, "  UTF8 file detected, switching to UTF8 text encoding");
		}
	} else {
		_gameRef->_textEncoding = TEXT_ANSI;
	}

	uint32 lineLength = 0;
	while (pos < size) {
		lineLength = 0;
		while (pos + lineLength < size && buffer[pos + lineLength] != '\n' && buffer[pos + lineLength] != '\0') {
			lineLength++;
		}

		uint32 realLength = lineLength - (pos + lineLength >= size ? 0 : 1);
		char *line = new char[realLength + 1];
		Common::strlcpy(line, (char *)&buffer[pos], realLength + 1);
		char *value = strchr(line, '\t');
		if (value == NULL) {
			value = strchr(line, ' ');
		}

		if (line[0] != ';') {
			if (value != NULL) {
				value[0] = '\0';
				value++;
				for (uint32 i = 0; i < strlen(value); i++) {
					if (value[i] == '|') {
						value[i] = '\n';
					}
				}
				addString(line, value, clearOld);
			} else if (line[0] != '\0') {
				addString(line, "", clearOld);
			}
		}

		delete[] line;
		pos += lineLength + 1;
	}

	delete[] buffer;

	_gameRef->LOG(0, "  %d strings loaded", _strings.size());

	return STATUS_OK;
}

} // end of namespace Wintermute
