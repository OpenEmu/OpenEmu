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

#ifndef MADE_DATABASE_H
#define MADE_DATABASE_H

#include "common/hashmap.h"

namespace Common {
class SeekableReadStream;
class WriteStream;
class String;
}

namespace Made {

class MadeEngine;

class Object {
public:

	Object();
	virtual ~Object();

	virtual int load(Common::SeekableReadStream &source) = 0;
	virtual int load(byte *source) = 0;
	virtual int save(Common::WriteStream &dest) = 0;
	virtual uint16 getFlags() = 0;
	virtual uint16 getClass() = 0;
	virtual uint16 getSize() = 0;
	virtual byte getCount1() = 0;
	virtual byte getCount2() = 0;
	virtual byte *getData() = 0;
	virtual bool isConstant() = 0;

	const char *getString();
	void setString(const char *str);

	bool isObject();
	bool isVector();

	int16 getVectorSize();
	int16 getVectorItem(int16 index);
	void setVectorItem(int16 index, int16 value);

	void dump(const Common::String &filename);

protected:
	bool _freeData;
	uint16 _objSize;
	byte *_objData;
};

class ObjectV2 : public Object {
public:
	int load(Common::SeekableReadStream &source);
	int load(byte *source);
	int save(Common::WriteStream &dest);
	uint16 getFlags();
	uint16 getClass();
	uint16 getSize();
	byte getCount1();
	byte getCount2();
	byte *getData();

	bool isConstant() {
		return false;
	}
};

class ObjectV1 : public ObjectV2 {
public:
	int load(Common::SeekableReadStream &source);
};

class ObjectV3 : public Object {
public:
	int load(Common::SeekableReadStream &source);
	int load(byte *source);
	int save(Common::WriteStream &dest);
	uint16 getFlags();
	uint16 getClass();
	uint16 getSize();
	byte getCount1();
	byte getCount2();
	byte *getData();

	bool isConstant() {
		return !(getFlags() & 1);
	}

};

class GameDatabase {
public:

	GameDatabase(MadeEngine *vm);
	virtual ~GameDatabase();

	void open(const char *filename);
	void openFromRed(const char *redFilename, const char *filename);

	void reload();

	Object *getObject(int16 index) const {
		if (index >= 1)
			return _objects[index - 1];
		else
			return NULL;
	}

	uint getObjectCount() const { return _objects.size(); }

	int16 getMainCodeObjectIndex() const { return _mainCodeObjectIndex; }

	int16 getVar(int16 index);
	void setVar(int16 index, int16 value);

	const char *getObjectString(int16 index);
	void setObjectString(int16 index, const char *str);

	virtual int16 *findObjectProperty(int16 objectIndex, int16 propertyId, int16 &propertyFlag) = 0;
	int16 *findObjectPropertyCached(int16 objectIndex, int16 propertyId, int16 &propertyFlag);
	virtual const char *getString(uint16 offset) = 0;
	virtual bool getSavegameDescription(const char *filename, Common::String &description, int16 version) = 0;
	virtual int16 savegame(const char *filename, const char *description, int16 version) = 0;
	virtual int16 loadgame(const char *filename, int16 version) = 0;

	int16 getObjectProperty(int16 objectIndex, int16 propertyId);
	int16 setObjectProperty(int16 objectIndex, int16 propertyId, int16 value);

	void dumpObject(int16 index);

protected:
	typedef Common::HashMap<uint32, int16 *> ObjectPropertyCacheMap;
	MadeEngine *_vm;
	Common::Array<Object *> _objects;
	ObjectPropertyCacheMap _objectPropertyCache;
	byte *_gameState;
	uint32 _gameStateSize;
	int16 _mainCodeObjectIndex;
	bool _isRedSource;
	Common::String _filename, _redFilename;
	virtual void load(Common::SeekableReadStream &sourceS) = 0;
	virtual void reloadFromStream(Common::SeekableReadStream &sourceS) = 0;
};

class GameDatabaseV2 : public GameDatabase {
public:
	GameDatabaseV2(MadeEngine *vm);
	~GameDatabaseV2();
	int16 *findObjectProperty(int16 objectIndex, int16 propertyId, int16 &propertyFlag);
	const char *getString(uint16 offset);
	bool getSavegameDescription(const char *filename, Common::String &description, int16 version);
	int16 savegame(const char *filename, const char *description, int16 version);
	int16 loadgame(const char *filename, int16 version);
protected:
	char *_gameText;
	void load(Common::SeekableReadStream &sourceS);
	void reloadFromStream(Common::SeekableReadStream &sourceS);
};

class GameDatabaseV3 : public GameDatabase {
public:
	GameDatabaseV3(MadeEngine *vm);
	int16 *findObjectProperty(int16 objectIndex, int16 propertyId, int16 &propertyFlag);
	const char *getString(uint16 offset);
	bool getSavegameDescription(const char *filename, Common::String &description, int16 version);
	int16 savegame(const char *filename, const char *description, int16 version);
	int16 loadgame(const char *filename, int16 version);
protected:
	char *_gameText;
	uint32 _gameStateOffs;
	void load(Common::SeekableReadStream &sourceS);
	void reloadFromStream(Common::SeekableReadStream &sourceS);
};

} // End of namespace Made

#endif /* MADE_H */
