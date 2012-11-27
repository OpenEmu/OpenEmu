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
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/sword25.h"
#include "sword25/sfx/soundengine.h"
#include "sword25/package/packagemanager.h"
#include "sword25/kernel/resource.h"
#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"

#include "audio/decoders/vorbis.h"

#include "common/system.h"
#include "common/config-manager.h"

namespace Sword25 {

class SoundResource : public Resource {
public:
	SoundResource(const Common::String &fileName) : Resource(fileName, Resource::TYPE_SOUND), _fname(fileName) {}
	virtual ~SoundResource() {
		debugC(1, kDebugSound, "SoundResource: Unloading file %s", _fname.c_str());
	}

private:
	Common::String _fname;
};


SoundEngine::SoundEngine(Kernel *pKernel) : ResourceService(pKernel) {
	if (!registerScriptBindings())
		error("Script bindings could not be registered.");
	else
		debugC(kDebugSound, "Script bindings registered.");

	_mixer = g_system->getMixer();

	_maxHandleId = 1;

	for (int i = 0; i < SOUND_HANDLES; i++)
		_handles[i].type = kFreeHandle;
}

bool SoundEngine::init(uint sampleRate, uint channels) {
	return true;
}

void SoundEngine::update() {
}

void SoundEngine::setVolume(float volume, SOUND_TYPES type) {
	int val = (int)(255 * volume);

	switch (type) {
	case SoundEngine::MUSIC:
		ConfMan.setInt("music_volume", val);
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, val);
		break;
	case SoundEngine::SPEECH:
		ConfMan.setInt("speech_volume", val);
		_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, val);
		break;
	case SoundEngine::SFX:
		ConfMan.setInt("sfx_volume", val);
		_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, val);
		break;
	default:
		error("Unknown SOUND_TYPE");
	}
}

float SoundEngine::getVolume(SOUND_TYPES type) {
	int val = 0;

	switch (type) {
	case SoundEngine::MUSIC:
		val = ConfMan.getInt("music_volume");
		break;
	case SoundEngine::SPEECH:
		val = ConfMan.getInt("speech_volume");
		break;
	case SoundEngine::SFX:
		val = ConfMan.getInt("sfx_volume");
		break;
	default:
		error("Unknown SOUND_TYPE");
	}

	return (float)val / 255.0;
}

void SoundEngine::pauseAll() {
	debugC(1, kDebugSound, "SoundEngine::pauseAll()");

	_mixer->pauseAll(true);
}

void SoundEngine::resumeAll() {
	debugC(1, kDebugSound, "SoundEngine::resumeAll()");

	_mixer->pauseAll(false);
}

void SoundEngine::pauseLayer(uint layer) {
	// Not used in the game

	warning("SoundEngine::pauseLayer(%d)", layer);
}

void SoundEngine::resumeLayer(uint layer) {
	// Not used in the game

	warning("SoundEngine::resumeLayer(%d)", layer);
}

SndHandle *SoundEngine::getHandle(uint *id) {

	for (uint i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type != kFreeHandle && !_mixer->isSoundHandleActive(_handles[i].handle)) {
			debugC(1, kDebugSound, "Handle %d has finished playing", _handles[i].id);
			_handles[i].type = kFreeHandle;
		}
	}

	for (uint i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type == kFreeHandle) {
			debugC(1, kDebugSound, "Allocated handle %d", _handles[i].id);
			_handles[i].id = _maxHandleId;
			_handles[i].type = kAllocatedHandle;

			if (id)
				*id = _maxHandleId;

			_maxHandleId++;

			return &_handles[i];
		}
	}

	error("Sound::getHandle(): Too many sound handles");

	return NULL;
}

SndHandle *SoundEngine::findHandle(uint id) {
	for (uint i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].id == id)
			return &_handles[i];
	}

	warning("Sound::findHandle(): Unknown handle");

	return NULL;
}

Audio::Mixer::SoundType getType(SoundEngine::SOUND_TYPES type) {
	switch (type) {
	case SoundEngine::MUSIC:
		return Audio::Mixer::kMusicSoundType;
	case SoundEngine::SPEECH:
		return Audio::Mixer::kSpeechSoundType;
	case SoundEngine::SFX:
		return Audio::Mixer::kSFXSoundType;
	default:
		error("Unknown SOUND_TYPE");
	}

	return Audio::Mixer::kPlainSoundType;
}

bool SoundEngine::playSound(const Common::String &fileName, SOUND_TYPES type, float volume, float pan, bool loop, int loopStart, int loopEnd, uint layer) {
	debugC(1, kDebugSound, "SoundEngine::playSound(%s, %d, %f, %f, %d, %d, %d, %d)", fileName.c_str(), type, volume, pan, loop, loopStart, loopEnd, layer);

	playSoundEx(fileName, type, volume, pan, loop, loopStart, loopEnd, layer);

	return true;
}

uint SoundEngine::playSoundEx(const Common::String &fileName, SOUND_TYPES type, float volume, float pan, bool loop, int loopStart, int loopEnd, uint layer, uint handleId) {
	Common::SeekableReadStream *in = Kernel::getInstance()->getPackage()->getStream(fileName);
#ifdef USE_VORBIS
	Audio::SeekableAudioStream *stream = Audio::makeVorbisStream(in, DisposeAfterUse::YES);
#endif
	uint id = handleId;
	SndHandle *handle;

	if (handleId == 0x1337)
		handle = getHandle(&id);
	else
		handle = &_handles[handleId];

	handle->fileName = fileName;
	handle->sndType = type;
	handle->volume = volume;
	handle->pan = pan;
	handle->loop = loop;
	handle->loopStart = loopStart;
	handle->loopEnd = loopEnd;
	handle->layer = layer;

	debugC(1, kDebugSound, "SoundEngine::playSoundEx(%s, %d, %f, %f, %d, %d, %d, %d)", fileName.c_str(), type, volume, pan, loop, loopStart, loopEnd, layer);

#ifdef USE_VORBIS
	_mixer->playStream(getType(type), &(handle->handle), stream, -1, (byte)(volume * 255), (int8)(pan * 127));
#endif

	return id;
}

void SoundEngine::setSoundVolume(uint handle, float volume) {
	debugC(1, kDebugSound, "SoundEngine::setSoundVolume(%d, %f)", handle, volume);

	SndHandle* sndHandle = findHandle(handle);
	if (sndHandle != NULL) {
		sndHandle->volume = volume;
		_mixer->setChannelVolume(sndHandle->handle, (byte)(volume * 255));
	}
}

void SoundEngine::setSoundPanning(uint handle, float pan) {
	debugC(1, kDebugSound, "SoundEngine::setSoundPanning(%d, %f)", handle, pan);

	SndHandle* sndHandle = findHandle(handle);
	if (sndHandle != NULL) {
		sndHandle->pan = pan;
		_mixer->setChannelBalance(sndHandle->handle, (int8)(pan * 127));
	}
}

void SoundEngine::pauseSound(uint handle) {
	debugC(1, kDebugSound, "SoundEngine::pauseSound(%d)", handle);

	SndHandle* sndHandle = findHandle(handle);
	if (sndHandle != NULL)
		_mixer->pauseHandle(sndHandle->handle, true);
}

void SoundEngine::resumeSound(uint handle) {
	debugC(1, kDebugSound, "SoundEngine::resumeSound(%d)", handle);

	SndHandle* sndHandle = findHandle(handle);
	if (sndHandle != NULL)
		_mixer->pauseHandle(sndHandle->handle, false);
}

void SoundEngine::stopSound(uint handle) {
	debugC(1, kDebugSound, "SoundEngine::stopSound(%d)", handle);

	SndHandle* sndHandle = findHandle(handle);
	if (sndHandle != NULL)
		_mixer->stopHandle(sndHandle->handle);
}

bool SoundEngine::isSoundPaused(uint handle) {
	// Not used in the game

	warning("SoundEngine::isSoundPaused(%d)", handle);

	return false;
}

bool SoundEngine::isSoundPlaying(uint handle) {
	debugC(1, kDebugSound, "SoundEngine::isSoundPlaying(%d)", handle);

	SndHandle* sndHandle = findHandle(handle);
	if (sndHandle == NULL)
		return false;
	return _mixer->isSoundHandleActive(sndHandle->handle);
}

float SoundEngine::getSoundVolume(uint handle) {
	debugC(1, kDebugSound, "SoundEngine::getSoundVolume(%d)", handle);

	SndHandle* sndHandle = findHandle(handle);
	if (sndHandle == NULL)
		return 0.f;
	return (float)_mixer->getChannelVolume(sndHandle->handle) / 255.0;
}

float SoundEngine::getSoundPanning(uint handle) {
	debugC(1, kDebugSound, "SoundEngine::getSoundPanning(%d)", handle);

	SndHandle* sndHandle = findHandle(handle);
	if (sndHandle == NULL)
		return 0.f;
	return (float)_mixer->getChannelBalance(sndHandle->handle) / 127.0;
}

Resource *SoundEngine::loadResource(const Common::String &fileName) {
	return new SoundResource(fileName);
}

bool SoundEngine::canLoadResource(const Common::String &fileName) {
	Common::String fname = fileName;

	debugC(1, kDebugSound, "SoundEngine::canLoadResource(%s)", fileName.c_str());

	fname.toLowercase();

	return fname.hasSuffix(".ogg");
}

bool SoundEngine::persist(OutputPersistenceBlock &writer) {
	writer.write(_maxHandleId);

	for (uint i = 0; i < SOUND_HANDLES; i++) {
		writer.write(_handles[i].id);

		// Don't restart sounds which already finished playing.
		if (_handles[i].type != kFreeHandle && !_mixer->isSoundHandleActive(_handles[i].handle))
			_handles[i].type = kFreeHandle;

		writer.writeString(_handles[i].fileName);
		writer.write((int)_handles[i].sndType);
		writer.write(_handles[i].volume);
		writer.write(_handles[i].pan);
		writer.write(_handles[i].loop);
		writer.write(_handles[i].loopStart);
		writer.write(_handles[i].loopEnd);
		writer.write(_handles[i].layer);
	}

	return true;
}

bool SoundEngine::unpersist(InputPersistenceBlock &reader) {
	_mixer->stopAll();

	if (reader.getVersion() < 2)
		return true;

	reader.read(_maxHandleId);

	for (uint i = 0; i < SOUND_HANDLES; i++) {
		reader.read(_handles[i].id);

		Common::String fileName;
		int sndType;
		float volume;
		float pan;
		bool loop;
		int loopStart;
		int loopEnd;
		uint layer;

		reader.readString(fileName);
		reader.read(sndType);
		reader.read(volume);
		reader.read(pan);
		reader.read(loop);
		reader.read(loopStart);
		reader.read(loopEnd);
		reader.read(layer);

		if (reader.isGood()) {
			if (sndType != kFreeHandle)
				playSoundEx(fileName, (SOUND_TYPES)sndType, volume, pan, loop, loopStart, loopEnd, layer, i);
		} else
			return false;
	}

	return reader.isGood();
}


} // End of namespace Sword25
