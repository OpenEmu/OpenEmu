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

#include "common/debug.h"

#include "toon/audio.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "audio/decoders/adpcm_intern.h"

namespace Toon {

AudioManager::AudioManager(ToonEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer) {
	for (int32 i = 0; i < 16; i++)
		_channels[i] = NULL;

	for (int32 i = 0; i < 4; i++)
		_audioPacks[i] = NULL;

	for (int32 i = 0; i < 4; i++) {
		_ambientSFXs[i]._delay = 0;
		_ambientSFXs[i]._enabled = false;
		_ambientSFXs[i]._id = -1;
		_ambientSFXs[i]._channel = -1;
		_ambientSFXs[i]._lastTimer = 0;
		_ambientSFXs[i]._volume = 255;
	}

	_voiceMuted = false;
	_musicMuted = false;
	_sfxMuted = false;

	_currentMusicChannel = 0;
}

AudioManager::~AudioManager(void) {
	_mixer->stopAll();
	for (int32 i = 0; i < 4; i++) {
		closeAudioPack(i);
	}
}

void AudioManager::muteMusic(bool muted) {
	setMusicVolume(muted ? 0 : 255);
	_musicMuted = muted;
}

void AudioManager::muteVoice(bool muted) {
	if (voiceStillPlaying() && _channels[2]) {
		_channels[2]->setVolume(muted ? 0 : 255);
	}
	_voiceMuted = muted;
}

void AudioManager::muteSfx(bool muted) {
	_sfxMuted = muted;
}

void AudioManager::removeInstance(AudioStreamInstance *inst) {
	debugC(1, kDebugAudio, "removeInstance(inst)");

	for (int32 i = 0; i < 16; i++) {
		if (inst == _channels[i])
			_channels[i] = NULL;
	}
}

void AudioManager::playMusic(const Common::String &dir, const Common::String &music) {
	debugC(1, kDebugAudio, "playMusic(%s, %s)", dir.c_str(), music.c_str());

	// two musics can be played at same time
	Common::String path = Common::String::format("ACT%d/%s/%s.MUS", _vm->state()->_currentChapter, dir.c_str(), music.c_str());

	if (_currentMusicName == music)
		return;

	_currentMusicName = music;

	Common::SeekableReadStream *srs = _vm->resources()->openFile(path);
	if (!srs)
		return;

	// see what channel to take
	// if the current channel didn't really start. reuse this one
	if (_channels[_currentMusicChannel] && _channels[_currentMusicChannel]->isPlaying()) {
		if (_channels[_currentMusicChannel]->getPlayedSampleCount() < 500) {
			_channels[_currentMusicChannel]->stop(false);
			_currentMusicChannel = 1 - _currentMusicChannel;
		}
		else
		{
			_channels[_currentMusicChannel]->stop(true);
		}
	}
	// go to the next channel
	_currentMusicChannel = 1 - _currentMusicChannel;

	// if it's already playing.. stop it quickly (no fade)
	if (_channels[_currentMusicChannel] && _channels[_currentMusicChannel]->isPlaying()) {
		_channels[_currentMusicChannel]->stop(false);
	}

	// no need to delete instance here it will automatically deleted by the mixer is done with it
	_channels[_currentMusicChannel] = new AudioStreamInstance(this, _mixer, srs, true, true);
	_channels[_currentMusicChannel]->setVolume(_musicMuted ? 0 : 255);
	_channels[_currentMusicChannel]->play(true, Audio::Mixer::kMusicSoundType);
}

bool AudioManager::voiceStillPlaying() {
	if (!_channels[2])
		return false;

	return _channels[2]->isPlaying();
}

void AudioManager::playVoice(int32 id, bool genericVoice) {
	debugC(1, kDebugAudio, "playVoice(%d, %d)", id, (genericVoice) ? 1 : 0);

	if (voiceStillPlaying()) {
		// stop current voice
		_channels[2]->stop(false);
	}

	Common::SeekableReadStream *stream;
	if (genericVoice)
		stream = _audioPacks[0]->getStream(id);
	else
		stream = _audioPacks[1]->getStream(id);

	// no need to delete channel 2, it will be deleted by the mixer when the stream is finished
	_channels[2] = new AudioStreamInstance(this, _mixer, stream, false, true);
	_channels[2]->play(false, Audio::Mixer::kSpeechSoundType);
	_channels[2]->setVolume(_voiceMuted ? 0 : 255);

}

int32 AudioManager::playSFX(int32 id, int volume , bool genericSFX) {
	debugC(4, kDebugAudio, "playSFX(%d, %d)", id, (genericSFX) ? 1 : 0);

	// find a free SFX channel
	Common::SeekableReadStream *stream;

	if (genericSFX)
		stream = _audioPacks[2]->getStream(id, true);
	else
		stream = _audioPacks[3]->getStream(id, true);

	if (stream->size() == 0)
		return -1;

	for (int32 i = 3; i < 16; i++) {
		if (!_channels[i]) {
			_channels[i] = new AudioStreamInstance(this, _mixer, stream, false, true);
			_channels[i]->play(false, Audio::Mixer::kSFXSoundType);
			_channels[i]->setVolume(_sfxMuted ? 0 : volume);
			return i;
		}
	}

	return -1;
}

void AudioManager::stopAllSfxs() {
	for (int32 i = 3; i < 16; i++) {
		if (_channels[i] && _channels[i]->isPlaying()) {
			_channels[i]->stop(false);
		}
	}
}

void AudioManager::stopCurrentVoice() {
	debugC(1, kDebugAudio, "stopCurrentVoice()");

	if (_channels[2] && _channels[2]->isPlaying())
		_channels[2]->stop(false);
}

void AudioManager::closeAudioPack(int32 id) {
	delete _audioPacks[id];
	_audioPacks[id] = NULL;
}

bool AudioManager::loadAudioPack(int32 id, const Common::String &indexFile, const Common::String &packFile) {
	debugC(4, kDebugAudio, "loadAudioPack(%d, %s, %s)", id, indexFile.c_str(), packFile.c_str());

	closeAudioPack(id);
	_audioPacks[id] = new AudioStreamPackage(_vm);
	return _audioPacks[id]->loadAudioPackage(indexFile, packFile);
}

void AudioManager::setMusicVolume(int32 volume) {
	debugC(1, kDebugAudio, "setMusicVolume(%d)", volume);
	if (_channels[0])
		_channels[0]->setVolume(volume);

	if (_channels[1])
		_channels[1]->setVolume(volume);
}

void AudioManager::stopMusic() {
	debugC(1, kDebugAudio, "stopMusic()");

	if (_channels[0])
		_channels[0]->stop(true);
	if (_channels[1])
		_channels[1]->stop(true);
}

AudioStreamInstance::AudioStreamInstance(AudioManager *man, Audio::Mixer *mixer, Common::SeekableReadStream *stream , bool looping, bool deleteFileStreamAtEnd) {
	_compBufferSize = 0;
	_buffer = NULL;
	_bufferMaxSize = 0;
	_mixer = mixer;
	_compBuffer = NULL;
	_bufferOffset = 0;
	_lastSample = 0;
	_lastStepIndex = 0;
	_file = stream;
	_fadingIn = false;
	_fadingOut = false;
	_fadeTime = 0;
	_stopped = false;
	_volume = 255;
	_totalSize = stream->size();
	_currentReadSize = 8;
	_man = man;
	_looping = looping;
	_musicAttenuation = 1000;
	_deleteFileStream = deleteFileStreamAtEnd;
	_playedSamples = 0;

	// preload one packet
	if (_totalSize > 0) {
		_file->skip(8);
		readPacket();
	} else {
		stopNow();
	}
}

AudioStreamInstance::~AudioStreamInstance() {
	delete[] _buffer;
	delete[] _compBuffer;

	if (_man)
		_man->removeInstance(this);

	if (_deleteFileStream) {
		delete _file;
	}
}

int AudioStreamInstance::readBuffer(int16 *buffer, const int numSamples) {
	debugC(5, kDebugAudio, "readBuffer(buffer, %d)", numSamples);

	if (_stopped)
		return 0;

	handleFade(numSamples);
	int32 leftSamples = numSamples;
	int32 destOffset = 0;
	if ((_bufferOffset + leftSamples) * 2 >= _bufferSize) {
		if (_bufferSize - _bufferOffset * 2 > 0) {
			memcpy(buffer, &_buffer[_bufferOffset], _bufferSize - _bufferOffset * 2);
			leftSamples -= (_bufferSize - _bufferOffset * 2) / 2;
			destOffset += (_bufferSize - _bufferOffset * 2) / 2;
		}
		if (!readPacket())
			return 0;

		_bufferOffset = 0;
	}

	if (leftSamples >= 0) {
		memcpy(buffer + destOffset, &_buffer[_bufferOffset], MIN(leftSamples * 2, _bufferSize));
		_bufferOffset += leftSamples;
	}

	_playedSamples += numSamples;

	return numSamples;
}

bool AudioStreamInstance::readPacket() {
	debugC(5, kDebugAudio, "readPacket()");

	if (_file->eos() || (_currentReadSize >= _totalSize)) {
		if (_looping) {
			_file->seek(8);
			_currentReadSize = 8;
			_lastSample = 0;
			_lastStepIndex = 0;
		} else {
			_bufferSize = 0;
			stopNow();
			return false;
		}
	}
	int16 numCompressedBytes = _file->readSint16LE();
	int16 numDecompressedBytes = _file->readSint16LE();
	_file->readSint32LE();

	if (numCompressedBytes > _compBufferSize) {
		delete[] _compBuffer;
		_compBufferSize = numCompressedBytes;
		_compBuffer = new uint8[_compBufferSize];
	}

	if (numDecompressedBytes > _bufferMaxSize) {
		delete[] _buffer;
		_bufferMaxSize = numDecompressedBytes;
		_buffer = new int16[numDecompressedBytes];
	}

	_bufferSize = numDecompressedBytes;
	_file->read(_compBuffer, numCompressedBytes);
	_currentReadSize += 8 + numCompressedBytes;

	decodeADPCM(_compBuffer, _buffer, numCompressedBytes);
	return true;
}

void AudioStreamInstance::decodeADPCM(uint8 *comp, int16 *dest, int32 packetSize) {
	debugC(5, kDebugAudio, "decodeADPCM(comp, dest, %d)", packetSize);

	// standard IMA ADPCM decoding
	int32 numSamples = 2 * packetSize;
	int32 samp = _lastSample;
	int32 stepIndex = _lastStepIndex;
	for (int32 i = 0; i < numSamples; i++) {
		uint8 comm = *comp;
		bool isOddSample = (i & 1);

		uint8 code;
		if (!isOddSample)
			code = comm & 0xf;
		else
			code = (comm & 0xf0) >> 4;

		uint8 sample = code & 0x7;

		int32 step = Audio::Ima_ADPCMStream::_imaTable[stepIndex];
		int32 E = step >> 3;
		if (sample & 4)
			E += step;
		if (sample & 2)
			E += step >> 1;
		if (sample & 1)
			E += step >> 2;

		stepIndex += Audio::ADPCMStream::_stepAdjustTable[sample];
		stepIndex = CLIP<int32>(stepIndex, 0, ARRAYSIZE(Audio::Ima_ADPCMStream::_imaTable) - 1);

		if (code & 0x8)
			samp -= E;
		else
			samp += E;

		samp = CLIP<int32>(samp, -32768, 32767);

		*dest = samp;
		if (isOddSample)
			comp++;
		dest++;
	}

	_lastSample = samp;
	_lastStepIndex = stepIndex;
}

void AudioStreamInstance::play(bool fade, Audio::Mixer::SoundType soundType) {
	debugC(1, kDebugAudio, "play(%d)", (fade) ? 1 : 0);

	_stopped = false;
	_fadingIn = fade;
	_fadeTime = 0;
	_soundType = soundType;
	_musicAttenuation = 1000; // max volume
	_mixer->playStream(soundType, &_handle, this, -1);
	handleFade(0);
}

void AudioStreamInstance::handleFade(int32 numSamples) {
	debugC(5, kDebugAudio, "handleFade(%d)", numSamples);

	// Fading enabled only for music
	if (_soundType != Audio::Mixer::kMusicSoundType)
		return;

	int32 finalVolume = _volume;

	if (_fadingOut) {
		_fadeTime += numSamples;

		if (_fadeTime > 40960) {
			_fadeTime = 40960;
			stopNow();
			_fadingOut = false;
		}
		finalVolume = _volume - _fadeTime * _volume / 40960;
	} else {
		if (_fadingIn) {
			_fadeTime += numSamples;
			if (_fadeTime > 40960) {
				_fadeTime = 40960;
				_fadingIn = false;
			}

			finalVolume = _volume * _fadeTime / 40960;
		}
	}

	// the music is too loud when someone is talking
	// smoothing to avoid big volume changes
	if (_man->voiceStillPlaying()) {
		_musicAttenuation -= numSamples >> 4;
		if (_musicAttenuation < 250)
			_musicAttenuation = 250;
	} else {
		_musicAttenuation += numSamples >> 5;
		if (_musicAttenuation > 1000)
			_musicAttenuation = 1000;
	}

	_mixer->setChannelVolume(_handle, finalVolume * _musicAttenuation / 1000);
}

void AudioStreamInstance::stop(bool fade /*= false*/) {
	debugC(1, kDebugAudio, "stop(%d)", (fade) ? 1 : 0);

	if (fade) {
		if (!_fadingOut) {
			_fadingIn = false;
			_fadingOut = true;
			_fadeTime = 0;
		}
	} else {
		stopNow();
	}
}

void AudioStreamInstance::stopNow() {
	debugC(1, kDebugAudio, "stopNow()");

	_stopped = true;
}

void AudioStreamInstance::setVolume(int32 volume) {
	debugC(1, kDebugAudio, "setVolume(%d)", volume);

	_volume = volume;
	_mixer->setChannelVolume(_handle, volume);
}

AudioStreamPackage::AudioStreamPackage(ToonEngine *vm) : _vm(vm) {
	_indexBuffer = NULL;
	_file = NULL;
}

AudioStreamPackage::~AudioStreamPackage() {
	delete[] _indexBuffer;
	delete _file;
}

bool AudioStreamPackage::loadAudioPackage(const Common::String &indexFile, const Common::String &streamFile) {
	debugC(4, kDebugAudio, "loadAudioPackage(%s, %s)", indexFile.c_str(), streamFile.c_str());

	uint32 size = 0;
	uint8 *fileData = _vm->resources()->getFileData(indexFile, &size);
	if (!fileData)
		return false;

	delete[] _indexBuffer;
	_indexBuffer = new uint32[size / 4];
	memcpy(_indexBuffer, fileData, size);

	_file = _vm->resources()->openFile(streamFile);
	if (!_file)
		return false;

	return true;
}

void AudioStreamPackage::getInfo(int32 id, int32 *offset, int32 *size) {
	debugC(1, kDebugAudio, "getInfo(%d, offset, size)", id);

	*offset = READ_LE_UINT32(_indexBuffer + id);
	*size = READ_LE_UINT32(_indexBuffer + id + 1) - READ_LE_UINT32(_indexBuffer + id);
}

Common::SeekableReadStream *AudioStreamPackage::getStream(int32 id, bool ownMemory) {
	debugC(1, kDebugAudio, "getStream(%d, %d)", id, (ownMemory) ? 1 : 0);

	int32 offset = 0;
	int32 size = 0;
	getInfo(id, &offset, &size);
	if (ownMemory) {
		byte *memory = (byte *)malloc(size);
		_file->seek(offset);
		_file->read(memory, size);
		return new Common::MemoryReadStream(memory, size, DisposeAfterUse::YES);
	} else {
		return new Common::SeekableSubReadStream(_file, offset, size + offset);
	}
}

void AudioManager::startAmbientSFX(int32 id, int32 delay, int32 mode, int32 volume)
{
	int32 found = -1;
	for (int32 i = 0; i < 4; i++) {
		if (!_ambientSFXs[i]._enabled) {
			found = i;
			break;
		}
	}

	if (found < 0)
		return;

	_ambientSFXs[found]._lastTimer = _vm->getOldMilli() - 1;
	_ambientSFXs[found]._delay = delay;
	_ambientSFXs[found]._enabled = true;
	_ambientSFXs[found]._mode = mode;
	_ambientSFXs[found]._volume = volume;
	_ambientSFXs[found]._id = id;
	updateAmbientSFX();

}

void AudioManager::setAmbientSFXVolume(int32 id, int volume) {
	for (int32 i = 0; i < 4; i++) {
		AudioAmbientSFX* ambient = &_ambientSFXs[i];
		if (ambient->_id == id && ambient->_enabled) {
			ambient->_volume = volume;
			if (ambient->_channel >= 0 && _channels[ambient->_channel] && _channels[ambient->_channel]->isPlaying()) {
				_channels[ambient->_channel]->setVolume(volume);
			}
			break;
		}
	}
}

void AudioManager::killAmbientSFX(int32 id)
{
	for (int32 i = 0; i < 4; i++) {
		AudioAmbientSFX* ambient = &_ambientSFXs[i];
		if (ambient->_id == id && ambient->_enabled) {
			ambient->_enabled = false;
			ambient->_id = -1;

			if (_channels[ambient->_channel]) {
				_channels[ambient->_channel]->stop(false);
			}
		}

	}
}

void AudioManager::killAllAmbientSFX()
{
	for (int32 i = 0; i < 4; i++) {
		AudioAmbientSFX* ambient = &_ambientSFXs[i];
		if (ambient->_enabled) {
			ambient->_enabled = false;
			ambient->_id = -1;
			if (ambient->_channel >= 0 && _channels[ambient->_channel] && _channels[ambient->_channel]->isPlaying()) {
				_channels[ambient->_channel]->stop(false);
			}
			ambient->_channel = -1;
		}
	}
}

void AudioManager::updateAmbientSFX()
{
	if (_vm->getMoviePlayer()->isPlaying())
		return;

	for (int32 i = 0; i < 4; i++) {
		AudioAmbientSFX* ambient = &_ambientSFXs[i];
		if (ambient->_enabled && (ambient->_channel < 0 || !(_channels[ambient->_channel] && _channels[ambient->_channel]->isPlaying()))) {
			if  (ambient->_mode == 1) {
				if (_vm->randRange(0, 32767) < ambient->_delay) {
					ambient->_channel = playSFX(ambient->_id, ambient->_volume, false);
				}
			} else {
				if (_vm->getOldMilli() > ambient->_lastTimer) {
					ambient->_channel = playSFX(ambient->_id, ambient->_volume, false);
					ambient->_lastTimer = _vm->getOldMilli(); // + 60 * _vm->getTickLength() * ambient->_delay;
				}
			}
		}
	}
}

} // End of namespace Toon
