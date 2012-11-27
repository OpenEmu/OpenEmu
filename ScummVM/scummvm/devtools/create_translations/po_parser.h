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

#ifndef PO_PARSER_H
#define PO_PARSER_H


/**
 * List of english messages.
 */
class PoMessageList {
public:
	PoMessageList();
	~PoMessageList();

	void insert(const char *msg);
	int findIndex(const char *msg);

	int size() const;
	const char *operator[](int) const;

private:
	char **_messages;
	int _size;
	int _allocated;
};

/**
 * Describes a translation entry.
 */
struct PoMessageEntry {
	char *msgstr;
	char *msgid;
	char *msgctxt;

	PoMessageEntry(const char *translation, const char *message, const char *context = NULL) :
		msgstr(NULL), msgid(NULL), msgctxt(NULL)
	{
		if (translation != NULL && *translation != '\0') {
			msgstr = new char[1 + strlen(translation)];
			strcpy(msgstr, translation);
		}
		if (message != NULL && *message != '\0') {
			msgid = new char[1 + strlen(message)];
			strcpy(msgid, message);
		}
		if (context != NULL && *context != '\0') {
			msgctxt = new char[1 + strlen(context)];
			strcpy(msgctxt, context);
		}
	}
	~PoMessageEntry() {
		delete[] msgstr;
		delete[] msgid;
		delete[] msgctxt;
	}
};

/**
 * List of translation entries for one language.
 */
class PoMessageEntryList {
public:
	PoMessageEntryList(const char *language);
	~PoMessageEntryList();

	void addMessageEntry(const char *translation, const char *message, const char *context = NULL);

	const char *language() const;
	const char *languageName() const;
	const char *charset() const;

	int size() const;
	const PoMessageEntry *entry(int) const;

private:
	char *_lang;
	char *_charset;
	char *_langName;

	PoMessageEntry **_list;
	int _size;
	int _allocated;
};


PoMessageEntryList *parsePoFile(const char *file, PoMessageList &);
char *stripLine(char *);
char *parseLine(const char *line, const char *field);

#endif /* PO_PARSER_H */
