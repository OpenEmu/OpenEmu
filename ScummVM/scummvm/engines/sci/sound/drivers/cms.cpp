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

#include "sci/sound/drivers/mididriver.h"

#include "audio/softsynth/emumidi.h"
#include "audio/softsynth/cms.h"
#include "audio/mixer.h"

#include "common/system.h"

#include "sci/resource.h"

namespace Sci {

// FIXME: We don't seem to be sending the polyphony init data, so disable this for now
#define CMS_DISABLE_VOICE_MAPPING

class MidiDriver_CMS : public MidiDriver_Emulated {
public:
	MidiDriver_CMS(Audio::Mixer *mixer, ResourceManager *resMan)
	    : MidiDriver_Emulated(mixer), _resMan(resMan), _cms(0), _rate(0), _playSwitch(true), _masterVolume(0) {
	}

	int open();
	void close();

	void send(uint32 b);
	uint32 property(int prop, uint32 param);

	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }

	bool isStereo() const { return true; }
	int getRate() const { return _rate; }

	void playSwitch(bool play);
private:
	void generateSamples(int16 *buffer, int len);

	ResourceManager *_resMan;
	CMSEmulator *_cms;

	void writeToChip1(int address, int data);
	void writeToChip2(int address, int data);

	int32 _samplesPerCallback;
	int32 _samplesPerCallbackRemainder;
	int32 _samplesTillCallback;
	int32 _samplesTillCallbackRemainder;

	int _rate;
	bool _playSwitch;
	uint16 _masterVolume;

	uint8 *_patchData;

	struct Channel {
		Channel()
		    : patch(0), volume(0), pan(0x40), hold(0), extraVoices(0),
		      pitchWheel(0x2000), pitchModifier(0), pitchAdditive(false),
		      lastVoiceUsed(0) {
		}

		uint8 patch;
		uint8 volume;
		uint8 pan;
		uint8 hold;
		uint8 extraVoices;
		uint16 pitchWheel;
		uint8 pitchModifier;
		bool pitchAdditive;
		uint8 lastVoiceUsed;
	};

	Channel _channel[16];

	struct Voice {
		Voice() : channel(0xFF), note(0xFF), sustained(0xFF), ticks(0),
		    turnOffTicks(0), patchDataPtr(0), patchDataIndex(0),
		    amplitudeTimer(0), amplitudeModifier(0), turnOff(false),
		    velocity(0) {
		}

		uint8 channel;
		uint8 note;
		uint8 sustained;
		uint16 ticks;
		uint16 turnOffTicks;
		const uint8 *patchDataPtr;
		uint8 patchDataIndex;
		uint8 amplitudeTimer;
		uint8 amplitudeModifier;
		bool turnOff;
		uint8 velocity;
	};

	Voice _voice[12];

	void voiceOn(int voice, int note, int velocity);
	void voiceOff(int voice);

	void noteSend(int voice);

	void noteOn(int channel, int note, int velocity);
	void noteOff(int channel, int note);
	void controlChange(int channel, int control, int value);
	void pitchWheel(int channel, int value);

	void voiceMapping(int channel, int value);
	void bindVoices(int channel, int voices);
	void unbindVoices(int channel, int voices);
	void donateVoices();
	int findVoice(int channel);

	int findVoiceBasic(int channel);

	void updateVoiceAmplitude(int voice);
	void setupVoiceAmplitude(int voice);

	uint8 _octaveRegs[2][3];

	static const int _timerFreq = 60;

	static const int _frequencyTable[];
	static const int _velocityTable[];
};

const int MidiDriver_CMS::_frequencyTable[] = {
	  3,  10,  17,  24,
	 31,  38,  46,  51,
	 58,  64,  71,  77,
	 83,  89,  95, 101,
	107, 113, 119, 124,
	130, 135, 141, 146,
	151, 156, 162, 167,
	172, 177, 182, 186,
	191, 196, 200, 205,
	209, 213, 217, 222,
	226, 230, 234, 238,
	242, 246, 250, 253
};

const int MidiDriver_CMS::_velocityTable[] = {
	 1,  3,  6,  8,  9, 10, 11, 12,
	12, 13, 13, 14, 14, 14, 15, 15,
	 0,  1,  2,  2,  3,  4,  4,  5,
	 6,  6,  7,  8,  8,  9, 10, 10
};

int MidiDriver_CMS::open() {
	if (_cms)
		return MERR_ALREADY_OPEN;

	assert(_resMan);
	Resource *res = _resMan->findResource(ResourceId(kResourceTypePatch, 101), 0);
	if (!res)
		return -1;

	_patchData = new uint8[res->size];
	memcpy(_patchData, res->data, res->size);

	for (uint i = 0; i < ARRAYSIZE(_channel); ++i)
		_channel[i] = Channel();

	for (uint i = 0; i < ARRAYSIZE(_voice); ++i)
		_voice[i] = Voice();

	_rate = _mixer->getOutputRate();
	_cms = new CMSEmulator(_rate);
	assert(_cms);
	_playSwitch = true;
	_masterVolume = 0;

	for (int i = 0; i < 31; ++i) {
		writeToChip1(i, 0);
		writeToChip2(i, 0);
	}

	writeToChip1(0x14, 0xFF);
	writeToChip2(0x14, 0xFF);

	writeToChip1(0x1C, 1);
	writeToChip2(0x1C, 1);

	_samplesPerCallback = getRate() / _timerFreq;
	_samplesPerCallbackRemainder = getRate() % _timerFreq;
	_samplesTillCallback = 0;
	_samplesTillCallbackRemainder = 0;

	int retVal = MidiDriver_Emulated::open();
	if (retVal != 0)
		return retVal;

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO);
	return 0;
}

void MidiDriver_CMS::close() {
	_mixer->stopHandle(_mixerSoundHandle);

	delete[] _patchData;
	delete _cms;
	_cms = 0;
}

void MidiDriver_CMS::send(uint32 b) {
	const uint8 command = b & 0xf0;
	const uint8 channel = b & 0xf;
	const uint8 op1 = (b >> 8) & 0xff;
	const uint8 op2 = (b >> 16) & 0xff;

	switch (command) {
	case 0x80:
		noteOff(channel, op1);
		break;

	case 0x90:
		noteOn(channel, op1, op2);
		break;

	case 0xB0:
		controlChange(channel, op1, op2);
		break;

	case 0xC0:
		_channel[channel].patch = op1;
		break;

	case 0xE0:
		pitchWheel(channel, (op1 & 0x7f) | ((op2 & 0x7f) << 7));
		break;

	default:
		break;
	}
}

uint32 MidiDriver_CMS::property(int prop, uint32 param) {
	switch (prop) {
	case MIDI_PROP_MASTER_VOLUME:
		if (param != 0xffff)
			_masterVolume = param;
		return _masterVolume;

	default:
		return MidiDriver_Emulated::property(prop, param);
	}
}

void MidiDriver_CMS::playSwitch(bool play) {
	_playSwitch = play;
}

void MidiDriver_CMS::writeToChip1(int address, int data) {
	_cms->portWrite(0x221, address);
	_cms->portWrite(0x220, data);

	if (address >= 16 && address <= 18)
		_octaveRegs[0][address - 16] = data;
}

void MidiDriver_CMS::writeToChip2(int address, int data) {
	_cms->portWrite(0x223, address);
	_cms->portWrite(0x222, data);

	if (address >= 16 && address <= 18)
		_octaveRegs[1][address - 16] = data;
}

void MidiDriver_CMS::voiceOn(int voiceNr, int note, int velocity) {
	Voice &voice = _voice[voiceNr];
	voice.note = note;
	voice.turnOff = false;
	voice.patchDataIndex = 0;
	voice.amplitudeTimer = 0;
	voice.ticks = 0;
	voice.turnOffTicks = 0;
	voice.patchDataPtr = _patchData + READ_LE_UINT16(&_patchData[_channel[voice.channel].patch * 2]);
	if (velocity)
		velocity = _velocityTable[(velocity >> 3)];
	voice.velocity = velocity;
	noteSend(voiceNr);
}

void MidiDriver_CMS::voiceOff(int voiceNr) {
	Voice &voice = _voice[voiceNr];
	voice.velocity = 0;
	voice.note = 0xFF;
	voice.sustained = 0;
	voice.turnOff = false;
	voice.patchDataIndex = 0;
	voice.amplitudeTimer = 0;
	voice.amplitudeModifier = 0;
	voice.ticks = 0;
	voice.turnOffTicks = 0;

	setupVoiceAmplitude(voiceNr);
}

void MidiDriver_CMS::noteSend(int voiceNr) {
	Voice &voice = _voice[voiceNr];

	int frequency = (CLIP<int>(voice.note, 21, 116) - 21) * 4;
	if (_channel[voice.channel].pitchModifier) {
		int modifier = _channel[voice.channel].pitchModifier;

		if (!_channel[voice.channel].pitchAdditive) {
			if (frequency > modifier)
				frequency -= modifier;
			else
				frequency = 0;
		} else {
			int tempFrequency = 384 - frequency;
			if (modifier < tempFrequency)
				frequency += modifier;
			else
				frequency = 383;
		}
	}

	int chipNumber = 0;
	if (voiceNr >= 6) {
		voiceNr -= 6;
		chipNumber = 1;
	}

	int octave = 0;
	while (frequency >= 48) {
		frequency -= 48;
		++octave;
	}

	frequency = _frequencyTable[frequency];

	if (chipNumber == 1)
		writeToChip2(8 + voiceNr, frequency);
	else
		writeToChip1(8 + voiceNr, frequency);

	uint8 octaveData = _octaveRegs[chipNumber][voiceNr >> 1];

	if (voiceNr & 1) {
		octaveData &= 0x0F;
		octaveData |= (octave << 4);
	} else {
		octaveData &= 0xF0;
		octaveData |= octave;
	}

	if (chipNumber == 1)
		writeToChip2(0x10 + (voiceNr >> 1), octaveData);
	else
		writeToChip1(0x10 + (voiceNr >> 1), octaveData);
}

void MidiDriver_CMS::noteOn(int channel, int note, int velocity) {
	if (note < 21 || note > 116)
		return;

	if (velocity == 0) {
		noteOff(channel, note);
		return;
	}

	for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
		if (_voice[i].channel == channel && _voice[i].note == note) {
			_voice[i].sustained = 0;
			voiceOff(i);
			voiceOn(i, note, velocity);
			return;
		}
	}

#ifdef CMS_DISABLE_VOICE_MAPPING
	int voice = findVoiceBasic(channel);
#else
	int voice = findVoice(channel);
#endif
	if (voice != -1)
		voiceOn(voice, note, velocity);
}

int MidiDriver_CMS::findVoiceBasic(int channel) {
	int voice = -1;
	int oldestVoice = -1;
	int oldestAge = -1;

	// Try to find a voice assigned to this channel that is free (round-robin)
	for (int i = 0; i < ARRAYSIZE(_voice); i++) {
		int v = (_channel[channel].lastVoiceUsed + i + 1) % ARRAYSIZE(_voice);

		if (_voice[v].note == 0xFF) {
			voice = v;
			break;
		}

		// We also keep track of the oldest note in case the search fails
		if (_voice[v].ticks > oldestAge) {
			oldestAge = _voice[v].ticks;
			oldestVoice = v;
		}
	}

	if (voice == -1) {
		if (oldestVoice != -1) {
			voiceOff(oldestVoice);
			voice = oldestVoice;
		} else {
			return -1;
		}
	}

	_voice[voice].channel = channel;
	_channel[channel].lastVoiceUsed = voice;
	return voice;
}

void MidiDriver_CMS::noteOff(int channel, int note) {
	for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
		if (_voice[i].channel == channel && _voice[i].note == note) {
			if (_channel[channel].hold != 0)
				_voice[i].sustained = true;
			else
				_voice[i].turnOff = true;
		}
	}
}

void MidiDriver_CMS::controlChange(int channel, int control, int value) {
	switch (control) {
	case 7:
		if (value) {
			value >>= 3;
			if (!value)
				++value;
		}

		_channel[channel].volume = value;
		break;

	case 10:
		_channel[channel].pan = value;
		break;

	case 64:
		_channel[channel].hold = value;

		if (!value) {
			for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
				if (_voice[i].channel == channel && _voice[i].sustained) {
					_voice[i].sustained = 0;
					_voice[i].turnOff = true;
				}
			}
		}
		break;

	case 75:
#ifndef CMS_DISABLE_VOICE_MAPPING
		voiceMapping(channel, value);
#endif
		break;

	case 123:
		for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
			if (_voice[i].channel == channel && _voice[i].note != 0xFF)
				voiceOff(i);
		}
		break;

	default:
		return;
	}
}

void MidiDriver_CMS::pitchWheel(int channelNr, int value) {
	Channel &channel = _channel[channelNr];
	channel.pitchWheel = value;
	channel.pitchAdditive = false;
	channel.pitchModifier = 0;

	if (value < 0x2000) {
		channel.pitchModifier = (0x2000 - value) / 170;
	} else if (value > 0x2000) {
		channel.pitchModifier = (value - 0x2000) / 170;
		channel.pitchAdditive = true;
	}

	for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
		if (_voice[i].channel == channelNr && _voice[i].note != 0xFF)
			noteSend(i);
	}
}

void MidiDriver_CMS::voiceMapping(int channelNr, int value) {
	int curVoices = 0;

	for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
		if (_voice[i].channel == channelNr)
			++curVoices;
	}

	curVoices += _channel[channelNr].extraVoices;

	if (curVoices == value) {
		return;
	} else if (curVoices < value) {
		bindVoices(channelNr, value - curVoices);
	} else {
		unbindVoices(channelNr, curVoices - value);
		donateVoices();
	}
}

void MidiDriver_CMS::bindVoices(int channel, int voices) {
	for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
		if (_voice[i].channel == 0xFF)
			continue;

		Voice &voice = _voice[i];
		voice.channel = channel;

		if (voice.note != 0xFF)
			voiceOff(i);

		--voices;
		if (voices == 0)
			break;
	}

	_channel[channel].extraVoices += voices;

	// The original called "PatchChange" here, since this just
	// copies the value of _channel[channel].patch to itself
	// it is left out here though.
}

void MidiDriver_CMS::unbindVoices(int channelNr, int voices) {
	Channel &channel = _channel[channelNr];

	if (channel.extraVoices >= voices) {
		channel.extraVoices -= voices;
	} else {
		voices -= channel.extraVoices;
		channel.extraVoices = 0;

		for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
			if (_voice[i].channel == channelNr
			    && _voice[i].note == 0xFF) {
				--voices;
				if (voices == 0)
					return;
			}
		}

		do {
			uint16 voiceTime = 0;
			uint voiceNr = 0;

			for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
				if (_voice[i].channel != channelNr)
					continue;

				uint16 curTime = _voice[i].turnOffTicks;
				if (curTime)
					curTime += 0x8000;
				else
					curTime = _voice[i].ticks;

				if (curTime >= voiceTime) {
					voiceNr = i;
					voiceTime = curTime;
				}
			}

			_voice[voiceNr].sustained = 0;
			voiceOff(voiceNr);
			_voice[voiceNr].channel = 0xFF;
			--voices;
		} while (voices != 0);
	}
}

void MidiDriver_CMS::donateVoices() {
	int freeVoices = 0;

	for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
		if (_voice[i].channel == 0xFF)
			++freeVoices;
	}

	if (!freeVoices)
		return;

	for (uint i = 0; i < ARRAYSIZE(_channel); ++i) {
		Channel &channel = _channel[i];

		if (!channel.extraVoices) {
			continue;
		} else if (channel.extraVoices < freeVoices) {
			freeVoices -= channel.extraVoices;
			channel.extraVoices = 0;
			bindVoices(i, channel.extraVoices);
		} else {
			channel.extraVoices -= freeVoices;
			bindVoices(i, freeVoices);
			return;
		}
	}
}

int MidiDriver_CMS::findVoice(int channelNr) {
	Channel &channel = _channel[channelNr];
	int voiceNr = channel.lastVoiceUsed;

	int newVoice = 0;
	uint16 newVoiceTime = 0;

	bool loopDone = false;
	do {
		++voiceNr;

		if (voiceNr == 12)
			voiceNr = 0;

		Voice &voice = _voice[voiceNr];

		if (voiceNr == channel.lastVoiceUsed)
			loopDone = true;

		if (voice.channel == channelNr) {
			if (voice.note == 0xFF) {
				channel.lastVoiceUsed = voiceNr;
				return voiceNr;
			}

			uint16 curTime = voice.turnOffTicks;
			if (curTime)
				curTime += 0x8000;
			else
				curTime = voice.ticks;

			if (curTime >= newVoiceTime) {
				newVoice = voiceNr;
				newVoiceTime = curTime;
			}
		}
	} while (!loopDone);

	if (newVoiceTime > 0) {
		voiceNr = newVoice;
		_voice[voiceNr].sustained = 0;
		voiceOff(voiceNr);
		channel.lastVoiceUsed = voiceNr;
		return voiceNr;
	} else {
		return -1;
	}
}

void MidiDriver_CMS::updateVoiceAmplitude(int voiceNr) {
	Voice &voice = _voice[voiceNr];

	if (voice.amplitudeTimer != 0 && voice.amplitudeTimer != 254) {
		--voice.amplitudeTimer;
		return;
	} else if (voice.amplitudeTimer == 254) {
		if (!voice.turnOff)
			return;

		voice.amplitudeTimer = 0;
	}

	int nextDataIndex = voice.patchDataIndex;
	uint8 timerData = 0;
	uint8 amplitudeData = voice.patchDataPtr[nextDataIndex];

	if (amplitudeData == 255) {
		timerData = amplitudeData = 0;
		voiceOff(voiceNr);
	} else {
		timerData = voice.patchDataPtr[nextDataIndex + 1];
		nextDataIndex += 2;
	}

	voice.patchDataIndex = nextDataIndex;
	voice.amplitudeTimer = timerData;
	voice.amplitudeModifier = amplitudeData;
}

void MidiDriver_CMS::setupVoiceAmplitude(int voiceNr) {
	Voice &voice = _voice[voiceNr];
	uint amplitude = 0;

	if (_channel[voice.channel].volume && voice.velocity
	    && voice.amplitudeModifier && _masterVolume) {
		amplitude = _channel[voice.channel].volume * voice.velocity;
		amplitude /= 0x0F;
		amplitude *= voice.amplitudeModifier;
		amplitude /= 0x0F;
		amplitude *= _masterVolume;
		amplitude /= 0x0F;

		if (!amplitude)
			++amplitude;
	}

	uint8 amplitudeData = 0;
	int pan = _channel[voice.channel].pan >> 2;
	if (pan >= 16) {
		amplitudeData = (amplitude * (31 - pan) / 0x0F) & 0x0F;
		amplitudeData |= (amplitude << 4);
	} else {
		amplitudeData = (amplitude * pan / 0x0F) & 0x0F;
		amplitudeData <<= 4;
		amplitudeData |= amplitude;
	}

	if (!_playSwitch)
		amplitudeData = 0;

	if (voiceNr >= 6)
		writeToChip2(voiceNr - 6, amplitudeData);
	else
		writeToChip1(voiceNr, amplitudeData);
}

void MidiDriver_CMS::generateSamples(int16 *buffer, int len) {
	while (len) {
		if (!_samplesTillCallback) {
			for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
				if (_voice[i].note == 0xFF)
					continue;

				++_voice[i].ticks;
				if (_voice[i].turnOff)
					++_voice[i].turnOffTicks;

				updateVoiceAmplitude(i);
				setupVoiceAmplitude(i);
			}

			_samplesTillCallback = _samplesPerCallback;
			_samplesTillCallbackRemainder += _samplesPerCallbackRemainder;
			if (_samplesTillCallbackRemainder >= _timerFreq) {
				_samplesTillCallback++;
				_samplesTillCallbackRemainder -= _timerFreq;
			}
		}

		int32 render = MIN<int32>(len, _samplesTillCallback);
		len -= render;
		_samplesTillCallback -= render;
		_cms->readBuffer(buffer, render);
		buffer += render * 2;
	}
}


class MidiPlayer_CMS : public MidiPlayer {
public:
	MidiPlayer_CMS(SciVersion version) : MidiPlayer(version) {
	}

	int open(ResourceManager *resMan) {
		if (_driver)
			return MidiDriver::MERR_ALREADY_OPEN;

		_driver = new MidiDriver_CMS(g_system->getMixer(), resMan);
		int driverRetVal = _driver->open();
		if (driverRetVal != 0)
			return driverRetVal;

		return 0;
	}

	void close() {
		_driver->setTimerCallback(0, 0);
		_driver->close();
		delete _driver;
		_driver = 0;
	}

	bool hasRhythmChannel() const { return false; }
	byte getPlayId() const { return 9; }
	int getPolyphony() const { return 12; }

	void playSwitch(bool play) { static_cast<MidiDriver_CMS *>(_driver)->playSwitch(play); }
};

MidiPlayer *MidiPlayer_CMS_create(SciVersion version) {
	return new MidiPlayer_CMS(version);
}

} // End of namespace SCI
