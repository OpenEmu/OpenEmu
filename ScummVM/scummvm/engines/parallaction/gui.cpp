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

#include "common/textconsole.h"

#include "parallaction/gui.h"

namespace Parallaction {

bool MenuInputHelper::run() {
	if (_newState == 0) {
		debugC(3, kDebugExec, "MenuInputHelper has set NULL state");
		return false;
	}

	if (_newState != _state) {
		debugC(3, kDebugExec, "MenuInputHelper changing state to '%s'", _newState->_name.c_str());

		_newState->enter();
		_state = _newState;
	}

	_newState = _state->run();

	return true;
}

MenuInputHelper::~MenuInputHelper() {
	StateMap::iterator b = _map.begin();
	for ( ; b != _map.end(); ++b) {
		delete b->_value;
	}
	_map.clear();
}


void Parallaction::runGuiFrame() {
	if (_input->_inputMode != Input::kInputModeMenu) {
		return;
	}

	if (!_menuHelper) {
		error("No menu helper defined");
	}

	bool res = _menuHelper->run();

	if (!res) {
		cleanupGui();
		_input->_inputMode = Input::kInputModeGame;
	}

}

void Parallaction::cleanupGui() {
	delete _menuHelper;
	_menuHelper = 0;
}

void Parallaction::setInternLanguage(uint id) {
	//TODO: assert id!

	_language = id;
	_disk->setLanguage(id);
}

uint Parallaction::getInternLanguage() {
	return _language;
}


} // namespace  Parallaction
