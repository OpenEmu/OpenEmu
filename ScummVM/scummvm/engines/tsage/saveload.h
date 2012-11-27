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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_SAVELOAD_H
#define TSAGE_SAVELOAD_H

#include "common/scummsys.h"
#include "common/list.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/serializer.h"

namespace TsAGE {

typedef void (*SaveNotifierFn)(bool postFlag);

#define TSAGE_SAVEGAME_VERSION 10

class SavedObject;

struct tSageSavegameHeader {
	uint8 version;
	Common::String saveName;
	Graphics::Surface *thumbnail;
	int saveYear, saveMonth, saveDay;
	int saveHour, saveMinutes;
	int totalFrames;
};

/*--------------------------------------------------------------------------*/

// FIXME: workaround to supress spurious strict-alias warnings on older GCC
// versions. this should be resolved with the savegame rewrite
#define SYNC_POINTER(x) do { \
	SavedObject **y = (SavedObject **)((void *)&x); \
	s.syncPointer(y); \
} while (false)

#define SYNC_ENUM(FIELD, TYPE) int v_##FIELD = (int)FIELD; s.syncAsUint16LE(v_##FIELD); \
	if (s.isLoading()) FIELD = (TYPE)v_##FIELD;

/**
 * Derived serializer class with extra synchronisation types
 */
class Serializer : public Common::Serializer {
public:
	Serializer(Common::SeekableReadStream *in, Common::WriteStream *out) : Common::Serializer(in, out) {}

	// HACK: TSAGE saved games contain a single byte for the savegame version,
	// thus the normal syncVersion() Serializer member won't work here. In order
	// to maintain compatibility with older game saves, this method is provided
	// in order to set the savegame version from a byte
	void setSaveVersion(byte version) { _version = version; }

	void syncPointer(SavedObject **ptr, Common::Serializer::Version minVersion = 0,
		Common::Serializer::Version maxVersion = kLastVersion);
	void validate(const Common::String &s, Common::Serializer::Version minVersion = 0,
		Common::Serializer::Version maxVersion = kLastVersion);
	void validate(int v, Common::Serializer::Version minVersion = 0,
		Common::Serializer::Version maxVersion = kLastVersion);
	void syncAsDouble(double &v);
};

/*--------------------------------------------------------------------------*/

class Serialisable {
public:
	virtual ~Serialisable() {}
	virtual void synchronize(Serializer &s) = 0;
};

class SaveListener {
public:
	virtual ~SaveListener() {}
	virtual void listenerSynchronize(Serializer &s) = 0;
};

/*--------------------------------------------------------------------------*/

class SavedObject : public Serialisable {
public:
	SavedObject();
	virtual ~SavedObject();

	virtual Common::String getClassName() { return "SavedObject"; }
	virtual void synchronize(Serializer &s) {}

	static SavedObject *createInstance(const Common::String &className);
};

/*--------------------------------------------------------------------------*/

/**
 * Derived list class with extra functionality
 */
template<typename T>
class SynchronizedList : public Common::List<T> {
public:
	void synchronize(Serializer &s) {
		int entryCount = 0;

		if (s.isLoading()) {
			this->clear();
			s.syncAsUint32LE(entryCount);

			for (int idx = 0; idx < entryCount; ++idx) {
				this->push_back(static_cast<T>((T)NULL));
				T &obj = Common::List<T>::back();
				s.syncPointer((SavedObject **)&obj);
			}
		} else {
			// Get the list size
			entryCount = this->size();

			// Write out list
			s.syncAsUint32LE(entryCount);
			for (typename Common::List<T>::iterator i = this->begin(); i != this->end(); ++i) {
				s.syncPointer((SavedObject **)&*i);
			}
		}
	}

	void addBefore(T existingItem, T newItem) {
		typename SynchronizedList<T>::iterator i = this->begin();
		while ((i != this->end()) && (*i != existingItem)) ++i;
		this->insert(i, newItem);
	}
	void addAfter(T existingItem, T newItem) {
		typename SynchronizedList<T>::iterator i = this->begin();
		while ((i != this->end()) && (*i != existingItem)) ++i;
		if (i != this->end()) ++i;
		this->insert(i, newItem);
	}
};

/**
 * Search whether an element is contained in a list.
 *
 * @param l List to search.
 * @param v Element to search for.
 * @return True in case the element is contained, false otherwise.
 */
template<typename T>
inline bool contains(const Common::List<T> &l, const T &v) {
	return (Common::find(l.begin(), l.end(), v) != l.end());
}

/**
 * Derived list class for holding function pointers
 */
template<typename T>
class FunctionList : public Common::List<void (*)(T)> {
public:
	void notify(T v) {
		for (typename Common::List<void (*)(T)>::iterator i = this->begin(); i != this->end(); ++i) {
			(*i)(v);
		}
	}
};

/*--------------------------------------------------------------------------*/

class SavedObjectRef {
public:
	SavedObject **_savedObject;
	int _objIndex;

	SavedObjectRef() : _savedObject(NULL), _objIndex(-1) {}
	SavedObjectRef(SavedObject **so, int objIndex) : _savedObject(so),  _objIndex(objIndex) {}
};

typedef SavedObject *(*SavedObjectFactory)(const Common::String &className);

class Saver {
private:
	Common::List<SavedObject *> _objList;
	FunctionList<bool> _saveNotifiers;
	FunctionList<bool> _loadNotifiers;
	Common::List<SaveListener *> _listeners;

	Common::List<SavedObjectRef> _unresolvedPtrs;
	SavedObjectFactory _factoryPtr;

	bool _macroSaveFlag;
	bool _macroRestoreFlag;
	int _saveSlot;

	void resolveLoadPointers();
public:
	Saver();
	~Saver();

	Common::Error save(int slot, const Common::String &saveName);
	Common::Error restore(int slot);
	static bool readSavegameHeader(Common::InSaveFile *in, tSageSavegameHeader &header);
	static void writeSavegameHeader(Common::OutSaveFile *out, tSageSavegameHeader &header);

	void addListener(SaveListener *obj);
	void addSaveNotifier(SaveNotifierFn fn);
	void addLoadNotifier(SaveNotifierFn fn);
	void addObject(SavedObject *obj);
	void removeObject(SavedObject *obj);
	void addFactory(SavedObjectFactory fn) { _factoryPtr = fn; }
	void addSavedObjectPtr(SavedObject **ptr, int objIndex) {
		_unresolvedPtrs.push_back(SavedObjectRef(ptr, objIndex));
	}

	bool savegamesExist() const;
	bool getMacroSaveFlag() const { return _macroSaveFlag; }
	bool getMacroRestoreFlag() const { return _macroRestoreFlag; }
	int blockIndexOf(SavedObject *p);
	int getObjectCount() const;
	void listObjects();
};

extern Saver *g_saver;

} // End of namespace TsAGE

#endif
