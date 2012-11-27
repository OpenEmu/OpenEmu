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

#ifndef AUDIO_TIMESTAMP_H
#define AUDIO_TIMESTAMP_H

#include "common/scummsys.h"

namespace Audio {

/**
 * Timestamps allow specifying points in time and measuring time intervals
 * with a sub-millisecond granularity.
 *
 * When dealing with audio and video decoding, it is often necessary to
 * measure time (intervals) in terms of frames, relative to a fixed
 * frame rate (that is, a fixed number of frames per seconds). For
 * example, in a typical video there are 24 frames per second, and in a
 * typical sound there are 44100 frames (i.e. samples for mono sound
 * and pairs of samples for stereo) per second.
 *
 * At the same time, the system clock provided by ScummVM measures time
 * in milliseconds. For syncing purposes and other reasons, it is often
 * necessary to convert between and compare time measures given on the
 * one hand as a frame count, and on the other hand as a number of
 * milliseconds.
 *
 * If handled carelessly, this can introduce rounding errors that
 * quickly accumulate, resulting in user noticeable disturbance, such as
 * audio and video running out of sync. E.g. a typical approach is to
 * measure all time in milliseconds. But with a frame rate of 24 frames
 * per second, one frame is 41.66666... milliseconds long. On the other
 * hand, if measuring in frames, then similar rounding issue occur when
 * converting from milliseconds to frames.
 *
 * One solution is to use floating point arithmetic to compute with
 * fractional frames resp. (milli)seconds. This has other undesirable
 * side effects; foremost, some platforms ScummVM runs on still have
 * only limited (and slow) floating point support.
 *
 * This class provides an alternate solution: It stores time in terms of
 * frames, but with a twist: Client code can specify arbitrary
 * (integral) framerates; but internally, Timestamp modifies the
 * framerate to be a multiple of 1000. This way, both numbers of frames
 * (relative to the original framerate) as well as milliseconds can be
 * represented as integers. This change is completely hidden from the
 * user, however.
 *
 * A Timestamp can be converted to a frame count or milliseconds at
 * virtually no cost. Likewise, it is posible to compute the difference
 * between two Timestamps in milliseconds or number of frames.
 * Timestamps can be easily compared using regular comparison operators,
 * resulting in nicely readable code; this is even possible for
 * timestamps that are specified using different framerates.
 * Client code can modify Timestamps by adding a number of frames
 * to it, or adding a number of milliseconds. Adding negative amounts is
 * also allowed, and a Timestamp can even represent a "negative time"
 * (mainly useful when using the Timestamp to store a time interval).
 */
class Timestamp {
public:
	/**
	 * Set up a timestamp with a given time and framerate.
	 * @param msecs     starting time in milliseconds
	 * @param framerate number of frames per second (must be > 0)
	 */
	Timestamp(uint msecs = 0, uint framerate = 1);

	/**
	 * Set up a timestamp with a given time, frames and framerate.
	 * @param secs      starting time in seconds
	 * @param frames    starting frames
	 * @param framerate number of frames per second (must be > 0)
	 */
	Timestamp(uint secs, uint frames, uint framerate);

	/**
	 * Return a timestamp which represents as closely as possible
	 * the point in time describes by this timestamp, but with
	 * a different framerate.
	 */
	Timestamp convertToFramerate(uint newFramerate) const;

	/**
	 * Check whether to timestamps describe the exact same moment
	 * in time. This means that two timestamps can compare
	 * as equal even if they use different framerates.
	 */
	bool operator==(const Timestamp &ts) const;
	bool operator!=(const Timestamp &ts) const;
	bool operator<(const Timestamp &ts) const;
	bool operator<=(const Timestamp &ts) const;
	bool operator>(const Timestamp &ts) const;
	bool operator>=(const Timestamp &ts) const;

	/**
	 * Returns a new timestamp, which corresponds to the time encoded
	 * by this timestamp with the given number of frames added.
	 * @param frames     number of frames to add
	 */
	Timestamp addFrames(int frames) const;

	/**
	 * Returns a new timestamp, which corresponds to the time encoded
	 * by this timestamp with the given number of milliseconds added.
	 * @param msecs     number of milliseconds to add
	 */
	Timestamp addMsecs(int msecs) const;


	// unary minus
	Timestamp operator-() const;

	/**
	 * Compute the sum of two timestamps. This is only
	 * allowed if they use the same framerate.
	 */
	Timestamp operator+(const Timestamp &ts) const;

	/**
	 * Compute the difference between two timestamps. This is only
	 * allowed if they use the same framerate.
	 */
	Timestamp operator-(const Timestamp &ts) const;

	/**
	 * Computes the number of frames between this timestamp and ts.
	 * The frames are with respect to the framerate used by this
	 * Timestamp (which may differ from the framerate used by ts).
	 */
	int frameDiff(const Timestamp &ts) const;

	/** Computes the number off milliseconds between this timestamp and ts. */
	int msecsDiff(const Timestamp &ts) const;

	/**
	 * Return the time in milliseconds described by this timestamp,
	 * rounded down.
	 */
	int msecs() const;

	/**
	 * Return the time in seconds described by this timestamp,
	 * rounded down.
	 */
	inline int secs() const {
		return _secs;
	}

	/**
	 * Return the time in frames described by this timestamp.
	 */
	inline int totalNumberOfFrames() const {
		return _numFrames / (int)_framerateFactor + _secs * (int)(_framerate / _framerateFactor);
	}

	/**
	 * A timestamp consists of a number of seconds, plus a number
	 * of frames, the latter describing a fraction of a second.
	 * This method returns the latter number.
	 */
	inline int numberOfFrames() const {
		return _numFrames / (int)_framerateFactor;
	}

	/** Return the framerate used by this timestamp. */
	inline uint framerate() const { return _framerate / _framerateFactor; }

protected:
	/**
	 * Compare this timestamp to another one and return
	 * a value similar to strcmp.
	 */
	int cmp(const Timestamp &ts) const;

	/**
	 * Normalize this timestamp by making _numFrames non-negative
	 * and reducing it modulo _framerate.
	 */
	void normalize();

	/**
	 * Add another timestamp to this one and normalize the result.
	 */
	void addIntern(const Timestamp &ts);

protected:
	/**
	 * The seconds part of this timestamp.
	 * The total time in seconds represented by this timestamp can be
	 * computed as follows:
	 *   _secs + (double)_numFrames / _framerate
	 */
	int _secs;

	/**
	 * The number of frames which together with _secs encodes the
	 * timestamp. The total number of *internal* frames represented
	 * by this timestamp can be computed as follows:
	 *   _numFrames + _secs * _framerate
	 * To obtain the number of frames with respect to the original
	 * framerate, this value has to be divided by _framerateFactor.
	 *
	 * This is always a value greater or equal to zero.
	 * The only reason this is an int and not an uint is to
	 * allow intermediate negative values.
	 */
	int _numFrames;

	/**
	 * The internal framerate, i.e. the number of frames per second.
	 * This is computed as the least common multiple of the framerate
	 * specified by the client code, and 1000.
	 * This way, we ensure that we can store both frames and
	 * milliseconds without any rounding losses.
	 */
	uint _framerate;

	/**
	 * Factor by which the original framerate specified by the client
	 * code was multipled to obtain the internal _framerate value.
	 */
	uint _framerateFactor;
};


} // End of namespace Audio

#endif
