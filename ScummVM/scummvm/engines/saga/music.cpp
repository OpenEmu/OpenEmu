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

// MIDI and digital music class

#include "saga/saga.h"

#include "saga/resource.h"
#include "saga/music.h"

#include "audio/audiostream.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/decoders/raw.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/substream.h"

namespace Saga {

#define BUFFER_SIZE 4096
#define MUSIC_SUNSPOT 26

MusicDriver::MusicDriver() : _isGM(false) {

	MidiPlayer::createDriver();

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	_driverType = MidiDriver::getMusicType(dev);

	int retValue = _driver->open();
	if (retValue == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		_driver->setTimerCallback(this, &timerCallback);
	}
}

void MusicDriver::send(uint32 b) {
	if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		// Remap MT32 instruments to General Midi
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}
	Audio::MidiPlayer::send(b);
}

void MusicDriver::metaEvent(byte type, byte *data, uint16 length) {
	// TODO: Seems SAGA does not want / need to handle end-of-track events?
}

void MusicDriver::play(SagaEngine *vm, ByteArray *buffer, bool loop) {
	if (buffer->size() < 4) {
		error("Music::play() wrong music resource size");
	}

	// Check if the game is using XMIDI or SMF music
	if (vm->getGameId() == GID_IHNM && vm->isMacResources()) {
		// Just set an XMIDI parser for Mac IHNM for now
		_parser = MidiParser::createParser_XMIDI();
	} else {
		if (!memcmp(buffer->getBuffer(), "FORM", 4)) {
			_parser = MidiParser::createParser_XMIDI();
			// ITE had MT32 mapped instruments
			_isGM = (vm->getGameId() != GID_ITE);
		} else {
			_parser = MidiParser::createParser_SMF();
			// ITE with standalone MIDI files is General MIDI
			_isGM = (vm->getGameId() == GID_ITE);
		}
	}

	if (!_parser->loadMusic(buffer->getBuffer(), buffer->size()))
		error("Music::play() wrong music resource");

	_parser->setTrack(0);
	_parser->setMidiDriver(this);
	_parser->setTimerRate(_driver->getBaseTempo());
	_parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);
	_parser->property(MidiParser::mpSendSustainOffOnNotesOff, 1);

	// Handle music looping
	_parser->property(MidiParser::mpAutoLoop, loop);
//	_isLooping = loop;

	_isPlaying = true;
}

void MusicDriver::pause() {
	_isPlaying = false;
}

void MusicDriver::resume() {
	_isPlaying = true;
}


Music::Music(SagaEngine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer) {
	_currentVolume = 0;
	_currentMusicBuffer = NULL;
	_player = new MusicDriver();

	_digitalMusicContext = _vm->_resource->getContext(GAME_DIGITALMUSICFILE);
	if (!_player->isAdlib())
		_musicContext = _vm->_resource->getContext(GAME_MUSICFILE_GM);
	else
		_musicContext = _vm->_resource->getContext(GAME_MUSICFILE_FM);

	if (!_musicContext) {
		if (_vm->getGameId() == GID_ITE) {
			_musicContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
		} else {
			// I've listened to music from both the FM and the GM
			// file, and I've tentatively reached the conclusion
			// that they are both General MIDI. My guess is that
			// the FM file has been reorchestrated to sound better
			// on AdLib and other FM synths.
			//
			// Sev says the AdLib music does not sound like in the
			// original, but I still think assuming General MIDI is
			// the right thing to do. Some music, like the End
			// Title (song 0) sound absolutely atrocious when piped
			// through our MT-32 to GM mapping.
			//
			// It is, however, quite possible that the original
			// used a different GM to FM mapping. If the original
			// sounded markedly better, perhaps we should add some
			// way of replacing our stock mapping in adlib.cpp?
			//
			// For the composer's own recording of the End Title,
			// see http://www.johnottman.com/

			// Oddly enough, the intro music (song 1) is very
			// different in the two files. I have no idea why.
			// Note that the IHNM demo has only got one music file
			// (music.rsc). It is assumed that it contains FM music
			_musicContext = _vm->_resource->getContext(GAME_MUSICFILE_FM);
		}
	}

	_digitalMusic = false;
}

Music::~Music() {
	_vm->getTimerManager()->removeTimerProc(&musicVolumeGaugeCallback);
	_mixer->stopHandle(_musicHandle);
	delete _player;
}

void Music::musicVolumeGaugeCallback(void *refCon) {
	((Music *)refCon)->musicVolumeGauge();
}

void Music::musicVolumeGauge() {
	int volume;

	_currentVolumePercent += 10;

	if (_currentVolume - _targetVolume > 0) { // Volume decrease
		volume = _targetVolume + (_currentVolume - _targetVolume) * (100 - _currentVolumePercent) / 100;
	} else {
		volume = _currentVolume + (_targetVolume - _currentVolume) * _currentVolumePercent / 100;
	}

	if (volume < 0)
		volume = 1;

	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
	_player->setVolume(volume);

	if (_currentVolumePercent == 100) {
		_vm->getTimerManager()->removeTimerProc(&musicVolumeGaugeCallback);
		_currentVolume = _targetVolume;
	}
}

void Music::setVolume(int volume, int time) {
	_targetVolume = volume;
	_currentVolumePercent = 0;

	if (volume == -1) // Set Full volume
		volume = 255;

	if (time == 1) {
		if (ConfMan.hasKey("mute") && ConfMan.getBool("mute"))
			volume = 0;

		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
		_player->setVolume(volume);
		_vm->getTimerManager()->removeTimerProc(&musicVolumeGaugeCallback);
		_currentVolume = volume;
		return;
	}

	_vm->getTimerManager()->installTimerProc(&musicVolumeGaugeCallback, time * 3000L, this, "sagaMusicVolume");
}

bool Music::isPlaying() {
	return _mixer->isSoundHandleActive(_musicHandle) || _player->isPlaying();
}

void Music::play(uint32 resourceId, MusicFlags flags) {
	Audio::SeekableAudioStream *audioStream = NULL;
	uint32 loopStart = 0;

	debug(2, "Music::play %d, %d", resourceId, flags);

	if (isPlaying() && _trackNumber == resourceId) {
		return;
	}

	_trackNumber = resourceId;
	_mixer->stopHandle(_musicHandle);
	_player->stop();

	int realTrackNumber;

	if (_vm->getGameId() == GID_ITE) {
		if (flags == MUSIC_DEFAULT) {
			if (resourceId == 13 || resourceId == 19) {
				flags = MUSIC_NORMAL;
			} else {
				flags = MUSIC_LOOP;
			}
		}
		realTrackNumber = resourceId - 8;
	} else {
		realTrackNumber = resourceId + 1;
	}

	// Try to open standalone digital track
	char trackName[2][16];
	sprintf(trackName[0], "track%d", realTrackNumber);
	sprintf(trackName[1], "track%02d", realTrackNumber);
	Audio::SeekableAudioStream *stream = 0;
	for (int i = 0; i < 2; ++i) {
		stream = Audio::SeekableAudioStream::openStreamFile(trackName[i]);
		if (stream) {
			_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle,
			                        Audio::makeLoopingAudioStream(stream, (flags == MUSIC_LOOP) ? 0 : 1));
			_digitalMusic = true;
			return;
		}
	}

	if (_vm->getGameId() == GID_ITE) {
		if (resourceId >= 9 && resourceId <= 34) {
			if (_digitalMusicContext != NULL) {
				loopStart = 0;
				// Fix ITE sunstatm/sunspot score
				if (resourceId == MUSIC_SUNSPOT)
					loopStart = 18727;

				// Digital music
				ResourceData *resData = _digitalMusicContext->getResourceData(resourceId - 9);
				Common::File *musicFile = _digitalMusicContext->getFile(resData);
				int offs = (_digitalMusicContext->isCompressed()) ? 9 : 0;

				Common::SeekableSubReadStream *musicStream = new Common::SeekableSubReadStream(musicFile,
							(uint32)resData->offset + offs, (uint32)resData->offset + resData->size - offs);

				if (!_digitalMusicContext->isCompressed()) {
					byte musicFlags = Audio::FLAG_STEREO |
										Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;

					if (_vm->isBigEndian())
						musicFlags &= ~Audio::FLAG_LITTLE_ENDIAN;

					// The newer ITE Mac demo version contains a music file, but it has mono music.
					// This is the only music file that is about 7MB, whereas all the other ones
					// are much larger. Thus, we use this simple heuristic to determine if we got
					// mono music in the ITE demos or not.
					if (!strcmp(_digitalMusicContext->fileName(), "musicd.rsc") &&
						_digitalMusicContext->fileSize() < 8000000)
						musicFlags &= ~Audio::FLAG_STEREO;

					audioStream = Audio::makeRawStream(musicStream, 11025, musicFlags, DisposeAfterUse::YES);
				} else {
					// Read compressed header to determine compression type
					musicFile->seek((uint32)resData->offset, SEEK_SET);
					byte identifier = musicFile->readByte();

					if (identifier == 0) {		// MP3
#ifdef USE_MAD
						audioStream = Audio::makeMP3Stream(musicStream, DisposeAfterUse::YES);
#endif
					} else if (identifier == 1) {	// OGG
#ifdef USE_VORBIS
						audioStream = Audio::makeVorbisStream(musicStream, DisposeAfterUse::YES);
#endif
					} else if (identifier == 2) {	// FLAC
#ifdef USE_FLAC
						audioStream = Audio::makeFLACStream(musicStream, DisposeAfterUse::YES);
#endif
					}
				}

				if (!audioStream)
					delete musicStream;
			}
		}
	}

	if (audioStream) {
		debug(2, "Playing digitized music");
		if (loopStart) {
			_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle,
			                        new Audio::SubLoopingAudioStream(audioStream,
			                        (flags == MUSIC_LOOP ? 0 : 1),
			                        Audio::Timestamp(0, loopStart, audioStream->getRate()),
			                        audioStream->getLength()));
		} else {
			_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle,
			                        Audio::makeLoopingAudioStream(audioStream, (flags == MUSIC_LOOP ? 0 : 1)));
		}
		_digitalMusic = true;
		return;
	}

	if (flags == MUSIC_DEFAULT)
		flags = MUSIC_NORMAL;

	// Load MIDI/XMI resource data
	if (_vm->getGameId() == GID_IHNM && _vm->isMacResources()) {
		// Load the external music file for Mac IHNM
#if 0
		Common::File musicFile;
		char musicFileName[40];
		sprintf(musicFileName, "Music/Music%02x", resourceId);
		musicFile.open(musicFileName);
		resourceSize = musicFile.size();
		resourceData = new byte[resourceSize];
		musicFile.read(resourceData, resourceSize);
		musicFile.close();

		// TODO: The Mac music format is unsupported (QuickTime MIDI)
		// so stop here
#endif
		return;
	} else {
		if (_currentMusicBuffer == &_musicBuffer[1]) {
			_currentMusicBuffer = &_musicBuffer[0];
		} else {
			_currentMusicBuffer = &_musicBuffer[1];
		}
		_vm->_resource->loadResource(_musicContext, resourceId, *_currentMusicBuffer);
	}

	_player->play(_vm, _currentMusicBuffer, (flags & MUSIC_LOOP));
	setVolume(_vm->_musicVolume);
}

void Music::pause() {
	_player->pause();
	_player->setVolume(0);
}

void Music::resume() {
	_player->resume();
	_player->setVolume(_vm->_musicVolume);
}

void Music::stop() {
	_player->stop();
}

} // End of namespace Saga
