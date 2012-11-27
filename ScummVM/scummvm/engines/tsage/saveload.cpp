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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/savefile.h"
#include "common/mutex.h"
#include "graphics/palette.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "tsage/globals.h"
#include "tsage/saveload.h"
#include "tsage/sound.h"
#include "tsage/tsage.h"

namespace TsAGE {

Saver *g_saver;

SavedObject::SavedObject() {
	g_saver->addObject(this);
}

SavedObject::~SavedObject() {
	g_saver->removeObject(this);
}

/*--------------------------------------------------------------------------*/

Saver::Saver() {
	_macroSaveFlag = false;
	_macroRestoreFlag = false;
}

Saver::~Saver() {
	// Internal validation that no saved object is still present
	int totalLost = 0;
	for (SynchronizedList<SavedObject *>::iterator i = g_saver->_objList.begin(); i != g_saver->_objList.end(); ++i) {
		SavedObject *so = *i;
		if (so)
			++totalLost;
	}

	if (totalLost)
		warning("Saved object not destroyed");
}

/*--------------------------------------------------------------------------*/

void Serializer::syncPointer(SavedObject **ptr, Common::Serializer::Version minVersion,
		Common::Serializer::Version maxVersion) {
	int idx = 0;
	assert(ptr);

	if (isSaving()) {
		// Get the object index for the given pointer and write it out
		if (*ptr) {
			idx = g_saver->blockIndexOf(*ptr);
			assert(idx > 0);
		}
		syncAsUint32LE(idx);
	} else {
		// Load in the object index and add it into the unresolved pointer list
		syncAsUint32LE(idx);
		*ptr = NULL;
		if (idx > 0)
			// For non-zero (null) pointers, create a record for later resolving it to an address
			g_saver->addSavedObjectPtr(ptr, idx);
	}
}

void Serializer::validate(const Common::String &s, Common::Serializer::Version minVersion,
		Common::Serializer::Version maxVersion) {
	Common::String tempStr = s;
	syncString(tempStr, minVersion, maxVersion);

	if (isLoading() && (tempStr != s))
		error("Savegame is corrupt");
}

void Serializer::validate(int v, Common::Serializer::Version minVersion,
		Common::Serializer::Version maxVersion) {
	int tempVal = v;
	syncAsUint32LE(tempVal, minVersion, maxVersion);
	if (isLoading() && (tempVal != v))
		error("Savegame is corrupt");
}

#define DOUBLE_PRECISION 1000000000

void Serializer::syncAsDouble(double &v) {
	int32 num = (int32)(v);
	uint32 fraction = (uint32)((v - (int32)v) * DOUBLE_PRECISION);

	syncAsSint32LE(num);
	syncAsUint32LE(fraction);

	if (isLoading())
		v = num + (double)fraction / DOUBLE_PRECISION;
}

/*--------------------------------------------------------------------------*/

Common::Error Saver::save(int slot, const Common::String &saveName) {
	assert(!getMacroRestoreFlag());
	Common::StackLock slock1(g_globals->_soundManager._serverDisabledMutex);

	// Signal any objects registered for notification
	_saveNotifiers.notify(false);

	// Set fields
	_macroSaveFlag = true;
	_saveSlot = slot;

	// Try and create the save file
	Common::OutSaveFile *saveFile = g_system->getSavefileManager()->openForSaving(g_vm->generateSaveName(slot));
	if (!saveFile)
		return Common::kCreatingFileFailed;

	// Set up the serializer
	Serializer serializer(NULL, saveFile);
	serializer.setSaveVersion(TSAGE_SAVEGAME_VERSION);

	// Write out the savegame header
	tSageSavegameHeader header;
	header.saveName = saveName;
	header.version = TSAGE_SAVEGAME_VERSION;
	writeSavegameHeader(saveFile, header);

	// Save out objects that need to come at the start of the savegame
	for (SynchronizedList<SaveListener *>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->listenerSynchronize(serializer);
	}

	// Save each registered SaveObject descendant object into the savegame file
	for (SynchronizedList<SavedObject *>::iterator i = _objList.begin(); i != _objList.end(); ++i) {
		serializer.validate((*i)->getClassName());
		(*i)->synchronize(serializer);
	}

	// Save file complete
	saveFile->writeString("END");
	saveFile->finalize();
	delete saveFile;

	// Final post-save notification
	_macroSaveFlag = false;
	_saveNotifiers.notify(true);

	return Common::kNoError;
}

Common::Error Saver::restore(int slot) {
	assert(!getMacroRestoreFlag());
	Common::StackLock slock1(g_globals->_soundManager._serverDisabledMutex);

	// Signal any objects registered for notification
	_loadNotifiers.notify(false);

	// Set fields
	_macroRestoreFlag = true;
	_saveSlot = slot;
	_unresolvedPtrs.clear();

	// Set up the serializer
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(g_vm->generateSaveName(slot));
	if (!saveFile)
		return Common::kReadingFailed;

	Serializer serializer(saveFile, NULL);

	// Read in the savegame header
	tSageSavegameHeader header;
	readSavegameHeader(saveFile, header);
	if (header.thumbnail)
		header.thumbnail->free();
	delete header.thumbnail;

	serializer.setSaveVersion(header.version);

	// Load in data for objects that need to come at the start of the savegame
	for (Common::List<SaveListener *>::iterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		(*i)->listenerSynchronize(serializer);
	}

	// Loop through each registered object to load in the data
	for (SynchronizedList<SavedObject *>::iterator i = _objList.begin(); i != _objList.end(); ++i) {
		serializer.validate((*i)->getClassName());
		(*i)->synchronize(serializer);
	}

	// Loop through the remaining data of the file, instantiating new objects.
	// Note: I don't store pointers to instantiated objects here, because it's not necessary - the mere act
	// of instantiating a saved object registers it with the saver, and will then be resolved to whatever
	// object originally had a pointer to it as part of the post-processing step
	Common::String className;
	serializer.syncString(className);
	while (className != "END") {
		SavedObject *savedObject;
		if (!_factoryPtr || ((savedObject = _factoryPtr(className)) == NULL))
			error("Unknown class name '%s' encountered trying to restore savegame", className.c_str());

		// Populate the contents of the object
		savedObject->synchronize(serializer);

		// Move to next object
		serializer.syncString(className);
	}

	// Post-process any unresolved pointers to get the correct pointer
	resolveLoadPointers();

	delete saveFile;

	// Final post-restore notifications
	_macroRestoreFlag = false;
	_loadNotifiers.notify(true);

	return Common::kNoError;
}

const char *SAVEGAME_STR = "SCUMMVM_TSAGE";
#define SAVEGAME_STR_SIZE 13

bool Saver::readSavegameHeader(Common::InSaveFile *in, tSageSavegameHeader &header) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];
	header.thumbnail = NULL;

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header.version = in->readByte();
	if (header.version > TSAGE_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header.saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0') header.saveName += ch;

	// Get the thumbnail
	header.thumbnail = Graphics::loadThumbnail(*in);
	if (!header.thumbnail)
		return false;

	// Read in save date/time
	header.saveYear = in->readSint16LE();
	header.saveMonth = in->readSint16LE();
	header.saveDay = in->readSint16LE();
	header.saveHour = in->readSint16LE();
	header.saveMinutes = in->readSint16LE();
	header.totalFrames = in->readUint32LE();

	return true;
}

void Saver::writeSavegameHeader(Common::OutSaveFile *out, tSageSavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(TSAGE_SAVEGAME_VERSION);

	// Write savegame name
	out->write(header.saveName.c_str(), header.saveName.size() + 1);

	// Get the active palette
	uint8 thumbPalette[256 * 3];
	g_system->getPaletteManager()->grabPalette(thumbPalette, 0, 256);

	// Create a thumbnail and save it
	Graphics::Surface *thumb = new Graphics::Surface();
	Graphics::Surface s = g_globals->_screenSurface.lockSurface();
	::createThumbnail(thumb, (const byte *)s.pixels, SCREEN_WIDTH, SCREEN_HEIGHT, thumbPalette);
	Graphics::saveThumbnail(*out, *thumb);
	g_globals->_screenSurface.unlockSurface();
	thumb->free();
	delete thumb;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
	out->writeUint32LE(g_globals->_events.getFrameNumber());
}

/**
 * Adds a serialisable object that should be saved/restored before any other objects
 */
void Saver::addListener(SaveListener *obj) {
	_listeners.push_back(obj);
}

/**
 * Adds a listener to be notified before the saving starts
 */
void Saver::addSaveNotifier(SaveNotifierFn fn) {
	_saveNotifiers.push_back(fn);
}

/**
 * Adds a listener to be notified before the saving starts
 */
void Saver::addLoadNotifier(SaveNotifierFn fn) {
	_loadNotifiers.push_back(fn);
}

/**
 * Registers a SavedObject descendant object for being saved in savegame files
 */
void Saver::addObject(SavedObject *obj) {
	_objList.push_back(obj);
}

/**
 * Removes a SavedObject descendant object from the save object list
 */
void Saver::removeObject(SavedObject *obj) {
	_objList.remove(obj);
}

/**
 * Returns true if any savegames exist
 */
bool Saver::savegamesExist() const {
	Common::String slot1Name = g_vm->generateSaveName(1);

	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slot1Name);
	bool result = saveFile != NULL;
	delete saveFile;
	return result;
}

/**
 * Returns the index of the saved block associated with the given saved object pointer
 */
int Saver::blockIndexOf(SavedObject *p) {
	int objIndex = 1;
	Common::List<SavedObject *>::iterator iObj;

	for (iObj = _objList.begin(); iObj != _objList.end(); ++iObj, ++objIndex) {
		SavedObject *iObjP = *iObj;
		if (iObjP == p)
			return objIndex;
	}

	return 0;
}

/**
 * Returns the number of objects in the object list registry
 */
int Saver::getObjectCount() const {
	return _objList.size();
}

/**
 * List any currently active objects
 */
void Saver::listObjects() {
	Common::List<SavedObject *>::iterator i;
	int count = 1;

	for (i = _objList.begin(); i != _objList.end(); ++i, ++count)
		debug("%d - %s", count, (*i)->getClassName().c_str());
	debugN("\n");
}

/**
 * Returns the pointer associated with the specified object index
 */
void Saver::resolveLoadPointers() {
	if (_unresolvedPtrs.size() == 0)
		// Nothing to resolve
		return;

	// Outer loop through the main object list
	int objIndex = 1;
	for (SynchronizedList<SavedObject *>::iterator iObj = _objList.begin(); iObj != _objList.end(); ++iObj, ++objIndex) {
		Common::List<SavedObjectRef>::iterator iPtr;
		SavedObject *pObj = *iObj;

		for (iPtr = _unresolvedPtrs.begin(); iPtr != _unresolvedPtrs.end(); ) {
			SavedObjectRef &r = *iPtr;
			if (r._objIndex == objIndex) {
				// Found an unresolved pointer to this object
				SavedObject **objPP = r._savedObject;
				*objPP = pObj;
				iPtr = _unresolvedPtrs.erase(iPtr);
			} else {
				++iPtr;
			}
		}
	}

	// At this point, all the unresolved pointers should have been resolved and removed
	if (_unresolvedPtrs.size() > 0)
		error("Could not resolve savegame block pointers");
}

} // End of namespace TsAGE
