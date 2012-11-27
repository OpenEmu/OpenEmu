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

#ifndef WINTERMUTE_SXDATE_H
#define WINTERMUTE_SXDATE_H

#include "common/system.h"
#include "engines/wintermute/base/base_scriptable.h"

namespace Wintermute {

class SXDate : public BaseScriptable {
public:
	int scCompare(BaseScriptable *Value);
	DECLARE_PERSISTENT(SXDate, BaseScriptable)
	SXDate(BaseGame *inGame, ScStack *Stack);
	virtual ~SXDate();
	ScValue *scGetProperty(const Common::String &name);
	bool scSetProperty(const char *name, ScValue *value);
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	const char *scToString();
private:
	TimeDate _tm;
	Common::String _strRep;
};

} // end of namespace Wintermute

#endif
