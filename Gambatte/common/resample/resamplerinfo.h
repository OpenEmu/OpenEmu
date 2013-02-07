/***************************************************************************
 *   Copyright (C) 2008 by Sindre Aamås                                    *
 *   sinamas@users.sourceforge.net                                         *
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
#ifndef RESAMPLER_INFO_H
#define RESAMPLER_INFO_H

#include "resampler.h"

/** Used for creating instances of resamplers, and getting information on available resamplers.
  * Currently creates resamplers that expect stereo samples. All 'numbers of samples' are in
  * number of stereo samples. (This can be changed by adjusting the 'channels' enum in src/chainresampler.h
  * to the number of desired channels.).
  */
struct ResamplerInfo {
	/** Short character string description of the resampler. */
	const char *desc;
	
	/** Points to a function that can be used to create an instance of the resampler.
	  * @param inRate The input sampling rate.
	  * @param outRate The desired output sampling rate.
	  * @param periodSz The maximum number of input samples to resample at a time. That is the maximal inlen passed to Resampler::resample.
	  * @return Pointer to the created instance (on the heap). Caller must free this with the delete operator.
	  */
	Resampler* (*create)(long inRate, long outRate, std::size_t periodSz);
	
	/** Returns the number of ResamplerInfos that can be gotten with get(). */
	static std::size_t num() { return num_; }
	
	/** Returns ResamplerInfo number n. Where n is less than num(). */
	static const ResamplerInfo& get(std::size_t n) { return resamplers[n]; }
	
private:
	static const ResamplerInfo resamplers[];
	static const std::size_t num_;
};

#endif
