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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "audio/softsynth/fmtowns_pc98/towns_midi.h"
#include "audio/softsynth/fmtowns_pc98/towns_pc98_fmsynth.h"
#include "common/textconsole.h"
#include "common/system.h"

class TownsMidiOutputChannel {
friend class TownsMidiInputChannel;
public:
	TownsMidiOutputChannel(MidiDriver_TOWNS *driver, int chanId);
	~TownsMidiOutputChannel();

	void noteOn(uint8 msb, uint16 lsb);
	void noteOnPitchBend(uint8 msb, uint16 lsb);
	void setupProgram(const uint8 *data, uint8 mLevelPara, uint8 tLevelPara);
	void setupEffects(int index, uint8 flags, const uint8 *effectData);
	void setModWheel(uint8 value);

	void connect(TownsMidiInputChannel *chan);
	void disconnect();

	bool update();

	enum CheckPriorityStatus {
		kDisconnected = -2,
		kHighPriority = -1
	};

	int checkPriority(int pri);

private:
	struct EffectEnvelope {
		uint8 state;
		int32 currentLevel;
		int32 duration;
		int32 maxLevel;
		int32 startLevel;
		uint8 loop;
		uint8 stateTargetLevels[4];
		uint8 stateModWheelLevels[4];
		int8 modWheelSensitivity;
		int8 modWheelState;
		int8 modWheelLast;
		uint16 numSteps;
		uint32 stepCounter;
		int32 incrPerStep;
		int8 dir;
		uint32 incrPerStepRem;
		uint32 incrCountRem;
	} *_effectEnvelopes;

	struct EffectDef {
		int32 phase;
		uint8 type;
		uint8 useModWheel;
		uint8 loopRefresh;
		EffectEnvelope *s;
	} *_effectDefs;

	void startEffect(EffectEnvelope *s, const uint8 *effectData);
	void updateEffectGenerator(EffectEnvelope *s, EffectDef *d);
	int advanceEffectEnvelope(EffectEnvelope *s, EffectDef *d);
	void initNextEnvelopeState(EffectEnvelope *s);
	int16 getEffectStartLevel(uint8 type);
	int getEffectModLevel(int lvl, int mod);

	void keyOn();
	void keyOff();
	void keyOnSetFreq(uint16 frq);
	void out(uint8 reg, uint8 val);

	TownsMidiInputChannel *_in;
	TownsMidiOutputChannel *_prev;
	TownsMidiOutputChannel *_next;
	uint8 _adjustModTl;
	uint8 _chan;
	uint8 _note;
	uint8 _operator2Tl;
	uint8 _operator1Tl;
	uint8 _sustainNoteOff;
	int16 _duration;

	uint16 _freq;
	int16 _freqAdjust;

	MidiDriver_TOWNS *_driver;

	static const uint8 _chanMap[];
	static const uint8 _chanMap2[];
	static const uint8 _effectDefaults[];
	static const uint16 _effectEnvStepTable[];
	static const uint8 _freqMSB[];
	static const uint16 _freqLSB[];
};

class TownsMidiInputChannel : public MidiChannel {
friend class TownsMidiOutputChannel;
public:
	TownsMidiInputChannel(MidiDriver_TOWNS *driver, int chanIndex);
	~TownsMidiInputChannel();

	MidiDriver *device() { return _driver; }
	byte getNumber() { return _chanIndex; }
	bool allocate();
	void release();

	void send(uint32 b);

	void noteOff(byte note);
	void noteOn(byte note, byte velocity);
	void programChange(byte program);
	void pitchBend(int16 bend);
	void controlChange(byte control, byte value);
	void pitchBendFactor(byte value);
	void priority(byte value);
	void sysEx_customInstrument(uint32 type, const byte *instr);

private:
	void controlModulationWheel(byte value);
	void controlVolume(byte value);
	void controlPanPos(byte value);
	void controlSustain(byte value);

	void releasePedal();

	TownsMidiOutputChannel *_out;

	uint8 *_instrument;
	uint8 _chanIndex;
	uint8 _priority;
	uint8 _tl;
	int8 _transpose;
	int8 _detune;
	int8 _modWheel;
	uint8 _sustain;
	uint8 _pitchBendFactor;
	int16 _pitchBend;
	uint16 _freqLSB;

	bool _allocated;

	MidiDriver_TOWNS *_driver;

	static const uint8 _programAdjustLevel[];
};

class TownsMidiChanState {
public:
	TownsMidiChanState();
	~TownsMidiChanState() {}
	uint8 get(uint8 type);

	uint8 unk1;
	uint8 mulAmsFms;
	uint8 tl;
	uint8 attDec;
	uint8 sus;
	uint8 fgAlg;
	uint8 unk2;
};

TownsMidiChanState::TownsMidiChanState() {
	unk1 = mulAmsFms = tl =	attDec = sus = fgAlg = unk2 = 0;
}

uint8 TownsMidiChanState::get(uint8 type) {
	switch (type) {
	case 0:
		return unk1;
	case 1:
		return mulAmsFms;
	case 2:
		return tl;
	case 3:
		return attDec;
	case 4:
		return sus;
	case 5:
		return fgAlg;
	case 6:
		return unk2;
	default:
		break;
	}
	return 0;
}

TownsMidiOutputChannel::TownsMidiOutputChannel(MidiDriver_TOWNS *driver, int chanIndex) : _driver(driver), _chan(chanIndex),
	_in(0), _prev(0), _next(0), _adjustModTl(0), _operator2Tl(0), _note(0), _operator1Tl(0), _sustainNoteOff(0), _duration(0), _freq(0), _freqAdjust(0) {
	_effectEnvelopes = new EffectEnvelope[2];
	_effectDefs = new EffectDef[2];

	memset(_effectEnvelopes, 0, 2 * sizeof(EffectEnvelope));
	memset(_effectDefs, 0, 2 * sizeof(EffectDef));
	_effectDefs[0].s = &_effectEnvelopes[1];
	_effectDefs[1].s = &_effectEnvelopes[0];
}

TownsMidiOutputChannel::~TownsMidiOutputChannel() {
	delete[] _effectEnvelopes;
	delete[] _effectDefs;
}

void TownsMidiOutputChannel::noteOn(uint8 msb, uint16 lsb) {
	_freq = (msb << 7) + lsb;
	_freqAdjust = 0;
	keyOnSetFreq(_freq);
}

void TownsMidiOutputChannel::noteOnPitchBend(uint8 msb, uint16 lsb) {
	_freq = (msb << 7) + lsb;
	keyOnSetFreq(_freq + _freqAdjust);
}

void TownsMidiOutputChannel::setupProgram(const uint8 *data, uint8 mLevelPara, uint8 tLevelPara) {
	// This driver uses only 2 operators and 2 algorithms (algorithm 5 and 7),
	// since it is just a modified AdLib driver. It also uses AdLib programs.
	// There are no FM-TOWNS specific programs. This is the reason for the low quality of the FM-TOWNS
	// music (unsuitable data is just forced into the wrong audio device).

	static const uint8 mul[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15 };
	uint8 chan = _chanMap[_chan];

	uint8 mulAmsFms1 = _driver->_chanState[chan].mulAmsFms = data[0];
	uint8 tl1 = _driver->_chanState[chan].tl = (data[1] | 0x3f) - mLevelPara;
	uint8 attDec1 = _driver->_chanState[chan].attDec = ~data[2];
	uint8 sus1 = _driver->_chanState[chan].sus = ~data[3];
	_driver->_chanState[chan].unk2 = data[4];
	chan += 3;

	out(0x30, mul[mulAmsFms1 & 0x0f]);
	out(0x40, (tl1 & 0x3f) + 15);
	out(0x50, ((attDec1 >> 4) << 1) | ((attDec1 >> 4) & 1));
	out(0x60, ((attDec1 << 1) | (attDec1 & 1)) & 0x1f);
	out(0x70, (mulAmsFms1 & 0x20) ^ 0x20 ? (((sus1 & 0x0f) << 1) | 1) : 0);
	out(0x80, sus1);

	uint8 mulAmsFms2 = _driver->_chanState[chan].mulAmsFms = data[5];
	uint8 tl2 = _driver->_chanState[chan].tl = (data[6] | 0x3f) - tLevelPara;
	uint8 attDec2 = _driver->_chanState[chan].attDec = ~data[7];
	uint8 sus2 = _driver->_chanState[chan].sus = ~data[8];
	_driver->_chanState[chan].unk2 = data[9];

	uint8 mul2 = mul[mulAmsFms2 & 0x0f];
	tl2 = (tl2 & 0x3f) + 15;
	uint8 ar2 = ((attDec2 >> 4) << 1) | ((attDec2 >> 4) & 1);
	uint8 dec2 = ((attDec2 << 1) | (attDec2 & 1)) & 0x1f;
	uint8 sus2r = (mulAmsFms2 & 0x20) ^ 0x20 ? (((sus2 & 0x0f) << 1) | 1) : 0;

	for (int i = 4; i < 16; i += 4) {
		out(0x30 + i, mul2);
		out(0x40 + i, tl2);
		out(0x50 + i, ar2);
		out(0x60 + i, dec2);
		out(0x70 + i, sus2r);
		out(0x80 + i, sus2);
	}

	_driver->_chanState[chan].fgAlg = data[10];

	uint8 alg = 5 + 2 * (data[10] & 1);
	uint8 fb = 4 * (data[10] & 0x0e);
	out(0xb0, fb | alg);
	uint8 t = mulAmsFms1 | mulAmsFms2;
	out(0xb4, (0xc0 | ((t & 0x80) >> 3) | ((t & 0x40) >> 5)));
}

void TownsMidiOutputChannel::setupEffects(int index, uint8 flags, const uint8 *effectData) {
	uint16 effectMaxLevel[] = { 0x2FF, 0x1F, 0x07, 0x3F, 0x0F, 0x0F, 0x0F, 0x03, 0x3F, 0x0F, 0x0F, 0x0F, 0x03, 0x3E, 0x1F };
	uint8 effectType[] = { 0x1D, 0x1C, 0x1B, 0x00, 0x03, 0x04, 0x07, 0x08, 0x0D, 0x10, 0x11, 0x14, 0x15, 0x1e, 0x1f, 0x00 };

	EffectEnvelope *s = &_effectEnvelopes[index];
	EffectDef *d = &_effectDefs[index];

	d->phase = 0;
	d->useModWheel = flags & 0x40;
	s->loop = flags & 0x20;
	d->loopRefresh = flags & 0x10;
	d->type = effectType[flags & 0x0f];
	s->maxLevel = effectMaxLevel[flags & 0x0f];
	s->modWheelSensitivity = 31;
	s->modWheelState = d->useModWheel ? _in->_modWheel >> 2 : 31;

	switch (d->type) {
	case 0:
		s->startLevel = _operator2Tl;
		break;
	case 13:
		s->startLevel = _operator1Tl;
		break;
	case 30:
		s->startLevel = 31;
		d->s->modWheelState = 0;
		break;
	case 31:
		s->startLevel = 0;
		d->s->modWheelSensitivity = 0;
		break;
	default:
		s->startLevel = getEffectStartLevel(d->type);
		break;
	}

	startEffect(s, effectData);
}

void TownsMidiOutputChannel::setModWheel(uint8 value) {
	if (_effectEnvelopes[0].state != kEnvReady && _effectDefs[0].type)
		_effectEnvelopes[0].modWheelState = value >> 2;

	if (_effectEnvelopes[1].state != kEnvReady && _effectDefs[1].type)
		_effectEnvelopes[1].modWheelState = value >> 2;
}

void TownsMidiOutputChannel::connect(TownsMidiInputChannel *chan) {
	if (!chan)
		return;

	_in = chan;
	_next = chan->_out;
	_prev = 0;
	chan->_out = this;
	if (_next)
		_next->_prev = this;
}

void TownsMidiOutputChannel::disconnect() {
	keyOff();

	TownsMidiOutputChannel *p = _prev;
	TownsMidiOutputChannel *n = _next;

	if (n)
		n->_prev = p;
	if (p)
		p->_next = n;
	else
		_in->_out = n;
	_in = 0;
}

bool TownsMidiOutputChannel::update() {
	if (!_in)
		return false;

	if (_duration) {
		_duration -= 17;
		if (_duration <= 0) {
			disconnect();
			return true;
		}
	}

	for (int i = 0; i < 2; i++) {
		if (_effectEnvelopes[i].state != kEnvReady)
			updateEffectGenerator(&_effectEnvelopes[i], &_effectDefs[i]);
	}

	return false;
}

int TownsMidiOutputChannel::checkPriority(int pri) {
	if (!_in)
		return kDisconnected;

	if (!_next && pri >= _in->_priority)
		return _in->_priority;

	return kHighPriority;
}

void TownsMidiOutputChannel::startEffect(EffectEnvelope *s, const uint8 *effectData) {
	s->state = kEnvAttacking;
	s->currentLevel = 0;
	s->modWheelLast = 31;
	s->duration = effectData[0] * 63;
	s->stateTargetLevels[0] = effectData[1];
	s->stateTargetLevels[1] = effectData[3];
	s->stateTargetLevels[2] = effectData[5];
	s->stateTargetLevels[3] = effectData[6];
	s->stateModWheelLevels[0] = effectData[2];
	s->stateModWheelLevels[1] = effectData[4];
	s->stateModWheelLevels[2] = 0;
	s->stateModWheelLevels[3] = effectData[7];
	initNextEnvelopeState(s);
}

void TownsMidiOutputChannel::updateEffectGenerator(EffectEnvelope *s, EffectDef *d) {
	uint8 f = advanceEffectEnvelope(s, d);

	if (f & 1) {
		switch (d->type) {
		case 0:
			_operator2Tl = s->startLevel + d->phase;
			break;
		case 13:
			_operator1Tl = s->startLevel + d->phase;
			break;
		case 30:
			d->s->modWheelState = d->phase;
			break;
		case 31:
			d->s->modWheelSensitivity = d->phase;
			break;
		default:
			break;
		}
	}

	if (f & 2) {
		if (d->loopRefresh)
			keyOn();
	}
}

int TownsMidiOutputChannel::advanceEffectEnvelope(EffectEnvelope *s, EffectDef *d) {
	if (s->duration) {
		s->duration -= 17;
		if (s->duration <= 0) {
			s->state = kEnvReady;
			return 0;
		}
	}

	int32 t = s->currentLevel + s->incrPerStep;

	s->incrCountRem += s->incrPerStepRem;
	if (s->incrCountRem >= s->numSteps) {
		s->incrCountRem -= s->numSteps;
		t += s->dir;
	}

	int retFlags = 0;

	if (t != s->currentLevel || (s->modWheelState != s->modWheelLast)) {
		s->currentLevel = t;
		s->modWheelLast = s->modWheelState;
		t = getEffectModLevel(t, s->modWheelState);
		if (t != d->phase) {
			d->phase = t;
			retFlags |= 1;
		}
	}

	if (--s->stepCounter)
		return retFlags;

	if (++s->state > kEnvReleasing) {
		if (!s->loop) {
			s->state = kEnvReady;
			return retFlags;
		}
		s->state = kEnvAttacking;
		retFlags |= 2;
	}

	initNextEnvelopeState(s);

	return retFlags;
}

void TownsMidiOutputChannel::initNextEnvelopeState(EffectEnvelope *s) {
	uint8 v = s->stateTargetLevels[s->state - 1];
	int32 e = _effectEnvStepTable[_driver->_operatorLevelTable[((v & 0x7f) << 5) + s->modWheelSensitivity]];

	if (v & 0x80)
		e = _driver->randomValue(e);

	if (!e)
		e = 1;

	s->numSteps = s->stepCounter = e;
	int32 d = 0;

	if (s->state != kEnvSustaining) {
		v = s->stateModWheelLevels[s->state - 1];
		e = getEffectModLevel(s->maxLevel, (v & 0x7f) - 31);

		if (v & 0x80)
			e = _driver->randomValue(e);

		if (e + s->startLevel > s->maxLevel) {
			e = s->maxLevel - s->startLevel;
		} else {
			if (e + s->startLevel < 0)
				e = -s->startLevel;
		}

		d = e - s->currentLevel;
	}

	s->incrPerStep = d / s->numSteps;
	s->dir = (d < 0) ? -1 : 1;
	d *= s->dir;
	s->incrPerStepRem = d % s->numSteps;
	s->incrCountRem = 0;
}

int16 TownsMidiOutputChannel::getEffectStartLevel(uint8 type) {
	uint8 chan = (type < 13) ? _chanMap2[_chan] : ((type < 26) ? _chanMap[_chan] : _chan);

	if (type == 28)
		return 15;
	else if (type == 29)
		return 383;
	else if (type > 29)
		return 0;
	else if (type > 12)
		type -= 13;

	const uint8 *def = &_effectDefaults[type << 2];
	uint8 res = (_driver->_chanState[chan].get(def[0] >> 5) & def[2]) >> def[1];
	if (def[3])
		res = def[3] - res;

	return res;
}

int TownsMidiOutputChannel::getEffectModLevel(int lvl, int mod) {
	if (mod == 0)
		return 0;

	if (mod == 31)
		return lvl;

	if (lvl > 63 || lvl < -63)
		return ((lvl + 1) * mod) >> 5;

	if (mod < 0) {
		if (lvl < 0)
			return _driver->_operatorLevelTable[((-lvl) << 5) - mod];
		else
			return -_driver->_operatorLevelTable[(lvl << 5) - mod];
	} else {
		if (lvl < 0)
			return -_driver->_operatorLevelTable[((-lvl) << 5) + mod];
		else
			return _driver->_operatorLevelTable[(lvl << 5) + mod];
	}

	return 0;
}

void TownsMidiOutputChannel::keyOn() {
	out(0x28, 0x30);
}

void TownsMidiOutputChannel::keyOff() {
	out(0x28, 0);
}

void TownsMidiOutputChannel::keyOnSetFreq(uint16 frq) {
	uint16 note = (frq << 1) >> 8;
	frq = (_freqMSB[note] << 11) | _freqLSB[note] ;
	out(0xa4, frq >> 8);
	out(0xa0, frq & 0xff);
	//out(0x28, 0x00);
	out(0x28, 0x30);
}

void TownsMidiOutputChannel::out(uint8 reg, uint8 val) {
	static const uint8 chanRegOffs[] = { 0, 1, 2, 0, 1, 2 };
	static const uint8 keyValOffs[] = { 0, 1, 2, 4, 5, 6 };

	if (reg == 0x28)
		val = (val & 0xf0) | keyValOffs[_chan];
	if (reg < 0x30)
		_driver->_intf->callback(17, 0, reg, val);
	else
		_driver->_intf->callback(17, _chan / 3, (reg & ~3) | chanRegOffs[_chan], val);
}

const uint8 TownsMidiOutputChannel::_chanMap[] = {
	0, 1, 2, 8, 9, 10
};

const uint8 TownsMidiOutputChannel::_chanMap2[] = {
	3, 4, 5, 11, 12, 13
};

const uint8 TownsMidiOutputChannel::_effectDefaults[] = {
	0x40, 0x00, 0x3F, 0x3F, 0xE0, 0x02, 0x00, 0x00, 0x40, 0x06, 0xC0, 0x00,
	0x20, 0x00, 0x0F, 0x00, 0x60, 0x04, 0xF0, 0x0F, 0x60, 0x00, 0x0F, 0x0F,
	0x80, 0x04, 0xF0, 0x0F, 0x80, 0x00, 0x0F, 0x0F, 0xE0, 0x00, 0x03, 0x00,
	0x20, 0x07, 0x80, 0x00, 0x20, 0x06, 0x40, 0x00, 0x20, 0x05, 0x20, 0x00,
	0x20, 0x04, 0x10, 0x00, 0xC0, 0x00, 0x01, 0x00, 0xC0, 0x01, 0x0E, 0x00
};

const uint16 TownsMidiOutputChannel::_effectEnvStepTable[] = {
	0x0001, 0x0002, 0x0004, 0x0005, 0x0006, 0x0007,	0x0008, 0x0009,
	0x000A, 0x000C, 0x000E, 0x0010,	0x0012, 0x0015, 0x0018, 0x001E,
	0x0024, 0x0032,	0x0040, 0x0052, 0x0064, 0x0088, 0x00A0, 0x00C0,
	0x00F0, 0x0114, 0x0154, 0x01CC, 0x0258, 0x035C,	0x04B0, 0x0640
};

const uint8 TownsMidiOutputChannel::_freqMSB[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
	0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x80, 0x81, 0x83, 0x85,
	0x87, 0x88, 0x8A, 0x8C, 0x8E, 0x8F, 0x91, 0x93, 0x95, 0x96, 0x98, 0x9A,
	0x9C, 0x9E, 0x9F, 0xA1, 0xA3, 0xA5, 0xA6, 0xA8, 0xAA, 0xAC, 0xAD, 0xAF,
	0xB1, 0xB3, 0xB4, 0xB6, 0xB8, 0xBA, 0xBC, 0xBD, 0xBF, 0xC1, 0xC3, 0xC4,
	0xC6, 0xC8, 0xCA, 0xCB, 0xCD, 0xCF, 0xD1, 0xD2, 0xD4, 0xD6, 0xD8, 0xDA,
	0xDB, 0xDD, 0xDF, 0xE1, 0xE2, 0xE4, 0xE6, 0xE8, 0xE9, 0xEB, 0xED, 0xEF
};

const uint16 TownsMidiOutputChannel::_freqLSB[] = {
	0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6,
	0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x02D6, 0x0301, 0x032F,
	0x0360, 0x0393, 0x03C9, 0x0403, 0x0440, 0x0481, 0x04C6, 0x050E,
	0x055B, 0x02D6, 0x0301, 0x032F, 0x0360, 0x0393, 0x03C9, 0x0403,
	0x0440, 0x0481, 0x04C6, 0x050E, 0x055B, 0x02D6, 0x0301, 0x032F,
	0x0360, 0x0393, 0x03C9, 0x0403, 0x0440, 0x0481, 0x04C6, 0x050E,
	0x055B, 0x02D6, 0x0301, 0x032F, 0x0360, 0x0393, 0x03C9, 0x0403,
	0x0440, 0x0481, 0x04C6, 0x050E, 0x055B, 0x02D6, 0x0301, 0x032F,
	0x0360, 0x0393, 0x03C9, 0x0403, 0x0440, 0x0481, 0x04C6, 0x050E,
	0x055B, 0x02D6, 0x0301, 0x032F, 0x0360, 0x0393, 0x03C9, 0x0403,
	0x0440, 0x0481, 0x04C6, 0x050E, 0x055B, 0x02D6, 0x0301, 0x032F,
	0x0360, 0x0393, 0x03C9, 0x0403, 0x0440, 0x0481, 0x04C6, 0x050E,
	0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B,
	0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B,
	0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B,
	0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B, 0x055B
};

TownsMidiInputChannel::TownsMidiInputChannel(MidiDriver_TOWNS *driver, int chanIndex) : MidiChannel(), _driver(driver), _out(0), _chanIndex(chanIndex),
	_priority(0), _tl(0), _transpose(0), _pitchBendFactor(0), _pitchBend(0), _sustain(0), _freqLSB(0), _detune(0), _modWheel(0), _allocated(false) {
	_instrument = new uint8[30];
	memset(_instrument, 0, 30);
}

TownsMidiInputChannel::~TownsMidiInputChannel() {
	delete[] _instrument;
}

bool TownsMidiInputChannel::allocate() {
	if (_allocated)
		return false;
	_allocated = true;
	return true;
}

void TownsMidiInputChannel::release() {
	_allocated = false;
}

void TownsMidiInputChannel::send(uint32 b) {
	_driver->send(b | _chanIndex);
}

void TownsMidiInputChannel::noteOff(byte note) {
	if (!_out)
		return;

	for (TownsMidiOutputChannel *oc = _out; oc; oc = oc->_next) {
		if (oc->_note != note)
			continue;

		if (_sustain)
			oc->_sustainNoteOff = 1;
		else
			oc->disconnect();
	}
}

void TownsMidiInputChannel::noteOn(byte note, byte velocity) {
	TownsMidiOutputChannel *oc = _driver->allocateOutputChannel(_priority);

	if (!oc)
		return;

	oc->connect(this);

	oc->_adjustModTl = _instrument[10] & 1;
	oc->_note = note;
	oc->_sustainNoteOff = 0;
	oc->_duration = _instrument[29] * 63;

	oc->_operator1Tl = (_instrument[1] & 0x3f) + _driver->_operatorLevelTable[((velocity >> 1) << 5) + (_instrument[4] >> 2)];
	if (oc->_operator1Tl > 63)
		oc->_operator1Tl = 63;

	oc->_operator2Tl = (_instrument[6] & 0x3f) + _driver->_operatorLevelTable[((velocity >> 1) << 5) + (_instrument[9] >> 2)];
	if (oc->_operator2Tl > 63)
		oc->_operator2Tl = 63;

	oc->setupProgram(_instrument, oc->_adjustModTl == 1 ? _programAdjustLevel[_driver->_operatorLevelTable[(_tl >> 2) + (oc->_operator1Tl << 5)]] : oc->_operator1Tl, _programAdjustLevel[_driver->_operatorLevelTable[(_tl >> 2) + (oc->_operator2Tl << 5)]]);
	oc->noteOn(note + _transpose, _freqLSB);

	if (_instrument[11] & 0x80)
		oc->setupEffects(0, _instrument[11], &_instrument[12]);
	else
		oc->_effectEnvelopes[0].state = kEnvReady;

	if (_instrument[20] & 0x80)
		oc->setupEffects(1, _instrument[20], &_instrument[21]);
	else
		oc->_effectEnvelopes[1].state = kEnvReady;
}

void TownsMidiInputChannel::programChange(byte program) {
	// Not implemented (The loading and assignment of programs
	// is handled externally by the SCUMM engine. The programs
	// get sent via sysEx_customInstrument.)
}

void TownsMidiInputChannel::pitchBend(int16 bend) {
	_pitchBend = bend;
	_freqLSB = ((_pitchBend * _pitchBendFactor) >> 6) + _detune;
	for (TownsMidiOutputChannel *oc = _out; oc; oc = oc->_next)
		oc->noteOnPitchBend(oc->_note + oc->_in->_transpose, _freqLSB);
}

void TownsMidiInputChannel::controlChange(byte control, byte value) {
	switch (control) {
	case 1:
		controlModulationWheel(value);
		break;
	case 7:
		controlVolume(value);
		break;
	case 10:
		controlPanPos(value);
		break;
	case 64:
		controlSustain(value);
		break;
	case 123:
		while (_out)
			_out->disconnect();
		break;
	default:
		break;
	}
}

void TownsMidiInputChannel::pitchBendFactor(byte value) {
	_pitchBendFactor = value;
	_freqLSB = ((_pitchBend * _pitchBendFactor) >> 6) + _detune;
	for (TownsMidiOutputChannel *oc = _out; oc; oc = oc->_next)
		oc->noteOnPitchBend(oc->_note + oc->_in->_transpose, _freqLSB);
}

void TownsMidiInputChannel::priority(byte value) {
	_priority = value;
}

void TownsMidiInputChannel::sysEx_customInstrument(uint32 type, const byte *instr) {
	memcpy(_instrument, instr, 30);
}

void TownsMidiInputChannel::controlModulationWheel(byte value) {
	_modWheel = value;
	for (TownsMidiOutputChannel *oc = _out; oc; oc = oc->_next)
		oc->setModWheel(value);
}

void TownsMidiInputChannel::controlVolume(byte value) {
	/* This is all done inside the imuse code
	uint16 v1 = _ctrlVolume + 1;
	uint16 v2 = value;
	if (_chanIndex != 16) {
		_ctrlVolume = value;
		v2 = _player->getEffectiveVolume();
	}
	_tl = (v1 * v2) >> 7;*/

	_tl = value;
}

void TownsMidiInputChannel::controlPanPos(byte value) {
	// not implemented
}

void TownsMidiInputChannel::controlSustain(byte value) {
	_sustain = value;
	if (!value)
		releasePedal();
}

void TownsMidiInputChannel::releasePedal() {
	for (TownsMidiOutputChannel *oc = _out; oc; oc = oc->_next) {
		if (oc->_sustainNoteOff)
			oc->disconnect();
	}
}

const uint8 TownsMidiInputChannel::_programAdjustLevel[] = {
	0x00, 0x04, 0x07, 0x0B, 0x0D, 0x10, 0x12, 0x14,
	0x16, 0x18, 0x1A, 0x1B, 0x1D, 0x1E, 0x1F, 0x21,
	0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x2A, 0x2B, 0x2C, 0x2C, 0x2D, 0x2E, 0x2F, 0x2F,
	0x30, 0x31, 0x31, 0x32, 0x33, 0x33, 0x34, 0x35,
	0x35, 0x36, 0x36, 0x37, 0x37, 0x38, 0x38, 0x39,
	0x39, 0x3A, 0x3A, 0x3B, 0x3B, 0x3C, 0x3C, 0x3C,
	0x3D, 0x3D, 0x3E, 0x3E, 0x3E, 0x3F, 0x3F, 0x3F
};

MidiDriver_TOWNS::MidiDriver_TOWNS(Audio::Mixer *mixer) : _timerProc(0), _timerProcPara(0), _channels(0), _out(0),
	_baseTempo(10080), _chanState(0), _operatorLevelTable(0), _tickCounter(0), _rand(1), _allocCurPos(0), _isOpen(false) {
	// We set exteral mutex handling to true to avoid lockups in SCUMM which has its own mutex.
	_intf = new TownsAudioInterface(mixer, this, true);

	_channels = new TownsMidiInputChannel*[32];
	for (int i = 0; i < 32; i++)
		_channels[i] = new TownsMidiInputChannel(this, i > 8 ? (i + 1) : i);

	_out = new TownsMidiOutputChannel*[6];
	for (int i = 0; i < 6; i++)
		_out[i] = new TownsMidiOutputChannel(this, i);

	_chanState = new TownsMidiChanState[32];

	_operatorLevelTable = new uint8[2048];
	for (int i = 0; i < 64; i++) {
		for (int ii = 0; ii < 32; ii++)
			_operatorLevelTable[(i << 5) + ii] = ((i * (ii + 1)) >> 5) & 0xff;
	}
	for (int i = 0; i < 64; i++)
		_operatorLevelTable[i << 5] = 0;
}

MidiDriver_TOWNS::~MidiDriver_TOWNS() {
	close();
	delete _intf;

	if (_channels) {
		for (int i = 0; i < 32; i++)
			delete _channels[i];
		delete[] _channels;
	}
	_channels = 0;

	if (_out) {
		for (int i = 0; i < 6; i++)
			delete _out[i];
		delete[] _out;
	}
	_out = 0;

	delete[] _chanState;
	_chanState = 0;
	delete[] _operatorLevelTable;
	_operatorLevelTable = 0;
}

int MidiDriver_TOWNS::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	if (!_intf->init())
		return MERR_CANNOT_CONNECT;

	_intf->callback(0);

	_intf->callback(21, 255, 1);
	_intf->callback(21, 0, 1);
	_intf->callback(22, 255, 221);

	_intf->callback(33, 8);
	_intf->setSoundEffectChanMask(~0x3f);

	_allocCurPos = 0;

	_isOpen = true;

	return 0;
}

void MidiDriver_TOWNS::close() {
	if (!_isOpen)
		return;

	_isOpen = false;

	setTimerCallback(0, 0);
	g_system->delayMillis(20);
}

void MidiDriver_TOWNS::send(uint32 b) {
	if (!_isOpen)
		return;

	byte param2 = (b >> 16) & 0xFF;
	byte param1 = (b >> 8) & 0xFF;
	byte cmd = b & 0xF0;

	TownsMidiInputChannel *c = _channels[b & 0x0F];

	switch (cmd) {
	case 0x80:
		c->noteOff(param1);
		break;
	case 0x90:
		if (param2)
			c->noteOn(param1, param2);
		else
			c->noteOff(param1);
		break;
	case 0xB0:
		c->controlChange(param1, param2);
		break;
	case 0xC0:
		c->programChange(param1);
		break;
	case 0xE0:
		c->pitchBend((param1 | (param2 << 7)) - 0x2000);
		break;
	case 0xF0:
		warning("MidiDriver_TOWNS: Receiving SysEx command on a send() call");
		break;

	default:
		break;
	}
}

void MidiDriver_TOWNS::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_timerProc = timer_proc;
	_timerProcPara = timer_param;
}

uint32 MidiDriver_TOWNS::getBaseTempo() {
	return _baseTempo;
}

MidiChannel *MidiDriver_TOWNS::allocateChannel() {
	if (!_isOpen)
		return 0;

	for (int i = 0; i < 32; ++i) {
		TownsMidiInputChannel *chan = _channels[i];
		if (chan->allocate())
			return chan;
	}

	return 0;
}

MidiChannel *MidiDriver_TOWNS::getPercussionChannel() {
	return 0;
}

void MidiDriver_TOWNS::timerCallback(int timerId) {
	if (!_isOpen)
		return;

	switch (timerId) {
	case 1:
		updateParser();
		updateOutputChannels();
		break;
	default:
		break;
	}
}

void MidiDriver_TOWNS::updateParser() {
	if (_timerProc)
		_timerProc(_timerProcPara);
}

void MidiDriver_TOWNS::updateOutputChannels() {
	_tickCounter += _baseTempo;
	while (_tickCounter >= 16667) {
		_tickCounter -= 16667;
		for (int i = 0; i < 6; i++) {
			if (_out[i]->update())
				return;
		}
	}
}

TownsMidiOutputChannel *MidiDriver_TOWNS::allocateOutputChannel(uint8 pri) {
	TownsMidiOutputChannel *res = 0;

	for (int i = 0; i < 6; i++) {
		if (++_allocCurPos == 6)
			_allocCurPos = 0;

		int s = _out[_allocCurPos]->checkPriority(pri);
		if (s == TownsMidiOutputChannel::kDisconnected)
			return _out[_allocCurPos];

		if (s != TownsMidiOutputChannel::kHighPriority) {
			pri = s;
			res = _out[_allocCurPos];
		}
	}

	if (res)
		res->disconnect();

	return res;
}

int MidiDriver_TOWNS::randomValue(int para) {
	_rand = (_rand & 1) ? (_rand >> 1) ^ 0xb8 : (_rand >> 1);
	return (_rand * para) >> 8;
}
