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

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/demos/batplayer.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/draw.h"
#include "gob/inter.h"
#include "gob/videoplayer.h"

namespace Gob {

BATPlayer::BATPlayer(GobEngine *vm) : DemoPlayer(vm) {
}

BATPlayer::~BATPlayer() {
}

bool BATPlayer::playStream(Common::SeekableReadStream &bat) {
	// Iterate over all lines
	while (!bat.err() && !bat.eos()) {
		Common::String line = bat.readLine();

		// Interpret
		if (lineStartsWith(line, "slide ")) {
			playVideo(line.c_str() + 6);
			clearScreen();
		}

		// Mind user input
		_vm->_util->processInput();
		if (_vm->shouldQuit())
			return true;
	}

	if (bat.err())
		return false;

	return true;
}

} // End of namespace Gob
