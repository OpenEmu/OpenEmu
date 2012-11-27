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

#include "scumm/scumm_v3.h"
#include "scumm/actor.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v3, x)

void ScummEngine_v3::setupOpcodes() {
	ScummEngine_v4::setupOpcodes();

	if (!(_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine)) {
		OPCODE(0x30, o3_setBoxFlags);
		OPCODE(0xb0, o3_setBoxFlags);
	}

	OPCODE(0x3b, o3_waitForActor);
	OPCODE(0xbb, o3_waitForActor);

	OPCODE(0x4c, o3_waitForSentence);
}

void ScummEngine_v3::o3_setBoxFlags() {
	int a, b;

	a = getVarOrDirectByte(PARAM_1);
	b = fetchScriptByte();
	setBoxFlags(a, b);
}

void ScummEngine_v3::o3_waitForActor() {
	// This opcode was a NOP in LOOM. Also, we cannot directly use
	// o2_waitForActor because there the _scriptPointer is different (it
	// assumes that getVar reads only a single byte, which is correct
	// for v2 but not for v3). Of course we could copy the code here to
	// o2_waitForActor and then merge the two, but right now I am
	// keeping this as it is.
	if (_game.id == GID_INDY3) {
		const byte *oldaddr = _scriptPointer - 1;
		Actor *a = derefActor(getVarOrDirectByte(PARAM_1), "o3_waitForActor");
		if (a->_moving) {
			_scriptPointer = oldaddr;
			o5_breakHere();
		}
	}
}

void ScummEngine_v3::o3_waitForSentence() {
	// FIXME/TODO: Can we merge this with o2_waitForSentence? I think
	// the code here is actually incorrect, and the code in
	// o2_waitForSentence correct, but somebody should check the
	// disassembly for Indy and Loom.
	if (_sentenceNum) {
		if (_sentence[_sentenceNum - 1].freezeCount && !isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
			return;
	} else if (!isScriptInUse(VAR(VAR_SENTENCE_SCRIPT)))
		return;

	_scriptPointer--;
	o5_breakHere();
}



} // End of namespace Scumm
