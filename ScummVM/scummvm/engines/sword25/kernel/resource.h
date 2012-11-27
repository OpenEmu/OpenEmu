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

#ifndef SWORD25_RESOURCE_H
#define SWORD25_RESOURCE_H

#include "common/list.h"
#include "common/str.h"
#include "sword25/kernel/common.h"

namespace Sword25 {

class Kernel;
class ResourceManager;

class Resource {
	friend class ResourceManager;

public:
	enum RESOURCE_TYPES {
		TYPE_UNKNOWN,
		TYPE_BITMAP,
		TYPE_ANIMATION,
		TYPE_SOUND,
		TYPE_FONT
	};

	Resource(const Common::String &uniqueFileName, RESOURCE_TYPES type);

	/**
	 * Prevents the resource from being released.
	 * @remarks             This method allows a resource to be locked multiple times.
	 **/
	void addReference() {
		++_refCount;
	}

	/**
	 * Cancels a previous lock
	 * @remarks             The resource can still be released more times than it was 'locked', although it is
	 * not recommended.
	 **/
	void release();

	/**
	 * Returns the current lock count for the resource
	 * @return              The current lock count
	 **/
	int getLockCount() const {
		return _refCount;
	}

	/**
	 * Returns the absolute path of the given resource
	 */
	const Common::String &getFileName() const {
		return _fileName;
	}

	/**
	 * Returns a resource's type
	 */
	uint getType() const {
		return _type;
	}

protected:
	virtual ~Resource() {}

private:
	Common::String _fileName;          ///< The absolute filename
	uint _refCount;          ///< The number of locks
	uint _type;              ///< The type of the resource
	Common::List<Resource *>::iterator _iterator;        ///< Points to the resource position in the LRU list
};

} // End of namespace Sword25

#endif
