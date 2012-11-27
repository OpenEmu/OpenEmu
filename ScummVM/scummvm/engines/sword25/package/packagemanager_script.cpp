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

#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"

#include "sword25/package/packagemanager.h"

namespace Sword25 {

static PackageManager *getPM() {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	PackageManager *pPM = pKernel->getPackage();
	assert(pPM);
	return pPM;
}

static int loadPackage(lua_State *L) {
	PackageManager *pPM = getPM();

	lua_pushbooleancpp(L, pPM->loadPackage(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));

	return 1;
}

static int loadDirectoryAsPackage(lua_State *L) {
	PackageManager *pPM = getPM();

	lua_pushbooleancpp(L, pPM->loadDirectoryAsPackage(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));

	return 1;
}

static void splitSearchPath(const Common::String &path, Common::String &directory, Common::String &filter) {
	// Scan backwards for a trailing slash
	const char *sPath = path.c_str();
	const char *lastSlash = sPath + strlen(sPath) - 1;
	while ((lastSlash >= sPath) && (*lastSlash != '/')) --lastSlash;

	if (lastSlash >= sPath) {
		directory = "";
		filter = path;
	} else {
		directory = Common::String(sPath, lastSlash - sPath);
		filter = Common::String(lastSlash + 1);
	}
}

static void doSearch(lua_State *L, const Common::String &path, uint type) {
	PackageManager *pPM = getPM();

	// Der Packagemanager-Service muss den Suchstring und den Pfad getrennt übergeben bekommen.
	// Um die Benutzbarkeit zu verbessern sollen Skriptprogrammierer dieses als ein Pfad übergeben können.
	// Daher muss der übergebene Pfad am letzten Slash aufgesplittet werden.
	Common::String directory;
	Common::String filter;
	splitSearchPath(path, directory, filter);

	// Ergebnistable auf dem Lua-Stack erstellen
	lua_newtable(L);

	// Suche durchführen und die Namen aller gefundenen Dateien in die Ergebnistabelle einfügen.
	// Als Indizes werden fortlaufende Nummern verwandt.
	uint resultNr = 1;
	Common::ArchiveMemberList list;
	int numMatches;

	numMatches = pPM->doSearch(list, filter, directory, type);
	if (numMatches) {
		for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it) {
			lua_pushnumber(L, resultNr);
			lua_pushstring(L, (*it)->getName().c_str());
			lua_settable(L, -3);
			resultNr++;
		}
	}
}

static int findFiles(lua_State *L) {
	doSearch(L, luaL_checkstring(L, 1), PackageManager::FT_FILE);
	return 1;
}

static int findDirectories(lua_State *L) {
	doSearch(L, luaL_checkstring(L, 1), PackageManager::FT_DIRECTORY);
	return 1;
}

static int getFileAsString(lua_State *L) {
	PackageManager *pPM = getPM();

	uint fileSize;
	char *fileData = (char *)pPM->getFile(luaL_checkstring(L, 1), &fileSize);
	if (fileData) {
		lua_pushlstring(L, fileData, fileSize);
		delete[] fileData;

		return 1;
	} else
		return 0;
}

static int fileExists(lua_State *L) {
	lua_pushbooleancpp(L, getPM()->fileExists(luaL_checkstring(L, 1)));
	return 1;
}

// Marks a function that should never be used
static int dummyFuncError(lua_State *L) {
	error("Dummy function invoked by LUA");
	return 1;
}

static const char *PACKAGE_LIBRARY_NAME = "Package";

static const luaL_reg PACKAGE_FUNCTIONS[] = {
	{"LoadPackage", loadPackage},
	{"LoadDirectoryAsPackage", loadDirectoryAsPackage},
	{"GetCurrentDirectory", dummyFuncError},
	{"ChangeDirectory", dummyFuncError},
	{"GetAbsolutePath", dummyFuncError},
	{"GetFileSize", dummyFuncError},
	{"GetFileType", dummyFuncError},
	{"FindFiles", findFiles},
	{"FindDirectories", findDirectories},
	{"GetFileAsString", getFileAsString},
	{"FileExists", fileExists},
	{0, 0}
};

bool PackageManager::registerScriptBindings() {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	ScriptEngine *pScript = pKernel->getScript();
	assert(pScript);
	lua_State *L = static_cast<lua_State *>(pScript->getScriptObject());
	assert(L);

	if (!LuaBindhelper::addFunctionsToLib(L, PACKAGE_LIBRARY_NAME, PACKAGE_FUNCTIONS))
		return false;

	return true;
}

} // End of namespace Sword25
