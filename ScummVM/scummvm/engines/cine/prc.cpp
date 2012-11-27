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
#include "common/events.h"
#include "common/config-manager.h"
#include "common/system.h"	// for g_system->getEventManager()

#include "cine/cine.h"
#include "cine/various.h"


namespace Cine {

//char currentPrcName[20];

/**
 * @todo Is script size of 0 valid?
 * @todo Fix script dump code
 * @return Was the loading successful?
 */
bool loadPrc(const char *pPrcName) {
	byte i;
	uint16 numScripts;
	byte *scriptPtr, *dataPtr;

	assert(pPrcName);

	g_cine->_globalScripts.clear();
	g_cine->_scriptTable.clear();

	// This is copy protection. Used to hang the machine
	if (!scumm_stricmp(pPrcName, COPY_PROT_FAIL_PRC_NAME)) {
		Common::Event event;
		event.type = Common::EVENT_RTL;
		g_system->getEventManager()->pushEvent(event);
		return false;
	}

	checkDataDisk(-1);
	if ((g_cine->getGameType() == Cine::GType_FW) &&
		(!scumm_stricmp(pPrcName, BOOT_PRC_NAME) || !scumm_stricmp(pPrcName, "demo.prc"))) {
		scriptPtr = dataPtr = readFile(pPrcName, (g_cine->getFeatures() & GF_CRYPTED_BOOT_PRC) != 0);
	} else {
		scriptPtr = dataPtr = readBundleFile(findFileInBundle(pPrcName));
	}

	assert(scriptPtr);

	setMouseCursor(MOUSE_CURSOR_DISK);

	numScripts = READ_BE_UINT16(scriptPtr);
	scriptPtr += 2;
	assert(numScripts <= NUM_MAX_SCRIPT);

	for (i = 0; i < numScripts; i++) {
		RawScriptPtr tmp(new RawScript(READ_BE_UINT16(scriptPtr)));
		scriptPtr += 2;
		assert(tmp);
		g_cine->_scriptTable.push_back(tmp);
	}

	for (i = 0; i < numScripts; i++) {
		uint16 size = g_cine->_scriptTable[i]->_size;
		// TODO: delete the test?
		if (size) {
			g_cine->_scriptTable[i]->setData(*scriptInfo, scriptPtr);
			scriptPtr += size;
		}
	}

	free(dataPtr);

#ifdef DUMP_SCRIPTS

	{
		uint16 s;
		char buffer[256];

		for (s = 0; s < numScripts; s++) {
			if (g_cine->_scriptTable[s]->_size) {
				sprintf(buffer, "%s_%03d.txt", pPrcName, s);

				decompileScript((const byte *)g_cine->_scriptTable[s]->getString(0), g_cine->_scriptTable[s]->_size, s);
				dumpScript(buffer);
			}
		}
	}
#endif

	return true;
}

} // End of namespace Cine
