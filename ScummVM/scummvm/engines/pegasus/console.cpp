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

#include "pegasus/console.h"
#include "pegasus/interface.h"
#include "pegasus/pegasus.h"
#include "pegasus/neighborhood/neighborhood.h"

namespace Pegasus {

PegasusConsole::PegasusConsole(PegasusEngine *vm) : GUI::Debugger(), _vm(vm) {
	DCmd_Register("die", WRAP_METHOD(PegasusConsole, Cmd_Die));

	// These functions are non-demo specific
	if (!_vm->isDemo())
		DCmd_Register("jump", WRAP_METHOD(PegasusConsole, Cmd_Jump));
}

PegasusConsole::~PegasusConsole() {
}

bool PegasusConsole::Cmd_Die(int argc, const char **argv) {
	if (argc == 1) {
		DebugPrintf("Usage: die <death reason>\n");
		return true;
	}

	int reason = atoi(argv[1]);

	bool invalidReason = (reason == 0 || reason > kPlayerWonGame);

	if (!invalidReason && _vm->isDemo())
		invalidReason = (reason != kDeathFallOffCliff) && (reason != kDeathEatenByDinosaur) &&
				(reason != kDeathStranded) && (reason != kPlayerWonGame);


	if (invalidReason) {
		DebugPrintf("Invalid death reason %d\n", reason);
		return true;
	}

	_vm->die(atoi(argv[1]));
	return false;
}

bool PegasusConsole::Cmd_Jump(int argc, const char **argv) {
	if (!g_interface) {
		// TODO
		DebugPrintf("Cannot jump without interface set up\n");
		return true;
	}

	// TODO: Default room/direction for each neighborhood

	if (argc < 4) {
		DebugPrintf("Usage: jump <neighborhood> <room> <direction>\n");
		return true;
	}

	NeighborhoodID neighborhood = (NeighborhoodID)atoi(argv[1]);
	RoomID room = (RoomID)atoi(argv[2]);
	DirectionConstant direction = (DirectionConstant)atoi(argv[3]);

	if ((neighborhood < kCaldoriaID || neighborhood > kNoradDeltaID || neighborhood == kFinalTSAID) &&
			neighborhood != kNoradSubChaseID) {
		DebugPrintf("Invalid neighborhood %d", neighborhood);
		return true;
	}

	// No real way to check room validity at this point

	if (direction > kWest) {
		DebugPrintf("Invalid direction %d", direction);
		return true;
	}

	// Here we go!
	// TODO: Can't clear menu since the engine is paused
	_vm->jumpToNewEnvironment(neighborhood, room, direction);
	return false;
}

} // End of namespace Pegasus
