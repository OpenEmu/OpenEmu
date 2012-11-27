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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// sound.c - sound effects and music support

#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/config-manager.h"

#include "audio/decoders/raw.h"
#include "audio/audiostream.h"
#include "audio/midiparser.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/file.h"
#include "hugo/sound.h"
#include "hugo/text.h"

namespace Hugo {

MidiPlayer::MidiPlayer() {
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_driver = MidiDriver::createMidi(dev);
	assert(_driver);
	_paused = false;


	int ret = _driver->open();
	if (ret == 0) {
		_driver->sendGMReset();

		_driver->setTimerCallback(this, &timerCallback);
	}
}

void MidiPlayer::play(uint8 *stream, uint16 size) {
	debugC(3, kDebugMusic, "MidiPlayer::play");

	Common::StackLock lock(_mutex);

	stop();
	if (!stream)
		return;

	_midiData = (uint8 *)malloc(size);
	if (_midiData) {
		memcpy(_midiData, stream, size);

		syncVolume();	// FIXME: syncVolume calls setVolume which in turn also locks the mutex! ugh

		_parser = MidiParser::createParser_SMF();
		_parser->loadMusic(_midiData, size);
		_parser->setTrack(0);
		_parser->setMidiDriver(this);
		_parser->setTimerRate(_driver->getBaseTempo());
		_isLooping = false;
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

void MidiPlayer::sendToChannel(byte channel, uint32 b) {
	if (!_channelsTable[channel]) {
		_channelsTable[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channelsTable[channel])
			_channelsTable[channel]->volume(_channelsVolume[channel] * _masterVolume / 255);
	}

	if (_channelsTable[channel])
		_channelsTable[channel]->send(b);
}


SoundHandler::SoundHandler(HugoEngine *vm) : _vm(vm) {
	_midiPlayer = new MidiPlayer();
	_speakerStream = new Audio::PCSpeaker(_vm->_mixer->getOutputRate());
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
						_speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
	_DOSSongPtr = 0;
	_curPriority = 0;
	_pcspkrTimer = 0;
	_pcspkrOctave = 3;
	_pcspkrNoteDuration = 2;
}

SoundHandler::~SoundHandler() {
	_vm->getTimerManager()->removeTimerProc(&loopPlayer);
	_vm->_mixer->stopHandle(_speakerHandle);
	delete _speakerStream;
	delete _midiPlayer;
}

/**
 * Set the FM music volume from config.mvolume (0..100%)
 */
void SoundHandler::setMusicVolume() {
	_midiPlayer->syncVolume();
}

/**
 * Stop any sound that might be playing
 */
void SoundHandler::stopSound() {
	_vm->_mixer->stopAll();
}

/**
 * Stop any tune that might be playing
 */
void SoundHandler::stopMusic() {
	_midiPlayer->stop();
}

/**
 * Turn music on and off
 */
void SoundHandler::toggleMusic() {
	_vm->_config._musicFl = !_vm->_config._musicFl;

	_midiPlayer->pause(!_vm->_config._musicFl);
}

/**
 * Turn digitized sound on and off
 */
void SoundHandler::toggleSound() {
	_vm->_config._soundFl = !_vm->_config._soundFl;
}

void SoundHandler::playMIDI(SoundPtr seqPtr, uint16 size) {
	_midiPlayer->play(seqPtr, size);
}

/**
 * Read a tune sequence from the sound database and start playing it
 */
void SoundHandler::playMusic(int16 tune) {
	SoundPtr seqPtr;                                // Sequence data from file
	uint16 size;                                    // Size of sequence data

	if (_vm->_config._musicFl) {
		_vm->getGameStatus()._song = tune;
		seqPtr = _vm->_file->getSound(tune, &size);
		playMIDI(seqPtr, size);
		free(seqPtr);
	}
}

/**
 * Produce various sound effects on supplied stereo channel(s)
 * Override currently playing sound only if lower or same priority
 */
void SoundHandler::playSound(int16 sound, const byte priority) {
	// uint32 dwVolume;                               // Left, right volume of sound
	SoundPtr soundPtr;                                // Sound data
	uint16 size;                                      // Size of data

	// Sound disabled
	if (!_vm->_config._soundFl || !_vm->_mixer->isReady())
		return;

	syncVolume();
	_curPriority = priority;

	// Get sound data
	if ((soundPtr = _vm->_file->getSound(sound, &size)) == 0)
		return;

	Audio::AudioStream *stream = Audio::makeRawStream(soundPtr, size, 11025, Audio::FLAG_UNSIGNED);
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream);
}

/**
 * Initialize for MCI sound and midi
 */
void SoundHandler::initSound() {
	//_midiPlayer->open();
}

void SoundHandler::syncVolume() {
	int soundVolume;

	if (ConfMan.getBool("sfx_mute") || ConfMan.getBool("mute"))
		soundVolume = -1;
	else
		soundVolume = MIN(255, ConfMan.getInt("sfx_volume"));

	_vm->_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolume);
	_midiPlayer->syncVolume();
}

/**
 * Check if music is still playing.
 * If not, select the next track in the playlist and play it
 */
void SoundHandler::checkMusic() {
	if (_midiPlayer->isPlaying())
		return;

	for (int i = 0; _vm->_defltTunes[i] != -1; i++) {
		if (_vm->_defltTunes[i] == _vm->getGameStatus()._song) {
			if (_vm->_defltTunes[i + 1] != -1)
				playMusic(_vm->_defltTunes[i + 1]);
			else
				playMusic(_vm->_defltTunes[0]);
			break;
		}
	}
}

void SoundHandler::loopPlayer(void *refCon) {
	((SoundHandler *)refCon)->pcspkr_player();
}

/**
 * Decrement last note's timer and see if time to play next note yet.
 * If so, interpret next note in string and play it.  Update ptr to string
 * Timer: >0 - song still going, 0 - Stop note, -1 - Set next note
 */
void SoundHandler::pcspkr_player() {
	static const uint16 pcspkrNotes[8] =  {1352, 1205, 2274, 2026, 1805, 1704, 1518}; // The 3rd octave note counts A..G
	static const uint16 pcspkrSharps[8] = {1279, 1171, 2150, 1916, 1755, 1611, 1435}; // The sharps, A# to B#
	static const uint16 pcspkrFlats[8] =  {1435, 1279, 2342, 2150, 1916, 1755, 1611}; // The flats, Ab to Bb

	// Does the user not want any sound?
	if (!_vm->_config._soundFl || !_vm->_mixer->isReady())
		return;

	// Is there no song?
	if (!_DOSSongPtr)
		return;

	// Did we reach the end of the song?
	if (!*_DOSSongPtr)
		return;

	// Update the timer.
	_pcspkrTimer--;

	// Check the timer state..
	if (!_pcspkrTimer) {
		// A note just finished, stop the sound (if any) and return.
		_speakerStream->stop();
		return;
	} else if (_pcspkrTimer > 0) {
		// A (rest or normal) note is still playing, return.
		return;
	}

	// The timer is <0, time to play the next note.
	bool cmdNote = true;
	do {
		switch (*_DOSSongPtr) {
		case 'O':
			// Switch to new octave 0..7
			_DOSSongPtr++;
			_pcspkrOctave = *_DOSSongPtr - '0';
			if ((_pcspkrOctave < 0) || (_pcspkrOctave > 7))
				error("pcspkr_player() - Bad octave");
			_DOSSongPtr++;
			break;
		case 'L':
			// Switch to new duration (in ticks)
			_DOSSongPtr++;
			_pcspkrNoteDuration = *_DOSSongPtr - '0';
			if (_pcspkrNoteDuration < 0)
				error("pcspkr_player() - Bad duration");
			_pcspkrNoteDuration--;
			_DOSSongPtr++;
			break;
		case '<':
		case '^':
			// Move up an octave
			_DOSSongPtr++;
			_pcspkrOctave++;
			break;
		case '>':
		case 'v':
			// Move down an octave
			_DOSSongPtr++;
			_pcspkrOctave--;
			break;
		default:
			// Not a command, probably a note; so we should stop
			// processing commands and move onward now.
			cmdNote = false;
			break;
		}
	} while (cmdNote);

	switch (*_DOSSongPtr) {
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
		// Play a note.

		// First, what frequency does this note get played at?
		// We must check for sharp or flat (#, -).
		uint16 count;
		switch (_DOSSongPtr[1]) {
		case '#':
			count = pcspkrSharps[*_DOSSongPtr++ - 'A'];
			break;
		case 'b':
			count = pcspkrFlats[*_DOSSongPtr++ - 'A'];
			break;
		default:
			count = pcspkrNotes[*_DOSSongPtr - 'A'];
			break;
		}
		// Adjust for the octave if needed.
		if (_pcspkrOctave > 3)
			count /= (1 << (_pcspkrOctave - 3));
		else if (_pcspkrOctave < 3)
			count *= (1 << (3 - _pcspkrOctave));

		// Start a note playing (we will stop it when the timer expires).
		_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, kHugoCNT / count, -1);
		_pcspkrTimer = _pcspkrNoteDuration;
		_DOSSongPtr++;
		break;
	case '.':
		// Play a 'rest note' by being silent for a bit.
		_speakerStream->stop();
		_pcspkrTimer = _pcspkrNoteDuration;
		_DOSSongPtr++;
		break;
	default:
		warning("pcspkr_player() - Unhandled note");
	}
}

void SoundHandler::loadIntroSong(Common::ReadStream &in) {
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numBuf = in.readUint16BE();
		if (varnt == _vm->_gameVariant)
			_DOSIntroSong = _vm->_text->getTextData(numBuf);
	}
}

void SoundHandler::initPcspkrPlayer() {
	_vm->getTimerManager()->installTimerProc(&loopPlayer, 1000000 / _vm->_normalTPS, this, "hugoSoundLoop");
}

} // End of namespace Hugo
