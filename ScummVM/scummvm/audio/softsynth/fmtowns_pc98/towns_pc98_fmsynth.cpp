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

#include "audio/softsynth/fmtowns_pc98/towns_pc98_fmsynth.h"
#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"

class TownsPC98_FmSynthOperator {
public:
	TownsPC98_FmSynthOperator(const uint32 timerbase, const uint32 rtt, const uint8 *rateTable,
	                          const uint8 *shiftTable, const uint8 *attackDecayTable, const uint32 *frqTable,
	                          const uint32 *sineTable, const int32 *tlevelOut, const int32 *detuneTable);
	~TownsPC98_FmSynthOperator() {}

	void keyOn();
	void keyOff();
	void frequency(int freq);
	void updatePhaseIncrement();
	void recalculateRates();
	void generateOutput(int32 phasebuf, int32 *feedbuf, int32 &out);

	void feedbackLevel(int32 level);
	void detune(int value);
	void multiple(uint32 value);
	void attackRate(uint32 value);
	bool scaleRate(uint8 value);
	void decayRate(uint32 value);
	void sustainRate(uint32 value);
	void sustainLevel(uint32 value);
	void releaseRate(uint32 value);
	void totalLevel(uint32 value);
	void ampModulation(bool enable);
	void reset();

protected:
	EnvelopeState _state;
	bool _holdKey;
	uint32 _feedbackLevel;
	uint32 _multiple;
	uint32 _totalLevel;
	uint8 _keyScale1;
	uint8 _keyScale2;
	uint32 _specifiedAttackRate;
	uint32 _specifiedDecayRate;
	uint32 _specifiedSustainRate;
	uint32 _specifiedReleaseRate;
	uint32 _tickCount;
	uint32 _sustainLevel;

	bool _ampMod;
	uint32 _frequency;
	uint8 _kcode;
	uint32 _phase;
	uint32 _phaseIncrement;
	const int32 *_detn;

	const uint8 *_rateTbl;
	const uint8 *_rshiftTbl;
	const uint8 *_adTbl;
	const uint32 *_fTbl;
	const uint32 *_sinTbl;
	const int32 *_tLvlTbl;
	const int32 *_detnTbl;

	const uint32 _tickLength;
	uint32 _timer;
	const uint32 _rtt;
	int32 _currentLevel;

	struct EvpState {
		uint8 rate;
		uint8 shift;
	} fs_a, fs_d, fs_s, fs_r;
};

TownsPC98_FmSynthOperator::TownsPC98_FmSynthOperator(const uint32 timerbase, const uint32 rtt,
		const uint8 *rateTable, const uint8 *shiftTable, const uint8 *attackDecayTable,
		const uint32 *frqTable, const uint32 *sineTable, const int32 *tlevelOut, const int32 *detuneTable) :
	_rtt(rtt), _rateTbl(rateTable), _rshiftTbl(shiftTable), _adTbl(attackDecayTable), _fTbl(frqTable),
	_sinTbl(sineTable), _tLvlTbl(tlevelOut), _detnTbl(detuneTable), _tickLength(timerbase * 2),
	_specifiedAttackRate(0), _specifiedDecayRate(0), _specifiedReleaseRate(0), _specifiedSustainRate(0),
	_sustainLevel(0), _phase(0), _state(kEnvReady), _holdKey(false), _timer(0), _keyScale1(0),
	_keyScale2(0), _currentLevel(1023), _ampMod(false), _tickCount(0) {

	fs_a.rate = fs_a.shift = fs_d.rate = fs_d.shift = fs_s.rate = fs_s.shift = fs_r.rate = fs_r.shift = 0;

	reset();
}

void TownsPC98_FmSynthOperator::keyOn() {
	if (_holdKey)
		return;

	_holdKey = true;
	_state = kEnvAttacking;
	_phase = 0;
}

void TownsPC98_FmSynthOperator::keyOff() {
	if (!_holdKey)
		return;

	_holdKey = false;
	if (_state != kEnvReady)
		_state = kEnvReleasing;
}

void TownsPC98_FmSynthOperator::frequency(int freq) {
	uint8 block = (freq >> 11);
	uint16 pos = (freq & 0x7ff);
	uint8 c = pos >> 7;

	_kcode = (block << 2) | ((c < 7) ? 0 : ((c > 8) ? 3 : c - 6));
	_frequency = _fTbl[pos << 1] >> (7 - block);
}

void TownsPC98_FmSynthOperator::updatePhaseIncrement() {
	_phaseIncrement = ((_frequency + _detn[_kcode]) * _multiple) >> 1;
	uint8 keyscale = _kcode >> _keyScale1;
	if (_keyScale2 != keyscale) {
		_keyScale2 = keyscale;
		recalculateRates();
	}
}

void TownsPC98_FmSynthOperator::recalculateRates() {
	int k = _keyScale2;
	int r = _specifiedAttackRate ? (_specifiedAttackRate << 1) + 0x20 : 0;
	fs_a.rate = ((r + k) < 94) ? _rateTbl[r + k] : 136;
	fs_a.shift = ((r + k) < 94) ? _rshiftTbl[r + k] : 0;

	r = _specifiedDecayRate ? (_specifiedDecayRate << 1) + 0x20 : 0;
	fs_d.rate = _rateTbl[r + k];
	fs_d.shift = _rshiftTbl[r + k];

	r = _specifiedSustainRate ? (_specifiedSustainRate << 1) + 0x20 : 0;
	fs_s.rate = _rateTbl[r + k];
	fs_s.shift = _rshiftTbl[r + k];

	r = (_specifiedReleaseRate << 2) + 0x22;
	fs_r.rate = _rateTbl[r + k];
	fs_r.shift = _rshiftTbl[r + k];
}

void TownsPC98_FmSynthOperator::generateOutput(int32 phasebuf, int32 *feed, int32 &out) {
	if (_state == kEnvReady)
		return;

	_timer += _tickLength;
	while (_timer > _rtt) {
		_timer -= _rtt;
		++_tickCount;

		int32 levelIncrement = 0;
		uint32 targetTime = 0;
		int32 targetLevel = 0;
		EnvelopeState nextState = kEnvReady;

		for (bool loop = true; loop;) {
			switch (_state) {
			case kEnvReady:
				return;
			case kEnvAttacking:
				targetLevel = 0;
				nextState = _sustainLevel ? kEnvDecaying : kEnvSustaining;
				if ((_specifiedAttackRate << 1) + _keyScale2 < 62) {
					targetTime = (1 << fs_a.shift) - 1;
					levelIncrement = (~_currentLevel * _adTbl[fs_a.rate + ((_tickCount >> fs_a.shift) & 7)]) >> 4;
				} else {
					_currentLevel = targetLevel;
					_state = nextState;
					continue;
				}
				break;
			case kEnvDecaying:
				targetTime = (1 << fs_d.shift) - 1;
				nextState = kEnvSustaining;
				targetLevel = _sustainLevel;
				levelIncrement = _adTbl[fs_d.rate + ((_tickCount >> fs_d.shift) & 7)];
				break;
			case kEnvSustaining:
				targetTime = (1 << fs_s.shift) - 1;
				nextState = kEnvSustaining;
				targetLevel = 1023;
				levelIncrement = _adTbl[fs_s.rate + ((_tickCount >> fs_s.shift) & 7)];
				break;
			case kEnvReleasing:
				targetTime = (1 << fs_r.shift) - 1;
				nextState = kEnvReady;
				targetLevel = 1023;
				levelIncrement = _adTbl[fs_r.rate + ((_tickCount >> fs_r.shift) & 7)];
				break;
			}
			loop = false;
		}

		if (!(_tickCount & targetTime)) {
			_currentLevel += levelIncrement;
			if ((_state == kEnvAttacking && _currentLevel <= targetLevel) || (_state != kEnvAttacking && _currentLevel >= targetLevel)) {
				if (_state != kEnvDecaying)
					_currentLevel = targetLevel;
				_state = nextState;
			}
		}
	}

	uint32 lvlout = _totalLevel + (uint32) _currentLevel;


	int32 outp = 0;
	int32 *i = &outp, *o = &outp;
	int phaseShift = 0;

	if (feed) {
		o = &feed[0];
		i = &feed[1];
		phaseShift = _feedbackLevel ? ((*o + *i) << _feedbackLevel) : 0;
		*o = *i;
	} else {
		phaseShift = phasebuf << 15;
	}

	if (lvlout < 832) {
		uint32 index = (lvlout << 3) + _sinTbl[(((int32)((_phase & 0xffff0000)
		                                        + phaseShift)) >> 16) & 0x3ff];
		*i = ((index < 6656) ? _tLvlTbl[index] : 0);
	} else {
		*i = 0;
	}

	_phase += _phaseIncrement;
	out += *o;
}

void TownsPC98_FmSynthOperator::feedbackLevel(int32 level) {
	_feedbackLevel = level ? level + 6 : 0;
}

void TownsPC98_FmSynthOperator::detune(int value) {
	_detn = &_detnTbl[value << 5];
}

void TownsPC98_FmSynthOperator::multiple(uint32 value) {
	_multiple = value ? (value << 1) : 1;
}

void TownsPC98_FmSynthOperator::attackRate(uint32 value) {
	_specifiedAttackRate = value;
}

bool TownsPC98_FmSynthOperator::scaleRate(uint8 value) {
	value = 3 - value;
	if (_keyScale1 != value) {
		_keyScale1 = value;
		return true;
	}

	int k = _keyScale2;
	int r = _specifiedAttackRate ? (_specifiedAttackRate << 1) + 0x20 : 0;
	fs_a.rate = ((r + k) < 94) ? _rateTbl[r + k] : 136;
	fs_a.shift = ((r + k) < 94) ? _rshiftTbl[r + k] : 0;
	return false;
}

void TownsPC98_FmSynthOperator::decayRate(uint32 value) {
	_specifiedDecayRate = value;
	recalculateRates();
}

void TownsPC98_FmSynthOperator::sustainRate(uint32 value) {
		_specifiedSustainRate = value;
		recalculateRates();
	}

void TownsPC98_FmSynthOperator::sustainLevel(uint32 value) {
	_sustainLevel = (value == 0x0f) ? 0x3e0 : value << 5;
}

void TownsPC98_FmSynthOperator::releaseRate(uint32 value) {
	_specifiedReleaseRate = value;
	recalculateRates();
}

void TownsPC98_FmSynthOperator::totalLevel(uint32 value) {
	_totalLevel = value << 3;
}

void TownsPC98_FmSynthOperator::ampModulation(bool enable) {
	_ampMod = enable;
}

void TownsPC98_FmSynthOperator::reset() {
	keyOff();
	_timer = 0;
	_keyScale2 = 0;
	_currentLevel = 1023;

	frequency(0);
	detune(0);
	scaleRate(0);
	multiple(0);
	updatePhaseIncrement();
	attackRate(0);
	decayRate(0);
	releaseRate(0);
	sustainRate(0);
	feedbackLevel(0);
	totalLevel(127);
	ampModulation(false);
}

class TownsPC98_FmSynthSquareSineSource {
public:
	TownsPC98_FmSynthSquareSineSource(const uint32 timerbase, const uint32 rtt);
	~TownsPC98_FmSynthSquareSineSource();

	void init(const int *rsTable, const int *rseTable);
	void reset();
	void writeReg(uint8 address, uint8 value, bool force = false);

	void nextTick(int32 *buffer, uint32 bufferSize);

	void setVolumeIntern(int volA, int volB) {
		_volumeA = volA;
		_volumeB = volB;
	}
	void setVolumeChannelMasks(int channelMaskA, int channelMaskB) {
		_volMaskA = channelMaskA;
		_volMaskB = channelMaskB;
	}

	uint8 chanEnable() const {
		return _chanEnable;
	}
private:
	void updateRegs();

	uint8 _updateRequestBuf[64];
	int _updateRequest;
	int _rand;

	int8 _evpTimer;
	uint32 _pReslt;
	uint8 _attack;

	bool _evpUpdate, _cont;

	int _evpUpdateCnt;
	uint8 _outN;
	int _nTick;

	int32 *_tlTable;
	int32 *_tleTable;

	const uint32 _tickLength;
	uint32 _timer;
	const uint32 _rtt;

	struct Channel {
		int tick;
		uint8 smp;
		uint8 out;

		uint8 frqL;
		uint8 frqH;
		uint8 vol;
	} _channels[3];

	uint8 _noiseGenerator;
	uint8 _chanEnable;

	uint8 **_reg;

	uint16 _volumeA;
	uint16 _volumeB;
	int _volMaskA;
	int _volMaskB;

	bool _ready;
};

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
class TownsPC98_FmSynthPercussionSource {
public:
	TownsPC98_FmSynthPercussionSource(const uint32 timerbase, const uint32 rtt);
	~TownsPC98_FmSynthPercussionSource() {
		delete[] _reg;
	}

	void init(const uint8 *instrData = 0);
	void reset();
	void writeReg(uint8 address, uint8 value);

	void nextTick(int32 *buffer, uint32 bufferSize);

	void setVolumeIntern(int volA, int volB) {
		_volumeA = volA;
		_volumeB = volB;
	}
	void setVolumeChannelMasks(int channelMaskA, int channelMaskB) {
		_volMaskA = channelMaskA;
		_volMaskB = channelMaskB;
	}

private:
	struct RhtChannel {
		const uint8 *data;

		const uint8 *start;
		const uint8 *end;
		const uint8 *pos;
		uint32 size;
		bool active;
		uint8 level;

		int8 decState;
		uint8 decStep;

		int16 samples[2];
		int out;

		uint8 startPosH;
		uint8 startPosL;
		uint8 endPosH;
		uint8 endPosL;
	};

	void recalcOuput(RhtChannel *ins);
	void advanceInput(RhtChannel *ins);

	RhtChannel _rhChan[6];

	uint8 _totalLevel;

	const uint32 _tickLength;
	uint32 _timer;
	const uint32 _rtt;

	uint8 **_reg;

	uint16 _volumeA;
	uint16 _volumeB;
	int _volMaskA;
	int _volMaskB;

	bool _ready;
};
#endif // DISABLE_PC98_RHYTHM_CHANNEL

TownsPC98_FmSynthSquareSineSource::TownsPC98_FmSynthSquareSineSource(const uint32 timerbase, const uint32 rtt) : _tlTable(0),
	_rtt(rtt), _tleTable(0), _updateRequest(-1), _tickLength(timerbase * 27), _ready(0), _reg(0), _rand(1), _outN(1),
	_nTick(0), _evpUpdateCnt(0), _evpTimer(0x1f), _pReslt(0x1f), _attack(0), _cont(false), _evpUpdate(true),
	_timer(0), _noiseGenerator(0), _chanEnable(0),
	_volMaskA(0), _volMaskB(0), _volumeA(Audio::Mixer::kMaxMixerVolume), _volumeB(Audio::Mixer::kMaxMixerVolume) {

	memset(_channels, 0, sizeof(_channels));
	memset(_updateRequestBuf, 0, sizeof(_updateRequestBuf));
	_reg = new uint8 *[11];

	_reg[0] = &_channels[0].frqL;
	_reg[1] = &_channels[0].frqH;
	_reg[2] = &_channels[1].frqL;
	_reg[3] = &_channels[1].frqH;
	_reg[4] = &_channels[2].frqL;
	_reg[5] = &_channels[2].frqH;
	_reg[6] = &_noiseGenerator;
	_reg[7] = &_chanEnable;
	_reg[8] = &_channels[0].vol;
	_reg[9] = &_channels[1].vol;
	_reg[10] = &_channels[2].vol;

	reset();
}

TownsPC98_FmSynthSquareSineSource::~TownsPC98_FmSynthSquareSineSource() {
	delete[] _tlTable;
	delete[] _tleTable;
	delete[] _reg;
}

void TownsPC98_FmSynthSquareSineSource::init(const int *rsTable, const int *rseTable) {
	if (_ready) {
		reset();
		return;
	}

	delete[] _tlTable;
	delete[] _tleTable;
	_tlTable = new int32[16];
	_tleTable = new int32[32];
	float a, b, d;
	d = 801.0f;

	for (int i = 0; i < 16; i++) {
		b = 1.0f / rsTable[i];
		a = 1.0f / d + b + 1.0f / 1000.0f;
		float v = (b / a) * 32767.0f;
		_tlTable[i] = (int32) v;

		b = 1.0f / rseTable[i];
		a = 1.0f / d + b + 1.0f / 1000.0f;
		v = (b / a) * 32767.0f;
		_tleTable[i] = (int32) v;
	}

	for (int i = 16; i < 32; i++) {
		b = 1.0f / rseTable[i];
		a = 1.0f / d + b + 1.0f / 1000.0f;
		float v = (b / a) * 32767.0f;
		_tleTable[i] = (int32) v;
	}

	_ready = true;
}

void TownsPC98_FmSynthSquareSineSource::reset() {
	_rand = 1;
	_outN = 1;
	_updateRequest = -1;
	_nTick = _evpUpdateCnt = 0;
	_evpTimer = 0x1f;
	_pReslt = 0x1f;
	_attack = 0;
	_cont = false;
	_evpUpdate = true;
	_timer = 0;

	for (int i = 0; i < 3; i++) {
		_channels[i].tick = 0;
		_channels[i].smp = _channels[i].out = 0;
	}

	for (int i = 0; i < 14; i++)
		writeReg(i, 0, true);

	writeReg(7, 0xbf, true);
}

void TownsPC98_FmSynthSquareSineSource::writeReg(uint8 address, uint8 value, bool force) {
	if (!_ready)
		return;

	if (address > 10 || *_reg[address] == value) {
		if ((address == 11 || address == 12 || address == 13) && value)
			warning("TownsPC98_FmSynthSquareSineSource: unsupported reg address: %d", address);
		return;
	}

	if (!force) {
		if (_updateRequest >= 63) {
			warning("TownsPC98_FmSynthSquareSineSource: event buffer overflow");
			_updateRequest = -1;
		}
		_updateRequestBuf[++_updateRequest] = value;
		_updateRequestBuf[++_updateRequest] = address;
		return;
	}

	*_reg[address] = value;
}

void TownsPC98_FmSynthSquareSineSource::nextTick(int32 *buffer, uint32 bufferSize) {
	if (!_ready)
		return;

	for (uint32 i = 0; i < bufferSize; i++) {
		_timer += _tickLength;
		while (_timer > _rtt) {
			_timer -= _rtt;

			if (++_nTick >= (_noiseGenerator & 0x1f)) {
				if ((_rand + 1) & 2)
					_outN ^= 1;

				_rand = (((_rand & 1) ^ ((_rand >> 3) & 1)) << 16) | (_rand >> 1);
				_nTick = 0;
			}

			for (int ii = 0; ii < 3; ii++) {
				if (++_channels[ii].tick >= (((_channels[ii].frqH & 0x0f) << 8) | _channels[ii].frqL)) {
					_channels[ii].tick = 0;
					_channels[ii].smp ^= 1;
				}
				_channels[ii].out = (_channels[ii].smp | ((_chanEnable >> ii) & 1)) & (_outN | ((_chanEnable >> (ii + 3)) & 1));
			}

			if (_evpUpdate) {
				if (++_evpUpdateCnt >= 0) {
					_evpUpdateCnt = 0;

					if (--_evpTimer < 0) {
						if (_cont) {
							_evpTimer &= 0x1f;
						} else {
							_evpUpdate = false;
							_evpTimer = 0;
						}
					}
				}
			}
			_pReslt = _evpTimer ^ _attack;
			updateRegs();
		}

		int32 finOut = 0;
		for (int ii = 0; ii < 3; ii++) {
			int32 finOutTemp = ((_channels[ii].vol >> 4) & 1) ? _tleTable[_channels[ii].out ? _pReslt : 0] : _tlTable[_channels[ii].out ? (_channels[ii].vol & 0x0f) : 0];

			if ((1 << ii) & _volMaskA)
				finOutTemp = (finOutTemp * _volumeA) / Audio::Mixer::kMaxMixerVolume;

			if ((1 << ii) & _volMaskB)
				finOutTemp = (finOutTemp * _volumeB) / Audio::Mixer::kMaxMixerVolume;

			finOut += finOutTemp;
		}

		finOut /= 3;

		buffer[i << 1] += finOut;
		buffer[(i << 1) + 1] += finOut;
	}
}

void TownsPC98_FmSynthSquareSineSource::updateRegs() {
	for (int i = 0; i < _updateRequest;) {
		uint8 b = _updateRequestBuf[i++];
		uint8 a = _updateRequestBuf[i++];
		writeReg(a, b, true);
	}
	_updateRequest = -1;
}

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
TownsPC98_FmSynthPercussionSource::TownsPC98_FmSynthPercussionSource(const uint32 timerbase, const uint32 rtt) :
	_rtt(rtt), _tickLength(timerbase * 2), _timer(0), _totalLevel(0), _volMaskA(0), _volMaskB(0),
	_volumeA(Audio::Mixer::kMaxMixerVolume), _volumeB(Audio::Mixer::kMaxMixerVolume), _ready(false) {

	memset(_rhChan, 0, sizeof(RhtChannel) * 6);
	_reg = new uint8 *[40];

	_reg[0] = _reg[1] = _reg[2] = _reg[3] = _reg[4] = _reg[5] = _reg[6] = _reg[7] = _reg[8] = _reg[9] = _reg[10] = _reg[11] = _reg[12] = _reg[13] = _reg[14] = _reg[15] = 0;
	_reg[16] = &_rhChan[0].startPosL;
	_reg[17] = &_rhChan[1].startPosL;
	_reg[18] = &_rhChan[2].startPosL;
	_reg[19] = &_rhChan[3].startPosL;
	_reg[20] = &_rhChan[4].startPosL;
	_reg[21] = &_rhChan[5].startPosL;
	_reg[22] = &_rhChan[0].startPosH;
	_reg[23] = &_rhChan[1].startPosH;
	_reg[24] = &_rhChan[2].startPosH;
	_reg[25] = &_rhChan[3].startPosH;
	_reg[26] = &_rhChan[4].startPosH;
	_reg[27] = &_rhChan[5].startPosH;
	_reg[28] = &_rhChan[0].endPosL;
	_reg[29] = &_rhChan[1].endPosL;
	_reg[30] = &_rhChan[2].endPosL;
	_reg[31] = &_rhChan[3].endPosL;
	_reg[32] = &_rhChan[4].endPosL;
	_reg[33] = &_rhChan[5].endPosL;
	_reg[34] = &_rhChan[0].endPosH;
	_reg[35] = &_rhChan[1].endPosH;
	_reg[36] = &_rhChan[2].endPosH;
	_reg[37] = &_rhChan[3].endPosH;
	_reg[38] = &_rhChan[4].endPosH;
	_reg[39] = &_rhChan[5].endPosH;
}

void TownsPC98_FmSynthPercussionSource::init(const uint8 *instrData) {
	if (_ready) {
		reset();
		return;
	}

	const uint8 *start = instrData;
	const uint8 *pos = start;

	if (instrData) {
		for (int i = 0; i < 6; i++) {
			_rhChan[i].data = start + READ_BE_UINT16(pos);
			pos += 2;
			_rhChan[i].size = READ_BE_UINT16(pos);
			pos += 2;
		}
		reset();
		_ready = true;
	} else {
		memset(_rhChan, 0, sizeof(RhtChannel) * 6);
		_ready = false;
	}
}

void TownsPC98_FmSynthPercussionSource::reset() {
	_timer = 0;
	_totalLevel = 63;

	for (int i = 0; i < 6; i++) {
		RhtChannel *s = &_rhChan[i];
		s->pos = s->start = s->data;
		s->end = s->data + s->size;
		s->active = false;
		s->level = 0;
		s->out = 0;
		s->decStep = 1;
		s->decState = 0;
		s->samples[0] = s->samples[1] = 0;
		s->startPosH = s->startPosL = s->endPosH = s->endPosL = 0;
	}
}

void TownsPC98_FmSynthPercussionSource::writeReg(uint8 address, uint8 value) {
	if (!_ready)
		return;

	uint8 h = address >> 4;
	uint8 l = address & 15;

	if (address > 15)
		*_reg[address] = value;

	if (address == 0) {
		if (value & 0x80) {
			//key off
			for (int i = 0; i < 6; i++) {
				if ((value >> i) & 1)
					_rhChan[i].active = false;
			}
		} else {
			//key on
			for (int i = 0; i < 6; i++) {
				if ((value >> i) & 1) {
					RhtChannel *s = &_rhChan[i];
					s->pos = s->start;
					s->active = true;
					s->out = 0;
					s->samples[0] = s->samples[1] = 0;
					s->decStep = 1;
					s->decState = 0;
				}
			}
		}
	} else if (address == 1) {
		// total level
		_totalLevel = (value & 63) ^ 63;
		for (int i = 0; i < 6; i++)
			recalcOuput(&_rhChan[i]);
	} else if (!h && l & 8) {
		// instrument level
		l &= 7;
		_rhChan[l].level = (value & 0x1f) ^ 0x1f;
		recalcOuput(&_rhChan[l]);
	} else if (h & 3) {
		l &= 7;
		if (h == 1) {
			// set start offset
			_rhChan[l].start  = _rhChan[l].data + ((_rhChan[l].startPosH << 8 | _rhChan[l].startPosL) << 8);
		} else if (h == 2) {
			// set end offset
			_rhChan[l].end = _rhChan[l].data + ((_rhChan[l].endPosH << 8 | _rhChan[l].endPosL) << 8) + 255;
		}
	}
}

void TownsPC98_FmSynthPercussionSource::nextTick(int32 *buffer, uint32 bufferSize) {
	if (!_ready)
		return;

	for (uint32 i = 0; i < bufferSize; i++) {
		_timer += _tickLength;
		while (_timer > _rtt) {
			_timer -= _rtt;

			for (int ii = 0; ii < 6; ii++) {
				RhtChannel *s = &_rhChan[ii];
				if (s->active) {
					recalcOuput(s);
					if (s->decStep) {
						advanceInput(s);
						if (s->pos == s->end)
							s->active = false;
					}
					s->decStep ^= 1;
				}
			}
		}

		int32 finOut = 0;

		for (int ii = 0; ii < 6; ii++) {
			if (_rhChan[ii].active)
				finOut += _rhChan[ii].out;
		}

		finOut <<= 1;

		if (1 & _volMaskA)
			finOut = (finOut * _volumeA) / Audio::Mixer::kMaxMixerVolume;

		if (1 & _volMaskB)
			finOut = (finOut * _volumeB) / Audio::Mixer::kMaxMixerVolume;

		buffer[i << 1] += finOut;
		buffer[(i << 1) + 1] += finOut;
	}
}

void TownsPC98_FmSynthPercussionSource::recalcOuput(RhtChannel *ins) {
	uint32 s = _totalLevel + ins->level;
	uint32 x = s > 62 ? 0 : (1 + (s >> 3));
	int32 y = s > 62 ? 0 : (15 - (s & 7));
	ins->out = ((ins->samples[ins->decStep] * y) >> x) & ~3;
}

void TownsPC98_FmSynthPercussionSource::advanceInput(RhtChannel *ins) {
	static const int8 adjustIndex[] = { -1, -1, -1, -1, 2, 5, 7, 9 };

	static const int16 stepTable[] = {
		16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55,
		60, 66, 73, 80, 88, 97, 107, 118, 130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337,
		371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552
	};

	uint8 cur = (int8)*ins->pos++;

	for (int i = 0; i < 2; i++) {
		int b = (2 * (cur & 7) + 1) * stepTable[ins->decState] / 8;
		ins->samples[i] = CLIP<int16>(ins->samples[i ^ 1] + (cur & 8 ? b : -b), -2048, 2047);
		ins->decState = CLIP<int8>(ins->decState + adjustIndex[cur & 7], 0, 48);
		cur >>= 4;
	}
}
#endif // DISABLE_PC98_RHYTHM_CHANNEL

TownsPC98_FmSynth::TownsPC98_FmSynth(Audio::Mixer *mixer, EmuType type, bool externalMutexHandling) :
	_mixer(mixer),
	_chanInternal(0), _ssg(0),
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	_prc(0),
#endif
	_numChan(type == kType26 ? 3 : 6), _numSSG(type == kTypeTowns ? 0 : 3),
	_hasPercussion(type == kType86 ? true : false),
	_oprRates(0), _oprRateshift(0), _oprAttackDecay(0), _oprFrq(0), _oprSinTbl(0), _oprLevelOut(0), _oprDetune(0),
	 _rtt(type == kTypeTowns ? 0x514767 : 0x5B8D80), _baserate(55125.0f / (float)mixer->getOutputRate()),
	_volMaskA(0), _volMaskB(0), _volumeA(255), _volumeB(255),
	_regProtectionFlag(false), _externalMutex(externalMutexHandling), _ready(false) {

	memset(&_timers[0], 0, sizeof(ChipTimer));
	memset(&_timers[1], 0, sizeof(ChipTimer));

	_timers[0].cb = _timers[1].cb = &TownsPC98_FmSynth::idleTimerCallback;
	_timerbase = (uint32)(_baserate * 1000000.0f);
}

TownsPC98_FmSynth::~TownsPC98_FmSynth() {
	if (_ready)
		deinit();

	Common::StackLock lock(_mutex);

	delete _ssg;
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	delete _prc;
#endif
	delete[] _chanInternal;

	delete[] _oprRates;
	delete[] _oprRateshift;
	delete[] _oprFrq;
	delete[] _oprAttackDecay;
	delete[] _oprSinTbl;
	delete[] _oprLevelOut;
	delete[] _oprDetune;
}

bool TownsPC98_FmSynth::init() {
	if (_ready) {
		reset();
		return true;
	}

	generateTables();

	_chanInternal = new ChanInternal[_numChan];
	for (int i = 0; i < _numChan; i++) {
		memset(&_chanInternal[i], 0, sizeof(ChanInternal));
		for (int j = 0; j < 4; ++j)
			_chanInternal[i].opr[j] = new TownsPC98_FmSynthOperator(_timerbase, _rtt, _oprRates, _oprRateshift, _oprAttackDecay, _oprFrq, _oprSinTbl, _oprLevelOut, _oprDetune);
	}

	if (_numSSG) {
		_ssg = new TownsPC98_FmSynthSquareSineSource(_timerbase, _rtt);
		_ssg->init(&_ssgTables[0], &_ssgTables[16]);
	}

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	if (_hasPercussion) {
		_prc = new TownsPC98_FmSynthPercussionSource(_timerbase, _rtt);
		_prc->init(_percussionData);
	}
#endif

	_timers[0].cb = &TownsPC98_FmSynth::timerCallbackA;
	_timers[1].cb = &TownsPC98_FmSynth::timerCallbackB;

	_mixer->playStream(Audio::Mixer::kPlainSoundType,
	                   &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_ready = true;

	return true;
}

void TownsPC98_FmSynth::reset() {
	Common::StackLock lock(_mutex);
	for (int i = 0; i < _numChan; i++) {
		for (int ii = 0; ii < 4; ii++)
			_chanInternal[i].opr[ii]->reset();
		memset(_chanInternal[i].feedbuf, 0, 3);
		_chanInternal[i].algorithm = 0;
		_chanInternal[i].frqTemp = 0;
		_chanInternal[i].enableLeft = _chanInternal[i].enableRight = true;
		_chanInternal[i].updateEnvelopeParameters = false;
	}

	writeReg(0, 0x27, 0x33);

	if (_ssg)
		_ssg->reset();

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	if (_prc)
		_prc->reset();
#endif
}

void TownsPC98_FmSynth::writeReg(uint8 part, uint8 regAddress, uint8 value) {
	if (_regProtectionFlag || !_ready)
		return;

	Common::StackLock lock(_mutex);

	static const uint8 oprOrdr[] = { 0, 2, 1, 3 };

	uint8 h = regAddress & 0xf0;
	uint8 l = (regAddress & 0x0f);

	ChanInternal *c = 0;
	TownsPC98_FmSynthOperator **co = 0;
	TownsPC98_FmSynthOperator *o = 0;

	if (regAddress > 0x2F) {
		c = &_chanInternal[(l & 3) + 3 * part];
		co = c->opr;
		o = c->opr[oprOrdr[(l - (l & 3)) >> 2]];
	} else if (regAddress == 0x28) {
		c = &_chanInternal[(value & 3) + ((value & 4) ? 3 : 0)];
		co = c->opr;
	}

	switch (h) {
	case 0x00:
		// ssg
		if (_ssg)
			_ssg->writeReg(l, value);
		break;
	case 0x10:
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
		// pcm rhythm channel
		if (_prc)
			_prc->writeReg(l, value);
#endif
		break;
	case 0x20:
		if (l == 8) {
			// Key on/off
			for (int i = 0; i < 4; i++) {
				if ((value >> (4 + i)) & 1)
					co[oprOrdr[i]]->keyOn();
				else
					co[oprOrdr[i]]->keyOff();
			}
		} else if (l == 4) {
			// Timer A
			_timers[0].value = (_timers[0].value & 3) | (value << 2);
		} else if (l == 5) {
			// Timer A
			_timers[0].value = (_timers[0].value & 0x3fc) | (value & 3);
		} else if (l == 6) {
			// Timer B
			_timers[1].value = value & 0xff;
		} else if (l == 7) {
			if (value & 1) {
				float spc = (float)(0x400 - _timers[0].value) / _baserate;
				if (spc < 1) {
					warning("TownsPC98_FmSynth: Invalid Timer A setting: %d", _timers[0].value);
					spc = 1;
				}

				_timers[0].smpPerCb = (int32) spc;
				_timers[0].smpPerCbRem = (uint32)((spc - (float)_timers[0].smpPerCb) * 1000000.0f);
				_timers[0].smpTillCb = _timers[0].smpPerCb;
				_timers[0].smpTillCbRem = _timers[0].smpPerCbRem;
				_timers[0].enabled = true;
			} else {
				_timers[0].enabled = false;
			}

			if (value & 2) {
				float spc = (float)(0x100 - _timers[1].value) * 16.0f / _baserate;
				if (spc < 1) {
					warning("TownsPC98_FmSynth: Invalid Timer B setting: %d", _timers[1].value);
					spc = 1;
				}

				_timers[1].smpPerCb = (int32) spc;
				_timers[1].smpPerCbRem = (uint32)((spc - (float)_timers[1].smpPerCb) * 1000000.0f);
				_timers[1].smpTillCb = _timers[1].smpPerCb;
				_timers[1].smpTillCbRem = _timers[1].smpPerCbRem;
				_timers[1].enabled = true;
			} else {
				_timers[1].enabled = false;
			}

			if (value & 0x10) {
				_timers[0].smpTillCb = _timers[0].smpPerCb;
				_timers[0].smpTillCbRem = _timers[0].smpTillCbRem;
			}

			if (value & 0x20) {
				_timers[1].smpTillCb = _timers[1].smpPerCb;
				_timers[1].smpTillCbRem = _timers[1].smpTillCbRem;
			}
		} else if (l == 2) {
			// LFO
			if (value & 8)
				warning("TownsPC98_FmSynth: TRYING TO USE LFO (NOT SUPPORTED)");
		} else if (l == 10 || l == 11) {
			// DAC
			if (l == 11 && (value & 0x80))
				warning("TownsPC98_FmSynth: TRYING TO USE DAC (NOT SUPPORTED)");
		}
		break;

	case 0x30:
		// detune, multiple
		o->detune((value >> 4) & 7);
		o->multiple(value & 0x0f);
		c->updateEnvelopeParameters = true;
		break;

	case 0x40:
		// total level
		o->totalLevel(value & 0x7f);
		break;

	case 0x50:
		// rate scaling, attack rate
		o->attackRate(value & 0x1f);
		if (o->scaleRate(value >> 6))
			c->updateEnvelopeParameters = true;
		break;

	case 0x60:
		// first decay rate, amplitude modulation
		o->decayRate(value & 0x1f);
		o->ampModulation(value & 0x80 ? true : false);
		break;

	case 0x70:
		// secondary decay rate
		o->sustainRate(value & 0x1f);
		break;

	case 0x80:
		// secondary amplitude, release rate;
		o->sustainLevel(value >> 4);
		o->releaseRate(value & 0x0f);
		break;

	case 0x90:
		warning("TownsPC98_FmSynth: TRYING TO USE SSG ENVELOPE SHAPES (NOT SUPPORTED)");
		break;

	case 0xa0:
		// frequency
		l &= ~3;
		if (l == 0) {
			c->frqTemp = (c->frqTemp & 0xff00) | value;
			c->updateEnvelopeParameters = true;
			c->fmIndex = (c->frqTemp >> 4 & 0x7f);
			for (int i = 0; i < 4; i++)
				co[i]->frequency(c->frqTemp);
		} else if (l == 4) {
			c->frqTemp = (c->frqTemp & 0xff) | (value << 8);
		} else if (l == 8) {
			// Ch 3/6 special mode frq
			warning("TownsPC98_FmSynth: TRYING TO USE CH 3/6 SPECIAL MODE FREQ (NOT SUPPORTED)");
		} else if (l == 12) {
			// Ch 3/6 special mode frq
			warning("TownsPC98_FmSynth: TRYING TO USE CH 3/6 SPECIAL MODE FREQ (NOT SUPPORTED)");
		}
		break;

	case 0xb0:
		l &= ~3;
		if (l == 0) {
			// feedback, _algorithm
			co[0]->feedbackLevel((value >> 3) & 7);
			c->algorithm = value & 7;
		} else if (l == 4) {
			// stereo, LFO sensitivity
			c->enableLeft = value & 0x80 ? true : false;
			c->enableRight = value & 0x40 ? true : false;
			c->ampModSensitivity((value & 0x30) >> 4);
			c->frqModSensitivity(value & 3);
		}
		break;

	default:
		warning("TownsPC98_FmSynth: UNKNOWN ADDRESS %d", regAddress);
	}
}

int TownsPC98_FmSynth::readBuffer(int16 *buffer, const int numSamples) {
	memset(buffer, 0, sizeof(int16) * numSamples);
	int32 *tmp = new int32[numSamples];
	int32 *tmpStart = tmp;
	memset(tmp, 0, sizeof(int32) * numSamples);
	int32 samplesLeft = numSamples >> 1;

	bool locked = false;
	if (_ready) {
		_mutex.lock();
		locked = true;
	}

	while (_ready && samplesLeft) {
		int32 render = samplesLeft;

		for (int i = 0; i < 2; i++) {
			if (_timers[i].enabled && _timers[i].cb) {
				if (!_timers[i].smpTillCb) {

					if (locked && _externalMutex) {
						_mutex.unlock();
						locked = false;
					}

					(this->*_timers[i].cb)();

					if (!locked && _externalMutex) {
						_mutex.lock();
						locked = true;
					}

					_timers[i].smpTillCb = _timers[i].smpPerCb;

					_timers[i].smpTillCbRem += _timers[i].smpPerCbRem;
					if (_timers[i].smpTillCbRem >= _timerbase) {
						_timers[i].smpTillCb++;
						_timers[i].smpTillCbRem -= _timerbase;
					}
				}
				render = MIN(render, _timers[i].smpTillCb);
			}
		}

		samplesLeft -= render;

		for (int i = 0; i < 2; i++) {
			if (_timers[i].enabled && _timers[i].cb) {
				_timers[i].smpTillCb -= render;
			}
		}

		nextTick(tmp, render);

		if (_ssg)
			_ssg->nextTick(tmp, render);
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
		if (_prc)
			_prc->nextTick(tmp, render);
#endif

		nextTickEx(tmp, render);

		for (int i = 0; i < render; ++i) {
			int32 l = CLIP<int32>(tmp[i << 1], -32767, 32767);
			buffer[i << 1] = (int16) l;
			int32 r = CLIP<int32>(tmp[(i << 1) + 1], -32767, 32767);
			buffer[(i << 1) + 1] = (int16) r;
		}

		buffer += (render << 1);
		tmp += (render << 1);
	}

	if (locked)
		_mutex.unlock();

	delete[] tmpStart;

	return numSamples;
}

bool TownsPC98_FmSynth::isStereo() const {
	return true;
}

bool TownsPC98_FmSynth::endOfData() const {
	return false;
}

int TownsPC98_FmSynth::getRate() const {
	return _mixer->getOutputRate();
}

void TownsPC98_FmSynth::deinit() {
	_ready = false;
	_mixer->stopHandle(_soundHandle);
	Common::StackLock lock(_mutex);
	_timers[0].cb = _timers[1].cb = &TownsPC98_FmSynth::idleTimerCallback;
}

void TownsPC98_FmSynth::toggleRegProtection(bool prot) {
	_regProtectionFlag = prot;
}

uint8 TownsPC98_FmSynth::readSSGStatus() {
	return _ssg->chanEnable();
}

void TownsPC98_FmSynth::setVolumeIntern(int volA, int volB) {
	Common::StackLock lock(_mutex);
	_volumeA = CLIP<uint16>(volA, 0, Audio::Mixer::kMaxMixerVolume);
	_volumeB = CLIP<uint16>(volB, 0, Audio::Mixer::kMaxMixerVolume);
	if (_ssg)
		_ssg->setVolumeIntern(_volumeA, _volumeB);
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	if (_prc)
		_prc->setVolumeIntern(_volumeA, _volumeB);
#endif
}

void TownsPC98_FmSynth::setVolumeChannelMasks(int channelMaskA, int channelMaskB) {
	Common::StackLock lock(_mutex);
	_volMaskA = channelMaskA;
	_volMaskB = channelMaskB;
	if (_ssg)
		_ssg->setVolumeChannelMasks(_volMaskA >> _numChan, _volMaskB >> _numChan);
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	if (_prc)
		_prc->setVolumeChannelMasks(_volMaskA >> (_numChan + _numSSG), _volMaskB >> (_numChan + _numSSG));
#endif
}

void TownsPC98_FmSynth::generateTables() {
	delete[] _oprRates;
	_oprRates = new uint8[128];

	WRITE_BE_UINT32(_oprRates + 32, _numChan == 6 ? 0x90900000 : 0x00081018);
	WRITE_BE_UINT32(_oprRates + 36, _numChan == 6 ? 0x00001010 : 0x00081018);
	memset(_oprRates, 0x90, 32);
	memset(&_oprRates[96], 0x80, 32);
	uint8 *dst = (uint8 *)_oprRates + 40;
	for (int i = 0; i < 40; i += 4)
		WRITE_BE_UINT32(dst + i, 0x00081018);
	for (int i = 0; i < 48; i += 4)
		WRITE_BE_UINT32(dst + i, 0x00081018);
	dst += 40;
	for (uint8 i = 0; i < 16; i ++) {
		uint8 v = (i < 12) ? i : 12;
		*dst++ = ((4 + v) << 3);
	}

	delete[] _oprRateshift;
	_oprRateshift = new uint8[128];
	memset(_oprRateshift, 0, 128);
	dst = (uint8 *)_oprRateshift + 32;
	for (int i = 11; i; i--) {
		memset(dst, i, 4);
		dst += 4;
	}

	delete[] _oprFrq;
	_oprFrq = new uint32[0x1000];
	for (uint32 i = 0; i < 0x1000; i++)
		_oprFrq[i] = (uint32)(_baserate * (float)(i << 11));

	delete[] _oprAttackDecay;
	_oprAttackDecay = new uint8[152];
	memset(_oprAttackDecay, 0, 152);
	for (int i = 0; i < 36; i++)
		WRITE_BE_UINT32(_oprAttackDecay + (i << 2), _adtStat[i]);

	delete[] _oprSinTbl;
	_oprSinTbl = new uint32[1024];
	for (int i = 0; i < 1024; i++) {
		double val = sin((double)(((i << 1) + 1) * M_PI / 1024.0));
		double d_dcb = log(1.0 / (double)ABS(val)) / log(2.0) * 256.0;
		int32 i_dcb = (int32)(2.0 * d_dcb);
		i_dcb = (i_dcb & 1) ? (i_dcb >> 1) + 1 : (i_dcb >> 1);
		_oprSinTbl[i] = (i_dcb << 1) + (val >= 0.0 ? 0 : 1);
	}

	delete[] _oprLevelOut;
	_oprLevelOut = new int32[0x1a00];
	for (int i = 0; i < 256; i++) {
		double val = floor(65536.0 / pow(2.0, 0.00390625 * (double)(1 + i)));
		int32 val_int = ((int32) val) >> 4;
		_oprLevelOut[i << 1] = (val_int & 1) ? ((val_int >> 1) + 1) << 2 : (val_int >> 1) << 2;
		_oprLevelOut[(i << 1) + 1] = -_oprLevelOut[i << 1];
		for (int ii = 1; ii < 13; ++ii) {
			_oprLevelOut[(i << 1) + (ii << 9)] =  _oprLevelOut[i << 1] >> ii;
			_oprLevelOut[(i << 1) + (ii << 9) + 1] = -_oprLevelOut[(i << 1) + (ii << 9)];
		}
	}

	uint8 *dtt = new uint8[128];
	memset(dtt, 0, 36);
	memset(&dtt[36], 1, 8);
	memcpy(&dtt[44], _detSrc, 84);

	delete[] _oprDetune;
	_oprDetune = new int32[256];
	for (int i = 0; i < 128; i++) {
		_oprDetune[i] = (int32)((float)dtt[i] * _baserate * 64.0);
		_oprDetune[i + 128] = -_oprDetune[i];
	}

	delete[] dtt;
}

void TownsPC98_FmSynth::nextTick(int32 *buffer, uint32 bufferSize) {
	if (!_ready)
		return;

	for (int i = 0; i < _numChan; i++) {
		TownsPC98_FmSynthOperator **o = _chanInternal[i].opr;

		if (_chanInternal[i].updateEnvelopeParameters) {
			_chanInternal[i].updateEnvelopeParameters = false;
			for (int ii = 0; ii < 4 ; ii++)
				o[ii]->updatePhaseIncrement();
		}

		for (uint32 ii = 0; ii < bufferSize ; ii++) {
			int32 phbuf1, phbuf2, output;
			phbuf1 = phbuf2 = output = 0;

			int32 *leftSample = &buffer[ii * 2];
			int32 *rightSample = &buffer[ii * 2 + 1];
			int32 *del = &_chanInternal[i].feedbuf[2];
			int32 *feed = _chanInternal[i].feedbuf;

			switch (_chanInternal[i].algorithm) {
			case 0:
				o[0]->generateOutput(0, feed, phbuf1);
				o[2]->generateOutput(*del, 0, phbuf2);
				*del = 0;
				o[1]->generateOutput(phbuf1, 0, *del);
				o[3]->generateOutput(phbuf2, 0, output);
				break;
			case 1:
				o[0]->generateOutput(0, feed, phbuf1);
				o[2]->generateOutput(*del, 0, phbuf2);
				o[1]->generateOutput(0, 0, phbuf1);
				o[3]->generateOutput(phbuf2, 0, output);
				*del = phbuf1;
				break;
			case 2:
				o[0]->generateOutput(0, feed, phbuf2);
				o[2]->generateOutput(*del, 0, phbuf2);
				o[1]->generateOutput(0, 0, phbuf1);
				o[3]->generateOutput(phbuf2, 0, output);
				*del = phbuf1;
				break;
			case 3:
				o[0]->generateOutput(0, feed, phbuf2);
				o[2]->generateOutput(0, 0, *del);
				o[1]->generateOutput(phbuf2, 0, phbuf1);
				o[3]->generateOutput(*del, 0, output);
				*del = phbuf1;
				break;
			case 4:
				o[0]->generateOutput(0, feed, phbuf1);
				o[2]->generateOutput(0, 0, phbuf2);
				o[1]->generateOutput(phbuf1, 0, output);
				o[3]->generateOutput(phbuf2, 0, output);
				*del = 0;
				break;
			case 5:
				o[0]->generateOutput(0, feed, phbuf1);
				o[2]->generateOutput(*del, 0, output);
				o[1]->generateOutput(phbuf1, 0, output);
				o[3]->generateOutput(phbuf1, 0, output);
				*del = phbuf1;
				break;
			case 6:
				o[0]->generateOutput(0, feed, phbuf1);
				o[2]->generateOutput(0, 0, output);
				o[1]->generateOutput(phbuf1, 0, output);
				o[3]->generateOutput(0, 0, output);
				*del = 0;
				break;
			case 7:
				o[0]->generateOutput(0, feed, output);
				o[2]->generateOutput(0, 0, output);
				o[1]->generateOutput(0, 0, output);
				o[3]->generateOutput(0, 0, output);
				*del = 0;
				break;
			};

			int32 finOut = (output << 2) / ((_numChan + _numSSG - 3) / 3);

			if ((1 << i) & _volMaskA)
				finOut = (finOut * _volumeA) / Audio::Mixer::kMaxMixerVolume;

			if ((1 << i) & _volMaskB)
				finOut = (finOut * _volumeB) / Audio::Mixer::kMaxMixerVolume;

			if (_chanInternal[i].enableLeft)
				*leftSample += finOut;

			if (_chanInternal[i].enableRight)
				*rightSample += finOut;
		}
	}
}

const uint32 TownsPC98_FmSynth::_adtStat[] = {
	0x00010001, 0x00010001, 0x00010001, 0x01010001,
	0x00010101, 0x00010101, 0x00010101, 0x01010101,
	0x01010101, 0x01010101, 0x01010102, 0x01010102,
	0x01020102, 0x01020102, 0x01020202, 0x01020202,
	0x02020202, 0x02020202, 0x02020204, 0x02020204,
	0x02040204, 0x02040204, 0x02040404, 0x02040404,
	0x04040404, 0x04040404, 0x04040408, 0x04040408,
	0x04080408, 0x04080408, 0x04080808, 0x04080808,
	0x08080808, 0x08080808, 0x10101010, 0x10101010
};

const uint8 TownsPC98_FmSynth::_detSrc[] = {
	0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x01, 0x01, 0x01, 0x01,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07,
	0x08, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	0x10, 0x10, 0x10, 0x10, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05,
	0x05, 0x06, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a,
	0x0b, 0x0c, 0x0d, 0x0e, 0x10, 0x11, 0x13, 0x14,
	0x16, 0x16, 0x16, 0x16
};

const int TownsPC98_FmSynth::_ssgTables[] = {
	0x01202A, 0x0092D2, 0x006B42, 0x0053CB, 0x003DF8, 0x003053, 0x0022DA, 0x001A8C,
	0x00129B, 0x000DC1, 0x000963, 0x0006C9, 0x000463, 0x0002FA, 0x0001B6, 0x0000FB,
	0x0193B6, 0x01202A, 0x00CDB1, 0x0092D2, 0x007D7D, 0x006B42, 0x005ECD, 0x0053CB,
	0x00480F, 0x003DF8, 0x0036B9, 0x003053, 0x00290A, 0x0022DA, 0x001E6B, 0x001A8C,
	0x001639, 0x00129B, 0x000FFF, 0x000DC1, 0x000B5D, 0x000963, 0x0007FB, 0x0006C9,
	0x000575, 0x000463, 0x00039D, 0x0002FA, 0x000242, 0x0001B6, 0x00014C, 0x0000FB
};

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
const uint8 TownsPC98_FmSynth::_percussionData[] = {
	0, 24, 1, 192, 1, 216, 2, 128, 4, 88, 23, 64, 27, 152, 1, 128, 29, 24, 2, 128, 31, 152, 0, 128, 136, 128, 128, 128, 0, 136, 97, 103, 153, 139, 34, 163, 72, 195, 27, 69, 1, 154, 137, 35, 8, 51, 169, 122, 164, 75, 133, 203, 81, 146, 168, 121, 185, 68, 202, 8, 33, 237, 49, 177, 12, 133, 140, 17, 160, 42, 161, 10, 0, 137, 176, 57,
	233, 41, 160, 136, 235, 65, 177, 137, 128, 26, 164, 28, 3, 157, 51, 137, 1, 152, 113, 161, 40, 146, 115, 192, 56, 5, 169, 66, 161, 56, 1, 50, 145, 59, 39, 168, 97, 1, 160, 57, 7, 153, 50, 153, 32, 2, 25, 129, 32, 20, 186, 66, 129, 24, 153, 164, 142, 130, 169, 153, 26, 242, 138, 217, 9, 128, 204, 58, 209, 172, 40, 176, 141,
	128, 155, 144, 203, 139, 0, 235, 9, 177, 172, 0, 185, 168, 138, 25, 240, 59, 211, 139, 19, 176, 90, 160, 17, 26, 132, 41, 1, 5, 25, 3, 50, 144, 115, 147, 42, 39, 152, 41, 3, 56, 193, 105, 130, 155, 66, 200, 26, 19, 218, 154, 49, 201, 171, 138, 176, 251, 139, 185, 172, 136, 189, 139, 145, 207, 41, 160, 171, 152, 186, 139,
	186, 141, 128, 218, 171, 51, 217, 170, 56, 163, 12, 4, 155, 81, 147, 42, 37, 152, 32, 54, 136, 49, 50, 48, 37, 32, 69, 0, 17, 50, 50, 83, 2, 16, 68, 20, 8, 66, 4, 154, 84, 145, 24, 33, 24, 32, 17, 18, 145, 32, 22, 168, 49, 163, 1, 33, 50, 184, 115, 129, 25, 66, 1, 24, 67, 2, 80, 35, 40, 53, 2, 65, 51, 19, 67, 37, 0, 52, 35, 49, 37,
	34, 49, 37, 17, 52, 17, 35, 35, 35, 34, 32, 49, 33, 152, 34, 145, 24, 24, 128, 138, 128, 184, 9, 177, 171, 168, 185, 155, 152, 172, 155, 186, 172, 185, 172, 155, 186, 173, 153, 202, 187, 185, 202, 170, 171, 202, 186, 169, 170, 170, 171, 139, 154, 171, 153, 154, 169, 10, 168, 154, 128, 168, 154, 0, 153, 152, 136, 137,
	128, 153, 0, 152, 8, 128, 137, 0, 136, 136, 8, 9, 8, 9, 8, 24, 153, 128, 136, 153, 144, 0, 161, 138, 1, 169, 136, 128, 160, 168, 152, 153, 138, 137, 154, 153, 153, 154, 153, 170, 168, 170, 185, 168, 169, 154, 169, 171, 153, 169, 170, 153, 152, 154, 153, 137, 169, 137, 136, 144, 152, 144, 128, 128, 144, 129, 129, 0, 33,
	0, 17, 17, 17, 33, 33, 18, 18, 34, 34, 34, 34, 34, 34, 35, 19, 35, 19, 35, 35, 18, 19, 18, 35, 18, 33, 0, 8, 8, 8, 8, 8, 8, 8, 160, 205, 65, 176, 171, 203, 16, 240, 95, 242, 120, 145, 156, 66, 177, 26, 19, 153, 9, 35, 35, 239, 56, 132, 138, 154, 50, 145, 203, 25, 32, 20, 237, 24, 130, 138, 160, 27, 39, 173, 50, 203, 64, 145, 139,
	18, 168, 48, 146, 171, 65, 18, 176, 12, 52, 128, 25, 5, 57, 240, 104, 161, 25, 129, 18, 188, 114, 160, 26, 36, 200, 154, 18, 1, 128, 186, 73, 162, 173, 32, 184, 25, 144, 137, 234, 8, 154, 32, 160, 158, 18, 187, 81, 2, 235, 41, 36, 144, 154, 17, 67, 128, 33, 160, 114, 146, 26, 37, 33, 232, 41, 130, 41, 178, 29, 50, 251, 24,
	1, 153, 138, 160, 76, 179, 155, 11, 0, 38, 252, 41, 146, 41, 178, 27, 193, 43, 39, 170, 136, 17, 129, 8, 49, 233, 48, 129, 11, 6, 26, 130, 136, 128, 64, 1, 248, 105, 145, 9, 16, 144, 140, 5, 25, 168, 16, 186, 48, 5, 171, 217, 57, 134, 171, 8, 34, 188, 20, 203, 41, 6, 155, 161, 89, 164, 140, 2, 136, 51, 202, 41, 131, 56, 144,
	8, 97, 144, 146, 13, 69, 200, 42, 130, 25, 152, 57, 6, 220, 88, 177, 26, 148, 9, 168, 8, 67, 192, 156, 65, 145, 137, 10, 4, 154, 18, 157, 67, 160, 154, 1, 50, 188, 82, 170, 82, 185, 49, 220, 97, 144, 10, 8, 16, 145, 9, 136, 18, 202, 51, 184, 141, 114, 179, 139, 24, 19, 8, 250, 121, 160, 40, 160, 10, 18, 152, 168, 42, 35, 216,
	187, 120, 145, 18, 156, 203, 84, 144, 9, 144, 26, 66, 161, 13, 1, 128, 17, 154, 18, 142, 6, 154, 65, 192, 29, 35, 186, 64, 192, 24, 9, 146, 56, 185, 16, 248, 121, 176, 40, 129, 136, 171, 96, 147, 140, 50, 203, 64, 144, 41, 128, 161, 187, 71, 200, 24, 129, 24, 217, 56, 20, 220, 24, 4, 169, 9, 1, 33, 201, 26, 134, 141, 51, 201,
	25, 16, 33, 235, 32, 144, 33, 153, 169, 99, 160, 11, 3, 136, 58, 210, 33, 203, 48, 163, 17, 219, 128, 140, 38, 8, 184, 141, 50, 131, 159, 33, 128, 153, 25, 18, 153, 88, 242, 43, 3, 9, 136, 157, 53, 202, 40, 145, 25, 2, 204, 105, 146, 156, 66, 152, 8, 153, 33, 128, 129, 136, 153, 50, 186, 55, 188, 51, 249, 64, 178, 27, 128,
	48, 177, 156, 18, 35, 175, 51, 189, 32, 51, 234, 155, 69, 184, 26, 2, 152, 9, 17, 136, 144, 137, 50, 235, 115, 216, 24, 2, 170, 67, 187, 49, 129, 155, 4, 27, 129, 56, 232, 43, 39, 203, 40, 3, 154, 169, 66, 184, 114, 224, 25, 2, 9, 128, 11, 35, 155, 18, 11, 202, 84, 169, 26, 5, 154, 8, 160, 98, 185, 17, 187, 50, 23, 188, 33,
	1, 139, 4, 154, 90, 147, 12, 3, 43, 2, 170, 171, 103, 193, 28, 132, 137, 8, 129, 24, 170, 50, 201, 42, 35, 202, 169, 52, 201, 33, 218, 40, 39, 203, 0, 40, 147, 29, 163, 139, 83, 185, 1, 4, 159, 34, 160, 12, 21, 155, 40, 129, 137, 58, 151, 13, 2, 136, 144, 16, 153, 40, 17, 131, 207, 51, 144, 140, 4, 154, 17, 146, 170, 73, 163,
	44, 164, 12, 152, 37, 203, 17, 128, 144, 139, 23, 154, 128, 138, 38, 216, 41, 1, 0, 233, 73, 131, 171, 49, 136, 9, 164, 46, 3, 171, 32, 0, 145, 157, 38, 187, 64, 176, 58, 134, 155, 18, 136, 217, 64, 1, 200, 140, 38, 153, 170, 66, 161, 8, 169, 65, 185, 98, 200, 41, 3, 155, 144, 58, 23, 187, 1, 145, 40, 147, 189, 32, 68, 249,
	1, 112, 255, 199, 195, 19, 108, 76, 187, 247, 247, 183, 40, 168, 212, 245, 199, 227, 68, 45, 59, 10, 145, 177, 198, 24, 130, 76, 26, 193, 180, 129, 0, 162, 42, 160, 199, 162, 0, 16, 152, 137, 132, 168, 195, 130, 162, 181, 227, 163, 161, 179, 211, 180, 179, 164, 128, 162, 161, 194, 164, 179, 40, 153, 195, 213, 146, 178,
	147, 176, 50, 186, 161, 196, 151, 58, 16, 28, 162, 160, 131, 122, 155, 33, 241, 146, 128, 40, 26, 128, 154, 36, 170, 89, 59, 9, 24, 144, 77, 161, 8, 177, 112, 139, 33, 232, 148, 24, 41, 61, 9, 26, 162, 32, 30, 58, 153, 32, 59, 73, 59, 11, 79, 137, 57, 9, 49, 30, 24, 153, 131, 25, 106, 61, 153, 73, 28, 56, 27, 41, 137, 148,
	76, 43, 74, 58, 13, 161, 3, 171, 149, 32, 77, 10, 74, 42, 168, 16, 0, 123, 138, 129, 162, 178, 225, 50, 140, 161, 0, 147, 10, 129, 41, 244, 210, 165, 1, 152, 24, 162, 184, 166, 32, 144, 59, 216, 132, 177, 8, 145, 67, 143, 146, 160, 183, 162, 130, 24, 192, 32, 225, 146, 144, 33, 44, 73, 30, 129, 137, 32, 76, 152, 25, 161,
	2, 154, 32, 177, 132, 232, 2, 136, 210, 128, 149, 177, 32, 58, 27, 168, 225, 133, 8, 44, 107, 136, 25, 136, 17, 26, 58, 46, 16, 11, 145, 17, 144, 79, 136, 144, 136, 145, 152, 33, 31, 162, 130, 200, 82, 153, 74, 137, 147, 26, 0, 13, 133, 170, 149, 16, 192, 0, 178, 0, 128, 152, 182, 150, 9, 16, 9, 137, 33, 59, 63, 10, 152, 32,
	179, 192, 5, 154, 228, 182, 145, 130, 144, 42, 128, 242, 2, 136, 41, 168, 17, 76, 57, 31, 129, 136, 17, 47, 8, 41, 138, 32, 138, 123, 59, 58, 10, 136, 161, 4, 46, 25, 145, 136, 129, 25, 56, 28, 91, 41, 154, 108, 9, 16, 44, 24, 137, 48, 15, 0, 194, 162, 41, 194, 56, 241, 163, 146, 0, 139, 7, 186, 150, 129, 152, 1, 208, 33, 176,
	136, 164, 163, 185, 7, 138, 130, 242, 162, 163, 177, 88, 136, 184, 166, 146, 0, 25, 25, 177, 199, 146, 16, 136, 9, 145, 178, 178, 0, 147, 138, 229, 18, 152, 25, 144, 163, 246, 162, 129, 129, 184, 5, 152, 178, 145, 148, 136, 146, 95, 152, 128, 144, 33, 170, 81, 11, 40, 202, 131, 0, 243, 24, 1, 11, 148, 42, 24, 163, 140,
	120, 9, 76, 58, 153, 145, 56, 30, 72, 46, 42, 9, 8, 57, 91, 76, 59, 26, 160, 129, 41, 76, 10, 57, 192, 163, 129, 16, 225, 2, 27, 40, 200, 48, 91, 226, 40, 145, 43, 177, 177, 182, 196, 145, 33, 184, 165, 17, 192, 163, 194, 129, 211, 128, 162, 197, 129, 0, 136, 211, 146, 8, 162, 144, 0, 167, 160, 1, 176, 150, 137, 1, 24, 243,
	0, 129, 145, 25, 123, 169, 130, 168, 132, 41, 63, 42, 136, 137, 120, 26, 136, 8, 24, 89, 29, 58, 177, 193, 147, 1, 26, 162, 176, 167, 180, 8, 49, 28, 29, 178, 162, 88, 43, 42, 57, 43, 61, 8, 29, 129, 128, 128, 123, 137, 24, 243, 16, 136, 16, 46, 0, 169, 149, 128, 1, 60, 153, 72, 154, 90, 25, 25, 25, 8, 91, 73, 12, 16, 137, 144,
	72, 11, 8, 167, 128, 129, 9, 138, 166, 193, 147, 162, 123, 137, 145, 1, 162, 26, 1, 219, 147, 129, 210, 147, 243, 1, 243, 16, 144, 145, 160, 131, 200, 4, 59, 75, 57, 218, 2, 178, 77, 24, 60, 11, 147, 10, 50, 141, 64, 27, 185, 122, 161, 41, 128, 90, 136, 24, 46, 16, 139, 16, 24, 28, 124, 9, 41, 8, 26, 121, 10, 42, 40, 139, 129,
	0, 201, 135, 137, 56, 176, 176, 35, 215, 145, 1, 26, 145, 144, 160, 135, 138, 1, 177, 146, 146, 161, 65, 242, 136, 164, 177, 1, 1, 186, 151, 208, 148, 129, 10, 32, 241, 145, 163, 178, 17, 168, 136, 151, 168, 2, 148, 185, 133, 176, 130, 129, 154, 163, 215, 0, 146, 136, 40, 211, 161, 131, 171, 81, 144, 170, 21, 184, 56,
	195, 168, 133, 177, 91, 16, 187, 5, 145, 153, 66, 172, 18, 177, 42, 120, 138, 27, 134, 26, 106, 42, 138, 146, 184, 66, 75, 46, 41, 168, 0, 145, 57, 91, 75, 27, 24, 27, 48, 169, 40, 122, 9, 109, 10, 8, 177, 146, 16, 74, 30, 129, 160, 162, 146, 41, 124, 138, 24, 145, 152, 3, 1, 14, 3, 139, 1, 192, 161, 151, 177, 122, 8, 10, 0,
	176, 130, 129, 27, 88, 225, 0, 2, 154, 129, 129, 193, 49, 203, 81, 153, 226, 33, 0, 30, 0, 176, 179, 18, 9, 96, 156, 162, 148, 160, 129, 2, 29, 195, 128, 0, 56, 156, 20, 232, 129, 128, 32, 10, 144, 74, 183, 9, 145, 162, 1, 162, 138, 23, 171, 1, 164, 224, 34, 43, 43, 177, 200, 135, 161, 91, 57, 154, 177, 148, 145, 146, 58,
	108, 136, 170, 35, 208, 177, 34, 128, 44, 129, 155, 151, 243, 16, 1, 154, 72, 193, 144, 18, 11, 122, 160, 153, 5, 192, 24, 130, 184, 132, 226, 0, 128, 153, 131, 181, 136, 65, 154, 128, 17, 170, 39, 28, 59, 144, 168, 80, 25, 47, 24, 26, 144, 32, 47, 41, 153, 161, 148, 8, 92, 9, 9, 129, 144, 33, 26, 47, 24, 137, 108, 25, 10,
	17, 10, 73, 75, 47, 24, 184, 48, 8, 45, 57, 138, 136, 150, 10, 48, 139, 136, 35, 203, 121, 8, 27, 179, 161, 106, 0, 29, 16, 176, 179, 3, 185, 19, 227, 41, 145, 168, 61, 197, 177, 20, 10, 57, 42, 250, 147, 196, 16, 41, 138, 24, 195, 208, 135, 137, 0, 145, 160, 2, 210, 146, 195, 177, 132, 136, 153, 167, 210, 146, 162, 40, 8,
	138, 148, 227, 145, 17, 137, 40, 169, 179, 130, 242, 2, 196, 9, 146, 145, 169, 167, 146, 130, 137, 136, 51, 220, 17, 163, 28, 74, 10, 76, 40, 140, 5, 137, 43, 18, 12, 107, 137, 40, 8, 201, 50, 0, 143, 3, 138, 161, 134, 138, 104, 169, 16, 162, 160, 121, 25, 28, 129, 152, 32, 56, 14, 16, 184, 146, 3, 46, 25, 176, 129, 179,
	193, 17, 130, 202, 135, 8, 57, 25, 154, 148, 184, 120, 9, 153, 211, 165, 24, 128, 26, 17, 242, 161, 18, 185, 81, 42, 11, 17, 12, 25, 181, 137, 66, 42, 47, 41, 184, 166, 129, 24, 91, 27, 136, 196, 0, 0, 74, 28, 178, 161, 149, 160, 32, 8, 225, 32, 128, 59, 8, 169, 50, 139, 47, 72, 186, 16, 132, 9, 122, 9, 160, 146, 144, 89, 153,
	10, 149, 178, 0, 121, 11, 146, 152, 162, 48, 13, 123, 177, 24, 0, 106, 27, 9, 144, 132, 12, 17, 0, 168, 0, 181, 56, 169, 129, 242, 195, 129, 17, 154, 64, 161, 244, 16, 137, 24, 144, 144, 164, 129, 75, 42, 176, 149, 9, 179, 148, 203, 4, 166, 136, 163, 128, 227, 163, 8, 57, 11, 30, 165, 0, 74, 59, 62, 9, 208, 131, 144, 40, 76,
	26, 27, 196, 129, 1, 25, 43, 49, 174, 67, 153, 136, 106, 152, 41, 25, 28, 2, 43, 44, 104, 45, 59, 8, 43, 128, 144, 120, 25, 12, 17, 152, 9, 130, 155, 151, 145, 74, 40, 13, 48, 192, 58, 90, 43, 43, 177, 146, 49, 31, 75, 24, 217, 131, 0, 76, 26, 152, 149, 161, 24, 74, 154, 193, 166, 145, 32, 27, 161, 164, 176, 135, 152, 24, 193,
	162, 146, 164, 58, 227, 193, 148, 161, 128, 18, 234, 130, 180, 145, 2, 200, 1, 163, 186, 98, 184, 129, 149, 153, 49, 42, 186, 151, 242, 129, 1, 43, 8, 177, 212, 165, 8, 40, 137, 24, 8, 144, 90, 9, 25, 48, 44, 46, 24, 138, 40, 144, 108, 58, 27, 128, 181, 128, 80, 29, 42, 152, 162, 130, 25, 106, 136, 11, 148, 8, 144, 128, 136,
	112, 139, 80, 153, 24, 136, 129, 46, 0, 60, 129, 208, 1, 3, 13, 57, 168, 144, 1, 242, 17, 9, 26, 2, 185, 27, 55, 140, 73, 137, 179, 16, 192, 3, 145, 143, 33, 9, 171, 135, 160, 17, 137, 10, 151, 168, 3, 178, 44, 17, 208, 144, 167, 0, 40, 155, 16, 167, 152, 18, 144, 26, 160, 199, 1, 136, 91, 136, 160, 178, 150, 161, 1, 10, 181,
	145, 161, 1, 145, 161, 198, 2, 9, 90, 137, 177, 160, 150, 40, 29, 129, 144, 145, 162, 57, 77, 169, 16, 148, 42, 42, 40, 141, 34, 170, 121, 154, 210, 131, 162, 107, 8, 9, 160, 195, 40, 73, 139, 18, 224, 162, 34, 139, 0, 244, 178, 163, 24, 26, 146, 194, 166, 49, 29, 42, 137, 130, 192, 16, 93, 128, 154, 19, 59, 11, 122, 11,
	146, 177, 120, 42, 26, 43, 164, 152, 17, 60, 63, 137, 128, 48, 10, 58, 92, 9, 59, 91, 75, 139, 32, 25, 25, 61, 74, 28, 177, 40, 130, 74, 29, 73, 168, 130, 128, 48, 14, 8, 77, 9, 25, 26, 179, 211, 32, 78, 26, 41, 152, 161, 180, 89, 59, 9, 153, 166, 160, 3, 26, 57, 106, 154, 88, 184, 40, 1, 27, 58, 73, 143, 131, 169, 3, 161, 184,
	122, 152, 16, 181, 145, 129, 17, 15, 129, 193, 147, 145, 192, 33, 193, 162, 183, 163, 136, 178, 129, 178, 197, 2, 41, 216, 131, 168, 163, 181, 226, 163, 178, 1, 33, 187, 166, 212, 129, 1, 27, 24, 162, 184, 151, 8, 16, 160, 144, 181, 210, 72, 168, 128, 32, 42, 25, 40, 142, 5, 185, 88, 58, 11, 58, 177, 32, 129, 63, 42, 136,
	186, 53, 29, 75, 58, 144, 144, 129, 77, 128, 11, 144, 133, 29, 40, 152, 24, 161, 129, 80, 155, 60, 3, 12, 89, 8, 60, 152, 152, 49, 136, 47, 57, 224, 129, 16, 41, 90, 139, 162, 147, 170, 51, 169, 27, 17, 95, 26, 26, 160, 5, 139, 48, 76, 10, 228, 146, 1, 136, 44, 161, 147, 209, 130, 137, 73, 224, 1, 162, 195, 32, 210, 177, 180,
	179, 148, 145, 154, 132, 242, 146, 1, 152, 32, 192, 1, 144, 155, 7, 177, 168, 5, 138, 178, 148, 152, 150, 136, 89, 152, 9, 41, 196, 145, 40, 28, 16, 8, 10, 178, 167, 24, 1, 44, 123, 137, 136, 145, 194, 48, 27, 74, 26, 192, 179, 135, 136, 88, 27, 10, 177, 163, 164, 128, 73, 24, 31, 8, 0, 192, 149, 144, 129, 9, 106, 41, 200,
	161, 151, 41, 138, 0, 24, 226, 162, 49, 42, 11, 90, 136, 136, 152, 17, 145, 10, 63, 40, 11, 56, 245, 162, 16, 26, 73, 11, 144, 135, 137, 58, 106, 10, 25, 8, 57, 137, 28, 33, 129, 156, 113, 10, 10, 161, 18, 8, 153, 77, 3, 217, 0, 1, 242, 128, 193, 18, 128, 75, 60, 178, 154, 37, 45, 58, 29, 144, 1, 184, 66, 41, 29, 8, 145, 10,
	194, 33, 148, 170, 107, 89, 139, 128, 163, 178, 16, 63, 59, 176, 144, 151, 129, 42, 74, 10, 129, 192, 2, 128, 154, 97, 192, 0, 177, 128, 178, 183, 16, 16, 155, 149, 145, 184, 84, 138, 8, 192, 161, 20, 225, 0, 130, 138, 165, 0, 28, 148, 153, 18, 209, 128, 88, 153, 89, 152, 9, 17, 9, 29, 130, 43, 122, 153, 24, 32, 202, 49,
	24, 43, 106, 154, 130, 193, 27, 51, 29, 28, 133, 138, 65, 11, 123, 25, 10, 40, 152, 44, 130, 26, 43, 148, 45, 73, 140, 33, 8, 153, 88, 128, 61, 144, 42, 59, 225, 128, 18, 155, 50, 75, 186, 20, 202, 120, 144, 42, 92, 176, 162, 165, 25, 2, 169, 152, 135, 185, 19, 152, 8, 146, 160, 123, 195, 137, 132, 209, 0, 16, 11, 2, 242,
	146, 164, 152, 73, 193, 136, 130, 178, 1, 136, 169, 23, 169, 128, 164, 242, 129, 178, 129, 32, 138, 180, 167, 153, 132, 8, 138, 2, 209, 4, 138, 1, 128, 138, 92, 136, 44, 129, 136, 162, 33, 63, 40, 141, 2, 160, 144, 106, 137, 64, 155, 17, 129, 60, 30, 146, 26, 17, 28, 48, 46, 169, 51, 154, 91, 137, 41, 26, 32, 143, 18, 138,
	1, 32, 28, 123, 177, 9, 181, 195, 56, 57, 14, 145, 161, 17, 17, 31, 41, 152, 145, 194, 194, 20, 153, 41, 9, 243, 129, 180, 0, 128, 45, 16, 43, 170, 135, 144, 16, 25, 42, 137, 242, 163, 194, 16, 0, 57, 14, 130, 194, 178, 16, 33, 30, 8, 59, 211, 163, 160, 5, 137, 44, 10, 17, 170, 3, 120, 9, 44, 146, 136, 131, 140, 91, 9, 171,
	7, 161, 32, 73, 13, 8, 161, 40, 106, 11, 25, 129, 59, 0, 49, 31, 42, 28, 40, 11, 0, 81, 176, 61, 32, 138, 25, 178, 241, 148, 136, 106, 8, 136, 128, 177, 90, 8, 155, 96, 176, 9, 18, 217, 132, 129, 10, 81, 156, 40, 178, 161, 36, 169, 76, 147, 203, 150, 0, 10, 146, 200, 147, 149, 128, 144, 148, 154, 182, 24, 0, 137, 11, 134, 211,
	24, 136, 129, 145, 209, 33, 8, 43, 163, 243, 88, 41, 13, 0, 160, 145, 33, 31, 32, 185, 145, 4, 155, 17, 32, 47, 161, 128, 73, 160, 44, 56, 176, 75, 74, 12, 35, 141, 104, 137, 9, 89, 152, 58, 56, 44, 41, 30, 41, 40, 157, 48, 128, 154, 88, 41, 42, 8, 14, 3, 184, 59, 120, 152, 9, 56, 10, 128, 41, 57, 227, 186, 52, 152, 62, 8, 56,
	242, 0, 58, 8, 156, 34, 243, 128, 24, 176, 51, 169, 58, 183, 192, 146, 164, 177, 18, 170, 7, 177, 208, 132, 161, 24, 136, 27, 147, 243, 128, 133, 10, 24, 161, 161, 178, 214, 17, 160, 25, 16, 161, 137, 165, 192, 48, 27, 72, 58, 218, 133, 162, 26, 72, 27, 10, 197, 178, 49, 138, 89, 56, 142, 1, 24, 11, 0, 44, 105, 10, 25, 0,
	194, 9, 3, 47, 8, 138, 147, 18, 28, 48, 202, 147, 199, 146, 25, 161, 0, 145, 194, 163, 57, 11, 146, 248, 130, 32, 57, 63, 154, 16, 48, 14, 128, 144, 209, 133, 26, 56, 154, 182, 162, 195, 18, 152, 44, 194, 180, 168, 5, 24, 137, 138, 35, 192, 232, 66, 176, 161, 24, 41, 26, 244, 129, 163, 160, 75, 129, 226, 147, 40, 145, 61,
	13, 130, 177, 17, 137, 112, 170, 130, 0, 136, 75, 152, 177, 241, 34, 0, 59, 156, 51, 186, 178, 91, 132, 137, 137, 122, 1, 45, 28, 50, 172, 57, 108, 8, 26, 136, 32, 152, 46, 144, 131, 171, 4, 152, 18, 141, 148, 1, 216, 32, 9, 60, 169, 66, 152, 128, 72, 90, 201, 1, 17, 201, 136, 3, 195, 26, 73, 133, 200, 176, 150, 146, 169,
	24, 33, 178, 184, 151, 73, 11, 28, 72, 44, 153, 82, 153, 17, 42, 57, 78, 153, 8, 160, 0, 1, 123, 11, 19, 171, 195, 18, 59, 31, 129, 10, 162, 2, 58, 96, 142, 130, 26, 75, 128, 176, 17, 180, 123, 9, 90, 137, 211, 145, 32, 26, 76, 43, 145, 130, 12, 90, 41, 27, 58, 160, 160, 128, 178, 7, 76, 59, 0, 203, 180, 147, 33, 62, 10, 0, 243,
	129, 146, 73, 29, 145, 144, 0, 26, 56, 153, 185, 83, 8, 76, 27, 166, 161, 193, 146, 131, 224, 145, 165, 161, 40, 168, 149, 162, 226, 2, 136, 138, 163, 131, 211, 0, 59, 146, 218, 148, 1, 192, 16, 16, 58, 248, 88, 144, 177, 136, 1, 58, 45, 9, 195, 197, 147, 48, 29, 10, 0, 162, 176, 64, 122, 9, 10, 17, 9, 153, 56, 75, 27, 31,
	72, 136, 9, 129, 129, 61, 45, 59, 10, 161, 18, 122, 43, 59, 41, 169, 34, 155, 130, 131, 219, 120, 162, 27, 49, 208, 160, 131, 156, 66, 12, 145, 50, 240, 16, 136, 12, 162, 40, 129, 130, 15, 129, 162, 146, 180, 83, 139, 58, 217, 129, 177, 4, 0, 169, 197, 163, 144, 242, 131, 168, 179, 179, 17, 197, 145, 178, 164, 128, 160,
	211, 2, 244, 163, 145, 162, 129, 212, 177, 163, 17, 208, 163, 195, 180, 57, 24, 170, 182, 164, 129, 0, 60, 60, 169, 149, 162, 177, 122, 26, 24, 136, 136, 133, 43, 27, 178, 56, 77, 24, 128, 240, 0, 2, 44, 46, 8, 128, 193, 146, 64, 27, 42, 16, 193, 25, 0, 192, 148, 11, 52, 47, 153, 147, 243, 0, 24, 73, 28, 144, 161, 150, 9,
	8, 73, 170, 2, 162, 25, 27, 147, 167, 131, 29, 1, 168, 200, 165, 16, 91, 137, 8, 162, 176, 35, 41, 31, 24, 169, 50, 168, 58, 123, 144, 48, 128, 13, 73, 169, 144, 16, 57, 123, 44, 200, 163, 56, 153, 80, 10, 176, 146, 57, 94, 8, 152, 131, 9, 168, 125, 26, 145, 177, 132, 137, 41, 60, 26, 144, 243, 32, 192, 34, 60, 43, 26, 16,
	249, 164, 16, 58, 61, 11, 130, 243, 146, 2, 42, 44, 27, 128, 165, 137, 49, 45, 28, 16, 43, 8, 211, 48, 28, 152, 105, 9, 9, 163, 161, 169, 35, 107, 42, 232, 164, 130, 168, 72, 42, 168, 210, 148, 144, 136, 129, 3, 217, 194, 50, 27, 192, 41, 210, 147, 40, 76, 226, 1, 161, 1, 155, 132, 145, 147, 171, 67, 173, 210, 132, 161, 106,
	137, 56, 169, 209, 131, 64, 13, 129, 9, 194, 17, 57, 61, 169, 17, 128, 40, 31, 16, 10, 162, 57, 61, 75, 139, 40, 242, 17, 58, 59, 138, 179, 144, 50, 105, 140, 179, 243, 57, 40, 26, 9, 243, 130, 24, 29, 57, 128, 210, 129, 25, 59, 91, 137, 162, 178, 72, 27, 181, 168, 19, 129, 8, 184, 231, 147, 178, 32, 28, 184, 198, 148, 144,
	1, 26, 128, 16, 192, 2, 26, 144, 244, 129, 0, 16, 10, 197, 177, 181, 1, 41, 9, 178, 165, 211, 129, 25, 145, 137, 210, 147, 152, 210, 163, 132, 194, 17, 91, 169, 145, 181, 130, 9, 89, 137, 152, 178, 4, 128, 9, 63, 160, 128, 106, 8, 25, 43, 10, 32, 47, 26, 123, 152, 24, 40, 25, 27, 18, 186, 35, 158, 64, 42, 216, 33, 25, 58, 58,
	45, 184, 147, 29, 72, 46, 9, 0, 178, 146, 58, 77, 26, 25, 209, 165, 128, 145, 17, 153, 128, 129, 148, 240, 129, 1, 40, 31, 0, 152, 242, 163, 16, 59, 44, 24, 243, 146, 128, 1, 26, 26, 179, 213, 145, 130, 176, 131, 40, 25, 145, 219, 179, 167, 8, 33, 59, 14, 176, 166, 16, 136, 74, 128, 176, 128, 149, 8, 8, 209, 148, 152, 0, 72,
	153, 161, 178, 35, 62, 75, 154, 163, 153, 19, 62, 170, 133, 179, 136, 89, 12, 129, 164, 144, 3, 47, 58, 193, 177, 148, 0, 61, 43, 10, 129, 17, 41, 61, 43, 25, 8, 126, 26, 25, 137, 145, 34, 44, 45, 129, 216, 179, 1, 90, 25, 137, 32, 227, 8, 16, 9, 170, 49, 31, 32, 29, 128, 145, 148, 75, 25, 75, 153, 162, 192, 35, 12, 80, 136,
	176, 8, 194, 24, 1, 176, 21, 154, 145, 80, 251, 130, 2, 30, 9, 8, 130, 145, 128, 98, 27, 26, 129, 136, 162, 15, 33, 168, 59, 65, 177, 77, 141, 1, 128, 168, 113, 10, 137, 178, 163, 146, 132, 74, 153, 224, 164, 33, 184, 19, 184, 228, 161, 17, 91, 152, 25, 146, 152, 44, 121, 9, 160, 145, 17, 25, 28, 93, 128, 152, 2, 25, 27, 161,
	210, 129, 146, 45, 179, 227, 163, 162, 9, 40, 193, 148, 179, 57, 107, 140, 196, 32, 25, 57, 47, 136, 210, 130, 24, 40, 28, 152, 210, 182, 145, 40, 8, 129, 184, 147, 147, 140, 163, 166, 160, 34, 45, 144, 194, 161, 134, 41, 46, 152, 162, 162, 3, 44, 58, 75, 209, 162, 144, 57, 129, 47, 152, 130, 59, 16, 248, 129, 17, 26, 57,
	9, 29, 167, 2, 60, 42, 138, 136, 209, 130, 90, 42, 42, 176, 146, 178, 120, 28, 8, 160, 145, 16, 33, 31, 1, 8, 160, 129, 128, 242, 164, 32, 152, 177, 146, 213, 196, 128, 40, 26, 160, 163, 180, 146, 108, 60, 144, 144, 136, 147, 137, 40, 90, 161, 3, 17, 219, 243, 33, 184, 130, 60, 136, 243, 178, 179, 132, 26, 8, 168, 212, 147,
	16, 57, 42, 31, 145, 145, 160, 32, 43, 184, 66, 45, 180, 33, 140, 226, 1, 91, 152, 16, 144, 193, 162, 48, 77, 25, 137, 153, 17, 178, 78, 0, 0, 16, 14, 90, 152, 153, 19, 129, 13, 123, 137, 129, 160, 1, 73, 44, 9, 129, 0, 153, 120, 10, 9, 162, 195, 32, 139, 28, 151, 161, 2, 128, 26, 45, 193, 146, 48, 29, 146, 153, 194, 5, 59,
	29, 128, 144, 195, 1, 64, 43, 208, 178, 149, 8, 9, 16, 240, 163, 129, 16, 42, 185, 181, 211, 24, 48, 45, 137, 149, 9, 24, 41, 75, 184, 177, 4, 43, 91, 128, 180, 16, 144, 29, 25, 184, 167, 1, 59, 60, 153, 148, 161, 146, 91, 42, 186, 4, 24, 145, 123, 11, 2, 178, 77, 136, 26, 25, 195, 40, 115, 61, 27, 168, 177, 3, 59, 79, 26, 25,
	144, 1, 48, 13, 56, 154, 248, 1, 16, 9, 129, 8, 2, 178, 31, 130, 153, 162, 20, 15, 33, 170, 56, 40, 29, 28, 128, 152, 149, 144, 56, 120, 11, 162, 212, 129, 144, 145, 59, 180, 243, 147, 145, 144, 16, 152, 48, 241, 0, 161, 176, 1, 134, 10, 129, 200, 166, 144, 128, 121, 26, 24, 177, 178, 196, 48, 75, 138, 41, 180, 195, 26, 24,
	89, 138, 24, 33, 187, 41, 84, 155, 57, 79, 136, 160, 210, 130, 0, 58, 58, 168, 243, 132, 27, 41, 75, 138, 3, 8, 61, 8, 29, 145, 179, 76, 24, 28, 146, 208, 2, 49, 140, 75, 196, 144, 0, 40, 44, 179, 208, 3, 176, 33, 15, 177, 2, 160, 106, 8, 160, 164, 164, 8, 73, 27, 226, 179, 161, 1, 57, 1, 196, 211, 128, 40, 156, 145, 166, 178,
	131, 29, 128, 145, 162, 165, 40, 27, 216, 146, 135, 144, 40, 160, 194, 177, 145, 20, 139, 200, 151, 178, 17, 136, 40, 25, 205, 130, 17, 11, 17, 129, 156, 38, 26, 25, 137, 179, 163, 11, 79, 16, 12, 146, 147, 143, 89, 25, 136, 136, 25, 48, 26, 46, 129, 40, 29, 42, 29, 8, 145, 2, 56, 27, 62, 8, 25, 212, 161, 48, 43, 144, 129,
	29, 145, 144, 41, 106, 10, 107, 43, 184, 131, 1, 36, 61, 13, 138, 2, 194, 1, 16, 27, 75, 186, 181, 151, 8, 1, 161, 138, 211, 129, 2, 59, 248, 129, 16, 0, 144, 63, 152, 150, 136, 24, 25, 128, 30, 161, 128, 17, 24, 225, 146, 10, 16, 0, 9, 227, 183, 129, 40, 60, 26, 162, 194, 181, 24, 90, 9, 24, 0, 176, 161, 193, 194, 35, 12, 63,
	8, 210, 162, 1, 32, 78, 28, 152, 164, 144, 16, 48, 45, 137, 162, 147, 168, 152, 98, 27, 43, 33, 12, 160, 165, 129, 137, 63, 41, 153, 153, 151, 16, 91, 26, 8, 8, 9, 56, 10, 46, 24, 146, 57, 168, 160, 166, 241, 129, 32, 140, 16, 145, 179, 164, 137, 113, 138, 208, 131, 26, 25, 1, 42, 178, 196, 106, 24, 171, 18, 196, 8, 18, 29,
	41, 194, 128, 3, 249, 57, 162, 152, 48, 184, 120, 160, 208, 33, 137, 74, 57, 187, 149, 129, 26, 35, 158, 72, 128, 168, 32, 26, 25, 180, 75, 2, 136, 15, 163, 161, 136, 120, 27, 41, 160, 128, 182, 56, 60, 25, 12, 178, 151, 128, 168, 72, 10, 152, 4, 177, 26, 147, 137, 113, 44, 42, 33, 220, 2, 152, 41, 82, 11, 210, 163, 184,
	133, 162, 10, 196, 128, 3, 234, 40, 149, 152, 161, 1, 44, 129, 194, 4, 225, 16, 58, 168, 24, 194, 146, 146, 154, 49, 21, 218, 33, 152, 248, 129, 194, 147, 0, 28, 1, 195, 162, 20, 140, 42, 25, 160, 198, 1, 33, 136, 142, 3, 25, 24, 141, 16, 177, 208, 112, 0, 138, 41, 160, 130, 45, 60, 32, 170, 73, 24, 75, 59, 161, 176, 49, 159,
	97, 26, 168, 149, 145, 32, 28, 25, 184, 211, 129, 179, 74, 73, 8, 153, 136, 193, 151, 160, 32, 48, 143, 9, 147, 181, 145, 32, 60, 9, 187, 133, 166, 144, 32, 152, 25, 136, 161, 150, 168, 145, 81, 10, 42, 0, 169, 182, 148, 136, 58, 41, 187, 182, 211, 131, 16, 137, 25, 243, 144, 129, 2, 9, 8, 202, 7, 25, 185, 21, 144, 136, 153,
	65, 184, 137, 56, 151, 10, 153, 49, 16, 145, 14, 56, 176, 11, 192, 19, 89, 91, 44, 168, 147, 2, 8, 147, 63, 27, 1, 136, 229, 129, 73, 26, 136, 26, 137, 81, 170, 147, 77, 72, 12, 42, 42, 192, 24, 104, 91, 26, 27, 65, 177, 27, 32, 41, 60, 14, 136, 17, 170, 150, 129, 24, 58, 11, 16, 251, 162, 19, 57, 31, 0, 152, 129, 145, 17, 61,
	14, 1, 129, 27, 129, 66, 169, 178, 74, 12, 11, 19, 198, 145, 75, 33, 138, 174, 133, 1, 184, 57, 40, 136, 169, 20, 1, 60, 174, 20, 154, 201, 67, 26, 162, 151, 42, 16, 138, 59, 130, 204, 20, 169, 59, 180, 59, 114, 184, 56, 178, 242, 128, 130, 43, 8, 194, 3, 229, 144, 33, 185, 144, 34, 181, 145, 168, 17, 149, 153, 74, 35, 220,
	129, 128, 1, 88, 59, 75, 225, 136, 130, 168, 17, 144, 12, 151, 8, 25, 179, 8, 1, 240, 16, 8, 25, 145, 211, 41, 130, 138, 115, 169, 160, 163, 168, 84, 154, 74, 0, 170, 144, 211, 149, 2, 30, 128, 137, 9, 149, 1, 144, 58, 60, 57, 153, 178, 150, 17, 29, 27, 74, 25, 195, 152, 56, 15, 1, 25, 26, 152, 149, 80, 153, 57, 73, 140, 128,
	160, 144, 113, 27, 56, 28, 25, 4, 42, 44, 137, 60, 171, 130, 50, 240, 8, 5, 139, 145, 1, 105, 137, 200, 80, 137, 145, 146, 178, 179, 160, 46, 16, 240, 195, 131, 128, 144, 24, 164, 198, 128, 0, 136, 137, 131, 194, 165, 177, 2, 161, 147, 11, 144, 188, 181, 148, 144, 23, 0, 28, 224, 128, 131, 192, 32, 1, 224, 1, 168, 132, 145,
	9, 41, 208, 58, 137, 179, 151, 145, 16, 1, 30, 8, 145, 178, 1, 47, 32, 186, 72, 169, 146, 75, 8, 41, 48, 136, 89, 13, 48, 9, 10, 124, 26, 11, 42, 32, 129, 91, 77, 16, 12, 128, 42, 57, 138, 10, 60, 2, 63, 9, 0, 93, 128, 152, 90, 8, 10, 24, 40, 44, 144, 29, 49, 188, 48, 72, 25, 30, 177, 33, 128, 186, 120, 129, 186, 133, 152, 130,
	24, 156, 51, 154, 8, 226, 2, 56, 155, 2, 179, 233, 167, 128, 24, 129, 176, 136, 151, 8, 184, 0, 33, 224, 152, 21, 177, 24, 10, 163, 16, 250, 17, 130, 171, 83, 137, 136, 37, 12, 56, 242, 154, 17, 160, 145, 82, 13, 3, 201, 128, 18, 137, 24, 162, 63, 162, 8, 107, 178, 128, 57, 158, 32, 24, 200, 18, 0, 106, 154, 73, 16, 248, 8,
	73, 137, 57, 75, 0, 128, 12, 65, 137, 59, 75, 28, 144, 129, 122, 0, 58, 140, 160, 195, 145, 105, 56, 28, 153, 145, 164, 88, 8, 28, 25, 153, 9, 162, 113, 89, 153, 136, 33, 234, 147, 128, 41, 72, 11, 138, 151, 144, 145, 16, 43, 58, 248, 130, 178, 42, 4, 40, 10, 196, 154, 147, 216, 24, 7, 136, 10, 161, 148, 210, 161, 98, 138,
	137, 128, 146, 176, 33, 105, 27, 43, 163, 49, 185, 6, 10, 136, 43, 67, 174, 161, 162, 151, 137, 1, 64, 200, 193, 24, 64, 200, 56, 145, 242, 24, 57, 137, 1, 128, 3, 162, 175, 80, 128, 162, 152, 25, 58, 175, 17, 17, 0, 200, 64, 168, 162, 91, 1, 154, 44, 211, 177, 35, 64, 160, 161, 144, 4, 241, 41, 209, 162, 25, 1, 3, 242, 176,
	134, 153, 42, 41, 136, 135, 154, 2, 130, 46, 41, 161, 153, 180, 145, 34, 26, 46, 18, 242, 137, 146, 129, 25, 128, 11, 151, 161, 40, 179, 27, 122, 168, 59, 137, 181, 50, 172, 36, 56, 15, 9, 129, 137, 128, 75, 2, 58, 12, 52, 141, 8, 24, 58, 153, 157, 122, 145, 9, 1, 80, 27, 184, 32, 74, 219, 50, 57, 168, 153, 180, 48, 28, 143,
	131, 144, 178, 65, 13, 48, 168, 162, 147, 155, 121, 9, 170, 5, 16, 153, 21, 29, 144, 161, 91, 0, 184, 57, 128, 137, 17, 159, 88, 178, 128, 105, 152, 9, 162, 33, 164, 141, 88, 178, 224, 1, 0, 16, 27, 185, 150, 161, 9, 4, 139, 16, 128, 160, 194, 144, 65, 180, 46, 40, 136, 27, 135, 160, 16, 44, 57, 145, 236, 2, 195, 40, 75, 177,
	2, 200, 179, 146, 186, 104, 50, 141, 24, 169, 165, 148, 11, 97, 10, 11, 130, 177, 49, 57, 78, 42, 154, 128, 165, 59, 33, 28, 30, 1, 136, 16, 192, 41, 128, 152, 123, 136, 24, 1, 169, 113, 10, 11, 49, 153, 14, 147, 19, 45, 43, 8, 176, 210, 148, 8, 16, 11, 96, 144, 192, 163, 150, 10, 128, 43, 26, 150, 178, 165, 24, 41, 171, 18,
	27, 215, 1, 8, 128, 136, 40, 35, 208, 11, 161, 193, 18, 73, 154, 133, 155, 165, 164, 10, 49, 154, 8, 199, 0, 2, 168, 64, 192, 0, 40, 162, 43, 202, 180, 150, 10, 106, 24, 185, 145, 131, 184, 113, 43, 24, 162, 187, 73, 146, 42, 81, 171, 121, 58, 155, 151, 16, 43, 32, 31, 9, 160, 146, 17, 136, 94, 10, 24, 145, 25, 9, 130, 59,
	65, 13, 91, 25, 169, 146, 176, 112, 42, 59, 16, 217, 130, 20, 13, 25, 9, 40, 161, 138, 68, 169, 154, 18, 62, 154, 180, 145, 135, 152, 56, 58, 155, 165, 211, 8, 40, 42, 10, 198, 1, 2, 184, 57, 184, 224, 51, 154, 27, 134, 168, 19, 202, 73, 75, 184, 35, 176, 75, 24, 25, 209, 51, 157, 19, 30, 184, 179, 3, 33, 148, 45, 232, 146,
	129, 168, 41, 32, 170, 149, 193, 35, 136, 16, 50, 191, 56, 146, 173, 149, 16, 24, 41, 30, 129, 168, 209, 3, 57, 31, 0, 16, 176, 147, 41, 152, 10, 17, 181, 14, 40, 144, 49, 170, 75, 97, 141, 25, 162, 146, 72, 177, 92, 137, 137, 19, 137, 153, 113, 154, 2, 41, 60, 129, 217, 2, 211, 152, 73, 42, 193, 197, 146, 147, 10, 59, 0,
	192, 196, 132, 41, 160, 25, 88, 169, 16, 40, 241, 1, 153, 81, 28, 10, 147, 161, 209, 88, 75, 9, 161, 162, 180, 16, 43, 57, 235, 33, 56, 156, 129, 144, 2, 135, 31, 128, 145, 136, 163, 56, 59, 154, 57, 167, 160, 105, 137, 0, 138, 163, 3, 41, 47, 185, 211, 131, 41, 41, 60, 139, 182, 146, 16, 16, 43, 242, 144, 145, 129, 16, 179,
	183, 1, 26, 9, 147, 240, 131, 160, 91, 74, 152, 184, 166, 178, 33, 140, 9, 4, 162, 233, 34, 136, 129, 144, 163, 60, 142, 144, 149, 128, 33, 73, 13, 161, 194, 131, 0, 26, 56, 142, 128, 163, 128, 1, 233, 56, 209, 41, 145, 194, 147, 179, 149, 64, 30, 8, 128, 216, 18, 24, 43, 43, 32, 153, 25, 74, 109, 137, 153, 48, 8, 137, 122,
	25, 144, 26, 43, 59, 30, 33, 41, 27, 24, 96, 153, 160, 50, 76, 27, 47, 152, 145, 163, 73, 40, 14, 152, 131, 176, 74, 90, 8, 8, 200, 67, 155, 154, 50, 49, 155, 28, 124, 177, 152, 1, 2, 17, 62, 138, 180, 176, 4, 25, 9, 177, 245, 162, 129, 40, 25, 176, 164, 130, 172, 4, 8, 181, 194, 49, 11, 168, 154, 165, 133, 152, 40, 136, 226,
	179, 19, 26, 185, 16, 167, 194, 16, 25, 57, 243, 136, 147, 1, 31, 25, 184, 132, 160, 33, 62, 138, 129, 130, 41, 121, 137, 153, 145, 26, 17, 107, 136, 179, 1, 61, 60, 26, 162, 168, 148, 64, 31, 25, 32, 168, 152, 64, 31, 137, 8, 129, 33, 62, 24, 137, 8, 16, 59, 47, 153, 33, 162, 91, 59, 41, 170, 145, 5, 43, 60, 41, 13, 178, 134,
	57, 153, 12, 194, 227, 8, 2, 128, 57, 208, 162, 19, 216, 32, 178, 25, 128, 160, 48, 194, 195, 37, 155, 10, 33, 251, 163, 146, 16, 136, 12, 166, 195, 160, 148, 129, 176, 147, 178, 150, 160, 72, 162, 162, 193, 162, 60, 200, 145, 5, 144, 25, 122, 216, 129, 161, 130, 0, 10, 73, 1, 241, 2, 9, 168, 33, 13, 161, 165, 24, 64, 203,
	50, 1, 14, 9, 9, 129, 161, 106, 33, 27, 13, 164, 128, 40, 41, 107, 169, 160, 33, 136, 60, 92, 168, 152, 2, 91, 57, 176, 129, 0, 144, 47, 136, 162, 164, 128, 80, 43, 154, 179, 213, 130, 74, 27, 0, 145, 145, 167, 58, 59, 160, 9, 26, 76, 8, 171, 5, 49, 28, 44, 169, 162, 183, 130, 72, 28, 144, 179, 228, 2, 25, 26, 129, 186, 151,
	1, 75, 128, 169, 17, 178, 15, 57, 170, 16, 166, 16, 57, 8, 139, 162, 181, 1, 8, 152, 164, 181, 41, 81, 43, 10, 242, 145, 57, 139, 89, 8, 193, 18, 154, 32, 176, 10, 165, 129, 137, 147, 177, 134, 0, 25, 25, 201, 147, 227, 129, 72, 59, 185, 167, 128, 129, 160, 91, 25, 176, 130, 147, 145, 9, 160, 5, 202, 17, 16, 186, 136, 37,
	177, 56, 76, 42, 169, 186, 48, 9, 145, 57, 24, 128, 41, 169, 134, 137, 145, 147, 28, 41, 168, 131, 228, 32, 27, 9, 60, 129, 178, 64, 60, 45, 25, 9, 24, 152, 49, 31, 136, 57, 42, 0, 25, 12, 181, 18, 153, 57, 96, 169, 177, 132, 153, 123, 9, 152, 129, 177, 17, 74, 43, 24, 169, 128, 121, 137, 25, 1, 139, 96, 42, 10, 146, 178, 18,
	44, 29, 1, 161, 164, 146, 31, 137, 146, 177, 19, 1, 10, 26, 209, 165, 146, 43, 40, 138, 240, 130, 18, 144, 25, 40, 212, 1, 58, 11, 152, 196, 147, 10, 74, 26, 152, 225, 130, 146, 58, 60, 210, 145, 16, 148, 16, 185, 192, 18, 44, 42, 57, 199, 162, 1, 9, 87, 47, 186, 215, 231, 197, 179, 180, 195, 212, 164, 32, 59, 92, 126, 62,
	41, 59, 76, 59, 60, 168, 179, 213, 197, 163, 72, 44, 25, 74, 126, 127, 127, 79, 26, 177, 148, 90, 27, 225, 247, 165, 0, 152, 147, 123, 138, 211, 164, 72, 126, 127, 46, 210, 196, 163, 228, 215, 64, 11, 210, 180, 1, 8, 58, 153, 1, 224, 149, 57, 76, 27, 24, 76, 42, 43, 136, 128, 243, 179, 130, 106, 60, 42, 42, 92, 28, 243, 231,
	147, 24, 57, 44, 58, 94, 45, 8, 57, 139, 214, 148, 40, 77, 26, 9, 16, 10, 144, 64, 62, 43, 25, 123, 59, 138, 162, 48, 63, 26, 41, 92, 60, 43, 176, 3, 59, 232, 214, 164, 16, 75, 75, 76, 60, 153, 179, 33, 62, 26, 136, 40, 75, 169, 197, 163, 129, 57, 60, 59, 75, 138, 145, 64, 63, 138, 179, 1, 42, 136, 90, 43, 176, 214, 180, 1, 25,
	152, 195, 129, 129, 106, 76, 60, 137, 145, 178, 2, 25, 10, 228, 130, 57, 59, 44, 41, 154, 165, 105, 76, 44, 144, 16, 76, 26, 41, 76, 26, 152, 1, 58, 26, 9, 193, 165, 16, 92, 26, 41, 77, 59, 76, 76, 60, 26, 136, 161, 130, 152, 195, 163, 211, 146, 0, 57, 11, 211, 130, 8, 25, 40, 62, 153, 162, 17, 109, 60, 153, 146, 40, 76, 60,
	26, 160, 179, 211, 163, 32, 60, 42, 153, 179, 194, 199, 130, 24, 58, 43, 58, 27, 128, 161, 195, 129, 226, 196, 147, 90, 59, 75, 44, 136, 128, 145, 160, 148, 123, 59, 42, 26, 41, 26, 57, 27, 192, 215, 147, 57, 59, 27, 161, 145, 213, 130, 106, 76, 43, 9, 144, 162, 129, 177, 181, 130, 136, 194, 146, 40, 10, 129, 25, 210, 146,
	178, 197, 196, 179, 196, 130, 8, 41, 9, 144, 178, 130, 209, 182, 17, 92, 43, 176, 147, 144, 212, 130, 136, 0, 177, 130, 73, 62, 10, 161, 130, 91, 75, 59, 43, 57, 46, 25, 41, 77, 10, 177, 164, 16, 26, 136, 210, 197, 179, 130, 128, 57, 77, 43, 25, 75, 10, 227, 179, 180, 179, 146, 128, 57, 185, 183, 163, 145, 0, 8, 8, 10, 119,
	114, 120, 16, 210, 244, 60, 28, 41, 25, 152, 149, 56, 161, 35, 44, 89, 27, 24, 136, 24, 164, 211, 17, 233, 176, 136, 192, 129, 179, 17, 17, 25, 0, 10, 46, 160, 132, 49, 66, 24, 132, 177, 147, 193, 56, 72, 26, 29, 232, 168, 176, 12, 137, 41, 139, 147, 9, 1, 41, 15, 91, 136, 35, 148, 21, 18, 48, 40, 1, 168, 167, 144, 0, 42, 172,
	177, 204, 193, 155, 232, 152, 152, 26, 152, 41, 146, 17, 6, 4, 65, 34, 35, 135, 4, 16, 32, 9, 24, 186, 176, 0, 250, 153, 204, 186, 173, 154, 153, 177, 3, 65, 41, 34, 145, 134, 35, 65, 98, 49, 50, 50, 2, 33, 169, 138, 155, 175, 170, 172, 204, 192, 138, 234, 136, 155, 136, 10, 32, 18, 5, 52, 48, 24, 162, 17, 67, 54, 66, 51, 34,
	131, 184, 174, 234, 153, 10, 9, 40, 0, 152, 251, 168, 142, 154, 9, 16, 33, 49, 33, 128, 154, 170, 156, 34, 54, 54, 33, 68, 0, 1, 136, 201, 137, 26, 88, 48, 35, 99, 8, 152, 189, 189, 187, 155, 171, 16, 24, 130, 145, 188, 175, 203, 144, 49, 115, 67, 67, 50, 19, 2, 1, 0, 0, 130, 131, 1, 136, 206, 216, 188, 203, 204, 187, 187,
	156, 153, 0, 0, 51, 17, 34, 24, 112, 20, 69, 67, 67, 34, 19, 0, 136, 169, 185, 137, 186, 232, 185, 219, 201, 203, 187, 173, 170, 154, 153, 129, 131, 6, 2, 19, 49, 49, 21, 65, 19, 53, 51, 83, 34, 16, 168, 201, 154, 172, 156, 138, 0, 1, 24, 201, 233, 186, 204, 186, 171, 137, 3, 37, 48, 24, 128, 201, 202, 202, 129, 17, 48, 21,
	22, 20, 19, 19, 32, 16, 2, 66, 52, 68, 4, 3, 1, 203, 235, 188, 189, 186, 171, 153, 137, 153, 170, 219, 170, 140, 9, 17, 53, 115, 50, 52, 67, 51, 51, 51, 17, 130, 0, 145, 154, 169, 188, 236, 187, 190, 203, 187, 172, 171, 138, 136, 17, 33, 18, 2, 34, 98, 98, 50, 50, 52, 66, 34, 35, 2, 19, 24, 169, 203, 203, 188, 219, 169, 154,
	9, 137, 171, 204, 188, 203, 184, 136, 34, 83, 50, 33, 153, 184, 170, 170, 152, 40, 57, 19, 36, 50, 50, 18, 35, 17, 2, 49, 49, 66, 66, 66, 34, 17, 168, 233, 202, 202, 170, 171, 170, 186, 219, 203, 188, 188, 154, 138, 25, 33, 68, 52, 68, 67, 67, 36, 51, 36, 18, 17, 17, 136, 8, 170, 176, 202, 188, 206, 202, 171, 172, 186, 169,
	153, 8, 25, 144, 128, 1, 34, 68, 52, 68, 51, 52, 34, 49, 18, 34, 2, 144, 136, 155, 140, 187, 186, 186, 154, 154, 185, 185, 153, 9, 9, 0, 24, 0, 128, 144, 168, 169, 170, 154, 154, 153, 9, 8, 16, 8, 0, 144, 19, 35, 68, 51, 52, 67, 51, 66, 34, 50, 33, 1, 144, 185, 186, 172, 204, 187, 188, 173, 172, 186, 172, 186, 154, 138, 41,
	33, 52, 53, 83, 50, 51, 52, 52, 37, 34, 34, 18, 16, 144, 152, 154, 187, 219, 203, 188, 173, 186, 186, 186, 170, 154, 153, 138, 144, 16, 17, 67, 82, 50, 51, 21, 34, 19, 33, 2, 18, 33, 1, 8, 153, 169, 153, 153, 136, 128, 0, 136, 154, 153, 153, 8, 8, 1, 16, 0, 169, 170, 187, 171, 171, 154, 153, 153, 152, 153, 153, 0, 16, 51, 83,
	66, 50, 67, 50, 51, 67, 51, 52, 35, 18, 136, 186, 219, 187, 189, 186, 171, 187, 173, 187, 188, 187, 203, 138, 9, 16, 33, 50, 52, 53, 67, 67, 147, 8, 128, 128, 128, 128, 128, 128, 128, 128, 0, 240, 255, 55, 232, 23, 220, 0, 148, 1, 9, 18, 148, 10, 189, 32, 163, 62, 160, 5, 137, 12, 149, 42, 153, 144, 34, 42, 8, 1, 138, 181,
	45, 136, 18, 144, 105, 138, 1, 160, 14, 128, 132, 145, 186, 37, 138, 41, 192, 48, 145, 46, 160, 33, 44, 24, 225, 16, 13, 132, 136, 137, 16, 148, 25, 170, 194, 82, 152, 136, 91, 24, 42, 169, 33, 233, 131, 179, 24, 185, 149, 16, 57, 172, 164, 18, 10, 211, 160, 147, 211, 33, 138, 243, 129, 16, 41, 193, 0, 43, 132, 155, 73,
	58, 145, 244, 145, 43, 35, 9, 171, 16, 110, 25, 8, 28, 74, 162, 128, 26, 27, 82, 45, 136, 153, 18, 8, 136, 8
};
#endif // DISABLE_PC98_RHYTHM_CHANNEL

TownsPC98_FmSynth::ChanInternal::ChanInternal() {
	memset(this, 0, sizeof(ChanInternal));
}

TownsPC98_FmSynth::ChanInternal::~ChanInternal() {
	for (uint i = 0; i < ARRAYSIZE(opr); ++i)
		delete opr[i];
}
