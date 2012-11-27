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

#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/mixer.h"
#include "audio/midiparser.h"
#include "audio/midiplayer.h"
#include "audio/mods/protracker.h"
#include "audio/decoders/raw.h"

#include "parallaction/disk.h"
#include "parallaction/parallaction.h"
#include "parallaction/sound.h"


namespace Parallaction {


/*
 * List of calls to the original music driver.
 *
 *
 * 1 set music buffer segment
 * 2 set music buffer offset
 * 3 set music buffer size
 * 4 play/resume
 * 5 stop
 * 6 pause
 * 7 set channel volume
 * 8 set fade in flag
 * 9 set fade out flag
 * 10 set global volume
 * 11 shutdown
 * 12 get status (stopped, playing, paused)
 * 13 set fade volume change rate
 * 14 get global volume
 * 15 get fade in flag
 * 16 get fade out flag
 * 17 set tempo
 * 18 get tempo
 * 19 get fade volume change rate
 * 20 get looping flag
 * 21 set looping flag
 * 22 get version??
 * 23 get version??
 * 24 get busy flag (dsp has pending data)
 */

class MidiParser_MSC : public MidiParser {
protected:
	virtual void parseNextEvent(EventInfo &info);
	virtual bool loadMusic(byte *data, uint32 size);

	uint8  read1(byte *&data) {
		return *data++;
	}

	void parseMetaEvent(EventInfo &info);
	void parseMidiEvent(EventInfo &info);

	bool byte_11C5A;
	uint8 _beats;
	uint8 _lastEvent;
	byte *_trackEnd;

public:
	MidiParser_MSC() : byte_11C5A(false) {
	}
};

void MidiParser_MSC::parseMetaEvent(EventInfo &info) {
	uint8 type = read1(_position._playPos);
	uint8 len = read1(_position._playPos);
	info.ext.type = type;
	info.length = len;
	info.ext.data = 0;

	if (type == 0x51) {
		info.ext.data = _position._playPos;
	} else {
		warning("unknown meta event 0x%02X", type);
		info.ext.type = 0;
	}

	_position._playPos += len;
}

void MidiParser_MSC::parseMidiEvent(EventInfo &info) {
	uint8 type = info.command();

	switch (type) {
	case 0x8:
	case 0x9:
	case 0xA:
	case 0xB:
	case 0xE:
		info.basic.param1 = read1(_position._playPos);
		info.basic.param2 = read1(_position._playPos);
		break;

	case 0xC:
	case 0xD:
		info.basic.param1 = read1(_position._playPos);
		info.basic.param2 = 0;
		break;

	default:
		warning("Unexpected midi event 0x%02X in midi data", info.event);
	}

	//if ((type == 0xB) && (info.basic.param1 == 64)) info.basic.param2 = 127;

}

void MidiParser_MSC::parseNextEvent(EventInfo &info) {
	info.start = _position._playPos;

	if (_position._playPos >= _trackEnd) {
		// fake an end-of-track meta event
		info.delta = 0;
		info.event = 0xFF;
		info.ext.type = 0x2F;
		info.length = 0;
		return;
	}

	info.length = 0;
	info.delta = readVLQ(_position._playPos);
	info.event = read1(_position._playPos);

	if (info.event == 0xFF) {
		parseMetaEvent(info);
		return;
	}

	if (info.event < 0x80) {
		_position._playPos--;
		info.event = _lastEvent;
	}

	parseMidiEvent(info);
	_lastEvent = info.event;

}

bool MidiParser_MSC::loadMusic(byte *data, uint32 size) {
	unloadMusic();

	byte *pos = data;

	if (memcmp("MSCt", pos, 4)) {
		warning("Expected header not found in music file");
		return false;
	}
	pos += 4;

	_beats = read1(pos);
	_ppqn = read2low(pos);

	if (byte_11C5A) {
		// do something with byte_11C4D
	}

	_lastEvent = 0;
	_trackEnd = data + size;

	_numTracks = 1;
	_tracks[0] = pos;

	setTempo(500000);
	setTrack(0);
	return true;
}


MidiParser *createParser_MSC() {
	return new MidiParser_MSC;
}


class MidiPlayer_MSC : public Audio::MidiPlayer {
public:

	MidiPlayer_MSC();

	void play(Common::SeekableReadStream *stream);
	virtual void pause(bool p);
	virtual void setVolume(int volume);
	virtual void onTimer();

	// MidiDriver_BASE interface
	virtual void send(uint32 b);


private:
	void setVolumeInternal(int volume);
	bool _paused;
};



MidiPlayer_MSC::MidiPlayer_MSC()
	: _paused(false) {

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	const MusicType musicType = MidiDriver::getMusicType(dev);
	if (musicType == MT_ADLIB) {
		_driver = createAdLibDriver();
	} else {
		_driver = MidiDriver::createMidi(dev);
	}
	assert(_driver);

	int ret = _driver->open();
	if (ret == 0) {
		_driver->setTimerCallback(this, &timerCallback);
	}
}

void MidiPlayer_MSC::play(Common::SeekableReadStream *stream) {
	Common::StackLock lock(_mutex);

	stop();
	if (!stream)
		return;

	int size = stream->size();
	_midiData = (uint8 *)malloc(size);
	if (_midiData) {
		stream->read(_midiData, size);
		delete stream;

		_parser = createParser_MSC();
		_parser->loadMusic(_midiData, size);
		_parser->setTrack(0);
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		_isLooping = true;
		_isPlaying = true;
	}
}

void MidiPlayer_MSC::pause(bool p) {
	_paused = p;
	setVolumeInternal(_paused ? 0 : _masterVolume);
}

void MidiPlayer_MSC::onTimer() {
	Common::StackLock lock(_mutex);

	if (!_paused && _isPlaying && _parser) {
		_parser->onTimer();
	}
}

void MidiPlayer_MSC::setVolume(int volume) {
	_masterVolume = CLIP(volume, 0, 255);
	setVolumeInternal(_masterVolume);
}

void MidiPlayer_MSC::setVolumeInternal(int volume) {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < kNumChannels; ++i) {
		if (_channelsTable[i]) {
			_channelsTable[i]->volume(_channelsVolume[i] * volume / 255);
		}
	}
}

void MidiPlayer_MSC::send(uint32 b) {
	// FIXME/TODO: Unlike Audio::MidiPlayer::send(), this code
	// does not handle All Note Off. Is this on purpose?
	// If not, we could simply remove this method, and use the
	// inherited one.
	const byte ch = b & 0x0F;
	byte param2 = (b >> 16) & 0xFF;

	switch (b & 0xFFF0) {
	case 0x07B0: // volume change
		_channelsVolume[ch] = param2;
		break;
	}

	sendToChannel(ch, b);
}

DosSoundMan_br::DosSoundMan_br(Parallaction_br *vm) : SoundMan_br(vm) {
	_midiPlayer = new MidiPlayer_MSC();
	assert(_midiPlayer);
}

DosSoundMan_br::~DosSoundMan_br() {
	delete _midiPlayer;
}

Audio::AudioStream *DosSoundMan_br::loadChannelData(const char *filename, Channel *ch, bool looping) {
	Common::SeekableReadStream *stream = _vm->_disk->loadSound(filename);

	uint32 dataSize = stream->size();
	byte *data = (byte *)malloc(dataSize);
	if (stream->read(data, dataSize) != dataSize)
		error("DosSoundMan_br::loadChannelData: Read failed");

	delete stream;

	// TODO: Confirm sound rate
	int rate = 11025;

	ch->stream = Audio::makeLoopingAudioStream(
			Audio::makeRawStream(data, dataSize, rate, Audio::FLAG_UNSIGNED),
			looping ? 0 : 1);
	return ch->stream;
}

void DosSoundMan_br::playSfx(const char *filename, uint channel, bool looping, int volume) {
	stopSfx(channel);

	if (!_sfxEnabled) {
		return;
	}

	debugC(1, kDebugAudio, "DosSoundMan_br::playSfx(%s, %u, %i, %i)", filename, channel, looping, volume);

	Channel *ch = &_channels[channel];
	Audio::AudioStream *input = loadChannelData(filename, ch, looping);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
}

void DosSoundMan_br::playMusic() {
	if (_musicFile.empty()) {
		return;
	}

	if (!_musicEnabled) {
		return;
	}

	Common::SeekableReadStream *s = _vm->_disk->loadMusic(_musicFile.c_str());
	assert(s);
	_midiPlayer->play(s);
}

void DosSoundMan_br::stopMusic() {
	_midiPlayer->stop();
}

void DosSoundMan_br::pause(bool p) {
	_midiPlayer->pause(p);
}

AmigaSoundMan_br::AmigaSoundMan_br(Parallaction_br *vm) : SoundMan_br(vm)  {
	_musicStream = 0;
}

AmigaSoundMan_br::~AmigaSoundMan_br() {
	stopMusic();
}

Audio::AudioStream *AmigaSoundMan_br::loadChannelData(const char *filename, Channel *ch, bool looping) {
	Common::SeekableReadStream *stream = _vm->_disk->loadSound(filename);
	Audio::AudioStream *input = 0;

	if (_vm->getFeatures() & GF_DEMO) {
		uint32 dataSize = stream->size();
		int8 *data = (int8 *)malloc(dataSize);
		if (stream->read(data, dataSize) != dataSize)
			error("DosSoundMan_br::loadChannelData: Read failed");

		// TODO: Confirm sound rate
		int rate = 11025;
		input = Audio::makeRawStream((byte *)data, dataSize, rate, 0);
	} else {
		input = Audio::make8SVXStream(*stream, looping);
	}

	delete stream;

	ch->stream = input;
	return input;
}

void AmigaSoundMan_br::playSfx(const char *filename, uint channel, bool looping, int volume) {
	if (channel >= NUM_SFX_CHANNELS) {
		warning("unknown sfx channel");
		return;
	}

	stopSfx(channel);

	if (!_sfxEnabled) {
		return;
	}

	debugC(1, kDebugAudio, "AmigaSoundMan_ns::playSfx(%s, %i)", filename, channel);

	Channel *ch = &_channels[channel];
	Audio::AudioStream *input = loadChannelData(filename, ch, looping);

	if (volume == -1) {
		volume = ch->volume;
	}

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
}

void AmigaSoundMan_br::playMusic() {
	stopMusic();

	if (!_musicEnabled) {
		return;
	}

	debugC(1, kDebugAudio, "AmigaSoundMan_ns::playMusic()");

	Common::SeekableReadStream *stream = _vm->_disk->loadMusic(_musicFile.c_str());
	// NOTE: Music files don't always exist
	if (!stream)
		return;

	_musicStream = Audio::makeProtrackerStream(stream);
	delete stream;

	debugC(3, kDebugAudio, "AmigaSoundMan_ns::playMusic(): created new music stream");

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, _musicStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, false);
}

void AmigaSoundMan_br::stopMusic() {
	debugC(1, kDebugAudio, "AmigaSoundMan_ns::stopMusic()");

	if (_mixer->isSoundHandleActive(_musicHandle)) {
		_mixer->stopHandle(_musicHandle);
		delete _musicStream;
		_musicStream = 0;
	}
}

void AmigaSoundMan_br::pause(bool p) {
	_mixer->pauseHandle(_musicHandle, p);
}

SoundMan_br::SoundMan_br(Parallaction_br *vm) : _vm(vm) {
	_mixer = _vm->_mixer;

	_musicEnabled = true;
	_sfxEnabled = true;
}

SoundMan_br::~SoundMan_br() {
	stopAllSfx();
}

void SoundMan_br::stopAllSfx() {
	stopSfx(0);
	stopSfx(1);
	stopSfx(2);
	stopSfx(3);
}

void SoundMan_br::setMusicFile(const char *name) {
	stopMusic();
	_musicFile = name;
}

void SoundMan_br::stopSfx(uint channel) {
	if (channel >= NUM_SFX_CHANNELS) {
		warning("unknown sfx channel");
		return;
	}

	debugC(1, kDebugAudio, "SoundMan_br::stopSfx(%i)", channel);
	_mixer->stopHandle(_channels[channel].handle);
	_channels[channel].stream = 0;
}

void SoundMan_br::execute(int command, const char *parm) {
	uint32 n = parm ? strtoul(parm, 0, 10) : 0;
	bool b = (n == 1) ? true : false;

	switch (command) {
	case SC_PLAYMUSIC:
		playMusic();
		break;
	case SC_STOPMUSIC:
		stopMusic();
		break;
	case SC_SETMUSICFILE:
		setMusicFile(parm);
		break;

	case SC_PLAYSFX:
		playSfx(parm, _sfxChannel, _sfxLooping, _sfxVolume);
		break;
	case SC_STOPSFX:
		stopSfx(n);
		break;

	case SC_SETSFXCHANNEL:
		_sfxChannel = n;
		break;
	case SC_SETSFXLOOPING:
		_sfxLooping = b;
		break;
	case SC_SETSFXVOLUME:
		_sfxVolume = n;
		break;

	case SC_PAUSE:
		pause(b);
		break;
	}
}

void SoundMan_br::enableSfx(bool enable) {
	if (!enable) {
		stopAllSfx();
	}
	_sfxEnabled = enable;
}

void SoundMan_br::enableMusic(bool enable) {
	if (enable) {
		playMusic();
	} else {
		stopMusic();
	}
	_musicEnabled = enable;
}

bool SoundMan_br::isSfxEnabled() const {
	return _sfxEnabled;
}

bool SoundMan_br::isMusicEnabled() const {
	return _musicEnabled;
}

} // namespace Parallaction
