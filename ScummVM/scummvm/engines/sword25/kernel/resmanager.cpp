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

#include "sword25/sword25.h"	// for kDebugResource
#include "sword25/kernel/resmanager.h"
#include "sword25/kernel/resource.h"
#include "sword25/kernel/resservice.h"
#include "sword25/package/packagemanager.h"

namespace Sword25 {

// Sets the amount of resources that are simultaneously loaded.
// This needs to be a relatively high number, as all the animation
// frames in each scene are loaded as separate resources.
// Also, George's walk states are all loaded here (150 files)
#define SWORD25_RESOURCECACHE_MIN 400
// The maximum number of loaded resources. If more than these resources
// are loaded, the resource manager will start purging resources till it
// hits the minimum limit above
#define SWORD25_RESOURCECACHE_MAX 500

ResourceManager::~ResourceManager() {
	// Clear all unlocked resources
	emptyCache();

	// All remaining resources are not released, so print warnings and release
	Common::List<Resource *>::iterator iter = _resources.begin();
	for (; iter != _resources.end(); ++iter) {
		warning("Resource \"%s\" was not released.", (*iter)->getFileName().c_str());

		// Set the lock count to zero
		while ((*iter)->getLockCount() > 0) {
			(*iter)->release();
		};

		// Delete the resource
		delete(*iter);
	}
}

/**
 * Registers a RegisterResourceService. This method is the constructor of
 * BS_ResourceService, and thus helps all resource services in the ResourceManager list
 * @param pService      Which service
 */
bool ResourceManager::registerResourceService(ResourceService *pService) {
	if (!pService) {
		error("Can't register NULL resource service.");
		return false;
	}

	_resourceServices.push_back(pService);

	return true;
}

/**
 * Deletes resources as necessary until the specified memory limit is not being exceeded.
 */
void ResourceManager::deleteResourcesIfNecessary() {
	// If enough memory is available, or no resources are loaded, then the function can immediately end
	if (_resources.size() < SWORD25_RESOURCECACHE_MAX)
		return;

	// Keep deleting resources until the memory usage of the process falls below the set maximum limit.
	// The list is processed backwards in order to first release those resources that have been
	// not been accessed for the longest
	Common::List<Resource *>::iterator iter = _resources.end();
	do {
		--iter;

		// The resource may be released only if it isn't locked
		if ((*iter)->getLockCount() == 0)
			iter = deleteResource(*iter);
	} while (iter != _resources.begin() && _resources.size() >= SWORD25_RESOURCECACHE_MIN);

	// Are we still above the minimum? If yes, then start releasing locked resources
	// FIXME: This code shouldn't be needed at all, but it seems like there is a bug
	// in the resource lock code, and resources are not unlocked when changing rooms.
	// Only image/animation resources are unlocked forcibly, thus this shouldn't have
	// any impact on the game itself.
	if (_resources.size() <= SWORD25_RESOURCECACHE_MIN)
		return;

	iter = _resources.end();
	do {
		--iter;

		// Only unlock image/animation resources
		if ((*iter)->getFileName().hasSuffix(".swf") ||
			(*iter)->getFileName().hasSuffix(".png")) {

			warning("Forcibly unlocking %s", (*iter)->getFileName().c_str());

			// Forcibly unlock the resource
			while ((*iter)->getLockCount() > 0)
				(*iter)->release();

			iter = deleteResource(*iter);
		}
	} while (iter != _resources.begin() && _resources.size() >= SWORD25_RESOURCECACHE_MIN);
}

/**
 * Releases all resources that are not locked.
 */
void ResourceManager::emptyCache() {
	// Scan through the resource list
	Common::List<Resource *>::iterator iter = _resources.begin();
	while (iter != _resources.end()) {
		if ((*iter)->getLockCount() == 0) {
			// Delete the resource
			iter = deleteResource(*iter);
		} else
			++iter;
	}
}

void ResourceManager::emptyThumbnailCache() {
	// Scan through the resource list
	Common::List<Resource *>::iterator iter = _resources.begin();
	while (iter != _resources.end()) {
		if ((*iter)->getFileName().hasPrefix("/saves")) {
			// Unlock the thumbnail
			while ((*iter)->getLockCount() > 0)
				(*iter)->release();
			// Delete the thumbnail
			iter = deleteResource(*iter);
		} else
			++iter;
	}
}

/**
 * Returns a requested resource. If any error occurs, returns NULL
 * @param FileName      Filename of resource
 */
Resource *ResourceManager::requestResource(const Common::String &fileName) {
	// Get the absolute path to the file
	Common::String uniqueFileName = getUniqueFileName(fileName);
	if (uniqueFileName.empty())
		return NULL;

	// Determine whether the resource is already loaded
	// If the resource is found, it will be placed at the head of the resource list and returned
	Resource *pResource = getResource(uniqueFileName);
	if (!pResource)
		pResource = loadResource(uniqueFileName);
	if (pResource) {
		moveToFront(pResource);
		(pResource)->addReference();
		return pResource;
	}

	return NULL;
}

#ifdef PRECACHE_RESOURCES

/**
 * Loads a resource into the cache
 * @param FileName      The filename of the resource to be cached
 * @param ForceReload   Indicates whether the file should be reloaded if it's already in the cache.
 * This is useful for files that may have changed in the interim
 */
bool ResourceManager::precacheResource(const Common::String &fileName, bool forceReload) {
	// Get the absolute path to the file
	Common::String uniqueFileName = getUniqueFileName(fileName);
	if (uniqueFileName.empty())
		return false;

	Resource *resourcePtr = getResource(uniqueFileName);

	if (forceReload && resourcePtr) {
		if (resourcePtr->getLockCount()) {
			error("Could not force precaching of \"%s\". The resource is locked.", fileName.c_str());
			return false;
		} else {
			deleteResource(resourcePtr);
			resourcePtr = 0;
		}
	}

	if (!resourcePtr && loadResource(uniqueFileName) == NULL) {
		// This isn't fatal - e.g. it can happen when loading saved games
		debugC(kDebugResource, "Could not precache \"%s\",", fileName.c_str());
		return false;
	}

	return true;
}

#endif

/**
 * Moves a resource to the top of the resource list
 * @param pResource     The resource
 */
void ResourceManager::moveToFront(Resource *pResource) {
	// Erase the resource from it's current position
	_resources.erase(pResource->_iterator);
	// Re-add the resource at the front of the list
	_resources.push_front(pResource);
	// Reset the resource iterator to the repositioned item
	pResource->_iterator = _resources.begin();
}

/**
 * Loads a resource and updates the m_UsedMemory total
 *
 * The resource must not already be loaded
 * @param FileName      The unique filename of the resource to be loaded
 */
Resource *ResourceManager::loadResource(const Common::String &fileName) {
	// ResourceService finden, der die Resource laden kann.
	for (uint i = 0; i < _resourceServices.size(); ++i) {
		if (_resourceServices[i]->canLoadResource(fileName)) {
			// If more memory is desired, memory must be released
			deleteResourcesIfNecessary();

			// Load the resource
			Resource *pResource = _resourceServices[i]->loadResource(fileName);
			if (!pResource) {
				error("Responsible service could not load resource \"%s\".", fileName.c_str());
				return NULL;
			}

			// Add the resource to the front of the list
			_resources.push_front(pResource);
			pResource->_iterator = _resources.begin();

			// Also store the resource in the hash table for quick lookup
			_resourceHashMap[pResource->getFileName()] = pResource;

			return pResource;
		}
	}

	// This isn't fatal - e.g. it can happen when loading saved games
	debugC(kDebugResource, "Could not find a service that can load \"%s\".", fileName.c_str());
	return NULL;
}

/**
 * Returns the full path of a given resource filename.
 * It will return an empty string if a path could not be created.
*/
Common::String ResourceManager::getUniqueFileName(const Common::String &fileName) const {
	// Get a pointer to the package manager
	PackageManager *pPackage = (PackageManager *)_kernelPtr->getPackage();
	if (!pPackage) {
		error("Could not get package manager.");
		return Common::String();
	}

	// Absoluten Pfad der Datei bekommen und somit die Eindeutigkeit des Dateinamens sicherstellen
	Common::String uniquefileName = pPackage->getAbsolutePath(fileName);
	if (uniquefileName.empty())
		error("Could not create absolute file name for \"%s\".", fileName.c_str());

	return uniquefileName;
}

/**
 * Deletes a resource, removes it from the lists, and updates m_UsedMemory
 */
Common::List<Resource *>::iterator ResourceManager::deleteResource(Resource *pResource) {
	// Remove the resource from the hash table
	_resourceHashMap.erase(pResource->_fileName);

	// Delete the resource from the resource list
	Common::List<Resource *>::iterator result = _resources.erase(pResource->_iterator);

	// Delete the resource
	delete pResource;

	// Return the iterator
	return result;
}

/**
 * Returns a pointer to a loaded resource. If any error occurs, NULL will be returned.
 * @param UniquefileName        The absolute path and filename
 */
Resource *ResourceManager::getResource(const Common::String &uniquefileName) const {
	// Determine whether the resource is already loaded
	ResMap::iterator it = _resourceHashMap.find(uniquefileName);
	if (it != _resourceHashMap.end())
		return it->_value;

	// Resource was not found, i.e. has not yet been loaded.
	return NULL;
}

/**
 * Writes the names of all currently locked resources to the log file
 */
void ResourceManager::dumpLockedResources() {
	for (Common::List<Resource *>::iterator iter = _resources.begin(); iter != _resources.end(); ++iter) {
		if ((*iter)->getLockCount() > 0) {
			debugC(kDebugResource, "%s", (*iter)->getFileName().c_str());
		}
	}
}

} // End of namespace Sword25
