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
#include "kyra/resource.h"

#include "audio/mixer.h"
#include "audio/mods/maxtrax.h"

namespace Kyra {

SoundAmiga::SoundAmiga(KyraEngine_v1 *vm, Audio::Mixer *mixer)
	: Sound(vm, mixer),
	  _driver(0),
	  _musicHandle(),
	  _fileLoaded(kFileNone),
	  _tableSfxIntro(0),
	  _tableSfxGame(0),
	  _tableSfxIntro_Size(0),
	  _tableSfxGame_Size(0) {
}

SoundAmiga::~SoundAmiga() {
	_mixer->stopHandle(_musicHandle);
	delete _driver;
}

bool SoundAmiga::init() {
	_driver = new Audio::MaxTrax(_mixer->getOutputRate(), true);

	_tableSfxIntro = _vm->staticres()->loadAmigaSfxTable(k1AmigaIntroSFXTable, _tableSfxIntro_Size);
	_tableSfxGame = _vm->staticres()->loadAmigaSfxTable(k1AmigaGameSFXTable, _tableSfxGame_Size);

	return _driver != 0 && _tableSfxIntro && _tableSfxGame;
}

void SoundAmiga::initAudioResourceInfo(int set, void *info) {
	// See comment below
}

void SoundAmiga::selectAudioResourceSet(int set) {
	// It seems that loadSoundFile() is doing what would normally be done in here.
	// As long as this driver is only required for one single target (Kyra 1 Amiga)
	// this doesn't matter much.
}

bool SoundAmiga::hasSoundFile(uint file) const {
	if (file < 3)
		return true;
	return false;
}

void SoundAmiga::loadSoundFile(uint file) {
	debugC(5, kDebugLevelSound, "SoundAmiga::loadSoundFile(%d)", file);

	static const char *const tableFilenames[3][2] = {
		{ "introscr.mx",  "introinst.mx" },
		{ "kyramusic.mx", 0 },
		{ "finalescr.mx", "introinst.mx" }
	};
	assert(file < ARRAYSIZE(tableFilenames));
	if (_fileLoaded == (FileType)file)
		return;
	const char *scoreName = tableFilenames[file][0];
	const char *sampleName = tableFilenames[file][1];
	bool loaded = false;

	Common::SeekableReadStream *scoreIn = _vm->resource()->createReadStream(scoreName);
	if (sampleName) {
		Common::SeekableReadStream *sampleIn = _vm->resource()->createReadStream(sampleName);
		if (scoreIn && sampleIn) {
			_fileLoaded = kFileNone;
			loaded = _driver->load(*scoreIn, true, false);
			loaded = loaded && _driver->load(*sampleIn, false, true);
		} else
			warning("SoundAmiga: missing atleast one of those music files: %s, %s", scoreName, sampleName);
		delete sampleIn;
	} else {
		if (scoreIn) {
			_fileLoaded = kFileNone;
			loaded = _driver->load(*scoreIn);
		} else
			warning("SoundAmiga: missing music file: %s", scoreName);
	}
	delete scoreIn;

	if (loaded)
		_fileLoaded = (FileType)file;
}

void SoundAmiga::playTrack(uint8 track) {
	debugC(5, kDebugLevelSound, "SoundAmiga::playTrack(%d)", track);

	static const byte tempoIntro[] = { 0x46, 0x55, 0x3C, 0x41 };
	static const byte tempoFinal[] = { 0x78, 0x50 };
	static const byte tempoIngame[] = {
		0x64, 0x64, 0x64, 0x64, 0x64, 0x73, 0x4B, 0x64,
		0x64, 0x64, 0x55, 0x9C, 0x6E, 0x91, 0x78, 0x84,
		0x32, 0x64, 0x64, 0x6E, 0x3C, 0xD8, 0xAF
	};
	static const byte loopIngame[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01,
		0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00
	};

	int score = -1;
	bool loop = false;
	byte volume = 0x40;
	byte tempo = 0;


	switch (_fileLoaded) {
	case kFileIntro:
		if (track >= 2 && track < ARRAYSIZE(tempoIntro) + 2) {
			score = track - 2;
			tempo = tempoIntro[score];
		}
		break;

	case kFileGame:
		if (track >= 11 && track < ARRAYSIZE(tempoIngame) + 11) {
			score = track - 11;
			loop = loopIngame[score] != 0;
			tempo = tempoIngame[score];
		}
		break;

	case kFileFinal:
		// score 0 gets started immediately after loading the music-files with different tempo.
		// we need to define a track-value for the fake call of this function
		if (track >= 2 && track < ARRAYSIZE(tempoFinal) + 2) {
			score = track - 2;
			loop = true;
			tempo = tempoFinal[score];
		}
		break;

	default:
		return;
	}

	if (score >= 0) {
		if (_musicEnabled && _driver->playSong(score, loop)) {
			_driver->setVolume(volume);
			_driver->setTempo(tempo << 4);
			if (!_mixer->isSoundHandleActive(_musicHandle))
				_mixer->playStream(Audio::Mixer::kPlainSoundType, &_musicHandle, _driver, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		}
	} else if (track == 0)
		_driver->stopMusic();
	else if (track == 1)
		beginFadeOut();
}

void SoundAmiga::haltTrack() {
	debugC(5, kDebugLevelSound, "SoundAmiga::haltTrack()");
	_driver->stopMusic();
}

void SoundAmiga::beginFadeOut() {
	debugC(5, kDebugLevelSound, "SoundAmiga::beginFadeOut()");
	for (int i = 0x3F; i >= 0; --i) {
		_driver->setVolume((byte)i);
		_vm->delay(_vm->tickLength());
	}

	_driver->stopMusic();
	_vm->delay(_vm->tickLength());
	_driver->setVolume(0x40);
}

void SoundAmiga::playSoundEffect(uint8 track, uint8) {
	debugC(5, kDebugLevelSound, "SoundAmiga::playSoundEffect(%d)", track);
	const AmigaSfxTable *sfx = 0;
	bool pan = false;

	switch (_fileLoaded) {
	case kFileFinal:
	case kFileIntro:
		// We only allow playing of sound effects, which are included in the table.
		if (track < _tableSfxIntro_Size) {
			sfx = &_tableSfxIntro[track];
			pan = (sfx->pan != 0);
		}
		break;

	case kFileGame:
		if (0x61 <= track && track <= 0x63)
			playTrack(track - 0x4F);

		if (track >= _tableSfxGame_Size)
			return;

		if (_tableSfxGame[track].note) {
			sfx = &_tableSfxGame[track];
			pan = (sfx->pan != 0) && (sfx->pan != 2);
		}

		break;

	default:
		return;
	}

	if (_sfxEnabled && sfx) {
		const bool success = _driver->playNote(sfx->note, sfx->patch, sfx->duration, sfx->volume, pan);
		if (success && !_mixer->isSoundHandleActive(_musicHandle))
			_mixer->playStream(Audio::Mixer::kPlainSoundType, &_musicHandle, _driver, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	}
}

} // End of namespace Kyra
