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

Init_Fascination::Init_Fascination(GobEngine *vm) : Init_v2(vm) {
}

Init_Fascination::~Init_Fascination() {
}

void Init_Fascination::updateConfig() {
// In Fascination, some empty texts are present and used to clean up the text area.
// Using _doSubtitles does the trick.
// The first obvious example is in the hotel hall: 'Use ...' is displayed at
// the same place than the character dialogs.
	_vm->_global->_doSubtitles = true;
}

void Init_Fascination::initGame() {
// HACK - Suppress
// the PC Speaker, as the script checks in the intro for it's presence
// to play or not some noices.
	_vm->_global->_soundFlags = MIDI_FLAG | BLASTER_FLAG | ADLIB_FLAG;

	Init::initGame();
}

} // End of namespace Gob
