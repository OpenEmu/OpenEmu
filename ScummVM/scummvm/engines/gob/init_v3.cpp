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
#include "gob/init.h"
#include "gob/game.h"
#include "gob/global.h"

namespace Gob {

Init_v3::Init_v3(GobEngine *vm) : Init_v2(vm) {
}

Init_v3::~Init_v3() {
}

void Init_v3::updateConfig() {
// In the CD and Windows version of Goblins3, some texts are flagged
// 'subtitles' incorrectly and therefore should be displayed in all cases.
// The first obvious example is just after Death level.
	if ((_vm->getGameType() == kGameTypeGob3) &&
			(_vm->isCD() || (_vm->getPlatform() == Common::kPlatformWindows)))
		_vm->_global->_doSubtitles = true;
}

void Init_v3::initVideo() {
	Init_v2::initVideo();

	_vm->_game->_noScroll = false;
}

} // End of namespace Gob
