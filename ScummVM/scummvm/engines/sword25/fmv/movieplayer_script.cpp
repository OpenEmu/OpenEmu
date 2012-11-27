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

#include "common/scummsys.h"	// for USE_THEORADEC

#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"

#include "sword25/fmv/movieplayer.h"

namespace Sword25 {

int loadMovie(lua_State *L) {
	MoviePlayer *FMVPtr = Kernel::getInstance()->getFMV();
	assert(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->loadMovie(luaL_checkstring(L, 1), lua_gettop(L) == 2 ? static_cast<uint>(luaL_checknumber(L, 2)) : 10));

	return 1;
}

int unloadMovie(lua_State *L) {
	MoviePlayer *FMVPtr = Kernel::getInstance()->getFMV();
	assert(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->unloadMovie());

	return 1;
}

int play(lua_State *L) {
	MoviePlayer *FMVPtr = Kernel::getInstance()->getFMV();
	assert(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->play());

	return 1;
}

int pause(lua_State *L) {
	MoviePlayer *FMVPtr = Kernel::getInstance()->getFMV();
	assert(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->pause());

	return 1;
}

int update(lua_State *L) {
	MoviePlayer *FMVPtr = Kernel::getInstance()->getFMV();
	assert(FMVPtr);

	FMVPtr->update();

	return 0;
}

int isMovieLoaded(lua_State *L) {
	MoviePlayer *FMVPtr = Kernel::getInstance()->getFMV();
	assert(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->isMovieLoaded());

	return 1;
}

int isPaused(lua_State *L) {
	MoviePlayer *FMVPtr = Kernel::getInstance()->getFMV();
	assert(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->isPaused());

	return 1;
}

int getScaleFactor(lua_State *L) {
	MoviePlayer *FMVPtr = Kernel::getInstance()->getFMV();
	assert(FMVPtr);

	lua_pushnumber(L, FMVPtr->getScaleFactor());

	return 1;
}

int setScaleFactor(lua_State *L) {
	MoviePlayer *FMVPtr = Kernel::getInstance()->getFMV();
	assert(FMVPtr);

	FMVPtr->setScaleFactor(static_cast<float>(luaL_checknumber(L, 1)));

	return 0;
}

int getTime(lua_State *L) {
	MoviePlayer *FMVPtr = Kernel::getInstance()->getFMV();
	assert(FMVPtr);

	lua_pushnumber(L, FMVPtr->getTime());

	return 1;
}

const char *LIBRARY_NAME = "Movieplayer";

const luaL_reg LIBRARY_FUNCTIONS[] = {
	{ "LoadMovie", loadMovie },
	{ "UnloadMovie", unloadMovie },
	{ "Play", play },
	{ "Pause", pause },
	{ "Update", update },
	{ "IsMovieLoaded", isMovieLoaded },
	{ "IsPaused", isPaused },
	{ "GetScaleFactor", getScaleFactor },
	{ "SetScaleFactor", setScaleFactor },
	{ "GetTime", getTime },
	{ 0, 0 }
};

bool MoviePlayer::registerScriptBindings() {
	ScriptEngine *pScript = Kernel::getInstance()->getScript();
	assert(pScript);
	lua_State *L = static_cast<lua_State *>(pScript->getScriptObject());
	assert(L);

	if (!LuaBindhelper::addFunctionsToLib(L, LIBRARY_NAME, LIBRARY_FUNCTIONS)) return false;

	return true;
}

} // End of namespace Sword25
