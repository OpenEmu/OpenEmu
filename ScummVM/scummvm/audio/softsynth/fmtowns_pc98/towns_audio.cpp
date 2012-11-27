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

#include "audio/softsynth/fmtowns_pc98/towns_audio.h"
#include "audio/softsynth/fmtowns_pc98/towns_pc98_fmsynth.h"

#include "common/endian.h"
#include "common/util.h"
#include "common/textconsole.h"
#include "backends/audiocd/audiocd.h"

class TownsAudio_WaveTable {
friend class TownsAudioInterfaceInternal;
friend class TownsAudio_PcmChannel;
public:
	TownsAudio_WaveTable();
	~TownsAudio_WaveTable();

private:
	void readHeader(const uint8 *buffer);
	void readData(const uint8 *buffer);
	void clear();

	char name[9];
	int32 id;
	uint32 size;
	uint32 loopStart;
	uint32 loopLen;
	uint16 rate;
	uint16 rateOffs;
	uint16 baseNote;
	int8 *data;
};

class TownsAudio_PcmChannel {
public:
	TownsAudio_PcmChannel();
	~TownsAudio_PcmChannel();

	void clear();

	void loadData(TownsAudio_WaveTable *w);
	void loadData(uint8 *buffer, uint32 size);

	int initInstrument(uint8 &note, TownsAudio_WaveTable *&tables, int numTables);
	void keyOn(uint8 note, uint8 velo, TownsAudio_WaveTable *w);
	void keyOff();

	void updateEnvelopeGenerator();

	void setInstrument(uint8 *instr);
	void setLevel(uint8 lvl);
	void setPitch(uint32 pt);
	void setBalance(uint8 blc);

	void updateOutput();
	int32 currentSampleLeft();
	int32 currentSampleRight();

	bool _keyPressed;
	bool _reserved;
	bool _activeKey;
	bool _activeEffect;
	bool _activeOutput;

private:
	void setupLoop(uint32 loopStart, uint32 len);
	void setNote(uint8 note, TownsAudio_WaveTable *w, bool stepLimit = false);
	void setVelo(uint8 velo);

	void envAttack();
	void envDecay();
	void envSustain();
	void envRelease();

	uint8 *_curInstrument;

	uint8 _note;

	uint8 _velo;
	uint8 _level;
	uint8 _tl;

	uint8 _panLeft;
	uint8 _panRight;

	int8 *_data;
	int8 *_dataEnd;

	int8 *_loopEnd;
	uint32 _loopLen;

	uint16 _stepNote;
	uint16 _stepPitch;
	uint16 _step;

	uint32 _pos;

	uint8 _envTotalLevel;
	uint8 _envAttackRate;
	uint8 _envDecayRate;
	uint8 _envSustainLevel;
	uint8 _envSustainRate;
	uint8 _envReleaseRate;
	int16 _envStep;
	int16 _envCurrentLevel;

	EnvelopeState _envState;

	int8 *_extData;

	static const uint16 _pcmPhase1[];
	static const uint16 _pcmPhase2[];
};

class TownsAudioInterfaceInternal : public TownsPC98_FmSynth {
private:
	TownsAudioInterfaceInternal(Audio::Mixer *mixer, TownsAudioInterface *owner, TownsAudioInterfacePluginDriver *driver, bool externalMutexHandling = false);
public:
	~TownsAudioInterfaceInternal();

	static TownsAudioInterfaceInternal *addNewRef(Audio::Mixer *mixer, TownsAudioInterface *owner, TownsAudioInterfacePluginDriver *driver, bool externalMutexHandling = false);
	static void releaseRef(TownsAudioInterface *owner);

	bool init();

	int callback(int command, ...);
	int processCommand(int command, va_list &args);

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);
	// Defines the channels used as sound effect channels for the purpose of ScummVM GUI volume control.
	// The first 6 bits are the 6 fm channels. The next 8 bits are pcm channels.
	void setSoundEffectChanMask(int mask);

private:
	bool assignPluginDriver(TownsAudioInterface *owner, TownsAudioInterfacePluginDriver *driver, bool externalMutexHandling = false);
	void removePluginDriver(TownsAudioInterface *owner);

	void nextTickEx(int32 *buffer, uint32 bufferSize);

	void timerCallbackA();
	void timerCallbackB();

	typedef int (TownsAudioInterfaceInternal::*TownsAudioIntfCallback)(va_list &);
	const TownsAudioIntfCallback *_intfOpcodes;

	int intf_reset(va_list &args);
	int intf_keyOn(va_list &args);
	int intf_keyOff(va_list &args);
	int intf_setPanPos(va_list &args);
	int intf_setInstrument(va_list &args);
	int intf_loadInstrument(va_list &args);
	int intf_setPitch(va_list &args);
	int intf_setLevel(va_list &args);
	int intf_chanOff(va_list &args);
	int intf_writeReg(va_list &args);
	int intf_writeRegBuffer(va_list &args);
	int intf_readRegBuffer(va_list &args);
	int intf_setTimerA(va_list &args);
	int intf_setTimerB(va_list &args);
	int intf_enableTimerA(va_list &args);
	int intf_enableTimerB(va_list &args);
	int intf_loadSamples(va_list &args);
	int intf_reserveEffectChannels(va_list &args);
	int intf_loadWaveTable(va_list &args);
	int intf_unloadWaveTable(va_list &args);
	int intf_pcmPlayEffect(va_list &args);
	int intf_pcmChanOff(va_list &args);
	int intf_pcmEffectPlaying(va_list &args);
	int intf_fmKeyOn(va_list &args);
	int intf_fmKeyOff(va_list &args);
	int intf_fmSetPanPos(va_list &args);
	int intf_fmSetInstrument(va_list &args);
	int intf_fmLoadInstrument(va_list &args);
	int intf_fmSetPitch(va_list &args);
	int intf_fmSetLevel(va_list &args);
	int intf_fmReset(va_list &args);
	int intf_setOutputVolume(va_list &args);
	int intf_resetOutputVolume(va_list &args);
	int intf_getOutputVolume(va_list &args);
	int intf_setOutputMute(va_list &args);
	int intf_cdaToggle(va_list &args);
	int intf_getOutputVolume2(va_list &args);
	int intf_getOutputMute(va_list &args);
	int intf_pcmUpdateEnvelopeGenerator(va_list &args);

	int intf_notImpl(va_list &args);

	void fmReset();
	int fmKeyOn(int chan, int note, int velo);
	int fmKeyOff(int chan);
	int fmChanOff(int chan);
	int fmSetPanPos(int chan, int mode);
	int fmSetInstrument(int chan, int instrId);
	int fmLoadInstrument(int instrId, const uint8 *data);
	int fmSetPitch(int chan, int pitch);
	int fmSetLevel(int chan, int lvl);

	void bufferedWriteReg(uint8 part, uint8 regAddress, uint8 value);

	uint8 _fmChanPlaying;
	uint8 _fmChanNote[6];
	int16 _fmChanPitch[6];

	uint8 *_fmSaveReg[2];
	uint8 *_fmInstruments;

	void pcmReset();
	int pcmKeyOn(int chan, int note, int velo);
	int pcmKeyOff(int chan);
	int pcmChanOff(int chan);
	int pcmSetPanPos(int chan, int mode);
	int pcmSetInstrument(int chan, int instrId);
	int pcmLoadInstrument(int instrId, const uint8 *data);
	int pcmSetPitch(int chan, int pitch);
	int pcmSetLevel(int chan, int lvl);

	TownsAudio_PcmChannel *_pcmChan;

	uint8 _numReservedChannels;
	uint8 *_pcmInstruments;

	TownsAudio_WaveTable *_waveTables;
	uint8 _numWaveTables;
	uint32 _waveTablesTotalDataSize;

	void updateOutputVolume();
	void updateOutputVolumeInternal();
	uint8 _outputVolumeFlags;
	uint8 _outputLevel[16];
	uint8 _outputMute[16];
	bool _updateOutputVol;

	const float _baserate;
	uint32 _timerBase;
	uint32 _tickLength;
	uint32 _timer;

	uint16 _musicVolume;
	uint16 _sfxVolume;
	int _pcmSfxChanMask;

	TownsAudioInterfacePluginDriver *_drv;
	void *_drvOwner;
	bool _ready;

	static TownsAudioInterfaceInternal *_refInstance;
	static int _refCount;

	static const uint8 _chanFlags[];
	static const uint16 _frequency[];
	static const uint8 _carrier[];
	static const uint8 _fmDefaultInstrument[];
};

TownsAudioInterfaceInternal::TownsAudioInterfaceInternal(Audio::Mixer *mixer, TownsAudioInterface *owner, TownsAudioInterfacePluginDriver *driver, bool externalMutexHandling) :
	TownsPC98_FmSynth(mixer, kTypeTowns, externalMutexHandling),
	_fmInstruments(0), _pcmInstruments(0), _pcmChan(0), _waveTables(0), _waveTablesTotalDataSize(0),
	_baserate(55125.0f / (float)mixer->getOutputRate()), _tickLength(0), _timer(0), _drv(driver), _drvOwner(owner),
	_pcmSfxChanMask(0),	_musicVolume(Audio::Mixer::kMaxMixerVolume), _sfxVolume(Audio::Mixer::kMaxMixerVolume),
	_outputVolumeFlags(0), _fmChanPlaying(0),
	_numReservedChannels(0), _numWaveTables(0), _updateOutputVol(false), _ready(false) {

#define INTCB(x) &TownsAudioInterfaceInternal::intf_##x
	static const TownsAudioIntfCallback intfCb[] = {
		// 0
		INTCB(reset),
		INTCB(keyOn),
		INTCB(keyOff),
		INTCB(setPanPos),
		// 4
		INTCB(setInstrument),
		INTCB(loadInstrument),
		INTCB(notImpl),
		INTCB(setPitch),
		// 8
		INTCB(setLevel),
		INTCB(chanOff),
		INTCB(notImpl),
		INTCB(notImpl),
		// 12
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 16
		INTCB(notImpl),
		INTCB(writeReg),
		INTCB(notImpl),
		INTCB(writeRegBuffer),
		// 20
		INTCB(readRegBuffer),
		INTCB(setTimerA),
		INTCB(setTimerB),
		INTCB(enableTimerA),
		// 24
		INTCB(enableTimerB),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 28
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 32
		INTCB(loadSamples),
		INTCB(reserveEffectChannels),
		INTCB(loadWaveTable),
		INTCB(unloadWaveTable),
		// 36
		INTCB(notImpl),
		INTCB(pcmPlayEffect),
		INTCB(notImpl),
		INTCB(pcmChanOff),
		// 40
		INTCB(pcmEffectPlaying),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 44
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 48
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(fmKeyOn),
		INTCB(fmKeyOff),
		// 52
		INTCB(fmSetPanPos),
		INTCB(fmSetInstrument),
		INTCB(fmLoadInstrument),
		INTCB(notImpl),
		// 56
		INTCB(fmSetPitch),
		INTCB(fmSetLevel),
		INTCB(fmReset),
		INTCB(notImpl),
		// 60
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 64
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(setOutputVolume),
		// 68
		INTCB(resetOutputVolume),
		INTCB(getOutputVolume),
		INTCB(setOutputMute),
		INTCB(notImpl),
		// 72
		INTCB(notImpl),
		INTCB(cdaToggle),
		INTCB(getOutputVolume2),
		INTCB(getOutputMute),
		// 76
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 80
		INTCB(pcmUpdateEnvelopeGenerator),
		INTCB(notImpl)
	};
#undef INTCB

	_intfOpcodes = intfCb;

	memset(_fmSaveReg, 0, sizeof(_fmSaveReg));
	memset(_fmChanNote, 0, sizeof(_fmChanNote));
	memset(_fmChanPitch, 0, sizeof(_fmChanPitch));
	memset(_outputLevel, 0, sizeof(_outputLevel));
	memset(_outputMute, 0, sizeof(_outputMute));

	_timerBase = (uint32)(_baserate * 1000000.0f);
	_tickLength = 2 * _timerBase;
}

TownsAudioInterfaceInternal::~TownsAudioInterfaceInternal() {
	_ready = false;
	deinit();

	Common::StackLock lock(_mutex);

	delete[] _fmSaveReg[0];
	delete[] _fmSaveReg[1];
	delete[] _fmInstruments;
	delete[] _pcmInstruments;
	delete[] _waveTables;
	delete[] _pcmChan;
}

TownsAudioInterfaceInternal *TownsAudioInterfaceInternal::addNewRef(Audio::Mixer *mixer, TownsAudioInterface *owner, TownsAudioInterfacePluginDriver *driver, bool externalMutexHandling) {
	_refCount++;
	if (_refCount == 1 && _refInstance == 0)
		_refInstance = new TownsAudioInterfaceInternal(mixer, owner, driver, externalMutexHandling);
	else if (_refCount < 2 || _refInstance == 0)
		error("TownsAudioInterfaceInternal::addNewRef(): Internal reference management failure");
	else if (!_refInstance->assignPluginDriver(owner, driver, externalMutexHandling))
		error("TownsAudioInterfaceInternal::addNewRef(): Plugin driver conflict");

	return _refInstance;
}

void TownsAudioInterfaceInternal::releaseRef(TownsAudioInterface *owner) {
	if (!_refCount)
		return;

	_refCount--;

	if (_refCount) {
		if (_refInstance)
			_refInstance->removePluginDriver(owner);
	} else {
		delete _refInstance;
		_refInstance = 0;
	}
}

bool TownsAudioInterfaceInternal::init() {
	if (_ready)
		return true;

	if (!TownsPC98_FmSynth::init())
		return false;

	_fmSaveReg[0] = new uint8[256];
	_fmSaveReg[1] = new uint8[256];
	_fmInstruments = new uint8[128 * 48];
	_pcmInstruments = new uint8[32 * 128];
	_waveTables = new TownsAudio_WaveTable[128];
	_pcmChan = new TownsAudio_PcmChannel[8];

	_timer = 0;

	setVolumeChannelMasks(-1, 0);

	_ready = true;
	callback(0);

	return true;
}

int TownsAudioInterfaceInternal::callback(int command, ...) {
	if (!_ready)
		return 1;

	va_list args;
	va_start(args, command);

	int res = processCommand(command, args);

	va_end(args);
	return res;
}

int TownsAudioInterfaceInternal::processCommand(int command, va_list &args) {
	if (!_ready)
		return 1;

	if (command < 0 || command > 81)
		return 4;

	Common::StackLock lock(_mutex);
	int res = (this->*_intfOpcodes[command])(args);

	return res;
}

void TownsAudioInterfaceInternal::setMusicVolume(int volume) {
	_musicVolume = CLIP<uint16>(volume, 0, Audio::Mixer::kMaxMixerVolume);
	setVolumeIntern(_musicVolume, _sfxVolume);
}

void TownsAudioInterfaceInternal::setSoundEffectVolume(int volume) {
	_sfxVolume = CLIP<uint16>(volume, 0, Audio::Mixer::kMaxMixerVolume);
	setVolumeIntern(_musicVolume, _sfxVolume);
}

void TownsAudioInterfaceInternal::setSoundEffectChanMask(int mask) {
	_pcmSfxChanMask = mask >> 6;
	mask &= 0x3f;
	setVolumeChannelMasks(~mask, mask);
}

bool TownsAudioInterfaceInternal::assignPluginDriver(TownsAudioInterface *owner, TownsAudioInterfacePluginDriver *driver, bool externalMutexHandling) {
	if (_refCount <= 1)
		return true;

	if (_drv) {
		if (driver && driver != _drv)
			return false;
	} else {
		Common::StackLock lock(_mutex);
		_drv = driver;
		_drvOwner = owner;
		_externalMutex = externalMutexHandling;
	}

	return true;
}

void TownsAudioInterfaceInternal::removePluginDriver(TownsAudioInterface *owner) {
	if (_drvOwner == owner) {
		Common::StackLock lock(_mutex);
		_drv = 0;
	}
}

void TownsAudioInterfaceInternal::nextTickEx(int32 *buffer, uint32 bufferSize) {
	if (!_ready)
		return;

	if (_updateOutputVol)
		updateOutputVolumeInternal();

	for (uint32 i = 0; i < bufferSize; i++) {
		_timer += _tickLength;
		while (_timer > 0x514767) {
			_timer -= 0x514767;

			for (int ii = 0; ii < 8; ii++)
				_pcmChan[ii].updateOutput();
		}

		int32 finOutL = 0;
		int32 finOutR = 0;

		for (int ii = 0; ii < 8; ii++) {
			if (_pcmChan[ii]._activeOutput) {
				int32 oL = _pcmChan[ii].currentSampleLeft();
				int32 oR = _pcmChan[ii].currentSampleRight();
				if ((1 << ii) & (~_pcmSfxChanMask)) {
					oL = (oR * _musicVolume) / Audio::Mixer::kMaxMixerVolume;
					oR = (oR * _musicVolume) / Audio::Mixer::kMaxMixerVolume;
				}
				if ((1 << ii) & _pcmSfxChanMask) {
					oL = (oL * _sfxVolume) / Audio::Mixer::kMaxMixerVolume;
					oR = (oR * _sfxVolume) / Audio::Mixer::kMaxMixerVolume;
				}
				finOutL += oL;
				finOutR += oR;

				if (!(_pcmChan[ii]._activeKey || _pcmChan[ii]._activeEffect))
					_pcmChan[ii]._activeOutput = false;
			}
		}

		buffer[i << 1] += finOutL;
		buffer[(i << 1) + 1] += finOutR;
	}
}

void TownsAudioInterfaceInternal::timerCallbackA() {
	if (_drv && _ready)
		_drv->timerCallback(0);
}

void TownsAudioInterfaceInternal::timerCallbackB() {
	if (_ready) {
		if (_drv)
			_drv->timerCallback(1);
		callback(80);
	}
}

int TownsAudioInterfaceInternal::intf_reset(va_list &args) {
	fmReset();
	pcmReset();
	callback(68);
	return 0;
}

int TownsAudioInterfaceInternal::intf_keyOn(va_list &args) {
	int chan = va_arg(args, int);
	int note = va_arg(args, int);
	int velo = va_arg(args, int);
	return (chan & 0x40) ? pcmKeyOn(chan, note, velo) : fmKeyOn(chan, note, velo);
}

int TownsAudioInterfaceInternal::intf_keyOff(va_list &args) {
	int chan = va_arg(args, int);
	return (chan & 0x40) ? pcmKeyOff(chan) : fmKeyOff(chan);
}

int TownsAudioInterfaceInternal::intf_setPanPos(va_list &args) {
	int chan = va_arg(args, int);
	int mode = va_arg(args, int);
	return (chan & 0x40) ? pcmSetPanPos(chan, mode) : fmSetPanPos(chan, mode);
}

int TownsAudioInterfaceInternal::intf_setInstrument(va_list &args) {
	int chan = va_arg(args, int);
	int instrId = va_arg(args, int);
	return (chan & 0x40) ? pcmSetInstrument(chan, instrId) : fmSetInstrument(chan, instrId);
}

int TownsAudioInterfaceInternal::intf_loadInstrument(va_list &args) {
	int chanType = va_arg(args, int);
	int instrId = va_arg(args, int);
	uint8 *instrData = va_arg(args, uint8 *);
	return (chanType & 0x40) ? pcmLoadInstrument(instrId, instrData) : fmLoadInstrument(instrId, instrData);
}

int TownsAudioInterfaceInternal::intf_setPitch(va_list &args) {
	int chan = va_arg(args, int);
	int16 pitch = (int16)(va_arg(args, int) & 0xffff);
	return (chan & 0x40) ? pcmSetPitch(chan, pitch) : fmSetPitch(chan, pitch);
}

int TownsAudioInterfaceInternal::intf_setLevel(va_list &args) {
	int chan = va_arg(args, int);
	int lvl = va_arg(args, int);
	return (chan & 0x40) ? pcmSetLevel(chan, lvl) : fmSetLevel(chan, lvl);
}

int TownsAudioInterfaceInternal::intf_chanOff(va_list &args) {
	int chan = va_arg(args, int);
	return (chan & 0x40) ? pcmChanOff(chan) : fmChanOff(chan);
}

int TownsAudioInterfaceInternal::intf_writeReg(va_list &args) {
	int part = va_arg(args, int) ? 1 : 0;
	int reg = va_arg(args, int);
	int val = va_arg(args, int);
	if ((!part && reg < 0x20) || (part && reg < 0x30) || (reg > 0xb6))
		return 3;

	bufferedWriteReg(part, reg, val);
	return 0;
}

int TownsAudioInterfaceInternal::intf_writeRegBuffer(va_list &args) {
	int part = va_arg(args, int) ? 1 : 0;
	int reg = va_arg(args, int);
	int val = va_arg(args, int);

	if ((!part && reg < 0x20) || (part && reg < 0x30) || (reg > 0xef))
		return 3;

	_fmSaveReg[part][reg] = val;
	return 0;
}

int TownsAudioInterfaceInternal::intf_readRegBuffer(va_list &args) {
	int part = va_arg(args, int) ? 1 : 0;
	int reg = va_arg(args, int);
	uint8 *dst = va_arg(args, uint8 *);
	*dst = 0;

	if ((!part && reg < 0x20) || (part && reg < 0x30) || (reg > 0xef))
		return 3;

	*dst = _fmSaveReg[part][reg];
	return 0;
}

int TownsAudioInterfaceInternal::intf_setTimerA(va_list &args) {
	int enable = va_arg(args, int);
	int tempo = va_arg(args, int);

	if (enable) {
		bufferedWriteReg(0, 0x25, tempo & 3);
		bufferedWriteReg(0, 0x24, (tempo >> 2) & 0xff);
		bufferedWriteReg(0, 0x27, _fmSaveReg[0][0x27] | 0x05);
	} else {
		bufferedWriteReg(0, 0x27, (_fmSaveReg[0][0x27] & 0xfa) | 0x10);
	}

	return 0;
}

int TownsAudioInterfaceInternal::intf_setTimerB(va_list &args) {
	int enable = va_arg(args, int);
	int tempo = va_arg(args, int);

	if (enable) {
		bufferedWriteReg(0, 0x26, tempo & 0xff);
		bufferedWriteReg(0, 0x27, _fmSaveReg[0][0x27] | 0x0A);
	} else {
		bufferedWriteReg(0, 0x27, (_fmSaveReg[0][0x27] & 0xf5) | 0x20);
	}

	return 0;
}

int TownsAudioInterfaceInternal::intf_enableTimerA(va_list &args) {
	bufferedWriteReg(0, 0x27, _fmSaveReg[0][0x27] | 0x15);
	return 0;
}

int TownsAudioInterfaceInternal::intf_enableTimerB(va_list &args) {
	bufferedWriteReg(0, 0x27, _fmSaveReg[0][0x27] | 0x2a);
	return 0;
}

int TownsAudioInterfaceInternal::intf_loadSamples(va_list &args) {
	uint32 dest = va_arg(args, uint32);
	int size = va_arg(args, int);
	uint8 *src = va_arg(args, uint8*);

	if (dest >= 65536 || size == 0 || size > 65536)
		return 3;
	if (size + dest > 65536)
		return 5;

	int dwIndex = _numWaveTables - 1;
	for (uint32 t = _waveTablesTotalDataSize; dwIndex && (dest < t); dwIndex--)
		t -= _waveTables[dwIndex].size;

	TownsAudio_WaveTable *s = &_waveTables[dwIndex];
	_waveTablesTotalDataSize -= s->size;
	s->size = size;
	s->readData(src);
	_waveTablesTotalDataSize += s->size;

	return 0;
}

int TownsAudioInterfaceInternal::intf_reserveEffectChannels(va_list &args) {
	int numChan = va_arg(args, int);
	if (numChan > 8)
		return 3;
	if ((numChan << 13) + _waveTablesTotalDataSize > 65536)
		return 5;

	if (numChan == _numReservedChannels)
		return 0;

	if (numChan < _numReservedChannels) {
		int c = 8 - _numReservedChannels;
		for (int i = numChan; i; i--)
			_pcmChan[c--]._activeEffect = false;
	} else {
		int c = 7 - _numReservedChannels;
		for (int i = numChan - _numReservedChannels; i; i--) {
			_pcmChan[c]._keyPressed = false;
			_pcmChan[c--]._activeKey = false;
		}
	}

	_numReservedChannels = numChan;
	for (int i = 0; i < 8; i++)
		_pcmChan[i]._reserved = i >= (8 - _numReservedChannels) ? true : false;

	return 0;
}

int TownsAudioInterfaceInternal::intf_loadWaveTable(va_list &args) {
	uint8 *data = va_arg(args, uint8 *);
	if (_numWaveTables > 127)
		return 3;

	TownsAudio_WaveTable w;
	w.readHeader(data);
	if (!w.size)
		return 6;

	if (_waveTablesTotalDataSize + w.size > 65504)
		return 5;

	for (int i = 0; i < _numWaveTables; i++) {
		if (_waveTables[i].id == w.id)
			return 10;
	}

	TownsAudio_WaveTable *s = &_waveTables[_numWaveTables++];
	s->readHeader(data);

	_waveTablesTotalDataSize += s->size;
	callback(32, _waveTablesTotalDataSize, s->size, data + 32);

	return 0;
}

int TownsAudioInterfaceInternal::intf_unloadWaveTable(va_list &args) {
	int id = va_arg(args, int);

	if (id == -1) {
		for (int i = 0; i < 128; i++)
			_waveTables[i].clear();
		_numWaveTables = 0;
		_waveTablesTotalDataSize = 0;
	} else {
		if (_waveTables) {
			for (int i = 0; i < _numWaveTables; i++) {
				if (_waveTables[i].id == id) {
					_numWaveTables--;
					_waveTablesTotalDataSize -= _waveTables[i].size;
					_waveTables[i].clear();
					for (; i < _numWaveTables; i++)
						memcpy(&_waveTables[i], &_waveTables[i + 1], sizeof(TownsAudio_WaveTable));
					return 0;
				}
				return 9;
			}
		}
	}

	return 0;
}

int TownsAudioInterfaceInternal::intf_pcmPlayEffect(va_list &args) {
	int chan = va_arg(args, int);
	int note = va_arg(args, int);
	int velo = va_arg(args, int);
	uint8 *data = va_arg(args, uint8 *);

	if (chan < 0x40 || chan > 0x47)
		return 1;

	if (note & 0x80 || velo & 0x80)
		return 3;

	chan -= 0x40;

	if (!_pcmChan[chan]._reserved)
		return 7;

	if (_pcmChan[chan]._activeEffect)
		return 2;

	TownsAudio_WaveTable w;
	w.readHeader(data);

	if (w.size < (w.loopStart + w.loopLen))
		return 13;

	if (!w.size)
		return 6;

	TownsAudio_PcmChannel *p = &_pcmChan[chan];

	p->loadData(data + 32, w.size);
	p->keyOn(note, velo, &w);

	return 0;
}

int TownsAudioInterfaceInternal::intf_pcmChanOff(va_list &args) {
	int chan = va_arg(args, int);
	pcmChanOff(chan);
	return 0;
}

int TownsAudioInterfaceInternal::intf_pcmEffectPlaying(va_list &args) {
	int chan = va_arg(args, int);
	if (chan < 0x40 || chan > 0x47)
		return 1;
	chan -= 0x40;
	return _pcmChan[chan]._activeEffect ? 1 : 0;
}

int TownsAudioInterfaceInternal::intf_fmKeyOn(va_list &args) {
	int chan = va_arg(args, int);
	int note = va_arg(args, int);
	int velo = va_arg(args, int);
	return fmKeyOn(chan, note, velo);
}

int TownsAudioInterfaceInternal::intf_fmKeyOff(va_list &args) {
	int chan = va_arg(args, int);
	return fmKeyOff(chan);
}

int TownsAudioInterfaceInternal::intf_fmSetPanPos(va_list &args) {
	int chan = va_arg(args, int);
	int mode = va_arg(args, int);
	return fmSetPanPos(chan, mode);
}

int TownsAudioInterfaceInternal::intf_fmSetInstrument(va_list &args) {
	int chan = va_arg(args, int);
	int instrId = va_arg(args, int);
	return fmSetInstrument(chan, instrId);
}

int TownsAudioInterfaceInternal::intf_fmLoadInstrument(va_list &args) {
	int instrId = va_arg(args, int);
	uint8 *instrData = va_arg(args, uint8 *);
	return fmLoadInstrument(instrId, instrData);
}

int TownsAudioInterfaceInternal::intf_fmSetPitch(va_list &args) {
	int chan = va_arg(args, int);
	uint16 freq = va_arg(args, int) & 0xffff;
	return fmSetPitch(chan, freq);
}

int TownsAudioInterfaceInternal::intf_fmSetLevel(va_list &args) {
	int chan = va_arg(args, int);
	int lvl = va_arg(args, int);
	return fmSetLevel(chan, lvl);
}

int TownsAudioInterfaceInternal::intf_fmReset(va_list &args) {
	fmReset();
	return 0;
}

int TownsAudioInterfaceInternal::intf_setOutputVolume(va_list &args) {
	int chanType = va_arg(args, int);
	int left = va_arg(args, int);
	int right = va_arg(args, int);

	if (left & 0xff80 || right & 0xff80)
		return 3;

	static const uint8 flags[] = { 0x0C, 0x30, 0x40, 0x80 };

	uint8 chan = (chanType & 0x40) ? 8 : 12;

	chanType &= 3;
	left = (left & 0x7e) >> 1;
	right = (right & 0x7e) >> 1;

	if (chan == 12)
		_outputVolumeFlags |= flags[chanType];
	else
		_outputVolumeFlags &= ~flags[chanType];

	if (chanType > 1) {
		_outputLevel[chan + chanType] = left;
		_outputMute[chan + chanType] = 0;
	} else {
		if (chanType == 0)
			chan -= 8;
		_outputLevel[chan] = left;
		_outputLevel[chan + 1] = right;
		_outputMute[chan] = _outputMute[chan + 1] = 0;
	}

	updateOutputVolume();

	return 0;
}

int TownsAudioInterfaceInternal::intf_resetOutputVolume(va_list &args) {
	memset(_outputLevel, 0, sizeof(_outputLevel));
	_outputVolumeFlags = 0;
	updateOutputVolume();
	return 0;
}

int TownsAudioInterfaceInternal::intf_getOutputVolume(va_list &args) {
	int chanType = va_arg(args, int);
	int *left = va_arg(args, int*);
	int *right = va_arg(args, int*);

	uint8 chan = (chanType & 0x40) ? 8 : 12;
	chanType &= 3;

	if (chanType > 1) {
		*left = _outputLevel[chan + chanType] & 0x3f;
	} else {
		if (chanType == 0)
			chan -= 8;
		*left = _outputLevel[chan] & 0x3f;
		*right = _outputLevel[chan + 1] & 0x3f;
	}

	return 0;
}

int TownsAudioInterfaceInternal::intf_setOutputMute(va_list &args) {
	int flags = va_arg(args, int);
	_outputVolumeFlags = flags;
	uint8 mute = flags & 3;
	uint8 f = flags & 0xff;

	memset(_outputMute, 1, 8);
	if (mute & 2)
		memset(&_outputMute[12], 1, 4);
	if (mute & 1)
		memset(&_outputMute[8], 1, 4);

	_outputMute[(f < 0x80) ? 11 : 15] = 0;
	f += f;
	_outputMute[(f < 0x80) ? 10 : 14] = 0;
	f += f;
	_outputMute[(f < 0x80) ? 8 : 12] = 0;
	f += f;
	_outputMute[(f < 0x80) ? 9 : 13] = 0;
	f += f;
	_outputMute[(f < 0x80) ? 0 : 4] = 0;
	f += f;
	_outputMute[(f < 0x80) ? 1 : 5] = 0;
	f += f;

	updateOutputVolume();
	return 0;
}

int TownsAudioInterfaceInternal::intf_cdaToggle(va_list &args) {
	//int mode = va_arg(args, int);
	//_unkMask = mode ? 0x7f : 0x3f;
	return 0;
}

int TownsAudioInterfaceInternal::intf_getOutputVolume2(va_list &args) {
	return 0;
}

int TownsAudioInterfaceInternal::intf_getOutputMute (va_list &args) {
	return 0;
}

int TownsAudioInterfaceInternal::intf_pcmUpdateEnvelopeGenerator(va_list &args) {
	for (int i = 0; i < 8; i++)
		_pcmChan[i].updateEnvelopeGenerator();
	return 0;
}

int TownsAudioInterfaceInternal::intf_notImpl(va_list &args) {
	return 4;
}

void TownsAudioInterfaceInternal::fmReset() {
	TownsPC98_FmSynth::reset();

	_fmChanPlaying = 0;
	memset(_fmChanNote, 0, sizeof(_fmChanNote));
	memset(_fmChanPitch, 0, sizeof(_fmChanPitch));

	memset(_fmSaveReg[0], 0, 240);
	memset(&_fmSaveReg[0][240], 0x7f, 16);
	memset(_fmSaveReg[1], 0, 256);
	memset(&_fmSaveReg[1][240], 0x7f, 16);
	_fmSaveReg[0][243] = _fmSaveReg[0][247] = _fmSaveReg[0][251] = _fmSaveReg[0][255] = _fmSaveReg[1][243] = _fmSaveReg[1][247] = _fmSaveReg[1][251] = _fmSaveReg[1][255] = 0xff;

	for (int i = 0; i < 128; i++)
		fmLoadInstrument(i, _fmDefaultInstrument);

	bufferedWriteReg(0, 0x21, 0);
	bufferedWriteReg(0, 0x2C, 0x80);
	bufferedWriteReg(0, 0x2B, 0);
	bufferedWriteReg(0, 0x27, 0x30);

	for (int i = 0; i < 6; i++) {
		fmKeyOff(i);
		fmSetInstrument(i, 0);
		fmSetLevel(i, 127);
	}
}

int TownsAudioInterfaceInternal::fmKeyOn(int chan, int note, int velo) {
	if (chan > 5)
		return 1;
	if (note < 12 || note > 107 || (velo & 0x80))
		return 3;
	if (_fmChanPlaying & _chanFlags[chan])
		return 2;

	_fmChanPlaying |= _chanFlags[chan];
	note -= 12;

	_fmChanNote[chan] = note;
	int16 pitch = _fmChanPitch[chan];

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	int frq = 0;
	uint8 bl = 0;

	if (note) {
		frq = _frequency[(note - 1) % 12];
		bl = (note - 1) / 12;
	} else {
		frq = 616;
	}

	frq += pitch;

	if (frq < 616) {
		if (!bl) {
			frq = 616;
		} else {
			frq += 616;
			--bl;
		}
	} else if (frq > 1232) {
		if (bl == 7) {
			frq = 15500;
		} else {
			frq -= 616;
			++bl;
		}
	}

	frq |= (bl << 11);

	bufferedWriteReg(part, chan + 0xa4, (frq >> 8) & 0xff);
	bufferedWriteReg(part, chan + 0xa0, frq & 0xff);

	velo = (velo >> 2) + 96;
	uint16 c = _carrier[_fmSaveReg[part][0xb0 + chan] & 7];
	_fmSaveReg[part][0xe0 + chan] = velo;

	for (uint8 reg = 0x40 + chan; reg < 0x50; reg += 4) {
		c += c;
		if (c & 0x100) {
			c &= 0xff;
			bufferedWriteReg(part, reg, (((((((_fmSaveReg[part][0x80 + reg] ^ 0x7f) * velo) >> 7) + 1) * _fmSaveReg[part][0xd0 + chan]) >> 7) + 1) ^ 0x7f);
		}
	}

	uint8 v = chan;
	if (part)
		v |= 4;

	for (uint8 reg = 0x80 + chan; reg < 0x90; reg += 4)
		writeReg(part, reg, _fmSaveReg[part][reg] | 0x0f);

	writeReg(0, 0x28, v);

	for (uint8 reg = 0x80 + chan; reg < 0x90; reg += 4)
		writeReg(part, reg, _fmSaveReg[part][reg]);

	bufferedWriteReg(0, 0x28, v | 0xf0);

	return 0;
}

int TownsAudioInterfaceInternal::fmKeyOff(int chan) {
	if (chan > 5)
		return 1;
	_fmChanPlaying &= ~_chanFlags[chan];
	if (chan > 2)
		chan++;
	bufferedWriteReg(0, 0x28, chan);
	return 0;
}

int TownsAudioInterfaceInternal::fmChanOff(int chan) {
	if (chan > 5)
		return 1;
	_fmChanPlaying &= ~_chanFlags[chan];

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	for (uint8 reg = 0x80 + chan; reg < 0x90; reg += 4)
		writeReg(part, reg, _fmSaveReg[part][reg] | 0x0f);

	if (part)
		chan += 4;
	writeReg(0, 0x28, chan);
	return 0;
}

int TownsAudioInterfaceInternal::fmSetPanPos(int chan, int value) {
	if (chan > 5)
		return 1;

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	if (value > 0x40)
		value = 0x40;
	else if (value < 0x40)
		value = 0x80;
	else
		value = 0xC0;

	bufferedWriteReg(part, 0xb4 + chan, (_fmSaveReg[part][0xb4 + chan] & 0x3f) | value);
	return 0;
}

int TownsAudioInterfaceInternal::fmSetInstrument(int chan, int instrId) {
	if (chan > 5)
		return 1;
	if (instrId > 127)
		return 3;

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	uint8 *src = &_fmInstruments[instrId * 48 + 8];

	uint16 c = _carrier[src[24] & 7];
	uint8 reg = 0x30 + chan;

	for (; reg < 0x40; reg += 4)
		bufferedWriteReg(part, reg, *src++);

	for (; reg < 0x50; reg += 4) {
		uint8 v = *src++;
		_fmSaveReg[part][0x80 + reg] = _fmSaveReg[part][reg] = v;
		c += c;
		if (c & 0x100) {
			c &= 0xff;
			v = 127;
		}
		writeReg(part, reg, v);
	}

	for (; reg < 0x90; reg += 4)
		bufferedWriteReg(part, reg, *src++);

	reg += 0x20;
	bufferedWriteReg(part, reg, *src++);

	uint8 v = *src++;
	reg += 4;
	if (v < 64)
		v |= (_fmSaveReg[part][reg] & 0xc0);
	bufferedWriteReg(part, reg, v);

	return 0;
}

int TownsAudioInterfaceInternal::fmLoadInstrument(int instrId, const uint8 *data) {
	if (instrId > 127)
		return 3;
	assert(data);
	memcpy(&_fmInstruments[instrId * 48], data, 48);
	return 0;
}

int TownsAudioInterfaceInternal::fmSetPitch(int chan, int pitch) {
	if (chan > 5)
		return 1;

	uint8 bl = _fmChanNote[chan];
	int frq = 0;

	if (pitch < 0) {
		if (bl) {
			if (pitch < -8008)
				pitch = -8008;
			pitch *= -1;
			pitch /= 13;
			frq = _frequency[(bl - 1) % 12] - pitch;
			bl = (bl - 1) / 12;
			_fmChanPitch[chan] = -pitch;

			if (frq < 616) {
				if (bl) {
					frq += 616;
					bl--;
				} else {
					frq = 616;
					bl = 0;
				}
			}
		} else {
			frq = 616;
			bl = 0;
		}

	} else if (pitch > 0) {
		if (bl < 96) {
			if (pitch > 8008)
				pitch = 8008;
			pitch /= 13;

			if (bl) {
				frq = _frequency[(bl - 1) % 12] + pitch;
				bl = (bl - 1) / 12;
			} else {
				frq = 616;
				bl = 0;
			}

			_fmChanPitch[chan] = pitch;

			if (frq > 1232) {
				if (bl < 7) {
					frq -= 616;
					bl++;
				} else {
					frq = 1164;
					bl = 7;
				}
			} else {
				if (bl >= 7 && frq > 1164)
					frq = 1164;
			}

		} else {
			frq = 1164;
			bl = 7;
		}
	} else {
		_fmChanPitch[chan] = 0;
		if (bl) {
			frq = _frequency[(bl - 1) % 12];
			bl = (bl - 1) / 12;
		} else {
			frq = 616;
			bl = 0;
		}
	}

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	frq |= (bl << 11);

	bufferedWriteReg(part, chan + 0xa4, (frq >> 8));
	bufferedWriteReg(part, chan + 0xa0, (frq & 0xff));

	return 0;
}

int TownsAudioInterfaceInternal::fmSetLevel(int chan, int lvl) {
	if (chan > 5)
		return 1;
	if (lvl > 127)
		return 3;

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	uint16 c = _carrier[_fmSaveReg[part][0xb0 + chan] & 7];
	_fmSaveReg[part][0xd0 + chan] = lvl;

	for (uint8 reg = 0x40 + chan; reg < 0x50; reg += 4) {
		c += c;
		if (c & 0x100) {
			c &= 0xff;
			bufferedWriteReg(part, reg, (((((((_fmSaveReg[part][0x80 + reg] ^ 0x7f) * lvl) >> 7) + 1) * _fmSaveReg[part][0xe0 + chan]) >> 7) + 1) ^ 0x7f);
		}
	}
	return 0;
}

void TownsAudioInterfaceInternal::bufferedWriteReg(uint8 part, uint8 regAddress, uint8 value) {
	_fmSaveReg[part][regAddress] = value;
	writeReg(part, regAddress, value);
}

void TownsAudioInterfaceInternal::pcmReset() {
	_numReservedChannels = 0;

	for (int i = 0; i < 8; i++)
		_pcmChan[i].clear();

	memset(_pcmInstruments, 0, 128 * 32);
	static uint8 name[] = { 0x4E, 0x6F, 0x20, 0x44, 0x61, 0x74, 0x61, 0x21 };
	for (int i = 0; i < 32; i++)
		memcpy(_pcmInstruments + i * 128, name, 8);

	for (int i = 0; i < 128; i++)
		_waveTables[i].clear();
	_numWaveTables = 0;
	_waveTablesTotalDataSize = 0;

	for (int i = 0x40; i < 0x48; i++) {
		pcmSetInstrument(i, 0);
		pcmSetLevel(i, 127);
	}
}

int TownsAudioInterfaceInternal::pcmKeyOn(int chan, int note, int velo) {
	if (chan < 0x40 || chan > 0x47)
		return 1;

	if (note & 0x80 || velo & 0x80)
		return 3;

	chan -= 0x40;
	uint8 noteT = note;
	TownsAudio_PcmChannel *p = &_pcmChan[chan];

	if (p->_reserved || p->_keyPressed)
		return 2;

	TownsAudio_WaveTable *w = _waveTables;
	int res = p->initInstrument(noteT, w, _numWaveTables);
	if (res)
		return res;

	p->loadData(w);
	p->keyOn(noteT, velo, w);

	return 0;
}

int TownsAudioInterfaceInternal::pcmKeyOff(int chan) {
	if (chan < 0x40 || chan > 0x47)
		return 1;

	chan -= 0x40;
	_pcmChan[chan].keyOff();
	return 0;
}

int TownsAudioInterfaceInternal::pcmChanOff(int chan) {
	if (chan < 0x40 || chan > 0x47)
		return 1;

	chan -= 0x40;
	_pcmChan[chan]._keyPressed = _pcmChan[chan]._activeEffect = _pcmChan[chan]._activeKey = _pcmChan[chan]._activeOutput = false;

	return 0;
}

int TownsAudioInterfaceInternal::pcmSetPanPos(int chan, int mode) {
	if (chan > 0x47)
		return 1;
	if (mode & 0x80)
		return 3;

	chan -= 0x40;
	uint8 blc = 0x77;

	if (mode > 64) {
		mode -= 64;
		blc = ((blc ^ (mode >> 3)) + (mode << 4)) & 0xff;
	} else if (mode < 64) {
		mode = (mode >> 3) ^ 7;
		blc = ((119 + mode) ^ (mode << 4)) & 0xff;
	}

	_pcmChan[chan].setBalance(blc);

	return 0;
}

int TownsAudioInterfaceInternal::pcmSetInstrument(int chan, int instrId) {
	if (chan > 0x47)
		return 1;
	if (instrId > 31)
		return 3;
	chan -= 0x40;
	_pcmChan[chan].setInstrument(&_pcmInstruments[instrId * 128]);

	return 0;
}

int TownsAudioInterfaceInternal::pcmLoadInstrument(int instrId, const uint8 *data) {
	if (instrId > 31)
		return 3;
	assert(data);
	memcpy(&_pcmInstruments[instrId * 128], data, 128);
	return 0;
}

int TownsAudioInterfaceInternal::pcmSetPitch(int chan, int pitch) {
	if (chan > 0x47)
		return 1;

	if (pitch < -8192 || pitch > 8191)
		return 3;

	chan -= 0x40;
	TownsAudio_PcmChannel *p = &_pcmChan[chan];

	uint32 pts = 0x4000;

	if (pitch < 0)
		pts = (0x20000000 / (-pitch + 0x2001)) >> 2;
	else if (pitch > 0)
		pts = (((pitch + 0x2001) << 16) / 0x2000) >> 2;

	p->setPitch(pts);

	return 0;
}

int TownsAudioInterfaceInternal::pcmSetLevel(int chan, int lvl) {
	if (chan > 0x47)
		return 1;

	if (lvl & 0x80)
		return 3;

	chan -= 0x40;
	_pcmChan[chan].setLevel(lvl);

	return 0;
}

void TownsAudioInterfaceInternal::updateOutputVolume() {
	// Avoid calls to g_system->getAudioCDManager() functions from the main thread
	// since this can cause mutex lockups.
	_updateOutputVol = true;
}

void TownsAudioInterfaceInternal::updateOutputVolumeInternal() {
	if (!_ready)
		return;

	// FM Towns seems to support volumes of 0 - 63 for each channel.
	// We recalculate sane values for our 0 to 255 volume range and
	// balance values for our -128 to 127 volume range

	// CD-AUDIO
	uint32 maxVol = MAX(_outputLevel[12] * (_outputMute[12] ^ 1), _outputLevel[13] * (_outputMute[13] ^ 1));

	int volume = (int)(((float)(maxVol * 255) / 63.0f));
	int balance = maxVol ? (int)( ( ((int)_outputLevel[13] * (_outputMute[13] ^ 1) - _outputLevel[12] * (_outputMute[12] ^ 1)) * 127) / (float)maxVol) : 0;

	g_system->getAudioCDManager()->setVolume(volume);
	g_system->getAudioCDManager()->setBalance(balance);

	_updateOutputVol = false;
}

TownsAudioInterfaceInternal *TownsAudioInterfaceInternal::_refInstance = 0;

int TownsAudioInterfaceInternal::_refCount = 0;

const uint8 TownsAudioInterfaceInternal::_chanFlags[] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

const uint16 TownsAudioInterfaceInternal::_frequency[] = {
	0x028C, 0x02B4, 0x02DC, 0x030A, 0x0338, 0x0368, 0x039C, 0x03D4, 0x040E, 0x044A, 0x048C, 0x04D0
};

const uint8 TownsAudioInterfaceInternal::_carrier[] = {
	0x10, 0x10, 0x10, 0x10, 0x30, 0x70, 0x70, 0xF0
};

const uint8 TownsAudioInterfaceInternal::_fmDefaultInstrument[] = {
	0x45, 0x4C, 0x45, 0x50, 0x49, 0x41, 0x4E, 0x4F, 0x01, 0x0A, 0x02, 0x01,
	0x1E, 0x32, 0x05, 0x00, 0x9C, 0xDC, 0x9C, 0xDC, 0x07, 0x03, 0x14, 0x08,
	0x00, 0x03, 0x05, 0x05, 0x55, 0x45, 0x27, 0xA7, 0x04, 0xC0, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

TownsAudio_PcmChannel::TownsAudio_PcmChannel() {
	_extData = 0;
	clear();
}

TownsAudio_PcmChannel::~TownsAudio_PcmChannel() {
	clear();
}

void TownsAudio_PcmChannel::clear() {
	_curInstrument = 0;
	_note = _tl = _level = _velo = 0;

	_data = 0;
	_dataEnd = 0;
	_loopLen = 0;

	_pos = 0;
	_loopEnd = 0;

	_step = 0;
	_stepNote = 0x4000;
	_stepPitch = 0x4000;

	_panLeft = _panRight = 7;

	_envTotalLevel = _envAttackRate = _envDecayRate = _envSustainLevel = _envSustainRate = _envReleaseRate = 0;
	_envStep = _envCurrentLevel = 0;

	_envState = kEnvReady;

	_activeKey = _activeEffect = _activeOutput = _keyPressed = _reserved = false;

	delete[] _extData;
	_extData = 0;
}

void TownsAudio_PcmChannel::loadData(TownsAudio_WaveTable *w) {
	_data = w->data;
	_dataEnd = w->data + w->size;
}

void TownsAudio_PcmChannel::loadData(uint8 *buffer, uint32 size) {
	delete[] _extData;
	_extData = new int8[size];
	int8 *src = (int8 *)buffer;
	int8 *dst = _extData;
	for (uint32 i = 0; i < size; i++)
		*dst++ = *src & 0x80 ? (*src++ & 0x7f) : -*src++;

	_data = _extData;
	_dataEnd = _extData + size;
	_pos = 0;
}

int TownsAudio_PcmChannel::initInstrument(uint8 &note, TownsAudio_WaveTable *&tables, int numTables) {
	int i = 0;
	for (; i < 8; i++) {
		if (note <= _curInstrument[16 + 2 * i])
			break;
	}

	if (i == 8)
		return 8;

	uint8 *d = &_curInstrument[(i << 3) + 64];
	_envTotalLevel = d[0];
	_envAttackRate = d[1];
	_envDecayRate = d[2];
	_envSustainLevel = d[3];
	_envSustainRate = d[4];
	_envReleaseRate = d[5];
	_envStep = 0;
	note += d[6];

	int32 id = (int32)READ_LE_UINT32(&_curInstrument[i * 4 + 32]);

	for (i = 0; i < numTables; i++) {
		if (id == tables[i].id)
			break;
	}

	if (i == numTables)
		return 9;

	tables = &tables[i];
	return 0;
}

void TownsAudio_PcmChannel::keyOn(uint8 note, uint8 velo, TownsAudio_WaveTable *w) {
	setupLoop(w->loopStart, w->loopLen);
	setNote(note, w, _reserved);
	setVelo(velo);

	if (_reserved)
		_activeEffect = true;
	else
		_keyPressed = _activeKey = true;

	_activeOutput = true;
}

void TownsAudio_PcmChannel::keyOff() {
	_keyPressed = false;
	envRelease();
}

void TownsAudio_PcmChannel::updateEnvelopeGenerator() {
	if (!_envCurrentLevel) {
		_activeKey = false;
		_envState = kEnvReady;
	}

	if (!_activeKey)
		return;

	switch (_envState) {
	case kEnvAttacking:
		if (((_envCurrentLevel + _envStep) >> 8) > _envTotalLevel) {
			envDecay();
			return;
		} else {
			_envCurrentLevel += _envStep;
		}
		break;

	case kEnvDecaying:
		if (((_envCurrentLevel - _envStep) >> 8) < _envSustainLevel) {
			envSustain();
			return;
		} else {
			_envCurrentLevel -= _envStep;
		}
		break;

	case kEnvSustaining:
	case kEnvReleasing:
		_envCurrentLevel -= _envStep;
		if (_envCurrentLevel <= 0)
			_envCurrentLevel = 0;
		break;

	default:
		break;
	}
	_tl = (_envCurrentLevel >> 8) << 1;
}

void TownsAudio_PcmChannel::setInstrument(uint8 *instr) {
	_curInstrument = instr;
}

void TownsAudio_PcmChannel::setLevel(uint8 lvl) {
	if (_reserved) {
		_velo = lvl;
		_tl = lvl << 1;
	} else {
		int32 t = _envStep * lvl;
		if (_level)
			t /= _level;
		_envStep = t;
		t = _envCurrentLevel * lvl;
		if (_level)
			t /= _level;
		_envCurrentLevel = t;
		_level = lvl;
		_tl = _envCurrentLevel >> 8;
	}
}

void TownsAudio_PcmChannel::setPitch(uint32 pt) {
	_stepPitch = pt & 0xffff;
	_step = (_stepNote * _stepPitch) >> 14;

//	if (_pcmChanUnkFlag & _chanFlags[chan])
//		 unk[chan] = (((p->step * 1000) << 11) / 98) / 20833;

	/*else*/
	if (_activeEffect && (_step > 2048))
		_step = 2048;
}

void TownsAudio_PcmChannel::setBalance(uint8 blc) {
	_panLeft = blc & 0x0f;
	_panRight = blc >> 4;
}

void TownsAudio_PcmChannel::updateOutput() {
	if (_activeKey || _activeEffect) {
		_pos += _step;

		if (&_data[_pos >> 11] >= _loopEnd) {
			if (_loopLen) {
				_pos -= _loopLen;
			} else {
				_pos = 0;
				_activeKey = _activeEffect = false;
			}
		}
	}
}

int32 TownsAudio_PcmChannel::currentSampleLeft() {
	return (_activeOutput && _panLeft) ? (((_data[_pos >> 11] * _tl) * _panLeft) >> 3) : 0;
}

int32 TownsAudio_PcmChannel::currentSampleRight() {
	return (_activeOutput && _panRight) ? (((_data[_pos >> 11] * _tl) * _panRight) >> 3) : 0;
}

void TownsAudio_PcmChannel::setupLoop(uint32 loopStart, uint32 len) {
	_loopLen = len << 11;
	_loopEnd = _loopLen ? &_data[(loopStart + _loopLen) >> 11] : _dataEnd;
	_pos = loopStart;
}

void TownsAudio_PcmChannel::setNote(uint8 note, TownsAudio_WaveTable *w, bool stepLimit) {
	_note = note;
	int8 diff = _note - w->baseNote;
	uint16 r = w->rate + w->rateOffs;
	uint16 bl = 0;
	uint32 s = 0;

	if (diff < 0) {
		diff *= -1;
		bl = diff % 12;
		diff /= 12;
		s = (r >> diff);
		if (bl)
			s = (s * _pcmPhase2[bl]) >> 16;

	} else if (diff > 0) {
		bl = diff % 12;
		diff /= 12;
		s = (r << diff);
		if (bl)
			s += ((s * _pcmPhase1[bl]) >> 16);

	} else {
		s = r;
	}

	_stepNote = s & 0xffff;
	_step = (s * _stepPitch) >> 14;

	if (stepLimit && _step > 2048)
		_step = 2048;
}

void TownsAudio_PcmChannel::setVelo(uint8 velo) {
	if (_reserved) {
		_velo = velo;
		_tl = velo << 1;
	} else {
		_velo = velo;
		uint32 lvl = _level * _velo;
		_envTotalLevel = ((_envTotalLevel * lvl) >> 14) & 0xff;
		_envSustainLevel = ((_envSustainLevel * lvl) >> 14) & 0xff;
		envAttack();
		_tl = (_envCurrentLevel >> 8) << 1;
	}
}

void TownsAudio_PcmChannel::envAttack() {
	_envState = kEnvAttacking;
	int16 t = _envTotalLevel << 8;
	if (_envAttackRate == 127) {
		_envCurrentLevel = _envStep = 0;
	} else if (_envAttackRate) {
		_envStep = t / _envAttackRate;
		_envCurrentLevel = 1;
	} else {
		_envCurrentLevel = t;
		envDecay();
	}
}

void TownsAudio_PcmChannel::envDecay() {
	_envState = kEnvDecaying;
	int16 t = _envTotalLevel - _envSustainLevel;
	if (t < 0 || _envDecayRate == 127) {
		_envStep = 0;
	} else if (_envDecayRate) {
		_envStep = (t << 8) / _envDecayRate;
	} else {
		_envCurrentLevel = _envSustainLevel << 8;
		envSustain();
	}
}

void TownsAudio_PcmChannel::envSustain() {
	_envState = kEnvSustaining;
	if (_envSustainLevel && _envSustainRate)
		_envStep = (_envSustainRate == 127) ? 0 : (_envCurrentLevel / _envSustainRate) >> 1;
	else
		_envStep = _envCurrentLevel = 1;
}

void TownsAudio_PcmChannel::envRelease() {
	_envState = kEnvReleasing;
	if (_envReleaseRate == 127)
		_envStep = 0;
	else if (_envReleaseRate)
		_envStep = _envCurrentLevel / _envReleaseRate;
	else
		_envStep = _envCurrentLevel = 1;
}

const uint16 TownsAudio_PcmChannel::_pcmPhase1[] =  {
	0x879B, 0x0F37, 0x1F58, 0x306E, 0x4288, 0x55B6, 0x6A08, 0x7F8F, 0x965E, 0xAE88, 0xC882, 0xE341
};

const uint16 TownsAudio_PcmChannel::_pcmPhase2[] =  {
	0xFEFE, 0xF1A0, 0xE411, 0xD744, 0xCB2F, 0xBFC7, 0xB504, 0xAAE2, 0xA144, 0x9827, 0x8FAC
};

TownsAudio_WaveTable::TownsAudio_WaveTable() {
	data = 0;
	clear();
}

TownsAudio_WaveTable::~TownsAudio_WaveTable() {
	clear();
}

void TownsAudio_WaveTable::readHeader(const uint8 *buffer) {
	memcpy(name, buffer, 8);
	name[8] = 0;
	id = READ_LE_UINT32(&buffer[8]);
	size = READ_LE_UINT32(&buffer[12]);
	loopStart = READ_LE_UINT32(&buffer[16]);
	loopLen = READ_LE_UINT32(&buffer[20]);
	rate = READ_LE_UINT16(&buffer[24]);
	rateOffs = READ_LE_UINT16(&buffer[26]);
	baseNote = READ_LE_UINT32(&buffer[28]);
}

void TownsAudio_WaveTable::readData(const uint8 *buffer) {
	if (!size)
		return;

	delete[] data;
	data = new int8[size];

	const int8 *src = (const int8 *)buffer;
	int8 *dst = data;
	for (uint32 i = 0; i < size; i++)
		*dst++ = *src & 0x80 ? (*src++ & 0x7f) : -*src++;
}

void TownsAudio_WaveTable::clear() {
	name[0] = name[8] = 0;
	id = -1;
	size = 0;
	loopStart = 0;
	loopLen = 0;
	rate = 0;
	rateOffs = 0;
	baseNote = 0;
	delete[] data;
	data = 0;
}

TownsAudioInterface::TownsAudioInterface(Audio::Mixer *mixer, TownsAudioInterfacePluginDriver *driver, bool externalMutexHandling) {
	_intf = TownsAudioInterfaceInternal::addNewRef(mixer, this, driver, externalMutexHandling);
}

TownsAudioInterface::~TownsAudioInterface() {
	TownsAudioInterfaceInternal::releaseRef(this);
	_intf = 0;
}

bool TownsAudioInterface::init() {
	return _intf->init();
}

int TownsAudioInterface::callback(int command, ...) {
	va_list args;
	va_start(args, command);

	int res = _intf->processCommand(command, args);

	va_end(args);
	return res;
}

void TownsAudioInterface::setMusicVolume(int volume) {
	_intf->setMusicVolume(volume);
}

void TownsAudioInterface::setSoundEffectVolume(int volume) {
	_intf->setSoundEffectVolume(volume);
}

void TownsAudioInterface::setSoundEffectChanMask(int mask) {
	_intf->setSoundEffectChanMask(mask);
}
