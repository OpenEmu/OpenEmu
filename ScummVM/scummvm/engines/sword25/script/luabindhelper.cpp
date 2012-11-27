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

#include "sword25/kernel/kernel.h"
#include "sword25/script/luabindhelper.h"
#include "sword25/script/luascript.h"

namespace {
const char *METATABLES_TABLE_NAME = "__METATABLES";
const char *PERMANENTS_TABLE_NAME = "Permanents";

bool registerPermanent(lua_State *L, const Common::String &name) {
	// A C function has to be on the stack
	if (!lua_iscfunction(L, -1))
		return false;

	// Make sure that the Permanents-Table is on top of the stack
	lua_getfield(L, LUA_REGISTRYINDEX, PERMANENTS_TABLE_NAME);
	if (lua_isnil(L, -1)) {
		// Permanents-Table does not yet exist, so it has to be created

		// Pop nil from the stack
		lua_pop(L, 1);

		// Create Permanents-Table and insert a second reference to it on the stack
		lua_newtable(L);
		lua_pushvalue(L, -1);

		// Store the Permanents-Table in the registry. The second reference is left
		// on the stack to be used in the connection
		lua_setfield(L, LUA_REGISTRYINDEX, PERMANENTS_TABLE_NAME);
	}

	// C function with the name of an index in the Permanents-Table
	lua_insert(L, -2);
	lua_setfield(L, -2, name.c_str());

	// Remove the Permanents-Table from the stack
	lua_pop(L, 1);

	return true;
}
}

namespace Sword25 {

/**
 * Registers a set of functions into a Lua library.
 * @param L             A pointer to the Lua VM
 * @param LibName       The name of the library.
 * If this is an empty string, the functions will be added to the global namespace.
 * @param Functions     An array of function pointers along with their names.
 * The array must be terminated with the enry (0, 0)
 * @return              Returns true if successful, otherwise false.
 */
bool LuaBindhelper::addFunctionsToLib(lua_State *L, const Common::String &libName, const luaL_reg *functions) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// If the table name is empty, the functions are to be added to the global namespace
	if (libName.size() == 0) {
		for (; functions->name; ++functions) {
			lua_pushstring(L, functions->name);
			lua_pushcclosure(L, functions->func, 0);
			lua_settable(L, LUA_GLOBALSINDEX);

			// Function is being permanently registed, so persistence can be ignored
			lua_pushstring(L, functions->name);
			lua_gettable(L, LUA_GLOBALSINDEX);
			registerPermanent(L, functions->name);
		}
	} else { // If the table name is not empty, the functions are added to the given table
		// Ensure that the library table exists
		if (!createTable(L, libName)) return false;

		// Register each function into the table
		for (; functions->name; ++functions) {
			// Function registration
			lua_pushstring(L, functions->name);
			lua_pushcclosure(L, functions->func, 0);
			lua_settable(L, -3);

			// Function is being permanently registed, so persistence can be ignored
			lua_pushstring(L, functions->name);
			lua_gettable(L, -2);
			registerPermanent(L, libName + "." + functions->name);
		}

		// Remove the library table from the Lua stack
		lua_pop(L, 1);
	}

#ifdef DEBUG
	assert(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

/**
 * Adds a set of constants to the Lua library
 * @param L             A pointer to the Lua VM
 * @param LibName       The name of the library.
 * If this is an empty string, the functions will be added to the global namespace.
 * @param Constants     An array of the constant values along with their names.
 * The array must be terminated with the enry (0, 0)
 * @return              Returns true if successful, otherwise false.
 */
bool LuaBindhelper::addConstantsToLib(lua_State *L, const Common::String &libName, const lua_constant_reg *constants) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// If the table is empty, the constants are added to the global namespace
	if (libName.size() == 0) {
		for (; constants->Name; ++constants) {
			lua_pushstring(L, constants->Name);
			lua_pushnumber(L, constants->Value);
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}
	// If the table name is nto empty, the constants are added to that table
	else {
		// Ensure that the library table exists
		if (!createTable(L, libName)) return false;

		// Register each constant in the table
		for (; constants->Name; ++constants) {
			lua_pushstring(L, constants->Name);
			lua_pushnumber(L, constants->Value);
			lua_settable(L, -3);
		}

		// Remove the library table from the Lua stack
		lua_pop(L, 1);
	}

#ifdef DEBUG
	assert(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

/**
 * Adds a set of methods to a Lua class
 * @param L             A pointer to the Lua VM
 * @param ClassName     The name of the class
 * When the class name specified does not exist, it is created.
 * @param Methods       An array of function pointers along with their method names.
 * The array must be terminated with the enry (0, 0)
 * @return              Returns true if successful, otherwise false.
 */
bool LuaBindhelper::addMethodsToClass(lua_State *L, const Common::String &className, const luaL_reg *methods) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Load the metatable onto the Lua stack
	if (!getMetatable(L, className)) return false;

	// Register each method in the Metatable
	for (; methods->name; ++methods) {
		lua_pushstring(L, methods->name);
		lua_pushcclosure(L, methods->func, 0);
		lua_settable(L, -3);

		// Function is being permanently registed, so persistence can be ignored
		lua_pushstring(L, methods->name);
		lua_gettable(L, -2);
		registerPermanent(L, className + "." + methods->name);
	}

	// Remove the metatable from the stack
	lua_pop(L, 1);

#ifdef DEBUG
	assert(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

/**
 * Sets the garbage collector callback method when items of a particular class are deleted
 * @param L             A pointer to the Lua VM
 * @param ClassName     The name of the class
 * When the class name specified does not exist, it is created.
 * @param GCHandler     A function pointer
 * @return              Returns true if successful, otherwise false.
 */
bool LuaBindhelper::setClassGCHandler(lua_State *L, const Common::String &className, lua_CFunction GCHandler) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Load the metatable onto the Lua stack
	if (!getMetatable(L, className)) return false;

	// Add the GC handler to the Metatable
	lua_pushstring(L, "__gc");
	lua_pushcclosure(L, GCHandler, 0);
	lua_settable(L, -3);

	// Function is being permanently registed, so persistence can be ignored
	lua_pushstring(L, "__gc");
	lua_gettable(L, -2);
	registerPermanent(L, className + ".__gc");

	// Remove the metatable from the stack
	lua_pop(L, 1);

#ifdef DEBUG
	assert(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

} // End of namespace Sword25

namespace {
void pushMetatableTable(lua_State *L) {
	// Push the Metatable table onto the stack
	lua_getglobal(L, METATABLES_TABLE_NAME);

	// If the table doesn't yet exist, it must be created
	if (lua_isnil(L, -1)) {
		// Pop nil from stack
		lua_pop(L, 1);

		// New table has been created, so add it to the global table and leave reference on stack
		lua_newtable(L);
		lua_pushvalue(L, -1);
		lua_setglobal(L, METATABLES_TABLE_NAME);
	}
}
}

namespace Sword25 {

bool LuaBindhelper::getMetatable(lua_State *L, const Common::String &tableName) {
	// Push the Metatable table onto the stack
	pushMetatableTable(L);

	// Versuchen, die gewünschte Metatabelle auf den Stack zu legen. Wenn sie noch nicht existiert, muss sie erstellt werden.
	lua_getfield(L, -1, tableName.c_str());
	if (lua_isnil(L, -1)) {
		// Pop nil from stack
		lua_pop(L, 1);

		// Create new table
		lua_newtable(L);

		// Set the __index field in the table
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");

		// Flag the table as persisted. This ensures that objects within this table get stored
		lua_pushbooleancpp(L, true);
		lua_setfield(L, -2, "__persist");

		// Set the table name and push it onto the stack
		lua_pushvalue(L, -1);
		lua_setfield(L, -3, tableName.c_str());
	}

	// Remove the Metatable table from the stack
	lua_remove(L, -2);

	return true;
}

// Like luaL_checkudata, only without that no error is generated.
void *LuaBindhelper::my_checkudata(lua_State *L, int ud, const char *tname) {
	int top = lua_gettop(L);

	void *p = lua_touserdata(L, ud);
	if (p != NULL) { /* value is a userdata? */
		if (lua_getmetatable(L, ud)) { /* does it have a metatable? */
			// lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
			LuaBindhelper::getMetatable(L, tname);
			if (lua_rawequal(L, -1, -2)) { /* does it have the correct mt? */
				lua_settop(L, top);
				return p;
			}
		}
	}

	lua_settop(L, top);
	return NULL;
}


bool LuaBindhelper::createTable(lua_State *L, const Common::String &tableName) {
	const char *partBegin = tableName.c_str();

	while (partBegin - tableName.c_str() < (int)tableName.size()) {
		const char *partEnd = strchr(partBegin, '.');
		if (!partEnd)
			partEnd = partBegin + strlen(partBegin);
		Common::String subTableName(partBegin, partEnd);

		// Tables with an empty string as the name are not allowed
		if (subTableName.size() == 0)
			return false;

		// Verify that the table with the name already exists
		// The first round will be searched in the global namespace, with later passages
		// in the corresponding parent table in the stack
		if (partBegin == tableName.c_str()) {
			lua_pushstring(L, subTableName.c_str());
			lua_gettable(L, LUA_GLOBALSINDEX);
		} else {
			lua_pushstring(L, subTableName.c_str());
			lua_gettable(L, -2);
			if (!lua_isnil(L, -1))
				lua_remove(L, -2);
		}

		// If it doesn't exist, create table
		if (lua_isnil(L, -1)) {
			// Pop nil from stack
			lua_pop(L, 1);

			// Create new table
			lua_newtable(L);
			lua_pushstring(L, subTableName.c_str());
			lua_pushvalue(L, -2);
			if (partBegin == tableName.c_str())
				lua_settable(L, LUA_GLOBALSINDEX);
			else {
				lua_settable(L, -4);
				lua_remove(L, -2);
			}
		}

		partBegin = partEnd + 1;
	}

	return true;
}

} // End of namespace Sword25

namespace {
Common::String getLuaValueInfo(lua_State *L, int stackIndex) {
	switch (lua_type(L, stackIndex)) {
	case LUA_TNUMBER:
		lua_pushstring(L, lua_tostring(L, stackIndex));
		break;

	case LUA_TSTRING:
		lua_pushfstring(L, "\"%s\"", lua_tostring(L, stackIndex));
		break;

	case LUA_TBOOLEAN:
		lua_pushstring(L, (lua_toboolean(L, stackIndex) ? "true" : "false"));
		break;

	case LUA_TNIL:
		lua_pushliteral(L, "nil");
		break;

	default:
		lua_pushfstring(L, "%s: %p", luaL_typename(L, stackIndex), lua_topointer(L, stackIndex));
		break;
	}

	Common::String result(lua_tostring(L, -1));
	lua_pop(L, 1);

	return result;
}
}

namespace Sword25 {

Common::String LuaBindhelper::stackDump(lua_State *L) {
	Common::String oss;

	int i = lua_gettop(L);
	oss += "------------------- Stack Dump -------------------\n";

	while (i) {
		oss += Common::String::format("%d: ", i) + getLuaValueInfo(L, i) + "\n";
		i--;
	}

	oss += "-------------- Stack Dump Finished ---------------\n";

	return oss;
}

Common::String LuaBindhelper::tableDump(lua_State *L) {
	Common::String oss;

	oss += "------------------- Table Dump -------------------\n";

	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		// Get the value of the current element on top of the stack, including the index
		oss += getLuaValueInfo(L, -2) + " : " + getLuaValueInfo(L, -1) + "\n";

		// Pop value from the stack. The index is then ready for the next call to lua_next()
		lua_pop(L, 1);
	}

	oss += "-------------- Table Dump Finished ---------------\n";

	return oss;
}

} // End of namespace Sword25
