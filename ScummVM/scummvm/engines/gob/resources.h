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

#ifndef GOB_RESOURCES_H
#define GOB_RESOURCES_H

#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace Gob {

class GobEngine;

class Resource {
public:
	Resource(byte *data, int32 size, bool needFree = true,
			int16 width = 0, int16 height = 0);
	~Resource();

	byte *getData  () const;
	int32 getSize  () const;
	int16 getWidth () const;
	int16 getHeight() const;

	Common::SeekableReadStream *stream() const;

private:
	byte *_data;
	int32 _size;
	int16 _width;
	int16 _height;
	bool  _needFree;

	Common::SeekableReadStream *_stream;
};

class TextItem {
public:
	TextItem(byte *data, int32 size);
	~TextItem();

	byte *getData() const;
	int32 getSize() const;

	Common::SeekableReadStream *stream() const;

private:
	byte *_data;
	int32 _size;

	Common::SeekableReadStream *_stream;
};

class Resources {
public:
	Resources(GobEngine *vm);
	~Resources();

	bool load(const Common::String &fileName);
	void unload(bool del = true);

	bool isLoaded() const;

	Resource *getResource(uint16 id, int16 *width = 0, int16 *height = 0) const;
	TextItem *getTextItem(uint16 id) const;

	byte *getTexts() const;

	bool dumpResource(const Resource &resource,
			const Common::String &fileName) const;
	bool dumpResource(const Resource &resource, uint16 id,
			const Common::String &ext = "dmp") const;

private:
	// Structure sizes in the files
	static const int kTOTResItemSize   = 4 + 2 + 2 + 2;
	static const int kTOTResTableSize  = 2 + 1;
	static const int kEXTResItemSize   = 4 + 2 + 2 + 2;
	static const int kEXTResTableSize  = 2 + 1;
	static const int kTOTTextTableSize = 2;
	static const int kTOTTextItemSize  = 2 + 2;

	enum ResourceType {
		kResourceTOT = 0,
		kResourceIM,
		kResourceEXT,
		kResourceEX
	};

	struct TOTResourceItem {
		ResourceType type;
		uint16 size;
		int16 width;
		int16 height;
		union {
			int32 offset;
			int32 index;
		};
	};

	struct TOTResourceTable {
		int16 itemsCount;
		byte unknown;
		TOTResourceItem *items;
		uint32 dataOffset;

		TOTResourceTable();
		~TOTResourceTable();
	};

	struct EXTResourceItem {
		ResourceType type;
		int32 offset;
		uint16 size;
		int16 width;
		int16 height;
		bool packed;
	};

	struct EXTResourceTable {
		int16 itemsCount;
		byte unknown;
		EXTResourceItem *items;

		EXTResourceTable();
		~EXTResourceTable();
	};

	struct TOTTextItem {
		uint16 offset;
		int16 size;
	};

	struct TOTTextTable {
		bool needFree;
		int16 itemsCount;

		byte *data;
		int32 size;

		TOTTextItem *items;

		TOTTextTable();
		~TOTTextTable();
	};

	GobEngine *_vm;

	Common::String _fileBase;
	Common::String _totFile;
	Common::String _extFile;
	Common::String _exFile;

	byte *_totData;
	uint32 _totSize;

	byte *_imData;
	uint32 _imSize;

	bool _hasLOM;
	int32 _totResStart;

	TOTResourceTable *_totResourceTable;
	EXTResourceTable *_extResourceTable;
	TOTTextTable     *_totTextTable;

	bool loadTOTResourceTable();
	bool loadEXTResourceTable();
	bool loadTOTTextTable(const Common::String &fileBase);
	bool loadIMFile();
	bool loadEXFile();

	byte *loadTOTLocTexts(const Common::String &fileBase, int32 &size);
	bool getLocTextFile(char *locTextFile, int language);
	Common::String getLocTextFile(const Common::String &fileBase, int language);

	Resource *getTOTResource(uint16 id) const;
	Resource *getEXTResource(uint16 id) const;

	byte *getTOTData(TOTResourceItem &totItem) const;
	byte *getIMData(TOTResourceItem &totItem)  const;
	byte *getEXTData(EXTResourceItem &extItem, uint32 &size) const;
	byte *getEXData(EXTResourceItem &extItem,  uint32 &size) const;
};

} // End of namespace Gob

#endif // GOB_RESOURCES_H
