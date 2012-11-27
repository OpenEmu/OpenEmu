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

#include "gmchannel.h"
#include "common/util.h"
#include "common/textconsole.h"
#include "audio/mididrv.h"

namespace Sky {

GmChannel::GmChannel(uint8 *pMusicData, uint16 startOfData, MidiDriver *pMidiDrv, const byte *pInstMap, const byte *veloTab) {
	_musicData = pMusicData;
	_midiDrv = pMidiDrv;
	_channelData.midiChannelNumber = 0;
	_channelData.loopPoint = startOfData;
	_channelData.eventDataPtr = startOfData;
	_channelData.channelActive = true;
	_channelData.nextEventTime = getNextEventTime();
	_instMap = pInstMap;
	_veloTab = veloTab;

	_musicVolume = 0x7F;
	_currentChannelVolume = 0x7F;
}

GmChannel::~GmChannel() {
	stopNote();
}

bool GmChannel::isActive() {
	return _channelData.channelActive;
}

void GmChannel::updateVolume(uint16 pVolume) {
	_musicVolume = pVolume;
	if (_musicVolume > 0)
		_musicVolume = (_musicVolume * 2) / 3 + 43;

	uint8 newVol = (_currentChannelVolume * _musicVolume) >> 7;
	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x700 | (newVol << 16));
}

void GmChannel::stopNote() {
	// All Notes Off
	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x7B00 | 0 | 0x79000000);
	// Reset the Pitch Wheel. See bug #1016556.
	_midiDrv->send((0xE0 | _channelData.midiChannelNumber) | 0x400000);
}

int32 GmChannel::getNextEventTime() {
	int32 retV = 0;
	uint8 cnt, lVal = 0;
	for (cnt = 0; cnt < 4; cnt++) {
		lVal = _musicData[_channelData.eventDataPtr];
		_channelData.eventDataPtr++;
		retV = (retV << 7) | (lVal & 0x7F);
		if (!(lVal & 0x80))
			break;
	}
	if (lVal & 0x80) { // should never happen
		return -1;
	} else
		return retV;
}

uint8 GmChannel::process(uint16 aktTime) {
	if (!_channelData.channelActive)
		return 0;

	uint8 returnVal = 0;

	_channelData.nextEventTime -= aktTime;
	uint8 opcode;

	while ((_channelData.nextEventTime < 0) && (_channelData.channelActive)) {
		opcode = _musicData[_channelData.eventDataPtr];
		_channelData.eventDataPtr++;
		if (opcode&0x80) {
			if (opcode == 0xFF) {
				// dummy opcode
			} else if (opcode >= 0x90) {
				switch (opcode&0xF) {
				case 0: com90_caseNoteOff(); break;
				case 1: com90_stopChannel(); break;
				case 2: com90_setupInstrument(); break;
				case 3:
					returnVal = com90_updateTempo();
					break;
				case 5: com90_getPitch(); break;
				case 6: com90_getChannelVolume(); break;
				case 8: com90_loopMusic(); break;
				case 9: com90_keyOff(); break;
				case 11: com90_getChannelPanValue(); break;
				case 12: com90_setLoopPoint(); break;
				case 13: com90_getChannelControl(); break;

				default:
					error("GmChannel: Unknown music opcode 0x%02X", opcode);
					break;
				}
			} else {
				// new midi channel assignment
				_channelData.midiChannelNumber = opcode&0xF;
			}
		} else {
			_channelData.note = opcode;
			byte velocity = _musicData[_channelData.eventDataPtr];
			if (_veloTab)
				velocity = _veloTab[velocity];
			_channelData.eventDataPtr++;
			_midiDrv->send((0x90 | _channelData.midiChannelNumber) | (opcode << 8) | (velocity << 16));
		}
		if (_channelData.channelActive)
			_channelData.nextEventTime += getNextEventTime();
	}
	return returnVal;
}

//- command 90h routines

void GmChannel::com90_caseNoteOff() {
	_midiDrv->send((0x90 | _channelData.midiChannelNumber) | (_musicData[_channelData.eventDataPtr] << 8));
	_channelData.eventDataPtr++;
}

void GmChannel::com90_stopChannel() {
	stopNote();
	_channelData.channelActive = false;
}

void GmChannel::com90_setupInstrument() {
	byte instrument = _musicData[_channelData.eventDataPtr];
	if (_instMap)
		instrument = _instMap[instrument];
	_midiDrv->send((0xC0 | _channelData.midiChannelNumber) | (instrument << 8));
	_channelData.eventDataPtr++;
}

uint8 GmChannel::com90_updateTempo() {
	return _musicData[_channelData.eventDataPtr++];
}

void GmChannel::com90_getPitch() {
	_midiDrv->send((0xE0 | _channelData.midiChannelNumber) | 0 | (_musicData[_channelData.eventDataPtr] << 16));
	_channelData.eventDataPtr++;
}

void GmChannel::com90_getChannelVolume() {
	_currentChannelVolume = _musicData[_channelData.eventDataPtr++];
	uint8 newVol = (uint8)((_currentChannelVolume * _musicVolume) >> 7);
	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x700 | (newVol << 16));
}

void GmChannel::com90_loopMusic() {
	_channelData.eventDataPtr = _channelData.loopPoint;
}

void GmChannel::com90_keyOff() {
	_midiDrv->send((0x90 | _channelData.midiChannelNumber) | (_channelData.note << 8) | 0);
}

void GmChannel::com90_setLoopPoint() {
	_channelData.loopPoint = _channelData.eventDataPtr;
}

void GmChannel::com90_getChannelPanValue() {
	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | 0x0A00 | (_musicData[_channelData.eventDataPtr] << 16));
	_channelData.eventDataPtr++;
}

void GmChannel::com90_getChannelControl() {
	uint8 conNum = _musicData[_channelData.eventDataPtr++];
	uint8 conDat = _musicData[_channelData.eventDataPtr++];
	_midiDrv->send((0xB0 | _channelData.midiChannelNumber) | (conNum << 8) | (conDat << 16));
}

} // End of namespace Sky
