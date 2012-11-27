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

#ifndef SWORD25_OBJECTREGISTRY_H
#define SWORD25_OBJECTREGISTRY_H

#include "common/func.h"
#include "common/hashmap.h"
#include "common/textconsole.h"
#include "sword25/kernel/common.h"

namespace Sword25 {

template<typename T>
class ObjectRegistry {
public:
	ObjectRegistry() : _nextHandle(1) {}
	virtual ~ObjectRegistry() {}

	uint registerObject(T *objectPtr) {
		// Null-Pointer können nicht registriert werden.
		if (objectPtr == 0) {
			error("Cannot register a null pointer.");
			return 0;
		}

		// Falls das Objekt bereits registriert wurde, wird eine Warnung ausgeben und das Handle zurückgeben.
		uint handle = findHandleByPtr(objectPtr);
		if (handle != 0) {
			warning("Tried to register a object that was already registered.");
			return handle;
		}
		// Ansonsten wird das Objekt in beide Maps eingetragen und das neue Handle zurückgeben.
		else {
			_handle2PtrMap[_nextHandle] = objectPtr;
			_ptr2HandleMap[objectPtr] = _nextHandle;

			return _nextHandle++;
		}
	}

	uint registerObject(T *objectPtr, uint handle) {
		// Null-Pointer und Null-Handle können nicht registriert werden.
		if (objectPtr == 0 || handle == 0) {
			error("Cannot register a null pointer or a null handle.");
			return 0;
		}

		// Falls das Objekt bereits registriert wurde, wird ein Fehler ausgegeben und 0 zurückgeben.
		uint handleTest = findHandleByPtr(objectPtr);
		if (handleTest != 0) {
			error("Tried to register a object that was already registered.");
			return 0;
		}
		// Falls das Handle bereits vergeben ist, wird ein Fehler ausgegeben und 0 zurückgegeben.
		else if (findPtrByHandle(handle) != 0) {
			error("Tried to register a handle that is already taken.");
			return 0;
		}
		// Ansonsten wird das Objekt in beide Maps eingetragen und das gewünschte Handle zurückgeben.
		else {
			_handle2PtrMap[handle] = objectPtr;
			_ptr2HandleMap[objectPtr] = handle;

			// Falls das vergebene Handle größer oder gleich dem nächsten automatische vergebenen Handle ist, wird das nächste automatisch
			// vergebene Handle erhöht.
			if (handle >= _nextHandle)
				_nextHandle = handle + 1;

			return handle;
		}
	}

	void deregisterObject(T *objectPtr) {
		uint handle = findHandleByPtr(objectPtr);

		if (handle != 0) {
			// Registriertes Objekt aus beiden Maps entfernen.
			_handle2PtrMap.erase(findHandleByPtr(objectPtr));
			_ptr2HandleMap.erase(objectPtr);
		} else {
			warning("Tried to remove a object that was not registered.");
		}
	}

	T *resolveHandle(uint handle) {
		// Zum Handle gehöriges Objekt in der Hash-Map finden.
		T *objectPtr = findPtrByHandle(handle);

		// Pointer zurückgeben. Im Fehlerfall ist dieser 0.
		return objectPtr;
	}

	uint resolvePtr(T *objectPtr) {
		// Zum Pointer gehöriges Handle in der Hash-Map finden.
		uint handle = findHandleByPtr(objectPtr);

		// Handle zurückgeben. Im Fehlerfall ist dieses 0.
		return handle;
	}

protected:
	struct ClassPointer_EqualTo {
		bool operator()(const T *x, const T *y) const {
			return x == y;
		}
	};
	struct ClassPointer_Hash {
		uint operator()(const T *x) const {
			return (uint)(x - (const T *)0);
		}
	};

	typedef Common::HashMap<uint, T *>  HANDLE2PTR_MAP;
	typedef Common::HashMap<T *, uint, ClassPointer_Hash, ClassPointer_EqualTo> PTR2HANDLE_MAP;

	HANDLE2PTR_MAP  _handle2PtrMap;
	PTR2HANDLE_MAP  _ptr2HandleMap;
	uint    _nextHandle;

	T *findPtrByHandle(uint handle) {
		// Zum Handle gehörigen Pointer finden.
		typename HANDLE2PTR_MAP::const_iterator it = _handle2PtrMap.find(handle);

		// Pointer zurückgeben, oder, falls keiner gefunden wurde, 0 zurückgeben.
		return (it != _handle2PtrMap.end()) ? it->_value : 0;
	}

	uint findHandleByPtr(T *objectPtr) {
		// Zum Pointer gehöriges Handle finden.
		typename PTR2HANDLE_MAP::const_iterator it = _ptr2HandleMap.find(objectPtr);

		// Handle zurückgeben, oder, falls keines gefunden wurde, 0 zurückgeben.
		return (it != _ptr2HandleMap.end()) ? it->_value : 0;
	}
};

} // End of namespace Sword25

#endif
