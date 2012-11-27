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

#include "audio/midiplayer.h"
#include "audio/midiparser.h"

#include "common/config-manager.h"

namespace Audio {

MidiPlayer::MidiPlayer() :
	_driver(0),
	_parser(0),
	_midiData(0),
	_isLooping(false),
	_isPlaying(false),
	_masterVolume(0),
	_nativeMT32(false) {

	memset(_channelsTable, 0, sizeof(_channelsTable));
	memset(_channelsVolume, 127, sizeof(_channelsVolume));

// TODO
}

MidiPlayer::~MidiPlayer() {
	// FIXME/TODO: In some engines, stop() was called first;
	// in others, _driver->setTimerCallback(NULL, NULL) came first.
	// Hopefully, this make no real difference, but we should
	// watch out for regressions.
	stop();

	// Unhook & unload the driver
	if (_driver) {
		_driver->setTimerCallback(0, 0);
		_driver->close();
		delete _driver;
		_driver = 0;
	}
}

void MidiPlayer::createDriver(int flags) {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(flags);
	_nativeMT32 = ((MidiDriver::getMusicType(dev) == MT_MT32) || ConfMan.getBool("native_mt32"));

	_driver = MidiDriver::createMidi(dev);
	assert(_driver);
	if (_nativeMT32)
		_driver->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);
}


void MidiPlayer::setVolume(int volume) {
	volume = CLIP(volume, 0, 255);
	if (_masterVolume == volume)
		return;

	Common::StackLock lock(_mutex);

	_masterVolume = volume;
	for (int i = 0; i < kNumChannels; ++i) {
		if (_channelsTable[i]) {
			_channelsTable[i]->volume(_channelsVolume[i] * _masterVolume / 255);
		}
	}
}

void MidiPlayer::syncVolume() {
	int volume = ConfMan.getInt("music_volume");
	if (ConfMan.getBool("mute")) {
		volume = -1;
	}
	setVolume(volume);
}


void MidiPlayer::send(uint32 b) {
	byte ch = (byte)(b & 0x0F);
	if ((b & 0xFFF0) == 0x07B0) {
		// Adjust volume changes by master volume
		byte volume = (byte)((b >> 16) & 0x7F);
		_channelsVolume[ch] = volume;
		volume = volume * _masterVolume / 255;
		b = (b & 0xFF00FFFF) | (volume << 16);
	} else if ((b & 0xFFF0) == 0x007BB0) {
		// Only respond to All Notes Off if this channel
		// has currently been allocated
		if (!_channelsTable[ch])
			return;
	}

	sendToChannel(ch, b);
}

void MidiPlayer::sendToChannel(byte ch, uint32 b) {
	if (!_channelsTable[ch]) {
		_channelsTable[ch] = (ch == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// TODO: Some engines overload this method to insert code at this
		// point which calls the channel's volume() method.
		// Does this make sense, and should we maybe do it in general?
	}
	if (_channelsTable[ch]) {
		_channelsTable[ch]->send(b);
	}
}

void MidiPlayer::metaEvent(byte type, byte *data, uint16 length) {
	switch (type) {
	case 0x2F:	// End of Track
		endOfTrack();
		break;
	default:
		//warning("Unhandled meta event: %02x", type);
		break;
	}
}

void MidiPlayer::endOfTrack() {
	if (_isLooping) {
		assert(_parser);
		_parser->jumpToTick(0);
	} else
		stop();
}

void MidiPlayer::timerCallback(void *data) {
	assert(data);
	((MidiPlayer *)data)->onTimer();
}

void MidiPlayer::onTimer() {
	Common::StackLock lock(_mutex);

	// TODO: Maybe we can replace _isPlaying
	// by a simple check for "_parser != 0" ?

	if (_isPlaying && _parser) {
		_parser->onTimer();
	}
}


void MidiPlayer::stop() {
	Common::StackLock lock(_mutex);

	_isPlaying = false;
	if (_parser) {
		_parser->unloadMusic();

		// FIXME/TODO: The MidiParser destructor calls allNotesOff()
		// but unloadMusic also does. To suppress double notes-off,
		// we reset the midi driver of _parser before deleting it.
		// This smells very fishy, in any case.
		_parser->setMidiDriver(0);

		delete _parser;
		_parser = NULL;
	}

	free(_midiData);
	_midiData = 0;
}

void MidiPlayer::pause() {
//	debugC(2, kDraciSoundDebugLevel, "Pausing track %d", _track);
	_isPlaying = false;
	setVolume(-1);	// FIXME: This should be 0, shouldn't it?
}

void MidiPlayer::resume() {
//	debugC(2, kDraciSoundDebugLevel, "Resuming track %d", _track);
	syncVolume();
	_isPlaying = true;
}

} // End of namespace Audio
