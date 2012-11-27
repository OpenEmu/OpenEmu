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
 * A (some would say very) small collection of utility functions.
 */

#include "tinsel/dw.h"
#include "tinsel/film.h"
#include "tinsel/handle.h"
#include "tinsel/multiobj.h"
#include "tinsel/scn.h"
#include "tinsel/tinsel.h"	// for _vm

namespace Tinsel {

/**
 * Given a scene handle and a chunk id, gets the scene in RAM and
 * locates the requested chunk.
 * @param handle			Scene handle
 * @param chunk				Chunk Id
 */
byte *FindChunk(SCNHANDLE handle, uint32 chunk) {
	byte *bptr = LockMem(handle);
	uint32 *lptr = (uint32 *)bptr;
	uint32 add;

	// Initial adjustmnet for Tinsel 1 chunk types
	if ((TinselVersion != TINSEL_V2) && (chunk >= CHUNK_SCENE) &&
		(chunk != CHUNK_MBSTRING))
		--chunk;

	// V0 chunk types can be found by substracting 2 from the
	// chunk type. Note that CHUNK_STRING and CHUNK_BITMAP are
	// the same in V0 and V1
	if (TinselVersion == TINSEL_V0 &&
		chunk != CHUNK_STRING && chunk != CHUNK_BITMAP)
		chunk -= 0x2L;

	while (1) {
		if (READ_32(lptr) == chunk)
			return (byte *)(lptr + 2);

		++lptr;
		add = READ_32(lptr);

		if (!add)
			// End of file reached
			return NULL;

		// Move to next chunk
		lptr = (uint32 *)(bptr + add);
	}
}

} // End of namespace Tinsel
