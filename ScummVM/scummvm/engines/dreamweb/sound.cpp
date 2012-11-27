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

#include "audio/decoders/raw.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"

#include "dreamweb/dreamweb.h"
#include "dreamweb/sound.h"

namespace DreamWeb {

DreamWebSound::DreamWebSound(DreamWebEngine *vm) : _vm(vm) {
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));

	_currentSample = 0xff;
	_channel0Playing = 255;
	_channel0Repeat = 0;
	_channel0NewSound = false;
	_channel1Playing = 255;
	_channel1NewSound = false;

	_volume = 0;
	_volumeTo = 0;
	_volumeDirection = 0;
	_volumeCount = 0;
}

DreamWebSound::~DreamWebSound() {
}

bool DreamWebSound::loadSpeech(byte type1, int idx1, byte type2, int idx2) {
	cancelCh1();

	Common::String name = Common::String::format("%c%02d%c%04d.RAW", type1, idx1, type2, idx2);
	debug(2, "loadSpeech() name:%s", name.c_str());
	return loadSpeech(name);
}

void DreamWebSound::volumeChange(uint8 value, int8 direction) {
	_volumeTo = value;
	_volumeDirection = direction;
}

void DreamWebSound::volumeAdjust() {
	if (_volumeDirection == 0)
		return;
	if (_volume != _volumeTo) {
		_volumeCount += 64;
		// Only modify the volume every 256/64 = 4th time around
		if (_volumeCount == 0)
			_volume += _volumeDirection;
	} else {
		_volumeDirection = 0;
	}
}

void DreamWebSound::playChannel0(uint8 index, uint8 repeat) {
	debug(1, "playChannel0(index:%d, repeat:%d)", index, repeat);

	_channel0Playing = index;
	_channel0Repeat = repeat;
	_channel0NewSound = true;
}

void DreamWebSound::playChannel1(uint8 index) {
	debug(1, "playChannel1(index:%d)", index);
	if (_channel1Playing == 7)
		return;

	_channel1Playing = index;
	_channel1NewSound = true;
}

void DreamWebSound::cancelCh0() {
	debug(1, "cancelCh0()");
	_channel0Playing = 255;
	_channel0Repeat = 0;
	stopSound(0);
}

void DreamWebSound::cancelCh1() {
	debug(1, "cancelCh1()");
	_channel1Playing = 255;
	stopSound(1);
}

void DreamWebSound::loadRoomsSample(uint8 sample) {
	debug(1, "loadRoomsSample(sample:%d)", sample);

	if (sample == 255 || _currentSample == sample)
		return; // loaded already

	assert(sample < 100);
	Common::String sampleSuffix = Common::String::format("V%02d", sample);
	_currentSample = sample;

	uint8 ch0 = _channel0Playing;
	if (ch0 >= 12 && ch0 != 255)
		cancelCh0();
	uint8 ch1 = _channel1Playing;
	if (ch1 >= 12)
		cancelCh1();
	loadSounds(1, sampleSuffix.c_str());
}

void DreamWebSound::playSound(uint8 channel, uint8 id, uint8 loops) {
	debug(1, "playSound(channel:%u, id:%u, loops:%u)", channel, id, loops);

	int bank = 0;
	bool speech = false;
	Audio::Mixer::SoundType type = channel == 0?
		Audio::Mixer::kMusicSoundType: Audio::Mixer::kSFXSoundType;

	if (id >= 12) {
		id -= 12;
		bank = 1;
		if (id == 50) {
			speech = true;
			type = Audio::Mixer::kSpeechSoundType;
		}
	}
	const SoundData &data = _soundData[bank];

	Audio::SeekableAudioStream *raw;
	if (!speech) {
		if (id >= data.samples.size() || data.samples[id].size == 0) {
			warning("invalid sample #%u played", id);
			return;
		}

		const Sample &sample = data.samples[id];
		uint8 *buffer = (uint8 *)malloc(sample.size);
		if (!buffer)
			error("out of memory: cannot allocate memory for sound(%u bytes)", sample.size);
		memcpy(buffer, data.data.begin() + sample.offset, sample.size);

		raw = Audio::makeRawStream(buffer, sample.size, 22050, Audio::FLAG_UNSIGNED);
	} else {
		uint8 *buffer = (uint8 *)malloc(_speechData.size());
		if (!buffer)
			error("out of memory: cannot allocate memory for sound(%u bytes)", _speechData.size());
		memcpy(buffer, _speechData.begin(), _speechData.size());
		raw = Audio::makeRawStream(buffer, _speechData.size(), 22050, Audio::FLAG_UNSIGNED);
	}

	Audio::AudioStream *stream;
	if (loops > 1) {
		stream = new Audio::LoopingAudioStream(raw, (loops < 255) ? loops : 0);
	} else
		stream = raw;

	if (_vm->_mixer->isSoundHandleActive(_channelHandle[channel]))
		_vm->_mixer->stopHandle(_channelHandle[channel]);
	_vm->_mixer->playStream(type, &_channelHandle[channel], stream);
}

void DreamWebSound::stopSound(uint8 channel) {
	debug(1, "stopSound(%u)", channel);
	assert(channel == 0 || channel == 1);
	_vm->_mixer->stopHandle(_channelHandle[channel]);
}

bool DreamWebSound::loadSpeech(const Common::String &filename) {
	if (!_vm->hasSpeech())
		return false;

	Common::File file;
	if (!file.open(_vm->getSpeechDirName() + "/" + filename))
		return false;

	debug(1, "loadSpeech(%s)", filename.c_str());

	uint size = file.size();
	_speechData.resize(size);
	file.read(_speechData.begin(), size);
	file.close();
	return true;
}

void DreamWebSound::soundHandler() {
	_vm->_subtitles = ConfMan.getBool("subtitles");
	volumeAdjust();

	uint volume = _volume;
	//.vol file loaded into soundbuf:0x4000
	//volume table at (volume * 0x100 + 0x3f00)
	//volume value could be from 1 to 7
	//1 - 0x10-0xff
	//2 - 0x1f-0xdf
	//3 - 0x2f-0xd0
	//4 - 0x3e-0xc1
	//5 - 0x4d-0xb2
	//6 - 0x5d-0xa2
	//7 - 0x6f-0x91
	if (volume >= 8)
		volume = 7;
	volume = (8 - volume) * Audio::Mixer::kMaxChannelVolume / 8;
	_vm->_mixer->setChannelVolume(_channelHandle[0], volume);

	if (_channel0NewSound) {
		_channel0NewSound = false;
		if (_channel0Playing != 255) {
			playSound(0, _channel0Playing, _channel0Repeat);
		}
	}
	if (_channel1NewSound) {
		_channel1NewSound = false;
		if (_channel1Playing != 255) {
			playSound(1, _channel1Playing, 1);
		}
	}

	if (!_vm->_mixer->isSoundHandleActive(_channelHandle[0])) {
		_channel0Playing = 255;
	}
	if (!_vm->_mixer->isSoundHandleActive(_channelHandle[1])) {
		_channel1Playing = 255;
	}
}

void DreamWebSound::loadSounds(uint bank, const Common::String &suffix) {
	Common::String filename = _vm->getDatafilePrefix() + suffix;
	debug(1, "loadSounds(%u, %s)", bank, filename.c_str());
	Common::File file;
	if (!file.open(filename)) {
		warning("cannot open %s", filename.c_str());
		return;
	}

	uint8 header[96];
	file.read(header, sizeof(header));
	uint tablesize = READ_LE_UINT16(header + 50);
	debug(1, "table size = %u", tablesize);

	SoundData &soundData = _soundData[bank];
	soundData.samples.resize(tablesize / 6);
	uint total = 0;
	for (uint i = 0; i < tablesize / 6; ++i) {
		uint8 entry[6];
		Sample &sample = soundData.samples[i];
		file.read(entry, sizeof(entry));
		sample.offset = entry[0] * 16384 + READ_LE_UINT16(entry + 1);
		sample.size = READ_LE_UINT16(entry + 3) * 2048;
		total += sample.size;
		debug(1, "offset: %08x, size: %u", sample.offset, sample.size);
	}
	soundData.data.resize(total);
	file.read(soundData.data.begin(), total);
	file.close();
}

} // End of namespace DreamWeb
