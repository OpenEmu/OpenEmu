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


#include "common/endian.h"

#include "sky/music/adlibmusic.h"
#include "sky/music/adlibchannel.h"
#include "audio/mixer.h"
#include "sky/sky.h"

namespace Sky {

AdLibMusic::AdLibMusic(Audio::Mixer *pMixer, Disk *pDisk) : MusicBase(pMixer, pDisk) {
	_driverFileBase = 60202;
	_sampleRate = pMixer->getOutputRate();

	_opl = makeAdLibOPL(_sampleRate);

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

AdLibMusic::~AdLibMusic() {
	OPLDestroy(_opl);
	_mixer->stopHandle(_soundHandle);
}

int AdLibMusic::readBuffer(int16 *data, const int numSamples) {
	if (_musicData == NULL) {
		// no music loaded
		memset(data, 0, numSamples * sizeof(int16));
	} else if ((_currentMusic == 0) || (_numberOfChannels == 0)) {
		// music loaded but not played as of yet
		memset(data, 0, numSamples * sizeof(int16));
		// poll anyways as pollMusic() can activate the music
		pollMusic();
		_nextMusicPoll = _sampleRate / 50;
	} else {
		uint32 render;
		uint remaining = numSamples;
		while (remaining) {
			render = (remaining > _nextMusicPoll) ? _nextMusicPoll : remaining;
			remaining -= render;
			_nextMusicPoll -= render;
			YM3812UpdateOne(_opl, data, render);
			data += render;
			if (_nextMusicPoll == 0) {
				pollMusic();
				_nextMusicPoll = _sampleRate / 50;
			}
		}
	}
	return numSamples;
}

void AdLibMusic::setupPointers() {
	if (SkyEngine::_systemVars.gameVersion == 109) {
		// disk demo uses a different AdLib driver version, some offsets have changed
		//_musicDataLoc = (_musicData[0x11CC] << 8) | _musicData[0x11CB];
		//_initSequence = _musicData + 0xEC8;

		_musicDataLoc = READ_LE_UINT16(_musicData + 0x1200);
		_initSequence = _musicData + 0xEFB;
	} else if (SkyEngine::_systemVars.gameVersion == 267) {
		_musicDataLoc = READ_LE_UINT16(_musicData + 0x11F7);
		_initSequence = _musicData + 0xE87;
	} else {
		_musicDataLoc = READ_LE_UINT16(_musicData + 0x1201);
		_initSequence = _musicData + 0xE91;
	}
	_nextMusicPoll = 0;
}

void AdLibMusic::setupChannels(uint8 *channelData) {
	_numberOfChannels = channelData[0];
	channelData++;
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
		uint16 chDataStart = READ_LE_UINT16((uint16 *)channelData + cnt) + _musicDataLoc;
		_channels[cnt] = new AdLibChannel(_opl, _musicData, chDataStart);
	}
}

void AdLibMusic::startDriver() {
	uint16 cnt = 0;
	while (_initSequence[cnt] || _initSequence[cnt + 1]) {
		OPLWriteReg (_opl, _initSequence[cnt], _initSequence[cnt + 1]);
		cnt += 2;
	}
}

void AdLibMusic::setVolume(uint16 param) {
	_musicVolume = param;
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, 2 * param);
}

bool AdLibMusic::isStereo() const {
	return false;
}

bool AdLibMusic::endOfData() const {
	return false;
}

int AdLibMusic::getRate() const {
	return _sampleRate;
}

} // End of namespace Sky
