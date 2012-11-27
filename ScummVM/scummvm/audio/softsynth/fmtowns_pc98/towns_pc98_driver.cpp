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

#include "audio/softsynth/fmtowns_pc98/towns_pc98_driver.h"
#include "common/endian.h"
#include "common/textconsole.h"

class TownsPC98_MusicChannel {
public:
	TownsPC98_MusicChannel(TownsPC98_AudioDriver *driver, uint8 regOffs, uint8 flgs, uint8 num,
	                       uint8 key, uint8 prt, uint8 id);
	virtual ~TownsPC98_MusicChannel();
	virtual void init();

	typedef enum channelState {
		CHS_RECALCFREQ = 0x01,
		CHS_KEYOFF     = 0x02,
		CHS_SSGOFF     = 0x04,
		CHS_VBROFF     = 0x08,
		CHS_ALLOFF     = 0x0f,
		CHS_PROTECT    = 0x40,
		CHS_EOT        = 0x80
	} ChannelState;

	virtual void loadData(uint8 *data);
	virtual void processEvents();
	virtual void processFrequency();
	virtual bool processControlEvent(uint8 cmd);

	virtual void keyOn();
	void keyOff();

	void setOutputLevel();
	virtual void fadeStep();
	virtual void reset();

	const uint8 _idFlag;

protected:
	void setupVibrato();
	bool processVibrato();

	bool control_dummy(uint8 para);
	bool control_f0_setPatch(uint8 para);
	bool control_f1_presetOutputLevel(uint8 para);
	bool control_f2_setKeyOffTime(uint8 para);
	bool control_f3_setFreqLSB(uint8 para);
	bool control_f4_setOutputLevel(uint8 para);
	bool control_f5_setTempo(uint8 para);
	bool control_f6_repeatSection(uint8 para);
	bool control_f7_setupVibrato(uint8 para);
	bool control_f8_toggleVibrato(uint8 para);
	bool control_fa_writeReg(uint8 para);
	virtual bool control_fb_incOutLevel(uint8 para);
	virtual bool control_fc_decOutLevel(uint8 para);
	bool control_fd_jump(uint8 para);
	virtual bool control_ff_endOfTrack(uint8 para);

	uint8 _ticksLeft;
	uint8 _algorithm;
	uint8 _instr;
	uint8 _totalLevel;
	uint8 _frqBlockMSB;
	int8 _frqLSB;
	uint8 _keyOffTime;
	bool _hold;
	uint8 *_dataPtr;
	uint8 _vbrInitDelayHi;
	uint8 _vbrInitDelayLo;
	int16 _vbrModInitVal;
	uint8 _vbrDuration;
	uint8 _vbrCurDelay;
	int16 _vbrModCurVal;
	uint8 _vbrDurLeft;
	uint16 _frequency;
	uint8 _block;
	uint8 _regOffset;
	uint8 _flags;
	uint8 _ssgTl;
	uint8 _ssgStep;
	uint8 _ssgTicksLeft;
	uint8 _ssgTargetLvl;
	uint8 _ssgStartLvl;

	const uint8 _chanNum;
	const uint8 _keyNum;
	const uint8 _part;

	TownsPC98_AudioDriver *_drv;

	typedef bool (TownsPC98_MusicChannel::*ControlEventFunc)(uint8 para);
	const ControlEventFunc *controlEvents;
};

class TownsPC98_MusicChannelSSG : public TownsPC98_MusicChannel {
public:
	TownsPC98_MusicChannelSSG(TownsPC98_AudioDriver *driver, uint8 regOffs,
	                          uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id);
	virtual ~TownsPC98_MusicChannelSSG() {}
	void init();

	virtual void loadData(uint8 *data);
	void processEvents();
	void processFrequency();
	bool processControlEvent(uint8 cmd);

	void keyOn();
	void nextShape();

	void protect();
	void restore();
	virtual void reset();

	void fadeStep();

protected:
	void setOutputLevel(uint8 lvl);

	bool control_f0_setPatch(uint8 para);
	bool control_f1_setTotalLevel(uint8 para);
	bool control_f4_setAlgorithm(uint8 para);
	bool control_f9_loadCustomPatch(uint8 para);
	bool control_fb_incOutLevel(uint8 para);
	bool control_fc_decOutLevel(uint8 para);
	bool control_ff_endOfTrack(uint8 para);

	typedef bool (TownsPC98_MusicChannelSSG::*ControlEventFunc)(uint8 para);
	const ControlEventFunc *controlEvents;
};

class TownsPC98_SfxChannel : public TownsPC98_MusicChannelSSG {
public:
	TownsPC98_SfxChannel(TownsPC98_AudioDriver *driver, uint8 regOffs,
	                     uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id) :
		TownsPC98_MusicChannelSSG(driver, regOffs, flgs, num, key, prt, id) {}
	~TownsPC98_SfxChannel() {}

	void loadData(uint8 *data);
	void reset();
};

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
class TownsPC98_MusicChannelPCM : public TownsPC98_MusicChannel {
public:
	TownsPC98_MusicChannelPCM(TownsPC98_AudioDriver *driver, uint8 regOffs,
	                          uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id);
	~TownsPC98_MusicChannelPCM() {}
	void init();

	void loadData(uint8 *data);
	void processEvents();
	bool processControlEvent(uint8 cmd);

private:
	bool control_f1_prcStart(uint8 para);
	bool control_ff_endOfTrack(uint8 para);

	typedef bool (TownsPC98_MusicChannelPCM::*ControlEventFunc)(uint8 para);
	const ControlEventFunc *controlEvents;
};
#endif

TownsPC98_MusicChannel::TownsPC98_MusicChannel(TownsPC98_AudioDriver *driver, uint8 regOffs, uint8 flgs, uint8 num,
        uint8 key, uint8 prt, uint8 id) : _drv(driver), _regOffset(regOffs), _flags(flgs), _chanNum(num), _keyNum(key),
	_part(prt), _idFlag(id), controlEvents(0) {

	_ticksLeft = _algorithm = _instr = _totalLevel = _frqBlockMSB = _keyOffTime = 0;
	_ssgStartLvl = _ssgTl = _ssgStep = _ssgTicksLeft = _ssgTargetLvl = _block = 0;
	_vbrInitDelayHi = _vbrInitDelayLo = _vbrDuration = _vbrCurDelay = _vbrDurLeft = 0;
	_frqLSB = 0;
	_hold = false;
	_dataPtr = 0;
	_vbrModInitVal = _vbrModCurVal = 0;
	_frequency = 0;
}

TownsPC98_MusicChannel::~TownsPC98_MusicChannel() {
}

void TownsPC98_MusicChannel::init() {
#define Control(x) &TownsPC98_MusicChannel::control_##x
	static const ControlEventFunc ctrlEvents[] = {
		Control(f0_setPatch),
		Control(f1_presetOutputLevel),
		Control(f2_setKeyOffTime),
		Control(f3_setFreqLSB),
		Control(f4_setOutputLevel),
		Control(f5_setTempo),
		Control(f6_repeatSection),
		Control(f7_setupVibrato),
		Control(f8_toggleVibrato),
		Control(dummy),
		Control(fa_writeReg),
		Control(fb_incOutLevel),
		Control(fc_decOutLevel),
		Control(fd_jump),
		Control(dummy),
		Control(ff_endOfTrack)
	};
#undef Control

	controlEvents = ctrlEvents;
}

void TownsPC98_MusicChannel::keyOff() {
	// all operators off
	uint8 value = _keyNum & 0x0f;
	if (_part)
		value |= 4;
	uint8 regAddress = 0x28;
	_drv->writeReg(0, regAddress, value);
	_flags |= CHS_KEYOFF;
}

void TownsPC98_MusicChannel::keyOn() {
	// all operators on
	uint8 value = _keyNum | 0xf0;
	if (_part)
		value |= 4;
	uint8 regAddress = 0x28;
	_drv->writeReg(0, regAddress, value);
}

void TownsPC98_MusicChannel::loadData(uint8 *data) {
	_flags = (_flags & ~CHS_EOT) | CHS_ALLOFF;
	_ticksLeft = 1;
	_dataPtr = data;
	_totalLevel = 0x7F;

	uint8 *tmp = _dataPtr;
	for (bool loop = true; loop;) {
		uint8 cmd = *tmp++;
		if (cmd < 0xf0) {
			tmp++;
		} else if (cmd == 0xff) {
			if (READ_LE_UINT16(tmp)) {
				_drv->_looping |= _idFlag;
				tmp += _drv->_opnFxCmdLen[cmd - 240];
			} else
				loop = false;
		} else if (cmd == 0xf6) {
			// reset repeat section countdown
			tmp[0] = tmp[1];
			tmp += 4;
		} else {
			tmp += _drv->_opnFxCmdLen[cmd - 240];
		}
	}
}

void TownsPC98_MusicChannel::processEvents() {
	if (_flags & CHS_EOT)
		return;

	if (!_hold && _ticksLeft == _keyOffTime)
		keyOff();

	if (--_ticksLeft)
		return;

	if (!_hold)
		keyOff();

	uint8 cmd = 0;
	bool loop = true;

	while (loop) {
		cmd = *_dataPtr++;
		if (cmd < 0xf0)
			loop = false;
		else if (!processControlEvent(cmd))
			return;
	}

	uint8 para = *_dataPtr++;

	if (cmd == 0x80) {
		keyOff();
		_hold = false;
	} else {
		keyOn();

		if (_hold == false || cmd != _frqBlockMSB)
			_flags |= CHS_RECALCFREQ;

		_hold = (para & 0x80) ? true : false;
		_frqBlockMSB = cmd;
	}

	_ticksLeft = para & 0x7f;
}

void TownsPC98_MusicChannel::processFrequency() {
	if (_flags & CHS_RECALCFREQ) {

		_frequency = (READ_LE_UINT16(&_drv->_opnFreqTable[(_frqBlockMSB & 0x0f) << 1]) + _frqLSB) | (((_frqBlockMSB & 0x70) >> 1) << 8);

		_drv->writeReg(_part, _regOffset + 0xa4, (_frequency >> 8));
		_drv->writeReg(_part, _regOffset + 0xa0, (_frequency & 0xff));

		setupVibrato();
	}

	if (!(_flags & CHS_VBROFF)) {
		if (!processVibrato())
			return;

		_drv->writeReg(_part, _regOffset + 0xa4, (_frequency >> 8));
		_drv->writeReg(_part, _regOffset + 0xa0, (_frequency & 0xff));
	}
}

void TownsPC98_MusicChannel::setupVibrato() {
	_vbrCurDelay = _vbrInitDelayHi;
	if (_flags & CHS_KEYOFF) {
		_vbrModCurVal = _vbrModInitVal;
		_vbrCurDelay += _vbrInitDelayLo;
	}
	_vbrDurLeft = (_vbrDuration >> 1);
	_flags &= ~(CHS_KEYOFF | CHS_RECALCFREQ);
}

bool TownsPC98_MusicChannel::processVibrato() {
	if (--_vbrCurDelay)
		return false;

	_vbrCurDelay = _vbrInitDelayHi;
	_frequency += _vbrModCurVal;

	if (!--_vbrDurLeft) {
		_vbrDurLeft = _vbrDuration;
		_vbrModCurVal = -_vbrModCurVal;
	}

	return true;
}

bool TownsPC98_MusicChannel::processControlEvent(uint8 cmd) {
	uint8 para = *_dataPtr++;
	return (this->*controlEvents[cmd & 0x0f])(para);
}

void TownsPC98_MusicChannel::setOutputLevel() {
	uint8 outopr = _drv->_opnCarrier[_algorithm];
	uint8 reg = 0x40 + _regOffset;

	for (int i = 0; i < 4; i++) {
		if (outopr & 1)
			_drv->writeReg(_part, reg, _totalLevel);
		outopr >>= 1;
		reg += 4;
	}
}

void TownsPC98_MusicChannel::fadeStep() {
	_totalLevel += 3;
	if (_totalLevel > 0x7f)
		_totalLevel = 0x7f;
	setOutputLevel();
}

void TownsPC98_MusicChannel::reset() {
	_hold = false;
	_keyOffTime = 0;
	_ticksLeft = 1;

	_flags = (_flags & ~CHS_EOT) | CHS_ALLOFF;

	_totalLevel = 0;
	_algorithm = 0;

	_block = 0;
	_frequency = 0;
	_frqBlockMSB = 0;
	_frqLSB = 0;

	_ssgTl = 0;
	_ssgStartLvl = 0;
	_ssgTargetLvl = 0;
	_ssgStep = 0;
	_ssgTicksLeft = 0;

	_vbrInitDelayHi = 0;
	_vbrInitDelayLo = 0;
	_vbrModInitVal = 0;
	_vbrDuration = 0;
	_vbrCurDelay = 0;
	_vbrModCurVal = 0;
	_vbrDurLeft = 0;
}

bool TownsPC98_MusicChannel::control_f0_setPatch(uint8 para) {
	_instr = para;
	uint8 reg = _regOffset + 0x80;

	for (int i = 0; i < 4; i++) {
		// set release rate for each operator
		_drv->writeReg(_part, reg, 0x0f);
		reg += 4;
	}

	const uint8 *tptr = _drv->_patches + ((uint32)_instr << 5);
	reg = _regOffset + 0x30;

	// write registers 0x30 to 0x8f
	for (int i = 0; i < 6; i++) {
		_drv->writeReg(_part, reg, tptr[0]);
		reg += 4;
		_drv->writeReg(_part, reg, tptr[2]);
		reg += 4;
		_drv->writeReg(_part, reg, tptr[1]);
		reg += 4;
		_drv->writeReg(_part, reg, tptr[3]);
		reg += 4;
		tptr += 4;
	}

	reg = _regOffset + 0xB0;
	_algorithm = tptr[0] & 7;
	// set feedback and algorithm
	_drv->writeReg(_part, reg, tptr[0]);

	setOutputLevel();
	return true;
}

bool TownsPC98_MusicChannel::control_f1_presetOutputLevel(uint8 para) {
	if (_drv->_fading)
		return true;

	_totalLevel = _drv->_opnLvlPresets[para];
	setOutputLevel();
	return true;
}

bool TownsPC98_MusicChannel::control_f2_setKeyOffTime(uint8 para) {
	_keyOffTime = para;
	return true;
}

bool TownsPC98_MusicChannel::control_f3_setFreqLSB(uint8 para) {
	_frqLSB = (int8) para;
	return true;
}

bool TownsPC98_MusicChannel::control_f4_setOutputLevel(uint8 para) {
	if (_drv->_fading)
		return true;

	_totalLevel = para;
	setOutputLevel();
	return true;
}

bool TownsPC98_MusicChannel::control_f5_setTempo(uint8 para) {
	_drv->setMusicTempo(para);
	return true;
}

bool TownsPC98_MusicChannel::control_f6_repeatSection(uint8 para) {
	_dataPtr--;
	_dataPtr[0]--;

	if (*_dataPtr) {
		// repeat section until counter has reached zero
		_dataPtr = _drv->_trackPtr + READ_LE_UINT16(_dataPtr + 2);
	} else {
		// reset counter, advance to next section
		_dataPtr[0] = _dataPtr[1];
		_dataPtr += 4;
	}
	return true;
}

bool TownsPC98_MusicChannel::control_f7_setupVibrato(uint8 para) {
	_vbrInitDelayHi = _dataPtr[0];
	_vbrInitDelayLo = para;
	_vbrModInitVal = (int16) READ_LE_UINT16(_dataPtr + 1);
	_vbrDuration = _dataPtr[3];
	_dataPtr += 4;
	_flags = (_flags & ~CHS_VBROFF) | CHS_KEYOFF | CHS_RECALCFREQ;
	return true;
}

bool TownsPC98_MusicChannel::control_f8_toggleVibrato(uint8 para) {
	if (para == 0x10) {
		if (*_dataPtr++) {
			_flags = (_flags & ~CHS_VBROFF) | CHS_KEYOFF;
		} else {
			_flags |= CHS_VBROFF;
		}
	} else {
		/* NOT IMPLEMENTED
		uint8 skipChannels = para / 36;
		uint8 entry = para % 36;
		TownsPC98_AudioDriver::TownsPC98_MusicChannel *t = &chan[skipChannels];

		t->unnamedEntries[entry] = *_dataPtr++;*/
	}
	return true;
}

bool TownsPC98_MusicChannel::control_fa_writeReg(uint8 para) {
	_drv->writeReg(_part, para, *_dataPtr++);
	return true;
}

bool TownsPC98_MusicChannel::control_fb_incOutLevel(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	uint8 val = (_totalLevel + 3);
	if (val > 0x7f)
		val = 0x7f;

	_totalLevel = val;
	setOutputLevel();
	return true;
}

bool TownsPC98_MusicChannel::control_fc_decOutLevel(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	int8 val = (int8)(_totalLevel - 3);
	if (val < 0)
		val = 0;

	_totalLevel = (uint8) val;
	setOutputLevel();
	return true;
}

bool TownsPC98_MusicChannel::control_fd_jump(uint8 para) {
	uint8 *tmp = _drv->_trackPtr + READ_LE_UINT16(_dataPtr - 1);
	_dataPtr = (tmp[1] == 1) ? tmp : (_dataPtr + 1);
	return true;
}

bool TownsPC98_MusicChannel::control_dummy(uint8 para) {
	_dataPtr--;
	return true;
}

bool TownsPC98_MusicChannel::control_ff_endOfTrack(uint8 para) {
	uint16 val = READ_LE_UINT16(--_dataPtr);
	if (val) {
		// loop
		_dataPtr = _drv->_trackPtr + val;
		return true;
	} else {
		// quit parsing for active channel
		--_dataPtr;
		_flags |= CHS_EOT;
		_drv->_finishedChannelsFlag |= _idFlag;
		keyOff();
		return false;
	}
}

TownsPC98_MusicChannelSSG::TownsPC98_MusicChannelSSG(TownsPC98_AudioDriver *driver, uint8 regOffs,
        uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id) :
	TownsPC98_MusicChannel(driver, regOffs, flgs, num, key, prt, id), controlEvents(0) {
}

void TownsPC98_MusicChannelSSG::init() {
	_algorithm = 0x80;

#define Control(x) &TownsPC98_MusicChannelSSG::control_##x
	static const ControlEventFunc ctrlEventsSSG[] = {
		Control(f0_setPatch),
		Control(f1_setTotalLevel),
		Control(f2_setKeyOffTime),
		Control(f3_setFreqLSB),
		Control(f4_setAlgorithm),
		Control(f5_setTempo),
		Control(f6_repeatSection),
		Control(f7_setupVibrato),
		Control(f8_toggleVibrato),
		Control(f9_loadCustomPatch),
		Control(fa_writeReg),
		Control(fb_incOutLevel),
		Control(fc_decOutLevel),
		Control(fd_jump),
		Control(dummy),
		Control(ff_endOfTrack)
	};
#undef Control

	controlEvents = ctrlEventsSSG;
}

void TownsPC98_MusicChannelSSG::processEvents() {
	if (_flags & CHS_EOT)
		return;

	_drv->toggleRegProtection(_flags & CHS_PROTECT ? true : false);

	if (!_hold && _ticksLeft == _keyOffTime)
		nextShape();

	if (!--_ticksLeft) {

		uint8 cmd = 0;
		bool loop = true;

		while (loop) {
			cmd = *_dataPtr++;
			if (cmd < 0xf0)
				loop = false;
			else if (!processControlEvent(cmd))
				return;
		}

		uint8 para = *_dataPtr++;

		if (cmd == 0x80) {
			nextShape();
			_hold = false;
		} else {
			if (!_hold) {
				_instr &= 0xf0;
				_ssgStep = _drv->_ssgPatches[_instr];
				_ssgTicksLeft = _drv->_ssgPatches[_instr + 1] & 0x7f;
				_ssgTargetLvl = _drv->_ssgPatches[_instr + 2];
				_ssgStartLvl = _drv->_ssgPatches[_instr + 3];
				_flags = (_flags & ~CHS_SSGOFF) | CHS_KEYOFF;
			}

			keyOn();

			if (_hold == false || cmd != _frqBlockMSB)
				_flags |= CHS_RECALCFREQ;

			_hold = (para & 0x80) ? true : false;
			_frqBlockMSB = cmd;
		}

		_ticksLeft = para & 0x7f;
	}

	if (!(_flags & CHS_SSGOFF)) {
		if (--_ssgTicksLeft) {
			if (!_drv->_fading)
				setOutputLevel(_ssgStartLvl);
			return;
		}

		_ssgTicksLeft = _drv->_ssgPatches[_instr + 1] & 0x7f;

		if (_drv->_ssgPatches[_instr + 1] & 0x80) {
			uint8 t = _ssgStartLvl - _ssgStep;

			if (_ssgStep <= _ssgStartLvl && _ssgTargetLvl < t) {
				if (!_drv->_fading)
					setOutputLevel(t);
				return;
			}
		} else {
			int t = _ssgStartLvl + _ssgStep;
			uint8 p = (uint8)(t & 0xff);

			if (t < 256 && _ssgTargetLvl > p) {
				if (!_drv->_fading)
					setOutputLevel(p);
				return;
			}
		}

		setOutputLevel(_ssgTargetLvl);
		if (_ssgStartLvl && !(_instr & 8)) {
			_instr += 4;
			_ssgStep = _drv->_ssgPatches[_instr];
			_ssgTicksLeft = _drv->_ssgPatches[_instr + 1] & 0x7f;
			_ssgTargetLvl = _drv->_ssgPatches[_instr + 2];
		} else {
			_flags |= CHS_SSGOFF;
			setOutputLevel(0);
		}
	}
}

void TownsPC98_MusicChannelSSG::processFrequency() {
	if (_algorithm & 0x40)
		return;

	if (_flags & CHS_RECALCFREQ) {
		_block = _frqBlockMSB >> 4;
		_frequency = READ_LE_UINT16(&_drv->_opnFreqTableSSG[(_frqBlockMSB & 0x0f) << 1]) + _frqLSB;

		uint16 f = _frequency >> _block;
		_drv->writeReg(_part, _regOffset << 1, f & 0xff);
		_drv->writeReg(_part, (_regOffset << 1) + 1, f >> 8);

		setupVibrato();
	}

	if (!(_flags & (CHS_EOT | CHS_VBROFF | CHS_SSGOFF))) {
		if (!processVibrato())
			return;

		uint16 f = _frequency >> _block;
		_drv->writeReg(_part, _regOffset << 1, f & 0xff);
		_drv->writeReg(_part, (_regOffset << 1) + 1, f >> 8);
	}
}

bool TownsPC98_MusicChannelSSG::processControlEvent(uint8 cmd) {
	uint8 para = *_dataPtr++;
	return (this->*controlEvents[cmd & 0x0f])(para);
}

void TownsPC98_MusicChannelSSG::nextShape() {
	_instr = (_instr & 0xf0) + 0x0c;
	_ssgStep = _drv->_ssgPatches[_instr];
	_ssgTicksLeft = _drv->_ssgPatches[_instr + 1] & 0x7f;
	_ssgTargetLvl = _drv->_ssgPatches[_instr + 2];
}

void TownsPC98_MusicChannelSSG::keyOn() {
	uint8 c = 0x7b;
	uint8 t = (_algorithm & 0xC0) << 1;
	if (_algorithm & 0x80)
		t |= 4;

	c = (c << (_regOffset + 1)) | (c >> (7 - _regOffset));
	t = (t << (_regOffset + 1)) | (t >> (7 - _regOffset));

	if (!(_algorithm & 0x80))
		_drv->writeReg(_part, 6, _algorithm & 0x7f);

	uint8 e = (_drv->readSSGStatus() & c) | t;
	_drv->writeReg(_part, 7, e);
}

void TownsPC98_MusicChannelSSG::protect() {
	_flags |= CHS_PROTECT;
}

void TownsPC98_MusicChannelSSG::restore() {
	_flags &= ~CHS_PROTECT;
	keyOn();
	_drv->writeReg(_part, 8 + _regOffset, _ssgTl);
	uint16 f = _frequency >> _block;
	_drv->writeReg(_part, _regOffset << 1, f & 0xff);
	_drv->writeReg(_part, (_regOffset << 1) + 1, f >> 8);
}

void TownsPC98_MusicChannelSSG::loadData(uint8 *data) {
	_drv->toggleRegProtection(_flags & CHS_PROTECT ? true : false);
	TownsPC98_MusicChannel::loadData(data);
	setOutputLevel(0);
	_algorithm = 0x80;
}

void TownsPC98_MusicChannelSSG::setOutputLevel(uint8 lvl) {
	_ssgStartLvl = lvl;
	uint16 newTl = (((uint16)_totalLevel + 1) * (uint16)lvl) >> 8;
	if (newTl == _ssgTl)
		return;
	_ssgTl = newTl;
	_drv->writeReg(_part, 8 + _regOffset, _ssgTl);
}

void TownsPC98_MusicChannelSSG::reset() {
	TownsPC98_MusicChannel::reset();

	// Unlike the original we restore the default patch data. This fixes a bug
	// where certain sound effects would bring each other out of tune (e.g. the
	// dragon's fire in Darm's house in Kyra 1 would sound different each time
	// you triggered another sfx by dropping an item etc.)
	uint8 i = (10 + _regOffset) << 4;
	const uint8 *src = &_drv->_drvTables[156];
	_drv->_ssgPatches[i] = src[i];
	_drv->_ssgPatches[i + 3] = src[i + 3];
	_drv->_ssgPatches[i + 4] = src[i + 4];
	_drv->_ssgPatches[i + 6] = src[i + 6];
	_drv->_ssgPatches[i + 8] = src[i + 8];
	_drv->_ssgPatches[i + 12] = src[i + 12];
}

void TownsPC98_MusicChannelSSG::fadeStep() {
	_totalLevel--;
	if ((int8)_totalLevel < 0)
		_totalLevel = 0;
	setOutputLevel(_ssgStartLvl);
}

bool TownsPC98_MusicChannelSSG::control_f0_setPatch(uint8 para) {
	_instr = para << 4;
	para = (para >> 3) & 0x1e;
	if (para)
		return control_f4_setAlgorithm(para | 0x40);
	return true;
}

bool TownsPC98_MusicChannelSSG::control_f1_setTotalLevel(uint8 para) {
	if (!_drv->_fading)
		_totalLevel = para;
	return true;
}

bool TownsPC98_MusicChannelSSG::control_f4_setAlgorithm(uint8 para) {
	_algorithm = para;
	return true;
}

bool TownsPC98_MusicChannelSSG::control_f9_loadCustomPatch(uint8 para) {
	_instr = (_drv->_sfxOffs + 10 + _regOffset) << 4;
	_drv->_ssgPatches[_instr] = *_dataPtr++;
	_drv->_ssgPatches[_instr + 3] = para;
	_drv->_ssgPatches[_instr + 4] = *_dataPtr++;
	_drv->_ssgPatches[_instr + 6] = *_dataPtr++;
	_drv->_ssgPatches[_instr + 8] = *_dataPtr++;
	_drv->_ssgPatches[_instr + 12] = *_dataPtr++;
	return true;
}

bool TownsPC98_MusicChannelSSG::control_fb_incOutLevel(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	_totalLevel--;
	if ((int8)_totalLevel < 0)
		_totalLevel = 0;

	return true;
}

bool TownsPC98_MusicChannelSSG::control_fc_decOutLevel(uint8 para) {
	_dataPtr--;
	if (_drv->_fading)
		return true;

	if (_totalLevel + 1 < 0x10)
		_totalLevel++;

	return true;
}

bool TownsPC98_MusicChannelSSG::control_ff_endOfTrack(uint8 para) {
	if (!_drv->_sfxOffs) {
		uint16 val = READ_LE_UINT16(--_dataPtr);
		if (val) {
			// loop
			_dataPtr = _drv->_trackPtr + val;
			return true;
		} else {
			// stop parsing
			if (!_drv->_fading)
				setOutputLevel(0);
			--_dataPtr;
			_flags |= CHS_EOT;
			_drv->_finishedSSGFlag |= _idFlag;
		}
	} else {
		// end of sfx track - restore ssg music channel
		_flags |= CHS_EOT;
		_drv->_finishedSfxFlag |= _idFlag;
		_drv->_ssgChannels[_chanNum]->restore();
	}

	return false;
}

void TownsPC98_SfxChannel::loadData(uint8 *data) {
	_flags = CHS_ALLOFF;
	_ticksLeft = 1;
	_dataPtr = data;
	_ssgTl = 0xff;
	_algorithm = 0x80;

	uint8 *tmp = _dataPtr;
	for (bool loop = true; loop;) {
		uint8 cmd = *tmp++;
		if (cmd < 0xf0) {
			tmp++;
		} else if (cmd == 0xff) {
			loop = false;
		} else if (cmd == 0xf6) {
			// reset repeat section countdown
			tmp[0] = tmp[1];
			tmp += 4;
		} else {
			tmp += _drv->_opnFxCmdLen[cmd - 240];
		}
	}
}

void TownsPC98_SfxChannel::reset() {
	TownsPC98_MusicChannel::reset();

	// Unlike the original we restore the default patch data. This fixes a bug
	// where certain sound effects would bring each other out of tune (e.g. the
	// dragon's fire in Darm's house in Kyra 1 would sound different each time
	// you triggered another sfx by dropping an item etc.)
	uint8 i = (13 + _regOffset) << 4;
	const uint8 *src = &_drv->_drvTables[156];
	_drv->_ssgPatches[i] = src[i];
	_drv->_ssgPatches[i + 3] = src[i + 3];
	_drv->_ssgPatches[i + 4] = src[i + 4];
	_drv->_ssgPatches[i + 6] = src[i + 6];
	_drv->_ssgPatches[i + 8] = src[i + 8];
	_drv->_ssgPatches[i + 12] = src[i + 12];
}

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
TownsPC98_MusicChannelPCM::TownsPC98_MusicChannelPCM(TownsPC98_AudioDriver *driver, uint8 regOffs,
        uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id) :
	TownsPC98_MusicChannel(driver, regOffs, flgs, num, key, prt, id), controlEvents(0) {
}

void TownsPC98_MusicChannelPCM::init() {
	_algorithm = 0x80;

#define Control(x) &TownsPC98_MusicChannelPCM::control_##x
	static const ControlEventFunc ctrlEventsPCM[] = {
		Control(dummy),
		Control(f1_prcStart),
		Control(dummy),
		Control(dummy),
		Control(dummy),
		Control(dummy),
		Control(f6_repeatSection),
		Control(dummy),
		Control(dummy),
		Control(dummy),
		Control(fa_writeReg),
		Control(dummy),
		Control(dummy),
		Control(dummy),
		Control(dummy),
		Control(ff_endOfTrack)
	};
#undef Control

	controlEvents = ctrlEventsPCM;
}

void TownsPC98_MusicChannelPCM::loadData(uint8 *data) {
	_flags = (_flags & ~CHS_EOT) | CHS_ALLOFF;
	_ticksLeft = 1;
	_dataPtr = data;
	_totalLevel = 0x7F;
}

void TownsPC98_MusicChannelPCM::processEvents()  {
	if (_flags & CHS_EOT)
		return;

	if (--_ticksLeft)
		return;

	uint8 cmd = 0;
	bool loop = true;

	while (loop) {
		cmd = *_dataPtr++;
		if (cmd == 0x80) {
			loop = false;
		} else if (cmd < 0xf0) {
			_drv->writeReg(_part, 0x10, cmd);
		} else if (!processControlEvent(cmd)) {
			return;
		}
	}

	_ticksLeft = *_dataPtr++;
}

bool TownsPC98_MusicChannelPCM::processControlEvent(uint8 cmd) {
	uint8 para = *_dataPtr++;
	return (this->*controlEvents[cmd & 0x0f])(para);
}

bool TownsPC98_MusicChannelPCM::control_f1_prcStart(uint8 para) {
	_totalLevel = para;
	_drv->writeReg(_part, 0x11, para);
	return true;
}

bool TownsPC98_MusicChannelPCM::control_ff_endOfTrack(uint8 para) {
	uint16 val = READ_LE_UINT16(--_dataPtr);
	if (val) {
		// loop
		_dataPtr = _drv->_trackPtr + val;
		return true;
	} else {
		// quit parsing for active channel
		--_dataPtr;
		_flags |= CHS_EOT;
		_drv->_finishedRhythmFlag |= _idFlag;
		return false;
	}
}
#endif // DISABLE_PC98_RHYTHM_CHANNEL

TownsPC98_AudioDriver::TownsPC98_AudioDriver(Audio::Mixer *mixer, EmuType type) : TownsPC98_FmSynth(mixer, type),
	_channels(0), _ssgChannels(0), _sfxChannels(0),
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	_rhythmChannel(0),
#endif
	_trackPtr(0), _sfxData(0), _sfxOffs(0), _ssgPatches(0),
	_patches(0), _sfxBuffer(0), _musicBuffer(0),

	_opnCarrier(_drvTables + 76), _opnFreqTable(_drvTables + 108), _opnFreqTableSSG(_drvTables + 132),
	_opnFxCmdLen(_drvTables + 36), _opnLvlPresets(_drvTables + (type == kTypeTowns ? 52 : 84)),

	_updateChannelsFlag(type == kType26 ? 0x07 : 0x3F), _finishedChannelsFlag(0),
	_updateSSGFlag(type == kTypeTowns ? 0x00 : 0x07), _finishedSSGFlag(0),
	_updateRhythmFlag(type == kType86 ?
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	0x01
#else
	0x00
#endif
	: 0x00), _finishedRhythmFlag(0),
	_updateSfxFlag(0), _finishedSfxFlag(0),

	_musicTickCounter(0),

	_musicVolume(255), _sfxVolume(255),

	_musicPlaying(false), _sfxPlaying(false), _fading(false), _looping(0), _ready(false) {

	_sfxOffsets[0] = _sfxOffsets[1] = 0;
}

TownsPC98_AudioDriver::~TownsPC98_AudioDriver() {
	_ready = false;
	deinit();

	Common::StackLock lock(_mutex);

	if (_channels) {
		for (int i = 0; i < _numChan; i++)
			delete _channels[i];
		delete[] _channels;
	}

	if (_ssgChannels) {
		for (int i = 0; i < _numSSG; i++)
			delete _ssgChannels[i];
		delete[] _ssgChannels;
	}

	if (_sfxChannels) {
		for (int i = 0; i < 2; i++)
			delete _sfxChannels[i];
		delete[] _sfxChannels;
	}
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	delete _rhythmChannel;
#endif

	delete[] _ssgPatches;
}

bool TownsPC98_AudioDriver::init() {
	if (_ready) {
		reset();
		return true;
	}

	TownsPC98_FmSynth::init();

	setVolumeChannelMasks(-1, 0);

	_channels = new TownsPC98_MusicChannel *[_numChan];
	for (int i = 0; i < _numChan; i++) {
		int ii = i * 6;
		_channels[i] = new TownsPC98_MusicChannel(this, _drvTables[ii], _drvTables[ii + 1],
		        _drvTables[ii + 2], _drvTables[ii + 3], _drvTables[ii + 4], _drvTables[ii + 5]);
		_channels[i]->init();
	}

	if (_numSSG) {
		_ssgPatches = new uint8[256];
		memcpy(_ssgPatches, _drvTables + 156, 256);

		_ssgChannels = new TownsPC98_MusicChannelSSG *[_numSSG];
		for (int i = 0; i < _numSSG; i++) {
			int ii = i * 6;
			_ssgChannels[i] = new TownsPC98_MusicChannelSSG(this, _drvTables[ii], _drvTables[ii + 1],
			        _drvTables[ii + 2], _drvTables[ii + 3], _drvTables[ii + 4], _drvTables[ii + 5]);
			_ssgChannels[i]->init();
		}

		_sfxChannels = new TownsPC98_SfxChannel *[2];
		for (int i = 0; i < 2; i++) {
			int ii = (i + 1) * 6;
			_sfxChannels[i] = new TownsPC98_SfxChannel(this, _drvTables[ii], _drvTables[ii + 1],
			        _drvTables[ii + 2], _drvTables[ii + 3], _drvTables[ii + 4], _drvTables[ii + 5]);
			_sfxChannels[i]->init();
		}
	}

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	if (_hasPercussion) {
		_rhythmChannel = new TownsPC98_MusicChannelPCM(this, 0, 0, 0, 0, 0, 1);
		_rhythmChannel->init();
	}
#endif

	setMusicTempo(84);
	setSfxTempo(654);

	_ready = true;

	return true;
}

void TownsPC98_AudioDriver::loadMusicData(uint8 *data, bool loadPaused) {
	if (!_ready) {
		warning("TownsPC98_AudioDriver: Driver must be initialized before loading data");
		return;
	}

	if (!data) {
		warning("TownsPC98_AudioDriver: Invalid music file data");
		return;
	}

	reset();

	Common::StackLock lock(_mutex);
	uint8 *src_a = _trackPtr = _musicBuffer = data;

	for (uint8 i = 0; i < 3; i++) {
		_channels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	for (int i = 0; i < _numSSG; i++) {
		_ssgChannels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	for (uint8 i = 3; i < _numChan; i++) {
		_channels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	if (_hasPercussion) {
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
		_rhythmChannel->loadData(data + READ_LE_UINT16(src_a));
#endif
		src_a += 2;
	}

	toggleRegProtection(false);

	_patches = src_a + 4;
	_finishedChannelsFlag = _finishedSSGFlag = _finishedRhythmFlag = 0;

	_musicPlaying = !loadPaused;
}

void TownsPC98_AudioDriver::loadSoundEffectData(uint8 *data, uint8 trackNum) {
	if (!_ready) {
		warning("TownsPC98_AudioDriver: Driver must be initialized before loading data");
		return;
	}

	if (!_sfxChannels) {
		warning("TownsPC98_AudioDriver: Sound effects not supported by this configuration");
		return;
	}

	if (!data) {
		warning("TownsPC98_AudioDriver: Invalid sound effects file data");
		return;
	}

	Common::StackLock lock(_mutex);
	_sfxData = _sfxBuffer = data;
	_sfxOffsets[0] = READ_LE_UINT16(&_sfxData[(trackNum << 2)]);
	_sfxOffsets[1] = READ_LE_UINT16(&_sfxData[(trackNum << 2) + 2]);
	_sfxPlaying = true;
	_finishedSfxFlag = 0;
}

void TownsPC98_AudioDriver::reset() {
	Common::StackLock lock(_mutex);

	_musicPlaying = false;
	_sfxPlaying = false;
	_fading = false;
	_looping = 0;
	_musicTickCounter = 0;
	_sfxData = 0;

	TownsPC98_FmSynth::reset();

	for (int i = 0; i < _numChan; i++)
		_channels[i]->reset();
	for (int i = 0; i < _numSSG; i++)
		_ssgChannels[i]->reset();

	if (_numSSG) {
		for (int i = 0; i < 2; i++)
			_sfxChannels[i]->reset();

		memcpy(_ssgPatches, _drvTables + 156, 256);
	}

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	if (_rhythmChannel)
		_rhythmChannel->reset();
#endif
}

void TownsPC98_AudioDriver::fadeStep() {
	if (!_musicPlaying)
		return;

	for (int j = 0; j < _numChan; j++) {
		if (_updateChannelsFlag & _channels[j]->_idFlag)
			_channels[j]->fadeStep();
	}

	for (int j = 0; j < _numSSG; j++) {
		if (_updateSSGFlag & _ssgChannels[j]->_idFlag)
			_ssgChannels[j]->fadeStep();
	}

	if (!_fading) {
		_fading = 19;
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
		if (_hasPercussion) {
			if (_updateRhythmFlag & _rhythmChannel->_idFlag)
				_rhythmChannel->reset();
		}
#endif
	} else {
		if (!--_fading)
			reset();
	}
}

void TownsPC98_AudioDriver::pause() {
	_musicPlaying = false;
}

void TownsPC98_AudioDriver::cont() {
	_musicPlaying = true;
}

bool TownsPC98_AudioDriver::looping() {
	return _looping == _updateChannelsFlag ? true : false;
}

bool TownsPC98_AudioDriver::musicPlaying() {
	return _musicPlaying;
}

void TownsPC98_AudioDriver::setMusicVolume(int volume) {
	_musicVolume = volume;
	setVolumeIntern(_musicVolume, _sfxVolume);
}

void TownsPC98_AudioDriver::setSoundEffectVolume(int volume) {
	_sfxVolume = volume;
	setVolumeIntern(_musicVolume, _sfxVolume);
}

void TownsPC98_AudioDriver::timerCallbackA() {
	if (_sfxChannels && _sfxPlaying) {
		if (_sfxData)
			startSoundEffect();

		_sfxOffs = 3;
		_trackPtr = _sfxBuffer;

		for (int i = 0; i < 2; i++) {
			if (_updateSfxFlag & _sfxChannels[i]->_idFlag) {
				_sfxChannels[i]->processEvents();
				_sfxChannels[i]->processFrequency();
			}
		}

		_trackPtr = _musicBuffer;
	}

	if (_updateSfxFlag && _finishedSfxFlag == _updateSfxFlag) {
		_sfxPlaying = false;
		_updateSfxFlag = 0;
		setVolumeChannelMasks(-1, 0);
	}
}

void TownsPC98_AudioDriver::timerCallbackB() {
	_sfxOffs = 0;

	if (_musicPlaying) {
		_musicTickCounter++;

		for (int i = 0; i < _numChan; i++) {
			if (_updateChannelsFlag & _channels[i]->_idFlag) {
				_channels[i]->processEvents();
				_channels[i]->processFrequency();
			}
		}

		for (int i = 0; i < _numSSG; i++) {
			if (_updateSSGFlag & _ssgChannels[i]->_idFlag) {
				_ssgChannels[i]->processEvents();
				_ssgChannels[i]->processFrequency();
			}
		}

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
		if (_hasPercussion)
			if (_updateRhythmFlag & _rhythmChannel->_idFlag)
				_rhythmChannel->processEvents();
#endif
	}

	toggleRegProtection(false);

	if (_finishedChannelsFlag == _updateChannelsFlag && _finishedSSGFlag == _updateSSGFlag && _finishedRhythmFlag == _updateRhythmFlag)
		_musicPlaying = false;
}

void TownsPC98_AudioDriver::startSoundEffect() {
	int volFlags = 0;

	for (int i = 0; i < 2; i++) {
		if (_sfxOffsets[i]) {
			_ssgChannels[i + 1]->protect();
			_sfxChannels[i]->reset();
			_sfxChannels[i]->loadData(_sfxData + _sfxOffsets[i]);
			_updateSfxFlag |= _sfxChannels[i]->_idFlag;
			volFlags |= (_sfxChannels[i]->_idFlag << _numChan);
		} else {
			_ssgChannels[i + 1]->restore();
			_updateSfxFlag &= ~_sfxChannels[i]->_idFlag;
		}
	}

	setVolumeChannelMasks(~volFlags, volFlags);
	_sfxData = 0;
}

void TownsPC98_AudioDriver::setMusicTempo(uint8 tempo) {
	writeReg(0, 0x26, tempo);
	writeReg(0, 0x27, 0x33);
}

void TownsPC98_AudioDriver::setSfxTempo(uint16 tempo) {
	writeReg(0, 0x24, tempo & 0xff);
	writeReg(0, 0x25, tempo >> 8);
	writeReg(0, 0x27, 0x33);
}
const uint8 TownsPC98_AudioDriver::_drvTables[] = {
	//  channel presets
	0x00, 0x80, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x80, 0x01, 0x01, 0x00, 0x02,
	0x02, 0x80, 0x02, 0x02, 0x00, 0x04,
	0x00, 0x80, 0x03, 0x04, 0x01, 0x08,
	0x01, 0x80, 0x04, 0x05, 0x01, 0x10,
	0x02, 0x80, 0x05, 0x06, 0x01, 0x20,

	//  control event size
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x05,
	0x02, 0x06, 0x02, 0x00, 0x00, 0x02, 0x00, 0x02,

	//  fmt level presets
	0x54, 0x50, 0x4C, 0x48, 0x44, 0x40, 0x3C, 0x38,
	0x34, 0x30, 0x2C, 0x28, 0x24, 0x20, 0x1C, 0x18,
	0x14, 0x10, 0x0C, 0x08, 0x04, 0x90, 0x90, 0x90,

	//  carriers
	0x08, 0x08, 0x08, 0x08, 0x0C, 0x0E, 0x0E, 0x0F,

	//  pc98 level presets
	0x40, 0x3B, 0x38, 0x34, 0x30, 0x2A, 0x28, 0x25,
	0x22, 0x20, 0x1D, 0x1A, 0x18, 0x15, 0x12, 0x10,
	0x0D, 0x0A, 0x08, 0x05, 0x02, 0x90, 0x90, 0x90,

	//  frequencies
	0x6A, 0x02, 0x8F, 0x02, 0xB6, 0x02, 0xDF, 0x02,
	0x0B, 0x03, 0x39, 0x03, 0x6A, 0x03, 0x9E, 0x03,
	0xD5, 0x03, 0x10, 0x04, 0x4E, 0x04, 0x8F, 0x04,

	//  ssg frequencies
	0xE8, 0x0E, 0x12, 0x0E, 0x48, 0x0D, 0x89, 0x0C,
	0xD5, 0x0B, 0x2B, 0x0B, 0x8A, 0x0A, 0xF3, 0x09,
	0x64, 0x09, 0xDD, 0x08, 0x5E, 0x08, 0xE6, 0x07,

	// ssg patch data
	0x00, 0x00, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x37, 0x81, 0xC8, 0x00,
	0x00, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x37, 0x81, 0xC8, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x00, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x04, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x0A, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0x01, 0x00,
	0xFF, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xFF, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x64, 0x01, 0xFF, 0x64, 0xFF, 0x81, 0xFF, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,

	0x02, 0x01, 0xFF, 0x28, 0xFF, 0x81, 0xF0, 0x00,
	0x00, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xC8, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x28, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0x78, 0x5F, 0x81, 0xA0, 0x00,
	0x05, 0x81, 0x00, 0x00, 0x28, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00
};

#undef EUPHONY_FADEOUT_TICKS
