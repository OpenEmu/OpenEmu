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
#ifndef TEXT_H
#define TEXT_H

namespace Hugo {

class TextHandler {
public:
	TextHandler(HugoEngine *vm);
	~TextHandler();

	const char *getNoun(int idx1, int idx2) const;
	const char *getScreenNames(int screenIndex) const;
	const char *getStringtData(int stringIndex) const;
	const char *getTextData(int textIndex) const;
	const char *getTextEngine(int engineIndex) const;
	const char *getTextIntro(int introIndex) const;
	const char *getTextMouse(int mouseIndex) const;
	const char *getTextParser(int parserIndex) const;
	const char *getTextUtil(int utilIndex) const;
	const char *getVerb(int idx1, int idx2) const;
	char **getNounArray(int idx1) const;
	char **getVerbArray(int idx1) const;

	void loadAllTexts(Common::ReadStream &in);
	void freeAllTexts();

private:
	HugoEngine *_vm;

	char ***_arrayNouns;
	char ***_arrayVerbs;

	char  **_screenNames;
	char  **_stringtData;
	char  **_textData;
	char  **_textEngine;
	char  **_textIntro;
	char  **_textMouse;
	char  **_textParser;
	char  **_textUtil;

	char ***loadTextsArray(Common::ReadStream &in);
	char  **loadTextsVariante(Common::ReadStream &in, uint16 *arraySize);
	char  **loadTexts(Common::ReadStream &in);

	void    freeTexts(char **ptr);

};

} // End of namespace Hugo
#endif // TEXT_H
