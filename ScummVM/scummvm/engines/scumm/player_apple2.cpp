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
#include "scumm/player_apple2.h"
#include "scumm/scumm.h"

namespace Scumm {

/************************************
 * Apple-II sound-resource parsers
 ************************************/

/*
 * SoundFunction1: frequency up/down
 */
class AppleII_SoundFunction1_FreqUpDown : public AppleII_SoundFunction {
public:
	virtual void init(Player_AppleII *player, const byte *params) {
		_player = player;
		_delta = params[0];
		_count = params[1];
		_interval = params[2];
		_limit = params[3];
		_decInterval = (params[4] >= 0x40);
	}

	virtual bool update() { // D085
		if (_decInterval) {
			do {
				_update(_interval, _count);
				_interval -= _delta;
			} while (_interval >= _limit);
		} else {
			do {
				_update(_interval, _count);
				_interval += _delta;
			} while (_interval < _limit);
		}
		return true;
	}

private:
	void _update(int interval /*a*/, int count /*y*/) { // D076
		assert(interval > 0); // 0 == 256?
		assert(count > 0); // 0 == 256?

		for (; count >= 0; --count) {
			_player->speakerToggle();
			_player->generateSamples(17 + 5 * interval);
		}
	}

protected:
	int _delta;
	int _count;
	byte _interval; // must be unsigned byte ("interval < delta" possible)
	int _limit;
	bool _decInterval;
};

/*
 * SoundFunction2: symmetric wave (~)
 */
class AppleII_SoundFunction2_SymmetricWave : public AppleII_SoundFunction {
public:
	virtual void init(Player_AppleII *player, const byte *params) {
		_player = player;
		_params = params;
		_pos = 1;
	}

	virtual bool update() { // D0D6
		// while (pos = 1; pos < 256; ++pos)
		if (_pos < 256) {
			byte interval = _params[_pos];
			if (interval == 0xFF)
				return true;
			_update(interval, _params[0] /*, LD12F=interval*/);

			++_pos;
			return false;
		}
		return true;
	}

private:
	void _update(int interval /*a*/, int count) { // D0EF
		if (interval == 0xFE) {
			_player->wait(interval, 10);
		} else {
			assert(count > 0); // 0 == 256?
			assert(interval > 0); // 0 == 256?

			int a = (interval >> 3) + count;
			for (int y = a; y > 0; --y) {
				_player->generateSamples(1292 - 5*interval);
				_player->speakerToggle();

				_player->generateSamples(1287 - 5*interval);
				_player->speakerToggle();
			}
		}
	}

protected:
	const byte *_params;
	int _pos;
};

/*
 * SoundFunction3: asymmetric wave (__-)
 */
class AppleII_SoundFunction3_AsymmetricWave : public AppleII_SoundFunction {
public:
	virtual void init(Player_AppleII *player, const byte *params) {
		_player = player;
		_params = params;
		_pos = 1;
	}

	virtual bool update() { // D132
		// while (pos = 1; pos < 256; ++pos)
		if (_pos < 256) {
			byte interval = _params[_pos];
			if (interval == 0xFF)
				return true;
			_update(interval, _params[0]);

			++_pos;
			return false;
		}
		return true;
	}

private:
	void _update(int interval /*a*/, int count /*LD12D*/) { // D14B
		if (interval == 0xFE) {
			_player->wait(interval, 70);
		} else {
			assert(interval > 0); // 0 == 256?
			assert(count > 0); // 0 == 256?

			for (int y = count; y > 0; --y) {
				_player->generateSamples(1289 - 5*interval);
				_player->speakerToggle();
			}
		}
	}

protected:
	const byte *_params;
	int _pos;
};

/*
 * SoundFunction4: polyphone (2 voices)
 */
class AppleII_SoundFunction4_Polyphone : public AppleII_SoundFunction {
public:
	virtual void init(Player_AppleII *player, const byte *params) {
		_player = player;
		_params = params;
		_updateRemain1 = 80;
		_updateRemain2 = 10;
		_count = 0;
	}

	virtual bool update() { // D170
		// while (_params[0] != 0x01)
		if (_params[0] != 0x01) {
			if (_count == 0) // prepare next loop
				nextLoop(_params[0], _params[1], _params[2]);
			if (loopIteration()) // loop finished -> fetch next parameter set
				_params += 3;
			return false;
		}
		return true;
	}

private:
	/*
	 * prepare for next parameter set loop
	 */
	void nextLoop(byte param0, byte param1, byte param2) { // LD182
		_count = (-param2 << 8) | 0x3;

		_bitmask1 = 0x3;
		_bitmask2 = 0x3;

		_updateInterval2 = param0;
		if (_updateInterval2 == 0)
			_bitmask2 = 0x0;

		_updateInterval1 = param1;
		if (_updateInterval1 == 0) {
			_bitmask1 = 0x0;
			if (_bitmask2 != 0) {
				_bitmask1 = _bitmask2;
				_bitmask2 = 0;
				_updateInterval1 = _updateInterval2;
			}
		}

		_speakerShiftReg = 0;
	}

	/*
	 * perform one loop iteration
	 * Returns true if loop finished
	 */
	bool loopIteration() { // D1A2
		--_updateRemain1;
		--_updateRemain2;

		if (_updateRemain2 == 0) {
			_updateRemain2 = _updateInterval2;
			// use only first voice's data (bitmask1) if both voices are triggered
			if (_updateRemain1 != 0) {
				_speakerShiftReg ^= _bitmask2;
			}
		}

		if (_updateRemain1 == 0) {
			_updateRemain1 = _updateInterval1;
			_speakerShiftReg ^= _bitmask1;
		}

		if (_speakerShiftReg & 0x1)
			_player->speakerToggle();
		_speakerShiftReg >>= 1;
		_player->generateSamples(42); /* actually 42.5 */

		++_count;
		return (_count == 0);
	}

protected:
	const byte *_params;

	byte _updateRemain1;
	byte _updateRemain2;

	uint16 _count;
	byte _bitmask1;
	byte _bitmask2;
	byte _updateInterval1;
	byte _updateInterval2;
	byte _speakerShiftReg;
};

/*
 * SoundFunction5: periodic noise
 */
class AppleII_SoundFunction5_Noise : public AppleII_SoundFunction {
public:
	virtual void init(Player_AppleII *player, const byte *params) {
		_player = player;
		_index = 0;
		_param0 = params[0];
		assert(_param0 > 0);
	}

	virtual bool update() { // D222
		const byte noiseMask[] = {
			0x3F, 0x3F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F
		};

		// while (i = 0; i < 10; ++i)
		if (_index < 10) {
			int count = _param0;
			do {
				_update(noise() & noiseMask[_index], 1);
				--count;
			} while (count > 0);

			++_index;
			return false;
		}

		return true;
	}

private:
	void _update(int interval /*a*/, int count) { // D270
		assert(count > 0); // 0 == 256?
		if (interval == 0)
			interval = 256;

		for (int i = count; i > 0; --i) {
			_player->generateSamples(10 + 5*interval);
			_player->speakerToggle();

			_player->generateSamples(5 + 5*interval);
			_player->speakerToggle();
		}
	}

	byte /*a*/ noise() { // D261
		static int pos = 0; // initial value?
		byte result = _noiseTable[pos];
		pos = (pos + 1) % 256;
		return result;
	}

protected:
	int _index;
	int _param0;

private:
	static const byte _noiseTable[256];
};

// LD000[loc] ^ LD00A[loc]
const byte AppleII_SoundFunction5_Noise::_noiseTable[256] = {
	0x65, 0x1b, 0xda, 0x11, 0x61, 0xe5, 0x77, 0x57, 0x92, 0xc8, 0x51, 0x1c, 0xd4, 0x91, 0x62, 0x63,
	0x00, 0x38, 0x57, 0xd5, 0x18, 0xd8, 0xdc, 0x40, 0x03, 0x86, 0xd3, 0x2f, 0x10, 0x11, 0xd8, 0x3c,
	0xbe, 0x00, 0x19, 0xc5, 0xd2, 0xc3, 0xca, 0x34, 0x00, 0x28, 0xbf, 0xb9, 0x18, 0x20, 0x01, 0xcc,
	0xda, 0x08, 0xbc, 0x75, 0x7c, 0xb0, 0x8d, 0xe0, 0x09, 0x18, 0xbf, 0x5d, 0xe9, 0x8c, 0x75, 0x64,
	0xe5, 0xb5, 0x5d, 0xe0, 0xb7, 0x7d, 0xe9, 0x8c, 0x55, 0x65, 0xc5, 0xb5, 0x5d, 0xd8, 0x09, 0x0d,
	0x64, 0xf0, 0xf0, 0x08, 0x63, 0x03, 0x00, 0x55, 0x35, 0xc0, 0x00, 0x20, 0x74, 0xa5, 0x1e, 0xe3,
	0x00, 0x06, 0x3c, 0x52, 0xd1, 0x70, 0xd0, 0x57, 0x02, 0xf0, 0x00, 0xb6, 0xfc, 0x02, 0x11, 0x9a,
	0x3b, 0xc8, 0x38, 0xdf, 0x1a, 0xb0, 0xd1, 0xb8, 0xd0, 0x18, 0x8a, 0x4a, 0xea, 0x1b, 0x12, 0x5d,
	0x29, 0x58, 0xd8, 0x43, 0xb8, 0x2d, 0xd2, 0x61, 0x10, 0x3c, 0x0c, 0x5d, 0x1b, 0x61, 0x10, 0x3c,
	0x0a, 0x5d, 0x1d, 0x61, 0x10, 0x3c, 0x0b, 0x19, 0x88, 0x21, 0xc0, 0x21, 0x07, 0x00, 0x65, 0x62,
	0x08, 0xe9, 0x36, 0x40, 0x20, 0x41, 0x06, 0x00, 0x20, 0x00, 0x00, 0xed, 0xa3, 0x00, 0x88, 0x06,
	0x98, 0x01, 0x5d, 0x7f, 0x02, 0x1d, 0x78, 0x03, 0x60, 0xcb, 0x3a, 0x01, 0xbd, 0x78, 0x02, 0x5d,
	0x7e, 0x03, 0x1d, 0xf5, 0xa6, 0x40, 0x81, 0xb4, 0xd0, 0x8d, 0xd3, 0xd0, 0x6d, 0xd5, 0x61, 0x48,
	0x61, 0x4d, 0xd1, 0xc8, 0xb1, 0xd8, 0x69, 0xff, 0x61, 0xd9, 0xed, 0xa0, 0xfe, 0x19, 0x91, 0x37,
	0x19, 0x37, 0x00, 0xf1, 0x00, 0x01, 0x1f, 0x00, 0xad, 0xc1, 0x01, 0x01, 0x2e, 0x00, 0x40, 0xc6,
	0x7a, 0x9b, 0x95, 0x43, 0xfc, 0x18, 0xd2, 0x9e, 0x2a, 0x5a, 0x4b, 0x2a, 0xb6, 0x87, 0x30, 0x6c
};

/************************************
 * Apple-II player
 ************************************/

Player_AppleII::Player_AppleII(ScummEngine *scumm, Audio::Mixer *mixer)
	: _mixer(mixer), _vm(scumm), _soundFunc(0) {
	resetState();
	setSampleRate(_mixer->getOutputRate());
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Player_AppleII::~Player_AppleII() {
	_mixer->stopHandle(_soundHandle);
	delete _soundFunc;
}

void Player_AppleII::resetState() {
	_soundNr = 0;
	_type = 0;
	_loop = 0;
	_params = NULL;
	_speakerState = 0;
	delete _soundFunc;
	_soundFunc = 0;
	_sampleConverter.reset();
}

void Player_AppleII::startSound(int nr) {
	Common::StackLock lock(_mutex);

	byte *data = _vm->getResourceAddress(rtSound, nr);
	assert(data);
	byte *ptr1 = data + 4;

	resetState();
	_soundNr = nr;
	_type = ptr1[0];
	_loop = ptr1[1];
	_params = &ptr1[2];

	switch (_type) {
	case 0: // empty (nothing to play)
		resetState();
		return;
	case 1:
		_soundFunc = new AppleII_SoundFunction1_FreqUpDown();
		break;
	case 2:
		_soundFunc = new AppleII_SoundFunction2_SymmetricWave();
		break;
	case 3:
		_soundFunc = new AppleII_SoundFunction3_AsymmetricWave();
		break;
	case 4:
		_soundFunc = new AppleII_SoundFunction4_Polyphone();
		break;
	case 5:
		_soundFunc = new AppleII_SoundFunction5_Noise();
		break;
	}
	_soundFunc->init(this, _params);

	assert(_loop > 0);

	debug(4, "startSound %d: type %d, loop %d",
		  nr, _type, _loop);
}

bool Player_AppleII::updateSound() {
	if (!_soundFunc)
		return false;

	if (_soundFunc->update()) {
		--_loop;
		if (_loop <= 0) {
			delete _soundFunc;
			_soundFunc = 0;
		} else {
			// reset function state on each loop
			_soundFunc->init(this, _params);
		}
	}

	return true;
}

void Player_AppleII::stopAllSounds() {
	Common::StackLock lock(_mutex);
	resetState();
}

void Player_AppleII::stopSound(int nr) {
	Common::StackLock lock(_mutex);
	if (_soundNr == nr) {
		resetState();
	}
}

int Player_AppleII::getSoundStatus(int nr) const {
	Common::StackLock lock(_mutex);
	return (_soundNr == nr);
}

int Player_AppleII::getMusicTimer() {
	/* Apple-II sounds are synchronous -> no music timer */
	return 0;
}

int Player_AppleII::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);

	if (!_soundNr)
		return 0;

	int samplesLeft = numSamples;
	do {
		int nSamplesRead = _sampleConverter.readSamples(buffer, samplesLeft);
		samplesLeft -= nSamplesRead;
		buffer += nSamplesRead;
	} while ((samplesLeft > 0) && updateSound());

	// reset state if sound is played completely
	if (!_soundFunc && (_sampleConverter.availableSize() == 0))
		resetState();

	return numSamples - samplesLeft;
}

/************************************
 * Apple-II sound-resource helpers
 ************************************/

// toggle speaker on/off
void Player_AppleII::speakerToggle() {
	_speakerState ^= 0x1;
}

void Player_AppleII::generateSamples(int cycles) {
	_sampleConverter.addCycles(_speakerState, cycles);
}

void Player_AppleII::wait(int interval, int count /*y*/) {
	assert(count > 0); // 0 == 256?
	assert(interval > 0); // 0 == 256?
	generateSamples(11 + count*(8 + 5 * interval));
}

} // End of namespace Scumm
