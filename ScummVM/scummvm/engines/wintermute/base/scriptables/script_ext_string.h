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

#ifndef WINTERMUTE_SXSTRING_H
#define WINTERMUTE_SXSTRING_H


#include "engines/wintermute/base/base_scriptable.h"

namespace Wintermute {

class SXString : public BaseScriptable {
public:
	virtual int scCompare(BaseScriptable *Val);
	DECLARE_PERSISTENT(SXString, BaseScriptable)
	ScValue *scGetProperty(const Common::String &name);
	bool scSetProperty(const char *name, ScValue *value);
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	void scSetString(const char *val);
	const char *scToString();
	void setStringVal(const char *val);

	SXString(BaseGame *inGame, ScStack *Stack);
	virtual ~SXString();

private:
	char *_string;
	int _capacity;
};

} // end of namespace Wintermute

#endif
