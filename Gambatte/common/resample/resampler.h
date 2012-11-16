/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aamås                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef RESAMPLER_H
#define RESAMPLER_H

#include <cstddef>

/** Interface to a Resampler. */
class Resampler {
	long inRate_;
	long outRate_;
	
protected:
	void setRate(const long inRate, const long outRate) { inRate_ = inRate; outRate_ = outRate; }
	Resampler() : inRate_(0), outRate_(0) {}
	
public:
	/** Returns the sampling rate of the input that this resampler expects. */
	long inRate() const { return inRate_; }
	
	/** Returns the approximate sampling rate of the output. */
	long outRate() const { return outRate_; }
	
	/** Can be used to adjust the input and output sampling rates slightly with minimal disturbance in the output.
	  * Should only be used for slight changes or the quality could detoriate.
	  * It can for instance be useful to tweak the output rate slightly to synchronize production speed to playback
	  * speed when synchronizing video frame rate to refresh rate while playing back audio from the same source.
	  * This can reduce skipped or duplicated video frames (or avoid audio underruns if no frame skipping is done).
	  *
	  * @param inRate New input sampling rate.
	  * @param outRate Desired new output sampling rate.
	  */
	virtual void adjustRate(long inRate, long outRate) = 0;
	
	/** Returns the exact ratio that this resampler is configured to use,
	  * such that the actual output sampling rate is (input rate) * mul / div.
	  * outRate() / inRate() is not necessarily equal to mul / div.
	  * Many resampler are intended for real-time purposes where it does not matter
	  * much whether the output sampling rate is 100% exact. Playback hardware is also slightly off.
	  */
	virtual void exactRatio(unsigned long &mul, unsigned long &div) const = 0;
	
	/** Returns an upper bound on how many samples are produced for 'inlen' input samples.
	  * Can be used to calculate buffer sizes.
	  */
	virtual std::size_t maxOut(std::size_t inlen) const = 0;
	
	/** Resamples the samples in 'in' and puts the resulting samples in 'out'.
	  *
	  * @param inlen The number of samples in 'in' to be resampled/consumed.
	  * @return The number of samples produced in 'out'.
	  */
	virtual std::size_t resample(short *out, const short *in, std::size_t inlen) = 0;
	virtual ~Resampler() {}
};

#endif
