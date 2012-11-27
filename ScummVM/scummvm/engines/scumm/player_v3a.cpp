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
#include "scumm/player_v3a.h"
#include "scumm/scumm.h"

namespace Scumm {

static const uint16 note_freqs[4][12] = {
	{0x06B0, 0x0650, 0x05F4, 0x05A0, 0x054C, 0x0500, 0x04B8, 0x0474, 0x0434, 0x03F8, 0x03C0, 0x0388},
	{0x0358, 0x0328, 0x02FA, 0x02D0, 0x02A6, 0x0280, 0x025C, 0x023A, 0x021A, 0x01FC, 0x01E0, 0x01C4},
	{0x01AC, 0x0194, 0x017D, 0x0168, 0x0153, 0x0140, 0x012E, 0x011D, 0x010D, 0x00FE, 0x00F0, 0x00E2},
	{0x00D6, 0x00CA, 0x00BE, 0x00B4, 0x00A9, 0x00A0, 0x0097, 0x008E, 0x0086, 0x007F, 0x00F0, 0x00E2}
};

Player_V3A::Player_V3A(ScummEngine *scumm, Audio::Mixer *mixer) {
	int i;
	_vm = scumm;
	for (i = 0; i < V3A_MAXMUS; i++) {
		_mus[i].id = 0;
		_mus[i].dur = 0;
	}
	for (i = 0; i < V3A_MAXSFX; i++) {
		_sfx[i].id = 0;
		_sfx[i].dur = 0;
	}

	_curSong = 0;
	_songData = NULL;
	_songPtr = 0;
	_songDelay = 0;

	_music_timer = 0;

	_isinit = false;

	_mod = new Player_MOD(mixer);
	_mod->setUpdateProc(update_proc, this, 60);
}

Player_V3A::~Player_V3A() {
	int i;
	delete _mod;
	if (_isinit) {
		for (i = 0; _wavetable[i] != NULL; i++) {
			for (int j = 0; j < 6; j++) {
				free(_wavetable[i]->_idat[j]);
				free(_wavetable[i]->_ldat[j]);
			}
			free(_wavetable[i]);
		}
		free(_wavetable);
	}
}

void Player_V3A::setMusicVolume (int vol) {
	_mod->setMusicVolume(vol);
}

int Player_V3A::getMusChan (int id) const {
	int i;
	for (i = 0; i < V3A_MAXMUS; i++) {
		if (_mus[i].id == id)
			break;
	}
	if (i == V3A_MAXMUS) {
		if (id == 0)
			warning("player_v3a - out of music channels");
		return -1;
	}
	return i;
}
int Player_V3A::getSfxChan (int id) const {
	int i;
	for (i = 0; i < V3A_MAXSFX; i++) {
		if (_sfx[i].id == id)
			break;
	}
	if (i == V3A_MAXSFX) {
		if (id == 0)
			warning("player_v3a - out of sfx channels");
		return -1;
	}
	return i;
}

void Player_V3A::stopAllSounds() {
	int i;
	for (i = 0; i < V3A_MAXMUS; i++) {
		if (_mus[i].id)
			_mod->stopChannel(_mus[i].id);
		_mus[i].id = 0;
		_mus[i].dur = 0;
	}
	_curSong = 0;
	_songPtr = 0;
	_songDelay = 0;
	_songData = NULL;
	for (i = 0; i < V3A_MAXSFX; i++) {
		if (_sfx[i].id)
			_mod->stopChannel(_sfx[i].id | 0x100);
		_sfx[i].id = 0;
		_sfx[i].dur = 0;
	}
}

void Player_V3A::stopSound(int nr) {
	int i;
	if (nr == 0) {	// Amiga Loom does this near the end, when Chaos casts SILENCE on Hetchel
		stopAllSounds();
		return;
	}
	if (nr == _curSong) {
		for (i = 0; i < V3A_MAXMUS; i++) {
			if (_mus[i].id)
				_mod->stopChannel(_mus[i].id);
			_mus[i].id = 0;
			_mus[i].dur = 0;
		}
		_curSong = 0;
		_songPtr = 0;
		_songDelay = 0;
		_songData = NULL;
	} else {
		i = getSfxChan(nr);
		if (i != -1) {
			_mod->stopChannel(nr | 0x100);
			_sfx[i].id = 0;
			_sfx[i].dur = 0;
		}
	}
}

void Player_V3A::startSound(int nr) {
	assert(_vm);
	byte *data = _vm->getResourceAddress(rtSound, nr);
	assert(data);

	if ((_vm->_game.id != GID_INDY3) && (_vm->_game.id != GID_LOOM))
		error("player_v3a - unknown game");

	if (!_isinit) {
		int i;
		unsigned char *ptr;
		int offset = 4;
		int numInstruments;

		if (_vm->_game.id == GID_INDY3) {
			ptr = _vm->getResourceAddress(rtSound, 83);
			numInstruments = 12;
		} else {
			ptr = _vm->getResourceAddress(rtSound, 79);
			numInstruments = 9;
		}
		assert(ptr);
		_wavetable = (instData **)malloc((numInstruments + 1) * sizeof(void *));
		for (i = 0; i < numInstruments; i++) {
			_wavetable[i] = (instData *)malloc(sizeof(instData));
			for (int j = 0; j < 6; j++) {
				int off, len;
				off = READ_BE_UINT16(ptr + offset + 0);
				_wavetable[i]->_ilen[j] = len = READ_BE_UINT16(ptr + offset + 2);
				if (len) {
					_wavetable[i]->_idat[j] = (char *)malloc(len);
					memcpy(_wavetable[i]->_idat[j],ptr + off,len);
				} else	_wavetable[i]->_idat[j] = NULL;
				off = READ_BE_UINT16(ptr + offset + 4);
				_wavetable[i]->_llen[j] = len = READ_BE_UINT16(ptr + offset + 6);
				if (len) {
					_wavetable[i]->_ldat[j] = (char *)malloc(len);
					memcpy(_wavetable[i]->_ldat[j],ptr + off,len);
				} else	_wavetable[i]->_ldat[j] = NULL;
				_wavetable[i]->_oct[j] = READ_BE_UINT16(ptr + offset + 8);
				offset += 10;
			}
			if (_vm->_game.id == GID_INDY3) {
				_wavetable[i]->_pitadjust = 0;
				offset += 2;
			} else {
				_wavetable[i]->_pitadjust = READ_BE_UINT16(ptr + offset + 2);
				offset += 4;
			}
		}
		_wavetable[i] = NULL;
		_isinit = true;
	}

	if (getSoundStatus(nr))
		stopSound(nr);	// if a sound is playing, restart it

	if (data[26]) {
		if (_curSong)
			stopSound(_curSong);
		_curSong = nr;
		_songData = data;
		_songPtr = 0x1C;
		_songDelay = 1;
		_music_timer = 0;
	} else {
		int size = READ_BE_UINT16(data + 12);
		int rate = 3579545 / READ_BE_UINT16(data + 20);
		char *sound = (char *)malloc(size);
		int vol = (data[24] << 1) | (data[24] >> 5);	// if I boost this to 0-255, it gets too loud and starts to clip
		memcpy(sound, data + READ_BE_UINT16(data + 8), size);
		int loopStart = 0, loopEnd = 0;
		int loopcount = data[27];
		if (loopcount > 1) {
			loopStart = READ_BE_UINT16(data + 10) - READ_BE_UINT16(data + 8);
			loopEnd = READ_BE_UINT16(data + 14);
		}
		int i = getSfxChan();
		if (i == -1) {
			free(sound);
			return;
		}
		_sfx[i].id = nr;
		_sfx[i].dur = 1 + loopcount * 60 * size / rate;
		if (READ_BE_UINT16(data + 16)) {
			_sfx[i].rate = READ_BE_UINT16(data + 20) << 16;
			_sfx[i].delta = (int32)READ_BE_UINT32(data + 32);
			_sfx[i].dur = READ_BE_UINT32(data + 40);
		} else {
			_sfx[i].delta = 0;
		}
		_mod->startChannel(nr | 0x100, sound, size, rate, vol, loopStart, loopEnd);
	}
}

void Player_V3A::update_proc(void *param) {
	((Player_V3A *)param)->playMusic();
}

void Player_V3A::playMusic() {
	int i;
	for (i = 0; i < V3A_MAXMUS; i++) {
		if (_mus[i].id) {
			_mus[i].dur--;
			if (_mus[i].dur)
				continue;
			_mod->stopChannel(_mus[i].id);
			_mus[i].id = 0;
		}
	}
	for (i = 0; i < V3A_MAXSFX; i++) {
		if (_sfx[i].id) {
			if (_sfx[i].delta) {
				uint16 oldrate = _sfx[i].rate >> 16;
				_sfx[i].rate += _sfx[i].delta;
				if (_sfx[i].rate < (55 << 16))
					_sfx[i].rate = 55 << 16;	// at rates below 55, frequency
				uint16 newrate = _sfx[i].rate >> 16;	// exceeds 65536, which is bad
				if (oldrate != newrate)
					_mod->setChannelFreq(_sfx[i].id | 0x100, 3579545 / newrate);
			}
			_sfx[i].dur--;
			if (_sfx[i].dur)
				continue;
			_mod->stopChannel(_sfx[i].id | 0x100);
			_sfx[i].id = 0;
		}
	}

	_music_timer++;
	if (!_curSong)
		return;
	if (_songDelay && --_songDelay)
		return;
	if (_songPtr == 0) {
		// at the end of the song, and it wasn't looped - kill it
		_curSong = 0;
		return;
	}
	while (1) {
		int inst, pit, vol, dur, oct;
		inst = _songData[_songPtr++];
		if ((inst & 0xF0) != 0x80) {
			// tune is at the end - figure out what's still playing
			// and see how long we have to wait until we stop/restart
			for (i = 0; i < V3A_MAXMUS; i++) {
				if (_songDelay < _mus[i].dur)
					_songDelay = _mus[i].dur;
			}
			if (inst == 0xFB)	// it's a looped song, restart it afterwards
				_songPtr = 0x1C;
			else	_songPtr = 0;	// otherwise, terminate it
			break;
		}
		inst &= 0xF;
		pit = _songData[_songPtr++];
		vol = _songData[_songPtr++] & 0x7F;	// if I boost this to 0-255, it gets too loud and starts to clip
		dur = _songData[_songPtr++];
		if (pit == 0) {
			_songDelay = dur;
			break;
		}
		pit += _wavetable[inst]->_pitadjust;
		oct = (pit / 12) - 2;
		pit = pit % 12;
		if (oct < 0)
			oct = 0;
		if (oct > 5)
			oct = 5;
		int rate = 3579545 / note_freqs[_wavetable[inst]->_oct[oct]][pit];
		if (!_wavetable[inst]->_llen[oct])
			dur = _wavetable[inst]->_ilen[oct] * 60 / rate;
		char *data = (char *)malloc(_wavetable[inst]->_ilen[oct] + _wavetable[inst]->_llen[oct]);
		if (_wavetable[inst]->_idat[oct])
			memcpy(data, _wavetable[inst]->_idat[oct], _wavetable[inst]->_ilen[oct]);
		if (_wavetable[inst]->_ldat[oct])
			memcpy(data + _wavetable[inst]->_ilen[oct], _wavetable[inst]->_ldat[oct], _wavetable[inst]->_llen[oct]);

		i = getMusChan();
		if (i == -1) {
			free(data);
			return;
		}
		_mus[i].id = i + 1;
		_mus[i].dur = dur + 1;
		_mod->startChannel(_mus[i].id, data, _wavetable[inst]->_ilen[oct] + _wavetable[inst]->_llen[oct], rate, vol,
			_wavetable[inst]->_ilen[oct], _wavetable[inst]->_ilen[oct] + _wavetable[inst]->_llen[oct]);
	}
}

int Player_V3A::getMusicTimer() {
	return _music_timer / 30;
}

int Player_V3A::getSoundStatus(int nr) const {
	if (nr == _curSong)
		return 1;
	if (getSfxChan(nr) != -1)
		return 1;
	return 0;
}

} // End of namespace Scumm
