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

class Resampler {
	long inRate_;
	long outRate_;
	
protected:
	void setRate(const long inRate, const long outRate) { inRate_ = inRate; outRate_ = outRate; }
	
public:
	Resampler() : inRate_(0), outRate_(0) {}
	long inRate() const { return inRate_; }
	long outRate() const { return outRate_; }
	
	virtual void adjustRate(long inRate, long outRate) = 0;
	virtual void exactRatio(unsigned long &mul, unsigned long &div) const = 0;
	virtual std::size_t maxOut(std::size_t inlen) const = 0;
	virtual std::size_t resample(short *out, const short *in, std::size_t inlen) = 0;
	virtual ~Resampler() {}
};

#endif
