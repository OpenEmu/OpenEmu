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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/system/sys_instance.h"
#include "engines/wintermute/system/sys_class_registry.h"
#include "engines/wintermute/system/sys_class.h"
#include "engines/wintermute/wintermute.h"
#include "common/stream.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
SystemClassRegistry::SystemClassRegistry() {
	_count = 0;
	_disabled = false;
}


//////////////////////////////////////////////////////////////////////////
SystemClassRegistry::~SystemClassRegistry() {
	unregisterClasses();
}

//////////////////////////////////////////////////////////////////////////
SystemClassRegistry *SystemClassRegistry::getInstance() {
	return BaseEngine::instance().getClassRegistry();
}

void SystemClassRegistry::unregisterClasses() {
	// SystemClass calls UnregisterClass upon destruction.
	while (_classes.size() > 0) {
		delete _classes.begin()->_value;
	}
}

//////////////////////////////////////////////////////////////////////////
bool SystemClassRegistry::registerClass(SystemClass *classObj) {
	classObj->setID(_count++);
	//_classes.insert(classObj);
	_classes[classObj] = classObj;

	_nameMap[classObj->getName()] = classObj;
	_idMap[classObj->getID()] = classObj;

	return true;
}


//////////////////////////////////////////////////////////////////////////
bool SystemClassRegistry::unregisterClass(SystemClass *classObj) {

	Classes::iterator it = _classes.find(classObj);
	if (it == _classes.end()) {
		return false;
	}

	if (classObj->getNumInstances() != 0) {
		debugC(Wintermute::kWintermuteDebugSaveGame, "Memory leak@class %-20s: %d instance(s) left\n", classObj->getName().c_str(), classObj->getNumInstances());
	}
	_classes.erase(it);

	NameMap::iterator mapIt = _nameMap.find(classObj->getName());
	if (mapIt != _nameMap.end()) {
		_nameMap.erase(mapIt);
	}

	IdMap::iterator idIt = _idMap.find(classObj->getID());
	if (idIt != _idMap.end()) {
		_idMap.erase(idIt);
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
bool SystemClassRegistry::registerInstance(const char *className, void *instance) {
	if (_disabled) {
		return true;
	}

	NameMap::iterator mapIt = _nameMap.find(className);
	if (mapIt == _nameMap.end()) {
		return false;
	}

	SystemInstance *inst = (*mapIt)._value->addInstance(instance, _count++);
	return (inst != NULL);
}

//////////////////////////////////////////////////////////////////////////
void SystemClassRegistry::addInstanceToTable(SystemInstance *instance, void *pointer) {
	_instanceMap[pointer] = instance;

	if (instance->getSavedID() >= 0) {
		_savedInstanceMap[instance->getSavedID()] = instance;
	}
}

//////////////////////////////////////////////////////////////////////////
int SystemClassRegistry::getNextID() {
	return _count++;
}

//////////////////////////////////////////////////////////////////////////
bool SystemClassRegistry::unregisterInstance(const char *className, void *instance) {
	NameMap::iterator mapIt = _nameMap.find(className);
	if (mapIt == _nameMap.end()) {
		return false;
	}
	(*mapIt)._value->removeInstance(instance);

	InstanceMap::iterator instIt = _instanceMap.find(instance);
	if (instIt != _instanceMap.end()) {
		_instanceMap.erase(instIt);
		return true;
	} else {
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
bool SystemClassRegistry::getPointerID(void *pointer, int *classID, int *instanceID) {
	if (pointer == NULL) {
		return true;
	}

	InstanceMap::iterator it = _instanceMap.find(pointer);
	if (it == _instanceMap.end()) {
		return false;
	}


	SystemInstance *inst = (*it)._value;
	*instanceID = inst->getID();
	*classID = inst->getClass()->getID();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void *SystemClassRegistry::idToPointer(int classID, int instanceID) {
	SavedInstanceMap::iterator it = _savedInstanceMap.find(instanceID);
	if (it == _savedInstanceMap.end()) {
		return NULL;
	} else {
		return (*it)._value->getInstance();
	}
}

bool checkHeader(const char *tag, BasePersistenceManager *pm) {
	char *test = pm->getString();
	Common::String verify = test;
	delete[] test;
	bool retVal = (verify == tag);
	if (!retVal) {
		error("Expected %s in Save-file not found", tag);
	}
	return retVal;
}

//////////////////////////////////////////////////////////////////////////
bool SystemClassRegistry::saveTable(BaseGame *gameRef, BasePersistenceManager *persistMgr, bool quickSave) {
	persistMgr->putString("<CLASS_REGISTRY_TABLE>");
	persistMgr->putDWORD(_classes.size());

	int counter = 0;

	Classes::iterator it;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		counter++;

		if (!quickSave) {
			gameRef->_renderer->setIndicatorVal((int)(50.0f / (float)((float)_classes.size() / (float)counter)));
		}

		(it->_value)->saveTable(gameRef,  persistMgr);
	}
	persistMgr->putString("</CLASS_REGISTRY_TABLE>");
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool SystemClassRegistry::loadTable(BaseGame *gameRef, BasePersistenceManager *persistMgr) {
	checkHeader("<CLASS_REGISTRY_TABLE>", persistMgr);

	// reset SavedID of current instances
	Classes::iterator it;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		(it->_value)->resetSavedIDs();
	}

	for (it = _classes.begin(); it != _classes.end(); ++it) {
		if ((it->_value)->isPersistent()) {
			continue;
		}
		(it->_value)->removeAllInstances();
	}

	_instanceMap.clear();

	uint32 numClasses = persistMgr->getDWORD();

	for (uint32 i = 0; i < numClasses; i++) {
		gameRef->_renderer->setIndicatorVal((int)(50.0f / (float)((float)numClasses / (float)i)));

		Common::String className = persistMgr->getStringObj();
		NameMap::iterator mapIt = _nameMap.find(className);
		if (mapIt != _nameMap.end()) {
			(*mapIt)._value->loadTable(gameRef,  persistMgr);
		}
	}

	checkHeader("</CLASS_REGISTRY_TABLE>", persistMgr);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool SystemClassRegistry::saveInstances(BaseGame *gameRef, BasePersistenceManager *persistMgr, bool quickSave) {

	Classes::iterator it;

	// count total instances
	int numInstances = 0;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		numInstances += (it->_value)->getNumInstances();
	}

	persistMgr->putDWORD(numInstances);

	int counter = 0;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		counter++;

		if (!quickSave) {
			if (counter % 20 == 0) {
				gameRef->_renderer->setIndicatorVal((int)(50.0f + 50.0f / (float)((float)_classes.size() / (float)counter)));
			}
		}
		gameRef->miniUpdate();

		(it->_value)->saveInstances(gameRef,  persistMgr);
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool SystemClassRegistry::loadInstances(BaseGame *gameRef, BasePersistenceManager *persistMgr) {
	// get total instances
	int numInstances = persistMgr->getDWORD();

	for (int i = 0; i < numInstances; i++) {
		if (i % 20 == 0) {
			gameRef->_renderer->setIndicatorVal((int)(50.0f + 50.0f / (float)((float)numInstances / (float)i)));
		}

		checkHeader("<INSTANCE_HEAD>", persistMgr);

		int classID = persistMgr->getDWORD();
		int instanceID = persistMgr->getDWORD();
		void *instance = idToPointer(classID, instanceID);

		checkHeader("</INSTANCE_HEAD>", persistMgr);

		Classes::iterator it;
		for (it = _classes.begin(); it != _classes.end(); ++it) {
			if ((it->_value)->getSavedID() == classID) {
				(it->_value)->loadInstance(instance, persistMgr);
				break;
			}
		}
		checkHeader("</INSTANCE>", persistMgr);
	}

	_savedInstanceMap.clear();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool SystemClassRegistry::enumInstances(SYS_INSTANCE_CALLBACK lpCallback, const char *className, void *lpData) {
	NameMap::iterator mapIt = _nameMap.find(className);
	if (mapIt == _nameMap.end()) {
		return STATUS_FAILED;
	}

	(*mapIt)._value->instanceCallback(lpCallback, lpData);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void SystemClassRegistry::dumpClasses(Common::WriteStream *stream) {
	Classes::iterator it;
	for (it = _classes.begin(); it != _classes.end(); ++it) {
		(it->_value)->dump(stream);
	}
}

} // end of namespace Wintermute
