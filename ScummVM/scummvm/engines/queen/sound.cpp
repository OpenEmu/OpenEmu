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
#include "common/endian.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "queen/sound.h"
#include "queen/input.h"
#include "queen/logic.h"
#include "queen/music.h"
#include "queen/queen.h"
#include "queen/resource.h"

#include "audio/audiostream.h"
#include "audio/decoders/flac.h"
#include "audio/mididrv.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/vorbis.h"

#define	SB_HEADER_SIZE_V104 110
#define	SB_HEADER_SIZE_V110 122

namespace Queen {

// The sounds in the PC versions are all played at 11840 Hz. Unfortunately, we
// did not know that at the time, so there are plenty of compressed versions
// which claim that they should be played at 11025 Hz. This "wrapper" class
// works around that.

class AudioStreamWrapper : public Audio::AudioStream {
protected:
	Audio::AudioStream *_stream;
	int _rate;

public:
	AudioStreamWrapper(Audio::AudioStream *stream) {
		_stream = stream;

		int rate = _stream->getRate();

		// A file where the sample rate claims to be 11025 Hz is
		// probably compressed with the old tool. We force the real
		// sample rate, which is 11840 Hz.
		//
		// However, a file compressed with the newer tool is not
		// guaranteed to have a sample rate of 11840 Hz. LAME will
		// automatically resample it to 12000 Hz. So in all other
		// cases, we use the rate from the file.

		if (rate == 11025)
			_rate = 11840;
		else
			_rate = rate;
	}
	~AudioStreamWrapper() {
		delete _stream;
	}
	int readBuffer(int16 *buffer, const int numSamples) {
		return _stream->readBuffer(buffer, numSamples);
	}
	bool isStereo() const {
		return _stream->isStereo();
	}
	bool endOfData() const {
		return _stream->endOfData();
	}
	bool endOfStream() const {
		return _stream->endOfStream();
	}
	int getRate() const {
		return _rate;
	}
};

class SilentSound : public PCSound {
public:
	SilentSound(Audio::Mixer *mixer, QueenEngine *vm) : PCSound(mixer, vm) {}
protected:
	void playSoundData(Common::File *f, uint32 size, Audio::SoundHandle *soundHandle) {
		// Do nothing
	}
};

class SBSound : public PCSound {
public:
	SBSound(Audio::Mixer *mixer, QueenEngine *vm) : PCSound(mixer, vm) {}
protected:
	void playSoundData(Common::File *f, uint32 size, Audio::SoundHandle *soundHandle);
};

#ifdef USE_MAD
class MP3Sound : public PCSound {
public:
	MP3Sound(Audio::Mixer *mixer, QueenEngine *vm) : PCSound(mixer, vm) {}
protected:
	void playSoundData(Common::File *f, uint32 size, Audio::SoundHandle *soundHandle) {
		Common::SeekableReadStream *tmp = f->readStream(size);
		assert(tmp);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, soundHandle, new AudioStreamWrapper(Audio::makeMP3Stream(tmp, DisposeAfterUse::YES)));
	}
};
#endif

#ifdef USE_VORBIS
class OGGSound : public PCSound {
public:
	OGGSound(Audio::Mixer *mixer, QueenEngine *vm) : PCSound(mixer, vm) {}
protected:
	void playSoundData(Common::File *f, uint32 size, Audio::SoundHandle *soundHandle) {
		Common::SeekableReadStream *tmp = f->readStream(size);
		assert(tmp);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, soundHandle, new AudioStreamWrapper(Audio::makeVorbisStream(tmp, DisposeAfterUse::YES)));
	}
};
#endif

#ifdef USE_FLAC
class FLACSound : public PCSound {
public:
	FLACSound(Audio::Mixer *mixer, QueenEngine *vm) : PCSound(mixer, vm) {}
protected:
	void playSoundData(Common::File *f, uint32 size, Audio::SoundHandle *soundHandle) {
		Common::SeekableReadStream *tmp = f->readStream(size);
		assert(tmp);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, soundHandle, new AudioStreamWrapper(Audio::makeFLACStream(tmp, DisposeAfterUse::YES)));
	}
};
#endif // #ifdef USE_FLAC

Sound::Sound(Audio::Mixer *mixer, QueenEngine *vm) :
	_mixer(mixer), _vm(vm), _sfxToggle(true), _speechToggle(true), _musicToggle(true),
	_speechSfxExists(false), _lastOverride(0), _musicVolume(0) {
}

Sound *Sound::makeSoundInstance(Audio::Mixer *mixer, QueenEngine *vm, uint8 compression) {
	if (vm->resource()->getPlatform() == Common::kPlatformAmiga)
		return new AmigaSound(mixer, vm);

	switch (compression) {
	case COMPRESSION_NONE:
		return new SBSound(mixer, vm);
	case COMPRESSION_MP3:
#ifndef USE_MAD
		warning("Using MP3 compressed datafile, but MP3 support not compiled in");
		return new SilentSound(mixer, vm);
#else
		return new MP3Sound(mixer, vm);
#endif
	case COMPRESSION_OGG:
#ifndef USE_VORBIS
		warning("Using OGG compressed datafile, but OGG support not compiled in");
		return new SilentSound(mixer, vm);
#else
		return new OGGSound(mixer, vm);
#endif
	case COMPRESSION_FLAC:
#ifndef USE_FLAC
		warning("Using FLAC compressed datafile, but FLAC support not compiled in");
		return new SilentSound(mixer, vm);
#else
		return new FLACSound(mixer, vm);
#endif
	default:
		warning("Unknown compression type");
		return new SilentSound(mixer, vm);
	}
}

void Sound::setVolume(int vol) {
	if (ConfMan.hasKey("mute") && ConfMan.getBool("mute"))
		_musicVolume = 0;
	else
		_musicVolume = vol;

	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, _musicVolume);
}

void Sound::saveState(byte *&ptr) {
	WRITE_BE_UINT16(ptr, _lastOverride); ptr += 2;
}

void Sound::loadState(uint32 ver, byte *&ptr) {
	_lastOverride = (int16)READ_BE_INT16(ptr); ptr += 2;
}

PCSound::PCSound(Audio::Mixer *mixer, QueenEngine *vm)
	: Sound(mixer, vm) {

	_music = new MidiMusic(vm);
}

PCSound::~PCSound() {
	delete _music;
}

void PCSound::playSfx(uint16 sfx) {
	if (sfxOn() && sfx != 0)
		playSound(_sfxName[sfx - 1], false);
}

void PCSound::playSong(int16 songNum) {
	if (songNum <= 0) {
		_music->stopSong();
		return;
	}

	int16 newTune;
	if (_vm->resource()->isDemo()) {
		if (songNum == 17) {
			_music->stopSong();
			return;
		}
		newTune = _songDemo[songNum - 1].tuneList[0] - 1;
	} else {
		newTune = _song[songNum - 1].tuneList[0] - 1;
	}

	if (_tune[newTune].sfx[0]) {
		playSfx(_tune[newTune].sfx[0]);
		return;
	}

	if (!musicOn())
		return;

	int override = (_vm->resource()->isDemo()) ? _songDemo[songNum - 1].override : _song[songNum - 1].override;
	switch (override) {
	// Override all songs
	case  1:
		break;
	// Alter song settings (such as volume) and exit
	case  2:
		_music->toggleVChange();
	default:
		return;
	}

	_lastOverride = songNum;

	_music->queueTuneList(newTune);
	_music->playMusic();
}

void PCSound::stopSong() {
	_music->stopSong();
}

void PCSound::playSpeech(const char *base) {
	if (speechOn()) {
		playSound(base, true);
	}
}

void PCSound::setVolume(int vol) {
	Sound::setVolume(vol);
	_music->setVolume(vol);
}

void PCSound::playSound(const char *base, bool isSpeech) {
	char name[13];
	strcpy(name, base);
	// alter filename to add zeros and append ".SB"
	for (int i = 0; i < 8; i++) {
		if (name[i] == ' ')
			name[i] = '0';
	}
	strcat(name, ".SB");
	if (isSpeech) {
		while (_mixer->isSoundHandleActive(_speechHandle)) {
			_vm->input()->delay(10);
		}
	} else {
		_mixer->stopHandle(_sfxHandle);
	}
	uint32 size;
	Common::File *f = _vm->resource()->findSound(name, &size);
	if (f) {
		playSoundData(f, size, isSpeech ? &_speechHandle : &_sfxHandle);
		_speechSfxExists = isSpeech;
	} else {
		_speechSfxExists = false;
	}
}

void SBSound::playSoundData(Common::File *f, uint32 size, Audio::SoundHandle *soundHandle) {
	// In order to simplify the code, we don't parse the .sb header but hard-code the
	// values. Refer to tracker item #1876741 for details on the format/fields.
	int headerSize;
	f->seek(2, SEEK_CUR);
	uint16 version = f->readUint16LE();
	switch (version) {
	case 104:
		headerSize = SB_HEADER_SIZE_V104;
		break;
	case 110:
		headerSize = SB_HEADER_SIZE_V110;
		break;
	default:
		warning("Unhandled SB file version %d, defaulting to 104", version);
		headerSize = SB_HEADER_SIZE_V104;
		break;
	}
	f->seek(headerSize - 4, SEEK_CUR);
	size -= headerSize;
	uint8 *sound = (uint8 *)malloc(size);
	if (sound) {
		f->read(sound, size);
		Audio::Mixer::SoundType type = (soundHandle == &_speechHandle) ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType;

		Audio::AudioStream *stream = Audio::makeRawStream(sound, size, 11840, Audio::FLAG_UNSIGNED);
		_mixer->playStream(type, soundHandle, stream);
	}
}

AmigaSound::AmigaSound(Audio::Mixer *mixer, QueenEngine *vm)
	: Sound(mixer, vm), _fanfareRestore(0), _fanfareCount(0), _fluteCount(0) {
}

void AmigaSound::playSfx(uint16 sfx) {
	if (_vm->logic()->currentRoom() == 111) {
		// lightning sound
		playSound("88SSSSSS");
	}
}

void AmigaSound::playSong(int16 song) {
	debug(2, "Sound::playSong %d override %d", song, _lastOverride);

	if (song < 0) {
		stopSong();
		return;
	}

	// remap song numbers for the Amiga
	switch (song) {
	case 1:
	case 2:
		song = 39;
		break;
	case 37:
	case 52:
	case 196:
		song = 90;
		break;
	case 38:
	case 89:
		song = 3;
		break;
	case 24:
	case 158:
		song = 117;
		break;
	case 71:
	case 72:
	case 73:
	case 75:
		song = 133;
		break;
	case 203:
		song = 67;
		break;
	case 145:
		song = 140;
		break;
	case 53:
	case 204:
		song = 44;
		break;
	case 136:
	case 142:
	case 179:
		song = 86;
		break;
	case 101:
	case 102:
	case 143:
		song = 188;
		break;
	case 65:
	case 62:
		song = 69;
		break;
	case 118:
	case 119:
		song = 137;
		break;
	case 130:
	case 131:
		song = 59;
		break;
	case 174:
	case 175:
		song = 57;
		break;
	case 171:
	case 121:
		song = 137;
		break;
	case 138:
	case 170:
	case 149:
		song = 28;
		break;
	case 122:
	case 180:
	case 83:
	case 98:
		song = 83;
		break;
	case 20:
	case 33:
		song = 34;
		break;
	case 29:
	case 35:
		song = 36;
		break;
	case 7:
	case 9:
	case 10:
		song = 11;
		break;
	case 110:
		song = 94;
		break;
	case 111:
		song = 95;
		break;
	case 30:
		song = 43;
		break;
	case 76:
		song = 27;
		break;
	case 194:
	case 195:
		song = 32;
		break;
	}

	if (_lastOverride != 32 && _lastOverride != 44) {
		if (playSpecialSfx(song)) {
			return;
		}
	}

	if (_lastOverride == song && _mixer->isSoundHandleActive(_modHandle)) {
		return;
	}
	switch (song) {
	// hotel
	case 39:
		playModule("HOTEL", 1);
		break;
	case 19:
		playModule("HOTEL", 3);
		break;
	case 34:
		playModule("HOTEL", 2);
		break;
	case 36:
		playModule("HOTEL", 4);
		_fanfareRestore = _lastOverride;
		_fanfareCount = 60;
		break;
	// jungle
	case 40:
		playModule("JUNG", 1);
		_fanfareRestore = _lastOverride;
		_fanfareCount = 80;
		_fluteCount = 100;
		break;
	case 3:
		playModule("JUNG", 2);
		_fluteCount = 100;
		break;
	// temple
	case 54:
		playModule("TEMPLE", 1);
		break;
	case 12:
		playModule("TEMPLE", 2);
		break;
	case 11:
		playModule("TEMPLE", 3);
		break;
	case 31:
		playModule("TEMPLE", 4);
		_fanfareRestore = _lastOverride;
		_fanfareCount = 80;
		break;
	// floda
	case 41:
		playModule("FLODA", 4);
		_fanfareRestore = _lastOverride;
		_fanfareCount = 60;
		break;
	case 13:
		playModule("FLODA", 3);
		break;
	case 16:
		playModule("FLODA", 1);
		break;
	case 17:
		playModule("FLODA", 2);
		break;
	case 43:
		playModule("FLODA", 5);
		break;
	// end credits
	case 67:
		playModule("TITLE", 1);
		break;
	// intro credits
	case 88:
		playModule("TITLE", 1);
		break;
	// valley
	case 90:
		playModule("AWESTRUK", 1);
		break;
	// confrontation
	case 91:
		playModule("'JUNGLE'", 1);
		break;
	// Frank
	case 46:
		playModule("FRANK", 1);
		break;
	// trader bob
	case 6:
		playModule("BOB", 1);
		break;
	// azura
	case 44:
		playModule("AZURA", 1);
		break;
	// amazon fortress
	case 21:
		playModule("FORT", 1);
		break;
	// rocket
	case 32:
		playModule("ROCKET", 1);
		break;
	// robot
	case 92:
		playModule("ROBOT", 1);
		break;
	default:
		// song not available in the amiga version
		return;
	}
	_lastOverride = song;
}

void AmigaSound::stopSfx() {
	_mixer->stopHandle(_sfxHandle);
}

void AmigaSound::stopSong() {
	_mixer->stopHandle(_modHandle);
	_fanfareCount = _fluteCount = 0;
}

void AmigaSound::updateMusic() {
	if (_fanfareCount > 0) {
		--_fanfareCount;
		if (_fanfareCount == 0) {
			playSong(_fanfareRestore);
		}
	}
	if (_fluteCount > 0 && (_lastOverride == 40 || _lastOverride == 3)) {
		--_fluteCount;
		if (_fluteCount == 0) {
			playPattern("JUNG", 5 + _vm->randomizer.getRandomNumber(6));
			_fluteCount = 100;
		}
	}
}

void AmigaSound::playSound(const char *base) {
	debug(7, "AmigaSound::playSound(%s)", base);
	char soundName[20];
	sprintf(soundName, "%s.AMR", base);

	uint32 soundSize;
	Common::File *f = _vm->resource()->findSound(soundName, &soundSize);
	if (f) {
		uint8 *soundData = (uint8 *)malloc(soundSize);
		if (soundData) {
			f->read(soundData, soundSize);

			Audio::AudioStream *stream = Audio::makeRawStream(soundData, soundSize, 11025, 0);
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, stream);
		}
	}
}

Audio::AudioStream *AmigaSound::loadModule(const char *base, int num) {
	debug(7, "AmigaSound::loadModule(%s, %d)", base, num);
	char name[20];

	// load song/pattern data
	uint32 sngDataSize;
	sprintf(name, "%s.SNG", base);
	uint8 *sngData = _vm->resource()->loadFile(name, 0, &sngDataSize);
	Common::MemoryReadStream sngStr(sngData, sngDataSize);

	// load instruments/wave data
	uint32 insDataSize;
	sprintf(name, "%s.INS", base);
	uint8 *insData = _vm->resource()->loadFile(name, 0, &insDataSize);
	Common::MemoryReadStream insStr(insData, insDataSize);

	Audio::AudioStream *stream = Audio::makeRjp1Stream(&sngStr, &insStr, num, _mixer->getOutputRate());

	delete[] sngData;
	delete[] insData;

	return stream;
}

void AmigaSound::playModule(const char *base, int song) {
	_mixer->stopHandle(_modHandle);
	Audio::AudioStream *stream = loadModule(base, song);
	if (stream) {
		_mixer->playStream(Audio::Mixer::kMusicSoundType, &_modHandle, stream);
	}
	_fanfareCount = 0;
}

void AmigaSound::playPattern(const char *base, int pattern) {
	_mixer->stopHandle(_patHandle);
	Audio::AudioStream *stream = loadModule(base, -pattern);
	if (stream) {
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_patHandle, stream);
	}
}

bool AmigaSound::playSpecialSfx(int16 sfx) {
	switch (sfx) {
	case 5: // normal volume
		break;
	case 15: // soft volume
		break;
	case 14: // medium volume
		break;
	case 25: // open door
		playSound("116BSSSS");
		break;
	case 26: // close door
		playSound("105ASSSS");
		break;
	case 56: // light switch
		playSound("27SSSSSS");
		break;
	case 57: // hydraulic doors open
		playSound("96SSSSSS");
		break;
	case 58: // hydraulic doors close
		playSound("97SSSSSS");
		break;
	case 59: // metallic door slams
		playSound("105SSSSS");
		break;
	case 63: // oracle rezzes in
		playSound("132SSSSS");
		break;
	case 27: // cloth slide 1
		playSound("135SSSSS");
		break;
	case 83: // splash
		playSound("18SSSSSS");
		break;
	case 85: // agression enhancer
		playSound("138BSSSS");
		break;
	case 68: // dino ray
		playSound("138SSSSS");
		break;
	case 140: // dino transformation
		playSound("55BSSSSS");
		break;
	case 141: // experimental laser
		playSound("55SSSSSS");
		break;
	case 94: // plane hatch open
		playSound("3SSSSSSS");
		break;
	case 95: // plane hatch close
		playSound("4SSSSSSS");
		break;
	case 117: // oracle rezzes out
		playSound("70SSSSSS");
		break;
	case 124: // dino horn
		playSound("103SSSSS");
		break;
	case 127: // punch
		playSound("128SSSSS");
		break;
	case 128: // body hits ground
		playSound("129SSSSS");
		break;
	case 137: // explosion
		playSound("88SSSSSS");
		break;
	case 86: // stone door grind 1
		playSound("1001SSSS");
		break;
	case 188: // stone door grind 2
		playSound("1002SSSS");
		break;
	case 28: // cloth slide 2
		playSound("1005SSSS");
		break;
	case 151: // rattle bars
		playSound("115SSSSS");
		break;
	case 152: // door dissolves
		playSound("56SSSSSS");
		break;
	case 153: // altar slides
		playSound("85SSSSSS");
		break;
	case 166 : // pull lever
		playSound("1008SSSS");
		break;
	case 182: // zap Frank
		playSound("1023SSSS");
		break;
	case 69: // splorch
		playSound("137ASSSS");
		break;
	case 70: // robot laser
		playSound("61SSSSSS");
		break;
	case 133: // pick hits stone
		playSound("71SSSSSS");
		break;
	case 165: // press button
		playSound("1007SSSS");
		break;
	default:
		return false;
	}
	return true;
}

} //End of namespace Queen
