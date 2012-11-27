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

#ifndef MT32EMU_A_REVERB_MODEL_H
#define MT32EMU_A_REVERB_MODEL_H

namespace MT32Emu {

struct AReverbSettings {
	const Bit32u *allpassSizes;
	const Bit32u *delaySizes;
	const float *decayTimes;
	const float *wetLevels;
	float filtVal;
	float damp1;
	float damp2;
};

class RingBuffer {
protected:
	float *buffer;
	Bit32u size;
	Bit32u index;
public:
	RingBuffer(Bit32u size);
	virtual ~RingBuffer();
	float next();
	bool isEmpty();
	void mute();
};

class AllpassFilter : public RingBuffer {
public:
	AllpassFilter(Bit32u size);
	float process(float in);
};

class Delay : public RingBuffer {
public:
	Delay(Bit32u size);
	float process(float in);
};

class AReverbModel : public ReverbModel {
	AllpassFilter **allpasses;
	Delay **delays;

	const AReverbSettings *currentSettings;
	float decayTime;
	float wetLevel;
	float filterhist1, filterhist2;
	float combhist;
	void mute();
public:
	AReverbModel(const AReverbSettings *newSettings);
	~AReverbModel();
	void open(unsigned int sampleRate);
	void close();
	void setParameters(Bit8u time, Bit8u level);
	void process(const float *inLeft, const float *inRight, float *outLeft, float *outRight, unsigned long numSamples);
	bool isActive() const;

	static const AReverbSettings REVERB_MODE_0_SETTINGS;
	static const AReverbSettings REVERB_MODE_1_SETTINGS;
	static const AReverbSettings REVERB_MODE_2_SETTINGS;
};

// Default reverb settings for modes 0-2

}

#endif
