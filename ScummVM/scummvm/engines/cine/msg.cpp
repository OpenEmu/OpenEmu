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

#include "common/debug.h"
#include "common/endian.h"
#include "common/textconsole.h"

#include "cine/msg.h"
#include "cine/various.h"

namespace Cine {

void loadMsg(char *pMsgName) {
	uint32 sourceSize;

	checkDataDisk(-1);
	g_cine->_messageTable.clear();
	byte *dataPtr = readBundleFile(findFileInBundle(pMsgName), &sourceSize);

	setMouseCursor(MOUSE_CURSOR_DISK);

	uint count = READ_BE_UINT16(dataPtr);
	uint messageLenPos = 2;
	uint messageDataPos = messageLenPos + 2 * count;

	// Read in the messages
	for (uint i = 0; i < count; i++) {
		// Read message's length
		uint messageLen = READ_BE_UINT16(dataPtr + messageLenPos);
		messageLenPos += 2;

		// Store the read message.
		// This code works around input data that has empty strings residing outside the input
		// buffer (e.g. message indexes 58-254 in BATEAU.MSG in PROCS08 in Operation Stealth).
		if (messageDataPos < sourceSize) {
			g_cine->_messageTable.push_back((const char *)(dataPtr + messageDataPos));
		} else {
			if (messageLen > 0) { // Only warn about overflowing non-empty strings
				warning("loadMsg(%s): message (%d. / %d) is overflowing the input buffer. Replacing it with an empty string", pMsgName, i + 1, count);
			} else {
				debugC(5, kCineDebugPart, "loadMsg(%s): empty message (%d. / %d) resides outside input buffer", pMsgName, i + 1, count);
			}
			// Message resides outside the input buffer so we replace it with an empty string
			g_cine->_messageTable.push_back("");
		}
		// Jump to the next message
		messageDataPos += messageLen;
	}

	free(dataPtr);
}

} // End of namespace Cine
