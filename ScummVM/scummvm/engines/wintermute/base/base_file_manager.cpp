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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_persistence_manager.h"
#include "engines/wintermute/base/file/base_disk_file.h"
#include "engines/wintermute/base/file/base_save_thumb_file.h"
#include "engines/wintermute/base/file/base_package.h"
#include "engines/wintermute/base/file/base_resources.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/wintermute.h"
#include "common/debug.h"
#include "common/str.h"
#include "common/tokenizer.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/fs.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
BaseFileManager::BaseFileManager(Common::Language lang) {
	_language = lang;
	initPaths();
	registerPackages();
}

//////////////////////////////////////////////////////////////////////
BaseFileManager::~BaseFileManager() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::cleanup() {
	// delete registered paths
	_packagePaths.clear();

	// close open files
	for (uint32 i = 0; i < _openFiles.size(); i++) {
		delete _openFiles[i];
	}
	_openFiles.clear();

	// delete packages
	_packages.clear();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
byte *BaseFileManager::readWholeFile(const Common::String &filename, uint32 *size, bool mustExist) {
	byte *buffer = NULL;

	Common::SeekableReadStream *file = openFile(filename);
	if (!file) {
		if (mustExist) {
			debugC(kWintermuteDebugFileAccess | kWintermuteDebugLog, "Error opening file '%s'", filename.c_str());
		}
		return NULL;
	}

	buffer = new byte[file->size() + 1];
	if (buffer == NULL) {
		debugC(kWintermuteDebugFileAccess | kWintermuteDebugLog, "Error allocating buffer for file '%s' (%d bytes)", filename.c_str(), file->size() + 1);
		closeFile(file);
		return NULL;
	}

	if (file->read(buffer, (uint32)file->size()) != (uint32)file->size()) {
		debugC(kWintermuteDebugFileAccess | kWintermuteDebugLog, "Error reading file '%s'", filename.c_str());
		closeFile(file);
		delete[] buffer;
		return NULL;
	};

	buffer[file->size()] = '\0';
	if (size != NULL) {
		*size = file->size();
	}
	closeFile(file);

	return buffer;
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::addPath(TPathType type, const Common::FSNode &path) {
	if (!path.exists()) {
		return STATUS_FAILED;
	}

	switch (type) {
	case PATH_SINGLE:
		//  _singlePaths.push_back(path);
		error("TODO: Allow adding single-paths");
		break;
	case PATH_PACKAGE:
		_packagePaths.push_back(path);
		break;
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::reloadPaths() {
	// delete registered paths
	//_singlePaths.clear();
	_packagePaths.clear();

	return initPaths();
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::initPaths() {
	// Removed: Config-based file-path choice.

	// package files paths
	const Common::FSNode gameData(ConfMan.get("path"));
	addPath(PATH_PACKAGE, gameData);

	Common::FSNode dataSubFolder = gameData.getChild("data");
	if (dataSubFolder.exists()) {
		addPath(PATH_PACKAGE, dataSubFolder);
	}
	Common::FSNode languageSubFolder = gameData.getChild("language");
	if (languageSubFolder.exists()) {
		addPath(PATH_PACKAGE, languageSubFolder);
	}
	return STATUS_OK;
}

bool BaseFileManager::registerPackages(const Common::FSList &fslist) {
	for (Common::FSList::const_iterator it = fslist.begin(); it != fslist.end(); ++it) {
		debugC(kWintermuteDebugFileAccess, "Adding %s", (*it).getName().c_str());
		if ((*it).getName().contains(".dcp")) {
			if (registerPackage((*it))) {
				addPath(PATH_PACKAGE, (*it));
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::registerPackages() {
	debugC(kWintermuteDebugFileAccess | kWintermuteDebugLog, "Scanning packages");

	// Register without using SearchMan, as otherwise the FSNode-based lookup in openPackage will fail
	// and that has to be like that to support the detection-scheme.
	Common::FSList files;
	for (Common::FSList::iterator it = _packagePaths.begin(); it != _packagePaths.end(); ++it) {
		debugC(kWintermuteDebugFileAccess, "Should register folder: %s %s", (*it).getPath().c_str(), (*it).getName().c_str());
		(*it).getChildren(files, Common::FSNode::kListFilesOnly);
		for (Common::FSList::iterator fileIt = files.begin(); fileIt != files.end(); ++fileIt) {
			if (!fileIt->getName().hasSuffix(".dcp")) {
				continue;
			}
			// Avoid registering all the language files
			// TODO: Select based on the gameDesc.
			if (_language != Common::UNK_LANG && fileIt->getParent().getName() == "language") {
				Common::String parentName = fileIt->getParent().getName();
				Common::String dcpName = fileIt->getName();
				if (_language == Common::EN_ANY && fileIt->getName() != "english.dcp") {
					continue;
				} else if (_language == Common::CZ_CZE && fileIt->getName() != "czech.dcp") {
					continue;
				} else if (_language == Common::IT_ITA && fileIt->getName() != "italian.dcp") {
					continue;
				} else if (_language == Common::PL_POL && fileIt->getName() != "polish.dcp") {
					continue;
				} else if (_language == Common::RU_RUS && fileIt->getName() != "russian.dcp") {
					continue;
				}
			}
			debugC(kWintermuteDebugFileAccess, "Registering %s %s", (*fileIt).getPath().c_str(), (*fileIt).getName().c_str());
			registerPackage((*fileIt));
		}
	}

//	debugC(kWintermuteDebugFileAccess | kWintermuteDebugLog, "  Registered %d files in %d package(s)", _files.size(), _packages.size());

	return STATUS_OK;
}

bool BaseFileManager::registerPackage(Common::FSNode file, const Common::String &filename, bool searchSignature) {
	PackageSet *pack = new PackageSet(file, filename, searchSignature);
	_packages.add(file.getName(), pack, pack->getPriority() , true);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *BaseFileManager::openPkgFile(const Common::String &filename) {
	Common::String upcName = filename;
	upcName.toUppercase();
	Common::SeekableReadStream *file = NULL;
	char fileName[MAX_PATH_LENGTH];
	strcpy(fileName, upcName.c_str());

	// correct slashes
	for (uint32 i = 0; i < upcName.size(); i++) {
		if (upcName[(int32)i] == '/') {
			upcName.setChar('\\', (uint32)i);
		}
	}
	Common::ArchiveMemberPtr entry = _packages.getMember(upcName);
	if (!entry) {
		return NULL;
	}
	file = entry->createReadStream();
	return file;
}

bool BaseFileManager::hasFile(const Common::String &filename) {
	if (scumm_strnicmp(filename.c_str(), "savegame:", 9) == 0) {
		BasePersistenceManager pm(BaseEngine::instance().getGameId());
		if (filename.size() <= 9) {
			return false;
		}
		int slot = atoi(filename.c_str() + 9);
		return pm.getSaveExists(slot);
	}
	if (diskFileExists(filename)) {
		return true;
	}
	if (_packages.hasFile(filename)) {
		return true;    // We don't bother checking if the file can actually be opened, something bigger is wrong if that is the case.
	}
	if (BaseResources::hasFile(filename)) {
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *BaseFileManager::openFile(const Common::String &filename, bool absPathWarning, bool keepTrackOf) {
	if (strcmp(filename.c_str(), "") == 0) {
		return NULL;
	}
	debugC(kWintermuteDebugFileAccess, "Open file %s", filename.c_str());

	Common::SeekableReadStream *file = openFileRaw(filename);
	if (file && keepTrackOf) {
		_openFiles.push_back(file);
	}
	return file;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFileManager::closeFile(Common::SeekableReadStream *File) {
	for (uint32 i = 0; i < _openFiles.size(); i++) {
		if (_openFiles[i] == File) {
			delete _openFiles[i];
			_openFiles.remove_at(i);
			return STATUS_OK;
		}
	}
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *BaseFileManager::openFileRaw(const Common::String &filename) {
	Common::SeekableReadStream *ret = NULL;

	if (scumm_strnicmp(filename.c_str(), "savegame:", 9) == 0) {
		if (!BaseEngine::instance().getGameRef()) {
			error("Attempt to load filename: %s without BaseEngine-object, this is unsupported", filename.c_str());
		}
		BaseSaveThumbFile *saveThumbFile = new BaseSaveThumbFile();
		if (DID_SUCCEED(saveThumbFile->open(filename))) {
			ret = saveThumbFile->getMemStream();
		}
		delete saveThumbFile;
		return ret;
	}

	ret = openDiskFile(filename);
	if (ret) {
		return ret;
	}

	ret = openPkgFile(filename);
	if (ret) {
		return ret;
	}

	ret = BaseResources::getFile(filename);
	if (ret) {
		return ret;
	}

	debugC(kWintermuteDebugFileAccess ,"BFileManager::OpenFileRaw - Failed to open %s", filename.c_str());
	return NULL;
}

BaseFileManager *BaseFileManager::getEngineInstance() {
	if (BaseEngine::instance().getFileManager()) {
		return BaseEngine::instance().getFileManager();
	}
	return NULL;
}

} // end of namespace Wintermute
