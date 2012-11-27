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

#ifndef SCUMM_PLAYER_SID_H
#define SCUMM_PLAYER_SID_H

#include "common/mutex.h"
#include "common/scummsys.h"
#include "scumm/music.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/softsynth/sid.h"

namespace Scumm {

// the "channel" parameters seem to be in fact SID register
// offsets. Should be replaced.
enum sid_reg_t {
	FREQ_VOICE1,
	FREQ_VOICE2,
	FREQ_VOICE3,
	FREQ_FILTER,
	PULSE_VOICE1,
	PULSE_VOICE2,
	PULSE_VOICE3
};

enum VideoStandard {
	PAL,
	NTSC
};

class ScummEngine;

class Player_SID : public Audio::AudioStream, public MusicEngine {
public:
	Player_SID(ScummEngine *scumm, Audio::Mixer *mixer);
	virtual ~Player_SID();

	virtual void setMusicVolume(int vol) { _maxvol = vol; }
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getSoundStatus(int sound) const;
	virtual int  getMusicTimer();

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return false; }
	int getRate() const { return _sampleRate; }

private:
	Resid::SID *_sid;
	void SID_Write(int reg, uint8 data);
	void initSID();
	uint8 *getResource(int resID);

	// number of cpu cycles until next frame update
	Resid::cycle_count _cpuCyclesLeft;

	ScummEngine *_vm;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;
	int _sampleRate;
	int _maxvol;
	Common::Mutex _mutex;

	VideoStandard _videoSystem;

	int _music_timer;
	uint8* _music;

private:
	void initMusic(int songResIndex); // $7de6
	int initSound(int soundResID); // $4D0A
	void stopSound_intern(int soundResID); // $5093
	void stopMusic_intern(); // $4CAA

	void resetSID(); // $48D8
	void update(); // $481B
	void handleMusicBuffer();
	int setupSongFileData(); // $36cb
	void func_3674(int channel); // $3674
	void resetPlayerState(); // $48f7
	void processSongData(int channel); // $4939
	void readSetSIDFilterAndProps(int *offset, uint8* dataPtr);  // $49e7
	void saveSongPos(int y, int channel);
	void updateFreq(int channel);
	void resetFreqDelta(int channel);
	void readSongChunk(int channel); // $4a6b
	void setSIDFreqAS(int channel); // $4be6
	void setSIDWaveCtrlReg(int channel); // $4C0D
	int setupSongPtr(int channel); // $4C1C
	void unlockResource(int chanResIndex); // $4CDA
	void countFreeChannels(); // $4f26
	void func_4F45(int channel); // $4F45
	void safeUnlockResource(int resIndex); // $4FEA
	void releaseResource(int resIndex); // $5031
	void releaseResChannels(int resIndex); // $5070
	void releaseResourceUnk(int resIndex); // $50A4
	void releaseChannel(int channel);
	void clearSIDWaveform(int channel);
	void stopChannel(int channel);
	void swapVars(int channel, int swapIndex); // $51a5
	void resetSwapVars(); // $52d0
	void prepareSwapVars(int channel); // $52E5
	void useSwapVars(int channel); // $5342
	void lockResource(int resIndex); // $4ff4
	void reserveChannel(int channel, uint8 prioValue, int chanResIndex); // $4ffe
	void unlockCodeLocation(); // $513e
	void lockCodeLocation(); // $514f
	void func_7eae(int channel, uint8* songFileDataPtr); // $7eae
	void func_819b(int channel); // $819b
	void buildStepTbl(int step); // $82B4
	int reserveSoundFilter(uint8 value, uint8 chanResIndex); // $4ED0
	int reserveSoundVoice(uint8 value, uint8 chanResIndex); // $4EB8
	void findLessPrioChannels(uint8 soundPrio); // $4ED8
	void releaseResourceBySound(int resID); // $5088
	void readVec6Data(int x, int *offset, uint8 *songFilePtr, int chanResID); // $4E99

	void unused1(); // $50AF

	uint8 chanBuffer[3][45];

	int resID_song;

	// statusBits1A/1B are always equal
	uint8 statusBits1A;
	uint8 statusBits1B;

	uint8 busyChannelBits;

	uint8 SIDReg23;
	uint8 SIDReg23Stuff;
	uint8 SIDReg24;

	uint8* chanFileData[3];
	uint16 chanDataOffset[3];
	uint8* songPosPtr[7];

	// 0..2: freq value voice1/2/3
	// 3:    filter freq
	// 4..6: pulse width
	uint16 freqReg[7];

	// start offset[i] for songFileOrChanBufData to obtain songPosPtr[i]
	//	vec6[0..2] = 0x0008;
	//	vec6[4..6] = 0x0019;
	uint16 vec6[7];

	// current offset[i] for songFileOrChanBufData to obtain songPosPtr[i] (starts with vec6[i], increased later)
	uint16 songFileOrChanBufOffset[7];

	uint16 freqDelta[7];
	int freqDeltaCounter[7];
	uint8* swapSongPosPtr[3];
	uint8* swapVec5[3];
	uint16 swapVec8[3];
	uint16 swapVec10[3];
	uint16 swapFreqReg[3];
	int swapVec11[3];

	// never read
	//uint8* vec5[7];
	// never read
	//uint8 vec19[7];
	// never read (needed by scumm engine?)
	//bool curChannelActive;

	uint8* vec20[7];

	uint8* swapVec20[3];

	// resource status (never read)
	// bit7: some flag
	// bit6..0: counter (use-count?), maybe just bit0 as flag (used/unused?)
	uint8 resStatus[70];

	uint8* songFileOrChanBufData;
	uint8* actSongFileData;

	uint16 stepTbl[33];

	bool initializing;
	bool _soundInQueue;
	bool isVoiceChannel;

	bool isMusicPlaying;
	bool swapVarLoaded;
	bool bgSoundActive;
	bool filterUsed;

	uint8 bgSoundResID;
	uint8 freeChannelCount;

	// seems to be used for managing the three voices
	// bit[0..2]: 0 -> unused, 1 -> already in use
	uint8 usedChannelBits;
	uint8 attackReg[3];
	uint8 sustainReg[3];

	// -1/0/1
	int var481A;

	// bit-array: 00000cba
	// a/b/c: channel1/2/3
	uint8 songChannelBits;

	bool pulseWidthSwapped;
	bool swapPrepared;

	// never read
	//uint8 var5163;

	bool filterSwapped;
	uint8 SIDReg24_HiNibble;
	bool keepSwapVars;

	uint8 phaseBit[3];
	bool releasePhase[3];

	// values: a resID or -1
	// resIDs: 3, 4, 5 or song-number
	int _soundQueue[7];

	// values: a resID or 0
	// resIDs: 3, 4, 5 or song-number
	int channelMap[7];

	uint8 songPosUpdateCounter[7];

	// priortity of channel contents
	// MM:  1: lowest .. 120: highest (1,2,A,64,6E,73,78)
	// Zak: -???: lowest .. 120: highest (5,32,64,65,66,6E,78, A5,A6,AF,D7)
	uint8 chanPrio[7];

	// only [0..2] used?
	uint8 waveCtrlReg[7];

	uint8 swapAttack[2];
	uint8 swapSustain[2];
	uint8 swapSongPrio[3];
	int swapVec479C[3];
	uint8 swapVec19[3];
	uint8 swapSongPosUpdateCounter[3];
	uint8 swapWaveCtrlReg[3];

	bool actFilterHasLowerPrio;
	uint8 chansWithLowerPrioCount;
	uint8 minChanPrio;
	uint8 minChanPrioIndex;
};

} // End of namespace Scumm

#endif
