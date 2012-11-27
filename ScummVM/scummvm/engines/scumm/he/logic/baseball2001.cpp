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
 *     Backyard Baseball 2001
 */
class LogicHEbaseball2001 : public LogicHE {
public:
	LogicHEbaseball2001(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);
};

int LogicHEbaseball2001::versionID() {
	return 1;
}

int32 LogicHEbaseball2001::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
	case 3001:
		// Check network status
		break;

	default:
		LogicHE::dispatch(op, numArgs, args);
	}

	return res;
}

LogicHE *makeLogicHEbaseball2001(ScummEngine_v90he *vm) {
	return new LogicHEbaseball2001(vm);
}

} // End of namespace Scumm
