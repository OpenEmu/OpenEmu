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

#include "gui/debugger.h"

#include "gob/gob.h"
#include "gob/cheater.h"
#include "gob/inter.h"

#include "gob/minigames/geisha/diving.h"
#include "gob/minigames/geisha/penetration.h"

namespace Gob {

Cheater_Geisha::Cheater_Geisha(GobEngine *vm, Geisha::Diving *diving, Geisha::Penetration *penetration) :
	Cheater(vm), _diving(diving), _penetration(penetration) {

}

Cheater_Geisha::~Cheater_Geisha() {
}

bool Cheater_Geisha::cheat(GUI::Debugger &console) {
	// A cheat to get around the Diving minigame
	if (_diving->isPlaying()) {
		_diving->cheatWin();
		return false;
	}

	// A cheat to get around the Penetration minigame
	if (_penetration->isPlaying()) {
		_penetration->cheatWin();
		return false;
	}

	// A cheat to get around the mastermind puzzle
	if (_vm->isCurrentTot("hard.tot") && _vm->_inter->_variables) {
		uint32 digit1 = READ_VARO_UINT32(0x768);
		uint32 digit2 = READ_VARO_UINT32(0x76C);
		uint32 digit3 = READ_VARO_UINT32(0x770);
		uint32 digit4 = READ_VARO_UINT32(0x774);
		uint32 digit5 = READ_VARO_UINT32(0x778);

		if (digit1 && digit2 && digit3 && digit4 && digit5)
			console.DebugPrintf("Mastermind solution: %d %d %d %d %d\n",
			                    digit1, digit2, digit3, digit4, digit5);

		return true;
	}

	return true;
}

} // End of namespace Gob
