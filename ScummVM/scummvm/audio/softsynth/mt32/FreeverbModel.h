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

#ifndef MT32EMU_FREEVERB_MODEL_H
#define MT32EMU_FREEVERB_MODEL_H

class revmodel;

namespace MT32Emu {

class FreeverbModel : public ReverbModel {
	revmodel *freeverb;
	float scaleTuning;
	float filtVal;
	float wet;
	Bit8u room;
	float damp;
public:
	FreeverbModel(float useScaleTuning, float useFiltVal, float useWet, Bit8u useRoom, float useDamp);
	~FreeverbModel();
	void open(unsigned int sampleRate);
	void close();
	void setParameters(Bit8u time, Bit8u level);
	void process(const float *inLeft, const float *inRight, float *outLeft, float *outRight, unsigned long numSamples);
	bool isActive() const;
};

}

#endif
