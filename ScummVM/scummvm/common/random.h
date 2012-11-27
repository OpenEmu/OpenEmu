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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef COMMON_RANDOM_H
#define COMMON_RANDOM_H

#include "common/scummsys.h"

namespace Common {

class String;

/**
 * Simple random number generator. Although it is definitely not suitable for
 * cryptographic purposes, it serves our purposes just fine.
 */
class RandomSource {
private:
	uint32 _randSeed;

public:
	/**
	 * Construct a new randomness source with the specific name.
	 * The name used name must be globally unique, and is used to
	 * register the randomness source with the active event recorder,
	 * if any.
	 */
	RandomSource(const String &name);

	void setSeed(uint32 seed);

	uint32 getSeed() const {
		return _randSeed;
	}

	/**
	 * Generates a random unsigned integer in the interval [0, max].
	 * @param max	the upper bound
	 * @return	a random number in the interval [0, max]
	 */
	uint getRandomNumber(uint max);

	/**
	 * Generates a random bit, i.e. either 0 or 1.
	 * Identical to getRandomNumber(1), but potentially faster.
	 * @return	a random bit, either 0 or 1
	 */
	uint getRandomBit();

	/**
	 * Generates a random unsigned integer in the interval [min, max].
	 * @param min	the lower bound
	 * @param max	the upper bound
	 * @return	a random number in the interval [min, max]
	 */
	uint getRandomNumberRng(uint min, uint max);
};

}	// End of namespace Common

#endif
