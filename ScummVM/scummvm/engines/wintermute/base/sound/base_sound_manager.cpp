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

#include "engines/wintermute/base/sound/base_sound_manager.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/sound/base_sound_buffer.h"
#include "engines/wintermute/wintermute.h"
#include "common/config-manager.h"
#include "audio/mixer.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//IMPLEMENT_PERSISTENT(BaseSoundMgr, true);

//////////////////////////////////////////////////////////////////////////
BaseSoundMgr::BaseSoundMgr(BaseGame *inGame) : BaseClass(inGame) {
	_soundAvailable = false;
	_volumeMaster = 255;
}


//////////////////////////////////////////////////////////////////////////
BaseSoundMgr::~BaseSoundMgr() {
	saveSettings();
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::cleanup() {
	for (uint32 i = 0; i < _sounds.size(); i++) {
		delete _sounds[i];
	}
	_sounds.clear();
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void BaseSoundMgr::saveSettings() {
	if (_soundAvailable) {
		ConfMan.setInt("master_volume", _volumeMaster);
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::initialize() {
	_soundAvailable = false;

	if (!g_system->getMixer()->isReady()) {
		return STATUS_FAILED;
	}
	_volumeMaster = (ConfMan.hasKey("master_volume") ? ConfMan.getInt("master_volume") : 255);
	_soundAvailable = true;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
BaseSoundBuffer *BaseSoundMgr::addSound(const Common::String &filename, Audio::Mixer::SoundType type, bool streamed) {
	if (!_soundAvailable) {
		return NULL;
	}

	BaseSoundBuffer *sound;

	Common::String useFilename = filename;
	// try to switch WAV to OGG file (if available)
	AnsiString ext = PathUtil::getExtension(filename);
	if (StringUtil::compareNoCase(ext, "wav")) {
		AnsiString path = PathUtil::getDirectoryName(filename);
		AnsiString name = PathUtil::getFileNameWithoutExtension(filename);

		AnsiString newFile = PathUtil::combine(path, name + "ogg");
		if (BaseFileManager::getEngineInstance()->hasFile(newFile)) {
			useFilename = newFile;
		}
	}

	sound = new BaseSoundBuffer(_gameRef);
	if (!sound) {
		return NULL;
	}

	sound->setStreaming(streamed);
	sound->setType(type);


	bool res = sound->loadFromFile(useFilename);
	if (DID_FAIL(res)) {
		_gameRef->LOG(res, "Error loading sound '%s'", useFilename.c_str());
		delete sound;
		return NULL;
	}

	// Make sure the master-volume is applied to the sound.
	sound->updateVolume();

	// register sound
	_sounds.push_back(sound);

	return sound;

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::addSound(BaseSoundBuffer *sound, Audio::Mixer::SoundType type) {
	if (!sound) {
		return STATUS_FAILED;
	}

	// Make sure the master-volume is applied to the sound.
	sound->updateVolume();

	// register sound
	_sounds.push_back(sound);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::removeSound(BaseSoundBuffer *sound) {
	for (uint32 i = 0; i < _sounds.size(); i++) {
		if (_sounds[i] == sound) {
			delete _sounds[i];
			_sounds.remove_at(i);
			return STATUS_OK;
		}
	}

	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::setVolume(Audio::Mixer::SoundType type, int volume) {
	if (!_soundAvailable) {
		return STATUS_OK;
	}

	switch (type) {
	case Audio::Mixer::kSFXSoundType:
		ConfMan.setInt("sfx_volume", volume);
		break;
	case Audio::Mixer::kSpeechSoundType:
		ConfMan.setInt("speech_volume", volume);
		break;
	case Audio::Mixer::kMusicSoundType:
		ConfMan.setInt("music_volume", volume);
		break;
	case Audio::Mixer::kPlainSoundType:
		error("Plain sound type shouldn't be used in WME");
	}
	g_engine->syncSoundSettings();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::setVolumePercent(Audio::Mixer::SoundType type, byte percent) {
	return setVolume(type, percent * 255 / 100);
}


//////////////////////////////////////////////////////////////////////////
byte BaseSoundMgr::getVolumePercent(Audio::Mixer::SoundType type) {
	int volume = 0;

	switch (type) {
	case Audio::Mixer::kSFXSoundType:
	case Audio::Mixer::kSpeechSoundType:
	case Audio::Mixer::kMusicSoundType:
		volume = g_system->getMixer()->getVolumeForSoundType(type);
		break;
	default:
		error("Sound-type not set");
		break;
	}

	return (byte)(volume * 100 / 255);
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::setMasterVolume(byte value) {
	_volumeMaster = value;
	for (uint32 i = 0; i < _sounds.size(); i++) {
		_sounds[i]->updateVolume();
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::setMasterVolumePercent(byte percent) {
	setMasterVolume(percent * 255 / 100);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
byte BaseSoundMgr::getMasterVolumePercent() {
	return getMasterVolume() * 100 / 255;
}

//////////////////////////////////////////////////////////////////////////
byte BaseSoundMgr::getMasterVolume() {
	return (byte)_volumeMaster;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::pauseAll(bool includingMusic) {

	for (uint32 i = 0; i < _sounds.size(); i++) {
		if (_sounds[i]->isPlaying() && (_sounds[i]->_type != Audio::Mixer::kMusicSoundType || includingMusic)) {
			_sounds[i]->pause();
			_sounds[i]->_freezePaused = true;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSoundMgr::resumeAll() {

	for (uint32 i = 0; i < _sounds.size(); i++) {
		if (_sounds[i]->_freezePaused) {
			_sounds[i]->resume();
			_sounds[i]->_freezePaused = false;
		}
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
float BaseSoundMgr::posToPan(int x, int y) {
	float relPos = (float)x / ((float)_gameRef->_renderer->_width);

	float minPan = -0.7f;
	float maxPan = 0.7f;

	return minPan + relPos * (maxPan - minPan);
}

} // end of namespace Wintermute
