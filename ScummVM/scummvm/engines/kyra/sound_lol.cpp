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

#ifdef ENABLE_LOL

#include "kyra/lol.h"
#include "kyra/sound.h"
#include "kyra/resource.h"

#include "common/system.h"

#include "audio/audiostream.h"

namespace Kyra {

bool LoLEngine::snd_playCharacterSpeech(int id, int8 speaker, int) {
	if (!speechEnabled())
		return false;

	if (speaker < 65) {
		if (_characters[speaker].flags & 1)
			speaker = (int) _characters[speaker].name[0];
		else
			speaker = 0;
	}

	if (_lastSpeechId == id && speaker == _lastSpeaker)
		return true;

	_lastSpeechId = id;
	_lastSpeaker = speaker;
	_nextSpeechId = _nextSpeaker = -1;

	Common::String pattern1;
	Common::String file1;
	Common::String file2;
	Common::String file3;

	SpeechList newSpeechList;

	Common::String pattern2 = Common::String::format("%02d", id & 0x4000 ? 0 : _curTlkFile);

	if (id & 0x4000) {
		pattern1 = Common::String::format("%03X", id & 0x3fff);
	} else if (id < 1000) {
		pattern1 = Common::String::format("%03d", id);
	} else {
		file3 = Common::String::format("@%04d%c.%s", id - 1000, (char)speaker, pattern2.c_str());
		if (_sound->isVoicePresent(file3.c_str()))
			newSpeechList.push_back(_sound->getVoiceStream(file3.c_str()));
	}

	if (file3.empty()) {
		for (char i = 0; ; i++) {
			char symbol = '0' + i;
			file1 = Common::String::format("%s%c%c.%s", pattern1.c_str(), (char)speaker, symbol, pattern2.c_str());
			file2 = Common::String::format("%s%c%c.%s", pattern1.c_str(), '_', symbol, pattern2.c_str());
			if (_sound->isVoicePresent(file1.c_str()))
				newSpeechList.push_back(_sound->getVoiceStream(file1.c_str()));
			else if (_sound->isVoicePresent(file2.c_str()))
				newSpeechList.push_back(_sound->getVoiceStream(file2.c_str()));
			else
				break;
		}
	}

	if (newSpeechList.empty())
		return false;

	while (_sound->voiceIsPlaying(&_speechHandle))
		delay(_tickLength, true);

	while (_sound->allVoiceChannelsPlaying())
		delay(_tickLength);

	for (SpeechList::iterator i = _speechList.begin(); i != _speechList.end(); ++i)
		delete *i;
	_speechList.clear();
	_speechList = newSpeechList;

	_activeVoiceFileTotalTime = 0;
	for (SpeechList::iterator i = _speechList.begin(); i != _speechList.end();) {
		// Just in case any file loading failed: Remove the bad streams here.
		if (!*i)
			i = _speechList.erase(i);
		else
			_activeVoiceFileTotalTime += (*i++)->getLength().msecs();
	}

	_sound->playVoiceStream(*_speechList.begin(), &_speechHandle);
	_speechList.pop_front();

	if (!_activeVoiceFileTotalTime)
		return false;

	_tim->_abortFlag = 0;

	return true;
}

int LoLEngine::snd_updateCharacterSpeech() {
	if (_sound->voiceIsPlaying(&_speechHandle))
		return 2;

	if (_speechList.begin() != _speechList.end()) {
		_sound->playVoiceStream(*_speechList.begin(), &_speechHandle);
		_speechList.pop_front();
		return 2;

	} else if (_nextSpeechId != -1) {
		_lastSpeechId = _lastSpeaker = -1;
		_activeVoiceFileTotalTime = 0;
		if (snd_playCharacterSpeech(_nextSpeechId, _nextSpeaker, 0))
			return 2;
	}

	_lastSpeechId = _lastSpeaker = -1;
	_activeVoiceFileTotalTime = 0;

	return 0;
}

void LoLEngine::snd_stopSpeech(bool setFlag) {
	if (!_sound->voiceIsPlaying(&_speechHandle))
		return;

	//_dlgTimer = 0;
	_sound->voiceStop(&_speechHandle);
	_activeVoiceFileTotalTime = 0;
	_nextSpeechId = _nextSpeaker = -1;

	for (SpeechList::iterator i = _speechList.begin(); i != _speechList.end(); ++i)
		delete *i;
	_speechList.clear();

	if (setFlag)
		_tim->_abortFlag = 1;
}

void LoLEngine::snd_playSoundEffect(int track, int volume) {
	if ((track == 1 && (_lastSfxTrack == -1 || _lastSfxTrack == 1)) || shouldQuit())
		return;

	_lastSfxTrack = track;
	if (track == -1 || track >= _ingameSoundListSize)
		return;

	volume &= 0xff;
	int16 prIndex = (int16)READ_LE_UINT16(&_ingameSoundIndex[track * 2 + 1]);
	uint16 priority = (prIndex > 0) ? (prIndex * volume) >> 8 : -prIndex;

	static const uint8 volTable1[] = { 223, 159, 95, 47, 15, 0 };
	static const uint8 volTable2[] = { 255, 191, 127, 63, 30, 0 };

	for (int i = 0; i < 6; i++) {
		if (volTable1[i] < volume) {
			volume = volTable2[i];
			break;
		}
	}

	int16 vocIndex = (int16)READ_LE_UINT16(&_ingameSoundIndex[track * 2]);

	bool hasVocFile = false;
	if (vocIndex != -1) {
		if (scumm_stricmp(_ingameSoundList[vocIndex], "EMPTY"))
			hasVocFile = true;
	}

	if (hasVocFile) {
		if (_sound->isVoicePresent(_ingameSoundList[vocIndex]))
			_sound->voicePlay(_ingameSoundList[vocIndex], 0, volume, priority, true);
	} else if (_flags.platform == Common::kPlatformPC) {
		if (_sound->getSfxType() == Sound::kMidiMT32)
			track = (track < _ingameMT32SoundIndexSize) ? (_ingameMT32SoundIndex[track] - 1) : -1;
		else if (_sound->getSfxType() == Sound::kMidiGM)
			track = (track < _ingameGMSoundIndexSize) ? (_ingameGMSoundIndex[track] - 1) : -1;
		else if (_sound->getSfxType() == Sound::kPCSpkr)
			track = (track < _ingamePCSpeakerSoundIndexSize) ? (_ingamePCSpeakerSoundIndex[track] - 1) : -1;

		if (track == 168)
			track = 167;

		if (track != -1)
			KyraEngine_v1::snd_playSoundEffect(track, volume);
	}
}

bool LoLEngine::snd_processEnvironmentalSoundEffect(int soundId, int block) {
	if (!KyraRpgEngine::snd_processEnvironmentalSoundEffect(soundId, block))
		return false;

	if (block != _currentBlock) {
		static const int8 blockShiftTable[] = { -32, -31, 1, 33, 32, 31, -1, -33 };
		uint16 cbl = _currentBlock;

		for (int i = 3; i > 0; i--) {
			int dir = calcMonsterDirection(cbl & 0x1f, cbl >> 5, block & 0x1f, block >> 5);
			cbl = (cbl + blockShiftTable[dir]) & 0x3ff;
			if (cbl != block) {
				if (testWallFlag(cbl, 0, 1))
					_environmentSfxVol >>= 1;
			}
		}
	}

	if (!soundId || _sceneUpdateRequired)
		return false;

	return snd_processEnvironmentalSoundEffect(0, 0);
}

void LoLEngine::snd_queueEnvironmentalSoundEffect(int soundId, int block) {
	if (_envSfxUseQueue && _envSfxNumTracksInQueue < 10) {
		_envSfxQueuedTracks[_envSfxNumTracksInQueue] = soundId;
		_envSfxQueuedBlocks[_envSfxNumTracksInQueue] = block;
		_envSfxNumTracksInQueue++;
	} else {
		snd_processEnvironmentalSoundEffect(soundId, block);
	}
}

void LoLEngine::snd_playQueuedEffects() {
	for (int i = 0; i < _envSfxNumTracksInQueue; i++)
		snd_processEnvironmentalSoundEffect(_envSfxQueuedTracks[i], _envSfxQueuedBlocks[i]);
	_envSfxNumTracksInQueue = 0;
}

void LoLEngine::snd_loadSoundFile(int track) {
	if (_sound->musicEnabled()) {
		if (_flags.platform != Common::kPlatformPC98) {
			int t = (track - 250) * 3;
			if (_curMusicFileIndex != _musicTrackMap[t] || _curMusicFileExt != (char)_musicTrackMap[t + 1]) {
				snd_stopMusic();
				_sound->loadSoundFile(Common::String::format("LORE%02d%c", _musicTrackMap[t], (char)_musicTrackMap[t + 1]));
				_curMusicFileIndex = _musicTrackMap[t];
				_curMusicFileExt = (char)_musicTrackMap[t + 1];
			} else {
				snd_stopMusic();
			}
		}
	}
}

int LoLEngine::snd_playTrack(int track) {
	if (track == -1)
		return _lastMusicTrack;

	int res = _lastMusicTrack;
	_lastMusicTrack = track;

	if (_sound->musicEnabled()) {
		if (_flags.platform == Common::kPlatformPC98) {
			_sound->playTrack(track - 249);
		} else {
			snd_loadSoundFile(track);
			int t = (track - 250) * 3;
			_sound->playTrack(_musicTrackMap[t + 2]);
		}
	}

	return res;
}

int LoLEngine::snd_stopMusic() {
	if (_sound->musicEnabled()) {
		if (_sound->isPlaying()) {
			_sound->beginFadeOut();
			_system->delayMillis(3 * _tickLength);
		}

		_sound->haltTrack();
	}
	return snd_playTrack(-1);
}

int LoLEngine::convertVolumeToMixer(int value) {
	value -= 2;
	return (value * Audio::Mixer::kMaxMixerVolume) / 100;
}

int LoLEngine::convertVolumeFromMixer(int value) {
	return (value * 100) / Audio::Mixer::kMaxMixerVolume + 2;
}

} // End of namespace Kyra

#endif // ENABLE_LOL
