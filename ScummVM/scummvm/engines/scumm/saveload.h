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

#ifndef SCUMM_SAVELOAD_H
#define SCUMM_SAVELOAD_H

#include "common/scummsys.h"
#include <stddef.h>	// for ptrdiff_t

namespace Common {
class SeekableReadStream;
class WriteStream;
}

namespace Scumm {


/**
 * The current savegame format version.
 * Our save/load system uses an elaborate scheme to allow us to modify the
 * savegame while keeping full backward compatibility, in the sense that newer
 * ScummVM versions always are able to load old savegames.
 * In order to achieve that, we store a version in the savegame files, and whenever
 * the savegame layout is modified, the version is incremented.
 *
 * This roughly works by marking each savegame entry with a range of versions
 * for which it is valid; the save/load code iterates over all entries, but
 * only saves/loads those which are valid for the version of the savegame
 * which is being loaded/saved currently.
 */
#define CURRENT_VER 93

/**
 * An auxillary macro, used to specify savegame versions. We use this instead
 * of just writing the raw version, because this way they stand out more to
 * the reading eye, making it a bit easier to navigate through the code.
 */
#define VER(x) x


/**
 * The OFFS macro essentially provides the functionality of offsetof(), that
 * is, it determines the offset of a struct/class member within instances of
 * that class.
 *
 * This is a place where we cheat a bit and sacrifice some potential portability
 * (although so far we haven't encountered any platform where this matters).
 *
 * To work around a warning in GCC 3.2 (and 3.1 ?) regarding non-POD types,
 * we use a small trick: instead of 0 we use 42. Why? Well, it seems newer GCC
 * versions have a heuristic built in to detect "offset-of" patterns - which is exactly
 * what our OFFS macro does. Now, for non-POD types this is not really legal, because
 * member need not be at a fixed offset relative to the variable, even if they are in
 * current reality (many of our complex structs are non-POD; for an explanation of
 * what POD means refer to <http://en.wikipedia.org/wiki/Plain_Old_Data_Structures> or
 * to <http://www.informit.com/guides/content.asp?g=cplusplus&seqNum=32&rl=1>)
 */
#define OFFS(type,item) (((ptrdiff_t)(&((type *)42)->type::item))-42)

/**
 * Similar to the OFFS macro, this macro computes the size (in bytes) of a
 * member of a given struct/class type.
 */
#define SIZE(type,item) sizeof(((type *)42)->type::item)

// Any item that is still in use automatically gets a maxVersion equal to CURRENT_VER
#define MKLINE(type,item,saveas,minVer) {OFFS(type,item),saveas,SIZE(type,item),minVer,CURRENT_VER}
#define MKARRAY(type,item,saveas,dim,minVer) {OFFS(type,item),128|saveas,SIZE(type,item),minVer,CURRENT_VER}, {dim,1,0,0,0}
#define MKARRAY2(type,item,saveas,dim,dim2,rowlen,minVer) {OFFS(type,item),128|saveas,SIZE(type,item),minVer,CURRENT_VER}, {dim,dim2,rowlen,0,0}

// Use this if you have an entry that used to be smaller:
#define MKLINE_OLD(type,item,saveas,minVer,maxVer) {OFFS(type,item),saveas,SIZE(type,item),minVer,maxVer}
#define MKARRAY_OLD(type,item,saveas,dim,minVer,maxVer) {OFFS(type,item),128|saveas,SIZE(type,item),minVer,maxVer}, {dim,1,0,0,0}
#define MKARRAY2_OLD(type,item,saveas,dim,dim2,rowlen,minVer,maxVer) {OFFS(type,item),128|saveas,SIZE(type,item),minVer,maxVer}, {dim,dim2,rowlen,0,0}

// An obsolete item/array, to be ignored upon load. We retain the type/item params to make it easier to debug.
// Obsolete items have size == 0.
#define MK_OBSOLETE(type,item,saveas,minVer,maxVer) {0,saveas,0,minVer,maxVer}
#define MK_OBSOLETE_ARRAY(type,item,saveas,dim,minVer,maxVer) {0,128|saveas,0,minVer,maxVer}, {dim,1,0,0,0}
#define MK_OBSOLETE_ARRAY2(type,item,saveas,dim,dim2,rowlen,minVer,maxVer) {0,128|saveas,0,minVer,maxVer}, {dim,dim2,rowlen,0,0}

// End marker
#define MKEND() {0xFFFF,0xFF,0xFF,0,0}


enum {
	sleByte = 1,
	sleUint8 = 1,
	sleInt8 = 1,
	sleInt16 = 2,
	sleUint16 = 3,
	sleInt32 = 4,
	sleUint32 = 5
};

struct SaveLoadEntry {
	uint32 offs;	// or: array dimension
	uint16 type;	// or: array dimension 2
	uint16 size;	// or: array row length
	uint8 minVersion;
	uint8 maxVersion;
};

class Serializer {
public:
	Serializer(Common::SeekableReadStream *in, Common::WriteStream *out, uint32 savegameVersion)
		: _loadStream(in), _saveStream(out),
		  _savegameVersion(savegameVersion)
	{ }

	void saveLoadArrayOf(void *b, int len, int datasize, byte filetype);
	void saveLoadArrayOf(void *b, int num, int datasize, const SaveLoadEntry *sle);
	void saveLoadEntries(void *d, const SaveLoadEntry *sle);

	bool isSaving() { return (_saveStream != 0); }
	bool isLoading() { return (_loadStream != 0); }
	uint32 getVersion() { return _savegameVersion; }

	void saveUint32(uint32 d);
	void saveUint16(uint16 d);
	void saveByte(byte b);

	byte loadByte();
	uint16 loadUint16();
	uint32 loadUint32();

	void saveBytes(void *b, int len);
	void loadBytes(void *b, int len);

protected:
	Common::SeekableReadStream *_loadStream;
	Common::WriteStream *_saveStream;
	uint32 _savegameVersion;

	void saveArrayOf(void *b, int len, int datasize, byte filetype);
	void loadArrayOf(void *b, int len, int datasize, byte filetype);

	void saveEntries(void *d, const SaveLoadEntry *sle);
	void loadEntries(void *d, const SaveLoadEntry *sle);
};


// Mixin class / interface. Maybe call it ISerializable or SerializableMixin ?
class Serializable {
public:
	virtual ~Serializable() {}
	virtual void saveLoadWithSerializer(Serializer *ser) = 0;
};

} // End of namespace Scumm

#endif
