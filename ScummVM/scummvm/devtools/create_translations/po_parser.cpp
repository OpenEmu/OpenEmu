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
 * This is a utility for create the translations.dat file from all the po files.
 * The generated files is used by ScummVM to propose translation of its GUI.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "po_parser.h"

PoMessageList::PoMessageList() : _messages(NULL), _size(0), _allocated(0) {
}

PoMessageList::~PoMessageList() {
	for (int i = 0; i < _size; ++i)
		delete[] _messages[i];
	delete[] _messages;
}

void PoMessageList::insert(const char *msg) {
	if (msg == NULL || *msg == '\0')
		return;

	// binary-search for the insertion index
	int leftIndex = 0;
	int rightIndex = _size - 1;
	while (rightIndex >= leftIndex) {
		int midIndex = (leftIndex + rightIndex) / 2;
		int compareResult = strcmp(msg, _messages[midIndex]);
		if (compareResult == 0)
			return;
		else if (compareResult < 0)
			rightIndex = midIndex - 1;
		else
			leftIndex = midIndex + 1;
	}
	// We now have rightIndex = leftIndex - 1 and we need to insert the new message
	// between the two (i.a. at leftIndex).
	if (_size + 1 > _allocated) {
		_allocated += 100;
		char **newMessages = new char*[_allocated];
		for (int i = 0; i < leftIndex; ++i)
			newMessages[i] = _messages[i];
		for (int i = leftIndex; i < _size; ++i)
			newMessages[i + 1] = _messages[i];
		delete[] _messages;
		_messages = newMessages;
	} else {
		for (int i = _size - 1; i >= leftIndex; --i)
			_messages[i + 1] = _messages[i];
	}
	_messages[leftIndex] = new char[1 + strlen(msg)];
	strcpy(_messages[leftIndex], msg);
	++_size;
}

int PoMessageList::findIndex(const char *msg) {
	if (msg == NULL || *msg == '\0')
		return -1;

	// binary-search for the message
	int leftIndex = 0;
	int rightIndex = _size - 1;

	while (rightIndex >= leftIndex) {
		const int midIndex = (leftIndex + rightIndex) / 2;
		const int compareResult = strcmp(msg, _messages[midIndex]);
		if (compareResult == 0)
			return midIndex;
		else if (compareResult < 0)
			rightIndex = midIndex - 1;
		else
			leftIndex = midIndex + 1;
	}

	return -1;
}

int PoMessageList::size() const {
	return _size;
}

const char *PoMessageList::operator[](int index) const {
	if (index < 0 || index >= _size)
		return NULL;
	return _messages[index];
}

PoMessageEntryList::PoMessageEntryList(const char *lang) :
	_lang(NULL), _charset(NULL), _langName(NULL),
	_list(NULL), _size(0), _allocated(0)
{
	_lang = new char[1 + strlen(lang)];
	strcpy(_lang, lang);
	// Set default charset to empty string
	_charset = new char[1];
	_charset[0] = '\0';
	// Set default langName to lang
	_langName = new char[1 + strlen(lang)];
	strcpy(_langName, lang);
}

PoMessageEntryList::~PoMessageEntryList() {
	delete[] _lang;
	delete[] _charset;
	delete[] _langName;
	for (int i = 0; i < _size; ++i)
		delete _list[i];
	delete[] _list;
}

void PoMessageEntryList::addMessageEntry(const char *translation, const char *message, const char *context) {
	if (*message == '\0') {
		// This is the header.
		// We get the charset and the language name from the translation string
		char *str = parseLine(translation, "Language:");
		if (str != NULL) {
			delete[] _langName;
			_langName = str;
		}
		str = parseLine(translation, "charset=");
		if (str != NULL) {
			delete[] _charset;
			_charset = str;
		}
		return;
	}

	// binary-search for the insertion index
	int leftIndex = 0;
	int rightIndex = _size - 1;
	while (rightIndex >= leftIndex) {
		int midIndex = (leftIndex + rightIndex) / 2;
		int compareResult = strcmp(message, _list[midIndex]->msgid);
		if (compareResult == 0) {
			if (context == NULL) {
				if (_list[midIndex]->msgctxt == NULL)
					return;
				compareResult = -1;
			} else {
				if (_list[midIndex]->msgctxt == NULL)
					compareResult = 1;
				else {
					compareResult = strcmp(context, _list[midIndex]->msgctxt);
					if (compareResult == 0)
						return;
				}
			}
		}
		if (compareResult < 0)
			rightIndex = midIndex - 1;
		else
			leftIndex = midIndex + 1;
	}
	// We now have rightIndex = leftIndex - 1 and we need to insert the new message
	// between the two (i.a. at leftIndex).
	// However since the TranslationManager will pick the translation associated to no
	// context if it is not present for a specific context, we can optimize the file
	// size, memory used at run-time and performances (less strings to read from the file
	// and less strings to look for) by avoiding duplicate.
	if (context != NULL && *context != '\0') {
		// Check if we have the same translation for no context
		int contextIndex = leftIndex - 1;
		while (contextIndex >= 0 && strcmp (message, _list[contextIndex]->msgid) == 0) {
			--contextIndex;
		}
		++contextIndex;
		if (contextIndex < leftIndex && _list[contextIndex]->msgctxt == NULL && strcmp(translation, _list[contextIndex]->msgstr) == 0)
			return;
	}


	if (_size + 1 > _allocated) {
		_allocated += 100;
		PoMessageEntry **newList = new PoMessageEntry*[_allocated];
		for (int i = 0; i < leftIndex; ++i)
			newList[i] = _list[i];
		for (int i = leftIndex; i < _size; ++i)
			newList[i + 1] = _list[i];
		delete[] _list;
		_list = newList;
	} else {
		for (int i = _size - 1; i >= leftIndex; --i)
			_list[i + 1] = _list[i];
	}
	_list[leftIndex] = new PoMessageEntry(translation, message, context);
	++_size;

	if (context == NULL || *context == '\0') {
		// Remove identical translations for a specific context (see comment above)
		int contextIndex = leftIndex + 1;
		int removed = 0;
		while (contextIndex < _size && strcmp(message, _list[contextIndex]->msgid) == 0) {
			if (strcmp(translation, _list[contextIndex]->msgstr) == 0) {
				delete _list[contextIndex];
				++removed;
			} else {
				_list[contextIndex - removed] = _list[contextIndex];
			}
			++contextIndex;
		}
		if (removed > 0) {
			while (contextIndex < _size) {
				_list[contextIndex - removed] = _list[contextIndex];
				++contextIndex;
			}
		}
		_size -= removed;
	}

}

const char *PoMessageEntryList::language() const {
	return _lang;
}

const char *PoMessageEntryList::languageName() const {
	return _langName;
}

const char *PoMessageEntryList::charset() const {
	return _charset;
}

int PoMessageEntryList::size() const {
	return _size;
}

const PoMessageEntry *PoMessageEntryList::entry(int index) const {
	if (index < 0 || index >= _size)
		return NULL;
	return _list[index];
}


PoMessageEntryList *parsePoFile(const char *file, PoMessageList& messages) {
	FILE *inFile = fopen(file, "r");
	if (!inFile)
		return NULL;

	char msgidBuf[1024], msgctxtBuf[1024], msgstrBuf[1024];
	char line[1024], *currentBuf = msgstrBuf;

	// Get language from file name and create PoMessageEntryList
	int index = 0, start_index = strlen(file) - 1;
	while (start_index > 0 && file[start_index - 1] != '/' && file[start_index - 1] != '\\') {
		--start_index;
	}
	while (file[start_index + index] != '.' && file[start_index + index] != '\0') {
		msgidBuf[index] = file[start_index + index];
		++index;
	}
	msgidBuf[index] = '\0';
	PoMessageEntryList *list = new PoMessageEntryList(msgidBuf);

	// Initialize the message attributes.
	bool fuzzy = false;
	bool fuzzy_next = false;

	// Parse the file line by line.
	// The msgstr is always the last line of an entry (i.e. msgid and msgctxt always
	// precede the corresponding msgstr).
	msgidBuf[0] = msgstrBuf[0] = msgctxtBuf[0] = '\0';
	while (!feof(inFile) && fgets(line, 1024, inFile)) {
		if (line[0] == '#' && line[1] == ',') {
			// Handle message attributes.
			if (strstr(line, "fuzzy")) {
				fuzzy_next = true;
				continue;
			}
		}
		// Skip empty and comment line
		if (*line == '\n' || *line == '#')
			continue;
		if (strncmp(line, "msgid", 5) == 0) {
			if (currentBuf == msgstrBuf) {
				// add previous entry
				if (*msgstrBuf != '\0' && !fuzzy) {
					messages.insert(msgidBuf);
					list->addMessageEntry(msgstrBuf, msgidBuf, msgctxtBuf);
				}
				msgidBuf[0] = msgstrBuf[0] = msgctxtBuf[0] = '\0';

				// Reset the attribute flags.
				fuzzy = fuzzy_next;
				fuzzy_next = false;
			}
			strcpy(msgidBuf, stripLine(line));
			currentBuf = msgidBuf;
		} else if (strncmp(line, "msgctxt", 7) == 0) {
			if (currentBuf == msgstrBuf) {
				// add previous entry
				if (*msgstrBuf != '\0' && !fuzzy) {
					messages.insert(msgidBuf);
					list->addMessageEntry(msgstrBuf, msgidBuf, msgctxtBuf);
				}
				msgidBuf[0] = msgstrBuf[0] = msgctxtBuf[0] = '\0';

				// Reset the attribute flags
				fuzzy = fuzzy_next;
				fuzzy_next = false;
			}
			strcpy(msgctxtBuf, stripLine(line));
			currentBuf = msgctxtBuf;
		} else if (strncmp(line, "msgstr", 6) == 0) {
			strcpy(msgstrBuf, stripLine(line));
			currentBuf = msgstrBuf;
		} else {
			// concatenate the string at the end of the current buffer
			if (currentBuf)
				strcat(currentBuf, stripLine(line));
		}
	}

	fclose(inFile);
	return list;
}

char *stripLine(char *const line) {
	// This function modifies line in place and return it.
	// Keep only the text between the first two unprotected quotes.
	// It also look for literal special characters (e.g. preceded by '\n', '\\', '\"', '\'', '\t')
	// and replace them by the special character so that strcmp() can match them at run time.
	// Look for the first quote
	char const *src = line;
	while (*src != '\0' && *src++ != '"') {}
	// shift characters until we reach the end of the string or an unprotected quote
	char *dst = line;
	while (*src != '\0' && *src != '"') {
		char c = *src++;
		if (c == '\\') {
			switch (c = *src++) {
			case  'n': c = '\n'; break;
			case  't': c = '\t'; break;
			case '\"': c = '\"'; break;
			case '\'': c = '\''; break;
			case '\\': c = '\\'; break;
			default:
				// Just skip
				fprintf(stderr, "Unsupported special character \"\\%c\" in string. Please contact ScummVM developers.\n", c);
				continue;
			}
		}
		*dst++ = c;
	}
	*dst = '\0';
	return line;
}

char *parseLine(const char *line, const char *field) {
	// This function allocate and return a new char*.
	// It will return a NULL pointer if the field is not found.
	// It is used to parse the header of the po files to find the language name
	// and the charset.
	const char *str = strstr(line, field);
	if (str == NULL)
		return NULL;
	str += strlen(field);
	// Skip spaces
	while (*str != '\0' && isspace(*str)) {
		++str;
	}
	// Find string length (stop at the first '\n')
	int len = 0;
	while (str[len] != '\0' && str[len] != '\n') {
		++len;
	}
	if (len == 0)
		return NULL;
	// Create result string
	char *result = new char[len + 1];
	strncpy(result, str, len);
	result[len] = '\0';
	return result;
}
