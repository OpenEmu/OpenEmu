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

#ifndef COMMON_ZLIB_H
#define COMMON_ZLIB_H

#include "common/scummsys.h"

namespace Common {

class SeekableReadStream;
class WriteStream;

#if defined(USE_ZLIB)

/**
 * Thin wrapper around zlib's uncompress() function. This wrapper makes
 * it possible to uncompress data in engines without being forced to link
 * them against zlib, thus simplifying the build system.
 *
 * Taken from the zlib manual:
 * Decompresses the src buffer into the dst buffer.
 * srcLen is the byte length of the source buffer. Upon entry, dstLen is the
 * total size of the destination buffer, which must be large enough to hold
 * the entire uncompressed data. Upon exit, dstLen is the actual size of the
 * compressed buffer.
 *
 * @param dst       the buffer to store into.
 * @param dstLen    a pointer to the size of the destination buffer.
 * @param src       the data to be decompressed.
 * @param srcLen    the size of the compressed data.
 *
 * @return true on success (i.e. Z_OK), false otherwise.
 */
bool uncompress(byte *dst, unsigned long *dstLen, const byte *src, unsigned long srcLen);

/**
 * Wrapper around zlib's inflate functions. This function will call the
 * necessary inflate functions to uncompress data compressed with deflate
 * but *not* with the standard zlib header.
 *
 * Decompresses the src buffer into the dst buffer.
 * srcLen is the byte length of the source buffer, dstLen is the byte
 * length of the output buffer.
 * It decompress as much data as possible, up to dstLen bytes.
 * If a dictionary is provided through the dict buffer, uses it to initializes
 * the internal decompression dictionary, before the decompression takes place.
 *
 * @param dst       the buffer to store into.
 * @param dstLen    the size of the destination buffer.
 * @param src       the data to be decompressed.
 * @param dstLen    the size of the compressed data.
 * @param dict      (optional) a decompress dictionary.
 * @param dictLen   (optional) the size of the dictionary.
 *                  Mandatory if dict is not 0.
 *
 * @return true on success (Z_OK or Z_STREAM_END), false otherwise.
 */
bool inflateZlibHeaderless(byte *dst, uint dstLen, const byte *src, uint srcLen, const byte *dict = 0, uint dictLen = 0);

/**
 * Wrapper around zlib's inflate functions. This function will call the
 * necessary inflate functions to uncompress data compressed for InstallShield
 * cabinet files.
 *
 * Decompresses the src buffer into the dst buffer.
 * srcLen is the byte length of the source buffer, dstLen is the byte
 * length of the output buffer.
 * It decompress as much data as possible, up to dstLen bytes.
 *
 * @param dst       the buffer to store into.
 * @param dstLen    the size of the destination buffer.
 * @param src       the data to be decompressed.
 * @param dstLen    the size of the compressed data.
 *
 * @return true on success (Z_OK or Z_STREAM_END), false otherwise.
 */
bool inflateZlibInstallShield(byte *dst, uint dstLen, const byte *src, uint srcLen);

#endif

/**
 * Take an arbitrary SeekableReadStream and wrap it in a custom stream which
 * provides transparent on-the-fly decompression. Assumes the data it
 * retrieves from the wrapped stream to be either uncompressed or in gzip
 * format. In the former case, the original stream is returned unmodified
 * (and in particular, not wrapped).
 *
 * Certain GZip-formats don't supply an easily readable length, if you
 * still need the length carried along with the stream, and you know
 * the decompressed length at wrap-time, then it can be supplied as knownSize
 * here. knownSize will be ignored if the GZip-stream DOES include a length.
 *
 * It is safe to call this with a NULL parameter (in this case, NULL is
 * returned).
 *
 * @param toBeWrapped	the stream to be wrapped (if it is in gzip-format)
 * @param knownSize		a supplied length of the compressed data (if not available directly)
 */
SeekableReadStream *wrapCompressedReadStream(SeekableReadStream *toBeWrapped, uint32 knownSize = 0);

/**
 * Take an arbitrary WriteStream and wrap it in a custom stream which provides
 * transparent on-the-fly compression. The compressed data is written in the
 * gzip format, unless ZLIB support has been disabled, in which case the given
 * stream is returned unmodified (and in particular, not wrapped).
 *
 * It is safe to call this with a NULL parameter (in this case, NULL is
 * returned).
 */
WriteStream *wrapCompressedWriteStream(WriteStream *toBeWrapped);

}	// End of namespace Common

#endif
