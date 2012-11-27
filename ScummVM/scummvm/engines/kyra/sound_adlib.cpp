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
 * LGPL License
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "kyra/sound_intern.h"
#include "kyra/resource.h"

#include "common/system.h"
#include "common/mutex.h"
#include "common/config-manager.h"

#include "audio/mixer.h"
#include "audio/fmopl.h"
#include "audio/audiostream.h"

// Basic AdLib Programming:
// http://www.gamedev.net/reference/articles/article446.asp

#define CALLBACKS_PER_SECOND 72

namespace Kyra {

class AdLibDriver : public Audio::AudioStream {
public:
	AdLibDriver(Audio::Mixer *mixer, int version);
	~AdLibDriver();

	void initDriver();
	void setSoundData(uint8 *data);
	void queueTrack(int track, int volume);
	bool isChannelPlaying(int channel) const;
	void stopAllChannels();
	int getSoundTrigger() const { return _soundTrigger; }
	void resetSoundTrigger() { _soundTrigger = 0; }

	void callback();

	// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) {
		int32 samplesLeft = numSamples;
		memset(buffer, 0, sizeof(int16) * numSamples);
		while (samplesLeft) {
			if (!_samplesTillCallback) {
				callback();
				_samplesTillCallback = _samplesPerCallback;
				_samplesTillCallbackRemainder += _samplesPerCallbackRemainder;
				if (_samplesTillCallbackRemainder >= CALLBACKS_PER_SECOND) {
					_samplesTillCallback++;
					_samplesTillCallbackRemainder -= CALLBACKS_PER_SECOND;
				}
			}

			int32 render = MIN(samplesLeft, _samplesTillCallback);
			samplesLeft -= render;
			_samplesTillCallback -= render;
			YM3812UpdateOne(_adlib, buffer, render);
			buffer += render;
		}
		return numSamples;
	}

	bool isStereo() const { return false; }
	bool endOfData() const { return false; }
	int getRate() const { return _mixer->getOutputRate(); }

	void setSyncJumpMask(uint16 mask) { _syncJumpMask = mask; }

	void setMusicVolume(uint8 volume);
	void setSfxVolume(uint8 volume);

private:
	// These variables have not yet been named, but some of them are partly
	// known nevertheless:
	//
	// pitchBend - Sound-related. Possibly some sort of pitch bend.
	// unk18 - Sound-effect. Used for secondaryEffect1()
	// unk19 - Sound-effect. Used for secondaryEffect1()
	// unk20 - Sound-effect. Used for secondaryEffect1()
	// unk21 - Sound-effect. Used for secondaryEffect1()
	// unk22 - Sound-effect. Used for secondaryEffect1()
	// unk29 - Sound-effect. Used for primaryEffect1()
	// unk30 - Sound-effect. Used for primaryEffect1()
	// unk31 - Sound-effect. Used for primaryEffect1()
	// unk32 - Sound-effect. Used for primaryEffect2()
	// unk33 - Sound-effect. Used for primaryEffect2()
	// unk34 - Sound-effect. Used for primaryEffect2()
	// unk35 - Sound-effect. Used for primaryEffect2()
	// unk36 - Sound-effect. Used for primaryEffect2()
	// unk37 - Sound-effect. Used for primaryEffect2()
	// unk38 - Sound-effect. Used for primaryEffect2()
	// unk39 - Currently unused, except for updateCallback56()
	// unk40 - Currently unused, except for updateCallback56()
	// unk41 - Sound-effect. Used for primaryEffect2()

	struct Channel {
		bool lock;	// New to ScummVM
		uint8 opExtraLevel2;
		uint8 *dataptr;
		uint8 duration;
		uint8 repeatCounter;
		int8 baseOctave;
		uint8 priority;
		uint8 dataptrStackPos;
		uint8 *dataptrStack[4];
		int8 baseNote;
		uint8 unk29;
		uint8 unk31;
		uint16 unk30;
		uint16 unk37;
		uint8 unk33;
		uint8 unk34;
		uint8 unk35;
		uint8 unk36;
		uint8 unk32;
		uint8 unk41;
		uint8 unk38;
		uint8 opExtraLevel1;
		uint8 spacing2;
		uint8 baseFreq;
		uint8 tempo;
		uint8 position;
		uint8 regAx;
		uint8 regBx;
		typedef void (AdLibDriver::*Callback)(Channel&);
		Callback primaryEffect;
		Callback secondaryEffect;
		uint8 fractionalSpacing;
		uint8 opLevel1;
		uint8 opLevel2;
		uint8 opExtraLevel3;
		uint8 twoChan;
		uint8 unk39;
		uint8 unk40;
		uint8 spacing1;
		uint8 durationRandomness;
		uint8 unk19;
		uint8 unk18;
		int8 unk20;
		int8 unk21;
		uint8 unk22;
		uint16 offset;
		uint8 tempoReset;
		uint8 rawNote;
		int8 pitchBend;
		uint8 volumeModifier;
	};

	void primaryEffect1(Channel &channel);
	void primaryEffect2(Channel &channel);
	void secondaryEffect1(Channel &channel);

	void resetAdLibState();
	void writeOPL(byte reg, byte val);
	void initChannel(Channel &channel);
	void noteOff(Channel &channel);
	void unkOutput2(uint8 num);

	uint16 getRandomNr();
	void setupDuration(uint8 duration, Channel &channel);

	void setupNote(uint8 rawNote, Channel &channel, bool flag = false);
	void setupInstrument(uint8 regOffset, uint8 *dataptr, Channel &channel);
	void noteOn(Channel &channel);

	void adjustVolume(Channel &channel);

	uint8 calculateOpLevel1(Channel &channel);
	uint8 calculateOpLevel2(Channel &channel);

	uint16 checkValue(int16 val) {
		if (val < 0)
			val = 0;
		else if (val > 0x3F)
			val = 0x3F;
		return val;
	}

	// The sound data has at least two lookup tables:
	//
	// * One for programs, starting at offset 0.
	// * One for instruments, starting at offset 500.

	uint8 *getProgram(int progId) {
		uint16 offset = READ_LE_UINT16(_soundData + 2 * progId);
		//TODO: Check in LoL CD AdLib driver
		if (offset == 0xFFFF)
			return 0;
		return _soundData + READ_LE_UINT16(_soundData + 2 * progId);
	}

	uint8 *getInstrument(int instrumentId) {
		return getProgram(_numPrograms + instrumentId);
	}

	void setupPrograms();
	void executePrograms();

	struct ParserOpcode {
		typedef int (AdLibDriver::*POpcode)(uint8 *&dataptr, Channel &channel, uint8 value);
		POpcode function;
		const char *name;
	};

	void setupParserOpcodeTable();
	const ParserOpcode *_parserOpcodeTable;
	int _parserOpcodeTableSize;

	int update_setRepeat(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_checkRepeat(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupProgram(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setNoteSpacing(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_jump(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_jumpToSubroutine(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_returnFromSubroutine(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setBaseOctave(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_stopChannel(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_playRest(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_writeAdLib(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupNoteAndDuration(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setBaseNote(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupSecondaryEffect1(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_stopOtherChannel(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_waitForEndOfProgram(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupInstrument(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupPrimaryEffect1(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removePrimaryEffect1(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setBaseFreq(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupPrimaryEffect2(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setPriority(uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback23(uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback24(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setExtraLevel1(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupDuration(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_playNote(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setFractionalNoteSpacing(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setTempo(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removeSecondaryEffect1(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setChannelTempo(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setExtraLevel3(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setExtraLevel2(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_changeExtraLevel2(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setAMDepth(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setVibratoDepth(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_changeExtraLevel1(uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback38(uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback39(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removePrimaryEffect2(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_pitchBend(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_resetToGlobalTempo(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_nop(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setDurationRandomness(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_changeChannelTempo(uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback46(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setupRhythmSection(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_playRhythmSection(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_removeRhythmSection(uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback51(uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback52(uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback53(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setSoundTrigger(uint8 *&dataptr, Channel &channel, uint8 value);
	int update_setTempoReset(uint8 *&dataptr, Channel &channel, uint8 value);
	int updateCallback56(uint8 *&dataptr, Channel &channel, uint8 value);
private:
	// These variables have not yet been named, but some of them are partly
	// known nevertheless:
	//
	// _unkValue1      - Unknown. Used for updating _unkValue2
	// _unkValue2      - Unknown. Used for updating _unkValue4
	// _unkValue4      - Unknown. Used for updating _unkValue5
	// _unkValue5      - Unknown. Used for controlling updateCallback24().
	// _unkValue6      - Unknown. Rhythm section volume?
	// _unkValue7      - Unknown. Rhythm section volume?
	// _unkValue8      - Unknown. Rhythm section volume?
	// _unkValue9      - Unknown. Rhythm section volume?
	// _unkValue10     - Unknown. Rhythm section volume?
	// _unkValue11     - Unknown. Rhythm section volume?
	// _unkValue12     - Unknown. Rhythm section volume?
	// _unkValue13     - Unknown. Rhythm section volume?
	// _unkValue14     - Unknown. Rhythm section volume?
	// _unkValue15     - Unknown. Rhythm section volume?
	// _unkValue16     - Unknown. Rhythm section volume?
	// _unkValue17     - Unknown. Rhythm section volume?
	// _unkValue18     - Unknown. Rhythm section volume?
	// _unkValue19     - Unknown. Rhythm section volume?
	// _unkValue20     - Unknown. Rhythm section volume?
	// _freqTable[]     - Probably frequences for the 12-tone scale.
	// _unkTable2[]    - Unknown. Currently only used by updateCallback46()
	// _unkTable2_1[]  - One of the tables in _unkTable2[]
	// _unkTable2_2[]  - One of the tables in _unkTable2[]
	// _unkTable2_3[]  - One of the tables in _unkTable2[]

	int32 _samplesPerCallback;
	int32 _samplesPerCallbackRemainder;
	int32 _samplesTillCallback;
	int32 _samplesTillCallbackRemainder;

	int _curChannel;
	uint8 _soundTrigger;

	uint16 _rnd;

	uint8 _unkValue1;
	uint8 _unkValue2;
	uint8 _callbackTimer;
	uint8 _unkValue4;
	uint8 _unkValue5;
	uint8 _unkValue6;
	uint8 _unkValue7;
	uint8 _unkValue8;
	uint8 _unkValue9;
	uint8 _unkValue10;
	uint8 _unkValue11;
	uint8 _unkValue12;
	uint8 _unkValue13;
	uint8 _unkValue14;
	uint8 _unkValue15;
	uint8 _unkValue16;
	uint8 _unkValue17;
	uint8 _unkValue18;
	uint8 _unkValue19;
	uint8 _unkValue20;

	FM_OPL *_adlib;

	uint8 *_soundData;

	struct QueueEntry {
		QueueEntry() : data(0), id(0), volume(0) {}
		QueueEntry(uint8 *ptr, uint8 track, uint8 vol) : data(ptr), id(track), volume(vol) {}
		uint8 *data;
		uint8 id;
		uint8 volume;
	};

	QueueEntry _programQueue[16];
	int _programStartTimeout;
	int _programQueueStart, _programQueueEnd;
	bool _retrySounds;

	void adjustSfxData(uint8 *data, int volume);
	uint8 *_sfxPointer;
	int _sfxPriority;
	int _sfxVelocity;

	Channel _channels[10];

	uint8 _vibratoAndAMDepthBits;
	uint8 _rhythmSectionBits;

	uint8 _curRegOffset;
	uint8 _tempo;

	const uint8 *_tablePtr1;
	const uint8 *_tablePtr2;

	static const uint8 _regOffset[];
	static const uint16 _freqTable[];
	static const uint8 *const _unkTable2[];
	static const uint8 _unkTable2_1[];
	static const uint8 _unkTable2_2[];
	static const uint8 _unkTable2_3[];
	static const uint8 _pitchBendTables[][32];

	uint16 _syncJumpMask;

	Common::Mutex _mutex;
	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundHandle;

	uint8 _musicVolume, _sfxVolume;

	int _numPrograms;
	int _version;
};

AdLibDriver::AdLibDriver(Audio::Mixer *mixer, int version) {
	setupParserOpcodeTable();

	_version = version;
	_numPrograms = (_version == 1) ? 150 : ((_version == 4) ? 500 : 250);

	_mixer = mixer;

	_adlib = makeAdLibOPL(getRate());
	assert(_adlib);

	memset(_channels, 0, sizeof(_channels));
	_soundData = 0;

	_vibratoAndAMDepthBits = _curRegOffset = 0;

	_curChannel = _rhythmSectionBits = 0;
	_rnd = 0x1234;

	_tempo = 0;
	_soundTrigger = 0;
	_programStartTimeout = 0;

	_callbackTimer = 0xFF;
	_unkValue1 = _unkValue2 = _unkValue4 = _unkValue5 = 0;
	_unkValue6 = _unkValue7 = _unkValue8 = _unkValue9 = _unkValue10 = 0;
	_unkValue11 = _unkValue12 = _unkValue13 = _unkValue14 = _unkValue15 =
	_unkValue16 = _unkValue17 = _unkValue18 = _unkValue19 = _unkValue20 = 0;

	_tablePtr1 = _tablePtr2 = 0;

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_samplesPerCallback = getRate() / CALLBACKS_PER_SECOND;
	_samplesPerCallbackRemainder = getRate() % CALLBACKS_PER_SECOND;
	_samplesTillCallback = 0;
	_samplesTillCallbackRemainder = 0;

	_syncJumpMask = 0;

	_musicVolume = 0;
	_sfxVolume = 0;

	_sfxPointer = 0;

	_programQueueStart = _programQueueEnd = 0;
	_retrySounds = false;
}

AdLibDriver::~AdLibDriver() {
	_mixer->stopHandle(_soundHandle);
	OPLDestroy(_adlib);
	_adlib = 0;
}

void AdLibDriver::setMusicVolume(uint8 volume) {
	Common::StackLock lock(_mutex);

	_musicVolume = volume;

	for (uint i = 0; i < 6; ++i) {
		Channel &chan = _channels[i];
		chan.volumeModifier = volume;

		const uint8 regOffset = _regOffset[i];

		// Level Key Scaling / Total Level
		writeOPL(0x40 + regOffset, calculateOpLevel1(chan));
		writeOPL(0x43 + regOffset, calculateOpLevel2(chan));
	}

	// For now we use the music volume for both sfx and music in Kyra1 and EoB
	if (_version < 4) {
		_sfxVolume = volume;

		for (uint i = 6; i < 9; ++i) {
			Channel &chan = _channels[i];
			chan.volumeModifier = volume;

			const uint8 regOffset = _regOffset[i];

			// Level Key Scaling / Total Level
			writeOPL(0x40 + regOffset, calculateOpLevel1(chan));
			writeOPL(0x43 + regOffset, calculateOpLevel2(chan));
		}
	}
}

void AdLibDriver::setSfxVolume(uint8 volume) {
	// We only support sfx volume in version 4 games.
	if (_version < 4)
		return;

	Common::StackLock lock(_mutex);

	_sfxVolume = volume;

	for (uint i = 6; i < 9; ++i) {
		Channel &chan = _channels[i];
		chan.volumeModifier = volume;

		const uint8 regOffset = _regOffset[i];

		// Level Key Scaling / Total Level
		writeOPL(0x40 + regOffset, calculateOpLevel1(chan));
		writeOPL(0x43 + regOffset, calculateOpLevel2(chan));
	}
}

void AdLibDriver::initDriver() {
	Common::StackLock lock(_mutex);
	resetAdLibState();
}

void AdLibDriver::setSoundData(uint8 *data) {
	Common::StackLock lock(_mutex);

	// Drop all tracks that are still queued. These would point to the old
	// sound data.
	_programQueueStart = _programQueueEnd = 0;
	memset(_programQueue, 0, sizeof(_programQueue));

	if (_soundData) {
		delete[] _soundData;
		_soundData = _sfxPointer = 0;
	}

	_soundData = data;
}

void AdLibDriver::queueTrack(int track, int volume) {
	Common::StackLock lock(_mutex);

	uint8 *trackData = getProgram(track);
	if (!trackData)
		return;

	// Don't drop tracks in EoB. The queue is always full there if a couple of monsters are around.
	// If we drop the incoming tracks we get no sound effects, but tons of warnings instead.
	if (_version >= 3 && _programQueueEnd == _programQueueStart && _programQueue[_programQueueEnd].data != 0) {
		warning("AdLibDriver: Program queue full, dropping track %d", track);
		return;
	}

	_programQueue[_programQueueEnd] = QueueEntry(trackData, track, volume);
	_programQueueEnd = (_programQueueEnd + 1) & 15;
}

bool AdLibDriver::isChannelPlaying(int channel) const {
	Common::StackLock lock(_mutex);

	assert(channel >= 0 && channel <= 9);
	return (_channels[channel].dataptr != 0);
}

void AdLibDriver::stopAllChannels() {
	Common::StackLock lock(_mutex);

	for (int channel = 0; channel <= 9; ++channel) {
		_curChannel = channel;

		Channel &chan = _channels[_curChannel];
		chan.priority = 0;
		chan.dataptr = 0;

		if (channel != 9)
			noteOff(chan);
	}
	_retrySounds = false;
}

// timer callback

void AdLibDriver::callback() {
	Common::StackLock lock(_mutex);
	if (_programStartTimeout)
		--_programStartTimeout;
	else
		setupPrograms();
	executePrograms();

	uint8 temp = _callbackTimer;
	_callbackTimer += _tempo;
	if (_callbackTimer < temp) {
		if (!(--_unkValue2)) {
			_unkValue2 = _unkValue1;
			++_unkValue4;
		}
	}
}

void AdLibDriver::setupPrograms() {
	// If there is no program queued, we skip this.
	if (_programQueueStart == _programQueueEnd)
		return;

	uint8 *ptr = _programQueue[_programQueueStart].data;

	// The AdLib driver (in its old versions used for EOB) is not suitable for modern (fast) CPUs.
	// The stop sound track (track 0 which has a priority of 50) will often still be busy when the
	// next sound (with a lower priority) starts which will cause that sound to be skipped. We simply
	// restart incoming sounds during stop sound execution.
	// UPDATE: This stilly applies after introduction of the _programQueue.
	QueueEntry retrySound;
	if (_version < 3 && _programQueue[_programQueueStart].id == 0)
		_retrySounds = true;
	else if (_retrySounds)
		retrySound = _programQueue[_programQueueStart];

	// Adjust data in case we hit a sound effect.
	adjustSfxData(ptr, _programQueue[_programQueueStart].volume);

	// Clear the queue entry
	_programQueue[_programQueueStart].data = 0;
	_programQueueStart = (_programQueueStart + 1) & 15;

	const int chan = *ptr++;
	const int priority = *ptr++;

	// Only start this sound if its priority is higher than the one
	// already playing.

	Channel &channel = _channels[chan];

	if (priority >= channel.priority) {
		initChannel(channel);
		channel.priority = priority;
		channel.dataptr = ptr;
		channel.tempo = 0xFF;
		channel.position = 0xFF;
		channel.duration = 1;

		if (chan <= 5)
			channel.volumeModifier = _musicVolume;
		else
			channel.volumeModifier = _sfxVolume;

		unkOutput2(chan);

		// We need to wait two callback calls till we can start another track.
		// This is (probably) required to assure that the sfx are started with
		// the correct priority and velocity.
		_programStartTimeout = 2;

		retrySound = QueueEntry();
	}

	if (retrySound.data) {
		debugC(9, kDebugLevelSound, "AdLibDriver::setupPrograms(): WORKAROUND - Restarting skipped sound %d)", retrySound.id);
		queueTrack(retrySound.id, retrySound.volume);
	}
}

void AdLibDriver::adjustSfxData(uint8 *ptr, int volume) {
	// Check whether we need to reset the data of an old sfx which has been
	// started.
	if (_sfxPointer) {
		_sfxPointer[1] = _sfxPriority;
		_sfxPointer[3] = _sfxVelocity;
		_sfxPointer = 0;
	}

	// Only music tracks are started on channel 9, thus we need to make sure
	// we do not have a music track here.
	if (*ptr == 9)
		return;

	// Store the pointer so we can reset the data when a new program is started.
	_sfxPointer = ptr;

	// Store the old values.
	_sfxPriority = ptr[1];
	_sfxVelocity = ptr[3];

	// Adjust the values.
	if (volume != 0xff) {
		if (_version >= 3) {
			int newVal = ((((ptr[3]) + 63) * volume) >> 8) & 0xFF;
			ptr[3] = -newVal + 63;
			ptr[1] = ((ptr[1] * volume) >> 8) & 0xFF;
		} else {
			int newVal = ((_sfxVelocity << 2) ^ 0xff) * volume;
			ptr[3] = (newVal >> 10) ^ 0x3f;
			ptr[1] = newVal >> 11;
		}
	}
}

// A few words on opcode parsing and timing:
//
// First of all, We simulate a timer callback 72 times per second. Each timeout
// we update each channel that has something to play.
//
// Each channel has its own individual tempo, which is added to its position.
// This will frequently cause the position to "wrap around" but that is
// intentional. In fact, it's the signal to go ahead and do more stuff with
// that channel.
//
// Each channel also has a duration, indicating how much time is left on the
// its current task. This duration is decreased by one. As long as it still has
// not reached zero, the only thing that can happen is that the note is turned
// off depending on manual or automatic note spacing. Once the duration reaches
// zero, a new set of musical opcodes are executed.
//
// An opcode is one byte, followed by a variable number of parameters. Since
// most opcodes have at least one one-byte parameter, we read that as well. Any
// opcode that doesn't have that one parameter is responsible for moving the
// data pointer back again.
//
// If the most significant bit of the opcode is 1, it's a function; call it.
// The opcode functions return either 0 (continue), 1 (stop) or 2 (stop, and do
// not run the effects callbacks).
//
// If the most significant bit of the opcode is 0, it's a note, and the first
// parameter is its duration. (There are cases where the duration is modified
// but that's an exception.) The note opcode is assumed to return 1, and is the
// last opcode unless its duration is zero.
//
// Finally, most of the times that the callback is called, it will invoke the
// effects callbacks. The final opcode in a set can prevent this, if it's a
// function and it returns anything other than 1.

void AdLibDriver::executePrograms() {
	// Each channel runs its own program. There are ten channels: One for
	// each AdLib channel (0-8), plus one "control channel" (9) which is
	// the one that tells the other channels what to do.

	// This is where we ensure that channels that are made to jump "in
	// sync" do so.

	if (_syncJumpMask) {
		bool forceUnlock = true;

		for (_curChannel = 9; _curChannel >= 0; --_curChannel) {
			if ((_syncJumpMask & (1 << _curChannel)) == 0)
				continue;

			if (_channels[_curChannel].dataptr && !_channels[_curChannel].lock)
				forceUnlock = false;
		}

		if (forceUnlock) {
			for (_curChannel = 9; _curChannel >= 0; --_curChannel)
				if (_syncJumpMask & (1 << _curChannel))
					_channels[_curChannel].lock = false;
		}
	}

	for (_curChannel = 9; _curChannel >= 0; --_curChannel) {
		int result = 1;

		if (!_channels[_curChannel].dataptr)
			continue;

		if (_channels[_curChannel].lock && (_syncJumpMask & (1 << _curChannel)))
			continue;

		Channel &channel = _channels[_curChannel];
		if (_curChannel == 9)
			_curRegOffset = 0;
		else
			_curRegOffset = _regOffset[_curChannel];

		if (channel.tempoReset)
			channel.tempo = _tempo;

		uint8 backup = channel.position;
		channel.position += channel.tempo;
		if (channel.position < backup) {
			if (--channel.duration) {
				if (channel.duration == channel.spacing2)
					noteOff(channel);
				if (channel.duration == channel.spacing1 && _curChannel != 9)
					noteOff(channel);
			} else {
				// An opcode is not allowed to modify its own
				// data pointer except through the 'dataptr'
				// parameter. To enforce that, we have to work
				// on a copy of the data pointer.
				//
				// This fixes a subtle music bug where the
				// wrong music would play when getting the
				// quill in Kyra 1.
				uint8 *dataptr = channel.dataptr;
				while (dataptr) {
					uint8 opcode = *dataptr++;
					uint8 param = *dataptr++;

					if (opcode & 0x80) {
						opcode &= 0x7F;
						if (opcode >= _parserOpcodeTableSize)
							opcode = _parserOpcodeTableSize - 1;
						debugC(9, kDebugLevelSound, "Calling opcode '%s' (%d) (channel: %d)", _parserOpcodeTable[opcode].name, opcode, _curChannel);
						result = (this->*(_parserOpcodeTable[opcode].function))(dataptr, channel, param);
						channel.dataptr = dataptr;
						if (result)
							break;
					} else {
						debugC(9, kDebugLevelSound, "Note on opcode 0x%02X (duration: %d) (channel: %d)", opcode, param, _curChannel);
						setupNote(opcode, channel);
						noteOn(channel);
						setupDuration(param, channel);
						if (param) {
							// We need to make sure we are always running the
							// effects after this. Otherwise some sounds are
							// wrong. Like the sfx when bumping into a wall in
							// LoL.
							result = 1;
							channel.dataptr = dataptr;
							break;
						}
					}
				}
			}
		}

		if (result == 1) {
			if (channel.primaryEffect)
				(this->*(channel.primaryEffect))(channel);
			if (channel.secondaryEffect)
				(this->*(channel.secondaryEffect))(channel);
		}
	}
}

//

void AdLibDriver::resetAdLibState() {
	debugC(9, kDebugLevelSound, "resetAdLibState()");
	_rnd = 0x1234;

	// Authorize the control of the waveforms
	writeOPL(0x01, 0x20);

	// Select FM music mode
	writeOPL(0x08, 0x00);

	// I would guess the main purpose of this is to turn off the rhythm,
	// thus allowing us to use 9 melodic voices instead of 6.
	writeOPL(0xBD, 0x00);

	int loop = 10;
	while (loop--) {
		if (loop != 9) {
			// Silence the channel
			writeOPL(0x40 + _regOffset[loop], 0x3F);
			writeOPL(0x43 + _regOffset[loop], 0x3F);
		}
		initChannel(_channels[loop]);
	}
}

// Old calling style: output0x388(0xABCD)
// New calling style: writeOPL(0xAB, 0xCD)

void AdLibDriver::writeOPL(byte reg, byte val) {
	OPLWriteReg(_adlib, reg, val);
}

void AdLibDriver::initChannel(Channel &channel) {
	debugC(9, kDebugLevelSound, "initChannel(%lu)", (long)(&channel - _channels));
	memset(&channel.dataptr, 0, sizeof(Channel) - ((char *)&channel.dataptr - (char *)&channel));

	channel.tempo = 0xFF;
	channel.priority = 0;
	// normally here are nullfuncs but we set 0 for now
	channel.primaryEffect = 0;
	channel.secondaryEffect = 0;
	channel.spacing1 = 1;
	channel.lock = false;
}

void AdLibDriver::noteOff(Channel &channel) {
	debugC(9, kDebugLevelSound, "noteOff(%lu)", (long)(&channel - _channels));

	// The control channel has no corresponding AdLib channel

	if (_curChannel >= 9)
		return;

	// When the rhythm section is enabled, channels 6, 7 and 8 are special.

	if (_rhythmSectionBits && _curChannel >= 6)
		return;

	// This means the "Key On" bit will always be 0
	channel.regBx &= 0xDF;

	// Octave / F-Number / Key-On
	writeOPL(0xB0 + _curChannel, channel.regBx);
}

void AdLibDriver::unkOutput2(uint8 chan) {
	debugC(9, kDebugLevelSound, "unkOutput2(%d)", chan);

	// The control channel has no corresponding AdLib channel

	if (chan >= 9)
		return;

	// I believe this has to do with channels 6, 7, and 8 being special
	// when AdLib's rhythm section is enabled.

	if (_rhythmSectionBits && chan >= 6)
		return;

	uint8 offset = _regOffset[chan];

	// The channel is cleared: First the attack/delay rate, then the
	// sustain level/release rate, and finally the note is turned off.

	writeOPL(0x60 + offset, 0xFF);
	writeOPL(0x63 + offset, 0xFF);

	writeOPL(0x80 + offset, 0xFF);
	writeOPL(0x83 + offset, 0xFF);

	writeOPL(0xB0 + chan, 0x00);

	// ...and then the note is turned on again, with whatever value is
	// still lurking in the A0 + chan register, but everything else -
	// including the two most significant frequency bit, and the octave -
	// set to zero.
	//
	// This is very strange behavior, and causes problems with the ancient
	// FMOPL code we borrowed from AdPlug. I've added a workaround. See
	// fmopl.cpp for more details.
	//
	// More recent versions of the MAME FMOPL don't seem to have this
	// problem, but cannot currently be used because of licensing and
	// performance issues.
	//
	// Ken Silverman's AdLib emulator (which can be found on his Web page -
	// http://www.advsys.net/ken - and as part of AdPlug) also seems to be
	// immune, but is apparently not as feature complete as MAME's.

	writeOPL(0xB0 + chan, 0x20);
}

// I believe this is a random number generator. It actually does seem to
// generate an even distribution of almost all numbers from 0 through 65535,
// though in my tests some numbers were never generated.

uint16 AdLibDriver::getRandomNr() {
	_rnd += 0x9248;
	uint16 lowBits = _rnd & 7;
	_rnd >>= 3;
	_rnd |= (lowBits << 13);
	return _rnd;
}

void AdLibDriver::setupDuration(uint8 duration, Channel &channel) {
	debugC(9, kDebugLevelSound, "setupDuration(%d, %lu)", duration, (long)(&channel - _channels));
	if (channel.durationRandomness) {
		channel.duration = duration + (getRandomNr() & channel.durationRandomness);
		return;
	}
	if (channel.fractionalSpacing)
		channel.spacing2 = (duration >> 3) * channel.fractionalSpacing;
	channel.duration = duration;
}

// This function may or may not play the note. It's usually followed by a call
// to noteOn(), which will always play the current note.

void AdLibDriver::setupNote(uint8 rawNote, Channel &channel, bool flag) {
	debugC(9, kDebugLevelSound, "setupNote(%d, %lu)", rawNote, (long)(&channel - _channels));

	if (_curChannel >= 9)
		return;

	channel.rawNote = rawNote;

	int8 note = (rawNote & 0x0F) + channel.baseNote;
	int8 octave = ((rawNote + channel.baseOctave) >> 4) & 0x0F;

	// There are only twelve notes. If we go outside that, we have to
	// adjust the note and octave.

	if (note >= 12) {
		note -= 12;
		octave++;
	} else if (note < 0) {
		note += 12;
		octave--;
	}

	// The calculation of frequency looks quite different from the original
	// disassembly at a first glance, but when you consider that the
	// largest possible value would be 0x0246 + 0xFF + 0x47 (and that's if
	// baseFreq is unsigned), freq is still a 10-bit value, just as it
	// should be to fit in the Ax and Bx registers.
	//
	// If it were larger than that, it could have overflowed into the
	// octave bits, and that could possibly have been used in some sound.
	// But as it is now, I can't see any way it would happen.

	uint16 freq = _freqTable[note] + channel.baseFreq;

	// When called from callback 41, the behavior is slightly different:
	// We adjust the frequency, even when channel.pitchBend is 0.

	if (channel.pitchBend || flag) {
		const uint8 *table;

		if (channel.pitchBend >= 0) {
			table = _pitchBendTables[(channel.rawNote & 0x0F) + 2];
			freq += table[channel.pitchBend];
		} else {
			table = _pitchBendTables[channel.rawNote & 0x0F];
			freq -= table[-channel.pitchBend];
		}
	}

	channel.regAx = freq & 0xFF;
	channel.regBx = (channel.regBx & 0x20) | (octave << 2) | ((freq >> 8) & 0x03);

	// Keep the note on or off
	writeOPL(0xA0 + _curChannel, channel.regAx);
	writeOPL(0xB0 + _curChannel, channel.regBx);
}

void AdLibDriver::setupInstrument(uint8 regOffset, uint8 *dataptr, Channel &channel) {
	debugC(9, kDebugLevelSound, "setupInstrument(%d, %p, %lu)", regOffset, (const void *)dataptr, (long)(&channel - _channels));

	if (_curChannel >= 9)
		return;

	// Amplitude Modulation / Vibrato / Envelope Generator Type /
	// Keyboard Scaling Rate / Modulator Frequency Multiple
	writeOPL(0x20 + regOffset, *dataptr++);
	writeOPL(0x23 + regOffset, *dataptr++);

	uint8 temp = *dataptr++;

	// Feedback / Algorithm

	// It is very likely that _curChannel really does refer to the same
	// channel as regOffset, but there's only one Cx register per channel.

	writeOPL(0xC0 + _curChannel, temp);

	// The algorithm bit. I don't pretend to understand this fully, but
	// "If set to 0, operator 1 modulates operator 2. In this case,
	// operator 2 is the only one producing sound. If set to 1, both
	// operators produce sound directly. Complex sounds are more easily
	// created if the algorithm is set to 0."

	channel.twoChan = temp & 1;

	// Waveform Select
	writeOPL(0xE0 + regOffset, *dataptr++);
	writeOPL(0xE3 + regOffset, *dataptr++);

	channel.opLevel1 = *dataptr++;
	channel.opLevel2 = *dataptr++;

	// Level Key Scaling / Total Level
	writeOPL(0x40 + regOffset, calculateOpLevel1(channel));
	writeOPL(0x43 + regOffset, calculateOpLevel2(channel));

	// Attack Rate / Decay Rate
	writeOPL(0x60 + regOffset, *dataptr++);
	writeOPL(0x63 + regOffset, *dataptr++);

	// Sustain Level / Release Rate
	writeOPL(0x80 + regOffset, *dataptr++);
	writeOPL(0x83 + regOffset, *dataptr++);
}

// Apart from playing the note, this function also updates the variables for
// primary effect 2.

void AdLibDriver::noteOn(Channel &channel) {
	debugC(9, kDebugLevelSound, "noteOn(%lu)", (long)(&channel - _channels));

	// The "note on" bit is set, and the current note is played.

	if (_curChannel >= 9)
		return;

	channel.regBx |= 0x20;
	writeOPL(0xB0 + _curChannel, channel.regBx);

	int8 shift = 9 - channel.unk33;
	uint16 temp = channel.regAx | (channel.regBx << 8);
	channel.unk37 = ((temp & 0x3FF) >> shift) & 0xFF;
	channel.unk38 = channel.unk36;
}

void AdLibDriver::adjustVolume(Channel &channel) {
	debugC(9, kDebugLevelSound, "adjustVolume(%lu)", (long)(&channel - _channels));

	if (_curChannel >= 9)
		return;

	// Level Key Scaling / Total Level

	writeOPL(0x43 + _regOffset[_curChannel], calculateOpLevel2(channel));
	if (channel.twoChan)
		writeOPL(0x40 + _regOffset[_curChannel], calculateOpLevel1(channel));
}

// This is presumably only used for some sound effects, e.g. Malcolm blowing up
// the trees in the intro (but not the effect where he "booby-traps" the big
// tree) and turning Kallak to stone. Related functions and variables:
//
// update_setupPrimaryEffect1()
//    - Initializes unk29, unk30 and unk31
//    - unk29 is not further modified
//    - unk30 is not further modified, except by update_removePrimaryEffect1()
//
// update_removePrimaryEffect1()
//    - Deinitializes unk30
//
// unk29 - determines how often the notes are played
// unk30 - modifies the frequency
// unk31 - determines how often the notes are played

void AdLibDriver::primaryEffect1(Channel &channel) {
	debugC(9, kDebugLevelSound, "Calling primaryEffect1 (channel: %d)", _curChannel);

	if (_curChannel >= 9)
		return;

	uint8 temp = channel.unk31;
	channel.unk31 += channel.unk29;
	if (channel.unk31 >= temp)
		return;

	// Initialize unk1 to the current frequency
	int16 unk1 = ((channel.regBx & 3) << 8) | channel.regAx;

	// This is presumably to shift the "note on" bit so far to the left
	// that it won't be affected by any of the calculations below.
	int16 unk2 = ((channel.regBx & 0x20) << 8) | (channel.regBx & 0x1C);

	int16 unk3 = (int16)channel.unk30;

	if (unk3 >= 0) {
		unk1 += unk3;
		if (unk1 >= 734) {
			// The new frequency is too high. Shift it down and go
			// up one octave.
			unk1 >>= 1;
			if (!(unk1 & 0x3FF))
				++unk1;
			unk2 = (unk2 & 0xFF00) | ((unk2 + 4) & 0xFF);
			unk2 &= 0xFF1C;
		}
	} else {
		unk1 += unk3;
		if (unk1 < 388) {
			// The new frequency is too low. Shift it up and go
			// down one octave.
			unk1 <<= 1;
			if (!(unk1 & 0x3FF))
				--unk1;
			unk2 = (unk2 & 0xFF00) | ((unk2 - 4) & 0xFF);
			unk2 &= 0xFF1C;
		}
	}

	// Make sure that the new frequency is still a 10-bit value.
	unk1 &= 0x3FF;

	writeOPL(0xA0 + _curChannel, unk1 & 0xFF);
	channel.regAx = unk1 & 0xFF;

	// Shift down the "note on" bit again.
	uint8 value = unk1 >> 8;
	value |= (unk2 >> 8) & 0xFF;
	value |= unk2 & 0xFF;

	writeOPL(0xB0 + _curChannel, value);
	channel.regBx = value;
}

// This is presumably only used for some sound effects, e.g. Malcolm entering
// and leaving Kallak's hut. Related functions and variables:
//
// update_setupPrimaryEffect2()
//    - Initializes unk32, unk33, unk34, unk35 and unk36
//    - unk32 is not further modified
//    - unk33 is not further modified
//    - unk34 is a countdown that gets reinitialized to unk35 on zero
//    - unk35 is based on unk34 and not further modified
//    - unk36 is not further modified
//
// noteOn()
//    - Plays the current note
//    - Updates unk37 with a new (lower?) frequency
//    - Copies unk36 to unk38. The unk38 variable is a countdown.
//
// unk32 - determines how often the notes are played
// unk33 - modifies the frequency
// unk34 - countdown, updates frequency on zero
// unk35 - initializer for unk34 countdown
// unk36 - initializer for unk38 countdown
// unk37 - frequency
// unk38 - countdown, begins playing on zero
// unk41 - determines how often the notes are played
//
// Note that unk41 is never initialized. Not that it should matter much, but it
// is a bit sloppy.

void AdLibDriver::primaryEffect2(Channel &channel) {
	debugC(9, kDebugLevelSound, "Calling primaryEffect2 (channel: %d)", _curChannel);

	if (_curChannel >= 9)
		return;

	if (channel.unk38) {
		--channel.unk38;
		return;
	}

	uint8 temp = channel.unk41;
	channel.unk41 += channel.unk32;
	if (channel.unk41 < temp) {
		uint16 unk1 = channel.unk37;
		if (!(--channel.unk34)) {
			unk1 ^= 0xFFFF;
			++unk1;
			channel.unk37 = unk1;
			channel.unk34 = channel.unk35;
		}

		uint16 unk2 = (channel.regAx | (channel.regBx << 8)) & 0x3FF;
		unk2 += unk1;

		channel.regAx = unk2 & 0xFF;
		channel.regBx = (channel.regBx & 0xFC) | (unk2 >> 8);

		// Octave / F-Number / Key-On
		writeOPL(0xA0 + _curChannel, channel.regAx);
		writeOPL(0xB0 + _curChannel, channel.regBx);
	}
}

// I don't know where this is used. The same operation is performed several
// times on the current channel, using a chunk of the _soundData[] buffer for
// parameters. The parameters are used starting at the end of the chunk.
//
// Since we use _curRegOffset to specify the final register, it's quite
// unlikely that this function is ever used to play notes. It's probably only
// used to modify the sound. Another thing that supports this idea is that it
// can be combined with any of the effects callbacks above.
//
// Related functions and variables:
//
// update_setupSecondaryEffect1()
//    - Initialies unk18, unk19, unk20, unk21, unk22 and offset
//    - unk19 is not further modified
//    - unk20 is not further modified
//    - unk22 is not further modified
//    - offset is not further modified
//
// unk18 -  determines how often the operation is performed
// unk19 -  determines how often the operation is performed
// unk20 -  the start index into the data chunk
// unk21 -  the current index into the data chunk
// unk22 -  the operation to perform
// offset - the offset to the data chunk

void AdLibDriver::secondaryEffect1(Channel &channel) {
	debugC(9, kDebugLevelSound, "Calling secondaryEffect1 (channel: %d)", _curChannel);

	if (_curChannel >= 9)
		return;

	uint8 temp = channel.unk18;
	channel.unk18 += channel.unk19;
	if (channel.unk18 < temp) {
		if (--channel.unk21 < 0)
			channel.unk21 = channel.unk20;
		writeOPL(channel.unk22 + _curRegOffset, _soundData[channel.offset + channel.unk21]);
	}
}

uint8 AdLibDriver::calculateOpLevel1(Channel &channel) {
	int8 value = channel.opLevel1 & 0x3F;

	if (channel.twoChan) {
		value += channel.opExtraLevel1;
		value += channel.opExtraLevel2;

		uint16 level3 = (channel.opExtraLevel3 ^ 0x3F) * channel.volumeModifier;
		if (level3) {
			level3 += 0x3F;
			level3 >>= 8;
		}

		value += level3 ^ 0x3F;
	}

	value = CLIP<int8>(value, 0, 0x3F);

	if (!channel.volumeModifier)
		value = 0x3F;

	// Preserve the scaling level bits from opLevel1

	return checkValue(value) | (channel.opLevel1 & 0xC0);
}

uint8 AdLibDriver::calculateOpLevel2(Channel &channel) {
	int8 value = channel.opLevel2 & 0x3F;

	value += channel.opExtraLevel1;
	value += channel.opExtraLevel2;

	uint16 level3 = (channel.opExtraLevel3 ^ 0x3F) * channel.volumeModifier;
	if (level3) {
		level3 += 0x3F;
		level3 >>= 8;
	}

	value += level3 ^ 0x3F;

	value = CLIP<int8>(value, 0, 0x3F);

	if (!channel.volumeModifier)
		value = 0x3F;

	// Preserve the scaling level bits from opLevel2

	return checkValue(value) | (channel.opLevel2 & 0xC0);
}

// parser opcodes

int AdLibDriver::update_setRepeat(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.repeatCounter = value;
	return 0;
}

int AdLibDriver::update_checkRepeat(uint8 *&dataptr, Channel &channel, uint8 value) {
	++dataptr;
	if (--channel.repeatCounter) {
		int16 add = READ_LE_UINT16(dataptr - 2);
		dataptr += add;
	}
	return 0;
}

int AdLibDriver::update_setupProgram(uint8 *&dataptr, Channel &channel, uint8 value) {
	if (value == 0xFF)
		return 0;

	uint8 *ptr = getProgram(value);
	//TODO: Check in LoL CD AdLib driver
	if (!ptr)
		return 0;
	uint8 chan = *ptr++;
	uint8 priority = *ptr++;

	Channel &channel2 = _channels[chan];

	if (priority >= channel2.priority) {
		// We keep new tracks from being started for two further iterations of
		// the callback. This assures the correct velocity is used for this
		// program.
		_programStartTimeout = 2;
		initChannel(channel2);
		channel2.priority = priority;
		channel2.dataptr = ptr;
		channel2.tempo = 0xFF;
		channel2.position = 0xFF;
		channel2.duration = 1;

		if (chan <= 5)
			channel2.volumeModifier = _musicVolume;
		else
			channel2.volumeModifier = _sfxVolume;

		unkOutput2(chan);
	}

	return 0;
}

int AdLibDriver::update_setNoteSpacing(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.spacing1 = value;
	return 0;
}

int AdLibDriver::update_jump(uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	int16 add = READ_LE_UINT16(dataptr); dataptr += 2;
	if (_version == 1)
		dataptr = _soundData + add - 191;
	else
		dataptr += add;
	if (_syncJumpMask & (1 << (&channel - _channels)))
		channel.lock = true;
	return 0;
}

int AdLibDriver::update_jumpToSubroutine(uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	int16 add = READ_LE_UINT16(dataptr); dataptr += 2;
	channel.dataptrStack[channel.dataptrStackPos++] = dataptr;
	if (_version < 3)
		dataptr = _soundData + add - 191;
	else
		dataptr += add;
	return 0;
}

int AdLibDriver::update_returnFromSubroutine(uint8 *&dataptr, Channel &channel, uint8 value) {
	dataptr = channel.dataptrStack[--channel.dataptrStackPos];
	return 0;
}

int AdLibDriver::update_setBaseOctave(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.baseOctave = value;
	return 0;
}

int AdLibDriver::update_stopChannel(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.priority = 0;
	if (_curChannel != 9)
		noteOff(channel);
	dataptr = 0;
	return 2;
}

int AdLibDriver::update_playRest(uint8 *&dataptr, Channel &channel, uint8 value) {
	setupDuration(value, channel);
	noteOff(channel);
	return (value != 0);
}

int AdLibDriver::update_writeAdLib(uint8 *&dataptr, Channel &channel, uint8 value) {
	writeOPL(value, *dataptr++);
	return 0;
}

int AdLibDriver::update_setupNoteAndDuration(uint8 *&dataptr, Channel &channel, uint8 value) {
	setupNote(value, channel);
	value = *dataptr++;
	setupDuration(value, channel);
	return (value != 0);
}

int AdLibDriver::update_setBaseNote(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.baseNote = value;
	return 0;
}

int AdLibDriver::update_setupSecondaryEffect1(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.unk18 = value;
	channel.unk19 = value;
	channel.unk20 = channel.unk21 = *dataptr++;
	channel.unk22 = *dataptr++;
	// WORKAROUND: The original code reads a true offset which later gets translated via xlat (in
	// the current segment). This means that the outcome depends on the sound data offset.
	// Unfortunately this offset is different in most implementations of the audio driver and
	// probably also different from the offset assumed by the sequencer.
	// It seems that the driver assumes an offset of 191 which is wrong for all the game driver
	// implementations.
	// This bug has probably not been noticed, since the effect is hardly used and the sounds are
	// not necessarily worse. I noticed the difference between ScummVM and DOSBox for the EOB II
	// teleporter sound. I also found the location of the table which is supposed to be used here
	// (simple enough: it is located at the end of the track after the 0x88 ending opcode).
	// Teleporters in EOB I and II now sound exactly the same which I am sure was the intended way,
	// since the sound data is exactly the same.
	// In DOSBox the teleporters will sound different in EOB I and II, due to different sound
	// data offsets.
	channel.offset = READ_LE_UINT16(dataptr) - 191; dataptr += 2;
	channel.secondaryEffect = &AdLibDriver::secondaryEffect1;
	return 0;
}

int AdLibDriver::update_stopOtherChannel(uint8 *&dataptr, Channel &channel, uint8 value) {
	Channel &channel2 = _channels[value];
	channel2.duration = 0;
	channel2.priority = 0;
	channel2.dataptr = 0;
	return 0;
}

int AdLibDriver::update_waitForEndOfProgram(uint8 *&dataptr, Channel &channel, uint8 value) {
	uint8 *ptr = getProgram(value);
	uint8 chan = *ptr;

	if (!_channels[chan].dataptr)
		return 0;

	dataptr -= 2;
	return 2;
}

int AdLibDriver::update_setupInstrument(uint8 *&dataptr, Channel &channel, uint8 value) {
	setupInstrument(_curRegOffset, getInstrument(value), channel);
	return 0;
}

int AdLibDriver::update_setupPrimaryEffect1(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.unk29 = value;
	channel.unk30 = READ_BE_UINT16(dataptr);
	dataptr += 2;
	channel.primaryEffect = &AdLibDriver::primaryEffect1;
	channel.unk31 = 0xFF;
	return 0;
}

int AdLibDriver::update_removePrimaryEffect1(uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	channel.primaryEffect = 0;
	channel.unk30 = 0;
	return 0;
}

int AdLibDriver::update_setBaseFreq(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.baseFreq = value;
	return 0;
}

int AdLibDriver::update_setupPrimaryEffect2(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.unk32 = value;
	channel.unk33 = *dataptr++;
	uint8 temp = *dataptr++;
	channel.unk34 = temp + 1;
	channel.unk35 = temp << 1;
	channel.unk36 = *dataptr++;
	channel.primaryEffect = &AdLibDriver::primaryEffect2;
	return 0;
}

int AdLibDriver::update_setPriority(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.priority = value;
	return 0;
}

int AdLibDriver::updateCallback23(uint8 *&dataptr, Channel &channel, uint8 value) {
	value >>= 1;
	_unkValue1 = _unkValue2 = value;
	_callbackTimer = 0xFF;
	_unkValue4 = _unkValue5 = 0;
	return 0;
}

int AdLibDriver::updateCallback24(uint8 *&dataptr, Channel &channel, uint8 value) {
	if (_unkValue5) {
		if (_unkValue4 & value) {
			_unkValue5 = 0;
			return 0;
		}
	}

	if (!(value & _unkValue4))
		++_unkValue5;

	dataptr -= 2;
	channel.duration = 1;
	return 2;
}

int AdLibDriver::update_setExtraLevel1(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.opExtraLevel1 = value;
	adjustVolume(channel);
	return 0;
}

int AdLibDriver::update_setupDuration(uint8 *&dataptr, Channel &channel, uint8 value) {
	setupDuration(value, channel);
	return (value != 0);
}

int AdLibDriver::update_playNote(uint8 *&dataptr, Channel &channel, uint8 value) {
	setupDuration(value, channel);
	noteOn(channel);
	return (value != 0);
}

int AdLibDriver::update_setFractionalNoteSpacing(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.fractionalSpacing = value & 7;
	return 0;
}

int AdLibDriver::update_setTempo(uint8 *&dataptr, Channel &channel, uint8 value) {
	_tempo = value;
	return 0;
}

int AdLibDriver::update_removeSecondaryEffect1(uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	channel.secondaryEffect = 0;
	return 0;
}

int AdLibDriver::update_setChannelTempo(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.tempo = value;
	return 0;
}

int AdLibDriver::update_setExtraLevel3(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.opExtraLevel3 = value;
	return 0;
}

int AdLibDriver::update_setExtraLevel2(uint8 *&dataptr, Channel &channel, uint8 value) {
	int channelBackUp = _curChannel;

	_curChannel = value;
	Channel &channel2 = _channels[value];
	channel2.opExtraLevel2 = *dataptr++;
	adjustVolume(channel2);

	_curChannel = channelBackUp;
	return 0;
}

int AdLibDriver::update_changeExtraLevel2(uint8 *&dataptr, Channel &channel, uint8 value) {
	int channelBackUp = _curChannel;

	_curChannel = value;
	Channel &channel2 = _channels[value];
	channel2.opExtraLevel2 += *dataptr++;
	adjustVolume(channel2);

	_curChannel = channelBackUp;
	return 0;
}

// Apart from initializing to zero, these two functions are the only ones that
// modify _vibratoAndAMDepthBits.

int AdLibDriver::update_setAMDepth(uint8 *&dataptr, Channel &channel, uint8 value) {
	if (value & 1)
		_vibratoAndAMDepthBits |= 0x80;
	else
		_vibratoAndAMDepthBits &= 0x7F;

	writeOPL(0xBD, _vibratoAndAMDepthBits);
	return 0;
}

int AdLibDriver::update_setVibratoDepth(uint8 *&dataptr, Channel &channel, uint8 value) {
	if (value & 1)
		_vibratoAndAMDepthBits |= 0x40;
	else
		_vibratoAndAMDepthBits &= 0xBF;

	writeOPL(0xBD, _vibratoAndAMDepthBits);
	return 0;
}

int AdLibDriver::update_changeExtraLevel1(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.opExtraLevel1 += value;
	adjustVolume(channel);
	return 0;
}

int AdLibDriver::updateCallback38(uint8 *&dataptr, Channel &channel, uint8 value) {
	int channelBackUp = _curChannel;

	_curChannel = value;
	Channel &channel2 = _channels[value];
	channel2.duration = channel2.priority = 0;
	channel2.dataptr = 0;
	channel2.opExtraLevel2 = 0;

	if (value != 9) {
		uint8 outValue = _regOffset[value];

		// Feedback strength / Connection type
		writeOPL(0xC0 + _curChannel, 0x00);

		// Key scaling level / Operator output level
		writeOPL(0x43 + outValue, 0x3F);

		// Sustain Level / Release Rate
		writeOPL(0x83 + outValue, 0xFF);

		// Key On / Octave / Frequency
		writeOPL(0xB0 + _curChannel, 0x00);
	}

	_curChannel = channelBackUp;
	return 0;
}

int AdLibDriver::updateCallback39(uint8 *&dataptr, Channel &channel, uint8 value) {
	if (_curChannel >= 9)
		return 0;

	uint16 unk = *dataptr++;
	unk |= value << 8;
	unk &= getRandomNr();

	uint16 unk2 = ((channel.regBx & 0x1F) << 8) | channel.regAx;
	unk2 += unk;
	unk2 |= ((channel.regBx & 0x20) << 8);

	// Frequency
	writeOPL(0xA0 + _curChannel, unk2 & 0xFF);

	// Key On / Octave / Frequency
	writeOPL(0xB0 + _curChannel, (unk2 & 0xFF00) >> 8);

	return 0;
}

int AdLibDriver::update_removePrimaryEffect2(uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	channel.primaryEffect = 0;
	return 0;
}

int AdLibDriver::update_pitchBend(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.pitchBend = value;
	setupNote(channel.rawNote, channel, true);
	return 0;
}

int AdLibDriver::update_resetToGlobalTempo(uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	channel.tempo = _tempo;
	return 0;
}

int AdLibDriver::update_nop(uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	return 0;
}

int AdLibDriver::update_setDurationRandomness(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.durationRandomness = value;
	return 0;
}

int AdLibDriver::update_changeChannelTempo(uint8 *&dataptr, Channel &channel, uint8 value) {
	int tempo = channel.tempo + (int8)value;

	if (tempo <= 0)
		tempo = 1;
	else if (tempo > 255)
		tempo = 255;

	channel.tempo = tempo;
	return 0;
}

int AdLibDriver::updateCallback46(uint8 *&dataptr, Channel &channel, uint8 value) {
	uint8 entry = *dataptr++;
	_tablePtr1 = _unkTable2[entry++];
	_tablePtr2 = _unkTable2[entry];
	if (value == 2) {
		// Frequency
		writeOPL(0xA0, _tablePtr2[0]);
	}
	return 0;
}

int AdLibDriver::update_setupRhythmSection(uint8 *&dataptr, Channel &channel, uint8 value) {
	int channelBackUp = _curChannel;
	int regOffsetBackUp = _curRegOffset;

	_curChannel = 6;
	_curRegOffset = _regOffset[6];

	setupInstrument(_curRegOffset, getInstrument(value), channel);
	_unkValue6 = channel.opLevel2;

	_curChannel = 7;
	_curRegOffset = _regOffset[7];

	setupInstrument(_curRegOffset, getInstrument(*dataptr++), channel);
	_unkValue7 = channel.opLevel1;
	_unkValue8 = channel.opLevel2;

	_curChannel = 8;
	_curRegOffset = _regOffset[8];

	setupInstrument(_curRegOffset, getInstrument(*dataptr++), channel);
	_unkValue9 = channel.opLevel1;
	_unkValue10 = channel.opLevel2;

	// Octave / F-Number / Key-On for channels 6, 7 and 8

	_channels[6].regBx = *dataptr++ & 0x2F;
	writeOPL(0xB6, _channels[6].regBx);
	writeOPL(0xA6, *dataptr++);

	_channels[7].regBx = *dataptr++ & 0x2F;
	writeOPL(0xB7, _channels[7].regBx);
	writeOPL(0xA7, *dataptr++);

	_channels[8].regBx = *dataptr++ & 0x2F;
	writeOPL(0xB8, _channels[8].regBx);
	writeOPL(0xA8, *dataptr++);

	_rhythmSectionBits = 0x20;

	_curRegOffset = regOffsetBackUp;
	_curChannel = channelBackUp;
	return 0;
}

int AdLibDriver::update_playRhythmSection(uint8 *&dataptr, Channel &channel, uint8 value) {
	// Any instrument that we want to play, and which was already playing,
	// is temporarily keyed off. Instruments that were off already, or
	// which we don't want to play, retain their old on/off status. This is
	// probably so that the instrument's envelope is played from its
	// beginning again...

	writeOPL(0xBD, (_rhythmSectionBits & ~(value & 0x1F)) | 0x20);

	// ...but since we only set the rhythm instrument bits, and never clear
	// them (until the entire rhythm section is disabled), I'm not sure how
	// useful the cleverness above is. We could perhaps simply turn off all
	// the rhythm instruments instead.

	_rhythmSectionBits |= value;

	writeOPL(0xBD, _vibratoAndAMDepthBits | 0x20 | _rhythmSectionBits);
	return 0;
}

int AdLibDriver::update_removeRhythmSection(uint8 *&dataptr, Channel &channel, uint8 value) {
	--dataptr;
	_rhythmSectionBits = 0;

	// All the rhythm bits are cleared. The AM and Vibrato depth bits
	// remain unchanged.

	writeOPL(0xBD, _vibratoAndAMDepthBits);
	return 0;
}

int AdLibDriver::updateCallback51(uint8 *&dataptr, Channel &channel, uint8 value) {
	uint8 value2 = *dataptr++;

	if (value & 1) {
		_unkValue12 = value2;

		// Channel 7, op1: Level Key Scaling / Total Level
		writeOPL(0x51, checkValue(value2 + _unkValue7 + _unkValue11 + _unkValue12));
	}

	if (value & 2) {
		_unkValue14 = value2;

		// Channel 8, op2: Level Key Scaling / Total Level
		writeOPL(0x55, checkValue(value2 + _unkValue10 + _unkValue13 + _unkValue14));
	}

	if (value & 4) {
		_unkValue15 = value2;

		// Channel 8, op1: Level Key Scaling / Total Level
		writeOPL(0x52, checkValue(value2 + _unkValue9 + _unkValue16 + _unkValue15));
	}

	if (value & 8) {
		_unkValue18 = value2;

		// Channel 7, op2: Level Key Scaling / Total Level
		writeOPL(0x54, checkValue(value2 + _unkValue8 + _unkValue17 + _unkValue18));
	}

	if (value & 16) {
		_unkValue20 = value2;

		// Channel 6, op2: Level Key Scaling / Total Level
		writeOPL(0x53, checkValue(value2 + _unkValue6 + _unkValue19 + _unkValue20));
	}

	return 0;
}

int AdLibDriver::updateCallback52(uint8 *&dataptr, Channel &channel, uint8 value) {
	uint8 value2 = *dataptr++;

	if (value & 1) {
		_unkValue11 = checkValue(value2 + _unkValue7 + _unkValue11 + _unkValue12);

		// Channel 7, op1: Level Key Scaling / Total Level
		writeOPL(0x51, _unkValue11);
	}

	if (value & 2) {
		_unkValue13 = checkValue(value2 + _unkValue10 + _unkValue13 + _unkValue14);

		// Channel 8, op2: Level Key Scaling / Total Level
		writeOPL(0x55, _unkValue13);
	}

	if (value & 4) {
		_unkValue16 = checkValue(value2 + _unkValue9 + _unkValue16 + _unkValue15);

		// Channel 8, op1: Level Key Scaling / Total Level
		writeOPL(0x52, _unkValue16);
	}

	if (value & 8) {
		_unkValue17 = checkValue(value2 + _unkValue8 + _unkValue17 + _unkValue18);

		// Channel 7, op2: Level Key Scaling / Total Level
		writeOPL(0x54, _unkValue17);
	}

	if (value & 16) {
		_unkValue19 = checkValue(value2 + _unkValue6 + _unkValue19 + _unkValue20);

		// Channel 6, op2: Level Key Scaling / Total Level
		writeOPL(0x53, _unkValue19);
	}

	return 0;
}

int AdLibDriver::updateCallback53(uint8 *&dataptr, Channel &channel, uint8 value) {
	uint8 value2 = *dataptr++;

	if (value & 1) {
		_unkValue11 = value2;

		// Channel 7, op1: Level Key Scaling / Total Level
		writeOPL(0x51, checkValue(value2 + _unkValue7 + _unkValue12));
	}

	if (value & 2) {
		_unkValue13 = value2;

		// Channel 8, op2: Level Key Scaling / Total Level
		writeOPL(0x55, checkValue(value2 + _unkValue10 + _unkValue14));
	}

	if (value & 4) {
		_unkValue16 = value2;

		// Channel 8, op1: Level Key Scaling / Total Level
		writeOPL(0x52, checkValue(value2 + _unkValue9 + _unkValue15));
	}

	if (value & 8) {
		_unkValue17 = value2;

		// Channel 7, op2: Level Key Scaling / Total Level
		writeOPL(0x54, checkValue(value2 + _unkValue8 + _unkValue18));
	}

	if (value & 16) {
		_unkValue19 = value2;

		// Channel 6, op2: Level Key Scaling / Total Level
		writeOPL(0x53, checkValue(value2 + _unkValue6 + _unkValue20));
	}

	return 0;
}

int AdLibDriver::update_setSoundTrigger(uint8 *&dataptr, Channel &channel, uint8 value) {
	_soundTrigger = value;
	return 0;
}

int AdLibDriver::update_setTempoReset(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.tempoReset = value;
	return 0;
}

int AdLibDriver::updateCallback56(uint8 *&dataptr, Channel &channel, uint8 value) {
	channel.unk39 = value;
	channel.unk40 = *dataptr++;
	return 0;
}

// static res

#define COMMAND(x) { &AdLibDriver::x, #x }

void AdLibDriver::setupParserOpcodeTable() {
	static const ParserOpcode parserOpcodeTable[] = {
		// 0
		COMMAND(update_setRepeat),
		COMMAND(update_checkRepeat),
		COMMAND(update_setupProgram),
		COMMAND(update_setNoteSpacing),

		// 4
		COMMAND(update_jump),
		COMMAND(update_jumpToSubroutine),
		COMMAND(update_returnFromSubroutine),
		COMMAND(update_setBaseOctave),

		// 8
		COMMAND(update_stopChannel),
		COMMAND(update_playRest),
		COMMAND(update_writeAdLib),
		COMMAND(update_setupNoteAndDuration),

		// 12
		COMMAND(update_setBaseNote),
		COMMAND(update_setupSecondaryEffect1),
		COMMAND(update_stopOtherChannel),
		COMMAND(update_waitForEndOfProgram),

		// 16
		COMMAND(update_setupInstrument),
		COMMAND(update_setupPrimaryEffect1),
		COMMAND(update_removePrimaryEffect1),
		COMMAND(update_setBaseFreq),

		// 20
		COMMAND(update_stopChannel),
		COMMAND(update_setupPrimaryEffect2),
		COMMAND(update_stopChannel),
		COMMAND(update_stopChannel),

		// 24
		COMMAND(update_stopChannel),
		COMMAND(update_stopChannel),
		COMMAND(update_setPriority),
		COMMAND(update_stopChannel),

		// 28
		COMMAND(updateCallback23),
		COMMAND(updateCallback24),
		COMMAND(update_setExtraLevel1),
		COMMAND(update_stopChannel),

		// 32
		COMMAND(update_setupDuration),
		COMMAND(update_playNote),
		COMMAND(update_stopChannel),
		COMMAND(update_stopChannel),

		// 36
		COMMAND(update_setFractionalNoteSpacing),
		COMMAND(update_stopChannel),
		COMMAND(update_setTempo),
		COMMAND(update_removeSecondaryEffect1),

		// 40
		COMMAND(update_stopChannel),
		COMMAND(update_setChannelTempo),
		COMMAND(update_stopChannel),
		COMMAND(update_setExtraLevel3),

		// 44
		COMMAND(update_setExtraLevel2),
		COMMAND(update_changeExtraLevel2),
		COMMAND(update_setAMDepth),
		COMMAND(update_setVibratoDepth),

		// 48
		COMMAND(update_changeExtraLevel1),
		COMMAND(update_stopChannel),
		COMMAND(update_stopChannel),
		COMMAND(updateCallback38),

		// 52
		COMMAND(update_stopChannel),
		COMMAND(updateCallback39),
		COMMAND(update_removePrimaryEffect2),
		COMMAND(update_stopChannel),

		// 56
		COMMAND(update_stopChannel),
		COMMAND(update_pitchBend),
		COMMAND(update_resetToGlobalTempo),
		COMMAND(update_nop),

		// 60
		COMMAND(update_setDurationRandomness),
		COMMAND(update_changeChannelTempo),
		COMMAND(update_stopChannel),
		COMMAND(updateCallback46),

		// 64
		COMMAND(update_nop),
		COMMAND(update_setupRhythmSection),
		COMMAND(update_playRhythmSection),
		COMMAND(update_removeRhythmSection),

		// 68
		COMMAND(updateCallback51),
		COMMAND(updateCallback52),
		COMMAND(updateCallback53),
		COMMAND(update_setSoundTrigger),

		// 72
		COMMAND(update_setTempoReset),
		COMMAND(updateCallback56),
		COMMAND(update_stopChannel)
	};

	_parserOpcodeTable = parserOpcodeTable;
	_parserOpcodeTableSize = ARRAYSIZE(parserOpcodeTable);
}
#undef COMMAND

// This table holds the register offset for operator 1 for each of the nine
// channels. To get the register offset for operator 2, simply add 3.

const uint8 AdLibDriver::_regOffset[] = {
	0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11,
	0x12
};

//These are the F-Numbers (10 bits) for the notes of the 12-tone scale.
// However, it does not match the table in the AdLib documentation I've seen.

const uint16 AdLibDriver::_freqTable[] = {
	0x0134, 0x0147, 0x015A, 0x016F, 0x0184, 0x019C, 0x01B4, 0x01CE, 0x01E9,
	0x0207, 0x0225, 0x0246
};

// These tables are currently only used by updateCallback46(), which only ever
// uses the first element of one of the sub-tables.

const uint8 *const AdLibDriver::_unkTable2[] = {
	AdLibDriver::_unkTable2_1,
	AdLibDriver::_unkTable2_2,
	AdLibDriver::_unkTable2_1,
	AdLibDriver::_unkTable2_2,
	AdLibDriver::_unkTable2_3,
	AdLibDriver::_unkTable2_2
};

const uint8 AdLibDriver::_unkTable2_1[] = {
	0x50, 0x50, 0x4F, 0x4F, 0x4E, 0x4E, 0x4D, 0x4D,
	0x4C, 0x4C, 0x4B, 0x4B, 0x4A, 0x4A, 0x49, 0x49,
	0x48, 0x48, 0x47, 0x47, 0x46, 0x46, 0x45, 0x45,
	0x44, 0x44, 0x43, 0x43, 0x42, 0x42, 0x41, 0x41,
	0x40, 0x40, 0x3F, 0x3F, 0x3E, 0x3E, 0x3D, 0x3D,
	0x3C, 0x3C, 0x3B, 0x3B, 0x3A, 0x3A, 0x39, 0x39,
	0x38, 0x38, 0x37, 0x37, 0x36, 0x36, 0x35, 0x35,
	0x34, 0x34, 0x33, 0x33, 0x32, 0x32, 0x31, 0x31,
	0x30, 0x30, 0x2F, 0x2F, 0x2E, 0x2E, 0x2D, 0x2D,
	0x2C, 0x2C, 0x2B, 0x2B, 0x2A, 0x2A, 0x29, 0x29,
	0x28, 0x28, 0x27, 0x27, 0x26, 0x26, 0x25, 0x25,
	0x24, 0x24, 0x23, 0x23, 0x22, 0x22, 0x21, 0x21,
	0x20, 0x20, 0x1F, 0x1F, 0x1E, 0x1E, 0x1D, 0x1D,
	0x1C, 0x1C, 0x1B, 0x1B, 0x1A, 0x1A, 0x19, 0x19,
	0x18, 0x18, 0x17, 0x17, 0x16, 0x16, 0x15, 0x15,
	0x14, 0x14, 0x13, 0x13, 0x12, 0x12, 0x11, 0x11,
	0x10, 0x10
};

// no don't ask me WHY this table exsits!
const uint8 AdLibDriver::_unkTable2_2[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x6F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F
};

const uint8 AdLibDriver::_unkTable2_3[] = {
	0x40, 0x40, 0x40, 0x3F, 0x3F, 0x3F, 0x3E, 0x3E,
	0x3E, 0x3D, 0x3D, 0x3D, 0x3C, 0x3C, 0x3C, 0x3B,
	0x3B, 0x3B, 0x3A, 0x3A, 0x3A, 0x39, 0x39, 0x39,
	0x38, 0x38, 0x38, 0x37, 0x37, 0x37, 0x36, 0x36,
	0x36, 0x35, 0x35, 0x35, 0x34, 0x34, 0x34, 0x33,
	0x33, 0x33, 0x32, 0x32, 0x32, 0x31, 0x31, 0x31,
	0x30, 0x30, 0x30, 0x2F, 0x2F, 0x2F, 0x2E, 0x2E,
	0x2E, 0x2D, 0x2D, 0x2D, 0x2C, 0x2C, 0x2C, 0x2B,
	0x2B, 0x2B, 0x2A, 0x2A, 0x2A, 0x29, 0x29, 0x29,
	0x28, 0x28, 0x28, 0x27, 0x27, 0x27, 0x26, 0x26,
	0x26, 0x25, 0x25, 0x25, 0x24, 0x24, 0x24, 0x23,
	0x23, 0x23, 0x22, 0x22, 0x22, 0x21, 0x21, 0x21,
	0x20, 0x20, 0x20, 0x1F, 0x1F, 0x1F, 0x1E, 0x1E,
	0x1E, 0x1D, 0x1D, 0x1D, 0x1C, 0x1C, 0x1C, 0x1B,
	0x1B, 0x1B, 0x1A, 0x1A, 0x1A, 0x19, 0x19, 0x19,
	0x18, 0x18, 0x18, 0x17, 0x17, 0x17, 0x16, 0x16,
	0x16, 0x15
};

// This table is used to modify the frequency of the notes, depending on the
// note value and the pitch bend value. In theory, we could very well try to
// access memory outside this table, but in reality that probably won't happen.
//

const uint8 AdLibDriver::_pitchBendTables[][32] = {
	// 0
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08,
	  0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
	  0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x19,
	  0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21 },
	// 1
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x09,
	  0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
	  0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x1A,
	  0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x22, 0x24 },
	// 2
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x09,
	  0x0A, 0x0C, 0x0D, 0x0E, 0x0F, 0x11, 0x12, 0x13,
	  0x14, 0x15, 0x16, 0x17, 0x19, 0x1A, 0x1C, 0x1D,
	  0x1E, 0x1F, 0x20, 0x21, 0x22, 0x24, 0x25, 0x26 },
	// 3
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x0A,
	  0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x11, 0x12, 0x13,
	  0x14, 0x15, 0x16, 0x17, 0x18, 0x1A, 0x1C, 0x1D,
	  0x1E, 0x1F, 0x20, 0x21, 0x23, 0x25, 0x27, 0x28 },
	// 4
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x0A,
	  0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x11, 0x13, 0x15,
	  0x16, 0x17, 0x18, 0x19, 0x1B, 0x1D, 0x1F, 0x20,
	  0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x28, 0x2A },
	// 5
	{ 0x00, 0x01, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0B,
	  0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x13, 0x15,
	  0x16, 0x17, 0x18, 0x19, 0x1B, 0x1D, 0x1F, 0x20,
	  0x21, 0x22, 0x23, 0x25, 0x27, 0x29, 0x2B, 0x2D },
	// 6
	{ 0x00, 0x01, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0B,
	  0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x13, 0x15,
	  0x16, 0x17, 0x18, 0x1A, 0x1C, 0x1E, 0x21, 0x24,
	  0x25, 0x26, 0x27, 0x29, 0x2B, 0x2D, 0x2F, 0x30 },
	// 7
	{ 0x00, 0x01, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C,
	  0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x13, 0x15, 0x18,
	  0x19, 0x1A, 0x1C, 0x1D, 0x1F, 0x21, 0x23, 0x25,
	  0x26, 0x27, 0x29, 0x2B, 0x2D, 0x2F, 0x30, 0x32 },
	// 8
	{ 0x00, 0x01, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0D,
	  0x0E, 0x0F, 0x10, 0x11, 0x12, 0x14, 0x17, 0x1A,
	  0x19, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x25, 0x28,
	  0x29, 0x2A, 0x2B, 0x2D, 0x2F, 0x31, 0x33, 0x35 },
	// 9
	{ 0x00, 0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0E,
	  0x0F, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1B,
	  0x1C, 0x1D, 0x1E, 0x20, 0x22, 0x24, 0x26, 0x29,
	  0x2A, 0x2C, 0x2E, 0x30, 0x32, 0x34, 0x36, 0x39 },
	// 10
	{ 0x00, 0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0E,
	  0x0F, 0x10, 0x12, 0x14, 0x16, 0x19, 0x1B, 0x1E,
	  0x1F, 0x21, 0x23, 0x25, 0x27, 0x29, 0x2B, 0x2D,
	  0x2E, 0x2F, 0x31, 0x32, 0x34, 0x36, 0x39, 0x3C },
	// 11
	{ 0x00, 0x01, 0x03, 0x05, 0x07, 0x0A, 0x0C, 0x0F,
	  0x10, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1E,
	  0x1F, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2B, 0x2E,
	  0x2F, 0x30, 0x32, 0x34, 0x36, 0x39, 0x3C, 0x3F },
	// 12
	{ 0x00, 0x02, 0x04, 0x06, 0x08, 0x0B, 0x0D, 0x10,
	  0x11, 0x12, 0x14, 0x16, 0x18, 0x1B, 0x1E, 0x21,
	  0x22, 0x23, 0x25, 0x27, 0x29, 0x2C, 0x2F, 0x32,
	  0x33, 0x34, 0x36, 0x38, 0x3B, 0x34, 0x41, 0x44 },
	// 13
	{ 0x00, 0x02, 0x04, 0x06, 0x08, 0x0B, 0x0D, 0x11,
	  0x12, 0x13, 0x15, 0x17, 0x1A, 0x1D, 0x20, 0x23,
	  0x24, 0x25, 0x27, 0x29, 0x2C, 0x2F, 0x32, 0x35,
	  0x36, 0x37, 0x39, 0x3B, 0x3E, 0x41, 0x44, 0x47 }
};

#pragma mark -

// Kyra 1 sound triggers. Most noticeably, these are used towards the end of
// the game, in the castle, to cycle between different songs. The same music is
// used in other places throughout the game, but the player is less likely to
// spend enough time there to notice.

const int SoundAdLibPC::_kyra1SoundTriggers[] = {
	0, 4, 5, 3
};

const int SoundAdLibPC::_kyra1NumSoundTriggers = ARRAYSIZE(SoundAdLibPC::_kyra1SoundTriggers);

SoundAdLibPC::SoundAdLibPC(KyraEngine_v1 *vm, Audio::Mixer *mixer)
	: Sound(vm, mixer), _driver(0), _trackEntries(), _soundDataPtr(0) {
	memset(_trackEntries, 0, sizeof(_trackEntries));

	_soundTriggers = 0;
	_numSoundTriggers = 0;
	_sfxPlayingSound = -1;
	_soundFileLoaded.clear();
	_currentResourceSet = 0;
	memset(&_resInfo, 0, sizeof(_resInfo));

	switch (vm->game()) {
	case GI_LOL:
		_version = _vm->gameFlags().isDemo ? 3 : 4;
		break;
	case GI_KYRA2:
		_version = 4;
		break;
	case GI_KYRA1:
		_version = 3;
		_soundTriggers = _kyra1SoundTriggers;
		_numSoundTriggers = _kyra1NumSoundTriggers;
		break;
	case GI_EOB2:
		_version = 2;
		break;
	case GI_EOB1:
		_version = 1;
		break;
	default:
		break;
	}

	_driver = new AdLibDriver(mixer, _version);
	assert(_driver);
}

SoundAdLibPC::~SoundAdLibPC() {
	delete _driver;
	delete[] _soundDataPtr;
	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

bool SoundAdLibPC::init() {
	_driver->initDriver();
	return true;
}

void SoundAdLibPC::process() {
	int trigger = _driver->getSoundTrigger();

	if (trigger < _numSoundTriggers) {
		int soundId = _soundTriggers[trigger];

		if (soundId)
			playTrack(soundId);
	} else {
		warning("Unknown sound trigger %d", trigger);
		// TODO: At this point, we really want to clear the trigger...
	}
}

void SoundAdLibPC::updateVolumeSettings() {
	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	int newMusicVolume = mute ? 0 : ConfMan.getInt("music_volume");
	//newMusicVolume = (newMusicVolume * 145) / Audio::Mixer::kMaxMixerVolume + 110;
	newMusicVolume = CLIP(newMusicVolume, 0, 255);

	int newSfxVolume = mute ? 0 : ConfMan.getInt("sfx_volume");
	//newSfxVolume = (newSfxVolume * 200) / Audio::Mixer::kMaxMixerVolume + 55;
	newSfxVolume = CLIP(newSfxVolume, 0, 255);

	_driver->setMusicVolume(newMusicVolume);
	_driver->setSfxVolume(newSfxVolume);
}

void SoundAdLibPC::playTrack(uint8 track) {
	if (_musicEnabled) {
		// WORKAROUND: There is a bug in the Kyra 1 "Pool of Sorrow"
		// music which causes the channels to get progressively out of
		// sync for each loop. To avoid that, we declare that all four
		// of the song channels have to jump "in sync".

		if (track == 4 && _soundFileLoaded.equalsIgnoreCase("KYRA1B.ADL"))
			_driver->setSyncJumpMask(0x000F);
		else
			_driver->setSyncJumpMask(0);
		play(track, 0xff);
	}
}

void SoundAdLibPC::haltTrack() {
	play(0, 0);
	play(0, 0);
	//_vm->_system->delayMillis(3 * 60);
}

bool SoundAdLibPC::isPlaying() const {
	return _driver->isChannelPlaying(0);
}

void SoundAdLibPC::playSoundEffect(uint8 track, uint8 volume) {
	if (_sfxEnabled)
		play(track, volume);
}

void SoundAdLibPC::play(uint8 track, uint8 volume) {
	uint16 soundId = 0;

	if (_version == 4)
		soundId = READ_LE_UINT16(&_trackEntries[track<<1]);
	else
		soundId = _trackEntries[track];

	if ((soundId == 0xFFFF && _version == 4) || (soundId == 0xFF && _version < 4) || !_soundDataPtr)
		return;

	_driver->queueTrack(soundId, volume);
}

void SoundAdLibPC::beginFadeOut() {
	play(_version > 2 ? 1 : 15, 0xff);
}

int SoundAdLibPC::checkTrigger() {
	return _driver->getSoundTrigger();
}

void SoundAdLibPC::resetTrigger() {
	_driver->resetSoundTrigger();
}

void SoundAdLibPC::initAudioResourceInfo(int set, void *info) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		delete _resInfo[set];
		_resInfo[set] = info ? new SoundResourceInfo_PC(*(SoundResourceInfo_PC*)info) : 0;
	}
}

void SoundAdLibPC::selectAudioResourceSet(int set) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		if (_resInfo[set])
			_currentResourceSet = set;
	}
}

bool SoundAdLibPC::hasSoundFile(uint file) const {
	if (file < res()->fileListSize)
		return (res()->fileList[file] != 0);
	return false;
}

void SoundAdLibPC::loadSoundFile(uint file) {
	if (file < res()->fileListSize)
		internalLoadFile(res()->fileList[file]);
}

void SoundAdLibPC::loadSoundFile(Common::String file) {
	internalLoadFile(file);
}

void SoundAdLibPC::internalLoadFile(Common::String file) {
	file += ((_version == 1) ? ".DAT" : ".ADL");
	if (_soundFileLoaded == file)
		return;

	if (_soundDataPtr)
		haltTrack();

	uint8 *fileData = 0; uint32 fileSize = 0;

	fileData = _vm->resource()->fileData(file.c_str(), &fileSize);
	if (!fileData) {
		warning("Couldn't find music file: '%s'", file.c_str());
		return;
	}

	playSoundEffect(0);
	playSoundEffect(0);

	_driver->stopAllChannels();
	_soundDataPtr = 0;

	int soundDataSize = fileSize;
	uint8 *p = fileData;

	if (_version == 4) {
		memcpy(_trackEntries, p, 500);
		p += 500;
		soundDataSize -= 500;
	} else {
		memcpy(_trackEntries, p, 120);
		p += 120;
		soundDataSize -= 120;
	}

	_soundDataPtr = new uint8[soundDataSize];
	assert(_soundDataPtr);

	memcpy(_soundDataPtr, p, soundDataSize*sizeof(uint8));

	delete[] fileData;
	fileData = p = 0;
	fileSize = 0;

	_driver->setSoundData(_soundDataPtr);

	_soundFileLoaded = file;
}

} // End of namespace Kyra
