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

#include "common/textconsole.h"

#include "parallaction/parallaction.h"
#include "parallaction/parser.h"


namespace Parallaction {

#define MAX_TOKENS	50

int				_numTokens;
char			_tokens[MAX_TOKENS][MAX_TOKEN_LEN];

Script::Script(Common::ReadStream *input, bool disposeSource) : _input(input), _disposeSource(disposeSource), _line(0) {}

Script::~Script() {
	if (_disposeSource)
		delete _input;
}

/*
 * readLineIntern read a text line and prepares it for
 * parsing, by stripping the leading whitespace and
 * changing tabs to spaces. It will stop on a CR, LF, or
 * SUB (0x1A), which may all occur at the end of a script
 * line.
 * Returns an empty string (length = 0) when a line
 * has no printable text in it.
 */
char *Script::readLineIntern(char *buf, size_t bufSize) {
	uint i = 0;
	for ( ; i < bufSize; ) {
		char c = _input->readSByte();
		if (_input->eos())
			break;
		// break if EOL
		if (c == '\n' || c == '\r' || c == (char)0x1A)
			break;
		if (c == '\t')
			c = ' ';

		if ((c != ' ') || (i > 0)) {
			buf[i] = c;
			i++;
		}
	}
	_line++;
	if (i == bufSize) {
		warning("overflow in readLineIntern (line %i)", _line);
	}
	if (i == 0 && _input->eos()) {
		return 0;
	}
	buf[i] = '\0';
	return buf;
}

bool isCommentLine(char *text) {
	return text[0] == '#';
}

bool isStartOfCommentBlock(char *text) {
	return (text[0] == '[');
}

bool isEndOfCommentBlock(char *text) {
	return (text[0] == ']');
}

char *Script::readLine(char *buf, size_t bufSize) {
	bool inBlockComment = false;
	bool ignoreLine = true;

	char *line = 0;
	do {
		line = readLineIntern(buf, bufSize);
		if (line == 0) {
			return 0;
		}

		if (line[0] == '\0')
			continue;

		ignoreLine = false;

		line = Common::ltrim(line);
		if (isCommentLine(line)) {
			// ignore this line
			ignoreLine = true;
		} else
		if (isStartOfCommentBlock(line)) {
			// mark this and the following lines as comment
			inBlockComment = true;
		} else
		if (isEndOfCommentBlock(line)) {
			// comment is finished, so stop ignoring
			inBlockComment = false;
			// the current line must be skipped, though,
			// as it contains the end-of-comment marker
			ignoreLine = true;
		}

	} while (inBlockComment || ignoreLine);

	return line;
}



void Script::clearTokens() {
	memset(_tokens, 0, sizeof(_tokens));
	_numTokens = 0;
}

void Script::skip(const char* endToken) {

	while (scumm_stricmp(_tokens[0], endToken)) {
		readLineToken(true);
	}

}

//
//	Scans 's' until one of the stop-chars in 'brk' is found, building a token.
//	If the routine encounters quotes, it will extract the contained text and
//  make a proper token. When scanning inside quotes, 'brk' is ignored and
//  only newlines are considered stop-chars.
//
//	The routine returns the unparsed portion of the input string 's'.
//
char *Script::parseNextToken(char *s, char *tok, uint16 count, const char *brk) {

	enum STATES { NORMAL, QUOTED };

	STATES state = NORMAL;

	while (count > 0) {

		switch (state) {
		case NORMAL:
			if (*s == '\0') {
				*tok = '\0';
				return s;
			} else
			if (strchr(brk, *s)) {
				*tok = '\0';
				return ++s;
			} else
			if (*s == '"') {
				state = QUOTED;
				s++;
			} else {
				*tok++ = *s++;
				count--;
			}
			break;

		case QUOTED:
			if (*s == '\0') {
				*tok = '\0';
				return s;
			} else
			if (*s == '"') {
				*tok = '\0';
				return ++s;
			} else {
				*tok++ = *s++;
				count--;
			}
			break;
		}

	}

	*tok = '\0';
	// TODO: if execution flows here, make *REALLY* sure everything has been parsed
	// out of the input string. This is what is supposed to happen, but never ever
	// allocated time to properly check.

	return tok;

}

uint16 Script::readLineToken(bool errorOnEOF) {
	char buf[200];
	char *line = readLine(buf, 200);
	if (!line) {
		if (errorOnEOF)
			error("unexpected end of file while parsing");
		else
			return 0;
	}

	clearTokens();
	while (*line && _numTokens < MAX_TOKENS) {
		line = parseNextToken(line, _tokens[_numTokens], MAX_TOKEN_LEN, " ");
		line = Common::ltrim(line);
		_numTokens++;
	}
	return _numTokens;
}


void Parser::reset() {
	_currentOpcodes = 0;
	_currentStatements = 0;

	_statements.clear();
	_opcodes.clear();
}

void Parser::pushTables(OpcodeSet *opcodes, Table *statements) {
	_opcodes.push(_currentOpcodes);
	_statements.push(_currentStatements);

	_currentOpcodes = opcodes;
	_currentStatements = statements;
}

void Parser::popTables() {
	assert(_opcodes.size() > 0);

	_currentOpcodes = _opcodes.pop();
	_currentStatements = _statements.pop();
}

void Parser::parseStatement() {
	assert(_currentOpcodes != 0);

	_lookup = _currentStatements->lookup(_tokens[0]);

	debugC(9, kDebugParser, "parseStatement: %s (lookup = %i)", _tokens[0], _lookup);

	(*(*_currentOpcodes)[_lookup])();
}

} // End of namespace Parallaction
