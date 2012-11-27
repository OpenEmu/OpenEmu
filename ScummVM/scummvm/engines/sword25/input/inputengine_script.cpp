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

#include "common/ptr.h"
#include "common/str.h"
#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"
#include "sword25/script/luacallback.h"

#include "sword25/input/inputengine.h"

namespace Sword25 {

static void theCharacterCallback(int character);
static void theCommandCallback(int command);

namespace {
class CharacterCallbackClass : public LuaCallback {
public:
	CharacterCallbackClass(lua_State *L) : LuaCallback(L) {}

	Common::String _character;

protected:
	int PreFunctionInvokation(lua_State *L) {
		lua_pushstring(L, _character.c_str());
		return 1;
	}
};

static CharacterCallbackClass *characterCallbackPtr = 0;	// FIXME: should be turned into InputEngine member var

class CommandCallbackClass : public LuaCallback {
public:
	CommandCallbackClass(lua_State *L) : LuaCallback(L) {
		_command = InputEngine::KEY_COMMAND_BACKSPACE;
	}

	InputEngine::KEY_COMMANDS _command;

protected:
	int preFunctionInvokation(lua_State *L) {
		lua_pushnumber(L, _command);
		return 1;
	}
};

static CommandCallbackClass *commandCallbackPtr = 0;	// FIXME: should be turned into InputEngine member var

}

static InputEngine *getIE() {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	InputEngine *pIE = pKernel->getInput();
	assert(pIE);
	return pIE;
}

static int init(lua_State *L) {
	InputEngine *pIE = getIE();

	lua_pushbooleancpp(L, pIE->init());
	return 1;
}

static int update(lua_State *L) {
	InputEngine *pIE = getIE();

	pIE->update();
	return 0;
}

static int isLeftMouseDown(lua_State *L) {
	InputEngine *pIE = getIE();

	lua_pushbooleancpp(L, pIE->isLeftMouseDown());
	return 1;
}

static int isRightMouseDown(lua_State *L) {
	InputEngine *pIE = getIE();

	lua_pushbooleancpp(L, pIE->isRightMouseDown());
	return 1;
}

static int wasLeftMouseDown(lua_State *L) {
	InputEngine *pIE = getIE();

	lua_pushbooleancpp(L, pIE->wasLeftMouseDown());
	return 1;
}

static int wasRightMouseDown(lua_State *L) {
	InputEngine *pIE = getIE();

	lua_pushbooleancpp(L, pIE->wasRightMouseDown());
	return 1;
}

static int isLeftDoubleClick(lua_State *L) {
	InputEngine *pIE = getIE();

	lua_pushbooleancpp(L, pIE->isLeftDoubleClick());
	return 1;
}

static int getMouseX(lua_State *L) {
	InputEngine *pIE = getIE();

	lua_pushnumber(L, pIE->getMouseX());
	return 1;
}

static int getMouseY(lua_State *L) {
	InputEngine *pIE = getIE();

	lua_pushnumber(L, pIE->getMouseY());
	return 1;
}

static int isKeyDown(lua_State *L) {
	InputEngine *pIE = getIE();

	lua_pushbooleancpp(L, pIE->isKeyDown((uint)luaL_checknumber(L, 1)));
	return 1;
}

static int wasKeyDown(lua_State *L) {
	InputEngine *pIE = getIE();

	lua_pushbooleancpp(L, pIE->wasKeyDown((uint)luaL_checknumber(L, 1)));
	return 1;
}

static void theCharacterCallback(int character) {
	characterCallbackPtr->_character = static_cast<byte>(character);
	lua_State *L = static_cast<lua_State *>(Kernel::getInstance()->getScript()->getScriptObject());
	characterCallbackPtr->invokeCallbackFunctions(L, 1);
}

static void theCommandCallback(int command) {
	commandCallbackPtr->_command = static_cast<InputEngine::KEY_COMMANDS>(command);
	lua_State *L = static_cast<lua_State *>(Kernel::getInstance()->getScript()->getScriptObject());
	commandCallbackPtr->invokeCallbackFunctions(L, 1);
}

// Marks a function that should never be used
static int dummyFuncError(lua_State *L) {
	error("Dummy function invoked by LUA");
	return 1;
}

static const char *PACKAGE_LIBRARY_NAME = "Input";

static const luaL_reg PACKAGE_FUNCTIONS[] = {
	{"Init", init},
	{"Update", update},
	{"IsLeftMouseDown", isLeftMouseDown},
	{"IsRightMouseDown", isRightMouseDown},
	{"WasLeftMouseDown", wasLeftMouseDown},
	{"WasRightMouseDown", wasRightMouseDown},
	{"IsLeftDoubleClick", isLeftDoubleClick},
	{"GetMouseX", getMouseX},
	{"GetMouseY", getMouseY},
	{"SetMouseX", dummyFuncError},
	{"SetMouseY", dummyFuncError},
	{"IsKeyDown", isKeyDown},
	{"WasKeyDown", wasKeyDown},
	{"RegisterCharacterCallback", dummyFuncError},	// debug
	{"UnregisterCharacterCallback", dummyFuncError},
	{"RegisterCommandCallback", dummyFuncError},
	{"UnregisterCommandCallback", dummyFuncError},
	{0, 0}
};

#define X(k) {"KEY_" #k, InputEngine::KEY_##k}
#define Y(k) {"KEY_COMMAND_" #k, InputEngine::KEY_COMMAND_##k}
static const lua_constant_reg PACKAGE_CONSTANTS[] = {
	X(BACKSPACE), X(TAB), X(CLEAR), X(RETURN), X(PAUSE), X(CAPSLOCK), X(ESCAPE), X(SPACE), X(PAGEUP), X(PAGEDOWN), X(END), X(HOME), X(LEFT),
	X(UP), X(RIGHT), X(DOWN), X(PRINTSCREEN), X(INSERT), X(DELETE), X(0), X(1), X(2), X(3), X(4), X(5), X(6), X(7), X(8), X(9), X(A), X(B),
	X(C), X(D), X(E), X(F), X(G), X(H), X(I), X(J), X(K), X(L), X(M), X(N), X(O), X(P), X(Q), X(R), X(S), X(T), X(U), X(V), X(W), X(X), X(Y),
	X(Z), X(NUMPAD0), X(NUMPAD1), X(NUMPAD2), X(NUMPAD3), X(NUMPAD4), X(NUMPAD5), X(NUMPAD6), X(NUMPAD7), X(NUMPAD8), X(NUMPAD9), X(MULTIPLY),
	X(ADD), X(SEPARATOR), X(SUBTRACT), X(DECIMAL), X(DIVIDE), X(F1), X(F2), X(F3), X(F4), X(F5), X(F6),  X(F7), X(F8), X(F9), X(F10), X(F11),
	X(F12), X(NUMLOCK), X(SCROLL), X(LSHIFT), X(RSHIFT), X(LCONTROL), X(RCONTROL),
	Y(ENTER), Y(LEFT), Y(RIGHT), Y(HOME), Y(END), Y(BACKSPACE), Y(TAB), Y(INSERT), Y(DELETE),
	{0, 0}
};
#undef X
#undef Y

// -----------------------------------------------------------------------------

bool InputEngine::registerScriptBindings() {
	Kernel *pKernel = Kernel::getInstance();
	assert(pKernel);
	ScriptEngine *pScript = pKernel->getScript();
	assert(pScript);
	lua_State *L = static_cast<lua_State *>(pScript->getScriptObject());
	assert(L);

	if (!LuaBindhelper::addFunctionsToLib(L, PACKAGE_LIBRARY_NAME, PACKAGE_FUNCTIONS)) return false;
	if (!LuaBindhelper::addConstantsToLib(L, PACKAGE_LIBRARY_NAME, PACKAGE_CONSTANTS)) return false;

	assert(characterCallbackPtr == 0);
	characterCallbackPtr = new CharacterCallbackClass(L);

	assert(commandCallbackPtr == 0);
	commandCallbackPtr = new CommandCallbackClass(L);

	setCharacterCallback(theCharacterCallback);
	setCommandCallback(theCommandCallback);

	return true;
}

void InputEngine::unregisterScriptBindings() {
	delete characterCallbackPtr;
	characterCallbackPtr = 0;

	delete commandCallbackPtr;
	commandCallbackPtr = 0;
}

} // End of namespace Sword25
