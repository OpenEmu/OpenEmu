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
#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/text.h"

namespace Hugo {

TextHandler::TextHandler(HugoEngine *vm) : _vm(vm), _textData(0), _stringtData(0),
	_textEngine(0), _textIntro(0), _textMouse(0), _textParser(0), _textUtil(0),
	_screenNames(0), _arrayNouns(0), _arrayVerbs(0) {
}

TextHandler::~TextHandler() {
}

const char *TextHandler::getNoun(int idx1, int idx2) const {
	return _arrayNouns[idx1][idx2];
}

const char *TextHandler::getScreenNames(int screenIndex) const {
	return _screenNames[screenIndex];
}

const char *TextHandler::getStringtData(int stringIndex) const {
	return _stringtData[stringIndex];
}

const char *TextHandler::getTextData(int textIndex) const {
	return _textData[textIndex];
}

const char *TextHandler::getTextEngine(int engineIndex) const {
	return _textEngine[engineIndex];
}

const char *TextHandler::getTextIntro(int introIndex) const {
	return _textIntro[introIndex];
}

const char *TextHandler::getTextMouse(int mouseIndex) const {
	return _textMouse[mouseIndex];
}

const char *TextHandler::getTextParser(int parserIndex) const {
	return _textParser[parserIndex];
}

const char *TextHandler::getTextUtil(int utilIndex) const {
	return _textUtil[utilIndex];
}

const char *TextHandler::getVerb(int idx1, int idx2) const {
	return _arrayVerbs[idx1][idx2];
}

char **TextHandler::getNounArray(int idx1) const {
	return _arrayNouns[idx1];
}

char **TextHandler::getVerbArray(int idx1) const {
	return _arrayVerbs[idx1];
}

char **TextHandler::loadTextsVariante(Common::ReadStream &in, uint16 *arraySize) {
	int  numTexts;
	int  entryLen;
	int  len;
	char **res = 0;
	char *pos = 0;
	char *posBck = 0;

	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		numTexts = in.readUint16BE();
		entryLen = in.readUint16BE();
		pos = (char *)malloc(entryLen);
		if (varnt == _vm->_gameVariant) {
			if (arraySize)
				*arraySize = numTexts;
			res = (char **)malloc(sizeof(char *) * numTexts);
			res[0] = pos;
			in.read(res[0], entryLen);
			res[0] += DATAALIGNMENT;
		} else {
			in.read(pos, entryLen);
			posBck = pos;
		}

		pos += DATAALIGNMENT;

		for (int i = 1; i < numTexts; i++) {
			pos -= 2;

			len = READ_BE_UINT16(pos);
			pos += 2 + len;

			if (varnt == _vm->_gameVariant)
				res[i] = pos;
		}

		if (varnt != _vm->_gameVariant)
			free(posBck);
	}

	return res;
}

char ***TextHandler::loadTextsArray(Common::ReadStream &in) {
	char ***resArray = 0;
	uint16 arraySize;

	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		arraySize = in.readUint16BE();
		if (varnt == _vm->_gameVariant) {
			resArray = (char ***)malloc(sizeof(char **) * (arraySize + 1));
			resArray[arraySize] = 0;
		}
		for (int i = 0; i < arraySize; i++) {
			int numTexts = in.readUint16BE();
			int entryLen = in.readUint16BE();
			char *pos = (char *)malloc(entryLen);
			char *posBck = 0;
			char **res = 0;
			if (varnt == _vm->_gameVariant) {
				res = (char **)malloc(sizeof(char *) * numTexts);
				res[0] = pos;
				in.read(res[0], entryLen);
				res[0] += DATAALIGNMENT;
			} else {
				in.read(pos, entryLen);
				posBck = pos;
			}

			pos += DATAALIGNMENT;

			for (int j = 0; j < numTexts; j++) {
				if (varnt == _vm->_gameVariant)
					res[j] = pos;

				pos -= 2;
				int len = READ_BE_UINT16(pos);
				pos += 2 + len;
			}

			if (varnt == _vm->_gameVariant)
				resArray[i] = res;
			else
				free(posBck);
		}
	}

	return resArray;
}

char **TextHandler::loadTexts(Common::ReadStream &in) {
	int numTexts = in.readUint16BE();
	char **res = (char **)malloc(sizeof(char *) * numTexts);
	int entryLen = in.readUint16BE();
	char *pos = (char *)malloc(entryLen);

	in.read(pos, entryLen);

	pos += DATAALIGNMENT;
	res[0] = pos;

	for (int i = 1; i < numTexts; i++) {
		pos -= 2;
		int len = READ_BE_UINT16(pos);
		pos += 2 + len;
		res[i] = pos;
	}

	return res;
}

void TextHandler::loadAllTexts(Common::ReadStream &in) {
	// Read textData
	_textData = loadTextsVariante(in, 0);

	// Read stringtData
	// Only Hugo 1 DOS should use this array
	_stringtData = loadTextsVariante(in, 0);

	// Read arrayNouns
	_arrayNouns = loadTextsArray(in);

	// Read arrayVerbs
	_arrayVerbs = loadTextsArray(in);

	// Read screenNames
	_screenNames = loadTextsVariante(in, &_vm->_numScreens);

	// Read textEngine
	_textEngine = loadTexts(in);

	// Read textIntro
	_textIntro = loadTextsVariante(in, 0);

	// Read textMouse
	_textMouse = loadTexts(in);

	// Read textParser
	_textParser = loadTexts(in);

	// Read textUtil
	_textUtil = loadTextsVariante(in, 0);
}

void TextHandler::freeTexts(char **ptr) {
	if (!ptr)
		return;

	free(*ptr - DATAALIGNMENT);
	free(ptr);
}

void TextHandler::freeAllTexts() {
	freeTexts(_textData);
	freeTexts(_stringtData);

	if (_arrayNouns) {
		for (int i = 0; _arrayNouns[i]; i++)
			freeTexts(_arrayNouns[i]);
		free(_arrayNouns);
	}

	if (_arrayVerbs) {
		for (int i = 0; _arrayVerbs[i]; i++)
			freeTexts(_arrayVerbs[i]);
		free(_arrayVerbs);
	}

	freeTexts(_screenNames);
	freeTexts(_textEngine);
	freeTexts(_textIntro);
	freeTexts(_textMouse);
	freeTexts(_textParser);
	freeTexts(_textUtil);
}

} // End of namespace Hugo
