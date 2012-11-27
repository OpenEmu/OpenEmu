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

#include "common/savefile.h"
#include "common/system.h"
#include "graphics/thumbnail.h"

#include "saga/saga.h"
#include "saga/actor.h"
#include "saga/events.h"
#include "saga/interface.h"
#include "saga/isomap.h"
#include "saga/music.h"
#include "saga/render.h"
#include "saga/scene.h"
#include "saga/script.h"

#define CURRENT_SAGA_VER 8

namespace Saga {

static SaveFileData emptySlot = {
	 "", 0
};

char* SagaEngine::calcSaveFileName(uint slotNumber) {
	static char name[MAX_FILE_NAME];
	sprintf(name, "%s.s%02u", _targetName.c_str(), slotNumber);
	return name;
}

SaveFileData *SagaEngine::getSaveFile(uint idx) {
	if (idx >= MAX_SAVES) {
		error("getSaveFileName wrong idx");
	}
	if (isSaveListFull()) {
		return &_saveFiles[_saveFilesCount - idx - 1];
	} else {
		if (!emptySlot.name[0])
			strcpy(emptySlot.name, getTextString(kTextNewSave));

		return (idx == 0) ? &emptySlot : &_saveFiles[_saveFilesCount - idx];
	}
}

bool SagaEngine::locateSaveFile(char *saveName, uint &titleNumber) {
	uint i;
	for (i = 0; i < _saveFilesCount; i++) {
		if (strcmp(saveName, _saveFiles[i].name) == 0) {
			if (isSaveListFull()) {
				titleNumber = _saveFilesCount - i - 1;
			} else {
				titleNumber = _saveFilesCount - i;
			}
			return true;
		}
	}
	return false;
}

uint SagaEngine::getNewSaveSlotNumber() const {
	uint i, j;
	bool found;
	for (i = 0; i < MAX_SAVES; i++) {
		found = false;
		for (j = 0; j < _saveFilesCount; j++) {
			if (_saveFiles[j].slotNumber == i) {
				found = true;
				break;
			}
		}
		if (!found) {
			return i;
		}
	}

	error("getNewSaveSlotNumber save list is full");
}

static int compareSaveFileData(const void *a, const void *b) {
	const SaveFileData *s1 = (const SaveFileData *)a;
	const SaveFileData *s2 = (const SaveFileData *)b;

	if (s1->slotNumber < s2->slotNumber) {
		return -1;
	} else if (s1->slotNumber > s2->slotNumber) {
		return 1;
	} else {
		return 0;
	}
}

void SagaEngine::fillSaveList() {

	int i;
	Common::InSaveFile *in;
	Common::StringArray filenames;
	char slot[3];
	int slotNumber;
	char *name;

	name = calcSaveFileName(MAX_SAVES);
	name[strlen(name) - 2] = '*';
	name[strlen(name) - 1] = 0;

	filenames = _saveFileMan->listSavefiles(name);

	for (i = 0; i < MAX_SAVES; i++) {
		_saveFiles[i].name[0] = 0;
		_saveFiles[i].slotNumber = (uint)-1;
	}

	_saveFilesCount = 0;

	for (Common::StringArray::iterator file = filenames.begin(); file != filenames.end(); ++file){
		//Obtain the last 2 digits of the filename, since they correspond to the save slot
		slot[0] = file->c_str()[file->size()-2];
		slot[1] = file->c_str()[file->size()-1];
		slot[2] = 0;

		slotNumber = atoi(slot);
		if (slotNumber >= 0 && slotNumber < MAX_SAVES) {
			name = calcSaveFileName(slotNumber);
			if ((in = _saveFileMan->openForLoading(name)) != NULL) {
				_saveHeader.type = in->readUint32BE();
				_saveHeader.size = in->readUint32LE();
				_saveHeader.version = in->readUint32LE();
				in->read(_saveHeader.name, sizeof(_saveHeader.name));

				if (_saveHeader.type != MKTAG('S','A','G','A')) {
					warning("SagaEngine::load wrong save %s format", name);
					i++;
					continue;
				}
				strcpy(_saveFiles[_saveFilesCount].name, _saveHeader.name);
				_saveFiles[_saveFilesCount].slotNumber = slotNumber;
				delete in;
				_saveFilesCount++;
			}
		}
	}

	qsort(_saveFiles, _saveFilesCount, sizeof(_saveFiles[0]), compareSaveFileData);
}

void SagaEngine::save(const char *fileName, const char *saveName) {
	Common::OutSaveFile *out;
	char title[TITLESIZE];

	if (!(out = _saveFileMan->openForSaving(fileName))) {
		return;
	}

	_saveHeader.type = MKTAG('S','A','G','A');
	_saveHeader.size = 0;
	_saveHeader.version = CURRENT_SAGA_VER;
	// Note that IHNM has a smaller save title size than ITE
	// We allocate the ITE save title size here, to preserve
	// savegame backwards compatibility
	strncpy(_saveHeader.name, saveName, SAVE_TITLE_SIZE);

	out->writeUint32BE(_saveHeader.type);
	out->writeUint32LE(_saveHeader.size);
	out->writeUint32LE(_saveHeader.version);
	out->write(_saveHeader.name, sizeof(_saveHeader.name));

	// Original game title
	memset(title, 0, TITLESIZE);
	strncpy(title, _gameTitle.c_str(), TITLESIZE);
	out->write(title, TITLESIZE);

	// Thumbnail
	// First draw scene without save dialog
	int oldMode = _interface->getMode();
	_interface->setMode(kPanelMain);
	_render->drawScene();

	Graphics::saveThumbnail(*out);

	_interface->setMode(oldMode);

	// Date / time
	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;

	out->writeUint32BE(saveDate);
	out->writeUint16BE(saveTime);
	out->writeUint32BE(playTime);

	// Surrounding scene
	out->writeSint32LE(_scene->getOutsetSceneNumber());
#ifdef ENABLE_IHNM
	if (getGameId() == GID_IHNM) {
		out->writeSint32LE(_scene->currentChapterNumber());
		// Protagonist
		out->writeSint32LE(_scene->currentProtag());
		out->writeSint32LE(_scene->getCurrentMusicTrack());
		out->writeSint32LE(_scene->getCurrentMusicRepeat());
	}
#endif
	// Inset scene
	out->writeSint32LE(_scene->currentSceneNumber());

#ifdef ENABLE_IHNM
	if (getGameId() == GID_IHNM) {
		out->writeUint32LE(_globalFlags);
		for (int i = 0; i < ARRAYSIZE(_ethicsPoints); i++)
			out->writeSint16LE(_ethicsPoints[i]);
	}
#endif

	_interface->saveState(out);

	_actor->saveState(out);

	out->writeSint16LE(_script->_commonBuffer.size());

	out->write(_script->_commonBuffer.getBuffer(), _script->_commonBuffer.size());

	// ISO map x, y coordinates for ITE
	if (getGameId() == GID_ITE) {
		out->writeSint16LE(_isoMap->getMapPosition().x);
		out->writeSint16LE(_isoMap->getMapPosition().y);
	}

	out->finalize();

	if (out->err())
		warning("Can't write file '%s'. (Disk full?)", fileName);

	delete out;

	_interface->resetSaveReminder();
}

void SagaEngine::load(const char *fileName) {
	Common::InSaveFile *in;
	int commonBufferSize;
	int sceneNumber, insetSceneNumber;
	int mapx, mapy;
	char title[TITLESIZE];

	if (!(in = _saveFileMan->openForLoading(fileName))) {
		return;
	}

	_saveHeader.type = in->readUint32BE();
	_saveHeader.size = in->readUint32LE();
	_saveHeader.version = in->readUint32LE();
	in->read(_saveHeader.name, sizeof(_saveHeader.name));

	// Some older saves were not written in an endian safe fashion.
	// We try to detect this here by checking for extremly high version values.
	// If found, we retry with the data swapped.
	if (_saveHeader.version > 0xFFFFFF) {
		warning("This savegame is not endian safe, retrying with the data swapped");
		_saveHeader.version = SWAP_BYTES_32(_saveHeader.version);
	}

	debug(2, "Save version: 0x%X", _saveHeader.version);

	if (_saveHeader.version < 4)
		warning("This savegame is not endian-safe. There may be problems");

	if (_saveHeader.type != MKTAG('S','A','G','A')) {
		error("SagaEngine::load wrong save game format");
	}

	if (_saveHeader.version > 4) {
		in->read(title, TITLESIZE);
		debug(0, "Save is for: %s", title);
	}

	if (_saveHeader.version >= 6) {
		// We don't need the thumbnail here, so just read it and discard it
		Graphics::skipThumbnail(*in);

		in->readUint32BE();	// save date
		in->readUint16BE(); // save time

		if (_saveHeader.version >= 8) {
			uint32 playTime = in->readUint32BE();
			g_engine->setTotalPlayTime(playTime * 1000);
		}
	}

	// Clear pending events here, and don't process queued music events
	_events->clearList(false);

	// Surrounding scene
	sceneNumber = in->readSint32LE();
#ifdef ENABLE_IHNM
	if (getGameId() == GID_IHNM) {
		int currentChapter = _scene->currentChapterNumber();
		_scene->setChapterNumber(in->readSint32LE());
		_scene->setProtag(in->readSint32LE());
		if (_scene->currentChapterNumber() != currentChapter)
			_scene->changeScene(-2, 0, kTransitionFade, _scene->currentChapterNumber());
		_scene->setCurrentMusicTrack(in->readSint32LE());
		_scene->setCurrentMusicRepeat(in->readSint32LE());
		_music->stop();
		if (_scene->currentChapterNumber() == 8)
			_interface->setMode(kPanelChapterSelection);
		if (!isIHNMDemo()) {
			_music->play(_music->_songTable[_scene->getCurrentMusicTrack()], _scene->getCurrentMusicRepeat() ? MUSIC_LOOP : MUSIC_NORMAL);
		} else {
			_music->play(3, MUSIC_LOOP);
		}
	}
#endif

	// Inset scene
	insetSceneNumber = in->readSint32LE();

#ifdef ENABLE_IHNM
	if (getGameId() == GID_IHNM) {
		_globalFlags = in->readUint32LE();
		for (int i = 0; i < ARRAYSIZE(_ethicsPoints); i++)
			_ethicsPoints[i] = in->readSint16LE();
	}
#endif

	_interface->loadState(in);

	_actor->loadState(in);

	commonBufferSize = in->readSint16LE();
	_script->_commonBuffer.resize(commonBufferSize);
	in->read(_script->_commonBuffer.getBuffer(), commonBufferSize);

	if (getGameId() == GID_ITE) {
		mapx = in->readSint16LE();
		mapy = in->readSint16LE();
		_isoMap->setMapPosition(mapx, mapy);
	}
	// Note: the mapx, mapy ISO map positions were incorrectly saved
	// for IHNM too, which has no ISO map scenes, up to save version 6.
	// Since they're at the end of the savegame, we just ignore them

	delete in;

	// Mute volume to prevent outScene music play
	int volume = _music->getVolume();
	_music->setVolume(0);

#ifdef ENABLE_IHNM
	// Protagonist swapping
	if (getGameId() == GID_IHNM) {
		if (_scene->currentProtag() != 0 && _scene->currentChapterNumber() != 6) {
			ActorData *actor1 = _actor->getFirstActor();
			ActorData *actor2;
			// The original gets actor2 from the current protagonist ID, but this is sometimes wrong
			// If the current protagonist ID is not correct, use the stored protagonist
			if (!_actor->validActorId(_scene->currentProtag())) {
				actor2 = _actor->_protagonist;
			} else {
				actor2 = _actor->getActor(_scene->currentProtag());
			}

			SWAP(actor1->_location, actor2->_location);

			actor2->_flags &= ~kProtagonist;
			actor1->_flags |= kProtagonist;
			_actor->_protagonist = _actor->_centerActor = actor1;
			_scene->setProtag(actor1->_id);
		}
	}
#endif

	_scene->clearSceneQueue();
	_scene->changeScene(sceneNumber, ACTOR_NO_ENTRANCE, kTransitionNoFade);

	_events->handleEvents(0); //dissolve backgrounds

	if (insetSceneNumber != sceneNumber) {
		_render->setFlag(RF_DISABLE_ACTORS);
		_scene->draw();
		_render->drawScene();
		_render->clearFlag(RF_DISABLE_ACTORS);
		_scene->changeScene(insetSceneNumber, ACTOR_NO_ENTRANCE, kTransitionNoFade);
	}

	_music->setVolume(volume);

	_interface->draw();
}

} // End of namespace Saga
