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
#include "common/file.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/timer.h"
#include "common/mutex.h"
#include "common/config-manager.h"

#include "cine/cine.h"
#include "cine/sound.h"

#include "audio/audiostream.h"
#include "audio/fmopl.h"
#include "audio/mididrv.h"
#include "audio/decoders/raw.h"
#include "audio/mods/soundfx.h"

namespace Cine {

class PCSoundDriver {
public:
	typedef void (*UpdateCallback)(void *);

	virtual ~PCSoundDriver() {}

	virtual void setupChannel(int channel, const byte *data, int instrument, int volume) = 0;
	virtual void setChannelFrequency(int channel, int frequency) = 0;
	virtual void stopChannel(int channel) = 0;
	virtual void playSample(const byte *data, int size, int channel, int volume) = 0;
	virtual void stopAll() = 0;
	virtual const char *getInstrumentExtension() const { return ""; }
	virtual void notifyInstrumentLoad(const byte *data, int size, int channel) {}

	virtual void setUpdateCallback(UpdateCallback upCb, void *ref) = 0;
	void resetChannel(int channel);
	void findNote(int freq, int *note, int *oct) const;

protected:

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

class AdLibSoundDriver : public PCSoundDriver, Audio::AudioStream {
public:
	AdLibSoundDriver(Audio::Mixer *mixer);
	virtual ~AdLibSoundDriver();

	// PCSoundDriver interface
	virtual void setUpdateCallback(UpdateCallback upCb, void *ref);
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
	void loadRegisterInstrument(const byte *data, AdLibRegisterSoundInstrument *reg);
	virtual void loadInstrument(const byte *data, AdLibSoundInstrument *asi) = 0;

protected:
	UpdateCallback _upCb;
	void *_upRef;

	FM_OPL *_opl;
	int _sampleRate;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;

	byte _vibrato;
	int _channelsVolumeTable[4];
	AdLibSoundInstrument _instrumentsTable[4];

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
	0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19, 20, 21
};

const int AdLibSoundDriver::_operatorsTableCount = ARRAYSIZE(_operatorsTable);

const int AdLibSoundDriver::_voiceOperatorsTable[] = {
	0, 3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 16, 16, 14, 14, 17, 17, 13, 13
};

const int AdLibSoundDriver::_voiceOperatorsTableCount = ARRAYSIZE(_voiceOperatorsTable);

// Future Wars AdLib driver
class AdLibSoundDriverINS : public AdLibSoundDriver {
public:
	AdLibSoundDriverINS(Audio::Mixer *mixer) : AdLibSoundDriver(mixer) {}
	virtual const char *getInstrumentExtension() const { return ".INS"; }
	virtual void loadInstrument(const byte *data, AdLibSoundInstrument *asi);
	virtual void setChannelFrequency(int channel, int frequency);
	virtual void playSample(const byte *data, int size, int channel, int volume);
};

// Operation Stealth AdLib driver
class AdLibSoundDriverADL : public AdLibSoundDriver {
public:
	AdLibSoundDriverADL(Audio::Mixer *mixer) : AdLibSoundDriver(mixer) {}
	virtual const char *getInstrumentExtension() const { return ".ADL"; }
	virtual void loadInstrument(const byte *data, AdLibSoundInstrument *asi);
	virtual void setChannelFrequency(int channel, int frequency);
	virtual void playSample(const byte *data, int size, int channel, int volume);
};

// (Future Wars) MIDI driver
class MidiSoundDriverH32 : public PCSoundDriver {
public:
	MidiSoundDriverH32(MidiDriver *output);
	~MidiSoundDriverH32();

	virtual void setUpdateCallback(UpdateCallback upCb, void *ref);
	virtual void setupChannel(int channel, const byte *data, int instrument, int volume);
	virtual void setChannelFrequency(int channel, int frequency);
	virtual void stopChannel(int channel);
	virtual void playSample(const byte *data, int size, int channel, int volume);
	virtual void stopAll() {}
	virtual const char *getInstrumentExtension() const { return ".H32"; }
	virtual void notifyInstrumentLoad(const byte *data, int size, int channel);

private:
	MidiDriver *_output;
	UpdateCallback _callback;
	Common::Mutex _mutex;

	void writeInstrument(int offset, const byte *data, int size);
	void selectInstrument(int channel, int timbreGroup, int timbreNumber, int volume);
};

class PCSoundFxPlayer {
public:

	PCSoundFxPlayer(PCSoundDriver *driver);
	~PCSoundFxPlayer();

	bool load(const char *song);
	void play();
	void stop();
	void fadeOut();

	static void updateCallback(void *ref);

	enum {
		NUM_INSTRUMENTS = 15,
		NUM_CHANNELS = 4
	};

private:

	void update();
	void handleEvents();
	void handlePattern(int channel, const byte *patternData);
	void unload();

	bool _playing;
	int _currentPos;
	int _currentOrder;
	int _numOrders;
	int _eventsDelay;
	int _fadeOutCounter;
	int _updateTicksCounter;
	int _instrumentsChannelTable[NUM_CHANNELS];
	byte *_sfxData;
	byte *_instrumentsData[NUM_INSTRUMENTS];
	PCSoundDriver *_driver;
	Common::Mutex _mutex;
};


void PCSoundDriver::findNote(int freq, int *note, int *oct) const {
	if (freq > 0x777)
		*oct = 0;
	else if (freq > 0x3BB)
		*oct = 1;
	else if (freq > 0x1DD)
		*oct = 2;
	else if (freq > 0x0EE)
		*oct = 3;
	else if (freq > 0x077)
		*oct = 4;
	else if (freq > 0x03B)
		*oct = 5;
	else if (freq > 0x01D)
		*oct = 6;
	else
		*oct = 7;

	*note = 11;
	for (int i = 0; i < 12; ++i) {
		if (_noteTable[*oct * 12 + i] <= freq) {
			*note = i;
			break;
		}
	}
}

void PCSoundDriver::resetChannel(int channel) {
	stopChannel(channel);
	stopAll();
}

AdLibSoundDriver::AdLibSoundDriver(Audio::Mixer *mixer)
	: _mixer(mixer) {
	_sampleRate = _mixer->getOutputRate();
	_opl = makeAdLibOPL(_sampleRate);
	memset(_channelsVolumeTable, 0, sizeof(_channelsVolumeTable));
	memset(_instrumentsTable, 0, sizeof(_instrumentsTable));
	initCard();
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

AdLibSoundDriver::~AdLibSoundDriver() {
	_mixer->stopHandle(_soundHandle);
	OPLDestroy(_opl);
}

void AdLibSoundDriver::setUpdateCallback(UpdateCallback upCb, void *ref) {
	_upCb = upCb;
	_upRef = ref;
}

void AdLibSoundDriver::setupChannel(int channel, const byte *data, int instrument, int volume) {
	assert(channel < 4);
	if (data) {
		if (volume > 80) {
			volume = 80;
		} else if (volume < 0) {
			volume = 0;
		}
		volume += volume / 4;
		if (volume > 127) {
			volume = 127;
		}
		_channelsVolumeTable[channel] = volume;
		setupInstrument(data, channel);
	}
}

void AdLibSoundDriver::stopChannel(int channel) {
	assert(channel < 4);
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
	assert(channel < 4);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	loadInstrument(data, ins);

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
			tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel];
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
	tmp = (63 - (reg->outputLevel & 0x3F)) * _channelsVolumeTable[channel];
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

void AdLibSoundDriverINS::loadInstrument(const byte *data, AdLibSoundInstrument *asi) {
	asi->mode = *data++;
	asi->channel = *data++;
	loadRegisterInstrument(data, &asi->regMod); data += 26;
	loadRegisterInstrument(data, &asi->regCar); data += 26;
	asi->waveSelectMod = data[0] & 3; data += 2;
	asi->waveSelectCar = data[0] & 3; data += 2;
	asi->amDepth = data[0]; data += 2;
}

void AdLibSoundDriverINS::setChannelFrequency(int channel, int frequency) {
	assert(channel < 4);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || ins->channel == 6) {
		int freq, note, oct;
		findNote(frequency, &note, &oct);
		if (channel == 6)
			oct = 0;
		freq = _freqTable[note % 12];
		OPLWriteReg(_opl, 0xA0 | channel, freq);
		freq = (oct << 2) | ((freq & 0x300) >> 8);
		if (ins->mode == 0) {
			freq |= 0x20;
		}
		OPLWriteReg(_opl, 0xB0 | channel, freq);
	}
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - ins->channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
}

void AdLibSoundDriverINS::playSample(const byte *data, int size, int channel, int volume) {
	assert(channel < 4);
	_channelsVolumeTable[channel] = 127;
	resetChannel(channel);
	setupInstrument(data + 257, channel);
	AdLibSoundInstrument *ins = &_instrumentsTable[channel];
	if (ins->mode != 0 && ins->channel == 6) {
		channel = 6;
	}
	if (ins->mode == 0 || channel == 6) {
		uint16 note = 12;
		int freq = _freqTable[note % 12];
		OPLWriteReg(_opl, 0xA0 | channel, freq);
		freq = ((note / 12) << 2) | ((freq & 0x300) >> 8);
		if (ins->mode == 0) {
			freq |= 0x20;
		}
		OPLWriteReg(_opl, 0xB0 | channel, freq);
	}
	if (ins->mode != 0) {
		_vibrato |= 1 << (10 - ins->channel);
		OPLWriteReg(_opl, 0xBD, _vibrato);
	}
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
	assert(channel < 4);
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
	if (ins->amDepth) {
		note = ins->amDepth;
		oct = note / 12;
	}
	if (note < 0) {
		note = 0;
		oct = 0;
	}

	freq = _freqTable[note % 12];
	OPLWriteReg(_opl, 0xA0 | channel, freq);
	freq = (oct << 2) | ((freq & 0x300) >> 8);
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
	assert(channel < 4);
	_channelsVolumeTable[channel] = 127;
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

MidiSoundDriverH32::MidiSoundDriverH32(MidiDriver *output)
	: _output(output), _callback(0), _mutex() {
}

MidiSoundDriverH32::~MidiSoundDriverH32() {
	if (_callback)
		g_system->getTimerManager()->removeTimerProc(_callback);

	_output->close();
	delete _output;
}

void MidiSoundDriverH32::setUpdateCallback(UpdateCallback upCb, void *ref) {
	Common::StackLock lock(_mutex);

	Common::TimerManager *timer = g_system->getTimerManager();
	assert(timer);

	if (_callback)
		timer->removeTimerProc(_callback);

	_callback = upCb;
	if (_callback)
		timer->installTimerProc(_callback, 1000000 / 50, ref, "MidiSoundDriverH32");
}

void MidiSoundDriverH32::setupChannel(int channel, const byte *data, int instrument, int volume) {
	Common::StackLock lock(_mutex);

	if (volume < 0 ||  volume > 100)
		volume = 0;

	if (!data)
		selectInstrument(channel, 0, 0, volume);
	// In case the instrument is a builtin instrument select it directly.
	else if (data[0] < 0x80)
		selectInstrument(channel, data[0] / 0x40, data[0] % 0x40, volume);
	// In case we use a custom instrument we need to specify the timbre group
	// 2, which means it's a timbre from the timbre memory area.
	else
		selectInstrument(channel, 2, instrument, volume);
}

void MidiSoundDriverH32::setChannelFrequency(int channel, int frequency) {
	Common::StackLock lock(_mutex);

	int note, oct;
	findNote(frequency, &note, &oct);
	note %= 12;
	note = oct * 12 + note + 12;

	_output->send(0x91 + channel, note, 0x7F);
}

void MidiSoundDriverH32::stopChannel(int channel) {
	Common::StackLock lock(_mutex);

	_output->send(0xB1 + channel, 0x7B, 0x00);
}

void MidiSoundDriverH32::playSample(const byte *data, int size, int channel, int volume) {
	Common::StackLock lock(_mutex);

	stopChannel(channel);

	volume = volume * 8 / 5;

	if (data[0] < 0x80) {
		selectInstrument(channel, data[0] / 0x40, data[0] % 0x40, volume);
	} else {
		writeInstrument(channel * 512 + 0x80000, data + 1, 256);
		selectInstrument(channel, 2, channel, volume);
	}

	_output->send(0x91 + channel, 12, 0x7F);
}

void MidiSoundDriverH32::notifyInstrumentLoad(const byte *data, int size, int channel) {
	Common::StackLock lock(_mutex);

	// In case we specify a standard instrument or standard rhythm instrument
	// do not do anything here. It might be noteworthy that the instrument
	// selection client code does not support rhythm instruments!
	if (data[0] < 0x80 || data[0] > 0xC0)
		return;

	writeInstrument(channel * 512 + 0x80000, data + 1, size - 1);
}

void MidiSoundDriverH32::writeInstrument(int offset, const byte *data, int size) {
	byte sysEx[254];

	sysEx[0] = 0x41;
	sysEx[1] = 0x10;
	sysEx[2] = 0x16;
	sysEx[3] = 0x12;
	sysEx[4] = (offset >> 16) & 0xFF;
	sysEx[5] = (offset >>  8) & 0xFF;
	sysEx[6] = (offset >>  0) & 0xFF;
	int copySize = MIN(246, size);
	memcpy(&sysEx[7], data, copySize);

	byte checkSum = 0;
	for (int i = 0; i < copySize + 3; ++i)
		checkSum += sysEx[4 + i];
	sysEx[7 + copySize] = 0x80 - (checkSum & 0x7F);

	_output->sysEx(sysEx, copySize + 8);
}

void MidiSoundDriverH32::selectInstrument(int channel, int timbreGroup, int timbreNumber, int volume) {
	const int offset = channel * 16 + 0x30000; // 0x30000 is the start of the patch temp area

	byte sysEx[24] = {
		0x41, 0x10, 0x16, 0x12,
		0x00, 0x00, 0x00,       // offset
		0x00, // Timbre group   _ timbreGroup * 64 + timbreNumber should be the
		0x00, // Timbre number /  MT-32 instrument in case timbreGroup is 0 or 1.
		0x18, // Key shift (= 0)
		0x32, // Fine tune (= 0)
		0x0C, // Bender Range
		0x03, // Assign Mode
		0x01, // Reverb Switch (= enabled)
		0x00, // dummy
		0x00, // Output level
		0x07, // Panpot (= balanced)
		0x00, // dummy
		0x00, // dummy
		0x00, // dummy
		0x00, // dummy
		0x00, // dummy
		0x00, // dummy
		0x00  // checksum
	};


	sysEx[4] = (offset >> 16) & 0xFF;
	sysEx[5] = (offset >>  8) & 0xFF;
	sysEx[6] = (offset >>  0) & 0xFF;

	sysEx[7] = timbreGroup;
	sysEx[8] = timbreNumber;

	sysEx[15] = volume;

	byte checkSum = 0;

	for (int i = 4; i < 23; ++i)
		checkSum += sysEx[i];

	sysEx[23] = 0x80 - (checkSum & 0x7F);

	_output->sysEx(sysEx, 24);
}

PCSoundFxPlayer::PCSoundFxPlayer(PCSoundDriver *driver)
	: _playing(false), _driver(driver), _mutex() {
	memset(_instrumentsData, 0, sizeof(_instrumentsData));
	_sfxData = NULL;
	_fadeOutCounter = 0;
	_driver->setUpdateCallback(updateCallback, this);
}

PCSoundFxPlayer::~PCSoundFxPlayer() {
	Common::StackLock lock(_mutex);

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

	Common::StackLock lock(_mutex);

	stop();

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

		if (instrument[0] != '\0') {
			char *dot = strrchr(instrument, '.');
			if (dot) {
				*dot = '\0';
			}
			strcat(instrument, _driver->getInstrumentExtension());
			uint32 instrumentSize;
			_instrumentsData[i] = readBundleSoundFile(instrument, &instrumentSize);
			if (!_instrumentsData[i]) {
				warning("Unable to load soundfx instrument '%s'", instrument);
			} else {
				_driver->notifyInstrumentLoad(_instrumentsData[i], instrumentSize, i);
			}
		}
	}
	return 1;
}

void PCSoundFxPlayer::play() {
	debug(9, "PCSoundFxPlayer::play()");
	Common::StackLock lock(_mutex);
	if (_sfxData) {
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			_instrumentsChannelTable[i] = -1;
		}
		_currentPos = 0;
		_currentOrder = 0;
		_numOrders = _sfxData[470];
		_eventsDelay = (252 - _sfxData[471]) * 50 / 1060;
		_updateTicksCounter = 0;
		_playing = true;
	}
}

void PCSoundFxPlayer::stop() {
	Common::StackLock lock(_mutex);
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
	Common::StackLock lock(_mutex);
	if (_playing) {
		_fadeOutCounter = 1;
		_playing = false;
	}
}

void PCSoundFxPlayer::updateCallback(void *ref) {
	((PCSoundFxPlayer *)ref)->update();
}

void PCSoundFxPlayer::update() {
	Common::StackLock lock(_mutex);
	if (_playing || (_fadeOutCounter != 0 && _fadeOutCounter < 100)) {
		++_updateTicksCounter;
		if (_updateTicksCounter > _eventsDelay) {
			handleEvents();
			_updateTicksCounter = 0;
		}
	}
}

void PCSoundFxPlayer::handleEvents() {
	const byte *patternData = _sfxData + 600;
	const byte *orderTable = _sfxData + 472;
	uint16 patternNum = orderTable[_currentOrder] * 1024;

	for (int i = 0; i < 4; ++i) {
		handlePattern(i, patternData + patternNum + _currentPos);
		patternData += 4;
	}

	if (_fadeOutCounter != 0 && _fadeOutCounter < 100) {
		_fadeOutCounter += 2;
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
		free(_instrumentsData[i]);
		_instrumentsData[i] = NULL;
	}
	free(_sfxData);
	_sfxData = NULL;
}


PCSound::PCSound(Audio::Mixer *mixer, CineEngine *vm)
	: Sound(mixer, vm), _soundDriver(0) {

	const MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB);
	const MusicType musicType = MidiDriver::getMusicType(dev);
	if (musicType == MT_MT32 || musicType == MT_GM) {
		const bool isMT32 = (musicType == MT_MT32 || ConfMan.getBool("native_mt32"));
		if (isMT32) {
			MidiDriver *driver = MidiDriver::createMidi(dev);
			if (driver && driver->open() == 0) {
				driver->sendMT32Reset();
				_soundDriver = new MidiSoundDriverH32(driver);
			} else {
				warning("Could not create MIDI output, falling back to AdLib");
			}
		} else {
			warning("General MIDI output devices are not supported, falling back to AdLib");
		}
	}

	if (!_soundDriver) {
		if (_vm->getGameType() == GType_FW) {
			_soundDriver = new AdLibSoundDriverINS(_mixer);
		} else {
			_soundDriver = new AdLibSoundDriverADL(_mixer);
		}
	}

	_player = new PCSoundFxPlayer(_soundDriver);
}

PCSound::~PCSound() {
	delete _player;
	delete _soundDriver;
}

void PCSound::loadMusic(const char *name) {
	debugC(5, kCineDebugSound, "PCSound::loadMusic('%s')", name);
	_player->load(name);
}

void PCSound::playMusic() {
	debugC(5, kCineDebugSound, "PCSound::playMusic()");
	_player->play();
}

void PCSound::stopMusic() {
	debugC(5, kCineDebugSound, "PCSound::stopMusic()");
	_player->stop();
}

void PCSound::fadeOutMusic() {
	debugC(5, kCineDebugSound, "PCSound::fadeOutMusic()");
	_player->fadeOut();
}

void PCSound::playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat) {
	debugC(5, kCineDebugSound, "PCSound::playSound() channel %d size %d", channel, size);
	_soundDriver->playSample(data, size, channel, volume);
}

void PCSound::stopSound(int channel) {
	debugC(5, kCineDebugSound, "PCSound::stopSound() channel %d", channel);
	_soundDriver->resetChannel(channel);
}

PaulaSound::PaulaSound(Audio::Mixer *mixer, CineEngine *vm)
	: Sound(mixer, vm), _sfxTimer(0), _musicTimer(0), _musicFadeTimer(0) {
	_moduleStream = 0;
	// The original is using the following timer frequency:
	// 0.709379Mhz / 8000 = 88.672375Hz
	// 1000000 / 88.672375Hz = 11277.46944863us
	g_system->getTimerManager()->installTimerProc(&PaulaSound::sfxTimerProc, 11277, this, "PaulaSound::sfxTimerProc");
	// The original is using the following timer frequency:
	// 0.709379Mhz / 14565 = 48.704359Hz
	// 1000000 / 48.704359Hz = 20532.04313806us
	g_system->getTimerManager()->installTimerProc(&PaulaSound::musicTimerProc, 20532, this, "PaulaSound::musicTimerProc");
}

PaulaSound::~PaulaSound() {
	Common::StackLock sfxLock(_sfxMutex);
	g_system->getTimerManager()->removeTimerProc(&PaulaSound::sfxTimerProc);
	for (int i = 0; i < NUM_CHANNELS; ++i) {
		stopSound(i);
	}

	Common::StackLock musicLock(_musicMutex);
	g_system->getTimerManager()->removeTimerProc(&PaulaSound::musicTimerProc);
	stopMusic();
}

void PaulaSound::loadMusic(const char *name) {
	debugC(5, kCineDebugSound, "PaulaSound::loadMusic('%s')", name);
	for (int i = 0; i < NUM_CHANNELS; ++i) {
		stopSound(i);
	}

	// Fade music out when there is music playing.
	_musicMutex.lock();
	if (_mixer->isSoundHandleActive(_moduleHandle)) {
		// Only start fade out when it is not in progress.
		if (!_musicFadeTimer) {
			_musicFadeTimer = 1;
		}

		_musicMutex.unlock();
		while (_musicFadeTimer != 64) {
			g_system->delayMillis(50);
		}
	} else {
		_musicMutex.unlock();
	}

	Common::StackLock lock(_musicMutex);
	assert(!_mixer->isSoundHandleActive(_moduleHandle));

	if (_vm->getGameType() == GType_FW) {
		// look for separate files
		Common::File f;
		if (f.open(name)) {
			_moduleStream = Audio::makeSoundFxStream(&f, 0, _mixer->getOutputRate());
		}
	} else {
		// look in bundle files
		uint32 size;
		byte *buf = readBundleSoundFile(name, &size);
		if (buf) {
			Common::MemoryReadStream s(buf, size);
			_moduleStream = Audio::makeSoundFxStream(&s, readBundleSoundFile, _mixer->getOutputRate());
			free(buf);
		}
	}
}

void PaulaSound::playMusic() {
	debugC(5, kCineDebugSound, "PaulaSound::playMusic()");
	Common::StackLock lock(_musicMutex);

	_mixer->stopHandle(_moduleHandle);
	if (_moduleStream) {
		_musicFadeTimer = 0;
		_mixer->playStream(Audio::Mixer::kMusicSoundType, &_moduleHandle, _moduleStream);
	}
}

void PaulaSound::stopMusic() {
	debugC(5, kCineDebugSound, "PaulaSound::stopMusic()");
	Common::StackLock lock(_musicMutex);

	_mixer->stopHandle(_moduleHandle);
}

void PaulaSound::fadeOutMusic() {
	debugC(5, kCineDebugSound, "PaulaSound::fadeOutMusic()");
	Common::StackLock lock(_musicMutex);

	_musicFadeTimer = 1;
}

void PaulaSound::playSound(int channel, int frequency, const uint8 *data, int size, int volumeStep, int stepCount, int volume, int repeat) {
	debugC(5, kCineDebugSound, "PaulaSound::playSound() channel %d size %d", channel, size);
	Common::StackLock lock(_sfxMutex);
	assert(frequency > 0);

	stopSound(channel);
	if (size > 0) {
		byte *sound = (byte *)malloc(size);
		if (sound) {
			// Create the audio stream
			memcpy(sound, data, size);

			// Clear the first and last 16 bits like in the original.
			sound[0] = sound[1] = sound[size - 2] = sound[size - 1] = 0;

			Audio::SeekableAudioStream *stream = Audio::makeRawStream(sound, size, PAULA_FREQ / frequency, 0);

			// Initialize the volume control
			_channelsTable[channel].initialize(volume, volumeStep, stepCount);

			// Start the sfx
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channelsTable[channel].handle,
			                   Audio::makeLoopingAudioStream(stream, repeat ? 0 : 1),
			                   -1, volume * Audio::Mixer::kMaxChannelVolume / 63,
			                   _channelBalance[channel]);
		}
	}
}

void PaulaSound::stopSound(int channel) {
	debugC(5, kCineDebugSound, "PaulaSound::stopSound() channel %d", channel);
	Common::StackLock lock(_sfxMutex);

	_mixer->stopHandle(_channelsTable[channel].handle);
}

void PaulaSound::sfxTimerProc(void *param) {
	PaulaSound *sound = (PaulaSound *)param;
	sound->sfxTimerCallback();
}

void PaulaSound::sfxTimerCallback() {
	Common::StackLock lock(_sfxMutex);

	if (_sfxTimer < 6) {
		++_sfxTimer;

		for (int i = 0; i < NUM_CHANNELS; ++i) {
			// Only process active channels
			if (!_mixer->isSoundHandleActive(_channelsTable[i].handle)) {
				continue;
			}

			if (_channelsTable[i].curStep) {
				--_channelsTable[i].curStep;
			} else {
				_channelsTable[i].curStep = _channelsTable[i].stepCount;
				const int volume = CLIP(_channelsTable[i].volume + _channelsTable[i].volumeStep, 0, 63);
				_channelsTable[i].volume = volume;
				// Unlike the original we stop silent sounds
				if (volume) {
					_mixer->setChannelVolume(_channelsTable[i].handle, volume * Audio::Mixer::kMaxChannelVolume / 63);
				} else {
					_mixer->stopHandle(_channelsTable[i].handle);
				}
			}
		}
	} else {
		_sfxTimer = 0;
		// Possible TODO: The original only ever started sounds here. This
		// should not be noticable though. So we do not do it for now.
	}
}

void PaulaSound::musicTimerProc(void *param) {
	PaulaSound *sound = (PaulaSound *)param;
	sound->musicTimerCallback();
}

void PaulaSound::musicTimerCallback() {
	Common::StackLock lock(_musicMutex);

	++_musicTimer;
	if (_musicTimer == 6) {
		_musicTimer = 0;
		if (_musicFadeTimer) {
			++_musicFadeTimer;
			if (_musicFadeTimer == 64) {
				stopMusic();
			} else {
				if (_mixer->isSoundHandleActive(_moduleHandle)) {
					_mixer->setChannelVolume(_moduleHandle, (64 - _musicFadeTimer) * Audio::Mixer::kMaxChannelVolume / 64);
				}
			}
		}
	}
}

const int PaulaSound::_channelBalance[NUM_CHANNELS] = {
	// L/R/R/L This is according to the Hardware Reference Manual.
	// TODO: It seems the order is swapped for some Amiga models:
	// http://www.amiga.org/forums/archive/index.php/t-7862.html
	// Maybe we should consider using R/L/L/R to match Amiga 500?
	// This also is a bit more drastic to what WineUAE defaults,
	// which is only 70% of full panning.
	-127, 127, 127, -127
};

} // End of namespace Cine
