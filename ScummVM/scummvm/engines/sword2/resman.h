/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef	SWORD2_RESMAN_H
#define	SWORD2_RESMAN_H

namespace Common {
class File;
}

#define MAX_MEM_CACHE (8 * 1024 * 1024) // we keep up to 8 megs of resource data files in memory
#define	MAX_res_files 20

namespace Sword2 {

class Sword2Engine;

struct Resource {
	byte *ptr;
	uint32 size;
	uint32 refCount;
	Resource *next, *prev;
};

struct ResourceFile {
	char fileName[20];
	int32 numEntries;
	uint32 *entryTab;
	uint8 cd;
};

class ResourceManager {
private:
	Common::File *openCluFile(uint16 fileNum);
	void readCluIndex(uint16 fileNum, Common::File *file);
	void removeFromCacheList(Resource *res);
	void addToCacheList(Resource *res);
	void checkMemUsage();

	Sword2Engine *_vm;

	int _curCD;
	uint32 _totalResFiles;
	uint32 _totalClusters;

	// Gode generated res-id to res number/rel number conversion table

	uint16 *_resConvTable;
	ResourceFile _resFiles[MAX_res_files];
	Resource *_resList;

	Resource *_cacheStart, *_cacheEnd;
	uint32 _usedMem; // amount of used memory in bytes

public:
	ResourceManager(Sword2Engine *vm);	// read in the config file
	~ResourceManager();

	bool init();

	uint32 getNumResFiles() { return _totalResFiles; }
	uint32 getNumClusters() { return _totalClusters; }
	ResourceFile *getResFiles() { return _resFiles; }
	Resource *getResList() { return _resList; }

	byte *openResource(uint32 res, bool dump = false);
	void closeResource(uint32 res);

	bool checkValid(uint32 res);
	uint32 fetchLen(uint32 res);
	uint8 fetchType(byte *ptr);
	uint8 fetchType(uint32 res) {
		byte *ptr = openResource(res);
		uint8 type = fetchType(ptr);
		closeResource(res);

		return type;
	}

	byte *fetchName(uint32 res, byte *buf = NULL) {
		static byte tempbuf[NAME_LEN];

		if (!buf)
			buf = tempbuf;

		byte *ptr = openResource(res);
		memcpy(buf, ptr + 10, NAME_LEN);
		closeResource(res);

		return buf;
	}

	byte *fetchName(byte *ptr) {
		return ptr + 10;
	}

	// Prompts the user for the specified CD.
	void askForCD(int cd);

	void setCD(int cd) {
		if (cd)
			_curCD = cd;
	}

	int getCD() {
		return _curCD;
	}

	void remove(int res);
	void removeAll();

	// ----console commands

	void killAll(bool wantInfo);
	void killAllObjects(bool wantInfo);
};

} // End of namespace Sword2

#endif
