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

#include "mt32emu.h"
#include "AReverbModel.h"

namespace MT32Emu {

// Default reverb settings for modes 0-2

static const unsigned int NUM_ALLPASSES = 6;
static const unsigned int NUM_DELAYS = 5;

static const Bit32u MODE_0_ALLPASSES[] = {729, 78, 394, 994, 1250, 1889};
static const Bit32u MODE_0_DELAYS[] = {846, 4, 1819, 778, 346};
static const float MODE_0_TIMES[] = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.9f};
static const float MODE_0_LEVELS[] = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 1.01575f};

static const Bit32u MODE_1_ALLPASSES[] = {176, 809, 1324, 1258};
static const Bit32u MODE_1_DELAYS[] = {2262, 124, 974, 2516, 356};
static const float MODE_1_TIMES[] = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.95f};
static const float MODE_1_LEVELS[] = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 1.01575f};

static const Bit32u MODE_2_ALLPASSES[] = {78, 729, 994, 389};
static const Bit32u MODE_2_DELAYS[] = {846, 4, 1819, 778, 346};
static const float MODE_2_TIMES[] = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f};
static const float MODE_2_LEVELS[] = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f};

const AReverbSettings AReverbModel::REVERB_MODE_0_SETTINGS = {MODE_0_ALLPASSES, MODE_0_DELAYS, MODE_0_TIMES, MODE_0_LEVELS, 0.687770909f, 0.5f, 0.5f};
const AReverbSettings AReverbModel::REVERB_MODE_1_SETTINGS = {MODE_1_ALLPASSES, MODE_1_DELAYS, MODE_1_TIMES, MODE_1_LEVELS, 0.712025098f, 0.375f, 0.625f};
const AReverbSettings AReverbModel::REVERB_MODE_2_SETTINGS = {MODE_2_ALLPASSES, MODE_2_DELAYS, MODE_2_TIMES, MODE_2_LEVELS, 0.939522749f, 0.0f, 0.0f};

RingBuffer::RingBuffer(Bit32u newsize) {
	index = 0;
	size = newsize;
	buffer = new float[size];
}

RingBuffer::~RingBuffer() {
	delete[] buffer;
	buffer = NULL;
	size = 0;
}

float RingBuffer::next() {
	index++;
	if (index >= size) {
		index = 0;
	}
	return buffer[index];
}

bool RingBuffer::isEmpty() {
	if (buffer == NULL) return true;

	float *buf = buffer;
	float total = 0;
	for (Bit32u i = 0; i < size; i++) {
		total += (*buf < 0 ? -*buf : *buf);
		buf++;
	}
	return ((total / size) < .0002 ? true : false);
}

void RingBuffer::mute() {
	float *buf = buffer;
	for (Bit32u i = 0; i < size; i++) {
		*buf++ = 0;
	}
}

AllpassFilter::AllpassFilter(Bit32u useSize) : RingBuffer(useSize) {
}

Delay::Delay(Bit32u useSize) : RingBuffer(useSize) {
}

float AllpassFilter::process(float in) {
	// This model corresponds to the allpass filter implementation in the real CM-32L device
	// found from sample analysis

	float out;

	out = next();

	// store input - feedback / 2
	buffer[index] = in - 0.5f * out;

	// return buffer output + feedforward / 2
	return out + 0.5f * buffer[index];
}

float Delay::process(float in) {
	// Implements a very simple delay

	float out;

	out = next();

	// store input
	buffer[index] = in;

	// return buffer output
	return out;
}

AReverbModel::AReverbModel(const AReverbSettings *useSettings) : allpasses(NULL), delays(NULL), currentSettings(useSettings) {
}

AReverbModel::~AReverbModel() {
	close();
}

void AReverbModel::open(unsigned int /*sampleRate*/) {
	// FIXME: filter sizes must be multiplied by sample rate to 32000Hz ratio
	// IIR filter values depend on sample rate as well
	allpasses = new AllpassFilter*[NUM_ALLPASSES];
	for (Bit32u i = 0; i < NUM_ALLPASSES; i++) {
		allpasses[i] = new AllpassFilter(currentSettings->allpassSizes[i]);
	}
	delays = new Delay*[NUM_DELAYS];
	for (Bit32u i = 0; i < NUM_DELAYS; i++) {
		delays[i] = new Delay(currentSettings->delaySizes[i]);
	}
	mute();
}

void AReverbModel::close() {
	if (allpasses != NULL) {
		for (Bit32u i = 0; i < NUM_ALLPASSES; i++) {
			if (allpasses[i] != NULL) {
				delete allpasses[i];
				allpasses[i] = NULL;
			}
		}
		delete[] allpasses;
		allpasses = NULL;
	}
	if (delays != NULL) {
		for (Bit32u i = 0; i < NUM_DELAYS; i++) {
			if (delays[i] != NULL) {
				delete delays[i];
				delays[i] = NULL;
			}
		}
		delete[] delays;
		delays = NULL;
	}
}

void AReverbModel::mute() {
	for (Bit32u i = 0; i < NUM_ALLPASSES; i++) {
		allpasses[i]->mute();
	}
	for (Bit32u i = 0; i < NUM_DELAYS; i++) {
		delays[i]->mute();
	}
	filterhist1 = 0;
	filterhist2 = 0;
	combhist = 0;
}

void AReverbModel::setParameters(Bit8u time, Bit8u level) {
// FIXME: wetLevel definitely needs ramping when changed
// Although, most games don't set reverb level during MIDI playback
	decayTime = currentSettings->decayTimes[time];
	wetLevel = currentSettings->wetLevels[level];
}

bool AReverbModel::isActive() const {
	bool bActive = false;
	for (Bit32u i = 0; i < NUM_ALLPASSES; i++) {
		bActive |= !allpasses[i]->isEmpty();
	}
	for (Bit32u i = 0; i < NUM_DELAYS; i++) {
		bActive |= !delays[i]->isEmpty();
	}
	return bActive;
}

void AReverbModel::process(const float *inLeft, const float *inRight, float *outLeft, float *outRight, unsigned long numSamples) {
// Three series allpass filters followed by a delay, fourth allpass filter and another delay
	float dry, link, outL1, outL2, outR1, outR2;

	for (unsigned long i = 0; i < numSamples; i++) {
		dry = *inLeft + *inRight;

		// Implementation of 2-stage IIR single-pole low-pass filter
		// found at the entrance of reverb processing on real devices
		filterhist1 += (dry - filterhist1) * currentSettings->filtVal;
		filterhist2 += (filterhist1 - filterhist2) * currentSettings->filtVal;

		link = allpasses[0]->process(-filterhist2);
		link = allpasses[1]->process(link);

		// this implements a comb filter cross-linked with the fourth allpass filter
		link += combhist * decayTime;
		link = allpasses[2]->process(link);
		link = delays[0]->process(link);
		outL1 = link;
		link = allpasses[3]->process(link);
		link = delays[1]->process(link);
		outR1 = link;
		link = allpasses[4]->process(link);
		link = delays[2]->process(link);
		outL2 = link;
		link = allpasses[5]->process(link);
		link = delays[3]->process(link);
		outR2 = link;
		link = delays[4]->process(link);

		// comb filter end point
		combhist = combhist * currentSettings->damp1 + link * currentSettings->damp2;

		*outLeft = (outL1 + outL2) * wetLevel;
		*outRight = (outR1 + outR2) * wetLevel;

		inLeft++;
		inRight++;
		outLeft++;
		outRight++;
	}
}

}
