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

#include "cine/cine.h"
#include "cine/various.h"

namespace Cine {

/**
 * @todo Is script size of 0 valid?
 * @todo Fix script dump code
 */
void loadRel(char *pRelName) {
	uint16 numEntry;
	uint16 i;
	uint16 size, p1, p2, p3;
	byte *ptr, *dataPtr;

	checkDataDisk(-1);

	g_cine->_objectScripts.clear();
	g_cine->_relTable.clear();

	ptr = dataPtr = readBundleFile(findFileInBundle(pRelName));

	setMouseCursor(MOUSE_CURSOR_DISK);

	numEntry = READ_BE_UINT16(ptr); ptr += 2;

	for (i = 0; i < numEntry; i++) {
		size = READ_BE_UINT16(ptr); ptr += 2;
		p1 = READ_BE_UINT16(ptr); ptr += 2;
		p2 = READ_BE_UINT16(ptr); ptr += 2;
		p3 = READ_BE_UINT16(ptr); ptr += 2;
		RawObjectScriptPtr tmp(new RawObjectScript(size, p1, p2, p3));
		assert(tmp);
		g_cine->_relTable.push_back(tmp);
	}

	for (i = 0; i < numEntry; i++) {
		size = g_cine->_relTable[i]->_size;
		// TODO: delete the test?
		if (size) {
			g_cine->_relTable[i]->setData(*scriptInfo, ptr);
			ptr += size;
		}
	}

	free(dataPtr);

#ifdef DUMP_SCRIPTS

	{
		uint16 s;
		char buffer[256];

		for (s = 0; s < numEntry; s++) {
			if (g_cine->_relTable[s]->_size) {
				sprintf(buffer, "%s_%03d.txt", pRelName, s);

				decompileScript((const byte *)g_cine->_relTable[s]->getString(0), g_cine->_relTable[s]->_size, s);
				dumpScript(buffer);
			}
		}
	}
#endif
}

} // End of namespace Cine
