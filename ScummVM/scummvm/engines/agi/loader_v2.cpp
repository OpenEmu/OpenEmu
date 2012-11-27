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

#include "common/textconsole.h"

#include "agi/agi.h"

namespace Agi {

int AgiLoader_v2::detectGame() {
	if (!Common::File::exists(LOGDIR) ||
			!Common::File::exists(PICDIR) ||
			!Common::File::exists(SNDDIR) ||
			!Common::File::exists(VIEWDIR))
		return errInvalidAGIFile;

	return _vm->setupV2Game(_vm->getVersion());
}

int AgiLoader_v2::loadDir(AgiDir *agid, const char *fname) {
	Common::File fp;
	uint8 *mem;
	uint32 flen;
	uint i;

	debug(0, "Loading directory: %s", fname);

	if (!fp.open(fname)) {
		return errBadFileOpen;
	}

	fp.seek(0, SEEK_END);
	flen = fp.pos();
	fp.seek(0, SEEK_SET);

	if ((mem = (uint8 *)malloc(flen + 32)) == NULL) {
		fp.close();
		return errNotEnoughMemory;
	}

	fp.read(mem, flen);

	// set all directory resources to gone
	for (i = 0; i < MAX_DIRS; i++) {
		agid[i].volume = 0xff;
		agid[i].offset = _EMPTY;
	}

	// build directory entries
	for (i = 0; i < flen; i += 3) {
		agid[i / 3].volume = *(mem + i) >> 4;
		agid[i / 3].offset = READ_BE_UINT24(mem + i) & (uint32) _EMPTY;
		debugC(3, kDebugLevelResources, "%d: volume %d, offset 0x%05x", i / 3, agid[i / 3].volume, agid[i / 3].offset);
	}

	free(mem);
	fp.close();

	return errOK;
}

int AgiLoader_v2::init() {
	int ec = errOK;

	// load directory files
	ec = loadDir(_vm->_game.dirLogic, LOGDIR);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirPic, PICDIR);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirView, VIEWDIR);
	if (ec == errOK)
		ec = loadDir(_vm->_game.dirSound, SNDDIR);

	return ec;
}

int AgiLoader_v2::deinit() {
	int ec = errOK;

#if 0
	// unload words
	agiV2UnloadWords();

	// unload objects
	agiV2UnloadObjects();
#endif

	return ec;
}

int AgiLoader_v2::unloadResource(int t, int n) {
	debugC(3, kDebugLevelResources, "unload resource");

	switch (t) {
	case rLOGIC:
		_vm->unloadLogic(n);
		break;
	case rPICTURE:
		_vm->_picture->unloadPicture(n);
		break;
	case rVIEW:
		_vm->unloadView(n);
		break;
	case rSOUND:
		_vm->_sound->unloadSound(n);
		break;
	}

	return errOK;
}

/**
 * This function does noting but load a raw resource into memory,
 * if further decoding is required, it must be done by another
 * routine. NULL is returned if unsucsessfull.
 */
uint8 *AgiLoader_v2::loadVolRes(struct AgiDir *agid) {
	uint8 *data = NULL;
	char x[MAXPATHLEN], *path;
	Common::File fp;
	unsigned int sig;

	sprintf(x, "vol.%i", agid->volume);
	path = x;
	debugC(3, kDebugLevelResources, "Vol res: path = %s", path);

	if (agid->offset != _EMPTY && fp.open(path)) {
		debugC(3, kDebugLevelResources, "loading resource at offset %d", agid->offset);
		fp.seek(agid->offset, SEEK_SET);
		fp.read(&x, 5);
		if ((sig = READ_BE_UINT16((uint8 *) x)) == 0x1234) {
			agid->len = READ_LE_UINT16((uint8 *) x + 3);
			data = (uint8 *)calloc(1, agid->len + 32);
			if (data != NULL) {
				fp.read(data, agid->len);
			} else {
				error("AgiLoader_v2::loadVolRes out of memory");
			}
		} else {
			warning("AgiLoader_v2::loadVolRes: bad signature %04x", sig);
			return 0;
		}
		fp.close();
	} else {
		// we have a bad volume resource
		// set that resource to NA
		agid->offset = _EMPTY;
	}

	return data;
}

/**
 * Loads a resource into memory, a raw resource is loaded in
 * with above routine, then further decoded here.
 */
int AgiLoader_v2::loadResource(int t, int n) {
	int ec = errOK;
	uint8 *data = NULL;

	debugC(3, kDebugLevelResources, "(t = %d, n = %d)", t, n);
	if (n > MAX_DIRS)
		return errBadResource;

	switch (t) {
	case rLOGIC:
		if (~_vm->_game.dirLogic[n].flags & RES_LOADED) {
			debugC(3, kDebugLevelResources, "loading logic resource %d", n);
			unloadResource(rLOGIC, n);

			// load raw resource into data
			data = loadVolRes(&_vm->_game.dirLogic[n]);

			_vm->_game.logics[n].data = data;
			ec = data ? _vm->decodeLogic(n) : errBadResource;

			_vm->_game.logics[n].sIP = 2;
		}

		// if logic was cached, we get here
		// reset code pointers incase it was cached

		_vm->_game.logics[n].cIP = _vm->_game.logics[n].sIP;
		break;
	case rPICTURE:
		// if picture is currently NOT loaded *OR* cacheing is off,
		// unload the resource (caching == off) and reload it

		debugC(3, kDebugLevelResources, "loading picture resource %d", n);
		if (_vm->_game.dirPic[n].flags & RES_LOADED)
			break;

		// if loaded but not cached, unload it
		// if cached but not loaded, etc
		unloadResource(rPICTURE, n);
		data = loadVolRes(&_vm->_game.dirPic[n]);

		if (data != NULL) {
			_vm->_game.pictures[n].rdata = data;
			_vm->_game.dirPic[n].flags |= RES_LOADED;
		} else {
			ec = errBadResource;
		}
		break;
	case rSOUND:
		debugC(3, kDebugLevelResources, "loading sound resource %d", n);
		if (_vm->_game.dirSound[n].flags & RES_LOADED)
			break;

		data = loadVolRes(&_vm->_game.dirSound[n]);

		if (data != NULL) {
			// Freeing of the raw resource from memory is delegated to the createFromRawResource-function
			_vm->_game.sounds[n] = AgiSound::createFromRawResource(data, _vm->_game.dirSound[n].len, n, *_vm->_sound, _vm->_soundemu);
			_vm->_game.dirSound[n].flags |= RES_LOADED;
		} else {
			ec = errBadResource;
		}
		break;
	case rVIEW:
		// Load a VIEW resource into memory...
		// Since VIEWS alter the view table ALL the time
		// can we cache the view? or must we reload it all
		// the time?
		if (_vm->_game.dirView[n].flags & RES_LOADED)
			break;

		debugC(3, kDebugLevelResources, "loading view resource %d", n);
		unloadResource(rVIEW, n);
		data = loadVolRes(&_vm->_game.dirView[n]);
		if (data) {
			_vm->_game.views[n].rdata = data;
			_vm->_game.dirView[n].flags |= RES_LOADED;
			ec = _vm->decodeView(n);
		} else {
			ec = errBadResource;
		}
		break;
	default:
		ec = errBadResource;
		break;
	}

	return ec;
}

int AgiLoader_v2::loadObjects(const char *fname) {
	return _vm->loadObjects(fname);
}

int AgiLoader_v2::loadWords(const char *fname) {
	return _vm->loadWords(fname);
}

} // End of namespace Agi
