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

#ifndef DRACI_SAVELOAD_H
#define DRACI_SAVELOAD_H

#include "common/savefile.h"
#include "common/scummsys.h"
#include "graphics/surface.h"

namespace Draci {

#define DRACI_SAVEGAME_VERSION 1

struct DraciSavegameHeader {
	uint8 version;
	Common::String saveName;
	uint32 date;
	uint16 time;
	uint32 playtime;
	Graphics::Surface *thumbnail;
};

class DraciEngine;

bool readSavegameHeader(Common::InSaveFile *in, DraciSavegameHeader &header);
void writeSavegameHeader(Common::OutSaveFile *out, const DraciSavegameHeader &header);
Common::Error saveSavegameData(int saveGameIdx, const Common::String &saveName, DraciEngine &vm);
Common::Error loadSavegameData(int saveGameIdx, DraciEngine *vm);

} // End of namespace Draci

#endif
