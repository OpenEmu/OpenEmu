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

// Only compiled if SCUMM is built-in or we're building for dynamic modules
#if !defined(AUDIO_MODS_TFMX_H) && (defined(ENABLE_SCUMM) || defined(DYNAMIC_MODULES))
#define AUDIO_MODS_TFMX_H

#include "audio/mods/paula.h"

namespace Audio {

class Tfmx : public Paula {
public:
	Tfmx(int rate, bool stereo);
	virtual ~Tfmx();

	/**
	 * Stops a playing Song (but leaves macros running) and optionally also stops the player
	 *
	 * @param stopAudio	stops player and audio output
	 * @param dataSize	number of bytes to be written
	 * @return the number of bytes which were actually written.
	 */
	void stopSong(bool stopAudio = true) { Common::StackLock lock(_mutex); stopSongImpl(stopAudio); }
	/**
	 * Stops currently playing Song (if any) and cues up a new one.
	 * if stopAudio is specified, the player gets reset before starting the new song
	 *
	 * @param songPos	index of Song to play
	 * @param stopAudio	stops player and audio output
	 * @param dataSize	number of bytes to be written
	 * @return the number of bytes which were actually written.
	 */
	void doSong(int songPos, bool stopAudio = false);
	/**
	 * plays an effect from the sfx-table, does not start audio-playback.
	 *
	 * @param sfxIndex	index of effect to play
	 * @param unlockChannel	overwrite higher priority effects
	 * @return	index of the channel which now queued up the effect.
	 *			-1 in case the effect couldnt be queued up
	 */
	int doSfx(uint16 sfxIndex, bool unlockChannel = false);
	/**
	 * stop a running macro channel
	 *
	 * @param channel	index of effect to stop
	 */
	void stopMacroEffect(int channel);

	void doMacro(int note, int macro, int relVol = 0, int finetune = 0, int channelNo = 0);
	int getTicks() const { return _playerCtx.tickCount; }
	int getSongIndex() const { return _playerCtx.song; }
	void setSignalPtr(uint16 *ptr, uint16 numSignals) { _playerCtx.signal = ptr; _playerCtx.numSignals = numSignals; }
	void freeResources() { _deleteResource = true; freeResourceDataImpl(); }
	bool load(Common::SeekableReadStream &musicData, Common::SeekableReadStream &sampleData, bool autoDelete = true);
	void setModuleData(Tfmx &otherPlayer);

protected:
	void interrupt();

private:
	enum { kPalDefaultCiaVal = 11822, kNtscDefaultCiaVal = 14320, kCiaBaseInterval = 0x1B51F8 };
	enum { kNumVoices = 4, kNumChannels = 8, kNumSubsongs = 32, kMaxPatternOffsets = 128, kMaxMacroOffsets = 128 };

	struct MdatResource {
		const byte *mdatAlloc;	///< allocated Block of Memory
		const byte *mdatData;  	///< Start of mdat-File, might point before mdatAlloc to correct Offset
		uint32 mdatLen;

		uint16 headerFlags;
//		uint32 headerUnknown;
//		char textField[6 * 40];

		struct Subsong {
			uint16 songstart;	///< Index in Trackstep-Table
			uint16 songend;		///< Last index in Trackstep-Table
			uint16 tempo;
		} subsong[kNumSubsongs];

		uint32 trackstepOffset;	///< Offset in mdat
		uint32 sfxTableOffset;

		uint32 patternOffset[kMaxPatternOffsets];	///< Offset in mdat
		uint32 macroOffset[kMaxMacroOffsets];	///< Offset in mdat

		void boundaryCheck(const void *address, size_t accessLen = 1) const {
			assert(mdatAlloc <= address && (const byte *)address + accessLen <= (const byte *)mdatData + mdatLen);
		}
	} const *_resource;

	struct SampleResource {
		const int8 *sampleData;	///< The whole sample-File
		uint32 sampleLen;

		void boundaryCheck(const void *address, size_t accessLen = 2) const {
			assert(sampleData <= address && (const byte *)address + accessLen <= (const byte *)sampleData + sampleLen);
		}
	} _resourceSample;

	bool _deleteResource;

	bool hasResources() {
		return _resource && _resource->mdatLen && _resourceSample.sampleLen;
	}

	struct ChannelContext {
		byte	paulaChannel;

//		byte	macroIndex;
		uint16	macroWait;
		uint32	macroOffset;
		uint32	macroReturnOffset;
		uint16	macroStep;
		uint16	macroReturnStep;
		uint8	macroLoopCount;
		bool	macroRun;
		int8	macroSfxRun;	///< values are the folowing: -1 macro disabled, 0 macro init, 1 macro running

		uint32	customMacro;
		uint8	customMacroIndex;
		uint8	customMacroPrio;

		bool	sfxLocked;
		int16	sfxLockTime;
		bool	keyUp;

		bool	deferWait;
		uint16	dmaIntCount;

		uint32	sampleStart;
		uint16	sampleLen;
		uint16	refPeriod;
		uint16	period;

		int8	volume;
		uint8	relVol;
		uint8	note;
		uint8	prevNote;
		int16	fineTune; // always a signextended byte

		uint8	portaSkip;
		uint8	portaCount;
		uint16	portaDelta;
		uint16	portaValue;

		uint8	envSkip;
		uint8	envCount;
		uint8	envDelta;
		int8	envEndVolume;

		uint8	vibLength;
		uint8	vibCount;
		int16	vibValue;
		int8	vibDelta;

		uint8	addBeginLength;
		uint8	addBeginCount;
		int32	addBeginDelta;
	} _channelCtx[kNumVoices];

	struct PatternContext {
		uint32	offset; // patternStart, Offset from mdat
		uint32	savedOffset;	// for subroutine calls
		uint16	step;	// distance from patternStart
		uint16	savedStep;

		uint8	command;
		int8	expose;
		uint8	loopCount;
		uint8	wait;	///< how many ticks to wait before next Command
	} _patternCtx[kNumChannels];

	struct TrackStepContext {
		uint16	startInd;
		uint16	stopInd;
		uint16	posInd;
		int16	loopCount;
	} _trackCtx;

	struct PlayerContext {
		int8	song;	///< >= 0 if Song is running (means process Patterns)

		uint16	patternCount;
		uint16	patternSkip;	///< skip that amount of CIA-Interrupts

		int8	volume;	///< Master Volume

		uint8	fadeSkip;
		uint8	fadeCount;
		int8	fadeEndVolume;
		int8	fadeDelta;

		int		tickCount;

		uint16	*signal;
		uint16	numSignals;

		bool	stopWithLastPattern; ///< hack to automatically stop the whole player if no Pattern is running
	} _playerCtx;

	const byte *getSfxPtr(uint16 index = 0) const {
		const byte *sfxPtr = (const byte *)(_resource->mdatData + _resource->sfxTableOffset + index * 8);

		_resource->boundaryCheck(sfxPtr, 8);
		return sfxPtr;
	}

	const uint16 *getTrackPtr(uint16 trackstep = 0) const {
		const uint16 *trackData = (const uint16 *)(_resource->mdatData + _resource->trackstepOffset + 16 * trackstep);

		_resource->boundaryCheck(trackData, 16);
		return trackData;
	}

	const uint32 *getPatternPtr(uint32 offset) const {
		const uint32 *pattData = (const uint32 *)(_resource->mdatData + offset);

		_resource->boundaryCheck(pattData, 4);
		return pattData;
	}

	const uint32 *getMacroPtr(uint32 offset) const {
		const uint32 *macroData = (const uint32 *)(_resource->mdatData + offset);

		_resource->boundaryCheck(macroData, 4);
		return macroData;
	}

	const int8 *getSamplePtr(const uint32 offset) const {
		const int8 *sample = _resourceSample.sampleData + offset;

		_resourceSample.boundaryCheck(sample, 2);
		return sample;
	}

	static inline void initMacroProgramm(ChannelContext &channel);
	static inline void clearEffects(ChannelContext &channel);
	static inline void haltMacroProgramm(ChannelContext &channel);
	static inline void unlockMacroChannel(ChannelContext &channel);
	static inline void initPattern(PatternContext &pattern, uint8 cmd, int8 expose, uint32 offset);
	void stopSongImpl(bool stopAudio = true);
	static inline void setNoteMacro(ChannelContext &channel, uint note, int fineTune);
	void initFadeCommand(const uint8 fadeTempo, const int8 endVol);
	void setModuleData(const MdatResource *resource, const int8 *sampleData, uint32 sampleLen, bool autoDelete = true);
	static const MdatResource *loadMdatFile(Common::SeekableReadStream &musicData);
	static const int8 *loadSampleFile(uint32 &sampleLen, Common::SeekableReadStream &sampleStream);
	void freeResourceDataImpl();
	void effects(ChannelContext &channel);
	void macroRun(ChannelContext &channel);
	void advancePatterns();
	bool patternRun(PatternContext &pattern);
	bool trackRun(bool incStep = false);
	void noteCommand(uint8 note, uint8 param1, uint8 param2, uint8 param3);
};

}	// End of namespace Audio

#endif // !defined(AUDIO_MODS_TFMX_H)
