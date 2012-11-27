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

#ifndef GOB_SAVE_SAVEHANDLER_H
#define GOB_SAVE_SAVEHANDLER_H

#include "common/savefile.h"
#include "common/array.h"

#include "engines/gob/video.h" // for SurfacePtr

namespace Gob {

class GobEngine;
class SavePartInfo;
class SavePartVars;
class SavePartSprite;
class SaveConverter;

/** Slot file related class. */
class SlotFile {
public:
	/** The constructor.
	 *
	 *  @param slotCount Number of slots.
	 *  @param base The file's base string.
	 */
	SlotFile(GobEngine *vm, uint32 slotCount, const Common::String &base);
	virtual ~SlotFile();

	/** Calculates which slot to use. */
	virtual int getSlot(int32 offset) const = 0;
	/** Calculates the slot remainder, for error checking. */
	virtual int getSlotRemainder(int32 offset) const = 0;

protected:
	GobEngine *_vm;
	Common::String _base;

	uint32 _slotCount;
};

/** An indexed slot file ("foobar.s00", "foobar.s01", ...). */
class SlotFileIndexed : public SlotFile {
public:
	SlotFileIndexed(GobEngine *vm, uint32 slotCount, const Common::String &base,
			const Common::String &extStub);
	~SlotFileIndexed();

	/** Build the save file name. */
	Common::String build(int slot) const;

	/** Returns the highest filled slot number. */
	virtual uint32 getSlotMax() const;

	/** Returns the size of all existing slots + the index. */
	virtual int32 tallyUpFiles(uint32 slotSize, uint32 indexSize) const;

	/** Creates an index in buffer. */
	virtual void buildIndex(byte *buffer, SavePartInfo &info,
			SaveConverter *converter = 0, bool setLongest = false) const;

	virtual bool exists(int slot) const;
	virtual Common::InSaveFile *openRead(int slot) const;
	virtual Common::OutSaveFile *openWrite(int slot) const;

protected:
	Common::String _ext;
};

/** A static slot file ("foo.bar"). */
class SlotFileStatic : public SlotFile {
public:
	SlotFileStatic(GobEngine *vm, const Common::String &base, const Common::String &ext);
	~SlotFileStatic();

	int getSlot(int32 offset) const;
	int getSlotRemainder(int32 offset) const;

	/** Build the save file name. */
	Common::String build() const;

	virtual bool exists() const;
	virtual Common::InSaveFile *openRead() const;
	virtual Common::OutSaveFile *openWrite() const;

protected:
	Common::String _ext;
};

/** A handler for a specific save file. */
class SaveHandler {
public:
	SaveHandler(GobEngine *vm);
	virtual ~SaveHandler();

	/** Returns the file's (virtual) size. */
	virtual int32 getSize() = 0;
	/** Loads (parts of) the file. */
	virtual bool load(int16 dataVar, int32 size, int32 offset) = 0;
	/** Saves (parts of) the file. */
	virtual bool save(int16 dataVar, int32 size, int32 offset) = 0;

	/** Deletes the file. */
	virtual bool deleteFile();

	static uint32 getVarSize(GobEngine *vm);

protected:
	GobEngine *_vm;
};

/** A handler for temporary sprites. */
class TempSpriteHandler : public SaveHandler {
public:
	TempSpriteHandler(GobEngine *vm);
	~TempSpriteHandler();

	int32 getSize();
	bool load(int16 dataVar, int32 size, int32 offset);
	bool save(int16 dataVar, int32 size, int32 offset);

	bool create(uint32 width, uint32 height, bool trueColor);
	bool createFromSprite(int16 dataVar, int32 size, int32 offset);

protected:
	SavePartSprite *_sprite;

	/** Determine whether it's a dummy sprite save/load. */
	static bool isDummy(int32 size);
	/** Determine whether using a sprite was requested. */
	static bool isSprite(int32 size);
	/** Determine which sprite is meant. */
	static int getIndex(int32 size);
	/** Determine whether the palette should be used too. */
	static bool usesPalette(int32 size);

	SurfacePtr createSprite(int16 dataVar, int32 size, int32 offset);
};

/** A handler for notes. */
class NotesHandler : public SaveHandler {
public:
	NotesHandler(uint32 notesSize, GobEngine *vm, const Common::String &target);
	~NotesHandler();

	int32 getSize();
	bool load(int16 dataVar, int32 size, int32 offset);
	bool save(int16 dataVar, int32 size, int32 offset);

private:
	class File : public SlotFileStatic {
	public:
		File(GobEngine *vm, const Common::String &base);
		~File();
	};

	uint32 _notesSize;
	File *_file;
	SavePartVars *_notes;
};

/** A handler that behaves like a file but keeps the contents in memory. */
class FakeFileHandler : public SaveHandler {
public:
	FakeFileHandler(GobEngine *vm);
	~FakeFileHandler();

	int32 getSize();
	bool load(int16 dataVar, int32 size, int32 offset);
	bool save(int16 dataVar, int32 size, int32 offset);

	bool deleteFile();

private:
	Common::Array<byte> _data;
};

} // End of namespace Gob

#endif // GOB_SAVE_SAVEHANDLER_H
