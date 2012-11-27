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

#ifndef CINE_SAVELOAD_H
#define CINE_SAVELOAD_H

#include "common/endian.h"

namespace Cine {

/**
 * Cine engine's save game formats.
 * Enumeration entries (Excluding the one used as an error)
 * are sorted according to age (i.e. top one is oldest, last one newest etc).
 *
 * ANIMSIZE_UNKNOWN:
 * - Animation data entry size is unknown (Used as an error).
 *
 * ANIMSIZE_23:
 * - Animation data entry size is 23 bytes.
 * - Used at least by 0.11.0 and 0.11.1 releases of ScummVM.
 * - Introduced in revision 21772, stopped using in revision 31444.
 *
 * ANIMSIZE_30_PTRS_BROKEN:
 * - Animation data entry size is 30 bytes.
 * - Data and mask pointers in the saved structs are always NULL.
 * - Introduced in revision 31453, stopped using in revision 32073.
 *
 * ANIMSIZE_30_PTRS_INTACT:
 * - Animation data entry size is 30 bytes.
 * - Data and mask pointers in the saved structs are intact,
 *   so you can test them for equality or inequality with NULL
 *   but don't try using them for anything else, it won't work.
 * - Introduced in revision 31444, got broken in revision 31453,
 *   got fixed in revision 32073 and used after that.
 *
 * TEMP_OS_FORMAT:
 * - Temporary Operation Stealth savegame format.
 * - NOT backward compatible and NOT to be supported in the future.
 *   This format should ONLY be used during development and abandoned
 *   later in favor of a better format!
 */
enum CineSaveGameFormat {
	ANIMSIZE_UNKNOWN,
	ANIMSIZE_23,
	ANIMSIZE_30_PTRS_BROKEN,
	ANIMSIZE_30_PTRS_INTACT,
	TEMP_OS_FORMAT
};

/** Identifier for the temporary Operation Stealth savegame format. */
static const uint32 TEMP_OS_FORMAT_ID = MKTAG('T', 'E', 'M', 'P');

/** The current version number of Operation Stealth's savegame format. */
static const uint32 CURRENT_OS_SAVE_VER = 1;

/** Chunk header used by the temporary Operation Stealth savegame format. */
struct ChunkHeader {
	uint32 id;      ///< Identifier (e.g. MKTAG('T','E','M','P'))
	uint32 version; ///< Version number
	uint32 size;    ///< Size of the chunk after this header in bytes
};


} // End of namespace Cine

#endif
