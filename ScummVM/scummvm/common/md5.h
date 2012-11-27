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

#ifndef COMMON_MD5_H
#define COMMON_MD5_H

#include "common/scummsys.h"

namespace Common {

class ReadStream;
class String;

/**
 * Compute the MD5 checksum of the content of the given ReadStream.
 * The 128 bit MD5 checksum is returned directly in the array digest.
 * If length is set to a positive value, then only the first length
 * bytes of the stream are used to compute the checksum.
 * @param[in] stream	the stream of whose data the MD5 is computed
 * @param[out] digest	the computed MD5 checksum
 * @param[in] length	the number of bytes for which to compute the checksum; 0 means all
 * @return true on success, false if an error occurred
 */
bool computeStreamMD5(ReadStream &stream, uint8 digest[16], uint32 length = 0);

/**
 * Compute the MD5 checksum of the content of the given ReadStream.
 * The 128 bit MD5 checksum is converted to a human readable
 * lowercase hex string of length 32.
 * If length is set to a positive value, then only the first length
 * bytes of the stream are used to compute the checksum.
 * @param[in] stream	the stream of whose data the MD5 is computed
 * @param[in] length	the number of bytes for which to compute the checksum; 0 means all
 * @return the MD5 as a hex string on success, and an empty string if an error occurred
 */
String computeStreamMD5AsString(ReadStream &stream, uint32 length = 0);

} // End of namespace Common

#endif
