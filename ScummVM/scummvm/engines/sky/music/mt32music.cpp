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

#include "sky/music/mt32music.h"
#include "sky/music/gmchannel.h"
#include "common/util.h"
#include "common/system.h"
#include "common/endian.h"
#include "common/textconsole.h"
#include "audio/mididrv.h"

namespace Sky {

void MT32Music::passTimerFunc(void *param) {
	((MT32Music *)param)->timerCall();
}

MT32Music::MT32Music(MidiDriver *pMidiDrv, Audio::Mixer *pMixer, Disk *pDisk) : MusicBase(pMixer, pDisk) {
	_driverFileBase = 60200;
	_midiDrv = pMidiDrv;
	int midiRes = _midiDrv->open();
	if (midiRes != 0)
		error("Can't open midi device. Errorcode: %d",midiRes);
	_timerCount = 0;
	_midiDrv->setTimerCallback(this, passTimerFunc);
	_midiDrv->sendMT32Reset();
}

MT32Music::~MT32Music() {
	_midiDrv->close();
	_midiDrv->setTimerCallback(NULL, NULL);
	delete _midiDrv;
}

void MT32Music::timerCall() {
	_timerCount += _midiDrv->getBaseTempo();
	if (_timerCount > (1000000 / 50)) {
		// call pollMusic() 50 times per second
		_timerCount -= 1000000 / 50;
		if (_musicData != NULL)
			pollMusic();
	}
}

void MT32Music::setVolume(uint16 volume) {
	uint8 sysEx[10] = "\x41\x10\x16\x12\x10\x00\x16\x00\x00";
	_musicVolume = volume;
	sysEx[7] = (volume > 100) ? 100 : (uint8)volume;
	sysEx[8] = 0x00;
	for (uint8 cnt = 4; cnt < 8; cnt++)
		sysEx[8] -= sysEx[cnt];
	sysEx[8] &= 0x7F;
	_midiDrv->sysEx(sysEx, 9);
}

void MT32Music::setupPointers() {
	_musicDataLoc = READ_LE_UINT16(_musicData + 0x7DC);
	_sysExSequence = READ_LE_UINT16(_musicData + 0x7E0) + _musicData;
}

void MT32Music::setupChannels(uint8 *channelData) {
	_numberOfChannels = channelData[0];
	channelData++;
	for (uint8 cnt = 0; cnt < _numberOfChannels; cnt++) {
		uint16 chDataStart = READ_LE_UINT16((uint16 *)channelData + cnt) + _musicDataLoc;
		_channels[cnt] = new GmChannel(_musicData, chDataStart, _midiDrv, NULL, NULL);
		_channels[cnt]->updateVolume(_musicVolume);
	}
}

bool MT32Music::processPatchSysEx(uint8 *sysExData) {
	uint8 sysExBuf[15];
	uint8 crc = 0;
	if (sysExData[0] & 0x80)
		return false;

	// decompress data from stream
	sysExBuf[ 0] = 0x41;
	sysExBuf[ 1] = 0x10;
	sysExBuf[ 2] = 0x16;
	sysExBuf[ 3] = 0x12;
	sysExBuf[ 4] = 0x5;
	sysExBuf[ 5] = sysExData[0] >> 4;			// patch offset part 1
	sysExBuf[ 6] = (sysExData[0] & 0xF) << 3;	// patch offset part 2
	sysExBuf[ 7] = sysExData[1] >> 6;			// timbre group
	sysExBuf[ 8] = sysExData[1] & 0x3F;			// timbre num
	sysExBuf[ 9] = sysExData[2] & 0x3F;			// key shift
	sysExBuf[10] = sysExData[3] & 0x7F;			// fine tune
	sysExBuf[11] = sysExData[4] & 0x7F;         // bender range
	sysExBuf[12] = sysExData[2] >> 6;			// assign mode
	sysExBuf[13] = sysExData[3] >> 7;			// reverb switch
	for (uint8 cnt = 4; cnt < 14; cnt++)
		crc -= sysExBuf[cnt];
	sysExBuf[14] = crc & 0x7F;					// crc
	_midiDrv->sysEx(sysExBuf, 15);
	// We delay the time it takes to send the sysEx plus an
	// additional 40ms, which is required for MT-32 rev00,
	// to assure no buffer overflow or missing bytes
	g_system->delayMillis(17 * 1000 / 3125 + 40);
	return true;
}

void MT32Music::startDriver() {
	// setup timbres and patches using SysEx data
	uint8* sysExData = _sysExSequence;
	uint8 timbreNum = sysExData[0];
	uint8 cnt, crc;
	sysExData++;
	uint8 sendBuf[256];
	uint8 len;
	sendBuf[0] = 0x41;
	sendBuf[1] = 0x10;
	sendBuf[2] = 0x16;
	sendBuf[3] = 0x12;
	for (cnt = 0; cnt < timbreNum; cnt++) {
		len = 7;
		crc = 0;
		// Timbre address
		sendBuf[4] = 0x8 | (sysExData[0] >> 6);
		sendBuf[5] = (sysExData[0] & 0x3F) << 1;
		sendBuf[6] = 0xA;
		sysExData++;
		crc -= sendBuf[4] + sendBuf[5] + sendBuf[6];
		uint8 dataLen = sysExData[0];
		sysExData++;
		// Timbre data:
		do {
			uint8 rlVal = 1;
			uint8 codeVal = sysExData[0];
			sysExData++;

			if (codeVal & 0x80) {
				codeVal &= 0x7F;
				rlVal = sysExData[0];
				sysExData++;
				dataLen--;
			}
			for (uint8 cnt2 = 0; cnt2 < rlVal; cnt2++) {
				sendBuf[len] = codeVal;
				len++;
				crc -= codeVal;
			}
			dataLen--;
		} while (dataLen > 0);
		sendBuf[len] = crc & 0x7F;
		len++;
		_midiDrv->sysEx(sendBuf, len);
		// We delay the time it takes to send the sysEx plus an
		// additional 40ms, which is required for MT-32 rev00,
		// to assure no buffer overflow or missing bytes
		g_system->delayMillis((len + 2) * 1000 / 3125 + 40);
	}

	while (processPatchSysEx(sysExData))
		sysExData += 5;
}

} // End of namespace Sky
