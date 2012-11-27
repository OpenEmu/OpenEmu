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

// Only compiled if Kyra is built-in or we're building for dynamic modules
#if !defined(AUDIO_MODS_MAXTRAX_H) && (defined(ENABLE_KYRA) || defined(DYNAMIC_MODULES))
#define AUDIO_MODS_MAXTRAX_H

// #define MAXTRAX_HAS_MODULATION
// #define MAXTRAX_HAS_MICROTONAL

#include "audio/mods/paula.h"

namespace Audio {

class MaxTrax : public Paula {
public:
	MaxTrax(int rate, bool stereo, uint16 vBlankFreq = 50, uint16 maxScores = 128);
	virtual ~MaxTrax();

	bool load(Common::SeekableReadStream &musicData, bool loadScores = true, bool loadSamples = true);
	bool playSong(int songIndex, bool loop = false);
	void advanceSong(int advance = 1);
	int playNote(byte note, byte patch, uint16 duration, uint16 volume, bool rightSide);
	void setVolume(const byte volume) { Common::StackLock lock(_mutex); _playerCtx.volume = volume; }
	void setTempo(const uint16 tempo);
	void stopMusic();
	/**
	 * Set a callback function for sync-events.
	 * @param callback Callback function, will be called synchronously, so DONT modify the player
	 *		directly in response
	 */
	void setSignalCallback(void (*callback) (int));

protected:
	void interrupt();

private:
	enum { kNumPatches = 64, kNumVoices = 4, kNumChannels = 16, kNumExtraChannels = 1 };
	enum { kPriorityScore, kPriorityNote, kPrioritySound };

#ifdef MAXTRAX_HAS_MICROTONAL
	int16	_microtonal[128];
#endif

	struct Event {
		uint16	startTime;
		uint16	stopTime;
		byte	command;
		byte	parameter;
	};

	const struct Score {
		const Event	*events;
		uint32	numEvents;
	} *_scores;

	int _numScores;

	struct {
		uint32	sineValue;
		uint16	vBlankFreq;
		int32	ticks;
		int32	tickUnit;
		uint16	frameUnit;

		uint16	maxScoreNum;
		uint16	tempo;
		uint16	tempoInitial;
		uint16	tempoStart;
		int16	tempoDelta;
		int32	tempoTime;
		int32	tempoTicks;

		byte	volume;

		bool	filterOn;
		bool	handleVolume;
		bool	musicLoop;

		int		scoreIndex;
		const Event	*nextEvent;
		int32	nextEventTime;

		void (*syncCallBack) (int);
		const Event	*repeatPoint[4];
		byte	repeatCount[4];
	} _playerCtx;

	struct Envelope {
		uint16	duration;
		uint16	volume;
	};

	struct Patch {
		const Envelope *attackPtr;
		//Envelope *releasePtr;
		uint16	attackLen;
		uint16	releaseLen;

		int16	tune;
		uint16	volume;

		// this was the SampleData struct in the assembler source
		const int8	*samplePtr;
		uint32	sampleTotalLen;
		uint32	sampleAttackLen;
		uint16	sampleOctaves;
	} _patch[kNumPatches];

	struct ChannelContext {
		const Patch	*patch;
		uint16	regParamNumber;

		uint16	modulation;
		uint16	modulationTime;

		int16	microtonal;

		uint16	portamentoTime;

		int16	pitchBend;
		int16	pitchReal;
		int8	pitchBendRange;

		uint8	volume;
//		uint8	voicesActive;

		enum {
			kFlagRightChannel = 1 << 0,
			kFlagPortamento = 1 << 1,
			kFlagDamper = 1 << 2,
			kFlagMono = 1 << 3,
			kFlagMicrotonal = 1 << 4,
			kFlagModVolume = 1 << 5
		};
		byte	flags;
		bool	isAltered;

		uint8	lastNote;
//		uint8	program;

	} _channelCtx[kNumChannels + kNumExtraChannels];

	struct VoiceContext {
		ChannelContext *channel;
		const Patch	*patch;
		const Envelope *envelope;
//		uint32	uinqueId;
		int32	preCalcNote;
		uint32	ticksLeft;
		int32	portaTicks;
		int32	incrVolume;
//		int32	periodOffset;
		uint16	envelopeLeft;
		uint16	noteVolume;
		uint16	baseVolume;
		uint16	lastPeriod;
		byte	baseNote;
		byte	endNote;
		byte	octave;
//		byte	number;
//		byte	link;
		enum {
			kStatusFree,
			kStatusHalt,
			kStatusDecay,
			kStatusRelease,
			kStatusSustain,
			kStatusAttack,
			kStatusStart
		};
		uint8	isBlocked;
		uint8	priority;
		byte	status;
		byte	lastVolume;
		byte	tieBreak;
		bool	hasDamper;
		bool	hasPortamento;
		byte	dmaOff;

		int32	stopEventTime;
	} _voiceCtx[kNumVoices];

	void controlCh(ChannelContext &channel, byte command, byte data);
	void freePatches();
	void freeScores();
	void resetChannel(ChannelContext &chan, bool rightChannel);
	void resetPlayer();

	int8 pickvoice(uint pick, int16 pri);
	uint16 calcNote(const VoiceContext &voice);
	int8 noteOn(ChannelContext &channel, byte note, uint16 volume, uint16 pri);
	void killVoice(byte num);

	static void outPutEvent(const Event &ev, int num = -1);
	static void outPutScore(const Score &sc, int num = -1);
};
}	// End of namespace Audio

#endif // !defined(AUDIO_MODS_MAXTRAX_H)
