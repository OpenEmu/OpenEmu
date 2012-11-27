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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/debug.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/config-manager.h"

#include "graphics/surface.h"
#include "graphics/decoders/pcx.h"
#include "graphics/thumbnail.h"

#include "gui/saveload.h"

#include "hugo/hugo.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/util.h"
#include "hugo/object.h"
#include "hugo/text.h"
#include "hugo/mouse.h"

namespace Hugo {

namespace {
static const char s_bootCypher[] = "Copyright 1992, David P Gray, Gray Design Associates";
static const int s_bootCypherLen = sizeof(s_bootCypher) - 1;
}


FileManager::FileManager(HugoEngine *vm) : _vm(vm) {
	_hasReadHeader = false;
	_firstUIFFl = true;
}

FileManager::~FileManager() {
}

/**
 * Name scenery and objects picture databases
 */
const char *FileManager::getBootFilename() const {
	return "HUGO.BSF";
}

const char *FileManager::getObjectFilename() const {
	return "objects.dat";
}

const char *FileManager::getSceneryFilename() const {
	return "scenery.dat";
}

const char *FileManager::getSoundFilename() const {
	return "sounds.dat";
}

const char *FileManager::getStringFilename() const {
	return "strings.dat";
}

const char *FileManager::getUifFilename() const {
	return "uif.dat";
}

/**
 * Read a pcx file of length len.  Use supplied seqPtr and image_p or
 * allocate space if NULL.  Name used for errors.  Returns address of seqPtr
 * Set first TRUE to initialize b_index (i.e. not reading a sequential image in file).
 */
Seq *FileManager::readPCX(Common::SeekableReadStream &f, Seq *seqPtr, byte *imagePtr, const bool firstFl, const char *name) {
	debugC(1, kDebugFile, "readPCX(..., %s)", name);

	// Allocate memory for Seq if 0
	if (seqPtr == 0) {
		if ((seqPtr = (Seq *)malloc(sizeof(Seq))) == 0)
			error("Insufficient memory to run game.");
	}

	Graphics::PCXDecoder pcx;
	if (!pcx.loadStream(f))
		error("Error while reading PCX image");

	const Graphics::Surface *pcxSurface = pcx.getSurface();
	if (pcxSurface->format.bytesPerPixel != 1)
		error("Invalid bytes per pixel in PCX surface (%d)", pcxSurface->format.bytesPerPixel);

	// Find size of image data in 8-bit DIB format
	// Note save of x2 - marks end of valid data before garbage
	seqPtr->_lines = pcxSurface->h;
	seqPtr->_x2 = seqPtr->_bytesPerLine8 = pcxSurface->w;
	// Size of the image
	uint16 size = pcxSurface->w * pcxSurface->h;

	// Allocate memory for image data if NULL
	if (imagePtr == 0)
		imagePtr = (byte *)malloc((size_t) size);

	assert(imagePtr);

	seqPtr->_imagePtr = imagePtr;
	for (uint16 y = 0; y < pcxSurface->h; y++)
		memcpy(imagePtr + y * pcxSurface->w, pcxSurface->getBasePtr(0, y), pcxSurface->w);

	return seqPtr;
}

/**
 * Read object file of PCC images into object supplied
 */
void FileManager::readImage(const int objNum, Object *objPtr) {
	debugC(1, kDebugFile, "readImage(%d, Object *objPtr)", objNum);

	/**
	 * Structure of object file lookup entry
	 */
	struct objBlock_t {
		uint32 objOffset;
		uint32 objLength;
	};

	if (!objPtr->_seqNumb)                           // This object has no images
		return;

	if (_vm->isPacked()) {
		_objectsArchive.seek((uint32)objNum * sizeof(objBlock_t), SEEK_SET);

		objBlock_t objBlock;                        // Info on file within database
		objBlock.objOffset = _objectsArchive.readUint32LE();
		objBlock.objLength = _objectsArchive.readUint32LE();

		_objectsArchive.seek(objBlock.objOffset, SEEK_SET);
	} else {
		Common::String buf;
		buf = _vm->_picDir + Common::String(_vm->_text->getNoun(objPtr->_nounIndex, 0)) + ".PIX";
		if (!_objectsArchive.open(buf)) {
			buf = Common::String(_vm->_text->getNoun(objPtr->_nounIndex, 0)) + ".PIX";
			if (!_objectsArchive.open(buf))
				error("File not found: %s", buf.c_str());
		}
	}

	bool  firstImgFl = true;                        // Initializes pcx read function
	Seq *seqPtr = 0;                              // Ptr to sequence structure

	// Now read the images into an images list
	for (int j = 0; j < objPtr->_seqNumb; j++) {     // for each sequence
		for (int k = 0; k < objPtr->_seqList[j]._imageNbr; k++) { // each image
			if (k == 0) {                           // First image
				// Read this image - allocate both seq and image memory
				seqPtr = readPCX(_objectsArchive, 0, 0, firstImgFl, _vm->_text->getNoun(objPtr->_nounIndex, 0));
				objPtr->_seqList[j]._seqPtr = seqPtr;
				firstImgFl = false;
			} else {                                // Subsequent image
				// Read this image - allocate both seq and image memory
				seqPtr->_nextSeqPtr = readPCX(_objectsArchive, 0, 0, firstImgFl, _vm->_text->getNoun(objPtr->_nounIndex, 0));
				seqPtr = seqPtr->_nextSeqPtr;
			}

			// Compute the bounding box - x1, x2, y1, y2
			// Note use of x2 - marks end of valid data in row
			uint16 x2 = seqPtr->_x2;
			seqPtr->_x1 = seqPtr->_x2;
			seqPtr->_x2 = 0;
			seqPtr->_y1 = seqPtr->_lines;
			seqPtr->_y2 = 0;

			ImagePtr dibPtr = seqPtr->_imagePtr;
			for (int y = 0; y < seqPtr->_lines; y++, dibPtr += seqPtr->_bytesPerLine8 - x2) {
				for (int x = 0; x < x2; x++) {
					if (*dibPtr++) {                // Some data found
						if (x < seqPtr->_x1)
							seqPtr->_x1 = x;
						if (x > seqPtr->_x2)
							seqPtr->_x2 = x;
						if (y < seqPtr->_y1)
							seqPtr->_y1 = y;
						if (y > seqPtr->_y2)
							seqPtr->_y2 = y;
					}
				}
			}
		}
		assert(seqPtr);
		seqPtr->_nextSeqPtr = objPtr->_seqList[j]._seqPtr; // loop linked list to head
	}

	// Set the current image sequence to first or last
	switch (objPtr->_cycling) {
	case kCycleInvisible:                           // (May become visible later)
	case kCycleAlmostInvisible:
	case kCycleNotCycling:
	case kCycleForward:
		objPtr->_currImagePtr = objPtr->_seqList[0]._seqPtr;
		break;
	case kCycleBackward:
		objPtr->_currImagePtr = seqPtr;
		break;
	default:
		warning("Unexpected cycling: %d", objPtr->_cycling);
	}

	if (!_vm->isPacked())
		_objectsArchive.close();
}

/**
 * Read sound (or music) file data.  Call with SILENCE to free-up
 * any allocated memory.  Also returns size of data
 */
SoundPtr FileManager::getSound(const int16 sound, uint16 *size) {
	debugC(1, kDebugFile, "getSound(%d)", sound);

	// No more to do if SILENCE (called for cleanup purposes)
	if (sound == _vm->_soundSilence)
		return 0;

	// Open sounds file
	Common::File fp;                                // Handle to SOUND_FILE
	if (!fp.open(getSoundFilename())) {
		warning("Hugo Error: File not found %s", getSoundFilename());
		return 0;
	}

	if (!_hasReadHeader) {
		for (int i = 0; i < kMaxSounds; i++) {
			_soundHdr[i]._size = fp.readUint16LE();
			_soundHdr[i]._offset = fp.readUint32LE();
		}
		if (fp.err())
			error("Wrong sound file format");
		_hasReadHeader = true;
	}

	*size = _soundHdr[sound]._size;
	if (*size == 0)
		error("Wrong sound file format or missing sound %d", sound);

	// Allocate memory for sound or music, if possible
	SoundPtr soundPtr = (byte *)malloc(_soundHdr[sound]._size); // Ptr to sound data
	assert(soundPtr);

	// Seek to data and read it
	fp.seek(_soundHdr[sound]._offset, SEEK_SET);
	if (fp.read(soundPtr, _soundHdr[sound]._size) != _soundHdr[sound]._size)
		error("Wrong sound file format");

	fp.close();

	return soundPtr;
}

/**
 * Save game to supplied slot
 */
bool FileManager::saveGame(const int16 slot, const Common::String &descrip) {
	debugC(1, kDebugFile, "saveGame(%d, %s)", slot, descrip.c_str());

	int16 savegameId;
	Common::String savegameDescription;

	if (slot == -1) {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser("Save game:", "Save", true);
		savegameId = dialog->runModalWithCurrentTarget();
		savegameDescription = dialog->getResultString();
		delete dialog;
	} else {
		savegameId = slot;
		if (!descrip.empty()) {
			savegameDescription = descrip;
		} else {
			savegameDescription = Common::String::format("Quick save #%d", slot);
		}
	}

	if (savegameId < 0)                             // dialog aborted
		return false;

	Common::String savegameFile = _vm->getSavegameFilename(savegameId);
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::OutSaveFile *out = saveMan->openForSaving(savegameFile);

	if (!out) {
		warning("Can't create file '%s', game not saved", savegameFile.c_str());
		return false;
	}

	// Write version.  We can't restore from obsolete versions
	out->writeByte(kSavegameVersion);

	if (savegameDescription == "") {
		savegameDescription = "Untitled savegame";
	}

	out->writeSint16BE(savegameDescription.size() + 1);
	out->write(savegameDescription.c_str(), savegameDescription.size() + 1);

	Graphics::saveThumbnail(*out);

	TimeDate curTime;
	_vm->_system->getTimeAndDate(curTime);

	uint32 saveDate = (curTime.tm_mday & 0xFF) << 24 | ((curTime.tm_mon + 1) & 0xFF) << 16 | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = (curTime.tm_hour & 0xFF) << 8 | ((curTime.tm_min) & 0xFF);

	out->writeUint32BE(saveDate);
	out->writeUint16BE(saveTime);

	_vm->_object->saveObjects(out);

	const Status &gameStatus = _vm->getGameStatus();

	// Save whether hero image is swapped
	out->writeByte(_vm->_heroImage);

	// Save score
	out->writeSint16BE(_vm->getScore());

	// Save story mode
	out->writeByte((gameStatus._storyModeFl) ? 1 : 0);

	// Save jumpexit mode
	out->writeByte((_vm->_mouse->getJumpExitFl()) ? 1 : 0);

	// Save gameover status
	out->writeByte((gameStatus._gameOverFl) ? 1 : 0);

	// Save screen states
	for (int i = 0; i < _vm->_numStates; i++)
		out->writeByte(_vm->_screenStates[i]);

	_vm->_scheduler->saveSchedulerData(out);
	// Save palette table
	_vm->_screen->savePal(out);

	// Save maze status
	out->writeByte((_vm->_maze._enabledFl) ? 1 : 0);
	out->writeByte(_vm->_maze._size);
	out->writeSint16BE(_vm->_maze._x1);
	out->writeSint16BE(_vm->_maze._y1);
	out->writeSint16BE(_vm->_maze._x2);
	out->writeSint16BE(_vm->_maze._y2);
	out->writeSint16BE(_vm->_maze._x3);
	out->writeSint16BE(_vm->_maze._x4);
	out->writeByte(_vm->_maze._firstScreenIndex);

	out->writeByte((byte)_vm->getGameStatus()._viewState);

	out->finalize();

	delete out;

	return true;
}

/**
 * Restore game from supplied slot number
 */
bool FileManager::restoreGame(const int16 slot) {
	debugC(1, kDebugFile, "restoreGame(%d)", slot);

	int16 savegameId;

	if (slot == -1) {
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser("Restore game:", "Restore", false);
		savegameId = dialog->runModalWithCurrentTarget();
		delete dialog;
	} else {
		savegameId = slot;
	}

	if (savegameId < 0)                             // dialog aborted
		return false;

	Common::String savegameFile = _vm->getSavegameFilename(savegameId);
	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *in = saveMan->openForLoading(savegameFile);

	if (!in)
		return false;

	// Initialize new-game status
	_vm->initStatus();

	// Check version, can't restore from different versions
	int saveVersion = in->readByte();
	if (saveVersion != kSavegameVersion) {
		warning("Savegame of incompatible version");
		delete in;
		return false;
	}

	// Skip over description
	int32 saveGameNameSize = in->readSint16BE();
	in->skip(saveGameNameSize);

	Graphics::skipThumbnail(*in);

	in->skip(6);                                    // Skip date & time

	// If hero image is currently swapped, swap it back before restore
	if (_vm->_heroImage != kHeroIndex)
		_vm->_object->swapImages(kHeroIndex, _vm->_heroImage);

	_vm->_object->restoreObjects(in);

	_vm->_heroImage = in->readByte();

	// If hero swapped in saved game, swap it
	byte heroImg = _vm->_heroImage;
	if (heroImg != kHeroIndex)
		_vm->_object->swapImages(kHeroIndex, _vm->_heroImage);
	_vm->_heroImage = heroImg;

	Status &gameStatus = _vm->getGameStatus();

	int score = in->readSint16BE();
	_vm->setScore(score);

	gameStatus._storyModeFl = (in->readByte() == 1);
	_vm->_mouse->setJumpExitFl(in->readByte() == 1);
	gameStatus._gameOverFl = (in->readByte() == 1);
	for (int i = 0; i < _vm->_numStates; i++)
		_vm->_screenStates[i] = in->readByte();

	_vm->_scheduler->restoreSchedulerData(in);

	// Restore palette and change it if necessary
	_vm->_screen->restorePal(in);

	// Restore maze status
	_vm->_maze._enabledFl = (in->readByte() == 1);
	_vm->_maze._size = in->readByte();
	_vm->_maze._x1 = in->readSint16BE();
	_vm->_maze._y1 = in->readSint16BE();
	_vm->_maze._x2 = in->readSint16BE();
	_vm->_maze._y2 = in->readSint16BE();
	_vm->_maze._x3 = in->readSint16BE();
	_vm->_maze._x4 = in->readSint16BE();
	_vm->_maze._firstScreenIndex = in->readByte();

	_vm->_scheduler->restoreScreen(*_vm->_screenPtr);
	if ((_vm->getGameStatus()._viewState = (Vstate) in->readByte()) != kViewPlay)
		_vm->_screen->hideCursor();


	delete in;
	return true;
}

/**
 * Reads boot file for program environment.  Fatal error if not there or
 * file checksum is bad.  De-crypts structure while checking checksum
 */
void FileManager::readBootFile() {
	debugC(1, kDebugFile, "readBootFile()");

	Common::File ofp;
	if (!ofp.open(getBootFilename())) {
		if (_vm->_gameVariant == kGameVariantH1Dos) {
			//TODO initialize properly _boot structure
			warning("readBootFile - Skipping as H1 Dos may be a freeware");
			memset(_vm->_boot._distrib, '\0', sizeof(_vm->_boot._distrib));
			_vm->_boot._registered = kRegFreeware;
			return;
		} else if (_vm->getPlatform() == Common::kPlatformPC) {
			warning("readBootFile - Skipping as H2 and H3 Dos may be shareware");
			memset(_vm->_boot._distrib, '\0', sizeof(_vm->_boot._distrib));
			_vm->_boot._registered = kRegShareware;
			return;
		} else {
			Utils::notifyBox(Common::String::format("Missing startup file '%s'", getBootFilename()));
			_vm->getGameStatus()._doQuitFl = true;
			return;
		}
	}

	if (ofp.size() < (int32)sizeof(_vm->_boot)) {
		Utils::notifyBox(Common::String::format("Corrupted startup file '%s'", getBootFilename()));
		_vm->getGameStatus()._doQuitFl = true;
		return;
	}

	_vm->_boot._checksum = ofp.readByte();
	_vm->_boot._registered = ofp.readByte();
	ofp.read(_vm->_boot._pbswitch, sizeof(_vm->_boot._pbswitch));
	ofp.read(_vm->_boot._distrib, sizeof(_vm->_boot._distrib));
	_vm->_boot._exitLen = ofp.readUint16LE();

	byte *p = (byte *)&_vm->_boot;

	byte checksum = 0;
	for (uint32 i = 0; i < sizeof(_vm->_boot); i++) {
		checksum ^= p[i];
		p[i] ^= s_bootCypher[i % s_bootCypherLen];
	}
	ofp.close();

	if (checksum) {
		Utils::notifyBox(Common::String::format("Corrupted startup file '%s'", getBootFilename()));
		_vm->getGameStatus()._doQuitFl = true;
	}
}

/**
 * Returns address of uif_hdr[id], reading it in if first call
 * This file contains, between others, the bitmaps of the fonts used in the application
 * UIF means User interface database (Windows Only)
 */
UifHdr *FileManager::getUIFHeader(const Uif id) {
	debugC(1, kDebugFile, "getUIFHeader(%d)", id);

	// Initialize offset lookup if not read yet
	if (_firstUIFFl) {
		_firstUIFFl = false;
		// Open unbuffered to do far read
		Common::File ip;                            // Image data file
		if (!ip.open(getUifFilename()))
			error("File not found: %s", getUifFilename());

		if (ip.size() < (int32)sizeof(_UIFHeader))
			error("Wrong UIF file format");

		for (int i = 0; i < kMaxUifs; ++i) {
			_UIFHeader[i]._size = ip.readUint16LE();
			_UIFHeader[i]._offset = ip.readUint32LE();
		}

		ip.close();
	}
	return &_UIFHeader[id];
}

/**
 * Read uif item into supplied buffer.
 */
void FileManager::readUIFItem(const int16 id, byte *buf) {
	debugC(1, kDebugFile, "readUIFItem(%d, ...)", id);

	// Open uif file to read data
	Common::File ip;                                // UIF_FILE handle
	if (!ip.open(getUifFilename()))
		error("File not found: %s", getUifFilename());

	// Seek to data
	UifHdr *_UIFHeaderPtr = getUIFHeader((Uif)id);
	ip.seek(_UIFHeaderPtr->_offset, SEEK_SET);

	// We support pcx images and straight data
	Seq *dummySeq;                                // Dummy Seq for image data
	switch (id) {
	case UIF_IMAGES:                                // Read uif images file
		dummySeq = readPCX(ip, 0, buf, true, getUifFilename());
		free(dummySeq);
		break;
	default:                                        // Read file data into supplied array
		if (ip.read(buf, _UIFHeaderPtr->_size) != _UIFHeaderPtr->_size)
			error("Wrong UIF file format");
		break;
	}

	ip.close();
}

/**
 * Read the uif image file (inventory icons)
 */
void FileManager::readUIFImages() {
	debugC(1, kDebugFile, "readUIFImages()");

	readUIFItem(UIF_IMAGES, _vm->_screen->getGUIBuffer());   // Read all uif images
}

} // End of namespace Hugo
