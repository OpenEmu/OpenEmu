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

/*
 * BS_InputEngine
 * -------------
 * This is the input interface engine that contains all the methods that an
 * input source must implement.
 * All input engines must be derived from this class.
 *
 * Autor: Alex Arnst
 */

#ifndef SWORD25_INPUTENGINE_H
#define SWORD25_INPUTENGINE_H

#include "common/keyboard.h"

#include "sword25/kernel/common.h"
#include "sword25/kernel/service.h"
#include "sword25/kernel/persistable.h"

namespace Sword25 {

/// Class definitions

class InputEngine : public Service, public Persistable {
public:
	InputEngine(Kernel *pKernel);
	~InputEngine();

	// NOTE: These codes are registered in inputengine_script.cpp
	// If you add or remove entries of this enum, you must also adjust
	// the above file.
	enum KEY_CODES  {
		KEY_BACKSPACE   = Common::KEYCODE_BACKSPACE,
		KEY_TAB         = Common::KEYCODE_TAB,
		KEY_CLEAR       = Common::KEYCODE_CLEAR,
		KEY_RETURN      = Common::KEYCODE_RETURN,
		KEY_PAUSE       = Common::KEYCODE_PAUSE,
		KEY_CAPSLOCK    = Common::KEYCODE_CAPSLOCK,
		KEY_ESCAPE      = Common::KEYCODE_ESCAPE,
		KEY_SPACE       = Common::KEYCODE_SPACE,
		KEY_PAGEUP      = Common::KEYCODE_PAGEUP,
		KEY_PAGEDOWN    = Common::KEYCODE_PAGEDOWN,
		KEY_END         = Common::KEYCODE_END,
		KEY_HOME        = Common::KEYCODE_HOME,
		KEY_LEFT        = Common::KEYCODE_LEFT,
		KEY_UP          = Common::KEYCODE_UP,
		KEY_RIGHT       = Common::KEYCODE_RIGHT,
		KEY_DOWN        = Common::KEYCODE_DOWN,
		KEY_PRINTSCREEN = Common::KEYCODE_PRINT,
		KEY_INSERT      = Common::KEYCODE_INSERT,
		KEY_DELETE      = Common::KEYCODE_DELETE,
		KEY_0           = Common::KEYCODE_0,
		KEY_1           = Common::KEYCODE_1,
		KEY_2           = Common::KEYCODE_2,
		KEY_3           = Common::KEYCODE_3,
		KEY_4           = Common::KEYCODE_4,
		KEY_5           = Common::KEYCODE_5,
		KEY_6           = Common::KEYCODE_6,
		KEY_7           = Common::KEYCODE_7,
		KEY_8           = Common::KEYCODE_8,
		KEY_9           = Common::KEYCODE_9,
		KEY_A           = Common::KEYCODE_a,
		KEY_B           = Common::KEYCODE_b,
		KEY_C           = Common::KEYCODE_c,
		KEY_D           = Common::KEYCODE_d,
		KEY_E           = Common::KEYCODE_e,
		KEY_F           = Common::KEYCODE_f,
		KEY_G           = Common::KEYCODE_g,
		KEY_H           = Common::KEYCODE_h,
		KEY_I           = Common::KEYCODE_i,
		KEY_J           = Common::KEYCODE_j,
		KEY_K           = Common::KEYCODE_k,
		KEY_L           = Common::KEYCODE_l,
		KEY_M           = Common::KEYCODE_m,
		KEY_N           = Common::KEYCODE_n,
		KEY_O           = Common::KEYCODE_o,
		KEY_P           = Common::KEYCODE_p,
		KEY_Q           = Common::KEYCODE_q,
		KEY_R           = Common::KEYCODE_r,
		KEY_S           = Common::KEYCODE_s,
		KEY_T           = Common::KEYCODE_t,
		KEY_U           = Common::KEYCODE_u,
		KEY_V           = Common::KEYCODE_v,
		KEY_W           = Common::KEYCODE_w,
		KEY_X           = Common::KEYCODE_x,
		KEY_Y           = Common::KEYCODE_y,
		KEY_Z           = Common::KEYCODE_z,
		KEY_NUMPAD0     = Common::KEYCODE_KP0,
		KEY_NUMPAD1     = Common::KEYCODE_KP1,
		KEY_NUMPAD2     = Common::KEYCODE_KP2,
		KEY_NUMPAD3     = Common::KEYCODE_KP3,
		KEY_NUMPAD4     = Common::KEYCODE_KP4,
		KEY_NUMPAD5     = Common::KEYCODE_KP5,
		KEY_NUMPAD6     = Common::KEYCODE_KP6,
		KEY_NUMPAD7     = Common::KEYCODE_KP7,
		KEY_NUMPAD8     = Common::KEYCODE_KP8,
		KEY_NUMPAD9     = Common::KEYCODE_KP9,
		KEY_MULTIPLY    = Common::KEYCODE_KP_MULTIPLY,
		KEY_ADD         = Common::KEYCODE_KP_PLUS,
		KEY_SEPARATOR   = Common::KEYCODE_EQUALS,	// FIXME: This mapping is just a wild guess!!
		KEY_SUBTRACT    = Common::KEYCODE_KP_MINUS,
		KEY_DECIMAL     = Common::KEYCODE_KP_PERIOD,
		KEY_DIVIDE      = Common::KEYCODE_KP_DIVIDE,
		KEY_F1          = Common::KEYCODE_F1,
		KEY_F2          = Common::KEYCODE_F2,
		KEY_F3          = Common::KEYCODE_F3,
		KEY_F4          = Common::KEYCODE_F4,
		KEY_F5          = Common::KEYCODE_F5,
		KEY_F6          = Common::KEYCODE_F6,
		KEY_F7          = Common::KEYCODE_F7,
		KEY_F8          = Common::KEYCODE_F8,
		KEY_F9          = Common::KEYCODE_F9,
		KEY_F10         = Common::KEYCODE_F10,
		KEY_F11         = Common::KEYCODE_F11,
		KEY_F12         = Common::KEYCODE_F12,
		KEY_NUMLOCK     = Common::KEYCODE_NUMLOCK,
		KEY_SCROLL      = Common::KEYCODE_SCROLLOCK,
		KEY_LSHIFT      = Common::KEYCODE_LSHIFT,
		KEY_RSHIFT      = Common::KEYCODE_RSHIFT,
		KEY_LCONTROL    = Common::KEYCODE_LCTRL,
		KEY_RCONTROL    = Common::KEYCODE_RCTRL
	};

	// NOTE: These codes are registered in inputengine_script.cpp
	// If you add or remove entries of this enum, you must also adjust
	// the above file.
	enum KEY_COMMANDS {
		KEY_COMMAND_ENTER = 1,
		KEY_COMMAND_LEFT = 2,
		KEY_COMMAND_RIGHT = 3,
		KEY_COMMAND_HOME = 4,
		KEY_COMMAND_END = 5,
		KEY_COMMAND_BACKSPACE = 6,
		KEY_COMMAND_TAB = 7,
		KEY_COMMAND_INSERT = 8,
		KEY_COMMAND_DELETE = 9
	};

	/// --------------------------------------------------------------
	/// THESE METHODS MUST BE IMPLEMENTED BY THE INPUT ENGINE
	/// --------------------------------------------------------------

	/**
	 * Initializes the input engine
	 * @return          Returns a true on success, otherwise false.
	 */
	bool init();

	/**
	 * Performs a "tick" of the input engine.
	 *
	 * This method should be called once per frame. It can be used by implementations
	 * of the input engine that are not running in their own thread, or to perform
	 * additional administrative tasks that are needed.
	 */
	void update();

	/**
	 * Returns true if the left mouse button is pressed
	 */
	bool isLeftMouseDown();

	/**
	 * Returns true if the right mouse button is pressed.
	*/
	bool isRightMouseDown();

	/**
	 * Returns true if the left mouse button was pressed and released.
	 *
	 * The difference between this and IsLeftMouseDown() is that this only returns
	 * true when the left mouse button is released.
	*/
	bool wasLeftMouseDown();

	/**
	 * Returns true if the right mouse button was pressed and released.
	 *
	 * The difference between this and IsRightMouseDown() is that this only returns
	 * true when the right mouse button is released.
	*/
	bool wasRightMouseDown();

	/**
	 * Returns true if the left mouse button double click was done
	 */
	bool isLeftDoubleClick();

	/**
	 * Returns the X position of the cursor in pixels
	*/
	int getMouseX();

	/**
	 * Returns the Y position of the cursor in pixels
	 */
	int getMouseY();

	/**
	 * Returns true if a given key was pressed
	 * @param KeyCode       The key code to be checked
	 * @return              Returns true if the given key is done, otherwise false.
	 */
	bool isKeyDown(uint keyCode);

	/**
	 * Returns true if a certain key was pushed and released.
	 *
	 * The difference between IsKeyDown() is that this only returns true after the key
	 * has been released. This method facilitates the retrieval of keys, and reading
	 * strings that users type.
	 * @param KeyCode       The key code to be checked
	 */
	bool wasKeyDown(uint keyCode);

	typedef void (*CharacterCallback)(int command);

	/**
	 * Registers a callback function for keyboard input.
	 *
	 * The callback that is registered with this function will be called whenever an
	 * input key is pressed. A letter entry is different from the query using the
	 * methods isKeyDown() and wasKeyDown() in the sense that are treated instead
	 * of actual scan-coded letters. These were taken into account, among other things:
	 * the keyboard layout, the condition the Shift and Caps Lock keys and the repetition
	 * of longer holding the key.
	 * The input of strings by the user through use of callbacks should be implemented.
	*/
	void setCharacterCallback(CharacterCallback callback);

	typedef void (*CommandCallback)(int command);

	/**
	 * Registers a callback function for the input of commands that can have influence on the string input
	 *
	 * The callback that is registered with this function will be called whenever the input service
	 * has a key that affects the character string input. This could be the following keys:
	 * Enter, End, Left, Right, ...
	 * The input of strings by the user through the use of callbacks should be implemented.
	 */
	void setCommandCallback(CommandCallback callback);

	void reportCharacter(byte character);
	void reportCommand(KEY_COMMANDS command);

	bool persist(OutputPersistenceBlock &writer);
	bool unpersist(InputPersistenceBlock &reader);

private:
	bool registerScriptBindings();
	void unregisterScriptBindings();

private:
	void testForLeftDoubleClick();
	void alterKeyboardState(int keycode, byte newState);

	byte _keyboardState[2][512];
	bool _leftMouseState[2];
	bool _rightMouseState[2];
	uint _currentState;
	int _mouseX;
	int _mouseY;
	bool _leftMouseDown;
	bool _rightMouseDown;
	bool _leftDoubleClick;
	uint _doubleClickTime;
	int _doubleClickRectWidth;
	int _doubleClickRectHeight;
	uint _lastLeftClickTime;
	int _lastLeftClickMouseX;
	int _lastLeftClickMouseY;
	CommandCallback _commandCallback;
	CharacterCallback _characterCallback;
};

} // End of namespace Sword25

#endif
