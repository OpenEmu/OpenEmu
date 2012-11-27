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

#include "engines/engine.h"
#include "scumm/player_v4a.h"
#include "scumm/scumm.h"

#include "common/file.h"

namespace Scumm {

Player_V4A::Player_V4A(ScummEngine *scumm, Audio::Mixer *mixer)
	: _vm(scumm),
	  _mixer(mixer),
	  _tfmxMusic(_mixer->getOutputRate(), true),
	  _tfmxSfx(_mixer->getOutputRate(), true),
	  _musicHandle(),
	  _sfxHandle(),
	  _musicId(),
	  _sfxSlots(),
	  _initState(0),
	  _signal(0) {

	assert(scumm);
	assert(mixer);
	assert(_vm->_game.id == GID_MONKEY_VGA);
	_tfmxMusic.setSignalPtr(&_signal, 1);
}

bool Player_V4A::init() {
	if (_vm->_game.id != GID_MONKEY_VGA)
		error("player_v4a - unknown game");

	Common::File fileMdat, fileSample;

	if (fileMdat.open("music.dat") && fileSample.open("sample.dat")) {
		// explicitly request that no instance delets the resources automatically
		if (_tfmxMusic.load(fileMdat, fileSample, false)) {
			_tfmxSfx.setModuleData(_tfmxMusic);
			return true;
		}
	} else
		warning("player_v4a: couldnt load one of the music resources: music.dat, sample.dat");

	return false;
}

Player_V4A::~Player_V4A() {
	_mixer->stopHandle(_musicHandle);
	_mixer->stopHandle(_sfxHandle);
	_tfmxMusic.freeResources();
}

void Player_V4A::setMusicVolume(int vol) {
	debug(5, "player_v4a: setMusicVolume %i", vol);
}

void Player_V4A::stopAllSounds() {
	debug(5, "player_v4a: stopAllSounds");
	if (_initState > 0) {
		_tfmxMusic.stopSong();
		_signal = 0;
		_musicId = 0;

		_tfmxSfx.stopSong();
		clearSfxSlots();
	} else
		_mixer->stopHandle(_musicHandle);
}

void Player_V4A::stopSound(int nr) {
	debug(5, "player_v4a: stopSound %d", nr);
	if (nr == 0)
		return;
	if (nr == _musicId) {
		_musicId = 0;
		if (_initState > 0)
			_tfmxMusic.stopSong();
		else
			_mixer->stopHandle(_musicHandle);
		_signal = 0;
	} else {
		const int chan = getSfxChan(nr);
		if (chan != -1) {
			setSfxSlot(chan, 0);
			_tfmxSfx.stopMacroEffect(chan);
		}
	}
}

void Player_V4A::startSound(int nr) {
	static const int8 monkeyCommands[52] = {
		 -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,
		 -9, -10, -11, -12, -13, -14,  18,  17,
		-17, -18, -19, -20, -21, -22, -23, -24,
		-25, -26, -27, -28, -29, -30, -31, -32,
		-33,  16, -35,   0,   1,   2,   3,   7,
		  8,  10,  11,   4,   5,  14,  15,  12,
		  6,  13,   9,  19
	};

	const byte *ptr = _vm->getResourceAddress(rtSound, nr);
	assert(ptr);

	const int val = ptr[9];
	if (val < 0 || val >= ARRAYSIZE(monkeyCommands)) {
		warning("player_v4a: illegal Songnumber %i", val);
		return;
	}

	if (!_initState)
		_initState = init() ? 1 : -1;

	if (_initState < 0)
		return;

	int index = monkeyCommands[val];
	const byte type = ptr[6];
	if (index < 0) {	// SoundFX
		index = -index - 1;
		debug(3, "player_v4a: play %d: custom %i - %02X", nr, index, type);

		// start an empty Song so timing is setup
		if (_tfmxSfx.getSongIndex() < 0)
			_tfmxSfx.doSong(0x18);

		const int chan = _tfmxSfx.doSfx((uint16)index);
		if (chan >= 0 && chan < ARRAYSIZE(_sfxSlots))
			setSfxSlot(chan, nr, type);
		else
			warning("player_v4a: custom %i is not of required type", index);

		// the Tfmx-player never "ends" the output by itself, so this should be threadsafe
		if (!_mixer->isSoundHandleActive(_sfxHandle))
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, &_tfmxSfx, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);

	} else {	// Song
		debug(3, "player_v4a: play %d: song %i - %02X", nr, index, type);
		if (ptr[6] != 0x7F)
			warning("player_v4a: Song has wrong type");

		_tfmxMusic.doSong(index);
		_signal = 2;

		// the Tfmx-player never "ends" the output by itself, so this should be threadsafe
		if (!_mixer->isSoundHandleActive(_musicHandle))
			_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, &_tfmxMusic, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
		_musicId = nr;
	}
}

int Player_V4A::getMusicTimer() {
	// A workaround if the modplayer couldnt load the datafiles - just return a number big enough to pass all tests
	if (_initState < 0)
		return 2000;
	if (_musicId) {
		// The titlesong (and a few others) is running with ~70 ticks per second and the scale seems to be based on that.
		// The Game itself doesnt get the timing from the Tfmx Player however, so we just use the elapsed time
		// 357 ~ 1000 * 25 * (1 / 70)
		return _mixer->getSoundElapsedTime(_musicHandle) / 357;
	}
	return 0;
}

int Player_V4A::getSoundStatus(int nr) const {
	// For music the game queues a variable the Tfmx Player sets through a special command.
	// For sfx there seems to be no way to queue them, and the game doesnt try to.
	return (nr == _musicId) ? _signal : 0;
}

} // End of namespace Scumm
