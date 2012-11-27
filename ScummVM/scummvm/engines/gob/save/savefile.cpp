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

#include "common/util.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/savefile.h"

#include "gob/gob.h"
#include "gob/save/savefile.h"
#include "gob/video.h"
#include "gob/inter.h"
#include "gob/variables.h"

namespace Gob {

static inline bool flushStream(Common::WriteStream &stream) {
	// Flush and check for errors

	if (!stream.flush())
		return false;
	if (stream.err())
		return false;

	return true;
}

SaveHeader::SaveHeader(uint32 type, uint32 version, uint32 size) {
	_type = type;
	_version = version;
	_size = size;
}

bool SaveHeader::read(Common::ReadStream &stream) {
	// Read the header and verify the global IDs
	if (stream.readUint32BE() != kID1)
		return false;
	if (stream.readUint32BE() != kID2)
		return false;

	_type = stream.readUint32BE();
	_version = stream.readUint32LE();
	_size = stream.readUint32LE();

	return !stream.err();
}

bool SaveHeader::verify(Common::ReadStream &stream) const {
	// Compare the header with the stream's content

	if (stream.readUint32BE() != kID1)
		return false;
	if (stream.readUint32BE() != kID2)
		return false;
	if (stream.readUint32BE() != _type)
		return false;
	if (stream.readUint32LE() != _version)
		return false;
	if (stream.readUint32LE() != _size)
		return false;

	return !stream.err();
}

bool SaveHeader::operator==(const SaveHeader &header) const {
	return (_type    == header._type)    &&
	       (_version == header._version) &&
	       (_size    == header._size);
}

bool SaveHeader::operator!=(const SaveHeader &header) const {
	return !(*this == header);
}

bool SaveHeader::verifyReadSize(Common::ReadStream &stream) {
	// Compare the header with the stream's content, expect for the size

	if (stream.readUint32BE() != kID1)
		return false;
	if (stream.readUint32BE() != kID2)
		return false;
	if (stream.readUint32BE() != _type)
		return false;
	if (stream.readUint32LE() != _version)
		return false;

	// Read the size out of the stream instead
	_size = stream.readUint32LE();

	return !stream.err();
}

bool SaveHeader::write(Common::WriteStream &stream) const {
	stream.writeUint32BE(kID1);
	stream.writeUint32BE(kID2);
	stream.writeUint32BE(_type);
	stream.writeUint32LE(_version);
	stream.writeUint32LE(_size);

	return flushStream(stream);
}

uint32 SaveHeader::getType() const {
	return _type;
}

uint32 SaveHeader::getVersion() const {
	return _version;
}

uint32 SaveHeader::getSize() const {
	return _size;
}

void SaveHeader::setType(uint32 type) {
	_type = type;
}

void SaveHeader::setVersion(uint32 version) {
	_version = version;
}

void SaveHeader::setSize(uint32 size) {
	_size = size;
}

SavePart::SavePart() {
}

SavePart::~SavePart() {
}

uint32 SavePart::getSize() const {
	// A part's size is the content's size plus the header's size
	return _header.getSize() + SaveHeader::kSize;
}

SavePartMem::SavePartMem(uint32 size) : SavePart(), _size(size) {
	_header.setType(kID);
	_header.setVersion(kVersion);
	_header.setSize(size);

	_data = new byte[size];
}

SavePartMem::~SavePartMem() {
	delete[] _data;
}

bool SavePartMem::read(Common::ReadStream &stream) {
	if (!_header.verify(stream))
		return false;

	if (stream.read(_data, _size) != _size)
		return false;

	return !stream.err();
}

bool SavePartMem::write(Common::WriteStream &stream) const {
	if (!_header.write(stream))
		return false;

	if (stream.write(_data, _size) != _size)
		return false;

	return flushStream(stream);
}

bool SavePartMem::readFrom(const byte *data, uint32 offset, uint32 size) {
	if ((offset + size) > _size)
		return false;

	memcpy(_data + offset, data, size);

	return true;
}

bool SavePartMem::writeInto(byte *data, uint32 offset, uint32 size) const {
	if ((offset + size) > _size)
		return false;

	memcpy(data, _data + offset, size);

	return true;
}

SavePartVars::SavePartVars(GobEngine *vm, uint32 size) : SavePart(), _size(size), _vm(vm) {
	_header.setType(kID);
	_header.setVersion(kVersion);
	_header.setSize(size);

	_data = new byte[size];
}

SavePartVars::~SavePartVars() {
	delete[] _data;
}

bool SavePartVars::read(Common::ReadStream &stream) {
	if (!_header.verify(stream))
		return false;

	if (stream.read(_data, _size) != _size)
		return false;

	return !stream.err();
}

bool SavePartVars::write(Common::WriteStream &stream) const {
	if (!_header.write(stream))
		return false;

	if (stream.write(_data, _size) != _size)
		return false;

	return flushStream(stream);
}

bool SavePartVars::readFrom(uint32 var, uint32 offset, uint32 size) {
	if (!_vm->_inter->_variables)
		return false;

	if ((offset + size) > _size)
		return false;

	// Get raw variables
	return _vm->_inter->_variables->copyTo(var, _data + offset, size);
}

bool SavePartVars::readFromRaw(const byte *data, uint32 size) {
	if (size != _size)
		return false;

	memcpy(_data, data, size);
	return true;
}

bool SavePartVars::writeInto(uint32 var, uint32 offset, uint32 size) const {
	if (!_vm->_inter->_variables)
		return false;

	if ((offset + size) > _size)
		return false;

	// Write raw variables
	if (!_vm->_inter->_variables->copyFrom(var, _data + offset, size))
		return false;

	return true;
}

SavePartSprite::SavePartSprite(uint32 width, uint32 height, bool trueColor) {
	assert((width > 0) && (height > 0));

	_width = width;
	_height = height;

	_oldFormat = false;
	_trueColor = trueColor;

	_header.setType(kID);
	_header.setVersion(kVersion);

	_spriteSize = _width * _height;
	if (_trueColor)
		_spriteSize *= 3;

		//          width + height + color +    sprite   + palette
	_header.setSize(4   +   4    +   1   + _spriteSize + 768);

	_dataSprite  = new byte[_spriteSize];
	_dataPalette = new byte[768];

	memset(_dataSprite,  0, _spriteSize);
	memset(_dataPalette, 0, 768);
}

SavePartSprite::~SavePartSprite() {
	delete[] _dataSprite;
	delete[] _dataPalette;
}

bool SavePartSprite::read(Common::ReadStream &stream) {
	SaveHeader header;
	header.read(stream);

	if (_header != header) {
		if (!_trueColor) {
			// Header validation failed, trying again with the old version

			_header.setVersion(1);
			_header.setSize(_header.getSize() - 1);

			if (_header != header)
				// Nope, isn't it either
				return false;

			_oldFormat = true;

			_header.setVersion(kVersion);
			_header.setSize(_header.getSize() + 1);

		} else
			return false;
	}

	// The sprite's dimensions have to fit
	if (stream.readUint32LE() != _width)
		return false;
	if (stream.readUint32LE() != _height)
		return false;

	// If it's in the current format, the true color flag has to be the same too
	if (!_oldFormat)
		if ((stream.readByte() != 0) != _trueColor)
			return false;

	// Sprite data
	if (stream.read(_dataSprite, _spriteSize) != _spriteSize)
		return false;

	// Palette data
	if (stream.read(_dataPalette, 768) != 768)
		return false;

	return !stream.err();
}

bool SavePartSprite::write(Common::WriteStream &stream) const {
	if (!_header.write(stream))
		return false;

	// The sprite's dimensions
	stream.writeUint32LE(_width);
	stream.writeUint32LE(_height);
	stream.writeByte(_trueColor);

	// Sprite data
	if (stream.write(_dataSprite, _spriteSize) != _spriteSize)
		return false;

	// Palette data
	if (stream.write(_dataPalette, 768) != 768)
		return false;

	return flushStream(stream);
}

bool SavePartSprite::readPalette(const byte *palette) {
	memcpy(_dataPalette, palette, 768);

	return true;
}

bool SavePartSprite::readSprite(const Surface &sprite) {
	// The sprite's dimensions have to fit
	if (((uint32)sprite.getWidth()) != _width)
		return false;
	if (((uint32)sprite.getHeight()) != _height)
		return false;

	if (_trueColor) {
		if (sprite.getBPP() <= 1)
			return false;

		Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();

		byte *data = _dataSprite;
		ConstPixel pixel = sprite.get();
		for (uint32 i = 0; i < (_width * _height); i++, ++pixel, data += 3)
			pixelFormat.colorToRGB(pixel.get(), data[0], data[1], data[2]);

	} else {
		if (sprite.getBPP() != 1)
			return false;

		memcpy(_dataSprite, sprite.getData(), _width * _height);
	}

	return true;
}

bool SavePartSprite::readSpriteRaw(const byte *data, uint32 size) {
	if (size != _spriteSize)
		return false;

	memcpy(_dataSprite, data, size);
	return true;
}

bool SavePartSprite::writePalette(byte *palette) const {
	memcpy(palette, _dataPalette, 768);

	return true;
}

bool SavePartSprite::writeSprite(Surface &sprite) const {
	// The sprite's dimensions have to fit
	if (((uint32)sprite.getWidth()) != _width)
		return false;
	if (((uint32)sprite.getHeight()) != _height)
		return false;

	if (_trueColor) {
		if (sprite.getBPP() <= 1)
			return false;

		Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();

		const byte *data = _dataSprite;
		Pixel pixel = sprite.get();
		for (uint32 i = 0; i < (_width * _height); i++, ++pixel, data += 3)
			pixel.set(pixelFormat.RGBToColor(data[0], data[1], data[2]));

	} else {
		if (sprite.getBPP() != 1)
			return false;

		memcpy(sprite.getData(), _dataSprite, _spriteSize);
	}

	return true;
}

SavePartInfo::SavePartInfo(uint32 descMaxLength, uint32 gameID,
		uint32 gameVersion, byte endian, uint32 varCount) {

	_header.setType(kID);
	_header.setVersion(kVersion);
	//                        descMaxLength + gameID + gameVersion + endian + varCount
	_header.setSize(descMaxLength + 4       +   4    +     4       +    1   +    4);

	_descMaxLength = descMaxLength;
	_gameID = gameID;
	_gameVersion = gameVersion;
	_endian = endian;
	_varCount = varCount;

	_desc = new char[_descMaxLength + 1];
	memset(_desc, 0, _descMaxLength + 1);
}

SavePartInfo::~SavePartInfo() {
	delete[] _desc;
}

const char *SavePartInfo::getDesc() const {
	return _desc;
}

uint32 SavePartInfo::getDescMaxLength() const {
	return _descMaxLength;
}

void SavePartInfo::setVarCount(uint32 varCount) {
	_varCount = varCount;
}

void SavePartInfo::setDesc(const char *desc) {
	if (!desc) {
		memset(_desc, 0, _descMaxLength + 1);
		return;
	}

	uint32 n = MIN<uint32>(strlen(desc), _descMaxLength);

	// Copy the description and fill with 0
	memcpy(_desc, desc, n);
	memset(_desc + n, 0, _descMaxLength + 1 - n);
}

void SavePartInfo::setDesc(const byte *desc, uint32 size) {
	if (!desc || !size) {
		memset(_desc, 0, _descMaxLength + 1);
		return;
	}

	uint32 n = MIN<uint32>(size, _descMaxLength);
	memcpy(_desc, desc, n);
	memset(_desc + n, 0, _descMaxLength + 1 - n);
}

bool SavePartInfo::read(Common::ReadStream &stream) {
	if (!_header.verify(stream))
		return false;

	if (stream.readUint32LE() != _gameID)
		return false;
	if (stream.readUint32LE() != _gameVersion)
		return false;
	if (stream.readByte() != _endian)
		return false;
	if (stream.readUint32LE() != _varCount)
		return false;
	if (stream.readUint32LE() != _descMaxLength)
		return false;

	if (stream.read(_desc, _descMaxLength) != _descMaxLength)
		return false;

	_desc[_descMaxLength] = 0;

	return !stream.err();
}

bool SavePartInfo::write(Common::WriteStream &stream) const {
	if (!_header.write(stream))
		return false;

	stream.writeUint32LE(_gameID);
	stream.writeUint32LE(_gameVersion);
	stream.writeByte(_endian);
	stream.writeUint32LE(_varCount);
	stream.writeUint32LE(_descMaxLength);

	if (stream.write(_desc, _descMaxLength) != _descMaxLength)
		return false;

	return flushStream(stream);
}

SaveContainer::Part::Part(uint32 s) {
	size = s;
	data = new byte[size];
}

SaveContainer::Part::~Part() {
	delete[] data;
}

Common::WriteStream *SaveContainer::Part::createWriteStream() {
	return new Common::MemoryWriteStream(data, size);
}

Common::ReadStream *SaveContainer::Part::createReadStream() const {
	return new Common::MemoryReadStream(data, size);
}

SaveContainer::SaveContainer(uint32 partCount, uint32 slot) {
	assert(partCount > 0);

	_slot = slot;
	_partCount = partCount;

	_parts.resize(partCount);
	for (PartIterator it = _parts.begin(); it != _parts.end(); ++it)
		*it = 0;

	_header.setType(kID);
	_header.setVersion(kVersion);
	_header.setSize(4); // uint32 # of parts
}

SaveContainer::~SaveContainer() {
	clear();
}

uint32 SaveContainer::getSlot() const {
	return _slot;
}

uint32 SaveContainer::getSize() const {
	return _header.getSize() + SaveHeader::kSize;
}

bool SaveContainer::hasAllParts() const {
	for (PartConstIterator it = _parts.begin(); it != _parts.end(); ++it)
		if (!*it)
			return false;

	return true;
}

uint32 SaveContainer::calcSize() const {
	uint32 size = 4; // uint32 # of parts

	for (PartConstIterator it = _parts.begin(); it != _parts.end(); ++it)
		if (*it)
			//              uint32 part size
			size += (*it)->size + 4;

	return size;
}

void SaveContainer::clear() {
	for (PartIterator it = _parts.begin(); it != _parts.end(); ++it) {
		Part *&p = *it;

		delete p;
		p = 0;
	}
}

bool SaveContainer::writePart(uint32 partN, const SavePart *part) {
	// Sanity checks
	if (!part)
		return false;
	if (partN >= _partCount)
		return false;

	Part *&p = _parts[partN];

	delete p;
	// Create the part
	p = new Part(part->getSize());

	Common::WriteStream *pStream = p->createWriteStream();

	// Write
	if (!part->write(*pStream)) {
		delete p;
		p = 0;

		delete pStream;
		return false;
	}

	delete pStream;

	// Update size
	_header.setSize(calcSize());

	return true;
}

bool SaveContainer::readPart(uint32 partN, SavePart *part) const {
	// Sanity checks
	if (!part)
		return false;
	if (partN >= _partCount)
		return false;

	const Part * const &p = _parts[partN];

	// Check if the part actually exists
	if (!p)
		return false;

	Common::ReadStream *pStream = p->createReadStream();

	// Read
	if (!part->read(*pStream)) {
		delete pStream;
		return false;
	}

	delete pStream;
	return true;
}

bool SaveContainer::readPartHeader(uint32 partN, SaveHeader *header) const {
	// Sanity checks
	if (!header)
		return false;
	if (partN >= _partCount)
		return false;

	const Part * const &p = _parts[partN];

	// Check if the part actually exists
	if (!p)
		return false;

	Common::ReadStream *pStream = p->createReadStream();

	// Read
	if (!header->read(*pStream)) {
		delete pStream;
		return false;
	}

	delete pStream;
	return true;
}

bool SaveContainer::read(Common::ReadStream &stream) {
	// Verify the header and get the container's size
	if (!_header.verifyReadSize(stream))
		return false;

	// The part count has to be correct
	if (stream.readUint32LE() != _partCount)
		return false;

	// Iterate over all parts
	for (PartIterator it = _parts.begin(); it != _parts.end(); ++it) {
		// Read the size
		uint32 size = stream.readUint32LE();

		if (stream.err()) {
			clear();
			return false;
		}

		Part *&p = *it;

		delete p;
		// Create a suitable part
		p = new Part(size);
	}

	// Update size
	_header.setSize(calcSize());

	// Iterate over all parts
	for (PartIterator it = _parts.begin(); it != _parts.end(); ++it) {
		Part *&p = *it;

		// Read the part
		if (stream.read(p->data, p->size) != p->size) {
			clear();
			return false;
		}
	}

	return !stream.err();
}

bool SaveContainer::write(Common::WriteStream &stream) const {
	// Write the header
	if (!_header.write(stream))
		return false;

	// Write the part count
	stream.writeUint32LE(_partCount);

	// Iterate over all parts
	for (PartConstIterator it = _parts.begin(); it != _parts.end(); ++it) {
		// Part doesn't actually exist => error
		if (!*it)
			return false;

		// Write the part's size
		stream.writeUint32LE((*it)->size);
	}

	if (!flushStream(stream))
		return false;

	// Iterate over all parts
	for (PartConstIterator it = _parts.begin(); it != _parts.end(); ++it) {
		Part * const &p = *it;

		// Write the part
		if (stream.write(p->data, p->size) != p->size)
			return false;
	}

	return flushStream(stream);
}

Common::Array<SaveContainer::PartInfo> *SaveContainer::getPartsInfo(Common::SeekableReadStream &stream) {
	Common::Array<PartInfo> *parts = 0;

	// Remember the stream's position to seek back to
	uint32 startPos = stream.pos();

	SaveHeader header;

	header.setType(kID);
	header.setVersion(kVersion);

	// Verify the header
	if (!header.verifyReadSize(stream)) {
		// Seek back
		stream.seek(startPos);
		return 0;
	}

	// Read the part count
	uint32 partCount = stream.readUint32LE();

	// Create a part information array
	parts = new Common::Array<PartInfo>;
	parts->resize(partCount);

	// Read all part sizes
	for (uint32 i = 0; i < partCount; i++)
		(*parts)[i].size = stream.readUint32LE();

	// Iterate over all parts
	for (uint32 i = 0; i < partCount; i++) {
		// The part's offset (from the starting point of the stream)
		(*parts)[i].offset = stream.pos() - startPos;

		SaveHeader partHeader;

		// Read the header
		if (!partHeader.read(stream)) {
			// Seek back
			stream.seek(startPos);
			delete parts;
			return 0;
		}

		// Fill in the ID
		(*parts)[i].id = partHeader.getType();

		// Skip the part's content
		stream.skip(partHeader.getSize());
	}

	if (stream.err()) {
		delete parts;
		parts = 0;
	}

	// Seek back
	stream.seek(startPos);

	return parts;
}

bool SaveContainer::isSave(Common::SeekableReadStream &stream) {
	// Remember the stream's position to seek back to
	uint32 startPos = stream.pos();

	SaveHeader header;

	header.setType(kID);
	header.setVersion(kVersion);

	bool result = header.verifyReadSize(stream);

	// Seek back
	stream.seek(startPos);

	return result;
}


SaveReader::SaveReader(uint32 partCount, uint32 slot, const Common::String &fileName) :
	SaveContainer(partCount, slot), _fileName(fileName) {

	_stream = 0;

	_loaded = false;
}

SaveReader::SaveReader(uint32 partCount, uint32 slot, Common::SeekableReadStream &stream) :
	SaveContainer(partCount, slot) {

	_stream = &stream;

	_loaded = false;
}

SaveReader::~SaveReader() {
}

// Open the save and read it
bool SaveReader::load() {

	Common::InSaveFile *in = 0;
	Common::SeekableReadStream *stream;

	if (!_fileName.empty()) {
		in = openSave();

		if (!in)
			return false;

		stream = in;
	} else if (_stream)
		stream = _stream;
	else
		return false;

	if (!SaveContainer::read(*stream)) {
		delete in;
		return false;
	}

	delete in;
	_loaded = true;
	return true;
}

bool SaveReader::readPartHeader(uint32 partN, SaveHeader *header) const {
	// The save has to be loaded
	if (!_loaded)
		return false;

	return SaveContainer::readPartHeader(partN, header);
}

bool SaveReader::readPart(uint32 partN, SavePart *part) const {
	// The save has to be loaded
	if (!_loaded)
		return false;

	if (!SaveContainer::readPart(partN, part))
		return false;

	return true;
}

Common::InSaveFile *SaveReader::openSave(const Common::String &fileName) {
	if (fileName.empty())
		return 0;

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	return saveMan->openForLoading(fileName);
}

Common::InSaveFile *SaveReader::openSave() {
	return openSave(_fileName);
}

bool SaveReader::getInfo(Common::SeekableReadStream &stream, SavePartInfo &info) {
	// Remeber the stream's starting position to seek back to
	uint32 startPos = stream.pos();

	// Get parts' basic information
	Common::Array<SaveContainer::PartInfo> *partsInfo = getPartsInfo(stream);

	// No parts => fail
	if (!partsInfo) {
		stream.seek(startPos);
		return false;
	}

	bool result = false;
	// Iterate over all parts
	for (Common::Array<SaveContainer::PartInfo>::iterator it = partsInfo->begin();
	     it != partsInfo->end(); ++it) {

		// Check for the info part
		if (it->id == SavePartInfo::kID) {
			if (!stream.seek(it->offset))
				break;

			// Read it
			result = info.read(stream);
			break;
		}
	}

	stream.seek(startPos);

	delete partsInfo;
	return result;
}

bool SaveReader::getInfo(const Common::String &fileName, SavePartInfo &info) {
	Common::InSaveFile *in = openSave(fileName);

	if (!in)
		return false;

	bool result = getInfo(*in, info);

	delete in;

	return result;
}

SaveWriter::SaveWriter(uint32 partCount, uint32 slot) :
	SaveContainer(partCount, slot) {
}

SaveWriter::SaveWriter(uint32 partCount, uint32 slot, const Common::String &fileName) :
	SaveContainer(partCount, slot), _fileName(fileName) {
}

SaveWriter::~SaveWriter() {
}

bool SaveWriter::writePart(uint32 partN, const SavePart *part) {
	// Write the part
	if (!SaveContainer::writePart(partN, part))
		return false;

	// If all parts have been written, save and clear
	if (hasAllParts() && canSave()) {
		if (save()) {
			clear();
			return true;
		}

		return false;
	}

	return true;
}

bool SaveWriter::save(Common::WriteStream &stream) {
	return SaveContainer::write(stream);
}

bool SaveWriter::save() {
	Common::OutSaveFile *out = openSave();

	if (!out)
		return false;

	bool success = save(*out);

	delete out;

	return success;
}

bool SaveWriter::canSave() const {
	return (!_fileName.empty());
}

Common::OutSaveFile *SaveWriter::openSave(const Common::String &fileName) {
	if (fileName.empty())
		return 0;

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	return saveMan->openForSaving(fileName);
}

Common::OutSaveFile *SaveWriter::openSave() {
	return openSave(_fileName);
}

} // End of namespace Gob
