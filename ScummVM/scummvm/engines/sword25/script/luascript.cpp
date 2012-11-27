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

#include "common/array.h"
#include "common/debug-channels.h"

#include "sword25/sword25.h"
#include "sword25/package/packagemanager.h"
#include "sword25/script/luascript.h"
#include "sword25/script/luabindhelper.h"

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

#include "sword25/util/lua/lua.h"
#include "sword25/util/lua/lualib.h"
#include "sword25/util/lua/lauxlib.h"
#include "sword25/util/pluto/pluto.h"

namespace Sword25 {

LuaScriptEngine::LuaScriptEngine(Kernel *KernelPtr) :
	ScriptEngine(KernelPtr),
	_state(0),
	_pcallErrorhandlerRegistryIndex(0) {
}

LuaScriptEngine::~LuaScriptEngine() {
	// Lua de-initialisation
	if (_state)
		lua_close(_state);
}

namespace {
int panicCB(lua_State *L) {
	error("Lua panic. Error message: %s", lua_isnil(L, -1) ? "" : lua_tostring(L, -1));
	return 0;
}

void debugHook(lua_State *L, lua_Debug *ar) {
	if (!lua_getinfo(L, "Sn", ar))
		return;

	debug("LUA: %s %s: %s %d", ar->namewhat, ar->name, ar->short_src, ar->currentline);
}
}

bool LuaScriptEngine::init() {
	// Lua-State initialisation, as well as standard libaries initialisation
	_state = luaL_newstate();
	if (!_state || ! registerStandardLibs() || !registerStandardLibExtensions()) {
		error("Lua could not be initialized.");
		return false;
	}

	// Register panic callback function
	lua_atpanic(_state, panicCB);

	// Error handler for lua_pcall calls
	// The code below contains a local error handler function
	const char errorHandlerCode[] =
	    "local function ErrorHandler(message) "
	    "	return message .. '\\n' .. debug.traceback('', 2) "
	    "end "
	    "return ErrorHandler";

	// Compile the code
	if (luaL_loadbuffer(_state, errorHandlerCode, strlen(errorHandlerCode), "PCALL ERRORHANDLER") != 0) {
		// An error occurred, so dislay the reason and exit
		error("Couldn't compile luaL_pcall errorhandler:\n%s", lua_tostring(_state, -1));
		lua_pop(_state, 1);

		return false;
	}
	// Running the code, the error handler function sets the top of the stack
	if (lua_pcall(_state, 0, 1, 0) != 0) {
		// An error occurred, so dislay the reason and exit
		error("Couldn't prepare luaL_pcall errorhandler:\n%s", lua_tostring(_state, -1));
		lua_pop(_state, 1);

		return false;
	}

	// Place the error handler function in the Lua registry, and remember the index
	_pcallErrorhandlerRegistryIndex = luaL_ref(_state, LUA_REGISTRYINDEX);

	// Initialize the Pluto-Persistence library
	luaopen_pluto(_state);
	lua_pop(_state, 1);

	// Initialize debugging callback
	if (DebugMan.isDebugChannelEnabled(kDebugScript)) {
		int mask = 0;
		if ((gDebugLevel & 1) != 0)
			mask |= LUA_MASKCALL;
		if ((gDebugLevel & 2) != 0)
			mask |= LUA_MASKRET;
		if ((gDebugLevel & 4) != 0)
			mask |= LUA_MASKLINE;

		if (mask != 0)
			lua_sethook(_state, debugHook, mask, 0);
	}

	debugC(kDebugScript, "Lua initialized.");

	return true;
}

bool LuaScriptEngine::executeFile(const Common::String &fileName) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(_state);
#endif
	debug(2, "LuaScriptEngine::executeFile(%s)", fileName.c_str());

	// Get a pointer to the package manager
	PackageManager *pPackage = Kernel::getInstance()->getPackage();
	assert(pPackage);

	// File read
	uint fileSize;
	byte *fileData = pPackage->getFile(fileName, &fileSize);
	if (!fileData) {
		error("Couldn't read \"%s\".", fileName.c_str());
#ifdef DEBUG
		assert(__startStackDepth == lua_gettop(_state));
#endif
		return false;
	}

	// Run the file content
	if (!executeBuffer(fileData, fileSize, "@" + pPackage->getAbsolutePath(fileName))) {
		// Release file buffer
		delete[] fileData;
#ifdef DEBUG
		assert(__startStackDepth == lua_gettop(_state));
#endif
		return false;
	}

	// Release file buffer
	delete[] fileData;

#ifdef DEBUG
	assert(__startStackDepth == lua_gettop(_state));
#endif

	return true;
}

bool LuaScriptEngine::executeString(const Common::String &code) {
	return executeBuffer((const byte *)code.c_str(), code.size(), "???");
}

namespace {

void removeForbiddenFunctions(lua_State *L) {
	static const char *FORBIDDEN_FUNCTIONS[] = {
		"dofile",
		0
	};

	const char **iterator = FORBIDDEN_FUNCTIONS;
	while (*iterator) {
		lua_pushnil(L);
		lua_setfield(L, LUA_GLOBALSINDEX, *iterator);
		++iterator;
	}
}
}

bool LuaScriptEngine::registerStandardLibs() {
	luaL_openlibs(_state);
	removeForbiddenFunctions(_state);
	return true;
}

bool LuaScriptEngine::executeBuffer(const byte *data, uint size, const Common::String &name) const {
	// Compile buffer
	if (luaL_loadbuffer(_state, (const char *)data, size, name.c_str()) != 0) {
		error("Couldn't compile \"%s\":\n%s", name.c_str(), lua_tostring(_state, -1));
		lua_pop(_state, 1);

		return false;
	}

	// Error handling function to be executed after the function is put on the stack
	lua_rawgeti(_state, LUA_REGISTRYINDEX, _pcallErrorhandlerRegistryIndex);
	lua_insert(_state, -2);

	// Run buffer contents
	if (lua_pcall(_state, 0, 0, -2) != 0) {
		error("An error occured while executing \"%s\":\n%s.",
		               name.c_str(),
		               lua_tostring(_state, -1));
		lua_pop(_state, 2);

		return false;
	}

	// Remove the error handler function from the stack
	lua_pop(_state, 1);

	return true;
}

void LuaScriptEngine::setCommandLine(const Common::StringArray &commandLineParameters) {
	lua_newtable(_state);

	for (size_t i = 0; i < commandLineParameters.size(); ++i) {
		lua_pushnumber(_state, i + 1);
		lua_pushstring(_state, commandLineParameters[i].c_str());
		lua_settable(_state, -3);
	}

	lua_setglobal(_state, "CommandLine");
}

namespace {
const char *PERMANENTS_TABLE_NAME = "Permanents";

// This array contains the name of global Lua objects that should not be persisted
const char *STANDARD_PERMANENTS[] = {
	"string",
	"xpcall",
	"package",
	"tostring",
	"print",
	"os",
	"unpack",
	"require",
	"getfenv",
	"setmetatable",
	"next",
	"assert",
	"tonumber",
	"io",
	"rawequal",
	"collectgarbage",
	"getmetatable",
	"module",
	"rawset",
	"warning",
	"math",
	"debug",
	"pcall",
	"table",
	"newproxy",
	"type",
	"coroutine",
	"select",
	"gcinfo",
	"pairs",
	"rawget",
	"loadstring",
	"ipairs",
	"_VERSION",
	"setfenv",
	"load",
	"error",
	"loadfile",

	"pairs_next",
	"ipairs_next",
	"pluto",
	"Cfg",
	"Translator",
	"Persistence",
	"CommandLine",
	0
};

enum PERMANENT_TABLE_TYPE {
	PTT_PERSIST,
	PTT_UNPERSIST
};

bool pushPermanentsTable(lua_State *L, PERMANENT_TABLE_TYPE tableType) {
	// Permanents-Table
	lua_newtable(L);

	// All standard permanents are inserted into this table
	uint Index = 0;
	while (STANDARD_PERMANENTS[Index]) {
		// Permanents are placed onto the stack; if it does not exist, it is simply ignored
		lua_getglobal(L, STANDARD_PERMANENTS[Index]);
		if (!lua_isnil(L, -1)) {
			// Name of the element as a unique value on the stack
			lua_pushstring(L, STANDARD_PERMANENTS[Index]);

			// If it is loaded, then it can be used
			// In this case, the position of name and object are reversed on the stack
			if (tableType == PTT_UNPERSIST)
				lua_insert(L, -2);

			// Make an entry in the table
			lua_settable(L, -3);
		} else {
			// Pop nil value from stack
			lua_pop(L, 1);
		}

		++Index;
	}

	// All registered C functions to be inserted into the table
	// BS_LuaBindhelper places in the register a table in which all registered C functions
	// are stored

	// Table is put on the stack
	lua_getfield(L, LUA_REGISTRYINDEX, PERMANENTS_TABLE_NAME);

	if (!lua_isnil(L, -1)) {
		// Iterate over all elements of the table
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			// Value and index duplicated on the stack and changed in the sequence
			lua_pushvalue(L, -1);
			lua_pushvalue(L, -3);

			// If it is loaded, then it can be used
			// In this case, the position of name and object are reversed on the stack
			if (tableType == PTT_UNPERSIST)
				lua_insert(L, -2);

			// Make an entry in the results table
			lua_settable(L, -6);

			// Pop value from the stack. The index is then ready for the next call to lua_next()
			lua_pop(L, 1);
		}
	}

	// Pop the C-Permanents table from the stack
	lua_pop(L, 1);

	// coroutine.yield must be registered in the extra-Permanents table because they
	// are inactive coroutine C functions on the stack

	// Function coroutine.yield placed on the stack
	lua_getglobal(L, "coroutine");
	lua_pushstring(L, "yield");
	lua_gettable(L, -2);

	// Store coroutine.yield with it's own unique value in the Permanents table
	lua_pushstring(L, "coroutine.yield");

	if (tableType == PTT_UNPERSIST)
		lua_insert(L, -2);

	lua_settable(L, -4);

	// Coroutine table is popped from the stack
	lua_pop(L, 1);

	return true;
}
}

namespace {
int chunkwriter(lua_State *L, const void *p, size_t sz, void *ud) {
	Common::Array<byte> & chunkData = *reinterpret_cast<Common::Array<byte> * >(ud);
	const byte *buffer = reinterpret_cast<const byte *>(p);

	while (sz--)
		chunkData.push_back(*buffer++);

	return 1;
}
}

bool LuaScriptEngine::persist(OutputPersistenceBlock &writer) {
	// Empty the Lua stack. pluto_persist() xepects that the stack is empty except for its parameters
	lua_settop(_state, 0);

	// Garbage Collection erzwingen.
	lua_gc(_state, LUA_GCCOLLECT, 0);

	// Permanents-Table is set on the stack
	// pluto_persist expects these two items on the Lua stack
	pushPermanentsTable(_state, PTT_PERSIST);
	lua_getglobal(_state, "_G");

	// Lua persists and stores the data in a Common::Array
	Common::Array<byte> chunkData;
	pluto_persist(_state, chunkwriter, &chunkData);

	// Persistenzdaten in den Writer schreiben.
	writer.writeByteArray(chunkData);

	// Die beiden Tabellen vom Stack nehmen.
	lua_pop(_state, 2);

	return true;
}

namespace {

struct ChunkreaderData {
	void   *BufferPtr;
	size_t  Size;
	bool    BufferReturned;
};

const char *chunkreader(lua_State *L, void *ud, size_t *sz) {
	ChunkreaderData &cd = *reinterpret_cast<ChunkreaderData *>(ud);

	if (!cd.BufferReturned) {
		cd.BufferReturned = true;
		*sz = cd.Size;
		return reinterpret_cast<const char *>(cd.BufferPtr);
	} else {
		return 0;
	}
}

void clearGlobalTable(lua_State *L, const char **exceptions) {
	// Iterate over all elements of the global table
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		// Now the value and the index of the current element is on the stack
		// This value does not interest us, so it is popped from the stack
		lua_pop(L, 1);

		// Determine whether the item is set to nil, so you want to remove from the global table.
		// For this will determine whether the element name is a string and is present in
		// the list of exceptions
		bool setElementToNil = true;
		if (lua_isstring(L, -1)) {
			const char *indexString = lua_tostring(L, -1);
			const char **exceptionsWalker = exceptions;
			while (*exceptionsWalker) {
				if (strcmp(indexString, *exceptionsWalker) == 0)
					setElementToNil = false;
				++exceptionsWalker;
			}
		}

		// If the above test showed that the item should be removed, it is removed by setting the value to nil.
		if (setElementToNil) {
			lua_pushvalue(L, -1);
			lua_pushnil(L);
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}

	// Pop the Global table from the stack
	lua_pop(L, 1);

	// Perform garbage collection, so that all removed elements are deleted
	lua_gc(L, LUA_GCCOLLECT, 0);
}
}

bool LuaScriptEngine::unpersist(InputPersistenceBlock &reader) {
	// Empty the Lua stack. pluto_persist() xepects that the stack is empty except for its parameters
	lua_settop(_state, 0);

	// Permanents table is placed on the stack. This has already happened at this point, because
	// to create the table all permanents must be accessible. This is the case only for the
	// beginning of the function, because the global table is emptied below
	pushPermanentsTable(_state, PTT_UNPERSIST);

	// All items from global table of _G and __METATABLES are removed.
	// After a garbage collection is performed, and thus all managed objects deleted

	// __METATABLES is not immediately removed becausen the Metatables are needed
	// for the finalisers of objects.
	static const char *clearExceptionsFirstPass[] = {
		"_G",
		"__METATABLES",
		0
	};
	clearGlobalTable(_state, clearExceptionsFirstPass);

	// In the second pass, the Metatables are removed
	static const char *clearExceptionsSecondPass[] = {
		"_G",
		0
	};
	clearGlobalTable(_state, clearExceptionsSecondPass);

	// Persisted Lua data
	Common::Array<byte> chunkData;
	reader.readByteArray(chunkData);

	// Chunk-Reader initialisation. It is used with pluto_unpersist to restore read data
	ChunkreaderData cd;
	cd.BufferPtr = &chunkData[0];
	cd.Size = chunkData.size();
	cd.BufferReturned = false;

	pluto_unpersist(_state, chunkreader, &cd);

	// Permanents-Table is removed from stack
	lua_remove(_state, -2);

	// The read elements in the global table about
	lua_pushnil(_state);
	while (lua_next(_state, -2) != 0) {
		// The referenec to the global table (_G) must not be overwritten, or ticks from Lua total
		bool isGlobalReference = lua_isstring(_state, -2) && strcmp(lua_tostring(_state, -2), "_G") == 0;
		if (!isGlobalReference) {
			lua_pushvalue(_state, -2);
			lua_pushvalue(_state, -2);

			lua_settable(_state, LUA_GLOBALSINDEX);
		}

		// Pop value from the stack. The index is then ready for the next call to lua_next()
		lua_pop(_state, 1);
	}

	// The table with the loaded data is popped from the stack
	lua_pop(_state, 1);

	// Force garbage collection
	lua_gc(_state, LUA_GCCOLLECT, 0);

	return true;
}

} // End of namespace Sword25
