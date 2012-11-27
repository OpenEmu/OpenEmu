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

#include "sword25/kernel/resource.h"
#include "sword25/kernel/kernel.h"
#include "sword25/package/packagemanager.h"

namespace Sword25 {

Resource::Resource(const Common::String &fileName, RESOURCE_TYPES type) :
	_type(type),
	_refCount(0) {
	PackageManager *pPM = Kernel::getInstance()->getPackage();
	assert(pPM);

	_fileName = pPM->getAbsolutePath(fileName);
}

void Resource::release() {
	if (_refCount) {
		--_refCount;
	} else
		warning("Released unlocked resource \"%s\".", _fileName.c_str());
}

} // End of namespace Sword25
