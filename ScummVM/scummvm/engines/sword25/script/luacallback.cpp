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

#include "common/textconsole.h"

#include "sword25/script/luacallback.h"
#include "sword25/script/luabindhelper.h"

#include "sword25/util/lua/lua.h"
#include "sword25/util/lua/lauxlib.h"

const char *CALLBACKTABLE_NAME = "__CALLBACKS";

namespace Sword25 {

LuaCallback::LuaCallback(lua_State *L) {
	// Create callback table
	lua_newtable(L);
	lua_setglobal(L, CALLBACKTABLE_NAME);
}

LuaCallback::~LuaCallback() {
}

void LuaCallback::registerCallbackFunction(lua_State *L, uint objectHandle) {
	assert(lua_isfunction(L, -1));
	ensureObjectCallbackTableExists(L, objectHandle);

	// Store function in the callback object table store
	lua_pushvalue(L, -2);
	luaL_ref(L, -2);

	// Pop the function and object callback table from the stack
	lua_pop(L, 2);
}

void LuaCallback::unregisterCallbackFunction(lua_State *L, uint objectHandle) {
	assert(lua_isfunction(L, -1));
	ensureObjectCallbackTableExists(L, objectHandle);

	// Iterate over all elements of the object callback table and remove the function from it
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		// The value of the current element is the top of the stack, including the index

		// If the value is identical to the function parameters, it is removed from the table
		if (lua_equal(L, -1, -4)) {
			lua_pushvalue(L, -2);
			lua_pushnil(L);
			lua_settable(L, -5);

			// The function was found, iteration can be stopped
			lua_pop(L, 2);
			break;
		} else {
			// Pop value from the stack. The index is then ready for the next call to lua_next()
			lua_pop(L, 1);
		}
	}

	// Function and object table are popped from the stack
	lua_pop(L, 2);
}

void LuaCallback::removeAllObjectCallbacks(lua_State *L, uint objectHandle) {
	pushCallbackTable(L);

	// Remove the object callback from the callback table
	lua_pushnumber(L, objectHandle);
	lua_pushnil(L);
	lua_settable(L, -3);

	lua_pop(L, 1);
}

void LuaCallback::invokeCallbackFunctions(lua_State *L, uint objectHandle) {
	ensureObjectCallbackTableExists(L, objectHandle);

	// Iterate through the table and perform all the callbacks
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		// The value of the current element is at the top of the stack, including the index

		// If the value is a function, execute it
		if (lua_type(L, -1) == LUA_TFUNCTION) {
			// Pre-Function Call
			// Derived classes can function in this parameter onto the stack.
			// The return value indicates the number of parameters
			int argumentCount = preFunctionInvokation(L);

			// Lua_pcall the function and the parameters pop themselves from the stack
			if (lua_pcall(L, argumentCount, 0, 0) != 0) {
				// An error has occurred
				error("An error occured executing a callback function: %s", lua_tostring(L, -1));

				// Pop error message from the stack
				lua_pop(L, 1);
			}
		} else {
			// Pop value from the stack. The index is then ready for the next call to lua_next()
			lua_pop(L, 1);
		}
	}
}

void LuaCallback::ensureObjectCallbackTableExists(lua_State *L, uint objectHandle) {
	pushObjectCallbackTable(L, objectHandle);

	// If the table is nil, it must first be created
	if (lua_isnil(L, -1)) {
		// Pop nil from stack
		lua_pop(L, 1);

		pushCallbackTable(L);

		// Create the table, and put the objectHandle into it
		lua_newtable(L);
		lua_pushnumber(L, objectHandle);
		lua_pushvalue(L, -2);
		lua_settable(L, -4);

		// Pop the callback table from the stack
		lua_remove(L, -2);
	}
}

void LuaCallback::pushCallbackTable(lua_State *L) {
	lua_getglobal(L, CALLBACKTABLE_NAME);
}

void LuaCallback::pushObjectCallbackTable(lua_State *L, uint objectHandle) {
	pushCallbackTable(L);

	// Push Object Callback table onto the stack
	lua_pushnumber(L, objectHandle);
	lua_gettable(L, -2);

	// Pop the callback table from the stack
	lua_remove(L, -2);
}

} // End of namespace Sword25
