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

#ifndef DISABLE_SID

#include "engines/engine.h"
#include "scumm/player_sid.h"
#include "scumm/scumm.h"
#include "audio/mixer.h"

namespace Scumm {

/*
 * The player's update() routine is called once per (NTSC/PAL) frame as it is
 * called by the VIC Rasterline interrupt handler which is in turn called
 * approx. 50 (PAL) or 60 (NTSC) times per second.
 * The SCUMM V0/V1 music playback routines or sound data have not been adjusted
 * to PAL systems. As a consequence, music is played audibly (-16%) slower
 * on PAL systems.
 * In addition, the SID oscillator frequency depends on the video clock too.
 * As SCUMM games use an NTSC frequency table for both NTSC and PAL versions
 * all tone frequencies on PAL systems are slightly (-4%) lower than on NTSC ones.
 *
 * For more info on the SID chip see:
 * - http://www.dopeconnection.net/C64_SID.htm (German)
 * For more info on the VIC chip see:
 * - http://www.htu.tugraz.at/~herwig/c64/man-vic.php (German)
 * - http://www.c64-wiki.de/index.php/VIC (German)
 */

struct TimingProps {
	double clockFreq;
	int cyclesPerFrame;
};

static const TimingProps timingProps[2] = {
	{ 17734472.0 / 18, 312 * 63 }, // PAL:  312*63 cycles/frame @  985248 Hz (~50Hz)
	{ 14318180.0 / 14, 263 * 65 }  // NTSC: 263*65 cycles/frame @ 1022727 Hz (~60Hz)
};

static const uint8 BITMASK[7] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40
};
static const uint8 BITMASK_INV[7] = {
	0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF
};

static const int SID_REG_OFFSET[7] = {
	0, 7, 14, 21, 2, 9, 16
};

// NTSC frequency table (also used for PAL versions).
// FREQ_TBL[i] = tone_freq[i] * 2^24 / clockFreq
static const uint16 FREQ_TBL[97] = {
	0x0000, 0x010C, 0x011C, 0x012D, 0x013E, 0x0151, 0x0166, 0x017B,
	0x0191, 0x01A9, 0x01C3, 0x01DD, 0x01FA, 0x0218, 0x0238, 0x025A,
	0x027D, 0x02A3, 0x02CC, 0x02F6, 0x0323, 0x0353, 0x0386, 0x03BB,
	0x03F4, 0x0430, 0x0470, 0x04B4, 0x04FB, 0x0547, 0x0598, 0x05ED,
	0x0647, 0x06A7, 0x070C, 0x0777, 0x07E9, 0x0861, 0x08E1, 0x0968,
	0x09F7, 0x0A8F, 0x0B30, 0x0BDA, 0x0C8F, 0x0D4E, 0x0E18, 0x0EEF,
	0x0FD2, 0x10C3, 0x11C3, 0x12D1, 0x13EF, 0x151F, 0x1660, 0x17B5,
	0x191E, 0x1A9C, 0x1C31, 0x1DDF, 0x1FA5, 0x2187, 0x2386, 0x25A2,
	0x27DF, 0x2A3E, 0x2CC1, 0x2F6B, 0x323C, 0x3539, 0x3863, 0x3BBE,
	0x3F4B, 0x430F, 0x470C, 0x4B45, 0x4FBF, 0x547D, 0x5983, 0x5ED6,
	0x6479, 0x6A73, 0x70C7, 0x777C, 0x7E97, 0x861E, 0x8E18, 0x968B,
	0x9F7E, 0xA8FA, 0xB306, 0xBDAC, 0xC8F3, 0xD4E6, 0xE18F, 0xEEF8,
	0xFD2E
};

static const int SONG_CHANNEL_OFFSET[3] = { 6, 8, 10 };
static const int RES_ID_CHANNEL[3] = { 3, 4, 5 };

#define LOBYTE_(a) ((a) & 0xFF)
#define HIBYTE_(a) (((a) >> 8) & 0xFF)

#define GETBIT(var, pos) ((var) & (1<<(pos)))

void Player_SID::handleMusicBuffer() { // $33cd
	int channel = 2;
	while (channel >= 0) {
		if ((statusBits1A & BITMASK[channel]) == 0 ||
		    (busyChannelBits & BITMASK[channel]) != 0) {
			--channel;
			continue;
		}

		if (setupSongFileData() == 1)
			return;

		uint8* l_chanFileDataPtr = chanFileData[channel];

		uint16 l_freq = 0;
		bool l_keepFreq = false;

		int y = 0;
		uint8 curByte = l_chanFileDataPtr[y++];

		// freq or 0/0xFF
		if (curByte == 0) {
			func_3674(channel);
			if (!isMusicPlaying)
				return;
			continue;
		} else if (curByte == 0xFF) {
			l_keepFreq = true;
		} else {
			l_freq = FREQ_TBL[curByte];
		}

		uint8 local1 = 0;
		curByte = l_chanFileDataPtr[y++];
		bool isLastCmdByte = (curByte & 0x80) != 0;
		uint16 curStepSum = stepTbl[curByte & 0x7f];

		for (int i = 0; !isLastCmdByte && (i < 2); ++i) {
			curByte = l_chanFileDataPtr[y++];
			isLastCmdByte = (curByte & 0x80) != 0;
			if (curByte & 0x40) {
				// note: bit used in zak theme (95) only (not used/handled in MM)
				_music_timer = curByte & 0x3f;
			} else {
				local1 = curByte & 0x3f;
			}
		}

		chanFileData[channel] += y;
		chanDataOffset[channel] += y;

		uint8 *l_chanBuf = getResource(RES_ID_CHANNEL[channel]);

		if (local1 != 0) {
			// TODO: signed or unsigned?
			uint16 offset = READ_LE_UINT16(&actSongFileData[local1*2 + 12]);
			l_chanFileDataPtr = actSongFileData + offset;

			// next five bytes: freqDelta, attack, sustain and phase bit
			for (int i = 0; i < 5; ++i) {
				l_chanBuf[15 + i] = l_chanFileDataPtr[i];
			}
			phaseBit[channel] = l_chanFileDataPtr[4];

			for (int i = 0; i < 17; ++i) {
				l_chanBuf[25 + i] = l_chanFileDataPtr[5 + i];
			}
		}

		if (l_keepFreq) {
			if (!releasePhase[channel]) {
				l_chanBuf[10] &= 0xfe; // release phase
			}
			releasePhase[channel] = true;
		} else {
			if (releasePhase[channel]) {
				l_chanBuf[19] = phaseBit[channel];
				l_chanBuf[10] |= 0x01; // attack phase
			}
			l_chanBuf[11] = LOBYTE_(l_freq);
			l_chanBuf[12] = HIBYTE_(l_freq);
			releasePhase[channel] = false;
		}

		// set counter value for frequency update (freqDeltaCounter)
		l_chanBuf[13] = LOBYTE_(curStepSum);
		l_chanBuf[14] = HIBYTE_(curStepSum);

		_soundQueue[channel] = RES_ID_CHANNEL[channel];
		processSongData(channel);
		_soundQueue[channel+4] = RES_ID_CHANNEL[channel];
		processSongData(channel+4);
		--channel;
	}
}

int Player_SID::setupSongFileData() { // $36cb
	// no song playing
	// TODO: remove (never NULL)
	if (_music == NULL) {
		for (int i = 2; i >= 0; --i) {
			if (songChannelBits & BITMASK[i]) {
				func_3674(i);
			}
		}
		return 1;
	}

	// no new song
	songFileOrChanBufData = _music;
	if (_music == actSongFileData) {
		return 0;
	}

	// new song selected
	actSongFileData = _music;
	for (int i = 0; i < 3; ++i) {
		chanFileData[i] = _music + chanDataOffset[i];
	}

	return -1;
}

//x:0..2
void Player_SID::func_3674(int channel) { // $3674
	statusBits1B &= BITMASK_INV[channel];
	if (statusBits1B == 0) {
		isMusicPlaying = false;
		unlockCodeLocation();
		safeUnlockResource(resID_song);
		for (int i = 0; i < 3; ++i) {
			safeUnlockResource(RES_ID_CHANNEL[i]);
		}
	}

	chanPrio[channel] = 2;

	statusBits1A &= BITMASK_INV[channel];
	phaseBit[channel] = 0;

	func_4F45(channel);
}

void Player_SID::resetPlayerState() { // $48f7
	for (int i = 6; i >= 0; --i)
		releaseChannel(i);

	isMusicPlaying = false;
	unlockCodeLocation(); // does nothing
	statusBits1B = 0;
	statusBits1A = 0;
	freeChannelCount = 3;
	swapPrepared = false;
	filterSwapped = false;
	pulseWidthSwapped = false;
	//var5163 = 0;
}

void Player_SID::resetSID() { // $48D8
	SIDReg24 = 0x0f;

	SID_Write( 4, 0);
	SID_Write(11, 0);
	SID_Write(18, 0);
	SID_Write(23, 0);
	SID_Write(21, 0);
	SID_Write(22, 0);
	SID_Write(24, SIDReg24);

	resetPlayerState();
}

void Player_SID::update() { // $481B
	if (initializing)
		return;

	if (_soundInQueue) {
		for (int i = 6; i >= 0; --i) {
			if (_soundQueue[i] != -1)
				processSongData(i);
		}
		_soundInQueue = false;
	}

	// no sound
	if (busyChannelBits == 0)
		return;

	for (int i = 6; i >= 0; --i) {
		if (busyChannelBits & BITMASK[i]) {
			updateFreq(i);
		}
	}

	// seems to be used for background (prio=1?) sounds.
	// If a bg sound cannot be played because all SID
	// voices are used by higher priority sounds, the
	// bg sound's state is updated here so it will be at
	// the correct state when a voice is available again.
	if (swapPrepared) {
		swapVars(0, 0);
		swapVarLoaded = true;
		updateFreq(0);
		swapVars(0, 0);
		if (pulseWidthSwapped) {
			swapVars(4, 1);
			updateFreq(4);
			swapVars(4, 1);
		}
		swapVarLoaded = false;
	}

	for (int i = 6; i >= 0; --i) {
		if (busyChannelBits & BITMASK[i])
			setSIDWaveCtrlReg(i);
	};

	if (isMusicPlaying) {
		handleMusicBuffer();
	}

	return;
}

// channel: 0..6
void Player_SID::processSongData(int channel) { // $4939
	// always: _soundQueue[channel] != -1
	// -> channelMap[channel] != -1
	channelMap[channel] = _soundQueue[channel];
	_soundQueue[channel] = -1;
	songPosUpdateCounter[channel] = 0;

	isVoiceChannel = (channel < 3);

	songFileOrChanBufOffset[channel] = vec6[channel];

	setupSongPtr(channel);

	//vec5[channel] = songFileOrChanBufData; // not used

	if (songFileOrChanBufData == NULL) { // chanBuf (4C1C)
		/*
		// TODO: do we need this?
		LOBYTE_(vec20[channel]) = 0;
		LOBYTE_(songPosPtr[channel]) = LOBYTE_(songFileOrChanBufOffset[channel]);
		*/
		releaseResourceUnk(channel);
		return;
	}

	vec20[channel] = songFileOrChanBufData; // chanBuf (4C1C)
	songPosPtr[channel] = songFileOrChanBufData + songFileOrChanBufOffset[channel]; // chanBuf (4C1C)
	uint8* ptr1 = songPosPtr[channel];

	int y = -1;
	if (channel < 4) {
		++y;
		if (channel == 3) {
			readSetSIDFilterAndProps(&y, ptr1);
		} else if (statusBits1A & BITMASK[channel]) {
			++y;
		} else { // channel = 0/1/2
			waveCtrlReg[channel] = ptr1[y];

			++y;
			if (ptr1[y] & 0x0f) {
				// filter on for voice channel
				SIDReg23 |= BITMASK[channel];
			} else {
				// filter off for voice channel
				SIDReg23 &= BITMASK_INV[channel];
			}
			SID_Write(23, SIDReg23);
		}
	}

	saveSongPos(y, channel);
	busyChannelBits |= BITMASK[channel];
	readSongChunk(channel);
}

void Player_SID::readSetSIDFilterAndProps(int *offset, uint8* dataPtr) {  // $49e7
	SIDReg23 |= dataPtr[*offset];
	SID_Write(23, SIDReg23);
	++*offset;
	SIDReg24 = dataPtr[*offset];
	SID_Write(24, SIDReg24);
}

void Player_SID::saveSongPos(int y, int channel) {
	++y;
	songPosPtr[channel] += y;
	songFileOrChanBufOffset[channel] += y;
}

// channel: 0..6
void Player_SID::updateFreq(int channel) {
	isVoiceChannel = (channel < 3);

	--freqDeltaCounter[channel];
	if (freqDeltaCounter[channel] < 0) {
		readSongChunk(channel);
	} else {
		freqReg[channel] += freqDelta[channel];
	}
	setSIDFreqAS(channel);
}

void Player_SID::resetFreqDelta(int channel) {
	freqDeltaCounter[channel] = 0;
	freqDelta[channel] = 0;
}

void Player_SID::readSongChunk(int channel) { // $4a6b
	while (true) {
		if (setupSongPtr(channel) == 1) {
			// do something with code resource
			releaseResourceUnk(1);
			return;
		}

		uint8* ptr1 = songPosPtr[channel];

		//curChannelActive = true;

		uint8 l_cmdByte = ptr1[0];
		if (l_cmdByte == 0) {
			//curChannelActive = false;
			songPosUpdateCounter[channel] = 0;

			var481A = -1;
			releaseChannel(channel);
			return;
		}

		//vec19[channel] = l_cmdByte;

		// attack (1) / release (0) phase
		if (isVoiceChannel) {
			if (GETBIT(l_cmdByte, 0))
				waveCtrlReg[channel] |= 0x01; // start attack phase
			else
				waveCtrlReg[channel] &= 0xfe; // start release phase
		}

		// channel finished bit
		if (GETBIT(l_cmdByte, 1)) {
			var481A = -1;
			releaseChannel(channel);
			return;
		}

		int y = 0;

		// frequency
		if (GETBIT(l_cmdByte, 2)) {
			y += 2;
			freqReg[channel] = READ_LE_UINT16(&ptr1[y-1]);
			if (!GETBIT(l_cmdByte, 6)) {
				y += 2;
				freqDeltaCounter[channel] = READ_LE_UINT16(&ptr1[y-1]);
				y += 2;
				freqDelta[channel] = READ_LE_UINT16(&ptr1[y-1]);
			} else {
				resetFreqDelta(channel);
			}
		} else {
			resetFreqDelta(channel);
		}

		// attack / release
		if (isVoiceChannel && GETBIT(l_cmdByte, 3)) {
			// start release phase
			waveCtrlReg[channel] &= 0xfe;
			setSIDWaveCtrlReg(channel);

			++y;
			attackReg[channel] = ptr1[y];
			++y;
			sustainReg[channel] = ptr1[y];

			// set attack (1) or release (0) phase
			waveCtrlReg[channel]  |= (l_cmdByte & 0x01);
		}

		if (GETBIT(l_cmdByte, 4)) {
			++y;
			uint8 curByte = ptr1[y];

			// pulse width
			if (isVoiceChannel && GETBIT(curByte, 0)) {
				int reg = SID_REG_OFFSET[channel+4];

				y += 2;
				SID_Write(reg, ptr1[y-1]);
				SID_Write(reg+1, ptr1[y]);
			}

			if (GETBIT(curByte, 1)) {
				++y;
				readSetSIDFilterAndProps(&y, ptr1);

				y += 2;
				SID_Write(21, ptr1[y-1]);
				SID_Write(22, ptr1[y]);
			}

			if (GETBIT(curByte, 2)) {
				resetFreqDelta(channel);

				y += 2;
				freqDeltaCounter[channel] = READ_LE_UINT16(&ptr1[y-1]);
			}
		}

		// set waveform (?)
		if (GETBIT(l_cmdByte, 5)) {
			++y;
			waveCtrlReg[channel] = (waveCtrlReg[channel] & 0x0f) | ptr1[y];
		}

		// song position
		if (GETBIT(l_cmdByte, 7)) {
			if (songPosUpdateCounter[channel] == 1) {
				y += 2;
				--songPosUpdateCounter[channel];
				saveSongPos(y, channel);
			} else {
				// looping / skipping / ...
				++y;
				songPosPtr[channel] -= ptr1[y];
				songFileOrChanBufOffset[channel] -= ptr1[y];

				++y;
				if (songPosUpdateCounter[channel] == 0) {
					songPosUpdateCounter[channel] = ptr1[y];
				} else {
					--songPosUpdateCounter[channel];
				}
			}
		} else {
			saveSongPos(y, channel);
			return;
		}
	}
}

/**
 * Sets frequency, attack and sustain register
 */
void Player_SID::setSIDFreqAS(int channel) { // $4be6
	if (swapVarLoaded)
		return;
	int reg = SID_REG_OFFSET[channel];
	SID_Write(reg,   LOBYTE_(freqReg[channel]));   // freq/pulseWidth voice 1/2/3
	SID_Write(reg+1, HIBYTE_(freqReg[channel]));
	if (channel < 3) {
		SID_Write(reg+5, attackReg[channel]); // attack
		SID_Write(reg+6, sustainReg[channel]); // sustain
	}
}

void Player_SID::setSIDWaveCtrlReg(int channel) { // $4C0D
	if (channel < 3) {
		int reg = SID_REG_OFFSET[channel];
		SID_Write(reg+4, waveCtrlReg[channel]);
	}
}

// channel: 0..6
int Player_SID::setupSongPtr(int channel) { // $4C1C
	//resID:5,4,3,songid
	int resID = channelMap[channel];

	// TODO: when does this happen, only if resID == 0?
	if (getResource(resID) == NULL) {
		releaseResourceUnk(resID);
		if (resID == bgSoundResID) {
			bgSoundResID = 0;
			bgSoundActive = false;
			swapPrepared = false;
			pulseWidthSwapped = false;
		}
		return 1;
	}

	songFileOrChanBufData = getResource(resID); // chanBuf (4C1C)
	if (songFileOrChanBufData == vec20[channel]) {
		return 0;
	} else {
		vec20[channel] = songFileOrChanBufData;
		songPosPtr[channel] = songFileOrChanBufData + songFileOrChanBufOffset[channel];
		return -1;
	}
}

// ignore: no effect
// chanResIndex: 3,4,5 or 58
void Player_SID::unlockResource(int chanResIndex) { // $4CDA
	if ((resStatus[chanResIndex] & 0x7F) != 0)
		--resStatus[chanResIndex];
}

void Player_SID::countFreeChannels() { // $4f26
	freeChannelCount = 0;
	for (int i = 0; i < 3; ++i) {
		if (GETBIT(usedChannelBits, i) == 0)
			++freeChannelCount;
	}
}

void Player_SID::func_4F45(int channel) { // $4F45
	if (swapVarLoaded) {
		if (channel == 0) {
			swapPrepared = false;
			resetSwapVars();
		}
		pulseWidthSwapped = false;
	} else {
		if (channel == 3) {
			filterUsed = false;
		}

		if (chanPrio[channel] == 1) {
			if (var481A == 1)
				prepareSwapVars(channel);
			else if (channel < 3)
				clearSIDWaveform(channel);
		} else if (channel < 3 && bgSoundActive && swapPrepared &&
		    !(filterSwapped && filterUsed))
		{
			busyChannelBits |= BITMASK[channel];
			useSwapVars(channel);
			waveCtrlReg[channel] |= 0x01;
			setSIDWaveCtrlReg(channel);

			safeUnlockResource(channelMap[channel]);
			return;
		}

		chanPrio[channel] = 0;
		usedChannelBits &= BITMASK_INV[channel];
		countFreeChannels();
	}

	int resIndex = channelMap[channel];
	channelMap[channel] = 0;
	safeUnlockResource(resIndex);
}

// chanResIndex: 3,4,5 or 58
void Player_SID::safeUnlockResource(int resIndex) { // $4FEA
	if (!isMusicPlaying) {
		unlockResource(resIndex);
	}
}

void Player_SID::releaseResource(int resIndex) { // $5031
	releaseResChannels(resIndex);
	if (resIndex == bgSoundResID && var481A == -1) {
		safeUnlockResource(resIndex);

		bgSoundResID = 0;
		bgSoundActive = false;
		swapPrepared = false;
		pulseWidthSwapped = false;

		resetSwapVars();
	}
}

void Player_SID::releaseResChannels(int resIndex) { // $5070
	for (int i = 3; i >= 0; --i) {
		if (resIndex == channelMap[i]) {
			releaseChannel(i);
		}
	}
}

void Player_SID::stopSound_intern(int soundResID) { // $5093
	for (int i = 0; i < 7; ++i) {
		if (soundResID == _soundQueue[i]) {
			_soundQueue[i] = -1;
		}
	}
	var481A = -1;
	releaseResource(soundResID);
}

void Player_SID::stopMusic_intern() { // $4CAA
	statusBits1B = 0;
	isMusicPlaying = false;

	if (resID_song != 0) {
		unlockResource(resID_song);
	}

	chanPrio[0] = 2;
	chanPrio[1] = 2;
	chanPrio[2] = 2;

	statusBits1A = 0;
	phaseBit[0] = 0;
	phaseBit[1] = 0;
	phaseBit[2] = 0;
}

void Player_SID::releaseResourceUnk(int resIndex) { // $50A4
	var481A = -1;
	releaseResource(resIndex);
}

// a: 0..6
void Player_SID::releaseChannel(int channel) {
	stopChannel(channel);
	if (channel >= 4) {
		return;
	}
	if (channel < 3) {
		SIDReg23Stuff = SIDReg23;
		clearSIDWaveform(channel);
	}
	func_4F45(channel);
	if (channel >= 3) {
		return;
	}
	if ((SIDReg23 != SIDReg23Stuff) &&
	    (SIDReg23 & 0x07) == 0)
	{
		if (filterUsed) {
			func_4F45(3);
			stopChannel(3);
		}
	}

	stopChannel(channel + 4);
}

void Player_SID::clearSIDWaveform(int channel) {
	if (!isMusicPlaying && var481A == -1) {
		waveCtrlReg[channel] &= 0x0e;
		setSIDWaveCtrlReg(channel);
	}
}

void Player_SID::stopChannel(int channel) {
	songPosUpdateCounter[channel] = 0;
	// clear "channel" bit
	busyChannelBits &= BITMASK_INV[channel];
	if (channel >= 4) {
		// pulsewidth = 0
		channelMap[channel] = 0;
	}
}

// channel: 0..6, swapIndex: 0..2
void Player_SID::swapVars(int channel, int swapIndex) { // $51a5
	if (channel < 3) {
		SWAP(attackReg[channel], swapAttack[swapIndex]);
		SWAP(sustainReg[channel], swapSustain[swapIndex]);
	}
	//SWAP(vec5[channel],  swapVec5[swapIndex]);  // not used
	//SWAP(vec19[channel], swapVec19[swapIndex]); // not used

	SWAP(chanPrio[channel], swapSongPrio[swapIndex]);
	SWAP(channelMap[channel], swapVec479C[swapIndex]);
	SWAP(songPosUpdateCounter[channel], swapSongPosUpdateCounter[swapIndex]);
	SWAP(waveCtrlReg[channel], swapWaveCtrlReg[swapIndex]);
	SWAP(songPosPtr[channel],  swapSongPosPtr[swapIndex]);
	SWAP(freqReg[channel],  swapFreqReg[swapIndex]);
	SWAP(freqDeltaCounter[channel], swapVec11[swapIndex]);
	SWAP(freqDelta[channel], swapVec10[swapIndex]);
	SWAP(vec20[channel], swapVec20[swapIndex]);
	SWAP(songFileOrChanBufOffset[channel],  swapVec8[swapIndex]);
}

void Player_SID::resetSwapVars() { // $52d0
	for (int i = 0; i < 2; ++i) {
		swapAttack[i] = 0;
		swapSustain[i] = 0;
	}
	for (int i = 0; i < 3; ++i) {
		swapVec5[i] = 0;
		swapSongPrio[i] = 0;
		swapVec479C[i] = 0;
		swapVec19[i] = 0;
		swapSongPosUpdateCounter[i] = 0;
		swapWaveCtrlReg[i] = 0;
		swapSongPosPtr[i] = 0;
		swapFreqReg[i] = 0;
		swapVec11[i] = 0;
		swapVec10[i] = 0;
		swapVec20[i] = 0;
		swapVec8[i] = 0;
	}
}

void Player_SID::prepareSwapVars(int channel) { // $52E5
	if (channel >= 4)
		return;

	if (channel < 3) {
		if (!keepSwapVars) {
			resetSwapVars();
		}
		swapVars(channel, 0);
		if (busyChannelBits & BITMASK[channel+4]) {
			swapVars(channel+4, 1);
			pulseWidthSwapped = true;
		}
	} else if (channel == 3) {
		SIDReg24_HiNibble = SIDReg24 & 0x70;
		resetSwapVars();
		keepSwapVars = true;
		swapVars(3, 2);
		filterSwapped = true;
	}
	swapPrepared = true;
}

void Player_SID::useSwapVars(int channel) { // $5342
	if (channel >= 3)
		return;

	swapVars(channel, 0);
	setSIDFreqAS(channel);
	if (pulseWidthSwapped) {
		swapVars(channel+4, 1);
		setSIDFreqAS(channel+4);
	}
	if (filterSwapped) {
		swapVars(3, 2);

		// resonating filter freq. or voice-to-filter mapping?
		SIDReg23 = (SIDReg23Stuff & 0xf0) | BITMASK[channel];
		SID_Write(23, SIDReg23);

		// filter props
		SIDReg24 = (SIDReg24 & 0x0f) | SIDReg24_HiNibble;
		SID_Write(24, SIDReg24);

		// filter freq.
		SID_Write(21, LOBYTE_(freqReg[3]));
		SID_Write(22, HIBYTE_(freqReg[3]));
	} else {
		SIDReg23 = SIDReg23Stuff & BITMASK_INV[channel];
		SID_Write(23, SIDReg23);
	}

	swapPrepared = false;
	pulseWidthSwapped = false;
	keepSwapVars = false;
	SIDReg24_HiNibble = 0;
	filterSwapped = false;
}

// ignore: no effect
// resIndex: 3,4,5 or 58
void Player_SID::lockResource(int resIndex) { // $4ff4
	if (!isMusicPlaying)
		++resStatus[resIndex];
}

void Player_SID::reserveChannel(int channel, uint8 prioValue, int chanResIndex) { // $4ffe
	if (channel == 3) {
		filterUsed = true;
	} else if (channel < 3) {
		usedChannelBits |= BITMASK[channel];
		countFreeChannels();
	}

	chanPrio[channel] = prioValue;
	lockResource(chanResIndex);
}

// ignore: no effect
void Player_SID::unlockCodeLocation() { // $513e
	resStatus[1] &= 0x80;
	resStatus[2] &= 0x80;
}

// ignore: no effect
void Player_SID::lockCodeLocation() { // $514f
	resStatus[1] |= 0x01;
	resStatus[2] |= 0x01;
}

void Player_SID::initMusic(int songResIndex) { // $7de6
	unlockResource(resID_song);

	resID_song = songResIndex;
	_music = getResource(resID_song);
	if (_music == NULL) {
		return;
	}

	// song base address
	uint8* songFileDataPtr = _music;
	actSongFileData = _music;

	initializing = true;
	_soundInQueue = false;
	isMusicPlaying = false;

	unlockCodeLocation();
	resetPlayerState();

	lockResource(resID_song);
	buildStepTbl(songFileDataPtr[5]);

	// fetch sound
	songChannelBits = songFileDataPtr[4];
	for (int i = 2; i >= 0; --i) {
		if ((songChannelBits & BITMASK[i]) != 0) {
			func_7eae(i, songFileDataPtr);
		}
	}

	isMusicPlaying = true;
	lockCodeLocation();

	SIDReg23 &= 0xf0;
	SID_Write(23, SIDReg23);

	handleMusicBuffer();

	initializing = false;
	_soundInQueue = true;
}

// params:
//   channel: channel 0..2
void Player_SID::func_7eae(int channel, uint8* songFileDataPtr) {
	int pos = SONG_CHANNEL_OFFSET[channel];
	chanDataOffset[channel] = READ_LE_UINT16(&songFileDataPtr[pos]);
	chanFileData[channel] = songFileDataPtr + chanDataOffset[channel];

	//vec5[channel+4] = vec5[channel] = CHANNEL_BUFFER_ADDR[RES_ID_CHANNEL[channel]]; // not used
	vec6[channel+4] = 0x0019;
	vec6[channel]   = 0x0008;

	func_819b(channel);

	waveCtrlReg[channel] = 0;
}

void Player_SID::func_819b(int channel) {
	reserveChannel(channel, 127, RES_ID_CHANNEL[channel]);

	statusBits1B |= BITMASK[channel];
	statusBits1A |= BITMASK[channel];
}

void Player_SID::buildStepTbl(int step) { // $82B4
	stepTbl[0] = 0;
	stepTbl[1] = step - 2;
	for (int i = 2; i < 33; ++i) {
		stepTbl[i] = stepTbl[i-1] + step;
	}
}

int Player_SID::reserveSoundFilter(uint8 value, uint8 chanResIndex) { // $4ED0
	int channel = 3;
	reserveChannel(channel, value, chanResIndex);
	return channel;
}

int Player_SID::reserveSoundVoice(uint8 value, uint8 chanResIndex) { // $4EB8
	for (int i = 2; i >= 0; --i) {
		if ((usedChannelBits & BITMASK[i]) == 0) {
			reserveChannel(i, value, chanResIndex);
			return i;
		}
	}
	return 0;
}

void Player_SID::findLessPrioChannels(uint8 soundPrio) { // $4ED8
	minChanPrio = 127;

	chansWithLowerPrioCount = 0;
	for (int i = 2; i >= 0; --i) {
		if (usedChannelBits & BITMASK[i]) {
			if (chanPrio[i] < soundPrio)
				++chansWithLowerPrioCount;
			if (chanPrio[i] < minChanPrio) {
				minChanPrio = chanPrio[i];
				minChanPrioIndex = i;
			}
		}
	}

	if (chansWithLowerPrioCount == 0)
		return;

	if (soundPrio >= chanPrio[3]) {
		actFilterHasLowerPrio = true;
	} else {
		/* TODO: is this really a no-op?
		if (minChanPrioIndex < chanPrio[3])
			minChanPrioIndex = minChanPrioIndex;
		*/

		actFilterHasLowerPrio = false;
	}
}

void Player_SID::releaseResourceBySound(int resID) { // $5088
	var481A = 1;
	releaseResource(resID);
}

void Player_SID::readVec6Data(int x, int *offset, uint8 *songFilePtr, int chanResID) { // $4E99
	//vec5[x] = songFilePtr;
	vec6[x] = songFilePtr[*offset];
	*offset += 2;
	_soundQueue[x] = chanResID;
}

int Player_SID::initSound(int soundResID) { // $4D0A
	initializing = true;

	if (isMusicPlaying && (statusBits1A & 0x07) == 0x07) {
		initializing = false;
		return -2;
	}

	uint8 *songFilePtr = getResource(soundResID);
	if (songFilePtr == NULL) {
		initializing = false;
		return 1;
	}

	uint8 soundPrio = songFilePtr[4];
	// for (mostly but not always looped) background sounds
	if (soundPrio == 1) {
		bgSoundResID = soundResID;
		bgSoundActive = true;
	}

	uint8 requestedChannels = 0;
	if ((songFilePtr[5] & 0x40) == 0) {
		++requestedChannels;
		if (songFilePtr[5] & 0x02)
			++requestedChannels;
		if (songFilePtr[5] & 0x08)
			++requestedChannels;
	}

	bool filterNeeded = (songFilePtr[5] & 0x20) != 0;
	bool filterBlocked = (filterUsed && filterNeeded);
	if (filterBlocked || (freeChannelCount < requestedChannels)) {
		findLessPrioChannels(soundPrio);

		if ((freeChannelCount + chansWithLowerPrioCount < requestedChannels) ||
		    (filterBlocked && !actFilterHasLowerPrio)) {
			initializing = false;
			return -1;
		}

		if (filterBlocked) {
			if (soundPrio < chanPrio[3]) {
				initializing = false;
				return -1;
			}

			uint8 l_resID = channelMap[3];
			releaseResourceBySound(l_resID);
		}

		while ((freeChannelCount < requestedChannels) || (filterNeeded && filterUsed)) {
			findLessPrioChannels(soundPrio);
			if (minChanPrio >= soundPrio) {
				initializing = false;
				return -1;
			}

			uint8 l_resID = channelMap[minChanPrioIndex];
			releaseResourceBySound(l_resID);
		}
	}

	int x;
	uint8 soundByte5 = songFilePtr[5];
	if (soundByte5 & 0x40)
		x = reserveSoundFilter(soundPrio, soundResID);
	else
		x = reserveSoundVoice(soundPrio, soundResID);

	uint8 var4CF3 = x;
	int y = 6;
	if (soundByte5 & 0x01) {
		x += 4;
		readVec6Data(x, &y, songFilePtr, soundResID);
	}
	if (soundByte5 & 0x02) {
		x = reserveSoundVoice(soundPrio, soundResID);
		readVec6Data(x, &y, songFilePtr, soundResID);
	}
	if (soundByte5 & 0x04) {
		x += 4;
		readVec6Data(x, &y, songFilePtr, soundResID);
	}
	if (soundByte5 & 0x08) {
		x = reserveSoundVoice(soundPrio, soundResID);
		readVec6Data(x, &y, songFilePtr, soundResID);
	}
	if (soundByte5 & 0x10) {
		x += 4;
		readVec6Data(x, &y, songFilePtr, soundResID);
	}
	if (soundByte5 & 0x20) {
		x = reserveSoundFilter(soundPrio, soundResID);
		readVec6Data(x, &y, songFilePtr, soundResID);
	}

	//vec5[var4CF3] = songFilePtr;
	vec6[var4CF3] = y;
	_soundQueue[var4CF3] = soundResID;

	initializing = false;
	_soundInQueue = true;

	return soundResID;
}

void Player_SID::unused1() { // $50AF
	var481A = -1;
	if (bgSoundResID != 0) {
		releaseResourceUnk(bgSoundResID);
	}
}

///////////////////////////
///////////////////////////

#define ZEROMEM(a) memset(a, 0, sizeof(a))

Player_SID::Player_SID(ScummEngine *scumm, Audio::Mixer *mixer) {
	/*
	 * clear memory
	 */

	resID_song = 0;
	statusBits1A = 0;
	statusBits1B = 0;
	busyChannelBits = 0;
	SIDReg23 = 0;
	SIDReg23Stuff = 0;
	SIDReg24 = 0;
	bgSoundResID = 0;
	freeChannelCount = 0;
	usedChannelBits = 0;
	var481A = 0;
	songChannelBits = 0;
	//var5163 = 0;
	SIDReg24_HiNibble = 0;
	chansWithLowerPrioCount = 0;
	minChanPrio = 0;
	minChanPrioIndex = 0;

	_music = NULL;
	songFileOrChanBufData = NULL;
	actSongFileData = NULL;

	initializing = false;
	_soundInQueue = false;
	isVoiceChannel = false;
	isMusicPlaying = false;
	swapVarLoaded = false;
	bgSoundActive = false;
	filterUsed = false;
	pulseWidthSwapped = false;
	swapPrepared = false;
	filterSwapped = false;
	keepSwapVars = false;
	actFilterHasLowerPrio = false;

	ZEROMEM(chanFileData);
	ZEROMEM(chanDataOffset);
	ZEROMEM(songPosPtr);
	ZEROMEM(freqReg);
	ZEROMEM(vec6);
	ZEROMEM(songFileOrChanBufOffset);
	ZEROMEM(freqDelta);
	ZEROMEM(freqDeltaCounter);
	ZEROMEM(swapSongPosPtr);
	ZEROMEM(swapVec5);
	ZEROMEM(swapVec8);
	ZEROMEM(swapVec10);
	ZEROMEM(swapFreqReg);
	ZEROMEM(swapVec11);
	ZEROMEM(vec20);
	ZEROMEM(swapVec20);
	ZEROMEM(resStatus);
	ZEROMEM(attackReg);
	ZEROMEM(sustainReg);
	ZEROMEM(phaseBit);
	ZEROMEM(releasePhase);
	ZEROMEM(_soundQueue);
	ZEROMEM(channelMap);
	ZEROMEM(songPosUpdateCounter);
	ZEROMEM(chanPrio);
	ZEROMEM(waveCtrlReg);
	ZEROMEM(swapAttack);
	ZEROMEM(swapSustain);
	ZEROMEM(swapSongPrio);
	ZEROMEM(swapVec479C);
	ZEROMEM(swapVec19);
	ZEROMEM(swapSongPosUpdateCounter);
	ZEROMEM(swapWaveCtrlReg);
	ZEROMEM(stepTbl);

	/*
	 * initialize data
	 */

	const uint8 chanBuffer_const[3][45] = {
		{
			0x00,0x00,0x00,0x00,0x7f,0x01,0x19,0x00,
			0x00,0x00,0x2d,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0xf0,0x40,0x10,0x04,0x00,0x00,
			0x00,0x04,0x27,0x03,0xff,0xff,0x01,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00
		},
		{
			0x00,0x00,0x00,0x00,0x7f,0x01,0x19,0x00,
			0x00,0x00,0x2d,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0xf0,0x20,0x10,0x04,0x00,0x00,
			0x00,0x04,0x27,0x03,0xff,0xff,0x02,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00
		},
		{
			0x00,0x00,0x00,0x00,0x7f,0x01,0x19,0x00,
			0x00,0x00,0x2d,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0xf0,0x20,0x10,0x04,0x00,0x00,
			0x00,0x04,0x27,0x03,0xff,0xff,0x02,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00
		}
	};
	memcpy(chanBuffer, chanBuffer_const, sizeof(chanBuffer_const));

	for (int i = 0; i < 7; ++i) {
		_soundQueue[i] = -1;
	};

	_music_timer = 0;

	_mixer = mixer;
	_sampleRate = _mixer->getOutputRate();
	_vm = scumm;

	// sound speed is slightly different on NTSC and PAL machines
	// as the SID clock depends on the frame rate.
	// ScummVM does not distinguish between NTSC and PAL targets
	// so we use the NTSC timing here as the music was composed for
	// NTSC systems (music on PAL systems is slower).
	_videoSystem = NTSC;
	_cpuCyclesLeft = 0;

	initSID();
	resetSID();

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

Player_SID::~Player_SID() {
	_mixer->stopHandle(_soundHandle);
	delete _sid;
}

uint8 *Player_SID::getResource(int resID) {
	switch (resID) {
	case 0:
		return NULL;
	case 3:
	case 4:
	case 5:
		return 	chanBuffer[resID-3];
	default:
		return _vm->getResourceAddress(rtSound, resID);
	}
}

int Player_SID::readBuffer(int16 *buffer, const int numSamples) {
	int samplesLeft = numSamples;

	Common::StackLock lock(_mutex);

	while (samplesLeft > 0) {
		// update SID status after each frame
		if (_cpuCyclesLeft <= 0) {
			update();
			_cpuCyclesLeft = timingProps[_videoSystem].cyclesPerFrame;
		}
		// fetch samples
		int sampleCount = _sid->updateClock(_cpuCyclesLeft, (short *)buffer, samplesLeft);
		samplesLeft -= sampleCount;
		buffer += sampleCount;
	}

	return numSamples;
}

void Player_SID::SID_Write(int reg, uint8 data) {
	_sid->write(reg, data);
}

void Player_SID::initSID() {
	_sid = new Resid::SID();
	_sid->set_sampling_parameters(
		timingProps[_videoSystem].clockFreq,
		_sampleRate);
	_sid->enable_filter(true);

	_sid->reset();
	// Synchronize the waveform generators (must occur after reset)
	_sid->write( 4, 0x08);
	_sid->write(11, 0x08);
	_sid->write(18, 0x08);
	_sid->write( 4, 0x00);
	_sid->write(11, 0x00);
	_sid->write(18, 0x00);
}

void Player_SID::startSound(int nr) {
	byte *data = _vm->getResourceAddress(rtSound, nr);
	assert(data);

	// WORKAROUND:
	// sound[4] contains either a song prio or a music channel usage byte.
	// As music channel usage is always 0x07 for all music files and
	// prio 7 is never used in any sound file use this byte for auto-detection.
	bool isMusic = (data[4] == 0x07);

	Common::StackLock lock(_mutex);

	if (isMusic) {
		initMusic(nr);
	} else {
		stopSound_intern(nr);
		initSound(nr);
	}
}

void Player_SID::stopSound(int nr) {
	if (nr == -1)
		return;

	Common::StackLock lock(_mutex);
	stopSound_intern(nr);
}

void Player_SID::stopAllSounds() {
	Common::StackLock lock(_mutex);
	resetPlayerState();
}

int Player_SID::getSoundStatus(int nr) const {
	int result = 0;

	//Common::StackLock lock(_mutex);

	if (resID_song == nr && isMusicPlaying) {
		result = 1;
	}

	for (int i = 0; (i < 4) && (result == 0); ++i) {
		if (nr == _soundQueue[i] || nr == channelMap[i]) {
			result = 1;
		}
	}

	return result;
}

int Player_SID::getMusicTimer() {
	int result = _music_timer;
	_music_timer = 0;
	return result;
}

} // End of namespace Scumm

#endif
