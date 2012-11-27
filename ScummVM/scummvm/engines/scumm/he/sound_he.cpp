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

#include "scumm/actor.h"
#include "scumm/file.h"
#include "scumm/imuse/imuse.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/he/sound_he.h"
#include "scumm/he/intern_he.h"
#include "scumm/util.h"

#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/timer.h"
#include "common/util.h"

#include "audio/decoders/adpcm.h"
#include "audio/decoders/flac.h"
#include "audio/mididrv.h"
#include "audio/mixer.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"

namespace Scumm {

SoundHE::SoundHE(ScummEngine *parent, Audio::Mixer *mixer)
	:
	Sound(parent, mixer),
	_vm((ScummEngine_v60he *)parent),
	_overrideFreq(0),
	_heMusic(0),
	_heMusicTracks(0) {

	memset(_heChannel, 0, sizeof(_heChannel));
}

SoundHE::~SoundHE() {
	free(_heMusic);
}

void SoundHE::addSoundToQueue(int sound, int heOffset, int heChannel, int heFlags) {
	if (_vm->VAR_LAST_SOUND != 0xFF)
		_vm->VAR(_vm->VAR_LAST_SOUND) = sound;

	if ((_vm->_game.heversion <= 99 && (heFlags & 16)) || (_vm->_game.heversion >= 100 && (heFlags & 8))) {
		playHESound(sound, heOffset, heChannel, heFlags);
		return;
	} else {

		Sound::addSoundToQueue(sound, heOffset, heChannel, heFlags);
	}
}

void SoundHE::addSoundToQueue2(int sound, int heOffset, int heChannel, int heFlags) {
	int i = _soundQue2Pos;
	while (i--) {
		if (_soundQue2[i].sound == sound && !(heFlags & 2))
			return;
	}

	Sound::addSoundToQueue2(sound, heOffset, heChannel, heFlags);
}

void SoundHE::processSoundQueues() {
	int snd, heOffset, heChannel, heFlags;

	if (_vm->_game.heversion >= 72) {
		for (int i = 0; i <_soundQue2Pos; i++) {
			snd = _soundQue2[i].sound;
			heOffset = _soundQue2[i].offset;
			heChannel = _soundQue2[i].channel;
			heFlags = _soundQue2[i].flags;
			if (snd)
				playHESound(snd, heOffset, heChannel, heFlags);
		}
		_soundQue2Pos = 0;
	} else {
		while (_soundQue2Pos) {
			_soundQue2Pos--;
			snd = _soundQue2[_soundQue2Pos].sound;
			heOffset = _soundQue2[_soundQue2Pos].offset;
			heChannel = _soundQue2[_soundQue2Pos].channel;
			heFlags = _soundQue2[_soundQue2Pos].flags;
			if (snd)
				playHESound(snd, heOffset, heChannel, heFlags);
		}
	}

	Sound::processSoundQueues();
}

int SoundHE::isSoundRunning(int sound) const {
	if (_vm->_game.heversion >= 70) {
		if (sound >= 10000) {
			return _mixer->getSoundID(_heSoundChannels[sound - 10000]);
		}
	} else if (_vm->_game.heversion >= 60) {
		if (sound == -2) {
			sound = _heChannel[0].sound;
		} else if (sound == -1) {
			sound = _currentMusic;
		}
	}

	if (_mixer->isSoundIDActive(sound))
		return sound;

	if (isSoundInQueue(sound))
		return sound;

	if (_vm->_musicEngine &&_vm->_musicEngine->getSoundStatus(sound))
		return sound;

	return 0;
}

void SoundHE::stopSound(int sound) {
	if (_vm->_game.heversion >= 70) {
		if ( sound >= 10000) {
			stopSoundChannel(sound - 10000);
		}
	} else if (_vm->_game.heversion >= 60) {
		if (sound == -2) {
			sound = _heChannel[0].sound;
		} else if (sound == -1) {
			sound = _currentMusic;
		}
	}

	Sound::stopSound(sound);

	for (int i = 0; i < ARRAYSIZE(_heChannel); i++) {
		if (_heChannel[i].sound == sound) {
			_heChannel[i].sound = 0;
			_heChannel[i].priority = 0;
			_heChannel[i].rate = 0;
			_heChannel[i].timer = 0;
			_heChannel[i].sbngBlock = 0;
			_heChannel[i].codeOffs = 0;
			memset(_heChannel[i].soundVars, 0, sizeof(_heChannel[i].soundVars));
		}
	}

	if (_vm->_game.heversion >= 70 && sound == 1) {
		_vm->_haveMsg = 3;
		_vm->_talkDelay = 0;
	}
}

void SoundHE::stopAllSounds() {
	// Clear sound channels for HE games
	memset(_heChannel, 0, sizeof(_heChannel));

	Sound::stopAllSounds();
}

void SoundHE::setupSound() {
	Sound::setupSound();

	if (_vm->_game.heversion >= 70) {
		setupHEMusicFile();
	}
}

void SoundHE::stopSoundChannel(int chan) {
	if (_heChannel[chan].sound == 1) {
		_vm->_haveMsg = 3;
		_vm->_talkDelay = 0;
	}

	_mixer->stopHandle(_heSoundChannels[chan]);

	_heChannel[chan].sound = 0;
	_heChannel[chan].priority = 0;
	_heChannel[chan].rate = 0;
	_heChannel[chan].timer = 0;
	_heChannel[chan].sbngBlock = 0;
	_heChannel[chan].codeOffs = 0;
	memset(_heChannel[chan].soundVars, 0, sizeof(_heChannel[chan].soundVars));

	for (int i = 0; i < ARRAYSIZE(_soundQue2); i++) {
		if (_soundQue2[i].channel == chan) {
			_soundQue2[i].sound = 0;
			_soundQue2[i].offset = 0;
			_soundQue2[i].channel = 0;
			_soundQue2[i].flags = 0;
		}
	}
}

int SoundHE::findFreeSoundChannel() {
	int chan, min;

	min = _vm->VAR(_vm->VAR_RESERVED_SOUND_CHANNELS);
	if (min == 0) {
		_vm->VAR(_vm->VAR_RESERVED_SOUND_CHANNELS) = 8;
		return 1;
	}

	if (min < 8) {
		for (chan = min; chan < ARRAYSIZE(_heChannel); chan++) {
			if (_mixer->isSoundHandleActive(_heSoundChannels[chan]) == 0)
				return chan;
		}
	} else {
		return 1;
	}

	return min;
}

int SoundHE::isSoundCodeUsed(int sound) {
	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1 && _mixer->isSoundHandleActive(_heSoundChannels[chan])) {
		return _heChannel[chan].sbngBlock;
	} else {
		return 0;
	}
}

int SoundHE::getSoundPos(int sound) {
	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1 && _mixer->isSoundHandleActive(_heSoundChannels[chan])) {
		int time =  _vm->getHETimer(chan + 4) * _heChannel[chan].rate / 1000;
		return time;
	} else {
		return 0;
	}
}

int SoundHE::getSoundVar(int sound, int var) {
	if (_vm->_game.heversion >= 90 && var == 26) {
		return isSoundCodeUsed(sound);
	}

	assertRange(0, var, 25, "sound variable");

	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1 && _mixer->isSoundHandleActive(_heSoundChannels[chan])) {
		debug(5, "getSoundVar: sound %d var %d result %d", sound, var, _heChannel[chan].soundVars[var]);
		return _heChannel[chan].soundVars[var];
	} else {
		return 0;
	}
}

void SoundHE::setSoundVar(int sound, int var, int val) {
	assertRange(0, var, 25, "sound variable");

	int chan = -1;
	for (int i = 0; i < ARRAYSIZE(_heChannel); i ++) {
		if (_heChannel[i].sound == sound)
			chan = i;
	}

	if (chan != -1) {
		debug(5, "setSoundVar: sound %d var %d val %d", sound, var, val);
		_heChannel[chan].soundVars[var] = val;
	}
}

void SoundHE::setOverrideFreq(int freq) {
	_overrideFreq = freq;
}

void SoundHE::setupHEMusicFile() {
	int i;
	Common::File musicFile;
	Common::String buf(_vm->generateFilename(-4));

	if (musicFile.open(buf) == true) {
		musicFile.seek(4, SEEK_SET);
		/*int total_size =*/ musicFile.readUint32BE();
		musicFile.seek(16, SEEK_SET);
		_heMusicTracks = musicFile.readUint32LE();
		debug(5, "Total music tracks %d", _heMusicTracks);

		int musicStart = (_vm->_game.heversion >= 80) ? 56 : 20;
		musicFile.seek(musicStart, SEEK_SET);

		_heMusic = (HEMusic *)malloc((_heMusicTracks + 1) * sizeof(HEMusic));
		for (i = 0; i < _heMusicTracks; i++) {
			_heMusic[i].id = musicFile.readUint32LE();
			_heMusic[i].offset = musicFile.readUint32LE();
			_heMusic[i].size = musicFile.readUint32LE();

			if (_vm->_game.heversion >= 80) {
				musicFile.seek(+9, SEEK_CUR);
			} else {
				musicFile.seek(+13, SEEK_CUR);
			}
		}

		musicFile.close();
	}
}

bool SoundHE::getHEMusicDetails(int id, int &musicOffs, int &musicSize) {
	int i;

	for (i = 0; i < _heMusicTracks; i++) {
		if (_heMusic[i].id == id) {
			musicOffs = _heMusic[i].offset;
			musicSize = _heMusic[i].size;
			return 1;
		}
	}

	return 0;
}

void SoundHE::processSoundCode() {
	byte *codePtr;
	int chan, tmr, size, time;

	for (chan = 0; chan < ARRAYSIZE(_heChannel); chan++) {
		if (_heChannel[chan].sound == 0) {
			continue;
		}

		if (_heChannel[chan].codeOffs == -1) {
			continue;
		}

		tmr = _vm->getHETimer(chan + 4) * _heChannel[chan].rate / 1000;
		tmr += _vm->VAR(_vm->VAR_SOUNDCODE_TMR);
		if (tmr < 0)
			tmr = 0;

		if (_heChannel[chan].sound > _vm->_numSounds) {
			codePtr = _vm->getResourceAddress(rtSpoolBuffer, chan);
		} else {
			codePtr = _vm->getResourceAddress(rtSound, _heChannel[chan].sound);
		}
		assert(codePtr);
		codePtr += _heChannel[chan].codeOffs;

		while (1) {
			size = READ_LE_UINT16(codePtr);
			time = READ_LE_UINT32(codePtr + 2);

			if (size == 0) {
				_heChannel[chan].codeOffs = -1;
				break;
			}

			debug(5, "Channel %d Timer %d Time %d", chan, tmr, time);
			if (time >= tmr)
				break;

			processSoundOpcodes(_heChannel[chan].sound, codePtr + 6, _heChannel[chan].soundVars);

			codePtr += size;
			_heChannel[chan].codeOffs += size;
		}
	}

	for (chan = 0; chan < ARRAYSIZE(_heChannel); chan++) {
		if (_heChannel[chan].sound == 0)
			continue;

		if (_heChannel[chan].timer == 0)
			continue;

		if (_vm->getHETimer(chan + 4) > _heChannel[chan].timer) {
			if (_heChannel[chan].sound == 1) {
				_vm->stopTalk();
			}

			_heChannel[chan].sound = 0;
			_heChannel[chan].priority = 0;
			_heChannel[chan].rate = 0;
			_heChannel[chan].timer = 0;
			_heChannel[chan].sbngBlock = 0;
			_heChannel[chan].codeOffs = 0;
			_heChannel[chan].soundVars[0] = 0;
		}
	}
}

void SoundHE::processSoundOpcodes(int sound, byte *codePtr, int *soundVars) {
	int arg, opcode, var, val;

	while (READ_LE_UINT16(codePtr) != 0) {
		codePtr += 2;
		opcode = READ_LE_UINT16(codePtr); codePtr += 2;
		opcode = (opcode & 0xFFF) >> 4;
		arg = opcode & 3;
		opcode &= ~3;
		debug(5, "processSoundOpcodes: sound %d opcode %d", sound, opcode);
		switch (opcode) {
		case 0: // Continue
			break;
		case 16: // Set talk state
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			setSoundVar(sound, 19, val);
			break;
		case 32: // Set var
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			setSoundVar(sound, var, val);
			break;
		case 48: // Add
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) + val;
			setSoundVar(sound, var, val);
			break;
		case 56: // Subtract
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) - val;
			setSoundVar(sound, var, val);
			break;
		case 64: // Multiple
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) * val;
			setSoundVar(sound, var, val);
			break;
		case 80: // Divide
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = READ_LE_UINT16(codePtr); codePtr += 2;
			if (arg == 2) {
				val = getSoundVar(sound, val);
			}
			val = getSoundVar(sound, var) / val;
			setSoundVar(sound, var, val);
			break;
		case 96: // Increment
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = getSoundVar(sound, var) + 1;
			setSoundVar(sound, var, val);
			break;
		case 104: // Decrement
			var = READ_LE_UINT16(codePtr); codePtr += 2;
			val = getSoundVar(sound, var) - 1;
			setSoundVar(sound, var, val);
			break;
		default:
			error("Illegal sound %d opcode %d", sound, opcode);
		}
	}
}

byte *findSoundTag(uint32 tag, byte *ptr) {
	byte *endPtr;
	uint32 offset, size;

	if (READ_BE_UINT32(ptr) == MKTAG('W','S','O','U')) {
		ptr += 8;
	}

	if (READ_BE_UINT32(ptr) != MKTAG('R','I','F','F'))
		return NULL;

	endPtr = (ptr + 12);
	size = READ_LE_UINT32(ptr + 4);

	while (endPtr < ptr + size) {
		offset = READ_LE_UINT32(endPtr + 4);

		if (offset <= 0)
			error("Illegal chunk length - %d bytes.", offset);

		if (offset > size)
			error("Chunk extends beyond file end - %d versus %d.", offset, size);

		if (READ_BE_UINT32(endPtr) == tag)
			return endPtr;

		endPtr = endPtr + offset + 8;
	}

	return NULL;
}

void SoundHE::playHESound(int soundID, int heOffset, int heChannel, int heFlags) {
	Audio::RewindableAudioStream *stream = 0;
	byte *ptr, *spoolPtr;
	int size = -1;
	int priority, rate;
	byte flags = Audio::FLAG_UNSIGNED;

	Audio::Mixer::SoundType type = Audio::Mixer::kSFXSoundType;
	if (soundID > _vm->_numSounds)
		type = Audio::Mixer::kMusicSoundType;
	else if (soundID == 1)
		type = Audio::Mixer::kSpeechSoundType;


	if (heChannel == -1)
		heChannel = (_vm->VAR_RESERVED_SOUND_CHANNELS != 0xFF) ? findFreeSoundChannel() : 1;

	debug(5,"playHESound: soundID %d heOffset %d heChannel %d heFlags %d", soundID, heOffset, heChannel, heFlags);

	if (soundID >= 10000) {
		// Special codes, used in pjgames
		return;
	}

	if (soundID > _vm->_numSounds) {
		int music_offs;
		Common::File musicFile;
		Common::String buf(_vm->generateFilename(-4));

		if (musicFile.open(buf) == false) {
			warning("playHESound: Can't open music file %s", buf.c_str());
			return;
		}
		if (!getHEMusicDetails(soundID, music_offs, size)) {
			debug(0, "playHESound: musicID %d not found", soundID);
			return;
		}

		musicFile.seek(music_offs, SEEK_SET);

		_mixer->stopHandle(_heSoundChannels[heChannel]);
		spoolPtr = _vm->_res->createResource(rtSpoolBuffer, heChannel, size);
		assert(spoolPtr);
		musicFile.read(spoolPtr, size);
		musicFile.close();

		if (_vm->_game.heversion == 70) {
			stream = Audio::makeRawStream(spoolPtr, size, 11025, flags, DisposeAfterUse::NO);
			_mixer->playStream(type, &_heSoundChannels[heChannel], stream, soundID);
			return;
		}
	}

	if (soundID > _vm->_numSounds) {
		ptr = _vm->getResourceAddress(rtSpoolBuffer, heChannel);
	} else {
		ptr = _vm->getResourceAddress(rtSound, soundID);
	}

	if (!ptr) {
		return;
	}

	// Support for sound in later HE games
	if (READ_BE_UINT32(ptr) == MKTAG('R','I','F','F') || READ_BE_UINT32(ptr) == MKTAG('W','S','O','U')) {
		uint16 compType;
		int blockAlign;
		int codeOffs = -1;

		priority = (soundID > _vm->_numSounds) ? 255 : *(ptr + 18);

		byte *sbngPtr = findSoundTag(MKTAG('S','B','N','G'), ptr);
		if (sbngPtr != NULL) {
			codeOffs = sbngPtr - ptr + 8;
		}

		if (_mixer->isSoundHandleActive(_heSoundChannels[heChannel])) {
			int curSnd = _heChannel[heChannel].sound;
			if (curSnd == 1 && soundID != 1)
				return;
			if (curSnd != 0 && curSnd != 1 && soundID != 1 && _heChannel[heChannel].priority > priority)
				return;
		}

		if (READ_BE_UINT32(ptr) == MKTAG('W','S','O','U'))
			ptr += 8;

		size = READ_LE_UINT32(ptr + 4);
		Common::MemoryReadStream memStream(ptr, size);

		if (!Audio::loadWAVFromStream(memStream, size, rate, flags, &compType, &blockAlign)) {
			error("playHESound: Not a valid WAV file (%d)", soundID);
		}

		assert(heOffset >= 0 && heOffset < size);

		// FIXME: Disabled sound offsets, due to asserts been triggered
		heOffset = 0;

		_vm->setHETimer(heChannel + 4);
		_heChannel[heChannel].sound = soundID;
		_heChannel[heChannel].priority = priority;
		_heChannel[heChannel].rate = rate;
		_heChannel[heChannel].sbngBlock = (codeOffs != -1) ? 1 : 0;
		_heChannel[heChannel].codeOffs = codeOffs;
		memset(_heChannel[heChannel].soundVars, 0, sizeof(_heChannel[heChannel].soundVars));

		// TODO: Extra sound flags
		if (heFlags & 1) {
			_heChannel[heChannel].timer = 0;
		} else {
			_heChannel[heChannel].timer = size * 1000 / rate;
		}

		_mixer->stopHandle(_heSoundChannels[heChannel]);
		if (compType == 17) {
			Audio::AudioStream *voxStream = Audio::makeADPCMStream(&memStream, DisposeAfterUse::NO, size, Audio::kADPCMMSIma, rate, (flags & Audio::FLAG_STEREO) ? 2 : 1, blockAlign);

			// FIXME: Get rid of this crude hack to turn a ADPCM stream into a raw stream.
			// It seems it is only there to allow looping -- if that is true, we certainly
			// can do without it, using a LoopingAudioStream.

			byte *sound = (byte *)malloc(size * 4);
			/* On systems where it matters, malloc will return
			 * even addresses, so the use of (void *) in the
			 * following cast shuts the compiler from warning
			 * unnecessarily. */
			size = voxStream->readBuffer((int16 *)(void *)sound, size * 2);
			size *= 2; // 16bits.
			delete voxStream;

			_heChannel[heChannel].rate = rate;
			if (_heChannel[heChannel].timer)
				_heChannel[heChannel].timer = size * 1000 / rate;

			// makeADPCMStream returns a stream in native endianness, but RawMemoryStream
			// defaults to big endian. If we're on a little endian system, set the LE flag.
#ifdef SCUMM_LITTLE_ENDIAN
			flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif
			stream = Audio::makeRawStream(sound + heOffset, size - heOffset, rate, flags);
		} else {
			stream = Audio::makeRawStream(ptr + memStream.pos() + heOffset, size - heOffset, rate, flags, DisposeAfterUse::NO);
		}
		_mixer->playStream(type, &_heSoundChannels[heChannel],
						Audio::makeLoopingAudioStream(stream, (heFlags & 1) ? 0 : 1), soundID);
	}
	// Support for sound in Humongous Entertainment games
	else if (READ_BE_UINT32(ptr) == MKTAG('D','I','G','I') || READ_BE_UINT32(ptr) == MKTAG('T','A','L','K')) {
		byte *sndPtr = ptr;
		int codeOffs = -1;

		priority = (soundID > _vm->_numSounds) ? 255 : *(ptr + 18);
		rate = READ_LE_UINT16(ptr + 22);

		// Skip DIGI/TALK (8) and HSHD (24) blocks
		ptr += 32;

		if (_mixer->isSoundHandleActive(_heSoundChannels[heChannel])) {
			int curSnd = _heChannel[heChannel].sound;
			if (curSnd == 1 && soundID != 1)
				return;
			if (curSnd != 0 && curSnd != 1 && soundID != 1 && _heChannel[heChannel].priority > priority)
				return;
		}

		if (READ_BE_UINT32(ptr) == MKTAG('S','B','N','G')) {
			codeOffs = ptr - sndPtr + 8;
			ptr += READ_BE_UINT32(ptr + 4);
		}

		assert(READ_BE_UINT32(ptr) == MKTAG('S','D','A','T'));
		size = READ_BE_UINT32(ptr + 4) - 8;
		if (heOffset < 0 || heOffset > size) {
			// Occurs when making fireworks in puttmoon
			heOffset = 0;
		}
		size -= heOffset;

		if (_overrideFreq) {
			// Used by the piano in Fatty Bear's Birthday Surprise
			rate = _overrideFreq;
			_overrideFreq = 0;
		}

		_vm->setHETimer(heChannel + 4);
		_heChannel[heChannel].sound = soundID;
		_heChannel[heChannel].priority = priority;
		_heChannel[heChannel].rate = rate;
		_heChannel[heChannel].sbngBlock = (codeOffs != -1) ? 1 : 0;
		_heChannel[heChannel].codeOffs = codeOffs;
		memset(_heChannel[heChannel].soundVars, 0, sizeof(_heChannel[heChannel].soundVars));

		// TODO: Extra sound flags
		if (heFlags & 1) {
			_heChannel[heChannel].timer = 0;
		} else {
			_heChannel[heChannel].timer = size * 1000 / rate;
		}

		_mixer->stopHandle(_heSoundChannels[heChannel]);

		stream = Audio::makeRawStream(ptr + heOffset + 8, size, rate, flags, DisposeAfterUse::NO);
		_mixer->playStream(type, &_heSoundChannels[heChannel],
						Audio::makeLoopingAudioStream(stream, (heFlags & 1) ? 0 : 1), soundID);
	}
	// Support for PCM music in 3DO versions of Humongous Entertainment games
	else if (READ_BE_UINT32(ptr) == MKTAG('M','R','A','W')) {
		priority = *(ptr + 18);
		rate = READ_LE_UINT16(ptr + 22);

		// Skip DIGI (8) and HSHD (24) blocks
		ptr += 32;

		assert(READ_BE_UINT32(ptr) == MKTAG('S','D','A','T'));
		size = READ_BE_UINT32(ptr + 4) - 8;

		byte *sound = (byte *)malloc(size);
		memcpy(sound, ptr + 8, size);

		_mixer->stopID(_currentMusic);
		_currentMusic = soundID;

		stream = Audio::makeRawStream(sound, size, rate, 0);
		_mixer->playStream(Audio::Mixer::kMusicSoundType, NULL, stream, soundID);
	}
	else if (READ_BE_UINT32(ptr) == MKTAG('M','I','D','I')) {
		if (_vm->_imuse) {
			// This is used in the DOS version of Fatty Bear's
			// Birthday Surprise to change the note on the piano
			// when not using a digitized instrument.
			_vm->_imuse->stopSound(_currentMusic);
			_currentMusic = soundID;
			_vm->_imuse->startSoundWithNoteOffset(soundID, heOffset);
		}
	}
}

void SoundHE::startHETalkSound(uint32 offset) {
	byte *ptr;
	int32 size;

	if (ConfMan.getBool("speech_mute"))
		return;

	if (_sfxFilename.empty()) {
		// This happens in the Pajama Sam's Lost & Found demo, on the
		// main menu screen, so don't make it a fatal error.
		warning("startHETalkSound: Speech file is not found");
		return;
	}

	ScummFile file;
	if (!_vm->openFile(file, _sfxFilename)) {
		warning("startHETalkSound: Could not open speech file %s", _sfxFilename.c_str());
		return;
	}
	file.setEnc(_sfxFileEncByte);

	_sfxMode |= 2;
	_vm->_res->nukeResource(rtSound, 1);

	file.seek(offset + 4, SEEK_SET);
	 size = file.readUint32BE();
	file.seek(offset, SEEK_SET);

	_vm->_res->createResource(rtSound, 1, size);
	ptr = _vm->getResourceAddress(rtSound, 1);
	file.read(ptr, size);

	int channel = (_vm->VAR_TALK_CHANNEL != 0xFF) ? _vm->VAR(_vm->VAR_TALK_CHANNEL) : 0;
	addSoundToQueue2(1, 0, channel, 0);
}

#ifdef ENABLE_HE
void ScummEngine_v80he::createSound(int snd1id, int snd2id) {
	byte *snd1Ptr, *snd2Ptr;
	byte *sbng1Ptr, *sbng2Ptr;
	byte *sdat1Ptr, *sdat2Ptr;
	byte *src, *dst, *tmp;
	int len, offs, size;
	int sdat1size, sdat2size;

	sbng1Ptr = NULL;
	sbng2Ptr = NULL;

	if (snd2id == -1) {
		_sndPtrOffs = 0;
		_sndTmrOffs = 0;
		_sndDataSize = 0;
		return;
	}

	if (snd1id != _curSndId) {
		_curSndId = snd1id;
		_sndPtrOffs = 0;
		_sndTmrOffs = 0;
		_sndDataSize = 0;
	}

	snd1Ptr = getResourceAddress(rtSound, snd1id);
	assert(snd1Ptr);
	snd2Ptr = getResourceAddress(rtSound, snd2id);
	assert(snd2Ptr);

	int i;
	int chan = -1;
	for (i = 0; i < ARRAYSIZE(((SoundHE *)_sound)->_heChannel); i++) {
		if (((SoundHE *)_sound)->_heChannel[i].sound == snd1id)
			chan =  i;
	}

	if (!findSoundTag(MKTAG('d','a','t','a'), snd1Ptr)) {
		sbng1Ptr = heFindResource(MKTAG('S','B','N','G'), snd1Ptr);
		sbng2Ptr = heFindResource(MKTAG('S','B','N','G'), snd2Ptr);
	}

	if (sbng1Ptr != NULL && sbng2Ptr != NULL) {
		if (chan != -1 && ((SoundHE *)_sound)->_heChannel[chan].codeOffs > 0) {
			int curOffs = ((SoundHE *)_sound)->_heChannel[chan].codeOffs;

			src = snd1Ptr + curOffs;
			dst = sbng1Ptr + 8;
			size = READ_BE_UINT32(sbng1Ptr + 4);
			len = sbng1Ptr - snd1Ptr + size - curOffs;

			byte *data = (byte *)malloc(len);
			memcpy(data, src, len);
			memcpy(dst, data, len);
			free(data);

			dst = sbng1Ptr + 8;
			while ((size = READ_LE_UINT16(dst)) != 0)
				dst += size;
		} else {
			dst = sbng1Ptr + 8;
		}

		((SoundHE *)_sound)->_heChannel[chan].codeOffs = sbng1Ptr - snd1Ptr + 8;

		tmp = sbng2Ptr + 8;
		while ((offs = READ_LE_UINT16(tmp)) != 0) {
			tmp += offs;
		}

		src = sbng2Ptr + 8;
		len = tmp - sbng2Ptr - 6;
		memcpy(dst, src, len);

		int32 time;
		while ((size = READ_LE_UINT16(dst)) != 0) {
			time = READ_LE_UINT32(dst + 2);
			time += _sndTmrOffs;
			WRITE_LE_UINT32(dst + 2, time);
			dst += size;
		}
	}

	if (findSoundTag(MKTAG('d','a','t','a'), snd1Ptr)) {
		sdat1Ptr = findSoundTag(MKTAG('d','a','t','a'), snd1Ptr);
		assert(sdat1Ptr);
		sdat2Ptr = findSoundTag(MKTAG('d','a','t','a'), snd2Ptr);
		assert(sdat2Ptr);

		if (!_sndDataSize)
			_sndDataSize = READ_LE_UINT32(sdat1Ptr + 4) - 8;

		sdat2size = READ_LE_UINT32(sdat2Ptr + 4) - 8;
	} else {
		sdat1Ptr = heFindResource(MKTAG('S','D','A','T'), snd1Ptr);
		assert(sdat1Ptr);
		sdat2Ptr = heFindResource(MKTAG('S','D','A','T'), snd2Ptr);
		assert(sdat2Ptr);

		_sndDataSize = READ_BE_UINT32(sdat1Ptr + 4) - 8;

		sdat2size = READ_BE_UINT32(sdat2Ptr + 4) - 8;
	}

	sdat1size = _sndDataSize - _sndPtrOffs;
	if (sdat2size < sdat1size) {
		src = sdat2Ptr + 8;
		dst = sdat1Ptr + 8 + _sndPtrOffs;
		len = sdat2size;

		memcpy(dst, src, len);

		_sndPtrOffs += sdat2size;
		_sndTmrOffs += sdat2size;
	} else {
		src = sdat2Ptr + 8;
		dst = sdat1Ptr + 8 + _sndPtrOffs;
		len = sdat1size;

		memcpy(dst, src, len);

		if (sdat2size != sdat1size) {
			src = sdat2Ptr + 8 + sdat1size;
			dst = sdat1Ptr + 8;
			len = sdat2size - sdat1size;

			memcpy(dst, src, len);
		}

		_sndPtrOffs = sdat2size - sdat1size;
		_sndTmrOffs += sdat2size;
	}
}
#endif

} // End of namespace Scumm
