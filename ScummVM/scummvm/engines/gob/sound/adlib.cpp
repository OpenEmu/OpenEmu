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

#include "common/util.h"
#include "common/textconsole.h"
#include "common/debug.h"
#include "common/config-manager.h"

#include "audio/fmopl.h"

#include "gob/gob.h"
#include "gob/sound/adlib.h"

namespace Gob {

static const int kPitchTom        = 24;
static const int kPitchTomToSnare =  7;
static const int kPitchSnareDrum  = kPitchTom + kPitchTomToSnare;


// Is the operator a modulator (0) or a carrier (1)?
const uint8 AdLib::kOperatorType[kOperatorCount] = {
	0, 0, 0, 1, 1, 1,
	0, 0, 0, 1, 1, 1,
	0, 0, 0, 1, 1, 1
};

// Operator number to register offset on the OPL
const uint8 AdLib::kOperatorOffset[kOperatorCount] = {
	 0,  1,  2,  3,  4,  5,
	 8,  9, 10, 11, 12, 13,
	16, 17, 18, 19, 20, 21
};

// For each operator, the voice it belongs to
const uint8 AdLib::kOperatorVoice[kOperatorCount] = {
	0, 1, 2,
	0, 1, 2,
	3, 4, 5,
	3, 4, 5,
	6, 7, 8,
	6, 7, 8,
};

// Voice to operator set, for the 9 melodyvoices (only 6 useable in percussion mode)
const uint8 AdLib::kVoiceMelodyOperator[kOperatorsPerVoice][kMelodyVoiceCount] = {
	{0, 1, 2, 6,  7,  8, 12, 13, 14},
	{3, 4, 5, 9, 10, 11, 15, 16, 17}
};

// Voice to operator set, for the 5 percussion voices (only useable in percussion mode)
const uint8 AdLib::kVoicePercussionOperator[kOperatorsPerVoice][kPercussionVoiceCount] = {
	{12, 16, 14, 17, 13},
	{15,  0,  0,  0,  0}
};

// Mask bits to set each percussion instrument on/off
const byte AdLib::kPercussionMasks[kPercussionVoiceCount] = {0x10, 0x08, 0x04, 0x02, 0x01};

// Default instrument presets
const uint16 AdLib::kPianoParams   [kOperatorsPerVoice][kParamCount] = {
	{ 1,  1,  3, 15,  5,  0,  1,  3, 15,  0,  0,  0,  1,  0},
	{ 0,  1,  1, 15,  7,  0,  2,  4,  0,  0,  0,  1,  0,  0}  };
const uint16 AdLib::kBaseDrumParams[kOperatorsPerVoice][kParamCount] = {
	{ 0,  0,  0, 10,  4,  0,  8, 12, 11,  0,  0,  0,  1,  0 },
	{ 0,  0,  0, 13,  4,  0,  6, 15,  0,  0,  0,  0,  1,  0 } };
const uint16 AdLib::kSnareDrumParams[kParamCount] = {
	  0, 12,  0, 15, 11,  0,  8,  5,  0,  0,  0,  0,  0,  0   };
const uint16 AdLib::kTomParams      [kParamCount] = {
	  0,  4,  0, 15, 11,  0,  7,  5,  0,  0,  0,  0,  0,  0   };
const uint16 AdLib::kCymbalParams   [kParamCount] = {
	  0,  1,  0, 15, 11,  0,  5,  5,  0,  0,  0,  0,  0,  0   };
const uint16 AdLib::kHihatParams    [kParamCount] = {
	  0,  1,  0, 15, 11,  0,  7,  5,  0,  0,  0,  0,  0,  0   };


AdLib::AdLib(Audio::Mixer &mixer) : _mixer(&mixer), _opl(0),
	_toPoll(0), _repCount(0), _first(true), _playing(false), _ended(true) {

	_rate = _mixer->getOutputRate();

	initFreqs();

	createOPL();
	initOPL();

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_handle,
			this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

AdLib::~AdLib() {
	_mixer->stopHandle(_handle);

	delete _opl;
}

// Creates the OPL. Try to use the DOSBox emulator, unless that one is not compiled in,
// or the user explicitly wants the MAME emulator. The MAME one is slightly buggy, leading
// to some wrong sounds, especially noticeable in the title music of Gobliins 2, so we
// really don't want to use it, if we can help it.
void AdLib::createOPL() {
	Common::String oplDriver = ConfMan.get("opl_driver");

	if (oplDriver.empty() || (oplDriver == "auto") || (OPL::Config::parse(oplDriver) == -1)) {
		// User has selected OPL driver auto detection or an invalid OPL driver.
		// Set it to our preferred driver (DOSBox), if we can.

		if (OPL::Config::parse("db") <= 0) {
			warning("The DOSBox AdLib emulator is not compiled in. Please keep in mind that the MAME one is buggy");
		} else
			oplDriver = "db";

	} else if (oplDriver == "mame") {
		// User has selected the MAME OPL driver. It is buggy, so warn the user about that.

		warning("You have selected the MAME AdLib emulator. It is buggy; AdLib music might be slightly glitchy now");
	}

	_opl = OPL::Config::create(OPL::Config::parse(oplDriver), OPL::Config::kOpl2);
	if (!_opl || !_opl->init(_rate)) {
		delete _opl;

		error("Could not create an AdLib emulator");
	}
}

int AdLib::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock slock(_mutex);

	// Nothing to do, fill with silence
	if (!_playing) {
		memset(buffer, 0, numSamples * sizeof(int16));
		return numSamples;
	}

	// Read samples from the OPL, polling in more music when necessary
	uint32 samples = numSamples;
	while (samples && _playing) {
		if (_toPoll) {
			const uint32 render = MIN(samples, _toPoll);

			_opl->readBuffer(buffer, render);

			buffer  += render;
			samples -= render;
			_toPoll -= render;

		} else {
			// Song ended, fill the rest with silence
			if (_ended) {
				memset(buffer, 0, samples * sizeof(int16));
				samples = 0;
				break;
			}

			// Poll more music
			_toPoll = pollMusic(_first);
			_first  = false;
		}
	}

	// Song ended, loop if requested
	if (_ended) {
		_toPoll = 0;

		// _repCount == 0: No looping (anymore); _repCount < 0: Infinite looping
		if (_repCount != 0) {
			if (_repCount > 0)
				_repCount--;

			_first  = true;
			_ended  = false;

			reset();
			rewind();
		} else
			_playing = false;
	}

	return numSamples;
}

bool AdLib::isStereo() const {
	return _opl->isStereo();
}

bool AdLib::endOfData() const {
	return !_playing;
}

bool AdLib::endOfStream() const {
	return false;
}

int AdLib::getRate() const {
	return _rate;
}

bool AdLib::isPlaying() const {
	return _playing;
}

int32 AdLib::getRepeating() const {
	Common::StackLock slock(_mutex);

	return _repCount;
}

void AdLib::setRepeating(int32 repCount) {
	Common::StackLock slock(_mutex);

	_repCount = repCount;
}

uint32 AdLib::getSamplesPerSecond() const {
	return _rate * (isStereo() ? 2 : 1);
}

void AdLib::startPlay() {
	Common::StackLock slock(_mutex);

	_playing = true;
	_ended   = false;
	_first   = true;

	reset();
	rewind();
}

void AdLib::stopPlay() {
	Common::StackLock slock(_mutex);

	end(true);

	_playing = false;
}

void AdLib::writeOPL(byte reg, byte val) {
	debugC(6, kDebugSound, "AdLib::writeOPL (%02X, %02X)", reg, val);

	_opl->writeReg(reg, val);
}

void AdLib::reset() {
	allOff();
	initOPL();
}

void AdLib::allOff() {
	// NOTE: Explicit casts are necessary, because of 5.16 paragraph 4 of the C++ standard
	int numVoices = isPercussionMode() ? (int)kMaxVoiceCount : (int)kMelodyVoiceCount;

	for (int i = 0; i < numVoices; i++)
		noteOff(i);
}

void AdLib::end(bool killRepeat) {
	reset();

	_ended = true;

	if (killRepeat)
		_repCount = 0;
}

void AdLib::initOPL() {
	_tremoloDepth = false;
	_vibratoDepth = false;
	_keySplit     = false;

	_enableWaveSelect = true;

	for (int i = 0; i < kMaxVoiceCount; i++) {
		_voiceNote[i] = 0;
		_voiceOn  [i] = 0;
	}

	_opl->reset();

	initOperatorVolumes();
	resetFreqs();

	setPercussionMode(false);

	setTremoloDepth(false);
	setVibratoDepth(false);
	setKeySplit(false);

	for(int i = 0; i < kMelodyVoiceCount; i++)
		voiceOff(i);

	setPitchRange(1);

	enableWaveSelect(true);
}

bool AdLib::isPercussionMode() const {
	return _percussionMode;
}

void AdLib::setPercussionMode(bool percussion) {
	if (percussion) {
		voiceOff(kVoiceBaseDrum);
		voiceOff(kVoiceSnareDrum);
		voiceOff(kVoiceTom);

		/* set the frequency for the last 4 percussion voices: */
		setFreq(kVoiceTom, kPitchTom, 0);
		setFreq(kVoiceSnareDrum, kPitchSnareDrum, 0);
	}

	_percussionMode = percussion;
	_percussionBits = 0;

	initOperatorParams();
	writeTremoloVibratoDepthPercMode();
}

void AdLib::enableWaveSelect(bool enable) {
	_enableWaveSelect = enable;

	for (int i = 0; i < kOperatorCount; i++)
		writeOPL(0xE0 + kOperatorOffset[i], 0);

	writeOPL(0x011, _enableWaveSelect ? 0x20 : 0);
}

void AdLib::setPitchRange(uint8 range) {
	_pitchRange     = CLIP<uint8>(range, 0, 12);
	_pitchRangeStep = _pitchRange * kPitchStepCount;
}

void AdLib::setTremoloDepth(bool tremoloDepth) {
	_tremoloDepth = tremoloDepth;

	writeTremoloVibratoDepthPercMode();
}

void AdLib::setVibratoDepth(bool vibratoDepth) {
	_vibratoDepth = vibratoDepth;

	writeTremoloVibratoDepthPercMode();
}

void AdLib::setKeySplit(bool keySplit) {
	_keySplit = keySplit;

	writeKeySplit();
}

void AdLib::setVoiceTimbre(uint8 voice, const uint16 *params) {
	const uint16 *params0 = params;
	const uint16 *params1 = params + kParamCount - 1;
	const uint16 *waves   = params + 2 * (kParamCount - 1);

	const int voicePerc = voice - kVoiceBaseDrum;

	if (!isPercussionMode() || (voice < kVoiceBaseDrum)) {
		setOperatorParams(kVoiceMelodyOperator[0][voice], params0, waves[0]);
		setOperatorParams(kVoiceMelodyOperator[1][voice], params1, waves[1]);
	} else if (voice == kVoiceBaseDrum) {
		setOperatorParams(kVoicePercussionOperator[0][voicePerc], params0, waves[0]);
		setOperatorParams(kVoicePercussionOperator[1][voicePerc], params1, waves[1]);
	} else {
		setOperatorParams(kVoicePercussionOperator[0][voicePerc], params0, waves[0]);
	}
}

void AdLib::setVoiceVolume(uint8 voice, uint8 volume) {
	int oper;

	const int voicePerc = voice - kVoiceBaseDrum;

	if (!isPercussionMode() || (voice < kVoiceBaseDrum))
		oper = kVoiceMelodyOperator[1][ voice];
	else
		oper = kVoicePercussionOperator[voice == kVoiceBaseDrum ? 1 : 0][voicePerc];

	_operatorVolume[oper] = MIN<uint8>(volume, kMaxVolume);
	writeKeyScaleLevelVolume(oper);
}

void AdLib::bendVoicePitch(uint8 voice, uint16 pitchBend) {
	if (isPercussionMode() && (voice > kVoiceBaseDrum))
		return;

	changePitch(voice, MIN<uint16>(pitchBend, kMaxPitch));
	setFreq(voice, _voiceNote[voice], _voiceOn[voice]);
}

void AdLib::noteOn(uint8 voice, uint8 note) {
	note = MAX<int>(0, note - (kStandardMidC - kOPLMidC));

	if (isPercussionMode() && (voice >= kVoiceBaseDrum)) {

		if        (voice == kVoiceBaseDrum) {
			setFreq(kVoiceBaseDrum , note                   , false);
		} else if (voice == kVoiceTom) {
			setFreq(kVoiceTom      , note                   , false);
			setFreq(kVoiceSnareDrum, note + kPitchTomToSnare, false);
		}

		_percussionBits |= kPercussionMasks[voice - kVoiceBaseDrum];
		writeTremoloVibratoDepthPercMode();

	} else
		setFreq(voice, note, true);
}

void AdLib::noteOff(uint8 voice) {
	if (isPercussionMode() && (voice >= kVoiceBaseDrum)) {
		_percussionBits &= ~kPercussionMasks[voice - kVoiceBaseDrum];
		writeTremoloVibratoDepthPercMode();
	} else
		setFreq(voice, _voiceNote[voice], false);
}

void AdLib::writeKeyScaleLevelVolume(uint8 oper) {
	uint16 volume = 0;

	volume = (63 - (_operatorParams[oper][kParamLevel] & 0x3F)) * _operatorVolume[oper];
	volume = 63 - ((2 * volume + kMaxVolume) / (2 * kMaxVolume));

	uint8 keyScale = _operatorParams[oper][kParamKeyScaleLevel] << 6;

	writeOPL(0x40 + kOperatorOffset[oper], volume | keyScale);
}

void AdLib::writeKeySplit() {
	writeOPL(0x08, _keySplit ? 0x40 : 0);
}

void AdLib::writeFeedbackFM(uint8 oper) {
	if (kOperatorType[oper] == 1)
		return;

	uint8 value = 0;

	value |= _operatorParams[oper][kParamFeedback] << 1;
	value |= _operatorParams[oper][kParamFM] ? 0 : 1;

	writeOPL(0xC0 + kOperatorVoice[oper], value);
}

void AdLib::writeAttackDecay(uint8 oper) {
	uint8 value = 0;

	value |= _operatorParams[oper][kParamAttack] << 4;
	value |= _operatorParams[oper][kParamDecay] & 0x0F;

	writeOPL(0x60 + kOperatorOffset[oper], value);
}

void AdLib::writeSustainRelease(uint8 oper) {
	uint8 value = 0;

	value |= _operatorParams[oper][kParamSustain] << 4;
	value |= _operatorParams[oper][kParamRelease] & 0x0F;

	writeOPL(0x80 + kOperatorOffset[oper], value);
}

void AdLib::writeTremoloVibratoSustainingKeyScaleRateFreqMulti(uint8 oper) {
	uint8 value = 0;

	value |= _operatorParams[oper][kParamAM]           ? 0x80 : 0;
	value |= _operatorParams[oper][kParamVib]          ? 0x40 : 0;
	value |= _operatorParams[oper][kParamSustaining]   ? 0x20 : 0;
	value |= _operatorParams[oper][kParamKeyScaleRate] ? 0x10 : 0;
	value |= _operatorParams[oper][kParamFreqMulti]    & 0x0F;

	writeOPL(0x20 + kOperatorOffset[oper], value);
}

void AdLib::writeTremoloVibratoDepthPercMode() {
	uint8 value = 0;

	value |= _tremoloDepth       ? 0x80 : 0;
	value |= _vibratoDepth       ? 0x40 : 0;
	value |= isPercussionMode() ? 0x20 : 0;
	value |= _percussionBits;

	writeOPL(0xBD, value);
}

void AdLib::writeWaveSelect(uint8 oper) {
	uint8 wave = 0;
	if (_enableWaveSelect)
		wave = _operatorParams[oper][kParamWaveSelect] & 0x03;

	writeOPL(0xE0 + kOperatorOffset[ oper], wave);
}

void AdLib::writeAllParams(uint8 oper) {
	writeTremoloVibratoDepthPercMode();
	writeKeySplit();
	writeKeyScaleLevelVolume(oper);
	writeFeedbackFM(oper);
	writeAttackDecay(oper);
	writeSustainRelease(oper);
	writeTremoloVibratoSustainingKeyScaleRateFreqMulti(oper);
	writeWaveSelect(oper);
}

void AdLib::initOperatorParams() {
	for (int i = 0; i < kOperatorCount; i++)
		setOperatorParams(i, kPianoParams[kOperatorType[i]], kPianoParams[kOperatorType[i]][kParamCount - 1]);

	if (isPercussionMode()) {
		setOperatorParams(12, kBaseDrumParams [0], kBaseDrumParams [0][kParamCount - 1]);
		setOperatorParams(15, kBaseDrumParams [1], kBaseDrumParams [1][kParamCount - 1]);
		setOperatorParams(16, kSnareDrumParams   , kSnareDrumParams   [kParamCount - 1]);
		setOperatorParams(14, kTomParams         , kTomParams         [kParamCount - 1]);
		setOperatorParams(17, kCymbalParams      , kCymbalParams      [kParamCount - 1]);
		setOperatorParams(13, kHihatParams       , kHihatParams       [kParamCount - 1]);
	}
}

void AdLib::initOperatorVolumes() {
	for(int i = 0; i < kOperatorCount; i++)
		_operatorVolume[i] = kMaxVolume;
}

void AdLib::setOperatorParams(uint8 oper, const uint16 *params, uint8 wave) {
	byte *operParams = _operatorParams[oper];

	for (int i = 0; i < (kParamCount - 1); i++)
		operParams[i] = params[i];

	operParams[kParamCount - 1] = wave & 0x03;

	writeAllParams(oper);
}

void AdLib::voiceOff(uint8 voice) {
	writeOPL(0xA0 + voice, 0);
	writeOPL(0xB0 + voice, 0);
}

int32 AdLib::calcFreq(int32 deltaDemiToneNum, int32 deltaDemiToneDenom) {
	int32 freq = 0;

	freq  = ((deltaDemiToneDenom * 100) + 6 * deltaDemiToneNum) * 52088;
	freq /= deltaDemiToneDenom * 2500;

	return (freq * 147456) / 111875;
}

void AdLib::setFreqs(uint16 *freqs, int32 num, int32 denom) {
	int32 val = calcFreq(num, denom);

	*freqs++ = (4 + val) >> 3;

	for (int i = 1; i < kHalfToneCount; i++) {
		val = (val * 106) / 100;

		*freqs++ = (4 + val) >> 3;
	}
}

void AdLib::initFreqs() {
	const int numStep = 100 / kPitchStepCount;

	for (int i = 0; i < kPitchStepCount; i++)
		setFreqs(_freqs[i], i * numStep, 100);

	resetFreqs();
}

void AdLib::resetFreqs() {
	for (int i = 0; i < kMaxVoiceCount; i++) {
		_freqPtr       [i] = _freqs[0];
		_halfToneOffset[i] = 0;
	}
}

void AdLib::changePitch(uint8 voice, uint16 pitchBend) {

	int full   = 0;
	int frac   = 0;
	int amount = ((pitchBend - kMidPitch) * _pitchRangeStep) / kMidPitch;

	if (amount >= 0) {
		// Bend up

		full = amount / kPitchStepCount;
		frac = amount % kPitchStepCount;

	} else {
		// Bend down

		amount = kPitchStepCount - 1 - amount;

		full = -(amount / kPitchStepCount);
		frac = (amount - kPitchStepCount + 1) % kPitchStepCount;
		if (frac)
			frac = kPitchStepCount - frac;

	}

	_halfToneOffset[voice] = full;
	_freqPtr       [voice] = _freqs[frac];
}

void AdLib::setFreq(uint8 voice, uint16 note, bool on) {
	_voiceOn  [voice] = on;
	_voiceNote[voice] = note;

	note = CLIP<int>(note + _halfToneOffset[voice], 0, kNoteCount - 1);

	uint16 freq = _freqPtr[voice][note % kHalfToneCount];

	uint8 value = 0;
	value |= on ? 0x20 : 0;
	value |= ((note / kHalfToneCount) << 2) | ((freq >> 8) & 0x03);

	writeOPL(0xA0 + voice, freq);
	writeOPL(0xB0 + voice, value);
}

} // End of namespace Gob
