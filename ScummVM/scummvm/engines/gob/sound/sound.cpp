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

#include "gob/gob.h"
#include "gob/sound/sound.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/game.h"
#include "gob/inter.h"

#include "gob/sound/pcspeaker.h"
#include "gob/sound/soundblaster.h"
#include "gob/sound/adlplayer.h"
#include "gob/sound/musplayer.h"
#include "gob/sound/infogrames.h"
#include "gob/sound/protracker.h"
#include "gob/sound/cdrom.h"

namespace Gob {

Sound::Sound(GobEngine *vm) : _vm(vm) {
	_pcspeaker = new PCSpeaker(*_vm->_mixer);
	_blaster = new SoundBlaster(*_vm->_mixer);

	_adlPlayer = 0;
	_mdyPlayer = 0;
	_infogrames = 0;
	_protracker = 0;
	_cdrom = 0;
	_bgatmos = 0;

	_hasAdLib = (!_vm->_noMusic && _vm->hasAdLib());

	_hasAdLibBg = _hasAdLib;

	if (!_vm->_noMusic && (_vm->getPlatform() == Common::kPlatformAmiga)) {
		_infogrames = new Infogrames(*_vm->_mixer);
		_protracker = new Protracker(*_vm->_mixer);
	}
	if (_vm->isCD())
		_cdrom = new CDROM;
	if (_vm->getGameType() == kGameTypeWoodruff)
		_bgatmos = new BackgroundAtmosphere(*_vm->_mixer);
	if ((_vm->getGameType() == kGameTypeUrban) ||
	    (_vm->getGameType() == kGameTypeAdibou2)) {
		_bgatmos = new BackgroundAtmosphere(*_vm->_mixer);
		_bgatmos->setShadable(false);
	}
}

Sound::~Sound() {
	delete _pcspeaker;
	delete _blaster;
	delete _adlPlayer;
	delete _mdyPlayer;
	delete _infogrames;
	delete _protracker;
	delete _cdrom;
	delete _bgatmos;

	for (int i = 0; i < kSoundsCount; i++)
		_sounds[i].free();
}

void Sound::convToSigned(byte *buffer, int length) {
	while (length-- > 0)
		*buffer++ ^= 0x80;
}

SoundDesc *Sound::sampleGetBySlot(int slot) {
	if ((slot < 0) || (slot >= kSoundsCount))
		return 0;

	return &_sounds[slot];
}

const SoundDesc *Sound::sampleGetBySlot(int slot) const {
	if ((slot < 0) || (slot >= kSoundsCount))
		return 0;

	return &_sounds[slot];
}

int Sound::sampleGetNextFreeSlot() const {
	for (int i = 0; i < kSoundsCount; i++)
		if (_sounds[i].empty())
			return i;

	return -1;
}

bool Sound::sampleLoad(SoundDesc *sndDesc, SoundType type, const char *fileName) {
	if (!sndDesc)
		return false;

	debugC(2, kDebugSound, "Loading sample \"%s\"", fileName);

	int32 size;
	byte *data = _vm->_dataIO->getFile(fileName, size);

	if (!data || !sndDesc->load(type, data, size)) {
		delete data;

		warning("Sound::sampleLoad(): Failed to load sound \"%s\"", fileName);
		return false;
	}

	return true;
}

void Sound::sampleFree(SoundDesc *sndDesc, bool noteAdLib, int index) {
	if (!sndDesc || sndDesc->empty())
		return;

	if (sndDesc->getType() == SOUND_ADL) {

		if (noteAdLib) {
			if (_adlPlayer)
				if ((index == -1) || (_adlPlayer->getIndex() == index))
					_adlPlayer->unload();
		}

	} else {

		if (_blaster)
			_blaster->stopSound(0, sndDesc);

	}

	sndDesc->free();
}

void Sound::speakerOn(int16 frequency, int32 length) {
	if (!_pcspeaker)
		return;

	debugC(1, kDebugSound, "PCSpeaker: Playing tone (%d, %d)", frequency, length);

	_pcspeaker->speakerOn(frequency, length);
}

void Sound::speakerOff() {
	if (!_pcspeaker)
		return;

	debugC(1, kDebugSound, "PCSpeaker: Stopping tone");

	_pcspeaker->speakerOff();
}

void Sound::speakerOnUpdate(uint32 millis) {
	if (!_pcspeaker)
		return;

	_pcspeaker->onUpdate(millis);
}

bool Sound::infogramesLoadInstruments(const char *fileName) {
	if (!_infogrames)
		return false;

	debugC(1, kDebugSound, "Infogrames: Loading instruments \"%s\"", fileName);

	return _infogrames->loadInstruments(fileName);
}

bool Sound::infogramesLoadSong(const char *fileName) {
	if (!_infogrames)
		return false;

	debugC(1, kDebugSound, "Infogrames: Loading song \"%s\"", fileName);

	return _infogrames->loadSong(fileName);
}

bool Sound::protrackerPlay(const char *fileName) {
	if (!_protracker)
		return false;

	debugC(1, kDebugSound, "Protracker: Playing song \"%s\"", fileName);

	return _protracker->play(fileName);
}

void Sound::protrackerStop() {
	if (!_protracker)
		return;

	debugC(1, kDebugSound, "Protracker: Stopping playback");

	_protracker->stop();
}

void Sound::infogramesPlay() {
	if (!_infogrames)
		return;

	debugC(1, kDebugSound, "Infogrames: Starting playback");

	_infogrames->play();
}

void Sound::infogramesStop() {
	if (!_infogrames)
		return;

	debugC(1, kDebugSound, "Infogrames: Stopping playback");

	_infogrames->stop();
}

bool Sound::adlibLoadADL(const char *fileName) {
	if (!_hasAdLib)
		return false;

	if (!_adlPlayer)
		_adlPlayer = new ADLPlayer(*_vm->_mixer);

	debugC(1, kDebugSound, "AdLib: Loading ADL data (\"%s\")", fileName);

	Common::SeekableReadStream *stream = _vm->_dataIO->getFile(fileName);
	if (!stream) {
		warning("Can't open ADL file \"%s\"", fileName);
		return false;
	}

	bool loaded = _adlPlayer->load(*stream);

	delete stream;

	return loaded;
}

bool Sound::adlibLoadADL(byte *data, uint32 size, int index) {
	if (!_hasAdLib)
		return false;

	if (!_adlPlayer)
		_adlPlayer = new ADLPlayer(*_vm->_mixer);

	debugC(1, kDebugSound, "AdLib: Loading ADL data (%d)", index);

	return _adlPlayer->load(data, size, index);
}

void Sound::adlibUnload() {
	if (!_hasAdLib)
		return;

	debugC(1, kDebugSound, "AdLib: Unloading data");

	if (_adlPlayer)
		_adlPlayer->unload();
	if (_mdyPlayer)
		_mdyPlayer->unload();
}

bool Sound::adlibLoadMDY(const char *fileName) {
	if (!_hasAdLib)
		return false;

	createMDYPlayer();

	debugC(1, kDebugSound, "AdLib: Loading MDY data (\"%s\")", fileName);

	Common::SeekableReadStream *stream = _vm->_dataIO->getFile(fileName);
	if (!stream) {
		warning("Can't open MDY file \"%s\"", fileName);
		return false;
	}

	bool loaded = _mdyPlayer->loadMUS(*stream);

	delete stream;

	return loaded;
}

bool Sound::adlibLoadTBR(const char *fileName) {
	if (!_hasAdLib)
		return false;

	createMDYPlayer();

	Common::SeekableReadStream *stream = _vm->_dataIO->getFile(fileName);
	if (!stream) {
		warning("Can't open TBR file \"%s\"", fileName);
		return false;
	}

	debugC(1, kDebugSound, "AdLib: Loading MDY instruments (\"%s\")", fileName);

	bool loaded = _mdyPlayer->loadSND(*stream);

	delete stream;

	return loaded;
}

void Sound::adlibPlayTrack(const char *trackname) {
	if (!_hasAdLib)
		return;

	createADLPlayer();

	if (_adlPlayer->isPlaying())
		return;

	if (adlibLoadADL(trackname))
		adlibPlay();
}

void Sound::adlibPlayBgMusic() {
	if (!_hasAdLib || _hasAdLibBg)
		return;

	createADLPlayer();

	static const char *const tracksMac[] = {
//		"musmac1.adl", // This track seems to be missing instruments...
		"musmac2.adl",
		"musmac3.adl",
		"musmac4.adl",
		"musmac5.adl",
		"musmac6.adl"
	};

	static const char *const tracksWin[] = {
		"musmac1.mid",
		"musmac2.mid",
		"musmac3.mid",
		"musmac4.mid",
		"musmac5.mid"
	};

	const char *track = 0;
	if (_vm->getPlatform() == Common::kPlatformWindows)
		track = tracksWin[_vm->_util->getRandom(ARRAYSIZE(tracksWin))];
	else
		track = tracksMac[_vm->_util->getRandom(ARRAYSIZE(tracksMac))];

	if (!track || !_vm->_dataIO->hasFile(track)) {
		_hasAdLibBg = false;
		return;
	}

	adlibPlayTrack(track);
}

void Sound::adlibPlay() {
	if (!_hasAdLib)
		return;

	debugC(1, kDebugSound, "AdLib: Starting playback");

	if (_adlPlayer)
		_adlPlayer->startPlay();
	if (_mdyPlayer)
		_mdyPlayer->startPlay();
}

void Sound::adlibStop() {
	if (!_hasAdLib)
		return;

	debugC(1, kDebugSound, "AdLib: Stopping playback");

	if (_adlPlayer)
		_adlPlayer->stopPlay();
	if (_mdyPlayer)
		_mdyPlayer->stopPlay();
}

bool Sound::adlibIsPlaying() const {
	if (!_hasAdLib)
		return false;

	if (_adlPlayer && _adlPlayer->isPlaying())
		return true;
	if (_mdyPlayer && _mdyPlayer->isPlaying())
		return true;

	return false;
}

int Sound::adlibGetIndex() const {
	if (!_hasAdLib)
		return -1;

	if (_adlPlayer)
		return _adlPlayer->getIndex();

	return -1;
}

int32 Sound::adlibGetRepeating() const {
	if (!_hasAdLib)
		return false;

	if (_adlPlayer)
		return _adlPlayer->getRepeating();
	if (_mdyPlayer)
		return _mdyPlayer->getRepeating();

	return false;
}

void Sound::adlibSetRepeating(int32 repCount) {
	if (!_hasAdLib)
		return;

	if (_adlPlayer)
		_adlPlayer->setRepeating(repCount);
	if (_mdyPlayer)
		_mdyPlayer->setRepeating(repCount);
}

void Sound::blasterPlay(SoundDesc *sndDesc, int16 repCount,
		int16 frequency, int16 fadeLength) {
	if (!_blaster || !sndDesc)
		return;

	debugC(1, kDebugSound, "SoundBlaster: Playing sample (%d, %d, %d)",
			repCount, frequency, fadeLength);

	blasterStopComposition();

	_blaster->playSample(*sndDesc, repCount, frequency, fadeLength);
}

void Sound::blasterRepeatComposition(int32 repCount) {
	_blaster->repeatComposition(repCount);
}

void Sound::blasterStop(int16 fadeLength, SoundDesc *sndDesc) {
	if (!_blaster)
		return;

	debugC(1, kDebugSound, "SoundBlaster: Stopping playback");

	_blaster->stopSound(fadeLength, sndDesc);
}

void Sound::blasterPlayComposition(const int16 *composition, int16 freqVal,
		SoundDesc *sndDescs, int8 sndCount) {
	if (!_blaster)
		return;

	debugC(1, kDebugSound, "SoundBlaster: Playing composition (%d, %d)",
			freqVal, sndCount);

	blasterWaitEndPlay();
	_blaster->stopComposition();

	if (!sndDescs)
		sndDescs = _sounds;

	_blaster->playComposition(composition, freqVal, sndDescs, sndCount);
}

void Sound::blasterStopComposition() {
	if (!_blaster)
		return;

	debugC(1, kDebugSound, "SoundBlaster: Stopping composition");

	_blaster->stopComposition();
}

char Sound::blasterPlayingSound() const {
	if (!_blaster)
		return 0;

	return _blaster->getPlayingSound();
}

void Sound::blasterSetRepeating(int32 repCount) {
	if (!_blaster)
		return;

	_blaster->setRepeating(repCount);
}

void Sound::blasterWaitEndPlay(bool interruptible, bool stopComp) {
	if (!_blaster)
		return;

	debugC(1, kDebugSound, "SoundBlaster: Waiting for playback to end");

	if (stopComp)
		_blaster->endComposition();

	while (_blaster->isPlaying() && !_vm->shouldQuit()) {
		if (interruptible && (_vm->_util->checkKey() == kKeyEscape)) {
			WRITE_VAR(57, (uint32) -1);
			return;
		}
		_vm->_util->longDelay(200);
	}

	_blaster->stopSound(0);
}

void Sound::cdLoadLIC(const Common::String &fname) {
	if (!_cdrom)
		return;

	debugC(1, kDebugSound, "CDROM: Loading LIC \"%s\"", fname.c_str());

	Common::SeekableReadStream *stream = _vm->_dataIO->getFile(fname);
	if (!stream)
		return;

	_cdrom->readLIC(*stream);

	delete stream;
}

void Sound::cdUnloadLIC() {
	if (!_cdrom)
		return;

	debugC(1, kDebugSound, "CDROM: Unloading LIC");

	_cdrom->freeLICBuffer();
}

void Sound::cdPlayBgMusic() {
	if (!_cdrom)
		return;

	static const char *const tracks[][2] = {
		{"avt00.tot",  "mine"},
		{"avt001.tot", "nuit"},
		{"avt002.tot", "campagne"},
		{"avt003.tot", "extsor1"},
		{"avt004.tot", "interieure"},
		{"avt005.tot", "zombie"},
		{"avt006.tot", "zombie"},
		{"avt007.tot", "campagne"},
		{"avt008.tot", "campagne"},
		{"avt009.tot", "extsor1"},
		{"avt010.tot", "extsor1"},
		{"avt011.tot", "interieure"},
		{"avt012.tot", "zombie"},
		{"avt014.tot", "nuit"},
		{"avt015.tot", "interieure"},
		{"avt016.tot", "statue"},
		{"avt017.tot", "zombie"},
		{"avt018.tot", "statue"},
		{"avt019.tot", "mine"},
		{"avt020.tot", "statue"},
		{"avt021.tot", "mine"},
		{"avt022.tot", "zombie"}
	};

	for (int i = 0; i < ARRAYSIZE(tracks); i++)
		if (_vm->isCurrentTot(tracks[i][0])) {
			debugC(1, kDebugSound, "CDROM: Playing background music \"%s\" (\"%s\")", tracks[i][1], tracks[i][0]);
			_cdrom->startTrack(tracks[i][1]);
			break;
		}
}

void Sound::cdPlayMultMusic() {
	if (!_cdrom)
		return;

	static const char *const tracks[][6] = {
		{"avt005.tot", "fra1", "all1", "ang1", "esp1", "ita1"},
		{"avt006.tot", "fra2", "all2", "ang2", "esp2", "ita2"},
		{"avt012.tot", "fra3", "all3", "ang3", "esp3", "ita3"},
		{"avt016.tot", "fra4", "all4", "ang4", "esp4", "ita4"},
		{"avt019.tot", "fra5", "all5", "ang5", "esp5", "ita5"},
		{"avt022.tot", "fra6", "all6", "ang6", "esp6", "ita6"}
	};

	// Default to "ang?" for other languages (including EN_USA)
	int language = _vm->_global->_language <= 4 ? _vm->_global->_language : 2;
	for (int i = 0; i < ARRAYSIZE(tracks); i++)
		if (_vm->isCurrentTot(tracks[i][0])) {
			debugC(1, kDebugSound, "CDROM: Playing mult music \"%s\" (\"%s\")", tracks[i][language + 1], tracks[i][0]);
			_cdrom->startTrack(tracks[i][language + 1]);
			break;
		}
}

void Sound::cdPlay(const Common::String &trackName) {
	if (!_cdrom)
		return;
	debugC(1, kDebugSound, "CDROM: Playing track \"%s\"", trackName.c_str());

// WORKAROUND - In Fascination CD, in the storage room, a track has the wrong
// name in the scripts, and therefore doesn't play. This fixes the problem.
	if ((_vm->getGameType() == kGameTypeFascination) && trackName.equalsIgnoreCase("boscle"))
		_cdrom->startTrack("bosscle");
	else
		_cdrom->startTrack(trackName.c_str());
}

void Sound::cdStop() {
	if (!_cdrom)
		return;

	debugC(1, kDebugSound, "CDROM: Stopping playback");
	_cdrom->stopPlaying();
}

bool Sound::cdIsPlaying() const {
	if (!_cdrom)
		return false;

	return _cdrom->isPlaying();
}

int32 Sound::cdGetTrackPos(const char *keyTrack) const {
	if (!_cdrom)
		return -1;

	return _cdrom->getTrackPos(keyTrack);
}

const char *Sound::cdGetCurrentTrack() const {
	if (!_cdrom)
		return "";

	return _cdrom->getCurTrack();
}

void Sound::cdTest(int trySubst, const char *label) {
	if (!_cdrom)
		return;

	_cdrom->testCD(trySubst, label);
}

void Sound::bgPlay(const char *file, SoundType type) {
	if (!_bgatmos)
		return;

	debugC(1, kDebugSound, "BackgroundAtmosphere: Playing \"%s\"", file);

	_bgatmos->stopBA();
	_bgatmos->queueClear();

	SoundDesc *sndDesc = new SoundDesc;
	if (!sampleLoad(sndDesc, type, file)) {
		delete sndDesc;
		return;
	}

	_bgatmos->queueSample(*sndDesc);
	_bgatmos->playBA();
}

void Sound::bgPlay(const char *base, const char *ext, SoundType type, int count) {
	if (!_bgatmos)
		return;

	debugC(1, kDebugSound, "BackgroundAtmosphere: Playing \"%s\" (%d)", base, count);

	_bgatmos->stopBA();
	_bgatmos->queueClear();

	SoundDesc *sndDesc;

	for (int i = 1; i <= count; i++) {
		Common::String fileName = Common::String::format("%s%02d.%s", base, i, ext);

		sndDesc = new SoundDesc;
		if (sampleLoad(sndDesc, type, fileName.c_str()))
			_bgatmos->queueSample(*sndDesc);
		else
			delete sndDesc;
	}

	_bgatmos->playBA();
}

void Sound::bgStop() {
	if (!_bgatmos)
		return;

	debugC(1, kDebugSound, "BackgroundAtmosphere: Stopping playback");

	_bgatmos->stopBA();
	_bgatmos->queueClear();
}

void Sound::bgSetPlayMode(BackgroundAtmosphere::PlayMode mode) {
	if (!_bgatmos)
		return;

	_bgatmos->setPlayMode(mode);
}

void Sound::bgShade() {
	if (!_bgatmos)
		return;

	debugC(1, kDebugSound, "BackgroundAtmosphere: Shading playback");

	_bgatmos->shade();
}

void Sound::bgUnshade() {
	if (!_bgatmos)
		return;

	debugC(1, kDebugSound, "BackgroundAtmosphere: Unshading playback");

	_bgatmos->unshade();
}

void Sound::createMDYPlayer() {
	if (_mdyPlayer)
		return;

	delete _adlPlayer;
	_adlPlayer = 0;

	_mdyPlayer = new MUSPlayer(*_vm->_mixer);
}

void Sound::createADLPlayer() {
	if (_adlPlayer)
		return;

	delete _mdyPlayer;
	_mdyPlayer= 0;

	_adlPlayer = new ADLPlayer(*_vm->_mixer);
}

} // End of namespace Gob
