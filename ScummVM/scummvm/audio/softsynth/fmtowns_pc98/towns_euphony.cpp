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

#include "audio/softsynth/fmtowns_pc98/towns_euphony.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/textconsole.h"

TownsEuphonyDriver::TownsEuphonyDriver(Audio::Mixer *mixer) : _activeChannels(0), _sustainChannels(0),
	_assignedChannels(0), _paraCount(0), _command(0), _tEnable(0), _tMode(0), _tOrdr(0), _tLevel(0),
	_tTranspose(0), _musicPos(0), _musicStart(0), _playing(false), _eventBuffer(0), _bufferedEventsCount(0),
	_tempoControlMode(0), _timerSetting(0), _tempoDiff(0), _timeStampBase(0), _elapsedEvents(0), _loop(false),
	_endOfTrack(false), _suspendParsing(false), _musicTrackSize(0) {
	_para[0] = _para[1] = 0;
	_intf = new TownsAudioInterface(mixer, this);
	resetTempo();
}

TownsEuphonyDriver::~TownsEuphonyDriver() {
	delete _intf;
	delete[] _activeChannels;
	delete[] _sustainChannels;
	delete[] _assignedChannels;
	delete[] _eventBuffer;
	delete[] _tEnable;
	delete[] _tMode;
	delete[] _tOrdr;
	delete[] _tLevel;
	delete[] _tTranspose;
}

bool TownsEuphonyDriver::init() {
	if (!_intf->init())
		return false;

	delete[] _activeChannels;
	delete[] _sustainChannels;
	delete[] _assignedChannels;
	delete[] _eventBuffer;
	delete[] _tEnable;
	delete[] _tMode;
	delete[] _tOrdr;
	delete[] _tLevel;
	delete[] _tTranspose;

	_activeChannels = new int8[16];
	_sustainChannels = new int8[16];
	_assignedChannels = new ActiveChannel[128];
	_eventBuffer = new DlEvent[64];

	_tEnable = new uint8[32];
	_tMode = new uint8[32];
	_tOrdr = new uint8[32];
	_tLevel = new int8[32];
	_tTranspose = new int8[32];

	reset();

	return true;
}

void TownsEuphonyDriver::reset() {
	_intf->callback(0);

	_intf->callback(74);
	_intf->callback(70, 0);
	_intf->callback(75, 3);

	setTimerA(true, 1);
	setTimerA(false, 1);
	setTimerB(true, 221);

	_paraCount = _command = _para[0] = _para[1] = 0;
	memset(_sustainChannels, 0, 16);
	memset(_activeChannels, -1, 16);
	for (int i = 0; i < 128; i++) {
		_assignedChannels[i].chan = _assignedChannels[i].next = -1;
		_assignedChannels[i].note = _assignedChannels[i].sub = 0;
	}

	int e = 0;
	for (int i = 0; i < 6; i++)
		assignChannel(i, e++);
	for (int i = 0x40; i < 0x48; i++)
		assignChannel(i, e++);

	resetTables();

	memset(_eventBuffer, 0, 64 * sizeof(DlEvent));
	_bufferedEventsCount = 0;

	_playing = _endOfTrack = _suspendParsing = _loop = false;
	_elapsedEvents = 0;
	_tempoDiff = 0;

	resetTempo();

	if (_tempoControlMode == 1) {
		//if (///)
		//  return;
		setTempoIntern(_defaultTempo);
	} else {
		setTempoIntern(_defaultTempo);
	}

	resetControl();
}

void TownsEuphonyDriver::loadInstrument(int chanType, int id, const uint8 *data) {
	_intf->callback(5, chanType, id, data);
}

void TownsEuphonyDriver::loadWaveTable(const uint8 *data) {
	_intf->callback(34, data);
}

void TownsEuphonyDriver::unloadWaveTable(int id) {
	_intf->callback(35, id);
}

void TownsEuphonyDriver::reserveSoundEffectChannels(int num) {
	_intf->callback(33, num);
	uint32 volMask = 0;

	if (num > 8)
		return;

	for (uint32 v = 1 << 13; num; num--) {
		volMask |= v;
		v >>= 1;
	}

	_intf->setSoundEffectChanMask(volMask);
}

int TownsEuphonyDriver::setMusicTempo(int tempo) {
	if (tempo > 250)
		return 3;
	_defaultTempo = tempo;
	_trackTempo = tempo;
	setTempoIntern(tempo);
	return 0;
}

int TownsEuphonyDriver::startMusicTrack(const uint8 *data, int trackSize, int startTick) {
	if (_playing)
		return 2;

	_musicPos = _musicStart = data;
	_defaultBaseTickLen = _baseTickLen = startTick;
	_musicTrackSize = trackSize;
	_timeStampBase = _timeStampDest = 0;
	_tickCounter = 0;
	_playing = true;

	return 0;
}

void TownsEuphonyDriver::setMusicLoop(bool loop) {
	_loop = loop;
}

void TownsEuphonyDriver::stopParser() {
	if (_playing) {
		_playing = false;
		_pulseCount = 0;
		_endOfTrack = false;
		flushEventBuffer();
		resetControl();
	}
}

void TownsEuphonyDriver::continueParsing() {
	_suspendParsing = false;
}

void TownsEuphonyDriver::playSoundEffect(int chan, int note, int velo, const uint8 *data) {
	_intf->callback(37, chan, note, velo, data);
}

void TownsEuphonyDriver::stopSoundEffect(int chan) {
	_intf->callback(39, chan);
}

bool TownsEuphonyDriver::soundEffectIsPlaying(int chan) {
	return _intf->callback(40, chan) ? true : false;
}

void TownsEuphonyDriver::chanPanPos(int chan, int mode) {
	_intf->callback(3, chan, mode);
}

void TownsEuphonyDriver::chanPitch(int chan, int pitch) {
	_intf->callback(7, chan, pitch);
}

void TownsEuphonyDriver::chanVolume(int chan, int vol) {
	_intf->callback(8, chan, vol);
}

void TownsEuphonyDriver::setOutputVolume(int mode, int volLeft, int volRight) {
	_intf->callback(67, mode, volLeft, volRight);
}

int TownsEuphonyDriver::configChan_enable(int tableEntry, int val) {
	if (tableEntry > 31)
		return 3;
	_tEnable[tableEntry] = val;
	return 0;
}

int TownsEuphonyDriver::configChan_setMode(int tableEntry, int val) {
	if (tableEntry > 31)
		return 3;
	_tMode[tableEntry] = val;
	return 0;
}

int TownsEuphonyDriver::configChan_remap(int tableEntry, int val) {
	if (tableEntry > 31)
		return 3;
	if (val < 16)
		_tOrdr[tableEntry] = val;
	return 0;
}

int TownsEuphonyDriver::configChan_adjustVolume(int tableEntry, int val) {
	if (tableEntry > 31)
		return 3;
	if (val <= 40)
		_tLevel[tableEntry] = (int8)(val & 0xff);
	return 0;
}

int TownsEuphonyDriver::configChan_setTranspose(int tableEntry, int val) {
	if (tableEntry > 31)
		return 3;
	if (val <= 40)
		_tTranspose[tableEntry] = (int8)(val & 0xff);
	return 0;
}

int TownsEuphonyDriver::assignChannel(int chan, int tableEntry) {
	if (tableEntry > 15 || chan > 127 || chan < 0)
		return 3;

	ActiveChannel *a = &_assignedChannels[chan];
	if (a->chan == tableEntry)
		return 0;

	if (a->chan != -1) {
		int8 *b = &_activeChannels[a->chan];
		while (*b != chan) {
			b = &_assignedChannels[*b].next;
			if (*b == -1 && *b != chan)
				return 3;
		}

		*b = a->next;

		if (a->note)
			_intf->callback(2, chan);

		a->chan = a->next = -1;
		a->note = 0;
	}

	a->next = _activeChannels[tableEntry];
	_activeChannels[tableEntry] = chan;
	a->chan = tableEntry;
	a->note = a->sub = 0;

	return 0;
}

void TownsEuphonyDriver::timerCallback(int timerId) {
	switch (timerId) {
	case 0:
		updatePulseCount();
		while (_pulseCount > 0) {
			--_pulseCount;
			updateTimeStampBase();
			if (!_playing)
				continue;
			updateEventBuffer();
			updateParser();
			updateCheckEot();
		}
		break;
	default:
		break;
	}
}

void TownsEuphonyDriver::setMusicVolume(int volume) {
	_intf->setMusicVolume(volume);
}

void TownsEuphonyDriver::setSoundEffectVolume(int volume) {
	_intf->setSoundEffectVolume(volume);
}

void TownsEuphonyDriver::resetTables() {
	memset(_tEnable, 0xff, 32);
	memset(_tMode, 0xff, 16);
	memset(_tMode + 16, 0, 16);
	for (int i = 0; i < 32; i++)
		_tOrdr[i] = i & 0x0f;
	memset(_tLevel, 0, 32);
	memset(_tTranspose, 0, 32);
}

void TownsEuphonyDriver::resetTempo() {
	_defaultBaseTickLen = _baseTickLen = 0x33;
	_pulseCount = 0;
	_extraTimingControlRemainder = 0;
	_extraTimingControl = 16;
	_tempoModifier = 0;
	_timeStampDest = 0;
	_deltaTicks = 0;
	_tickCounter = 0;
	_defaultTempo = 90;
	_trackTempo = 90;
}

void TownsEuphonyDriver::setTempoIntern(int tempo) {
	tempo = CLIP(tempo + _tempoModifier, 0, 500);
	if (_tempoControlMode == 0) {
		_timerSetting = 34750 / (tempo + 30);
		_extraTimingControl = 16;

		while (_timerSetting < 126) {
			_timerSetting <<= 1;
			_extraTimingControl <<= 1;
		}

		while (_timerSetting > 383) {
			_timerSetting >>= 1;
			_extraTimingControl >>= 1;
		}

		setTimerA(true, -(_timerSetting - 2));

	} else if (_tempoControlMode == 1) {
		_timerSetting = 312500 / (tempo + 30);
		_extraTimingControl = 16;
		while (_timerSetting < 1105) {
			_timerSetting <<= 1;
			_extraTimingControl <<= 1;
		}

	} else if (_tempoControlMode == 2) {
		_timerSetting = 625000 / (tempo + 30);
		_extraTimingControlRemainder = 0;
	}
}

void TownsEuphonyDriver::setTimerA(bool enable, int tempo) {
	_intf->callback(21, enable ? 255 : 0, tempo);
}

void TownsEuphonyDriver::setTimerB(bool enable, int tempo) {
	_intf->callback(22, enable ? 255 : 0, tempo);
}

void TownsEuphonyDriver::updatePulseCount() {
	int tc = _extraTimingControl + _extraTimingControlRemainder;
	_extraTimingControlRemainder = tc & 0x0f;
	tc >>= 4;
	_tempoDiff -= tc;

	while (_tempoDiff < 0) {
		_elapsedEvents++;
		_tempoDiff += 4;
	}

	if (_playing && !_suspendParsing)
		_pulseCount += tc;
}

void TownsEuphonyDriver::updateTimeStampBase() {
	static const uint16 table[] = { 0x180, 0xC0, 0x80, 0x60, 0x40, 0x30, 0x20, 0x18 };
	if ((uint32)(table[_baseTickLen >> 4] * ((_baseTickLen & 0x0f) + 1)) > ++_tickCounter)
		return;
	++_timeStampDest;
	_tickCounter = 0;
	_deltaTicks = 0;
}

void TownsEuphonyDriver::updateParser() {
	for (bool loop = true; loop;) {
		uint8 cmd = _musicPos[0];

		if (cmd == 0xff || cmd == 0xf7) {
			jumpNextLoop();

		} else if (cmd < 0x90) {
			_endOfTrack = true;
			flushEventBuffer();
			loop = false;

		} else if (_timeStampBase > _timeStampDest) {
			loop = false;

		} else {
			if (_timeStampBase == _timeStampDest) {
				uint16 timeStamp = READ_LE_UINT16(&_musicPos[2]);
				uint8 l = (timeStamp & 0xff) + (timeStamp & 0xff);
				timeStamp = ((timeStamp & 0xff00) | l) >> 1;
				if (timeStamp > _tickCounter)
					loop = false;
			}

			if (loop) {
				if (parseNext())
					loop = false;
			}
		}
	}
}

void TownsEuphonyDriver::updateCheckEot() {
	if (!_endOfTrack || _bufferedEventsCount)
		return;
	stopParser();
}

bool TownsEuphonyDriver::parseNext() {
#define OPC(x) &TownsEuphonyDriver::evt##x
	static const EuphonyOpcode opcodes[] = {
		OPC(NotImpl),
		OPC(SetupNote),
		OPC(PolyphonicAftertouch),
		OPC(ControlPitch),
		OPC(InstrumentChanAftertouch),
		OPC(InstrumentChanAftertouch),
		OPC(ControlPitch)
	};
#undef OPC

	uint cmd = _musicPos[0];
	if (cmd != 0xfe && cmd != 0xfd) {
		if (cmd >= 0xf0) {
			cmd &= 0x0f;
			if (cmd == 0)
				evtLoadInstrument();
			else if (cmd == 2)
				evtAdvanceTimestampOffset();
			else if (cmd == 8)
				evtTempo();
			else if (cmd == 12)
				evtModeOrdrChange();
			jumpNextLoop();
			return false;

		} else if (!(this->*opcodes[(cmd - 0x80) >> 4])()) {
			jumpNextLoop();
			return false;
		}
	}

	if (cmd == 0xfd) {
		_suspendParsing = true;
		return true;
	}

	if (!_loop) {
		_endOfTrack = true;
		return true;
	}

	_endOfTrack = false;
	_musicPos = _musicStart;
	_timeStampBase = _timeStampDest = _tickCounter = 0;
	_baseTickLen = _defaultBaseTickLen;

	return false;
}

void TownsEuphonyDriver::jumpNextLoop() {
	_musicPos += 6;
	if (_musicPos >= _musicStart + _musicTrackSize)
		_musicPos = _musicStart;
}

void TownsEuphonyDriver::updateEventBuffer() {
	DlEvent *e = _eventBuffer;
	for (int i = _bufferedEventsCount; i; e++) {
		if (e->evt == 0)
			continue;
		if (--e->len) {
			--i;
			continue;
		}
		processBufferNote(e->mode, e->evt, e->note, e->velo);
		e->evt = 0;
		--i;
		--_bufferedEventsCount;
	}
}

void TownsEuphonyDriver::flushEventBuffer() {
	DlEvent *e = _eventBuffer;
	for (int i = _bufferedEventsCount; i; e++) {
		if (e->evt == 0)
			continue;
		processBufferNote(e->mode, e->evt, e->note, e->velo);
		e->evt = 0;
		--i;
		--_bufferedEventsCount;
	}
}

void TownsEuphonyDriver::processBufferNote(int mode, int evt, int note, int velo) {
	if (!velo)
		evt &= 0x8f;
	sendEvent(mode, evt);
	sendEvent(mode, note);
	sendEvent(mode, velo);
}

void TownsEuphonyDriver::resetControl() {
	for (int i = 0; i < 32; i++) {
		if (_tOrdr[i] > 15) {
			for (int ii = 0; ii < 16; ii++)
				resetControlIntern(_tMode[i], ii);
		} else {
			resetControlIntern(_tMode[i], _tOrdr[i]);
		}
	}
}

void TownsEuphonyDriver::resetControlIntern(int mode, int chan) {
	sendEvent(mode, 0xb0 | chan);
	sendEvent(mode, 0x40);
	sendEvent(mode, 0);
	sendEvent(mode, 0xb0 | chan);
	sendEvent(mode, 0x7b);
	sendEvent(mode, 0);
	sendEvent(mode, 0xb0 | chan);
	sendEvent(mode, 0x79);
	sendEvent(mode, 0x40);
}

uint8 TownsEuphonyDriver::appendEvent(uint8 evt, uint8 chan) {
	if (evt >= 0x80 && evt < 0xf0 && _tOrdr[chan] < 16)
		return (evt & 0xf0) | _tOrdr[chan];
	return evt;
}

void TownsEuphonyDriver::sendEvent(uint8 mode, uint8 command) {
	if (mode == 0) {
		// warning("TownsEuphonyDriver: Mode 0 not implemented");

	} else if (mode == 0x10) {
		warning("TownsEuphonyDriver: Mode 0x10 not implemented");

	} else if (mode == 0xff) {
		if (command >= 0xf0) {
			_paraCount = 1;
			_command = 0;
		} else if (command >= 0x80) {
			_paraCount = 1;
			_command = command;
		} else if (_command >= 0x80) {
			switch ((_command - 0x80) >> 4) {
			case 0:
				if (_paraCount < 2) {
					_paraCount++;
					_para[0] = command;
				} else {
					_paraCount = 1;
					_para[1] = command;
					sendNoteOff();
				}
				break;

			case 1:
				if (_paraCount < 2) {
					_paraCount++;
					_para[0] = command;
				} else {
					_paraCount = 1;
					_para[1] = command;
					if (command)
						sendNoteOn();
					else
						sendNoteOff();
				}
				break;

			case 2:
				if (_paraCount < 2) {
					_paraCount++;
					_para[0] = command;
				} else {
					_paraCount = 1;
				}
				break;

			case 3:
				if (_paraCount < 2) {
					_paraCount++;
					_para[0] = command;
				} else {
					_paraCount = 1;
					_para[1] = command;

					if (_para[0] == 7)
						sendChanVolume();
					else if (_para[0] == 10)
						sendPanPosition();
					else if (_para[0] == 64)
						sendAllNotesOff();
				}
				break;

			case 4:
				_paraCount = 1;
				_para[0] = command;
				sendSetInstrument();
				break;

			case 5:
				_paraCount = 1;
				_para[0] = command;
				break;

			case 6:
				if (_paraCount < 2) {
					_paraCount++;
					_para[0] = command;
				} else {
					_paraCount = 1;
					_para[1] = command;
					sendPitch();
				}
				break;
			}
		}
	}
}

bool TownsEuphonyDriver::evtSetupNote() {
	if (_musicPos[1] > 31)
		return false;
	if (!_tEnable[_musicPos[1]]) {
		jumpNextLoop();
		return (_musicPos[0] == 0xfe || _musicPos[0] == 0xfd) ? true : false;
	}
	uint8 evt = appendEvent(_musicPos[0], _musicPos[1]);
	uint8 mode = _tMode[_musicPos[1]];
	uint8 note = _musicPos[4];
	uint8 velo = _musicPos[5];

	sendEvent(mode, evt);
	sendEvent(mode, applyTranspose(note));
	sendEvent(mode, applyVolumeAdjust(velo));

	jumpNextLoop();
	if (_musicPos[0] == 0xfe || _musicPos[0] == 0xfd)
		return true;

	velo = _musicPos[5];
	uint16 len = ((((_musicPos[1] << 4) | (_musicPos[2] << 8)) >> 4) & 0xff) | ((((_musicPos[3] << 4) | (_musicPos[4] << 8)) >> 4) << 8);

	int i = 0;
	for (; i < 64; i++) {
		if (_eventBuffer[i].evt == 0)
			break;
	}

	if (i == 64) {
		processBufferNote(mode, evt, note, velo);
	} else {
		_eventBuffer[i].evt = evt;
		_eventBuffer[i].mode = mode;
		_eventBuffer[i].note = note;
		_eventBuffer[i].velo = velo;
		_eventBuffer[i].len = len ? len : 1;
		_bufferedEventsCount++;
	}

	return false;
}

bool TownsEuphonyDriver::evtPolyphonicAftertouch() {
	if (_musicPos[1] > 31)
		return false;
	if (!_tEnable[_musicPos[1]])
		return false;

	uint8 evt = appendEvent(_musicPos[0], _musicPos[1]);
	uint8 mode = _tMode[_musicPos[1]];

	sendEvent(mode, evt);
	sendEvent(mode, applyTranspose(_musicPos[4]));
	sendEvent(mode, _musicPos[5]);

	return false;
}

bool TownsEuphonyDriver::evtControlPitch() {
	if (_musicPos[1] > 31)
		return false;
	if (!_tEnable[_musicPos[1]])
		return false;

	uint8 evt = appendEvent(_musicPos[0], _musicPos[1]);
	uint8 mode = _tMode[_musicPos[1]];

	sendEvent(mode, evt);
	sendEvent(mode, _musicPos[4]);
	sendEvent(mode, _musicPos[5]);

	return false;
}

bool TownsEuphonyDriver::evtInstrumentChanAftertouch() {
	if (_musicPos[1] > 31)
		return false;
	if (!_tEnable[_musicPos[1]])
		return false;

	uint8 evt = appendEvent(_musicPos[0], _musicPos[1]);
	uint8 mode = _tMode[_musicPos[1]];

	sendEvent(mode, evt);
	sendEvent(mode, _musicPos[4]);

	return false;
}

bool TownsEuphonyDriver::evtLoadInstrument() {
	return false;
}

bool TownsEuphonyDriver::evtAdvanceTimestampOffset() {
	++_timeStampBase;
	_baseTickLen = _musicPos[1];
	return false;
}

bool TownsEuphonyDriver::evtTempo() {
	uint8 l = _musicPos[4] << 1;
	_trackTempo = (l | (_musicPos[5] << 8)) >> 1;
	setTempoIntern(_trackTempo);
	return false;
}

bool TownsEuphonyDriver::evtModeOrdrChange() {
	if (_musicPos[1] > 31)
		return false;
	if (!_tEnable[_musicPos[1]])
		return false;

	if (_musicPos[4] == 1)
		_tMode[_musicPos[1]] = _musicPos[5];
	else if (_musicPos[4] == 2)
		_tOrdr[_musicPos[1]] = _musicPos[5];

	return false;
}

uint8 TownsEuphonyDriver::applyTranspose(uint8 in) {
	int out = _tTranspose[_musicPos[1]];
	if (!out)
		return in;
	out += (in & 0x7f);

	if (out > 127)
		out -= 12;

	if (out < 0)
		out += 12;

	return out & 0xff;
}

uint8 TownsEuphonyDriver::applyVolumeAdjust(uint8 in) {
	int out = _tLevel[_musicPos[1]];
	out += (in & 0x7f);
	out = CLIP(out, 1, 127);

	return out & 0xff;
}

void TownsEuphonyDriver::sendNoteOff() {
	int8 *chan = &_activeChannels[_command & 0x0f];
	if (*chan == -1)
		return;

	while (_assignedChannels[*chan].note != _para[0]) {
		chan = &_assignedChannels[*chan].next;
		if (*chan == -1)
			return;
	}

	if (_sustainChannels[_command & 0x0f]) {
		_assignedChannels[*chan].note |= 0x80;
	} else {
		_assignedChannels[*chan].note = 0;
		_intf->callback(2, *chan);
	}
}

void TownsEuphonyDriver::sendNoteOn() {
	if (!_para[0])
		return;
	int8 *chan = &_activeChannels[_command & 0x0f];
	if (*chan == -1)
		return;

	do {
		_assignedChannels[*chan].sub++;
		chan = &_assignedChannels[*chan].next;
	} while (*chan != -1);

	chan = &_activeChannels[_command & 0x0f];

	int d = 0;
	int c = 0;
	bool found = false;

	do {
		if (!_assignedChannels[*chan].note) {
			found = true;
			break;
		}
		if (d <= _assignedChannels[*chan].sub) {
			c = *chan;
			d = _assignedChannels[*chan].sub;
		}
		chan = &_assignedChannels[*chan].next;
	} while (*chan != -1);

	if (found)
		c = *chan;
	else
		_intf->callback(2, c);

	_assignedChannels[c].note = _para[0];
	_assignedChannels[c].sub = 0;
	_intf->callback(1, c, _para[0], _para[1]);
}

void TownsEuphonyDriver::sendChanVolume() {
	int8 *chan = &_activeChannels[_command & 0x0f];
	while (*chan != -1) {
		_intf->callback(8, *chan, _para[1] & 0x7f);
		chan = &_assignedChannels[*chan].next;
	}
}

void TownsEuphonyDriver::sendPanPosition() {
	int8 *chan = &_activeChannels[_command & 0x0f];
	while (*chan != -1) {
		_intf->callback(3, *chan, _para[1] & 0x7f);
		chan = &_assignedChannels[*chan].next;
	}
}

void TownsEuphonyDriver::sendAllNotesOff() {
	if (_para[1] > 63) {
		_sustainChannels[_command & 0x0f] = -1;
		return;
	}

	_sustainChannels[_command & 0x0f] = 0;
	int8 *chan = &_activeChannels[_command & 0x0f];
	while (*chan != -1) {
		if (_assignedChannels[*chan].note & 0x80) {
			_assignedChannels[*chan].note = 0;
			_intf->callback(2, *chan);
		}
		chan = &_assignedChannels[*chan].next;
	}
}

void TownsEuphonyDriver::sendSetInstrument() {
	int8 *chan = &_activeChannels[_command & 0x0f];
	while (*chan != -1) {
		_intf->callback(4, *chan, _para[0]);
		_intf->callback(7, *chan, 0);
		chan = &_assignedChannels[*chan].next;
	}
}

void TownsEuphonyDriver::sendPitch() {
	int8 *chan = &_activeChannels[_command & 0x0f];
	while (*chan != -1) {
		_para[0] += _para[0];
		int16 pitch = (((READ_LE_UINT16(_para)) >> 1) & 0x3fff) - 0x2000;
		_intf->callback(7, *chan, pitch);
		chan = &_assignedChannels[*chan].next;
	}
}
