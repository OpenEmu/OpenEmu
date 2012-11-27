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

#ifndef GOB_SAVE_SAVECONVERTER_H
#define GOB_SAVE_SAVECONVERTER_H

#include "common/stream.h"

namespace Gob {

class GobEngine;
class SavePartInfo;
class SavePartVars;
class SavePartMem;
class SavePartSprite;
class SaveWriter;

/** A wrapping stream class for old saves. */
class SaveConverter : public Common::SeekableReadStream {
public:
	SaveConverter(GobEngine *vm, const Common::String &fileName);
	virtual ~SaveConverter();

	/** Clear the converter. */
	virtual void clear();
	/** Set the filename on which to operate. */
	virtual void setFileName(const Common::String &fileName);

	/** Is it actually an old save? */
	virtual int isOldSave(Common::InSaveFile **save = 0) const = 0;
	/** Directly return the description without processing the whole save. */
	virtual char *getDescription(Common::SeekableReadStream &save) const = 0;
	/** Load the whole save. */
	virtual bool load() = 0;

	/** Set the name and return the description. */
	char *getDescription(const Common::String &fileName);
	/** Get the current fileName's description. */
	char *getDescription() const;

	// Stream
	virtual bool err() const;
	virtual void clearErr();
	// ReadStream
	virtual bool eos() const;
	virtual uint32 read(void *dataPtr, uint32 dataSize);
	// SeekableReadStream
	virtual int32 pos() const;
	virtual int32 size() const;
	virtual bool seek(int32 offset, int whence = SEEK_SET);

protected:
	GobEngine *_vm;

	Common::String _fileName;

	byte *_data;
	Common::SeekableReadStream *_stream;

	Common::InSaveFile *openSave() const;

	/** Write a warning to stdout to notify the user what's going on. */
	virtual void displayWarning() const;

	virtual uint32 getActualSize(Common::InSaveFile **save = 0) const;

	SavePartInfo *readInfo(Common::SeekableReadStream &stream,
		uint32 descLength, bool hasSizes = true) const;
	SavePartVars *readVars(Common::SeekableReadStream &stream,
			uint32 count, bool endian) const;
	SavePartMem *readMem(Common::SeekableReadStream &stream,
			uint32 count, bool endian) const;
	SavePartSprite *readSprite(Common::SeekableReadStream &stream,
			uint32 width, uint32 height, bool palette) const;

	bool createStream(SaveWriter &writer);

	/** Swap the endianness of the complete data area. */
	static bool swapDataEndian(byte *data, const byte *sizes, uint32 count);

private:
	byte *readData(Common::SeekableReadStream &stream,
			uint32 count, bool endian) const;
};

/** A wrapper for old notes saves. */
class SaveConverter_Notes : public SaveConverter {
public:
	SaveConverter_Notes(GobEngine *vm, uint32 notesSize, const Common::String &fileName = "");
	~SaveConverter_Notes();

	int isOldSave(Common::InSaveFile **save = 0) const;
	char *getDescription(Common::SeekableReadStream &save) const;

	bool load();

private:
	uint32 _size;

	bool loadFail(SavePartVars *vars, Common::InSaveFile *save);
};

/** A wrapper for old v2-style saves (Gobliins 2, Ween: The Prophecy and Bargon Attack). */
class SaveConverter_v2 : public SaveConverter {
public:
	SaveConverter_v2(GobEngine *vm, const Common::String &fileName = "");
	~SaveConverter_v2();

	int isOldSave(Common::InSaveFile **save = 0) const;
	char *getDescription(Common::SeekableReadStream &save) const;

	bool load();

private:
	static const uint32 kSlotCount = 15;
	static const uint32 kSlotNameLength = 40;

	bool loadFail(SavePartInfo *info, SavePartVars *vars,
			Common::InSaveFile *save);
};

/** A wrapper for old v3-style saves (Goblins 3 and Lost in Time). */
class SaveConverter_v3 : public SaveConverter {
public:
	SaveConverter_v3(GobEngine *vm, const Common::String &fileName = "");
	~SaveConverter_v3();

	int isOldSave(Common::InSaveFile **save = 0) const;
	char *getDescription(Common::SeekableReadStream &save) const;

	bool load();

private:
	static const uint32 kSlotCount = 30;
	static const uint32 kSlotNameLength = 40;

	bool loadFail(SavePartInfo *info, SavePartVars *vars,
			SavePartSprite *sprite, Common::InSaveFile *save);

	void getScreenShotProps(int type,
			bool &used, uint32 &width, uint32 &height);
};

/** A wrapper for old v4-style saves (Woodruff). */
class SaveConverter_v4 : public SaveConverter {
public:
	SaveConverter_v4(GobEngine *vm, const Common::String &fileName = "");
	~SaveConverter_v4();

	int isOldSave(Common::InSaveFile **save = 0) const;
	char *getDescription(Common::SeekableReadStream &save) const;

	bool load();

private:
	static const uint32 kSlotCount = 60;
	static const uint32 kSlotNameLength = 40;

	bool loadFail(SavePartInfo *info, SavePartVars *vars,
			SavePartMem *props, Common::InSaveFile *save);
};

} // End of namespace Gob

#endif // GOB_SAVE_SAVECONVERTER_H
