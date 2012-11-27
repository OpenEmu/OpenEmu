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

#ifndef SWORD25_RESOURCEMANAGER_H
#define SWORD25_RESOURCEMANAGER_H

#include "common/list.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

#include "sword25/kernel/common.h"

namespace Sword25 {

//#define PRECACHE_RESOURCES

class ResourceService;
class Resource;
class Kernel;

class ResourceManager {
	friend class Kernel;

public:
	/**
	 * Returns a requested resource. If any error occurs, returns NULL
	 * @param FileName      Filename of resource
	 */
	Resource *requestResource(const Common::String &fileName);

#ifdef PRECACHE_RESOURCES
	/**
	 * Loads a resource into the cache
	 * @param FileName      The filename of the resource to be cached
	 * @param ForceReload   Indicates whether the file should be reloaded if it's already in the cache.
	 * This is useful for files that may have changed in the interim
	 */
	bool precacheResource(const Common::String &fileName, bool forceReload = false);
#endif

	/**
	 * Registers a RegisterResourceService. This method is the constructor of
	 * BS_ResourceService, and thus helps all resource services in the ResourceManager list
	 * @param pService      Which service
	 */
	bool registerResourceService(ResourceService *pService);

	/**
	 * Releases all resources that are not locked.
	 **/
	void emptyCache();

	/**
	 * Removes all the savegame thumbnails from the cache
	 **/
	void emptyThumbnailCache();

	/**
	 * Writes the names of all currently locked resources to the log file
	 */
	void dumpLockedResources();

private:
	/**
	 * Creates a new resource manager
	 * Only the BS_Kernel class can generate copies this class. Thus, the constructor is private
	 */
	ResourceManager(Kernel *pKernel) :
		_kernelPtr(pKernel)
	{}
	virtual ~ResourceManager();

	/**
	 * Moves a resource to the top of the resource list
	 * @param pResource     The resource
	 */
	void moveToFront(Resource *pResource);

	/**
	 * Loads a resource and updates the m_UsedMemory total
	 *
	 * The resource must not already be loaded
	 * @param FileName      The unique filename of the resource to be loaded
	 */
	Resource *loadResource(const Common::String &fileName);

	/**
	 * Returns the full path of a given resource filename.
	 * It will return an empty string if a path could not be created.
	*/
	Common::String getUniqueFileName(const Common::String &fileName) const;

	/**
	 * Deletes a resource, removes it from the lists, and updates m_UsedMemory
	 */
	Common::List<Resource *>::iterator deleteResource(Resource *pResource);

	/**
	 * Returns a pointer to a loaded resource. If any error occurs, NULL will be returned.
	 * @param UniqueFileName        The absolute path and filename
	 */
	Resource *getResource(const Common::String &uniqueFileName) const;

	/**
	 * Deletes resources as necessary until the specified memory limit is not being exceeded.
	 */
	void deleteResourcesIfNecessary();

	Kernel *_kernelPtr;
	Common::Array<ResourceService *> _resourceServices;
	Common::List<Resource *> _resources;
	typedef Common::HashMap<Common::String, Resource *> ResMap;
	ResMap _resourceHashMap;
};

} // End of namespace Sword25

#endif
