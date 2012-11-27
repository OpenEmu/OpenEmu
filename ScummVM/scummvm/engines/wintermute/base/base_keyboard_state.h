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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_KEYBOARD_STATE_H
#define WINTERMUTE_BASE_KEYBOARD_STATE_H


#include "engines/wintermute/base/base.h"
#include "engines/wintermute/base/base_scriptable.h"
#include "common/keyboard.h"
#include "common/events.h"

namespace Wintermute {

class BaseKeyboardState : public BaseScriptable {
public:
	uint32 _currentKeyData;
	uint32 _currentCharCode;
	bool _currentPrintable;

	bool _currentShift;
	bool _currentAlt;
	bool _currentControl;

	DECLARE_PERSISTENT(BaseKeyboardState, BaseScriptable)
	BaseKeyboardState(BaseGame *inGame);
	virtual ~BaseKeyboardState();
	bool readKey(Common::Event *event);

	void handleKeyPress(Common::Event *event);
	void handleKeyRelease(Common::Event *event);
	static bool isShiftDown();
	static bool isControlDown();
	static bool isAltDown();

	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name);
	virtual bool scSetProperty(const char *name, ScValue *value);
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	virtual const char *scToString();

private:
	uint8 *_keyStates;
	uint32 keyCodeToVKey(Common::Event *event);
	Common::KeyCode vKeyToKeyCode(uint32 vkey); //TODO, reimplement using ScummVM-backend
};

} // end of namespace Wintermute

#endif
