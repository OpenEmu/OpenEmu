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
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_LUACALLBACK_H
#define SWORD25_LUACALLBACK_H

#include "sword25/kernel/common.h"

struct lua_State;

namespace Sword25 {

class LuaCallback {
public:
	LuaCallback(lua_State *L);
	virtual ~LuaCallback();

	// Funktion muss auf dem Lua-Stack liegen.
	void registerCallbackFunction(lua_State *L, uint objectHandle);

	// Funktion muss auf dem Lua-Stack liegen.
	void unregisterCallbackFunction(lua_State *L, uint objectHandle);

	void removeAllObjectCallbacks(lua_State *L, uint objectHandle);

	void invokeCallbackFunctions(lua_State *L, uint objectHandle);

protected:
	virtual int preFunctionInvokation(lua_State *L) {
		return 0;
	}

private:
	void ensureObjectCallbackTableExists(lua_State *L, uint objectHandle);
	void pushCallbackTable(lua_State *L);
	void pushObjectCallbackTable(lua_State *L, uint objectHandle);
};

} // End of namespace Sword25

#endif
