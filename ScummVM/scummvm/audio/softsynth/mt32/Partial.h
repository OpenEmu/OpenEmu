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

#ifndef MT32EMU_PARTIAL_H
#define MT32EMU_PARTIAL_H

namespace MT32Emu {

class Synth;
class Part;
class TVA;
struct ControlROMPCMStruct;

struct StereoVolume {
	float leftVol;
	float rightVol;
};

// A partial represents one of up to four waveform generators currently playing within a poly.
class Partial {
private:
	Synth *synth;
	const int debugPartialNum; // Only used for debugging
	// Number of the sample currently being rendered by generateSamples(), or 0 if no run is in progress
	// This is only kept available for debugging purposes.
	unsigned long sampleNum;

	int ownerPart; // -1 if unassigned
	int mixType;
	int structurePosition; // 0 or 1 of a structure pair
	StereoVolume stereoVolume;

	// Distance in (possibly fractional) samples from the start of the current pulse
	float wavePos;

	float lastFreq;

	float myBuffer[MAX_SAMPLES_PER_RUN];

	// Only used for PCM partials
	int pcmNum;
	// FIXME: Give this a better name (e.g. pcmWaveInfo)
	PCMWaveEntry *pcmWave;

	// Final pulse width value, with velfollow applied, matching what is sent to the LA32.
	// Range: 0-255
	int pulseWidthVal;

	float pcmPosition;

	Poly *poly;

	LA32Ramp ampRamp;
	LA32Ramp cutoffModifierRamp;

	float *mixBuffersRingMix(float *buf1, float *buf2, unsigned long len);
	float *mixBuffersRing(float *buf1, float *buf2, unsigned long len);

	float getPCMSample(unsigned int position);

public:
	const PatchCache *patchCache;
	TVA *tva;
	TVP *tvp;
	TVF *tvf;

	PatchCache cachebackup;

	Partial *pair;
	bool alreadyOutputed;

	Partial(Synth *synth, int debugPartialNum);
	~Partial();

	int debugGetPartialNum() const;
	unsigned long debugGetSampleNum() const;

	int getOwnerPart() const;
	int getKey() const;
	const Poly *getPoly() const;
	bool isActive() const;
	void activate(int part);
	void deactivate(void);
	void startPartial(const Part *part, Poly *usePoly, const PatchCache *useCache, const MemParams::RhythmTemp *rhythmTemp, Partial *pairPartial);
	void startAbort();
	void startDecayAll();
	bool shouldReverb();
	bool hasRingModulatingSlave() const;
	bool isRingModulatingSlave() const;
	bool isPCM() const;
	const ControlROMPCMStruct *getControlROMPCMStruct() const;
	Synth *getSynth() const;

	// Returns true only if data written to buffer
	// This function (unlike the one below it) returns processed stereo samples
	// made from combining this single partial with its pair, if it has one.
	bool produceOutput(float *leftBuf, float *rightBuf, unsigned long length);

	// This function writes mono sample output to the provided buffer, and returns the number of samples written
	unsigned long generateSamples(float *partialBuf, unsigned long length);
};

}

#endif
