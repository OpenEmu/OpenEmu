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

#include "common/algorithm.h"
#include "common/events.h"
#include "common/system.h"
#include "common/util.h"

#include "sword25/sword25.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/input/inputengine.h"

namespace Sword25 {

#define DOUBLE_CLICK_TIME 500
#define DOUBLE_CLICK_RECT_SIZE 4

InputEngine::InputEngine(Kernel *pKernel) :
	Service(pKernel),
	_currentState(0),
	_leftMouseDown(false),
	_rightMouseDown(false),
	_mouseX(0),
	_mouseY(0),
	_leftDoubleClick(false),
	_doubleClickTime(DOUBLE_CLICK_TIME),
	_doubleClickRectWidth(DOUBLE_CLICK_RECT_SIZE),
	_doubleClickRectHeight(DOUBLE_CLICK_RECT_SIZE),
	_lastLeftClickTime(0),
	_lastLeftClickMouseX(0),
	_lastLeftClickMouseY(0) {
	memset(_keyboardState[0], 0, sizeof(_keyboardState[0]));
	memset(_keyboardState[1], 0, sizeof(_keyboardState[1]));
	_leftMouseState[0] = false;
	_leftMouseState[1] = false;
	_rightMouseState[0] = false;
	_rightMouseState[1] = false;

	if (!registerScriptBindings())
		error("Script bindings could not be registered.");
	else
		debugC(kDebugScript, "Script bindings registered.");
}

InputEngine::~InputEngine() {
	unregisterScriptBindings();
}

bool InputEngine::init() {
	// No initialisation needed
	return true;
}

void InputEngine::update() {
	Common::Event event;

	// We keep two sets of keyboard states: The current one, and that of
	// the previous frame. This allows us to detect which keys changed
	// state. Also, by keeping a single central keystate array, we
	// ensure that all script queries for key state during a single
	// frame get the same consistent replies.
	_currentState ^= 1;
	memcpy(_keyboardState[_currentState], _keyboardState[_currentState ^ 1], sizeof(_keyboardState[0]));

	// Loop through processing any pending events
	bool handleEvents = true;
	while (handleEvents && g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
			_leftMouseDown = event.type == Common::EVENT_LBUTTONDOWN;
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			handleEvents = false;
			break;
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:
			_rightMouseDown = event.type == Common::EVENT_RBUTTONDOWN;
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			handleEvents = false;
			break;

		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;

		case Common::EVENT_KEYDOWN:
		case Common::EVENT_KEYUP:
			// FIXME - Need to work out how to expose getDebugger() to this module
			//if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_d && event.type == Common::EVENT_KEYDOWN) {
			//	_vm->getDebugger()->attach();
			//	_vm->getDebugger()->onFrame();
			//}

			alterKeyboardState(event.kbd.keycode, (event.type == Common::EVENT_KEYDOWN) ? 0x80 : 0);
			break;

		default:
			break;
		}
	}

	_leftMouseState[_currentState] = _leftMouseDown;
	_rightMouseState[_currentState] = _rightMouseDown;

	testForLeftDoubleClick();
}

bool InputEngine::isLeftMouseDown() {
	return _leftMouseDown;
}

bool InputEngine::isRightMouseDown() {
	return _rightMouseDown;
}

void InputEngine::testForLeftDoubleClick() {
	_leftDoubleClick = false;

	// Only bother checking for a double click if the left mouse button was clicked
	if (wasLeftMouseDown()) {
		// Get the time now
		uint now = Kernel::getInstance()->getMilliTicks();

		// A double click is signalled if
		// 1. The two clicks are close enough together
		// 2. The mouse cursor hasn't moved much
		if (now - _lastLeftClickTime <= _doubleClickTime &&
		        ABS(_mouseX - _lastLeftClickMouseX) <= _doubleClickRectWidth / 2 &&
		        ABS(_mouseY - _lastLeftClickMouseY) <= _doubleClickRectHeight / 2) {
			_leftDoubleClick = true;

			// Reset the time and position of the last click, so that clicking is not
			// interpreted as the first click of a further double-click
			_lastLeftClickTime = 0;
			_lastLeftClickMouseX = 0;
			_lastLeftClickMouseY = 0;
		} else {
			// There is no double click. Remember the position and time of the click,
			// in case it's the first click of a double-click sequence
			_lastLeftClickTime = now;
			_lastLeftClickMouseX = _mouseX;
			_lastLeftClickMouseY = _mouseY;
		}
	}
}

void InputEngine::alterKeyboardState(int keycode, byte newState) {
	assert(keycode < ARRAYSIZE(_keyboardState[_currentState]));
	_keyboardState[_currentState][keycode] = newState;
}

bool InputEngine::isLeftDoubleClick() {
	return _leftDoubleClick;
}

bool InputEngine::wasLeftMouseDown() {
	return (_leftMouseState[_currentState] == false) && (_leftMouseState[_currentState ^ 1] == true);
}

bool InputEngine::wasRightMouseDown() {
	return (_rightMouseState[_currentState] == false) && (_rightMouseState[_currentState ^ 1] == true);
}

int InputEngine::getMouseX() {
	return _mouseX;
}

int InputEngine::getMouseY() {
	return _mouseY;
}

bool InputEngine::isKeyDown(uint keyCode) {
	assert(keyCode < ARRAYSIZE(_keyboardState[_currentState]));
	return (_keyboardState[_currentState][keyCode] & 0x80) != 0;
}

bool InputEngine::wasKeyDown(uint keyCode) {
	assert(keyCode < ARRAYSIZE(_keyboardState[_currentState]));
	return ((_keyboardState[_currentState][keyCode] & 0x80) == 0) &&
	       ((_keyboardState[_currentState ^ 1][keyCode] & 0x80) != 0);
}

void InputEngine::setCharacterCallback(CharacterCallback callback) {
	_characterCallback = callback;
}

void InputEngine::setCommandCallback(CommandCallback callback) {
	_commandCallback = callback;
}

void InputEngine::reportCharacter(byte character) {
	if (_characterCallback)
		(*_characterCallback)(character);
}

void InputEngine::reportCommand(KEY_COMMANDS command) {
	if (_commandCallback)
		(*_commandCallback)(command);
}

bool InputEngine::persist(OutputPersistenceBlock &writer) {
	// Write out the number of command callbacks and their names.
	// Note: We do this only for compatibility with older engines resp.
	// the original engine.
	writer.write((uint)1);
	writer.writeString("LuaCommandCB");

	// Write out the number of command callbacks and their names.
	// Note: We do this only for compatibility with older engines resp.
	// the original engine.
	writer.write((uint)1);
	writer.writeString("LuaCharacterCB");

	return true;
}

bool InputEngine::unpersist(InputPersistenceBlock &reader) {
	Common::String callbackFunctionName;

	// Read number of command callbacks and their names.
	// Note: We do this only for compatibility with older engines resp.
	// the original engine.
	uint commandCallbackCount;
	reader.read(commandCallbackCount);
	assert(commandCallbackCount == 1);

	reader.readString(callbackFunctionName);
	assert(callbackFunctionName == "LuaCommandCB");

	// Read number of character callbacks and their names.
	// Note: We do this only for compatibility with older engines resp.
	// the original engine.
	uint characterCallbackCount;
	reader.read(characterCallbackCount);
	assert(characterCallbackCount == 1);

	reader.readString(callbackFunctionName);
	assert(callbackFunctionName == "LuaCharacterCB");

	return reader.isGood();
}

} // End of namespace Sword25
