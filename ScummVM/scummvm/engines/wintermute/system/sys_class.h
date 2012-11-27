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

#ifndef WINTERMUTE_SYSCLASS_H
#define WINTERMUTE_SYSCLASS_H

#include "engines/wintermute/persistent.h"
#include "engines/wintermute/dctypes.h"
#include "common/hashmap.h"
#include "common/func.h"
#include "common/stream.h"

namespace Wintermute {
class SystemInstance;
class BaseGame;
class BasePersistenceManager;
class SystemClass;

}

namespace Common {
template<typename T> struct Hash;

template<> struct Hash<void *> : public UnaryFunction<void *, uint> {
	uint operator()(void *val) const {
		return (uint)((size_t)val);
	}
};

template<> struct Hash<Wintermute::SystemInstance *> : public UnaryFunction<Wintermute::SystemInstance *, uint> {
	uint operator()(Wintermute::SystemInstance *val) const {
		return (uint)((size_t)val);
	}
};


}

namespace Wintermute {

class SystemClass {
public:
	SystemClass(const AnsiString &name, PERSISTBUILD build, PERSISTLOAD load, bool persistentClass);
	~SystemClass();

	int getNumInstances();
	bool removeInstance(void *instance);
	SystemInstance *addInstance(void *instance, int id, int savedId = -1);
	bool removeAllInstances();

	int getInstanceID(void *pointer);
	void *idToPointer(int savedID);

	void setID(int id) {
		_iD = id;
	}
	int getID() const {
		return _iD;
	}

	int getSavedID() const {
		return _savedID;
	}

	bool isPersistent() const {
		return _persistent;
	}

	AnsiString getName() const {
		return _name;
	}

	void saveTable(BaseGame *Game, BasePersistenceManager *PersistMgr);
	void loadTable(BaseGame *Game, BasePersistenceManager *PersistMgr);

	void saveInstances(BaseGame *Game, BasePersistenceManager *PersistMgr);
	void loadInstance(void *instance, BasePersistenceManager *PersistMgr);

	void instanceCallback(SYS_INSTANCE_CALLBACK lpCallback, void *lpData);

	void resetSavedIDs();

	void dump(Common::WriteStream *stream);

private:
	int _numInst;
	bool _persistent;
	SystemClass *_next;
	int _iD;
	int _savedID;
	AnsiString _name;
	PERSISTBUILD _build;
	PERSISTLOAD _load;

	//typedef std::set<SystemInstance *> Instances;
	typedef Common::HashMap<SystemInstance *, SystemInstance *> Instances;
	Instances _instances;

	typedef Common::HashMap<void *, SystemInstance *> InstanceMap;
	InstanceMap _instanceMap;
};

} // end of namespace Wintermute

#endif
