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

#ifndef ENGINES_OBSOLETE_H
#define ENGINES_OBSOLETE_H

#include "engines/game.h"

namespace Engines {

/**
 * Structure for autoupgrading targets using an obsolete gameid
 * to the correct new gameid.
 */
struct ObsoleteGameID {

	/** Name of the obsolete gameid. */
	const char *from;

	/** Name of the corresponding new gameid. */
	const char *to;

	/**
	 * If platform is set to a value different from Common::kPlatformUnknown,
	 * then upgradeTargetIfNecessary() will use this value to set the platform
	 * attribute of any target it updates using this ObsoleteGameID record.
	 * This is useful when the old gameid encoded the target platform (e.g.
	 * "zakTowns" for FM-TOWNS) while the new gameid does not (e.g. "zak").
	 */
	Common::Platform platform;
};

/**
 * Check if the currently active game target has an obsolete gameid;
 * if so, replace it by the correct new gameid.
 * This function is typically invoked by a MetaEngine::createInstance
 * implementation.
 */
void upgradeTargetIfNecessary(const ObsoleteGameID *obsoleteList);


/**
 * Scan through the given list of plain game descriptors specified and search
 * for 'gameid' in there. If a match is found, returns a GameDescriptor
 * with gameid and description set.
 *
 * Optionally can take a list of obsolete game ids into account in order
 * to support obsolete gameids.
 */
GameDescriptor findGameID(
	const char *gameid,
	const PlainGameDescriptor *gameids,
	const ObsoleteGameID *obsoleteList = 0
	);


} // End of namespace Engines

#endif
