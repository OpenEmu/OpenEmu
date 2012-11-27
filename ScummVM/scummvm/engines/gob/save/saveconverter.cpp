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

#include "common/endian.h"
#include "common/memstream.h"
#include "common/savefile.h"

#include "gob/gob.h"
#include "gob/save/saveconverter.h"
#include "gob/save/savefile.h"
#include "gob/save/savehandler.h"

namespace Gob {

SaveConverter::SaveConverter(GobEngine *vm, const Common::String &fileName)
: _vm(vm), _fileName(fileName) {

	_data = 0;
	_stream = 0;
}

SaveConverter::~SaveConverter() {
	delete _stream;
	delete[] _data;
}

void SaveConverter::clear() {
	delete[] _data;
	delete _stream;

	_data = 0;
	_stream = 0;
}

void SaveConverter::setFileName(const Common::String &fileName) {
	clear();
	_fileName = fileName;
}

Common::InSaveFile *SaveConverter::openSave() const {
	if (_fileName.empty())
		return 0;

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	return saveMan->openForLoading(_fileName);
}

void SaveConverter::displayWarning() const {
	warning("Old save format detected, trying to convert. If this does not work, your "
	        "save is broken and can't be used anymore. Sorry for the inconvenience");
}

char *SaveConverter::getDescription(const Common::String &fileName) {
	setFileName(fileName);
	return getDescription();
}

char *SaveConverter::getDescription() const {
	Common::InSaveFile *save;

	// Test if it's an old savd
	if (!isOldSave(&save) || !save)
		return 0;

	char *desc = getDescription(*save);

	delete save;
	return desc;
}

uint32 SaveConverter::getActualSize(Common::InSaveFile **save) const {
	Common::InSaveFile *saveFile = openSave();

	if (!saveFile)
		return false;

	// Is it a valid new save?
	if (SaveContainer::isSave(*saveFile)) {
		delete saveFile;
		return false;
	}

	int32 saveSize = saveFile->size();

	if (saveSize <= 0) {
		delete saveFile;
		return 0;
	}

	if (save)
		*save = saveFile;
	else
		delete saveFile;

	return saveSize;
}

bool SaveConverter::swapDataEndian(byte *data, const byte *sizes, uint32 count) {
	if (!data || !sizes || (count == 0))
		return false;

	while (count-- > 0) {
		if      (*sizes == 3) // 32bit value (3 additional bytes)
			WRITE_UINT32(data, SWAP_BYTES_32(READ_UINT32(data)));
		else if (*sizes == 1) // 16bit value (1 additional byte)
			WRITE_UINT16(data, SWAP_BYTES_16(READ_UINT16(data)));
		else if (*sizes != 0) // else, it has to be an 8bit value
			return false;

		count -= *sizes;
		data  += *sizes + 1;
		sizes += *sizes + 1;
	}

	return true;
}

SavePartInfo *SaveConverter::readInfo(Common::SeekableReadStream &stream,
	uint32 descLength, bool hasSizes) const {

	uint32 varSize = SaveHandler::getVarSize(_vm);
	if (varSize == 0)
		return 0;

	char *desc = getDescription(stream);
	if (!desc)
		return 0;

	// If it has sizes, skip them
	if (hasSizes)
		if (!stream.skip(descLength)) {
			delete[] desc;
			return 0;
		}

	SavePartInfo *info = new SavePartInfo(descLength, (uint32) _vm->getGameType(),
			0, _vm->getEndianness(), varSize);

	info->setDesc(desc);

	delete[] desc;

	return info;
}

byte *SaveConverter::readData(Common::SeekableReadStream &stream,
		uint32 count, bool endian) const {

	byte *data = new byte[count];

	// Read variable data
	if (stream.read(data, count) != count) {
		delete[] data;
		return 0;
	}

	/* Check the endianness. The old save data was always written
	 * as little endian, so we might need to swap the bytes. */

	if (endian && (_vm->getEndianness() == kEndiannessBE)) {
		// Big endian => swapping needed

		// Read variable sizes
		byte *sizes = new byte[count];
		if (stream.read(sizes, count) != count) {
			delete[] data;
			delete[] sizes;
			return 0;
		}

		// Swap bytes
		if (!swapDataEndian(data, sizes, count)) {
			delete[] data;
			delete[] sizes;
			return 0;
		}

		delete[] sizes;

	} else {
		// Little endian => just skip the sizes part

		if (!stream.skip(count)) {
			delete[] data;
			return 0;
		}
	}

	return data;
}

SavePartVars *SaveConverter::readVars(Common::SeekableReadStream &stream,
		uint32 count, bool endian) const {

	byte *data = readData(stream, count, endian);
	if (!data)
		return 0;

	SavePartVars *vars = new SavePartVars(_vm, count);

	// Read variables into part
	if (!vars->readFromRaw(data, count)) {
		delete[] data;
		delete vars;
		return 0;
	}

	delete[] data;
	return vars;
}

SavePartMem *SaveConverter::readMem(Common::SeekableReadStream &stream,
		uint32 count, bool endian) const {

	byte *data = readData(stream, count, endian);
	if (!data)
		return 0;

	SavePartMem *mem = new SavePartMem(count);

	// Read mem into part
	if (!mem->readFrom(data, 0, count)) {
		delete[] data;
		delete mem;
		return 0;
	}

	delete[] data;
	return mem;
}

SavePartSprite *SaveConverter::readSprite(Common::SeekableReadStream &stream,
		uint32 width, uint32 height, bool palette) const {

	assert((width > 0) && (height > 0));

	uint32 spriteSize = width * height;

	byte pal[768];
	if (palette)
		if (stream.read(pal, 768) != 768)
			return 0;

	byte *data = new byte[spriteSize];

	// Read variable data
	if (stream.read(data, spriteSize) != spriteSize) {
		delete[] data;
		return 0;
	}

	SavePartSprite *sprite = new SavePartSprite(width, height);

	if (!sprite->readSpriteRaw(data, spriteSize)) {
		delete[] data;
		return 0;
	}

	delete[] data;

	if (palette)
		if (!sprite->readPalette(pal))
			return 0;

	return sprite;
}

bool SaveConverter::createStream(SaveWriter &writer) {
	// Allocate memory for the internal new save data
	uint32 contSize = writer.getSize();
	_data = new byte[contSize];

	// Save the newly created new save data
	Common::MemoryWriteStream writeStream(_data, contSize);
	if (!writer.save(writeStream))
		return false;

	// Create a reading stream upon that new save data
	_stream = new Common::MemoryReadStream(_data, contSize);

	return true;
}

/* Stream functions. If the new save data stream is available, redirect the stream
 * operations to that stream. Normal stream error behavior if not. */

bool SaveConverter::err() const {
	if (!_data || !_stream)
		return true;

	return _stream->err();
}

void SaveConverter::clearErr() {
	if (!_data || !_stream)
		return;

	_stream->clearErr();
}

bool SaveConverter::eos() const {
	if (!_data || !_stream)
		return true;

	return _stream->eos();
}

uint32 SaveConverter::read(void *dataPtr, uint32 dataSize) {
	if (!_data || !_stream)
		return 0;

	return _stream->read(dataPtr, dataSize);
}

int32 SaveConverter::pos() const {
	if (!_data || !_stream)
		return -1;

	return _stream->pos();
}

int32 SaveConverter::size() const {
	if (!_data || !_stream)
		return -1;

	return _stream->size();
}

bool SaveConverter::seek(int32 offset, int whence) {
	if (!_data || !_stream)
		return false;

	return _stream->seek(offset, whence);
}


SaveConverter_Notes::SaveConverter_Notes(GobEngine *vm, uint32 notesSize,
		const Common::String &fileName) : SaveConverter(vm, fileName) {

	_size = notesSize;
}

SaveConverter_Notes::~SaveConverter_Notes() {
}

int SaveConverter_Notes::isOldSave(Common::InSaveFile **save) const {
	if (_size == 0)
		return 0;

	uint32 saveSize = getActualSize(save);
	if (saveSize == 0)
		return 0;

	// The size of the old save always follows that rule
	if (saveSize == (_size * 2))
		return 1;

	// Not an old save, clean up
	if (save) {
		delete *save;
		*save = 0;
	}

	return 0;
}

char *SaveConverter_Notes::getDescription(Common::SeekableReadStream &save) const  {
	return 0;
}

bool SaveConverter_Notes::loadFail(SavePartVars *vars, Common::InSaveFile *save) {
	delete vars;
	delete save;

	clear();

	return false;
}

// Loads the old save by constructing a new save containing the old save's data
bool SaveConverter_Notes::load() {
	if (_size == 0)
		return false;

	Common::InSaveFile *save;

	// Test if it's an old savd
	if (!isOldSave(&save) || !save)
		return false;

	displayWarning();

	SaveWriter writer(1, 0);

	SavePartVars *vars = readVars(*save, _size, false);
	if (!vars)
		return loadFail(0, save);

	// We don't need the save anymore
	delete save;

	// Write all parts
	if (!writer.writePart(0, vars))
		return loadFail(0, 0);

	// We don't need this anymore
	delete vars;

	// Create the final read stream
	if (!createStream(writer))
		return loadFail(0, 0);

	return true;
}

} // End of namespace Gob
