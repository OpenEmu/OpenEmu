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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/math/regionregistry.h"
#include "sword25/math/region.h"

namespace Common {
DECLARE_SINGLETON(Sword25::RegionRegistry);
}

namespace Sword25 {

bool RegionRegistry::persist(OutputPersistenceBlock &writer) {
	bool result = true;

	// write out the next handle
	writer.write(_nextHandle);

	// Number of regions to write
	writer.write(_handle2PtrMap.size());

	// Persist all the BS_Regions
	HANDLE2PTR_MAP::const_iterator iter = _handle2PtrMap.begin();
	while (iter != _handle2PtrMap.end()) {
		// Handle persistence
		writer.write(iter->_key);

		// Persist object
		result &= iter->_value->persist(writer);

		++iter;
	}

	return result;
}

bool RegionRegistry::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	// read in the next handle
	reader.read(_nextHandle);

	// Destroy all existing BS_Regions
//FIXME: This doesn't seem right - the value is being deleted but not the actual hash node itself?
	while (!_handle2PtrMap.empty())
		delete _handle2PtrMap.begin()->_value;

	// read in the number of BS_Regions
	uint regionCount;
	reader.read(regionCount);

	// Restore all the BS_Regions objects
	for (uint i = 0; i < regionCount; ++i)  {
		// Handle read
		uint handle;
		reader.read(handle);

		// BS_Region restore
		result &= Region::create(reader, handle) != 0;
	}

	return reader.isGood() && result;
}

} // End of namespace Sword25
