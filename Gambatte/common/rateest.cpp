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
#include "rateest.h"
#include <cstdlib>

void RateEst::SumQueue::reset() {
	q.clear();
	samples_ = usecs_ = 0;
}

void RateEst::SumQueue::push(const long samples, const usec_t usecs) {
	q.push_back(pair_t(samples, usecs));
	samples_ += samples;
	usecs_ += usecs;
}

void RateEst::SumQueue::pop() {
	const pair_t &f = q.front();
	samples_ -= f.first;
	usecs_ -= f.second;
	q.pop_front();
}

static usec_t sampleUsecs(long samples, long rate) {
	return static_cast<usec_t>((samples * 1000000.0f) / (rate ? rate : 1) + 0.5f);
}

static long limit(long est, const long reference) {
	if (est > reference + (reference >> 6))
		est = reference + (reference >> 6);
	else if (est < reference - (reference >> 6))
		est = reference - (reference >> 6);

	return est;
}

void RateEst::init(long srate, long reference, const long maxSamplePeriod) {
	maxPeriod = sampleUsecs(maxSamplePeriod, reference);

	srate <<= UPSHIFT;
	reference <<= UPSHIFT;

	this->srate.est = limit(srate, reference);
	this->srate.var = srate >> 12;
	last = 0;
	this->reference = reference;
	samples = ((this->srate.est >> UPSHIFT) * 12) << 5;
	usecs = 12000000 << 5;
	sumq.reset();
}

void RateEst::feed(long samplesIn, const usec_t now) {
	usec_t usecsIn = now - last;

	if (last && usecsIn < maxPeriod) {
		sumq.push(samplesIn, usecsIn);

		while ((usecsIn = sumq.usecs()) > 100000) {
			samplesIn = sumq.samples();
			sumq.pop();

			if (std::abs(static_cast<long>(samplesIn * (1000000.0f * UP) / usecsIn) - reference) < reference >> 1) {
				samples += (samplesIn - sumq.samples()) << 5;
				usecs += (usecsIn - sumq.usecs()) << 5;

				long est = static_cast<long>(samples * (1000000.0f * UP) / usecs + 0.5f);
				est = limit((srate.est * 31 + est + 16) >> 5, reference);
				srate.var = (srate.var * 15 + std::abs(est - srate.est) + 8) >> 4;
				srate.est = est;

				if (usecs > 16000000 << 5) {
					samples = (samples * 3 + 2) >> 2;
					usecs = (usecs * 3 + 2) >> 2;
				}
			}
		}
	}

	last = now;
}
