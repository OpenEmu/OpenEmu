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

#include "kyra/sound_intern.h"

#include "audio/mixer.h"
#include "audio/softsynth/pcspk.h"

namespace Kyra {

MidiDriver_PCSpeaker::MidiDriver_PCSpeaker(Audio::Mixer *mixer)
	: MidiDriver_Emulated(mixer), _rate(mixer->getOutputRate()) {
	_timerValue = 0;
	memset(_channel, 0, sizeof(_channel));
	memset(_note, 0, sizeof(_note));

	for (int i = 0; i < 2; ++i)
		_note[i].hardwareChannel = 0xFF;

	_speaker = new Audio::PCSpeaker(_rate);
	assert(_speaker);
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_mixerSoundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_countdown = 0xFFFF;
	_hardwareChannel[0] = 0xFF;
	_modulationFlag = false;
}

MidiDriver_PCSpeaker::~MidiDriver_PCSpeaker() {
	_mixer->stopHandle(_mixerSoundHandle);
	delete _speaker;
	_speaker = 0;
}

void MidiDriver_PCSpeaker::send(uint32 data) {
	Common::StackLock lock(_mutex);

	uint8 channel = data & 0x0F;
	uint8 param1 = (data >>  8) & 0xFF;
	uint8 param2 = (data >> 16) & 0xFF;

	uint8 flags = 0x00;

	if (channel > 1)
		return;

	switch (data & 0xF0) {
	case 0x80:	// note off
		noteOff(channel, param1);
		return;

	case 0x90:	// note on
		if (channel > 1)
			return;

		if (param2)
			noteOn(channel, param1);
		else
			noteOff(channel, param1);
		return;

	case 0xB0:	// controller
		switch (param1) {
		case 0x01:	// modulation
			_channel[channel].modulation = param2;
			break;

		case 0x40:	// hold
			_channel[channel].hold = param2;
			if (param2 < 0x40)
				resetController(channel);
			return;

		case 0x70:	// voice protect
			_channel[channel].voiceProtect = param2;
			return;

		case 0x79:	// all notes off
			_channel[channel].hold = 0;
			resetController(channel);
			_channel[channel].modulation = 0;
			_channel[channel].pitchBendLow = 0;
			_channel[channel].pitchBendHigh = 0x40;
			flags = 0x01;
			break;

		default:
			return;
		}
		break;

	case 0xE0:	// pitch bend
		flags = 0x01;
		_channel[channel].pitchBendLow = param1;
		_channel[channel].pitchBendHigh = param2;
		break;

	default:
		return;
	}

	for (int i = 0; i < 2; ++i) {
		if (_note[i].enabled && _note[i].midiChannel == channel) {
			_note[i].flags |= flags;
			setupTone(i);
		}
	}
}

void MidiDriver_PCSpeaker::resetController(int channel) {
	for (int i = 0; i < 2; ++i) {
		if (_note[i].enabled && _note[i].midiChannel == channel && _note[i].processHold)
			noteOff(channel, _note[i].note);
	}
}

void MidiDriver_PCSpeaker::noteOn(int channel, int note) {
	int n = 0;

	while (n < 2 && _note[n].enabled)
		++n;

	if (n >= 2)
		return;

	_note[n].midiChannel = channel;
	_note[n].note = note;
	_note[n].enabled = true;
	_note[n].processHold = false;
	_note[n].hardwareFlags = 0x20;
	_note[n].priority = 0x7FFF;
	_note[n].flags = 0x01;

	turnNoteOn(n);
}

void MidiDriver_PCSpeaker::turnNoteOn(int note) {
	if (_hardwareChannel[0] == 0xFF) {
		_note[note].hardwareChannel = 0;
		++_channel[_note[note].midiChannel].noteCount;
		_hardwareChannel[0] = _note[note].midiChannel;
		_note[note].flags = 0x01;

		setupTone(note);
	} else {
		overwriteNote(note);
	}
}

void MidiDriver_PCSpeaker::overwriteNote(int note) {
	int totalNotes = 0;
	for (int i = 0; i < 2; ++i) {
		if (_note[i].enabled) {
			++totalNotes;
			const int channel = _note[i].midiChannel;

			uint16 priority = 0xFFFF;
			if (_channel[channel].voiceProtect < 0x40)
				priority = _note[i].priority;

			if (_channel[channel].noteCount > priority)
				priority = 0;
			else
				priority -= _channel[channel].noteCount;

			_note[i].precedence = priority;
		}
	}

	if (totalNotes <= 1)
		return;

	do {
		uint16 maxValue = 0;
		uint16 minValue = 0xFFFF;
		int newNote = 0;

		for (int i = 0; i < 2; ++i) {
			if (_note[i].enabled) {
				if (_note[i].hardwareChannel == 0xFF) {
					if (_note[i].precedence >= maxValue) {
						maxValue = _note[i].precedence;
						newNote = i;
					}
				} else {
					if (_note[i].precedence <= minValue) {
						minValue = _note[i].precedence;
						note = i;
					}
				}
			}
		}

		if (maxValue < minValue)
			return;

		turnNoteOff(_note[note].hardwareChannel);
		_note[note].enabled = false;

		_note[newNote].hardwareChannel = _note[note].hardwareChannel;
		++_channel[_note[newNote].midiChannel].noteCount;
		_hardwareChannel[_note[note].hardwareChannel] = _note[newNote].midiChannel;
		_note[newNote].flags = 0x01;

		setupTone(newNote);
	} while (--totalNotes);
}

void MidiDriver_PCSpeaker::noteOff(int channel, int note) {
	for (int i = 0; i < 2; ++i) {
		if (_note[i].enabled && _note[i].note == note && _note[i].midiChannel == channel) {
			if (_channel[i].hold < 0x40) {
				turnNoteOff(i);
				_note[i].enabled = false;
			} else {
				_note[i].processHold = true;
			}
		}
	}
}

void MidiDriver_PCSpeaker::turnNoteOff(int note) {
	if (_note[note].hardwareChannel != 0xFF) {
		_note[note].hardwareFlags &= 0xDF;
		_note[note].flags |= 1;

		setupTone(note);

		--_channel[_note[note].midiChannel].noteCount;

		_hardwareChannel[_note[note].hardwareChannel] = 0xFF;
		_note[note].hardwareChannel = 0xFF;
	}
}

void MidiDriver_PCSpeaker::setupTone(int note) {
	if (_note[note].hardwareChannel == 0xFF)
		return;

	if (!(_note[note].flags & 0x01))
		return;

	if (!(_note[note].hardwareFlags & 0x20)) {
		_speaker->stop();
	} else {
		const int midiChannel = _note[note].midiChannel;
		uint16 pitchBend = (_channel[midiChannel].pitchBendHigh << 7) | _channel[midiChannel].pitchBendLow;

		int noteValue = _note[note].note;

		noteValue -= 24;
		do {
			noteValue += 12;
		} while (noteValue < 0);

		noteValue += 12;
		do {
			noteValue -= 12;
		} while (noteValue > 95);

		int16 modulation = _note[note].modulation;

		int tableIndex = MAX(noteValue - 12, 0);
		uint16 note1 = (_noteTable2[tableIndex] << 8) | _noteTable1[tableIndex];
		tableIndex = MIN(noteValue + 12, 95);
		uint16 note2 = (_noteTable2[tableIndex] << 8) | _noteTable1[tableIndex];
		uint16 note3 = (_noteTable2[noteValue] << 8) | _noteTable1[noteValue];

		int32 countdown = pitchBend - 0x2000;
		countdown += modulation;

		if (countdown >= 0)
			countdown *= (note2 - note3);
		else
			countdown *= (note3 - note1);

		countdown /= 0x2000;
		countdown += note3;

		countdown = uint16(countdown & 0xFFFF);
		if (countdown != _countdown)
			_countdown = countdown;

		_speaker->play(Audio::PCSpeaker::kWaveFormSquare, 1193180 / _countdown, -1);
	}

	_note[note].flags &= 0xFE;
}

void MidiDriver_PCSpeaker::generateSamples(int16 *buffer, int numSamples) {
	Common::StackLock lock(_mutex);
	_speaker->readBuffer(buffer, numSamples);
}

void MidiDriver_PCSpeaker::onTimer() {
	/*Common::StackLock lock(_mutex);

	_timerValue += 20;
	if (_timerValue < 120)
		return;
	_timerValue -= 120;

	_modulationFlag = !_modulationFlag;
	for (int i = 0; i < 2; ++i) {
		if (_note[i].enabled) {
			uint16 modValue = 5 * _channel[_note[i].midiChannel].modulation;
			if (_modulationFlag)
				modValue = -modValue;
			_note[i].modulation = modValue;
			_note[i].flags |= 1;

			setupTone(i);
		}
	}*/
}

const uint8 MidiDriver_PCSpeaker::_noteTable1[] = {
	0x88, 0xB5, 0x4E, 0x40, 0x41, 0xCD, 0xC4, 0x3D,
	0x43, 0x7C, 0x2A, 0xD6, 0x88, 0xB5, 0xFF, 0xD1,
	0x20, 0xA7, 0xE2, 0x1E, 0xCE, 0xBE, 0xF2, 0x8A,
	0x44, 0x41, 0x7F, 0xE8, 0x90, 0x63, 0x63, 0x8F,
	0xE7, 0x5F, 0x01, 0xBD, 0xA2, 0xA0, 0xBF, 0xF4,
	0x48, 0xB1, 0x31, 0xC7, 0x70, 0x2F, 0xFE, 0xE0,
	0xD1, 0xD0, 0xDE, 0xFB, 0x24, 0x58, 0x98, 0xE3,
	0x39, 0x97, 0xFF, 0x6F, 0xE8, 0x68, 0xEF, 0x7D,
	0x11, 0xAC, 0x4C, 0xF1, 0x9C, 0x4B, 0xFF, 0xB7,
	0x74, 0x34, 0xF7, 0xBE, 0x88, 0x56, 0x26, 0xF8,
	0xCE, 0xA5, 0x7F, 0x5B, 0x3A, 0x1A, 0xFB, 0xDF,
	0xC4, 0xAB, 0x93, 0x7C, 0x67, 0x52, 0x3F, 0x2D
};

const uint8 MidiDriver_PCSpeaker::_noteTable2[] = {
	0x8E, 0x86, 0xFD, 0xF0, 0xE2, 0xD5, 0xC9, 0xBE,
	0xB3, 0xA9, 0xA0, 0x96, 0x8E, 0x86, 0x7E, 0x77,
	0x71, 0x6A, 0x64, 0x5F, 0x59, 0x54, 0x4F, 0x4B,
	0x47, 0x43, 0x3F, 0x3B, 0x38, 0x35, 0x32, 0x2F,
	0x2C, 0x2A, 0x28, 0x25, 0x23, 0x21, 0x1F, 0x1D,
	0x1C, 0x1A, 0x19, 0x17, 0x16, 0x15, 0x13, 0x12,
	0x11, 0x10, 0x0F, 0x0E, 0x0E, 0x0D, 0x0C, 0x0B,
	0x0B, 0x0A, 0x09, 0x09, 0x08, 0x08, 0x07, 0x07,
	0x07, 0x06, 0x06, 0x05, 0x05, 0x05, 0x04, 0x04,
	0x04, 0x04, 0x03, 0x03, 0x03, 0x03, 0x03, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};

} // End of namespace Kyra
