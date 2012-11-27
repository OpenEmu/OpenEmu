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

#include "common/stream.h"
#include "common/system.h"
#include "common/func.h"
#include "common/serializer.h"
#include "graphics/thumbnail.h"

#include "sci/sci.h"
#include "sci/event.h"

#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/message.h"
#include "sci/engine/savegame.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm_types.h"
#include "sci/engine/script.h"	// for SCI_OBJ_EXPORTS and SCI_OBJ_SYNONYMS
#include "sci/graphics/helpers.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/ports.h"
#include "sci/parser/vocabulary.h"
#include "sci/sound/audio.h"
#include "sci/sound/music.h"

#ifdef ENABLE_SCI32
#include "sci/graphics/frameout.h"
#endif

namespace Sci {


#define VER(x) Common::Serializer::Version(x)


#pragma mark -

// Experimental hack: Use syncWithSerializer to sync. By default, this assume
// the object to be synced is a subclass of Serializable and thus tries to invoke
// the saveLoadWithSerializer() method. But it is possible to specialize this
// template function to handle stuff that is not implementing that interface.
template<typename T>
void syncWithSerializer(Common::Serializer &s, T &obj) {
	obj.saveLoadWithSerializer(s);
}

// By default, sync using syncWithSerializer, which in turn can easily be overloaded.
template<typename T>
struct DefaultSyncer : Common::BinaryFunction<Common::Serializer, T, void> {
	void operator()(Common::Serializer &s, T &obj) const {
		//obj.saveLoadWithSerializer(s);
		syncWithSerializer(s, obj);
	}
};

/**
 * Sync a Common::Array using a Common::Serializer.
 * When saving, this writes the length of the array, then syncs (writes) all entries.
 * When loading, it loads the length of the array, then resizes it accordingly, before
 * syncing all entries.
 *
 * Note: This shouldn't be in common/array.h nor in common/serializer.h, after
 * all, not all code using arrays wants to use the serializer, and vice versa.
 * But we could put this into a separate header file in common/ at some point.
 * Something like common/serializer-extras.h or so.
 *
 * TODO: Add something like this for lists, queues....
 */
template<typename T, class Syncer = DefaultSyncer<T> >
struct ArraySyncer : Common::BinaryFunction<Common::Serializer, T, void> {
	void operator()(Common::Serializer &s, Common::Array<T> &arr) const {
		uint len = arr.size();
		s.syncAsUint32LE(len);
		Syncer sync;

		// Resize the array if loading.
		if (s.isLoading())
			arr.resize(len);

		typename Common::Array<T>::iterator i;
		for (i = arr.begin(); i != arr.end(); ++i) {
			sync(s, *i);
		}
	}
};

// Convenience wrapper
template<typename T>
void syncArray(Common::Serializer &s, Common::Array<T> &arr) {
	ArraySyncer<T> sync;
	sync(s, arr);
}


template<>
void syncWithSerializer(Common::Serializer &s, reg_t &obj) {
	// Segment and offset are accessed directly here
	s.syncAsUint16LE(obj._segment);
	s.syncAsUint16LE(obj._offset);
}

template<>
void syncWithSerializer(Common::Serializer &s, synonym_t &obj) {
	s.syncAsUint16LE(obj.replaceant);
	s.syncAsUint16LE(obj.replacement);
}

void SegManager::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.isLoading()) {
		resetSegMan();

		// Reset _scriptSegMap, to be restored below
		_scriptSegMap.clear();

#ifdef ENABLE_SCI32
		// Clear any planes/screen items currently showing so they
		// don't show up after the load.
		if (getSciVersion() >= SCI_VERSION_2)
			g_sci->_gfxFrameout->clear();
#endif
	}

	s.skip(4, VER(14), VER(18));		// OBSOLETE: Used to be _exportsAreWide

	uint sync_heap_size = _heap.size();
	s.syncAsUint32LE(sync_heap_size);
	_heap.resize(sync_heap_size);
	for (uint i = 0; i < sync_heap_size; ++i) {
		SegmentObj *&mobj = _heap[i];

		// Sync the segment type
		SegmentType type = (s.isSaving() && mobj) ? mobj->getType() : SEG_TYPE_INVALID;
		s.syncAsUint32LE(type);

		if (type == SEG_TYPE_HUNK) {
			// Don't save or load HunkTable segments
			continue;
		} else if (type == SEG_TYPE_INVALID) {
			// If we were saving and mobj == 0, or if we are loading and this is an
			// entry marked as empty -> skip to next
			continue;
		} else if (type == 5) {
			// Don't save or load the obsolete system string segments
			if (s.isSaving()) {
				continue;
			} else {
				// Old saved game. Skip the data.
				Common::String tmp;
				for (int j = 0; j < 4; j++) {
					s.syncString(tmp);	// OBSOLETE: name
					s.skip(4);			// OBSOLETE: maxSize
					s.syncString(tmp);	// OBSOLETE: value
				}
				_heap[i] = NULL;	// set as freed
				continue;
			}
#ifdef ENABLE_SCI32
		} else if (type == SEG_TYPE_ARRAY) {
			// Set the correct segment for SCI32 arrays
			_arraysSegId = i;
		} else if (type == SEG_TYPE_STRING) {
			// Set the correct segment for SCI32 strings
			_stringSegId = i;
#endif
		}

		if (s.isLoading())
			mobj = SegmentObj::createSegmentObj(type);

		assert(mobj);

		// Let the object sync custom data. Scripts are loaded at this point.
		mobj->saveLoadWithSerializer(s);

		if (type == SEG_TYPE_SCRIPT) {
			Script *scr = (Script *)mobj;

			// If we are loading a script, perform some extra steps
			if (s.isLoading()) {
				// Hook the script up in the script->segment map
				_scriptSegMap[scr->getScriptNumber()] = i;

				ObjMap objects = scr->getObjectMap();
				for (ObjMap::iterator it = objects.begin(); it != objects.end(); ++it)
					it->_value.syncBaseObject(scr->getBuf(it->_value.getPos().getOffset()));

			}

			// Sync the script's string heap
			if (s.getVersion() >= 28)
				scr->syncStringHeap(s);
		}
	}

	s.syncAsSint32LE(_clonesSegId);
	s.syncAsSint32LE(_listsSegId);
	s.syncAsSint32LE(_nodesSegId);

	syncArray<Class>(s, _classTable);

	// Now that all scripts are loaded, init their objects
	for (uint i = 0; i < _heap.size(); i++) {
		if (!_heap[i] ||  _heap[i]->getType() != SEG_TYPE_SCRIPT)
			continue;

		Script *scr = (Script *)_heap[i];
		scr->syncLocalsBlock(this);

		ObjMap objects = scr->getObjectMap();
		for (ObjMap::iterator it = objects.begin(); it != objects.end(); ++it) {
			reg_t addr = it->_value.getPos();
			Object *obj = scr->scriptObjInit(addr, false);

			if (getSciVersion() < SCI_VERSION_1_1) {
				if (!obj->initBaseObject(this, addr, false)) {
					// TODO/FIXME: This should not be happening at all. It might indicate a possible issue
					// with the garbage collector. It happens for example in LSL5 (German, perhaps English too).
					warning("Failed to locate base object for object at %04X:%04X; skipping", PRINT_REG(addr));
					objects.erase(addr.toUint16());
				}
			}
		}
	}
}


template<>
void syncWithSerializer(Common::Serializer &s, Class &obj) {
	s.syncAsSint32LE(obj.script);
	syncWithSerializer(s, obj.reg);
}

static void sync_SavegameMetadata(Common::Serializer &s, SavegameMetadata &obj) {
	s.syncString(obj.name);
	s.syncVersion(CURRENT_SAVEGAME_VERSION);
	obj.version = s.getVersion();
	s.syncString(obj.gameVersion);
	s.syncAsSint32LE(obj.saveDate);
	s.syncAsSint32LE(obj.saveTime);
	if (s.getVersion() < 22) {
		obj.gameObjectOffset = 0;
		obj.script0Size = 0;
	} else {
		s.syncAsUint16LE(obj.gameObjectOffset);
		s.syncAsUint16LE(obj.script0Size);
	}

	// Playtime
	obj.playTime = 0;
	if (s.isLoading()) {
		if (s.getVersion() >= 26)
			s.syncAsUint32LE(obj.playTime);
	} else {
		obj.playTime = g_engine->getTotalPlayTime() / 1000;
		s.syncAsUint32LE(obj.playTime);
	}
}

void EngineState::saveLoadWithSerializer(Common::Serializer &s) {
	Common::String tmp;
	s.syncString(tmp, VER(14), VER(23));			// OBSOLETE: Used to be gameVersion

	if (getSciVersion() <= SCI_VERSION_1_1) {
		// Save/Load picPort as well for SCI0-SCI1.1. Necessary for Castle of Dr. Brain,
		// as the picPort has been changed when loading during the intro
		int16 picPortTop, picPortLeft;
		Common::Rect picPortRect;

		if (s.isSaving())
			picPortRect = g_sci->_gfxPorts->kernelGetPicWindow(picPortTop, picPortLeft);

		s.syncAsSint16LE(picPortRect.top);
		s.syncAsSint16LE(picPortRect.left);
		s.syncAsSint16LE(picPortRect.bottom);
		s.syncAsSint16LE(picPortRect.right);
		s.syncAsSint16LE(picPortTop);
		s.syncAsSint16LE(picPortLeft);

		if (s.isLoading())
			g_sci->_gfxPorts->kernelSetPicWindow(picPortRect, picPortTop, picPortLeft, false);
	}

	_segMan->saveLoadWithSerializer(s);

	g_sci->_soundCmd->syncPlayList(s);
	g_sci->_gfxPalette->saveLoadWithSerializer(s);
}

void Vocabulary::saveLoadWithSerializer(Common::Serializer &s) {
	syncArray<synonym_t>(s, _synonyms);
}

void LocalVariables::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(script_id);
	syncArray<reg_t>(s, _locals);
}

void Object::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_flags);
	syncWithSerializer(s, _pos);
	s.syncAsSint32LE(_methodCount);		// that's actually a uint16

	syncArray<reg_t>(s, _variables);
}

template<>
void syncWithSerializer(Common::Serializer &s, SegmentObjTable<Clone>::Entry &obj) {
	s.syncAsSint32LE(obj.next_free);

	syncWithSerializer<Object>(s, obj);
}

template<>
void syncWithSerializer(Common::Serializer &s, SegmentObjTable<List>::Entry &obj) {
	s.syncAsSint32LE(obj.next_free);

	syncWithSerializer(s, obj.first);
	syncWithSerializer(s, obj.last);
}

template<>
void syncWithSerializer(Common::Serializer &s, SegmentObjTable<Node>::Entry &obj) {
	s.syncAsSint32LE(obj.next_free);

	syncWithSerializer(s, obj.pred);
	syncWithSerializer(s, obj.succ);
	syncWithSerializer(s, obj.key);
	syncWithSerializer(s, obj.value);
}

#ifdef ENABLE_SCI32
template<>
void syncWithSerializer(Common::Serializer &s, SegmentObjTable<SciArray<reg_t> >::Entry &obj) {
	s.syncAsSint32LE(obj.next_free);

	byte type = 0;
	uint32 size = 0;

	if (s.isSaving()) {
		type = (byte)obj.getType();
		size = obj.getSize();
	}
	s.syncAsByte(type);
	s.syncAsUint32LE(size);
	if (s.isLoading()) {
		obj.setType((int8)type);

		// HACK: Skip arrays that have a negative type
		if ((int8)type < 0)
			return;

		obj.setSize(size);
	}

	for (uint32 i = 0; i < size; i++) {
		reg_t value;

		if (s.isSaving())
			value = obj.getValue(i);

		syncWithSerializer(s, value);

		if (s.isLoading())
			obj.setValue(i, value);
	}
}

template<>
void syncWithSerializer(Common::Serializer &s, SegmentObjTable<SciString>::Entry &obj) {
	s.syncAsSint32LE(obj.next_free);

	uint32 size = 0;

	if (s.isSaving()) {
		size = obj.getSize();
		s.syncAsUint32LE(size);
	} else {
		s.syncAsUint32LE(size);
		obj.setSize(size);
	}

	for (uint32 i = 0; i < size; i++) {
		char value = 0;

		if (s.isSaving())
			value = obj.getValue(i);

		s.syncAsByte(value);

		if (s.isLoading())
			obj.setValue(i, value);
	}
}
#endif

template<typename T>
void sync_Table(Common::Serializer &s, T &obj) {
	s.syncAsSint32LE(obj.first_free);
	s.syncAsSint32LE(obj.entries_used);

	syncArray<typename T::Entry>(s, obj._table);
}

void CloneTable::saveLoadWithSerializer(Common::Serializer &s) {
	sync_Table<CloneTable>(s, *this);
}

void NodeTable::saveLoadWithSerializer(Common::Serializer &s) {
	sync_Table<NodeTable>(s, *this);
}

void ListTable::saveLoadWithSerializer(Common::Serializer &s) {
	sync_Table<ListTable>(s, *this);
}

void HunkTable::saveLoadWithSerializer(Common::Serializer &s) {
	// Do nothing, hunk tables are not actually saved nor loaded.
}

void Script::syncStringHeap(Common::Serializer &s) {
	if (getSciVersion() < SCI_VERSION_1_1) {
		// Sync all of the SCI_OBJ_STRINGS blocks
		byte *buf = _buf;
		bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

		if (oldScriptHeader)
			buf += 2;

		do {
			int blockType = READ_LE_UINT16(buf);
			int blockSize;
			if (blockType == 0)
				break;

			blockSize = READ_LE_UINT16(buf + 2);
			assert(blockSize > 0);

			if (blockType == SCI_OBJ_STRINGS)
				s.syncBytes(buf, blockSize);

			buf += blockSize;

			if (_buf - buf == 0)
				break;
		} while (1);

 	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1){
		// Strings in SCI1.1 come after the object instances
		byte *buf = _heapStart + 4 + READ_SCI11ENDIAN_UINT16(_heapStart + 2) * 2;

		// Skip all of the objects
		while (READ_SCI11ENDIAN_UINT16(buf) == SCRIPT_OBJECT_MAGIC_NUMBER)
			buf += READ_SCI11ENDIAN_UINT16(buf + 2) * 2;

		// Now, sync everything till the end of the buffer
		s.syncBytes(buf, _heapSize - (buf - _heapStart));
	} else if (getSciVersion() == SCI_VERSION_3) {
		warning("TODO: syncStringHeap(): Implement SCI3 variant");
	}
}

void Script::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_nr);

	if (s.isLoading())
		load(_nr, g_sci->getResMan());
	s.skip(4, VER(14), VER(22));		// OBSOLETE: Used to be _bufSize
	s.skip(4, VER(14), VER(22));		// OBSOLETE: Used to be _scriptSize
	s.skip(4, VER(14), VER(22));		// OBSOLETE: Used to be _heapSize

	s.skip(4, VER(14), VER(19));		// OBSOLETE: Used to be _numExports
	s.skip(4, VER(14), VER(19));		// OBSOLETE: Used to be _numSynonyms
	s.syncAsSint32LE(_lockers);

	// Sync _objects. This is a hashmap, and we use the following on disk format:
	// First we store the number of items in the hashmap, then we store each
	// object (which is an 'Object' instance). For loading, we take advantage
	// of the fact that the key of each Object obj is just obj._pos.offset !
	// By "chance" this format is identical to the format used to sync Common::Array<>,
	// hence we can still old savegames with identical code :).

	uint numObjs = _objects.size();
	s.syncAsUint32LE(numObjs);

	if (s.isLoading()) {
		_objects.clear();
		Object tmp;
		for (uint i = 0; i < numObjs; ++i) {
			syncWithSerializer(s, tmp);
			_objects[tmp.getPos().getOffset()] = tmp;
		}
	} else {
		ObjMap::iterator it;
		const ObjMap::iterator end = _objects.end();
		for (it = _objects.begin(); it != end; ++it) {
			syncWithSerializer(s, it->_value);
		}
	}

	s.skip(4, VER(14), VER(20));		// OBSOLETE: Used to be _localsOffset
	s.syncAsSint32LE(_localsSegment);

	s.syncAsSint32LE(_markedAsDeleted);
}

void DynMem::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_size);
	s.syncString(_description);
	if (!_buf && _size) {
		_buf = (byte *)calloc(_size, 1);
	}
	if (_size)
		s.syncBytes(_buf, _size);
}

void DataStack::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsUint32LE(_capacity);
	if (s.isLoading()) {
		free(_entries);
		_entries = (reg_t *)calloc(_capacity, sizeof(reg_t));
	}
}

#pragma mark -

void SciMusic::saveLoadWithSerializer(Common::Serializer &s) {
	// Sync song lib data. When loading, the actual song lib will be initialized
	// afterwards in gamestate_restore()
	int songcount = 0;
	byte masterVolume = soundGetMasterVolume();
	byte reverb = _pMidiDrv->getReverb();

	if (s.isSaving()) {
		s.syncAsByte(_soundOn);
		s.syncAsByte(masterVolume);
		s.syncAsByte(reverb, VER(17));
	} else if (s.isLoading()) {
		if (s.getVersion() >= 15) {
			s.syncAsByte(_soundOn);
			s.syncAsByte(masterVolume);
			reverb = 0;
			s.syncAsByte(reverb, VER(17));
		} else {
			_soundOn = true;
			masterVolume = 15;
			reverb = 0;
		}

		soundSetSoundOn(_soundOn);
		soundSetMasterVolume(masterVolume);
		setGlobalReverb(reverb);
	}

	if (s.isSaving())
		songcount = _playList.size();
	s.syncAsUint32LE(songcount);

	if (s.isLoading())
		clearPlayList();

	Common::StackLock lock(_mutex);

	if (s.isLoading()) {
		for (int i = 0; i < songcount; i++) {
			MusicEntry *curSong = new MusicEntry();
			curSong->saveLoadWithSerializer(s);
			_playList.push_back(curSong);
		}
	} else {
		for (int i = 0; i < songcount; i++) {
			_playList[i]->saveLoadWithSerializer(s);
		}
	}
}

void MusicEntry::saveLoadWithSerializer(Common::Serializer &s) {
	syncWithSerializer(s, soundObj);
	s.syncAsSint16LE(resourceId);
	s.syncAsSint16LE(dataInc);
	s.syncAsSint16LE(ticker);
	s.syncAsSint16LE(signal, VER(17));
	s.syncAsByte(priority);
	s.syncAsSint16LE(loop, VER(17));
	s.syncAsByte(volume);
	s.syncAsByte(hold, VER(17));
	s.syncAsByte(fadeTo);
	s.syncAsSint16LE(fadeStep);
	s.syncAsSint32LE(fadeTicker);
	s.syncAsSint32LE(fadeTickerStep);
	s.syncAsByte(status);

	// pMidiParser and pStreamAud will be initialized when the
	// sound list is reconstructed in gamestate_restore()
	if (s.isLoading()) {
		soundRes = 0;
		pMidiParser = 0;
		pStreamAud = 0;
		reverb = -1;	// invalid reverb, will be initialized in processInitSound()
	}
}

void SoundCommandParser::syncPlayList(Common::Serializer &s) {
	_music->saveLoadWithSerializer(s);
}

void SoundCommandParser::reconstructPlayList() {
	Common::StackLock lock(_music->_mutex);

	const MusicList::iterator end = _music->getPlayListEnd();
	for (MusicList::iterator i = _music->getPlayListStart(); i != end; ++i) {
		initSoundResource(*i);

		if ((*i)->status == kSoundPlaying) {
			// Sync the sound object's selectors related to playing with the stored
			// ones in the playlist, as they may have been invalidated when loading.
			// Refer to bug #3104624.
			writeSelectorValue(_segMan, (*i)->soundObj, SELECTOR(loop), (*i)->loop);
			writeSelectorValue(_segMan, (*i)->soundObj, SELECTOR(priority), (*i)->priority);
			if (_soundVersion >= SCI_VERSION_1_EARLY)
				writeSelectorValue(_segMan, (*i)->soundObj, SELECTOR(vol), (*i)->volume);

			processPlaySound((*i)->soundObj);
		}
	}
}

#ifdef ENABLE_SCI32
void ArrayTable::saveLoadWithSerializer(Common::Serializer &ser) {
	if (ser.getVersion() < 18)
		return;

	sync_Table<ArrayTable>(ser, *this);
}

void StringTable::saveLoadWithSerializer(Common::Serializer &ser) {
	if (ser.getVersion() < 18)
		return;

	sync_Table<StringTable>(ser, *this);
}
#endif

void GfxPalette::palVarySaveLoadPalette(Common::Serializer &s, Palette *palette) {
	s.syncBytes(palette->mapping, 256);
	s.syncAsUint32LE(palette->timestamp);
	for (int i = 0; i < 256; i++) {
		s.syncAsByte(palette->colors[i].used);
		s.syncAsByte(palette->colors[i].r);
		s.syncAsByte(palette->colors[i].g);
		s.syncAsByte(palette->colors[i].b);
	}
	s.syncBytes(palette->intensity, 256);
}

void GfxPalette::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.getVersion() >= 25) {
		// We need to save intensity of the _sysPalette at least for kq6 when entering the dark cave (room 390)
		//  from room 340. scripts will set intensity to 60 for this room and restore them when leaving.
		//  Sierra SCI is also doing this (although obviously not for SCI0->SCI01 games, still it doesn't hurt
		//  to save it everywhere). ffs. bug #3072868
		s.syncBytes(_sysPalette.intensity, 256);
	}
	if (s.getVersion() >= 24) {
		if (s.isLoading() && _palVaryResourceId != -1)
			palVaryRemoveTimer();

		s.syncAsSint32LE(_palVaryResourceId);
		if (_palVaryResourceId != -1) {
			palVarySaveLoadPalette(s, &_palVaryOriginPalette);
			palVarySaveLoadPalette(s, &_palVaryTargetPalette);
			s.syncAsSint16LE(_palVaryStep);
			s.syncAsSint16LE(_palVaryStepStop);
			s.syncAsSint16LE(_palVaryDirection);
			s.syncAsUint16LE(_palVaryTicks);
			s.syncAsSint32LE(_palVaryPaused);
		}

		_palVarySignal = 0;

		if (s.isLoading() && _palVaryResourceId != -1) {
			palVaryInstallTimer();
		}
	}
}

void GfxPorts::saveLoadWithSerializer(Common::Serializer &s) {
	if (s.isLoading())
		reset();	// remove all script generated windows

	if (s.getVersion() >= 27) {
		uint windowCount = 0;
		uint id = PORTS_FIRSTSCRIPTWINDOWID;
		if (s.isSaving()) {
			while (id < _windowsById.size()) {
				if (_windowsById[id])
					windowCount++;
				id++;
			}
		}
		// Save/Restore window count
		s.syncAsUint32LE(windowCount);

		if (s.isSaving()) {
			id = PORTS_FIRSTSCRIPTWINDOWID;
			while (id < _windowsById.size()) {
				if (_windowsById[id]) {
					Window *window = (Window *)_windowsById[id];
					window->saveLoadWithSerializer(s);
				}
				id++;
			}
		} else {
			id = PORTS_FIRSTSCRIPTWINDOWID;
			while (windowCount) {
				Window *window = new Window(0);
				window->saveLoadWithSerializer(s);

				// add enough entries inside _windowsById as needed
				while (id <= window->id) {
					_windowsById.push_back(0);
					id++;
				}
				_windowsById[window->id] = window;
				// _windowList may not be 100% correct using that way of restoring
				//  saving/restoring ports won't work perfectly anyway, because the contents
				//  of the window can only get repainted by the scripts and they dont do that
				//  so we will get empty, transparent windows instead. So perfect window order
				//  shouldn't really matter
				if (window->counterTillFree) {
					_freeCounter++;
				} else {
					if (window->wndStyle & SCI_WINDOWMGR_STYLE_TOPMOST)
						_windowList.push_front(window);
					else
						_windowList.push_back(window);
				}

				windowCount--;
			}
		}
	}
}

void SegManager::reconstructStack(EngineState *s) {
	DataStack *stack = (DataStack *)(_heap[findSegmentByType(SEG_TYPE_STACK)]);
	s->stack_base = stack->_entries;
	s->stack_top = s->stack_base + stack->_capacity;
}

void SegManager::reconstructClones() {
	for (uint i = 0; i < _heap.size(); i++) {
		SegmentObj *mobj = _heap[i];
		if (mobj && mobj->getType() == SEG_TYPE_CLONES) {
			CloneTable *ct = (CloneTable *)mobj;

			for (uint j = 0; j < ct->_table.size(); j++) {
				// Check if the clone entry is used
				uint entryNum = (uint)ct->first_free;
				bool isUsed = true;
				while (entryNum != ((uint) CloneTable::HEAPENTRY_INVALID)) {
					if (entryNum == j) {
						isUsed = false;
						break;
					}
					entryNum = ct->_table[entryNum].next_free;
				}

				if (!isUsed)
					continue;

				CloneTable::Entry &seeker = ct->_table[j];
				const Object *baseObj = getObject(seeker.getSpeciesSelector());
				seeker.cloneFromObject(baseObj);
				if (!baseObj) {
					// Can happen when loading some KQ6 savegames
					warning("Clone entry without a base class: %d", j);
				}
			}	// end for
		}	// end if
	}	// end for
}


#pragma mark -


bool gamestate_save(EngineState *s, Common::WriteStream *fh, const Common::String &savename, const Common::String &version) {
	TimeDate curTime;
	g_system->getTimeAndDate(curTime);

	SavegameMetadata meta;
	meta.version = CURRENT_SAVEGAME_VERSION;
	meta.name = savename;
	meta.gameVersion = version;
	meta.saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	meta.saveTime = ((curTime.tm_hour & 0xFF) << 16) | (((curTime.tm_min) & 0xFF) << 8) | ((curTime.tm_sec) & 0xFF);

	Resource *script0 = g_sci->getResMan()->findResource(ResourceId(kResourceTypeScript, 0), false);
	meta.script0Size = script0->size;
	meta.gameObjectOffset = g_sci->getGameObject().getOffset();

	// Checking here again
	if (s->executionStackBase) {
		warning("Cannot save from below kernel function");
		return false;
	}

	Common::Serializer ser(0, fh);
	sync_SavegameMetadata(ser, meta);
	Graphics::saveThumbnail(*fh);
	s->saveLoadWithSerializer(ser);		// FIXME: Error handling?
	if (g_sci->_gfxPorts)
		g_sci->_gfxPorts->saveLoadWithSerializer(ser);
	Vocabulary *voc = g_sci->getVocabulary();
	if (voc)
		voc->saveLoadWithSerializer(ser);

	return true;
}

extern void showScummVMDialog(const Common::String &message);

void gamestate_restore(EngineState *s, Common::SeekableReadStream *fh) {
	SavegameMetadata meta;

	Common::Serializer ser(fh, 0);
	sync_SavegameMetadata(ser, meta);

	if (fh->eos()) {
		s->r_acc = TRUE_REG;	// signal failure
		return;
	}

	if ((meta.version < MINIMUM_SAVEGAME_VERSION) ||
	    (meta.version > CURRENT_SAVEGAME_VERSION)) {
		/*
		if (meta.version < MINIMUM_SAVEGAME_VERSION)
			warning("Old savegame version detected, unable to load it");
		else
			warning("Savegame version is %d, maximum supported is %0d", meta.version, CURRENT_SAVEGAME_VERSION);
		*/

		showScummVMDialog("The format of this saved game is obsolete, unable to load it");

		s->r_acc = TRUE_REG;	// signal failure
		return;
	}

	if (meta.gameObjectOffset > 0 && meta.script0Size > 0) {
		Resource *script0 = g_sci->getResMan()->findResource(ResourceId(kResourceTypeScript, 0), false);
		if (script0->size != meta.script0Size || g_sci->getGameObject().getOffset() != meta.gameObjectOffset) {
			//warning("This saved game was created with a different version of the game, unable to load it");

			showScummVMDialog("This saved game was created with a different version of the game, unable to load it");

			s->r_acc = TRUE_REG;	// signal failure
			return;
		}
	}

	// We don't need the thumbnail here, so just read it and discard it
	Graphics::skipThumbnail(*fh);

	s->reset(true);
	s->saveLoadWithSerializer(ser);	// FIXME: Error handling?

	// Now copy all current state information

	s->_segMan->reconstructStack(s);
	s->_segMan->reconstructClones();
	s->initGlobals();
	s->gcCountDown = GC_INTERVAL - 1;

	// Time state:
	s->lastWaitTime = g_system->getMillis();
	s->_screenUpdateTime = g_system->getMillis();
	g_engine->setTotalPlayTime(meta.playTime * 1000);

	if (g_sci->_gfxPorts)
		g_sci->_gfxPorts->saveLoadWithSerializer(ser);

	Vocabulary *voc = g_sci->getVocabulary();
	if (ser.getVersion() >= 30 && voc)
		voc->saveLoadWithSerializer(ser);

	g_sci->_soundCmd->reconstructPlayList();

	// Message state:
	delete s->_msgState;
	s->_msgState = new MessageState(s->_segMan);

	// System strings:
	s->_segMan->initSysStrings();

	s->abortScriptProcessing = kAbortLoadGame;

	// signal restored game to game scripts
	s->gameIsRestarting = GAMEISRESTARTING_RESTORE;
}

bool get_savegame_metadata(Common::SeekableReadStream *stream, SavegameMetadata *meta) {
	assert(stream);
	assert(meta);

	Common::Serializer ser(stream, 0);
	sync_SavegameMetadata(ser, *meta);

	if (stream->eos())
		return false;

	if ((meta->version < MINIMUM_SAVEGAME_VERSION) ||
	    (meta->version > CURRENT_SAVEGAME_VERSION)) {
		if (meta->version < MINIMUM_SAVEGAME_VERSION)
			warning("Old savegame version detected- can't load");
		else
			warning("Savegame version is %d- maximum supported is %0d", meta->version, CURRENT_SAVEGAME_VERSION);

		return false;
	}

	return true;
}

} // End of namespace Sci
