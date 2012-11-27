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
#include "gob/scenery.h"
#include "gob/util.h"
#include "gob/sound/sound.h"

namespace Gob {

Scenery_v1::Scenery_v1(GobEngine *vm) : Scenery(vm) {
}

int16 Scenery_v1::loadAnim(char search) {
	if (_vm->_sound->cdIsPlaying()) {
		while (_vm->_sound->cdGetTrackPos() != -1)
		    _vm->_util->longDelay(50);

		_vm->_sound->cdStop();
	}

	return Scenery::loadAnim(search);
}

} // End of namespace Gob
