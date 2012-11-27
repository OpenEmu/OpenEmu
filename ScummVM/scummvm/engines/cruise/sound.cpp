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
#include "common/system.h"
#include "common/textconsole.h"

#include "cruise/cruise.h"
#include "cruise/cruise_main.h"
#include "cruise/sound.h"
#include "cruise/volume.h"

#include "audio/audiostream.h"
#include "audio/fmopl.h"
#include "audio/mixer.h"
#include "audio/mods/soundfx.h"

namespace Cruise {

class PCSoundDriver {
public:
	typedef void (*UpdateCallback)(void *);

	PCSoundDriver() { _upCb = NULL, _upRef = NULL, _musicVolume = 0, _sfxVolume = 0; }
	virtual ~PCSoundDriver() {}

	virtual void setupChannel(int channel, const byte *data, int instrument, int volume) = 0;
	virtual void setChannelFrequency(int channel, int frequency) = 0;
	virtual void stopChannel(int channel) = 0;
	virtual void playSample(const byte *data, int size, int channel, int volume) = 0;
	virtual void stopAll() = 0;
	virtual const char *getInstrumentExtension() const { return ""; }
	virtual void syncSounds();

	void setUpdateCallback(UpdateCallback upCb, void *ref);
	void resetChannel(int channel);
	void findNote(int freq, int *note, int *oct) const;


protected:
	UpdateCallback _upCb;
	void *_upRef;
	uint8 _musicVolume;
	uint8 _sfxVolume;

	static const int _noteTable[];
	static const int _noteTableCount;
};

const int PCSoundDriver::_noteTable[] = {
	0xEEE, 0xE17, 0xD4D, 0xC8C, 0xBD9, 0xB2F, 0xA8E, 0x9F7,
	0x967, 0x8E0, 0x861, 0x7E8, 0x777, 0x70B, 0x6A6, 0x647,
	0x5EC, 0x597, 0x547, 0x4FB, 0x4B3, 0x470, 0x430, 0x3F4,
	0x3BB, 0x385, 0x353, 0x323, 0x2F6, 0x2CB, 0x2A3, 0x27D,
	0x259, 0x238, 0x218, 0x1FA, 0x1DD, 0x1C2, 0x1A9, 0x191,
	0x17B, 0x165, 0x151, 0x13E, 0x12C, 0x11C, 0x10C, 0x0FD,
	0x0EE, 0x0E1, 0x0D4, 0x0C8, 0x0BD, 0x0B2, 0x0A8, 0x09F,
	0x096, 0x08E, 0x086, 0x07E, 0x077, 0x070, 0x06A, 0x064,
	0x05E, 0x059, 0x054, 0x04F, 0x04B, 0x047, 0x043, 0x03F,
	0x03B, 0x038, 0x035, 0x032, 0x02F, 0x02C, 0x02A, 0x027,
	0x025, 0x023, 0x021, 0x01F, 0x01D, 0x01C, 0x01A, 0x019,
	0x017, 0x016, 0x015, 0x013, 0x012, 0x011, 0x010, 0x00F
};

const int PCSoundDriver::_noteTableCount = ARRAYSIZE(_noteTable);

struct AdLibRegisterSoundInstrument {
	uint8 vibrato;
	uint8 attackDecay;
	uint8 sustainRelease;
	uint8 feedbackStrength;
	uint8 keyScaling;
	uint8 outputLevel;
	uint8 freqMod;
};

struct AdLibSoundInstrument {
	byte mode;
	byte channel;
	AdLibRegisterSoundInstrument regMod;
	AdLibRegisterSoundInstrument regCar;
	byte waveSelectMod;
	byte waveSelectCar;
	byte amDepth;
};

struct VolumeEntry {
	int original;
	int adjusted;
};

class AdLibSoundDriver : public PCSoundDriver, Audio::AudioStream {
public:
	AdLibSoundDriver(Audio::Mixer *mixer);
	virtual ~AdLibSoundDriver();

	// PCSoundDriver interface
	virtual void setupChannel(int channel, const byte *data, int instrument, int volume);
	virtual void stopChannel(int channel);
	virtual void stopAll();

	// AudioStream interface
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return false; }
	virtual bool endOfData() const { return false; }
	virtual int getRate() const { return _sampleRate; }

	void initCard();
	void update(int16 *buf, int len);
	void setupInstrument(const byte *data, int channel);
	void setupInstrument(const AdLibSoundInstrument *ins, int channel);
	void loadRegisterInstrument(const byte *data, AdLibRegisterSoundInstrument *reg);
	virtual void loadInstrument(const byte *data, AdLibSoundInstrument *asi) = 0;
	virtual void syncSounds();

	void adjustVolume(int channel, int volume);

protected:
	FM_OPL *_opl;
	int _sampleRate;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;

	byte _vibrato;
	VolumeEntry _channelsVolumeTable[5];
	AdLibSoundInstrument _instrumentsTable[5];

	static const int _freqTable[];
	static const int _freqTableCount;
	static const int _operatorsTable[];
	static const int _operatorsTableCount;
	static const int _voiceOperatorsTable[];
	static const int _voiceOperatorsTableCount;
};

const int AdLibSoundDriver::_freqTable[] = {
	0x157, 0x16C, 0x181, 0x198, 0x1B1, 0x1CB,
	0x1E6, 0x203, 0x222, 0x243, 0x266, 0x28A
};

const int AdLibSoundDriver::_freqTableCount = ARRAYSIZE(_freqTable);

const int AdLibSoundDriver::_operatorsTable[] = {
	0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13,	16, 17, 18, 19, 20, 21
};

const int AdLibSoundDriver::_operatorsTableCount = ARRAYSIZE(_operatorsTable);

const int AdLibSoundDriver::_voiceOperatorsTable[] = {
	0, 3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 16, 16, 14, 14, 17, 17, 13, 13
};

const int AdLibSoundDriver::_voiceOperatorsTableCount = ARRAYSIZE(_voiceOperatorsTable);

class AdLibSoundDriverADL : public AdLibSoundDriver {
public:
	AdLibSoundDriverADL(Audio::Mixer *mixer) : AdLibSoundDriver(mixer) {}
	virtual const char *getInstrumentExtension() const { return ".ADL"; }
	virtual void loadInstrument(const byte *data, AdLibSoundInstrument *asi);
	virtual void setChannelFrequency(int channel, int frequency);
	virtual void playSample(const byte *data, int size, int channel, int volume);
};

class PCSoundFxPlayer {
private:
	enum {
		NUM_INSTRUMENTS = 15,
		NUM_CHANNELS = 4
	};

	void update();
	void handleEvents();
	void handlePattern(int channel, const byte *patternData);

	char _musicName[33];
	bool _playing;
	bool _songPlayed;
	int _currentPos;
	int _currentOrder;
	int _numOrders;
	int _eventsDelay;
	bool _looping;
	int _fadeOutCounter;
	int _updateTicksCounter;
	int _instrumentsChannelTable[NUM_CHANNELS];
	byte *_sfxData;
	byte *_instrumentsData[NUM_INSTRUMENTS];
	PCSoundDriver *_driver;

public:
	PCSoundFxPlayer(PCSoundDriver *driver);
	~PCSoundFxPlayer();

	bool load(const char *song);
	void play();
	void stop();
	void unload();
	void fadeOut();
	void doSync(Common::Serializer &s);

	static void updateCallback(void *ref);

	bool songLoaded() const { return _sfxData != NULL; }
	bool songPlayed() const { return _songPlayed; }
	bool playing() const { return _playing; }
	uint8 numOrders() const { assert(_sfxData); return _sfxData[470]; }
	void setNumOrders(uint8 v) { assert(_sfxData); _sfxData[470] = v; }
	void setPattern(int offset, uint8 value) { assert(_sfxData); _sfxData[472 + offset] = value; }
	const char *musicName() { return _musicName; }

	// Note: Original game never actually uses looping variable. Songs are hardcoded to loop
	bool looping() const { return _looping; }
	void setLooping(bool v) { _looping = v; }
};

byte *readBundleSoundFile(const char *name) {
	// Load the correct file
	int fileIdx = findFileInDisks(name);
	if (fileIdx < 0) return NULL;

	int unpackedSize = volumePtrToFileDescriptor[fileIdx].extSize + 2;
	byte *data = (byte *)MemAlloc(unpackedSize);
	assert(data);

	if (volumePtrToFileDescriptor[fileIdx].size + 2 != unpackedSize) {
		uint8 *packedBuffer = (uint8 *)mallocAndZero(volumePtrToFileDescriptor[fileIdx].size + 2);

		loadPackedFileToMem(fileIdx, packedBuffer);

		//uint32 realUnpackedSize = READ_BE_UINT32(packedBuffer + volumePtrToFileDescriptor[fileIdx].size - 4);

		delphineUnpack(data, packedBuffer, volumePtrToFileDescriptor[fileIdx].size);

		MemFree(packedBuffer);
	} else {
		loadPackedFileToMem(fileIdx, data);
	}

	return data;
}


void PCSoundDriver::setUpdateCallback(UpdateCallback upCb, void *ref) {
	_upCb = upCb;
	_upRef = ref;
}

void PCSoundDriver::findNote(int freq, int *note, int *oct) const {
	*note = _noteTableCount - 1;
	for (int i = 0; i < _noteTableCount; ++i) {
		if (_noteTable[i] <= freq) {
			*note = i;
			break;
		}
	}

	*oct = *note / 12;
	*note %= 12;
}

void PCSoundDriver::resetChannel(int channel) {
	stopChannel(channel);
	stopAll();
}

void PCSoundDriver::syncSounds() {
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	bool music_mute = mute;
	bool sfx_mute = mute;

	if (!mute) {
		music_mute = ConfMan.getBool("music_mute");
		sfx_mute = ConfMan.getBool("sfx_mute");
	}

	// Get the new music and sfx volumes
	_musicVolume = music_mute ? 0 : MIN(255, ConfMan.getInt("music_volume"));
	_sfxVolume = sfx_mute ? 0 : MIN(255, ConfMan.getInt("sfx_volume"));
}

AdLibSoundDriver::AdLibSoundDriver(Audio::Mixer *mixer)
	: _mixer(mixer) {
	_sampleRate = _mixer->getOutputRate();
	_opl = makeAdLibOPL(_sampleRate);

	for (int i = 0; i < 5; ++i) {
		_channelsVolumeTable[i].original = 0;
		_channelsVolumeTable[i].adjusted = 0;
	}
	memset(_instrumentsTable, 0, sizeof(_instrumentsTable));
	initCard();
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_musicVolume = ConfMan.getBool("music_mute") ? 0 : MIN(255, ConfMan.getInt("music_volume"));
	_sfxVolume = ConfMan.getBool("sfx_mute") ? 0 : MIN(255, ConfMan.getInt("sfx_volume"));
}

AdLibSoundDriver::~AdLibSoundDriver() {
	_mixer->stopHandle(_soundHandle);
	OPLDestroy(_opl);
}

void AdLibSoundDriver::syncSounds() {
	PCSoundDriver::syncSounds();

	// Force all instruments to reload on the next playing point
	for (int i = 0; i < 5; ++i) {
		adjustVolume(i, _channelsVolumeTable[i].original);
		AdLibSoundInstrument *ins = &_instrumentsTable[i];
		setupInstrument(ins, i);
	}
}

void AdLibSoundDriver::adjustVolume(int channel, int volume) {
	_channelsVolumeTable[channel].original = volume;

	if (volume > 80) {
		volume = 80;
	} else if (volume < 0) {
		volume = 0;
	}
	volume += volume / 4;
	if (volume > 127) {
		volume = 127;
	}

	int volAdjust = (channel == 4) ? _sfxVolume : _musicVolume;
	volume = (volume * volAdjust) / 128;

	if (volume > 127)
		volume = 127;

	_channelsVolumeTable[channel].adjusted = volume;
}

void AdLibSoundDriver::setupChannel(int channel, const byte *data, int instrument, int volume) {
	assert(channel < 5);
	if (data) {
		adjustVolume(channel, volume);
		setupInstrument(data, channel);
	}
}

void AdLibSoundDriver::stopChannel(int channel) {
	assert(channel < 5);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || channel == 6) {
		OPLWriteReg(_opl, 0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato &= ~(1 << (10 - ins->channel));
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

void AdLibSoundDriver::stopAll() {
	int i;
	for (i = 0; i < 18; ++i) {
		OPLWriteReg(_opl, 0x40 | _operatorsTable[i], 63);
	}
	for (i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xB0 | i, 0);
	}
	OPLWriteReg(_opl, 0xBD, 0);
}

int AdLibSoundDriver::readBuffer(int16 *buffer, const int numSamples) {
	update(buffer, numSamples);
	return numSamples;
}

void AdLibSoundDriver::initCard() {
	_vibrato = 0x20;
	OPLWriteReg(_opl, 0xBD, _vibrato);
	OPLWriteReg(_opl, 0x08, 0x40);

	static const int oplRegs[] = { 0x40, 0x60, 0x80, 0x20, 0xE0 };

	for (int i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xB0 | i, 0);
	}
	for (int i = 0; i < 9; ++i) {
		OPLWriteReg(_opl, 0xC0 | i, 0);
	}

	for (int j = 0; j < 5; j++) {
		for (int i = 0; i < 18; ++i) {
			OPLWriteReg(_opl, oplRegs[j] | _operatorsTable[i], 0);
		}
	}

	OPLWriteReg(_opl, 1, 0x20);
	OPLWriteReg(_opl, 1, 0);
}

void AdLibSoundDriver::update(int16 *buf, int len) {
	static int samplesLeft = 0;
	while (len != 0) {
		int count = samplesLeft;
		if (count > len) {
			count = len;
		}
		samplesLeft -= count;
		len -= count;
		YM3812UpdateOne(_opl, buf, count);
		if (samplesLeft == 0) {
			if (_upCb) {
				(*_upCb)(_upRef);
			}
			samplesLeft = _sampleRate / 50;
		}
		buf += count;
	}
}

void AdLibSoundDriver::setupInstrument(const byte *data, int channel) {
	assert(channel < 5);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	loadInstrument(data, ins);

	setupInstrument(ins, channel);
}

void AdLibSoundDriver::setupInstrument(const AdLibSoundInstrument *ins, int channel) {
	int mod, car, tmp;
	const AdLibRegisterSoundInstrument *reg;

	if (ins->mode != 0)  {
		mod = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * ins->channel + 1]];
	} else {
		mod = _operatorsTable[_voiceOperatorsTable[2 * channel + 0]];
		car = _operatorsTable[_voiceOperatorsTable[2 * channel + 1]];
	}

	if (ins->mode == 0 || ins->channel == 6) {
		reg = &ins->regMod;
		OPLWriteReg(_opl, 0x20 | mod, reg->vibrato);
		if (reg->freqMod) {
			tmp = reg->outputLevel & 0x3F;
		} else {
			tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel].adjusted;
			tmp = 63 - (2 * tmp + 127) / (2 * 127);
		}
		OPLWriteReg(_opl, 0x40 | mod, tmp | (reg->keyScaling << 6));
		OPLWriteReg(_opl, 0x60 | mod, reg->attackDecay);
		OPLWriteReg(_opl, 0x80 | mod, reg->sustainRelease);
		if (ins->mode != 0) {
			OPLWriteReg(_opl, 0xC0 | ins->channel, reg->feedbackStrength);
		} else {
			OPLWriteReg(_opl, 0xC0 | channel, reg->feedbackStrength);
		}
		OPLWriteReg(_opl, 0xE0 | mod, ins->waveSelectMod);
	}

	reg = &ins->regCar;
	OPLWriteReg(_opl, 0x20 | car, reg->vibrato);
	tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel].adjusted;
	tmp = 63 - (2 * tmp + 127) / (2 * 127);
	OPLWriteReg(_opl, 0x40 | car, tmp | (reg->keyScaling << 6));
	OPLWriteReg(_opl, 0x60 | car, reg->attackDecay);
	OPLWriteReg(_opl, 0x80 | car, reg->sustainRelease);
	OPLWriteReg(_opl, 0xE0 | car, ins->waveSelectCar);
}

void AdLibSoundDriver::loadRegisterInstrument(const byte *data, AdLibRegisterSoundInstrument *reg) {
	reg->vibrato = 0;
	if (READ_LE_UINT16(data + 18)) { // amplitude vibrato
		reg->vibrato |= 0x80;
	}
	if (READ_LE_UINT16(data + 20)) { // frequency vibrato
		reg->vibrato |= 0x40;
	}
	if (READ_LE_UINT16(data + 10)) { // sustaining sound
		reg->vibrato |= 0x20;
	}
	if (READ_LE_UINT16(data + 22)) { // envelope scaling
		reg->vibrato |= 0x10;
	}
	reg->vibrato |= READ_LE_UINT16(data + 2) & 0xF; // frequency multiplier

	reg->attackDecay = READ_LE_UINT16(data + 6) << 4; // attack rate
	reg->attackDecay |= READ_LE_UINT16(data + 12) & 0xF; // decay rate

	reg->sustainRelease = READ_LE_UINT16(data + 8) << 4; // sustain level
	reg->sustainRelease |= READ_LE_UINT16(data + 14) & 0xF; // release rate

	reg->feedbackStrength = READ_LE_UINT16(data + 4) << 1; // feedback
	if (READ_LE_UINT16(data + 24) == 0) { // frequency modulation
		reg->feedbackStrength |= 1;
	}

	reg->keyScaling = READ_LE_UINT16(data);
	reg->outputLevel = READ_LE_UINT16(data + 16);
	reg->freqMod = READ_LE_UINT16(data + 24);
}

void AdLibSoundDriverADL::loadInstrument(const byte *data, AdLibSoundInstrument *asi) {
	asi->mode = *data++;
	asi->channel = *data++;
	asi->waveSelectMod = *data++ & 3;
	asi->waveSelectCar = *data++ & 3;
	asi->amDepth = *data++;
	++data;
	loadRegisterInstrument(data, &asi->regMod); data += 26;
	loadRegisterInstrument(data, &asi->regCar); data += 26;
}

void AdLibSoundDriverADL::setChannelFrequency(int channel, int frequency) {
	assert(channel < 5);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0) {
		channel = ins->channel;
		if (channel == 9) {
			channel = 8;
		} else if (channel == 10) {
			channel = 7;
		}
	}
	int freq, note, oct;
	findNote(frequency, &note, &oct);

	note += oct * 12;
	if (ins->amDepth) {
		note = ins->amDepth;
	}
	if (note < 0) {
		note = 0;
	}

	freq = _freqTable[note % 12];
	OPLWriteReg(_opl, 0xA0 | channel, freq);
	freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
	if (ins->mode == 0) {
		freq |= 0x20;
	}
	OPLWriteReg(_opl, 0xB0 | channel, freq);
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

void AdLibSoundDriverADL::playSample(const byte *data, int size, int channel, int volume) {
	assert(channel < 5);
	adjustVolume(channel, 127);

	setupInstrument(data, channel);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		OPLWriteReg(_opl, 0xB0 | channel, 0);
	}
	if (ins->mode != 0) {
		_vibrato &= ~(1 << (10 - ins->channel));
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
	if (ins->mode != 0) {
		channel = ins->channel;
		if (channel == 9) {
			channel = 8;
		} else if (channel == 10) {
			channel = 7;
		}
	}
	uint16 note = 48;
	if (ins->amDepth) {
		note = ins->amDepth;
	}
	int freq = _freqTable[note % 12];
	OPLWriteReg(_opl, 0xA0 | channel, freq);
	freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
	if (ins->mode == 0) {
		freq |= 0x20;
	}
	OPLWriteReg(_opl, 0xB0 | channel, freq);
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

PCSoundFxPlayer::PCSoundFxPlayer(PCSoundDriver *driver)
	: _playing(false), _songPlayed(false), _driver(driver) {
	memset(_instrumentsData, 0, sizeof(_instrumentsData));
	_sfxData = NULL;
	_fadeOutCounter = 0;
	_driver->setUpdateCallback(updateCallback, this);
}

PCSoundFxPlayer::~PCSoundFxPlayer() {
	_driver->setUpdateCallback(NULL, NULL);
	stop();
}

bool PCSoundFxPlayer::load(const char *song) {
	debug(9, "PCSoundFxPlayer::load('%s')", song);

	/* stop (w/ fade out) the previous song */
	while (_fadeOutCounter != 0 && _fadeOutCounter < 100) {
		g_system->delayMillis(50);
	}
	_fadeOutCounter = 0;

	if (_playing) {
		stop();
	}

	strcpy(_musicName, song);
	_songPlayed = false;
	_looping = false;
	_sfxData = readBundleSoundFile(song);
	if (!_sfxData) {
		warning("Unable to load soundfx module '%s'", song);
		return 0;
	}

	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		_instrumentsData[i] = NULL;

		char instrument[64];
		memset(instrument, 0, 64); // Clear the data first
		memcpy(instrument, _sfxData + 20 + i * 30, 12);
		instrument[63] = '\0';

		if (strlen(instrument) != 0) {
			char *dot = strrchr(instrument, '.');
			if (dot) {
				*dot = '\0';
			}
			strcat(instrument, _driver->getInstrumentExtension());
			_instrumentsData[i] = readBundleSoundFile(instrument);
			if (!_instrumentsData[i]) {
				warning("Unable to load soundfx instrument '%s'", instrument);
			}
		}
	}
	return 1;
}

void PCSoundFxPlayer::play() {
	debug(9, "PCSoundFxPlayer::play()");
	if (_sfxData) {
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_instrumentsChannelTable[i] = -1;
		}
		_currentPos = 0;
		_currentOrder = 0;
		_numOrders = _sfxData[470];
		_eventsDelay = (244 - _sfxData[471]) * 100 / 1060;
		_updateTicksCounter = 0;
		_playing = true;
	}
}

void PCSoundFxPlayer::stop() {
	if (_playing || _fadeOutCounter != 0) {
		_fadeOutCounter = 0;
		_playing = false;
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_driver->stopChannel(i);
		}
		_driver->stopAll();
	}
	unload();
}

void PCSoundFxPlayer::fadeOut() {
	if (_playing) {
		_fadeOutCounter = 1;
		_playing = false;
	}
}

void PCSoundFxPlayer::updateCallback(void *ref) {
	((PCSoundFxPlayer *)ref)->update();
}

void PCSoundFxPlayer::update() {
	if (_playing || (_fadeOutCounter != 0 && _fadeOutCounter < 100)) {
		++_updateTicksCounter;
		if (_updateTicksCounter > _eventsDelay) {
			handleEvents();
			_updateTicksCounter = 0;
		}
	}
}

void PCSoundFxPlayer::handleEvents() {
	const byte *patternData = _sfxData + 600 + 1800;
	const byte *orderTable = _sfxData + 472;
	uint16 patternNum = orderTable[_currentOrder] * 1024;

	for (int i = 0; i < 4; ++i) {
		handlePattern(i, patternData + patternNum + _currentPos);
		patternData += 4;
	}

	if (_fadeOutCounter != 0 && _fadeOutCounter < 100) {
		_fadeOutCounter += 2;
	}
	if (_fadeOutCounter >= 100) {
		stop();
		return;
	}

	_currentPos += 16;
	if (_currentPos >= 1024) {
		_currentPos = 0;
		++_currentOrder;
		if (_currentOrder == _numOrders) {
			_currentOrder = 0;
		}
	}
	debug(7, "_currentOrder=%d/%d _currentPos=%d", _currentOrder, _numOrders, _currentPos);
}

void PCSoundFxPlayer::handlePattern(int channel, const byte *patternData) {
	int instrument = patternData[2] >> 4;
	if (instrument != 0) {
		--instrument;
		if (_instrumentsChannelTable[channel] != instrument || _fadeOutCounter != 0) {
			_instrumentsChannelTable[channel] = instrument;
			const int volume = _sfxData[instrument] - _fadeOutCounter;
			_driver->setupChannel(channel, _instrumentsData[instrument], instrument, volume);
		}
	}
	int16 freq = (int16)READ_BE_UINT16(patternData);
	if (freq > 0) {
		_driver->stopChannel(channel);
		_driver->setChannelFrequency(channel, freq);
	}
}

void PCSoundFxPlayer::unload() {
	for (int i = 0; i < NUM_INSTRUMENTS; ++i) {
		MemFree(_instrumentsData[i]);
		_instrumentsData[i] = NULL;
	}
	MemFree(_sfxData);
	_sfxData = NULL;
	_songPlayed = true;
}

void PCSoundFxPlayer::doSync(Common::Serializer &s) {
	s.syncBytes((byte *)_musicName, 33);
	uint16 v = (uint16)songLoaded();
	s.syncAsSint16LE(v);

	if (s.isLoading() && v) {
		load(_musicName);

		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_instrumentsChannelTable[i] = -1;
		}

		_numOrders = _sfxData[470];
		_eventsDelay = (244 - _sfxData[471]) * 100 / 1060;
		_updateTicksCounter = 0;
	}

	s.syncAsSint16LE(_songPlayed);
	s.syncAsSint16LE(_looping);
	s.syncAsSint16LE(_currentPos);
	s.syncAsSint16LE(_currentOrder);
	s.syncAsSint16LE(_playing);
}

PCSound::PCSound(Audio::Mixer *mixer, CruiseEngine *vm) {
	_vm = vm;
	_mixer = mixer;
	_soundDriver = new AdLibSoundDriverADL(_mixer);
	_player = new PCSoundFxPlayer(_soundDriver);
}

PCSound::~PCSound() {
	delete _player;
	delete _soundDriver;
}

void PCSound::loadMusic(const char *name) {
	debugC(5, kCruiseDebugSound, "PCSound::loadMusic('%s')", name);
	_player->load(name);
}

void PCSound::playMusic() {
	debugC(5, kCruiseDebugSound, "PCSound::playMusic()");
	_player->play();
}

void PCSound::stopMusic() {
	debugC(5, kCruiseDebugSound, "PCSound::stopMusic()");
	_player->stop();
}

void PCSound::removeMusic() {
	debugC(5, kCruiseDebugSound, "PCSound::removeMusic()");
	_player->unload();
}

void PCSound::fadeOutMusic() {
	debugC(5, kCruiseDebugSound, "PCSound::fadeOutMusic()");
	_player->fadeOut();
}

void PCSound::playSound(const uint8 *data, int size, int volume) {
	debugC(5, kCruiseDebugSound, "PCSound::playSound() channel %d size %d", 4, size);
	_soundDriver->playSample(data, size, 4, volume);
}

void PCSound::stopSound(int channel) {
	debugC(5, kCruiseDebugSound, "PCSound::stopSound() channel %d", channel);
	_soundDriver->resetChannel(channel);
}

void PCSound::stopChannel(int channel) {
	debugC(5, kCruiseDebugSound, "PCSound::stopChannel() channel %d", channel);
	_soundDriver->stopChannel(channel);
}

bool PCSound::isPlaying() const {
	return _player->playing();
}

bool PCSound::songLoaded() const {
	return _player->songLoaded();
}

bool PCSound::songPlayed() const {
	return _player->songPlayed();
}

void PCSound::fadeSong() {
	_player->fadeOut();
}

uint8 PCSound::numOrders() const {
	return _player->numOrders();
}

void PCSound::setNumOrders(uint8 v) {
	_player->setNumOrders(v);
}

void PCSound::setPattern(int offset, uint8 value) {
	_player->setPattern(offset, value);
}

bool PCSound::musicLooping() const {
	return _player->looping();
}

void PCSound::musicLoop(bool v) {
	_player->setLooping(v);
}

void PCSound::startNote(int channel, int volume, int freq) {
	warning("TODO: startNote");
//	_soundDriver->setVolume(channel, volume);
	_soundDriver->setChannelFrequency(channel, freq);
}

void PCSound::doSync(Common::Serializer &s) {
	_player->doSync(s);
	s.syncAsSint16LE(_genVolume);
}

const char *PCSound::musicName() {
	return _player->musicName();
}

void PCSound::syncSounds() {
	_soundDriver->syncSounds();
}

} // End of namespace Cruise
