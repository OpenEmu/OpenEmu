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
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "backends/keymapper/action.h"

#ifdef ENABLE_KEYMAPPER

#include "backends/keymapper/keymap.h"

namespace Common {

Action::Action(Keymap *boss, const char *i,	String des)
	: _boss(boss), description(des), _hwInput(0) {
	assert(i);
	assert(_boss);

	Common::strlcpy(id, i, ACTION_ID_SIZE);

	_boss->addAction(this);
}

void Action::mapInput(const HardwareInput *input) {
	if (_hwInput)
		_boss->unregisterMapping(this);

	_hwInput = input;

	if (_hwInput)
		_boss->registerMapping(this, _hwInput);
}

const HardwareInput *Action::getMappedInput() const {
	return _hwInput;
}

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER
