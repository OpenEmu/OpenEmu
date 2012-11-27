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

#include "agi/agi.h"
#include "agi/opcodes.h"

namespace Agi {

//
// Currently, there is no known difference between v3.002.098 -> v3.002.149
// So version emulated;
//
// 0x0086,
// 0x0149
//

/**
 *
 */
int AgiEngine::setupV2Game(int ver) {
	int ec = errOK;

	// Should this go above the previous lines, so we can force emulation versions
	// even for AGDS games? -- dsymonds
	if (getFeatures() & GF_AGDS)
		setVersion(ver = 0x2440);	// ALL AGDS games built for 2.440

	debug(0, "Setting up for version 0x%04X", ver);

	// 'quit' takes 0 args for 2.089
	if (ver == 0x2089)
//		logicNamesCmd[0x86].numArgs = 0;
		logicNamesCmd[0x86].args = "";

	// 'print.at' and 'print.at.v' take 3 args before 2.272
	// This is documented in the specs as only < 2.440, but it seems
	// that KQ3 (2.272) needs a 'print.at' taking 4 args.
	if (ver < 0x2272) {
//		logicNamesCmd[0x97].numArgs = 3;
//		logicNamesCmd[0x98].numArgs = 3;
		logicNamesCmd[0x97].args = "vvv";
		logicNamesCmd[0x98].args = "vvv";
	}

	return ec;
}

/**
 *
 */
int AgiEngine::setupV3Game(int ver) {
	int ec = errOK;

	debug(0, "Setting up for version 0x%04X", ver);

	// 'unknown176' takes 1 arg for 3.002.086, not 0 args.
	// 'unknown173' also takes 1 arg for 3.002.068, not 0 args.
	// Is this actually used anywhere? -- dsymonds
	if (ver == 0x3086) {
//		logicNamesCmd[0xb0].numArgs = 1;
//		logicNamesCmd[0xad].numArgs = 1;
		logicNamesCmd[0xb0].args = "n";
		logicNamesCmd[0xad].args = "n";
	}

	// FIXME: Apply this fix to other games also that use 2 arguments for command 182.
	// 'adj.ego.move.to.x.y' (i.e. command 182) takes 2 arguments for at least the
	// Amiga Gold Rush! (v2.05 1989-03-09) using Amiga AGI 2.316. Amiga's Gold Rush
	// has been set to use AGI 3.149 in ScummVM so that's why this initialization is
	// here and not in setupV2Game.
	if (getGameID() == GID_GOLDRUSH && getPlatform() == Common::kPlatformAmiga)
//		logicNamesCmd[182].numArgs = 2;
		logicNamesCmd[182].args = "vv";

	return ec;
}

} // End of namespace Agi
