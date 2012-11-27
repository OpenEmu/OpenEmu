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

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

namespace Scumm {

/**
 * Logic code for:
 *     Moonbase Commander
 */
class LogicHEmoonbase : public LogicHE {
public:
	LogicHEmoonbase(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
};

int LogicHEmoonbase::versionID() {
	if (_vm->_game.features & GF_DEMO)
		return -100;
	else
		return 100;
}

LogicHE *makeLogicHEmoonbase(ScummEngine_v90he *vm) {
	return new LogicHEmoonbase(vm);
}

} // End of namespace Scumm
