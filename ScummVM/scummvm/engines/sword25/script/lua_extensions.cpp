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

#include "sword25/script/luascript.h"
#include "sword25/script/luabindhelper.h"

namespace Sword25 {

static int warning(lua_State *L) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	luaL_checkstring(L, 1);
	luaL_where(L, 1);
	lua_pushstring(L, "WARNING - ");
	lua_pushvalue(L, 1);
	lua_concat(L, 3);
	lua_pop(L, 1);

#ifdef DEBUG
	assert(__startStackDepth == lua_gettop(L));
#endif

	return 0;
}

static const luaL_reg GLOBAL_FUNCTIONS[] = {
	{"warning", warning},
	{0, 0}
};

bool LuaScriptEngine::registerStandardLibExtensions() {
	lua_State *L = _state;
	assert(_state);

	if (!LuaBindhelper::addFunctionsToLib(L, "", GLOBAL_FUNCTIONS))
		return false;

	return true;
}

} // End of namespace Sword25
