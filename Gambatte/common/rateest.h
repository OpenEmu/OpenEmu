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
#ifndef RATEEST_H
#define RATEEST_H

#include "usec.h"
#include <deque>
#include <utility>

class RateEst {
public:
	struct Result {
		long est;
		long var;
	};

private:
	class SumQueue {
		typedef std::pair<long, usec_t> pair_t;
		typedef std::deque<pair_t> q_t;

		q_t q;
		long samples_;
		usec_t usecs_;

	public:
		SumQueue() : samples_(0), usecs_(0) {}
		void reset();
		long samples() const { return samples_; }
		usec_t usecs() const { return usecs_; }
		void push(long samples, usec_t usecs);
		void pop();
	};

	enum { UPSHIFT = 5 };
	enum { UP = 1 << UPSHIFT };

	Result srate;
	SumQueue sumq;
	usec_t last;
	usec_t usecs;
	usec_t maxPeriod;
	long reference;
	long samples;

public:
	RateEst(long srate = 0) { init(srate); }
	RateEst(long srate, long reference) { init(srate, reference); }
	void init(long srate) { init(srate, srate); }
	void init(long srate, long reference) { init(srate, reference, reference); }
	void init(long srate, long reference, long maxSamplePeriod);
	void reset() { last = 0; }
	void feed(long samples, usec_t usecs = getusecs());
	const Result result() const { const Result res = { (srate.est + UP / 2) >> UPSHIFT, (srate.var + UP / 2) >> UPSHIFT }; return res; }
};

#endif
