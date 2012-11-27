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


#include "common/system.h"
#include "scumm/actor.h"
#include "scumm/boxes.h"
#ifdef ENABLE_HE
#include "scumm/he/intern_he.h"
#endif
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm_v3.h"
#include "scumm/sound.h"
#include "scumm/util.h"

namespace Scumm {

/**
 * Start a 'scene' by loading the specified room with the given main actor.
 * The actor is placed next to the object indicated by objectNr.
 */
void ScummEngine::startScene(int room, Actor *a, int objectNr) {
	int i, where;

	debugC(DEBUG_GENERAL, "Loading room %d", room);

	stopTalk();

	fadeOut(_switchRoomEffect2);
	_newEffect = _switchRoomEffect;

	ScriptSlot *ss = &vm.slot[_currentScript];

	if (_currentScript != 0xFF) {
		if (ss->where == WIO_ROOM || ss->where == WIO_FLOBJECT) {
			if (ss->cutsceneOverride && _game.version >= 5)
				error("Object %d stopped with active cutscene/override in exit", ss->number);

			nukeArrays(_currentScript);
			_currentScript = 0xFF;
		} else if (ss->where == WIO_LOCAL) {
			if (ss->cutsceneOverride && _game.version >= 5)
				error("Script %d stopped with active cutscene/override in exit", ss->number);

			nukeArrays(_currentScript);
			_currentScript = 0xFF;
		}
	}

	if (VAR_NEW_ROOM != 0xFF)
		VAR(VAR_NEW_ROOM) = room;

	runExitScript();

	killScriptsAndResources();
	if (_game.version >= 4 && _game.heversion <= 62)
		stopCycle(0);

	if (_game.id == GID_SAMNMAX) {
		// WORKAROUND bug #85373 SAM: Overlapping music at Bigfoot convention
		// Added sound queue processing between execution of exit
		// script and entry script. In the case of this bug, the
		// entry script required that the iMuse state be fully up
		// to date, including last-moment changes from the previous
		// exit script.
		_sound->processSound();
	}

	clearDrawQueues();

	// For HE80+ games
	for (i = 0; i < _numRoomVariables; i++)
		_roomVars[i] = 0;
	nukeArrays(0xFF);

	for (i = 1; i < _numActors; i++) {
		_actors[i]->hideActor();
	}

	if (_game.version >= 7) {
		// Set the shadow palette(s) to all black. This fixes
		// bug #795940, and actually makes some sense (after all,
		// shadows tend to be rather black, don't they? ;-)
		memset(_shadowPalette, 0, NUM_SHADOW_PALETTE * 256);
	} else {
		for (i = 0; i < 256; i++) {
			_roomPalette[i] = i;
			if (_shadowPalette)
				_shadowPalette[i] = i;
		}
		if (_game.features & GF_SMALL_HEADER)
			setDirtyColors(0, 255);
	}

	VAR(VAR_ROOM) = room;
	_fullRedraw = true;

	_res->increaseResourceCounters();

	_currentRoom = room;
	VAR(VAR_ROOM) = room;

	if (room >= 0x80 && _game.version < 7 && _game.heversion <= 71)
		_roomResource = _resourceMapper[room & 0x7F];
	else
		_roomResource = room;

	if (VAR_ROOM_RESOURCE != 0xFF)
		VAR(VAR_ROOM_RESOURCE) = _roomResource;

	if (room != 0)
		ensureResourceLoaded(rtRoom, room);

	clearRoomObjects();

	if (_currentRoom == 0) {
		_ENCD_offs = _EXCD_offs = 0;
		_numObjectsInRoom = 0;
		return;
	}

	setupRoomSubBlocks();
	resetRoomSubBlocks();

	initBGBuffers(_roomHeight);

	resetRoomObjects();

	if (VAR_ROOM_WIDTH != 0xFF && VAR_ROOM_HEIGHT != 0xFF) {
		VAR(VAR_ROOM_WIDTH) = _roomWidth;
		VAR(VAR_ROOM_HEIGHT) = _roomHeight;
	}

	if (VAR_CAMERA_MIN_X != 0xFF)
		VAR(VAR_CAMERA_MIN_X) = _screenWidth / 2;
	if (VAR_CAMERA_MAX_X != 0xFF)
		VAR(VAR_CAMERA_MAX_X) = _roomWidth - (_screenWidth / 2);

	if (_game.version >= 7) {
		VAR(VAR_CAMERA_MIN_Y) = _screenHeight / 2;
		VAR(VAR_CAMERA_MAX_Y) = _roomHeight - (_screenHeight / 2);
		setCameraAt(_screenWidth / 2, _screenHeight / 2);
	} else {
		camera._mode = kNormalCameraMode;
		if (_game.version > 2)
			camera._cur.x = camera._dest.x = _screenWidth / 2;
		camera._cur.y = camera._dest.y = _screenHeight / 2;
	}

	if (_roomResource == 0)
		return;

	memset(gfxUsageBits, 0, sizeof(gfxUsageBits));

	if (_game.version >= 5 && a) {
		where = whereIsObject(objectNr);
		if (where != WIO_ROOM && where != WIO_FLOBJECT)
			error("startScene: Object %d is not in room %d", objectNr,
					_currentRoom);
		int x, y, dir;
		getObjectXYPos(objectNr, x, y, dir);
		a->putActor(x, y, _currentRoom);
		a->setDirection(dir + 180);
		a->stopActorMoving();
		if (_game.id == GID_SAMNMAX) {
			camera._cur.x = camera._dest.x = a->getPos().x;
			setCameraAt(a->getPos().x, a->getPos().y);
		}
	}

	showActors();

	_egoPositioned = false;

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	towns_resetPalCycleFields();
#endif

	runEntryScript();
	if (_game.version >= 1 && _game.version <= 2) {
		runScript(5, 0, 0, 0);
	} else if (_game.version >= 5 && _game.version <= 6) {
		if (a && !_egoPositioned) {
			int x, y;
			getObjectXYPos(objectNr, x, y);
			a->putActor(x, y, _currentRoom);
			a->_moving = 0;
		}
	} else if (_game.version >= 7) {
		if (camera._follows) {
			a = derefActor(camera._follows, "startScene: follows");
			setCameraAt(a->getPos().x, a->getPos().y);
		}
	}

	_doEffect = true;

	// Hint the backend about the virtual keyboard during copy protection screens
	if (_game.id == GID_MONKEY2) {
		if (_system->getFeatureState(OSystem::kFeatureVirtualKeyboard)) {
			if (room != 108)
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
		} else if (room == 108)
			_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	} else if (_game.id == GID_MONKEY_EGA) {	// this is my estimation that the room code is 90 (untested)
		if (_system->getFeatureState(OSystem::kFeatureVirtualKeyboard)) {
			if (room != 90)
				_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
		} else if (room == 90)
			_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	}

}

/**
 * Init some static room data after a room has been loaded.
 * E.g. the room dimension, the offset to the graphics data, the room scripts,
 * the offset to the room palette and other things which won't be changed
 * late on.
 * So it is possible to call this after loading a savegame.
 */
void ScummEngine::setupRoomSubBlocks() {
	int i;
	const byte *ptr;
	byte *roomptr, *searchptr, *roomResPtr = 0;
	const RoomHeader *rmhd;

	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_EPAL_offs = 0;
	_CLUT_offs = 0;
	_PALS_offs = 0;

	// Determine the room and room script base address
	roomResPtr = roomptr = getResourceAddress(rtRoom, _roomResource);
	if (_game.version == 8)
		roomResPtr = getResourceAddress(rtRoomScripts, _roomResource);
	if (!roomptr || !roomResPtr)
		error("Room %d: data not found (" __FILE__  ":%d)", _roomResource, __LINE__);

	//
	// Determine the room dimensions (width/height)
	//
	rmhd = (const RoomHeader *)findResourceData(MKTAG('R','M','H','D'), roomptr);

	if (_game.version == 8) {
		_roomWidth = READ_LE_UINT32(&(rmhd->v8.width));
		_roomHeight = READ_LE_UINT32(&(rmhd->v8.height));
		_numObjectsInRoom = (byte)READ_LE_UINT32(&(rmhd->v8.numObjects));
	} else if (_game.version == 7) {
		_roomWidth = READ_LE_UINT16(&(rmhd->v7.width));
		_roomHeight = READ_LE_UINT16(&(rmhd->v7.height));
		_numObjectsInRoom = (byte)READ_LE_UINT16(&(rmhd->v7.numObjects));
	} else {
		_roomWidth = READ_LE_UINT16(&(rmhd->old.width));
		_roomHeight = READ_LE_UINT16(&(rmhd->old.height));
		_numObjectsInRoom = (byte)READ_LE_UINT16(&(rmhd->old.numObjects));
	}

	//
	// Find the room image data
	//
	if (_game.version == 8) {
		_IM00_offs = getObjectImage(roomptr, 1) - roomptr;
	} else if (_game.features & GF_SMALL_HEADER) {
		_IM00_offs = findResourceData(MKTAG('I','M','0','0'), roomptr) - roomptr;
	} else if (_game.heversion >= 70) {
		byte *roomImagePtr = getResourceAddress(rtRoomImage, _roomResource);
		_IM00_offs = findResource(MKTAG('I','M','0','0'), roomImagePtr) - roomImagePtr;
	} else {
		_IM00_offs = findResource(MKTAG('I','M','0','0'), findResource(MKTAG('R','M','I','M'), roomptr)) - roomptr;
	}

	//
	// Look for an exit script
	//
	ptr = findResourceData(MKTAG('E','X','C','D'), roomResPtr);
	if (ptr)
		_EXCD_offs = ptr - roomResPtr;
	if (_dumpScripts && _EXCD_offs)
		dumpResource("exit-", _roomResource, roomResPtr + _EXCD_offs - _resourceHeaderSize, -1);

	//
	// Look for an entry script
	//
	ptr = findResourceData(MKTAG('E','N','C','D'), roomResPtr);
	if (ptr)
		_ENCD_offs = ptr - roomResPtr;
	if (_dumpScripts && _ENCD_offs)
		dumpResource("entry-", _roomResource, roomResPtr + _ENCD_offs - _resourceHeaderSize, -1);

	//
	// Setup local scripts
	//

	// Determine the room script base address
	roomResPtr = roomptr = getResourceAddress(rtRoom, _roomResource);
	if (_game.version == 8)
		roomResPtr = getResourceAddress(rtRoomScripts, _roomResource);
	searchptr = roomResPtr;

	memset(_localScriptOffsets, 0, sizeof(_localScriptOffsets));

	if (_game.features & GF_SMALL_HEADER) {
		ResourceIterator localScriptIterator(searchptr, true);
		while ((ptr = localScriptIterator.findNext(MKTAG('L','S','C','R'))) != NULL) {
			int id = 0;
			ptr += _resourceHeaderSize;	/* skip tag & size */
			id = ptr[0];

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}

			_localScriptOffsets[id - _numGlobalScripts] = ptr + 1 - roomptr;
		}
	} else if (_game.heversion >= 90) {
		ResourceIterator localScriptIterator2(searchptr, false);
		while ((ptr = localScriptIterator2.findNext(MKTAG('L','S','C','2'))) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			id = READ_LE_UINT32(ptr);

			assertRange(_numGlobalScripts, id, _numLocalScripts + _numGlobalScripts, "local script");
			_localScriptOffsets[id - _numGlobalScripts] = ptr + 4 - roomResPtr;

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}
		}

		ResourceIterator localScriptIterator(searchptr, false);
		while ((ptr = localScriptIterator.findNext(MKTAG('L','S','C','R'))) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			id = ptr[0];
			_localScriptOffsets[id - _numGlobalScripts] = ptr + 1 - roomResPtr;

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}
		}

	} else {
		ResourceIterator localScriptIterator(searchptr, false);
		while ((ptr = localScriptIterator.findNext(MKTAG('L','S','C','R'))) != NULL) {
			int id = 0;

			ptr += _resourceHeaderSize;	/* skip tag & size */

			if (_game.version == 8) {
				id = READ_LE_UINT32(ptr);
				assertRange(_numGlobalScripts, id, _numLocalScripts + _numGlobalScripts, "local script");
				_localScriptOffsets[id - _numGlobalScripts] = ptr + 4 - roomResPtr;
			} else if (_game.version == 7) {
				id = READ_LE_UINT16(ptr);
				assertRange(_numGlobalScripts, id, _numLocalScripts + _numGlobalScripts, "local script");
				_localScriptOffsets[id - _numGlobalScripts] = ptr + 2 - roomResPtr;
			} else {
				id = ptr[0];
				_localScriptOffsets[id - _numGlobalScripts] = ptr + 1 - roomResPtr;
			}

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);
				dumpResource(buf, id, ptr - _resourceHeaderSize);
			}
		}
	}

	// Locate the EGA palette (currently unused).
	ptr = findResourceData(MKTAG('E','P','A','L'), roomptr);
	if (ptr)
		_EPAL_offs = ptr - roomptr;

	// Locate the standard room palette (for V3-V5 games).
	ptr = findResourceData(MKTAG('C','L','U','T'), roomptr);
	if (ptr)
		_CLUT_offs = ptr - roomptr;

	// Locate the standard room palettes (for V6+ games).
	if (_game.version >= 6) {
		ptr = findResource(MKTAG('P','A','L','S'), roomptr);
		if (ptr) {
			_PALS_offs = ptr - roomptr;
		}
	}

	// Transparent color
	byte trans;
	if (_game.version == 8)
		trans = (byte)READ_LE_UINT32(&(rmhd->v8.transparency));
	else {
		ptr = findResourceData(MKTAG('T','R','N','S'), roomptr);
		if (ptr)
			trans = ptr[0];
		else
			trans = 255;
	}

	// Actor Palette in HE 70 games
	if (_game.heversion == 70) {
		ptr = findResourceData(MKTAG('R','E','M','P'), roomptr);
		if (ptr) {
			for (i = 0; i < 256; i++)
				_HEV7ActorPalette[i] = *ptr++;
		} else {
			for (i = 0; i < 256; i++)
				_HEV7ActorPalette[i] = i;
		}
	}


	// WORKAROUND bug #1074444: The dreaded DOTT "Can't get teeth" bug
	// makes it impossible to go on playing w/o cheating in some way.
	// It's not quite clear what causes it, but the effect is that object
	// 182, the teeth, are still in class 32 (kObjectClassUntouchable),
	// when they shouldn't be. Luckily, bitvar69 is set to 1 if and only if
	// the teeth are trapped and have not yet been taken by the player. So
	// we can make use of that fact to fix the object class of obj 182.
	if (_game.id == GID_TENTACLE && _roomResource == 26 && readVar(0x8000 + 69)
			&& getClass(182, kObjectClassUntouchable)) {
		putClass(182, kObjectClassUntouchable, 0);
	}

	_gdi->roomChanged(roomptr);
	_gdi->setTransparentColor(trans);
}

/**
 * Init some dynamic room data after a room has been loaded.
 * E.g. the initial box data is loaded, the initial palette is set etc.
 * All of the things setup in here can be modified later on by scripts.
 * So it is not appropriate to call it after loading a savegame.
 */
void ScummEngine::resetRoomSubBlocks() {
	ResId i;
	const byte *ptr;
	byte *roomptr;

	// Determine the room and room script base address
	roomptr = getResourceAddress(rtRoom, _roomResource);
	if (!roomptr)
		error("Room %d: data not found (" __FILE__  ":%d)", _roomResource, __LINE__);

	//
	// Load box data
	//
	memset(_extraBoxFlags, 0, sizeof(_extraBoxFlags));

	_res->nukeResource(rtMatrix, 1);
	_res->nukeResource(rtMatrix, 2);
	if (_game.features & GF_SMALL_HEADER) {
		ptr = findResourceData(MKTAG('B','O','X','D'), roomptr);
		if (ptr) {
			byte numOfBoxes = *ptr;
			int size;
			if (_game.version == 3)
				size = numOfBoxes * SIZEOF_BOX_V3 + 1;
			else
				size = numOfBoxes * SIZEOF_BOX + 1;

			_res->createResource(rtMatrix, 2, size);
			memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
			ptr += size;

			size = getResourceDataSize(ptr - size - _resourceHeaderSize) - size;
			if (size > 0) {					// do this :)
				_res->createResource(rtMatrix, 1, size);
				memcpy(getResourceAddress(rtMatrix, 1), ptr, size);
			}

		}
	} else {
		ptr = findResourceData(MKTAG('B','O','X','D'), roomptr);
		if (ptr) {
			int size = getResourceDataSize(ptr);
			_res->createResource(rtMatrix, 2, size);
			roomptr = getResourceAddress(rtRoom, _roomResource);
			ptr = findResourceData(MKTAG('B','O','X','D'), roomptr);
			memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
		}

		ptr = findResourceData(MKTAG('B','O','X','M'), roomptr);
		if (ptr) {
			int size = getResourceDataSize(ptr);
			_res->createResource(rtMatrix, 1, size);
			roomptr = getResourceAddress(rtRoom, _roomResource);
			ptr = findResourceData(MKTAG('B','O','X','M'), roomptr);
			memcpy(getResourceAddress(rtMatrix, 1), ptr, size);
		}
	}

	//
	// Load scale data
	//
	for (i = 1; i < _res->_types[rtScaleTable].size(); i++)
		_res->nukeResource(rtScaleTable, i);

	ptr = findResourceData(MKTAG('S','C','A','L'), roomptr);
	if (ptr) {
		int s1, s2, y1, y2;
		if (_game.version == 8) {
			for (i = 1; i < _res->_types[rtScaleTable].size(); i++, ptr += 16) {
				s1 = READ_LE_UINT32(ptr);
				y1 = READ_LE_UINT32(ptr + 4);
				s2 = READ_LE_UINT32(ptr + 8);
				y2 = READ_LE_UINT32(ptr + 12);
				setScaleSlot(i, 0, y1, s1, 0, y2, s2);
			}
		} else {
			for (i = 1; i < _res->_types[rtScaleTable].size(); i++, ptr += 8) {
				s1 = READ_LE_UINT16(ptr);
				y1 = READ_LE_UINT16(ptr + 2);
				s2 = READ_LE_UINT16(ptr + 4);
				y2 = READ_LE_UINT16(ptr + 6);
				if (s1 || y1 || s2 || y2) {
					setScaleSlot(i, 0, y1, s1, 0, y2, s2);
				}
			}
		}
	}

	// We need to setup the current palette before initCycl for Indy4 Amiga.
	if (_PALS_offs || _CLUT_offs)
		setCurrentPalette(0);

	// Color cycling
	// HE 7.0 games load resources but don't use them.
	if (_game.version >= 4 && _game.heversion <= 62) {
		ptr = findResourceData(MKTAG('C','Y','C','L'), roomptr);
		if (ptr) {
			initCycl(ptr);
		}
	}

#ifdef ENABLE_HE
	// Polygons in HE 80+ games
	if (_game.heversion >= 80) {
		ptr = findResourceData(MKTAG('P','O','L','D'), roomptr);
		if (ptr) {
			((ScummEngine_v71he *)this)->_wiz->polygonLoad(ptr);
		}
	}
#endif
}


void ScummEngine_v3old::setupRoomSubBlocks() {
	const byte *ptr;
	byte *roomptr;
	const RoomHeader *rmhd;

	_ENCD_offs = 0;
	_EXCD_offs = 0;
	_EPAL_offs = 0;
	_CLUT_offs = 0;
	_PALS_offs = 0;

	// Determine the room and room script base address
	roomptr = getResourceAddress(rtRoom, _roomResource);
	if (!roomptr)
		error("Room %d: data not found (" __FILE__  ":%d)", _roomResource, __LINE__);

	//
	// Determine the room dimensions (width/height)
	//
	rmhd = (const RoomHeader *)(roomptr + 4);

	if (_game.version <= 1) {
		if (_game.platform == Common::kPlatformNES) {
			_roomWidth = READ_LE_UINT16(&(rmhd->old.width)) * 8;
			_roomHeight = READ_LE_UINT16(&(rmhd->old.height)) * 8;

			// HACK: To let our code work normal with narrow rooms we
			// adjust width. It will render garbage on right edge but we do
			// not render it anyway
			if (_roomWidth < 32 * 8)
				_roomWidth = 32 * 8;
		} else {
			_roomWidth = roomptr[4] * 8;
			_roomHeight = roomptr[5] * 8;
		}
	} else {
		_roomWidth = READ_LE_UINT16(&(rmhd->old.width));
		_roomHeight = READ_LE_UINT16(&(rmhd->old.height));
	}
	_numObjectsInRoom = roomptr[20];

	//
	// Find the room image data
	//
	if (_game.version <= 1) {
		_IM00_offs = 0;
	} else {
		_IM00_offs = READ_LE_UINT16(roomptr + 0x0A);
	}

	//
	// Look for an exit script
	//
	int EXCD_len = -1;
	if (_game.version <= 2) {
		_EXCD_offs = READ_LE_UINT16(roomptr + 0x18);
		EXCD_len = READ_LE_UINT16(roomptr + 0x1A) - _EXCD_offs + _resourceHeaderSize;	// HACK
	} else {
		_EXCD_offs = READ_LE_UINT16(roomptr + 0x19);
		EXCD_len = READ_LE_UINT16(roomptr + 0x1B) - _EXCD_offs + _resourceHeaderSize;	// HACK
	}
	if (_dumpScripts && _EXCD_offs)
		dumpResource("exit-", _roomResource, roomptr + _EXCD_offs - _resourceHeaderSize, EXCD_len);

	//
	// Look for an entry script
	//
	int ENCD_len = -1;
	if (_game.version <= 2) {
		_ENCD_offs = READ_LE_UINT16(roomptr + 0x1A);
		ENCD_len = READ_LE_UINT16(roomptr) - _ENCD_offs + _resourceHeaderSize; // HACK
	} else {
		_ENCD_offs = READ_LE_UINT16(roomptr + 0x1B);
		// FIXME - the following is a hack which assumes that immediately after
		// the entry script the first local script follows.
		int num_objects = *(roomptr + 20);
		int num_sounds = *(roomptr + 23);
		int num_scripts = *(roomptr + 24);
		ptr = roomptr + 29 + num_objects * 4 + num_sounds + num_scripts;
		ENCD_len = READ_LE_UINT16(ptr + 1) - _ENCD_offs + _resourceHeaderSize; // HACK
	}
	if (_dumpScripts && _ENCD_offs)
		dumpResource("entry-", _roomResource, roomptr + _ENCD_offs - _resourceHeaderSize, ENCD_len);

	//
	// Setup local scripts
	//

	// Determine the room script base address
	roomptr = getResourceAddress(rtRoom, _roomResource);

	memset(_localScriptOffsets, 0, sizeof(_localScriptOffsets));

	int num_objects = *(roomptr + 20);
	int num_sounds;
	int num_scripts;

	if (_game.version <= 2) {
		num_sounds = *(roomptr + 22);
		num_scripts = *(roomptr + 23);
		ptr = roomptr + 28 + num_objects * 4;
		while (num_sounds--)
			loadResource(rtSound, *ptr++);
		while (num_scripts--)
			loadResource(rtScript, *ptr++);
	} else /* if (_game.version == 3) */ {
		num_sounds = *(roomptr + 23);
		num_scripts = *(roomptr + 24);
		ptr = roomptr + 29 + num_objects * 4 + num_sounds + num_scripts;
		while (*ptr) {
			int id = *ptr;

			_localScriptOffsets[id - _numGlobalScripts] = READ_LE_UINT16(ptr + 1);
			ptr += 3;

			if (_dumpScripts) {
				char buf[32];
				sprintf(buf, "room-%d-", _roomResource);

				// HACK: to determine the sizes of the local scripts, we assume that
				// a) their order in the data file is the same as in the index
				// b) the last script at the same time is the last item in the room "header"
				int len = - (int)_localScriptOffsets[id - _numGlobalScripts] + _resourceHeaderSize;
				if (*ptr)
					len += READ_LE_UINT16(ptr + 1);
				else
					len += READ_LE_UINT16(roomptr);
				dumpResource(buf, id, roomptr + _localScriptOffsets[id - _numGlobalScripts] - _resourceHeaderSize, len);
			}
		}
	}

	_gdi->roomChanged(roomptr);
}

void ScummEngine_v3old::resetRoomSubBlocks() {
	const byte *ptr;
	byte *roomptr;

	// Determine the room and room script base address
	roomptr = getResourceAddress(rtRoom, _roomResource);
	if (!roomptr)
		error("Room %d: data not found (" __FILE__  ":%d)", _roomResource, __LINE__);

	// Reset room color for V1 zak
	if (_game.version <= 1)
		_roomPalette[0] = 0;

	//
	// Load box data
	//
	_res->nukeResource(rtMatrix, 1);
	_res->nukeResource(rtMatrix, 2);

	if (_game.version <= 2)
		ptr = roomptr + *(roomptr + 0x15);
	else
		ptr = roomptr + READ_LE_UINT16(roomptr + 0x15);
	if (ptr) {
		byte numOfBoxes = 0;
		int size;

		if (_game.version == 0) {
			// Count number of boxes
			while (*ptr != 0xFF) {
				numOfBoxes++;
				ptr += 5;
			}

			ptr = roomptr + *(roomptr + 0x15);
			size = numOfBoxes * SIZEOF_BOX_V0 + 1;

			_res->createResource(rtMatrix, 2, size + 1);
			getResourceAddress(rtMatrix, 2)[0] = numOfBoxes;
			memcpy(getResourceAddress(rtMatrix, 2) + 1, ptr, size);
		} else {
			numOfBoxes = *ptr;
			if (_game.version <= 2)
				size = numOfBoxes * SIZEOF_BOX_V2 + 1;
			else
				size = numOfBoxes * SIZEOF_BOX_V3 + 1;

			_res->createResource(rtMatrix, 2, size);
			memcpy(getResourceAddress(rtMatrix, 2), ptr, size);
		}

		ptr += size;
		if (_game.version == 0) {
			const byte *tmp = ptr;
			size = 0;

			// Compute matrix size
			for (int i = 0; i < numOfBoxes; i++) {
				while (*tmp != 0xFF) {
					size++;
					tmp++;
				}
				size++;
				tmp++;
			}
		} else if (_game.version <= 2) {
			size = numOfBoxes * (numOfBoxes + 1);
		} else {
			// FIXME. This is an evil HACK!!!
			size = (READ_LE_UINT16(roomptr + 0x0A) - READ_LE_UINT16(roomptr + 0x15)) - size;
		}

		if (size > 0) {					// do this :)
			_res->createResource(rtMatrix, 1, size);
			memcpy(getResourceAddress(rtMatrix, 1), ptr, size);
		}

	}

	//
	// No scale data in old bundle games
	//
	for (ResId id = 1; id < _res->_types[rtScaleTable].size(); id++)
		_res->nukeResource(rtScaleTable, id);

}

} // End of namespace Scumm
