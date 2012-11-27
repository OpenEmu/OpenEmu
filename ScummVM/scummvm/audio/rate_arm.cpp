/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * The code in this file, together with the rate_arm_asm.s file offers
 * an ARM optimised version of the code in rate.cpp. The operation of this
 * code should be identical to that of rate.cpp, but faster. The heavy
 * lifting is done in the assembler file.
 *
 * To be as portable as possible we implement the core routines with C
 * linkage in assembly, and implement the C++ routines that call into
 * the C here. The C++ symbol mangling varies wildly between compilers,
 * so this is the simplest way to ensure that the C/C++ combination should
 * work on as many ARM based platforms as possible.
 *
 * Essentially the algorithm herein is the same as that in rate.cpp, so
 * anyone seeking to understand this should attempt to understand that
 * first. That code was based in turn on code with Copyright 1998 Fabrice
 * Bellard - part of SoX (http://sox.sourceforge.net).
 * Max Horn adapted that code to the needs of ScummVM and partially rewrote
 * it, in the process removing any use of floating point arithmetic. Various
 * other improvments over the original code were made.
 */

#include "audio/audiostream.h"
#include "audio/rate.h"
#include "audio/mixer.h"
#include "common/util.h"
#include "common/textconsole.h"

//#define DEBUG_RATECONV

namespace Audio {

/**
 * The precision of the fractional computations used by the rate converter.
 * Normally you should never have to modify this value.
 * This stuff is defined in common/frac.h, but we redefine it here as the
 * ARM routine we call doesn't respect those definitions.
 */
#define FRAC_BITS 16
#define FRAC_ONE  (1 << FRAC_BITS)

/**
 * The size of the intermediate input cache. Bigger values may increase
 * performance, but only until some point (depends largely on cache size,
 * target processor and various other factors), at which it will decrease
 * again.
 */
#define INTERMEDIATE_BUFFER_SIZE 512


/**
 * Audio rate converter based on simple resampling. Used when no
 * interpolation is required.
 *
 * Limited to sampling frequency <= 65535 Hz.
 */
typedef struct {
	const st_sample_t *inPtr;
	int inLen;

	/** position of how far output is ahead of input */
	/** Holds what would have been opos-ipos */
	long opos;

	/** fractional position increment in the output stream */
	long opos_inc;

	st_sample_t inBuf[INTERMEDIATE_BUFFER_SIZE];
} SimpleRateDetails;

template<bool stereo, bool reverseStereo>
class SimpleRateConverter : public RateConverter {
protected:
	SimpleRateDetails  sr;
public:
	SimpleRateConverter(st_rate_t inrate, st_rate_t outrate);
	int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r);
	int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
		return (ST_SUCCESS);
	}
};


/*
 * Prepare processing.
 */
template<bool stereo, bool reverseStereo>
SimpleRateConverter<stereo, reverseStereo>::SimpleRateConverter(st_rate_t inrate, st_rate_t outrate) {
	if (inrate == outrate) {
		error("Input and Output rates must be different to use rate effect");
	}

	if ((inrate % outrate) != 0) {
		error("Input rate must be a multiple of Output rate to use rate effect");
	}

	if (inrate >= 65536 || outrate >= 65536) {
		error("rate effect can only handle rates < 65536");
	}

	sr.opos = 1;

	/* increment */
	sr.opos_inc = inrate / outrate;

	sr.inLen = 0;
}

#ifndef IPHONE
#define ARM_SimpleRate_M _ARM_SimpleRate_M
#define ARM_SimpleRate_S _ARM_SimpleRate_S
#define ARM_SimpleRate_R _ARM_SimpleRate_R
#endif

extern "C" st_sample_t *ARM_SimpleRate_M(
								AudioStream &input,
								int (*fn)(Audio::AudioStream&,int16*,int),
								SimpleRateDetails *sr,
								st_sample_t *obuf,
								st_size_t osamp,
								st_volume_t vol_l,
								st_volume_t vol_r);

extern "C" st_sample_t *ARM_SimpleRate_S(
								AudioStream &input,
								int (*fn)(Audio::AudioStream&,int16*,int),
								SimpleRateDetails *sr,
								st_sample_t *obuf,
								st_size_t osamp,
								st_volume_t vol_l,
								st_volume_t vol_r);

extern "C" st_sample_t *ARM_SimpleRate_R(
								AudioStream &input,
								int (*fn)(Audio::AudioStream&,int16*,int),
								SimpleRateDetails *sr,
								st_sample_t *obuf,
								st_size_t osamp,
								st_volume_t vol_l,
								st_volume_t vol_r);

extern "C" int SimpleRate_readFudge(Audio::AudioStream &input, int16 *a, int b)
{
#ifdef DEBUG_RATECONV
	debug("Reading ptr=%x n%d", a, b);
#endif
	return input.readBuffer(a, b);
}

template<bool stereo, bool reverseStereo>
int SimpleRateConverter<stereo, reverseStereo>::flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {

#ifdef DEBUG_RATECONV
	debug("Simple st=%d rev=%d", stereo, reverseStereo);
#endif
	st_sample_t *ostart = obuf;

	if (!stereo) {
		obuf = ARM_SimpleRate_M(input,
								&SimpleRate_readFudge,
								&sr,
								obuf, osamp, vol_l, vol_r);
	} else if (reverseStereo) {
		obuf = ARM_SimpleRate_R(input,
								&SimpleRate_readFudge,
								&sr,
								obuf, osamp, vol_l, vol_r);
	} else {
		obuf = ARM_SimpleRate_S(input,
								&SimpleRate_readFudge,
								&sr,
								obuf, osamp, vol_l, vol_r);
	}

	return (obuf - ostart) / 2;
}

/**
 * Audio rate converter based on simple linear Interpolation.
 *
 * The use of fractional increment allows us to use no buffer. It
 * avoid the problems at the end of the buffer we had with the old
 * method which stored a possibly big buffer of size
 * lcm(in_rate,out_rate).
 *
 * Limited to sampling frequency <= 65535 Hz.
 */

typedef struct {
	const st_sample_t *inPtr;
	int inLen;

	/** position of how far output is ahead of input */
	/** Holds what would have been opos-ipos<<16 + opos_frac */
	long opos;

	/** integer position increment in the output stream */
	long opos_inc;

	/** current sample(s) in the input stream (left/right channel) */
	st_sample_t icur[2];
	/** last sample(s) in the input stream (left/right channel) */
	/** Note, these are deliberately ints, not st_sample_t's */
	int32 ilast[2];

	st_sample_t inBuf[INTERMEDIATE_BUFFER_SIZE];
} LinearRateDetails;

extern "C" {
#ifndef IPHONE
#define ARM_LinearRate_M _ARM_LinearRate_M
#define ARM_LinearRate_S _ARM_LinearRate_S
#define ARM_LinearRate_R _ARM_LinearRate_R
#endif
}

extern "C" st_sample_t *ARM_LinearRate_M(
								AudioStream &input,
								int (*fn)(Audio::AudioStream&,int16*,int),
								LinearRateDetails *lr,
								st_sample_t *obuf,
								st_size_t osamp,
								st_volume_t vol_l,
								st_volume_t vol_r);

extern "C" st_sample_t *ARM_LinearRate_S(
								AudioStream &input,
								int (*fn)(Audio::AudioStream&,int16*,int),
								LinearRateDetails *lr,
								st_sample_t *obuf,
								st_size_t osamp,
								st_volume_t vol_l,
								st_volume_t vol_r);

extern "C" st_sample_t *ARM_LinearRate_R(
								AudioStream &input,
								int (*fn)(Audio::AudioStream&,int16*,int),
								LinearRateDetails *lr,
								st_sample_t *obuf,
								st_size_t osamp,
								st_volume_t vol_l,
								st_volume_t vol_r);

template<bool stereo, bool reverseStereo>
class LinearRateConverter : public RateConverter {
protected:
	LinearRateDetails lr;

public:
	LinearRateConverter(st_rate_t inrate, st_rate_t outrate);
	int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r);
	int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
		return (ST_SUCCESS);
	}
};


/*
 * Prepare processing.
 */
template<bool stereo, bool reverseStereo>
LinearRateConverter<stereo, reverseStereo>::LinearRateConverter(st_rate_t inrate, st_rate_t outrate) {
	unsigned long incr;

	if (inrate == outrate) {
		error("Input and Output rates must be different to use rate effect");
	}

	if (inrate >= 65536 || outrate >= 65536) {
		error("rate effect can only handle rates < 65536");
	}

	lr.opos = FRAC_ONE;

	/* increment */
	incr = (inrate << FRAC_BITS) / outrate;

	lr.opos_inc = incr;

	lr.ilast[0] = lr.ilast[1] = 32768;
	lr.icur[0] = lr.icur[1] = 0;

	lr.inLen = 0;
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of sample pairs processed.
 */
template<bool stereo, bool reverseStereo>
int LinearRateConverter<stereo, reverseStereo>::flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {

#ifdef DEBUG_RATECONV
	debug("Linear st=%d rev=%d", stereo, reverseStereo);
#endif
	st_sample_t *ostart = obuf;

	if (vol_l > 0xff)
		vol_l = 0xff;

	if (vol_r > 0xff)
		vol_r = 0xff;

	if (!stereo) {
		obuf = ARM_LinearRate_M(input,
								&SimpleRate_readFudge,
								&lr,
								obuf, osamp, vol_l, vol_r);
	} else if (reverseStereo) {
		obuf = ARM_LinearRate_R(input,
								&SimpleRate_readFudge,
								&lr,
								obuf, osamp, vol_l, vol_r);
	} else {
		obuf = ARM_LinearRate_S(input,
								&SimpleRate_readFudge,
								&lr,
								obuf, osamp, vol_l, vol_r);
	}
	return (obuf - ostart) / 2;
}


#pragma mark -


/**
 * Simple audio rate converter for the case that the inrate equals the outrate.
 */
extern "C" {
#ifndef IPHONE
#define ARM_CopyRate_M _ARM_CopyRate_M
#define ARM_CopyRate_S _ARM_CopyRate_S
#define ARM_CopyRate_R _ARM_CopyRate_R
#endif
}

extern "C" st_sample_t *ARM_CopyRate_M(
								st_size_t len,
								st_sample_t *obuf,
								st_volume_t vol_l,
								st_volume_t vol_r,
								st_sample_t *_buffer);

extern "C" st_sample_t *ARM_CopyRate_S(
								st_size_t len,
								st_sample_t *obuf,
								st_volume_t vol_l,
								st_volume_t vol_r,
								st_sample_t *_buffer);

extern "C" st_sample_t *ARM_CopyRate_R(
								st_size_t len,
								st_sample_t *obuf,
								st_volume_t vol_l,
								st_volume_t vol_r,
								st_sample_t *_buffer);


template<bool stereo, bool reverseStereo>
class CopyRateConverter : public RateConverter {
	st_sample_t *_buffer;
	st_size_t _bufferSize;

public:
	CopyRateConverter() : _buffer(0), _bufferSize(0) {}
	~CopyRateConverter() {
		free(_buffer);
	}

	virtual int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
		assert(input.isStereo() == stereo);

#ifdef DEBUG_RATECONV
		debug("Copy st=%d rev=%d", stereo, reverseStereo);
#endif
		st_size_t len;
		st_sample_t *ostart = obuf;

		if (stereo)
			osamp *= 2;

		// Reallocate temp buffer, if necessary
		if (osamp > _bufferSize) {
			free(_buffer);
			_buffer = (st_sample_t *)malloc(osamp * 2);
			_bufferSize = osamp;
		}

		// Read up to 'osamp' samples into our temporary buffer
		len = input.readBuffer(_buffer, osamp);
		if (len <= 0)
			return 0;

		// Mix the data into the output buffer
		if (stereo && reverseStereo)
			obuf = ARM_CopyRate_R(len, obuf, vol_l, vol_r, _buffer);
		else if (stereo)
			obuf = ARM_CopyRate_S(len, obuf, vol_l, vol_r, _buffer);
		else
			obuf = ARM_CopyRate_M(len, obuf, vol_l, vol_r, _buffer);

		return (obuf - ostart) / 2;
	}

	virtual int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
		return (ST_SUCCESS);
	}
};


#pragma mark -


/**
 * Create and return a RateConverter object for the specified input and output rates.
 */
RateConverter *makeRateConverter(st_rate_t inrate, st_rate_t outrate, bool stereo, bool reverseStereo) {
	if (inrate != outrate) {
		if ((inrate % outrate) == 0) {
			if (stereo) {
				if (reverseStereo)
					return new SimpleRateConverter<true, true>(inrate, outrate);
				else
					return new SimpleRateConverter<true, false>(inrate, outrate);
			} else
				return new SimpleRateConverter<false, false>(inrate, outrate);
		} else {
			if (stereo) {
				if (reverseStereo)
					return new LinearRateConverter<true, true>(inrate, outrate);
				else
					return new LinearRateConverter<true, false>(inrate, outrate);
			} else
				return new LinearRateConverter<false, false>(inrate, outrate);
		 }
	} else {
		if (stereo) {
			if (reverseStereo)
				return new CopyRateConverter<true, true>();
			else
				return new CopyRateConverter<true, false>();
		} else
			return new CopyRateConverter<false, false>();
	}
}

} // End of namespace Audio
