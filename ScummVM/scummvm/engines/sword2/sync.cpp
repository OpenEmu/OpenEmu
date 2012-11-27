/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/textconsole.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"

namespace Sword2 {

/**
 * Clear any syncs registered for this id. Call this just after the id has been
 * processed. Theoretically there could be more than one sync waiting for us,
 * so clear the lot.
 */

void Logic::clearSyncs(uint32 id) {
	for (int i = 0; i < ARRAYSIZE(_syncList); i++) {
		if (_syncList[i].id == id) {
			debug(5, "removing sync %d for %d", i, id);
			_syncList[i].id = 0;
		}
	}
}

void Logic::sendSync(uint32 id, uint32 sync) {
	for (int i = 0; i < ARRAYSIZE(_syncList); i++) {
		if (_syncList[i].id == 0) {
			debug(5, "%d sends sync %d to %d", readVar(ID), sync, id);
			_syncList[i].id = id;
			_syncList[i].sync = sync;
			return;
		}
	}

	// The original code didn't even check for this condition, so maybe
	// it should be a fatal error?

	warning("No free sync slot");
}

/**
 * Check for a sync waiting for this character. Called from fnAnim() to see if
 * animation is to be finished. Returns an index into _syncList[], or -1.
 */

int Logic::getSync() {
	uint32 id = readVar(ID);

	for (int i = 0; i < ARRAYSIZE(_syncList); i++) {
		if (_syncList[i].id == id)
			return i;
	}

	return -1;
}

} // End of namespace Sword2
