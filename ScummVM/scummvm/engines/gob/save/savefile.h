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

#ifndef GOB_SAVE_SAVEFILE_H
#define GOB_SAVE_SAVEFILE_H

#include "common/endian.h"
#include "common/array.h"
#include "common/savefile.h"

namespace Gob {

class GobEngine;
class Surface;

/**
 * A class wrapping a save part header.
 *
 * A save part header consists of 4 fields:
 * ID      : The 8 character ID \0SCVMGOB
 * Type    : The 4 character ID for this part's type
 * Version : This part's version. Each type has its own version counter
 * Size    : The size of the contents, i.e. excluding this header
*/
class SaveHeader {
public:
	/** The size of the header. */
	static const int kSize = 20;
	static const uint32 kID1 = MKTAG(0,'S','C','V');
	static const uint32 kID2 = MKTAG('M','G','O','B');

	SaveHeader(uint32 type = 0, uint32 version = 0, uint32 size = 0);

	bool operator==(const SaveHeader &header) const;
	bool operator!=(const SaveHeader &header) const;

	/** Read the header out of a stream into this class. */
	bool read(Common::ReadStream &stream);
	/** Read the header out of a stream and checks it against this class's contents. */
	bool verify(Common::ReadStream &stream) const;
	/** Read the header out of a stream and checks it against this class's contents,
	 *  but read the size field instead.
	 */
	bool verifyReadSize(Common::ReadStream &stream);
	/** Write this class's contents into a stream. */
	bool write(Common::WriteStream &stream) const;

	uint32 getType() const;
	uint32 getVersion() const;
	uint32 getSize() const;

	void setType(uint32 type);
	void setVersion(uint32 version);
	void setSize(uint32 size);

private:
	/** An ID specifying the part's type. */
	uint32 _type;
	/** The part's version. */
	uint32 _version;
	/** The size of the contents. */
	uint32 _size;
};

/** An abstract class for a part in a save file. */
class SavePart {
public:
	SavePart();
	virtual ~SavePart();

	/** Return the total size of the part. */
	virtual uint32 getSize() const;

	/** Read the part (with header) out of the stream. */
	virtual bool read(Common::ReadStream &stream) = 0;
	/** Write the part (with header) into the stream. */
	virtual bool write(Common::WriteStream &stream) const = 0;

protected:
	SaveHeader _header;
};

/** A save part consisting of plain memory. */
class SavePartMem : public SavePart {
public:
	static const uint32 kVersion = 1;
	static const uint32 kID = MKTAG('P','M','E','M');

	SavePartMem(uint32 size);
	~SavePartMem();

	bool read(Common::ReadStream &stream);
	bool write(Common::WriteStream &stream) const;

	/** Read size bytes of data into the part at the specified offset. */
	bool readFrom(const byte *data, uint32 offset, uint32 size);
	/** Write size bytes of the part at the specified offset int data. */
	bool writeInto(byte *data, uint32 offset, uint32 size) const;

private:
	uint32 _size;
	byte *_data;
};

/** A save part holding script variables. */
class SavePartVars : public SavePart {
public:
	static const uint32 kVersion = 1;
	static const uint32 kID = MKTAG('V','A','R','S');

	SavePartVars(GobEngine *vm, uint32 size);
	~SavePartVars();

	bool read(Common::ReadStream &stream);
	bool write(Common::WriteStream &stream) const;

	/** Read size bytes of variables starting at var into the part at the specified offset. */
	bool readFrom(uint32 var, uint32 offset, uint32 size);
	/** Write size bytes of the part at the specified offset into the variable starting at var. */
	bool writeInto(uint32 var, uint32 offset, uint32 size) const;

	/** Read size bytes of raw data into the part. */
	bool readFromRaw(const byte *data, uint32 size);

private:
	GobEngine *_vm;

	uint32 _size;
	byte *_data;
};

/** A save part holding a sprite. */
class SavePartSprite : public SavePart {
public:
	static const uint32 kVersion = 2;
	static const uint32 kID = MKTAG('S','P','R','T');

	SavePartSprite(uint32 width, uint32 height, bool trueColor = false);
	~SavePartSprite();

	bool read(Common::ReadStream &stream);
	bool write(Common::WriteStream &stream) const;

	/** Read a palette into the part. */
	bool readPalette(const byte *palette);
	/** Read a sprite into the part. */
	bool readSprite(const Surface &sprite);

	/** Read size bytes of raw data into the sprite. */
	bool readSpriteRaw(const byte *data, uint32 size);

	/** Write a palette out of the part. */
	bool writePalette(byte *palette) const;
	/** Write a sprite out of the part. */
	bool writeSprite(Surface &sprite) const;

private:
	uint32 _width;
	uint32 _height;

	uint32 _spriteSize;

	bool _oldFormat;
	bool _trueColor;

	byte *_dataSprite;
	byte *_dataPalette;
};

/** A save part containing informations about the save's game. */
class SavePartInfo : public SavePart {
public:
	static const uint32 kVersion = 1;
	static const uint32 kID = MKTAG('I','N','F','O');

	/**
	 * The constructor.
	 * @param descMaxLength The maximal number of bytes that fit into the description.
	 * @param gameID An ID for the game (Gob1, Gob2, Gob3, ...).
	 * @param gameVersion An ID for game specific versioning
	 * @param endian Endianness of the platform the game originally ran on.
	 * @param varCount The number of script variables.
	 */
	SavePartInfo(uint32 descMaxLength, uint32 gameID,
			uint32 gameVersion, byte endian, uint32 varCount);
	~SavePartInfo();

	/** Return the save's description. */
	const char *getDesc() const;
	/** Return the description's maximal length. */
	uint32 getDescMaxLength() const;

	/** Set the variable count. */
	void setVarCount(uint32 varCount);
	/** Set the save's description. */
	void setDesc(const char *desc = 0);
	/** Set the save's description. */
	void setDesc(const byte *desc, uint32 size);

	bool read(Common::ReadStream &stream);
	bool write(Common::WriteStream &stream) const;

private:
	char *_desc;
	uint32 _descMaxLength;
	uint32 _gameID;
	uint32 _gameVersion;
	uint32 _varCount;
	byte _endian;
};

/** A container for several save parts. */
class SaveContainer {
public:
	static const uint32 kVersion = 1;
	static const uint32 kID = MKTAG('C','O','N','T');

	/**
	 * The constructor.
	 * @param partCount The number parts this container shall hold.
	 * @param slot The save slot this save's for.
	 */
	SaveContainer(uint32 partCount, uint32 slot);
	~SaveContainer();

	uint32 getSlot() const;
	uint32 getSize() const;

	/** All parts filled? */
	bool hasAllParts() const;

	/** Empty all parts. */
	void clear();

	/** Write a SavePart into the container's part. */
	bool writePart(uint32 partN, const SavePart *part);
	/** Read the container's part's content into a SavePart. */
	bool readPart(uint32 partN, SavePart *part) const;
	/** Read only the container's part's header. */
	bool readPartHeader(uint32 partN, SaveHeader *header) const;

	/** Checks if the stream is a valid save container. */
	static bool isSave(Common::SeekableReadStream &stream);

protected:
	/** A part. */
	struct Part {
		uint32 size;
		byte *data;

		Part(uint32 s);
		~Part();

		Common::WriteStream *createWriteStream();
		Common::ReadStream *createReadStream() const;
	};

	/** Basic information about a part. */
	struct PartInfo {
		uint32 id;
		uint32 offset;
		uint32 size;
	};

	typedef Common::Array<Part *>::iterator PartIterator;
	typedef Common::Array<Part *>::const_iterator PartConstIterator;

	uint32 _partCount;
	uint32 _slot;

	SaveHeader _header;
	Common::Array<Part *> _parts;

	uint32 calcSize() const;

	bool read(Common::ReadStream &stream);
	bool write(Common::WriteStream &stream) const;

	/** Get an array containing basic information about all parts in the container in the stream. */
	static Common::Array<PartInfo> *getPartsInfo(Common::SeekableReadStream &stream);
};

/** Reads a save. */
class SaveReader : public SaveContainer {
public:
	SaveReader(uint32 partCount, uint32 slot, const Common::String &fileName);
	SaveReader(uint32 partCount, uint32 slot, Common::SeekableReadStream &stream);
	~SaveReader();

	bool load();

	bool readPart(uint32 partN, SavePart *part) const;
	bool readPartHeader(uint32 partN, SaveHeader *header) const;

	/** Find and read the save's info part. */
	static bool getInfo(Common::SeekableReadStream &stream, SavePartInfo &info);
	/** Find and read the save's info part. */
	static bool getInfo(const Common::String &fileName, SavePartInfo &info);

protected:
	Common::String _fileName;
	Common::SeekableReadStream *_stream;

	bool _loaded;

	static Common::InSaveFile *openSave(const Common::String &fileName);
	Common::InSaveFile *openSave();
};

/** Writes a save. */
class SaveWriter: public SaveContainer {
public:
	SaveWriter(uint32 partCount, uint32 slot);
	SaveWriter(uint32 partCount, uint32 slot, const Common::String &fileName);
	~SaveWriter();

	bool writePart(uint32 partN, const SavePart *part);

	bool save(Common::WriteStream &stream);

protected:
	bool save();

	Common::String _fileName;

	/** Is everything ready for saving? */
	bool canSave() const;

	static Common::OutSaveFile *openSave(const Common::String &fileName);
	Common::OutSaveFile *openSave();
};

} // End of namespace Gob

#endif // GOB_SAVE_SAVEFILE_H
