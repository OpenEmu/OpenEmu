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

#include "gob/gob.h"
#include "gob/save/savehandler.h"
#include "gob/save/savefile.h"
#include "gob/save/saveconverter.h"
#include "gob/global.h"
#include "gob/video.h"
#include "gob/draw.h"
#include "gob/variables.h"
#include "gob/inter.h"

namespace Gob {

SlotFile::SlotFile(GobEngine *vm, uint32 slotCount, const Common::String &base) : _vm(vm) {
	_base = base;
	_slotCount = slotCount;
}

SlotFile::~SlotFile() {
}

uint32 SlotFileIndexed::getSlotMax() const {
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in;

	// Find the last filled save slot and base the save file size calculate on that
	for (int i = (_slotCount - 1); i >= 0; i--) {
		Common::String slotFile = build(i);

		if (slotFile.empty())
			continue;

		in = saveMan->openForLoading(slotFile);

		if (in) {
			delete in;
			return i + 1;
		}
	}

	return 0;
}

int32 SlotFileIndexed::tallyUpFiles(uint32 slotSize, uint32 indexSize) const {
	uint32 maxSlot = getSlotMax();

	if (maxSlot == 0)
		return -1;

	return ((maxSlot * slotSize) + indexSize);
}

void SlotFileIndexed::buildIndex(byte *buffer, SavePartInfo &info,
		SaveConverter *converter, bool setLongest) const {

	uint32 descLength = info.getDescMaxLength();

	uint32 longest     = 0;
	byte  *bufferStart = buffer;

	// Iterate over all files
	for (uint32 i = 0; i < _slotCount; i++, buffer += descLength) {
		Common::String slotFile = build(i);

		if (!slotFile.empty()) {
			char *desc = 0;

			if (converter && (desc = converter->getDescription(slotFile)))
				// Old style save
				memcpy(buffer, desc, descLength);
			else if (SaveReader::getInfo(slotFile, info))
				// New style save
				memcpy(buffer, info.getDesc(), descLength);
			else
				// No known format, fill with 0
				memset(buffer, 0, descLength);

			delete[] desc;

			longest = MAX<uint32>(longest, strlen((const char *) buffer));

		} else
			// No valid slot, fill with 0
			memset(buffer, 0, descLength);
	}

	if (setLongest) {
		uint32 slot0Len;
		for (slot0Len = strlen((const char *) bufferStart); slot0Len < longest; slot0Len++)
			bufferStart[slot0Len] = ' ';
		bufferStart[slot0Len] = '\0';
	}
}

bool SlotFileIndexed::exists(int slot) const {
	Common::InSaveFile *in = openRead(slot);
	bool result = (in != 0);
	delete in;
	return result;
}

Common::InSaveFile *SlotFileIndexed::openRead(int slot) const {
	Common::String name = build(slot);
	if (name.empty())
		return 0;
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *result = saveMan->openForLoading(name);
	return result;
}

Common::OutSaveFile *SlotFileIndexed::openWrite(int slot) const {
	Common::String name = build(slot);
	if (name.empty())
		return 0;
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::OutSaveFile *result = saveMan->openForSaving(name);
	return result;
}


SlotFileIndexed::SlotFileIndexed(GobEngine *vm, uint32 slotCount,
		const Common::String &base, const Common::String &extStub) : SlotFile(vm, slotCount, base) {

	_ext = extStub;
}

SlotFileIndexed::~SlotFileIndexed() {
}

Common::String SlotFileIndexed::build(int slot) const {
	if ((slot < 0) || (((uint32) slot) >= _slotCount))
		return Common::String();

	Common::String buf = Common::String::format("%02d", slot);

	return _base + "." + _ext + buf;
}

SlotFileStatic::SlotFileStatic(GobEngine *vm, const Common::String &base,
		const Common::String &ext) : SlotFile(vm, 1, base) {

	_ext = "." + ext;
}

SlotFileStatic::~SlotFileStatic() {
}

int SlotFileStatic::getSlot(int32 offset) const {
	return -1;
}

int SlotFileStatic::getSlotRemainder(int32 offset) const {
	return -1;
}

Common::String SlotFileStatic::build() const {
	return _base + _ext;
}

bool SlotFileStatic::exists() const {
	Common::InSaveFile *in = openRead();
	bool result = (in != 0);
	delete in;
	return result;
}

Common::InSaveFile *SlotFileStatic::openRead() const {
	Common::String name = build();
	if (name.empty())
		return 0;
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *result = saveMan->openForLoading(name);
	return result;
}

Common::OutSaveFile *SlotFileStatic::openWrite() const {
	Common::String name = build();
	if (name.empty())
		return 0;
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::OutSaveFile *result = saveMan->openForSaving(name);
	return result;
}


SaveHandler::SaveHandler(GobEngine *vm) : _vm(vm) {
}

SaveHandler::~SaveHandler() {
}

uint32 SaveHandler::getVarSize(GobEngine *vm) {
	// Sanity checks
	if (!vm || !vm->_inter || !vm->_inter->_variables)
		return 0;

	return vm->_inter->_variables->getSize();
}

bool SaveHandler::deleteFile() {
	return true;
}


TempSpriteHandler::TempSpriteHandler(GobEngine *vm) : SaveHandler(vm) {
	_sprite = 0;
}

TempSpriteHandler::~TempSpriteHandler() {
	delete _sprite;
}

int32 TempSpriteHandler::getSize() {
	if (!_sprite)
		return -1;

	return _sprite->getSize();
}

bool TempSpriteHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (isDummy(size))
		return true;

	// Sprite available?
	if (!_sprite)
		return false;

	// Sprite requested?
	if (!isSprite(size))
		return false;

	// Index sane?
	int index = getIndex(size);
	if ((index < 0) || (index >= Draw::kSpriteCount))
		return false;

	SurfacePtr sprite = _vm->_draw->_spritesArray[index];

	// Target sprite exists?
	if (!sprite)
		return false;

	// Load the sprite
	if (!_sprite->writeSprite(*sprite))
		return false;

	// Handle palette
	if (usesPalette(size)) {
		if (!_sprite->writePalette((byte *)_vm->_global->_pPaletteDesc->vgaPal))
			return false;

		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
	}

	if (index == 21) {
		// We wrote into the backbuffer, blit
		_vm->_draw->forceBlit();
		_vm->_video->retrace();
	} else if (index == 20)
		// We wrote into the frontbuffer, retrace
		_vm->_video->retrace();

	return true;
}

bool TempSpriteHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (isDummy(size))
		return true;

	SurfacePtr sprite = createSprite(dataVar, size, offset);
	if (!sprite)
		return false;

	// Save the sprite
	if (!_sprite->readSprite(*sprite))
		return false;

	// Handle palette
	if (usesPalette(size))
		if (!_sprite->readPalette((const byte *)_vm->_global->_pPaletteDesc->vgaPal))
			return false;

	return true;
}

bool TempSpriteHandler::create(uint32 width, uint32 height, bool trueColor) {
	delete _sprite;
	_sprite = 0;

	// Create a new temporary sprite
	_sprite = new SavePartSprite(width, height, trueColor);

	return true;
}

bool TempSpriteHandler::createFromSprite(int16 dataVar, int32 size, int32 offset) {
	return createSprite(dataVar, size, offset) != 0;
}

SurfacePtr TempSpriteHandler::createSprite(int16 dataVar, int32 size, int32 offset) {
	SurfacePtr sprt;

	// Sprite requested?
	if (!isSprite(size))
		return sprt;

	// Index sane?
	int index = getIndex(size);
	if ((index < 0) || (index >= Draw::kSpriteCount))
		return sprt;

	// Sprite exists?
	if (!(sprt = _vm->_draw->_spritesArray[index]))
		return sprt;

	if (!create(sprt->getWidth(), sprt->getHeight(), sprt->getBPP() > 1))
		sprt.reset();

	return sprt;
}

// A size of 0 means no proper sprite should be saved/loaded,
// but no error should be thrown either.
bool TempSpriteHandler::isDummy(int32 size) {
	return (size == 0);
}

// A negative size is the flag for using a sprite
bool TempSpriteHandler::isSprite(int32 size) {
	return (size < 0);
}

// Contruct the index
int TempSpriteHandler::getIndex(int32 size) {
	// Palette flag
	if (size < -3000)
		size += 3000;
	if (size < -1000)
		size += 1000;

	return (-size - 1);
}

// A size smaller than -1000 indicates palette usage
bool TempSpriteHandler::usesPalette(int32 size) {
	return (size < -1000);
}


NotesHandler::File::File(GobEngine *vm, const Common::String &base) :
	SlotFileStatic(vm, base, "blo") {
}

NotesHandler::File::~File() {
}

NotesHandler::NotesHandler(uint32 notesSize, GobEngine *vm, const Common::String &target) :
	SaveHandler(vm) {

	_notesSize = notesSize;

	_file = new File(vm, target);

	_notes = new SavePartVars(vm, _notesSize);
}

NotesHandler::~NotesHandler() {
	delete _file;
	delete _notes;
}

int32 NotesHandler::getSize() {
	Common::String fileName = _file->build();

	if (fileName.empty())
		return -1;

	Common::InSaveFile *saveFile;

	SaveConverter_Notes converter(_vm, _notesSize, fileName);
	if (converter.isOldSave(&saveFile)) {
		// Old save, get the size olden-style

		int32 size = saveFile->size();

		delete saveFile;
		return size;
	}

	SaveReader reader(1, 0, fileName);
	SaveHeader header;

	if (!reader.load())
		return -1;

	if (!reader.readPartHeader(0, &header))
		return -1;

	// Return the part's size
	return header.getSize();
}

bool NotesHandler::load(int16 dataVar, int32 size, int32 offset) {
	if ((dataVar < 0) || (size < 0) || (offset < 0))
		return false;

	Common::String fileName = _file->build();

	if (fileName.empty())
		return false;

	SaveReader *reader;

	SaveConverter_Notes converter(_vm, _notesSize, fileName);
	if (converter.isOldSave()) {
		// Old save, plug the converter in
		if (!converter.load())
			return false;

		reader = new SaveReader(1, 0, converter);

	} else
		// New save, load directly
		reader = new SaveReader(1, 0, fileName);

	SavePartVars vars(_vm, _notesSize);

	if (!reader->load()) {
		delete reader;
		return false;
	}

	if (!reader->readPart(0, &vars)) {
		delete reader;
		return false;
	}

	if (!vars.writeInto(dataVar, offset, size)) {
		delete reader;
		return false;
	}

	delete reader;
	return true;
}

bool NotesHandler::save(int16 dataVar, int32 size, int32 offset) {
	if ((dataVar < 0) || (size < 0) || (offset < 0))
		return false;

	Common::String fileName = _file->build();

	if (fileName.empty())
		return false;

	SaveWriter writer(1, 0, fileName);
	SavePartVars vars(_vm, _notesSize);

	if (!vars.readFrom(dataVar, offset, size))
		return false;

	return writer.writePart(0, &vars);
}


FakeFileHandler::FakeFileHandler(GobEngine *vm) : SaveHandler(vm) {
}

FakeFileHandler::~FakeFileHandler() {
}

int32 FakeFileHandler::getSize() {
	if (_data.empty())
		return -1;

	return _data.size();
}

bool FakeFileHandler::load(int16 dataVar, int32 size, int32 offset) {
	if (size <= 0)
		return false;

	if ((uint32)(offset + size) > _data.size())
		return false;

	_vm->_inter->_variables->copyFrom(dataVar, &_data[0] + offset, size);

	return true;
}

bool FakeFileHandler::save(int16 dataVar, int32 size, int32 offset) {
	if (size <= 0)
		return false;

	if ((uint32)(offset + size) > _data.size())
		_data.resize(offset + size);

	_vm->_inter->_variables->copyTo(dataVar, &_data[0] + offset, size);

	return true;
}

bool FakeFileHandler::deleteFile() {
	_data.clear();

	return true;
}

} // End of namespace Gob
