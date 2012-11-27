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
#include "common/textconsole.h"

#include "audio/fmopl.h"
#include "audio/softsynth/emumidi.h"

namespace Queen {

class AdLibMidiDriver : public MidiDriver_Emulated {
public:

	AdLibMidiDriver(Audio::Mixer *mixer) : MidiDriver_Emulated(mixer) { _adlibWaveformSelect = 0; }
	~AdLibMidiDriver() {}

	// MidiDriver
	int open();
	void close();
	void send(uint32 b);
	void metaEvent(byte type, byte *data, uint16 length);
	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }

	// AudioStream
	bool isStereo() const { return false; }
	int getRate() const { return _mixer->getOutputRate(); }

	// MidiDriver_Emulated
	void generateSamples(int16 *buf, int len);

private:

	void handleMidiEvent0x90_NoteOn(int channel, int param1, int param2);
	void handleSequencerSpecificMetaEvent1(int channel, const uint8 *data);
	void handleSequencerSpecificMetaEvent2(uint8 value);
	void handleSequencerSpecificMetaEvent3(uint8 value);

	void adlibWrite(uint8 port, uint8 value);
	void adlibSetupCard();
	void adlibSetupChannels(int fl);
	void adlibResetAmpVibratoRhythm(int am, int vib, int kso);
	void adlibResetChannels();
	void adlibSetAmpVibratoRhythm();
	void adlibSetCSMKeyboardSplit();
	void adlibSetNoteMul(int mul);
	void adlibSetWaveformSelect(int fl);
	void adlibSetPitchBend(int channel, int range);
	void adlibPlayNote(int channel);
	uint8 adlibPlayNoteHelper(int channel, int note1, int note2, int oct);
	void adlibTurnNoteOff(int channel);
	void adlibTurnNoteOn(int channel, int note);
	void adlibSetupChannelFromSequence(int channel, const uint8 *src, int fl);
	void adlibSetupChannel(int channel, const uint16 *src, int fl);
	void adlibSetNoteVolume(int channel, int volume);
	void adlibSetupChannelHelper(int channel);
	void adlibSetChannel0x40(int channel);
	void adlibSetChannel0xC0(int channel);
	void adlibSetChannel0x60(int channel);
	void adlibSetChannel0x80(int channel);
	void adlibSetChannel0x20(int channel);
	void adlibSetChannel0xE0(int channel);

	FM_OPL *_opl;
	int _midiNumberOfChannels;
	int _adlibNoteMul;
	int _adlibWaveformSelect;
	int _adlibAMDepthEq48;
	int _adlibVibratoDepthEq14;
	int _adlibRhythmEnabled;
	int _adlibKeyboardSplitOn;
	int _adlibVibratoRhythm;
	uint8 _midiChannelsFreqTable[9];
	uint8 _adlibChannelsLevelKeyScalingTable[11];
	uint8 _adlibSetupChannelSequence1[14 * 18];
	uint16 _adlibSetupChannelSequence2[14];
	int16 _midiChannelsNote2Table[9];
	uint8 _midiChannelsNote1Table[9];
	uint8 _midiChannelsOctTable[9];
	uint16 _adlibChannelsVolume[11];
	uint16 _adlibMetaSequenceData[28];

	static const uint8 _adlibChannelsMappingTable1[];
	static const uint8 _adlibChannelsNoFeedback[];
	static const uint8 _adlibChannelsMappingTable2[];
	static const uint8 _adlibChannelsMappingTable3[];
	static const uint8 _adlibChannelsKeyScalingTable1[];
	static const uint8 _adlibChannelsKeyScalingTable2[];
	static const uint8 _adlibChannelsVolumeTable[];
	static const uint8 _adlibInitSequenceData1[];
	static const uint8 _adlibInitSequenceData2[];
	static const uint8 _adlibInitSequenceData3[];
	static const uint8 _adlibInitSequenceData4[];
	static const uint8 _adlibInitSequenceData5[];
	static const uint8 _adlibInitSequenceData6[];
	static const uint8 _adlibInitSequenceData7[];
	static const uint8 _adlibInitSequenceData8[];
	static const int16 _midiChannelsNoteTable[];
	static const int16 _midiNoteFreqTable[];
};

int AdLibMidiDriver::open() {
	MidiDriver_Emulated::open();
	_opl = makeAdLibOPL(getRate());
	adlibSetupCard();
	for (int i = 0; i < 11; ++i) {
		_adlibChannelsVolume[i] = 0;
		adlibSetNoteVolume(i, 0);
		adlibTurnNoteOff(i);
	}
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_mixerSoundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
	return 0;
}

void AdLibMidiDriver::close() {
	_mixer->stopHandle(_mixerSoundHandle);
	OPLDestroy(_opl);
}

void AdLibMidiDriver::send(uint32 b) {
	int channel = b & 15;
	int cmd = (b >> 4) & 7;
	int param1 = (b >> 8) & 255;
	int param2 = (b >> 16) & 255;
	switch (cmd) {
	case 0:
		adlibTurnNoteOff(channel);
		break;
	case 1:
		handleMidiEvent0x90_NoteOn(channel, param1, param2);
		break;
	case 3:
		break;
	case 5:
		adlibSetNoteVolume(channel, param1);
		_adlibChannelsVolume[channel] = param1;
		break;
	case 6:
		adlibSetPitchBend(channel, param1 | (param2 << 7));
		break;
	default:
//		warning("Unhandled cmd %d channel %d (0x%X)", cmd, channel, b);
		break;
	}
}

void AdLibMidiDriver::metaEvent(byte type, byte *data, uint16 length) {
	int event = 0;
	if (length > 4 && READ_BE_UINT32(data) == 0x3F00) {
		event = data[4];
		switch (event) {
		case 1:
			if (length == 34) {
				handleSequencerSpecificMetaEvent1(data[5], data + 6);
				return;
			}
			break;
		case 2:
			if (length == 6) {
				handleSequencerSpecificMetaEvent2(data[5]);
				return;
			}
			break;
		case 3:
			if (length == 6) {
				handleSequencerSpecificMetaEvent3(data[5]);
				return;
			}
			break;
		}
	}
	warning("Unhandled meta event %d len %d", event, length);
}

void AdLibMidiDriver::generateSamples(int16 *data, int len) {
	memset(data, 0, sizeof(int16) * len);
	YM3812UpdateOne(_opl, data, len);
}

void AdLibMidiDriver::handleSequencerSpecificMetaEvent1(int channel, const uint8 *data) {
	for (int i = 0; i < 28; ++i) {
		_adlibMetaSequenceData[i] = data[i];
	}
	if (_midiNumberOfChannels > channel) {
		const uint8 *p;
		if (_adlibRhythmEnabled) {
			p = &_adlibChannelsKeyScalingTable2[channel * 2];
		} else {
			p = &_adlibChannelsKeyScalingTable1[channel * 2];
		}
		adlibSetupChannel(p[0], _adlibMetaSequenceData, _adlibMetaSequenceData[26]);
		if (p[1] != 255) {
			adlibSetupChannel(p[1], _adlibMetaSequenceData + 13, _adlibMetaSequenceData[27]);
		}
	}
}

void AdLibMidiDriver::handleSequencerSpecificMetaEvent2(uint8 value) {
	_adlibRhythmEnabled = value;
	_midiNumberOfChannels = _adlibRhythmEnabled ? 11 : 9;
	adlibSetAmpVibratoRhythm();
}

void AdLibMidiDriver::handleSequencerSpecificMetaEvent3(uint8 value) {
	adlibSetNoteMul(value);
}

void AdLibMidiDriver::handleMidiEvent0x90_NoteOn(int channel, int param1, int param2) { // note, volume
	if (param2 == 0) {
		adlibTurnNoteOff(channel);
		_adlibChannelsVolume[channel] = param2;
	} else {
		adlibSetNoteVolume(channel, param2);
		_adlibChannelsVolume[channel] = param2;
		adlibTurnNoteOff(channel);
		adlibTurnNoteOn(channel, param1);
	}
}

void AdLibMidiDriver::adlibWrite(uint8 port, uint8 value) {
	OPLWriteReg(_opl, port, value);
}

void AdLibMidiDriver::adlibSetupCard() {
	for (int i = 1; i <= 0xF5; ++i) {
		adlibWrite(i, 0);
	}
	adlibWrite(4, 6);
	for (int i = 0; i < 9; ++i) {
		_midiChannelsNote2Table[i] = 8192;
		_midiChannelsOctTable[i] = 0;
		_midiChannelsNote1Table[i] = 0;
		_midiChannelsFreqTable[i] = 0;
	}
	memset(_adlibChannelsLevelKeyScalingTable, 127, 11);
	adlibSetupChannels(0);
	adlibResetAmpVibratoRhythm(0, 0, 0);
	adlibSetNoteMul(1);
	adlibSetWaveformSelect(1);
}

void AdLibMidiDriver::adlibSetupChannels(int fl) {
	if (fl != 0) {
		_midiChannelsNote1Table[8] = 24;
		_midiChannelsNote2Table[8] = 8192;
		adlibPlayNote(8);
		_midiChannelsNote1Table[7] = 31;
		_midiChannelsNote2Table[7] = 8192;
		adlibPlayNote(7);
	}
	_adlibRhythmEnabled = fl;
	_midiNumberOfChannels = fl ? 11 : 9;
	_adlibVibratoRhythm = 0;
	_adlibAMDepthEq48 = 0;
	_adlibVibratoDepthEq14 = 0;
	_adlibKeyboardSplitOn = 0;
	adlibResetChannels();
	adlibSetAmpVibratoRhythm();
}

void AdLibMidiDriver::adlibResetAmpVibratoRhythm(int am, int vib, int kso) {
	_adlibAMDepthEq48 = am;
	_adlibVibratoDepthEq14 = vib;
	_adlibKeyboardSplitOn = kso;
	adlibSetAmpVibratoRhythm();
	adlibSetCSMKeyboardSplit();
}

void AdLibMidiDriver::adlibResetChannels() {
	for (int i = 0; i < 18; ++i) {
		adlibSetupChannelFromSequence(i, _adlibChannelsNoFeedback[i] ? _adlibInitSequenceData2 : _adlibInitSequenceData1, 0);
	}
	if (_adlibRhythmEnabled) {
		adlibSetupChannelFromSequence(12, _adlibInitSequenceData3, 0);
		adlibSetupChannelFromSequence(15, _adlibInitSequenceData4, 0);
		adlibSetupChannelFromSequence(16, _adlibInitSequenceData5, 0);
		adlibSetupChannelFromSequence(14, _adlibInitSequenceData6, 0);
		adlibSetupChannelFromSequence(17, _adlibInitSequenceData7, 0);
		adlibSetupChannelFromSequence(13, _adlibInitSequenceData8, 0);
	}
}

void AdLibMidiDriver::adlibSetAmpVibratoRhythm() {
	uint8 value = 0;
	if (_adlibAMDepthEq48) {
		value |= 0x80;
	}
	if (_adlibVibratoDepthEq14) {
		value |= 0x40;
	}
	if (_adlibRhythmEnabled) {
		value |= 0x20;
	}
	adlibWrite(0xBD, value | _adlibVibratoRhythm);
}

void AdLibMidiDriver::adlibSetCSMKeyboardSplit() {
	uint8 value = _adlibKeyboardSplitOn ? 0x40 : 0;
	adlibWrite(8, value);
}

void AdLibMidiDriver::adlibSetNoteMul(int mul) {
	if (mul > 12) {
		mul = 12;
	} else if (mul < 1) {
		mul = 1;
	}
	_adlibNoteMul = mul;
}

void AdLibMidiDriver::adlibSetWaveformSelect(int fl) {
	_adlibWaveformSelect = fl ? 0x20 : 0;
	for (int i = 0; i < 18; ++i) {
		adlibWrite(0xE0 + _adlibChannelsMappingTable1[i], 0);
	}
	adlibWrite(1, _adlibWaveformSelect);
}

void AdLibMidiDriver::adlibSetPitchBend(int channel, int range) {
	if ((_adlibRhythmEnabled && channel <= 6) || channel < 9) {
		if (range > 16383) {
			range = 16383;
		}
		_midiChannelsNote2Table[channel] = range;
		adlibPlayNote(channel);
	}
}

void AdLibMidiDriver::adlibPlayNote(int channel) {
	_midiChannelsFreqTable[channel] = adlibPlayNoteHelper(channel, _midiChannelsNote1Table[channel], _midiChannelsNote2Table[channel], _midiChannelsOctTable[channel]);
}

uint8 AdLibMidiDriver::adlibPlayNoteHelper(int channel, int note1, int note2, int oct) {
	int n = ((note2 * _midiChannelsNoteTable[channel]) >> 8) - 8192;
	if (n != 0) {
		n >>= 5;
		n *= _adlibNoteMul;
	}
	n += (note1 << 8) + 8;
	n >>= 4;
	if (n < 0) {
		n = 0;
	} else if (n > 1535) {
		n = 1535;
	}
	int index = (((n >> 4) % 12) << 4) | (n & 0xF);
	int f = _midiNoteFreqTable[index];
	int o = (n >> 4) / 12 - 1;
	if (f < 0) {
		++o;
	}
	if (o < 0) {
		++o;
		f >>= 1;
	}
	adlibWrite(0xA0 + channel, f & 0xFF);
	int value = ((f >> 8) & 3) | (o << 2) | oct;
	adlibWrite(0xB0 + channel, value);
	return value;
}

void AdLibMidiDriver::adlibTurnNoteOff(int channel) {
	if ((_adlibRhythmEnabled && channel <= 6) || channel < 9) {
		_midiChannelsOctTable[channel] = 0;
		_midiChannelsFreqTable[channel] &= ~0x20;
		adlibWrite(0xB0 + channel, _midiChannelsFreqTable[channel]);
	} else if (_adlibRhythmEnabled && channel <= 10) {
		_adlibVibratoRhythm &= ~(1 << (4 - (channel - 6)));
		adlibSetAmpVibratoRhythm();
	}
}

void AdLibMidiDriver::adlibTurnNoteOn(int channel, int note) {
	note -= 12;
	if (note < 0) {
		note = 0;
	}
	if ((_adlibRhythmEnabled && channel <= 6) || channel < 9) {
		_midiChannelsNote1Table[channel] = note;
		_midiChannelsOctTable[channel] = 0x20;
		adlibPlayNote(channel);
	} else if (_adlibRhythmEnabled && channel <= 10) {
		if (channel == 6) {
			_midiChannelsNote1Table[6] = note;
			adlibPlayNote(channel);
		} else if (channel == 8 && _midiChannelsNote1Table[8] == note) {
			_midiChannelsNote1Table[8] = note;
			_midiChannelsNote1Table[7] = note + 7;
			adlibPlayNote(8);
			adlibPlayNote(7);
		}
		_adlibVibratoRhythm = 1 << (4 - (channel - 6));
		adlibSetAmpVibratoRhythm();
	}
}

void AdLibMidiDriver::adlibSetupChannelFromSequence(int channel, const uint8 *src, int fl) {
	for (int i = 0; i < 13; ++i) {
		_adlibSetupChannelSequence2[i] = src[i];
	}
	adlibSetupChannel(channel, _adlibSetupChannelSequence2, fl);
}

void AdLibMidiDriver::adlibSetupChannel(int channel, const uint16 *src, int fl) {
	for (int i = 0; i < 13; ++i) {
		_adlibSetupChannelSequence1[14 * channel + i] = src[i];
	}
	_adlibSetupChannelSequence1[14 * channel + 13] = fl & 3;
	adlibSetupChannelHelper(channel);
}

void AdLibMidiDriver::adlibSetNoteVolume(int channel, int volume) {
	if (_midiNumberOfChannels > channel) {
		if (volume > 127) {
			volume = 127;
		}
		_adlibChannelsLevelKeyScalingTable[channel] = volume;
		const uint8 *p;
		if (_adlibRhythmEnabled) {
			p = &_adlibChannelsKeyScalingTable2[channel * 2];
		} else {
			p = &_adlibChannelsKeyScalingTable1[channel * 2];
		}
		adlibSetChannel0x40(p[0]);
		if (p[1] != 255) {
			adlibSetChannel0x40(p[1]);
		}
	}
}

void AdLibMidiDriver::adlibSetupChannelHelper(int channel) {
	adlibSetAmpVibratoRhythm();
	adlibSetCSMKeyboardSplit();
	adlibSetChannel0x40(channel);
	adlibSetChannel0xC0(channel);
	adlibSetChannel0x60(channel);
	adlibSetChannel0x80(channel);
	adlibSetChannel0x20(channel);
	adlibSetChannel0xE0(channel);
}

void AdLibMidiDriver::adlibSetChannel0x40(int channel) {
	int index, value, fl;

	if (_adlibRhythmEnabled) {
		index = _adlibChannelsMappingTable3[channel];
	} else {
		index = _adlibChannelsMappingTable2[channel];
	}
	value = 63 - (_adlibSetupChannelSequence1[channel * 14 + 8] & 63);
	fl = 0;
	if (_adlibRhythmEnabled && index > 6) {
		fl = -1;
	}
	if (_adlibChannelsNoFeedback[channel] || _adlibSetupChannelSequence1[channel * 14 + 12] == 0 || fl != 0) {
		value = ((_adlibChannelsLevelKeyScalingTable[index] * value) + 64) >> 7;
	}
	value = (_adlibChannelsVolumeTable[index] * value * 2) >> 8;
	if (value > 63) {
		value = 63;
	}
	value = 63 - value;
	value |= _adlibSetupChannelSequence1[channel * 14] << 6;
	adlibWrite(0x40 + _adlibChannelsMappingTable1[channel], value);
}

void AdLibMidiDriver::adlibSetChannel0xC0(int channel) {
	if (_adlibChannelsNoFeedback[channel] == 0) {
		const uint8 *p = &_adlibSetupChannelSequence1[channel * 14];
		uint8 value = p[2] << 1;
		if (p[12] == 0) {
			value |= 1;
		}
		adlibWrite(0xC0 + _adlibChannelsMappingTable2[channel], value);
	}
}

void AdLibMidiDriver::adlibSetChannel0x60(int channel) {
	const uint8 *p = &_adlibSetupChannelSequence1[channel * 14];
	uint8 value = (p[3] << 4) | (p[6] & 15);
	adlibWrite(0x60 + _adlibChannelsMappingTable1[channel], value);
}

void AdLibMidiDriver::adlibSetChannel0x80(int channel) {
	const uint8 *p = &_adlibSetupChannelSequence1[channel * 14];
	uint8 value = (p[4] << 4) | (p[7] & 15);
	adlibWrite(0x80 + _adlibChannelsMappingTable1[channel], value);
}

void AdLibMidiDriver::adlibSetChannel0x20(int channel) {
	const uint8 *p = &_adlibSetupChannelSequence1[channel * 14];
	uint8 value = p[1] & 15;
	if (p[9]) {
		value |= 0x80;
	}
	if (p[10]) {
		value |= 0x40;
	}
	if (p[5]) {
		value |= 0x20;
	}
	if (p[11]) {
		value |= 0x10;
	}
	adlibWrite(0x20 + _adlibChannelsMappingTable1[channel], value);
}

void AdLibMidiDriver::adlibSetChannel0xE0(int channel) {
	uint8 value = 0;
	if (_adlibWaveformSelect) {
		const uint8 *p = &_adlibSetupChannelSequence1[channel * 14];
		value = p[13] & 3;
	}
	adlibWrite(0xE0 + _adlibChannelsMappingTable1[channel], value);
}

const uint8 AdLibMidiDriver::_adlibChannelsMappingTable1[] = {
	0, 1, 2, 3, 4, 5, 8, 9, 10, 11, 12, 13, 16, 17, 18, 19, 20, 21
};

const uint8 AdLibMidiDriver::_adlibChannelsNoFeedback[] = {
	0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1
};

const uint8 AdLibMidiDriver::_adlibChannelsMappingTable2[] = {
	0, 1, 2, 0, 1, 2, 3, 4, 5, 3, 4, 5, 6, 7, 8, 6, 7, 8
};

const uint8 AdLibMidiDriver::_adlibChannelsMappingTable3[] = {
	0, 1, 2, 0, 1, 2, 3, 4, 5, 3, 4, 5, 6, 10, 8, 6, 7, 9
};

const uint8 AdLibMidiDriver::_adlibChannelsKeyScalingTable1[] = {
	0, 3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 13, 16, 14, 17
};

const uint8 AdLibMidiDriver::_adlibChannelsKeyScalingTable2[] = {
	0, 3, 1, 4, 2, 5, 6, 9, 7, 10, 8, 11, 12, 15, 16, 255, 14, 255, 17, 255, 13, 255
};

const uint8 AdLibMidiDriver::_adlibChannelsVolumeTable[] = {
	128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128
};

const uint8 AdLibMidiDriver::_adlibInitSequenceData1[] = {
	1, 1, 3, 15, 5, 0, 1, 3, 15, 0, 0, 0, 1, 0
};

const uint8 AdLibMidiDriver::_adlibInitSequenceData2[] = {
	0, 1, 1, 15, 7, 0, 2, 4, 0, 0, 0, 1, 0, 0
};

const uint8 AdLibMidiDriver::_adlibInitSequenceData3[] = {
	0, 0, 0, 10, 4, 0, 8, 12, 11, 0, 0, 0, 1, 0
};

const uint8 AdLibMidiDriver::_adlibInitSequenceData4[] = {
	0, 0, 0, 13, 4, 0, 6, 15, 0, 0, 0, 0, 1, 0
};

const uint8 AdLibMidiDriver::_adlibInitSequenceData5[] = {
	0, 12, 0, 15, 11, 0, 8, 5, 0, 0, 0, 0, 0, 0
};

const uint8 AdLibMidiDriver::_adlibInitSequenceData6[] = {
	0, 4, 0, 15, 11, 0, 7, 5, 0, 0, 0, 0, 0, 0
};

const uint8 AdLibMidiDriver::_adlibInitSequenceData7[] = {
	0, 1, 0, 15, 11, 0, 5, 5, 0, 0, 0, 0, 0, 0
};

const uint8 AdLibMidiDriver::_adlibInitSequenceData8[] = {
	0, 1, 0, 15, 11, 0, 7, 5, 0, 0, 0, 0, 0, 0
};

const int16 AdLibMidiDriver::_midiChannelsNoteTable[] = {
	256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256
};

const int16 AdLibMidiDriver::_midiNoteFreqTable[] = {
	 690,  692,  695,  697,  700,  702,  705,  707,  710,  713,  715,  718,
	 720,  723,  726,  728,  731,  733,  736,  739,  741,  744,  747,  749,
	 752,  755,  758,  760,  763,  766,  769,  771,  774,  777,  780,  783,
	 785,  788,  791,  794,  797,  800,  803,  806,  809,  811,  814,  817,
	 820,  823,  826,  829,  832,  835,  838,  841,  844,  847,  850,  854,
	 857,  860,  863,  866,  869,  872,  875,  879,  882,  885,  888,  891,
	 895,  898,  901,  904,  908,  911,  914,  917,  921,  924,  927,  931,
	 934,  937,  941,  944,  948,  951,  955,  958,  961,  965,  968,  972,
	 975,  979,  983,  986,  990,  993,  997, 1000, 1004, 1008, 1011, 1015,
	1019, 1022, -511, -509, -507, -505, -504, -502, -500, -498, -496, -494,
	-492, -490, -488, -486, -484, -482, -480, -479, -477, -475, -473, -471,
	-469, -467, -465, -463, -460, -458, -456, -454, -452, -450, -448, -446,
	-444, -442, -440, -438, -436, -433, -431, -429, -427, -425, -423, -420,
	-418, -416, -414, -412, -409, -407, -405, -403, -401, -398, -396, -394,
	-391, -389, -387, -385, -382, -380, -378, -375, -373, -371, -368, -366,
	-363, -361, -359, -356, -354, -351, -349, -347, -344, -342, -339, -337
};

MidiDriver *C_Player_CreateAdLibMidiDriver(Audio::Mixer *mixer) {
	return new AdLibMidiDriver(mixer);
}

} // End of namespace Queen
