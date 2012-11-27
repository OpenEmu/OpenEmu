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

#include "common/config-manager.h"
#include "common/stream.h"

#include "audio/midiparser.h"

#include "touche/midi.h"

namespace Touche {

static const uint8 _gmToRol[256] = {
	0x01, 0x02, 0x03, 0x08, 0x04, 0x05, 0x11, 0x14, 0x66, 0x66, 0x66, 0x62, 0x69, 0x68, 0x67, 0x26,
	0x09, 0x0A, 0x0B, 0x0E, 0x0F, 0x10, 0x10, 0x10, 0x3C, 0x3D, 0x3D, 0x3D, 0x3D, 0x3E, 0x3F, 0x3F,
	0x47, 0x41, 0x42, 0x48, 0x45, 0x46, 0x1D, 0x1E, 0x35, 0x36, 0x37, 0x39, 0x33, 0x34, 0x3A, 0x71,
	0x31, 0x32, 0x31, 0x32, 0x23, 0x23, 0x23, 0x7B, 0x59, 0x5B, 0x5F, 0x5A, 0x5D, 0x60, 0x19, 0x1A,
	0x4F, 0x50, 0x51, 0x52, 0x55, 0x56, 0x57, 0x53, 0x4B, 0x49, 0x4D, 0x4E, 0x6F, 0x6C, 0x6D, 0x6E,
	0x30, 0x19, 0x4E, 0x2B, 0x28, 0x23, 0x19, 0x30, 0x21, 0x25, 0x1C, 0x21, 0x24, 0x22, 0x21, 0x22,
	0x2A, 0x25, 0x24, 0x26, 0x2E, 0x22, 0x29, 0x21, 0x40, 0x40, 0x6A, 0x6A, 0x68, 0x10, 0x35, 0x10,
	0x7F, 0x6B, 0x69, 0x75, 0x76, 0x72, 0x74, 0x01, 0x01, 0x70, 0x01, 0x7D, 0x7C, 0x01, 0x01, 0x01
};


MidiPlayer::MidiPlayer() {

	// FIXME: Necessary?
	memset(_channelsVolume, 0, sizeof(_channelsVolume));

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_nativeMT32 = ((MidiDriver::getMusicType(dev) == MT_MT32) || ConfMan.getBool("native_mt32"));
	_driver = MidiDriver::createMidi(dev);
	int ret = _driver->open();
	if (ret == 0) {
		_driver->setTimerCallback(this, &timerCallback);

		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();
	}
}

void MidiPlayer::play(Common::ReadStream &stream, int size, bool loop) {
	Common::StackLock lock(_mutex);

	stop();
	_midiData = (byte *)malloc(size);
	if (_midiData) {
		stream.read(_midiData, size);

		_parser = MidiParser::createParser_SMF();
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());

		_parser->loadMusic(_midiData, size);
		_parser->setTrack(0);
		_isLooping = loop;
		_isPlaying = true;
	}
}

void MidiPlayer::adjustVolume(int diff) {
	setVolume(_masterVolume + diff);
}

void MidiPlayer::setVolume(int volume) {
	// FIXME: This is almost identical to Audio::MidiPlayer::setVolume,
	// the only difference is that this implementation will always
	// transmit the volume change, even if the current _masterVolume
	// equals the new master volume. This *could* make a difference in
	// some situations.
	// So, we should determine whether Touche requires this behavioral
	// difference; and maybe also if other engines could benefit from it
	// (as hypothetically, it might fix some subtle bugs?)
	_masterVolume = CLIP(volume, 0, 255);
	Common::StackLock lock(_mutex);
	for (int i = 0; i < kNumChannels; ++i) {
		if (_channelsTable[i]) {
			_channelsTable[i]->volume(_channelsVolume[i] * _masterVolume / 255);
		}
	}
}

void MidiPlayer::send(uint32 b) {
	if ((b & 0xF0) == 0xC0 && _nativeMT32) { // program change
		b = (b & 0xFFFF00FF) | (_gmToRol[(b >> 8) & 0x7F] << 8);
	}
	Audio::MidiPlayer::send(b);
}

} // Touche namespace
