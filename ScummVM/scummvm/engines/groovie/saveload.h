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

#ifndef GROOVIE_SAVELOAD_H
#define GROOVIE_SAVELOAD_H

#include "common/savefile.h"
#include "engines/game.h"
#include "engines/savestate.h"

namespace Groovie {

class SaveLoad {
public:
	// Validating slot numbers
	static int getMaximumSlot();
	static bool isSlotValid(int slot);

	// Getting information
	static Common::String getSlotSaveName(const Common::String &target, int slot);
	static SaveStateList listValidSaves(const Common::String &target);

	// Opening savefiles
	static Common::InSaveFile *openForLoading(const Common::String &target, int slot, SaveStateDescriptor *descriptor = NULL);
	static Common::OutSaveFile *openForSaving(const Common::String &target, int slot);
};

} // End of Groovie namespace

#endif // GROOVIE_SAVELOAD_H
