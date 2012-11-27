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

#include "sci/sci.h"

#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/softsynth/fmtowns_pc98/towns_audio.h"

#include "sci/resource.h"
#include "sci/sound/drivers/mididriver.h"

namespace Sci {

class MidiDriver_FMTowns;

class TownsChannel {
public:
	TownsChannel(MidiDriver_FMTowns *driver, uint8 id);
	~TownsChannel() {}

	void noteOff();
	void noteOn(uint8 note, uint8 velo);
	void pitchBend(int16 val);
	void updateVolume();
	void updateDuration();

	uint8 _assign;
	uint8 _note;
	uint8 _sustain;
	uint16 _duration;

private:
	uint8 _id;
	uint8 _velo;
	uint8 _program;

	MidiDriver_FMTowns *_drv;
};

class TownsMidiPart {
friend class MidiDriver_FMTowns;
public:
	TownsMidiPart(MidiDriver_FMTowns *driver, uint8 id);
	~TownsMidiPart() {}

	void noteOff(uint8 note);
	void noteOn(uint8 note, uint8 velo);
	void controlChangeVolume(uint8 vol);
	void controlChangeSustain(uint8 sus);
	void controlChangePolyphony(uint8 numChan);
	void controlChangeAllNotesOff();
	void programChange(uint8 prg);
	void pitchBend(int16 val);

	void addChannels(int num);
	void dropChannels(int num);

	uint8 currentProgram() const;

private:
	int allocateChannel();

	uint8 _id;
	uint8 _program;
	uint8 _volume;
	uint8 _sustain;
	uint8 _chanMissing;
	int16 _pitchBend;
	uint8 _outChan;

	MidiDriver_FMTowns *_drv;
};

class MidiDriver_FMTowns : public MidiDriver, public TownsAudioInterfacePluginDriver {
friend class TownsChannel;
friend class TownsMidiPart;
public:
	MidiDriver_FMTowns(Audio::Mixer *mixer, SciVersion version);
	~MidiDriver_FMTowns();

	int open();
	void loadInstruments(const uint8 *data);
	bool isOpen() const { return _isOpen; }
	void close();

	void send(uint32 b);

	uint32 property(int prop, uint32 param);
	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc);

	void setSoundOn(bool toggle);

	uint32 getBaseTempo();
	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }

	uint8 currentProgram();

	void timerCallback(int timerId);

private:
	int getChannelVolume(uint8 midiPart);
	void addMissingChannels();

	void updateParser();
	void updateChannels();

	Common::TimerManager::TimerProc _timerProc;
	void *_timerProcPara;

	TownsMidiPart **_parts;
	TownsChannel **_out;

	uint8 _masterVolume;

	bool _soundOn;

	bool _isOpen;
	bool _ready;

	const uint16 _baseTempo;
	SciVersion _version;

	TownsAudioInterface *_intf;
};

class MidiPlayer_FMTowns : public MidiPlayer {
public:
	MidiPlayer_FMTowns(SciVersion version);
	~MidiPlayer_FMTowns();

	int open(ResourceManager *resMan);

	bool hasRhythmChannel() const;
	byte getPlayId() const;
	int getPolyphony() const;
	void playSwitch(bool play);

private:
	MidiDriver_FMTowns *_townsDriver;
};

TownsChannel::TownsChannel(MidiDriver_FMTowns *driver, uint8 id) : _drv(driver), _id(id), _assign(0xff), _note(0xff), _velo(0), _sustain(0), _duration(0), _program(0xff) {
}

void TownsChannel::noteOn(uint8 note, uint8 velo) {
	_duration = 0;

	if (_drv->_version != SCI_VERSION_1_EARLY) {
		if (_program != _drv->_parts[_assign]->currentProgram() && _drv->_soundOn) {
			_program = _drv->_parts[_assign]->currentProgram();
			_drv->_intf->callback(4, _id, _program);
		}
	}

	_note = note;
	_velo = velo;
	_drv->_intf->callback(1, _id, _note, _velo);
}

void TownsChannel::noteOff() {
	if (_sustain)
		return;

	_drv->_intf->callback(2, _id);
	_note = 0xff;
	_duration = 0;
}

void TownsChannel::pitchBend(int16 val) {
	_drv->_intf->callback(7, _id, val);
}

void TownsChannel::updateVolume() {
	if (_assign > 15 && _drv->_version != SCI_VERSION_1_EARLY)
		return;
	_drv->_intf->callback(8, _id, _drv->getChannelVolume((_drv->_version == SCI_VERSION_1_EARLY) ? 0 : _assign));
}

void TownsChannel::updateDuration() {
	if (_note != 0xff)
		_duration++;
}

TownsMidiPart::TownsMidiPart(MidiDriver_FMTowns *driver, uint8 id) : _drv(driver), _id(id), _program(0), _volume(0x3f), _sustain(0), _chanMissing(0), _pitchBend(0x2000), _outChan(0) {
}

void TownsMidiPart::noteOff(uint8 note) {
	for (int i = 0; i < 6; i++) {
		if ((_drv->_out[i]->_assign != _id && _drv->_version != SCI_VERSION_1_EARLY) || _drv->_out[i]->_note != note)
			continue;
		if (_sustain)
			_drv->_out[i]->_sustain = 1;
		else
			_drv->_out[i]->noteOff();
		return;
	}
}

void TownsMidiPart::noteOn(uint8 note, uint8 velo) {
	if (note < 12 || note > 107)
		return;

	if (velo == 0) {
		noteOff(note);
		return;
	}

	if (_drv->_version != SCI_VERSION_1_EARLY)
		velo >>= 1;

	for (int i = 0; i < 6; i++) {
		if ((_drv->_out[i]->_assign != _id && _drv->_version != SCI_VERSION_1_EARLY) || _drv->_out[i]->_note != note)
			continue;
		_drv->_out[i]->_sustain = 0;
		_drv->_out[i]->noteOff();
		_drv->_out[i]->noteOn(note, velo);
		return;
	}

	int chan = allocateChannel();
	if (chan != -1)
		_drv->_out[chan]->noteOn(note, velo);
}

void TownsMidiPart::controlChangeVolume(uint8 vol) {
	if (_drv->_version == SCI_VERSION_1_EARLY)
		return;

	_volume = vol >> 1;
	for (int i = 0; i < 6; i++) {
		if (_drv->_out[i]->_assign == _id)
			_drv->_out[i]->updateVolume();
	}
}

void TownsMidiPart::controlChangeSustain(uint8 sus) {
	if (_drv->_version == SCI_VERSION_1_EARLY)
		return;

	_sustain = sus;
	if (_sustain)
		return;

	for (int i = 0; i < 6; i++) {
		if (_drv->_out[i]->_assign == _id && _drv->_out[i]->_sustain) {
			_drv->_out[i]->_sustain = 0;
			_drv->_out[i]->noteOff();
		}
	}
}

void TownsMidiPart::controlChangePolyphony(uint8 numChan) {
	if (_drv->_version == SCI_VERSION_1_EARLY)
		return;

	uint8 numAssigned = 0;
	for (int i = 0; i < 6; i++) {
		if (_drv->_out[i]->_assign == _id)
			numAssigned++;
	}

	numAssigned += _chanMissing;
	if (numAssigned < numChan) {
		addChannels(numChan - numAssigned);
	} else if (numAssigned > numChan) {
		dropChannels(numAssigned - numChan);
		_drv->addMissingChannels();
	}
}

void TownsMidiPart::controlChangeAllNotesOff() {
	for (int i = 0; i < 6; i++) {
		if ((_drv->_out[i]->_assign == _id || _drv->_version == SCI_VERSION_1_EARLY) && _drv->_out[i]->_note != 0xff)
			_drv->_out[i]->noteOff();
	}
}

void TownsMidiPart::programChange(uint8 prg) {
	_program = prg;
}

void TownsMidiPart::pitchBend(int16 val) {
	_pitchBend = val;
	val -= 0x2000;
	for (int i = 0; i < 6; i++) {
		// Strangely, the early version driver applies the setting to channel 0 only.
		if (_drv->_out[i]->_assign == _id || (_drv->_version == SCI_VERSION_1_EARLY && i == 0))
			_drv->_out[i]->pitchBend(val);
	}
}

void TownsMidiPart::addChannels(int num) {
	for (int i = 0; i < 6; i++) {
		if (_drv->_out[i]->_assign != 0xff)
			continue;

		_drv->_out[i]->_assign = _id;
		_drv->_out[i]->updateVolume();

		if (_drv->_out[i]->_note != 0xff)
			_drv->_out[i]->noteOff();

		if (!--num)
			break;
	}

	_chanMissing += num;
	programChange(_program);
}

void TownsMidiPart::dropChannels(int num) {
	if (_chanMissing == num) {
		_chanMissing = 0;
		return;
	} else if (_chanMissing > num) {
		_chanMissing -= num;
		return;
	}

	num -= _chanMissing;
	_chanMissing = 0;

	for (int i = 0; i < 6; i++) {
		if (_drv->_out[i]->_assign != _id || _drv->_out[i]->_note != 0xff)
			continue;
		_drv->_out[i]->_assign = 0xff;
		if (!--num)
			return;
	}

	for (int i = 0; i < 6; i++) {
		if (_drv->_out[i]->_assign != _id)
			continue;
		_drv->_out[i]->_sustain = 0;
		_drv->_out[i]->noteOff();
		_drv->_out[i]->_assign = 0xff;
		if (!--num)
			return;
	}
}

uint8 TownsMidiPart::currentProgram() const {
	return _program;
}

int TownsMidiPart::allocateChannel() {
	int chan = _outChan;
	int ovrChan = 0;
	int ld = 0;
	bool found = false;

	for (bool loop = true; loop; ) {
		if (++chan == 6)
			chan = 0;

		if (chan == _outChan)
			loop = false;

		if (_id == _drv->_out[chan]->_assign || _drv->_version == SCI_VERSION_1_EARLY) {
			if (_drv->_out[chan]->_note == 0xff) {
				found = true;
				break;
			}

			if (_drv->_out[chan]->_duration >= ld) {
				ld = _drv->_out[chan]->_duration;
				ovrChan = chan;
			}
		}
	}

	if (!found) {
		if (!ld)
			return -1;
		chan = ovrChan;
		_drv->_out[chan]->_sustain = 0;
		_drv->_out[chan]->noteOff();
	}

	_outChan = chan;
	return chan;
}

MidiDriver_FMTowns::MidiDriver_FMTowns(Audio::Mixer *mixer, SciVersion version) : _version(version), _timerProc(0), _timerProcPara(0), _baseTempo(10080), _ready(false), _isOpen(false), _masterVolume(0x0f), _soundOn(true) {
	_intf = new TownsAudioInterface(mixer, this, true);
	_out = new TownsChannel*[6];
	for (int i = 0; i < 6; i++)
		_out[i] = new TownsChannel(this, i);
	_parts = new TownsMidiPart*[16];
	for (int i = 0; i < 16; i++)
		_parts[i] = new TownsMidiPart(this, i);
}

MidiDriver_FMTowns::~MidiDriver_FMTowns() {
	delete _intf;

	if (_parts) {
		for (int i = 0; i < 16; i++) {
			delete _parts[i];
			_parts[i] = 0;
		}
		delete[] _parts;
		_parts = 0;
	}

	if (_out) {
		for (int i = 0; i < 6; i++) {
			delete _out[i];
			_out[i] = 0;
		}
		delete[] _out;
		_out = 0;
	}
}

int MidiDriver_FMTowns::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	if (!_ready) {
		if (!_intf->init())
			return MERR_CANNOT_CONNECT;

		_intf->callback(0);

		_intf->callback(21, 255, 1);
		_intf->callback(21, 0, 1);
		_intf->callback(22, 255, 221);

		_intf->callback(33, 8);
		_intf->setSoundEffectChanMask(~0x3f);

		_ready = true;
	}

	_isOpen = true;

	return 0;
}

void MidiDriver_FMTowns::loadInstruments(const uint8 *data) {
	if (data) {
		data += 6;
		for (int i = 0; i < 128; i++) {
			_intf->callback(5, 0, i, data);
			data += 48;
		}
	}
	_intf->callback(70, 3);
	property(MIDI_PROP_MASTER_VOLUME, _masterVolume);
}

void MidiDriver_FMTowns::close() {
	_isOpen = false;
}

void MidiDriver_FMTowns::send(uint32 b) {
	if (!_isOpen)
		return;

	byte para2 = (b >> 16) & 0xFF;
	byte para1 = (b >> 8) & 0xFF;
	byte cmd = b & 0xF0;

	TownsMidiPart *chan = _parts[b & 0x0F];

	switch (cmd) {
	case 0x80:
		chan->noteOff(para1);
		break;
	case 0x90:
		chan->noteOn(para1, para2);
		break;
	case 0xb0:
		switch (para1) {
		case 7:
			chan->controlChangeVolume(para2);
			break;
		case 64:
			chan->controlChangeSustain(para2);
			break;
		case SCI_MIDI_SET_POLYPHONY:
			chan->controlChangePolyphony(para2);
			break;
		case SCI_MIDI_CHANNEL_NOTES_OFF:
			chan->controlChangeAllNotesOff();
			break;
		default:
			break;
		}
		break;
	case 0xc0:
		chan->programChange(para1);
		break;
	case 0xe0:
		chan->pitchBend(para1 | (para2 << 7));
		break;
	default:
		break;
	}
}

uint32 MidiDriver_FMTowns::property(int prop, uint32 param) {
	switch(prop) {
	case MIDI_PROP_MASTER_VOLUME:
		if (param != 0xffff) {
			_masterVolume = param;
			for (int i = 0; i < 6; i++)
				_out[i]->updateVolume();
		}
		return _masterVolume;
	default:
		break;
	}
	return 0;
}

void MidiDriver_FMTowns::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_timerProc = timer_proc;
	_timerProcPara = timer_param;
}

void MidiDriver_FMTowns::setSoundOn(bool toggle) {
	_soundOn = toggle;
}

uint32 MidiDriver_FMTowns::getBaseTempo() {
	return _baseTempo;
}

void MidiDriver_FMTowns::timerCallback(int timerId) {
	if (!_isOpen)
		return;

	switch (timerId) {
	case 1:
		updateParser();
		updateChannels();
		break;
	default:
		break;
	}
}

int MidiDriver_FMTowns::getChannelVolume(uint8 midiPart) {
	static const uint8 volumeTable[] = { 0x00, 0x0D, 0x1B, 0x28, 0x36, 0x43, 0x51, 0x5F, 0x63, 0x67, 0x6B, 0x6F, 0x73, 0x77, 0x7B, 0x7F };
	int tableIndex = (_version == SCI_VERSION_1_EARLY) ? _masterVolume : (_parts[midiPart]->_volume * (_masterVolume + 1)) >> 6;
	assert(tableIndex < 16);
	return volumeTable[tableIndex];
}

void MidiDriver_FMTowns::addMissingChannels() {
	uint8 avlChan = 0;
	for (int i = 0; i < 6; i++) {
		if (_out[i]->_assign == 0xff)
			avlChan++;
	}

	if (!avlChan)
		return;

	for (int i = 0; i < 16; i++) {
		if (!_parts[i]->_chanMissing)
			continue;

		if (_parts[i]->_chanMissing < avlChan) {
			avlChan -= _parts[i]->_chanMissing;
			uint8 m = _parts[i]->_chanMissing;
			_parts[i]->_chanMissing = 0;
			_parts[i]->addChannels(m);
		} else {
			_parts[i]->_chanMissing -= avlChan;
			_parts[i]->addChannels(avlChan);
			return;
		}
	}
}

void MidiDriver_FMTowns::updateParser() {
	if (_timerProc)
		_timerProc(_timerProcPara);
}

void MidiDriver_FMTowns::updateChannels() {
	for (int i = 0; i < 6; i++)
		_out[i]->updateDuration();
}

MidiPlayer_FMTowns::MidiPlayer_FMTowns(SciVersion version) : MidiPlayer(version) {
	_driver = _townsDriver = new MidiDriver_FMTowns(g_system->getMixer(), version);
}

MidiPlayer_FMTowns::~MidiPlayer_FMTowns() {
	delete _driver;
}

int MidiPlayer_FMTowns::open(ResourceManager *resMan) {
	int result = MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
	if (_townsDriver) {
		result = _townsDriver->open();
		if (!result && _version == SCI_VERSION_1_LATE)
			_townsDriver->loadInstruments((resMan->findResource(ResourceId(kResourceTypePatch, 8), true))->data);
	}
	return result;
}

bool MidiPlayer_FMTowns::hasRhythmChannel() const {
	return false;
}

byte MidiPlayer_FMTowns::getPlayId() const {
	return (_version == SCI_VERSION_1_EARLY) ? 0x00 : 0x16;
}

int MidiPlayer_FMTowns::getPolyphony() const {
	return (_version == SCI_VERSION_1_EARLY) ? 1 : 6;
}

void MidiPlayer_FMTowns::playSwitch(bool play) {
	if (_townsDriver)
		_townsDriver->setSoundOn(play);
}

MidiPlayer *MidiPlayer_FMTowns_create(SciVersion _soundVersion) {
	return new MidiPlayer_FMTowns(_soundVersion);
}

} // End of namespace Sci

