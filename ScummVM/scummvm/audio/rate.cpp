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
 * The code in this file is based on code with Copyright 1998 Fabrice Bellard
 * Fabrice original code is part of SoX (http://sox.sourceforge.net).
 * Max Horn adapted that code to the needs of ScummVM and rewrote it partial,
 * in the process removing any use of floating point arithmetic. Various other
 * improvements over the original code were made.
 */

#include "audio/audiostream.h"
#include "audio/rate.h"
#include "audio/mixer.h"
#include "common/frac.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Audio {


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
template<bool stereo, bool reverseStereo>
class SimpleRateConverter : public RateConverter {
protected:
	st_sample_t inBuf[INTERMEDIATE_BUFFER_SIZE];
	const st_sample_t *inPtr;
	int inLen;

	/** position of how far output is ahead of input */
	/** Holds what would have been opos-ipos */
	long opos;

	/** fractional position increment in the output stream */
	long opos_inc;

public:
	SimpleRateConverter(st_rate_t inrate, st_rate_t outrate);
	int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r);
	int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
		return ST_SUCCESS;
	}
};


/*
 * Prepare processing.
 */
template<bool stereo, bool reverseStereo>
SimpleRateConverter<stereo, reverseStereo>::SimpleRateConverter(st_rate_t inrate, st_rate_t outrate) {
	if ((inrate % outrate) != 0) {
		error("Input rate must be a multiple of output rate to use rate effect");
	}

	if (inrate >= 65536 || outrate >= 65536) {
		error("rate effect can only handle rates < 65536");
	}

	opos = 1;

	/* increment */
	opos_inc = inrate / outrate;

	inLen = 0;
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of sample pairs processed.
 */
template<bool stereo, bool reverseStereo>
int SimpleRateConverter<stereo, reverseStereo>::flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
	st_sample_t *ostart, *oend;

	ostart = obuf;
	oend = obuf + osamp * 2;

	while (obuf < oend) {

		// read enough input samples so that opos >= 0
		do {
			// Check if we have to refill the buffer
			if (inLen == 0) {
				inPtr = inBuf;
				inLen = input.readBuffer(inBuf, ARRAYSIZE(inBuf));
				if (inLen <= 0)
					return (obuf - ostart) / 2;
			}
			inLen -= (stereo ? 2 : 1);
			opos--;
			if (opos >= 0) {
				inPtr += (stereo ? 2 : 1);
			}
		} while (opos >= 0);

		st_sample_t out0, out1;
		out0 = *inPtr++;
		out1 = (stereo ? *inPtr++ : out0);

		// Increment output position
		opos += opos_inc;

		// output left channel
		clampedAdd(obuf[reverseStereo    ], (out0 * (int)vol_l) / Audio::Mixer::kMaxMixerVolume);

		// output right channel
		clampedAdd(obuf[reverseStereo ^ 1], (out1 * (int)vol_r) / Audio::Mixer::kMaxMixerVolume);

		obuf += 2;
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

template<bool stereo, bool reverseStereo>
class LinearRateConverter : public RateConverter {
protected:
	st_sample_t inBuf[INTERMEDIATE_BUFFER_SIZE];
	const st_sample_t *inPtr;
	int inLen;

	/** fractional position of the output stream in input stream unit */
	frac_t opos;

	/** fractional position increment in the output stream */
	frac_t opos_inc;

	/** last sample(s) in the input stream (left/right channel) */
	st_sample_t ilast0, ilast1;
	/** current sample(s) in the input stream (left/right channel) */
	st_sample_t icur0, icur1;

public:
	LinearRateConverter(st_rate_t inrate, st_rate_t outrate);
	int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r);
	int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
		return ST_SUCCESS;
	}
};


/*
 * Prepare processing.
 */
template<bool stereo, bool reverseStereo>
LinearRateConverter<stereo, reverseStereo>::LinearRateConverter(st_rate_t inrate, st_rate_t outrate) {
	if (inrate >= 65536 || outrate >= 65536) {
		error("rate effect can only handle rates < 65536");
	}

	opos = FRAC_ONE;

	// Compute the linear interpolation increment.
	// This will overflow if inrate >= 2^16, and underflow if outrate >= 2^16.
	// Also, if the quotient of the two rate becomes too small / too big, that
	// would cause problems, but since we rarely scale from 1 to 65536 Hz or vice
	// versa, I think we can live with that limitation ;-).
	opos_inc = (inrate << FRAC_BITS) / outrate;

	ilast0 = ilast1 = 0;
	icur0 = icur1 = 0;

	inLen = 0;
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of sample pairs processed.
 */
template<bool stereo, bool reverseStereo>
int LinearRateConverter<stereo, reverseStereo>::flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
	st_sample_t *ostart, *oend;

	ostart = obuf;
	oend = obuf + osamp * 2;

	while (obuf < oend) {

		// read enough input samples so that opos < 0
		while ((frac_t)FRAC_ONE <= opos) {
			// Check if we have to refill the buffer
			if (inLen == 0) {
				inPtr = inBuf;
				inLen = input.readBuffer(inBuf, ARRAYSIZE(inBuf));
				if (inLen <= 0)
					return (obuf - ostart) / 2;
			}
			inLen -= (stereo ? 2 : 1);
			ilast0 = icur0;
			icur0 = *inPtr++;
			if (stereo) {
				ilast1 = icur1;
				icur1 = *inPtr++;
			}
			opos -= FRAC_ONE;
		}

		// Loop as long as the outpos trails behind, and as long as there is
		// still space in the output buffer.
		while (opos < (frac_t)FRAC_ONE && obuf < oend) {
			// interpolate
			st_sample_t out0, out1;
			out0 = (st_sample_t)(ilast0 + (((icur0 - ilast0) * opos + FRAC_HALF) >> FRAC_BITS));
			out1 = (stereo ?
						  (st_sample_t)(ilast1 + (((icur1 - ilast1) * opos + FRAC_HALF) >> FRAC_BITS)) :
						  out0);

			// output left channel
			clampedAdd(obuf[reverseStereo    ], (out0 * (int)vol_l) / Audio::Mixer::kMaxMixerVolume);

			// output right channel
			clampedAdd(obuf[reverseStereo ^ 1], (out1 * (int)vol_r) / Audio::Mixer::kMaxMixerVolume);

			obuf += 2;

			// Increment output position
			opos += opos_inc;
		}
	}
	return (obuf - ostart) / 2;
}


#pragma mark -


/**
 * Simple audio rate converter for the case that the inrate equals the outrate.
 */
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

		st_sample_t *ptr;
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

		if (!_buffer)
			error("[CopyRateConverter::flow] Cannot allocate memory for temp buffer");

		// Read up to 'osamp' samples into our temporary buffer
		len = input.readBuffer(_buffer, osamp);

		// Mix the data into the output buffer
		ptr = _buffer;
		for (; len > 0; len -= (stereo ? 2 : 1)) {
			st_sample_t out0, out1;
			out0 = *ptr++;
			out1 = (stereo ? *ptr++ : out0);

			// output left channel
			clampedAdd(obuf[reverseStereo    ], (out0 * (int)vol_l) / Audio::Mixer::kMaxMixerVolume);

			// output right channel
			clampedAdd(obuf[reverseStereo ^ 1], (out1 * (int)vol_r) / Audio::Mixer::kMaxMixerVolume);

			obuf += 2;
		}
		return (obuf - ostart) / 2;
	}

	virtual int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
		return ST_SUCCESS;
	}
};


#pragma mark -

template<bool stereo, bool reverseStereo>
RateConverter *makeRateConverter(st_rate_t inrate, st_rate_t outrate) {
	if (inrate != outrate) {
		if ((inrate % outrate) == 0) {
			return new SimpleRateConverter<stereo, reverseStereo>(inrate, outrate);
		} else {
			return new LinearRateConverter<stereo, reverseStereo>(inrate, outrate);
		}
	} else {
		return new CopyRateConverter<stereo, reverseStereo>();
	}
}

/**
 * Create and return a RateConverter object for the specified input and output rates.
 */
RateConverter *makeRateConverter(st_rate_t inrate, st_rate_t outrate, bool stereo, bool reverseStereo) {
	if (stereo) {
		if (reverseStereo)
			return makeRateConverter<true, true>(inrate, outrate);
		else
			return makeRateConverter<true, false>(inrate, outrate);
	} else
		return makeRateConverter<false, false>(inrate, outrate);
}

} // End of namespace Audio
