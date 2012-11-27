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

#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/zlib.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/imuse/imuse.h"
#include "scumm/player_towns.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/saveload.h"
#include "scumm/scumm_v0.h"
#include "scumm/scumm_v7.h"
#include "scumm/sound.h"
#include "scumm/he/sprite_he.h"
#include "scumm/verbs.h"

#include "audio/mixer.h"

#include "backends/audiocd/audiocd.h"

#include "graphics/thumbnail.h"

namespace Scumm {

struct SaveGameHeader {
	uint32 type;
	uint32 size;
	uint32 ver;
	char name[32];
};

struct SaveInfoSection {
	uint32 type;
	uint32 version;
	uint32 size;

	uint32 timeTValue;  // Obsolete since version 2, but kept for compatibility
	uint32 playtime;

	uint32 date;
	uint16 time;
};

#define SaveInfoSectionSize (4+4+4 + 4+4 + 4+2)

#define INFOSECTION_VERSION 2

#pragma mark -

Common::Error ScummEngine::loadGameState(int slot) {
	requestLoad(slot);
	return Common::kNoError;
}

bool ScummEngine::canLoadGameStateCurrently() {
	// FIXME: For now always allow loading in V0-V3 games
	// FIXME: Actually, we might wish to support loading in more places.
	// As long as we are sure it won't cause any problems... Are we
	// aware of *any* spots where loading is not supported?

	// HE games are limited to original load and save interface only,
	// due to numerous glitches (see bug #1726909) that can occur.
	//
	// Except the earliest HE Games (3DO and initial DOS version of
	// puttputt), which didn't offer scripted load/save screens.
	if (_game.heversion >= 62)
		return false;

	// COMI always disables saving/loading (to tell the truth:
	// the main menu) via its scripts, thus we need to make an
	// exception here. This the same forced overwriting of the
	// script decisions as in ScummEngine::processKeyboard.
	if (_game.id == GID_CMI)
		return true;

	return (VAR_MAINMENU_KEY == 0xFF || VAR(VAR_MAINMENU_KEY) != 0);
}

Common::Error ScummEngine::saveGameState(int slot, const Common::String &desc) {
	requestSave(slot, desc);
	return Common::kNoError;
}

bool ScummEngine::canSaveGameStateCurrently() {
	// FIXME: For now always allow loading in V0-V3 games
	// TODO: Should we disallow saving in some more places,
	// e.g. when a SAN movie is playing? Not sure whether the
	// original EXE allowed this.

	// HE games are limited to original load and save interface only,
	// due to numerous glitches (see bug #1726909) that can occur.
	//
	// Except the earliest HE Games (3DO and initial DOS version of
	// puttputt), which didn't offer scripted load/save screens.
	if (_game.heversion >= 62)
		return false;

	// COMI always disables saving/loading (to tell the truth:
	// the main menu) via its scripts, thus we need to make an
	// exception here. This the same forced overwriting of the
	// script decisions as in ScummEngine::processKeyboard.
	if (_game.id == GID_CMI)
		return true;

	// SCUMM v4+ doesn't allow saving in room 0 or if
	// VAR(VAR_MAINMENU_KEY) to set to zero.
	return (VAR_MAINMENU_KEY == 0xFF || (VAR(VAR_MAINMENU_KEY) != 0 && _currentRoom != 0));
}


void ScummEngine::requestSave(int slot, const Common::String &name) {
	_saveLoadSlot = slot;
	_saveTemporaryState = false;
	_saveLoadFlag = 1;		// 1 for save
	_saveLoadDescription = name;
}

void ScummEngine::requestLoad(int slot) {
	_saveLoadSlot = slot;
	_saveTemporaryState = false;
	_saveLoadFlag = 2;		// 2 for load
}

static bool saveSaveGameHeader(Common::OutSaveFile *out, SaveGameHeader &hdr) {
	hdr.type = MKTAG('S','C','V','M');
	hdr.size = 0;
	hdr.ver = CURRENT_VER;

	out->writeUint32BE(hdr.type);
	out->writeUint32LE(hdr.size);
	out->writeUint32LE(hdr.ver);
	out->write(hdr.name, sizeof(hdr.name));
	return true;
}

bool ScummEngine::saveState(Common::OutSaveFile *out, bool writeHeader) {
	SaveGameHeader hdr;

	if (writeHeader) {
		Common::strlcpy(hdr.name, _saveLoadDescription.c_str(), sizeof(hdr.name));
		saveSaveGameHeader(out, hdr);
	}
#if !defined(__DS__) && !defined(__N64__) /* && !defined(__PLAYSTATION2__) */
	Graphics::saveThumbnail(*out);
#endif
	saveInfos(out);

	Serializer ser(0, out, CURRENT_VER);
	saveOrLoad(&ser);
	return true;
}

bool ScummEngine::saveState(int slot, bool compat) {
	bool saveFailed;
	Common::String filename;
	Common::OutSaveFile *out;

	pauseEngine(true);

	if (_saveLoadSlot == 255) {
		// Allow custom filenames for save game system in HE Games
		filename = _saveLoadFileName;
	} else {
		filename = makeSavegameName(slot, compat);
	}
	if (!(out = _saveFileMan->openForSaving(filename)))
		return false;

	saveFailed = false;
	if (!saveState(out))
		saveFailed = true;

	out->finalize();
	if (out->err())
		saveFailed = true;
	delete out;

	if (saveFailed) {
		debug(1, "State save as '%s' FAILED", filename.c_str());
		return false;
	}
	debug(1, "State saved as '%s'", filename.c_str());

	pauseEngine(false);

	return true;
}


void ScummEngine_v4::prepareSavegame() {
	Common::MemoryWriteStreamDynamic *memStream;
	Common::WriteStream *writeStream;

	// free memory of the last prepared savegame
	delete _savePreparedSavegame;
	_savePreparedSavegame = NULL;

	// store headerless savegame in a compressed memory stream
	memStream = new Common::MemoryWriteStreamDynamic();
	writeStream = Common::wrapCompressedWriteStream(memStream);
	if (saveState(writeStream, false)) {
		// we have to finalize the compression-stream first, otherwise the internal
		// memory-stream pointer will be zero (Important: flush() does not work here!).
		writeStream->finalize();
		if (!writeStream->err()) {
			// wrap uncompressing MemoryReadStream around the savegame data
			_savePreparedSavegame = Common::wrapCompressedReadStream(
				new Common::MemoryReadStream(memStream->getData(), memStream->size(), DisposeAfterUse::YES));
		}
	}
	// free the CompressedWriteStream and MemoryWriteStreamDynamic
	// but not the memory stream's internal buffer
	delete writeStream;
}

bool ScummEngine_v4::savePreparedSavegame(int slot, char *desc) {
	bool success;
	Common::String filename;
	Common::OutSaveFile *out;
	SaveGameHeader hdr;
	uint32 nread, nwritten;

	out = 0;
	success = true;

	// check if savegame was successfully stored in memory
	if (!_savePreparedSavegame)
		success = false;

	// open savegame file
	if (success) {
		filename = makeSavegameName(slot, false);
		if (!(out = _saveFileMan->openForSaving(filename))) {
			success = false;
		}
	}

	// write header to file
	if (success) {
		memset(hdr.name, 0, sizeof(hdr.name));
		strncpy(hdr.name, desc, sizeof(hdr.name)-1);
		success = saveSaveGameHeader(out, hdr);
	}

	// copy savegame from memory-stream to file
	if (success) {
		_savePreparedSavegame->seek(0, SEEK_SET);
		byte buffer[1024];
		while ((nread = _savePreparedSavegame->read(buffer, sizeof(buffer)))) {
			nwritten = out->write(buffer, nread);
			if (nwritten < nread) {
				success = false;
				break;
			}
		}
	}

	if (out) {
		out->finalize();
		if (out->err())
			success = false;
		delete out;
	}

	if (!success) {
		debug(1, "State save as '%s' FAILED", filename.c_str());
		return false;
	} else {
		debug(1, "State saved as '%s'", filename.c_str());
		return true;
	}
}

static bool loadSaveGameHeader(Common::SeekableReadStream *in, SaveGameHeader &hdr) {
	hdr.type = in->readUint32BE();
	hdr.size = in->readUint32LE();
	hdr.ver = in->readUint32LE();
	in->read(hdr.name, sizeof(hdr.name));
	return !in->err() && hdr.type == MKTAG('S','C','V','M');
}

bool ScummEngine::loadState(int slot, bool compat) {
	Common::String filename;
	Common::SeekableReadStream *in;
	SaveGameHeader hdr;
	int sb, sh;

	if (_saveLoadSlot == 255) {
		// Allow custom filenames for save game system in HE Games
		filename = _saveLoadFileName;
	} else {
		filename = makeSavegameName(slot, compat);
	}
	if (!(in = _saveFileMan->openForLoading(filename)))
		return false;

	if (!loadSaveGameHeader(in, hdr)) {
		warning("Invalid savegame '%s'", filename.c_str());
		delete in;
		return false;
	}

	// In older versions of ScummVM, the header version was not endian safe.
	// We account for that by retrying once with swapped byte order in case
	// we see a version that is higher than anything we'd expect...
	if (hdr.ver > 0xFFFFFF)
		hdr.ver = SWAP_BYTES_32(hdr.ver);

	// Reject save games which are too old or too new. Note that
	// We do not really support V7 games, but still accept them here
	// to work around a bug from the stone age (see below for more
	// information).
	if (hdr.ver < VER(7) || hdr.ver > CURRENT_VER) {
		warning("Invalid version of '%s'", filename.c_str());
		delete in;
		return false;
	}

	// We (deliberately) broke HE savegame compatibility at some point.
	if (hdr.ver < VER(50) && _game.heversion >= 71) {
		warning("Unsupported version of '%s'", filename.c_str());
		delete in;
		return false;
	}

	// Since version 52 a thumbnail is saved directly after the header.
	if (hdr.ver >= VER(52)) {
		// Prior to version 75 we always required an thumbnail to be present
		if (hdr.ver <= VER(74)) {
			if (!Graphics::checkThumbnailHeader(*in)) {
				warning("Can not load thumbnail");
				delete in;
				return false;
			}
		}

		Graphics::skipThumbnail(*in);
	}

	// Since version 56 we save additional information about the creation of
	// the save game and the save time.
	if (hdr.ver >= VER(56)) {
		SaveStateMetaInfos infos;
		if (!loadInfos(in, &infos)) {
			warning("Info section could not be found");
			delete in;
			return false;
		}

		setTotalPlayTime(infos.playtime * 1000);
	} else {
		// start time counting
		setTotalPlayTime();
	}

	// Due to a bug in scummvm up to and including 0.3.0, save games could be saved
	// in the V8/V9 format but were tagged with a V7 mark. Ouch. So we just pretend V7 == V8 here
	if (hdr.ver == VER(7))
		hdr.ver = VER(8);

	hdr.name[sizeof(hdr.name)-1] = 0;
	_saveLoadDescription = hdr.name;

	// Unless specifically requested with _saveSound, we do not save the iMUSE
	// state for temporary state saves - such as certain cutscenes in DOTT,
	// FOA, Sam and Max, etc.
	//
	// Thus, we should probably not stop music when restoring from one of
	// these saves. This change stops the Mole Man theme from going quiet in
	// Sam & Max when Doug tells you about the Ball of Twine, as mentioned in
	// patch #886058.
	//
	// If we don't have iMUSE at all we may as well stop the sounds. The previous
	// default behavior here was to stopAllSounds on all state restores.

	if (!_imuse || _saveSound || !_saveTemporaryState)
		_sound->stopAllSounds();

#ifdef ENABLE_SCUMM_7_8
	if (_imuseDigital) {
		_imuseDigital->stopAllSounds();
		_imuseDigital->resetState();
	}
#endif

	_sound->stopCD();

	_sound->pauseSounds(true);

	closeRoom();

	memset(_inventory, 0, sizeof(_inventory[0]) * _numInventory);
	memset(_newNames, 0, sizeof(_newNames[0]) * _numNewNames);

	// Because old savegames won't fill the entire gfxUsageBits[] array,
	// clear it here just to be sure it won't hold any unforseen garbage.
	memset(gfxUsageBits, 0, sizeof(gfxUsageBits));

	// Nuke all resources
	for (ResType type = rtFirst; type <= rtLast; type = ResType(type + 1))
		if (type != rtTemp && type != rtBuffer && (type != rtSound || _saveSound || !compat))
			for (ResId idx = 0; idx < _res->_types[type].size(); idx++) {
				_res->nukeResource(type, idx);
			}

	resetScummVars();

	if (_game.features & GF_OLD_BUNDLE)
		loadCharset(0); // FIXME - HACK ?

	//
	// Now do the actual loading
	//
	Serializer ser(in, 0, hdr.ver);
	saveOrLoad(&ser);
	delete in;

	// Update volume settings
	syncSoundSettings();

	if (_townsPlayer && (hdr.ver >= VER(81)))
		_townsPlayer->restoreAfterLoad();

	// Init NES costume data
	if (_game.platform == Common::kPlatformNES) {
		if (hdr.ver < VER(47))
			_NESCostumeSet = 0;
		NES_loadCostumeSet(_NESCostumeSet);
	}

	// Normally, _vm->_screenTop should always be >= 0, but for some old save games
	// it is not, hence we check & correct it here.
	if (_screenTop < 0)
		_screenTop = 0;

	// WORKAROUND bug #795214: For unknown reasons, object 819 sometimes is in
	// state 1 in old save games, implying it should be drawn. This in turn
	// results in a crash when entering the church, as object 819 is part of the
	// exitof the church and there are no graphics assigned to it.
	if (_game.id == GID_MONKEY_VGA) {
		putState(819, 0);
	}

	if (hdr.ver < VER(33) && _game.version >= 7) {
		// For a long time, we didn't set these vars to default values.
		VAR(VAR_DEFAULT_TALK_DELAY) = 60;
		if (_game.version == 7)
			VAR(VAR_NUM_GLOBAL_OBJS) = _numGlobalObjects - 1;
	}

	if (hdr.ver < VER(30)) {
		// For a long time, we used incorrect location, causing it to default to zero.
		if (_game.version == 8)
			_scummVars[VAR_CHARINC] = (_game.features & GF_DEMO) ? 3 : 1;
		// Needed due to subtitle speed changes
		_defaultTalkDelay /= 20;
	}

	// For a long time, we used incorrect locations for some camera related
	// scumm vars. We now know the proper locations. To be able to properly use
	// old save games, we update the old (bad) variables to the new (correct)
	// ones.
	if (hdr.ver < VER(28) && _game.version == 8) {
		_scummVars[VAR_CAMERA_MIN_X] = _scummVars[101];
		_scummVars[VAR_CAMERA_MAX_X] = _scummVars[102];
		_scummVars[VAR_CAMERA_MIN_Y] = _scummVars[103];
		_scummVars[VAR_CAMERA_MAX_Y] = _scummVars[104];
		_scummVars[VAR_CAMERA_THRESHOLD_X] = _scummVars[105];
		_scummVars[VAR_CAMERA_THRESHOLD_Y] = _scummVars[106];
		_scummVars[VAR_CAMERA_SPEED_X] = _scummVars[107];
		_scummVars[VAR_CAMERA_SPEED_Y] = _scummVars[108];
		_scummVars[VAR_CAMERA_ACCEL_X] = _scummVars[109];
		_scummVars[VAR_CAMERA_ACCEL_Y] = _scummVars[110];
	}

	// For a long time, we used incorrect values for some camera related
	// scumm vars. We now know the proper values. To be able to properly use
	// old save games, we update the old (bad) values to the new (correct)
	// ones.
	if (hdr.ver < VER(77) && _game.version >= 7) {
		_scummVars[VAR_CAMERA_THRESHOLD_X] = 100;
		_scummVars[VAR_CAMERA_THRESHOLD_Y] = 70;
		_scummVars[VAR_CAMERA_ACCEL_X] = 100;
		_scummVars[VAR_CAMERA_ACCEL_Y] = 100;
	}

	// With version 22, we replaced the scale items with scale slots. So when
	// loading such an old save game, try to upgrade the old to new format.
	if (hdr.ver < VER(22)) {
		// Convert all rtScaleTable resources to matching scale items
		for (ResId idx = 1; idx < _res->_types[rtScaleTable].size(); idx++) {
			convertScaleTableToScaleSlot(idx);
		}
	}

	// Reset the palette.
	resetPalette();

	if (hdr.ver < VER(35) && _game.id == GID_MANIAC && _game.version <= 1)
		resetV1ActorTalkColor();

	// Load the static room data
	setupRoomSubBlocks();

	if (_game.version < 7) {
		camera._last.x = camera._cur.x;
	}

	sb = _screenB;
	sh = _screenH;

	// Restore the virtual screens and force a fade to black.
	initScreens(0, _screenHeight);

	VirtScreen *vs = &_virtscr[kMainVirtScreen];
	memset(vs->getPixels(0, 0), 0, vs->pitch * vs->h);
	vs->setDirtyRange(0, vs->h);
	updateDirtyScreen(kMainVirtScreen);
	updatePalette();
	initScreens(sb, sh);

	_completeScreenRedraw = true;

	// Reset charset mask
	_charset->_hasMask = false;
	clearTextSurface();

	_lastCodePtr = NULL;
	_drawObjectQueNr = 0;
	_verbMouseOver = 0;

	cameraMoved();

	initBGBuffers(_roomHeight);

	if (VAR_ROOM_FLAG != 0xFF)
		VAR(VAR_ROOM_FLAG) = 1;

	// Sync with current config setting
	if (VAR_VOICE_MODE != 0xFF)
		VAR(VAR_VOICE_MODE) = ConfMan.getBool("subtitles");

	debug(1, "State loaded from '%s'", filename.c_str());

	_sound->pauseSounds(false);

	// WORKAROUND: Original save/load script ran this script
	// after game load, and o2_loadRoomWithEgo() does as well
	// this script starts character-dependent music
	//
	// Fixes bug #1766072: MANIACNES: Music Doesn't Start On Load Game
	if (_game.platform == Common::kPlatformNES) {
		runScript(5, 0, 0, 0);

		if (VAR(224)) {
			_sound->addSoundToQueue(VAR(224));
		}
	}

	return true;
}

Common::String ScummEngine::makeSavegameName(const Common::String &target, int slot, bool temporary) {
	Common::String extension;
	extension = Common::String::format(".%c%02d", temporary ? 'c' : 's', slot);
	return target + extension;
}

void ScummEngine::listSavegames(bool *marks, int num) {
	assert(marks);

	char slot[3];
	int slotNum;
	Common::StringArray files;

	Common::String prefix = makeSavegameName(99, false);
	prefix.setChar('*', prefix.size()-2);
	prefix.setChar(0, prefix.size()-1);
	memset(marks, false, num * sizeof(bool));	//assume no savegames for this title
	files = _saveFileMan->listSavefiles(prefix);

	for (Common::StringArray::const_iterator file = files.begin(); file != files.end(); ++file) {
		//Obtain the last 2 digits of the filename, since they correspond to the save slot
		slot[0] = file->c_str()[file->size()-2];
		slot[1] = file->c_str()[file->size()-1];
		slot[2] = 0;

		slotNum = atoi(slot);
		if (slotNum >= 0 && slotNum < num)
			marks[slotNum] = true;	//mark this slot as valid
	}
}

bool getSavegameName(Common::InSaveFile *in, Common::String &desc, int heversion);

bool ScummEngine::getSavegameName(int slot, Common::String &desc) {
	Common::InSaveFile *in = 0;
	bool result = false;

	desc.clear();
	Common::String filename = makeSavegameName(slot, false);
	in = _saveFileMan->openForLoading(filename);
	if (in) {
		result = Scumm::getSavegameName(in, desc, _game.heversion);
		delete in;
	}
	return result;
}

bool getSavegameName(Common::InSaveFile *in, Common::String &desc, int heversion) {
	SaveGameHeader hdr;

	if (!loadSaveGameHeader(in, hdr)) {
		desc = "Invalid savegame";
		return false;
	}

	if (hdr.ver > CURRENT_VER)
		hdr.ver = TO_LE_32(hdr.ver);
	if (hdr.ver < VER(7) || hdr.ver > CURRENT_VER) {
		desc = "Invalid version";
		return false;
	}

	// We (deliberately) broke HE savegame compatibility at some point.
	if (hdr.ver < VER(57) && heversion >= 60) {
		desc = "Unsupported version";
		return false;
	}

	hdr.name[sizeof(hdr.name) - 1] = 0;
	desc = hdr.name;
	return true;
}

Graphics::Surface *ScummEngine::loadThumbnailFromSlot(const char *target, int slot) {
	Common::SeekableReadStream *in;
	SaveGameHeader hdr;

	if (slot < 0)
		return 0;

	Common::String filename = ScummEngine::makeSavegameName(target, slot, false);
	if (!(in = g_system->getSavefileManager()->openForLoading(filename))) {
		return 0;
	}

	if (!loadSaveGameHeader(in, hdr)) {
		delete in;
		return 0;
	}

	if (hdr.ver > CURRENT_VER)
		hdr.ver = TO_LE_32(hdr.ver);
	if (hdr.ver < VER(52)) {
		delete in;
		return 0;
	}

	Graphics::Surface *thumb = 0;
	if (Graphics::checkThumbnailHeader(*in)) {
		thumb = Graphics::loadThumbnail(*in);
	}

	delete in;
	return thumb;
}

bool ScummEngine::loadInfosFromSlot(const char *target, int slot, SaveStateMetaInfos *stuff) {
	Common::SeekableReadStream *in;
	SaveGameHeader hdr;

	if (slot < 0)
		return 0;

	Common::String filename = makeSavegameName(target, slot, false);
	if (!(in = g_system->getSavefileManager()->openForLoading(filename))) {
		return false;
	}

	if (!loadSaveGameHeader(in, hdr)) {
		delete in;
		return false;
	}

	if (hdr.ver > CURRENT_VER)
		hdr.ver = TO_LE_32(hdr.ver);
	if (hdr.ver < VER(56)) {
		delete in;
		return false;
	}

	if (!Graphics::skipThumbnail(*in)) {
		delete in;
		return false;
	}

	if (!loadInfos(in, stuff)) {
		delete in;
		return false;
	}

	delete in;
	return true;
}

bool ScummEngine::loadInfos(Common::SeekableReadStream *file, SaveStateMetaInfos *stuff) {
	memset(stuff, 0, sizeof(SaveStateMetaInfos));

	SaveInfoSection section;
	section.type = file->readUint32BE();
	if (section.type != MKTAG('I','N','F','O')) {
		return false;
	}

	section.version = file->readUint32BE();
	section.size = file->readUint32BE();

	// If we ever extend this we should add a table containing the sizes corresponding to each
	// version, so that we are able to properly verify their correctness.
	if (section.version == INFOSECTION_VERSION && section.size != SaveInfoSectionSize) {
		warning("Info section is corrupt");
		file->skip(section.size);
		return false;
	}

	section.timeTValue = file->readUint32BE();
	section.playtime = file->readUint32BE();

	// For header version 1, we load the data in with our old method
	if (section.version == 1) {
		//time_t tmp = section.timeTValue;
		//tm *curTime = localtime(&tmp);
		//stuff->date = (curTime->tm_mday & 0xFF) << 24 | ((curTime->tm_mon + 1) & 0xFF) << 16 | (curTime->tm_year + 1900) & 0xFFFF;
		//stuff->time = (curTime->tm_hour & 0xFF) << 8 | (curTime->tm_min) & 0xFF;
		stuff->date = 0;
		stuff->time = 0;
	}

	if (section.version >= 2) {
		section.date = file->readUint32BE();
		section.time = file->readUint16BE();

		stuff->date = section.date;
		stuff->time = section.time;
	}

	stuff->playtime = section.playtime;

	// Skip over the remaining (unsupported) data
	if (section.size > SaveInfoSectionSize)
		file->skip(section.size - SaveInfoSectionSize);

	return true;
}

void ScummEngine::saveInfos(Common::WriteStream* file) {
	SaveInfoSection section;
	section.type = MKTAG('I','N','F','O');
	section.version = INFOSECTION_VERSION;
	section.size = SaveInfoSectionSize;

	// still save old format for older versions
	section.timeTValue = 0;
	section.playtime = getTotalPlayTime() / 1000;

	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	section.date = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	section.time = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);

	file->writeUint32BE(section.type);
	file->writeUint32BE(section.version);
	file->writeUint32BE(section.size);
	file->writeUint32BE(section.timeTValue);
	file->writeUint32BE(section.playtime);
	file->writeUint32BE(section.date);
	file->writeUint16BE(section.time);
}

void ScummEngine::saveOrLoad(Serializer *s) {
	const SaveLoadEntry objectEntries[] = {
		MKLINE(ObjectData, OBIMoffset, sleUint32, VER(8)),
		MKLINE(ObjectData, OBCDoffset, sleUint32, VER(8)),
		MKLINE(ObjectData, walk_x, sleUint16, VER(8)),
		MKLINE(ObjectData, walk_y, sleUint16, VER(8)),
		MKLINE(ObjectData, obj_nr, sleUint16, VER(8)),
		MKLINE(ObjectData, x_pos, sleInt16, VER(8)),
		MKLINE(ObjectData, y_pos, sleInt16, VER(8)),
		MKLINE(ObjectData, width, sleUint16, VER(8)),
		MKLINE(ObjectData, height, sleUint16, VER(8)),
		MKLINE(ObjectData, actordir, sleByte, VER(8)),
		MKLINE(ObjectData, parentstate, sleByte, VER(8)),
		MKLINE(ObjectData, parent, sleByte, VER(8)),
		MKLINE(ObjectData, state, sleByte, VER(8)),
		MKLINE(ObjectData, fl_object_index, sleByte, VER(8)),
		MKLINE(ObjectData, flags, sleByte, VER(46)),
		MKEND()
	};

	const SaveLoadEntry verbEntries[] = {
		MKLINE(VerbSlot, curRect.left, sleInt16, VER(8)),
		MKLINE(VerbSlot, curRect.top, sleInt16, VER(8)),
		MKLINE(VerbSlot, curRect.right, sleInt16, VER(8)),
		MKLINE(VerbSlot, curRect.bottom, sleInt16, VER(8)),
		MKLINE(VerbSlot, oldRect.left, sleInt16, VER(8)),
		MKLINE(VerbSlot, oldRect.top, sleInt16, VER(8)),
		MKLINE(VerbSlot, oldRect.right, sleInt16, VER(8)),
		MKLINE(VerbSlot, oldRect.bottom, sleInt16, VER(8)),

		MKLINE_OLD(VerbSlot, verbid, sleByte, VER(8), VER(11)),
		MKLINE(VerbSlot, verbid, sleInt16, VER(12)),

		MKLINE(VerbSlot, color, sleByte, VER(8)),
		MKLINE(VerbSlot, hicolor, sleByte, VER(8)),
		MKLINE(VerbSlot, dimcolor, sleByte, VER(8)),
		MKLINE(VerbSlot, bkcolor, sleByte, VER(8)),
		MKLINE(VerbSlot, type, sleByte, VER(8)),
		MKLINE(VerbSlot, charset_nr, sleByte, VER(8)),
		MKLINE(VerbSlot, curmode, sleByte, VER(8)),
		MKLINE(VerbSlot, saveid, sleByte, VER(8)),
		MKLINE(VerbSlot, key, sleByte, VER(8)),
		MKLINE(VerbSlot, center, sleByte, VER(8)),
		MKLINE(VerbSlot, prep, sleByte, VER(8)),
		MKLINE(VerbSlot, imgindex, sleUint16, VER(8)),
		MKEND()
	};

	const SaveLoadEntry mainEntries[] = {
		MKARRAY(ScummEngine, _gameMD5[0], sleUint8, 16, VER(39)),
		MK_OBSOLETE(ScummEngine, _roomWidth, sleUint16, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _roomHeight, sleUint16, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _ENCD_offs, sleUint32, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _EXCD_offs, sleUint32, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _IM00_offs, sleUint32, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _CLUT_offs, sleUint32, VER(8), VER(50)),
		MK_OBSOLETE(ScummEngine, _EPAL_offs, sleUint32, VER(8), VER(9)),
		MK_OBSOLETE(ScummEngine, _PALS_offs, sleUint32, VER(8), VER(50)),
		MKLINE(ScummEngine, _curPalIndex, sleByte, VER(8)),
		MKLINE(ScummEngine, _currentRoom, sleByte, VER(8)),
		MKLINE(ScummEngine, _roomResource, sleByte, VER(8)),
		MKLINE(ScummEngine, _numObjectsInRoom, sleByte, VER(8)),
		MKLINE(ScummEngine, _currentScript, sleByte, VER(8)),
		MK_OBSOLETE_ARRAY(ScummEngine, _localScriptOffsets[0], sleUint32, _numLocalScripts, VER(8), VER(50)),


		// vm.localvar grew from 25 to 40 script entries and then from
		// 16 to 32 bit variables (but that wasn't reflect here)... and
		// THEN from 16 to 25 variables.
		MKARRAY2_OLD(ScummEngine, vm.localvar[0][0], sleUint16, 17, 25, (byte *)vm.localvar[1] - (byte *)vm.localvar[0], VER(8), VER(8)),
		MKARRAY2_OLD(ScummEngine, vm.localvar[0][0], sleUint16, 17, 40, (byte *)vm.localvar[1] - (byte *)vm.localvar[0], VER(9), VER(14)),

		// We used to save 25 * 40 = 1000 blocks; but actually, each 'row consisted of 26 entry,
		// i.e. 26 * 40 = 1040. Thus the last 40 blocks of localvar where not saved at all. To be
		// able to load this screwed format, we use a trick: We load 26 * 38 = 988 blocks.
		// Then, we mark the followin 12 blocks (24 bytes) as obsolete.
		MKARRAY2_OLD(ScummEngine, vm.localvar[0][0], sleUint16, 26, 38, (byte *)vm.localvar[1] - (byte *)vm.localvar[0], VER(15), VER(17)),
		MK_OBSOLETE_ARRAY(ScummEngine, vm.localvar[39][0], sleUint16, 12, VER(15), VER(17)),

		// This was the first proper multi dimensional version of the localvars, with 32 bit values
		MKARRAY2_OLD(ScummEngine, vm.localvar[0][0], sleUint32, 26, 40, (byte *)vm.localvar[1] - (byte *)vm.localvar[0], VER(18), VER(19)),

		// Then we doubled the script slots again, from 40 to 80
		MKARRAY2(ScummEngine, vm.localvar[0][0], sleUint32, 26, NUM_SCRIPT_SLOT, (byte *)vm.localvar[1] - (byte *)vm.localvar[0], VER(20)),


		MKARRAY(ScummEngine, _resourceMapper[0], sleByte, 128, VER(8)),
		MKARRAY(ScummEngine, _charsetColorMap[0], sleByte, 16, VER(8)),

		// _charsetData grew from 10*16, to 15*16, to 23*16 bytes
		MKARRAY_OLD(ScummEngine, _charsetData[0][0], sleByte, 10 * 16, VER(8), VER(9)),
		MKARRAY_OLD(ScummEngine, _charsetData[0][0], sleByte, 15 * 16, VER(10), VER(66)),
		MKARRAY(ScummEngine, _charsetData[0][0], sleByte, 23 * 16, VER(67)),

		MK_OBSOLETE(ScummEngine, _curExecScript, sleUint16, VER(8), VER(62)),

		MKLINE(ScummEngine, camera._dest.x, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._dest.y, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._cur.x, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._cur.y, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._last.x, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._last.y, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._accel.x, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._accel.y, sleInt16, VER(8)),
		MKLINE(ScummEngine, _screenStartStrip, sleInt16, VER(8)),
		MKLINE(ScummEngine, _screenEndStrip, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._mode, sleByte, VER(8)),
		MKLINE(ScummEngine, camera._follows, sleByte, VER(8)),
		MKLINE(ScummEngine, camera._leftTrigger, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._rightTrigger, sleInt16, VER(8)),
		MKLINE(ScummEngine, camera._movingToActor, sleUint16, VER(8)),

		MKLINE(ScummEngine, _actorToPrintStrFor, sleByte, VER(8)),
		MKLINE(ScummEngine, _charsetColor, sleByte, VER(8)),

		// _charsetBufPos was changed from byte to int
		MKLINE_OLD(ScummEngine, _charsetBufPos, sleByte, VER(8), VER(9)),
		MKLINE(ScummEngine, _charsetBufPos, sleInt16, VER(10)),

		MKLINE(ScummEngine, _haveMsg, sleByte, VER(8)),
		MKLINE(ScummEngine, _haveActorSpeechMsg, sleByte, VER(61)),
		MKLINE(ScummEngine, _useTalkAnims, sleByte, VER(8)),

		MKLINE(ScummEngine, _talkDelay, sleInt16, VER(8)),
		MKLINE(ScummEngine, _defaultTalkDelay, sleInt16, VER(8)),
		MK_OBSOLETE(ScummEngine, _numInMsgStack, sleInt16, VER(8), VER(27)),
		MKLINE(ScummEngine, _sentenceNum, sleByte, VER(8)),

		MKLINE(ScummEngine, vm.cutSceneStackPointer, sleByte, VER(8)),
		MKARRAY(ScummEngine, vm.cutScenePtr[0], sleUint32, 5, VER(8)),
		MKARRAY(ScummEngine, vm.cutSceneScript[0], sleByte, 5, VER(8)),
		MKARRAY(ScummEngine, vm.cutSceneData[0], sleInt16, 5, VER(8)),
		MKLINE(ScummEngine, vm.cutSceneScriptIndex, sleInt16, VER(8)),

		MKLINE(ScummEngine, vm.numNestedScripts, sleByte, VER(8)),
		MKLINE(ScummEngine, _userPut, sleByte, VER(8)),
		MKLINE(ScummEngine, _userState, sleUint16, VER(17)),
		MKLINE(ScummEngine, _cursor.state, sleByte, VER(8)),
		MK_OBSOLETE(ScummEngine, _gdi->_cursorActive, sleByte, VER(8), VER(20)),
		MKLINE(ScummEngine, _currentCursor, sleByte, VER(8)),
		MKARRAY(ScummEngine, _grabbedCursor[0], sleByte, 8192, VER(20)),
		MKLINE(ScummEngine, _cursor.width, sleInt16, VER(20)),
		MKLINE(ScummEngine, _cursor.height, sleInt16, VER(20)),
		MKLINE(ScummEngine, _cursor.hotspotX, sleInt16, VER(20)),
		MKLINE(ScummEngine, _cursor.hotspotY, sleInt16, VER(20)),
		MKLINE(ScummEngine, _cursor.animate, sleByte, VER(20)),
		MKLINE(ScummEngine, _cursor.animateIndex, sleByte, VER(20)),
		MKLINE(ScummEngine, _mouse.x, sleInt16, VER(20)),
		MKLINE(ScummEngine, _mouse.y, sleInt16, VER(20)),

		MKARRAY(ScummEngine, _colorUsedByCycle[0], sleByte, 256, VER(60)),
		MKLINE(ScummEngine, _doEffect, sleByte, VER(8)),
		MKLINE(ScummEngine, _switchRoomEffect, sleByte, VER(8)),
		MKLINE(ScummEngine, _newEffect, sleByte, VER(8)),
		MKLINE(ScummEngine, _switchRoomEffect2, sleByte, VER(8)),
		MKLINE(ScummEngine, _bgNeedsRedraw, sleByte, VER(8)),

		// The state of palManipulate is stored only since V10
		MKLINE(ScummEngine, _palManipStart, sleByte, VER(10)),
		MKLINE(ScummEngine, _palManipEnd, sleByte, VER(10)),
		MKLINE(ScummEngine, _palManipCounter, sleUint16, VER(10)),

		// gfxUsageBits grew from 200 to 410 entries. Then 3 * 410 entries:
		MKARRAY_OLD(ScummEngine, gfxUsageBits[0], sleUint32, 200, VER(8), VER(9)),
		MKARRAY_OLD(ScummEngine, gfxUsageBits[0], sleUint32, 410, VER(10), VER(13)),
		MKARRAY(ScummEngine, gfxUsageBits[0], sleUint32, 3 * 410, VER(14)),

		MK_OBSOLETE(ScummEngine, _gdi->_transparentColor, sleByte, VER(8), VER(50)),
		MKARRAY(ScummEngine, _currentPalette[0], sleByte, 768, VER(8)),
		MKARRAY(ScummEngine, _darkenPalette[0], sleByte, 768, VER(53)),

		// Sam & Max specific palette replaced by _shadowPalette now.
		MK_OBSOLETE_ARRAY(ScummEngine, _proc_special_palette[0], sleByte, 256, VER(8), VER(33)),

		MKARRAY(ScummEngine, _charsetBuffer[0], sleByte, 256, VER(8)),

		MKLINE(ScummEngine, _egoPositioned, sleByte, VER(8)),

		// _gdi->_imgBufOffs grew from 4 to 5 entries. Then one day we realized
		// that we don't have to store it since initBGBuffers() recomputes it.
		MK_OBSOLETE_ARRAY(ScummEngine, _gdi->_imgBufOffs[0], sleUint16, 4, VER(8), VER(9)),
		MK_OBSOLETE_ARRAY(ScummEngine, _gdi->_imgBufOffs[0], sleUint16, 5, VER(10), VER(26)),

		// See _imgBufOffs: _numZBuffer is recomputed by initBGBuffers().
		MK_OBSOLETE(ScummEngine, _gdi->_numZBuffer, sleByte, VER(8), VER(26)),

		MKLINE(ScummEngine, _screenEffectFlag, sleByte, VER(8)),

		MK_OBSOLETE(ScummEngine, _randSeed1, sleUint32, VER(8), VER(9)),
		MK_OBSOLETE(ScummEngine, _randSeed2, sleUint32, VER(8), VER(9)),

		// Converted _shakeEnabled to boolean and added a _shakeFrame field.
		MKLINE_OLD(ScummEngine, _shakeEnabled, sleInt16, VER(8), VER(9)),
		MKLINE(ScummEngine, _shakeEnabled, sleByte, VER(10)),
		MKLINE(ScummEngine, _shakeFrame, sleUint32, VER(10)),

		MKLINE(ScummEngine, _keepText, sleByte, VER(8)),

		MKLINE(ScummEngine, _screenB, sleUint16, VER(8)),
		MKLINE(ScummEngine, _screenH, sleUint16, VER(8)),

		MKLINE(ScummEngine, _NESCostumeSet, sleUint16, VER(47)),

		MK_OBSOLETE(ScummEngine, _cd_track, sleInt16, VER(9), VER(9)),
		MK_OBSOLETE(ScummEngine, _cd_loops, sleInt16, VER(9), VER(9)),
		MK_OBSOLETE(ScummEngine, _cd_frame, sleInt16, VER(9), VER(9)),
		MK_OBSOLETE(ScummEngine, _cd_end, sleInt16, VER(9), VER(9)),

		MKEND()
	};

	const SaveLoadEntry scriptSlotEntries[] = {
		MKLINE(ScriptSlot, offs, sleUint32, VER(8)),
		MKLINE(ScriptSlot, delay, sleInt32, VER(8)),
		MKLINE(ScriptSlot, number, sleUint16, VER(8)),
		MKLINE(ScriptSlot, delayFrameCount, sleUint16, VER(8)),
		MKLINE(ScriptSlot, status, sleByte, VER(8)),
		MKLINE(ScriptSlot, where, sleByte, VER(8)),
		MKLINE(ScriptSlot, freezeResistant, sleByte, VER(8)),
		MKLINE(ScriptSlot, recursive, sleByte, VER(8)),
		MKLINE(ScriptSlot, freezeCount, sleByte, VER(8)),
		MKLINE(ScriptSlot, didexec, sleByte, VER(8)),
		MKLINE(ScriptSlot, cutsceneOverride, sleByte, VER(8)),
		MKLINE(ScriptSlot, cycle, sleByte, VER(46)),
		MK_OBSOLETE(ScriptSlot, unk5, sleByte, VER(8), VER(10)),
		MKEND()
	};

	const SaveLoadEntry nestedScriptEntries[] = {
		MKLINE(NestedScript, number, sleUint16, VER(8)),
		MKLINE(NestedScript, where, sleByte, VER(8)),
		MKLINE(NestedScript, slot, sleByte, VER(8)),
		MKEND()
	};

	const SaveLoadEntry sentenceTabEntries[] = {
		MKLINE(SentenceTab, verb, sleUint8, VER(8)),
		MKLINE(SentenceTab, preposition, sleUint8, VER(8)),
		MKLINE(SentenceTab, objectA, sleUint16, VER(8)),
		MKLINE(SentenceTab, objectB, sleUint16, VER(8)),
		MKLINE(SentenceTab, freezeCount, sleUint8, VER(8)),
		MKEND()
	};

	const SaveLoadEntry stringTabEntries[] = {
		// Then _default/restore of a StringTab entry becomes a one liner.
		MKLINE(StringTab, xpos, sleInt16, VER(8)),
		MKLINE(StringTab, _default.xpos, sleInt16, VER(8)),
		MKLINE(StringTab, ypos, sleInt16, VER(8)),
		MKLINE(StringTab, _default.ypos, sleInt16, VER(8)),
		MKLINE(StringTab, right, sleInt16, VER(8)),
		MKLINE(StringTab, _default.right, sleInt16, VER(8)),
		MKLINE(StringTab, color, sleInt8, VER(8)),
		MKLINE(StringTab, _default.color, sleInt8, VER(8)),
		MKLINE(StringTab, charset, sleInt8, VER(8)),
		MKLINE(StringTab, _default.charset, sleInt8, VER(8)),
		MKLINE(StringTab, center, sleByte, VER(8)),
		MKLINE(StringTab, _default.center, sleByte, VER(8)),
		MKLINE(StringTab, overhead, sleByte, VER(8)),
		MKLINE(StringTab, _default.overhead, sleByte, VER(8)),
		MKLINE(StringTab, no_talk_anim, sleByte, VER(8)),
		MKLINE(StringTab, _default.no_talk_anim, sleByte, VER(8)),
		MKLINE(StringTab, wrapping, sleByte, VER(71)),
		MKLINE(StringTab, _default.wrapping, sleByte, VER(71)),
		MKEND()
	};

	const SaveLoadEntry colorCycleEntries[] = {
		MKLINE(ColorCycle, delay, sleUint16, VER(8)),
		MKLINE(ColorCycle, counter, sleUint16, VER(8)),
		MKLINE(ColorCycle, flags, sleUint16, VER(8)),
		MKLINE(ColorCycle, start, sleByte, VER(8)),
		MKLINE(ColorCycle, end, sleByte, VER(8)),
		MKEND()
	};

	const SaveLoadEntry scaleSlotsEntries[] = {
		MKLINE(ScaleSlot, x1, sleUint16, VER(13)),
		MKLINE(ScaleSlot, y1, sleUint16, VER(13)),
		MKLINE(ScaleSlot, scale1, sleUint16, VER(13)),
		MKLINE(ScaleSlot, x2, sleUint16, VER(13)),
		MKLINE(ScaleSlot, y2, sleUint16, VER(13)),
		MKLINE(ScaleSlot, scale2, sleUint16, VER(13)),
		MKEND()
	};

	// MSVC6 FIX (Jamieson630):
	// MSVC6 has a problem with any notation that involves
	// more than one set of double colons ::
	// The following MKLINE macros expand to such things
	// as AudioCDManager::Status::playing, and MSVC6 has
	// a fit with that. This typedef simplifies the notation
	// to something MSVC6 can grasp.
	typedef AudioCDManager::Status AudioCDManager_Status;
	const SaveLoadEntry audioCDEntries[] = {
		MKLINE(AudioCDManager_Status, playing, sleUint32, VER(24)),
		MKLINE(AudioCDManager_Status, track, sleInt32, VER(24)),
		MKLINE(AudioCDManager_Status, start, sleUint32, VER(24)),
		MKLINE(AudioCDManager_Status, duration, sleUint32, VER(24)),
		MKLINE(AudioCDManager_Status, numLoops, sleInt32, VER(24)),
		MKEND()
	};

	int i;
	int var120Backup;
	int var98Backup;
	uint8 md5Backup[16];

	// MD5 Operations: Backup on load, compare, and reset.
	if (s->isLoading())
		memcpy(md5Backup, _gameMD5, 16);


	//
	// Save/load main state (many members of class ScummEngine get saved here)
	//
	s->saveLoadEntries(this, mainEntries);

	// MD5 Operations: Backup on load, compare, and reset.
	if (s->isLoading()) {
		char md5str1[32+1], md5str2[32+1];
		for (i = 0; i < 16; i++) {
			sprintf(md5str1 + i*2, "%02x", (int)_gameMD5[i]);
			sprintf(md5str2 + i*2, "%02x", (int)md5Backup[i]);
		}

		debug(2, "Save version: %d", s->getVersion());
		debug(2, "Saved game MD5: %s", (s->getVersion() >= 39) ? md5str1 : "unknown");

		if (memcmp(md5Backup, _gameMD5, 16) != 0) {
			warning("Game was saved with different gamedata - you may encounter problems");
			debug(1, "You have %s and save is %s.", md5str2, md5str1);
			memcpy(_gameMD5, md5Backup, 16);
		}
	}


	// Starting V14, we extended the usage bits, to be able to cope with games
	// that have more than 30 actors (up to 94 are supported now, in theory).
	// Since the format of the usage bits was changed by this, we have to
	// convert them when loading an older savegame.
	if (s->isLoading() && s->getVersion() < VER(14))
		upgradeGfxUsageBits();

	// When loading, move the mouse to the saved mouse position.
	if (s->isLoading() && s->getVersion() >= VER(20)) {
		updateCursor();
		_system->warpMouse(_mouse.x, _mouse.y);
	}

	// Before V61, we re-used the _haveMsg flag to handle "alternative" speech
	// sound files (see charset code 10).
	if (s->isLoading() && s->getVersion() < VER(61)) {
		if (_haveMsg == 0xFE) {
			_haveActorSpeechMsg = false;
			_haveMsg = 0xFF;
		} else {
			_haveActorSpeechMsg = true;
		}
	}

	//
	// Save/load actors
	//
	for (i = 0; i < _numActors; i++)
		_actors[i]->saveLoadWithSerializer(s);


	//
	// Save/load sound data
	//
	_sound->saveLoadWithSerializer(s);


	//
	// Save/load script data
	//
	if (s->getVersion() < VER(9))
		s->saveLoadArrayOf(vm.slot, 25, sizeof(vm.slot[0]), scriptSlotEntries);
	else if (s->getVersion() < VER(20))
		s->saveLoadArrayOf(vm.slot, 40, sizeof(vm.slot[0]), scriptSlotEntries);
	else
		s->saveLoadArrayOf(vm.slot, NUM_SCRIPT_SLOT, sizeof(vm.slot[0]), scriptSlotEntries);

	if (s->getVersion() < VER(46)) {
		// When loading an old savegame, make sure that the 'cycle'
		// field is set to something sensible, otherwise the scripts
		// that were running probably won't be.

		for (i = 0; i < NUM_SCRIPT_SLOT; i++) {
			vm.slot[i].cycle = 1;
		}
	}


	//
	// Save/load local objects
	//
	s->saveLoadArrayOf(_objs, _numLocalObjects, sizeof(_objs[0]), objectEntries);
	if (s->isLoading()) {
		if (s->getVersion() < VER(13)) {
			// Since roughly v13 of the save games, the objs storage has changed a bit
			for (i = _numObjectsInRoom; i < _numLocalObjects; i++)
				_objs[i].obj_nr = 0;
		} else if (_game.version == 0 && s->getVersion() < VER(91)) {
			for (i = 0; i < _numLocalObjects; i++) {
				// Merge object id and type (previously stored in flags)
				if (_objs[i].obj_nr != 0 && OBJECT_V0_TYPE(_objs[i].obj_nr) == 0 && _objs[i].flags != 0)
					_objs[i].obj_nr = OBJECT_V0(_objs[i].obj_nr, _objs[i].flags);
				_objs[i].flags = 0;
			}
		}
	}


	//
	// Save/load misc stuff
	//
	s->saveLoadArrayOf(_verbs, _numVerbs, sizeof(_verbs[0]), verbEntries);
	s->saveLoadArrayOf(vm.nest, 16, sizeof(vm.nest[0]), nestedScriptEntries);
	s->saveLoadArrayOf(_sentence, 6, sizeof(_sentence[0]), sentenceTabEntries);
	s->saveLoadArrayOf(_string, 6, sizeof(_string[0]), stringTabEntries);
	s->saveLoadArrayOf(_colorCycle, 16, sizeof(_colorCycle[0]), colorCycleEntries);
	if (s->getVersion() >= VER(13))
		s->saveLoadArrayOf(_scaleSlots, 20, sizeof(_scaleSlots[0]), scaleSlotsEntries);


	//
	// Save/load resources
	//
	ResType type;
	ResId idx;
	if (s->getVersion() >= VER(26)) {
		// New, more robust resource save/load system. This stores the type
		// and index of each resource. Thus if we increase e.g. the maximum
		// number of script resources, savegames won't break.
		if (s->isSaving()) {
			for (type = rtFirst; type <= rtLast; type = ResType(type + 1)) {
				if (_res->_types[type]._mode != kStaticResTypeMode && type != rtTemp && type != rtBuffer) {
					s->saveUint16(type);	// Save the res type...
					for (idx = 0; idx < _res->_types[type].size(); idx++) {
						// Only save resources which actually exist...
						if (_res->_types[type][idx]._address) {
							s->saveUint16(idx);	// Save the index of the resource
							saveResource(s, type, idx);
						}
					}
					s->saveUint16(0xFFFF);	// End marker
				}
			}
			s->saveUint16(0xFFFF);	// End marker
		} else {
			while ((type = (ResType)s->loadUint16()) != 0xFFFF) {
				while ((idx = s->loadUint16()) != 0xFFFF) {
					assert(idx < _res->_types[type].size());
					loadResource(s, type, idx);
				}
			}
		}
	} else {
		// Old, fragile resource save/load system. Doesn't save resources
		// with index 0, and breaks whenever we change the limit on a given
		// resource type.
		for (type = rtFirst; type <= rtLast; type = ResType(type + 1))
			if (_res->_types[type]._mode != kStaticResTypeMode && type != rtTemp && type != rtBuffer) {
				// For V1-V5 games, there used to be no object name resources.
				// At some point this changed. But since old savegames rely on
				// unchanged resource counts, we have to hard code the following check
				if (_game.version < 6 && type == rtObjectName)
					continue;
				for (idx = 1; idx < _res->_types[type].size(); idx++)
					loadResourceOLD(s, type, idx);
			}
	}


	//
	// Save/load global object state
	//
	s->saveLoadArrayOf(_objectOwnerTable, _numGlobalObjects, sizeof(_objectOwnerTable[0]), sleByte);
	s->saveLoadArrayOf(_objectStateTable, _numGlobalObjects, sizeof(_objectStateTable[0]), sleByte);
	if (_objectRoomTable)
		s->saveLoadArrayOf(_objectRoomTable, _numGlobalObjects, sizeof(_objectRoomTable[0]), sleByte);


	//
	// Save/load palette data
	// Don't save 16 bit palette in FM-Towns and PCE games, since it gets regenerated afterwards anyway.
	if (_16BitPalette && !(_game.platform == Common::kPlatformFMTowns && s->getVersion() < VER(82)) && !((_game.platform == Common::kPlatformFMTowns || _game.platform == Common::kPlatformPCEngine) && s->getVersion() > VER(87))) {
		s->saveLoadArrayOf(_16BitPalette, 512, sizeof(_16BitPalette[0]), sleUint16);
	}


	// FM-Towns specific (extra palette data, color cycle data, etc.)
	// In earlier save game versions (below 87) the FM-Towns specific data would get saved (and loaded) even in non FM-Towns games.
	// This would cause an unnecessary save file incompatibility between DS (which uses the DISABLE_TOWNS_DUAL_LAYER_MODE setting)
	// and other ports.
	// In version 88 and later the save files from FM-Towns targets are compatible between DS and other platforms, too.

#ifdef DISABLE_TOWNS_DUAL_LAYER_MODE
	byte hasTownsData = 0;
	if (_game.platform == Common::kPlatformFMTowns && s->getVersion() > VER(87))
		s->saveLoadArrayOf(&hasTownsData, 1, sizeof(byte), sleByte);

	if (hasTownsData) {
		// Skip FM-Towns specific data
		for (int i = 69 * sizeof(uint8) + 44 * sizeof(int16); i; i--)
			s->loadByte();
	}

#else
	byte hasTownsData = ((_game.platform == Common::kPlatformFMTowns && s->getVersion() >= VER(87)) || (s->getVersion() >= VER(82) && s->getVersion() < VER(87))) ? 1 : 0;
	if (_game.platform == Common::kPlatformFMTowns && s->getVersion() > VER(87))
		s->saveLoadArrayOf(&hasTownsData, 1, sizeof(byte), sleByte);

	if (hasTownsData) {
		const SaveLoadEntry townsFields[] = {
			MKLINE(Common::Rect, left, sleInt16, VER(82)),
			MKLINE(Common::Rect, top, sleInt16, VER(82)),
			MKLINE(Common::Rect, right, sleInt16, VER(82)),
			MKLINE(Common::Rect, bottom, sleInt16, VER(82)),
			MKEND()
		};

		const SaveLoadEntry townsExtraEntries[] = {
			MKLINE(ScummEngine, _townsOverrideShadowColor, sleUint8, VER(82)),
			MKLINE(ScummEngine, _numCyclRects, sleUint8, VER(82)),
			MKLINE(ScummEngine, _townsPaletteFlags, sleUint8, VER(82)),
			MKLINE(ScummEngine, _townsClearLayerFlag, sleUint8, VER(82)),
			MKLINE(ScummEngine, _townsActiveLayerFlags, sleUint8, VER(82)),
			MKEND()
		};

		s->saveLoadArrayOf(_textPalette, 48, sizeof(_textPalette[0]), sleUint8);
		s->saveLoadArrayOf(_cyclRects, 10, sizeof(_cyclRects[0]), townsFields);
		s->saveLoadArrayOf(&_curStringRect, 1, sizeof(_curStringRect), townsFields);
		s->saveLoadArrayOf(_townsCharsetColorMap, 16, sizeof(_townsCharsetColorMap[0]), sleUint8);
		s->saveLoadEntries(this, townsExtraEntries);
	} else if (_game.platform == Common::kPlatformFMTowns && s->getVersion() >= VER(82)) {
		warning("Save file is missing FM-Towns specific graphic data (game was apparently saved on another platform)");
	}
#endif

	if (_shadowPaletteSize) {
		s->saveLoadArrayOf(_shadowPalette, _shadowPaletteSize, 1, sleByte);
		// _roomPalette didn't show up until V21 save games
		// Note that we also save the room palette for Indy4 Amiga, since it
		// is used as palette map there too, but we do so slightly a bit
		// further down to group it with the other special palettes needed.
		if (s->getVersion() >= VER(21) && _game.version < 5)
			s->saveLoadArrayOf(_roomPalette, sizeof(_roomPalette), 1, sleByte);
	}

	// PalManip data was not saved before V10 save games
	if (s->getVersion() < VER(10))
		_palManipCounter = 0;
	if (_palManipCounter) {
		if (!_palManipPalette)
			_palManipPalette = (byte *)calloc(0x300, 1);
		if (!_palManipIntermediatePal)
			_palManipIntermediatePal = (byte *)calloc(0x600, 1);
		s->saveLoadArrayOf(_palManipPalette, 0x300, 1, sleByte);
		s->saveLoadArrayOf(_palManipIntermediatePal, 0x600, 1, sleByte);
	}

	// darkenPalette was not saved before V53
	if (s->isLoading() && s->getVersion() < VER(53)) {
		memcpy(_darkenPalette, _currentPalette, 768);
	}

	// _colorUsedByCycle was not saved before V60
	if (s->isLoading() && s->getVersion() < VER(60)) {
		memset(_colorUsedByCycle, 0, sizeof(_colorUsedByCycle));
	}

	// Indy4 Amiga specific palette tables were not saved before V85
	if (_game.platform == Common::kPlatformAmiga && _game.id == GID_INDY4) {
		if (s->getVersion() >= 85) {
			s->saveLoadArrayOf(_roomPalette, 256, 1, sleByte);
			s->saveLoadArrayOf(_verbPalette, 256, 1, sleByte);
			s->saveLoadArrayOf(_amigaPalette, 3 * 64, 1, sleByte);

			// Starting from version 86 we also save the first used color in
			// the palette beyond the verb palette. For old versions we just
			// look for it again, which hopefully won't cause any troubles.
			if (s->getVersion() >= 86) {
				s->saveLoadArrayOf(&_amigaFirstUsedColor, 1, 2, sleUint16);
			} else {
				amigaPaletteFindFirstUsedColor();
			}
		} else {
			warning("Save with old Indiana Jones 4 Amiga palette handling detected");
			// We need to restore the internal state of the Amiga palette for Indy4
			// Amiga. This might lead to graphics glitches!
			setAmigaPaletteFromPtr(_currentPalette);
		}
	}

	//
	// Save/load more global object state
	//
	s->saveLoadArrayOf(_classData, _numGlobalObjects, sizeof(_classData[0]), sleUint32);


	//
	// Save/load script variables
	//
	var120Backup = _scummVars[120];
	var98Backup = _scummVars[98];

	if (s->getVersion() > VER(37))
		s->saveLoadArrayOf(_roomVars, _numRoomVariables, sizeof(_roomVars[0]), sleInt32);

	// The variables grew from 16 to 32 bit.
	if (s->getVersion() < VER(15))
		s->saveLoadArrayOf(_scummVars, _numVariables, sizeof(_scummVars[0]), sleInt16);
	else
		s->saveLoadArrayOf(_scummVars, _numVariables, sizeof(_scummVars[0]), sleInt32);

	if (_game.id == GID_TENTACLE)	// Maybe misplaced, but that's the main idea
		_scummVars[120] = var120Backup;
	if (_game.id == GID_INDY4)
		_scummVars[98] = var98Backup;

	s->saveLoadArrayOf(_bitVars, _numBitVariables >> 3, 1, sleByte);


	//
	// Save/load a list of the locked objects
	//
	if (s->isSaving()) {
		for (type = rtFirst; type <= rtLast; type = ResType(type + 1))
			for (idx = 1; idx < _res->_types[type].size(); idx++) {
				if (_res->isLocked(type, idx)) {
					s->saveByte(type);
					s->saveUint16(idx);
				}
			}
		s->saveByte(0xFF);
	} else {
		while ((type = (ResType)s->loadByte()) != 0xFF) {
			idx = s->loadUint16();
			_res->lock(type, idx);
		}
	}


	//
	// Save/load the Audio CD status
	//
	if (s->getVersion() >= VER(24)) {
		AudioCDManager::Status info;
		if (s->isSaving())
			info = _system->getAudioCDManager()->getStatus();
		s->saveLoadArrayOf(&info, 1, sizeof(info), audioCDEntries);
		// If we are loading, and the music being loaded was supposed to loop
		// forever, then resume playing it. This helps a lot when the audio CD
		// is used to provide ambient music (see bug #788195).
		if (s->isLoading() && info.playing && info.numLoops < 0)
			_system->getAudioCDManager()->play(info.track, info.numLoops, info.start, info.duration);
	}


	//
	// Save/load the iMuse status
	//
	if (_imuse && (_saveSound || !_saveTemporaryState)) {
		_imuse->save_or_load(s, this);
	}


	// Save/load FM-Towns audio status
	if (_townsPlayer)
		_townsPlayer->saveLoadWithSerializer(s);

	//
	// Save/load the charset renderer state
	//
	if (s->getVersion() >= VER(73)) {
		_charset->saveLoadWithSerializer(s);
	} else if (s->isLoading()) {
		if (s->getVersion() == VER(72)) {
			_charset->setCurID(s->loadByte());
		} else {
			// Before V72, the charset id wasn't saved. This used to cause issues such
			// as the one described in the bug report #1722153. For these savegames,
			// we reinitialize the id using a, hopefully, sane value.
			_charset->setCurID(_string[0]._default.charset);
		}
	}
}

void ScummEngine_v0::saveOrLoad(Serializer *s) {
	ScummEngine_v2::saveOrLoad(s);

	const SaveLoadEntry v0Entrys[] = {
		MKLINE(ScummEngine_v0, _currentMode, sleByte, VER(78)),
		MKLINE(ScummEngine_v0, _currentLights, sleByte, VER(78)),
		MKLINE(ScummEngine_v0, _activeVerb, sleByte, VER(92)),
		MKLINE(ScummEngine_v0, _activeObject, sleUint16, VER(92)),
		MKLINE(ScummEngine_v0, _activeObject2, sleUint16, VER(92)),
		MKLINE(ScummEngine_v0, _cmdVerb, sleByte, VER(92)),
		MKLINE(ScummEngine_v0, _cmdObject, sleUint16, VER(92)),
		MKLINE(ScummEngine_v0, _cmdObject2, sleUint16, VER(92)),
		MKLINE(ScummEngine_v0, _walkToObject, sleUint16, VER(92)),
		MKLINE(ScummEngine_v0, _walkToObjectState, sleByte, VER(92)),
		MKEND()
	};
 	s->saveLoadEntries(this, v0Entrys);
}


void ScummEngine_v2::saveOrLoad(Serializer *s) {
	ScummEngine::saveOrLoad(s);

	const SaveLoadEntry v2Entrys[] = {
		MKLINE(ScummEngine_v2, _inventoryOffset, sleUint16, VER(79)),
		MKEND()
	};
	s->saveLoadEntries(this, v2Entrys);

	// In old saves we didn't store _inventoryOffset -> reset it to
	// a sane default when loading one of those.
	if (s->getVersion() < 79 && s->isLoading()) {
		_inventoryOffset = 0;
	}
}

void ScummEngine_v5::saveOrLoad(Serializer *s) {
	ScummEngine::saveOrLoad(s);

	const SaveLoadEntry cursorEntries[] = {
		MKARRAY2(ScummEngine_v5, _cursorImages[0][0], sleUint16, 16, 4, (byte *)_cursorImages[1] - (byte *)_cursorImages[0], VER(44)),
		MKARRAY(ScummEngine_v5, _cursorHotspots[0], sleByte, 8, VER(44)),
		MKEND()
	};

	// This is probably only needed for Loom.
	s->saveLoadEntries(this, cursorEntries);

	// Reset cursors for old FM-Towns savegames saved with 256 color setting.
	// Otherwise the cursor will be messed up when displayed in the new hi color setting.
	if (_game.platform == Common::kPlatformFMTowns && _outputPixelFormat.bytesPerPixel == 2 && s->isLoading() && s->getVersion() < VER(82)) {
		if (_game.id == GID_LOOM) {
			redefineBuiltinCursorFromChar(1, 1);
			redefineBuiltinCursorHotspot(1, 0, 0);
		} else {
			resetCursors();
		}
	}

	// Regenerate 16bit palette after loading.
	// This avoids color issues when loading savegames that have been saved with a different ScummVM port
	// that uses a different 16bit color mode than the ScummVM port which is currently used.
#ifdef USE_RGB_COLOR
	if (_game.platform == Common::kPlatformPCEngine && s->isLoading()) {
		for (int i = 0; i < 256; ++i)
			_16BitPalette[i] = get16BitColor(_currentPalette[i * 3 + 0], _currentPalette[i * 3 + 1], _currentPalette[i * 3 + 2]);
	}
#endif
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::saveOrLoad(Serializer *s) {
	ScummEngine::saveOrLoad(s);

	const SaveLoadEntry subtitleQueueEntries[] = {
		MKARRAY(SubtitleText, text[0], sleByte, 256, VER(61)),
		MKLINE(SubtitleText, charset, sleByte, VER(61)),
		MKLINE(SubtitleText, color, sleByte, VER(61)),
		MKLINE(SubtitleText, xpos, sleInt16, VER(61)),
		MKLINE(SubtitleText, ypos, sleInt16, VER(61)),
		MKLINE(SubtitleText, actorSpeechMsg, sleByte, VER(61)),
		MKEND()
	};

	const SaveLoadEntry V7Entries[] = {
		MKLINE(ScummEngine_v7, _subtitleQueuePos, sleInt32, VER(61)),
		MK_OBSOLETE(ScummEngine_v7, _verbCharset, sleInt32, VER(68), VER(68)),
		MKLINE(ScummEngine_v7, _verbLineSpacing, sleInt32, VER(68)),
		MKEND()
	};

	_imuseDigital->saveOrLoad(s);

	s->saveLoadArrayOf(_subtitleQueue, ARRAYSIZE(_subtitleQueue), sizeof(_subtitleQueue[0]), subtitleQueueEntries);
	s->saveLoadEntries(this, V7Entries);

	if (s->getVersion() <= VER(68) && s->isLoading()) {
		// WORKAROUND bug #1846049: Reset the default charset color to a sane value.
		_string[0]._default.charset = 1;
	}
}
#endif

void ScummEngine_v60he::saveOrLoad(Serializer *s) {
	ScummEngine::saveOrLoad(s);

	s->saveLoadArrayOf(_arraySlot, _numArray, sizeof(_arraySlot[0]), sleByte);
}

void ScummEngine_v70he::saveOrLoad(Serializer *s) {
	ScummEngine_v60he::saveOrLoad(s);

	const SaveLoadEntry HE70Entries[] = {
		MKLINE(ScummEngine_v70he, _heSndSoundId, sleInt32, VER(51)),
		MKLINE(ScummEngine_v70he, _heSndOffset, sleInt32, VER(51)),
		MKLINE(ScummEngine_v70he, _heSndChannel, sleInt32, VER(51)),
		MKLINE(ScummEngine_v70he, _heSndFlags, sleInt32, VER(51)),
		MKEND()
	};

	s->saveLoadEntries(this, HE70Entries);
}

#ifdef ENABLE_HE
void ScummEngine_v71he::saveOrLoad(Serializer *s) {
	ScummEngine_v70he::saveOrLoad(s);

	const SaveLoadEntry polygonEntries[] = {
		MKLINE(WizPolygon, vert[0].x, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[0].y, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[1].x, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[1].y, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[2].x, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[2].y, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[3].x, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[3].y, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[4].x, sleInt16, VER(40)),
		MKLINE(WizPolygon, vert[4].y, sleInt16, VER(40)),
		MKLINE(WizPolygon, bound.left, sleInt16, VER(40)),
		MKLINE(WizPolygon, bound.top, sleInt16, VER(40)),
		MKLINE(WizPolygon, bound.right, sleInt16, VER(40)),
		MKLINE(WizPolygon, bound.bottom, sleInt16, VER(40)),
		MKLINE(WizPolygon, id, sleInt16, VER(40)),
		MKLINE(WizPolygon, numVerts, sleInt16, VER(40)),
		MKLINE(WizPolygon, flag, sleByte, VER(40)),
		MKEND()
	};

	s->saveLoadArrayOf(_wiz->_polygons, ARRAYSIZE(_wiz->_polygons), sizeof(_wiz->_polygons[0]), polygonEntries);
}

void ScummEngine_v90he::saveOrLoad(Serializer *s) {
	ScummEngine_v71he::saveOrLoad(s);

	const SaveLoadEntry floodFillEntries[] = {
		MKLINE(FloodFillParameters, box.left, sleInt32, VER(51)),
		MKLINE(FloodFillParameters, box.top, sleInt32, VER(51)),
		MKLINE(FloodFillParameters, box.right, sleInt32, VER(51)),
		MKLINE(FloodFillParameters, box.bottom, sleInt32, VER(51)),
		MKLINE(FloodFillParameters, x, sleInt32, VER(51)),
		MKLINE(FloodFillParameters, y, sleInt32, VER(51)),
		MKLINE(FloodFillParameters, flags, sleInt32, VER(51)),
		MK_OBSOLETE(FloodFillParameters, unk1C, sleInt32, VER(51), VER(62)),
		MKEND()
	};

	const SaveLoadEntry HE90Entries[] = {
		MKLINE(ScummEngine_v90he, _curMaxSpriteId, sleInt32, VER(51)),
		MKLINE(ScummEngine_v90he, _curSpriteId, sleInt32, VER(51)),
		MKLINE(ScummEngine_v90he, _curSpriteGroupId, sleInt32, VER(51)),
		MK_OBSOLETE(ScummEngine_v90he, _numSpritesToProcess, sleInt32, VER(51), VER(63)),
		MKLINE(ScummEngine_v90he, _heObject, sleInt32, VER(51)),
		MKLINE(ScummEngine_v90he, _heObjectNum, sleInt32, VER(51)),
		MKLINE(ScummEngine_v90he, _hePaletteNum, sleInt32, VER(51)),
		MKEND()
	};

	_sprite->saveOrLoadSpriteData(s);

	s->saveLoadArrayOf(&_floodFillParams, 1, sizeof(_floodFillParams), floodFillEntries);

	s->saveLoadEntries(this, HE90Entries);
}

void ScummEngine_v99he::saveOrLoad(Serializer *s) {
	ScummEngine_v90he::saveOrLoad(s);

	s->saveLoadArrayOf(_hePalettes, (_numPalettes + 1) * _hePaletteSlot, sizeof(_hePalettes[0]), sleUint8);
}

void ScummEngine_v100he::saveOrLoad(Serializer *s) {
	ScummEngine_v99he::saveOrLoad(s);

	const SaveLoadEntry HE100Entries[] = {
		MKLINE(ScummEngine_v100he, _heResId, sleInt32, VER(51)),
		MKLINE(ScummEngine_v100he, _heResType, sleInt32, VER(51)),
		MKEND()
	};

	s->saveLoadEntries(this, HE100Entries);
}
#endif

void ScummEngine::loadResourceOLD(Serializer *ser, ResType type, ResId idx) {
	uint32 size;

	if (type == rtSound && ser->getVersion() >= VER(23)) {
		// Save/load only a list of resource numbers that need to be reloaded.
		if (ser->loadUint16())
			ensureResourceLoaded(rtSound, idx);
	} else if (_res->_types[type]._mode == kDynamicResTypeMode) {
		size = ser->loadUint32();
		if (size) {
			_res->createResource(type, idx, size);
			ser->loadBytes(getResourceAddress(type, idx), size);
			if (type == rtInventory) {
				_inventory[idx] = ser->loadUint16();
			}
			if (type == rtObjectName && ser->getVersion() >= VER(25)) {
				// Paranoia: We increased the possible number of new names
				// to fix bugs #933610 and #936323. The savegame format
				// didn't change, but at least during the transition
				// period there is a slight chance that we try to load
				// more names than we have allocated space for. If so,
				// discard them.
				if (idx < _numNewNames)
					_newNames[idx] = ser->loadUint16();
			}
		}
	}
}

void ScummEngine::saveResource(Serializer *ser, ResType type, ResId idx) {
	assert(_res->_types[type][idx]._address);

	if (_res->_types[type]._mode == kDynamicResTypeMode) {
		byte *ptr = _res->_types[type][idx]._address;
		uint32 size = _res->_types[type][idx]._size;

		ser->saveUint32(size);
		ser->saveBytes(ptr, size);

		if (type == rtInventory) {
			ser->saveUint16(_inventory[idx]);
		}
		if (type == rtObjectName) {
			ser->saveUint16(_newNames[idx]);
		}
	}
}

void ScummEngine::loadResource(Serializer *ser, ResType type, ResId idx) {
	if (_game.heversion >= 60 && ser->getVersion() <= VER(65) &&
		((type == rtSound && idx == 1) || (type == rtSpoolBuffer))) {
		uint32 size = ser->loadUint32();
		assert(size);
		_res->createResource(type, idx, size);
		ser->loadBytes(getResourceAddress(type, idx), size);
	} else if (type == rtSound) {
		// HE Games use sound resource 1 for speech
		if (_game.heversion >= 60 && idx == 1)
			return;

		ensureResourceLoaded(rtSound, idx);
	} else if (_res->_types[type]._mode == kDynamicResTypeMode) {
		uint32 size = ser->loadUint32();
		assert(size);
		byte *ptr = _res->createResource(type, idx, size);
		ser->loadBytes(ptr, size);

		if (type == rtInventory) {
			_inventory[idx] = ser->loadUint16();
		}
		if (type == rtObjectName) {
			_newNames[idx] = ser->loadUint16();
		}
	}
}

void Serializer::saveBytes(void *b, int len) {
	_saveStream->write(b, len);
}

void Serializer::loadBytes(void *b, int len) {
	_loadStream->read(b, len);
}

void Serializer::saveUint32(uint32 d) {
	_saveStream->writeUint32LE(d);
}

void Serializer::saveUint16(uint16 d) {
	_saveStream->writeUint16LE(d);
}

void Serializer::saveByte(byte b) {
	_saveStream->writeByte(b);
}

uint32 Serializer::loadUint32() {
	return _loadStream->readUint32LE();
}

uint16 Serializer::loadUint16() {
	return _loadStream->readUint16LE();
}

byte Serializer::loadByte() {
	return _loadStream->readByte();
}

void Serializer::saveArrayOf(void *b, int len, int datasize, byte filetype) {
	byte *at = (byte *)b;
	uint32 data;

	// speed up byte arrays
	if (datasize == 1 && filetype == sleByte) {
		if (len > 0) {
			saveBytes(b, len);
		}
		return;
	}

	while (--len >= 0) {
		if (datasize == 0) {
			// Do nothing for obsolete data
			data = 0;
		} else if (datasize == 1) {
			data = *(byte *)at;
			at += 1;
		} else if (datasize == 2) {
			data = *(uint16 *)at;
			at += 2;
		} else if (datasize == 4) {
			data = *(uint32 *)at;
			at += 4;
		} else {
			error("saveArrayOf: invalid size %d", datasize);
		}
		switch (filetype) {
		case sleByte:
			saveByte((byte)data);
			break;
		case sleUint16:
		case sleInt16:
			saveUint16((int16)data);
			break;
		case sleInt32:
		case sleUint32:
			saveUint32(data);
			break;
		default:
			error("saveArrayOf: invalid filetype %d", filetype);
		}
	}
}

void Serializer::loadArrayOf(void *b, int len, int datasize, byte filetype) {
	byte *at = (byte *)b;
	uint32 data;

	// speed up byte arrays
	if (datasize == 1 && filetype == sleByte) {
		loadBytes(b, len);
		return;
	}

	while (--len >= 0) {
		switch (filetype) {
		case sleByte:
			data = loadByte();
			break;
		case sleUint16:
			data = loadUint16();
			break;
		case sleInt16:
			data = (int16)loadUint16();
			break;
		case sleUint32:
			data = loadUint32();
			break;
		case sleInt32:
			data = (int32)loadUint32();
			break;
		default:
			error("loadArrayOf: invalid filetype %d", filetype);
		}
		if (datasize == 0) {
			// Do nothing for obsolete data
		} else if (datasize == 1) {
			*(byte *)at = (byte)data;
			at += 1;
		} else if (datasize == 2) {
			*(uint16 *)at = (uint16)data;
			at += 2;
		} else if (datasize == 4) {
			*(uint32 *)at = data;
			at += 4;
		} else {
			error("loadArrayOf: invalid size %d", datasize);
		}
	}
}

void Serializer::saveLoadArrayOf(void *b, int num, int datasize, const SaveLoadEntry *sle) {
	byte *data = (byte *)b;

	if (isSaving()) {
		while (--num >= 0) {
			saveEntries(data, sle);
			data += datasize;
		}
	} else {
		while (--num >= 0) {
			loadEntries(data, sle);
			data += datasize;
		}
	}
}

void Serializer::saveLoadArrayOf(void *b, int len, int datasize, byte filetype) {
	if (isSaving())
		saveArrayOf(b, len, datasize, filetype);
	else
		loadArrayOf(b, len, datasize, filetype);
}

void Serializer::saveLoadEntries(void *d, const SaveLoadEntry *sle) {
	if (isSaving())
		saveEntries(d, sle);
	else
		loadEntries(d, sle);
}

void Serializer::saveEntries(void *d, const SaveLoadEntry *sle) {
	byte type;
	byte *at;
	int size;

	while (sle->offs != 0xFFFF) {
		at = (byte *)d + sle->offs;
		size = sle->size;
		type = (byte) sle->type;

		if (sle->maxVersion != CURRENT_VER) {
			// Skip obsolete entries
			if (type & 128)
				sle++;
		} else {
			// save entry
			int columns = 1;
			int rows = 1;
			int rowlen = 0;
			if (type & 128) {
				sle++;
				columns = sle->offs;
				rows = sle->type;
				rowlen = sle->size;
				type &= ~128;
			}
			while (rows--) {
				saveArrayOf(at, columns, size, type);
				at += rowlen;
			}
		}
		sle++;
	}
}

void Serializer::loadEntries(void *d, const SaveLoadEntry *sle) {
	byte type;
	byte *at;
	int size;

	while (sle->offs != 0xFFFF) {
		at = (byte *)d + sle->offs;
		size = sle->size;
		type = (byte) sle->type;

		if (_savegameVersion < sle->minVersion || _savegameVersion > sle->maxVersion) {
			// Skip entries which are not present in this save game version
			if (type & 128)
				sle++;
		} else {
			// load entry
			int columns = 1;
			int rows = 1;
			int rowlen = 0;

			if (type & 128) {
				sle++;
				columns = sle->offs;
				rows = sle->type;
				rowlen = sle->size;
				type &= ~128;
			}
			while (rows--) {
				loadArrayOf(at, columns, size, type);
				at += rowlen;
			}
		}
		sle++;
	}
}

} // End of namespace Scumm
