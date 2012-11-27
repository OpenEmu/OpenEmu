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


#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "sky/music/adlibchannel.h"
#include "sky/sky.h"

namespace Sky {

AdLibChannel::AdLibChannel(FM_OPL *opl, uint8 *pMusicData, uint16 startOfData) {
	_opl = opl;
	_musicData = pMusicData;
	_channelData.loopPoint = startOfData;
	_channelData.eventDataPtr = startOfData;
	_channelData.channelActive = true;
	_channelData.tremoVibro = 0;
	_channelData.assignedInstrument = 0xFF;
	_channelData.channelVolume = 0x7F;
	_channelData.nextEventTime = getNextEventTime();

	_channelData.adlibChannelNumber = _channelData.lastCommand = _channelData.note =
		_channelData.adlibReg1 = _channelData.adlibReg2 = _channelData.freqOffset = 0;
	_channelData.frequency = 0;
	_channelData.instrumentData = NULL;

	uint16 instrumentDataLoc;

	if (SkyEngine::_systemVars.gameVersion == 109) {
		//instrumentDataLoc = (_musicData[0x11D0] << 8) | _musicData[0x11CF];
		//_frequenceTable = (uint16 *)(_musicData + 0x835);
		//_registerTable = _musicData + 0xE35;
		//_opOutputTable = _musicData + 0xE47;
		//_adlibRegMirror = _musicData + 0xF4A;

		instrumentDataLoc = READ_LE_UINT16(_musicData + 0x1204);
		_frequenceTable = (uint16 *)(_musicData + 0x868);
		_registerTable = _musicData + 0xE68;
		_opOutputTable = _musicData + 0xE7A;
		_adlibRegMirror = _musicData + 0xF7D;
	} else if (SkyEngine::_systemVars.gameVersion == 267) {
		instrumentDataLoc = READ_LE_UINT16(_musicData + 0x11FB);
		_frequenceTable = (uint16 *)(_musicData + 0x7F4);
		_registerTable = _musicData + 0xDF4;
		_opOutputTable = _musicData + 0xE06;
		_adlibRegMirror = _musicData + 0xF55;
	} else {
		instrumentDataLoc = READ_LE_UINT16(_musicData + 0x1205);
		_frequenceTable = (uint16 *)(_musicData + 0x7FE);
		_registerTable = _musicData + 0xDFE;
		_opOutputTable = _musicData + 0xE10;
		_adlibRegMirror = _musicData + 0xF5F;
	}

	_instrumentMap = _musicData+instrumentDataLoc;
	_instruments = (InstrumentStruct *)(_instrumentMap+0x80);
}

AdLibChannel::~AdLibChannel() {
	stopNote();
}

bool AdLibChannel::isActive() {
	return _channelData.channelActive;
}

void AdLibChannel::updateVolume(uint16 pVolume) {
	// Do nothing. The mixer handles the music volume for us.
}

/*	This class uses the same area for the register mirror as the original
	asm driver did (_musicData[0xF5F..0x105E]), so the cache is indeed shared
	by all instances of the class.
*/
void AdLibChannel::setRegister(uint8 regNum, uint8 value) {
	if (_adlibRegMirror[regNum] != value) {
		OPLWriteReg (_opl, regNum, value);
		_adlibRegMirror[regNum] = value;
	}
}

void AdLibChannel::stopNote() {
	if (_channelData.note & 0x20) {
		_channelData.note &= ~0x20;
		setRegister(0xB0 | _channelData.adlibChannelNumber, _channelData.note);
	}
}

int32 AdLibChannel::getNextEventTime() {
	int32 retV = 0;
	uint8 cnt, lVal = 0;
	for (cnt = 0; cnt < 4; cnt++) {
		lVal = _musicData[_channelData.eventDataPtr];
		_channelData.eventDataPtr++;
		retV = (retV << 7) | (lVal & 0x7F);
		if (!(lVal & 0x80))
			break;
	}
	if (lVal & 0x80) {
		return -1; // should never happen
	} else
		return retV;
}

uint8 AdLibChannel::process(uint16 aktTime) {
	if (!_channelData.channelActive) {
		return 0;
	}

	uint8 returnVal = 0;

	_channelData.nextEventTime -= aktTime;
	uint8 opcode;
	while ((_channelData.nextEventTime < 0) && (_channelData.channelActive)) {
		opcode = _musicData[_channelData.eventDataPtr];
		_channelData.eventDataPtr++;
		if (opcode & 0x80) {
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
				case 5: com90_getFreqOffset(); break;
				case 6: com90_getChannelVolume(); break;
				case 7: com90_getTremoVibro(); break;
				case 8: com90_loopMusic(); break;
				case 9: com90_keyOff(); break;
				case 12: com90_setLoopPoint(); break;

				default:
					error("AdLibChannel: Unknown music opcode 0x%02X", opcode);
					break;
				}
			} else {
				// new adlib channel assignment
				_channelData.adlibChannelNumber = opcode & 0xF;
				_channelData.adlibReg1 = _registerTable[((opcode & 0xF) << 1) | 0];
				_channelData.adlibReg2 = _registerTable[((opcode & 0xF) << 1) | 1];
			}
		} else {
			_channelData.lastCommand = opcode;
			stopNote();
			// not sure why this "if" is necessary...either a bug in my
			// code or a bug in the music data (section 1, music 2)
			if (_channelData.instrumentData || _channelData.tremoVibro) {
				setupInstrument(opcode);

				opcode = _musicData[_channelData.eventDataPtr];
				_channelData.eventDataPtr++;
				setupChannelVolume(opcode);
			} else
				_channelData.eventDataPtr++;
		}
		if (_channelData.channelActive)
			_channelData.nextEventTime += getNextEventTime();
	}
	return returnVal;
}

void AdLibChannel::setupInstrument(uint8 opcode) {
	uint16 nextNote;
	if (_channelData.tremoVibro) {
		uint8 newInstrument = _instrumentMap[opcode];
		if (newInstrument != _channelData.assignedInstrument) {
			_channelData.assignedInstrument = newInstrument;
			_channelData.instrumentData = _instruments + newInstrument;
			adlibSetupInstrument();
		}
		_channelData.lastCommand = _channelData.instrumentData->bindedEffect;
		nextNote = getNextNote(_channelData.lastCommand);
	} else {
		nextNote = getNextNote(opcode - 0x18 + _channelData.instrumentData->bindedEffect);
	}
	_channelData.frequency = nextNote;
	setRegister(0xA0 | _channelData.adlibChannelNumber, (uint8)nextNote);
	setRegister(0xB0 | _channelData.adlibChannelNumber, (uint8)((nextNote >> 8) | 0x20));
	_channelData.note = (uint8)((nextNote >> 8) | 0x20);
}

void AdLibChannel::setupChannelVolume(uint8 volume) {
	uint8 resultOp;
	uint32 resVol = ((volume + 1) * (_channelData.instrumentData->totOutLev_Op2 + 1)) << 1;
	resVol &= 0xFFFF;
	resVol *= (_channelData.channelVolume + 1) << 1;
	resVol >>= 16;
	assert(resVol < 0x81);
	resultOp = ((_channelData.instrumentData->scalingLevel << 6) & 0xC0) | _opOutputTable[resVol];
	setRegister(0x40 | _channelData.adlibReg2, resultOp);
	if (_channelData.instrumentData->feedBack & 1) {
		resVol = ((volume + 1) * (_channelData.instrumentData->totOutLev_Op1 + 1)) << 1;
		resVol &= 0xFFFF;
		resVol *= (_channelData.channelVolume + 1) << 1;
		resVol >>= 16;
	} else
		resVol = _channelData.instrumentData->totOutLev_Op1;
	assert(resVol < 0x81);
	resultOp = ((_channelData.instrumentData->scalingLevel << 2) & 0xC0) | _opOutputTable[resVol];
	setRegister(0x40 | _channelData.adlibReg1, resultOp);
}

void AdLibChannel::adlibSetupInstrument() {
	setRegister(0x60 | _channelData.adlibReg1, _channelData.instrumentData->ad_Op1);
	setRegister(0x60 | _channelData.adlibReg2, _channelData.instrumentData->ad_Op2);
	setRegister(0x80 | _channelData.adlibReg1, _channelData.instrumentData->sr_Op1);
	setRegister(0x80 | _channelData.adlibReg2, _channelData.instrumentData->sr_Op2);
	setRegister(0xE0 | _channelData.adlibReg1, _channelData.instrumentData->waveSelect_Op1);
	setRegister(0xE0 | _channelData.adlibReg2, _channelData.instrumentData->waveSelect_Op2);
	setRegister(0xC0 | _channelData.adlibChannelNumber, _channelData.instrumentData->feedBack);
	setRegister(0x20 | _channelData.adlibReg1, _channelData.instrumentData->ampMod_Op1);
	setRegister(0x20 | _channelData.adlibReg2, _channelData.instrumentData->ampMod_Op2);
}

uint16 AdLibChannel::getNextNote(uint8 param) {
	int16 freqIndex = ((int16)_channelData.freqOffset) - 0x40;
	if (freqIndex >= 0x3F)
		freqIndex++;
	freqIndex *= 2;
	freqIndex += param << 6;
	uint16 freqData = FROM_LE_16(_frequenceTable[freqIndex % 0x300]);
	if ((freqIndex % 0x300 >= 0x1C0) || (freqIndex / 0x300 > 0)) {
		return (((freqIndex / 0x300) - 1) << 10) + (freqData & 0x7FF);
	} else {
		// looks like a bug. dunno why. It's what the ASM code says.
		return (uint16)(((int16)freqData) >> 1);
	}
}

//- command 90h routines

void AdLibChannel::com90_caseNoteOff() {
	if (_musicData[_channelData.eventDataPtr] == _channelData.lastCommand)
		stopNote();
	_channelData.eventDataPtr++;
}

void AdLibChannel::com90_stopChannel() {
	stopNote();
	_channelData.channelActive = false;
}

void AdLibChannel::com90_setupInstrument() {
	_channelData.channelVolume = 0x7F;
	_channelData.freqOffset = 0x40;
	_channelData.assignedInstrument = _musicData[_channelData.eventDataPtr];
	_channelData.eventDataPtr++;
	_channelData.instrumentData = _instruments + _channelData.assignedInstrument;
	adlibSetupInstrument();
}

uint8 AdLibChannel::com90_updateTempo() {
	return _musicData[_channelData.eventDataPtr++];
}

void AdLibChannel::com90_getFreqOffset() {
	_channelData.freqOffset = _musicData[_channelData.eventDataPtr++];
	if (_channelData.note & 0x20) {
		uint16 nextNote = getNextNote(
			_channelData.lastCommand - 0x18 + _channelData.instrumentData->bindedEffect);
		setRegister(0xA0 | _channelData.adlibChannelNumber, (uint8)nextNote);
		setRegister(0xB0 | _channelData.adlibChannelNumber, (uint8)((nextNote >> 8) | 0x20));
		_channelData.note = (uint8)(nextNote >> 8) | 0x20;
	}
}

void AdLibChannel::com90_getChannelVolume() {
	_channelData.channelVolume = _musicData[_channelData.eventDataPtr++];
}

void AdLibChannel::com90_getTremoVibro() {
	_channelData.tremoVibro = _musicData[_channelData.eventDataPtr++];
}

void AdLibChannel::com90_loopMusic() {
	_channelData.eventDataPtr = _channelData.loopPoint;
}

void AdLibChannel::com90_keyOff() {
	stopNote();
}

void AdLibChannel::com90_setLoopPoint() {
	_channelData.loopPoint = _channelData.eventDataPtr;
}

} // End of namespace Sky
