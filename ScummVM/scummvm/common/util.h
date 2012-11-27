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

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include "common/scummsys.h"
#include "common/str.h"

/**
 * Check whether a given pointer is aligned correctly.
 * Note that 'alignment' must be a power of two!
 */
#define IS_ALIGNED(value, alignment) \
          ((((size_t)value) & ((alignment) - 1)) == 0)


#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

template<typename T> inline T ABS (T x)			{ return (x>=0) ? x : -x; }
template<typename T> inline T MIN (T a, T b)	{ return (a<b) ? a : b; }
template<typename T> inline T MAX (T a, T b)	{ return (a>b) ? a : b; }
template<typename T> inline T CLIP (T v, T amin, T amax)
		{ if (v < amin) return amin; else if (v > amax) return amax; else return v; }

/**
 * Template method which swaps the vaulues of its two parameters.
 */
template<typename T> inline void SWAP(T &a, T &b) { T tmp = a; a = b; b = tmp; }

/**
 * Macro which determines the number of entries in a fixed size array.
 */
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

/**
 * Compute a pointer to one past the last element of an array.
 */
#define ARRAYEND(x) ((x) + ARRAYSIZE((x)))


/**
 * @def SCUMMVM_CURRENT_FUNCTION
 * This macro evaluates to the current function's name on compilers supporting this.
 */
#if defined(__GNUC__)
# define SCUMMVM_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#  define SCUMMVM_CURRENT_FUNCTION	__func__
#elif defined(_MSC_VER) && _MSC_VER >= 1300
#  define SCUMMVM_CURRENT_FUNCTION __FUNCTION__
#else
#  define SCUMMVM_CURRENT_FUNCTION "<unknown>"
#endif

namespace Common {

/**
 * Print a hexdump of the data passed in. The number of bytes per line is
 * customizable.
 * @param data	the data to be dumped
 * @param len	the length of that data
 * @param bytesPerLine	number of bytes to print per line (default: 16)
 * @param startOffset	shift the shown offsets by the starting offset (default: 0)
 */
extern void hexdump(const byte * data, int len, int bytesPerLine = 16, int startOffset = 0);


/**
 * Parse a string for a boolean value.
 * The strings "true", "yes", and "1" are interpreted as true.
 * The strings "false", "no", and "0" are interpreted as false.
 * This function ignores case.
 *
 * @param[in] val			the string to parse
 * @param[out] valAsBool	the parsing result
 * @return 	true if the string parsed correctly, false if an error occurred.
 */
bool parseBool(const String &val, bool &valAsBool);


/**
 * Test whether the given character is alphanumeric (a-z, A-Z, 0-9).
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is alphanumeric, false otherwise.
 */
bool isAlnum(int c);

/**
 * Test whether the given character is an alphabetic letter (a-z, A-Z).
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is TODO, false otherwise.
 */
bool isAlpha(int c);

/**
 * Test whether the given character is a decimal-digit (0-9).
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is a decimal-digit, false otherwise.
 */
bool isDigit(int c);

/**
 * Test whether the given character is a lower-case letter (a-z).
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is a lower-case letter, false otherwise.
 */
bool isLower(int c);

/**
 * Test whether the given character is a white-space.
 * White-space characters are ' ', '\t', '\r', '\n', '\v', '\f'.
 *
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is a white-space, false otherwise.
 */
bool isSpace(int c);

/**
 * Test whether the given character is an upper-case letter (A-Z).
 * If the parameter is outside the range of a signed or unsigned char, then
 * false is returned.
 *
 * @param c		the character to test
 * @return		true if the character is an upper-case letter, false otherwise.
 */
bool isUpper(int c);

}	// End of namespace Common

#endif
