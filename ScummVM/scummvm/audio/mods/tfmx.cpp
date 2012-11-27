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

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/debug.h"
#include "common/textconsole.h"

#include "audio/mods/tfmx.h"

// test for engines using this class.
#if defined(AUDIO_MODS_TFMX_H)

// couple debug-functions
namespace {

#if 0
void displayPatternstep(const void * const vptr);
void displayMacroStep(const void * const vptr);
#endif

static const uint16 noteIntervalls[64] = {
	1710, 1614, 1524, 1438, 1357, 1281, 1209, 1141, 1077, 1017,  960,  908,
	 856,  810,  764,  720,  680,  642,  606,  571,  539,  509,  480,  454,
	 428,  404,  381,  360,  340,  320,  303,  286,  270,  254,  240,  227,
	 214,  202,  191,  180,  170,  160,  151,  143,  135,  127,  120,  113,
	 214,  202,  191,  180,  170,  160,  151,  143,  135,  127,  120,  113,
	 214,  202,  191,  180
};

} // End of anonymous namespace

namespace Audio {

Tfmx::Tfmx(int rate, bool stereo)
	: Paula(stereo, rate),
	  _resource(),
	  _resourceSample(),
	  _playerCtx(),
	  _deleteResource(false) {

	_playerCtx.stopWithLastPattern = false;

	for (int i = 0; i < kNumVoices; ++i)
		_channelCtx[i].paulaChannel = (byte)i;

	_playerCtx.volume = 0x40;
	_playerCtx.patternSkip = 6;
	stopSongImpl();

	setTimerBaseValue(kPalCiaClock);
	setInterruptFreqUnscaled(kPalDefaultCiaVal);
}

Tfmx::~Tfmx() {
	freeResourceDataImpl();
}

void Tfmx::interrupt() {
	assert(!_end);
	++_playerCtx.tickCount;

	for (int i = 0; i < kNumVoices; ++i) {
		if (_channelCtx[i].dmaIntCount) {
			// wait for DMA Interupts to happen
			int doneDma = getChannelDmaCount(i);
			if (doneDma >= _channelCtx[i].dmaIntCount) {
				_channelCtx[i].dmaIntCount = 0;
				_channelCtx[i].macroRun = true;
			}
		}
	}

	for (int i = 0; i < kNumVoices; ++i) {
		ChannelContext &channel = _channelCtx[i];

		if (channel.sfxLockTime >= 0)
			--channel.sfxLockTime;
		else {
			channel.sfxLocked = false;
			channel.customMacroPrio = 0;
		}

		// externally queued macros
		if (channel.customMacro) {
			const byte * const noteCmd = (const byte *)&channel.customMacro;
			channel.sfxLocked = false;
			noteCommand(noteCmd[0], noteCmd[1], (noteCmd[2] & 0xF0) | (uint8)i, noteCmd[3]);
			channel.customMacro = 0;
			channel.sfxLocked = (channel.customMacroPrio != 0);
		}

		// apply timebased effects on Parameters
		if (channel.macroSfxRun > 0)
			effects(channel);

		// see if we have to run the macro-program
		if (channel.macroRun) {
			if (!channel.macroWait)
				macroRun(channel);
			else
				--channel.macroWait;
		}

		Paula::setChannelPeriod(i, channel.period);
		if (channel.macroSfxRun >= 0)
			channel.macroSfxRun = 1;

		// TODO: handling pending DMAOff?
	}

	// Patterns are only processed each _playerCtx.timerCount + 1 tick
	if (_playerCtx.song >= 0 && !_playerCtx.patternCount--) {
		_playerCtx.patternCount = _playerCtx.patternSkip;
		advancePatterns();
	}
}

void Tfmx::effects(ChannelContext &channel) {
	// addBegin
	if (channel.addBeginLength) {
		channel.sampleStart += channel.addBeginDelta;
		Paula::setChannelSampleStart(channel.paulaChannel, getSamplePtr(channel.sampleStart));
		if (!(--channel.addBeginCount)) {
			channel.addBeginCount = channel.addBeginLength;
			channel.addBeginDelta = -channel.addBeginDelta;
		}
	}

	// vibrato
	if (channel.vibLength) {
		channel.vibValue += channel.vibDelta;
		if (--channel.vibCount == 0) {
			channel.vibCount = channel.vibLength;
			channel.vibDelta = -channel.vibDelta;
		}
		if (!channel.portaDelta) {
			// 16x16 bit multiplication, casts needed for the right results
			channel.period = (uint16)(((uint32)channel.refPeriod * (uint16)((1 << 11) + channel.vibValue)) >> 11);
		}
	}

	// portamento
	if (channel.portaDelta && !(--channel.portaCount)) {
		channel.portaCount = channel.portaSkip;

		bool resetPorta = true;
		const uint16 period = channel.refPeriod;
		uint16 portaVal = channel.portaValue;

		if (period > portaVal) {
			portaVal = ((uint32)portaVal * (uint16)((1 << 8) + channel.portaDelta)) >> 8;
			resetPorta = (period <= portaVal);

		} else if (period < portaVal) {
			portaVal = ((uint32)portaVal * (uint16)((1 << 8) - channel.portaDelta)) >> 8;
			resetPorta = (period >= portaVal);
		}

		if (resetPorta) {
			channel.portaDelta = 0;
			channel.portaValue = period & 0x7FF;
		} else
			channel.period = channel.portaValue = portaVal & 0x7FF;
	}

	// envelope
	if (channel.envSkip && !channel.envCount--) {
		channel.envCount = channel.envSkip;

		const int8 endVol = channel.envEndVolume;
		int8 volume = channel.volume;
		bool resetEnv = true;

		if (endVol > volume) {
			volume += channel.envDelta;
			resetEnv = endVol <= volume;
		} else {
			volume -= channel.envDelta;
			resetEnv = volume <= 0 || endVol >= volume;
		}

		if (resetEnv) {
			channel.envSkip = 0;
			volume = endVol;
		}
		channel.volume = volume;
	}

	// Fade
	if (_playerCtx.fadeDelta && !(--_playerCtx.fadeCount)) {
		_playerCtx.fadeCount = _playerCtx.fadeSkip;

		_playerCtx.volume += _playerCtx.fadeDelta;
		if (_playerCtx.volume == _playerCtx.fadeEndVolume)
			_playerCtx.fadeDelta = 0;
	}

	// Volume
	const uint8 finVol = _playerCtx.volume * channel.volume >> 6;
	Paula::setChannelVolume(channel.paulaChannel, finVol);
}

void Tfmx::macroRun(ChannelContext &channel) {
	bool deferWait = channel.deferWait;
	for (;;) {
		const byte *const macroPtr = (const byte *)(getMacroPtr(channel.macroOffset) + channel.macroStep);
		++channel.macroStep;

		switch (macroPtr[0]) {
		case 0x00:	// Reset + DMA Off. Parameters: deferWait, addset, vol
			clearEffects(channel);
			// FT
		case 0x13:	// DMA Off. Parameters:  deferWait, addset, vol
			// TODO: implement PArameters
			Paula::disableChannel(channel.paulaChannel);
			channel.deferWait = deferWait = (macroPtr[1] != 0);
			if (deferWait) {
				// if set, then we expect a DMA On in the same tick.
				channel.period = 4;
				//Paula::setChannelPeriod(channel.paulaChannel, channel.period);
				Paula::setChannelSampleLen(channel.paulaChannel, 1);
				// in this state we then need to allow some commands that normally
				// would halt the macroprogamm to continue instead.
				// those commands are: Wait, WaitDMA, AddPrevNote, AddNote, SetNote, <unknown Cmd>
				// DMA On is affected aswell
				// TODO remember time disabled, remember pending dmaoff?.
			}

			if (macroPtr[2] || macroPtr[3]) {
				channel.volume = (macroPtr[2] ? 0 : channel.relVol * 3) + macroPtr[3];
				Paula::setChannelVolume(channel.paulaChannel, channel.volume);
			}
			continue;

		case 0x01:	// DMA On
			// TODO: Parameter macroPtr[1] - en-/disable effects
			channel.dmaIntCount = 0;
			if (deferWait) {
				// TODO
				// there is actually a small delay in the player, but I think that
				// only allows to clear DMA-State on real Hardware
			}
			Paula::setChannelPeriod(channel.paulaChannel, channel.period);
			Paula::enableChannel(channel.paulaChannel);
			channel.deferWait = deferWait = false;
			continue;

		case 0x02:	// Set Beginn. Parameters: SampleOffset(L)
			channel.addBeginLength = 0;
			channel.sampleStart = READ_BE_UINT32(macroPtr) & 0xFFFFFF;
			Paula::setChannelSampleStart(channel.paulaChannel, getSamplePtr(channel.sampleStart));
			continue;

		case 0x03:	// SetLength. Parameters: SampleLength(W)
			channel.sampleLen = READ_BE_UINT16(&macroPtr[2]);
			Paula::setChannelSampleLen(channel.paulaChannel, channel.sampleLen);
			continue;

		case 0x04:	// Wait. Parameters: Ticks to wait(W).
			// TODO: some unknown Parameter? (macroPtr[1] & 1)
			channel.macroWait = READ_BE_UINT16(&macroPtr[2]);
			break;

		case 0x10:	// Loop Key Up. Parameters: Loopcount, MacroStep(W)
			if (channel.keyUp)
				continue;
			// FT
		case 0x05:	// Loop. Parameters: Loopcount, MacroStep(W)
			if (channel.macroLoopCount != 0) {
				if (channel.macroLoopCount == 0xFF)
					channel.macroLoopCount = macroPtr[1];
				channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
			}
			--channel.macroLoopCount;
			continue;

		case 0x06:	// Jump. Parameters: MacroIndex, MacroStep(W)
			// channel.macroIndex = macroPtr[1] & (kMaxMacroOffsets - 1);
			channel.macroOffset = _resource->macroOffset[macroPtr[1] & (kMaxMacroOffsets - 1)];
			channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
			channel.macroLoopCount = 0xFF;
			continue;

		case 0x07:	// Stop Macro
			channel.macroRun = false;
			--channel.macroStep;
			return;

		case 0x08:	// AddNote. Parameters: Note, Finetune(W)
			setNoteMacro(channel, channel.note + macroPtr[1], READ_BE_UINT16(&macroPtr[2]));
			break;

		case 0x09:	// SetNote. Parameters: Note, Finetune(W)
			setNoteMacro(channel, macroPtr[1], READ_BE_UINT16(&macroPtr[2]));
			break;

		case 0x0A:	// Clear Effects
			clearEffects(channel);
			continue;

		case 0x0B:	// Portamento. Parameters: count, speed
			channel.portaSkip = macroPtr[1];
			channel.portaCount = 1;
			// if porta is already running, then keep using old value
			if (!channel.portaDelta)
				channel.portaValue = channel.refPeriod;
			channel.portaDelta = READ_BE_UINT16(&macroPtr[2]);
			continue;

		case 0x0C:	// Vibrato. Parameters: Speed, intensity
			channel.vibLength = macroPtr[1];
			channel.vibCount = macroPtr[1] / 2;
			channel.vibDelta = macroPtr[3];
			// TODO: Perhaps a bug, vibValue could be left uninitialized
			if (!channel.portaDelta) {
				channel.period = channel.refPeriod;
				channel.vibValue = 0;
			}
			continue;

		case 0x0D:	// Add Volume. Parameters: note, addNoteFlag, volume
			if (macroPtr[2] == 0xFE)
				setNoteMacro(channel, channel.note + macroPtr[1], 0);
			channel.volume = channel.relVol * 3 + macroPtr[3];
			continue;

		case 0x0E:	// Set Volume. Parameters: note, addNoteFlag, volume
			if (macroPtr[2] == 0xFE)
				setNoteMacro(channel, channel.note + macroPtr[1], 0);
			channel.volume = macroPtr[3];
			continue;

		case 0x0F:	// Envelope. Parameters: speed, count, endvol
			channel.envDelta = macroPtr[1];
			channel.envCount = channel.envSkip = macroPtr[2];
			channel.envEndVolume = macroPtr[3];
			continue;

		case 0x11:	// Add Beginn. Parameters: times, Offset(W)
			channel.addBeginLength = channel.addBeginCount = macroPtr[1];
			channel.addBeginDelta = (int16)READ_BE_UINT16(&macroPtr[2]);
			channel.sampleStart += channel.addBeginDelta;
			Paula::setChannelSampleStart(channel.paulaChannel, getSamplePtr(channel.sampleStart));
			continue;

		case 0x12:	// Add Length. Parameters: added Length(W)
			channel.sampleLen += (int16)READ_BE_UINT16(&macroPtr[2]);
			Paula::setChannelSampleLen(channel.paulaChannel, channel.sampleLen);
			continue;

		case 0x14:	// Wait key up. Parameters: wait cycles
			if (channel.keyUp || channel.macroLoopCount == 0) {
				channel.macroLoopCount = 0xFF;
				continue;
			} else if (channel.macroLoopCount == 0xFF)
				channel.macroLoopCount = macroPtr[3];
			--channel.macroLoopCount;
			--channel.macroStep;
			return;

		case 0x15:	// Subroutine. Parameters: MacroIndex, Macrostep(W)
			channel.macroReturnOffset = channel.macroOffset;
			channel.macroReturnStep = channel.macroStep;

			channel.macroOffset = _resource->macroOffset[macroPtr[1] & (kMaxMacroOffsets - 1)];
			channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
			// TODO: MI does some weird stuff there. Figure out which varioables need to be set
			continue;

		case 0x16:	// Return from Sub.
			channel.macroOffset = channel.macroReturnOffset;
			channel.macroStep = channel.macroReturnStep;
			continue;

		case 0x17:	// Set Period. Parameters: Period(W)
			channel.refPeriod = READ_BE_UINT16(&macroPtr[2]);
			if (!channel.portaDelta) {
				channel.period = channel.refPeriod;
				//Paula::setChannelPeriod(channel.paulaChannel, channel.period);
			}
			continue;

		case 0x18: {	// Sampleloop. Parameters: Offset from Samplestart(W)
			// TODO: MI loads 24 bit, but thats useless?
			const uint16 temp = /* ((int8)macroPtr[1] << 16) | */ READ_BE_UINT16(&macroPtr[2]);
			if (macroPtr[1] || (temp & 1))
				warning("Tfmx: Problematic value for sampleloop: %06X", (macroPtr[1] << 16) | temp);
			channel.sampleStart += temp & 0xFFFE;
			channel.sampleLen -= (temp / 2) /* & 0x7FFF */;
			Paula::setChannelSampleStart(channel.paulaChannel, getSamplePtr(channel.sampleStart));
			Paula::setChannelSampleLen(channel.paulaChannel, channel.sampleLen);
			continue;
		}
		case 0x19:	// Set One-Shot Sample
			channel.addBeginLength = 0;
			channel.sampleStart = 0;
			channel.sampleLen = 1;
			Paula::setChannelSampleStart(channel.paulaChannel, getSamplePtr(0));
			Paula::setChannelSampleLen(channel.paulaChannel, 1);
			continue;

		case 0x1A:	// Wait on DMA. Parameters: Cycles-1(W) to wait
			channel.dmaIntCount = READ_BE_UINT16(&macroPtr[2]) + 1;
			channel.macroRun = false;
			Paula::setChannelDmaCount(channel.paulaChannel);
			break;

/*		case 0x1B:	// Random play. Parameters: macro/speed/mode
			warnMacroUnimplemented(macroPtr, 0);
			continue;*/

		case 0x1C:	// Branch on Note. Parameters: note/macrostep(W)
			if (channel.note > macroPtr[1])
				channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
			continue;

		case 0x1D:	// Branch on Volume. Parameters: volume/macrostep(W)
			if (channel.volume > macroPtr[1])
				channel.macroStep = READ_BE_UINT16(&macroPtr[2]);
			continue;

/*		case 0x1E:	// Addvol+note. Parameters: note/CONST./volume
			warnMacroUnimplemented(macroPtr, 0);
			continue;*/

		case 0x1F:	// AddPrevNote. Parameters: Note, Finetune(W)
			setNoteMacro(channel, channel.prevNote + macroPtr[1], READ_BE_UINT16(&macroPtr[2]));
			break;

		case 0x20:	// Signal. Parameters: signalnumber, value(W)
			if (_playerCtx.numSignals > macroPtr[1])
				_playerCtx.signal[macroPtr[1]] = READ_BE_UINT16(&macroPtr[2]);
			continue;

		case 0x21:	// Play macro. Parameters: macro, chan, detune
			noteCommand(channel.note, macroPtr[1], (channel.relVol << 4) | macroPtr[2], macroPtr[3]);
			continue;

		// 0x22 - 0x29 are used by Gem`X
		// 0x30 - 0x34 are used by Carribean Disaster

		default:
			debug(3, "Tfmx: Macro %02X not supported", macroPtr[0]);
		}
		if (!deferWait)
			return;
	}
}

void Tfmx::advancePatterns() {
startPatterns:
	int runningPatterns = 0;

	for (int i = 0; i < kNumChannels; ++i) {
		PatternContext &pattern = _patternCtx[i];
		const uint8 pattCmd = pattern.command;
		if (pattCmd < 0x90) {	// execute Patternstep
			++runningPatterns;
			if (!pattern.wait) {
				// issue all Steps for this tick
				if (patternRun(pattern)) {
					// we load the next Trackstep Command and then process all Channels again
					if (trackRun(true))
						goto startPatterns;
					else
						break;
				}

			} else
				--pattern.wait;

		} else if (pattCmd == 0xFE) {	// Stop voice in pattern.expose
			pattern.command = 0xFF;
			ChannelContext &channel = _channelCtx[pattern.expose & (kNumVoices - 1)];
			if (!channel.sfxLocked) {
				haltMacroProgramm(channel);
				Paula::disableChannel(channel.paulaChannel);
			}
		} // else this pattern-Channel is stopped
	}
	if (_playerCtx.stopWithLastPattern && !runningPatterns) {
		stopPaula();
	}
}

bool Tfmx::patternRun(PatternContext &pattern) {
	for (;;) {
		const byte *const patternPtr = (const byte *)(getPatternPtr(pattern.offset) + pattern.step);
		++pattern.step;
		const byte pattCmd = patternPtr[0];

		if (pattCmd < 0xF0) { // Playnote
			bool doWait = false;
			byte noteCmd = pattCmd + pattern.expose;
			byte param3  = patternPtr[3];
			if (pattCmd < 0xC0) {	// Note
				if (pattCmd >= 0x80) {	// Wait
					pattern.wait = param3;
					param3 = 0;
					doWait = true;
				}
				noteCmd &= 0x3F;
			}	// else Portamento
			noteCommand(noteCmd, patternPtr[1], patternPtr[2], param3);
			if (doWait)
				return false;

		} else {	// Patterncommand
			switch (pattCmd & 0xF) {
			case 0: 	// End Pattern + Next Trackstep
				pattern.command = 0xFF;
				--pattern.step;
				return true;

			case 1: 	// Loop Pattern. Parameters: Loopcount, PatternStep(W)
				if (pattern.loopCount != 0) {
					if (pattern.loopCount == 0xFF)
						pattern.loopCount = patternPtr[1];
					pattern.step = READ_BE_UINT16(&patternPtr[2]);
				}
				--pattern.loopCount;
				continue;

			case 2: 	// Jump. Parameters: PatternIndex, PatternStep(W)
				pattern.offset = _resource->patternOffset[patternPtr[1] & (kMaxPatternOffsets - 1)];
				pattern.step = READ_BE_UINT16(&patternPtr[2]);
				continue;

			case 3: 	// Wait. Paramters: ticks to wait
				pattern.wait = patternPtr[1];
				return false;

			case 14: 	// Stop custompattern
				// TODO apparently toggles on/off pattern channel 7
				debug(3, "Tfmx: Encountered 'Stop custompattern' command");
				// FT
			case 4: 	// Stop this pattern
				pattern.command = 0xFF;
				--pattern.step;
				// TODO: try figuring out if this was the last Channel?
				return false;

			case 5: 	// Key Up Signal. Paramters: channel
				if (!_channelCtx[patternPtr[2] & (kNumVoices - 1)].sfxLocked)
					_channelCtx[patternPtr[2] & (kNumVoices - 1)].keyUp = true;
				continue;

			case 6: 	// Vibrato. Parameters: length, channel, rate
			case 7: 	// Envelope. Parameters: rate, tempo | channel, endVol
				noteCommand(pattCmd, patternPtr[1], patternPtr[2], patternPtr[3]);
				continue;

			case 8: 	// Subroutine. Parameters: pattern, patternstep(W)
				pattern.savedOffset = pattern.offset;
				pattern.savedStep = pattern.step;

				pattern.offset = _resource->patternOffset[patternPtr[1] & (kMaxPatternOffsets - 1)];
				pattern.step = READ_BE_UINT16(&patternPtr[2]);
				continue;

			case 9: 	// Return from Subroutine
				pattern.offset = pattern.savedOffset;
				pattern.step = pattern.savedStep;
				continue;

			case 10:	// fade. Parameters: tempo, endVol
				initFadeCommand((uint8)patternPtr[1], (int8)patternPtr[3]);
				continue;

			case 11:	// play pattern. Parameters: patternCmd, channel, expose
				initPattern(_patternCtx[patternPtr[2] & (kNumChannels - 1)], patternPtr[1], patternPtr[3], _resource->patternOffset[patternPtr[1] & (kMaxPatternOffsets - 1)]);
				continue;

			case 12: 	// Lock. Parameters: lockFlag, channel, lockTime
				_channelCtx[patternPtr[2] & (kNumVoices - 1)].sfxLocked = (patternPtr[1] != 0);
				_channelCtx[patternPtr[2] & (kNumVoices - 1)].sfxLockTime = patternPtr[3];
				continue;

			case 13: 	// Cue. Parameters: signalnumber, value(W)
				if (_playerCtx.numSignals > patternPtr[1])
					_playerCtx.signal[patternPtr[1]] = READ_BE_UINT16(&patternPtr[2]);
				continue;

			case 15: 	// NOP
				continue;
			}
		}
	}
}

bool Tfmx::trackRun(const bool incStep) {
	assert(_playerCtx.song >= 0);
	if (incStep) {
		// TODO Optionally disable looping
		if (_trackCtx.posInd == _trackCtx.stopInd)
			_trackCtx.posInd = _trackCtx.startInd;
		else
			++_trackCtx.posInd;
	}
	for (;;) {
		const uint16 *const trackData = getTrackPtr(_trackCtx.posInd);

		if (trackData[0] != FROM_BE_16(0xEFFE)) {
			// 8 commands for Patterns
			for (int i = 0; i < 8; ++i) {
				const uint8 *patCmd = (const uint8 *)&trackData[i];
				// First byte is pattern number
				const uint8 patNum = patCmd[0];
				// if highest bit is set then keep previous pattern
				if (patNum < 0x80) {
					initPattern(_patternCtx[i], patNum, patCmd[1], _resource->patternOffset[patNum]);
				} else {
					_patternCtx[i].command = patNum;
					_patternCtx[i].expose = (int8)patCmd[1];
				}
			}
			return true;

		} else {
			// 16 byte Trackstep Command
			switch (READ_BE_UINT16(&trackData[1])) {
			case 0:	// Stop Player. No Parameters
				stopPaula();
				return false;

			case 1:	// Branch/Loop section of tracksteps. Parameters: branch target, loopcount
				if (_trackCtx.loopCount != 0) {
					if (_trackCtx.loopCount < 0)
						_trackCtx.loopCount = READ_BE_UINT16(&trackData[3]);
					_trackCtx.posInd    = READ_BE_UINT16(&trackData[2]);
					continue;
				}
				--_trackCtx.loopCount;
				break;

			case 2:	{ // Set Tempo. Parameters: tempo, divisor
				_playerCtx.patternCount = _playerCtx.patternSkip = READ_BE_UINT16(&trackData[2]); // tempo
				const uint16 temp = READ_BE_UINT16(&trackData[3]); // divisor

				if (!(temp & 0x8000) && (temp & 0x1FF))
					setInterruptFreqUnscaled(temp & 0x1FF);
				break;
			}
			case 4:	// Fade. Parameters: tempo, endVol
				// load the LSB of the 16bit words
				initFadeCommand(((const uint8 *)&trackData[2])[1], ((const int8 *)&trackData[3])[1]);
				break;

			case 3:	// Unknown, stops player aswell
			default:
				debug(3, "Tfmx: Unknown Trackstep Command: %02X", READ_BE_UINT16(&trackData[1]));
				// MI-Player handles this by stopping the player, we just continue
			}
		}

		if (_trackCtx.posInd == _trackCtx.stopInd) {
			warning("Tfmx: Reached invalid Song-Position");
			return false;
		}
		++_trackCtx.posInd;
	}
}

void Tfmx::noteCommand(const uint8 note, const uint8 param1, const uint8 param2, const uint8 param3) {
	ChannelContext &channel = _channelCtx[param2 & (kNumVoices - 1)];

	if (note == 0xFC) {	// Lock command
		channel.sfxLocked = (param1 != 0);
		channel.sfxLockTime = param3; // only 1 byte read!

	} else if (channel.sfxLocked) {	// Channel still locked, do nothing

	} else if (note < 0xC0) {	// Play Note - Parameters: note, macro, relVol | channel, finetune

		channel.prevNote = channel.note;
		channel.note = note;
		// channel.macroIndex = param1 & (kMaxMacroOffsets - 1);
		channel.macroOffset = _resource->macroOffset[param1 & (kMaxMacroOffsets - 1)];
		channel.relVol = param2 >> 4;
		channel.fineTune = (int8)param3;

		// TODO: the point where the channel gets initialized varies with the games, needs more research.
		initMacroProgramm(channel);
		channel.keyUp = false; // key down = playing a Note

	} else if (note < 0xF0) {	// Portamento - Parameters: note, tempo, channel, rate
		channel.portaSkip = param1;
		channel.portaCount = 1;
		if (!channel.portaDelta)
			channel.portaValue = channel.refPeriod;
		channel.portaDelta = param3;

		channel.note = note & 0x3F;
		channel.refPeriod = noteIntervalls[channel.note];

	} else switch (note) {	// Command

		case 0xF5:	// Key Up Signal
			channel.keyUp = true;
			break;

		case 0xF6:	// Vibratio - Parameters: length, channel, rate
			channel.vibLength = param1 & 0xFE;
			channel.vibCount = param1 / 2;
			channel.vibDelta = param3;
			channel.vibValue = 0;
			break;

		case 0xF7:	// Envelope - Parameters: rate, tempo | channel, endVol
			channel.envDelta = param1;
			channel.envCount = channel.envSkip = (param2 >> 4) + 1;
			channel.envEndVolume = param3;
			break;
	}
}

void Tfmx::initMacroProgramm(ChannelContext &channel) {
	channel.macroStep = 0;
	channel.macroWait = 0;
	channel.macroRun = true;
	channel.macroSfxRun = 0;
	channel.macroLoopCount = 0xFF;
	channel.dmaIntCount = 0;
	channel.deferWait = false;

	channel.macroReturnOffset = 0;
	channel.macroReturnStep = 0;
}

void Tfmx::clearEffects(ChannelContext &channel) {
	channel.addBeginLength = 0;
	channel.envSkip = 0;
	channel.vibLength = 0;
	channel.portaDelta = 0;
}

void Tfmx::haltMacroProgramm(ChannelContext &channel) {
	channel.macroRun = false;
	channel.dmaIntCount = 0;
}

void Tfmx::unlockMacroChannel(ChannelContext &channel) {
	channel.customMacro = 0;
	channel.customMacroIndex = 0;
	channel.customMacroPrio = 0;
	channel.sfxLocked = false;
	channel.sfxLockTime = -1;
}

void Tfmx::initPattern(PatternContext &pattern, uint8 cmd, int8 expose, uint32 offset) {
	pattern.command = cmd;
	pattern.offset = offset;
	pattern.expose = expose;
	pattern.step = 0;
	pattern.wait = 0;
	pattern.loopCount = 0xFF;

	pattern.savedOffset = 0;
	pattern.savedStep = 0;
}

void Tfmx::stopSongImpl(bool stopAudio) {
	 _playerCtx.song = -1;
	for (int i = 0; i < kNumChannels; ++i) {
		_patternCtx[i].command = 0xFF;
		_patternCtx[i].expose = 0;
	}
	if (stopAudio) {
		stopPaula();
		for (int i = 0; i < kNumVoices; ++i) {
			clearEffects(_channelCtx[i]);
			unlockMacroChannel(_channelCtx[i]);
			haltMacroProgramm(_channelCtx[i]);
			_channelCtx[i].note = 0;
			_channelCtx[i].volume = 0;
			_channelCtx[i].macroSfxRun = -1;
			_channelCtx[i].vibValue = 0;

			_channelCtx[i].sampleStart = 0;
			_channelCtx[i].sampleLen = 2;
			_channelCtx[i].refPeriod = 4;
			_channelCtx[i].period = 4;
			Paula::disableChannel(i);
		}
	}
}

void Tfmx::setNoteMacro(ChannelContext &channel, uint note, int fineTune) {
	const uint16 noteInt = noteIntervalls[note & 0x3F];
	const uint16 finetune = (uint16)(fineTune + channel.fineTune + (1 << 8));
	channel.refPeriod = ((uint32)noteInt * finetune >> 8);
	if (!channel.portaDelta)
		channel.period = channel.refPeriod;
}

void Tfmx::initFadeCommand(const uint8 fadeTempo, const int8 endVol) {
	_playerCtx.fadeCount = _playerCtx.fadeSkip = fadeTempo;
	_playerCtx.fadeEndVolume = endVol;

	if (fadeTempo) {
		const int diff = _playerCtx.fadeEndVolume - _playerCtx.volume;
		_playerCtx.fadeDelta = (diff != 0) ? ((diff > 0) ? 1 : -1) : 0;
	} else {
		_playerCtx.volume = endVol;
		_playerCtx.fadeDelta = 0;
	}
}

void Tfmx::setModuleData(Tfmx &otherPlayer) {
	setModuleData(otherPlayer._resource, otherPlayer._resourceSample.sampleData, otherPlayer._resourceSample.sampleLen, false);
}

bool Tfmx::load(Common::SeekableReadStream &musicData, Common::SeekableReadStream &sampleData, bool autoDelete) {
	const MdatResource *mdat = loadMdatFile(musicData);
	if (mdat) {
		uint32 sampleLen = 0;
		const int8 *sampleDat = loadSampleFile(sampleLen, sampleData);
		if (sampleDat) {
			setModuleData(mdat, sampleDat, sampleLen, autoDelete);
			return true;
		}
		delete[] mdat->mdatAlloc;
		delete mdat;
	}
	return false;
}

void Tfmx::freeResourceDataImpl() {
	if (_deleteResource) {
		if (_resource) {
			delete[] _resource->mdatAlloc;
			delete _resource;
		}
		delete[] _resourceSample.sampleData;
	}
	_resource = 0;
	_resourceSample.sampleData = 0;
	_resourceSample.sampleLen = 0;
	_deleteResource = false;
}

void Tfmx::setModuleData(const MdatResource *resource, const int8 *sampleData, uint32 sampleLen, bool autoDelete) {
	Common::StackLock lock(_mutex);
	stopSongImpl(true);
	freeResourceDataImpl();
	_resource = resource;
	_resourceSample.sampleData = sampleData;
	_resourceSample.sampleLen = sampleData ? sampleLen : 0;
	_deleteResource = autoDelete;
}

const int8 *Tfmx::loadSampleFile(uint32 &sampleLen, Common::SeekableReadStream &sampleStream) {
	sampleLen = 0;

	const int32 sampleSize = sampleStream.size();
	if (sampleSize < 4) {
		warning("Tfmx: Cant load Samplefile");
		return 0;
	}

	int8 *sampleAlloc = new int8[sampleSize];
	if (!sampleAlloc) {
		warning("Tfmx: Could not allocate Memory: %dKB", sampleSize / 1024);
		return 0;
	}

	if (sampleStream.read(sampleAlloc, sampleSize) == (uint32)sampleSize) {
		sampleAlloc[0] = sampleAlloc[1] = sampleAlloc[2] = sampleAlloc[3] = 0;
		sampleLen = sampleSize;
	} else {
		delete[] sampleAlloc;
		warning("Tfmx: Encountered IO-Error");
		return 0;
	}
	return sampleAlloc;
}

const Tfmx::MdatResource *Tfmx::loadMdatFile(Common::SeekableReadStream &musicData) {
	bool hasHeader = false;
	const int32 mdatSize = musicData.size();
	if (mdatSize >= 0x200) {
		byte buf[16] = { 0 };
		// 0x0000: 10 Bytes Header "TFMX-SONG "
		musicData.read(buf, 10);
		hasHeader = memcmp(buf, "TFMX-SONG ", 10) == 0;
	}

	if (!hasHeader) {
		warning("Tfmx: File is not a Tfmx Module");
		return 0;
	}

	MdatResource *resource = new MdatResource;

	resource->mdatAlloc = 0;
	resource->mdatData = 0;
	resource->mdatLen = 0;

	// 0x000A: int16 flags
	resource->headerFlags = musicData.readUint16BE();
	// 0x000C: int32 ?
	// 0x0010: 6*40 Textfield
	musicData.skip(4 + 6 * 40);

	/* 0x0100: Songstart x 32*/
	for (int i = 0; i < kNumSubsongs; ++i)
		resource->subsong[i].songstart = musicData.readUint16BE();
	/* 0x0140: Songend x 32*/
	for (int i = 0; i < kNumSubsongs; ++i)
		resource->subsong[i].songend = musicData.readUint16BE();
	/* 0x0180: Tempo x 32*/
	for (int i = 0; i < kNumSubsongs; ++i)
		resource->subsong[i].tempo  = musicData.readUint16BE();

	/* 0x01c0: unused ? */
	musicData.skip(16);

	/* 0x01d0: trackstep, pattern data p, macro data p */
	const uint32 offTrackstep = musicData.readUint32BE();
	uint32 offPatternP, offMacroP;

	// This is how MI`s TFMX-Player tests for unpacked Modules.
	if (offTrackstep == 0) { // unpacked File
		resource->trackstepOffset = 0x600 + 0x200;
		offPatternP		= 0x200 + 0x200;
		offMacroP		= 0x400 + 0x200;
	} else { // packed File
		resource->trackstepOffset = offTrackstep;
		offPatternP = musicData.readUint32BE();
		offMacroP = musicData.readUint32BE();
	}

	// End of basic header, check if everything worked ok
	if (musicData.err()) {
		warning("Tfmx: Encountered IO-Error");
		delete resource;
		return 0;
	}

	// TODO: if a File is packed it could have for Ex only 2 Patterns/Macros
	// the following loops could then read beyond EOF.
	// To correctly handle this it would be necessary to sort the pointers and
	// figure out the number of Macros/Patterns
	// We could also analyze pointers if they are correct offsets,
	// so that accesses can be unchecked later

	// Read in pattern starting offsets
	musicData.seek(offPatternP);
	for (int i = 0; i < kMaxPatternOffsets; ++i)
		resource->patternOffset[i] = musicData.readUint32BE();

	// use last PatternOffset (stored at 0x5FC in mdat) if unpacked File
	// or fixed offset 0x200 if packed
	resource->sfxTableOffset = offTrackstep ? 0x200 : resource->patternOffset[127];

	// Read in macro starting offsets
	musicData.seek(offMacroP);
	for (int i = 0; i < kMaxMacroOffsets; ++i)
		resource->macroOffset[i] = musicData.readUint32BE();

	// Read in mdat-file
	// TODO: we can skip everything thats already stored in the resource-structure.
	const int32 mdatOffset = offTrackstep ? 0x200 : 0x600;	// 0x200 is very conservative
	const uint32 allocSize = (uint32)mdatSize - mdatOffset;

	byte *mdatAlloc = new byte[allocSize];
	if (!mdatAlloc) {
		warning("Tfmx: Could not allocate Memory: %dKB", allocSize / 1024);
		delete resource;
		return 0;
	}
	musicData.seek(mdatOffset);
	if (musicData.read(mdatAlloc, allocSize) == allocSize) {
		resource->mdatAlloc = mdatAlloc;
		resource->mdatData = mdatAlloc - mdatOffset;
		resource->mdatLen = mdatSize;
	} else {
		delete[] mdatAlloc;
		warning("Tfmx: Encountered IO-Error");
		delete resource;
		return 0;
	}

	return resource;
}

void Tfmx::doMacro(int note, int macro, int relVol, int finetune, int channelNo) {
	assert(0 <= macro && macro < kMaxMacroOffsets);
	assert(0 <= note && note < 0xC0);
	Common::StackLock lock(_mutex);

	if (!hasResources())
		return;
	channelNo &= (kNumVoices - 1);
	ChannelContext &channel = _channelCtx[channelNo];
	unlockMacroChannel(channel);

	noteCommand((uint8)note, (uint8)macro, (uint8)((relVol << 4) | channelNo), (uint8)finetune);
	startPaula();
}

void Tfmx::stopMacroEffect(int channel) {
	assert(0 <= channel && channel < kNumVoices);
	Common::StackLock lock(_mutex);
	unlockMacroChannel(_channelCtx[channel]);
	haltMacroProgramm(_channelCtx[channel]);
	Paula::disableChannel(_channelCtx[channel].paulaChannel);
}

void Tfmx::doSong(int songPos, bool stopAudio) {
	assert(0 <= songPos && songPos < kNumSubsongs);
	Common::StackLock lock(_mutex);

	stopSongImpl(stopAudio);

	if (!hasResources())
		return;

	_trackCtx.loopCount = -1;
	_trackCtx.startInd = _trackCtx.posInd = _resource->subsong[songPos].songstart;
	_trackCtx.stopInd = _resource->subsong[songPos].songend;
	_playerCtx.song = (int8)songPos;

	const bool palFlag = (_resource->headerFlags & 2) != 0;
	const uint16 tempo = _resource->subsong[songPos].tempo;
	uint16 ciaIntervall;
	if (tempo >= 0x10) {
		ciaIntervall = (uint16)(kCiaBaseInterval / tempo);
		_playerCtx.patternSkip = 0;
	} else {
		ciaIntervall = palFlag ? (uint16)kPalDefaultCiaVal : (uint16)kNtscDefaultCiaVal;
		_playerCtx.patternSkip = tempo;
	}
	setInterruptFreqUnscaled(ciaIntervall);
	Paula::setAudioFilter(true);

	_playerCtx.patternCount = 0;
	if (trackRun())
		startPaula();
}

int Tfmx::doSfx(uint16 sfxIndex, bool unlockChannel) {
	assert(sfxIndex < 128);
	Common::StackLock lock(_mutex);

	if (!hasResources())
		return -1;
	const byte *sfxEntry = getSfxPtr(sfxIndex);
	if (sfxEntry[0] == 0xFB) {
		warning("Tfmx: custom patterns are not supported");
		// custompattern
		/* const uint8 patCmd = sfxEntry[2];
		const int8 patExp = (int8)sfxEntry[3]; */
	} else {
		// custommacro
		const byte channelNo = ((_playerCtx.song >= 0) ? sfxEntry[2] : sfxEntry[4]) & (kNumVoices - 1);
		const byte priority = sfxEntry[5] & 0x7F;

		ChannelContext &channel = _channelCtx[channelNo];
		if (unlockChannel)
			unlockMacroChannel(channel);

		const int16 sfxLocktime = channel.sfxLockTime;
		if (priority >= channel.customMacroPrio || sfxLocktime < 0) {
			if (sfxIndex != channel.customMacroIndex || sfxLocktime < 0 || (sfxEntry[5] < 0x80)) {
				channel.customMacro = READ_UINT32(sfxEntry); // intentionally not "endian-correct"
				channel.customMacroPrio = priority;
				channel.customMacroIndex = (uint8)sfxIndex;
				debug(3, "Tfmx: running Macro %08X on channel %i - priority: %02X", TO_BE_32(channel.customMacro), channelNo, priority);
				return channelNo;
			}
		}
	}
	return -1;
}

}	// End of namespace Audio

// some debugging functions
#if 0
namespace {

void displayMacroStep(const void * const vptr) {
	static const char *tableMacros[] = {
		"DMAoff+Resetxx/xx/xx flag/addset/vol   ",
		"DMAon (start sample at selected begin) ",
		"SetBegin    xxxxxx   sample-startadress",
		"SetLen      ..xxxx   sample-length     ",
		"Wait        ..xxxx   count (VBI''s)    ",
		"Loop        xx/xxxx  count/step        ",
		"Cont        xx/xxxx  macro-number/step ",
		"-------------STOP----------------------",
		"AddNote     xx/xxxx  note/detune       ",
		"SetNote     xx/xxxx  note/detune       ",
		"Reset   Vibrato-Portamento-Envelope    ",
		"Portamento  xx/../xx count/speed       ",
		"Vibrato     xx/../xx speed/intensity   ",
		"AddVolume   ....xx   volume 00-3F      ",
		"SetVolume   ....xx   volume 00-3F      ",
		"Envelope    xx/xx/xx speed/count/endvol",
		"Loop key up xx/xxxx  count/step        ",
		"AddBegin    xx/xxxx  count/add to start",
		"AddLen      ..xxxx   add to sample-len ",
		"DMAoff stop sample but no clear        ",
		"Wait key up ....xx   count (VBI''s)    ",
		"Go submacro xx/xxxx  macro-number/step ",
		"--------Return to old macro------------",
		"Setperiod   ..xxxx   DMA period        ",
		"Sampleloop  ..xxxx   relative adress   ",
		"-------Set one shot sample-------------",
		"Wait on DMA ..xxxx   count (Wavecycles)",
		"Random play xx/xx/xx macro/speed/mode  ",
		"Splitkey    xx/xxxx  key/macrostep     ",
		"Splitvolume xx/xxxx  volume/macrostep  ",
		"Addvol+note xx/fe/xx note/CONST./volume",
		"SetPrevNote xx/xxxx  note/detune       ",
		"Signal      xx/xxxx  signalnumber/value",
		"Play macro  xx/.x/xx macro/chan/detune ",
		"SID setbeg  xxxxxx   sample-startadress",
		"SID setlen  xx/xxxx  buflen/sourcelen  ",
		"SID op3 ofs xxxxxx   offset            ",
		"SID op3 frq xx/xxxx  speed/amplitude   ",
		"SID op2 ofs xxxxxx   offset            ",
		"SID op2 frq xx/xxxx  speed/amplitude   ",
		"SID op1     xx/xx/xx speed/amplitude/TC",
		"SID stop    xx....   flag (1=clear all)"
	};

	const byte *const macroData = (const byte * const)vptr;
	if (macroData[0] < ARRAYSIZE(tableMacros))
		debug("%s %02X%02X%02X", tableMacros[macroData[0]], macroData[1], macroData[2], macroData[3]);
	else
		debug("Unknown Macro #%02X %02X%02X%02X", macroData[0], macroData[1], macroData[2], macroData[3]);
}

void displayPatternstep(const void * const vptr) {
	static const char *tablePatterns[] = {
		"End --Next track  step--",
		"Loop[count     / step.w]",
		"Cont[patternno./ step.w]",
		"Wait[count 00-FF--------",
		"Stop--Stop this pattern-",
		"Kup^-Set key up/channel]",
		"Vibr[speed     / rate.b]",
		"Enve[speed /endvolume.b]",
		"GsPt[patternno./ step.w]",
		"RoPt-Return old pattern-",
		"Fade[speed /endvolume.b]",
		"PPat[patt./track+transp]",
		"Lock---------ch./time.b]",
		"Cue [number.b/  value.w]",
		"Stop-Stop custompattern-",
		"NOP!-no operation-------"
	};

	const byte * const patData = (const byte * const)vptr;
	const byte command = patData[0];
	if (command < 0xF0) { // Playnote
		const byte flags = command >> 6; // 0-1 means note+detune, 2 means wait, 3 means portamento?
		const char *flagsSt[] = { "Note ", "Note ", "Wait ", "Porta" };
		debug("%s %02X%02X%02X%02X", flagsSt[flags], patData[0], patData[1], patData[2], patData[3]);
	} else
		debug("%s %02X%02X%02X",tablePatterns[command & 0xF], patData[1], patData[2], patData[3]);
}

} // End of anonymous namespace
#endif

#endif // #if defined(AUDIO_MODS_TFMX_H)
