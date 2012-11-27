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

#ifndef SWORD25_RESOURCESERVICE_H
#define SWORD25_RESOURCESERVICE_H

#include "sword25/kernel/common.h"
#include "sword25/kernel/service.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/resmanager.h"

namespace Sword25 {

class Resource;

class ResourceService : public Service {
public:
	ResourceService(Kernel *pKernel) : Service(pKernel) {
		ResourceManager *pResource = pKernel->getResourceManager();
		pResource->registerResourceService(this);
	}

	virtual ~ResourceService() {}

	/**
	 * Loads a resource
	 * @return      Returns the resource if successful, otherwise NULL
	 */
	virtual Resource *loadResource(const Common::String &fileName) = 0;

	/**
	 * Checks whether the given name can be loaded by the resource service
	 * @param FileName  Dateiname
	 * @return          Returns true if the resource can be loaded.
	 */
	virtual bool canLoadResource(const Common::String &fileName) = 0;

};

} // End of namespace Sword25

#endif
