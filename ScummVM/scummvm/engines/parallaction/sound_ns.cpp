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

#include "common/file.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "audio/mixer.h"
#include "audio/midiparser.h"
#include "audio/midiplayer.h"
#include "audio/mods/protracker.h"
#include "audio/decoders/raw.h"

#include "parallaction/sound.h"
#include "parallaction/parallaction.h"


namespace Parallaction {

class MidiPlayer : public Audio::MidiPlayer {
public:

	MidiPlayer();

	void play(Common::SeekableReadStream *stream);
	void pause(bool p);
	virtual void onTimer();

private:
	bool _paused;
};

MidiPlayer::MidiPlayer()
	: _paused(false) {

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_driver = MidiDriver::createMidi(dev);
	assert(_driver);

	int ret = _driver->open();
	if (ret == 0) {
		_driver->setTimerCallback(this, &timerCallback);
	}
}

void MidiPlayer::play(Common::SeekableReadStream *stream) {
	Common::StackLock lock(_mutex);

	stop();
	if (!stream)
		return;

	int size = stream->size();
	_midiData = (uint8 *)malloc(size);
	if (_midiData) {
		stream->read(_midiData, size);
		delete stream;

		_parser = MidiParser::createParser_SMF();
		_parser->loadMusic(_midiData, size);
		_parser->setTrack(0);
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		_isLooping = true;
		_isPlaying = true;
	}
}

void MidiPlayer::pause(bool p) {
	_paused = p;

	for (int i = 0; i < kNumChannels; ++i) {
		if (_channelsTable[i]) {
			_channelsTable[i]->volume(_paused ? 0 : _channelsVolume[i] * _masterVolume / 255);
		}
	}
}

void MidiPlayer::onTimer() {
	Common::StackLock lock(_mutex);

	if (!_paused && _isPlaying && _parser) {
		_parser->onTimer();
	}
}

DosSoundMan_ns::DosSoundMan_ns(Parallaction_ns *vm) : SoundMan_ns(vm), _playing(false) {
	_midiPlayer = new MidiPlayer();
}

DosSoundMan_ns::~DosSoundMan_ns() {
	debugC(1, kDebugAudio, "DosSoundMan_ns_ns::playMusic()");

	delete _midiPlayer;
}

bool DosSoundMan_ns::isLocationSilent(const char *locationName) {

	// these are the prefixes for location names with no background midi music
	const char *noMusicPrefix[] = { "museo", "intgrottadopo", "caveau", "estgrotta", "plaza1", "endtgz", "common", 0 };
	Common::String s(locationName);

	for (int i = 0; noMusicPrefix[i]; i++) {
		if (s.hasPrefix(noMusicPrefix[i])) {
			return true;
		}
	}

	return false;
}

void DosSoundMan_ns::playMusic() {
	debugC(1, kDebugAudio, "DosSoundMan_ns_ns::playMusic()");

	if (isLocationSilent(_vm->_location._name)) {
		// just stop the music if this location is silent
		_midiPlayer->stop();
		return;
	}

	Common::SeekableReadStream *stream = _vm->_disk->loadMusic(_musicFile);
	_midiPlayer->play(stream);
	_midiPlayer->setVolume(255);

	_playing = true;
}

void DosSoundMan_ns::stopMusic() {
	_musicFile[0] = 0;
	_midiPlayer->stop();

	_playing = false;
}

void DosSoundMan_ns::pause(bool p) {
	SoundMan_ns::pause(p);
	_midiPlayer->pause(p);
}

bool DosSoundMan_ns::locationHasOwnSoftMusic(const char *locationName) {
	return !scumm_stricmp(locationName, "night") || !scumm_stricmp(locationName, "intsushi");
}

void DosSoundMan_ns::playCharacterMusic(const char *character) {
	if (!character || locationHasOwnSoftMusic(_vm->_location._name)) {
		return;
	}

	char *name = const_cast<char *>(character);
	const char *newMusicFile = 0;

	if (!scumm_stricmp(name, g_dinoName)) {
		newMusicFile = "dino";
	} else
	if (!scumm_stricmp(name, g_donnaName)) {
		newMusicFile = "donna";
	} else
	if (!scumm_stricmp(name, g_doughName)) {
		newMusicFile = "nuts";
	} else {
		warning("unknown character '%s' in DosSoundMan_ns_ns::playCharacterMusic", character);
		return;
	}

	if (!_playing || (newMusicFile && scumm_stricmp(newMusicFile, _musicFile))) {
		// avoid restarting the same piece
		setMusicFile(newMusicFile);
		playMusic();
		debugC(2, kDebugExec, "changeLocation: started character specific music (%s)", newMusicFile);
	}
}

void DosSoundMan_ns::playLocationMusic(const char *location) {
	if (locationHasOwnSoftMusic(location)) {
		setMusicFile("soft");
		playMusic();
		debugC(2, kDebugExec, "changeLocation: started music 'soft'");
	} else
	if (isLocationSilent(location)) {
		stopMusic();
		debugC(2, kDebugExec, "changeLocation: music stopped");
	} else {
		playCharacterMusic(_vm->_char.getBaseName());
	}
}




#pragma mark Amiga sound manager code


#define AMIGABEEP_SIZE	16
#define NUM_REPEATS		60

static int8 res_amigaBeep[AMIGABEEP_SIZE] = {
	0, 20, 40, 60, 80, 60, 40, 20, 0, -20, -40, -60, -80, -60, -40, -20
};

AmigaSoundMan_ns::AmigaSoundMan_ns(Parallaction_ns *vm) : SoundMan_ns(vm) {
	_musicStream = 0;

	// initialize the waveform for the 'beep' sound
	beepSoundBufferSize = AMIGABEEP_SIZE * NUM_REPEATS;
	beepSoundBuffer = new int8[beepSoundBufferSize];
	int8 *odata = beepSoundBuffer;
	for (int i = 0; i < NUM_REPEATS; i++) {
		memcpy(odata, res_amigaBeep, AMIGABEEP_SIZE);
		odata += AMIGABEEP_SIZE;
	}

}

AmigaSoundMan_ns::~AmigaSoundMan_ns() {
	stopMusic();
	stopSfx(0);
	stopSfx(1);
	stopSfx(2);
	stopSfx(3);

	delete[] beepSoundBuffer;
}

Audio::AudioStream *AmigaSoundMan_ns::loadChannelData(const char *filename, Channel *ch, bool looping) {
	Audio::AudioStream *input = 0;

	if (!scumm_stricmp("beep", filename)) {
		int rate = 11934;
		ch->volume = 160;
		input = Audio::makeRawStream((byte *)beepSoundBuffer, beepSoundBufferSize, rate, 0, DisposeAfterUse::NO);
	} else {
		Common::SeekableReadStream *stream = _vm->_disk->loadSound(filename);
		input = Audio::make8SVXStream(*stream, looping);
		delete stream;
	}

	ch->stream = input;

	return input;
}

void AmigaSoundMan_ns::playSfx(const char *filename, uint channel, bool looping, int volume) {
	if (channel >= NUM_SFX_CHANNELS) {
		warning("unknown sfx channel");
		return;
	}

	stopSfx(channel);

	debugC(1, kDebugAudio, "AmigaSoundMan_ns::playSfx(%s, %i)", filename, channel);

	Channel *ch = &_channels[channel];
	Audio::AudioStream *input = loadChannelData(filename, ch, looping);

	if (volume == -1) {
		volume = ch->volume;
	}

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
}

void AmigaSoundMan_ns::stopSfx(uint channel) {
	if (channel >= NUM_SFX_CHANNELS) {
		warning("unknown sfx channel");
		return;
	}

	debugC(1, kDebugAudio, "AmigaSoundMan_ns::stopSfx(%i)", channel);
	_mixer->stopHandle(_channels[channel].handle);
	_channels[channel].stream = 0;
}

void AmigaSoundMan_ns::playMusic() {
	stopMusic();

	debugC(1, kDebugAudio, "AmigaSoundMan_ns::playMusic()");

	Common::SeekableReadStream *stream = _vm->_disk->loadMusic(_musicFile);
	_musicStream = Audio::makeProtrackerStream(stream);
	delete stream;

	debugC(3, kDebugAudio, "AmigaSoundMan_ns::playMusic(): created new music stream");

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, _musicStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, false);
}

void AmigaSoundMan_ns::stopMusic() {
	debugC(1, kDebugAudio, "AmigaSoundMan_ns::stopMusic()");

	if (_mixer->isSoundHandleActive(_musicHandle)) {
		_mixer->stopHandle(_musicHandle);
		delete _musicStream;
		_musicStream = 0;
	}
}

void AmigaSoundMan_ns::playCharacterMusic(const char *character) {
}

void AmigaSoundMan_ns::playLocationMusic(const char *location) {
}


SoundMan_ns::SoundMan_ns(Parallaction_ns *vm) : _vm(vm) {
	_mixer = _vm->_mixer;
}

void SoundMan_ns::setMusicVolume(int value) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, value);
}

void SoundMan_ns::setMusicFile(const char *filename) {
	strcpy(_musicFile, filename);
}

void SoundMan_ns::execute(int command, const char *parm = 0) {
	uint32 n = strtoul(parm, 0, 10);
	bool b = (n == 1) ? true : false;

	switch (command) {
	case SC_PLAYMUSIC:
		if (_musicType == MUSIC_CHARACTER) playCharacterMusic(parm);
		else if (_musicType == MUSIC_LOCATION) playLocationMusic(parm);
		else playMusic();
		break;
	case SC_STOPMUSIC:
		stopMusic();
		break;
	case SC_SETMUSICTYPE:
		_musicType = n;
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

} // namespace Parallaction
