/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/console.h"
#include "sword2/logic.h"
#include "sword2/memory.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/screen.h"
#include "sword2/sound.h"

#define Debug_Printf _vm->_debugger->DebugPrintf

namespace Sword2 {

// Welcome to the easy resource manager - written in simple code for easy
// maintenance
//
// The resource compiler will create two files
//
//	resource.inf which is a list of ascii cluster file names
//	resource.tab which is a table which tells us which cluster a resource
//	is located in and the number within the cluster

enum {
	BOTH		= 0x0,		// Cluster is on both CDs
	CD1		= 0x1,		// Cluster is on CD1 only
	CD2		= 0x2,		// Cluster is on CD2 only
	LOCAL_CACHE	= 0x4,		// Cluster is cached on HDD
	LOCAL_PERM	= 0x8		// Cluster is on HDD.
};

struct CdInf {
	uint8 clusterName[20];	// Null terminated cluster name.
	uint8 cd;		// Cd cluster is on and whether it is on the local drive or not.
};

ResourceManager::ResourceManager(Sword2Engine *vm) {
	_vm = vm;

	_totalClusters = 0;
	_resList = NULL;
	_resConvTable = NULL;
	_cacheStart = NULL;
	_cacheEnd = NULL;
	_usedMem = 0;
}

ResourceManager::~ResourceManager() {
	Resource *res = _cacheStart;
	while (res) {
		_vm->_memory->memFree(res->ptr);
		res = res->next;
	}
	for (uint i = 0; i < _totalClusters; i++)
		free(_resFiles[i].entryTab);
	free(_resList);
	free(_resConvTable);
}


bool ResourceManager::init() {
	uint32 i, j;

	// Until proven differently, assume we're on CD 1. This is so the start
	// dialog will be able to play any music at all.

	setCD(1);

	// We read in the resource info which tells us the names of the
	// resource cluster files ultimately, although there might be groups
	// within the clusters at this point it makes no difference. We only
	// wish to know what resource files there are and what is in each

	Common::File file;

	if (!file.open("resource.inf")) {
		GUIErrorMessage("Broken Sword II: Cannot open resource.inf");
		return false;
	}

	// The resource.inf file is a simple text file containing the names of
	// all the resource files.

	while (1) {
		char *buf = _resFiles[_totalClusters].fileName;
		uint len = sizeof(_resFiles[_totalClusters].fileName);

		if (!file.readLine(buf, len))
			break;

		int pos = strlen(buf);
		if (buf[pos - 1] == 0x0A)
			buf[pos - 1] = 0;

		_resFiles[_totalClusters].numEntries = -1;
		_resFiles[_totalClusters].entryTab = NULL;
		if (++_totalClusters >= MAX_res_files) {
			GUIErrorMessage("Broken Sword II: Too many entries in resource.inf");
			return false;
		}
	}

	file.close();

	// Now load in the binary id to res conversion table
	if (!file.open("resource.tab")) {
		GUIErrorMessage("Broken Sword II: Cannot open resource.tab");
		return false;
	}

	// Find how many resources
	uint32 size = file.size();

	_totalResFiles = size / 4;

	// Table seems ok so malloc some space
	_resConvTable = (uint16 *)malloc(size);

	for (i = 0; i < size / 2; i++)
		_resConvTable[i] = file.readUint16LE();

	if (file.eos() || file.err()) {
		file.close();
		GUIErrorMessage("Broken Sword II: Cannot read resource.tab");
		return false;
	}

	file.close();

	// Check that we have cd.inf file, unless we are running PSX
	// version, which has all files on one disc.

	if (!file.open("cd.inf") && !Sword2Engine::isPsx()) {
		GUIErrorMessage("Broken Sword II: Cannot open cd.inf");
		return false;
	}

	CdInf *cdInf = new CdInf[_totalClusters];

	for (i = 0; i < _totalClusters; i++) {

		if (Sword2Engine::isPsx()) { // We are running PSX version, artificially fill CdInf structure
			cdInf[i].cd = CD1;
		} else { // We are running PC version, read cd.inf file
			file.read(cdInf[i].clusterName, sizeof(cdInf[i].clusterName));

			cdInf[i].cd = file.readByte();

			if (file.eos() || file.err()) {
				delete[] cdInf;
				file.close();
				GUIErrorMessage("Broken Sword II: Cannot read cd.inf");
				return false;
			}

		}

		// It has been reported that there are two different versions
		// of the cd.inf file: One where all clusters on CD also have
		// the LOCAL_CACHE bit set. This bit is no longer used. To
		// avoid future problems, let's normalize the flag once and for
		// all here.

		if (cdInf[i].cd & LOCAL_PERM)
			cdInf[i].cd = 0;
		else if (cdInf[i].cd & CD1)
			cdInf[i].cd = 1;
		else if (cdInf[i].cd & CD2)
			cdInf[i].cd = 2;
		else
			cdInf[i].cd = 0;

		// Any file on "CD 0" may be needed at all times. Verify that
		// it exists. Any other missing cluster will be requested with
		// an "insert CD" message. Of course, the file may still vanish
		// during game-play (oh, that wascally wabbit!) in which case
		// the resource manager will print a fatal error.

		if (cdInf[i].cd == 0 && !Common::File::exists((char *)cdInf[i].clusterName)) {
			GUIErrorMessage("Broken Sword II: Cannot find " + Common::String((char *)cdInf[i].clusterName));
			delete[] cdInf;
			return false;
		}
	}

	file.close();

	// We check the presence of resource files in cd.inf
	// This is ok in PC version, but in PSX version we don't
	// have cd.inf so we'll have to skip this.
	if (!Sword2Engine::isPsx()) {
		for (i = 0; i < _totalClusters; i++) {
			for (j = 0; j < _totalClusters; j++) {
				if (scumm_stricmp((char *)cdInf[j].clusterName, _resFiles[i].fileName) == 0)
					break;
			}

			if (j == _totalClusters) {
				delete[] cdInf;
				GUIErrorMessage(Common::String(_resFiles[i].fileName) + " is not in cd.inf");
				return false;
			}

			_resFiles[i].cd = cdInf[j].cd;
		}
	}

	delete[] cdInf;

	debug(1, "%d resources in %d cluster files", _totalResFiles, _totalClusters);
	for (i = 0; i < _totalClusters; i++)
		debug(2, "filename of cluster %d: -%s (%d)", i, _resFiles[i].fileName, _resFiles[i].cd);

	_resList = (Resource *)malloc(_totalResFiles * sizeof(Resource));

	for (i = 0; i < _totalResFiles; i++) {
		_resList[i].ptr = NULL;
		_resList[i].size = 0;
		_resList[i].refCount = 0;
		_resList[i].prev = _resList[i].next = NULL;
	}

	return true;
}

/**
 * Returns the address of a resource. Loads if not in memory. Retains a count.
 */
byte *ResourceManager::openResource(uint32 res, bool dump) {
	assert(res < _totalResFiles);


	// FIXME: In PSX edition, not all top menu icons are present (TOP menu is not used).
	// Though, at present state, the engine still ask for the resources.
	if (Sword2Engine::isPsx()) { // We need to "rewire" missing icons
		if (res == 342) res = 364; // Rewire RESTORE ICON to SAVE ICON
	}

	// Is the resource in memory already? If not, load it.

	if (!_resList[res].ptr) {
		// Fetch the correct file and read in the correct portion.
		uint16 cluFileNum = _resConvTable[res * 2]; // points to the number of the ascii filename

		assert(cluFileNum != 0xffff);

		// Relative resource within the file
		// First we have to find the file via the _resConvTable
		uint16 actual_res = _resConvTable[(res * 2) + 1];

		debug(5, "openResource %s res %d", _resFiles[cluFileNum].fileName, res);

		// If we're loading a cluster that's only available from one
		// of the CDs, remember which one so that we can play the
		// correct speech and music.

		if (Sword2Engine::isPsx()) // We have only one disk in PSX version
			setCD(CD1);
		else
			setCD(_resFiles[cluFileNum].cd);

		// Actually, as long as the file can be found we don't really
		// care which CD it's on. But if we can't find it, keep asking
		// for the CD until we do.

		Common::File *file = openCluFile(cluFileNum);

		if (_resFiles[cluFileNum].entryTab == NULL) {
			// we didn't read from this file before, get its index table
			readCluIndex(cluFileNum, file);
		}

		assert(_resFiles[cluFileNum].entryTab);

		uint32 pos = _resFiles[cluFileNum].entryTab[actual_res * 2 + 0];
		uint32 len = _resFiles[cluFileNum].entryTab[actual_res * 2 + 1];

		file->seek(pos, SEEK_SET);

		debug(6, "res len %d", len);

		// Ok, we know the length so try and allocate the memory.
		_resList[res].ptr = _vm->_memory->memAlloc(len, res);
		_resList[res].size = len;
		_resList[res].refCount = 0;

		file->read(_resList[res].ptr, len);

		debug(3, "Loaded resource '%s' (%d) from '%s' on CD %d (%d)", fetchName(_resList[res].ptr), res, _resFiles[cluFileNum].fileName, getCD(), _resFiles[cluFileNum].cd);

		if (dump) {
			char buf[256];
			const char *tag;

			switch (fetchType(_resList[res].ptr)) {
			case ANIMATION_FILE:
				tag = "anim";
				break;
			case SCREEN_FILE:
				tag = "layer";
				break;
			case GAME_OBJECT:
				tag = "object";
				break;
			case WALK_GRID_FILE:
				tag = "walkgrid";
				break;
			case GLOBAL_VAR_FILE:
				tag = "globals";
				break;
			case PARALLAX_FILE_null:
				tag = "parallax";	// Not used!
				break;
			case RUN_LIST:
				tag = "runlist";
				break;
			case TEXT_FILE:
				tag = "text";
				break;
			case SCREEN_MANAGER:
				tag = "screen";
				break;
			case MOUSE_FILE:
				tag = "mouse";
				break;
			case WAV_FILE:
				tag = "wav";
				break;
			case ICON_FILE:
				tag = "icon";
				break;
			case PALETTE_FILE:
				tag = "palette";
				break;
			default:
				tag = "unknown";
				break;
			}

			sprintf(buf, "dumps/%s-%d.dmp", tag, res);

			if (!Common::File::exists(buf)) {
				Common::DumpFile out;
				if (out.open(buf))
					out.write(_resList[res].ptr, len);
			}
		}

		// close the cluster
		file->close();
		delete file;

		_usedMem += len;
		checkMemUsage();
	} else if (_resList[res].refCount == 0)
		removeFromCacheList(_resList + res);

	_resList[res].refCount++;

	return _resList[res].ptr;
}

void ResourceManager::closeResource(uint32 res) {
	assert(res < _totalResFiles);

	// Don't try to close the resource if it has already been forcibly
	// closed, e.g. by fnResetGlobals().

	if (_resList[res].ptr == NULL)
		return;

	assert(_resList[res].refCount > 0);

	_resList[res].refCount--;
	if (_resList[res].refCount == 0)
		addToCacheList(_resList + res);

	// It's tempting to free the resource immediately when refCount
	// reaches zero, but that'd be a mistake. Closing a resource does not
	// mean "I'm not going to use this resource any more". It means that
	// "the next time I use this resource I'm going to ask for a new
	// pointer to it".
	//
	// Since the original memory manager had to deal with memory
	// fragmentation, keeping a resource open - and thus locked down to a
	// specific memory address - was considered a bad thing.
}

void ResourceManager::removeFromCacheList(Resource *res) {
	if (_cacheStart == res)
		_cacheStart = res->next;

	if (_cacheEnd == res)
		_cacheEnd = res->prev;

	if (res->prev)
		res->prev->next = res->next;
	if (res->next)
		res->next->prev = res->prev;
	res->prev = res->next = NULL;
}

void ResourceManager::addToCacheList(Resource *res) {
	res->prev = NULL;
	res->next = _cacheStart;
	if (_cacheStart)
		_cacheStart->prev = res;
	_cacheStart = res;
	if (!_cacheEnd)
		_cacheEnd = res;
}

Common::File *ResourceManager::openCluFile(uint16 fileNum) {
	Common::File *file = new Common::File;
	while (!file->open(_resFiles[fileNum].fileName)) {
		// HACK: We have to check for this, or it'll be impossible to
		// quit while the game is asking for the user to insert a CD.
		// But recovering from this situation gracefully is just too
		// much trouble, so quit now.
		if (_vm->shouldQuit())
			g_system->quit();

		// If the file is supposed to be on hard disk, or we're
		// playing a demo, then we're in trouble if the file
		// can't be found!

		if ((_vm->_features & GF_DEMO) || _resFiles[fileNum].cd == 0)
			error("Could not find '%s'", _resFiles[fileNum].fileName);

		askForCD(_resFiles[fileNum].cd);
	}
	return file;
}

void ResourceManager::readCluIndex(uint16 fileNum, Common::File *file) {
	// we didn't read from this file before, get its index table
	assert(_resFiles[fileNum].entryTab == NULL);
	assert(file);

	// 1st DWORD of a cluster is an offset to the look-up table
	uint32 table_offset = file->readUint32LE();
	debug(6, "table offset = %d", table_offset);
	uint32 tableSize = file->size() - table_offset; // the table is stored at the end of the file
	file->seek(table_offset);

	assert((tableSize % 8) == 0);
	_resFiles[fileNum].entryTab = (uint32 *)malloc(tableSize);
	_resFiles[fileNum].numEntries = tableSize / 8;

	assert(_resFiles[fileNum].entryTab);

	file->read(_resFiles[fileNum].entryTab, tableSize);
	if (file->eos() || file->err())
		error("unable to read index table from file %s", _resFiles[fileNum].fileName);

#ifdef SCUMM_BIG_ENDIAN
	for (int tabCnt = 0; tabCnt < _resFiles[fileNum].numEntries * 2; tabCnt++)
		_resFiles[fileNum].entryTab[tabCnt] = FROM_LE_32(_resFiles[fileNum].entryTab[tabCnt]);
#endif
}

/**
 * Returns true if resource is valid, otherwise false.
 */

bool ResourceManager::checkValid(uint32 res) {
	// Resource number out of range
	if (res >= _totalResFiles)
		return false;

	// Points to the number of the ascii filename
	uint16 parent_res_file = _resConvTable[res * 2];

	// Null & void resource
	if (parent_res_file == 0xffff)
		return false;

	return true;
}

/**
 * Fetch resource type
 */

uint8 ResourceManager::fetchType(byte *ptr) {
	if (!Sword2Engine::isPsx()) {
		return ptr[0];
	} else { // in PSX version, some files got a "garbled" resource header, with type stored in ninth byte
		if (ptr[0]) {
			return ptr[0];
		} else if (ptr[8]) {
			return ptr[8];
		} else  {            // In PSX version there is no resource header for audio files,
			return WAV_FILE; // but hopefully all audio files got first 16 bytes zeroed,
		}                    // Allowing us to check for this condition.
							 // Alas, this doesn't work with PSX DEMO audio files.

	}
}

/**
 * Returns the total file length of a resource - i.e. all headers are included
 * too.
 */

uint32 ResourceManager::fetchLen(uint32 res) {
	if (_resList[res].ptr)
		return _resList[res].size;

	// Does this ever happen?
	warning("fetchLen: Resource %u is not loaded; reading length from file", res);

	// Points to the number of the ascii filename
	uint16 parent_res_file = _resConvTable[res * 2];

	// relative resource within the file
	uint16 actual_res = _resConvTable[(res * 2) + 1];

	// first we have to find the file via the _resConvTable
	// open the cluster file

	if (_resFiles[parent_res_file].entryTab == NULL) {
		Common::File *file = openCluFile(parent_res_file);
		readCluIndex(parent_res_file, file);
		delete file;
	}
	return _resFiles[parent_res_file].entryTab[actual_res * 2 + 1];
}

void ResourceManager::checkMemUsage() {
	while (_usedMem > MAX_MEM_CACHE) {
		// we're using up more memory than we wanted to. free some old stuff.
		// Newly loaded objects are added to the start of the list,
		// we start freeing from the end, to free the oldest items first
		if (_cacheEnd) {
			Resource *tmp = _cacheEnd;
			assert((tmp->refCount == 0) && (tmp->ptr) && (tmp->next == NULL));
			removeFromCacheList(tmp);

			_vm->_memory->memFree(tmp->ptr);
			tmp->ptr = NULL;
			_usedMem -= tmp->size;
		} else {
			warning("%d bytes of memory used, but cache list is empty", _usedMem);
			return;
		}
	}
}

void ResourceManager::remove(int res) {
	if (_resList[res].ptr) {
		removeFromCacheList(_resList + res);

		_vm->_memory->memFree(_resList[res].ptr);
		_resList[res].ptr = NULL;
		_resList[res].refCount = 0;
		_usedMem -= _resList[res].size;
	}
}

/**
 * Remove all res files from memory - ready for a total restart. This includes
 * the player object and global variables resource.
 */

void ResourceManager::removeAll() {
	// We need to clear the FX queue, because otherwise the sound system
	// will still believe that the sound resources are in memory. We also
	// need to kill the movie lead-in/out.

	_vm->_sound->clearFxQueue(true);

	for (uint i = 0; i < _totalResFiles; i++)
		remove(i);
}

/**
 * Remove all resources from memory.
 */

void ResourceManager::killAll(bool wantInfo) {
	int nuked = 0;

	// We need to clear the FX queue, because otherwise the sound system
	// will still believe that the sound resources are in memory. We also
	// need to kill the movie lead-in/out.

	_vm->_sound->clearFxQueue(true);

	for (uint i = 0; i < _totalResFiles; i++) {
		// Don't nuke the global variables or the player object!
		if (i == 1 || i == CUR_PLAYER_ID)
			continue;

		if (_resList[i].ptr) {
			if (wantInfo)
				Debug_Printf("Nuked %5d: %s\n", i, fetchName(_resList[i].ptr));

			remove(i);
			nuked++;
		}
	}

	if (wantInfo)
		Debug_Printf("Expelled %d resources\n", nuked);
}

/**
 * Like killAll but only kills objects (except George & the variable table of
 * course) - ie. forcing them to reload & restart their scripts, which
 * simulates the effect of a save & restore, thus checking that each object's
 * re-entrant logic works correctly, and doesn't cause a statuette to
 * disappear forever, or some plaster-filled holes in sand to crash the game &
 * get James in trouble again.
 */

void ResourceManager::killAllObjects(bool wantInfo) {
	int nuked = 0;

	for (uint i = 0; i < _totalResFiles; i++) {
		// Don't nuke the global variables or the player object!
		if (i == 1 || i == CUR_PLAYER_ID)
			continue;

		if (_resList[i].ptr) {
			if (fetchType(_resList[i].ptr) == GAME_OBJECT) {
				if (wantInfo)
					Debug_Printf("Nuked %5d: %s\n", i, fetchName(_resList[i].ptr));

				remove(i);
				nuked++;
			}
		}
	}

	if (wantInfo)
		Debug_Printf("Expelled %d resources\n", nuked);
}

void ResourceManager::askForCD(int cd) {
	byte *textRes;

	// Stop any music from playing - so the system no longer needs the
	// current CD - otherwise when we take out the CD, Windows will
	// complain!

	_vm->_sound->stopMusic(true);

	textRes = openResource(2283);
	_vm->_screen->displayMsg(_vm->fetchTextLine(textRes, 5 + cd) + 2, 0);
	closeResource(2283);

	// The original code probably determined automagically when the correct
	// CD had been inserted, but our backend doesn't support that, and
	// anyway I don't know if all systems allow that sort of thing. So we
	// wait for the user to press any key instead, or click the mouse.
	//
	// But just in case we ever try to identify the CDs by their labels,
	// they should be:
	//
	// CD1: "RBSII1" (or "PCF76" for the PCF76 version, whatever that is)
	// CD2: "RBSII2"
}

} // End of namespace Sword2
