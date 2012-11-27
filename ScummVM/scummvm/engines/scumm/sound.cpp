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
#include "common/timer.h"
#include "common/util.h"
#include "common/ptr.h"
#include "common/substream.h"

#include "scumm/actor.h"
#include "scumm/file.h"
#include "scumm/imuse/imuse.h"
#include "scumm/imuse_digi/dimuse.h"
#include "scumm/player_towns.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/util.h"

#include "backends/audiocd/audiocd.h"

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

struct MP3OffsetTable {					/* Compressed Sound (.SO3) */
	int org_offset;
	int new_offset;
	int num_tags;
	int compressed_size;
};


Sound::Sound(ScummEngine *parent, Audio::Mixer *mixer)
	:
	_vm(parent),
	_mixer(mixer),
	_soundQuePos(0),
	_soundQue2Pos(0),
	_sfxFilename(),
	_sfxFileEncByte(0),
	_offsetTable(0),
	_numSoundEffects(0),
	_soundMode(kVOCMode),
	_talk_sound_a1(0),
	_talk_sound_a2(0),
	_talk_sound_b1(0),
	_talk_sound_b2(0),
	_talk_sound_mode(0),
	_talk_sound_channel(0),
	_mouthSyncMode(false),
	_endOfMouthSync(false),
	_curSoundPos(0),
	_currentCDSound(0),
	_currentMusic(0),
	_lastSound(0),
	_soundsPaused(false),
	_sfxMode(0) {

	memset(_soundQue, 0, sizeof(_soundQue));
	memset(_soundQue2, 0, sizeof(_soundQue2));
	memset(_mouthSyncTimes, 0, sizeof(_mouthSyncTimes));

	_musicType = MDT_NONE;
}

Sound::~Sound() {
	stopCDTimer();
	g_system->getAudioCDManager()->stop();
	free(_offsetTable);
}

void Sound::addSoundToQueue(int sound, int heOffset, int heChannel, int heFlags) {
	if (_vm->VAR_LAST_SOUND != 0xFF)
		_vm->VAR(_vm->VAR_LAST_SOUND) = sound;
	_lastSound = sound;

	// HE music resources are in separate file
	if (sound <= _vm->_numSounds)
		_vm->ensureResourceLoaded(rtSound, sound);

	addSoundToQueue2(sound, heOffset, heChannel, heFlags);
}

void Sound::addSoundToQueue2(int sound, int heOffset, int heChannel, int heFlags) {
	assert(_soundQue2Pos < ARRAYSIZE(_soundQue2));
	_soundQue2[_soundQue2Pos].sound = sound;
	_soundQue2[_soundQue2Pos].offset = heOffset;
	_soundQue2[_soundQue2Pos].channel = heChannel;
	_soundQue2[_soundQue2Pos].flags = heFlags;
	_soundQue2Pos++;
}

void Sound::processSound() {
	if (_vm->_game.version >= 7) {
		processSfxQueues();
	} else if (_vm->_game.heversion >= 80) {
		processSoundQueues();
	} else {
		processSfxQueues();
		processSoundQueues();
	}
}

void Sound::processSoundQueues() {
	int i = 0, num;
	int snd;
	int data[16];

	while (_soundQue2Pos) {
		_soundQue2Pos--;
		snd = _soundQue2[_soundQue2Pos].sound;
		if (snd)
			playSound(snd);
	}

	while (i < _soundQuePos) {
		num = _soundQue[i++];
		if (i + num > _soundQuePos) {
			error("processSoundQues: invalid num value");
			break;
		}
		memset(data, 0, sizeof(data));
		if (num > 0) {
			for (int j = 0; j < num; j++)
				data[j] = _soundQue[i + j];
			i += num;

			debugC(DEBUG_IMUSE, "processSoundQues(%d,%d,%d,%d,%d,%d,%d,%d,%d)",
						data[0] >> 8, data[0] & 0xFF,
						data[1], data[2], data[3], data[4], data[5], data[6], data[7]);

			if (_vm->_townsPlayer)
				_vm->VAR(_vm->VAR_SOUNDRESULT) = (short)_vm->_townsPlayer->doCommand(num, data);
			else if (_vm->_imuse)
				_vm->VAR(_vm->VAR_SOUNDRESULT) = (short)_vm->_imuse->doCommand(num, data);
		}
	}
	_soundQuePos = 0;
}

void Sound::playSound(int soundID) {
	byte *ptr;
	byte *sound;
	Audio::AudioStream *stream;
	int size = -1;
	int rate;

	if (_vm->_game.id == GID_LOOM && _vm->_game.platform == Common::kPlatformPCEngine) {
		if (soundID >= 13 && soundID <= 32) {
			static const char tracks[20] = {3, 4, 5, 7, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 21, 19, 20, 21};

			_currentCDSound = soundID;

			// The original game had hard-coded lengths for all
			// tracks, but this one track is the only one (as far
			// as we know) where this actually matters. See bug
			// #3024173 - LOOM-PCE: Music stops prematurely.

			int track = tracks[soundID - 13];
			if (track == 6) {
				playCDTrack(track, 1, 0, 260);
			} else {
				playCDTrack(track, 1, 0, 0);
			}
		} else {
			if (_vm->_musicEngine) {
				_vm->_musicEngine->startSound(soundID);
			}
		}
		return;
	}

	debugC(DEBUG_SOUND, "playSound #%d", soundID);

	ptr = _vm->getResourceAddress(rtSound, soundID);

	if (!ptr) {
		return;
	}

	// Support for SFX in Monkey Island 1, Mac version
	// This is rather hackish right now, but works OK. SFX are not sounding
	// 100% correct, though, not sure right now what is causing this.
	else if (READ_BE_UINT32(ptr) == MKTAG('M','a','c','1')) {
		// Read info from the header
		size = READ_BE_UINT32(ptr+0x60);
		rate = READ_BE_UINT16(ptr+0x64);

		// Skip over the header (fixed size)
		ptr += 0x72;

		// Allocate a sound buffer, copy the data into it, and play
		sound = (byte *)malloc(size);
		memcpy(sound, ptr, size);

		stream = Audio::makeRawStream(sound, size, rate, Audio::FLAG_UNSIGNED);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, NULL, stream, soundID);
	}
	// WORKAROUND bug # 1311447
	else if (READ_BE_UINT32(ptr) == 0x460e200d) {
		// This sound resource occurs in the Macintosh version of Monkey Island.
		// I do now know whether it is used in any place other than the one
		// mentioned in the bug report above; in case it is, I put a check here.
		assert(soundID == 39);

		// The samplerate is copied from the sound resouce 39 of the PC CD/VGA
		// version of Monkey Island.

		// Read info from the header
		size = READ_BE_UINT32(ptr+4);
		rate = 6849;

		// Skip over the header (fixed size)
		ptr += 0x26;

		// Allocate a sound buffer, copy the data into it, and play
		sound = (byte *)malloc(size);
		memcpy(sound, ptr, size);
		stream = Audio::makeRawStream(sound, size, rate, Audio::FLAG_UNSIGNED);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, NULL, stream, soundID);
	}
	// Support for sampled sound effects in Monkey Island 1 and 2
	else if (_vm->_game.platform != Common::kPlatformFMTowns
	         // The Macintosh m68k versions of MI2/Indy4 just ignore SBL effects.
	         && !_vm->isMacM68kIMuse()
	         && READ_BE_UINT32(ptr) == MKTAG('S','B','L',' ')) {
		debugC(DEBUG_SOUND, "Using SBL sound effect");

		// SBL resources essentially contain VOC sound data.
		// There are at least two main variants: in one,
		// there are two subchunks AUhd and AUdt, in the other
		// the chunks are called WVhd and WVdt. Besides that,
		// the two variants seem pretty similiar.

		// The first subchunk (AUhd resp. WVhd) seems to always
		// contain three bytes (00 00 80) of unknown meaning.
		// After that, a second subchunk contains VOC data.
		// Two real examples:
		//
		// 53 42 4c 20 00 00 11 ae  |SBL ....|
		// 41 55 68 64 00 00 00 03  |AUhd....|
		// 00 00 80 41 55 64 74 00  |...AUdt.|
		// 00 11 9b 01 96 11 00 a6  |........|
		// 00 7f 7f 7e 7e 7e 7e 7e  |...~~~~~|
		// 7e 7f 7f 80 80 7f 7f 7f  |~.......|
		// 7f 80 80 7f 7e 7d 7d 7e  |....~}}~|
		// 7e 7e 7e 7e 7e 7e 7e 7f  |~~~~~~~.|
		//
		// And from the non-interactive Sam & Max demo:
		//
		// 53 42 4c 20 00 01 15 6e  |SBL ...n|
		// 57 56 68 64 00 00 00 03  |WVhd....|
		// 00 00 80 57 56 64 74 00  |...WVdt.|
		// 01 15 5b 01 56 15 01 a6  |..[.V...|
		// 00 80 80 80 80 80 80 80  |........|
		// 80 80 80 80 80 80 80 80  |........|
		// 80 80 80 80 80 80 80 80  |........|
		// 80 80 80 80 80 80 80 80  |........|

		size = READ_BE_UINT32(ptr + 4) - 27;
		ptr += 27;

		// Fingolfin says: after eyeballing a single SEGA
		// SBL resource, it would seem as if the content of the
		// data subchunk (AUdt) is XORed with 0x16. At least
		// then a semi-sane VOC header is revealed, with
		// a sampling rate of ~25000 Hz (does that make sense?).
		// I'll add some code to test that theory for now.

		// Check if the resource has already been demangled
		if ((_vm->_game.platform == Common::kPlatformSegaCD) && (ptr[0] != 1))	{
			for (int i = 0; i < size; i++)   {
				ptr[i] ^= 0x16;
				if (ptr[i] >= 0x7F) {
					ptr[i] = 0xFE - ptr[i];
					ptr[i] ^= 0x80;
				}
			}
		}

		// TODO: It would be nice if we could use readVOCFromMemory() here.
		// We'd have to add the 'Creative Voice File' header for this, though,
		// or make readVOCFromMemory() less strict.

		Audio::VocBlockHeader &voc_block_hdr = *(Audio::VocBlockHeader *)ptr;
		assert(voc_block_hdr.blocktype == 1);
		size = voc_block_hdr.size[0] + (voc_block_hdr.size[1] << 8) + (voc_block_hdr.size[2] << 16) - 2;
		rate = Audio::getSampleRateFromVOCRate(voc_block_hdr.sr);
		assert(voc_block_hdr.pack == 0);

		// Allocate a sound buffer, copy the data into it, and play
		sound = (byte *)malloc(size);
		memcpy(sound, ptr + 6, size);
		stream = Audio::makeRawStream(sound, size, rate, Audio::FLAG_UNSIGNED);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, NULL, stream, soundID);
	}
	else if (_vm->_game.platform != Common::kPlatformFMTowns && READ_BE_UINT32(ptr) == MKTAG('S','O','U','N')) {
		if (_vm->_game.version != 3)
			ptr += 2;

		int type = *(ptr + 0x0D);

		if (type == 2) {
			// CD track resource
			ptr += 0x16;
			if (soundID == _currentCDSound && pollCD() == 1)
				return;

			int track = ptr[0];
			int loops = ptr[1];
			int start = (ptr[2] * 60 + ptr[3]) * 75 + ptr[4];
			int end = (ptr[5] * 60 + ptr[6]) * 75 + ptr[7];

			playCDTrack(track, loops == 0xff ? -1 : loops, start, end <= start ? 0 : end - start);
			_currentCDSound = soundID;
		} else {
			// All other sound types are ignored
			warning("Scumm::Sound::playSound: encountered audio resoure with chunk type 'SOUN' and sound type %d", type);
		}
	}
	else if ((_vm->_game.id == GID_LOOM) && (_vm->_game.platform == Common::kPlatformMacintosh))  {
		// Mac version of Loom uses yet another sound format
		/*
		playSound #9 (room 70)
		000000: 55 00 00 45  73 6f 00 64  01 00 00 00  00 00 00 00   |U..Eso.d........|
		000010: 00 05 00 8e  2a 8f 2d 1c  2a 8f 2a 8f  2d 1c 00 28   |....*.-.*.*.-..(|
		000020: 00 31 00 3a  00 43 00 4c  00 01 00 00  00 01 00 64   |.1.:.C.L.......d|
		000030: 5a 00 01 00  00 00 01 00  64 00 00 01  00 00 00 01   |Z.......d.......|
		000040: 00 64 5a 00  01 00 00 00  01 00 64 5a  00 01 00 00   |.dZ.......dZ....|
		000050: 00 01 00 64  00 00 00 00  00 00 00 07  00 00 00 64   |...d...........d|
		000060: 64 00 00 4e  73 6f 00 64  01 00 00 00  00 00 00 00   |d..Nso.d........|
		000070: 00 05 00 89  3d 57 2d 1c  3d 57 3d 57  2d 1c 00 28   |....=W-.=W=W-..(|
		playSound #16 (room 69)
		000000: dc 00 00 a5  73 6f 00 64  01 00 00 00  00 00 00 00   |....so.d........|
		000010: 00 05 00 00  2a 8f 03 e8  03 e8 03 e8  03 e8 00 28   |....*..........(|
		000020: 00 79 00 7f  00 85 00 d6  00 01 00 00  00 19 01 18   |.y..............|
		000030: 2f 00 18 00  01 18 32 00  18 00 01 18  36 00 18 00   |/.....2.....6...|
		000040: 01 18 3b 00  18 00 01 18  3e 00 18 00  01 18 42 00   |..;.....>.....B.|
		000050: 18 00 01 18  47 00 18 00  01 18 4a 00  18 00 01 18   |....G.....J.....|
		000060: 4e 00 10 00  01 18 53 00  10 00 01 18  56 00 10 00   |N.....S.....V...|
		000070: 01 18 5a 00  10 00 02 28  5f 00 01 00  00 00 00 00   |..Z....(_.......|
		*/
	}
	else if ((_vm->_game.platform == Common::kPlatformMacintosh) && (_vm->_game.id == GID_INDY3) && READ_BE_UINT16(ptr + 8) == 0x1C) {
		// Sound format as used in Indy3 EGA Mac.
		// It seems to be closely related to the Amiga format, see player_v3a.cpp
		// The following is known:
		// offset 0, 16 LE: total size
		// offset 2-7: ?
		// offset 8, 16BE: offset to sound data (0x1C = 28 -> header size 28?)
		// offset 10-11: ? another offset, maybe related to looping?
		// offset 12, 16BE: size of sound data
		// offset 14-15: ? often the same as 12-13: maybe loop size/end?
		// offset 16-19: ? all 0?
		// offset 20, 16BE: rate divisor
		// offset 22-23: ? often identical to the rate divisior? (but not in sound 8, which loops)
		// offset 24, byte (?): volume
		// offset 25: ? same as volume -- maybe left vs. right channel?
		// offset 26: ?  if != 0: stop current sound?
		// offset 27: ?  loopcount? 0xff == -1 for infinite?

		size = READ_BE_UINT16(ptr + 12);
		assert(size);

		rate = 3579545 / READ_BE_UINT16(ptr + 20);
		sound = (byte *)malloc(size);
		int vol = ptr[24] * 4;
		int loopStart = 0, loopEnd = 0;
		int loopcount = ptr[27];

		memcpy(sound, ptr + READ_BE_UINT16(ptr + 8), size);
		Audio::SeekableAudioStream *plainStream = Audio::makeRawStream(sound, size, rate, 0);

		if (loopcount > 1) {
			loopStart = READ_BE_UINT16(ptr + 10) - READ_BE_UINT16(ptr + 8);
			loopEnd = READ_BE_UINT16(ptr + 14);

			// TODO: Currently we will only ever play till "loopEnd", even when we only have
			// a finite repetition count.
			stream = new Audio::SubLoopingAudioStream(plainStream, loopcount == 255 ? 0 : loopcount, Audio::Timestamp(0, loopStart, rate), Audio::Timestamp(0, loopEnd, rate));
		} else {
			stream = plainStream;
		}

		_mixer->playStream(Audio::Mixer::kSFXSoundType, NULL, stream, soundID, vol, 0);
	}
	else {

		if (_vm->_game.id == GID_MONKEY_VGA || _vm->_game.id == GID_MONKEY_EGA
			|| (_vm->_game.id == GID_MONKEY && _vm->_game.platform == Common::kPlatformMacintosh)) {
			// Works around the fact that in some places in MonkeyEGA/VGA,
			// the music is never explicitly stopped.
			// Rather it seems that starting a new music is supposed to
			// automatically stop the old song.
			if (_vm->_imuse) {
				if (READ_BE_UINT32(ptr) != MKTAG('A','S','F','X'))
					_vm->_imuse->stopAllSounds();
			}
		}

		if (_vm->_musicEngine) {
			_vm->_musicEngine->startSound(soundID);
		}

		if (_vm->_townsPlayer)
			_currentCDSound = _vm->_townsPlayer->getCurrentCdaSound();
	}
}

void Sound::processSfxQueues() {

	if (_talk_sound_mode != 0) {
		if (_talk_sound_mode & 1)
			startTalkSound(_talk_sound_a1, _talk_sound_b1, 1);
		if (_talk_sound_mode & 2)
			startTalkSound(_talk_sound_a2, _talk_sound_b2, 2, &_talkChannelHandle);
		_talk_sound_mode = 0;
	}

	const int act = _vm->getTalkingActor();
	if ((_sfxMode & 2) && act != 0) {
		Actor *a;
		bool finished;

		if (_vm->_imuseDigital) {
			finished = !isSoundRunning(kTalkSoundID);
		} else if (_vm->_game.heversion >= 60) {
			finished = !isSoundRunning(1);
		} else {
			finished = !_mixer->isSoundHandleActive(_talkChannelHandle);
		}

		if ((uint) act < 0x80 && ((_vm->_game.version == 8) || (_vm->_game.version <= 7 && !_vm->_string[0].no_talk_anim))) {
			a = _vm->derefActor(act, "processSfxQueues");
			if (a->isInCurrentRoom()) {
				if (isMouthSyncOff(_curSoundPos) && !_mouthSyncMode) {
					if (!_endOfMouthSync)
						a->runActorTalkScript(a->_talkStopFrame);
					_mouthSyncMode = 0;
				} else  if (isMouthSyncOff(_curSoundPos) == 0 && !_mouthSyncMode) {
					a->runActorTalkScript(a->_talkStartFrame);
					_mouthSyncMode = 1;
				}

				if (_vm->_game.version <= 6 && finished)
					a->runActorTalkScript(a->_talkStopFrame);
			}
		}

		if ((!ConfMan.getBool("subtitles") && finished) || (finished && _vm->_talkDelay == 0)) {
			if (!(_vm->_game.version == 8 && _vm->VAR(_vm->VAR_HAVE_MSG) == 0))
				_vm->stopTalk();
		}
	}

	if (_sfxMode & 1) {
		if (isSfxFinished()) {
			_sfxMode &= ~1;
		}
	}
}

static int compareMP3OffsetTable(const void *a, const void *b) {
	return ((const MP3OffsetTable *)a)->org_offset - ((const MP3OffsetTable *)b)->org_offset;
}

void Sound::startTalkSound(uint32 offset, uint32 b, int mode, Audio::SoundHandle *handle) {
	int num = 0, i;
	int size = 0;
	int id = -1;
	Common::ScopedPtr<ScummFile> file;

	if (_vm->_game.id == GID_CMI) {
		_sfxMode |= mode;
		return;
	} else if (_vm->_game.id == GID_DIG) {
		_sfxMode |= mode;
		if (!(_vm->_game.features & GF_DEMO))
			return;

		char filename[30];
		char roomname[10];

		if (offset == 1)
			strcpy(roomname, "logo");
		else if (offset == 15)
			strcpy(roomname, "canyon");
		else if (offset == 17)
			strcpy(roomname, "pig");
		else if (offset == 18)
			strcpy(roomname, "derelict");
		else if (offset == 19)
			strcpy(roomname, "wreck");
		else if (offset == 20)
			strcpy(roomname, "grave");
		else if (offset == 23)
			strcpy(roomname, "nexus");
		else if (offset == 79)
			strcpy(roomname, "newton");
		else {
			warning("startTalkSound: dig demo: unknown room number: %d", offset);
			return;
		}

		file.reset(new ScummFile());
		if (!file)
			error("startTalkSound: Out of memory");

		sprintf(filename, "audio/%s.%u/%u.voc", roomname, offset, b);
		if (!_vm->openFile(*file, filename)) {
			sprintf(filename, "audio/%s_%u/%u.voc", roomname, offset, b);
			_vm->openFile(*file, filename);
		}

		if (!file->isOpen()) {
			sprintf(filename, "%u.%u.voc", offset, b);
			_vm->openFile(*file, filename);
		}

		if (!file->isOpen()) {
			warning("startTalkSound: dig demo: voc file not found");
			return;
		}
	} else {

		if (_sfxFilename.empty()) {
			warning("startTalkSound: SFX file not found");
			return;
		}

		// Some games frequently assume that starting one sound effect will
		// automatically stop any other that may be playing at that time. So
		// that is what we do here, but we make an exception for speech.

		if (mode == 1 && (_vm->_game.id == GID_TENTACLE || _vm->_game.id == GID_SAMNMAX)) {
			id = 777777 + _talk_sound_channel;
			_mixer->stopID(id);
		}

		if (b > 8) {
			num = (b - 8) >> 1;
		}

		if (_offsetTable != NULL) {
			MP3OffsetTable *result = NULL, key;

			key.org_offset = offset;
			result = (MP3OffsetTable *)bsearch(&key, _offsetTable, _numSoundEffects,
													sizeof(MP3OffsetTable), compareMP3OffsetTable);

			if (result == NULL) {
				warning("startTalkSound: did not find sound at offset %d", offset);
				return;
			}
			if (2 * num != result->num_tags) {
				warning("startTalkSound: number of tags do not match (%d - %d)", b,
								result->num_tags);
				num = result->num_tags;
			}
			offset = result->new_offset;
			size = result->compressed_size;
		} else {
			offset += 8;
			size = -1;
		}

		file.reset(new ScummFile());
		if (!file)
			error("startTalkSound: Out of memory");

		if (!_vm->openFile(*file, _sfxFilename)) {
			warning("startTalkSound: could not open sfx file %s", _sfxFilename.c_str());
			return;
		}

		file->setEnc(_sfxFileEncByte);
		file->seek(offset, SEEK_SET);

		assert(num + 1 < (int)ARRAYSIZE(_mouthSyncTimes));
		for (i = 0; i < num; i++)
			_mouthSyncTimes[i] = file->readUint16BE();

		// Adjust offset to account for the mouth sync times. It is noteworthy
		// that we do not adjust the size here for compressed streams, since
		// they only set size to the size of the compressed sound data.
		offset += num * 2;
		// TODO: In case we ever set up the size for VOC streams, we should
		// really check whether the size contains the _mouthSyncTimes.
		//if (_soundMode == kVOCMode)
		//	size -= num * 2;

		_mouthSyncTimes[i] = 0xFFFF;
		_sfxMode |= mode;
		_curSoundPos = 0;
		_mouthSyncMode = true;
	}

	if (!_soundsPaused && _mixer->isReady()) {
		Audio::AudioStream *input = NULL;

		switch (_soundMode) {
		case kMP3Mode:
#ifdef USE_MAD
			{
			assert(size > 0);
			input = Audio::makeMP3Stream(new Common::SeekableSubReadStream(file.release(), offset, offset + size, DisposeAfterUse::YES), DisposeAfterUse::YES);
			}
#endif
			break;
		case kVorbisMode:
#ifdef USE_VORBIS
			{
			assert(size > 0);
			input = Audio::makeVorbisStream(new Common::SeekableSubReadStream(file.release(), offset, offset + size, DisposeAfterUse::YES), DisposeAfterUse::YES);
			}
#endif
			break;
		case kFLACMode:
#ifdef USE_FLAC
			{
			assert(size > 0);
			input = Audio::makeFLACStream(new Common::SeekableSubReadStream(file.release(), offset, offset + size, DisposeAfterUse::YES), DisposeAfterUse::YES);
			}
#endif
			break;
		default:
			input = Audio::makeVOCStream(file.release(), Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
			break;
		}

		if (!input) {
			warning("startSfxSound failed to load sound");
			return;
		}

		if (_vm->_imuseDigital) {
#ifdef ENABLE_SCUMM_7_8
			//_vm->_imuseDigital->stopSound(kTalkSoundID);
			_vm->_imuseDigital->startVoice(kTalkSoundID, input);
#endif
		} else {
			_mixer->playStream(Audio::Mixer::kSpeechSoundType, handle, input, id);
		}
	}
}

void Sound::stopTalkSound() {
	if (_sfxMode & 2) {
		if (_vm->_imuseDigital) {
#ifdef ENABLE_SCUMM_7_8
			_vm->_imuseDigital->stopSound(kTalkSoundID);
#endif
		} else if (_vm->_game.heversion >= 60) {
			stopSound(1);
		} else {
			_mixer->stopHandle(_talkChannelHandle);
		}
		_sfxMode &= ~2;
	}
}

bool Sound::isMouthSyncOff(uint pos) {
	uint j;
	bool val = true;
	uint16 *ms = _mouthSyncTimes;

	_endOfMouthSync = false;
	do {
		val = !val;
		j = *ms++;
		if (j == 0xFFFF) {
			_endOfMouthSync = true;
			break;
		}
	} while (pos > j);
	return val;
}

int Sound::isSoundRunning(int sound) const {
#ifdef ENABLE_SCUMM_7_8
	if (_vm->_imuseDigital)
		return (_vm->_imuseDigital->getSoundStatus(sound) != 0);
#endif

	if (sound == _currentCDSound)
		return pollCD();

	if (_mixer->isSoundIDActive(sound))
		return 1;

	if (isSoundInQueue(sound))
		return 1;

	if (sound > _vm->_numSounds || !_vm->_res->isResourceLoaded(rtSound, sound))
		return 0;

	if (_vm->_musicEngine)
		return _vm->_musicEngine->getSoundStatus(sound);

	return 0;
}

/**
 * Check whether the sound resource with the specified ID is still
 * used. This is invoked by ScummEngine::isResourceInUse, to determine
 * which resources can be expired from memory.
 * Technically, this works very similar to isSoundRunning, however it
 * calls IMuse::get_sound_active() instead of IMuse::getSoundStatus().
 * The difference between those two is in how they treat sounds which
 * are being faded out: get_sound_active() returns true even when the
 * sound is being faded out, while getSoundStatus() returns false in
 * that case.
 */
bool Sound::isSoundInUse(int sound) const {

#ifdef ENABLE_SCUMM_7_8
	if (_vm->_imuseDigital)
		return (_vm->_imuseDigital->getSoundStatus(sound) != 0);
#endif

	if (sound == _currentCDSound)
		return pollCD() != 0;

	if (isSoundInQueue(sound))
		return true;

	if (!_vm->_res->isResourceLoaded(rtSound, sound))
		return false;

	if (_vm->_imuse)
		return _vm->_imuse->get_sound_active(sound);

	if (_mixer->isSoundIDActive(sound))
		return 1;

	return false;
}

bool Sound::isSoundInQueue(int sound) const {
	int i, num;

	i = _soundQue2Pos;
	while (i--) {
		if (_soundQue2[i].sound == sound)
			return true;
	}

	i = 0;
	while (i < _soundQuePos) {
		num = _soundQue[i++];

		if (num > 0) {
			if (_soundQue[i + 0] == 0x10F && _soundQue[i + 1] == 8 && _soundQue[i + 2] == sound)
				return true;
			i += num;
		}
	}
	return false;
}

void Sound::stopSound(int sound) {
	int i;

	if (sound != 0 && sound == _currentCDSound) {
		_currentCDSound = 0;
		stopCD();
		stopCDTimer();
	}

	if (_vm->_game.version < 7)
		_mixer->stopID(sound);

	if (_vm->_musicEngine)
		_vm->_musicEngine->stopSound(sound);

	for (i = 0; i < ARRAYSIZE(_soundQue2); i++) {
		if (_soundQue2[i].sound == sound) {
			_soundQue2[i].sound = 0;
			_soundQue2[i].offset = 0;
			_soundQue2[i].channel = 0;
			_soundQue2[i].flags = 0;
		}
	}
}

void Sound::stopAllSounds() {
	if (_currentCDSound != 0) {
		_currentCDSound = 0;
		stopCD();
		stopCDTimer();
	}

	// Clear the (secondary) sound queue
	_lastSound = 0;
	_soundQue2Pos = 0;
	memset(_soundQue2, 0, sizeof(_soundQue2));

	if (_vm->_musicEngine) {
		_vm->_musicEngine->stopAllSounds();
	}

	// Stop all SFX
	if (!_vm->_imuseDigital) {
		_mixer->stopAll();
	}
}

void Sound::soundKludge(int *list, int num) {
	int i;

#ifdef ENABLE_SCUMM_7_8
	if (_vm->_imuseDigital) {
		_vm->_imuseDigital->parseScriptCmds(list[0], list[1], list[2], list[3], list[4],
												list[5], list[6], list[7]);
		return;
	}
#endif

	if (list[0] == -1) {
		processSound();
	} else {
		_soundQue[_soundQuePos++] = num;

		for (i = 0; i < num; i++) {
			_soundQue[_soundQuePos++] = list[i];
		}
	}
}

void Sound::talkSound(uint32 a, uint32 b, int mode, int channel) {
	if (_vm->_game.version >= 5 && ConfMan.getBool("speech_mute"))
		return;

	if (mode == 1) {
		_talk_sound_a1 = a;
		_talk_sound_b1 = b;
		_talk_sound_channel = channel;
	} else {
		_talk_sound_a2 = a;
		_talk_sound_b2 = b;
	}

	_talk_sound_mode |= mode;
}

void Sound::setupSound() {
	setupSfxFile();

	if (_vm->_game.id == GID_FT) {
		_vm->VAR(_vm->VAR_VOICE_BUNDLE_LOADED) = _sfxFilename.empty() ? 0 : 1;
	}
}

void Sound::pauseSounds(bool pause) {
	if (_vm->_imuse)
		_vm->_imuse->pause(pause);

	// Don't pause sounds if the game isn't active
	// FIXME - this is quite a nasty hack, replace with something cleaner, and w/o
	// having to access member vars directly!
	if (!_vm->_roomResource)
		return;

	_soundsPaused = pause;

#ifdef ENABLE_SCUMM_7_8
	if (_vm->_imuseDigital) {
		_vm->_imuseDigital->pause(pause);
	}
#endif

	_mixer->pauseAll(pause);

	if ((_vm->_game.features & GF_AUDIOTRACKS) && _vm->VAR(_vm->VAR_MUSIC_TIMER) > 0) {
		if (pause)
			stopCDTimer();
		else
			startCDTimer();
	}
}

void Sound::setupSfxFile() {
	struct SoundFileExtensions {
		const char *ext;
		SoundMode mode;
	};

	static const SoundFileExtensions extensions[] = {
		{ "sou", kVOCMode },
#ifdef USE_FLAC
		{ "sof", kFLACMode },
#endif
#ifdef USE_VORBIS
		{ "sog", kVorbisMode },
#endif
#ifdef USE_MAD
		{ "so3", kMP3Mode },
#endif
		{ 0, kVOCMode }
	};

	ScummFile file;
	_offsetTable = NULL;
	_sfxFileEncByte = 0;
	_sfxFilename.clear();

	/* Try opening the file <baseName>.sou first, e.g. tentacle.sou.
	 * That way, you can keep .sou files for multiple games in the
	 * same directory */

	Common::String basename[2];
	Common::String tmp;

	const char *ptr = strchr(_vm->_filenamePattern.pattern, '.');
	if (ptr) {
		basename[0] = Common::String(_vm->_filenamePattern.pattern, ptr - _vm->_filenamePattern.pattern + 1);
	} else {
		basename[0] = _vm->_filenamePattern.pattern;
		basename[0] += '.';
	}
	basename[1] = "monster.";

	if (_vm->_game.heversion >= 60) {
		if ((_vm->_game.heversion <= 62 && _vm->_game.platform == Common::kPlatformMacintosh) || (_vm->_game.heversion >= 70)) {
			tmp = _vm->generateFilename(-2);
		} else {
			tmp = basename[0] + "tlk";
		}

		if (file.open(tmp))
			_sfxFilename = tmp;

		if (_vm->_game.heversion <= 74)
			_sfxFileEncByte = 0x69;

		_soundMode = kVOCMode;
	} else {
		for (uint j = 0; j < 2 && !file.isOpen(); ++j) {
			for (int i = 0; extensions[i].ext; ++i) {
				tmp = basename[j] + extensions[i].ext;
				if (_vm->openFile(file, tmp)) {
					_soundMode = extensions[i].mode;
					_sfxFilename = tmp;
					break;
				}
			}
		}
	}

	if (_soundMode != kVOCMode) {
		/* Now load the 'offset' index in memory to be able to find the MP3 data

		   The format of the .SO3 file is easy :
		   - number of bytes of the 'index' part
		   - N times the following fields (4 bytes each) :
		   + offset in the original sound file
		   + offset of the MP3 data in the .SO3 file WITHOUT taking into account
		   the index field and the 'size' field
		   + the number of 'tags'
		   + the size of the MP3 data
		   - and then N times :
		   + the tags
		   + the MP3 data
		 */
		int size, compressed_offset;
		MP3OffsetTable *cur;
		compressed_offset = file.readUint32BE();
		_offsetTable = (MP3OffsetTable *) malloc(compressed_offset);
		_numSoundEffects = compressed_offset / 16;

		size = compressed_offset;
		cur = _offsetTable;
		while (size > 0) {
			cur->org_offset = file.readUint32BE();
			cur->new_offset = file.readUint32BE() + compressed_offset + 4; /* The + 4 is to take into accound the 'size' field */
			cur->num_tags = file.readUint32BE();
			cur->compressed_size = file.readUint32BE();
			size -= 4 * 4;
			cur++;
		}
	}
}

bool Sound::isSfxFinished() const {
	return !_mixer->hasActiveChannelOfType(Audio::Mixer::kSFXSoundType);
}

// We use a real timer in an attempt to get better sync with CD tracks. This is
// necessary for games like Loom CD.

static void cd_timer_handler(void *refCon) {
	ScummEngine *scumm = (ScummEngine *)refCon;

	// FIXME: Turn off the timer when it's no longer needed. In theory, it
	// should be possible to check with pollCD(), but since CD sound isn't
	// properly restarted when reloading a saved game, I don't dare to.

	scumm->VAR(scumm->VAR_MUSIC_TIMER) += 6;
}

void Sound::startCDTimer() {
	// This timer interval is based on two scenes: The Monkey Island 1
	// intro, and the scene in Loom CD where Chaos appears. In both cases
	// the game plays the scene as two separate sounds, even though both
	// halves are right next to each other in the CD track. Probably so
	// that you can hit Escape to skip the first half.
	//
	// Make it too low, and the Monkey Island theme will be cut short. Make
	// it too high, and there will be a nasty "hiccup" just as Chaos
	// appears.

	_vm->getTimerManager()->removeTimerProc(&cd_timer_handler);
	_vm->getTimerManager()->installTimerProc(&cd_timer_handler, 100700, _vm, "scummCDtimer");
}

void Sound::stopCDTimer() {
	_vm->getTimerManager()->removeTimerProc(&cd_timer_handler);
}

void Sound::playCDTrack(int track, int numLoops, int startFrame, int duration) {
	// Reset the music timer variable at the start of a new track
	_vm->VAR(_vm->VAR_MUSIC_TIMER) = 0;

	// Play it
	if (!_soundsPaused)
		g_system->getAudioCDManager()->play(track, numLoops, startFrame, duration);

	// Start the timer after starting the track. Starting an MP3 track is
	// almost instantaneous, but a CD player may take some time. Hopefully
	// playCD() will block during that delay.
	startCDTimer();
}

void Sound::stopCD() {
	g_system->getAudioCDManager()->stop();
}

int Sound::pollCD() const {
	return g_system->getAudioCDManager()->isPlaying();
}

void Sound::updateCD() {
	g_system->getAudioCDManager()->updateCD();
}

void Sound::saveLoadWithSerializer(Serializer *ser) {
	static const SaveLoadEntry soundEntries[] = {
		MKLINE(Sound, _currentCDSound, sleInt16, VER(35)),
		MKLINE(Sound, _currentMusic, sleInt16, VER(35)),
		MKEND()
	};

	ser->saveLoadEntries(this, soundEntries);
}


#pragma mark -
#pragma mark --- Sound resource handling ---
#pragma mark -

static void convertMac0Resource(ResourceManager *res, ResId idx, byte *src_ptr, int size);


/*
 * TODO: The way we handle sound/music resources really is one huge hack.
 * We probably should reconsider how we do this, and maybe come up with a
 * better/cleaner solution. Even if we keep the existing code, it really
 * could stand a thorough cleanup!
 */

int ScummEngine::readSoundResource(ResId idx) {
	uint32 pos, total_size, size, tag, basetag, max_total_size;
	int pri, best_pri;
	uint32 best_size = 0, best_offs = 0;
	byte *ptr;

	debugC(DEBUG_RESOURCE, "readSoundResource(%d)", idx);

	pos = 0;

	_fileHandle->readUint32LE();
	max_total_size = _fileHandle->readUint32BE() - 8;
	basetag = _fileHandle->readUint32BE();
	total_size = _fileHandle->readUint32BE();

	debugC(DEBUG_RESOURCE, "  basetag: %s, total_size=%d", tag2str(basetag), total_size);

	switch (basetag) {
	case MKTAG('M','I','D','I'):
	case MKTAG('i','M','U','S'):
		if (_sound->_musicType != MDT_PCSPK && _sound->_musicType != MDT_PCJR) {
			_fileHandle->seek(-8, SEEK_CUR);
			_fileHandle->read(_res->createResource(rtSound, idx, total_size + 8), total_size + 8);
			return 1;
		}
		break;
	case MKTAG('S','O','U',' '):
		best_pri = -1;
		while (pos < total_size) {
			tag = _fileHandle->readUint32BE();
			size = _fileHandle->readUint32BE() + 8;
			pos += size;

			pri = -1;

			switch (tag) {
			case MKTAG('T','O','W','S'):
				pri = 16;
				break;
			case MKTAG('S','B','L',' '):
				pri = 15;
				break;
			case MKTAG('A','D','L',' '):
				pri = 1;
				if (_sound->_musicType == MDT_ADLIB || _sound->_musicType == MDT_TOWNS)
					pri = 10;
				break;
			case MKTAG('A','M','I',' '):
				pri = 3;
				break;
			case MKTAG('R','O','L',' '):
				pri = 3;
				if (_native_mt32)
					pri = 5;
				break;
			case MKTAG('G','M','D',' '):
				pri = 4;
				break;
			case MKTAG('M','A','C',' '):	// Occurs in Mac MI2, FOA
				pri = 2;
				break;
			case MKTAG('S','P','K',' '):
				pri = -1;
				if (_sound->_musicType == MDT_PCSPK || _sound->_musicType == MDT_PCJR)
					pri = 11;
				break;
			}

			// We only allow SPK resources for PC Speaker and PCJr here
			// since other resource would sound horribly with their output
			// drivers.
			if ((_sound->_musicType == MDT_PCSPK || _sound->_musicType == MDT_PCJR) && pri != 11)
				pri = -1;

			// We only allow ADL, SBL and TOWS resources when AdLib
			// or FM-Towns is used as primary audio output. This fixes some
			// odd sounds when Indy and Sophia leave Atlantis with the
			// submarine in Indy4. (Easy to check with bootparam 4061 in
			// the CD version). It seems the game only contains a ROL resource
			// for sound id 60. Formerly we tried to play that via the AdLib
			// or FM-Towns audio driver resulting in strange noises. Now we
			// behave like the original did.
			// We make an exception for Macintosh, which uses priority 2 for
			// its sound resources, and Amiga games, which feature only ROL
			// resources, since we are a doing Midi -> AdLib conversion for
			// these.
			if ((_sound->_musicType == MDT_ADLIB || _sound->_musicType == MDT_TOWNS) && pri != 16
				&& pri != 15 && pri != 10 && pri != 2 && _game.platform != Common::kPlatformAmiga)
				pri = -1;

			debugC(DEBUG_RESOURCE, "    tag: %s, total_size=%d, pri=%d", tag2str(tag), size, pri);

			if (pri > best_pri) {
				best_pri = pri;
				best_size = size;
				best_offs = _fileHandle->pos();
			}

			_fileHandle->seek(size - 8, SEEK_CUR);
		}

		if (best_pri != -1) {
			_fileHandle->seek(best_offs - 8, SEEK_SET);
			ptr = _res->createResource(rtSound, idx, best_size);
			_fileHandle->read(ptr, best_size);
			//dumpResource("sound-", idx, ptr);
			return 1;
		}
		break;
	case MKTAG('M','a','c','0'):
		_fileHandle->seek(-12, SEEK_CUR);
		total_size = _fileHandle->readUint32BE() - 8;
		ptr = (byte *)calloc(total_size, 1);
		_fileHandle->read(ptr, total_size);
		//dumpResource("sound-", idx, ptr);
		convertMac0Resource(_res, idx, ptr, total_size);
		free(ptr);
		return 1;

	case MKTAG('M','a','c','1'):
	case MKTAG('R','I','F','F'):
	case MKTAG('T','A','L','K'):
	case MKTAG('D','I','G','I'):
	case MKTAG('C','r','e','a'):
	case 0x460e200d:	// WORKAROUND bug # 1311447
		_fileHandle->seek(-12, SEEK_CUR);
		total_size = _fileHandle->readUint32BE();
		ptr = _res->createResource(rtSound, idx, total_size);
		_fileHandle->read(ptr, total_size - 8);
		//dumpResource("sound-", idx, ptr);
		return 1;

	case MKTAG('H','S','H','D'):
		// HE sound type without SOUN header
		_fileHandle->seek(-16, SEEK_CUR);
		total_size = max_total_size + 8;
		ptr = _res->createResource(rtSound, idx, total_size);
		_fileHandle->read(ptr, total_size);
		//dumpResource("sound-", idx, ptr);
		return 1;

	case MKTAG('F','M','U','S'): {
		// Used in 3DO version of puttputt joins the parade and probably others
		// Specifies a separate file to be used for music from what I gather.
		int tmpsize;
		Common::File dmuFile;
		char buffer[128];
		debugC(DEBUG_SOUND, "Found base tag FMUS in sound %d, size %d", idx, total_size);
		debugC(DEBUG_SOUND, "It was at position %d", _fileHandle->pos());

		_fileHandle->seek(4, SEEK_CUR);
		// HSHD size
		tmpsize = _fileHandle->readUint32BE();
		// skip to size part of the SDAT block
		_fileHandle->seek(tmpsize - 4, SEEK_CUR);
		// SDAT size
		tmpsize = _fileHandle->readUint32BE();

		// SDAT contains name of file we want
		_fileHandle->read(buffer, MIN(128, tmpsize - 8));
		// files seem to be 11 chars (8.3)
		char *p = (char *)memchr(buffer, '.', 12);
		if (!p) p = &buffer[8];
		strcpy(p, ".dmu");
		debugC(DEBUG_SOUND, "FMUS file %s", buffer);

		if (!dmuFile.open(buffer)) {
			error("Can't open music file %s", buffer);
			_res->_types[rtSound][idx]._roomoffs = RES_INVALID_OFFSET;
			return 0;
		}
		dmuFile.seek(4, SEEK_SET);
		total_size = dmuFile.readUint32BE();
		debugC(DEBUG_SOUND, "dmu file size %d", total_size);
		dmuFile.seek(-8, SEEK_CUR);
		dmuFile.read(_res->createResource(rtSound, idx, total_size), total_size);
		dmuFile.close();
		}
		return 1;

	default:
		if (SWAP_BYTES_32(basetag) == max_total_size) {
			_fileHandle->seek(-12, SEEK_CUR);
			total_size = _fileHandle->readUint32BE();
			_fileHandle->seek(-8, SEEK_CUR);
			ptr = _res->createResource(rtSound, idx, total_size);
			_fileHandle->read(ptr, total_size);
			//dumpResource("sound-", idx, ptr);
			return 1;
		}
		error("Unrecognized base tag 0x%08x in sound %d", basetag, idx);
	}
	_res->_types[rtSound][idx]._roomoffs = RES_INVALID_OFFSET;
	return 0;
}

// AdLib MIDI-SYSEX to set MIDI instruments for small header games.
static const byte ADLIB_INSTR_MIDI_HACK[95] = {
	0x00, 0xf0, 0x14, 0x7d, 0x00,  // sysex 00: part on/off
	0x00, 0x00, 0x03,              // part/channel  (offset  5)
	0x00, 0x00, 0x07, 0x0f, 0x00, 0x00, 0x08, 0x00,
	0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0xf7,
	0x00, 0xf0, 0x41, 0x7d, 0x10,  // sysex 16: set instrument
	0x00, 0x01,                    // part/channel  (offset 28)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xf7,
	0x00, 0xb0, 0x07, 0x64        // Controller 7 = 100 (offset 92)
};

static const byte map_param[7] = {
	0, 2, 3, 4, 8, 9, 0,
};

static const byte freq2note[128] = {
	/*128*/	6, 6, 6, 6,
	/*132*/ 7, 7, 7, 7, 7, 7, 7,
	/*139*/ 8, 8, 8, 8, 8, 8, 8, 8, 8,
	/*148*/ 9, 9, 9, 9, 9, 9, 9, 9, 9,
	/*157*/ 10, 10, 10, 10, 10, 10, 10, 10, 10,
	/*166*/ 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	/*176*/ 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	/*186*/ 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	/*197*/ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	/*209*/ 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	/*222*/ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	/*235*/ 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
	/*249*/ 18, 18, 18, 18, 18, 18, 18
};

static const uint16 num_steps_table[] = {
	1, 2, 4, 5,
	6, 7, 8, 9,
	10, 12, 14, 16,
	18, 21, 24, 30,
	36, 50, 64, 82,
	100, 136, 160, 192,
	240, 276, 340, 460,
	600, 860, 1200, 1600
};

static int convert_extraflags(byte * ptr, byte * src_ptr) {
	int flags = src_ptr[0];

	int t1, t2, t3, t4, time;
	int v1, v2, v3;

	if (!(flags & 0x80))
		return -1;

	t1 = (src_ptr[1] & 0xf0) >> 3;
	t2 = (src_ptr[2] & 0xf0) >> 3;
	t3 = (src_ptr[3] & 0xf0) >> 3 | (flags & 0x40 ? 0x80 : 0);
	t4 = (src_ptr[3] & 0x0f) << 1;
	v1 = (src_ptr[1] & 0x0f);
	v2 = (src_ptr[2] & 0x0f);
	v3 = 31;
	if ((flags & 0x7) == 0) {
	  v1 = v1 + 31 + 8;
	  v2 = v2 + 31 + 8;
	} else {
	  v1 = v1 * 2 + 31;
	  v2 = v2 * 2 + 31;
	}

	/* flags a */
	if ((flags & 0x7) == 6)
		ptr[0] = 0;
	else {
		ptr[0] = (flags >> 4) & 0xb;
		ptr[1] = map_param[flags & 0x7];
	}

	/* extra a */
	ptr[2] = 0;
	ptr[3] = 0;
	ptr[4] = t1 >> 4;
	ptr[5] = t1 & 0xf;
	ptr[6] = v1 >> 4;
	ptr[7] = v1 & 0xf;
	ptr[8] = t2 >> 4;
	ptr[9] = t2 & 0xf;
	ptr[10] = v2 >> 4;
	ptr[11] = v2 & 0xf;
	ptr[12] = t3 >> 4;
	ptr[13] = t3 & 0xf;
	ptr[14] = t4 >> 4;
	ptr[15] = t4 & 0xf;
	ptr[16] = v3 >> 4;
	ptr[17] = v3 & 0xf;

	time = num_steps_table[t1] + num_steps_table[t2]
		+ num_steps_table[t3 & 0x7f] + num_steps_table[t4];
	if (flags & 0x20) {
		int playtime = ((src_ptr[4] >> 4) & 0xf) * 118 +
			(src_ptr[4] & 0xf) * 8;
		if (playtime > time)
			time = playtime;
	}
	/*
	time = ((src_ptr[4] >> 4) & 0xf) * 118 +
		(src_ptr[4] & 0xf) * 8;
	*/
	return time;
}

#define kMIDIHeaderSize		46
static byte *writeMIDIHeader(byte *ptr, const char *type, int ppqn, int total_size) {
	uint32 dw = TO_BE_32(total_size);

	memcpy(ptr, type, 4); ptr += 4;
	memcpy(ptr, &dw, 4); ptr += 4;
	memcpy(ptr, "MDhd", 4); ptr += 4;
	ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 8;
	ptr += 4;
	memset(ptr, 0, 8); ptr += 8;
	memcpy(ptr, "MThd", 4); ptr += 4;
	ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 6;
	ptr += 4;
	ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 1; // MIDI format 0 with 1 track
	ptr += 4;

	*ptr++ = ppqn >> 8;
	*ptr++ = ppqn & 0xFF;

	memcpy(ptr, "MTrk", 4); ptr += 4;
	memcpy(ptr, &dw, 4); ptr += 4;

	return ptr;
}

static byte *writeVLQ(byte *ptr, int value) {
	if (value > 0x7f) {
		if (value > 0x3fff) {
			*ptr++ = (value >> 14) | 0x80;
			value &= 0x3fff;
		}
		*ptr++ = (value >> 7) | 0x80;
		value &= 0x7f;
	}
	*ptr++ = value;
	return ptr;
}

static byte Mac0ToGMInstrument(uint32 type, int &transpose) {
	transpose = 0;
	switch (type) {
	case MKTAG('M','A','R','I'): return 12;
	case MKTAG('P','L','U','C'): return 45;
	case MKTAG('H','A','R','M'): return 22;
	case MKTAG('P','I','P','E'): return 19;
	case MKTAG('T','R','O','M'): transpose = -12; return 57;
	case MKTAG('S','T','R','I'): return 48;
	case MKTAG('H','O','R','N'): return 60;
	case MKTAG('V','I','B','E'): return 11;
	case MKTAG('S','H','A','K'): return 77;
	case MKTAG('P','A','N','P'): return 75;
	case MKTAG('W','H','I','S'): return 76;
	case MKTAG('O','R','G','A'): return 17;
	case MKTAG('B','O','N','G'): return 115;
	case MKTAG('B','A','S','S'): transpose = -24; return 35;
	default:
		error("Unknown Mac0 instrument %s found", tag2str(type));
	}
}

static void convertMac0Resource(ResourceManager *res, ResId idx, byte *src_ptr, int size) {
	/*
	From Markus Magnuson (superqult) we got this information:
	Mac0
	---
	   4 bytes - 'SOUN'
	BE 4 bytes - block length

		   4 bytes  - 'Mac0'
		BE 4 bytes  - (blockLength - 27)
		   28 bytes - ???

		   do this three times (once for each channel):
			  4 bytes  - 'Chan'
		   BE 4 bytes  - channel length
			  4 bytes  - instrument name (e.g. 'MARI')

			  do this for ((chanLength-24)/4) times:
				 2 bytes  - note duration
				 1 byte   - note value
				 1 byte   - note velocity

			  4 bytes - ???
			  4 bytes - 'Loop'/'Done'
			  4 bytes - ???

	   1 byte - 0x09
	---

	Instruments (General Midi):
	"MARI" - Marimba (12)
	"PLUC" - Pizzicato Strings (45)
	"HARM" - Harmonica (22)
	"PIPE" - Church Organ? (19) or Flute? (73) or Bag Pipe (109)
	"TROM" - Trombone (57)
	"STRI" - String Ensemble (48 or 49)
	"HORN" - French Horn? (60) or English Horn? (69)
	"VIBE" - Vibraphone (11)
	"SHAK" - Shakuhachi? (77)
	"PANP" - Pan Flute (75)
	"WHIS" - Whistle (78) / Bottle (76)
	"ORGA" - Drawbar Organ (16; but could also be 17-20)
	"BONG" - Woodblock? (115)
	"BASS" - Bass (32-39)


	Now the task could be to convert this into MIDI, to be fed into iMuse.
	Or we do something similiar to what is done in Player_V3, assuming
	we can identify SFX in the MI datafiles for each of the instruments
	listed above.
	*/

#if 0
	byte *ptr = _res->createResource(rtSound, idx, size);
	memcpy(ptr, src_ptr, size);
#else
	const int ppqn = 480;
	byte *ptr, *start_ptr;

	int total_size = 0;
	total_size += kMIDIHeaderSize; // Header
	total_size += 7;               // Tempo META
	total_size += 3 * 3;           // Three program change mesages
	total_size += 22;              // Possible jump SysEx
	total_size += 5;               // EOT META

	int i, len;
	byte track_instr[3];
	byte *track_data[3];
	int track_len[3];
	int track_transpose[3];
	bool looped = false;

	src_ptr += 8;
	// TODO: Decipher the unknown bytes in the header. For now, skip 'em
	src_ptr += 28;

	// Parse the three channels
	for (i = 0; i < 3; i++) {
		assert(READ_BE_UINT32(src_ptr) == MKTAG('C','h','a','n'));
		len = READ_BE_UINT32(src_ptr + 4);
		track_len[i] = len - 24;
		track_instr[i] = Mac0ToGMInstrument(READ_BE_UINT32(src_ptr + 8), track_transpose[i]);
		track_data[i] = src_ptr + 12;
		src_ptr += len;
		looped = (READ_BE_UINT32(src_ptr - 8) == MKTAG('L','o','o','p'));

		// For each note event, we need up to 6 bytes for the
		// Note On (3 VLQ, 3 event), and 6 bytes for the Note
		// Off (3 VLQ, 3 event). So 12 bytes total.
		total_size += 12 * track_len[i];
	}
	assert(*src_ptr == 0x09);

	// Create sound resource
	start_ptr = res->createResource(rtSound, idx, total_size);

	// Insert MIDI header
	ptr = writeMIDIHeader(start_ptr, "GMD ", ppqn, total_size);

	// Write a tempo change Meta event
	// 473 / 4 Hz, convert to micro seconds.
	uint32 dw = 1000000 * 437 / 4 / ppqn; // 1000000 * ppqn * 4 / 473;
	memcpy(ptr, "\x00\xFF\x51\x03", 4); ptr += 4;
	*ptr++ = (byte)((dw >> 16) & 0xFF);
	*ptr++ = (byte)((dw >> 8) & 0xFF);
	*ptr++ = (byte)(dw & 0xFF);

	// Insert program change messages
	*ptr++ = 0; // VLQ
	*ptr++ = 0xC0;
	*ptr++ = track_instr[0];
	*ptr++ = 0; // VLQ
	*ptr++ = 0xC1;
	*ptr++ = track_instr[1];
	*ptr++ = 0; // VLQ
	*ptr++ = 0xC2;
	*ptr++ = track_instr[2];

	// And now, the actual composition. Please turn all cell phones
	// and pagers off during the performance. Thank you.
	uint16 nextTime[3] = { 1, 1, 1 };
	int stage[3] = { 0, 0, 0 };

	while (track_len[0] | track_len[1] | track_len[2]) {
		int best = -1;
		uint16 bestTime = 0xFFFF;
		for (i = 0; i < 3; ++i) {
			if (track_len[i] && nextTime[i] < bestTime) {
				bestTime = nextTime[i];
				best = i;
			}
		}
		assert (best != -1);

		if (!stage[best]) {
			// We are STARTING this event.
			if (track_data[best][2] > 1) {
				// Note On
				ptr = writeVLQ(ptr, nextTime[best]);
				*ptr++ = 0x90 | best;
				*ptr++ = track_data[best][2] + track_transpose[best];
				*ptr++ = track_data[best][3] * 127 / 100; // Scale velocity
				for (i = 0; i < 3; ++i)
					nextTime[i] -= bestTime;
			}
			nextTime[best] += READ_BE_UINT16 (track_data[best]);
			stage[best] = 1;
		} else {
			// We are ENDING this event.
			if (track_data[best][2] > 1) {
				// There was a Note On, so do a Note Off
				ptr = writeVLQ(ptr, nextTime[best]);
				*ptr++ = 0x80 | best;
				*ptr++ = track_data[best][2] + track_transpose[best];
				*ptr++ = track_data[best][3] * 127 / 100; // Scale velocity
				for (i = 0; i < 3; ++i)
					nextTime[i] -= bestTime;
			}
			track_data[best] += 4;
			track_len[best] -= 4;
			stage[best] = 0;
		}
	}

	// Is this a looped song? If so, effect a loop by
	// using the S&M maybe_jump SysEx command.
	// FIXME: Jamieson630: The jump seems to be happening
	// too quickly! There should maybe be a pause after
	// the last Note Off? But I couldn't find one in the
	// MI1 Lookout music, where I was hearing problems.
	if (looped) {
		memcpy(ptr, "\x00\xf0\x13\x7d\x30\00", 6); ptr += 6; // maybe_jump
		memcpy(ptr, "\x00\x00", 2); ptr += 2;            // cmd -> 0 means always jump
		memcpy(ptr, "\x00\x00\x00\x00", 4); ptr += 4;    // track -> 0 (only track)
		memcpy(ptr, "\x00\x00\x00\x01", 4); ptr += 4;    // beat -> 1 (first beat)
		memcpy(ptr, "\x00\x00\x00\x01", 4); ptr += 4;    // tick -> 1
		memcpy(ptr, "\x00\xf7", 2); ptr += 2;            // SysEx end marker
	}

	// Insert end of song META
	memcpy(ptr, "\x00\xff\x2f\x00\x00", 5); ptr += 5;

	assert(ptr <= start_ptr + total_size);

	// Rewrite MIDI header, this time with true size
	total_size = ptr - start_ptr;
	ptr = writeMIDIHeader(start_ptr, "GMD ", ppqn, total_size);
#endif
}

static void convertADResource(ResourceManager *res, const GameSettings& game, ResId idx, byte *src_ptr, int size) {
	// We will ignore the PPQN in the original resource, because
	// it's invalid anyway. We use a constant PPQN of 480.
	const int ppqn = 480;
	uint32 dw;
	int i, ch;
	byte *ptr;
	int total_size = kMIDIHeaderSize + 7 + 8 * sizeof(ADLIB_INSTR_MIDI_HACK) + size;
	total_size += 24;	// Up to 24 additional bytes are needed for the jump sysex

	ptr = res->createResource(rtSound, idx, total_size);

	src_ptr += 2;
	size -= 2;

	// 0x80 marks a music resource. Otherwise it's a SFX
	if (*src_ptr == 0x80) {
		byte ticks, play_once;
		byte num_instr;
		byte *channel, *instr, *track;

		ptr = writeMIDIHeader(ptr, "ADL ", ppqn, total_size);

		// The "speed" of the song
		ticks = *(src_ptr + 1);

		// Flag that tells us whether we should loop the song (0) or play it only once (1)
		play_once = *(src_ptr + 2);

		// Number of instruments used
		num_instr = *(src_ptr + 8);	// Normally 8

		// copy the pointer to instrument data
		channel = src_ptr + 9;
		instr   = src_ptr + 0x11;

		// skip over the rest of the header and copy the MIDI data into a buffer
		src_ptr  += 0x11 + 8 * 16;
		size -= 0x11 + 8 * 16;

		track = src_ptr;

		// Convert the ticks into a MIDI tempo.
		// Unfortunate LOOM and INDY3 have different interpretation
		// of the ticks value.
		if (game.id == GID_INDY3) {
			// Note: since we fix ppqn at 480, ppqn/473 is almost 1
			dw = 500000 * 256 / 473 * ppqn / ticks;
		} else if (game.id == GID_LOOM && game.version == 3) {
			dw = 500000 * ppqn / 4 / ticks;
		} else {
			dw = 500000 * 256 / ticks;
		}
		debugC(DEBUG_SOUND, "  ticks = %d, speed = %d", ticks, dw);

		// Write a tempo change Meta event
		memcpy(ptr, "\x00\xFF\x51\x03", 4); ptr += 4;
		*ptr++ = (byte)((dw >> 16) & 0xFF);
		*ptr++ = (byte)((dw >> 8) & 0xFF);
		*ptr++ = (byte)(dw & 0xFF);

		// Copy our hardcoded instrument table into it
		// Then, convert the instrument table as given in this song resource
		// And write it *over* the hardcoded table.
		// Note: we deliberately.

		/* now fill in the instruments */
		for (i = 0; i < num_instr; i++) {
			ch = channel[i] - 1;
			if (ch < 0 || ch > 15)
				continue;

			if (instr[i*16 + 13])
				debugC(DEBUG_SOUND, "Sound %d instrument %d uses percussion", idx, i);

			debugC(DEBUG_SOUND, "Sound %d: instrument %d on channel %d.", idx, i, ch);

			memcpy(ptr, ADLIB_INSTR_MIDI_HACK, sizeof(ADLIB_INSTR_MIDI_HACK));

			ptr[5]  += ch;
			ptr[28] += ch;
			ptr[92] += ch;

			/* mod_characteristics */
			ptr[30 + 0] = (instr[i * 16 + 3] >> 4) & 0xf;
			ptr[30 + 1] = instr[i * 16 + 3] & 0xf;

			/* mod_scalingOutputLevel */
			ptr[30 + 2] = (instr[i * 16 + 4] >> 4) & 0xf;
			ptr[30 + 3] = instr[i * 16 + 4] & 0xf;

			/* mod_attackDecay */
			ptr[30 + 4] = ((~instr[i * 16 + 5]) >> 4) & 0xf;
			ptr[30 + 5] = (~instr[i * 16 + 5]) & 0xf;

			/* mod_sustainRelease */
			ptr[30 + 6] = ((~instr[i * 16 + 6]) >> 4) & 0xf;
			ptr[30 + 7] = (~instr[i * 16 + 6]) & 0xf;

			/* mod_waveformSelect */
			ptr[30 + 8] = (instr[i * 16 + 7] >> 4) & 0xf;
			ptr[30 + 9] = instr[i * 16 + 7] & 0xf;

			/* car_characteristic */
			ptr[30 + 10] = (instr[i * 16 + 8] >> 4) & 0xf;
			ptr[30 + 11] = instr[i * 16 + 8] & 0xf;

			/* car_scalingOutputLevel */
			ptr[30 + 12] = (instr[i * 16 + 9] >> 4) & 0xf;
			ptr[30 + 13] = instr[i * 16 + 9] & 0xf;

			/* car_attackDecay */
			ptr[30 + 14] = ((~instr[i * 16 + 10]) >> 4) & 0xf;
			ptr[30 + 15] = (~instr[i * 16 + 10]) & 0xf;

			/* car_sustainRelease */
			ptr[30 + 16] = ((~instr[i * 16 + 11]) >> 4) & 0xf;
			ptr[30 + 17] = (~instr[i * 16 + 11]) & 0xf;

			/* car_waveFormSelect */
			ptr[30 + 18] = (instr[i * 16 + 12] >> 4) & 0xf;
			ptr[30 + 19] = instr[i * 16 + 12] & 0xf;

			/* feedback */
			ptr[30 + 20] = (instr[i * 16 + 2] >> 4) & 0xf;
			ptr[30 + 21] = instr[i * 16 + 2] & 0xf;
			ptr += sizeof(ADLIB_INSTR_MIDI_HACK);
		}

		// There is a constant delay of ppqn/3 before the music starts.
		if (ppqn / 3 >= 128)
			*ptr++ = ((ppqn / 3) >> 7) | 0x80;
		*ptr++ = ppqn / 3 & 0x7f;

		// Now copy the actual music data
		memcpy(ptr, track, size);
		ptr += size;

		if (!play_once) {
			// The song is meant to be looped. We achieve this by inserting just
			// before the song end a jump to the song start. More precisely we abuse
			// a S&M sysex, "maybe_jump" to achieve this effect. We could also
			// use a set_loop sysex, but it's a bit longer, a little more complicated,
			// and has no advantage either.

			// First, find the track end
			byte *end = ptr;
			ptr -= size;
			for (; ptr < end; ptr++) {
				if (*ptr == 0xff && *(ptr + 1) == 0x2f)
					break;
			}
			assert(ptr < end);

			// Now insert the jump. The jump offset is measured in ticks.
			// We have ppqn/3 ticks before the first note.

			const int jump_offset = ppqn / 3;
			memcpy(ptr, "\xf0\x13\x7d\x30\00", 5); ptr += 5;	// maybe_jump
			memcpy(ptr, "\x00\x00", 2); ptr += 2;			// cmd -> 0 means always jump
			memcpy(ptr, "\x00\x00\x00\x00", 4); ptr += 4;	// track -> there is only one track, 0
			memcpy(ptr, "\x00\x00\x00\x01", 4); ptr += 4;	// beat -> for now, 1 (first beat)
			// Ticks
			*ptr++ = (byte)((jump_offset >> 12) & 0x0F);
			*ptr++ = (byte)((jump_offset >> 8) & 0x0F);
			*ptr++ = (byte)((jump_offset >> 4) & 0x0F);
			*ptr++ = (byte)(jump_offset & 0x0F);
			memcpy(ptr, "\x00\xf7", 2); ptr += 2;	// sysex end marker
		}
	} else {

		/* This is a sfx resource.  First parse it quickly to find the parallel
		 * tracks.
		 */
		ptr = writeMIDIHeader(ptr, "ASFX", ppqn, total_size);

		byte current_instr[3][14];
		int  current_note[3];
		int track_time[3];
		byte *track_data[3];

		int track_ctr = 0;
		byte chunk_type = 0;
		int delay, delay2, olddelay;

		// Write a tempo change Meta event
		// 473 / 4 Hz, convert to micro seconds.
		dw = 1000000 * ppqn * 4 / 473;
		memcpy(ptr, "\x00\xFF\x51\x03", 4); ptr += 4;
		*ptr++ = (byte)((dw >> 16) & 0xFF);
		*ptr++ = (byte)((dw >> 8) & 0xFF);
		*ptr++ = (byte)(dw & 0xFF);

		for (i = 0; i < 3; i++) {
			track_time[i] = -1;
			current_note[i] = -1;
		}
		while (size > 0) {
			assert(track_ctr < 3);
			track_data[track_ctr] = src_ptr;
			track_time[track_ctr] = 0;
			track_ctr++;
			while (size > 0) {
				chunk_type = *(src_ptr);
				if (chunk_type == 1) {
					src_ptr += 15;
					size -= 15;
				} else if (chunk_type == 2) {
					src_ptr += 11;
					size -= 11;
				} else if (chunk_type == 0x80) {
					src_ptr ++;
					size --;
				} else {
					break;
				}
			}
			if (chunk_type == 0xff)
				break;
			src_ptr++;
		}

		int curtime = 0;
		for (;;) {
			int mintime = -1;
			ch = -1;
			for (i = 0; i < 3; i++) {
				if (track_time[i] >= 0 &&
					(mintime == -1 || mintime > track_time[i])) {
					mintime = track_time[i];
					ch = i;
				}
			}
			if (mintime < 0)
				break;

			src_ptr = track_data[ch];
			chunk_type = *src_ptr;

			if (current_note[ch] >= 0) {
				delay = mintime - curtime;
				curtime = mintime;
				ptr = writeVLQ(ptr, delay);
				*ptr++ = 0x80 + ch; // key off channel;
				*ptr++ = current_note[ch];
				*ptr++ = 0;
				current_note[ch] = -1;
			}

			switch (chunk_type) {
			case 1:
				/* Instrument definition */
				memcpy(current_instr[ch], src_ptr + 1, 14);
				src_ptr += 15;
				break;

			case 2:
				/* tone/parammodulation */
				memcpy(ptr, ADLIB_INSTR_MIDI_HACK,
					   sizeof(ADLIB_INSTR_MIDI_HACK));

				ptr[5]  += ch;
				ptr[28] += ch;
				ptr[92] += ch;

				/* mod_characteristic */
				ptr[30 + 0] = (current_instr[ch][3] >> 4) & 0xf;
				ptr[30 + 1] = current_instr[ch][3] & 0xf;

				/* mod_scalingOutputLevel */
				ptr[30 + 2] = (current_instr[ch][4] >> 4) & 0xf;
				ptr[30 + 3] = current_instr[ch][4] & 0xf;

				/* mod_attackDecay */
				ptr[30 + 4] = ((~current_instr[ch][5]) >> 4) & 0xf;
				ptr[30 + 5] = (~current_instr[ch][5]) & 0xf;

				/* mod_sustainRelease */
				ptr[30 + 6] = ((~current_instr[ch][6]) >> 4) & 0xf;
				ptr[30 + 7] = (~current_instr[ch][6]) & 0xf;

				/* mod_waveformSelect */
				ptr[30 + 8] = (current_instr[ch][7] >> 4) & 0xf;
				ptr[30 + 9] = current_instr[ch][7] & 0xf;

				/* car_characteristic */
				ptr[30 + 10] = (current_instr[ch][8] >> 4) & 0xf;
				ptr[30 + 11] = current_instr[ch][8] & 0xf;

				/* car_scalingOutputLevel */
				ptr[30 + 12] = ((current_instr[ch][9]) >> 4) & 0xf;
				ptr[30 + 13] = (current_instr[ch][9]) & 0xf;

				/* car_attackDecay */
				ptr[30 + 14] = ((~current_instr[ch][10]) >> 4) & 0xf;
				ptr[30 + 15] = (~current_instr[ch][10]) & 0xf;

				/* car_sustainRelease */
				ptr[30 + 16] = ((~current_instr[ch][11]) >> 4) & 0xf;
				ptr[30 + 17] = (~current_instr[ch][11]) & 0xf;

				/* car_waveFormSelect */
				ptr[30 + 18] = (current_instr[ch][12] >> 4) & 0xf;
				ptr[30 + 19] = current_instr[ch][12] & 0xf;

				/* feedback */
				ptr[30 + 20] = (current_instr[ch][2] >> 4) & 0xf;
				ptr[30 + 21] = current_instr[ch][2] & 0xf;

				delay = mintime - curtime;
				curtime = mintime;

				{
					delay = convert_extraflags(ptr + 30 + 22, src_ptr + 1);
					delay2 = convert_extraflags(ptr + 30 + 40, src_ptr + 6);
					debugC(DEBUG_SOUND, "delays: %d / %d", delay, delay2);
					if (delay2 >= 0 && delay2 < delay)
						delay = delay2;
					if (delay == -1)
						delay = 0;
				}

				/* duration */
				ptr[30 + 58] = 0; // ((delay * 17 / 63) >> 4) & 0xf;
				ptr[30 + 59] = 0; // (delay * 17 / 63) & 0xf;

				ptr += sizeof(ADLIB_INSTR_MIDI_HACK);

				olddelay = mintime - curtime;
				curtime = mintime;
				ptr = writeVLQ(ptr, olddelay);

				{
					int freq = ((current_instr[ch][1] & 3) << 8)
						| current_instr[ch][0];
					if (!freq)
						freq = 0x80;
					freq <<= (((current_instr[ch][1] >> 2) + 1) & 7);
					int note = -11;
					while (freq >= 0x100) {
						note += 12;
						freq >>= 1;
					}
					debugC(DEBUG_SOUND, "Freq: %d (%x) Note: %d", freq, freq, note);
					if (freq < 0x80)
						note = 0;
					else
						note += freq2note[freq - 0x80];

					debugC(DEBUG_SOUND, "Note: %d", note);
					if (note <= 0)
						note = 1;
					else if (note > 127)
						note = 127;

					// Insert a note on event
					*ptr++ = 0x90 + ch; // key on channel
					*ptr++ = note;
					*ptr++ = 63;
					current_note[ch] = note;
					track_time[ch] = curtime + delay;
				}
				src_ptr += 11;
				break;

			case 0x80:
				// FIXME: This is incorrect. The original uses 0x80 for
				// looping a single channel. We currently interpret it as stop
				// thus we won't get looping for sound effects. It should
				// always jump to the start of the channel.
				//
				// Since we convert the data to MIDI and we cannot only loop a
				// single channel via MIDI fixing this will require some more
				// thought.
				track_time[ch] = -1;
				src_ptr ++;
				break;

			default:
				track_time[ch] = -1;
				break;
			}
			track_data[ch] = src_ptr;
		}
	}

	// Insert end of song sysex
	memcpy(ptr, "\x00\xff\x2f\x00\x00", 5); ptr += 5;
}


int ScummEngine::readSoundResourceSmallHeader(ResId idx) {
	uint32 pos, total_size, size, tag;
	uint32 ad_size = 0, ad_offs = 0;
	uint32 ro_size = 0, ro_offs = 0;
	uint32 wa_size = 0, wa_offs = 0;

	debug(4, "readSoundResourceSmallHeader(%d)", idx);

	if ((_game.id == GID_LOOM) && (_game.version == 3) && (_game.platform == Common::kPlatformPC) && VAR(VAR_SOUNDCARD) == 4) {
		// Roland resources in Loom are tagless
		// So we add an RO tag to allow imuse to detect format
		byte *ptr, *src_ptr;
		ro_offs = _fileHandle->pos();
		ro_size = _fileHandle->readUint16LE();

		src_ptr = (byte *) calloc(ro_size - 4, 1);
		_fileHandle->seek(ro_offs + 4, SEEK_SET);
		_fileHandle->read(src_ptr, ro_size -4);

		ptr = _res->createResource(rtSound, idx, ro_size + 2);
		memcpy(ptr, "RO", 2); ptr += 2;
		memcpy(ptr, src_ptr, ro_size - 4); ptr += ro_size - 4;
		free(src_ptr);
		return 1;
	} else if (_game.features & GF_OLD_BUNDLE) {
		wa_offs = _fileHandle->pos();
		wa_size = _fileHandle->readUint16LE();
		_fileHandle->seek(wa_size - 2, SEEK_CUR);

		if (!(_game.platform == Common::kPlatformAtariST || _game.platform == Common::kPlatformMacintosh)) {
			ad_offs = _fileHandle->pos();
			ad_size = _fileHandle->readUint16LE();
		}
		_fileHandle->seek(4, SEEK_CUR);
		total_size = wa_size + ad_size;
	} else {
		total_size = size = _fileHandle->readUint32LE();
		tag = _fileHandle->readUint16LE();
		debug(4, "  tag='%c%c', size=%d", (char) (tag & 0xff),
				(char) ((tag >> 8) & 0xff), size);

		if (tag == 0x4F52) { // RO
			ro_offs = _fileHandle->pos();
			ro_size = size;
		} else {
			pos = 6;
			while (pos < total_size) {
				size = _fileHandle->readUint32LE();
				tag = _fileHandle->readUint16LE();
				debug(4, "  tag='%c%c', size=%d", (char) (tag & 0xff),
						(char) ((tag >> 8) & 0xff), size);
				pos += size;

				// MI1 and Indy3 uses one or more nested SO resources, which contains AD and WA
				// resources.
				if ((tag == 0x4441) && !(ad_offs)) { // AD
					ad_size = size;
					ad_offs = _fileHandle->pos();
				} else if ((tag == 0x4157) && !(wa_offs)) { // WA
					wa_size = size;
					wa_offs = _fileHandle->pos();
				} else { // other AD, WA and nested SO resources
					if (tag == 0x4F53) { // SO
						pos -= size;
						size = 6;
						pos += 6;
					}
				}
				_fileHandle->seek(size - 6, SEEK_CUR);
			}
		}
	}

	if ((_sound->_musicType == MDT_PCSPK || _sound->_musicType == MDT_PCJR) && wa_offs != 0) {
		if (_game.features & GF_OLD_BUNDLE) {
			_fileHandle->seek(wa_offs, SEEK_SET);
			_fileHandle->read(_res->createResource(rtSound, idx, wa_size), wa_size);
		} else {
			_fileHandle->seek(wa_offs - 6, SEEK_SET);
			_fileHandle->read(_res->createResource(rtSound, idx, wa_size + 6), wa_size + 6);
		}
		return 1;
	} else if (_sound->_musicType == MDT_CMS) {
		if (_game.features & GF_OLD_BUNDLE) {
			bool hasAdLibMusicTrack = false;

			if (ad_offs) {
				_fileHandle->seek(ad_offs + 4 + 2, SEEK_SET);
				hasAdLibMusicTrack = (_fileHandle->readByte() == 0x80);
			}

			if (hasAdLibMusicTrack) {
				_fileHandle->seek(ad_offs, SEEK_SET);
				_fileHandle->read(_res->createResource(rtSound, idx, ad_size), ad_size);
			} else {
				_fileHandle->seek(wa_offs, SEEK_SET);
				_fileHandle->read(_res->createResource(rtSound, idx, wa_size), wa_size);
			}
		} else {
			bool hasAdLibMusicTrack = false;

			if (ad_offs) {
				_fileHandle->seek(ad_offs + 2, SEEK_SET);
				hasAdLibMusicTrack = (_fileHandle->readByte() == 0x80);
			}

			if (hasAdLibMusicTrack) {
				_fileHandle->seek(ad_offs - 4, SEEK_SET);
				_fileHandle->read(_res->createResource(rtSound, idx, ad_size + 4), ad_size + 4);
			} else {
				_fileHandle->seek(wa_offs - 6, SEEK_SET);
				_fileHandle->read(_res->createResource(rtSound, idx, wa_size + 6), wa_size + 6);
			}
		}
	} else if (ad_offs != 0) {
		// AD resources have a header, instrument definitions and one MIDI track.
		// We build an 'ADL ' resource from that:
		//   8 bytes resource header
		//  16 bytes MDhd header
		//  14 bytes MThd header
		//   8 bytes MTrk header
		//   7 bytes MIDI tempo sysex
		//     + some default instruments
		byte *ptr;
		if (_game.features & GF_OLD_BUNDLE) {
			ad_size -= 4;
			_fileHandle->seek(ad_offs + 4, SEEK_SET);
		} else {
			ad_size -= 6;
			_fileHandle->seek(ad_offs, SEEK_SET);
		}
		ptr = (byte *) calloc(ad_size, 1);
		_fileHandle->read(ptr, ad_size);
		convertADResource(_res, _game, idx, ptr, ad_size);
		free(ptr);
		return 1;
	} else if (ro_offs != 0) {
		_fileHandle->seek(ro_offs - 2, SEEK_SET);
		_fileHandle->read(_res->createResource(rtSound, idx, ro_size - 4), ro_size - 4);
		return 1;
	}
	_res->_types[rtSound][idx]._roomoffs = RES_INVALID_OFFSET;
	return 0;
}


} // End of namespace Scumm
