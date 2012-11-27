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

#include "sword25/math/vertex.h"

#include "sword25/util/lua/lua.h"
#include "sword25/util/lua/lauxlib.h"

namespace Sword25 {

Vertex &Vertex::luaVertexToVertex(lua_State *L, int stackIndex, Vertex &vertex) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Ensure that we actually consider a table
	luaL_checktype(L, stackIndex, LUA_TTABLE);

	// Read X Component
	lua_pushstring(L, "X");
	lua_gettable(L, stackIndex);
	if (!lua_isnumber(L, -1)) luaL_argcheck(L, 0, stackIndex, "the X component has to be a number");
	vertex.x = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);

	// Read Y Component
	lua_pushstring(L, "Y");
	lua_gettable(L, stackIndex);
	if (!lua_isnumber(L, -1)) luaL_argcheck(L, 0, stackIndex, "the Y component has to be a number");
	vertex.y = static_cast<int>(lua_tonumber(L, -1));
	lua_pop(L, 1);

#ifdef DEBUG
	assert(__startStackDepth == lua_gettop(L));
#endif

	return vertex;
}

void Vertex::vertexToLuaVertex(lua_State *L, const Vertex &vertex) {
	// Create New Table
	lua_newtable(L);

	// X value is written to table
	lua_pushstring(L, "X");
	lua_pushnumber(L, vertex.x);
	lua_settable(L, -3);

	// Y value is written to table
	lua_pushstring(L, "Y");
	lua_pushnumber(L, vertex.y);
	lua_settable(L, -3);
}

} // End of namespace Sword25
