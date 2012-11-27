/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//#include <cmath>
//#include <cstdlib>
//#include <cstring>

#include "mt32emu.h"
#include "mmath.h"

namespace MT32Emu {

#ifdef INACCURATE_SMOOTH_PAN
// Mok wanted an option for smoother panning, and we love Mok.
static const float PAN_NUMERATOR_NORMAL[] = {0.0f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 4.5f, 5.0f, 5.5f, 6.0f, 6.5f, 7.0f};
#else
// CONFIRMED by Mok: These NUMERATOR values (as bytes, not floats, obviously) are sent exactly like this to the LA32.
static const float PAN_NUMERATOR_NORMAL[] = {0.0f, 0.0f, 1.0f, 1.0f, 2.0f, 2.0f, 3.0f, 3.0f, 4.0f, 4.0f, 5.0f, 5.0f, 6.0f, 6.0f, 7.0f};
#endif
static const float PAN_NUMERATOR_MASTER[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f};
static const float PAN_NUMERATOR_SLAVE[]  = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f, 7.0f};

Partial::Partial(Synth *useSynth, int useDebugPartialNum) :
	synth(useSynth), debugPartialNum(useDebugPartialNum), sampleNum(0), tva(new TVA(this, &ampRamp)), tvp(new TVP(this)), tvf(new TVF(this, &cutoffModifierRamp)) {
	ownerPart = -1;
	poly = NULL;
	pair = NULL;
}

Partial::~Partial() {
	delete tva;
	delete tvp;
	delete tvf;
}

// Only used for debugging purposes
int Partial::debugGetPartialNum() const {
	return debugPartialNum;
}

// Only used for debugging purposes
unsigned long Partial::debugGetSampleNum() const {
	return sampleNum;
}

int Partial::getOwnerPart() const {
	return ownerPart;
}

bool Partial::isActive() const {
	return ownerPart > -1;
}

const Poly *Partial::getPoly() const {
	return poly;
}

void Partial::activate(int part) {
	// This just marks the partial as being assigned to a part
	ownerPart = part;
}

void Partial::deactivate() {
	if (!isActive()) {
		return;
	}
	ownerPart = -1;
	if (poly != NULL) {
		poly->partialDeactivated(this);
		if (pair != NULL) {
			pair->pair = NULL;
		}
	}
#if MT32EMU_MONITOR_PARTIALS > 2
	synth->printDebug("[+%lu] [Partial %d] Deactivated", sampleNum, debugPartialNum);
	synth->printPartialUsage(sampleNum);
#endif
}

// DEPRECATED: This should probably go away eventually, it's currently only used as a kludge to protect our old assumptions that
// rhythm part notes were always played as key MIDDLEC.
int Partial::getKey() const {
	if (poly == NULL) {
		return -1;
	} else if (ownerPart == 8) {
		// FIXME: Hack, should go away after new pitch stuff is committed (and possibly some TVF changes)
		return MIDDLEC;
	} else {
		return poly->getKey();
	}
}

void Partial::startPartial(const Part *part, Poly *usePoly, const PatchCache *usePatchCache, const MemParams::RhythmTemp *rhythmTemp, Partial *pairPartial) {
	if (usePoly == NULL || usePatchCache == NULL) {
		synth->printDebug("[Partial %d] *** Error: Starting partial for owner %d, usePoly=%s, usePatchCache=%s", debugPartialNum, ownerPart, usePoly == NULL ? "*** NULL ***" : "OK", usePatchCache == NULL ? "*** NULL ***" : "OK");
		return;
	}
	patchCache = usePatchCache;
	poly = usePoly;
	mixType = patchCache->structureMix;
	structurePosition = patchCache->structurePosition;

	Bit8u panSetting = rhythmTemp != NULL ? rhythmTemp->panpot : part->getPatchTemp()->panpot;
	float panVal;
	if (mixType == 3) {
		if (structurePosition == 0) {
			panVal = PAN_NUMERATOR_MASTER[panSetting];
		} else {
			panVal = PAN_NUMERATOR_SLAVE[panSetting];
		}
		// Do a normal mix independent of any pair partial.
		mixType = 0;
		pairPartial = NULL;
	} else {
		panVal = PAN_NUMERATOR_NORMAL[panSetting];
	}

	// FIXME: Sample analysis suggests that the use of panVal is linear, but there are some some quirks that still need to be resolved.
	stereoVolume.leftVol = panVal / 7.0f;
	stereoVolume.rightVol = 1.0f - stereoVolume.leftVol;

	// SEMI-CONFIRMED: From sample analysis:
	// Found that timbres with 3 or 4 partials (i.e. one using two partial pairs) are mixed in two different ways.
	// Either partial pairs are added or subtracted, it depends on how the partial pairs are allocated.
	// It seems that partials are grouped into quarters and if the partial pairs are allocated in different quarters the subtraction happens.
	// Though, this matters little for the majority of timbres, it becomes crucial for timbres which contain several partials that sound very close.
	// In this case that timbre can sound totally different depending of the way it is mixed up.
	// Most easily this effect can be displayed with the help of a special timbre consisting of several identical square wave partials (3 or 4).
	// Say, it is 3-partial timbre. Just play any two notes simultaneously and the polys very probably are mixed differently.
	// Moreover, the partial allocator retains the last partial assignment it did and all the subsequent notes will sound the same as the last released one.
	// The situation is better with 4-partial timbres since then a whole quarter is assigned for each poly. However, if a 3-partial timbre broke the normal
	// whole-quarter assignment or after some partials got aborted, even 4-partial timbres can be found sounding differently.
	// This behaviour is also confirmed with two more special timbres: one with identical sawtooth partials, and one with PCM wave 02.
	// For my personal taste, this behaviour rather enriches the sounding and should be emulated.
	// Also, the current partial allocator model probably needs to be refined.
	if (debugPartialNum & 8) {
		stereoVolume.leftVol = -stereoVolume.leftVol;
		stereoVolume.rightVol = -stereoVolume.rightVol;
	}

	if (patchCache->PCMPartial) {
		pcmNum = patchCache->pcm;
		if (synth->controlROMMap->pcmCount > 128) {
			// CM-32L, etc. support two "banks" of PCMs, selectable by waveform type parameter.
			if (patchCache->waveform > 1) {
				pcmNum += 128;
			}
		}
		pcmWave = &synth->pcmWaves[pcmNum];
	} else {
		pcmWave = NULL;
		wavePos = 0.0f;
		lastFreq = 0.0;
	}

	// CONFIRMED: pulseWidthVal calculation is based on information from Mok
	pulseWidthVal = (poly->getVelocity() - 64) * (patchCache->srcPartial.wg.pulseWidthVeloSensitivity - 7) + Tables::getInstance().pulseWidth100To255[patchCache->srcPartial.wg.pulseWidth];
	if (pulseWidthVal < 0) {
		pulseWidthVal = 0;
	} else if (pulseWidthVal > 255) {
		pulseWidthVal = 255;
	}

	pcmPosition = 0.0f;
	pair = pairPartial;
	alreadyOutputed = false;
	tva->reset(part, patchCache->partialParam, rhythmTemp);
	tvp->reset(part, patchCache->partialParam);
	tvf->reset(patchCache->partialParam, tvp->getBasePitch());
}

float Partial::getPCMSample(unsigned int position) {
	if (position >= pcmWave->len) {
		if (!pcmWave->loop) {
			return 0;
		}
		position = position % pcmWave->len;
	}
	return synth->pcmROMData[pcmWave->addr + position];
}

unsigned long Partial::generateSamples(float *partialBuf, unsigned long length) {
	const Tables &tables = Tables::getInstance();
	if (!isActive() || alreadyOutputed) {
		return 0;
	}
	if (poly == NULL) {
		synth->printDebug("[Partial %d] *** ERROR: poly is NULL at Partial::generateSamples()!", debugPartialNum);
		return 0;
	}

	alreadyOutputed = true;

	// Generate samples

	for (sampleNum = 0; sampleNum < length; sampleNum++) {
		float sample = 0;
		Bit32u ampRampVal = ampRamp.nextValue();
		if (ampRamp.checkInterrupt()) {
			tva->handleInterrupt();
		}
		if (!tva->isPlaying()) {
			deactivate();
			break;
		}

		Bit16u pitch = tvp->nextPitch();

		// SEMI-CONFIRMED: From sample analysis:
		// (1) Tested with a single partial playing PCM wave 77 with pitchCoarse 36 and no keyfollow, velocity follow, etc.
		// This gives results within +/- 2 at the output (before any DAC bitshifting)
		// when sustaining at levels 156 - 255 with no modifiers.
		// (2) Tested with a special square wave partial (internal capture ID tva5) at TVA envelope levels 155-255.
		// This gives deltas between -1 and 0 compared to the real output. Note that this special partial only produces
		// positive amps, so negative still needs to be explored, as well as lower levels.
		//
		// Also still partially unconfirmed is the behaviour when ramping between levels, as well as the timing.

#if MT32EMU_ACCURATE_WG == 1
		float amp = EXP2F((32772 - ampRampVal / 2048) / -2048.0f);
		float freq = EXP2F(pitch / 4096.0f - 16.0f) * 32000.0f;
#else
		static const float ampFactor = EXP2F(32772 / -2048.0f);
		float amp = EXP2I(ampRampVal >> 10) * ampFactor;

		static const float freqFactor = EXP2F(-16.0f) * 32000.0f;
		float freq = EXP2I(pitch) * freqFactor;
#endif

		if (patchCache->PCMPartial) {
			// Render PCM waveform
			int len = pcmWave->len;
			int intPCMPosition = (int)pcmPosition;
			if (intPCMPosition >= len && !pcmWave->loop) {
				// We're now past the end of a non-looping PCM waveform so it's time to die.
				deactivate();
				break;
			}
			Bit32u pcmAddr = pcmWave->addr;
			float positionDelta = freq * 2048.0f / synth->myProp.sampleRate;

			// Linear interpolation
			float firstSample = synth->pcmROMData[pcmAddr + intPCMPosition];
			// We observe that for partial structures with ring modulation the interpolation is not applied to the slave PCM partial.
			// It's assumed that the multiplication circuitry intended to perform the interpolation on the slave PCM partial
			// is borrowed by the ring modulation circuit (or the LA32 chip has a similar lack of resources assigned to each partial pair).
			if (pair == NULL || mixType == 0 || structurePosition == 0) {
				sample = firstSample + (getPCMSample(intPCMPosition + 1) - firstSample) * (pcmPosition - intPCMPosition);
			} else {
				sample = firstSample;
			}

			float newPCMPosition = pcmPosition + positionDelta;
			if (pcmWave->loop) {
				newPCMPosition = fmod(newPCMPosition, (float)pcmWave->len);
			}
			pcmPosition = newPCMPosition;
		} else {
			// Render synthesised waveform
			wavePos *= lastFreq / freq;
			lastFreq = freq;

			Bit32u cutoffModifierRampVal = cutoffModifierRamp.nextValue();
			if (cutoffModifierRamp.checkInterrupt()) {
				tvf->handleInterrupt();
			}
			float cutoffModifier = cutoffModifierRampVal / 262144.0f;

			// res corresponds to a value set in an LA32 register
			Bit8u res = patchCache->srcPartial.tvf.resonance + 1;

			// Using tiny exact table for computation of EXP2F(1.0f - (32 - res) / 4.0f)
			float resAmp = tables.resAmpMax[res];

			// The cutoffModifier may not be supposed to be directly added to the cutoff -
			// it may for example need to be multiplied in some way.
			// The 240 cutoffVal limit was determined via sample analysis (internal Munt capture IDs: glop3, glop4).
			// More research is needed to be sure that this is correct, however.
			float cutoffVal = tvf->getBaseCutoff() + cutoffModifier;
			if (cutoffVal > 240.0f) {
				cutoffVal = 240.0f;
			}

			// Wave length in samples
			float waveLen = synth->myProp.sampleRate / freq;

			// Init cosineLen
			float cosineLen = 0.5f * waveLen;
			if (cutoffVal > 128.0f) {
#if MT32EMU_ACCURATE_WG == 1
				cosineLen *= EXP2F((cutoffVal - 128.0f) / -16.0f); // found from sample analysis
#else
				static const float cosineLenFactor = EXP2F(128.0f / -16.0f);
				cosineLen *= EXP2I(Bit32u((256.0f - cutoffVal) * 256.0f)) * cosineLenFactor;
#endif
			}

			// Start playing in center of first cosine segment
			// relWavePos is shifted by a half of cosineLen
			float relWavePos = wavePos + 0.5f * cosineLen;
			if (relWavePos > waveLen) {
				relWavePos -= waveLen;
			}

			float pulseLen = 0.5f;
			if (pulseWidthVal > 128) {
				pulseLen += tables.pulseLenFactor[pulseWidthVal - 128];
			}
			pulseLen *= waveLen;

			float lLen = pulseLen - cosineLen;

			// Ignore pulsewidths too high for given freq
			if (lLen < 0.0f) {
				lLen = 0.0f;
			}

			// Ignore pulsewidths too high for given freq and cutoff
			float hLen = waveLen - lLen - 2 * cosineLen;
			if (hLen < 0.0f) {
				hLen = 0.0f;
			}

			// Correct resAmp for cutoff in range 50..66
			if ((cutoffVal >= 128.0f) && (cutoffVal < 144.0f)) {
#if MT32EMU_ACCURATE_WG == 1
				resAmp *= sinf(FLOAT_PI * (cutoffVal - 128.0f) / 32.0f);
#else
				resAmp *= tables.sinf10[Bit32u(64 * (cutoffVal - 128.0f))];
#endif
			}

			// Produce filtered square wave with 2 cosine waves on slopes

			// 1st cosine segment
			if (relWavePos < cosineLen) {
#if MT32EMU_ACCURATE_WG == 1
				sample = -cosf(FLOAT_PI * relWavePos / cosineLen);
#else
				sample = -tables.sinf10[Bit32u(2048.0f * relWavePos / cosineLen) + 1024];
#endif
			} else

			// high linear segment
			if (relWavePos < (cosineLen + hLen)) {
				sample = 1.f;
			} else

			// 2nd cosine segment
			if (relWavePos < (2 * cosineLen + hLen)) {
#if MT32EMU_ACCURATE_WG == 1
				sample = cosf(FLOAT_PI * (relWavePos - (cosineLen + hLen)) / cosineLen);
#else
				sample = tables.sinf10[Bit32u(2048.0f * (relWavePos - (cosineLen + hLen)) / cosineLen) + 1024];
#endif
			} else {

			// low linear segment
				sample = -1.f;
			}

			if (cutoffVal < 128.0f) {

				// Attenuate samples below cutoff 50
				// Found by sample analysis
#if MT32EMU_ACCURATE_WG == 1
				sample *= EXP2F(-0.125f * (128.0f - cutoffVal));
#else
				static const float cutoffAttenuationFactor = EXP2F(-0.125f * 128.0f);
				sample *= EXP2I(Bit32u(512.0f * cutoffVal)) * cutoffAttenuationFactor;
#endif
			} else {

				// Add resonance sine. Effective for cutoff > 50 only
				float resSample = 1.0f;

				// Now relWavePos counts from the middle of first cosine
				relWavePos = wavePos;

				// negative segments
				if (!(relWavePos < (cosineLen + hLen))) {
					resSample = -resSample;
					relWavePos -= cosineLen + hLen;
				}

				// Resonance sine WG
#if MT32EMU_ACCURATE_WG == 1
				resSample *= sinf(FLOAT_PI * relWavePos / cosineLen);
#else
				resSample *= tables.sinf10[Bit32u(2048.0f * relWavePos / cosineLen) & 4095];
#endif

				// Resonance sine amp
				float resAmpFadeLog2 = -tables.resAmpFadeFactor[res >> 2] * (relWavePos / cosineLen); // seems to be exact
#if MT32EMU_ACCURATE_WG == 1
				float resAmpFade = EXP2F(resAmpFadeLog2);
#else
				static const float resAmpFadeFactor = EXP2F(-30.0f);
				float resAmpFade = (resAmpFadeLog2 < -30.0f) ? 0.0f : EXP2I(Bit32u((30.0f + resAmpFadeLog2) * 4096.0f)) * resAmpFadeFactor;
#endif

				// Now relWavePos set negative to the left from center of any cosine
				relWavePos = wavePos;

				// negative segment
				if (!(wavePos < (waveLen - 0.5f * cosineLen))) {
					relWavePos -= waveLen;
				} else

				// positive segment
				if (!(wavePos < (hLen + 0.5f * cosineLen))) {
					relWavePos -= cosineLen + hLen;
				}

				// Fading to zero while within cosine segments to avoid jumps in the wave
				// Sample analysis suggests that this window is very close to cosine
				if (relWavePos < 0.5f * cosineLen) {
#if MT32EMU_ACCURATE_WG == 1
					resAmpFade *= 0.5f * (1.0f - cosf(FLOAT_PI * relWavePos / (0.5f * cosineLen)));
#else
					resAmpFade *= 0.5f * (1.0f + tables.sinf10[Bit32s(2048.0f * relWavePos / (0.5f * cosineLen)) + 3072]);
#endif
				}

				sample += resSample * resAmp * resAmpFade;
			}

			// sawtooth waves
			if ((patchCache->waveform & 1) != 0) {
#if MT32EMU_ACCURATE_WG == 1
				sample *= cosf(FLOAT_2PI * wavePos / waveLen);
#else
				sample *= tables.sinf10[(Bit32u(4096.0f * wavePos / waveLen) & 4095) + 1024];
#endif
			}

			wavePos++;

			// wavePos isn't supposed to be > waveLen
			if (wavePos > waveLen) {
				wavePos -= waveLen;
			}
		}

		// Multiply sample with current TVA value
		sample *= amp;
		*partialBuf++ = sample;
	}
	unsigned long renderedSamples = sampleNum;
	sampleNum = 0;
	return renderedSamples;
}

float *Partial::mixBuffersRingMix(float *buf1, float *buf2, unsigned long len) {
	if (buf1 == NULL) {
		return NULL;
	}
	if (buf2 == NULL) {
		return buf1;
	}

	while (len--) {
		// FIXME: At this point we have no idea whether this is remotely correct...
		*buf1 = *buf1 * *buf2 + *buf1;
		buf1++;
		buf2++;
	}
	return buf1;
}

float *Partial::mixBuffersRing(float *buf1, float *buf2, unsigned long len) {
	if (buf1 == NULL) {
		return NULL;
	}
	if (buf2 == NULL) {
		return NULL;
	}

	while (len--) {
		// FIXME: At this point we have no idea whether this is remotely correct...
		*buf1 = *buf1 * *buf2;
		buf1++;
		buf2++;
	}
	return buf1;
}

bool Partial::hasRingModulatingSlave() const {
	return pair != NULL && structurePosition == 0 && (mixType == 1 || mixType == 2);
}

bool Partial::isRingModulatingSlave() const {
	return pair != NULL && structurePosition == 1 && (mixType == 1 || mixType == 2);
}

bool Partial::isPCM() const {
	return pcmWave != NULL;
}

const ControlROMPCMStruct *Partial::getControlROMPCMStruct() const {
	if (pcmWave != NULL) {
		return pcmWave->controlROMPCMStruct;
	}
	return NULL;
}

Synth *Partial::getSynth() const {
	return synth;
}

bool Partial::produceOutput(float *leftBuf, float *rightBuf, unsigned long length) {
	if (!isActive() || alreadyOutputed || isRingModulatingSlave()) {
		return false;
	}
	if (poly == NULL) {
		synth->printDebug("[Partial %d] *** ERROR: poly is NULL at Partial::produceOutput()!", debugPartialNum);
		return false;
	}

	float *partialBuf = &myBuffer[0];
	unsigned long numGenerated = generateSamples(partialBuf, length);
	if (mixType == 1 || mixType == 2) {
		float *pairBuf;
		unsigned long pairNumGenerated;
		if (pair == NULL) {
			pairBuf = NULL;
			pairNumGenerated = 0;
		} else {
			pairBuf = &pair->myBuffer[0];
			pairNumGenerated = pair->generateSamples(pairBuf, numGenerated);
			// pair will have been set to NULL if it deactivated within generateSamples()
			if (pair != NULL) {
				if (!isActive()) {
					pair->deactivate();
					pair = NULL;
				} else if (!pair->isActive()) {
					pair = NULL;
				}
			}
		}
		if (pairNumGenerated > 0) {
			if (mixType == 1) {
				mixBuffersRingMix(partialBuf, pairBuf, pairNumGenerated);
			} else {
				mixBuffersRing(partialBuf, pairBuf, pairNumGenerated);
			}
		}
		if (numGenerated > pairNumGenerated) {
			if (mixType == 2) {
				numGenerated = pairNumGenerated;
				deactivate();
			}
		}
	}

	for (unsigned int i = 0; i < numGenerated; i++) {
		*leftBuf++ = partialBuf[i] * stereoVolume.leftVol;
	}
	for (unsigned int i = 0; i < numGenerated; i++) {
		*rightBuf++ = partialBuf[i] * stereoVolume.rightVol;
	}
	while (numGenerated < length) {
		*leftBuf++ = 0.0f;
		*rightBuf++ = 0.0f;
		numGenerated++;
	}
	return true;
}

bool Partial::shouldReverb() {
	if (!isActive()) {
		return false;
	}
	return patchCache->reverb;
}

void Partial::startAbort() {
	// This is called when the partial manager needs to terminate partials for re-use by a new Poly.
	tva->startAbort();
}

void Partial::startDecayAll() {
	tva->startDecay();
	tvp->startDecay();
	tvf->startDecay();
}

}
