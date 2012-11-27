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

#ifdef ENABLE_HE

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

namespace Scumm {

LogicHE::LogicHE(ScummEngine_v90he *vm) : _vm(vm) {
}

LogicHE::~LogicHE() {
}

void LogicHE::writeScummVar(int var, int32 value) {
	_vm->writeVar(var, value);
}

int LogicHE::versionID() {
	return 1;
}

int LogicHE::getFromArray(int arg0, int idx2, int idx1) {
	_vm->VAR(_vm->VAR_U32_ARRAY_UNK) = arg0;
	return _vm->readArray(116, idx2, idx1);
}

void LogicHE::putInArray(int arg0, int idx2, int idx1, int val) {
	_vm->VAR(_vm->VAR_U32_ARRAY_UNK) = arg0;
	_vm->writeArray(116, idx2, idx1, val);
}

int32 LogicHE::dispatch(int op, int numArgs, int32 *args) {
#if 1
	Common::String str;

	str = Common::String::format("LogicHE::dispatch(%d, %d, [", op, numArgs);
	if (numArgs > 0)
		str += Common::String::format("%d", args[0]);
	for (int i = 1; i < numArgs; i++) {
		str += Common::String::format(", %d", args[i]);
	}
	str += "])";

	debug(0, "%s", str.c_str());
#else
	// Used for parallel trace utility
	for (int i = 0; i < numArgs; i++)
		debug(0, "args[%d] = %d;", i, args[i]);

	debug(0, "dispatch(%d, %d, args);", op, numArgs);

#endif

	return 1;
}

LogicHE *LogicHE::makeLogicHE(ScummEngine_v90he *vm) {
	switch (vm->_game.id) {
	case GID_PUTTRACE:
		return makeLogicHErace(vm);

	case GID_FUNSHOP:
		return makeLogicHEfunshop(vm);

	case GID_FOOTBALL:
		return makeLogicHEfootball(vm);

	case GID_FOOTBALL2002:
		return makeLogicHEfootball2002(vm);

	case GID_SOCCER:
	case GID_SOCCERMLS:
	case GID_SOCCER2004:
		return makeLogicHEsoccer(vm);

	case GID_BASEBALL2001:
		return makeLogicHEbaseball2001(vm);

	case GID_BASKETBALL:
		return makeLogicHEbasketball(vm);

	case GID_MOONBASE:
		return makeLogicHEmoonbase(vm);

	default:
		return new LogicHE(vm);
	}
}

} // End of namespace Scumm

#endif // ENABLE_HE
