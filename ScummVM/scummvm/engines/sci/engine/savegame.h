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

#ifndef SCI_ENGINE_SAVEGAME_H
#define SCI_ENGINE_SAVEGAME_H

#include "common/scummsys.h"
#include "common/str.h"

#include "sci/sci.h"

namespace Sci {

struct EngineState;

/*
 * Savegame format history:
 *
 * Version - new/changed feature
 * =============================
 *      30 - synonyms
 *      29 - system strings
 *      28 - heap
 *      27 - script created windows
 *      26 - play time
 *      25 - palette intensity
 *      24 - palvary
 *      23 - script buffer and heap size
 *      22 - game signature
 *      21 - script local variables
 *      20 - exports/synonyms
 *      19 - exportsAreWide
 *      18 - SCI32 arrays/strings
 *      17 - sound
 *
 */

enum {
	CURRENT_SAVEGAME_VERSION = 30,
	MINIMUM_SAVEGAME_VERSION = 14
};

// Savegame metadata
struct SavegameMetadata {
	Common::String name;
	int version;
	Common::String gameVersion;
	int saveDate;
	int saveTime;
	uint32 playTime;
	uint16 gameObjectOffset;
	uint16 script0Size;
};


/**
 * Saves a game state to the hard disk in a portable way.
 * @param s			The state to save
 * @param save		The stream to save to
 * @param savename	The description of the savegame
 * @return 0 on success, 1 otherwise
 */
bool gamestate_save(EngineState *s, Common::WriteStream *save, const Common::String &savename, const Common::String &version);

/**
 * Restores a game state from a directory.
 * @param s			An older state from the same game
 * @param dirname	The subdirectory to restore from
 */
void gamestate_restore(EngineState *s, Common::SeekableReadStream *save);

/**
 * Read the header from a savegame.
 */
bool get_savegame_metadata(Common::SeekableReadStream* stream, SavegameMetadata* meta);


} // End of namespace Sci

#endif // SCI_ENGINE_SAVEGAME_H
