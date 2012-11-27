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

#ifndef WINTERMUTE_BASE_PARSER_H
#define WINTERMUTE_BASE_PARSER_H


#define TOKEN_DEF_START         \
	enum                  \
	{                 \
		TOKEN_NONE = 0,
#define TOKEN_DEF(name)         \
	TOKEN_ ## name,
#define TOKEN_DEF_END           \
	TOKEN_TOTAL_COUNT           \
	};
#define TOKEN_TABLE_START(name)     \
	static const BaseParser::TokenDesc name [] =      \
	        {
#define TOKEN_TABLE(name)       \
	{ TOKEN_ ## name, #name },
#define TOKEN_TABLE_END         \
	{ 0, 0 }                \
	};

#define PARSERR_GENERIC         -3
#define PARSERR_EOF             -2
#define PARSERR_TOKENNOTFOUND   -1

#include "engines/wintermute/coll_templ.h"

namespace Wintermute {

class BaseParser {
public:
	struct TokenDesc {
		int32   id;
		const char *token;
	};

public:
	int scanStr(const char *in, const char *format, ...);
	int32 getCommand(char **buf, const TokenDesc *tokens, char **params);
	BaseParser();
	virtual ~BaseParser();
private:
	char *getLastOffender();
	void skipToken(char **buf, char *tok, char *msg = NULL);
	int getTokenInt(char **buf);
	float getTokenFloat(char **buf);
	Common::String getToken(char **buf);
	char *getAssignmentText(char **buf);
	char *getSubText(char **buf, char open, char close);
	void skipCharacters(char **buf, const char *toSkip);
	int32 getObject(char **buf, const TokenDesc *tokens, char **name, char **data);
	int _parserLine;
	char _lastOffender[255];
	char *_whiteSpace;
};

} // end of namespace Wintermute

#endif
