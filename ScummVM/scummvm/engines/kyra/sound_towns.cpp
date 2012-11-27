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

#include "common/config-manager.h"
#include "common/system.h"

#include "backends/audiocd/audiocd.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Kyra {

SoundTowns::SoundTowns(KyraEngine_v1 *vm, Audio::Mixer *mixer)
	: Sound(vm, mixer), _lastTrack(-1), _musicTrackData(0), _sfxFileData(0), _cdaPlaying(0),
	_sfxFileIndex((uint)-1), _musicFadeTable(0), _sfxWDTable(0), _sfxBTTable(0), _sfxChannel(0x46), _currentResourceSet(0) {
	memset(&_resInfo, 0, sizeof(_resInfo));
	_driver = new TownsEuphonyDriver(_mixer);
}

SoundTowns::~SoundTowns() {
	g_system->getAudioCDManager()->stop();
	haltTrack();
	delete _driver;
	delete[] _musicTrackData;
	delete[] _sfxFileData;
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

bool SoundTowns::init() {
	_vm->checkCD();
	int unused = 0;
	_musicFadeTable = _vm->staticres()->loadRawData(k1TownsMusicFadeTable, unused);
	_sfxWDTable = _vm->staticres()->loadRawData(k1TownsSFXwdTable, unused);
	_sfxBTTable = _vm->staticres()->loadRawData(k1TownsSFXbtTable, unused);
	_musicTrackData = new uint8[50570];

	if (!_driver->init())
		return false;

	if (!loadInstruments())
		return false;

	_driver->intf()->callback(68);
	_driver->intf()->callback(70, 0x33);
	_driver->setOutputVolume(1, 118, 118);

	return true;
}

void SoundTowns::process() {
	g_system->getAudioCDManager()->updateCD();
}

void SoundTowns::playTrack(uint8 track) {
	if (track < 2)
		return;
	track -= 2;

	uint tTableIndex = 3 * track;

	assert(tTableIndex + 2 < res()->cdaTableSize);

	int trackNum = (int)READ_LE_UINT32(&res()->cdaTable[tTableIndex + 2]);
	int32 loop = (int32)READ_LE_UINT32(&res()->cdaTable[tTableIndex + 1]);

	if (track == _lastTrack && _musicEnabled)
		return;

	beginFadeOut();

	if (_musicEnabled == 2 && trackNum != -1) {
		_driver->setOutputVolume(1, 118, 118);
		g_system->getAudioCDManager()->play(trackNum + 1, loop ? -1 : 1, 0, 0);
		g_system->getAudioCDManager()->updateCD();
		_cdaPlaying = true;
	} else if (_musicEnabled) {
		playEuphonyTrack(READ_LE_UINT32(&res()->cdaTable[tTableIndex]), loop);
		_cdaPlaying = false;
	}

	_lastTrack = track;
}

void SoundTowns::haltTrack() {
	_lastTrack = -1;
	g_system->getAudioCDManager()->stop();
	g_system->getAudioCDManager()->updateCD();
	_cdaPlaying = false;

	for (int i = 0; i < 6; i++)
		_driver->chanVolume(i, 0);
	for (int i = 0x40; i < 0x46; i++)
		_driver->chanVolume(i, 0);
	for (int i = 0; i < 32; i++)
		_driver->configChan_enable(i, 0);
	_driver->stopParser();
}

void SoundTowns::initAudioResourceInfo(int set, void *info) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		delete _resInfo[set];
		_resInfo[set] = info ? new SoundResourceInfo_Towns(*(SoundResourceInfo_Towns*)info) : 0;
	}
}

void SoundTowns::selectAudioResourceSet(int set) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		if (_resInfo[set])
			_currentResourceSet = set;
	}
}

bool SoundTowns::hasSoundFile(uint file) const {
	if (file < res()->fileListSize)
		return (res()->fileList[file] != 0);
	return false;
}

void SoundTowns::loadSoundFile(uint file) {
	if (_sfxFileIndex == file || file >= res()->fileListSize)
		return;
	_sfxFileIndex = file;
	delete[] _sfxFileData;
	_sfxFileData = _vm->resource()->fileData(res()->fileList[file], 0);
}

void SoundTowns::playSoundEffect(uint8 track, uint8) {
	if (!_sfxEnabled || !_sfxFileData)
		return;

	if (track == 0 || track == 10) {
		stopAllSoundEffects();
		return;
	} else if (track == 1) {
		fadeOutSoundEffects();
		return;
	}

	uint8 note = 60;
	if (_sfxFileIndex == 5) {
		if (track == 16) {
			note = 62;
			track = 15;
		} else if (track == 17) {
			note = 64;
			track = 15;
		} else if (track == 18) {
			note = 65;
			track = 15;
		}
	}

	uint8 *fileBody = _sfxFileData + 0x01b8;
	int32 offset = (int32)READ_LE_UINT32(_sfxFileData + (track - 0x0b) * 4);
	if (offset == -1)
		return;

	if (!_driver->soundEffectIsPlaying(_sfxChannel ^ 1)) {
		_sfxChannel ^= 1;
	} else if (_driver->soundEffectIsPlaying(_sfxChannel)) {
		_sfxChannel ^= 1;
		_driver->stopSoundEffect(_sfxChannel);
	}

	uint32 *sfxHeader = (uint32 *)(fileBody + offset);
	uint32 sfxHeaderID = READ_LE_UINT32(sfxHeader);
	uint32 playbackBufferSize = sfxHeaderID == 1 ? 30704 : READ_LE_UINT32(&sfxHeader[3]);

	uint8 *sfxPlaybackBuffer = new uint8[playbackBufferSize + 32];
	memcpy(sfxPlaybackBuffer, fileBody + offset, 32);

	uint8 *dst = sfxPlaybackBuffer + 32;
	memset(dst, 0x80, playbackBufferSize);

	uint8 *sfxBody = ((uint8 *)sfxHeader) + 0x20;

	if (!sfxHeaderID) {
		memcpy(dst, sfxBody, playbackBufferSize);
	} else if (sfxHeaderID == 1) {
		Screen::decodeFrame4(sfxBody, dst, playbackBufferSize);
	} else if (_sfxWDTable) {
		uint8 *tgt = dst;
		uint32 sfx_BtTable_Offset = 0;
		uint32 sfx_WdTable_Offset = 0;
		uint32 sfx_WdTable_Number = 5;
		uint32 inSize = READ_LE_UINT32(&sfxHeader[1]);

		for (uint32 i = 0; i < inSize; i++) {
			sfx_WdTable_Offset = (sfx_WdTable_Number * 3 << 9) + sfxBody[i] * 6;
			sfx_WdTable_Number = READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset);

			sfx_BtTable_Offset += (int16)READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset + 2);
			*tgt++ = _sfxBTTable[((sfx_BtTable_Offset >> 2) & 0xff)];

			sfx_BtTable_Offset += (int16)READ_LE_UINT16(_sfxWDTable + sfx_WdTable_Offset + 4);
			*tgt++ = _sfxBTTable[((sfx_BtTable_Offset >> 2) & 0xff)];
		}
	}

	_driver->chanVolume(_sfxChannel, 127);
	_driver->chanPanPos(_sfxChannel, 0x40);
	_driver->chanPitch(_sfxChannel, 0);
	_driver->playSoundEffect(_sfxChannel, note, 127, sfxPlaybackBuffer);
}

void SoundTowns::updateVolumeSettings() {
	if (!_driver)
		return;

	bool mute = false;
	_driver->setSoundEffectVolume(ConfMan.getInt("sfx_volume"));
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_driver->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	_driver->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

void SoundTowns::stopAllSoundEffects() {
	_driver->chanVolume(0x46, 0);
	_driver->chanVolume(0x47, 0);
	_driver->stopSoundEffect(0x46);
	_driver->stopSoundEffect(0x47);
	_sfxChannel = 0x46;
}

void SoundTowns::beginFadeOut() {
	if (_cdaPlaying) {
		for (int i = 118; i > 103; i--) {
			_driver->setOutputVolume(1, i, i);
			_vm->delay(2 * _vm->tickLength());
		}

		for (int i = 103; i > 83; i -= 2) {
			_driver->setOutputVolume(1, i, i);
			_vm->delay(2 * _vm->tickLength());
		}

		for (int i = 83; i > 58; i -= 2) {
			_driver->setOutputVolume(1, i, i);
			_vm->delay(_vm->tickLength());
		}

		for (int i = 58; i > 0; i--) {
			_driver->setOutputVolume(1, i, i);
			_vm->delay(1);
		}

		_driver->setOutputVolume(1, 0, 0);

	} else {
		if (_lastTrack == -1)
			return;

		uint32 ticks = 2;
		int tickAdv = 0;

		uint16 fadeVolCur[12];
		uint16 fadeVolStep[12];

		for (int i = 0; i < 6; i++) {
			fadeVolCur[i] = READ_LE_UINT16(&_musicFadeTable[(_lastTrack * 12 + i) * 2]);
			fadeVolStep[i] = fadeVolCur[i] / 50;
			fadeVolCur[i + 6] = READ_LE_UINT16(&_musicFadeTable[(_lastTrack * 12 + 6 + i) * 2]);
			fadeVolStep[i + 6] = fadeVolCur[i + 6] / 30;
		}

		for (int i = 0; i < 12; i++) {
			for (int ii = 0; ii < 6; ii++)
				_driver->chanVolume(ii, fadeVolCur[ii]);
			for (int ii = 0x40; ii < 0x46; ii++)
				_driver->chanVolume(ii, fadeVolCur[ii - 0x3a]);

			for (int ii = 0; ii < 6; ii++) {
				fadeVolCur[ii] -= fadeVolStep[ii];
				if (fadeVolCur[ii] < 10)
					fadeVolCur[ii] = 0;
				fadeVolCur[ii + 6] -= fadeVolStep[ii + 6];
				if (fadeVolCur[ii + 6] < 10)
					fadeVolCur[ii + 6] = 0;
			}

			if (++tickAdv == 3) {
				tickAdv = 0;
				ticks += 2;
			}
			_vm->delay(ticks * _vm->tickLength());
		}
	}

	haltTrack();
}

bool SoundTowns::loadInstruments() {
	uint8 *twm = _vm->resource()->fileData("twmusic.pak", 0);
	if (!twm)
		return false;

	Screen::decodeFrame4(twm, _musicTrackData, 50570);
	for (int i = 0; i < 128; i++)
		_driver->loadInstrument(0, i, &_musicTrackData[i * 48 + 8]);

	Screen::decodeFrame4(twm + 3232, _musicTrackData, 50570);
	for (int i = 0; i < 32; i++)
		_driver->loadInstrument(0x40, i, &_musicTrackData[i * 128 + 8]);

	_driver->unloadWaveTable(-1);
	uint8 *src = &_musicTrackData[32 * 128 + 8];
	for (int i = 0; i < 10; i++) {
		_driver->loadWaveTable(src);
		src = src + READ_LE_UINT16(&src[12]) + 32;
	}

	_driver->reserveSoundEffectChannels(2);

	delete[] twm;

	return true;
}

void SoundTowns::playEuphonyTrack(uint32 offset, int loop) {
	uint8 *twm = _vm->resource()->fileData("twmusic.pak", 0);
	Screen::decodeFrame4(twm + 19312 + offset, _musicTrackData, 50570);
	delete[] twm;

	const uint8 *src = _musicTrackData + 852;
	for (int i = 0; i < 32; i++)
		_driver->configChan_enable(i, *src++);
	for (int i = 0; i < 32; i++)
		_driver->configChan_setMode(i, *src++);
	for (int i = 0; i < 32; i++)
		_driver->configChan_remap(i, *src++);
	for (int i = 0; i < 32; i++)
		_driver->configChan_adjustVolume(i, *src++);
	for (int i = 0; i < 32; i++)
		_driver->configChan_setTranspose(i, *src++);

	src = _musicTrackData + 1748;
	for (int i = 0; i < 6; i++)
		_driver->assignChannel(i, *src++);
	for (int i = 0x40; i < 0x46; i++)
		_driver->assignChannel(i, *src++);

	uint32 trackSize = READ_LE_UINT32(_musicTrackData + 2048);
	uint8 startTick = _musicTrackData[2052];

	_driver->setMusicTempo(_musicTrackData[2053]);

	src = _musicTrackData + 2054;
	uint32 l = READ_LE_UINT32(src + trackSize);
	trackSize += (l + 4);
	l = READ_LE_UINT32(src + trackSize);
	trackSize += (l + 4);

	_driver->setMusicLoop(loop);
	_driver->startMusicTrack(src, trackSize, startTick);
}

void SoundTowns::fadeOutSoundEffects() {
	for (int i = 127; i > 0; i-= 12) {
		_driver->chanVolume(0x46, i);
		_driver->chanVolume(0x47, i);
		_vm->delay(_vm->tickLength());
	}
	stopAllSoundEffects();
}

SoundPC98::SoundPC98(KyraEngine_v1 *vm, Audio::Mixer *mixer) :
	Sound(vm, mixer), _musicTrackData(0), _sfxTrackData(0), _lastTrack(-1), _driver(0), _currentResourceSet(0) {
	memset(&_resInfo, 0, sizeof(_resInfo));
}

SoundPC98::~SoundPC98() {
	delete[] _musicTrackData;
	delete[] _sfxTrackData;
	delete _driver;
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

bool SoundPC98::init() {
	_driver = new TownsPC98_AudioDriver(_mixer, TownsPC98_AudioDriver::kType26);
	bool reslt = _driver->init();
	updateVolumeSettings();
	return reslt;
}

void SoundPC98::initAudioResourceInfo(int set, void *info) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		delete _resInfo[set];
		_resInfo[set] = info ? new Common::String(((SoundResourceInfo_PC98*)info)->pattern) : 0;
	}
}

void SoundPC98::selectAudioResourceSet(int set) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		if (_resInfo[set])
			_currentResourceSet = set;
	}
}

bool SoundPC98::hasSoundFile(uint file) const {
	return true;
}

void SoundPC98::loadSoundFile(uint) {
	if (_currentResourceSet == kMusicIntro) {
		delete[] _sfxTrackData;
		_sfxTrackData = 0;

		int dataSize = 0;
		const uint8 *tmp = _vm->staticres()->loadRawData(k1PC98IntroSfx, dataSize);

		if (!tmp) {
			warning("Could not load static intro sound effects data\n");
			return;
		}

		_sfxTrackData = new uint8[dataSize];
		memcpy(_sfxTrackData, tmp, dataSize);
	}
}

void SoundPC98::loadSoundFile(Common::String file) {
	delete[] _sfxTrackData;
	_sfxTrackData = _vm->resource()->fileData(file.c_str(), 0);
}

void SoundPC98::playTrack(uint8 track) {
	track -= 1;

	if (track == _lastTrack && _musicEnabled)
		return;

	beginFadeOut();

	Common::String musicFile = Common::String::format(resPattern(), track);
	delete[] _musicTrackData;
	_musicTrackData = _vm->resource()->fileData(musicFile.c_str(), 0);
	if (_musicEnabled)
		_driver->loadMusicData(_musicTrackData);

	_lastTrack = track;
}

void SoundPC98::haltTrack() {
	_lastTrack = -1;
	g_system->getAudioCDManager()->stop();
	g_system->getAudioCDManager()->updateCD();
	_driver->reset();
}

void SoundPC98::beginFadeOut() {
	if (!_driver->musicPlaying())
		return;

	for (int i = 0; i < 20; i++) {
		_driver->fadeStep();
		_vm->delay(32);
	}
	haltTrack();
}

void SoundPC98::playSoundEffect(uint8 track, uint8) {
	if (!_sfxTrackData)
		return;

	_driver->loadSoundEffectData(_sfxTrackData, track);
}

void SoundPC98::updateVolumeSettings() {
	if (!_driver)
		return;

	bool mute = false;
	_driver->setSoundEffectVolume(ConfMan.getInt("sfx_volume"));
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_driver->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	_driver->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

//	KYRA 2

SoundTownsPC98_v2::SoundTownsPC98_v2(KyraEngine_v1 *vm, Audio::Mixer *mixer) :
	Sound(vm, mixer), _currentSFX(0), _musicTrackData(0), _sfxTrackData(0), _lastTrack(-1), _driver(0), _useFmSfx(false), _currentResourceSet(0) {
	memset(&_resInfo, 0, sizeof(_resInfo));
}

SoundTownsPC98_v2::~SoundTownsPC98_v2() {
	delete[] _musicTrackData;
	delete[] _sfxTrackData;
	delete _driver;
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

bool SoundTownsPC98_v2::init() {
	_driver = new TownsPC98_AudioDriver(_mixer, _vm->gameFlags().platform == Common::kPlatformPC98 ?
		TownsPC98_AudioDriver::kType86 : TownsPC98_AudioDriver::kTypeTowns);

	if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
		_vm->checkCD();
		// FIXME: While checking for 'track1.XXX(X)' looks like
		// a good idea, we should definitely not be doing this
		// here. Basically our filenaming scheme could change
		// or we could add support for other audio formats. Also
		// this misses the possibility that we play the tracks
		// right off CD. So we should find another way to
		// check if we have access to CD audio.
		Resource *r = _vm->resource();
		if (_musicEnabled &&
			(r->exists("track1.mp3") || r->exists("track1.ogg") || r->exists("track1.flac") || r->exists("track1.fla")))
				_musicEnabled = 2;
		else
			_musicEnabled = 1;
		_useFmSfx = false;

	} else {
		_useFmSfx = true;
	}

	bool reslt = _driver->init();
	updateVolumeSettings();
	return reslt;
}

void SoundTownsPC98_v2::initAudioResourceInfo(int set, void *info) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		delete _resInfo[set];
		_resInfo[set] = info ? new SoundResourceInfo_TownsPC98V2(*(SoundResourceInfo_TownsPC98V2*)info) : 0;
	}
}

void SoundTownsPC98_v2::selectAudioResourceSet(int set) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		if (_resInfo[set])
			_currentResourceSet = set;
	}
}

bool SoundTownsPC98_v2::hasSoundFile(uint file) const {
	if (file < res()->fileListSize)
		return (res()->fileList[file] != 0);
	return false;
}

void SoundTownsPC98_v2::loadSoundFile(Common::String file) {
	delete[] _sfxTrackData;
	_sfxTrackData = _vm->resource()->fileData(file.c_str(), 0);
}

void SoundTownsPC98_v2::process() {
	g_system->getAudioCDManager()->updateCD();
}

void SoundTownsPC98_v2::playTrack(uint8 track) {
	if (track == _lastTrack && _musicEnabled)
		return;

	int trackNum = -1;
	if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
		for (uint i = 0; i < res()->cdaTableSize; i++) {
			if (track == (uint8) READ_LE_UINT16(&res()->cdaTable[i * 2])) {
				trackNum = (int) READ_LE_UINT16(&res()->cdaTable[i * 2 + 1]) - 1;
				break;
			}
		}
	}

	beginFadeOut();

	Common::String musicFile = res()->pattern ? Common::String::format(res()->pattern, track) : (res()->fileList ? res()->fileList[track] : 0);
	if (musicFile.empty())
		return;

	delete[] _musicTrackData;

	_musicTrackData = _vm->resource()->fileData(musicFile.c_str(), 0);
	_driver->loadMusicData(_musicTrackData, true);

	if (_musicEnabled == 2 && trackNum != -1) {
		g_system->getAudioCDManager()->play(trackNum+1, _driver->looping() ? -1 : 1, 0, 0);
		g_system->getAudioCDManager()->updateCD();
	} else if (_musicEnabled) {
		_driver->cont();
	}

	_lastTrack = track;
}

void SoundTownsPC98_v2::haltTrack() {
	_lastTrack = -1;
	g_system->getAudioCDManager()->stop();
	g_system->getAudioCDManager()->updateCD();
	_driver->reset();
}

void SoundTownsPC98_v2::beginFadeOut() {
	if (!_driver->musicPlaying())
		return;

	for (int i = 0; i < 20; i++) {
		_driver->fadeStep();
		_vm->delay(32);
	}

	haltTrack();
}

int32 SoundTownsPC98_v2::voicePlay(const char *file, Audio::SoundHandle *handle, uint8 volume, uint8 priority, bool) {
	static const uint16 rates[] = { 0x10E1, 0x0CA9, 0x0870, 0x0654, 0x0438, 0x032A, 0x021C, 0x0194 };
	static const char patternHOF[] = "%s.PCM";
	static const char patternLOL[] = "%s.VOC";

	int h = 0;
	if (_currentSFX) {
		while (h < kNumChannelHandles && _mixer->isSoundHandleActive(_soundChannels[h].handle))
			h++;

		if (h >= kNumChannelHandles) {
			h = 0;
			while (h < kNumChannelHandles && _soundChannels[h].priority > priority)
				++h;
			if (h < kNumChannelHandles)
				voiceStop(&_soundChannels[h].handle);
		}

		if (h >= kNumChannelHandles)
			return 0;
	}

	Common::String fileName = Common::String::format( _vm->game() == GI_LOL ? patternLOL : patternHOF, file);

	uint8 *data = _vm->resource()->fileData(fileName.c_str(), 0);
	uint8 *src = data;
	if (!src)
		return 0;

	uint16 sfxRate = rates[READ_LE_UINT16(src)];
	src += 2;
	bool compressed = (READ_LE_UINT16(src) & 1) ? true : false;
	src += 2;
	uint32 outsize = READ_LE_UINT32(src);
	uint8 *sfx = (uint8 *)malloc(outsize);
	uint8 *dst = sfx;
	src += 4;

	if (compressed) {
		for (uint32 i = outsize; i;) {
			uint8 cnt = *src++;
			if (cnt & 0x80) {
				cnt &= 0x7F;
				memset(dst, *src++, cnt);
			} else {
				memcpy(dst, src, cnt);
				src += cnt;
			}
			dst += cnt;
			i -= cnt;
		}
	} else {
		memcpy(dst, src, outsize);
	}

	for (uint32 i = 0; i < outsize; i++) {
		uint8 cmd = sfx[i];
		if (cmd & 0x80) {
			cmd = ~cmd;
		} else {
			cmd |= 0x80;
			if (cmd == 0xff)
				cmd--;
		}
		if (cmd < 0x80)
			cmd = 0x80 - cmd;
		sfx[i] = cmd;
	}

	_currentSFX = Audio::makeRawStream(sfx, outsize, sfxRate * 10, Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundChannels[h].handle, _currentSFX, -1, volume);
	_soundChannels[h].priority = priority;
	if (handle)
		*handle = _soundChannels[h].handle;

	delete[] data;
	return 1;
}

void SoundTownsPC98_v2::playSoundEffect(uint8 track, uint8) {
	if (!_useFmSfx || !_sfxTrackData)
		return;

	_driver->loadSoundEffectData(_sfxTrackData, track);
}

void SoundTownsPC98_v2::updateVolumeSettings() {
	if (!_driver)
		return;

	bool mute = false;
	_driver->setSoundEffectVolume(ConfMan.getInt("sfx_volume"));
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	_driver->setMusicVolume((mute ? 0 : ConfMan.getInt("music_volume")));
	_driver->setSoundEffectVolume((mute ? 0 : ConfMan.getInt("sfx_volume")));
}

} // End of namespace Kyra

#undef EUPHONY_FADEOUT_TICKS
